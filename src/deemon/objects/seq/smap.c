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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SMAP_C
#define GUARD_DEEMON_OBJECTS_SEQ_SMAP_C 1

#include "smap.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "svec.h"

#undef si_key
#undef si_value
#undef si_hash

DECL_BEGIN

STATIC_ASSERT(sizeof(DeeSharedItem) == 2 * sizeof(DeeObject *));


/* Assert that we can re-use components from svec-iterator. */
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(SharedMap, sm_lock) == offsetof(SharedVector, sv_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(SharedMap, sm_length) == offsetof(SharedVector, sv_length));
STATIC_ASSERT(offsetof(SharedMap, sm_vector) == offsetof(SharedVector, sv_vector));
STATIC_ASSERT(offsetof(SharedMapIterator, sm_seq) == offsetof(SharedVectorIterator, si_seq));
STATIC_ASSERT(offsetof(SharedMapIterator, sm_index) == offsetof(SharedVectorIterator, si_index));
STATIC_ASSERT(sizeof(SharedMapIterator) == sizeof(SharedVectorIterator));




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smap_nsi_nextkey(SharedMapIterator *__restrict self) {
	DREF DeeObject *result_key;
	SharedMap *map = self->sm_seq;
#ifndef CONFIG_NO_THREADS
	for (;;) {
		size_t index;
		rwlock_read(&map->sm_lock);
		index = ATOMIC_READ(self->sm_index);
		if (self->sm_index >= map->sm_length) {
			rwlock_endread(&map->sm_lock);
			return ITER_DONE;
		}
		result_key = map->sm_vector[index].si_key;
		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result_key);
		rwlock_endread(&map->sm_lock);
		if (ATOMIC_CMPXCH(self->sm_index, index, index + 1))
			break;
		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result_key);
	}
#else /* !CONFIG_NO_THREADS */
	if (self->sm_index < map->sm_length) {
		result_key = map->sm_vector[self->sm_index].si_key;
		Dee_Incref(result_key);
		++self->sm_index;
	}
#endif /* CONFIG_NO_THREADS */
	return result_key;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smap_nsi_nextvalue(SharedMapIterator *__restrict self) {
	DREF DeeObject *result_value;
	SharedMap *map = self->sm_seq;
#ifndef CONFIG_NO_THREADS
	for (;;) {
		size_t index;
		rwlock_read(&map->sm_lock);
		index = ATOMIC_READ(self->sm_index);
		if (self->sm_index >= map->sm_length) {
			rwlock_endread(&map->sm_lock);
			return ITER_DONE;
		}
		result_value = map->sm_vector[index].si_value;
		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result_value);
		rwlock_endread(&map->sm_lock);
		if (ATOMIC_CMPXCH(self->sm_index, index, index + 1))
			break;
		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result_value);
	}
#else /* !CONFIG_NO_THREADS */
	if (self->sm_index < map->sm_length) {
		result_value = map->sm_vector[self->sm_index].si_value;
		Dee_Incref(result_value);
		++self->sm_index;
	}
#endif /* CONFIG_NO_THREADS */
	return result_value;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smapiter_next(SharedMapIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *result_key, *result_value;
	SharedMap *map = self->sm_seq;
#ifndef CONFIG_NO_THREADS
	for (;;) {
		size_t index;
		rwlock_read(&map->sm_lock);
		index = ATOMIC_READ(self->sm_index);
		if (self->sm_index >= map->sm_length) {
			rwlock_endread(&map->sm_lock);
			return ITER_DONE;
		}
		result_key   = map->sm_vector[index].si_key;
		result_value = map->sm_vector[index].si_value;
		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result_key);
		Dee_Incref(result_value);
		rwlock_endread(&map->sm_lock);
		if (ATOMIC_CMPXCH(self->sm_index, index, index + 1))
			break;
		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result_value);
		Dee_Decref(result_key);
	}
#else /* !CONFIG_NO_THREADS */
	if (self->sm_index < map->sm_length) {
		result_key   = map->sm_vector[self->sm_index].si_key;
		result_value = map->sm_vector[self->sm_index].si_value;
		Dee_Incref(result_key);
		Dee_Incref(result_value);
		++self->sm_index;
	}
#endif /* CONFIG_NO_THREADS */
	/* Got the key+value. Now pack them together into a tuple. */
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result) {
		Dee_Decref(result_value);
		Dee_Decref(result_key);
		goto done;
	}
	DeeTuple_SET(result, 0, result_key);   /* Inherit reference. */
	DeeTuple_SET(result, 1, result_value); /* Inherit reference. */
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
smapiter_ctor(SharedVectorIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SharedMapIterator", &self->si_seq))
		goto err;
	if (DeeObject_AssertTypeExact(self->si_seq, &SharedMap_Type))
		goto err;
	Dee_Incref(self->si_seq);
	self->si_index = 0;
	return 0;
err:
	return -1;
}

INTDEF NONNULL((1)) void DCALL
sveciter_fini(SharedVectorIterator *__restrict self);
#define smapiter_fini sveciter_fini

INTDEF NONNULL((1, 2)) void DCALL
sveciter_visit(SharedVectorIterator *__restrict self, dvisit_t proc, void *arg);
#define smapiter_visit sveciter_visit

INTDEF WUNUSED NONNULL((1)) int DCALL
sveciter_bool(SharedVectorIterator *__restrict self);
#define smapiter_bool sveciter_bool

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
sveciter_copy(SharedVectorIterator *__restrict self,
              SharedVectorIterator *__restrict other);
#define smapiter_copy sveciter_copy

INTDEF struct type_cmp sveciter_cmp;
#define smapiter_cmp sveciter_cmp

PRIVATE struct type_member tpconst smapiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SharedMapIterator, sm_seq), "->?Ert:SharedMap"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(SharedMapIterator, sm_index)),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SharedMapIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SharedMapIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&smapiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&smapiter_ctor,
				TYPE_FIXED_ALLOCATOR(SharedMapIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&smapiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&smapiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&smapiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &smapiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&smapiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ smapiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





PRIVATE NONNULL((1)) void DCALL
smap_fini(SharedMap *__restrict self) {
	DREF DeeObject **vector;
	vector = Dee_Decrefv((DREF DeeObject **)self->sm_vector,
	                     self->sm_length * 2);
	Dee_Free(vector);
}

PRIVATE NONNULL((1, 2)) void DCALL
smap_visit(SharedMap *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visitv((DeeObject **)self->sm_vector,
	           self->sm_length * 2);
}

PRIVATE WUNUSED NONNULL((1)) DREF SharedMapIterator *DCALL
smap_iter(SharedMap *__restrict self) {
	DREF SharedMapIterator *result;
	result = DeeObject_MALLOC(SharedMapIterator);
	if unlikely(!result)
		goto done;
	result->sm_seq = self;
	Dee_Incref(self);
	result->sm_index = 0;
	DeeObject_Init(result, &SharedMapIterator_Type);
done:
	return result;
}


PRIVATE NONNULL((1, 2, 3)) void DCALL
smap_cache(SharedMap *__restrict self,
           DeeObject *__restrict key,
           DeeObject *__restrict value,
           dhash_t hash) {
	dhash_t i, perturb;
	SharedItemEx *item;
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		item = SMAP_HASHIT(self, i);
		if (item->si_key == key)
			return; /* Already cached. (possible due to race conditions) */
		if (item->si_key)
			continue;
		/* Make sure that the given `key' isn't already cached.
		 * NOTE: Because we can't be certain that `hash' is consistent
		 *       for the object (since a malicious caller may intentionally
		 *       cause different hash-values to be used), we have to manually
		 *       check the entire cache for other instances. */
		for (i = 0; i <= self->sm_mask; ++i) {
			if (self->sm_map[i].si_key == key)
				return; /* Already in cache! */
		}
		/* Setup this cache-entry as a slot for this key. */
		item->si_key   = key;
		item->si_value = value;
		item->si_hash  = hash;
#ifndef NDEBUG
		for (i = 0; i <= self->sm_mask; ++i) {
			if (!self->sm_map[i].si_key)
				goto sentinal_does_exist;
		}
		_DeeAssert_Failf("self->sm_map[*].si_key == NULL", __FILE__, __LINE__,
		                 "Shared map doesn't have a NULL-sentinel anymore");
sentinal_does_exist:
#endif /* !NDEBUG */
		break;
	}
}

PRIVATE NONNULL((1)) void DCALL
smap_mark_loaded_and_endwrite(SharedMap *__restrict self) {
	self->sm_loaded = 1;
	rwlock_endwrite(&self->sm_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
smap_contains(SharedMap *self, DeeObject *key) {
	dhash_t i, perturb, hash;
	SharedItemEx *item;
	int temp;
	bool was_loaded;
	was_loaded = self->sm_loaded != 0;
	COMPILER_READ_BARRIER();

	/* Search the hash-table. */
	hash = DeeObject_Hash(key);
again_search:
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		DREF DeeObject *item_key;
		item = SMAP_HASHIT(self, i);
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		rwlock_read(&self->sm_lock);
		item_key = item->si_key;
		if (self->sm_length == 0)
			goto endread_and_not_found;
		Dee_Incref(item_key);
		rwlock_endread(&self->sm_lock);
		temp = DeeObject_CompareEq(key, item->si_key);
		Dee_Decref(item_key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return_true;
		}
	}
	/* Find the item in the key vector. */
	rwlock_read(&self->sm_lock);
	if (self->sm_loaded) {
		if (!was_loaded) {
			rwlock_endread(&self->sm_lock);
			was_loaded = true;
			goto again_search;
		}
	} else {
		for (i = 0; i < self->sm_length; ++i) {
			DREF DeeObject *item_key, *item_value;
			dhash_t item_hash;
			item_key   = self->sm_vector[i].si_key;
			item_value = self->sm_vector[i].si_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			rwlock_endread(&self->sm_lock);
			item_hash = DeeObject_Hash(item_key);
			/* Cache the key-value pair in the hash-vector */
			rwlock_write(&self->sm_lock);
			smap_cache(self, item_key, item_value, item_hash);
			rwlock_endwrite(&self->sm_lock);
			/* Check if this is the key we're looking for. */
			Dee_Decref(item_value);
			if (item_hash == hash) {
				temp = DeeObject_CompareEq(key, item_key);
				if (temp != 0) {
					Dee_Decref(item_key);
					if unlikely(temp < 0)
						goto err;
					return_true; /* Found it! */
				}
			}
			Dee_Decref(item_key);
			rwlock_read(&self->sm_lock);
		}
		if (rwlock_tryupgrade(&self->sm_lock)) {
			smap_mark_loaded_and_endwrite(self);
			goto not_found;
		}
	}
endread_and_not_found:
	rwlock_endread(&self->sm_lock);
not_found:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
smap_getitem(SharedMap *self, DeeObject *key) {
	dhash_t i, perturb, hash;
	SharedItemEx *item;
	int temp;
	bool was_loaded;
	was_loaded = self->sm_loaded != 0;
	COMPILER_READ_BARRIER();

	/* Search the hash-table. */
	hash = DeeObject_Hash(key);
again_search:
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		item = SMAP_HASHIT(self, i);
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		rwlock_read(&self->sm_lock);
		item_key = item->si_key;
		if (self->sm_length == 0) {
			rwlock_endread(&self->sm_lock);
			goto not_found;
		}
		item_value = item->si_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		rwlock_endread(&self->sm_lock);
		temp = DeeObject_CompareEq(key, item->si_key);
		Dee_Decref(item_key);
		if (temp != 0) {
			if unlikely(temp < 0) {
				Dee_Decref(item_value);
				goto err;
			}
			return item_value;
		}
		Dee_Decref(item_value);
	}
	/* Find the item in the key vector. */
	rwlock_read(&self->sm_lock);
	if (self->sm_loaded) {
		if (!was_loaded) {
			rwlock_endread(&self->sm_lock);
			was_loaded = true;
			goto again_search;
		}
	} else {
		for (i = 0; i < self->sm_length; ++i) {
			DREF DeeObject *item_key, *item_value;
			dhash_t item_hash;
			item_key   = self->sm_vector[i].si_key;
			item_value = self->sm_vector[i].si_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			rwlock_endread(&self->sm_lock);
			item_hash = DeeObject_Hash(item_key);
			/* Cache the key-value pair in the hash-vector */
			rwlock_write(&self->sm_lock);
			smap_cache(self, item_key, item_value, item_hash);
			rwlock_endwrite(&self->sm_lock);
			/* Check if this is the key we're looking for. */
			if (item_hash == hash) {
				temp = DeeObject_CompareEq(key, item_key);
				if (temp != 0) {
					Dee_Decref(item_key);
					if unlikely(temp < 0) {
						Dee_Decref(item_value);
						goto err;
					}
					return item_value;
				}
			}
			Dee_Decref(item_value);
			Dee_Decref(item_key);
			rwlock_read(&self->sm_lock);
		}
		if (rwlock_tryupgrade(&self->sm_lock)) {
			smap_mark_loaded_and_endwrite(self);
			goto not_found;
		}
	}
	rwlock_endread(&self->sm_lock);
not_found:
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
smap_nsi_getdefault(SharedMap *self, DeeObject *key, DeeObject *defl) {
	dhash_t i, perturb, hash;
	SharedItemEx *item;
	int temp;
	bool was_loaded;
	was_loaded = self->sm_loaded != 0;
	COMPILER_READ_BARRIER();

	/* Search the hash-table. */
	hash = DeeObject_Hash(key);
again_search:
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		item = SMAP_HASHIT(self, i);
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		rwlock_read(&self->sm_lock);
		item_key = item->si_key;
		if (self->sm_length == 0) {
			rwlock_endread(&self->sm_lock);
			goto not_found;
		}
		item_value = item->si_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		rwlock_endread(&self->sm_lock);
		temp = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if (temp != 0) {
			if unlikely(temp < 0) {
				Dee_Decref(item_value);
				goto err;
			}
			return item_value;
		}
		Dee_Decref(item_value);
	}
	/* Find the item in the key vector. */
	rwlock_read(&self->sm_lock);
	if (self->sm_loaded) {
		if (!was_loaded) {
			rwlock_endread(&self->sm_lock);
			was_loaded = true;
			goto again_search;
		}
	} else {
		for (i = 0; i < self->sm_length; ++i) {
			DREF DeeObject *item_key, *item_value;
			dhash_t item_hash;
			item_key   = self->sm_vector[i].si_key;
			item_value = self->sm_vector[i].si_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			rwlock_endread(&self->sm_lock);
			item_hash = DeeObject_Hash(item_key);
			/* Cache the key-value pair in the hash-vector */
			rwlock_write(&self->sm_lock);
			smap_cache(self, item_key, item_value, item_hash);
			rwlock_endwrite(&self->sm_lock);
			/* Check if this is the key we're looking for. */
			if (item_hash == hash) {
				temp = DeeObject_CompareEq(key, item_key);
				if (temp != 0) {
					Dee_Decref(item_key);
					if unlikely(temp < 0) {
						Dee_Decref(item_value);
						goto err;
					}
					return item_value;
				}
			}
			Dee_Decref(item_value);
			Dee_Decref(item_key);
			rwlock_read(&self->sm_lock);
		}
		if (rwlock_tryupgrade(&self->sm_lock)) {
			smap_mark_loaded_and_endwrite(self);
			goto not_found;
		}
	}
	rwlock_endread(&self->sm_lock);
not_found:
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smap_get(SharedMap *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return smap_nsi_getdefault(self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smap_size(SharedMap *__restrict self) {
#ifdef CONFIG_NO_THREADS
	size_t result = self->sm_length;
#else /* CONFIG_NO_THREADS */
	size_t result = ATOMIC_READ(self->sm_length);
#endif /* !CONFIG_NO_THREADS */
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
smap_bool(SharedMap *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->sm_length != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->sm_length) != 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
smap_nsi_getsize(SharedMap *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->sm_length;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->sm_length);
#endif /* !CONFIG_NO_THREADS */
}


PRIVATE struct type_nsi tpconst smap_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&smap_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&smap_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&smap_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&smap_nsi_getdefault
		}
	}
};

PRIVATE struct type_seq smap_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&smap_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&smap_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&smap_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&smap_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &smap_nsi
};

PRIVATE struct type_member tpconst smap_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SharedMapIterator_Type),
	TYPE_MEMBER_END
};

DOC_REF(map_get_doc);

PRIVATE struct type_method tpconst smap_methods[] = {
	{ DeeString_STR(&str_get),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&smap_get,
	  DOC_GET(map_get_doc) },
	/* TODO: _SharedMap.byhash(template:?O)->?DSequence */
	{ NULL }
};

INTERN DeeTypeObject SharedMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SharedMap",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&smap_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&smap_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&smap_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &smap_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ smap_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ smap_class_members
};



/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedMap_Decref()' is called.
 * NOTE: This function implicitly inherits a reference to each item
 *       of the given vector, though does not actually inherit the
 *       vector itself! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSharedMap_NewShared(size_t length, DREF DeeSharedItem *vector) {
	DREF SharedMap *result;
	size_t mask = 0x03;
	while (length * 2 >= mask)
		mask = (mask << 1) | 1;
	result = (DREF SharedMap *)DeeObject_Calloc(SHAREDMAP_SIZEOF(mask));
	if unlikely(!result)
		goto done;
	result->sm_length = length;
	result->sm_vector = vector;
	result->sm_loaded = 0;
	result->sm_mask   = mask;
	rwlock_cinit(&result->sm_lock);
	DeeObject_Init(result, &SharedMap_Type);
done:
	return (DREF DeeObject *)result;
}

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained objects.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
PUBLIC NONNULL((1)) void DCALL
DeeSharedMap_Decref(DeeObject *__restrict self) {
	size_t count;
	DREF DeeObject **vector;
	SharedMap *me = (SharedMap *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &SharedMap_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		vector = (DREF DeeObject **)me->sm_vector;
		count  = me->sm_length * 2;
		Dee_DecrefNokill(&SharedMap_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
done_decref_vector:
		Dee_Decrefv(vector, count);
		return;
	}
	/* Difficult case: must duplicate the vector. */
	rwlock_write(&me->sm_lock);
	if (!me->sm_length) {
		me->sm_vector = NULL;
	} else {
		ASSERT(me->sm_length == me->sm_length);
		vector = (DREF DeeObject **)Dee_TryMalloc(me->sm_length * 2 *
		                                          sizeof(DREF DeeObject *));
		if unlikely(!vector)
			goto err_cannot_inherit;
		/* Simply copy all the elements, transferring
		 * all the references that they represent. */
		vector = (DREF DeeObject **)memcpyc(vector, me->sm_vector,
		                                    me->sm_length * 2,
		                                    sizeof(DREF DeeObject *));
		/* Give the SharedMap its very own copy
		 * which it will take to its grave. */
		me->sm_vector = (DeeSharedItem *)vector;
	}
	rwlock_endwrite(&me->sm_lock);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Special case: failed to create a copy that the vector may call its own. */
	vector = (DREF DeeObject **)me->sm_vector;
	count  = me->sm_length * 2;
	/* Override with an empty vector. */
	me->sm_vector = NULL;
	me->sm_length = 0;
	rwlock_endwrite(&me->sm_lock);
	Dee_Decref(me);
	/* Destroy the items that the caller wanted the vector to inherit. */
	goto done_decref_vector;
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SMAP_C */
