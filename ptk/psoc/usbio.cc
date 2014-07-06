/*
 * The PSoC USB driver depends on code that has been generated by Cypress' IDE,
 * PSoC Creator. Since the function names in the generated API depend on what
 * the IDE user named the instance of the USBFS component, it's not possible to
 * write generic code that will work in all PSoC Creator designs. The comporomise 
 * taken here is to require that the preprocessor macro _USB_NAME_ be defined as
 * the name of the USBFS instance. This definition can be placed in a Makefile,
 * or in source code that's processed before this file.
 */

#if defined(_USB_NAME_)

extern "C" {
#include "project.h"
}

#include "ptk/psoc/usbio.h"
#include <algorithm>

using namespace std;
using namespace ptk;
using namespace ptk::psoc;

#define USB_(x) USB_HELPER_(_USB_NAME_,x)
#define USB_HELPER_(x,y) USB_HELPER2_(x,y)
#define USB_HELPER2_(x,y) x##y

volatile uint8_t *PSoCUSBEndpoint::get_output_data_ptr() {
  uint8 ri = (ep_id - USB_(_EP1)) << USB_(_EPX_CNTX_ADDR_SHIFT);
  return (reg8 *) (USB_(_ARB_RW1_DR_IND) + ri);
}

size_t PSoCUSBEndpoint::get_max_buffer_size() {
  return USB_(_EP)[ep_id].bufferSize;
}

size_t PSoCUSBEndpoint::get_bytes_in_endpoint() {
  unsigned ri = ((ep_id - USB_(_EP1)) << USB_(_EPX_CNTX_ADDR_SHIFT));
  unsigned result = (uint8_t) (CY_GET_REG8((reg8 *)(USB_(_SIE_EP1_CNT0_IND) + ri)) & USB_(_EPX_CNT0_MASK));
  result = (result << 8u) | CY_GET_REG8((reg8 *)(USB_(_SIE_EP1_CNT1_IND) + ri));

  // value from hardware register includes 2 bytes for CRC, don't count them
  result -= USB_(_EPX_CNTX_CRC_COUNT);
  return result;
}

size_t PSoCUSBEndpoint::read_output_data(uint8_t *dst, unsigned limit) {
  return USB_(_ReadOutEP)(ep_id, dst, limit);
}

// extern "C"
void usbfs_endpoint_isr(int ep) {
  enter_isr();
  lock_from_isr();
  USBEndpoint::transfer_endpoint_data(ep);
  unlock_from_isr();
  leave_isr();
}

// extern "C"
void usbfs_reset_isr() {
  enter_isr();
  lock_from_isr();
  // do something here?
  unlock_from_isr();
  leave_isr();
}

namespace ptk {
  namespace psoc {
    uint8_t get_usb_cdc_input_endpoint() {
      return USB_(_cdc_data_in_ep);
    }

    uint8_t get_usb_cdc_output_endpoint() {
      return USB_(_cdc_data_out_ep);
    }

    void init_usb_driver() {
      USB_(_Start)(0, USB_(_3V_OPERATION));
    }

    void init_cdc_driver() {
      USB_(_CDC_Init)();
    }

    bool usb_is_enumerated() {
      return USB_(_GetConfiguration)() != 0;
    }
  }
}

PSoCUSBInStream::PSoCUSBInStream() :
  DeviceInStream(fifo_storage, sizeof(fifo_storage))
{
}

void PSoCUSBInStream::init(uint8_t ep) {
  USBEndpoint::init(ep);
  USB_(_EnableOutEP)(ep);
}

void PSoCUSBInStream::transfer() {
  // assume lock_from_isr() has already been called

  unsigned bytes_in_endpoint = get_bytes_in_endpoint();
  unsigned space_in_fifo = fifo.write_capacity();
  reg8 *src = get_output_data_ptr();

  // Unfortunately, the way Cypress' code is generated, read_out_ep_data()
  // must be called exactly once after the endpoint contains data. Any data
  // not read in the first call will be discarded. This makes it difficult
  // to copy endpoint data into a ring buffer, since it may take two copy
  // operations to move everything. To get around this, we do the copying
  // ourselves and then make a final "dummy" call to read_out_ep_data() with
  // a length of zero. This call won't actually move any more data, but
  // will reset the endpoint so it can receive more data from the host.

  while ((bytes_in_endpoint > 0) && (space_in_fifo > 0)) {
    unsigned bytes_to_copy = min(bytes_in_endpoint, space_in_fifo);

    uint8_t *dst = &fifo.poke(0);
    for (unsigned i=bytes_to_copy; i; --i) {
      uint8_t ch = CY_GET_REG8(src);
      *dst++ = ch;
    }

    fifo.fake_write(bytes_to_copy);
    bytes_in_endpoint -= bytes_to_copy;

    // recalculate space because the pointers may have wrapped inside the FIFO
    space_in_fifo = fifo.write_capacity();
  }

  uint8 *arbitrary_non_NULL_ptr = (uint8 *) 0xdeadbeef;
  uint16 zero_bytes = 0;

  // NOTE: this code requires manual endpoint memory management
  // as defined in the USBFS device descriptor
  read_output_data(arbitrary_non_NULL_ptr, zero_bytes);

  if (bytes_in_endpoint > 0) {
    // buffer overflow. what to do?
  }

  device_wrote_to_fifo();
  // assume unlock_from_isr() will be called
}

PSoCUSBOutStream::PSoCUSBOutStream() :
  DeviceOutStream(fifo_storage, sizeof(fifo_storage))
{
}

unsigned PSoCUSBOutStream::write(const uint8_t *buffer, const unsigned len0) {
  unsigned len = len0;
  lock_kernel();
  bool save_unwritten_data = true;

  // if the fifo already has data in it, the new data has to wait
  if (fifo.read_capacity() > 0) {
    unsigned saved = fifo.write(buffer, len);

    // if the fifo didn't have room, say how much was left over
    if (saved < len) {
      return saved;
    } else {
      buffer = &fifo.peek(0);
      len = fifo.read_capacity();

      // if there's data left over later, remember that it's already saved
      save_unwritten_data = false;
    }
  }

  uint8_t usb_state;
  if ((usb_state = USB_(_GetEPState)(ep_id)) == USB_(_IN_BUFFER_EMPTY)) {
    // send as much as we can
    unsigned bytes_to_copy = min(get_max_buffer_size(), len);
    USB_(_LoadInEP)(ep_id, buffer, bytes_to_copy);
    buffer += bytes_to_copy;
    len -= bytes_to_copy;
  } else {
    bool usb_is_busy = true;
  }

  if ((len > 0) && save_unwritten_data) {
    len -= fifo.write(buffer, len);
  }

  if (len > 0) {
    // buffer overflow. what to do?
  }

  unlock_kernel();

  return len0 - len;
}

void PSoCUSBOutStream::transfer() {
  // assume lock_from_isr() has been called

  unsigned bytes_in_fifo = fifo.read_capacity();
  if (bytes_in_fifo > 0) {
    unsigned bytes_to_copy = min(get_max_buffer_size(), bytes_in_fifo);
    USB_(_LoadInEP)(ep_id, &fifo.peek(0), bytes_to_copy);
    fifo.fake_read(bytes_to_copy);

    device_read_from_fifo();
  }
  // assume unlock_from_isr() will be called
}

#if 0
USBEcho::USBEcho() {
}

void USBEcho::run() {
  PTK_BEGIN();

  init_usb_device();
  // wait until it's enumerated on the USB bus
  PTK_WAIT_UNTIL(is_enumerated(), TIME_INFINITE);
  init_cdc();

  rx.init(USB_(_cdc_data_out_ep));
  tx.init(USB_(_cdc_data_in_ep));

  while (1) {
    PTK_WAIT_EVENT(rx.not_empty, TIME_INFINITE);

    unsigned bytes_copied;
    do {
      uint8_t buf[8];

      bytes_copied = rx.read(buf, sizeof(buf));
      tx.write(buf, bytes_copied);
    } while (bytes_copied > 0);
  }
  PTK_END();
}
#endif

PSoCEcho::PSoCEcho() :
  echo(rx, tx)
{ }

void PSoCEcho::run() {
  PTK_BEGIN();

  init_usb_driver();
  PTK_WAIT_UNTIL(usb_is_enumerated(), TIME_INFINITE);
  init_cdc_driver();

  rx.init(get_usb_cdc_output_endpoint());
  tx.init(get_usb_cdc_input_endpoint());

  PTK_WAIT_SUBTHREAD(echo, TIME_INFINITE);
  PTK_END();
}

void CDCDriver::init() {
  init_usb_driver();
}

bool CDCDriver::is_enumerated() {
  return usb_is_enumerated();
}

void CDCDriver::init_cdc() {
  in.init(get_usb_cdc_output_endpoint());
  out.init(get_usb_cdc_input_endpoint());
}

#endif // defined(_USB_NAME_)
