#pragma once
#include <cstddef>
#include <cstdint>
namespace stick {
bool beginTransport();
void pollTransport(uint32_t now);
bool transportReady();
size_t transportLinks();
uint32_t transportDrops();
const char* transportStatus();
}
