#include "utf8.h"

usz utf8_encode(char* out, u32 v) {
	char* it = out;
	if (v < 0x80) // If it is an ascii char
		*it++ = v;
	else if (v < 0x800) { // If it fits within 11 bits
		*it++ = 0xC0 | (v >> 6);
		*it++ = 0x80 | (v & 0x3F);
	}
	else if (v < 0x10000) { // If it fits within 16 bits
		*it++ = 0xE0 | (v >> 12);
		*it++ = 0x80 | ((v >> 6) & 0x3F);
		*it++ = 0x80 | ((v >> 0) & 0x3F);
	}
	else { // If it needs the whole 21 bit space
		*it++ = 0xE0 | (v >> 18);
		*it++ = 0x80 | ((v >> 12) & 0x3F);
		*it++ = 0x80 | ((v >> 6) & 0x3F);
		*it++ = 0x80 | ((v >> 0) & 0x3F);
	}
	return it - out;
}

usz utf8_decode(char* str, u32* out) {
	u32 c = *str;
	if (!(c & 0x80)) {
		*out = c;
		return 1;
	}
	else if ((c & 0xF0) == 0xC0) {
		*out = ((c & 0x1F) << 6) | (str[1] & 0x3F);
		return 2;
	}
	else if ((c & 0xF0) == 0xE0) {
		*out = ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
		return 3;
	}
	else if ((c & 0xF0) == 0xF0) {
		*out = ((c & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
		return 4;
	}
	else
		return 1;
}

usz utf8_decode_len(u8 v) {
	if ((v & 0xF0) == 0xF0)
		return 4;
	else if ((v & 0xE0) == 0xE0)
		return 3;
	else if ((v & 0xC0) == 0xC0)
		return 2;
	else
		return 1;
}

usz utf8_encode_len(u32 v) {
	if (v < 0x80)
		return 1;
	else if (v < 0x800)
		return 2;
	else if (v < 0x10000)
		return 3;
	else
		return 4;
}

