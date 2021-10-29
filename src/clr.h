// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef CLR_H
#define CLR_H

#include "common.h"

#define CLR_LINENUM 1
#define CLR_LINENUM_UFLOW 2
#define CLR_LINENUM_SEL 3

#define CLR_HEADER_TAB 4
#define CLR_HEADER_BG 5

#define CLR_EDITOR 6
#define CLR_EDITOR_SEL 7

#define CLR_SYNTAX_UNKNOWN 8

#define CLR_SYNTAX_STRING 9
#define CLR_SYNTAX_CHAR 10
#define CLR_SYNTAX_NUMBER 11

#define CLR_SYNTAX_IDENTIFIER 12
#define CLR_SYNTAX_KEYWORD 13
#define CLR_SYNTAX_COMMENT 14
#define CLR_SYNTAX_DATATYPE 15

#define CLR_SYNTAX_HASH 16
#define CLR_SYNTAX_OPERATOR 17
#define CLR_SYNTAX_PUNCTUATION 18
#define CLR_SYNTAX_FUNCTION 19

#define CLR_NOTIFY_ERROR 20

#define CLR_LIST_HEAD 21
#define CLR_LIST_ENTRY 22
#define CLR_LIST_HIGHL 23

#define CLR_SYNTAX_TRAIL_INDENT 24

#define CLR_COUNT 25

typedef struct conf conf_t;
typedef struct aframe aframe_t;

extern char* clr_strs[];

char* conf_clr_default(aframe_t* arena, conf_t* conf, lstr_t key, char* default_);

void clr_load(aframe_t* arena, conf_t* clr_conf);

#endif
