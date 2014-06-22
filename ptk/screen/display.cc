#include "ptk/screen/display.h"
#include "ptk/kernel.h"

using namespace ptk;
using namespace ptk::screen;

IO::IO() :
  SubThread()
{
}

SubThread &IO::interpret(const uint8_t *cmds) {
  buffer = cmds;
  return *this;
}

void IO::run() {
  PTK_BEGIN();

  while (buffer) {
    buffer0 = buffer;

    // find next escape sequence
    while (*buffer != ESC) ++buffer;

    if (buffer0 < buffer) {
      // send non-escaped bytes as is
      PTK_WAIT_SUBTHREAD(send_data(buffer0, (buffer - buffer0)), TIME_INFINITE);

      // note that buffer has already been incremented to the
      // next escape code
    }

    PTK_ASSERT(*buffer == ESC,
               "Excpected ESC in DisplayIO commands");
    buffer += 1;

    // handle the escape
    switch (*buffer++) {
    case SLEEP :
      PTK_SLEEP((unsigned) *buffer++);
      break;

    case CS : // chip select (arg == 0 when display is *NOT* selected)
      signal_pin(CS_PIN, 0 == *buffer++);
      break;

    case CMDS : // commands follow
      signal_pin(DC_PIN, false);
      break;

    case RESET : // send a reset pulse (arg == pulse width in msec)
      signal_pin(RESET_PIN, true); // active low
      PTK_SLEEP((unsigned) buffer[0]);
      signal_pin(RESET_PIN, false); // active low
      PTK_SLEEP((unsigned) buffer[0]);
      signal_pin(RESET_PIN, true); // active low

      buffer += 1;
      break;

    case DATA : {
      unsigned len = *buffer++;
      const uint8_t *data = buffer;
      buffer += len;
      signal_pin(DC_PIN, true);
      if (len > 0) PTK_WAIT_SUBTHREAD(send_data(data, len), TIME_INFINITE);
      break;
    }

    case PIN :
      signal_pin((logical_pin_t) buffer[0], buffer[1]);
      buffer += 2;
      break;

    case END :
      buffer = 0;
      break;

    case ESC :
      // buffer has already been incremented, so subtract one to get a
      // pointer to the ESC byte (which is what we need to send).
      PTK_WAIT_SUBTHREAD(send_data(buffer-1, 1), TIME_INFINITE);
      break;

    default :
      PTK_ASSERT(false,
                 "Unrecognized command byte in DisplayIO");
    }
  }
  PTK_END();
}

SSD1306_128x32::SSD1306_128x32(ptk::screen::IO &io) :
  akt::views::Canvas(akt::views::Size(W, H)),
  io(io)
{
  uint8_t count=0;

  for (unsigned page=0; page < H/8; ++page) {
    for (unsigned col=0; col < W; ++col) {
      pages[page][col] = count++;
    }
  }
}

void SSD1306_128x32::reset() {
  for (;;);
}

void SSD1306_128x32::flush(const akt::views::Rect &r) {
  flush_rect = r;
  signal_event(redraw, 0);
}

void SSD1306_128x32::set_pixel(akt::views::Point p, akt::views::pixel value) {
  assert(p.x >= 0 && p.x < (int16_t) W);
  assert(p.y >= 0 && p.y < (int16_t) H);

  uint8_t mask = 0x01 << (p.y % 8);

  if (value) {
    pages[p.y/8][p.x] |=  mask;
  } else {
    pages[p.y/8][p.x] &= ~mask;
  }
}

akt::views::pixel SSD1306_128x32::get_pixel(akt::views::Point p) const {
  assert(p.x >= 0 && p.x < (int16_t) W);
  assert(p.y >= 0 && p.y < (int16_t) H);

  uint8_t mask = 0x01 << (p.y % 8);

  return (pages[p.y/8][p.x] & mask) ? 1 : 0;
}

static const uint8_t reset_sequence[] = {
  DISPLAY_IO_UNSELECT,
  DISPLAY_IO_RESET(1),
  DISPLAY_IO_SLEEP(10),
  DISPLAY_IO_COMMANDS,
  DISPLAY_IO_SELECT,
  0xae,          // display off
  0xd5, 0x80,    // clock divide ratio
  0xa8, 0x1f,    // multiplex ratio
  0xd3, 0x00,    // display offset
  0x40,          // display start line
  0x8d, 0x14,    // charge pump
  0x20, 0x00,    // horizontal memory mode
  0xa1,          // segment remap
  0xc8,          // com output scan direction
  0xda, 0x02,    // com pins hardware configuration (2nd byte = 0x02 for 32 rows)
  0x81, 0x8f,    // contrast control
  0xd9, 0xf1,    // pre-charge period
  0xdb, 0x40,    // VCOMH deselect level
  0xa4,          // display pixels from RAM
  0xa6,          // display normal (1==on)
  0xaf,          // display on
  DISPLAY_IO_UNSELECT,
  DISPLAY_IO_END
};


static const uint8_t framebuffer_prologue[] = {
  DISPLAY_IO_COMMANDS,
  DISPLAY_IO_SELECT,

  0x21, 0, 127,     // set column start=0, end=127
  0x22, 0, (SSD1306_128x32::H/8)-1, // set page start=0, end=H/8

  DISPLAY_IO_DATA(0),  // setup for framebuffer DMA
  DISPLAY_IO_END
};

static const uint8_t framebuffer_epilogue[] = {
  DISPLAY_IO_UNSELECT,
  DISPLAY_IO_END
};

void SSD1306_128x32::run() {
  PTK_BEGIN();

  io.init();
  PTK_WAIT_SUBTHREAD(io.interpret(reset_sequence), TIME_INFINITE);

  for (;;) {
    PTK_WAIT_SUBTHREAD(io.interpret(framebuffer_prologue), TIME_INFINITE);
    PTK_WAIT_SUBTHREAD(io.send_data((const uint8_t *) pages, sizeof(pages)), TIME_INFINITE);
    PTK_WAIT_SUBTHREAD(io.interpret(framebuffer_epilogue), TIME_INFINITE);

    PTK_WAIT_EVENT(redraw, TIME_INFINITE);
  }

  PTK_END();
}
