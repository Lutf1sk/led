// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef CLR_H
#define CLR_H

#define CLR_LINENUM 1
#define CLR_LINENUM_UFLOW 2
#define CLR_LINENUM_SEL 3
#define CLR_HEADER_TAB 4
#define CLR_EDITOR 5
#define CLR_EDITOR_SEL 6

#define CLR_SYNTAX_UNKNOWN 11

#define CLR_SYNTAX_STRING 13
#define CLR_SYNTAX_CHAR 14
#define CLR_SYNTAX_NUMBER 15

#define CLR_SYNTAX_IDENTIFIER 16
#define CLR_SYNTAX_KEYWORD 17
#define CLR_SYNTAX_COMMENT 18
#define CLR_SYNTAX_DATATYPE 19

#define CLR_SYNTAX_HASH 20
#define CLR_SYNTAX_OPERATOR 22
#define CLR_SYNTAX_PUNCTUATION 23
#define CLR_SYNTAX_FUNCTION 24

#define CLR_NOTIFY_ERROR 25

#define CLR_BROWSE_FILES_INPUT 26
#define CLR_BROWSE_FILES_ENTRY 27
#define CLR_BROWSE_FILES_SEL 28

#define CLR_HEADER_BG 29

#define CLR_SYNTAX_TRAIL_INDENT 30

typedef struct conf conf_t;

extern char* clr_strs[];

void clr_load(conf_t* clr_conf);

#endif
