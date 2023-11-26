// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>

#include "focus.h"
#include "clr.h"
#include "editor.h"

#include "draw.h"

focus_t focus_notify_error = { draw_notify_error, NULL, input_notify_error };

void notify_error(char* str) {
	focus = focus_notify_error;
	focus.draw_args = str;
}

void draw_notify_error(editor_t* ed, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_NOTIFY_ERROR]);
	rec_str(clr_strs[CLR_NOTIFY_ERROR]);
	rec_str(args);
}

void input_notify_error(editor_t* ed, u32 c) {
	edit_file(ed, ed->doc);
}

