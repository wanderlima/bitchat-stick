#include "Mesh.h"
#include "PacketSecurity.h"
#include <Preferences.h>
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace stick;
static unsigned seed=11;static unsigned handshakes=0;static bool sendAllowed=true;
void entropy(void* data,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(data)[i]=uint8_t(i+seed);}
bool send(wire::Bytes bytes,uint16_t,uint16_t){if(!sendAllowed)return false;wire::FrameView frame;assert(wire::decode(bytes,frame)==wire::Error::None);if(frame.type==0x10&&frame.payload.size==32)++handshakes;return true;}
int main(){
  Identity local,phone;assert(local.begin(entropy));fake_preferences::identityExists=false;fake_preferences::identityLength=0;++seed;assert(phone.begin(entropy));
  static Mesh model;model.begin(local,entropy,send);constexpr uint64_t epoch=1800000000000ULL;model.tick(100,epoch);
  uint8_t announcement[80]={1,4,'T','e','s','t',2,32};std::memcpy(announcement+8,phone.dhPublic(),32);announcement[40]=3;announcement[41]=32;std::memcpy(announcement+42,phone.signingPublic(),32);
  uint64_t stamp=epoch;
  auto inject=[&](uint8_t type,wire::Bytes payload,bool corrupt=false){uint8_t out[4096],scratch[4096],sig[64];wire::FrameView f;f.type=type;f.ttl=7;f.sender={phone.id(),8};f.timestamp=++stamp;f.payload=payload;assert(wire::signPacket(f,phone.signingSecret(),scratch,sizeof(scratch),sig));if(corrupt)sig[0]^=1;f.signature={sig,64};size_t n=0;assert(wire::encode(f,out,sizeof(out),n)==wire::Error::None);model.receive({out,n},1);};
  auto command=[&](const char* text,bool corrupt=false){inject(2,{reinterpret_cast<const uint8_t*>(text),std::strlen(text)},corrupt);};
  inject(1,{announcement,74});assert(model.authenticatedCount()==0);
  command("!info");assert(model.stats().botReplies==1); // A signed public command needs no private session.
  assert(model.message(0)&&model.message(0)->outgoing);
  assert(std::strcmp(model.message(0)->text,"InfoBot: Bitchat Stick | ESP32 | #bluetooth local relay and private text DMs.")==0);
  assert(!std::strstr(model.message(0)->text,"M5StickC"));
  assert(std::strcmp(model.botStatus(),"Public reply")==0);
  command("!help");assert(model.stats().botReplies==1&&std::strcmp(model.botStatus(),"Wait 3s")==0);
  model.tick(3100,epoch+3000);stamp=epoch+3000;command("  !ABOUT\n");assert(model.stats().botReplies==2);
  assert(std::strstr(model.message(0)->text,"https://github.com/wanderlima/bitchat-stick"));
  model.tick(6100,epoch+6000);stamp=epoch+6000;command("!regras");assert(model.stats().botReplies==3);
  model.tick(9100,epoch+9000);stamp=epoch+9000;command("!ajuda");assert(model.stats().botReplies==4);
  model.setBotEnabled(false);command("!info");assert(model.stats().botReplies==4&&std::strcmp(model.botStatus(),"Bot OFF")==0);
  model.setBotEnabled(true);model.tick(12100,epoch+12000);stamp=epoch+12000;command("!info",true);assert(model.stats().botReplies==4);
  command("please !info");assert(model.stats().botReplies==4);
  model.tick(200000,epoch+200000);stamp=epoch+12000;command("!info");assert(model.stats().botReplies==4&&std::strcmp(model.botStatus(),"Check clock")==0);
  stamp=epoch+200000;command("!info");assert(model.stats().botReplies==5);
  model.tick(203100,epoch+203100);stamp=epoch+203100;sendAllowed=false;command("!help");
  assert(model.stats().botReplies==5&&std::strcmp(model.botStatus(),"Queue busy")==0);
  sendAllowed=true;command("!help");assert(model.stats().botReplies==6);
  // Discovery can contain more identities than physical BLE links. A third identity
  // must not be permanently excluded by the old two-session pool.
  const auto beforeSessions=handshakes;
  for(unsigned i=0;i<3;++i){Identity extra;fake_preferences::identityExists=false;fake_preferences::identityLength=0;++seed;assert(extra.begin(entropy));
    uint8_t payload[80]={1,4,'T','e','s','t',2,32},out[4096],scratch[4096],sig[64];std::memcpy(payload+8,extra.dhPublic(),32);payload[40]=3;payload[41]=32;std::memcpy(payload+42,extra.signingPublic(),32);
    wire::FrameView frame;frame.type=1;frame.ttl=7;frame.sender={extra.id(),8};frame.timestamp=epoch+203100+i;frame.payload={payload,74};assert(wire::signPacket(frame,extra.signingSecret(),scratch,sizeof(scratch),sig));frame.signature={sig,64};size_t n=0;assert(wire::encode(frame,out,sizeof(out),n)==wire::Error::None);model.receive({out,n},1);
    assert(!model.sendDirect(extra.id(),"hi")&&std::strcmp(model.sendStatus(),"Connecting")==0);
  }
  assert(handshakes==beforeSessions+3);
  assert(model.stats().botIntroductions==0); // Public chat never triggers a private introduction.
  std::puts("InfoBot: signed public commands without a private session passed");
}
