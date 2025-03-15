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
#ifndef GUARD_DEEMON_OBJECTS_ROSET_C
#define GUARD_DEEMON_OBJECTS_ROSET_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/system-features.h> /* memcpy */
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>
/**/

#include "../runtime/strings.h"
#include "generic-proxy.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

typedef DeeRoSetObject RoSet;

typedef struct {
	PROXY_OBJECT_HEAD_EX(RoSet, rosi_set)  /* [1..1][const] The set being iterated. */
	struct roset_item          *rosi_next; /* [?..1][in(rosi_set->rs_elem)][atomic]
	                                        * The first candidate for the next item. */
} RoSetIterator;
#define READ_ITEM(x) atomic_read(&(x)->rosi_next)

INTDEF DeeTypeObject RoSetIterator_Type;

PRIVATE WUNUSED NONNULL((1)) int DCALL
rosetiterator_ctor(RoSetIterator *__restrict self) {
	self->rosi_set = DeeRoSet_New();
	if unlikely(!self->rosi_set)
		goto err;
	self->rosi_next = self->rosi_set->rs_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rosetiterator_init(RoSetIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	RoSet *set;
	_DeeArg_Unpack1(err, argc, argv, "_RoSetIterator", &set);
	if (DeeObject_AssertTypeExact(set, &DeeRoSet_Type))
		goto err;
	self->rosi_set = set;
	Dee_Incref(set);
	self->rosi_next = set->rs_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rosetiterator_copy(RoSetIterator *__restrict self,
                   RoSetIterator *__restrict other) {
	self->rosi_set = other->rosi_set;
	Dee_Incref(self->rosi_set);
	self->rosi_next = READ_ITEM(other);
	return 0;
}

STATIC_ASSERT(offsetof(RoSetIterator, rosi_set) == offsetof(ProxyObject, po_obj));
#define rosetiterator_fini  generic_proxy__fini
#define rosetiterator_visit generic_proxy__visit

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

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
rosetiterator_hash(RoSetIterator *self) {
	return Dee_HashPointer(READ_ITEM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rosetiterator_compare(RoSetIterator *self, RoSetIterator *other) {
	if (DeeObject_AssertType(other, &RoSetIterator_Type))
		goto err;
	Dee_return_compareT(struct roset_item *, READ_ITEM(self),
	                    /*                */ READ_ITEM(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp rosetiterator_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&rosetiterator_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rosetiterator_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE struct type_member tpconst roset_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RoSetIterator, rosi_set), "->?Ert:RoSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RoSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSetIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(set:?Ert:RoSet)"),
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
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rosetiterator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rosetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &rosetiterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rosetiterator_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ roset_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__E31EBEB26CC72F83),
};





#define ROSET_ALLOC(mask)  ((DREF RoSet *)DeeObject_Calloc(SIZEOF_ROSET(mask)))
#define SIZEOF_ROSET(mask) _Dee_MallococBufsize(offsetof(RoSet, rs_elem), (mask) + 1, sizeof(struct roset_item))
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
		error = DeeObject_TryCompareEq(key, item->rsi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
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
#define DeeRoSet_InsertSequence_foreach_PTR ((Dee_foreach_t)&DeeRoSet_Insert)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
#define DeeRoSet_InsertSequence_foreach_PTR &DeeRoSet_InsertSequence_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeRoSet_InsertSequence_foreach(void *arg, DeeObject *elem) {
	return DeeRoSet_Insert((RoSet **)arg, elem);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromSequence(DeeObject *__restrict sequence) {
	DREF RoSet *result;
	size_t size_hint, mask;

	/* Optimization: Since rosets are immutable, re-return if the
	 *               given sequence already is a read-only RoSet. */
	if (DeeRoSet_CheckExact(sequence))
		return_reference_(sequence);

	/* TODO: Optimization: if (DeeSet_CheckExact(sequence)) ...
	 * (fix the set's hash-vector to not contain dummies,
	 * then copy as-is) */

	/* Construct a read-only RoSet from a generic sequence. */
	mask      = ROSET_INITIAL_MASK;
	size_hint = DeeObject_SizeFast(sequence);
	if (size_hint != (size_t)-1) {
		while (mask <= size_hint)
			mask = (mask << 1) | 1;
		mask = (mask << 1) | 1;
	}
	result = ROSET_ALLOC(mask);
	if likely(result) {
		/*result->rd_size = 0;*/
		result->rs_mask = mask;
		DeeObject_Init(result, &DeeRoSet_Type);
		if unlikely(DeeObject_Foreach(sequence, DeeRoSet_InsertSequence_foreach_PTR, &result))
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
roset_size(RoSet *__restrict self) {
	return self->rs_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
roset_contains(RoSet *self, DeeObject *key) {
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
		error = DeeObject_TryCompareEq(key, item->rsi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
roset_foreach(RoSet *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		DeeObject *key = self->rs_elem[i].rsi_key;
		if (!key)
			continue;
		temp = (*proc)(arg, key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
roset_asvector_nothrow(RoSet *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if likely(dst_length >= self->rs_size) {
		struct roset_item *iter, *end;
		end = (iter = self->rs_elem) + (self->rs_mask + 1);
		for (; iter < end; ++iter) {
			DeeObject *key = iter->rsi_key;
			if (key == NULL)
				continue;
			Dee_Incref(key);
			*dst++ = key;
		}
	}
	return self->rs_size;
}

PRIVATE struct type_seq roset_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&roset_contains,
	/* .tp_getitem                    = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&roset_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem                    = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&roset_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&roset_size,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&roset_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&roset_asvector_nothrow,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_sizeof(RoSet *self) {
	size_t result;
	result = _Dee_MallococBufsize(offsetof(RoSet, rs_elem),
	                              self->rs_mask + 1,
	                              sizeof(struct roset_item));
	return DeeInt_NewSize(result);
}

PRIVATE struct type_method tpconst roset_methods[] = {
	/* TODO: HashSet.Frozen.byhash(template:?O)->?DSequence */
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst roset_getsets[] = {
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &roset_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
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
	_DeeArg_Unpack1(err, argc, argv, "_RoSet", &seq);
	return (DREF RoSet *)DeeRoSet_FromSequence(seq);
err:
	return NULL;
}



PRIVATE struct type_operator const roset_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
};

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
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&roset_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&roset_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&roset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__F6E3D7B2219AE1EB),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__A5C53AFDF1233C5A), /* TODO: &roset_cmp */
	/* .tp_seq           = */ &roset_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ roset_methods,
	/* .tp_getsets       = */ roset_getsets,
	/* .tp_members       = */ roset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ roset_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ roset_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(roset_operators),
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ROSET_C */
