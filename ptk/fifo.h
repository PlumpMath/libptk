// -*- Mode:C++ -*-

#pragma once

#include "ptk/assert.h"

#include <cstdlib>
#include <stdint.h>
#include <cstddef>

namespace ptk {
  template<class T>
  class FIFO {
    T *const storage, *const limit;
    T *write_position, *read_position;

  public:
    FIFO(T *const s, size_t capacity) :
      storage(s),
      limit(storage + capacity),
      write_position(s),
      read_position(s)
    {}

    void reset() {
      write_position = read_position = storage;
    }

    // number of contiguous elements that can read
    size_t read_capacity() const {
      if (write_position >= read_position) {
        return (size_t) (write_position - read_position);
      } else {
        return (size_t) (limit - read_position);
      }
    }

    // number of contiguous elements that can be written
    size_t write_capacity() const {
      if (write_position >= read_position) {
        return (size_t) (limit - write_position);
      } else {
        return (size_t) ((read_position - write_position) - 1);
      }
    }

    // try to read n elements. actual number may be less
    size_t read(T *dst, size_t n) {
      size_t actually_read = 0;

      while (n > 0) {
        size_t run;

        if (write_position >= read_position) {
          run = write_position - read_position;
        } else {
          run = limit - read_position;
        }

        if (run == 0) break;
        if (n < run) run = n;

        for(uint32_t i=run; i > 0; --i) *dst++ = *read_position++;
        if (read_position == limit) read_position = storage;

        n -= run;
        actually_read += run;
      };

      return actually_read;
    }

    // adjust internal state as if offset elements had actually been read
    void fake_read(uint32_t offset) {
      assert(offset <= read_capacity());
      read_position += offset;
      if (read_position == limit) read_position = storage;
    }

    // lvalue of elements to be read, where peek(0) is the first unread element
    // in the buffer, and peek(1) is the second, etc.
    inline T &peek(int offset) const {
      assert((offset < 0) || (offset < (int) read_capacity()));
      T *rp = read_position + offset;

      if (rp >= limit) {
        rp -= (limit - storage);
      } else if (rp <  storage) {
        rp += (limit - storage);
      }

      return *rp;
    }

    // try to write n elements. actual number may be less
    size_t write(const T *src, size_t n) {
      size_t written = 0;

      while (n > 0) {
        size_t run;

        if (write_position >= read_position) {
          run = limit - write_position;
        } else {
          run = (read_position - write_position) - 1;
        }

        if (run == 0) break;
        if (n < run) run = n;

        for (uint32_t i=run; i > 0; --i) *write_position++ = *src++;
        if (write_position == limit) write_position = storage;

        n -= run;
        written += run;
      };

      return written;
    }

    // move internal pointers as if offset elements had actually been written
    void fake_write(uint32_t offset) {
      assert(offset <= write_capacity());
      write_position += offset;
      if (write_position == limit) write_position = storage;
    }

    // lvalue of elements to be written, where poke(0) is the next element to
    // be written and poke(-1) is the last element written.
    inline T &poke(int offset) const {
      assert((offset >= 0) || (offset < (int) write_capacity()));
      T *wp = write_position + offset;

      if (wp >= limit) {
        wp -= (limit - storage);
      } else if (wp <  storage) {
        wp += (limit - storage);
      }

      return *wp;
    }
  };

  template<typename T, unsigned N>
  class StaticFIFO : public FIFO<T> {
    T data[N];
  public:
    enum {CAPACITY = N};
    StaticFIFO() : FIFO<T>(data, N) { }
  };
};
