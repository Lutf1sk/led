// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef ASSERT_H
#define ASSERT_H 1

int assert_failed(char* file, int line, char* assertion);
int assert_unreachable_failed(char* file, int line);

#endif
