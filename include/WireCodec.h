#pragma once
#include <cstddef>
#include <cstdint>

namespace stick { namespace wire {
constexpr size_t kMaxFrameBytes = 4096;
constexpr size_t kMaxPayloadBytes = 2048;
struct Bytes {
  const uint8_t* data;
  size_t size;
  constexpr Bytes(const uint8_t* bytes = nullptr, size_t length = 0) : data(bytes), size(length) {}
};
// Non-owning views: caller keeps the backing buffer alive. Signature bytes are
// opaque here; successful framing is NOT signature verification/authentication.
struct FrameView {
  uint8_t version = 1, type = 0, ttl = 0;
  uint64_t timestamp = 0;
  bool isRsr = false;
  Bytes sender, recipient, route, payload, signature;
  size_t encodedSize = 0; // Without optional padding.
};
enum class Error { None, Truncated, TooLarge, Version, Flags, Compression, Padding, InvalidField, Capacity };
Error decode(Bytes input, FrameView& result);
// Uncompressed, unpadded output. Source views and output must not overlap.
Error encode(const FrameView& frame, uint8_t* output, size_t capacity, size_t& written);
} } // namespace stick::wire
