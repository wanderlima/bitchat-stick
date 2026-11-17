#pragma once

namespace stick {
struct BoardProfile {
  const char* id;
  bool display;
  bool input;
  bool battery;
  bool speaker;
  bool rtc;
  bool hat;
};

constexpr bool boardProfileValid(const BoardProfile& profile) {
  return profile.id && profile.id[0] && !(profile.display && !profile.input) &&
         !(profile.hat && !profile.input);
}

const BoardProfile& boardProfile();
}
