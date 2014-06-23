#include "ptk/screen/view.h"
#include "ptk/screen/canvas.h"

#include "ptk/assert.h"

using namespace ptk::screen;

View::View() :
  superview(0),
  subviews(0),
  frame(0, 0, 0, 0)
{
}

void View::add_subview(View &sub) {
  assert(sub.superview == 0);

  if (subviews) {
    sub.join(subviews);
  } else {
    subviews = &sub;
  }
  sub.superview = this;
}

void View::set_frame(const Rect &r) {
  frame = r;
}

void View::remove_from_superview() {
  if (superview) {
    if (superview->subviews == this) {
      superview->subviews = empty() ? 0 : begin();
    }
    join(this);
    superview = 0;
  }
}

void View::draw_self(Canvas &c) {
}

void View::draw_all(Canvas &c) {
  Rect original_clip = c.clip;

  c.clip = frame & original_clip;
  draw_self(c);

  if (subviews) {
    View *sub = subviews;
    do {
      if (sub->frame.intersects(original_clip)) {
        c.clip = sub->frame & original_clip;
        sub->draw_all(c);
      }
      sub = (View *) sub->left;
    } while (sub != subviews);
  }

  c.clip = original_clip;
}

int View::count_subviews() const {
  int count=0;
  if (subviews) {
    View *sub = subviews;
    do {
      ++count;
      sub = (View *) sub->left;
    } while (sub != subviews);
  }
  return count;
}

void View::remove_all_subviews() {
  if (subviews) {
    View *sub = subviews, *last, *next;
    do {
      last = sub;
      next = (View *) sub->left;

      sub->superview = 0;
      sub->join(sub);
      sub = next;
    } while (next != last);

    subviews = 0;
  }
}


Size View::good_size() const {
  return Size(0,0);
}

void View::invalidate(const Rect &r) {
  if (superview) superview->invalidate(r & frame);
}

Screen::Screen(Canvas &c) :
  View(),
  SubThread(),
  root(c)
{
}

void Screen::init() {
}

void Screen::reset() {
  root.reset();
  frame = root.bounds;
  is_dirty = false;
  dirty_rect = Rect(0, 0, 0, 0);
}

void Screen::draw_all() {
  View::draw_all(root);
}

void Screen::invalidate(const Rect &r) {
  if (!r.empty()) {
    if (is_dirty) {
      dirty_rect = dirty_rect | (r & root.bounds);
    } else {
      dirty_rect = r;
    }

    is_dirty = true;
  }
}

#if 0
void Screen::update_if_dirty() {
  Rect dirty_rect_copy(dirty_rect);
  bool is_dirty_copy(is_dirty);

  dirty_rect = Rect(0, 0, 0, 0);
  is_dirty = false;

  if (is_dirty_copy) {
    root.clip = dirty_rect_copy;
    draw_all();
    root.flush(dirty_rect_copy);
  }
}
#endif
