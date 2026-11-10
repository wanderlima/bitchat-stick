#include "Transport.h"
#include "Mesh.h"
#include <NimBLEDevice.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>
#include <cstring>
namespace stick {
namespace {
constexpr const char* serviceUuid="F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C";
constexpr const char* characteristicUuid="A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D";
struct Event {uint8_t kind=0;uint16_t handle=0, length=0;uint8_t bytes[512]{};};
struct Link {uint16_t handle=kAnyLink;bool subscribed=false;};
struct Tx {uint16_t target=kAnyLink,excluded=kAnyLink;size_t length=0;uint8_t bytes[4096]{};};
Identity identity;
NimBLEServer* server=nullptr;NimBLECharacteristic* characteristic=nullptr;
QueueHandle_t events=nullptr;
std::atomic<uint32_t> drops{0};
Link links[2];Tx tx[6];size_t head=0,tail=0,count=0,txLink=0,txOffset=0;uint32_t lastSend=0;uint8_t fragmentBytes[512]{};bool started=false;
void entropy(void* out,size_t n){esp_fill_random(out,n);}
void queueEvent(Event& event){if(xQueueSend(events,&event,0)!=pdTRUE){++drops;server->disconnect(event.handle);}}
class ServerCallbacks:public NimBLEServerCallbacks {
 void onConnect(NimBLEServer* s,NimBLEConnInfo& info) override {
   if(s->getConnectedCount()>2){s->disconnect(info.getConnHandle());return;}
   Event event;event.kind=1;event.handle=info.getConnHandle();queueEvent(event);
 }
 void onDisconnect(NimBLEServer*,NimBLEConnInfo& info,int) override {Event event;event.kind=2;event.handle=info.getConnHandle();queueEvent(event);}
} serverCallbacks;
class CharacteristicCallbacks:public NimBLECharacteristicCallbacks {
 void onWrite(NimBLECharacteristic* c,NimBLEConnInfo& info) override {
   const auto value=c->getValue();if(!value.size()||value.size()>512){++drops;return;}
   Event event;event.kind=3;event.handle=info.getConnHandle();event.length=value.size();std::memcpy(event.bytes,value.data(),event.length);queueEvent(event);
 }
 void onSubscribe(NimBLECharacteristic*,NimBLEConnInfo& info,uint16_t value) override {Event event;event.kind=value&1?4:5;event.handle=info.getConnHandle();queueEvent(event);}
} characteristicCallbacks;
bool enqueue(wire::Bytes bytes,uint16_t target,uint16_t excluded){
 bool destination=false;for(const auto& link:links)if(link.subscribed&&link.handle!=excluded&&(target==kAnyLink||target==link.handle))destination=true;
 if(!destination||bytes.size>4096||count==6){return false;}
 auto& item=tx[tail];item.target=target;item.excluded=excluded;item.length=bytes.size;std::memcpy(item.bytes,bytes.data,bytes.size);tail=(tail+1)%6;++count;return true;
}
void drainTx(uint32_t now){
 if(!count||uint32_t(now-lastSend)<20)return;lastSend=now;
 auto& item=tx[head];
 if(txLink>=2){head=(head+1)%6;--count;txLink=txOffset=0;return;}
 auto& link=links[txLink];
 if(!link.subscribed||link.handle==item.excluded||(item.target!=kAnyLink&&item.target!=link.handle)){++txLink;txOffset=0;return;}
 const size_t mtu=server->getPeerMTU(link.handle);const size_t capacity=mtu>3?mtu-3:0;
 if(item.length<=capacity){if(!characteristic->notify(item.bytes,item.length,link.handle))++drops;++txLink;txOffset=0;return;}
 // Fragment header: wire(14+8) + stream(8)+index(2)+count(2)+type(1).
 const size_t overhead=35;
 if(capacity<=overhead){++drops;++txLink;txOffset=0;return;}
 size_t chunk=capacity-overhead;if(chunk>400)chunk=400;
 const size_t total=(item.length+chunk-1)/chunk;
 if(total>wire::kMaxFragments){++drops;++txLink;txOffset=0;return;}
 uint8_t payload[413];uint8_t digest[32];crypto::hash({item.bytes,item.length},{},digest);std::memcpy(payload,digest,8);
 size_t index=txOffset/chunk;payload[8]=uint8_t(index>>8);payload[9]=uint8_t(index);payload[10]=uint8_t(total>>8);payload[11]=uint8_t(total);payload[12]=item.bytes[1];
 size_t n=item.length-txOffset;if(n>chunk)n=chunk;std::memcpy(payload+13,item.bytes+txOffset,n);
 wire::FrameView original;if(wire::decode({item.bytes,item.length},original)!=wire::Error::None){++drops;++txLink;txOffset=0;return;}
 wire::FrameView fragment;fragment.type=0x20;fragment.ttl=original.ttl;fragment.timestamp=original.timestamp;fragment.sender=original.sender;fragment.payload={payload,n+13};
 size_t written=0;if(wire::encode(fragment,fragmentBytes,sizeof(fragmentBytes),written)!=wire::Error::None||!characteristic->notify(fragmentBytes,written,link.handle)){++drops;++txLink;txOffset=0;return;}
 txOffset+=n;if(txOffset==item.length){++txLink;txOffset=0;}
}
}
bool beginTransport(){
 // NimBLE 2.5 requires an idle GAP controller while registering GATT.
 // beginScanner() has already initialized the stack and started discovery.
 auto* scan=NimBLEDevice::getScan();
 if(scan && scan->isScanning() && !scan->stop())return false;
 events=xQueueCreate(12,sizeof(Event));if(!events||!identity.begin(entropy))return false;
 NimBLEDevice::setMTU(512);server=NimBLEDevice::createServer();if(!server)return false;
 server->setCallbacks(&serverCallbacks,false);server->advertiseOnDisconnect(true);
 auto* service=server->createService(serviceUuid);if(!service)return false;
 characteristic=service->createCharacteristic(characteristicUuid,NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE|NIMBLE_PROPERTY::WRITE_NR|NIMBLE_PROPERTY::NOTIFY,512);
 if(!characteristic)return false;characteristic->setCallbacks(&characteristicCallbacks);
 if(!server->start())return false;
 auto* advertising=NimBLEDevice::getAdvertising();advertising->addServiceUUID(serviceUuid);advertising->enableScanResponse(true);advertising->setName("Bitchat Stick");
 started=advertising->start();if(started)mesh().begin(identity,entropy,enqueue);return started;
}
void pollTransport(uint32_t now){if(!started)return;
 Event event;
 for(unsigned i=0;i<8&&xQueueReceive(events,&event,0)==pdTRUE;++i){
   Link* link=nullptr;for(auto& candidate:links)if(candidate.handle==event.handle)link=&candidate;
   if(event.kind==1){mesh().disconnected(event.handle);if(!link)for(auto& candidate:links)if(candidate.handle==kAnyLink){link=&candidate;break;}if(link){link->handle=event.handle;link->subscribed=false;}else server->disconnect(event.handle);}
   else if(event.kind==2){mesh().disconnected(event.handle);if(link)*link=Link{};}
   else if(link){if(event.kind==3)mesh().receive({event.bytes,event.length},event.handle);
     else if(event.kind==4){link->subscribed=true;mesh().announce(event.handle);}
     else if(event.kind==5){link->subscribed=false;mesh().disconnected(event.handle);}}
 }
 for(auto& link:links)if(link.handle!=kAnyLink&&server->getPeerMTU(link.handle)==0){mesh().disconnected(link.handle);link=Link{};}
 drainTx(now);
}
bool transportReady(){return started;}
size_t transportLinks(){size_t n=0;for(const auto& link:links)n+=link.subscribed;return n;}
uint32_t transportDrops(){return drops.load();}
const char* transportStatus(){if(!started)return "BLE error";if(!mesh().ready())return "Clock error";for(const auto& link:links)if(link.subscribed&&server->getPeerMTU(link.handle)<=38)return "MTU too low";return transportLinks()?"Connected":"Advertising";}
}
