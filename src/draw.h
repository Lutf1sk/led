// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef DRAW_H
#define DRAW_H 1

#include <lt/lt.h>
#include <lt/fwd.h>

#include <string.h>

#define HEADER_HEIGHT 1
#define LINENUM_WIDTH 5

#define EDITOR_HSTART (LINENUM_WIDTH)
#define EDITOR_VSTART (HEADER_HEIGHT)

#define EDITOR_HEIGHT (lt_term_height - EDITOR_VSTART)
#define EDITOR_WIDTH (lt_term_width - EDITOR_HSTART)

extern char* write_buf;
extern char* write_it;

static LT_INLINE
void rec_c(char c) {
	*write_it++ = c;
}

static LT_INLINE
void rec_str(char* str) {
	usz len = strlen(str);
	memcpy(write_it, str, len);
	write_it += len;
}

static LT_INLINE
void rec_lstr(lstr_t str) {
	memcpy(write_it, str.str, str.len);
	write_it += str.len;
}

static
isz rec_write(void* usr, void* data, usz len) {
	memcpy(write_it, data, len);
	write_it += len;
	return len;
}

isz rec_write(void* usr, void* data, usz len);
void recf(char* fmt, ...);
void rec_nc(usz n, char c);

void rec_goto(u32 x, u32 y);

void rec_clear(char* clr);
void rec_clearline(char* clr);

void rec_led(lt_texted_t* ed, char* sel_clr, char* normal_clr);

void rec_csave(void);
void rec_crestore(void);

#endif
