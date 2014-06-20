#pragma once

#include "ptk/dqueue.h"
#include "ptk/timer.h"

namespace ptk {
  class Kernel;
  class Semaphore;

  typedef int32_t wakeup_t;

  enum {
    WAKEUP_OK             = 1 << 0,
    WAKEUP_TIMEOUT        = 1 << 1,
    WAKEUP_RESET          = 1 << 2,
    WAKEUP_SUBTHREAD_DONE = 1 << 3,
  };

#define PTK_THREAD_STATES                       \
  PTK_THREAD_STATE(INIT,           1)           \
    PTK_THREAD_STATE(READY,          2)         \
    PTK_THREAD_STATE(YIELDED,        4)         \
    PTK_THREAD_STATE(SLEEPING,       8)         \
    PTK_THREAD_STATE(WAIT_COND,      16)        \
    PTK_THREAD_STATE(WAIT_EVENT,     32)        \
    PTK_THREAD_STATE(WAIT_SUBTHREAD, 64)        \
    PTK_THREAD_STATE(FINAL,          128)       \
    PTK_THREAD_STATE(RESET,          256)

  enum thread_state {
#define PTK_THREAD_STATE(name,val) name##_STATE = val,
    PTK_THREAD_STATES
#undef PTK_THREAD_STATE
  };

  enum {
    RUNNABLE_STATES = (READY_STATE | YIELDED_STATE | WAIT_COND_STATE)
  };

  const char *thread_state_name(thread_state s);

#define PTK_LABEL_AT_LINE_HELPER(n) PTK_LINE_##n
#define PTK_LABEL_AT_LINE(n) PTK_LABEL_AT_LINE_HELPER(n)
#define PTK_HERE PTK_LABEL_AT_LINE(__LINE__)

  class Thread : protected Timer {
    friend class Kernel;
    friend class Semaphore;
    friend class Event;

    DLink<Thread> registry_link;
    DLink<Thread> ready_link;

  protected:
    virtual void run() = 0;
    virtual void timer_expired();
    virtual void ptk_end();

#if defined(PTK_DEBUG)
    const char *file;
    int line;
#endif

    void *continuation;
    thread_state state;
    wakeup_t wakeup_reason;


  public:
    Thread();
    virtual ~Thread();
  };

  class SubThread : public Thread {
    friend class Kernel;

  protected:
    Thread *parent;

    virtual void reset();
    virtual void ptk_end();

  public:
    SubThread();
  };

#if defined(PTK_DEBUG)
#define PTK_DEBUG_SAVE()                            \
  file = __FILE__;                                  \
  line = __LINE__;
#else
#define PTK_DEBUG_SAVE()
#endif

#define PTK_BEGIN()                                 \
  do {                                              \
    if (continuation != 0) goto *continuation;      \
    this->timer_expiration = TIME_NEVER;            \
  } while (0)                                        
                                                     
#define PTK_YIELD()                                 \
  do {                                              \
    state = YIELDED_STATE;                          \
    continuation = &&PTK_HERE;                      \
    PTK_DEBUG_SAVE();                               \
    return;                                         \
  PTK_HERE: ;                                       \
  } while (0)                                        
                                                     
#define PTK_SLEEP(duration)                         \
  do {                                              \
    lock_kernel();                                  \
    state = SLEEPING_STATE;                         \
    unschedule_thread(*this);                       \
    arm_timer(*this, (duration));                   \
    continuation = &&PTK_HERE;                      \
    PTK_DEBUG_SAVE();                               \
    unlock_kernel();                                \
    return;                                         \
  PTK_HERE: ;                                       \
  } while (0)                                        
                                                     
#define PTK_WAIT_EVENT(event,duration)              \
  do {                                              \
    wait_event(*this, event, duration);             \
    continuation = &&PTK_HERE;                      \
    PTK_DEBUG_SAVE();                               \
    state = WAIT_EVENT_STATE;                       \
    return;                                         \
  PTK_HERE: ;                                       \
    lock_kernel();                                  \
    disarm_timer(*this);                            \
    unlock_kernel();                                \
  } while(0)

#define PTK_UNLOCK_WAIT_EVENT(event,duration)       \
  do {                                              \
    wait_event(*this, event, duration);             \
    continuation = &&PTK_HERE;                      \
    PTK_DEBUG_SAVE();                               \
    state = WAIT_EVENT_STATE;                       \
    unlock_kernel();                                \
    return;                                         \
  PTK_HERE: ;                                       \
    lock_kernel();                                  \
    disarm_timer(*this);                            \
    unlock_kernel();                                \
  } while(0)

#define PTK_WAIT_SUBTHREAD(sub,duration)            \
  do {                                              \
    lock_kernel();                                  \
    wait_subthread(*this, sub, duration);           \
    continuation = &&PTK_HERE;                      \
    PTK_DEBUG_SAVE();                               \
    state = WAIT_SUBTHREAD_STATE;                   \
    unlock_kernel();                                \
    return;                                         \
  PTK_HERE: ;                                       \
    lock_kernel();                                  \
    disarm_timer(*this);                            \
    unlock_kernel();                                \
  } while(0)

#define PTK_WAIT_UNTIL(condition,duration)          \
  do {                                              \
  PTK_HERE: ;                                       \
    ptk_time_t duration_temp = (duration);          \
    continuation = &&PTK_HERE;                      \
    PTK_DEBUG_SAVE();                               \
    if (duration_temp == TIME_INFINITE) {           \
      if (!(condition)) {                           \
        state = WAITING_STATE;                      \
        return;                                     \
      }                                             \
    } else {                                        \
      if (!(condition)) {                           \
        if (timer_expiration == TIME_NEVER) {       \
          lock_kernel();                            \
          arm_timer(*this, duration_temp);          \
          unlock_kernel();                          \
        }                                           \
        state = WAITING_STATE;                      \
        return;                                     \
      } else {                                      \
        lock_kernel();                              \
        disarm_timer(*this);                        \
        unlock_kernel();                            \
      }                                             \
    }                                               \
  } while (0)

#define PTK_END()                                   \
  do {                                              \
  thread_exit:                                      \
    ptk_end();                                      \
    void *pexit = &&thread_exit;                    \
    (void) pexit;                                   \
  } while (0)


}
