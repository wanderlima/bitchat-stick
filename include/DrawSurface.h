#pragma once
#include <stdint.h>

namespace stick {
enum class UiFont { Body16, Compact14, Bold9, Bold12 };

bool surfaceBegin();
void displaySetBrightness(uint8_t brightness);
void frameStart();
void frameFill(uint16_t color);
void frameFillRect(int x, int y, int width, int height, uint16_t color);
void frameDrawRect(int x, int y, int width, int height, uint16_t color);
void frameHLine(int x, int y, int width, uint16_t color);
void frameSetFont(UiFont font);
int frameTextWidth(const char* text);
void frameSetColor(uint16_t color);
void frameDrawText(int x, int y, const char* text);
void frameFinish();
}
