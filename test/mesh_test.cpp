#include "Mesh.h"
#include "PacketSecurity.h"
#include <Preferences.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>
using namespace stick;
static uint8_t generation=1;static std::vector<std::vector<uint8_t>> sent;
void entropy(void* p,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(p)[i]=uint8_t(i+generation);}
bool send(wire::Bytes b,uint16_t,uint16_t){sent.emplace_back(b.data,b.data+b.size);return true;}
std::vector<uint8_t> packet(Identity& who,uint8_t type,wire::Bytes payload,uint64_t stamp=1800000000000ULL,wire::Bytes recipient={},bool sign=true){
 uint8_t scratch[4096],sig[64],out[4096];size_t n=0;wire::FrameView f;f.type=type;f.ttl=7;f.sender={who.id(),8};f.recipient=recipient;f.payload=payload;f.timestamp=stamp;
 if(sign){assert(wire::signPacket(f,who.signingSecret(),scratch,sizeof(scratch),sig));f.signature={sig,64};}
 assert(wire::encode(f,out,sizeof(out),n)==wire::Error::None);return {out,out+n};
}
void receive(Mesh& m,const std::vector<uint8_t>& b){m.receive({b.data(),b.size()},1);}
int main(){
 Identity local,remote;assert(local.begin(entropy));fake_preferences::identityExists=false;fake_preferences::identityLength=0;++generation;assert(remote.begin(entropy));
 static Mesh m;m.begin(local,entropy,send);m.tick(100,1800000000000ULL);
 uint8_t announce[80]={1,4,'T','e','s','t',2,32};std::memcpy(announce+8,remote.dhPublic(),32);announce[40]=3;announce[41]=32;std::memcpy(announce+42,remote.signingPublic(),32);
 auto a=packet(remote,1,{announce,74});receive(m,a);assert(m.peerCount()==1&&m.authenticatedCount()==0);
 auto text=packet(remote,2,{reinterpret_cast<const uint8_t*>("hello"),5});receive(m,text);assert(m.messageCount()==1);receive(m,text);assert(m.messageCount()==1);
 text.back()^=1;auto rejected=m.stats().rejected;receive(m,text);assert(m.stats().rejected>rejected);
 NoiseSession phone;uint8_t ephemeral[32],handshake[128],response[128];entropy(ephemeral,32);size_t n=0,r=0;
 wire::FrameView reply;
 // The first announcement automatically initiates Noise, before relay emits the announcement.
 const std::vector<uint8_t>* initiation=nullptr;
 for(const auto& raw:sent){assert(wire::decode({raw.data(),raw.size()},reply)==wire::Error::None);if(reply.type==0x10 && reply.payload.size==32)initiation=&raw;}
 assert(initiation);assert(wire::decode({initiation->data(),initiation->size()},reply)==wire::Error::None);
 assert(phone.begin(false,remote.dhSecret(),ephemeral,local.id(),handshake,sizeof(handshake),n));
 assert(phone.receive(reply.payload,response,sizeof(response),r));
 receive(m,packet(remote,0x10,{response,r},1800000000000ULL,{local.id(),8},false));
 const std::vector<uint8_t>* finalHandshake=nullptr;
 for(const auto& raw:sent){assert(wire::decode({raw.data(),raw.size()},reply)==wire::Error::None);if(reply.type==0x10 && reply.payload.size==64)finalHandshake=&raw;}
 assert(finalHandshake);assert(wire::decode({finalHandshake->data(),finalHandshake->size()},reply)==wire::Error::None);
 assert(phone.receive(reply.payload,response,sizeof(response),r));assert(phone.ready());
 assert(wire::decode({sent.back().data(),sent.back().size()},reply)==wire::Error::None);assert(reply.type==0x11);
 assert(phone.decrypt(reply.payload,response,sizeof(response),r));assert(r==39&&response[0]==0x21);
 uint8_t state[39]={0x21,1,1,1,0,2,32};std::memcpy(state+7,remote.signingPublic(),32);
 assert(phone.encrypt({state,sizeof(state)},handshake,sizeof(handshake),n));receive(m,packet(remote,0x11,{handshake,n},1800000000000ULL,{local.id(),8},false));assert(m.authenticatedCount()==1);
 auto command=packet(remote,2,{reinterpret_cast<const uint8_t*>("!info"),5},1800000000001ULL);receive(m,command);assert(m.stats().botReplies==1);receive(m,command);assert(m.stats().botReplies==1);
 receive(m,packet(remote,2,{reinterpret_cast<const uint8_t*>("!help"),5},1800000000002ULL));assert(m.stats().botReplies==1);
 m.disconnected(1);assert(m.authenticatedCount()==0);
 receive(m,packet(remote,2,{reinterpret_cast<const uint8_t*>("!rules"),6},1700000000000ULL));assert(m.stats().botReplies==1);
 std::puts("Mesh: signed discovery/chat, forgery/replay rejection, Noise key binding, authenticated state, relay and bot cooldown passed");
}
