// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clr.h"
#include "editor.h"

#include "draw.h"

focus_t focus_notify_error = { draw_notify_error, NULL, input_notify_error };

void notify_error(char* str) {
	focus = focus_notify_error;
	focus.draw_args = str;
}

void draw_notify_error(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_NOTIFY_ERROR]);
	rec_str(args);
}

void input_notify_error(global_t* ed_global, u32 c) {
	(void)ed_global;
	edit_file(ed_global, *ed_global->ed);
}

