#pragma once

#include "ptk/thread.h"
#include <stdint.h>

namespace ptk {
  enum logical_pin_t {
    CS_PIN         =  1,
    DC_PIN         =  2,
    RESET_PIN      =  4,
    ENABLE_PIN     =  5,
    MODE_PIN       =  6,
    COM_PIN        =  7,
  };

  class DisplayIO : public SubThread {
  SubThread &send_thread;
  const uint8_t *buffer, *buffer0;

  protected:
    virtual void send_data(const uint8_t *src, unsigned len) = 0;
    virtual void signal_pin(logical_pin_t pin, bool value) = 0;

  public:
    DisplayIO(SubThread &sender);
    void interpret(const uint8_t *cmds);
    virtual void run();

    enum cmd_t {
      SLEEP = 0x00,
      CS    = 0x01,
      CMDS  = 0x02,
      RESET = 0x03,
      DATA  = 0x04,
      PIN   = 0x05,
      END   = 0x06,
      ESC   = 0xff
    };
  };
}

#define VIEW_IO_SLEEP(msec) ptk::DisplayIO::ESC, ptk::DisplayIO::SLEEP, msec
#define VIEW_IO_SELECT      ptk::DisplayIO::ESC, ptk::DisplayIO::CS,    1
#define VIEW_IO_UNSELECT    ptk::DisplayIO::ESC, ptk::DisplayIO::CS,    0
#define VIEW_IO_COMMANDS    ptk::DisplayIO::ESC, ptk::DisplayIO::CMDS
#define VIEW_IO_RESET(msec) ptk::DisplayIO::ESC, ptk::DisplayIO::RESET, msec
#define VIEW_IO_DATA(len)   ptk::DisplayIO::ESC, ptk::DisplayIO::DATA,  len
#define VIEW_IO_ESC         ptk::DisplayIO::ESC, ptk::DisplayIO::ESC
#define VIEW_IO_END         ptk::DisplayIO::ESC, ptk::DisplayIO::END
#define VIEW_IO_PIN(p,lvl)  ptk::DisplayIO::ESC, ptk::DisplayIO::PIN, p, lvl
