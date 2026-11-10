#include "Clock.h"
#include <M5Unified.h>
#include <sys/time.h>
#include <ctime>
#include "BuildTime.h"
namespace stick {
namespace {bool rtc=false;}
bool beginClock(){
  // RTC is stored in UTC. A newly built firmware repairs an unset/older RTC.
  setenv("TZ","UTC0",1);tzset();
  time_t epoch=STICK_BUILD_EPOCH;m5::rtc_datetime_t dt;
  rtc=M5.Rtc.isEnabled()&&M5.Rtc.getDateTime(&dt)&&dt.isValid();
  if(rtc){auto fields=dt.get_tm();time_t read=mktime(&fields);if(read>=epoch)epoch=read;else {tm fieldsUtc;gmtime_r(&epoch,&fieldsUtc);rtc=M5.Rtc.setDateTime(&fieldsUtc);}}
  else if(M5.Rtc.isEnabled()){tm fields;gmtime_r(&epoch,&fields);rtc=M5.Rtc.setDateTime(&fields);}
  timeval tv{epoch,0};return settimeofday(&tv,nullptr)==0;
}
uint64_t clockMilliseconds(){timeval tv{};gettimeofday(&tv,nullptr);return uint64_t(tv.tv_sec)*1000+tv.tv_usec/1000;}
bool clockRtc(){return rtc;}
}
