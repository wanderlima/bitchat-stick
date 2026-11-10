#include "Settings.h"
#include <Preferences.h>
#include <cassert>
#include <cstdio>
#include <initializer_list>
int main() {
  using namespace stick;
  using namespace fake_preferences;
  assert(loadBrightnessIndex() == 2 && writes == 0);
  for (uint32_t invalid : {0u, 0xBC510105u, 0xBC5101FFu, 0xBC510203u, 0xFFFFFFFFu}) {
    record = invalid; assert(loadBrightnessIndex() == 2);
  }
  record = 0xBC510104; assert(loadBrightnessIndex() == 4);
  assert(saveBrightnessIndex(4) && writes == 0);
  assert(!saveBrightnessIndex(5) && writes == 0);
  assert(saveBrightnessIndex(0) && record == 0xBC510100 && writes == 1);
  assert(loadBrightnessIndex() == 0); // Simulated reboot reads the previous explicit choice.
  writeSucceeds = false;
  assert(!saveBrightnessIndex(3) && record == 0xBC510100);
  writeSucceeds = true;
  assert(saveBrightnessIndex(3) && record == 0xBC510103); // Failure does not poison retries.
  writable = false; assert(!saveBrightnessIndex(1));
  readable = false; assert(loadBrightnessIndex() == 2);
  std::puts("Settings: reboot, validation, schema, unchanged values and storage failures passed");
}
