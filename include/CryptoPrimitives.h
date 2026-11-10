#pragma once
#include "WireCodec.h"
namespace stick { namespace crypto {
void hash(wire::Bytes first, wire::Bytes second, uint8_t output[32]);
void hmac(const uint8_t key[32], wire::Bytes input, uint8_t output[32]);
void hkdf2(const uint8_t salt[32], wire::Bytes input, uint8_t first[32], uint8_t second[32]);
void publicDh(const uint8_t secret[32], uint8_t output[32]);
bool dh(const uint8_t secret[32], const uint8_t remote[32], uint8_t output[32]);
void signingKeys(const uint8_t seed[32], uint8_t secret[64], uint8_t publicKey[32]);
void sign(const uint8_t secret[64], wire::Bytes input, uint8_t signature[64]);
bool verify(const uint8_t publicKey[32], wire::Bytes input, const uint8_t signature[64]);
bool seal(const uint8_t key[32], uint64_t nonce, wire::Bytes aad, wire::Bytes plain, uint8_t* out, size_t capacity, size_t& written);
bool open(const uint8_t key[32], uint64_t nonce, wire::Bytes aad, wire::Bytes cipher, uint8_t* out, size_t capacity, size_t& written);
void wipe(void* data, size_t size);
bool equal(const uint8_t* a, const uint8_t* b, size_t size);
} }
