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

#define ATTR_BOLD      (0x80 << 24)
#define ATTR_RGB8_FG   (0x40 << 24)
#define ATTR_RGB8_BG   (0x20 << 24)
#define ATTR_CLR8      (0x10 << 24)
#define ATTR_FILL      (0x08 << 24)

#define ATTR_RGB8_MASK (0xFFFFFF)

#define ATTR_FG_BLK (0x00 << 16)
#define ATTR_FG_RED (0x01 << 16)
#define ATTR_FG_GRN (0x02 << 16)
#define ATTR_FG_YLW (0x03 << 16)
#define ATTR_FG_BLU (0x04 << 16)
#define ATTR_FG_MAG (0x05 << 16)
#define ATTR_FG_CYN (0x06 << 16)
#define ATTR_FG_WHT (0x07 << 16)

#define ATTR_FG_BRIGHT (0x08 << 16)

#define ATTR_FG_MASK (0x0F << 16)

#define ATTR_BG_BLK (0x00 << 16)
#define ATTR_BG_RED (0x10 << 16)
#define ATTR_BG_GRN (0x20 << 16)
#define ATTR_BG_YLW (0x30 << 16)
#define ATTR_BG_BLU (0x40 << 16)
#define ATTR_BG_MAG (0x50 << 16)
#define ATTR_BG_CYN (0x60 << 16)
#define ATTR_BG_WHT (0x70 << 16)

#define ATTR_BG_BRIGHT (0x80 << 16)

#define ATTR_BG_MASK (0xF0 << 16)

#define ATTR_CLR8_MASK (0xFF << 16)

#define ATTR_FG(a) ((a >> 16) & 0x07)
#define ATTR_BG(a) ((a >> 20) & 0x07)

typedef
struct draw_char {
	u32 c;
	u32 attr;
} draw_char_t;

typedef
struct draw_buf {
	usz w, h;
	i32 cx, cy;
	draw_char_t* text;
} draw_buf_t;

void buf_set_cursor(i32 line, i32 col);

void buf_set_pos(usz line, usz col);
void buf_write_utf8(u32 attr, lstr_t str);
void buf_write_char(u32 attr, u32 c);
void buf_write_txed(u32 sel_attr, u32 attr, lt_texted_t* txed);

void buf_writeln(usz line, const draw_char_t* text, usz size);
void buf_writeln_utf8(usz line, u32 attr, lstr_t str);

void buf_present(void);
void buf_resize(usz w, usz h);

#endif
