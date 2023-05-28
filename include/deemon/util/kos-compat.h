/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_KOS_COMPAT_H
#define GUARD_DEEMON_UTIL_KOS_COMPAT_H 1

#include "../api.h"
/**/

#include "../format.h"
#include "../string.h"
#include "../stringutils.h"
#include "../system-features.h"
/**/

#include <stdint.h>
/**/

#include <hybrid/__bit.h>
#include <hybrid/__alloca.h>
#include <hybrid/typecore.h>
/**/

/* Compatibility header for porting KOS system libraries for use with deemon */

DECL_BEGIN

/* Base system config */
#undef __NO_FPU
#ifndef CONFIG_HAVE_FPU
#define __NO_FPU
#endif /* CONFIG_HAVE_FPU */

/* Supplement non-standard <string.h> functions */
#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

#ifndef CONFIG_HAVE_memmem
#define CONFIG_HAVE_memmem
#undef memmem
#define memmem  dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */

#ifndef CONFIG_HAVE_memrmem
#define CONFIG_HAVE_memrmem
#undef memrmem
#define memrmem dee_memrmem
DeeSystem_DEFINE_memrmem(dee_memrmem)
#endif /* !CONFIG_HAVE_memrmem */

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

#ifndef CONFIG_HAVE_mempsetw
#define CONFIG_HAVE_mempsetw
#undef mempsetw
#define mempsetw dee_mempsetw
DeeSystem_DEFINE_mempsetw(dee_mempsetw)
#endif /* !CONFIG_HAVE_mempsetw */

#ifndef CONFIG_HAVE_mempsetl
#define CONFIG_HAVE_mempsetl
#undef mempsetl
#define mempsetl dee_mempsetl
DeeSystem_DEFINE_mempsetl(dee_mempsetl)
#endif /* !CONFIG_HAVE_mempsetl */

#ifndef CONFIG_HAVE_memcmpw
#define CONFIG_HAVE_memcmpw
#undef memcmpw
#define memcmpw dee_memcmpw
DeeSystem_DEFINE_memcmpw(dee_memcmpw)
#endif /* !CONFIG_HAVE_memcmpw */

#ifndef CONFIG_HAVE_memcmpl
#define CONFIG_HAVE_memcmpl
#undef memcmpl
#define memcmpl dee_memcmpl
DeeSystem_DEFINE_memcmpl(dee_memcmpl)
#endif /* !CONFIG_HAVE_memcmpl */

#ifndef CONFIG_HAVE_memchrw
#define CONFIG_HAVE_memchrw
#undef memchrw
#define memchrw dee_memchrw
DeeSystem_DEFINE_memchrw(dee_memchrw)
#endif /* !CONFIG_HAVE_memchrw */

#ifndef CONFIG_HAVE_memchrl
#define CONFIG_HAVE_memchrl
#undef memchrl
#define memchrl dee_memchrl
DeeSystem_DEFINE_memchrl(dee_memchrl)
#endif /* !CONFIG_HAVE_memchrl */

#ifndef CONFIG_HAVE_memrchrw
#define CONFIG_HAVE_memrchrw
#undef memrchrw
#define memrchrw dee_memrchrw
DeeSystem_DEFINE_memrchrw(dee_memrchrw)
#endif /* !CONFIG_HAVE_memrchrw */

#ifndef CONFIG_HAVE_memrchrl
#define CONFIG_HAVE_memrchrl
#undef memrchrl
#define memrchrl dee_memrchrl
DeeSystem_DEFINE_memrchrl(dee_memrchrl)
#endif /* !CONFIG_HAVE_memrchrl */

#ifndef CONFIG_HAVE_memxend
#define CONFIG_HAVE_memxend
#undef memxend
#define memxend dee_memxend
DeeSystem_DEFINE_memxend(dee_memxend)
#endif /* !CONFIG_HAVE_memxend */

#ifndef CONFIG_HAVE_strcmpz
#define CONFIG_HAVE_strcmpz
#undef strcmpz
#define strcmpz dee_strcmpz
DeeSystem_DEFINE_strcmpz(dee_strcmpz)
#endif /* !CONFIG_HAVE_strcmpz */

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#undef memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */

#undef memrchrb
#define memrchrb (uint8_t *)memrchr
#undef memxendb
#define memxendb (uint8_t *)memxend

#undef memsetc
#define memsetc(dst, word, word_count, word_size)                                    \
	__builtin_choose_expr((word_size) == 1,                                          \
	                      (void *)memsetb(dst, (__UINT8_TYPE__)(word), word_count),  \
	__builtin_choose_expr((word_size) == 2,                                          \
	                      (void *)memsetw(dst, (__UINT16_TYPE__)(word), word_count), \
	__builtin_choose_expr((word_size) == 4,                                          \
	                      (void *)memsetl(dst, (__UINT32_TYPE__)(word), word_count), \
	                      (void *)memsetq(dst, (__UINT64_TYPE__)(word), word_count))))





/* Configure system types */
#ifndef alloca
#define alloca __hybrid_alloca
#endif /* !alloca */
#undef pformatprinter
#undef __pformatprinter
#define pformatprinter   Dee_formatprinter_t
#define __pformatprinter Dee_formatprinter_t
#ifndef DPRINTER_CC_IS_FORMATPRINTER_CC
#undef FORMATPRINTER_CC
#define FORMATPRINTER_CC DPRINTER_CC
#endif /* !DPRINTER_CC_IS_FORMATPRINTER_CC */
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

/* Hook heap functions */
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
#define calloc(c, n)         Dee_TryCallocc(c, n)
#define realloc(p, s)        Dee_TryRealloc(p, s)
#define reallocv(p, c, n)    Dee_TryReallocc(p, c, n)
#define free(p)              Dee_Free(p)
#define __libc_malloc(s)     Dee_TryMalloc(s)
#define __libc_calloc(c, n)  Dee_TryCallocc(c, n)
#define __libc_realloc(p, s) Dee_TryRealloc(p, s)
#define __libc_free(p)       Dee_Free(p)

/* Hook extended ctype functions */
#undef _itoa_digits
#define _itoa_digits _DeeAscii_Itoa
#undef __LOCAL_itoa_digits
#define __LOCAL_itoa_digits _DeeAscii_Itoa
#undef itoa_digit
#define itoa_digit DeeAscii_ItoaDigit
#undef __LOCAL_itoa_digit
#define __LOCAL_itoa_digit DeeAscii_ItoaDigit
#undef itoa_decimal
#define itoa_decimal(digit) (char)('0' + (digit))
#undef __LOCAL_itoa_decimal
#define __LOCAL_itoa_decimal(digit) (char)('0' + (digit))
#undef _itoa_lower_digits
#define _itoa_lower_digits DeeAscii_ItoaDigits(false)
#undef __LOCAL_itoa_lower_digits
#define __LOCAL_itoa_lower_digits DeeAscii_ItoaDigits(false)
#undef _itoa_upper_digits
#define _itoa_upper_digits DeeAscii_ItoaDigits(true)
#undef __LOCAL_itoa_upper_digits
#define __LOCAL_itoa_upper_digits DeeAscii_ItoaDigits(true)

#undef issymstrt
#define issymstrt(ch) (isalpha(ch) || (ch) == '_' || (ch) == '$')
#undef issymcont
#define issymcont(ch) (isalnum(ch) || (ch) == '_' || (ch) == '$')

#ifndef CONFIG_HAVE_UNICODE_H
#undef unicode_utf8seqlen
#define unicode_utf8seqlen Dee_utf8_sequence_len
#undef unicode_readutf8
#define unicode_readutf8 Dee_utf8_readchar_u
#undef unicode_readutf8_n
#define unicode_readutf8_n Dee_utf8_readchar
#undef unicode_readutf8_rev
#define unicode_readutf8_rev(p_text) Dee_utf8_readchar_rev(p_text, (char const *)0)
#undef unicode_readutf8_rev_n
#define unicode_readutf8_rev_n Dee_utf8_readchar_rev
#undef unicode_writeutf8
#define unicode_writeutf8 Dee_utf8_writechar
/* TODO: unicode_readutf16le_n */
/* TODO: unicode_readutf16be_n */
/* TODO: unicode_readutf32le_n */
/* TODO: unicode_readutf32be_n */
/* TODO: unicode_readutf16le_rev_n */
/* TODO: unicode_readutf32le_rev_n */
/* TODO: unicode_readutf16be_rev_n */
/* TODO: unicode_readutf32be_rev_n */
#undef UNICODE_UTF8_CURLEN
#define UNICODE_UTF8_CURLEN Dee_UTF8_CUR_MBLEN
#undef unicode_tolower
#define unicode_tolower DeeUni_ToLower
#undef unicode_toupper
#define unicode_toupper DeeUni_ToUpper
#undef unicode_totitle
#define unicode_totitle DeeUni_ToTitle
#undef unicode_islf
#define unicode_islf DeeUni_IsLF
#undef unicode_isspace
#define unicode_isspace DeeUni_IsSpace
#undef unicode_isdigit
#define unicode_isdigit DeeUni_IsDigit
#undef unicode_isxdigit
#define unicode_isxdigit DeeUni_IsXDigit
#undef unicode_ishex
#define unicode_ishex DeeUni_IsHex
#undef unicode_getnumeric
#define unicode_getnumeric DeeUni_GetNumeric8
#undef unicode_getnumeric64
#define unicode_getnumeric64 DeeUni_GetNumeric64
#undef unicode_getnumericdbl
#define unicode_getnumericdbl DeeUni_GetNumericD
#undef unicode_issymstrt
#define unicode_issymstrt DeeUni_IsSymStrt
#undef unicode_issymcont
#define unicode_issymcont DeeUni_IsSymCont
#undef unicode_asdigit
#define unicode_asdigit DeeUni_AsDigit
#undef __unicode_descriptor
#define __unicode_descriptor(c) DeeUni_Descriptor(c)
#undef __ut_flags
#define __ut_flags ut_flags
#undef __UNICODE_ISCNTRL
#define __UNICODE_ISCNTRL Dee_UNICODE_ISCNTRL
#undef __UNICODE_ISSPACE
#define __UNICODE_ISSPACE Dee_UNICODE_ISSPACE
#undef __UNICODE_ISUPPER
#define __UNICODE_ISUPPER Dee_UNICODE_ISUPPER
#undef __UNICODE_ISLOWER
#define __UNICODE_ISLOWER Dee_UNICODE_ISLOWER
#undef __UNICODE_ISALPHA
#define __UNICODE_ISALPHA Dee_UNICODE_ISALPHA
#undef __UNICODE_ISDIGIT
#define __UNICODE_ISDIGIT Dee_UNICODE_ISDIGIT
#undef __UNICODE_ISXDIGIT
#define __UNICODE_ISXDIGIT Dee_UNICODE_ISXDIGIT
#undef __UNICODE_ISALNUM
#define __UNICODE_ISALNUM Dee_UNICODE_ISALNUM
#undef __UNICODE_ISPUNCT
#define __UNICODE_ISPUNCT Dee_UNICODE_ISPUNCT
#undef __UNICODE_ISGRAPH
#define __UNICODE_ISGRAPH Dee_UNICODE_ISGRAPH
#undef __UNICODE_ISPRINT
#define __UNICODE_ISPRINT Dee_UNICODE_ISPRINT
#undef __UNICODE_ISBLANK
#define __UNICODE_ISBLANK Dee_UNICODE_ISBLANK
#undef __UNICODE_ISSYMSTRT
#define __UNICODE_ISSYMSTRT Dee_UNICODE_ISSYMSTRT
#undef __UNICODE_ISSYMCONT
#define __UNICODE_ISSYMCONT Dee_UNICODE_ISSYMCONT
#undef __UNICODE_ISTAB
#define __UNICODE_ISTAB Dee_UNICODE_ISTAB
#undef __UNICODE_ISWHITE
#define __UNICODE_ISWHITE Dee_UNICODE_ISWHITE
#undef __UNICODE_ISEMPTY
#define __UNICODE_ISEMPTY Dee_UNICODE_ISEMPTY
#undef __UNICODE_ISLF
#define __UNICODE_ISLF Dee_UNICODE_ISLF
#undef __UNICODE_ISHEX
#define __UNICODE_ISHEX Dee_UNICODE_ISHEX
#undef __UNICODE_ISTITLE
#define __UNICODE_ISTITLE Dee_UNICODE_ISTITLE
#undef __UNICODE_ISNUMERIC
#define __UNICODE_ISNUMERIC Dee_UNICODE_ISNUMERIC


/* Define missing unicode parsing functions */

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf16_n)(char16_t const **__restrict ptext,
                                           char16_t const *text_end) {
	char32_t result;
	char16_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = (char32_t)(uint16_t)*text++;
	if (result >= 0xd800 &&
	    result <= 0xdbff &&
	    text < text_end) {
		result -= 0xd800;
		result <<= 10;
		result += 0x10000 - 0xdc00;
		result += *text++; /* low surrogate */
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf16_swap_n)(char16_t const **__restrict ptext,
                                                char16_t const *text_end) {
	char32_t result;
	char16_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = (char32_t)__hybrid_bswap16((uint16_t)*text);
	++text;
	if (result >= 0xd800 &&
	    result <= 0xdbff &&
	    text < text_end) {
		result -= 0xd800;
		result <<= 10;
		result += 0x10000 - 0xdc00;
		result += __hybrid_bswap16(*text); /* low surrogate */
		++text;
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf16_rev_n)(char16_t const **__restrict ptext,
                                               char16_t const *text_start) {
	char32_t result;
	char16_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	result = (char32_t)(uint16_t)*--text;
	if (result >= 0xdc00 &&
	    result <= 0xdfff && likely(text > text_start)) {
		char32_t high = *--text;
		high   -= 0xd800;
		high   <<= 10;
		high   += 0x10000 - 0xdc00;
		result += high;
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_IN(2) ATTR_INOUT(1) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf16_swap_rev_n)(char16_t const **__restrict ptext,
                                                    char16_t const *text_start) {
	char32_t result;
	char16_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	--text;
	result = (char32_t)__hybrid_bswap16((uint16_t)*text);
	if (result >= 0xdc00 &&
	    result <= 0xdfff && likely(text > text_start)) {
		char32_t high = (--text, __hybrid_bswap16(*text));
		high   -= 0xd800;
		high   <<= 10;
		high   += 0x10000 - 0xdc00;
		result += high;
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf32_n)(/*utf-32*/ char32_t const **__restrict ptext,
                                           char32_t const *text_end) {
	char32_t result;
	char32_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = *text++;
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf32_rev_n)(/*utf-32*/ char32_t const **__restrict ptext,
                                               char32_t const *text_start) {
	char32_t result;
	char32_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	result = *--text;
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf32_swap_n)(/*utf-32*/ char32_t const **__restrict ptext,
                                                char32_t const *text_end) {
	char32_t result;
	char32_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = *text++;
	*ptext = text;
	return __hybrid_bswap32(result);
}

LOCAL ATTR_INOUT(1) NONNULL((2)) char32_t
NOTHROW_NCX(DCALL dee_unicode_readutf32_swap_rev_n)(/*utf-32*/ char32_t const **__restrict ptext,
                                                    char32_t const *text_start) {
	char32_t result;
	char32_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	result = *--text;
	*ptext = text;
	return __hybrid_bswap32(result);
}



#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define dee_unicode_readutf16le_n     dee_unicode_readutf16_n
#define dee_unicode_readutf32le_n     dee_unicode_readutf32_n
#define dee_unicode_readutf16be_n     dee_unicode_readutf16_swap_n
#define dee_unicode_readutf32be_n     dee_unicode_readutf32_swap_n
#define dee_unicode_readutf16le_rev_n dee_unicode_readutf16_rev_n
#define dee_unicode_readutf32le_rev_n dee_unicode_readutf32_rev_n
#define dee_unicode_readutf16be_rev_n dee_unicode_readutf16_swap_rev_n
#define dee_unicode_readutf32be_rev_n dee_unicode_readutf32_swap_rev_n
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define dee_unicode_readutf16be_n     dee_unicode_readutf16_n
#define dee_unicode_readutf32be_n     dee_unicode_readutf32_n
#define dee_unicode_readutf16le_n     dee_unicode_readutf16_swap_n
#define dee_unicode_readutf32le_n     dee_unicode_readutf32_swap_n
#define dee_unicode_readutf16be_rev_n dee_unicode_readutf16_rev_n
#define dee_unicode_readutf32be_rev_n dee_unicode_readutf32_rev_n
#define dee_unicode_readutf16le_rev_n dee_unicode_readutf16_swap_rev_n
#define dee_unicode_readutf32le_rev_n dee_unicode_readutf32_swap_rev_n
#endif /* __BYTE_ORDER__ == ... */

#undef unicode_readutf16_n
#undef unicode_readutf32_n
#undef unicode_readutf16_swap_n
#undef unicode_readutf32_swap_n
#undef unicode_readutf16_rev_n
#undef unicode_readutf32_rev_n
#undef unicode_readutf16_swap_rev_n
#undef unicode_readutf32_swap_rev_n
#undef unicode_readutf16le_n
#undef unicode_readutf32le_n
#undef unicode_readutf16be_n
#undef unicode_readutf32be_n
#undef unicode_readutf16le_rev_n
#undef unicode_readutf32le_rev_n
#undef unicode_readutf16be_rev_n
#undef unicode_readutf32be_rev_n
#define unicode_readutf16_n          dee_unicode_readutf16_n
#define unicode_readutf32_n          dee_unicode_readutf32_n
#define unicode_readutf16_swap_n     dee_unicode_readutf16_swap_n
#define unicode_readutf32_swap_n     dee_unicode_readutf32_swap_n
#define unicode_readutf16_rev_n      dee_unicode_readutf16_rev_n
#define unicode_readutf32_rev_n      dee_unicode_readutf32_rev_n
#define unicode_readutf16_swap_rev_n dee_unicode_readutf16_swap_rev_n
#define unicode_readutf32_swap_rev_n dee_unicode_readutf32_swap_rev_n
#define unicode_readutf16le_n        dee_unicode_readutf16_n
#define unicode_readutf32le_n        dee_unicode_readutf32_n
#define unicode_readutf16be_n        dee_unicode_readutf16_swap_n
#define unicode_readutf32be_n        dee_unicode_readutf32_swap_n
#define unicode_readutf16le_rev_n    dee_unicode_readutf16_rev_n
#define unicode_readutf32le_rev_n    dee_unicode_readutf32_rev_n
#define unicode_readutf16be_rev_n    dee_unicode_readutf16_swap_rev_n
#define unicode_readutf32be_rev_n    dee_unicode_readutf32_swap_rev_n
#endif /* !CONFIG_HAVE_UNICODE_H */

#undef __libc_hex2int
#define __libc_hex2int(ch, p_result) \
	DeeUni_AsDigit(ch, 16, p_result)

#ifndef __native_char16_t_defined
#undef char16_t
#define char16_t uint16_t
#undef char32_t
#define char32_t uint32_t
#endif /* !__native_char16_t_defined */
#undef assert
#define assert ASSERT
#undef assert_failed
#define assert_failed(message) _DeeAssert_XFail(message, __FILE__, __LINE__)
#undef assertf
#define assertf ASSERTF
#undef static_assert
#define static_assert(expr, ...) STATIC_ASSERT(expr)
#undef lengthof
#define lengthof COMPILER_LENOF

#undef format_printf
#define format_printf DeeFormat_Printf
#undef format_vprintf
#define format_vprintf DeeFormat_VPrintf
#undef PRIx8
#define PRIx8 PRFx8
#undef PRIx16
#define PRIx16 PRFx16
#undef PRIx32
#define PRIx32 PRFx32
#undef PRIx64
#define PRIx64 PRFx64
#undef PRIu8
#define PRIu8 PRFu8
#undef PRIu16
#define PRIu16 PRFu16
#undef PRIu32
#define PRIu32 PRFu32
#undef PRIu64
#define PRIu64 PRFu64
#undef PRId8
#define PRId8 PRFd8
#undef PRId16
#define PRId16 PRFd16
#undef PRId32
#define PRId32 PRFd32
#undef PRId64
#define PRId64 PRFd64
#undef PRIdSIZ
#define PRIdSIZ PRFdSIZ
#undef PRIuSIZ
#define PRIuSIZ PRFuSIZ
#undef PRIxSIZ
#define PRIxSIZ PRFxSIZ
#undef PRIdPTR
#define PRIdPTR PRFdPTR
#undef PRIuPTR
#define PRIuPTR PRFuPTR
#undef PRIxPTR
#define PRIxPTR PRFxPTR

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



DECL_END


#endif /* !GUARD_DEEMON_UTIL_KOS_COMPAT_H */