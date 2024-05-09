#include <lt/texted.h>
#include <lt/term.h>
#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/math.h>
#include <lt/strstream.h>

#include "clipboard.h"
#include "editor.h"
#include "focus.h"

focus_t focus_extract = { draw_extract, NULL, input_extract };

static b8 remove_src = 0;
static b8 insert_new = 0;

void extract(b8 remove_src_, b8 insert_new_) {
	remove_src = remove_src_;
	insert_new = insert_new_;

	focus = focus_extract;
}

void draw_extract(editor_t* editor, void* args) {

}

void input_extract(editor_t* ed, u32 c) {
	lt_texted_t* txed = &ed->doc->ed;

	c &= LT_TERM_KEY_MASK;

	usz nlines = 1;
	if (lt_is_digit(c)) {
		nlines = c - '0';
		goto select_lines;
	}

	switch (c) {
	case LT_TERM_KEY_BSPACE:
	case LT_TERM_KEY_ESC:
		edit_file(ed, ed->doc);
		return;

	case 'c':
		lt_texted_gotox(txed, lt_texted_count_line_leading_indent(txed, txed->cursor_y), 1);
		lt_texted_gotoxy(txed, -1, txed->cursor_y, 0);
		break;

	select_lines:
	case '\n': case ';': case 'l':
		lt_texted_gotox(txed, 0, 1);
		lt_texted_gotoy(txed, txed->cursor_y + nlines, 0);
		break;
	}

	clipboard_clear(0);
	lt_texted_write_selection(txed, (lt_write_fn_t)lt_strstream_write, &clipboards[0]);
	if (remove_src) {
		lt_texted_erase_selection(txed);
	}

	lt_texted_sync_selection(txed);
	if (insert_new) {
		lt_texted_input_str(txed, clipboards[0].str);
	}
	edit_file(ed, ed->doc);
}
