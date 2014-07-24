#include "ptk/assert.h"
#include "ptk/thread.h"
#include "ptk/kernel.h"

using namespace ptk;

Thread *ptk::all_registered_threads = 0;

const char *Thread::state_name() const {
  switch (state) {
  case INIT_STATE : return "INIT";
  case READY_STATE : return "READY";
  case SLEEPING_STATE : return "SLEEP";
  case WAIT_COND_STATE : return "W_COND";
  case WAIT_EVENT_STATE : return "W_EVNT";
  case WAIT_SUBTHREAD_STATE : return "W_THRD";
  case FINAL_STATE : return "FINAL";
  case RESET_STATE : return "RESET";
  default : return "???";
  }
}

Thread::Thread() :
  Timer(),
#if defined(PTK_DEBUG)
  debug_file(0),
  debug_line(0),
#endif
  continuation(0),
  state(INIT_STATE),
  wakeup_reason(WAKEUP_OK)
{
  next_registered_thread = all_registered_threads;
  all_registered_threads = this;
}

Thread::~Thread() {
  // need to remove this thread from the all_registered_threads list
  ptk_halt("dangling thread");
}

void Thread::timer_expired() {
  lock_from_isr();
  wakeup_reason = WAKEUP_TIMEOUT;
  timer_expiration = TIME_EXPIRED;
  schedule_thread(*this);
  unlock_from_isr();
}

void Thread::ptk_end() {
  state = FINAL_STATE;
  continuation = 0;
}

SubThread::SubThread() :
  parent(0)
{
}

void SubThread::reset() {
  parent = 0;
  state = INIT_STATE;
}

void SubThread::ptk_end() {
  Thread::ptk_end();
  if (parent != 0) {
    lock_kernel();
    wakeup_thread(*parent, WAKEUP_SUBTHREAD_DONE);
    parent = 0;
    unlock_kernel();
  }
}
