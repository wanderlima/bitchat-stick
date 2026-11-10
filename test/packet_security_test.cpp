#include "PacketSecurity.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace stick;
std::vector<uint8_t> bytes(const std::string& s){std::vector<uint8_t>b;for(size_t i=0;i<s.size();i+=2)b.push_back(uint8_t(std::stoul(s.substr(i,2),nullptr,16)));return b;}
int main(int argc,char** argv){
 std::ofstream exported;if(argc==2)exported.open(argv[1]);
 std::ifstream input("test/fixtures/signed-packet-vectors.txt");assert(input);std::string line;unsigned count=0;
 uint8_t scratch[wire::kMaxFrameBytes],payload[wire::kMaxPayloadBytes],seed[32],secret[64],pub[32],sig[64];
 for(size_t i=0;i<32;++i)seed[i]=uint8_t(i);crypto::signingKeys(seed,secret,pub);
 while(std::getline(input,line)){
  if(line.empty()||line[0]=='#')continue;std::istringstream row(line);std::string name,raw,pre,key;row>>name>>raw>>pre>>key;
  auto data=bytes(raw),expected=bytes(pre),publicKey=bytes(key);assert(publicKey.size()==32 && crypto::equal(pub,publicKey.data(),32));
  wire::FrameView frame;assert(wire::decode({data.data(),data.size()},frame)==wire::Error::None);
  size_t n=0;assert(wire::canonical(frame,scratch,sizeof(scratch),n) && n==expected.size() && crypto::equal(scratch,expected.data(),n));
  assert(wire::verifyPacket(frame,pub,scratch,sizeof(scratch)));
  assert(wire::signPacket(frame,secret,scratch,sizeof(scratch),sig));
  assert(crypto::verify(pub,{scratch,expected.size()},sig));
  if(exported){exported<<name<<" "<<pre<<" "<<key<<" ";for(auto c:sig){const char*digits="0123456789abcdef";exported<<digits[c>>4]<<digits[c&15];}exported<<"\n";}
  assert(wire::unpackPayload(frame,payload,sizeof(payload),n));
  if(name=="compressed"){assert(frame.compressed && n==600);for(size_t i=0;i<600;++i)assert(payload[i]=="hello "[i%6]);}
  else assert(n==5 && std::memcmp(payload,"hello",5)==0);
  frame.ttl=2;frame.isRsr=true;assert(wire::verifyPacket(frame,pub,scratch,sizeof(scratch)));
  data[frame.payload.data-data.data()]^=1;assert(!wire::verifyPacket(frame,pub,scratch,sizeof(scratch)));++count;
 }
 assert(count==3);std::puts("Packet security: official signed/compressed/routed frames, canonical padding, relay fields and tampering passed");
}
