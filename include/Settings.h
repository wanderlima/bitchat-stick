#pragma once
#include <cstdint>

namespace stick {
constexpr uint8_t kDefaultBrightnessIndex = 2;
constexpr uint8_t kBrightnessPresetCount = 5;
// Invalid/missing records fall back to 50%; only explicit confirmations write flash; known unchanged values are skipped.
uint8_t loadBrightnessIndex();
bool saveBrightnessIndex(uint8_t index);
} // namespace stick

namespace stick {
enum PreferenceFlag : uint8_t {
  SoundUi = 1 << 0,
  SoundDm = 1 << 1,
  SoundMesh = 1 << 2,
  JoycClick = 1 << 3,
  JoycLed = 1 << 4
};
uint8_t loadPreferenceFlags();
bool savePreferenceFlags(uint8_t flags);
}
