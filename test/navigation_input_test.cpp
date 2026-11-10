#include "../src/NavigationInput.h"
#include <cassert>
#include <cstdio>
using stick::NavigationAction;
using stick::NavigationInput;
int main() {
  using A = NavigationAction;
  NavigationInput input;
  assert(input.update(true, false, false, 0, false) == A::None);
  assert(input.update(false, false, false, 100, false) == A::Select);
  input.update(false, true, false, 200, false);
  assert(input.update(false, false, false, 300, false) == A::Up);
  input.update(false, false, true, 400, false);
  assert(input.update(false, false, false, 500, false) == A::Down);
  input.update(false, true, false, 600, false);
  assert(input.update(false, true, false, 1299, false) == A::None);
  assert(input.update(false, true, false, 1300, false) == A::Back);
  assert(input.update(false, true, false, 1500, false) == A::None);
  assert(input.update(false, false, false, 1600, false) == A::None);
  // Long C is reserved for power; releasing must never navigate.
  input.update(false, false, true, 2000, false);
  assert(input.update(false, false, true, 8000, false) == A::None);
  assert(input.update(false, false, false, 8100, false) == A::None);
  // Each physical button wakes without triggering its normal action.
  for (unsigned button = 0; button < 3; ++button) {
    assert(input.update(button == 0, button == 1, button == 2, 9000, true) == A::Wake);
    assert(input.update(button == 0, button == 1, button == 2, 10000, false) == A::None);
    assert(input.update(false, false, false, 10100, false) == A::None);
  }
  // No selection after long A, and wraparound does not break B-long.
  input.update(true, false, false, 11000, false);
  assert(input.update(true, false, false, 11700, false) == A::None);
  assert(input.update(false, false, false, 11800, false) == A::None);
  input.update(false, true, false, UINT32_MAX - 400, false);
  assert(input.update(false, true, false, 299, false) == A::Back);
  assert(input.update(false, false, false, 300, false) == A::None);
  std::puts("NavigationInput: A/B/C, long press, wake suppression and wrap passed");
}
