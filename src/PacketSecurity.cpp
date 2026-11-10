#include "PacketSecurity.h"
#include <cstring>
#include <initializer_list>
extern "C" size_t lgfx_tinfl_decompress_mem_to_mem(void*,size_t,const void*,size_t,int);
namespace stick { namespace wire {
bool canonical(const FrameView& frame,uint8_t* scratch,size_t capacity,size_t& written) {
  FrameView unsignedFrame=frame;unsignedFrame.ttl=0;unsignedFrame.signature={};unsignedFrame.isRsr=false;
  written=0;size_t size=0;
  if(encode(unsignedFrame,scratch,capacity,size)!=Error::None)return false;
  size_t target=size;
  for(size_t bucket: {size_t(256),size_t(512),size_t(1024),size_t(2048)})
    if(size+16<=bucket){target=bucket;break;}
  const size_t padding=target-size;
  if(padding && padding<=255) {
    if(target>capacity)return false;
    std::memset(scratch+size,int(padding),padding);size=target;
  }
  written=size;return true;
}
bool signPacket(const FrameView& frame,const uint8_t secret[64],uint8_t* scratch,size_t capacity,uint8_t signature[64]) {
  size_t size=0;if(!canonical(frame,scratch,capacity,size))return false;
  crypto::sign(secret,{scratch,size},signature);return true;
}
bool verifyPacket(const FrameView& frame,const uint8_t publicKey[32],uint8_t* scratch,size_t capacity) {
  if(frame.signature.size!=64 || !frame.signature.data)return false;
  size_t size=0;if(!canonical(frame,scratch,capacity,size))return false;
  return crypto::verify(publicKey,{scratch,size},frame.signature.data);
}
bool unpackPayload(const FrameView& frame,uint8_t* output,size_t capacity,size_t& written) {
  written=0;if(!output || (frame.payload.size&&!frame.payload.data))return false;
  if(!frame.compressed) {
    if(frame.payload.size>capacity || frame.payload.size>kMaxPayloadBytes)return false;
    if(frame.payload.size)std::memcpy(output,frame.payload.data,frame.payload.size);
    written=frame.payload.size;return true;
  }
  const size_t prefix=frame.version==1?2:4;
  if(frame.payload.size<=prefix || !frame.decodedPayloadSize || frame.decodedPayloadSize>kMaxPayloadBytes || capacity<frame.decodedPayloadSize)return false;
  const auto result=lgfx_tinfl_decompress_mem_to_mem(output,frame.decodedPayloadSize,frame.payload.data+prefix,frame.payload.size-prefix,0);
  if(result!=frame.decodedPayloadSize){crypto::wipe(output,frame.decodedPayloadSize);return false;}
  written=result;return true;
}
} }
