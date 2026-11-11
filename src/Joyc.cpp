#include "Joyc.h"
#include <M5GFX.h>

namespace stick {
namespace {
// Mini JoyC U156: HAT SDA G0/SCL G26, separate from the Plus2 internal RTC bus.
constexpr uint8_t address = 0x54;
constexpr uint8_t position = 0x20; // Each axis requires its own one-byte transaction.
constexpr uint32_t sampleInterval = 20, retryInterval = 1000;
// A software port avoids the Arduino hardware-I2C driver and its locks.
constexpr int bus = -1;
JoycSample sample;
bool initialized = false, polled = false;
uint32_t lastPoll = 0;

int16_t signedAxis(int raw) { return raw < 128 ? raw : raw - 256; }
bool readRegister(uint8_t reg, uint8_t& value) {
  return lgfx::i2c::transactionWrite(bus, address, &reg, 1, 100000).has_value()
      && lgfx::i2c::transactionRead(bus, address, &value, 1, 100000).has_value();
}
void recoverBus() {
  lgfx::i2c::release(bus);
  initialized = false;
}
}

JoycSample pollJoyc(uint32_t now, bool readButton) {
  if (polled && uint32_t(now - lastPoll) < (sample.valid ? sampleInterval : retryInterval)) return sample;
  polled = true;
  lastPoll = now;
  sample = {};
  if (!initialized) {
    initialized = lgfx::i2c::init(bus, 0, 26).has_value();
    if (!initialized) return sample;
  }
  uint8_t axes[2]{};
  for (uint8_t axis = 0; axis < 2; ++axis) {
    if (!readRegister(position + axis, axes[axis])) {
      recoverBus();
      return sample;
    }
  }
  const int x = axes[0], y = axes[1];
  sample.x = signedAxis(x);
  sample.y = signedAxis(y);
  sample.valid = true;
  if (readButton) {
    uint8_t button = 1;
    if (readRegister(0x30, button) && button <= 1) {
      sample.buttonValid = true;
      sample.pressed = button == 0;
    } else recoverBus();
  }
  return sample;
}
bool setJoycLed(uint8_t brightness) {
  if (!initialized || !sample.valid) return false;
  const uint8_t payload[] = {0x40, 0, brightness, 0};
  if (lgfx::i2c::transactionWrite(bus, address, payload, sizeof(payload), 100000).has_value()) return true;
  sample = {};
  recoverBus();
  return false;
}

}
