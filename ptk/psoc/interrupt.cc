#include "cydevice_trm.h"
#include "cytypes.h"

#include "ptk/psoc/interrupt.h"

using namespace ptk::psoc;

extern "C" {
  void IntDefaultHandler(void);
}

// setup for X-Macro
#define INTERRUPTS \
  INTERRUPT(0)     \
  INTERRUPT(1)     \
  INTERRUPT(2)     \
  INTERRUPT(3)     \
  INTERRUPT(4)     \
  INTERRUPT(5)     \
  INTERRUPT(6)     \
  INTERRUPT(7)     \
  INTERRUPT(8)     \
  INTERRUPT(9)     \
  INTERRUPT(10)    \
  INTERRUPT(11)    \
  INTERRUPT(12)    \
  INTERRUPT(13)    \
  INTERRUPT(14)    \
  INTERRUPT(15)    \
  INTERRUPT(16)    \
  INTERRUPT(17)    \
  INTERRUPT(18)    \
  INTERRUPT(19)    \
  INTERRUPT(20)    \
  INTERRUPT(21)    \
  INTERRUPT(22)    \
  INTERRUPT(23)    \
  INTERRUPT(24)    \
  INTERRUPT(25)    \
  INTERRUPT(26)    \
  INTERRUPT(27)    \
  INTERRUPT(28)    \
  INTERRUPT(29)    \
  INTERRUPT(30)    \
  INTERRUPT(31)

enum {
  MAX_INTERRUPTS = 32
};

Interrupt *psoc_interrupts[MAX_INTERRUPTS];

// define a hook function to call each intterupt's isr() method
#define INTERRUPT(n) void irq_hook##n() { if (psoc_interrupts[n]) psoc_interrupts[n]->isr(); }
INTERRUPTS
#undef INTERRUPT

// ROMable table of interrupt hook functions
cyisraddress const psoc_interrupt_hooks[MAX_INTERRUPTS] = {
#define INTERRUPT(n) &irq_hook##n,
INTERRUPTS
#undef INTERRUPT
};

static reg8 *nvic_priority_register(uint8_t interrupt_number) {
  // the first priority register is at CYREG_NVIC_PRI_0, and the addresses
  // increase by one byte for each
  return ((reg8 *) CYREG_NVIC_PRI_0) + interrupt_number;
}

static void nvic_set_interrupt_vector(uint8_t interrupt_number, void (*isr)(void)) {
  cyisraddress *ram_vector_table = *(cyisraddress **) CYREG_NVIC_VECT_OFFSET;
  const unsigned CYINT_IRQ_BASE = 16;

  ram_vector_table[CYINT_IRQ_BASE + interrupt_number] = isr;
}

void Interrupt::isr() {
}

Interrupt::Interrupt(uint8_t number, uint8_t prio) :
  number(number),
  initial_priority(prio)
{
}

void Interrupt::start() {
  set_enable(false);
  psoc_interrupts[number] = this;
  nvic_set_interrupt_vector(number, psoc_interrupt_hooks[number]);
  set_priority(initial_priority);
  set_enable(true);
}

void Interrupt::stop() {
  set_enable(false);
  nvic_set_interrupt_vector(number, &IntDefaultHandler);
  psoc_interrupts[number] = 0;
}

uint8_t Interrupt::set_priority(uint8_t new_prio) {
  reg8 *prio_reg = nvic_priority_register(number);
  uint8_t old_prio = *prio_reg >> 5;

  *prio_reg = new_prio << 5;
  return old_prio;
}

void Interrupt::set_enable(bool value) {
  reg8 *nvic_set_enable_register = (reg8 *) CYREG_NVIC_SETENA0;
  reg8 *nvic_clr_enable_register = (reg8 *) CYREG_NVIC_CLRENA0;

  if (value) {
    *nvic_set_enable_register = 1 << number;
  } else {
    *nvic_clr_enable_register = 1 << number;
  }
}

bool Interrupt::get_enable() const {
  reg8 *nvic_set_enable_register = (reg8 *) CYREG_NVIC_SETENA0;
  return (*nvic_set_enable_register & (1 << number)) != 0;
}

void Interrupt::set_pending(bool value) {
  reg8 *nvic_set_pend_register = (reg8 *) CYREG_NVIC_SETPEND0;
  reg8 *nvic_clr_pend_register = (reg8 *) CYREG_NVIC_CLRPEND0;

  if (value) {
    *nvic_set_pend_register = 1 << number;
  } else {
    *nvic_clr_pend_register = 1 << number;
  }
}

InterruptEvent::InterruptEvent(uint8_t number, uint8_t prio) :
  Interrupt(number, prio),
  Event()
{
}

void InterruptEvent::isr() {
  enter_isr();

  isr_hook();

  lock_from_isr();
  broadcast_event(*this, mask);
  unlock_from_isr();
  leave_isr();
}

void InterruptEvent::isr_hook() {
}
