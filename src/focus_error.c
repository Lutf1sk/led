// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clr.h"
#include "editor.h"

#include <curses.h>
#include "curses_helpers.h"

focus_t focus_notify_error = { draw_notify_error, NULL, input_notify_error };

void notify_error(char* str) {
	focus = focus_notify_error;
	focus.draw_args = str;
}

void draw_notify_error(global_t* ed_global, void* win_, void* args) {
	WINDOW* win = win_;

	wattr_set(win, A_BOLD, PAIR_NOTIFY_ERROR, NULL);
	char* str = args;

	mvwprintw(win, ed_global->height - 1, 0, " %s", str);
	waddnch(win, ed_global->width - getcurx(win), ' ');
}

void input_notify_error(global_t* ed_global, int c) {
	(void)ed_global;
	edit_file(ed_global, *ed_global->ed);
}

