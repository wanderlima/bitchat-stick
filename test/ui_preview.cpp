#include "../src/Ui.cpp"
#include <cassert>
#include <fstream>
namespace stick {
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
  beginUi();battery=72;voltage=4000;
  const Screen pages[]={Screen::Dashboard,Screen::Peers,Screen::Channels,Screen::Messages,Screen::Bot,Screen::Settings,Screen::Splash,Screen::Brightness,Screen::About,Screen::Battery,Screen::Diagnostics};
  const char* names[]={"01-dashboard","02-nearby-peers","03-channels","04-channel-messages","05-infobot","06-settings","07-splash","08-brightness","09-about","10-battery","11-diagnostics"};
  for(unsigned i=0;i<11;++i) {
    screen=pages[i];selection=pages[i]==Screen::Brightness ? 2 : 0;detail=false;draw(peers,3000);
    assert(M5.Display.boundsOk);
    std::ofstream out((std::string(argv[1])+"/")+names[i]+".ops");
    for(const auto& op:M5.Display.ops) out<<op<<'\n';
  }
}
