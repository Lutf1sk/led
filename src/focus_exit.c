// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "clr.h"
#include "editor.h"
#include "file_browser.h"
#include "draw.h"

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

void draw_exit(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_NOTIFY_ERROR]);
	rec_str("files have unsaved changed, are you sure? (Y/n)");
}

void input_exit(global_t* ed_global, u32 c) {
	switch (c) {
	case 'N': case 'n':
		edit_file(ed_global, *ed_global->ed);
		break;

	case 'y': case 'Y':
		exit(0);
		break;
	}
}

