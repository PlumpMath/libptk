#include "ptk/assert.h"

#if defined(PTK_STM32F4XX)
#include "stm32f4xx.h"
#endif

#if defined(PTK_USE_CC_STUBS)

void *operator new[](unsigned size) {
  ptk_halt("unimplemented: operator new[]()");
  // not reached
  return (void *) 0xffffffff;
}

void operator delete(void *ptr) {
  ptk_halt("unimplemented: operator delete()");
}

extern "C" {
  void __cxa_pure_virtual() {
    ptk_halt("unimplemented: pure virtual");
  }

  void *__dso_handle = NULL;

  void _exit(int status) {
    (void) status;
    ptk_halt("exit");
  }

  pid_t _getpid(void) {
    return 1;
  }

  void _kill(pid_t id) {
    (void) id;
  }

#if defined(PTK_TM4C)
#define BSS_END_SYMBOL _ebss
#endif

  void *_sbrk(int incr) {
    extern unsigned long BSS_END_SYMBOL; // Defined by the linker
    static char *limit = 0;

    if (limit == 0) limit = (char *) &BSS_END_SYMBOL;

    void *allocated = limit;
    char *main_stack_pointer;

    // this should work on any ARM Cortex
    asm volatile ("mov %[out], sp" : [out] "=r" (main_stack_pointer));

    if (limit + incr > main_stack_pointer) {
      ptk_halt("out of memory");
    }

    // NOTE: even if we get here, the stack can collide with the heap later on

    limit += incr;
    return allocated;
  }
};

#endif // defined(PTK_USE_CC_STUBS)
