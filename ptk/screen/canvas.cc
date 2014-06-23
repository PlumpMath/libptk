#include "ptk/screen/canvas.h"
#include "ptk/screen/fonts.h"
#include "ptk/screen/image.h"

#include <algorithm>

using namespace ptk::screen;

Canvas::Canvas(Size s) :
  size(s),
  bounds(Point(0, 0), s),
  clip(bounds)
{
}

void Canvas::reset() {
  clip = bounds;
}

void Canvas::fill_rect(const Rect &r, pixel value) {
  Rect cr = r & clip;

  unsigned int xlim = cr.max.x, ylim = cr.max.y;
  for (unsigned int x=cr.min.x; x < xlim; ++x) {
    for (unsigned int y=cr.min.y; y < ylim; ++y) {
      set_pixel(Point(x, y), value);
    }
  }
}

void Canvas::draw_rect(const Rect &r, pixel value) {
  Point bottom_right(r.max.x-1, r.max.y-1);

  draw_line(r.min, Point(bottom_right.x, r.min.y), value);
  draw_line(r.min, Point(r.min.x, bottom_right.y), value);
  draw_line(bottom_right, Point(r.min.x, bottom_right.y), value);
  draw_line(bottom_right, Point(bottom_right.x, r.min.y), value);
}

void Canvas::draw_pixel(Point p, pixel value) {
  // check p+1 because the pixel extends down and right
  if (clip.contains(p+1)) set_pixel(p, value);
}

#if USE_SIMPLIFIED_BRESENHAM
void Canvas::draw_line(Point p0, Point p1, pixel value) {
  if (!clip.clip(p0, p1)) return;

  if (p0.x == p1.x) { // vertical line
    coord ymax = std::max(p0.y, p1.y);
    for (coord y=std::min(p0.y, p1.y); y <= ymax; ++y) set_pixel(Point(p0.x, y), value);
  } else if (p0.y == p1.y) {
    coord xmax = std::max(p0.x, p1.x);
    for (coord x=std::min(p0.x, p1.x); x <= xmax; ++x) set_pixel(Point(x, p0.y), value);
  } else {
    int dx = std::abs(p1.x-p0.x), dy = std::abs(p1.y-p0.y);
    int sx = (p0.x < p1.x) ? 1 : -1, sy = (p0.y < p1.y) ? 1 : -1;
    int err = dx-dy;

    while (true) {
      set_pixel(p0, value);
      if (p0.x == p1.x && p0.y == p1.y) return;
      int e2 = 2*err;
      if (e2 > -dy) {
        err -= dy;
        p0.x += sx;
      } else if (e2 < dx) {
        err += dx;
        p0.y += sy;
      }
    }
  }
}
#else
void Canvas::draw_line(Point p0, Point p1, pixel value) {
  if (!clip.clip(p0, p1)) return;

  if (p0.x == p1.x) { // vertical line
    coord ymax = std::max(p0.y, p1.y);
    for (coord y=std::min(p0.y, p1.y); y <= ymax; ++y) set_pixel(Point(p0.x, y), value);
  } else if (p0.y == p1.y) {
    coord xmax = std::max(p0.x, p1.x);
    for (coord x=std::min(p0.x, p1.x); x <= xmax; ++x) set_pixel(Point(x, p0.y), value);
  } else {
    bool steep = std::abs(p1.y - p0.y) > std::abs(p1.x - p0.x);

    if (steep) {
      std::swap(p0.x, p0.y);
      std::swap(p1.x, p1.y);
    }

    if (p0.x > p1.x) {
      std::swap(p0, p1);
    }

    coord dx = p1.x - p0.x;
    coord dy = std::abs(p1.y - p0.y);

    coord error = dx/2;
    coord y = p0.y;
    coord ystep = (p0.y < p1.y) ? 1 : -1;

    for (coord x=p0.x; x <= p1.x; ++x) {
      if (steep) {
        set_pixel(Point(y, x), value);
      } else {
        set_pixel(Point(x, y), value);
      }

      if ((error -= dy) < 0) {
        y += ystep;
        error += dx;
      }
    }
  }
}
#endif


void Canvas::draw_string(Point p, const char *str, const FontBase &f, pixel value) {
  f.draw_string(this, p, str, value);
}


void Canvas::draw_image(Point p, ImageBase &img) {
  img.draw(this, p);
}

