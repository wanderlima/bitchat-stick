#pragma once
#include "PeerTable.h"
#include <cstdint>
namespace stick {
bool beginScanner();
void pollScanner(PeerTable& peers, uint32_t now);
bool scannerActive();
bool scannerFault();
uint32_t scannerDrops();
uint32_t scannerReports();
} // namespace stick
