/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
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

DECL_BEGIN


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_Invert(DeeObject *__restrict self) {
	DREF DeeInverseSetObject *result;
	/* Just re-return the original set. */
	if (DeeInverseSet_CheckExact(self))
		return_reference(DeeInverseSet_SET(self));
	/* Construct a new inverse-set wrapper. */
	result = DeeObject_MALLOC(DeeInverseSetObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeInverseSet_Type);
	result->is_set = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
invset_ctor(DeeInverseSetObject *__restrict self) {
	self->is_set = Dee_EmptySet;
	Dee_Incref(Dee_EmptySet);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
invset_init(DeeInverseSetObject *__restrict self,
            size_t argc, DeeObject *const *argv) {
	self->is_set = Dee_EmptySet;
	if (DeeArg_Unpack(argc, argv, "|o:_InverseSet", &self->is_set))
		goto err;
	Dee_Incref(self->is_set);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
invset_fini(DeeInverseSetObject *__restrict self) {
	Dee_Decref(self->is_set);
}

PRIVATE NONNULL((1, 2)) void DCALL
invset_visit(DeeInverseSetObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->is_set);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
invset_repr(DeeInverseSetObject *__restrict self) {
	return DeeString_Newf("~%r", self->is_set);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
invset_iterself(DeeInverseSetObject *__restrict self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERSELF);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invset_tpcontains(DeeInverseSetObject *self, DeeObject *key) {
	int result = DeeObject_Contains(self->is_set, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE struct type_seq invset_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&invset_iterself,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&invset_tpcontains,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
invset_iterator_get(DeeTypeObject *__restrict self) {
	err_unknown_attribute(self,
	                      DeeString_STR(&str_Iterator),
	                      ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE struct type_getset invset_class_getsets[] = {
	{ DeeString_STR(&str_Iterator), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&invset_iterator_get, NULL, NULL },
	{ NULL }
};

PRIVATE struct type_member invset_members[] = {
	TYPE_MEMBER_FIELD("__blacklist__", STRUCT_OBJECT, offsetof(DeeInverseSetObject, is_set)),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeInverseSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_InverseSet",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &invset_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &invset_init,
				TYPE_FIXED_ALLOCATOR(DeeInverseSetObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&invset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&invset_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&invset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &invset_seq,
	/* .tp_iter_next     = */ NULL,
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
	lhs_iter = DeeObject_IterSelf(lhs);
	if unlikely(!lhs_iter)
		goto err;
	while (ITER_ISOK(lhs_item = DeeObject_IterNext(lhs_iter))) {
		/* Check if this item appears in `rhs' */
		temp = DeeObject_Contains(rhs, lhs_item);
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
	if (DeeInverseSet_CheckExact(lhs)) {
		/* An inverse set can only ever be the sub-set of another inverse set. */
		if (!DeeInverseSet_CheckExact(rhs))
			return 0;
		return DeeSet_IsSubSet(DeeInverseSet_SET(rhs),
		                       DeeInverseSet_SET(lhs));
	} else {
		dssize_t result = set_issubset_impl(lhs, rhs);
		return (unlikely(result == -2))
		       ? -1
		       : result < 0
		         ? 0
		         : 1;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_IsTrueSubSet(DeeObject *lhs, DeeObject *rhs) {
	if (DeeInverseSet_CheckExact(lhs)) {
		/* An inverse set can only ever be the sub-set of another inverse set. */
		if (!DeeInverseSet_CheckExact(rhs))
			return 0;
		return DeeSet_IsTrueSubSet(DeeInverseSet_SET(rhs),
		                           DeeInverseSet_SET(lhs));
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
		if (DeeInverseSet_CheckExact(rhs))
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
	if (DeeInverseSet_CheckExact(lhs)) {
		/* An inverse set can never equal a non-inverse set. */
		if (!DeeInverseSet_CheckExact(rhs))
			return 0;
		lhs = DeeInverseSet_SET(lhs);
		rhs = DeeInverseSet_SET(rhs);
	}
	if (DeeInverseSet_CheckExact(rhs))
		return 0; /* A regular set can never match an inverse set. */
	result = set_issubset_impl(lhs, rhs);
	if unlikely(result == -2)
		return -1;
	if (result < 0)
		return 0;
	/* Check the size of `rhs' to make sure
	 * it contains the same number of as `lhs' */
	rhs_size = DeeObject_Size(rhs);
	if unlikely(rhs_size == (size_t)-1)
		return -1;
	return rhs_size == (size_t)result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_IsDisjoint(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *iter, *item;
	int result = 1;
	if (DeeInverseSet_CheckExact(lhs)) {
		/* 2 inverse sets can never be disjoint, because there's
		 * always an imaginary object that is shared by both. */
		if (DeeInverseSet_CheckExact(rhs))
			return 0;
		/* If all elements from `rhs' are black-listed, then
		 * our inverse set is disjoint from it, meaning that
		 * `rhs' is a subset of our black-list. */
		return DeeSet_IsSubSet(rhs, DeeInverseSet_SET(lhs));
	}
	/* Verify that no items from `lhs' appear in `rhs' */
	iter = DeeObject_IterSelf(lhs);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		/* Make sure that `rhs' doesn't contain this item. */
		int temp = DeeObject_Contains(rhs, item);
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
		return NULL;
	return DeeSet_Difference(self, other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_intersection(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:intersection", &other))
		return NULL;
	return DeeSet_Intersection(self, other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_isdisjoint(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:isdisjoint", &other) ||
	    (result = DeeSet_IsDisjoint(self, other)) < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_union(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:union", &other))
		return NULL;
	return DeeSet_Union(self, other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_symmetric_difference(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:symmetric_difference", &other))
		return NULL;
	return DeeSet_SymmetricDifference(self, other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_issubset(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:issubset", &other) ||
	    (result = DeeSet_IsSubSet(self, other)) < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_issuperset(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:issuperset", &other) ||
	    (result = DeeSet_IsSubSet(other, self)) < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_sub(DeeObject **__restrict pself, DeeObject *items) {
	DeeObject *self = *pself;
	size_t i, size;
	DREF DeeObject *remove_func, *callback_result, *remove_args[1];
	remove_func = DeeObject_GetAttr(self, &str_remove);
	if unlikely(!remove_func)
		goto err;
	size = DeeFastSeq_GetSize(items);
	if (size != DEE_FASTSEQ_NOTFAST) {
		for (i = 0; i < size; ++i) {
			remove_args[0] = DeeFastSeq_GetItem(items, i);
			if unlikely(!remove_args[0])
				goto err_func;
			callback_result = DeeObject_Call(remove_func, 1, remove_args);
			Dee_Decref(remove_args[0]);
			if unlikely(!callback_result)
				goto err_func;
			Dee_Decref(callback_result);
		}
	} else {
		items = DeeObject_IterSelf(items);
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
set_inplace_mul(DeeObject **__restrict pself, DeeObject *count) {
	size_t count_integer;
	if (DeeObject_AsSize(count, &count_integer))
		goto err;
	if (count_integer == 0) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallAttr(*pself, &str_clear, 0, NULL);
		if unlikely(!callback_result)
			goto err;
		Dee_Decref(callback_result);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_intersection(DeeObject **__restrict pself, DeeObject *items) {
	DeeObject *self = *pself;
	int result;
	DREF DeeObject *new_self;
	new_self = DeeObject_And(self, items);
	if unlikely(!new_self)
		return -1;
	result = DeeObject_Assign(self, new_self);
	Dee_Decref(new_self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_union(DeeObject **__restrict pself, DeeObject *items) {
	DREF DeeObject *callback_result;
	callback_result = DeeObject_CallAttr(*pself,
	                                     &str_insertall,
	                                     1,
	                                     (DeeObject **)&items);
	if unlikely(!callback_result)
		return -1;
	Dee_Decref(callback_result);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
set_inplace_symmetric_difference(DeeObject **__restrict pself, DeeObject *items) {
	DeeObject *self = *pself;
	int result;
	DREF DeeObject *new_self;
	new_self = DeeObject_Xor(self, items);
	if unlikely(!new_self)
		return -1;
	result = DeeObject_Assign(self, new_self);
	Dee_Decref(new_self);
	return result;
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

INTERN struct type_method set_methods[] = {
	{ "difference", &set_difference,
	  DOC("(to:?.)->?.\n"
	      "Same as ${this.operator - (to)}") },
	{ "intersection", &set_intersection,
	  DOC("(with_:?.)->?.\n"
	      "Same as ${this.operator & (with_)}") },
	{ "isdisjoint", &set_isdisjoint,
	  DOC("(with_:?.)->?Dbool\n"
	      "Returns ?t if ${##(this & with_) == 0}\n"
	      "In other words: If @this and @with_ have no items in common") },
	{ "union", &set_union,
	  DOC("(with_:?.)->?.\n"
	      "Same as ${this.operator | (with_)}") },
	{ "symmetric_difference", &set_symmetric_difference,
	  DOC("(with_:?.)->?.\n"
	      "Same as ${this.operator ^ (with_)}") },
	{ "issubset", &set_issubset,
	  DOC("(of:?.)->?Dbool\n"
	      "Same as ${this.operator <= (of)}") },
	{ "issuperset", &set_issuperset,
	  DOC("(of:?.)->?Dbool\n"
	      "Same as ${this.operator >= (of)}") },
	{ NULL }
};

INTERN struct type_getset set_getsets[] = {
	{ "frozen", &DeeRoSet_FromSequence, NULL, NULL,
	  DOC("->?#Frozen\n"
	      "Returns a copy of @this Set, with all of its current elements frozen in place, "
	      "constructing a snapshot of the Set's current contents. - The actual type of "
	      "Set returned is implementation- and type- specific, and copying itself may "
	      "either be done immediately, or as copy-on-write") },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
set_iterator_get(DeeTypeObject *__restrict self) {
	if (self == &DeeSet_Type)
		return_reference_(&DeeIterator_Type);
	err_unknown_attribute(self,
	                      DeeString_STR(&str_Iterator),
	                      ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
set_frozen_get(DeeTypeObject *__restrict self) {
	int error;
	DREF DeeTypeObject *result;
	struct attribute_info info;
	struct attribute_lookup_rules rules;
	rules.alr_name       = "Frozen";
	rules.alr_hash       = Dee_HashPtr("Frozen", COMPILER_STRLEN("Frozen"));
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = ATTR_PERMGET | ATTR_IMEMBER;
	rules.alr_perm_value = ATTR_PERMGET | ATTR_IMEMBER;
	error = DeeAttribute_Lookup(Dee_TYPE(self),
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
		/* Fallback: just tell the caller what they already know: a Set will be returned... */
		result = &DeeSet_Type;
		Dee_Incref(&DeeSet_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset set_class_getsets[] = {
	{ DeeString_STR(&str_Iterator),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&set_iterator_get,
	  NULL,
	  NULL,
	  DOC("->?DType\n"
	      "Returns the iterator class used by instances of @this Set type\n"
	      "This member must be overwritten by sub-classes of :Set") },
	{ "Frozen",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&set_frozen_get,
	  NULL,
	  NULL,
	  DOC("->?DType\n"
	      "Returns the type of sequence returned by the ?#frozen property") },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_eq(DeeObject *self, DeeObject *some_object) {
	int result;
	result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_ne(DeeObject *self, DeeObject *some_object) {
	int result;
	result = DeeSet_IsSameSet(self, some_object);
	if unlikely(result < 0)
		return NULL;
	return_bool_(!result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_lo(DeeObject *self, DeeObject *some_object) {
	int result;
	result = DeeSet_IsTrueSubSet(self, some_object);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_le(DeeObject *self, DeeObject *some_object) {
	int result;
	result = DeeSet_IsSubSet(self, some_object);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_gr(DeeObject *self, DeeObject *some_object) {
	int result;
	result = DeeSet_IsTrueSubSet(some_object, self);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
set_ge(DeeObject *self, DeeObject *some_object) {
	int result;
	result = DeeSet_IsSubSet(self, some_object);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}


PRIVATE struct type_cmp set_cmp = {
		/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))NULL,
		/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&set_eq,
		/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&set_ne,
		/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&set_lo,
		/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&set_le,
		/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&set_gr,
		/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&set_ge,
};

INTDEF WUNUSED DREF DeeObject *DCALL new_empty_sequence_iterator(void);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_iterself(DeeObject *self) {
	if unlikely(Dee_TYPE(self) == &DeeSet_Type) {
		/* Special case: Create an empty iterator.
		 * >> This can happen when someone tries to iterate a symbolic empty-mapping object. */
		return new_empty_sequence_iterator();
	}
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERSELF);
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
	/* .tp_iter_self = */ &set_iterself,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ &set_tpcontains,
	/* .tp_get       = */ &set_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ &set_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};


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

PUBLIC DeeTypeObject DeeSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Set),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any Set type "
	                        "that wishes to implement the Object-Set protocol\n"
	                        "An object derived from this class must implement "
	                        "${operator contains}, and preferrably ${operator iter}\n"
	                        "\n"
	                        "()\n"
	                        "A no-op default constructor that is implicitly called by sub-classes\n"
	                        "When invoked manually, a general-purpose, empty Set is returned\n"
	                        "\n"
	                        "repr->\n"
	                        "Returns the representation of all sequence elements, "
	                        "using abstract sequence syntax\n"
	                        "e.g.: ${{ \"foo\", \"bar\", \"baz\" }}\n"
	                        "\n"
	                        "contains->\n"
	                        "Returns ?t indicative of @item being apart of @this Set\n"
	                        "\n"
	                        "bool->\n"
	                        "Returns ?t if @this Set is non-empty\n"
	                        "\n"
	                        "iter->\n"
	                        "Returns an iterator for enumerating all items apart of @this Set\n"
	                        "Note that some Set types do not implement this functionality, most "
	                        "notably symbolically inversed sets\n"
	                        "\n"
	                        "sub->\n"
	                        "Returns a Set of all objects from @this, excluding those also found in @other\n"
	                        "\n"
	                        "&->\n"
	                        "Returns the intersection of @this and @other\n"
	                        "\n"
	                        "|->\n"
	                        "add->\n"
	                        "Returns the union of @this and @other\n"
	                        "\n"
	                        "^->\n"
	                        "Returns a Set containing objects only found in either "
	                        "@this or @other, but not those found in both\n"
	                        "\n"
	                        "<=->\n"
	                        "Returns ?t if all items found in @this Set can also be found in @other\n"
	                        "\n"
	                        "==->\n"
	                        "Returns ?t if @this Set contains the same items as @other, and not any more than that\n"
	                        "\n"
	                        "!=->\n"
	                        "Returns ?t if @this contains different, or less items than @other\n"
	                        "\n"
	                        "<->\n"
	                        "The result of ${this <= other && this != other}\n"
	                        "\n"
	                        ">=->\n"
	                        "Returns ?t if all items found in @other can also be found in @this Set\n"
	                        "\n"
	                        ">->\n"
	                        "The result of ${this >= other && this != other}\n"
	                        "\n"
	                        "~->\n"
	                        "Returns a symbolic Set that behaves as though it contained "
	                        "any feasible object that isn't already apart of @this Set\n"
	                        "Note however that due to the impossibility of such a Set, you "
	                        "cannot iterate its elements, and the only ~real~ operator "
	                        "implemented by it is ${operator contains}\n"
	                        "Its main purpose is for being used in conjunction with "
	                        "${operator &} in order to create a sub-set that doesn't "
	                        "contain a certain set of sub-elements:\n"
	                        "${"
	                        "import Set from deemon;\n"
	                        "local x = { 10, 11, 15, 20, 30 };\n"
	                        "local y = { 11, 15 };\n"
	                        "print repr((x as Set) & ~(y as Set)); // { 10, 20, 30 }"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&none_i1, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (void *)&none_i2,
				/* .tp_deep_ctor = */ (void *)&none_i2,
				/* .tp_any_ctor  = */ NULL,
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
	/* .tp_cmp           = */ &set_cmp,
	/* .tp_seq           = */ &set_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ set_methods,
	/* .tp_getsets       = */ set_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ set_class_getsets,
	/* .tp_class_members = */ NULL
};

PUBLIC DeeObject DeeSet_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeSet_Type)
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SET_C */
