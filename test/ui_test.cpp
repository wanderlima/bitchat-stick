#include "../src/Ui.cpp"
#include <cassert>
#include <Preferences.h>
#include "PacketSecurity.h"
#include <cstdio>
#include <cstring>
namespace stick {
bool active=true, faulted=false;
bool transportReady() { return active && !faulted; }
size_t transportLinks() { return 0; }
uint32_t transportDrops() { return 0; }
const char* transportStatus() { return "Advertising"; }
bool clockRtc() { return true; }
bool scannerActive() { return active; }
bool scannerFault() { return faulted; }
uint32_t drops=2, reports=123;
uint32_t scannerDrops() { return drops; }
uint32_t scannerReports() { return reports; }
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
  press(0);assert(screen==Screen::Dashboard && has("Nearby: 1") && has("P:0 New:0"));
  bool square=false;
  for(const auto& r:M5.Display.rects) if(r.x==6 && r.y==7 && r.w==5 && r.h==5 && r.color==green) square=true;
  assert(square);
  bool divider=false;
  for(const auto& r:M5.Display.rects)
    if(r.x==5 && r.y==158 && r.w==125 && r.h==1 && r.color==fg) divider=true;
  assert(divider); // Shared separators between Home menu items.
  press(0);assert(screen==Screen::Peers && has("Test phone") && has("-64 dBm") && has("Bitchat?") && has("Not verified") && !has("Mainnet?"));
  bool greenCard=false;
  for(const auto& r:M5.Display.rects) if(r.x==4 && r.w==3 && r.color==green) greenCard=true;
  assert(greenCard);
  press(0);assert(detail && has("AA:BB:CC") && has("DD:EE:FF"));
  press(1,true);assert(screen==Screen::Peers && !detail);
  press(1,true);assert(screen==Screen::Dashboard);
  press(1,true);assert(screen==Screen::Settings);
  press(0);assert(screen==Screen::Brightness && selection==2);
  press(2);press(0);assert(brightness==191 && brightnessIndex==3 && screen==Screen::Settings);
  assert(fake_preferences::writes==1);
  press(0);press(2);press(1,true);assert(screen==Screen::Settings && brightnessIndex==3); // cancel without applying
  assert(fake_preferences::writes==1);
  press(2);press(0);assert(screen==Screen::Battery && has("4000 mV"));
  press(1,true);press(2);press(0);assert(screen==Screen::About && has("Wander Lima") && !has("SOON"));
  press(1,true);press(2);press(0);assert(screen==Screen::Diagnostics && has("BLE 123") && has("Drop 2"));
  // Visit the remaining primary screens and detail states with the same controls.
  screen=Screen::Channels;draw(table,tick);check();assert(has("#bluetooth") && has("Mesh"));press(0);assert(screen==Screen::Messages);
  press(1,true);assert(screen==Screen::Channels);
  screen=Screen::Bot;selection=0;draw(table,tick);check();press(0);assert(detail);press(1,true);assert(!detail);
  // Verify all presets, header error/idle states, and low/unknown battery rendering.
  screen=Screen::Brightness;
  for(size_t i=0;i<5;++i) { selection=i;draw(table,tick);check(); }
  screen=Screen::Dashboard;active=false;draw(table,tick);check();assert(has("IDLE"));
  faulted=true;draw(table,tick);check();assert(has("ERR"));
  battery=-1;draw(table,tick);check();assert(has("--%"));
  battery=5;draw(table,tick);check();assert(has("5%"));
  // Nearby excludes ordinary BLE and never invents peers when no candidate is present.
  screen=Screen::Peers;detail=false;peerSelection=PeerSelection{};
  draw(PeerTable{},tick);check();assert(has("No Bitchat") && !has("Test phone"));
  PeerTable ordinary;obs.network=Network::Other;ordinary.observe(obs);
  draw(ordinary,tick);check();assert(has("No Bitchat") && !has("Test phone") && !has("Other BLE"));
  obs.network=Network::Mainnet;
  PeerTable full;
  for(unsigned i=0;i<kPeerCapacity;++i) {
    std::snprintf(obs.address,sizeof(obs.address),"AA:BB:CC:DD:EE:%02X",i);
    std::strcpy(obs.name,"A name that is too long");full.observe(obs);
  }
  peerSelection.move(-1,full);draw(full,tick);check();assert(has("Device 32/32"));
  screen=Screen::Settings;draw(table,tick);check();assert(has("Diagnostics"));
  screen=Screen::Bot;
  for(selection=0;selection<3;++selection) {detail=true;draw(table,tick);check();assert(has("Public reply"));}
  detail=false;selection=3;press(0);assert(!mesh().botEnabled());press(0);assert(mesh().botEnabled());
  // Applied brightness and focus are distinct; inspecting a row never changes PWM.
  const auto applied=brightnessIndex;const auto pwm=M5.Display.brightness;
  screen=Screen::Brightness;brightnessIndex=2;selection=3;draw(table,tick);check();
  auto pixel=[](int x,int y) {
    uint16_t color=0;
    for(const auto& r:M5.Display.rects) if(x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h) color=r.color;
    return color;
  };
  assert(pixel(18,136)==fg); // Filled radio on the applied 50% row, outside focus.
  assert(pixel(18,167)==fg); // Unfilled center retains the focused 75% cream background.
  assert(M5.Display.brightness==pwm);brightnessIndex=applied;
  screen=Screen::Diagnostics;reports=0xffffffff;drops=0xffffffff;draw(table,0xffffffff);check();
  fake_preferences::writeSucceeds=false;
  screen=Screen::Brightness;selection=4;press(0);
  assert(!settingsSaved && has("Not saved") && brightness==255);
  fake_preferences::writeSucceeds=true;
  screen=Screen::Brightness;selection=4;press(0);
  assert(settingsSaved && !has("Not saved") && fake_preferences::record==0xBC510104);
  // Exercise live signed peer/message rendering, long-text scrolling and back behavior.
  Identity identity;
  auto entropy=[](void* out,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(out)[i]=uint8_t(i+41);};
  assert(identity.begin(entropy));
  mesh().begin(identity,entropy,[](wire::Bytes,uint16_t,uint16_t){return true;});
  mesh().tick(tick,1800000000000ULL);
  fake_preferences::identityExists=false;fake_preferences::identityLength=0;
  Identity other;auto otherEntropy=[](void* out,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(out)[i]=uint8_t(i+72);};assert(other.begin(otherEntropy));
  uint8_t packet[4096],scratch[4096],signature[64],announcement[80]={1,4,'L','i','v','e',2,32};
  std::memcpy(announcement+8,other.dhPublic(),32);announcement[40]=3;announcement[41]=32;std::memcpy(announcement+42,other.signingPublic(),32);
  auto inject=[&](uint8_t type,wire::Bytes payload){wire::FrameView f;f.type=type;f.ttl=7;f.timestamp=1800000000000ULL;f.sender={other.id(),8};f.payload=payload;
    assert(wire::signPacket(f,other.signingSecret(),scratch,sizeof(scratch),signature));f.signature={signature,64};size_t n=0;assert(wire::encode(f,packet,sizeof(packet),n)==wire::Error::None);mesh().receive({packet,n},1);};
  inject(1,{announcement,74});screen=Screen::Peers;selection=0;detail=false;draw(table,tick);check();assert(has("Live")&&has("Signed only"));
  uint8_t longText[192];std::memset(longText,'x',sizeof(longText));inject(2,{longText,sizeof(longText)});
  screen=Screen::Dashboard;draw(table,tick);check();assert(mesh().unreadCount()==1&&has("P:1 New:1"));
  auto unreadIcon=[](){for(const auto& r:M5.Display.rects)if(r.x==56&&r.y==6&&r.w==10&&r.color==green)return true;return false;};
  assert(unreadIcon());battery=100;draw(table,tick);check();assert(has("100%")&&unreadIcon());
  const Screen notificationScreens[]={Screen::Splash,Screen::Dashboard,Screen::Peers,Screen::Channels,Screen::Messages,Screen::Bot,Screen::Settings,Screen::Brightness,Screen::Battery,Screen::About,Screen::Diagnostics};
  messageScope=MessageScope::Direct; // Viewing an empty other inbox must not read the public message.
  for(auto page:notificationScreens){screen=page;selection=0;detail=false;draw(table,tick);check();assert(unreadIcon()&&mesh().unreadCount()==1);}

  screen=Screen::Channels;selection=1;detail=false;draw(table,tick);check();assert(has("Mesh")&&has("Direct DMs"));
  press(0);assert(screen==Screen::Messages&&messageScope==MessageScope::Direct&&has("No messages")&&mesh().unreadCount()==1);
  press(1,true);assert(screen==Screen::Channels&&selection==1);press(1);assert(selection==0);press(0);
  assert(messageScope==MessageScope::Public&&has("Live")&&mesh().unreadCount()==0&&!unreadIcon());
  const uint32_t reading=activeMessage;longText[0]='y';inject(2,{longText,sizeof(longText)});draw(table,tick);check();
  assert(activeMessage==reading&&mesh().unreadCount()==1&&messageOffset==1); // Incoming text never steals the reader's focus.
  press(2);assert(activeMessage!=reading&&mesh().unreadCount()==0);
  press(0);assert(detail);press(2);assert(textLine==1);press(1,true);assert(!detail&&screen==Screen::Messages);press(1,true);assert(screen==Screen::Channels);
  assert(buffered && M5.Display.clears==0 && M5.Display.frames>10);
  std::puts("UI: navigation, presets, credits, live header/data, grouping and text bounds passed");
}
