#pragma once
#include <ctime>

namespace stick {
bool boardRtcEnabled();
bool boardRtcRead(time_t& epoch);
bool boardRtcWrite(time_t epoch);
}
