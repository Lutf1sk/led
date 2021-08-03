// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_H
#define FOCUS_H 1

typedef struct global global_t;

typedef void (*fn_focus_draw_t)(global_t*, void*,void*);
typedef void (*fn_focus_input_t)(global_t*, int);

typedef
struct focus {
	fn_focus_draw_t draw;
	void* draw_args;
	
	fn_focus_input_t input;
} focus_t;

extern focus_t focus;

#endif
