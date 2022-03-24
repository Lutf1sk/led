// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"
#include "file_browser.h"
#include "editor.h"
#include "algo.h"
#include "utf8.h"
#include "highlight.h"
#include "term.h"

#include <string.h>
#include <stdlib.h>

focus_t focus_editor = { NULL, NULL, input_editor };

static char* clipboard = NULL;
static usz clipboard_len = 0;
static usz clipboard_alloc_len = 0;

void edit_file(global_t* ed_global, editor_t* ed) {
	focus = focus_editor;
	*ed_global->ed = ed;
}

void input_editor(global_t* ed_globals, u32 c) {
	editor_t* ed = *ed_globals->ed;

	if (!ed)
		return;

	u8 modified = 1;

	b8 sync_target_x = 1;
	b8 sync_target_y = 1;
	b8 sync_selection = 1;

	static b8 m1_pressed = 0;

	switch (c) {
	case 'Q' | LT_TERM_MOD_CTRL: modified = 0;
		fb_close(ed);
		*ed_globals->ed = fb_first_file();
		return;

	case 'K' | LT_TERM_MOD_CTRL: sync_selection = 0; modified = 0;
		browse_files();
		break;

	case 'F' | LT_TERM_MOD_CTRL: modified = 0; {
		find_local(ed->cy, ed->cx);
	}	break;

	case 'C' | LT_TERM_MOD_CTRL: sync_selection = 0; modified = 0; {
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

	case 'X' | LT_TERM_MOD_CTRL: {
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

	case 'V' | LT_TERM_MOD_CTRL: {
		ed_delete_selection_if_available(ed);

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

	case 'D' | LT_TERM_MOD_CTRL: sync_selection = 0; modified = 0; {
		ed->sel_x = ed_find_indent(ed);
		ed->sel_y = ed->cy;
		ed->cx = ed->doc.lines[ed->cy].len;
	}	break;

	case 'S' | LT_TERM_MOD_CTRL: modified = 0;
		if (!doc_save(&ed->doc))
			notify_error("Failed to save document\n");
		break;

	case '/' | LT_TERM_MOD_CTRL: sync_selection = 0;
		ed_prefix_nonempty_selection(ed, CLSTR("// "));
		// This is a pretty hackish way of toggling a comment,
		// might want to improve this later
		ed_delete_selection_prefix(ed, CLSTR("// // "));
		break;

	case '\\' | LT_TERM_MOD_CTRL: modified = 0;
		goto_line();
		break;

	case 'P' | LT_TERM_MOD_ALT: modified = 0;
		ed_paren_match(ed);
		break;

	// ----- MOUSE
	case LT_TERM_KEY_MB1_DN: modified = 0;
		ed->sel_y = clamp(ed->line_top + (lt_term_mouse_y - ed->global->vstart), 0, ed->doc.line_count - 1);
		ed->sel_x = ed_screen_x_to_cx(ed, lt_term_mouse_x - ed->global->hstart, ed->sel_y);
		ed->cy = ed->sel_y;
		ed->cx = ed->sel_x;
		m1_pressed = 1;
		break;

	case LT_TERM_KEY_MB1_UP: modified = 0;
		if (m1_pressed) {
			ed->cy = clamp(ed->line_top + (lt_term_mouse_y - ed->global->vstart), 0, ed->doc.line_count - 1);
			ed->cx = ed_screen_x_to_cx(ed, lt_term_mouse_x - ed->global->hstart, ed->cy);
		}
		sync_selection = 0;
		m1_pressed = 0;
		break;

	case LT_TERM_KEY_MPOS: modified = 0;
		if (m1_pressed) {
			ed->cy = clamp(ed->line_top + (lt_term_mouse_y - ed->global->vstart), 0, ed->doc.line_count - 1);
			ed->cx = ed_screen_x_to_cx(ed, lt_term_mouse_x - ed->global->hstart, ed->cy);
		}
		sync_selection = 0;
		break;

	// ----- UP

	case LT_TERM_KEY_UP | LT_TERM_MOD_SHIFT: sync_selection = 0; sync_target_x = 0; modified = 0;
		ed_cur_up(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy - 1));
		break;

	case LT_TERM_KEY_UP: sync_target_x = 0; modified = 0;
		if (ed_selection_available(ed) && ed->cy != ed->sel_y) {
			ed_move_to_selection_start(ed);
			break;
		}
		ed_cur_up(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy - 1));
		break;

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL: sync_target_x = 0; modified = 0;
		if (ed_selection_available(ed) && ed->cy != ed->sel_y) {
			ed_move_to_selection_start(ed);
			break;
		}

		for (usz i = 0; i < ed_globals->vstep; ++i)
			ed_cur_up(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy - 1));
		break;

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: sync_selection = 0; sync_target_x = 0; modified = 0;
		for (usz i = 0; i < ed_globals->vstep; ++i)
			ed_cur_up(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy - 1));
		break;


	case LT_TERM_KEY_UP | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT | LT_TERM_MOD_CTRL:
	case LT_TERM_KEY_UP | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: sync_selection = 0;
	case LT_TERM_KEY_UP | LT_TERM_MOD_ALT: sync_target_x = 0; modified = 0; {
		isz move_h = ed->global->height / 2;
		if (ed->cy >= ed->line_top + move_h || ed->line_top == 0)
			ed_goto_line(ed, ed->cy - move_h + ed->global->scroll_offs);
		ed_center_line(ed, ed->cy);
	}	break;

	// ----- DOWN

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_SHIFT: sync_selection = 0; sync_target_x = 0; modified = 0;
		ed_cur_down(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy + 1));
		break;

	case LT_TERM_KEY_DOWN: sync_target_x = 0; modified = 0;
		if (ed_selection_available(ed) && ed->cy != ed->sel_y) {
			ed_move_to_selection_end(ed);
			break;
		}
		ed_cur_down(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy + 1));
		break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL: sync_target_x = 0; modified = 0;
		if (ed_selection_available(ed) && ed->cy != ed->sel_y) {
			ed_move_to_selection_end(ed);
			break;
		}

		for (usz i = 0; i < ed_globals->vstep; ++i)
			ed_cur_down(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy + 1));
		break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: sync_selection = 0; sync_target_x = 0; modified = 0; {
		for (usz i = 0; i < ed_globals->vstep; ++i)
			ed_cur_down(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy + 1));
	}	break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT | LT_TERM_MOD_CTRL:
	case LT_TERM_KEY_DOWN | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: sync_selection = 0;
	case LT_TERM_KEY_DOWN | LT_TERM_MOD_ALT: sync_target_x = 0; modified = 0; {
		isz move_h = ed->global->height / 2;

		if (ed->cy <= ed->line_top + move_h || ed->line_top + ed->global->height >= ed->doc.line_count)
			ed_goto_line(ed, ed->cy + move_h - ed->global->scroll_offs);
		ed_center_line(ed, ed->cy);
	}	break;

	// ----- RIGHT

	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_SHIFT: sync_selection = 0; modified = 0;
		ed_cur_right(ed);
		break;

	case LT_TERM_KEY_RIGHT: modified = 0;
		if (ed_selection_available(ed)) {
			ed_move_to_selection_end(ed);
			break;
		}
		ed_cur_right(ed);
		break;

	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: sync_selection = 0; modified = 0; {
		usz move_chars = ed_find_word_fwd(ed) - ed->cx;
		if (!move_chars)
			ed_cur_right(ed);
		else
			ed->cx += move_chars;
	}	break;

	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_CTRL: modified = 0; {
		if (ed_selection_available(ed)) {
			ed_move_to_selection_end(ed);
			break;
		}

		usz move_chars = ed_find_word_fwd(ed) - ed->cx;
		if (!move_chars)
			ed_cur_right(ed);
		else
			ed->cx += move_chars;
	}	break;


	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT | LT_TERM_MOD_CTRL:
	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: sync_selection = 0;
	case LT_TERM_KEY_RIGHT | LT_TERM_MOD_ALT: sync_target_x = 1; modified = 0; {
		isz len = ed->doc.lines[ed->cy].len, indent = ed_find_indent(ed), move_cols = (len - indent) / 2;
		isz move_to = ed->cx + move_cols;
		if (ed->cx < indent)
			move_to = indent + move_cols;
		ed->cx = clamp(move_to, 0, len);
	}	break;

	// ----- LEFT

	case LT_TERM_KEY_LEFT | LT_TERM_MOD_SHIFT: sync_selection = 0; modified = 0;
		ed_cur_left(ed);
		break;

	case LT_TERM_KEY_LEFT: modified = 0;
		if (ed_selection_available(ed)) {
			ed_move_to_selection_start(ed);
			break;
		}
		ed_cur_left(ed);
		break;

	case LT_TERM_KEY_LEFT | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: sync_selection = 0; modified = 0; {
		usz move_chars = ed->cx - ed_find_word_bwd(ed);
		if (!move_chars)
			ed_cur_left(ed);
		else
			ed->cx -= move_chars;
	}	break;

	case LT_TERM_KEY_LEFT | LT_TERM_MOD_CTRL: modified = 0; {
		if (ed_selection_available(ed)) {
			ed_move_to_selection_start(ed);
			break;
		}
		usz move_chars = ed->cx - ed_find_word_bwd(ed);
		if (!move_chars)
			ed_cur_left(ed);
		else
			ed->cx -= move_chars;
	}	break;

	case LT_TERM_KEY_LEFT | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT | LT_TERM_MOD_CTRL:
	case LT_TERM_KEY_LEFT | LT_TERM_MOD_ALT | LT_TERM_MOD_SHIFT: sync_selection = 0;
	case LT_TERM_KEY_LEFT | LT_TERM_MOD_ALT: sync_target_x = 1; modified = 0; {
		isz len = ed->doc.lines[ed->cy].len, indent = ed_find_indent(ed), move_cols = (len - indent) / 2;
		isz move_to = ed->cx - move_cols;
		if (indent == len)
			move_to = 0;
		ed->cx = clamp(move_to, 0, len);
	}	break;

	// ----- PAGE UP/DOWN

	case LT_TERM_KEY_PAGEUP: sync_target_y = 0; sync_target_x = 0; modified = 0;
		ed_page_up(ed);
		break;

	case LT_TERM_KEY_PAGEDN: sync_target_y = 0; sync_target_x = 0; modified = 0;
		ed_page_down(ed);
		break;

	// ----- DELETE

	case LT_TERM_KEY_DELETE | LT_TERM_MOD_CTRL: ed_delete_word_fwd(ed); break;

	case LT_TERM_KEY_DELETE:
		if (ed_selection_available(ed))
			ed_delete_selection(ed);
		else if (ed->cx < ed->doc.lines[ed->cy].len)
			doc_erase_str(&ed->doc, ed->cy, ed->cx, utf8_decode_len(ed->doc.lines[ed->cy].str[ed->cx]));
		else if (ed->cy + 1 < ed->doc.line_count)
			doc_merge_line(&ed->doc, ed->cy + 1);
		break;

	// ----- BACKSPACE

	case LT_TERM_KEY_BSPACE | LT_TERM_MOD_CTRL: ed_delete_word_bwd(ed); break;

	case LT_TERM_KEY_BSPACE:
		if (ed_selection_available(ed))
			ed_delete_selection(ed);
		else if (ed->cx) {
			while (ed->cx && (ed->doc.lines[ed->cy].str[ed->cx - 1] & 0xC0) == 0x80)
				doc_erase_char(&ed->doc, ed->cy, --ed->cx);
			doc_erase_char(&ed->doc, ed->cy, --ed->cx);
		}
		else if (ed->cy) {
			ed_cur_up(ed, ISIZE_MAX);
			doc_merge_line(&ed->doc, ed->cy + 1);
		}
		break;

	// ----- HOME/END

	case LT_TERM_KEY_HOME | LT_TERM_MOD_SHIFT | LT_TERM_MOD_CTRL: sync_selection = 0;
	case LT_TERM_KEY_HOME: modified = 0;
		ed->cx = ed_find_indent_pfx(ed);
		break;

	case LT_TERM_KEY_END | LT_TERM_MOD_SHIFT | LT_TERM_MOD_CTRL: sync_selection = 0;
	case LT_TERM_KEY_END: modified = 0;
		ed->cx = ed->doc.lines[ed->cy].len;
		break;

	// ----- TAB

	case LT_TERM_KEY_TAB | LT_TERM_MOD_SHIFT: sync_selection = 0;
		ed_delete_selection_prefix(ed, CLSTR("\t"));
		break;

	case LT_TERM_KEY_TAB:
		if (ed_selection_available(ed)) {
			sync_selection = 0;
			ed_prefix_nonempty_selection(ed, CLSTR("\t"));
		}
		else
			doc_insert_char(&ed->doc, ed->cy, ed->cx++, '\t');
		break;

	// ----- MISC.

	case '\n': {
		ed_delete_selection_if_available(ed);

		isz indent_len = ed_find_indent_pfx(ed);
		lstr_t* line = &ed->doc.lines[ed->cy];

		doc_split_line(&ed->doc, ed->cy, ed->cx);
		ed_cur_down(ed, 0);

		doc_insert_str(&ed->doc, ed->cy, 0, LSTR(line->str, indent_len));
		ed->cx = indent_len;
	}	break;

	case LT_TERM_KEY_F4: modified = 0; {
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

	case LT_TERM_KEY_BPASTE: {
		while ((c = lt_term_getkey()) != LT_TERM_KEY_NBPASTE) {
			if (c == '\n') {
				doc_split_line(&ed->doc, ed->cy, ed->cx);
				ed_cur_down(ed, 0);
			}
			else
				doc_insert_char(&ed->doc, ed->cy, ed->cx++, c);
		}
	}	break;

	default:
 		if ((c >= 32 && c < 127) || (c & 0xE0) == 0xC0) {
			if ((c & 0xE0) == 0xC0)
				ed->global->await_utf8 = utf8_decode_len(c) - 1;

			ed_delete_selection_if_available(ed);
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
		else if ((c & 0xC0) == 0x80 && ed->global->await_utf8) {
			--ed->global->await_utf8;
			doc_insert_char(&ed->doc, ed->cy, ed->cx++, c);
		}
 		else {
 			modified = 0;
 			sync_selection = 0;
 		}
		break;
	}

	if (sync_target_x)
		ed_sync_target_cx(ed);
	if (sync_target_y)
		ed_sync_target_cy(ed);
	if (sync_selection)
		ed_sync_selection(ed);

	// Move screen if cursor is above the upper boundary
	isz vbound_top = ed->line_top + ed->global->scroll_offs;
	if (ed->cy < vbound_top) {
		ed->line_top -= vbound_top - ed->cy;
		ed->line_top = max(ed->line_top, 0);
	}

	// Move screen if cursor is below the lower boundary
	isz vbound_bottom = (ed->line_top + ed->global->height) - ed->global->scroll_offs - 1;
	if (ed->cy > vbound_bottom) {
		ed->line_top += ed->cy - vbound_bottom;
		ed->line_top = min(ed->line_top, max(0, ed->doc.line_count - ed->global->height));
	}

	if (modified) {
		aframe_restore(ed->highl_arena, &ed->restore);
		ed->highl_lines = highl_generate(ed->highl_arena, &ed->doc);
	}
}

