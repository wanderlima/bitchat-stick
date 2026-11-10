#pragma once
#include "PeerTable.h"

namespace stick {
// Radio address/type is a navigation key, not an authenticated identity.
class PeerSelection {
 public:
  bool sync(const PeerTable& peers); // True when the displayed device changes/disappears.
  void move(int direction, const PeerTable& peers);
  size_t index() const { return index_; }
 private:
  void remember(const Peer& peer);
  char address_[18]{};
  uint8_t addressType_ = 0;
  size_t index_ = 0;
  bool tracked_ = false;
};
} // namespace stick
