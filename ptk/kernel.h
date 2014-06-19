#pragma once

#include "ptk/dqueue.h"
#include "ptk/timer.h"
#include "ptk/event.h"

namespace ptk {
  class Thread;

  class Kernel {
  protected:
    DQueue<Thread> ready_list;
    DQueue<Timer> armed_timers;
    DQueue<Thread> thread_registry;
    Thread *active_thread;
    volatile int16_t isr_depth;
    volatile int16_t lock_depth;
      
  public:
    Kernel();

    void register_thread(Thread &t);
    void unregister_thread(Thread &t);
    void arm_timer(Timer &t, ptk_time_t when);
    void disarm_timer(Timer &t);
    bool timer_is_armed(const Timer &t);

    void schedule(Thread &t);
    void unschedule(Thread &t);
    void wait_subthread(Thread &parent, SubThread &sub, ptk_time_t duration);
    void wakeup(Thread &t, wakeup_t reason);

    bool run_once();
    void expire_timers(uint32_t time_delta);

    void enter_isr();
    void lock_from_isr();
    void unlock_from_isr();
    void leave_isr();

    void wait_event(Thread &t, Event &e, ptk_time_t duration);
    void signal_event(Event &e, eventmask_t mask);
    void broadcast_event(Event &e, eventmask_t mask);

    void lock();
    void unlock();
    void dump();
  };
}

