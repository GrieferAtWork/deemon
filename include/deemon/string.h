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
#ifndef GUARD_DEEMON_STRING_H
#define GUARD_DEEMON_STRING_H 1

#include "api.h"
/**/

#include "types.h"
#ifndef __INTELLISENSE__
#include "alloc.h"
#include "object.h"
#endif /* !__INTELLISENSE__ */
/**/

#include <hybrid/__overflow.h>
#include <hybrid/byteorder.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
/**/

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uintX_t, uintptr_t */

#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#elif !defined(CONFIG_HAVE_STRING_H) && \
      (defined(__NO_has_include) || __has_include(<string.h>))
#define CONFIG_HAVE_STRING_H
#endif

#ifdef CONFIG_NO_STRINGS_H
#undef CONFIG_HAVE_STRINGS_H
#elif !defined(CONFIG_HAVE_STRINGS_H) && \
      (__has_include(<strings.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_STRINGS_H
#endif

#ifdef CONFIG_HAVE_STRING_H
#include <string.h>
#endif /* CONFIG_HAVE_STRING_H */

#ifdef CONFIG_HAVE_STRINGS_H
#include <strings.h>
#endif /* CONFIG_HAVE_STRINGS_H */

#ifdef CONFIG_NO_strlen
#undef CONFIG_HAVE_strlen
#else
#define CONFIG_HAVE_strlen
#endif

#ifdef CONFIG_NO_memcmp
#undef CONFIG_HAVE_memcmp
#else
#define CONFIG_HAVE_memcmp
#endif

#ifdef CONFIG_NO_bcmp
#undef CONFIG_HAVE_bcmp
#elif !defined(CONFIG_HAVE_bcmp) && \
      (defined(bcmp) || defined(__bcmp_defined) || defined(CONFIG_HAVE_STRINGS_H))
#define CONFIG_HAVE_bcmp
#endif

#ifdef CONFIG_NO_UNICODE_H
#undef CONFIG_HAVE_UNICODE_H
#elif !defined(CONFIG_HAVE_UNICODE_H) && \
      (__has_include(<unicode.h>) || (defined(__NO_has_include) && \
      (defined(__KOS__) && __KOS_VERSION__ >= 400)))
#define CONFIG_HAVE_UNICODE_H
#endif

#ifdef CONFIG_HAVE_UNICODE_H
#include <unicode.h>
#if (!defined(__unicode_utf8seqlen_defined) ||                         \
     !defined(__UNICODE_ISCNTRL) || !defined(unicode_iscntrl) ||       \
     !defined(__UNICODE_ISCTAB) || !defined(unicode_isctab) ||         \
     !defined(__UNICODE_ISXTAB) || !defined(unicode_isxtab) ||         \
     !defined(__UNICODE_ISTAB) || !defined(unicode_istab) ||           \
     !defined(__UNICODE_ISWHITE) || !defined(unicode_iswhite) ||       \
     !defined(__UNICODE_ISEMPTY) || !defined(unicode_isempty) ||       \
     !defined(__UNICODE_ISLF) || !defined(unicode_islf) ||             \
     !defined(__UNICODE_ISSPACE) || !defined(unicode_isspace) ||       \
     !defined(__UNICODE_ISLOWER) || !defined(unicode_islower) ||       \
     !defined(__UNICODE_ISUPPER) || !defined(unicode_isupper) ||       \
     !defined(__UNICODE_ISXALPHA) || !defined(unicode_isxalpha) ||     \
     !defined(__UNICODE_ISALPHA) || !defined(unicode_isalpha) ||       \
     !defined(__UNICODE_ISDIGIT) || !defined(unicode_isdigit) ||       \
     !defined(__UNICODE_ISHEX) || !defined(unicode_ishex) ||           \
     !defined(__UNICODE_ISXDIGIT) || !defined(unicode_isxdigit) ||     \
     !defined(__UNICODE_ISALNUM) || !defined(unicode_isalnum) ||       \
     !defined(__UNICODE_ISPUNCT) || !defined(unicode_ispunct) ||       \
     !defined(__UNICODE_ISGRAPH) || !defined(unicode_isgraph) ||       \
     !defined(__UNICODE_ISPRINT) || !defined(unicode_isprint) ||       \
     !defined(__UNICODE_ISBLANK) || !defined(unicode_isblank) ||       \
     !defined(__UNICODE_ISTITLE) || !defined(unicode_istitle) ||       \
     !defined(__UNICODE_ISXNUMERIC) || !defined(unicode_isxnumeric) || \
     !defined(__UNICODE_ISNUMERIC) || !defined(unicode_isnumeric) ||   \
     !defined(__UNICODE_ISSYMSTRT) || !defined(unicode_issymstrt) ||   \
     !defined(__UNICODE_ISSYMCONT) || !defined(unicode_issymcont) ||   \
     !defined(____unicode_descriptor_defined) ||                       \
     !defined(__CRT_HAVE_unicode_fold) ||                              \
     !defined(UNICODE_FOLDED_MAX))
#undef CONFIG_HAVE_UNICODE_H
#define CONFIG_NO_UNICODE_H
#endif /* !... */
#endif /* CONFIG_HAVE_UNICODE_H */


DECL_BEGIN

#ifndef CONFIG_HAVE_strlen
#define CONFIG_HAVE_strlen
#undef strlen
#define strlen dee_strlen
LOCAL WUNUSED NONNULL((1)) size_t dee_strlen(char const *str) {
	size_t result;
	for (result = 0; str[result]; ++result)
		;
	return result;
}
#endif /* !CONFIG_HAVE_strlen */

#ifndef CONFIG_HAVE_memcmp
#define CONFIG_HAVE_memcmp
#undef memcmp
#define memcmp dee_memcmp
LOCAL WUNUSED NONNULL((1, 2)) int
dee_memcmp(void const *s1, void const *s2, size_t n) {
	uint8_t const *p1 = (uint8_t const *)s1;
	uint8_t const *p2 = (uint8_t const *)s2;
	while (n--) {
		uint8_t v1, v2;
		if ((v1 = *p1++) != (v2 = *p2++)) {
			return v1 < v2 ? -1 : 1;
		}
	}
	return 0;
}
#endif /* !CONFIG_HAVE_memcmp */

#ifndef CONFIG_HAVE_bcmp
#define CONFIG_HAVE_bcmp
#undef bcmp
#define bcmp(s1, s2, num_bytes) memcmp(s1, s2, num_bytes)
#endif /* !CONFIG_HAVE_bcmp */


/* Explanation of WIDTH-STRINGS vs. UTF-STRINGS:
 *
 *  - 1-byte string: 100% the same as LATIN-1
 *    Only able to encode the unicode character range U+00-U+FF
 *    Generated and returned by `DeeString_Bytes()', which
 *    will encode characters outside that range as '?'
 *  - UTF-8 string:
 *    A multi-byte / variable-character-width string capable
 *    of encoding the entirety of the unicode range, as well
 *    as share binary compatibility to ASCII.
 *    For strings containing only characters <= 0x7F, this is
 *    identical to a 1-byte string.
 *
 *  - 2-byte string:
 *    Encodes all characters in the address range U+0000-U+FFFF.
 *    Unlike for 1-byte strings, there is no way to force this
 *    representation to be generated (although such a function
 *    could easily be added as an afterthought)
 *    Strings may indicate their width to be 2-bytes, even if
 *    all contained characters fit into the 1-byte range.
 *    In this case, `DeeString_Bytes()' still functions normally,
 *    and will succeed.
 *  - UTF-16 string:
 *    A 1/2-word / variable-character-width string that can encode
 *    the unicode range U+000000 - U+10FFFF. Characters outside that
 *    range cannot be represented, despite being able to be referred
 *    to by UTF-8
 *
 *  - 4-byte string:
 *    All characters are encoded in the address range U+00000000-U+FFFFFFFF.
 *    Strings may indicate their width to be 4-bytes, even if
 *    all contained characters fit into the 2-, or 1-byte range.
 *    In this case, `DeeString_Bytes()' still functions normally,
 *    and will succeed for 1-byte ranges.
 *  - UTF-32 string:
 *    Literally the same as a 4-byte string.
 *
 */


#ifdef DEE_SOURCE
#define Dee_ascii_printer   ascii_printer
#define Dee_unicode_printer unicode_printer
#define Dee_string_object   string_object
#define Dee_string_utf      string_utf
#define Dee_unitraits       unitraits
#define Dee_regex_range     regex_range
#define Dee_regex_range_ex  regex_range_ex
#define Dee_regex_range_ptr regex_range_ptr
#endif /* DEE_SOURCE */


struct Dee_string_utf;
struct Dee_ascii_printer;
struct Dee_unicode_printer;
typedef struct Dee_string_object DeeStringObject;

#ifndef __SIZEOF_WCHAR_T__
#error "__SIZEOF_WCHAR_T__ should have been defined by <hybrid/typecore.h>"
#endif /* !__SIZEOF_WCHAR_T__ */

#ifndef __WCHAR_TYPE__
#error "__WCHAR_TYPE__ should have been defined by <hybrid/typecore.h>"
#endif /* !__WCHAR_TYPE__ */

#ifdef __native_wchar_t_defined
typedef wchar_t Dee_wchar_t;
#else /* __native_wchar_t_defined */
typedef __WCHAR_TYPE__ Dee_wchar_t;
#endif /* !__native_wchar_t_defined */
#ifdef DEE_SOURCE
#define Dee_charptr       dcharptr
#define Dee_charptr_const dcharptr_const
#endif /* DEE_SOURCE */

#ifndef DEE_CHARPTR_DEFINED
#define DEE_CHARPTR_DEFINED
union Dee_charptr {
	void        *ptr;
	uint8_t     *cp8;
	uint16_t    *cp16;
	uint32_t    *cp32;
	char        *cp_char;
	Dee_wchar_t *cp_wchar;
};
union Dee_charptr_const {
	void        const *ptr;
	uint8_t     const *cp8;
	uint16_t    const *cp16;
	uint32_t    const *cp32;
	char        const *cp_char;
	Dee_wchar_t const *cp_wchar;
};
#endif /* !DEE_CHARPTR_DEFINED */



#define Dee_STRING_WIDTH_1BYTE 0u /* All characters are within the range U+0000 - U+00FF (LATIN-1) */
#define Dee_STRING_WIDTH_2BYTE 1u /* All characters are within the range U+0000 - U+FFFF (BMP) */
#define Dee_STRING_WIDTH_4BYTE 2u /* All characters are within the range U+0000 - U+10FFFF (Full unicode; UTF-32) */
#define Dee_STRING_WIDTH_COUNT 3u /* the number of of known string width encodings. */

/* 
 *   00 | 00  -> 00 == a|b
 *   00 | 01  -> 01 == a|b
 *   00 | 10  -> 10 == a|b
 *   01 | 00  -> 01 == a|b
 *   01 | 01  -> 01 == a|b
 *   01 | 10  -> 10 == a|b & ~1
 *   10 | 00  -> 10 == a|b
 *   10 | 01  -> 10 == a|b & ~1
 *   10 | 10  -> 10 == a|b
 */
#ifndef __NO_XBLOCK
#define Dee_STRING_WIDTH_COMMON(x, y) \
	XBLOCK({ unsigned int _x=(x), _y=(y); XRETURN _x >= _y ? _x : _y; })
#define Dee_STRING_WIDTH_COMMON3(x, y, z) \
	XBLOCK({ unsigned int _x=(x), _y=(y), _z=(z); XRETURN _x >= _y ? (_x >= _z ? _x : _z) : (_y >= _z ? _y : _z); })
#elif !defined(__NO_ATTR_FORCEINLINE) && (!defined(_MSC_VER) || defined(NDEBUG))
LOCAL ATTR_CONST unsigned int DCALL
_Dee_string_width_common(unsigned int x, unsigned int y) {
	return x >= y ? x : y;
}

LOCAL ATTR_CONST unsigned int DCALL
_Dee_string_width_common3(unsigned int x, unsigned int y, unsigned int z) {
	return x >= y ? (x >= z ? x : z) : (y >= z ? y : z);
}
#define Dee_STRING_WIDTH_COMMON(x, y)     _Dee_string_width_common(x, y)
#define Dee_STRING_WIDTH_COMMON3(x, y, z) _Dee_string_width_common3(x, y, z)
#else /* !__NO_XBLOCK */
#define Dee_STRING_WIDTH_COMMON(x, y)     ((x) >= (y) ? (x) : (y))
#define Dee_STRING_WIDTH_COMMON3(x, y, z) ((x) >= (y) ? Dee_STRING_WIDTH_COMMON(x, z) : Dee_STRING_WIDTH_COMMON(y, z))
#endif /* __NO_XBLOCK */


/* Encoding error flags. */
#define Dee_STRING_ERROR_FNORMAL 0x0000 /* Normal string decoding. */
#define Dee_STRING_ERROR_FSTRICT 0x0000 /* Throw `Error.UnicodeError.UnicodeDecodeError' when something goes wrong. */
#define Dee_STRING_ERROR_FREPLAC 0x0001 /* Replace bad characters with `?'. */
#define Dee_STRING_ERROR_FIGNORE 0x0002 /* Ignore (truncate or drop) bad characters. */

/* Returns the length of a width-string (which is any string obtained from a `string' object) */
#define Dee_WSTR_LENGTH(x) (((size_t *)(x))[-1])

/* Given the character-width `width', the base address `base', and the
 * index `index', return the unicode character found at that location */
#define Dee_STRING_WIDTH_GETCHAR(width, base, index) \
	(likely((width) == Dee_STRING_WIDTH_1BYTE)       \
	 ? (uint32_t)((uint8_t const *)(base))[index]    \
	 : ((width) == Dee_STRING_WIDTH_2BYTE)           \
	   ? (uint32_t)((uint16_t const *)(base))[index] \
	   : ((uint32_t const *)(base))[index])

/* Given the character-width `width', the base address `base', and the
 * index `index', set the unicode character found at that location to `value' */
#define Dee_STRING_WIDTH_SETCHAR(width, base, index, value)      \
	(likely((width) == Dee_STRING_WIDTH_1BYTE)                   \
	 ? (void)(((uint8_t *)(base))[index] = (uint8_t)(value))     \
	 : ((width) == Dee_STRING_WIDTH_2BYTE)                       \
	   ? (void)(((uint16_t *)(base))[index] = (uint16_t)(value)) \
	   : (void)(((uint32_t *)(base))[index] = (uint32_t)(value)))

/* Return the width (in bytes) of a string width `width' */
#define Dee_STRING_SIZEOF_WIDTH(width) ((size_t)1 << (width))

/* Return the result of `x * Dee_STRING_SIZEOF_WIDTH(width)' */
#define Dee_STRING_MUL_SIZEOF_WIDTH(x, width) ((size_t)(x) << (width))
#define Dee_STRING_DIV_SIZEOF_WIDTH(x, width) ((size_t)(x) >> (width))

#ifndef __NO_builtin_expect
#define Dee_SWITCH_SIZEOF_WIDTH(x) switch (__builtin_expect(x, Dee_STRING_WIDTH_1BYTE))
#else /* !__NO_builtin_expect */
#define Dee_SWITCH_SIZEOF_WIDTH(x) switch (x)
#endif /* __NO_builtin_expect */

#ifndef __NO_builtin_unreachable
#define Dee_CASE_WIDTH_1BYTE default: __builtin_unreachable(); \
                             case Dee_STRING_WIDTH_1BYTE
#else /* !__NO_builtin_unreachable */
#define Dee_CASE_WIDTH_1BYTE default
#endif /* __NO_builtin_unreachable */
#define Dee_CASE_WIDTH_2BYTE case Dee_STRING_WIDTH_2BYTE
#define Dee_CASE_WIDTH_4BYTE case Dee_STRING_WIDTH_4BYTE


/* String flags */
#define Dee_STRING_UTF_FNORMAL 0x0000 /* Normal UTF flags */
#define Dee_STRING_UTF_FASCII  0x0001 /* FLAG: The string contains no character outside the ASCII range.
                                       * NOTE: This flag isn't required to be set, even if there are only ASCII characters. */
#define Dee_STRING_UTF_FINVBYT 0x0002 /* FLAG: The string in `u_data[Dee_STRING_WIDTH_1BYTE]' contains truncated characters (represented as `?') */
#define Dee_STRING_UTF_FREGEX  0x0004 /* FLAG: The string appears in the regex cache */


#ifdef DEE_SOURCE
#define STRING_WIDTH_1BYTE      Dee_STRING_WIDTH_1BYTE
#define STRING_WIDTH_2BYTE      Dee_STRING_WIDTH_2BYTE
#define STRING_WIDTH_4BYTE      Dee_STRING_WIDTH_4BYTE
#define STRING_WIDTH_COUNT      Dee_STRING_WIDTH_COUNT
#define STRING_WIDTH_COMMON     Dee_STRING_WIDTH_COMMON
#define STRING_WIDTH_COMMON3    Dee_STRING_WIDTH_COMMON3
#define STRING_ERROR_FNORMAL    Dee_STRING_ERROR_FNORMAL
#define STRING_ERROR_FSTRICT    Dee_STRING_ERROR_FSTRICT
#define STRING_ERROR_FREPLAC    Dee_STRING_ERROR_FREPLAC
#define STRING_ERROR_FIGNORE    Dee_STRING_ERROR_FIGNORE
#define WSTR_LENGTH             Dee_WSTR_LENGTH
#define STRING_WIDTH_GETCHAR    Dee_STRING_WIDTH_GETCHAR
#define STRING_WIDTH_SETCHAR    Dee_STRING_WIDTH_SETCHAR
#define STRING_SIZEOF_WIDTH     Dee_STRING_SIZEOF_WIDTH
#define STRING_MUL_SIZEOF_WIDTH Dee_STRING_MUL_SIZEOF_WIDTH
#define SWITCH_SIZEOF_WIDTH     Dee_SWITCH_SIZEOF_WIDTH
#define CASE_WIDTH_1BYTE        Dee_CASE_WIDTH_1BYTE
#define CASE_WIDTH_2BYTE        Dee_CASE_WIDTH_2BYTE
#define CASE_WIDTH_4BYTE        Dee_CASE_WIDTH_4BYTE
#define STRING_UTF_FNORMAL      Dee_STRING_UTF_FNORMAL
#define STRING_UTF_FASCII       Dee_STRING_UTF_FASCII
#define STRING_UTF_FINVBYT      Dee_STRING_UTF_FINVBYT
#define STRING_UTF_FREGEX       Dee_STRING_UTF_FREGEX
#endif /* DEE_SOURCE */

struct Dee_string_utf {
	__UINTPTR_HALF_TYPE__ u_width; /* [const] The minimum encoding size (One of `STRING_WIDTH_*').
	                                *         Also used as index into the encoding-data vector below.
	                                *         NOTE: The data-representation indexed by this is _always_ allocated! */
	__UINTPTR_HALF_TYPE__ u_flags; /* [const] UTF flags (Set of `STRING_UTF_F*') */
	size_t      *u_data[Dee_STRING_WIDTH_COUNT]; /* [0..1][owned][lock(WRITE_ONCE)][*]
	                                              * Multi-byte string variant.
	                                              * Variants at indices `>= u_width' can always be accessed without
	                                              * any problems, with the version at `u_width' even being guarantied
	                                              * to be 1..1 (that version is returned by `DeeString_WSTR()').
	                                              * Accessing lower-order variants requires the string to be cast into
	                                              * that width-class, with the result being that characters which don't fit
	                                              * into that class are replaced by `?', potentially causing a DecodeError.
	                                              * >> Dee_ASSERT(u_data[u_width] != NULL);
	                                              * >> Dee_ASSERT(!u_data[Dee_STRING_WIDTH_1BYTE] || Dee_WSTR_LENGTH(u_data[u_width]) == Dee_WSTR_LENGTH(u_data[Dee_STRING_WIDTH_1BYTE]));
	                                              * >> Dee_ASSERT(!u_data[Dee_STRING_WIDTH_2BYTE] || Dee_WSTR_LENGTH(u_data[u_width]) == Dee_WSTR_LENGTH(u_data[Dee_STRING_WIDTH_2BYTE]));
	                                              * >> Dee_ASSERT(!u_data[Dee_STRING_WIDTH_4BYTE] || Dee_WSTR_LENGTH(u_data[u_width]) == Dee_WSTR_LENGTH(u_data[Dee_STRING_WIDTH_4BYTE]));
	                                              * >> if (u_width == Dee_STRING_WIDTH_1BYTE) {
	                                              * >>     Dee_ASSERT(u_data[Dee_STRING_WIDTH_1BYTE] == :s_str);
	                                              * >>     Dee_ASSERT(!u_utf16 || u_utf16 == u_data[Dee_STRING_WIDTH_2BYTE]);
	                                              * >> }
	                                              * >> if (u_flags & Dee_STRING_UTF_FASCII)
	                                              * >>     Dee_ASSERT(u_utf8 == :s_str); */
	char        *u_utf8;  /* [0..1][lock(WRITE_ONCE)][owned_if(!= :s_str)]
	                       * A lazily allocated width-string (meaning you can use `Dee_WSTR_LENGTH' to
	                       * determine its length), representing the UTF-8 variant of this string. */
#if __SIZEOF_WCHAR_T__ == 2
#define _Dee_string_utf_utf16_t Dee_wchar_t
	Dee_wchar_t *u_utf16; /* [0..1][lock(WRITE_ONCE)][owned_if(!= u_data[Dee_STRING_WIDTH_2BYTE])]
	                       * A lazily allocated width-string (meaning you can use `Dee_WSTR_LENGTH' to
	                       * determine its length), representing the UTF-16 variant of this string. */
#else /* __SIZEOF_WCHAR_T__ == 2 */
#define _Dee_string_utf_utf16_t uint16_t
	uint16_t    *u_utf16; /* [0..1][lock(WRITE_ONCE)][owned_if(!= u_data[Dee_STRING_WIDTH_2BYTE])]
	                       * A lazily allocated width-string (meaning you can use `Dee_WSTR_LENGTH' to
	                       * determine its length), representing the UTF-16 variant of this string. */
#endif /* __SIZEOF_WCHAR_T__ != 2 */
};

#define Dee_string_utf_fini(self, str)                                                                            \
	do {                                                                                                          \
		unsigned int _i;                                                                                          \
		for (_i = 1; _i < STRING_WIDTH_COUNT; ++_i) {                                                             \
			if ((self)->u_data[_i])                                                                               \
				Dee_Free((self)->u_data[_i] - 1);                                                                 \
		}                                                                                                         \
		if ((self)->u_data[Dee_STRING_WIDTH_1BYTE] &&                                                             \
		    (self)->u_data[Dee_STRING_WIDTH_1BYTE] != (size_t *)DeeString_STR(str))                               \
			Dee_Free(((size_t *)(self)->u_data[Dee_STRING_WIDTH_1BYTE]) - 1);                                     \
		if ((self)->u_utf8 && (self)->u_utf8 != (char *)DeeString_STR(str) &&                                     \
		    (self)->u_utf8 != (char *)(self)->u_data[Dee_STRING_WIDTH_1BYTE])                                     \
			Dee_Free(((size_t *)(self)->u_utf8) - 1);                                                             \
		if ((self)->u_utf16 && (uint16_t *)(self)->u_utf16 != (uint16_t *)(self)->u_data[Dee_STRING_WIDTH_2BYTE]) \
			Dee_Free(((size_t *)(self)->u_utf16) - 1);                                                            \
	}	__WHILE0

#if 1
#define Dee_string_utf_alloc()                     DeeObject_MALLOC(struct Dee_string_utf)
#define Dee_string_utf_tryalloc()                  DeeObject_TRYMALLOC(struct Dee_string_utf)
#define Dee_string_utf_free(ptr)                   DeeObject_FFree(ptr, sizeof(struct Dee_string_utf))
#define Dee_string_utf_untrack(ptr)                (struct Dee_string_utf *)DeeObject_UntrackAlloc(ptr)
#define DeeDbg_string_utf_alloc(file, line)        DeeDbgObject_MALLOC(struct Dee_string_utf, file, line)
#define DeeDbg_string_utf_tryalloc(file, line)     DeeDbgObject_TRYMALLOC(struct Dee_string_utf, file, line)
#define DeeDbg_string_utf_free(ptr, file, line)    DeeDbgObject_FFree(ptr, sizeof(struct Dee_string_utf), file, line)
#define DeeDbg_string_utf_untrack(ptr, file, line) DeeDbgObject_UntrackAlloc(ptr, file, line)
#else
#define Dee_string_utf_alloc()                     ((struct Dee_string_utf *)Dee_Malloc(sizeof(struct Dee_string_utf)))
#define Dee_string_utf_tryalloc()                  ((struct Dee_string_utf *)Dee_TryMalloc(sizeof(struct Dee_string_utf)))
#define Dee_string_utf_free(ptr)                   Dee_Free(ptr)
#define Dee_string_utf_untrack(ptr)                (struct Dee_string_utf *)Dee_UntrackAlloc(utf)
#define DeeDbg_string_utf_alloc(file, line)        ((struct Dee_string_utf *)DeeDbg_Malloc(sizeof(struct Dee_string_utf), file, line))
#define DeeDbg_string_utf_tryalloc(file, line)     ((struct Dee_string_utf *)DeeDbg_TryMalloc(sizeof(struct Dee_string_utf), file, line))
#define DeeDbg_string_utf_free(ptr, file, line)    DeeDbg_Free(ptr, file, line)
#define DeeDbg_string_utf_untrack(ptr, file, line) DeeDbg_UntrackAlloc(utf, file, line)
#endif


struct Dee_string_object {
	Dee_OBJECT_HEAD
	struct Dee_string_utf  *s_data; /* [0..1][owned][lock(WRITE_ONCE)]
	                                 * Extended string data, as well as unicode information & lazily allocated caches. */
	Dee_hash_t              s_hash; /* [valid_if(!= DEE_STRING_HASH_UNSET)][lock(WRITE_ONCE)] The string's hash. */
#define DEE_STRING_HASH_UNSET ((Dee_hash_t)-1) /* A hash-representation of the string. */
	size_t                  s_len;  /* [const] The number of bytes found in the single-byte string text. */
	COMPILER_FLEXIBLE_ARRAY(/*unsigned*/ char,
	                        s_str); /* [const][s_len] The single-byte string text.
	                                 * This string is either encoded as UTF-8, when the string's character
	                                 * width isn't 1 byte, or encoded as LATIN-1, if all characters fit
	                                 * within the unicode range U+0000 - U+00FF, though in this case it is
	                                 * also possible for this string to also be UTF-8 (iow: here, it is
	                                 * ambiguous)
	                                 * Which of these 2 is the case can be determined with:
	                                 * - DeeString_STR_ISUTF8
	                                 * - DeeString_STR_ISLATIN1
	                                 */
};

#define Dee_DEFINE_STRING(name, str)                              \
	struct {                                                      \
		Dee_OBJECT_HEAD                                           \
		struct Dee_string_utf *s_data;                            \
		Dee_hash_t             s_hash;                            \
		size_t                 s_len;                             \
		char                   s_str[sizeof(str) / sizeof(char)]; \
		struct Dee_string_utf  s_utf;                             \
	} name = {                                                    \
		Dee_OBJECT_HEAD_INIT(&DeeString_Type),                    \
		&name.s_utf,                                              \
		DEE_STRING_HASH_UNSET,                                    \
		(sizeof(str) / sizeof(char)) - 1,                         \
		str,                                                      \
		{ Dee_STRING_WIDTH_1BYTE,                                 \
		  Dee_STRING_UTF_FASCII,                                  \
		  { (size_t *)name.s_str },                               \
		  name.s_str }                                            \
	}
#define Dee_DEFINE_STRING_EX(name, str, hash32, hash64)           \
	struct {                                                      \
		Dee_OBJECT_HEAD                                           \
		struct Dee_string_utf *s_data;                            \
		Dee_hash_t             s_hash;                            \
		size_t                 s_len;                             \
		char                   s_str[sizeof(str) / sizeof(char)]; \
		struct Dee_string_utf  s_utf;                             \
	} name = {                                                    \
		Dee_OBJECT_HEAD_INIT(&DeeString_Type),                    \
		&name.s_utf,                                              \
		_Dee_HashSelectC(hash32, hash64),                         \
		(sizeof(str) / sizeof(char)) - 1,                         \
		str,                                                      \
		{ Dee_STRING_WIDTH_1BYTE,                                 \
		  Dee_STRING_UTF_FASCII,                                  \
		  { (size_t *)name.s_str },                               \
		  name.s_str }                                            \
	}

struct Dee_empty_string_struct {
	Dee_OBJECT_HEAD
	struct Dee_string_utf *s_data;
	Dee_hash_t             s_hash;
	size_t                 s_len;
	char                   s_zero;
};

DDATDEF struct Dee_empty_string_struct DeeString_Empty;
#define Dee_EmptyString ((DeeObject *)&DeeString_Empty)
#ifdef __INTELLISENSE__
#define DeeString_NewEmpty() Dee_EmptyString
#else /* __INTELLISENSE__ */
#define DeeString_NewEmpty() (Dee_Incref(&DeeString_Empty), Dee_EmptyString)
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#define DEFINE_STRING    Dee_DEFINE_STRING
#define DEFINE_STRING_EX Dee_DEFINE_STRING_EX
#endif /* DEE_SOURCE */


DDATDEF DeeTypeObject DeeString_Type; /* `string from deemon' */
#define DeeString_Check(x)      DeeObject_InstanceOfExact(x, &DeeString_Type)
#define DeeString_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeString_Type)


/* Return a pointer to the multi-byte encoded variant of the given string.
 * Characters <= 127 follow ASCII characters, while the meaning of characters
 * above this depends on `DeeString_WIDTH()':
 *   - Dee_STRING_WIDTH_1BYTE:
 *      - If there are no characters above 127, the string is pure ASCII
 *      - Otherwise, the string may have been created from raw, non-decoded
 *        data, and might potentially not even contain readable text.
 *      - Another possibility is that the string follows a host-specific
 *        ANSI character set, where characters above 127 have host-specific
 *        meaning.
 *     -> In either case, any character above 127 should be treated as part
 *        of the unicode character range U+0080...U+00FF, meaning that the
 *        string should behave like any ordinary unicode string.
 *   - Dee_STRING_WIDTH_2BYTE:
 *      - `DeeString_STR()' is the multi-byte, UTF-8 form of a UTF-16 string
 *        found in `x->s_data->u_data[Dee_STRING_WIDTH_2BYTE]', while `DeeString_SIZE()'
 *        refers to the number of bytes used by the multi-byte string.
 *   - Dee_STRING_WIDTH_4BYTE:
 *      - `DeeString_STR()' is the multi-byte, UTF-8 form of a UTF-32 string
 *        found in `x->s_data->u_data[Dee_STRING_WIDTH_4BYTE]', while `DeeString_SIZE()'
 *        refers to the number of bytes used by the multi-byte string.
 */
#define DeeString_STR(x)  ((DeeStringObject const *)Dee_REQUIRES_OBJECT(x))->s_str
#define DeeString_SIZE(x) ((DeeStringObject const *)Dee_REQUIRES_OBJECT(x))->s_len
#define DeeString_END(x)  (((DeeStringObject const *)Dee_REQUIRES_OBJECT(x))->s_str + ((DeeStringObject const *)(x))->s_len)


#define DeeString_STR8(x)  (((DeeStringObject const *)Dee_REQUIRES_OBJECT(x))->s_data ? (uint8_t const *)((DeeStringObject const *)(x))->s_data->u_data[Dee_STRING_WIDTH_1BYTE] : (uint8_t const *)((DeeStringObject const *)(x))->s_str)
#define DeeString_STR16(x) ((uint16_t const *)((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data->u_data[Dee_STRING_WIDTH_2BYTE])
#define DeeString_STR32(x) ((uint32_t const *)((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data->u_data[Dee_STRING_WIDTH_4BYTE])
#define DeeString_LEN8(x)  (((DeeStringObject const *)Dee_REQUIRES_OBJECT(x))->s_data ? Dee_WSTR_LENGTH(((DeeStringObject *)(x))->s_data->u_data[Dee_STRING_WIDTH_1BYTE]) : ((DeeStringObject const *)(x))->s_len)
#define DeeString_LEN16(x) Dee_WSTR_LENGTH(DeeString_STR16(x))
#define DeeString_LEN32(x) Dee_WSTR_LENGTH(DeeString_STR32(x))

/* Returns true if `DeeString_STR()' is encoded in UTF-8
 * HINT: This is very useful when creating transformed sub-strings
 *       of another string, when caring about encodings, but only
 *       working within the ASCII-range:
 *    >> DREF DeeObject *partition_after(DeeObject *__restrict str, char ch) {
 *    >>     DREF DeeObject *result;
 *    >>     char const *pos;
 *    >>     pos = (char const *)memchr(DeeString_STR(str), ch,
 *    >>                                DeeString_SIZE(str));
 *    >>     if (!pos)
 *    >>         return_none;
 *    >>     // Return the sub-string after `ch'
 *    >>     ++pos;
 *    >>     result = DeeString_NewSized(pos, (size_t)(DeeString_END(str) - pos));
 *    >>     if likely(result && DeeString_STR_ISUTF8(str))
 *    >>        result = DeeString_SetUtf8(result); // Interpret raw data as UTF-8
 *    >>     return result;
 *    >> }
 *       This method works with any kind of string, but has an O(n) slowdown in
 *       `DeeString_SetUtf8()' when the input string contains ASCII characters
 *       outside of the ASCII range (or in some cases: LATIN-1 range).
 */
#define DeeString_STR_ISUTF8(x)                                              \
	(((DeeStringObject *)(x))->s_data &&                                     \
	 ((((DeeStringObject *)(x))->s_data->u_flags & Dee_STRING_UTF_FASCII) || \
	  ((DeeStringObject *)(x))->s_data->u_width != Dee_STRING_WIDTH_1BYTE))

/* Returns true if `DeeString_STR()' is encoded in LATIN-1, or ASCII */
#define DeeString_STR_ISLATIN1(x)                                           \
	(!((DeeStringObject *)(x))->s_data ||                                   \
	 (((DeeStringObject *)(x))->s_data->u_flags & Dee_STRING_UTF_FASCII) || \
	 ((DeeStringObject *)(x))->s_data->u_width == Dee_STRING_WIDTH_1BYTE)


/* Check if the given string object `x' has had its hash calculated. */
#define DeeString_HASHOK(x) (((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_hash != (Dee_hash_t)-1)
#define DeeString_HASH(x)   ((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_hash

/* Check if the given string object `x' is an (the) empty string. */
#define DeeString_IsEmpty(x) (((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_len == 0)


#define DeeString_EQUALS_ASCII(x, ascii_str)            \
	(DeeString_SIZE(x) == COMPILER_STRLEN(ascii_str) && \
	 bcmp(DeeString_STR(x), ascii_str, sizeof(ascii_str) - sizeof(char)) == 0)
#define DeeString_STARTSWITH_ASCII(x, ascii_str)        \
	(DeeString_SIZE(x) >= COMPILER_STRLEN(ascii_str) && \
	 bcmp(DeeString_STR(x), ascii_str, sizeof(ascii_str) - sizeof(char)) == 0)
#define DeeString_ENDSWITH_ASCII(x, ascii_str)                               \
	(DeeString_SIZE(x) >= COMPILER_STRLEN(ascii_str) &&                      \
	 bcmp(DeeString_STR(x) + DeeString_SIZE(x) - COMPILER_STRLEN(ascii_str), \
	      ascii_str, sizeof(ascii_str) - sizeof(char)) == 0)

/* FIXME: These macros need to check for `DeeString_STR_ISUTF8()':
 * >> local a = string.fromseq({ 0x100 });
 * >> local b = string.fromseq({ 0xc4, 0x80 });
 * >> assert a.__str_bytes__ == b.__str_bytes__; // Same str-bytes, but different strings!
 */
#define DeeString_EqualsSTR(lhs, rhs_STR)              \
	(DeeString_SIZE(lhs) == DeeString_SIZE(rhs_STR) && \
	 bcmp(DeeString_STR(lhs), DeeString_STR(rhs_STR),  \
	      DeeString_SIZE(lhs) * sizeof(char)) == 0)
#define DeeString_EqualsCStr(lhs, rhs_CStr) \
	(bcmp(DeeString_STR(lhs), rhs_CStr, (DeeString_SIZE(lhs) + 1) * sizeof(char)) == 0)
#define DeeString_StartsWithSTR(lhs, rhs_STR)          \
	(DeeString_SIZE(lhs) >= DeeString_SIZE(rhs_STR) && \
	 bcmp(DeeString_STR(lhs), DeeString_STR(rhs_STR),  \
	      DeeString_SIZE(rhs_STR) * sizeof(char)) == 0)
#define DeeString_EndsWithSTR(lhs, rhs_STR)             \
	(DeeString_SIZE(lhs) >= DeeString_SIZE(rhs_STR) &&  \
	 bcmp(DeeString_END(lhs) - DeeString_SIZE(rhs_STR), \
	      DeeString_STR(rhs_STR), DeeString_SIZE(rhs_STR) * sizeof(char)) == 0)

/* FIXME: These macros don't account for `DeeString_STR_ISUTF8()' */
#define DeeString_EqualsBuf(lhs, rhs_base, rhs_size) \
	(DeeString_SIZE(lhs) == (rhs_size) &&            \
	 bcmp(DeeString_STR(lhs), rhs_base,              \
	      DeeString_SIZE(lhs) * sizeof(char)) == 0)
#define DeeString_StartsWithBuf(lhs, rhs_base, rhs_size) \
	(DeeString_SIZE(lhs) >= (rhs_size) &&                \
	 bcmp(DeeString_STR(lhs), rhs_base,                  \
	      (rhs_size) * sizeof(char)) == 0)
#define DeeString_EndsWithBuf(lhs, rhs_base, rhs_size) \
	(DeeString_SIZE(lhs) >= (rhs_size) &&              \
	 bcmp(DeeString_END(lhs) - (rhs_size),             \
	      rhs_base, (rhs_size) * sizeof(char)) == 0)


/* Return the unicode character-width of characters found in the given string `x' */
#define DeeString_WIDTH(x)                                          \
	(((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data            \
	 ? ((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data->u_width \
	 : Dee_STRING_WIDTH_1BYTE)

#define DeeString_Is1Byte(x) (!((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data || ((DeeStringObject *)(x))->s_data->u_width == Dee_STRING_WIDTH_1BYTE)
#define DeeString_Is2Byte(x) (((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data && ((DeeStringObject *)(x))->s_data->u_width == Dee_STRING_WIDTH_2BYTE)
#define DeeString_Is4Byte(x) (((DeeStringObject *)Dee_REQUIRES_OBJECT(x))->s_data && ((DeeStringObject *)(x))->s_data->u_width == Dee_STRING_WIDTH_4BYTE)

/* Return a pointer to the unicode character-array for which all
 * characters can be fitted into the same number of bytes, the
 * amount of which can be determined by `Dee_WSTR_LENGTH(DeeString_WSTR(x))',
 * or `DeeString_WLEN(x)', with their individual size in bytes determinable
 * as `Dee_STRING_SIZEOF_WIDTH(DeeString_WSTR(x))'.
 * HINT: The string length returned by `string.operator #' is `DeeString_WLEN()' */
#define DeeString_WSTR(x) _DeeString_WStr((DeeStringObject *)Dee_REQUIRES_OBJECT(x))
#define DeeString_WLEN(x) _DeeString_WLen((DeeStringObject *)Dee_REQUIRES_OBJECT(x))
#define DeeString_WEND(x) _DeeString_WEnd((DeeStringObject *)Dee_REQUIRES_OBJECT(x))
#define DeeString_WSIZ(x) _DeeString_WSiz((DeeStringObject *)Dee_REQUIRES_OBJECT(x))

LOCAL ATTR_PURE WUNUSED NONNULL((1)) size_t const *DCALL
_DeeString_WStr(DeeStringObject *__restrict self) {
	struct Dee_string_utf *utf;
	if ((utf = self->s_data) != NULL)
		return utf->u_data[utf->u_width];
	return (size_t const *)self->s_str;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) void const *DCALL
_DeeString_WEnd(DeeStringObject *__restrict self) {
	struct Dee_string_utf *utf;
	if ((utf = self->s_data) != NULL) {
		Dee_SWITCH_SIZEOF_WIDTH(utf->u_width) {
		Dee_CASE_WIDTH_1BYTE: {
			uint8_t const *result = (uint8_t const *)utf->u_data[Dee_STRING_WIDTH_1BYTE];
			return result + Dee_WSTR_LENGTH(result);
		}
		Dee_CASE_WIDTH_2BYTE: {
			uint16_t const *result = (uint16_t const *)utf->u_data[Dee_STRING_WIDTH_2BYTE];
			return result + Dee_WSTR_LENGTH(result);
		}
		Dee_CASE_WIDTH_4BYTE: {
			uint32_t const *result = (uint32_t const *)utf->u_data[Dee_STRING_WIDTH_4BYTE];
			return result + Dee_WSTR_LENGTH(result);
		}
		}
	}
	return self->s_str + self->s_len;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
_DeeString_WLen(DeeStringObject const *__restrict self) {
	struct Dee_string_utf const *utf;
	if ((utf = self->s_data) != NULL)
		return utf->u_data[utf->u_width][-1];
	return self->s_len;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
_DeeString_WSiz(DeeStringObject const *__restrict self) {
	struct Dee_string_utf const *utf;
	if ((utf = self->s_data) != NULL) {
		return Dee_STRING_MUL_SIZEOF_WIDTH(utf->u_data[utf->u_width][-1],
		                                   utf->u_width);
	}
	return self->s_len;
}


/* Return the hash of `self', or calculate it if it wasn't already. */
#ifdef CONFIG_BUILDING_DEEMON
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeString_Hash(DeeObject *__restrict self);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeString_Hash(self) DeeObject_Hash(self)
#endif /* !CONFIG_BUILDING_DEEMON */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeString_HashCase(DeeObject *__restrict self);

/* Delete cached buffer encodings from a given 1-byte string. */
PUBLIC NONNULL((1)) void DCALL DeeString_FreeWidth(DeeObject *__restrict self);

/* Return the given string's characters as a byte-array.
 * Characters above 0xFF either cause `NULL' to be returned, alongside
 * a ValueError being thrown, or cause them to be replaced with '?'.
 * @return: * :   The Bytes-data of the given string `self' (encoded as a width-string)
 *                NOTE: The length of this block also matches `DeeString_WLEN(self)'
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) /*latin-1*/ __BYTE_TYPE__ const *DCALL
DeeString_AsBytes(DeeObject *__restrict self, bool allow_invalid);
#define DeeString_AsLatin1(self, allow_invalid) DeeString_AsBytes(self, allow_invalid)

/* Quickly access the 1, 2 or 4-byte variants of a given string, allowing
 * for the assumption that all characters of the string are guarantied to
 * fit the requested amount of bytes. */
DFUNDEF WUNUSED NONNULL((1)) uint16_t const *(DCALL DeeString_As2Byte)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint32_t const *(DCALL DeeString_As4Byte)(DeeObject *__restrict self);
#ifdef __INTELLISENSE__
ATTR_RETNONNULL WUNUSED NONNULL((1)) uint8_t const *(DeeString_As1Byte)(DeeObject *__restrict self);
#endif /* __INTELLISENSE__ */

#ifdef __INTELLISENSE__
ATTR_RETNONNULL WUNUSED NONNULL((1)) uint8_t const *(DeeString_Get1Byte)(DeeObject *__restrict self);
ATTR_RETNONNULL WUNUSED NONNULL((1)) uint16_t const *(DeeString_Get2Byte)(DeeObject *__restrict self);
ATTR_RETNONNULL WUNUSED NONNULL((1)) uint32_t const *(DeeString_Get4Byte)(DeeObject *__restrict self);
#else /* __INTELLISENSE__ */
#define DeeString_Get1Byte(self) DeeString_As1Byte(self)
#define DeeString_Get2Byte(self)                                                  \
	(ASSERTF(((DeeStringObject *)(self))->s_data &&                               \
	         ((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_2BYTE], \
	         "The 2-byte variant hasn't been allocated"),                         \
	 (uint16_t const *)((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_2BYTE])
#define DeeString_Get4Byte(self)                                                  \
	(ASSERTF(((DeeStringObject *)(self))->s_data &&                               \
	         ((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_4BYTE], \
	         "The 4-byte variant hasn't been allocated"),                         \
	 (uint32_t const *)((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_4BYTE])
#define DeeString_As1Byte(self)                                                      \
	(ASSERTF(!((DeeStringObject *)(self))->s_data ||                                 \
	         ((DeeStringObject *)(self))->s_data->u_width == Dee_STRING_WIDTH_1BYTE, \
	         "The string is too wide to view its 1-byte variant"),                   \
	 (uint8_t const *)DeeString_STR(self))
#define DeeString_As2Byte(self)                                                              \
	(((DeeStringObject *)(self))->s_data &&                                                  \
	 (ASSERTF(((DeeStringObject *)(self))->s_data->u_width <= Dee_STRING_WIDTH_2BYTE,        \
	          "The string is too wide to view its 2-byte variant"),                          \
	  ((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_2BYTE])                   \
	 ? (uint16_t const *)((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_2BYTE] \
	 : (DeeString_As2Byte)(self))
#define DeeString_As4Byte(self)                                                              \
	(((DeeStringObject *)(self))->s_data &&                                                  \
	 ((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_4BYTE]                     \
	 ? (uint32_t const *)((DeeStringObject *)(self))->s_data->u_data[Dee_STRING_WIDTH_4BYTE] \
	 : (DeeString_As4Byte)(self))
#endif /* !__INTELLISENSE__ */


/* Return the UTF-8 variant of the given string.
 * The returned string can be used as a width-string, meaning that
 * upon success (return != NULL), you can determine its length by
 * using `Dee_WSTR_LENGTH(return)'.
 * @return: * :   A pointer to the UTF-8 variant-string of `self'
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) /*utf-8*/ char const *DCALL
DeeString_AsUtf8(DeeObject *__restrict self);

/* Same as `DeeString_AsUtf8()', but returns NULL without throwing an error. */
DFUNDEF WUNUSED NONNULL((1)) /*utf-8*/ char const *DCALL
DeeString_TryAsUtf8(DeeObject *__restrict self);

/* Returns the UTF-16 variant of the given string (as a width-string). */
DFUNDEF WUNUSED NONNULL((1)) uint16_t const *DCALL
DeeString_AsUtf16(DeeObject *__restrict self, unsigned int error_mode);

/* Returns the UTF-32 variant of the given string (as a width-string). */
#ifdef __INTELLISENSE__
uint32_t const *DeeString_AsUtf32(DeeObject *__restrict self);
#else /* __INTELLISENSE__ */
#define DeeString_AsUtf32(self) DeeString_As4Byte(self)
#endif /* !__INTELLISENSE__ */

/* Returns the Wide-string variant of the given string (as a width-string). */
#ifdef __INTELLISENSE__
Dee_wchar_t const *DeeString_AsWide(DeeObject *__restrict self);
#elif __SIZEOF_WCHAR_T__ == 2
#define DeeString_AsWide(self) ((Dee_wchar_t const *)DeeString_AsUtf16(self, Dee_STRING_ERROR_FREPLAC))
#else /* __SIZEOF_WCHAR_T__ == 2 */
#define DeeString_AsWide(self) ((Dee_wchar_t const *)DeeString_AsUtf32(self))
#endif /* __SIZEOF_WCHAR_T__ != 2 */




/* ================================================================================= */
/*   STRING BUFFER API                                                               */
/* ================================================================================= */

/* Construct an uninitialized single-byte string,
 * capable of representing up to `num_bytes' bytes of text (representing LATIN-1 text). */
#undef DeeString_NewBuffer
#undef DeeDbgString_NewBuffer
DFUNDEF WUNUSED DREF DeeObject *(DCALL DeeString_NewBuffer)(size_t num_bytes);
DFUNDEF WUNUSED DREF DeeObject *(DCALL DeeDbgString_NewBuffer)(size_t num_bytes, char const *file, int line);
#ifdef NDEBUG
#define DeeDbgString_NewBuffer(num_bytes, file, line) DeeString_NewBuffer(num_bytes)
#else /* !NDEBUG */
#define DeeString_NewBuffer(num_bytes) DeeDbgString_NewBuffer(num_bytes, __FILE__, __LINE__)
#endif /* !NDEBUG */
#define DeeString_GetBuffer(self) ((DeeStringObject *)Dee_REQUIRES_OBJECT(self))->s_str

/* Resize a single-byte string to have a length of `num_bytes' bytes.
 * You may pass `NULL' for `self', or a reference to `Dee_EmptyString'
 * in order to allocate and return a new buffer. */
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeString_ResizeBuffer(DREF DeeObject *self, size_t num_bytes);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeString_TryResizeBuffer(DREF DeeObject *self, size_t num_bytes);

#ifdef CONFIG_BUILDING_DEEMON
/* Print the text of `self' to `printer', encoded as a UTF-8 string.
 * NOTE: If `printer' is `&unicode_printer_print', special optimization
 *       is done, meaning that this is the preferred method of printing
 *       an object to a unicode printer.
 * NOTE: This optimization is also done when `DeeObject_Print' is used. */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeString_PrintUtf8(DeeObject *__restrict self,
                    Dee_formatprinter_t printer,
                    void *arg);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeString_PrintUtf8 DeeObject_Print
#endif /* !CONFIG_BUILDING_DEEMON */
#define DeeString_PrintAscii(self, printer, arg) \
	(*printer)(arg, DeeString_STR(self), DeeString_SIZE(self))

/* Print the escape-encoded variant of `self' */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeString_PrintRepr(DeeObject *__restrict self,
                    Dee_formatprinter_t printer,
                    void *arg);


/* Construct a string from the given escape-sequence.
 * This function automatically deals with escaped characters above
 * the single-byte range, and expects the input to be structured as
 * UTF-8 text.
 *  - Surrounding quotation marks should be stripped before calling this function.
 *  - Escaped linefeeds are implicitly parsed, too.
 * @param: error_mode: One of `STRING_ERROR_F*' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_FromBackslashEscaped(/*utf-8*/ char const *__restrict start,
                               size_t length, unsigned int error_mode);

/* @return: 0 : Success
 * @return: -1: Error */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeString_DecodeBackslashEscaped(struct Dee_unicode_printer *__restrict printer,
                                 /*utf-8*/ char const *__restrict start,
                                 size_t length, unsigned int error_mode);



/* Construct a new, non-decoded single-byte-per-character string `str'.
 * The string itself may contain characters above 127, which are then
 * interpreted as part of the unicode character-range U+0080...U+00FF. */
#undef DeeString_New
#undef DeeDbgString_New
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_New)(/*unsigned latin-1*/ char const *__restrict str);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeDbgString_New)(/*unsigned latin-1*/ char const *__restrict str, char const *file, int line);
#define DeeString_NewWithHash(str, hash)                ((void)(hash), DeeString_New(str))                /* XXX: Take advantage of this? */
#define DeeDbgString_NewWithHash(str, hash, file, line) ((void)(hash), DeeDbgString_New(str, file, line)) /* XXX: Take advantage of this? */
#ifdef NDEBUG
#define DeeDbgString_New(str, file, line) DeeString_New(str)
#else /* NDEBUG */
#define DeeString_New(str) DeeDbgString_New(str, __FILE__, __LINE__)
#endif /* !NDEBUG */


#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

/* Check if `str' is the `DeeString_STR()' of a string object, and return
 * a pointer to that object, only if it being that can be guarantied.
 * Otherwise return `NULL'.
 * WARNING: In order to prevent race conditions, only use this function
 *          with statically allocated strings, as a heap implementation
 *          that doesn't fill free memory and uses a block-size field in
 *          an unfortunate location could wrongfully trigger a match. */
#if defined(__ARCH_PAGESIZE_MIN) && !defined(__OPTIMIZE_SIZE__)
LOCAL WUNUSED NONNULL((1)) DeeObject *DCALL
DeeString_IsObject(/*unsigned*/ char const *__restrict str) {
	DeeStringObject *base;
	base = COMPILER_CONTAINER_OF(str, DeeStringObject, s_str);
	/* Check if the string object base would be part of the same page
	 * as the string pointer we were given. - Only if it is, can we
	 * safely access the supposed string object to check if it matches
	 * what we'd expect of a string instance. */
	if (((uintptr_t)base & ~(__ARCH_PAGESIZE_MIN - 1)) == ((uintptr_t)str & ~(__ARCH_PAGESIZE_MIN - 1)) &&
	    ((uintptr_t)base & (sizeof(void *) - 1)) == 0) {
		/* Most important check: Does the object type indicate that it's a string? */
		if (base->ob_type == &DeeString_Type) {
			/* Check that the object's reference counter is non-zero.
			 * The combination of these 2 checks can verify a string object
			 * allowed by any kind of allocator, regardless of what may be
			 * done to freed objects:
			 * #1: If the heap does some debug stuff to memset() free memory,
			 *     and the given `str' is allocated within such a region, ob_type
			 *     would not be a pointer to `DeeString_Type'
			 * #2: If the heap doesn't do debug-memset()-stuff, and we were unlucky
			 *     enough to be given an `str' allocated at the exact location where
			 *     then `ob_refcnt' would still be zero.
			 * #3: UNLUCKY: Same as in #2, but `ob_refcnt' is the heap-size field,
			 *              in which case we'd get an invalid match by assuming
			 *              that the heap-size field was our reference counter.
			 * -> Because of case #3, this function can't be used for heap-allocated strings. */
			if (base->ob_refcnt != 0)
				return (DeeObject *)base;
		}
	}
	return NULL;
}

#ifdef NDEBUG
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewAuto(/*unsigned*/ char const *__restrict str)
#define DeeDbgString_NewAuto(str, file, line) DeeString_NewAuto(str)
#else /* NDEBUG */
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDbgString_NewAuto(/*unsigned*/ char const *__restrict str, char const *file, int line)
#define DeeString_NewAuto(str) DeeDbgString_NewAuto(str, __FILE__, __LINE__)
#endif /* !NDEBUG */
{
	DeeObject *result;
	result = DeeString_IsObject(str);
	if (result) {
#ifndef __INTELLISENSE__
		Dee_Incref(result);
#endif /* !__INTELLISENSE__ */
	} else {
#ifdef NDEBUG
		result = DeeString_New(str);
#else /* NDEBUG */
		result = DeeDbgString_New(str, file, line);
#endif /* !NDEBUG */
	}
	return result;
}

#ifdef NDEBUG
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewAutoWithHash(/*unsigned*/ char const *__restrict str, Dee_hash_t hash)
#define DeeDbgString_NewAutoWithHash(str, hash, file, line) DeeString_NewAutoWithHash(str, hash)
#else /* NDEBUG */
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDbgString_NewAutoWithHash(/*unsigned*/ char const *__restrict str,
                             Dee_hash_t hash, char const *file, int line)
#define DeeString_NewAutoWithHash(str, hash) DeeDbgString_NewAutoWithHash(str, hash, __FILE__, __LINE__)
#endif /* !NDEBUG */
{
	DeeObject *result;
	result = DeeString_IsObject(str);
	if (result) {
		Dee_hash_t hashof_str = ((DeeStringObject const *)result)->s_hash;
		if unlikely(hashof_str != hash) {
			if unlikely(hashof_str != DEE_STRING_HASH_UNSET)
				goto return_new_string;
			((DeeStringObject *)result)->s_hash = hash;
		}
#ifndef __INTELLISENSE__
		Dee_Incref(result);
#endif /* !__INTELLISENSE__ */
	} else {
return_new_string:
#ifdef NDEBUG
		result = DeeString_NewWithHash(str, hash);
#else /* NDEBUG */
		result = DeeDbgString_NewWithHash(str, hash, file, line);
#endif /* !NDEBUG */
	}
	return result;
}

#else /* __ARCH_PAGESIZE_MIN && !__OPTIMIZE_SIZE__ */
#define DeeString_IsObject_IS_NOOP
#define DeeString_IsObject(str)                             ((DeeObject *)NULL)
#define DeeString_NewAuto(str)                              DeeString_New(str)
#define DeeString_NewAutoWithHash(str, hash)                DeeString_NewWithHash(str, hash)
#define DeeDbgString_NewAuto(str, file, line)               DeeDbgString_New(str, file, line)
#define DeeDbgString_NewAutoWithHash(str, hash, file, line) DeeDbgString_NewWithHash(str, hash, file, line)
#endif /* !__ARCH_PAGESIZE_MIN || __OPTIMIZE_SIZE__ */

/* Construct a new string using printf-like (and deemon-enhanced) format-arguments. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DeeString_Newf(/*utf-8*/ char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_VNewf(/*utf-8*/ char const *__restrict format, va_list args);

/* Construct strings with basic width-data. */
#undef DeeString_NewSized
#undef DeeString_New2Byte
#undef DeeString_New4Byte
#undef DeeDbgString_NewSized
#undef DeeDbgString_New2Byte
#undef DeeDbgString_New4Byte
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_NewSized)(/*unsigned latin-1*/ char const *__restrict str, size_t length);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_New2Byte)(uint16_t const *__restrict str, size_t length);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_New4Byte)(uint32_t const *__restrict str, size_t length);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeDbgString_NewSized)(/*unsigned latin-1*/ char const *__restrict str, size_t length, char const *file, int line);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeDbgString_New2Byte)(uint16_t const *__restrict str, size_t length, char const *file, int line);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeDbgString_New4Byte)(uint32_t const *__restrict str, size_t length, char const *file, int line);
#define DeeString_NewSizedWithHash(str, length, hash) \
	((void)(hash), DeeString_NewSized(str, length)) /* XXX: Take advantage of this? */
#define DeeDbgString_NewSizedWithHash(str, length, hash, file, line) \
	((void)(hash), DeeDbgString_NewSized(str, length, file, line)) /* XXX: Take advantage of this? */
#ifdef NDEBUG
#define DeeDbgString_NewSized(str, length, file, line) DeeString_NewSized(str, length)
#define DeeDbgString_New2Byte(str, length, file, line) DeeString_New2Byte(str, length)
#define DeeDbgString_New4Byte(str, length, file, line) DeeString_New4Byte(str, length)
#else /* !NDEBUG */
#define DeeString_NewSized(str, length) DeeDbgString_NewSized(str, length, __FILE__, __LINE__)
#define DeeString_New2Byte(str, length) DeeDbgString_New2Byte(str, length, __FILE__, __LINE__)
#define DeeString_New4Byte(str, length) DeeDbgString_New4Byte(str, length, __FILE__, __LINE__)
#endif /* !NDEBUG */


#ifdef __INTELLISENSE__
DREF DeeObject *DeeString_New1Byte(uint8_t const *__restrict str, size_t length);
DREF DeeObject *DeeDbgString_New1Byte(uint8_t const *__restrict str, size_t length, char const *file, int line);
#else /* __INTELLISENSE__ */
#define DeeString_New1Byte(str, length)                DeeString_NewSized((char const *)(str), length)
#define DeeDbgString_New1Byte(str, length, file, line) DeeDbgString_NewSized((char const *)(str), length, file, line)
#endif /* !__INTELLISENSE__ */


/* Construct a string from a UTF-8 character sequence. */
#undef DeeString_NewUtf8
#undef DeeDbgString_NewUtf8
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_NewUtf8)(/*utf-8*/ char const *__restrict str, size_t length, unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeDbgString_NewUtf8)(/*utf-8*/ char const *__restrict str, size_t length, unsigned int error_mode, char const *file, int line);
#ifdef NDEBUG
#define DeeDbgString_NewUtf8(str, length, error_mode, file, line) DeeString_NewUtf8(str, length, error_mode)
#else /* NDEBUG */
#define DeeString_NewUtf8(str, length, error_mode) DeeDbgString_NewUtf8(str, length, error_mode, __FILE__, __LINE__)
#endif /* !NDEBUG */

/* Given a string `self' that has previously been allocated as a byte-buffer
 * string (such as `DeeString_NewSized()' or `DeeString_NewBuffer()'), convert
 * it into a UTF-8 string, using the byte-buffer data as UTF-8 text.
 *
 * This function _always_ inherits a reference to `self', and will return
 * `NULL' on error. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_SetUtf8(/*inherit(always)*/ DREF DeeObject *__restrict self,
                  unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_TrySetUtf8(/*inherit(on_success)*/ DREF DeeObject *__restrict self);

#ifndef DeeString_IsObject_IS_NOOP
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewAutoUtf8(/*unsigned*/ char const *__restrict str) {
	DeeObject *result;
	result = DeeString_IsObject(str);
	if (result) {
#ifndef __INTELLISENSE__
		Dee_Incref(result);
#endif /* !__INTELLISENSE__ */
	} else {
		result = DeeString_NewUtf8(str, strlen(str), Dee_STRING_ERROR_FIGNORE);
	}
	return result;
}
#else /* !DeeString_IsObject_IS_NOOP */
#define DeeString_NewAutoUtf8(str) \
	DeeString_NewUtf8(str, strlen(str), Dee_STRING_ERROR_FIGNORE)
#endif /* DeeString_IsObject_IS_NOOP */


/* Construct strings from UTF-16/32 encoded content.
 * @param: error_mode: One of `STRING_ERROR_F*' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf16(uint16_t const *__restrict str,
                   size_t length, unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf32(uint32_t const *__restrict str,
                   size_t length, unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf16AltEndian(uint16_t const *__restrict str,
                            size_t length, unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf32AltEndian(uint32_t const *__restrict str,
                            size_t length, unsigned int error_mode);

#ifdef __INTELLISENSE__
WUNUSED NONNULL((1)) DREF DeeObject *
DeeString_NewWide(Dee_wchar_t const *__restrict str,
                  size_t length, unsigned int error_mode);
WUNUSED NONNULL((1)) DREF DeeObject *
DeeString_NewWideAltEndian(Dee_wchar_t const *__restrict str,
                           size_t length, unsigned int error_mode);
#elif __SIZEOF_WCHAR_T__ == 2
#define DeeString_NewWide(str, length, error_mode) \
	DeeString_NewUtf16((uint16_t *)(str), length, error_mode)
#define DeeString_NewWideAltEndian(str, length, error_mode) \
	DeeString_NewUtf16AltEndian((uint16_t *)(str), length, error_mode)
#elif __SIZEOF_WCHAR_T__ == 4
#define DeeString_NewWide(str, length, error_mode) \
	DeeString_NewUtf32((uint32_t *)(str), length, error_mode)
#define DeeString_NewWideAltEndian(str, length, error_mode) \
	DeeString_NewUtf32AltEndian((uint32_t *)(str), length, error_mode)
#endif /* __SIZEOF_WCHAR_T__ == ... */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DeeString_NewUtf16Le DeeString_NewUtf16
#define DeeString_NewUtf32Le DeeString_NewUtf32
#define DeeString_NewUtf16Be DeeString_NewUtf16AltEndian
#define DeeString_NewUtf32Be DeeString_NewUtf32AltEndian
#define DeeString_NewWideLe  DeeString_NewWide
#define DeeString_NewWideBe  DeeString_NewWideAltEndian
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define DeeString_NewUtf16Le DeeString_NewUtf16AltEndian
#define DeeString_NewUtf32Le DeeString_NewUtf32AltEndian
#define DeeString_NewUtf16Be DeeString_NewUtf16
#define DeeString_NewUtf32Be DeeString_NewUtf32
#define DeeString_NewWideLe  DeeString_NewWideAltEndian
#define DeeString_NewWideBe  DeeString_NewWide
#endif /* __BYTE_ORDER__ == ... */



/* API for string construction from buffers. */
#if 0
#define DeeString_SizeOf2ByteBuffer(num_chars)        (sizeof(size_t) + ((num_chars) + 1) * 2)
#define DeeString_SizeOf4ByteBuffer(num_chars)        (sizeof(size_t) + ((num_chars) + 1) * 4)
#define DeeString_SizeOfWidthBuffer(num_chars, width) (sizeof(size_t) + (num_chars + 1) * Dee_STRING_SIZEOF_WIDTH(width))
#else
LOCAL ATTR_CONST WUNUSED size_t (DCALL DeeString_SizeOf2ByteBuffer)(size_t num_chars) {
	size_t result;
	if unlikely(__hybrid_overflow_umul(num_chars, 2, &result))
		goto overflow;
	if unlikely(__hybrid_overflow_uadd(result, sizeof(size_t) + 2, &result))
		goto overflow;
	return result;
overflow: /* Force downstream failure */
	return (size_t)-1;
}
LOCAL ATTR_CONST WUNUSED size_t (DCALL DeeString_SizeOf4ByteBuffer)(size_t num_chars) {
	size_t result;
	if unlikely(__hybrid_overflow_umul(num_chars, 4, &result))
		goto overflow;
	if unlikely(__hybrid_overflow_uadd(result, sizeof(size_t) + 4, &result))
		goto overflow;
	return result;
overflow: /* Force downstream failure */
	return (size_t)-1;
}
LOCAL ATTR_CONST WUNUSED size_t (DCALL DeeString_SizeOfWidthBuffer)(size_t num_chars, unsigned int width) {
	size_t result, chwidth = Dee_STRING_SIZEOF_WIDTH(width);
	if unlikely(__hybrid_overflow_umul(num_chars, chwidth, &result)) /* XXX: overflow_ushl() ??? */
		goto overflow;
	if unlikely(__hybrid_overflow_uadd(result, sizeof(size_t) + chwidth, &result))
		goto overflow;
	return result;
overflow: /* Force downstream failure */
	return (size_t)-1;
}
#endif

#ifdef __INTELLISENSE__
/* Allocate a new buffer for constructing a string form either
 * wide-character encoding, or any 1-, 2- or 4-byte encoding.
 * (aka. LATIN-1 or UTF-8 / raw 2-byte or utf-16 / raw 4-byte or utf-32).  */
ATTR_MALLOC WUNUSED Dee_wchar_t *(DCALL DeeString_NewWideBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED uint8_t  *(DCALL DeeString_New1ByteBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED uint16_t *(DCALL DeeString_New2ByteBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED uint32_t *(DCALL DeeString_New4ByteBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED Dee_wchar_t *(DCALL DeeString_TryNewWideBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED uint8_t  *(DCALL DeeString_TryNew1ByteBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED uint16_t *(DCALL DeeString_TryNew2ByteBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED uint32_t *(DCALL DeeString_TryNew4ByteBuffer)(size_t num_chars);
ATTR_MALLOC WUNUSED Dee_wchar_t *(DCALL DeeDbgString_NewWideBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED uint8_t  *(DCALL DeeDbgString_New1ByteBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED uint16_t *(DCALL DeeDbgString_New2ByteBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED uint32_t *(DCALL DeeDbgString_New4ByteBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED Dee_wchar_t *(DCALL DeeDbgString_TryNewWideBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED uint8_t  *(DCALL DeeDbgString_TryNew1ByteBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED uint16_t *(DCALL DeeDbgString_TryNew2ByteBuffer)(size_t num_chars, char const *file, int line);
ATTR_MALLOC WUNUSED uint32_t *(DCALL DeeDbgString_TryNew4ByteBuffer)(size_t num_chars, char const *file, int line);

/* Resize a string buffer. */
WUNUSED Dee_wchar_t *(DCALL DeeString_ResizeWideBuffer)(Dee_wchar_t *buffer, size_t num_chars);
WUNUSED uint8_t  *(DCALL DeeString_Resize1ByteBuffer)(uint8_t *buffer, size_t num_chars);
WUNUSED uint16_t *(DCALL DeeString_Resize2ByteBuffer)(uint16_t *buffer, size_t num_chars);
WUNUSED uint32_t *(DCALL DeeString_Resize4ByteBuffer)(uint32_t *buffer, size_t num_chars);
WUNUSED Dee_wchar_t *(DCALL DeeString_TryResizeWideBuffer)(Dee_wchar_t *buffer, size_t num_chars);
WUNUSED uint8_t  *(DCALL DeeString_TryResize1ByteBuffer)(uint8_t *buffer, size_t num_chars);
WUNUSED uint16_t *(DCALL DeeString_TryResize2ByteBuffer)(uint16_t *buffer, size_t num_chars);
WUNUSED uint32_t *(DCALL DeeString_TryResize4ByteBuffer)(uint32_t *buffer, size_t num_chars);
WUNUSED Dee_wchar_t *(DCALL DeeDbgString_ResizeWideBuffer)(Dee_wchar_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED uint8_t  *(DCALL DeeDbgString_Resize1ByteBuffer)(uint8_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED uint16_t *(DCALL DeeDbgString_Resize2ByteBuffer)(uint16_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED uint32_t *(DCALL DeeDbgString_Resize4ByteBuffer)(uint32_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED Dee_wchar_t *(DCALL DeeDbgString_TryResizeWideBuffer)(Dee_wchar_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED uint8_t  *(DCALL DeeDbgString_TryResize1ByteBuffer)(uint8_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED uint16_t *(DCALL DeeDbgString_TryResize2ByteBuffer)(uint16_t *buffer, size_t num_chars, char const *file, int line);
WUNUSED uint32_t *(DCALL DeeDbgString_TryResize4ByteBuffer)(uint32_t *buffer, size_t num_chars, char const *file, int line);

/* Truncate a string buffer. */
WUNUSED ATTR_RETNONNULL NONNULL((1)) Dee_wchar_t *(DCALL DeeString_TruncateWideBuffer)(Dee_wchar_t *__restrict buffer, size_t num_chars);
WUNUSED ATTR_RETNONNULL NONNULL((1)) uint8_t  *(DCALL DeeString_Truncate1ByteBuffer)(uint8_t *__restrict buffer, size_t num_chars);
WUNUSED ATTR_RETNONNULL NONNULL((1)) uint16_t *(DCALL DeeString_Truncate2ByteBuffer)(uint16_t *__restrict buffer, size_t num_chars);
WUNUSED ATTR_RETNONNULL NONNULL((1)) uint32_t *(DCALL DeeString_Truncate4ByteBuffer)(uint32_t *__restrict buffer, size_t num_chars);
WUNUSED ATTR_RETNONNULL NONNULL((1)) Dee_wchar_t *(DCALL DeeDbgString_TruncateWideBuffer)(Dee_wchar_t *__restrict buffer, size_t num_chars, char const *file, int line);
WUNUSED ATTR_RETNONNULL NONNULL((1)) uint8_t  *(DCALL DeeDbgString_Truncate1ByteBuffer)(uint8_t *__restrict buffer, size_t num_chars, char const *file, int line);
WUNUSED ATTR_RETNONNULL NONNULL((1)) uint16_t *(DCALL DeeDbgString_Truncate2ByteBuffer)(uint16_t *__restrict buffer, size_t num_chars, char const *file, int line);
WUNUSED ATTR_RETNONNULL NONNULL((1)) uint32_t *(DCALL DeeDbgString_Truncate4ByteBuffer)(uint32_t *__restrict buffer, size_t num_chars, char const *file, int line);

/* Free a string buffer. */
void (DCALL DeeString_FreeWideBuffer)(Dee_wchar_t *buffer);
void (DCALL DeeString_Free1ByteBuffer)(uint8_t *buffer);
void (DCALL DeeString_Free2ByteBuffer)(uint16_t *buffer);
void (DCALL DeeString_Free4ByteBuffer)(uint32_t *buffer);
void (DCALL DeeDbgString_FreeWideBuffer)(Dee_wchar_t *buffer, char const *file, int line);
void (DCALL DeeDbgString_Free1ByteBuffer)(uint8_t *buffer, char const *file, int line);
void (DCALL DeeDbgString_Free2ByteBuffer)(uint16_t *buffer, char const *file, int line);
void (DCALL DeeDbgString_Free4ByteBuffer)(uint32_t *buffer, char const *file, int line);

/* Package a string buffer. */
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_Pack1ByteBuffer)(/*inherit(always)*/ uint8_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_Pack2ByteBuffer)(/*inherit(always)*/ uint16_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_Pack4ByteBuffer)(/*inherit(always)*/ uint32_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_PackWideBuffer)(/*inherit(always)*/ Dee_wchar_t *__restrict buffer, unsigned int error_mode);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_PackUtf8Buffer)(/*inherit(always)*/ uint8_t *__restrict buffer, unsigned int error_mode);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_PackUtf16Buffer)(/*inherit(always)*/ uint16_t *__restrict buffer, unsigned int error_mode);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_PackUtf32Buffer)(/*inherit(always)*/ uint32_t *__restrict buffer, unsigned int error_mode);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPack1ByteBuffer)(/*inherit(on_success)*/ uint8_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPack2ByteBuffer)(/*inherit(on_success)*/ uint16_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPack4ByteBuffer)(/*inherit(on_success)*/ uint32_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPackWideBuffer)(/*inherit(on_success)*/ Dee_wchar_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPackUtf8Buffer)(/*inherit(on_success)*/ uint8_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPackUtf16Buffer)(/*inherit(on_success)*/ uint16_t *__restrict buffer);
WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPackUtf32Buffer)(/*inherit(on_success)*/ uint32_t *__restrict buffer);
#else /* __INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_Pack2ByteBuffer(/*inherit(always)*/ uint16_t *__restrict buffer);
#define DeeString_Pack4ByteBuffer(buffer) DeeString_PackUtf32Buffer(buffer, Dee_STRING_ERROR_FIGNORE)
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_TryPack2ByteBuffer(/*inherit(on_success)*/ uint16_t *__restrict buffer);
#define DeeString_TryPack4ByteBuffer(buffer) DeeString_TryPackUtf32Buffer(buffer)
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_PackUtf16Buffer(/*inherit(always)*/ uint16_t *__restrict buffer, unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_TryPackUtf16Buffer(/*inherit(on_success)*/ uint16_t *__restrict buffer);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_PackUtf32Buffer(/*inherit(always)*/ uint32_t *__restrict buffer, unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeString_TryPackUtf32Buffer(/*inherit(on_success)*/ uint32_t *__restrict buffer);
#if __SIZEOF_WCHAR_T__ == 2
#define DeeString_NewWideBuffer(num_chars)                              ((Dee_wchar_t *)DeeString_New2ByteBuffer(num_chars))
#define DeeString_TryNewWideBuffer(num_chars)                           ((Dee_wchar_t *)DeeString_TryNew2ByteBuffer(num_chars))
#define DeeString_ResizeWideBuffer(buffer, num_chars)                   ((Dee_wchar_t *)DeeString_Resize2ByteBuffer((uint16_t *)(buffer), num_chars))
#define DeeString_TryResizeWideBuffer(buffer, num_chars)                ((Dee_wchar_t *)DeeString_TryResize2ByteBuffer((uint16_t *)(buffer), num_chars))
#define DeeString_TruncateWideBuffer(buffer, num_chars)                 ((Dee_wchar_t *)DeeString_Truncate2ByteBuffer((uint16_t *)(buffer), num_chars))
#define DeeString_PackWideBuffer(buf, error_mode)                       DeeString_PackUtf16Buffer((uint16_t *)(buf), error_mode)
#define DeeString_TryPackWideBuffer(buf)                                DeeString_TryPackUtf16Buffer((uint16_t *)(buf))
#define DeeString_FreeWideBuffer(buffer)                                DeeString_Free2ByteBuffer((uint16_t *)(buffer))
#define DeeDbgString_NewWideBuffer(num_chars, file, line)               ((Dee_wchar_t *)DeeDbgString_New2ByteBuffer(num_chars, file, line))
#define DeeDbgString_TryNewWideBuffer(num_chars, file, line)            ((Dee_wchar_t *)DeeDbgString_TryNew2ByteBuffer(num_chars, file, line))
#define DeeDbgString_ResizeWideBuffer(buffer, num_chars, file, line)    ((Dee_wchar_t *)DeeDbgString_Resize2ByteBuffer((uint16_t *)(buffer), num_chars, file, line))
#define DeeDbgString_TryResizeWideBuffer(buffer, num_chars, file, line) ((Dee_wchar_t *)DeeDbgString_TryResize2ByteBuffer((uint16_t *)(buffer), num_chars, file, line))
#define DeeDbgString_TruncateWideBuffer(buffer, num_chars, file, line)  ((Dee_wchar_t *)DeeDbgString_Truncate2ByteBuffer((uint16_t *)(buffer), num_chars, file, line))
#define DeeDbgString_FreeWideBuffer(buffer, file, line)                 DeeDbgString_Free2ByteBuffer((uint16_t *)(buffer), file, line)
#elif __SIZEOF_WCHAR_T__ == 4
#define DeeString_NewWideBuffer(num_chars)                              ((Dee_wchar_t *)DeeString_New4ByteBuffer(num_chars))
#define DeeString_TryNewWideBuffer(num_chars)                           ((Dee_wchar_t *)DeeString_TryNew4ByteBuffer(num_chars))
#define DeeString_ResizeWideBuffer(buffer, num_chars)                   ((Dee_wchar_t *)DeeString_Resize4ByteBuffer((uint32_t *)(buffer), num_chars))
#define DeeString_TryResizeWideBuffer(buffer, num_chars)                ((Dee_wchar_t *)DeeString_TryResize4ByteBuffer((uint32_t *)(buffer), num_chars))
#define DeeString_TruncateWideBuffer(buffer, num_chars)                 ((Dee_wchar_t *)DeeString_Truncate4ByteBuffer((uint32_t *)(buffer), num_chars))
#define DeeString_PackWideBuffer(buf, error_mode)                       DeeString_PackUtf32Buffer((uint32_t *)(buf), error_mode)
#define DeeString_TryPackWideBuffer(buf)                                DeeString_TryPackUtf32Buffer((uint32_t *)(buf))
#define DeeString_FreeWideBuffer(buffer)                                DeeString_Free4ByteBuffer((uint32_t *)(buffer))
#define DeeDbgString_NewWideBuffer(num_chars, file, line)               ((Dee_wchar_t *)DeeDbgString_New4ByteBuffer(num_chars, file, line))
#define DeeDbgString_TryNewWideBuffer(num_chars, file, line)            ((Dee_wchar_t *)DeeDbgString_TryNew4ByteBuffer(num_chars, file, line))
#define DeeDbgString_ResizeWideBuffer(buffer, num_chars, file, line)    ((Dee_wchar_t *)DeeDbgString_Resize4ByteBuffer((uint32_t *)(buffer), num_chars, file, line))
#define DeeDbgString_TryResizeWideBuffer(buffer, num_chars, file, line) ((Dee_wchar_t *)DeeDbgString_TryResize4ByteBuffer((uint32_t *)(buffer), num_chars, file, line))
#define DeeDbgString_TruncateWideBuffer(buffer, num_chars, file, line)  ((Dee_wchar_t *)DeeDbgString_Truncate4ByteBuffer((uint32_t *)(buffer), num_chars, file, line))
#define DeeDbgString_FreeWideBuffer(buffer, file, line)                 DeeDbgString_Free4ByteBuffer((uint32_t *)(buffer), file, line)
#else /* __SIZEOF_WCHAR_T__ == ... */
#error "Unsupported sizeof(wchar_t) (must be either 2 or 4)"
#endif /* __SIZEOF_WCHAR_T__ != ... */
#ifdef NDEBUG
#define DeeDbgString_New1ByteBuffer(num_chars, file, line) \
	DeeString_New1ByteBuffer(num_chars)
#define DeeDbgString_New2ByteBuffer(num_chars, file, line) \
	DeeString_New2ByteBuffer(num_chars)
#define DeeDbgString_New4ByteBuffer(num_chars, file, line) \
	DeeString_New4ByteBuffer(num_chars)
#define DeeDbgString_TryNew1ByteBuffer(num_chars, file, line) \
	DeeString_TryNew1ByteBuffer(num_chars)
#define DeeDbgString_TryNew2ByteBuffer(num_chars, file, line) \
	DeeString_TryNew2ByteBuffer(num_chars)
#define DeeDbgString_TryNew4ByteBuffer(num_chars, file, line) \
	DeeString_TryNew4ByteBuffer(num_chars)
#define DeeDbgString_Resize1ByteBuffer(buffer, num_chars, file, line) \
	DeeString_Resize1ByteBuffer(buffer, num_chars)
#define DeeDbgString_Resize2ByteBuffer(buffer, num_chars, file, line) \
	DeeString_Resize2ByteBuffer(buffer, num_chars)
#define DeeDbgString_Resize4ByteBuffer(buffer, num_chars, file, line) \
	DeeString_Resize4ByteBuffer(buffer, num_chars)
#define DeeDbgString_TryResize1ByteBuffer(buffer, num_chars, file, line) \
	DeeString_TryResize1ByteBuffer(buffer, num_chars)
#define DeeDbgString_TryResize2ByteBuffer(buffer, num_chars, file, line) \
	DeeString_TryResize2ByteBuffer(buffer, num_chars)
#define DeeDbgString_TryResize4ByteBuffer(buffer, num_chars, file, line) \
	DeeString_TryResize4ByteBuffer(buffer, num_chars)
#define DeeDbgString_Truncate1ByteBuffer(buffer, num_chars, file, line) \
	DeeString_Truncate1ByteBuffer(buffer, num_chars)
#define DeeDbgString_Truncate2ByteBuffer(buffer, num_chars, file, line) \
	DeeString_Truncate2ByteBuffer(buffer, num_chars)
#define DeeDbgString_Truncate4ByteBuffer(buffer, num_chars, file, line) \
	DeeString_Truncate4ByteBuffer(buffer, num_chars)
#define DeeDbgString_Free1ByteBuffer(buffer, file, line) \
	DeeString_Free1ByteBuffer(buffer)
#define DeeDbgString_Free2ByteBuffer(buffer, file, line) \
	_DeeString_FreeBuffer(buffer)
#define DeeDbgString_Free4ByteBuffer(buffer, file, line) \
	_DeeString_FreeBuffer(buffer)
#define DeeString_Free2ByteBuffer(buffer) \
	_DeeString_FreeBuffer(buffer)
#define DeeString_Free4ByteBuffer(buffer) \
	_DeeString_FreeBuffer(buffer)
LOCAL ATTR_MALLOC WUNUSED uint16_t *
(DCALL DeeString_New2ByteBuffer)(size_t num_chars) {
	size_t *result = (size_t *)Dee_Malloc(DeeString_SizeOf2ByteBuffer(num_chars));
	if likely(result)
		*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL ATTR_MALLOC WUNUSED uint32_t *
(DCALL DeeString_New4ByteBuffer)(size_t num_chars) {
	size_t *result = (size_t *)Dee_Malloc(DeeString_SizeOf4ByteBuffer(num_chars));
	if likely(result)
		*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL ATTR_MALLOC WUNUSED uint16_t *
(DCALL DeeString_TryNew2ByteBuffer)(size_t num_chars) {
	size_t *result = (size_t *)Dee_TryMalloc(DeeString_SizeOf2ByteBuffer(num_chars));
	if likely(result)
		*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL ATTR_MALLOC WUNUSED uint32_t *
(DCALL DeeString_TryNew4ByteBuffer)(size_t num_chars) {
	size_t *result = (size_t *)Dee_TryMalloc(DeeString_SizeOf4ByteBuffer(num_chars));
	if likely(result)
		*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL WUNUSED uint16_t *
(DCALL DeeString_Resize2ByteBuffer)(uint16_t *buffer, size_t num_chars) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)Dee_Realloc(result, DeeString_SizeOf2ByteBuffer(num_chars));
	if likely(result)
		*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL WUNUSED uint32_t *
(DCALL DeeString_Resize4ByteBuffer)(uint32_t *buffer, size_t num_chars) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)Dee_Realloc(result, DeeString_SizeOf4ByteBuffer(num_chars));
	if likely(result)
		*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL WUNUSED uint16_t *
(DCALL DeeString_TryResize2ByteBuffer)(uint16_t *buffer, size_t num_chars) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)Dee_TryRealloc(result, DeeString_SizeOf2ByteBuffer(num_chars));
	if likely(result) {
		*result++ = num_chars;
	} else if (buffer && num_chars < Dee_WSTR_LENGTH(buffer)) {
		Dee_WSTR_LENGTH(buffer) = num_chars;
		result = (size_t *)buffer;
	}
	return (uint16_t *)result;
}

LOCAL WUNUSED uint32_t *
(DCALL DeeString_TryResize4ByteBuffer)(uint32_t *buffer, size_t num_chars) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)Dee_TryRealloc(result, sizeof(size_t) + (num_chars + 1) * 4);
	if likely(result) {
		*result++ = num_chars;
	} else if (buffer && num_chars < Dee_WSTR_LENGTH(buffer)) {
		Dee_WSTR_LENGTH(buffer) = num_chars;
		result = (size_t *)buffer;
	}
	return (uint32_t *)result;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) uint16_t *
(DCALL DeeString_Truncate2ByteBuffer)(uint16_t *__restrict buffer, size_t num_chars) {
	size_t *result;
	Dee_ASSERT(*((size_t *)buffer - 1) >= num_chars);
	result = (size_t *)Dee_TryRealloc((size_t *)buffer - 1,
	                                  DeeString_SizeOf2ByteBuffer(num_chars));
	if unlikely(!result)
		result = (size_t *)buffer - 1;
	*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) uint32_t *
(DCALL DeeString_Truncate4ByteBuffer)(uint32_t *__restrict buffer, size_t num_chars) {
	size_t *result;
	result = (size_t *)Dee_TryRealloc((size_t *)buffer - 1,
	                                  DeeString_SizeOf4ByteBuffer(num_chars));
	if unlikely(!result)
		result = (size_t *)buffer - 1;
	*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL void (DCALL _DeeString_FreeBuffer)(void *buffer) {
	if (buffer)
		Dee_Free((size_t *)buffer - 1);
}

LOCAL ATTR_MALLOC WUNUSED uint8_t *
(DCALL DeeString_New1ByteBuffer)(size_t num_chars) {
	DeeStringObject *result;
	result = (DeeStringObject *)DeeObject_Mallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                              num_chars + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL ATTR_MALLOC WUNUSED uint8_t *
(DCALL DeeString_TryNew1ByteBuffer)(size_t num_chars) {
	DeeStringObject *result;
	result = (DeeStringObject *)DeeObject_TryMallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                 num_chars + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL WUNUSED uint8_t *
(DCALL DeeString_Resize1ByteBuffer)(uint8_t *buffer, size_t num_chars) {
	DeeStringObject *result;
	result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
	result = (DeeStringObject *)DeeObject_Reallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                               num_chars + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL WUNUSED uint8_t *
(DCALL DeeString_TryResize1ByteBuffer)(uint8_t *buffer, size_t num_chars) {
	DeeStringObject *result;
	result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
	result = (DeeStringObject *)DeeObject_TryReallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                  num_chars + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) uint8_t *
(DCALL DeeString_Truncate1ByteBuffer)(uint8_t *__restrict buffer, size_t num_chars) {
	DeeStringObject *result;
	result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
	Dee_ASSERT(result->s_len >= num_chars);
	result = (DeeStringObject *)DeeObject_TryReallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                  num_chars + 1, sizeof(char));
	if unlikely(!result)
		result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
}

LOCAL void (DCALL DeeString_Free1ByteBuffer)(uint8_t *buffer) {
	if (buffer)
		DeeObject_Free(COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str));
}
#else /* NDEBUG */
#define DeeString_New1ByteBuffer(num_chars) \
	DeeDbgString_New1ByteBuffer(num_chars, __FILE__, __LINE__)
#define DeeString_New2ByteBuffer(num_chars) \
	DeeDbgString_New2ByteBuffer(num_chars, __FILE__, __LINE__)
#define DeeString_New4ByteBuffer(num_chars) \
	DeeDbgString_New4ByteBuffer(num_chars, __FILE__, __LINE__)
#define DeeString_TryNew1ByteBuffer(num_chars) \
	DeeDbgString_TryNew1ByteBuffer(num_chars, __FILE__, __LINE__)
#define DeeString_TryNew2ByteBuffer(num_chars) \
	DeeDbgString_TryNew2ByteBuffer(num_chars, __FILE__, __LINE__)
#define DeeString_TryNew4ByteBuffer(num_chars) \
	DeeDbgString_TryNew4ByteBuffer(num_chars, __FILE__, __LINE__)
#define DeeString_Resize1ByteBuffer(buffer, num_chars) \
	DeeDbgString_Resize1ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_Resize2ByteBuffer(buffer, num_chars) \
	DeeDbgString_Resize2ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_Resize4ByteBuffer(buffer, num_chars) \
	DeeDbgString_Resize4ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_TryResize1ByteBuffer(buffer, num_chars) \
	DeeDbgString_TryResize1ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_TryResize2ByteBuffer(buffer, num_chars) \
	DeeDbgString_TryResize2ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_TryResize4ByteBuffer(buffer, num_chars) \
	DeeDbgString_TryResize4ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_Truncate1ByteBuffer(buffer, num_chars) \
	DeeDbgString_Truncate1ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_Truncate2ByteBuffer(buffer, num_chars) \
	DeeDbgString_Truncate2ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_Truncate4ByteBuffer(buffer, num_chars) \
	DeeDbgString_Truncate4ByteBuffer(buffer, num_chars, __FILE__, __LINE__)
#define DeeString_Free1ByteBuffer(buffer) \
	DeeDbgString_Free1ByteBuffer(buffer, __FILE__, __LINE__)
#define DeeString_Free2ByteBuffer(buffer) \
	_DeeDbgString_FreeBuffer(buffer, __FILE__, __LINE__)
#define DeeString_Free4ByteBuffer(buffer) \
	_DeeDbgString_FreeBuffer(buffer, __FILE__, __LINE__)
#define DeeDbgString_Free2ByteBuffer(buffer, file, line) \
	_DeeDbgString_FreeBuffer(buffer, file, line)
#define DeeDbgString_Free4ByteBuffer(buffer, file, line) \
	_DeeDbgString_FreeBuffer(buffer, file, line)

LOCAL ATTR_MALLOC WUNUSED uint16_t *
(DCALL DeeDbgString_New2ByteBuffer)(size_t num_chars, char const *file, int line) {
	size_t *result = (size_t *)DeeDbg_Malloc(DeeString_SizeOf2ByteBuffer(num_chars), file, line);
	if likely(result)
		*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL ATTR_MALLOC WUNUSED uint32_t *
(DCALL DeeDbgString_New4ByteBuffer)(size_t num_chars, char const *file, int line) {
	size_t *result = (size_t *)DeeDbg_Malloc(DeeString_SizeOf4ByteBuffer(num_chars), file, line);
	if likely(result)
		*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL ATTR_MALLOC WUNUSED uint16_t *
(DCALL DeeDbgString_TryNew2ByteBuffer)(size_t num_chars, char const *file, int line) {
	size_t *result = (size_t *)DeeDbg_TryMalloc(DeeString_SizeOf2ByteBuffer(num_chars), file, line);
	if likely(result)
		*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL ATTR_MALLOC WUNUSED uint32_t *
(DCALL DeeDbgString_TryNew4ByteBuffer)(size_t num_chars, char const *file, int line) {
	size_t *result = (size_t *)DeeDbg_TryMalloc(DeeString_SizeOf4ByteBuffer(num_chars), file, line);
	if likely(result)
		*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL WUNUSED uint16_t *
(DCALL DeeDbgString_Resize2ByteBuffer)(uint16_t *buffer, size_t num_chars,
                                       char const *file, int line) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)DeeDbg_Realloc(result, DeeString_SizeOf2ByteBuffer(num_chars), file, line);
	if likely(result)
		*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL WUNUSED uint32_t *
(DCALL DeeDbgString_Resize4ByteBuffer)(uint32_t *buffer, size_t num_chars,
                                       char const *file, int line) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)DeeDbg_Realloc(result, DeeString_SizeOf4ByteBuffer(num_chars), file, line);
	if likely(result)
		*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL WUNUSED uint16_t *
(DCALL DeeDbgString_TryResize2ByteBuffer)(uint16_t *buffer, size_t num_chars,
                                          char const *file, int line) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)DeeDbg_TryRealloc(result, DeeString_SizeOf2ByteBuffer(num_chars), file, line);
	if likely(result) {
		*result++ = num_chars;
	} else if (buffer && num_chars < Dee_WSTR_LENGTH(buffer)) {
		Dee_WSTR_LENGTH(buffer) = num_chars;
		result = (size_t *)buffer;
	}
	return (uint16_t *)result;
}

LOCAL WUNUSED uint32_t *
(DCALL DeeDbgString_TryResize4ByteBuffer)(uint32_t *buffer, size_t num_chars,
                                          char const *file, int line) {
	size_t *result;
	result = buffer ? (size_t *)buffer - 1 : NULL;
	result = (size_t *)DeeDbg_TryRealloc(result, DeeString_SizeOf4ByteBuffer(num_chars), file, line);
	if likely(result) {
		*result++ = num_chars;
	} else if (buffer && num_chars < Dee_WSTR_LENGTH(buffer)) {
		Dee_WSTR_LENGTH(buffer) = num_chars;
		result = (size_t *)buffer;
	}
	return (uint32_t *)result;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) uint16_t *
(DCALL DeeDbgString_Truncate2ByteBuffer)(uint16_t *__restrict buffer, size_t num_chars,
                                         char const *file, int line) {
	size_t *result;
	Dee_ASSERT(*((size_t *)buffer - 1) >= num_chars);
	result = (size_t *)DeeDbg_TryRealloc((size_t *)buffer - 1,
	                                     DeeString_SizeOf2ByteBuffer(num_chars),
	                                     file, line);
	if unlikely(!result)
		result = (size_t *)buffer - 1;
	*result++ = num_chars;
	return (uint16_t *)result;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) uint32_t *
(DCALL DeeDbgString_Truncate4ByteBuffer)(uint32_t *__restrict buffer, size_t num_chars,
                                         char const *file, int line) {
	size_t *result;
	result = (size_t *)DeeDbg_TryRealloc((size_t *)buffer - 1,
	                                     DeeString_SizeOf4ByteBuffer(num_chars),
	                                     file, line);
	if unlikely(!result)
		result = (size_t *)buffer - 1;
	*result++ = num_chars;
	return (uint32_t *)result;
}

LOCAL void (DCALL _DeeDbgString_FreeBuffer)(void *buffer, char const *file, int line) {
	if (buffer)
		DeeDbg_Free((size_t *)buffer - 1, file, line);
}

LOCAL ATTR_MALLOC WUNUSED uint8_t *
(DCALL DeeDbgString_New1ByteBuffer)(size_t num_chars, char const *file, int line) {
	DeeStringObject *result;
	result = (DeeStringObject *)DeeDbgObject_Mallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                 num_chars + 1, sizeof(char), file, line);
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL ATTR_MALLOC WUNUSED uint8_t *
(DCALL DeeDbgString_TryNew1ByteBuffer)(size_t num_chars, char const *file, int line) {
	DeeStringObject *result;
	result = (DeeStringObject *)DeeDbgObject_TryMallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                    num_chars + 1, sizeof(char), file, line);
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL WUNUSED uint8_t *
(DCALL DeeDbgString_Resize1ByteBuffer)(uint8_t *buffer, size_t num_chars,
                                       char const *file, int line) {
	DeeStringObject *result;
	result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
	result = (DeeStringObject *)DeeDbgObject_Reallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                  num_chars + 1, sizeof(char), file, line);
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL WUNUSED uint8_t *
(DCALL DeeDbgString_TryResize1ByteBuffer)(uint8_t *buffer, size_t num_chars,
                                          char const *file, int line) {
	DeeStringObject *result;
	result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
	result = (DeeStringObject *)DeeDbgObject_TryReallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                     num_chars + 1, sizeof(char), file, line);
	if unlikely(!result)
		goto err;
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
err:
	return NULL;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) uint8_t *
(DCALL DeeDbgString_Truncate1ByteBuffer)(uint8_t *__restrict buffer, size_t num_chars,
                                         char const *file, int line) {
	DeeStringObject *result;
	result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
	Dee_ASSERT(result->s_len >= num_chars);
	result = (DeeStringObject *)DeeDbgObject_TryReallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
	                                                     num_chars + 1, sizeof(char), file, line);
	if unlikely(!result)
		result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
	result->s_len = num_chars;
	return (uint8_t *)result->s_str;
}

LOCAL void (DCALL DeeDbgString_Free1ByteBuffer)(uint8_t *buffer, char const *file, int line) {
	if (buffer)
		DeeDbgObject_Free(COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str), file, line);
}
#endif /* !NDEBUG */

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeString_Pack1ByteBuffer)(/*inherit(always)*/ uint8_t *__restrict buffer) {
	DREF DeeStringObject *result;
	result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
	result->s_data               = NULL;
	result->s_hash               = DEE_STRING_HASH_UNSET;
	result->s_str[result->s_len] = '\0';
	DeeObject_Init(result, &DeeString_Type);
	return (DREF DeeObject *)result;
}

LOCAL WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_PackUtf8Buffer)(/*inherit(always)*/ uint8_t *__restrict buffer,
                                 unsigned int error_mode) {
	return DeeString_SetUtf8(DeeString_Pack1ByteBuffer(buffer), error_mode);
}

#define DeeString_TryPack1ByteBuffer(buffer) DeeString_Pack1ByteBuffer(buffer)

LOCAL WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_TryPackUtf8Buffer)(/*inherit(on_success)*/ uint8_t *__restrict buffer) {
	return DeeString_TrySetUtf8(DeeString_Pack1ByteBuffer(buffer));
}
#endif /* !__INTELLISENSE__ */



/* API for string construction from buffers, with character size being determined at runtime.
 * NOTE: All of these functions take a `width' argument that is one of `STRING_WIDTH_*BYTE',
 *       behaving identical to the same `DeeString_*NByteBuffer' function.
 * NOTE: Packing of strings is done as raw word streams (no utf-8/16/32 decoding is performed!) */
#ifdef __INTELLISENSE__
LOCAL ATTR_MALLOC WUNUSED void *(DCALL DeeString_NewWidthBuffer)(size_t num_chars, unsigned int width);
LOCAL ATTR_MALLOC WUNUSED void *(DCALL DeeString_TryNewWidthBuffer)(size_t num_chars, unsigned int width);
LOCAL WUNUSED void *(DCALL DeeString_ResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width);
LOCAL WUNUSED void *(DCALL DeeString_TryResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width);
LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) void *(DCALL DeeString_TruncateWidthBuffer)(void *__restrict buffer, size_t num_chars, unsigned int width);
LOCAL void (DCALL DeeString_FreeWidthBuffer)(void *buffer, unsigned int width);
LOCAL ATTR_MALLOC WUNUSED void *(DCALL DeeDbgString_NewWidthBuffer)(size_t num_chars, unsigned int width, char const *file, int line);
LOCAL ATTR_MALLOC WUNUSED void *(DCALL DeeDbgString_TryNewWidthBuffer)(size_t num_chars, unsigned int width, char const *file, int line);
LOCAL WUNUSED void *(DCALL DeeDbgString_ResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width, char const *file, int line);
LOCAL WUNUSED void *(DCALL DeeDbgString_TryResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width, char const *file, int line);
LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) void *(DCALL DeeDbgString_TruncateWidthBuffer)(void *__restrict buffer, size_t num_chars, unsigned int width, char const *file, int line);
LOCAL void (DCALL DeeDbgString_FreeWidthBuffer)(void *buffer, unsigned int width, char const *file, int line);
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_PackWidthBuffer)(/*inherit(always)*/ void *__restrict buffer, unsigned int width);
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeString_TryPackWidthBuffer)(/*inherit(on_success)*/ void *__restrict buffer, unsigned int width);
#else /* __INTELLISENSE__ */
#ifdef NDEBUG
#define DeeDbgString_NewWidthBuffer(num_chars, width, file, line) \
	DeeString_NewWidthBuffer(num_chars, width)
#define DeeDbgString_TryNewWidthBuffer(num_chars, width, file, line) \
	DeeString_TryNewWidthBuffer(num_chars, width)
#define DeeDbgString_ResizeWidthBuffer(buffer, num_chars, width, file, line) \
	DeeString_ResizeWidthBuffer(buffer, num_chars, width)
#define DeeDbgString_TryResizeWidthBuffer(buffer, num_chars, width, file, line) \
	DeeString_TryResizeWidthBuffer(buffer, num_chars, width)
#define DeeDbgString_TruncateWidthBuffer(buffer, num_chars, width, file, line) \
	DeeString_TruncateWidthBuffer(buffer, num_chars, width)
#define DeeDbgString_PackWidthBuffer(buffer, width, file, line) \
	DeeString_PackWidthBuffer(buffer, width)
#define DeeDbgString_TryPackWidthBuffer(buffer, width, file, line) \
	DeeString_TryPackWidthBuffer(buffer, width)
#define DeeDbgString_FreeWidthBuffer(buffer, width, file, line) \
	DeeString_FreeWidthBuffer(buffer, width)

LOCAL ATTR_MALLOC WUNUSED void *
(DCALL DeeString_NewWidthBuffer)(size_t num_chars, unsigned int width) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = (DeeStringObject *)DeeObject_Mallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                              num_chars + 1, sizeof(char));
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = (size_t *)Dee_Malloc(DeeString_SizeOfWidthBuffer(num_chars, width));
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL ATTR_MALLOC WUNUSED void *(DCALL DeeString_TryNewWidthBuffer)(size_t num_chars, unsigned int width) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = (DeeStringObject *)DeeObject_TryMallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                 num_chars + 1, sizeof(char));
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = (size_t *)Dee_TryMalloc(DeeString_SizeOfWidthBuffer(num_chars, width));
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL WUNUSED void *(DCALL DeeString_ResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
		result = (DeeStringObject *)DeeObject_Reallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                               num_chars + 1, sizeof(char));
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = buffer ? (size_t *)buffer - 1 : NULL;
		result = (size_t *)Dee_Realloc(result, DeeString_SizeOfWidthBuffer(num_chars, width));
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL WUNUSED void *(DCALL DeeString_TryResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
		result = (DeeStringObject *)DeeObject_TryReallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                  num_chars + 1, sizeof(char));
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = buffer ? (size_t *)buffer - 1 : NULL;
		result = (size_t *)Dee_TryRealloc(result, DeeString_SizeOfWidthBuffer(num_chars, width));
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) void *
(DCALL DeeString_TruncateWidthBuffer)(void *__restrict buffer, size_t num_chars, unsigned int width) {
	Dee_ASSERT(*((size_t *)buffer - 1) >= num_chars);
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = (DeeStringObject *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str),
		                                                  COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                  num_chars + 1, sizeof(char));
		if unlikely(!result)
			result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = (size_t *)Dee_TryRealloc((size_t *)buffer - 1, DeeString_SizeOfWidthBuffer(num_chars, width));
		if unlikely(!result)
			result = (size_t *)buffer - 1;
		*result++ = num_chars;
		return result;
	}
}

LOCAL void (DCALL DeeString_FreeWidthBuffer)(void *buffer, unsigned int width) {
	if (buffer) {
		if (width == Dee_STRING_WIDTH_1BYTE) {
			DeeObject_Free(COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str));
		} else {
			Dee_Free((size_t *)buffer - 1);
		}
	}
}

#else /* NDEBUG */

#define DeeString_NewWidthBuffer(num_chars, width) \
	DeeDbgString_NewWidthBuffer(num_chars, width, __FILE__, __LINE__)
#define DeeString_TryNewWidthBuffer(num_chars, width) \
	DeeDbgString_TryNewWidthBuffer(num_chars, width, __FILE__, __LINE__)
#define DeeString_ResizeWidthBuffer(buffer, num_chars, width) \
	DeeDbgString_ResizeWidthBuffer(buffer, num_chars, width, __FILE__, __LINE__)
#define DeeString_TryResizeWidthBuffer(buffer, num_chars, width) \
	DeeDbgString_TryResizeWidthBuffer(buffer, num_chars, width, __FILE__, __LINE__)
#define DeeString_TruncateWidthBuffer(buffer, num_chars, width) \
	DeeDbgString_TruncateWidthBuffer(buffer, num_chars, width, __FILE__, __LINE__)
#define DeeString_FreeWidthBuffer(buffer, width) \
	DeeDbgString_FreeWidthBuffer(buffer, width, __FILE__, __LINE__)

LOCAL ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgString_NewWidthBuffer)(size_t num_chars, unsigned int width, char const *file, int line) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = (DeeStringObject *)DeeDbgObject_Mallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                 num_chars + 1, sizeof(char), file, line);
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = (size_t *)DeeDbg_Malloc(DeeString_SizeOfWidthBuffer(num_chars, width), file, line);
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgString_TryNewWidthBuffer)(size_t num_chars, unsigned int width, char const *file, int line) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = (DeeStringObject *)DeeDbgObject_TryMallocc(COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                    num_chars + 1, sizeof(char), file, line);
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = (size_t *)DeeDbg_TryMalloc(DeeString_SizeOfWidthBuffer(num_chars, width), file, line);
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL WUNUSED void *
(DCALL DeeDbgString_ResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width, char const *file, int line) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
		result = (DeeStringObject *)DeeDbgObject_Reallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                  num_chars + 1, sizeof(char), file, line);
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = buffer ? (size_t *)buffer - 1 : NULL;
		result = (size_t *)DeeDbg_Realloc(result, DeeString_SizeOfWidthBuffer(num_chars, width), file, line);
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL WUNUSED void *
(DCALL DeeDbgString_TryResizeWidthBuffer)(void *buffer, size_t num_chars, unsigned int width, char const *file, int line) {
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = buffer ? COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str) : NULL;
		result = (DeeStringObject *)DeeDbgObject_TryReallocc(result, COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                     num_chars + 1, sizeof(char), file, line);
		if unlikely(!result)
			goto err;
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = buffer ? (size_t *)buffer - 1 : NULL;
		result = (size_t *)DeeDbg_TryRealloc(result, DeeString_SizeOfWidthBuffer(num_chars, width), file, line);
		if unlikely(!result)
			goto err;
		*result++ = num_chars;
		return result;
	}
err:
	return NULL;
}

LOCAL WUNUSED ATTR_RETNONNULL NONNULL((1)) void *
(DCALL DeeDbgString_TruncateWidthBuffer)(void *__restrict buffer, size_t num_chars, unsigned int width,
                                         char const *file, int line) {
	Dee_ASSERT(*((size_t *)buffer - 1) >= num_chars);
	if (width == Dee_STRING_WIDTH_1BYTE) {
		DeeStringObject *result;
		result = (DeeStringObject *)DeeDbgObject_TryReallocc(COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str),
		                                                     COMPILER_OFFSETOF(DeeStringObject, s_str),
		                                                     num_chars + 1, sizeof(char), file, line);
		if unlikely(!result)
			result = COMPILER_CONTAINER_OF((char *)buffer, DeeStringObject, s_str);
		result->s_len = num_chars;
		return (uint8_t *)result->s_str;
	} else {
		size_t *result;
		result = (size_t *)DeeDbg_TryRealloc((size_t *)buffer - 1, DeeString_SizeOfWidthBuffer(num_chars, width), file, line);
		if unlikely(!result)
			result = (size_t *)buffer - 1;
		*result++ = num_chars;
		return result;
	}
}

LOCAL void
(DCALL DeeDbgString_FreeWidthBuffer)(void *buffer, unsigned int width, char const *file, int line) {
	if (buffer) {
		if (width == Dee_STRING_WIDTH_1BYTE) {
			DeeDbgObject_Free(COMPILER_CONTAINER_OF((char *)buffer,
			                                        DeeStringObject,
			                                        s_str),
			                  file, line);
		} else {
			DeeDbg_Free((size_t *)buffer - 1, file, line);
		}
	}
}

#endif /* !NDEBUG */

LOCAL WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_PackWidthBuffer)(/*inherit(always)*/ void *__restrict buffer, unsigned int width) {
	Dee_SWITCH_SIZEOF_WIDTH(width) {

	Dee_CASE_WIDTH_1BYTE:
		return DeeString_Pack1ByteBuffer((uint8_t *)buffer);

	Dee_CASE_WIDTH_2BYTE:
		return DeeString_Pack2ByteBuffer((uint16_t *)buffer);

	Dee_CASE_WIDTH_4BYTE:
		return DeeString_Pack4ByteBuffer((uint32_t *)buffer);
	}
}

LOCAL WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_TryPackWidthBuffer)(/*inherit(on_success)*/ void *__restrict buffer, unsigned int width) {
	Dee_SWITCH_SIZEOF_WIDTH(width) {

	Dee_CASE_WIDTH_1BYTE:
		return DeeString_TryPack1ByteBuffer((uint8_t *)buffer);

	Dee_CASE_WIDTH_2BYTE:
		return DeeString_TryPack2ByteBuffer((uint16_t *)buffer);

	Dee_CASE_WIDTH_4BYTE:
		return DeeString_TryPack4ByteBuffer((uint32_t *)buffer);
	}
}
#endif /* !__INTELLISENSE__ */




LOCAL WUNUSED DREF DeeObject *DCALL
DeeString_NewWithWidth(void const *__restrict str,
                       size_t length, unsigned int width) {
	Dee_SWITCH_SIZEOF_WIDTH(width) {

	Dee_CASE_WIDTH_1BYTE:
		return DeeString_New1Byte((uint8_t *)str, length);

	Dee_CASE_WIDTH_2BYTE:
		return DeeString_New2Byte((uint16_t *)str, length);

	Dee_CASE_WIDTH_4BYTE:
		return DeeString_New4Byte((uint32_t *)str, length);
	}
}



/* Generic format printer function (can be used to implement
 * both string (unicode), as well as Bytes formatting)
 * @param: pattern_printer: Printer used to emit data from `pattern' (i.e. static data)
 *                          Note that this is a distinct argument so-as to allow `pattern'
 *                          to be an ASCII string, rather than it needing to be utf-8.
 * @param: data_printer:    Printer used to emit data from `args' (i.e. dynamic data) */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeString_FormatPrinter(char const *pattern, size_t pattern_length, DeeObject *args,
                        Dee_formatprinter_t pattern_printer,
                        Dee_formatprinter_t data_printer, void *arg);

/* API functions for `string.format'. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
DeeString_FormatWStr(/*utf-8*/ char const *pattern_wstr, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
DeeString_Format(DeeObject *pattern, DeeObject *args);




/* Return a string containing a single character. */
#if defined(__INTELLISENSE__) && defined(__cplusplus)
extern "C++" {
DREF DeeObject *DeeString_Chr(uint8_t ch);
DREF DeeObject *DeeString_Chr(uint16_t ch);
DREF DeeObject *DeeString_Chr(uint32_t ch);
}
#else /* __INTELLISENSE__ && __cplusplus */
#ifndef __NO_builtin_choose_expr
#define DeeString_Chr(ch)                                                   \
	__builtin_choose_expr(sizeof(ch) == 1, DeeString_Chr8((uint8_t)(ch)),   \
	__builtin_choose_expr(sizeof(ch) == 2, DeeString_Chr16((uint16_t)(ch)), \
	                                       DeeString_Chr32((uint32_t)(ch))))
#else /* !__NO_builtin_choose_expr */
#define DeeString_Chr(ch)                                \
	(sizeof(ch) == 1 ? DeeString_Chr8((uint8_t)(ch)) :   \
	 sizeof(ch) == 2 ? DeeString_Chr16((uint16_t)(ch)) : \
	                   DeeString_Chr32((uint32_t)(ch)))
#endif /* __NO_builtin_choose_expr */
DFUNDEF WUNUSED DREF DeeObject *(DCALL DeeString_Chr8)(uint8_t ch);
DFUNDEF WUNUSED DREF DeeObject *(DCALL DeeString_Chr16)(uint16_t ch);
DFUNDEF WUNUSED DREF DeeObject *(DCALL DeeString_Chr32)(uint32_t ch);
#endif /* !__INTELLISENSE__ || !__cplusplus */

#if defined(CONFIG_BUILDING_DEEMON) && defined(CONFIG_STRING_LATIN1_STATIC)
typedef struct {
	Dee_OBJECT_HEAD
	struct Dee_string_utf *s_data;
	Dee_hash_t             s_hash;
	size_t                 s_len;
	unsigned char          s_str[2];
} DeeStringObject1Char;

INTDEF DeeStringObject1Char DeeString_Latin1[256];
#endif /* CONFIG_BUILDING_DEEMON && CONFIG_STRING_LATIN1_STATIC */



#ifdef CONFIG_HAVE_UNICODE_H
#define Dee_UNICODE_ISCNTRL    __UNICODE_ISCNTRL
#define Dee_UNICODE_ISCTAB     __UNICODE_ISCTAB
#define Dee_UNICODE_ISXTAB     __UNICODE_ISXTAB
#define Dee_UNICODE_ISTAB      __UNICODE_ISTAB
#define Dee_UNICODE_ISWHITE    __UNICODE_ISWHITE
#define Dee_UNICODE_ISEMPTY    __UNICODE_ISEMPTY
#define Dee_UNICODE_ISLF       __UNICODE_ISLF
#define Dee_UNICODE_ISSPACE    __UNICODE_ISSPACE
#define Dee_UNICODE_ISLOWER    __UNICODE_ISLOWER
#define Dee_UNICODE_ISUPPER    __UNICODE_ISUPPER
#define Dee_UNICODE_ISXALPHA   __UNICODE_ISXALPHA
#define Dee_UNICODE_ISALPHA    __UNICODE_ISALPHA
#define Dee_UNICODE_ISDIGIT    __UNICODE_ISDIGIT
#define Dee_UNICODE_ISHEX      __UNICODE_ISHEX
#define Dee_UNICODE_ISXDIGIT   __UNICODE_ISXDIGIT
#define Dee_UNICODE_ISALNUM    __UNICODE_ISALNUM
#define Dee_UNICODE_ISPUNCT    __UNICODE_ISPUNCT
#define Dee_UNICODE_ISGRAPH    __UNICODE_ISGRAPH
#define Dee_UNICODE_ISPRINT    __UNICODE_ISPRINT
#define Dee_UNICODE_ISBLANK    __UNICODE_ISBLANK
#define Dee_UNICODE_ISTITLE    __UNICODE_ISTITLE
#define Dee_UNICODE_ISXNUMERIC __UNICODE_ISXNUMERIC
#define Dee_UNICODE_ISNUMERIC  __UNICODE_ISNUMERIC
#define Dee_UNICODE_ISSYMSTRT  __UNICODE_ISSYMSTRT
#define Dee_UNICODE_ISSYMCONT  __UNICODE_ISSYMCONT
typedef uint16_t Dee_uniflag_t;

#ifdef DEE_SOURCE
#undef Dee_unitraits
#define unitraits      __unitraits
#endif /* DEE_SOURCE */
#define Dee_unitraits  __unitraits
#define ut_flags       __ut_flags
#define ut_digit_idx   __ut_digit_idx
#define ut_fold_idx    __ut_fold_idx
#define ut_lower       __ut_lower
#define ut_upper       __ut_upper
#define ut_title       __ut_title

/* Unicode character traits database access. */
#define DeeUni_Descriptor(ch) __unicode_descriptor((__CHAR32_TYPE__)(ch))

/* Max number of characters which may be generated
 * by case folding a single unicode character. */
#define Dee_UNICODE_FOLDED_MAX UNICODE_FOLDED_MAX

/* case-fold the given unicode character `ch', and
 * return the number of resulting folded characters.
 * @assume(return >= 1 && return <= Dee_UNICODE_FOLDED_MAX); */
#define DeeUni_ToFolded(ch, buf) \
	((size_t)(unicode_fold((__CHAR32_TYPE__)(ch), (__CHAR32_TYPE__ *)(buf)) - (__CHAR32_TYPE__ *)(buf)))

#define DeeUni_Flags(ch) __unicode_descriptor(ch)->__ut_flags

/* ctype-style character conversion. */
#define DeeUni_ToLower unicode_tolower
#define DeeUni_ToUpper unicode_toupper
#define DeeUni_ToTitle unicode_totitle

/* character-to-decimal conversion. */
#define DeeUni_AsDigitVal(ch)               __unicode_asdigit(ch)
#define DeeUni_AsDigit(ch, radix, p_result) unicode_asdigit(ch, radix, p_result)
#define DeeUni_GetNumericIdx8(idx)          __unicode_descriptor_digit(idx)
#define DeeUni_GetNumericIdx64(idx)         __unicode_descriptor_digit64(idx)
#define DeeUni_GetNumericIdxD(idx)          __unicode_descriptor_digitd(idx)
#define DeeUni_GetNumeric8(ch)              unicode_getnumeric(ch)
#define DeeUni_GetNumeric64(ch)             unicode_getnumeric64(ch)
#define DeeUni_GetNumericD(ch)              unicode_getnumericdbl(ch)

#define DeeUni_IsCntrl(ch)    unicode_iscntrl(ch)
#define DeeUni_IsTab(ch)      unicode_istab(ch)
#define DeeUni_IsWhite(ch)    unicode_iswhite(ch)
#define DeeUni_IsEmpty(ch)    unicode_isempty(ch)
#define DeeUni_IsLF(ch)       unicode_islf(ch)
#define DeeUni_IsSpace(ch)    unicode_isspace(ch)
#define DeeUni_IsLower(ch)    unicode_islower(ch)
#define DeeUni_IsUpper(ch)    unicode_isupper(ch)
#define DeeUni_IsAlpha(ch)    unicode_isalpha(ch)
#define DeeUni_IsDigit(ch)    unicode_isdigit(ch)
#define DeeUni_IsHex(ch)      unicode_ishex(ch)
#define DeeUni_IsXDigit(ch)   unicode_isxdigit(ch)
#define DeeUni_IsAlnum(ch)    unicode_isalnum(ch)
#define DeeUni_IsPunct(ch)    unicode_ispunct(ch)
#define DeeUni_IsGraph(ch)    unicode_isgraph(ch)
#define DeeUni_IsPrint(ch)    unicode_isprint(ch)
#define DeeUni_IsBlank(ch)    unicode_isblank(ch)
#define DeeUni_IsTitle(ch)    unicode_istitle(ch)
#define DeeUni_IsNumeric(ch)  unicode_isnumeric(ch)
#define DeeUni_IsSymStrt(ch)  unicode_issymstrt(ch)
#define DeeUni_IsSymCont(ch)  unicode_issymcont(ch)

#else /* CONFIG_HAVE_UNICODE_H */
#define Dee_UNICODE_ISCNTRL  0x0001                                      /* iscntrl: 00-1F, 7F */
#define Dee_UNICODE_ISCTAB   0x0002                                      /*          09 */
#define Dee_UNICODE_ISXTAB   0x0004                                      /*          0B-0C */
#define Dee_UNICODE_ISTAB    (Dee_UNICODE_ISCTAB | Dee_UNICODE_ISXTAB)   /*          09, 0B-0C */
#define Dee_UNICODE_ISWHITE  0x0008                                      /*          20 */
#define Dee_UNICODE_ISEMPTY  (Dee_UNICODE_ISTAB | Dee_UNICODE_ISWHITE)   /*          09, 0B-0C, 20 */
#define Dee_UNICODE_ISLF     0x0010                                      /*          0A, 0D */
#define Dee_UNICODE_ISSPACE  (Dee_UNICODE_ISEMPTY | Dee_UNICODE_ISLF)    /* isspace: 09-0D, 20 */
#define Dee_UNICODE_ISLOWER  0x0020                                      /* islower: 61-7A */
#define Dee_UNICODE_ISUPPER  0x0040                                      /* isupper: 41-5A */
#define Dee_UNICODE_ISXALPHA 0x0080                                      /* <Misc alpha characters> (doesn't appear in ASCII) */
#define Dee_UNICODE_ISALPHA  (Dee_UNICODE_ISLOWER | Dee_UNICODE_ISUPPER | Dee_UNICODE_ISTITLE | Dee_UNICODE_ISXALPHA) /* isalpha: 41-5A, 61-7A */
#define Dee_UNICODE_ISDIGIT  0x0100                                      /*          30-39 */
#define Dee_UNICODE_ISHEX    0x0200                                      /*          41-46, 61-66 */
#define Dee_UNICODE_ISXDIGIT (Dee_UNICODE_ISDIGIT | Dee_UNICODE_ISHEX)   /* isxdigit: 30-39, 41-46, 61-66 */
#define Dee_UNICODE_ISALNUM  (Dee_UNICODE_ISALPHA | Dee_UNICODE_ISDIGIT) /* isalnum: 30-39, 41-5A, 61-7A */
#define Dee_UNICODE_ISPUNCT  0x0400                                      /* ispunct: 21-2F, 3A-40, 5B-60, 7B-7E */
#define Dee_UNICODE_ISGRAPH  (Dee_UNICODE_ISPUNCT | Dee_UNICODE_ISALNUM | Dee_UNICODE_ISXNUMERIC) /* isgraph: 21-7E */
#define Dee_UNICODE_ISPRINT  (Dee_UNICODE_ISWHITE | Dee_UNICODE_ISGRAPH) /* isprint: 20-7E */
#define Dee_UNICODE_ISBLANK  (Dee_UNICODE_ISCTAB | Dee_UNICODE_ISWHITE)  /* isblank: 09, 20 */
/* Extended unicode flags. */
#define Dee_UNICODE_ISTITLE    0x0800 /* Is title cased */
#define Dee_UNICODE_ISXNUMERIC 0x1000 /* + all other characters that contain numbers, such as ² */
#define Dee_UNICODE_ISNUMERIC  (Dee_UNICODE_ISDIGIT | Dee_UNICODE_ISXNUMERIC) /* All numeric characters */
#define Dee_UNICODE_ISSYMSTRT  0x2000 /* Symbol start character */
#define Dee_UNICODE_ISSYMCONT  0x4000 /* Symbol continue character */
/*efine Dee_UNICODE_IS         0x8000  * ... */
typedef uint16_t Dee_uniflag_t;

/* Character flags for the first 256 unicode characters
 * -> Using this, we can greatly optimize unicode database
 *    lookups by checking the size of a character at compile-
 *    time, meaning that when working with 8-bit characters,
 *    no call to `DeeUni_Descriptor()' needs to be assembled.
 * NOTE: Technically, this should be called `DeeLatin1_Flags'... */
DDATDEF Dee_uniflag_t const _DeeAscii_Flags[256];

/* When `ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT', then the associated unicode
 * character has a digit value equal to `ut_digit_idx' (iow: the digit decode step
 * can be skipped, since `ut_digit_idx' _is_ the digit value) */
#define Dee_UNICODE_DIGIT_IDENTITY_COUNT 51
struct Dee_unitraits {
	Dee_uniflag_t const ut_flags;     /* Character flags (Set of `UNICODE_IS*') */
	uint8_t       const ut_digit_idx; /* Digit/decimal extension index, or `0xff'.
	                                   * When `< Dee_UNICODE_DIGIT_IDENTITY_COUNT', this _is_ the digit value. */
	uint8_t       const ut_fold_idx;  /* Unicode fold extension index, or `0xff'. */
	int32_t       const ut_lower;     /* Delta added to the character to convert it to lowercase, or 0. */
	int32_t       const ut_upper;     /* Delta added to the character to convert it to uppercase, or 0. */
	int32_t       const ut_title;     /* Delta added to the character to convert it to titlecase, or 0. */
};

/* Unicode character traits database access. */
DFUNDEF ATTR_CONST ATTR_RETNONNULL WUNUSED struct Dee_unitraits const *
(DCALL DeeUni_Descriptor)(uint32_t ch);

/* Max number of characters which may be generated
 * by case folding a single unicode character. */
#define Dee_UNICODE_FOLDED_MAX 3

/* case-fold the given unicode character `ch', and
 * return the number of resulting folded characters.
 * @assume(return >= 1 && return <= Dee_UNICODE_FOLDED_MAX); */
DFUNDEF NONNULL((2)) size_t
(DCALL DeeUni_ToFolded)(uint32_t ch, uint32_t buf[Dee_UNICODE_FOLDED_MAX]);

#ifndef __NO_builtin_choose_expr
#define DeeUni_Flags(ch)                                  \
	__builtin_choose_expr(sizeof(ch) == 1,                \
	                      _DeeAscii_Flags[(uint8_t)(ch)], \
	                      DeeUni_Descriptor(ch)->ut_flags)
#else /* !__NO_builtin_choose_expr */
#define DeeUni_Flags(ch)              \
	(sizeof(ch) == 1                  \
	 ? _DeeAscii_Flags[(uint8_t)(ch)] \
	 : DeeUni_Descriptor(ch)->ut_flags)
#endif /* __NO_builtin_choose_expr */



#define _DeeAscii_IsUpper(ch) (_DeeAscii_Flags[(uint8_t)(ch)] & Dee_UNICODE_ISUPPER)
#define _DeeAscii_IsLower(ch) (_DeeAscii_Flags[(uint8_t)(ch)] & Dee_UNICODE_ISLOWER)
#define _DeeAscii_IsDigit(ch) (_DeeAscii_Flags[(uint8_t)(ch)] & Dee_UNICODE_ISDIGIT)
#define _DeeAscii_ToLower(ch) (_DeeAscii_IsUpper(ch) ? (uint8_t)(ch) + 0x20 : (uint8_t)(ch))
#define _DeeAscii_ToUpper(ch) (_DeeAscii_IsLower(ch) ? (uint8_t)(ch) - 0x20 : (uint8_t)(ch))
#define _DeeAscii_ToTitle(ch) _DeeAscii_ToUpper(ch)

/* Lookup table for the numerical hex-value of ascii characters.
 * Bytes that can't be used as hex-characters yield some value >= 16. */
DDATDEF uint8_t const _DeeAscii_HexValue[256];
#define _DeeAscii_GetNumeric8(ch) _DeeAscii_HexValue[(uint8_t)(ch)]

LOCAL WUNUSED ATTR_CONST uint8_t DCALL _DeeAscii_SwapCase(uint8_t ch) {
	if (ch >= 0x41 && ch <= 0x5a)
		return ch + 0x20;
	if (ch >= 0x61 && ch <= 0x7a)
		return ch - 0x20;
	return ch;
}

LOCAL WUNUSED ATTR_CONST uint32_t DCALL _DeeUni_SwapCase(uint32_t ch) {
	struct Dee_unitraits const *record = DeeUni_Descriptor(ch);
	return (uint32_t)(ch + ((record->ut_flags & Dee_UNICODE_ISUPPER)
	                        ? record->ut_lower
	                        : record->ut_upper));
}



#define DeeUni_Convert(ch, kind) ((uint32_t)((ch) + *(int32_t *)((uintptr_t)DeeUni_Descriptor(ch) + (kind))))
#define Dee_UNICODE_CONVERT_LOWER offsetof(struct Dee_unitraits, ut_lower)
#define Dee_UNICODE_CONVERT_UPPER offsetof(struct Dee_unitraits, ut_upper)
#define Dee_UNICODE_CONVERT_TITLE offsetof(struct Dee_unitraits, ut_title)


/* ctype-style character conversion. */
#ifndef __NO_builtin_choose_expr
#define DeeUni_ToLower(ch)  __builtin_choose_expr(sizeof(ch) == 1, (uint32_t)_DeeAscii_ToLower(ch), (uint32_t)((ch) + DeeUni_Descriptor(ch)->ut_lower))
#define DeeUni_ToUpper(ch)  __builtin_choose_expr(sizeof(ch) == 1, (uint32_t)_DeeAscii_ToUpper(ch), (uint32_t)((ch) + DeeUni_Descriptor(ch)->ut_upper))
#define DeeUni_ToTitle(ch)  __builtin_choose_expr(sizeof(ch) == 1, (uint32_t)_DeeAscii_ToTitle(ch), (uint32_t)((ch) + DeeUni_Descriptor(ch)->ut_title))
#define DeeUni_SwapCase(ch) __builtin_choose_expr(sizeof(ch) == 1, (uint32_t)_DeeAscii_SwapCase((uint8_t)(ch)), _DeeUni_SwapCase(ch))
#else /* !__NO_builtin_choose_expr */
#define DeeUni_ToLower(ch)  (sizeof(ch) == 1 ? (uint32_t)_DeeAscii_ToLower(ch) : (uint32_t)((ch) + DeeUni_Descriptor(ch)->ut_lower))
#define DeeUni_ToUpper(ch)  (sizeof(ch) == 1 ? (uint32_t)_DeeAscii_ToUpper(ch) : (uint32_t)((ch) + DeeUni_Descriptor(ch)->ut_upper))
#define DeeUni_ToTitle(ch)  (sizeof(ch) == 1 ? (uint32_t)_DeeAscii_ToTitle(ch) : (uint32_t)((ch) + DeeUni_Descriptor(ch)->ut_title))
#define DeeUni_SwapCase(ch) (sizeof(ch) == 1 ? (uint32_t)_DeeAscii_SwapCase((uint8_t)(ch)) : _DeeUni_SwapCase(ch))
#endif /* __NO_builtin_choose_expr */



/* character-to-decimal conversion. */
DFUNDEF ATTR_CONST WUNUSED uint8_t (DCALL DeeUni_GetNumericIdx8)(uint8_t digit_idx);
DFUNDEF ATTR_CONST WUNUSED uint64_t (DCALL DeeUni_GetNumericIdx64)(uint8_t digit_idx);
DFUNDEF ATTR_CONST WUNUSED double (DCALL DeeUni_GetNumericIdxD)(uint8_t digit_idx);
LOCAL ATTR_CONST WUNUSED uint8_t (DCALL _DeeUni_GetNumeric8)(uint32_t ch) {
	struct Dee_unitraits const *traits = DeeUni_Descriptor(ch);
#ifndef __OPTIMIZE_SIZE__
	if likely(traits->ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT)
		return traits->ut_digit_idx;
#endif /* !__OPTIMIZE_SIZE__ */
	return DeeUni_GetNumericIdx8(traits->ut_digit_idx);
}

LOCAL ATTR_CONST WUNUSED uint64_t (DCALL _DeeUni_GetNumeric64)(uint32_t ch) {
	struct Dee_unitraits const *traits = DeeUni_Descriptor(ch);
#ifndef __OPTIMIZE_SIZE__
	if likely(traits->ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT)
		return traits->ut_digit_idx;
#endif /* !__OPTIMIZE_SIZE__ */
	return DeeUni_GetNumericIdx64(traits->ut_digit_idx);
}

LOCAL ATTR_CONST WUNUSED double (DCALL _DeeUni_GetNumericD)(uint32_t ch) {
	struct Dee_unitraits const *traits = DeeUni_Descriptor(ch);
#ifndef __OPTIMIZE_SIZE__
	if likely(traits->ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT)
		return (double)traits->ut_digit_idx;
#endif /* !__OPTIMIZE_SIZE__ */
	return DeeUni_GetNumericIdxD(traits->ut_digit_idx);
}
#define _DeeAscii_GetNumeric64(ch) (uint64_t)_DeeAscii_GetNumeric8(ch)
#define _DeeAscii_GetNumericD(ch)  (double)_DeeAscii_GetNumeric8(ch)
#ifndef __NO_builtin_choose_expr
#define DeeUni_GetNumeric8(ch)  __builtin_choose_expr(sizeof(ch) == 1, _DeeAscii_GetNumeric8(ch), _DeeUni_GetNumeric8(ch))
#define DeeUni_GetNumeric64(ch) __builtin_choose_expr(sizeof(ch) == 1, _DeeAscii_GetNumeric64(ch), _DeeUni_GetNumeric64(ch))
#define DeeUni_GetNumericD(ch)  __builtin_choose_expr(sizeof(ch) == 1, _DeeAscii_GetNumericD(ch), _DeeUni_GetNumericD(ch))
#else /* !__NO_builtin_choose_expr */
#define DeeUni_GetNumeric8(ch)  (sizeof(ch) == 1 ? _DeeAscii_GetNumeric8(ch) : _DeeUni_GetNumeric8(ch))
#define DeeUni_GetNumeric64(ch) (sizeof(ch) == 1 ? _DeeAscii_GetNumeric64(ch) : _DeeUni_GetNumeric64(ch))
#define DeeUni_GetNumericD(ch)  (sizeof(ch) == 1 ? _DeeAscii_GetNumericD(ch) : _DeeUni_GetNumericD(ch))
#endif /* __NO_builtin_choose_expr */




/* Store the digit-value of `ch' in `*p_result' and check if it belongs to `radix'
 * @return: true:  Success (`*p_result' contains the decimal value of the digit)
 * @return: false: Error. Either `ch' isn't a digit, or it can't be used in `radix'.
 *                 In this case, the contents of `*p_result' are undefined. */
#define DeeUni_AsDigit(ch, radix, p_result) \
	((*(p_result) = DeeUni_AsDigitVal(ch)) < (radix))

/* Returns 0xff if `ch' isn't a digit */
#ifndef __NO_builtin_choose_expr
#define DeeUni_AsDigitVal(ch)                        \
	__builtin_choose_expr(sizeof(ch) == 1,           \
	                      _DeeAscii_GetNumeric8(ch), \
	                      _DeeUni_FastGetDigit8(ch))
#else /* !__NO_builtin_choose_expr */
#define DeeUni_AsDigitVal(ch)    \
	(sizeof(ch) == 1             \
	 ? _DeeAscii_GetNumeric8(ch) \
	 : _DeeUni_FastGetDigit8(ch))
#endif /* __NO_builtin_choose_expr */
LOCAL ATTR_CONST WUNUSED uint8_t DCALL _DeeUni_FastGetDigit8(uint32_t ch) {
	struct Dee_unitraits const *traits = DeeUni_Descriptor(ch);
	if (!(traits->ut_flags & Dee_UNICODE_ISXDIGIT))
		return 0xff; /* Filter out stuff like `²' */
	/* We assume that `ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT', and
	 * that `Dee_UNICODE_DIGIT_IDENTITY_COUNT >= 16'. With these  assumptions,
	 * we are allowed to simply return the digit index to our caller  (which
	 * is  the `unicode_asdigit(3)'  macro), who  will then  check that said
	 * index is less than their radix.
	 *
	 * Thus, so-long as `radix <= Dee_UNICODE_DIGIT_IDENTITY_COUNT', there won't
	 * be any inconsistencies! */
	return traits->ut_digit_idx;
}
#endif /* !CONFIG_HAVE_UNICODE_H */


DDATDEF char const _DeeAscii_Itoa[101];
/* >> char DeeAscii_ItoaDigit(bool upper, uint8_t digit); */
#define DeeAscii_ItoaDigit(upper, digit) \
	_DeeAscii_Itoa[(digit) + (!!(upper) << 6)]
#define DeeAscii_ItoaDigits(upper)     (&_DeeAscii_Itoa[(!!(upper) << 6)])
#define DeeAscii_ItoaLowerDigit(digit) DeeAscii_ItoaDigit(0, digit)
#define DeeAscii_ItoaUpperDigit(digit) DeeAscii_ItoaDigit(1, digit)




#define DeeUniTrait_AsDigit(traits, radix, p_result) \
	((*(p_result) = _DeeUniTrait_FastGetDigit(traits)) < (radix))
#define _DeeUniTrait_FastGetDigit(traits) \
	((traits)->ut_flags & Dee_UNICODE_ISXDIGIT ? (traits)->ut_digit_idx : 0xff)

/* ctype-style character traits testing. */
#ifndef DeeUni_IsCntrl
#define DeeUni_IsCntrl(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISCNTRL)
#endif /* !DeeUni_IsCntrl */
#ifndef DeeUni_IsTab
#define DeeUni_IsTab(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISTAB)
#endif /* !DeeUni_IsTab */
#ifndef DeeUni_IsWhite
#define DeeUni_IsWhite(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISWHITE)
#endif /* !DeeUni_IsWhite */
#ifndef DeeUni_IsEmpty
#define DeeUni_IsEmpty(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISEMPTY)
#endif /* !DeeUni_IsEmpty */
#ifndef DeeUni_IsLF
#define DeeUni_IsLF(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISLF)
#endif /* !DeeUni_IsLF */
#ifndef DeeUni_IsSpace
#define DeeUni_IsSpace(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISSPACE)
#endif /* !DeeUni_IsSpace */
#ifndef DeeUni_IsLower
#define DeeUni_IsLower(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISLOWER)
#endif /* !DeeUni_IsLower */
#ifndef DeeUni_IsUpper
#define DeeUni_IsUpper(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISUPPER)
#endif /* !DeeUni_IsUpper */
#ifndef DeeUni_IsAlpha
#define DeeUni_IsAlpha(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISALPHA)
#endif /* !DeeUni_IsAlpha */
#ifndef DeeUni_IsDigit
#define DeeUni_IsDigit(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISDIGIT)
#endif /* !DeeUni_IsDigit */
#ifndef DeeUni_IsHex
#define DeeUni_IsHex(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISHEX)
#endif /* !DeeUni_IsHex */
#ifndef DeeUni_IsXDigit
#define DeeUni_IsXDigit(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISXDIGIT)
#endif /* !DeeUni_IsXDigit */
#ifndef DeeUni_IsAlnum
#define DeeUni_IsAlnum(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISALNUM)
#endif /* !DeeUni_IsAlnum */
#ifndef DeeUni_IsPunct
#define DeeUni_IsPunct(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISPUNCT)
#endif /* !DeeUni_IsPunct */
#ifndef DeeUni_IsGraph
#define DeeUni_IsGraph(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISGRAPH)
#endif /* !DeeUni_IsGraph */
#ifndef DeeUni_IsPrint
#define DeeUni_IsPrint(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISPRINT)
#endif /* !DeeUni_IsPrint */
#ifndef DeeUni_IsBlank
#define DeeUni_IsBlank(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISBLANK)
#endif /* !DeeUni_IsBlank */
#ifndef DeeUni_IsTitle
#define DeeUni_IsTitle(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISTITLE)
#endif /* !DeeUni_IsTitle */
#ifndef DeeUni_IsNumeric
#define DeeUni_IsNumeric(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISNUMERIC)
#endif /* !DeeUni_IsNumeric */
#ifndef DeeUni_IsSymStrt
#define DeeUni_IsSymStrt(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISSYMSTRT)
#endif /* !DeeUni_IsSymStrt */
#ifndef DeeUni_IsSymCont
#define DeeUni_IsSymCont(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISSYMCONT)
#endif /* !DeeUni_IsSymCont */
#define DeeUni_IsSpaceNoLf(ch) (DeeUni_Flags(ch) & Dee_UNICODE_ISEMPTY)

#ifdef DEE_SOURCE
#define UNICODE_ISCNTRL    Dee_UNICODE_ISCNTRL
#define UNICODE_ISCTAB     Dee_UNICODE_ISCTAB
#define UNICODE_ISXTAB     Dee_UNICODE_ISXTAB
#define UNICODE_ISTAB      Dee_UNICODE_ISTAB
#define UNICODE_ISWHITE    Dee_UNICODE_ISWHITE
#define UNICODE_ISEMPTY    Dee_UNICODE_ISEMPTY
#define UNICODE_ISLF       Dee_UNICODE_ISLF
#define UNICODE_ISSPACE    Dee_UNICODE_ISSPACE
#define UNICODE_ISLOWER    Dee_UNICODE_ISLOWER
#define UNICODE_ISUPPER    Dee_UNICODE_ISUPPER
#define UNICODE_ISXALPHA   Dee_UNICODE_ISXALPHA
#define UNICODE_ISALPHA    Dee_UNICODE_ISALPHA
#define UNICODE_ISDIGIT    Dee_UNICODE_ISDIGIT
#define UNICODE_ISHEX      Dee_UNICODE_ISHEX
#define UNICODE_ISXDIGIT   Dee_UNICODE_ISXDIGIT
#define UNICODE_ISALNUM    Dee_UNICODE_ISALNUM
#define UNICODE_ISPUNCT    Dee_UNICODE_ISPUNCT
#define UNICODE_ISGRAPH    Dee_UNICODE_ISGRAPH
#define UNICODE_ISPRINT    Dee_UNICODE_ISPRINT
#define UNICODE_ISBLANK    Dee_UNICODE_ISBLANK
#define UNICODE_ISTITLE    Dee_UNICODE_ISTITLE
#define UNICODE_ISXNUMERIC Dee_UNICODE_ISXNUMERIC
#define UNICODE_ISNUMERIC  Dee_UNICODE_ISNUMERIC
#define UNICODE_ISSYMSTRT  Dee_UNICODE_ISSYMSTRT
#define UNICODE_ISSYMCONT  Dee_UNICODE_ISSYMCONT
typedef Dee_uniflag_t uniflag_t;
#ifndef UNICODE_FOLDED_MAX
#define UNICODE_FOLDED_MAX Dee_UNICODE_FOLDED_MAX
#endif /* !UNICODE_FOLDED_MAX */
#define UNICODE_CONVERT_LOWER Dee_UNICODE_CONVERT_LOWER
#define UNICODE_CONVERT_UPPER Dee_UNICODE_CONVERT_UPPER
#define UNICODE_CONVERT_TITLE Dee_UNICODE_CONVERT_TITLE
#endif /* DEE_SOURCE */






/* =================================================================================== */
/*   ASCII / LATIN-1 PRINTER API (Formerly `string_printer')                           */
/*   Superseded by `Dee_unicode_printer' (only use this one for pure ascii strings!)   */
/* =================================================================================== */
struct Dee_ascii_printer {
	size_t           ap_length; /* Used string length. */
	DeeStringObject *ap_string; /* [0..1][owned] Generated string object. */
};
#define Dee_ASCII_PRINTER_INIT      { 0, NULL }
#define Dee_ASCII_PRINTER_STR(self) ((self)->ap_string->s_str)
#define Dee_ASCII_PRINTER_LEN(self) ((self)->ap_length)

#ifdef __INTELLISENSE__
NONNULL((1)) void Dee_ascii_printer_init(struct Dee_ascii_printer *__restrict self);
NONNULL((1)) void Dee_ascii_printer_fini(struct Dee_ascii_printer *__restrict self);
#else /* __INTELLISENSE__ */
#define Dee_ascii_printer_init(self) (void)((self)->ap_length = 0, (self)->ap_string = NULL)
#define Dee_ascii_printer_fini(self) DeeObject_Free((self)->ap_string)
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#define ASCII_PRINTER_INIT Dee_ASCII_PRINTER_INIT
#define ASCII_PRINTER_STR  Dee_ASCII_PRINTER_STR
#define ASCII_PRINTER_LEN  Dee_ASCII_PRINTER_LEN
#define ascii_printer_init Dee_ascii_printer_init
#define ascii_printer_fini Dee_ascii_printer_fini
#ifdef __INTELLISENSE__
#define Dee_ascii_printer_print    ascii_printer_print
#define Dee_ascii_printer_alloc    ascii_printer_alloc
#define Dee_ascii_printer_release  ascii_printer_release
#define Dee_ascii_printer_printf   ascii_printer_printf
#define Dee_ascii_printer_vprintf  ascii_printer_vprintf
#define ASCII_PRINTER_PRINT        Dee_ASCII_PRINTER_PRINT
#define Dee_ascii_printer_putc     ascii_printer_putc
#define Dee_ascii_printer_allocstr ascii_printer_allocstr
#define Dee_ascii_printer_pack     ascii_printer_pack
#endif /* __INTELLISENSE__ */
#endif /* DEE_SOURCE */



/* Append the given data to a string printer. (HINT: Use this one as a `Dee_formatprinter_t') */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DPRINTER_CC
Dee_ascii_printer_print(void *__restrict self, char const *__restrict data, size_t datalen);

/* Allocate space for `datalen' bytes at the end of `self',
 * then return a pointer to the start of this new buffer. */
DFUNDEF WUNUSED NONNULL((1)) char *DCALL
Dee_ascii_printer_alloc(struct Dee_ascii_printer *__restrict self, size_t datalen);

/* Release exactly `datalen' bytes from the printer to be
 * re-used in subsequent calls, or be truncated eventually. */
DFUNDEF NONNULL((1)) void DCALL
Dee_ascii_printer_release(struct Dee_ascii_printer *__restrict self, size_t datalen);


#ifdef __INTELLISENSE__
WUNUSED NONNULL((1, 2)) Dee_ssize_t Dee_ascii_printer_printf(struct Dee_ascii_printer *__restrict self, char const *__restrict format, ...);
WUNUSED NONNULL((1, 2)) Dee_ssize_t Dee_ascii_printer_vprintf(struct Dee_ascii_printer *__restrict self, char const *__restrict format, va_list args);
#define Dee_ASCII_PRINTER_PRINT(self, S) Dee_ascii_printer_print(self, S, COMPILER_STRLEN(S))
#else /* __INTELLISENSE__ */
#define Dee_ascii_printer_printf(self, ...)           DeeFormat_Printf(&Dee_ascii_printer_print, self, __VA_ARGS__)
#define Dee_ascii_printer_vprintf(self, format, args) DeeFormat_VPrintf(&Dee_ascii_printer_print, self, format, args)
#define Dee_ASCII_PRINTER_PRINT(self, S)              Dee_ascii_printer_print(self, S, COMPILER_STRLEN(S))
#endif /* !__INTELLISENSE__ */

/* Print a single character, returning -1 on error or 0 on success. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_ascii_printer_putc)(struct Dee_ascii_printer *__restrict self, char ch);

/* Search the buffer that has already been created for an existing instance
 * of `str...+=length' and if found, return a pointer to its location.
 * Otherwise, append the given string and return a pointer to that location.
 * Upon error (append failed to allocate more memory), NULL is returned.
 * HINT: This function is very useful when creating
 *       string tables for NUL-terminated strings:
 *       >> ascii_printer_allocstr("foobar\0"); // Table is now `foobar\0'
 *       >> ascii_printer_allocstr("foo\0");    // Table is now `foobar\0foo\0'
 *       >> ascii_printer_allocstr("bar\0");    // Table is still `foobar\0foo\0' - `bar\0' points into `foobar\0'
 * @return: * :   A pointer to a volatile memory location within the already printed string
 *                (the caller should calculate the offset to `ASCII_PRINTER_STR(self)'
 *                to ensure consistency if the function is called multiple times)
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) char *DCALL
Dee_ascii_printer_allocstr(struct Dee_ascii_printer *__restrict self,
                           char const *__restrict str, size_t length);

/* Pack together data from a string printer and return the generated contained string.
 * Upon success, as well as upon failure, the state of `self' is undefined upon return. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_ascii_printer_pack(/*inherit(always)*/ struct Dee_ascii_printer *__restrict self);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define Dee_ascii_printer_putc(self, ch) __builtin_expect(Dee_ascii_printer_putc(self, ch), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#ifndef __INTELLISENSE__
#define ascii_printer_print    Dee_ascii_printer_print
#define ascii_printer_alloc    Dee_ascii_printer_alloc
#define ascii_printer_release  Dee_ascii_printer_release
#define ascii_printer_printf   Dee_ascii_printer_printf
#define ascii_printer_vprintf  Dee_ascii_printer_vprintf
#define ASCII_PRINTER_PRINT    Dee_ASCII_PRINTER_PRINT
#define ascii_printer_putc     Dee_ascii_printer_putc
#define ascii_printer_allocstr Dee_ascii_printer_allocstr
#define ascii_printer_pack     Dee_ascii_printer_pack
#endif /* !__INTELLISENSE__ */
#endif /* DEE_SOURCE */




/* ================================================================================= */
/*   UNICODE PRINTER API                                                             */
/* ================================================================================= */
struct Dee_unicode_printer {
	size_t        up_length; /* The length (in characters) of the string printed thus far. */
#if 1
	union {
		void     *up_buffer; /* [0..1][owned(DeeString_FreeWidthBuffer)] The current width-buffer. */
		char     *_up_str;   /* Only here, so the printer's string can be viewed in a debugger. */
		Dee_wchar_t *_up_wstr;  /* Only here, so the printer's string can be viewed in a debugger. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define up_buffer  _dee_aunion.up_buffer
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
#else
	void         *up_buffer; /* [0..1][owned(DeeString_FreeWidthBuffer)] The current width-buffer. */
#endif
#define Dee_UNICODE_PRINTER_FWIDTH   0x0f /* Mask of the string-width produced by this printer. */
#define Dee_UNICODE_PRINTER_FPENDING 0xf0 /* Mask for the number of pending UTF-8 characters. */
#define Dee_UNICODE_PRINTER_FPENDING_SHFT 4 /* Shift for the number of pending UTF-8 characters. */
	unsigned char up_flags;  /* The currently required max-width of the resulting string.  */
	unsigned char up_pend[7];/* Pending UTF-8 characters. */
};

#define Dee_UNICODE_PRINTER_INIT { 0, { NULL }, Dee_STRING_WIDTH_1BYTE }

#ifdef __INTELLISENSE__
NONNULL((1)) void Dee_unicode_printer_init(struct Dee_unicode_printer *__restrict self);
NONNULL((1)) void Dee_unicode_printer_fini(struct Dee_unicode_printer *__restrict self);
#else /* __INTELLISENSE__ */
#define Dee_unicode_printer_init(self)                \
	((self)->up_length = 0, (self)->up_buffer = NULL, \
	 (self)->up_flags = Dee_STRING_WIDTH_1BYTE)
#define Dee_unicode_printer_fini(self) \
	DeeString_FreeWidthBuffer((self)->up_buffer, Dee_UNICODE_PRINTER_WIDTH(self))
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#define UNICODE_PRINTER_FWIDTH              Dee_UNICODE_PRINTER_FWIDTH
#define UNICODE_PRINTER_FPENDING            Dee_UNICODE_PRINTER_FPENDING
#define UNICODE_PRINTER_FPENDING_SHFT       Dee_UNICODE_PRINTER_FPENDING_SHFT
#define UNICODE_PRINTER_INIT                Dee_UNICODE_PRINTER_INIT
#define unicode_printer_init                Dee_unicode_printer_init
#define unicode_printer_fini                Dee_unicode_printer_fini
#define UNICODE_PRINTER_ISEMPTY             Dee_UNICODE_PRINTER_ISEMPTY
#define UNICODE_PRINTER_LENGTH              Dee_UNICODE_PRINTER_LENGTH
#define UNICODE_PRINTER_BUFSIZE             Dee_UNICODE_PRINTER_BUFSIZE
#define UNICODE_PRINTER_WIDTH               Dee_UNICODE_PRINTER_WIDTH
#define UNICODE_PRINTER_GETCHAR             Dee_UNICODE_PRINTER_GETCHAR
#define UNICODE_PRINTER_SETCHAR             Dee_UNICODE_PRINTER_SETCHAR
#define unicode_printer_truncate            Dee_unicode_printer_truncate
#define unicode_printer_clear               Dee_unicode_printer_clear
#ifdef __INTELLISENSE__
#define Dee_unicode_printer_init_string     unicode_printer_init_string
#define Dee_unicode_printer_pack            unicode_printer_pack
#define Dee_unicode_printer_trypack         unicode_printer_trypack
#define Dee_unicode_printer_allocate        unicode_printer_allocate
#define Dee_unicode_printer_putc            unicode_printer_putc
#define Dee_unicode_printer_putascii        unicode_printer_putascii
#define Dee_unicode_printer_pututf8         unicode_printer_pututf8
#define Dee_unicode_printer_pututf16        unicode_printer_pututf16
#define Dee_unicode_printer_pututf32        unicode_printer_pututf32
#define Dee_unicode_printer_put8            unicode_printer_put8
#define Dee_unicode_printer_put16           unicode_printer_put16
#define Dee_unicode_printer_put32           unicode_printer_put32
#define Dee_unicode_printer_print           unicode_printer_print
#define Dee_unicode_printer_printutf16      unicode_printer_printutf16
#define Dee_unicode_printer_printascii      unicode_printer_printascii
#define Dee_unicode_printer_printutf8       unicode_printer_printutf8
#define Dee_unicode_printer_printutf32      unicode_printer_printutf32
#define Dee_unicode_printer_printwide       unicode_printer_printwide
#define Dee_unicode_printer_repeatascii     unicode_printer_repeatascii
#define Dee_unicode_printer_reserve         unicode_printer_reserve
#define Dee_unicode_printer_printinto       unicode_printer_printinto
#define Dee_unicode_printer_printstring     unicode_printer_printstring
#define Dee_unicode_printer_print8          unicode_printer_print8
#define Dee_unicode_printer_print16         unicode_printer_print16
#define Dee_unicode_printer_print32         unicode_printer_print32
#define Dee_unicode_printer_reuse           unicode_printer_reuse
#define Dee_unicode_printer_reuse8          unicode_printer_reuse8
#define Dee_unicode_printer_reuse16         unicode_printer_reuse16
#define Dee_unicode_printer_reuse32         unicode_printer_reuse32
#define Dee_unicode_printer_alloc_utf8      unicode_printer_alloc_utf8
#define Dee_unicode_printer_tryalloc_utf8   unicode_printer_tryalloc_utf8
#define Dee_unicode_printer_resize_utf8     unicode_printer_resize_utf8
#define Dee_unicode_printer_tryresize_utf8  unicode_printer_tryresize_utf8
#define Dee_unicode_printer_free_utf8       unicode_printer_free_utf8
#define Dee_unicode_printer_commit_utf8     unicode_printer_commit_utf8
#define Dee_unicode_printer_alloc_utf16     unicode_printer_alloc_utf16
#define Dee_unicode_printer_tryalloc_utf16  unicode_printer_tryalloc_utf16
#define Dee_unicode_printer_resize_utf16    unicode_printer_resize_utf16
#define Dee_unicode_printer_tryresize_utf16 unicode_printer_tryresize_utf16
#define Dee_unicode_printer_free_utf16      unicode_printer_free_utf16
#define Dee_unicode_printer_commit_utf16    unicode_printer_commit_utf16
#define Dee_unicode_printer_alloc_utf32     unicode_printer_alloc_utf32
#define Dee_unicode_printer_tryalloc_utf32  unicode_printer_tryalloc_utf32
#define Dee_unicode_printer_resize_utf32    unicode_printer_resize_utf32
#define Dee_unicode_printer_tryresize_utf32 unicode_printer_tryresize_utf32
#define Dee_unicode_printer_free_utf32      unicode_printer_free_utf32
#define Dee_unicode_printer_commit_utf32    unicode_printer_commit_utf32
#define Dee_unicode_printer_alloc_wchar     unicode_printer_alloc_wchar
#define Dee_unicode_printer_tryalloc_wchar  unicode_printer_tryalloc_wchar
#define Dee_unicode_printer_resize_wchar    unicode_printer_resize_wchar
#define Dee_unicode_printer_tryresize_wchar unicode_printer_tryresize_wchar
#define Dee_unicode_printer_free_wchar      unicode_printer_free_wchar
#define Dee_unicode_printer_commit_wchar    unicode_printer_commit_wchar
#if 0
#define Dee_unicode_printer_alloc8          unicode_printer_alloc8
#define Dee_unicode_printer_tryalloc8       unicode_printer_tryalloc8
#define Dee_unicode_printer_resize8         unicode_printer_resize8
#define Dee_unicode_printer_tryresize8      unicode_printer_tryresize8
#define Dee_unicode_printer_free8           unicode_printer_free8
#define Dee_unicode_printer_confirm8        unicode_printer_confirm8
#define Dee_unicode_printer_alloc16         unicode_printer_alloc16
#define Dee_unicode_printer_tryalloc16      unicode_printer_tryalloc16
#define Dee_unicode_printer_resize16        unicode_printer_resize16
#define Dee_unicode_printer_tryresize16     unicode_printer_tryresize16
#define Dee_unicode_printer_free16          unicode_printer_free16
#define Dee_unicode_printer_confirm16       unicode_printer_confirm16
#define Dee_unicode_printer_alloc32         unicode_printer_alloc32
#define Dee_unicode_printer_tryalloc32      unicode_printer_tryalloc32
#define Dee_unicode_printer_resize32        unicode_printer_resize32
#define Dee_unicode_printer_tryresize32     unicode_printer_tryresize32
#define Dee_unicode_printer_free32          unicode_printer_free32
#define Dee_unicode_printer_confirm32       unicode_printer_confirm32
#endif
#define Dee_unicode_printer_memchr          unicode_printer_memchr
#define Dee_unicode_printer_memrchr         unicode_printer_memrchr
#define Dee_unicode_printer_memmove         unicode_printer_memmove
#define Dee_unicode_printer_memcmp8         unicode_printer_memcmp8
#define Dee_unicode_printer_memcmp16        unicode_printer_memcmp16
#define Dee_unicode_printer_memcmp32        unicode_printer_memcmp32
#define Dee_unicode_printer_memcpy8         unicode_printer_memcpy8
#define Dee_unicode_printer_memcpy16        unicode_printer_memcpy16
#define Dee_unicode_printer_memcpy32        unicode_printer_memcpy32
#define Dee_unicode_printer_printf          unicode_printer_printf
#define Dee_unicode_printer_vprintf         unicode_printer_vprintf
#define Dee_unicode_printer_printobject     unicode_printer_printobject
#define Dee_unicode_printer_printobjectrepr unicode_printer_printobjectrepr
#define UNICODE_PRINTER_PRINT               Dee_UNICODE_PRINTER_PRINT
#endif /* __INTELLISENSE__ */
#endif /* DEE_SOURCE */


#define Dee_UNICODE_PRINTER_ISEMPTY(x)       ((x)->up_length == 0)
#define Dee_UNICODE_PRINTER_LENGTH(x)        ((x)->up_length) /* Used length */
#define Dee_UNICODE_PRINTER_BUFSIZE(x)       ((x)->up_buffer ? Dee_WSTR_LENGTH((x)->up_buffer) : NULL) /* Allocated length */
#define Dee_UNICODE_PRINTER_WIDTH(x)         ((x)->up_flags & Dee_UNICODE_PRINTER_FWIDTH) /* Current width */
#define Dee_UNICODE_PRINTER_GETCHAR(x, i)    Dee_STRING_WIDTH_GETCHAR(Dee_UNICODE_PRINTER_WIDTH(x), (x)->up_buffer, i)    /* Get a character */
#define Dee_UNICODE_PRINTER_SETCHAR(x, i, v) Dee_STRING_WIDTH_SETCHAR(Dee_UNICODE_PRINTER_WIDTH(x), (x)->up_buffer, i, v) /* Replace a character (`v' must fit into the buffer's current width) */

#ifndef NDEBUG
#define Dee_unicode_printer_truncate(self, len)                                           \
	(void)(Dee_ASSERT((len) <= (self)->up_length),                                        \
	       (self)->up_length ? (void)Dee_UNICODE_PRINTER_SETCHAR(self, len, 0) : (void)0, \
	       (self)->up_length = (len))
#define Dee_unicode_printer_clear(self)                                                 \
	(void)((self)->up_length ? (void)Dee_UNICODE_PRINTER_SETCHAR(self, 0, 0) : (void)0, \
	       (self)->up_length = 0)
#else /* !NDEBUG */
#define Dee_unicode_printer_truncate(self, len)    \
	(void)(Dee_ASSERT((len) <= (self)->up_length), \
	       (self)->up_length = (len))
#define Dee_unicode_printer_clear(self) \
	(void)((self)->up_length = 0)
#endif /* NDEBUG */

/* Initialize a unicode printer from a given `string'
 * The caller must ensure that `!DeeObject_IsShared(string) || string == Dee_EmptyString' */
DFUNDEF NONNULL((1, 2)) void DCALL
Dee_unicode_printer_init_string(struct Dee_unicode_printer *__restrict self,
                                /*inherit(always)*/ DREF DeeObject *__restrict string);


/* _Always_ inherit all string data (even upon error) saved in
 * `self', and construct a new string from all that data, before
 * returning a reference to that string.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `Dee_unicode_printer_fini()'
 * @return: * :   A reference to the packed string.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_unicode_printer_pack(/*inherit(always)*/ struct Dee_unicode_printer *__restrict self);

/* Same as `Dee_unicode_printer_pack()', but don't throw errors upon failure, but
 * simply return `NULL' and leave `self' in a valid state, ready for the call
 * to be repeated at a later time. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_unicode_printer_trypack(/*inherit(on_success)*/ struct Dee_unicode_printer *__restrict self);

/* Try to pre-allocate memory for `num_chars' characters.
 * NOTE: This function merely acts as a hint, and calls may even be ignored.
 * @return: true:  The pre-allocation was successful.
 * @return: false: The pre-allocation has failed. */
DFUNDEF NONNULL((1)) bool DCALL
Dee_unicode_printer_allocate(struct Dee_unicode_printer *__restrict self,
                             size_t num_chars, unsigned int width);

/* Append a single character to the given printer.
 * If `ch' can't fit the currently set `up_width', copy already
 * written data into a larger representation before appending `ch'.
 * @return:  0: Successfully appended the character.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_putc)(struct Dee_unicode_printer *__restrict self,
                                 uint32_t ch);

/* Append an ASCII character. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_putascii)(struct Dee_unicode_printer *__restrict self,
                                     char ch);

/* Append a UTF-8 character. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_pututf8)(struct Dee_unicode_printer *__restrict self,
                                    uint8_t ch);

/* Append a UTF-16 character. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_unicode_printer_pututf16)(struct Dee_unicode_printer *__restrict self,
                                     uint16_t ch);

/* Append a UTF-32 character. */
#ifdef __INTELLISENSE__
WUNUSED NONNULL((1)) int
(Dee_unicode_printer_pututf32)(struct Dee_unicode_printer *__restrict self,
                               uint32_t ch);
#else /* __INTELLISENSE__ */
#define Dee_unicode_printer_pututf32(self, ch) Dee_unicode_printer_putc(self, ch)
#endif /* !__INTELLISENSE__ */

/* Append an 8-, 16-, or 32-bit unicode character. */
#ifdef __INTELLISENSE__
WUNUSED NONNULL((1)) int
(Dee_unicode_printer_put8)(struct Dee_unicode_printer *__restrict self,
                           uint8_t ch);
WUNUSED NONNULL((1)) int
(Dee_unicode_printer_put16)(struct Dee_unicode_printer *__restrict self,
                            uint16_t ch);
WUNUSED NONNULL((1)) int
(Dee_unicode_printer_put32)(struct Dee_unicode_printer *__restrict self,
                            uint32_t ch);
#else /* __INTELLISENSE__ */
#define Dee_unicode_printer_put8(self, ch)  Dee_unicode_printer_putc(self, ch)
#define Dee_unicode_printer_put16(self, ch) Dee_unicode_printer_putc(self, ch)
#define Dee_unicode_printer_put32(self, ch) Dee_unicode_printer_putc(self, ch)
#endif /* !__INTELLISENSE__ */

/* Append UTF-8 text to the back of the given printer.
 * An incomplete UTF-8 sequences can be completed by future uses of this function.
 * HINT: This function is intentionally designed as compatible with `Dee_formatprinter_t'
 *       >> DeeObject_Print(ob, &Dee_unicode_printer_print, &printer);
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DPRINTER_CC Dee_unicode_printer_print)(void *__restrict self,
                                        /*utf-8*/ char const *__restrict text,
                                        size_t textlen);

/* Append UTF-16 text to the back of the given printer.
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printutf16)(struct Dee_unicode_printer *__restrict self,
                                       /*utf-16*/ uint16_t const *__restrict text,
                                       size_t textlen);

/* Explicitly print utf-8/utf-32 text. */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printascii)(struct Dee_unicode_printer *__restrict self,
                                       /*ascii*/ char const *__restrict text,
                                       size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printutf8)(struct Dee_unicode_printer *__restrict self,
                                      /*utf-8*/ char const *__restrict text,
                                      size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printutf32)(struct Dee_unicode_printer *__restrict self,
                                       /*utf-32*/ uint32_t const *__restrict text,
                                       size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printwide)(struct Dee_unicode_printer *__restrict self,
                                      Dee_wchar_t const *__restrict text,
                                      size_t textlen);
#else /* __INTELLISENSE__ */
#define Dee_unicode_printer_printascii(self, text, textlen) \
	Dee_unicode_printer_print8(self, (uint8_t *)(text), textlen)
#define Dee_unicode_printer_printutf8(self, text, textlen) \
	Dee_unicode_printer_print(self, text, textlen)
#define Dee_unicode_printer_printutf32(self, text, textlen) \
	Dee_unicode_printer_print32(self, text, textlen)
#if __SIZEOF_WCHAR_T__ == 2
#define Dee_unicode_printer_printwide(self, text, textlen) \
	Dee_unicode_printer_printutf16(self, (uint16_t *)(text), textlen)
#else /* __SIZEOF_WCHAR_T__ == 2 */
#define Dee_unicode_printer_printwide(self, text, textlen) \
	Dee_unicode_printer_printutf32(self, (uint32_t *)(text), textlen)
#endif /* __SIZEOF_WCHAR_T__ != 2 */
#endif /* !__INTELLISENSE__ */

/* Append the ASCII character `ch' a total of `num_repetitions' times. */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t
(DCALL Dee_unicode_printer_repeatascii)(struct Dee_unicode_printer *__restrict self,
                                        char ch, size_t num_repetitions);


/* Reserve `num_chars' characters, to-be either using
 * `Dee_UNICODE_PRINTER_SETCHAR()', or `DeeString_SetChar()'.
 * The return value of this function is the starting index of the reservation,
 * which is made at the end of the currently printed portion of text.
 * @return: * : The starting index of the reservation.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t
(DCALL Dee_unicode_printer_reserve)(struct Dee_unicode_printer *__restrict self,
                                    size_t num_chars);


/* Print the entire contents of `self' into `printer'
 * NOTE: This function is allowed to erase characters from `self' if `printer'
 *       is recognized to be another unicode-printer. However, regardless of
 *       whether of not this is done, the caller must still finalize `self'! */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printinto)(struct Dee_unicode_printer *__restrict self,
                                      Dee_formatprinter_t printer, void *arg);

/* Append the characters from the given string object `string' at the end of `self' */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_printstring)(struct Dee_unicode_printer *__restrict self,
                                        DeeObject *__restrict string);


/* Print raw 8, 16 or 32-bit-per-character sequences, consisting of unicode characters.
 *  - `Dee_unicode_printer_print8' prints characters from the range U+0000 .. U+00FF (aka. latin-1)
 *  - `Dee_unicode_printer_print16' prints characters from the range U+0000 .. U+FFFF
 *  - `Dee_unicode_printer_print32' prints characters from the range U+0000 .. U+10FFFF (really is FFFFFFFF)
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_print8)(struct Dee_unicode_printer *__restrict self,
                                   uint8_t const *__restrict text, size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_print16)(struct Dee_unicode_printer *__restrict self,
                                    uint16_t const *__restrict text, size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_print32)(struct Dee_unicode_printer *__restrict self,
                                    uint32_t const *__restrict text, size_t textlen);

/* Search for existing occurrences of, or append a new instance of a given string.
 * Upon success, return the index (offset from the base) of the string (in characters).
 * @return: * : The offset from the base of string being printed, where the given `str' can be found.
 * @return: -1: Failed to allocate the string. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_reuse)(struct Dee_unicode_printer *__restrict self,
                                  /*utf-8*/ char const *__restrict str, size_t length);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_reuse8)(struct Dee_unicode_printer *__restrict self,
                                   uint8_t const *__restrict str, size_t length);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_reuse16)(struct Dee_unicode_printer *__restrict self,
                                    uint16_t const *__restrict str, size_t length);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL Dee_unicode_printer_reuse32)(struct Dee_unicode_printer *__restrict self,
                                    uint32_t const *__restrict str, size_t length);


/* Allocate buffers for UTF-8 with the intent of appending them to the end of the unicode printer.
 * Under specific circumstances, these functions allow the printer to allocate the utf-8 string as
 * in-line to the string being generated, with `Dee_unicode_printer_commit_utf8()' then checking
 * if the buffer contains non-ascii characters, in which case the string would be up-cast.
 *
 * However, if the buffer cannot be allocated in-line, it is allocated on the heap, and a later call
 * to `Dee_unicode_printer_commit_utf8()' will append it the same way `Dee_unicode_printer_print()'
 * would. Note however that when a UTF-8 buffer has been allocated, no text may be printed to the
 * printer before that buffer is either confirmed, or freed. However, this shouldn't be a problem,
 * considering the intended usage case in something like this:
 *
 * >> Dee_ssize_t print_pwd(struct Dee_unicode_printer *__restrict printer) {
 * >>     size_t bufsize = 256;
 * >>     char *buffer, *new_buffer;
 * >>     buffer = Dee_unicode_printer_alloc_utf8(printer, bufsize);
 * >>     if unlikely(!buffer) goto err;
 * >>     while unlikely(getcwd(buffer, bufsize) == NULL) {
 * >>         if (errno != ERANGE) {
 * >>             ...
 * >>             goto err_buffer;
 * >>         }
 * >>         bufsize *= 2;
 * >>         new_buffer = Dee_unicode_printer_resize_utf8(printer, buffer, bufsize);
 * >>         if unlikely(!new_buffer) goto err_buffer;
 * >>         buffer = new_buffer;
 * >>     }
 * >>     return Dee_unicode_printer_commit_utf8(printer, buffer, strlen(buffer));
 * >> err_buffer:
 * >>     Dee_unicode_printer_free_utf8(printer, buffer);
 * >> err:
 * >>     return -1;
 * >> }
 * HINT: The unicode printer always allocates 1 byte more than you requested, allowing
 *       you to write 1 past the end (usually meant for some trailing \0-character that
 *       you don't really care about)
 * NOTE: All functions operate identical to those one would expect to find in a
 *       heap-API, with `Dee_unicode_printer_commit_utf8()' acting as a semantically
 *       similar behavior to `Dee_unicode_printer_free_utf8()', which will ensure that
 *       the contents of the buffer are decoded and appended at the end of the string
 *       that is being printed.
 * NOTE: Passing `NULL' for `buf' to `Dee_unicode_printer_resize_utf8()' or
 *       `Dee_unicode_printer_tryresize_utf8()' will allocate a new buffer, the same
 *       way `Dee_unicode_printer_alloc_utf8()' and `Dee_unicode_printer_tryalloc_utf8()' would have)
 * NOTE: Passing `NULL' for `buf' to `Dee_unicode_printer_free_utf8()' is a no-op
 * NOTE: Passing `NULL' for `buf' to `Dee_unicode_printer_commit_utf8()' is a no-op and causes `0' to be returned.
 * @return[Dee_unicode_printer_alloc_utf8]:     * :   The pointer to the base of a utf-8 buffer consisting of `length' bytes.
 * @return[Dee_unicode_printer_alloc_utf8]:     NULL: An error occurred.
 * @return[Dee_unicode_printer_tryalloc_utf8]:  * :   Failed to allocate the buffer.
 * @return[Dee_unicode_printer_tryalloc_utf8]:  NULL: An error occurred.
 * @return[Dee_unicode_printer_resize_utf8]:    * :   The reallocated base pointer.
 * @return[Dee_unicode_printer_resize_utf8]:    NULL: An error occurred.
 * @return[Dee_unicode_printer_tryresize_utf8]: * :   The reallocated base pointer.
 * @return[Dee_unicode_printer_tryresize_utf8]: NULL: Failed to allocate / resize the buffer.
 * @return[Dee_unicode_printer_commit_utf8]:    * :   The number of printed characters.
 * @return[Dee_unicode_printer_commit_utf8]:    < 0 : An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) char *(DCALL Dee_unicode_printer_alloc_utf8)(struct Dee_unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) char *(DCALL Dee_unicode_printer_tryalloc_utf8)(struct Dee_unicode_printer *__restrict self, size_t length);                  /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) char *(DCALL Dee_unicode_printer_resize_utf8)(struct Dee_unicode_printer *__restrict self, char *buf, size_t new_length);     /* Dee_Realloc()-like */
DFUNDEF WUNUSED NONNULL((1)) char *(DCALL Dee_unicode_printer_tryresize_utf8)(struct Dee_unicode_printer *__restrict self, char *buf, size_t new_length);  /* Dee_TryRealloc()-like */
DFUNDEF         NONNULL((1)) void (DCALL Dee_unicode_printer_free_utf8)(struct Dee_unicode_printer *__restrict self, char *buf);                           /* Dee_Free()-like */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t (DCALL Dee_unicode_printer_commit_utf8)(struct Dee_unicode_printer *__restrict self, /*inherit(always)*/ char *buf, size_t final_length);

/* Same as the functions above, however used to allocate utf-16 string buffers. */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *(DCALL Dee_unicode_printer_alloc_utf16)(struct Dee_unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *(DCALL Dee_unicode_printer_tryalloc_utf16)(struct Dee_unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *(DCALL Dee_unicode_printer_resize_utf16)(struct Dee_unicode_printer *__restrict self, uint16_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *(DCALL Dee_unicode_printer_tryresize_utf16)(struct Dee_unicode_printer *__restrict self, uint16_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF         NONNULL((1)) void (DCALL Dee_unicode_printer_free_utf16)(struct Dee_unicode_printer *__restrict self, uint16_t *buf);                              /* Dee_Free()-like */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t (DCALL Dee_unicode_printer_commit_utf16)(struct Dee_unicode_printer *__restrict self, /*inherit(always)*/ uint16_t *buf, size_t final_length);

/* Same as the functions above, however used to allocate utf-32 string buffers. */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *(DCALL Dee_unicode_printer_alloc_utf32)(struct Dee_unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *(DCALL Dee_unicode_printer_tryalloc_utf32)(struct Dee_unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *(DCALL Dee_unicode_printer_resize_utf32)(struct Dee_unicode_printer *__restrict self, uint32_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *(DCALL Dee_unicode_printer_tryresize_utf32)(struct Dee_unicode_printer *__restrict self, uint32_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF         NONNULL((1)) void (DCALL Dee_unicode_printer_free_utf32)(struct Dee_unicode_printer *__restrict self, uint32_t *buf);                              /* Dee_Free()-like */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t (DCALL Dee_unicode_printer_commit_utf32)(struct Dee_unicode_printer *__restrict self, /*inherit(always)*/ uint32_t *buf, size_t final_length);


/* Same as the functions above, however used to allocate wide-string buffers. */
#ifdef __INTELLISENSE__
WUNUSED NONNULL((1)) Dee_wchar_t *(Dee_unicode_printer_alloc_wchar)(struct Dee_unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
WUNUSED NONNULL((1)) Dee_wchar_t *(Dee_unicode_printer_tryalloc_wchar)(struct Dee_unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
WUNUSED NONNULL((1)) Dee_wchar_t *(Dee_unicode_printer_resize_wchar)(struct Dee_unicode_printer *__restrict self, Dee_wchar_t *buf, size_t new_length);    /* Dee_Realloc()-like */
WUNUSED NONNULL((1)) Dee_wchar_t *(Dee_unicode_printer_tryresize_wchar)(struct Dee_unicode_printer *__restrict self, Dee_wchar_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
/*   */ NONNULL((1)) void (Dee_unicode_printer_free_wchar)(struct Dee_unicode_printer *__restrict self, Dee_wchar_t *buf);                              /* Dee_Free()-like */
WUNUSED NONNULL((1)) Dee_ssize_t (Dee_unicode_printer_commit_wchar)(struct Dee_unicode_printer *__restrict self, /*inherit(always)*/ Dee_wchar_t *buf, size_t final_length);
#elif __SIZEOF_WCHAR_T__ == 2
#define Dee_unicode_printer_alloc_wchar(self, length)              ((Dee_wchar_t *)Dee_unicode_printer_alloc_utf16(self, length))
#define Dee_unicode_printer_tryalloc_wchar(self, length)           ((Dee_wchar_t *)Dee_unicode_printer_tryalloc_utf16(self, length))
#define Dee_unicode_printer_resize_wchar(self, buf, new_length)    ((Dee_wchar_t *)Dee_unicode_printer_resize_utf16(self, (uint16_t *)(buf), new_length))
#define Dee_unicode_printer_tryresize_wchar(self, buf, new_length) ((Dee_wchar_t *)Dee_unicode_printer_tryresize_utf16(self, (uint16_t *)(buf), new_length))
#define Dee_unicode_printer_free_wchar(self, buf)                  Dee_unicode_printer_free_utf16(self, (uint16_t *)(buf))
#define Dee_unicode_printer_commit_wchar(self, buf, final_length)  Dee_unicode_printer_commit_utf16(self, (uint16_t *)(buf), final_length)
#else /* __SIZEOF_WCHAR_T__ == 2 */
#define Dee_unicode_printer_alloc_wchar(self, length)              ((Dee_wchar_t *)Dee_unicode_printer_alloc_utf32(self, length))
#define Dee_unicode_printer_tryalloc_wchar(self, length)           ((Dee_wchar_t *)Dee_unicode_printer_tryalloc_utf32(self, length))
#define Dee_unicode_printer_resize_wchar(self, buf, new_length)    ((Dee_wchar_t *)Dee_unicode_printer_resize_utf32(self, (uint32_t *)(buf), new_length))
#define Dee_unicode_printer_tryresize_wchar(self, buf, new_length) ((Dee_wchar_t *)Dee_unicode_printer_tryresize_utf32(self, (uint32_t *)(buf), new_length))
#define Dee_unicode_printer_free_wchar(self, buf)                  Dee_unicode_printer_free_utf32(self, (uint32_t *)(buf))
#define Dee_unicode_printer_commit_wchar(self, buf, final_length)  Dee_unicode_printer_commit_utf32(self, (uint32_t *)(buf), final_length)
#endif /* __SIZEOF_WCHAR_T__ != 2 */

#if 0
/* Allocate raw unicode character buffer, as opposed to buffers for certain encodings. */
DFUNDEF WUNUSED NONNULL((1)) uint8_t *DCALL Dee_unicode_printer_alloc8(struct Dee_unicode_printer *__restrict self, size_t length);                       /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint8_t *DCALL Dee_unicode_printer_tryalloc8(struct Dee_unicode_printer *__restrict self, size_t length);                    /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint8_t *DCALL Dee_unicode_printer_resize8(struct Dee_unicode_printer *__restrict self, uint8_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint8_t *DCALL Dee_unicode_printer_tryresize8(struct Dee_unicode_printer *__restrict self, uint8_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF         NONNULL((1)) void DCALL Dee_unicode_printer_free8(struct Dee_unicode_printer *__restrict self, uint8_t *buf);                             /* Dee_Free()-like */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL Dee_unicode_printer_confirm8(struct Dee_unicode_printer *__restrict self, uint8_t *buf, size_t final_length);
DFUNDEF WUNUSED NONNULL((1)) uint16_t *DCALL Dee_unicode_printer_alloc16(struct Dee_unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *DCALL Dee_unicode_printer_tryalloc16(struct Dee_unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *DCALL Dee_unicode_printer_resize16(struct Dee_unicode_printer *__restrict self, uint16_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint16_t *DCALL Dee_unicode_printer_tryresize16(struct Dee_unicode_printer *__restrict self, uint16_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF         NONNULL((1)) void DCALL Dee_unicode_printer_free16(struct Dee_unicode_printer *__restrict self, uint16_t *buf);                              /* Dee_Free()-like */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL Dee_unicode_printer_confirm16(struct Dee_unicode_printer *__restrict self, uint16_t *buf, size_t final_length);
DFUNDEF WUNUSED NONNULL((1)) uint32_t *DCALL Dee_unicode_printer_alloc32(struct Dee_unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *DCALL Dee_unicode_printer_tryalloc32(struct Dee_unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *DCALL Dee_unicode_printer_resize32(struct Dee_unicode_printer *__restrict self, uint32_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF WUNUSED NONNULL((1)) uint32_t *DCALL Dee_unicode_printer_tryresize32(struct Dee_unicode_printer *__restrict self, uint32_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF         NONNULL((1)) void DCALL Dee_unicode_printer_free32(struct Dee_unicode_printer *__restrict self, uint32_t *buf);                              /* Dee_Free()-like */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL Dee_unicode_printer_confirm32(struct Dee_unicode_printer *__restrict self, uint32_t *buf, size_t final_length);
#endif



/* Find a given unicode character within the specified index-range.
 * @return: * : The index of the character, offset from the start of the printer.
 * @return: -1: The character wasn't found. */
DFUNDEF WUNUSED NONNULL((1)) size_t
(DCALL Dee_unicode_printer_memchr)(struct Dee_unicode_printer *__restrict self,
                                   uint32_t chr, size_t start, size_t length);
DFUNDEF WUNUSED NONNULL((1)) size_t
(DCALL Dee_unicode_printer_memrchr)(struct Dee_unicode_printer *__restrict self,
                                    uint32_t chr, size_t start, size_t length);

/* Move `length' characters from `src' to `dst' */
DFUNDEF NONNULL((1)) void
(DCALL Dee_unicode_printer_memmove)(struct Dee_unicode_printer *__restrict self,
                                    size_t dst, size_t src, size_t length);

/* Compare characters of `self' against `rhs' */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL Dee_unicode_printer_memcmp8)(struct Dee_unicode_printer const *__restrict self,
                                    uint8_t const *rhs, size_t lhs_start, size_t num_chars);
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL Dee_unicode_printer_memcmp16)(struct Dee_unicode_printer const *__restrict self,
                                     uint16_t const *rhs, size_t lhs_start, size_t num_chars);
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL Dee_unicode_printer_memcmp32)(struct Dee_unicode_printer const *__restrict self,
                                     uint32_t const *rhs, size_t lhs_start, size_t num_chars);

/* Copy characters from `src' into `self' */
DFUNDEF NONNULL((1, 2)) void
(DCALL Dee_unicode_printer_memcpy8)(struct Dee_unicode_printer *__restrict self,
                                    uint8_t const *src, size_t dst, size_t num_chars);
DFUNDEF NONNULL((1, 2)) void
(DCALL Dee_unicode_printer_memcpy16)(struct Dee_unicode_printer *__restrict self,
                                     uint16_t const *src, size_t dst, size_t num_chars);
DFUNDEF NONNULL((1, 2)) void
(DCALL Dee_unicode_printer_memcpy32)(struct Dee_unicode_printer *__restrict self,
                                     uint32_t const *src, size_t dst, size_t num_chars);

/* Erase `count' characters at index `i' from the given unicode_printer `self' */
#define Dee_unicode_printer_erase(self, i, count)                                           \
	(void)(Dee_unicode_printer_memmove(self, i, (i) + (count),                              \
	                                   Dee_UNICODE_PRINTER_LENGTH(self) - ((i) + (count))), \
	       (self)->up_length -= (count))
#ifdef DEE_SOURCE
#define unicode_printer_erase Dee_unicode_printer_erase
#endif /* DEE_SOURCE */



#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define Dee_unicode_printer_putc(self, ch)     __builtin_expect(Dee_unicode_printer_putc(self, ch), 0)
#define Dee_unicode_printer_putascii(self, ch) __builtin_expect(Dee_unicode_printer_putascii(self, ch), 0)
#define Dee_unicode_printer_pututf8(self, ch)  __builtin_expect(Dee_unicode_printer_pututf8(self, ch), 0)
#define Dee_unicode_printer_pututf16(self, ch) __builtin_expect(Dee_unicode_printer_pututf16(self, ch), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */



/* Helper macros for printing compile-time strings, or printf-style data. */
#define Dee_UNICODE_PRINTER_PRINT(self, S) \
	Dee_unicode_printer_print8(self, (uint8_t *)(S), COMPILER_STRLEN(S))
#ifdef __INTELLISENSE__
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_unicode_printer_printf)(struct Dee_unicode_printer *__restrict self, char const *__restrict format, ...);
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_unicode_printer_vprintf)(struct Dee_unicode_printer *__restrict self, char const *__restrict format, va_list args);
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_unicode_printer_printobject)(struct Dee_unicode_printer *__restrict self, DeeObject *__restrict ob);
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_unicode_printer_printobjectrepr)(struct Dee_unicode_printer *__restrict self, DeeObject *__restrict ob);
#else /* __INTELLISENSE__ */
#define Dee_unicode_printer_printf(self, ...)           DeeFormat_Printf(&Dee_unicode_printer_print, self, __VA_ARGS__)
#define Dee_unicode_printer_vprintf(self, format, args) DeeFormat_VPrintf(&Dee_unicode_printer_print, self, format, args)
#define Dee_unicode_printer_printobject(self, ob)       DeeObject_Print(ob, &Dee_unicode_printer_print, self)
#define Dee_unicode_printer_printobjectrepr(self, ob)   DeeObject_PrintRepr(ob, &Dee_unicode_printer_print, self)
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#ifndef __INTELLISENSE__
#define unicode_printer_init_string     Dee_unicode_printer_init_string
#define unicode_printer_pack            Dee_unicode_printer_pack
#define unicode_printer_trypack         Dee_unicode_printer_trypack
#define unicode_printer_allocate        Dee_unicode_printer_allocate
#define unicode_printer_putc            Dee_unicode_printer_putc
#define unicode_printer_putascii        Dee_unicode_printer_putascii
#define unicode_printer_pututf8         Dee_unicode_printer_pututf8
#define unicode_printer_pututf16        Dee_unicode_printer_pututf16
#define unicode_printer_pututf32        Dee_unicode_printer_pututf32
#define unicode_printer_put8            Dee_unicode_printer_put8
#define unicode_printer_put16           Dee_unicode_printer_put16
#define unicode_printer_put32           Dee_unicode_printer_put32
#define unicode_printer_print           Dee_unicode_printer_print
#define unicode_printer_printutf16      Dee_unicode_printer_printutf16
#define unicode_printer_printascii      Dee_unicode_printer_printascii
#define unicode_printer_printutf8       Dee_unicode_printer_printutf8
#define unicode_printer_printutf32      Dee_unicode_printer_printutf32
#define unicode_printer_printwide       Dee_unicode_printer_printwide
#define unicode_printer_repeatascii     Dee_unicode_printer_repeatascii
#define unicode_printer_reserve         Dee_unicode_printer_reserve
#define unicode_printer_printinto       Dee_unicode_printer_printinto
#define unicode_printer_printstring     Dee_unicode_printer_printstring
#define unicode_printer_print8          Dee_unicode_printer_print8
#define unicode_printer_print16         Dee_unicode_printer_print16
#define unicode_printer_print32         Dee_unicode_printer_print32
#define unicode_printer_reuse           Dee_unicode_printer_reuse
#define unicode_printer_reuse8          Dee_unicode_printer_reuse8
#define unicode_printer_reuse16         Dee_unicode_printer_reuse16
#define unicode_printer_reuse32         Dee_unicode_printer_reuse32
#define unicode_printer_alloc_utf8      Dee_unicode_printer_alloc_utf8
#define unicode_printer_tryalloc_utf8   Dee_unicode_printer_tryalloc_utf8
#define unicode_printer_resize_utf8     Dee_unicode_printer_resize_utf8
#define unicode_printer_tryresize_utf8  Dee_unicode_printer_tryresize_utf8
#define unicode_printer_free_utf8       Dee_unicode_printer_free_utf8
#define unicode_printer_commit_utf8     Dee_unicode_printer_commit_utf8
#define unicode_printer_alloc_utf16     Dee_unicode_printer_alloc_utf16
#define unicode_printer_tryalloc_utf16  Dee_unicode_printer_tryalloc_utf16
#define unicode_printer_resize_utf16    Dee_unicode_printer_resize_utf16
#define unicode_printer_tryresize_utf16 Dee_unicode_printer_tryresize_utf16
#define unicode_printer_free_utf16      Dee_unicode_printer_free_utf16
#define unicode_printer_commit_utf16    Dee_unicode_printer_commit_utf16
#define unicode_printer_alloc_utf32     Dee_unicode_printer_alloc_utf32
#define unicode_printer_tryalloc_utf32  Dee_unicode_printer_tryalloc_utf32
#define unicode_printer_resize_utf32    Dee_unicode_printer_resize_utf32
#define unicode_printer_tryresize_utf32 Dee_unicode_printer_tryresize_utf32
#define unicode_printer_free_utf32      Dee_unicode_printer_free_utf32
#define unicode_printer_commit_utf32    Dee_unicode_printer_commit_utf32
#define unicode_printer_alloc_wchar     Dee_unicode_printer_alloc_wchar
#define unicode_printer_tryalloc_wchar  Dee_unicode_printer_tryalloc_wchar
#define unicode_printer_resize_wchar    Dee_unicode_printer_resize_wchar
#define unicode_printer_tryresize_wchar Dee_unicode_printer_tryresize_wchar
#define unicode_printer_free_wchar      Dee_unicode_printer_free_wchar
#define unicode_printer_commit_wchar    Dee_unicode_printer_commit_wchar
#if 0
#define unicode_printer_alloc8          Dee_unicode_printer_alloc8
#define unicode_printer_tryalloc8       Dee_unicode_printer_tryalloc8
#define unicode_printer_resize8         Dee_unicode_printer_resize8
#define unicode_printer_tryresize8      Dee_unicode_printer_tryresize8
#define unicode_printer_free8           Dee_unicode_printer_free8
#define unicode_printer_confirm8        Dee_unicode_printer_confirm8
#define unicode_printer_alloc16         Dee_unicode_printer_alloc16
#define unicode_printer_tryalloc16      Dee_unicode_printer_tryalloc16
#define unicode_printer_resize16        Dee_unicode_printer_resize16
#define unicode_printer_tryresize16     Dee_unicode_printer_tryresize16
#define unicode_printer_free16          Dee_unicode_printer_free16
#define unicode_printer_confirm16       Dee_unicode_printer_confirm16
#define unicode_printer_alloc32         Dee_unicode_printer_alloc32
#define unicode_printer_tryalloc32      Dee_unicode_printer_tryalloc32
#define unicode_printer_resize32        Dee_unicode_printer_resize32
#define unicode_printer_tryresize32     Dee_unicode_printer_tryresize32
#define unicode_printer_free32          Dee_unicode_printer_free32
#define unicode_printer_confirm32       Dee_unicode_printer_confirm32
#endif
#define unicode_printer_memchr          Dee_unicode_printer_memchr
#define unicode_printer_memrchr         Dee_unicode_printer_memrchr
#define unicode_printer_memmove         Dee_unicode_printer_memmove
#define unicode_printer_memcmp8         Dee_unicode_printer_memcmp8
#define unicode_printer_memcmp16        Dee_unicode_printer_memcmp16
#define unicode_printer_memcmp32        Dee_unicode_printer_memcmp32
#define unicode_printer_memcpy8         Dee_unicode_printer_memcpy8
#define unicode_printer_memcpy16        Dee_unicode_printer_memcpy16
#define unicode_printer_memcpy32        Dee_unicode_printer_memcpy32
#define unicode_printer_printf          Dee_unicode_printer_printf
#define unicode_printer_vprintf         Dee_unicode_printer_vprintf
#define unicode_printer_printobject     Dee_unicode_printer_printobject
#define unicode_printer_printobjectrepr Dee_unicode_printer_printobjectrepr
#define UNICODE_PRINTER_PRINT           Dee_UNICODE_PRINTER_PRINT
#endif /* !__INTELLISENSE__ */
#endif /* DEE_SOURCE */








/* Encode/decode `self' (usually a bytes- or string-object) to/from a codec `name'.
 * These functions will start by normalizing `name', checking if it refers to
 * one of the builtin codecs, and if it doesn't, make an external function
 * call to `encode from codecs' / `decode from codecs':
 * >> name = name.casefold().replace("_", "-");
 * >> if (name.startswith("iso-"))
 * >>     name = "iso" + name[4:];
 * >> else if (name.startswith("cp-")) {
 * >>     name = "cp" + name[3:];
 * >> }
 * >> if (has_builtin_codec(name))
 * >>     return builtin_encode(self, name, error_mode); // or decode...
 * The following is a list of the recognized builtin codecs.
 *  - "ascii", "646", "us-ascii"
 *  - "latin-1", "iso8859-1", "iso8859", "8859", "cp819", "latin", "latin1", "l1"
 *  - "utf-8", "utf8", "u8", "utf"
 *  - "utf-16", "utf16", "u16"
 *  - "utf-16-le", "utf16-le", "u16-le", "utf-16le", "utf16le", "u16le"
 *  - "utf-16-be", "utf16-be", "u16-be", "utf-16be", "utf16be", "u16be"
 *  - "utf-32", "utf32", "u32"
 *  - "utf-32-le", "utf32-le", "u32-le", "utf-32le", "utf32le", "u32le"
 *  - "utf-32-be", "utf32-be", "u32-be", "utf-32be", "utf32be", "u32be"
 *  - "string-escape", "backslash-escape", "c-escape"
 * @throw: ValueError: The given `name' is not a recognized codec name.
 * @param: error_mode: One of `STRING_ERROR_F*'
 * @return: * :   The encoded/decoded variant of `self'
 *                The type of this object is unrelated to `self', but rather
 *                depends on `self' and is usually a bytes, or string object.
 *                In most cases, `DeeCodec_Decode()' returns a string object,
 *                while `DeeCodec_Encode()' returns a Bytes object.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_Decode(DeeObject *self, DeeObject *name,
                unsigned int error_mode);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_Encode(DeeObject *self, DeeObject *name,
                unsigned int error_mode);

DECL_END

#endif /* !GUARD_DEEMON_STRING_H */

