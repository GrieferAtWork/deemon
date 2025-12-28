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
#ifndef GUARD_DEEMON_OBJECTS_GC_INSPECT_C
#define GUARD_DEEMON_OBJECTS_GC_INSPECT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/super.h>
#include <deemon/util/atomic.h>
/**/

#include "../runtime/strings.h"
#include "gc_inspect.h"
#include "generic-proxy.h"
/**/

#include <stddef.h> /* size_t, offsetof */

DECL_BEGIN

/* GC inspection provides things such as sequences implemented using the `tp_visit'
 * interface, allowing user-code to determine if object `a' is reachable from `b',
 * as well as enumerate objects reachable from some point, as well as determine how
 * far one would have to travel before getting to the destination.
 * Note that only non-trivial objects are guarantied to be testable for this
 * purpose, meaning that the implementation is allowed to not account for objects
 * that are unable to ever cause a reference loop, the main example here being
 * strings, or conversely: sequences of string, as well as integer members that
 * are only created when the associated attribute is accessed. */

typedef struct {
	PROXY_OBJECT_HEAD_EX(GCSet, gsi_set);  /* [1..1][const] The set being iterated. */
	DWEAK size_t                gsi_index; /* [lock(ATOMIC)] Index of the next set element to-be iterated. */
} GCSetIterator;
#define READ_INDEX(x) atomic_read(&(x)->gsi_index)


PRIVATE WUNUSED NONNULL((1)) int DCALL
gcsetiterator_ctor(GCSetIterator *__restrict self) {
	self->gsi_set   = DeeGCSet_NewEmpty();
	self->gsi_index = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gcsetiterator_copy(GCSetIterator *__restrict self,
                   GCSetIterator *__restrict other) {
	self->gsi_set = other->gsi_set;
	Dee_Incref(self->gsi_set);
	self->gsi_index = atomic_read(&other->gsi_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gcsetiterator_init(GCSetIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_GCSetIterator", &self->gsi_set);
	if (DeeObject_AssertTypeExact(self->gsi_set, &DeeGCSet_Type))
		goto err;
	Dee_Incref(self->gsi_set);
	self->gsi_index = 0;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(GCSetIterator, gsi_set) == offsetof(ProxyObject, po_obj));
#define gcsetiterator_fini  generic_proxy__fini
#define gcsetiterator_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcsetiterator_next(GCSetIterator *__restrict self) {
	DeeObject *result;
	size_t index, new_index;
	do {
		index     = atomic_read(&self->gsi_index);
		new_index = index;
		for (;;) {
			if (new_index > self->gsi_set->gs_mask)
				return ITER_DONE;
			result = self->gsi_set->gs_elem[new_index++];
			if (result)
				break;
		}
	} while (!atomic_cmpxch_weak_or_write(&self->gsi_index, index, new_index));
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gcsetiterator_bool(GCSetIterator *__restrict self) {
	size_t idx;
	idx = atomic_read(&self->gsi_index);
	for (;;) {
		if (idx > self->gsi_set->gs_mask)
			return 0;
		if (self->gsi_set->gs_elem[idx++])
			break;
	}
	return 1;
}

PRIVATE struct type_member tpconst gcset_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(GCSetIterator, gsi_set), "->?Ert:GCSet"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(GCSetIterator, gsi_index)),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject DeeGCSetIterator_Type;

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
gcset_iterator_hash(GCSetIterator *self) {
	return atomic_read(&self->gsi_index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gcset_iterator_compare(GCSetIterator *self, GCSetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &DeeGCSetIterator_Type))
		goto err;
	Dee_return_compareT(size_t, atomic_read(&self->gsi_index),
	                    /*   */ atomic_read(&other->gsi_index));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp gcset_iterator_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&gcset_iterator_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


INTERN DeeTypeObject DeeGCSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCSetIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ GCSetIterator,
			/* tp_ctor:        */ &gcsetiterator_ctor,
			/* tp_copy_ctor:   */ &gcsetiterator_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &gcsetiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gcsetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&gcsetiterator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&gcsetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &gcset_iterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcsetiterator_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ gcset_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED DREF GCSet *DCALL gcset_ctor(void) {
	return DeeGCSet_NewEmpty();
}

PRIVATE WUNUSED NONNULL((1)) DREF GCSet *DCALL
gcset_deepcopy(GCSet *__restrict self) {
	if (!self->gs_size)
		return gcset_ctor();
	{
		size_t i;
		GCSetMaker maker = GCSETMAKER_INIT;
		for (i = 0; i <= self->gs_mask; ++i) {
			DREF DeeObject *copy;
			int error;
			if (!self->gs_elem[i])
				continue;
			copy = DeeObject_DeepCopy(self->gs_elem[i]);
			if unlikely(!copy)
				goto err_maker;
			error = GCSetMaker_Insert(&maker, copy);
			if (error == 0)
				continue;
			Dee_Decref(copy);
			if unlikely(error < 0)
				goto err_maker;
		}
		return GCSetMaker_Pack(&maker);
err_maker:
		GCSetMaker_Fini(&maker);
		return NULL;
	}
}

PRIVATE NONNULL((1)) void DCALL
gcset_fini(GCSet *__restrict self) {
	Dee_XDecrefv(self->gs_elem, self->gs_mask + 1);
}

PRIVATE NONNULL((1, 2)) void DCALL
gcset_visit(GCSet *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisitv(self->gs_elem, self->gs_mask + 1);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gcset_bool(GCSet *__restrict self) {
	return self->gs_size != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF GCSetIterator *DCALL
gcset_iter(GCSet *__restrict self) {
	DREF GCSetIterator *result;
	result = DeeObject_MALLOC(GCSetIterator);
	if unlikely(!result)
		goto done;
	result->gsi_index = 0;
	result->gsi_set   = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeGCSetIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
gcset_size(GCSet *__restrict self) {
	return self->gs_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
gcset_contains(GCSet *self, DeeObject *other) {
	Dee_hash_t i, perturb, j;
	i = perturb = GCSET_HASHOBJ(other) & self->gs_mask;
	for (;; GCSET_HASHNXT(i, perturb)) {
		j = i & self->gs_mask;
		if (!self->gs_elem[j])
			break;
		if (self->gs_elem[j] == other)
			return_true;
	}
	return_false;
}

PRIVATE struct type_seq gcset_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcset_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_contains,
	/* .tp_getitem                    = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem                    = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&gcset_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&gcset_size,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
};

PRIVATE struct type_member tpconst gcset_class_members[] = {
    TYPE_MEMBER_CONST(STR_Iterator, &DeeGCSetIterator_Type),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeGCSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCSet",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &gcset_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &gcset_deepcopy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gcset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&gcset_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&gcset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__F6E3D7B2219AE1EB),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__A5C53AFDF1233C5A),
	/* .tp_seq           = */ &gcset_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ gcset_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN GCSet_Empty DeeGCSet_Empty = {
	OBJECT_HEAD_INIT(&DeeGCSet_Type),
	/* .gs_size = */ 0,
	/* .gs_mask = */ 0,
	/* .gs_elem = */ {
		/* [0] = */ NULL
	}
};



/* Finalize the given GC-set maker. */
INTERN NONNULL((1)) void DCALL
GCSetMaker_Fini(GCSetMaker *__restrict self) {
	GCSet *set = self->gs_set;
	if (set) {
		Dee_XDecrefv(set->gs_elem, set->gs_mask + 1);
		DeeObject_Free(set);
	}
}

INTERN WUNUSED NONNULL((1)) bool DCALL
GCSetMaker_Rehash(GCSetMaker *__restrict self) {
	size_t new_mask;
	GCSet *new_set, *old_set;
	if ((old_set = self->gs_set) != NULL) {
		new_mask = (old_set->gs_mask << 1) | 1;
	} else {
		new_mask = 31;
	}
	new_set = (GCSet *)DeeObject_TryCallocc(offsetof(GCSet, gs_elem),
	                                        new_mask + 1, sizeof(DREF DeeObject *));
	if unlikely(!new_set)
		return false;
	new_set->gs_mask = new_mask;
	self->gs_set     = new_set;
	if (old_set) {
		size_t i, j, perturb;
		new_set->gs_size = old_set->gs_size;
		for (i = 0; i <= old_set->gs_mask; ++i) {
			DREF DeeObject *elem = old_set->gs_elem[i];
			if (!elem)
				continue;
			j = perturb = GCSET_HASHOBJ(elem) & new_mask;
			for (;; GCSET_HASHNXT(j, perturb)) {
				size_t k = j & new_mask;
				if (new_set->gs_elem[k])
					continue;
				new_set->gs_elem[k] = elem;
				break;
			}
		}
		DeeObject_Free(old_set);
	}
	return true;
}

/* @return:  1: Object was already inserted into the set.
 * @return:  0: Object was newly inserted into the set.
 * @return: -1: An allocation failed (release all locks and to collect `self->gs_err' bytes of memory) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
GCSetMaker_Insert(GCSetMaker *__restrict self,
                  /*inherit(return == 0)*/ DREF DeeObject *__restrict ob) {
	size_t j, i, perturb;
	GCSet *set;
again:
	set = self->gs_set;
	if (set) {
		/* Check if the object is already apart of the set. */
		i = perturb = GCSET_HASHOBJ(ob) & set->gs_mask;
		for (;; GCSET_HASHNXT(i, perturb)) {
			j = i & set->gs_mask;
			if (set->gs_elem[j] == ob)
				return 1;
			if (!set->gs_elem[j])
				break;
		}
		if (set->gs_size < (set->gs_mask * 2) / 3) {
			set->gs_elem[j] = ob; /* Inherit reference. */
			++set->gs_size;
			return 0;
		}
	}
	if (!GCSetMaker_Rehash(self))
		goto err;
	goto again;
err:
	return -1;
}


/* Remove all non-GC objects from the given set. */
INTERN WUNUSED NONNULL((1)) int DCALL
GCSetMaker_RemoveNonGC(GCSetMaker *__restrict self) {
	GCSet *new_set, *old_set;
	size_t i, j, perturb;
	old_set = self->gs_set;
	if (!old_set)
		return 0;
	new_set = (GCSet *)DeeObject_TryCallocc(offsetof(GCSet, gs_elem),
	                                        old_set->gs_mask + 1,
	                                        sizeof(DREF DeeObject *));
	if unlikely(!new_set)
		return false;
	new_set->gs_mask = old_set->gs_mask;
	self->gs_set     = new_set;
	new_set->gs_size = old_set->gs_size;
	for (i = 0; i <= old_set->gs_mask; ++i) {
		DREF DeeObject *elem = old_set->gs_elem[i];
		if (!elem)
			continue;
		if (!DeeType_IsGC(Dee_TYPE(elem))) {
			/* Not a GC object. */
			Dee_Decref(elem);
			continue;
		}
		j = perturb = GCSET_HASHOBJ(elem) & new_set->gs_mask;
		for (;; GCSET_HASHNXT(j, perturb)) {
			size_t k = j & new_set->gs_mask;
			if (new_set->gs_elem[k])
				continue;
			new_set->gs_elem[k] = elem;
			break;
		}
	}
	DeeObject_Free(old_set);
	return 0;
}


/* Pack the current set of objects and return them after (always) finalizing `self' */
INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
GCSetMaker_Pack(/*inherit(always)*/ GCSetMaker *__restrict self) {
	DREF GCSet *result;
	if ((result = self->gs_set) != NULL) {
		DeeObject_Init(result, &DeeGCSet_Type);
	} else {
		result = DeeGCSet_NewEmpty();
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReferred(DeeObject *__restrict start) {
	DeeTypeObject *tp_start = Dee_TYPE(start);
	if (tp_start == &DeeSuper_Type) {
		tp_start = DeeSuper_TYPE(start);
		start    = DeeSuper_SELF(start);
	}
	return DeeGC_TNewReferred(tp_start, start);
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReachable(DeeObject *__restrict start) {
	DeeTypeObject *tp_start = Dee_TYPE(start);
	if (tp_start == &DeeSuper_Type) {
		tp_start = DeeSuper_TYPE(start);
		start    = DeeSuper_SELF(start);
	}
	return DeeGC_TNewReachable(tp_start, start);
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReferredGC(DeeObject *__restrict start) {
	DeeTypeObject *tp_start = Dee_TYPE(start);
	if (tp_start == &DeeSuper_Type) {
		tp_start = DeeSuper_TYPE(start);
		start    = DeeSuper_SELF(start);
	}
	return DeeGC_TNewReferredGC(tp_start, start);
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReachableGC(DeeObject *__restrict start) {
	DeeTypeObject *tp_start = Dee_TYPE(start);
	if (tp_start == &DeeSuper_Type) {
		tp_start = DeeSuper_TYPE(start);
		start    = DeeSuper_SELF(start);
	}
	return DeeGC_TNewReachableGC(tp_start, start);
}


INTERN WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL
DeeGC_TNewReferred(DeeTypeObject *tp_start,
                   DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReferred(&maker, tp_start, start))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL
DeeGC_TNewReachable(DeeTypeObject *tp_start,
                    DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReachable(&maker, tp_start, start))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL
DeeGC_TNewReferredGC(DeeTypeObject *tp_start,
                     DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReferred(&maker, tp_start, start))
		goto err;
	if (GCSetMaker_RemoveNonGC(&maker))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL
DeeGC_TNewReachableGC(DeeTypeObject *tp_start,
                      DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReachable(&maker, tp_start, start))
		goto err;
	if (GCSetMaker_RemoveNonGC(&maker))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewGCReferred(DeeObject *__restrict target) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectGCReferred(&maker, target))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}


PRIVATE NONNULL((1, 2)) void DCALL
visit_referr_func(DeeObject *__restrict self,
                  GCSetMaker *__restrict data) {
	if (data->gs_err == 0) {
		int error = GCSetMaker_Insert(data, self);
		if (error == 0)
			Dee_Incref(self);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
visit_reachable_func(DeeObject *__restrict self,
                     GCSetMaker *__restrict data) {
	if (data->gs_err == 0) {
		int error = GCSetMaker_Insert(data, self);
		if (error == 0) {
			Dee_Incref(self);
			DeeObject_Visit(self, (Dee_visit_t)&visit_reachable_func, data);
		}
	}
}

/* Collect referred, or reachable objects.
 * @return:  0: Collection was OK.
 * @return: -1: An error occurred. (not enough memory) */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeGC_CollectReferred(GCSetMaker *__restrict self,
                      DeeTypeObject *tp_start,
                      DeeObject *__restrict start) {
again:
	self->gs_err = 0;
	DeeObject_TVisit(tp_start, start, (Dee_visit_t)&visit_referr_func, self);
	if unlikely(self->gs_err) {
		if (Dee_CollectMemory(self->gs_err))
			goto again;
		return -1;
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeGC_CollectReachable(GCSetMaker *__restrict self,
                       DeeTypeObject *tp_start,
                       DeeObject *__restrict start) {
again:
	self->gs_err = 0;
	DeeObject_TVisit(tp_start, start, (Dee_visit_t)&visit_reachable_func, self);
	if unlikely(self->gs_err) {
		if (Dee_CollectMemory(self->gs_err))
			goto again;
		return -1;
	}
	return 0;
}


/* Returns `true' if `target' is directly referred to by `source' */
struct visit_referred_by_data {
	DeeObject *target;
	bool       did;
};

PRIVATE NONNULL((1, 2)) void DCALL
visit_referred_by_func(DeeObject *__restrict self,
                       struct visit_referred_by_data *__restrict data) {
	if (data->target == self)
		data->did = true;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeGC_ReferredBy(DeeObject *source,
                 DeeObject *target) {
	struct visit_referred_by_data data;
	/* Check for special case: the 2 objects are identical. */
	if (source == target)
		return true;
	data.target = target;
	data.did    = false;
	DeeObject_Visit(source, (Dee_visit_t)&visit_referred_by_func, &data);
	return data.did;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GC_INSPECT_C */
