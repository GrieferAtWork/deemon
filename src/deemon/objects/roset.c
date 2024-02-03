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
#ifndef GUARD_DEEMON_OBJECTS_ROSET_C
#define GUARD_DEEMON_OBJECTS_ROSET_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

typedef DeeRoSetObject RoSet;

typedef struct {
	OBJECT_HEAD
	RoSet             *rosi_set;  /* [1..1][const] The set being iterated. */
	struct roset_item *rosi_next; /* [?..1][in(rosi_set->rs_elem)][atomic]
	                               * The first candidate for the next item. */
} RoSetIterator;
#define READ_ITEM(x) atomic_read(&(x)->rosi_next)

INTDEF DeeTypeObject RoSetIterator_Type;

INTERN WUNUSED NONNULL((1)) int DCALL
rosetiterator_ctor(RoSetIterator *__restrict self) {
	self->rosi_set = DeeRoSet_New();
	if unlikely(!self->rosi_set)
		goto err;
	self->rosi_next = self->rosi_set->rs_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
rosetiterator_init(RoSetIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	RoSet *set;
	if (DeeArg_Unpack(argc, argv, "o:_RoSetIterator", &set))
		goto err;
	if (DeeObject_AssertTypeExact(set, &DeeRoSet_Type))
		goto err;
	self->rosi_set = set;
	Dee_Incref(set);
	self->rosi_next = set->rs_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
rosetiterator_copy(RoSetIterator *__restrict self,
                   RoSetIterator *__restrict other) {
	self->rosi_set = other->rosi_set;
	Dee_Incref(self->rosi_set);
	self->rosi_next = READ_ITEM(other);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
rosetiterator_fini(RoSetIterator *__restrict self) {
	Dee_Decref(self->rosi_set);
}

PRIVATE NONNULL((1, 2)) void DCALL
rosetiterator_visit(RoSetIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rosi_set);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rosetiterator_bool(RoSetIterator *__restrict self) {
	struct roset_item *item = READ_ITEM(self);
	RoSet *set                = self->rosi_set;
	for (;; ++item) {
		/* Check if the iterator is in-bounds. */
		if (item > set->rs_elem + set->rs_mask)
			return 0;
		if (item->rsi_key)
			break;
	}
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rosetiterator_next(RoSetIterator *__restrict self) {
	struct roset_item *item, *end;
	end = self->rosi_set->rs_elem + self->rosi_set->rs_mask + 1;
	for (;;) {
		struct roset_item *old_item;
		item     = atomic_read(&self->rosi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->rsi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rosi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rosi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->rsi_key);
iter_exhausted:
	return ITER_DONE;
}

INTDEF DeeTypeObject RoSetIterator_Type;
#define DEFINE_ROSETITERATOR_COMPARE(name, op)                \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL     \
	name(RoSetIterator *self, RoSetIterator *other) {         \
		if (DeeObject_AssertType(other, &RoSetIterator_Type)) \
			goto err;                                         \
		return_bool(READ_ITEM(self) op READ_ITEM(other));     \
	err:                                                      \
		return NULL;                                          \
	}
DEFINE_ROSETITERATOR_COMPARE(rosetiterator_eq, ==)
DEFINE_ROSETITERATOR_COMPARE(rosetiterator_ne, !=)
DEFINE_ROSETITERATOR_COMPARE(rosetiterator_lo, <)
DEFINE_ROSETITERATOR_COMPARE(rosetiterator_le, <=)
DEFINE_ROSETITERATOR_COMPARE(rosetiterator_gr, >)
DEFINE_ROSETITERATOR_COMPARE(rosetiterator_ge, >=)
#undef DEFINE_ROSETITERATOR_COMPARE

PRIVATE struct type_cmp rosetiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_ge
};


PRIVATE struct type_member tpconst roset_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RoSetIterator, rosi_set), "->?Ert:RoSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RoSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSetIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rosetiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rosetiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&rosetiterator_init,
				TYPE_FIXED_ALLOCATOR(RoSetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rosetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rosetiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rosetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rosetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rosetiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ roset_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





#define ROSET_ALLOC(mask)  ((DREF RoSet *)DeeObject_Calloc(SIZEOF_ROSET(mask)))
#define SIZEOF_ROSET(mask) (offsetof(RoSet, rs_elem) + (((mask) + 1) * sizeof(struct roset_item)))
#define ROSET_INITIAL_MASK 0x03

PRIVATE WUNUSED NONNULL((1)) DREF RoSet *DCALL
DeeRoSet_Rehash(/*inherit(on_success)*/ DREF RoSet *__restrict self, size_t new_mask) {
	DREF RoSet *result;
	size_t i;
	result = ROSET_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	ASSERT(self->ob_refcnt == 1);
	ASSERT(self->ob_type == &DeeRoSet_Type);
	result->ob_refcnt = 1;
	result->ob_type = &DeeRoSet_Type;
	result->rs_size = self->rs_size;
	result->rs_mask = new_mask;
	for (i = 0; i <= self->rs_mask; ++i) {
		size_t j, perturb;
		struct roset_item *item;
		if (!self->rs_elem[i].rsi_key)
			continue;
		perturb = j = self->rs_elem[i].rsi_hash & new_mask;
		for (;; ROSET_HASHNX(j, perturb)) {
			item = &result->rs_elem[j & new_mask];
			if (!item->rsi_key)
				break;
		}
		/* Copy the old item into the new slot. */
		memcpy(item, &self->rs_elem[i], sizeof(struct roset_item));
	}
	DeeObject_Free(self);
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeRoSet_Insert(/*in|out*/ DREF RoSet **__restrict p_self,
                DeeObject *__restrict key) {
	size_t i, perturb, hash;
	struct roset_item *item;
	DREF RoSet *me = *p_self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeRoSet_Type);
	ASSERT(!DeeObject_IsShared(me));
	ASSERT(key != (DeeObject *)me);
	if unlikely(me->rs_size * 2 > me->rs_mask) {
		size_t new_mask = (me->rs_mask << 1) | 1;
		me = DeeRoSet_Rehash(me, new_mask);
		if unlikely(!me)
			goto err;
		*p_self = me;
	}

	/* Insert the new key/value-pair into the RoSet. */
	hash    = DeeObject_Hash(key);
	perturb = i = hash & me->rs_mask;
	for (;; ROSET_HASHNX(i, perturb)) {
		int error;
		item = &me->rs_elem[i & me->rs_mask];
		if (!item->rsi_key)
			break;
		if (item->rsi_hash != hash)
			continue;

		/* Same hash. -> Check if it's also the same key. */
		error = DeeObject_CompareEq(key, item->rsi_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Not the same key. */

		/* It _is_ the same key! (override it...) */
		--me->rs_size;
		Dee_Decref(item->rsi_key);
		break;
	}

	/* Fill in the item. */
	++me->rs_size;
	item->rsi_hash  = hash;
	item->rsi_key   = key;
	Dee_Incref(key);
	return 0;
err:
	return -1;
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define DeeRoSet_InsertSequence_foreach (*(Dee_foreach_t)&DeeRoSet_Insert)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
DeeRoSet_InsertSequence_foreach(void *arg, DeeObject *elem) {
	return DeeRoSet_Insert((RoSet **)arg, elem);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromSequence(DeeObject *__restrict sequence) {
	DREF RoSet *result;
	size_t length_hint, mask;

	/* Optimization: Since rodicts are immutable, re-return if the
	 *               given sequence already is a read-only RoSet. */
	if (DeeRoSet_CheckExact(sequence))
		return_reference_(sequence);

	/* TODO: Optimization: if (DeeSet_CheckExact(sequence)) ...
	 * (fix the set's hash-vector to not contain dummies,
	 * then copy as-is) */

	/* Construct a read-only RoSet from a generic sequence. */
	mask        = ROSET_INITIAL_MASK;
	length_hint = DeeFastSeq_GetSize(sequence);
	if (length_hint != DEE_FASTSEQ_NOTFAST) {
		while (mask <= length_hint)
			mask = (mask << 1) | 1;
		mask = (mask << 1) | 1;
	}
	result = ROSET_ALLOC(mask);
	if likely(result) {
		/*result->rd_size = 0;*/
		result->rs_mask = mask;
		DeeObject_Init(result, &DeeRoSet_Type);
		if unlikely(DeeObject_Foreach(sequence, &DeeRoSet_InsertSequence_foreach, &result))
			goto err_r;
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_DecrefDokill(result);
/*err:*/
	return NULL;
}

/* Internal functions for constructing a read-only set object. */
PUBLIC WUNUSED DREF RoSet *DCALL
DeeRoSet_New(void) {
	DREF RoSet *result;
	result = ROSET_ALLOC(ROSET_INITIAL_MASK);
	if unlikely(!result)
		goto done;
	result->rs_mask = ROSET_INITIAL_MASK;
	result->rs_size = 0;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return result;
}

PUBLIC WUNUSED DREF RoSet *DCALL
DeeRoSet_NewWithHint(size_t num_items) {
	DREF RoSet *result;
	size_t mask = ROSET_INITIAL_MASK;
	while (mask < num_items)
		mask = (mask << 1) | 1;
	mask   = (mask << 1) | 1;
	result = ROSET_ALLOC(mask);
	if unlikely(!result)
		goto done;
	result->rs_mask = mask;
	result->rs_size = 0;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF RoSetIterator *DCALL
roset_iter(RoSet *__restrict self) {
	DREF RoSetIterator *result;
	result = DeeObject_MALLOC(RoSetIterator);
	if unlikely(!result)
		goto done;
	result->rosi_set  = self;
	result->rosi_next = self->rs_elem;
	Dee_Incref(self);
	DeeObject_Init(result, &RoSetIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_size(RoSet *__restrict self) {
	return DeeInt_NewSize(self->rs_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
roset_contains(RoSet *self,
               DeeObject *key) {
	size_t i, perturb, hash;
	struct roset_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = ROSET_HASHST(self, hash);
	for (;; ROSET_HASHNX(i, perturb)) {
		int error;
		item = ROSET_HASHIT(self, i);
		if (!item->rsi_key)
			break;
		if (item->rsi_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->rsi_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_true;
	}
	return_false;
err:
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
roset_fini(RoSet *__restrict self) {
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].rsi_key)
			continue;
		Dee_Decref(self->rs_elem[i].rsi_key);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
roset_visit(RoSet *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].rsi_key)
			continue;
		Dee_Visit(self->rs_elem[i].rsi_key);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
roset_bool(RoSet *__restrict self) {
	return self->rs_size != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
roset_printrepr(RoSet *__restrict self,
                dformatprinter printer, void *arg) {
	dssize_t temp, result;
	bool is_first = true;
	size_t i;
	result = DeeFormat_PRINT(printer, arg, "HashSet.Frozen({ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].rsi_key)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_PrintObjectRepr(printer, arg, self->rs_elem[i].rsi_key));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
roset_hash(RoSet *__restrict self) {
	size_t i;
	dhash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].rsi_key)
			continue;
		result ^= DeeObject_Hash(self->rs_elem[i].rsi_key);
	}
	return result;
}

PRIVATE struct type_cmp roset_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&roset_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &roset_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &roset_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &roset_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &roset_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &roset_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &roset_ge,
};

PRIVATE struct type_seq roset_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&roset_contains
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_sizeof(RoSet *self) {
	return DeeInt_NewSize(offsetof(RoSet, rs_elem) +
	                      ((self->rs_mask + 1) *
	                       sizeof(struct roset_item)));
}

PRIVATE struct type_method tpconst roset_methods[] = {
	/* TODO: HashSet.Frozen.byhash(template:?O)->?DSequence */
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst roset_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_F("__sizeof__", &roset_sizeof, TYPE_GETSET_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst roset_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RoSet, rs_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RoSet, rs_size)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst roset_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RoSetIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoSet_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED DREF RoSet *DCALL roset_ctor(void) {
	DREF RoSet *result;
	result = ROSET_ALLOC(1);
	if unlikely(!result)
		goto done;
	result->rs_mask = 1;
	result->rs_size = 0;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF RoSet *DCALL
roset_deepcopy(RoSet *__restrict self) {
	DREF RoSet *result;
	size_t i;
	int temp;
	result = (DREF RoSet *)DeeRoSet_NewWithHint(self->rs_size);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= self->rs_mask; ++i) {
		DREF DeeObject *key_copy;

		/* Deep-copy the key & value */
		if (!self->rs_elem[i].rsi_key)
			continue;
		key_copy = DeeObject_DeepCopy(self->rs_elem[i].rsi_key);
		if unlikely(!key_copy)
			goto err;

		/* Insert the copied key & value into the new set. */
		temp = DeeRoSet_Insert(&result, key_copy);
		Dee_Decref(key_copy);
		if unlikely(temp)
			goto err;
	}
done:
	return result;
err:
	Dee_Clear(result);
	goto done;
}

PRIVATE WUNUSED DREF RoSet *DCALL
roset_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_RoSet", &seq))
		goto err;
	return (DREF RoSet *)DeeRoSet_FromSequence(seq);
err:
	return NULL;
}



/* The main `_RoSet' container class. */
PUBLIC DeeTypeObject DeeRoSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSet",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&roset_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&roset_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&roset_init,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&roset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&roset_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&roset_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&roset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &roset_cmp,
	/* .tp_seq           = */ &roset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ roset_methods,
	/* .tp_getsets       = */ roset_getsets,
	/* .tp_members       = */ roset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ roset_class_members
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ROSET_C */
