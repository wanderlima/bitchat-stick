#pragma once
#include "NavigationInput.h"
#include <cstdint>

namespace stick {
// Independent of I2C: only valid, signed normalized samples enter this policy.
class JoycNavigation {
 public:
  NavigationAction update(int16_t x, int16_t y, bool valid, uint32_t now,
                          bool sleeping, bool buttonsBusy = false) {
    using A = NavigationAction;
    if (!valid || buttonsBusy) {
      blocked_ = true;
      held_ = A::None;
      return A::None;
    }
    const bool centered = x >= -10 && x <= 10 && y >= -10 && y <= 10;
    if (centered) {
      blocked_ = false;
      held_ = A::None;
      return A::None;
    }
    const auto direction = classify(x, y);
    if (sleeping && direction != A::None) {
      blocked_ = true;
      held_ = A::None;
      return A::Wake;
    }
    if (blocked_) return A::None;
    if (direction == A::Back || direction == A::Select) {
      blocked_ = true; // One horizontal action for the whole gesture.
      held_ = A::None;
      return direction;
    }
    if (direction == A::None) {
      held_ = A::None; // Middle values do nothing and stop a held repeat.
      return A::None;
    }
    if (direction != held_ || uint32_t(now - lastRepeat_) >= 1500) {
      held_ = direction;
      lastRepeat_ = now;
      return direction;
    }
    return A::None;
  }
 private:
  static NavigationAction classify(int16_t x, int16_t y) {
    const int ax = x < 0 ? -int(x) : int(x);
    const int ay = y < 0 ? -int(y) : int(y);
    // A diagonal produces at most one action. Equal diagonals are ignored.
    if (ax > 80 && ax > ay) return x < 0 ? NavigationAction::Back : NavigationAction::Select;
    if (ay > 80 && ay > ax) return y < 0 ? NavigationAction::Up : NavigationAction::Down;
    return NavigationAction::None;
  }
  bool blocked_ = true; // Boot/reconnect must first observe a real centered sample.
  NavigationAction held_ = NavigationAction::None;
  uint32_t lastRepeat_ = 0;
};
}
