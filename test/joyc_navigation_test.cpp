#include "../src/JoycNavigation.h"
#include <cassert>
#include <cstdio>
using stick::JoycNavigation;
using A=stick::NavigationAction;
int main(){
  JoycNavigation j;
  auto input=[&](int16_t x,int16_t y,uint32_t now=0,bool sleeping=false,bool valid=true,bool busy=false){return j.update(x,y,valid,now,sleeping,busy);};
  assert(input(127,0)==A::None); // No boot-held confirmation.
  assert(input(0,0)==A::None);
  for(int v=-80;v<=80;++v){assert(input(v,0)==A::None);assert(input(0,v)==A::None);}
  assert(input(81,0)==A::Select);assert(input(127,0,1500)==A::None);assert(input(127,0,31000,true)==A::None);
  assert(input(-127,0,1600)==A::None); // Left cannot follow right without centering.
  assert(input(11,0)==A::None);assert(input(-81,0)==A::None);
  assert(input(10,-10)==A::None);assert(input(-81,0)==A::Back);
  assert(input(-127,0,10000)==A::None);assert(input(0,11)==A::None);assert(input(81,0)==A::None);
  input(0,0);assert(input(0,81,100)==A::Down);
  assert(input(0,127,1599)==A::None);assert(input(0,127,1600)==A::Down);
  assert(input(0,127,3099)==A::None);assert(input(0,127,3100)==A::Down);
  assert(input(0,79,3200)==A::None);assert(input(0,-81,3300)==A::Up);
  assert(input(0,-127,4799)==A::None);assert(input(0,-127,4800)==A::Up);
  input(0,0);assert(input(100,100)==A::None);assert(input(-100,-100)==A::None);
  assert(input(90,-110,5000)==A::Up);input(0,0);assert(input(110,-90)==A::Select);
  input(0,0);assert(input(-90,0,100,true)==A::Wake);
  assert(input(-90,0,200)==A::None);input(0,0);assert(input(-90,0,300)==A::Back);
  input(0,0);assert(input(90,0,400,false,false)==A::None);
  assert(input(90,0,500)==A::None);input(0,0);assert(input(90,0,600)==A::Select);
  input(0,0);assert(input(90,0,700,false,true,true)==A::None);
  assert(input(90,0,800)==A::None);input(0,0);assert(input(90,0,900)==A::Select);
  input(0,0);assert(input(0,100,UINT32_MAX-1000)==A::Down);
  assert(input(0,100,498)==A::None);assert(input(0,100,499)==A::Down);
  std::puts("JoyC navigation: strict thresholds, center latch, 1500ms repeat, diagonals, wake, disconnect, buttons and wrap passed");
}
