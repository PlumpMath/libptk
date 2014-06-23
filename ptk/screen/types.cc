// -*- Mode:C++ -*-

#include "ptk/screen/types.h"
#include "ptk/assert.h"
#include <algorithm>

using namespace ptk::screen;
Point::Point() {
}

Point::Point(int x, int y) :
  x(x),
  y(y)
{}

Point &Point::operator +=(const Point &p) {
  x += p.x;
  y += p.y;
  return *this;
}

Size::Size() {
}

Size::Size(int w, int h) :
  w(w),
  h(h)
{}

Rect::Rect() {
}

Rect::Rect(int x, int y, int w, int h) :
  min(x, y),
  max(x + w, y + h)
{}

Rect::Rect(const Point &p0, const Point &p1) :
  min(p0),
  max(p1)
{
  normalize();
}

Rect::Rect(const Point &p, const Size &s) :
  min(p),
  max(p.x + s.w, p.y + s.h)
{
  normalize();
}

void Rect::normalize() {
  Point p1(std::min(min.x, max.x), std::min(min.y, max.y));
  Point p2(std::max(min.x, max.x), std::max(min.y, max.y));
  min = p1;
  max = p2;
}

bool Rect::intersects(const Rect &r) const {
  Rect i = r & *this;
  return ((i.max.x - i.min.x) > 0) && ((i.max.y - i.min.y) > 0);
}

bool Rect::contains(const Point &p) const {
  return (p.x >= min.x) && (p.x <= max.x) &&
    (p.y >= min.y) && (p.y <= max.y);
}

bool Rect::contains(const Rect &r) const {
  return contains(r.min) && contains(r.max);
}

Rect &Rect::operator +=(const Point &p) {
  min += p;
  max += p;
  return *this;
}

Rect &Rect::operator=(const Point &p) {
  coord w = width(), h = height();
  min = p;
  max.x = min.x + w;
  max.y = min.y + h;
  return *this;
}

Rect Rect::operator +(Size s) {
  return Rect(min.x + s.w, min.y + s.h, max.x + s.w, max.y + s.h);
}

Rect Rect::operator -(Size s) {
  return Rect(min.x - s.w, min.y - s.h, max.x - s.w, max.y - s.h);
}

Point Rect::center() const {
  return Point((min.x + max.x)/2, (min.y + max.y)/2);
}

/*
 * Note that a point that lies on the right or bottom edges of
 * a rectangle generates a non-zero outcode even though the point
 * is technically "within" the rectangle. This is because the
 * main purpose of outcode() is to generate points suitable for
 * line drawing and points on these two edges will actually be
 * rendered outside the rectangle.
 */
int Rect::outcode(const Point &p) const {
  int code = INSIDE;

  if (p.x <  min.x) code |= LEFT;
  if (p.x >= max.x) code |= RIGHT;
  if (p.y <  min.y) code |= TOP;
  if (p.y >= max.y) code |= BOTTOM;

  return code;
}

/*
 * This is a tweaked version of the Cohen-Sutherland algorithm
 * for clipping 2D lines.
 *
 * See http://en.wikipedia.org/wiki/Cohen-Sutherland_algorithm
 */
bool Rect::clip(Point &p0, Point &p1) const {
  int out0 = outcode(p0), out1 = outcode(p1);

  while (true) {
    if ((out0 | out1) == INSIDE) return true;
    if (out0 & out1) return false;
    int out = out0 ? out0 : out1;
    int dx = (p1.x - p0.x), dy = (p1.y - p0.y);
    Point p;

    if (out & TOP) {
      p.x = p0.x + dx*(min.y - p0.y)/dy;
      p.y = min.y;
    } else if (out & BOTTOM) {
      p.x = p0.x + dx*(max.y - (p0.y + 1))/dy;
      p.y = max.y - 1;
    } else if (out & RIGHT) {
      p.y = p0.y + dy*(max.x - (p0.x + 1))/dx;
      p.x = max.x - 1;
    } else if (out & LEFT) {
      p.y = p0.y + dy*(min.x - p0.x)/dx;
      p.x = min.x;
    }

    if (out == out0) {
      out0 = outcode((p0 = p));
    } else {
      out1 = outcode((p1 = p));
    }
  }
}

bool operator==(const Rect &r1, const Rect &r2) {
  return r1.min == r2.min && r1.max == r2.max;
}

Rect Rect::operator &(const Rect &right) const {
  assert(normal() && right.normal());

  Rect i; // intersection

  if (max.x >= right.min.x && min.x <= right.max.x) {
    i.min.x = std::max(min.x, right.min.x);
    i.max.x = std::min(max.x, right.max.x);
  } else {
    i.min.x = max.x; // abnormal
    i.max.x = min.x;
  }

  if (max.y >= right.min.y && min.y <= right.max.y) {
    i.min.y = std::max(min.y, right.min.y);
    i.max.y = std::min(max.y, right.max.y);
  } else {
    i.min.y = max.y; // abnormal
    i.max.y = min.y;
  }

  return i;
}

Rect operator |(const Rect &left, const Rect &right) {
  assert(left.normal() && right.normal());

  Rect u; // union

  u.min.x = std::min(left.min.x, right.min.x);
  u.min.y = std::min(left.min.y, right.min.y);
  u.max.x = std::max(left.max.x, right.max.x);
  u.max.y = std::max(left.max.y, right.max.y);

  return u;
}

Rect center_size_within(const Rect &r, const Size &s) {
  Point upper_left = r.center() - s/2;
  return Rect(upper_left, upper_left + s);
}
