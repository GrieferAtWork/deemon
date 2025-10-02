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
#ifndef GUARD_DEEMON_ERROR_RT_H
#define GUARD_DEEMON_ERROR_RT_H 1

#include "api.h"
/**/

#include "types.h"
/**/

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* int64_t, uint64_t */

DECL_BEGIN

/************************************************************************/
/* Runtime error throwing helpers                                       */
/************************************************************************/


/* Throws a `DeeError_RuntimeError' indicating that there is no active
 * exception. Thrown by user-code "throw;" (re-throw exception) statement
 * when there is no active exception. */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNoActiveException)(void);


/* Throws a `DeeError_IntegerOverflow' indicating that some an integer
 * object or native (C) value cannot be used/processed because its value
 * exceeds the maximum supported value bounds within some context-of-use.
 *
 * The unsigned overflow throwing functions will only take the upper
 * bound (greatest) of valid values, and assume that the lower bound
 * is equal to `0'
 *
 * @param: positive: When true, assume "value > maxval".
 *                   Else, assume "value < maxval" */
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrIntegerOverflow)(/*Numeric*/ DeeObject *value,
                                 /*Numeric*/ DeeObject *minval,
                                 /*Numeric*/ DeeObject *maxval,
                                 bool positive);

/* Same as "DeeRT_ErrIntegerOverflow", but minval/maxval are set as
 * >> minval   = (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) ? -(1 << (num_bits - 1)) : 0;
 * >> maxval   = (flags & DeeRT_ErrIntegerOverflowEx_F_SIGNED) ? ((1 << (num_bits - 1)) - 1) : ((1 << num_bits) - 1);
 * >> positive = (flags & DeeRT_ErrIntegerOverflowEx_F_POSITIVE) != 0; */
DFUNDEF ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIntegerOverflowEx)(/*Numeric*/ DeeObject *value,
                                   size_t num_bits,
                                   unsigned int flags);
#define DeeRT_ErrIntegerOverflowEx_F_NEGATIVE 0 /* Given "value" is negative (and less than "minval") */
#define DeeRT_ErrIntegerOverflowEx_F_POSITIVE 1 /* Given "value" is positive (and greater than "maxval") */
#define DeeRT_ErrIntegerOverflowEx_F_UNSIGNED 0 /* Wanted to case to "uint{num_bits}_t" */
#define DeeRT_ErrIntegerOverflowEx_F_SIGNED   2 /* Wanted to case to "int{num_bits}_t" */
#define DeeRT_ErrIntegerOverflowEx_F_ANYSIGN  4 /* Wanted to case to "int{num_bits}_t" or "uint{num_bits}_t" */

DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowS)(Dee_ssize_t value, Dee_ssize_t minval, Dee_ssize_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowU)(size_t value, size_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowUMul)(size_t lhs, size_t rhs);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowUAdd)(size_t lhs, size_t rhs);
#if __SIZEOF_SIZE_T__ >= 8
#define DeeRT_ErrIntegerOverflowS64(value, minval, maxval) DeeRT_ErrIntegerOverflowS(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU64(value, maxval)         DeeRT_ErrIntegerOverflowU(value, maxval)
#else  /* __SIZEOF_SIZE_T__ >= 8 */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowS64)(int64_t value, int64_t minval, int64_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowU64)(uint64_t value, uint64_t maxval);
#endif /* __SIZEOF_SIZE_T__ < 8 */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowS128)(Dee_int128_t value, Dee_int128_t minval, Dee_int128_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowU128)(Dee_uint128_t value, Dee_uint128_t maxval);
#define DeeRT_ErrIntegerOverflowS8(value, minval, maxval) DeeRT_ErrIntegerOverflowS(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU8(value, maxval)         DeeRT_ErrIntegerOverflowU(value, maxval)
#if __SIZEOF_SIZE_T__ >= 2
#define DeeRT_ErrIntegerOverflowS16(value, minval, maxval) DeeRT_ErrIntegerOverflowS(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU16(value, maxval)         DeeRT_ErrIntegerOverflowU(value, maxval)
#else /* __SIZEOF_SIZE_T__ >= 2 */
#define DeeRT_ErrIntegerOverflowS16(value, minval, maxval) DeeRT_ErrIntegerOverflowS64(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU16(value, maxval)         DeeRT_ErrIntegerOverflowU64(value, maxval)
#endif /* __SIZEOF_SIZE_T__ < 2 */
#if __SIZEOF_SIZE_T__ >= 4
#define DeeRT_ErrIntegerOverflowS32(value, minval, maxval) DeeRT_ErrIntegerOverflowS(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU32(value, maxval)         DeeRT_ErrIntegerOverflowU(value, maxval)
#else /* __SIZEOF_SIZE_T__ >= 4 */
#define DeeRT_ErrIntegerOverflowS32(value, minval, maxval) DeeRT_ErrIntegerOverflowS64(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU32(value, maxval)         DeeRT_ErrIntegerOverflowU64(value, maxval)
#endif /* __SIZEOF_SIZE_T__ < 4 */


/* Throws an `DeeError_UnboundItem' indicating that a given index/key is unbound */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKey)(DeeObject *seq, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnboundKeyInt)(DeeObject *seq, size_t key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKeyStr)(DeeObject *seq, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKeyStrLen)(DeeObject *seq, char const *key, size_t keylen);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnboundIndex)(DeeObject *seq, size_t index);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundIndexObj)(DeeObject *seq, DeeObject *index);







#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define DeeRT_ErrNoActiveException()                              Dee_ASSUMED_VALUE((DeeRT_ErrNoActiveException)(), -1)
#define DeeRT_ErrIntegerOverflow(value, minval, maxval, positive) Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflow)(value, minval, maxval, positive), -1)
#define DeeRT_ErrIntegerOverflowEx(value, num_bits, flags)        Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowEx)(value, num_bits, flags), -1)
#define DeeRT_ErrIntegerOverflowS(value, minval, maxval)          Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowS)(value, minval, maxval), -1)
#define DeeRT_ErrIntegerOverflowU(value, maxval)                  Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowU)(value, maxval), -1)
#define DeeRT_ErrIntegerOverflowUMul(lhs, rhs)                    Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowUMul)(lhs, rhs), -1)
#define DeeRT_ErrIntegerOverflowUAdd(lhs, rhs)                    Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowUAdd)(lhs, rhs), -1)
#if __SIZEOF_SIZE_T__ < 8
#define DeeRT_ErrIntegerOverflowS64(value, minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowS64)(value, minval, maxval), -1)
#define DeeRT_ErrIntegerOverflowU64(value, maxval)         Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowU64)(value, maxval), -1)
#endif /* __SIZEOF_SIZE_T__ < 8 */
#define DeeRT_ErrIntegerOverflowS128(value, minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowS128)(value, minval, maxval), -1)
#define DeeRT_ErrIntegerOverflowU128(value, maxval)         Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowU128)(value, maxval), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */

DECL_END

#endif /* !GUARD_DEEMON_ERROR_RT_H */
