#include "WireCodec.h"
#include <array>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace stick::wire;
int main() {
  std::ifstream source("test/fixtures/bitchat-wire-vectors.txt");assert(source);
  std::string line;unsigned fixtures=0;
  std::vector<uint8_t> sample;
  while (std::getline(source,line)) {
    if(line.empty() || line[0]=='#')continue;
    const auto space=line.find(' ');assert(space!=std::string::npos);
    const auto hex=line.substr(space+1);assert(hex.size()%2==0);
    std::vector<uint8_t> bytes;
    for(size_t i=0;i<hex.size();i+=2)bytes.push_back(uint8_t(std::stoul(hex.substr(i,2),nullptr,16)));
    FrameView frame;assert(decode({bytes.data(),bytes.size()},frame)==Error::None);
    assert(frame.type==2 && frame.ttl==7 && frame.timestamp==0x0102030405060708ULL);
    assert(frame.payload.size==5 && std::memcmp(frame.payload.data,"hello",5)==0);
    assert(frame.sender.size==8 && frame.sender.data[7]==7);
    std::array<uint8_t,kMaxFrameBytes> encoded{};size_t written=0;
    assert(encode(frame,encoded.data(),encoded.size(),written)==Error::None);
    assert(written==frame.encodedSize && std::memcmp(bytes.data(),encoded.data(),written)==0);
    for(size_t size=0;size<frame.encodedSize;++size) {
      FrameView truncated;assert(decode({bytes.data(),size},truncated)!=Error::None);
      assert(truncated.sender.data==nullptr && truncated.encodedSize==0);
    }
    assert(encode(frame,encoded.data(),written-1,written)==Error::Capacity && written==0);
    sample=bytes;++fixtures;
  }
  assert(fixtures==5);
  FrameView result;
  std::array<uint8_t,8> sender{};
  std::array<uint8_t,32> target{};target.fill(0xaa);
  size_t count=99;FrameView invalidField;invalidField.sender={sender.data(),7};
  assert(encode(invalidField,target.data(),target.size(),count)==Error::InvalidField && count==0);
  for(auto byte:target)assert(byte==0xaa); // Preflight failures never partially write output.
  invalidField.sender={nullptr,8};assert(encode(invalidField,target.data(),target.size(),count)==Error::InvalidField);
  invalidField.sender={sender.data(),8};invalidField.route={sender.data(),8};
  assert(encode(invalidField,target.data(),target.size(),count)==Error::Flags);
  auto invalid=sample;invalid[0]=3;assert(decode({invalid.data(),invalid.size()},result)==Error::Version);
  invalid=sample;invalid[11]|=0x80;assert(decode({invalid.data(),invalid.size()},result)==Error::Flags);
  invalid=sample;invalid[11]|=4;assert(decode({invalid.data(),invalid.size()},result)!=Error::None);
  invalid=sample;invalid[12]=0xff;assert(decode({invalid.data(),invalid.size()},result)==Error::TooLarge);
  invalid=sample;invalid.back()=0;assert(decode({invalid.data(),invalid.size()},result)==Error::Padding);
  assert(decode({nullptr,100},result)==Error::Truncated);
  std::array<uint8_t,kMaxFrameBytes+1> fuzz{};
  assert(decode({fuzz.data(),fuzz.size()},result)==Error::TooLarge);
  uint32_t state=17;
  for(unsigned trial=0;trial<10000;++trial) {
    for(auto& b:fuzz){state=state*1664525u+1013904223u;b=uint8_t(state>>24);}
    const size_t length=state%fuzz.size();
    (void)decode({fuzz.data(),length},result);
  }
  std::puts("WireCodec: 5 official serializer fixtures, truncation, flags, bounds, padding and 10000 hostile frames passed");
}
