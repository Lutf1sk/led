#include <lt/str.h>
#include <lt/ctype.h>
#include <lt/math.h>
#include <lt/mem.h>

#include "clipboard.h"
#include "command.h"
#include "highlight.h"

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
	b8 sync_tx;
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
lstr_t parse_name(ctx_t* cx) {
	char* start = cx->it;
	while (cx->it < cx->end && lt_is_ident_body(*cx->it)) {
		cx->it++;
	}
	return LSTR(start, cx->it - start);
}

static
void skip_whitespace(ctx_t* cx) {
	while (cx->it < cx->end && lt_is_space(*cx->it)) {
		cx->it++;
	}
}

static
pos_t parse_pos(ctx_t* cx) {
	lt_texted_t* txed = &cx->ed->doc->ed;

	pos_t pos;
	pos.line = txed->cursor_y;
	pos.col = txed->cursor_x;
	pos.sync_tx = 0;

	if (cx->it >= cx->end) {
		return pos;
	}

	switch (*cx->it++) {
	case 'f': pos.col += parse_uint(cx); pos.sync_tx = 1; break;
	case 'b': pos.col -= parse_uint(cx); pos.sync_tx = 1; break;
	case 'u': pos.line -= parse_uint(cx); break;
	case 'd': pos.line += parse_uint(cx); break;

	case 'w':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 'f': pos.col = lt_texted_find_word_fwd(txed); pos.sync_tx = 1; break;
		case 'b': pos.col = lt_texted_find_word_bwd(txed); pos.sync_tx = 1; break;
		default: --cx->it; break;
		}
		break;

	case 't':
		pos.line = 0;
		pos.col = 0;
		pos.sync_tx = 1;
		break;

	case 'e':
		pos.line = lt_texted_line_count(txed) - 1;
		pos.col = lt_texted_line_len(txed, pos.line);
		pos.sync_tx = 1;
		break;

	case 's':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 's': lt_texted_get_selection(txed, &pos.col, &pos.line, NULL, NULL); pos.sync_tx = 1; break;
		case 'e': lt_texted_get_selection(txed, NULL, NULL, &pos.col, &pos.line); pos.sync_tx = 1; break;
		default: --cx->it; break;
		}
		break;

	case 'l':
		if (cx->it >= cx->end) {
			break;
		}
		switch (*cx->it++) {
		case 's': pos.col = 0; pos.sync_tx = 1; break;
		case 'e': pos.col = lt_texted_line_len(txed, pos.line); pos.sync_tx = 1; break;
		default: --cx->it; pos.line = lt_isz_max(parse_uint(cx) - 1, 0); pos.sync_tx = 1; break;
		}
		break;

	case 'i': pos.col = lt_texted_count_line_leading_indent(txed, pos.line); pos.sync_tx = 1; break;
	case 'c': pos.col = parse_uint(cx); pos.sync_tx = 1; break;
	case ' ': case '\t': case '\v': break;
	default: --cx->it; break;
	}

	pos.line = lt_clamp(pos.line, 0, lt_texted_line_count(txed) - 1);
	pos.col = lt_clamp(pos.col, 0, lt_texted_line_len(txed, pos.line));

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

	case 'p': {
		pos_t cmp_pos = parse_pos(cx);
		return txed->cursor_x == cmp_pos.col && txed->cursor_y == cmp_pos.line;
	}

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
	case 'a': break;
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
	case '\\': cx->it = cx->end; break;
	case ' ': case '\t': case '\v': break;
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
		txed->cursor_y = lt_min(pos.line, lt_texted_line_count(txed));
		txed->cursor_x = lt_min(pos.col, lt_texted_line_len(txed, pos.line));
		if (pos.sync_tx) {
			lt_texted_sync_tx(txed);
		}
		else {
			lt_texted_gototx(txed);
		}
		if (sync_selection) {
			lt_texted_sync_selection(txed);
		}
	}	break;

	case 'c': {
		usz clipboard = parse_uint(cx);
		if (clipboard >= CLIPBOARD_COUNT) {
			break;
		}
		clipboard_clear(clipboard);
		lt_texted_write_selection(txed, (lt_write_fn_t)lt_strstream_write, &clipboards[clipboard]);
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
		if (lt_texted_selection_present(txed)) {
			lt_texted_erase_selection(txed);
			cx->modified = 1;
		}
		break;

	case 'a':
		auto_indent_selection(cx->ed);
		cx->modified = 1;
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
		lstr_t if_true = parse_block(cx);
		if (cond) {
			execute_string(cx->ed, if_true);
		}
		if (cx->it >= cx->end || *cx->it != 'e') {
			break;
		}
		++cx->it;
		lstr_t if_false = parse_block(cx);
		if (!cond)
			execute_string(cx->ed, if_false);
	}	break;

	case 'f': {
		if (cx->it >= cx->end) {
			break;
		}

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

	case '\\': {
		if (cx->it >= cx->end) {
			break;
		}

		lstr_t command = parse_name(cx);
		if (lt_lseq(command, CLSTR("mode"))) {
			skip_whitespace(cx);
			lstr_t mode_str = parse_string(cx);
			modeid_t mode = hl_find_mode_by_name(mode_str);
			if (mode != HL_UNKNOWN)
				cx->ed->doc->hl_mode = mode;
		}
		else if (lt_lseq(command, CLSTR("tabs"))) {
			cx->ed->tabs_to_spaces = 0;
		}
		else if (lt_lseq(command, CLSTR("spaces"))) {
			cx->ed->tabs_to_spaces = 1;
		}
		else if (lt_lseq(command, CLSTR("tabsize"))) {
			skip_whitespace(cx);
			lstr_t size_str = parse_string(cx);
			u64 size;
			if (lt_lstou(size_str, &size) == LT_SUCCESS && size && size < 256) {
				cx->ed->tab_size = size;
				update_tab_size(size);
			}
		}

		cx->it = cx->end;
		break;
	}

	case ' ': case '\t': case '\v': break;
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
