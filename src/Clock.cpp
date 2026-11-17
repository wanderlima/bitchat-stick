#include "Clock.h"
#include "BoardClock.h"
#ifndef STICK_BUILD_EPOCH
#include "BuildTime.h"
#endif
#include <sys/time.h>
#include <cstdlib>

namespace stick {
namespace {
bool rtc = false;

bool commitClock(time_t epoch) {
#if defined(STICK_CLOCK_DRY_RUN)
  (void)epoch;
  return true;
#else
  timeval tv{epoch, 0};
  return settimeofday(&tv, nullptr) == 0;
#endif
}
}

bool beginClock() {
  setenv("TZ", "UTC0", 1);
  tzset();
  time_t epoch = STICK_BUILD_EPOCH;
  time_t stored = 0;
  const bool enabled = boardRtcEnabled();
  rtc = enabled && boardRtcRead(stored);
  if (rtc && stored >= epoch) epoch = stored;
  else if (enabled) {
    boardRtcWrite(epoch);
    rtc = boardRtcRead(stored) && stored >= epoch && stored - epoch <= 2;
    if (rtc) epoch = stored;
  }
  return commitClock(epoch);
}

uint64_t clockMilliseconds() {
  timeval tv{};
  gettimeofday(&tv, nullptr);
  return uint64_t(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

bool clockRtc() { return rtc; }
}
