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
#ifndef GUARD_DEEMON_OBJECTS_NONE_C
#define GUARD_DEEMON_OBJECTS_NONE_C 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/error-rt.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/mro.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include <hybrid/typecore.h>

#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* int32_t */

DECL_BEGIN

STATIC_ASSERT(__SIZEOF_POINTER__ == __SIZEOF_SIZE_T__);
STATIC_ASSERT(__SIZEOF_POINTER__ == Dee_SIZEOF_HASH_T);

#ifdef DCALL_RETURN_COMMON
STATIC_ASSERT(__SIZEOF_REGISTER__ >= __SIZEOF_SIZE_T__);
STATIC_ASSERT(__SIZEOF_REGISTER__ >= __SIZEOF_INT__);
#endif /* DCALL_RETURN_COMMON */

PUBLIC ATTR_RETNONNULL WUNUSED DREF DeeObject *(DCALL DeeNone_NewRef)(void) {
	Dee_Incref(&DeeNone_Singleton);
	return (DREF DeeObject *)&DeeNone_Singleton;
}


#ifdef DCALL_CALLER_CLEANUP
PUBLIC size_t (DCALL _DeeNone_rets0)(void) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets1)(void) {
	return 1;
}

PUBLIC size_t (DCALL _DeeNone_retsm1)(void) {
	return (size_t)-1;
}

#if !defined(DCALL_RETURN_COMMON) && __SIZEOF_SIZE_T__ != __SIZEOF_INT__
PUBLIC int (DCALL _DeeNone_reti1)(void) {
	return 1;
}

PUBLIC int (DCALL _DeeNone_reti0)(void) {
	return 0;
}
#endif /* !DCALL_RETURN_COMMON && __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
#else /* DCALL_CALLER_CLEANUP */
PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef1)(void *UNUSED(a)) {
	return_none;
}

PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef2)(void *UNUSED(a), void *UNUSED(b)) {
	return_none;
}

PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef3)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return_none;
}

PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef4)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return_none;
}

PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef5)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return_none;
}

PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef6)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f)) {
	return_none;
}

PUBLIC WUNUSED DREF DeeObject *(DCALL _DeeNone_NewRef7)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f), void *UNUSED(g)) {
	return_none;
}

PUBLIC size_t (DCALL _DeeNone_rets0_1)(void *UNUSED(a)) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets0_2)(void *UNUSED(a), void *UNUSED(b)) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets0_3)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets0_4)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets0_5)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets0_6)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f)) {
	return 0;
}

PUBLIC size_t (DCALL _DeeNone_rets1_1)(void *UNUSED(a)) {
	return 1;
}

PUBLIC int (DCALL _DeeNone_reti1_2)(void *UNUSED(a), void *UNUSED(b)) {
	return 1;
}

PUBLIC int (DCALL _DeeNone_reti1_3)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 1;
}

PUBLIC int (DCALL _DeeNone_reti1_4)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return 1;
}

PUBLIC size_t (DCALL _DeeNone_retsm1_1)(void *UNUSED(a)) {
	return (size_t)-1;
}

PUBLIC size_t (DCALL _DeeNone_retsm1_2)(void *UNUSED(a), void *UNUSED(b)) {
	return (size_t)-1;
}

PUBLIC size_t (DCALL _DeeNone_retsm1_3)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return (size_t)-1;
}

PUBLIC size_t (DCALL _DeeNone_retsm1_4)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return (size_t)-1;
}

PUBLIC size_t (DCALL _DeeNone_retsm1_5)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return (size_t)-1;
}

#if !defined(DCALL_RETURN_COMMON) && __SIZEOF_SIZE_T__ != __SIZEOF_INT__
PUBLIC int (DCALL _DeeNone_reti1_1)(void *UNUSED(a)) {
	return 1;
}

PUBLIC int (DCALL _DeeNone_reti0_1)(void *UNUSED(a)) {
	return 0;
}

PUBLIC int (DCALL _DeeNone_reti0_2)(void *UNUSED(a), void *UNUSED(b)) {
	return 0;
}

PUBLIC int (DCALL _DeeNone_reti0_3)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 0;
}

PUBLIC int (DCALL _DeeNone_reti0_4)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return 0;
}

PUBLIC int (DCALL _DeeNone_reti0_5)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return 0;
}
#endif /* !DCALL_RETURN_COMMON && __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PUBLIC int (DCALL _DeeNone_reti0_6)(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f)) {
	return 0;
}

#endif /* !DCALL_CALLER_CLEANUP */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeNone_OperatorInt32(DeeObject *__restrict UNUSED(self), int32_t *__restrict result) {
	*result = 0;
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeNone_OperatorInt64(DeeObject *__restrict UNUSED(self), int64_t *__restrict result) {
	*result = 0;
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeNone_OperatorDouble(DeeObject *__restrict UNUSED(self), double *__restrict result) {
	*result = 0.0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeNone_OperatorInt(DeeObject *__restrict UNUSED(self)) {
	return DeeInt_NewZero();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeNone_OperatorEq(DeeObject *UNUSED(self), DeeObject *other) {
	return_bool(DeeNone_Check(other));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeNone_OperatorCompare(DeeObject *UNUSED(self), DeeObject *other) {
	return DeeNone_Check(other) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeNone_OperatorNe(DeeObject *UNUSED(self), DeeObject *other) {
	return_bool(!DeeNone_Check(other));
}

#define DeeNone_OperatorVarCtor    DeeNone_NewRef
#define DeeNone_OperatorVarCopy    (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define DeeNone_OperatorVarInit    (*(DREF DeeObject *(DCALL *)(size_t, DeeObject *const *))&_DeeNone_NewRef2)
#define DeeNone_OperatorInv        DeeNone_OperatorVarCopy
#define DeeNone_OperatorPos        DeeNone_OperatorVarCopy
#define DeeNone_OperatorNeg        DeeNone_OperatorVarCopy
#define DeeNone_OperatorAdd        (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define DeeNone_OperatorSub        DeeNone_OperatorAdd
#define DeeNone_OperatorMul        DeeNone_OperatorAdd
#define DeeNone_OperatorDiv        DeeNone_OperatorAdd
#define DeeNone_OperatorMod        DeeNone_OperatorAdd
#define DeeNone_OperatorShl        DeeNone_OperatorAdd
#define DeeNone_OperatorShr        DeeNone_OperatorAdd
#define DeeNone_OperatorAnd        DeeNone_OperatorAdd
#define DeeNone_OperatorOr         DeeNone_OperatorAdd
#define DeeNone_OperatorXor        DeeNone_OperatorAdd
#define DeeNone_OperatorPow        DeeNone_OperatorAdd
#define DeeNone_OperatorInc        (*(int (DCALL *)(DeeObject **__restrict))&_DeeNone_reti0_1)
#define DeeNone_OperatorDec        DeeNone_OperatorInc
#define DeeNone_OperatorInplaceAdd (*(int (DCALL *)(DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define DeeNone_OperatorInplaceSub DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceMul DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceDiv DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceMod DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceShl DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceShr DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceAnd DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceOr  DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplaceXor DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorInplacePow DeeNone_OperatorInplaceAdd
#define DeeNone_OperatorNextPair   (*(int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&_DeeNone_reti1_2)
#define DeeNone_OperatorAdvance    (*(size_t (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_rets0_2)
#define DeeNone_OperatorHash       (*(Dee_hash_t (DCALL *)(DeeObject *))&_DeeNone_rets0_1)

#define DeeNone_OperatorIter                    DeeNone_OperatorVarCopy
#define DeeNone_OperatorSizeOb                  DeeNone_OperatorVarCopy
#define DeeNone_OperatorContains                DeeNone_OperatorAdd
#define DeeNone_OperatorGetItem                 DeeNone_OperatorAdd
#define DeeNone_OperatorDelItem                 (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define DeeNone_OperatorSetItem                 (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
#define DeeNone_OperatorGetRange                (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define DeeNone_OperatorDelRange                (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
#define DeeNone_OperatorSetRange                (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_4)
#define DeeNone_OperatorForeach                 (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
#define DeeNone_OperatorForeachPair             (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
#define DeeNone_OperatorBoundItem               (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
#define DeeNone_OperatorHasItem                 (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
#define DeeNone_OperatorSize                    (*(size_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
#define DeeNone_OperatorSizeFast                (*(size_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
#define DeeNone_OperatorGetItemIndex            (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&_DeeNone_NewRef2)
#define DeeNone_OperatorGetItemIndexFast        (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&_DeeNone_NewRef2)
#define DeeNone_OperatorDelItemIndex            (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti0_2)
#define DeeNone_OperatorSetItemIndex            (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_reti0_3)
#define DeeNone_OperatorBoundItemIndex          (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti1_2)
#define DeeNone_OperatorHasItemIndex            (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti1_2)
#define DeeNone_OperatorGetRangeIndex           (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&_DeeNone_NewRef3)
#define DeeNone_OperatorDelRangeIndex           (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&_DeeNone_reti0_3)
#define DeeNone_OperatorSetRangeIndex           (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&_DeeNone_reti0_4)
#define DeeNone_OperatorGetRangeIndexN          (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_NewRef2)
#define DeeNone_OperatorDelRangeIndexN          (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_reti0_2)
#define DeeNone_OperatorSetRangeIndexN          (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&_DeeNone_reti0_3)
#define DeeNone_OperatorTryGetItem              DeeNone_OperatorGetItem
#define DeeNone_OperatorTryGetItemIndex         (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&_DeeNone_NewRef2)
#define DeeNone_OperatorTryGetItemStringHash    (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_NewRef3)
#define DeeNone_OperatorGetItemStringHash       (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_NewRef3)
#define DeeNone_OperatorDelItemStringHash       (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti0_3)
#define DeeNone_OperatorSetItemStringHash       (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&_DeeNone_reti0_4)
#define DeeNone_OperatorBoundItemStringHash     (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti1_3)
#define DeeNone_OperatorHasItemStringHash       (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti1_3)
#define DeeNone_OperatorTryGetItemStringLenHash (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_NewRef4)
#define DeeNone_OperatorGetItemStringLenHash    (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_NewRef4)
#define DeeNone_OperatorDelItemStringLenHash    (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti0_4)
#define DeeNone_OperatorSetItemStringLenHash    (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&_DeeNone_reti0_5)
#define DeeNone_OperatorBoundItemStringLenHash  (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti1_4)
#define DeeNone_OperatorHasItemStringLenHash    (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti1_4)
#define DeeNone_OperatorAsVector                (*(size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&_DeeNone_rets0_3)
#define DeeNone_OperatorAsVectorNothrow         DeeNone_OperatorAsVector

#define DeeNone_OperatorGetAttr                   (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&_DeeNone_NewRef2)
#define DeeNone_OperatorDelAttr                   (*(int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&_DeeNone_reti0_2)
#define DeeNone_OperatorSetAttr                   (*(int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&_DeeNone_reti0_3)
#define DeeNone_OperatorFindAttr                  (*(int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&_DeeNone_reti1_4)
#define DeeNone_OperatorHasAttr                   (*(int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&_DeeNone_reti1_2)
#define DeeNone_OperatorBoundAttr                 (*(int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&_DeeNone_reti1_2)
#define DeeNone_OperatorCallAttr                  (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, size_t, DeeObject *const *))&_DeeNone_NewRef4)
#define DeeNone_OperatorCallAttrKw                (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, size_t, DeeObject *const *, DeeObject *))&_DeeNone_NewRef5)
#define DeeNone_OperatorVcallAttrf                (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, char const *, va_list))&_DeeNone_NewRef4)
#define DeeNone_OperatorGetAttrStringHash         (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_NewRef3)
#define DeeNone_OperatorDelAttrStringHash         (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti0_3)
#define DeeNone_OperatorSetAttrStringHash         (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&_DeeNone_reti0_4)
#define DeeNone_OperatorHasAttrStringHash         (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti1_3)
#define DeeNone_OperatorBoundAttrStringHash       (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti1_3)
#define DeeNone_OperatorCallAttrStringHash        (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&_DeeNone_NewRef5)
#define DeeNone_OperatorCallAttrStringHashKw      (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&_DeeNone_NewRef6)
#define DeeNone_OperatorVCallAttrStringHashf      (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, char const *, va_list))&_DeeNone_NewRef5)
#define DeeNone_OperatorGetAttrStringLenHash      (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_NewRef4)
#define DeeNone_OperatorDelAttrStringLenHash      (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti0_4)
#define DeeNone_OperatorSetAttrStringLenHash      (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&_DeeNone_reti0_5)
#define DeeNone_OperatorHasAttrStringLenHash      (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti1_4)
#define DeeNone_OperatorBoundAttrStringLenHash    (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti1_4)
#define DeeNone_OperatorCallAttrStringLenHash     (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&_DeeNone_NewRef6)
#define DeeNone_OperatorCallAttrStringLenHashKw   (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&_DeeNone_NewRef7)
#define DeeNone_OperatorFindAttrInfoStringLenHash (*(bool (DCALL *)(DeeTypeObject *, DeeObject *, char const *__restrict, size_t, Dee_hash_t, struct Dee_attrinfo *__restrict))&_DeeNone_reti0_6)
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DeeNone_OperatorCallAttrTuple             (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define DeeNone_OperatorCallAttrTupleKw           (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef4)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DeeNone_OperatorTryGetItemNR              (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&_DeeNone_retsm1_2)
#define DeeNone_OperatorTryGetItemNRStringHash    (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&_DeeNone_retsm1_3)
#define DeeNone_OperatorTryGetItemNRStringLenHash (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&_DeeNone_retsm1_4)

#define DeeNone_OperatorEnter (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define DeeNone_OperatorLeave (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)

#define DeeNone_OperatorAssign     (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define DeeNone_OperatorMoveAssign DeeNone_OperatorAssign
#define DeeNone_OperatorBool       (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define DeeNone_OperatorCall       (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&_DeeNone_NewRef3)
#define DeeNone_OperatorCallKw     (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&_DeeNone_NewRef4)
#define DeeNone_OperatorThisCall   (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&_DeeNone_NewRef4)
#define DeeNone_OperatorThisCallKw (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&_DeeNone_NewRef5)
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DeeNone_OperatorCallTuple       (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define DeeNone_OperatorCallTupleKw     (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define DeeNone_OperatorThisCallTuple   (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define DeeNone_OperatorThisCallTupleKw (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef4)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

STATIC_ASSERT_MSG((size_t)(uintptr_t)ITER_DONE == (size_t)-1, "Assumed by definition of `DeeNone_OperatorIterNext'");
#define DeeNone_OperatorIterNext (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_retsm1_1)

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeNone_OperatorIterAttr(DeeTypeObject *UNUSED(tp_self), DeeObject *UNUSED(self),
                         struct Dee_attriter *iterbuf, size_t bufsize,
                         struct Dee_attrhint const *__restrict UNUSED(hint)) {
	return Dee_attriter_initempty(iterbuf, bufsize);
}


PRIVATE struct type_iterator none_iterator = {
	/* .tp_nextpair  = */ &DeeNone_OperatorNextPair,
	/* .tp_nextkey   = */ &DeeNone_OperatorIterNext,
	/* .tp_nextvalue = */ &DeeNone_OperatorIterNext,
	/* .tp_advance   = */ &DeeNone_OperatorAdvance,
};

PRIVATE struct type_math none_math = {
	/* .tp_int32       = */ &DeeNone_OperatorInt32,
	/* .tp_int64       = */ &DeeNone_OperatorInt64,
	/* .tp_double      = */ &DeeNone_OperatorDouble,
	/* .tp_int         = */ &DeeNone_OperatorInt,
	/* .tp_inv         = */ &DeeNone_OperatorInv,
	/* .tp_pos         = */ &DeeNone_OperatorPos,
	/* .tp_neg         = */ &DeeNone_OperatorNeg,
	/* .tp_add         = */ &DeeNone_OperatorAdd,
	/* .tp_sub         = */ &DeeNone_OperatorSub,
	/* .tp_mul         = */ &DeeNone_OperatorMul,
	/* .tp_div         = */ &DeeNone_OperatorDiv,
	/* .tp_mod         = */ &DeeNone_OperatorMod,
	/* .tp_shl         = */ &DeeNone_OperatorShl,
	/* .tp_shr         = */ &DeeNone_OperatorShr,
	/* .tp_and         = */ &DeeNone_OperatorAnd,
	/* .tp_or          = */ &DeeNone_OperatorOr,
	/* .tp_xor         = */ &DeeNone_OperatorXor,
	/* .tp_pow         = */ &DeeNone_OperatorPow,
	/* .tp_inc         = */ &DeeNone_OperatorInc,
	/* .tp_dec         = */ &DeeNone_OperatorDec,
	/* .tp_inplace_add = */ &DeeNone_OperatorInplaceAdd,
	/* .tp_inplace_sub = */ &DeeNone_OperatorInplaceSub,
	/* .tp_inplace_mul = */ &DeeNone_OperatorInplaceMul,
	/* .tp_inplace_div = */ &DeeNone_OperatorInplaceDiv,
	/* .tp_inplace_mod = */ &DeeNone_OperatorInplaceMod,
	/* .tp_inplace_shl = */ &DeeNone_OperatorInplaceShl,
	/* .tp_inplace_shr = */ &DeeNone_OperatorInplaceShr,
	/* .tp_inplace_and = */ &DeeNone_OperatorInplaceAnd,
	/* .tp_inplace_or  = */ &DeeNone_OperatorInplaceOr,
	/* .tp_inplace_xor = */ &DeeNone_OperatorInplaceXor,
	/* .tp_inplace_pow = */ &DeeNone_OperatorInplacePow,
};

PRIVATE struct type_cmp none_cmp = {
	/* .tp_hash          = */ &DeeNone_OperatorHash,
	/* .tp_compare_eq    = */ &DeeNone_OperatorCompare,
	/* .tp_compare       = */ &DeeNone_OperatorCompare,
	/* .tp_trycompare_eq = */ &DeeNone_OperatorCompare,
	/* .tp_eq            = */ &DeeNone_OperatorEq,
	/* .tp_ne            = */ &DeeNone_OperatorNe,
	/* .tp_lo            = */ &DeeNone_OperatorNe,
	/* .tp_le            = */ &DeeNone_OperatorEq,
	/* .tp_gr            = */ &DeeNone_OperatorNe,
	/* .tp_ge            = */ &DeeNone_OperatorEq,
};

PRIVATE struct type_seq none_seq = {
	/* .tp_iter                         = */ &DeeNone_OperatorIter,
	/* .tp_sizeob                       = */ &DeeNone_OperatorSizeOb,
	/* .tp_contains                     = */ &DeeNone_OperatorContains,
	/* .tp_getitem                      = */ &DeeNone_OperatorGetItem,
	/* .tp_delitem                      = */ &DeeNone_OperatorDelItem,
	/* .tp_setitem                      = */ &DeeNone_OperatorSetItem,
	/* .tp_getrange                     = */ &DeeNone_OperatorGetRange,
	/* .tp_delrange                     = */ &DeeNone_OperatorDelRange,
	/* .tp_setrange                     = */ &DeeNone_OperatorSetRange,
	/* .tp_foreach                      = */ &DeeNone_OperatorForeach,
	/* .tp_foreach_pair                 = */ &DeeNone_OperatorForeachPair,
	/* .tp_bounditem                    = */ &DeeNone_OperatorBoundItem,
	/* .tp_hasitem                      = */ &DeeNone_OperatorHasItem,
	/* .tp_size                         = */ &DeeNone_OperatorSize,
	/* .tp_size_fast                    = */ &DeeNone_OperatorSizeFast,
	/* .tp_getitem_index                = */ &DeeNone_OperatorGetItemIndex,
	/* .tp_getitem_index_fast           = */ &DeeNone_OperatorGetItemIndexFast,
	/* .tp_delitem_index                = */ &DeeNone_OperatorDelItemIndex,
	/* .tp_setitem_index                = */ &DeeNone_OperatorSetItemIndex,
	/* .tp_bounditem_index              = */ &DeeNone_OperatorBoundItemIndex,
	/* .tp_hasitem_index                = */ &DeeNone_OperatorHasItemIndex,
	/* .tp_getrange_index               = */ &DeeNone_OperatorGetRangeIndex,
	/* .tp_delrange_index               = */ &DeeNone_OperatorDelRangeIndex,
	/* .tp_setrange_index               = */ &DeeNone_OperatorSetRangeIndex,
	/* .tp_getrange_index_n             = */ &DeeNone_OperatorGetRangeIndexN,
	/* .tp_delrange_index_n             = */ &DeeNone_OperatorDelRangeIndexN,
	/* .tp_setrange_index_n             = */ &DeeNone_OperatorSetRangeIndexN,
	/* .tp_trygetitem                   = */ &DeeNone_OperatorTryGetItem,
	/* .tp_trygetitem_index             = */ &DeeNone_OperatorTryGetItemIndex,
	/* .tp_trygetitem_string_hash       = */ &DeeNone_OperatorTryGetItemStringHash,
	/* .tp_getitem_string_hash          = */ &DeeNone_OperatorGetItemStringHash,
	/* .tp_delitem_string_hash          = */ &DeeNone_OperatorDelItemStringHash,
	/* .tp_setitem_string_hash          = */ &DeeNone_OperatorSetItemStringHash,
	/* .tp_bounditem_string_hash        = */ &DeeNone_OperatorBoundItemStringHash,
	/* .tp_hasitem_string_hash          = */ &DeeNone_OperatorHasItemStringHash,
	/* .tp_trygetitem_string_len_hash   = */ &DeeNone_OperatorTryGetItemStringLenHash,
	/* .tp_getitem_string_len_hash      = */ &DeeNone_OperatorGetItemStringLenHash,
	/* .tp_delitem_string_len_hash      = */ &DeeNone_OperatorDelItemStringLenHash,
	/* .tp_setitem_string_len_hash      = */ &DeeNone_OperatorSetItemStringLenHash,
	/* .tp_bounditem_string_len_hash    = */ &DeeNone_OperatorBoundItemStringLenHash,
	/* .tp_hasitem_string_len_hash      = */ &DeeNone_OperatorHasItemStringLenHash,
	/* .tp_asvector                     = */ &DeeNone_OperatorAsVector,
	/* .tp_asvector_nothrow             = */ &DeeNone_OperatorAsVectorNothrow,
	/* .tp_trygetitemnr                 = */ &DeeNone_OperatorTryGetItemNR,
	/* .tp_trygetitemnr_string_hash     = */ &DeeNone_OperatorTryGetItemNRStringHash,
	/* .tp_trygetitemnr_string_len_hash = */ &DeeNone_OperatorTryGetItemNRStringLenHash,
};

PRIVATE struct type_attr none_attr = {
	/* .tp_getattr                       = */ &DeeNone_OperatorGetAttr,
	/* .tp_delattr                       = */ &DeeNone_OperatorDelAttr,
	/* .tp_setattr                       = */ &DeeNone_OperatorSetAttr,
	/* .tp_iterattr                      = */ &DeeNone_OperatorIterAttr,
	/* .tp_findattr                      = */ &DeeNone_OperatorFindAttr,
	/* .tp_hasattr                       = */ &DeeNone_OperatorHasAttr,
	/* .tp_boundattr                     = */ &DeeNone_OperatorBoundAttr,
	/* .tp_callattr                      = */ &DeeNone_OperatorCallAttr,
	/* .tp_callattr_kw                   = */ &DeeNone_OperatorCallAttrKw,
	/* .tp_vcallattrf                    = */ &DeeNone_OperatorVcallAttrf,
	/* .tp_getattr_string_hash           = */ &DeeNone_OperatorGetAttrStringHash,
	/* .tp_delattr_string_hash           = */ &DeeNone_OperatorDelAttrStringHash,
	/* .tp_setattr_string_hash           = */ &DeeNone_OperatorSetAttrStringHash,
	/* .tp_hasattr_string_hash           = */ &DeeNone_OperatorHasAttrStringHash,
	/* .tp_boundattr_string_hash         = */ &DeeNone_OperatorBoundAttrStringHash,
	/* .tp_callattr_string_hash          = */ &DeeNone_OperatorCallAttrStringHash,
	/* .tp_callattr_string_hash_kw       = */ &DeeNone_OperatorCallAttrStringHashKw,
	/* .tp_vcallattr_string_hashf        = */ &DeeNone_OperatorVCallAttrStringHashf,
	/* .tp_getattr_string_len_hash       = */ &DeeNone_OperatorGetAttrStringLenHash,
	/* .tp_delattr_string_len_hash       = */ &DeeNone_OperatorDelAttrStringLenHash,
	/* .tp_setattr_string_len_hash       = */ &DeeNone_OperatorSetAttrStringLenHash,
	/* .tp_hasattr_string_len_hash       = */ &DeeNone_OperatorHasAttrStringLenHash,
	/* .tp_boundattr_string_len_hash     = */ &DeeNone_OperatorBoundAttrStringLenHash,
	/* .tp_callattr_string_len_hash      = */ &DeeNone_OperatorCallAttrStringLenHash,
	/* .tp_callattr_string_len_hash_kw   = */ &DeeNone_OperatorCallAttrStringLenHashKw,
	/* .tp_findattr_info_string_len_hash = */ &DeeNone_OperatorFindAttrInfoStringLenHash,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_callattr_tuple                = */ &DeeNone_OperatorCallAttrTuple,
	/* .tp_callattr_tuple_kw             = */ &DeeNone_OperatorCallAttrTupleKw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PRIVATE struct type_with none_with = {
	/* .tp_enter = */ &DeeNone_OperatorEnter,
	/* .tp_leave = */ &DeeNone_OperatorLeave,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeNone_OperatorStr(DeeObject *__restrict UNUSED(a)) {
	return_reference_((DREF DeeObject *)&str_none);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeNone_OperatorPrint(DeeObject *__restrict UNUSED(a),
                      Dee_formatprinter_t printer, void *arg) {
	return (*printer)(arg, STR_none, 4);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeNone_OperatorGetBuf(DeeObject *__restrict UNUSED(self),
                       DeeBuffer *__restrict info,
                       unsigned int UNUSED(flags)) {
	info->bb_base = DeeObject_DATA(&DeeNone_Singleton);
	info->bb_size = 0;
	return 0;
}

PRIVATE struct type_buffer none_buffer = {
	/* .tp_getbuf       = */ &DeeNone_OperatorGetBuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_callable none_callable = {
	/* .tp_call_kw     = */ &DeeNone_OperatorCallKw,
	/* .tp_thiscall    = */ &DeeNone_OperatorThisCall,
	/* .tp_thiscall_kw = */ &DeeNone_OperatorThisCallKw,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ &DeeNone_OperatorCallTuple,
	/* .tp_call_tuple_kw     = */ &DeeNone_OperatorCallTupleKw,
	/* .tp_thiscall_tuple    = */ &DeeNone_OperatorThisCallTuple,
	/* .tp_thiscall_tuple_kw = */ &DeeNone_OperatorThisCallTupleKw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invoke_none_file_char_operator(DeeTypeObject *tp_self, DeeObject *self,
                               /*0..1*/ DeeObject **p_self,
                               size_t argc, DeeObject *const *argv,
                               Dee_operator_t opname) {
	(void)tp_self;
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
#if GETC_EOF == -1
	return DeeInt_NewMinusOne();
#else /* GETC_EOF == -1 */
	return DeeInt_NewInt8(GETC_EOF);
#endif /* GETC_EOF != -1 */
}


/* All operators implemented by "none" can be constant propagated.
 * They also never throw any exceptions. */
PRIVATE struct type_operator const none_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0003_DESTRUCTOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0009_ITERNEXT, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000A_CALL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000B_INT, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000C_FLOAT, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_000D_INV, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000E_POS, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000F_NEG, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0011_SUB, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0013_DIV, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0014_MOD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0015_SHL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0016_SHR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0017_AND, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0018_OR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0019_XOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001A_POW, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001B_INC, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001C_DEC, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001D_INPLACE_ADD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001E_INPLACE_SUB, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001F_INPLACE_MUL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0020_INPLACE_DIV, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0021_INPLACE_MOD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0022_INPLACE_SHL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0023_INPLACE_SHR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0024_INPLACE_AND, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0025_INPLACE_OR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0026_INPLACE_XOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0027_INPLACE_POW, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0036_DELRANGE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0037_SETRANGE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0038_GETATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0039_DELATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003A_SETATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003B_ENUMATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003C_ENTER, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003D_LEAVE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	/**/

	/* Implement char-related file operators such that we always return `GETC_EOF'.
	 * Without this, `DeeType_GetCustomOperatorById()' would pick noop_custom_operator_cb,
	 * which would re-return "none", which would then evaluate to "0" (which isn't,
	 * and can't be the value of `GETC_EOF')
	 * iow: without this, `none' would be "/dev/zero", but we want it to be "/dev/null" */
	TYPE_OPERATOR_CUSTOM(OPERATOR_FILE_0008_GETC, &invoke_none_file_char_operator, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_CUSTOM(OPERATOR_FILE_0009_UNGETC, &invoke_none_file_char_operator, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_CUSTOM(OPERATOR_FILE_000A_PUTC, &invoke_none_file_char_operator, METHOD_FCONSTCALL | METHOD_FNOTHROW),
};

PRIVATE struct Dee_type_mh_cache mh_cache_none = {
	/* clang-format off */
/*[[[deemon (printSpecialTypeMhCacheBody from "..method-hints.method-hints")({"$none", "$empty"});]]]*/
	/* .mh_seq_operator_bool                       = */ &default__seq_operator_bool__none,
	/* .mh_seq_operator_sizeob                     = */ &default__seq_operator_sizeob__none,
	/* .mh_seq_operator_size                       = */ &default__seq_operator_size__empty,
	/* .mh_seq_operator_iter                       = */ &default__seq_operator_iter__none,
	/* .mh_seq_operator_foreach                    = */ &default__seq_operator_foreach__empty,
	/* .mh_seq_operator_foreach_pair               = */ &default__seq_operator_foreach_pair__empty,
	/* .mh_seq_operator_getitem                    = */ &default__seq_operator_getitem__none,
	/* .mh_seq_operator_getitem_index              = */ &default__seq_operator_getitem_index__none,
	/* .mh_seq_operator_trygetitem                 = */ &default__seq_operator_trygetitem__none,
	/* .mh_seq_operator_trygetitem_index           = */ &default__seq_operator_trygetitem_index__none,
	/* .mh_seq_operator_hasitem                    = */ &default__seq_operator_hasitem__none,
	/* .mh_seq_operator_hasitem_index              = */ &default__seq_operator_hasitem_index__none,
	/* .mh_seq_operator_bounditem                  = */ &default__seq_operator_bounditem__none,
	/* .mh_seq_operator_bounditem_index            = */ &default__seq_operator_bounditem_index__none,
	/* .mh_seq_operator_delitem                    = */ &default__seq_operator_delitem__none,
	/* .mh_seq_operator_delitem_index              = */ &default__seq_operator_delitem_index__none,
	/* .mh_seq_operator_setitem                    = */ &default__seq_operator_setitem__none,
	/* .mh_seq_operator_setitem_index              = */ &default__seq_operator_setitem_index__none,
	/* .mh_seq_operator_getrange                   = */ &default__seq_operator_getrange__none,
	/* .mh_seq_operator_getrange_index             = */ &default__seq_operator_getrange_index__none,
	/* .mh_seq_operator_getrange_index_n           = */ &default__seq_operator_getrange_index_n__none,
	/* .mh_seq_operator_delrange                   = */ &default__seq_operator_delrange__none,
	/* .mh_seq_operator_delrange_index             = */ &default__seq_operator_delrange_index__none,
	/* .mh_seq_operator_delrange_index_n           = */ &default__seq_operator_delrange_index_n__none,
	/* .mh_seq_operator_setrange                   = */ &default__seq_operator_setrange__none,
	/* .mh_seq_operator_setrange_index             = */ &default__seq_operator_setrange_index__none,
	/* .mh_seq_operator_setrange_index_n           = */ &default__seq_operator_setrange_index_n__none,
	/* .mh_seq_operator_assign                     = */ &default__seq_operator_assign__none,
	/* .mh_seq_operator_hash                       = */ &default__seq_operator_hash__none,
	/* .mh_seq_operator_compare                    = */ &default__seq_operator_compare__empty,
	/* .mh_seq_operator_compare_eq                 = */ &default__seq_operator_compare_eq__empty,
	/* .mh_seq_operator_trycompare_eq              = */ &default__seq_operator_trycompare_eq__empty,
	/* .mh_seq_operator_eq                         = */ &default__seq_operator_eq__empty,
	/* .mh_seq_operator_ne                         = */ &default__seq_operator_ne__empty,
	/* .mh_seq_operator_lo                         = */ &default__seq_operator_lo__empty,
	/* .mh_seq_operator_le                         = */ &default__seq_operator_le__empty,
	/* .mh_seq_operator_gr                         = */ &default__seq_operator_gr__empty,
	/* .mh_seq_operator_ge                         = */ &default__seq_operator_ge__empty,
	/* .mh_seq_operator_add                        = */ &default__seq_operator_add__none,
	/* .mh_seq_operator_mul                        = */ &default__seq_operator_mul__none,
	/* .mh_seq_operator_inplace_add                = */ &default__seq_operator_inplace_add__none,
	/* .mh_seq_operator_inplace_mul                = */ &default__seq_operator_inplace_mul__none,
	/* .mh_seq_enumerate                           = */ &default__seq_enumerate__empty,
	/* .mh_seq_enumerate_index                     = */ &default__seq_enumerate_index__empty,
	/* .mh_seq_makeenumeration                     = */ &default__seq_makeenumeration__none,
	/* .mh_seq_makeenumeration_with_range          = */ &default__seq_makeenumeration_with_range__none,
	/* .mh_seq_makeenumeration_with_intrange       = */ &default__seq_makeenumeration_with_intrange__none,
	/* .mh_seq_foreach_reverse                     = */ &default__seq_foreach_reverse__empty,
	/* .mh_seq_enumerate_index_reverse             = */ &default__seq_enumerate_index_reverse__empty,
	/* .mh_seq_unpack                              = */ &default__seq_unpack__none,
	/* .mh_seq_unpack_ex                           = */ &default__seq_unpack_ex__none,
	/* .mh_seq_unpack_ub                           = */ &default__seq_unpack_ub__none,
	/* .mh_seq_trygetfirst                         = */ &default__seq_trygetfirst__none,
	/* .mh_seq_getfirst                            = */ &default__seq_getfirst__none,
	/* .mh_seq_boundfirst                          = */ &default__seq_boundfirst__none,
	/* .mh_seq_delfirst                            = */ &default__seq_delfirst__none,
	/* .mh_seq_setfirst                            = */ &default__seq_setfirst__none,
	/* .mh_seq_trygetlast                          = */ &default__seq_trygetlast__none,
	/* .mh_seq_getlast                             = */ &default__seq_getlast__none,
	/* .mh_seq_boundlast                           = */ &default__seq_boundlast__none,
	/* .mh_seq_dellast                             = */ &default__seq_dellast__none,
	/* .mh_seq_setlast                             = */ &default__seq_setlast__none,
	/* .mh_seq_cached                              = */ &default__seq_cached__none,
	/* .mh_seq_frozen                              = */ &default__seq_frozen__none,
	/* .mh_seq_any                                 = */ &default__seq_any__empty,
	/* .mh_seq_any_with_key                        = */ &default__seq_any_with_key__empty,
	/* .mh_seq_any_with_range                      = */ &default__seq_any_with_range__empty,
	/* .mh_seq_any_with_range_and_key              = */ &default__seq_any_with_range_and_key__empty,
	/* .mh_seq_all                                 = */ &default__seq_all__empty,
	/* .mh_seq_all_with_key                        = */ &default__seq_all_with_key__empty,
	/* .mh_seq_all_with_range                      = */ &default__seq_all_with_range__empty,
	/* .mh_seq_all_with_range_and_key              = */ &default__seq_all_with_range_and_key__empty,
	/* .mh_seq_parity                              = */ &default__seq_parity__empty,
	/* .mh_seq_parity_with_key                     = */ &default__seq_parity_with_key__empty,
	/* .mh_seq_parity_with_range                   = */ &default__seq_parity_with_range__empty,
	/* .mh_seq_parity_with_range_and_key           = */ &default__seq_parity_with_range_and_key__empty,
	/* .mh_seq_reduce                              = */ &default__seq_reduce__none,
	/* .mh_seq_reduce_with_init                    = */ &default__seq_reduce_with_init__empty,
	/* .mh_seq_reduce_with_range                   = */ &default__seq_reduce_with_range__none,
	/* .mh_seq_reduce_with_range_and_init          = */ &default__seq_reduce_with_range_and_init__empty,
	/* .mh_seq_min                                 = */ &default__seq_min__empty,
	/* .mh_seq_min_with_key                        = */ &default__seq_min_with_key__empty,
	/* .mh_seq_min_with_range                      = */ &default__seq_min_with_range__none,
	/* .mh_seq_min_with_range_and_key              = */ &default__seq_min_with_range_and_key__none,
	/* .mh_seq_max                                 = */ &default__seq_max__empty,
	/* .mh_seq_max_with_key                        = */ &default__seq_max_with_key__empty,
	/* .mh_seq_max_with_range                      = */ &default__seq_max_with_range__none,
	/* .mh_seq_max_with_range_and_key              = */ &default__seq_max_with_range_and_key__none,
	/* .mh_seq_sum                                 = */ &default__seq_sum__none,
	/* .mh_seq_sum_with_range                      = */ &default__seq_sum_with_range__none,
	/* .mh_seq_count                               = */ &default__seq_count__empty,
	/* .mh_seq_count_with_key                      = */ &default__seq_count_with_key__empty,
	/* .mh_seq_count_with_range                    = */ &default__seq_count_with_range__empty,
	/* .mh_seq_count_with_range_and_key            = */ &default__seq_count_with_range_and_key__empty,
	/* .mh_seq_contains                            = */ &default__seq_contains__empty,
	/* .mh_seq_contains_with_key                   = */ &default__seq_contains_with_key__empty,
	/* .mh_seq_contains_with_range                 = */ &default__seq_contains_with_range__empty,
	/* .mh_seq_contains_with_range_and_key         = */ &default__seq_contains_with_range_and_key__empty,
	/* .mh_seq_operator_contains                   = */ &default__seq_operator_contains__none,
	/* .mh_seq_locate                              = */ &default__seq_locate__none,
	/* .mh_seq_locate_with_range                   = */ &default__seq_locate_with_range__none,
	/* .mh_seq_rlocate                             = */ &default__seq_rlocate__none,
	/* .mh_seq_rlocate_with_range                  = */ &default__seq_rlocate_with_range__none,
	/* .mh_seq_startswith                          = */ &default__seq_startswith__empty,
	/* .mh_seq_startswith_with_key                 = */ &default__seq_startswith_with_key__empty,
	/* .mh_seq_startswith_with_range               = */ &default__seq_startswith_with_range__empty,
	/* .mh_seq_startswith_with_range_and_key       = */ &default__seq_startswith_with_range_and_key__empty,
	/* .mh_seq_endswith                            = */ &default__seq_endswith__empty,
	/* .mh_seq_endswith_with_key                   = */ &default__seq_endswith_with_key__empty,
	/* .mh_seq_endswith_with_range                 = */ &default__seq_endswith_with_range__empty,
	/* .mh_seq_endswith_with_range_and_key         = */ &default__seq_endswith_with_range_and_key__empty,
	/* .mh_seq_find                                = */ &default__seq_find__empty,
	/* .mh_seq_find_with_key                       = */ &default__seq_find_with_key__empty,
	/* .mh_seq_rfind                               = */ &default__seq_rfind__empty,
	/* .mh_seq_rfind_with_key                      = */ &default__seq_rfind_with_key__empty,
	/* .mh_seq_erase                               = */ &default__seq_erase__empty,
	/* .mh_seq_insert                              = */ &default__seq_insert__none,
	/* .mh_seq_insertall                           = */ &default__seq_insertall__none,
	/* .mh_seq_pushfront                           = */ &default__seq_pushfront__none,
	/* .mh_seq_append                              = */ &default__seq_append__none,
	/* .mh_seq_extend                              = */ &default__seq_extend__none,
	/* .mh_seq_xchitem_index                       = */ &default__seq_xchitem_index__none,
	/* .mh_seq_clear                               = */ &default__seq_clear__none,
	/* .mh_seq_pop                                 = */ &default__seq_pop__none,
	/* .mh_seq_remove                              = */ &default__seq_remove__none,
	/* .mh_seq_remove_with_key                     = */ &default__seq_remove_with_key__none,
	/* .mh_seq_rremove                             = */ &default__seq_rremove__none,
	/* .mh_seq_rremove_with_key                    = */ &default__seq_rremove_with_key__none,
	/* .mh_seq_removeall                           = */ &default__seq_removeall__none,
	/* .mh_seq_removeall_with_key                  = */ &default__seq_removeall_with_key__none,
	/* .mh_seq_removeif                            = */ &default__seq_removeif__none,
	/* .mh_seq_resize                              = */ &default__seq_resize__none,
	/* .mh_seq_fill                                = */ &default__seq_fill__empty,
	/* .mh_seq_reverse                             = */ &default__seq_reverse__none,
	/* .mh_seq_reversed                            = */ &default__seq_reversed__none,
	/* .mh_seq_sort                                = */ &default__seq_sort__none,
	/* .mh_seq_sort_with_key                       = */ &default__seq_sort_with_key__none,
	/* .mh_seq_sorted                              = */ &default__seq_sorted__none,
	/* .mh_seq_sorted_with_key                     = */ &default__seq_sorted_with_key__none,
	/* .mh_seq_bfind                               = */ &default__seq_bfind__empty,
	/* .mh_seq_bfind_with_key                      = */ &default__seq_bfind_with_key__empty,
	/* .mh_seq_bposition                           = */ &default__seq_bposition__empty,
	/* .mh_seq_bposition_with_key                  = */ &default__seq_bposition_with_key__empty,
	/* .mh_seq_brange                              = */ &default__seq_brange__empty,
	/* .mh_seq_brange_with_key                     = */ &default__seq_brange_with_key__empty,
	/* .mh_set_operator_iter                       = */ &default__set_operator_iter__none,
	/* .mh_set_operator_foreach                    = */ &default__set_operator_foreach__empty,
	/* .mh_set_operator_sizeob                     = */ &default__set_operator_sizeob__none,
	/* .mh_set_operator_size                       = */ &default__set_operator_size__empty,
	/* .mh_set_operator_hash                       = */ &default__set_operator_hash__none,
	/* .mh_set_operator_compare_eq                 = */ &default__set_operator_compare_eq__empty,
	/* .mh_set_operator_trycompare_eq              = */ &default__set_operator_trycompare_eq__empty,
	/* .mh_set_operator_eq                         = */ &default__set_operator_eq__empty,
	/* .mh_set_operator_ne                         = */ &default__set_operator_ne__empty,
	/* .mh_set_operator_lo                         = */ &default__set_operator_lo__empty,
	/* .mh_set_operator_le                         = */ &default__set_operator_le__empty,
	/* .mh_set_operator_gr                         = */ &default__set_operator_gr__empty,
	/* .mh_set_operator_ge                         = */ &default__set_operator_ge__empty,
	/* .mh_set_operator_inv                        = */ &default__set_operator_inv__none,
	/* .mh_set_operator_add                        = */ &default__set_operator_add__none,
	/* .mh_set_operator_sub                        = */ &default__set_operator_sub__none,
	/* .mh_set_operator_and                        = */ &default__set_operator_and__none,
	/* .mh_set_operator_xor                        = */ &default__set_operator_xor__none,
	/* .mh_set_operator_inplace_add                = */ &default__set_operator_inplace_add__none,
	/* .mh_set_operator_inplace_sub                = */ &default__set_operator_inplace_sub__none,
	/* .mh_set_operator_inplace_and                = */ &default__set_operator_inplace_and__none,
	/* .mh_set_operator_inplace_xor                = */ &default__set_operator_inplace_xor__none,
	/* .mh_set_frozen                              = */ &default__set_frozen__none,
	/* .mh_set_unify                               = */ &default__set_unify__none,
	/* .mh_set_insert                              = */ &default__set_insert__none,
	/* .mh_set_insertall                           = */ &default__set_insertall__none,
	/* .mh_set_remove                              = */ &default__set_remove__none,
	/* .mh_set_removeall                           = */ &default__set_removeall__none,
	/* .mh_set_pop                                 = */ &default__set_pop__none,
	/* .mh_set_pop_with_default                    = */ &default__set_pop_with_default__none,
	/* .mh_map_operator_iter                       = */ &default__map_operator_iter__none,
	/* .mh_map_operator_foreach_pair               = */ &default__map_operator_foreach_pair__empty,
	/* .mh_map_operator_sizeob                     = */ &default__map_operator_sizeob__none,
	/* .mh_map_operator_size                       = */ &default__map_operator_size__none,
	/* .mh_map_operator_hash                       = */ &default__map_operator_hash__none,
	/* .mh_map_operator_getitem                    = */ &default__map_operator_getitem__none,
	/* .mh_map_operator_trygetitem                 = */ &default__map_operator_trygetitem__none,
	/* .mh_map_operator_getitem_index              = */ &default__map_operator_getitem_index__none,
	/* .mh_map_operator_trygetitem_index           = */ &default__map_operator_trygetitem_index__none,
	/* .mh_map_operator_getitem_string_hash        = */ &default__map_operator_getitem_string_hash__none,
	/* .mh_map_operator_trygetitem_string_hash     = */ &default__map_operator_trygetitem_string_hash__none,
	/* .mh_map_operator_getitem_string_len_hash    = */ &default__map_operator_getitem_string_len_hash__none,
	/* .mh_map_operator_trygetitem_string_len_hash = */ &default__map_operator_trygetitem_string_len_hash__none,
	/* .mh_map_operator_bounditem                  = */ &default__map_operator_bounditem__none,
	/* .mh_map_operator_bounditem_index            = */ &default__map_operator_bounditem_index__none,
	/* .mh_map_operator_bounditem_string_hash      = */ &default__map_operator_bounditem_string_hash__none,
	/* .mh_map_operator_bounditem_string_len_hash  = */ &default__map_operator_bounditem_string_len_hash__none,
	/* .mh_map_operator_hasitem                    = */ &default__map_operator_hasitem__none,
	/* .mh_map_operator_hasitem_index              = */ &default__map_operator_hasitem_index__none,
	/* .mh_map_operator_hasitem_string_hash        = */ &default__map_operator_hasitem_string_hash__none,
	/* .mh_map_operator_hasitem_string_len_hash    = */ &default__map_operator_hasitem_string_len_hash__none,
	/* .mh_map_operator_delitem                    = */ &default__map_operator_delitem__empty,
	/* .mh_map_operator_delitem_index              = */ &default__map_operator_delitem_index__empty,
	/* .mh_map_operator_delitem_string_hash        = */ &default__map_operator_delitem_string_hash__empty,
	/* .mh_map_operator_delitem_string_len_hash    = */ &default__map_operator_delitem_string_len_hash__empty,
	/* .mh_map_operator_setitem                    = */ &default__map_operator_setitem__none,
	/* .mh_map_operator_setitem_index              = */ &default__map_operator_setitem_index__none,
	/* .mh_map_operator_setitem_string_hash        = */ &default__map_operator_setitem_string_hash__none,
	/* .mh_map_operator_setitem_string_len_hash    = */ &default__map_operator_setitem_string_len_hash__none,
	/* .mh_map_operator_contains                   = */ &default__map_operator_contains__none,
	/* .mh_map_keys                                = */ &default__map_keys__none,
	/* .mh_map_iterkeys                            = */ &default__map_iterkeys__none,
	/* .mh_map_values                              = */ &default__map_values__none,
	/* .mh_map_itervalues                          = */ &default__map_itervalues__none,
	/* .mh_map_enumerate                           = */ &default__map_enumerate__empty,
	/* .mh_map_enumerate_range                     = */ &default__map_enumerate_range__empty,
	/* .mh_map_makeenumeration                     = */ &default__map_makeenumeration__none,
	/* .mh_map_makeenumeration_with_range          = */ &default__map_makeenumeration_with_range__none,
	/* .mh_map_operator_compare_eq                 = */ &default__map_operator_compare_eq__empty,
	/* .mh_map_operator_trycompare_eq              = */ &default__map_operator_trycompare_eq__empty,
	/* .mh_map_operator_eq                         = */ &default__map_operator_eq__empty,
	/* .mh_map_operator_ne                         = */ &default__map_operator_ne__empty,
	/* .mh_map_operator_lo                         = */ &default__map_operator_lo__empty,
	/* .mh_map_operator_le                         = */ &default__map_operator_le__empty,
	/* .mh_map_operator_gr                         = */ &default__map_operator_gr__empty,
	/* .mh_map_operator_ge                         = */ &default__map_operator_ge__empty,
	/* .mh_map_operator_add                        = */ &default__map_operator_add__none,
	/* .mh_map_operator_sub                        = */ &default__map_operator_sub__none,
	/* .mh_map_operator_and                        = */ &default__map_operator_and__none,
	/* .mh_map_operator_xor                        = */ &default__map_operator_xor__none,
	/* .mh_map_operator_inplace_add                = */ &default__map_operator_inplace_add__none,
	/* .mh_map_operator_inplace_sub                = */ &default__map_operator_inplace_sub__none,
	/* .mh_map_operator_inplace_and                = */ &default__map_operator_inplace_and__none,
	/* .mh_map_operator_inplace_xor                = */ &default__map_operator_inplace_xor__none,
	/* .mh_map_frozen                              = */ &default__map_frozen__none,
	/* .mh_map_setold                              = */ &default__map_setold__none,
	/* .mh_map_setold_ex                           = */ &default__map_setold_ex__none,
	/* .mh_map_setnew                              = */ &default__map_setnew__none,
	/* .mh_map_setnew_ex                           = */ &default__map_setnew_ex__none,
	/* .mh_map_setdefault                          = */ &default__map_setdefault__none,
	/* .mh_map_update                              = */ &default__map_update__none,
	/* .mh_map_remove                              = */ &default__map_remove__none,
	/* .mh_map_removekeys                          = */ &default__map_removekeys__none,
	/* .mh_map_pop                                 = */ &default__map_pop__none,
	/* .mh_map_pop_with_default                    = */ &default__map_pop_with_default__none,
	/* .mh_map_popitem                             = */ &default__map_popitem__empty,
/*[[[end]]]*/
	/* clang-format on */
};

PUBLIC DeeTypeObject DeeNone_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_none),
	/* .tp_doc      = */ DOC("None is a singleton object that can be used in any operation "
	                         /**/ "as either a placeholder, or as a no-op object. Besides being a "
	                         /**/ "no-op for everything, it has several special characteristics, "
	                         /**/ "like its ability to be expanded to any number of itself without "
	                         /**/ "causing :{UnpackError}s to be thrown.\n"

	                         "\n"
	                         "(args!)\n"
	                         "Taking any number of arguments, return the none-singleton\n"

	                         "\n"
	                         "str->\n"
	                         "repr->\n"
	                         "Always returns $\"none\"\n"

	                         "\n"
	                         "bool->\n"
	                         "Always returns $false\n"

	                         "\n"
	                         "int->\n"
	                         "Always returns $0\n"

	                         "\n"
	                         "float->\n"
	                         "Always returns $0.0\n"

	                         "\n"
	                         "copy->\n"
	                         "deepcopy->\n"
	                         ":=->\n"
	                         "move:=->\n"
	                         "next->?N\n"
	                         "call->?N\n"
	                         "~->\n"
	                         "pos->\n"
	                         "neg->\n"
	                         "+(other:?O)->\n"
	                         "-(other:?O)->\n"
	                         "*(other:?O)->\n"
	                         "/(other:?O)->\n"
	                         "%(other:?O)->\n"
	                         "<<(other:?O)->\n"
	                         ">>(other:?O)->\n"
	                         "&(other:?O)->\n"
	                         "|(other:?O)->\n"
	                         "^(other:?O)->\n"
	                         "**(other:?O)->\n"
	                         "++->\n"
	                         "--->\n"
	                         "+=(other:?O)->\n"
	                         "-=(other:?O)->\n"
	                         "*=(other:?O)->\n"
	                         "/=(other:?O)->\n"
	                         "%=(other:?O)->\n"
	                         "<<=(other:?O)->\n"
	                         ">>=(other:?O)->\n"
	                         "&=(other:?O)->\n"
	                         "|=(other:?O)->\n"
	                         "^=(other:?O)->\n"
	                         "**=(other:?O)->\n"
	                         "==(other:?O)->\n"
	                         "!=(other:?O)->\n"
	                         "<(other:?O)->\n"
	                         "<=(other:?O)->\n"
	                         ">(other:?O)->\n"
	                         ">=(other:?O)->\n"
	                         "iter->?N\n"
	                         "#->?N\n"
	                         "contains(other:?O)->?N\n"
	                         "[](index:?O)->?N\n"
	                         "del[](index:?O)->\n"
	                         "[]=(index:?O,value:?O)->\n"
	                         "[:](start:?O,end:?O)->?N\n"
	                         "del[:](start:?O,end:?O)->\n"
	                         "[:]=(start:?O,end:?O,value:?O)->\n"
	                         ".(attr:?Dstring)->?N\n"
	                         "del.(attr:?Dstring)->?N\n"
	                         ".=(attr:?Dstring)->?N\n"
	                         "No-op that ignores all arguments and always re-returns ?N"),
	/* .tp_flags    = */ TP_FVARIABLE | TP_FNORMAL | TP_FNAMEOBJECT | TP_FABSTRACT | TP_FFINAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeNoneObject),
	/* .tp_features = */ TF_SINGLETON | TF_KW,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&DeeNone_OperatorVarCtor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&DeeNone_OperatorVarCopy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&DeeNone_OperatorVarCopy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&DeeNone_OperatorVarInit
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ &DeeNone_OperatorAssign,
		/* .tp_move_assign = */ &DeeNone_OperatorMoveAssign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ &DeeNone_OperatorStr,
		/* .tp_repr      = */ &DeeNone_OperatorStr,
		/* .tp_bool      = */ &DeeNone_OperatorBool,
		/* .tp_print     = */ &DeeNone_OperatorPrint,
		/* .tp_printrepr = */ &DeeNone_OperatorPrint,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &none_math,
	/* .tp_cmp           = */ &none_cmp,
	/* .tp_seq           = */ &none_seq,
	/* .tp_iter_next     = */ &DeeNone_OperatorIterNext,
	/* .tp_iterator      = */ &none_iterator,
	/* .tp_attr          = */ &none_attr,
	/* .tp_with          = */ &none_with,
	/* .tp_buffer        = */ &none_buffer,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ &DeeNone_OperatorCall,
	/* .tp_callable      = */ &none_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ none_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(none_operators),
	/* .tp_mhcache       = */ &mh_cache_none,
};

PUBLIC DeeNoneObject DeeNone_Singleton = {
	OBJECT_HEAD_INIT(&DeeNone_Type),
	WEAKREF_SUPPORT_INIT
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_NONE_C */
