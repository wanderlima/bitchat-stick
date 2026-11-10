#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace stick {
constexpr size_t kPeerCapacity = 32;
constexpr uint32_t kPeerExpiryMs = 60000;
enum class Network : uint8_t { Other, Mainnet, Testnet };
struct Observation {
  char address[18]{};
  char name[25]{};
  uint8_t addressType = 0;
  int16_t rssi = 0;
  Network network = Network::Other;
  uint32_t seenAt = 0;
};
struct Peer {
  Observation observation{};
  bool used = false;
};
// Owned by the application loop. BLE callbacks communicate through a bounded queue.
class PeerTable {
 public:
  void observe(const Observation& observation);
  void expire(uint32_t now);
  size_t count() const;
  size_t candidates() const;
  const Peer* at(size_t ordinal) const;
 private:
  std::array<Peer, kPeerCapacity> peers_{};
};
} // namespace stick
