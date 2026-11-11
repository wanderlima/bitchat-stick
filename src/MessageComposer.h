#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

namespace stick {
// RAM-only draft. Recipient identity never follows a live list index.
class MessageComposer {
 public:
  static constexpr size_t capacity=192, keyCount=34;
  inline static constexpr const char* presets[]={"Hello!", "Thanks!", "Yes.", "No.", "On my way.", "I'm here.", "Please wait", "Need help.",
    "Come here.", "Follow me.", "Go ahead.", "Stop.", "Wait here.", "Turn right.", "Turn left.", "Go back.",
    "Meet here.", "Stay close.", "Over here!", "Call me.", "Are you OK?", "I'm OK.", "Need help!", "Help!",
    "SOS!", "Emergency!", "Call help!", "I'm lost.", "Be careful.", "Danger!", "I see you.", "Got it."};
  static constexpr size_t presetCount=sizeof(presets)/sizeof(presets[0]);
  void open(const uint8_t id[8],const char* name) {
    clear();std::memcpy(recipient,id,8);std::snprintf(recipientName,sizeof(recipientName),"%s",name);
  }
  void clear() { std::memset(draft,0,sizeof(draft));std::memset(recipient,0,sizeof(recipient));recipientName[0]=0;mode=0; }
  size_t length() const {return std::strlen(draft);}
  bool ready() const {for(const char* p=draft;*p;++p)if(*p!=' ')return true;return false;}
  void preset(size_t index) {if(index<presetCount)std::snprintf(draft,sizeof(draft),"%s",presets[index]);}
  char key(size_t index) const {
    constexpr const char* layouts[]={"qwertyuiopasdfghjkl.zxcvbnm,?!", "QWERTYUIOPASDFGHJKL.ZXCVBNM,?!", "1234567890@#$%&*()-_+=/:;!?.,'"};
    return index<30?layouts[mode][index]:0;
  }
  // Returns true for Done only when nonblank text exists.
  bool selectKey(size_t index) {
    const size_t size=length();
    if(index==30)mode=(mode+1)%3;
    else if(index==32){if(size)draft[size-1]=0;}
    else if(index==33)return ready();
    else if(size<capacity && index<32){draft[size]=index==31?' ':key(index);draft[size+1]=0;}
    return false;
  }
  uint8_t recipient[8]{};
  char recipientName[25]{},draft[capacity+1]{};
  uint8_t mode=0;
};
}
