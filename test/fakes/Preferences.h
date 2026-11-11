#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
namespace fake_preferences {
inline bool readable = true, writable = true, writeSucceeds = true;
inline uint32_t record = 0, flagsRecord = 0;
inline uint8_t identity[65]{};
inline size_t identityLength=0;
inline bool identityExists=false;
inline unsigned writes = 0, closes = 0;
}
class Preferences {
 public:
  bool begin(const char* name, bool readOnly = false) {
    assert(std::strcmp(name, "bitchat-stick") == 0);
    return readOnly ? fake_preferences::readable : fake_preferences::writable;
  }
  uint32_t getUInt(const char* key, uint32_t fallback) {
    if (std::strcmp(key, "opts-v1") == 0) return fake_preferences::flagsRecord ? fake_preferences::flagsRecord : fallback;
    assert(std::strcmp(key, "ui-v1") == 0);
    return fake_preferences::record ? fake_preferences::record : fallback;
  }
  size_t putUInt(const char* key, uint32_t value) {
    assert(std::strcmp(key, "ui-v1") == 0 || std::strcmp(key, "opts-v1") == 0);
    ++fake_preferences::writes;
    if (!fake_preferences::writeSucceeds) return 0;
    if (std::strcmp(key, "opts-v1") == 0) fake_preferences::flagsRecord = value;
    else fake_preferences::record = value;
    return sizeof(value);
  }
  bool isKey(const char*) { return fake_preferences::identityExists; }
  size_t getBytesLength(const char*) { return fake_preferences::identityLength; }
  size_t getBytes(const char*, void* out, size_t n) { std::memcpy(out,fake_preferences::identity,n);return n; }
  size_t putBytes(const char*, const void* data, size_t n) {
    if (!fake_preferences::writeSucceeds) return 0;
    assert(n==65);std::memcpy(fake_preferences::identity,data,n);
    fake_preferences::identityLength=n;fake_preferences::identityExists=true;return n;
  }
  void end() { ++fake_preferences::closes; }
};
