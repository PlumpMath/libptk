#pragma once

#include "ptk/assert.h"
#include <iterator>

namespace ptk {
  /**
   * @class DLink
   * @brief Template class holding pointers in a doubly-linked list
   *
   * The DLink template holds a pair of pointers to a parent type T,
   * which is intended to have one or more members of type DLink<T>.
   * The pointers are initialzed to 0.
   *
   * This template is used in conjunction with the DQueue template below.
   *
   * @tparam T a class usually containing DLink<T> members
   */
  template<typename T>
  class DLink {
  public:
    T *prev, *next;

    DLink() : prev(0), next(0) {}
    bool is_first()   const { return prev == 0; }
    bool is_last()    const { return next == 0; }
  };

  /**
   * @class DQueue
   * @brief Template class representing a doubly-linked queue
   * @tparam T type of the queue elements
   * @tparam member pointer-to-member that links elements in this queue
   *
   * The DQueue template represents a doubly-linked queue of elements.
   * The implementation is probably more complex than expected because it's
   * designed to handle cases where each element can participate in more than
   * one queue.
   *
   * @code
   * struct Element {
   *  DLink<Element> link_a;
   *  DLink<Element> link_b;
   *  ...
   * } e1, e2, e3;
   *
   * struct Queues {
   *  DQueue<Element, &Element::link_a> queue_a;
   *  DQueue,Element, &Element::link_b> queue_b;
   * } qs;
   *
   * qs.queue_a.insert_after(e1);
   * qs.queue_a.insert_after(e3);
   * qs.queue_b.insert_after(e3);
   * qs.queue_b.insert_after(e2);
   * @endcode
   *
   * qs.queue_a is (e1, e3) and qs.queue_b is (e3, e2). e3 simultaneously
   * exists in both.
   */
  template<typename T>
  class DQueue : public DLink<T> {
    DLink<T> T::* const member;

  public:
    DQueue(DLink<T> T::*m) : member(m) {}

    /**
     * @brief tests whether a queue has any elements
     * @returns true if there are no elements in the queue, false otherwise
     *
     * Note that when prev == 0, it should also be the case that next == 0, so
     * only one pointer needs to be checked.
     */
    bool empty() const { return this->prev == 0; }

    /**
     * @brief removes all elements from a queue
     * @returns a reference to the (empty) queue
     */
    DQueue &clear() {
      T *e = this->next;
      while(e) {
        T *e1 = (e->*member).next;
        (e->*member).next = 0;
        (e->*member).prev = 0;
        e = e1;
      }

      this->next = this->prev = 0;
      return *this;
    }

    /**
     * @brief removes an element from a queue
     * @param[in] element the element to be removed
     * @returns a reference to the queue
     *
     * If element is not in this queue when remove() is called, nothing bad
     * happens. If element is in another queue (but using the same member link),
     * the result is undefined.
     */
    DQueue &remove(T &element) {
      if ((element.*member).prev) {
        // what preceeds element needs to skip past element
        ((element.*member).prev->*member).next = (element.*member).next;
      } else if (this->next == &element) {
        // element used to be at the head
        this->next = (element.*member).next;
      }

      if ((element.*member).next) {
        // what followed element needs a new back pointer
        ((element.*member).next->*member).prev = (element.*member).prev;
      } else if (this->prev == &element) {
        // element used to be at the tail
        this->prev = (element.*member).prev;
      }

      // element is no longer in the queue
      (element.*member).prev = (element.*member).next = 0;

      if ((this->prev == this->next) && (this->prev != 0)) {
        (this->prev->*member).prev = 0;
        (this->prev->*member).next = 0;
      }

      PTK_ASSERT(!((this->prev == this->next) && (this->prev != 0)) ||
                 ((this->prev->*member).prev == 0 &&
                  (this->prev->*member).next == 0),
                 "remove() leaves bad pointers behind");

      return *this;
    }

    /**
     * @brief inserts and element at the head of a queue or before another element
     * @param[in] element the element to be inserted
     * @param[in] position pointer to an element in the queue, or 0 meaning the head
     * @returns a reference to the queue
     *
     * When position is 0, element is added at the head of the queue.
     * The DLink pointers in element for this queue must both be 0.
     *
     * The element must not already be in a queue using the same DLink. In this
     * case, the result is undefined.
     */
    DQueue &insert_before(T &element, T *position=0) {
      if (position == 0 || (position->*member).is_first()) {
        // element will join the queue at the head
        // element's next is the old head of the queue
        // note that element's prev is already 0
        (element.*member).next = this->next;

        // the old head of the queue has a back pointer to element
        if (this->next) (this->next->*member).prev = &element;

        // element is now at the head
        this->next = &element;

        // if the queue was empty, element is also at the tail
        if (this->prev == 0) this->prev = &element;
      } else {
        // element will join the queue somewhere in the middle

        // element's prev points to what precedes the position
        (element.*member).prev = (position->*member).prev;

        // element's next points to the position
        (element.*member).next = position;

        // what preceeded the position needs a forward pointer to element
        ((position->*member).prev->*member).next = &element;

        // position needs a back pointer to element
        (position->*member).prev = &element;
      }

      return *this;
    }

    /**
     * @brief inserts and element at the tail of a queue or after another element
     * @param[in] element the element to be inserted
     * @param[in] position pointer to an element in the queue, or 0 meaning the tail
     * @returns a reference to the queue
     *
     * When position is 0, element is added at the tail of the queue.
     * The DLink pointers in element for this queue must both be 0.
     *
     * The element must not already be in a queue using the same DLink. In this
     * case, the result is undefined.
     */
    DQueue &insert_after(T &element, T *position=0) {
      if (position == 0 || (position->*member).is_last()) {
        // element will join the queue at the tail

        // element's prev is the old tail of the queue
        // note that element's next is already 0
        (element.*member).prev = this->prev;

        // the old tail of the queue now has a forward pointer to element
        if (this->prev) (this->prev->*member).next = &element;

        // element is now at the tail
        this->prev = &element;

        // if the queue was empty, element is also at the head
        if (this->next == 0) this->next = &element;
      } else {
        // element will join the queue somewhere in the middle

        // element's next points to what currently follows the position
        (element.*member).next = (position->*member).next;

        // element's prev points to the position
        (element.*member).prev = position;

        // what follows position needs a back pointer to element
        ((position->*member).next->*member).prev = &element;

        // position needs a forward pointer to element
        (position->*member).next = &element;
      }

      return *this;
    }

    T &front() const { return *this->next; }
    T &back() const { return *this->prev; }
    T &push(T &element) { insert_before(element); return element; }
    void pop() { remove(*this->next); }

    class Iterator : public std::iterator<std::forward_iterator_tag, T> {
      T *p;
      DLink<T> T::* const member;

    public:
      Iterator(T *p0, DLink<T> T::*m) : p(p0), member(m) { }
      Iterator(const Iterator &other) : p(other.p), member(other.member) { }
      const Iterator &operator++() { p = (p->*member).next; return *this; }
      Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
      bool operator==(const Iterator &rhs) const { return p == rhs.p; }
      bool operator!=(const Iterator &rhs) const { return p != rhs.p; }
      T *operator*() { return p; }
      T *operator->() { return p; }
    };

    Iterator begin()  const { return Iterator(this->next, member); }
    Iterator rbegin() const { return Iterator(this->prev, member); }
    Iterator end()    const { return Iterator(0,          member); }
  };
}
