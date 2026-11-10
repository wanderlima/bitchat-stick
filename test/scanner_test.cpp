#include "Scanner.h"
#include <NimBLEDevice.h>
#include <Arduino.h>
#include <cassert>
#include <cstdio>
#include <cstring>
int main() {
  using namespace stick;
  PeerTable peers;assert(beginScanner());auto& scanner=NimBLEDevice::scanner;
  assert(scannerActive() && !scannerFault() && !scanner.active);
  NimBLEAdvertisedDevice device;device.name="Not Bitchat";
  for(unsigned i=0;i<100;++i)scanner.emit(device);
  pollScanner(peers,0);assert(peers.count()==0 && scannerReports()==100 && scannerDrops()==0);
  device.services={"F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C"};
  device.name="Phone\n\xC3\xA9";device.address.type=1;fakeMillis=50;scanner.emit(device);pollScanner(peers,50);
  assert(peers.candidates()==1 && peers.candidateAt(0)->observation.network==Network::Mainnet);
  const auto& observed=peers.candidateAt(0)->observation;
  assert(std::strcmp(observed.name,"Phone???")==0 && observed.addressType==1 && observed.rssi==-64 && observed.seenAt==50);
  device.services={"F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5A"};
  device.address.text="11:22:33:44:55:66";device.name=std::string(100,'X');scanner.emit(device);pollScanner(peers,50);
  assert(peers.candidateAt(1)->observation.network==Network::Testnet);
  assert(std::strlen(peers.candidateAt(1)->observation.name)==24);
  for(unsigned i=0;i<40;++i)scanner.emit(device);
  assert(scannerDrops()==8);pollScanner(peers,50);assert(scannerReports()==142 && scannerDrops()==8);
  scanner.scanning=false;scanner.startSucceeds=false;pollScanner(peers,7000);
  assert(scannerFault() && !scannerActive());
  scanner.startSucceeds=true;pollScanner(peers,14000);assert(!scannerFault() && scannerActive());
  pollScanner(peers,60050);assert(peers.candidates()==0);
  std::puts("Scanner: actual callback filtering, both UUIDs, sanitization, queue bounds, counters and restart passed");
}
