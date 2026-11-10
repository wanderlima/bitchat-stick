#pragma once
#include <cstdint>
inline uint32_t fakeMillis = 0;
inline uint32_t millis() { return fakeMillis; }
