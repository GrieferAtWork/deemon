/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_COLLECTIONS_USET_C
#define GUARD_DEX_COLLECTIONS_USET_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dict.h> /* DeeDict_Dummy */
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/system-features.h> /* memcpyc(), ... */

#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>

DECL_BEGIN

/* Dummy key object. */
#define dummy (&DeeDict_Dummy)

PRIVATE /*WUNUSED*/ NONNULL((1)) int DCALL USet_InitEmpty(USet *__restrict self);
PRIVATE NONNULL((1)) void DCALL USet_Fini(USet *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_InitCopy(USet *__restrict self, USet *__restrict other);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_InitIterator(USet *__restrict self, DeeObject *__restrict iterator);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_InitSequence(USet *__restrict self, DeeObject *__restrict sequence);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_Insert(USet *__restrict self, DeeObject *__restrict ob);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_DoInsertNolock(USet *__restrict self, DeeObject *__restrict ob);
LOCAL void DCALL USet_DoInsertUnlocked(USet *__restrict self, DREF DeeObject *__restrict ob);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL USet_Remove(USet *__restrict self, DeeObject *__restrict ob);
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL USet_Contains(USet *__restrict self, DeeObject *__restrict ob);


INTDEF DeeTypeObject USet_Type;
INTDEF DeeTypeObject USetIterator_Type;
INTDEF DeeTypeObject URoSet_Type;
INTDEF DeeTypeObject URoSetIterator_Type;




#ifdef CONFIG_NO_THREADS
#define READ_ITEM(x) ((x)->si_next)
#else /* CONFIG_NO_THREADS */
#define READ_ITEM(x) ATOMIC_READ((x)->si_next)
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usetiterator_next(USetIterator *__restrict self) {
	DREF DeeObject *result;
	struct uset_item *item, *end;
	USet *set = self->si_set;
	USet_LockRead(set);
	end = set->s_elem + (set->s_mask + 1);
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->si_next);
#else /* CONFIG_NO_THREADS */
		struct uset_item *old_item;
		old_item = item = ATOMIC_READ(self->si_next);
#endif /* !CONFIG_NO_THREADS */
		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto set_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < set->s_elem)
			goto set_has_changed;
		/* Search for the next non-empty item. */
		while (item != end && (!item->si_key || item->si_key == dummy))
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->si_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->si_next, old_item, item))
				continue;
#endif /*!CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->si_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->si_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	result = item->si_key;
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
	self->si_set = (DREF USet *)DeeObject_NewDefault(&USet_Type);
	if unlikely(!self->si_set)
		goto err;
	self->si_next = self->si_set->s_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usetiterator_copy(USetIterator *__restrict self,
                  USetIterator *__restrict other) {
	self->si_set = other->si_set;
	Dee_Incref(self->si_set);
	self->si_next = READ_ITEM(other);
	return 0;
}


INTERN NONNULL((1)) void DCALL
usetiterator_fini(USetIterator *__restrict self) {
	Dee_Decref(self->si_set);
}

INTERN NONNULL((1, 2)) void DCALL
usetiterator_visit(USetIterator *__restrict self,
                   dvisit_t proc, void *arg) {
	Dee_Visit(self->si_set);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
usetiterator_init(USetIterator *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	USet *set;
	if (DeeArg_Unpack(argc, argv, "o:_UniqueSetIterator", &set))
		goto err;
	if (DeeObject_AssertType(set, &USet_Type))
		goto err;
	self->si_set = set;
	Dee_Incref(set);
	self->si_next = set->s_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
usetiterator_bool(USetIterator *__restrict self) {
	struct uset_item *item = READ_ITEM(self);
	USet *set              = self->si_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item >= set->s_elem &&
	        item < set->s_elem + (set->s_mask + 1));
}

#define DEFINE_ITERATOR_COMPARE(name, op)                     \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL     \
	name(USetIterator *self, USetIterator *other) {           \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                         \
		return_bool(READ_ITEM(self) op READ_ITEM(other));     \
	err:                                                      \
		return NULL;                                          \
	}
DEFINE_ITERATOR_COMPARE(usetiterator_eq, ==)
DEFINE_ITERATOR_COMPARE(usetiterator_ne, !=)
DEFINE_ITERATOR_COMPARE(usetiterator_lo, <)
DEFINE_ITERATOR_COMPARE(usetiterator_le, <=)
DEFINE_ITERATOR_COMPARE(usetiterator_gr, >)
DEFINE_ITERATOR_COMPARE(usetiterator_ge, >=)
#undef DEFINE_ITERATOR_COMPARE

INTERN struct type_cmp usetiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&usetiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&usetiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&usetiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&usetiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&usetiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&usetiterator_ge
};

PRIVATE struct type_member tpconst usetiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(USetIterator, si_set), "->?GUniqueSet"),
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&usetiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&usetiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&usetiterator_init,
				TYPE_FIXED_ALLOCATOR(USetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&usetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&usetiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&usetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &usetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&usetiterator_next,
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
	self->s_mask = 0;
	self->s_size = 0;
	self->s_used = 0;
	self->s_elem = empty_set_items;
	atomic_rwlock_init(&self->s_lock);
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
USet_InitIterator(USet *__restrict self,
                  DeeObject *__restrict iterator) {
	DREF DeeObject *elem;
	USet_InitEmpty(self);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		int error;
		error = USet_DoInsertNolock(self, elem);
		Dee_Decref(elem);
		if unlikely(error < 0)
			goto err;
	}
	if unlikely(!elem)
		goto err;
	return 0;
err:
	USet_Fini(self);
	return -1;
}

LOCAL void DCALL
USet_DoInsertUnlocked(USet *__restrict self,
                      DREF DeeObject *__restrict ob) {
	dhash_t i, perturb;
	perturb = i = UHASH(ob) & self->s_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->s_elem[i & self->s_mask];
		if (item->si_key) { /* Already in use */
			if likely(!USAME(item->si_key, ob))
				continue;
			--self->s_size;
			--self->s_used;
			Dee_Decref_unlikely(ob);
			return;
		}
		item->si_key = ob; /* Inherit reference. */
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
	size_t new_mask = self->s_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!self->s_used) {
			ASSERT(!self->s_used);
			/* Special case: delete the vector. */
			if (self->s_size) {
				ASSERT(self->s_elem != empty_set_items);
				/* Must discard dummy items. */
				end = (iter = self->s_elem) + (self->s_mask + 1);
				for (; iter < end; ++iter) {
					ASSERT(iter->si_key == NULL ||
					       iter->si_key == dummy);
					if (iter->si_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->s_elem != empty_set_items)
				Dee_Free(self->s_elem);
			self->s_elem = empty_set_items;
			self->s_mask = 0;
			self->s_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (self->s_used >= new_mask)
			return true;
	}
	ASSERT(self->s_used < new_mask);
	ASSERT(self->s_used <= self->s_size);
	new_vector = (struct uset_item *)Dee_TryCalloc((new_mask + 1) *
	                                               sizeof(struct uset_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->s_elem == empty_set_items) == (self->s_mask == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_used == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_size == 0));
	if (self->s_elem != empty_set_items) {
		/* Re-insert all existing items into the new set vector. */
		end = (iter = self->s_elem) + (self->s_mask + 1);
		for (; iter < end; ++iter) {
			struct uset_item *item;
			dhash_t i, perturb;
			/* Skip dummy keys. */
			if (iter->si_key == dummy)
				continue;
			perturb = i = UHASH(iter->si_key) & new_mask;
			for (;; USet_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->si_key)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			item->si_key = iter->si_key;
		}
		Dee_Free(self->s_elem);
		/* With all dummy items gone, the size now equals what is actually used. */
		self->s_size = self->s_used;
	}
	ASSERT(self->s_size == self->s_used);
	self->s_mask = new_mask;
	self->s_elem = new_vector;
	return true;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
USet_Remove(USet *__restrict self,
            DeeObject *__restrict ob) {
	size_t mask;
	struct uset_item *vector;
	dhash_t i, perturb;
	USet_LockRead(self);
restart:
	vector  = self->s_elem;
	mask    = self->s_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct uset_item *item = &vector[i & mask];
		item_key               = item->si_key;
		if (!item_key)
			break; /* Not found */
		if (!USAME(item_key, ob))
			continue;
		/* Found it! */
		if (!USet_LockUpgrade(self)) {
			/* Check if the set was modified. */
			if (self->s_elem != vector ||
			    self->s_mask != mask ||
			    item->si_key != item_key) {
				USet_LockDowngrade(self);
				goto restart;
			}
		}
		item->si_key = dummy;
		Dee_Incref(dummy);
		ASSERT(self->s_used);
		if (--self->s_used < self->s_size / 2)
			uset_rehash(self, -1);
		USet_LockEndWrite(self);
		Dee_Decref(item_key);
		return 1;
	}
	USet_LockEndRead(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
USet_Contains(USet *__restrict self,
              DeeObject *__restrict ob) {
	size_t mask;
	dhash_t i, perturb;
	USet_LockRead(self);
	mask    = self->s_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->s_elem[i & mask];
		if (!item->si_key)
			break; /* Not found */
		if (!USAME(item->si_key, ob))
			continue;
		/* Found it! */
		USet_LockEndRead(self);
		return 1;
	}
	USet_LockEndRead(self);
	return 0;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
USet_Insert(USet *__restrict self,
            DeeObject *__restrict ob) {
	struct uset_item *first_dummy;
	size_t mask;
	dhash_t i, perturb;
again_lock:
	USet_LockRead(self);
again:
	first_dummy = NULL;
	mask        = self->s_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->s_elem[i & mask];
		if (!item->si_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->si_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (USAME(item->si_key, ob)) { /* Same object */
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
	if (first_dummy && self->s_size + 1 < self->s_mask) {
		ASSERT(first_dummy != empty_set_items);
		ASSERT(!first_dummy->si_key ||
		       first_dummy->si_key == dummy);
		if (first_dummy->si_key)
			Dee_DecrefNokill(first_dummy->si_key);
		/* Fill in the target slot. */
		first_dummy->si_key = ob;
		Dee_Incref(ob);
		++self->s_used;
		++self->s_size;
		/* Try to keep the set vector big at least twice as big as the element count. */
		if (self->s_size * 2 > self->s_mask)
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
USet_DoInsertNolock(USet *__restrict self,
                    DeeObject *__restrict ob) {
	struct uset_item *first_dummy;
	size_t mask;
	dhash_t i, perturb;
again:
	first_dummy = NULL;
	mask        = self->s_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->s_elem[i & mask];
		if (!item->si_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->si_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (USAME(item->si_key, ob)) /* Same object */
			return 0;                /* Already exists. */
	}
	if (first_dummy && self->s_size + 1 < self->s_mask) {
		ASSERT(first_dummy != empty_set_items);
		ASSERT(!first_dummy->si_key ||
		       first_dummy->si_key == dummy);
		if (first_dummy->si_key)
			Dee_DecrefNokill(first_dummy->si_key);
		/* Fill in the target slot. */
		first_dummy->si_key = ob;
		Dee_Incref(ob);
		++self->s_used;
		++self->s_size;
		/* Try to keep the set vector big at least twice as big as the element count. */
		if (self->s_size * 2 > self->s_mask)
			uset_rehash(self, 1);
		return 1; /* New item. */
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
		atomic_rwlock_init(&self->s_lock);
again_hashset:
		USet_LockRead(src);
		self->s_used = self->s_size = src->s_used;
		if (!self->s_used) {
			self->s_mask = 0;
			self->s_elem = empty_set_items;
		} else {
			size_t i;
			self->s_mask = src->s_mask;
			self->s_elem = (struct uset_item *)Dee_TryCalloc((src->s_mask + 1) *
			                                                 sizeof(struct uset_item));
			if unlikely(!self->s_elem) {
				USet_LockEndRead(src);
				if (Dee_CollectMemory((self->s_mask + 1) * sizeof(struct uset_item)))
					goto again_hashset;
				return -1;
			}
			for (i = 0; i <= src->s_mask; ++i) {
				DeeObject *key = src->s_elem[i].si_key;
				if (!key || key == dummy)
					continue;
				Dee_Incref(key);
				USet_DoInsertUnlocked(self, key);
			}
		}
		USet_LockEndRead(src);
		weakref_support_init(self);
		return 0;
	}
	if (type == &URoSet_Type) {
		URoSet *src = (URoSet *)sequence;
		atomic_rwlock_init(&self->s_lock);
		self->s_used = self->s_size = src->rs_size;
		if unlikely(!self->s_size) {
			self->s_mask = 0;
			self->s_elem = (struct uset_item *)empty_set_items;
		} else {
			self->s_mask = src->rs_mask;
			self->s_elem = (struct uset_item *)Dee_Malloc((src->rs_mask + 1) *
			                                              sizeof(struct uset_item));
			if unlikely(!self->s_elem)
				goto err;
			Dee_XMovrefv((DeeObject **)self->s_elem,
			             (DeeObject **)src->rs_elem,
			             src->rs_mask + 1);
		}
		weakref_support_init(self);
		return 0;
	}
	if (type == &DeeRoSet_Type) {
		size_t i;
		DeeRoSetObject *src;
		src = (DeeRoSetObject *)sequence;
		atomic_rwlock_init(&self->s_lock);
		self->s_used = self->s_size = src->rs_size;
		if unlikely(!self->s_size) {
			self->s_mask = 0;
			self->s_elem = (struct uset_item *)empty_set_items;
		} else {
			self->s_mask = src->rs_mask;
			self->s_elem = (struct uset_item *)Dee_Calloc((src->rs_mask + 1) *
			                                              sizeof(struct uset_item));
			if unlikely(!self->s_elem)
				goto err;
			for (i = 0; i <= src->rs_mask; ++i) {
				DeeObject *key = src->rs_elem[i].si_key;
				if (!key)
					continue;
				Dee_Incref(key);
				USet_DoInsertUnlocked(self, key);
			}
		}
		weakref_support_init(self);
		return 0;
	}
	{
		/* Fallback: Try the fast-sequence interface. */
		size_t i, fastsize = DeeFastSeq_GetSize(sequence);
		if (fastsize != DEE_FASTSEQ_NOTFAST) {
			size_t min_mask, mask;
			if (fastsize == 0)
				return USet_InitEmpty(self);
			min_mask = 16 - 1;
			/* Figure out how large the mask of the set is going to be. */
			while ((fastsize & min_mask) != fastsize)
				min_mask = (min_mask << 1) | 1;
			/* Prefer using a mask of one greater level to improve performance. */
			mask         = (min_mask << 1) | 1;
			self->s_elem = (struct uset_item *)Dee_TryCalloc((mask + 1) * sizeof(struct uset_item));
			if unlikely(!self->s_elem) {
				/* Try one level less if that failed. */
				mask         = min_mask;
				self->s_elem = (struct uset_item *)Dee_Calloc((mask + 1) * sizeof(struct uset_item));
				if unlikely(!self->s_elem)
					goto err;
			}
			/* Without any dummy items, these are identical. */
			self->s_mask = mask;
			self->s_used = fastsize;
			self->s_size = fastsize;
			for (i = 0; i < fastsize; ++i) {
				DREF DeeObject *key;
				key = DeeFastSeq_GetItemUnbound(sequence, i);
				if unlikely(!ITER_ISOK(key)) {
					if unlikely(key == ITER_DONE)
						goto err_elem;
					ASSERT(self->s_size);
					ASSERT(self->s_used);
					--self->s_size;
					--self->s_used;
					if unlikely(!self->s_size) {
						Dee_Free(self->s_elem);
						self->s_elem = empty_set_items;
						ASSERT(self->s_used == 0);
						break;
					}
					continue;
				}
				USet_DoInsertUnlocked(self, key);
			}
			atomic_rwlock_init(&self->s_lock);
			weakref_support_init(self);
			return 0;
		}
	}
	{
		/* Fallback: Use iterators. */
		int result;
		DREF DeeObject *iterator;
		iterator = DeeObject_IterSelf(sequence);
		if unlikely(!iterator)
			goto err;
		result = USet_InitIterator(self, iterator);
		Dee_Decref(iterator);
		return result;
	}
	{
		size_t i;
err_elem:
		for (i = 0; i <= self->s_mask; ++i)
			Dee_XDecref(self->s_elem[i].si_key);
	}
	Dee_Free(self->s_elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF USet *DCALL
USet_FromSequence(DeeObject *__restrict sequence) {
	DREF USet *result;
	result = DeeGCObject_MALLOC(USet);
	if unlikely(!result)
		goto done;
	if unlikely(USet_InitSequence(result, sequence))
		goto err;
	DeeObject_Init(result, &USet_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return result;
err:
	DeeGCObject_FREE(result);
	return NULL;
}









PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_sizeof(USet *self) {
	return DeeInt_NewSize(sizeof(USet) +
	                      ((self->s_mask + 1) *
	                       sizeof(struct uset_item)));
}

PRIVATE struct type_getset tpconst uset_getsets[] = {
	{ "frozen",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&URoSet_FromUSet, NULL, NULL,
	  DOC("->?AFrozen?.\n"
	      "Returns a read-only (frozen) copy of @this set") },
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uset_sizeof, NULL, NULL,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_member tpconst uset_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(USet, s_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(USet, s_size)),
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
	atomic_rwlock_init(&self->s_lock);
again:
	USet_LockRead(other);
	self->s_mask = other->s_mask;
	self->s_size = other->s_size;
	self->s_used = other->s_used;
	if ((self->s_elem = other->s_elem) != empty_set_items) {
		self->s_elem = (struct uset_item *)Dee_TryMalloc((other->s_mask + 1) *
		                                                 sizeof(struct uset_item));
		if unlikely(!self->s_elem) {
			USet_LockEndRead(other);
			if (Dee_CollectMemory((self->s_mask + 1) * sizeof(struct uset_item)))
				goto again;
			return -1;
		}
		iter = (struct uset_item *)memcpyc(self->s_elem, other->s_elem,
		                                   self->s_mask + 1,
		                                   sizeof(struct uset_item));
		end  = iter + (self->s_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->si_key)
				continue;
			Dee_Incref(iter->si_key);
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
		if (self->s_elem == empty_set_items) {
			USet_LockEndRead(self);
			return 0;
		}
		item_count = self->s_used;
		if (item_count <= ols_item_count)
			break;
		USet_LockEndRead(self);
		new_items = (DREF DeeObject **)Dee_Realloc(items,
		                                           item_count *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!new_items)
			goto err_items;
		ols_item_count = item_count;
		items          = new_items;
	}
	/* Copy all used items. */
	for (i = 0, hash_i = 0; i < item_count; ++hash_i) {
		ASSERT(hash_i <= self->s_mask);
		if (self->s_elem[hash_i].si_key == NULL)
			continue;
		if (self->s_elem[hash_i].si_key == dummy)
			continue;
		items[i] = self->s_elem[hash_i].si_key;
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
	new_map = (struct uset_item *)Dee_Calloc((new_mask + 1) * sizeof(struct uset_item));
	if unlikely(!new_map)
		goto err_items_v;
	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		dhash_t j, perturb;
		perturb = j = UHASH(items[i]) & new_mask;
		for (;; USet_HashNx(j, perturb)) {
			struct uset_item *item = &new_map[j & new_mask];
			if (item->si_key) {
				if likely(!USAME(item->si_key, items[i]))
					continue; /* Already in use */
				Dee_Decref(items[i]);
				goto next_item;
			}
			item->si_key = items[i]; /* Inherit reference. */
			break;
		}
next_item:
		;
	}
	USet_LockWrite(self);
	i            = self->s_mask + 1;
	self->s_mask = new_mask;
	self->s_used = item_count;
	self->s_size = item_count;
	ols_map      = self->s_elem;
	self->s_elem = new_map;
	USet_LockEndWrite(self);
	if (ols_map != empty_set_items) {
		while (i--)
			Dee_XDecref(ols_map[i].si_key);
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
	ASSERT((self->s_elem == empty_set_items) == (self->s_mask == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_size == 0));
	ASSERT(self->s_used <= self->s_size);
	if (self->s_elem != empty_set_items) {
		size_t i;
		for (i = 0; i <= self->s_mask; ++i)
			Dee_XDecref(self->s_elem[i].si_key);
		Dee_Free(self->s_elem);
	}
}

PRIVATE NONNULL((1)) void DCALL uset_clear(USet *__restrict self) {
	struct uset_item *elem;
	size_t mask;
	USet_LockWrite(self);
	ASSERT((self->s_elem == empty_set_items) == (self->s_mask == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_used == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_size == 0));
	ASSERT(self->s_used <= self->s_size);
	/* Extract the vector and mask. */
	elem         = self->s_elem;
	mask         = self->s_mask;
	self->s_elem = empty_set_items;
	self->s_mask = 0;
	self->s_used = 0;
	self->s_size = 0;
	USet_LockEndWrite(self);
	/* Destroy the vector. */
	if (elem != empty_set_items) {
		size_t i;
		for (i = 0; i <= mask; ++i) {
			DeeObject *key = elem[i].si_key;
			Dee_XDecref(key);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
uset_visit(USet *__restrict self, dvisit_t proc, void *arg) {
	USet_LockRead(self);
	ASSERT((self->s_elem == empty_set_items) == (self->s_mask == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_used == 0));
	ASSERT((self->s_elem == empty_set_items) == (self->s_size == 0));
	ASSERT(self->s_used <= self->s_size);
	if (self->s_elem != empty_set_items) {
		size_t i;
		for (i = 0; i <= self->s_mask; ++i) {
			DeeObject *key = self->s_elem[i].si_key;
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
	result->si_set = self;
	Dee_Incref(self);
#ifdef CONFIG_NO_THREADS
	result->si_next = self->s_elem;
#else /* CONFIG_NO_THREADS */
	result->si_next = ATOMIC_READ(self->s_elem);
#endif /* !CONFIG_NO_THREADS */
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_size(USet *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return DeeInt_NewSize(self->s_used);
#else /* CONFIG_NO_THREADS */
	return DeeInt_NewSize(ATOMIC_READ(self->s_used));
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
uset_nsi_getsize(USet *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->s_used;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->s_used);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
uset_contains(USet *self, DeeObject *search_item) {
	return_bool(USet_Contains(self, search_item));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_repr(USet *__restrict self) {
	struct unicode_printer p;
	dssize_t error;
	struct uset_item *iter, *end;
	bool is_first;
	struct uset_item *vector;
	size_t mask;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "{ ") < 0)
		goto err;
	USet_LockRead(self);
	is_first = true;
	vector   = self->s_elem;
	mask     = self->s_mask;
	end      = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key;
		if (iter->si_key == NULL ||
		    iter->si_key == dummy)
			continue;
		key = iter->si_key;
		Dee_Incref(key);
		USet_LockEndRead(self);
		/* Print this item. */
		error = unicode_printer_printf(&p, "%s%r", is_first ? "" : ", ", key);
		Dee_Decref(key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
		USet_LockRead(self);
		if (self->s_elem != vector ||
		    self->s_mask != mask)
			goto restart;
	}
	USet_LockEndRead(self);
	if ((is_first ? unicode_printer_putascii(&p, '}')
	              : UNICODE_PRINTER_PRINT(&p, " }")) < 0)
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


INTERN WUNUSED NONNULL((1)) int DCALL
uset_bool(USet *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->s_used != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->s_used) != 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uset_init(USet *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if unlikely(DeeArg_Unpack(argc, argv, "o:UniqueSet", &seq))
		goto err;
	return USet_InitSequence(self, seq);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_pop(USet *self, size_t argc, DeeObject *const *argv) {
	size_t i;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":pop"))
		goto err;
	USet_LockWrite(self);
	for (i = 0; i <= self->s_mask; ++i) {
		struct uset_item *item = &self->s_elem[i];
		if ((result = item->si_key) == NULL)
			continue; /* Unused slot. */
		if (result == dummy)
			continue; /* Deleted slot. */
		item->si_key = dummy;
		Dee_Incref(dummy);
		ASSERT(self->s_used);
		if (--self->s_used < self->s_size / 2)
			uset_rehash(self, -1);
		USet_LockEndWrite(self);
		return result;
	}
	USet_LockEndWrite(self);
	/* Set is already empty. */
	err_empty_sequence((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_doclear(USet *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	uset_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_insert(USet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:insert", &item))
		goto err;
	result = USet_Insert(self, item);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_unify(USet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:unify", &item))
		goto err;
	/* Since items are always unique, simply try to insert the given one! */
	if (USet_Insert(self, item) < 0)
		goto err;
	return_reference_(item);
err:
	return NULL;
}

#if (__SIZEOF_INT__ >= __SIZEOF_POINTER__ || \
     (defined(__i386__) || defined(__x86_64__)))
#define insert_callback USet_Insert
#else /* ... */
PRIVATE dssize_t DCALL
insert_callback(USet *__restrict self, DeeObject *item) {
	return USet_Insert(self, item);
}
#endif /* !... */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_update(USet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	dssize_t result;
	if (DeeArg_Unpack(argc, argv, "o:update", &items))
		goto err;
	result = DeeObject_Foreach(items, (dforeach_t)&insert_callback, self);
	if unlikely(result < 0)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uset_remove(USet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:remove", &item))
		goto err;
	result = USet_Remove(self, item);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}





PRIVATE struct type_nsi tpconst uset_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SET,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_setlike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&uset_nsi_getsize,
			/* .nsi_insert     = */ (dfunptr_t)&USet_Insert,
			/* .nsi_remove     = */ (dfunptr_t)&USet_Remove,
		}
	}
};

PRIVATE struct type_seq uset_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uset_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uset_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&uset_contains,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &uset_nsi
};

PRIVATE struct type_method tpconst uset_methods[] = {
	{ "pop",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_pop,
	  DOC("->\n"
	      "@throw ValueError The set is empty\n"
	      "Pop a random item from the set and return it") },
	{ "clear",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_doclear,
	  DOC("()\n"
	      "Clear all items from the set") },
	{ "popitem",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_pop,
	  DOC("->\n"
	      "@throw ValueError The set is empty\n"
	      "Pop a random item from the set and return it (alias for ?#pop)") },
	{ "unify",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_unify,
	  DOC("(ob)->\n"
	      "Insert @ob into the set if it wasn't inserted before, "
	      "and re-return it, or the pre-existing instance") },
	{ "insert",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_insert,
	  DOC("(ob)->?Dbool\n"
	      "Returns ?t if the object wasn't apart of the set before") },
	{ "update",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_update,
	  DOC("(items:?S?O)->?Dint\n"
	      "Insert all items from @items into @this set, and return the number of inserted items") },
	{ "remove",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_remove,
	  DOC("(ob)->?Dbool\n"
	      "Returns ?t if the object was removed from the set") },
	/* Alternative function names. */
	{ "add",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_insert,
	  DOC("(ob)->?Dbool\n"
	      "Deprecated alias for ?#insert") },
	{ "discard",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&uset_remove,
	  DOC("(ob)->?Dbool\n"
	      "Deprecated alias for ?#remove") },
	{ NULL }
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
	                         "in @this set, following a random order"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(USet),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&USet_InitEmpty,
				/* .tp_copy_ctor = */ (dfunptr_t)&USet_InitCopy,
				/* .tp_deep_ctor = */ (dfunptr_t)&USet_InitCopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&uset_init,
				TYPE_FIXED_ALLOCATOR_GC(USet)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&USet_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&uset_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uset_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&uset_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&uset_visit,
	/* .tp_gc            = */ &uset_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &uset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ uset_methods,
	/* .tp_getsets       = */ uset_getsets,
	/* .tp_members       = */ uset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ uset_class_members
};










#undef READ_ITEM
#ifdef CONFIG_NO_THREADS
#define READ_ITEM(x) ((x)->si_next)
#else /* CONFIG_NO_THREADS */
#define READ_ITEM(x) ATOMIC_READ((x)->si_next)
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urosetiterator_next(URoSetIterator *__restrict self) {
	struct uset_item *item, *end;
	end = self->si_set->rs_elem + self->si_set->rs_mask + 1;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->si_next);
#else /* CONFIG_NO_THREADS */
		struct uset_item *old_item;
		old_item = item = ATOMIC_READ(self->si_next);
#endif /* !CONFIG_NO_THREADS */
		if (item >= end)
			goto iter_exhausted;
		while (item != end && !item->si_key)
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->si_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->si_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->si_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->si_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	return_reference_(item->si_key);
iter_exhausted:
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) int DCALL
urosetiterator_ctor(URoSetIterator *__restrict self) {
	self->si_set = URoSet_New();
	if unlikely(!self->si_set)
		goto err;
	self->si_next = self->si_set->rs_elem;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(USetIterator, si_set) == offsetof(URoSetIterator, si_set));
STATIC_ASSERT(offsetof(USetIterator, si_next) == offsetof(URoSetIterator, si_next));
#define urosetiterator_copy  usetiterator_copy
#define urosetiterator_fini  usetiterator_fini
#define urosetiterator_visit usetiterator_visit
#define urosetiterator_cmp   usetiterator_cmp

PRIVATE struct type_member tpconst urosetiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(URoSetIterator, si_set), "->?AFrozen?GUniqueSet"),
	TYPE_MEMBER_END
};


INTERN WUNUSED NONNULL((1)) int DCALL
urosetiterator_init(URoSetIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	URoSet *set;
	if (DeeArg_Unpack(argc, argv, "o:_UniqueRoSetIterator", &set))
		goto err;
	if (DeeObject_AssertTypeExact(set, &URoSet_Type))
		goto err;
	self->si_set = set;
	Dee_Incref(set);
	self->si_next = set->rs_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
urosetiterator_bool(URoSetIterator *__restrict self) {
	struct uset_item *item = READ_ITEM(self);
	URoSet *set            = self->si_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item >= set->rs_elem &&
	        item < set->rs_elem + (set->rs_mask + 1));
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&urosetiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&urosetiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&urosetiterator_init,
				TYPE_FIXED_ALLOCATOR(URoSetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&urosetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&urosetiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&urosetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &urosetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urosetiterator_next,
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


STATIC_ASSERT(offsetof(URoSet, rs_size) == offsetof(USet, s_used));
#define uroset_size        uset_size
#define uroset_bool        uset_bool
#define uroset_nsi_getsize uset_nsi_getsize


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uroset_repr(URoSet *__restrict self) {
	struct unicode_printer p;
	dssize_t error;
	bool is_first;
	size_t i;
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "{ ") < 0)
		goto err;
	is_first = true;
	for (i = 0; i <= self->rs_mask; ++i) {
		DREF DeeObject *key;
		key = self->rs_elem[i].si_key;
		if (key == NULL)
			continue;
		/* Print this item. */
		error = unicode_printer_printf(&p, "%s%r",
		                               is_first ? "" : ", ",
		                               key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
	}
	if ((is_first ? unicode_printer_putascii(&p, '}')
	              : UNICODE_PRINTER_PRINT(&p, " }")) < 0)
		goto err;
	return unicode_printer_pack(&p);
err:
	unicode_printer_fini(&p);
	return NULL;
}


PRIVATE struct type_nsi tpconst uroset_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SET,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_setlike = */ {
			/* .nsi_getsize = */ (dfunptr_t)&uroset_nsi_getsize,
		}
	}
};

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
URoSet_Contains(URoSet *__restrict self,
                DeeObject *__restrict ob) {
	size_t mask;
	dhash_t i, perturb;
	mask    = self->rs_mask;
	perturb = i = UHASH(ob) & mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->rs_elem[i & mask];
		if (!item->si_key)
			break; /* Not found */
		if (!USAME(item->si_key, ob))
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
		result->si_set  = self;
		result->si_next = self->rs_elem;
		Dee_Incref(self);
		DeeObject_Init(result, &URoSetIterator_Type);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
uroset_contains(URoSet *self, DeeObject *elem) {
	return_bool(URoSet_Contains(self, elem));
}

PRIVATE struct type_seq uroset_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uroset_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uroset_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&uroset_contains,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &uroset_nsi
};

PRIVATE NONNULL((1, 2)) void DCALL
uroset_visit(URoSet *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		DeeObject *key = self->rs_elem[i].si_key;
		/* Visit all keys and associated values. */
		Dee_XVisit(key);
	}
}

INTERN WUNUSED DREF URoSet *DCALL URoSet_New(void) {
	DREF URoSet *result;
	result = (DREF URoSet *)DeeObject_Malloc(offsetof(URoSet, rs_elem) +
	                                         1 * sizeof(struct uset_item));
	if likely(result) {
		result->rs_mask           = 0;
		result->rs_size           = 0;
		result->rs_elem[0].si_key = NULL;
		DeeObject_Init(result, &URoSet_Type);
	}
	return result;
}

LOCAL NONNULL((1, 2)) void DCALL
URoSet_DoInsertUnlocked(URoSet *__restrict self,
                        DREF DeeObject *__restrict ob) {
	dhash_t i, perturb;
	ASSERT(ob != dummy);
	perturb = i = UHASH(ob) & self->rs_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->rs_elem[i & self->rs_mask];
		if (item->si_key) { /* Already in use */
			if likely(!USAME(item->si_key, ob))
				continue;
			--self->rs_size;
			Dee_Decref_unlikely(ob);
			return;
		}
		item->si_key = ob; /* Inherit reference. */
		break;
	}
}

LOCAL NONNULL((1, 2)) void DCALL
URoSet_DoInsertForce(URoSet *__restrict self,
                     DREF DeeObject *__restrict ob) {
	dhash_t i, perturb;
	ASSERT(ob != dummy);
	perturb = i = UHASH(ob) & self->rs_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item = &self->rs_elem[i & self->rs_mask];
		if (item->si_key) { /* Already in use */
			ASSERT(!USAME(item->si_key, ob));
			continue;
		}
		item->si_key = ob; /* Inherit reference. */
		break;
	}
}

LOCAL WUNUSED NONNULL((1, 2)) URoSet *DCALL
URoSet_DoInsertOrRehash(URoSet *__restrict self,
                        /*inherit(always)*/ DREF DeeObject *__restrict ob) {
	dhash_t i, perturb;
	if ((self->rs_size + 1) >= self->rs_mask) {
		/* Must re-hash */
		size_t newmsk;
		URoSet *newset;
		newmsk = (self->rs_mask << 1) | 1;
		newset = (URoSet *)DeeObject_Calloc(offsetof(URoSet, rs_elem) +
		                                    (newmsk + 1) *
		                                    sizeof(struct uset_item));
		if unlikely(!newset)
			goto err_ob;
		newset->rs_mask = newmsk;
		for (i = 0; i <= self->rs_mask; ++i) {
			DREF DeeObject *key;
			key = self->rs_elem[i].si_key;
			if (key)
				URoSet_DoInsertForce(newset, key);
		}
		DeeObject_Free(self);
		self = newset;
	}
	perturb = i = UHASH(ob) & self->rs_mask;
	for (;; USet_HashNx(i, perturb)) {
		struct uset_item *item;
		item = &self->rs_elem[i & self->rs_mask];
		if (item->si_key) { /* Already in use */
			if likely(!USAME(item->si_key, ob))
				continue;
			/* Already included... */
			Dee_Decref_unlikely(ob);
			return self;
		}
		item->si_key = ob; /* Inherit reference. */
		break;
	}
	++self->rs_size;
	return self;
err_ob:
	Dee_Decref(ob);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF URoSet *DCALL
URoSet_FromIterator(DeeObject *__restrict iterator) {
	DREF DeeObject *elem;
	DREF URoSet *result;
	result = (DREF URoSet *)DeeObject_Malloc(offsetof(URoSet, rs_elem) +
	                                         1 * sizeof(struct uset_item));
	if unlikely(!result)
		goto err;
	result->rs_mask           = 0;
	result->rs_size           = 0;
	result->rs_elem[0].si_key = NULL;

	/* Insert all elements from the given iterator into the resulting set. */
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		DREF URoSet *new_result;
		new_result = URoSet_DoInsertOrRehash(result, elem);
		if unlikely(!new_result)
			goto err_r;
		result = new_result;
	}
	if unlikely(!elem)
		goto err_r;
	DeeObject_Init(result, &URoSet_Type);
	return result;
err_r:
	DeeObject_Free(result);
	{
		STATIC_ASSERT(sizeof(DREF DeeObject *) == sizeof(struct uset_item));
		Dee_XDecrefv((DREF DeeObject **)result->rs_elem,
		             result->rs_mask + 1);
	}
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF URoSet *DCALL
URoSet_FromUSet(USet *__restrict self) {
	DREF URoSet *result;
	size_t i, mask;
again_lock_hashset_src:
	USet_LockRead(self);
	mask = 1;
	while ((self->s_used & mask) != self->s_used)
		mask = (mask << 1) | 1;
	result = (DREF URoSet *)DeeObject_TryCalloc(offsetof(URoSet, rs_elem) +
	                                            (mask + 1) *
	                                            sizeof(struct uset_item));
	if unlikely(!result) {
		size_t oldsize;
		oldsize = self->s_used;
		USet_LockEndRead(self);
		result = (DREF URoSet *)DeeObject_Calloc(offsetof(URoSet, rs_elem) +
		                                         (mask + 1) *
		                                         sizeof(struct uset_item));
		if unlikely(!result)
			goto done;
		USet_LockRead(self);
		if (oldsize != self->s_used) {
			USet_LockEndRead(self);
			DeeObject_Free(result);
			goto again_lock_hashset_src;
		}
	}
	result->rs_mask = mask;
	result->rs_size = self->s_used;
	if (mask == self->s_mask) {
		for (i = 0; i <= mask; ++i) {
			DREF DeeObject *key;
			key = self->s_elem[i].si_key;
			if (key != NULL && key != dummy) {
				Dee_Incref(key);
				result->rs_elem[i].si_key = key;
			}
		}
	} else {
		for (i = 0; i <= self->s_mask; ++i) {
			DREF DeeObject *key;
			key = self->s_elem[i].si_key;
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

INTERN WUNUSED NONNULL((1)) DREF URoSet *DCALL
URoSet_FromSequence(DeeObject *__restrict sequence) {
	DREF URoSet *result;
	DeeTypeObject *type = Dee_TYPE(sequence);
	if (type == &URoSet_Type)
		return_reference_((URoSet *)sequence);
	if (type == &USet_Type)
		return URoSet_FromUSet((USet *)sequence);
	if (type == &DeeHashSet_Type) {
		size_t i, mask;
		DeeHashSetObject *src;
		src = (DeeHashSetObject *)sequence;
again_lock_hashset_src:
		DeeHashSet_LockRead(src);
		mask = 1;
		while ((src->s_used & mask) != src->s_used)
			mask = (mask << 1) | 1;
		result = (DREF URoSet *)DeeObject_TryCalloc(offsetof(URoSet, rs_elem) +
		                                            (mask + 1) *
		                                            sizeof(struct uset_item));
		if unlikely(!result) {
			size_t oldsize;
			oldsize = src->s_used;
			DeeHashSet_LockEndRead(src);
			result = (DREF URoSet *)DeeObject_Calloc(offsetof(URoSet, rs_elem) +
			                                         (mask + 1) *
			                                         sizeof(struct uset_item));
			if unlikely(!result)
				goto err;
			DeeHashSet_LockRead(src);
			if (oldsize != src->s_used) {
				DeeHashSet_LockEndRead(src);
				DeeObject_Free(result);
				goto again_lock_hashset_src;
			}
		}
		result->rs_mask = mask;
		result->rs_size = src->s_used;
		for (i = 0; i <= src->s_mask; ++i) {
			DREF DeeObject *key;
			key = src->s_elem[i].si_key;
			if (key != NULL && key != dummy) {
				Dee_Incref(key);
				URoSet_DoInsertUnlocked(result, key);
			}
		}
		DeeHashSet_LockEndRead(src);
return_result:
		DeeObject_Init(result, &URoSet_Type);
		return result;
	}
	if (type == &DeeRoSet_Type) {
		size_t i;
		DeeRoSetObject *src;
		src    = (DeeRoSetObject *)sequence;
		result = (DREF URoSet *)DeeObject_Calloc(offsetof(URoSet, rs_elem) +
		                                         (src->rs_mask + 1) *
		                                         sizeof(struct uset_item));
		if unlikely(!result)
			goto err;
		result->rs_mask = src->rs_mask;
		result->rs_size = src->rs_size;
		for (i = 0; i <= src->rs_mask; ++i) {
			DREF DeeObject *key;
			key = src->rs_elem[i].si_key;
			if (key != NULL) {
				Dee_Incref(key);
				URoSet_DoInsertUnlocked(result, key);
			}
		}
		goto return_result;
	}
	{
		/* Fallback: Try the fast-sequence interface. */
		size_t i, fastsize = DeeFastSeq_GetSize(sequence);
		if (fastsize != DEE_FASTSEQ_NOTFAST) {
			size_t mask;
			if (fastsize == 0)
				return URoSet_New();
			mask = 1;
			/* Figure out how large the mask of the set is going to be. */
			while ((fastsize & mask) != fastsize)
				mask = (mask << 1) | 1;
			result = (DREF URoSet *)DeeObject_Calloc(offsetof(URoSet, rs_elem) +
			                                         (mask + 1) *
			                                         sizeof(struct uset_item));
			if unlikely(!result)
				goto err;
			/* Without any dummy items, these are identical. */
			result->rs_mask = mask;
			result->rs_size = fastsize;
			for (i = 0; i < fastsize; ++i) {
				DREF DeeObject *key;
				key = DeeFastSeq_GetItemUnbound(sequence, i);
				if unlikely(!ITER_ISOK(key)) {
					if unlikely(key == ITER_DONE) {
						Dee_XDecrefv((DREF DeeObject **)result->rs_elem, i);
						goto err_r;
					}
					ASSERT(result->rs_size);
					--result->rs_size;
					continue;
				}
				URoSet_DoInsertUnlocked(result, key);
			}
			goto return_result;
		}
	}
	{
		/* Fallback: Use iterators. */
		DREF DeeObject *iterator;
		iterator = DeeObject_IterSelf(sequence);
		if unlikely(!iterator)
			goto err;
		result = URoSet_FromIterator(iterator);
		Dee_Decref(iterator);
		return result;
	}
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF URoSet *DCALL
uroset_deepcopy(URoSet *__restrict self) {
	DREF URoSet *result;
	size_t i;
	result = (DREF URoSet *)DeeObject_Calloc(offsetof(URoSet, rs_elem) +
	                                         (self->rs_mask + 1) *
	                                         sizeof(struct uset_item));
	if unlikely(!result)
		goto err;
	result->rs_mask = self->rs_mask;
	result->rs_size = self->rs_size;
	for (i = 0; i <= self->rs_mask; ++i) {
		DREF DeeObject *key;
		key = self->rs_elem[i].si_key;
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
	Dee_XDecrefv((DREF DeeObject **)result->rs_elem, i);
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF URoSet *DCALL
uroset_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_UniqueRoSet", &seq))
		goto err;
	return URoSet_FromSequence(seq);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
uroset_fini(URoSet *__restrict self) {
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		DREF DeeObject *key;
		key = self->rs_elem[i].si_key;
		Dee_XDecref(key);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uroset_sizeof(URoSet *self) {
	return DeeInt_NewSize(offsetof(URoSet, rs_elem) +
	                      ((self->rs_mask + 1) *
	                       sizeof(struct uset_item)));
}

PRIVATE struct type_getset tpconst uroset_getsets[] = {
	{ "frozen", &DeeObject_NewRef, NULL, NULL,
	  DOC("->?AFrozen?.\n"
	      "Simply re-return @this object") },
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uroset_sizeof, NULL, NULL,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_member tpconst uroset_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(URoSet, rs_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(URoSet, rs_size)),
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
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&URoSet_New,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&uroset_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&uroset_init
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uroset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uroset_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&uroset_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&uroset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &uroset_seq,
	/* .tp_iter_next     = */ NULL,
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
