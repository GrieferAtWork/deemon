/* Copyright (c) 2018-2026 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_DB_C
#define GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_DB_C 1

#include <deemon/api.h>

#include <deemon/string.h> /* Dee_UNICODE_FOLDED_MAX, Dee_UNICODE_IS*, Dee_uniflag_t, Dee_unitraits */

#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/typecore.h>  /* __CHAR32_TYPE__ */

#include <stddef.h> /* size_t */
#include <stdint.h> /* UINT32_C, UINT64_C, int32_t, uintN_t */

DECL_BEGIN

#ifdef CONFIG_HAVE_UNICODE_H

PUBLIC WUNUSED ATTR_RETNONNULL ATTR_CONST struct Dee_unitraits const *
(DCALL DeeUni_Descriptor)(uint32_t ch) {
	return (struct Dee_unitraits *)__unicode_descriptor(ch);
}

PUBLIC size_t
(DCALL DeeUni_ToFolded)(uint32_t ch,
                        uint32_t buf[Dee_UNICODE_FOLDED_MAX]) {
	return (size_t)(unicode_fold((__CHAR32_TYPE__)ch,
	                             (__CHAR32_TYPE__ *)buf) -
	                (__CHAR32_TYPE__ *)buf);
}

PUBLIC ATTR_CONST WUNUSED uint8_t
(DCALL DeeUni_GetNumericIdx8)(uint8_t digit_idx) {
	return __unicode_descriptor_digit(digit_idx);
}

PUBLIC ATTR_CONST WUNUSED uint64_t
(DCALL DeeUni_GetNumericIdx64)(uint8_t digit_idx) {
	return __unicode_descriptor_digit64(digit_idx);
}

PUBLIC ATTR_CONST WUNUSED double
(DCALL DeeUni_GetNumericIdxD)(uint8_t digit_idx) {
	return __unicode_descriptor_digitd(digit_idx);
}

#else /* CONFIG_HAVE_UNICODE_H */

#define TCNTRL    Dee_UNICODE_ISCNTRL
#define TCTAB     Dee_UNICODE_ISCTAB
#define TXTAB     Dee_UNICODE_ISXTAB
#define TWHITE    Dee_UNICODE_ISWHITE
#define TLF       Dee_UNICODE_ISLF
#define TLOWER    Dee_UNICODE_ISLOWER
#define TUPPER    Dee_UNICODE_ISUPPER
#define TXALPHA   Dee_UNICODE_ISXALPHA
#define TDIGIT    Dee_UNICODE_ISDIGIT
#define THEX      Dee_UNICODE_ISHEX
#define TPUNCT    Dee_UNICODE_ISPUNCT
#define TTITLE    Dee_UNICODE_ISTITLE
#define TXNUMERIC Dee_UNICODE_ISXNUMERIC
#define TSYMSTRT  Dee_UNICODE_ISSYMSTRT
#define TSYMCONT  Dee_UNICODE_ISSYMCONT

/* Representation of a unicode character numerical value (s.a. `unicode_getnumeric(3)')
 * Because characters exist that represent  fractions (like ½), we  need to be able  to
 * represent  not just whole numbers, but also  fractions. For this purpose, the follow
 * structure  exists, which can either be a whole 63-bit unsigned number, or a fraction
 * consisting of 2x 31-bit signed integers. */
typedef union {
#define UNIDIGIT_ISFRAC     UINT64_C(0x8000000000000000)
#define UNIDIGIT_WHOLE_MASK UINT64_C(0x7fffffffffffffff)
	uint64_t ud_whole;   /* Whole number (masked by `UNIDIGIT_WHOLE_MASK') */
	uint32_t ud_frac[2]; /* Fraction numerator/denominator */
#define unidigit_iswhole(self)             (!((self)->ud_whole & UNIDIGIT_ISFRAC))
#define unidigit_getwhole(self)            ((self)->ud_whole /*& UNIDIGIT_WHOLE_MASK*/)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define unidigit_getfrac_numerator(self)   ((int32_t)((self)->ud_frac[0]))
#define unidigit_getfrac_denominator(self) ((int32_t)((self)->ud_frac[1] & UINT32_C(0x7fffffff)))
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define unidigit_getfrac_numerator(self)   ((int32_t)((self)->ud_frac[0] & UINT32_C(0x7fffffff)))
#define unidigit_getfrac_denominator(self) ((int32_t)((self)->ud_frac[1]))
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
} unidigit_t;
#define D_INT(value) { UINT64_C(value) }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _D_FRAC(a, b) { UNIDIGIT_ISFRAC | (uint64_t)a | ((uint64_t)b << 32) }
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define _D_FRAC(a, b) { UNIDIGIT_ISFRAC | ((uint64_t)a << 32) | (uint64_t)b }
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
#define D_FRAC(a, b) _D_FRAC((uint32_t)(int32_t)(a), (uint32_t)(int32_t)(b))



#ifdef __INTELLISENSE__
#define UNICODE_FOLD_MAXLEN 3
#define UNICODE_FOLD_COUNT  42
struct unifold {
	uint16_t uf_repl[UNICODE_FOLD_MAXLEN];
};
PRIVATE struct unifold const unicode_fold_descriptors[UNICODE_FOLD_COUNT];
#define UNICODE_DIGIT_COUNT 149
PRIVATE unidigit_t const unicode_digits[UNICODE_DIGIT_COUNT];
#define UNICODE_DESCRIPTOR_COUNT 42
PRIVATE struct Dee_unitraits const unicode_descriptors[UNICODE_DESCRIPTOR_COUNT];
#define UNICODE_COUNT                0xe01f0
#define UNICODE_SHIFT                7
#define UNICODE_TAB1_MASK            0x7f
#define UNICODE_DESCRIPTOR_INDEX(ch) unicode_tab2[(unicode_tab1[(uint32_t)(ch) >> 7] << 7) + ((uint8_t)(ch) & 0x7f)]
#define UNICODE_DESCRIPTOR(ch)       unicode_descriptors[UNICODE_DESCRIPTOR_INDEX(ch)]
#define unicode_default_descriptor   unicode_descriptors[0]
PRIVATE uint16_t const unicode_tab1[42];
PRIVATE uint16_t const unicode_tab2[42];
#else /* __INTELLISENSE__ */
#undef __unitraits
#define __unitraits Dee_unitraits
#undef ATTR_SECTION
#define ATTR_SECTION(x) /* nothing */
#include "db/db.dat"
#endif /* !__INTELLISENSE__ */

STATIC_ASSERT_MSG(UNICODE_FOLD_MAXLEN <= Dee_UNICODE_FOLDED_MAX,
                  "Dee_UNICODE_FOLDED_MAX must be increased!");

PUBLIC ATTR_CONST ATTR_RETNONNULL WUNUSED struct Dee_unitraits const *
(DCALL DeeUni_Descriptor)(uint32_t ch) {
	if likely(ch < UNICODE_COUNT)
		return &UNICODE_DESCRIPTOR(ch);
	return &unicode_default_descriptor;
}

PUBLIC ATTR_CONST WUNUSED uint8_t
(DCALL DeeUni_GetNumericIdx8)(uint8_t digit_idx) {
	unidigit_t const *digit;
	if unlikely(digit_idx >= UNICODE_DIGIT_COUNT)
		return 0;
	digit = &unicode_digits[digit_idx];
	if likely(unidigit_iswhole(digit))
		return (uint8_t)unidigit_getwhole(digit);
	return (uint8_t)(unidigit_getfrac_numerator(digit) /
	                 unidigit_getfrac_denominator(digit));
}

PUBLIC ATTR_CONST WUNUSED uint64_t
(DCALL DeeUni_GetNumericIdx64)(uint8_t digit_idx) {
	unidigit_t const *digit;
	if unlikely(digit_idx >= UNICODE_DIGIT_COUNT)
		return 0;
	digit = &unicode_digits[digit_idx];
	if likely(unidigit_iswhole(digit))
		return (uint64_t)unidigit_getwhole(digit);
	return (uint64_t)unidigit_getfrac_numerator(digit) /
	       (uint64_t)unidigit_getfrac_denominator(digit);
}

PUBLIC ATTR_CONST WUNUSED double
(DCALL DeeUni_GetNumericIdxD)(uint8_t digit_idx) {
	unidigit_t const *digit;
	if unlikely(digit_idx >= UNICODE_DIGIT_COUNT)
		return 0.0;
	digit = &unicode_digits[digit_idx];
	if likely(unidigit_iswhole(digit))
		return (double)unidigit_getwhole(digit);
	return (double)unidigit_getfrac_numerator(digit) /
	       (double)unidigit_getfrac_denominator(digit);
}

PUBLIC NONNULL((2)) size_t
(DCALL DeeUni_ToFolded)(uint32_t ch,
                        uint32_t buf[Dee_UNICODE_FOLDED_MAX]) {
	struct Dee_unitraits const *trt;
	struct unifold const *fold;
	trt = DeeUni_Descriptor(ch);
	if (trt->ut_fold_idx >= UNICODE_FOLD_COUNT) {
		buf[0] = ch + trt->ut_lower; /* tolower */
		return 1;
	}
	fold   = &unicode_fold_descriptors[trt->ut_fold_idx];
	buf[0] = fold->uf_repl[0];
#if UNICODE_FOLD_MAXLEN >= 2
	if ((buf[1] = fold->uf_repl[1]) == 0)
		return 1;
#endif /* UNICODE_FOLD_MAXLEN >= 2 */
#if UNICODE_FOLD_MAXLEN >= 3
	if ((buf[2] = fold->uf_repl[2]) == 0)
		return 2;
#endif /* UNICODE_FOLD_MAXLEN >= 3 */
#if UNICODE_FOLD_MAXLEN >= 4
	if ((buf[3] = fold->uf_repl[3]) == 0)
		return 3;
#endif /* UNICODE_FOLD_MAXLEN >= 4 */
#if UNICODE_FOLD_MAXLEN >= 5
#error "Not implemented"
#endif /* UNICODE_FOLD_MAXLEN >= 5 */
	return UNICODE_FOLD_MAXLEN;
}


PUBLIC Dee_uniflag_t const _DeeAscii_Flags[256] = {
/*[[[deemon

#define ascii_iscntrl(ch)    ((ch) <= 0x1f || (ch) == 0x7f)
#define ascii_isctab(ch)     ((ch) == 0x09)
#define ascii_isxtab(ch)     ((ch) >= 0x0b && (ch) <= 0x0c)
#define ascii_iswhite(ch)    ((ch) == 0x20)
#define ascii_islf(ch)       ((ch) == 0xa || (ch) == 0xd)
#define ascii_islower(ch)    ((ch) >= 0x61 && (ch) <= 0x7a)
#define ascii_isupper(ch)    ((ch) >= 0x41 && (ch) <= 0x5a)
#define ascii_isxalpha(ch)   false
#define ascii_isdigit(ch)    ((ch) >= 0x30 && (ch) <= 0x39)
#define ascii_ishex(ch)      (((ch) >= 0x41 && (ch) <= 0x46) || ((ch) >= 0x61 && (ch) <= 0x66))
#define ascii_ispunct(ch)    (((ch) >= 0x21 && (ch) <= 0x2f) || ((ch) >= 0x3a && (ch) <= 0x40) || ((ch) >= 0x5b && (ch) <= 0x60) || ((ch) >= 0x7b && (ch) <= 0x7e))
#define ascii_istitle(ch)    ascii_isupper(ch)
#define ascii_isxnumeric(ch) false
#define ascii_issymstrt(ch)  (ascii_isupper(ch) || ascii_islower(ch) || (ch) == 0x5f || (ch) == 0x24)
#define ascii_issymcont(ch)  (ascii_isupper(ch) || ascii_islower(ch) || ascii_isdigit(ch) || (ch) == 0x5f || (ch) == 0x24)

import string from deemon;
for (local i: [:256]) {
	local s = string.chr(i);
	local flags = [];
	if (ascii_iscntrl(i))
		flags.append("TCNTRL");
	if (ascii_isctab(i))
		flags.append("TCTAB");
	if (ascii_isxtab(i))
		flags.append("TXTAB");
	if (ascii_iswhite(i))
		flags.append("TWHITE");
	if (ascii_islf(i))
		flags.append("TLF");
	if (ascii_islower(i))
		flags.append("TLOWER");
	if (ascii_isupper(i))
		flags.append("TUPPER");
	if (ascii_isxalpha(i))
		flags.append("TXALPHA");
	if (ascii_isdigit(i))
		flags.append("TDIGIT");
	if (ascii_ishex(i))
		flags.append("THEX");
	if (ascii_ispunct(i))
		flags.append("TPUNCT");
	if (ascii_istitle(i))
		flags.append("TTITLE");
	if (ascii_isxnumeric(i))
		flags.append("TXNUMERIC");
	if (ascii_issymstrt(i))
		flags.append("TSYMSTRT");
	if (ascii_issymcont(i))
		flags.append("TSYMCONT");
	print("\t", flags ? " | ".join(flags) : "0", ",");
}
]]]*/
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL | TCTAB,
	TCNTRL | TLF,
	TCNTRL | TXTAB,
	TCNTRL | TXTAB,
	TCNTRL | TLF,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TCNTRL,
	TWHITE,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT | TSYMSTRT | TSYMCONT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TDIGIT | TSYMCONT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TUPPER | THEX | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | THEX | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | THEX | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | THEX | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | THEX | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | THEX | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TUPPER | TTITLE | TSYMSTRT | TSYMCONT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT | TSYMSTRT | TSYMCONT,
	TPUNCT,
	TLOWER | THEX | TSYMSTRT | TSYMCONT,
	TLOWER | THEX | TSYMSTRT | TSYMCONT,
	TLOWER | THEX | TSYMSTRT | TSYMCONT,
	TLOWER | THEX | TSYMSTRT | TSYMCONT,
	TLOWER | THEX | TSYMSTRT | TSYMCONT,
	TLOWER | THEX | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TLOWER | TSYMSTRT | TSYMCONT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TPUNCT,
	TCNTRL,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
/*[[[end]]]*/
};

/* clang-format off */
PUBLIC_CONST uint8_t const _DeeAscii_HexValue[256] = {
	/* ASCII */
#define FF 0xff
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x00-0x0f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x10-0x1f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x20-0x2f */
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, FF,FF,FF,FF,FF,FF, /* 0x30-0x3f */
	FF,10,11,12,13,14,15,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x40-0x4f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x50-0x5f */
	FF,10,11,12,13,14,15,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x60-0x6f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x70-0x7f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x80-0x8f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0x90-0x9f */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0xa0-0xaf */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0xb0-0xbf */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0xc0-0xcf */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0xd0-0xdf */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF, /* 0xe0-0xef */
	FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF  /* 0xf0-0xff */
#undef FF
};
/* clang-format on */

#endif /* !CONFIG_HAVE_UNICODE_H */

PUBLIC_CONST char const _DeeAscii_Itoa[101] =
"0123456789abcdefghijklmnopqrstuvwxyz\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_DB_C */
