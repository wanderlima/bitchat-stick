#include "Mesh.h"
#include "PacketSecurity.h"
#include "InfoBot.h"
#include <cstring>
#include <cstdio>
namespace stick {
namespace {
bool same(const uint8_t* a,const uint8_t* b,size_t n=8){return crypto::equal(a,b,n);}
bool liveTimestamp(uint64_t now,uint64_t stamp){return stamp<now?now-stamp<=120000:stamp-now<=120000;}
bool broadcast(wire::Bytes id){if(!id.size)return true;for(size_t i=0;i<id.size;++i)if(id.data[i]!=255)return false;return true;}
// Reject malformed UTF-8 and controls; the display substitutes unsupported glyphs.
bool textValid(wire::Bytes b){
  for(size_t i=0;i<b.size;){uint32_t cp=b.data[i++];unsigned n=0;uint32_t minimum=0;
    if(cp<128){if(cp<32 && cp!=10 && cp!=9)return false; if(cp==127)return false;continue;}
    if(cp>=0xc2 && cp<=0xdf){n=1;cp&=31;minimum=128;}
    else if(cp>=0xe0 && cp<=0xef){n=2;cp&=15;minimum=2048;}
    else if(cp>=0xf0 && cp<=0xf4){n=3;cp&=7;minimum=65536;}else return false;
    if(i+n>b.size)return false;
    while(n--){uint8_t c=b.data[i++];if((c&0xc0)!=0x80)return false;cp=(cp<<6)|(c&63);}
    if(cp<minimum || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff))return false;
  }return true;
}
void displayText(char* out,size_t capacity,wire::Bytes text){size_t n=text.size<capacity-1?text.size:capacity-1;for(size_t i=0;i<n;++i){auto c=text.data[i];out[i]=(c>=32&&c<=126)?char(c):' ';}out[n]=0;}
}
Mesh& mesh(){static Mesh value;return value;}
void Mesh::begin(Identity& identity,Identity::Entropy entropy,Send send){identity_=&identity;entropy_=entropy;send_=send;}
void Mesh::tick(uint32_t now,uint64_t epochMs){now_=now;epoch_=epochMs;assembler_.expire(now);
  for(auto& s:sessions_)if(s.used && (!find(s.id)||uint32_t(now-s.started)>(s.noise.ready()?3600000u:15000u))){auto* p=find(s.id);if(p)p->authenticated=false;s.noise.reset();s.used=false;}
  for(auto& p:peers_)if(p.used && uint32_t(now-p.seen)>900000){p=MeshPeer{};}
  if(uint32_t(now-lastReceiptAttempt_)>=1000){
    lastReceiptAttempt_=now;
    for(auto& message:messages_)if(message.privateMessage&&!message.unread&&!message.readReceiptSent){
      if(sendReadReceipt(message))break;
    }
  }
  if(ready() && (!lastAnnounce_ || uint32_t(now-lastAnnounce_)>=30000)){lastAnnounce_=now;announce();}
}
size_t Mesh::peerCount()const{size_t n=0;for(const auto& p:peers_)n+=p.used;return n;}
size_t Mesh::authenticatedCount()const{size_t n=0;for(const auto& p:peers_)n+=p.used&&p.authenticated;return n;}
const MeshPeer* Mesh::peer(size_t ordinal)const{for(const auto& p:peers_)if(p.used && !ordinal--)return &p;return nullptr;}
MeshPeer* Mesh::find(const uint8_t* id){for(auto& p:peers_)if(p.used&&same(p.id,id))return &p;return nullptr;}
namespace {
bool inScope(const ChatMessage& message,MessageScope scope){
  return scope==MessageScope::All || message.privateMessage==(scope==MessageScope::Direct);
}
}
size_t Mesh::messageCount(MessageScope scope)const{
  size_t count=0;for(size_t i=0;i<messageCount_;++i)count+=inScope(messages_[(messageNext_+messages_.size()-1-i)%messages_.size()],scope);return count;
}
const ChatMessage* Mesh::message(size_t offset,MessageScope scope)const{
  for(size_t i=0;i<messageCount_;++i){const auto& m=messages_[(messageNext_+messages_.size()-1-i)%messages_.size()];if(inScope(m,scope)&&!offset--)return &m;}return nullptr;
}
size_t Mesh::unreadCount(MessageScope scope)const{
  size_t count=0;for(size_t i=0;i<messageCount_;++i){const auto& m=messages_[(messageNext_+messages_.size()-1-i)%messages_.size()];count+=m.unread&&inScope(m,scope);}return count;
}
// Return true when a live-session attempt was made, including a full queue.
// The tick loop permits at most one encryption/enqueue attempt per second.
bool Mesh::sendReadReceipt(ChatMessage& message){
  if(message.outgoing||!message.privateMessage||!message.messageIdLength||message.readReceiptSent||!ready())return false;
  auto* peer=find(message.senderId);if(!peer||!peer->authenticated)return false;
  for(auto& s:sessions_)if(s.used&&s.noise.ready()&&same(s.id,message.senderId)){
    uint8_t payload[65],encrypted[85];payload[0]=0x02;std::memcpy(payload+1,message.messageId,message.messageIdLength);size_t n=0;
    if(!s.noise.encrypt({payload,size_t(message.messageIdLength)+1},encrypted,sizeof(encrypted),n))return true;
    wire::FrameView f;f.type=0x11;f.ttl=7;f.sender={identity_->id(),8};f.recipient={message.senderId,8};f.timestamp=epoch_;f.payload={encrypted,n};
    message.readReceiptSent=emit(f,s.link,kAnyLink,false);return true;
  }
  return false;
}
void Mesh::markRead(uint32_t sequence){for(auto& m:messages_)if(m.sequence==sequence)m.unread=false;}
ChatMessage* Mesh::store(const char* name,wire::Bytes text,bool outgoing,bool privateMessage){auto& m=messages_[messageNext_];m=ChatMessage{};if(!++messageSequence_)++messageSequence_;m.sequence=messageSequence_;m.unread=!outgoing;std::snprintf(m.name,sizeof(m.name),"%s",name);displayText(m.text,sizeof(m.text),text);m.outgoing=outgoing;m.privateMessage=privateMessage;m.truncated=text.size>=sizeof(m.text);messageNext_=(messageNext_+1)%messages_.size();if(messageCount_<messages_.size())++messageCount_;return &m;}
bool Mesh::emit(wire::FrameView& frame,uint16_t target,uint16_t except,bool sign){
  if(!ready()||!send_)return false;
  uint8_t signature[64];
  if(sign){frame.sender={identity_->id(),8};uint64_t stamp=epoch_>lastStamp_?epoch_:lastStamp_+1;lastStamp_=stamp;frame.timestamp=stamp;
    if(!wire::signPacket(frame,identity_->signingSecret(),scratch_,sizeof(scratch_),signature))return false;frame.signature={signature,64};}
  size_t n=0;bool ok=wire::encode(frame,out_,sizeof(out_),n)==wire::Error::None && send_({out_,n},target,except);
  if(ok)++stats_.sent;else ++stats_.dropped;return ok;
}
bool Mesh::announce(uint16_t target){if(!ready())return false;
  uint8_t payload[100];const char* name="Bitchat Stick";size_t n=std::strlen(name),p=0;
  payload[p++]=1;payload[p++]=uint8_t(n);std::memcpy(payload+p,name,n);p+=n;
  payload[p++]=2;payload[p++]=32;std::memcpy(payload+p,identity_->dhPublic(),32);p+=32;
  payload[p++]=3;payload[p++]=32;std::memcpy(payload+p,identity_->signingPublic(),32);p+=32;
  wire::FrameView f;f.type=1;f.ttl=7;f.payload={payload,p};return emit(f,target);
}
bool Mesh::sendText(const char* text){
  sendStatus_="Not ready";if(!ready()||!send_)return false;
  sendStatus_="Bad text";if(!text)return false;size_t n=std::strlen(text);
  if(!n||n>192||!textValid({reinterpret_cast<const uint8_t*>(text),n}))return false;
  wire::FrameView f;f.type=2;f.ttl=7;f.payload={reinterpret_cast<const uint8_t*>(text),n};
  if(!emit(f)){sendStatus_="No phone";return false;}
  store("You",f.payload,true);sendStatus_="Queued";return true;
}
bool Mesh::sendDirect(const uint8_t recipient[8],const char* text) {
  sendStatus_="Not ready";
  if(!ready()||!entropy_)return false;
  sendStatus_="Bad text";if(!recipient||!text)return false;
  const size_t length=strnlen(text,193);
  if(!length||length>192||!textValid({reinterpret_cast<const uint8_t*>(text),length}))return false;
  bool visible=false;for(size_t i=0;i<length;++i)visible|=uint8_t(text[i])>32;
  if(!visible)return false;
  auto* peer=find(recipient);if(!peer){sendStatus_="Peer gone";return false;}
  if(!peer->authenticated){startNoise(*peer,peer->link);sendStatus_="Connecting";return false;}
  Session* live=nullptr;
  for(auto& s:sessions_)if(s.used&&s.noise.ready()&&same(s.id,recipient)){live=&s;break;}
  if(!live){peer->authenticated=false;startNoise(*peer,peer->link);sendStatus_="Connecting";return false;}
  uint8_t random[16];entropy_(random,sizeof(random));char id[33];
  constexpr char hex[]="0123456789abcdef";
  for(size_t i=0;i<sizeof(random);++i){id[2*i]=hex[random[i]>>4];id[2*i+1]=hex[random[i]&15];}id[32]=0;
  crypto::wipe(random,sizeof(random));
  uint8_t payload[229]{1,0,32},encrypted[249];
  std::memcpy(payload+3,id,32);payload[35]=1;payload[36]=uint8_t(length);std::memcpy(payload+37,text,length);
  size_t written=0;const bool encryptedOk=live->noise.encrypt({payload,37+length},encrypted,sizeof(encrypted),written);
  crypto::wipe(payload,sizeof(payload));
  if(!encryptedOk){peer->authenticated=false;live->noise.reset();live->used=false;startNoise(*peer,peer->link);sendStatus_="Connecting";return false;}
  wire::FrameView frame;frame.type=0x11;frame.ttl=7;frame.sender={identity_->id(),8};frame.recipient={recipient,8};frame.timestamp=epoch_;frame.payload={encrypted,written};
  if(!emit(frame,live->link,kAnyLink,false)){sendStatus_="Busy";return false;}
  auto* message=store(peer->name,{reinterpret_cast<const uint8_t*>(text),length},true,true);
  std::memcpy(message->senderId,recipient,8);std::memcpy(message->messageId,id,32);message->messageIdLength=32;
  message->readReceiptSent=true;sendStatus_="Queued";return true;
}
bool Mesh::duplicate(const wire::FrameView& f){size_t n=0;if(!wire::canonical(f,scratch_,sizeof(scratch_),n))return true;uint8_t digest[32];crypto::hash({scratch_,n},f.signature,digest);return rememberDigest(digest);
}
bool Mesh::rememberDigest(const uint8_t digest[32]){
  for(size_t i=0;i<dedupCount_;++i)if(same(dedup_[i].data(),digest,16))return true;
  std::memcpy(dedup_[dedupNext_].data(),digest,16);dedupNext_=(dedupNext_+1)%dedup_.size();if(dedupCount_<dedup_.size())++dedupCount_;return false;
}
void Mesh::receive(wire::Bytes bytes,uint16_t link){if(!ready())return;wire::FrameView frame;
  if(wire::decode(bytes,frame)!=wire::Error::None){lastReject_="frame";++stats_.rejected;return;}
  if(frame.type==0x20){size_t n=0;auto r=assembler_.append(frame,now_,assembled_,sizeof(assembled_),n);
    if(r==wire::AssemblyResult::Complete){if(wire::decode({assembled_,n},frame)!=wire::Error::None){++stats_.rejected;return;}}
    else {if(r!=wire::AssemblyResult::Stored && r!=wire::AssemblyResult::Duplicate)++stats_.rejected;return;}}
  process(frame,link);
}
void Mesh::process(wire::FrameView& f,uint16_t link){
  if(same(f.sender.data,identity_->id()) || !f.ttl || f.ttl>7)return;
  if(f.timestamp+900000<epoch_ || f.timestamp>epoch_+300000){lastReject_="clock";++stats_.rejected;return;}
  if(f.type==0x10||f.type==0x11){handleNoise(f,link);return;}
  if(!broadcast(f.recipient) || (f.type!=1 && f.type!=2 && f.type!=3)){++stats_.rejected;return;}
  MeshPeer* p=find(f.sender.data);size_t size=0;
  if(!wire::unpackPayload(f,plain_,sizeof(plain_),size)){++stats_.rejected;return;}
  if(f.type==1){wire::Bytes name,key,signing;bool valid=true;size_t at=0;
    while(at<size){if(at+2>size){valid=false;break;}uint8_t type=plain_[at++],len=plain_[at++];if(at+len>size){valid=false;break;}
      wire::Bytes* field=type==1?&name:type==2?&key:type==3?&signing:nullptr;
      if(field){if(field->data){valid=false;break;}*field={plain_+at,len};}at+=len;}
    uint8_t hash[32];if(key.size==32)crypto::hash(key,{},hash);
    if(!valid||!name.data||name.size>255||!textValid(name)||key.size!=32||signing.size!=32||!same(hash,f.sender.data)||!wire::verifyPacket(f,signing.data,scratch_,sizeof(scratch_))){++stats_.rejected;return;}
    if(p && (!same(p->noiseKey,key.data,32)||!same(p->signingKey,signing.data,32))){++stats_.rejected;return;}
    if(duplicate(f))return;
    if(!p){for(auto& slot:peers_)if(!slot.used){p=&slot;break;}}
    if(!p){++stats_.dropped;return;}
    p->used=true;std::memcpy(p->id,f.sender.data,8);std::memcpy(p->noiseKey,key.data,32);std::memcpy(p->signingKey,signing.data,32);displayText(p->name,sizeof(p->name),name);p->seen=now_;p->link=link;
    startNoise(*p,link);
  }else{
    if(!p||!wire::verifyPacket(f,p->signingKey,scratch_,sizeof(scratch_))){lastReject_="signature";++stats_.rejected;return;}
    if(f.type==2 && (!size||!textValid({plain_,size}))){++stats_.rejected;return;}
    if(duplicate(f))return;p->seen=now_;
    if(f.type==2){++stats_.receivedMesh;auto* message=store(p->name,{plain_,size},false);std::memcpy(message->senderId,p->id,8);bot(*p,{plain_,size},f.timestamp);}
    else {p->authenticated=false;}
  }
  ++stats_.accepted;
  if(f.ttl>1){--f.ttl;if(emit(f,kAnyLink,link,false))++stats_.relayed;}
}
Mesh::Session* Mesh::session(const uint8_t* id,uint16_t link){for(auto& s:sessions_)if(s.used&&same(s.id,id)&&s.link==link)return &s;return nullptr;}
void Mesh::startNoise(MeshPeer& p,uint16_t link){
  if(link==kAnyLink)return;
  if(auto* existing=session(p.id,link)){
    if(existing->noise.state()==NoiseSession::State::Failed || (existing->noise.ready()&&!p.authenticated)){
      existing->noise.reset();existing->used=false;
    }else return;
  }
  Session* s=nullptr;for(auto& slot:sessions_)if(!slot.used){s=&slot;break;}if(!s)return;
  uint8_t ephemeral[32],response[128];size_t n=0;entropy_(ephemeral,32);
  s->noise.reset();s->used=true;s->initiator=true;s->link=link;s->started=now_;std::memcpy(s->id,p.id,8);
  bool ok=s->noise.begin(true,identity_->dhSecret(),ephemeral,p.id,response,sizeof(response),n);crypto::wipe(ephemeral,sizeof(ephemeral));
  if(!ok){s->used=false;return;}
  wire::FrameView f;f.type=0x10;f.ttl=7;f.sender={identity_->id(),8};f.recipient={p.id,8};f.timestamp=epoch_;f.payload={response,n};
  if(!emit(f,link,kAnyLink,false)){s->noise.reset();s->used=false;}
}
void Mesh::handleNoise(const wire::FrameView& f,uint16_t link){
  if(f.recipient.size!=8||!same(f.recipient.data,identity_->id())||f.compressed){++stats_.rejected;return;}
  auto* p=find(f.sender.data);if(!p){++stats_.rejected;return;}
  auto* s=session(f.sender.data,link);uint8_t response[128];size_t n=0;
  if(f.type==0x10){
    if(f.payload.size==32){
      if(s && s->initiator && s->noise.state()==NoiseSession::State::Second){
        // Resolve crossed initiations deterministically; the smaller peer ID stays initiator.
        if(std::memcmp(identity_->id(),p->id,8)<0)return;
      } else if(s && uint32_t(now_-s->started)<5000){++stats_.rejected;return;}
      if(!s){for(auto& slot:sessions_)if(!slot.used){s=&slot;break;}}
      if(!s){++stats_.dropped;return;}
      s->noise.reset();s->used=true;s->initiator=false;s->link=link;s->started=now_;std::memcpy(s->id,f.sender.data,8);p->authenticated=false;
      uint8_t ephemeral[32];entropy_(ephemeral,32);bool ok=s->noise.begin(false,identity_->dhSecret(),ephemeral,p->id,response,sizeof(response),n);crypto::wipe(ephemeral,sizeof(ephemeral));if(!ok){s->used=false;return;}
    }
    if(!s||!s->noise.receive(f.payload,response,sizeof(response),n)){lastReject_="handshake";++stats_.rejected;return;}
    if(n){wire::FrameView reply;reply.type=0x10;reply.ttl=7;reply.sender={identity_->id(),8};reply.recipient={p->id,8};reply.timestamp=epoch_;reply.payload={response,n};emit(reply,link,kAnyLink,false);}
    if(s->noise.ready()){
      if(!same(s->noise.remoteKey(),p->noiseKey,32)){s->noise.reset();s->used=false;++stats_.rejected;return;}
      uint8_t state[39]={0x21,1,1,1,0,2,32};std::memcpy(state+7,identity_->signingPublic(),32);size_t encrypted=0;
      if(s->noise.encrypt({state,sizeof(state)},response,sizeof(response),encrypted)){wire::FrameView reply;reply.type=0x11;reply.ttl=7;reply.sender={identity_->id(),8};reply.recipient={p->id,8};reply.timestamp=epoch_;reply.payload={response,encrypted};emit(reply,link,kAnyLink,false);}
    }
  }else{
    if(!s||!s->noise.ready()){lastReject_="no-session";++stats_.rejected;return;}
    if(!s->noise.decrypt(f.payload,plain_,sizeof(plain_),n)){lastReject_="decrypt";++stats_.rejected;return;}
    struct ClearPlain {uint8_t* data;size_t size;~ClearPlain(){crypto::wipe(data,size);}} clear{plain_,sizeof(plain_)};
    if(n && plain_[0]==0x01){
      // Private text stays private: bot replies must use the same private context.
      wire::Bytes id,content;size_t at=1;bool valid=p->authenticated;
      while(at<n && valid){if(at+2>n){valid=false;break;}uint8_t type=plain_[at++],len=plain_[at++];if(at+len>n){valid=false;break;}
        wire::Bytes* field=type==0?&id:type==1?&content:nullptr;
        if(!field||field->data){valid=false;break;}*field={plain_+at,len};at+=len;
      }
      if(!valid||!id.size||id.size>64||!content.size||!textValid(id)||!textValid(content)){lastReject_=p->authenticated?"dm-payload":"unauth-dm";++stats_.rejected;return;}
      uint8_t digest[32];crypto::hash({p->id,8},id,digest);
      const bool isNew=!rememberDigest(digest);
      if(isNew){
        ++stats_.receivedDm;
        auto* message=store(p->name,content,false,true);std::memcpy(message->senderId,p->id,8);
        std::memcpy(message->messageId,id.data,id.size);message->messageIdLength=uint8_t(id.size);
        p->seen=now_;++stats_.accepted;
      }
      // Delivery acknowledgement is encrypted and means stored, never read.
      uint8_t receipt[65],encrypted[85];receipt[0]=0x03;std::memcpy(receipt+1,id.data,id.size);size_t written=0;
      if(s->noise.encrypt({receipt,id.size+1},encrypted,sizeof(encrypted),written)){
        wire::FrameView reply;reply.type=0x11;reply.ttl=7;reply.sender={identity_->id(),8};reply.recipient={p->id,8};reply.timestamp=epoch_;reply.payload={encrypted,written};emit(reply,link,kAnyLink,false);
      }
      if(isNew){bot(*p,content,f.timestamp,true);introduce(*p,f.timestamp,infoBotReply(content)!=nullptr);}
      return;
    }
    if(n>=2&&(plain_[0]==0x02||plain_[0]==0x03)) {
      if(!p->authenticated||n>65||!textValid({plain_+1,n-1})){++stats_.rejected;return;}
      for(auto& message:messages_)if(message.outgoing&&message.privateMessage&&same(message.senderId,p->id)&&
          message.messageIdLength==n-1&&same(message.messageId,plain_+1,n-1)) {
        if(plain_[0]==0x02)message.delivery=DeliveryState::Read;
        else if(message.delivery!=DeliveryState::Read)message.delivery=DeliveryState::Delivered;
        ++stats_.accepted;return;
      }
      return; // Evicted/unknown message receipts cannot update another conversation.
    }
    // Other private media/group payloads remain unsupported.
    if(n<2||plain_[0]!=0x21||plain_[1]!=1){++stats_.rejected;return;}
    bool caps=false,key=false;size_t at=2;
    while(at<n){if(at+2>n){++stats_.rejected;return;}uint8_t type=plain_[at++],len=plain_[at++];if(at+len>n){++stats_.rejected;return;}
      if(type==1){if(caps||!len||len>8||(len>1&&plain_[at+len-1]==0)){++stats_.rejected;return;}caps=true;}
      if(type==2){if(key||len!=32||!same(plain_+at,p->signingKey,32)){++stats_.rejected;return;}key=true;}at+=len;
    }
    if(!caps||!key){++stats_.rejected;return;}p->authenticated=true;p->seen=now_;p->link=link;++stats_.accepted;
  }
}
void Mesh::disconnected(uint16_t link){for(auto& s:sessions_)if(s.used&&s.link==link){auto* p=find(s.id);if(p)p->authenticated=false;s.noise.reset();s.used=false;}}
void Mesh::introduce(MeshPeer& p,uint64_t timestamp,bool command){
  if(!botEnabled_||!p.authenticated||!liveTimestamp(epoch_,timestamp))return;
  for(size_t i=0;i<introducedCount_;++i)if(same(introduced_[i].data(),p.id))return;
  if(introducedCount_==introduced_.size())return;
  if(!sendDirect(p.id,kBotIntroduction)){if(!command)botStatus_="Queue busy";return;}
  std::memcpy(introduced_[introducedCount_++].data(),p.id,8);++stats_.botIntroductions;
  if(!command)botStatus_="Intro sent";
}
void Mesh::bot(MeshPeer& p,wire::Bytes text,uint64_t timestamp,bool direct){
  const char* reply=infoBotReply(text);if(!reply)return;
  if(!botEnabled_){botStatus_="Bot OFF";return;}
  // Public callers already verified the packet signature; only DMs need Noise auth.
  if(direct&&!p.authenticated){botStatus_="Needs auth";return;}
  // Tolerate ordinary clock skew, but do not answer delayed history as a live command.
  if(!liveTimestamp(epoch_,timestamp)){botStatus_="Check clock";return;}
  if((p.botReplied&&uint32_t(now_-p.lastBot)<3000)||(replied_&&uint32_t(now_-lastBot_)<1000)){botStatus_="Wait 3s";return;}
  if(direct?sendDirect(p.id,reply):sendText(reply)){
    p.botReplied=true;p.lastBot=now_;lastBot_=now_;replied_=true;++stats_.botReplies;botStatus_=direct?"DM reply":"Public reply";
  }else botStatus_="Queue busy";
}
}
