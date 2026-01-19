/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_COLLECTIONS_USET_C
#define GUARD_DEX_COLLECTIONS_USET_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dict.h> /* DeeDict_Dummy */
#include <deemon/error-rt.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/util/atomic.h>

#include <hybrid/sched/yield.h>

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

/* Dummy key object. */
#define dummy (&DeeDict_Dummy)

PRIVATE /*WUNUSED*/ NONNULL((1)) int DCALL USet_InitEmpty(USet *__restrict self);
PRIVATE NONNULL((1)) void DCALL USet_Fini(USet *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_InitCopy(USet *__restrict self, USet *__restrict other);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_InitSequence(USet *__restrict self, DeeObject *__restrict sequence);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL uset_mh_insert(USet *self, DeeObject *ob);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL uset_mh_remove(USet *self, DeeObject *ob);
LOCAL NONNULL((1, 2)) void DCALL USet_DoInsertTrackedUnlocked(USet *self, DREF DeeObject *ob);
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL USet_Contains(USet *self, DeeObject *ob);


INTDEF DeeTypeObject USet_Type;
INTDEF DeeTypeObject USetIterator_Type;
INTDEF DeeTypeObject URoSet_Type;
INTDEF DeeTypeObject URoSetIterator_Type;

#define READ_ITEM(x) atomic_read(&(x)->usi_next)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usetiterator_next(USetIterator *__restrict self) {
	DREF DeeObject *result;
	struct uset_item *item, *end;
	USet *set = self->usi_set;
	USet_LockRead(set);
	end = set->us_elem + (set->us_mask + 1);
	for (;;) {
		struct uset_item *old_item;
		item     = atomic_read(&self->usi_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto set_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < set->us_elem)
			goto set_has_changed;

		/* Search for the next non-empty item. */
		while (item < end && (!item->usi_key || item->usi_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->usi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->usi_next, old_item, item + 1))
			break;
	}
	result = item->usi_key;
	Dee_Incref(result);
	USet_LockEndRead(set);
	return result;
set_has_changed:
	USet_LockEndRead(set);
	err_changed_sequence((DeeObject *)set);
	return NULL;
iter_exhausted:
	USet_LockEndRead(set);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
usetiterator_ctor(USetIterator *__restrict self) {
	self->usi_set = (DREF USet *)DeeObject_NewDefault(&USet_Type);
	if unlikely(!self->usi_set)
		goto err;
	self->usi_next = self->usi_set->us_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usetiterator_copy(USetIterator *__restrict self,
                  USetIterator *__restrict other) {
	self->usi_set = other->usi_set;
	Dee_Incref(self->usi_set);
	self->usi_next = READ_ITEM(other);
	return 0;
}


INTERN NONNULL((1)) void DCALL
usetiterator_fini(USetIterator *__restrict self) {
	Dee_Decref(self->usi_set);
}

INTERN NONNULL((1, 2)) void DCALL
usetiterator_visit(USetIterator *__restrict self,
                   Dee_visit_t proc, void *arg) {
	Dee_Visit(self->usi_set);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
usetiterator_init(USetIterator *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	USet *set;
	DeeArg_Unpack1(err, argc, argv, "_UniqueSetIterator", &set);
	if (DeeObject_AssertType(set, &USet_Type))
		goto err;
	self->usi_set = set;
	Dee_Incref(set);
	self->usi_next = set->us_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
usetiterator_bool(USetIterator *__restrict self) {
	struct uset_item *item = READ_ITEM(self);
	USet *set              = self->usi_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item >= set->us_elem &&
	        item < set->us_elem + (set->us_mask + 1));
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
usetiterator_hash(USetIterator *self) {
	return Dee_HashPointer(READ_ITEM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
usetiterator_compare(USetIterator *self, USetIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(struct uset_item *, READ_ITEM(self),
	                    /*               */ READ_ITEM(other));
err:
	return Dee_COMPARE_ERR;
}

INTERN struct type_cmp usetiterator_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&usetiterator_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&usetiterator_compare,
};

PRIVATE struct type_member tpconst usetiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(USetIterator, usi_set), "->?GUniqueSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject USetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueSetIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ USetIterator,
			/* tp_ctor:        */ &usetiterator_ctor,
			/* tp_copy_ctor:   */ &usetiterator_copy,
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ &usetiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&usetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&usetiterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&usetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &usetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&usetiterator_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ usetiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE /*WUNUSED*/ NONNULL((1)) int DCALL
USet_InitEmpty(USet *__restrict self) {
	self->us_mask = 0;
	self->us_size = 0;
	self->us_used = 0;
	self->us_elem = empty_set_items;
	Dee_atomic_rwlock_init(&self->us_lock);
	weakref_support_init(self);
	return 0;
}

PRIVATE /*WUNUSED*/ NONNULL((1)) int DCALL
USet_InitWithHint(USet *__restrict self, size_t size_hint) {
	size_t min_mask, mask;
	if unlikely(size_hint == 0)
		return USet_InitEmpty(self);
	min_mask = 16 - 1;
	/* Figure out how large the mask of the set is going to be. */
	while ((size_hint & min_mask) != size_hint)
		min_mask = (min_mask << 1) | 1;
	/* Prefer using a mask of one greater level to improve performance. */
	mask          = (min_mask << 1) | 1;
	self->us_elem = (struct uset_item *)Dee_TryCallocc(mask + 1, sizeof(struct uset_item));
	if unlikely(!self->us_elem) {
		/* Try one level less if that failed. */
		mask = min_mask;
		self->us_elem = (struct uset_item *)Dee_TryCallocc(mask + 1, sizeof(struct uset_item));
		if unlikely(!self->us_elem)
			return USet_InitEmpty(self);
	}
	self->us_mask = mask;
	self->us_used = 0;
	self->us_size = 0;
	Dee_atomic_rwlock_init(&self->us_lock);
	weakref_support_init(self);
	return 0;
}

LOCAL NONNULL((1, 2)) void DCALL
USet_DoInsertTrackedUnlocked(USet *self, DREF DeeObject *ob) {
	Dee_hash_t i, perturb;
	perturb = i = UHASH(ob) & self->us_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->us_elem[i & self->us_mask];
		if (item->usi_key) { /* Already in use */
			if likely(!USAME(item->usi_key, ob))
				continue;
			--self->us_size;
			--self->us_used;
			Dee_Decref_unlikely(ob);
			return;
		}
		item->usi_key = ob; /* Inherit reference. */
		break;
	}
}

/* Resize the hash size by a factor of 2 and re-insert all elements.
 * When `sizedir > 0', increase the hash; When `sizedir < 0', decrease it.
 * During this process all dummy items are discarded.
 * @return: true:  Successfully rehashed the set.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
uset_rehash(USet *__restrict self, int sizedir) {
	struct uset_item *new_vector, *iter, *end;
	size_t new_mask = self->us_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!self->us_used) {
			ASSERT(!self->us_used);
			/* Special case: delete the vector. */
			if (self->us_size) {
				ASSERT(self->us_elem != empty_set_items);
				/* Must discard dummy items. */
				end = (iter = self->us_elem) + (self->us_mask + 1);
				for (; iter < end; ++iter) {
					ASSERT(iter->usi_key == NULL ||
					       iter->usi_key == dummy);
					if (iter->usi_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->us_elem != empty_set_items)
				Dee_Free(self->us_elem);
			self->us_elem = empty_set_items;
			self->us_mask = 0;
			self->us_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (self->us_used >= new_mask)
			return true;
	}
	ASSERT(self->us_used < new_mask);
	ASSERT(self->us_used <= self->us_size);
	new_vector = (struct uset_item *)Dee_TryCallocc(new_mask + 1,
	                                                sizeof(struct uset_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->us_elem == empty_set_items) == (self->us_mask == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_used == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_size == 0));
	if (self->us_elem != empty_set_items) {
		/* Re-insert all existing items into the new set vector. */
		end = (iter = self->us_elem) + (self->us_mask + 1);
		for (; iter < end; ++iter) {
			struct uset_item *item;
			Dee_hash_t i, perturb;
			/* Skip dummy and NULL keys. */
			if (iter->usi_key == NULL || iter->usi_key == dummy)
				continue;
			perturb = i = UHASH(iter->usi_key) & new_mask;
			for (;; USet_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->usi_key)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			item->usi_key = iter->usi_key;
		}
		Dee_Free(self->us_elem);
		/* With all dummy items gone, the size now equals what is actually used. */
		self->us_size = self->us_used;
	}
	ASSERT(self->us_size == self->us_used);
	self->us_mask = new_mask;
	self->us_elem = new_vector;
	return true;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uset_mh_remove(USet *self, DeeObject *ob) {
	size_t mask;
	struct uset_item *vector;
	Dee_hash_t i, perturb;
	USet_LockRead(self);
restart:
	vector  = self->us_elem;
	mask    = self->us_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct uset_item *item = &vector[i & mask];
		item_key               = item->usi_key;
		if (!item_key)
			break; /* Not found */
		if (!USAME(item_key, ob))
			continue;
		/* Found it! */
		if (!USet_LockUpgrade(self)) {
			/* Check if the set was modified. */
			if (self->us_elem != vector ||
			    self->us_mask != mask ||
			    item->usi_key != item_key) {
				USet_LockDowngrade(self);
				goto restart;
			}
		}
		item->usi_key = dummy;
		Dee_Incref(dummy);
		ASSERT(self->us_used);
		if (--self->us_used < self->us_size / 2)
			uset_rehash(self, -1);
		USet_LockEndWrite(self);
		Dee_Decref(item_key);
		return 1;
	}
	USet_LockEndRead(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
USet_Contains(USet *self, DeeObject *ob) {
	size_t mask;
	Dee_hash_t i, perturb;
	USet_LockRead(self);
	mask    = self->us_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->us_elem[i & mask];
		if (!item->usi_key)
			break; /* Not found */
		if (!USAME(item->usi_key, ob))
			continue;
		/* Found it! */
		USet_LockEndRead(self);
		return 1;
	}
	USet_LockEndRead(self);
	return 0;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uset_mh_insert(USet *self, DeeObject *ob) {
	struct uset_item *first_dummy;
	size_t mask;
	Dee_hash_t i, perturb;
again_lock:
	USet_LockRead(self);
again:
	first_dummy = NULL;
	mask        = self->us_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->us_elem[i & mask];
		if (!item->usi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->usi_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (USAME(item->usi_key, ob)) { /* Same object */
			USet_LockEndRead(self);
			return 0; /* Already exists. */
		}
	}
#ifndef CONFIG_NO_THREADS
	if (!USet_LockUpgrade(self)) {
		USet_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && self->us_size + 1 < self->us_mask) {
		ASSERT(first_dummy != empty_set_items);
		ASSERT(!first_dummy->usi_key ||
		       first_dummy->usi_key == dummy);
		if (first_dummy->usi_key)
			Dee_DecrefNokill(first_dummy->usi_key);
		/* Fill in the target slot. */
		first_dummy->usi_key = ob;
		Dee_Incref(ob);
		++self->us_used;
		++self->us_size;
		/* Try to keep the set vector big at least twice as big as the element count. */
		if (self->us_size * 2 > self->us_mask)
			uset_rehash(self, 1);
		USet_LockEndWrite(self);
		return 1; /* New item. */
	}
	/* Rehash the set and try again. */
	if (uset_rehash(self, 1)) {
		USet_LockDowngrade(self);
		goto again;
	}
	USet_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
uset_mh_unify(USet *self, DeeObject *ob) {
	int status = uset_mh_insert(self, ob);
	if unlikely(status < 0)
		goto err;
	return_reference_(ob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
USet_DoInsertNolock(USet *self, DeeObject *ob) {
	struct uset_item *first_dummy;
	size_t mask;
	Dee_hash_t i, perturb;
again:
	first_dummy = NULL;
	mask        = self->us_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->us_elem[i & mask];
		if (!item->usi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->usi_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (USAME(item->usi_key, ob)) /* Same object */
			return 0;                /* Already exists. */
	}
	if (first_dummy && self->us_size + 1 < self->us_mask) {
		ASSERT(first_dummy != empty_set_items);
		ASSERT(!first_dummy->usi_key ||
		       first_dummy->usi_key == dummy);
		if (first_dummy->usi_key)
			Dee_DecrefNokill(first_dummy->usi_key);
		/* Fill in the target slot. */
		first_dummy->usi_key = ob;
		Dee_Incref(ob);
		++self->us_used;
		++self->us_size;
		/* Try to keep the set vector big at least twice as big as the element count. */
		if (self->us_size * 2 > self->us_mask)
			uset_rehash(self, 1);
		return 0; /* New item. */
	}
	/* Rehash the set and try again. */
	if (uset_rehash(self, 1))
		goto again;
	if (Dee_CollectMemory(1))
		goto again;
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
USet_InitSequence(USet *__restrict self,
                  DeeObject *__restrict sequence) {
	DeeTypeObject *type = Dee_TYPE(sequence);
	if (type == &USet_Type)
		return USet_InitCopy(self, (USet *)sequence);
	if (type == &DeeHashSet_Type) {
		DeeHashSetObject *src;
		src = (DeeHashSetObject *)sequence;
		Dee_atomic_rwlock_init(&self->us_lock);
again_hashset:
		DeeHashSet_LockRead(src);
		self->us_used = self->us_size = src->hs_used;
		if (!self->us_used) {
			self->us_mask = 0;
			self->us_elem = empty_set_items;
		} else {
			size_t i;
			self->us_mask = src->hs_mask;
			self->us_elem = (struct uset_item *)Dee_TryCallocc(src->hs_mask + 1,
			                                                   sizeof(struct uset_item));
			if unlikely(!self->us_elem) {
				DeeHashSet_LockEndRead(src);
				if (Dee_CollectMemoryc(self->us_mask + 1, sizeof(struct uset_item)))
					goto again_hashset;
				return -1;
			}
			for (i = 0; i <= src->hs_mask; ++i) {
				DeeObject *key = src->hs_elem[i].hsi_key;
				if (!key || key == dummy)
					continue;
				Dee_Incref(key);
				USet_DoInsertTrackedUnlocked(self, key);
			}
		}
		DeeHashSet_LockEndRead(src);
		weakref_support_init(self);
	} else if (type == &URoSet_Type) {
		URoSet *src = (URoSet *)sequence;
		Dee_atomic_rwlock_init(&self->us_lock);
		self->us_used = self->us_size = src->urs_size;
		if unlikely(!self->us_size) {
			self->us_mask = 0;
			self->us_elem = (struct uset_item *)empty_set_items;
		} else {
			self->us_mask = src->urs_mask;
			self->us_elem = (struct uset_item *)Dee_Mallocc(src->urs_mask + 1,
			                                               sizeof(struct uset_item));
			if unlikely(!self->us_elem)
				goto err;
			Dee_XMovrefv((DeeObject **)self->us_elem,
			             (DeeObject **)src->urs_elem,
			             src->urs_mask + 1);
		}
		weakref_support_init(self);
	} else if (type == &DeeRoSet_Type) {
		size_t i;
		DeeRoSetObject *src;
		src = (DeeRoSetObject *)sequence;
		Dee_atomic_rwlock_init(&self->us_lock);
		self->us_used = self->us_size = src->rs_size;
		if unlikely(!self->us_size) {
			self->us_mask = 0;
			self->us_elem = (struct uset_item *)empty_set_items;
		} else {
			self->us_mask = src->rs_mask;
			self->us_elem = (struct uset_item *)Dee_Callocc(src->rs_mask + 1,
			                                                sizeof(struct uset_item));
			if unlikely(!self->us_elem)
				goto err;
			for (i = 0; i <= src->rs_mask; ++i) {
				DeeObject *key = src->rs_elem[i].rsi_key;
				if (!key)
					continue;
				Dee_Incref(key);
				USet_DoInsertTrackedUnlocked(self, key);
			}
		}
		weakref_support_init(self);
	} else {
		size_t sizehint = DeeObject_SizeFast(sequence);
		if (sizehint != (size_t)-1) {
			USet_InitWithHint(self, sizehint);
		} else {
			USet_InitEmpty(self);
		}
		if unlikely(DeeObject_Foreach(sequence, (Dee_foreach_t)&USet_DoInsertNolock, self))
			goto err_self;
		/* Free unused memory */
		while (self->us_used < (self->us_mask >> 2))
			uset_rehash(self, -1);
	}
	return 0;
	{
		size_t i;
err_self:
		for (i = 0; i <= self->us_mask; ++i)
			Dee_XDecref(self->us_elem[i].usi_key);
	}
	Dee_Free(self->us_elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF USet *DCALL
USet_FromSequence(DeeObject *__restrict sequence) {
	DREF USet *result;
	result = DeeGCObject_MALLOC(USet);
	if unlikely(!result)
		goto err;
	if unlikely(USet_InitSequence(result, sequence))
		goto err_r;
	DeeObject_Init(result, &USet_Type);
	return DeeGC_TRACK(USet, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}









PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_sizeof(USet *self) {
	return DeeInt_NewSize(sizeof(USet) +
	                      ((self->us_mask + 1) *
	                       sizeof(struct uset_item)));
}

PRIVATE struct type_getset tpconst uset_getsets[] = {
	TYPE_GETTER_AB_F("frozen", &URoSet_FromUSet, METHOD_FNOREFESCAPE,
	                 "->?AFrozen?.\n"
	                 "Returns a read-only (frozen) copy of @this set"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &uset_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst uset_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(USet, us_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(USet, us_size)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst uset_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &USetIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &URoSet_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
USet_InitCopy(USet *__restrict self,
              USet *__restrict other) {
	struct uset_item *iter, *end;
	Dee_atomic_rwlock_init(&self->us_lock);
again:
	USet_LockRead(other);
	self->us_mask = other->us_mask;
	self->us_size = other->us_size;
	self->us_used = other->us_used;
	if ((self->us_elem = other->us_elem) != empty_set_items) {
		self->us_elem = (struct uset_item *)Dee_TryMallocc(other->us_mask + 1,
		                                                  sizeof(struct uset_item));
		if unlikely(!self->us_elem) {
			USet_LockEndRead(other);
			if (Dee_CollectMemoryc(self->us_mask + 1, sizeof(struct uset_item)))
				goto again;
			return -1;
		}
		iter = (struct uset_item *)memcpyc(self->us_elem, other->us_elem,
		                                   self->us_mask + 1,
		                                   sizeof(struct uset_item));
		end  = iter + (self->us_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->usi_key)
				continue;
			Dee_Incref(iter->usi_key);
		}
	}
	USet_LockEndRead(other);
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uset_deepload(USet *__restrict self) {
	DREF DeeObject **new_items, **items = NULL;
	size_t i, hash_i, item_count, ols_item_count = 0;
	struct uset_item *new_map, *ols_map;
	size_t new_mask;
	for (;;) {
		USet_LockRead(self);
		/* Optimization: if the Set is empty, then there's nothing to copy! */
		if (self->us_elem == empty_set_items) {
			USet_LockEndRead(self);
			return 0;
		}
		item_count = self->us_used;
		if (item_count <= ols_item_count)
			break;
		USet_LockEndRead(self);
		new_items = (DREF DeeObject **)Dee_Reallocc(items, item_count,
		                                            sizeof(DREF DeeObject *));
		if unlikely(!new_items)
			goto err_items;
		ols_item_count = item_count;
		items          = new_items;
	}

	/* Copy all used items. */
	for (i = 0, hash_i = 0; i < item_count; ++hash_i) {
		ASSERT(hash_i <= self->us_mask);
		if (self->us_elem[hash_i].usi_key == NULL)
			continue;
		if (self->us_elem[hash_i].usi_key == dummy)
			continue;
		items[i] = self->us_elem[hash_i].usi_key;
		Dee_Incref(items[i]);
		++i;
	}
	USet_LockEndRead(self);

	/* With our own local copy of all items being
	 * used, replace all of them with deep copies. */
	for (i = 0; i < item_count; ++i) {
		if (DeeObject_InplaceDeepCopy(&items[i]))
			goto err_items_v;
	}
	new_mask = 1;
	while ((item_count & new_mask) != item_count)
		new_mask = (new_mask << 1) | 1;
	new_map = (struct uset_item *)Dee_Callocc(new_mask + 1, sizeof(struct uset_item));
	if unlikely(!new_map)
		goto err_items_v;
	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		Dee_hash_t j, perturb;
		perturb = j = UHASH(items[i]) & new_mask;
		for (;; USet_HashNx(j, perturb)) {
			struct uset_item *item = &new_map[j & new_mask];
			if (item->usi_key) {
				if likely(!USAME(item->usi_key, items[i]))
					continue; /* Already in use */
				Dee_Decref(items[i]);
				goto next_item;
			}
			item->usi_key = items[i]; /* Inherit reference. */
			break;
		}
next_item:
		;
	}
	USet_LockWrite(self);
	i            = self->us_mask + 1;
	self->us_mask = new_mask;
	self->us_used = item_count;
	self->us_size = item_count;
	ols_map      = self->us_elem;
	self->us_elem = new_map;
	USet_LockEndWrite(self);
	if (ols_map != empty_set_items) {
		while (i--)
			Dee_XDecref(ols_map[i].usi_key);
		Dee_Free(ols_map);
	}
	Dee_Free(items);
	return 0;
err_items_v:
	Dee_Decrefv(items, item_count);
err_items:
	Dee_Free(items);
	return -1;
}

PRIVATE NONNULL((1)) void DCALL USet_Fini(USet *__restrict self) {
	weakref_support_fini(self);
	ASSERT((self->us_elem == empty_set_items) == (self->us_mask == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_size == 0));
	ASSERT(self->us_used <= self->us_size);
	if (self->us_elem != empty_set_items) {
		size_t i;
		for (i = 0; i <= self->us_mask; ++i)
			Dee_XDecref(self->us_elem[i].usi_key);
		Dee_Free(self->us_elem);
	}
}

PRIVATE NONNULL((1)) void DCALL uset_clear(USet *__restrict self) {
	struct uset_item *elem;
	size_t mask;

	/* Extract the vector and mask. */
	USet_LockWrite(self);
	ASSERT((self->us_elem == empty_set_items) == (self->us_mask == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_used == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_size == 0));
	ASSERT(self->us_used <= self->us_size);
	elem         = self->us_elem;
	mask         = self->us_mask;
	self->us_elem = empty_set_items;
	self->us_mask = 0;
	self->us_used = 0;
	self->us_size = 0;
	USet_LockEndWrite(self);

	/* Destroy the vector. */
	if (elem != empty_set_items) {
		size_t i;
		for (i = 0; i <= mask; ++i) {
			DeeObject *key = elem[i].usi_key;
			Dee_XDecref(key);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
uset_visit(USet *__restrict self, Dee_visit_t proc, void *arg) {
	USet_LockRead(self);
	ASSERT((self->us_elem == empty_set_items) == (self->us_mask == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_used == 0));
	ASSERT((self->us_elem == empty_set_items) == (self->us_size == 0));
	ASSERT(self->us_used <= self->us_size);
	if (self->us_elem != empty_set_items) {
		size_t i;
		for (i = 0; i <= self->us_mask; ++i) {
			DeeObject *key = self->us_elem[i].usi_key;
			/* Visit all keys and associated values. */
			Dee_XVisit(key);
		}
	}
	USet_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF USetIterator *DCALL
uset_iter(USet *__restrict self) {
	DREF USetIterator *result;
	result = DeeObject_MALLOC(USetIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &USetIterator_Type);
	result->usi_set = self;
	Dee_Incref(self);
	result->usi_next = atomic_read(&self->us_elem);
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
uset_size(USet *__restrict self) {
	return atomic_read(&self->us_used);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
uset_contains(USet *self, DeeObject *search_item) {
	return_bool(USet_Contains(self, search_item));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_repr(USet *__restrict self) {
	struct unicode_printer p;
	Dee_ssize_t error;
	struct uset_item *iter, *end;
	bool is_first;
	struct uset_item *vector;
	size_t mask;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "collections.UniqueSet({ ") < 0)
		goto err;
	USet_LockRead(self);
	is_first = true;
	vector   = self->us_elem;
	mask     = self->us_mask;
	end      = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key;
		if (iter->usi_key == NULL ||
		    iter->usi_key == dummy)
			continue;
		key = iter->usi_key;
		Dee_Incref(key);
		USet_LockEndRead(self);
		/* Print this item. */
		error = unicode_printer_printf(&p, "%s%r", is_first ? "" : ", ", key);
		Dee_Decref(key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
		USet_LockRead(self);
		if (self->us_elem != vector ||
		    self->us_mask != mask)
			goto restart;
	}
	USet_LockEndRead(self);
	if ((is_first ? UNICODE_PRINTER_PRINT(&p, "})")
	              : UNICODE_PRINTER_PRINT(&p, " })")) < 0)
		goto err;
	return unicode_printer_pack(&p);
restart:
	USet_LockEndRead(self);
	unicode_printer_fini(&p);
	goto again;
err:
	unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
uset_printrepr(USet *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	struct uset_item *iter, *end;
	struct uset_item *vector;
	size_t mask;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "collections.UniqueSet({ ");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	USet_LockRead(self);
	vector = self->us_elem;
	mask   = self->us_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key;
		key = iter->usi_key;
		if (key == NULL || key == dummy)
			continue;
		Dee_Incref(key);
		USet_LockEndRead(self);
		/* Print this item. */
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0) {
				Dee_Decref(key);
				goto err;
			}
			result += temp;
		}
		temp = DeeObject_PrintRepr(key, printer, arg);
		Dee_Decref(key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		is_first = false;
		USet_LockRead(self);
		if unlikely(self->us_elem != vector ||
		            self->us_mask != mask) {
			USet_LockEndRead(self);
			temp = DeeFormat_PRINT(printer, arg, ", <UniqueSet changed while being iterated>");
			if (temp < 0)
				goto err;
			result += temp;
			goto stop_after_changed;
		}
	}
	USet_LockEndRead(self);
stop_after_changed:
	temp = is_first ? DeeFormat_PRINT(printer, arg, "})")
	                : DeeFormat_PRINT(printer, arg, " })");
	if (temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

INTERN WUNUSED NONNULL((1)) int DCALL
uset_bool(USet *__restrict self) {
	return atomic_read(&self->us_used) != 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uset_init(USet *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "UniqueSet", &seq);
	return USet_InitSequence(self, seq);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_mh_pop(USet *__restrict self) {
	size_t i;
	DREF DeeObject *result;
	USet_LockWrite(self);
	for (i = 0; i <= self->us_mask; ++i) {
		struct uset_item *item = &self->us_elem[i];
		if ((result = item->usi_key) == NULL)
			continue; /* Unused slot. */
		if (result == dummy)
			continue; /* Deleted slot. */
		item->usi_key = dummy;
		Dee_Incref(dummy);
		ASSERT(self->us_used);
		if (--self->us_used < self->us_size / 2)
			uset_rehash(self, -1);
		USet_LockEndWrite(self);
		return result;
	}
	USet_LockEndWrite(self);
	/* Set is already empty. */
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_mh_pop_with_default(USet *self, DeeObject *def) {
	size_t i;
	DREF DeeObject *result;
	USet_LockWrite(self);
	for (i = 0; i <= self->us_mask; ++i) {
		struct uset_item *item = &self->us_elem[i];
		if ((result = item->usi_key) == NULL)
			continue; /* Unused slot. */
		if (result == dummy)
			continue; /* Deleted slot. */
		item->usi_key = dummy;
		Dee_Incref(dummy);
		ASSERT(self->us_used);
		if (--self->us_used < self->us_size / 2)
			uset_rehash(self, -1);
		USet_LockEndWrite(self);
		return result;
	}
	USet_LockEndWrite(self);
	/* Set is already empty. */
	return_reference_(def);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uset_mh_clear(USet *__restrict self) {
	uset_clear(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
uset_foreach(USet *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	USet_LockRead(self);
	for (i = 0; i <= self->us_mask; ++i) {
		DeeObject *key = self->us_elem[i].usi_key;
		if (!key || key == dummy)
			continue;
		Dee_Incref(key);
		USet_LockEndRead(self);
		temp = (*proc)(arg, key);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		USet_LockRead(self);
	}
	USet_LockEndRead(self);
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
uset_asvector_nothrow(USet *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t result;
	USet_LockRead(self);
	result = self->us_used;
	if likely(dst_length >= result) {
		struct uset_item *iter, *end;
		end = (iter = self->us_elem) + (self->us_mask + 1);
		for (; iter < end; ++iter) {
			DeeObject *key = iter->usi_key;
			if (key == NULL || key == dummy)
				continue;
			Dee_Incref(key);
			*dst++ = key;
		}
	}
	USet_LockEndRead(self);
	return result;
}


PRIVATE struct type_seq uset_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uset_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&uset_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&uset_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&uset_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&uset_size,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&uset_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&uset_asvector_nothrow,
};

PRIVATE struct type_method tpconst uset_methods[] = {
	TYPE_METHOD_HINTREF(Set_pop),
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_HINTREF(Set_unify),
	TYPE_METHOD_HINTREF(Set_insert),
	TYPE_METHOD_HINTREF(Set_remove),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst uset_method_hints[] = {
	TYPE_METHOD_HINT_F(set_pop, &uset_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop_with_default, &uset_mh_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_clear, &uset_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_insert, &uset_mh_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_remove, &uset_mh_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_unify, &uset_mh_unify, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_gc tpconst uset_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&uset_clear
};


INTERN DeeTypeObject USet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "UniqueSet",
	/* .tp_doc      = */ DOC("A mutable set-like container that uses @?Aid?O "
	                         "and ${x === y} to detect/prevent duplicates\n"
	                         "\n"

	                         "()\n"
	                         "Create an empty set\n"
	                         "\n"

	                         "(items:?S?O)\n"
	                         "Create a new set populated with elements from @items\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a shallow copy of @this set\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Returns a deep copy of @this set\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this set is non-empty\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @item is apart of @this set\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of items apart of @this set\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating all items "
	                         /**/ "in @this set, following an undefined order"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(USet),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ USet,
			/* tp_ctor:        */ &USet_InitEmpty,
			/* tp_copy_ctor:   */ &USet_InitCopy,
			/* tp_deep_ctor:   */ &USet_InitCopy,
			/* tp_any_ctor:    */ &uset_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&USet_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&uset_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uset_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&uset_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&uset_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&uset_visit,
	/* .tp_gc            = */ &uset_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &uset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ uset_methods,
	/* .tp_getsets       = */ uset_getsets,
	/* .tp_members       = */ uset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ uset_class_members,
	/* .tp_method_hints  = */ uset_method_hints,
};










#undef READ_ITEM
#define READ_ITEM(x) atomic_read(&(x)->ursi_next)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urosetiterator_next(URoSetIterator *__restrict self) {
	struct uset_item *item, *end;
	end = self->ursi_set->urs_elem + self->ursi_set->urs_mask + 1;
	for (;;) {
		struct uset_item *old_item;
		item     = atomic_read(&self->ursi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->usi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->ursi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->ursi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->usi_key);
iter_exhausted:
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) int DCALL
urosetiterator_ctor(URoSetIterator *__restrict self) {
	self->ursi_set = URoSet_New();
	if unlikely(!self->ursi_set)
		goto err;
	self->ursi_next = self->ursi_set->urs_elem;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(USetIterator, usi_set) == offsetof(URoSetIterator, ursi_set));
STATIC_ASSERT(offsetof(USetIterator, usi_next) == offsetof(URoSetIterator, ursi_next));
#define urosetiterator_copy  usetiterator_copy
#define urosetiterator_fini  usetiterator_fini
#define urosetiterator_visit usetiterator_visit
#define urosetiterator_cmp   usetiterator_cmp

PRIVATE struct type_member tpconst urosetiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(URoSetIterator, ursi_set), "->?AFrozen?GUniqueSet"),
	TYPE_MEMBER_END
};


INTERN WUNUSED NONNULL((1)) int DCALL
urosetiterator_init(URoSetIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	URoSet *set;
	DeeArg_Unpack1(err, argc, argv, "_UniqueRoSetIterator", &set);
	if (DeeObject_AssertTypeExact(set, &URoSet_Type))
		goto err;
	self->ursi_set = set;
	Dee_Incref(set);
	self->ursi_next = set->urs_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
urosetiterator_bool(URoSetIterator *__restrict self) {
	struct uset_item *item = READ_ITEM(self);
	URoSet *set            = self->ursi_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item >= set->urs_elem &&
	        item < set->urs_elem + (set->urs_mask + 1));
}

INTERN DeeTypeObject URoSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueRoSetIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ URoSetIterator,
			/* tp_ctor:        */ &urosetiterator_ctor,
			/* tp_copy_ctor:   */ &urosetiterator_copy,
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ &urosetiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&urosetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&urosetiterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&urosetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &urosetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urosetiterator_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ urosetiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


STATIC_ASSERT(offsetof(URoSet, urs_size) == offsetof(USet, us_used));
#define uroset_size uset_size
#define uroset_bool uset_bool


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
uroset_printrepr(URoSet *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	bool is_first = true;
	size_t i;
	result = DeeFormat_PRINT(printer, arg, "collections.UniqueSet.Frozen({ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->urs_mask; ++i) {
		if (!self->urs_elem[i].usi_key)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_PrintObjectRepr(printer, arg, self->urs_elem[i].usi_key));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
URoSet_Contains(URoSet *__restrict self,
                DeeObject *__restrict ob) {
	size_t mask;
	Dee_hash_t i, perturb;
	mask    = self->urs_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->urs_elem[i & mask];
		if (!item->usi_key)
			break; /* Not found */
		if (!USAME(item->usi_key, ob))
			continue;
		/* Found it! */
		return 1;
	}
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF URoSetIterator *DCALL
uroset_iter(URoSet *__restrict self) {
	DREF URoSetIterator *result;
	result = DeeObject_MALLOC(URoSetIterator);
	if likely(result) {
		result->ursi_set  = self;
		result->ursi_next = self->urs_elem;
		Dee_Incref(self);
		DeeObject_Init(result, &URoSetIterator_Type);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
uroset_contains(URoSet *self, DeeObject *elem) {
	return_bool(URoSet_Contains(self, elem));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
uroset_foreach(URoSet *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i <= self->urs_mask; ++i) {
		DeeObject *key = self->urs_elem[i].usi_key;
		if (!key)
			continue;
		temp = (*proc)(arg, key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
uroset_asvector_nothrow(URoSet *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if likely(dst_length >= self->urs_size) {
		struct uset_item *iter, *end;
		end = (iter = self->urs_elem) + (self->urs_mask + 1);
		for (; iter < end; ++iter) {
			DeeObject *key = iter->usi_key;
			if (key == NULL)
				continue;
			Dee_Incref(key);
			*dst++ = key;
		}
	}
	return self->urs_size;
}


PRIVATE struct type_seq uroset_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uroset_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&uroset_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&uroset_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&uroset_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&uroset_size,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&uroset_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&uroset_asvector_nothrow,
};

PRIVATE NONNULL((1, 2)) void DCALL
uroset_visit(URoSet *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->urs_mask; ++i) {
		DeeObject *key = self->urs_elem[i].usi_key;
		/* Visit all keys and associated values. */
		Dee_XVisit(key);
	}
}

INTERN WUNUSED DREF URoSet *DCALL URoSet_New(void) {
	DREF URoSet *result;
	result = (DREF URoSet *)DeeObject_Mallocc(offsetof(URoSet, urs_elem),
	                                          1, sizeof(struct uset_item));
	if likely(result) {
		result->urs_mask           = 0;
		result->urs_size           = 0;
		result->urs_elem[0].usi_key = NULL;
		DeeObject_Init(result, &URoSet_Type);
	}
	return result;
}

LOCAL NONNULL((1, 2)) void DCALL
URoSet_DoInsertUnlocked(URoSet *__restrict self,
                        DREF DeeObject *__restrict ob) {
	Dee_hash_t i, perturb;
	ASSERT(ob != dummy);
	perturb = i = UHASH(ob) & self->urs_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->urs_elem[i & self->urs_mask];
		if (item->usi_key) { /* Already in use */
			if likely(!USAME(item->usi_key, ob))
				continue;
			--self->urs_size;
			Dee_Decref_unlikely(ob);
			return;
		}
		item->usi_key = ob; /* Inherit reference. */
		break;
	}
}

LOCAL NONNULL((1, 2)) void DCALL
URoSet_DoInsertForce(URoSet *__restrict self,
                     DREF DeeObject *__restrict ob) {
	Dee_hash_t i, perturb;
	ASSERT(ob != dummy);
	perturb = i = UHASH(ob) & self->urs_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->urs_elem[i & self->urs_mask];
		if (item->usi_key) { /* Already in use */
			ASSERT(!USAME(item->usi_key, ob));
			continue;
		}
		item->usi_key = ob; /* Inherit reference. */
		break;
	}
}

LOCAL WUNUSED NONNULL((1, 2)) URoSet *DCALL
URoSet_DoInsertOrRehash(URoSet *__restrict self,
                        /*inherit(always)*/ DREF DeeObject *__restrict ob) {
	Dee_hash_t i, perturb;
	if ((self->urs_size + 1) >= self->urs_mask) {
		/* Must re-hash */
		size_t newmsk;
		URoSet *newset;
		newmsk = (self->urs_mask << 1) | 1;
		newset = (URoSet *)DeeObject_Callocc(offsetof(URoSet, urs_elem),
		                                     newmsk + 1,
		                                     sizeof(struct uset_item));
		if unlikely(!newset)
			goto err_ob;
		newset->urs_size = self->urs_size;
		newset->urs_mask = newmsk;
		for (i = 0; i <= self->urs_mask; ++i) {
			DREF DeeObject *key;
			key = self->urs_elem[i].usi_key;
			if (key)
				URoSet_DoInsertForce(newset, key);
		}
		DeeObject_Free(self);
		self = newset;
	}
	perturb = i = UHASH(ob) & self->urs_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item;
		item = &self->urs_elem[i & self->urs_mask];
		if (item->usi_key) { /* Already in use */
			if likely(!USAME(item->usi_key, ob))
				continue;
			/* Already included... */
			Dee_Decref_unlikely(ob);
			return self;
		}
		item->usi_key = ob; /* Inherit reference. */
		break;
	}
	++self->urs_size;
	return self;
err_ob:
	Dee_Decref(ob);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF URoSet *DCALL
URoSet_FromUSet(USet *__restrict self) {
	DREF URoSet *result;
	size_t i, mask;
again_lock_hashset_src:
	USet_LockRead(self);
	mask = 1;
	while ((self->us_used & mask) != self->us_used)
		mask = (mask << 1) | 1;
	result = (DREF URoSet *)DeeObject_TryCallocc(offsetof(URoSet, urs_elem),
	                                             mask + 1, sizeof(struct uset_item));
	if unlikely(!result) {
		size_t oldsize;
		oldsize = self->us_used;
		USet_LockEndRead(self);
		result = (DREF URoSet *)DeeObject_Callocc(offsetof(URoSet, urs_elem),
		                                          mask + 1, sizeof(struct uset_item));
		if unlikely(!result)
			goto done;
		USet_LockRead(self);
		if (oldsize != self->us_used) {
			USet_LockEndRead(self);
			DeeObject_Free(result);
			goto again_lock_hashset_src;
		}
	}
	result->urs_mask = mask;
	result->urs_size = self->us_used;
	if (mask == self->us_mask) {
		for (i = 0; i <= mask; ++i) {
			DREF DeeObject *key;
			key = self->us_elem[i].usi_key;
			if (key != NULL && key != dummy) {
				Dee_Incref(key);
				result->urs_elem[i].usi_key = key;
			}
		}
	} else {
		for (i = 0; i <= self->us_mask; ++i) {
			DREF DeeObject *key;
			key = self->us_elem[i].usi_key;
			if (key != NULL && key != dummy) {
				Dee_Incref(key);
				URoSet_DoInsertUnlocked(result, key);
			}
		}
	}
	USet_LockEndRead(self);
	DeeObject_Init(result, &URoSet_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
roset_init_foreach_cb(void *arg, DeeObject *elem) {
	DREF URoSet *new_result;
	Dee_Incref(elem);
	new_result = URoSet_DoInsertOrRehash(*(DREF URoSet **)arg, elem);
	if unlikely(!new_result)
		goto err;
	*(DREF URoSet **)arg = new_result;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF URoSet *DCALL
URoSet_FromSequence(DeeObject *__restrict sequence) {
	DREF URoSet *result;
	DeeTypeObject *type = Dee_TYPE(sequence);
	if (type == &URoSet_Type) {
		return_reference_((URoSet *)sequence);
	} else if (type == &USet_Type) {
		return URoSet_FromUSet((USet *)sequence);
	} else if (type == &DeeHashSet_Type) {
		size_t i, mask;
		DeeHashSetObject *src;
		src = (DeeHashSetObject *)sequence;
again_lock_hashset_src:
		DeeHashSet_LockRead(src);
		mask = 1;
		while ((src->hs_used & mask) != src->hs_used)
			mask = (mask << 1) | 1;
		result = (DREF URoSet *)DeeObject_TryCallocc(offsetof(URoSet, urs_elem),
		                                             mask + 1, sizeof(struct uset_item));
		if unlikely(!result) {
			size_t oldsize;
			oldsize = src->hs_used;
			DeeHashSet_LockEndRead(src);
			result = (DREF URoSet *)DeeObject_Callocc(offsetof(URoSet, urs_elem),
			                                          mask + 1, sizeof(struct uset_item));
			if unlikely(!result)
				goto err;
			DeeHashSet_LockRead(src);
			if (oldsize != src->hs_used) {
				DeeHashSet_LockEndRead(src);
				DeeObject_Free(result);
				goto again_lock_hashset_src;
			}
		}
		result->urs_mask = mask;
		result->urs_size = src->hs_used;
		for (i = 0; i <= src->hs_mask; ++i) {
			DREF DeeObject *key;
			key = src->hs_elem[i].hsi_key;
			if (key != NULL && key != dummy) {
				Dee_Incref(key);
				URoSet_DoInsertUnlocked(result, key);
			}
		}
		DeeHashSet_LockEndRead(src);
	} else if (type == &DeeRoSet_Type) {
		size_t i;
		DeeRoSetObject *src;
		src    = (DeeRoSetObject *)sequence;
		result = (DREF URoSet *)DeeObject_Callocc(offsetof(URoSet, urs_elem),
		                                          src->rs_mask + 1,
		                                          sizeof(struct uset_item));
		if unlikely(!result)
			goto err;
		result->urs_mask = src->rs_mask;
		result->urs_size = src->rs_size;
		for (i = 0; i <= src->rs_mask; ++i) {
			DREF DeeObject *key;
			key = src->rs_elem[i].rsi_key;
			if (key != NULL) {
				Dee_Incref(key);
				URoSet_DoInsertUnlocked(result, key);
			}
		}
	} else {
		/* Fallback: Try the fast-sequence interface. */
		size_t sizehint = DeeObject_SizeFast(sequence);
		if (sizehint != (size_t)-1) {
			size_t mask;
			if (sizehint == 0)
				return URoSet_New();
			mask = 1;
			/* Figure out how large the mask of the set is going to be. */
			while ((sizehint & mask) != sizehint)
				mask = (mask << 1) | 1;
			result = (DREF URoSet *)DeeObject_TryCallocc(offsetof(URoSet, urs_elem),
			                                             mask + 1,
			                                             sizeof(struct uset_item));
			if unlikely(!result)
				goto use_zero_sizehint;
			result->urs_mask = mask;
		} else {
use_zero_sizehint:
			result = (DREF URoSet *)DeeObject_Mallocc(offsetof(URoSet, urs_elem),
			                                          1, sizeof(struct uset_item));
			if unlikely(!result)
				goto err;
			result->urs_mask            = 0;
			result->urs_size            = 0;
			result->urs_elem[0].usi_key = NULL;
		}
		if unlikely(DeeObject_Foreach(sequence, &roset_init_foreach_cb, &result))
			goto err_r_elem;
	}
	DeeObject_Init(result, &URoSet_Type);
	return result;
err_r_elem:
	{
		STATIC_ASSERT(sizeof(DREF DeeObject *) == sizeof(struct uset_item));
		Dee_XDecrefv((DREF DeeObject **)result->urs_elem,
		             result->urs_mask + 1);
	}
/*err_r:*/
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF URoSet *DCALL
uroset_deepcopy(URoSet *__restrict self) {
	DREF URoSet *result;
	size_t i;
	result = (DREF URoSet *)DeeObject_Callocc(offsetof(URoSet, urs_elem),
	                                          self->urs_mask + 1,
	                                          sizeof(struct uset_item));
	if unlikely(!result)
		goto err;
	result->urs_mask = self->urs_mask;
	result->urs_size = self->urs_size;
	for (i = 0; i <= self->urs_mask; ++i) {
		DREF DeeObject *key;
		key = self->urs_elem[i].usi_key;
		if (key != NULL) {
			key = DeeObject_DeepCopy(key);
			if unlikely(!key)
				goto err_r;
			URoSet_DoInsertUnlocked(result, key);
		}
	}
	DeeObject_Init(result, &URoSet_Type);
	return result;
err_r:
	Dee_XDecrefv((DREF DeeObject **)result->urs_elem, i);
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF URoSet *DCALL
uroset_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "_UniqueRoSet", &seq);
	return URoSet_FromSequence(seq);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
uroset_fini(URoSet *__restrict self) {
	size_t i;
	for (i = 0; i <= self->urs_mask; ++i) {
		DREF DeeObject *key;
		key = self->urs_elem[i].usi_key;
		Dee_XDecref(key);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uroset_sizeof(URoSet *self) {
	size_t result;
	result = _Dee_MallococBufsize(offsetof(URoSet, urs_elem),
	                              self->urs_mask + 1,
	                              sizeof(struct uset_item));
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset tpconst uroset_getsets[] = {
	TYPE_GETTER_AB("frozen", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &uroset_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst uroset_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(URoSet, urs_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(URoSet, urs_size)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst uroset_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &URoSetIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &URoSet_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject URoSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueRoSet",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &URoSet_New,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &uroset_deepcopy,
			/* tp_any_ctor:    */ &uroset_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uroset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&uroset_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&uroset_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&uroset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &uroset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ uroset_getsets,
	/* .tp_members       = */ uroset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ uroset_class_members
};



DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_USET_C */
