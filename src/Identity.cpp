#include "Identity.h"
#include <Preferences.h>
#include <cstring>
namespace stick {
Identity::~Identity() { crypto::wipe(noise_,sizeof(noise_)); crypto::wipe(signing_,sizeof(signing_)); }
bool Identity::begin(Entropy entropy) {
  ready_=false;
  crypto::wipe(noise_,sizeof(noise_)); crypto::wipe(signing_,sizeof(signing_));
  Preferences prefs;
  if (!entropy || !prefs.begin("bitchat-stick",false)) return false;
  uint8_t record[65]{};
  const size_t stored=prefs.getBytesLength("identity-v1");
  bool ok=false;
  if (!stored && !prefs.isKey("identity-v1")) {
    record[0]=1; entropy(record+1,64);
    ok=prefs.putBytes("identity-v1",record,sizeof(record))==sizeof(record);
  } else if (stored==sizeof(record)) {
    ok=prefs.getBytes("identity-v1",record,sizeof(record))==sizeof(record) && record[0]==1;
  }
  prefs.end();
  if (ok) {
    std::memcpy(noise_,record+1,32);
    crypto::publicDh(noise_,noisePublic_);
    crypto::signingKeys(record+33,signing_,signingPublic_);
    uint8_t digest[32];crypto::hash({noisePublic_,32},{},digest);
    std::memcpy(id_,digest,8);crypto::wipe(digest,sizeof(digest));ready_=true;
  }
  crypto::wipe(record,sizeof(record));return ready_;
}
}
