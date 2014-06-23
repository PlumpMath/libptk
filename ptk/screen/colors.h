// -*- Mode:C++ -*-

#pragma once

#include <stdint.h>

namespace ptk {
  namespace screen {
#define BYTESWAP(u16) ((((u16) & 0xff) << 8) | ((u16) >> 8))
#define RGB565(r,g,b) (((((uint16_t) r) & 0xf8) << 8) | ((((uint16_t) g) & 0xfc) << 3) | (((uint16_t) b) >> 3))

    namespace rgb565 {
      enum color {
        BLACK        = BYTESWAP(RGB565(0,   0,   0)),
        NAVY         = BYTESWAP(RGB565(0,   0,   128)),
        DARK_GREEN   = BYTESWAP(RGB565(0,   128, 0)),
        DARK_CYAN    = BYTESWAP(RGB565(0,   128, 128)),
        MAROON       = BYTESWAP(RGB565(128, 0,   0)),
        PURPLE       = BYTESWAP(RGB565(128, 0,   128)),
        OLIVE        = BYTESWAP(RGB565(128, 128, 0)),
        LIGHT_GRAY   = BYTESWAP(RGB565(192, 192, 192)),
        DARK_GRAY    = BYTESWAP(RGB565(128, 128, 128)),
        BLUE         = BYTESWAP(RGB565(0,   0,   255)),
        GREEN        = BYTESWAP(RGB565(0,   255, 0)),
        CYAN         = BYTESWAP(RGB565(0,   255, 255)),
        RED          = BYTESWAP(RGB565(255, 0,   0)),
        MAGENTA      = BYTESWAP(RGB565(255, 0,   255)),
        YELLOW       = BYTESWAP(RGB565(255, 255, 0)),
        WHITE        = BYTESWAP(RGB565(255, 255, 255)),
        ORANGE       = BYTESWAP(RGB565(255, 165, 0)),
        GREEN_YELLOW = BYTESWAP(RGB565(173, 255, 47)),
      };
    };

#undef RGB565
#undef BYTESWAP

    namespace b_and_w {
      enum color {
        WHITE = 0,
        BLACK = 1
      };
    }
  }
}

