// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include <lt/utf8.h>
#include <lt/term.h>
#include <lt/mem.h>
#include <lt/ctype.h>

#include "focus.h"
#include "file_browser.h"
#include "editor.h"
#include "algo.h"
#include "highlight.h"
#include "clipboard.h"
#include "command.h"
#include "keybinds.h"

#include <string.h>
#include <stdlib.h>

focus_t focus_editor = { NULL, NULL, input_editor };

void edit_file(global_t* ed_global, editor_t* ed) {
	focus = focus_editor;
	ed_global->ed = ed;
	if (ed)
		ed_regenerate_hl(ed);
}

void input_editor(global_t* ed_globals, u32 c) {
	editor_t* ed = ed_globals->ed;

	if (!ed)
		return;

	u8 modified = 1;

	b8 sync_target_x = 1;
	b8 sync_target_y = 1;
	b8 sync_selection = 1;

	static b8 m1_pressed = 0;

	if (c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT))
		ed_globals->consec_cdn = 0;
	if (c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL) && c != (LT_TERM_KEY_UP | LT_TERM_MOD_CTRL| LT_TERM_MOD_SHIFT))
		ed_globals->consec_cup = 0;

	switch (c) {
	case 'Q' | LT_TERM_MOD_CTRL: modified = 0;
		fb_close(ed);
		edit_file(ed_globals, fb_first_file());
		return;

	case 'K' | LT_TERM_MOD_CTRL: sync_selection = 0; modified = 0;
		browse_files();
		break;

	case 'F' | LT_TERM_MOD_CTRL: sync_selection = 0; modified = 0;
		find_local(ed->cy, ed->cx);
		break;

	case 'S' | LT_TERM_MOD_CTRL: modified = 0; sync_selection = 0;
		if (!doc_save(&ed->doc))
			notify_error("Failed to save document\n");
		break;

	case '/' | LT_TERM_MOD_CTRL: sync_selection = 0;
		ed_prefix_nonempty_selection(ed, CLSTR("// "));
		// This is a pretty hackish way of toggling a comment,
		// might want to improve this later
		ed_delete_selection_prefix(ed, CLSTR("// // "));
		break;

	case 'p' | LT_TERM_MOD_ALT: case 'P' | LT_TERM_MOD_ALT: modified = 0;
		ed_paren_match(ed);
		break;

	case '\\' | LT_TERM_MOD_CTRL: sync_selection = 0;
		command();
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

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL: sync_target_x = 0; modified = 0; {
		if (ed_selection_available(ed) && ed->cy != ed->sel_y) {
			ed_move_to_selection_start(ed);
			break;
		}

		usz vstep = ++ed_globals->consec_cup * ed_globals->vstep;
		for (usz i = 0; i < vstep; ++i)
			ed_cur_up(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy - 1));
	}	break;

	case LT_TERM_KEY_UP | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: sync_selection = 0; sync_target_x = 0; modified = 0; {
		usz vstep = ++ed_globals->consec_cup * ed_globals->vstep;
		for (usz i = 0; i < vstep; ++i)
			ed_cur_up(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy - 1));
	}	break;


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

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL: sync_target_x = 0; modified = 0; {
		if (ed_selection_available(ed) && ed->cy != ed->sel_y) {
			ed_move_to_selection_end(ed);
			break;
		}

		usz vstep = ++ed_globals->consec_cdn * ed_globals->vstep;
		for (usz i = 0; i < vstep; ++i)
			ed_cur_down(ed, ed_screen_x_to_cx(ed, ed->target_cx, ed->cy + 1));
	}	break;

	case LT_TERM_KEY_DOWN | LT_TERM_MOD_CTRL | LT_TERM_MOD_SHIFT: sync_selection = 0; sync_target_x = 0; modified = 0; {
		usz vstep = ++ed_globals->consec_cdn * ed_globals->vstep;
		for (usz i = 0; i < vstep; ++i)
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
			doc_erase_str(&ed->doc, ed->cy, ed->cx, lt_utf8_decode_len(ed->doc.lines[ed->cy].str[ed->cx]));
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
		lstr_t name = ed->doc.name;
		char name_buf[PATH_MAX_LEN];

		if (name.len < 2 || name.str[name.len - 2] != '.')
			break;

		char ext = name.str[name.len - 1];

		char new_ext = 0;
		if (ext == 'c')
			new_ext = 'h';
		else if (ext == 'h')
			new_ext = 'c';
		else
			break;

		memcpy(name_buf, name.str, name.len);
		name_buf[name.len - 1] = new_ext;

		editor_t* file = fb_find_file(LSTR(name_buf, name.len));
		if (file)
			edit_file(ed_globals, file);
	}	break;

	case LT_TERM_KEY_BPASTE: {
		while ((c = lt_term_getkey()) != LT_TERM_KEY_NBPASTE) {
			if (c == '\n') {
				doc_split_line(&ed->doc, ed->cy, ed->cx);
				ed_cur_down(ed, 0);
			}
			else {
				char utf8_buf[4];
				usz utf8_len = lt_utf8_encode(utf8_buf, c);
				doc_insert_str(&ed->doc, ed->cy, ed->cx, LSTR(utf8_buf, utf8_len));
				ed->cx += utf8_len;
			}
		}
	}	break;

	default: {
		lstr_t cmd = lookup_keybind(c);
		if (cmd.len) {
			execute_string(ed, cmd);
			sync_selection = 0;
			break;
		}

 		if (lt_is_unicode_control_char(c) || (c & (LT_TERM_KEY_SPECIAL_BIT | LT_TERM_MOD_MASK))) {
 			modified = 0;
 			sync_selection = 0;
 			break;
 		}

 		ed_delete_selection_if_available(ed);

		char utf8_buf[4];
		usz utf8_len = lt_utf8_encode(utf8_buf, c);
		doc_insert_str(&ed->doc, ed->cy, ed->cx, LSTR(utf8_buf, utf8_len));
		ed->cx += utf8_len;
	}	break;
	}

	if (sync_target_x)
		ed_sync_target_cx(ed);
	if (sync_target_y)
		ed_sync_target_cy(ed);
	if (sync_selection)
		ed_sync_selection(ed);

	ed_adjust_screen_pos(ed);

	if (modified)
		ed_regenerate_hl(ed);
}

