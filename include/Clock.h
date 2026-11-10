#pragma once
#include <cstdint>
namespace stick {
bool beginClock();
uint64_t clockMilliseconds();
bool clockRtc();
}
