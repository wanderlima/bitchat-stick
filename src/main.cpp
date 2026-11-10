#include <Arduino.h>
#include <M5Unified.h>
#include "PeerTable.h"
#include "Scanner.h"
#include "Ui.h"
#include "Mesh.h"
#include "Transport.h"
#include "Clock.h"

stick::PeerTable peers;
void setup() {
  // Plus2 power hold must be asserted early, including battery-only boot.
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  cfg.fallback_board = m5::board_t::board_M5StickCPlus2;
  cfg.internal_mic = false;
  cfg.internal_spk = false;
  cfg.internal_imu = false;
  cfg.internal_rtc = true;
  M5.begin(cfg);
  if (M5.getBoard() != m5::board_t::board_M5StickCPlus2) {
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
  stick::beginUi();
  stick::beginClock();
  Serial.println("Bitchat Stick 0.2.0 | signed BLE mesh | authenticated InfoBot");
  if (!stick::beginScanner()) Serial.println("BLE: scanner initialization failed.");
  else if (!stick::beginTransport()) Serial.println("BLE: transport/identity initialization failed.");
}
void loop() {
  M5.update();
  const uint32_t now = millis();
  stick::pollScanner(peers, now);
  stick::mesh().tick(now, stick::clockMilliseconds());
  stick::pollTransport(now);
  stick::updateUi(peers, now);
  if (M5.BtnPWR.pressedFor(6000)) M5.Power.powerOff();
  delay(10);
}
