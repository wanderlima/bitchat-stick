#include <Arduino.h>
#include "Board.h"
#include "PeerTable.h"
#include "Scanner.h"
#include "Mesh.h"
#include "Transport.h"
#include "Clock.h"
#include "Version.h"

stick::PeerTable peers;
void setup() {
  stick::boardEarlyPower();
  stick::boardBegin();
  stick::boardBeginUi();
  stick::beginClock();
  Serial.println("Bitchat Stick " STICK_VERSION " | signed BLE mesh | authenticated InfoBot");
  if (!stick::beginScanner()) Serial.println("BLE: scanner initialization failed.");
  else if (!stick::beginTransport()) Serial.println("BLE: transport/identity initialization failed.");
}
void loop() {
  stick::boardPoll();
  const uint32_t now = millis();
  stick::pollScanner(peers, now);
  stick::mesh().tick(now, stick::clockMilliseconds());
  stick::pollTransport(now);
  stick::boardUpdateUi(peers, now);
  stick::boardPowerOff();
  static uint32_t lastReport = 0;
  if (uint32_t(now - lastReport) >= 10000) {
    lastReport = now;
    const auto& stats = stick::mesh().stats();
    const auto radio = stick::transportDiagnostics();
    Serial.printf("mesh links=%u peers=%u auth=%u accepted=%lu rejected=%lu queued=%lu relay=%lu bot=%lu botState=%s intros=%lu drops=%lu heap=%u clock=%s joyc=%s up=%lu epoch=%llu rx=%lu pub=%lu dm=%lu txq=%u notify=%lu fail=%lu reject=%s send=%s\n",
      unsigned(stick::transportLinks()), unsigned(stick::mesh().peerCount()), unsigned(stick::mesh().authenticatedCount()),
      (unsigned long)stats.accepted, (unsigned long)stats.rejected, (unsigned long)stats.sent, (unsigned long)stats.relayed,
      (unsigned long)stats.botReplies, stick::mesh().botStatus(), (unsigned long)stats.botIntroductions, (unsigned long)(stats.dropped + stick::transportDrops()), unsigned(ESP.getFreeHeap()), stick::clockRtc() ? "RTC" : "build", stick::boardPeripheralStatus(now), (unsigned long)now, (unsigned long long)stick::clockMilliseconds(), (unsigned long)radio.writes, (unsigned long)stats.receivedMesh, (unsigned long)stats.receivedDm, unsigned(radio.pending), (unsigned long)radio.notified, (unsigned long)radio.notifyFailed, stick::mesh().lastReject(), stick::mesh().sendStatus());
  }
  delay(10);
}
