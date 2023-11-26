#include <lt/str.h>
#include <lt/ctype.h>
#include <lt/math.h>
#include <lt/mem.h>

#include "algo.h"
#include "clipboard.h"
#include "command.h"

typedef
struct ctx {
	editor_t* ed;
	char* it;
	char* end;
	b8 modified;
} ctx_t;

typedef
struct pos {
	isz line;
	isz col;
} pos_t;

static void skip_single_command(ctx_t* cx);
static void execute_single_command(ctx_t* cx);

static
u8 hex_char(char c) {
	if (c >= 'A' && c <= 'Z') {
		return (c - 'A' + 10);
	}
	if (c >= 'a' && c <= 'z') {
		return (c - 'a' + 10);
	}
	if (c >= '0' && c <= '9') {
		return (c - '0');
	}

	return 0; // TODO: Error checking
}

static
lstr_t unescape_string(lstr_t esc) {
	char* start = lt_malloc(lt_libc_heap, esc.len), *oit = start;

	char* iit = esc.str, *end = iit + esc.len;
	while (iit < end) {
		char c = *iit++;

		if (c != '\\' || iit >= end) {
			*oit++ = c;
			continue;
		}

		switch (*iit++) {
		case 'n': *oit++ = '\n'; break;
		case 'r': *oit++ = '\r'; break;
		case 't': *oit++ = '\t'; break;
		case 'v': *oit++ = '\v'; break;
		case 'b': *oit++ = '\b'; break;
		case '\\': *oit++ = '\\'; break;
		case '`': *oit++ = '`'; break;
		case 'x': {
			if (end - iit < 2) {
				*oit++ = '\\';
				*oit++ = 'x';
				break;
			}
			u8 b = hex_char(*iit++) << 4;
			*oit++ = b | hex_char(*iit++);
		}	break;
		}
	}

	return LSTR(start, oit - start);
}

static
usz parse_uint(ctx_t* cx) {
	char* start = cx->it;
	while (cx->it < cx->end && lt_is_digit(*cx->it)) {
		++cx->it;
	}
	u64 val;
	if (lt_lstou(LSTR(start, cx->it - start), &val) != LT_SUCCESS) {
		return 0;
	}
	return val;
}

static
lstr_t parse_block(ctx_t* cx) {
	if (cx->it >= cx->end) {
		return NLSTR();
	}

	char* start = cx->it;
	while (cx->it < cx->end && *cx->it != '`') {
		skip_single_command(cx);
	}
	lstr_t block = LSTR(start, cx->it - start);
	if (cx->it < cx->end && *cx->it == '`') {
		++cx->it;
	}
	return block;
}

static
lstr_t parse_string(ctx_t* cx) {
	if (cx->it >= cx->end)
		return NLSTR();

	char* start = cx->it;
	char last = 0;
	while (cx->it < cx->end && !(*cx->it == '`' && last != '\\')) {
		last = *cx->it++;
	}

	lstr_t block = LSTR(start, cx->it - start);
	if (cx->it < cx->end && *cx->it == '`') {
		++cx->it;
	}
	return block;
}

static
pos_t parse_pos(ctx_t* cx) {
	lt_texted_t* txed = &cx->ed->doc->ed;

	pos_t pos;
	pos.line = txed->cursor_y;
	pos.col = txed->cursor_x;

	if (cx->it >= cx->end) {
		return pos;
	}

	switch (*cx->it++) {
	case 'f': pos.col += parse_uint(cx); break;
	case 'b': pos.col -= parse_uint(cx); break;
	case 'u': pos.line -= parse_uint(cx); break;
	case 'd': pos.line += parse_uint(cx); break;

	case 'w':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 'f': pos.col = lt_texted_find_word_fwd(txed); break;
		case 'b': pos.col = lt_texted_find_word_bwd(txed); break;
		default: --cx->it; break;
		}
		break;

	case 't':
		pos.line = 0;
		pos.col = 0;
		break;

	case 'e':
		pos.line = lt_texted_line_count(txed) - 1;
		pos.col = lt_texted_line_len(txed, pos.line);
		break;

	case 's':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 's': lt_texted_get_selection(txed, &pos.line, &pos.col, NULL, NULL); break;
		case 'e': lt_texted_get_selection(txed, NULL, NULL, &pos.line, &pos.col); break;
		default: --cx->it; break;
		}
		break;

	case 'l':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 's': pos.col = 0; break;
		case 'e': pos.col = lt_texted_line_len(txed, pos.line); break;
		default: --cx->it; pos.line = lt_max_isz(parse_uint(cx) - 1, 0); break;
		}
		break;

	case 'i': pos.col = lt_texted_count_line_leading_indent(txed, pos.line); break;
	case 'c': pos.col = parse_uint(cx); break;
	case ' ': break;
	default: --cx->it; break;
	}

	return pos;
}

static
b8 parse_condition(ctx_t* cx) {
	lt_texted_t* txed = &cx->ed->doc->ed;

	if (cx->it >= cx->end) {
		return 0;
	}

	switch (*cx->it++) {
	case 's':
		if (cx->it >= cx->end || *cx->it != 'p') {
			break;
		}
		++cx->it;
		return lt_texted_selection_present(txed);

	case 'c':
		usz clipboard = parse_uint(cx);
		if (clipboard >= CLIPBOARD_COUNT || cx->it >= cx->end || *cx->it != 'p') {
			return 0;
		}
		++cx->it;
		return clipboards[clipboard].str.len > 0;

	default: --cx->it; break;
	}
	return 0;
}

static
void skip_single_command(ctx_t* cx) {
	switch (*cx->it++) {
	case 'j': case 's': parse_pos(cx); break;
	case 'c': parse_uint(cx); break;
	case 'p': parse_uint(cx); break;
	case 'd': break;
	case 'l': parse_uint(cx); parse_block(cx); break;
	case 'w': parse_string(cx); break;
	case 'i':
		parse_condition(cx); parse_block(cx);
		if (cx->it >= cx->end || *cx->it != 'e') {
			break;
		}
		++cx->it; parse_block(cx);
		break;
	case 'f':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 'f': parse_string(cx); break;
		case 'b': parse_string(cx); break;
		}
		break;
	case ' ': break;
	case '`': --cx->it; break;
	}
}

static
void execute_single_command(ctx_t* cx) {
	lt_texted_t* txed = &cx->ed->doc->ed;

	b8 sync_selection = 0;

	switch (*cx->it++) {
	case 'j': sync_selection = 1;
	case 's': {
		pos_t pos = parse_pos(cx);
		lt_texted_gotoxy(txed, pos.col, pos.line, sync_selection);
	}	break;

	case 'c': {
		usz clipboard = parse_uint(cx);
		if (clipboard >= CLIPBOARD_COUNT) {
			break;
		}
		clipboard_clear(clipboard);
		lt_texted_write_selection(txed, (lt_io_callback_t)lt_strstream_write, &clipboards[clipboard]);
	}	break;

	case 'p': {
		usz clipboard = parse_uint(cx);
		if (clipboard >= CLIPBOARD_COUNT) {
			break;
		}
		if (lt_texted_input_str(txed, clipboards[clipboard].str)) {
			cx->modified = 1;
		}
	}	break;

	case 'd':
		if (lt_texted_selection_present(&cx->ed->doc->ed)) {
			lt_texted_erase_selection(&cx->ed->doc->ed);
			cx->modified = 1;
		}
		break;

	case 'l': {
		usz iterations = parse_uint(cx);
		lstr_t block = parse_block(cx);
		for (usz i = 0; i < iterations; ++i) {
			execute_string(cx->ed, block);
		}
	}	break;

	case 'w': {
		lstr_t str = unescape_string(parse_string(cx));
		if (lt_texted_input_str(txed, str)) {
			cx->modified = 1;
		}
		lt_mfree(lt_libc_heap, str.str);
	}	break;

	case 'i': {
		u8 cond = parse_condition(cx);
		lstr_t true = parse_block(cx);
		if (cond) {
			execute_string(cx->ed, true);
		}
		if (cx->it >= cx->end || *cx->it != 'e') {
			break;
		}
		++cx->it;
		lstr_t false = parse_block(cx);
		if (!cond)
			execute_string(cx->ed, false);
	}	break;

	case 'f': {
		if (cx->it >= cx->end)
			break;

		usz x, y;
		lstr_t find = NLSTR();
		switch (*cx->it++) {
		case 'f':
			find = unescape_string(parse_string(cx));
			if (!lt_texted_find_next_occurence(txed, find, &x, &y)) {
				break;
			}
			goto found;

		case 'b':
			find = unescape_string(parse_string(cx));
			if (!lt_texted_find_last_occurence(txed, find, &x, &y)) {
				break;
			}

		found:
			lt_texted_gotoxy(txed, x, y, 1);
			if (cx->it >= cx->end || *cx->it != 'r') {
				break;
			}

			++cx->it;
			lstr_t replace = unescape_string(parse_string(cx));
			lt_texted_erase_range(txed, x, y, x + find.len, y);
			lt_texted_input_str(txed, replace);

			if (find.len && !lt_lseq(find, replace)) {
				cx->modified = 1;
			}
			lt_mfree(lt_libc_heap, replace.str);
		}

		if (find.str) {
			lt_mfree(lt_libc_heap, find.str);
		}
	}	break;

	case ' ': break;
	}
}

b8 execute_string(editor_t* ed, lstr_t cmd) {
	ctx_t cx;
	cx.ed = ed;
	cx.it = cmd.str;
	cx.end = cmd.str + cmd.len;
	cx.modified = 0;

	while (cx.it < cx.end) {
		execute_single_command(&cx);
	}
	adjust_vbounds(ed);

	if (cx.modified) {
		ed->doc->unsaved = 1;
	}
	return cx.modified;
}
