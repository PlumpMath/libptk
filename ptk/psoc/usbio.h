#pragma once

#include "ptk/io.h"

extern "C" {
  void usbfs_endpoint_isr(int ep);
  void usbfs_reset_isr();
}

namespace ptk {
  namespace psoc {
    class USBInEndpoint : public ptk::DeviceInputBase {
      uint8_t fifo_storage[64];

    public:
      uint8_t ep;

      USBInEndpoint();
      virtual unsigned read(uint8_t *buffer, unsigned maxlen);
      virtual void device_read_isr();
    };

    class USBOutEndpoint : public ptk::DeviceOutputBase {
      uint8_t fifo_storage[64];

    public:
      uint8_t ep;

      USBOutEndpoint();
      virtual unsigned write(const uint8_t *buffer, unsigned len);
      virtual void device_write_isr();
    };

    class USBEcho : public ptk::Thread {
    public:
      USBInEndpoint rx;
      USBOutEndpoint tx;

      USBEcho();
      virtual void run();
    };

    extern USBEcho usb_echo;
  }
}
