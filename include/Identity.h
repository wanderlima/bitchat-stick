#pragma once
#include "CryptoPrimitives.h"
namespace stick {
// A persisted identity is required before advertising. Storage failures fail closed.
class Identity {
 public:
  using Entropy = void (*)(void*, size_t);
  ~Identity();
  bool begin(Entropy entropy);
  bool ready() const { return ready_; }
  const uint8_t* dhSecret() const { return noise_; }
  const uint8_t* dhPublic() const { return noisePublic_; }
  const uint8_t* signingSecret() const { return signing_; }
  const uint8_t* signingPublic() const { return signingPublic_; }
  const uint8_t* id() const { return id_; }
  Identity() = default;
  Identity(const Identity&) = delete;
  Identity& operator=(const Identity&) = delete;
 private:
  uint8_t noise_[32]{}, noisePublic_[32]{}, signing_[64]{}, signingPublic_[32]{}, id_[8]{};
  bool ready_ = false;
};
}
