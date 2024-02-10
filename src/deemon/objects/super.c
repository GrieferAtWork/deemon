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
#ifndef GUARD_DEEMON_OBJECTS_SUPER_C
#define GUARD_DEEMON_OBJECTS_SUPER_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/super.h>

#include "../runtime/strings.h"

/* Define type-specific object operators. */
#define DEFINE_TYPED_OPERATORS 1
#define SUPER_PRIVATE_EXPANDARGS(...) (DeeTypeObject *tp_self, __VA_ARGS__)
#define DEFINE_OPERATOR(return, name, args) \
	PUBLIC return DCALL DeeObject_T##name SUPER_PRIVATE_EXPANDARGS args
#define DEFINE_INTERNAL_OPERATOR(return, name, args) \
	INTERN return DCALL DeeObject_T##name SUPER_PRIVATE_EXPANDARGS args
#include "../runtime/operator.c"
#undef DEFINE_TYPED_OPERATORS


DECL_BEGIN

typedef DeeSuperObject Super;

/* Create a new super-wrapper for `tp_self:self'.
 * NOTE: This function automatically checks the given operands for validity:
 *        - DeeType_Check(tp_self);
 *        - DeeObject_InstanceOf(self, tp_self);
 * It also automatically unwraps `self' should it already be a super-object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSuper_New(DeeTypeObject *tp_self, DeeObject *self) {
	DREF Super *result;
	if (tp_self == (DeeTypeObject *)Dee_None)
		tp_self = &DeeNone_Type;
	if (DeeObject_AssertType(tp_self, &DeeType_Type))
		goto err;
	if (DeeSuper_Check(self)) {
		if (!DeeType_IsAbstract(tp_self)) {
			if unlikely(!DeeType_InheritsFrom(DeeSuper_TYPE(self), tp_self))
				goto err_badtype;
		}
		self = DeeSuper_SELF(self);
	} else {
		if (!DeeType_IsAbstract(tp_self)) {
			if (DeeObject_AssertType(self, tp_self))
				goto err;
		}
	}
	ASSERT(!DeeSuper_Check(self));

	/* Allocate + construct a new super-object. */
	result = DeeObject_MALLOC(Super);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeSuper_Type);
	result->s_type = tp_self;
	result->s_self = self;
	Dee_Incref(tp_self);
	Dee_Incref(self);
	return (DREF DeeObject *)result;
err_badtype:
	DeeObject_TypeAssertFailed(self, tp_self);
err:
	return NULL;
}

/* Taking some object, return the effective super-class of it.
 * HINT: When `self' is another super-object, this is identical to
 *       `DeeSuper_New(DeeType_BASE(DeeSuper_TYPE(self)), DeeSuper_SELF(self))'
 * @throws: Error.TypeError: The class of `self' has no super-class. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSuper_Of(DeeObject *__restrict self) {
	DeeTypeObject *base;
	ASSERT_OBJECT(self);
	if (DeeSuper_Check(self)) {
		/* Dereference an existing super-object. */
		base = DeeType_Base(DeeSuper_TYPE(self));
		if unlikely(!base)
			goto nosuper;
		if (!DeeObject_IsShared(self)) {
			DeeSuper_TYPE(self) = base;
			return_reference_(self);
		}
		return DeeSuper_New(base, DeeSuper_SELF(self));
	}

	/* Create a new super-object for the base-type of the given object. */
	base = DeeType_Base(Dee_TYPE(self));
	if unlikely(!base)
		goto nosuper;
	return DeeSuper_New(base, self);
nosuper:

	/* Special case: There is no super-class to dereference. */
	err_no_super_class(DeeSuper_Check(self)
	                   ? DeeSuper_TYPE(self)
	                   : Dee_TYPE(self));
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
super_ctor(Super *__restrict self) {
	self->s_type = &DeeNone_Type;
	self->s_self = Dee_None;
	Dee_Incref(&DeeNone_Type);
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_copy(Super *__restrict self,
           Super *__restrict other) {
	ASSERT_OBJECT(self);
	self->s_self = DeeObject_TCopy(other->s_type, other->s_self);
	if unlikely(!self->s_self)
		goto err;
	self->s_type = Dee_TYPE(self->s_self);
	Dee_Incref(self->s_type);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_deepcopy(Super *__restrict self,
               Super *__restrict other) {
	ASSERT_OBJECT(self);
	self->s_self = DeeObject_TDeepCopy(other->s_type, other->s_self);
	if unlikely(!self->s_self)
		goto err;
	self->s_type = Dee_TYPE(self->s_self);
	Dee_Incref(self->s_type);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_init(Super *self, size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	DeeTypeObject *tp = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:Super", &ob, &tp))
		goto err;

	/* Special handling when the base-object is another super-object. */
	if (DeeSuper_Check(ob)) {
		if (!tp) {
			self->s_type = DeeType_Base(DeeSuper_TYPE(self));
			if unlikely(!self->s_type)
				goto err_nosuper;
			self->s_self = DeeSuper_SELF(self);
			Dee_Incref(self->s_type);
			Dee_Incref(self->s_self);
			return 0;
		}
		ob = DeeSuper_SELF(ob);
	}
	if (tp) {
		if (tp == (DeeTypeObject *)Dee_None) {
			tp = &DeeNone_Type;
		} else {
			/* Make sure the passed type matches. */
			if (DeeObject_AssertType(tp, &DeeType_Type))
				goto err;
			if (!DeeType_IsAbstract(tp)) {
				if (DeeObject_AssertType(ob, tp))
					goto err;
			}
		}
	} else {
		tp = DeeType_Base(Dee_TYPE(ob));
		if unlikely(!tp)
			goto err_nosuper;
	}
	self->s_self = ob;
	self->s_type = tp;
	Dee_Incref(ob);
	Dee_Incref(tp);
	return 0;
err_nosuper:

	/* Special case: There is no super-class to dereference. */
	err_no_super_class(DeeSuper_Check(self)
	                   ? DeeSuper_TYPE(self)
	                   : Dee_TYPE(self));
err:
	return -1;
}


INTERN NONNULL((1)) void DCALL
super_fini(Super *__restrict self) {
	Dee_Decref(self->s_self);
	Dee_Decref(self->s_type);
}

INTERN NONNULL((1, 2)) void DCALL
super_visit(Super *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->s_self);
	Dee_Visit(self->s_type);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_assign(Super *self, DeeObject *some_object) {
	return DeeObject_TAssign(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_moveassign(Super *self, DeeObject *some_object) {
	return DeeObject_TMoveAssign(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_str(Super *__restrict self) {
	return DeeObject_TStr(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_repr(Super *__restrict self) {
	return DeeObject_TRepr(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_bool(Super *__restrict self) {
	return DeeObject_TBool(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
super_print(Super *__restrict self, dformatprinter printer, void *arg) {
	return DeeObject_TPrint(self->s_type, self->s_self, printer, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
super_printrepr(Super *__restrict self, dformatprinter printer, void *arg) {
	return DeeObject_TPrintRepr(self->s_type, self->s_self, printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_call(Super *self, size_t argc, DeeObject *const *argv) {
	return DeeObject_TCall(self->s_type, self->s_self, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_call_kw(Super *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_TCallKw(self->s_type, self->s_self, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
super_hash(Super *__restrict self) {
	return DeeObject_THash(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_int32(Super *__restrict self,
            int32_t *__restrict result) {
	return DeeObject_TGet32Bit(self->s_type, self->s_self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_int64(Super *__restrict self,
            int64_t *__restrict result) {
	return DeeObject_TGet64Bit(self->s_type, self->s_self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_double(Super *__restrict self,
             double *__restrict result) {
	return DeeObject_TAsDouble(self->s_type, self->s_self, result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_int(Super *__restrict self) {
	return DeeObject_TInt(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_inv(Super *__restrict self) {
	return DeeObject_TInv(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_pos(Super *__restrict self) {
	return DeeObject_TPos(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_neg(Super *__restrict self) {
	return DeeObject_TNeg(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_add(Super *self, DeeObject *some_object) {
	return DeeObject_TAdd(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_sub(Super *self, DeeObject *some_object) {
	return DeeObject_TSub(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mul(Super *self, DeeObject *some_object) {
	return DeeObject_TMul(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_div(Super *self, DeeObject *some_object) {
	return DeeObject_TDiv(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mod(Super *self, DeeObject *some_object) {
	return DeeObject_TMod(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_shl(Super *self, DeeObject *some_object) {
	return DeeObject_TShl(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_shr(Super *self, DeeObject *some_object) {
	return DeeObject_TShr(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_and(Super *self, DeeObject *some_object) {
	return DeeObject_TAnd(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_or(Super *self, DeeObject *some_object) {
	return DeeObject_TOr(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_xor(Super *self, DeeObject *some_object) {
	return DeeObject_TXor(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_pow(Super *self, DeeObject *some_object) {
	return DeeObject_TPow(self->s_type, self->s_self, some_object);
}


#define INVOKE_INPLACE_OPERATOR(callback)                         \
	Super *self = *p_self;                                        \
	int error;                                                    \
	DREF DeeObject *value = self->s_self;                         \
	Dee_Incref(value);                                            \
	error = callback;                                             \
	if unlikely(error) {                                          \
		Dee_Decref(value);                                        \
		return error;                                             \
	}                                                             \
	if (value == self->s_self) {                                  \
		Dee_Decref(value);                                        \
	} else {                                                      \
		/* Create a new super-wrapper for the updated value. */   \
		if (DeeObject_InstanceOf(value, self->s_type)) {          \
			self = (Super *)DeeSuper_New(self->s_type, value);    \
		} else {                                                  \
			self = (Super *)DeeSuper_New(Dee_TYPE(value), value); \
		}                                                         \
		Dee_Decref(value);                                        \
		if unlikely(!self)                                        \
			return -1;                                            \
		Dee_Decref(*p_self); /* Drop the old self-value. */       \
		*p_self = self;      /* Inherit reference. */             \
	}                                                             \
	return 0

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_inc(Super **__restrict p_self) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInc(self->s_type, &value));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_dec(Super **__restrict p_self) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TDec(self->s_type, &value));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_add(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceAdd(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_sub(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceSub(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_mul(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceMul(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_div(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceDiv(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_mod(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceMod(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_shl(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceShl(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_shr(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceShr(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_and(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceAnd(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_or(Super **__restrict p_self,
                 DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceOr(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_xor(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplaceXor(self->s_type, &value, some_object));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_inplace_pow(Super **__restrict p_self,
                  DeeObject *some_object) {
	INVOKE_INPLACE_OPERATOR(DeeObject_TInplacePow(self->s_type, &value, some_object));
}
#undef INVOKE_INPLACE_OPERATOR

PRIVATE struct type_math super_math = {
	/* .tp_int32       = */ (int (DCALL *)(DeeObject *__restrict, int32_t *__restrict))&super_int32,
	/* .tp_int64       = */ (int (DCALL *)(DeeObject *__restrict, int64_t *__restrict))&super_int64,
	/* .tp_double      = */ (int (DCALL *)(DeeObject *__restrict, double *__restrict))&super_double,
	/* .tp_int         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_int,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_pow,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&super_inc,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&super_dec,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_add,
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_sub,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_mul,
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_div,
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_mod,
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_shl,
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_shr,
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_xor,
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))super_inplace_pow
};


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_eq(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareEqObject(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_ne(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareNeObject(self->s_type, self->s_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_lo(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareLoObject(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_le(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareLeObject(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_gr(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareGrObject(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_ge(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareGeObject(self->s_type, self->s_self, some_object);
}

PRIVATE struct type_cmp super_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&super_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_ge
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_iter_self(Super *__restrict self) {
	return DeeObject_TIterSelf(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_size(Super *__restrict self) {
	return DeeObject_TSizeObject(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_contains(Super *self, DeeObject *some_object) {
	return DeeObject_TContainsObject(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_get(Super *self, DeeObject *index) {
	return DeeObject_TGetItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_del(Super *self, DeeObject *index) {
	return DeeObject_TDelItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
super_set(Super *self, DeeObject *index, DeeObject *value) {
	return DeeObject_TSetItem(self->s_type, self->s_self, index, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_range_get(Super *self, DeeObject *begin, DeeObject *end) {
	return DeeObject_TGetRange(self->s_type, self->s_self, begin, end);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
super_range_del(Super *self, DeeObject *begin, DeeObject *end) {
	return DeeObject_TDelRange(self->s_type, self->s_self, begin, end);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
super_range_set(Super *self, DeeObject *begin, DeeObject *end, DeeObject *value) {
	return DeeObject_TSetRange(self->s_type, self->s_self, begin, end, value);
}


PRIVATE struct type_seq super_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_iter_self,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_get,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_del,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_set,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_range_get,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_range_del,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&super_range_set
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_iternext(Super *__restrict self) {
	return DeeObject_TIterNext(self->s_type, self->s_self);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getattr(Super *self, /*String*/ DeeObject *name) {
	return DeeObject_TGetAttr(self->s_type, self->s_self, name);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_delattr(Super *self, /*String*/ DeeObject *name) {
	return DeeObject_TDelAttr(self->s_type, self->s_self, name);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_setattr(Super *self, /*String*/ DeeObject *name, DeeObject *value) {
	return DeeObject_TSetAttr(self->s_type, self->s_self, name, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
super_enumattr(DeeTypeObject *UNUSED(tp_self),
               Super *self, denum_t proc, void *arg) {
	return DeeObject_EnumAttr(self->s_type, self->s_self, proc, arg);
}

PRIVATE struct type_attr tpconst super_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&super_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&super_delattr,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&super_setattr,
	/* .tp_enumattr = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&super_enumattr
};

PRIVATE WUNUSED NONNULL((1)) int DCALL super_enter(Super *__restrict self) {
	return DeeObject_TEnter(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL super_leave(Super *__restrict self) {
	return DeeObject_TLeave(self->s_type, self->s_self);
}

PRIVATE struct type_with super_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&super_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&super_leave
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_getbuf(Super *__restrict self,
             DeeBuffer *__restrict info,
             unsigned int flags) {
	return DeeObject_TGetBuf(self->s_type, self->s_self, info, flags);
}

PRIVATE NONNULL((1)) void DCALL
super_putbuf(Super *__restrict self,
             DeeBuffer *__restrict info,
             unsigned int flags) {
	DeeObject_TPutBuf(self->s_type, self->s_self, info, flags);
}

PRIVATE struct type_buffer super_buffer = {
	/* .tp_getbuf = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&super_getbuf,
	/* .tp_putbuf = */ (void (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&super_putbuf
};


/* Helper functions for extracting the type
 * and self fields of a super object. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_typeof(DeeObject *UNUSED(self),
             size_t argc, DeeObject *const *argv) {
	Super *super_object;
	if (DeeArg_Unpack(argc, argv, "o:typeof", &super_object))
		goto err;
	if (DeeObject_AssertTypeExact(super_object, &DeeSuper_Type))
		goto err;
	return_reference_((DeeObject *)super_object->s_type);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_selfof(DeeObject *UNUSED(self),
             size_t argc, DeeObject *const *argv) {
	Super *super_object;
	if (DeeArg_Unpack(argc, argv, "o:selfof", &super_object))
		goto err;
	if (DeeObject_AssertTypeExact(super_object, &DeeSuper_Type))
		goto err;
	return_reference_((DeeObject *)super_object->s_self);
err:
	return NULL;
}

PRIVATE struct type_method tpconst super_class_methods[] = {
	TYPE_METHOD("typeof", &super_typeof,
	            "(ob:?.)->?DType\n"
	            "#tTypeError{@ob is not a super-object}"
	            "#r{the type of a given super-view @ob}"),
	TYPE_METHOD("selfof", &super_selfof,
	            "(ob:?.)->\n"
	            "#tTypeError{@ob is not a super-object}"
	            "#r{the object of a given super-view @ob}"),
	TYPE_METHOD_END
};


PUBLIC DeeTypeObject DeeSuper_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Super),
	/* .tp_doc      = */ DOC("Emulate access to an instance of a derived class in "
	                         /**/ "a way that bypasses overwritten operators/methods\n"

	                         "\n"
	                         "()\n"
	                         "Same as ${Super(none, type none)}\n"

	                         "\n"
	                         "(ob:?.)\n"
	                         "Same as ${Super(Super.selfof(ob), Type.__base__(Super.typeof(ob))}\n"
	                         "#tTypeError{The class of @ob has no base-class}"

	                         "\n"
	                         "(ob)\n"
	                         "#tTypeError{The type of @ob has no base-class}"
	                         "Same as ${Super(ob, Type.__base__(type(ob))}\n"

	                         "\n"
	                         "(ob,tp:?DType)\n"
	                         "#tTypeError{The given object @ob is not an instance of @tp}"
	                         "Creates a new super-view for @ob as an instance of @tp\n"
	                         "When @ob is another super-view, ${Super.selfof(ob)} is used instead\n"

	                         "\n"
	                         ":=->\n"
	                         "move:=->\n"
	                         "str->\n"
	                         "repr->\n"
	                         "bool->\n"
	                         "int->\n"
	                         "call->\n"
	                         "~->?O\n"
	                         "+->?O\n"
	                         "+(other)->?O\n"
	                         "-->?O\n"
	                         "-(other)->?O\n"
	                         "*(other)->?O\n"
	                         "/(other)->?O\n"
	                         "%(other)->?O\n"
	                         "<<(shift)->?O\n"
	                         ">>(shift)->?O\n"
	                         "&(other)->?O\n"
	                         "|(other)->?O\n"
	                         "^(other)->?O\n"
	                         "**(other)->?O\n"
	                         "+=(other)->\n"
	                         "-=(other)->\n"
	                         "*=(other)->\n"
	                         "/=(other)->\n"
	                         "%=(other)->\n"
	                         "<<=(shift)->\n"
	                         ">>=(shift)->\n"
	                         "&=(other)->\n"
	                         "|=(other)->\n"
	                         "^=(other)->\n"
	                         "**=(other)->\n"
	                         "==(other)->?Dbool\n"
	                         "==(other)->?O\n"
	                         "!=(other)->?Dbool\n"
	                         "!=(other)->?O\n"
	                         "<(other)->?Dbool\n"
	                         "<(other)->?O\n"
	                         "<=(other)->?Dbool\n"
	                         "<=(other)->?O\n"
	                         ">(other)->?Dbool\n"
	                         ">(other)->?O\n"
	                         ">=(other)->?Dbool\n"
	                         ">=(other)->?O\n"
	                         "iter(other)->?O\n"
	                         "next(other)->?O\n"
	                         "size->?O\n"
	                         "contains(item)->?O\n"
	                         "[](key)->?O\n"
	                         "[]->?O\n"
	                         "del[](key)->\n"
	                         "del[]->\n"
	                         "[]=(key,value)=->\n"
	                         "[]=->\n"
	                         "[:]->\n"
	                         "del[:]->\n"
	                         "[:]=->\n"
	                         "enter->\n"
	                         "leave->\n"
	                         ".->\n"
	                         "del.->\n"
	                         ".=->\n"
	                         "Invoke the same operator implemented by the first "
	                         /**/ "reachable type, following regular method resolution order"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FMOVEANY | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&super_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&super_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&super_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&super_init,
				TYPE_FIXED_ALLOCATOR(Super)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&super_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_str,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&super_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&super_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&super_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&super_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&super_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &super_math,
	/* .tp_cmp           = */ &super_cmp,
	/* .tp_seq           = */ &super_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_iternext,
	/* .tp_attr          = */ &super_attr,
	/* .tp_with          = */ &super_with,
	/* .tp_buffer        = */ &super_buffer,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ super_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&super_call_kw,
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SUPER_C */
