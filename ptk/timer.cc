#include "ptk/timer.h"

using namespace ptk;

Timer::Timer() :
  timer_expiration(TIME_NEVER)
{}

void Timer::reset() {
  timer_expiration = TIME_NEVER;
}
