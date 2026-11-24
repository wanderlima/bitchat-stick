#include "Joyc.h"
#include <M5GFX.h>
#include <cassert>
#include <cstdio>
int main(){
  using stick::pollJoyc;
  assert(!pollJoyc(0).valid&&fake_joyc::probes==2);
  assert(!pollJoyc(999).valid&&fake_joyc::probes==2);
  fake_joyc::present=true;fake_joyc::axes[0]=128;fake_joyc::axes[1]=127;
  auto sample=pollJoyc(1000);assert(sample.valid&&sample.x==-127&&sample.y==127);
  assert(pollJoyc(1019).valid&&fake_joyc::probes==5);
  fake_joyc::axes[0]=255;fake_joyc::axes[1]=175;
  sample=pollJoyc(1020);assert(sample.valid&&sample.x==81&&sample.y==1);
  fake_joyc::axes[0]=81;fake_joyc::axes[1]=81;
  sample=pollJoyc(1040);assert(sample.x==-81&&sample.y==-81);
  fake_joyc::axes[0]=175;fake_joyc::axes[1]=175;
  sample=pollJoyc(1060);assert(sample.x==81&&sample.y==81);
  fake_joyc::complete=false;assert(!pollJoyc(1080).valid);
  fake_joyc::complete=true;fake_joyc::axes[0]=fake_joyc::axes[1]=0;
  assert(!pollJoyc(2079).valid);assert(pollJoyc(2080).valid);
  fake_joyc::present=false;assert(!pollJoyc(2100).valid);
  assert(fake_joyc::buttonReads==0);
  fake_joyc::present=true;fake_joyc::button=0;
  sample=pollJoyc(3100,true);assert(sample.valid&&sample.buttonValid&&sample.pressed&&fake_joyc::buttonReads==1);
  assert(stick::setJoycLed(42)&&fake_joyc::led[1]==42);
  sample=pollJoyc(3120,false);assert(sample.valid&&!sample.buttonValid&&fake_joyc::buttonReads==1);
  fake_joyc::present=false;assert(!pollJoyc(3140).valid);
  fake_joyc::present=true;fake_joyc::hardwareAddress=0x38;fake_joyc::axes[0]=100;fake_joyc::axes[1]=180;fake_joyc::button=0x10;
  sample=pollJoyc(4140,true);assert(sample.valid&&sample.x==-80&&sample.y==0&&sample.pressed);
  assert(stick::setJoycLed(0));
  assert(fake_joyc::releases>=3);
  std::puts("Mini JoyC driver: signed I2C axes, bounded polling, missing/short reads and recovery passed");
}
