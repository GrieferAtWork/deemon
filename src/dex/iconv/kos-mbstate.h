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
#ifndef GUARD_DEX_ICONV_KOS_MBSTATE_H
#define GUARD_DEX_ICONV_KOS_MBSTATE_H 1

#include <deemon/api.h>
/**/

#include <hybrid/typecore.h>
#include <deemon/util/kos-compat.h>

DECL_BEGIN

struct libiconv_mbstate {
	__UINT32_TYPE__ mb_word;
};

#define LIBICONV_MBSTATE_INIT { 0 }
#define libiconv_mbstate_init(x)    (void)((x)->mb_word = 0)
#define libiconv_mbstate_cinit(x)   (void)Dee_ASSERT((x)->mb_word == 0)
#define libiconv_mbstate_isempty(x) ((x)->mb_word == 0)

#define LIBICONV_MBSTATE_TYPE_MASK     0xfc000000
#define LIBICONV_MBSTATE_TYPE_EMPTY    0x00000000
#define LIBICONV_MBSTATE_TYPE_UTF8_SHIFT 6 /* Number of missing least significant bits provided by the last character of a utf-8 sequence. */
#define LIBICONV_MBSTATE_TYPE_UTF8_2_2 0x04000000 /* expect 2nd character of a 2-byte utf-8 sequence. { WORD & 0x0000001f } */
#define LIBICONV_MBSTATE_TYPE_UTF8_3_2 0x08000000 /* expect 2nd character of a 3-byte utf-8 sequence. { WORD & 0x000003c0 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_3_3 0x0c000000 /* expect 3rd character of a 3-byte utf-8 sequence. { WORD & 0x000003c0, WORD & 0x0000003f } */
#define LIBICONV_MBSTATE_TYPE_UTF8_4_2 0x10000000 /* expect 2nd character of a 4-byte utf-8 sequence. { WORD & 0x00007000 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_4_3 0x14000000 /* expect 3rd character of a 4-byte utf-8 sequence. { WORD & 0x00007000, WORD & 0x00000fc0 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_4_4 0x18000000 /* expect 4th character of a 4-byte utf-8 sequence. { WORD & 0x00007000, WORD & 0x00000fc0, WORD & 0x0000003f } */
#define LIBICONV_MBSTATE_TYPE_UTF8_5_2 0x1c000000 /* expect 2nd character of a 5-byte utf-8 sequence. { WORD & 0x000c0000 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_5_3 0x20000000 /* expect 3rd character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_5_4 0x24000000 /* expect 4th character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000, WORD & 0x00000fc0 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_5_5 0x28000000 /* expect 5th character of a 5-byte utf-8 sequence. { WORD & 0x000c0000, WORD & 0x0003f000, WORD & 0x00000fc0, WORD & 0x0000003f } */
#define LIBICONV_MBSTATE_TYPE_UTF8_6_2 0x2c000000 /* expect 2nd character of a 6-byte utf-8 sequence. { WORD & 0x01000000 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_6_3 0x30000000 /* expect 3rd character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_6_4 0x34000000 /* expect 4th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_6_5 0x38000000 /* expect 5th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000, WORD & 0x00000fc0 } */
#define LIBICONV_MBSTATE_TYPE_UTF8_6_6 0x3c000000 /* expect 6th character of a 6-byte utf-8 sequence. { WORD & 0x01000000, WORD & 0x00fc0000, WORD & 0x0003f000, WORD & 0x00000fc0, WORD & 0x0000003f } */
/* NOTE: mbstate is too small to support 7-character and 8-character utf-8 sequences.
 *       But that's OK, since officially UTF-8 only goes up to 4-character sequences,
 *       so  even just supporting 5-character and 6-character sequences is already an
 *       extension! */
#define LIBICONV_MBSTATE_TYPE_UTF16_LO    0x40000000 /* expect the low-surrogate value of a 2-uint16_t utf-16 sequence. `RESULT_CHAR = ((WORD & 0x000003ff) << 10) + 0x10000 + (SECOND_U16 - 0xdc00);' */
#define LIBICONV_MBSTATE_TYPE_WR_UTF16_LO 0x44000000 /* Write the low-surrogate value of a 2-uint16_t utf-16 sequence. `OUT_CHAR = 0xdc00 + (WORD & 0x000003ff)' */

/* Reserved for future expansion: there's still a lot of space left for more type codes! */
/*      LIBICONV_MBSTATE_TYPE_            0x48000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0x4c000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0x50000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0x54000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0x58000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0x5c000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            ...         * ... */
/*      LIBICONV_MBSTATE_TYPE_            0xf0000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0xf4000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0xf8000000  * ... */
/*      LIBICONV_MBSTATE_TYPE_            0xfc000000  * ... */

PRIVATE ATTR_INOUT(4) ATTR_INS(2, 3) ATTR_OUT(1) size_t
NOTHROW_NCX(DCALL libiconv_unicode_c8toc16)(char16_t *__restrict pc16,
                                            char const *__restrict s, size_t n,
                                            struct libiconv_mbstate *__restrict mbs);

PRIVATE ATTR_INOUT(4) ATTR_INS(2, 3) ATTR_OUT(1) size_t
NOTHROW_NCX(DCALL libiconv_unicode_c8toc32)(char32_t *__restrict pc32,
                                            char const *__restrict s, size_t n,
                                            struct libiconv_mbstate *__restrict mbs);

PRIVATE ATTR_INOUT(3) ATTR_OUT(1) size_t
NOTHROW_NCX(DCALL libiconv_unicode_c16toc8)(char pc8[3], char16_t c16,
                                            struct libiconv_mbstate *__restrict mbs);

DECL_END

#endif /* !GUARD_DEX_ICONV_KOS_MBSTATE_H */
