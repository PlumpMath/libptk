#pragma once

#include "ptk/thread.h"
#include "ptk/event.h"
#include "akt/views/views.h"

#include <stdint.h>

namespace ptk {
  namespace screen {
    enum logical_pin_t {
      CS_PIN         =  1,
      DC_PIN         =  2,
      RESET_PIN      =  4,
      ENABLE_PIN     =  5,
      MODE_PIN       =  6,
      COM_PIN        =  7,
    };

    class IO : protected SubThread {
    protected:
      const uint8_t *buffer, *buffer0;
      virtual void signal_pin(logical_pin_t pin, bool value) = 0;
      virtual void run();

    public:
      IO();
      virtual void init() = 0;
      SubThread &interpret(const uint8_t *cmds);
      virtual SubThread &send_data(const uint8_t *data, unsigned len) = 0;

    public:

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
}

#define DISPLAY_IO_SLEEP(msec) ptk::screen::IO::ESC, ptk::screen::IO::SLEEP, msec
#define DISPLAY_IO_SELECT      ptk::screen::IO::ESC, ptk::screen::IO::CS,    1
#define DISPLAY_IO_UNSELECT    ptk::screen::IO::ESC, ptk::screen::IO::CS,    0
#define DISPLAY_IO_COMMANDS    ptk::screen::IO::ESC, ptk::screen::IO::CMDS
#define DISPLAY_IO_RESET(msec) ptk::screen::IO::ESC, ptk::screen::IO::RESET, msec
#define DISPLAY_IO_DATA(len)   ptk::screen::IO::ESC, ptk::screen::IO::DATA,  len
#define DISPLAY_IO_ESC         ptk::screen::IO::ESC, ptk::screen::IO::ESC
#define DISPLAY_IO_END         ptk::screen::IO::ESC, ptk::screen::IO::END
#define DISPLAY_IO_PIN(p,lvl)  ptk::screen::IO::ESC, ptk::screen::IO::PIN, p, lvl

namespace ptk {
  namespace screen {
    class SSD1306_128x32 : public akt::views::Canvas, public SubThread {
    public:
      enum {W=128, H=32};
      Event redraw;
      SSD1306_128x32(ptk::screen::IO &io);

      virtual void flush(const akt::views::Rect &r);
      virtual void reset();

    protected:
      uint8_t pages[H/8][W];
      ptk::screen::IO &io;
      akt::views::Rect flush_rect;

      virtual void set_pixel(akt::views::Point p, akt::views::pixel value);
      virtual akt::views::pixel get_pixel(akt::views::Point p) const;
      virtual void run();
    };
  }
}
