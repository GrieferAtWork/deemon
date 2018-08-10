/* Copyright (c) 2018 Griefer@Work                                            *
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

#ifdef NDEBUG
INTDEF DREF DeeIntObject *DCALL DeeInt_Alloc(size_t n_digits);
#else
INTDEF DREF DeeIntObject *DCALL DeeInt_Alloc_dbg(size_t n_digits, char const *file, int line);
#define DeeInt_Alloc(n_digits)  DeeInt_Alloc_dbg(n_digits,__FILE__,__LINE__)
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

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_LOGIC_H */
