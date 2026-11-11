#include "Mesh.h"
#include "PacketSecurity.h"
#include <Preferences.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>
using namespace stick;
static bool sendAllowed=true;static uint8_t generation=1;static std::vector<std::vector<uint8_t>> sent;
void entropy(void* p,size_t n){for(size_t i=0;i<n;++i)static_cast<uint8_t*>(p)[i]=uint8_t(i+generation);}
bool send(wire::Bytes b,uint16_t,uint16_t){if(!sendAllowed)return false;sent.emplace_back(b.data,b.data+b.size);return true;}
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
 auto text=packet(remote,2,{reinterpret_cast<const uint8_t*>("hello"),5});receive(m,text);assert(m.messageCount()==1&&m.stats().receivedMesh==1);receive(m,text);assert(m.messageCount()==1&&m.stats().receivedMesh==1);
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
 // Receive an authenticated private TLV message and verify the encrypted delivery receipt.
 uint8_t dm[]={1,0,4,'d','m','0','1',1,5,'!','i','n','f','o'};
 assert(phone.encrypt({dm,sizeof(dm)},handshake,sizeof(handshake),n));
 receive(m,packet(remote,0x11,{handshake,n},1800000000001ULL,{local.id(),8},false));
 assert(m.unreadCount(MessageScope::Direct)==1&&m.unreadCount(MessageScope::Public)==1);
 m.markRead(m.message(0)->sequence);assert(m.unreadCount(MessageScope::Direct)==0&&m.unreadCount()==1);
 assert(m.stats().receivedDm==1);
 assert(m.message(0)->privateMessage && std::strcmp(m.message(0)->text,"!info")==0);assert(m.stats().botReplies==0);
 assert(wire::decode({sent.back().data(),sent.back().size()},reply)==wire::Error::None);assert(reply.type==0x11);
 assert(phone.decrypt(reply.payload,response,sizeof(response),r));assert(r==5&&response[0]==3&&std::memcmp(response+1,"dm01",4)==0);
 const auto beforeRead=sent.size();
 m.tick(1100,1800000001100ULL);assert(sent.size()==beforeRead+1);
 assert(wire::decode({sent.back().data(),sent.back().size()},reply)==wire::Error::None);
 assert(phone.decrypt(reply.payload,response,sizeof(response),r));assert(r==5&&response[0]==2&&std::memcmp(response+1,"dm01",4)==0);
 assert(m.message(0)->readReceiptSent);m.markRead(m.message(0)->sequence);m.tick(2200,1800000002200ULL);assert(sent.size()==beforeRead+1);
 const auto stored=m.messageCount();assert(phone.encrypt({dm,sizeof(dm)},handshake,sizeof(handshake),n));
 receive(m,packet(remote,0x11,{handshake,n},1800000000002ULL,{local.id(),8},false));assert(m.messageCount()==stored);
 // Duplicate content fields are rejected even inside a valid session.
 uint8_t malformed[]={1,0,1,'x',1,1,'a',1,1,'b'};
 assert(phone.encrypt({malformed,sizeof(malformed)},handshake,sizeof(handshake),n));
 receive(m,packet(remote,0x11,{handshake,n},1800000000003ULL,{local.id(),8},false));assert(m.messageCount()==stored);
 // Unopened DMs never send read receipts; a failed enqueue retries after opening.
 uint8_t secondDm[]={1,0,4,'d','m','0','2',1,2,'h','i'};
 assert(phone.encrypt({secondDm,sizeof(secondDm)},handshake,sizeof(handshake),n));
 receive(m,packet(remote,0x11,{handshake,n},1800000002201ULL,{local.id(),8},false));
 const auto pendingRead=sent.size();m.tick(3300,1800000003300ULL);assert(sent.size()==pendingRead&&m.unreadCount(MessageScope::Direct)==1);
 m.markRead(m.message(0)->sequence);sendAllowed=false;m.tick(4400,1800000004400ULL);assert(!m.message(0)->readReceiptSent&&sent.size()==pendingRead);
 sendAllowed=true;m.tick(5500,1800000005500ULL);assert(m.message(0)->readReceiptSent&&sent.size()==pendingRead+1);
 assert(wire::decode({sent.back().data(),sent.back().size()},reply)==wire::Error::None);
 assert(phone.decrypt(reply.payload,response,sizeof(response),r));assert(r==5&&response[0]==2&&std::memcmp(response+1,"dm02",4)==0);
 // Outgoing private text must use the authenticated session and official TLV payload.
 const auto beforeOutgoing=m.messageCount();
 assert(!m.sendDirect(nullptr,"hi")&&!m.sendDirect(remote.id(),"")&&!m.sendDirect(remote.id(),"   "));
 char tooLong[194];std::memset(tooLong,'x',193);tooLong[193]=0;assert(!m.sendDirect(remote.id(),tooLong));
 sendAllowed=false;assert(!m.sendDirect(remote.id(),"retry"));assert(m.messageCount()==beforeOutgoing);sendAllowed=true;
 char maximum[193];std::memset(maximum,'a',192);maximum[192]=0;
 assert(m.sendDirect(remote.id(),maximum));
 auto* outgoing=m.message(0);assert(outgoing->outgoing&&outgoing->privateMessage&&!outgoing->unread);
 assert(outgoing->delivery==DeliveryState::Queued&&outgoing->messageIdLength==32&&std::memcmp(outgoing->senderId,remote.id(),8)==0);
 assert(wire::decode({sent.back().data(),sent.back().size()},reply)==wire::Error::None);
 assert(reply.type==0x11&&reply.recipient.size==8&&std::memcmp(reply.recipient.data,remote.id(),8)==0);
 uint8_t decrypted[300];assert(phone.decrypt(reply.payload,decrypted,sizeof(decrypted),r));
 assert(r==229&&decrypted[0]==1&&decrypted[1]==0&&decrypted[2]==32&&decrypted[35]==1&&decrypted[36]==192);
 assert(std::memcmp(decrypted+37,maximum,192)==0);
 uint8_t ack[33]={3};std::memcpy(ack+1,outgoing->messageId,32);
 assert(phone.encrypt({ack,sizeof(ack)},handshake,sizeof(handshake),n));receive(m,packet(remote,0x11,{handshake,n},1800000005501ULL,{local.id(),8},false));
 assert(outgoing->delivery==DeliveryState::Delivered);
 ack[0]=2;assert(phone.encrypt({ack,sizeof(ack)},handshake,sizeof(handshake),n));receive(m,packet(remote,0x11,{handshake,n},1800000005502ULL,{local.id(),8},false));
 assert(outgoing->delivery==DeliveryState::Read);
 ack[0]=3;assert(phone.encrypt({ack,sizeof(ack)},handshake,sizeof(handshake),n));receive(m,packet(remote,0x11,{handshake,n},1800000005503ULL,{local.id(),8},false));
 assert(outgoing->delivery==DeliveryState::Read); // Late delivered cannot downgrade read.
 const auto beforeOutgoingRead=sent.size();m.markRead(outgoing->sequence);m.tick(6600,1800000006600ULL);assert(sent.size()==beforeOutgoingRead);
 auto command=packet(remote,2,{reinterpret_cast<const uint8_t*>("!info"),5},1800000000001ULL);receive(m,command);assert(m.stats().botReplies==1);receive(m,command);assert(m.stats().botReplies==1);
 receive(m,packet(remote,2,{reinterpret_cast<const uint8_t*>("!help"),5},1800000000002ULL));assert(m.stats().botReplies==1);
 m.setBotEnabled(false);m.tick(40000,1800000040000ULL);
 receive(m,packet(remote,2,{reinterpret_cast<const uint8_t*>("!rules"),6},1800000040000ULL));assert(m.stats().botReplies==1);
 m.setBotEnabled(true);receive(m,packet(remote,2,{reinterpret_cast<const uint8_t*>("!help"),5},1800000040001ULL));assert(m.stats().botReplies==2);
 m.disconnected(1);assert(m.authenticatedCount()==0);assert(!m.sendDirect(remote.id(),"offline"));
 receive(m,packet(remote,2,{reinterpret_cast<const uint8_t*>("!rules"),6},1700000000000ULL));assert(m.stats().botReplies==2);
 for(unsigned i=0;i<20;++i)assert(m.sendText("outgoing"));
 assert(m.messageCount()==12&&m.messageCount(MessageScope::Direct)==0&&m.unreadCount()==0);
 std::puts("Mesh: signed discovery/chat, forgery/replay rejection, Noise key binding, authenticated state, relay and bot cooldown passed");
}
