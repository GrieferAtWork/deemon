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
#ifndef GUARD_DEEMON_OBJECTS_SET_C
#define GUARD_DEEMON_OBJECTS_SET_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/default-api.h"

DECL_BEGIN


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
set_hash(DeeObject *__restrict self) {
	/* TODO: DeeSet_DefaultHashWithForeachDefault() */
	dhash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	DREF DeeObject *iter, *elem;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		/* Note how we don't use `Dee_HashCombine()' here!
		 * That become order doesn't matter for sets. */
		result ^= DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err:
	DeeError_Print("Unhandled error in `Set.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
invset_hash(DeeObject *__restrict self) {
	return ~set_hash(self); /* Return the inverse hash (we're an inverse set after all ;) ) */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_eq(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_ne(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_lo(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_IsTrueSubSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_le(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_IsSubSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_gr(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_IsTrueSubSet(some_object, self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_ge(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_IsSubSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_Invert(DeeObject *__restrict self) {
	DREF DeeSetInversionObject *result;
	/* Just re-return the original set. */
	if (DeeSetInversion_CheckExact(self))
		return_reference(DeeSetInversion_GetSet(self));

	/* Construct a new inverse-set wrapper. */
	result = DeeObject_MALLOC(DeeSetInversionObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeSetInversion_Type);
	result->si_set = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
invset_ctor(DeeSetInversionObject *__restrict self) {
	self->si_set = Dee_EmptySet;
	Dee_Incref(Dee_EmptySet);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
invset_init(DeeSetInversionObject *__restrict self,
            size_t argc, DeeObject *const *argv) {
	self->si_set = Dee_EmptySet;
	if (DeeArg_Unpack(argc, argv, "|o:_InverseSet", &self->si_set))
		goto err;
	Dee_Incref(self->si_set);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
invset_fini(DeeSetInversionObject *__restrict self) {
	Dee_Decref(self->si_set);
}

PRIVATE NONNULL((1, 2)) void DCALL
invset_visit(DeeSetInversionObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_set);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
invset_printrepr(DeeSetInversionObject *__restrict self,
                 dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "~%r", self->si_set);
}

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
invset_iterself(DeeSetInversionObject *__restrict self) {
	/* Sorry, but it's impossible to enumerate a set containing (almost) everything */
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	return NULL;
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_tpcontains(DeeSetInversionObject *self, DeeObject *key) {
	int result = DeeObject_ContainsAsBool(self->si_set, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE struct type_seq invset_seq = {
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	/* .tp_iter     = */ NULL,
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&invset_iterself,
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_tpcontains,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
invset_Iterator_get(DeeTypeObject *__restrict self) {
	err_unknown_attribute_string(self, STR_Iterator, ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE struct type_getset tpconst invset_class_getsets[] = {
	TYPE_GETTER_NODOC(STR_Iterator, &invset_Iterator_get),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst invset_members[] = {
	TYPE_MEMBER_FIELD_DOC("__blacklist__", STRUCT_OBJECT, offsetof(DeeSetInversionObject, si_set), "->?DSet"),
	TYPE_MEMBER_END
};

PRIVATE struct type_cmp invset_cmp = {
	/* .tp_hash          = */ (dhash_t (DCALL *)(DeeObject *__restrict))&invset_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_ge,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
invset_getset(DeeSetInversionObject *__restrict self) {
	return_reference_(self->si_set);
}

PRIVATE struct type_math invset_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&invset_getset,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &DeeSet_Union,
	/* .tp_sub         = */ &DeeSet_Difference,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ &DeeSet_Intersection,
	/* .tp_or          = */ &DeeSet_Union,
	/* .tp_xor         = */ &DeeSet_SymmetricDifference,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL,
};

PUBLIC DeeTypeObject DeeSetInversion_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_InverseSet",
	/* .tp_doc      = */ DOC("()\n"
	                         "(set:?DSet)"),
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
				TYPE_FIXED_ALLOCATOR(DeeSetInversionObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&invset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&invset_printrepr
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
	/* .tp_class_getsets = */ invset_class_getsets,
	/* .tp_class_members = */ NULL
};




/* Returns the number of items found in `lhs' if all of them appear in `rhs'.
 * Otherwise, return `-1' if some of them don't appear in `rhs'
 * Otherwise, return `-2' on error. */
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
set_issubset_impl(DeeObject *lhs, DeeObject *rhs) {
	dssize_t result = 0;
	int temp;
	DREF DeeObject *lhs_iter, *lhs_item;
	lhs_iter = DeeObject_Iter(lhs);
	if unlikely(!lhs_iter)
		goto err;
	while (ITER_ISOK(lhs_item = DeeObject_IterNext(lhs_iter))) {
		/* Check if this item appears in `rhs' */
		temp = DeeObject_ContainsAsBool(rhs, lhs_item);
		Dee_Decref(lhs_item);
		if unlikely(temp < 0)
			goto err_iter;
		if unlikely(!temp) {
			result = -1;
			break;
		}
		++result; /* Count the number of found items. */
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(lhs_iter);
	if unlikely(!lhs_item)
		result = -1;
	return result;
err_iter:
	Dee_Decref(lhs_iter);
err:
	return -2;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_IsSubSet(DeeObject *lhs, DeeObject *rhs) {
	if (DeeSetInversion_CheckExact(lhs)) {
		/* An inverse set can only ever be the sub-set of another inverse set. */
		if (!DeeSetInversion_CheckExact(rhs))
			return 0;
		/* ~lhs <= ~rhs   <===>   rhs <= lhs */
		return DeeSet_IsSubSet(DeeSetInversion_GetSet(rhs),
		                       DeeSetInversion_GetSet(lhs));
	} else {
		dssize_t result = set_issubset_impl(lhs, rhs);
		return unlikely(result == -2)
		       ? -1
		       : result < 0
		         ? 0
		         : 1;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_IsTrueSubSet(DeeObject *lhs, DeeObject *rhs) {
	if (DeeSetInversion_CheckExact(lhs)) {
		/* An inverse set can only ever be the sub-set of another inverse set. */
		if (!DeeSetInversion_CheckExact(rhs))
			return 0;
		/* ~lhs < ~rhs   <===>   rhs < lhs */
		return DeeSet_IsTrueSubSet(DeeSetInversion_GetSet(rhs),
		                           DeeSetInversion_GetSet(lhs));
	} else {
		dssize_t result;
		size_t rhs_size;
		result = set_issubset_impl(lhs, rhs);
		if unlikely(result == -2)
			goto err;
		if (result < 0)
			return 0;
		/* Check the size of `rhs' to make sure
		 * it contains more elements than `lhs' */
		if (DeeSetInversion_CheckExact(rhs))
			return 1; /* Inverse sets have an infinite size. */
		rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		return rhs_size > (size_t)result;
	}
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_IsSameSet(DeeObject *lhs, DeeObject *rhs) {
	dssize_t result;
	size_t rhs_size;
	if (DeeSetInversion_CheckExact(lhs)) {
		/* An inverse set can never equal a non-inverse set. */
		if (!DeeSetInversion_CheckExact(rhs))
			return 0;
		lhs = DeeSetInversion_GetSet(lhs);
		rhs = DeeSetInversion_GetSet(rhs);
	}
	if (DeeSetInversion_CheckExact(rhs))
		return 0; /* A regular set can never match an inverse set. */
	result = set_issubset_impl(lhs, rhs);
	if unlikely(result == -2)
		goto err;
	if (result < 0)
		return 0;
	/* Check the size of `rhs' to make sure
	 * it contains the same number of as `lhs' */
	rhs_size = DeeObject_Size(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	return rhs_size == (size_t)result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_IsDisjoint(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *iter, *item;
	int result = 1;
	if (DeeSetInversion_CheckExact(lhs)) {
		/* 2 inverse sets can never be disjoint, because there's
		 * always an imaginary object that is shared by both. */
		if (DeeSetInversion_CheckExact(rhs))
			return 0;
		/* If all elements from `rhs' are black-listed, then
		 * our inverse set is disjoint from it, meaning that
		 * `rhs' is a subset of our black-list. */
		return DeeSet_IsSubSet(rhs, DeeSetInversion_GetSet(lhs));
	}
	/* Verify that no items from `lhs' appear in `rhs' */
	iter = DeeObject_Iter(lhs);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		/* Make sure that `rhs' doesn't contain this item. */
		int temp = DeeObject_ContainsAsBool(rhs, item);
		Dee_Decref(item);
		if unlikely(temp < 0)
			goto err_iter;
		if (temp) {
			result = 0;
			break;
		} /* Rhs does contain this one... */
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iter);
	if unlikely(!item)
		result = -1;
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_difference(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:difference", &other))
		goto err;
	return DeeSet_Difference(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_intersection(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:intersection", &other))
		goto err;
	return DeeSet_Intersection(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_isdisjoint(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:isdisjoint", &other))
		goto err;
	result = DeeSet_IsDisjoint(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_union(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:union", &other))
		goto err;
	return DeeSet_Union(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_symmetric_difference(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:symmetric_difference", &other))
		goto err;
	return DeeSet_SymmetricDifference(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_issubset(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:issubset", &other))
		goto err;
	result = DeeSet_IsSubSet(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_issuperset(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:issuperset", &other))
		goto err;
	result = DeeSet_IsSubSet(other, self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_sub(DeeObject **__restrict p_self,
                DeeObject *items) {
	DeeObject *self = *p_self;
	size_t i, size;
	DREF DeeObject *remove_func, *callback_result, *remove_args[1];
	remove_func = DeeObject_GetAttr(self, (DeeObject *)&str_remove);
	if unlikely(!remove_func)
		goto err;
	size = DeeFastSeq_GetSize_deprecated(items);
	if (size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		for (i = 0; i < size; ++i) {
			remove_args[0] = DeeFastSeq_GetItem_deprecated(items, i);
			if unlikely(!remove_args[0])
				goto err_func;
			callback_result = DeeObject_Call(remove_func, 1, remove_args);
			Dee_Decref(remove_args[0]);
			if unlikely(!callback_result)
				goto err_func;
			Dee_Decref(callback_result);
		}
	} else {
		items = DeeObject_Iter(items);
		if unlikely(!items)
			goto err_func;
		while (ITER_ISOK(remove_args[0] = DeeObject_IterNext(items))) {
			callback_result = DeeObject_Call(remove_func, 1, remove_args);
			Dee_Decref(remove_args[0]);
			if unlikely(!callback_result)
				goto err_iter;
			Dee_Decref(callback_result);
			if (DeeThread_CheckInterrupt())
				goto err_iter;
		}
		if unlikely(!remove_args[0])
			goto err_iter;
		Dee_Decref(items);
	}
	Dee_Decref(remove_func);
	return 0;
err_iter:
	Dee_Decref(items);
err_func:
	Dee_Decref(remove_func);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_mul(DeeObject **__restrict p_self,
                DeeObject *count) {
	size_t count_integer;
	if (DeeObject_AsSize(count, &count_integer))
		goto err;
	if (count_integer == 0) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallAttr(*p_self, (DeeObject *)&str_clear, 0, NULL);
		if unlikely(!callback_result)
			goto err;
		Dee_Decref(callback_result);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_intersection(DeeObject **__restrict p_self,
                         DeeObject *items) {
	DeeObject *self = *p_self;
	int result;
	DREF DeeObject *new_self;
	new_self = DeeObject_And(self, items);
	if unlikely(!new_self)
		goto err;
	result = DeeObject_Assign(self, new_self);
	Dee_Decref(new_self);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_union(DeeObject **__restrict p_self,
                  DeeObject *items) {
	DREF DeeObject *callback_result;
	callback_result = DeeObject_CallAttr(*p_self,
	                                     (DeeObject *)&str_insertall,
	                                     1,
	                                     (DeeObject **)&items);
	if unlikely(!callback_result)
		goto err;
	Dee_Decref(callback_result);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_symmetric_difference(DeeObject **__restrict p_self,
                                 DeeObject *items) {
	DeeObject *self = *p_self;
	int result;
	DREF DeeObject *new_self;
	new_self = DeeObject_Xor(self, items);
	if unlikely(!new_self)
		goto err;
	result = DeeObject_Assign(self, new_self);
	Dee_Decref(new_self);
	return result;
err:
	return -1;
}

PRIVATE struct type_math set_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ &DeeSet_Invert,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &DeeSet_Union,
	/* .tp_sub         = */ &DeeSet_Difference,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ &DeeSet_Intersection,
	/* .tp_or          = */ &DeeSet_Union,
	/* .tp_xor         = */ &DeeSet_SymmetricDifference,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ &set_inplace_union,
	/* .tp_inplace_sub = */ &set_inplace_sub,
	/* .tp_inplace_mul = */ &set_inplace_mul,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ &set_inplace_intersection,
	/* .tp_inplace_or  = */ &set_inplace_union,
	/* .tp_inplace_xor = */ &set_inplace_symmetric_difference,
	/* .tp_inplace_pow = */ NULL,
};

INTDEF struct type_method tpconst set_methods[];
INTERN_TPCONST struct type_method tpconst set_methods[] = {
	TYPE_METHOD("difference", &set_difference,
	            "(to:?.)->?.\n"
	            "Same as ${this.operator - (to)}"),
	TYPE_METHOD("intersection", &set_intersection,
	            "(with_:?.)->?.\n"
	            "Same as ${this.operator & (with_)}"),
	TYPE_METHOD("isdisjoint", &set_isdisjoint,
	            "(with_:?.)->?Dbool\n"
	            "Returns ?t if ${##(this & with_) == 0}\n"
	            "In other words: If @this and @with_ have no items in common"),
	TYPE_METHOD("union", &set_union,
	            "(with_:?.)->?.\n"
	            "Same as ${this.operator | (with_)}"),
	TYPE_METHOD("symmetric_difference", &set_symmetric_difference,
	            "(with_:?.)->?.\n"
	            "Same as ${this.operator ^ (with_)}"),
	TYPE_METHOD("issubset", &set_issubset,
	            "(of:?.)->?Dbool\n"
	            "Same as ${this.operator <= (of)}"),
	TYPE_METHOD("issuperset", &set_issuperset,
	            "(of:?.)->?Dbool\n"
	            "Same as ${this.operator >= (of)}"),

	/* Default functions for mutable sets */
	TYPE_METHOD(STR_insert, &default_set_insert,
	            "(key)->?Dbool\n"
	            "Insert @key into @this set, returning !t if it was inserted and !f if it was already present"),
	TYPE_METHOD(STR_remove, &default_set_remove,
	            "(key)->?Dbool\n"
	            "Remove @key from @this set, returning !t if it was removed and !f if it wasn't present"),
	TYPE_METHOD(STR_insertall, &default_set_insertall,
	            "(keys:?S?O)\n"
	            "Insert all elements from @keys into @this set"),
	TYPE_METHOD(STR_removeall, &default_set_removeall,
	            "(keys:?S?O)\n"
	            "Remove all elements from @keys from @this set"),
	TYPE_METHOD(STR_unify, &default_set_unify,
	            "(key)->\n"
	            "Insert @key into @this set if it wasn't contained already, and "
	            /**/ "return the (potential) copy of @key that is part of the set"),
	TYPE_METHOD(STR_pop, &default_set_pop,
	            "(def?)->\n"
	            "#tValueError{Set is empty and no @def was given}\n"
	            "Remove and return some random key from @this set. "
	            /**/ "If the set is empty, return @def or throw :ValueError"),
	TYPE_METHOD_END
};

INTDEF struct type_getset tpconst set_getsets[];
INTERN_TPCONST struct type_getset tpconst set_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeRoSet_FromSequence,
	            "->?#Frozen\n"
	            "Returns a copy of @this ?., with all of its current elements frozen in place, "
	            /**/ "constructing a snapshot of the set's current elements. - The actual type of "
	            /**/ "set returned is implementation- and type- specific, and copying itself may "
	            /**/ "either be done immediately, or as copy-on-write"),
	/* TODO: "asseq->?DSequence"  -- alias for `this as Sequence' */
	TYPE_GETSET_END
};


/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("Frozen");
]]]*/
#define Dee_HashStr__Frozen _Dee_HashSelectC(0xa7ed3902, 0x16013e56a91991ea)
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
set_frozen_get(DeeTypeObject *__restrict self) {
	int error;
	DREF DeeTypeObject *result;
	struct attribute_info info;
	struct attribute_lookup_rules rules;
	rules.alr_name       = "Frozen";
	rules.alr_hash       = Dee_HashStr__Frozen;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = ATTR_PERMGET | ATTR_IMEMBER;
	rules.alr_perm_value = ATTR_PERMGET | ATTR_IMEMBER;
	error = DeeObject_FindAttr(Dee_TYPE(self),
	                           (DeeObject *)self,
	                           &info,
	                           &rules);
	if unlikely(error < 0)
		goto err;
	if (error != 0)
		return_reference_(&DeeRoSet_Type);
	if (info.a_attrtype) {
		result = info.a_attrtype;
		Dee_Incref(result);
	} else if (info.a_decl == (DeeObject *)&DeeSet_Type) {
		result = &DeeRoSet_Type;
		Dee_Incref(&DeeRoSet_Type);
	} else {
		if (info.a_doc) {
			/* TODO: Use doc meta-information to determine the return type! */
		}
		/* Fallback: just tell the caller what they already know: a set will be returned... */
		result = &DeeSet_Type;
		Dee_Incref(&DeeSet_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst set_class_getsets[] = {
	TYPE_GETTER("Frozen", &set_frozen_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#frozen property"),
	TYPE_GETSET_END
};


#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS

/* Generic set operators: treat "self" as a read-only set that can be enumerated
 * to yield keys.
 *
 * When "self" doesn't override any sequence operators, throw errors (unless
 * "self" explicitly uses operators from "Set", in which case it is treated
 * like an empty set).
 *
 * For this purpose, trust the return value of `DeeType_GetSeqClass()',
 * and wrap/modify operator invocation such that the object behaves as
 * though it was an indexable sequence. */
#define DeeType_RequireIter(tp_self)                  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireSizeOb(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_sizeob) || DeeType_InheritSize(tp_self))
#define DeeType_RequireSize(tp_self)                  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size) || DeeType_InheritSize(tp_self))
#define DeeType_RequireContains(tp_self)              (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_contains) || DeeType_InheritContains(tp_self))
#define DeeType_RequireForeach(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeachPair(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach_pair) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeachAndForeachPair(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach && (tp_self)->tp_seq->tp_foreach_pair) || DeeType_InheritIter(tp_self))
#define DeeType_RequireHash(tp_self)                  (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_hash) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireCompareEq(tp_self)             (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_compare_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireTryCompareEq(tp_self)          (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_trycompare_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireEq(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireNe(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_ne) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireLo(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_lo) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireLe(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_le) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireGr(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_gr) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireGe(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_ge) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireBool(tp_self)                  (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_set_iter(DeeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_set_sizeob(DeeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) size_t DCALL generic_set_size(DeeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1)) size_t DCALL generic_set_size_fast(DeeObject *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_contains(DeeObject *self, DeeObject *other);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_set_iter(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_RequireIter(tp_self)) {
		if (tp_self->tp_seq->tp_iter == &generic_set_iter)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_seq->tp_iter)(self);
	}
	err_unimplemented_operator(tp_self, OPERATOR_ITER);
	return NULL;
handle_empty:
	return_empty_iterator;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_set_foreach(DeeObject *__restrict self, Dee_foreach_t proc, void *arg) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_seq->tp_foreach)(self, proc, arg);
	}
	return err_unimplemented_operator(tp_self, OPERATOR_ITER);
handle_empty:
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_set_sizeob(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) != Dee_SEQCLASS_NONE && DeeType_RequireSizeOb(tp_self)) {
		if (tp_self->tp_seq->tp_sizeob == &generic_set_sizeob)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_seq->tp_sizeob)(self);
	}
	if (DeeType_RequireForeachAndForeachPair(tp_self)) {
		size_t result;
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		if (!DeeType_IsDefaultForeachPair(tp_self->tp_seq->tp_foreach_pair)) {
			result = DeeSeq_DefaultSizeWithForeachPair(self);
		} else {
			result = DeeSeq_DefaultSizeWithForeach(self);
		}
		if unlikely(result == (size_t)-1)
			goto err;
		return DeeInt_NewSize(result);
	}
	err_unimplemented_operator(tp_self, OPERATOR_SIZE);
err:
	return NULL;
handle_empty:
	return_reference_(DeeInt_Zero);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
generic_set_size(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) != Dee_SEQCLASS_NONE && DeeType_RequireSize(tp_self)) {
		if (tp_self->tp_seq->tp_size == &generic_set_size)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_seq->tp_size)(self);
	}
	if (DeeType_RequireForeachAndForeachPair(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		if (!DeeType_IsDefaultForeachPair(tp_self->tp_seq->tp_foreach_pair)) {
			return DeeSeq_DefaultSizeWithForeachPair(self);
		} else {
			return DeeSeq_DefaultSizeWithForeach(self);
		}
	}
	return (size_t)err_unimplemented_operator(tp_self, OPERATOR_SIZE);
handle_empty:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
generic_set_size_fast(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) != Dee_SEQCLASS_NONE && DeeType_RequireSize(tp_self)) {
		if (tp_self->tp_seq->tp_size_fast == &generic_set_size_fast)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_seq->tp_size_fast)(self);
	}
	return (size_t)-1;
handle_empty:
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_contains(DeeObject *self, DeeObject *other) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	switch (DeeType_GetSeqClass(tp_self)) {
	case Dee_SEQCLASS_SEQ:
	case Dee_SEQCLASS_SET:
		if (DeeType_RequireContains(tp_self)) {
			if (tp_self->tp_seq->tp_contains == &generic_set_contains)
				goto handle_empty; /* Empty set. */
			return (*tp_self->tp_seq->tp_contains)(self, other);
		}
		break;
	case Dee_SEQCLASS_MAP:
		if ((tp_self->tp_seq && tp_self->tp_seq->tp_trygetitem) || DeeType_InheritGetItem(tp_self)) {
			DREF DeeObject *wanted_key_value[2];
			DREF DeeObject *value, *result;
			if (DeeObject_Unpack(other, 2, wanted_key_value))
				goto err;
			value = (*tp_self->tp_seq->tp_trygetitem)(self, wanted_key_value[0]);
			Dee_Decref(wanted_key_value[0]);
			if unlikely(!value) {
				Dee_Decref(wanted_key_value[1]);
				goto err;
			}
			if (value == ITER_DONE) {
				Dee_Decref(wanted_key_value[1]);
				return_false;
			}
			result = DeeObject_CmpEq(wanted_key_value[1], value);
			Dee_Decref(wanted_key_value[1]);
			Dee_Decref(value);
			return result;
		}
		break;
	default: break;
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSeq_DefaultContainsWithForeachDefault(self, other);
	}
	err_unimplemented_operator(tp_self, OPERATOR_CONTAINS);
err:
	return NULL;
handle_empty:
	return_false;
}

PRIVATE struct type_seq set_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&generic_set_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&generic_set_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&generic_set_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&generic_set_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&generic_set_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&generic_set_size_fast,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};


INTERN WUNUSED NONNULL((1)) dhash_t DCALL
generic_set_hash(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireHash(tp_self)) {
		if (tp_self->tp_cmp->tp_hash == &generic_set_hash)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_hash)(self);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultHashWithForeachDefault(self);
	}
	return DeeObject_HashGeneric(self);
handle_empty:
	return DEE_HASHOF_EMPTY_SEQUENCE;
}


INTDEF WUNUSED NONNULL((1)) int DCALL empty_seq_compare(DeeObject *some_object);
INTDEF WUNUSED NONNULL((1)) int DCALL empty_seq_trycompare_eq(DeeObject *some_object);
#define empty_set_compare       empty_seq_compare
#define empty_set_trycompare_eq empty_seq_trycompare_eq

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_set_compare_eq(DeeObject *self, DeeObject *some_object) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireCompareEq(tp_self)) {
		if (tp_self->tp_cmp->tp_compare_eq == &generic_set_compare_eq)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_compare_eq)(self, some_object);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultCompareEqWithForeachDefault(self, some_object);
	}
	err_unimplemented_operator(tp_self, OPERATOR_EQ);
	return Dee_COMPARE_ERR;
handle_empty:
	return empty_set_compare(some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_set_trycompare_eq(DeeObject *self, DeeObject *some_object) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireTryCompareEq(tp_self)) {
		if (tp_self->tp_cmp->tp_trycompare_eq == &generic_set_trycompare_eq)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_trycompare_eq)(self, some_object);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultTryCompareEqWithForeachDefault(self, some_object);
	}
	return -1;
handle_empty:
	return empty_set_trycompare_eq(some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_eq(DeeObject *self, DeeObject *some_object) {
	int result;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireEq(tp_self)) {
		if (tp_self->tp_cmp->tp_eq == &generic_set_eq)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_eq)(self, some_object);
	}
	result = generic_set_compare_eq(self, some_object);
process_result:
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
handle_empty:
	result = empty_set_compare(some_object);
	goto process_result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_ne(DeeObject *self, DeeObject *some_object) {
	int result;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireNe(tp_self)) {
		if (tp_self->tp_cmp->tp_ne == &generic_set_ne)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_ne)(self, some_object);
	}
	result = generic_set_compare_eq(self, some_object);
process_result:
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
handle_empty:
	result = empty_set_compare(some_object);
	goto process_result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_lo(DeeObject *self, DeeObject *some_object) {
	int result;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireLo(tp_self)) {
		if (tp_self->tp_cmp->tp_lo == &generic_set_lo)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_lo)(self, some_object);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultLoWithForeachDefault(self, some_object);
	}
	err_unimplemented_operator(tp_self, OPERATOR_LO);
err:
	return NULL;
handle_empty:
	result = empty_set_compare(some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_le(DeeObject *self, DeeObject *some_object) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireLe(tp_self)) {
		if (tp_self->tp_cmp->tp_le == &generic_set_le)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_le)(self, some_object);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultLeWithForeachDefault(self, some_object);
	}
	err_unimplemented_operator(tp_self, OPERATOR_LE);
	return NULL;
handle_empty:
	return_true;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_gr(DeeObject *self, DeeObject *some_object) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireGr(tp_self)) {
		if (tp_self->tp_cmp->tp_gr == &generic_set_gr)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_gr)(self, some_object);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultGrWithForeachDefault(self, some_object);
	}
	err_unimplemented_operator(tp_self, OPERATOR_GR);
	return NULL;
handle_empty:
	return_false;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_ge(DeeObject *self, DeeObject *some_object) {
	int result;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeType_GetSeqClass(tp_self) == Dee_SEQCLASS_SET && DeeType_RequireGe(tp_self)) {
		if (tp_self->tp_cmp->tp_ge == &generic_set_ge)
			goto handle_empty; /* Empty set. */
		return (*tp_self->tp_cmp->tp_ge)(self, some_object);
	}
	if (DeeType_RequireForeach(tp_self)) {
		if (tp_self->tp_seq->tp_foreach == &generic_set_foreach)
			goto handle_empty; /* Empty set. */
		return DeeSet_DefaultGeWithForeachDefault(self, some_object);
	}
	err_unimplemented_operator(tp_self, OPERATOR_GE);
err:
	return NULL;
handle_empty:
	result = empty_set_compare(some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
}


INTERN struct type_cmp generic_set_cmp = {
	/* .tp_hash          = */ &generic_set_hash,
	/* .tp_compare_eq    = */ &generic_set_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &generic_set_trycompare_eq,
	/* .tp_eq            = */ &generic_set_eq,
	/* .tp_ne            = */ &generic_set_ne,
	/* .tp_lo            = */ &generic_set_lo,
	/* .tp_le            = */ &generic_set_le,
	/* .tp_gr            = */ &generic_set_gr,
	/* .tp_ge            = */ &generic_set_ge,
};

#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_iterself(DeeObject *__restrict self) {
	if unlikely(Dee_TYPE(self) == &DeeSet_Type) {
		/* Special case: Create an empty iterator.
		 * >> This can happen when someone tries to iterate a symbolic empty-mapping object. */
		return_empty_iterator;
	}
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_getitem(DeeObject *self, DeeObject *UNUSED(index)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
set_getrange(DeeObject *self, DeeObject *UNUSED(start), DeeObject *UNUSED(end)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_tpcontains(DeeObject *self, DeeObject *UNUSED(key)) {
	if unlikely(Dee_TYPE(self) == &DeeSet_Type)
		return_false;
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CONTAINS);
	return NULL;
}

PRIVATE struct type_seq set_seq = {
	/* .tp_iter     = */ &set_iterself,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ &set_tpcontains,
	/* .tp_getitem  = */ &set_getitem,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ &set_getrange,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
generic_set_compare_eq(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultCompareEqWithForeachDefault() */
	int result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		return Dee_COMPARE_ERR;
	return result ? 0 : 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_eq(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultCompareEqWithForeachDefault() */
	int result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_ne(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultCompareEqWithForeachDefault() */
	int result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_lo(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultLoWithForeachDefault() */
	int result;
	result = DeeSet_IsTrueSubSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_le(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultLeWithForeachDefault() */
	int result;
	result = DeeSet_IsSubSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_gr(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultGrWithForeachDefault() */
	int result;
	result = DeeSet_IsTrueSubSet(some_object, self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_set_ge(DeeObject *self, DeeObject *some_object) {
	/* TODO: DeeSet_DefaultGeWithForeachDefault() */
	int result;
	result = DeeSet_IsSubSet(self, some_object);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE struct type_cmp generic_set_cmp = {
	/* .tp_hash          = */ &set_hash,
	/* .tp_compare_eq    = */ &generic_set_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ &generic_set_eq,
	/* .tp_ne            = */ &generic_set_ne,
	/* .tp_lo            = */ &generic_set_lo,
	/* .tp_le            = */ &generic_set_le,
	/* .tp_gr            = */ &generic_set_gr,
	/* .tp_ge            = */ &generic_set_ge,
};
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */


/* TODO: insert(object item)->?Dbool
 *       >> // Option #1:
 *       >> return this.insertall([item]) != 0;
 *       >> // Option #2:
 *       >> if (item in this)
 *       >>     return false;
 *       >> this += [item];
 *       >> // Option #3:
 *       >> if (item in this)
 *       >>     return false;
 *       >> this |= [item];
 * TODO: insertall(items:?S?O)->?Dint
 *       >> local result = 0;
 *       >> for (local x: items) {
 *       >>     // Option #1:
 *       >>     if (this.insert([x]))
 *       >>         ++result;
 *       >>     // Option #2:
 *       >>     if (x in this)
 *       >>         continue;
 *       >>     this += [x];
 *       >>     // Option #3:
 *       >>     if (x in this)
 *       >>         continue;
 *       >>     this |= [x];
 *       >>     ++result;
 *       >> }
 *       >> return result;
 * TODO: remove(object item)->?Dbool
 *       >> // Option #1:
 *       >> return this.removeall([item]) != 0;
 *       >> // Option #2:
 *       >> if (item in this)
 *       >>     return false;
 *       >> this -= [item];
 *       >> return true;
 *       >> // Option #3:
 *       >> if (item in this)
 *       >>     return false;
 *       >> this &= ~([item] as Set from deemon);
 *       >> return true;
 * TODO: removeall(items:?S?O)->?Dint
 *       >> local result = 0;
 *       >> for (local x: items) {
 *       >>     // Option #1:
 *       >>     if (this.remove(x))
 *       >>         ++result;
 *       >>     // Option #2:
 *       >>     if (x in this)
 *       >>         continue;
 *       >>     this -= [x];
 *       >>     ++result;
 *       >>     // Option #3:
 *       >>     if (x in this)
 *       >>         continue;
 *       >>     this &= ~([x] as Set from deemon);
 *       >>     ++result;
 *       >> }
 *       >> return result;
 * TODO: clear(): none
 *       >> // Option #1:
 *       >> this := (Set from deemon)();
 *       >> // Option #2:
 *       >> this.removeall(this);
 *       >> // Option #3:
 *       >> this -= this;
 *       >> // Option #4:
 *       >> this &= ~this;
 *       >> // Option #5:
 *       >> this ^= this;
 *       >> // Option #6:
 *       >> for (local x: items)
 *       >>     this.remove(x);
 * TODO: operator += (items:?S?O)
 *       >> // Option #1:
 *       >> this |= items;
 *       >> // Option #2:
 *       >> this.insertall(items);
 *       >> // Option #3:
 *       >> for (local x: items) {
 *       >>     this.insert(x);
 *       >> }
 *       >> // Option #4:
 *       >> this := (this + items)
 *       >> return this;
 * TODO: operator |= (items:?S?O)
 *       >> // Option #1:
 *       >> this += items;
 *       >> // Option #2:
 *       >> this.insertall(items);
 *       >> // Option #3:
 *       >> for (local x: items) {
 *       >>     this.insert(x);
 *       >> }
 *       >> // Option #4:
 *       >> this := (this | items);
 *       >> return this;
 * TODO: operator &= (items:?S?O)
 *       >> if (items is <negated set>) {
 *       >>     // Option #1:
 *       >>     this -= ~items;
 *       >>     // Option #2:
 *       >>     this.removeall(~items);
 *       >>     // Option #3:
 *       >>     for (local x: ~items) {
 *       >>         this.remove(x];
 *       >>     }
 *       >> } else {
 *       >>     // Option #1:
 *       >>     this := (this & items);
 *       >> }
 *       >> return this;
 * TODO: operator := (items:?S?O)
 *       >> this.clear(); // s.a. default handling for clear()
 *       >> // Option #1:
 *       >> this += items;
 *       >> // Option #2:
 *       >> this.insertall(items);
 *       >> // Option #3:
 *       >> for (local x: items) {
 *       >>     this.insert(x);
 *       >> }
 *       >> // Option #4:
 *       >> this := (this | items);
 *       >> return this;
 */

INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));

PRIVATE struct type_operator const set_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_000D_INV, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0011_SUB, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0017_AND, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0018_OR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0019_XOR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE), /* Deleted */
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),  /* Deleted */
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE), /* Deleted */
};

/* `Set from deemon' */
PUBLIC DeeTypeObject DeeSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Set),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any set type "
	                         /**/ "that wishes to implement the Object-Set protocol\n"
	                         "An object derived from this class must implement "
	                         /**/ "${operator contains}, and preferrably ${operator iter}\n"
	                         "\n"

	                         "()\n"
	                         "A no-op default constructor that is implicitly called by sub-classes\n"
	                         "When invoked manually, a general-purpose, empty ?. is returned\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the representation of all sequence elements, "
	                         /**/ "using abstract sequence syntax\n"
	                         "e.g.: ${{ \"foo\", \"bar\", \"baz\" }}\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t indicative of @item being apart of @this ?.\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. is non-empty\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating all items apart of @this ?.\n"
	                         "Note that some set types do not implement this functionality, most "
	                         /**/ "notably symbolically inversed sets\n"
	                         "\n"

	                         "sub->\n"
	                         "Returns a ?. of all objects from @this, excluding those also found in @other\n"
	                         "\n"

	                         "&->\n"
	                         "Returns the intersection of @this and @other\n"
	                         "\n"

	                         "|->\n"
	                         "add->\n"
	                         "Returns the union of @this and @other\n"
	                         "\n"

	                         "^->\n"
	                         "Returns a ?. containing objects only found in either "
	                         /**/ "@this or @other, but not those found in both\n"
	                         "\n"

	                         "<=->\n"
	                         "Returns ?t if all items found in @this ?. can also be found in @other\n"
	                         "\n"

	                         "==->\n"
	                         "Returns ?t if @this ?. contains the same items as @other, and not any more than that\n"
	                         "\n"

	                         "!=->\n"
	                         "Returns ?t if @this contains different, or less items than @other\n"
	                         "\n"

	                         "<->\n"
	                         "The result of ${this <= other && this != other}\n"
	                         "\n"

	                         ">=->\n"
	                         "Returns ?t if all items found in @other can also be found in @this ?.\n"
	                         "\n"

	                         ">->\n"
	                         "The result of ${this >= other && this != other}\n"
	                         "\n"

	                         "~->\n"
	                         "Returns a symbolic ?. that behaves as though it contained "
	                         /**/ "any feasible object that isn't already apart of @this ?.\n"
	                         "Note however that due to the impossibility of such a ?., you "
	                         /**/ "cannot iterate its elements, and the only ~real~ operator "
	                         /**/ "implemented by it is ?#{op:contains}\n"
	                         "Its main purpose is for being used in conjunction with "
	                         /**/ "?#{op:and} in order to create a sub-set that doesn't "
	                         /**/ "contain a certain set of sub-elements:\n"
	                         "${"
	                         /**/ "import Set from deemon;\n"
	                         /**/ "local x = { 10, 11, 15, 20, 30 };\n"
	                         /**/ "local y = { 11, 15 };\n"
	                         /**/ "print repr((x as Set) & ~(y as Set)); // { 10, 20, 30 }"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_SET << Dee_TF_SEQCLASS_SHFT),
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* .tp_features = */ TF_NONE,
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &set_math,
	/* .tp_cmp           = */ &generic_set_cmp,
	/* .tp_seq           = */ &set_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ set_methods,
	/* .tp_getsets       = */ set_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ set_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ set_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(set_operators)
};

PUBLIC DeeObject DeeSet_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeSet_Type)
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SET_C */
