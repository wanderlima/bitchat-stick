#pragma once
#include <cstdint>
#include <string>
#include <vector>
constexpr uint16_t TFT_BLACK=0, TFT_WHITE=0xffff, TFT_YELLOW=0xffe0, TFT_RED=0xf800;
namespace lgfx { struct IFont { int height; int kind; }; }
namespace fonts {
inline const lgfx::IFont Font2{16,0}, FreeSansBold9pt7b{22,1}, FreeSansBold12pt7b{29,2};
}
// Bold-font advance metrics from pinned M5GFX 0.3.0; not a pixel renderer.
inline const unsigned char bold9[] = {5,6,9,10,10,16,13,5,6,6,7,11,4,6,4,5,10,10,10,10,10,10,10,10,10,10,4,4,11,11,11,11,18,13,13,13,13,12,11,14,13,6,10,13,11,16,14,14,12,14,13,12,12,13,12,17,12,12,11,6,5,6,11,10,5,10,11,10,11,10,6,11,11,5,5,10,5,16,11,11,11,11,7,10,6,11,10,14,10,10,9,7,5,7,9};
inline const unsigned char bold12[] = {7,8,11,13,13,21,17,6,8,8,9,14,6,8,6,7,13,14,13,13,13,13,13,13,13,13,6,6,14,14,14,15,23,17,17,17,17,16,15,18,18,7,14,17,15,21,18,19,16,19,17,16,15,18,16,23,16,15,15,8,7,8,14,13,6,14,15,13,15,14,8,15,14,7,7,14,6,21,15,15,15,15,9,13,8,15,13,19,13,13,12,9,7,9,12};
inline const unsigned char compact16[] = {6,3,4,9,8,9,9,3,7,7,8,6,3,6,5,7,8,8,8,8,8,8,8,8,8,8,3,3,6,6,6,8,9,8,8,8,8,8,8,8,8,4,8,8,7,10,8,8,8,8,8,8,8,8,8,10,8,8,8,4,7,4,7,9,5,7,7,7,7,7,6,7,7,4,5,6,4,8,7,8,7,8,6,6,5,7,8,8,6,7,7,5,3,5,8,6};
struct TextDraw { std::string text; int x,y; };
struct RectDraw { int x,y,w,h; uint16_t color; };
struct FakeDisplay {
  const lgfx::IFont* font=&fonts::Font2;
  std::vector<TextDraw> texts;
  std::vector<RectDraw> rects;
  std::vector<std::string> ops;
  uint16_t ink=0xffff;
  int x=0,y=0;
  bool boundsOk=true;
  uint8_t brightness=0;
  void setFont(const lgfx::IFont* f) { font=f; }
  void setTextSize(int) {}
  void setTextColor(uint16_t c) { ink=c; }
  void setTextWrap(bool) {}
  void setRotation(int) {}
  void setBrightness(uint8_t b) { brightness=b; }
  void setCursor(int a,int b) { x=a;y=b; }
  int textWidth(const char* s) {
    int n=0;
    for (const unsigned char c:std::string(s)) {
      if (font->kind && c>=32 && c<127) n+=(font->kind==1?bold9:bold12)[c-32];
      else if(c>=32 && c<128) n+=compact16[c-32];
      else n+=8;
    }
    return n;
  }
  void print(const char* s) {
    texts.push_back({s,x,y});
    ops.push_back("T "+std::to_string(x)+" "+std::to_string(y)+" "+std::to_string(font->kind)+" "+std::to_string(ink)+" "+s);
    if (x<0 || y<0 || x+textWidth(s)>135 || y+font->height>240) boundsOk=false;
  }
  void fillScreen(uint16_t color) { texts.clear();rects.clear();ops.clear();boundsOk=true;fillRect(0,0,135,240,color); }
  void startWrite() {}
  void endWrite() {}
  void fillRect(int a,int b,int w,int h,uint16_t c) {
    rects.push_back({a,b,w,h,c});
    ops.push_back("F "+std::to_string(a)+" "+std::to_string(b)+" "+std::to_string(w)+" "+std::to_string(h)+" "+std::to_string(c));
    if(a<0 || b<0 || a+w>135 || b+h>240) boundsOk=false;
  }
  void drawRect(int a,int b,int w,int h,uint16_t c) {
    fillRect(a,b,w,1,c);fillRect(a,b+h-1,w,1,c);fillRect(a,b,1,h,c);fillRect(a+w-1,b,1,h,c);
  }
  void drawFastHLine(int a,int b,int w,uint16_t c) { fillRect(a,b,w,1,c); }
};
struct FakeButton { bool down=false; bool isPressed() const { return down; } };
struct FakePower { int getBatteryVoltage() { return 4000; } int getBatteryLevel() { return 74; } };
struct FakeM5 { FakeDisplay Display; FakeButton BtnA,BtnB,BtnPWR; FakePower Power; };
inline FakeM5 M5;
struct FakeESP { unsigned getFreeHeap() { return 123456; } };
inline FakeESP ESP;
inline uint32_t millis() { return 0; }
