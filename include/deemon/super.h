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
#ifndef GUARD_DEEMON_SUPER_H
#define GUARD_DEEMON_SUPER_H 1

#include "api.h"

#include <stddef.h>

#include "object.h"

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

#define DeeSuper_TYPE(x) ((DeeSuperObject *)Dee_REQUIRES_OBJECT(x))->s_type
#define DeeSuper_SELF(x) ((DeeSuperObject *)Dee_REQUIRES_OBJECT(x))->s_self

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

DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TCopy(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDeepCopy(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TAssign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TMoveAssign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TStr(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TRepr(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TBool(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TCall(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TCallKw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TThisCall(DeeTypeObject *tp_self, DeeObject *self, DeeObject *this_arg, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TThisCallKw(DeeTypeObject *tp_self, DeeObject *self, DeeObject *this_arg, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED /*ATTR_PURE*/ NONNULL((1, 2)) Dee_hash_t (DCALL DeeObject_THash)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED ATTR_OUT(3) NONNULL((1, 2)) int (DCALL DeeObject_TGet32Bit)(DeeTypeObject *tp_self, DeeObject *__restrict self, int32_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(3) NONNULL((1, 2)) int (DCALL DeeObject_TGet64Bit)(DeeTypeObject *tp_self, DeeObject *__restrict self, int64_t *__restrict result);
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
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCompareEqObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCompareNeObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCompareLoObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCompareLeObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCompareGrObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCompareGeObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TSizeObject)(DeeTypeObject *tp_self, DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TContainsObject)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TSetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TGetRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TDelRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int (DCALL DeeObject_TSetRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *begin, DeeObject *end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_TPrint)(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_TPrintRepr)(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TIterSelf)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TIterNext)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TGetAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TDelAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TSetAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_TCallAttrKw)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
#if defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) || defined(__OPTIMIZE_SIZE__)
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TCallAttrTuple)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeObject_TCallAttrTupleKw)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args, DeeObject *kw);
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS || __OPTIMIZE_SIZE__ */
#define DeeObject_TCallAttrTuple(tp_self, self, attr, args)       DeeObject_TCallAttr(tp_self, self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_TCallAttrTupleKw(tp_self, self, attr, args, kw) DeeObject_TCallAttrKw(tp_self, self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TBoundAttr)(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TEnter)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_TLeave)(DeeTypeObject *tp_self, DeeObject *self);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_TGetBuf)(DeeTypeObject *tp_self, DeeObject *self, DeeBuffer *__restrict info, unsigned int flags);

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
