#if defined(STICK_SETTINGS_SELF_TEST)
#include <Arduino.h>
#include <Preferences.h>
#include "Settings.h"

namespace {
bool passed = false;
uint8_t original = 255, observed = 255, restored = 255;
}
void setup() {
  pinMode(4, OUTPUT);digitalWrite(4, HIGH);
  Serial.begin(115200);delay(200);
  Preferences marker;
  if (!marker.begin("stick-test-v1", false)) return;
  const auto phase = marker.getUChar("phase", 0);
  if (phase == 0) {
    original = stick::loadBrightnessIndex();
    const uint8_t target = original == 4 ? 0 : 4;
    // Preserve the original before modifying the tested preference.
    if (marker.putUChar("original", original) != 1 || marker.putUChar("phase", 1) != 1) {
      marker.end();return;
    }
    marker.end();
    if (!stick::saveBrightnessIndex(target)) return;
    Serial.println("SETTINGS_SELFTEST restarting to verify NVS");
    Serial.flush();delay(100);ESP.restart();
  } else if (phase == 1) {
    original = marker.getUChar("original", 255);
    if (original >= stick::kBrightnessPresetCount) { marker.end();return; }
    const uint8_t expected = original == 4 ? 0 : 4;
    observed = stick::loadBrightnessIndex();
    const bool saved = stick::saveBrightnessIndex(original);
    restored = stick::loadBrightnessIndex();
    passed = observed == expected && saved && restored == original;
    if (saved && restored == original) marker.clear(); // Only this test's namespace.
    marker.end();
  } else marker.end();
}
void loop() {
  Serial.printf("SETTINGS_SELFTEST %s original=%u persisted=%u restored=%u\n",
                passed ? "PASS" : "FAIL", original, observed, restored);
  delay(2000);
}
#endif
