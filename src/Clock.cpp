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

time_t epochFromCivil(const CivilTime& time) {
  int year = time.year;
  const int month = time.month;
  year -= month <= 2;
  const int era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(year - era * 400);
  const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + time.day - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const int days = era * 146097 + static_cast<int>(doe) - 719468;
  return static_cast<time_t>(days) * 86400 + time.hour * 3600 + time.minute * 60;
}

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

CivilTime clockCivil() {
  time_t now = time(nullptr);
  tm parts{};
  gmtime_r(&now, &parts);
  return {parts.tm_year + 1900, parts.tm_mon + 1, parts.tm_mday, parts.tm_hour, parts.tm_min};
}

bool clockSetCivil(CivilTime time) {
  if (!civilTimeValid(time)) return false;
  const time_t epoch = epochFromCivil(time);
  if (!boardRtcEnabled() || !boardRtcWrite(epoch)) return false;
  time_t stored = 0;
  if (!boardRtcRead(stored) || stored < epoch || stored - epoch > 2) return false;
  return commitClock(stored);
}
}
