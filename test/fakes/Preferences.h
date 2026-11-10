#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
namespace fake_preferences {
inline bool readable = true, writable = true, writeSucceeds = true;
inline uint32_t record = 0;
inline unsigned writes = 0, closes = 0;
}
class Preferences {
 public:
  bool begin(const char* name, bool readOnly = false) {
    assert(std::strcmp(name, "bitchat-stick") == 0);
    return readOnly ? fake_preferences::readable : fake_preferences::writable;
  }
  uint32_t getUInt(const char* key, uint32_t fallback) {
    assert(std::strcmp(key, "ui-v1") == 0);
    return fake_preferences::record ? fake_preferences::record : fallback;
  }
  size_t putUInt(const char* key, uint32_t value) {
    assert(std::strcmp(key, "ui-v1") == 0);
    ++fake_preferences::writes;
    if (!fake_preferences::writeSucceeds) return 0;
    fake_preferences::record = value;
    return sizeof(value);
  }
  void end() { ++fake_preferences::closes; }
};
