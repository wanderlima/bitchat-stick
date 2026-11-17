#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace stick {
struct SurfaceText { std::string text; int x, y; };
struct SurfaceRect { int x, y, w, h; uint16_t color; };
struct SurfaceProbe {
  std::vector<SurfaceText> texts;
  std::vector<SurfaceRect> rects;
  std::vector<std::string> ops;
  bool boundsOk = true;
  unsigned clears = 0;
  unsigned frames = 0;
  uint8_t brightness = 0;
};
SurfaceProbe& surfaceProbe();
void testSetButton(int key, bool down);
void testSetBrightness(uint8_t brightness);
}
