#include "DrawSurface.h"
#include "DisplayProfile.h"
#include <M5Unified.h>
#include "assets/UiFonts.h"

namespace stick {
namespace {
M5Canvas canvas(&M5.Display);
bool buffered = false;
lgfx::LGFXBase* target = nullptr;
uint16_t ink = 0xFFFF;

const lgfx::IFont* fontFor(UiFont font) {
  switch (font) {
    case UiFont::Compact14: return &assets::FreeMonoCompact14;
    case UiFont::Bold9: return &fonts::FreeMonoBold9pt7b;
    case UiFont::Bold12: return &fonts::FreeMonoBold12pt7b;
    case UiFont::Body16: return &assets::FreeMonoBody16;
  }
  return &assets::FreeMonoBody16;
}

lgfx::LGFXBase& out() { return *target; }
}

bool surfaceBegin() {
  const DisplayProfile& profile = displayProfile();
  M5.Display.setRotation(profile.rotation);
  canvas.setColorDepth(profile.colorDepth);
  buffered = canvas.createSprite(profile.width, profile.height) != nullptr;
  target = buffered ? static_cast<lgfx::LGFXBase*>(&canvas) : static_cast<lgfx::LGFXBase*>(&M5.Display);
  out().setTextSize(1);
  out().setTextWrap(false);
  out().setFont(fontFor(UiFont::Body16));
  return buffered;
}

void displaySetBrightness(uint8_t brightness) { M5.Display.setBrightness(brightness); }

void frameStart() { out().startWrite(); }

void frameFill(uint16_t color) { out().fillScreen(color); }

void frameFillRect(int x, int y, int width, int height, uint16_t color) {
  out().fillRect(x, y, width, height, color);
}

void frameDrawRect(int x, int y, int width, int height, uint16_t color) {
  out().drawRect(x, y, width, height, color);
}

void frameHLine(int x, int y, int width, uint16_t color) {
  out().drawFastHLine(x, y, width, color);
}

void frameSetFont(UiFont font) { out().setFont(fontFor(font)); }

int frameTextWidth(const char* text) { return out().textWidth(text); }

void frameSetColor(uint16_t color) { ink = color; }

void frameDrawText(int x, int y, const char* text) {
  out().setTextColor(ink);
  out().setCursor(x, y);
  out().print(text);
}

void frameFinish() {
  out().endWrite();
  if (buffered) canvas.pushSprite(0, 0);
}
}
