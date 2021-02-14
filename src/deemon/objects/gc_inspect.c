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
#ifndef GUARD_DEEMON_OBJECTS_GC_INSPECT_C
#define GUARD_DEEMON_OBJECTS_GC_INSPECT_C 1

#include "gc_inspect.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>

#include <hybrid/atomic.h>

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
	OBJECT_HEAD
	DREF GCSet  *gsi_set;   /* [1..1][const] The set being iterated. */
	DWEAK size_t gsi_index; /* Index of the next set element to-be iterated. */
} GCSetIterator;

#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)            ((x)->gsi_index)
#else /* CONFIG_NO_THREADS */
#define READ_INDEX(x) ATOMIC_READ((x)->gsi_index)
#endif /* !CONFIG_NO_THREADS */


PRIVATE WUNUSED NONNULL((1)) int DCALL
gcsetiterator_ctor(GCSetIterator *__restrict self) {
	self->gsi_set = &DeeGCSet_Empty;
	Dee_Incref(&DeeGCSet_Empty);
	self->gsi_index = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gcsetiterator_copy(GCSetIterator *__restrict self,
                   GCSetIterator *__restrict other) {
	self->gsi_set = other->gsi_set;
	Dee_Incref(self->gsi_set);
	self->gsi_index = READ_INDEX(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gcsetiterator_init(GCSetIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_GCSetIterator", &self->gsi_set))
		goto err;
	if (DeeObject_AssertTypeExact(self->gsi_set, &DeeGCSet_Type))
		goto err;
	Dee_Incref(self->gsi_set);
	self->gsi_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
gcsetiterator_fini(GCSetIterator *__restrict self) {
	Dee_Decref(self->gsi_set);
}

PRIVATE NONNULL((1, 2)) void DCALL
gcsetiterator_visit(GCSetIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->gsi_set);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcsetiterator_next(GCSetIterator *__restrict self) {
	DeeObject *result;
#ifdef CONFIG_NO_THREADS
	size_t new_index;
	new_index = self->gsi_index;
	for (;;) {
		if (new_index > self->gsi_set->gs_mask)
			return ITER_DONE;
		result = self->gsi_set->gs_elem[new_index++];
		if (result)
			break;
	}
	self->gsi_index = new_index;
#else /* CONFIG_NO_THREADS */
	size_t index, new_index;
	do {
		index     = self->gsi_index;
		new_index = index;
		for (;;) {
			if (new_index > self->gsi_set->gs_mask)
				return ITER_DONE;
			result = self->gsi_set->gs_elem[new_index++];
			if (result)
				break;
		}
	} while (!ATOMIC_CMPXCH(self->gsi_index, index, new_index));
#endif /* !CONFIG_NO_THREADS */
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gcsetiterator_bool(GCSetIterator *__restrict self) {
	size_t idx;
	idx = READ_INDEX(self);
	for (;;) {
		if (idx > self->gsi_set->gs_mask)
			return 0;
		if (self->gsi_set->gs_elem[idx++])
			break;
	}
	return 1;
}

PRIVATE struct type_member gcset_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(GCSetIterator, gsi_set), "->?Ert:GCSet"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(GCSetIterator, gsi_index)),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject DeeGCSetIterator_Type;
#define DEFINE_GCSET_ITERATOR_COMPARE(name, op)                       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL             \
	name(GCSetIterator *self, GCSetIterator *other) {                 \
		if (DeeObject_AssertTypeExact(other, &DeeGCSetIterator_Type)) \
			goto err;                                                 \
		return_bool(READ_INDEX(self) op READ_INDEX(other));           \
	err:                                                              \
		return NULL;                                                  \
	}
DEFINE_GCSET_ITERATOR_COMPARE(gcset_iterator_eq, ==)
DEFINE_GCSET_ITERATOR_COMPARE(gcset_iterator_ne, !=)
DEFINE_GCSET_ITERATOR_COMPARE(gcset_iterator_lo, <)
DEFINE_GCSET_ITERATOR_COMPARE(gcset_iterator_le, <=)
DEFINE_GCSET_ITERATOR_COMPARE(gcset_iterator_gr, >)
DEFINE_GCSET_ITERATOR_COMPARE(gcset_iterator_ge, >=)
#undef DEFINE_GCSET_ITERATOR_COMPARE

PRIVATE struct type_cmp gcset_iterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_iterator_ge,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&gcsetiterator_ctor,
				/* .tp_copy_ctor = */ (void *)&gcsetiterator_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&gcsetiterator_init,
				TYPE_FIXED_ALLOCATOR(GCSetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gcsetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&gcsetiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&gcsetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &gcset_iterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcsetiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ gcset_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED DREF GCSet *DCALL gcset_ctor(void) {
	return_reference_(&DeeGCSet_Empty);
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
				goto err;
			error = GCSetMaker_Insert(&maker, copy);
			if (error == 0)
				continue;
			Dee_Decref(copy);
			if unlikely(error < 0)
				goto err;
		}
		return GCSetMaker_Pack(&maker);
err:
		GCSetMaker_Fini(&maker);
		return NULL;
	}
}

PRIVATE NONNULL((1)) void DCALL
gcset_fini(GCSet *__restrict self) {
	size_t i;
	for (i = 0; i <= self->gs_mask; ++i)
		Dee_XDecref(self->gs_elem[i]);
}

PRIVATE NONNULL((1, 2)) void DCALL
gcset_visit(GCSet *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->gs_mask; ++i)
		Dee_XVisit(self->gs_elem[i]);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcset_size(GCSet *__restrict self) {
	return DeeInt_NewSize(self->gs_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
gcset_contains(GCSet *self, DeeObject *other) {
	dhash_t i, perturb, j;
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
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcset_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcset_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcset_contains
};

PRIVATE struct type_member gcset_class_members[] = {
    TYPE_MEMBER_CONST("Iterator", &DeeGCSetIterator_Type),
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
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcset_ctor,
				/* .tp_copy_ctor = */ &DeeObject_NewRef,
				/* .tp_deep_ctor = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcset_deepcopy,
				/* .tp_any_ctor  = */ NULL,
				/* .tp_free      = */ NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gcset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&gcset_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&gcset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &gcset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */gcset_class_members
};

INTERN GCSet DeeGCSet_Empty = {
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
	size_t i;
	GCSet *set = self->gs_set;
	if (!set)
		return;
	for (i = 0; i <= set->gs_mask; ++i)
		Dee_XDecref(set->gs_elem[i]);
	DeeObject_Free(set);
}

INTERN WUNUSED NONNULL((1)) bool DCALL
GCSetMaker_Rehash(GCSetMaker *__restrict self) {
	size_t new_mask;
	GCSet *new_set, *old_set;
	if ((old_set = self->gs_set) != NULL)
		new_mask = (old_set->gs_mask << 1) | 1;
	else {
		new_mask = 31;
	}
	new_set = (GCSet *)DeeObject_TryCalloc(offsetof(GCSet, gs_elem) +
	                                       (new_mask + 1) * sizeof(DREF DeeObject *));
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
INTERN int DCALL
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
		return -1;
	goto again;
}


/* Remove all non-GC objects from the given set. */
INTERN WUNUSED NONNULL((1)) int DCALL
GCSetMaker_RemoveNonGC(GCSetMaker *__restrict self) {
	GCSet *new_set, *old_set;
	size_t i, j, perturb;
	old_set = self->gs_set;
	if (!old_set)
		return 0;
	new_set = (GCSet *)DeeObject_TryCalloc(offsetof(GCSet, gs_elem) +
	                                       (old_set->gs_mask + 1) *
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
		result = &DeeGCSet_Empty;
		Dee_Incref(result);
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReferred(DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReferred(&maker, start))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReachable(DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReachable(&maker, start))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReferredGC(DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReferred(&maker, start))
		goto err;
	if (GCSetMaker_RemoveNonGC(&maker))
		goto err;
	return GCSetMaker_Pack(&maker);
err:
	GCSetMaker_Fini(&maker);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF GCSet *DCALL
DeeGC_NewReachableGC(DeeObject *__restrict start) {
	GCSetMaker maker = GCSETMAKER_INIT;
	if (DeeGC_CollectReachable(&maker, start))
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
			DeeObject_Visit(self, (dvisit_t)&visit_reachable_func, data);
		}
	}
}

/* Collect referred, or reachable objects.
 * @return:  0: Collection was OK.
 * @return: -1: An error occurred. (not enough memory) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeGC_CollectReferred(GCSetMaker *__restrict self,
                      DeeObject *__restrict start) {
again:
	self->gs_err = 0;
	DeeObject_Visit(start, (dvisit_t)&visit_referr_func, self);
	if unlikely(self->gs_err) {
		if (Dee_CollectMemory(self->gs_err))
			goto again;
		return -1;
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeGC_CollectReachable(GCSetMaker *__restrict self,
                       DeeObject *__restrict start) {
again:
	self->gs_err = 0;
	DeeObject_Visit(start, (dvisit_t)&visit_reachable_func, self);
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
	bool did;
};

PRIVATE void DCALL
visit_referred_by_func(DeeObject *__restrict self,
                       struct visit_referred_by_data *__restrict data) {
	if (data->target == self)
		data->did = true;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeGC_ReferredBy(DeeObject *__restrict source, DeeObject *__restrict target) {
	struct visit_referred_by_data data;
	/* Check for special case: the 2 objects are identical. */
	if (source == target)
		return true;
	data.target = target;
	data.did    = false;
	DeeObject_Visit(source, (dvisit_t)&visit_referred_by_func, &data);
	return data.did;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GC_INSPECT_C */
