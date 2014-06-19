#pragma once

#include <cstddef>
#include <cstdint>

namespace ptk {
  namespace psoc {
    enum dma_type_t {
      MEMORY_TO_PERIPH,
      MEMORY_TO_MEMORY,
      PERIPH_TO_MEMORY,
      PERIPH_TO_PERIPH
    };

    class SimpleDMA {
      uint8_t ch;
      uint8_t td;
      bool    preserve_tds;

    public:

      SimpleDMA();
      ~SimpleDMA();

      uint8_t init(uint8_t channel,
                   uint8_t priority,
                   uint8_t burst_size,
                   bool    request_per_burst,
                   uint8_t out0_sel,
                   uint8_t out1_sel,
                   uint8_t out_en,
                   uint8_t in_sel,
                   dma_type_t type);

      uint8_t enable();
      uint8_t disable();

      uint8_t status();
      uint8_t prepare(const void *src, volatile void *dst, size_t len);
    };
  }
}

#define SIMPLE_DMA_INIT(name,burst,req_per_burst, type)     \
  init(name##__DRQ_NUMBER,                                  \
       name##__PRIORITY,                                    \
       burst,                                               \
       req_per_burst,                                       \
       name##__TERMOUT0_SEL,                                \
       name##__TERMOUT1_SEL,                                \
       ((name##__TERMOUT0_EN ? TD_TERMOUT0_EN : 0) |        \
        (name##__TERMOUT1_EN ? TD_TERMOUT1_EN : 0)),        \
       name##__TERMIN_SEL,                                  \
       type)
