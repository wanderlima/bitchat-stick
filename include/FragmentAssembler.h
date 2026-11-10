#pragma once
#include "WireCodec.h"
#include <array>

namespace stick { namespace wire {
constexpr size_t kAssemblySlots = 2, kMaxFragments = 32, kMaxFragmentChunk = 1024;
constexpr uint32_t kAssemblyTimeoutMs = 30000;
enum class AssemblyResult { Stored, Duplicate, Complete, Invalid, Conflict, Full, TooLarge, Capacity };
// Bounded framing only. Complete frames still require signature/session checks.
class FragmentAssembler {
 public:
  AssemblyResult append(const FrameView& fragment, uint32_t now, uint8_t* output, size_t capacity, size_t& written);
  void expire(uint32_t now);
  size_t active() const;
 private:
  struct Slot {
    bool used = false;
    uint8_t sender[8]{}, id[8]{}, recipient[8]{};
    bool directed = false;
    uint16_t total = 0, received = 0;
    uint8_t type = 0;
    uint32_t started = 0;
    uint64_t timestamp = 0;
    size_t bytes = 0;
    std::array<uint16_t,kMaxFragments> offsets{}, lengths{};
    std::array<uint8_t,kMaxFrameBytes> data{};
  };
  std::array<Slot,kAssemblySlots> slots_{};
};
} } // namespace stick::wire
