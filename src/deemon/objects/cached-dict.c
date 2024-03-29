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
#ifndef GUARD_DEEMON_OBJECTS_CACHED_DICT_C
#define GUARD_DEEMON_OBJECTS_CACHED_DICT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/cached-dict.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/sched/yield.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

typedef DeeCachedDictObject CachedDict;

#define empty_cdict_items ((struct Dee_cached_dict_item *)DeeCachedDict_EmptyItems)



/************************************************************************/
/* CACHED DICT ITEARTOR                                                 */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *cdi_iter; /* [1..1][const] Underlying iterator. */
	DREF DeeObject *cdi_map;  /* [1..1][const] Underlying mapping. */
} CachedDictIterator;

INTDEF DeeTypeObject CachedDictIterator_Type;

PRIVATE NONNULL((1)) int DCALL
cdictiterator_ctor(CachedDictIterator *__restrict self) {
	self->cdi_map  = Dee_EmptyMapping;
	self->cdi_iter = DeeObject_IterSelf(self->cdi_map);
	if unlikely(!self->cdi_iter)
		goto err;
	Dee_Incref(self->cdi_map);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
cdictiterator_fini(CachedDictIterator *__restrict self) {
	Dee_Decref(self->cdi_iter);
	Dee_Decref(self->cdi_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
cdictiterator_visit(CachedDictIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->cdi_iter);
	Dee_Visit(self->cdi_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdictiterator_copy(CachedDictIterator *__restrict self,
                   CachedDictIterator *__restrict other) {
	self->cdi_iter = DeeObject_Copy(other->cdi_iter);
	if unlikely(!self->cdi_iter)
		goto err;
	self->cdi_map = other->cdi_map;
	Dee_Incref(self->cdi_map);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdictiterator_deep(CachedDictIterator *__restrict self,
                   CachedDictIterator *__restrict other) {
	self->cdi_iter = DeeObject_DeepCopy(other->cdi_iter);
	if unlikely(!self->cdi_iter)
		goto err;
	self->cdi_map = DeeObject_DeepCopy(other->cdi_map);
	if unlikely(!self->cdi_map)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->cdi_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cdictiterator_init(CachedDictIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o|o:CachedDict.Iterator", &self->cdi_map))
		goto err;
	self->cdi_iter = DeeObject_IterSelf(self->cdi_map);
	if unlikely(!self->cdi_iter)
		goto err;
	Dee_Incref(self->cdi_map);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cdictiterator_bool(CachedDictIterator *__restrict self) {
	return DeeObject_Bool(self->cdi_iter);
}


#define DEFINE_DICTITERATOR_COMPARE(name, Op)                                        \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                            \
	name(CachedDictIterator *self, CachedDictIterator *other) {                      \
		if (DeeObject_AssertType(other, &CachedDictIterator_Type))                   \
			goto err;                                                                \
		return_bool(DeeObject_Compare##Op##Object(self->cdi_iter, other->cdi_iter)); \
err:                                                                                 \
		return NULL;                                                                 \
	}
DEFINE_DICTITERATOR_COMPARE(cdictiterator_eq, Eq)
DEFINE_DICTITERATOR_COMPARE(cdictiterator_ne, Ne)
DEFINE_DICTITERATOR_COMPARE(cdictiterator_lo, Lo)
DEFINE_DICTITERATOR_COMPARE(cdictiterator_le, Le)
DEFINE_DICTITERATOR_COMPARE(cdictiterator_gr, Gr)
DEFINE_DICTITERATOR_COMPARE(cdictiterator_ge, Ge)
#undef DEFINE_DICTITERATOR_COMPARE


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdictiterator_next_item(CachedDictIterator *__restrict self) {
	return DeeObject_IterNext(self->cdi_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdictiterator_next_key(CachedDictIterator *__restrict self) {
	int temp;
	DREF DeeObject *item, *key_and_value[2];
	struct type_nsi const *nsi = DeeType_NSI(Dee_TYPE(self->cdi_map));
	if (nsi != NULL &&
	    nsi->nsi_class == Dee_TYPE_SEQX_CLASS_MAP &&
	    nsi->nsi_maplike.nsi_nextkey != NULL)
		return (*nsi->nsi_maplike.nsi_nextkey)(self->cdi_iter);
	item = DeeObject_IterNext(self->cdi_iter);
	if unlikely(!item)
		goto err;
	temp = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref(item);
	if unlikely(!temp)
		goto err;
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdictiterator_next_value(CachedDictIterator *__restrict self) {
	int temp;
	DREF DeeObject *item, *key_and_value[2];
	struct type_nsi const *nsi = DeeType_NSI(Dee_TYPE(self->cdi_map));
	if (nsi != NULL &&
	    nsi->nsi_class == Dee_TYPE_SEQX_CLASS_MAP &&
	    nsi->nsi_maplike.nsi_nextvalue != NULL)
		return (*nsi->nsi_maplike.nsi_nextvalue)(self->cdi_iter);
	item = DeeObject_IterNext(self->cdi_iter);
	if unlikely(!item)
		goto err;
	temp = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref(item);
	if unlikely(!temp)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}



PRIVATE struct type_cmp cdictiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdictiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdictiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdictiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdictiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdictiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdictiterator_ge
};

PRIVATE WUNUSED NONNULL((1)) DREF CachedDict *DCALL
cdictiterator_getseq(CachedDictIterator *__restrict self) {
	return (DREF CachedDict *)DeeCachedDict_New(self->cdi_map);
}

PRIVATE struct type_getset tpconst cdict_iterator_getsets[] = {
	TYPE_GETTER(STR_seq, &cdictiterator_getseq, "->?DIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cdict_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(CachedDictIterator, cdi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT, offsetof(CachedDictIterator, cdi_map), "->?DMapping"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject CachedDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedDictIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&cdictiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&cdictiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&cdictiterator_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&cdictiterator_init,
				TYPE_FIXED_ALLOCATOR(CachedDictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cdictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cdictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cdictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &cdictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cdictiterator_next_item,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cdict_iterator_getsets,
	/* .tp_members       = */ cdict_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





/************************************************************************/
/* CACHED DICT                                                          */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1)) int DCALL
cdict_ctor(CachedDict *__restrict self) {
	self->cd_mask = 0;
	self->cd_size = 0;
	self->cd_map  = Dee_EmptyMapping;
	Dee_Incref(Dee_EmptyMapping);
	self->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&self->cd_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_copy(CachedDict *__restrict self,
           CachedDict *__restrict other) {
	struct cached_dict_item *iter, *end;
	Dee_atomic_rwlock_init(&self->cd_lock);
again:
	DeeCachedDict_LockRead(other);
	self->cd_mask = other->cd_mask;
	self->cd_size = other->cd_size;
	if ((self->cd_elem = other->cd_elem) != empty_cdict_items) {
		self->cd_elem = (struct cached_dict_item *)Dee_TryMallocc(other->cd_mask + 1,
		                                                          sizeof(struct cached_dict_item));
		if unlikely(!self->cd_elem) {
			DeeCachedDict_LockEndRead(other);
			if (Dee_CollectMemory((other->cd_mask + 1) *
			                      sizeof(struct cached_dict_item)))
				goto again;
			goto err;
		}
		memcpyc(self->cd_elem, other->cd_elem,
		        self->cd_mask + 1,
		        sizeof(struct cached_dict_item));
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			Dee_Incref(iter->cdi_key);
			Dee_XIncref(iter->cdi_value);
		}
	}
	DeeCachedDict_LockEndRead(other);
	self->cd_map = other->cd_map;
	Dee_Incref(self->cd_map);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
cdict_fini(CachedDict *__restrict self) {
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));
	if (self->cd_elem != empty_cdict_items) {
		struct cached_dict_item *iter, *end;
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			Dee_Decref_unlikely(iter->cdi_key);
			Dee_XDecref_unlikely(iter->cdi_value);
		}
		Dee_Free(self->cd_elem);
	}
	Dee_Decref(self->cd_map);
}

PRIVATE NONNULL((1)) void DCALL
cdict_clear(CachedDict *__restrict self) {
	/* FIXME: CachedDict implementing "operator clear()" is a problem:
	 * - It is needed because CachedDict is a GC object
	 * - CachedDict must be a GC object because it can potentially reference itself (see code example below)
	 * - It is possible to trigger "CachedDict.operator clear()" while there are still NOREF arguments
	 * So in other words:
	 * - We get problems by implementing it (breaking the invariant that CachedDict keeps objects alive during the call)
	 * - And we'd get problems if we didn't implement it (reference loops that can't be resolved)
	 *
	 * see also: "util/test/deemon-kwcall-reference-loophole.dee"
	 */
	struct cached_dict_item *elem;
	size_t mask;
	DeeCachedDict_LockWrite(self);
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));

	/* Extract the vector and mask. */
	elem          = self->cd_elem;
	mask          = self->cd_mask;
	self->cd_elem = empty_cdict_items;
	self->cd_mask = 0;
	self->cd_size = 0;
	DeeCachedDict_LockEndWrite(self);

	/* Destroy the vector. */
	if (elem != empty_cdict_items) {
		struct cached_dict_item *iter, *end;
		end = (iter = elem) + (mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			Dee_Decref(iter->cdi_key);
			Dee_Decref(iter->cdi_value);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
cdict_visit(CachedDict *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->cd_map);
	DeeCachedDict_LockRead(self);
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));
	if (self->cd_elem != empty_cdict_items) {
		struct cached_dict_item *iter, *end;
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			/* Visit all keys and associated values. */
			Dee_Visit(iter->cdi_key);
			Dee_Visit(iter->cdi_value);
		}
	}
	DeeCachedDict_LockEndRead(self);
}


/* Resize the hash size by a factor of 2 and re-insert all elements.
 * @return: true:  Successfully rehashed the CachedDict.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
dict_rehash(CachedDict *__restrict self) {
	struct cached_dict_item *new_vector, *iter, *end;
	size_t new_mask;
	new_mask = self->cd_mask;
	new_mask = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1; /* Start out bigger than 2. */
	ASSERT(self->cd_size < new_mask);
	new_vector = (struct cached_dict_item *)Dee_TryCallocc(new_mask + 1, sizeof(struct cached_dict_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));
	if (self->cd_elem != empty_cdict_items) {
		/* Re-insert all existing items into the new CachedDict vector. */
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			struct cached_dict_item *item;
			dhash_t i, perturb;

			/* Skip dummy keys. */
			if (!iter->cdi_key)
				continue;
			perturb = i = iter->cdi_hash & new_mask;
			for (;; DeeCachedDict_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->cdi_key)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			item->cdi_key   = iter->cdi_key;
			item->cdi_hash  = iter->cdi_hash;
			item->cdi_value = iter->cdi_value;
		}
		Dee_Free(self->cd_elem);
	}
	self->cd_mask = new_mask;
	self->cd_elem = new_vector;
	return true;
}


/* Insert "value" into the cache for "key" */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
DeeCachedDict_Remember(DeeCachedDictObject *__restrict self,
                       /*inherit(always)*/ DREF DeeObject *key,
                       /*inherit(always)*/ DREF DeeObject *value, dhash_t hash) {
	size_t mask;
	struct cached_dict_item *vector;
	int error;
	struct cached_dict_item *first_dummy;
	dhash_t i, perturb;
again_lock:
	DeeCachedDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->cd_elem;
	mask        = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key;
		struct cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->cdi_key;
		Dee_Incref(item_key);
		DeeCachedDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		if (error > 0) {
			DREF DeeObject *item_value;
			/* Found an existing item. */
			DeeCachedDict_LockWrite(self);
			/* Check if the CachedDict was modified. */
			if (self->cd_elem != vector ||
			    self->cd_mask != mask ||
			    item->cdi_key != item_key) {
				DeeCachedDict_LockDowngrade(self);
				goto again;
			}
			item_value = item->cdi_value;
			item->cdi_key   = key;
			item->cdi_value = value;
			DeeCachedDict_LockEndWrite(self);
			Dee_Decref(item_key);
			Dee_Decref(item_value);
			return value;
		}
		DeeCachedDict_LockRead(self);
		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeCachedDict_LockUpgrade(self)) {
		DeeCachedDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if ((first_dummy != NULL) &&
	    (self->cd_size + 1 < self->cd_mask ||
	     first_dummy->cdi_key != NULL)) {
		ASSERT(first_dummy != empty_cdict_items);
		ASSERT(!first_dummy->cdi_key);

		/* Fill in the target slot. */
		first_dummy->cdi_key   = key; /* Inherit */
		first_dummy->cdi_hash  = hash;
		first_dummy->cdi_value = value; /* Inherit */
		++self->cd_size;

		/* Try to keep the CachedDict vector big at least twice as big as the element count. */
		if (self->cd_size * 2 > self->cd_mask)
			dict_rehash(self);
		DeeCachedDict_LockEndWrite(self);
		return value;
	}

	/* Rehash the CachedDict and try again. */
	if (dict_rehash(self)) {
		DeeCachedDict_LockDowngrade(self);
		goto again;
	}
	DeeCachedDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	Dee_Decref(key);
	Dee_Decref(value);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_LookupAndRemember(DeeCachedDictObject *__restrict self,
                                DeeObject *key, dhash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->cd_map, key);
	if likely(result) {
		Dee_Incref(key);
		result = DeeCachedDict_Remember(self, key, result, hash);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberStringHash(DeeCachedDictObject *__restrict self,
                                          char const *key, dhash_t hash) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_GetItemStringHash(basemap, key, hash);
	if likely(result) {
		DREF DeeObject *keyob;
		keyob = DeeString_NewWithHash(key, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberStringLenHash(DeeCachedDictObject *__restrict self,
                                             char const *key, size_t keylen, dhash_t hash) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_GetItemStringLenHash(basemap, key, keylen, hash);
	if likely(result) {
		DREF DeeObject *keyob;
		keyob = DeeString_NewSizedWithHash(key, keylen, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberDef(DeeCachedDictObject *__restrict self,
                                   DeeObject *key, dhash_t hash, DeeObject *def) {
	DREF DeeObject *result;
	result = DeeObject_GetItemDef(self->cd_map, key, def);
	if likely(result) {
		if (result == def) {
			if (def != ITER_DONE)
				Dee_DecrefNokill(def);
			return result;
		}
		Dee_Incref(key);
		result = DeeCachedDict_Remember(self, key, result, hash);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberStringHashDef(DeeCachedDictObject *__restrict self,
                                             char const *key, dhash_t hash, DeeObject *def) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_GetItemStringHashDef(basemap, key, hash, def);
	if likely(result) {
		DREF DeeObject *keyob;
		if (result == def) {
			if (def != ITER_DONE)
				Dee_DecrefNokill(def);
			return result;
		}
		keyob = DeeString_NewWithHash(key, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberStringLenHashDef(DeeCachedDictObject *__restrict self,
                                                char const *key, size_t keylen,
                                                dhash_t hash, DeeObject *def) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_GetItemStringLenHashDef(basemap, key, keylen, hash, def);
	if likely(result) {
		DREF DeeObject *keyob;
		if (result == def) {
			if (def != ITER_DONE)
				Dee_DecrefNokill(def);
			return result;
		}
		keyob = DeeString_NewSizedWithHash(key, keylen, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_GetItemNR(DeeCachedDictObject *self, DeeObject *key) {
	size_t mask;
	struct cached_dict_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(key);
	DeeCachedDict_LockRead(self);
restart:
	vector  = self->cd_elem;
	mask    = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key, *item_value;
		struct cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key   = item->cdi_key;
		item_value = item->cdi_value;
		DeeCachedDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		if (error > 0)
			return item_value; /* Found the item. */
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		DeeCachedDict_LockRead(self);

		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key ||
		    item->cdi_value != item_value)
			goto restart;
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRemember(self, key, hash);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_GetItemNRStringHash(DeeCachedDictObject *__restrict self,
                                  char const *__restrict key,
                                  dhash_t hash) {
	DeeObject *result;
	dhash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (!strcmp(DeeString_STR(item->cdi_key), key)) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberStringHash(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_GetItemNRStringLenHash(DeeCachedDictObject *__restrict self,
                                     char const *__restrict key,
                                     size_t keylen, dhash_t hash) {
	DeeObject *result;
	dhash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (DeeString_EqualsBuf(item->cdi_key, key, keylen)) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberStringLenHash(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
DeeCachedDict_GetItemNRDef(DeeCachedDictObject *self,
                           DeeObject *key, DeeObject *def) {
	size_t mask;
	struct cached_dict_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(key);
	DeeCachedDict_LockRead(self);
restart:
	vector  = self->cd_elem;
	mask    = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key, *item_value;
		struct cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key   = item->cdi_key;
		item_value = item->cdi_value;
		DeeCachedDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		if (error > 0)
			return item_value; /* Found the item. */
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		DeeCachedDict_LockRead(self);

		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key ||
		    item->cdi_value != item_value)
			goto restart;
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberDef(self, key, hash, def);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL
DeeCachedDict_GetItemNRStringHashDef(DeeCachedDictObject *__restrict self,
                                     char const *__restrict key,
                                     dhash_t hash, DeeObject *def) {
	DeeObject *result;
	dhash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (!strcmp(DeeString_STR(item->cdi_key), key)) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberStringHashDef(self, key, hash, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DeeObject *DCALL
DeeCachedDict_GetItemNRStringLenHashDef(DeeCachedDictObject *__restrict self,
                                        char const *__restrict key,
                                        size_t keylen, dhash_t hash,
                                        DeeObject *def) {
	DeeObject *result;
	dhash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (DeeString_EqualsBuf(item->cdi_key, key, keylen)) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberStringLenHashDef(self, key, keylen, hash, def);
}



INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeCachedDict_HasItemStringHash(DeeCachedDictObject *__restrict self,
                                char const *__restrict key,
                                dhash_t hash) {
	DeeObject *value = DeeCachedDict_GetItemNRStringHashDef(self, key, hash, ITER_DONE);
	if unlikely(!value)
		return -1;
	return value != ITER_DONE ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeCachedDict_HasItemStringLenHash(DeeCachedDictObject *__restrict self,
                                   char const *__restrict key,
                                   size_t keylen, dhash_t hash) {
	DeeObject *value = DeeCachedDict_GetItemNRStringLenHashDef(self, key, keylen, hash, ITER_DONE);
	if unlikely(!value)
		return -1;
	return value != ITER_DONE ? 1 : 0;
}

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeCachedDict_BoundItemStringHash(DeeCachedDictObject *__restrict self,
                                  char const *__restrict key,
                                  Dee_hash_t hash, bool allow_missing) {
	DeeObject *value = DeeCachedDict_GetItemNRStringHashDef(self, key, hash, ITER_DONE);
	if (value == ITER_DONE) {
		if (allow_missing)
			return -2;
		return err_unknown_key_str((DeeObject *)self, key);
	}
	if (!value) {
		if (DeeError_Catch(&DeeError_UnboundItem))
			return 0;
		if (allow_missing &&
		    (DeeError_Catch(&DeeError_IndexError) ||
		     DeeError_Catch(&DeeError_KeyError)))
			return -2;
		return -1;
	}
	return 1;
}

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeCachedDict_BoundItemStringLenHash(DeeCachedDictObject *__restrict self,
                                     char const *__restrict key, size_t keylen,
                                     Dee_hash_t hash, bool allow_missing) {
	DeeObject *value = DeeCachedDict_GetItemNRStringLenHashDef(self, key, keylen, hash, ITER_DONE);
	if (value == ITER_DONE) {
		if (allow_missing)
			return -2;
		return err_unknown_key_str_len((DeeObject *)self, key, keylen);
	}
	if (!value) {
		if (DeeError_Catch(&DeeError_UnboundItem))
			return 0;
		if (allow_missing &&
		    (DeeError_Catch(&DeeError_IndexError) ||
		     DeeError_Catch(&DeeError_KeyError)))
			return -2;
		return -1;
	}
	return 1;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_size(CachedDict *__restrict self) {
	return DeeObject_SizeObject(self->cd_map);
}

/* This one's basically your hasitem operator. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_contains(CachedDict *self, DeeObject *key) {
	DeeObject *value = DeeCachedDict_GetItemNRDef(self, key, ITER_DONE);
	if unlikely(!value)
		goto err;
	return_bool_(value != ITER_DONE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_nsi_getdefault(CachedDict *self, DeeObject *key, DeeObject *def) {
	DeeObject *result = DeeCachedDict_GetItemNRDef(self, key, def);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_getitem(CachedDict *self, DeeObject *key) {
	DeeObject *result = DeeCachedDict_GetItemNR(self, key);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cdict_nsi_getsize(CachedDict *__restrict self) {
	return DeeObject_Size(self->cd_map);
}

PRIVATE struct type_nsi tpconst cdict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&cdict_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&cdictiterator_next_key,
			/* .nsi_nextvalue  = */ (dfunptr_t)&cdictiterator_next_value,
			/* .nsi_getdefault = */ (dfunptr_t)&cdict_nsi_getdefault,
			/* .nsi_setdefault = */ (dfunptr_t)NULL,
			/* .nsi_updateold  = */ (dfunptr_t)NULL,
			/* .nsi_insertnew  = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cdict_printrepr(CachedDict *__restrict self,
                dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "CachedDict(%r)", self->cd_map);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
cdict_hash(CachedDict *__restrict self) {
	return DeeObject_Hash(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_eq(CachedDict *__restrict self, DeeObject *__restrict other) {
	return DeeObject_CompareEqObject(self->cd_map, other);
}
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_ne(CachedDict *__restrict self, DeeObject *__restrict other) {
	return DeeObject_CompareNeObject(self->cd_map, other);
}
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_lo(CachedDict *__restrict self, DeeObject *__restrict other) {
	return DeeObject_CompareLoObject(self->cd_map, other);
}
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_le(CachedDict *__restrict self, DeeObject *__restrict other) {
	return DeeObject_CompareLeObject(self->cd_map, other);
}
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_gr(CachedDict *__restrict self, DeeObject *__restrict other) {
	return DeeObject_CompareGrObject(self->cd_map, other);
}
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_ge(CachedDict *__restrict self, DeeObject *__restrict other) {
	return DeeObject_CompareGeObject(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1)) DREF CachedDictIterator *DCALL
cdict_iter(CachedDict *__restrict self) {
	DREF DeeObject *map_iter;
	DREF CachedDictIterator *result;
	result = DeeObject_MALLOC(CachedDictIterator);
	if unlikely(!result)
		goto err;
	map_iter = DeeObject_IterSelf(self->cd_map);
	if unlikely(!map_iter)
		goto err_r;
	result->cdi_iter = map_iter; /* Inherit */
	result->cdi_map  = self->cd_map;
	Dee_Incref(self->cd_map);
	DeeObject_Init(result, &CachedDictIterator_Type);
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE struct type_cmp cdict_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&cdict_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_ge,
};

PRIVATE struct type_seq cdict_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cdict_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cdict_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &cdict_nsi
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
cdict_bool(CachedDict *__restrict self) {
	return DeeObject_Bool(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cdict_init(CachedDict *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if unlikely(DeeArg_Unpack(argc, argv, "o:CachedDict", &self->cd_map))
		goto err;
	self->cd_mask = 0;
	self->cd_size = 0;
	self->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&self->cd_lock); /* Lock used for accessing this Dict. */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_get(CachedDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return DeeCachedDict_GetItemNRDef(self, key, def);
err:
	return NULL;
}

DOC_REF(map_get_doc);
PRIVATE struct type_method tpconst cdict_methods[] = {
	TYPE_METHOD_F(STR_get, &cdict_get, METHOD_FNOREFESCAPE, DOC_GET(map_get_doc)),
	TYPE_METHOD_END
};


PRIVATE struct type_member tpconst cdict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &CachedDictIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst cdict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&cdict_clear
};

PUBLIC DeeTypeObject DeeCachedDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "CachedDict",
	/* .tp_doc      = */ DOC("Cached mapping that remembers the bindings of keys as they are "
	                         /**/ "queried from an underlying ?DMapping (meaning every distinct "
	                         /**/ "key is only ever queried once in the underlying ?DMapping)\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(map:?DMapping)\n"
	                         "Create a cache mapping for @map"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&cdict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&cdict_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&cdict_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&cdict_init,
				TYPE_FIXED_ALLOCATOR_GC(CachedDict)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cdict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cdict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cdict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cdict_visit,
	/* .tp_gc            = */ &cdict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &cdict_cmp,
	/* .tp_seq           = */ &cdict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cdict_methods,
	/* .tp_getsets       = */ NULL, /* TODO: "property cache: Mapping = { get() }" -- Proxy dict that operates on the cache only (but mustn't allow removing items from the cache). */
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cdict_class_members
};


/* Construct a cached dict wrapper around "mapping" */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCachedDict_New(DeeObject *__restrict mapping) {
	DREF CachedDict *result;
	result = DeeGCObject_MALLOC(CachedDict);
	if unlikely(!result)
		goto done;
	result->cd_mask = 0;
	result->cd_size = 0;
	result->cd_map  = mapping;
	Dee_Incref(mapping);
	result->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&result->cd_lock);
	DeeObject_Init(result, &DeeCachedDict_Type);
	result = (DREF CachedDict *)DeeGC_Track((DREF DeeObject *)result);
done:
	return (DREF DeeObject *)result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CACHED_DICT_C */
