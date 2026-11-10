#pragma once
#include <cstdint>

namespace stick {
constexpr uint8_t kDefaultBrightnessIndex = 2;
constexpr uint8_t kBrightnessPresetCount = 5;
// Invalid/missing records fall back to 50%; only explicit confirmations write flash; known unchanged values are skipped.
uint8_t loadBrightnessIndex();
bool saveBrightnessIndex(uint8_t index);
} // namespace stick
