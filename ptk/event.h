#pragma once

#include "ptk/thread.h"

namespace ptk {
  typedef int32_t eventmask_t;
  class Thread;

  class Event {
    friend class ptk::Kernel;

    DQueue<Thread> waiting;
  public:
    eventmask_t mask;
  Event() : waiting(&Thread::ready_link) {}
  };
}
