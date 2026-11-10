#include "CryptoPrimitives.h"
#include <monocypher.h>
#include <optional/monocypher-ed25519.h>
#include <cstring>
#ifdef ARDUINO
#include <mbedtls/sha256.h>
#else
#include <CommonCrypto/CommonDigest.h>
#endif
namespace stick { namespace crypto {
void wipe(void* data, size_t size) { crypto_wipe(data,size); }
bool equal(const uint8_t* a,const uint8_t* b,size_t size) {
  uint8_t different=0;for(size_t i=0;i<size;++i)different|=a[i]^b[i];return different==0;
}
void hash(wire::Bytes first,wire::Bytes second,uint8_t output[32]) {
#ifdef ARDUINO
  mbedtls_sha256_context ctx;mbedtls_sha256_init(&ctx);mbedtls_sha256_starts_ret(&ctx,0);
  if(first.size)mbedtls_sha256_update_ret(&ctx,first.data,first.size);
  if(second.size)mbedtls_sha256_update_ret(&ctx,second.data,second.size);
  mbedtls_sha256_finish_ret(&ctx,output);mbedtls_sha256_free(&ctx);
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  CC_SHA256_CTX ctx;CC_SHA256_Init(&ctx);
  if(first.size)CC_SHA256_Update(&ctx,first.data,CC_LONG(first.size));
  if(second.size)CC_SHA256_Update(&ctx,second.data,CC_LONG(second.size));
  CC_SHA256_Final(output,&ctx);wipe(&ctx,sizeof(ctx));
#pragma clang diagnostic pop
#endif
}
void hmac(const uint8_t key[32],wire::Bytes input,uint8_t output[32]) {
  uint8_t pad[64],inner[32];
  for(size_t i=0;i<64;++i)pad[i]=uint8_t((i<32?key[i]:0)^0x36);
  hash({pad,64},input,inner);
  for(size_t i=0;i<64;++i)pad[i]=uint8_t((i<32?key[i]:0)^0x5c);
  hash({pad,64},{inner,32},output);wipe(pad,64);wipe(inner,32);
}
void hkdf2(const uint8_t salt[32],wire::Bytes input,uint8_t first[32],uint8_t second[32]) {
  uint8_t temp[32],next[33];hmac(salt,input,temp);next[0]=1;hmac(temp,{next,1},first);
  std::memcpy(next,first,32);next[32]=2;hmac(temp,{next,33},second);wipe(temp,32);wipe(next,33);
}
void publicDh(const uint8_t secret[32],uint8_t output[32]) { crypto_x25519_public_key(output,secret); }
bool dh(const uint8_t secret[32],const uint8_t remote[32],uint8_t output[32]) {
  crypto_x25519(output,secret,remote);uint8_t zero[32]{};
  if(equal(output,zero,32)){wipe(output,32);return false;}return true;
}
void signingKeys(const uint8_t seed[32],uint8_t secret[64],uint8_t publicKey[32]) {
  uint8_t copy[32];std::memcpy(copy,seed,32);crypto_ed25519_key_pair(secret,publicKey,copy);wipe(copy,32);
}
void sign(const uint8_t secret[64],wire::Bytes input,uint8_t signature[64]) { crypto_ed25519_sign(signature,secret,input.data,input.size); }
bool verify(const uint8_t publicKey[32],wire::Bytes input,const uint8_t signature[64]) { return crypto_ed25519_check(signature,publicKey,input.data,input.size)==0; }
static void initAead(crypto_aead_ctx& ctx,const uint8_t key[32],uint64_t nonce) {
  uint8_t bytes[12]{};for(size_t i=0;i<8;++i)bytes[4+i]=uint8_t(nonce>>(i*8));crypto_aead_init_ietf(&ctx,key,bytes);
}
bool seal(const uint8_t key[32],uint64_t nonce,wire::Bytes aad,wire::Bytes plain,uint8_t* out,size_t capacity,size_t& written) {
  written=0;if(!out || plain.size>wire::kMaxPayloadBytes || capacity<plain.size+16 || (plain.size&&!plain.data) || (aad.size&&!aad.data))return false;
  crypto_aead_ctx ctx;initAead(ctx,key,nonce);crypto_aead_write(&ctx,out,out+plain.size,aad.data,aad.size,plain.data,plain.size);wipe(&ctx,sizeof(ctx));written=plain.size+16;return true;
}
bool open(const uint8_t key[32],uint64_t nonce,wire::Bytes aad,wire::Bytes cipher,uint8_t* out,size_t capacity,size_t& written) {
  written=0;if(!out || !cipher.data || cipher.size<16 || cipher.size>wire::kMaxPayloadBytes+16 || capacity<cipher.size-16 || (aad.size&&!aad.data))return false;
  const size_t size=cipher.size-16;crypto_aead_ctx ctx;initAead(ctx,key,nonce);
  const bool ok=crypto_aead_read(&ctx,out,cipher.data+size,aad.data,aad.size,cipher.data,size)==0;wipe(&ctx,sizeof(ctx));
  if(!ok){wipe(out,size);return false;}written=size;return true;
}
} }
