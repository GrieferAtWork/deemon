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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SMAP_C
#define GUARD_DEEMON_OBJECTS_SEQ_SMAP_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TryMallocc */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/map.h>                /* DeeMapping_Type, DeeSharedItem */
#include <deemon/object.h>             /* ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_*, Dee_Decref*, Dee_Incref, Dee_Movrefv, Dee_foreach_pair_t, Dee_hash_t, Dee_return_compareT, Dee_return_compare_if_ne, Dee_ssize_t, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/pair.h>               /* DeeSeqPairObject, DeeSeq_* */
#include <deemon/seq.h>                /* DeeIterator_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/system-features.h>    /* memcpyc, strcmp */
#include <deemon/type.h>               /* DeeObject_Init, DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visitv, STRUCT_OBJECT_AB, STRUCT_SIZE_T, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */
#include <deemon/util/hash.h>          /* Dee_HashCombine, Dee_HashPointer */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_cinit */

#include <hybrid/overflow.h> /* OVERFLOW_UADD */
#include <hybrid/typecore.h> /* __SIZEOF_SIZE_T__ */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "smap.h"
#include "svec.h"

#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef si_key
#undef si_value
#undef si_hash

#ifndef CONFIG_TINY_DEEMON
#define WANT_smapiter_nextkey
#define WANT_smapiter_nextvalue
#define WANT_smapiter_next
#endif /* !CONFIG_TINY_DEEMON */

DECL_BEGIN

STATIC_ASSERT(sizeof(DeeSharedItem) == 2 * sizeof(DeeObject *));


/* Assert that we can re-use components from svec-iterator. */
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(SharedMap, sm_lock) == offsetof(SharedVector, sv_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(SharedMap, sm_length) == offsetof(SharedVector, sv_length));
STATIC_ASSERT(offsetof(SharedMap, sm_vector) == offsetof(SharedVector, sv_vector));




#ifdef WANT_smapiter_nextkey
#define PTR_smapiter_nextkey &smapiter_nextkey
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smapiter_nextkey(SharedMapIterator *__restrict self) {
	DREF DeeObject *result_key;
	SharedMap *map = self->smi_seq;
	for (;;) {
		size_t index;
		SharedMap_LockRead(map);
		index = atomic_read(&self->smi_index);
		if (self->smi_index >= map->sm_length) {
			SharedMap_LockEndRead(map);
			return ITER_DONE;
		}
		result_key = map->sm_vector[index].si_key;

		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result_key);
		SharedMap_LockEndRead(map);
		if (atomic_cmpxch_weak_or_write(&self->smi_index, index, index + 1))
			break;

		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result_key);
	}
	return result_key;
}
#else /* WANT_smapiter_nextkey */
#define PTR_smapiter_nextkey DEFIMPL(&default__nextkey__with__nextpair)
#endif /* !WANT_smapiter_nextkey */

#ifdef WANT_smapiter_nextvalue
#define PTR_smapiter_nextvalue &smapiter_nextvalue
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smapiter_nextvalue(SharedMapIterator *__restrict self) {
	DREF DeeObject *result_value;
	SharedMap *map = self->smi_seq;
	for (;;) {
		size_t index;
		SharedMap_LockRead(map);
		index = atomic_read(&self->smi_index);
		if (self->smi_index >= map->sm_length) {
			SharedMap_LockEndRead(map);
			return ITER_DONE;
		}
		result_value = map->sm_vector[index].si_value;

		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result_value);
		SharedMap_LockEndRead(map);
		if (atomic_cmpxch_weak_or_write(&self->smi_index, index, index + 1))
			break;

		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result_value);
	}
	return result_value;
}
#else /* WANT_smapiter_nextvalue */
#define PTR_smapiter_nextvalue DEFIMPL(&default__nextvalue__with__nextpair)
#endif /* !WANT_smapiter_nextvalue */

#ifdef WANT_smapiter_next
#define PTR_smapiter_next &smapiter_next
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
smapiter_next(SharedMapIterator *__restrict self) {
	SharedMap *map = self->smi_seq;
	DREF DeeObject *result_key_and_value[2];
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	for (;;) {
		size_t index;
		SharedMap_LockRead(map);
		index = atomic_read(&self->smi_index);
		if (self->smi_index >= map->sm_length) {
			SharedMap_LockEndRead(map);
			DeeSeq_FreePairUninitialized(result);
			return ITER_DONE;
		}
		/* Acquire a reference to keep the item alive. */
		result_key_and_value[0] = map->sm_vector[index].si_key;
		Dee_Incref(result_key_and_value[0]);
		result_key_and_value[1] = map->sm_vector[index].si_value;
		Dee_Incref(result_key_and_value[1]);
		SharedMap_LockEndRead(map);
		if (atomic_cmpxch_weak_or_write(&self->smi_index, index, index + 1))
			break;

		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result_key_and_value[1]);
		Dee_Decref(result_key_and_value[0]);
	}

	/* Got the key+value. Now pack them together into a pair. */
	return DeeSeq_InitPairvInherited(result, result_key_and_value);
err:
	return NULL;
}
#else /* WANT_smapiter_next */
#define PTR_smapiter_next DEFIMPL(&default__iter_next__with__nextpair)
#endif /* !WANT_smapiter_next */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
smapiter_nextpair(SharedMapIterator *__restrict self,
                  DREF DeeObject *key_and_value[2]) {
	SharedMap *map = self->smi_seq;
	for (;;) {
		size_t index;
		SharedMap_LockRead(map);
		index = atomic_read(&self->smi_index);
		if (self->smi_index >= map->sm_length) {
			SharedMap_LockEndRead(map);
			return 1;
		}
		key_and_value[0] = map->sm_vector[index].si_key;
		key_and_value[1] = map->sm_vector[index].si_value;

		/* Acquire a reference to keep the item alive. */
		Dee_Incref(key_and_value[0]);
		Dee_Incref(key_and_value[1]);
		SharedMap_LockEndRead(map);
		if (atomic_cmpxch_weak_or_write(&self->smi_index, index, index + 1))
			break;

		/* If some other thread stole the index, drop their value. */
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
smapiter_advance(SharedMapIterator *__restrict self, size_t skip) {
	size_t index, new_index;
	SharedMap *map = self->smi_seq;
	do {
		index = atomic_read(&self->smi_index);
		new_index = index + skip;
		if (OVERFLOW_UADD(index, skip, &new_index))
			new_index = (size_t)-1;
		SharedMap_LockRead(map);
		if (new_index >= map->sm_length)
			new_index = map->sm_length;
		SharedMap_LockEndRead(map);
	} while (!atomic_cmpxch_weak_or_write(&self->smi_index, index, index + 1));
	ASSERT(new_index >= index);
	ASSERT((new_index - index) != (size_t)-1);
	return new_index - index;
}

PRIVATE struct type_iterator smapiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&smapiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))PTR_smapiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))PTR_smapiter_nextvalue,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
smapiter_ctor(SharedMapIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_SharedMapIterator", &self->smi_seq);
	if (DeeObject_AssertTypeExact(self->smi_seq, &DeeSharedMap_Type))
		goto err;
	Dee_Incref(self->smi_seq);
	self->smi_index = 0;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(SharedMapIterator, smi_seq) == offsetof(ProxyObject, po_obj));
#define smapiter_fini      generic_proxy__fini
#define smapiter_visit     generic_proxy__visit
#define smapiter_serialize generic_proxy__serialize_and_wordcopy_atomic(__SIZEOF_SIZE_T__)

PRIVATE WUNUSED NONNULL((1)) int DCALL
smapiter_bool(SharedMapIterator *__restrict self) {
	return (atomic_read(&self->smi_index) <
	        atomic_read(&self->smi_seq->sm_length));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
smapiter_copy(SharedMapIterator *__restrict self,
              SharedMapIterator *__restrict other) {
	self->smi_index = atomic_read(&other->smi_index);
	self->smi_seq = other->smi_seq;
	Dee_Incref(self->smi_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
smapiter_hash(SharedMapIterator *self) {
	return Dee_HashCombine(Dee_HashPointer(self->smi_seq),
	                       atomic_read(&self->smi_index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
smapiter_compare(SharedMapIterator *self, SharedMapIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedMapIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->smi_seq, other->smi_seq);
	Dee_return_compareT(size_t, atomic_read(&self->smi_index),
	                    /*   */ atomic_read(&other->smi_index));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp smapiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&smapiter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&smapiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_method tpconst smapiter_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst smapiter_method_hints[] = {
	TYPE_METHOD_HINT(iter_advance, &smapiter_advance),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst smapiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(SharedMapIterator, smi_seq), "->?Ert:SharedMap"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(SharedMapIterator, smi_index)),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SharedMapIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &smapiter_copy,
			/* tp_any_ctor:    */ &smapiter_ctor,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &smapiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&smapiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&smapiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&smapiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &smapiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))PTR_smapiter_next,
	/* .tp_iterator      = */ &smapiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ smapiter_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ smapiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ smapiter_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};





PRIVATE NONNULL((1)) void DCALL
smap_fini(SharedMap *__restrict self) {
	DREF DeeObject **vector;
	vector = Dee_Decrefv((DREF DeeObject **)self->sm_vector,
	                     self->sm_length * 2);
	Dee_Free(vector);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
smap_serialize(SharedMap *__restrict self,
               DeeSerial *__restrict writer) {
#define ADDROF(field) (out_addr + offsetof(SharedMap, field))
	SharedMap *out;
	size_t self__sm_length;
	Dee_seraddr_t out_addr;
	Dee_seraddr_t out__sm_vector;
	size_t sizeof_self = SHAREDMAP_SIZEOF(self->sm_mask);
	DeeSharedItem *ou__sm_vector;
	out_addr = DeeSerial_ObjectCalloc(writer, sizeof_self, self);
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
again_read:
	SharedMap_LockRead(self);
	self__sm_length = self->sm_length;
	out__sm_vector  = DeeSerial_TryMalloc(writer, self__sm_length * sizeof(DeeSharedItem), NULL);
	if (!Dee_SERADDR_ISOK(out__sm_vector)) {
		SharedMap_LockEndRead(self);
		out__sm_vector = DeeSerial_Malloc(writer, self__sm_length * sizeof(DeeSharedItem), NULL);
		if (!Dee_SERADDR_ISOK(out__sm_vector))
			goto err;
		SharedMap_LockRead(self);
		if unlikely(self__sm_length != self->sm_length) {
			SharedMap_LockEndRead(self);
			DeeSerial_Free(writer, out__sm_vector, NULL);
			goto again_read;
		}
	}
	ou__sm_vector = DeeSerial_Addr2Mem(writer, out__sm_vector, DeeSharedItem);
	Dee_Movrefv((DeeObject **)ou__sm_vector,
	            (DeeObject *const *)self->sm_vector,
	            self__sm_length * 2);
	SharedMap_LockEndRead(self);
	if (DeeSerial_InplacePutObjectv(writer, out__sm_vector, self__sm_length * 2))
		goto err;
	if (DeeSerial_PutAddr(writer, ADDROF(sm_vector), out__sm_vector))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, SharedMap);
	out->sm_length = self__sm_length;
	Dee_atomic_rwlock_cinit(&out->sm_lock);
	ASSERT(out->sm_loaded == 0);
	out->sm_mask = self->sm_mask;
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
}

PRIVATE NONNULL((1, 2)) void DCALL
smap_visit(SharedMap *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visitv((DeeObject **)self->sm_vector,
	           self->sm_length * 2);
}

PRIVATE WUNUSED NONNULL((1)) DREF SharedMapIterator *DCALL
smap_iter(SharedMap *__restrict self) {
	DREF SharedMapIterator *result;
	result = DeeObject_MALLOC(SharedMapIterator);
	if unlikely(!result)
		goto done;
	result->smi_seq = self;
	Dee_Incref(self);
	result->smi_index = 0;
	DeeObject_Init(result, &SharedMapIterator_Type);
done:
	return result;
}


PRIVATE NONNULL((1, 2, 3)) void DCALL
smap_cache(SharedMap *self, DeeObject *key,
           DeeObject *value, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
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
	SharedMap_LockEndWrite(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
smap_trygetitem(SharedMap *self, DeeObject *key) {
	Dee_hash_t i, perturb, hash;
	SharedItemEx *item;
	bool was_loaded;
	was_loaded = self->sm_loaded != 0;
	COMPILER_READ_BARRIER();

	/* Search the hash-table. */
	hash = DeeObject_Hash(key);
again_search:
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		int temp;
		DREF DeeObject *item_key, *item_value;
		item = SMAP_HASHIT(self, i);
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		SharedMap_LockRead(self);
		item_key = item->si_key;
		if (self->sm_length == 0) {
			SharedMap_LockEndRead(self);
			goto not_found;
		}
		item_value = item->si_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		SharedMap_LockEndRead(self);
		temp = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (Dee_COMPARE_ISEQ_NO_ERR(temp))
			return item_value;
		Dee_Decref(item_value);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
	}

	/* Find the item in the key vector. */
	SharedMap_LockRead(self);
	if (self->sm_loaded) {
		if (!was_loaded) {
			SharedMap_LockEndRead(self);
			was_loaded = true;
			goto again_search;
		}
	} else {
		for (i = 0; i < self->sm_length; ++i) {
			DREF DeeObject *item_key, *item_value;
			Dee_hash_t item_hash;
			item_key   = self->sm_vector[i].si_key;
			item_value = self->sm_vector[i].si_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			SharedMap_LockEndRead(self);
			item_hash = DeeObject_Hash(item_key);

			/* Cache the key-value pair in the hash-vector */
			SharedMap_LockWrite(self);
			smap_cache(self, item_key, item_value, item_hash);
			SharedMap_LockEndWrite(self);

			/* Check if this is the key we're looking for. */
			if (item_hash == hash) {
				int temp = DeeObject_TryCompareEq(key, item_key);
				Dee_Decref(item_key);
				if (Dee_COMPARE_ISEQ_NO_ERR(temp))
					return item_value;
				Dee_Decref(item_value);
				if (Dee_COMPARE_ISERR(temp))
					goto err;
			} else {
				Dee_Decref(item_key);
				Dee_Decref(item_value);
			}
			SharedMap_LockRead(self);
		}
		if (SharedMap_LockTryUpgrade(self)) {
			smap_mark_loaded_and_endwrite(self);
			goto not_found;
		}
	}
	SharedMap_LockEndRead(self);
not_found:
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
smap_size(SharedMap *__restrict self) {
	return atomic_read(&self->sm_length);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
smap_bool(SharedMap *__restrict self) {
	return atomic_read(&self->sm_length) != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
smap_foreach(SharedMap *self, Dee_foreach_pair_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	SharedMap_LockRead(self);
	for (i = 0; i < self->sm_length; ++i) {
		DREF DeeObject *key, *value;
		key   = self->sm_vector[i].si_key;
		value = self->sm_vector[i].si_value;
		Dee_Incref(key);
		Dee_Incref(value);
		SharedMap_LockEndRead(self);
		temp = (*proc)(arg, key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		SharedMap_LockRead(self);
	}
	SharedMap_LockEndRead(self);
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
smap_trygetitem_string_hash(SharedMap *self, char const *key, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	SharedItemEx *item;
	bool was_loaded;
	was_loaded = self->sm_loaded != 0;
	COMPILER_READ_BARRIER();

	/* Search the hash-table. */
again_search:
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		item = SMAP_HASHIT(self, i);
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		SharedMap_LockRead(self);
		item_key = item->si_key;
		if (self->sm_length == 0) {
			SharedMap_LockEndRead(self);
			goto not_found;
		}
		if (!DeeString_Check(item_key) ||
		    strcmp(DeeString_STR(item_key), key) != 0) {
			SharedMap_LockEndRead(self);
			continue;
		}
		item_value = item->si_value;
		Dee_Incref(item_value);
		SharedMap_LockEndRead(self);
		return item_value;
	}

	/* Find the item in the key vector. */
	SharedMap_LockRead(self);
	if (self->sm_loaded) {
		if (!was_loaded) {
			SharedMap_LockEndRead(self);
			was_loaded = true;
			goto again_search;
		}
	} else {
		for (i = 0; i < self->sm_length; ++i) {
			DREF DeeObject *item_key, *item_value;
			Dee_hash_t item_hash;
			item_key   = self->sm_vector[i].si_key;
			item_value = self->sm_vector[i].si_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			SharedMap_LockEndRead(self);
			item_hash = DeeObject_Hash(item_key);

			/* Cache the key-value pair in the hash-vector */
			SharedMap_LockWrite(self);
			smap_cache(self, item_key, item_value, item_hash);
			SharedMap_LockEndWrite(self);

			/* Check if this is the key we're looking for. */
			if (item_hash == hash &&
			    DeeString_Check(item_key) &&
			    strcmp(DeeString_STR(item_key), key) == 0) {
				Dee_Decref(item_key);
				return item_value;
			}
			Dee_Decref(item_value);
			Dee_Decref(item_key);
			SharedMap_LockRead(self);
		}
		if (SharedMap_LockTryUpgrade(self)) {
			smap_mark_loaded_and_endwrite(self);
			goto not_found;
		}
	}
	SharedMap_LockEndRead(self);
not_found:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
smap_trygetitem_string_len_hash(SharedMap *self, char const *key, size_t keylen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	SharedItemEx *item;
	bool was_loaded;
	was_loaded = self->sm_loaded != 0;
	COMPILER_READ_BARRIER();

	/* Search the hash-table. */
again_search:
	perturb = i = SMAP_HASHST(self, hash);
	for (;; SMAP_HASHNX(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		item = SMAP_HASHIT(self, i);
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		SharedMap_LockRead(self);
		item_key = item->si_key;
		if (self->sm_length == 0) {
			SharedMap_LockEndRead(self);
			goto not_found;
		}
		if (!DeeString_Check(item_key) ||
		    !DeeString_EqualsBuf(item_key, key, keylen)) {
			SharedMap_LockEndRead(self);
			continue;
		}
		item_value = item->si_value;
		Dee_Incref(item_value);
		SharedMap_LockEndRead(self);
		return item_value;
	}

	/* Find the item in the key vector. */
	SharedMap_LockRead(self);
	if (self->sm_loaded) {
		if (!was_loaded) {
			SharedMap_LockEndRead(self);
			was_loaded = true;
			goto again_search;
		}
	} else {
		for (i = 0; i < self->sm_length; ++i) {
			DREF DeeObject *item_key, *item_value;
			Dee_hash_t item_hash;
			item_key   = self->sm_vector[i].si_key;
			item_value = self->sm_vector[i].si_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			SharedMap_LockEndRead(self);
			item_hash = DeeObject_Hash(item_key);

			/* Cache the key-value pair in the hash-vector */
			SharedMap_LockWrite(self);
			smap_cache(self, item_key, item_value, item_hash);
			SharedMap_LockEndWrite(self);

			/* Check if this is the key we're looking for. */
			if (item_hash == hash &&
			    DeeString_Check(item_key) &&
			    DeeString_EqualsBuf(item_key, key, keylen)) {
				Dee_Decref(item_key);
				return item_value;
			}
			Dee_Decref(item_value);
			Dee_Decref(item_key);
			SharedMap_LockRead(self);
		}
		if (SharedMap_LockTryUpgrade(self)) {
			smap_mark_loaded_and_endwrite(self);
			goto not_found;
		}
	}
	SharedMap_LockEndRead(self);
not_found:
	return ITER_DONE;
}


PRIVATE struct type_seq smap_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&smap_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__map_operator_contains__with__map_operator_trygetitem),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__trygetitem),
	/* .tp_delitem                    = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&smap_foreach,
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__trygetitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&smap_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&smap_size,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&smap_trygetitem,
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&smap_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__trygetitem_string_hash),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__trygetitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&smap_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__trygetitem_string_len_hash),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE struct type_getset tpconst smap_getsets[] = {
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst smap_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SharedMapIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeSharedMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SharedMap",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &smap_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&smap_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&smap_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&smap_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &smap_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO: _SharedMap.byhash(template:?O)->?DSequence */
	/* .tp_getsets       = */ smap_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ smap_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



/* Create a new shared map that will inherit elements from
 * the given vector once `DeeSharedMap_Decref()' is called.
 * NOTE: This function can implicitly inherit a reference to each item of the
 *       given vector, though does not actually inherit the vector itself:
 *       - DeeSharedMap_Decref:            The `vector' arg here is `DREF DeeSharedItem *const *'
 *       - DeeSharedMap_DecrefNoGiftItems: The `vector' arg here is `DeeSharedItem *const *'
 * NOTE: Do NOT free the given `vector' before calling `DeeSharedMap_Decref'
 *       on the returned object, as `vector' will be shared with it until
 *       that point in time! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSharedMap_NewShared(size_t length, /*inherit(maybe)*/ DREF DeeSharedItem const *vector) {
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
	Dee_atomic_rwlock_cinit(&result->sm_lock);
	DeeObject_Init(result, &DeeSharedMap_Type);
done:
	return Dee_AsObject(result);
}

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `vector',
 * as passed to `DeeSharedMap_NewShared()', but still decref()
 * all contained object.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sskv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedMap object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_MAP' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
PUBLIC NONNULL((1)) void DCALL
DeeSharedMap_Decref(DeeObject *__restrict self) {
	size_t length;
	DREF DeeSharedItem const *vector;
	DREF DeeSharedItem *vector_copy;
	SharedMap *me = (SharedMap *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSharedMap_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		Dee_Decrefv((DREF DeeObject **)me->sm_vector, me->sm_length * 2);
		Dee_DecrefNokill(&DeeSharedMap_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}

	/* Difficult case: must duplicate the vector. */
	SharedMap_LockWrite(me);
	vector_copy = (DREF DeeSharedItem *)Dee_TryMallocc(me->sm_length * 2,
	                                                   sizeof(DREF DeeSharedItem *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;

	/* Simply copy all the elements, transferring
	 * all the references that they represent. */
	vector_copy = (DREF DeeSharedItem *)memcpyc(vector_copy, me->sm_vector,
	                                            me->sm_length * 2,
	                                            sizeof(DREF DeeObject *));

	/* Give the SharedMap its very own copy
	 * which it will take to its grave. */
	me->sm_vector = vector_copy;
	SharedMap_LockEndWrite(me);
	Dee_Decref_unlikely(me);
	return;

err_cannot_inherit:
	/* Special case: failed to create a copy that the vector may call its own. */
	vector = me->sm_vector;
	length = me->sm_length;

	/* Override with an empty vector. */
	me->sm_vector = NULL;
	me->sm_length = 0;
	SharedMap_LockEndWrite(me);

	/* Destroy the items that the caller wanted the vector to inherit. */
	Dee_Decrefv((DREF DeeObject **)vector, length * 2);
	Dee_Decref_unlikely(me);
}

/* Same as `DeeSharedMap_Decref()', but should be used if the caller
 * does *not* want to gift the vector references to all of its items. */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedMap_DecrefNoGiftItems(DREF DeeObject *__restrict self) {
	DREF DeeSharedItem *vector_copy;
	SharedMap *me = (SharedMap *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSharedMap_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		Dee_DecrefNokill(&DeeSharedMap_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}

	/* Difficult case: must duplicate the vector. */
	SharedMap_LockWrite(me);
	vector_copy = (DREF DeeSharedItem *)Dee_TryMallocc(me->sm_length,
	                                                   sizeof(DREF DeeSharedItem *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;

	/* Simply copy all the elements, transferring
	 * all the references that they represent. */
	vector_copy = (DREF DeeSharedItem *)Dee_Movrefv((DREF DeeObject **)vector_copy,
	                                                (DREF DeeObject **)me->sm_vector,
	                                                me->sm_length * 2);

	/* Give the SharedMap its very own copy
	 * which it will take to its grave. */
	me->sm_vector = vector_copy;
	SharedMap_LockEndWrite(me);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Override with an empty vector. */
	me->sm_vector = NULL;
	me->sm_length = 0;
	SharedMap_LockEndWrite(me);
	Dee_Decref(me);
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SMAP_C */
