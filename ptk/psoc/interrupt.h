#pragma once

#include "ptk/ptk.h"

#include <cstddef>
#include <cstdint>

namespace ptk {
  namespace psoc {
    class InterruptEvent : public ptk::Event {
      const uint8_t number;

    public:
      InterruptEvent(uint8_t n);
    };
  }
}

