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
#ifndef GUARD_DEEMON_ERROR_RT_H
#define GUARD_DEEMON_ERROR_RT_H 1

#include "api.h"
/**/

#include <hybrid/typecore.h>

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
#define DeeRT_ErrNoActiveException() Dee_ASSUMED_VALUE((DeeRT_ErrNoActiveException)(), -1)


/* Throws a `DeeError_NotImplemented' indicating that `self' cannot be serialized */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrCannotSerialize)(DeeObject *__restrict self);
#define DeeRT_ErrCannotSerialize(self) Dee_ASSUMED_VALUE((DeeRT_ErrCannotSerialize)(Dee_AsObject(self)), -1)


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
DFUNDEF ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIntegerOverflow)(/*Numeric*/ /*1..1*/ DeeObject *value,
                                 /*Numeric*/ /*0..1*/ DeeObject *minval,
                                 /*Numeric*/ /*0..1*/ DeeObject *maxval,
                                 bool positive);
#define DeeRT_ErrIntegerOverflow(value, minval, maxval, positive)                          \
	Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflow)(Dee_AsObject(value),  \
	                                             Dee_AsObject(minval), \
	                                             Dee_AsObject(maxval), \
	                                             positive),                                \
	                  -1)

/* Same as "DeeRT_ErrIntegerOverflow", but minval/maxval are set as
 * >> minval   = (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) ? -(1 << (num_bits - 1)) : 0;
 * >> maxval   = (flags & DeeRT_ErrIntegerOverflowEx_F_SIGNED) ? ((1 << (num_bits - 1)) - 1) : ((1 << num_bits) - 1);
 * >> positive = (flags & DeeRT_ErrIntegerOverflowEx_F_POSITIVE) != 0; */
DFUNDEF ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIntegerOverflowEx)(/*Numeric*/ DeeObject *value,
                                   size_t num_bits,
                                   unsigned int flags);
#define DeeRT_ErrIntegerOverflowEx(value, num_bits, flags) \
	Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowEx)(Dee_AsObject(value), num_bits, flags), -1)
#define DeeRT_ErrIntegerOverflowEx_F_NEGATIVE 0 /* Given "value" is negative (and less than "minval") */
#define DeeRT_ErrIntegerOverflowEx_F_POSITIVE 1 /* Given "value" is positive (and greater than "maxval") */
#define DeeRT_ErrIntegerOverflowEx_F_UNSIGNED 0 /* Wanted to case to "uint{num_bits}_t" */
#define DeeRT_ErrIntegerOverflowEx_F_SIGNED   2 /* Wanted to case to "int{num_bits}_t" */
#define DeeRT_ErrIntegerOverflowEx_F_ANYSIGN  4 /* Wanted to case to "int{num_bits}_t" or "uint{num_bits}_t" */



DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowS)(Dee_ssize_t value, Dee_ssize_t minval, Dee_ssize_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowU)(size_t value, size_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowUMul)(size_t lhs, size_t rhs);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowUAdd)(size_t lhs, size_t rhs);
#define DeeRT_ErrIntegerOverflowS(value, minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowS)(value, minval, maxval), -1)
#define DeeRT_ErrIntegerOverflowU(value, maxval)         Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowU)(value, maxval), -1)
#define DeeRT_ErrIntegerOverflowUMul(lhs, rhs)           Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowUMul)(lhs, rhs), -1)
#define DeeRT_ErrIntegerOverflowUAdd(lhs, rhs)           Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowUAdd)(lhs, rhs), -1)
#if __SIZEOF_SIZE_T__ >= 8
#define DeeRT_ErrIntegerOverflowS64(value, minval, maxval) DeeRT_ErrIntegerOverflowS(value, minval, maxval)
#define DeeRT_ErrIntegerOverflowU64(value, maxval)         DeeRT_ErrIntegerOverflowU(value, maxval)
#else  /* __SIZEOF_SIZE_T__ >= 8 */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowS64)(int64_t value, int64_t minval, int64_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowU64)(uint64_t value, uint64_t maxval);
#define DeeRT_ErrIntegerOverflowS64(value, minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowS64)(value, minval, maxval), -1)
#define DeeRT_ErrIntegerOverflowU64(value, maxval)         Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowU64)(value, maxval), -1)
#endif /* __SIZEOF_SIZE_T__ < 8 */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowS128)(Dee_int128_t value, Dee_int128_t minval, Dee_int128_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrIntegerOverflowU128)(Dee_uint128_t value, Dee_uint128_t maxval);
#define DeeRT_ErrIntegerOverflowS128(value, minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowS128)(value, minval, maxval), -1)
#define DeeRT_ErrIntegerOverflowU128(value, maxval)         Dee_ASSUMED_VALUE((DeeRT_ErrIntegerOverflowU128)(value, maxval), -1)
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

/* Same as functions above, but check if the currently thrown error is `IntegerOverflow'
 * If so, wrap it in another nested `IntegerOverflow' that uses the specified minval/maxval
 * values, rather than those of the underlying `IntegerOverflow'
 *
 * These are needed to properly implement error handling for (e.g.) `DeeObject_AsUInt8',
 * which is implemented in terms of `DeeObject_Get32Bit()'. Now if `DeeObject_Get32Bit()'
 * already fails with an `IntegerOverflow', that error will list 2^32 as its upper limit,
 * when the caller's limit would have actually been 2^8. */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflow)(/*Numeric*/ /*0..1*/ DeeObject *minval, /*Numeric*/ /*0..1*/ DeeObject *maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflowS)(Dee_ssize_t minval, Dee_ssize_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflowU)(size_t maxval);
#define DeeRT_ErrNestedOverflow(minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrNestedOverflow)(Dee_AsObject(minval), Dee_AsObject(maxval)), -1)
#if __SIZEOF_SIZE_T__ >= 8
#define DeeRT_ErrNestedOverflowS64(minval, maxval) DeeRT_ErrNestedOverflowS(minval, maxval)
#define DeeRT_ErrNestedOverflowU64(maxval)         DeeRT_ErrNestedOverflowU(maxval)
#else  /* __SIZEOF_SIZE_T__ >= 8 */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflowS64)(int64_t minval, int64_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflowU64)(uint64_t maxval);
#define DeeRT_ErrNestedOverflowS64(minval, maxval) Dee_ASSUMED_VALUE((DeeRT_ErrNestedOverflowS64)(minval, maxval), -1)
#define DeeRT_ErrNestedOverflowU64(maxval)         Dee_ASSUMED_VALUE((DeeRT_ErrNestedOverflowU64)(maxval), -1)
#endif /* __SIZEOF_SIZE_T__ < 8 */
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflowS128)(Dee_int128_t minval, Dee_int128_t maxval);
DFUNDEF ATTR_COLD int (DCALL DeeRT_ErrNestedOverflowU128)(Dee_uint128_t maxval);
#define DeeRT_ErrNestedOverflowS8(minval, maxval) DeeRT_ErrNestedOverflowS(minval, maxval)
#define DeeRT_ErrNestedOverflowU8(maxval)         DeeRT_ErrNestedOverflowU(maxval)
#if __SIZEOF_SIZE_T__ >= 2
#define DeeRT_ErrNestedOverflowS16(minval, maxval) DeeRT_ErrNestedOverflowS(minval, maxval)
#define DeeRT_ErrNestedOverflowU16(maxval)         DeeRT_ErrNestedOverflowU(maxval)
#else /* __SIZEOF_SIZE_T__ >= 2 */
#define DeeRT_ErrNestedOverflowS16(minval, maxval) DeeRT_ErrNestedOverflowS64(minval, maxval)
#define DeeRT_ErrNestedOverflowU16(maxval)         DeeRT_ErrNestedOverflowU64(maxval)
#endif /* __SIZEOF_SIZE_T__ < 2 */
#if __SIZEOF_SIZE_T__ >= 4
#define DeeRT_ErrNestedOverflowS32(minval, maxval) DeeRT_ErrNestedOverflowS(minval, maxval)
#define DeeRT_ErrNestedOverflowU32(maxval)         DeeRT_ErrNestedOverflowU(maxval)
#else /* __SIZEOF_SIZE_T__ >= 4 */
#define DeeRT_ErrNestedOverflowS32(minval, maxval) DeeRT_ErrNestedOverflowS64(minval, maxval)
#define DeeRT_ErrNestedOverflowU32(maxval)         DeeRT_ErrNestedOverflowU64(maxval)
#endif /* __SIZEOF_SIZE_T__ < 4 */



/* Throws an `DeeError_DivideByZero' indicating that a zero-division attempt has taken place. */
struct Dee_variant;
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrDivideByZero)(DeeObject *lhs, DeeObject *rhs);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrDivideByZeroEx)(struct Dee_variant *lhs, struct Dee_variant *rhs);
#define DeeRT_ErrDivideByZero(lhs, rhs)   Dee_ASSUMED_VALUE((DeeRT_ErrDivideByZero)(Dee_AsObject(lhs), Dee_AsObject(rhs)), -1)
#define DeeRT_ErrDivideByZeroEx(lhs, rhs) Dee_ASSUMED_VALUE((DeeRT_ErrDivideByZeroEx)(lhs, rhs), -1)


/* Check if the currently-thrown exception is an `IntegerOverflow'. If so, wrap that
 * error within a `NegativeShift' (setting it as the `NegativeShift's "cause"), and
 * using `lhs' as the shift's left-hand-side expression.
 *
 * If the currently-thrown exception isn't an `IntegerOverflow', do nothing.
 *
 * @return: -1: Always returns `-1', no matter what this function ended up doing. */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrNegativeShiftOverflow)(DeeObject *lhs, bool is_left_shift);
#define DeeRT_ErrNegativeShiftOverflow(lhs, is_left_shift) \
	Dee_ASSUMED_VALUE((DeeRT_ErrNegativeShiftOverflow)(Dee_AsObject(lhs), is_left_shift), -1)




/* Throws an `DeeError_UnknownKey' indicating that a given index/key is unknown */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnknownKey)(DeeObject *map, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrUnknownKeyWithCause)(DeeObject *map, DeeObject *key, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnknownKeyStr)(DeeObject *map, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrUnknownKeyStrWithCause)(DeeObject *map, char const *key, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnknownKeyStrLen)(DeeObject *map, char const *key, size_t keylen);
DFUNDEF ATTR_COLD NONNULL((1, 2, 4)) int (DCALL DeeRT_ErrUnknownKeyStrLenWithCause)(DeeObject *map, char const *key, size_t keylen, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnknownKeyInt)(DeeObject *map, size_t key);
#define DeeRT_ErrUnknownKey(map, key)                               Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKey)(Dee_AsObject(map), Dee_AsObject(key)), -1)
#define DeeRT_ErrUnknownKeyWithCause(map, key, cause)               Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyWithCause)(Dee_AsObject(map), Dee_AsObject(key), Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnknownKeyStr(map, key)                            Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyStr)(Dee_AsObject(map), key), -1)
#define DeeRT_ErrUnknownKeyStrWithCause(map, key, cause)            Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyStrWithCause)(Dee_AsObject(map), key, Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnknownKeyStrLen(map, key, keylen)                 Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyStrLen)(Dee_AsObject(map), key, keylen), -1)
#define DeeRT_ErrUnknownKeyStrLenWithCause(map, key, keylen, cause) Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyStrLenWithCause)(Dee_AsObject(map), key, keylen, Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnknownKeyInt(map, key)                            Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyInt)(Dee_AsObject(map), key), -1)

/* Throws an `DeeError_ReadOnlyKey' indicating that a given key is read-only */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrReadOnlyKey)(DeeObject *map, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrReadOnlyKeyInt)(DeeObject *map, size_t key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrReadOnlyKeyStr)(DeeObject *map, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrReadOnlyKeyStrLen)(DeeObject *map, char const *key, size_t keylen);
#define DeeRT_ErrReadOnlyKey(map, key)               Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKey)(Dee_AsObject(map), Dee_AsObject(key)), -1)
#define DeeRT_ErrReadOnlyKeyInt(map, key)            Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKeyInt)(Dee_AsObject(map), key), -1)
#define DeeRT_ErrReadOnlyKeyStr(map, key)            Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKeyStr)(Dee_AsObject(map), key), -1)
#define DeeRT_ErrReadOnlyKeyStrLen(map, key, keylen) Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKeyStrLen)(Dee_AsObject(map), key, keylen), -1)

/* Throws an `DeeError_UnboundItem' indicating that a given index/key is unbound */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKey)(DeeObject *seq, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrUnboundKeyWithCause)(DeeObject *seq, DeeObject *key, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKeyStr)(DeeObject *seq, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrUnboundKeyStrWithCause)(DeeObject *seq, char const *key, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKeyStrLen)(DeeObject *seq, char const *key, size_t keylen);
DFUNDEF ATTR_COLD NONNULL((1, 2, 4)) int (DCALL DeeRT_ErrUnboundKeyStrLenWithCause)(DeeObject *seq, char const *key, size_t keylen, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnboundKeyInt)(DeeObject *seq, size_t key);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnboundIndex)(DeeObject *seq, size_t index);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundIndexObj)(DeeObject *seq, DeeObject *index);
#define DeeRT_ErrUnboundKey(seq, key)                               Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKey)(Dee_AsObject(seq), Dee_AsObject(key)), -1)
#define DeeRT_ErrUnboundKeyWithCause(seq, key, cause)               Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyWithCause)(Dee_AsObject(seq), Dee_AsObject(key), Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnboundKeyStr(seq, key)                            Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyStr)(Dee_AsObject(seq), key), -1)
#define DeeRT_ErrUnboundKeyStrWithCause(seq, key, cause)            Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyStrWithCause)(Dee_AsObject(seq), key, Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnboundKeyStrLen(seq, key, keylen)                 Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyStrLen)(Dee_AsObject(seq), key, keylen), -1)
#define DeeRT_ErrUnboundKeyStrLenWithCause(seq, key, keylen, cause) Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyStrLenWithCause)(Dee_AsObject(seq), key, keylen, Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnboundKeyInt(seq, key)                            Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyInt)(Dee_AsObject(seq), key), -1)
#define DeeRT_ErrUnboundIndex(seq, index)                           Dee_ASSUMED_VALUE((DeeRT_ErrUnboundIndex)(Dee_AsObject(seq), index), -1)
#define DeeRT_ErrUnboundIndexObj(seq, index)                        Dee_ASSUMED_VALUE((DeeRT_ErrUnboundIndexObj)(Dee_AsObject(seq), Dee_AsObject(index)), -1)

/* Throws an `DeeError_IndexError' indicating that a given index is out-of-bounds */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrIndexOutOfBounds)(DeeObject *seq, size_t index, size_t length);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrIndexOutOfBoundsObj)(DeeObject *seq, DeeObject *index, /*0..1*/ DeeObject *length);
#define DeeRT_ErrIndexOutOfBounds(seq, index, length)    Dee_ASSUMED_VALUE((DeeRT_ErrIndexOutOfBounds)(Dee_AsObject(seq), index, length), -1)
#define DeeRT_ErrIndexOutOfBoundsObj(seq, index, length) Dee_ASSUMED_VALUE((DeeRT_ErrIndexOutOfBoundsObj)(Dee_AsObject(seq), Dee_AsObject(index), Dee_AsObject(length)), -1)
#ifdef CONFIG_BUILDING_DEEMON
#ifdef DEE_SOURCE
#define Dee_code_frame code_frame
#endif /* DEE_SOURCE */
struct Dee_code_frame; /* Exception thrown by "ASM_VARARGS_GETITEM" if the index is out-of-bounds */
INTDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrVaIndexOutOfBounds)(struct Dee_code_frame const *__restrict frame, size_t index);
#define DeeRT_ErrVaIndexOutOfBounds(frame, index) \
	Dee_ASSUMED_VALUE((DeeRT_ErrVaIndexOutOfBounds)(frame, index), -1)
#endif /* CONFIG_BUILDING_DEEMON */

/* Check if the most-recently-thrown exception is one ... and wrap it
 * using another exception for the purposes to mapping sequence errors
 * from a nested sequence object as belonging to a surrounding sequence:
 * - DeeError_SequenceError
 * - DeeError_KeyError
 * - DeeError_IndexError
 * - DeeError_EmptySequence
 * - DeeError_UnboundItem
 * - DeeError_UnknownKey
 * - DeeError_ReadOnlyKey
 * - DeeError_ItemNotFound
 * - DeeError_UnpackError
 * @param: from: The expected inner sequence ("SequenceError.seq").
 *               If the current error isn't derived from "SequenceError",
 *               isn't one of the above types, or is for a sequence other
 *               than "from", this function does nothing.
 * @param: to: The surrounding sequence to map to. */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrNestSequenceError)(DeeObject *from, DeeObject *to);
#define DeeRT_ErrNestSequenceError(from, to) \
	Dee_ASSUMED_VALUE((DeeRT_ErrNestSequenceError)(Dee_AsObject(from), Dee_AsObject(to)), -1)



/* Throws an `DeeError_UnpackError' indicating that a sequence `seq'
 * of `actual_size' elements cannot be unpacked to `expected_size'. */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnpackError)(DeeObject *seq, size_t expected_size, size_t actual_size);
DFUNDEF ATTR_COLD NONNULL((1, 4)) int (DCALL DeeRT_ErrUnpackErrorWithCause)(DeeObject *seq, size_t expected_size, size_t actual_size, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnpackErrorEx)(DeeObject *seq, size_t expected_size_min, size_t expected_size_max, size_t actual_size);
DFUNDEF ATTR_COLD NONNULL((1, 5)) int (DCALL DeeRT_ErrUnpackErrorExWithCause)(DeeObject *seq, size_t expected_size_min, size_t expected_size_max, size_t actual_size, /*inherit(always)*/ DREF DeeObject *cause);
#define DeeRT_ErrUnpackError(seq, expected_size, actual_size) \
	Dee_ASSUMED_VALUE((DeeRT_ErrUnpackError)(Dee_AsObject(seq), expected_size, actual_size), -1)
#define DeeRT_ErrUnpackErrorWithCause(seq, expected_size, actual_size, cause) \
	Dee_ASSUMED_VALUE((DeeRT_ErrUnpackErrorWithCause)(Dee_AsObject(seq), expected_size, actual_size, Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnpackErrorEx(seq, expected_size_min, expected_size_max, actual_size) \
	Dee_ASSUMED_VALUE((DeeRT_ErrUnpackErrorEx)(Dee_AsObject(seq), expected_size_min, expected_size_max, actual_size), -1)
#define DeeRT_ErrUnpackErrorExWithCause(seq, expected_size_min, expected_size_max, actual_size, cause) \
	Dee_ASSUMED_VALUE((DeeRT_ErrUnpackErrorExWithCause)(Dee_AsObject(seq), expected_size_min, expected_size_max, actual_size, Dee_AsObject(cause)), -1)
#ifdef CONFIG_BUILDING_DEEMON
INTDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrVaUnpackError)(struct Dee_code_frame const *__restrict frame, size_t expected_size);
#define DeeRT_ErrVaUnpackError(frame, expected_size) \
	Dee_ASSUMED_VALUE((DeeRT_ErrVaUnpackError)(frame, expected_size), -1)
#endif /* CONFIG_BUILDING_DEEMON */


/* Check if the currently-thrown exception is an `IntegerOverflow'. If so, wrap that
 * error within an `IndexError' (setting it as the `IndexError's "cause"), and using
 * `seq' as the accompanying sequence.
 *
 * If the currently-thrown exception isn't an `IntegerOverflow', do nothing.
 *
 * @return: -1: Always returns `-1', no matter what this function ended up doing. */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrIndexOverflow)(DeeObject *seq);
#define DeeRT_ErrIndexOverflow(seq) Dee_ASSUMED_VALUE((DeeRT_ErrIndexOverflow)(Dee_AsObject(seq)), -1)


/* Throws an `DeeError_EmptySequence' indicating that a given sequence is empty */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrEmptySequence)(DeeObject *seq);
#define DeeRT_ErrEmptySequence(seq) Dee_ASSUMED_VALUE((DeeRT_ErrEmptySequence)(Dee_AsObject(seq)), -1)

/* Throws an `DeeError_ItemNotFound' indicating that a given item could not be found within some sequence */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrItemNotFound)(DeeObject *seq, DeeObject *item);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrItemNotFoundEx)(DeeObject *seq, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define DeeRT_ErrItemNotFound(seq, item)                    Dee_ASSUMED_VALUE((DeeRT_ErrItemNotFound)(Dee_AsObject(seq), Dee_AsObject(item)), -1)
#define DeeRT_ErrItemNotFoundEx(seq, item, start, end, key) Dee_ASSUMED_VALUE((DeeRT_ErrItemNotFoundEx)(Dee_AsObject(seq), Dee_AsObject(item), start, end, key), -1)
#define DeeRT_ErrSubstringNotFound(string, substring_or_substrings, start, end) \
	DeeRT_ErrItemNotFoundEx(string, substring_or_substrings, start, end, NULL)

/* Throws an `DeeError_RegexNotFound' indicating that
 * the given "regex" could not be found within "data"
 * @param: eflags: Set of `DEE_RE_EXEC_*' */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrRegexNotFound)(DeeObject *data, DeeObject *regex,
                               size_t start, size_t end, size_t range,
                               DeeObject *rules, unsigned int eflags);
#define DeeRT_ErrRegexNotFound(data, regex, start, end, range, rules, eflags)           \
	Dee_ASSUMED_VALUE((DeeRT_ErrRegexNotFound)(Dee_AsObject(data),  \
	                                           Dee_AsObject(regex), \
	                                           start, end, range,                       \
	                                           Dee_AsObject(rules), \
	                                           eflags),                                 \
	                  -1)

#ifdef DEE_SOURCE
#define Dee_class_attribute class_attribute
#define Dee_class_desc      class_desc
#define Dee_type_method     type_method
#define Dee_type_getset     type_getset
#define Dee_type_member     type_member
#endif /* DEE_SOURCE */
struct Dee_attrdesc;
struct Dee_class_attribute;
struct Dee_class_desc;
struct Dee_type_method;
struct Dee_type_getset;
struct Dee_type_member;

/* Throws an `DeeError_UnboundAttribute' indicating that some attribute isn't bound
 * @return: NULL: Always returns "NULL" (for easy chaining when called form getters) */
DFUNDEF ATTR_COLD NONNULL((1, 2)) DeeObject *(DCALL DeeRT_ErrUnboundAttr)(DeeObject *ob, /*string*/ DeeObject *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2)) DeeObject *(DCALL DeeRT_ErrUnboundAttrCStr)(DeeObject *ob, /*static*/ char const *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2)) DeeObject *(DCALL DeeRT_ErrUnboundMember)(DeeObject *ob, struct Dee_type_member const *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2)) DeeObject *(DCALL DeeRT_ErrUnboundAttrEx)(DeeObject *ob, struct Dee_attrdesc const *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) DeeObject *(DCALL DeeRT_ErrTUnboundAttr)(DeeObject *decl, DeeObject *ob, /*string*/ DeeObject *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) DeeObject *(DCALL DeeRT_ErrTUnboundAttrCStr)(DeeObject *decl, DeeObject *ob, /*static*/ char const *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2)) DeeObject *(DCALL DeeRT_ErrCUnboundAttrCA)(DeeObject *ob, struct Dee_class_attribute const *attr);
DFUNDEF ATTR_COLD NONNULL((1, 2)) DeeObject *(DCALL DeeRT_ErrCUnboundInstanceMember)(DeeTypeObject *class_type, DeeObject *instance, uint16_t addr);
DFUNDEF ATTR_COLD NONNULL((1)) DeeObject *(DCALL DeeRT_ErrCUnboundClassMember)(DeeTypeObject *class_type, uint16_t addr);
#define DeeRT_ErrUnboundAttr(ob, attr)                              Dee_ASSUMED_VALUE((DeeRT_ErrUnboundAttr)(Dee_AsObject(ob), Dee_AsObject(attr)), (DeeObject *)NULL)
#define DeeRT_ErrUnboundAttrCStr(ob, attr)                          Dee_ASSUMED_VALUE((DeeRT_ErrUnboundAttrCStr)(Dee_AsObject(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrUnboundAttrEx(ob, attr)                            Dee_ASSUMED_VALUE((DeeRT_ErrUnboundAttrEx)(Dee_AsObject(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrTUnboundAttr(decl, ob, attr)                       Dee_ASSUMED_VALUE((DeeRT_ErrTUnboundAttr)(Dee_AsObject(decl), Dee_AsObject(ob), Dee_AsObject(attr)), (DeeObject *)NULL)
#define DeeRT_ErrTUnboundAttrCStr(decl, ob, attr)                   Dee_ASSUMED_VALUE((DeeRT_ErrTUnboundAttrCStr)(Dee_AsObject(decl), Dee_AsObject(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrCUnboundAttrCA(ob, attr)                           Dee_ASSUMED_VALUE((DeeRT_ErrCUnboundAttrCA)(Dee_AsObject(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrCUnboundInstanceMember(class_type, instance, addr) Dee_ASSUMED_VALUE((DeeRT_ErrCUnboundInstanceMember)(class_type, instance, addr), (DeeObject *)NULL)
#define DeeRT_ErrCUnboundClassMember(class_type, addr)              Dee_ASSUMED_VALUE((DeeRT_ErrCUnboundClassMember)(class_type, addr), (DeeObject *)NULL)
#define DeeRT_ErrUnboundInstanceAttrCA(class_type, attr)            DeeRT_ErrCUnboundAttrCA(Dee_AsObject(class_type), attr)

/* Possible values for the "access" of `DeeRT_Err*UnknownAttr*' and `DeeRT_Err*RestrictedAttr*' */
#define DeeRT_ATTRIBUTE_ACCESS_GET   1 /* Attempted to get attribute */
#define DeeRT_ATTRIBUTE_ACCESS_DEL   2 /* Attempted to del attribute */
#define DeeRT_ATTRIBUTE_ACCESS_SET   4 /* Attempted to set attribute */
#define DeeRT_ATTRIBUTE_ACCESS_PRIVATE 0 /* Attempted to access a private attribute */
#define DeeRT_ATTRIBUTE_ACCESS_CALL  DeeRT_ATTRIBUTE_ACCESS_GET /* Call-to-attribute */
#define DeeRT_ATTRIBUTE_ACCESS_BOUND DeeRT_ATTRIBUTE_ACCESS_GET /* Bound test (with "allow_missing = false") */
#define DeeRT_ATTRIBUTE_ACCESS_INIT  DeeRT_ATTRIBUTE_ACCESS_SET /* Initialization */

/* Throws an `DeeError_UnknownAttribute' indicating that some attribute doesn't exist */
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTUnknownAttr)(DeeObject *decl, DeeObject *ob, DeeObject *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTUnknownAttrStr)(DeeObject *decl, DeeObject *ob, char const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTUnknownAttrStrLen)(DeeObject *decl, DeeObject *ob, char const *attr, size_t attrlen, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((2, 3, 5)) int (DCALL DeeRT_ErrTUnknownAttrWithCause)(DeeObject *decl, DeeObject *ob, DeeObject *attr, unsigned int access, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((2, 3, 5)) int (DCALL DeeRT_ErrTUnknownAttrStrWithCause)(DeeObject *decl, DeeObject *ob, char const *attr, unsigned int access, /*inherit(always)*/ DREF DeeObject *cause);
DFUNDEF ATTR_COLD NONNULL((2, 3, 6)) int (DCALL DeeRT_ErrTUnknownAttrStrLenWithCause)(DeeObject *decl, DeeObject *ob, char const *attr, size_t attrlen, unsigned int access, /*inherit(always)*/ DREF DeeObject *cause);
#define DeeRT_ErrTUnknownAttr(decl, ob, attr, access)                Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttr)(Dee_AsObject(decl), Dee_AsObject(ob), Dee_AsObject(attr), access), -1)
#define DeeRT_ErrTUnknownAttrStr(decl, ob, attr, access)             Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrStr)(Dee_AsObject(decl), Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrTUnknownAttrStrLen(decl, ob, attr, attrlen, access) Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrStrLen)(Dee_AsObject(decl), Dee_AsObject(ob), attr, attrlen, access), -1)
#define DeeRT_ErrUnknownAttr(ob, attr, access)                       DeeRT_ErrTUnknownAttr((DeeObject *)NULL, ob, attr, access)
#define DeeRT_ErrUnknownAttrStr(ob, attr, access)                    DeeRT_ErrTUnknownAttrStr((DeeObject *)NULL, ob, attr, access)
#define DeeRT_ErrUnknownAttrStrLen(ob, attr, attrlen, access)        DeeRT_ErrTUnknownAttrStrLen((DeeObject *)NULL, ob, attr, attrlen, access)
#define DeeRT_ErrUnknownAttrDuringInitialization(decl, attr)        \
	DeeRT_ErrTUnknownAttr(Dee_REQUIRES_TYPE(DeeTypeObject *, decl), \
	                      decl, attr, DeeRT_ATTRIBUTE_ACCESS_INIT)
#define DeeRT_ErrUnknownTypeAttr(self, attr, access)                        DeeRT_ErrTUnknownAttr(Dee_REQUIRES_TYPE(DeeTypeObject *, self), self, attr, access)
#define DeeRT_ErrUnknownTypeAttrStr(self, attr, access)                     DeeRT_ErrTUnknownAttrStr(Dee_REQUIRES_TYPE(DeeTypeObject *, self), self, attr, access)
#define DeeRT_ErrUnknownTypeAttrStrLen(self, attr, attrlen, access)         DeeRT_ErrTUnknownAttrStrLen(Dee_REQUIRES_TYPE(DeeTypeObject *, self), self, attr, attrlen, access)
#define DeeRT_ErrUnknownTypeInstanceAttr(self, attr, access)                DeeRT_ErrTUnknownAttr(Dee_REQUIRES_TYPE(DeeTypeObject *, self), self, attr, access)
#define DeeRT_ErrUnknownTypeInstanceAttrStr(self, attr, access)             DeeRT_ErrTUnknownAttrStr(Dee_REQUIRES_TYPE(DeeTypeObject *, self), self, attr, access)
#define DeeRT_ErrUnknownTypeInstanceAttrStrLen(self, attr, attrlen, access) DeeRT_ErrTUnknownAttrStrLen(Dee_REQUIRES_TYPE(DeeTypeObject *, self), self, attr, attrlen, access)
#define DeeRT_ErrTUnknownAttrWithCause(decl, ob, attr, access, cause)                Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrWithCause)(Dee_AsObject(decl), Dee_AsObject(ob), Dee_AsObject(attr), access, Dee_AsObject(cause)), -1)
#define DeeRT_ErrTUnknownAttrStrWithCause(decl, ob, attr, access, cause)             Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrStrWithCause)(Dee_AsObject(decl), Dee_AsObject(ob), attr, access, Dee_AsObject(cause)), -1)
#define DeeRT_ErrTUnknownAttrStrLenWithCause(decl, ob, attr, attrlen, access, cause) Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrStrLenWithCause)(Dee_AsObject(decl), Dee_AsObject(ob), attr, attrlen, access, Dee_AsObject(cause)), -1)
#define DeeRT_ErrUnknownAttrWithCause(ob, attr, access, cause)                       DeeRT_ErrTUnknownAttrWithCause((DeeObject *)NULL, ob, attr, access, cause)
#define DeeRT_ErrUnknownAttrStrWithCause(ob, attr, access, cause)                    DeeRT_ErrTUnknownAttrStrWithCause((DeeObject *)NULL, ob, attr, access, cause)
#define DeeRT_ErrUnknownAttrStrLenWithCause(ob, attr, attrlen, access, cause)        DeeRT_ErrTUnknownAttrStrLenWithCause((DeeObject *)NULL, ob, attr, attrlen, access, cause)

/* Throws an `DeeError_RestrictedAttribute' indicating that the specified attribute access is invalid */
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTRestrictedAttr)(DeeObject *decl, DeeObject *ob, DeeObject *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTRestrictedAttrCStr)(DeeObject *decl, DeeObject *ob, char const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrCRestrictedAttrCA)(DeeObject *ob, struct Dee_class_attribute const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedAttrEx)(DeeObject *ob, struct Dee_attrdesc const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedMethod)(DeeObject *ob, struct Dee_type_method const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedGetSet)(DeeObject *ob, struct Dee_type_getset const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedMember)(DeeObject *ob, struct Dee_type_member const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrCAlreadyBoundInstanceMember)(DeeTypeObject *class_type, DeeObject *instance, uint16_t addr);
#define DeeRT_ErrTRestrictedAttr(decl, ob, attr, access)        Dee_ASSUMED_VALUE((DeeRT_ErrTRestrictedAttr)(Dee_AsObject(decl), Dee_AsObject(ob), Dee_AsObject(attr), access), -1)
#define DeeRT_ErrTRestrictedAttrCStr(decl, ob, attr, access)    Dee_ASSUMED_VALUE((DeeRT_ErrTRestrictedAttrCStr)(Dee_AsObject(decl), Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrCRestrictedAttrCA(ob, attr, access)            Dee_ASSUMED_VALUE((DeeRT_ErrCRestrictedAttrCA)(Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrRestrictedAttrEx(ob, attr, access)             Dee_ASSUMED_VALUE((DeeRT_ErrRestrictedAttrEx)(Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrRestrictedMethod(ob, attr, access)             Dee_ASSUMED_VALUE((DeeRT_ErrRestrictedMethod)(Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrRestrictedGetSet(ob, attr, access)             Dee_ASSUMED_VALUE((DeeRT_ErrRestrictedGetSet)(Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrRestrictedMember(ob, attr, access)             Dee_ASSUMED_VALUE((DeeRT_ErrRestrictedMember)(Dee_AsObject(ob), attr, access), -1)
#define DeeRT_ErrRestrictedInstanceAttrCStr(type, attr, access) DeeRT_ErrTRestrictedAttrCStr(Dee_REQUIRES_TYPE(DeeTypeObject *, type), type, attr, access)
#define DeeRT_ErrRestrictedAttr(ob, attr, access)               DeeRT_ErrTRestrictedAttr((DeeObject *)NULL, ob, attr, access)
#define DeeRT_ErrRestrictedAttrCStr(ob, attr, access)           DeeRT_ErrTRestrictedAttrCStr((DeeObject *)NULL, ob, attr, access)
#define DeeRT_ErrRestrictedAttrCADuringInitialization(decl, attr)        \
	DeeRT_ErrCRestrictedAttrCA(Dee_REQUIRES_TYPE(DeeTypeObject *, decl), \
	                           attr, DeeRT_ATTRIBUTE_ACCESS_INIT)

DECL_END

#endif /* !GUARD_DEEMON_ERROR_RT_H */
