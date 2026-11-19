#pragma once
#include <cstdint>

namespace stick {
enum class NavigationAction { None, Wake, Select, Up, Down, Back, Left, Right };

// C is the Plus2 power button: only a short release may navigate.
// B-long fires once while held and never becomes B-short on release.
class NavigationInput {
 public:
  NavigationAction update(bool a, bool b, bool c, uint32_t now, bool sleeping) {
    if (sleeping && (a || b || c)) {
      suppress_ = true;
      return NavigationAction::Wake;
    }
    if (suppress_) {
      if (!a && !b && !c) {
        suppress_ = false;
        a_ = {}; b_ = {}; c_ = {};
      }
      return NavigationAction::None;
    }
    const auto ae = a_.update(a, now);
    const auto be = b_.update(b, now);
    const auto ce = c_.update(c, now);
    if (be == Press::Long) return NavigationAction::Back;
    if (ae == Press::Short) return NavigationAction::Select;
    if (be == Press::Short) return NavigationAction::Up;
    if (ce == Press::Short) return NavigationAction::Down;
    return NavigationAction::None;
  }
 private:
  enum class Press { None, Short, Long };
  struct Button {
    bool down = false, longSent = false;
    uint32_t since = 0;
    Press update(bool pressed, uint32_t now) {
      Press event = Press::None;
      if (pressed && !down) { since = now; longSent = false; }
      if (pressed && !longSent && uint32_t(now - since) >= 700) {
        longSent = true;
        event = Press::Long;
      }
      if (!pressed && down && !longSent) event = Press::Short;
      down = pressed;
      return event;
    }
  } a_, b_, c_;
  bool suppress_ = false;
};
} // namespace stick
