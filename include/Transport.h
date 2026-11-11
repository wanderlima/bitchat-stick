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
struct TransportDiagnostics {uint32_t writes=0,notified=0,notifyFailed=0;size_t pending=0;};
TransportDiagnostics transportDiagnostics();
}
