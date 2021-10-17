// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef TOKEN_CHARS_H
#define TOKEN_CHARS_H 1

#define NUMH (1 << 0)
#define NUMB (1 << 1)
#define NUMR (NUMH | NUMB)

#define IDNH (1 << 2)
#define IDNB (1 << 3)
#define IDNT (IDNH | IDNB)

#define IDNR (IDNT | NUMB)
#define IBNR (IDNB | NUMR)

#define WHSP (1 << 4)
#define OPER (1 << 5)
#define PARN (1 << 6)
#define PUNC (1 << 7)

static
const u8 ctype_table[256] = {
	/*00*/ 0, 0, 0, 0, /*04*/ 0, 0, 0, 0, /*08*/ 0, WHSP, WHSP, WHSP, /*0C*/ WHSP, WHSP, 0, 0,
	/*10*/ 0, 0, 0, 0, /*14*/ 0, 0, 0, 0, /*18*/ 0, 0, 0, 0, /*1C*/ 0, 0, 0, 0,
	/*20*/ WHSP, OPER|PUNC, 0, 0, /*24*/ 0, OPER, OPER, 0, /*28*/ PARN|PUNC, PARN|PUNC, OPER, OPER, /*2C*/ 0, OPER, PUNC, OPER,
	/*30*/ IBNR, IBNR, IBNR, IBNR, /*34*/ IBNR, IBNR, IBNR, IBNR, /*38*/ IBNR, IBNR, PUNC, PUNC, /*3C*/ OPER, OPER, OPER, OPER|PUNC,
	/*40*/ 0, IDNR, IDNR, IDNR, /*44*/ IDNR, IDNR, IDNR, IDNR, /*48*/ IDNR, IDNR, IDNR, IDNR, /*4C*/ IDNR, IDNR, IDNR, IDNR,
	/*50*/ IDNR, IDNR, IDNR, IDNR, /*54*/ IDNR, IDNR, IDNR, IDNR, /*58*/ IDNR, IDNR, IDNR, PARN|PUNC, /*5C*/ 0, PARN|PUNC, OPER, IDNR,
	/*60*/ 0, IDNR, IDNR, IDNR, /*64*/ IDNR, IDNR, IDNR, IDNR, /*68*/ IDNR, IDNR, IDNR, IDNR, /*6C*/ IDNR, IDNR, IDNR, IDNR,
	/*70*/ IDNR, IDNR, IDNR, IDNR, /*74*/ IDNR, IDNR, IDNR, IDNR, /*78*/ IDNR, IDNR, IDNR, PARN|PUNC, /*7C*/ OPER, PARN|PUNC, OPER, 0,

	/*80*/ IDNR, IDNR, IDNR, IDNR, /*84*/ IDNR, IDNR, IDNR, IDNR, /*88*/ IDNR, IDNR, IDNR, IDNR, /*8C*/ IDNR, IDNR, IDNR, IDNR,
	/*90*/ IDNR, IDNR, IDNR, IDNR, /*94*/ IDNR, IDNR, IDNR, IDNR, /*98*/ IDNR, IDNR, IDNR, IDNR, /*9C*/ IDNR, IDNR, IDNR, IDNR,
	/*A0*/ IDNR, IDNR, IDNR, IDNR, /*A4*/ IDNR, IDNR, IDNR, IDNR, /*A8*/ IDNR, IDNR, IDNR, IDNR, /*AC*/ IDNR, IDNR, IDNR, IDNR,
	/*B0*/ IDNR, IDNR, IDNR, IDNR, /*B4*/ IDNR, IDNR, IDNR, IDNR, /*B8*/ IDNR, IDNR, IDNR, IDNR, /*BC*/ IDNR, IDNR, IDNR, IDNR,
	/*C0*/ IDNR, IDNR, IDNR, IDNR, /*C4*/ IDNR, IDNR, IDNR, IDNR, /*C8*/ IDNR, IDNR, IDNR, IDNR, /*CC*/ IDNR, IDNR, IDNR, IDNR,
	/*D0*/ IDNR, IDNR, IDNR, IDNR, /*D4*/ IDNR, IDNR, IDNR, IDNR, /*D8*/ IDNR, IDNR, IDNR, IDNR, /*DC*/ IDNR, IDNR, IDNR, IDNR,
	/*E0*/ IDNR, IDNR, IDNR, IDNR, /*E4*/ IDNR, IDNR, IDNR, IDNR, /*E8*/ IDNR, IDNR, IDNR, IDNR, /*EC*/ IDNR, IDNR, IDNR, IDNR,
	/*F0*/ IDNR, IDNR, IDNR, IDNR, /*F4*/ IDNR, IDNR, IDNR, IDNR, /*F8*/ IDNR, IDNR, IDNR, IDNR, /*FC*/ IDNR, IDNR, IDNR, IDNR,
};

static inline INLINE
u8 is_ident_head(u8 c) {
	return ctype_table[c] & IDNH;
}

static inline INLINE
u8 is_ident_body(u8 c) {
	return ctype_table[c] & IDNB;
}

static inline INLINE
u8 is_numeric_head(u8 c) {
	return ctype_table[c] & NUMH;
}

static inline INLINE
u8 is_digit(u8 c) {
	return ctype_table[c] & NUMH;
}

static inline INLINE
u8 is_numeric_body(u8 c) {
	return ctype_table[c] & NUMB;
}

static inline INLINE
u8 is_space(u8 c) {
	return ctype_table[c] & WHSP;
}

static inline INLINE
u8 is_operator(u8 c) {
	return ctype_table[c] & OPER;
}

static inline INLINE
u8 is_paren(u8 c) {
	return ctype_table[c] & PARN;
}

static inline INLINE
u8 is_punc(u8 c) {
	return ctype_table[c] & PUNC;
}

#endif
