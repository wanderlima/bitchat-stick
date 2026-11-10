#include "Joyc.h"
#include <Wire.h>

namespace stick {
namespace {
// Mini JoyC U156: HAT SDA G0/SCL G26, separate from the Plus2 internal RTC bus.
constexpr uint8_t address = 0x54;
constexpr uint8_t position = 0x20; // Two consecutive signed 8-bit X/Y registers.
constexpr uint32_t sampleInterval = 20, retryInterval = 1000;
TwoWire bus(0);
JoycSample sample;
bool initialized = false, polled = false;
uint32_t lastPoll = 0;

int16_t signedAxis(int raw) { return raw < 128 ? raw : raw - 256; }
}

JoycSample pollJoyc(uint32_t now) {
  if (polled && uint32_t(now - lastPoll) < (sample.valid ? sampleInterval : retryInterval)) return sample;
  polled = true;
  lastPoll = now;
  sample = {};
  if (!initialized) {
    bus.setTimeOut(5);
    initialized = bus.begin(0, 26, 100000);
    if (!initialized) return sample;
  }
  bus.beginTransmission(address);
  bus.write(position);
  // Use the register-write/STOP/read sequence from the vendor library.
  if (bus.endTransmission() != 0 || bus.requestFrom(address, size_t(2), true) != 2) return sample;
  const int x = bus.read(), y = bus.read();
  if (x < 0 || y < 0) return sample;
  sample.x = signedAxis(x);
  sample.y = signedAxis(y);
  sample.valid = true;
  return sample;
}
}
