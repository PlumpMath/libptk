#include "ptk/display.h"
#include "ptk/kernel.h"

using namespace ptk;

DisplayIO::DisplayIO(Event &evt) :
  SubThread(),
  send_complete(evt)
{
}

void DisplayIO::interpret(const uint8_t *cmds) {
  buffer = cmds;
}

void DisplayIO::run() {
  PTK_BEGIN();

  while (buffer) {
    buffer0 = buffer;

    // find next escape sequence
    while (*buffer != ESC) ++buffer;

    if (buffer0 < buffer) {
      // send non-escaped bytes as is
      send_data(buffer0, (buffer - buffer0));
      PTK_UNLOCK_WAIT_EVENT(send_complete, TIME_INFINITE);

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
      signal_pin(DC_PIN, true);
      send_data(buffer, len);
      buffer += len;
      PTK_UNLOCK_WAIT_EVENT(send_complete, TIME_INFINITE);
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
      send_data(buffer-1, 1);
      PTK_UNLOCK_WAIT_EVENT(send_complete, TIME_INFINITE);
      break;

    default :
      PTK_ASSERT(false,
                 "Unrecognized command byte in DisplayIO");
    }
  }
  PTK_END();
}
