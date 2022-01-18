// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef LT_TERM_H
#define LT_TERM_H 1

#include "common.h"

typedef
enum lt_term_flags {
	LT_TERM_ECHO	= 0x01,
	LT_TERM_CANON	= 0x02,
	LT_TERM_SIGNAL	= 0x04,
	LT_TERM_UTF8	= 0x08,
	LT_TERM_MOUSE	= 0x10,
	LT_TERM_BPASTE	= 0x20,
	LT_TERM_ALTBUF	= 0x40,
} lt_term_flags_t;

typedef
enum lt_term_mod {
	LT_TERM_MOD_SHIFT	= 0x10000000,
	LT_TERM_MOD_ALT		= 0x20000000,
	LT_TERM_MOD_CTRL	= 0x40000000,
} lt_term_mod_t;

#define LT_TERM_MOD_MASK (0xF0000000)
#define LT_TERM_KEY_MASK (~LT_TERM_MOD_MASK)

typedef
enum lt_term_key {
	LT_TERM_KEY_BSPACE	= 0x7F,
	LT_TERM_KEY_ESC		= 0x1B,
	LT_TERM_KEY_TAB		= '\t',

	LT_TERM_KEY_UP		= 0x08000000,
	LT_TERM_KEY_DOWN	= 0x08000001,
	LT_TERM_KEY_RIGHT	= 0x08000002,
	LT_TERM_KEY_LEFT	= 0x08000003,

	LT_TERM_KEY_PAGEUP	= 0x08000010,
	LT_TERM_KEY_PAGEDN	= 0x08000011,
	LT_TERM_KEY_HOME	= 0x08000012,
	LT_TERM_KEY_END		= 0x08000013,
	LT_TERM_KEY_INSERT	= 0x08000014,
	LT_TERM_KEY_DELETE	= 0x08000015,

	LT_TERM_KEY_F1		= 0x08000020,
	LT_TERM_KEY_F2		= 0x08000021,
	LT_TERM_KEY_F3		= 0x08000022,
	LT_TERM_KEY_F4		= 0x08000023,
	LT_TERM_KEY_F5		= 0x08000024,
	LT_TERM_KEY_F6		= 0x08000025,
	LT_TERM_KEY_F7		= 0x08000026,
	LT_TERM_KEY_F8		= 0x08000027,
	LT_TERM_KEY_F9		= 0x08000028,
	LT_TERM_KEY_F10		= 0x08000029,
	LT_TERM_KEY_F11		= 0x0800002A,
	LT_TERM_KEY_F12		= 0x0800002B,

	LT_TERM_KEY_RESIZE	= 0x08000030,

	LT_TERM_KEY_BPASTE	= 0x08000040,
	LT_TERM_KEY_NBPASTE	= 0x08000041,

	LT_TERM_KEY_MB1_DN	= 0x08000050,
	LT_TERM_KEY_MB1_UP	= 0x08000051,
	LT_TERM_KEY_MB2_DN	= 0x08000052,
	LT_TERM_KEY_MB2_UP	= 0x08000053,
	LT_TERM_KEY_MB3_DN	= 0x08000054,
	LT_TERM_KEY_MB3_UP	= 0x08000055,
	LT_TERM_KEY_MPOS	= 0x08000056,
} lt_term_key;

extern u32 lt_term_width, lt_term_height;
extern i32 lt_term_mouse_x, lt_term_mouse_y;

void lt_term_init(lt_term_flags_t flags);
void lt_term_restore(void);

u32 lt_term_getkey(void);

void lt_term_write_direct(char* str, usz len);

#endif