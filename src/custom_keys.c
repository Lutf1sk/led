// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "custom_keys.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

#include <curses.h>

void register_custom_keys(void) {
	char* term_str_data = getenv("TERM");
	lstr_t term_str = LSTR(term_str_data, strlen(term_str_data));

	// CTRL + Arrows
	define_key("\x1B[1;5A", KEY_CUP);
	define_key("\x1B[1;5B", KEY_CDOWN);
	define_key("\x1B[1;5C", KEY_CRIGHT);
	define_key("\x1B[1;5D", KEY_CLEFT);

	// CTRL + Shift + Arrows
	define_key("\x1B[1;6A", KEY_CSUP);
	define_key("\x1B[1;6B", KEY_CSDOWN);
	define_key("\x1B[1;6C", KEY_CSRIGHT);
	define_key("\x1B[1;6D", KEY_CSLEFT);

	// ALT + HArrows
	define_key("\x1B[1;3C", KEY_ARIGHT);
	define_key("\x1B[1;3D", KEY_ALEFT);

	// SHIFT + VArrows
	define_key("\x1B[1;2A", KEY_SUP);
	define_key("\x1B[1;2B", KEY_SDOWN);


	define_key("\x1B[3;5~", KEY_CDC);

	lstr_t xterm_str = CLSTR("xterm");

	// For some reason, XTerm switches \x08 and \x7f.
	// In XTerm, \x7f is CBACKSPACE and \0x08 is BACKSPACE.
	// This hack fixes that.
	if (term_str.len == xterm_str.len && memcmp(term_str.str, xterm_str.str, term_str.len) == 0)
		define_key("\x7f", KEY_CBACKSPACE);

	define_key("\x1B[Z", KEY_STAB);
}
