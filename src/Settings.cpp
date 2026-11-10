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
