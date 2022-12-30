/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C
#define GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/regex.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>

#include <hybrid/align.h>
#include <hybrid/alloca.h>
#include <hybrid/bit.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

#include <stdbool.h>
#include <stdint.h>
/**/

#include <__stdinc.h>
#include <hybrid/host.h>

DECL_BEGIN

/* Define missing functions */
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

#ifndef CONFIG_HAVE_memsetq
#define CONFIG_HAVE_memsetq
#undef memsetq
#define memsetq dee_memsetq
DeeSystem_DEFINE_memsetq(dee_memsetq)
#endif /* !CONFIG_HAVE_memsetq */

#undef memsetc
#define memsetc(dst, word, word_count, word_size)                                    \
	__builtin_choose_expr((word_size) == 1,                                          \
	                      (void *)memsetb(dst, (__UINT8_TYPE__)(word), word_count),  \
	__builtin_choose_expr((word_size) == 2,                                          \
	                      (void *)memsetw(dst, (__UINT16_TYPE__)(word), word_count), \
	__builtin_choose_expr((word_size) == 4,                                          \
	                      (void *)memsetl(dst, (__UINT32_TYPE__)(word), word_count), \
	                      (void *)memsetq(dst, (__UINT64_TYPE__)(word), word_count))))

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#undef memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */


/* Configure system types */
#ifndef alloca
#define alloca __hybrid_alloca
#endif /* !alloca */
#undef pformatprinter
#undef __pformatprinter
#define pformatprinter   Dee_formatprinter_t
#define __pformatprinter Dee_formatprinter_t
#undef __ssize_t
#define __ssize_t __SSIZE_TYPE__
#undef ssize_t
#define ssize_t Dee_ssize_t
#undef __size_t
#define __size_t __SIZE_TYPE__
#undef __intptr_t
#define __intptr_t __INTPTR_TYPE__
#undef __uintptr_t
#define __uintptr_t __UINTPTR_TYPE__
#undef __int8_t
#define __int8_t __INT8_TYPE__
#undef __int16_t
#define __int16_t __INT16_TYPE__
#undef __int32_t
#define __int32_t __INT32_TYPE__
#undef __int64_t
#define __int64_t __INT64_TYPE__
#undef __uint8_t
#define __uint8_t __UINT8_TYPE__
#undef __uint16_t
#define __uint16_t __UINT16_TYPE__
#undef __uint32_t
#define __uint32_t __UINT32_TYPE__
#undef __uint64_t
#define __uint64_t __UINT64_TYPE__
#undef __byte_t
#define __byte_t __BYTE_TYPE__
#undef byte_t
#define byte_t __BYTE_TYPE__

/* Configure libregex */
#define LIBREGEX_NO_RE_CODE_DISASM
#define LIBREGEX_NO_MALLOC_USABLE_SIZE /* TODO: Support for when this _is_ actually available */
#define LIBREGEX_WANT_PROTOTYPES
#define LIBREGEX_NO_SYSTEM_INCLUDES
#define LIBREGEX_DECL INTDEF
#define LIBREGEX_DEFINE___CTYPE_C_FLAGS
#define LIBREGEX_REGEXEC_SINGLE_CHUNK
#define LIBREGEX_USED__re_max_failures 2000


/* Configure libregex syntax, and inject our `DEE_REGEX_COMPILE_*' flags. */
#undef RE_SYNTAX_ICASE
#undef RE_SYNTAX_NO_UTF8
#define LIBREGEX_CONSTANT__RE_SYNTAX_BACKSLASH_ESCAPE_IN_LISTS 1
#define LIBREGEX_CONSTANT__RE_SYNTAX_BK_PLUS_QM                0
#define LIBREGEX_CONSTANT__RE_SYNTAX_CHAR_CLASSES              1
#define LIBREGEX_CONSTANT__RE_SYNTAX_CONTEXT_INDEP_ANCHORS     1
#define LIBREGEX_CONSTANT__RE_SYNTAX_CONTEXT_INVALID_OPS       1
#define LIBREGEX_CONSTANT__RE_SYNTAX_DOT_NEWLINE               1
#define LIBREGEX_CONSTANT__RE_SYNTAX_DOT_NOT_NULL              0
#define LIBREGEX_CONSTANT__RE_SYNTAX_HAT_LISTS_NOT_NEWLINE     0
#define LIBREGEX_CONSTANT__RE_SYNTAX_INTERVALS                 1
#define LIBREGEX_CONSTANT__RE_SYNTAX_LIMITED_OPS               0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NEWLINE_ALT               0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_BRACES              1
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_PARENS              1
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_REFS                0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_VBAR                1
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_EMPTY_RANGES           1
#define LIBREGEX_CONSTANT__RE_SYNTAX_UNMATCHED_RIGHT_PAREN_ORD 0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_POSIX_BACKTRACKING     0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_GNU_OPS                0
#define LIBREGEX_CONSTANT__RE_SYNTAX_INVALID_INTERVAL_ORD      0
#define RE_SYNTAX_ICASE                                        DEE_REGEX_COMPILE_ICASE
#define LIBREGEX_CONSTANT__RE_SYNTAX_CARET_ANCHORS_HERE        1
#define LIBREGEX_CONSTANT__RE_SYNTAX_CONTEXT_INVALID_DUP       1
#define LIBREGEX_CONSTANT__RE_SYNTAX_ANCHORS_IGNORE_EFLAGS     0
#define RE_SYNTAX_NO_UTF8                                      DEE_REGEX_COMPILE_NOUTF8
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_KOS_OPS                0

/* Override libregex types & constants */
#define RE_REGOFF_UNSET ((re_regoff_t)-1)
#define re_code DeeRegexCode

#define RE_EXEC_NOTBOL DEE_RE_EXEC_NOTBOL
#define RE_EXEC_NOTEOL DEE_RE_EXEC_NOTEOL
#define __re_regmatch_t_defined
typedef struct DeeRegexMatch re_regmatch_t;

#define __re_regoff_t_defined
typedef size_t re_regoff_t;
typedef Dee_ssize_t re_sregoff_t;

#define __re_exec_defined
#define re_exec DeeRegexExec

/* Hook functions */
#undef __libc_bzero
#define __libc_bzero bzero
#undef malloc
#undef calloc
#undef realloc
#undef reallocv
#undef free
#undef __libc_malloc
#undef __libc_calloc
#undef __libc_realloc
#undef __libc_free
#define malloc(s)            Dee_TryMalloc(s)
#define calloc(c, n)         Dee_TryCalloc((c) * (n))
#define realloc(p, s)        Dee_TryRealloc(p, s)
#define reallocv(p, c, n)    Dee_TryRealloc(p, (c) * (n))
#define free(p)              Dee_Free(p)
#define __libc_malloc(s)     Dee_TryMalloc(s)
#define __libc_calloc(c, n)  Dee_TryCalloc((c) * (n))
#define __libc_realloc(p, s) Dee_TryRealloc(p, s)
#define __libc_free(p)       Dee_Free(p)

#undef unicode_utf8seqlen
#define unicode_utf8seqlen Dee_utf8_sequence_len
#undef unicode_readutf8
#define unicode_readutf8 Dee_utf8_readchar_u
#undef unicode_readutf8_n
#define unicode_readutf8_n Dee_utf8_readchar
#undef unicode_readutf8_rev
#define unicode_readutf8_rev(ptext) Dee_utf8_readchar_rev(ptext, (char const *)0)
#undef unicode_readutf8_rev_n
#define unicode_readutf8_rev_n Dee_utf8_readchar_rev
#undef unicode_writeutf8
#define unicode_writeutf8 Dee_utf8_writechar
#undef UNICODE_UTF8_MAXLEN
#define UNICODE_UTF8_MAXLEN Dee_UTF8_MAX_MBLEN
#undef unicode_tolower
#define unicode_tolower DeeUni_ToLower
#undef unicode_toupper
#define unicode_toupper DeeUni_ToUpper
#undef unicode_totitle
#define unicode_totitle DeeUni_ToTitle
#undef unicode_islf
#define unicode_islf DeeUni_IsLF
#undef unicode_issymstrt
#define unicode_issymstrt DeeUni_IsSymStrt
#undef unicode_issymcont
#define unicode_issymcont DeeUni_IsSymCont
#undef issymcont
#define issymcont(ch) (isalnum(ch) || (ch) == '_' || (ch) == '$')

#undef __unicode_descriptor
#define __unicode_descriptor(c) DeeUni_Descriptor(c)
#undef __ut_flags
#define __ut_flags ut_flags

/* TODO: Port KOS's new unicode trait database to deemon (replacing the old, less powerful database) */
#undef __UNICODE_ISCNTRL
#define __UNICODE_ISCNTRL Dee_UNICODE_FCNTRL
#undef __UNICODE_ISSPACE
#define __UNICODE_ISSPACE Dee_UNICODE_FSPACE
#undef __UNICODE_ISUPPER
#define __UNICODE_ISUPPER Dee_UNICODE_FUPPER
#undef __UNICODE_ISLOWER
#define __UNICODE_ISLOWER Dee_UNICODE_FLOWER
#undef __UNICODE_ISALPHA
#define __UNICODE_ISALPHA Dee_UNICODE_FALPHA
#undef __UNICODE_ISDIGIT
#define __UNICODE_ISDIGIT Dee_UNICODE_FDECIMAL
#undef __UNICODE_ISXDIGIT
#define __UNICODE_ISXDIGIT 0 /* TODO */
#undef __UNICODE_ISALNUM
#define __UNICODE_ISALNUM (Dee_UNICODE_FALPHA | Dee_UNICODE_FDECIMAL)
#undef __UNICODE_ISPUNCT
#define __UNICODE_ISPUNCT 0 /* TODO */
#undef __UNICODE_ISGRAPH
#define __UNICODE_ISGRAPH 0 /* TODO */
#undef __UNICODE_ISPRINT
#define __UNICODE_ISPRINT Dee_UNICODE_FPRINT
#undef __UNICODE_ISBLANK
#define __UNICODE_ISBLANK 0 /* TODO */
#undef __UNICODE_ISSYMSTRT
#define __UNICODE_ISSYMSTRT Dee_UNICODE_FSYMSTRT
#undef __UNICODE_ISSYMCONT
#define __UNICODE_ISSYMCONT Dee_UNICODE_FSYMCONT
#undef __UNICODE_ISTAB
#define __UNICODE_ISTAB 0 /* TODO */
#undef __UNICODE_ISWHITE
#define __UNICODE_ISWHITE 0 /* TODO */
#undef __UNICODE_ISEMPTY
#define __UNICODE_ISEMPTY 0 /* TODO */
#undef __UNICODE_ISLF
#define __UNICODE_ISLF Dee_UNICODE_FLF
#undef __UNICODE_ISHEX
#define __UNICODE_ISHEX 0 /* TODO */
#undef __UNICODE_ISTITLE
#define __UNICODE_ISTITLE Dee_UNICODE_FTITLE
#undef __UNICODE_ISNUMERIC
#define __UNICODE_ISNUMERIC (Dee_UNICODE_FDIGIT | Dee_UNICODE_FDECIMAL)

#undef char32_t
#define char32_t uint32_t
#undef assert
#define assert Dee_ASSERT
#undef assert_failed
#define assert_failed(message) _DeeAssert_Fail(message, __FILE__, __LINE__)
#undef assertf
#define assertf Dee_ASSERTF
#undef static_assert
#define static_assert(expr, ...) STATIC_ASSERT(expr)
#undef lengthof
#define lengthof COMPILER_LENOF
#undef DEFINE_PUBLIC_ALIAS
#define DEFINE_PUBLIC_ALIAS(new, old) /* nothing -- we only want to use regcomp internally! */

#undef format_printf
#define format_printf DeeFormat_Printf
#undef format_vprintf
#define format_vprintf DeeFormat_VPrintf
#undef PRIx8
#define PRIx8 "I8x"
#undef PRIx16
#define PRIx16 "I16x"
#undef PRIx32
#define PRIx32 "I32x"
#undef PRIx64
#define PRIx64 "I64x"
#undef PRIxSIZ
#define PRIxSIZ "Ix"
#undef PRIu8
#define PRIu8 "I8u"
#undef PRIu16
#define PRIu16 "I16u"
#undef PRIu32
#define PRIu32 "I32u"
#undef PRIu64
#define PRIu64 "I64u"
#undef PRIuSIZ
#define PRIuSIZ "Iu"

/* Emulation of `<sys/bitstring.h>' */
#undef bitstr_t
#define bitstr_t byte_t
#undef _bit_byte
#define _bit_byte(bitno) ((bitno) >> 3)                /*       bitno / 8 */
#undef _bit_mask
#define _bit_mask(bitno) (__UINT8_C(1) << ((bitno)&7)) /* 1 << (bitno % 8) */
#undef bitstr_size
#define bitstr_size(nbits) (((nbits) + 7) >> 3) /* CEILDIV(nbits, 8) */
#undef bit_decl
#define bit_decl(self, nbits) ((self)[bitstr_size(nbits)])
#undef bit_set
#define bit_set(self, bitno) (void)((self)[_bit_byte(bitno)] |= _bit_mask(bitno))
#undef bit_clear
#define bit_clear(self, bitno) (void)((self)[_bit_byte(bitno)] &= ~_bit_mask(bitno))
#undef bit_flip
#define bit_flip(self, bitno) (void)((self)[_bit_byte(bitno)] ^= _bit_mask(bitno))
#undef bit_test
#define bit_test(self, bitno) ((self)[_bit_byte(bitno)] & _bit_mask(bitno))
#undef bit_foreach
#define bit_foreach(bitno, self, nbits)             \
	for ((bitno) = 0; (bitno) < (nbits); ++(bitno)) \
		if (!bit_test(self, bitno))                 \
			;                                       \
		else
#undef bit_setall
#undef bit_clearall
#undef bit_flipall
#define bit_setall(self, nbits)   memset(self, 0xff, bitstr_size(nbits) * sizeof(bitstr_t))
#define bit_clearall(self, nbits) bzero(self, bitstr_size(nbits) * sizeof(bitstr_t))
#define bit_flipall(self, nbits)                                \
	do {                                                        \
		size_t _bfa_i;                                          \
		for (_bfa_i = 0; _bfa_i < bitstr_size(nbits); ++_bfa_i) \
			(self)[_bfa_i] ^= 0xff;                             \
	}	__WHILE0

#define dee_bit_nclear_impl(self, minbyte, maxbyte, minbyte_bitno, maxbyte_bitno) \
	((minbyte >= maxbyte)                                                         \
	 ? (self[maxbyte] &= ((0xff >> (8 - minbyte_bitno)) |                         \
	                      (0xff << (maxbyte_bitno + 1))))                         \
	 : (self[minbyte] &= 0xff >> (8 - minbyte_bitno),                             \
	    bzero(&self[minbyte + 1], maxbyte - (minbyte + 1)),                       \
	    self[maxbyte] &= 0xff << (maxbyte_bitno + 1)))
#define dee_bit_nset_impl(self, minbyte, maxbyte, minbyte_bitno, maxbyte_bitno) \
	((minbyte >= maxbyte)                                                       \
	 ? (self[maxbyte] |= ((0xff << minbyte_bitno) |                             \
	                      (0xff >> (7 - maxbyte_bitno))))                       \
	 : (self[minbyte] |= 0xff << minbyte_bitno,                                 \
	    memset(&self[minbyte + 1], 0xff, maxbyte - (minbyte + 1)),              \
	    self[maxbyte] |= 0xff >> (7 - maxbyte_bitno)))
#define __bit_nclear(self, minbitno, maxbitno)               \
	__register size_t minbyte = (size_t)_bit_byte(minbitno); \
	__register size_t maxbyte = (size_t)_bit_byte(maxbitno); \
	dee_bit_nclear_impl(self, minbyte, maxbyte, (minbitno & 7), (maxbitno & 7))
#define __bit_nset(self, minbitno, maxbitno)                 \
	__register size_t minbyte = (size_t)_bit_byte(minbitno); \
	__register size_t maxbyte = (size_t)_bit_byte(maxbitno); \
	dee_bit_nset_impl(self, minbyte, maxbyte, (minbitno & 7), (maxbitno & 7))
#undef bit_nclear
#define bit_nclear dee_bit_nclear
LOCAL NONNULL((1)) void
(dee_bit_nclear)(bitstr_t *__restrict self, int minbitno, int maxbitno) {
	__bit_nclear(self, minbitno, maxbitno);
}

#undef bit_nset
#define bit_nset dee_bit_nset
LOCAL NONNULL((1)) void
(dee_bit_nset)(bitstr_t *__restrict self, int minbitno, int maxbitno) {
	__bit_nset(self, minbitno, maxbitno);
}

#undef bit_noneset
#define bit_noneset(self, nbits) (!bit_anyset(self, nbits))

#undef bit_anyset
#define bit_anyset dee_bit_anyset
LOCAL NONNULL((1)) bool
(dee_bit_anyset)(bitstr_t *__restrict self, unsigned int nbits) {
	unsigned int i;
	for (i = 0; i < (nbits >> 3); ++i) {
		if (self[i] != 0)
			return 1;
	}
	if (nbits & 7) {
		if (self[i] & ((1 << (nbits & 7)) - 1))
			return 1;
	}
	return 0;
}

#undef bit_allset
#define bit_allset dee_bit_allset
LOCAL NONNULL((1)) bool
(dee_bit_allset)(bitstr_t *__restrict self, unsigned int nbits) {
	unsigned int i;
	for (i = 0; i < (nbits >> 3); ++i) {
		if (self[i] != 0xff)
			return 0;
	}
	if (nbits & 7) {
		bitstr_t mask = ((1 << (nbits & 7)) - 1);
		if ((self[i] & mask) != mask)
			return 0;
	}
	return 1;
}

#undef bit_nanyset
#define bit_nanyset dee_bit_nanyset
LOCAL NONNULL((1)) bool
(dee_bit_nanyset)(bitstr_t *__restrict self, unsigned int minbitno, unsigned int maxbitno) {
	__register size_t minbyte = (size_t)_bit_byte(minbitno);
	__register size_t maxbyte = (size_t)_bit_byte(maxbitno);
	if (minbyte >= maxbyte) {
		if (self[maxbyte] & ~((0xff >> (8 - (minbitno & 7))) |
		                      (0xff << ((maxbitno & 7) + 1))))
			return 1;
	} else {
		__register size_t i;
		if (self[minbyte] & ~(0xff >> (8 - (minbitno & 7))))
			return 1;
		for (i = minbyte + 1; i < maxbyte; ++i) {
			if (self[i] != 0)
				return 1;
		}
		if (self[maxbyte] & ~(0xff << ((maxbitno & 7) + 1)))
			return 1;
	}
	return 0;
}

#undef bit_popcount
#define bit_popcount dee_bit_popcount
LOCAL NONNULL((1)) unsigned int
(dee_bit_popcount)(bitstr_t *__restrict self, unsigned int nbits) {
	unsigned int result = 0;
	unsigned int i;
	for (i = 0; i < (nbits >> 3); ++i) {
		result += __hybrid_popcount8(self[i]);
	}
	if (nbits & 7) {
		bitstr_t mask = ((1 << (nbits & 7)) - 1);
		result += __hybrid_popcount8(self[i] & mask);
	}
	return result;
}

/* Count-leading-zeroes (undefined when `self' doesn't contain any set bits) */
#undef bit_clz
#define bit_clz dee_bit_clz
LOCAL NONNULL((1)) unsigned int
(dee_bit_clz)(bitstr_t *__restrict self) {
	bitstr_t word;
	unsigned int result = 0;
	for (; *self == 0; ++self)
		result += 8;
	word = *self;
	while (!(word & 1)) {
		++result;
		word >>= 1;
	}
	return result;
}

/* Count-trailing-zeroes (undefined when `self' doesn't contain any set bits) */
#undef bit_ctz
#define bit_ctz dee_bit_ctz
LOCAL NONNULL((1)) unsigned int
(dee_bit_ctz)(bitstr_t *__restrict self, unsigned int nbits) {
	bitstr_t word;
	unsigned int result  = 0;
	unsigned int byteoff = nbits >> 3;
	if (nbits & 7) {
		word = self[byteoff];
		word <<= 8 - (nbits & 7);
		if (word) {
			while (!(word & 0x80)) {
				word <<= 1;
				++result;
			}
			return result;
		}
		result = nbits & 7;
	}
	if (byteoff) {
		--byteoff;
		for (; self[byteoff] == 0; --byteoff)
			result += 8;
		word = self[byteoff];
		while (!(word & 0x80)) {
			++result;
			word <<= 1;
		}
	}
	return result;
}



#undef __libc_hex2int
#define __libc_hex2int(ch, p_result)                           \
	(((ch) >= '0' && (ch) <= '9')                              \
	 ? (*(p_result) = (unsigned char)((ch) - '0'), 1)          \
	 : ((ch) >= 'A' && (ch) <= 'F')                            \
	   ? (*(p_result) = (unsigned char)(10 + (ch) - 'A'), 1)   \
	   : ((ch) >= 'a' && (ch) <= 'f')                          \
	     ? (*(p_result) = (unsigned char)(10 + (ch) - 'a'), 1) \
	     : 0)

#undef strcmpz /* TODO: Support for when the system defines this function! */
#define strcmpz dee_strcmpz
PRIVATE int
dee_strcmpz(char const *lhs, char const *rhs, size_t rhs_len) {
	char c1, c2;
	do {
		c1 = *lhs++;
		if (!rhs_len--) {
			/* Once  RHS  reaches  the end  of  the string,
			 * compare the last character of LHS with `NUL' */
			return (int)((unsigned char)c1 - '\0');
		}
		c2 = *rhs++;
		if unlikely(c1 != c2)
			return (int)((unsigned char)c1 - (unsigned char)c2);
	} while (c1);
	return 0;
}




DECL_END

/* Define everything statically */
#undef INTDEF
#define INTDEF PRIVATE
#undef INTERN
#define INTERN PRIVATE

/* Pull in libregex's public headers */
/* clang-format off */
#include "../../../libregex/include/regcomp.h" /* 1 */
#include "../../../libregex/include/regexec.h" /* 2 */
/* clang-format on */

/* Pull in libregex's source files. */
/* clang-format off */
#include "../../../libregex/regexec.c" /* 1 */
#include "../../../libregex/regpeep.c" /* 2 */
#include "../../../libregex/regfast.c" /* 3 */
#include "../../../libregex/regcomp.c" /* 4 */
/* clang-format on */

/* Restore normal binding macros */
#undef INTDEF
#define INTDEF __INTDEF
#undef INTERN
#define INTERN __INTERN

DECL_BEGIN

static_assert(DEE_RE_STATUS_NOMATCH == (-RE_NOMATCH));

PRIVATE ATTR_CONST ATTR_RETNONNULL WUNUSED
char const *DCALL re_strerror(re_errno_t error) {
	char const *result;
	switch (error) {
	case RE_BADPAT: /*  */ result = "Invalid regex pattern"; break;
	case RE_ECOLLATE: /**/ result = "Invalid collation character"; break;
	case RE_ECTYPE: /*  */ result = "Invalid character class name"; break;
	case RE_EESCAPE: /* */ result = "Trailing backslash"; break;
	case RE_ESUBREG: /* */ result = "Invalid back reference"; break;
	case RE_EBRACK: /*  */ result = "Unmatched [, [^, [:, [., or [="; break;
	case RE_EPAREN: /*  */ result = "Unmatched ("; break;
	case RE_EBRACE: /*  */ result = "Unmatched {"; break;
	case RE_BADBR: /*   */ result = "Invalid content of {...}"; break;
	case RE_ERANGE: /*  */ result = "Set-range start is greater than its end"; break;
	case RE_ESPACE: /*  */ result = "Out of memory"; break;
	case RE_BADRPT: /*  */ result = "Nothing precedes +, *, ?, or {"; break;
	case RE_EEND: /*    */ result = "Unexpected end of pattern"; break;
	case RE_ESIZE: /*   */ result = "Regular expression violates a hard upper limit"; break;
	case RE_ERPAREN: /* */ result = "Unmatched )"; break;
	case RE_EILLSEQ: /* */ result = "Illegal unicode character"; break;
	case RE_EILLSET: /* */ result = "Cannot combine raw bytes with unicode characters in charsets"; break;
	default: result = "Unknown regex error"; break;
	}
	return result;
}

PRIVATE int DCALL re_handle_error(re_errno_t error) {
	DeeTypeObject *error_type;
	char const *message;
	switch (error) {
	case RE_ESPACE:
		return Dee_CollectMemory(1) ? 0 : -1;
	case RE_ESIZE:    /* Regex pattern is too complex */
		error_type = &DeeError_ValueError;
		break;
	case RE_EILLSEQ:
	case RE_EILLSET:
		error_type = &DeeError_UnicodeError;
		break;
	case RE_ECOLLATE:
	case RE_ECTYPE:
		error_type = &DeeError_SymbolError;
		break;
	default:
		error_type = &DeeError_SyntaxError;
		break;
	}
	message = re_strerror(error);
	return DeeError_Throwf(error_type, "Regex error: %s", message);
}



/************************************************************************/
/* Regex execute                                                        */
/************************************************************************/

/* Perform a regex match
 * @return: >= 0: The # of matched bytes starting at `exec->rx_startoff'
 * @return: DEE_RE_STATUS_NOMATCH: Nothing was matched
 * @return: DEE_RE_STATUS_ERROR:   An error occurred */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_Match(struct DeeRegexExec const *__restrict exec) {
	Dee_ssize_t result;
again:
	result = libre_exec_match(exec);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}

/* Similar to `DeeRegex_Match', try to match a pattern against the given input buffer. Do this
 * with increasing offsets for the first `search_range' bytes, meaning at most `search_range'
 * regex matches will be performed.
 * @param: search_range: One plus the max starting  byte offset (from `exec->rx_startoff')  to
 *                       check. Too great values for `search_range' are automatically clamped.
 * @param: p_match_size: When non-NULL, set to the # of bytes that were actually matched.
 *                       This would have  been the return  value of  `re_exec_match(3R)'.
 * @return: >= 0:        The offset where the matched area starts (in `[exec->rx_startoff, exec->rx_startoff + search_range)').
 * @return: DEE_RE_STATUS_NOMATCH: Nothing was matched
 * @return: DEE_RE_STATUS_ERROR:   An error occurred */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_Search(struct DeeRegexExec const *__restrict exec,
                size_t search_range, size_t *p_match_size) {
	Dee_ssize_t result;
again:
	result = libre_exec_search(exec, search_range, p_match_size);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}

/* Same as `DeeRegex_Search', but perform searching with starting
 * offsets in `[exec->rx_endoff - search_range, exec->rx_endoff)'
 * Too great values for `search_range' are automatically clamped.
 * The return value will thus be the greatest byte-offset where
 * the given pattern matches that is still within that range. */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t
DeeRegex_RSearch(struct DeeRegexExec const *__restrict exec,
                 size_t search_range, size_t *p_match_size) {
	Dee_ssize_t result;
again:
	result = libre_exec_rsearch(exec, search_range, p_match_size);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}




/************************************************************************/
/* Regex compile                                                        */
/************************************************************************/

/* Compile the regex pattern of a given string `self' */
PRIVATE WUNUSED NONNULL((1)) struct DeeRegexCode *DCALL
re_compile(DeeObject *__restrict self, unsigned int compile_flags) {
	re_errno_t comp_error;
	struct re_compiler comp;
	char *utf8;
again:
	utf8 = DeeString_AsUtf8(self);
	if unlikely(!utf8)
		goto err;

	/* Put together the regex compiler. */
	re_compiler_init(&comp,
	                 utf8,
	                 utf8 + WSTR_LENGTH(utf8),
	                 compile_flags);

	/* Initiate the compile. */
	comp_error = libre_compiler_compile(&comp);
	if unlikely(comp_error != RE_NOERROR)
		goto err_comp;

	/* Pack together the generated code. */
	return re_compiler_pack(&comp);
err_comp:
	re_compiler_fini(&comp);
	if (re_handle_error(comp_error) == 0)
		goto again;
err:
	return NULL;
}


/* Destroy the regex cache associated with `self'.
 * Called from `DeeString_Type.tp_fini' when `STRING_UTF_FREGEX' was set. */
INTERN NONNULL((1)) void DCALL
DeeString_DestroyRegex(DeeStringObject *__restrict self) {
	(void)self;
	COMPILER_IMPURE();
	/* TODO */
}


/* Lazily compile `self' as a deemon regex pattern.
 * Regex patterns for strings are compiled once, and cached thereafter,
 * before being destroyed at the same time as the corresponding string.
 * @param: compile_flags: Set of `DEE_REGEX_COMPILE_*'
 * @return: * :   The compiled regex pattern.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) struct DeeRegexCode *DCALL
DeeString_GetRegex(DeeObject *__restrict self, unsigned int compile_flags) {
	struct DeeRegexCode *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);

	/* Lookup regex in cache */
	/* TODO */

	/* Not found int cache -> create a new regex object. */
	result = re_compile(self, compile_flags);
	if unlikely(!result)
		goto done;

	/* Store produced regex object in cache. */
	/* TODO */

done:
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C */
