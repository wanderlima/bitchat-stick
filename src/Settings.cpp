#include "Settings.h"
#include <Preferences.h>

namespace stick {
namespace {
constexpr uint32_t recordVersion = 0xBC510100;
uint8_t savedIndex = kDefaultBrightnessIndex;
bool savedRecordKnown = false;
}
uint8_t loadBrightnessIndex() {
  savedIndex = kDefaultBrightnessIndex;
  savedRecordKnown = false;
  Preferences prefs;
  if (!prefs.begin("bitchat-stick", true)) return savedIndex;
  const auto record = prefs.getUInt("ui-v1", 0);
  prefs.end();
  const auto index = uint8_t(record);
  if ((record & 0xFFFFFF00) == recordVersion && index < kBrightnessPresetCount) {
    savedIndex = index;
    savedRecordKnown = true;
  }
  return savedIndex;
}
bool saveBrightnessIndex(uint8_t index) {
  if (index >= kBrightnessPresetCount) return false;
  if (savedRecordKnown && index == savedIndex) return true;
  Preferences prefs;
  if (!prefs.begin("bitchat-stick", false)) return false;
  // One NVS scalar keeps schema and value together across interrupted writes.
  const bool saved = prefs.putUInt("ui-v1", recordVersion | index) == sizeof(uint32_t);
  prefs.end();
  if (saved) { savedIndex = index; savedRecordKnown = true; }
  return saved;
}
} // namespace stick

namespace stick {
namespace {
constexpr uint32_t flagsVersion = 0xBC520100;
constexpr uint8_t validFlags = SoundUi | SoundDm | SoundMesh | JoycClick | JoycLed;
uint8_t savedFlags = 0;
bool flagsKnown = false;
}
uint8_t loadPreferenceFlags() {
  savedFlags = 0;
  flagsKnown = false;
  Preferences prefs;
  if (!prefs.begin("bitchat-stick", true)) return savedFlags;
  const uint32_t record = prefs.getUInt("opts-v1", 0);
  prefs.end();
  const uint8_t flags = uint8_t(record);
  if ((record & 0xFFFFFF00) == flagsVersion && !(flags & ~validFlags)) {
    savedFlags = flags;
    flagsKnown = true;
  }
  return savedFlags;
}
bool savePreferenceFlags(uint8_t flags) {
  if (flags & ~validFlags) return false;
  if (flagsKnown && flags == savedFlags) return true;
  Preferences prefs;
  if (!prefs.begin("bitchat-stick", false)) return false;
  const bool saved = prefs.putUInt("opts-v1", flagsVersion | flags) == sizeof(uint32_t);
  prefs.end();
  if (saved) {
    savedFlags = flags;
    flagsKnown = true;
  }
  return saved;
}
}
