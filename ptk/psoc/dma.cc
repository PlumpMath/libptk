#include "ptk/psoc/dma.h"

extern "C" {
#include "CyDmac.h"
}

using namespace ptk::psoc;

SimpleDMA::SimpleDMA() :
  ch(CY_DMA_INVALID_CHANNEL),
  td(CY_DMA_INVALID_TD),
  preserve_tds(false)
{}

SimpleDMA::~SimpleDMA() {
  if (td != CY_DMA_INVALID_TD) CyDmaTdFree(td);
  td = CY_DMA_INVALID_TD;

  if (ch != CY_DMA_INVALID_CHANNEL) CyDmaChFree(ch);
  ch = CY_DMA_INVALID_CHANNEL;
}
uint8_t SimpleDMA::init(uint8_t channel,
                        uint8_t priority,
                        uint8_t burst_size,
                        bool    request_per_burst,
                        uint8_t out0_sel,
                        uint8_t out1_sel,
                        uint8_t out_en,
                        uint8_t in_sel,
                        dma_type_t  type)
{
  ch = channel;
  CyDmaChSetConfiguration(ch,
                          burst_size,
                          request_per_burst,
                          out0_sel,
                          out1_sel,
                          in_sel);
  td = CyDmaTdAllocate();

  uint8_t flags=0;

  switch (type) {
  case MEMORY_TO_MEMORY : flags |= CY_DMA_TD_INC_DST_ADR; // fall through
  case MEMORY_TO_PERIPH : flags |= CY_DMA_TD_INC_SRC_ADR; break;
  case PERIPH_TO_MEMORY : flags |= CY_DMA_TD_INC_DST_ADR; break;
  case PERIPH_TO_PERIPH : ;
  }

  flags |= out_en;

  // SimpleDMA uses only one transaction descriptor, so we disable the chain
  // after this one is done. The flags determine which whether the src and/or
  // dst pointers get incremented after each byte is transferred.
  // See CyDmac.c:891
  CY_DMA_TDMEM_STRUCT_PTR[td].TD0[2u] = CY_DMA_DISABLE_TD;
  CY_DMA_TDMEM_STRUCT_PTR[td].TD0[3u] = flags;

  // Set the initial descriptor for the channel
  // See CyDmac.c:535
  CY_DMA_CH_STRUCT_PTR[ch].basic_status[1u] = td;

  return CYRET_SUCCESS;
}

uint8_t SimpleDMA::prepare(const void *src, volatile void *dst, size_t len) {
  uint16_t hi_src, hi_dst, lo_src, lo_dst;

  hi_src = HI16((uint32_t) src);
  lo_src = LO16((uint32_t) src);
  hi_dst = HI16((uint32_t) dst);
  lo_dst = LO16((uint32_t) dst);

  // setup src and dst pointers
  CyDmaChSetExtendedAddress(ch, hi_src, hi_dst);
  CyDmaTdSetAddress(td, lo_src, lo_dst);

  // setup transfer size
  // See CyDmac.c:899
  reg16 *convert = (reg16 *) &CY_DMA_TDMEM_STRUCT_PTR[td].TD0[0u];
  CY_SET_REG16(convert, (len & 0x0fff)); // only 12 bits!

  return CYRET_SUCCESS;
}

uint8_t SimpleDMA::enable() {
  return CyDmaChEnable(ch, true);
}

uint8_t SimpleDMA::disable() {
  return CyDmaChDisable(ch);
}
