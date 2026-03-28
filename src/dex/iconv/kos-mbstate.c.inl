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
#ifndef GUARD_DEX_ICONV_KOS_MBSTATE_C_INL
#define GUARD_DEX_ICONV_KOS_MBSTATE_C_INL 1

#include <deemon/api.h>
/**/

#include <deemon/util/kos-compat.h>

#include "kos-mbstate.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t, uint16_t, uint32_t */

#ifndef CONFIG_HAVE_UNICODE_H
DECL_BEGIN

PRIVATE ATTR_INOUT(4) ATTR_INS(2, 3) ATTR_OUT(1) size_t
NOTHROW_NCX(DCALL libiconv_unicode_c8toc16)(char16_t *__restrict pc16,
                                            char const *__restrict s, size_t n,
                                            struct libiconv_mbstate *__restrict mbs) {
	char32_t resch;
	size_t i;
	if ((mbs->mb_word & LIBICONV_MBSTATE_TYPE_MASK) == LIBICONV_MBSTATE_TYPE_WR_UTF16_LO) {
		*pc16 = 0xdc00 + (mbs->mb_word & 0x3ff);
		mbs->mb_word = LIBICONV_MBSTATE_TYPE_EMPTY;
		return 0;
	}
	for (i = 0; i < n; ++i) {
		uint32_t state;
		uint8_t ch;
		state = mbs->mb_word & LIBICONV_MBSTATE_TYPE_MASK;
		ch = (uint8_t)s[i];
		if (state == LIBICONV_MBSTATE_TYPE_EMPTY) {
			if (ch <= 0x7f) {
				*pc16 = ch;
				goto done;
			} else if (ch <= 0xbf) { /* NOLINT */
				goto error_ilseq;
			} else if (ch <= 0xdf) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_2_2 | (ch & 0x1f);
				continue;
			} else if (ch <= 0xef) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_3_2 | ((ch & 0xf) << 6);
				continue;
			} else if (ch <= 0xf7) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_4_2 | ((ch & 0x7) << 12);
				continue;
			} else if (ch <= 0xfb) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_2 | ((ch & 0x3) << 18);
				continue;
			} else if (ch <= 0xfd) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_2 | ((ch & 0x1) << 24);
				continue;
			} else {
				goto error_ilseq;
			}
		}
		if ((ch & 0xc0) != 0x80)
			goto error_ilseq; /* Must be a follow-up byte */
		ch &= 0x3f;
		switch (mbs->mb_word & LIBICONV_MBSTATE_TYPE_MASK) {

		case LIBICONV_MBSTATE_TYPE_UTF8_2_2: /* expect 2nd character of a 2-byte utf-8 sequence. { WORD & 0x0000001f } */
			*pc16 = ((mbs->mb_word & 0x1f) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty;

		case LIBICONV_MBSTATE_TYPE_UTF8_3_2: /* expect 2nd character of a 3-byte utf-8 sequence. { WORD & 0x000003c0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_3_3 | (mbs->mb_word & 0x3c0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_3_3: /* expect 3rd character of a 3-byte utf-8 sequence. { WORD & 0x000003c0, WORD & 0x0000003f } */
			resch = ((mbs->mb_word & 0x3ff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty_chk_surrogate;

		case LIBICONV_MBSTATE_TYPE_UTF8_4_2: /* expect 2nd character of a 4-byte utf-8 sequence. { WORD & 0x00007000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_4_3 | (mbs->mb_word & 0x7000) | ((uint32_t)ch << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT);
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_4_3: /* expect 3rd character of a 4-byte utf-8 sequence. { WORD & 0x00007000, WORD & 0x00000fc0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_4_4 | (mbs->mb_word & 0x7fc0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_4_4: /* expect 4th character of a 4-byte utf-8 sequence. { WORD & 0x00007000, WORD & 0x00000fc0, WORD & 0x0000003f } */
			resch = ((mbs->mb_word & 0x7fff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty_chk_surrogate;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_2: /* expect 2nd character of a 5-byte utf-8 sequence. { WORD & 0x000c0000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_3 | (mbs->mb_word & 0xc0000) | ((uint32_t)ch << (2 * LIBICONV_MBSTATE_TYPE_UTF8_SHIFT));
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_3: /* expect 3rd character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_4 | (mbs->mb_word & 0xff000) | ((uint32_t)ch << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT);
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_4: /* expect 4th character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000, WORD & 0x00000fc0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_5 | (mbs->mb_word & 0xfffc0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_5: /* expect 5th character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000, WORD & 0x00000fc0, WORD & 0x0000003f } */
			resch = ((mbs->mb_word & 0x000cffff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty_chk_surrogate;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_2: /* expect 2nd character of a 6-byte utf-8 sequence. { WORD & 0x01000000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_3 | (mbs->mb_word & 0x1000000) | ((uint32_t)ch << (3 * LIBICONV_MBSTATE_TYPE_UTF8_SHIFT));
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_3: /* expect 3rd character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_4 | (mbs->mb_word & 0x1fc0000) | ((uint32_t)ch << (2 * LIBICONV_MBSTATE_TYPE_UTF8_SHIFT));
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_4: /* expect 4th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_5 | (mbs->mb_word & 0x1fff000) | ((uint32_t)ch << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT);
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_5: /* expect 5th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000, WORD & 0x00000fc0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_6 | (mbs->mb_word & 0x1ffffc0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_6: /* expect 6th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000, WORD & 0x00000fc0, WORD & 0x0000003f } */
			resch = ((mbs->mb_word & 0x1ffffff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty_chk_surrogate;

		default:
error_ilseq:
			return (size_t)-1;
		}
	}
	/* Incomplete sequence (but `mbs' may have been updated) */
	return (size_t)-2;
done_empty_chk_surrogate:
	if ((resch >= 0xd800 && resch <= 0xdfff) || (resch >= 0x10000)) {
		if unlikely(resch > 0x10ffff)
			goto error_ilseq; /* Cannot be represented as UTF-16 */
		/* Need a utf-16 surrogate pair. */
		resch -= 0x10000;
		*pc16 = 0xd800 + (char16_t)(resch >> 10);
		mbs->mb_word = LIBICONV_MBSTATE_TYPE_WR_UTF16_LO | (uint16_t)(resch & 0x3ff);
	} else {
		*pc16 = (char16_t)resch;
	}
done_empty:
	mbs->mb_word = LIBICONV_MBSTATE_TYPE_EMPTY;
done:
	return i + 1;
}


PRIVATE ATTR_INOUT(4) ATTR_INS(2, 3) ATTR_OUT(1) size_t
NOTHROW_NCX(DCALL libiconv_unicode_c8toc32)(char32_t *__restrict pc32,
                                            char const *__restrict s, size_t n,
                                            struct libiconv_mbstate *__restrict mbs) {
	size_t i;
	for (i = 0; i < n; ++i) {
		uint32_t state;
		uint8_t ch;
		state = mbs->mb_word & LIBICONV_MBSTATE_TYPE_MASK;
		ch = (uint8_t)s[i];
		if (state == LIBICONV_MBSTATE_TYPE_EMPTY) {
			if (ch <= 0x7f) {
				*pc32 = ch;
				goto done;
			} else if (ch <= 0xbf) { /* NOLINT */
				goto error_ilseq;
			} else if (ch <= 0xdf) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_2_2 | (ch & 0x1f);
				continue;
			} else if (ch <= 0xef) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_3_2 | ((ch & 0xf) << 6);
				continue;
			} else if (ch <= 0xf7) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_4_2 | ((ch & 0x7) << 12);
				continue;
			} else if (ch <= 0xfb) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_2 | ((ch & 0x3) << 18);
				continue;
			} else if (ch <= 0xfd) {
				mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_2 | ((ch & 0x1) << 24);
				continue;
			} else {
				goto error_ilseq;
			}
		}
		if ((ch & 0xc0) != 0x80)
			goto error_ilseq; /* Must be a follow-up byte */
		ch &= 0x3f;
		switch (mbs->mb_word & LIBICONV_MBSTATE_TYPE_MASK) {

		case LIBICONV_MBSTATE_TYPE_UTF8_2_2: /* expect 2nd character of a 2-byte utf-8 sequence. { WORD & 0x0000001f } */
			*pc32 = ((mbs->mb_word & 0x1f) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty;

		case LIBICONV_MBSTATE_TYPE_UTF8_3_2: /* expect 2nd character of a 3-byte utf-8 sequence. { WORD & 0x000003c0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_3_3 | (mbs->mb_word & 0x3c0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_3_3: /* expect 3rd character of a 3-byte utf-8 sequence. { WORD & 0x000003c0, WORD & 0x0000003f } */
			*pc32 = ((mbs->mb_word & 0x3ff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty;

		case LIBICONV_MBSTATE_TYPE_UTF8_4_2: /* expect 2nd character of a 4-byte utf-8 sequence. { WORD & 0x00007000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_4_3 | (mbs->mb_word & 0x7000) | ((uint32_t)ch << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT);
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_4_3: /* expect 3rd character of a 4-byte utf-8 sequence. { WORD & 0x00007000, WORD & 0x00000fc0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_4_4 | (mbs->mb_word & 0x7fc0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_4_4: /* expect 4th character of a 4-byte utf-8 sequence. { WORD & 0x00007000, WORD & 0x00000fc0, WORD & 0x0000003f } */
			*pc32 = ((mbs->mb_word & 0x7fff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_2: /* expect 2nd character of a 5-byte utf-8 sequence. { WORD & 0x000c0000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_3 | (mbs->mb_word & 0xc0000) | ((uint32_t)ch << (2 * LIBICONV_MBSTATE_TYPE_UTF8_SHIFT));
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_3: /* expect 3rd character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_4 | (mbs->mb_word & 0xff000) | ((uint32_t)ch << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT);
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_4: /* expect 4th character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000, WORD & 0x00000fc0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_5_5 | (mbs->mb_word & 0xfffc0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_5_5: /* expect 5th character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000, WORD & 0x00000fc0, WORD & 0x0000003f } */
			*pc32 = ((mbs->mb_word & 0x000cffff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_2: /* expect 2nd character of a 6-byte utf-8 sequence. { WORD & 0x01000000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_3 | (mbs->mb_word & 0x1000000) | ((uint32_t)ch << (3 * LIBICONV_MBSTATE_TYPE_UTF8_SHIFT));
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_3: /* expect 3rd character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_4 | (mbs->mb_word & 0x1fc0000) | ((uint32_t)ch << (2 * LIBICONV_MBSTATE_TYPE_UTF8_SHIFT));
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_4: /* expect 4th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_5 | (mbs->mb_word & 0x1fff000) | ((uint32_t)ch << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT);
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_5: /* expect 5th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000, WORD & 0x00000fc0 } */
			mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF8_6_6 | (mbs->mb_word & 0x1ffffc0) | ch;
			break;

		case LIBICONV_MBSTATE_TYPE_UTF8_6_6: /* expect 6th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000, WORD & 0x00000fc0, WORD & 0x0000003f } */
			*pc32 = ((mbs->mb_word & 0x1ffffff) << LIBICONV_MBSTATE_TYPE_UTF8_SHIFT) | ch;
			goto done_empty;

		default:
error_ilseq:
			return (size_t)-1;
		}
	}
	/* Incomplete sequence (but `mbs' may have been updated) */
	return (size_t)-2;
done_empty:
	mbs->mb_word = LIBICONV_MBSTATE_TYPE_EMPTY;
done:
	return i + 1;
}

PRIVATE ATTR_INOUT(3) ATTR_OUT(1) size_t
NOTHROW_NCX(DCALL libiconv_unicode_c16toc8)(char pc8[3], char16_t c16,
                                            struct libiconv_mbstate *__restrict mbs) {
	char32_t ch32;
	if ((mbs->mb_word & LIBICONV_MBSTATE_TYPE_MASK) == LIBICONV_MBSTATE_TYPE_UTF16_LO) {
		if unlikely(!(c16 >= 0xdc00 &&
		              c16 <= 0xdfff))
			return (size_t)-1;
		ch32 = ((mbs->mb_word & 0x000003ff) << 10) + 0x10000 + ((uint16_t)c16 - 0xdc00);
		mbs->mb_word = LIBICONV_MBSTATE_TYPE_EMPTY;
	} else if (c16 >= 0xd800 && c16 <= 0xdbff) {
		mbs->mb_word = LIBICONV_MBSTATE_TYPE_UTF16_LO | ((uint16_t)c16 - 0xd800);
		return 0;
	} else {
		ch32 = (char32_t)c16;
	}
	if likely(ch32 <= ((uint32_t)1 << 7)-1) {
		pc8[0] = (char)(uint8_t)ch32;
		return 1;
	}
	if (ch32 <= ((uint32_t)1 << 11)-1) {
		pc8[0] = (char)(0xc0 | (uint8_t)((ch32 >> 6)/* & 0x1f*/));
		pc8[1] = (char)(0x80 | (uint8_t)((ch32) & 0x3f));
		return 2;
	}
	pc8[0] = (char)(0xe0 | (uint8_t)((ch32 >> 12)/* & 0x0f*/));
	pc8[1] = (char)(0x80 | (uint8_t)((ch32 >> 6) & 0x3f));
	pc8[2] = (char)(0x80 | (uint8_t)((ch32) & 0x3f));
	return 3;
}

DECL_END
#endif /* !CONFIG_HAVE_UNICODE_H */

#endif /* !GUARD_DEX_ICONV_KOS_MBSTATE_C_INL */
