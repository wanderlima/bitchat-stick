#include "NoiseSession.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace stick;
std::vector<uint8_t> unhex(const std::string& s) {std::vector<uint8_t> out;for(size_t i=0;i<s.size();i+=2)out.push_back(uint8_t(std::stoul(s.substr(i,2),nullptr,16)));return out;}
int main() {
  std::ifstream file("test/fixtures/noise-java-vectors.txt");assert(file);std::map<std::string,std::vector<uint8_t>> vectors;
  std::string line;while(std::getline(file,line)){if(line.empty() || line[0]=='#')continue;const auto p=line.find(' ');vectors[line.substr(0,p)]=unhex(line.substr(p+1));}
  uint8_t sa[32],sb[32],ea[32],eb[32],pa[32],pb[32],ida[32],idb[32];
  for(size_t i=0;i<32;++i){sa[i]=uint8_t(i+1);sb[i]=uint8_t(i+33);ea[i]=uint8_t(i+65);eb[i]=uint8_t(i+97);}
  crypto::publicDh(sa,pa);crypto::publicDh(sb,pb);crypto::hash({pa,32},{},ida);crypto::hash({pb,32},{},idb);
  NoiseSession a,b;uint8_t m1[128],m2[128],m3[128],output[128];size_t n1=0,n2=0,n3=0,n=0;
  assert(a.begin(true,sa,ea,idb,m1,sizeof(m1),n1) && b.begin(false,sb,eb,ida,nullptr,0,n));
  assert(n1==vectors["m1"].size() && std::memcmp(m1,vectors["m1"].data(),n1)==0);
  assert(b.receive({m1,n1},m2,sizeof(m2),n2));assert(n2==96 && std::memcmp(m2,vectors["m2"].data(),96)==0);
  assert(a.receive({m2,n2},m3,sizeof(m3),n3));assert(n3==64 && std::memcmp(m3,vectors["m3"].data(),64)==0);
  assert(b.receive({m3,n3},nullptr,0,n) && a.ready() && b.ready());
  assert(crypto::equal(a.handshakeHash(),vectors["hash"].data(),32) && crypto::equal(b.remoteKey(),pa,32));
  const wire::Bytes text{reinterpret_cast<const uint8_t*>("hello"),5};
  assert(a.encrypt(text,m1,sizeof(m1),n1));assert(n1==vectors["txi"].size() && std::memcmp(m1,vectors["txi"].data(),n1)==0);
  assert(b.decrypt({m1,n1},output,sizeof(output),n) && n==5 && std::memcmp(output,"hello",5)==0);
  assert(!b.decrypt({m1,n1},output,sizeof(output),n)); // Replay.
  assert(b.encrypt(text,m2,sizeof(m2),n2));assert(std::memcmp(m2,vectors["txr"].data(),n2)==0);
  assert(a.decrypt({m2,n2},output,sizeof(output),n));
  assert(a.encrypt(text,m2,sizeof(m2),n2) && a.encrypt(text,m3,sizeof(m3),n3));
  assert(b.decrypt({m3,n3},output,sizeof(output),n));assert(b.decrypt({m2,n2},output,sizeof(output),n)); // Out of order.
  assert(a.encrypt(text,m2,sizeof(m2),n2));m2[5]^=1;assert(!b.decrypt({m2,n2},output,sizeof(output),n));m2[5]^=1;assert(b.decrypt({m2,n2},output,sizeof(output),n));
  NoiseSession bad;uint8_t wrong[8]{};
  assert(bad.begin(true,sa,ea,wrong,m1,sizeof(m1),n1));const auto& response=vectors["m2"];
  assert(!bad.receive({response.data(),response.size()},m3,sizeof(m3),n3) && bad.state()==NoiseSession::State::Failed && bad.remoteKey()==nullptr);
  assert(a.begin(true,sa,ea,idb,m1,sizeof(m1),n1));auto damaged=response;damaged[45]^=1;
  assert(!a.receive({damaged.data(),damaged.size()},m3,sizeof(m3),n3));
  std::puts("NoiseXX: byte-for-byte Android handshake/hash/transport, identity binding, replay, ordering and tampering passed");
}
