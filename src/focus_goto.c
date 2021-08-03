// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus_goto.h"
#include "focus_editor.h"
#include "editor.h"
#include "clr.h"
#include "chartypes.h"
#include "algo.h"

#include <curses.h>
#include "curses_helpers.h"
#include "custom_keys.h"

focus_t focus_goto = { draw_goto, NULL, input_goto };

static char input_buf[64];
static lstr_t input = LSTR(input_buf, 0);

void goto_line(void) {
	input.len = 0;
	focus = focus_goto;
}

void draw_goto(global_t* ed_global, void* win_, void* args) {
	WINDOW* win = win_;
	
	isz width = ed_global->width;
	isz height = ed_global->height;
	
	wattr_set(win, 0, PAIR_BROWSE_FILES_INPUT, NULL);
	mvwprintw(win, height - 1, 0, " %.*s", (int)input.len, input.str);
	wcursyncup(win);
	waddnch(win, width - getcurx(win), ' ');
}

void input_goto(global_t* ed_global, int c) {
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
	
	case KEY_BACKSPACE:
		if (input.len)
			--input.len;
		else
			focus = focus_editor;
		break;
		
	case KEY_CBACKSPACE:
		if (!input.len)
			focus = focus_editor;
		while (input.len && is_digit(input.str[--input.len]))
			;
		break;

	case KEY_ENTER: case '\n': {
		b8 sync_selection = 1;
		char dir = 0;
		
		u64 line = 0;
		
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
		
		if (dir == 'u')
			ed_goto_line(ed, imax(ed->cy - line, 0));
		else if (dir == 'd')
			ed_goto_line(ed, ed->cy + line);
		else if (line)
			ed_goto_line(ed, line - 1);
		
		if (sync_selection)
			ed_sync_selection(ed);			

		focus = focus_editor;
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

