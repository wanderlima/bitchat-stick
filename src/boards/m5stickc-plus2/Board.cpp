#include "Board.h"
#include "BoardClock.h"
#include "Joyc.h"
#include "Ui.h"
#include <M5Unified.h>
#include <ctime>

namespace stick {
namespace {
bool validRtc(const m5::rtc_datetime_t& value, time_t& epoch) {
  if (value.date.year < 2000 || value.date.year > 2099 || value.date.month < 1 ||
      value.date.month > 12 || value.date.date < 1 || value.date.date > 31 ||
      uint8_t(value.time.hours) >= 24 || uint8_t(value.time.minutes) >= 60 ||
      uint8_t(value.time.seconds) >= 60) return false;
  auto fields = value.get_tm();
  epoch = mktime(&fields);
  return epoch >= 0;
}
}

void boardEarlyPower() {
  // Plus2 power hold must be asserted early, including battery-only boot.
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
}

void boardBegin() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  cfg.fallback_board = m5::board_t::board_M5StickCPlus2;
  cfg.internal_mic = false;
  cfg.internal_spk = true;
  cfg.internal_imu = false;
  cfg.internal_rtc = true;
  M5.begin(cfg);
  if (M5.getBoard() == m5::board_t::board_M5StickCPlus2) return;
  Serial.println("ERROR: detected board is not M5StickC Plus2; initialization stopped.");
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.print("Wrong board");
  while (true) {
    M5.update();
    if (M5.BtnPWR.pressedFor(6000)) M5.Power.powerOff();
    delay(10);
  }
}

void boardPoll() { M5.update(); }

void boardPowerOff() {
  if (M5.BtnPWR.pressedFor(6000)) M5.Power.powerOff();
}

const char* boardPeripheralStatus(uint32_t now) {
  return pollJoyc(now).valid ? "present" : "absent";
}

void boardBeginUi() { beginUi(); }

void boardUpdateUi(const PeerTable& peers, uint32_t now) { updateUi(peers, now); }

BoardButtons boardButtons() {
  return {M5.BtnA.isPressed(), M5.BtnB.isPressed(), M5.BtnPWR.isPressed()};
}

void boardBattery(int& voltageMv, int& percent) {
  voltageMv = M5.Power.getBatteryVoltage();
  percent = voltageMv > 0 ? M5.Power.getBatteryLevel() : -1;
}

uint32_t boardFreeHeap() { return ESP.getFreeHeap(); }

bool boardRtcEnabled() { return M5.Rtc.isEnabled(); }

bool boardRtcRead(time_t& epoch) {
  m5::rtc_datetime_t value;
  if (!M5.Rtc.getDateTime(&value)) return false;
  return validRtc(value, epoch);
}

bool boardRtcWrite(time_t epoch) {
  tm utc;
  gmtime_r(&epoch, &utc);
  M5.Rtc.setDateTime(&utc);
  return true;
}
}
