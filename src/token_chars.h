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

	/*80*/ 0, 0, 0, 0, /*84*/ 0, 0, 0, 0, /*88*/ 0, 0, 0, 0, /*8C*/ 0, 0, 0, 0,
	/*90*/ 0, 0, 0, 0, /*94*/ 0, 0, 0, 0, /*98*/ 0, 0, 0, 0, /*9C*/ 0, 0, 0, 0,
	/*A0*/ 0, 0, 0, 0, /*A4*/ 0, 0, 0, 0, /*A8*/ 0, 0, 0, 0, /*AC*/ 0, 0, 0, 0,
	/*B0*/ 0, 0, 0, 0, /*B4*/ 0, 0, 0, 0, /*B8*/ 0, 0, 0, 0, /*BC*/ 0, 0, 0, 0,
	/*C0*/ 0, 0, 0, 0, /*C4*/ 0, 0, 0, 0, /*C8*/ 0, 0, 0, 0, /*CC*/ 0, 0, 0, 0,
	/*D0*/ 0, 0, 0, 0, /*D4*/ 0, 0, 0, 0, /*D8*/ 0, 0, 0, 0, /*DC*/ 0, 0, 0, 0,
	/*E0*/ 0, 0, 0, 0, /*E4*/ 0, 0, 0, 0, /*E8*/ 0, 0, 0, 0, /*EC*/ 0, 0, 0, 0,
	/*F0*/ 0, 0, 0, 0, /*F4*/ 0, 0, 0, 0, /*F8*/ 0, 0, 0, 0, /*FC*/ 0, 0, 0, 0,
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
