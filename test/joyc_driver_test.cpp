#include "Joyc.h"
#include <M5GFX.h>
#include <cassert>
#include <cstdio>
int main(){
  using stick::pollJoyc;
  assert(!pollJoyc(0).valid&&fake_joyc::probes==1);
  assert(!pollJoyc(999).valid&&fake_joyc::probes==1);
  fake_joyc::present=true;fake_joyc::axes[0]=128;fake_joyc::axes[1]=127;
  auto sample=pollJoyc(1000);assert(sample.valid&&sample.x==-128&&sample.y==127);
  assert(pollJoyc(1019).valid&&fake_joyc::probes==3);
  fake_joyc::axes[0]=255;fake_joyc::axes[1]=175;
  sample=pollJoyc(1020);assert(sample.valid&&sample.x==-1&&sample.y==-81);
  fake_joyc::complete=false;assert(!pollJoyc(1040).valid);
  fake_joyc::complete=true;fake_joyc::axes[0]=fake_joyc::axes[1]=0;
  assert(!pollJoyc(2039).valid);assert(pollJoyc(2040).valid);
  fake_joyc::present=false;assert(!pollJoyc(2060).valid);
  assert(fake_joyc::buttonReads==0);
  fake_joyc::present=true;fake_joyc::button=0;
  sample=pollJoyc(3060,true);assert(sample.valid&&sample.buttonValid&&sample.pressed&&fake_joyc::buttonReads==1);
  assert(stick::setJoycLed(42)&&fake_joyc::led[1]==42);
  sample=pollJoyc(3080,false);assert(sample.valid&&!sample.buttonValid&&fake_joyc::buttonReads==1);
  assert(fake_joyc::releases>=3);
  std::puts("Mini JoyC driver: signed I2C axes, bounded polling, missing/short reads and recovery passed");
}
