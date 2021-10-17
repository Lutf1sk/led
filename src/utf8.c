#include "utf8.h"

usz utf8_encode(char* out, u32 v) {
	char* it = out;
	if (v < 0x80)
		*it++ = v;
	else if (v < 0x800) {
		*it++ = 0b11000000 | (v >> 6);
		*it++ = 0b10000000 | (v & 0b00111111);
	}
	return it - out;
}

usz utf8_decode(char* str, u32* out) {
	char c = *str;
	if ((c & 0xF0) == 0xF0)
		return 4;
	else if ((c & 0xE0) == 0xE0)
		return 3;
	else if ((c & 0xC0) == 0xC0)
		return 2;
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
