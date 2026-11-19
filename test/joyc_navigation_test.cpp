#include "../src/JoycNavigation.h"
#include <cassert>
#include <cstdio>
using stick::JoycNavigation;
using A=stick::NavigationAction;
int main(){
  JoycNavigation j;
  auto input=[&](int16_t x,int16_t y,uint32_t now=0,bool sleeping=false,bool valid=true,bool busy=false,bool grid=false){return j.update(x,y,valid,now,sleeping,busy,grid);};
  assert(input(127,0)==A::None); // No boot-held confirmation.
  assert(input(0,0)==A::None);
  for(int v=-80;v<=80;++v){assert(input(v,0)==A::None);assert(input(0,v)==A::None);}
  assert(input(81,0)==A::Select);assert(input(127,0,1500)==A::None);assert(input(127,0,31000,true)==A::None);
  assert(input(-127,0,1600)==A::None); // Left cannot follow right without centering.
  assert(input(11,0)==A::None);assert(input(-81,0)==A::None);
  assert(input(10,-10)==A::None);assert(input(-81,0)==A::Back);
  assert(input(-127,0,10000)==A::None);assert(input(0,11)==A::None);assert(input(81,0)==A::None);
  input(0,0);assert(input(0,81,100)==A::Down);
  assert(input(0,127,599)==A::None);assert(input(0,127,600)==A::Down);
  assert(input(0,127,1099)==A::None);assert(input(0,127,1100)==A::Down);
  assert(input(0,79,1200)==A::None);assert(input(0,-81,1300)==A::Up);
  assert(input(0,-127,1799)==A::None);assert(input(0,-127,1800)==A::Up);
  input(0,0);assert(input(100,100)==A::None);assert(input(-100,-100)==A::None);
  assert(input(90,-110,5000)==A::Up);input(0,0);assert(input(110,-90)==A::Select);
  input(0,0);assert(input(-90,0,100,true)==A::Wake);
  assert(input(-90,0,200)==A::None);input(0,0);assert(input(-90,0,300)==A::Back);
  input(0,0);assert(input(90,0,400,false,false)==A::None);
  assert(input(90,0,500)==A::None);input(0,0);assert(input(90,0,600)==A::Select);
  input(0,0);assert(input(90,0,700,false,true,true)==A::None);
  assert(input(90,0,800)==A::None);input(0,0);assert(input(90,0,900)==A::Select);
  input(0,0);assert(input(0,100,UINT32_MAX-100)==A::Down);
  assert(input(0,100,398)==A::None);assert(input(0,100,399)==A::Down);
  JoycNavigation keys;
  auto grid=[&](int16_t x,int16_t y,uint32_t now=0,bool sleeping=false){return keys.update(x,y,true,now,sleeping,false,true);};
  assert(grid(90,0)==A::None);assert(grid(0,0)==A::None);
  assert(grid(90,0,10)==A::Right);assert(grid(120,0,509)==A::None);assert(grid(120,0,510)==A::Right);
  assert(grid(-90,0,520)==A::Left);assert(grid(-120,0,1019)==A::None);assert(grid(-120,0,1020)==A::Left);
  assert(grid(0,-90,1030)==A::Up);assert(grid(0,90,1040)==A::Down);
  assert(grid(90,0,2000,true)==A::Wake);assert(grid(90,0,2100)==A::None);
  assert(grid(0,0,2200)==A::None);assert(grid(90,0,2300)==A::Right);
  stick::JoycClickInput click;
  assert(click.update(true,true,false,0,false,false)==A::None);
  assert(click.update(true,true,true,100,false,false)==A::None);
  assert(click.update(true,true,true,200,false,false)==A::None);
  click.update(false,true,true,300,false,false);click.update(false,true,true,330,false,false);
  click.update(true,true,true,400,false,false);assert(click.update(true,true,true,429,false,false)==A::None);
  assert(click.update(true,true,true,430,false,false)==A::Select);assert(click.update(true,true,true,900,false,false)==A::None);
  click.update(false,true,true,1000,false,false);click.update(false,true,true,1030,false,false);
  click.update(true,true,true,1100,true,false);assert(click.update(true,true,true,1130,true,false)==A::Wake);
  std::puts("JoyC navigation: strict thresholds, center latch, 500ms repeat, keyboard directions, diagonals, wake, disconnect, buttons and wrap passed");
}
