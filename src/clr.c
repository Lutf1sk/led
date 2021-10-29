// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "clr.h"
#include "common.h"
#include "conf.h"

#include <string.h>

char* clr_strs[256];

void clr_load(conf_t* clr_conf) {
	// Generate color pairs
	clr_strs[CLR_LINENUM] = "\x1B[22;37;100m";
	clr_strs[CLR_LINENUM_SEL] = "\x1B[22;30;47m";
	clr_strs[CLR_LINENUM_UFLOW] = "\x1B[22;37;100m";
	clr_strs[CLR_HEADER_TAB] = "\x1B[22;30;47m";
	clr_strs[CLR_HEADER_BG] = "\x1B[22;30;100m";
	clr_strs[CLR_EDITOR] = "\x1B[22;37;40m";
	clr_strs[CLR_EDITOR_SEL] = "\x1B[22;30;46m";

	// Syntax highlighting pairs
	clr_strs[CLR_SYNTAX_UNKNOWN] = "\x1B[22;37;40m";

	clr_strs[CLR_SYNTAX_STRING] = "\x1B[22;33;40m";
	clr_strs[CLR_SYNTAX_CHAR] = "\x1B[22;33;40m";
	clr_strs[CLR_SYNTAX_NUMBER] = "\x1B[22;37;40m";

	clr_strs[CLR_SYNTAX_IDENTIFIER] = "\x1B[22;97;40m";
	clr_strs[CLR_SYNTAX_KEYWORD] = "\x1B[22;93;40m";
	clr_strs[CLR_SYNTAX_COMMENT] = "\x1B[22;32;40m";
	clr_strs[CLR_SYNTAX_DATATYPE] = "\x1B[22;31;40m";

	clr_strs[CLR_SYNTAX_HASH] = "\x1B[22;37;40m";
	clr_strs[CLR_SYNTAX_OPERATOR] = "\x1B[22;36;40m";
	clr_strs[CLR_SYNTAX_PUNCTUATION] = "\x1B[22;31;40m";

	clr_strs[CLR_SYNTAX_FUNCTION] = "\x1B[1;97;40m";

	clr_strs[CLR_SYNTAX_TRAIL_INDENT] = "\x1B[22;30;41m";

	// Notification pairs
	clr_strs[CLR_NOTIFY_ERROR] = "\x1B[1;30;41m";

	// File browser pairs
	clr_strs[CLR_BROWSE_FILES_INPUT] = "\x1B[22;30;47m";
	clr_strs[CLR_BROWSE_FILES_ENTRY] = "\x1B[22;37;40m";
	clr_strs[CLR_BROWSE_FILES_SEL] = "\x1B[22;37;100m";
}

