#pragma once

#include "ptk/screen/types.h"
#include "ptk/thread.h"
#include "akt/ring.h"

namespace ptk {
  namespace screen {
    class Canvas;
    class FontBase;

    class View : public akt::Ring<View> {
    public:
      View *superview;
      View *subviews;
      Rect frame;

      View();
      virtual void set_frame(const Rect &r);
      void add_subview(View &v);
      void remove_from_superview();
      virtual void draw_self(Canvas &c);
      void draw_all(Canvas &c);
      int count_subviews() const;
      void remove_all_subviews();
      virtual Size good_size() const;
      virtual void invalidate(const Rect &r);
    };

    struct Style {
      FontBase &font;
      const pixel fg;
      const pixel bg;

      Style(FontBase &f, pixel fg, pixel bg) : font(f), fg(fg), bg(bg) {}
    };

    class StyledView : public View {
    protected:
      Style *style;

    public:
      StyledView() : style(0) {}
      virtual void set_style(Style &s) {style = &s;}
    };

    class Screen : public View, public ptk::SubThread {
    protected:
      bool is_dirty;
      Rect dirty_rect;

    public:
      Canvas &root;

      Screen(Canvas &c);

      void draw_all();

      virtual void init() = 0;
      virtual void reset();
      virtual void invalidate(const Rect &r);
    };
  }
}

