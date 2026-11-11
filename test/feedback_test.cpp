#include "Feedback.h"
#include "Settings.h"
#include "Joyc.h"
#include <M5Unified.h>
#include <cassert>
#include <cstdio>
namespace stick {uint8_t lastLed=0;bool setJoycLed(uint8_t level){lastLed=level;return true;}}
int main(){
 using namespace stick;using A=NavigationAction;
 updateFeedback(0,0,A::Select,0,0);assert(M5.Speaker.tones.empty());
 updateFeedback(100,SoundUi,A::Up,0,0);assert(M5.Speaker.tones.size()==1&&M5.Speaker.tones[0].duration==30);
 updateFeedback(200,SoundUi,A::Select,0,0);assert(M5.Speaker.tones.size()==2);
 updateFeedback(279,SoundUi,A::None,0,0);assert(M5.Speaker.tones.size()==2);
 updateFeedback(280,SoundUi,A::None,0,0);assert(M5.Speaker.tones.size()==3);
 updateFeedback(500,SoundDm,A::None,1,0);assert(M5.Speaker.tones.back().frequency==2400);
 const auto count=M5.Speaker.tones.size();updateFeedback(700,SoundDm,A::None,1,0);assert(M5.Speaker.tones.size()==count);
 updateFeedback(800,SoundMesh,A::None,1,1);assert(M5.Speaker.tones.back().frequency==1600);
 updateFeedback(900,JoycLed,A::Up,1,1);updateFeedback(960,JoycLed,A::None,1,1);assert(lastLed==48);
 updateFeedback(1140,JoycLed,A::None,1,1);assert(lastLed==0);
 std::puts("Feedback: independent sound events, double confirmation, no duplicate alerts and bounded LED pulse passed");
}
