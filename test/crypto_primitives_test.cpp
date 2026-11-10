#include "CryptoPrimitives.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
using namespace stick;
void unhex(const char* hex,uint8_t* out,size_t size) { for(size_t i=0;i<size;++i)out[i]=uint8_t(std::stoul(std::string(hex+i*2,2),nullptr,16)); }
int main() {
  uint8_t digest[32],expected[64],key[32]{};
  crypto::hash({reinterpret_cast<const uint8_t*>("abc"),3},{},digest);
  unhex("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",expected,32);assert(crypto::equal(digest,expected,32));
  std::memset(key,0x0b,20);crypto::hmac(key,{reinterpret_cast<const uint8_t*>("Hi There"),8},digest);
  unhex("b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7",expected,32);assert(crypto::equal(digest,expected,32));
  uint8_t seed[32],secret[64],pub[32],signature[64];
  unhex("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60",seed,32);
  crypto::signingKeys(seed,secret,pub);unhex("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a",expected,32);assert(crypto::equal(pub,expected,32));
  crypto::sign(secret,{},signature);unhex("e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b",expected,64);
  assert(crypto::equal(signature,expected,64) && crypto::verify(pub,{},signature));signature[0]^=1;assert(!crypto::verify(pub,{},signature));
  unhex("77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a",seed,32);crypto::publicDh(seed,pub);
  unhex("8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a",expected,32);assert(crypto::equal(pub,expected,32));
  uint8_t remote[32];unhex("de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f",remote,32);assert(crypto::dh(seed,remote,digest));
  unhex("4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742",expected,32);assert(crypto::equal(digest,expected,32));std::memset(remote,0,32);assert(!crypto::dh(seed,remote,digest));
  uint8_t encoded[64],plain[64];size_t written=0,opened=0;
  assert(crypto::seal(key,7,{}, {reinterpret_cast<const uint8_t*>("hello"),5},encoded,sizeof(encoded),written));
  unhex("97130e93716a10a1e082e835f16d1cda5d4da1ec78",expected,21);assert(written==21 && crypto::equal(encoded,expected,21)); // Independent Apple CryptoKit result.
  assert(crypto::open(key,7,{}, {encoded,written},plain,sizeof(plain),opened) && opened==5 && std::memcmp(plain,"hello",5)==0);
  encoded[0]^=1;assert(!crypto::open(key,7,{}, {encoded,written},plain,sizeof(plain),opened) && opened==0);
  std::puts("Crypto: SHA256/HMAC RFC4231, Ed25519 RFC8032, X25519 RFC7748, AEAD tampering and low-order rejection passed");
}
