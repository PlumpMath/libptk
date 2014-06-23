// -*- Mode:C++ -*-

#pragma once

#include "ptk/screen/types.h"

namespace ptk {
  namespace screen {
    class Canvas;

    class ImageBase {
    public:
      ImageBase(Size s);
      virtual void draw(Canvas *c, Point p) const = 0;

      const Size size;
    };

    class XBMImage : public ImageBase {
      const unsigned char * const data;

    public:
      XBMImage(int width, int height, const unsigned char *data, pixel color);
      virtual void draw(Canvas *c, Point p) const;

      const pixel color;
    };

    
  }
}
