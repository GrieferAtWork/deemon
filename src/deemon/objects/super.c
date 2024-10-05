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
#include <deemon/arg.h>
#include <deemon/attribute.h>
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
#define DEFINE_INTERNAL_SEQ_OPERATOR(return, name, args) \
	INTERN return DCALL DeeSeq_T##name SUPER_PRIVATE_EXPANDARGS args
#define DEFINE_INTERNAL_SET_OPERATOR(return, name, args) \
	INTERN return DCALL DeeSet_T##name SUPER_PRIVATE_EXPANDARGS args
#define DEFINE_INTERNAL_MAP_OPERATOR(return, name, args) \
	INTERN return DCALL DeeMap_T##name SUPER_PRIVATE_EXPANDARGS args
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
			if unlikely(!DeeType_Extends(DeeSuper_TYPE(self), tp_self))
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
			if (DeeObject_AssertTypeOrAbstract(ob, tp))
				goto err;
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


#define INVOKE_INPLACE_OPERATOR(callback)                             \
	Super *self = *p_self;                                            \
	int error;                                                        \
	DREF DeeObject *value = self->s_self;                             \
	Dee_Incref(value);                                                \
	error = callback;                                                 \
	if unlikely(error) {                                              \
		Dee_DecrefNokill(value);                                      \
	} else if (value == self->s_self) {                               \
		Dee_DecrefNokill(value);                                      \
	} else {                                                          \
		Dee_Decref(self);              /* Drop the old self-value. */ \
		*(DeeObject **)p_self = value; /* Inherit reference. */       \
	}                                                                 \
	return error

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
	return DeeObject_TCmpEq(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_ne(Super *self, DeeObject *some_object) {
	return DeeObject_TCmpNe(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_lo(Super *self, DeeObject *some_object) {
	return DeeObject_TCmpLo(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_le(Super *self, DeeObject *some_object) {
	return DeeObject_TCmpLe(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_gr(Super *self, DeeObject *some_object) {
	return DeeObject_TCmpGr(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_ge(Super *self, DeeObject *some_object) {
	return DeeObject_TCmpGe(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_compare_eq(Super *self, DeeObject *some_object) {
	return DeeObject_TCompareEq(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_compare(Super *self, DeeObject *some_object) {
	return DeeObject_TCompare(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_trycompare_eq(Super *self, DeeObject *some_object) {
	return DeeObject_TTryCompareEq(self->s_type, self->s_self, some_object);
}

PRIVATE struct type_cmp super_cmp = {
	/* .tp_hash          = */ (dhash_t (DCALL *)(DeeObject *__restrict))&super_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_trycompare_eq,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_ge,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_iter(Super *__restrict self) {
	return DeeObject_TIter(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_sizeob(Super *__restrict self) {
	return DeeObject_TSizeOb(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_contains(Super *self, DeeObject *some_object) {
	return DeeObject_TContains(self->s_type, self->s_self, some_object);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getitem(Super *self, DeeObject *index) {
	return DeeObject_TGetItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_delitem(Super *self, DeeObject *index) {
	return DeeObject_TDelItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
super_setitem(Super *self, DeeObject *index, DeeObject *value) {
	return DeeObject_TSetItem(self->s_type, self->s_self, index, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_getrange(Super *self, DeeObject *start, DeeObject *end) {
	return DeeObject_TGetRange(self->s_type, self->s_self, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
super_delrange(Super *self, DeeObject *start, DeeObject *end) {
	return DeeObject_TDelRange(self->s_type, self->s_self, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
super_setrange(Super *self, DeeObject *start, DeeObject *end, DeeObject *value) {
	return DeeObject_TSetRange(self->s_type, self->s_self, start, end, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_foreach(Super *me, Dee_foreach_t proc, void *arg) {
	return DeeObject_TForeach(me->s_type, me->s_self, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_foreach_pair(Super *me, Dee_foreach_pair_t proc, void *arg) {
	return DeeObject_TForeachPair(me->s_type, me->s_self, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_enumerate(Super *__restrict me, Dee_enumerate_t proc, void *arg) {
	return DeeObject_TEnumerate(me->s_type, me->s_self, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_enumerate_index(Super *__restrict me, Dee_enumerate_index_t proc,
                      void *arg, size_t start, size_t end) {
	return DeeObject_TEnumerateIndex(me->s_type, me->s_self, proc, arg, start, end);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_iterkeys(Super *__restrict self) {
	return DeeObject_TIterKeys(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_bounditem(Super *self, DeeObject *index) {
	return DeeObject_TBoundItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_hasitem(Super *self, DeeObject *index) {
	return DeeObject_THasItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
super_size(Super *self) {
	return DeeObject_TSize(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
super_size_fast(Super *self) {
	return DeeObject_TSizeFast(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_getitem_index(Super *self, size_t index) {
	return DeeObject_TGetItemIndex(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_delitem_index(Super *self, size_t index) {
	return DeeObject_TDelItemIndex(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
super_setitem_index(Super *self, size_t index, DeeObject *value) {
	return DeeObject_TSetItemIndex(self->s_type, self->s_self, index, value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_bounditem_index(Super *self, size_t index) {
	return DeeObject_TBoundItemIndex(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_hasitem_index(Super *self, size_t index) {
	return DeeObject_THasItemIndex(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_getrange_index(Super *self, size_t index) {
	return DeeObject_THasItemIndex(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_delrange_index(Super *self, Dee_ssize_t start, Dee_ssize_t end) {
	return DeeObject_TDelRangeIndex(self->s_type, self->s_self, start, end);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
super_setrange_index(Super *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	return DeeObject_TSetRangeIndex(self->s_type, self->s_self, start, end, values);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_getrange_index_n(Super *self, Dee_ssize_t start) {
	return DeeObject_TGetRangeIndexN(self->s_type, self->s_self, start);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
super_delrange_index_n(Super *self, Dee_ssize_t start) {
	return DeeObject_TDelRangeIndexN(self->s_type, self->s_self, start);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
super_setrange_index_n(Super *self, Dee_ssize_t start, DeeObject *values) {
	return DeeObject_TSetRangeIndexN(self->s_type, self->s_self, start, values);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_trygetitem(Super *self, DeeObject *index) {
	return DeeObject_TTryGetItem(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_trygetitem_index(Super *self, size_t index) {
	return DeeObject_TTryGetItemIndex(self->s_type, self->s_self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_trygetitem_string_hash(Super *self, char const *key, Dee_hash_t hash) {
	return DeeObject_TTryGetItemStringHash(self->s_type, self->s_self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getitem_string_hash(Super *self, char const *key, Dee_hash_t hash) {
	return DeeObject_TGetItemStringHash(self->s_type, self->s_self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_delitem_string_hash(Super *self, char const *key, Dee_hash_t hash) {
	return DeeObject_TDelItemStringHash(self->s_type, self->s_self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
super_setitem_string_hash(Super *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	return DeeObject_TSetItemStringHash(self->s_type, self->s_self, key, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_bounditem_string_hash(Super *self, char const *key, Dee_hash_t hash) {
	return DeeObject_TBoundItemStringHash(self->s_type, self->s_self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_hasitem_string_hash(Super *self, char const *key, Dee_hash_t hash) {
	return DeeObject_THasItemStringHash(self->s_type, self->s_self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_trygetitem_string_len_hash(Super *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_TTryGetItemStringLenHash(self->s_type, self->s_self, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getitem_string_len_hash(Super *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_TGetItemStringLenHash(self->s_type, self->s_self, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_delitem_string_len_hash(Super *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_TDelItemStringLenHash(self->s_type, self->s_self, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
super_setitem_string_len_hash(Super *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return DeeObject_TSetItemStringLenHash(self->s_type, self->s_self, key, keylen, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_bounditem_string_len_hash(Super *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_TBoundItemStringLenHash(self->s_type, self->s_self, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_hasitem_string_len_hash(Super *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_THasItemStringLenHash(self->s_type, self->s_self, key, keylen, hash);
}


PRIVATE struct type_seq super_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_setitem,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&super_setrange,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&super_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&super_foreach_pair,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&super_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&super_enumerate_index,
	/* .tp_iterkeys                   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&super_iterkeys,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&super_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&super_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&super_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&super_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&super_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&super_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&super_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&super_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&super_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&super_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&super_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&super_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&super_setrange_index_n,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&super_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&super_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&super_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_iternext(Super *__restrict self) {
	return DeeObject_TIterNext(self->s_type, self->s_self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getattr(Super *self, /*String*/ DeeObject *name) {
	return DeeObject_TGetAttr(self->s_type, self->s_self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_delattr(Super *self, /*String*/ DeeObject *name) {
	return DeeObject_TDelAttr(self->s_type, self->s_self, name);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
super_setattr(Super *self, /*String*/ DeeObject *name, DeeObject *value) {
	return DeeObject_TSetAttr(self->s_type, self->s_self, name, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
super_enumattr(DeeTypeObject *UNUSED(tp_self),
               Super *self, denum_t proc, void *arg) {
	return DeeObject_EnumAttr(self->s_type, self->s_self, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
super_findattr(DeeTypeObject *UNUSED(tp_self), Super *self,
               struct attribute_info *__restrict result,
               struct attribute_lookup_rules const *__restrict rules) {
	return DeeObject_FindAttr(self->s_type, self->s_self, result, rules);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_hasattr(Super *self, DeeObject *name) {
	return DeeObject_THasAttr(self->s_type, self->s_self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_boundattr(Super *self, DeeObject *name) {
	return DeeObject_TBoundAttr(self->s_type, self->s_self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_callattr(Super *self, DeeObject *name,
               size_t argc, DeeObject *const *argv) {
	return DeeObject_TCallAttr(self->s_type, self->s_self, name, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_callattr_kw(Super *self, DeeObject *name,
                  size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_TCallAttrKw(self->s_type, self->s_self, name, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_vcallattrf(Super *self, DeeObject *name,
                 char const *format, va_list args) {
	return DeeObject_TVCallAttrf(self->s_type, self->s_self, name, format, args);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getattr_string_hash(Super *self, char const *attr, Dee_hash_t hash) {
	return DeeObject_TGetAttrStringHash(self->s_type, self->s_self, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_delattr_string_hash(Super *self, char const *attr, Dee_hash_t hash) {
	return DeeObject_TDelAttrStringHash(self->s_type, self->s_self, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
super_setattr_string_hash(Super *self, char const *attr, Dee_hash_t hash, DeeObject *value) {
	return DeeObject_TSetAttrStringHash(self->s_type, self->s_self, attr, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_hasattr_string_hash(Super *self, char const *attr, Dee_hash_t hash) {
	return DeeObject_THasAttrStringHash(self->s_type, self->s_self, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_boundattr_string_hash(Super *self, char const *attr, Dee_hash_t hash) {
	return DeeObject_TBoundAttrStringHash(self->s_type, self->s_self, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_callattr_string_hash(Super *self, char const *attr, Dee_hash_t hash,
                           size_t argc, DeeObject *const *argv) {
	return DeeObject_TCallAttrStringHash(self->s_type, self->s_self, attr, hash, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_callattr_string_hash_kw(Super *self, char const *attr, Dee_hash_t hash,
                              size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_TCallAttrStringHashKw(self->s_type, self->s_self, attr, hash, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
super_vcallattr_string_hashf(Super *self, char const *attr, Dee_hash_t hash,
                             char const *format, va_list args) {
	return DeeObject_TVCallAttrStringHashf(self->s_type, self->s_self, attr, hash, format, args);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_getattr_string_len_hash(Super *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	return DeeObject_TGetAttrStringLenHash(self->s_type, self->s_self, attr, attrlen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_delattr_string_len_hash(Super *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	return DeeObject_TDelAttrStringLenHash(self->s_type, self->s_self, attr, attrlen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
super_setattr_string_len_hash(Super *self, char const *attr, size_t attrlen,
                              Dee_hash_t hash, DeeObject *value) {
	return DeeObject_TSetAttrStringLenHash(self->s_type, self->s_self, attr, attrlen, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_hasattr_string_len_hash(Super *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	return DeeObject_THasAttrStringLenHash(self->s_type, self->s_self, attr, attrlen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
super_boundattr_string_len_hash(Super *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	return DeeObject_TBoundAttrStringLenHash(self->s_type, self->s_self, attr, attrlen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_callattr_string_len_hash(Super *self, char const *attr, size_t attrlen,
                               Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	return DeeObject_TCallAttrStringLenHash(self->s_type, self->s_self, attr, attrlen, hash, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_callattr_string_len_hash_kw(Super *self, char const *attr, size_t attrlen,
                                  Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_TCallAttrStringLenHashKw(self->s_type, self->s_self, attr, attrlen, hash, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 6)) bool DCALL
super_findattr_info_string_len_hash(DeeTypeObject *tp_self, Super *self,
                                    char const *__restrict attr, size_t attrlen, Dee_hash_t hash,
                                    struct Dee_attrinfo *__restrict retinfo) {
	(void)tp_self;
	return DeeObject_TFindAttrInfoStringLenHash(self->s_type, self->s_self,
	                                            attr, attrlen, hash, retinfo);
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_callattr_tuple(Super *self, /*String*/ DeeObject *attr, DeeObject *args) {
	return DeeObject_TCallAttrTuple(self->s_type, self->s_self, attr, args);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_callattr_tuple_kw(Super *self, /*String*/ DeeObject *attr, DeeObject *args, DeeObject *kw) {
	return DeeObject_TCallAttrTupleKw(self->s_type, self->s_self, attr, args, kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */


PRIVATE struct type_attr super_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&super_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&super_delattr,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&super_setattr,
	/* .tp_enumattr                      = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&super_enumattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct attribute_info *__restrict, struct attribute_lookup_rules const *__restrict))&super_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&super_boundattr,
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&super_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&super_callattr_kw,
	/* .tp_vcallattrf                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, char const *, va_list))&super_vcallattrf,
	/* .tp_getattr_string_hash           = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_getattr_string_hash,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_delattr_string_hash,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&super_setattr_string_hash,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_hasattr_string_hash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&super_boundattr_string_hash,
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&super_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&super_callattr_string_hash_kw,
	/* .tp_vcallattr_string_hashf        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, char const *, va_list))&super_vcallattr_string_hashf,
	/* .tp_getattr_string_len_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_getattr_string_len_hash,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&super_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&super_boundattr_string_len_hash,
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&super_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&super_callattr_string_len_hash_kw,
	/* .tp_findattr_info_string_len_hash = */ (bool (DCALL *)(DeeTypeObject *, DeeObject *, char const *__restrict, size_t, Dee_hash_t, struct Dee_attrinfo *__restrict))&super_findattr_info_string_len_hash,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_callattr_tuple                = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&super_callattr_tuple,
	/* .tp_callattr_tuple_kw             = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&super_callattr_tuple_kw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
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
	/* This function should never get called, but implement it anyways. */
	DeeTypeObject *tp_self = self->s_type;
	DeeObject *ob_self     = self->s_self;
	do {
		if (tp_self->tp_buffer && tp_self->tp_buffer->tp_getbuf) {
			if (tp_self->tp_buffer->tp_putbuf)
				(*tp_self->tp_buffer->tp_putbuf)(ob_self, info, flags);
			break;
		}
	} while (DeeType_InheritBuffer(tp_self));
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
	/* .tp_iterator      = */ NULL,
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
