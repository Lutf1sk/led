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
	buf_set_pos(lt_term_height - 1, 0);
	buf_write_char(clr_attr[CLR_LIST_HEAD], ' ');
	buf_write_txed(clr_attr[CLR_EDITOR_SEL], clr_attr[CLR_LIST_HEAD], line_input);
	buf_write_char(clr_attr[CLR_LIST_HEAD] | ATTR_FILL, ' ');
	buf_write_char(0, 0);

	usz sx = cursor_x_to_screen_x(ed, lt_texted_line_str(line_input, 0), line_input->cursor_x);
	buf_set_cursor(lt_term_height - 1, sx + 1);
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

