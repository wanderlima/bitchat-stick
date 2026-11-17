#pragma once
#include <stdint.h>

namespace stick {
struct DisplayProfile {
  const char* id;
  const char* layoutId;
  uint16_t width;
  uint16_t height;
  uint8_t rotation;
  uint8_t colorDepth;
};

constexpr bool sameText(const char* left, const char* right) {
  return *left == *right && (!*left || sameText(left + 1, right + 1));
}

// The only layout this firmware draws. A different resolution needs its own layout.
constexpr DisplayProfile kDisplayProfile{"m5stickc-plus2", "135x240-portrait", 135, 240, 0, 16};
constexpr const DisplayProfile& displayProfile() { return kDisplayProfile; }
}
