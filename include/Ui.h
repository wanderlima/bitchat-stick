#pragma once
#include "PeerTable.h"
namespace stick {
void beginUi();
void updateUi(const PeerTable& peers, uint32_t now);
} // namespace stick
