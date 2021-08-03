// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef CURSES_HELPERS_H
#define CURSES_HELPERS_H 1

#include "common.h"

#define CTRL_MOD_DIFF ('A' - 1)

static inline INLINE
void waddnch(WINDOW* win, usz num, char c) {
	for (usz i = 0; i < num; ++i)
		waddch(win, c);
}

static inline INLINE
void mvwaddnch(WINDOW* win, int y, int x, usz num, char c) {
	wmove(win, y, x);
	waddnch(win, num, c);
}

#endif
