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

// The only layout this firmware draws. Rotation 1 is +90° from portrait.
constexpr DisplayProfile kDisplayProfile{"m5stickc-plus2", "240x135-landscape", 240, 135, 1, 16};
constexpr const DisplayProfile& displayProfile() { return kDisplayProfile; }
}
