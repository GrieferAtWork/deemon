/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
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
#include <deemon/object.h>
#include <deemon/int.h>

DECL_BEGIN

#if defined(CONFIG_NO_CACHES) || \
    defined(CONFIG_NO_INT_CACHES)
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
#endif


#ifdef NDEBUG
INTDEF DREF DeeIntObject *DCALL DeeInt_Alloc(size_t n_digits);
#else
INTDEF DREF DeeIntObject *DCALL DeeInt_Alloc_dbg(size_t n_digits, char const *file, int line);
#define DeeInt_Alloc(n_digits)  DeeInt_Alloc_dbg(n_digits,__FILE__,__LINE__)
#endif
#if CONFIG_INT_CACHE_MAXCOUNT != 0
INTDEF void DCALL DeeInt_Free(DeeIntObject *__restrict self);
#else
#define DeeInt_Free(self) DeeObject_Free(self)
#endif

INTDEF DREF DeeIntObject *DCALL int_copy(DeeIntObject const *__restrict self);
INTDEF DREF DeeIntObject *DCALL int_normalize(/*inherit(always)*/DREF DeeIntObject *__restrict v);
INTDEF DREF DeeObject *DCALL int_add(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_sub(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_mul(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_div(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_mod(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_inv(DeeIntObject *__restrict v);
INTDEF DREF DeeObject *DCALL int_neg(DeeIntObject *__restrict v);
INTDEF DREF DeeObject *DCALL int_shl(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_shr(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_and(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_xor(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_or(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
INTDEF DREF DeeObject *DCALL int_pow(DeeIntObject *__restrict a, DeeIntObject *__restrict b);

INTDEF int DCALL int_inc(DREF DeeIntObject **__restrict pself);
INTDEF int DCALL int_dec(DREF DeeIntObject **__restrict pself);

INTDEF DREF DeeObject *DCALL DeeInt_AddSDigit(DeeIntObject *__restrict a, sdigit b);
INTDEF DREF DeeObject *DCALL DeeInt_SubSDigit(DeeIntObject *__restrict a, sdigit b);
INTDEF DREF DeeObject *DCALL DeeInt_AddU32(DeeIntObject *__restrict a, uint32_t b);
INTDEF DREF DeeObject *DCALL DeeInt_SubU32(DeeIntObject *__restrict a, uint32_t b);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_LOGIC_H */
