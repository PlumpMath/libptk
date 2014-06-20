#include "ptk/assert.h"
#include "ptk/kernel.h"

using namespace ptk;

#if !defined(PTK_PORT_DISABLE_INTERRUPTS)
#define PTK_PORT_DISABLE_INTERRUPTS asm volatile ("cpsid i")
#else
#define PTK_PORT_DISABLE_INTERRUPS while (0) {}
#endif

#if !defined(PTK_PORT_ENABLE_INTERRUPTS)
#define PTK_PORT_ENABLE_INTERRUPTS asm volatile ("cpsie i")
#else
#define PTK_PORT_ENABLE_INTERRUPS while (0) {}
#endif

Kernel *ptk::the_kernel = 0;

Kernel::Kernel() :
ready_list(&Thread::ready_link),
  armed_timers(&Timer::timer_link),
  thread_registry(&Thread::registry_link),
  active_thread(0),
  isr_depth(0),
  lock_depth(0)
{}

void Kernel::register_thread(Thread &t) {
  thread_registry.push(t);
}

void Kernel::unregister_thread(Thread &t) {
  thread_registry.remove(t);
}

void Kernel::schedule(Thread &t) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to schedule a thread.");
  // add t to the end of the ready list
  ready_list.insert_after(t);
}

void Kernel::wakeup(Thread &t, wakeup_t reason) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to wakeup a thread.");
  t.wakeup_reason = reason;
  ready_list.insert_after(t);
}

void Kernel::wait_subthread(Thread &parent, SubThread &sub, ptk_time_t duration) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to wait on a subthread.");
  unschedule(parent);
  if (duration < TIME_INFINITE) arm_timer(parent, duration);
  sub.reset();
  sub.parent = &parent;
  schedule(sub);
}

void Kernel::unschedule(Thread &t) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to unschedule a thread.");
  ready_list.remove(t);
}

bool Kernel::timer_is_armed(const Timer &t) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to test if a timer is armed.");
  return t.timer_expiration != TIME_NEVER;
}

void Kernel::lock() {
  PTK_ASSERT(isr_depth == 0,
             "Kernel::lock() called while an interrupt is active.\n"
             "Use Kernel::lock_from_isr() instead.");
  PTK_ASSERT(lock_depth >= 0,
             "Negative lock_depth. Kernel::unlock() called without\n"
             "matching Kernel::lock().");
  PTK_PORT_DISABLE_INTERRUPTS;
  lock_depth++;
}

void Kernel::unlock() {
  PTK_ASSERT(isr_depth == 0,
             "Kernel::unlock() called while an interrupt is active.\n"
             "Use Kernel::unlock_from_isr() instead.");
  PTK_ASSERT(lock_depth > 0,
             "Negative lock_depth. Kernel::unlock() called without\n"
             "matching Kernel::lock().");
  if (--lock_depth == 0) {
    PTK_PORT_ENABLE_INTERRUPTS;
  }
}

void Kernel::lock_from_isr() {
  PTK_ASSERT(isr_depth > 0,
             "Kernel::lock_from_isr() called while no interrupt is active.\n"
             "Use Kernel::lock() instead.");
  PTK_ASSERT(lock_depth >= 0,
             "Negative lock_depth. Kernel::unlock_from_isr() called without\n"
             "matching Kernel::lock_from_isr().");
  PTK_PORT_DISABLE_INTERRUPTS;
  lock_depth++;
}

void Kernel::unlock_from_isr() {
  PTK_ASSERT(isr_depth > 0,
             "Kernel::unlock_from_isr() called while no interrupt is active.\n"
             "Use Kernel::unlock() instead.");
  PTK_ASSERT(lock_depth > 0,
             "Negative lock_depth. Kernel::unlock_from_isr() called without\n"
             "matching Kernel::lock_from_isr().");
  if (--lock_depth == 0) {
    PTK_PORT_ENABLE_INTERRUPTS;
  }
}

void Kernel::enter_isr() {
  PTK_ASSERT(lock_depth == 0,
             "An interrupt occurred while the kernel was locked.\n"
             "Interrupts should have been disabled!");
  PTK_ASSERT(isr_depth >= 0,
             "Negative isr_depth. Kernel::leave_isr() called without\n"
             "matching Kernel::enter_isr().");
  isr_depth++;
}

void Kernel::leave_isr() {
  PTK_ASSERT(isr_depth > 0,
             "Negative isr_depth. Kernel::leave_isr() called without\n"
             "matching Kernel::enter_isr().");
  --isr_depth;
}

void Kernel::arm_timer(Timer &t, ptk_time_t when) {
  PTK_ASSERT(t.timer_expiration == TIME_NEVER,
             "Attempt to arm a Timer that is already armed.");
  t.timer_expiration = when;
  armed_timers.insert_after(t);
}

void Kernel::disarm_timer(Timer &t) {
  armed_timers.remove(t);
  t.timer_expiration = TIME_NEVER;
}

void Kernel::expire_timers(uint32_t time_delta) {
  DQueue<Timer> expired(&Timer::timer_link);

  // phase 1: find the timers that have expired
  lock_from_isr();
  for (auto i = armed_timers.begin(); i != armed_timers.end();) {
    // Extract the Timer pointer from the iterator before (possibly) removing
    // the timer from the queue. This avoids screwing up iterator.
    Timer *t = *i++;

    if (t->timer_expiration <= time_delta) {
      // We want to tell the timer how much time has elapsed since it's
      // actual expiration. However, the timer_expiration field is unsigned
      // so we can't simply subtract the time_delta and save the result.
      // Instead, we'll save (time_delta-now), which will be non-negative.
      t->timer_expiration = time_delta - t->timer_expiration;

      armed_timers.remove(*t);
      expired.push(*t);
    } else if (t->timer_expiration < TIME_INFINITE) {
      t->timer_expiration -= time_delta;
    }
  }
  unlock_from_isr();

  // phase 2: call timer_expired() on each
  while (!expired.empty()) {
    Timer &t(expired.front());

    t.timer_expired();
    t.timer_expiration = TIME_NEVER;
    expired.pop();
  }
}

void Kernel::wait_event(Thread &thread, Event &event, ptk_time_t duration) {
  lock();
  if (duration != TIME_INFINITE) arm_timer(thread, duration);
  unschedule(thread);
  event.waiting.insert_before(thread);
  unlock();
}

void Kernel::signal_event(Event &event, eventmask_t mask) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to signal an event.");
 
  if (!event.waiting.empty()) {
    Thread &thread(event.waiting.front());

    event.waiting.pop();
    thread.wakeup_reason |= mask;
    schedule(thread);
  }
}

void Kernel::broadcast_event(Event &event, eventmask_t mask) {
  PTK_ASSERT(lock_depth > 0,
             "Kernel must be locked to broadcast an event.");

  while (!event.waiting.empty()) {
    Thread &thread(event.waiting.front());

    event.waiting.pop();
    thread.wakeup_reason |= mask;
    schedule(thread);
  }
}

bool Kernel::run_once() {
  if (!ready_list.empty()) {
    lock();
    active_thread = &ready_list.front();
    ready_list.pop();
    unlock();

    active_thread->run();

    lock();
    if (active_thread->state & RUNNABLE_STATES) schedule(*active_thread);
    active_thread = 0;
    unlock();
  }

  return !ready_list.empty();
}
