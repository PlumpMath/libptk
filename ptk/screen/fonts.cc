#include "ptk/screen/fonts.h"
#include "ptk/screen/canvas.h"

using namespace ptk::screen;

const char *FontBase::end_of_line(const char *str) {
  if (!str) return 0;
  while (1) {
    switch (*str) {
    case '\0' :
    case '\n' :
      return str;

    default :
      str += 1;
    }
  }

  // not reached
  return 0;
}

MikroFont::MikroFont(const uint8_t *data, const Size &s, uint16_t offset, char first, char last) :
  data(data),
  first((unsigned char) first),
  last((unsigned char) last)
{
  this->size = s;
  this->offset = offset;
  height_in_bytes = (size.h + 7)/8;
  glyph_stride = height_in_bytes*size.w + 1; // first byte in each glyph is width
}

coord MikroFont::text_width(const char *str, unsigned len) const {
  coord width = 0;

  for (unsigned i=0; i < len; ++i) {
    const glyph_t *g = glyph((unsigned char) str[i]);
    width += g->width;
  }

  if (len > 0) width += GAP*(len - 1);

  return width;
}

coord MikroFont::text_height(unsigned line_count) const {
  coord result = line_count*size.h;
  if (line_count > 1) result += (line_count - 1)*GAP;
  return result;
}

Size MikroFont::measure(const char *str) const {
  Size s(0,0);
  unsigned line_count = 0;

  while (1) {
    const char *end = end_of_line(str);
    coord w = text_width(str, end - str);
    if (w > s.w) s.w = w;
    s.h += size.h;
    line_count += 1;

    str = end;

    if (*str == '\0') {
      break;
    } else if (*str == '\n') {
      str++;
    }
  }

  if (line_count > 0) s.h += GAP*(line_count - 1);
  return s;
}

uint16_t MikroFont::draw_char1(Canvas *c, Point origin, char ch, pixel value) const {
  const glyph_t *g = glyph((unsigned char) ch);

  for (int x=0; x < g->width; ++x) {
    for (int y=0; y < size.h; ++y) {
      if (g->data[x*height_in_bytes + y/8] & (1 << (y & 7))) {
        Point p = Point(origin.x + x, origin.y + y);
        if (c->clip.contains(p)) c->draw_pixel(p, value);
      }
    }
  }

  return g->width + GAP;
}

void MikroFont::draw_char(Canvas *c, Point p, char ch, pixel value) const {
  draw_char1(c, p, ch, value);
}

void MikroFont::draw_string(Canvas *c, Point p0, const char *str, pixel value) const {
  Point p = p0;

  while (*str) {
    char ch = *str++;

    switch (ch) {
    case '\n' :
      p.x = p0.x;
      p.y += size.h;
      break;

    default :
      p.x += draw_char1(c, p, ch, value);
      break;
    }
  }
}
