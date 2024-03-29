// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>

#include "focus.h"
#include "clr.h"
#include "editor.h"
#include "file_browser.h"
#include "draw.h"

#include <stdlib.h>

focus_t focus_exit = { draw_exit, NULL, input_exit };

static lstr_t unsaved_path = NLSTR();

void clean_and_exit(int code) {
	doc_t* doc;
	while ((doc = fb_first_file()))
		fb_close(doc);

	lt_term_restore();

	exit(code);
}

void notify_exit(void) {
	focus = focus_exit;
	doc_t* unsaved = fb_find_unsaved();
	if (unsaved) {
		unsaved_path = unsaved->path;
	}
	else {
		clean_and_exit(0);
	}
}

void draw_exit(editor_t* ed, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_NOTIFY_ERROR]);
	rec_str("files have unsaved changed, are you sure? (Y/n)");
}

void input_exit(editor_t* ed, u32 c) {
	switch (c) {
	case 'N': case 'n':
		edit_file(ed, ed->doc);
		break;

	case 'y': case 'Y':
		clean_and_exit(0);
		break;
	}
}

