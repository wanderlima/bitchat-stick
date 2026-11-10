#pragma once
#include "CryptoPrimitives.h"
namespace stick { namespace wire {
bool canonical(const FrameView& frame,uint8_t* scratch,size_t capacity,size_t& written);
bool signPacket(const FrameView& frame,const uint8_t secret[64],uint8_t* scratch,size_t capacity,uint8_t signature[64]);
bool verifyPacket(const FrameView& frame,const uint8_t publicKey[32],uint8_t* scratch,size_t capacity);
// Bounded raw-DEFLATE expansion; never expose partial plaintext after failure.
bool unpackPayload(const FrameView& frame,uint8_t* output,size_t capacity,size_t& written);
} }
