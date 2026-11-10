#include "../src/Ui.cpp"
#include <cassert>
#include <cstdio>
#include <cstring>
namespace stick {
bool active=true, faulted=false;
bool scannerActive() { return active; }
bool scannerFault() { return faulted; }
uint32_t scannerDrops() { return 2; }
uint32_t scannerReports() { return 123; }
}
using namespace stick;
uint32_t tick=100;
PeerTable table;
bool has(const char* text) {
  for (const auto& draw:M5.Display.texts) if(draw.text==text) return true;
  return false;
}
void check() { assert(M5.Display.boundsOk); }
void press(int key,bool hold=false) {
  auto& button=key==0?M5.BtnA:key==1?M5.BtnB:M5.BtnPWR;
  button.down=true;updateUi(table,tick);tick+=hold?700:60;
  if(hold) updateUi(table,tick);
  button.down=false;updateUi(table,++tick);tick+=30;check();
}
int main() {
  Observation obs{};
  std::strcpy(obs.address,"AA:BB:CC:DD:EE:FF");std::strcpy(obs.name,"Test phone");
  obs.rssi=-64;obs.network=Network::Mainnet;table.observe(obs);
  beginUi();check();assert(has("BITCHAT") && has("A: START"));
  press(0);assert(screen==Screen::Dashboard && has("BLE: 1") && has("Cand. 1"));
  bool square=false;
  for(const auto& r:M5.Display.rects) if(r.x==6 && r.y==7 && r.w==5 && r.h==5 && r.color==green) square=true;
  assert(square);
  press(0);assert(screen==Screen::Peers && has("Test phone") && has("-64 dBm"));
  bool greenCard=false;
  for(const auto& r:M5.Display.rects) if(r.x==4 && r.w==3 && r.color==green) greenCard=true;
  assert(greenCard);
  press(0);assert(detail && has("AA:BB:CC") && has("DD:EE:FF"));
  press(1,true);assert(screen==Screen::Peers && !detail);
  press(1,true);assert(screen==Screen::Dashboard);
  press(1,true);assert(screen==Screen::Settings);
  press(0);assert(screen==Screen::Brightness && selection==2);
  press(2);press(0);assert(brightness==191 && brightnessIndex==3 && screen==Screen::Settings);
  press(0);press(2);press(1,true);assert(screen==Screen::Settings && brightnessIndex==3); // cancel without applying
  press(2);press(0);assert(screen==Screen::Battery && has("4000 mV"));
  press(1,true);press(2);press(0);assert(screen==Screen::About && has("Wander Lima") && has("wanderlima") && has("QR"));
  press(1,true);press(2);press(0);assert(screen==Screen::Diagnostics && has("BLE 123") && has("Drop 2"));
  // Visit the remaining primary screens and detail states with the same controls.
  screen=Screen::Channels;draw(table,tick);check();press(0);assert(screen==Screen::Messages);
  press(1,true);assert(screen==Screen::Channels);
  screen=Screen::Bot;selection=0;draw(table,tick);check();press(0);assert(detail);press(1,true);assert(!detail);
  // Verify all presets, header error/idle states, and low/unknown battery rendering.
  screen=Screen::Brightness;
  for(size_t i=0;i<5;++i) { selection=i;draw(table,tick);check(); }
  screen=Screen::Dashboard;active=false;draw(table,tick);check();assert(has("BLE idle"));
  faulted=true;draw(table,tick);check();assert(has("BLE err"));
  battery=-1;draw(table,tick);check();assert(has("--%"));
  battery=5;draw(table,tick);check();assert(has("5%"));
  std::puts("UI: navigation, presets, credits, live header/data, grouping and text bounds passed");
}
