/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_KOS_COMPAT_H
#define GUARD_DEEMON_UTIL_KOS_COMPAT_H 1

#include "../api.h"
/**/

#ifndef DEE_SOURCE
#error "This header should only be used when `DEE_SOURCE' is also enabled"
#endif /* !DEE_SOURCE */

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

#ifndef CONFIG_HAVE_memsetq
#define CONFIG_HAVE_memsetq
#undef memsetq
#define memsetq dee_memsetq
DeeSystem_DEFINE_memsetq(dee_memsetq)
#endif /* !CONFIG_HAVE_memsetq */

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

#ifndef CONFIG_HAVE_mempsetq
#define CONFIG_HAVE_mempsetq
#undef mempsetq
#define mempsetq dee_mempsetq
DeeSystem_DEFINE_mempsetq(dee_mempsetq)
#endif /* !CONFIG_HAVE_mempsetq */

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
#undef mallocv
#undef calloc
#undef realloc
#undef reallocv
#undef free
#undef __libc_malloc
#undef __libc_mallocv
#undef __libc_calloc
#undef __libc_realloc
#undef __libc_free
#define malloc(s)            Dee_TryMalloc(s)
#define mallocv(c, n)        Dee_TryMallocc(c, n)
#define calloc(c, n)         Dee_TryCallocc(c, n)
#define realloc(p, s)        Dee_TryRealloc(p, s)
#define reallocv(p, c, n)    Dee_TryReallocc(p, c, n)
#define free(p)              Dee_Free(p)
#define __libc_malloc(s)     Dee_TryMalloc(s)
#define __libc_mallocv(c, n) Dee_TryMallocc(c, n)
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

#undef char16_t
#define char16_t __UINT16_TYPE__
#undef char32_t
#define char32_t __UINT32_TYPE__

#ifndef CONFIG_HAVE_UNICODE_H
#undef UNICODE_FOLDED_MAX
#define UNICODE_FOLDED_MAX Dee_UNICODE_FOLDED_MAX
#undef unicode_fold
#define unicode_fold(ch, buf) ((buf) + DeeUni_ToFolded((uint32_t)(ch), (uint32_t *)(buf)))
#undef __unicode_asdigit
#define __unicode_asdigit DeeUni_AsDigitVal
#undef __unicode_descriptor_digit
#define __unicode_descriptor_digit DeeUni_GetNumericIdx8
#undef __unicode_descriptor_digit64
#define __unicode_descriptor_digit64 DeeUni_GetNumericIdx64
#undef __unicode_descriptor_digitd
#define __unicode_descriptor_digitd DeeUni_GetNumericIdxD
#undef unicode_tolower
#define unicode_tolower DeeUni_ToLower
#undef unicode_toupper
#define unicode_toupper DeeUni_ToUpper
#undef unicode_totitle
#define unicode_totitle DeeUni_ToTitle
#undef unicode_iscntrl
#define unicode_iscntrl DeeUni_IsCntrl
#undef unicode_istab
#define unicode_istab DeeUni_IsTab
#undef unicode_iswhite
#define unicode_iswhite DeeUni_IsWhite
#undef unicode_isempty
#define unicode_isempty DeeUni_IsEmpty
#undef unicode_islf
#define unicode_islf DeeUni_IsLF
#undef unicode_isspace
#define unicode_isspace DeeUni_IsSpace
#undef unicode_islower
#define unicode_islower DeeUni_IsLower
#undef unicode_isupper
#define unicode_isupper DeeUni_IsUpper
#undef unicode_isalpha
#define unicode_isalpha DeeUni_IsAlpha
#undef unicode_isdigit
#define unicode_isdigit DeeUni_IsDigit
#undef unicode_ishex
#define unicode_ishex DeeUni_IsHex
#undef unicode_isxdigit
#define unicode_isxdigit DeeUni_IsXDigit
#undef unicode_isalnum
#define unicode_isalnum DeeUni_IsAlnum
#undef unicode_ispunct
#define unicode_ispunct DeeUni_IsPunct
#undef unicode_isgraph
#define unicode_isgraph DeeUni_IsGraph
#undef unicode_isprint
#define unicode_isprint DeeUni_IsPrint
#undef unicode_isblank
#define unicode_isblank DeeUni_IsBlank
#undef unicode_istitle
#define unicode_istitle DeeUni_IsTitle
#undef unicode_isnumeric
#define unicode_isnumeric DeeUni_IsNumeric
#undef unicode_issymstrt
#define unicode_issymstrt DeeUni_IsSymStrt
#undef unicode_issymcont
#define unicode_issymcont DeeUni_IsSymCont
#undef unicode_getnumeric
#define unicode_getnumeric DeeUni_GetNumeric8
#undef unicode_getnumeric64
#define unicode_getnumeric64 DeeUni_GetNumeric64
#undef unicode_getnumericdbl
#define unicode_getnumericdbl DeeUni_GetNumericD
#undef unicode_asdigit
#define unicode_asdigit DeeUni_AsDigit
#undef __unitraits
#define __unitraits Dee_unitraits
#undef __ut_flags
#define __ut_flags ut_flags
#undef __ut_digit_idx
#define __ut_digit_idx ut_digit_idx
#undef __ut_fold_idx
#define __ut_fold_idx ut_fold_idx
#undef __ut_lower
#define __ut_lower ut_lower
#undef __ut_upper
#define __ut_upper ut_upper
#undef __ut_title
#define __ut_title ut_title
#undef __unicode_descriptor
#define __unicode_descriptor DeeUni_Descriptor
#undef __UNICODE_ISCNTRL
#define __UNICODE_ISCNTRL Dee_UNICODE_ISCNTRL
#undef __UNICODE_ISCTAB
#define __UNICODE_ISCTAB Dee_UNICODE_ISCTAB
#undef __UNICODE_ISXTAB
#define __UNICODE_ISXTAB Dee_UNICODE_ISXTAB
#undef __UNICODE_ISTAB
#define __UNICODE_ISTAB Dee_UNICODE_ISTAB
#undef __UNICODE_ISWHITE
#define __UNICODE_ISWHITE Dee_UNICODE_ISWHITE
#undef __UNICODE_ISEMPTY
#define __UNICODE_ISEMPTY Dee_UNICODE_ISEMPTY
#undef __UNICODE_ISLF
#define __UNICODE_ISLF Dee_UNICODE_ISLF
#undef __UNICODE_ISSPACE
#define __UNICODE_ISSPACE Dee_UNICODE_ISSPACE
#undef __UNICODE_ISLOWER
#define __UNICODE_ISLOWER Dee_UNICODE_ISLOWER
#undef __UNICODE_ISUPPER
#define __UNICODE_ISUPPER Dee_UNICODE_ISUPPER
#undef __UNICODE_ISXALPHA
#define __UNICODE_ISXALPHA Dee_UNICODE_ISXALPHA
#undef __UNICODE_ISALPHA
#define __UNICODE_ISALPHA Dee_UNICODE_ISALPHA
#undef __UNICODE_ISDIGIT
#define __UNICODE_ISDIGIT Dee_UNICODE_ISDIGIT
#undef __UNICODE_ISHEX
#define __UNICODE_ISHEX Dee_UNICODE_ISHEX
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
#undef __UNICODE_ISTITLE
#define __UNICODE_ISTITLE Dee_UNICODE_ISTITLE
#undef __UNICODE_ISXNUMERIC
#define __UNICODE_ISXNUMERIC Dee_UNICODE_ISXNUMERIC
#undef __UNICODE_ISNUMERIC
#define __UNICODE_ISNUMERIC Dee_UNICODE_ISNUMERIC
#undef __UNICODE_ISSYMSTRT
#define __UNICODE_ISSYMSTRT Dee_UNICODE_ISSYMSTRT
#undef __UNICODE_ISSYMCONT
#define __UNICODE_ISSYMCONT Dee_UNICODE_ISSYMCONT
#endif /* !CONFIG_HAVE_UNICODE_H */

#undef __libc_hex2int
#define __libc_hex2int(ch, p_result) \
	DeeUni_AsDigit(ch, 16, p_result)

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
