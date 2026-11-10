#pragma once
#include <string>
#include <vector>
#include <cstdint>
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
  bool isScanning() const{return scanning;}
  void emit(const NimBLEAdvertisedDevice& device){callback->onResult(&device);}
};
struct NimBLEDevice {
  static inline NimBLEScan scanner;
  static bool init(const char*){return true;}
  static NimBLEScan* getScan(){return &scanner;}
};
