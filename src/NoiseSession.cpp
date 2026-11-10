#include "NoiseSession.h"
#include <cstring>
namespace stick {
NoiseSession::~NoiseSession(){reset();}
void NoiseSession::reset(){crypto::wipe(this,sizeof(*this));state_=State::Idle;}
bool NoiseSession::fail(){reset();state_=State::Failed;return false;}
void NoiseSession::mixHash(wire::Bytes bytes){crypto::hash({hash_,32},bytes,hash_);}
bool NoiseSession::mixDh(const uint8_t secret[32],const uint8_t remote[32]) {
  uint8_t shared[32];if(!crypto::dh(secret,remote,shared))return false;
  crypto::hkdf2(chaining_,{shared,32},chaining_,cipher_);crypto::wipe(shared,32);handshakeNonce_=0;return true;
}
bool NoiseSession::encryptHash(wire::Bytes plain,uint8_t* out,size_t capacity,size_t& written) {
  if(!crypto::seal(cipher_,handshakeNonce_,{hash_,32},plain,out,capacity,written))return false;
  ++handshakeNonce_;mixHash({out,written});return true;
}
bool NoiseSession::decryptHash(wire::Bytes cipher,uint8_t* out,size_t capacity,size_t& written) {
  if(!crypto::open(cipher_,handshakeNonce_,{hash_,32},cipher,out,capacity,written))return false;
  ++handshakeNonce_;mixHash(cipher);return true;
}
bool NoiseSession::bound() const {
  uint8_t id[32];crypto::hash({remoteStatic_,32},{},id);return crypto::equal(id,expected_,8);
}
void NoiseSession::split() {
  uint8_t a[32],b[32];crypto::hkdf2(chaining_,{},a,b);
  std::memcpy(send_,initiator_?a:b,32);std::memcpy(receive_,initiator_?b:a,32);
  crypto::wipe(a,32);crypto::wipe(b,32);crypto::wipe(static_,32);crypto::wipe(ephemeral_,32);
  crypto::wipe(cipher_,32);crypto::wipe(chaining_,32);crypto::wipe(remoteEphemeral_,32);state_=State::Ready;
}
bool NoiseSession::begin(bool initiator,const uint8_t staticSecret[32],const uint8_t ephemeral[32],const uint8_t expectedPeer[8],uint8_t* out,size_t capacity,size_t& written) {
  reset();written=0;initiator_=initiator;
  std::memcpy(static_,staticSecret,32);std::memcpy(ephemeral_,ephemeral,32);std::memcpy(expected_,expectedPeer,8);
  crypto::publicDh(ephemeral_,localEphemeral_);
  static const char name[]="Noise_XX_25519_ChaChaPoly_SHA256";
  static_assert(sizeof(name)==33,"Noise protocol name length");
  std::memcpy(hash_,name,32);std::memcpy(chaining_,hash_,32);mixHash({});
  if(!initiator){state_=State::First;return true;}
  if(!out || capacity<32)return fail();
  std::memcpy(out,localEphemeral_,32);mixHash({out,32});mixHash({});written=32;state_=State::Second;return true;
}
bool NoiseSession::receive(wire::Bytes message,uint8_t* out,size_t capacity,size_t& written) {
  written=0;if(!message.data)return fail();size_t n=0;uint8_t empty[1]{},localStatic[32];
  if(state_==State::First && !initiator_) {
    if(message.size!=32 || !out || capacity<96)return fail();
    std::memcpy(remoteEphemeral_,message.data,32);mixHash(message);mixHash({});
    std::memcpy(out,localEphemeral_,32);mixHash({out,32});
    if(!mixDh(ephemeral_,remoteEphemeral_))return fail();
    crypto::publicDh(static_,localStatic);
    if(!encryptHash({localStatic,32},out+32,48,n) || !mixDh(static_,remoteEphemeral_) ||
       !encryptHash({},out+80,16,n))return fail();
    written=96;state_=State::Third;return true;
  }
  if(state_==State::Second && initiator_) {
    if(message.size!=96 || !out || capacity<64)return fail();
    std::memcpy(remoteEphemeral_,message.data,32);mixHash({message.data,32});
    if(!mixDh(ephemeral_,remoteEphemeral_) || !decryptHash({message.data+32,48},remoteStatic_,32,n) ||
       !mixDh(ephemeral_,remoteStatic_) || !decryptHash({message.data+80,16},empty,1,n) || !bound())return fail();
    crypto::publicDh(static_,localStatic);
    if(!encryptHash({localStatic,32},out,48,n) || !mixDh(static_,remoteEphemeral_) || !encryptHash({},out+48,16,n))return fail();
    written=64;split();return true;
  }
  if(state_==State::Third && !initiator_) {
    if(message.size!=64 || !decryptHash({message.data,48},remoteStatic_,32,n) || !mixDh(ephemeral_,remoteStatic_) ||
       !decryptHash({message.data+48,16},empty,1,n) || !bound())return fail();
    split();return true;
  }
  return fail();
}
bool NoiseSession::seen(uint32_t nonce) const {
  if(!received_ || nonce>highest_)return false;
  const uint32_t offset=highest_-nonce;
  return offset>=1024 || (window_[offset/8]&(1u<<(offset%8)))!=0;
}
void NoiseSession::remember(uint32_t nonce) {
  if(!received_ || nonce>highest_) {
    const uint32_t shift=received_?nonce-highest_:1024;
    if(shift>=1024)std::memset(window_,0,sizeof(window_));
    else for(int i=1023;i>=0;--i) {
      const bool bit=i>=int(shift) && (window_[(i-shift)/8]&(1u<<((i-shift)%8)));
      const uint8_t mask=uint8_t(1u<<(i%8));
      if(bit)window_[i/8]|=mask;else window_[i/8]&=uint8_t(~mask);
    }
    highest_=nonce;received_=true;
  }
  const auto offset=highest_-nonce;window_[offset/8]|=uint8_t(1u<<(offset%8));
}
bool NoiseSession::encrypt(wire::Bytes plain,uint8_t* out,size_t capacity,size_t& written) {
  written=0;if(!ready() || !out || capacity<4 || plain.size>wire::kMaxPayloadBytes-20 || sendNonce_>=10000)return false;
  size_t n=0;if(!crypto::seal(send_,sendNonce_,{},plain,out+4,capacity-4,n))return false;
  for(size_t i=0;i<4;++i)out[i]=uint8_t(sendNonce_>>(24-i*8));++sendNonce_;written=n+4;return true;
}
bool NoiseSession::decrypt(wire::Bytes cipher,uint8_t* out,size_t capacity,size_t& written) {
  written=0;if(!ready() || !cipher.data || cipher.size<20)return false;
  uint32_t nonce=0;for(size_t i=0;i<4;++i)nonce=(nonce<<8)|cipher.data[i];
  if(nonce>=10000 || seen(nonce))return false;
  if(!crypto::open(receive_,nonce,{}, {cipher.data+4,cipher.size-4},out,capacity,written))return false;
  remember(nonce);return true;
}
}
