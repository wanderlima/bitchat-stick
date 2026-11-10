#pragma once
#include "CryptoPrimitives.h"
namespace stick {
class NoiseSession {
 public:
  enum class State { Idle, First, Second, Third, Ready, Failed };
  NoiseSession() = default;
  ~NoiseSession();
  NoiseSession(const NoiseSession&) = delete;
  NoiseSession& operator=(const NoiseSession&) = delete;
  void reset();
  bool begin(bool initiator,const uint8_t staticSecret[32],const uint8_t ephemeral[32],const uint8_t expectedPeer[8],uint8_t* out,size_t capacity,size_t& written);
  bool receive(wire::Bytes message,uint8_t* out,size_t capacity,size_t& written);
  bool encrypt(wire::Bytes plain,uint8_t* out,size_t capacity,size_t& written);
  bool decrypt(wire::Bytes cipher,uint8_t* out,size_t capacity,size_t& written);
  bool ready() const { return state_==State::Ready; }
  State state() const { return state_; }
  const uint8_t* remoteKey() const { return ready()?remoteStatic_:nullptr; }
  const uint8_t* handshakeHash() const { return ready()?hash_:nullptr; }
 private:
  bool fail();void mixHash(wire::Bytes bytes);bool mixDh(const uint8_t secret[32],const uint8_t remote[32]);
  bool encryptHash(wire::Bytes plain,uint8_t* out,size_t capacity,size_t& written);
  bool decryptHash(wire::Bytes cipher,uint8_t* out,size_t capacity,size_t& written);
  bool bound() const;void split();
  bool seen(uint32_t nonce) const;void remember(uint32_t nonce);
  State state_=State::Idle;bool initiator_=false,received_=false;
  uint8_t static_[32]{},ephemeral_[32]{},localEphemeral_[32]{},remoteEphemeral_[32]{},remoteStatic_[32]{},expected_[8]{};
  uint8_t chaining_[32]{},hash_[32]{},cipher_[32]{},send_[32]{},receive_[32]{},window_[128]{};
  uint64_t handshakeNonce_=0;
  uint32_t sendNonce_=0,highest_=0;
};
}
