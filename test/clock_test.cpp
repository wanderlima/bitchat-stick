#include "Clock.h"
#include "BoardClock.h"
#include <cassert>
#include <cstdio>

namespace stick {
int reads = 0, writes = 0;
bool enabled = false, readOk = false, readbackOk = false;
time_t value = 0, readback = 0;

bool boardRtcEnabled() { return enabled; }
bool boardRtcRead(time_t& epoch) {
  ++reads;
  if (reads == 1) {
    if (!readOk) return false;
    epoch = value;
    return true;
  }
  if (!readbackOk) return false;
  epoch = readback;
  return true;
}
bool boardRtcWrite(time_t) { ++writes; return true; }
}

int main() {
  using namespace stick;
  constexpr time_t build = 1700000000;
  enabled = true;
  readOk = true;
  value = build + 100;
  assert(beginClock());
  assert(clockRtc());
  assert(writes == 0);

  reads = writes = 0;
  readOk = false;
  readbackOk = false;
  assert(beginClock());
  assert(!clockRtc());
  assert(writes == 1);

  reads = writes = 0;
  readbackOk = true;
  readback = build + 5;
  assert(beginClock());
  assert(!clockRtc());
  assert(writes == 1);

  reads = writes = 0;
  enabled = false;
  assert(beginClock());
  assert(!clockRtc());
  assert(writes == 0);
  assert(clockMilliseconds() > uint64_t(build + 100) * 1000);
  std::puts("clock: newer RTC, invalid RTC, rejected readback and system time passed");
}
