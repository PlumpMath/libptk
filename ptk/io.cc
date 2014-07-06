#include "ptk/io.h"

using namespace ptk;

size_t DeviceInStream::read(uint8_t *buffer, size_t len) {
  return fifo.read(buffer, len);
}

bool DeviceInStream::get(uint8_t &ch) {
  return fifo.read(&ch, 1) == 1;
}

size_t DeviceOutStream::write(const uint8_t *buffer, size_t len) {
  return fifo.write(buffer, len);
}

bool DeviceOutStream::put(uint8_t ch) {
  return write(&ch, 1) == 1;
}

bool OutStream::puts(const char *str) {
  if (str == 0) return false;

  size_t len=0;
  while (str[len] != '\0') len += 1;

  return len == write((const uint8_t *) str, len);
}

void OutStream::printf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

/*
 * This implementation was borrowed from ChaN. Original copyright included
 * below.
 */

/*------------------------------------------------------------------------/
/  Universal string handler for user console interface
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/
void OutStream::vprintf(const char *fmt, va_list arp) {
	unsigned int r, i, j, w, f;
	unsigned long v;
	char s[16], c, d, *p;


	for (;;) {
		c = *fmt++;					/* Get a char */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass through it if not a % sequense */
          put(c); continue;
		}
		f = 0;
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: '0' padded */
			f = 1; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
			w = w * 10 + c - '0';
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long int */
			f |= 4; c = *fmt++;
		}
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') d -= 0x20;
		switch (d) {				/* Type is... */
		case 'S' :					/* String */
			p = va_arg(arp, char*);
			for (j = 0; p[j]; j++) ;
			while (!(f & 2) && j++ < w) put(' ');
			puts(p);
			while (j++ < w) put(' ');
			continue;
		case 'C' :					/* Character */
          put((char)va_arg(arp, int)); continue;
		case 'B' :					/* Binary */
			r = 2; break;
		case 'O' :					/* Octal */
			r = 8; break;
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
		case 'X' :					/* Hexdecimal */
			r = 16; break;
		default:					/* Unknown type (passthrough) */
          put(c); continue;
		}

		/* Get an argument and put it in numeral */
		v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
		if (d == 'D' && (v & 0x80000000)) {
			v = 0 - v;
			f |= 8;
		}
		i = 0;
		do {
			d = (char)(v % r); v /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (v && i < sizeof(s));
		if (f & 8) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		while (!(f & 2) && j++ < w) put(d);
		do put(s[--i]); while(i);
		while (j++ < w) put(' ');
	}
}

USBEndpoint *USBEndpoint::eps[MAX_ENDPOINTS];

void USBEndpoint::init(uint8_t ep) {
  this->ep_id = ep;
  eps[ep] = this;
}

void USBEndpoint::transfer_endpoint_data(uint8_t id) {
  if (id < MAX_ENDPOINTS && eps[id]) {
    eps[id]->transfer();
  }
}
