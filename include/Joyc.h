#pragma once
#include <cstdint>

namespace stick {
struct JoycSample {
  int16_t x = 0, y = 0;
  bool valid = false;
};
// Poll an optional Mini JoyC on the HAT connector. Invalid samples never navigate.
JoycSample pollJoyc(uint32_t now);
}
