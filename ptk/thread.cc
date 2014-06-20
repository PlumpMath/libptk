#include "ptk/assert.h"
#include "ptk/thread.h"
#include "ptk/kernel.h"

using namespace ptk;

const char *thread_state_name(thread_state s) {
  switch (s) {
#define PTK_THREAD_STATE(name,val) case name##_STATE : return #name;
    PTK_THREAD_STATES
#undef PTK_THREAD_STATE
  default : return "?";
  }
}

Thread::Thread() :
  Timer(),
#if defined(PTK_DEBUG)
  file(""),
  line(0)
#endif
  continuation(0),
  state(INIT_STATE),
  wakeup_reason(WAKEUP_OK)
{
  register_thread(*this);
}

Thread::~Thread() {
  unregister_thread(*this);
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
