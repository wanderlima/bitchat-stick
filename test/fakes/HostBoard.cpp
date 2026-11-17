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
UiFont fontId = UiFont::Body16;
int fontKind = 4;
int fontHeight = 16;
float textScale = 1;
uint16_t ink = 0xFFFF;
bool buttonA = false, buttonB = false, buttonPower = false;

const unsigned char bold9[] = {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};
const unsigned char bold12[] = {14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14};
const unsigned char mono9[] = {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};

void rememberRect(int x, int y, int width, int height, uint16_t color) {
  rects.push_back({x, y, width, height, color});
  ops.push_back("F " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(width) + " " +
                std::to_string(height) + " " + std::to_string(color));
  if (x < 0 || y < 0 || x + width > kDisplayProfile.width || y + height > kDisplayProfile.height) boundsOk = false;
}
}

bool surfaceBegin() {
  textScale = 1;
  frameSetFont(UiFont::Body16);
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
  if (font == UiFont::Compact14) { fontKind = 3; fontHeight = 14; }
  else if (font == UiFont::Bold9) { fontKind = 1; fontHeight = 18; }
  else if (font == UiFont::Bold12) { fontKind = 2; fontHeight = 24; }
  else { fontKind = 4; fontHeight = 16; }
}

int frameTextWidth(const char* text) {
  const lgfx::GFXfont* gfx = fontId == UiFont::Compact14 ? &assets::FreeMonoCompact14
                            : fontId == UiFont::Body16 ? &assets::FreeMonoBody16 : nullptr;
  int width = 0;
  for (const unsigned char c : std::string(text ? text : "")) {
    if (gfx && c >= 32 && c < 127) width += gfx->glyph[c - 32].xAdvance;
    else if (fontKind && c >= 32 && c < 127) width += (fontKind == 1 ? bold9 : bold12)[c - 32];
    else if (c >= 32 && c < 128) width += mono9[c - 32];
    else width += 8;
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
