/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C
#define GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>
/**/

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t, offsetof */
#include <stdint.h>  /* uint32_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifndef CONFIG_HAVE_memsetw
#define CONFIG_HAVE_memsetw
#undef memsetw
#define memsetw dee_memsetw
DeeSystem_DEFINE_memsetw(dee_memsetw)
#endif /* !CONFIG_HAVE_memsetw */

#ifndef CONFIG_HAVE_memsetl
#define CONFIG_HAVE_memsetl
#undef memsetl
#define memsetl dee_memsetl
DeeSystem_DEFINE_memsetl(dee_memsetl)
#endif /* !CONFIG_HAVE_memsetl */


#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0


STATIC_ASSERT_MSG(sizeof(char) == sizeof(uint8_t), "Probably won't work...");
STATIC_ASSERT(STRING_SIZEOF_WIDTH(STRING_WIDTH_1BYTE) == 1);
STATIC_ASSERT(STRING_SIZEOF_WIDTH(STRING_WIDTH_2BYTE) == 2);
STATIC_ASSERT(STRING_SIZEOF_WIDTH(STRING_WIDTH_4BYTE) == 4);

/* clang-format off */
#define UTF8_SEQLEN_INIT(_0, _1, _2, _3, _4, _5, _6, _7, _8)             \
	{                                                                    \
		/* ASCII */                                                      \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x00-0x0f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x10-0x1f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x20-0x2f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x30-0x3f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x40-0x4f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x50-0x5f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x60-0x6f */ \
		_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1,_1, /* 0x70-0x7f */ \
		/* Unicode follow-up word (`0b10??????'). */                     \
		_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0, /* 0x80-0x8f */ \
		_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0, /* 0x90-0x9f */ \
		_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0, /* 0xa0-0xaf */ \
		_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0,_0, /* 0xb0-0xbf */ \
		/* `0b110?????' */                                               \
		_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2, /* 0xc0-0xcf */ \
		_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2,_2, /* 0xd0-0xdf */ \
		/* `0b1110????' */                                               \
		_3,_3,_3,_3,_3,_3,_3,_3,_3,_3,_3,_3,_3,_3,_3,_3, /* 0xe0-0xef */ \
		/* `0b11110???' */                                               \
		_4,_4,_4,_4,_4,_4,_4,_4,                                         \
		_5,_5,_5,_5,                                                     \
		_6,_6,                                                           \
		_7,                                                              \
		_8                                                               \
	}
/* clang-format on */
PUBLIC_CONST uint8_t const Dee_unicode_utf8seqlen[256] =
UTF8_SEQLEN_INIT(0, 1, 2, 3, 4, 5, 6, 7, 8);
PUBLIC_CONST uint8_t const Dee_unicode_utf8seqlen_safe[256] =
UTF8_SEQLEN_INIT(1, 1, 2, 3, 4, 5, 6, 7, 8);
#undef UTF8_SEQLEN_INIT


/* (Theoretical) utf-8 unicode sequence ranges:
 *  - 1-byte    -- 7               = 7 bits
 *  - 2-byte    -- 5+6             = 11 bits
 *  - 3-byte    -- 4+6+6           = 16 bits
 *  - 4-byte    -- 3+6+6+6         = 21 bits
 *  - 5-byte    -- 2+6+6+6+6       = 26 bits (Not valid unicode characters)
 *  - 6-byte    -- 1+6+6+6+6+6     = 31 bits (Not valid unicode characters)
 *  - 7-byte    --   6+6+6+6+6+6   = 36 bits (Not valid unicode characters)
 *  - 8-byte    --   6+6+6+6+6+6+6 = 42 bits (Not valid unicode characters)
 */
#define UTF8_1BYTE_MAX    (((uint32_t)1 << 7) - 1)
#define UTF8_2BYTE_MAX    (((uint32_t)1 << 11) - 1)
#define UTF8_3BYTE_MAX    (((uint32_t)1 << 16) - 1)
#define UTF8_4BYTE_MAX    (((uint32_t)1 << 21) - 1)
#define UTF8_5BYTE_MAX    (((uint32_t)1 << 26) - 1)
#define UTF8_6BYTE_MAX    (((uint32_t)1 << 31) - 1)


#define UTF16_HIGH_SURROGATE_MIN 0xd800
#define UTF16_HIGH_SURROGATE_MAX 0xdbff
#define UTF16_LOW_SURROGATE_MIN  0xdc00
#define UTF16_LOW_SURROGATE_MAX  0xdfff
#define UTF16_SURROGATE_SHIFT    0x10000

#define UTF16_COMBINE_SURROGATES(high, low)               \
	((((uint32_t)(high)-UTF16_HIGH_SURROGATE_MIN) << 10 | \
	  ((uint32_t)(low)-UTF16_LOW_SURROGATE_MIN)) +        \
	 UTF16_SURROGATE_SHIFT)





/* String functions related to unicode. */
typedef DeeStringObject String;

#ifdef CONFIG_STRING_LATIN1_CACHED
/* Preallocated latin1 character table.
 * Elements are lazily allocated. */
PRIVATE DREF String *latin1_chars[256] = {
	NULL,
};

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t latin1_chars_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define latin1_chars_lock_reading()    Dee_atomic_rwlock_reading(&latin1_chars_lock)
#define latin1_chars_lock_writing()    Dee_atomic_rwlock_writing(&latin1_chars_lock)
#define latin1_chars_lock_tryread()    Dee_atomic_rwlock_tryread(&latin1_chars_lock)
#define latin1_chars_lock_trywrite()   Dee_atomic_rwlock_trywrite(&latin1_chars_lock)
#define latin1_chars_lock_canread()    Dee_atomic_rwlock_canread(&latin1_chars_lock)
#define latin1_chars_lock_canwrite()   Dee_atomic_rwlock_canwrite(&latin1_chars_lock)
#define latin1_chars_lock_waitread()   Dee_atomic_rwlock_waitread(&latin1_chars_lock)
#define latin1_chars_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&latin1_chars_lock)
#define latin1_chars_lock_read()       Dee_atomic_rwlock_read(&latin1_chars_lock)
#define latin1_chars_lock_write()      Dee_atomic_rwlock_write(&latin1_chars_lock)
#define latin1_chars_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&latin1_chars_lock)
#define latin1_chars_lock_upgrade()    Dee_atomic_rwlock_upgrade(&latin1_chars_lock)
#define latin1_chars_lock_downgrade()  Dee_atomic_rwlock_downgrade(&latin1_chars_lock)
#define latin1_chars_lock_endwrite()   Dee_atomic_rwlock_endwrite(&latin1_chars_lock)
#define latin1_chars_lock_endread()    Dee_atomic_rwlock_endread(&latin1_chars_lock)
#define latin1_chars_lock_end()        Dee_atomic_rwlock_end(&latin1_chars_lock)

INTERN size_t DCALL
Dee_latincache_clearall(size_t max_clear) {
	size_t result      = 0;
	DREF String **iter = latin1_chars;
#ifndef CONFIG_NO_THREADS
again:
#endif /* !CONFIG_NO_THREADS */
	latin1_chars_lock_write();
	for (; iter < COMPILER_ENDOF(latin1_chars); ++iter) {
		DREF String *ob = *iter;
		if (!ob)
			continue;
		*iter = NULL;
		if (!Dee_DecrefIfNotOne(ob)) {
			latin1_chars_lock_endwrite();
			result += offsetof(String, s_str) + 2 * sizeof(char);
			Dee_Decref(ob);
			if (result >= max_clear)
				goto done;
#ifndef CONFIG_NO_THREADS
			goto again;
#endif /* !CONFIG_NO_THREADS */
		}
	}
	latin1_chars_lock_endwrite();
done:
	return result;
}
#endif /* CONFIG_STRING_LATIN1_CACHED */


#if defined(CONFIG_STRING_LATIN1_STATIC) || defined(__DEEMON__)
/*[[[deemon
import * from deemon;
import _Dee_HashSelect from rt.gen.hash;
print("INTERN DeeStringObject1Char DeeString_Latin1[256] = {");
for (local ord: [:256]) {
	local s = string.chr(ord);
	print("	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, ",
		_Dee_HashSelect(s), ", 1, { ", ord, ", 0 } }, /" "* ",
		s.isprint() ? f'"{s}"' : repr s, " *" "/");
}
print("};");
]]]*/
INTERN DeeStringObject1Char DeeString_Latin1[256] = {
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x514e28b7, 0x0), 1, { 0, 0 } }, /* "\0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe45ad1ab, 0x1ab11ea5a7b2c56e), 1, { 1, 0 } }, /* "\1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6c27d09f, 0x8488f4ef228ee909), 1, { 2, 0 } }, /* "\2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5e2a8076, 0x77116fc6d1ba1b16), 1, { 3, 0 } }, /* "\3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3961a4aa, 0x1edd4915741b947f), 1, { 4, 0 } }, /* "\4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x53cac555, 0xe6804c9a9bea3393), 1, { 5, 0 } }, /* "\5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xa9fc1939, 0xb4c78720ff46ba72), 1, { 6, 0 } }, /* "\6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6882f382, 0x388fa52c5d207365), 1, { 7, 0 } }, /* "\a" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2036bcc7, 0x3dba922ae83728ff), 1, { 8, 0 } }, /* "\b" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe99b2213, 0x510da8cb03a6ba5d), 1, { 9, 0 } }, /* "\t" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x924ee0ab, 0xe2cbf86c66d2232c), 1, { 10, 0 } }, /* "\n" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x4d79446c, 0xbe0d4913f44279cf), 1, { 11, 0 } }, /* "\v" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xace1a3cd, 0x7f5a6d792d8b6f3b), 1, { 12, 0 } }, /* "\f" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe1221a99, 0x1d2b0ea6ca90151e), 1, { 13, 0 } }, /* "\r" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xeb5763a1, 0x711f4a58ba40e6ca), 1, { 14, 0 } }, /* "\16" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x148ab378, 0x95e9424b074a2862), 1, { 15, 0 } }, /* "\17" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x228c7277, 0x7b752455d06e51fe), 1, { 16, 0 } }, /* "\20" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x289dca8c, 0xa6a120f3f2fe083), 1, { 17, 0 } }, /* "\21" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x85ddaad1, 0xb7e6b0cd364975ec), 1, { 18, 0 } }, /* "\22" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbc4e9726, 0x3e64fb3c012e2e22), 1, { 19, 0 } }, /* "\23" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbd1e70, 0x14bea87ca0d1e40b), 1, { 20, 0 } }, /* "\24" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x153ee7b7, 0x3162d58d98387ee), 1, { 21, 0 } }, /* "\25" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcd9eeecc, 0xcb4149cbbbb19d28), 1, { 22, 0 } }, /* "\26" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd5d81e52, 0xe6025a629709909d), 1, { 23, 0 } }, /* "\27" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe2f3801d, 0xc5598285b6e88600), 1, { 24, 0 } }, /* "\30" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5b033bf7, 0xa775472e47f202e3), 1, { 25, 0 } }, /* "\31" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6224ef5, 0x3a561d4d95202a3c), 1, { 26, 0 } }, /* "\32" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6af79834, 0xdbcab62bf0ad3169), 1, { 27, 0 } }, /* "\33" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xdd16e3e5, 0x9317dd0da1572fc7), 1, { 28, 0 } }, /* "\34" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe563905c, 0xa7e39b0dd61f3bdb), 1, { 29, 0 } }, /* "\35" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6355b1eb, 0xa3507485973cb655), 1, { 30, 0 } }, /* "\36" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xaba984a3, 0x197f29829dca1739), 1, { 31, 0 } }, /* "\37" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x7ef49b98, 0xbd8ef03efcae4ca0), 1, { 32, 0 } }, /* " " */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x72661cf4, 0x75614ceedfdeba7c), 1, { 33, 0 } }, /* "!" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xfc81fcb3, 0x14d4241e7e5fc106), 1, { 34, 0 } }, /* """ */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x44a5674f, 0xf0e41d188dc355aa), 1, { 35, 0 } }, /* "#" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1ae0d785, 0xe74b5189f539d341), 1, { 36, 0 } }, /* "$" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x17ba2eff, 0x378920ee36c6b0f7), 1, { 37, 0 } }, /* "%" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x24e7e891, 0xf447e6678b0567e4), 1, { 38, 0 } }, /* "&" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x9b6ea882, 0x9c5e5395e07b87c4), 1, { 39, 0 } }, /* "'" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1ddef3ed, 0xda5699556e782e41), 1, { 40, 0 } }, /* "(" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe5f76205, 0x2cbea844beffb9), 1, { 41, 0 } }, /* ")" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x14336e1c, 0x62c5ab1b3070fdc), 1, { 42, 0 } }, /* "*" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6498613c, 0x54079be8faed5cac), 1, { 43, 0 } }, /* "+" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcbae28fd, 0x5d273b2ad332baa7), 1, { 44, 0 } }, /* "," */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf945531f, 0x5b757171935fd125), 1, { 45, 0 } }, /* "-" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xaf56fc23, 0x1b2b6c69013a8f64), 1, { 46, 0 } }, /* "." */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x4a6b5e4f, 0x27081fa8b9e13cd6), 1, { 47, 0 } }, /* "/" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd271c07f, 0xd9d9bcaf40fdaad5), 1, { 48, 0 } }, /* "0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x9416ac93, 0x447dd899fc2bb5ea), 1, { 49, 0 } }, /* "1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x129e217, 0x158f35efebb8aab9), 1, { 50, 0 } }, /* "2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xfc7a1b4, 0x2af4a098b85f5930), 1, { 51, 0 } }, /* "3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe131cc88, 0x74ac3a9b2a405478), 1, { 52, 0 } }, /* "4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x531a35e4, 0xe593808f92cf9223), 1, { 53, 0 } }, /* "5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x27fa7cc0, 0x6bc23fbb4830080), 1, { 54, 0 } }, /* "6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x23ea8628, 0xe12a6762089fc72b), 1, { 55, 0 } }, /* "7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbd920017, 0x3bfb195271a78a93), 1, { 56, 0 } }, /* "8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x248be6a1, 0xd414ee5858b98bc2), 1, { 57, 0 } }, /* "9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2b266896, 0x65929552db381ae8), 1, { 58, 0 } }, /* ":" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xb4157375, 0x85ab26de0286c0cc), 1, { 59, 0 } }, /* ";" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x164d8194, 0x46a0e90b2e7b6cab), 1, { 60, 0 } }, /* "<" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf64d19e, 0xcdfe2e317adc04d2), 1, { 61, 0 } }, /* "=" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1c62ca5d, 0xf9a2fa989767c666), 1, { 62, 0 } }, /* ">" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x96615806, 0x31baa51535bd1fc0), 1, { 63, 0 } }, /* "?" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5645e867, 0x90e93fb5285b78dd), 1, { 64, 0 } }, /* "@" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x54dcf7ce, 0x4f38a775a0938899), 1, { 65, 0 } }, /* "A" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcca66a8a, 0x39e9518192e6d2ae), 1, { 66, 0 } }, /* "B" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xea1cfe8b, 0xdc7a29ac25a4f8e4), 1, { 67, 0 } }, /* "C" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x63c05906, 0xf04cefd058905a68), 1, { 68, 0 } }, /* "D" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf783207, 0x7acfbe7929f79558), 1, { 69, 0 } }, /* "E" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x982b1106, 0xa86ce1c477560280), 1, { 70, 0 } }, /* "F" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x7d180799, 0xd96fd6c55e8464cc), 1, { 71, 0 } }, /* "G" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x407f6809, 0x7f6feb701746c571), 1, { 72, 0 } }, /* "H" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5491dfca, 0x866e259a1cc5b17e), 1, { 73, 0 } }, /* "I" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x61a789d7, 0x1feb8a389a63801d), 1, { 74, 0 } }, /* "J" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xefcf429a, 0x4df2c4d23641ec44), 1, { 75, 0 } }, /* "K" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf3397f3f, 0xaf34746271da5603), 1, { 76, 0 } }, /* "L" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x612876a, 0xde2ddf38169307fa), 1, { 77, 0 } }, /* "M" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe62e0bb1, 0x38bca72bc0f50f89), 1, { 78, 0 } }, /* "N" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x458da2be, 0x3769074a877921e6), 1, { 79, 0 } }, /* "O" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xae0fd607, 0xb4ad32aadcf25c82), 1, { 80, 0 } }, /* "P" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x15adb099, 0xb993f2f5e215fb44), 1, { 81, 0 } }, /* "Q" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x79b365c1, 0x77d76d40122506fb), 1, { 82, 0 } }, /* "R" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x839a1ad4, 0xeebe717239e1319b), 1, { 83, 0 } }, /* "S" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf54ab753, 0x83d6a552eeb4f730), 1, { 84, 0 } }, /* "T" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc1350d02, 0x311b654773e189f0), 1, { 85, 0 } }, /* "U" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x65540c7f, 0xa80f37d1f5dab958), 1, { 86, 0 } }, /* "V" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc2992670, 0x799c602b8bbb7c2a), 1, { 87, 0 } }, /* "W" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2814d70c, 0x80f31de90234ea81), 1, { 88, 0 } }, /* "X" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xdd29f651, 0x7f64bc1ecef60ccc), 1, { 89, 0 } }, /* "Y" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3dbac09d, 0x2e68d2d2af66e5e2), 1, { 90, 0 } }, /* "Z" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x615493c2, 0xe17e9c8cbc769ca5), 1, { 91, 0 } }, /* "[" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x93a70c06, 0x857d9075d59e789f), 1, { 92, 0 } }, /* "\" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6efded50, 0xd35e77f1b2e1543c), 1, { 93, 0 } }, /* "]" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x57852da, 0x14b4e6e4cf94e648), 1, { 94, 0 } }, /* "^" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x352aeb03, 0x2f9338337f06ffcd), 1, { 95, 0 } }, /* "_" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x276ebecf, 0x7a5820f1ddcae8d1), 1, { 96, 0 } }, /* "`" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3c2569b2, 0x4292cee227b9150a), 1, { 97, 0 } }, /* "a" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x95de7e03, 0x88fbb133f8576bd5), 1, { 98, 0 } }, /* "b" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe132d65f, 0x32c769ee5c5509b0), 1, { 99, 0 } }, /* "c" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x27191473, 0x7a452383aa9bf7c4), 1, { 100, 0 } }, /* "d" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x656c4367, 0xe1b161496b7ea7eb), 1, { 101, 0 } }, /* "e" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2b64883b, 0x6bb4a0689fbad42e), 1, { 102, 0 } }, /* "f" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf18ae416, 0x2bcce9ad79bc2688), 1, { 103, 0 } }, /* "g" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd482b2d3, 0xe95875365480a8f0), 1, { 104, 0 } }, /* "h" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x811a702b, 0x8eb22eda95766e98), 1, { 105, 0 } }, /* "i" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xca745a39, 0xcb27011f259d2446), 1, { 106, 0 } }, /* "j" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcfbda5d1, 0x8d6857fddd7f21cf), 1, { 107, 0 } }, /* "k" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1d5d6a2c, 0x2343a72e98024302), 1, { 108, 0 } }, /* "l" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5ae4385c, 0x7a0c10e3609ea04b), 1, { 109, 0 } }, /* "m" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc651d8ac, 0x117b8667e4662809), 1, { 110, 0 } }, /* "n" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x68348473, 0x2c5d5bc33920e200), 1, { 111, 0 } }, /* "o" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x986fdf9a, 0x77f632a4e34f1526), 1, { 112, 0 } }, /* "p" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xff8209e8, 0x5cafc91a9b37763), 1, { 113, 0 } }, /* "q" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5c9373f1, 0x6ece84440d42ecf6), 1, { 114, 0 } }, /* "r" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xff4acaf1, 0xbc2cde4bbfc9c10), 1, { 115, 0 } }, /* "s" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xca87df4d, 0x91c9d2391242aebc), 1, { 116, 0 } }, /* "t" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x62203ae0, 0xaa98f317e9e2aa49), 1, { 117, 0 } }, /* "u" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbdafcc55, 0xbc2f961831e4ef6b), 1, { 118, 0 } }, /* "v" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xff439d1f, 0x809cbdb5d52b2aa9), 1, { 119, 0 } }, /* "w" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3e9a9b1b, 0x4bfc205e59fa416), 1, { 120, 0 } }, /* "x" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x477d9216, 0x4f56b733a4dfc78a), 1, { 121, 0 } }, /* "y" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc1f69a17, 0x62a103f6518de2b3), 1, { 122, 0 } }, /* "z" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcdfb52b1, 0x40afdafb3939fa19), 1, { 123, 0 } }, /* "{" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2a69134b, 0xf345f5312ecd8ccd), 1, { 124, 0 } }, /* "|" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x8247636, 0x31c3104b236e0754), 1, { 125, 0 } }, /* "}" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x9905b33e, 0x144e92869850d1f7), 1, { 126, 0 } }, /* "~" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5589d599, 0x2b0457437427bf51), 1, { 127, 0 } }, /* "\177" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xfeb9e1d, 0x99506f59d95da913), 1, { 128, 0 } }, /* "\u0080" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x69e10770, 0x35754fc06e5e31a3), 1, { 129, 0 } }, /* "\u0081" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x81acfcbd, 0x9e714eeb41271132), 1, { 130, 0 } }, /* "\u0082" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x103e5780, 0xd840a7a52fa600ed), 1, { 131, 0 } }, /* "\u0083" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x753c42e5, 0x73d2a30325cda55d), 1, { 132, 0 } }, /* "\u0084" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xee176745, 0x4c4228591bf1f96f), 1, { 133, 0 } }, /* "\u0085" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xebd2555, 0x30724347d3f20951), 1, { 134, 0 } }, /* "\u0086" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x652d2fef, 0x5a866d67cc7f445b), 1, { 135, 0 } }, /* "\u0087" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf2e29f14, 0x917327fcddf85704), 1, { 136, 0 } }, /* "\u0088" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x70ac4330, 0x607b7b95d145cf08), 1, { 137, 0 } }, /* "\u0089" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x470f369f, 0x44c6349627184802), 1, { 138, 0 } }, /* "\u008A" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe28626df, 0x13ec69d40bf03caa), 1, { 139, 0 } }, /* "\u008B" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6379a1b0, 0x177e6b1c4a80a0bb), 1, { 140, 0 } }, /* "\u008C" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x36759e32, 0x880f902e83b2d8da), 1, { 141, 0 } }, /* "\u008D" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x522aa98b, 0x7984551e18dd66b3), 1, { 142, 0 } }, /* "\u008E" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2c57271f, 0x85e239b1188aeaac), 1, { 143, 0 } }, /* "\u008F" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x48ee4298, 0x4e068e8401b6eb51), 1, { 144, 0 } }, /* "\u0090" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x99f7e3cc, 0xe5442bf774cec3e8), 1, { 145, 0 } }, /* "\u0091" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xef3ea33b, 0x22a7aa6b68852196), 1, { 146, 0 } }, /* "\u0092" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x75186c94, 0x1537adcaa877718c), 1, { 147, 0 } }, /* "\u0093" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc13ecad5, 0x67bbc04909965de), 1, { 148, 0 } }, /* "\u0094" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x7c75d1f6, 0xa5b4b22d476e9f13), 1, { 149, 0 } }, /* "\u0095" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x86fbfa03, 0x9be589a46c83d889), 1, { 150, 0 } }, /* "\u0096" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe18c9c86, 0xa732b43c47c4bd0), 1, { 151, 0 } }, /* "\u0097" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcf719abc, 0xad8fa068b6e14bc9), 1, { 152, 0 } }, /* "\u0098" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x7835e217, 0x9410355b44d0ff81), 1, { 153, 0 } }, /* "\u0099" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbcdd0f8a, 0xd2271da75c20c499), 1, { 154, 0 } }, /* "\u009A" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xa5f5c62, 0x982a0d6272875c2d), 1, { 155, 0 } }, /* "\u009B" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xdbb98833, 0xc0a005fb55157ca0), 1, { 156, 0 } }, /* "\u009C" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1139eb4e, 0x435f9aad494ed59b), 1, { 157, 0 } }, /* "\u009D" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x108cde5e, 0x6ed20e950ef243cc), 1, { 158, 0 } }, /* "\u009E" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x8a879c2b, 0x4a857f80c1db1eb5), 1, { 159, 0 } }, /* "\u009F" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x18602035, 0x2fff0ce915b80abb), 1, { 160, 0 } }, /* "\u00A0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x9ec1b696, 0xbd938ea2c75056fc), 1, { 161, 0 } }, /* "\u00A1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3741c033, 0x24012e47f0ff58c4), 1, { 162, 0 } }, /* "\u00A2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xa79b736b, 0x7b81264a4305b758), 1, { 163, 0 } }, /* "\u00A3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xb928b548, 0xa08822dc512093ba), 1, { 164, 0 } }, /* "\u00A4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x21aa1d86, 0x89ec774f1fc3efa5), 1, { 165, 0 } }, /* "\u00A5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x65300466, 0xa4218a77cf928b20), 1, { 166, 0 } }, /* "\u00A6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd50a5f83, 0x378c5bc7d28932ed), 1, { 167, 0 } }, /* "\u00A7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd93dec88, 0xce51f239393c566c), 1, { 168, 0 } }, /* "\u00A8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x54880c7e, 0xcefef2ab59255b65), 1, { 169, 0 } }, /* "\u00A9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x389ea08f, 0x131012eb1499f1ac), 1, { 170, 0 } }, /* "\u00AA" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x865b3bf, 0xba39153fe4798a42), 1, { 171, 0 } }, /* "\u00AB" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x35a7dc1b, 0xc79c5f93745e3319), 1, { 172, 0 } }, /* "\u00AC" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x766bd2aa, 0xf85783d9b5fd44f8), 1, { 173, 0 } }, /* "\u00AD" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2a77a704, 0x6ab6b046a01fd2d4), 1, { 174, 0 } }, /* "\u00AE" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x892a60b0, 0x13b3603ab647f158), 1, { 175, 0 } }, /* "\u00AF" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x572e35df, 0x1e63bd2046bd502), 1, { 176, 0 } }, /* "\u00B0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x98b0a259, 0x9567645b511655e), 1, { 177, 0 } }, /* "\u00B1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe22a9607, 0xc56e1fd0f9be4743), 1, { 178, 0 } }, /* "\u00B2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x18e7929e, 0x4ed56f90cecd1cdb), 1, { 179, 0 } }, /* "\u00B3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf6b10d3c, 0xd44f9594e774f364), 1, { 180, 0 } }, /* "\u00B4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x909924e2, 0xa07227702453d1f5), 1, { 181, 0 } }, /* "\u00B5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe8e2955b, 0x73d68175a5c45b1d), 1, { 182, 0 } }, /* "\u00B6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x817a4f7d, 0xadf4b3038468dd5d), 1, { 183, 0 } }, /* "\u00B7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x61310203, 0xafb20ebab3ef13e), 1, { 184, 0 } }, /* "\u00B8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x9d4a49c6, 0x2e8915a9d41f07b1), 1, { 185, 0 } }, /* "\u00B9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf34b9338, 0x6d619776c1931456), 1, { 186, 0 } }, /* "\u00BA" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x754d6492, 0xb2f56a49e31ceb1d), 1, { 187, 0 } }, /* "\u00BB" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd5319c0, 0x7890856d72536f63), 1, { 188, 0 } }, /* "\u00BC" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5ff4c2da, 0x1741c31c3ab065f5), 1, { 189, 0 } }, /* "\u00BD" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xa96733c4, 0xae4d280ad1361d4c), 1, { 190, 0 } }, /* "\u00BE" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x20eefa3, 0x9933aea2bfb6c76a), 1, { 191, 0 } }, /* "\u00BF" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x423c3593, 0xbb54e977176754fe), 1, { 192, 0 } }, /* "\u00C0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbd0a2b22, 0xefed258d67713c3f), 1, { 193, 0 } }, /* "\u00C1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1024e5c2, 0x4bca4557ab459e67), 1, { 194, 0 } }, /* "\u00C2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x29de8a53, 0x38aac49d7479d29d), 1, { 195, 0 } }, /* "\u00C3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xb27dc945, 0x11f76267f0acd7aa), 1, { 196, 0 } }, /* "\u00C4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x7a3719c2, 0xcec0331491a43040), 1, { 197, 0 } }, /* "\u00C5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xfdd287c7, 0x7b5a3313e7a634a5), 1, { 198, 0 } }, /* "\u00C6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe1b96e02, 0x5c4d6a862d740fc1), 1, { 199, 0 } }, /* "\u00C7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd4b8860, 0xa55a63e8431e49a), 1, { 200, 0 } }, /* "\u00C8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x553a8f49, 0x5f2dc204f2f25501), 1, { 201, 0 } }, /* "\u00C9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2f8cecee, 0xc362c292d6ff4fd7), 1, { 202, 0 } }, /* "\u00CA" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xe26ed4b5, 0xcfea35c7bed1f6aa), 1, { 203, 0 } }, /* "\u00CB" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd0511b5d, 0x9e0de8649b44cc38), 1, { 204, 0 } }, /* "\u00CC" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3ee4c4ee, 0x52e1c78825a0e88a), 1, { 205, 0 } }, /* "\u00CD" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd3aefb43, 0xcf17c34a7c215598), 1, { 206, 0 } }, /* "\u00CE" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6d1e9792, 0x23516d05cfc0f54e), 1, { 207, 0 } }, /* "\u00CF" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6254169d, 0x838a32c8d5d9f394), 1, { 208, 0 } }, /* "\u00D0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xdfdefd24, 0x3b7637a357a5142), 1, { 209, 0 } }, /* "\u00D1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x638ee7e5, 0x1d645db52aeedd30), 1, { 210, 0 } }, /* "\u00D2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xedcdfc3, 0x1cd8fefa4646de6e), 1, { 211, 0 } }, /* "\u00D3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2c4f3dfa, 0x5cf2a9d1a70bf440), 1, { 212, 0 } }, /* "\u00D4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xcadb7ecc, 0x4440c5bb5349d492), 1, { 213, 0 } }, /* "\u00D5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x8b7df0ec, 0x1ad0affbbafc439f), 1, { 214, 0 } }, /* "\u00D6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xdb393caa, 0xf96840db7a76cf4a), 1, { 215, 0 } }, /* "\u00D7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x54a5914e, 0xf76096b95cdbe5cb), 1, { 216, 0 } }, /* "\u00D8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x4a825262, 0xcb178d94685c2c32), 1, { 217, 0 } }, /* "\u00D9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x76e0e183, 0x6b9611b649e44517), 1, { 218, 0 } }, /* "\u00DA" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x86162190, 0x8f94247b227ecb7d), 1, { 219, 0 } }, /* "\u00DB" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x1408acb7, 0x22f70ccfc8cc5012), 1, { 220, 0 } }, /* "\u00DC" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xd3a8d217, 0x1be9bda75184e7db), 1, { 221, 0 } }, /* "\u00DD" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x603eb184, 0x58bab7867241c401), 1, { 222, 0 } }, /* "\u00DE" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xde8e7ee5, 0xa3435c8ca7b90dd6), 1, { 223, 0 } }, /* "\u00DF" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x72d89aff, 0x676a55394f4711fd), 1, { 224, 0 } }, /* "\u00E0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x9c92e2e, 0x5963ef834b1479f0), 1, { 225, 0 } }, /* "\u00E1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x71b53d0b, 0x8313e912dc0fe5a6), 1, { 226, 0 } }, /* "\u00E2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2e148332, 0xd35da5ed65c4a4cd), 1, { 227, 0 } }, /* "\u00E3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xa1eae10a, 0xdd9d08881a85d9ec), 1, { 228, 0 } }, /* "\u00E4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xea3aad3, 0xd4365583e439108e), 1, { 229, 0 } }, /* "\u00E5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5b40a9c5, 0xde2a435cd3cabceb), 1, { 230, 0 } }, /* "\u00E6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x824481cf, 0xc8322fb10cb3361d), 1, { 231, 0 } }, /* "\u00E7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xf1487507, 0xea384c058058d783), 1, { 232, 0 } }, /* "\u00E8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x44c6d4b2, 0x2e3f009e2330cf8a), 1, { 233, 0 } }, /* "\u00E9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xca28becc, 0xa4589dd3a6f0b641), 1, { 234, 0 } }, /* "\u00EA" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc05cbf3f, 0x3ab53e5222d07f16), 1, { 235, 0 } }, /* "\u00EB" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x69fde745, 0xc785e3d436f4a163), 1, { 236, 0 } }, /* "\u00EC" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xba1a82db, 0x4372ed80843c3abe), 1, { 237, 0 } }, /* "\u00ED" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x2f8a8a21, 0xc7de22ff0627cf09), 1, { 238, 0 } }, /* "\u00EE" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3f7bfe6a, 0x2d58d1808f3560f6), 1, { 239, 0 } }, /* "\u00EF" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xbcd766d, 0x97f840bcb3f482d), 1, { 240, 0 } }, /* "\u00F0" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x508d5a70, 0x9493b5ee0bf67899), 1, { 241, 0 } }, /* "\u00F1" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x8dc8d75d, 0xedd4260b1ce969e6), 1, { 242, 0 } }, /* "\u00F2" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x89875d4, 0xcf3134525fa53b8f), 1, { 243, 0 } }, /* "\u00F3" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x6f15207, 0xdb0d6723d2178f6d), 1, { 244, 0 } }, /* "\u00F4" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x31d7a60b, 0x7ec90f3b775f1197), 1, { 245, 0 } }, /* "\u00F5" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x73bcb0aa, 0x815fb5f67273f433), 1, { 246, 0 } }, /* "\u00F6" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x94ed7e8, 0x6c390dc23ad2e0aa), 1, { 247, 0 } }, /* "\u00F7" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x3233c4aa, 0xe68bea625d99199b), 1, { 248, 0 } }, /* "\u00F8" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x5f40e5cf, 0xbf334e81c5a86046), 1, { 249, 0 } }, /* "\u00F9" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x83bad66b, 0x79517fcd75d8419b), 1, { 250, 0 } }, /* "\u00FA" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x500d2e42, 0x1f60049a32b4a303), 1, { 251, 0 } }, /* "\u00FB" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xc04c9176, 0xa01b14fcb9489c5f), 1, { 252, 0 } }, /* "\u00FC" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x4ac4556d, 0x4abd1e8baa569960), 1, { 253, 0 } }, /* "\u00FD" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0x45e86e04, 0x6e1f6e31525df71), 1, { 254, 0 } }, /* "\u00FE" */
	{ Dee_OBJECT_HEAD_INIT(&DeeString_Type), NULL, _Dee_HashSelectC(0xfd6cf10d, 0xef5f27ae10fd1fb8), 1, { 255, 0 } }, /* "\u00FF" */
};
/*[[[end]]]*/
#endif /* CONFIG_STRING_LATIN1_STATIC || __DEEMON__ */


/* Return a string containing a single character. */
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_Chr8)(uint8_t ch) {
#ifdef CONFIG_STRING_LATIN1_CACHED
	DREF String *result;
	latin1_chars_lock_read();
	result = latin1_chars[ch];
	if (result) {
		Dee_Incref(result);
		latin1_chars_lock_endread();
	} else {
		latin1_chars_lock_endread();
		result = (DREF String *)DeeString_NewBuffer(1);
		if unlikely(!result)
			goto err;
		result->s_str[0] = (char)ch;
		latin1_chars_lock_write();
		if unlikely(latin1_chars[ch] != NULL) {
			DREF String *new_result = latin1_chars[ch];
			/* Special case: The string has been created in the mean time.
			 * This can even happen when threading is disabled, in case
			 * a GC callback invoked during allocation of our string did
			 * the job. */
			Dee_Incref(new_result);
			latin1_chars_lock_endwrite();
			Dee_Decref(result);
			return (DREF DeeObject *)new_result;
		}
		Dee_Incref(result); /* The reference stored in `latin1_chars' */
		latin1_chars[ch] = result;
		latin1_chars_lock_endwrite();
	}
	return (DREF DeeObject *)result;
err:
	return NULL;
#endif /* CONFIG_STRING_LATIN1_CACHED */

#ifdef CONFIG_STRING_LATIN1_NORMAL
	DREF String *result;
	result = (DREF String *)DeeString_NewBuffer(1);
	if likely(result)
		result->s_str[0] = (char)ch;
	return (DREF DeeObject *)result;
#endif /* CONFIG_STRING_LATIN1_NORMAL */

#ifdef CONFIG_STRING_LATIN1_STATIC
	String *result;
	result = (String *)&DeeString_Latin1[ch];
	Dee_Incref(result);
	return (DREF DeeObject *)result;
#endif /* CONFIG_STRING_LATIN1_STATIC */
}


PUBLIC WUNUSED NONNULL((1)) uint16_t const *
(DCALL DeeString_As2Byte)(DeeObject *__restrict self) {
	struct string_utf *utf;
again:
	utf = ((String *)self)->s_data;
	if (!utf) {
		utf = Dee_string_utf_alloc();
		if unlikely(!utf)
			goto err;
		bzero(utf, sizeof(struct string_utf));
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
#if STRING_WIDTH_1BYTE != 0
		utf->u_width = STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
		if unlikely(!atomic_cmpxch_weak(&((String *)self)->s_data, NULL, utf)) {
			Dee_string_utf_free(utf);
			goto again;
		}
		utf = Dee_string_utf_untrack(utf);
	}
	ASSERT(utf->u_width <= STRING_WIDTH_2BYTE);
	if likely(!utf->u_data[STRING_WIDTH_2BYTE]) {
		uint16_t *result;
		size_t i, length;
		uint8_t const *data;
		ASSERT(utf->u_width != STRING_WIDTH_2BYTE);
		ASSERT(utf->u_width == STRING_WIDTH_1BYTE);
		data   = (uint8_t const *)DeeString_STR(self);
		length = DeeString_SIZE(self);
		result = DeeString_New2ByteBuffer(length);
		if unlikely(!result)
			goto err;
		for (i = 0; i < length; ++i)
			result[i] = data[i];
		result[length] = 0;
		if likely(atomic_cmpxch(&utf->u_data[STRING_WIDTH_2BYTE], NULL, (size_t *)result)) {
			(void)Dee_UntrackAlloc((size_t *)result - 1);
			return result;
		}
		DeeString_Free2ByteBuffer(result);
	}
	return (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE];
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t const *
(DCALL DeeString_As4Byte)(DeeObject *__restrict self) {
	struct string_utf *utf;
again:
	utf = ((String *)self)->s_data;
	if (!utf) {
		utf = Dee_string_utf_alloc();
		if unlikely(!utf)
			goto err;
		bzero(utf, sizeof(struct string_utf));
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
#if STRING_WIDTH_1BYTE != 0
		utf->u_width = STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
		if unlikely(!atomic_cmpxch_weak(&((String *)self)->s_data, NULL, utf)) {
			Dee_string_utf_free(utf);
			goto again;
		}
		utf = Dee_string_utf_untrack(utf);
	}
	ASSERT(utf->u_width <= STRING_WIDTH_4BYTE);
	if likely(!utf->u_data[STRING_WIDTH_4BYTE]) {
		uint32_t *result;
		size_t i, length;
		ASSERT(utf->u_width != STRING_WIDTH_4BYTE);
		if (utf->u_width == STRING_WIDTH_1BYTE) {
			uint8_t const *data;
			data   = (uint8_t const *)DeeString_STR(self);
			length = DeeString_SIZE(self);
			result = DeeString_New4ByteBuffer(length);
			if unlikely(!result)
				goto err;
			for (i = 0; i < length; ++i)
				result[i] = data[i];
		} else {
			uint16_t const *data;
			ASSERT(utf->u_width == STRING_WIDTH_2BYTE);
			ASSERT(utf->u_data[STRING_WIDTH_2BYTE]);
			data   = (uint16_t const *)utf->u_data[STRING_WIDTH_2BYTE];
			length = WSTR_LENGTH(data);
			result = DeeString_New4ByteBuffer(length);
			if unlikely(!result)
				goto err;
			for (i = 0; i < length; ++i)
				result[i] = data[i];
		}
		result[length] = 0;
		if likely(atomic_cmpxch(&utf->u_data[STRING_WIDTH_4BYTE], NULL, result)) {
			(void)Dee_UntrackAlloc((size_t *)result - 1);
			return result;
		}
		DeeString_Free4ByteBuffer(result);
	}
	return (uint32_t *)utf->u_data[STRING_WIDTH_4BYTE];
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1)) /*utf-8*/ char const *DCALL
DeeString_AsUtf8(DeeObject *__restrict self) {
	struct string_utf *utf;
	uint8_t const *iter, *end;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
again:
	utf = ((String *)self)->s_data;
	if (utf) {
		if (utf->u_utf8)
			return utf->u_utf8;
		if ((utf->u_flags & STRING_UTF_FASCII) ||
		    (utf->u_width != STRING_WIDTH_1BYTE)) {
set_utf8_and_return_1byte:
			atomic_write(&utf->u_utf8, DeeString_STR(self));
			return DeeString_STR(self);
		}
	}
	/* We are either a LATIN1, or an ASCII string. */
	if (!utf) {
		/* Allocate the UTF-buffer. */
		utf = Dee_string_utf_alloc();
		if unlikely(!utf)
			goto err;
		bzero(utf, sizeof(struct string_utf));
		utf->u_width                    = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
		if unlikely(!atomic_cmpxch_weak(&((String *)self)->s_data, NULL, utf)) {
			Dee_string_utf_free(utf);
			goto again;
		}
		utf = Dee_string_utf_untrack(utf);
	}
	iter = (uint8_t const *)DeeString_STR(self);
	end  = iter + DeeString_SIZE(self);
	for (; iter < end; ++iter) {
		uint8_t ch = *iter;
		size_t result_length;
		uint8_t const *iter2;
		uint8_t *result, *dst;
		if (ch <= 0x7f)
			continue; /* ASCII character. */
		/* Well... This string _does_ contain some latin1 characters. */
		result_length = DeeString_SIZE(self) + 1;
		iter2         = iter + 1;
		for (; iter2 < end; ++iter2) {
			if (*iter2 >= 0x80)
				++result_length;
		}
		result = (uint8_t *)Dee_Mallococ(sizeof(size_t),
		                                 result_length + 1,
		                                 sizeof(uint8_t));
		if unlikely(!result)
			goto err;
		*(size_t *)result = result_length;
		result += sizeof(size_t);
		/* Copy leading ASCII-only data. */
		dst = (uint8_t *)mempcpyc(result, DeeString_STR(self),
		                          (size_t)(iter - (uint8_t const *)DeeString_STR(self)),
		                          sizeof(uint8_t));
		for (; iter < end; ++iter) {
			ch = *iter;
			if (ch <= 0x7f) {
				*dst++ = ch;
			} else {
				/* Encode the LATIN-1 character in UTF-8 */
				*dst++ = 0xc0 | ((ch & 0xc0) >> 6);
				*dst++ = 0x80 | (ch & 0x3f);
			}
		}
		ASSERT(WSTR_LENGTH(result) == result_length);
		ASSERT(dst == result + result_length);
		*dst = '\0';
		/* Save the generated UTF-8 string in the string's UTF cache. */
		if (!atomic_cmpxch(&utf->u_utf8, NULL, (char *)result)) {
			Dee_Free((size_t *)result - 1);
			ASSERT(utf->u_utf8 != NULL);
			return utf->u_utf8;
		}
		(void)Dee_UntrackAlloc((size_t *)result - 1);
		return (char const *)result;
	}
	/* No latin1 characters here! */
	atomic_or(&utf->u_flags, STRING_UTF_FASCII);
	goto set_utf8_and_return_1byte;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*utf-8*/ char const *DCALL
DeeString_TryAsUtf8(DeeObject *__restrict self) {
	struct string_utf *utf;
	uint8_t const *iter, *end;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
again:
	utf = ((String *)self)->s_data;
	if (utf) {
		if (utf->u_utf8)
			return utf->u_utf8;
		if ((utf->u_flags & STRING_UTF_FASCII) ||
		    (utf->u_width != STRING_WIDTH_1BYTE)) {
set_utf8_and_return_1byte:
			atomic_write(&utf->u_utf8, DeeString_STR(self));
			return DeeString_STR(self);
		}
	}
	/* We are either a LATIN1, or an ASCII string. */
	if (!utf) {
		/* Allocate the UTF-buffer. */
		utf = Dee_string_utf_tryalloc();
		if unlikely(!utf)
			goto err;
		bzero(utf, sizeof(struct string_utf));
		utf->u_width                    = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
		if unlikely(!atomic_cmpxch_weak(&((String *)self)->s_data, NULL, utf)) {
			Dee_string_utf_free(utf);
			goto again;
		}
		utf = Dee_string_utf_untrack(utf);
	}
	iter = (uint8_t const *)DeeString_STR(self);
	end  = iter + DeeString_SIZE(self);
	for (; iter < end; ++iter) {
		uint8_t ch = *iter;
		size_t result_length;
		uint8_t const *iter2;
		uint8_t *result, *dst;
		if (ch <= 0x7f)
			continue; /* ASCII character. */
		/* Well... This string _does_ contain some latin1 characters. */
		result_length = DeeString_SIZE(self) + 1;
		iter2         = iter + 1;
		for (; iter2 < end; ++iter2) {
			if (*iter2 >= 0x80)
				++result_length;
		}
		result = (uint8_t *)Dee_TryMalloc(sizeof(size_t) +
		                                  (result_length + 1) *
		                                  sizeof(uint8_t));
		if unlikely(!result)
			goto err;
		*(size_t *)result = result_length;
		result += sizeof(size_t);
		/* Copy leading ASCII-only data. */
		dst = (uint8_t *)mempcpyc(result, DeeString_STR(self),
		                          (size_t)(iter - (uint8_t const *)DeeString_STR(self)),
		                          sizeof(uint8_t));
		for (; iter < end; ++iter) {
			ch = *iter;
			if (ch <= 0x7f) {
				*dst++ = ch;
			} else {
				/* Encode the LATIN-1 character in UTF-8 */
				*dst++ = 0xc0 | ((ch & 0xc0) >> 6);
				*dst++ = 0x80 | (ch & 0x3f);
			}
		}
		ASSERT(WSTR_LENGTH(result) == result_length);
		ASSERT(dst == result + result_length);
		*dst = '\0';
		/* Save the generated UTF-8 string in the string's UTF cache. */
		if (!atomic_cmpxch(&utf->u_utf8, NULL, (char *)result)) {
			Dee_Free((size_t *)result - 1);
			ASSERT(utf->u_utf8 != NULL);
			return utf->u_utf8;
		}
		(void)Dee_UntrackAlloc((size_t *)result - 1);
		return (char const *)result;
	}
	/* No latin1 characters here! */
	atomic_or(&utf->u_flags, STRING_UTF_FASCII);
	goto set_utf8_and_return_1byte;
err:
	return NULL;
}

/* Return the given string's characters as a byte-array.
 * Characters above 0xFF either cause `NULL' to be returned, alongside
 * a ValueError being thrown, or cause them to be replaced with '?'.
 * @return: * :   The Bytes-data of the given string `self' (encoded as a width-string)
 *                NOTE: The length of this block also matches `DeeString_WLEN(self)'
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) /*latin-1*/ byte_t const *DCALL
DeeString_AsBytes(DeeObject *__restrict self, bool allow_invalid) {
	struct string_utf *utf;
	union dcharptr_const str;
	bool contains_invalid;
	byte_t *result;
	size_t i, length;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	utf = ((String *)self)->s_data;

	/* Simple case: No UTF data, or 1-byte string means that the string's
	 *              1-byte variant has no special encoding (aka. is LATIN-1). */
	if (!utf || utf->u_width == STRING_WIDTH_1BYTE)
		return (byte_t const *)DeeString_STR(self);

	/* The single-byte variant of the string was already allocated.
	 * Check how that variant's INV-byte flag compares to the caller's
	 * allow_invalid option. */
	if (!allow_invalid && (utf->u_flags & STRING_UTF_FINVBYT))
		goto err_invalid_string;

	/* Check for a cached single-byte variant. */
	result = (byte_t *)utf->u_data[STRING_WIDTH_1BYTE];
	if (result)
		return result;

	/* Since strings are allowed to use a wider default width than they
	 * may actually need, `self' may still only contain characters that
	 * fit into the 00-FF unicode range, so regardless of `allow_invalid'
	 * (which controls the behavior for characters outside that range) */
	str.ptr = utf->u_data[utf->u_width];
	ASSERT(utf->u_width != STRING_WIDTH_1BYTE);
	length = WSTR_LENGTH(str.ptr);
	ASSERT(length <= DeeString_SIZE(self));
	if (length == DeeString_SIZE(self)) {
		/* The actual length of the string matches the length of of its single-byte
		 * variant, in other words meaning that all of its characters could fit into
		 * that range, and that the string consists only of ASCII characters. */
		atomic_or(&utf->u_flags, STRING_UTF_FASCII);
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
		return (byte_t const *)DeeString_STR(self);
	}

	/* Try to construct the single-byte variant. */
	result = (byte_t *)Dee_Mallococ(sizeof(size_t),
	                                length + 1,
	                                sizeof(byte_t));
	if unlikely(!result)
		goto err;
	*(size_t *)result = length;
	result += sizeof(size_t);
	result[length]   = 0;
	contains_invalid = false;
	switch (utf->u_width) {

	case STRING_WIDTH_2BYTE:
		for (i = 0; i < length; ++i) {
			uint16_t ch = str.cp16[i];
			if (ch > 0xff) {
				if (!allow_invalid)
					goto err_result;
				contains_invalid = true;
				ch               = '?';
			}
			result[i] = (byte_t)ch;
		}
		break;

	case STRING_WIDTH_4BYTE:
		for (i = 0; i < length; ++i) {
			uint32_t ch = str.cp32[i];
			if (ch > 0xff) {
				if (!allow_invalid)
					goto err_result;
				contains_invalid = true;
				ch               = '?';
			}
			result[i] = (byte_t)ch;
		}
		break;

	default: __builtin_unreachable();
	}

	/* All right. - We've managed to construct the single-byte variant. */
	if (contains_invalid)
		atomic_or(&utf->u_flags, STRING_UTF_FINVBYT);

	/* Deal with race conditions. */
	if (!atomic_cmpxch(&utf->u_data[STRING_WIDTH_1BYTE], NULL, (size_t *)result)) {
		Dee_Free((size_t *)result - 1);
		return (byte_t const *)utf->u_data[STRING_WIDTH_1BYTE];
	}
	(void)Dee_UntrackAlloc((size_t *)result - 1);
	return result;
err_result:
	Dee_Free((size_t *)result - 1);
err_invalid_string:
	DeeError_Throwf(&DeeError_ValueError,
	                "The string contains characters that don't fit into a single byte");
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1)) uint16_t const *DCALL
DeeString_AsUtf16(DeeObject *__restrict self, unsigned int error_mode) {
	struct string_utf *utf;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	utf = ((String *)self)->s_data;
	if (utf) {
		if (utf->u_utf16)
			return (uint16_t *)utf->u_utf16;
		if (utf->u_width == STRING_WIDTH_1BYTE) {
			uint16_t const *result;
			/* A single-byte string has no characters within the surrogate-range,
			 * meaning we can simply request the string's 2-byte variant, and we'll
			 * automatically be given a valid utf-16 string! */
load_2byte_width:
			result = DeeString_As2Byte(self);
			if unlikely(!result)
				goto err;
			utf = ((String *)self)->s_data;
			ASSERT(utf != NULL);
			ASSERT(result == (uint16_t const *)utf->u_data[STRING_WIDTH_2BYTE]);
			utf->u_utf16 = (_Dee_string_utf_utf16_t *)result;
			return result;
		}
	} else {
		/* Without a UTF descriptor, we know that all characters are within
		 * the U+0000 ... U+00FF range, meaning we can simply request the 2-byte
		 * variant of the string, and automatically be given a valid utf-16 string. */
		goto load_2byte_width;
	}
	/* The complicated case: the string probably
	 * contains characters that need to be escaped. */
	ASSERT(utf != NULL);
	ASSERT(utf->u_width != STRING_WIDTH_1BYTE);
	switch (utf->u_width) {

	CASE_WIDTH_2BYTE: {
		size_t i, length;
		uint16_t const *str;
		uint16_t *result, *dst;
		str    = (uint16_t const *)utf->u_data[STRING_WIDTH_2BYTE];
		length = WSTR_LENGTH(str);
		/* Search if the string contains surrogate-characters. */
		for (i = 0; i < length; ++i) {
			uint16_t ch = str[i];
			if ((ch < UTF16_HIGH_SURROGATE_MIN || ch > UTF16_HIGH_SURROGATE_MAX) &&
			    (ch < UTF16_LOW_SURROGATE_MIN || ch > UTF16_LOW_SURROGATE_MAX))
				continue;
			if (!(error_mode & (STRING_ERROR_FREPLAC | STRING_ERROR_FIGNORE))) {
				DeeError_Throwf(&DeeError_UnicodeEncodeError,
				                "Invalid UTF-16 character U+%.4I16X", ch);
				goto err;
			}
			/* Must generate a fixed variant. */
			result = DeeString_New2ByteBuffer(length);
			if unlikely(!result)
				goto err;
			memcpyw(result, str, i);
			dst = result + 1;
			if (!(error_mode & STRING_ERROR_FIGNORE))
				*dst++ = '?';
			while (++i < length) {
				ch = str[i];
				if ((ch >= UTF16_HIGH_SURROGATE_MIN && ch <= UTF16_HIGH_SURROGATE_MAX) ||
				    (ch >= UTF16_LOW_SURROGATE_MIN && ch <= UTF16_LOW_SURROGATE_MAX)) {
					if (error_mode & STRING_ERROR_FIGNORE)
						continue;
					ch = '?';
				}
				*dst++ = ch;
			}
			*dst = 0;
			if (error_mode & STRING_ERROR_FIGNORE) {
				uint16_t *new_result;
				new_result = DeeString_TryResize2ByteBuffer(result, (size_t)(dst - result));
				if likely(new_result)
					result = new_result;
			}
			/* Save the utf-16 encoded string. */
			if (!atomic_cmpxch(&utf->u_utf16, NULL, (_Dee_string_utf_utf16_t *)result)) {
				DeeString_Free2ByteBuffer(result);
				return (uint16_t *)utf->u_utf16;
			}
			(void)Dee_UntrackAlloc((size_t *)result - 1);
			return result;
		}
		/* The 2-byte variant doesn't contain any illegal characters,
		 * so we can simply re-use it as the utf-16 variant. */
		atomic_cmpxch(&utf->u_utf16, NULL, (_Dee_string_utf_utf16_t *)str);
		return (uint16_t *)utf->u_utf16;
	}	break;

	CASE_WIDTH_4BYTE: {
		size_t i, length, result_length;
		uint32_t const *str;
		uint16_t *result, *dst;
		/* The string is full-on UTF-32.
		 * -> Count the number of characters that need surrogates */
		str = (uint32_t *)utf->u_data[STRING_WIDTH_4BYTE];
		result_length = length = WSTR_LENGTH(str);
		for (i = 0; i < length; ++i) {
			uint32_t ch = str[i];
			if (ch <= 0xffff) {
				if ((ch >= UTF16_HIGH_SURROGATE_MIN && ch <= UTF16_HIGH_SURROGATE_MAX) ||
				    (ch >= UTF16_LOW_SURROGATE_MIN && ch <= UTF16_LOW_SURROGATE_MAX))
					goto err_invalid_unicode;
				continue;
			} else if (ch > 0x10ffff) {
err_invalid_unicode:
				if (!(error_mode & (STRING_ERROR_FREPLAC | STRING_ERROR_FIGNORE))) {
					DeeError_Throwf(&DeeError_UnicodeEncodeError,
					                "Invalid unicode character U+%.4" PRFX32, ch);
					goto err;
				}
				if (error_mode & STRING_ERROR_FIGNORE)
					--result_length;
				continue;
			}
			++result_length;
		}
		result = DeeString_New2ByteBuffer(result_length);
		if unlikely(!result)
			goto err;
		dst = result;
		for (i = 0; i < length; ++i) {
			uint32_t ch = str[i];
			if (ch <= 0xffff) {
				if ((ch >= UTF16_HIGH_SURROGATE_MIN && ch <= UTF16_HIGH_SURROGATE_MAX) ||
				    (ch >= UTF16_LOW_SURROGATE_MIN && ch <= UTF16_LOW_SURROGATE_MAX)) {
					if (error_mode & STRING_ERROR_FIGNORE)
						continue;
					ch = '?';
				}
				*dst++ = (uint16_t)ch;
			} else if (ch > 0x10ffff) {
				if (error_mode & STRING_ERROR_FIGNORE)
					continue;
				*dst++ = '?';
			} else {
				/* Must encode as a high/low surrogate pair. */
				*dst++ = UTF16_HIGH_SURROGATE_MIN + (uint16_t)(ch >> 10);
				*dst++ = UTF16_LOW_SURROGATE_MIN + (uint16_t)(ch & 0x3ff);
			}
		}
		ASSERT(dst == result + result_length);
		*dst = 0;
		/* Save the utf-16 encoded string. */
		if (!atomic_cmpxch(&utf->u_utf16, NULL, (_Dee_string_utf_utf16_t *)result)) {
			DeeString_Free2ByteBuffer(result);
			return (uint16_t *)utf->u_utf16;
		}
		(void)Dee_UntrackAlloc((size_t *)result - 1);
		return result;
	}	break;

	default: __builtin_unreachable();
	}
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_printstring(struct unicode_printer *__restrict self,
                                DeeObject *__restrict string) {
	struct string_utf *utf;
	union dcharptr_const str;
	ASSERT_OBJECT_TYPE_EXACT(string, &DeeString_Type);
	utf = ((String *)string)->s_data;
	/* Try to optimize, by printing character data directly. */
	if (!utf) {
		/* Raw, 8-bit character (aka. LATIN-1) */
		return unicode_printer_print8(self,
		                              (uint8_t *)DeeString_STR(string),
		                              DeeString_SIZE(string));
	}
	/* Directly print sized string data (without the need to convert to & from UTF-8). */
	str.ptr = utf->u_data[utf->u_width];
	SWITCH_SIZEOF_WIDTH(utf->u_width) {

	CASE_WIDTH_1BYTE:
		return unicode_printer_print8(self, str.cp8, WSTR_LENGTH(str.cp8));

	CASE_WIDTH_2BYTE:
		return unicode_printer_print16(self, str.cp16, WSTR_LENGTH(str.cp16));

	CASE_WIDTH_4BYTE:
		return unicode_printer_print32(self, str.cp32, WSTR_LENGTH(str.cp32));
	}
}


/* Print the text of `self' to `printer', encoded as a UTF-8 string.
 * NOTE: If `printer' is `&unicode_printer_print', special optimization
 *       is done, meaning that this is the preferred method of printing
 *       an object to a unicode printer.
 * NOTE: This optimization is also done when `DeeObject_Print' is used. */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeString_PrintUtf8(DeeObject *__restrict self,
                    Dee_formatprinter_t printer,
                    void *arg) {
	struct string_utf *utf;
	uint8_t const *iter, *end, *flush_start;
	Dee_ssize_t temp, result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	/* `DeeString_STR()' is the single-byte variant.
	 * That only means UTF-8 if the string is an ASCII
	 * string, or if its character with is greater than
	 * one (in which case the single-byte variant is the
	 * UTF-8 version of the string) */
again:
	utf = ((String *)self)->s_data;
	if (printer == &unicode_printer_print) {
		union dcharptr_const str;
		/* Try to optimize, by printing character data directly. */
		if (!utf) {
			/* Raw, 8-bit character (aka. LATIN-1) */
			return unicode_printer_print8((struct unicode_printer *)arg,
			                              (uint8_t const *)DeeString_STR(self),
			                              DeeString_SIZE(self));
		}
		/* Directly print sized string data (without the need to convert to & from UTF-8). */
		str.ptr = utf->u_data[utf->u_width];
		SWITCH_SIZEOF_WIDTH(utf->u_width) {

		CASE_WIDTH_1BYTE:
			return unicode_printer_print8((struct unicode_printer *)arg, str.cp8, WSTR_LENGTH(str.cp8));

		CASE_WIDTH_2BYTE:
			return unicode_printer_print16((struct unicode_printer *)arg, str.cp16, WSTR_LENGTH(str.cp16));

		CASE_WIDTH_4BYTE:
			return unicode_printer_print32((struct unicode_printer *)arg, str.cp32, WSTR_LENGTH(str.cp32));
		}
	}
	if (utf) {
		/* The string has a pre-allocated UTF-8 variant. */
		if (utf->u_utf8)
			return (*printer)(arg, utf->u_utf8, WSTR_LENGTH(utf->u_utf8));
		if ((utf->u_flags & STRING_UTF_FASCII) ||
		    (utf->u_width != STRING_WIDTH_1BYTE)) {
			/* The single-byte variant is known to be encoded in UTF-8, or ASCII */
print_ascii:
			atomic_write(&utf->u_utf8, DeeString_STR(self));
			return (*printer)(arg, DeeString_STR(self), DeeString_SIZE(self));
		}
	}
	/* The UTF-8 string is encoded in LATIN-1, or ASCII */
	if (!utf) {
		/* Allocate the UTF-buffer. */
		utf = Dee_string_utf_alloc();
		if unlikely(!utf)
			goto err;
		bzero(utf, sizeof(struct string_utf));
		utf->u_width = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
		if unlikely(!atomic_cmpxch_weak(&((String *)self)->s_data, NULL, utf)) {
			Dee_string_utf_free(utf);
			goto again;
		}
		utf = Dee_string_utf_untrack(utf);
	}
	result = 0;
	iter = (uint8_t const *)DeeString_STR(self);
	end  = iter + DeeString_SIZE(self);
	flush_start = iter;
	for (; iter < end; ++iter) {
		uint8_t utf8_encoded[2];
		uint8_t ch = *iter;
		if (ch <= 0x7f)
			continue; /* ASCII character. */

		/* This is a true LATIN-1 character, which we
		 * must manually encode as a 2-byte UTF-8 string. */
		if (iter != flush_start) {
			size_t count = (size_t)((char const *)iter -
			                        (char const *)flush_start);
			DO(err_temp, (*printer)(arg, (char const *)flush_start, count));
		}

		/* Encode as: 110---xx 10xxxxxx */
		utf8_encoded[0] = 0xc0 | ((ch & 0xc0) >> 6);
		utf8_encoded[1] = 0x80 | (ch & 0x3f);
		DO(err_temp, (*printer)(arg, (char const *)utf8_encoded, 2 * sizeof(char)));
		flush_start = iter + 1;
	}
	if (flush_start == (uint8_t const *)DeeString_STR(self)) {
		/* The entire string is ASCII (remember this fact!) */
		atomic_or(&utf->u_flags, STRING_UTF_FASCII);
		goto print_ascii;
	}

	/* Flush the remainder. */
	if (flush_start != iter) {
		size_t count = (size_t)((char const *)iter -
		                        (char const *)flush_start);
		DO(err_temp, (*printer)(arg, (char const *)flush_start, count));
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

/* Print the escape-encoded variant of `self' */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeString_PrintRepr(DeeObject *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	union dcharptr_const str;
	Dee_ssize_t temp, result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	str.ptr = DeeString_WSTR(self);
	result = DeeFormat_PRINT(printer, arg, "\"");
	if unlikely(result < 0)
		goto done;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		temp = DeeFormat_Quote8(printer, arg, str.cp8, WSTR_LENGTH(str.cp8));
		break;

	CASE_WIDTH_2BYTE:
		temp = DeeFormat_Quote16(printer, arg, str.cp16, WSTR_LENGTH(str.cp16));
		break;

	CASE_WIDTH_4BYTE:
		temp = DeeFormat_Quote32(printer, arg, str.cp32, WSTR_LENGTH(str.cp32));
		break;
	}
	if unlikely(temp < 0)
		goto err;
	result += temp;
	DO(err, DeeFormat_PRINT(printer, arg, "\""));
done:
	return result;
err:
	return temp;
}



LOCAL NONNULL((1, 3)) void DCALL
char16_to_utf8(uint16_t const *__restrict src, size_t src_len,
               uint8_t *__restrict dst_utf8) {
	size_t i;
	for (i = 0; i < src_len; ++i) {
		uint16_t ch = src[i];
		if (ch <= UTF8_1BYTE_MAX) {
			*dst_utf8++ = (uint8_t)ch;
		} else if (ch <= UTF8_2BYTE_MAX) {
			*dst_utf8++ = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else {
			*dst_utf8++ = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		}
	}
}

LOCAL NONNULL((1, 3)) void DCALL
char16_to_utf8_fast(uint16_t const *__restrict src, size_t src_len,
                    uint8_t *__restrict dst_c8) {
	size_t i;
	for (i = 0; i < src_len; ++i) {
		uint16_t ch = src[i];
		ASSERT(ch <= UTF8_1BYTE_MAX);
		*dst_c8++ = (uint8_t)ch;
	}
}

LOCAL NONNULL((1, 3)) void DCALL
utf16_to_utf8(uint16_t const *__restrict src, size_t src_len,
              uint8_t *__restrict dst_utf8) {
	size_t i;
	for (i = 0; i < src_len; ++i) {
		uint32_t ch = src[i];
		if (ch >= UTF16_HIGH_SURROGATE_MIN &&
		    ch <= UTF16_HIGH_SURROGATE_MAX) {
			ASSERT(i < src_len - 1);
			ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
			++i;
			ASSERT(src[i] >= UTF16_LOW_SURROGATE_MIN);
			ASSERT(src[i] <= UTF16_LOW_SURROGATE_MAX);
			ch |= src[i] - UTF16_LOW_SURROGATE_MIN;
			ch += UTF16_SURROGATE_SHIFT;
		}
		ASSERT(ch <= 0x10FFFF);
		if (ch <= UTF8_1BYTE_MAX) {
			*dst_utf8++ = (uint8_t)ch;
		} else if (ch <= UTF8_2BYTE_MAX) {
			*dst_utf8++ = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else if (ch <= UTF8_3BYTE_MAX) {
			*dst_utf8++ = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else {
			*dst_utf8++ = 0xf0 | (uint8_t)((ch >> 18) /* & 0x07*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		}
	}
}

LOCAL NONNULL((1, 3, 4)) void DCALL
utf16_to_utf8_char16(uint16_t const *__restrict src, size_t src_len,
                     uint8_t *__restrict dst_utf8,
                     uint16_t *__restrict dst_c16) {
	size_t i;
	for (i = 0; i < src_len; ++i) {
		uint32_t ch = src[i];
		if (ch >= UTF16_HIGH_SURROGATE_MIN &&
		    ch <= UTF16_HIGH_SURROGATE_MAX) {
			ASSERT(i < src_len - 1);
			ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
			++i;
			ASSERT(src[i] >= UTF16_LOW_SURROGATE_MIN);
			ASSERT(src[i] <= UTF16_LOW_SURROGATE_MAX);
			ch |= src[i] - UTF16_LOW_SURROGATE_MIN;
			ch += UTF16_SURROGATE_SHIFT;
		}
		ASSERT(ch <= 0xFFFF);
		*dst_c16++ = (uint16_t)ch;
		if (ch <= UTF8_1BYTE_MAX) {
			*dst_utf8++ = (uint8_t)ch;
		} else if (ch <= UTF8_2BYTE_MAX) {
			*dst_utf8++ = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else {
			*dst_utf8++ = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		}
	}
}

LOCAL NONNULL((1, 3, 4)) void DCALL
utf16_to_utf8_char32(uint16_t const *__restrict src, size_t src_len,
                     uint8_t *__restrict dst_utf8,
                     uint32_t *__restrict dst_c32) {
	size_t i;
	for (i = 0; i < src_len; ++i) {
		uint32_t ch = src[i];
		if (ch >= UTF16_HIGH_SURROGATE_MIN &&
		    ch <= UTF16_HIGH_SURROGATE_MAX) {
			ASSERT(i < src_len - 1);
			ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
			++i;
			ASSERT(src[i] >= UTF16_LOW_SURROGATE_MIN);
			ASSERT(src[i] <= UTF16_LOW_SURROGATE_MAX);
			ch |= src[i] - UTF16_LOW_SURROGATE_MIN;
			ch += UTF16_SURROGATE_SHIFT;
		}
		ASSERT(ch <= 0x10FFFF);
		*dst_c32++ = ch;
		if (ch <= UTF8_1BYTE_MAX) {
			*dst_utf8++ = (uint8_t)ch;
		} else if (ch <= UTF8_2BYTE_MAX) {
			*dst_utf8++ = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else if (ch <= UTF8_3BYTE_MAX) {
			*dst_utf8++ = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else {
			*dst_utf8++ = 0xf0 | (uint8_t)((ch >> 18) /* & 0x07*/);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst_utf8++ = 0x80 | (uint8_t)((ch)&0x3f);
		}
	}
}

LOCAL NONNULL((1, 3)) void DCALL
utf32_to_utf8(uint32_t const *__restrict src, size_t src_len,
              uint8_t *__restrict dst) {
	size_t i;
	for (i = 0; i < src_len; ++i) {
		uint32_t ch = src[i];
		if (ch <= UTF8_1BYTE_MAX) {
			*dst++ = (uint8_t)ch;
		} else if (ch <= UTF8_2BYTE_MAX) {
			*dst++ = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
			*dst++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else if (ch <= UTF8_3BYTE_MAX) {
			*dst++ = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
			*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else if (ch <= UTF8_4BYTE_MAX) {
			*dst++ = 0xf0 | (uint8_t)((ch >> 18) /* & 0x07*/);
			*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else if (ch <= UTF8_5BYTE_MAX) {
			*dst++ = 0xf8 | (uint8_t)((ch >> 24) /* & 0x03*/);
			*dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else if (ch <= UTF8_6BYTE_MAX) {
			*dst++ = 0xfc | (uint8_t)((ch >> 30) /* & 0x01*/);
			*dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch)&0x3f);
		} else {
			*dst++ = 0xfe;
			*dst++ = 0x80 | (uint8_t)((ch >> 30) & 0x03 /* & 0x3f*/);
			*dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			*dst++ = 0x80 | (uint8_t)((ch)&0x3f);
		}
	}
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Pack2ByteBuffer(/*inherit(always)*/ uint16_t *__restrict text) {
	size_t i, length, utf8_length;
	DREF String *result;
	struct string_utf *utf;
	length = ((size_t *)text)[-1];
	if unlikely(!length) {
		Dee_Free((size_t *)text - 1);
		return DeeString_NewEmpty();
	}
	text[length] = 0;
	utf8_length  = 0;
	for (i = 0; i < length; ++i) {
		uint16_t ch = text[i];
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_length += 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_length += 2;
		} else {
			utf8_length += 3;
		}
	}
	ASSERT(utf8_length >= length);
	result = (DREF String *)DeeObject_Mallocc(offsetof(String, s_str),
	                                          utf8_length + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = utf8_length;
	utf = Dee_string_utf_alloc();
	if unlikely(!utf)
		goto err_r;
	bzero(utf, sizeof(struct string_utf));
	if (utf8_length == length) {
		utf->u_width = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
	} else {
		utf->u_width = STRING_WIDTH_2BYTE;
	}
	char16_to_utf8(text, length, (uint8_t *)result->s_str);
	utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text; /* Inherit data */
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_str[utf8_length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	(void)Dee_UntrackAlloc((size_t *)text - 1);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
err:
	Dee_Free((size_t *)text - 1);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_TryPack2ByteBuffer(/*inherit(on_success)*/ uint16_t *__restrict text) {
	size_t i, length, utf8_length;
	DREF String *result;
	struct string_utf *utf;
	length = ((size_t *)text)[-1];
	if unlikely(!length) {
		Dee_Free((size_t *)text - 1);
		return DeeString_NewEmpty();
	}
	text[length] = 0;
	utf8_length  = 0;
	for (i = 0; i < length; ++i) {
		uint16_t ch = text[i];
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_length += 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_length += 2;
		} else {
			utf8_length += 3;
		}
	}
	ASSERT(utf8_length >= length);
	result = (DREF String *)DeeObject_TryMallocc(offsetof(String, s_str),
	                                             utf8_length + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = utf8_length;
	utf = Dee_string_utf_alloc();
	if unlikely(!utf)
		goto err_r;
	bzero(utf, sizeof(struct string_utf));
	if (utf8_length == length) {
		utf->u_width = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
		char16_to_utf8_fast(text, length, (uint8_t *)result->s_str);
	} else {
		utf->u_width = STRING_WIDTH_2BYTE;
		char16_to_utf8(text, length, (uint8_t *)result->s_str);
	}
	utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text; /* Inherit data */
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_str[utf8_length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	(void)Dee_UntrackAlloc((size_t *)text - 1);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_PackUtf16Buffer(/*inherit(always)*/ uint16_t *__restrict text,
                          unsigned int error_mode) {
	size_t i, length, utf8_length;
	struct string_utf *utf;
	DREF String *result;
	size_t character_count;
	int kind = 0; /* 0: UTF-16 w/o surrogates
	               * 1: UTF-16 w surrogates
	               * 2: UTF-16 w surrogates that produce character > 0xffff */
	length = ((size_t *)text)[-1];
	if unlikely(!length) {
		Dee_Free((size_t *)text - 1);
		return DeeString_NewEmpty();
	}
	text[length]    = 0;
	utf8_length     = 0;
	i               = 0;
	character_count = 0;
continue_at_i:
	for (; i < length; ++i) {
		uint32_t ch;
read_text_i:
		ch = text[i];
		if (ch >= UTF16_HIGH_SURROGATE_MIN &&
		    ch <= UTF16_HIGH_SURROGATE_MAX) {
			uint16_t low_value;
			/* Surrogate pair. */
			if unlikely(i >= length - 1) {
				/* Missing high surrogate. */
				if (error_mode & STRING_ERROR_FREPLAC) {
					text[i] = '?';
					break;
				}
				if (error_mode & STRING_ERROR_FIGNORE) {
					text[i] = 0;
					--length;
					((size_t *)text)[-1] = length;
					break;
				}
				DeeError_Throwf(&DeeError_UnicodeDecodeError,
				                "Missing low surrogate for high surrogate U+%" PRFX32,
				                ch);
				goto err;
			}
			ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
			++i;
			low_value = text[i];
			if unlikely(low_value < UTF16_LOW_SURROGATE_MIN ||
			            low_value > UTF16_LOW_SURROGATE_MAX) {
				/* Invalid low surrogate. */
				if (error_mode & STRING_ERROR_FREPLAC) {
					text[i - 1] = '?';
					--length;
					((size_t *)text)[-1] = length;
					memmovedownc(&text[i],
					             &text[i + 1],
					             length - i,
					             sizeof(uint16_t));
					goto read_text_i;
				}
				if (error_mode & STRING_ERROR_FIGNORE) {
					--i;
					length -= 2;
					ASSERT(length >= i);
					((size_t *)text)[-1] = length;
					memmovedownc(&text[i],
					             &text[i + 2],
					             length - i,
					             sizeof(uint16_t));
					goto continue_at_i;
				}
				DeeError_Throwf(&DeeError_UnicodeDecodeError,
				                "Invalid low surrogate U+%I16X paired with high surrogate U+%" PRFX32,
				                low_value, ch);
				goto err;
			}
			ch |= low_value - UTF16_LOW_SURROGATE_MIN;
			ch += UTF16_SURROGATE_SHIFT;
			if (!kind)
				kind = 1;
		}
		++character_count;
		ASSERT(ch <= 0x10FFFF);
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_length += 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_length += 2;
		} else if (ch <= UTF8_3BYTE_MAX) {
			utf8_length += 3;
		} else {
			kind = 2;
			utf8_length += 4;
		}
	}
	ASSERT(((size_t *)text)[-1] == length);
	ASSERT(utf8_length >= length);
	result = (DREF String *)DeeObject_Mallocc(offsetof(String, s_str),
	                                          utf8_length + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = utf8_length;
	utf = Dee_string_utf_alloc();
	if unlikely(!utf)
		goto err_r;
	bzero(utf, sizeof(struct string_utf));
	utf->u_utf16 = (_Dee_string_utf_utf16_t *)text; /* Inherit data */
	if (utf8_length == length) {
		size_t j;
		/* Pure UTF-16 in ASCII range. */
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
		utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text;
		(void)Dee_UntrackAlloc((size_t *)text - 1);
		utf->u_width = STRING_WIDTH_1BYTE;
		ASSERT(character_count == utf8_length);
		ASSERT(character_count == length);
		ASSERT(character_count == WSTR_LENGTH(text));
		for (j = 0; j < length; ++j)
			result->s_str[j] = (char)(uint8_t)text[j];
	} else {
		switch (kind) {

		case 0: /* Pure UTF-16 (without surrogates) */
			utf->u_width                    = STRING_WIDTH_2BYTE;
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text;
			ASSERTF(character_count == WSTR_LENGTH(text),
			        "character_count   = %" PRFuSIZ "\n"
			        "WSTR_LENGTH(text) = %" PRFuSIZ "\n"
			        "length            = %" PRFuSIZ "\n"
			        "text              = %$I16s\n",
			        character_count, WSTR_LENGTH(text),
			        length, WSTR_LENGTH(text), text);
			char16_to_utf8(text, length, (uint8_t *)result->s_str);
			break;

		case 1: {
			uint16_t *buffer;
			/* Regular UTF-16 in 2-byte range (with surrogates)
			 * -> Must convert the UTF-16 text into a 2-byte string. */
			buffer = DeeString_New2ByteBuffer(character_count);
			if unlikely(!buffer)
				goto err_r_utf;
			buffer[character_count]         = 0;
			utf->u_width                    = STRING_WIDTH_2BYTE;
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer;
			(void)Dee_UntrackAlloc((size_t *)buffer - 1);
			ASSERT(character_count < WSTR_LENGTH(text));
			utf16_to_utf8_char16(text, length, (uint8_t *)result->s_str, buffer);
		}	break;

		case 2: {
			uint32_t *buffer;
			/* Regular UTF-16 outside the 2-byte range (with surrogates)
			 * -> Must convert the UTF-16 text into a 4-byte string. */
			buffer = DeeString_New4ByteBuffer(character_count);
			if unlikely(!buffer)
				goto err_r_utf;
			buffer[character_count]         = 0;
			utf->u_width                    = STRING_WIDTH_4BYTE;
			utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer;
			(void)Dee_UntrackAlloc((size_t *)buffer - 1);
			ASSERT(character_count < WSTR_LENGTH(text));
			utf16_to_utf8_char32(text, length, (uint8_t *)result->s_str, buffer);
		}	break;

		default: __builtin_unreachable();
		}
	}
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_str[utf8_length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	return (DREF DeeObject *)result;
err_r_utf:
	Dee_string_utf_free(utf);
err_r:
	DeeObject_Free(result);
err:
	Dee_Free((size_t *)text - 1);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_TryPackUtf16Buffer(/*inherit(on_success)*/ uint16_t *__restrict text) {
	size_t i, length, utf8_length;
	struct string_utf *utf;
	DREF String *result;
	size_t character_count;
	int kind = 0; /* 0: UTF-16 w/o surrogates
	               * 1: UTF-16 w surrogates
	               * 2: UTF-16 w surrogates that produce character > 0xffff */
	length = ((size_t *)text)[-1];
	if unlikely(!length) {
		Dee_Free((size_t *)text - 1);
		return DeeString_NewEmpty();
	}
	text[length]    = 0;
	utf8_length     = 0;
	i               = 0;
	character_count = 0;
continue_at_i:
	for (; i < length; ++i) {
		uint32_t ch;
/*read_text_i:*/
		ch = text[i];
		if (ch >= UTF16_HIGH_SURROGATE_MIN &&
		    ch <= UTF16_HIGH_SURROGATE_MAX) {
			uint16_t low_value;
			/* Surrogate pair. */
			if unlikely(i >= length - 1) {
				/* Missing high surrogate. */
				text[i] = 0;
				--length;
				((size_t *)text)[-1] = length;
				break;
			}
			ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
			++i;
			low_value = text[i];
			if unlikely(low_value < UTF16_LOW_SURROGATE_MIN ||
			            low_value > UTF16_LOW_SURROGATE_MAX) {
				/* Invalid low surrogate. */
				--i;
				length -= 2;
				((size_t *)text)[-1] = length;
				ASSERT(length >= i);
				memmovedownc(&text[i],
				             &text[i + 2],
				             length - i,
				             sizeof(uint16_t));
				goto continue_at_i;
			}
			ch |= low_value - UTF16_LOW_SURROGATE_MIN;
			ch += UTF16_SURROGATE_SHIFT;
			if (!kind)
				kind = 1;
		}
		++character_count;
		ASSERT(ch <= 0x10FFFF);
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_length += 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_length += 2;
		} else if (ch <= UTF8_3BYTE_MAX) {
			utf8_length += 3;
		} else {
			kind = 2;
			utf8_length += 4;
		}
	}
	ASSERT(((size_t *)text)[-1] == length);
	ASSERT(utf8_length >= length);
	result = (DREF String *)DeeObject_TryMallocc(offsetof(String, s_str),
	                                             utf8_length + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = utf8_length;
	utf = Dee_string_utf_tryalloc();
	if unlikely(!utf)
		goto err_r;
	bzero(utf, sizeof(struct string_utf));
	utf->u_utf16 = (_Dee_string_utf_utf16_t *)text; /* Inherit data */
	if (utf8_length == length) {
		size_t j;
		/* Pure UTF-16 in ASCII range. */
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
		utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text;
		(void)Dee_UntrackAlloc((size_t *)text - 1);
		utf->u_width = STRING_WIDTH_1BYTE;
		ASSERT(character_count == utf8_length);
		ASSERT(character_count == length);
		ASSERT(character_count == WSTR_LENGTH(text));
		for (j = 0; j < length; ++j)
			result->s_str[j] = (char)(uint8_t)text[j];
	} else {
		switch (kind) {

		case 0: /* Pure UTF-16 (without surrogates) */
			utf->u_width                    = STRING_WIDTH_2BYTE;
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text;
			ASSERTF(character_count == WSTR_LENGTH(text),
			        "character_count   = %" PRFuSIZ "\n"
			        "WSTR_LENGTH(text) = %" PRFuSIZ "\n"
			        "length            = %" PRFuSIZ "\n"
			        "text              = %$I16s\n",
			        character_count, WSTR_LENGTH(text), length, WSTR_LENGTH(text), text);
			char16_to_utf8(text, length, (uint8_t *)result->s_str);
			break;

		case 1: {
			uint16_t *buffer;
			/* Regular UTF-16 in 2-byte range (with surrogates)
				 * -> Must convert the UTF-16 text into a 2-byte string. */
			buffer = DeeString_TryNew2ByteBuffer(character_count);
			if unlikely(!buffer)
				goto err_r_utf;
			buffer[character_count]         = 0;
			utf->u_width                    = STRING_WIDTH_2BYTE;
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer;
			(void)Dee_UntrackAlloc((size_t *)buffer - 1);
			ASSERT(character_count < WSTR_LENGTH(text));
			utf16_to_utf8_char16(text, length, (uint8_t *)result->s_str, buffer);
		}	break;

		case 2: {
			uint32_t *buffer;
			/* Regular UTF-16 outside the 2-byte range (with surrogates)
			 * -> Must convert the UTF-16 text into a 4-byte string. */
			buffer = DeeString_TryNew4ByteBuffer(character_count);
			if unlikely(!buffer)
				goto err_r_utf;
			buffer[character_count]         = 0;
			utf->u_width                    = STRING_WIDTH_4BYTE;
			utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer;
			(void)Dee_UntrackAlloc((size_t *)buffer - 1);
			ASSERT(character_count < WSTR_LENGTH(text));
			utf16_to_utf8_char32(text, length, (uint8_t *)result->s_str, buffer);
		}	break;

		default: __builtin_unreachable();
		}
	}
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_str[utf8_length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	return (DREF DeeObject *)result;
err_r_utf:
	Dee_string_utf_free(utf);
err_r:
	DeeObject_Free(result);
err:
	/*Dee_Free((size_t *)text - 1);*/
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_PackUtf32Buffer(/*inherit(always)*/ uint32_t *__restrict text,
                          unsigned int error_mode) {
	size_t i, length, utf8_length;
	DREF String *result;
	struct string_utf *utf;
	length = ((size_t *)text)[-1];
	if unlikely(!length) {
		Dee_Free((size_t *)text - 1);
		return DeeString_NewEmpty();
	}
	text[length] = 0;
	utf8_length  = 0;
	for (i = 0; i < length; ++i) {
		uint32_t ch = text[i];
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_length += 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_length += 2;
		} else if (ch <= UTF8_3BYTE_MAX) {
			utf8_length += 3;
		} else if (ch <= UTF8_4BYTE_MAX) {
			utf8_length += 4;
		} else {
			/* Not actually a valid unicode character, but could be encoded! */
			if (error_mode & STRING_ERROR_FREPLAC) {
				/* Replace with a question mark. */
				text[i] = '?';
				utf8_length += 1;
			} else if (error_mode & STRING_ERROR_FIGNORE) {
				/* Just encode it... */
				if (ch <= UTF8_5BYTE_MAX) {
					utf8_length += 5;
				} else if (ch <= UTF8_6BYTE_MAX) {
					utf8_length += 6;
				} else {
					utf8_length += 7;
				}
			} else {
				DeeError_Throwf(&DeeError_UnicodeDecodeError,
				                "Invalid unicode character U+%" PRFX32,
				                ch);
				goto err;
			}
		}
	}
	ASSERT(utf8_length >= length);
	result = (DREF String *)DeeObject_Mallocc(offsetof(String, s_str),
	                                          utf8_length + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = utf8_length;
	utf = Dee_string_utf_alloc();
	if unlikely(!utf)
		goto err_r;
	bzero(utf, sizeof(struct string_utf));
	utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)text; /* Inherit data */
	utf32_to_utf8(text, length, (uint8_t *)result->s_str);
	if (utf8_length == length) {
		/* Pure UTF-32 in ASCII range. */
		utf->u_width                    = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
	} else {
		utf->u_width = STRING_WIDTH_4BYTE;
	}
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_str[utf8_length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	(void)Dee_UntrackAlloc((size_t *)text - 1);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
err:
	Dee_Free((size_t *)text - 1);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_TryPackUtf32Buffer(/*inherit(on_success)*/ uint32_t *__restrict text) {
	size_t i, length, utf8_length;
	DREF String *result;
	struct string_utf *utf;
	length = ((size_t *)text)[-1];
	if unlikely(!length) {
		Dee_Free((size_t *)text - 1);
		return DeeString_NewEmpty();
	}
	text[length] = 0;
	utf8_length  = 0;
	for (i = 0; i < length; ++i) {
		uint32_t ch = text[i];
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_length += 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_length += 2;
		} else if (ch <= UTF8_3BYTE_MAX) {
			utf8_length += 3;
		} else if (ch <= UTF8_4BYTE_MAX) {
			utf8_length += 4;
		} else if (ch <= UTF8_5BYTE_MAX) {
			utf8_length += 5;
		} else if (ch <= UTF8_6BYTE_MAX) {
			utf8_length += 6;
		} else {
			utf8_length += 7;
		}
	}
	ASSERT(utf8_length >= length);
	result = (DREF String *)DeeObject_TryMallocc(offsetof(String, s_str),
	                                             utf8_length + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = utf8_length;
	utf = Dee_string_utf_alloc();
	if unlikely(!utf)
		goto err_r;
	bzero(utf, sizeof(struct string_utf));
	utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)text; /* Inherit data */
	utf32_to_utf8(text, length, (uint8_t *)result->s_str);
	if (utf8_length == length) {
		/* Pure UTF-32 in ASCII range. */
		utf->u_width = STRING_WIDTH_1BYTE;
		utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
	} else {
		utf->u_width = STRING_WIDTH_4BYTE;
	}
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_str[utf8_length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	(void)Dee_UntrackAlloc((size_t *)text - 1);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
err:
	/*Dee_Free((size_t *)text - 1);*/
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1)) uint32_t DCALL
utf8_getchar(uint8_t const *__restrict base, uint8_t seqlen) {
	uint32_t result;
	switch (seqlen) {

	case 0:
		result = 0;
		break;

	case 1:
		result = base[0];
		break;

	case 2:
		result = (base[0] & 0x1f) << 6;
		result |= (base[1] & 0x3f);
		break;

	case 3:
		result = (base[0] & 0x0f) << 12;
		result |= (base[1] & 0x3f) << 6;
		result |= (base[2] & 0x3f);
		break;

	case 4:
		result = (base[0] & 0x07) << 18;
		result |= (base[1] & 0x3f) << 12;
		result |= (base[2] & 0x3f) << 6;
		result |= (base[3] & 0x3f);
		break;

	case 5:
		result = (base[0] & 0x03) << 24;
		result |= (base[1] & 0x3f) << 18;
		result |= (base[2] & 0x3f) << 12;
		result |= (base[3] & 0x3f) << 6;
		result |= (base[4] & 0x3f);
		break;

	case 6:
		result = (base[0] & 0x01) << 30;
		result |= (base[1] & 0x3f) << 24;
		result |= (base[2] & 0x3f) << 18;
		result |= (base[3] & 0x3f) << 12;
		result |= (base[4] & 0x3f) << 6;
		result |= (base[5] & 0x3f);
		break;

	case 7:
		result = (base[1] & 0x03 /*0x3f*/) << 30;
		result |= (base[2] & 0x3f) << 24;
		result |= (base[3] & 0x3f) << 18;
		result |= (base[4] & 0x3f) << 12;
		result |= (base[5] & 0x3f) << 6;
		result |= (base[6] & 0x3f);
		break;

	case 8:
		/*result = (base[0] & 0x3f) << 42;*/
		/*result = (base[1] & 0x3f) << 36;*/
		result = (base[2] & 0x03 /*0x3f*/) << 30;
		result |= (base[3] & 0x3f) << 24;
		result |= (base[4] & 0x3f) << 18;
		result |= (base[5] & 0x3f) << 12;
		result |= (base[6] & 0x3f) << 6;
		result |= (base[7] & 0x3f);
		break;

	default: __builtin_unreachable();
	}
	return result;
}

LOCAL WUNUSED NONNULL((1)) uint16_t DCALL
utf8_getchar16(uint8_t const *__restrict base, uint8_t seqlen) {
	uint16_t result;
	ASSERT(seqlen <= 3);
	switch (seqlen) {

	case 0:
		result = 0;
		break;

	case 1:
		result = base[0];
		break;

	case 2:
		result = (base[0] & 0x1f) << 6;
		result |= (base[1] & 0x3f);
		break;

	case 3:
		result = (base[0] & 0x0f) << 12;
		result |= (base[1] & 0x3f) << 6;
		result |= (base[2] & 0x3f);
		break;

	default: __builtin_unreachable();
	}
	return result;
}

/* Construct a string from a UTF-8 character sequence. */
#ifdef NDEBUG
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeDbgString_NewUtf8)(char const *__restrict str, size_t length,
                             unsigned int error_mode,
                             char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_NewUtf8(str, length, error_mode);
}
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_NewUtf8)(char const *__restrict str, size_t length,
                          unsigned int error_mode)
#define LOCAL_DeeDbgString(func, ...) DeeString_##func(__VA_ARGS__)
#else /* NDEBUG */
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_NewUtf8)(char const *__restrict str, size_t length,
                          unsigned int error_mode) {
	return DeeDbgString_NewUtf8(str, length, error_mode, NULL, 0);
}
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeDbgString_NewUtf8)(char const *__restrict str, size_t length,
                             unsigned int error_mode,
                             char const *file, int line)
#define LOCAL_DeeDbgString(func, ...) DeeDbgString_##func(__VA_ARGS__, file, line)
#endif /* !NDEBUG */
{
	DREF String *result;
	uint8_t *iter, *end;
	struct string_utf *utf = NULL;
	uint32_t *buffer32, *dst32;
	size_t i, simple_length, utf_length;
#ifdef NDEBUG
	result = (DREF String *)DeeObject_Mallocc(offsetof(String, s_str),
	                                          length + 1, sizeof(char));
#else /* NDEBUG */
	result = (DREF String *)DeeDbgObject_Mallocc(offsetof(String, s_str),
	                                             length + 1, sizeof(char),
	                                             file, line);
#endif /* !NDEBUG */
	if unlikely(!result)
		goto err;
	iter = (uint8_t *)memcpyc(result->s_str, str, length, sizeof(char));
	end  = iter + length;
	/* Search for multi-byte character sequences. */
	while (iter < end) {
		uint8_t seqlen, ch = *iter;
		uint32_t ch32;
		if (ch <= 0x7f) {
			++iter;
			continue;
		}
		seqlen = unicode_utf8seqlen[ch];
		if unlikely(!seqlen || iter + seqlen > end) {
			/* Invalid UTF-8 character */
			if (error_mode == STRING_ERROR_FREPLAC) {
				*iter = '?';
				++iter;
				continue;
			}
			if (error_mode == STRING_ERROR_FIGNORE) {
				--end;
				--length;
				memmovedownc(iter,
				             iter + 1,
				             end - iter,
				             sizeof(char));
				continue;
			}
			DeeError_Throwf(&DeeError_UnicodeDecodeError,
			                "Invalid utf-8 character byte 0x%.2I8x",
			                ch);
			goto err_r;
		}
		ch32 = utf8_getchar(iter, seqlen);
		if unlikely(ch32 <= 0x7f) {
			*iter = (uint8_t)ch32;
			memmovedownc(iter + 1,
			             iter + seqlen,
			             end - (iter + seqlen),
			             sizeof(char));
			++iter;
			continue;
		}
		if (ch32 > 0xffff) {
			/* Must decode into a 4-byte string */
			buffer32 = LOCAL_DeeDbgString(New4ByteBuffer, length - (seqlen - 1));
			if unlikely(!buffer32)
				goto err_r;
			dst32         = buffer32;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst32++ = (uint32_t)(uint8_t)result->s_str[i];
			*dst32++ = ch32;
			iter += seqlen;
use_buffer32:
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst32++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					if (error_mode == STRING_ERROR_FREPLAC) {
						*iter++  = '?';
						*dst32++ = '?';
						continue;
					}
					if (error_mode == STRING_ERROR_FIGNORE) {
						--end;
						memmovedownc(iter,
						             iter + 1,
						             end - iter,
						             sizeof(char));
						continue;
					}
					DeeError_Throwf(&DeeError_UnicodeDecodeError,
					                "Invalid utf-8 character byte 0x%.2I8x",
					                ch);
err_buffer32:
					LOCAL_DeeDbgString(Free4ByteBuffer, buffer32);
					goto err_r;
				}
				ch32     = utf8_getchar(iter, seqlen);
				*dst32++ = ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst32 - buffer32);
			ASSERT(utf_length <= WSTR_LENGTH(buffer32));
			if (utf_length != WSTR_LENGTH(buffer32)) {
				uint32_t *new_buffer32;
				new_buffer32 = LOCAL_DeeDbgString(TryResize4ByteBuffer, buffer32, utf_length);
				if likely(new_buffer32)
					buffer32 = new_buffer32;
			}
			utf = Dee_string_utf_alloc();
			if unlikely(!utf)
				goto err_buffer32;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer32; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer32 - 1);
			utf->u_width = STRING_WIDTH_4BYTE;
			utf->u_utf8  = result->s_str;
		} else {
			uint16_t *buffer16, *dst16;
			/* Must decode into a 2/4-byte string */
			buffer16 = LOCAL_DeeDbgString(New2ByteBuffer, length - (seqlen - 1));
			if unlikely(!buffer16)
				goto err_r;
			dst16         = buffer16;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst16++ = (uint16_t)(uint8_t)result->s_str[i];
			*dst16++ = (uint16_t)ch32;
			iter += seqlen;
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst16++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					if (error_mode == STRING_ERROR_FREPLAC) {
						*iter++  = '?';
						*dst16++ = '?';
						continue;
					}
					if (error_mode == STRING_ERROR_FIGNORE) {
						--end;
						memmovedownc(iter,
						             iter + 1,
						             end - iter,
						             sizeof(char));
						continue;
					}
					DeeError_Throwf(&DeeError_UnicodeDecodeError,
					                "Invalid utf-8 character byte 0x%.2I8x",
					                ch);
err_buffer16:
					LOCAL_DeeDbgString(Free2ByteBuffer, buffer16);
					goto err_r;
				}
				ch32 = utf8_getchar(iter, seqlen);
				if unlikely(ch32 > 0xffff) {
					iter += seqlen;
					/* Must upgrade to use a 4-byte buffer. */
					buffer32 = LOCAL_DeeDbgString(New4ByteBuffer, (dst16 - buffer16) + 1 + (end - iter));
					if unlikely(!buffer32)
						goto err_buffer16;
					simple_length = (size_t)(dst16 - buffer16);
					for (i = 0; i < simple_length; ++i)
						buffer32[i] = buffer16[i];
					LOCAL_DeeDbgString(Free2ByteBuffer, buffer16);
					dst32    = buffer32 + simple_length;
					*dst32++ = ch32;
					goto use_buffer32;
				}
				*dst16++ = (uint16_t)ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst16 - buffer16);
			ASSERT(utf_length <= WSTR_LENGTH(buffer16));
			if (utf_length != WSTR_LENGTH(buffer16)) {
				uint16_t *new_buffer16;
				new_buffer16 = LOCAL_DeeDbgString(TryResize2ByteBuffer, buffer16, utf_length);
				if likely(new_buffer16)
					buffer16 = new_buffer16;
			}
			utf = Dee_string_utf_alloc();
			if unlikely(!utf)
				goto err_buffer16;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer16; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer16 - 1);
			utf->u_width = STRING_WIDTH_2BYTE;
			utf->u_utf8  = result->s_str;
		}
		break;
	}
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_len  = length;
	result->s_str[length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
#undef LOCAL_DeeDbgString
}


/* Same as `DeeString_NewUtf8()', but uses `DeeObject_TryMalloc' & friends
 * Given `error_mode' _MUST_ be `Dee_STRING_ERROR_FREPLAC' or `Dee_STRING_ERROR_FIGNORE' */
#ifdef NDEBUG
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeDbgString_TryNewUtf8)(char const *__restrict str, size_t length,
                                unsigned int error_mode,
                                char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_TryNewUtf8(str, length, error_mode);
}
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_TryNewUtf8)(char const *__restrict str, size_t length,
                             unsigned int error_mode)
#define LOCAL_DeeDbgString(func, ...) DeeString_##func(__VA_ARGS__)
#else /* NDEBUG */
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_TryNewUtf8)(char const *__restrict str, size_t length,
                             unsigned int error_mode) {
	return DeeDbgString_TryNewUtf8(str, length, error_mode, NULL, 0);
}
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeDbgString_TryNewUtf8)(char const *__restrict str, size_t length,
                                unsigned int error_mode,
                                char const *file, int line)
#define LOCAL_DeeDbgString(func, ...) DeeDbgString_##func(__VA_ARGS__, file, line)
#endif /* !NDEBUG */
{
	DREF String *result;
	uint8_t *iter, *end;
	struct string_utf *utf = NULL;
	uint32_t *buffer32, *dst32;
	size_t i, simple_length, utf_length;
	ASSERT(error_mode == STRING_ERROR_FREPLAC ||
	       error_mode == STRING_ERROR_FIGNORE);
#ifdef NDEBUG
	result = (DREF String *)DeeObject_TryMallocc(offsetof(String, s_str),
	                                             length + 1, sizeof(char));
#else /* NDEBUG */
	result = (DREF String *)DeeDbgObject_TryMallocc(offsetof(String, s_str),
	                                                length + 1, sizeof(char),
	                                                file, line);
#endif /* !NDEBUG */
	if unlikely(!result)
		goto err;
	iter = (uint8_t *)memcpyc(result->s_str, str, length, sizeof(char));
	end  = iter + length;
	/* Search for multi-byte character sequences. */
	while (iter < end) {
		uint8_t seqlen, ch = *iter;
		uint32_t ch32;
		if (ch <= 0x7f) {
			++iter;
			continue;
		}
		seqlen = unicode_utf8seqlen[ch];
		if unlikely(!seqlen || iter + seqlen > end) {
			/* Invalid UTF-8 character */
			if (error_mode == STRING_ERROR_FREPLAC) {
				*iter = '?';
				++iter;
			} else {
				--end;
				--length;
				memmovedownc(iter,
				             iter + 1,
				             end - iter,
				             sizeof(char));
			}
			continue;
		}
		ch32 = utf8_getchar(iter, seqlen);
		if unlikely(ch32 <= 0x7f) {
			*iter = (uint8_t)ch32;
			memmovedownc(iter + 1,
			             iter + seqlen,
			             end - (iter + seqlen),
			             sizeof(char));
			++iter;
			continue;
		}
		if (ch32 > 0xffff) {
			/* Must decode into a 4-byte string */
			buffer32 = LOCAL_DeeDbgString(TryNew4ByteBuffer, length - (seqlen - 1));
			if unlikely(!buffer32)
				goto err_r;
			dst32         = buffer32;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst32++ = (uint32_t)(uint8_t)result->s_str[i];
			*dst32++ = ch32;
			iter += seqlen;
use_buffer32:
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst32++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					if (error_mode == STRING_ERROR_FREPLAC) {
						*iter++  = '?';
						*dst32++ = '?';
					} else {
						--end;
						memmovedownc(iter,
						             iter + 1,
						             end - iter,
						             sizeof(char));
					}
					continue;
err_buffer32:
					LOCAL_DeeDbgString(Free4ByteBuffer, buffer32);
					goto err_r;
				}
				ch32     = utf8_getchar(iter, seqlen);
				*dst32++ = ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst32 - buffer32);
			ASSERT(utf_length <= WSTR_LENGTH(buffer32));
			if (utf_length != WSTR_LENGTH(buffer32)) {
				uint32_t *new_buffer32;
				new_buffer32 = LOCAL_DeeDbgString(TryResize4ByteBuffer, buffer32, utf_length);
				if likely(new_buffer32)
					buffer32 = new_buffer32;
			}
			utf = Dee_string_utf_tryalloc();
			if unlikely(!utf)
				goto err_buffer32;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer32; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer32 - 1);
			utf->u_width = STRING_WIDTH_4BYTE;
			utf->u_utf8  = result->s_str;
		} else {
			uint16_t *buffer16, *dst16;
			/* Must decode into a 2/4-byte string */
			buffer16 = LOCAL_DeeDbgString(TryNew2ByteBuffer, length - (seqlen - 1));
			if unlikely(!buffer16)
				goto err_r;
			dst16         = buffer16;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst16++ = (uint16_t)(uint8_t)result->s_str[i];
			*dst16++ = (uint16_t)ch32;
			iter += seqlen;
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst16++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					if (error_mode == STRING_ERROR_FREPLAC) {
						*iter++  = '?';
						*dst16++ = '?';
					} else {
						--end;
						memmovedownc(iter,
						             iter + 1,
						             end - iter,
						             sizeof(char));
					}
					continue;
err_buffer16:
					LOCAL_DeeDbgString(Free2ByteBuffer, buffer16);
					goto err_r;
				}
				ch32 = utf8_getchar(iter, seqlen);
				if unlikely(ch32 > 0xffff) {
					iter += seqlen;
					/* Must upgrade to use a 4-byte buffer. */
					buffer32 = LOCAL_DeeDbgString(TryNew4ByteBuffer, (dst16 - buffer16) + 1 + (end - iter));
					if unlikely(!buffer32)
						goto err_buffer16;
					simple_length = (size_t)(dst16 - buffer16);
					for (i = 0; i < simple_length; ++i)
						buffer32[i] = buffer16[i];
					LOCAL_DeeDbgString(Free2ByteBuffer, buffer16);
					dst32    = buffer32 + simple_length;
					*dst32++ = ch32;
					goto use_buffer32;
				}
				*dst16++ = (uint16_t)ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst16 - buffer16);
			ASSERT(utf_length <= WSTR_LENGTH(buffer16));
			if (utf_length != WSTR_LENGTH(buffer16)) {
				uint16_t *new_buffer16;
				new_buffer16 = LOCAL_DeeDbgString(TryResize2ByteBuffer, buffer16, utf_length);
				if likely(new_buffer16)
					buffer16 = new_buffer16;
			}
			utf = Dee_string_utf_tryalloc();
			if unlikely(!utf)
				goto err_buffer16;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer16; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer16 - 1);
			utf->u_width = STRING_WIDTH_2BYTE;
			utf->u_utf8  = result->s_str;
		}
		break;
	}
	result->s_data = Dee_string_utf_untrack(utf);
	result->s_hash = DEE_STRING_HASH_UNSET;
	result->s_len  = length;
	result->s_str[length] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
#undef LOCAL_DeeDbgString
}


/* Given a string `self' that has previously been allocated as a byte-buffer
 * string (such as `DeeString_NewSized()' or `DeeString_NewBuffer()'), convert
 * it into a UTF-8 string, using the byte-buffer data as UTF-8 text.
 *
 * This function _always_ inherits a reference to `self', and will return
 * `NULL' on error. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeString_SetUtf8(/*inherit(always)*/ DREF DeeObject *__restrict self,
                  unsigned int error_mode) {
	DREF String *result;
	uint8_t *iter, *end;
	struct string_utf *utf = NULL;
	uint32_t *buffer32, *dst32;
	size_t i, simple_length, utf_length;
	result = (DREF String *)self;
	ASSERT(result->ob_type == &DeeString_Type);
	ASSERT(!DeeObject_IsShared(result));
	ASSERT(!result->s_data);
	/* Search for multi-byte character sequences. */
	end = (iter = (uint8_t *)result->s_str) + result->s_len;
	while (iter < end) {
		uint8_t seqlen, ch = *iter;
		uint32_t ch32;
		if (ch <= 0x7f) {
			++iter;
			continue;
		}
		seqlen = unicode_utf8seqlen[ch];
		if unlikely(!seqlen || iter + seqlen > end) {
			/* Invalid UTF-8 character */
			if (error_mode == STRING_ERROR_FREPLAC) {
				*iter = '?';
				++iter;
				continue;
			}
			if (error_mode == STRING_ERROR_FIGNORE) {
				--end;
				memmovedownc(iter,
				             iter + 1,
				             end - iter,
				             sizeof(char));
				continue;
			}
			DeeError_Throwf(&DeeError_UnicodeDecodeError,
			                "Invalid utf-8 character byte 0x%.2I8x",
			                ch);
			goto err_r;
		}
		ch32 = utf8_getchar(iter, seqlen);
		if unlikely(ch32 <= 0x7f) {
			*iter = (uint8_t)ch32;
			memmovedownc(iter + 1,
			             iter + seqlen,
			             end - (iter + seqlen),
			             sizeof(char));
			++iter;
			continue;
		}
		if (ch32 > 0xffff) {
			/* Must decode into a 4-byte string */
			buffer32 = DeeString_New4ByteBuffer(result->s_len - (seqlen - 1));
			if unlikely(!buffer32)
				goto err_r;
			dst32         = buffer32;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst32++ = (uint32_t)(uint8_t)result->s_str[i];
			*dst32++ = ch32;
			iter += seqlen;
use_buffer32:
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst32++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					if (error_mode == STRING_ERROR_FREPLAC) {
						*iter++  = '?';
						*dst32++ = '?';
						continue;
					}
					if (error_mode == STRING_ERROR_FIGNORE) {
						--end;
						memmovedownc(iter,
						             iter + 1,
						             end - iter,
						             sizeof(char));
						continue;
					}
					DeeError_Throwf(&DeeError_UnicodeDecodeError,
					                "Invalid utf-8 character byte 0x%.2I8x",
					                ch);
err_buffer32:
					DeeString_Free4ByteBuffer(buffer32);
					goto err_r;
				}
				ch32     = utf8_getchar(iter, seqlen);
				*dst32++ = ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst32 - buffer32);
			ASSERT(utf_length <= WSTR_LENGTH(buffer32));
			if (utf_length != WSTR_LENGTH(buffer32)) {
				uint32_t *new_buffer32;
				new_buffer32 = DeeString_TryResize4ByteBuffer(buffer32, utf_length);
				if likely(new_buffer32)
					buffer32 = new_buffer32;
			}
			utf = Dee_string_utf_alloc();
			if unlikely(!utf)
				goto err_buffer32;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer32; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer32 - 1);
			utf->u_width = STRING_WIDTH_4BYTE;
			utf->u_utf8  = result->s_str;
		} else {
			uint16_t *buffer16, *dst16;
			/* Must decode into a 2/4-byte string */
			buffer16 = DeeString_New2ByteBuffer(result->s_len - (seqlen - 1));
			if unlikely(!buffer16)
				goto err_r;
			dst16         = buffer16;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst16++ = (uint16_t)(uint8_t)result->s_str[i];
			*dst16++ = (uint16_t)ch32;
			iter += seqlen;
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst16++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					if (error_mode == STRING_ERROR_FREPLAC) {
						*iter++  = '?';
						*dst16++ = '?';
						continue;
					}
					if (error_mode == STRING_ERROR_FIGNORE) {
						--end;
						memmovedownc(iter,
						             iter + 1,
						             end - iter,
						             sizeof(char));
						continue;
					}
					DeeError_Throwf(&DeeError_UnicodeDecodeError,
					                "Invalid utf-8 character byte 0x%.2I8x",
					                ch);
err_buffer16:
					DeeString_Free2ByteBuffer(buffer16);
					goto err_r;
				}
				ch32 = utf8_getchar(iter, seqlen);
				if unlikely(ch32 > 0xffff) {
					iter += seqlen;
					/* Must upgrade to use a 4-byte buffer. */
					buffer32 = DeeString_New4ByteBuffer((dst16 - buffer16) + 1 + (end - iter));
					if unlikely(!buffer32)
						goto err_buffer16;
					simple_length = (size_t)(dst16 - buffer16);
					for (i = 0; i < simple_length; ++i)
						buffer32[i] = buffer16[i];
					DeeString_Free2ByteBuffer(buffer16);
					dst32    = buffer32 + simple_length;
					*dst32++ = ch32;
					goto use_buffer32;
				}
				*dst16++ = (uint16_t)ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst16 - buffer16);
			ASSERT(utf_length <= WSTR_LENGTH(buffer16));
			if (utf_length != WSTR_LENGTH(buffer16)) {
				uint16_t *new_buffer16;
				new_buffer16 = DeeString_TryResize2ByteBuffer(buffer16, utf_length);
				if likely(new_buffer16)
					buffer16 = new_buffer16;
			}
			utf = Dee_string_utf_alloc();
			if unlikely(!utf)
				goto err_buffer16;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer16; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer16 - 1);
			utf->u_width = STRING_WIDTH_2BYTE;
			utf->u_utf8  = result->s_str;
		}
		break;
	}
	ASSERT(result->ob_type == &DeeString_Type);
	ASSERT(!DeeObject_IsShared(result));
	ASSERT(!result->s_data);
	result->s_data = Dee_string_utf_untrack(utf);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FreeTracker((DeeObject *)result);
	DeeObject_Free(result);
	Dee_DecrefNokill(&DeeString_Type);
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeString_TrySetUtf8(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF String *result;
	uint8_t *iter, *end;
	struct string_utf *utf = NULL;
	uint32_t *buffer32, *dst32;
	size_t i, simple_length, utf_length;
	result = (DREF String *)self;
	ASSERT(result->ob_type == &DeeString_Type);
	ASSERT(!DeeObject_IsShared(result));
	ASSERT(!result->s_data);
	/* Search for multi-byte character sequences. */
	end = (iter = (uint8_t *)result->s_str) + result->s_len;
	while (iter < end) {
		uint8_t seqlen, ch = *iter;
		uint32_t ch32;
		if (ch <= 0x7f) {
			++iter;
			continue;
		}
		seqlen = unicode_utf8seqlen[ch];
		if unlikely(!seqlen || iter + seqlen > end) {
			/* Invalid UTF-8 character */
			--end;
			memmovedownc(iter,
			             iter + 1,
			             end - iter,
			             sizeof(char));
			continue;
		}
		ch32 = utf8_getchar(iter, seqlen);
		if unlikely(ch32 <= 0x7f) {
			*iter = (uint8_t)ch32;
			memmovedownc(iter + 1,
			             iter + seqlen,
			             end - (iter + seqlen),
			             sizeof(char));
			++iter;
			continue;
		}
		if (ch32 > 0xffff) {
			/* Must decode into a 4-byte string */
			buffer32 = DeeString_TryNew4ByteBuffer(result->s_len - (seqlen - 1));
			if unlikely(!buffer32)
				goto err_r;
			dst32         = buffer32;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst32++ = (uint32_t)(uint8_t)result->s_str[i];
			*dst32++ = ch32;
			iter += seqlen;
use_buffer32:
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst32++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					--end;
					memmovedownc(iter,
					             iter + 1,
					             (size_t)(end - iter),
					             sizeof(char));
					continue;
				}
				ch32     = utf8_getchar(iter, seqlen);
				*dst32++ = ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst32 - buffer32);
			ASSERT(utf_length <= WSTR_LENGTH(buffer32));
			if (utf_length != WSTR_LENGTH(buffer32)) {
				uint32_t *new_buffer32;
				new_buffer32 = DeeString_TryResize4ByteBuffer(buffer32, utf_length);
				if likely(new_buffer32)
					buffer32 = new_buffer32;
			}
			utf = Dee_string_utf_tryalloc();
			if unlikely(!utf) {
				DeeString_Free4ByteBuffer(buffer32);
				goto err_r;
			}
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer32; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer32 - 1);
			utf->u_width = STRING_WIDTH_4BYTE;
			utf->u_utf8  = result->s_str;
		} else {
			uint16_t *buffer16, *dst16;
			/* Must decode into a 2/4-byte string */
			buffer16 = DeeString_TryNew2ByteBuffer(result->s_len - (seqlen - 1));
			if unlikely(!buffer16)
				goto err_r;
			dst16         = buffer16;
			simple_length = iter - (uint8_t *)result->s_str;
			for (i = 0; i < simple_length; ++i)
				*dst16++ = (uint16_t)(uint8_t)result->s_str[i];
			*dst16++ = (uint16_t)ch32;
			iter += seqlen;
			while (iter < end) {
				ch = *iter;
				if (ch <= 0x7f) {
					++iter;
					*dst16++ = ch;
					continue;
				}
				seqlen = unicode_utf8seqlen[ch];
				if unlikely(!seqlen || iter + seqlen > end) {
					/* Invalid UTF-8 character */
					--end;
					memmovedownc(iter,
					             iter + 1,
					             end - iter,
					             sizeof(char));
					continue;
				}
				ch32 = utf8_getchar(iter, seqlen);
				if unlikely(ch32 > 0xffff) {
					iter += seqlen;
					/* Must upgrade to use a 4-byte buffer. */
					buffer32 = DeeString_TryNew4ByteBuffer((dst16 - buffer16) + 1 + (end - iter));
					if unlikely(!buffer32) {
err_buffer16:
						DeeString_Free2ByteBuffer(buffer16);
						goto err_r;
					}
					simple_length = (size_t)(dst16 - buffer16);
					for (i = 0; i < simple_length; ++i)
						buffer32[i] = buffer16[i];
					DeeString_Free2ByteBuffer(buffer16);
					dst32    = buffer32 + simple_length;
					*dst32++ = ch32;
					goto use_buffer32;
				}
				*dst16++ = (uint16_t)ch32;
				iter += seqlen;
			}
			utf_length = (size_t)(dst16 - buffer16);
			ASSERT(utf_length <= WSTR_LENGTH(buffer16));
			if (utf_length != WSTR_LENGTH(buffer16)) {
				uint16_t *new_buffer16;
				new_buffer16 = DeeString_TryResize2ByteBuffer(buffer16, utf_length);
				if likely(new_buffer16)
					buffer16 = new_buffer16;
			}
			utf = Dee_string_utf_tryalloc();
			if unlikely(!utf)
				goto err_buffer16;
			bzero(utf, sizeof(*utf));
			utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer16; /* Inherit data */
			(void)Dee_UntrackAlloc((size_t *)buffer16 - 1);
			utf->u_width = STRING_WIDTH_2BYTE;
			utf->u_utf8  = result->s_str;
		}
		break;
	}
	ASSERT(result->ob_type == &DeeString_Type);
	ASSERT(!DeeObject_IsShared(result));
	ASSERT(!result->s_data);
	result->s_data = Dee_string_utf_untrack(utf);
	return (DREF DeeObject *)result;
err_r:
	/*DeeObject_FreeTracker(result);*/
	/*DeeObject_Free(result);*/
	/*Dee_DecrefNokill(&DeeString_Type);*/
	return NULL;
}


PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_Chr16)(uint16_t ch) {
	uint16_t *buffer;
	if (ch <= 0xff)
		return DeeString_Chr8((uint8_t)ch);
	buffer = DeeString_New2ByteBuffer(1);
	if unlikely(!buffer)
		goto err;
	buffer[0] = ch;
	return DeeString_Pack2ByteBuffer(buffer);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_Chr32)(uint32_t ch) {
	if (ch <= 0xff)
		return DeeString_Chr8((uint8_t)ch);
	if (ch <= 0xffff) {
		uint16_t *buffer;
		buffer = DeeString_New2ByteBuffer(1);
		if unlikely(!buffer)
			goto err;
		buffer[0] = (uint16_t)ch;
		return DeeString_Pack2ByteBuffer(buffer);
	} else {
		uint32_t *buffer;
		buffer = DeeString_New4ByteBuffer(1);
		if unlikely(!buffer)
			goto err;
		buffer[0] = ch;
		return DeeString_Pack4ByteBuffer(buffer);
	}
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_Convert(String *__restrict self,
                  size_t start, size_t end,
                  uintptr_t kind) {
	unsigned int width;
	union dcharptr_const str;
	union dcharptr result;
	size_t i, length;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = DeeString_WSTR(self);
	length  = WSTR_LENGTH(str.ptr);
	if (start > length)
		start = length;
	if (end > length)
		end = length;
	if (start >= end)
		return_reference_((String *)Dee_EmptyString);
	end -= start;
	result.ptr = DeeString_NewWidthBuffer(end, width);
	if unlikely(!result.ptr)
		goto err;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		for (i = 0; i < end; ++i)
			result.cp8[i] = (uint8_t)DeeUni_Convert(str.cp8[start + i], kind);
		break;

	CASE_WIDTH_2BYTE:
		for (i = 0; i < end; ++i)
			result.cp16[i] = (uint16_t)DeeUni_Convert(str.cp16[start + i], kind);
		break;

	CASE_WIDTH_4BYTE:
		for (i = 0; i < end; ++i)
			result.cp32[i] = (uint32_t)DeeUni_Convert(str.cp32[start + i], kind);
		break;
	}
	return (DREF String *)DeeString_PackWidthBuffer(result.ptr, width);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_ToTitle(String *__restrict self, size_t start, size_t end) {
	uintptr_t kind = UNICODE_CONVERT_TITLE;
	unsigned int width;
	union dcharptr_const str;
	union dcharptr result;
	size_t i, length;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = DeeString_WSTR(self);
	length  = WSTR_LENGTH(str.ptr);
	if (start > length)
		start = length;
	if (end > length)
		end = length;
	if (start >= end)
		return_reference_((String *)Dee_EmptyString);
	end -= start;
	result.ptr = DeeString_NewWidthBuffer(end, width);
	if unlikely(!result.ptr)
		goto err;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		for (i = 0; i < end; ++i) {
			uint8_t ch = str.cp8[start + i];
			struct unitraits const *desc = DeeUni_Descriptor(ch);
			result.cp8[i] = (uint8_t)(ch + *(int32_t const *)((byte_t const *)desc + kind));
			kind = (desc->ut_flags & UNICODE_ISSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
		}
		break;

	CASE_WIDTH_2BYTE:
		for (i = 0; i < end; ++i) {
			uint16_t ch = str.cp16[start + i];
			struct unitraits const *desc  = DeeUni_Descriptor(ch);
			result.cp16[i] = (uint16_t)(ch + *(int32_t const *)((byte_t const *)desc + kind));
			kind = (desc->ut_flags & UNICODE_ISSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
		}
		break;

	CASE_WIDTH_4BYTE:
		for (i = 0; i < end; ++i) {
			uint32_t ch = str.cp32[start + i];
			struct unitraits const *desc  = DeeUni_Descriptor(ch);
			result.cp32[i] = (uint32_t)(ch + *(int32_t const *)((byte_t const *)desc + kind));
			kind = (desc->ut_flags & UNICODE_ISSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
		}
		break;
	}
	return (DREF String *)DeeString_PackWidthBuffer(result.ptr, width);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_Capitalize(String *__restrict self, size_t start, size_t end) {
	unsigned int width;
	union dcharptr_const str;
	union dcharptr result;
	size_t i, length;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = DeeString_WSTR(self);
	length  = WSTR_LENGTH(str.ptr);
	if (start > length)
		start = length;
	if (end > length)
		end = length;
	if (start >= end)
		return_reference_((String *)Dee_EmptyString);
	end -= start;
	result.ptr = DeeString_NewWidthBuffer(end, width);
	if unlikely(!result.ptr)
		goto err;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		result.cp8[0] = (uint8_t)DeeUni_ToUpper(str.cp8[start]);
		for (i = 1; i < end; ++i)
			result.cp8[i] = (uint8_t)DeeUni_ToLower(str.cp8[start + i]);
		break;

	CASE_WIDTH_2BYTE:
		result.cp16[0] = (uint16_t)DeeUni_ToUpper(str.cp16[start]);
		for (i = 1; i < end; ++i)
			result.cp16[i] = (uint16_t)DeeUni_ToLower(str.cp16[start + i]);
		break;

	CASE_WIDTH_4BYTE:
		result.cp32[0] = (uint32_t)DeeUni_ToUpper(str.cp32[start]);
		for (i = 1; i < end; ++i)
			result.cp32[i] = (uint32_t)DeeUni_ToLower(str.cp32[start + i]);
		break;
	}
	return (DREF String *)DeeString_PackWidthBuffer(result.ptr, width);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_Swapcase(String *__restrict self, size_t start, size_t end) {
	unsigned int width;
	union dcharptr_const str;
	union dcharptr result;
	size_t i, length;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = DeeString_WSTR(self);
	length  = WSTR_LENGTH(str.ptr);
	if (start > length)
		start = length;
	if (end > length)
		end = length;
	if (start >= end)
		return_reference_((String *)Dee_EmptyString);
	end -= start;
	result.ptr = DeeString_NewWidthBuffer(end, width);
	if unlikely(!result.ptr)
		goto err;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		for (i = 0; i < end; ++i)
			result.cp8[i] = (uint8_t)DeeUni_SwapCase(str.cp8[start + i]);
		break;

	CASE_WIDTH_2BYTE:
		for (i = 0; i < end; ++i)
			result.cp16[i] = (uint16_t)DeeUni_SwapCase(str.cp16[start + i]);
		break;

	CASE_WIDTH_4BYTE:
		for (i = 0; i < end; ++i)
			result.cp32[i] = (uint32_t)DeeUni_SwapCase(str.cp32[start + i]);
		break;
	}
	return (DREF String *)DeeString_PackWidthBuffer(result.ptr, width);
err:
	return NULL;
}




PUBLIC WUNUSED ATTR_INOUT(1) uint32_t
(DCALL Dee_unicode_readutf8)(char const **__restrict ptext) {
	char const *iter = *ptext;
	uint32_t result = (uint32_t)(uint8_t)*iter++;
	if (result >= 0xc0) {
		switch (unicode_utf8seqlen[result]) {

		case 0:
		case 1:
			break;

		case 2:
			result  = (result & 0x1f) << 6;
			result |= (iter[0] & 0x3f);
			iter += 1;
			break;

		case 3:
			result  = (result & 0x0f) << 12;
			result |= (iter[0] & 0x3f) << 6;
			result |= (iter[1] & 0x3f);
			iter += 2;
			break;

		case 4:
			result  = (result & 0x07) << 18;
			result |= (iter[0] & 0x3f) << 12;
			result |= (iter[1] & 0x3f) << 6;
			result |= (iter[2] & 0x3f);
			iter += 3;
			break;

		case 5:
			result  = (result & 0x03) << 24;
			result |= (iter[0] & 0x3f) << 18;
			result |= (iter[1] & 0x3f) << 12;
			result |= (iter[2] & 0x3f) << 6;
			result |= (iter[3] & 0x3f);
			iter += 4;
			break;

		case 6:
			result  = (result & 0x01) << 30;
			result |= (iter[0] & 0x3f) << 24;
			result |= (iter[1] & 0x3f) << 18;
			result |= (iter[2] & 0x3f) << 12;
			result |= (iter[3] & 0x3f) << 6;
			result |= (iter[4] & 0x3f);
			iter += 5;
			break;

		case 7:
			result  = (iter[0] & 0x03/*0x3f*/) << 30;
			result |= (iter[1] & 0x3f) << 24;
			result |= (iter[2] & 0x3f) << 18;
			result |= (iter[3] & 0x3f) << 12;
			result |= (iter[4] & 0x3f) << 6;
			result |= (iter[5] & 0x3f);
			iter += 6;
			break;

		case 8:
			/*result = (iter[0] & 0x3f) << 36;*/
			result  = (iter[1] & 0x03/*0x3f*/) << 30;
			result |= (iter[2] & 0x3f) << 24;
			result |= (iter[3] & 0x3f) << 18;
			result |= (iter[4] & 0x3f) << 12;
			result |= (iter[5] & 0x3f) << 6;
			result |= (iter[6] & 0x3f);
			iter += 7;
			break;

		default:
			__builtin_unreachable();
		}
	}
	*ptext = iter;
	return result;
}

PUBLIC ATTR_INOUT(1) NONNULL((2)) uint32_t
(DCALL Dee_unicode_readutf8_n)(char const **__restrict ptext,
                               char const *text_end) {
	uint32_t result;
	char const *iter = *ptext;
	if unlikely(iter >= text_end)
		return 0;
	result = (uint32_t)(uint8_t)*iter++;
	if (result >= 0xc0) {
		uint8_t len = unicode_utf8seqlen[result];
		if (iter + len - 1 >= text_end)
			len = (uint8_t)(text_end - iter)+1;
		switch (len) {

		case 0:
		case 1:
			break;

		case 2:
			result  = (result & 0x1f) << 6;
			result |= (iter[0] & 0x3f);
			iter += 1;
			break;

		case 3:
			result  = (result & 0x0f) << 12;
			result |= (iter[0] & 0x3f) << 6;
			result |= (iter[1] & 0x3f);
			iter += 2;
			break;

		case 4:
			result  = (result & 0x07) << 18;
			result |= (iter[0] & 0x3f) << 12;
			result |= (iter[1] & 0x3f) << 6;
			result |= (iter[2] & 0x3f);
			iter += 3;
			break;

		case 5:
			result  = (result & 0x03) << 24;
			result |= (iter[0] & 0x3f) << 18;
			result |= (iter[1] & 0x3f) << 12;
			result |= (iter[2] & 0x3f) << 6;
			result |= (iter[3] & 0x3f);
			iter += 4;
			break;

		case 6:
			result  = (result & 0x01) << 30;
			result |= (iter[0] & 0x3f) << 24;
			result |= (iter[1] & 0x3f) << 18;
			result |= (iter[2] & 0x3f) << 12;
			result |= (iter[3] & 0x3f) << 6;
			result |= (iter[4] & 0x3f);
			iter += 5;
			break;

		case 7:
			result  = (iter[0] & 0x03/*0x3f*/) << 30;
			result |= (iter[1] & 0x3f) << 24;
			result |= (iter[2] & 0x3f) << 18;
			result |= (iter[3] & 0x3f) << 12;
			result |= (iter[4] & 0x3f) << 6;
			result |= (iter[5] & 0x3f);
			iter += 6;
			break;

		case 8:
			/*result = (iter[0] & 0x3f) << 36;*/
			result  = (iter[1] & 0x03/*0x3f*/) << 30;
			result |= (iter[2] & 0x3f) << 24;
			result |= (iter[3] & 0x3f) << 18;
			result |= (iter[4] & 0x3f) << 12;
			result |= (iter[5] & 0x3f) << 6;
			result |= (iter[6] & 0x3f);
			iter += 7;
			break;

		default:
			__builtin_unreachable();
		}
	}
	*ptext = iter;
	return result;
}

PUBLIC ATTR_INOUT(1) uint32_t
(DCALL Dee_unicode_readutf8_rev_n)(char const **__restrict ptext,
                                   char const *text_start) {
	uint32_t result;
	char const *iter = *ptext;
	uint8_t seqlen = 1;
	if unlikely(iter <= text_start)
		return 0;
	for (;;) {
		result = (unsigned char)*--iter;
		if ((result & 0xc0) != 0x80)
			break;
		if (seqlen >= 8)
			break;
		++seqlen;
		if (iter <= text_start)
			break;
	}
	if (result >= 0xc0) {
		switch (seqlen) {

		case 0:
		case 1:
			break;

		case 2:
			result  = (result & 0x1f) << 6;
			result |= (iter[0] & 0x3f);
			break;

		case 3:
			result  = (result & 0x0f) << 12;
			result |= (iter[0] & 0x3f) << 6;
			result |= (iter[1] & 0x3f);
			break;

		case 4:
			result  = (result & 0x07) << 18;
			result |= (iter[0] & 0x3f) << 12;
			result |= (iter[1] & 0x3f) << 6;
			result |= (iter[2] & 0x3f);
			break;

		case 5:
			result  = (result & 0x03) << 24;
			result |= (iter[0] & 0x3f) << 18;
			result |= (iter[1] & 0x3f) << 12;
			result |= (iter[2] & 0x3f) << 6;
			result |= (iter[3] & 0x3f);
			break;

		case 6:
			result  = (result & 0x01) << 30;
			result |= (iter[0] & 0x3f) << 24;
			result |= (iter[1] & 0x3f) << 18;
			result |= (iter[2] & 0x3f) << 12;
			result |= (iter[3] & 0x3f) << 6;
			result |= (iter[4] & 0x3f);
			break;

		case 7:
			result  = (iter[0] & 0x03/*0x3f*/) << 30;
			result |= (iter[1] & 0x3f) << 24;
			result |= (iter[2] & 0x3f) << 18;
			result |= (iter[3] & 0x3f) << 12;
			result |= (iter[4] & 0x3f) << 6;
			result |= (iter[5] & 0x3f);
			break;

		case 8:
			/*result = (iter[0] & 0x3f) << 36;*/
			result  = (iter[1] & 0x03/*0x3f*/) << 30;
			result |= (iter[2] & 0x3f) << 24;
			result |= (iter[3] & 0x3f) << 18;
			result |= (iter[4] & 0x3f) << 12;
			result |= (iter[5] & 0x3f) << 6;
			result |= (iter[6] & 0x3f);
			break;

		default:
			__builtin_unreachable();
		}
	}
	*ptext = iter;
	return result;
}



PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
(DCALL Dee_unicode_writeutf8)(char *__restrict buffer, uint32_t ch) {
	uint8_t *dst = (uint8_t *)buffer;
	if (ch <= UTF8_1BYTE_MAX) {
		*dst++ = (uint8_t)ch;
	} else if (ch <= UTF8_2BYTE_MAX) {
		*dst++ = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
		*dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
	} else if (ch <= UTF8_3BYTE_MAX) {
		*dst++ = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
		*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
	} else if (ch <= UTF8_4BYTE_MAX) {
		*dst++ = 0xf0 | (uint8_t)((ch >> 18) /* & 0x07*/);
		*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
	} else if (ch <= UTF8_5BYTE_MAX) {
		*dst++ = 0xf8 | (uint8_t)((ch >> 24) /* & 0x03*/);
		*dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
	} else if (ch <= UTF8_6BYTE_MAX) {
		*dst++ = 0xfc | (uint8_t)((ch >> 30) /* & 0x01*/);
		*dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
	} else {
		STATIC_ASSERT(UNICODE_UTF8_CURLEN >= 7);
		*dst++ = 0xfe;
		*dst++ = 0x80 | (uint8_t)((ch >> 30) & 0x03 /* & 0x3f*/);
		*dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
		*dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
	}
	return (char *)dst;
}





#undef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
#define CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION

#ifndef UNICODE_PRINTER_INITIAL_BUFSIZE
#define UNICODE_PRINTER_INITIAL_BUFSIZE 64
#endif /* !UNICODE_PRINTER_INITIAL_BUFSIZE */


/* Unicode printer API */
PUBLIC NONNULL((1)) bool DCALL
Dee_unicode_printer_allocate(struct unicode_printer *__restrict self,
                             size_t num_chars, unsigned int width) {
	ASSERT(!(width & ~UNICODE_PRINTER_FWIDTH));
	if unlikely(!num_chars)
		goto done;
	if (!self->up_buffer) {
		/* Allocate an initial buffer. */
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if likely(!self->up_length) {
			self->up_length = num_chars;
			self->up_flags |= (unsigned char)width;
		} else {
			width = (unsigned int)STRING_WIDTH_COMMON((unsigned int)(self->up_flags &
			                                                         UNICODE_PRINTER_FWIDTH),
			                                          width);
			self->up_length += num_chars;
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
			self->up_flags |= width;
		}
#else /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		ASSERT(!self->up_length);
		self->up_buffer = DeeString_TryNewWidthBuffer(num_chars, width);
		if (!self->up_buffer)
			goto err;
		self->up_flags = (unsigned char)width;
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
	} else {
		/* Check if sufficient memory is already available. */
		union dcharptr buffer;
		size_t avail, required;
		buffer.ptr = self->up_buffer;
		avail      = WSTR_LENGTH(buffer.ptr);
		required   = self->up_length + num_chars;
		ASSERT(self->up_length <= avail);
		if (avail < required) {
			/* Try to pre-allocate memory by extending upon what has already been written. */
			SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

			CASE_WIDTH_1BYTE:
				if (width == STRING_WIDTH_1BYTE) {
					DeeStringObject *new_string;
					new_string = (DeeStringObject *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(buffer.ptr, DeeStringObject, s_str),
					                                                      offsetof(DeeStringObject, s_str), required + 1, sizeof(char));
					if unlikely(!new_string)
						goto err;
					new_string->s_len = required;
					self->up_buffer   = new_string->s_str;
				} else if (width == STRING_WIDTH_2BYTE) {
					/* Upgrade to a 2-byte string. */
					uint16_t *new_buffer;
					size_t i;
					new_buffer = DeeString_TryNew2ByteBuffer(required);
					if unlikely(!new_buffer)
						goto err;
					for (i = 0; i < self->up_length; ++i)
						new_buffer[i] = (uint16_t)buffer.cp8[i];
					DeeObject_Free(COMPILER_CONTAINER_OF(buffer.ptr,
					                                     DeeStringObject,
					                                     s_str));
					self->up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
					self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
					self->up_flags |= STRING_WIDTH_2BYTE;
				} else {
					/* Upgrade to a 4-byte string. */
					uint32_t *new_buffer;
					size_t i;
					new_buffer = DeeString_TryNew4ByteBuffer(required);
					if unlikely(!new_buffer)
						goto err;
					for (i = 0; i < self->up_length; ++i)
						new_buffer[i] = (uint32_t)buffer.cp8[i];
					DeeObject_Free(COMPILER_CONTAINER_OF(buffer.ptr,
					                                     DeeStringObject,
					                                     s_str));
					self->up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
					self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
					self->up_flags |= STRING_WIDTH_4BYTE;
				}
				break;

			CASE_WIDTH_2BYTE:
				if (width <= STRING_WIDTH_2BYTE) {
					uint16_t *new_buffer;
					/* Extend the 2-byte buffer. */
					new_buffer = DeeString_TryResize2ByteBuffer(buffer.cp16, required);
					if unlikely(!new_buffer)
						goto err;
					self->up_buffer = new_buffer;
				} else {
					/* Upgrade to a 4-byte buffer. */
					uint32_t *new_buffer;
					size_t i;
					new_buffer = DeeString_TryNew4ByteBuffer(required);
					if unlikely(!new_buffer)
						goto err;
					for (i = 0; i < self->up_length; ++i)
						new_buffer[i] = (uint32_t)buffer.cp16[i];
					DeeString_Free2ByteBuffer(buffer.cp16);
					self->up_buffer = new_buffer;
					self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
					self->up_flags |= STRING_WIDTH_4BYTE;
				}
				break;

			CASE_WIDTH_4BYTE: {
				uint32_t *new_buffer;
				/* Extend the 4-byte buffer. */
				new_buffer = DeeString_TryResize4ByteBuffer(buffer.cp32, required);
				if unlikely(!new_buffer)
					goto err;
				self->up_buffer = new_buffer;
			}	break;

			}
		}
	}
done:
	return true;
err:
	return false;
}


/* Initialize a unicode printer from a given `string'
 * The caller must ensure that `!DeeObject_IsShared(string) || string == Dee_EmptyString' */
PUBLIC NONNULL((1, 2)) void DCALL
Dee_unicode_printer_init_string(struct Dee_unicode_printer *__restrict self,
                                /*inherit(always)*/ DREF DeeObject *__restrict string) {
	String *str = (String *)string;
	struct Dee_string_utf *utf;
	ASSERT_OBJECT_TYPE_EXACT(str, &DeeString_Type);

	/* Check for special case: caller is giving using the empty string. */
	if unlikely(str == (String *)Dee_EmptyString) {
		unicode_printer_init(self);
		Dee_DecrefNokill(str);
		return;
	}

	/* Ensure that the string isn't being shared. */
	ASSERT(!DeeObject_IsShared(str));

	/* Check how the string is being encoded. */
	utf = str->s_data;
	if (!utf) {
		/* LATIN-1/ASCII string. */
inherit_latin1_string:
		DBG_memset(str, 0xcc, offsetof(String, s_len));
		self->up_length = str->s_len;
		self->up_buffer = str->s_str; /* This inherits the entire string. */
		self->up_flags  = STRING_WIDTH_1BYTE;
	} else {
		/* Extended-width string */
		ASSERT(utf->u_width == STRING_WIDTH_1BYTE ||
		       utf->u_width == STRING_WIDTH_2BYTE ||
		       utf->u_width == STRING_WIDTH_4BYTE);

		/* Check if it's a latin-1 string (s.a. `DeeString_STR_ISLATIN1()') */
		if ((utf->u_width == STRING_WIDTH_1BYTE) ||
		    (utf->u_flags & Dee_STRING_UTF_FASCII)) {
			/* LATIN-1 string. */
			Dee_string_utf_fini(utf, str);
			Dee_string_utf_free(utf);
			goto inherit_latin1_string;
		}

		/* It's an extended-width string */
		if ((uint8_t *)utf->u_utf8 != (uint8_t *)str->s_str)
			Dee_Free(utf->u_utf8);
		if ((uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[Dee_STRING_WIDTH_2BYTE])
			Dee_Free(utf->u_utf16);
		Dee_Free(utf->u_data[Dee_STRING_WIDTH_1BYTE]);

		/* Extract the actually used string. */
		self->up_flags = (unsigned char)utf->u_width;
		if (utf->u_width == STRING_WIDTH_2BYTE) {
			self->up_buffer = utf->u_data[STRING_WIDTH_2BYTE];
			Dee_Free(utf->u_data[STRING_WIDTH_4BYTE]);
		} else {
			self->up_buffer = utf->u_data[STRING_WIDTH_4BYTE];
			Dee_Free(utf->u_data[STRING_WIDTH_2BYTE]);
		}
		self->up_length = WSTR_LENGTH(self->up_buffer);
		Dee_string_utf_free(utf);
		DeeObject_Free(str);
	}
	Dee_DecrefNokill(&DeeString_Type);
}


/* _Always_ inherit all string data (even upon error) saved in
 * `self', and construct a new string from all that data, before
 * returning a reference to that string.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `unicode_printer_fini()'
 * @return: * :   A reference to the packed string.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_unicode_printer_pack(/*inherit(always)*/ struct unicode_printer *__restrict self) {
	void *result_buffer = self->up_buffer;
	if unlikely(!result_buffer)
		return DeeString_NewEmpty();
	if (self->up_length < WSTR_LENGTH(result_buffer)) {
		void *new_buffer;
		new_buffer = DeeString_TryResizeWidthBuffer(result_buffer,
		                                            self->up_length,
		                                            UNICODE_PRINTER_WIDTH(self));
		if likely(new_buffer)
			result_buffer = new_buffer;
	}
	{
		unsigned char flags = self->up_flags;
		DBG_memset(self, 0xcc, sizeof(*self));
		return DeeString_PackWidthBuffer(result_buffer,
		                                 flags & UNICODE_PRINTER_FWIDTH);
	}
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_unicode_printer_trypack(/*inherit(on_success)*/ struct unicode_printer *__restrict self) {
	void *result_buffer = self->up_buffer;
	if unlikely(!result_buffer)
		return DeeString_NewEmpty();
	ASSERT(self->up_length <= WSTR_LENGTH(result_buffer));
	if (self->up_length < WSTR_LENGTH(result_buffer)) {
		void *new_buffer;
		new_buffer = DeeString_TryResizeWidthBuffer(result_buffer,
		                                            self->up_length,
		                                            UNICODE_PRINTER_WIDTH(self));
		if likely(new_buffer)
			result_buffer = new_buffer;
	}
#ifdef NDEBUG
	return DeeString_TryPackWidthBuffer(result_buffer,
	                                    UNICODE_PRINTER_WIDTH(self));
#else /* NDEBUG */
	{
		DREF DeeObject *result;
		result = DeeString_TryPackWidthBuffer(result_buffer,
		                                      UNICODE_PRINTER_WIDTH(self));
		if likely(result)
			DBG_memset(self, 0xcc, sizeof(*self));
		return result;
	}
#endif /* !NDEBUG */
}


LOCAL WUNUSED NONNULL((1)) int
(DCALL unicode_printer_putc8)(struct unicode_printer *__restrict self,
                              uint8_t ch) {
	size_t size_avail;
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		/* Allocate the initial buffer. */
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (self->up_length > UNICODE_PRINTER_INITIAL_BUFSIZE ||
		    (UNICODE_PRINTER_WIDTH(self)) != STRING_WIDTH_1BYTE) {
			self->up_buffer = DeeString_TryNewWidthBuffer(self->up_length,
			                                              UNICODE_PRINTER_WIDTH(self));
			if (!self->up_buffer)
				goto allocate_initial_normally;
			self->up_length = 1;
			UNICODE_PRINTER_SETCHAR(self, 0, ch);
			goto done;
		} else {
			DeeStringObject *buffer;
allocate_initial_normally:
			ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
			buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
			                                                 UNICODE_PRINTER_INITIAL_BUFSIZE + 1,
			                                                 sizeof(char));
			if likely(buffer) {
				buffer->s_len = UNICODE_PRINTER_INITIAL_BUFSIZE;
			} else {
				buffer = (DeeStringObject *)DeeObject_Mallocc(offsetof(DeeStringObject, s_str),
				                                              1 + 1, sizeof(char));
				if unlikely(!buffer)
					goto err;
				buffer->s_len = 1;
			}
			buffer->s_str[0] = (char)ch;
			self->up_buffer  = buffer->s_str;
			self->up_length  = 1;
			goto done;
		}
#else /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		DeeStringObject *buffer;
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
		buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                 UNICODE_PRINTER_INITIAL_BUFSIZE + 1,
		                                                 sizeof(char));
		if likely(buffer) {
			buffer->s_len = UNICODE_PRINTER_INITIAL_BUFSIZE;
		} else {
			buffer = (DeeStringObject *)DeeObject_Mallocc(offsetof(DeeStringObject, s_str),
			                                              1 + 1, sizeof(char));
			if unlikely(!buffer)
				goto err;
			buffer->s_len = 1;
		}
		buffer->s_str[0] = (char)ch;
		self->up_buffer = buffer->s_str;
		self->up_length = 1;
		goto done;
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
	}
	size_avail = WSTR_LENGTH(string.ptr);
	ASSERT(size_avail >= self->up_length);
	if unlikely(size_avail == self->up_length) {
		/* Must allocate more memory. */
		string.ptr = DeeString_TryResizeWidthBuffer(string.ptr,
		                                            size_avail * 2,
		                                            UNICODE_PRINTER_WIDTH(self));
		if unlikely(!string.ptr) {
			string.ptr = DeeString_ResizeWidthBuffer(self->up_buffer,
			                                         size_avail + 1,
			                                         UNICODE_PRINTER_WIDTH(self));
			if unlikely(!string.ptr)
				goto err;
		}
		self->up_buffer = string.ptr;
	}

	/* Simply append the new character. */
	SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		string.cp8[self->up_length] = ch;
		break;

	CASE_WIDTH_2BYTE:
		string.cp16[self->up_length] = ch;
		break;

	CASE_WIDTH_4BYTE:
		string.cp32[self->up_length] = ch;
		break;
	}
	++self->up_length;
done:
	return 0;
err:
	return -1;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) uint16_t *DCALL
cast_8to16(DeeStringObject *__restrict buffer, size_t used_length) {
	uint16_t *result;
	size_t i, length = buffer->s_len;
	ASSERT(used_length <= length);
	result = DeeString_New2ByteBuffer(length);
	if unlikely(!result)
		goto done;
	for (i = 0; i < used_length; ++i)
		result[i] = (uint16_t)((uint8_t const *)buffer->s_str)[i];
	DeeObject_Free(buffer);
done:
	return result;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) uint32_t *DCALL
cast_8to32(DeeStringObject *__restrict buffer, size_t used_length) {
	uint32_t *result;
	size_t i, length = buffer->s_len;
	ASSERT(used_length <= length);
	result = DeeString_New4ByteBuffer(length);
	if unlikely(!result)
		goto done;
	for (i = 0; i < used_length; ++i)
		result[i] = (uint32_t)((uint8_t const *)buffer->s_str)[i];
	DeeObject_Free(buffer);
done:
	return result;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) uint32_t *DCALL
cast_16to32(uint16_t *__restrict buffer, size_t used_length) {
	uint32_t *result;
	size_t i, length = WSTR_LENGTH(buffer);
	ASSERT(used_length <= length);
	result = DeeString_New4ByteBuffer(length);
	if unlikely(!result)
		goto done;
	for (i = 0; i < used_length; ++i)
		result[i] = (uint32_t)buffer[i];
	Dee_Free((size_t *)buffer - 1);
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_putascii)(struct unicode_printer *__restrict self, char ch) {
	ASSERTF((uint8_t)ch <= 0x7f,
	        "The given ch (U+%.4I8x) is not an ASCII character",
	        (uint8_t)ch);
	return unicode_printer_putc8(self, (uint8_t)ch);
}


/* Append a single character to the given printer.
 * If `ch' can't fit the currently set `up_width', copy already
 * written data into a larger representation before appending `ch'.
 * @return:  0: Successfully appended the character.
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_putc)(struct unicode_printer *__restrict self, uint32_t ch) {
	union dcharptr string;
	if (ch <= 0xff)
		return unicode_printer_putc8(self, (uint8_t)ch);
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		/* Allocate the initial buffer. */
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		size_t initial_length;
		initial_length = self->up_length;
		if (initial_length < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_length = UNICODE_PRINTER_INITIAL_BUFSIZE;
		if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_4BYTE)
			goto allocate_initial_as_32;
		if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_2BYTE && ch <= 0xffff)
			goto allocate_initial_as_16;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
		if (ch <= 0xffff) {
allocate_initial_as_16:
			string.cp16 = DeeString_TryNew2ByteBuffer(initial_length);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_TryNew2ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
				if unlikely(!string.cp16) {
					string.cp16 = DeeString_New2ByteBuffer(1);
					if unlikely(!string.cp16)
						goto err;
				}
			}
			string.cp16[0] = (uint16_t)ch;
			self->up_flags |= STRING_WIDTH_2BYTE;
		} else {
allocate_initial_as_32:
			string.cp32 = DeeString_TryNew4ByteBuffer(initial_length);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_TryNew4ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
				if unlikely(!string.cp32) {
					string.cp32 = DeeString_New4ByteBuffer(1);
					if unlikely(!string.cp32)
						goto err;
				}
			}
			string.cp32[0] = ch;
			self->up_flags |= STRING_WIDTH_4BYTE;
		}
		self->up_length = 1;
		self->up_buffer = string.ptr;
		goto done;
#else /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
		if (ch <= 0xffff) {
			string.cp16 = DeeString_TryNew2ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_New2ByteBuffer(1);
				if unlikely(!string.cp16)
					goto err;
			}
			string.cp16[0] = (uint16_t)ch;
			self->up_flags |= STRING_WIDTH_2BYTE;
		} else {
			string.cp32 = DeeString_TryNew4ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_New4ByteBuffer(1);
				if unlikely(!string.cp32)
					goto err;
			}
			string.cp32[0] = ch;
			self->up_flags |= STRING_WIDTH_4BYTE;
		}
		self->up_length = 1;
		self->up_buffer = string.ptr;
		goto done;
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
	}
	ASSERT(self->up_length <= WSTR_LENGTH(string.ptr));
	SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		/* Must always upcast, regardless of where we're at. */
		if (ch <= 0xffff) {
			string.cp16 = cast_8to16(COMPILER_CONTAINER_OF(string.cp_char, DeeStringObject, s_str),
			                         self->up_length);
			if unlikely(!string.cp16)
				goto err;
			self->up_buffer = string.ptr;
#if STRING_WIDTH_1BYTE != 0
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->up_flags |= STRING_WIDTH_2BYTE;
			goto append_2byte;
		} else {
			string.cp32 = cast_8to32(COMPILER_CONTAINER_OF(string.cp_char, DeeStringObject, s_str),
			                         self->up_length);
			if unlikely(!string.cp32)
				goto err;
			self->up_buffer = string.ptr;
#if STRING_WIDTH_1BYTE != 0
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->up_flags |= STRING_WIDTH_4BYTE;
			goto append_4byte;
		}
		break;

	CASE_WIDTH_2BYTE:
		if (ch <= 0xffff) {
			/* No need to cast. */
append_2byte:
			ASSERT(self->up_length <= WSTR_LENGTH(string.cp16));
			if (self->up_length == WSTR_LENGTH(string.cp16)) {
				/* Must allocate more memory. */
				string.cp16 = DeeString_TryResize2ByteBuffer(string.cp16, WSTR_LENGTH(string.cp16) * 2);
				if unlikely(!string.cp16) {
					string.cp16 = DeeString_Resize2ByteBuffer((uint16_t *)self->up_buffer,
					                                          self->up_length + 1);
					if unlikely(!string.cp16)
						goto err;
				}
				self->up_buffer = string.cp16;
			}
			string.cp16[self->up_length] = (uint16_t)ch;
		} else {
			/* Must cast from 2 -> 4 bytes. */
			string.cp32 = cast_16to32(string.cp16, self->up_length);
			if unlikely(!string.cp32)
				goto err;
			self->up_buffer = string.cp32;
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
			self->up_flags |= STRING_WIDTH_4BYTE;
			goto append_4byte;
		}
		break;

	CASE_WIDTH_4BYTE:
append_4byte:
		/* We're already at max-size, so just append. */
		ASSERT(self->up_length <= WSTR_LENGTH(string.cp32));
		if (self->up_length == WSTR_LENGTH(string.cp32)) {
			/* Must allocate more memory. */
			string.cp32 = DeeString_TryResize4ByteBuffer((uint32_t *)string.cp32,
			                                             WSTR_LENGTH(string.cp32) * 2);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_Resize4ByteBuffer((uint32_t *)self->up_buffer,
				                                          self->up_length + 1);
				if unlikely(!string.cp32)
					goto err;
			}
			self->up_buffer = string.cp32;
		}
		string.cp32[self->up_length] = ch;
		break;
	}
	++self->up_length;
done:
	return 0;
err:
	return -1;
}

/* Append a given UTF-8 character. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_pututf8)(struct unicode_printer *__restrict self, uint8_t ch) {
	if (self->up_flags & UNICODE_PRINTER_FPENDING) {
		/* Complete a pending UTF-8 multi-byte sequence. */
		uint8_t curlen, reqlen;
		curlen = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		self->up_pend[curlen] = ch;
		reqlen = unicode_utf8seqlen[self->up_pend[0]];
		ASSERT(curlen + 1 <= reqlen);
		if (curlen + 1 == reqlen) {
			/* Append the full character. */
			int result;
			uint32_t ch32 = utf8_getchar((uint8_t *)self->up_pend, reqlen);
			result        = unicode_printer_putc(self, ch32);
			if likely(result >= 0)
				self->up_flags &= ~UNICODE_PRINTER_FPENDING;
			return result;
		}
		self->up_flags += 1 << UNICODE_PRINTER_FPENDING_SHFT;
		return 0;
	}
	if (ch >= 0xc0) {
		/* Start of a multi-byte sequence. */
		self->up_pend[0] = ch;
		self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
		return 0;
	}
	return unicode_printer_putc8(self, ch);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_pututf16)(struct unicode_printer *__restrict self, uint16_t ch) {
	if (self->up_flags & UNICODE_PRINTER_FPENDING) {
		uint32_t ch32;
		/* Complete a utf-16 surrogate pair. */
		ch32 = UTF16_COMBINE_SURROGATES(*(uint16_t *)self->up_pend, ch);
		self->up_flags &= ~UNICODE_PRINTER_FPENDING;
		return unicode_printer_putc(self, ch32);
	}
	if (ch >= UTF16_HIGH_SURROGATE_MIN &&
	    ch <= UTF16_HIGH_SURROGATE_MAX) {
		/* High surrogate word. */
		*(uint16_t *)self->up_pend = ch;
		self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
		return 0;
	}
	return unicode_printer_putc(self, ch);
}



/* Append UTF-8 text to the back of the given printer.
 * An incomplete UTF-8 sequences can be completed by future uses of this function.
 * HINT: This function is intentionally designed as compatible with `Dee_formatprinter_t'
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DPRINTER_CC
Dee_unicode_printer_print(void *__restrict self,
                          /*utf-8*/ char const *__restrict text,
                          size_t textlen) {
	struct unicode_printer *me;
	size_t result = textlen;
	char const *flush_start;
	me = (struct unicode_printer *)self;
	while (me->up_flags & UNICODE_PRINTER_FPENDING) {
		/* Complete a pending UTF-8 sequence. */
		uint8_t curlen, reqlen;
		if (!textlen)
			goto done;
		curlen = (me->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		me->up_pend[curlen] = (uint8_t)*text++;
		--textlen;
		reqlen = unicode_utf8seqlen[me->up_pend[0]];
		ASSERT(curlen + 1 <= reqlen);
		if (curlen + 1 >= reqlen) {
			/* Append the full character. */
			int error;
			uint32_t ch32 = utf8_getchar((uint8_t *)me->up_pend, reqlen);
			error         = unicode_printer_putc(me, ch32);
			if unlikely(error < 0)
				goto err;
			me->up_flags &= ~UNICODE_PRINTER_FPENDING;
			break;
		}
		me->up_flags += 1 << UNICODE_PRINTER_FPENDING_SHFT;
	}
again_flush:
	flush_start = text;
	while (textlen && (unsigned char)*text < 0xc0) {
		++text;
		--textlen;
	}

	/* Print ASCII text. */
	if (flush_start != text) {
		if unlikely(unicode_printer_print8(me,
		                                   (uint8_t const *)flush_start,
		                                   (size_t)(text - flush_start)) < 0)
			goto err;
	}
	if (textlen) {
		uint8_t seqlen;
		uint32_t ch32;
		ASSERT((unsigned char)*text >= 0xc0);
		seqlen = unicode_utf8seqlen[(uint8_t)*text];
		if (seqlen > textlen) {
			/* Incomplete sequence! (safe as pending UTF-8) */
			memcpyb(me->up_pend, text, textlen);
			me->up_flags |= (uint8_t)textlen << UNICODE_PRINTER_FPENDING_SHFT;
			goto done;
		}

		/* Expand the utf-8 sequence and emit it as a single character. */
		ch32 = utf8_getchar((uint8_t const *)text, seqlen);
		if (unicode_printer_putc(me, ch32))
			goto err;
		text += seqlen;
		textlen -= seqlen;
		goto again_flush;
	}
done:
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_printutf16(struct unicode_printer *__restrict self,
                               /*utf-16*/ uint16_t const *__restrict text,
                               size_t textlen) {
	size_t result = textlen;
	uint16_t const *flush_start;
	uint16_t low_surrogate, high_surrogate;
	if (self->up_flags & UNICODE_PRINTER_FPENDING) {
		/* Complete a pending UTF-16 sequence. */
		if (!textlen)
			goto done;
		high_surrogate = *(uint16_t *)self->up_pend;
		low_surrogate  = text[0];
		if (unicode_printer_putc(self, UTF16_COMBINE_SURROGATES(high_surrogate, low_surrogate)))
			goto err;
		self->up_flags &= ~UNICODE_PRINTER_FPENDING;
		++text;
		--textlen;
	}
again_flush:
	flush_start = text;
	while (textlen &&
	       (*text < UTF16_HIGH_SURROGATE_MIN ||
	        *text > UTF16_HIGH_SURROGATE_MAX)) {
		++text;
		--textlen;
	}

	/* Print pure UTF-16 text. */
	if (flush_start != text) {
		if unlikely(unicode_printer_print16(self, flush_start, (size_t)(text - flush_start)) < 0)
			goto err;
	}

	if (textlen) {
		uint32_t ch32;
		if (textlen == 1) {
			/* Incomplete surrogate pair. */
			*(uint16_t *)self->up_pend = text[0];
			self->up_flags |= (1 << UNICODE_PRINTER_FPENDING_SHFT);
			goto done;
		}

		/* Compile the surrogates and print the resulting unicode character. */
		high_surrogate = text[0];
		low_surrogate  = text[1];
		ch32 = UTF16_COMBINE_SURROGATES(high_surrogate, low_surrogate);
		if (unicode_printer_putc(self, ch32))
			goto err;
		text += 2;
		textlen -= 2;
		goto again_flush;
	}
done:
	return result;
err:
	return -1;
}




/* Print raw 8, 16 or 32-bit sequences of unicode characters.
 *  - `unicode_printer_print8' prints characters from the range U+0000 .. U+00FF (aka. latin-1)
 *  - `unicode_printer_print16' prints characters from the range U+0000 .. U+FFFF
 *  - `unicode_printer_print32' prints characters from the range U+0000 .. U+10FFFF (FFFFFFFF)
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_print8(struct unicode_printer *__restrict self,
                           uint8_t const *__restrict text,
                           size_t textlen) {
	size_t result = textlen;
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		String *base_string;
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		if unlikely(!textlen)
			goto done;
		/* Allocate the initial buffer. */
		initial_alloc = textlen;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		base_string = (String *)DeeObject_TryMallocc(offsetof(String, s_str),
		                                             initial_alloc + 1, sizeof(char));
		if unlikely(!base_string) {
			initial_alloc = textlen;
			base_string = (String *)DeeObject_Mallocc(offsetof(String, s_str),
			                                          initial_alloc + 1, sizeof(char));
			if unlikely(!base_string)
				goto err;
		}
		base_string->s_len = initial_alloc;
		self->up_buffer = memcpyb(base_string->s_str, text, textlen);
		self->up_length = textlen;
		goto done;
	}

	/* Append new string data. */
	SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		if (self->up_length + textlen > WSTR_LENGTH(string.cp8)) {
			/* Must allocate more memory. */
			String *new_string;
			size_t new_alloc = WSTR_LENGTH(string.cp_char);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			new_string = (String *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(string.cp_char, String, s_str),
			                                             offsetof(String, s_str), new_alloc + 1, sizeof(char));
			if unlikely(!new_string) {
				new_alloc  = self->up_length + textlen;
				new_string = (String *)DeeObject_Reallocc(COMPILER_CONTAINER_OF(string.cp_char, String, s_str),
				                                          offsetof(String, s_str), new_alloc + 1, sizeof(char));
				if unlikely(!new_string)
					goto err;
			}
			new_string->s_len = new_alloc;
			self->up_buffer = string.cp_char = new_string->s_str;
		}
		string.cp8 += self->up_length;
		self->up_length += textlen;
		memcpyb(string.cp8, text, textlen);
		break;

	CASE_WIDTH_2BYTE: {
		if (self->up_length + textlen > WSTR_LENGTH(string.cp16)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp16);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp16 = DeeString_TryResize2ByteBuffer(string.cp16, new_alloc);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_TryResize2ByteBuffer((uint16_t *)self->up_buffer,
				                                             self->up_length + textlen);
				if unlikely(!string.cp16)
					goto err;
			}
			self->up_buffer = string.cp16;
		}
		string.cp16 += self->up_length;
		self->up_length += textlen;
		while (textlen--)
			*string.cp16++ = *text++;
	}	break;

	CASE_WIDTH_4BYTE: {
		if (self->up_length + textlen > WSTR_LENGTH(string.cp32)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp32);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp32 = DeeString_TryResize4ByteBuffer(string.cp32, new_alloc);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_TryResize4ByteBuffer((uint32_t *)self->up_buffer,
				                                             self->up_length + textlen);
				if unlikely(!string.cp32)
					goto err;
			}
			self->up_buffer = string.cp32;
		}
		string.cp32 += self->up_length;
		self->up_length += textlen;
		while (textlen--)
			*string.cp32++ = *text++;
	}	break;

	}
done:
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
Dee_unicode_printer_repeatascii(struct unicode_printer *__restrict self,
                                char ch, size_t num_chars) {
	size_t result = num_chars;
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		String *base_string;
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		if unlikely(!num_chars)
			goto done;
		/* Allocate the initial buffer. */
		initial_alloc = num_chars;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		base_string = (String *)DeeObject_TryMallocc(offsetof(String, s_str),
		                                             initial_alloc + 1, sizeof(char));
		if unlikely(!base_string) {
			initial_alloc = num_chars;
			base_string = (String *)DeeObject_Mallocc(offsetof(String, s_str),
			                                          initial_alloc + 1, sizeof(char));
			if unlikely(!base_string)
				goto err;
		}
		base_string->s_len = initial_alloc;
		self->up_buffer = memsetb(base_string->s_str, (uint8_t)ch, num_chars);
		self->up_length = num_chars;
		goto done;
	}

	/* Append new string data. */
	SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		if (self->up_length + num_chars > WSTR_LENGTH(string.cp8)) {
			/* Must allocate more memory. */
			String *new_string;
			size_t new_alloc = WSTR_LENGTH(string.cp_char);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + num_chars);
			new_string = (String *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(string.cp_char, String, s_str),
			                                             offsetof(String, s_str), new_alloc + 1, sizeof(char));
			if unlikely(!new_string) {
				new_alloc  = self->up_length + num_chars;
				new_string = (String *)DeeObject_Reallocc(COMPILER_CONTAINER_OF(string.cp_char, String, s_str),
				                                          offsetof(String, s_str), new_alloc + 1, sizeof(char));
				if unlikely(!new_string)
					goto err;
			}
			new_string->s_len = new_alloc;
			self->up_buffer = string.cp_char = new_string->s_str;
		}
		string.cp8 += self->up_length;
		self->up_length += num_chars;
		memsetb(string.cp8, (uint8_t)ch, num_chars);
		break;

	CASE_WIDTH_2BYTE:
		if (self->up_length + num_chars > WSTR_LENGTH(string.cp16)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp16);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + num_chars);
			string.cp16 = DeeString_TryResize2ByteBuffer(string.cp16, new_alloc);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_TryResize2ByteBuffer((uint16_t *)self->up_buffer,
				                                             self->up_length + num_chars);
				if unlikely(!string.cp16)
					goto err;
			}
			self->up_buffer = string.cp16;
		}
		string.cp16 += self->up_length;
		self->up_length += num_chars;
		memsetw(string.cp16, (uint16_t)(uint8_t)ch, num_chars);
		break;

	CASE_WIDTH_4BYTE:
		if (self->up_length + num_chars > WSTR_LENGTH(string.cp32)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp32);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + num_chars);
			string.cp32 = DeeString_TryResize4ByteBuffer(string.cp32, new_alloc);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_TryResize4ByteBuffer((uint32_t *)self->up_buffer,
				                                             self->up_length + num_chars);
				if unlikely(!string.cp32)
					goto err;
			}
			self->up_buffer = string.cp32;
		}
		string.cp32 += self->up_length;
		self->up_length += num_chars;
		memsetl(string.cp32, (uint32_t)(uint8_t)ch, num_chars);
		break;

	}
done:
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_print16(struct unicode_printer *__restrict self,
                            uint16_t const *__restrict text,
                            size_t textlen) {
	size_t result = textlen;
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		if unlikely(!textlen)
			goto done;
		initial_alloc = textlen;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		/* Allocate the initial buffer. */
		string.cp16 = DeeString_TryNew2ByteBuffer(initial_alloc);
		if unlikely(!string.cp16) {
			string.cp16 = DeeString_New2ByteBuffer(textlen);
			if unlikely(!string.cp16)
				goto err;
		}
		memcpyw(string.cp16, text, textlen);
		self->up_buffer = string.cp16;
		self->up_length = textlen;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
		self->up_flags |= STRING_WIDTH_2BYTE;
		goto done;
	}

	SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		/* Check if there are any characters > 0xff */
		for (i = 0; i < textlen; ++i) {
			if (text[i] <= 0xff)
				continue;
			/* Must up-cast */
			string.cp16 = cast_8to16(COMPILER_CONTAINER_OF(string.cp_char, DeeStringObject, s_str),
			                         self->up_length);
			if unlikely(!string.cp16)
				goto err;
			self->up_buffer = string.cp16;
#if STRING_WIDTH_1BYTE != 0
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->up_flags |= STRING_WIDTH_2BYTE;
			goto append_2byte;
		}
		if (self->up_length + textlen > WSTR_LENGTH(string.cp8)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp8);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp8 = DeeString_TryResize1ByteBuffer(string.cp8, new_alloc);
			if unlikely(!string.cp8) {
				string.cp8 = DeeString_Resize1ByteBuffer((uint8_t *)self->up_buffer,
				                                         self->up_length + textlen);
				if unlikely(!string.cp8)
					goto err;
			}
			self->up_buffer = string.cp8;
		}
		string.cp8 += self->up_length;
		self->up_length += textlen;
		while (textlen--)
			*string.cp8++ = (uint8_t)*text++;
	}	break;

	CASE_WIDTH_2BYTE:
append_2byte:
		if (self->up_length + textlen > WSTR_LENGTH(string.cp16)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp16);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp16 = DeeString_TryResize2ByteBuffer(string.cp16, new_alloc);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_Resize2ByteBuffer((uint16_t *)self->up_buffer,
				                                          self->up_length + textlen);
				if unlikely(!string.cp16)
					goto err;
			}
			self->up_buffer = string.cp16;
		}
		string.cp16 += self->up_length;
		self->up_length += textlen;
		memcpyw(string.cp16, text, textlen);
		break;

	CASE_WIDTH_4BYTE:
		if (self->up_length + textlen > WSTR_LENGTH(string.cp32)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp32);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp32 = DeeString_TryResize4ByteBuffer(string.cp32, new_alloc);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_Resize4ByteBuffer((uint32_t *)self->up_buffer,
				                                          self->up_length + textlen);
				if unlikely(!string.cp32)
					goto err;
			}
			self->up_buffer = string.cp32;
		}
		string.cp32 += self->up_length;
		self->up_length += textlen;
		while (textlen--)
			*string.cp32++ = *text++;
		break;

	}
done:
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_print32(struct unicode_printer *__restrict self,
                            uint32_t const *__restrict text,
                            size_t textlen) {
	size_t i, result = textlen;
	union dcharptr string;
	if (textlen == 1)
		return unicode_printer_putc(self, text[0]) ? -1 : 1;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		if unlikely(!textlen)
			goto done;
		initial_alloc = textlen;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		/* Allocate the initial buffer. */
		string.cp32 = DeeString_TryNew4ByteBuffer(initial_alloc);
		if unlikely(!string.cp32) {
			string.cp32 = DeeString_New4ByteBuffer(textlen);
			if unlikely(!string.cp32)
				goto err;
		}
		memcpyl(string.cp32, text, textlen);
		self->up_buffer = string.cp32;
		self->up_length = textlen;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
		self->up_flags |= STRING_WIDTH_4BYTE;
		goto done;
	}

	SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		/* Check if there are any characters > 0xff */
		for (i = 0; i < textlen; ++i) {
			if (text[i] <= 0xff)
				continue;
			if (text[i] <= 0xffff) {
				/* Must up-cast to 16-bit */
				string.cp16 = cast_8to16(COMPILER_CONTAINER_OF(string.cp_char, DeeStringObject, s_str),
				                         self->up_length);
				if unlikely(!string.cp16)
					goto err;
				self->up_buffer = string.cp16;
#if STRING_WIDTH_1BYTE != 0
				self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
				self->up_flags |= STRING_WIDTH_2BYTE;
				goto append_2byte;
			}

			/* Must up-cast to 32-bit */
			string.cp32 = cast_8to32(COMPILER_CONTAINER_OF(string.cp_char, DeeStringObject, s_str),
			                         self->up_length);
			if unlikely(!string.cp32)
				goto err;
			self->up_buffer = string.cp32;
#if STRING_WIDTH_1BYTE != 0
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->up_flags |= STRING_WIDTH_4BYTE;
			goto append_4byte;
		}

		if (self->up_length + textlen > WSTR_LENGTH(string.cp8)) {
			size_t new_alloc;
			/* Must allocate more memory. */
			new_alloc = WSTR_LENGTH(string.cp8);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp8 = DeeString_TryResize1ByteBuffer(string.cp8, new_alloc);
			if unlikely(!string.cp8) {
				string.cp8 = DeeString_Resize1ByteBuffer((uint8_t *)self->up_buffer,
				                                         self->up_length + textlen);
				if unlikely(!string.cp8)
					goto err;
			}
			self->up_buffer = string.cp8;
		}
		string.cp8 += self->up_length;
		self->up_length += textlen;
		while (textlen--)
			*string.cp8++ = (uint8_t)*text++;
		break;

	CASE_WIDTH_2BYTE:
append_2byte:
		/* Check if there are any characters > 0xffff */
		for (i = 0; i < textlen; ++i) {
			if (text[i] <= 0xffff)
				continue;
			/* Must up-cast to 32-bit */
			string.cp32 = cast_16to32(string.cp16, self->up_length);
			if unlikely(!string.cp32)
				goto err;
			self->up_buffer = string.cp32;
			self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
			self->up_flags |= STRING_WIDTH_4BYTE;
			goto append_4byte;
		}
		if (self->up_length + textlen > WSTR_LENGTH(string.cp16)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp16);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp16 = DeeString_TryResize2ByteBuffer(string.cp16, new_alloc);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_Resize2ByteBuffer((uint16_t *)self->up_buffer,
				                                          self->up_length + textlen);
				if unlikely(!string.cp16)
					goto err;
			}
			self->up_buffer = string.cp16;
		}
		string.cp16 += self->up_length;
		self->up_length += textlen;
		while (textlen--)
			*string.cp16++ = (uint16_t)*text++;
		break;

	CASE_WIDTH_4BYTE:
append_4byte:
		if (self->up_length + textlen > WSTR_LENGTH(string.cp32)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp32);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + textlen);
			string.cp32 = DeeString_TryResize4ByteBuffer(string.cp32, new_alloc);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_Resize4ByteBuffer((uint32_t *)self->up_buffer,
				                                          self->up_length + textlen);
				if unlikely(!string.cp32)
					goto err;
			}
			self->up_buffer = string.cp32;
		}
		string.cp32 += self->up_length;
		self->up_length += textlen;
		memcpyl(string.cp32, text, textlen);
		break;

	}
done:
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_printinto(struct unicode_printer *__restrict self,
                              Dee_formatprinter_t printer, void *arg) {
	union dcharptr_const str;
	size_t length;
	Dee_ssize_t result;

	/* Optimization for when the target is another unicode-printer. */
	length = self->up_length;
	if (printer == &unicode_printer_print) {
		struct unicode_printer *dst;
		dst = (struct unicode_printer *)arg;
		if (!dst->up_buffer) {
			/* The simplest cast: the target buffer is currently empty, and since
			 * we're allocated to modify `self', we can simply move everything! */
			memcpy(dst, self, sizeof(struct unicode_printer));
			self->up_length = 0;
			self->up_buffer = NULL;
			self->up_flags  = STRING_WIDTH_1BYTE;
			return dst->up_length;
		}
		/* Append all data from out buffer.
		 * Since we know that the target is a unicode-printer,
		 * we don't have to perform any UTF-8 conversions! */
		str.ptr = self->up_buffer;
		SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			return unicode_printer_print8(dst, str.cp8, length);

		CASE_WIDTH_2BYTE:
			return unicode_printer_print16(dst, str.cp16, length);

		CASE_WIDTH_4BYTE:
			return unicode_printer_print32(dst, str.cp32, length);
		}
	}

	/* Because we must print everything in UTF-8, this part gets a
	 * bit complicated since we need to convert everything on-the-fly. */
	str.ptr = self->up_buffer;
	result  = 0;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
	if (str.ptr != NULL)
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
	{
		SWITCH_SIZEOF_WIDTH(UNICODE_PRINTER_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			result = DeeFormat_Print8(printer, arg, str.cp8, length);
			break;

		CASE_WIDTH_2BYTE:
			result = DeeFormat_Print16(printer, arg, str.cp16, length);
			break;

		CASE_WIDTH_4BYTE:
			result = DeeFormat_Print32(printer, arg, str.cp32, length);
			break;
		}
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
Dee_unicode_printer_reserve(struct unicode_printer *__restrict self,
                            size_t num_chars) {
	size_t result = self->up_length;
	union dcharptr str;
	str.ptr = self->up_buffer;
	if unlikely(!str.ptr) {
		String *init_buffer;
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(self->up_length == 0);
		ASSERT(self->up_flags == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		/* Allocate the initial buffer. */
		initial_alloc = num_chars;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		init_buffer = (String *)DeeObject_TryMallocc(offsetof(String, s_str),
		                                             initial_alloc + 1, sizeof(char));
		if unlikely(!init_buffer) {
			initial_alloc = num_chars;
			init_buffer = (String *)DeeObject_Mallocc(offsetof(String, s_str),
			                                          initial_alloc + 1, sizeof(char));
			if unlikely(!init_buffer)
				goto err;
		}
		init_buffer->s_len = initial_alloc;
		self->up_buffer    = init_buffer;
		self->up_length    = num_chars;
	} else {
		size_t size_avail = WSTR_LENGTH(str.ptr);
		size_t new_length = self->up_length + num_chars;
		if (new_length > size_avail) {
			/* Must allocate more memory. */
			size_t new_size = size_avail;
			do {
				new_size *= 2;
			} while (new_size < new_length);
			str.ptr = DeeString_TryResizeWidthBuffer(str.ptr, new_size,
			                                         UNICODE_PRINTER_WIDTH(self));
			if unlikely(!str.ptr) {
				str.ptr = DeeString_ResizeWidthBuffer(str.ptr, new_length,
				                                      UNICODE_PRINTER_WIDTH(self));
				if unlikely(!str.ptr)
					goto err;
			}
			self->up_buffer = str.ptr;
		}
		self->up_length = new_length;
	}
	return (Dee_ssize_t)result;
err:
	return -1;
}






/* Print a unicode character `ch', encoded as UTF-8 into `printer' */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeFormat_Putc(/*utf-8*/ Dee_formatprinter_t printer, void *arg, uint32_t ch) {
	char utf8_repr[UNICODE_UTF8_CURLEN];
	size_t utf8_len;
	if (printer == &unicode_printer_print) {
		if (unicode_printer_putc((struct unicode_printer *)arg, ch))
			goto err;
		return 1;
	}
	utf8_len = (size_t)(unicode_writeutf8(utf8_repr, ch) - utf8_repr);
	return (*printer)(arg, utf8_repr, utf8_len);
err:
	return -1;
}




/* Search for existing occurrences of, or append a new instance of a given string.
 * Upon success, return the index (offset from the base) of the string (in characters).
 * @return: * : The offset from the base of string being printed, where the given `str' can be found.
 * @return: -1: Failed to allocate the string. */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_reuse(struct unicode_printer *__restrict self,
                          /*utf-8*/ char const *__restrict str, size_t length) {
	size_t result;
	/* TODO: Search for an existing instance. */
	result = self->up_length;
	if unlikely(unicode_printer_print(self, str, length))
		goto err;
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_reuse8(struct unicode_printer *__restrict self,
                           uint8_t const *__restrict str, size_t length) {
	size_t result;
	/* TODO: Search for an existing instance. */
	result = self->up_length;
	if unlikely(unicode_printer_print8(self, str, length))
		goto err;
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_reuse16(struct unicode_printer *__restrict self,
                            uint16_t const *__restrict str, size_t length) {
	size_t result;
	/* TODO: Search for an existing instance. */
	result = self->up_length;
	if unlikely(unicode_printer_print16(self, str, length))
		goto err;
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_unicode_printer_reuse32(struct unicode_printer *__restrict self,
                            uint32_t const *__restrict str, size_t length) {
	size_t result;
	/* TODO: Search for an existing instance. */
	result = self->up_length;
	if unlikely(unicode_printer_print32(self, str, length))
		goto err;
	return result;
err:
	return -1;
}


/* Unicode utf-8 buffer API */
PUBLIC WUNUSED NONNULL((1)) char *DCALL
Dee_unicode_printer_tryalloc_utf8(struct unicode_printer *__restrict self,
                                  size_t length) {
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		String *base_string;
		size_t initial_alloc;
		uint8_t num_pending;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		num_pending = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		length += num_pending;
		/* Allocate the initial buffer. */
		initial_alloc = length;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		base_string = (String *)DeeObject_TryMallocc(offsetof(String, s_str),
		                                             initial_alloc + 1, sizeof(char));
		if unlikely(!base_string) {
			initial_alloc = length;
			base_string = (String *)DeeObject_Mallocc(offsetof(String, s_str),
			                                          initial_alloc + 1, sizeof(char));
			if unlikely(!base_string)
				goto err;
		}
		base_string->s_len = initial_alloc;
		self->up_length    = length;
		self->up_buffer    = base_string->s_str;
#ifndef __OPTIMIZE_SIZE__
		if likely(num_pending == 0)
			return base_string->s_str;
#endif /* !__OPTIMIZE_SIZE__ */
		return (char *)mempcpy(base_string->s_str, self->up_pend, num_pending);
	}
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE) {
		/* The unicode printer uses a single-byte character width, so we
		 * can allocate from that buffer and later check if only UTF-8
		 * was written. */
		uint8_t num_pending;
		num_pending = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		length += num_pending;
		if (self->up_length + length > WSTR_LENGTH(string.cp_char)) {
			String *new_string;
			size_t new_alloc;
			/* Must allocate more memory. */
			new_alloc = WSTR_LENGTH(string.cp_char);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + length);
			new_string = (String *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(string.cp_char, String, s_str),
			                                             offsetof(String, s_str), new_alloc + 1, sizeof(char));
			if unlikely(!new_string) {
				new_alloc  = self->up_length + length;
				new_string = (String *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(string.cp_char, String, s_str),
				                                             offsetof(String, s_str), new_alloc + 1, sizeof(char));
				if unlikely(!new_string)
					goto err;
			}
			new_string->s_len = new_alloc;
			self->up_buffer = string.cp_char = new_string->s_str;
		}
		string.cp8 += self->up_length;
		self->up_length += length;
#ifndef __OPTIMIZE_SIZE__
		if likely(num_pending == 0)
			return string.cp_char;
#endif /* !__OPTIMIZE_SIZE__ */
		return (char *)mempcpy(string.cp_char, self->up_pend, num_pending);
	}

	/* The unicode printer already uses a character width of more
	 * than 1 byte, meaning we can't allocate the buffer in-line.
	 * In this case, we allocate the caller-requested buffer separately on the heap. */
	return (char *)Dee_TryMallocc(length + 1, sizeof(char));
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) char *DCALL
Dee_unicode_printer_tryresize_utf8(struct unicode_printer *__restrict self,
                                   char *buf, size_t new_length) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE) {
		size_t old_length, total_avail, new_alloc, old_alloc;
		String *new_buffer;
		if (!buf)
			return unicode_printer_tryalloc_utf8(self, new_length);
		ASSERT(self->up_buffer != NULL);
		ASSERT(buf >= (char *)((uint8_t *)self->up_buffer));
		ASSERT(buf <= (char *)((uint8_t *)self->up_buffer + self->up_length));

		/* The buffer was allocated in-line. */
		old_length  = (size_t)(((uint8_t *)self->up_buffer + self->up_length) - (uint8_t *)buf);
		total_avail = (size_t)(((uint8_t *)self->up_buffer + WSTR_LENGTH(self->up_buffer)) - (uint8_t *)buf);
		ASSERT(total_avail >= old_length);
		if (new_length <= total_avail) {
			/* Update the buffer length within the pre-allocated bounds. */
			self->up_length -= old_length;
			self->up_length += new_length;
			return buf;
		}

		/* Must allocate a new buffer. */
		old_alloc = WSTR_LENGTH(self->up_buffer);
		new_alloc = (self->up_length - old_length) + new_length;
		ASSERT(old_alloc < new_alloc);
		ASSERT(old_alloc != 0);
		do {
			old_alloc *= 2;
		} while (old_alloc < new_alloc);

		/* Reallocate the buffer to fit the requested size. */
		new_buffer = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF((char *)self->up_buffer, String, s_str),
		                                            (old_alloc + 1) * sizeof(char));
		if unlikely(!new_buffer) {
			old_alloc  = new_alloc;
			new_buffer = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF((char *)self->up_buffer, String, s_str),
			                                            (old_alloc + 1) * sizeof(char));
			if unlikely(!new_buffer)
				goto err;
		}

		/* Install the reallocated buffer. */
		self->up_buffer   = new_buffer->s_str;
		new_buffer->s_len = old_alloc;
		self->up_length -= old_length;
		buf = (char *)((uint8_t *)new_buffer->s_str + self->up_length);
		self->up_length += new_length;
		return buf;
	} else {
		/* The buffer is purely heap-allocated. */
		return (char *)Dee_TryReallocc(buf, new_length + 1, sizeof(char));
	}
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) char *DCALL
Dee_unicode_printer_alloc_utf8(struct unicode_printer *__restrict self,
                               size_t length) {
	char *result;
	do {
		result = unicode_printer_tryalloc_utf8(self, length);
	} while (unlikely(!result) && Dee_CollectMemoryc(length, sizeof(uint8_t)));
	return result;
}

PUBLIC WUNUSED NONNULL((1)) char *DCALL
Dee_unicode_printer_resize_utf8(struct unicode_printer *__restrict self,
                                char *buf, size_t new_length) {
	char *result;
	do {
		result = unicode_printer_tryresize_utf8(self, buf, new_length);
	} while (unlikely(!result) && Dee_CollectMemoryc(new_length, sizeof(uint8_t)));
	return result;
}

PUBLIC NONNULL((1)) void DCALL
Dee_unicode_printer_free_utf8(struct unicode_printer *__restrict self, char *buf) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE) {
		if (!buf)
			return;

		/* Deal with pending characters. */
		buf -= (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		ASSERT(buf >= (char *)((uint8_t *)self->up_buffer));
		ASSERT(buf <= (char *)((uint8_t *)self->up_buffer + self->up_length));

		/* Mark the buffer memory as having been freed again. */
		self->up_length = (size_t)((uint8_t *)buf - (uint8_t *)self->up_buffer);
	} else {
		Dee_Free(buf);
	}
}

PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
Dee_unicode_printer_commit_utf8(struct unicode_printer *__restrict self,
                                char *buf, size_t final_length) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE) {
		size_t count;
		uint8_t *iter;
		size_t confirm_length = final_length;
		if (!buf)
			return 0;
		if (self->up_flags & UNICODE_PRINTER_FPENDING) {
			uint8_t num_pending;
			/* Include pending UTF-8 characters.
			 * -> When the buffer was originally allocated, these had been included
			 *    as a hidden prefix to the buffer that the user requested. */
			num_pending = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
			buf -= num_pending;
			confirm_length += num_pending;
			self->up_flags &= ~UNICODE_PRINTER_FPENDING;
		}
		ASSERT(buf >= (char *)((uint8_t *)self->up_buffer));
		ASSERT(buf + confirm_length <= (char *)((uint8_t *)self->up_buffer + self->up_length));

		/* Now that the caller initialized the UTF-8 content, we must
		 * check if it contains any unicode sequences that cannot appear
		 * as part of a 1-byte string. */
		iter  = (uint8_t *)buf;
		count = confirm_length;
		while (count--) {
			uint8_t ch, utf8_length;
			uint32_t ch32;
			ch = *iter++;
			if (ch < 0xc0)
				continue; /* Pure ASCII / invalid UTF-8 (which we ignore here) */

			/* Decode the full unicode character. */
			utf8_length = unicode_utf8seqlen[ch];
			ASSERT(utf8_length >= 2);
			if (utf8_length > count) {
				/* Incomplete, trailing UTF-8 sequence */
				self->up_flags |= (uint8_t)count << UNICODE_PRINTER_FPENDING_SHFT;
				memcpyc(self->up_pend, iter - 1,
				        count, sizeof(uint8_t));
				confirm_length -= count;
				break;
			}

			/* Decode the unicode character. */
			ASSERT(count != 0);
			ch32 = utf8_getchar(iter - 1, utf8_length);
			if (ch32 <= 0xff) {
				/* The character still fits into a single byte! */
				uint8_t *new_iter;
				iter[-1] = (uint8_t)ch32;
				new_iter = iter + utf8_length - 1;
				memmovedown(iter, new_iter,
				            (size_t)(((uint8_t *)buf + confirm_length) - new_iter));
				--count;
				--confirm_length;
				iter = new_iter;
				continue;
			}

			/* Must up-cast to a 16-bit, or 32-bit string. */
			{
				uint8_t *utf8_start       = iter - 1;
				size_t utf8_convlength    = count + 1;
				uint8_t *singlebyte_start = (uint8_t *)self->up_buffer;
				size_t singlebyte_length  = (size_t)(utf8_start - singlebyte_start);
				if (ch32 <= 0xffff) {
					/* Quick check if all characters still left to parsed can fit into 16 bits. */
					size_t i, w16_length = singlebyte_length + utf8_convlength;
					uint16_t *string16, *dst;
					for (i = utf8_convlength; i < utf8_convlength;) {
						ch = utf8_start[i];
						if (ch <= 0xc0) {
							++i;
							continue;
						}
						utf8_length = unicode_utf8seqlen[ch];
						ASSERT(utf8_length >= 2);
						if (i + utf8_length > utf8_convlength) {
							/* Incomplete UTF-8 sequence. */
							uint8_t pendsz = (uint8_t)(utf8_convlength - i);
							memcpyc(self->up_pend,
							        &utf8_start[i],
							        pendsz,
							        sizeof(unsigned char));
							self->up_flags |= pendsz << UNICODE_PRINTER_FPENDING_SHFT;
							utf8_convlength = i;
							break;
						}

						/* All utf-8 sequences of less than 4 characters can fit into 16 bits. */
						if (utf8_length >= 4)
							goto upcast_to_32bit;
						w16_length -= (utf8_length - 1);
						i += utf8_length;
					}

					/* Allocate the new 16-bit string. */
					string16 = DeeString_New2ByteBuffer(w16_length);
					if unlikely(!string16)
						goto err;
					for (i = 0; i < singlebyte_length; ++i)
						string16[i] = singlebyte_start[i];
					dst = string16 + singlebyte_length;
					for (i = utf8_convlength; i < utf8_convlength;) {
						ch = utf8_start[i];
						if (ch <= 0xc0) {
							*dst++ = ch;
							++i;
							continue;
						}
						utf8_length = unicode_utf8seqlen[ch];
						ASSERT(utf8_length >= 2);
						ASSERT(utf8_length <= 4);
						*dst++ = utf8_getchar16(&utf8_start[i], utf8_length);
						i += utf8_length;
					}
					ASSERT(dst == string16 + w16_length);

					/* All right! we've got the 16-bit string all created
					 * (including the new content from the caller's buffer)
					 * Now get rid of the old 8-bit string and upgrade the printer. */
					DeeObject_Free(COMPILER_CONTAINER_OF(self->up_buffer, String, s_str));
					self->up_buffer = string16;
					self->up_length = w16_length;
#if STRING_WIDTH_1BYTE != 0
					self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
					self->up_flags |= STRING_WIDTH_2BYTE;
					goto return_final_length;
				} else {
					size_t i, w32_length;
					uint32_t *string32, *dst;
upcast_to_32bit:
					w32_length = singlebyte_length + utf8_convlength;

					/* Determine the length of the 32-bit string that we're about to construct. */
					for (i = utf8_convlength; i < utf8_convlength;) {
						ch = utf8_start[i];
						if (ch <= 0xc0) {
							++i;
							continue;
						}
						utf8_length = unicode_utf8seqlen[ch];
						ASSERT(utf8_length >= 2);
						w32_length -= (utf8_length - 1);
						i += utf8_length;
					}

					/* Allocate the new 32-bit string. */
					string32 = DeeString_New4ByteBuffer(w32_length);
					if unlikely(!string32)
						goto err;
					for (i = 0; i < singlebyte_length; ++i)
						string32[i] = singlebyte_start[i];
					dst = string32 + singlebyte_length;
					for (i = utf8_convlength; i < utf8_convlength;) {
						ch = utf8_start[i];
						if (ch <= 0xc0) {
							*dst++ = ch;
							++i;
							continue;
						}
						utf8_length = unicode_utf8seqlen[ch];
						ASSERT(utf8_length >= 2);
						*dst++ = utf8_getchar(&utf8_start[i], utf8_length);
						i += utf8_length;
					}
					ASSERT(dst == string32 + w32_length);

					/* All right! we've got the 32-bit string all created
					 * (including the new content from the caller's buffer)
					 * Now get rid of the old 8-bit string and upgrade the printer. */
					DeeObject_Free(COMPILER_CONTAINER_OF(self->up_buffer, String, s_str));
					self->up_buffer = string32;
					self->up_length = w32_length;
#if STRING_WIDTH_1BYTE != 0
					self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
					self->up_flags |= STRING_WIDTH_4BYTE;
					goto return_final_length;
				}
			}
		}

		/* Remember the actual length of the buffer. */
		self->up_length = (size_t)(((uint8_t *)buf + confirm_length) - (uint8_t *)self->up_buffer);
return_final_length:
		return (Dee_ssize_t)final_length;
	} else {
		Dee_ssize_t result;

		/* Simply print the buffer as UTF-8 text. */
		result = unicode_printer_print(self, buf, final_length);
		Dee_Free(buf);
		return result;
	}
err:
	return -1;
}




PUBLIC WUNUSED NONNULL((1)) uint16_t *DCALL
Dee_unicode_printer_tryalloc_utf16(struct unicode_printer *__restrict self,
                                   size_t length) {
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		if (self->up_flags & UNICODE_PRINTER_FPENDING)
			++length;

		/* Allocate the initial buffer. */
		initial_alloc = length;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		string.cp16 = DeeString_TryNew2ByteBuffer(initial_alloc);
		if unlikely(!string.cp16) {
			string.cp16 = DeeString_TryNew2ByteBuffer(length);
			if unlikely(!string.cp16)
				goto err;
		}
		self->up_length = length;
		self->up_buffer = string.cp16;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
		self->up_flags |= STRING_WIDTH_2BYTE;
		if (!(self->up_flags & UNICODE_PRINTER_FPENDING))
			return string.cp16;
		*string.cp16 = *(uint16_t *)self->up_pend;
		return string.cp16 + 1;
	}
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_2BYTE) {
		if (self->up_flags & UNICODE_PRINTER_FPENDING)
			++length;
		if (self->up_length + length > WSTR_LENGTH(string.cp16)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp16);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + length);
			string.cp16 = DeeString_TryResize2ByteBuffer(string.cp16, new_alloc);
			if unlikely(!string.cp16) {
				string.cp16 = DeeString_TryResize2ByteBuffer((uint16_t *)self->up_buffer,
				                                             self->up_length + length);
				if unlikely(!string.cp16)
					goto err;
			}
			self->up_buffer = string.cp16;
		}
		string.cp16 += self->up_length;
		self->up_length += length;
		if (!(self->up_flags & UNICODE_PRINTER_FPENDING))
			return string.cp16;
		*string.cp16 = *(uint16_t *)self->up_pend;
		return string.cp16 + 1;
	}
	return (uint16_t *)Dee_TryMallocc(length + 1, sizeof(uint16_t));
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t *DCALL
Dee_unicode_printer_tryresize_utf16(struct unicode_printer *__restrict self,
                                    uint16_t *buf, size_t new_length) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_2BYTE) {
		uint16_t *string;
		size_t old_length, total_avail, new_alloc, old_alloc;
		if (!buf)
			return unicode_printer_tryalloc_utf16(self, new_length);
		string = (uint16_t *)self->up_buffer;
		ASSERT(string != NULL);
		ASSERT(buf >= string);
		ASSERT(buf <= string + self->up_length);

		/* The buffer was allocated in-line. */
		old_length  = (size_t)((string + self->up_length) - buf);
		total_avail = (size_t)((string + WSTR_LENGTH(string)) - buf);
		ASSERT(total_avail >= old_length);
		if (new_length <= total_avail) {
			/* Update the buffer length within the pre-allocated bounds. */
			self->up_length -= old_length;
			self->up_length += new_length;
			return buf;
		}

		/* Must allocate a new buffer. */
		old_alloc = WSTR_LENGTH(string);
		new_alloc = (self->up_length - old_length) + new_length;
		ASSERT(old_alloc < new_alloc);
		ASSERT(old_alloc != 0);
		do {
			old_alloc *= 2;
		} while (old_alloc < new_alloc);

		/* Reallocate the buffer to fit the requested size. */
		string = DeeString_TryResize2ByteBuffer(string, old_alloc);
		if unlikely(!string) {
			string = DeeString_TryResize2ByteBuffer(string, new_alloc);
			if unlikely(!string)
				goto err;
		}

		/* Install the reallocated buffer. */
		self->up_buffer = string;
		self->up_length -= old_length;
		buf = string + self->up_length;
		self->up_length += new_length;
		return buf;
	} else {
		/* The buffer is purely heap-allocated. */
		return (uint16_t *)Dee_TryReallocc(buf, new_length + 1, sizeof(uint16_t));
	}
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t *DCALL
Dee_unicode_printer_alloc_utf16(struct unicode_printer *__restrict self,
                                size_t length) {
	uint16_t *result;
	do {
		result = unicode_printer_tryalloc_utf16(self, length);
	} while (unlikely(!result) && Dee_CollectMemoryc(length, sizeof(uint16_t)));
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t *DCALL
Dee_unicode_printer_resize_utf16(struct unicode_printer *__restrict self,
                                 uint16_t *buf, size_t new_length) {
	uint16_t *result;
	do {
		result = unicode_printer_tryresize_utf16(self, buf, new_length);
	} while (unlikely(!result) && Dee_CollectMemoryc(new_length, sizeof(uint16_t)));
	return result;
}

PUBLIC NONNULL((1)) void DCALL
Dee_unicode_printer_free_utf16(struct unicode_printer *__restrict self,
                               uint16_t *buf) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_2BYTE) {
		if (!buf)
			return;

		/* Deal with pending characters. */
		buf -= (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		ASSERT(buf >= (uint16_t *)self->up_buffer);
		ASSERT(buf <= (uint16_t *)self->up_buffer + self->up_length);

		/* Mark the buffer memory as having been freed again. */
		self->up_length = (size_t)((uint16_t *)buf - (uint16_t *)self->up_buffer);
	} else {
		Dee_Free(buf);
	}
}

PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
Dee_unicode_printer_commit_utf16(struct unicode_printer *__restrict self,
                                 uint16_t *buf, size_t final_length) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_2BYTE) {
		size_t count;
		uint16_t *iter;
		size_t confirm_length = final_length;
		if (!buf)
			return 0;
		if (self->up_flags & UNICODE_PRINTER_FPENDING) {
			--buf;
			++confirm_length;
			self->up_flags &= ~UNICODE_PRINTER_FPENDING;
		}
		ASSERT(buf >= (uint16_t *)self->up_buffer);
		ASSERT(buf + confirm_length <= (uint16_t *)self->up_buffer + self->up_length);

		/* Search for surrogates to see if the string can works as a pure 16-bit string. */
		iter = buf, count = confirm_length;
		while (count--) {
			uint16_t ch = *iter++;
			if (ch < UTF16_HIGH_SURROGATE_MIN ||
			    ch > UTF16_HIGH_SURROGATE_MAX)
				continue;
			if (!count) {
				/* Unmatched high surrogate. */
				*(uint16_t *)self->up_pend = ch;
				self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
				break;
			}

			/* Must up-cast to a full 32-bit string. */
			{
				uint32_t *w32_string, *dst;
				uint16_t *utf16_start      = iter - 1;
				size_t utf16_length        = count + 1;
				uint16_t *doublebyte_start = (uint16_t *)self->up_buffer;
				size_t doublebyte_length   = (size_t)(utf16_start - doublebyte_start);
				size_t i, result_length = doublebyte_length + utf16_length + 1;
				goto check_low_surrogate;
				while (count--) {
					ch = *iter++;
					if (ch < UTF16_HIGH_SURROGATE_MIN ||
					    ch > UTF16_HIGH_SURROGATE_MAX)
						continue;
check_low_surrogate:
					--result_length; /* High surrogate of surrogate pair. */
					if (!count--) {
						/* Unmatched high surrogate. */
						*(uint16_t *)self->up_pend = ch;
						self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
						break;
					}
					ch = *iter++;
					if (ch < UTF16_LOW_SURROGATE_MIN ||
					    ch > UTF16_LOW_SURROGATE_MAX) {
						/* Invalid surrogate pair! */
						iter[-2] = '?';
						memmovedownw(iter - 1, iter, count);
						continue;
					}
				}

				/* Allocate the combined 32-bit string. */
				w32_string = DeeString_New4ByteBuffer(result_length);
				if unlikely(!w32_string)
					goto err;
				dst = w32_string;
				for (i = 0; i < doublebyte_length; ++i)
					*dst++ = doublebyte_start[i];

				/* Decode the utf-16 surrogates */
				for (i = 0; i < utf16_length; ++i) {
					uint16_t high;
					high = utf16_start[i];
					if (high < UTF16_HIGH_SURROGATE_MIN ||
					    high > UTF16_HIGH_SURROGATE_MAX) {
						*dst++ = high; /* Regular unicode character. */
					} else {
						++i;
						ASSERT(i < utf16_length);
						*dst++ = UTF16_COMBINE_SURROGATES(high, utf16_start[i]);
					}
				}
				*dst = 0;
				ASSERT(dst == w32_string + result_length);

				/* Store the new 32-bit string buffer in the printer. */
				Dee_Free((size_t *)self->up_buffer - 1);
				self->up_buffer = w32_string;
				self->up_length = result_length;
				self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
				self->up_flags |= STRING_WIDTH_4BYTE;
				goto return_final_length;
			}
		}

		/* Remember the actual length of the buffer. */
		self->up_length = (size_t)(((uint16_t *)buf + confirm_length) - (uint16_t *)self->up_buffer);
return_final_length:
		return (Dee_ssize_t)final_length;
	} else {
		Dee_ssize_t result;

		/* Simply print the buffer as UTF-16 text. */
		result = unicode_printer_printutf16(self,
		                                    buf,
		                                    final_length);
		Dee_Free(buf);
		return result;
	}
err:
	return -1;
}



PUBLIC WUNUSED NONNULL((1)) uint32_t *
(DCALL Dee_unicode_printer_tryalloc_utf32)(struct unicode_printer *__restrict self,
                                           size_t length) {
	union dcharptr string;
	string.ptr = self->up_buffer;
	if (!string.ptr) {
		size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		ASSERT(!self->up_length);
		ASSERT(UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_1BYTE);
#endif /* !CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */

		/* Allocate the initial buffer. */
		initial_alloc = length;
		if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
			initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
		if (initial_alloc < self->up_length)
			initial_alloc = self->up_length;
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags |= STRING_WIDTH_1BYTE;
#endif /* STRING_WIDTH_1BYTE != 0 */
#endif /* CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION */
		string.cp32 = DeeString_TryNew4ByteBuffer(initial_alloc);
		if unlikely(!string.cp32) {
			string.cp32 = DeeString_TryNew4ByteBuffer(length);
			if unlikely(!string.cp32)
				goto err;
		}
		self->up_length = length;
		self->up_buffer = string.cp32;
#if STRING_WIDTH_1BYTE != 0
		self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
		self->up_flags |= STRING_WIDTH_4BYTE;
		return string.cp32;
	}
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_4BYTE) {
		if (self->up_length + length > WSTR_LENGTH(string.cp32)) {
			/* Must allocate more memory. */
			size_t new_alloc = WSTR_LENGTH(string.cp32);
			do {
				new_alloc *= 2;
			} while (new_alloc < self->up_length + length);
			string.cp32 = DeeString_TryResize4ByteBuffer(string.cp32, new_alloc);
			if unlikely(!string.cp32) {
				string.cp32 = DeeString_TryResize4ByteBuffer((uint32_t *)self->up_buffer,
				                                             self->up_length + length);
				if unlikely(!string.cp32)
					goto err;
			}
			self->up_buffer = string.cp32;
		}
		string.cp32 += self->up_length;
		self->up_length += length;
		return string.cp32;
	}
	return (uint32_t *)Dee_TryMallocc(length + 1, sizeof(uint32_t));
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t *
(DCALL Dee_unicode_printer_tryresize_utf32)(struct unicode_printer *__restrict self,
                                            uint32_t *buf, size_t new_length) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_4BYTE) {
		uint32_t *string;
		size_t old_length, total_avail, new_alloc, old_alloc;
		if (!buf)
			return unicode_printer_tryalloc_utf32(self, new_length);
		string = (uint32_t *)self->up_buffer;
		ASSERT(string != NULL);
		ASSERT(buf >= string);
		ASSERT(buf <= string + self->up_length);

		/* The buffer was allocated in-line. */
		old_length  = (size_t)((string + self->up_length) - buf);
		total_avail = (size_t)((string + WSTR_LENGTH(string)) - buf);
		ASSERT(total_avail >= old_length);
		if (new_length <= total_avail) {
			/* Update the buffer length within the pre-allocated bounds. */
			self->up_length -= old_length;
			self->up_length += new_length;
			return buf;
		}

		/* Must allocate a new buffer. */
		old_alloc = WSTR_LENGTH(string);
		new_alloc = (self->up_length - old_length) + new_length;
		ASSERT(old_alloc < new_alloc);
		ASSERT(old_alloc != 0);
		do {
			old_alloc *= 2;
		} while (old_alloc < new_alloc);

		/* Reallocate the buffer to fit the requested size. */
		string = DeeString_TryResize4ByteBuffer(string, old_alloc);
		if unlikely(!string) {
			string = DeeString_TryResize4ByteBuffer(string, new_alloc);
			if unlikely(!string)
				goto err;
		}

		/* Install the reallocated buffer. */
		self->up_buffer = string;
		self->up_length -= old_length;
		buf = string + self->up_length;
		self->up_length += new_length;
		return buf;
	} else {
		/* The buffer is purely heap-allocated. */
		return (uint32_t *)Dee_TryReallocc(buf, new_length + 1, sizeof(uint32_t));
	}
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t *
(DCALL Dee_unicode_printer_alloc_utf32)(struct unicode_printer *__restrict self,
                                        size_t length) {
	uint32_t *result;
	do {
		result = unicode_printer_tryalloc_utf32(self, length);
	} while (unlikely(!result) && Dee_CollectMemoryc(length, sizeof(uint32_t)));
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t *
(DCALL Dee_unicode_printer_resize_utf32)(struct unicode_printer *__restrict self,
                                         uint32_t *buf, size_t new_length) {
	uint32_t *result;
	do {
		result = unicode_printer_tryresize_utf32(self, buf, new_length);
	} while (unlikely(!result) && Dee_CollectMemoryc(new_length, sizeof(uint32_t)));
	return result;
}

PUBLIC NONNULL((1)) void
(DCALL Dee_unicode_printer_free_utf32)(struct unicode_printer *__restrict self,
                                       uint32_t *buf) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_4BYTE) {
		if (!buf)
			return;
		ASSERT(buf >= (uint32_t *)self->up_buffer);
		ASSERT(buf <= (uint32_t *)self->up_buffer + self->up_length);
		/* Mark the buffer memory as having been freed again. */
		self->up_length = (size_t)((uint32_t *)buf - (uint32_t *)self->up_buffer);
	} else {
		Dee_Free(buf);
	}
}

PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t
(DCALL Dee_unicode_printer_commit_utf32)(struct unicode_printer *__restrict self,
                                         /*inherit(always)*/ uint32_t *buf,
                                         size_t final_length) {
	if (UNICODE_PRINTER_WIDTH(self) == STRING_WIDTH_2BYTE) {
		if (!buf)
			return 0;
		ASSERT(buf >= (uint32_t *)self->up_buffer);
		ASSERT(buf + final_length <= (uint32_t *)self->up_buffer + self->up_length);
		/* Remember the actual length of the buffer. */
		self->up_length = (size_t)(((uint32_t *)buf + final_length) -
		                           ((uint32_t *)self->up_buffer));
		return (Dee_ssize_t)final_length;
	} else {
		Dee_ssize_t result;
		/* Simply print the buffer as UTF-16 text. */
		result = unicode_printer_printutf32(self,
		                                    buf,
		                                    final_length);
		Dee_Free(buf);
		return result;
	}
}







/* Construct a string from the given escape-sequence.
 * This function automatically deals with escaped characters above
 * the single-byte range, and expects the input to be structured as
 * UTF-8 text.
 *  - Surrounding quotation marks should be stripped before calling this function.
 *  - Escaped linefeeds are implicitly parsed, too.
 * @param: error_mode: One of `STRING_ERROR_F*' */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_FromBackslashEscaped(/*utf-8*/ char const *__restrict start,
                               size_t length, unsigned int error_mode) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeString_DecodeBackslashEscaped(&printer, start, length, error_mode))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

/* @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeString_DecodeBackslashEscaped(struct unicode_printer *__restrict printer,
                                 /*utf-8*/ char const *__restrict start,
                                 size_t length, unsigned int error_mode) {
	char const *iter, *end, *flush_start;
	end         = (iter = start) + length;
	flush_start = iter;
	while (iter < end) {
		char ch = *iter;
		uint32_t digit_value;
		if (ch != '\\') {
			++iter;
			continue;
		}
		if unlikely(unicode_printer_print(printer, flush_start,
		                                  (size_t)(iter - flush_start)) < 0)
			goto err;
		++iter;
		ch = *iter++;
		switch (ch) {

		case '\\': /* Escaped the following character itself. */
		case '\'':
		case '\"':
		case '?':
			flush_start = iter - 1;
			continue;

		case '\r':
			if (iter < end && *iter == '\n')
				++iter;
			ATTR_FALLTHROUGH
		case '\n':
			break; /* Escaped line-feed */

		case 'U': {
			unsigned int count;
			unsigned int max_digits;
			max_digits = 8;
			goto parse_hex_integer;
		case 'u':
			max_digits = 4;
			goto parse_hex_integer;
		case 'x':
		case 'X':
			max_digits = (unsigned int)-1; /* Unlimited. (TODO: This is incorrect -- \x should encode actual bytes!) */
parse_hex_integer:
			count       = 0;
			digit_value = 0;
			while (count < max_digits) {
				uint32_t ch32;
				uint8_t val;
				char const *old_iter = iter;
				ch32 = unicode_readutf8_n(&iter, end);
				if (!DeeUni_AsDigit(ch32, 16, &val)) {
					iter = old_iter;
					break;
				}
				digit_value <<= 4;
				digit_value |= val;
				++count;
			}
			if (!count) {
				if (error_mode & (STRING_ERROR_FIGNORE |
				                  STRING_ERROR_FREPLAC))
					goto continue_or_replace;
				DeeError_Throwf(&DeeError_UnicodeDecodeError,
				                "No digits, or hex-chars found after \\x, \\u or \\U");
				goto err;
			}
			if (unicode_printer_putc(printer, digit_value))
				goto err;
		}	break;

		case 'a':
			ch = (char)0x07;
			goto put_ch;

		case 'b':
			ch = (char)0x08;
			goto put_ch;

		case 'f':
			ch = (char)0x0c;
			goto put_ch;

		case 'n':
			ch = (char)0x0a;
			goto put_ch;

		case 'r':
			ch = (char)0x0d;
			goto put_ch;

		case 't':
			ch = (char)0x09;
			goto put_ch;

		case 'v':
			ch = (char)0x0b;
			goto put_ch;

		case 'e':
			ch = (char)0x1b;
			/*goto put_ch;*/
put_ch:
			if (unicode_printer_putc(printer, (uint32_t)(unsigned char)ch))
				goto err;
			break;

		default:
			if (ch >= '0' && ch <= '7') {
				unsigned int count;
				digit_value = (uint32_t)(ch - '0');
parse_oct_integer:
				/* Octal-encoded integer. */
				count = 1;
				while (count < 3) {
					uint8_t digit;
					uint32_t ch32;
					char const *old_iter = iter;
					ch32 = unicode_readutf8_n(&iter, end);
					if (!DeeUni_AsDigit(ch32, 8, &digit)) {
						iter = old_iter;
						break;
					}
					digit_value <<= 3;
					digit_value |= digit;
					++count;
				}
				if (unicode_printer_putc(printer, digit_value))
					goto err;
				break;
			}
			if ((unsigned char)ch >= 0xc0) {
				uint32_t ch32;
				struct unitraits const *desc;
				--iter;
				ch32 = unicode_readutf8_n(&iter, end);
				desc = DeeUni_Descriptor(ch32);
				if (desc->ut_flags & UNICODE_ISLF)
					break; /* Escaped line-feed */
				if (DeeUniTrait_AsDigit(desc, 8, &digit_value))
					goto parse_oct_integer; /* Unicode digit character. */
			}

			/* Fallback: Disregard the character being escaped, and include
			 *           the following character as part of the next flush. */
			flush_start = iter - 1;
continue_or_replace:
			if (error_mode & STRING_ERROR_FIGNORE)
				continue;
			if (error_mode & STRING_ERROR_FREPLAC) {
				if (unicode_printer_putc(printer, '?'))
					goto err;
				continue;
			}
			DeeError_Throwf(&DeeError_UnicodeDecodeError,
			                "Unknown escape character %c",
			                ch);
			goto err;
		}
		flush_start = iter;
	}

	/* Flush the remainder. */
	if unlikely(unicode_printer_print(printer, flush_start,
	                                  (size_t)(end - flush_start)) < 0)
		goto err;
	return 0;
err:
	return -1;
}



/* Append the given `text' to the end of the Bytes object.
 * This function is intended to be used as the general-purpose
 * Dee_formatprinter_t-compatible callback for generating data
 * to-be written into a Bytes object. */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DPRINTER_CC
Dee_bytes_printer_print(void *__restrict self,
                        /*utf-8*/ char const *__restrict text,
                        size_t textlen) {
	uint32_t ch32;
	char const *flush_start;
	size_t result = textlen;
	struct bytes_printer *me;
	me = (struct bytes_printer *)self;
	while (me->bp_numpend) {
		/* Complete a pending UTF-8 sequence. */
		uint8_t reqlen;
		if (!textlen)
			goto done;
		me->bp_pend[me->bp_numpend] = (uint8_t)*text++;
		reqlen = unicode_utf8seqlen[me->bp_pend[0]];
		ASSERT(me->bp_numpend + 1 <= reqlen);
		if (me->bp_numpend + 1 == reqlen) {
			/* Append the full character. */
			ch32 = utf8_getchar(me->bp_pend, reqlen);
			if unlikely(ch32 > 0xff)
				goto err_bytes_too_large;
			if unlikely(bytes_printer_putc(me, (char)(uint8_t)ch32))
				goto err;
			me->bp_numpend = 0;
			break;
		}
		++me->bp_numpend;
	}
again_flush:
	flush_start = text;
	while (textlen && (unsigned char)*text < 0xc0) {
		++text;
		--textlen;
	}

	/* Print ASCII text. */
	if (flush_start < text) {
		if unlikely(bytes_printer_append(me, (uint8_t const *)flush_start,
		                                 (size_t)(text - flush_start)) < 0)
			goto err;
	}
	if (textlen) {
		uint8_t seqlen;
		ASSERT((unsigned char)*text >= 0xc0);
		seqlen = unicode_utf8seqlen[(uint8_t)*text];
		if (seqlen > textlen) {
			/* Incomplete sequence! (safe as pending UTF-8) */
			memcpyb(me->bp_pend, text, textlen);
			me->bp_numpend = (uint8_t)textlen;
			goto done;
		}

		/* Expand the utf-8 sequence and emit it as a single character. */
		ch32 = utf8_getchar((uint8_t const *)text, seqlen);
		if unlikely(ch32 > 0xff)
			goto err_bytes_too_large;
		if unlikely(bytes_printer_putc(me, (char)(uint8_t)ch32))
			goto err;
		text += seqlen;
		textlen -= seqlen;
		goto again_flush;
	}
done:
	return result;
err_bytes_too_large:
	DeeError_Throwf(&DeeError_UnicodeEncodeError,
	                "Unicode character U+%.4" PRFX32 " cannot fit into a single byte",
	                ch32);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_bytes_printer_putc)(struct bytes_printer *__restrict self, char ch) {
	/* Quick check: If the character is apart of the
	 *              ASCII range, just append it as a byte. */
	if likely((uint8_t)ch < 0x80)
		return bytes_printer_putb(self, (uint8_t)ch);

	/* Print the character as a UTF-8 string. */
	return unlikely(bytes_printer_print(self, &ch, 1) < 0) ? -1 : 0;
}

/* Convert an 8, 16, or 32-bit character array to UTF-8 and write it to `printer'
 * NOTE: 8-bit here refers to the unicode range U+0000 - U+00FF */
PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Print8(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                 /*latin-1*/ uint8_t const *__restrict text,
                 size_t textlen) {
	uint8_t const *iter, *end, *flush_start;
	Dee_ssize_t temp, result;
	uint8_t utf8_buffer[2];

	/* Optimizations for special printer targets. */
	if (printer == &bytes_printer_print)
		return bytes_printer_append((struct bytes_printer *)arg, text, textlen);
	if (printer == &unicode_printer_print)
		return unicode_printer_print8((struct unicode_printer *)arg, text, textlen);

	/* Ascii-only data can simply be forwarded one-on-one */
	result = 0;
	end = (iter = flush_start = text) + textlen;
	for (; iter < end; ++iter) {
		if (*iter <= 0x7f)
			continue;
		/* Flush unwritten data. */
		if (flush_start < iter) /* Flush the remainder. */
			DO(err, (*printer)(arg, (char const *)flush_start, (size_t)(iter - flush_start)));

		/* Convert to a 2-wide multi-byte UTF-8 character. */
		utf8_buffer[0] = 0xc0 | ((*iter & 0xc0) >> 6);
		utf8_buffer[1] = 0x80 | (*iter & 0x3f);
		DO(err, (*printer)(arg, (char *)utf8_buffer, 2));
		flush_start = iter + 1;
	}
	if (flush_start < end) /* Flush the remainder. */
		DO(err, (*printer)(arg, (char const *)flush_start, (size_t)(end - flush_start)));
	return result;
err:
	return temp;
}

PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Print16(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-16-without-surrogates*/ uint16_t const *__restrict text,
                  size_t textlen) {
	Dee_ssize_t temp, result = 0;
	uint8_t utf8_buffer[3]; /* TODO: Buffer more than 1 character at-a-time */
	size_t utf8_length;
	if (printer == &unicode_printer_print)
		return unicode_printer_print16((struct unicode_printer *)arg, text, textlen);
	while (textlen--) {
		uint16_t ch = *text++;
		if (ch <= UTF8_1BYTE_MAX) {
			utf8_buffer[0] = (uint8_t)ch;
			utf8_length    = 1;
		} else if (ch <= UTF8_2BYTE_MAX) {
			utf8_buffer[0] = 0xc0 | (uint8_t)((ch >> 6) /* & 0x1f*/);
			utf8_buffer[1] = 0x80 | (uint8_t)((ch)&0x3f);
			utf8_length    = 2;
		} else {
			utf8_buffer[0] = 0xe0 | (uint8_t)((ch >> 12) /* & 0x0f*/);
			utf8_buffer[1] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
			utf8_buffer[2] = 0x80 | (uint8_t)((ch)&0x3f);
			utf8_length    = 3;
		}
		DO(err, (*printer)(arg, (char const *)utf8_buffer, utf8_length));
	}
	return result;
err:
	return temp;
}

PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Print32(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-32*/ uint32_t const *__restrict text,
                  size_t textlen) {
	Dee_ssize_t temp, result = 0;
	size_t utf8_length;
	uint8_t utf8_buffer[UNICODE_UTF8_CURLEN]; /* TODO: Buffer more than 1 character at-a-time */
	if (printer == &unicode_printer_print)
		return unicode_printer_print32((struct unicode_printer *)arg, text, textlen);
	while (textlen--) {
		uint32_t ch = *text++;
		utf8_length = (size_t)((uint8_t *)unicode_writeutf8((char *)utf8_buffer, ch) - utf8_buffer);
		DO(err, (*printer)(arg, (char const *)utf8_buffer, utf8_length));
	}
	return result;
err:
	return temp;
}



/* Helper functions for manipulating strings that have already been created. */
PUBLIC WUNUSED NONNULL((1)) uint32_t
(DCALL DeeString_GetChar)(DeeStringObject *__restrict self, size_t index) {
	union dcharptr_const str;
	struct string_utf *utf;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	utf = self->s_data;
	if (!utf) {
		ASSERT(index <= self->s_len);
		return ((uint8_t *)self->s_str)[index];
	}
	str.ptr = utf->u_data[utf->u_width];
	ASSERT(index <= WSTR_LENGTH(str.ptr));
	SWITCH_SIZEOF_WIDTH(utf->u_width) {

	CASE_WIDTH_1BYTE:
		return str.cp8[index];

	CASE_WIDTH_2BYTE:
		return str.cp16[index];

	CASE_WIDTH_4BYTE:
		return str.cp32[index];
	}
}

PUBLIC NONNULL((1)) void
(DCALL DeeString_SetChar)(DeeStringObject *__restrict self,
                          size_t index, uint32_t value) {
	union dcharptr str;
	struct string_utf *utf;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT(!DeeObject_IsShared(self));
	utf = self->s_data;
	if (!utf) {
		ASSERT((index < self->s_len) ||
		       (index == self->s_len && !value));
		self->s_str[index] = (uint8_t)value;
	} else {
		str.ptr = utf->u_data[utf->u_width];
		ASSERT((index < WSTR_LENGTH(str.ptr)) ||
		       (index == WSTR_LENGTH(str.ptr) && !value));
		SWITCH_SIZEOF_WIDTH(utf->u_width) {

		CASE_WIDTH_1BYTE:
			ASSERT(value <= 0xff);
			ASSERT(str.cp8 == (uint8_t *)DeeString_STR(self));
			str.cp8[index] = (uint8_t)value;
			if (utf->u_utf8 &&
			    utf->u_utf8 != DeeString_STR(self)) {
				ASSERT(utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]);
				Dee_Free((size_t *)utf->u_utf8 - 1);
				utf->u_utf8 = NULL;
			}
			if (utf->u_data[STRING_WIDTH_2BYTE])
				((uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])[index] = (uint16_t)value;
			if (utf->u_data[STRING_WIDTH_4BYTE])
				((uint32_t *)utf->u_data[STRING_WIDTH_4BYTE])[index] = (uint32_t)value;
			if (utf->u_utf16 &&
			    (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
				utf->u_utf16[index] = (uint16_t)value;
			break;

		CASE_WIDTH_2BYTE:
			ASSERT(value <= 0xffff);
			str.cp16[index] = (uint16_t)value;
			if (utf->u_data[STRING_WIDTH_4BYTE]) {
				str.ptr         = utf->u_data[STRING_WIDTH_4BYTE];
				str.cp32[index] = (uint32_t)value;
			}
			goto check_1byte;

		CASE_WIDTH_4BYTE:
			str.cp32[index] = (uint32_t)value;
			if (utf->u_data[STRING_WIDTH_2BYTE]) {
				if ((uint16_t *)utf->u_utf16 == (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
					utf->u_utf16 = NULL;
				Dee_Free(((size_t *)utf->u_data[STRING_WIDTH_2BYTE]) - 1);
				utf->u_data[STRING_WIDTH_2BYTE] = NULL;
			}
			if (utf->u_utf16) {
				ASSERT((uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
				Dee_Free((size_t *)utf->u_utf16 - 1);
				utf->u_utf16 = NULL;
			}
check_1byte:
			if (utf->u_data[STRING_WIDTH_1BYTE]) {
				/* String bytes data. */
				if (utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)self->s_str) {
					ASSERT(DeeString_SIZE(self) == WSTR_LENGTH(str.ptr));
					self->s_str[index] = (char)(uint8_t)value; /* Data loss here cannot be prevented... */
					return;
				}
				if (utf->u_utf8 == (char *)utf->u_data[STRING_WIDTH_1BYTE])
					utf->u_utf8 = NULL;
				Dee_Free((size_t *)utf->u_data[STRING_WIDTH_1BYTE] - 1);
				utf->u_data[STRING_WIDTH_1BYTE] = NULL;
			}
			if (utf->u_utf8) {
				if (utf->u_utf8 == (char *)self->s_str) {
					/* Must update the utf-8 representation. */
					if (DeeString_SIZE(self) == WSTR_LENGTH(str.ptr)) {
						/* No unicode characters. */
						self->s_str[index] = (char)(uint8_t)value; /* Data loss here cannot be prevented... */
					} else {
						/* The difficult case. */
						size_t i = index;
						uint8_t *utf8_dst;
						utf8_dst = (uint8_t *)self->s_str;
						utf8_dst = (uint8_t *)unicode_skiputf8_c(utf8_dst, i);
						switch (unicode_utf8seqlen[*utf8_dst]) {

						case 0:
						case 1:
							*utf8_dst = (uint8_t)value;
							break;

						case 2:
							utf8_dst[0] = 0xc0 | (uint8_t)((value >> 6) /* & 0x1f*/);
							utf8_dst[1] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						case 3:
							utf8_dst[0] = 0xe0 | (uint8_t)((value >> 12) /* & 0x0f*/);
							utf8_dst[1] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
							utf8_dst[2] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						case 4:
							utf8_dst[0] = 0xf0 | (uint8_t)((value >> 18) /* & 0x07*/);
							utf8_dst[1] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
							utf8_dst[2] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
							utf8_dst[3] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						case 5:
							utf8_dst[0] = 0xf8 | (uint8_t)((value >> 24) /* & 0x03*/);
							utf8_dst[1] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
							utf8_dst[2] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
							utf8_dst[3] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
							utf8_dst[4] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						case 6:
							utf8_dst[0] = 0xfc | (uint8_t)((value >> 30) /* & 0x01*/);
							utf8_dst[1] = 0x80 | (uint8_t)((value >> 24) & 0x3f);
							utf8_dst[2] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
							utf8_dst[3] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
							utf8_dst[4] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
							utf8_dst[5] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						case 7:
							utf8_dst[0] = 0xfe;
							utf8_dst[1] = 0x80 | (uint8_t)((value >> 30) & 0x03 /* & 0x3f*/);
							utf8_dst[2] = 0x80 | (uint8_t)((value >> 24) & 0x3f);
							utf8_dst[3] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
							utf8_dst[4] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
							utf8_dst[5] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
							utf8_dst[6] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						case 8:
							utf8_dst[0] = 0xff;
							utf8_dst[1] = 0x80;
							utf8_dst[2] = 0x80 | (uint8_t)((value >> 30) & 0x03 /* & 0x3f*/);
							utf8_dst[3] = 0x80 | (uint8_t)((value >> 24) & 0x3f);
							utf8_dst[4] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
							utf8_dst[5] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
							utf8_dst[6] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
							utf8_dst[7] = 0x80 | (uint8_t)((value)&0x3f);
							break;

						default: __builtin_unreachable();
						}
					}
				} else {
					ASSERT(utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]);
					Dee_Free((size_t *)utf->u_utf8 - 1);
					utf->u_utf8 = NULL;
				}
			}
			break;
		}
	}
}

PUBLIC NONNULL((1)) void
(DCALL DeeString_Memmove)(DeeStringObject *__restrict self,
                          size_t dst, size_t src, size_t num_chars) {
	union dcharptr str;
	struct string_utf *utf;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT(!DeeObject_IsShared(self));
	utf = self->s_data;
	if (!utf) {
		ASSERT((dst + num_chars) <= self->s_len + 1);
		ASSERT((src + num_chars) <= self->s_len + 1);
		memmove(self->s_str + dst,
		        self->s_str + src,
		        num_chars * sizeof(char));
	} else {
		str.ptr = utf->u_data[utf->u_width];
		ASSERT((dst + num_chars) <= WSTR_LENGTH(str.ptr));
		ASSERT((src + num_chars) <= WSTR_LENGTH(str.ptr));
		if (utf->u_utf8 &&
		    utf->u_utf8 != (char *)DeeString_STR(self) &&
		    utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]) {
			ASSERT(str.ptr != utf->u_utf8);
			Dee_Free(((size_t *)utf->u_utf8) - 1);
			utf->u_utf8 = NULL;
		}
		if (utf->u_utf16 &&
		    (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]) {
			ASSERT(str.ptr != utf->u_utf16);
			Dee_Free(((size_t *)utf->u_utf16) - 1);
			utf->u_utf16 = NULL;
		}
		SWITCH_SIZEOF_WIDTH(utf->u_width) {

		CASE_WIDTH_1BYTE:
			memmoveb(str.cp8 + dst, str.cp8 + src, num_chars);
			if (utf->u_data[STRING_WIDTH_2BYTE]) {
				str.ptr = utf->u_data[STRING_WIDTH_2BYTE];
				memmovew(str.cp16 + dst, str.cp16 + src, num_chars);
			}
			if (utf->u_data[STRING_WIDTH_4BYTE]) {
				str.ptr = utf->u_data[STRING_WIDTH_4BYTE];
				memmovel(str.cp32 + dst, str.cp32 + src, num_chars);
			}
			break;

		CASE_WIDTH_2BYTE:
			ASSERT(!utf->u_data[STRING_WIDTH_1BYTE]);
			memmovew(str.cp16 + dst, str.cp16 + src, num_chars);
			if (utf->u_data[STRING_WIDTH_4BYTE]) {
				str.ptr = utf->u_data[STRING_WIDTH_4BYTE];
				memmovel(str.cp32 + dst, str.cp32 + src, num_chars);
			}
			goto check_1byte;

		CASE_WIDTH_4BYTE:
			if (utf->u_data[STRING_WIDTH_2BYTE]) {
				if ((uint16_t *)utf->u_utf16 == (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
					utf->u_utf16 = NULL;
				Dee_Free(((size_t *)utf->u_data[STRING_WIDTH_2BYTE]) - 1);
				utf->u_data[STRING_WIDTH_2BYTE] = NULL;
			}
			if (utf->u_utf16) {
				ASSERT((uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
				Dee_Free((size_t *)utf->u_utf16 - 1);
				utf->u_utf16 = NULL;
			}
			memmovel(str.cp32 + dst, str.cp32 + src, num_chars);
check_1byte:
			if (utf->u_data[STRING_WIDTH_1BYTE]) {
				/* String bytes data. */
				if (utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)self->s_str) {
					ASSERT(DeeString_SIZE(self) == WSTR_LENGTH(str.ptr));
					memmovec(self->s_str + dst,
					         self->s_str + src,
					         num_chars, sizeof(char));
					return;
				}
				if (utf->u_utf8 == (char *)utf->u_data[STRING_WIDTH_1BYTE])
					utf->u_utf8 = NULL;
				Dee_Free((size_t *)utf->u_data[STRING_WIDTH_1BYTE] - 1);
				utf->u_data[STRING_WIDTH_1BYTE] = NULL;
			}
			if (utf->u_utf8) {
				if (utf->u_utf8 == (char *)self->s_str) {
					/* Must update the utf-8 representation. */
					if (DeeString_SIZE(self) == WSTR_LENGTH(str.ptr)) {
						/* No unicode character. -> We can simply memmove the UTF-8 variable to update it. */
						memmovec(self->s_str + dst,
						         self->s_str + src,
						         num_chars, sizeof(char));
					} else {
						/* The difficult case. */
						char *utf8_src, *utf8_dst, *end;
						size_t i;
						if (dst < src) {
							utf8_dst = unicode_skiputf8_c(self->s_str, dst);
							utf8_src = unicode_skiputf8_c(utf8_dst, src - dst);
							end      = unicode_skiputf8_c(utf8_src, num_chars);
						} else {
							utf8_src = unicode_skiputf8_c(self->s_str, dst);
							utf8_dst = utf8_src;
							i        = dst - src;
							if (num_chars > i) {
								utf8_dst = unicode_skiputf8_c(utf8_dst, i);
								end      = unicode_skiputf8_c(utf8_dst, num_chars - (dst - src));
							} else {
								end = NULL;
								while (i--) {
									if (!num_chars--)
										end = utf8_dst;
									utf8_dst = unicode_skiputf8(utf8_dst);
								}
								ASSERT(end != NULL);
							}
						}
						memmovec(utf8_dst,
						         utf8_src,
						         (size_t)(end - utf8_src),
						         sizeof(char));
					}
				} else {
					ASSERT(utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]);
					Dee_Free((size_t *)utf->u_utf8 - 1);
					utf->u_utf8 = NULL;
				}
			}
			break;

		}
	}
}

#undef DO

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C */
