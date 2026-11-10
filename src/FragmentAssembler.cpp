#include "FragmentAssembler.h"
#include <cstring>

namespace stick { namespace wire {
void FragmentAssembler::expire(uint32_t now) {
  for (auto& slot : slots_)
    if (slot.used && uint32_t(now - slot.started) >= kAssemblyTimeoutMs) slot.used = false;
}
size_t FragmentAssembler::active() const {
  size_t count = 0;for (const auto& slot : slots_) if (slot.used) ++count;return count;
}
AssemblyResult FragmentAssembler::append(const FrameView& fragment, uint32_t now,
                                         uint8_t* output, size_t capacity, size_t& written) {
  written = 0;
  expire(now);
  if (fragment.type != 0x20 || fragment.sender.size != 8 || !fragment.sender.data ||
      !fragment.payload.data || fragment.payload.size <= 13 ||
      (fragment.recipient.size && (fragment.recipient.size != 8 || !fragment.recipient.data))) return AssemblyResult::Invalid;
  const auto* payload = fragment.payload.data;
  const size_t index = (size_t(payload[8]) << 8) | payload[9];
  const size_t total = (size_t(payload[10]) << 8) | payload[11];
  const size_t chunk = fragment.payload.size - 13;
  if (!total || total > kMaxFragments || index >= total || payload[12] == 0x20) return AssemblyResult::Invalid;
  if (chunk > kMaxFragmentChunk) return AssemblyResult::TooLarge;
  Slot* target = nullptr;
  for (auto& slot : slots_)
    if (slot.used && std::memcmp(slot.sender,fragment.sender.data,8) == 0 &&
        std::memcmp(slot.id,payload,8) == 0) { target = &slot;break; }
  if (!target) {
    for (auto& slot : slots_) if (!slot.used) { target = &slot;break; }
    if (!target) return AssemblyResult::Full;
    // Clear metadata only; the bounded byte storage is overwritten before use.
    target->used = true;target->directed = fragment.recipient.size != 0;
    target->total = uint16_t(total);target->received = 0;target->bytes = 0;
    target->type = payload[12];target->started = now;target->timestamp = fragment.timestamp;
    target->lengths.fill(0);
    std::memcpy(target->sender,fragment.sender.data,8);std::memcpy(target->id,payload,8);
    if (target->directed) std::memcpy(target->recipient,fragment.recipient.data,8);
  }
  if (target->total != total || target->type != payload[12] || target->timestamp != fragment.timestamp ||
      target->directed != (fragment.recipient.size != 0) ||
      (target->directed && std::memcmp(target->recipient,fragment.recipient.data,8) != 0)) {
    target->used = false;return AssemblyResult::Conflict;
  }
  const bool duplicate = target->lengths[index] != 0;
  if (duplicate) {
    if (target->lengths[index] != chunk || std::memcmp(target->data.data()+target->offsets[index],payload+13,chunk) != 0) {
      target->used = false;return AssemblyResult::Conflict;
    }
  } else {
    if (chunk > kMaxFrameBytes - target->bytes) { target->used = false;return AssemblyResult::TooLarge; }
    target->offsets[index] = uint16_t(target->bytes);target->lengths[index] = uint16_t(chunk);
    std::memcpy(target->data.data()+target->bytes,payload+13,chunk);
    target->bytes += chunk;++target->received;
  }
  if (target->received != target->total) return duplicate ? AssemblyResult::Duplicate : AssemblyResult::Stored;
  if (!output || capacity < target->bytes) return AssemblyResult::Capacity;
  size_t offset = 0;
  for (size_t i = 0; i < total; ++i) {
    std::memcpy(output+offset,target->data.data()+target->offsets[i],target->lengths[i]);offset += target->lengths[i];
  }
  target->used = false;
  FrameView recovered;
  if (decode({output,offset},recovered) != Error::None || recovered.type != target->type ||
      std::memcmp(recovered.sender.data,target->sender,8) != 0) return AssemblyResult::Invalid;
  written = offset;
  return AssemblyResult::Complete;
}
} } // namespace stick::wire
