#include "PeerTable.h"
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace stick;
Observation observation(unsigned id, uint32_t now, uint8_t type = 0) {
  Observation o{};
  std::snprintf(o.address, sizeof(o.address), "00:00:00:00:00:%02X", id);
  o.seenAt = now; o.addressType = type;
  return o;
}
int main() {
  PeerTable table;
  auto first = observation(0, 1);
  first.network = Network::Mainnet;
  table.observe(first);
  first.rssi = -71; first.seenAt = 2;
  table.observe(first);
  assert(table.count() == 1 && table.candidates() == 1);
  assert(table.at(0)->observation.rssi == -71);
  table.observe(observation(0, 3, 1)); // same address, different BLE type.
  assert(table.count() == 2);
  for (unsigned i = 1; i <= 40; ++i) table.observe(observation(i, 10 + i));
  assert(table.count() == kPeerCapacity);
  assert(table.candidates() == 0); // least recently observed candidate evicted.
  assert(table.at(kPeerCapacity) == nullptr);
  table.expire(60060);
  assert(table.count() == 0);
  table.observe(observation(1, UINT32_MAX - 100));
  table.expire(100);
  assert(table.count() == 1); // wrap is not expiry.
  table.expire(60000);
  assert(table.count() == 0);
  auto unterminated = observation(2, 60001);
  std::memset(unterminated.name, 'X', sizeof(unterminated.name));
  table.observe(unterminated);
  assert(table.at(0)->observation.name[sizeof(unterminated.name) - 1] == '\0');
  table.expire(120000);
  assert(table.count() == 1); // 59,999 ms: still fresh.
  table.expire(120001);
  assert(table.count() == 0); // exact expiry threshold.
  std::puts("PeerTable: updates, address types, capacity, expiry and wrap passed");
}
