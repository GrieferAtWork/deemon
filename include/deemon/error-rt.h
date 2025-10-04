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


/* Throws an `DeeError_UnknownKey' indicating that a given index/key is unknown */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnknownKey)(DeeObject *map, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrUnknownKeyWithInner)(DeeObject *map, DeeObject *key, /*inherit(always)*/ DREF DeeObject *inner);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnknownKeyInt)(DeeObject *map, size_t key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnknownKeyStr)(DeeObject *map, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnknownKeyStrLen)(DeeObject *map, char const *key, size_t keylen);

/* Throws an `DeeError_ReadOnlyKey' indicating that a given key is read-only */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrReadOnlyKey)(DeeObject *map, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrReadOnlyKeyInt)(DeeObject *map, size_t key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrReadOnlyKeyStr)(DeeObject *map, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrReadOnlyKeyStrLen)(DeeObject *map, char const *key, size_t keylen);

/* Throws an `DeeError_UnboundItem' indicating that a given index/key is unbound */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKey)(DeeObject *seq, DeeObject *key);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrUnboundKeyWithInner)(DeeObject *seq, DeeObject *key, /*inherit(always)*/ DREF DeeObject *inner);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKeyStr)(DeeObject *seq, char const *key);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundKeyStrLen)(DeeObject *seq, char const *key, size_t keylen);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnboundKeyInt)(DeeObject *seq, size_t key);
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrUnboundIndex)(DeeObject *seq, size_t index);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrUnboundIndexObj)(DeeObject *seq, DeeObject *index);

/* Throws an `DeeError_IndexError' indicating that a given index is out-of-bounds */
DFUNDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrIndexOutOfBounds)(DeeObject *seq, size_t index, size_t length);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeRT_ErrIndexOutOfBoundsObj)(DeeObject *seq, DeeObject *index, DeeObject *length);
#ifdef CONFIG_BUILDING_DEEMON
#ifdef DEE_SOURCE
#define Dee_code_frame code_frame
#endif /* DEE_SOURCE */
struct Dee_code_frame; /* Exception thrown by "ASM_VARARGS_GETITEM" if the index is out-of-bounds */
INTDEF ATTR_COLD NONNULL((1)) int (DCALL DeeRT_ErrVaIndexOutOfBounds)(struct Dee_code_frame const *__restrict frame, size_t index);
#endif /* CONFIG_BUILDING_DEEMON */

/* Throws an `DeeError_ItemNotFound' indicating that a given item could not be found within some sequence */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrItemNotFound)(DeeObject *seq, DeeObject *item);
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrItemNotFoundEx)(DeeObject *seq, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define DeeRT_ErrSubstringNotFound(string, substring_or_substrings, start, end) \
	DeeRT_ErrItemNotFoundEx(string, substring_or_substrings, start, end, NULL)

/* Throws an `DeeError_RegexNotFound' indicating that
 * the given "regex" could not be found within "data"
 * @param: eflags: Set of `DEE_RE_EXEC_*' */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrRegexNotFound)(DeeObject *data, DeeObject *regex,
                               size_t start, size_t end, size_t range,
                               DeeObject *rules, unsigned int eflags);

#ifdef DEE_SOURCE
#define Dee_class_attribute class_attribute
#define Dee_class_desc      class_desc
#define Dee_type_member     type_member
#endif /* DEE_SOURCE */
struct Dee_attrdesc;
struct Dee_class_attribute;
struct Dee_class_desc;
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
#define DeeRT_ErrUnboundAttr(ob, attr)                              Dee_ASSUMED_VALUE((DeeRT_ErrUnboundAttr)((DeeObject *)Dee_REQUIRES_OBJECT(ob), (DeeObject *)Dee_REQUIRES_OBJECT(attr)), (DeeObject *)NULL)
#define DeeRT_ErrUnboundAttrCStr(ob, attr)                          Dee_ASSUMED_VALUE((DeeRT_ErrUnboundAttrCStr)((DeeObject *)Dee_REQUIRES_OBJECT(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrUnboundAttrEx(ob, attr)                            Dee_ASSUMED_VALUE((DeeRT_ErrUnboundAttrEx)((DeeObject *)Dee_REQUIRES_OBJECT(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrTUnboundAttr(decl, ob, attr)                       Dee_ASSUMED_VALUE((DeeRT_ErrTUnboundAttr)((DeeObject *)Dee_REQUIRES_OBJECT(decl), (DeeObject *)Dee_REQUIRES_OBJECT(ob), (DeeObject *)Dee_REQUIRES_OBJECT(attr)), (DeeObject *)NULL)
#define DeeRT_ErrTUnboundAttrCStr(decl, ob, attr)                   Dee_ASSUMED_VALUE((DeeRT_ErrTUnboundAttrCStr)((DeeObject *)Dee_REQUIRES_OBJECT(decl), (DeeObject *)Dee_REQUIRES_OBJECT(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrCUnboundAttrCA(ob, attr)                           Dee_ASSUMED_VALUE((DeeRT_ErrCUnboundAttrCA)((DeeObject *)Dee_REQUIRES_OBJECT(ob), attr), (DeeObject *)NULL)
#define DeeRT_ErrCUnboundInstanceMember(class_type, instance, addr) Dee_ASSUMED_VALUE((DeeRT_ErrCUnboundInstanceMember)(class_type, instance, addr), (DeeObject *)NULL)
#define DeeRT_ErrCUnboundClassMember(class_type, addr)              Dee_ASSUMED_VALUE((DeeRT_ErrCUnboundClassMember)(class_type, addr), (DeeObject *)NULL)
#define DeeRT_ErrUnboundInstanceAttrCA(class_type, attr)            DeeRT_ErrCUnboundAttrCA((DeeObject *)Dee_REQUIRES_OBJECT(class_type), attr)

#define DeeRT_ATTRIBUTE_ACCESS_GET   1 /* Attempted to get attribute */
#define DeeRT_ATTRIBUTE_ACCESS_DEL   2 /* Attempted to del attribute */
#define DeeRT_ATTRIBUTE_ACCESS_SET   4 /* Attempted to set attribute */
#define DeeRT_ATTRIBUTE_ACCESS_BOUND DeeRT_ATTRIBUTE_ACCESS_GET /* Bound test (with "allow_missing = false") */
#define DeeRT_ATTRIBUTE_ACCESS_INIT  DeeRT_ATTRIBUTE_ACCESS_SET /* Initialization */

/* Throws an `DeeError_UnknownAttribute' indicating that some attribute doesn't exist */
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTUnknownAttr)(DeeObject *decl, DeeObject *ob, DeeObject *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTUnknownAttrStr)(DeeObject *decl, DeeObject *ob, char const *attr, unsigned int access);
DFUNDEF ATTR_COLD NONNULL((2, 3)) int (DCALL DeeRT_ErrTUnknownAttrStrLen)(DeeObject *decl, DeeObject *ob, char const *attr, size_t attrlen, unsigned int access);
#define DeeRT_ErrTUnknownAttr(decl, ob, attr, access)                Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttr)((DeeObject *)Dee_REQUIRES_OBJECT(decl), (DeeObject *)Dee_REQUIRES_OBJECT(ob), (DeeObject *)Dee_REQUIRES_OBJECT(attr), access), -1)
#define DeeRT_ErrTUnknownAttrStr(decl, ob, attr, access)             Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrStr)((DeeObject *)Dee_REQUIRES_OBJECT(decl), (DeeObject *)Dee_REQUIRES_OBJECT(ob), attr, access), -1)
#define DeeRT_ErrTUnknownAttrStrLen(decl, ob, attr, attrlen, access) Dee_ASSUMED_VALUE((DeeRT_ErrTUnknownAttrStrLen)((DeeObject *)Dee_REQUIRES_OBJECT(decl), (DeeObject *)Dee_REQUIRES_OBJECT(ob), attr, attrlen, access), -1)
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

//TODO:DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedAttr)(DeeObject *ob, DeeObject *attr, unsigned int access);
//TODO:DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedAttrStr)(DeeObject *ob, char const *attr, unsigned int access);
//TODO:DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedAttrStrLen)(DeeObject *ob, char const *attr, size_t attrlen, unsigned int access);
//TODO:DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrTRestrictedAttr)(DeeTypeObject *decl, DeeObject *ob, DeeObject *attr, unsigned int access);
//TODO:DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeRT_ErrRestrictedAttrEx)(DeeTypeObject *decl, DeeObject *ob, struct Dee_attrdesc const *attr, unsigned int access);







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
#define DeeRT_ErrUnknownKey(map, key)                       Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKey)(map, key), -1)
#define DeeRT_ErrUnknownKeyWithInner(map, key, inner)       Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyWithInner)(map, key, inner), -1)
#define DeeRT_ErrUnknownKeyInt(map, key)                    Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyInt)(map, key), -1)
#define DeeRT_ErrUnknownKeyStr(map, key)                    Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyStr)(map, key), -1)
#define DeeRT_ErrUnknownKeyStrLen(map, key, keylen)         Dee_ASSUMED_VALUE((DeeRT_ErrUnknownKeyStrLen)(map, key, keylen), -1)
#define DeeRT_ErrReadOnlyKey(map, key)                      Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKey)(map, key), -1)
#define DeeRT_ErrReadOnlyKeyInt(map, key)                   Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKeyInt)(map, key), -1)
#define DeeRT_ErrReadOnlyKeyStr(map, key)                   Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKeyStr)(map, key), -1)
#define DeeRT_ErrReadOnlyKeyStrLen(map, key, keylen)        Dee_ASSUMED_VALUE((DeeRT_ErrReadOnlyKeyStrLen)(map, key, keylen), -1)
#define DeeRT_ErrUnboundKey(seq, key)                       Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKey)(seq, key), -1)
#define DeeRT_ErrUnboundKeyWithInner(seq, key, inner)       Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyWithInner)(seq, key, inner), -1)
#define DeeRT_ErrUnboundKeyStr(seq, key)                    Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyStr)(seq, key), -1)
#define DeeRT_ErrUnboundKeyStrLen(seq, key, keylen)         Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyStrLen)(seq, key, keylen), -1)
#define DeeRT_ErrUnboundKeyInt(seq, key)                    Dee_ASSUMED_VALUE((DeeRT_ErrUnboundKeyInt)(seq, key), -1)
#define DeeRT_ErrUnboundIndex(seq, index)                   Dee_ASSUMED_VALUE((DeeRT_ErrUnboundIndex)(seq, index), -1)
#define DeeRT_ErrUnboundIndexObj(seq, index)                Dee_ASSUMED_VALUE((DeeRT_ErrUnboundIndexObj)(seq, index), -1)
#define DeeRT_ErrIndexOutOfBounds(seq, index, length)       Dee_ASSUMED_VALUE((DeeRT_ErrIndexOutOfBounds)(seq, index, length), -1)
#define DeeRT_ErrIndexOutOfBoundsObj(seq, index, length)    Dee_ASSUMED_VALUE((DeeRT_ErrIndexOutOfBoundsObj)(seq, index, length), -1)
#ifdef CONFIG_BUILDING_DEEMON
#define DeeRT_ErrVaIndexOutOfBounds(frame, index) Dee_ASSUMED_VALUE((DeeRT_ErrVaIndexOutOfBounds)(frame, index), -1)
#endif /* CONFIG_BUILDING_DEEMON */
#define DeeRT_ErrItemNotFound(seq, item)                    Dee_ASSUMED_VALUE((DeeRT_ErrItemNotFound)(seq, item), -1)
#define DeeRT_ErrItemNotFoundEx(seq, item, start, end, key) Dee_ASSUMED_VALUE((DeeRT_ErrItemNotFoundEx)(seq, item, start, end, key), -1)
#define DeeRT_ErrRegexNotFound(data, regex, start, end, range, rules, eflags) \
	Dee_ASSUMED_VALUE((DeeRT_ErrRegexNotFound)(data, regex, start, end, range, rules, eflags), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */

DECL_END

#endif /* !GUARD_DEEMON_ERROR_RT_H */
