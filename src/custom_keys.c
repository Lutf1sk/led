// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "custom_keys.h"

#include <curses.h>

void register_custom_keys(void) {
	define_key("\x1B[1;5A", KEY_CUP);
	define_key("\x1B[1;5B", KEY_CDOWN);
	define_key("\x1B[1;5C", KEY_CRIGHT);
	define_key("\x1B[1;5D", KEY_CLEFT);

	define_key("\x1B[1;6A", KEY_CSUP);
	define_key("\x1B[1;6B", KEY_CSDOWN);
	define_key("\x1B[1;6C", KEY_CSRIGHT);
	define_key("\x1B[1;6D", KEY_CSLEFT);

	define_key("\x1B[1;3C", KEY_ARIGHT);
	define_key("\x1B[1;3D", KEY_ALEFT);

	define_key("\x1B[1;2A", KEY_SUP);
	define_key("\x1B[1;2B", KEY_SDOWN);

	define_key("\x1B[3;5~", KEY_CDC);

	define_key("\x1B[Z", KEY_STAB);
}
