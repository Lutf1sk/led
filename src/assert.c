// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "assert.h"

#include <stdio.h>
#include <stdlib.h>

int assert_failed(char* file, int line, char* assertion) {
	fprintf(stderr, "%s:%d: Assertion failed: %s\n", file, line, assertion);
	exit(1);
}

int assert_unreachable_failed(char* file, int line) {
	fprintf(stderr, "%s:%d: Unreachable assertion reached\n", file, line);
	exit(1);
}
