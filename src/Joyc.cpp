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
}

JoycSample pollJoyc(uint32_t now) {
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
    const uint8_t reg = position + axis;
    const bool selected = lgfx::i2c::transactionWrite(bus, address, &reg, 1, 100000).has_value();
    const bool received = selected && lgfx::i2c::transactionRead(bus, address, &axes[axis], 1, 100000).has_value();
    if (!received) {
      lgfx::i2c::release(bus);
      initialized = false;
      return sample;
    }
  }
  const int x = axes[0], y = axes[1];
  sample.x = signedAxis(x);
  sample.y = signedAxis(y);
  sample.valid = true;
  return sample;
}
}
