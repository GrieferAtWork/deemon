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
/* NOTE: Deemon's integer object implementation is
 *       heavily based on python's `long' data type.
 *       With that in mind, licensing of deemon's integer
 *       implementation must be GPL-compatible, GPL being
 *       the license that python is restricted by.
 *    >> So to simplify this whole deal: I make no claim of having invented the
 *       way that deemon's (phyton's) arbitrary-length integers are implemented,
 *       with all algorithms found in `int_logic.c' originating from phython
 *       before being adjusted to fit deemon's runtime. */
#ifndef GUARD_DEEMON_OBJECTS_INT_LOGIC_H
#define GUARD_DEEMON_OBJECTS_INT_LOGIC_H 1

#include <deemon/api.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* memsetw/memsetl/... */

DECL_BEGIN

#if Dee_SIZEOF_DIGIT == 2
#define Dee_digit_memset(p, v, n) memsetw(p, v, n)
#elif Dee_SIZEOF_DIGIT == 4
#define Dee_digit_memset(p, v, n) memsetl(p, v, n)
#elif Dee_SIZEOF_DIGIT == 1
#define Dee_digit_memset(p, v, n) memset(p, v, n)
#elif Dee_SIZEOF_DIGIT == 8
#define Dee_digit_memset(p, v, n) memsetq(p, v, n)
#else /* Dee_SIZEOF_DIGIT == ... */
#define Dee_digit_memset(p, v, n)    \
	do {                             \
		size_t _i;                   \
		for (_i = 0; _i < (n); ++_i) \
			(p)[_i] = (v);           \
	}	__WHILE0
#endif /* Dee_SIZEOF_DIGIT != ... */



#if defined(CONFIG_NO_CACHES) || defined(CONFIG_NO_INT_CACHES)
#undef CONFIG_INT_CACHE_MAXSIZE
#define CONFIG_INT_CACHE_MAXSIZE  0
#undef CONFIG_INT_CACHE_MAXCOUNT
#define CONFIG_INT_CACHE_MAXCOUNT 0
#endif

/* The max amount of integers per cache */
#ifndef CONFIG_INT_CACHE_MAXSIZE
#define CONFIG_INT_CACHE_MAXSIZE   64
#endif

/* The max number of digits for which a cache is kept */
#ifndef CONFIG_INT_CACHE_MAXCOUNT
/* The max number of integer bits for which a dedicated cache must exist */
#ifndef CONFIG_INT_CACHE_BITCOUNT
#define CONFIG_INT_CACHE_BITCOUNT    64
#endif /* !CONFIG_INT_CACHE_BITCOUNT */
#define CONFIG_INT_CACHE_MAXCOUNT  ((CONFIG_INT_CACHE_BITCOUNT + (DIGIT_BITS-1)) / DIGIT_BITS)
#endif /* !CONFIG_INT_CACHE_MAXCOUNT */

#if !CONFIG_INT_CACHE_MAXSIZE
#undef CONFIG_INT_CACHE_MAXCOUNT
#define CONFIG_INT_CACHE_MAXCOUNT 0
#endif /* !CONFIG_INT_CACHE_MAXSIZE */


#ifdef NDEBUG
INTDEF WUNUSED DREF DeeIntObject *DCALL DeeInt_Alloc(size_t n_digits);
#else /* NDEBUG */
INTDEF WUNUSED DREF DeeIntObject *DCALL DeeInt_Alloc_d(size_t n_digits, char const *file, int line);
#define DeeInt_Alloc(n_digits) DeeInt_Alloc_d(n_digits, __FILE__, __LINE__)
#endif /* !NDEBUG */
#if CONFIG_INT_CACHE_MAXCOUNT != 0
INTDEF NONNULL((1)) void DCALL DeeInt_Free(DeeIntObject *__restrict self);
#else /* CONFIG_INT_CACHE_MAXCOUNT != 0 */
#define DeeInt_Free(self) DeeObject_Free(self)
#endif /* CONFIG_INT_CACHE_MAXCOUNT == 0 */
#define DeeInt_Destroy(self) (DeeInt_Free(self), Dee_DecrefNokill(&DeeInt_Type))

INTDEF WUNUSED NONNULL((1, 2)) int DCALL int_divmod(DeeIntObject *a, DeeIntObject *b, DeeIntObject **p_div, DeeIntObject **p_rem);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_add(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_sub(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_mul(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_div(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_mod(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_inv(DeeIntObject *__restrict v);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_neg(DeeIntObject *__restrict v);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_shl(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_shr(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_and(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_xor(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_or(DeeIntObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_pow(DeeIntObject *a, DeeObject *b);

INTDEF WUNUSED NONNULL((1)) int DCALL int_inc(DREF DeeIntObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL int_dec(DREF DeeIntObject **__restrict p_self);

INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_AddSDigit(DeeIntObject *__restrict a, sdigit b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_SubSDigit(DeeIntObject *__restrict a, sdigit b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_AddUInt32(DeeIntObject *__restrict a, uint32_t b);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL DeeInt_SubUInt32(DeeIntObject *__restrict a, uint32_t b);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_pext(DeeIntObject *self, DeeIntObject *mask);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL int_pdep(DeeIntObject *self, DeeIntObject *mask);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_LOGIC_H */
