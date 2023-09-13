// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef COMMAND_H
#define COMMAND_H 1

#include <lt/lt.h>

#include "editor.h"

b8 execute_string(editor_t* ed, lstr_t cmd);

#endif
