#pragma once
#include <cstdint>
namespace stick {
struct CivilTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
};

inline bool civilTimeValid(CivilTime time) {
  if (time.year < 2000 || time.year > 2099 || time.month < 1 || time.month > 12) return false;
  if (time.hour < 0 || time.hour > 23 || time.minute < 0 || time.minute > 59) return false;
  const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int limit = days[time.month];
  const bool leap = time.month == 2 && (time.year % 400 == 0 || (time.year % 4 == 0 && time.year % 100 != 0));
  if (leap) limit = 29;
  return time.day >= 1 && time.day <= limit;
}

bool beginClock();
uint64_t clockMilliseconds();
bool clockRtc();
CivilTime clockCivil();
bool clockSetCivil(CivilTime time);
}
