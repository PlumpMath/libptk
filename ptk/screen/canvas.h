// -*- Mode:C++ -*-

#pragma once

#include "ptk/screen/types.h"

#include <cstdint>

namespace ptk {
  namespace screen {
    class FontBase;
    class ImageBase;

    class Canvas {
    protected:
      virtual void set_pixel(Point p, pixel value) = 0;
      virtual pixel get_pixel(Point p) const = 0;

    public:
      const Size size;
      const Rect bounds;
      Rect clip;

      Canvas(Size s);
      virtual void reset();
      virtual void fill_rect(const Rect &r, pixel value);
      virtual void draw_rect(const Rect &r, pixel value);
      virtual void draw_pixel(Point p, pixel value);
      virtual void draw_line(Point p0, Point p1, pixel value);
      virtual void draw_string(Point p, const char *str, const FontBase &f, pixel value);
      virtual void draw_image(Point p, ImageBase &b);
    };
  }
}
