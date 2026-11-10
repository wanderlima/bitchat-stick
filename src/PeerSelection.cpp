#include "PeerSelection.h"
#include <cstring>

namespace stick {
void PeerSelection::remember(const Peer& peer) {
  std::memcpy(address_, peer.observation.address, sizeof(address_));
  address_[sizeof(address_) - 1] = '\0';
  addressType_ = peer.observation.addressType;
  tracked_ = true;
}
bool PeerSelection::sync(const PeerTable& peers) {
  const auto count = peers.candidates();
  if (!count) {
    const bool changed = tracked_;
    tracked_ = false;
    index_ = 0;
    return changed;
  }
  if (tracked_) {
    for (size_t i = 0; i < count; ++i) {
      const auto& obs = peers.candidateAt(i)->observation;
      if (obs.addressType == addressType_ && std::strncmp(obs.address, address_, sizeof(address_)) == 0) {
        index_ = i;
        return false;
      }
    }
  }
  if (index_ >= count) index_ = count - 1;
  remember(*peers.candidateAt(index_));
  return true;
}
void PeerSelection::move(int direction, const PeerTable& peers) {
  sync(peers);
  const auto count = peers.candidates();
  if (!count || direction == 0) return;
  index_ = direction < 0 ? (index_ + count - 1) % count : (index_ + 1) % count;
  remember(*peers.candidateAt(index_));
}
} // namespace stick
