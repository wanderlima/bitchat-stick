#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
namespace fake_wire {
inline unsigned probes=0;
inline bool present=false;
inline size_t responseSize=2;
inline int axis[2]={0,0};
}
class TwoWire {
 public:
  explicit TwoWire(int controller){assert(controller==0);}
  void setTimeOut(uint16_t timeout){assert(timeout==5);}
  bool begin(int sda,int scl,uint32_t speed){assert(sda==0&&scl==26&&speed==100000);return true;}
  void beginTransmission(uint8_t address){assert(address==0x54);++fake_wire::probes;}
  size_t write(uint8_t reg){assert(reg==0x20);return 1;}
  uint8_t endTransmission(){return fake_wire::present?0:2;}
  size_t requestFrom(uint8_t address,size_t count,bool stop){assert(address==0x54&&count==2&&stop);next_=0;return fake_wire::responseSize;}
  int read(){assert(next_<2);return fake_wire::axis[next_++];}
 private:
  unsigned next_=0;
};
