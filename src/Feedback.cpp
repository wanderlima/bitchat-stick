#include "Feedback.h"
#include "Settings.h"
#include "Joyc.h"
#include <M5Unified.h>

namespace stick {
namespace {
uint32_t lastDm = 0, lastMesh = 0, toneStarted = 0, pulseStarted = 0, lastLedUpdate = 0;
bool pendingDm = false, pendingMesh = false, toneActive = false, pulsing = false;
uint8_t pendingUi = 0, tonesRemaining = 0, soundCategory = 0, ledLevel = 0;
uint16_t toneDuration = 0, toneFrequency = 0;

void startTone(uint32_t now) {
  M5.Speaker.setVolume(64);
  M5.Speaker.tone(toneFrequency, toneDuration, 0, true);
  toneStarted = now;
  toneActive = true;
  --tonesRemaining;
}
void updateSound(uint32_t now, uint8_t flags) {
  if (!(flags & SoundUi)) pendingUi = 0;
  if (!(flags & SoundDm)) pendingDm = false;
  if (!(flags & SoundMesh)) pendingMesh = false;
  if (toneActive && !(flags & soundCategory)) {
    M5.Speaker.stop(0);
    toneActive = false;
    tonesRemaining = 0;
  }
  if (toneActive) {
    if (uint32_t(now - toneStarted) < uint32_t(toneDuration) + 50) return;
    if (tonesRemaining) {
      startTone(now);
      return;
    }
    toneActive = false;
  }
  if (pendingDm) {
    pendingDm = false;
    soundCategory = SoundDm;
    toneFrequency = 2400;
    toneDuration = 90;
    tonesRemaining = 1;
  } else if (pendingMesh) {
    pendingMesh = false;
    soundCategory = SoundMesh;
    toneFrequency = 1600;
    toneDuration = 65;
    tonesRemaining = 1;
  } else if (pendingUi) {
    soundCategory = SoundUi;
    toneFrequency = 2000;
    toneDuration = 30;
    tonesRemaining = pendingUi;
    pendingUi = 0;
  } else return;
  startTone(now);
}
void updateLed(uint32_t now, bool enabled, bool interacted) {
  if (!enabled) {
    pulsing = false;
    if (ledLevel) setJoycLed(0);
    ledLevel = 0;
    return;
  }
  if (interacted) {
    pulsing = true;
    pulseStarted = now;
  }
  if (!pulsing || uint32_t(now - lastLedUpdate) < 30) return;
  lastLedUpdate = now;
  const uint32_t elapsed = now - pulseStarted;
  uint8_t level = 0;
  if (elapsed < 60) level = uint8_t(elapsed * 48 / 60);
  else if (elapsed < 240) level = uint8_t((240 - elapsed) * 48 / 180);
  else pulsing = false;
  if (level != ledLevel) {
    if (!setJoycLed(level)) pulsing = false;
    ledLevel = level;
  }
}
}
void updateFeedback(uint32_t now, uint8_t flags, NavigationAction action,
                    uint32_t receivedDm, uint32_t receivedMesh) {
  if (receivedDm != lastDm && (flags & SoundDm)) pendingDm = true;
  if (receivedMesh != lastMesh && (flags & SoundMesh)) pendingMesh = true;
  lastDm = receivedDm;
  lastMesh = receivedMesh;
  const bool interacted = action != NavigationAction::None;
  if (interacted && (flags & SoundUi)) pendingUi = action == NavigationAction::Select ? 2 : 1;
  updateSound(now, flags);
  updateLed(now, flags & JoycLed, interacted);
}
}
