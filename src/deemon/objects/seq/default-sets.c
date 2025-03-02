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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/thread.h>
#include <deemon/util/lock.h>

#include <hybrid/limitcore.h>
/**/

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-sets.h"
/**/

#include <stddef.h> /* size_t */

#undef SSIZE_MIN
#define SSIZE_MIN __SSIZE_MIN__

DECL_BEGIN





/* ================================================================================ */
/*   SET INVERSION                                                                  */
/* ================================================================================ */

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
invset_hash(SetInversion *__restrict self) {
	return ~DeeSet_OperatorHash(self->si_set);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
invset_compare_eq(SetInversion *self, DeeObject *rhs) {
	if (SetInversion_Check(rhs)) {
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeSeq_OperatorCompareEq(self->si_set, xrhs->si_set);
	}
	return 1; /* not equal */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
invset_trycompare_eq(SetInversion *self, DeeObject *rhs) {
	if (SetInversion_Check(rhs)) {
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeSeq_OperatorTryCompareEq(self->si_set, xrhs->si_set);
	}
	return 1; /* not equal */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_lo(SetInversion *self, DeeObject *rhs) {
	/* >> self.TRUE_SUBSET_OF(rhs) */
	if (SetInversion_Check(rhs)) {
		/* ~lhs < ~rhs   <===>   lhs > rhs */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeSet_OperatorGr(self->si_set, xrhs->si_set);
	}
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_le(SetInversion *self, DeeObject *rhs) {
	/* >> self.SUBSET_OF_OR_EQUAL(rhs) */
	if (SetInversion_Check(rhs)) {
		/* ~lhs <= ~rhs   <===>   lhs >= rhs */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeSet_OperatorGe(self->si_set, xrhs->si_set);
	}
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_gr(SetInversion *self, DeeObject *rhs) {
	/* >> self.TRUE_SUPERSET_OF(rhs) */
	if (SetInversion_Check(rhs)) {
		/* ~lhs > ~rhs   <===>   lhs < rhs */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeSet_OperatorLo(self->si_set, xrhs->si_set);
	}
	return_true;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_ge(SetInversion *self, DeeObject *rhs) {
	/* >> self.SUPERSET_OF_OR_EQUAL(rhs) */
	if (SetInversion_Check(rhs)) {
		/* ~lhs >= ~rhs   <===>   lhs <= rhs */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeSet_OperatorLe(self->si_set, xrhs->si_set);
	}
	return_true;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
invset_ctor(SetInversion *__restrict self) {
	self->si_set = Dee_EmptySet;
	Dee_Incref(Dee_EmptySet);
	return 0;
}


STATIC_ASSERT(offsetof(SetInversion, si_set) == offsetof(ProxyObject, po_obj));
#define invset_init  generic_proxy__init
#define invset_fini  generic_proxy__fini
#define invset_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
invset_printrepr(SetInversion *__restrict self,
                 dformatprinter printer, void *arg) {
	if (DeeObject_Implements(self->si_set, &DeeSet_Type))
		return DeeFormat_Printf(printer, arg, "~%r", self->si_set);
	return DeeFormat_Printf(printer, arg, "Set.__inv__(%r)", self->si_set);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_contains(SetInversion *self, DeeObject *key) {
	int result = DeeSet_OperatorContainsAsBool(self->si_set, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE struct type_seq invset_seq = {
	/* .tp_iter     = */ DEFIMPL(&default__set_operator_iter__unsupported),
	/* .tp_sizeob   = */ DEFIMPL(&default__set_operator_sizeob__unsupported),
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_contains,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_foreach         = */ DEFIMPL(&default__set_operator_foreach__with__set_operator_iter),
	/* .tp_foreach_pair    = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_enumerate       = */ NULL,
	/* .tp_enumerate_index = */ NULL,
	/* .tp_iterkeys        = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ DEFIMPL(&default__set_operator_size__unsupported),
};

PRIVATE struct type_member tpconst invset_members[] = {
	TYPE_MEMBER_FIELD_DOC("__blacklist__", STRUCT_OBJECT, offsetof(SetInversion, si_set), "->?DSet"),
	TYPE_MEMBER_END
};

PRIVATE struct type_cmp invset_cmp = {
	/* .tp_hash          = */ (dhash_t (DCALL *)(DeeObject *__restrict))&invset_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&invset_compare_eq,
	/* .tp_compare       = */ DEFIMPL(&default__compare_eq__with__lo__and__gr),
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&invset_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_ge,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
invset_getset(SetInversion *__restrict self) {
	return_reference_(self->si_set);
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_operator_add(SetInversion *self, DeeObject *some_object) {
	if (SetInversion_Check(some_object)) {
		/* (~a | ~b)  <=>  ~(a & b) */
		SetInversion *xrhs = (SetInversion *)some_object;
		DREF DeeObject *intersection;
		intersection = DeeSet_OperatorAnd(self->si_set, xrhs->si_set);
		if unlikely(!intersection)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(intersection);
	} else {
		/* (~a | b)  <=>  ~(a & ~b) */
		DREF SetInversion *b_inv;
		DREF SetIntersection *intersection;
		b_inv = SetInversion_New(some_object);
		if unlikely(!b_inv)
			goto err;
		intersection = SetIntersection_New_inherit_b(self->si_set, b_inv);
		if unlikely(!intersection)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(intersection);
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_operator_and(SetInversion *self, DeeObject *some_object);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_operator_sub(SetInversion *self, DeeObject *some_object) {
	if (SetInversion_Check(some_object)) {
		/* (~a - ~b)  <=>  (~a & b) */
		SetInversion *xrhs = (SetInversion *)some_object;
		return invset_operator_and(self, xrhs->si_set);
	} else {
		/* (~a - b)  <=>  (~a & ~b)  <=>  ~(a | b) */
		DREF SetUnion *su = SetUnion_New(self->si_set, some_object);
		if unlikely(!su)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(su);
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_operator_and(SetInversion *self, DeeObject *some_object) {
	if (SetInversion_Check(some_object)) {
		/* (~a & ~b)  <=>  (~a - b) */
		SetInversion *xrhs = (SetInversion *)some_object;
		return invset_operator_sub(self, xrhs->si_set);
	} else {
		/* (~a & b)  <=>  (~a & b)  <=>  ~(a | ~b) */
		DREF SetInversion *b_inv;
		DREF SetUnion *su;
		b_inv = SetInversion_New(some_object);
		if unlikely(!b_inv)
			goto err;
		su = SetUnion_New_inherit_b(self->si_set, b_inv);
		if unlikely(!su)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(su);
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_operator_xor(SetInversion *self, DeeObject *some_object) {
	/* (~a ^ b)  <=>  ~(a ^ b) */
	DREF SetSymmetricDifference *ssd;
	ssd = SetSymmetricDifference_New(self->si_set, some_object);
	if unlikely(!ssd)
		goto err;
	return (DREF DeeObject *)SetInversion_New_inherit(ssd);
err:
	return NULL;
}


PRIVATE struct type_math invset_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&invset_getset,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_operator_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_operator_sub,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_operator_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_operator_add,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_operator_xor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL(&default__dec__with__sub),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__inplace_sub__with__sub),
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ DEFIMPL(&default__inplace_and__with__and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__inplace_or__with__or),
	/* .tp_inplace_xor = */ DEFIMPL(&default__inplace_xor__with__xor),
	/* .tp_inplace_pow = */ NULL,
};

INTERN DeeTypeObject SetInversion_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetInversion",
	/* .tp_doc      = */ DOC("()\n"
	                         "(s:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&invset_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&invset_init,
				TYPE_FIXED_ALLOCATOR(SetInversion)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&invset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL(&default__seq_operator_bool__with__set_operator_compare_eq),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&invset_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&invset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &invset_math,
	/* .tp_cmp           = */ &invset_cmp,
	/* .tp_seq           = */ &invset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ invset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};



/* A universal set is the inverse of an empty set */
DDATDEF SetInversion DeeSet_UniversalInstance;
PUBLIC SetInversion DeeSet_UniversalInstance = {
	OBJECT_HEAD_INIT(&SetInversion_Type),
	/* .si_set = */ Dee_EmptySet
};





/* ================================================================================ */
/*   SET UNION                                                                      */
/* ================================================================================ */
STATIC_ASSERT(offsetof(SetUnion, su_a) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetUnion, su_a) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SetUnion, su_b) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetUnion, su_b) == offsetof(ProxyObject2, po_obj2));
#define su_fini  generic_proxy2__fini
#define su_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetUnionIterator, sui_union) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetUnionIterator, sui_iter) == offsetof(ProxyObject2, po_obj2));
#define suiter_fini generic_proxy2__fini

PRIVATE NONNULL((1)) void DCALL
suiter_clear(SetUnionIterator *__restrict self) {
	DREF DeeObject *iter;
	SetUnionIterator_LockWrite(self);
	iter = self->sui_iter;
	Dee_Incref(Dee_None);
	self->sui_iter = Dee_None;
	SetUnionIterator_LockEndWrite(self);
	Dee_Decref(iter);
}

PRIVATE struct type_gc suiter_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *))&suiter_clear
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
suiter_get_iter(SetUnionIterator *__restrict self) {
	DREF DeeObject *result;
	SetUnionIterator_LockRead(self);
	result = self->sui_iter;
	Dee_Incref(result);
	SetUnionIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_set_iter(SetUnionIterator *__restrict self,
                DeeObject *__restrict iter) {
	DREF DeeObject *old_iter;
	Dee_Incref(iter);
	SetUnionIterator_LockRead(self);
	old_iter = self->sui_iter;
	self->sui_iter = iter;
	SetUnionIterator_LockEndRead(self);
	Dee_Decref(old_iter);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
suiter_get_in2nd(SetUnionIterator *__restrict self) {
	bool result;
	COMPILER_BARRIER();
	result = self->sui_in2nd;
	COMPILER_BARRIER();
	return_bool(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_set_in2nd(SetUnionIterator *__restrict self,
                 DeeObject *__restrict value) {
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		return newval;
	SetUnionIterator_LockWrite(self);
	self->sui_in2nd = !!newval;
	SetUnionIterator_LockEndWrite(self);
	return 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
suiter_visit(SetUnionIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sui_union);
	SetUnionIterator_LockRead(self);
	Dee_Visit(self->sui_iter);
	SetUnionIterator_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_copy(SetUnionIterator *__restrict self,
            SetUnionIterator *__restrict other) {
	DREF DeeObject *iter;
	SetUnionIterator_LockRead(other);
	iter            = other->sui_iter;
	self->sui_in2nd = other->sui_in2nd;
	Dee_Incref(iter);
	SetUnionIterator_LockEndRead(other);
	self->sui_iter = DeeObject_Copy(iter);
	Dee_Decref(iter);
	if unlikely(!self->sui_iter)
		goto err;
	Dee_atomic_rwlock_init(&self->sui_lock);
	self->sui_union = other->sui_union;
	Dee_Incref(self->sui_union);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_deep(SetUnionIterator *__restrict self,
            SetUnionIterator *__restrict other) {
	DREF DeeObject *iter;
	SetUnionIterator_LockRead(other);
	iter            = other->sui_iter;
	self->sui_in2nd = other->sui_in2nd;
	Dee_Incref(iter);
	SetUnionIterator_LockEndRead(other);
	self->sui_iter = DeeObject_DeepCopy(iter);
	Dee_Decref(iter);
	if unlikely(!self->sui_iter)
		goto err;
	Dee_atomic_rwlock_init(&self->sui_lock);
	self->sui_union = (DREF SetUnion *)DeeObject_DeepCopy((DeeObject *)other->sui_union);
	if unlikely(!self->sui_union)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->sui_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
suiter_ctor(SetUnionIterator *__restrict self) {
	self->sui_union = (DREF SetUnion *)DeeObject_NewDefault(self->ob_type == &SetUnionIterator_Type
	                                                        ? &SetUnion_Type
	                                                        : &SetSymmetricDifference_Type);
	if unlikely(!self->sui_union)
		goto err;
	self->sui_iter = DeeSet_OperatorIter(self->sui_union->su_a);
	if unlikely(!self->sui_iter)
		goto err_union;
	Dee_atomic_rwlock_init(&self->sui_lock);
	self->sui_in2nd = false;
	return 0;
err_union:
	Dee_Decref(self->sui_union);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
suiter_init(SetUnionIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SetUnionIterator", &self->sui_union))
		goto err;
	if (DeeObject_AssertTypeExact(self->sui_union, &SetUnion_Type))
		goto err;
	if ((self->sui_iter = DeeSet_OperatorIter(self->sui_union->su_a)) == NULL)
		goto err;
	Dee_Incref(self->sui_union);
	Dee_atomic_rwlock_init(&self->sui_lock);
	self->sui_in2nd = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
suiter_next(SetUnionIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *iter;
	bool is_second;
again:
	SetUnionIterator_LockRead(self);
	iter      = self->sui_iter;
	is_second = self->sui_in2nd;
	Dee_Incref(iter);
	SetUnionIterator_LockEndRead(self);
read_from_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (is_second)
		goto done;
	if (result != ITER_DONE) {
		if likely(result) {
			int temp;

			/* Check if the found item is also part of the second set.
			 * If it is, don't yield it now, but yield it later, as
			 * part of the enumeration of the second set. */
			temp = DeeSet_OperatorContainsAsBool(self->sui_union->su_b, result);
			if (temp != 0) {
				/* Error, or apart of second set. */
				Dee_Decref(result);
				if unlikely(temp < 0) {
					result = NULL;
					goto done;
				}
				goto again;
			}
		}
		goto done;
	}

	/* Try to switch to the next iterator. */
	if (self->sui_iter != iter)
		goto again;
	if (self->sui_in2nd)
		goto done;

#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->sui_iter != iter)
		goto again; /* Test again to prevent race conditions. */
#endif /* !CONFIG_NO_THREADS */

	/* Create the level #2 iterator. */
	result = DeeSet_OperatorIter(self->sui_union->su_b);
	if unlikely(!result)
		goto done;
	SetUnionIterator_LockWrite(self);

	/* Check for another race condition. */
	if unlikely(self->sui_iter != iter) {
		SetUnionIterator_LockEndWrite(self);
		Dee_Decref(result);
		goto again;
	}
	ASSERT(!self->sui_in2nd);
	self->sui_iter  = result; /* Inherit reference (x2) */
	self->sui_in2nd = true;
	Dee_Incref(result); /* Reference stored in `sui_iter' */
	SetUnionIterator_LockEndWrite(self);
	Dee_Decref(iter); /* Reference inherited from `sui_iter' */
	iter      = result;
	is_second = true;
	if (DeeThread_CheckInterrupt()) {
		Dee_Decref(iter);
		return NULL;
	}
	goto read_from_iter;
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
suiter_hash(SetUnionIterator *self) {
	Dee_hash_t result;
	bool my_2nd;
	DREF DeeObject *my_iter;
	SetUnionIterator_LockRead(self);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	SetUnionIterator_LockEndRead(self);
	result = Dee_HashCombine(my_2nd ? 1 : 0, DeeObject_Hash(my_iter));
	Dee_Decref(my_iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_compare(SetUnionIterator *self,
               SetUnionIterator *other) {
	int result;
	bool my_2nd, ot_2nd;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	SetUnionIterator_LockRead(self);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	SetUnionIterator_LockEndRead(self);
	SetUnionIterator_LockRead(other);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	SetUnionIterator_LockEndRead(other);
	if (my_2nd != ot_2nd) {
		result = Dee_CompareNe(my_2nd, ot_2nd);
	} else {
		result = DeeObject_Compare(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_compare_eq(SetUnionIterator *self,
                  SetUnionIterator *other) {
	int result;
	bool my_2nd, ot_2nd;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	SetUnionIterator_LockRead(self);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	SetUnionIterator_LockEndRead(self);
	SetUnionIterator_LockRead(other);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	SetUnionIterator_LockEndRead(other);
	if (my_2nd != ot_2nd) {
		result = 1;
	} else {
		result = DeeObject_CompareEq(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp suiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&suiter_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&suiter_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&suiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_member tpconst suiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SetUnionIterator, sui_union), "->?Ert:SetUnion"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst suiter_getsets[] = {
	TYPE_GETSET_F("__iter__", &suiter_get_iter, NULL, &suiter_set_iter, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETSET_F("__in2nd__", &suiter_get_in2nd, NULL, &suiter_set_in2nd, METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject SetUnionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetUnionIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(su?:?Ert:SetUnion)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&suiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&suiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&suiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&suiter_init,
				TYPE_FIXED_ALLOCATOR_GC(SetUnionIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&suiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL /* TODO */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&suiter_visit,
	/* .tp_gc            = */ &suiter_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &suiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&suiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ suiter_getsets,
	/* .tp_members       = */ suiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
su_ctor(SetUnion *__restrict self) {
	self->su_a = Dee_EmptySet;
	self->su_b = Dee_EmptySet;
	Dee_Incref(Dee_EmptySet);
	Dee_Incref(Dee_EmptySet);
	return 0;
}

STATIC_ASSERT(offsetof(SetUnion, su_a) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetUnion, su_a) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SetUnion, su_b) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetUnion, su_b) == offsetof(ProxyObject2, po_obj2));
#define su_init generic_proxy2__init
#define su_copy generic_proxy2__copy_alias12
#define su_deep generic_proxy2__deepcopy

PRIVATE WUNUSED NONNULL((1)) DREF SetUnionIterator *DCALL
su_iter(SetUnion *__restrict self) {
	DREF SetUnionIterator *result;
	result = DeeGCObject_MALLOC(SetUnionIterator);
	if unlikely(!result)
		goto err;
	result->sui_iter = DeeSet_OperatorIter(self->su_a);
	if unlikely(!result->sui_iter)
		goto err_r;
	Dee_atomic_rwlock_init(&result->sui_lock);
	result->sui_in2nd = false;
	result->sui_union = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SetUnionIterator_Type);
	return (DREF SetUnionIterator *)DeeGC_Track((DREF DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
su_contains(SetUnion *self, DeeObject *item) {
	DREF DeeObject *result;
	int temp;
	result = DeeSet_OperatorContains(self->su_a, item);
	if unlikely(!result)
		goto done;
	temp = DeeObject_Bool(result);
	if unlikely(temp < 0)
		goto err_r;
	if (temp)
		goto done;
	Dee_Decref_unlikely(result);

	/* Check the second set, and forward the return value. */
	result = DeeSet_OperatorContains(self->su_b, item);
done:
	return result;
err_r:
	Dee_Clear(result);
	goto done;
}

struct su_foreach_if_contained_in_data {
	DeeObject    *feicid_seq;  /* [1..1] Sequence that mustn't contain elements. */
	Dee_foreach_t feicid_proc; /* [1..1] Wrapper callback. */
	void         *feicid_arg;  /* [?..?] Cookie for `feicid_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL 
su_foreach_if_not_contained_in_cb(void *arg, DeeObject *elem) {
	struct su_foreach_if_contained_in_data *data;
	int contains;
	data = (struct su_foreach_if_contained_in_data *)arg;
	contains = DeeSet_OperatorContainsAsBool(data->feicid_seq, elem);
	if unlikely(contains < 0)
		goto err;
	if (contains)
		return 0; /* Don't enumerate if contained in caller-set object. */
	return (*data->feicid_proc)(data->feicid_arg, elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL 
su_foreach_if_contained_in_cb(void *arg, DeeObject *elem) {
	struct su_foreach_if_contained_in_data *data;
	int contains;
	data = (struct su_foreach_if_contained_in_data *)arg;
	contains = DeeSet_OperatorContainsAsBool(data->feicid_seq, elem);
	if unlikely(contains < 0)
		goto err;
	if (!contains)
		return 0; /* Don't enumerate if not contained in caller-set object. */
	return (*data->feicid_proc)(data->feicid_arg, elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL 
su_foreach(SetUnion *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t result;
	result = DeeSet_OperatorForeach(self->su_a, proc, arg);
	if likely(result >= 0) {
		Dee_ssize_t temp;
		struct su_foreach_if_contained_in_data data;
		data.feicid_seq  = self->su_a;
		data.feicid_proc = proc;
		data.feicid_arg  = arg;
		temp = DeeSet_OperatorForeach(self->su_b, &su_foreach_if_not_contained_in_cb, &data);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL 
su_bool(SetUnion *__restrict self) {
	int result = DeeSet_OperatorBool(self->su_a);
	if likely(result == 0)
		result = DeeSet_OperatorBool(self->su_b);
	return result;
}

PRIVATE struct type_seq su_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&su_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&su_contains,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_foreach  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&su_foreach,
	/* .tp_foreach_pair    = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_enumerate       = */ NULL,
	/* .tp_enumerate_index = */ NULL,
	/* .tp_iterkeys        = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
};

PRIVATE struct type_member tpconst su_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SetUnionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetUnion_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetUnion",
	/* .tp_doc      = */ DOC("()\n"
	                         "(a:?DSet,b:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&su_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&su_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&su_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&su_init,
				TYPE_FIXED_ALLOCATOR(SetUnion)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&su_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&su_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&su_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__AFC6A8FA89E9F0A6), /* TODO */
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7188129899C2A8D6),
	/* .tp_seq           = */ &su_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL, /* TODO */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ su_class_members,
};




/* ================================================================================ */
/*   SET SYMMETRIC DIFFERENCE                                                       */
/* ================================================================================ */

STATIC_ASSERT(sizeof(SetUnionIterator) == sizeof(SetSymmetricDifferenceIterator));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(SetSymmetricDifferenceIterator, ssd_set));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_iter) == offsetof(SetSymmetricDifferenceIterator, ssd_iter));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(SetIntersectionIterator, sii_intersect));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(SetUnionIterator, sui_lock) == offsetof(SetSymmetricDifferenceIterator, ssd_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(SetUnionIterator, sui_in2nd) == offsetof(SetSymmetricDifferenceIterator, ssd_in2nd));
#define ssditer_ctor    suiter_ctor
#define ssditer_copy    suiter_copy
#define ssditer_deep    suiter_deep
#define ssditer_init    suiter_init
#define ssditer_fini    suiter_fini
#define ssditer_visit   suiter_visit
#define ssditer_gc      suiter_gc
#define ssditer_cmp     suiter_cmp
#define ssditer_getsets suiter_getsets
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ssditer_next(SetSymmetricDifferenceIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *iter;
	bool is_second;
again:
	SetSymmetricDifferenceIterator_LockRead(self);
	iter      = self->ssd_iter;
	is_second = self->ssd_in2nd;
	Dee_Incref(iter);
	SetSymmetricDifferenceIterator_LockEndRead(self);
read_from_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (is_second)
		goto done;
	if unlikely(result != ITER_DONE) {
		if (result) {
			int temp;
			/* Only yield the item if it's not contained in the other set. */
			temp = DeeSet_OperatorContainsAsBool(is_second
			                                     ? self->ssd_set->ssd_b
			                                     : self->ssd_set->ssd_a,
			                                     result);
			if (temp != 0) {
				/* Error, or apart of second set. */
				Dee_Decref(result);
				if unlikely(temp < 0) {
					result = NULL;
					goto done;
				}
				goto again;
			}
		}
		goto done;
	}

	/* Try to switch to the next iterator. */
	if (self->ssd_iter != iter)
		goto again;
	if (self->ssd_in2nd)
		goto done;

#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->ssd_iter != iter)
		goto again; /* Test again to prevent race conditions. */
#endif /* !CONFIG_NO_THREADS */

	/* Create the level #2 iterator. */
	result = DeeSet_OperatorIter(self->ssd_set->ssd_b);
	if unlikely(!result)
		goto done;
	SetSymmetricDifferenceIterator_LockWrite(self);

	/* Check for another race condition. */
	if unlikely(self->ssd_iter != iter) {
		SetSymmetricDifferenceIterator_LockEndWrite(self);
		Dee_Decref(result);
		goto again;
	}
	ASSERT(!self->ssd_in2nd);
	self->ssd_iter  = result; /* Inherit reference (x2) */
	self->ssd_in2nd = true;
	Dee_Incref(result); /* Reference stored in `ssd_iter' */
	SetSymmetricDifferenceIterator_LockEndWrite(self);
	Dee_Decref(iter); /* Reference inherited from `ssd_iter' */
	iter      = result;
	is_second = true;
	if (DeeThread_CheckInterrupt()) {
		Dee_Decref(iter);
		return NULL;
	}
	goto read_from_iter;
done:
	return result;
}

PRIVATE struct type_member tpconst ssditer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT,
	                      offsetof(SetSymmetricDifferenceIterator, ssd_set),
	                      "->?Ert:SetSymmetricDifference"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetSymmetricDifferenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetSymmetricDifferenceIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(ssd?:?Ert:SetSymmetricDifference)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ssditer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&ssditer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ssditer_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&ssditer_init,
				TYPE_FIXED_ALLOCATOR_GC(SetSymmetricDifferenceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ssditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL /* TODO */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ssditer_visit,
	/* .tp_gc            = */ &ssditer_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &ssditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssditer_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ssditer_getsets,
	/* .tp_members       = */ ssditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

STATIC_ASSERT(offsetof(SetSymmetricDifference, ssd_a) == offsetof(SetUnion, su_a) ||
              offsetof(SetSymmetricDifference, ssd_a) == offsetof(SetUnion, su_a));
STATIC_ASSERT(offsetof(SetSymmetricDifference, ssd_b) == offsetof(SetUnion, su_b) ||
              offsetof(SetSymmetricDifference, ssd_b) == offsetof(SetUnion, su_b));
#define ssd_ctor su_ctor

STATIC_ASSERT(offsetof(SetSymmetricDifference, ssd_a) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetSymmetricDifference, ssd_a) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SetSymmetricDifference, ssd_b) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetSymmetricDifference, ssd_b) == offsetof(ProxyObject2, po_obj2));
#define ssd_init  generic_proxy2__init
#define ssd_copy  generic_proxy2__copy_alias12
#define ssd_deep  generic_proxy2__deepcopy
#define ssd_fini  generic_proxy2__fini
#define ssd_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) DREF SetSymmetricDifferenceIterator *DCALL
ssd_iter(SetSymmetricDifference *__restrict self) {
	DREF SetSymmetricDifferenceIterator *result;
	result = DeeGCObject_MALLOC(SetSymmetricDifferenceIterator);
	if unlikely(!result)
		goto err;
	result->ssd_iter = DeeSet_OperatorIter(self->ssd_a);
	if unlikely(!result->ssd_iter)
		goto err_r;
	Dee_atomic_rwlock_init(&result->ssd_lock);
	result->ssd_in2nd = false;
	result->ssd_set   = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SetSymmetricDifferenceIterator_Type);
	return (DREF SetSymmetricDifferenceIterator *)DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ssd_contains(SetSymmetricDifference *self, DeeObject *item) {
	DREF DeeObject *result;
	int cona, conb;
	cona = DeeSet_OperatorContainsAsBool(self->ssd_a, item);
	if unlikely(cona < 0)
		goto err;
	conb = DeeSet_OperatorContainsAsBool(self->ssd_b, item);
	if unlikely(conb < 0)
		goto err;
	result = DeeBool_For(!!cona ^ !!conb);
	Dee_Incref(result);
done:
	return result;
err:
	result = NULL;
	goto done;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL 
ssd_foreach(SetSymmetricDifference *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t r1, r2;
	struct su_foreach_if_contained_in_data data;
	data.feicid_seq  = self->ssd_b;
	data.feicid_proc = proc;
	data.feicid_arg  = arg;
	r1 = DeeSet_OperatorForeach(self->ssd_a, &su_foreach_if_not_contained_in_cb, &data);
	if unlikely(r1 < 0)
		return r1;
	data.feicid_seq = self->ssd_a;
	r2 = DeeSet_OperatorForeach(self->ssd_b, &su_foreach_if_not_contained_in_cb, &data);
	if unlikely(r2 < 0)
		return r2;
	return r1 + r2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ssd_bool(SetSymmetricDifference *__restrict self) {
	/* `(a ^ b) != {}'    <=>    `a != b' */
	int result = DeeSet_OperatorCompareEq(self->ssd_a, self->ssd_b);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 0 : 1;
err:
	return -1;
}

PRIVATE struct type_seq ssd_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssd_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssd_contains,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_foreach  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ssd_foreach,
	/* .tp_foreach_pair    = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_enumerate       = */ NULL,
	/* .tp_enumerate_index = */ NULL,
	/* .tp_iterkeys        = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
};

PRIVATE struct type_member tpconst ssd_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SetSymmetricDifferenceIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetSymmetricDifference_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetSymmetricDifference",
	/* .tp_doc      = */ DOC("()\n"
	                         "(a:?DSet,b:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ssd_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&ssd_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ssd_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&ssd_init,
				TYPE_FIXED_ALLOCATOR(SetSymmetricDifference)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ssd_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&ssd_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ssd_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__AFC6A8FA89E9F0A6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7188129899C2A8D6), /* TODO */
	/* .tp_seq           = */ &ssd_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL, /* TODO */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ssd_class_members,
};






/* ================================================================================ */
/*   SET INTERSECTION                                                               */
/* ================================================================================ */
STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_intersect) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetIntersectionIterator, sii_intersect) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetIntersectionIterator, sii_iter) == offsetof(ProxyObject2, po_obj2));
#define siiter_fini  generic_proxy2__fini
#define siiter_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
siiter_ctor(SetIntersectionIterator *__restrict self) {
	self->sii_intersect = (DREF SetIntersection *)DeeObject_NewDefault(self->ob_type == &SetDifferenceIterator_Type
	                                                                   ? &SetIntersection_Type
	                                                                   : &SetDifference_Type);
	if unlikely(!self->sii_intersect)
		goto err;
	self->sii_iter = DeeSet_OperatorIter(self->sii_intersect->si_a);
	if unlikely(!self->sii_iter)
		goto err_isec;
	self->sii_other = self->sii_intersect->si_b;
	return 0;
err_isec:
	Dee_Decref(self->sii_intersect);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
siiter_copy(SetIntersectionIterator *__restrict self,
            SetIntersectionIterator *__restrict other) {
	self->sii_iter = DeeObject_Copy(other->sii_iter);
	if unlikely(!self->sii_iter)
		goto err;
	self->sii_intersect = other->sii_intersect;
	self->sii_other     = other->sii_other;
	Dee_Incref(self->sii_intersect);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
siiter_deep(SetIntersectionIterator *__restrict self,
            SetIntersectionIterator *__restrict other) {
	self->sii_iter = DeeObject_DeepCopy(other->sii_iter);
	if unlikely(!self->sii_iter)
		goto err;
	self->sii_intersect = (DREF SetIntersection *)DeeObject_DeepCopy((DeeObject *)other->sii_intersect);
	if unlikely(!self->sii_intersect)
		goto err_iter;
	self->sii_other = self->sii_intersect->si_b;
	return 0;
err_iter:
	Dee_Decref(self->sii_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
siiter_init(SetIntersectionIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SetIntersectionIterator", &self->sii_intersect))
		goto err;
	if (DeeObject_AssertTypeExact(self->sii_intersect, &SetIntersection_Type))
		goto err;
	self->sii_iter = DeeSet_OperatorIter(self->sii_intersect->si_a);
	if unlikely(!self->sii_iter)
		goto err;
	Dee_Incref(self->sii_intersect);
	self->sii_other = self->sii_intersect->si_b;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
siiter_next(SetIntersectionIterator *__restrict self) {
	DREF DeeObject *result;
	int temp;
again:
	result = DeeObject_IterNext(self->sii_iter);
	if (!ITER_ISOK(result))
		goto done;

	/* Check if contained in the second set. */
	temp = DeeSet_OperatorContainsAsBool(self->sii_other, result);
	if (temp <= 0) {
		Dee_Decref(result);
		if (!temp) {
			if (!DeeThread_CheckInterrupt())
				goto again; /* If the object isn't contained within, read the next one. */
		}
		result = NULL; /* Error... */
	}
done:
	return result;
}

STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_iter) == offsetof(ProxyObject, po_obj));
#define siiter_hash          generic_proxy__hash_recursive
#define siiter_compare       generic_proxy__compare_recursive
#define siiter_compare_eq    generic_proxy__compare_eq_recursive
#define siiter_trycompare_eq generic_proxy__trycompare_eq_recursive
#define siiter_cmp           generic_proxy__cmp_recursive

PRIVATE struct type_member tpconst siiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SetIntersectionIterator, sii_intersect), "->?Ert:SetIntersection"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SetIntersectionIterator, sii_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__other__", STRUCT_OBJECT, offsetof(SetIntersectionIterator, sii_other), "->?DSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetIntersectionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetIntersectionIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(si?:?Ert:SetIntersection)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&siiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&siiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&siiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&siiter_init,
				TYPE_FIXED_ALLOCATOR(SetIntersectionIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&siiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL /* TODO */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&siiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &siiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&siiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ siiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

STATIC_ASSERT(offsetof(SetIntersection, si_a) == offsetof(SetUnion, su_a) ||
              offsetof(SetIntersection, si_a) == offsetof(SetUnion, su_a));
STATIC_ASSERT(offsetof(SetIntersection, si_b) == offsetof(SetUnion, su_b) ||
              offsetof(SetIntersection, si_b) == offsetof(SetUnion, su_b));
#define si_ctor su_ctor

STATIC_ASSERT(offsetof(SetIntersection, si_a) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetIntersection, si_a) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SetIntersection, si_b) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SetIntersection, si_b) == offsetof(ProxyObject2, po_obj2));
#define si_init  generic_proxy2__init
#define si_copy  generic_proxy2__copy_alias12
#define si_deep  generic_proxy2__deepcopy
#define si_fini  generic_proxy2__fini
#define si_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) DREF SetIntersectionIterator *DCALL
si_iter(SetIntersection *__restrict self) {
	DREF SetIntersectionIterator *result;
	result = DeeObject_MALLOC(SetIntersectionIterator);
	if unlikely(!result)
		goto done;
	result->sii_iter = DeeSet_OperatorIter(self->si_a);
	if unlikely(!result->sii_iter)
		goto err_r;
	Dee_Incref(self);
	result->sii_intersect = self;
	result->sii_other     = self->si_b;
	DeeObject_Init(result, &SetIntersectionIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
si_contains(SetIntersection *self, DeeObject *item) {
	DREF DeeObject *result;
	int temp;
	result = DeeSet_OperatorContains(self->si_a, item);
	if unlikely(!result)
		goto done;
	temp = DeeObject_Bool(result);
	if unlikely(temp < 0)
		goto err_r;
	if (!temp)
		goto done;
	Dee_Decref(result);

	/* Check the second set, and forward the return value. */
	result = DeeSet_OperatorContains(self->si_b, item);
done:
	return result;
err_r:
	Dee_Clear(result);
	goto done;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
si_foreach(SetIntersection *__restrict self, Dee_foreach_t proc, void *arg) {
	struct su_foreach_if_contained_in_data data;
	data.feicid_seq  = self->si_b;
	data.feicid_proc = proc;
	data.feicid_arg  = arg;
	return DeeSet_OperatorForeach(self->si_a, &su_foreach_if_contained_in_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
si_bool(SetIntersection *__restrict self) {
	return SetIntersection_NonEmpty(self->si_a, self->si_b);
}

PRIVATE struct type_seq si_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&si_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&si_contains,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_foreach  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&si_foreach,
	/* .tp_foreach_pair    = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_enumerate       = */ NULL,
	/* .tp_enumerate_index = */ NULL,
	/* .tp_iterkeys        = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
};

PRIVATE struct type_member tpconst si_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SetIntersectionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetIntersection_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetIntersection",
	/* .tp_doc      = */ DOC("()\n"
	                         "(a:?DSet,b:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&si_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&si_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&si_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&si_init,
				TYPE_FIXED_ALLOCATOR(SetIntersection)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&si_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&si_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&si_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__AFC6A8FA89E9F0A6), /* TODO */
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7188129899C2A8D6),
	/* .tp_seq           = */ &si_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL, /* TODO */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ si_class_members,
};





/* ================================================================================ */
/*   SET DIFFERENCE                                                                 */
/* ================================================================================ */
STATIC_ASSERT(sizeof(SetIntersectionIterator) == sizeof(SetDifferenceIterator));
STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_iter) == offsetof(SetDifferenceIterator, sdi_iter));
STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_other) == offsetof(SetDifferenceIterator, sdi_other));
#define sditer_ctor    siiter_ctor
#define sditer_copy    siiter_copy
#define sditer_deep    siiter_deep
#define sditer_init    siiter_init
#define sditer_fini    siiter_fini
#define sditer_visit   siiter_visit
#define sditer_cmp     siiter_cmp
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sditer_next(SetDifferenceIterator *__restrict self) {
	DREF DeeObject *result;
	int temp;
again:
	result = DeeObject_IterNext(self->sdi_iter);
	if (!ITER_ISOK(result))
		goto done;
	/* Check if contained in the second set. */
	temp = DeeSet_OperatorContainsAsBool(self->sdi_other, result);
	if (temp != 0) {
		Dee_Decref(result);
		if (temp) {
			if (!DeeThread_CheckInterrupt())
				goto again; /* If the object is contained within, read the next one. */
		}
		result = NULL; /* Error... */
	}
done:
	return result;
}

PRIVATE struct type_member tpconst sditer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SetDifferenceIterator, sdi_diff), "->?Ert:SetDifference"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SetDifferenceIterator, sdi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__other__", STRUCT_OBJECT, offsetof(SetDifferenceIterator, sdi_other), "->?DSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetDifferenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetDifferenceIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(sdi?:?Ert:SetDifference)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sditer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sditer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sditer_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&sditer_init,
				TYPE_FIXED_ALLOCATOR(SetDifferenceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL /* TODO */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &sditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sditer_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

STATIC_ASSERT(offsetof(SetDifference, sd_a) == offsetof(SetUnion, su_a));
STATIC_ASSERT(offsetof(SetDifference, sd_b) == offsetof(SetUnion, su_b));
#define sd_ctor su_ctor

STATIC_ASSERT(offsetof(SetDifference, sd_a) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(SetDifference, sd_b) == offsetof(ProxyObject2, po_obj2));
#define sd_init  generic_proxy2__init
#define sd_copy  generic_proxy2__copy_alias12
#define sd_deep  generic_proxy2__deepcopy
#define sd_fini  generic_proxy2__fini
#define sd_visit generic_proxy2__visit


PRIVATE WUNUSED NONNULL((1)) DREF SetDifferenceIterator *DCALL
sd_iter(SetDifference *__restrict self) {
	DREF SetDifferenceIterator *result;
	result = DeeObject_MALLOC(SetDifferenceIterator);
	if unlikely(!result)
		goto done;
	result->sdi_iter = DeeSet_OperatorIter(self->sd_a);
	if unlikely(!result->sdi_iter)
		goto err_r;
	result->sdi_diff  = self;
	result->sdi_other = self->sd_b;
	Dee_Incref(self);
	DeeObject_Init(result, &SetDifferenceIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sd_contains(SetDifference *self, DeeObject *item) {
	int temp;

	/* Check the primary set for the object. */
	temp = DeeSet_OperatorContainsAsBool(self->sd_a, item);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return_false;
	}

	/* The object is apart of the primary set.
	 * -> Return true if it's not apart of the secondary set.
	 * -> Return false otherwise. */
	temp = DeeSet_OperatorContainsAsBool(self->sd_b, item);
	if unlikely(temp < 0)
		goto err;
	return_bool_(!temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL 
sd_foreach(SetDifference *__restrict self, Dee_foreach_t proc, void *arg) {
	struct su_foreach_if_contained_in_data data;
	data.feicid_seq  = self->sd_b;
	data.feicid_proc = proc;
	data.feicid_arg  = arg;
	return DeeSet_OperatorForeach(self->sd_a, &su_foreach_if_not_contained_in_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sd_bool(SetDifference *__restrict self) {
	return SetDifference_NonEmpty(self->sd_a, self->sd_b);
}

PRIVATE struct type_seq sd_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sd_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sd_contains,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_foreach  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&sd_foreach,
	/* .tp_foreach_pair    = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_enumerate       = */ NULL,
	/* .tp_enumerate_index = */ NULL,
	/* .tp_iterkeys        = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
};

PRIVATE struct type_member tpconst sd_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SetDifferenceIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetDifference_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetDifference",
	/* .tp_doc      = */ DOC("()\n"
	                         "(a:?DSet,b:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sd_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sd_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sd_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&sd_init,
				TYPE_FIXED_ALLOCATOR(SetDifference)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sd_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sd_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sd_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__AFC6A8FA89E9F0A6), /* TODO */
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7188129899C2A8D6),
	/* .tp_seq           = */ &sd_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL, /* TODO */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sd_class_members,
};




#define SET_CONTAINSANY_FOREACH_LHS__FOUND SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_containsany_foreach_lhs_cb(void *arg, DeeObject *key) {
	int is_contained = DeeSet_OperatorContainsAsBool((DeeObject *)arg, key);
	if (is_contained != 0) {
		if unlikely(is_contained < 0)
			goto err;
		return SET_CONTAINSANY_FOREACH_LHS__FOUND;
	}
	return 0;
err:
	return -1;
}

/* >> for (local key: lhs) if (key in rhs) return true;
 * >> return false; */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_containsany_foreach_lhs(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t status;
	status = DeeSet_OperatorForeach(lhs, &set_containsany_foreach_lhs_cb, rhs);
	if (status == SET_CONTAINSANY_FOREACH_LHS__FOUND)
		return 1;
	if (status == 0)
		return 0;
	return -1;
}


#define SET_CONTAINSALL_FOREACH_LHS__MISSING SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_containsall_foreach_lhs_cb(void *arg, DeeObject *key) {
	int is_contained = DeeSet_OperatorContainsAsBool((DeeObject *)arg, key);
	if (is_contained <= 0) {
		if unlikely(is_contained < 0)
			goto err;
		return SET_CONTAINSALL_FOREACH_LHS__MISSING;
	}
	return 0;
err:
	return -1;
}

/* >> for (local key: lhs) if (key !in rhs) return false;
 * >> return true; */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_containsall_foreach_lhs(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t status;
	status = DeeSet_OperatorForeach(lhs, &set_containsall_foreach_lhs_cb, rhs);
	if (status == SET_CONTAINSALL_FOREACH_LHS__MISSING)
		return 0;
	if (status == 0)
		return 1;
	return -1;
}


/*
INTERN WUNUSED NONNULL((1, 2)) int DCALL
SetUnion_NonEmpty(DeeObject *a, DeeObject *b) {
	int result = DeeSet_OperatorBool(a);
	if (result == 0)
		result = DeeSet_OperatorBool(b);
	return result;
}
*/

INTERN WUNUSED NONNULL((1, 2)) int DCALL
SetIntersection_NonEmpty(DeeObject *a, DeeObject *b) {
	if (SetInversion_CheckExact(a)) {
		SetInversion *xa = (SetInversion *)a;
		/* `(~a & b) != {}'   <=>   `(b - a) != {}' */
		return SetDifference_NonEmpty(b, xa->si_set);
	} else if (SetInversion_CheckExact(b)) {
		SetInversion *xb = (SetInversion *)b;
		/* `(a & ~b) != {}'   <=>   `(a - b) != {}' */
		return SetDifference_NonEmpty(a, xb->si_set);
	} else {
		size_t size_a, size_b;
		size_a = DeeSet_OperatorSize(a);
		if unlikely(size_a == (size_t)-1)
			goto err;
		size_b = DeeSet_OperatorSize(b);
		if unlikely(size_b == (size_t)-1)
			goto err;
		if (size_a < size_b) {
			/* >> for (local key: a) if (key in b) return true; */
			return set_containsany_foreach_lhs(a, b);
		} else {
			/* >> for (local key: b) if (key in a) return true; */
			return set_containsany_foreach_lhs(b, a);
		}
		__builtin_unreachable();
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
SetDifference_NonEmpty(DeeObject *a, DeeObject *b) {
	if (SetInversion_CheckExact(a)) {
		/* `(~a - b) != {}'   <=>   `true'
		 * Reason: There is always an imaginary object
		 *         in "a" that does not exist in "b" */
		return 1;
	} else if (SetInversion_CheckExact(b)) {
		/* `(a - ~b) != {}'   <=>   `(a & b) != {}' */
		SetInversion *xb = (SetInversion *)b;
		return SetIntersection_NonEmpty(a, xb->si_set);
	} else {
		/* >> for (local key: a) if (key !in b) return true;
		 * >> return false; */
		int result = set_containsall_foreach_lhs(a, b);
		if likely(result >= 0)
			result = !result;
		return result;
	}
	__builtin_unreachable();
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_C */
