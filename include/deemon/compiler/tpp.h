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
#ifndef GUARD_DEEMON_COMPILER_TPP_H
#define GUARD_DEEMON_COMPILER_TPP_H 1

#include "../api.h"

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
#include <hybrid/sched/__yield.h> /* __hybrid_yield */
#include <hybrid/typecore.h>      /* __CHAR_BIT__, __SIZEOF_INTMAX_T__ */

#include "../alloc.h"           /* Dee_*alloc*, Dee_Alloca, Dee_Free */
#include "../file.h"            /* DeeFile_*, Dee_FILEIO_FNONBLOCKING, Dee_FILEIO_FNORMAL, Dee_OPEN_F*, Dee_STDIN, Dee_STDOUT */
#include "../int.h"             /* DeeIntObject, DeeInt_*, Dee_INT_PRINT_DEC, _DeeInt_NewU */
#include "../object.h"          /* DeeObject_*, Dee_COMPARE_ISERR, Dee_Decref, Dee_Incref */
#include "../string.h"          /* DeeAscii_*, DeeUni_* */
#include "../stringutils.h"     /* Dee_unicode_utf8seqlen_safe */
#include "../system-features.h" /* CONFIG_HAVE_memmem, DeeSystem_DEFINE_*, bzero, memchr, memcmp, memcpy, memmem, memmove, memmovedown, memmoveup, mempcpy, memset, strchr, strlen */
#include "../system.h"          /* DeeSystem_* */
#include "../thread.h"          /* DeeThread_CheckInterrupt */
#include "../types.h"           /* DREF, DeeObject, Dee_AsObject, Dee_ITER_DONE, Dee_formatprinter_t, Dee_ssize_t */
#include "../util/atomic.h"     /* Dee_atomic_* */
#include "../util/once.h"       /* Dee_ONCE */
#include "lexer.h"              /* PARSE_FLFSTMT, parser_flags */

#include <stdint.h> /* PTRDIFF_MAX, SIZE_MAX, UINTMAX_C, UINTMAX_MAX, UINTn_C, intmax_t, uint32_t, uintmax_t */

/************************************************************************/
/* TPP3 API hooks                                                       */
/************************************************************************/
/* Generic HARD_ERROR that means that a deemon error was thrown
 * NOTE: We re-use `TPP_ENOMEM` for this case, since `TPP_ENOMEM`
 *       is already thrown by TPP3 internally whenever one of our
 *       `Dee_Malloc()` functions fails (which they only do when
 *       also throwing an error) */
#define TPP_EDEEMON TPP_ENOMEM

#define TPP_CONFIG_USERDEFS_FILENAME "../../../../include/deemon/compiler/lexer.def"
#ifdef CONFIG_HOST_WINDOWS
#define TPP_OS_WINDOWS 1
#else /* CONFIG_HOST_WINDOWS */
#define TPP_OS_WINDOWS 0
#endif /* !CONFIG_HOST_WINDOWS */
#ifdef CONFIG_HOST_UNIX
#define TPP_OS_UNIX 1
#else /* CONFIG_HOST_UNIX */
#define TPP_OS_UNIX 0
#endif /* !CONFIG_HOST_UNIX */

#define TPP_HOST_NO_SYSTEM_INCLUDES 1 /* We do all includes ourselves! */
#ifdef __PREPROCESSOR_HAVE_VA_ARGS
#define TPP_HOST_HAVE_PP_VARARGS 1
#else /* __PREPROCESSOR_HAVE_VA_ARGS */
#define TPP_HOST_HAVE_PP_VARARGS 0
#endif /* !__PREPROCESSOR_HAVE_VA_ARGS */
#define TPPCALL           DCALL
#define TPPVCALL          /* nothing */
#define TPP_IMPL          __INTERN
#define TPP_DECL          __INTDEF
#define TPP_CONST_IMPL    __INTERN_CONST
#define tpp_assume(expr)  __builtin_assume(expr)
#define tpp_restrict      __restrict
#define TPP_NONNULL       __ATTR_NONNULL
#define TPP_WUNUSED       __ATTR_WUNUSED
#define TPP_RETNONNULL    __ATTR_RETNONNULL
#define TPP_PURECALL      __ATTR_PURE
#define TPP_CONSTCALL     __ATTR_CONST
#define TPP_COLDCALL      __ATTR_COLD
#define TPP_NOINLINE      __ATTR_NOINLINE
#define TPP_FALLTHRU      __ATTR_FALLTHROUGH
#define TPP_INLINE        __LOCAL
#define TPP_CHAR_BIT      __CHAR_BIT__
#define tpp_offsetof      __builtin_offsetof
#define tpp_container_of  __COMPILER_CONTAINER_OF
#define tpp_lengthof      __COMPILER_LENOF
#define tpp_unreachable() __builtin_unreachable()
#define tpp_expect        __builtin_expect
#define tpp_likely        __likely
#define tpp_unlikely      __unlikely

#define tpp_uint_least8       uint_least8_t
#define tpp_int_least8        int_least8_t
#define TPP_UINT_LEAST8_MAX   UINT_LEAST8_MAX
#define TPP_UINT_LEAST8_C(x)  UINT8_C(x)
#define tpp_uint_least16      uint_least16_t
#define tpp_int_least16       int_least16_t
#define TPP_UINT_LEAST16_MAX  UINT_LEAST16_MAX
#define TPP_UINT_LEAST16_C(x) UINT16_C(x)
#define tpp_uint_least32      uint_least32_t
#define tpp_int_least32       int_least32_t
#define TPP_UINT_LEAST32_MAX  UINT_LEAST32_MAX
#define TPP_UINT_LEAST32_C(x) UINT32_C(x)
#define tpp_uint_least64      uint_least64_t
#define tpp_int_least64       int_least64_t
#define TPP_UINT_LEAST64_MAX  UINT_LEAST64_MAX
#define TPP_UINT_LEAST64_C(x) UINT64_C(x)
#define tpp_uint_fast8        uint_fast8_t
#define tpp_int_fast8         int_fast8_t
#define TPP_UINT_FAST8_MAX    UINT_FAST8_MAX
#define TPP_UINT_FAST8_C(x)   UINT8_C(x)
#define tpp_uint_fast16       uint_fast16_t
#define tpp_int_fast16        int_fast16_t
#define TPP_UINT_FAST16_MAX   UINT_FAST16_MAX
#define TPP_UINT_FAST16_C(x)  UINT16_C(x)
#define tpp_uint_fast32       uint_fast32_t
#define tpp_int_fast32        int_fast32_t
#define TPP_UINT_FAST32_MAX   UINT_FAST32_MAX
#define TPP_UINT_FAST32_C(x)  UINT32_C(x)
#define tpp_intmax            intmax_t
#define tpp_uintmax           uintmax_t
#define TPP_UINTMAX_MAX       UINTMAX_MAX
#define TPP_UINTMAX_C(x)      UINTMAX_C(x)
#define tpp_size              size_t
#define TPP_SIZE_MAX          SIZE_MAX
#define tpp_ssize             ptrdiff_t
#define TPP_SSIZE_MAX         PTRDIFF_MAX
#ifdef __LONGDOUBLE
#define tpp_float __LONGDOUBLE
#else /* __LONGDOUBLE */
#define tpp_float double
#endif /* !__LONGDOUBLE */
#define TPP_REF               DREF
#define TPP_STATIC_ASSERT     __STATIC_ASSERT
#define TPP_STATIC_ASSERT_MSG __STATIC_ASSERT_MSG

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen Dee_libc_strnlen
DeeSystem_DEFINE_strnlen(Dee_libc_strnlen)
#endif /* !CONFIG_HAVE_strnlen */

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp Dee_libc_strcmp
DeeSystem_DEFINE_strcmp(Dee_libc_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#define tpp_strlen(s)        strlen(s)
#define tpp_strchr(s, c)     strchr(s, c)
#define _tpp_strnlen(s, n)   strnlen(s, n)
#define tpp_strcmp(a, b)     strcmp(a, b)
#define tpp_memcmp(a, b, n)  memcmp(a, b, n)
#define tpp_memcpy(d, s, n)  memcpy(d, s, n)
#define tpp_memset(d, c, n)  memset(d, c, n)
#define tpp_memchr(p, c, n)  memchr(p, c, n)
#define tpp_memmove(d, s, n) memmove(d, s, n)
#ifdef CONFIG_HAVE_memmem
#define tpp_memmem(h, hs, n, ns) memmem(h, hs, n, ns)
#endif /* CONFIG_HAVE_memmem */
#define tpp_memmoveup(d, s, n)   memmoveup(d, s, n)
#define tpp_memmovedown(d, s, n) memmovedown(d, s, n)
#define tpp_mempcpy(d, s, n)     mempcpy(d, s, n)
#define tpp_bzero(p, n)          bzero(p, n)
#ifndef CONFIG_HAVE_qsort
#define CONFIG_HAVE_qsort
#define qsort Dee_libc_qsort
DeeSystem_DEFINE_qsort(Dee_libc_qsort)
#endif /* !CONFIG_HAVE_qsort */
#define tpp_qsort(p, elem_count, elem_size, cmp)  qsort(p, elem_count, elem_size, cmp)
#define TPP_QSORT_DEFINE_CALLBACK(NAME, lhs, rhs) static int NAME(void const *lhs, void const *rhs)

#define tpp_trymalloc(s)     Dee_TryMalloc(s)
#define tpp_malloc(s)        Dee_Malloc(s)
#define tpp_tryrealloc(p, s) Dee_TryRealloc(p, s)
#define tpp_realloc(p, s)    Dee_Realloc(p, s)
#define tpp_free(p)          Dee_Free(p)
#ifdef Dee_Alloca
#define tpp_alloca(s) Dee_Alloca(s)
#endif /* Dee_Alloca */
#define tpp_assert Dee_ASSERT
#define TPP_SYSCALL(expr, return_error) \
	do {                                \
		if (DeeThread_CheckInterrupt()) \
			return_error(TPP_EIO);      \
		expr;                           \
	} while (0)
#define tpp_formatprinter Dee_formatprinter_t
#define tpp_formatprinter_print(printer, arg, text, num_bytes) \
	(*(printer))(arg, (char const *)(text), num_bytes)
#define tpp_formatprinter_of(NAME) (&NAME)
#define TPP_FORMATPRINTER_DECL(NAME)                        \
	TPP_DECL WUNUSED ATTR_INS(2, 3) Dee_ssize_t DPRINTER_CC \
	NAME(void *arg, char const *__restrict text, size_t num_bytes)
#define TPP_FORMATPRINTER_IMPL(NAME, arg, text, num_bytes)  \
	TPP_IMPL WUNUSED ATTR_INS(2, 3) Dee_ssize_t DPRINTER_CC \
	NAME(void *arg, char const *__restrict text, size_t num_bytes)
#define TPP_FORMATPRINTER_DEFINE(NAME, arg, text, num_bytes) \
	PRIVATE WUNUSED ATTR_INS(2, 3) Dee_ssize_t DPRINTER_CC   \
	NAME(void *arg, char const *__restrict text, size_t num_bytes)
#define tpp_formatprinter_print_byname(NAME, arg, text, num_bytes) \
	NAME(arg, (char const *)(text), num_bytes)
#ifdef CONFIG_NO_THREADS
#define TPP_SINGLE_THREADED 1
#else /* CONFIG_NO_THREADS */
#define TPP_SINGLE_THREADED 0
#endif /* !CONFIG_NO_THREADS */
#define tpp_atomic32                        uint32_t
#define TPP_ATOMIC32_INIT(value)            value
#define tpp_atomic32_init(p_atomic, value)  (void)(*(p_atomic) = (value))
#define tpp_atomic32_read(p_atomic)         Dee_atomic_read(p_atomic)
#define tpp_atomic32_xchg(p_atomic, newval) Dee_atomic_xch(p_atomic, newval)
#define tpp_atomic32_inc(p_atomic)          Dee_atomic_inc(p_atomic)
#define tpp_atomic32_decfetch(p_atomic)     Dee_atomic_decfetch(p_atomic)
#define tpp_sched_yield()                   __hybrid_yield()
#define tpp_once(expr)                      Dee_ONCE({ expr; })

#define TPP_HAVE_ASSUME_ASCII_CTYPE 1
#define tpp_ascii_issymstrt(ch)     DeeAscii_IsSymStrt(ch)
#define tpp_ascii_issymcont(ch)     DeeAscii_IsSymCont(ch)
#define tpp_ascii_isdigit(ch)       DeeAscii_IsDigit(ch)
#define tpp_ascii_isspace(ch)       DeeAscii_IsSpace(ch)
#define tpp_ascii_islf(ch)          DeeAscii_IsLF(ch)
#define tpp_ascii_isspace_nolf(ch)  DeeAscii_IsSpaceNoLf(ch)
#define tpp_ascii_isxdigit(ch)      DeeAscii_IsXDigit(ch)
#define tpp_ascii_asxdigit(ch)      DeeAscii_AsDigitVal(ch)
#define tpp_ascii_tolwrxdigit(v)    DeeAscii_ItoaLowerDigit(v)
#define tpp_ascii_touprxdigit(v)    DeeAscii_ItoaUpperDigit(v)

#define tpp_unicode_issymstrt(ord)    DeeUni_IsSymStrt(ord)
#define tpp_unicode_issymcont(ord)    DeeUni_IsSymCont(ord)
#define tpp_unicode_isspace(ord)      DeeUni_IsSpace(ord)
#define tpp_unicode_islf(ord)         DeeUni_IsLF(ord)
#define tpp_unicode_isspace_nolf(ord) DeeUni_IsSpaceNoLf(ord)

#if 0 /* Would return wrong values for over-long utf-8 sequences */
#define tpp_unicode_utf8seqlen_mb_getcur(first_utf8_byte) Dee_unicode_utf8seqlen[first_utf8_byte]
#define tpp_unicode_utf8seqlen_getcur(first_utf8_byte)    Dee_unicode_utf8seqlen[first_utf8_byte]
#endif
#define tpp_unicode_utf8seqlen_mb_getmax(first_utf8_byte) Dee_unicode_utf8seqlen_safe[first_utf8_byte]
#define tpp_unicode_utf8seqlen_getmax(first_utf8_byte)    Dee_unicode_utf8seqlen_safe[first_utf8_byte]
#ifdef CONFIG_HAVE_fuzzy_memcmp /* TODO: Add configure-test for "fuzzy_memcmp" */
#define tpp_fuzzy_memcmp(lhs, lhs_len, rhs, rhs_len) fuzzy_memcmp(lhs, lhs_len, rhs, rhs_len)
#endif /* CONFIG_HAVE_fuzzy_memcmp */

#define tpp_intvalue                 DREF DeeIntObject *
#define tpp_intvalue_fini(self)      Dee_Decref(*(self))
#define tpp_intvalue_init_zero(self) (*(self) = (DREF DeeIntObject *)DeeInt_NewZero(), TPP_EOK)
#define tpp_intvalue_init_copy(dst, src) \
	(*(dst) = *(src), Dee_Incref(*(dst)), TPP_EOK)
#define tpp_intvalue_asintmax(self, p_result) \
	(DeeInt_TryAsIntN(__SIZEOF_INTMAX_T__, Dee_AsObject(*(self)), p_result) ? TPP_EOK : TPP_ENOENT)

/* TODO: Proper support for `tpp_intvalue_builder` */
//TODO:typedef ... tpp_intvalue_builder;
//TODO:tpp_errno tpp_intvalue_builder_init(tpp_intvalue_builder *self, unsigned int radix);
//TODO:void tpp_intvalue_builder_fini(tpp_intvalue_builder *self);
//TODO:tpp_errno tpp_intvalue_builder_pack(/*inherit(always)*/ tpp_intvalue_builder *self,
//TODO:                                    /*initialize(on_success)*/ tpp_intvalue *p_intvalue);
//TODO:tpp_errno tpp_intvalue_builder_adddigit(tpp_intvalue_builder *self, unsigned int digit);

#define tpp_intvalue_init_uintmax(self, v) \
	((*(self) = (DREF DeeIntObject *)_DeeInt_NewU(__SIZEOF_INTMAX_T__, v)) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_init_one(self)     (*(self) = (DREF DeeIntObject *)DeeInt_NewOne(), TPP_EOK)
#define tpp_intvalue_init_bool(self, v) (*(self) = (DREF DeeIntObject *)DeeInt_NewSmallInt((v) ? 1 : 0), TPP_EOK)
#define tpp_intvalue_init_size(self, v) ((*(self) = (DREF DeeIntObject *)DeeInt_NewSize(v)) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_init_char(self, v) ((*(self) = (DREF DeeIntObject *)DeeInt_NewUInt8(v)) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_asbool(self)       (DeeInt_IsZero(*(self)) ? TPP_ENOENT : TPP_EOK)
#define tpp_intvalue_isneg(self)        (DeeInt_IsNeg(*(self)) ? TPP_EOK : TPP_ENOENT)
#define tpp_intvalue_neg(self, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Neg(Dee_AsObject(*(self)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_inv(self, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Inv(Dee_AsObject(*(self)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_add(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Add(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_sub(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Sub(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_mul(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Mul(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_div(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Div(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_mod(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Mod(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_shl(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Shl(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_shr(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Shr(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_and(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_And(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_xor(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Xor(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_or(lhs, rhs, p_result) \
	((*(p_result) = (DREF DeeIntObject *)DeeObject_Or(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))) != NULL ? TPP_EOK : TPP_EDEEMON)
#define tpp_intvalue_cmp(lhs, rhs, p_delta)                                        \
	((*(p_delta) = DeeObject_Compare(Dee_AsObject(*(lhs)), Dee_AsObject(*(rhs)))), \
	 Dee_COMPARE_ISERR(*(p_delta)) ? TPP_EDEEMON : TPP_EOK)
#define tpp_intvalue_printrepr(lexer, self, printer, arg) \
	DeeInt_Print(Dee_AsObject(*(self)), Dee_INT_PRINT_DEC, 0, printer, arg)

#ifdef DeeSystem_HAVE_FS_DRIVES
#define TPP_FS_HAVE_DRIVES 1
#else /* DeeSystem_HAVE_FS_DRIVES */
#define TPP_FS_HAVE_DRIVES 0
#endif /* !DeeSystem_HAVE_FS_DRIVES */

#ifdef DeeSystem_HAVE_FS_ICASE
#define TPP_FS_HAVE_ICASE 1
#else /* DeeSystem_HAVE_FS_ICASE */
#define TPP_FS_HAVE_ICASE 0
#endif /* !DeeSystem_HAVE_FS_ICASE */

#define TPP_FS_SEP    DeeSystem_SEP
#define TPP_FS_SEP_S  DeeSystem_SEP_S
#define TPP_FS_DELIM  DeeSystem_DELIM
#define TPP_FS_ALTSEP DeeSystem_ALTSEP
#define TPP_FS_ISSEP  DeeSystem_IsSep
#define TPP_FS_ISABS  DeeSystem_IsAbsN

/************************************************************************/
/* Configure TPP3                                                       */
/************************************************************************/
#define TPP_PROFILE                    TPP_PROFILE_MINIMAL
#define TPP_TABSIZE                    (-4)
#define TPP_ERROR_LIMIT                (-16)
#define TPP_MAX_INCLUDE_DEPTH          (-64)
#define TPP_MAX_RECURSIVE_MACRO_DEPTH  (-4096)
#define TPP_HAVE_FILE_NONBLOCK         0 /* Not for now... */
#define TPP_HAVE_UNICODE               1
#define TPP_HAVE_BUILTIN_CTYPE_UNICODE 0 /* We provide our own... */
#define TPP_HAVE_STRERROR              0 /* Not needed -- we do our own mapping */
#define TPP_HAVE_EUSER                 0 /* Not actually needed */
#define TPP_HAVE_STRTOKENID            1 /* For `TPP_EMITTER_HAVE_MODE_TYPED` */
#define TPP_HAVE_KEYWORD_USERDATA      1 /* Needed only for emitter */
#define TPP_HAVE_KEYWORD_ASSTRING      0
#define TPP_HAVE_KEYWORD_INCLCOUNT     1
#define TPP_HAVE_EXTENSIONS            1
#define TPP_HAVE_EXTENSIONS_PUSH_POP   1
#define TPP_HAVE_WARNINGS              1
#define TPP_HAVE_WARNINGS_PUSH_POP     1
#define TPP_HAVE_WARNING_NUMBERS       1
#define TPP_HAVE_WARNING_ERROR         1
#define TPP_HAVE_WARNING_SUPPRESS      1
#define TPP_HAVE_WARNING_DEFAULT       1
#define TPP_HAVE_FILE_NOCLOSE          0
#define TPP_HAVE_FILE_NOKWD            1
#define TPP_HAVE_FILE_LC_CACHE         1
#define TPP_HAVE_CR_LF_DETECTION       1
#define TPP_HAVE_LEXER_COPY            1 /* Needed for `deemon -F` */
#define TPP_HAVE_LEXER_WARNING_COUNT   0

#define TPP_COMMON_HAVE_TPP_TOK             0
#define TPP_COMMON_HAVE_TPP_TOK_SPACE       0
#define TPP_COMMON_HAVE_TPP_TOK_COMMENT     0
#define TPP_COMMON_HAVE_TPP_TOK_C_GENERIC   TPP_COMMON_HAVE_TPP_TOK
#define TPP_COMMON_HAVE_TPP_TOK_CXX_STRING  0
#define TPP_COMMON_HAVE_TPP_TOK_C_TOKENS    0
#define TPP_COMMON_HAVE_TPP_TOK_CXX_TOKENS  0
#define TPP_COMMON_HAVE_TPP_TOK_MISC_TOKENS 0
#define TPP_COMMON_HAVE_CPP_DIRECTIVES_STD  1
#define TPP_COMMON_HAVE_CPP_DIRECTIVES_EXT  1

#define TPP_HAVE_BSE                          1
#define TPP_HAVE_BSE_WHITESPACE               1
#define TPP_HAVE_IDENTIFIER_ESCAPE_UNI        1
#define TPP_HAVE_IDENTIFIER_ESCAPE_NAMED      1
#define TPP_HAVE_IDENTIFIER_ESCAPE_NAMED_MANY 1
#define TPP_HAVE_CPP_DIRECTIVES               1
#define TPP_HAVE_CPP_MACROS                   1
#define TPP_HAVE_MAGIC_WHITESPACE             1

#define TPP_HAVE_CPP_BUILTIN_MACROS    1
#define TPP_HAVE_CPP_PREDEFINED_MACROS TPP_CONF_EXT1 /* Disabled via `-undef` */
#define TPP_HAVE_CPP_EXCLAIM           1
#define TPP_HAVE_CPP_BLANK             1
#define TPP_HAVE_CPP_DIGIT_LINE        1
#define TPP_HAVE_LEXER_USERPWD         1
#define TPP_HAVE_CPP_LINE              1
#define TPP_HAVE_CPP_INCLUDE           1
#define TPP_HAVE_CPP_INCLUDE_NEXT      1
#define TPP_HAVE_CPP_IMPORT            1
#define TPP_HAVE_CPP_IF_ELSE_ENDIF     1
#define TPP_HAVE_CPP_DEFINE            1
#define TPP_HAVE_CPP_ASSERT            1
#define TPP_HAVE_CPP_ERROR             1
#define TPP_HAVE_CPP_WARNING           1
#define TPP_HAVE_CPP_IDENT_SCCS        0 /* No longer supported (used to be ignored in the past) */
#define TPP_HAVE_CPP_PRAGMA            1
#define TPP_HAVE_CPP_EMBED             1
#define TPP_HAVE_CPP_EMBED_OFFSET      1

#define TPP_HAVE_MACRO__Pragma                        1
#define TPP_HAVE_MACRO___pragma                       1
#define TPP_HAVE_CLANG_MACRO___has_attribute          0
#define TPP_HAVE_CLANG_MACRO___has_builtin            0
#define TPP_HAVE_CLANG_MACRO___has_cpp_attribute      0
#define TPP_HAVE_CLANG_MACRO___has_declspec_attribute 0
#define TPP_HAVE_CLANG_MACRO___has_extension          0
#define TPP_HAVE_CLANG_MACRO___has_feature            0
#define TPP_HAVE_CLANG_MACRO___has_c_attribute        0
#define TPP_HAVE_CLANG_EXTENSIONS_ARE_FEATURES        0
#define TPP_COMMON_HAVE_KEYWORD_FEATURES              0
#define TPP_HAVE_MACRO___is_identifier                1
#define TPP_HAVE_MACRO___is_deprecated                1
#define TPP_HAVE_MACRO___is_poisoned                  1
#define TPP_HAVE_MACRO___has_extension                1
#define TPP_HAVE_MACRO___has_known_extension          1
#define TPP_HAVE_MACRO___has_warning                  1
#define TPP_HAVE_MACRO___has_known_warning            1
#define TPP_HAVE_MACRO___has_include                  1
#define TPP_HAVE_MACRO___has_include_next             1
#define TPP_HAVE_MACRO___has_embed                    1
#define TPP_HAVE_MACRO___FILE__                       1
#define TPP_HAVE_MACRO___LINE__                       1
#define TPP_HAVE_MACRO___TIME__                       1
#define TPP_HAVE_MACRO___DATE__                       1
#define TPP_HAVE_MACRO___COLUMN__                     1
#define TPP_HAVE_MACRO___BASE_FILE__                  1
#define TPP_HAVE_MACRO___FILE_NAME__                  1
#define TPP_HAVE_MACRO___INCLUDE_LEVEL__              1
#define TPP_HAVE_MACRO___INCLUDE_DEPTH__              1
#define TPP_HAVE_MACRO___COUNTER__                    1
#define TPP_HAVE_MACRO___TIMESTAMP__                  1
#define TPP_HAVE_NUMERIC_DATE_MACROS                  1
#define TPP_HAVE_NUMERIC_TIME_MACROS                  1
#define TPP_HAVE_MACRO___TPP_EVAL                     1
#define TPP_HAVE_MACRO___TPP_EXEC                     1
#define TPP_HAVE_MACRO___TPP_UNIQUE                   1
#define TPP_HAVE_MACRO___TPP_LOAD_FILE                1
#define TPP_HAVE_MACRO___TPP_COUNTER                  1
#define TPP_HAVE_MACRO___TPP_RANDOM                   1
#define TPP_HAVE_MACRO___TPP_STR_DECOMPILE            1
#define TPP_HAVE_MACRO___TPP_STR_PACK                 1
#define TPP_HAVE_MACRO___TPP_STR_SUBSTR               1
#define TPP_HAVE_MACRO___TPP_STR_SIZE                 1
#define TPP_HAVE_MACRO___TPP_COUNT_TOKENS             1
#define TPP_HAVE_MACRO___TPP_IDENTIFIER               1
#define TPP_HAVE_MACRO_CXX_OPERATOR_NAMES             0

#define TPP_HAVE_ALTERNATIVE_MACRO_PARENTHESIS TPP_CONF_EXT1
#define TPP_HAVE_MACRO_ARGUMENT_WHITESPACE     TPP_CONF_EXT1
#define TPP_HAVE_MACRO_RECURSION               TPP_CONF_EXT0
#define TPP_HAVE_TRADITIONAL_MACROS            TPP_CONF_EXT1
#define TPP_HAVE_NAMED_VARARGS_IN_MACROS       1
#define TPP_HAVE_VA_ARGS_IN_MACROS             1
#define TPP_HAVE_VA_COMMA_IN_MACROS            1
#define TPP_HAVE_VA_OPT_IN_MACROS              1
#define TPP_HAVE_VA_NARGS_IN_MACROS            1
#define TPP_HAVE_VA_GLUE_COMMA_IN_MACROS       1
#define TPP_HAVE_STRINGIZE_MACRO_ARGUMENT      1
#define TPP_HAVE_CHARIZE_MACRO_ARGUMENT        1
#define TPP_HAVE_DONT_EXPAND_MACRO_ARGUMENT    1
#define TPP_HAVE_GLUE_MACRO_ARGUMENT           1

#define TPP_HAVE_PRAGMA_PUSH_MACRO                      1
#define TPP_HAVE_PRAGMA_ONCE                            1
#define TPP_HAVE_PRAGMA_DEPRECATED                      1
#define TPP_HAVE_PRAGMA_EXTENSION                       1
#define TPP_HAVE_PRAGMA_WARNING                         1
#define TPP_HAVE_PRAGMA_MESSAGE                         1
#define TPP_HAVE_PRAGMA_MESSAGE_PRINTS_LOCATION         TPP_CONF_EXT0
#define TPP_HAVE_PRAGMA_MESSAGE_OMITS_TRAILING_LINEFEED TPP_CONF_EXT0
#define TPP_HAVE_PRAGMA_ERROR                           1
#define TPP_HAVE_PRAGMA_REGION                          1
#define TPP_HAVE_PRAGMA_TPP_EXEC                        1
#define TPP_HAVE_PRAGMA_TPP_SET_KEYWORD_FLAGS           0 /* No longer supported */
#define TPP_HAVE_PRAGMA_GCC_POISON                      1
#define TPP_HAVE_PRAGMA_GCC_WARNING                     1
#define TPP_HAVE_PRAGMA_GCC_ERROR                       1
#define TPP_HAVE_PRAGMA_GCC_SYSTEM_HEADER               1
#define TPP_HAVE_PRAGMA_GCC_DIAGNOSTIC                  1
#define TPP_HAVE_PRAGMA_GCC_DEPENDENCY                  0 /* TODO: (disabled because of missing "tpp_io_compare_mtime") */
#define TPP_HAVE_PRAGMA_TPP_WARNING                     1
#define TPP_HAVE_PRAGMA_TPP_EXTENSION                   1
#define TPP_HAVE_PRAGMA_TPP_TPP_EXEC                    1
#define TPP_HAVE_PRAGMA_TPP_TPP_SET_KEYWORD_FLAGS       0 /* No longer supported */
#define TPP_HAVE_PRAGMA_TPP_INCLUDE_PATH                1
#define TPP_HAVE_PRAGMA_TPP_KEYWORD_FEATURES            0

#define TPP_HAVE_TRIGRAPHS TPP_CONF_EXT0
#define TPP_HAVE_DIGRAPHS  TPP_CONF_EXT0

#define TPP_HAVE_TOK_LF      TPP_CONF_FEAT0 /* Relevant for `deemon -E` and inline assembly */
#define TPP_HAVE_TOK_SPACE   TPP_CONF_FEAT0 /* Relevant for `deemon -E` */
#define TPP_HAVE_TOK_COMMENT TPP_CONF_FEAT0 /* Relevant for `deemon -E` and `deemon -F` (and `TPP_HAVE_TOK_AT_AT_COMMENT`) */

#define TPP_HAVE_TOK_CXX_COMMENT          1
#define TPP_HAVE_TOK_C_COMMENT            1
#define TPP_HAVE_TOK_PASCAL_COMMENT       0
#define TPP_HAVE_TOK_PASCAL_BRACE_COMMENT 0
#define TPP_HAVE_TOK_HTML_COMMENT         0
#define TPP_HAVE_TOK_SQL_COMMENT          0
#define TPP_HAVE_TOK_AT_AT_COMMENT        1 /* For doc strings */
#define TPP_HAVE_TOK_SHELL_COMMENT        1 /* Relevant in inline assembly */
#define TPP_HAVE_TOK_SLASH_COMMENT        0
#define TPP_HAVE_TOK_AT_COMMENT           0
#define TPP_HAVE_TOK_SOL_SHELL_COMMENT    0
#define TPP_HAVE_TOK_SOL_SLASH_COMMENT    0
#define TPP_HAVE_TOK_SOL_AT_COMMENT       0

#define TPP_HAVE_TOK_DOLLAR                             TPP_CONF_EXT0
#define TPP_HAVE_TOK_C_INT                              1
#define TPP_HAVE_THOUSANDS_SEPARATOR_UNDERSCORE         1
#define TPP_HAVE_THOUSANDS_SEPARATOR_SINGLETICK         1
#define TPP_HAVE_TOK_PASCAL_HEX                         0
#define TPP_HAVE_TOK_C_FLOAT                            1
#define TPP_HAVE_SMART_FLOAT_TOKENS                     TPP_CONF_EXT1
#define TPP_HAVE_TOK_C_CHAR                             1
#define TPP_HAVE_TOK_C_STRING                           1
#define TPP_HAVE_TOK_RAW_STRING_LITERAL                 1
#define TPP_HAVE_TOK_RAW_CHAR_LITERAL                   1
#define TPP_HAVE_TOK_BLOCK_STRING_LITERAL               1 /* New feature under `CONFIG_EXPERIMENTAL_USE_TPP3` */
#define TPP_HAVE_TOK_BLOCK_CHAR_LITERAL                 1 /* New feature under `CONFIG_EXPERIMENTAL_USE_TPP3` */
#define TPP_HAVE_TOK_PYTHON_FORMAT_STRING_LITERAL       1
#define TPP_HAVE_TOK_PYTHON_FORMAT_CHAR_LITERAL         1
#define TPP_HAVE_TOK_JAVASCRIPT_FORMAT_BACKTICK_LITERAL 0

#define TPP_HAVE_IFNDEF_INCLUDE_GUARDS             1
#define TPP_HAVE_INCLUDE_REMAP                     TPP_CONF_EXT0
#define TPP_HAVE_USER_KEYWORDS                     1
#define TPP_HAVE_RAW_STRING_BSE                    1
#define TPP_HAVE_STRING_ESCAPE_E                   1
#define TPP_HAVE_STRING_ESCAPE_S                   1
#define TPP_HAVE_STRING_ESCAPE_XML                 1
#define TPP_HAVE_STRING_ESCAPE_OCT                 1
#define TPP_HAVE_STRING_ESCAPE_OCT_BRACE           1
#define TPP_HAVE_STRING_ESCAPE_OCT_BRACE_MANY      1
#define TPP_HAVE_STRING_ESCAPE_HEX                 1
#define TPP_HAVE_STRING_ESCAPE_HEX_BIG             0 /* Nope: only 2-nibble is supported */
#define TPP_HAVE_STRING_ESCAPE_HEX_BRACE           1
#define TPP_HAVE_STRING_ESCAPE_HEX_BRACE_MANY      1
#define TPP_HAVE_STRING_ESCAPE_UNI                 1
#define TPP_HAVE_STRING_ESCAPE_UNI_BRACE           1
#define TPP_HAVE_STRING_ESCAPE_UNI_BRACE_MANY      1
#define TPP_HAVE_STRING_ESCAPE_NAMED               1
#define TPP_HAVE_STRING_ESCAPE_NAMED_MANY          1
#define TPP_HAVE_STRING_ESCAPE_FORMAT_PAREN        0
#define TPP_HAVE_STRING_ESCAPE_FORMAT_BRACKET      0
#define TPP_HAVE_STRING_ESCAPE_FORMAT_BRACE        0             /* XXX: Maybe turn this on? */
#define TPP_HAVE_STRING_ESCAPE_BIGCHAR             0             /* Nope: only 2-nibble is supported */
#define TPP_HAVE_STRING_ALLOW_MULTILINE            TPP_CONF_EXT1 /* Only exists due to legacy code */
#define TPP_HAVE_STRING_WARN_MULTILINE             1             /* Warn when legacy code uses this... */
#define TPP_HAVE_STRING_AUTO_CONCAT                1
#define TPP_HAVE_ESCAPE_NAMED_UNICODE_NAMES        1
#define TPP_HAVE_ESCAPE_NAMED_UNICODE_ORD          1
#define TPP_HAVE_ESCAPE_NAMED_XML                  1
#define TPP_HAVE_UNICODE_BYNAME_LOOKUP_ICASE       TPP_CONF_EXT1
#define TPP_HAVE_UNICODE_BYNAME_LOOKUP_ISPACE      TPP_CONF_EXT0
#define TPP_HAVE_UNICODE_BYNAME_LOOKUP_ENTRY_TABLE 1

#define TPP_COMMON_HAVE_HOOK_COOKIES 0 /* Not needed: can use offsets instead! */

#define TPP_HOOK_DEFAULT_BUILTIN TPP_HOOK_CONST_BUILTIN
#define TPP_HOOK_DEFAULT_USER    TPP_HOOK_CONST_USER
#define TPP_HOOK_DEFAULT_NOOP    TPP_HOOK_DISABLED

#define TPP_HAVE_WARNPRINTER_HOOK           TPP_HOOK_CONST_USER
#define TPP_HOOK_WARNPRINTER                DeeLexer_TPP_WarnPrinterHook
#define TPP_HAVE_WARNHANDLER_HOOK           TPP_HOOK_CONST_USER
#define TPP_HOOK_WARNHANDLER                DeeLexer_TPP_WarnHandlerHook
#define TPP_HAVE_BUILTIN_WARNHANDLER_HOOK   1
#define TPP_HAVE_MESGPRINTER_HOOK           TPP_HOOK_CONST_USER
#define TPP_HOOK_MESGPRINTER                DeeLexer_TPP_MesgPrinterHook
/* #define TPP_HOOK_PARSEEXPR TODO: parse a deemon expression */
#define TPP_HAVE_UNKNOWN_PRAGMA_HOOK        TPP_HOOK_RT_NOOP_C /* Must be configurable for EMITTER */
#define TPP_HAVE_NEW_DEPENDENCY_HOOK        TPP_HOOK_RT_NOOP_C /* Must be configurable for MAKEFILE */
#define TPP_HAVE_FILE_PUSHED_HOOK           TPP_HOOK_RT_NOOP_C /* Must be configurable for EMITTER */
#define TPP_HAVE_FILE_POPPED_HOOK           TPP_HOOK_RT_NOOP_C /* Must be configurable for EMITTER */
#define TPP_HAVE_INCLUDE_ENCOUNTERED_HOOK   TPP_HOOK_RT_NOOP_C /* Must be configurable for EMITTER */
#define TPP_HAVE_INCLUDE_NOT_FOUND_HOOK     TPP_HOOK_RT_NOOP_C /* Must be configurable for MAKEFILE */
#define TPP_HAVE_MACRO_DEFINED_HOOK         TPP_HOOK_RT_NOOP_C /* Must be configurable for EMITTER */
#define TPP_HAVE_MACRO_UNDEFINED_HOOK       TPP_HOOK_RT_NOOP_C /* Must be configurable for EMITTER */
#define TPP_HAVE_SYSTEM_INCLUDE_PATH_HOOK   TPP_HOOK_CONST_USER
#define TPP_HOOK_SYSTEM_INCLUDE_PATH        DeeLexer_TPP_SystemIncludePathHook
#define TPP_HAVE_RAISE_LEXERROR_HOOK        TPP_HOOK_CONST_USER
#define TPP_HOOK_RAISE_LEXERROR             DeeLexer_TPP_RaiseLexErrorHook


#define TPP_HOOK_ISFLOATSUFFIX(cookie, pos) TPP_ENOENT /* Deemon doesn't have floating-point suffixes */


#define TPP_HAVE_TOK_EXCLAIM_EQUAL              1
#define TPP_HAVE_TOK_EXCLAIM_EQUAL_EQUAL        1
#define TPP_HAVE_TOK_POUND_POUND                1
#define TPP_HAVE_TOK_PERCENT_EQUAL              1
#define TPP_HAVE_TOK_AMP_AMP                    1
#define TPP_HAVE_TOK_AMP_EQUAL                  1
#define TPP_HAVE_TOK_STAR_STAR                  1
#define TPP_HAVE_TOK_STAR_STAR_EQUAL            1
#define TPP_HAVE_TOK_STAR_EQUAL                 1
#define TPP_HAVE_TOK_PLUS_PLUS                  1
#define TPP_HAVE_TOK_PLUS_EQUAL                 1
#define TPP_HAVE_TOK_MINUS_MINUS                1
#define TPP_HAVE_TOK_MINUS_EQUAL                1
#define TPP_HAVE_TOK_MINUS_RANGLE               1
#define TPP_HAVE_TOK_DOT_DOT_DOT                1
#define TPP_HAVE_TOK_SLASH_EQUAL                1
#define TPP_HAVE_TOK_COLON_EQUAL                1
#define TPP_HAVE_TOK_LANGLE_LANGLE              1
#define TPP_HAVE_TOK_LANGLE_LANGLE_MINUS        1
#define TPP_HAVE_TOK_LANGLE_LANGLE_LANGLE       1
#define TPP_HAVE_TOK_LANGLE_LANGLE_LANGLE_EQUAL 1
#define TPP_HAVE_TOK_LANGLE_LANGLE_EQUAL        1
#define TPP_HAVE_TOK_LANGLE_EQUAL               1
#define TPP_HAVE_TOK_EQUAL_EQUAL                1
#define TPP_HAVE_TOK_EQUAL_EQUAL_EQUAL          1
#define TPP_HAVE_TOK_RANGLE_EQUAL               1
#define TPP_HAVE_TOK_RANGLE_RANGLE              1
#define TPP_HAVE_TOK_RANGLE_RANGLE_EQUAL        1
#define TPP_HAVE_TOK_RANGLE_RANGLE_RANGLE       1
#define TPP_HAVE_TOK_RANGLE_RANGLE_RANGLE_EQUAL 1
#define TPP_HAVE_TOK_QMARK_QMARK                1
#define TPP_HAVE_TOK_HAT_EQUAL                  1
#define TPP_HAVE_TOK_PIPE_EQUAL                 1
#define TPP_HAVE_TOK_PIPE_PIPE                  1

#define TPP_HAVE_BUILTIN_EXPR_DEFINED                1
#define TPP_HAVE_DONT_EXPAND_DEFINED_IN_EXPR         TPP_CONF_EXT1
#define TPP_HAVE_BUILTIN_EXPR_STRINGS                1
#define TPP_HAVE_BUILTIN_EXPR_FLOATS                 1
#define TPP_HAVE_BUILTIN_EXPR_IF_ELSE_OPTIONAL_TT    1
#define TPP_HAVE_BUILTIN_EXPR_IF_ELSE_IN_EXPRESSIONS 1
#define TPP_HAVE_BUILTIN_EXPR_LOGICAL_XOR            0
#define TPP_HAVE_BUILTIN_EXPR_CHARACTER_LITERALS     TPP_CONF_EXT0
#define TPP_HAVE_RT_FILE_AND_LINE_FORMAT             0
#define TPP_HAVE_QUALITY_WARNINGS                    1

#define TPP_HAVE_FORMAT_STRING_BUILTIN_EXPR          1
#define TPP_HAVE_INCLUDE_PATH_ENVIRON                1
#define TPP_CONFIG_INCLUDE_PATH_ENVIRON              2("DEEMON_PATH", "CPATH")
#define TPP_HAVE_INCLUDE_PATH_QUOTE                  1
#define TPP_HAVE_INCLUDE_PATH_SYSHDR                 1
#define TPP_HAVE_INCLUDE_PATH_AFTER                  1
#define TPP_HAVE_INCLUDE_RELATIVE_TO_CURRENT_FILE    1
#define TPP_HAVE_INCLUDE_PATH_EMBED                  1
#define TPP_HAVE_INCLUDE_PATH_PUSH_POP               1
#define TPP_HAVE_WERROR                              1
#define TPP_HAVE_WSYSTEM_HEADERS                     1
#define TPP_HAVE_FILE_GETLCINFO_EX_PROJPOS           1
#define TPP_HAVE_FILE_MACRO_TRACKARGS                1
#define TPP_HAVE_FILE_ENCODING_EMBED                 1
#define TPP_HAVE_LEXER_SKIP                          1
#define TPP_HAVE_LEXER_TRYSKIP_RAW                   1
#define TPP_HAVE_LEXER_PEEK_RAW                      1
#define TPP_HAVE_LEXER_REPRTOKENID                   1
#define TPP_HAVE_LEXER_GETKEYWORDFEATURE             0
#define TPP_HAVE_LEXER_GETKEYWORDDEFINED             1
#define TPP_HAVE_LEXER_ISIDENTIFIER                  1
#define TPP_HAVE_LEXER_ISIDENTIFIER_DEFAULT          0
#define TPP_HAVE_MACRO_NAME                          1
#define TPP_HAVE_LEXER_DUMP_DEFINITIONS              1
#define TPP_HAVE_LEXER_DUMP_DEFINITIONS_SORTED       1
#define TPP_HAVE_LEXER_DUMP_DEFINITIONS_EXTRAINFO    1
#define TPP_HAVE_LEXER_REQUIRE_WHITESPACE            1
#define TPP_HAVE_LEXER_PARSEEMBED                    1
#define TPP_HAVE_LEXER_DECODEINT                     1
#define TPP_HAVE_LEXER_DECODEINT_HEX_LITERALS        1
#define TPP_HAVE_LEXER_DECODEINT_BINARY_LITERALS     1
#define TPP_HAVE_LEXER_DECODEINT_OCTAL_LITERALS      0 /* Dumb... */
#define TPP_HAVE_LEXER_DECODEFLOAT                   1
#define TPP_INTVALUE_MATH_CANOVERFLOW                0 /* Nope: because we use `DeeIntObject` to get arbitrary-length integers */
#define TPP_INTVALUE_ASINTMAX_CANOVERFLOW            1 /* Yes: because `DeeIntObject` is used, these can get *really* big */
#define TPP_HAVE_TPP_EXTENSION_NEAREST               1
#define TPP_HAVE_TPP_WARNING_GROUP_NEAREST           1
#define TPP_HAVE_API_TOKEN_NAMES_IN_GLOBAL_NAMESPACE 0
#define TPP_HAVE_CPP_FEATURE_MACROS                  0
#define TPP_HAVE_XML_ENTITY_PRINTNEAREST             1
#define TPP_HAVE_UNICODE_BYNAME_PRINTNEAREST         1
#define TPP_HAVE_DECODE_NAMED_PRINTNEAREST           1

#define TPP_HAVE_CLI                              1
#define TPP_HAVE_CLI_HELP                         1
#define TPP_HAVE_CLI_HELP_ALL_SPELLINGS           1
#define TPP_HAVE_CLI_DASH_DEFINE_MACRO            1
#define TPP_HAVE_CLI_DASH_UNDEFINE_MACRO          1
#define TPP_HAVE_CLI_DASH_ASSERT                  1
#define TPP_HAVE_CLI_DASH_INCLUDE                 1
#define TPP_HAVE_CLI_DASH_IMACROS                 1
#define TPP_HAVE_CLI_DASH_UNDEF                   1
#define TPP_HAVE_CLI_DASH_FEXTENSION              1
#define TPP_HAVE_CLI_DASH_FDOLLARS_IN_IDENTIFIERS 1
#define TPP_HAVE_CLI_DASH_FMAX_INCLUDE_DEPTH      1
#define TPP_HAVE_CLI_DASH_FTABSTOP                1
#define TPP_HAVE_CLI_DASH_COMMENTS                1
#define TPP_HAVE_CLI_DASH_TRADITIONAL             1
#define TPP_HAVE_CLI_DASH_TRIGRAPHS               1
#define TPP_HAVE_CLI_DASH_INCLUDE_DIRECTORY       1
#define TPP_HAVE_CLI_DASH_IQUOTE                  1
#define TPP_HAVE_CLI_DASH_ISYSTEM                 1
#define TPP_HAVE_CLI_DASH_IDIRAFTER               1
#define TPP_HAVE_CLI_DASH_EMBED_DIR               1
#define TPP_HAVE_CLI_DASH_IWITHPREFIX             1
#define TPP_HAVE_CLI_DASH_IWITHPREFIXBEFORE       1
#define TPP_HAVE_CLI_DASH_IPREFIX                 1
#define TPP_HAVE_CLI_DASH_ISYSROOT                1
/*#define TPP_CONFIG_CLI_DEFAULT_SYSROOT TODO:DeeExec_GetHome() ??? */
#define TPP_HAVE_CLI_DASH_REMAP                   1
#define TPP_HAVE_CLI_DASH_WERROR                  1
#define TPP_HAVE_CLI_DASH_WFATAL_ERROR            1
#define TPP_HAVE_CLI_DASH_FMAX_ERRORS             1
#define TPP_HAVE_CLI_DASH_WWARNING                1
#define TPP_HAVE_CLI_DASH_WERROR_WARNING          1
#define TPP_HAVE_CLI_SETINPUTS                    1
#define TPP_HAVE_CLI_SETINPUTS_DASH               1
#define TPP_HAVE_CLI_DASH_FSEARCH_INCLUDE_PATH    1

#define TPP_HAVE_STATIC_EMPTY_STRING 0

/************************************************************************/
/* MAKEFILE                                                             */
/************************************************************************/
#define TPP_MAKEFILE_PROFILE                         TPP_PROFILE_MINIMAL
#define TPP_MAKEFILE_HAVE_USER_DEPENDENCIES          TPP_CONF_FEAT0
#define TPP_MAKEFILE_HAVE_MISSING_FILE_DEPENDENCIES  1
#define TPP_MAKEFILE_HAVE_PHONY                      TPP_CONF_FEAT0
#define TPP_MAKEFILE_HAVE_IO_HANDLE                  1
#define TPP_MAKEFILE_HAVE_OUTPUT_FILE_IO             1
#define TPP_MAKEFILE_HAVE_OUTPUT_FILE_IO_NOCLOSE     0
#define TPP_MAKEFILE_HAVE_OUTPUT_FILE                1
#define TPP_MAKEFILE_HAVE_CLI                        1
#define TPP_MAKEFILE_HAVE_CLI_HELP                   1
#define TPP_MAKEFILE_HAVE_CLI_HELP_ALL_SPELLINGS     1
#define TPP_MAKEFILE_HAVE_CLI_DASH_M                 1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MM                1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MF                1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MF_DASH           1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MG                1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MT                1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MQ                1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MD                1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MMD               1
#define TPP_MAKEFILE_HAVE_CLI_DASH_MP                1
#define TPP_MAKEFILE_HAVE_CLI_ENV_MD                 1
#define TPP_MAKEFILE_HAVE_CLI_ENV_MD_OMITS_MAIN_FILE 1
#define TPP_MAKEFILE_HAVE_CLI_ENV_MMD                1
#define TPP_MAKEFILE_DEFAULT_TARGET_FILENAME_PREFIX  "."
#define TPP_MAKEFILE_DEFAULT_TARGET_EXTENSION        ".dec"

/************************************************************************/
/* EMITTER                                                              */
/************************************************************************/
#define TPP_EMITTER_PROFILE                                 TPP_PROFILE_MINIMAL
#define TPP_EMITTER_HAVE_MODE_EMIT                          1
#define TPP_EMITTER_HAVE_MODE_DISPOSE                       1
#define TPP_EMITTER_HAVE_MODE_BRACKET                       1
#define TPP_EMITTER_HAVE_MODE_TYPED                         1
#define TPP_EMITTER_HAVE_MODE_ZERO                          1
#define TPP_EMITTER_HAVE_NORMALIZE_SPACE                    TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_LF                       TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_C_STRING                 TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_C_INT                    TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_KEYWORDS                 TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_BSE                      TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_TRIGRAPHS                TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NORMALIZE_DIGRAPHS                 TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_NOLINE                             TPP_CONF_FEAT0
#define TPP_EMITTER_HAVE_RELAXED_MACRO_COLUMN               TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_USE_CPP_DIGIT                      TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_USE_CPP_DIGIT_FLAGS                TPP_CONF_FEAT1
#define TPP_EMITTER_HAVE_USE_CPP_DIGIT_WORKING_DIRECTORY    TPP_CONF_FEAT0
#define TPP_EMITTER_HAVE_REEMIT_UNKNOWN_PRAGMA              1
#define TPP_EMITTER_HAVE_REEMIT_MACRO_DEFINITIONS           (-1)
#define TPP_EMITTER_HAVE_REEMIT_MACRO_DEFINITIONS_LAZY      TPP_CONF_FEAT0
#define TPP_EMITTER_HAVE_REEMIT_MACRO_DEFINITIONS_NAME_ONLY TPP_CONF_FEAT0
#define TPP_EMITTER_HAVE_REEMIT_INCLUDE_DIRECTIVES          (-1)
#define TPP_EMITTER_HAVE_TRACE_INCLUDES                     TPP_CONF_FEAT0
#define TPP_EMITTER_CONFIG_LINE_THRESHOLD                   (-4)

#define TPP_EMITTER_HAVE_CLI                             1
#define TPP_EMITTER_HAVE_CLI_HELP                        1
#define TPP_EMITTER_HAVE_CLI_HELP_ALL_SPELLINGS          1
#define TPP_EMITTER_HAVE_CLI_DASH_NO_LINE_COMMANDS       1
#define TPP_EMITTER_HAVE_CLI_DASH_DUMP_M                 1
#define TPP_EMITTER_HAVE_CLI_DASH_DUMP_D                 1
#define TPP_EMITTER_HAVE_CLI_DASH_DUMP_N                 1
#define TPP_EMITTER_HAVE_CLI_DASH_DUMP_I                 1
#define TPP_EMITTER_HAVE_CLI_DASH_DUMP_U                 1
#define TPP_EMITTER_HAVE_CLI_DASH_TRACE_INCLUDES         1
#define TPP_EMITTER_HAVE_CLI_DASH_FRELAXED_MACRO_COLUMN  1
#define TPP_EMITTER_HAVE_CLI_DASH_FREEMIT_UNKNOWN_PRAGMA 1
#define TPP_EMITTER_HAVE_CLI_DASH_FWORKING_DIRECTORY     1
#define TPP_EMITTER_HAVE_CLI_DASH_FUSE_CPP_DIGIT         1
#define TPP_EMITTER_HAVE_CLI_DASH_FUSE_CPP_DIGIT_FLAGS   1
#define TPP_EMITTER_HAVE_CLI_DASH_LINE_THRESHOLD         1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_SPACE       1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_LF          1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_STRINGS     1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_INT         1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_KEYWORDS    1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_BSE         1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_TRIGRAPHS   1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE_DIGRAPHS    1
#define TPP_EMITTER_HAVE_CLI_DASH_FNORMALIZE             1
#define TPP_EMITTER_HAVE_CLI_DASH_MODE_EMIT              1
#define TPP_EMITTER_HAVE_CLI_DASH_MODE_DISPOSE           1
#define TPP_EMITTER_HAVE_CLI_DASH_MODE_BRACKET           1
#define TPP_EMITTER_HAVE_CLI_DASH_MODE_TYPED             1
#define TPP_EMITTER_HAVE_CLI_DASH_MODE_ZERO              1


/* I/O Hooks */
#define tpp_io_handle DREF DeeObject * /* DeeFileObject */
#define tpp_io_getstdin(p_handle) \
	((*(p_handle) = DeeFile_GetStd(Dee_STDIN)) != NULL ? TPP_EOK : TPP_EDEEMON)
#define TPP_IO_GETSTDIN_MUST_CLOSE 1
#define tpp_io_open(filename, p_result)                                                 \
	((*(p_result) = DeeFile_OpenString(filename, Dee_OPEN_FRDONLY, 0)) == Dee_ITER_DONE \
	 ? TPP_ENOENT                                                                       \
	 : (*(p_result) ? TPP_EOK : TPP_EDEEMON))
#define tpp_io_close(handle) Dee_Decref(handle)
/* NOTE: `DeeFile_Read()` returns `(size_t)-1` on error, which just so happens to map to `TPP_EDEEMON` */
#if TPP_HAVE_FILE_NONBLOCK
#define tpp_io_read(file, buf, bufsize, nonblock) \
	((tpp_ssize)DeeFile_Readf(file, buf, bufsize, (nonblock) ? Dee_FILEIO_FNONBLOCKING : Dee_FILEIO_FNORMAL))
#else /* TPP_HAVE_FILE_NONBLOCK */
#define tpp_io_read(file, buf, bufsize) ((tpp_ssize)DeeFile_Read(file, buf, bufsize))
#endif /* !TPP_HAVE_FILE_NONBLOCK */

#define tpp_makefile_io_handle DREF DeeObject * /* DeeFileObject */
#define tpp_makefile_io_getstdout(p_handle) \
	((*(p_handle) = DeeFile_GetStd(Dee_STDOUT)) != NULL ? TPP_EOK : TPP_EDEEMON)
#define TPP_MAKEFILE_IO_GETSTDOUT_MUST_CLOSE 1
#define tpp_makefile_io_open(filename, p_result)            \
	((*(p_result) = DeeFile_OpenString(filename,            \
	                                   Dee_OPEN_FWRONLY |   \
	                                   Dee_OPEN_FCREAT |    \
	                                   Dee_OPEN_FTRUNC,     \
	                                   0)) == Dee_ITER_DONE \
	 ? TPP_ENOENT                                           \
	 : (*(p_result) ? TPP_EOK : TPP_EDEEMON))
#define tpp_makefile_io_close(handle) Dee_Decref(handle)
/* NOTE: `DeeFile_Write()` returns `(size_t)-1` on error, which just so happens to map to `TPP_ENOMEM` */
#define tpp_makefile_io_write(file, buf, bufsize) ((tpp_ssize)DeeFile_Write(file, buf, bufsize))

//TODO:tpp_io_skip_blocking
//TODO:tpp_io_withenv
//TODO:TPP_CONFIG_HAVE_LOCALTIME_R

//TODO:TPP_KWDIDENTIFIER_*
//TODO:TPP_EXTNAME_*
//TODO:TPP_HAVE_TPP_WG_*

#ifdef CONFIG_BUILDING_DEEMON
/* Pull in TPP3 headers */
/* clang-format off */
#include "../../../src/external/tpp3/src/tpp-amalgamation.h"
#include "../../../src/external/tpp3/src/tpp-makefile-amalgamation.h"
#include "../../../src/external/tpp3/src/tpp-emitter-amalgamation.h"
/* clang-format on */

#endif /* CONFIG_BUILDING_DEEMON */
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */

#ifdef CONFIG_BUILDING_DEEMON
#include "../alloc.h" /* Dee_Free */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* NULL, ptrdiff_t, size_t */
#include <stdint.h>  /* uint32_t */

#ifdef GUARD_TPP_H
#error "Don't #include `tpp.h` directly. - Deemon must configure it for itself!"
#endif /* GUARD_TPP_H */

DECL_BEGIN


#if 0 /* When defined, implement user-assembly as described
       * by `/lib/LANGUAGE.txt` for distributions lacking
       * inline-assembly support.
       *  - Still allow assembly for creation of artificial
       *    dependencies restricting the ast-based optimizer.
       *  - Cause a compiler error when the assembly text contains
       *    non-whitespace characters.
       */
#undef CONFIG_LANGUAGE_NO_ASM
#define CONFIG_LANGUAGE_NO_ASM
#endif


DFUNDEF ATTR_COLD int (DCALL Dee_BadAlloc)(size_t req_bytes);
#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define Dee_BadAlloc(req_bytes)      Dee_ASSUMED_VALUE(Dee_BadAlloc(req_bytes), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */

struct TPPFile;
struct TPPKeyword;

/* In order to prevent data redundancy of the library path (considering
 * that the way standard system include paths are now implemented as
 * `for (x: (Module from deemon).paths) yield (joinpath from fs)(x, "include");'),
 * we simply implement the unknown-file hook of TPP, allowing us to search the
 * default library path for a given filename whenever TPP couldn't find the file
 * as part of its own library path. */
INTDEF WUNUSED NONNULL((2)) struct TPPFile *DCALL
tpp_unknown_file(int mode, char *__restrict filename,
                 size_t filename_size,
                 struct TPPKeyword **p_keyword_entry);

struct Dee_file_object;

/* TPP isn't exported by deemon, so we configure it to only be used internally. */
#define TPPString_Free(x)                       Dee_Free(x)
#define TPP_assert                              Dee_ASSERT
#define TPPFUN                                  INTDEF
#define TPP(x)                                  x
#define TPPCALL                                 DCALL
#define TPP_USERDEFS                            <deemon/compiler/lexer.def>
#define TPP_CONFIG_ONELEXER                     2 /* Configure for one global lexer to speed things up. */
#define TPP_CONFIG_GCCFUNC                      0 /* Disable builtin GCC preprocessor functions. */
#define TPP_CONFIG_MINMACRO                     1 /* Enable minimal-macro mode, disabling all of those predefined C macros. */
#define TPP_CONFIG_USERSTREAMS                  1 /* Use `DeeFileObject *` as stream type for TPP. */
#define TPP_CONFIG_RAW_STRING_LITERALS          1 /* Enable support for raw string literals. */
#define TPP_USERSTREAM_TYPE                     struct Dee_file_object *
#define TPP_USERSTREAM_INVALID                  NULL
#define TPP_CONFIG_SET_API_ERROR                1 /* Get TPP to set errors on bad-alloc. */
#define TPP_CONFIG_SET_API_ERROR_BADALLOC       Dee_BadAlloc /* Get TPP to call this function on bad-alloc. */
#define TPP_CONFIG_NONBLOCKING_IO               1 /* Enable non-blocking I/O support. */
//#define TPP_CONFIG_NO_CALLBACK_PARSE_PRAGMA   1
#define TPP_CONFIG_NO_CALLBACK_PARSE_PRAGMA_GCC 1
#define TPP_CONFIG_NO_CALLBACK_INS_COMMENT      1
#define TPP_CONFIG_NO_CALLBACK_NEW_TEXTFILE     1
#define TPP_CONFIG_CALLBACK_UNKNOWN_FILE        tpp_unknown_file /* Statically link our unknown-file callback. */
#define TPP_CONFIG_CALLBACK_WARNING(...)        (parser_warnf(__VA_ARGS__) == 0)
#define TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS    1
#define TPP_CONFIG_USERDEFINED_KWD_DEFAULT      1
#define TPP_CONFIG_USERDEFINED_KWD_ASSERT       1
#define TPP_CONFIG_USERDEFINED_KWD_IMPORT       1
#define TPP_CONFIG_USERDEFINED_KWD_IF           1


/* Configure non-variable TPP options. */
/*      TPP_CONFIG_FEATURE_TRIGRAPHS           0 */
/*      TPP_CONFIG_FEATURE_DIGRAPHS            0 */
#define TPP_CONFIG_EXTENSION_GCC_VA_ARGS       1
#define TPP_CONFIG_EXTENSION_GCC_VA_COMMA      1
#define TPP_CONFIG_EXTENSION_GCC_IFELSE        1
#define TPP_CONFIG_EXTENSION_VA_COMMA          1
#define TPP_CONFIG_EXTENSION_VA_NARGS          1
#define TPP_CONFIG_EXTENSION_VA_ARGS           1
#define TPP_CONFIG_EXTENSION_VA_OPT            1
#define TPP_CONFIG_EXTENSION_STR_E             1
#define TPP_CONFIG_EXTENSION_ALTMAC            1
/*      TPP_CONFIG_EXTENSION_RECMAC            0 */
#define TPP_CONFIG_EXTENSION_BININTEGRAL       1
/*      TPP_CONFIG_EXTENSION_MSVC_PRAGMA       0 */
#define TPP_CONFIG_EXTENSION_STRINGOPS         1
#define TPP_CONFIG_EXTENSION_HASH_AT           1
#define TPP_CONFIG_EXTENSION_HASH_XCLAIM       1
#define TPP_CONFIG_EXTENSION_WARNING           1
#define TPP_CONFIG_EXTENSION_SHEBANG           1
#define TPP_CONFIG_EXTENSION_INCLUDE_NEXT      1
#define TPP_CONFIG_EXTENSION_IMPORT            1
#define TPP_CONFIG_EXTENSION_IDENT_SCCS        0
#define TPP_CONFIG_EXTENSION_BASEFILE          1
#define TPP_CONFIG_EXTENSION_INCLUDE_LEVEL     1
#define TPP_CONFIG_EXTENSION_COUNTER           1
#define TPP_CONFIG_EXTENSION_CLANG_FEATURES    1
#define TPP_CONFIG_EXTENSION_HAS_INCLUDE       1
#define TPP_CONFIG_EXTENSION_LXOR              0
#define TPP_CONFIG_EXTENSION_MULTICHAR_CONST   1
#define TPP_CONFIG_EXTENSION_DATEUTILS         1
#define TPP_CONFIG_EXTENSION_TIMEUTILS         1
#define TPP_CONFIG_EXTENSION_TIMESTAMP         1
#define TPP_CONFIG_EXTENSION_COLUMN            1
#define TPP_CONFIG_EXTENSION_TPP_EVAL          1
#define TPP_CONFIG_EXTENSION_TPP_UNIQUE        1
#define TPP_CONFIG_EXTENSION_TPP_LOAD_FILE     1
#define TPP_CONFIG_EXTENSION_TPP_COUNTER       1
#define TPP_CONFIG_EXTENSION_TPP_RANDOM        1
#define TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE 1
#define TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR    1
#define TPP_CONFIG_EXTENSION_TPP_STR_SIZE      1
#define TPP_CONFIG_EXTENSION_TPP_STR_PACK      1
#define TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS  1
/*      TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA   0 */
#define TPP_CONFIG_EXTENSION_ASSERTIONS        1
/*      TPP_CONFIG_EXTENSION_CANONICAL_HEADERS 0 */
/*      TPP_CONFIG_EXTENSION_EXT_ARE_FEATURES  0 */
#define TPP_CONFIG_EXTENSION_MSVC_FIXED_INT_DEFAULT 0 /* Default to disabled. */
/*      TPP_CONFIG_EXTENSION_NO_EXPAND_DEFINED 0 */
#define TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR    1
/*      TPP_CONFIG_EXTENSION_EXTENDED_IDENTS   0 */
/*      TPP_CONFIG_EXTENSION_TRADITIONAL_MACRO 0 */

DECL_END

/* TODO: Use `DeeStringObject *` for `struct TPPString` */
/* TODO: Use `DeeObject *` (String/Int) for `struct TPPConst` */
/* clang-format off */
#include <hybrid/typecore.h> /* Needed for better integration of tpp */
/* clang-format on */
#define TPP_NO_INCLUDE_STDLIB_H 1
#include "../../../src/tpp/src/tpp.h"

/* Forward-compatibility with TPP3 */
#include "../../../src/external/tpp3/src/tpp2-forward.h"
#else /* CONFIG_BUILDING_DEEMON */

DECL_BEGIN
struct TPPKeyword;
DECL_END

#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

#ifdef CONFIG_BUILDING_DEEMON
DECL_BEGIN

/************************************************************************/
/* Deemon wrapper for TPP lexer object                                  */
/************************************************************************/

typedef struct {
	tpp_lexer dl_lexer; /* TPP lexer */
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	/* TODO: Encountered warnings (to include in `DeeLexer_TPP_RaiseLexErrorHook`) */
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */
} DeeLexer;

#define DeeLexer_FromTPP(p_tpp_lexer) COMPILER_CONTAINER_OF(p_tpp_lexer, DeeLexer, dl_lexer)

#define DeeLexer_Has(self, feat)                 tpp_lexer_has(&(self)->dl_lexer, feat)
#define DeeLexer_GetTok(self)                    tpp_lexer_gettok(&(self)->dl_lexer)
#define DeeLexer_GetToken(self)                  tpp_lexer_gettoken(&(self)->dl_lexer)
#define DeeLexer_GetFile(self)                   tpp_lexer_getfile(&(self)->dl_lexer)
#define DeeLexer_HasTokenKwd(self)               tpp_lexer_hastokenkwd(&(self)->dl_lexer)
#define DeeLexer_GetTokenKwd(self)               tpp_lexer_gettokenkwd(&(self)->dl_lexer)
#define DeeLexer_GetTokenKwdCStr(self)           tpp_lexer_gettokenkwdcstr(&(self)->dl_lexer)
#define DeeLexer_GetTokenKwdStr(self)            tpp_lexer_gettokenkwdstr(&(self)->dl_lexer)
#define DeeLexer_GetTokenKwdLen(self)            tpp_lexer_gettokenkwdlen(&(self)->dl_lexer)
#define DeeLexer_GetTokenStart(self)             tpp_lexer_gettokenstart(&(self)->dl_lexer)
#define DeeLexer_GetTokenEnd(self)               tpp_lexer_gettokenend(&(self)->dl_lexer)
#define DeeLexer_GetTokenLen(self)               tpp_lexer_gettokenlen(&(self)->dl_lexer)
#define DeeLexer_SetTokenId(self, id)            tpp_lexer_settokenid(&(self)->dl_lexer, id)
#define DeeLexer_SetTokenRange(self, start, end) tpp_lexer_settokenrange(&(self)->dl_lexer, start, end)
#define DeeLexer_SetTokenEnd(self, end)          tpp_lexer_settokenend(&(self)->dl_lexer, end)

#define DeeLexer_YieldRaw(self)                    tpp_lexer_yieldraw_blocking(&(self)->dl_lexer)
#define DeeLexer_YieldPP(self)                     tpp_lexer_yieldpp_blocking(&(self)->dl_lexer)
#define DeeLexer_Yield(self)                       tpp_lexer_yield_blocking(&(self)->dl_lexer)
#define DeeLexer_YieldRawNB(self)                  tpp_lexer_yieldraw(&(self)->dl_lexer)
#define DeeLexer_YieldPPNB(self)                   tpp_lexer_yieldpp(&(self)->dl_lexer)
#define DeeLexer_YieldNB(self)                     tpp_lexer_yield(&(self)->dl_lexer)
#define DeeLexer_YieldRawXNB(self, allow_nonblock) ((allow_nonblock) ? DeeLexer_YieldRawNB(self) : DeeLexer_YieldRaw(self))
#define DeeLexer_YieldPPXNB(self, allow_nonblock)  ((allow_nonblock) ? DeeLexer_YieldPPNB(self) : DeeLexer_YieldPP(self))
#define DeeLexer_YieldXNB(self, allow_nonblock)    ((allow_nonblock) ? DeeLexer_YieldNB(self) : DeeLexer_Yield(self))

#define DeeLexer_PreparseSkipBseFwd(self, pos, end)   tpp_preparse_skipbse_fwd(&(self)->dl_lexer, pos, end)
#define DeeLexer_PreparseSkipBseBck(self, start, pos) tpp_preparse_skipbse_bck(&(self)->dl_lexer, start, pos)


/* Helper to check if the current token should be considered a string token */
#define DeeLexer_IsStringToken(self)                    \
	(TPP_TOK_ISSTRING_DQUOTE(DeeLexer_GetTok(self)) ||  \
	 (TPP_TOK_ISSTRING_SQUOTE(DeeLexer_GetTok(self)) && \
	  !DeeLexer_Has(self, CHARACTER_LITERALS)))



struct ast;
struct ast_loc {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	char const *l_name; /* [0..1] Filename (either statically allocated, or points into a `tpp_keyword`.
	                     * In either case, this string remains valid until `tpp_lexer_fini()` is called)
	                     * When user-code made use of a custom `#line` filename, then a keyword for that
	                     * filename is lazily allocated here (set to "NULL" if unknown) */
	tpp_lcinfo  l_lc;   /* Line/column information (0-based) */
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	struct TPPFile      *l_file; /* [0..1] Location file. */
#ifdef CONFIG_BUILDING_DEEMON
	union {
		struct TPPLCInfo l_lc;   /* [valid_if(l_file != NULL)] Line/column information. */
		struct {
			int          l_line; /* [valid_if(l_file != NULL)] Location line. */
			int          l_col;  /* [valid_if(l_file != NULL)] Location column. */
		}
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
		_dee_astruct
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
		;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define l_lc       _dee_aunion.l_lc /*!export-*/
#ifdef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define l_line     _dee_aunion.l_line /*!export-*/
#define l_col      _dee_aunion.l_col  /*!export-*/
#else /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#define l_line     _dee_aunion._dee_astruct.l_line /*!export-*/
#define l_col      _dee_aunion._dee_astruct.l_col  /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#elif !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT)
#define l_line     _dee_astruct.l_line /*!export-*/
#define l_col      _dee_astruct.l_col  /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
	;
#else /* CONFIG_BUILDING_DEEMON */
	int                  l_line; /* [valid_if(l_file != NULL)] Location line. */
	int                  l_col;  /* [valid_if(l_file != NULL)] Location column. */
#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
};

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
#define ast_loc_init_empty(self) \
	((self)->l_name = NULL, tpp_lcinfo_init_invalid(&(self)->l_lc))
#define ast_loc_getname(self) ((self)->l_name)
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
#define ast_loc_init_empty(self) (void)((self)->l_file = NULL)
#define ast_loc_getname(self) tpp_file_getfilename((self)->l_file)
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
#define ast_loc_getlc(self)   ((self)->l_lc)
#define ast_loc_getline(self) tpp_lcinfo_getline((self)->l_lc)
#define ast_loc_getcol(self)  tpp_lcinfo_getcol((self)->l_lc)


/* Fill the given AST location with the current LC source position.
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DFCALL
DeeLexer_GetLoc(DeeLexer *self, struct ast_loc *__restrict info);


#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
INTDEF NONNULL((1)) void DFCALL loc_here(struct ast_loc *__restrict info);

/* Helper to transition into a world where this gets passed along the stack */
#define _DeeLexer_Current DeeLexer_FromTPP(TPPLexer_Current)

/* Describe a region of code where `TPPLEXER_FLAG_WANTLF` should be off. */
#define DeeLexer_NoLf_Push(self)                                 \
	do {                                                         \
		uint32_t const _dlnlf_oflags = (self)->dl_lexer.l_flags; \
		(self)->dl_lexer.l_flags &= ~TPPLEXER_FLAG_WANTLF
#define DeeLexer_NoLf_Break(self) \
		(void)((self)->dl_lexer.l_flags |= _dlnlf_oflags & TPPLEXER_FLAG_WANTLF)
#define DeeLexer_NoLf_Pop(self)    \
		DeeLexer_NoLf_Break(self); \
	}	__WHILE0


/* Describe a region of code where `TPPLEXER_FLAG_WANTLF` is enabled if `PARSE_FLFSTMT` is set */
#define DeeLexer_EnableLf_Push(self)                             \
	do {                                                         \
		uint32_t const _dlelf_oflags = (self)->dl_lexer.l_flags; \
		if (!(parser_flags & PARSE_FLFSTMT)) {                   \
		} else                                                   \
			(self)->dl_lexer.l_flags |= TPPLEXER_FLAG_WANTLF
#define DeeLexer_EnableLf_Break(self) \
		(void)((self)->dl_lexer.l_flags &= _dlelf_oflags & ~TPPLEXER_FLAG_WANTLF)
#define DeeLexer_EnableLf_Pop(self)    \
		DeeLexer_EnableLf_Break(self); \
	}	__WHILE0


/* Emit a compiler warning/error, given its TPP warning number.
 * The passed var-args are interpreted based on `wnum`,
 * which is one of `W_*` defined by the lexer.
 * @return: -1: TPP had already been set to an error-state.
 * @return: -1: A fatal compiler error was thrown and TPP was set to an error-state.
 * @return:  0: The warning is being ignored.
 * @return:  0: The warning was printed, but is not considered dangerous.
 * @return:  0: The warning caused an error to be thrown, but the
 *              max number of compiler errors has yet to be reached. */
INTDEF ATTR_COLD int (parser_warnf)(int wnum, ...);
INTDEF ATTR_COLD int (DCALL parser_vwarnf)(int wnum, va_list args);
INTDEF ATTR_COLD int (parser_warnatf)(struct ast_loc *loc, int wnum, ...);
INTDEF ATTR_COLD int (parser_warnatrf)(struct ast_loc *loc, int wnum, ...); /* file from `loc` is guarantied to be reachable! */
INTDEF ATTR_COLD int (parser_warnastf)(struct ast *__restrict loc_ast, int wnum, ...);
INTDEF ATTR_COLD int (parser_warnatptrf)(char const *ptr, int wnum, ...);

/* Similar to `parser_warnf()`, but force the warning
 * to be fatal, regardless of its user-defined state.
 * @return: -1: Always returns -1. */
INTDEF ATTR_COLD int (parser_errf)(int wnum, ...);
INTDEF ATTR_COLD int (parser_erratf)(struct ast_loc *loc, int wnum, ...);
INTDEF ATTR_COLD int (parser_erratrf)(struct ast_loc *loc, int wnum, ...); /* file from `loc` is guarantied to be reachable! */
INTDEF ATTR_COLD int (parser_errastf)(struct ast *__restrict loc_ast, int wnum, ...);

DFUNDEF ATTR_COLD int (DCALL Dee_BadAlloc)(size_t req_bytes);

#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define parser_errf(...)             Dee_ASSUMED_VALUE(parser_errf(__VA_ARGS__), -1)
#define parser_erratf(loc, ...)      Dee_ASSUMED_VALUE(parser_erratf(loc, __VA_ARGS__), -1)
#define parser_erratrf(loc, ...)     Dee_ASSUMED_VALUE(parser_erratrf(loc, __VA_ARGS__), -1)
#define parser_errastf(loc_ast, ...) Dee_ASSUMED_VALUE(parser_errastf(loc_ast, __VA_ARGS__), -1)
#define Dee_BadAlloc(req_bytes)      Dee_ASSUMED_VALUE(Dee_BadAlloc(req_bytes), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */

/* Warn about use of `pack` (but only if we're not currently inside of a macro) */
INTDEF WUNUSED int DCALL parser_warn_pack_used(struct ast_loc *loc);


//#define DeeLexer_Skip(self, tid) tpp_lexer_skip(&(self)->dl_lexer, tid)
#define DeeLexer_VWarnf(self, id, args)             ((void)(self), parser_vwarnf(id, args))
#define DeeLexer_Warnf(self, ...)                   ((void)(self), parser_warnf(__VA_ARGS__))
//#define DeeLexer_VWarnfAt(self, file, pos, args)    ((void)(self), ...)
#define DeeLexer_WarnfAt(self, file, pos, ...)      ((void)(self), (void)(file), parser_warnatptrf(pos, __VA_ARGS__))
//#define DeeLexer_VWarnfLc(self, filename, lc, args) ((void)(self), ...)
//#define DeeLexer_WarnfLc(self, filename, lc, ...)   ((void)(self), ...)


INTDEF WUNUSED NONNULL((1)) int DFCALL
_parser_skip(DeeLexer *self, tpp_token_id expected_tok, int wnum);
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
_parser_paren_begin(DeeLexer *self, bool *__restrict p_has_paren, int wnum);
#define DeeLexer_ParenBegin2(self, p_has_paren, W_EXPECTED_LPAREN)  \
	(likely(DeeLexer_GetTok(self) == '(')                           \
	 ? (*(p_has_paren) = true, TPP_TOK_ISERR(DeeLexer_Yield(self))) \
	 : unlikely(_parser_paren_begin(self, p_has_paren, W_EXPECTED_LPAREN)))
#define DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN) \
	(likely(has_paren) && DeeLexer_Skip2(self, ')', W_EXPECTED_RPAREN))
#define DeeLexer_Skip2(self, expected_tok, W_UNEXPECTED_TOKEN) \
	(likely(DeeLexer_GetTok(self) == (expected_tok))           \
	 ? TPP_TOK_ISERR(DeeLexer_Yield(self))                     \
	 : unlikely(_parser_skip(self, expected_tok, W_UNEXPECTED_TOKEN)))

#define WARN(...)         parser_warnf(__VA_ARGS__)
#define WARNAT(loc, ...)  parser_warnatf(loc, __VA_ARGS__)
#define WARNSYM(sym, ...) parser_warnatrf(&(sym)->s_decl, __VA_ARGS__)
#define WARNAST(ast, ...) parser_warnastf(ast, __VA_ARGS__)
#define PERRAT(loc, ...)  parser_erratf(loc, __VA_ARGS__)
#define PERRAST(ast, ...) parser_errastf(ast, __VA_ARGS__)

INTDEF struct TPPKeyword TPPKeyword_Empty;
INTDEF WUNUSED char const *DCALL peek_next_token(struct TPPFile **tok_file);
INTDEF WUNUSED NONNULL((1)) char const *DCALL peek_next_advance(char const *p, struct TPPFile **tok_file);
INTDEF ATTR_CONST WUNUSED bool DCALL tpp_is_keyword_start(char ch);
INTDEF WUNUSED NONNULL((1, 2)) struct TPPKeyword *DCALL peek_keyword(struct TPPFile *__restrict tok_file, char const *__restrict tok_begin, int create_missing);
INTDEF WUNUSED struct TPPKeyword *DCALL peek_next_keyword(int create_missing);
INTDEF WUNUSED NONNULL((1)) char const *DCALL advance_wraplf(char const *__restrict p);
INTDEF WUNUSED NONNULL((1)) bool DCALL tpp_is_reachable_file(struct TPPFile *__restrict file);

#else /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
#define DeeLexer_Init(self) (tpp_lexer_init(&(self)->dl_lexer))
#define DeeLexer_Fini(self) (tpp_lexer_fini(&(self)->dl_lexer))

/* Describe a region of code where `TPP_TOK_LF` should not be produced. */
#define DeeLexer_NoLf_Push(self)                                                            \
	do {                                                                                    \
		bool const _dlnlf_olf = !!tpp_lexer_getfeature(&(self)->dl_lexer, TPP_FEAT_TOK_LF); \
		tpp_lexer_disablefeature(&(self)->dl_lexer, TPP_FEAT_TOK_LF)
#define DeeLexer_NoLf_Break(self) \
		tpp_lexer_setfeature(&(self)->dl_lexer, TPP_FEAT_TOK_LF, _dlnlf_olf)
#define DeeLexer_NoLf_Pop(self)    \
		DeeLexer_NoLf_Break(self); \
	}	__WHILE0

/* Describe a region of code where `TPP_TOK_LF` is enabled if `PARSE_FLFSTMT` is set */
#define DeeLexer_EnableLf_Push(self)                             \
	do {                                                         \
		bool const _dlelf_olf = !!tpp_lexer_getfeature(&(self)->dl_lexer, TPP_FEAT_TOK_LF); \
		if (!(parser_flags & PARSE_FLFSTMT)) {                   \
		} else                                                   \
			tpp_lexer_enablefeature(&(self)->dl_lexer, TPP_FEAT_TOK_LF)
#define DeeLexer_EnableLf_Break(self) \
		tpp_lexer_setfeature(&(self)->dl_lexer, TPP_FEAT_TOK_LF, _dlelf_olf)
#define DeeLexer_EnableLf_Pop(self)    \
		DeeLexer_EnableLf_Break(self); \
	}	__WHILE0



#define DeeLexer_VWarnf(self, id, args)             TPP_ISERR(tpp_lexer_vwarnf(&(self)->dl_lexer, id, args))
#define DeeLexer_Warnf(self, ...)                   TPP_ISERR(tpp_lexer_warnf(&(self)->dl_lexer, __VA_ARGS__))
#define DeeLexer_VWarnfAt(self, file, pos, args)    TPP_ISERR(tpp_lexer_vwarnf_at(&(self)->dl_lexer, file, pos, args))
#define DeeLexer_WarnfAt(self, file, pos, ...)      TPP_ISERR(tpp_lexer_warnf_at(&(self)->dl_lexer, file, pos, __VA_ARGS__))
#define DeeLexer_VWarnfLc(self, filename, lc, args) TPP_ISERR(tpp_lexer_vwarnf_lc(&(self)->dl_lexer, filename, lc, args))
#define DeeLexer_WarnfLc(self, filename, lc, ...)   TPP_ISERR(tpp_lexer_warnf_lc(&(self)->dl_lexer, filename, lc, __VA_ARGS__))

/* Static TPP Hooks */
INTDEF tpp_errno TPPCALL DeeLexer_TPP_WarnHandlerHook(tpp_lexer *lexer, struct tpp_lexer_printf_info *tpp_restrict info, tpp_warning_invokeinfo const *tpp_restrict invokeinfo, tpp_warning_id id, va_list args);
INTDEF Dee_ssize_t TPPCALL DeeLexer_TPP_WarnPrinterHook(void *arg, char const *__restrict text, size_t num_bytes);
INTDEF Dee_ssize_t TPPCALL DeeLexer_TPP_MesgPrinterHook(void *arg, char const *__restrict text, size_t num_bytes);
INTDEF tpp_errno TPPCALL DeeLexer_TPP_SystemIncludePathHook(tpp_lexer *lexer, tpp_token_id mode, tpp_hook_system_include_path_when when, tpp_errno (TPPCALL *cb)(void *arg, char const *relative_to tpp_lexer_foreach_include_path_flags__PARAM), void *arg);
INTDEF tpp_errno TPPCALL DeeLexer_TPP_RaiseLexErrorHook(tpp_lexer *lexer);


#define DeeLexer_Skip(self, expected_tok) \
	tpp_lexer_skip(&(self)->dl_lexer, expected_tok)
INTDEF WUNUSED NONNULL((1, 2)) int DFCALL
_DeeLexer_ParenBegin(DeeLexer *self, bool *__restrict p_has_paren);
#define DeeLexer_ParenBegin(self, p_has_paren)   \
	(likely(DeeLexer_GetTok(self) == TPP_TOK_OFCHAR('('))           \
	 ? (*(p_has_paren) = true, TPP_TOK_ISERR(DeeLexer_Yield(self))) \
	 : unlikely(_DeeLexer_ParenBegin(self, p_has_paren)))
#define DeeLexer_ParenEnd(self, has_paren) \
	(likely(has_paren) && TPP_TOK_ISERR(DeeLexer_Skip(self, TPP_TOK_OFCHAR(')'))))

/* Backwards compat... */
#define DeeLexer_Skip2(self, expected_tok, W_UNEXPECTED_TOKEN)     DeeLexer_Skip(self, expected_tok)
#define DeeLexer_ParenBegin2(self, p_has_paren, W_EXPECTED_LPAREN) DeeLexer_ParenBegin(self, p_has_paren)
#define DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN)     DeeLexer_ParenEnd(self, has_paren)
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_TPP_H */
