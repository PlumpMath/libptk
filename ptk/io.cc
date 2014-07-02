#include "ptk/io.h"

using namespace ptk;

size_t DeviceInStream::read(uint8_t *buffer, size_t len) {
  return fifo.read(buffer, len);
}

bool DeviceInStream::get(uint8_t &ch) {
  return fifo.read(&ch, 1) == 1;
}

size_t DeviceOutStream::write(const uint8_t *buffer, size_t len) {
  return fifo.write(buffer, len);
}

bool DeviceOutStream::put(uint8_t ch) {
  return fifo.write(&ch, 1) == 1;
}

USBEndpoint *USBEndpoint::eps[MAX_ENDPOINTS];

void USBEndpoint::init(uint8_t ep) {
  this->ep_id = ep;
  eps[ep] = this;
}

void USBEndpoint::transfer_endpoint_data(uint8_t id) {
  if (id < MAX_ENDPOINTS && eps[id]) {
    eps[id]->transfer();
  }
}
