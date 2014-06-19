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

Thread::Thread(Kernel &k, const char *name) :
  Timer(),
  kernel(k),
  continuation(0),
  state(INIT_STATE),
  wakeup_reason(WAKEUP_OK),
#if defined(PTK_DEBUG)
  file(""),
  line(0),
#endif
  name(name)
{
  kernel.register_thread(*this);
}

Thread::~Thread() {
  kernel.unregister_thread(*this);
}

void Thread::timer_expired() {
  kernel.lock_from_isr();
  wakeup_reason = WAKEUP_TIMEOUT;
  timer_expiration = TIME_EXPIRED;
  kernel.schedule(*this);
  kernel.unlock_from_isr();
}

void Thread::ptk_end() {
  state = FINAL_STATE;
  continuation = 0;
}

SubThread::SubThread(Kernel &k, const char *name) :
  Thread(k, name),
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
    kernel.lock();
    kernel.wakeup(*parent, WAKEUP_SUBTHREAD_DONE);
    parent = 0;
    kernel.unlock();
  }
}
