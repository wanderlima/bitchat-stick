#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cassert>
struct NimBLEUUID { std::string value; explicit NimBLEUUID(const char* s):value(s){} };
struct FakeAddress {
  std::string text="AA:BB:CC:DD:EE:FF";uint8_t type=0;
  std::string toString() const { return text; } uint8_t getType() const { return type; }
};
struct NimBLEAdvertisedDevice {
  FakeAddress address;std::string name;int rssi=-64;std::vector<std::string> services;
  FakeAddress getAddress() const { return address; }
  std::string getName() const { return name; } int getRSSI() const { return rssi; }
  bool isAdvertisingService(const NimBLEUUID& uuid) const {
    for(const auto& service:services)if(service==uuid.value)return true;
    return false;
  }
};
struct NimBLEScanCallbacks { virtual ~NimBLEScanCallbacks()=default;virtual void onResult(const NimBLEAdvertisedDevice*){} };
struct NimBLEScan {
  NimBLEScanCallbacks* callback=nullptr;bool active=false,scanning=false,startSucceeds=true;
  unsigned starts=0;
  void setScanCallbacks(NimBLEScanCallbacks* cb,bool) { callback=cb; }
  void setDuplicateFilter(bool){} void setMaxResults(unsigned){}
  void setActiveScan(bool value){active=value;} void setInterval(unsigned){} void setWindow(unsigned){}
  bool start(unsigned){++starts;scanning=startSucceeds;return scanning;}
  bool stop(){scanning=false;return true;}
  bool isScanning() const{return scanning;}
  void emit(const NimBLEAdvertisedDevice& device){callback->onResult(&device);}
};
struct NimBLEConnInfo {uint16_t handle=1;uint16_t getConnHandle()const{return handle;}};
struct NimBLEServer;
struct NimBLECharacteristic;
struct NimBLEServerCallbacks {virtual ~NimBLEServerCallbacks()=default;virtual void onConnect(NimBLEServer*,NimBLEConnInfo&){} virtual void onDisconnect(NimBLEServer*,NimBLEConnInfo&,int){} };
struct NimBLECharacteristicCallbacks {virtual ~NimBLECharacteristicCallbacks()=default;virtual void onWrite(NimBLECharacteristic*,NimBLEConnInfo&){} virtual void onSubscribe(NimBLECharacteristic*,NimBLEConnInfo&,uint16_t){} };
namespace NIMBLE_PROPERTY {constexpr unsigned READ=1,WRITE=2,WRITE_NR=4,NOTIFY=8;}
struct NimBLECharacteristic {
 NimBLECharacteristicCallbacks* callback=nullptr;std::vector<uint8_t> value;std::vector<std::vector<uint8_t>> notifications;
 void setCallbacks(NimBLECharacteristicCallbacks* c){callback=c;}
 std::vector<uint8_t> getValue()const{return value;}
 bool notify(const uint8_t* bytes,size_t n,uint16_t){notifications.emplace_back(bytes,bytes+n);return true;}
};
struct NimBLEService {NimBLECharacteristic characteristic;NimBLECharacteristic* createCharacteristic(const char*,unsigned,size_t){return &characteristic;}};
struct NimBLEAdvertising {bool active=false;unsigned starts=0;bool isAdvertising(){return active;}bool addServiceUUID(const char*){return true;}void enableScanResponse(bool){}bool setName(const char*){return true;}bool start(){++starts;active=true;return true;}};
struct NimBLEServer {
 NimBLEService service;NimBLEServerCallbacks* callback=nullptr;uint16_t mtu=185;unsigned connected=0,disconnects=0;
 unsigned getConnectedCount()const{return connected;}
 bool disconnect(uint16_t){++disconnects;return true;}
 void setCallbacks(NimBLEServerCallbacks* c,bool){callback=c;}
 void advertiseOnDisconnect(bool){}
 NimBLEService* createService(const char*){return &service;}
 bool start();uint16_t getPeerMTU(uint16_t)const{return mtu;}
};
struct NimBLEDevice {
  static inline NimBLEScan scanner;
  static inline NimBLEServer server;
  static inline NimBLEAdvertising advertising;
  static bool init(const char*){return true;}
  static NimBLEScan* getScan(){return &scanner;}
  static bool setMTU(uint16_t){return true;}
  static NimBLEServer* createServer(){return &server;}
  static NimBLEAdvertising* getAdvertising(){return &advertising;}
};
inline bool NimBLEServer::start(){assert(!NimBLEDevice::scanner.scanning);return true;}
