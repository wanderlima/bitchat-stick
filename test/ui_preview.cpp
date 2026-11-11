#include "../src/Ui.cpp"
#include <cassert>
#include <fstream>
#include <Preferences.h>
#include "PacketSecurity.h"
namespace stick {
JoycSample fakeJoyc;
JoycSample pollJoyc(uint32_t, bool) { return fakeJoyc; }
bool setJoycLed(uint8_t) { return true; }
bool transportReady(){return true;}
size_t transportLinks(){return 1;}
uint32_t transportDrops(){return 0;}
const char* transportStatus(){return "Connected";}
bool clockRtc(){return true;}
bool scannerActive() { return true; }
bool scannerFault() { return false; }
uint32_t scannerDrops() { return 2; }
uint32_t scannerReports() { return 123; }
}
int main(int argc, char** argv) {
  assert(argc == 2);
  using namespace stick;
  PeerTable peers;
  Observation value{};
  std::strcpy(value.address,"AA:BB:CC:DD:EE:FF");std::strcpy(value.name,"Test phone");
  value.rssi=-64;value.network=Network::Mainnet;value.seenAt=1000;peers.observe(value);
  Identity identity;
  auto entropy=[](void* out,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(out)[i]=uint8_t(i+17);};
  assert(identity.begin(entropy));mesh().begin(identity,entropy,[](wire::Bytes,uint16_t,uint16_t){return true;});
  mesh().tick(3000,1800000000000ULL);assert(mesh().sendText("Hello from the Stick"));
  // Synthetic incoming public traffic exercises the unread envelope and real peer grouping.
  fake_preferences::identityExists=false;fake_preferences::identityLength=0;
  Identity other;auto otherEntropy=[](void* out,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(out)[i]=uint8_t(i+59);};assert(other.begin(otherEntropy));
  uint8_t announcement[90]={1,10,'T','e','s','t',' ','p','h','o','n','e',2,32};
  std::memcpy(announcement+14,other.dhPublic(),32);announcement[46]=3;announcement[47]=32;std::memcpy(announcement+48,other.signingPublic(),32);
  auto incoming=[&](uint8_t type,wire::Bytes payload){uint8_t encoded[4096],scratch[4096],sig[64];size_t n=0;wire::FrameView f;f.type=type;f.ttl=7;f.timestamp=1800000000000ULL;f.sender={other.id(),8};f.payload=payload;
    assert(wire::signPacket(f,other.signingSecret(),scratch,sizeof(scratch),sig));f.signature={sig,64};assert(wire::encode(f,encoded,sizeof(encoded),n)==wire::Error::None);mesh().receive({encoded,n},1);};
  incoming(1,{announcement,80});const char* text="Hello from a phone";incoming(2,{reinterpret_cast<const uint8_t*>(text),std::strlen(text)});
  beginUi();battery=72;voltage=4000;
  const Screen pages[]={Screen::Dashboard,Screen::Peers,Screen::Channels,Screen::Messages,Screen::Bot,Screen::Settings,Screen::Splash,Screen::Brightness,Screen::About,Screen::Battery,Screen::Diagnostics,Screen::Sound,Screen::Joystick};
  const char* names[]={"01-dashboard","02-nearby-peers","03-channels","04-channel-messages","05-infobot","06-settings","07-splash","08-brightness","09-about","10-battery","11-diagnostics","12-sound","13-joyc-hat"};
  for(unsigned i=0;i<13;++i) {
    screen=pages[i];selection=pages[i]==Screen::Brightness ? 2 : 0;detail=false;draw(peers,3000);
    assert(M5.Display.boundsOk);
    std::ofstream out((std::string(argv[1])+"/")+names[i]+".ops");
    for(const auto& op:M5.Display.ops) out<<op<<'\n';
  }
}
