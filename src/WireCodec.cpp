#include "WireCodec.h"
#include <cstring>

namespace stick { namespace wire {
namespace {
bool valid(Bytes bytes) { return bytes.size == 0 || bytes.data != nullptr; }
size_t headerSize(uint8_t version) { return version == 1 ? 14 : 16; }
}
Error decode(Bytes input, FrameView& result) {
  result = {};
  if (!input.data || input.size < 22) return Error::Truncated;
  if (input.size > kMaxFrameBytes) return Error::TooLarge;
  FrameView frame;
  frame.version = input.data[0];
  if (frame.version != 1 && frame.version != 2) return Error::Version;
  const size_t header = headerSize(frame.version);
  if (input.size < header + 8) return Error::Truncated;
  frame.type = input.data[1]; frame.ttl = input.data[2];
  for (size_t i = 3; i < 11; ++i) frame.timestamp = (frame.timestamp << 8) | input.data[i];
  const uint8_t flags = input.data[11];
  if ((flags & ~0x1F) || (frame.version == 1 && (flags & 0x08))) return Error::Flags;
  frame.compressed = (flags & 0x04) != 0;
  frame.isRsr = (flags & 0x10) != 0;
  uint32_t payloadLength = 0;
  for (size_t i = 12; i < header; ++i) payloadLength = (payloadLength << 8) | input.data[i];
  if (payloadLength > kMaxPayloadBytes) return Error::TooLarge;
  size_t offset = header;
  auto take = [&](size_t count, Bytes& target) {
    if (count > input.size - offset) return false;
    target = {input.data + offset, count}; offset += count; return true;
  };
  if (!take(8, frame.sender)) return Error::Truncated;
  if ((flags & 0x01) && !take(8, frame.recipient)) return Error::Truncated;
  if (flags & 0x08) {
    if (offset == input.size) return Error::Truncated;
    const size_t count = input.data[offset++];
    if (!count) return Error::InvalidField;
    if (!take(count * 8, frame.route)) return Error::Truncated;
  }
  if (!take(payloadLength, frame.payload)) return Error::Truncated;
  if ((flags & 0x02) && !take(64, frame.signature)) return Error::Truncated;
  if (frame.compressed) {
    const size_t lengthBytes = header - 12;
    if (frame.payload.size <= lengthBytes) return Error::Compression;
    for (size_t i=0;i<lengthBytes;++i) frame.decodedPayloadSize=(frame.decodedPayloadSize<<8)|frame.payload.data[i];
    if (!frame.decodedPayloadSize || frame.decodedPayloadSize > (frame.payload.size-lengthBytes)*1032) return Error::Compression;
    if (frame.decodedPayloadSize > kMaxPayloadBytes) return Error::TooLarge;
  } else frame.decodedPayloadSize = uint32_t(frame.payload.size);
  frame.encodedSize = offset;
  const size_t padding = input.size - offset;
  if (padding > 255) return Error::Padding;
  for (; offset < input.size; ++offset)
    if (input.data[offset] != padding) return Error::Padding;
  result = frame;
  return Error::None;
}
Error encode(const FrameView& frame, uint8_t* output, size_t capacity, size_t& written) {
  written = 0;
  if (frame.version != 1 && frame.version != 2) return Error::Version;
  if (!valid(frame.sender) || frame.sender.size != 8 || !valid(frame.recipient) ||
      (frame.recipient.size && frame.recipient.size != 8) || !valid(frame.signature) ||
      (frame.signature.size && frame.signature.size != 64) || !valid(frame.route) ||
      frame.route.size % 8 || frame.route.size > 255 * 8 || !valid(frame.payload)) return Error::InvalidField;
  if (frame.version == 1 && frame.route.size) return Error::Flags;
  if (frame.payload.size > kMaxPayloadBytes) return Error::TooLarge;
  const size_t header = headerSize(frame.version);
  const size_t total = header + 8 + frame.recipient.size + frame.payload.size +
                       frame.signature.size + (frame.route.size ? 1 + frame.route.size : 0);
  if (total > kMaxFrameBytes) return Error::TooLarge;
  if (!output || capacity < total) return Error::Capacity;
  output[0] = frame.version; output[1] = frame.type; output[2] = frame.ttl;
  for (size_t i = 0; i < 8; ++i) output[3+i] = uint8_t(frame.timestamp >> (56 - i * 8));
  output[11] = (frame.recipient.size ? 0x01 : 0) | (frame.signature.size ? 0x02 : 0) |
               (frame.route.size ? 0x08 : 0) | (frame.isRsr ? 0x10 : 0) | (frame.compressed ? 0x04 : 0);
  for (size_t i = 12; i < header; ++i) output[i] = uint8_t(frame.payload.size >> ((header - i - 1) * 8));
  size_t offset = header;
  auto append = [&](Bytes value) {
    if (value.size) std::memcpy(output + offset, value.data, value.size);
    offset += value.size;
  };
  append(frame.sender); append(frame.recipient);
  if (frame.route.size) { output[offset++] = uint8_t(frame.route.size / 8); append(frame.route); }
  append(frame.payload); append(frame.signature);
  written = offset;
  return Error::None;
}
} } // namespace stick::wire
