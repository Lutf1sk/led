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
	lt_led_clear(line_input);
}

void draw_command(global_t* ed_global, void* args) {
	rec_goto(2, lt_term_height);
	rec_clearline(clr_strs[CLR_LIST_HEAD]);
	rec_led(line_input, clr_strs[CLR_EDITOR_SEL], clr_strs[CLR_LIST_HEAD]);
	rec_str(" ");
	rec_crestore();
}

void input_command(global_t* ed_global, u32 c) {
	editor_t* ed = ed_global->ed;

	switch (c) {
	case '\n':
		execute_string(ed, lt_led_get_str(line_input));
		lt_led_clear(line_input);
		edit_file(ed_global, ed);
		break;

	case LT_TERM_KEY_BSPACE: case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL:
		if (!lt_darr_count(line_input->str))
	case LT_TERM_KEY_ESC:
			edit_file(ed_global, ed);
	default:
		input_term_key(line_input, c);
		break;
	}
}

