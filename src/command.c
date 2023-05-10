#include <lt/str.h>
#include <lt/ctype.h>
#include <lt/math.h>

#include "algo.h"
#include "clipboard.h"
#include "command.h"

typedef
struct ctx {
	editor_t* ed;
	char* it;
	char* end;
} ctx_t;

typedef
struct pos {
	isz line;
	isz col;
} pos_t;

static void skip_single_command(ctx_t* cx);
static void execute_single_command(ctx_t* cx);

static
usz parse_uint(ctx_t* cx) {
	char* start = cx->it;
	while (cx->it < cx->end && lt_is_digit(*cx->it))
		++cx->it;
	return lt_lstr_uint(LSTR(start, cx->it - start));
}

static
lstr_t parse_block(ctx_t* cx) {
	if (cx->it >= cx->end)
		return NLSTR();

	char* start = cx->it;
	while (cx->it < cx->end && *cx->it != '`')
		skip_single_command(cx);
	lstr_t block = LSTR(start, cx->it - start);
	if (cx->it < cx->end && *cx->it == '`')
		++cx->it;
	return block;
}

static
lstr_t parse_string(ctx_t* cx) {
	if (cx->it >= cx->end)
		return NLSTR();

	char* start = cx->it;
	while (cx->it < cx->end && *cx->it != '`')
		++cx->it;
	lstr_t block = LSTR(start, cx->it - start);
	if (cx->it < cx->end && *cx->it == '`')
		++cx->it;
	return block;
}

static
pos_t parse_pos(ctx_t* cx) {
	pos_t pos;
	pos.line = cx->ed->cy;
	pos.col = cx->ed->cx;

	if (cx->it >= cx->end)
		return pos;
	switch (*cx->it++) {
	case 'f': pos.col += parse_uint(cx); break;
	case 'b': pos.col -= parse_uint(cx); break;
	case 'u': pos.line -= parse_uint(cx); break;
	case 'd': pos.line += parse_uint(cx); break;

	case 'w':
		if (cx->it >= cx->end)
			break;
		switch (*cx->it++) {
		case 'f': pos.col = ed_find_word_fwd(cx->ed); break;
		case 'b': pos.col = ed_find_word_bwd(cx->ed); break;
		}

	case 't':
		pos.line = 0;
		pos.col = 0;
		break;

	case 'e':
		pos.line = cx->ed->doc.line_count - 1;
		pos.col = cx->ed->doc.lines[pos.line].len;
		break;

	case 's':
		if (cx->it >= cx->end)
			break;
		switch (*cx->it++) {
		case 's': ed_get_selection(cx->ed, &pos.line, &pos.col, NULL, NULL); break;
		case 'e': ed_get_selection(cx->ed, NULL, NULL, &pos.line, &pos.col); break;
		}
		break;

	case 'l':
		if (cx->it >= cx->end)
			break;
		switch (*cx->it++) {
		case 's': pos.col = 0; break;
		case 'e': pos.col = cx->ed->doc.lines[pos.line].len; break;
		default: --cx->it; pos.line = lt_max_isz(parse_uint(cx) - 1, 0); break;
		}
		break;

	case 'i': pos.col = ed_find_indent(cx->ed); break;
	case 'c': pos.col = parse_uint(cx); break;
	case ' ': break;
	case '`': --cx->it; break;
	}

	return pos;
}

static
void skip_single_command(ctx_t* cx) {
	switch (*cx->it++) {
	case 'j': case 's': parse_pos(cx); break;
	case 'c': parse_uint(cx); break;
	case 'p': parse_uint(cx); break;
	case 'd': break;
	case 'l': parse_uint(cx); parse_block(cx); break;
	case 'i': parse_string(cx); break;
	case 'f': parse_string(cx); break;
	case ' ': break;
	case '`': --cx->it; break;
	}
}

static
void execute_single_command(ctx_t* cx) {
	b8 sync_selection = 0;

	switch (*cx->it++) {
	case 'j': sync_selection = 1;
	case 's': {
		pos_t pos = parse_pos(cx);
		cx->ed->cy = clamp(pos.line, 0, cx->ed->doc.line_count - 1);
		cx->ed->cx = clamp(pos.col, 0, cx->ed->doc.lines[cx->ed->cy].len);
		if (sync_selection)
			ed_sync_selection(cx->ed);
	}	break;

	case 'c': {
		usz clipboard = parse_uint(cx);
		if (clipboard >= CLIPBOARD_COUNT)
			break;
		clipboard_clear(clipboard);
		ed_write_selection_str(cx->ed, (lt_io_callback_t)lt_strstream_write, &clipboards[clipboard]);
	}	break;

	case 'p': {
		usz clipboard = parse_uint(cx);
		if (clipboard >= CLIPBOARD_COUNT)
			break;
		ed_insert_string(cx->ed, clipboards[clipboard].str);
		ed_sync_selection(cx->ed);
	}	break;

	case 'd':
		ed_delete_selection_if_available(cx->ed);
		ed_sync_selection(cx->ed);
		break;

	case 'l': {
		usz iterations = parse_uint(cx);
		lstr_t block = parse_block(cx);
		for (usz i = 0; i < iterations; ++i)
			execute_string(cx->ed, block);
	}	break;

	case 'i':
		ed_insert_string(cx->ed, parse_string(cx));
		ed_sync_selection(cx->ed);
		break;

	case 'f': {
		doc_pos_t pos;
		if (!ed_find_next_occurence(cx->ed, parse_string(cx), &pos))
			break;

		cx->ed->cy = pos.y;
		cx->ed->cx = pos.x;
		ed_sync_selection(cx->ed);
	}	break;

	case ' ': break;
	case '`': --cx->it; break;
	}
}

void execute_string(editor_t* ed, lstr_t cmd) {
	ctx_t cx;
	cx.ed = ed;
	cx.it = cmd.str;
	cx.end = cmd.str + cmd.len;

	while (cx.it < cx.end)
		execute_single_command(&cx);
	ed_adjust_screen_pos(ed);
}
