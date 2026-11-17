#include "BoardProfile.h"
#include <cassert>
#include <cstring>
#include <cstdio>

int main() {
  const stick::BoardProfile& profile = stick::boardProfile();
  assert(std::strcmp(profile.id, "m5stickc-plus2") == 0);
  assert(profile.display && profile.input && profile.battery && profile.speaker && profile.rtc && profile.hat);
  assert(stick::boardProfileValid(profile));
  constexpr stick::BoardProfile contradictory{"bare", true, false, false, false, false, false};
  static_assert(!stick::boardProfileValid(contradictory));
  std::puts("board profile: Plus2 capabilities and contradictory display without input passed");
}
