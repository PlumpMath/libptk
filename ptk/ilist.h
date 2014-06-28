#pragma once

// #include "ptk/assert.h"
#include <cstddef>

/*
 * This file contains an implementation of "intrusive" doubly linked lists.
 * Typically in a doubly linked list, each element contains a pointer to
 * the next and the previous element. This is done in such a way that the
 * elements themselves don't know about the links. In some cases, the link
 * nodes are even allocated separately.
 *
 * The idea of intrusive lists is to push the storage of these links
 * explicitly into the definition of the elements themselves. I.e., the
 * links "intrude" upon the elements. The main reason for doing this is to
 * allocate both element data and link data together, while preserving
 * the ability to have an element participate in more than one list at
 * the same time.
 */
namespace ptk {
  /**
   * @class I2Link
   * @brief Generic double-link class holding two pointers
   *
   * The I2Link class represents one node in a doubly-linked list. The
   * naming convention used here is "left" and "right" pointers. Note that
   * I2Link pointers should never be NULL. The constructor initializes both
   * left and right to point at the structure itself. Also, the test for
   * whether the link is connected to anything else is based on checking
   * if the left pointer is non-self-referential.
   */
  struct I2Link {
    I2Link *left;
    I2Link *right;

    bool is_joined() const {
      return left != this;
    }

    void join_right_of(I2Link &other) {
      left = &other;
      right = other.right;
      other.right->left = this;
      other.right = this;
    }

    void join_left_of(I2Link &other) {
      left = other.left;
      right = &other;
      other.left->right = this;
      other.left = this;
    }

    void leave() {
      if (is_joined()) {
        left->right = right;
        right->left = left;
        left = right = this;
      }
    }

    I2Link() : left(this), right(this) {}
    ~I2Link() { leave(); }
  };

  typedef I2Link i2link_t;

  /**
   * @class I2List
   * @brief Template class representing a doubly-linked list with intrusive links
   * @tparam T element type, which must include an i2link_t member
   *
   * @code
   * struct Element {
   *  i2link_t link_a;
   *  i2link_t link_b;
   *  ...
   * } e1, e2, e3;
   *
   * struct Controller {
   *  I2List<Element> a_list;
   *  I2List<Element> b_list;
   *  ...
   *  Controller() : a_list(&Element::link_a), b_list(&Element::link_b) ...
   *  ...
   * } controller;
   *
   * controller.a_list.add(e1);
   * controller.a_list.add(e2);
   * controller.b_list.add(e1);
   * controller.b_list.add(e3);
   * @endcode
   */
  template<typename T>
  class I2List  {
    friend class Iterator;

    I2Link *ring;
    const size_t offset;

    I2Link &link(T &element) const {
      return *(I2Link *)(offset + (char *) &element);
    }

    T &element(const I2Link &l) const {
      return *(T *)(((char *) &l) - offset);
    }

  public:
    I2List(I2Link T::*member) :
      ring(0),
      offset((size_t) &(static_cast<T*>(0)->*member))
    {}

    bool empty() const {
      return ring == 0;
    }

    // add at the front
    void push(T &elt) {
      if (ring) link(elt).join_left_of(*ring);
      ring = &link(elt);
    }

    // add at the back
    void push_back(T &elt) {
      if (ring) {
        link(elt).join_left_of(*ring);
      } else {
        ring = &link(elt);
      }
    }

    // remove from the front
    T *pop() {
      if (empty()) return 0;

      T *value = &element(*ring);

      if (ring->is_joined()) {
        I2Link *first_link = ring;
        ring = first_link->right;
        first_link->leave();
      } else {
        ring = 0;
      }

      return value;
    }

    void remove(T &element) {
      I2Link &lnk = link(element);
      if (ring == &lnk) {
        if (!lnk.is_joined()) {
          ring = 0;
          return;
        } else {
          ring = lnk.right;
        }
      }

      lnk.leave();
    }

    class Iterator {
      I2List &list;
      I2Link *last, *current, *limit;

      T *deref() const { return (T *)(((char *) current) - list.offset); }

    public:
      Iterator(I2List &l) :
        list(l),
        last(0),
        current(list.ring),
        limit(0)
      {
      }

      T *operator()()       { return deref(); }
      const T *operator()() const { return deref(); }
      T &operator* ()       { return *deref(); }
      const T &operator* () const { return  *deref(); }
      T *operator->()       { return deref(); }
      const T *operator->() const { return deref(); }

      bool more() const {
        return current && (current != limit);
      }

      void next() {
        last = current;
        current = current->right;
        if (limit == 0) limit = last;
      }

      void remove() {
        last = current;
        current = current->right;

        if (!current->is_joined()) current = 0;
        list.remove(list.element(*last));
      }
    };

    Iterator iter() {
      return Iterator(*this);
    }
  };
}

