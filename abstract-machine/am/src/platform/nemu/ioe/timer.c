#include <am.h>
#include <nemu.h>


void __am_timer_init() {

}


// AM系统启动时间, 可读出系统启动后的微秒数.
//TODO:: 框架代码中埋了一些小坑  即只有当高32位读的时候才更新时间，所以要先读高32位
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us=0;

  uint32_t high_time=inl(RTC_ADDR+sizeof(uint32_t));
  uint32_t low_time=inl(RTC_ADDR);


  //要先变成64位才能位移
  uptime->us=(((uint64_t)high_time)<<32)+(uint64_t)low_time;

  
}


void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
