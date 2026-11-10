#include "../src/Transport.cpp"
#include <cassert>
#include <cstdio>
using namespace stick;
int main(){
  auto& scanner=NimBLEDevice::scanner;scanner.scanning=true;
  assert(beginTransport());assert(!scanner.scanning);assert(transportReady());
  auto& server=NimBLEDevice::server;auto& characteristic=server.service.characteristic;
  NimBLEConnInfo info;server.connected=1;server.callback->onConnect(&server,info);
  characteristic.callback->onSubscribe(&characteristic,info,1);
  assert(transportLinks()==0);mesh().tick(1,1800000000000ULL);pollTransport(20);assert(transportLinks()==1);
  for(unsigned now=40;now<500;now+=20)pollTransport(now);
  assert(!characteristic.notifications.empty());wire::FrameView frame;
  assert(wire::decode({characteristic.notifications[0].data(),characteristic.notifications[0].size()},frame)==wire::Error::None);assert(frame.type==1);
  // Force MTU fragmentation, then independently recover and verify the complete frame.
  server.mtu=64;characteristic.notifications.clear();assert(mesh().announce());
  for(unsigned now=500;now<1500;now+=20)pollTransport(now);
  wire::FragmentAssembler assembler;uint8_t recovered[4096];size_t length=0;bool complete=false;
  for(const auto& raw:characteristic.notifications){assert(raw.size()<=61);assert(wire::decode({raw.data(),raw.size()},frame)==wire::Error::None);assert(frame.type==0x20);
    auto result=assembler.append(frame,1600,recovered,sizeof(recovered),length);if(result==wire::AssemblyResult::Complete)complete=true;}
  assert(complete&&length);assert(wire::decode({recovered,length},frame)==wire::Error::None);assert(frame.type==1);
  // Callbacks cannot directly mutate application state; floods are bounded.
  characteristic.value={1};auto errors=mesh().stats().rejected;
  for(unsigned i=0;i<20;++i)characteristic.callback->onWrite(&characteristic,info);
  assert(mesh().stats().rejected==errors);assert(transportDrops()>0);pollTransport(1700);assert(mesh().stats().rejected>errors);
  server.callback->onDisconnect(&server,info,0);pollTransport(1800);pollTransport(1820);assert(transportLinks()==0);
  std::puts("Transport: idle-GAP startup guard, queued callbacks, subscriptions, MTU fragmentation and overflow passed");
}
