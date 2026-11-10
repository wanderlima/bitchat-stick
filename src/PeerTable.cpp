#include "PeerTable.h"
#include <cstring>

namespace stick {
void PeerTable::observe(const Observation& value) {
  Peer* target = nullptr;
  for (auto& peer : peers_) {
    if (peer.used && peer.observation.addressType == value.addressType &&
        std::strncmp(peer.observation.address, value.address, sizeof(value.address)) == 0) {
      target = &peer;
      break;
    }
  }
  if (!target) {
    for (auto& peer : peers_) {
      if (!peer.used) { target = &peer; break; }
    }
  }
  if (!target) {
    target = &peers_[0];
    for (auto& peer : peers_) {
      // Unsigned subtraction also handles millis() wrapping.
      if (uint32_t(value.seenAt - peer.observation.seenAt) >
          uint32_t(value.seenAt - target->observation.seenAt)) target = &peer;
    }
  }
  target->observation = value;
  target->observation.address[sizeof(value.address) - 1] = '\0';
  target->observation.name[sizeof(value.name) - 1] = '\0';
  target->used = true;
}
void PeerTable::expire(uint32_t now) {
  for (auto& peer : peers_)
    if (peer.used && uint32_t(now - peer.observation.seenAt) >= kPeerExpiryMs) peer.used = false;
}
size_t PeerTable::count() const {
  size_t n = 0;
  for (const auto& peer : peers_) if (peer.used) ++n;
  return n;
}
size_t PeerTable::candidates() const {
  size_t n = 0;
  for (const auto& peer : peers_)
    if (peer.used && peer.observation.network != Network::Other) ++n;
  return n;
}
const Peer* PeerTable::at(size_t ordinal) const {
  for (const auto& peer : peers_) if (peer.used && ordinal-- == 0) return &peer;
  return nullptr;
}
} // namespace stick
