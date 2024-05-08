// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "draw.h"
#include "clr.h"

#include <lt/io.h>
#include <lt/texted.h>
#include <lt/text.h>
#include <lt/mem.h>
#include <lt/term.h>

#define LT_ANSI_SHORTEN_NAMES 1
#include <lt/ansi.h>

#include <string.h>

draw_buf_t buffers[2];

draw_buf_t* buf_last = &buffers[0];
draw_buf_t* buf_current = &buffers[1];

draw_char_t* buf_pos = 0;
draw_char_t* buf_line_end = 0;

void buf_set_cursor(i32 line, i32 col) {
	buf_current->cx = col;
	buf_current->cy = line;
}

void buf_set_pos(usz line, usz col) {
	buf_pos = buf_current->text + line * buf_current->w + col;
	buf_line_end = buf_pos + buf_current->w;
}

void buf_write_utf8(u32 attr, lstr_t str) {
	char* it = str.str, *end = it + str.len;
	while (it < end && buf_pos < buf_line_end) {
		u32 c;
		it += lt_utf8_decode(it, &c);
		*buf_pos++ = (draw_char_t) { .c = c, .attr = attr };
	}
}

void buf_write_char(u32 attr, u32 c) {
	if (buf_pos < buf_line_end) {
		*buf_pos++ = (draw_char_t) { .c = c, .attr = attr };
	}
}

void buf_writeln(usz line, const draw_char_t* text, usz count) {
	if (count > buf_current->w) {
		count = buf_current->w;
	}

	usz start_idx = line * buf_current->w;

	memcpy(buf_current->text + start_idx, text, count * sizeof(*text));
	if (count < buf_current->w) {
		buf_current->text[count] = (draw_char_t){0};
	}
}

void buf_writeln_utf8(usz line, u32 attr, lstr_t str) {
	draw_char_t* out = buf_current->text + line * buf_current->w, *out_end = out + buf_current->w;

	for (const char* it = str.str, *end = it + str.len; it < end && out < out_end;) {
		u32 c;
		it += lt_utf8_decode(it, &c);
		*out++ = (draw_char_t) { .c = c, .attr = attr };
	}

	if (out < out_end) {
		*out++ = (draw_char_t){0};
	}
}

char tmp_out_buf[LT_KB(256)];

void buf_present(void) {
	char* out = tmp_out_buf;

	out += lt_sprintf(out, CSET(1, 1));

	u32 attr = 0xFFFFFFFF;

	usz buf_chars = buf_current->w * buf_current->h;
	for (usz i = 0; i < buf_current->h; ++i) {
		if (i) {
			*out++ = '\n';
		}

		draw_char_t* pline_current = buf_current->text + i * buf_current->w;
		draw_char_t* pline_last    = buf_last->text    + i * buf_current->w;

		for (usz j = 0; j < buf_current->w; ++j) {
			draw_char_t dc = pline_current[j];

			if (!dc.c) {
				if (!(attr & ATTR_FILL)) {
					out += lt_sprintf(out, RESET);
					attr = 0;
				}
				out += lt_sprintf(out, CLL_TO_END);
				break;
			}

			if (dc.attr != attr) {
				attr = dc.attr;
				u32 bold = (attr & ATTR_BOLD) ? 1 : 22;
				u32 fg = 30 + ATTR_FG(attr) + 60 * !!(attr & ATTR_FG_BRIGHT);
				u32 bg = 40 + ATTR_BG(attr) + 60 * !!(attr & ATTR_BG_BRIGHT);
				if (bg == 40) {
					bg = 0;
				}
				out += lt_sprintf(out, "\x1b[%ud;%ud;%udm", bg, fg, bold);
			}

			*out++ = dc.c;
		}
	}

	out += lt_sprintf(out, "\x1b[%ud;%udH", buf_current->cy + 1, buf_current->cx + 1);

	lt_term_write_direct(tmp_out_buf, out - tmp_out_buf);

	buf_last = buf_current;
	if (buf_current == &buffers[0]) {
		buf_current = &buffers[1];
	}
	else {
		buf_current = &buffers[0];
	}
}

void buf_resize(usz w, usz h) {
	usz size = w * h * sizeof(draw_char_t);

	for (usz i = 0; i < LT_ARRAY_COUNT(buffers); ++i) {
		buffers[i].text = lt_hmrealloc(buffers[i].text, size);
		memset(buffers[i].text, 0, size);
		buffers[i].w = w;
		buffers[i].h = h;
	}
}

void buf_write_txed(u32 sel_attr, u32 attr, lt_texted_t* txed) {
	usz x1, x2;
	lt_texted_get_selection(txed, &x1, NULL, &x2, NULL);
	lstr_t str = lt_texted_line_str(txed, 0);

	buf_write_utf8(attr, LSTR(str.str, x1));
	buf_write_utf8(sel_attr, LSTR(str.str + x1, x2 - x1));
	buf_write_utf8(attr, LSTR(str.str + x2, str.len - x2));
}
