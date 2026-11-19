#include <M5Unified.h>
#include "DrawSurface.h"
#include "DisplayProfile.h"
#include "Board.h"
#include "SurfaceProbe.h"
#include "assets/UiFonts.h"
#include <string>

namespace stick {
namespace {
SurfaceProbe presented;
std::vector<SurfaceText> texts;
std::vector<SurfaceRect> rects;
std::vector<std::string> ops;
bool boundsOk = true;
UiFont fontId = UiFont::Vt16;
int fontKind = 16;
int fontHeight = 16;
float textScale = 1;
uint16_t ink = 0xFFFF;
bool buttonA = false, buttonB = false, buttonPower = false;

const lgfx::GFXfont* gfxFor(UiFont font) {
  switch (font) {
    case UiFont::Vt16: return &assets::Vt32316;
    case UiFont::Vt17: return &assets::Vt32317;
    case UiFont::Vt18: return &assets::Vt32318;
    case UiFont::Vt19: return &assets::Vt32319;
    case UiFont::Vt20: return &assets::Vt32320;
    case UiFont::Vt21: return &assets::Vt32321;
    case UiFont::Vt22: return &assets::Vt32322;
    case UiFont::Vt23: return &assets::Vt32323;
    case UiFont::Vt31: return &assets::Vt32331;
  }
  return &assets::Vt32316;
}

int fontPixels(UiFont font) {
  switch (font) {
    case UiFont::Vt16: return 16;
    case UiFont::Vt17: return 17;
    case UiFont::Vt18: return 18;
    case UiFont::Vt19: return 19;
    case UiFont::Vt20: return 20;
    case UiFont::Vt21: return 21;
    case UiFont::Vt22: return 22;
    case UiFont::Vt23: return 23;
    case UiFont::Vt31: return 31;
  }
  return 16;
}

void rememberRect(int x, int y, int width, int height, uint16_t color) {
  rects.push_back({x, y, width, height, color});
  ops.push_back("F " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(width) + " " +
                std::to_string(height) + " " + std::to_string(color));
  if (x < 0 || y < 0 || x + width > kDisplayProfile.width || y + height > kDisplayProfile.height) boundsOk = false;
}
}

bool surfaceBegin() {
  textScale = 1;
  frameSetFont(UiFont::Vt16);
  return true;
}

void displaySetBrightness(uint8_t brightness) { presented.brightness = brightness; }

void frameStart() {}

void frameFill(uint16_t color) {
  texts.clear();
  rects.clear();
  ops.clear();
  boundsOk = true;
  frameFillRect(0, 0, kDisplayProfile.width, kDisplayProfile.height, color);
}

void frameFillRect(int x, int y, int width, int height, uint16_t color) {
  rememberRect(x, y, width, height, color);
}

void frameDrawRect(int x, int y, int width, int height, uint16_t color) {
  frameFillRect(x, y, width, 1, color);
  frameFillRect(x, y + height - 1, width, 1, color);
  frameFillRect(x, y, 1, height, color);
  frameFillRect(x + width - 1, y, 1, height, color);
}

void frameHLine(int x, int y, int width, uint16_t color) { frameFillRect(x, y, width, 1, color); }

void frameSetFont(UiFont font) {
  fontId = font;
  fontKind = fontPixels(font);
  fontHeight = fontKind;
}

int frameTextWidth(const char* text) {
  const lgfx::GFXfont* gfx = gfxFor(fontId);
  int width = 0;
  for (const unsigned char c : std::string(text ? text : "")) {
    if (c >= 32 && c <= 126) width += gfx->glyph[c - 32].xAdvance;
    else width += fontHeight;
  }
  return int(width * textScale);
}

void frameSetColor(uint16_t color) { ink = color; }

void frameDrawText(int x, int y, const char* text) {
  const char* value = text ? text : "";
  texts.push_back({value, x, y});
  ops.push_back("T " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(fontKind) + " " +
                std::to_string(ink) + " " + std::to_string(textScale) + " " + value);
  if (x < 0 || y < 0 || x + frameTextWidth(value) > kDisplayProfile.width ||
      y + fontHeight * textScale > kDisplayProfile.height) boundsOk = false;
}

void frameFinish() {
  presented.texts = texts;
  presented.rects = rects;
  presented.ops = ops;
  presented.boundsOk = boundsOk;
  ++presented.frames;
}

SurfaceProbe& surfaceProbe() { return presented; }

void testSetButton(int key, bool down) {
  if (key == 0) buttonA = down;
  else if (key == 1) buttonB = down;
  else buttonPower = down;
}

void testSetBrightness(uint8_t brightness) { presented.brightness = brightness; }

BoardButtons boardButtons() { return {buttonA, buttonB, buttonPower}; }

void boardBattery(int& voltageMv, int& percent) {
  voltageMv = 4000;
  percent = 74;
}

uint32_t boardFreeHeap() { return 123456; }
}
