// -*- Mode:C++ -*-

#pragma once

#include <cstdint>

namespace ptk {
  namespace screen {
    typedef int16_t coord;
    typedef uint16_t pixel;

    class Size;

    enum display_orientation_t {
      NORMAL,
      ROTATE180
    };

    struct Point {
      coord x, y;

      Point();
      Point(int x, int y);

      Point operator+(Point p) const;
      Point operator-(Point p) const;
      Point operator+(Size s)  const;
      Point operator-(Size s)  const;
      Point operator+(int n)   const;
      Point operator-(int n)   const;

      Point &operator +=(const Point &p);
    };

    struct Size {
      coord w, h;

      Size();
      Size(int w, int h);
      bool empty() const;
      Size operator/(int n) const;
      Size operator*(int n) const;
      Size operator+(int n) const;
      Size operator-(int n) const;
    };

    class Rect {
      enum {
        INSIDE = 0,
        LEFT   = 1 << 0,
        RIGHT  = 1 << 1,
        BOTTOM = 1 << 2,
        TOP    = 1 << 3
      };

      int outcode(const Point &p) const;

    public:
      Point min, max;

      Rect();
      Rect(int x, int y, int w, int h);
      Rect(const Point &p0, const Point &p1);
      Rect(const Point &p, const Size &s);

      inline coord width()  const;
      inline coord height() const;
      inline Size  size()   const;
      inline bool  empty()  const;
      inline bool  normal() const;

      void normalize();
      bool intersects(const Rect &r) const;
      bool contains(const Point &p) const;
      bool contains(const Rect &r) const;

      Rect &operator +=(const Point &p);
      Rect &operator =(const Point &p);
      Rect operator &(const Rect &right) const;
      Rect operator +(Size s);
      Rect operator -(Size s);
      Point center() const;

      bool clip(Point &p0, Point &p1) const;
    };

    Rect center_size_within(const Rect &r, const Size &s);

    inline Point Point::operator+(Point p) const {return Point(x+p.x, y+p.y);}
    inline Point Point::operator-(Point p) const {return Point(x-p.x, y-p.y);}
    inline Point Point::operator+(Size s)  const {return Point(x+s.w, y+s.h);}
    inline Point Point::operator-(Size s)  const {return Point(x-s.w, y-s.h);}
    inline Point Point::operator+(int n)   const {return Point(x+n,   y+n);}
    inline Point Point::operator-(int n)   const {return Point(x-n,   y-n);}


    inline bool Size::empty() const {return w*h == 0;}
    inline Size Size::operator/(int n) const {return Size(w/n, h/n);}
    inline Size Size::operator*(int n) const {return Size(w*n, h*n);}
    inline Size Size::operator+(int n) const {return Size(w+n, h+n);}
    inline Size Size::operator-(int n) const {return Size(w-n, h-n);}

    inline coord Rect::width()  const {return max.x - min.x;}
    inline coord Rect::height() const {return max.y - min.y;}
    inline Size  Rect::size()   const {return Size(width(), height());}
    inline bool  Rect::empty()  const {return width()*height() == 0;}
    inline bool  Rect::normal() const {return (max.x >= min.x) && (max.y >= min.y);}
  }
}

bool operator ==(const ptk::screen::Rect &left, const ptk::screen::Rect &right);
ptk::screen::Rect operator |(const ptk::screen::Rect &left, const ptk::screen::Rect &right);

inline bool operator==(const ptk::screen::Size &s1, const ptk::screen::Size &s2) {
  return s1.w == s2.w && s1.h == s2.h;
}

inline bool operator==(const ptk::screen::Point &p1, const ptk::screen::Point &p2) {
  return p1.x == p2.x && p1.y == p2.y;
}

