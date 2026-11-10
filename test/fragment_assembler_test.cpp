#include "FragmentAssembler.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>
using namespace stick::wire;
uint8_t sender[]={0,1,2,3,4,5,6,7};
std::vector<uint8_t> makePayload(unsigned id,unsigned index,unsigned total,const uint8_t* bytes,size_t size,uint8_t type=2) {
  std::vector<uint8_t> result(13+size,0);result[0]=uint8_t(id);
  result[8]=uint8_t(index>>8);result[9]=uint8_t(index);
  result[10]=uint8_t(total>>8);result[11]=uint8_t(total);result[12]=type;
  if(size)std::memcpy(result.data()+13,bytes,size);return result;
}
FrameView fragment(const std::vector<uint8_t>& payload) {
  FrameView frame;frame.type=0x20;frame.sender={sender,8};frame.payload={payload.data(),payload.size()};return frame;
}
int main() {
  FragmentAssembler assembler;
  uint8_t encoded[100]{};uint8_t output[kMaxFrameBytes]{};size_t size=0,written=0;
  FrameView original;original.type=2;original.sender={sender,8};original.payload={reinterpret_cast<const uint8_t*>("hello"),5};
  assert(encode(original,encoded,sizeof(encoded),size)==Error::None);
  const size_t split=14;
  const auto first=makePayload(1,0,2,encoded,split),last=makePayload(1,1,2,encoded+split,size-split);
  assert(assembler.append(fragment(last),0,output,sizeof(output),written)==AssemblyResult::Stored);
  assert(assembler.append(fragment(last),100,output,sizeof(output),written)==AssemblyResult::Duplicate);
  assert(assembler.append(fragment(first),101,output,sizeof(output),written)==AssemblyResult::Complete);
  assert(written==size && std::memcmp(output,encoded,size)==0 && assembler.active()==0);
  assert(assembler.append(fragment(first),200,output,sizeof(output),written)==AssemblyResult::Stored);
  assert(assembler.append(fragment(last),201,output,1,written)==AssemblyResult::Capacity && written==0);
  assert(assembler.append(fragment(last),202,output,sizeof(output),written)==AssemblyResult::Complete);
  assembler.append(fragment(first),300,output,sizeof(output),written);
  auto conflict=first;conflict.back()^=1;
  assert(assembler.append(fragment(conflict),301,output,sizeof(output),written)==AssemblyResult::Conflict && assembler.active()==0);
  assembler.append(fragment(first),400,output,sizeof(output),written);
  auto wrongTotal=last;wrongTotal[11]=3;
  assert(assembler.append(fragment(wrongTotal),401,output,sizeof(output),written)==AssemblyResult::Conflict);
  auto other=first;other[0]=2;auto third=first;third[0]=3;
  assembler.append(fragment(first),500,output,sizeof(output),written);assembler.append(fragment(other),500,output,sizeof(output),written);
  assert(assembler.append(fragment(third),501,output,sizeof(output),written)==AssemblyResult::Full);
  assembler.append(fragment(first),30499,output,sizeof(output),written); // Duplicates do not extend lifetime.
  assembler.expire(30500);assert(assembler.active()==0);
  assembler.append(fragment(first),UINT32_MAX-100,output,sizeof(output),written);
  assembler.expire(100);assert(assembler.active()==1);
  assembler.expire(30000);assert(assembler.active()==0);
  const auto huge=makePayload(4,0,1,output,kMaxFragmentChunk+1);
  assert(assembler.append(fragment(huge),40000,output,sizeof(output),written)==AssemblyResult::TooLarge);
  const auto tooMany=makePayload(4,0,kMaxFragments+1,encoded,1);
  assert(assembler.append(fragment(tooMany),40000,output,sizeof(output),written)==AssemblyResult::Invalid);
  const auto nested=makePayload(4,0,1,encoded,size,0x20);
  assert(assembler.append(fragment(nested),40000,output,sizeof(output),written)==AssemblyResult::Invalid);
  auto corrupted=makePayload(5,0,1,encoded,size);corrupted[13+14]^=1; // Inner sender mismatch.
  assert(assembler.append(fragment(corrupted),40000,output,sizeof(output),written)==AssemblyResult::Invalid && written==0);
  for(unsigned i=0;i<5;++i) {
    const auto piece=makePayload(8,i,5,output,kMaxFragmentChunk);
    const auto outcome=assembler.append(fragment(piece),50000,output,sizeof(output),written);
    assert(outcome==(i==4 ? AssemblyResult::TooLarge : AssemblyResult::Stored));
  }
  assert(assembler.active()==0);
  std::puts("Fragments: reordered delivery, duplicates/conflicts, limits, capacity retry, expiry/wrap and sender binding passed");
}
