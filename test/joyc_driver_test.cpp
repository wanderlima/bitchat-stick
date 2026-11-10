#include "Joyc.h"
#include <Wire.h>
#include <cassert>
#include <cstdio>
int main(){
  using stick::pollJoyc;
  assert(!pollJoyc(0).valid&&fake_wire::probes==1);
  assert(!pollJoyc(999).valid&&fake_wire::probes==1);
  fake_wire::present=true;fake_wire::axis[0]=128;fake_wire::axis[1]=127;
  auto sample=pollJoyc(1000);assert(sample.valid&&sample.x==-128&&sample.y==127);
  assert(pollJoyc(1019).valid&&fake_wire::probes==2);
  fake_wire::axis[0]=255;fake_wire::axis[1]=175;
  sample=pollJoyc(1020);assert(sample.valid&&sample.x==-1&&sample.y==-81);
  fake_wire::responseSize=1;assert(!pollJoyc(1040).valid);
  fake_wire::responseSize=2;fake_wire::axis[0]=fake_wire::axis[1]=0;
  assert(!pollJoyc(2039).valid);assert(pollJoyc(2040).valid);
  fake_wire::present=false;assert(!pollJoyc(2060).valid);
  std::puts("Mini JoyC driver: signed I2C axes, bounded polling, missing/short reads and recovery passed");
}
