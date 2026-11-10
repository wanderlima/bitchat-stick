#include "PeerSelection.h"
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace stick;
Observation make(unsigned id, uint32_t at, uint8_t type=0, Network network=Network::Mainnet) {
  Observation o{};std::snprintf(o.address,sizeof(o.address),"AA:BB:CC:DD:EE:%02X",id);
  o.addressType=type;o.seenAt=at;o.network=network;return o;
}
int main() {
  PeerTable peers;PeerSelection selected;
  assert(!selected.sync(peers));selected.move(-1,peers);assert(selected.index()==0);
  peers.observe(make(1,0));peers.observe(make(2,1000));peers.observe(make(3,1000));
  assert(selected.sync(peers));selected.move(1,peers);
  assert(selected.index()==1);
  peers.expire(60000); // Earlier slot expires; selected device remains fresh.
  assert(!selected.sync(peers) && selected.index()==0);
  peers.observe(make(4,60000)); // Hole is reused before the selected device.
  assert(!selected.sync(peers) && selected.index()==1);
  peers.observe(make(2,60000)); // Same radio changes its RSSI/name, not selection.
  assert(!selected.sync(peers) && selected.index()==1);
  peers.observe(make(2,60000,1)); // Same MAC, different address type is a separate device.
  assert(!selected.sync(peers));selected.move(-1,peers);assert(selected.index()==0);
  selected.move(-1,peers);assert(selected.index()==3);selected.move(1,peers);assert(selected.index()==0);
  peers.observe(make(4,60000,0,Network::Other)); // Selected candidate becomes ineligible.
  assert(selected.sync(peers));assert(peers.candidateAt(selected.index())->observation.addressType==0);
  peers.expire(120000);assert(selected.sync(peers) && selected.index()==0);
  peers.observe(make(8,120001));assert(selected.sync(peers) && selected.index()==0);
  std::puts("PeerSelection: expiry, slot reuse, identity/type, filtering, fallback and wrap passed");
}
