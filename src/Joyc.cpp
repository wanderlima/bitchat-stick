#include "Joyc.h"
#include <M5Unified.h>

namespace stick {
namespace {
// Mini JoyC U156: HAT SDA G0/SCL G26, separate from the Plus2 internal RTC bus.
uint8_t address = 0x54;
bool classic = false, detected = false;
constexpr uint8_t position = 0x20; // Each axis requires its own one-byte transaction.
constexpr uint32_t sampleInterval = 20, retryInterval = 1000;
// Use the same M5Unified controller and STOP/read sequence as the working Nemo project.
JoycSample sample;
bool initialized = false, polled = false;
uint32_t lastPoll = 0;

int16_t signedAxis(int raw) { return raw < 128 ? raw : raw - 256; }
bool ping(uint8_t candidate) {
  if (!M5.Ex_I2C.start(candidate, false, 100000)) return false;
  return M5.Ex_I2C.stop();
}
bool readRegister(uint8_t reg, uint8_t& value) {
  if (!M5.Ex_I2C.start(address, false, 100000)) return false;
  const bool selected = M5.Ex_I2C.write(reg);
  const bool selectStopped = M5.Ex_I2C.stop();
  if (!selected || !selectStopped || !M5.Ex_I2C.start(address, true, 100000)) return false;
  const bool received = M5.Ex_I2C.read(&value, 1, true);
  const bool readStopped = M5.Ex_I2C.stop();
  return received && readStopped;
}
void recoverBus() {
  M5.Ex_I2C.release();
  initialized = false;
  detected = false;
}
}

JoycSample pollJoyc(uint32_t now, bool readButton) {
  if (polled && uint32_t(now - lastPoll) < (sample.valid ? sampleInterval : retryInterval)) return sample;
  polled = true;
  lastPoll = now;
  sample = {};
  if (!initialized) {
    initialized = M5.Ex_I2C.begin(I2C_NUM_0, 0, 26);
    if (!initialized) return sample;
  }
  if (!detected) {
    if (ping(0x54)) {
      address = 0x54;
      classic = false;
    } else if (ping(0x38)) {
      address = 0x38;
      classic = true;
    } else {
      recoverBus();
      return sample;
    }
    detected = true;
  }
  uint8_t axes[2]{};
  for (uint8_t axis = 0; axis < 2; ++axis) {
    if (!readRegister((classic ? 0x60 : position) + axis, axes[axis])) {
      recoverBus();
      return sample;
    }
  }
  const int x = classic ? int(axes[0]) - 100 : signedAxis(axes[0]);
  const int y = classic ? int(axes[1]) - 100 : signedAxis(axes[1]);
  // Portrait map, confirmed on the Plus2: raw X increases right and raw Y increases up.
  // Rotation 3 turns that portrait 90° CCW, so landscape right is the old down
  // and landscape down is the old left.
  const int portraitX = x < -127 ? -127 : (x > 127 ? 127 : x);
  int portraitY = -y;
  if (portraitY > 127) portraitY = 127;
  if (portraitY < -127) portraitY = -127;
  sample.x = portraitY;
  sample.y = -portraitX;
  if (sample.y > 127) sample.y = 127;
  if (sample.y < -127) sample.y = -127;
  sample.valid = true;
  if (readButton) {
    uint8_t button = 1;
    if (readRegister(classic ? 0x64 : 0x30, button) && (classic || button <= 1)) {
      sample.buttonValid = true;
      sample.pressed = classic ? (button & 0x11) != 0 : button == 0;
    } else recoverBus();
  }
  return sample;
}
bool setJoycLed(uint8_t brightness) {
  if (!initialized || !sample.valid) return false;
  const uint8_t payload[] = {uint8_t(classic ? 0x20 : 0x40), 0, brightness, 0};
  if (M5.Ex_I2C.start(address, false, 100000)) {
    const bool written = M5.Ex_I2C.write(payload, sizeof(payload));
    const bool stopped = M5.Ex_I2C.stop();
    if (written && stopped) return true;
  }
  sample = {};
  recoverBus();
  return false;
}

}
