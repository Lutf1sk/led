// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clr.h"
#include "editor.h"
#include "file_browser.h"

#include <curses.h>
#include "curses_helpers.h"

#include <stdlib.h>

focus_t focus_exit = { draw_exit, NULL, input_exit };

static char* unsaved_path = NULL;

void notify_exit(void) {
	focus = focus_exit;
	editor_t* unsaved = fb_find_unsaved();
	if (unsaved)
		unsaved_path = unsaved->doc.path;
	else
		exit(0);
}

void draw_exit(global_t* ed_global, void* win_, void* args) {
	WINDOW* win = win_;

	wattr_set(win, A_BOLD, PAIR_NOTIFY_ERROR, NULL);
	mvwprintw(win, ed_global->height - 1, 0, " '%s' has unsaved changed, are you sure? (Y/n)", unsaved_path);
	waddnch(win, ed_global->width - getcurx(win), ' ');
}

void input_exit(global_t* ed_global, int c) {
	switch (c) {
	case 'N': case 'n':
		edit_file(ed_global, *ed_global->ed);
		break;

	case 'y': case 'Y':
		exit(0);
		break;
	}
}

