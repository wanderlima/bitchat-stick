#include "Identity.h"
#include <Preferences.h>
#include <cassert>
#include <cstring>
#include <cstdio>
static unsigned calls;
void entropy(void* p,size_t n) { ++calls;for(size_t i=0;i<n;++i) static_cast<uint8_t*>(p)[i]=uint8_t(i+1); }
int main() {
  stick::Identity first,again,bad;
  assert(first.begin(entropy));assert(calls==1);assert(again.begin(entropy));assert(calls==1);
  assert(!std::memcmp(first.id(),again.id(),8));assert(!std::memcmp(first.signingPublic(),again.signingPublic(),32));
  fake_preferences::identity[0]=9;assert(!bad.begin(entropy));assert(!bad.ready());assert(calls==1);
  fake_preferences::identityLength=1;assert(!bad.begin(entropy));assert(calls==1);
  fake_preferences::identityLength=0;fake_preferences::identityExists=false;
  fake_preferences::writeSucceeds=false;assert(!bad.begin(entropy));assert(!bad.ready());
  std::puts("Identity: stable persisted keys and fail-closed corrupt/unwritable storage passed");
}
