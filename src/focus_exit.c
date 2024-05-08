// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/term.h>

#define LT_ANSI_SHORTEN_NAMES 1
#include <lt/ansi.h>

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

	lstr_t final_write = CLSTR(RESET CSET(9999, 0) "\n");
	lt_term_write_direct(final_write.str, final_write.len);

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
	buf_writeln_utf8(lt_term_height - 1, clr_attr[CLR_NOTIFY_ERROR], CLSTR(" files have unsaved changed, are you sure? (Y/n) "));
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
