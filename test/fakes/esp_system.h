#pragma once
#include <cstddef>
#include <cstdint>
inline void esp_fill_random(void* data,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(data)[i]=uint8_t(i+101);}
