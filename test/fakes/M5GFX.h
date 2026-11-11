#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
namespace fake_joyc {
inline unsigned probes=0, releases=0, buttonReads=0;
inline bool present=false, complete=true;
inline uint8_t hardwareAddress=0x54;
inline uint8_t axes[2]={0,0}, button=1, selected=0;
inline uint8_t led[3]={0,0,0};
}
namespace lgfx { namespace i2c {
struct Result {bool ok;bool has_value() const{return ok;}};
inline Result init(int bus,int sda,int scl){assert(bus==-1&&sda==0&&scl==26);return {true};}
inline Result release(int bus){assert(bus==-1);++fake_joyc::releases;return {true};}
inline Result transactionWrite(int bus,int address,const uint8_t* data,uint8_t length,uint32_t frequency){
  assert(bus==-1&&address==0x54&&frequency==100000);++fake_joyc::probes;
  fake_joyc::selected=data[0];
  if(length==4){assert(data[0]==0x40);for(unsigned i=0;i<3;++i)fake_joyc::led[i]=data[i+1];}
  else assert(length==1);
  return {fake_joyc::present};
}
inline Result transactionRead(int bus,int address,uint8_t* out,uint8_t length,uint32_t frequency){
  assert(bus==-1&&address==0x54&&frequency==100000);
  if(!fake_joyc::present||!fake_joyc::complete)return {false};
  if(fake_joyc::selected==0x20||fake_joyc::selected==0x21){assert(length==1);out[0]=fake_joyc::axes[fake_joyc::selected-0x20];}
  else {assert(fake_joyc::selected==0x30&&length==1);++fake_joyc::buttonReads;out[0]=fake_joyc::button;}
  return {true};
}
} }
