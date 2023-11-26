// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/texted.h>
#include <lt/term.h>

#include "command.h"
#include "focus.h"
#include "draw.h"
#include "clr.h"

focus_t focus_command = { draw_command, NULL, input_command };

void command(void) {
	focus = focus_command;
	lt_texted_clear(line_input);
}

void draw_command(editor_t* ed, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(line_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");
	rec_crestore();
}

void input_command(editor_t* ed, u32 c) {
	doc_t* doc = ed->doc;

	switch (c) {
	case '\n':
		if (execute_string(ed, lt_texted_line_str(line_input, 0))) {
			ed_regenerate_hl(ed);
		}
		edit_file(ed, doc);
		break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_texted_line_len(line_input, 0)) {
		case LT_TERM_KEY_ESC:
			edit_file(ed, doc);
		}
	default:
		input_term_key(line_input, c);
		break;
	}
}

