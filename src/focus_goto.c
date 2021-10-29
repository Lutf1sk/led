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
usz interp_str(editor_t* ed, lstr_t str, b8* out_sync_selection, char* out_dir) {
	b8 sync_selection = 1;
	char dir = 0;

	usz line = 0;

	for (usz i = 0; i < input.len; ++i) {
		char c = input.str[i];
		switch (c) {
		case 's':
			sync_selection = 0;
			break;

		case 'u': case'd':
			dir = c;
			break;

		case 'e':
			line = ed->doc.line_count + 1;
			break;

		case 'b':
			line = 1;
			break;

		default:
			if (is_digit(c)) {
				line *= 10;
				line += c - '0';
			}
			break;
		}
	}

	*out_sync_selection = sync_selection;
	*out_dir = dir;
	return line;
}

void goto_line(void) {
	input.len = 0;
	focus = focus_goto;
}

void draw_goto(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height - 1);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_lstr(input.str, input.len);

	b8 sync_selection; char dir; usz line;
	line = interp_str(*ed_global->ed, input, &sync_selection, &dir);

	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HIGHL]);
	rec_str(sync_selection ? " Jump" : " Select");
	if (line) {
		char buf[64];
		if (dir == 0)
			sprintf(buf, " to line %zu", line);
		else if (dir == 'u')
			sprintf(buf, " UP %zu lines", line);
		else if (dir == 'd')
			sprintf(buf, " DOWN %zu lines", line);
		rec_str(buf);
	}
}

void input_goto(global_t* ed_global, u32 c) {
	editor_t* ed = *ed_global->ed;

	switch (c) {
	case 's':
		if (!input.len)
			input.str[input.len++] = 's';
		break;

	case 'u': case 'd':
		if (input.len == 0 || (input.str[0] == 's' && input.len == 1))
			input.str[input.len++] = c;
		break;

	case LT_TERM_KEY_BSPACE:
		if (!input.len)
			edit_file(ed_global, ed);
		else
			--input.len;
		break;

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!input.len)
			edit_file(ed_global, ed);
		while (input.len && is_digit(input.str[--input.len]))
			;
		break;

	case '\n': {
		b8 sync_selection; char dir; usz line;
		line = interp_str(ed, input, &sync_selection, &dir);

		if (dir == 'u')
			ed_goto_line(ed, max(ed->cy - line, 0));
		else if (dir == 'd')
			ed_goto_line(ed, ed->cy + line);
		else if (line)
			ed_goto_line(ed, line - 1);

		if (sync_selection)
			ed_sync_selection(ed);

		edit_file(ed_global, ed);
	}	break;

	case 'b': case 'e':
		if (input.len && (is_digit(input.str[input.len - 1]) || input.str[input.len - 1] == 'e' || input.str[input.len - 1] == 'b'))
			break;

		if (input.len < sizeof(input_buf))
			input.str[input.len++] = c;
		break;

	default:
		if (input.len && (input.str[input.len - 1] == 'e' || input.str[input.len - 1] == 'b'))
			break;

		if (is_digit(c) && input.len < sizeof(input_buf))
			input.str[input.len++] = c;
		break;
	}
}

