#pragma once

#include "ptk/dqueue.h"
#include "ptk/timer.h"
#include "ptk/event.h"

namespace ptk {
  class Thread;
  class Event;

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

  extern Kernel *the_kernel;

  inline void register_thread(Thread &t) {
    the_kernel->register_thread(t);
  }

  inline void unregister_thread(Thread &t) {
    the_kernel->unregister_thread(t);
  }

  inline void arm_timer(Timer &t, ptk_time_t when) {
    the_kernel->arm_timer(t, when);
  }

  inline void disarm_timer(Timer &t) {
    the_kernel->disarm_timer(t);
  }

  inline bool timer_is_armed(const Timer &t) {
    return the_kernel->timer_is_armed(t);
  }

  inline void schedule_thread(Thread &t) {
    the_kernel->schedule(t);
  }

  inline void unschedule_thread(Thread &t) {
    the_kernel->unschedule(t);
  }

  inline void wait_subthread(Thread &parent, SubThread &sub, ptk_time_t duration) {
    the_kernel->wait_subthread(parent, sub, duration);
  }

  inline void wakeup_thread(Thread &t, wakeup_t reason) {
    the_kernel->wakeup(t, reason);
  }

  inline void enter_isr() {
    the_kernel->enter_isr();
  }

  inline void lock_from_isr() {
    the_kernel->lock_from_isr();
  }

  inline void unlock_from_isr() {
    the_kernel->unlock_from_isr();
  }

  inline void leave_isr() {
    the_kernel->leave_isr();
  }

  inline void wait_event(Thread &t, Event &e, ptk_time_t duration) {
    the_kernel->wait_event(t, e, duration);
  }

  inline void signal_event(Event &e, eventmask_t mask) {
    the_kernel->signal_event(e, mask);
  }

  inline void broadcast_event(Event &e, eventmask_t mask) {
    the_kernel->broadcast_event(e, mask);
  }

  inline void lock_kernel() {
    the_kernel->lock();
  }

  inline void unlock_kernel() {
    the_kernel->unlock();
  }

  inline void dump_kernel() {
    the_kernel->dump();
  }
    
}


