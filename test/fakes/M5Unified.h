#pragma once
#include <cstdint>
#include <string>
#include <vector>
constexpr uint16_t TFT_BLACK=0, TFT_WHITE=0xffff, TFT_YELLOW=0xffe0, TFT_RED=0xf800;
namespace lgfx {
struct IFont { int height; int kind; };
struct GFXglyph { uint32_t bitmapOffset; uint8_t width,height,xAdvance; int8_t xOffset,yOffset; };
struct GFXfont : IFont {
  uint8_t* bitmap; GFXglyph* glyph; uint16_t first,last;
  constexpr GFXfont(uint8_t* b,GFXglyph* g,uint16_t f,uint16_t l,uint8_t h)
      : IFont{h,h==14?3:4},bitmap(b),glyph(g),first(f),last(l) {}
};
}
namespace fonts {
inline const lgfx::IFont FreeMono9pt7b{18,0}, FreeMonoBold9pt7b{18,1}, FreeMonoBold12pt7b{24,2};
}
// Bold-font advance metrics from pinned M5GFX 0.3.0; not a pixel renderer.
inline const unsigned char bold9[] = {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};
inline const unsigned char bold12[] = {14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14};
inline const unsigned char mono9[] = {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};
struct TextDraw { std::string text; int x,y; };
struct RectDraw { int x,y,w,h; uint16_t color; };
struct FakeDisplay {
  const lgfx::IFont* font=&fonts::FreeMono9pt7b;
  std::vector<TextDraw> texts;
  std::vector<RectDraw> rects;
  std::vector<std::string> ops;
  uint16_t ink=0xffff;
  int x=0,y=0;
  float textScale=1;
  bool boundsOk=true;
  unsigned clears=0,frames=0;
  uint8_t brightness=0;
  void setFont(const lgfx::IFont* f) { font=f; }
  void setTextSize(float value) { textScale=value; }
  void setTextColor(uint16_t c) { ink=c; }
  void setTextWrap(bool) {}
  void setRotation(int) {}
  void setBrightness(uint8_t b) { brightness=b; }
  void setCursor(int a,int b) { x=a;y=b; }
  int textWidth(const char* s) {
    int n=0;
    for (const unsigned char c:std::string(s)) {
      if (font->kind>=3 && c>=32 && c<127) n+=static_cast<const lgfx::GFXfont*>(font)->glyph[c-32].xAdvance;
      else if (font->kind && c>=32 && c<127) n+=(font->kind==1?bold9:bold12)[c-32];
      else if(c>=32 && c<128) n+=mono9[c-32];
      else n+=8;
    }
    return int(n * textScale);
  }
  void print(const char* s) {
    texts.push_back({s,x,y});
    ops.push_back("T "+std::to_string(x)+" "+std::to_string(y)+" "+std::to_string(font->kind)+" "+std::to_string(ink)+" "+std::to_string(textScale)+" "+s);
    if (x<0 || y<0 || x+textWidth(s)>135 || y+font->height*textScale>240) boundsOk=false;
  }
  void fillScreen(uint16_t color) { ++clears; texts.clear();rects.clear();ops.clear();boundsOk=true;fillRect(0,0,135,240,color); }
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
namespace lgfx {using LGFXBase=FakeDisplay;}
struct M5Canvas:FakeDisplay {
 FakeDisplay* parent;explicit M5Canvas(FakeDisplay* value):parent(value){}
 void setColorDepth(int){}
 void* createSprite(int,int){return this;}
 void pushSprite(int,int){parent->texts=texts;parent->rects=rects;parent->ops=ops;parent->boundsOk=boundsOk;++parent->frames;}
};
struct FakeButton { bool down=false; bool isPressed() const { return down; } };
struct FakePower { int getBatteryVoltage() { return 4000; } int getBatteryLevel() { return 74; } };
struct FakeM5 { FakeDisplay Display; FakeButton BtnA,BtnB,BtnPWR; FakePower Power; };
inline FakeM5 M5;
struct FakeESP { unsigned getFreeHeap() { return 123456; } };
inline FakeESP ESP;
inline uint32_t millis() { return 0; }
