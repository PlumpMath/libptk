#pragma once

#include "ptk/ptk.h"

#include <cstddef>
#include <cstdint>

namespace ptk {
  namespace psoc {
    class Interrupt {
      const uint8_t number;
      const uint8_t initial_priority;

    public:
      Interrupt(uint8_t number, uint8_t prio);
      void start();
      void stop();
      uint8_t set_priority(uint8_t prio);
      void set_enable(bool value);
      bool get_enable() const;
      void set_pending(bool value);
      virtual void isr();
    };

    class InterruptEvent : public Interrupt, public Event {
    protected:
      virtual void isr_hook();

    public:
      InterruptEvent(uint8_t number, uint8_t prio);
      
      eventmask_t mask;
      virtual void isr() final;
    };
  }
}

#define PSOC_INTERRUPT_CTOR_ARGS(name) name##__INTC_NUMBER, name##__INTC_PRIOR_NUM
#define PSOC_INTERRUPT(name) name PSOC_INTERRUPT_CTOR_ARGS(name)
