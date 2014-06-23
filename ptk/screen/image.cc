#include "ptk/screen/image.h"
#include "ptk/screen/canvas.h"

using namespace ptk::screen;

ImageBase::ImageBase(Size s) :
  size(s)
{
}

XBMImage::XBMImage(int width, int height, const unsigned char *data, pixel color) :
  ImageBase(Size(width, height)),
  data(data),
  color(color)
{
}

void XBMImage::draw(Canvas *c, Point p) const {
  const unsigned stride = (size.w + 7)/8;

  for (int y=0; y < size.h; ++y) {
    for (int x=0; x < size.w; ++x) {
      if (data[stride*y + x/8] & (0x1 << (x % 8))) {
        c->draw_pixel(Point(p.x + x, p.y + y), color);
      }
    }
  }
}
