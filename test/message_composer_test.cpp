#include "../src/MessageComposer.h"
#include <cassert>
#include <cstdio>
using namespace stick;
int main() {
  MessageComposer c;uint8_t id[8]={1,2,3};c.open(id,"A long recipient name that is clipped");
  id[0]=9;assert(c.recipient[0]==1&&c.recipientName[24]==0&&!c.ready());
  c.selectKey(31);assert(!c.ready()&&!c.selectKey(33));c.selectKey(32);assert(c.length()==0);
  c.selectKey(0);assert(c.draft[0]=='q');c.selectKey(30);c.selectKey(0);assert(c.draft[1]=='Q');
  c.selectKey(30);c.selectKey(0);assert(c.draft[2]=='1');c.selectKey(30);assert(c.mode==0);
  for(unsigned mode=0;mode<3;++mode){for(size_t i=0;i<30;++i)assert(c.key(i)>=33&&c.key(i)<=126);c.selectKey(30);}
  for(unsigned i=0;i<300;++i)c.selectKey(0);assert(c.length()==192&&c.selectKey(33));
  for(unsigned i=0;i<300;++i)c.selectKey(32);assert(c.length()==0);
  for(size_t i=0;i<c.presetCount;++i){c.preset(i);assert(c.ready()&&c.length()<=11);}
  assert(c.presetCount==32);
  bool right=false,left=false,help=false,sos=false,come=false;
  for(const auto* text:c.presets){right|=std::strcmp(text,"Turn right.")==0;left|=std::strcmp(text,"Turn left.")==0;help|=std::strcmp(text,"Help!")==0;sos|=std::strcmp(text,"SOS!")==0;come|=std::strcmp(text,"Come here.")==0;}
  assert(right&&left&&help&&sos&&come);
  c.clear();assert(!c.ready()&&c.recipient[0]==0&&c.recipientName[0]==0);
  std::puts("Composer: stable recipient, presets, QWERTY modes, blank input and buffer limits passed");
}
