// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "editor.h"
#include "clr.h"
#include "token_chars.h"
#include "algo.h"
#include "draw.h"

#include <stdio.h>

focus_t focus_goto = { draw_goto, NULL, input_goto };

static char input_buf[64];
static lstr_t input = LSTR(input_buf, 0);

static
isz interp_str(editor_t* ed, lstr_t str) {
	char fwd = 1;
	isz line = 0;

	for (usz i = 0; i < input.len; ++i) {
		char c = input.str[i];
		switch (c) {
		case 'e': line += ed->doc.line_count; break;
		case 'b': line += -ed->doc.line_count; break;
		case '\\':
			fwd = 0;
			break;

		default:
			if (is_digit(c)) {
				line *= 10;
				line += c - '0';
			}
			break;
		}
	}

	if (fwd)
		return ed->cy + line;
	else
		return ed->cy - line;
}

void goto_line(void) {
	input.len = 0;
	focus = focus_goto;
}

void draw_goto(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_lstr(input.str, input.len);
}

void input_goto(global_t* ed_global, u32 c) {
	editor_t* ed = *ed_global->ed;

	switch (c) {
	case LT_TERM_KEY_BSPACE:
		if (!input.len)
			edit_file(ed_global, ed);
		else
			--input.len;
		break;

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!input.len)
			edit_file(ed_global, ed);
		input.len = 0;
		break;

	case '\n': {
		isz line = interp_str(ed, input);
		ed_goto_line(ed, line);
		ed_sync_selection(ed);
		edit_file(ed_global, ed);

		if (line < ed->line_top || line > ed->line_top + ed->global->height)
			ed_center_line(ed, line);
	}	break;

	default:
		if (input.len < sizeof(input_buf))
			input.str[input.len++] = c;
		break;
	}
}

