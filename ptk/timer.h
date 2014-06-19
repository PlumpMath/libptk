#pragma once

#include "ptk/assert.h"
#include "ptk/dqueue.h"

namespace ptk {
  class Kernel;

  typedef uint32_t ptk_time_t;

  /*
   * The order of the constants below is such that a ptk_time_t value
   * can be decremented to account for the passage of time if it's
   * value is less than TIME_INFINITE.
   */
  enum {
    TIME_IMMEDIATE = 0,
    TIME_INFINITE  = (unsigned) -3,
    TIME_EXPIRED   = (unsigned) -2,
    TIME_NEVER     = (unsigned) -1,
  };

  class Timer {
    friend class Kernel;
    virtual void timer_expired() = 0;
    ptk::DLink<Timer> timer_link;

  protected:
    ptk_time_t timer_expiration;

  public:
    Timer();
    void reset();
  };
}
