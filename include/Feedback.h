#pragma once
#include <cstdint>
#include "../src/NavigationInput.h"
namespace stick {
void updateFeedback(uint32_t now, uint8_t preferences, NavigationAction action,
                    uint32_t receivedDm, uint32_t receivedMesh);
}
