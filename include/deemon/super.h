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
#ifndef GUARD_DEEMON_SUPER_H
#define GUARD_DEEMON_SUPER_H 1

#include "api.h"
/**/

#include "types.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_super_object  super_object
#endif /* DEE_SOURCE */

typedef struct Dee_super_object DeeSuperObject;

struct Dee_super_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD
	DREF DeeTypeObject *s_type; /* [1..1][const] Super-type.
	                             * NOTE: This is never `&DeeSuper_Type' itself and the
	                             *       check `DeeObject_InstanceOf(s_self, s_type)'
	                             *       must always succeed. */
	DREF DeeObject     *s_self; /* [1..1][const] Wrapped object (Never another super-object). */
};

#define DeeSuper_TYPE(x) Dee_REQUIRES_OBJECT(DeeSuperObject, x)->s_type
#define DeeSuper_SELF(x) Dee_REQUIRES_OBJECT(DeeSuperObject, x)->s_self

DDATDEF DeeTypeObject DeeSuper_Type;
#define DeeSuper_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeSuper_Type) /* `Super' is final */
#define DeeSuper_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeSuper_Type)

/* Create a new super-wrapper for `tp_self:self'.
 * NOTE: This function automatically checks the given operands for validity:
 *        - DeeType_Check(tp_self);
 *        - DeeObject_InstanceOf(self, tp_self);
 * It also automatically unwraps `self' should it already be a super-object. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSuper_New(DeeTypeObject *tp_self, DeeObject *self);

/* Taking some object, return the effective super-class of it.
 * HINT: When `self' is another super-object, this is identical to
 *       `DeeSuper_New(DeeType_BASE(DeeSuper_TYPE(self)), DeeSuper_SELF(self))'
 * @throws: Error.TypeError: The class of `self' has no super-class. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSuper_Of(DeeObject *__restrict self);



/*
 * Expose all of the `DeeObject_T*' functions.
 * These behave the same as their equivalent `DeeObject_*', except that:
 * - It is assumed that `Dee_ASSERT_OBJECT_TYPE_A(self, tp_self)'
 * - MRO resolution behaves as if `Dee_TYPE(self) == tp_self'
 *
 * In other words, these functions behave like:
 *     DeeObject_TFoo(tp_self, self, ...)
 * <=> DeeObject_Foo(DeeSuper_New(tp_self, self), ...)
 */

/* Constructor-related operators */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TCopy(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDeepCopy(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TAssign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TMoveAssign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);

/* str/repr/bool operators */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TStr(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TRepr(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_TPrint)(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_TPrintRepr)(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TBool(DeeTypeObject *tp_self, DeeObject *self);

/* Call operators */
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TCall(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TCallKw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TThisCall(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TThisCallKw(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallTuple)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallTupleKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *args, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TThisCallTuple)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TThisCallTupleKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, DeeObject *args, DeeObject *kw);
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#define DeeObject_TCallTuple(tp_self, self, args)                    DeeObject_TCall(tp_self, self, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_TCallTupleKw(tp_self, self, args, kw)              DeeObject_TCallKw(tp_self, self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_TThisCallTuple(tp_self, self, thisarg, args)       DeeObject_TThisCall(tp_self, self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_TThisCallTupleKw(tp_self, self, thisarg, args, kw) DeeObject_TThisCallKw(tp_self, self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */

/* Math operators */
DFUNDEF WUNUSED ATTR_OUT(3) NONNULL((1, 2)) int (DCALL DeeObject_TGet32Bit)(DeeTypeObject *tp_self, DeeObject *self, int32_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(3) NONNULL((1, 2)) int (DCALL DeeObject_TGet64Bit)(DeeTypeObject *tp_self, DeeObject *self, int64_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(3) NONNULL((1, 2)) int (DCALL DeeObject_TAsDouble)(DeeTypeObject *tp_self, DeeObject *self, double *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TInt)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TInv)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TPos)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TNeg)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TAdd)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TSub)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TMul)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TDiv)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TMod)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TShl)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TShr)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TAnd)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TOr)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TXor)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TPow)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TInc)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TDec)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceAdd)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceSub)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceMul)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceDiv)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceMod)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceShl)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceShr)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceAnd)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceOr)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplaceXor)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TInplacePow)(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, DeeObject *some_object);

/* Compare operators */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_hash_t (DCALL DeeObject_THash)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCmpEq)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCmpNe)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCmpLo)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCmpLe)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCmpGr)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCmpGe)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TCompare)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TCompareEq)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TTryCompareEq)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *rhs);

/* Sequence operators */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_TForeach)(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_TForeachPair)(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TIter)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TIterNext)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TIterNextPair)(DeeTypeObject *tp_self, DeeObject *self, /*out*/ DREF DeeObject *key_and_value[2]);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TIterNextKey)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TIterNextValue)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t (DCALL DeeObject_TIterAdvance)(DeeTypeObject *tp_self, DeeObject *self, size_t step);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TSizeOb)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t (DCALL DeeObject_TSize)(DeeTypeObject *tp_self, DeeObject *self); /* @return: (size_t)-1: Error */
DFUNDEF WUNUSED NONNULL((1, 2)) size_t (DCALL DeeObject_TSizeFast)(DeeTypeObject *tp_self, DeeObject *self); /* @return: (size_t)-1: Fast size cannot be determined */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TContains)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TGetItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TTryGetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TTryGetItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TTryGetItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TTryGetItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TDelItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TSetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_TSetItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeObject_TSetItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeObject_TSetItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TGetRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TGetRangeIndex)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TGetRangeIndexN)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t begin);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TDelRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TDelRangeIndex)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TDelRangeIndexN)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t begin);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int (DCALL DeeObject_TSetRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *begin, DeeObject *end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeObject_TSetRangeIndex)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_TSetRangeIndexN)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t begin, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TBoundItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_THasItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_THasItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_THasItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_THasItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* Attribute operators */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TSetAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttrKw)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TCallAttrTuple)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TCallAttrTupleKw)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_THasAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TVCallAttrf)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, char const *format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeObject_TSetAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttrStringHashKw)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_THasAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeObject_TVCallAttrStringHashf)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash, char const *format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeObject_TSetAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttrStringLenHashKw)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_THasAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
//DFUNDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeObject_TVCallAttrStringLenHashf)(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, char const *format, va_list args);
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#define DeeObject_TCallAttrTuple(tp_self, self, attr, args)       DeeObject_TCallAttr(tp_self, self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_TCallAttrTupleKw(tp_self, self, attr, args, kw) DeeObject_TCallAttrKw(tp_self, self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */

/* With operators */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TEnter)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TLeave)(DeeTypeObject *tp_self, DeeObject *self);

/* Buffer operators */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TGetBuf)(DeeTypeObject *tp_self, DeeObject *self, DeeBuffer *__restrict info, unsigned int flags);

/* GC operators */
DFUNDEF NONNULL((1, 2, 3)) void (DCALL DeeObject_TVisit)(DeeTypeObject *tp_self, DeeObject *self, Dee_visit_t proc, void *arg);
DFUNDEF NONNULL((1, 2)) void (DCALL DeeObject_TClear)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF NONNULL((1, 2)) void (DCALL DeeObject_TPClear)(DeeTypeObject *tp_self, DeeObject *self, unsigned int gc_priority);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeObject_TAssign(tp_self, self, some_object)         __builtin_expect(DeeObject_TAssign(tp_self, self, some_object), 0)
#define DeeObject_TMoveAssign(tp_self, self, other)           __builtin_expect(DeeObject_TMoveAssign(tp_self, self, other), 0)
#define DeeObject_TInc(tp_self, p_self)                       __builtin_expect(DeeObject_TInc(tp_self, p_self), 0)
#define DeeObject_TDec(tp_self, p_self)                       __builtin_expect(DeeObject_TDec(tp_self, p_self), 0)
#define DeeObject_TInplaceAdd(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceAdd(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceSub(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceSub(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceMul(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceMul(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceDiv(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceDiv(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceMod(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceMod(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceShl(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceShl(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceShr(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceShr(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceAnd(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceAnd(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceOr(tp_self, p_self, some_object)    __builtin_expect(DeeObject_TInplaceOr(tp_self, p_self, some_object), 0)
#define DeeObject_TInplaceXor(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplaceXor(tp_self, p_self, some_object), 0)
#define DeeObject_TInplacePow(tp_self, p_self, some_object)   __builtin_expect(DeeObject_TInplacePow(tp_self, p_self, some_object), 0)
#define DeeObject_TDelItem(tp_self, self, index)              __builtin_expect(DeeObject_TDelItem(tp_self, self, index), 0)
#define DeeObject_TSetItem(tp_self, self, index, value)       __builtin_expect(DeeObject_TSetItem(tp_self, self, index, value), 0)
#define DeeObject_TDelRange(tp_self, self, begin, end)        __builtin_expect(DeeObject_TDelRange(tp_self, self, begin, end), 0)
#define DeeObject_TSetRange(tp_self, self, begin, end, value) __builtin_expect(DeeObject_TSetRange(tp_self, self, begin, end, value), 0)
#define DeeObject_TDelAttr(tp_self, self, attr)               __builtin_expect(DeeObject_TDelAttr(tp_self, self, attr), 0)
#define DeeObject_TSetAttr(tp_self, self, attr, value)        __builtin_expect(DeeObject_TSetAttr(tp_self, self, attr, value), 0)
#define DeeObject_TEnter(tp_self, self)                       __builtin_expect(DeeObject_TEnter(tp_self, self), 0)
#define DeeObject_TLeave(tp_self, self)                       __builtin_expect(DeeObject_TLeave(tp_self, self), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


DECL_END

#endif /* !GUARD_DEEMON_SUPER_H */
