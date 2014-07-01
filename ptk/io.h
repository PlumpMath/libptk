#pragma once

#include "ptk/fifo.h"
#include "ptk/ptk.h"

#include <stdint.h>

namespace ptk {
  struct DeviceInputBase {
    FIFO<uint8_t> fifo;
    Event not_empty;

    DeviceInputBase(uint8_t *storage, unsigned len);
    virtual void device_read_isr() = 0;
    virtual unsigned read(uint8_t *buffer, unsigned maxlen) = 0;
  };

  struct DeviceOutputBase {
    FIFO<uint8_t> fifo;
    Event not_full;

    DeviceOutputBase(uint8_t *storage, unsigned len);
    virtual void device_write_isr() = 0;
    virtual unsigned write(const uint8_t *buffer, unsigned len) = 0;
  };

  template<unsigned N>
  class DeviceInput : public DeviceInputBase {
    uint8_t data[N];

  public:
    DeviceInput() : DeviceInputBase(data, N) { }
  };

  template<unsigned N>
  class DeviceOutput : public DeviceOutputBase {
    uint8_t data[N];

  public:
    DeviceOutput() : DeviceOutputBase(data, N) { }
  };
}
