#pragma once

#include "ptk/fifo.h"
#include "ptk/ptk.h"

#include <cstdint>
#include <cstddef>

// Declarations

namespace ptk {
  struct InStream {
    virtual size_t read(uint8_t *buffer, size_t max) = 0;
    virtual bool get(uint8_t &ch) = 0;
  };

  struct OutStream {
    virtual size_t write(const uint8_t *buffer, size_t len) = 0;
    virtual bool put(uint8_t ch) = 0;
  };

  class DeviceInStream : public InStream {
  protected:
    FIFO<uint8_t> fifo;
    void device_wrote_to_fifo();

  public:
    DeviceInStream(uint8_t *fifo_storage, size_t fifo_size);
    virtual size_t read(uint8_t *buffer, size_t max);
    virtual bool get(uint8_t &ch);

    Event not_empty;
  };

  class DeviceOutStream : public OutStream {
  protected:
    FIFO<uint8_t> fifo;
    void device_read_from_fifo();

  public:
    DeviceOutStream(uint8_t *fifo_storage, size_t fifo_size);
    virtual size_t write(const uint8_t *buffer, size_t len);
    virtual bool put(uint8_t ch);

    Event not_full;
  };

  struct USBEndpoint {
    enum {MAX_ENDPOINTS = 9};
    static USBEndpoint *eps[MAX_ENDPOINTS];
    uint8_t ep_id;

    virtual void init(uint8_t ep);
    virtual void transfer() = 0;

    static void transfer_endpoint_data(uint8_t ep);
  };

  class EchoThread : public SubThread {
    DeviceInStream &in;
    OutStream &out;

  public:
    EchoThread(DeviceInStream &in, OutStream &out) :
      in(in), out(out)
    { }

    virtual void run() {
      PTK_BEGIN();

      while (1) {
        PTK_WAIT_EVENT(in.not_empty, TIME_INFINITE);

        unsigned bytes_copied;
        do {
          uint8_t buf[8]; // should work with any buffer size > 0
          
          bytes_copied = in.read(buf, sizeof(buf));
          out.write(buf, bytes_copied);
        } while (bytes_copied > 0);
      }

      PTK_END();
    }
  };
}

// Inline Definitions

namespace ptk {
  inline DeviceInStream::DeviceInStream(uint8_t *fifo_storage, size_t fifo_size) :
    fifo(fifo_storage, fifo_size)
  { }

  inline void DeviceInStream::device_wrote_to_fifo() {
    if (fifo.read_capacity() > 0) broadcast_event(not_empty, 0);
  }

  inline DeviceOutStream::DeviceOutStream(uint8_t *fifo_storage, size_t fifo_size) :
    fifo(fifo_storage, fifo_size)
  { }

  inline void DeviceOutStream::device_read_from_fifo() {
    if (fifo.write_capacity() > 0) broadcast_event(not_full, 0);
  }
}
