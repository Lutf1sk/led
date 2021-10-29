// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "focus.h"

focus_t focus = { NULL, NULL, NULL };

void focus_none(void) {
	focus = (focus_t){ NULL, NULL, NULL };
}

