#include "ptk/io.h"

DeviceInputBase::DeviceInputBase(uint8_t *storage, unsigned len) :
  fifo(storage, len)
{
}


DeviceOutputBase::DeviceOutputBase(uint8_t *storage, unsigned len) :
  fifo(storage, len)
{
}
