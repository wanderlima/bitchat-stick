#include "Clock.h"
#include <M5Unified.h>
#include <sys/time.h>
#include <ctime>
#include "BuildTime.h"

namespace stick {
namespace {
bool rtc = false;
bool readRtc(time_t& epoch) {
  m5::rtc_datetime_t value;
  if (!M5.Rtc.getDateTime(&value)) return false;
  if (value.date.year < 2000 || value.date.year > 2099 || value.date.month < 1 ||
      value.date.month > 12 || value.date.date < 1 || value.date.date > 31 ||
      uint8_t(value.time.hours) >= 24 || uint8_t(value.time.minutes) >= 60 ||
      uint8_t(value.time.seconds) >= 60) return false;
  auto fields = value.get_tm();
  epoch = mktime(&fields);
  return epoch >= 0;
}
}
bool beginClock() {
  setenv("TZ", "UTC0", 1);
  tzset();
  time_t epoch = STICK_BUILD_EPOCH;
  time_t stored = 0;
  rtc = M5.Rtc.isEnabled() && readRtc(stored);
  if (rtc && stored >= epoch) epoch = stored;
  else if (M5.Rtc.isEnabled()) {
    tm utc;
    gmtime_r(&epoch, &utc);
    M5.Rtc.setDateTime(&utc);
    // Older M5Unified setters return void. Verify the write through readback.
    rtc = readRtc(stored) && stored >= epoch && stored - epoch <= 2;
    if (rtc) epoch = stored;
  }
  timeval tv{epoch, 0};
  return settimeofday(&tv, nullptr) == 0;
}
uint64_t clockMilliseconds() {
  timeval tv{};
  gettimeofday(&tv, nullptr);
  return uint64_t(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}
bool clockRtc() { return rtc; }
}
