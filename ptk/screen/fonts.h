// -*- Mode:C++ -*-

#pragma once

#include "ptk/screen/types.h"

namespace ptk {
  namespace screen {
    class Canvas;

    class FontBase {
    public:
      Size size;
      uint16_t offset;

      static const char *end_of_line(const char *str);
      virtual coord text_width(const char *str, unsigned len) const = 0;
      virtual coord text_height(unsigned line_count) const = 0;
      virtual Size measure(const char *str) const = 0;
      virtual void draw_char(Canvas *c, Point p, char ch, pixel value) const = 0;
      virtual void draw_string(Canvas *c, Point p, const char *str, pixel value) const = 0;
    };

    /*
     * This class handles fonts generated by the MikroElektronica GLCD Font Creator
     * program found at http://www.mikroe.com/glcd-font-creator/
     *
     * The output of the program is an array of byte values that define the width
     * of each glyph in the font and which bits are set. Black pixels in the editor UI
     * correspond to '1' bits in the data, and '0' means white. The bit data for
     * each glyph is preceeded by a one byte integer specifying the rendered width
     * of that particular glyph. This allows proportionally spaced fonts.
     *
     * Glyphs are rasterized in column major order with the least significant bits
     * at the top. Consider the following 5x7 glyph for the numeral '1':
     *
     *
     *      0 0 1 0 0                 5 bytes of bit data
     *      0 1 1 0 0               ______________________
     *      0 0 1 0 0              v                      v
     *      0 0 1 0 0  ==> 0x04, 0x00, 0x42, 0x7f, 0x40, 0x00
     *      0 0 1 0 0        ^
     *      0 0 1 0 0         \
     *      0 1 1 1 0           1 byte of width
     */

    class MikroFont : public FontBase {
      enum {
        GAP = 1, // minimum space between rendered glyphs
      };

      const uint8_t *data;
      uint16_t height_in_bytes;
      uint16_t glyph_stride;
      unsigned char first, last;

    public:
      typedef struct {
        uint8_t width;
        uint8_t data[];
      } glyph_t;

      MikroFont(const uint8_t *data, const Size &size, uint16_t offset, char first, char last);

      inline const glyph_t *glyph(unsigned char c) const {
        if (c < first || c > last) c = first;
        return (glyph_t *) (data + (c-first)*glyph_stride);
      }

      coord text_width(const char *str, unsigned len) const;
      virtual coord text_height(unsigned line_count) const;
      virtual Size measure(const char *str) const;
      uint16_t draw_char1(Canvas *c, Point p, char ch, pixel value) const;
      void draw_char(Canvas *c, Point p, char ch, pixel value) const;
      void draw_string(Canvas *c, Point p, const char *str, pixel value) const;
    };
  }
}
