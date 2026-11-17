#pragma once
#include <stdint.h>

#ifndef STICK_BOARD_M5STICKC_PLUS2
#error "Define STICK_BOARD_M5STICKC_PLUS2. This firmware has no implicit generic ESP32 board."
#endif

namespace stick {
class PeerTable;
struct BoardButtons {
  bool a;
  bool b;
  bool power;
  BoardButtons(bool aIn = false, bool bIn = false, bool powerIn = false)
      : a(aIn), b(bIn), power(powerIn) {}
};

void boardEarlyPower();
void boardBegin();
void boardPoll();
void boardPowerOff();
const char* boardPeripheralStatus(uint32_t now);
void boardBeginUi();
void boardUpdateUi(const PeerTable& peers, uint32_t now);
BoardButtons boardButtons();
void boardBattery(int& voltageMv, int& percent);
uint32_t boardFreeHeap();
}
