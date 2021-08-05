// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "custom_keys.h"
#include "file_browser.h"
#include "editor.h"
#include "algo.h"

#include <string.h>
#include <stdlib.h>

#include <curses.h>
#include "curses_helpers.h"

focus_t focus_editor = { NULL, NULL, input_editor };

static char* clipboard = NULL;
static usz clipboard_len = 0;
static usz clipboard_alloc_len = 0;

void edit_file(global_t* ed_global, editor_t* ed) {
	focus = focus_editor;
	*ed_global->ed = ed;
}

void input_editor(global_t* ed_globals, int c) {
	editor_t* ed = *ed_globals->ed;

	if (!ed)
		return;

	b8 sync_target_x = 1;
	b8 sync_target_y = 1;
	b8 sync_selection = 1;

	switch (c) {
	case 'Q' - CTRL_MOD_DIFF:
		fb_close(ed);
		*ed_globals->ed = fb_first_file();
		break;

	case 'K' - CTRL_MOD_DIFF: sync_selection = 0;
		browse_files();
		break;

	case 'F' - CTRL_MOD_DIFF: {
		find_local(ed->cy, ed->cx);
	}	break;

	case 'C' - CTRL_MOD_DIFF: sync_selection = 0; {
		usz sel_len = ed_selection_len(ed);

		if (sel_len > clipboard_alloc_len) {
			clipboard = realloc(clipboard, sel_len);
			if (sel_len && !clipboard)
				ferrf("Memory allocation failed: %s\n", os_err_str());
			clipboard_alloc_len = sel_len;
		}
		clipboard_len = sel_len;

		ed_write_selection_str(ed, clipboard);
	}	break;

	case 'X' - CTRL_MOD_DIFF: {
		usz sel_len = ed_selection_len(ed);

		if (sel_len > clipboard_alloc_len) {
			clipboard = realloc(clipboard, sel_len);
			if (sel_len && !clipboard)
				ferrf("Memory allocation failed: %s\n", os_err_str());
			clipboard_alloc_len = sel_len;
		}
		clipboard_len = sel_len;

		ed_write_selection_str(ed, clipboard);
		ed_delete_selection(ed);
	}	break;

	case 'V' - CTRL_MOD_DIFF: {
		for (usz i = 0; i < clipboard_len; ++i) {
			char c = clipboard[i];

			if (c == '\n') {
				doc_split_line(&ed->doc, ed->cy, ed->cx);
				ed_cur_down(ed, 0);
			}
			else
				doc_insert_char(&ed->doc, ed->cy, ed->cx++, c);
		}
	}	break;

	case KEY_MOUSE: {
		static b8 pressed = 0;

		MEVENT mev;
		if (getmouse(&mev) != OK)
			break;

		if (mev.bstate & BUTTON1_PRESSED) {
			ed->sel_y = min(ed->line_top + (mev.y - ed->global->vstart), ed->doc.line_count - 1);
			ed->sel_x = ed_screen_x_to_cx(ed, mev.x - ed->global->hstart, ed->sel_y);
			ed->cy = ed->sel_y;
			ed->cx = ed->sel_x;
			pressed = 1;
		}
		else if (mev.bstate & BUTTON1_RELEASED) {
			if (pressed) {
				ed->cy = min(ed->line_top + (mev.y - ed->global->vstart), ed->doc.line_count - 1);
				ed->cx = ed_screen_x_to_cx(ed, mev.x - ed->global->hstart, ed->cy);
			}
			sync_selection = 0;
			pressed = 0;
		}
		else if (mev.bstate & REPORT_MOUSE_POSITION) {
			if (pressed) {
				ed->cy = min(ed->line_top + (mev.y - ed->global->vstart), ed->doc.line_count - 1);
				ed->cx = ed_screen_x_to_cx(ed, mev.x - ed->global->hstart, ed->cy);
			}
			sync_selection = 0;
		}
	}	break;

	case 'S' - CTRL_MOD_DIFF:
		if (!doc_save(&ed->doc))
			notify_error("Failed to save document\n");
		break;

	case KEY_SUP: sync_selection = 0;
	case KEY_UP: sync_target_x = 0;
		ed_cur_up(ed, ed->target_cx);
		break;

	case KEY_SDOWN: sync_selection = 0;
	case KEY_DOWN: sync_target_x = 0;
		ed_cur_down(ed, ed->target_cx);
		break;

	case KEY_SRIGHT: sync_selection = 0;
	case KEY_RIGHT:
		ed_cur_right(ed);
		break;

	case KEY_SLEFT: sync_selection = 0;
	case KEY_LEFT:
		ed_cur_left(ed);
		break;

	case KEY_CUP:
		for (usz i = 0; i < ed_globals->vstep; ++i)
			ed_cur_up(ed, ed->target_cx);
		break;

	case KEY_CDOWN:
		for (usz i = 0; i < ed_globals->vstep; ++i)
			ed_cur_down(ed, ed->target_cx);
		break;

	case KEY_CSRIGHT: sync_selection = 0;
	case KEY_CRIGHT: {
		usz move_chars = ed_find_word_fwd(ed) - ed->cx;
		if (!move_chars)
			ed_cur_right(ed);
		else for (usz i = 0; i < move_chars; ++i)
			ed_cur_right(ed);
	}	break;

	case KEY_CSLEFT: sync_selection = 0;
	case KEY_CLEFT: {
		usz move_chars = ed->cx - ed_find_word_bwd(ed);
		if (!move_chars)
			ed_cur_left(ed);
		else for (usz i = 0; i < move_chars; ++i)
			ed_cur_left(ed);
	}	break;

	case KEY_PPAGE: sync_target_y = 0; sync_target_x = 0;
		ed_page_up(ed);
		break;

	case KEY_NPAGE: sync_target_y = 0; sync_target_x = 0;
		ed_page_down(ed);
		break;

	case KEY_CSUP: sync_selection = 0; {
		isz start_y, start_x, end_y, end_x;
		ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);

		if (!start_y)
			break;

		ed->doc.unsaved = 1;

		isz line_count = end_y - start_y + 1;
		lstr_t* line_start = &ed->doc.lines[start_y - 1];
		lstr_t old_start = *line_start;

		memmove(line_start, line_start + 1, line_count * sizeof(lstr_t));

		--ed->sel_y;
		ed_cur_up(ed, ed->cx);

		ed->doc.lines[end_y] = old_start;
	}	break;

	case KEY_CSDOWN: sync_selection = 0; {
		isz start_y, start_x, end_y, end_x;
		ed_get_selection(ed, &start_y, &start_x, &end_y, &end_x);

		if (end_y + 1 >= ed->doc.line_count)
			break;

		ed->doc.unsaved = 1;

		isz line_count = end_y - start_y + 1;
		lstr_t* line_start = &ed->doc.lines[start_y];
		lstr_t old_end = ed->doc.lines[end_y + 1];

		memmove(line_start + 1, line_start, line_count * sizeof(lstr_t));

		++ed->sel_y;
		ed_cur_down(ed, ed->cx);

		*line_start = old_end;
	}	break;

	case KEY_CDC: ed_delete_word_fwd(ed); break;

	case KEY_DC:
		if (ed_selection_available(ed))
			ed_delete_selection(ed);
		else if (ed->cx < ed->doc.lines[ed->cy].len)
			doc_erase_char(&ed->doc, ed->cy, ed->cx);
		else if (ed->cy + 1 < ed->doc.line_count)
			doc_merge_line(&ed->doc, ed->cy + 1);
		break;

	case KEY_CBACKSPACE: ed_delete_word_bwd(ed); break;

	case KEY_BACKSPACE:
		if (ed_selection_available(ed))
			ed_delete_selection(ed);
		else if (ed->cx)
			doc_erase_char(&ed->doc, ed->cy, --ed->cx);
		else if (ed->cy) {
			ed_cur_up(ed, (usz)-1);
			doc_merge_line(&ed->doc, ed->cy + 1);
		}
		break;

	case KEY_HOME:
		ed->cx = 0;
		break;

	case KEY_END:
		ed->cx = ed->doc.lines[ed->cy].len;
		break;

	case KEY_ENTER: case '\n': {
		delete_selection_if_available(ed);

		usz tab_count = 0;
		lstr_t* line = &ed->doc.lines[ed->cy];
		for (usz i = 0; i < line->len && i < ed->cx; ++i) {
			if (line->str[i] != '\t')
				break;
			tab_count++;
		}

		doc_split_line(&ed->doc, ed->cy, ed->cx);
		ed_cur_down(ed, 0);

		for (usz i = 0; i < tab_count; ++i)
			doc_insert_char(&ed->doc, ed->cy, i, '\t');
		ed->cx = tab_count;
	}	break;

	case KEY_F(4): {
		char* name = ed->doc.name;
		usz len = strlen(name);
		char name_buf[PATH_MAX_LEN];

		if (len < 2 || name[len - 2] != '.')
			break;

		char ext = name[len - 1];

		char new_ext = 0;
		if (ext == 'c')
			new_ext = 'h';
		else if (ext == 'h')
			new_ext = 'c';
		else
			break;

		memcpy(name_buf, ed->doc.name, len);
		name_buf[len - 1] = new_ext;

		editor_t* file = fb_find_file(LSTR(name_buf, len));
		if (file)
			*ed->global->ed = file;
	}	break;

	case 'P' - CTRL_MOD_DIFF: sync_selection = 0;
		ed_prefix_selection(ed, CLSTR("// "));
		// This is a pretty hackish way of toggling a comment,
		// might want to improve this later
		ed_delete_selection_prefix(ed, CLSTR("// // "));
		break;

	case KEY_STAB: sync_selection = 0;
		ed_delete_selection_prefix(ed, CLSTR("\t"));
		break;

	case '\t':
		if (ed_selection_available(ed)) {
			sync_selection = 0;
			ed_prefix_selection(ed, CLSTR("\t"));
		}
		else
			doc_insert_char(&ed->doc, ed->cy, ed->cx++, '\t');
		break;

	case 'L' - CTRL_MOD_DIFF: sync_selection = 0;
		goto_line();
		break;

	default:
		if (c >= 32 && c < 127) {
			delete_selection_if_available(ed);

			doc_insert_char(&ed->doc, ed->cy, ed->cx++, c);

			if (!ed_globals->predict_brackets)
				break;

			if (c == '(')
				doc_insert_char(&ed->doc, ed->cy, ed->cx, ')');
			else if (c == '{')
				doc_insert_char(&ed->doc, ed->cy, ed->cx, '}');
			else if (c == '[')
				doc_insert_char(&ed->doc, ed->cy, ed->cx, ']');
		}
		else
			sync_selection = 0;
		break;
	}

	if (sync_target_x)
		ed_sync_target_cx(ed);
	if (sync_target_y)
		ed_sync_target_cy(ed);
	if (sync_selection)
		ed_sync_selection(ed);
}

