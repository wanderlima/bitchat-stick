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
  // A fallback is not evidence that NVS already holds the same choice.
  writable = true;record = 0xBC510104;readable = false;
  assert(loadBrightnessIndex()==2);
  readable = true;assert(saveBrightnessIndex(2) && record==0xBC510102);
  record = 0xFFFFFFFF;assert(loadBrightnessIndex()==2);
  assert(saveBrightnessIndex(2) && record==0xBC510102); // Explicit apply repairs an invalid record.
  const auto unchangedWrites=writes;assert(saveBrightnessIndex(2) && writes==unchangedWrites);
  assert(loadPreferenceFlags()==0);
  assert(savePreferenceFlags(SoundUi|JoycLed));assert(loadPreferenceFlags()==(SoundUi|JoycLed));
  const auto flagWrites=writes;assert(savePreferenceFlags(SoundUi|JoycLed)&&writes==flagWrites);
  assert(!savePreferenceFlags(0x80));assert(record==0xBC510102);
  writeSucceeds=false;assert(!savePreferenceFlags(JoycClick));assert(loadPreferenceFlags()==(SoundUi|JoycLed));
  writeSucceeds=true;assert(savePreferenceFlags(JoycClick));assert(loadPreferenceFlags()==JoycClick);
  flagsRecord=0xBC5201FF;assert(loadPreferenceFlags()==0);assert(savePreferenceFlags(0));assert(loadPreferenceFlags()==0);
  std::puts("Settings: reboot, validation, schema, unchanged values and storage failures passed");
}
