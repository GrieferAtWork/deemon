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
#ifndef GUARD_DEEMON_RUNTIME_GC_INSPECT_C_INL
#define GUARD_DEEMON_RUNTIME_GC_INSPECT_C_INL 1
#define DEE_SOURCE

#include "gc.c"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, Dee_TryCallocc */
#include <deemon/arg.h>                /* DeeArg_Unpack*, UNPxSIZ */
#include <deemon/bool.h>               /* return_false, return_true */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/gc.h>                 /* DeeGC_*, Dee_gc_head */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/object.h>             /* DeeObject_*, Dee_Decref*, Dee_Incref, Dee_XDecrefv_unlikely, return_reference */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_IsGC, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, Dee_Visit, Dee_XVisit, Dee_XVisitv, Dee_visit_t, METHOD_FNOREFESCAPE, TF_NONE, TF_SINGLETON, TP_F*, TYPE_*, type_* */
#include <deemon/types.h>              /* DREF, DeeObject, DeeTypeObject, DeeType_Extends, Dee_AsObject, Dee_TYPE, Dee_foreach_t, Dee_hash_t, Dee_ssize_t, ITER_DONE, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */
#include <deemon/util/hash.h>          /* DeeObject_HashGeneric */
#include <deemon/util/lock.h>          /* Dee_atomic_lock_* */
#include <deemon/util/once.h>          /* Dee_once_* */

#include "../objects/generic-proxy.h"
#include "kwlist.h"
#include "method-hint-defaults.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

#undef container_of
#define container_of COMPILER_CONTAINER_OF

#if defined(CONFIG_EXPERIMENTAL_REWORKED_GC) || defined(__DEEMON__)
DECL_BEGIN

struct gcset_item {
	DREF DeeObject *gsi_item; /* [0..1] Referenced object (or "NULL" if unused slot) */
};

typedef struct {
	struct gcset_item  *gs_map; /* [0..gs_msk+1][owned] Map of referenced objects, or "NULL" if not-yet-allocated */
	Dee_hash_t          gs_msk; /* Hash-mask for `gs_map' */
	size_t              gs_siz; /* # of non-NULL items in `gs_map', or `(size_t)-1' after an error */
} GCSet;

#define GCSet_HashSt(self, hash)  ((hash) & (self)->gs_msk)
#define GCSet_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define GCSet_HashIt(self, i)     ((self)->gs_map + ((i) & (self)->gs_msk))

#define GCSet_Init(self) \
	((self)->gs_map = NULL, (self)->gs_msk = (self)->gs_siz = 0)
#define GCSet_FiniMarkers(self) Dee_Free((self)->gs_map)
PRIVATE NONNULL((1)) void DCALL
GCSet_Fini(GCSet *__restrict self) {
	struct gcset_item *map = self->gs_map;
	if (map) {
		Dee_XDecrefv_unlikely((DREF DeeObject **)map, self->gs_msk + 1);
		Dee_Free(map);
	}
}

PRIVATE NONNULL((1)) bool DCALL
GCSet_TryRehash(GCSet *__restrict self) {
	size_t i;
	Dee_hash_t oldmsk = self->gs_msk;
	Dee_hash_t minmsk = (oldmsk << 1) | 1;
	Dee_hash_t newmsk = minmsk >= 31 ? minmsk : 31;
	struct gcset_item *newmap;
	struct gcset_item *oldmap;
	newmap = (struct gcset_item *)Dee_TryCallocc(newmsk + 1, sizeof(struct gcset_item));
	if unlikely(!newmap) {
		newmsk = minmsk;
		newmap = (struct gcset_item *)Dee_TryCallocc(newmsk + 1, sizeof(struct gcset_item));
		if unlikely(!newmap) {
			self->gs_siz = (size_t)-1;
			return false;
		}
	}
	oldmap = self->gs_map;
	self->gs_map = newmap;
	self->gs_msk = newmsk;
	if (oldmap) {
		for (i = 0; i <= oldmsk; ++i) {
			DREF DeeObject *ob = oldmap[i].gsi_item;
			if (ob) {
				Dee_hash_t j, perturb, hash = DeeObject_HashGeneric(ob);
				perturb = j = GCSet_HashSt(self, hash);
				for (;; GCSet_HashNx(j, perturb)) {
					struct gcset_item *item = GCSet_HashIt(self, j);
					if (!item->gsi_item) {
						item->gsi_item = ob;
						break;
					}
				}
			}
		}
		Dee_Free(oldmap);
	}
	return true;
}

/* Ensure that there is space for 1 more item
 * @return: 0 : Success
 * @return: -1: Failure: insufficient memory (no error was thrown) */
PRIVATE NONNULL((1)) int DCALL
GCSet_TryInsertPrepare(GCSet *__restrict self) {
	if unlikely(self->gs_siz >= (self->gs_msk * 2) / 3) {
		if unlikely(self->gs_siz == (size_t)-1)
			goto err; /* Preceding error */
		if (!GCSet_TryRehash(self)) {
			if unlikely(self->gs_siz >= self->gs_msk)
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

#if 0
PRIVATE WUNUSED NONNULL((1)) bool DCALL
GCSet_Contains(GCSet *self, DeeObject *ob) {
	Dee_hash_t i, perturb, hash = DeeObject_HashGeneric(ob);
	perturb = i = GCSet_HashSt(self, hash);
	for (;; GCSet_HashNx(i, perturb)) {
		struct gcset_item *item = GCSet_HashIt(self, i);
		if (!item->gsi_item)
			break;
		if (item->gsi_item == ob)
			return true;
	}
	return false;
}
#endif

PRIVATE NONNULL((1)) void DCALL
GCSet_DoInsertInherited(GCSet *self, /*inherit(always)*/ DREF DeeObject *ob) {
	Dee_hash_t i, perturb, hash = DeeObject_HashGeneric(ob);
	ASSERT(self->gs_siz < self->gs_msk);
	perturb = i = GCSet_HashSt(self, hash);
	for (;; GCSet_HashNx(i, perturb)) {
		struct gcset_item *item = GCSet_HashIt(self, i);
		if (!item->gsi_item) {
			item->gsi_item = ob; /* Inherit reference */
			++self->gs_siz;
			ASSERT(self->gs_siz != (size_t)-1);
			break;
		}
		ASSERT(item->gsi_item != ob);
	}
}

/* @return: 0 : Success
 * @return: 1 : Failure: object was already present
 * @return: -1: Failure: insufficient memory (no error was thrown) */
PRIVATE NONNULL((1)) int DCALL
GCSet_TryInsert(GCSet *self, DeeObject *ob) {
	Dee_hash_t i, perturb, hash = DeeObject_HashGeneric(ob);
	if unlikely(GCSet_TryInsertPrepare(self))
		goto err;
	perturb = i = GCSet_HashSt(self, hash);
	for (;; GCSet_HashNx(i, perturb)) {
		struct gcset_item *item = GCSet_HashIt(self, i);
		if (!item->gsi_item) {
			Dee_Incref(ob);
			item->gsi_item = ob;
			++self->gs_siz;
			ASSERT(self->gs_siz != (size_t)-1);
			return 0;
		}
		if (item->gsi_item == ob)
			break;
	}
	return 1;
err:
	return -1;
}

/* @return:  1: true-marker
 * @return:  0: false-marker
 * @return: -1: no marker */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
GCSet_GetMarker(GCSet *self, DeeObject *ob) {
	Dee_hash_t i, perturb, hash = DeeObject_HashGeneric(ob);
	perturb = i = GCSet_HashSt(self, hash);
	for (;; GCSet_HashNx(i, perturb)) {
		struct gcset_item *item = GCSet_HashIt(self, i);
		if (!item->gsi_item)
			break;
		if (((uintptr_t)item->gsi_item & ~1) == (uintptr_t)ob)
			return (uintptr_t)item->gsi_item & 1;
	}
	return -1;
}

/* @return: 1 : Already present
 * @return: 0 : Success, or already present
 * @return: -1: OOM failure (no error was thrown) */
PRIVATE NONNULL((1, 2)) int DCALL
GCSet_SetMarker(GCSet *self, DeeObject *ob, bool marker) {
	Dee_hash_t i, perturb, hash = DeeObject_HashGeneric(ob);
	ASSERT((uintptr_t)marker == 0 || (uintptr_t)marker == 1);
	if unlikely(GCSet_TryInsertPrepare(self))
		goto err;
	perturb = i = GCSet_HashSt(self, hash);
	for (;; GCSet_HashNx(i, perturb)) {
		struct gcset_item *item = GCSet_HashIt(self, i);
		if (!item->gsi_item) {
			item->gsi_item = (DeeObject *)((uintptr_t)ob | (uintptr_t)marker);
			++self->gs_siz;
			return 0;
		}
		if (((uintptr_t)item->gsi_item & ~1) == (uintptr_t)ob)
			break;
	}
	return 1;
err:
	return -1;
}




typedef struct gc_collection GCCollection;
typedef void (DCALL *gcset_populate_cb_t)(GCCollection *self);

struct gc_collection {
	PROXY_OBJECT_HEAD  (gcc_obj);     /* [1..1][const] The object whose reachable set is being described. */
	gcset_populate_cb_t gcc_populate; /* [1..1][const] Cache population function */
	GCSet               gcc_cache;    /* [const_if(Dee_once_hasrun(&gcc_loaded))] Cache of reachable objects (loaded lazily if needed for sequence operations) */
	Dee_once_t          gcc_loaded;   /* Is-loaded controller for `gcc_cache' */
	/* NOTE: `gcc_populate' should be one of:
	 * - GCCollection_PopulateReachableDirect
	 * - GCCollection_PopulateReachableTransitive
	 * - GCCollection_PopulateReferringDirect
	 * - GCCollection_PopulateReferringTransitive */
};

INTDEF DeeTypeObject GCCollection_Type;

/* Ensure that "self->gcc_cache" has been populated.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1)) int DCALL
gccoll_loadcache(GCCollection *__restrict self);

#define gccoll_loaded(self) Dee_once_hasrun(&(self)->gcc_loaded)

/* Create a new GC collection */
PRIVATE WUNUSED NONNULL((1, 2)) DREF GCCollection *DCALL
GCCollection_New(DeeObject *__restrict ob, gcset_populate_cb_t populate) {
	DREF GCCollection *result = DeeObject_MALLOC(GCCollection);
	if unlikely(!result)
		goto err;
	Dee_Incref(ob);
	result->gcc_obj      = ob;
	result->gcc_populate = populate;
	GCSet_Init(&result->gcc_cache);
	Dee_once_init(&result->gcc_loaded);
	DeeObject_Init(result, &GCCollection_Type);
	return result;
err:
	return NULL;
}



typedef struct {
	PROXY_OBJECT_HEAD_EX(GCCollection, gcci_coll); /* [1..1][const] Underlying collection (cache of this has already been loaded) */
	struct gcset_item                 *gcci_iter;  /* [1..1][const] Next-pointer (in `gcci_coll->gcc_cache.gs_map') */
	struct gcset_item                 *gcci_end;   /* [1..1][const][== gcci_coll->gcc_cache.gs_map+(.gs_msk+1)] End-pointer */
} GCCollectionIterator;

INTDEF DeeTypeObject GCCollectionIterator_Type;

STATIC_ASSERT(offsetof(GCCollectionIterator, gcci_coll) == offsetof(ProxyObject, po_obj));
#define gccolliter_fini  generic_proxy__fini
#define gccolliter_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
gccolliter_init(GCCollectionIterator *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_GCCollectionIterator", &self->gcci_coll);
	if (DeeObject_AssertTypeExact(self->gcci_coll, &GCCollection_Type))
		goto err;
	if (gccoll_loadcache(self->gcci_coll))
		goto err;
	Dee_Incref(self->gcci_coll);
	self->gcci_iter = self->gcci_coll->gcc_cache.gs_map;
	self->gcci_end  = self->gcci_iter + self->gcci_coll->gcc_cache.gs_msk + 1;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gccolliter_copy(GCCollectionIterator *__restrict self,
                GCCollectionIterator *__restrict other) {
	self->gcci_coll = other->gcci_coll;
	Dee_Incref(self->gcci_coll);
	self->gcci_iter = atomic_read(&other->gcci_iter);
	self->gcci_end  = other->gcci_end;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gccolliter_serialize(GCCollectionIterator *__restrict self,
                     DeeSerial *__restrict writer,
                     Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(GCCollectionIterator, field))
	if (DeeSerial_PutObject(writer, ADDROF(gcci_coll), self->gcci_coll))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(gcci_iter), atomic_read(&self->gcci_iter)))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(gcci_end), self->gcci_end);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gccolliter_next(GCCollectionIterator *__restrict self) {
	struct gcset_item *old_iter, *item;
	do {
		old_iter = atomic_read(&self->gcci_iter);
		for (item = old_iter;; ++item) {
			if (item >= self->gcci_end)
				return ITER_DONE;
			if (item->gsi_item)
				break;
		}
	} while (!atomic_cmpxch_weak_or_write(&self->gcci_iter,
	                                      old_iter, item + 1));
	return_reference(item->gsi_item);
}

INTERN DeeTypeObject GCCollectionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCCollectionIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ GCCollectionIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &gccolliter_copy,
			/* tp_any_ctor:    */ &gccolliter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &gccolliter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gccolliter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&gccolliter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gccolliter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};




/* Ensure that "self->gcc_cache" has been populated.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1)) int DCALL
gccoll_loadcache(GCCollection *__restrict self) {
	int status = Dee_once_begin(&self->gcc_loaded);
	if unlikely(status < 0)
		goto err;
	if (status == 0)
		return 0; /* Already initialized */

	/* It's now our job to do the initialization! */
	ASSERT(self->gcc_cache.gs_map == NULL);
	ASSERT(self->gcc_cache.gs_msk == 0);
	ASSERT(self->gcc_cache.gs_siz == 0);
retry:
	(*self->gcc_populate)(self);

	/* Make sure that the set isn't "NULL" (an empty map still requires a 1-element map) */
	if (self->gcc_cache.gs_map == NULL) {
		ASSERT(self->gcc_cache.gs_msk == 0);
		ASSERT(self->gcc_cache.gs_siz == 0);
		self->gcc_cache.gs_map = (struct gcset_item *)Dee_TryCallocc(1, sizeof(struct gcset_item));
		if unlikely(!self->gcc_cache.gs_map)
			self->gcc_cache.gs_siz = (size_t)-1;
	}

	/* Deal with allocation errors... */
	if unlikely(self->gcc_cache.gs_siz == (size_t)-1) {
		Dee_hash_t oldmsk = self->gcc_cache.gs_msk;
		Dee_hash_t minmsk = (oldmsk << 1) | 1;
		struct gcset_item *map;
		bool should_retry;
		map = (struct gcset_item *)Dee_Mallocc(minmsk + 1, sizeof(struct gcset_item));
		should_retry = map != NULL;
		Dee_Free(map);
		map = self->gcc_cache.gs_map;
		if (map) {
			Dee_XDecrefv_unlikely((DREF DeeObject **)map, self->gcc_cache.gs_msk + 1);
			Dee_Free(map);
		}
		self->gcc_cache.gs_map = NULL;
		self->gcc_cache.gs_msk = 0;
		self->gcc_cache.gs_siz = 0;
		if (should_retry)
			goto retry;
		goto err_abort;
	}

	Dee_once_commit(&self->gcc_loaded);
	return 0;
err_abort:
	Dee_once_abort(&self->gcc_loaded);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
GCSet_TryInsert_cb(DeeObject *__restrict ob, void *arg) {
	GCSet *me = (GCSet *)arg;
	GCSet_TryInsert(me, ob);
}

PRIVATE NONNULL((1)) void DCALL
GCSet_TryInsert_transitive_cb(DeeObject *__restrict ob, void *arg) {
	GCSet *me = (GCSet *)arg;
	int status = GCSet_TryInsert(me, ob);
	if (status == 0)
		DeeObject_Visit(ob, &GCSet_TryInsert_transitive_cb, me);
}

PRIVATE NONNULL((1)) void DCALL
GCCollection_PopulateReachableDirect(GCCollection *__restrict self) {
	DeeObject_Visit(self->gcc_obj, &GCSet_TryInsert_cb, &self->gcc_cache);
}

PRIVATE NONNULL((1)) void DCALL
GCCollection_PopulateReachableTransitive(GCCollection *__restrict self) {
	DeeObject_Visit(self->gcc_obj, &GCSet_TryInsert_transitive_cb, &self->gcc_cache);
}

PRIVATE NONNULL((1)) void DCALL
gc_is_referring_direct_cb(DeeObject *__restrict ob, void *arg) {
	DeeObject **p_obj = (DeeObject **)arg;
	if (*p_obj == ob)
		*p_obj = NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
gc_is_referring_direct(DeeObject *from, DeeObject *to) {
	DeeObject_Visit(from, &gc_is_referring_direct_cb, &to);
	return to == NULL;
}

PRIVATE NONNULL((1)) void DCALL
GCCollection_PopulateReferringDirect(GCCollection *__restrict self) {
	struct gc_generation *gen;
	gc_lock_acquire_and_reap(DeeGC_TRACK_F_NORMAL);
	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *gc_obj;
		for (gc_obj = gen->gg_objects; gc_obj;
		     gc_obj = DeeGC_Head(gc_obj)->gc_next) {
			if (gc_is_referring_direct(gc_obj, self->gcc_obj)) {
				if unlikely(GCSet_TryInsertPrepare(&self->gcc_cache))
					goto done;
				if (gc_tryincref(gc_obj))
					GCSet_DoInsertInherited(&self->gcc_cache, gc_obj);
			}
		}
	}
done:
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
}

struct gc_is_referring_transitive_data {
	GCSet        *girtd_visited;    /* Cache for objects already visited */
	GCCollection *girtd_collection; /* [1..1] The collection being filled */
	bool          girtd_found;      /* True if the object in question was found within this branch */
};

PRIVATE NONNULL((1)) void DCALL
gc_is_referring_transitive_cb(DeeObject *__restrict ob, void *arg) {
	struct gc_is_referring_transitive_data *data;
	data = (struct gc_is_referring_transitive_data *)arg;
#ifndef __OPTIMIZE_SIZE__
	if unlikely(data->girtd_collection->gcc_cache.gs_siz == (size_t)-1)
		return; /* Preceding OOM... */
#endif /* !__OPTIMIZE_SIZE__ */
	if (data->girtd_collection->gcc_obj == ob) {
		data->girtd_found = true;
	} else {
		struct gc_is_referring_transitive_data nested;
		int marker = GCSet_GetMarker(data->girtd_visited, ob);
		if (marker >= 0) {
			/* Already visited */
			nested.girtd_found = marker > 0;
		} else {
			nested.girtd_visited    = data->girtd_visited;
			nested.girtd_collection = data->girtd_collection;
			nested.girtd_found      = false;
			DeeObject_Visit(ob, &gc_is_referring_transitive_cb, &nested);
			GCSet_SetMarker(data->girtd_visited, ob, nested.girtd_found);
		}

		/* If reachable via this nested chain, add to resulting set. */
		if (nested.girtd_found)
			GCSet_TryInsert(&data->girtd_collection->gcc_cache, ob);
	}
}

PRIVATE NONNULL((1)) void DCALL
GCCollection_PopulateReferringTransitive(GCCollection *__restrict self) {
	GCSet visited, decref_later;
	struct gc_generation *gen;
	struct gc_is_referring_transitive_data data; 
again:
	GCSet_Init(&visited);
	GCSet_Init(&decref_later);
	data.girtd_visited    = &visited;
	data.girtd_collection = self;
	gc_lock_acquire_and_reap(DeeGC_TRACK_F_NORMAL);
	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *gc_obj;
		for (gc_obj = gen->gg_objects; gc_obj;
		     gc_obj = DeeGC_Head(gc_obj)->gc_next) {
			GCSet *target_set;
			if (!gc_tryincref(gc_obj))
				continue;
			data.girtd_found = false;
			DeeObject_Visit(gc_obj, &gc_is_referring_transitive_cb, &data);

			/* Figure out which set the object should go into. */
			if (data.girtd_found) {
				target_set = &self->gcc_cache;
			} else {
				if likely(Dee_DecrefIfNotOne(gc_obj)) 
					continue;
				/* Must decref "gc_obj" later... */
				target_set = &decref_later;
			}
			if unlikely(GCSet_TryInsertPrepare(target_set)) {
				gc_lock_release(DeeGC_TRACK_F_NORMAL);
				GCSet_FiniMarkers(&visited);
				GCSet_Fini(&decref_later);
				Dee_Decref_likely(gc_obj);
				if (data.girtd_found)
					return; /* Let the caller deal with the error. */

				/* In this case, "gc_obj" should have just gotten destroyed,
				 * meaning that if we just try to do another loop around, we
				 * shouldn't hit that same object again. */
				goto again;
			}
			GCSet_DoInsertInherited(target_set, gc_obj); /* Inherit reference */
		}
	}
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
	GCSet_FiniMarkers(&visited);
}




PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gccoll_serialize(GCCollection *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(GCCollection, field))
	GCCollection *out = DeeSerial_Addr2Mem(writer, addr, GCCollection);
	/* The contents of the cache aren't serialized! */
	GCSet_Init(&out->gcc_cache);
	Dee_once_init(&out->gcc_loaded);
	if (DeeSerial_PutFuncPtr(writer, ADDROF(gcc_populate), self->gcc_populate))
		goto err;
	return DeeSerial_PutObject(writer, ADDROF(gcc_obj), self->gcc_obj);
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
gccoll_fini(GCCollection *__restrict self) {
	GCSet_Fini(&self->gcc_cache);
	Dee_Decref(self->gcc_obj);
}

PRIVATE NONNULL((1, 2)) void DCALL
gccoll_visit(GCCollection *__restrict self, Dee_visit_t proc, void *arg) {
	if (gccoll_loaded(self)) {
		Dee_XVisitv((DeeObject **)self->gcc_cache.gs_map,
		            self->gcc_cache.gs_msk + 1);
	}
	Dee_Visit(self->gcc_obj);
}

PRIVATE WUNUSED NONNULL((1)) DREF GCCollectionIterator *DCALL
gccoll_iter(GCCollection *__restrict self) {
	DREF GCCollectionIterator *result;
	if (gccoll_loadcache(self))
		goto err;
	result = DeeObject_MALLOC(GCCollectionIterator);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->gcci_coll = self;
	result->gcci_iter = self->gcc_cache.gs_map;
	result->gcci_end  = self->gcc_cache.gs_map + self->gcc_cache.gs_msk + 1;
	DeeObject_Init(result, &GCCollectionIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
gccoll_contains(GCCollection *self, DeeObject *ob) {
	if (!gccoll_loaded(self)) {
		/* TODXXO: Optimizations for known load functions */
	}
	return default__seq_operator_contains__with__seq_contains(Dee_AsObject(self), ob);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
gccoll_size(GCCollection *__restrict self) {
	if (gccoll_loadcache(self))
		goto err;
	return self->gcc_cache.gs_siz;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
gccoll_foreach(GCCollection *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct gcset_item *iter, *end;
	if (gccoll_loadcache(self))
		goto err;
	iter = self->gcc_cache.gs_map;
	end  = iter + self->gcc_cache.gs_msk + 1;
	for (; iter < end; ++iter) {
		DeeObject *ob = iter->gsi_item;
		if (ob) {
			temp = (*cb)(arg, ob);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}



PRIVATE struct type_seq gccoll_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gccoll_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gccoll_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&gccoll_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&gccoll_size,
	/* .tp_size_fast                  = */ NULL,
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
};

PRIVATE struct type_member tpconst gccoll_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &GCCollectionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject GCCollection_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCCollection",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ GCCollection,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &gccoll_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gccoll_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&gccoll_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &gccoll_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: cached=return_self;  frozen=gccoll_loadcache()+return_self */
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ gccoll_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};




/* Iterator for enumerating the set of active GC objects. */
typedef struct {
	OBJECT_HEAD
	DREF struct gc_generation *gci_gen; /* [0..1] Current GC generation, or "NULL" if enumeration has finished. */
	DREF DeeObject            *gci_obj; /* [1..1][valid_if(gci_gen)] Next object to enumerate (object exists within `gci_gen') */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t          gci_lck; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
} GCIter;

#define gc_iter_available(self)  Dee_atomic_lock_available(&(self)->gci_lck)
#define gc_iter_acquired(self)   Dee_atomic_lock_acquired(&(self)->gci_lck)
#define gc_iter_tryacquire(self) Dee_atomic_lock_tryacquire(&(self)->gci_lck)
#define gc_iter_acquire(self)    Dee_atomic_lock_acquire(&(self)->gci_lck)
#define gc_iter_waitfor(self)    Dee_atomic_lock_waitfor(&(self)->gci_lck)
#define gc_iter_release(self)    Dee_atomic_lock_release(&(self)->gci_lck)


/* Update "gci_gen" and "gci_obj" to acquire a reference to the next object. */
PRIVATE NONNULL((1)) void DCALL
gciter_find_next_object(GCIter *__restrict self) {
	struct Dee_gc_head *head;
	DeeObject *obj = self->gci_obj;
select_next:
	ASSERT(obj);
	head = DeeGC_Head(obj);
	obj  = head->gc_next;
	while (!obj) {
		struct gc_generation *nextgen;
		nextgen = self->gci_gen->gg_next;
		if (nextgen == NULL) {
			gc_generation_decref(self->gci_gen);
			self->gci_gen = NULL;
			self->gci_obj = NULL;
			return;
		}
		gc_generation_incref(nextgen);
		gc_generation_decref(self->gci_gen);
		self->gci_gen = nextgen;
		obj = nextgen->gg_objects;
	}
	if (!gc_tryincref(obj))
		goto select_next;
	self->gci_obj = obj;
}

PRIVATE NONNULL((1)) void DCALL
gciter_setup(GCIter *__restrict self) {
	gc_lock_acquire_and_reap(DeeGC_TRACK_F_NORMAL);
	self->gci_gen = &gc_gen0;
	gc_generation_incref(&gc_gen0);
	self->gci_obj = gc_gen0.gg_objects;
	while (self->gci_obj == NULL) {
		struct gc_generation *nextgen;
		nextgen = self->gci_gen->gg_next;
		if (nextgen == NULL) {
			gc_generation_decref(self->gci_gen);
			self->gci_gen = NULL;
			goto done;
		}
		gc_generation_incref(nextgen);
		gc_generation_decref(self->gci_gen);
		self->gci_gen = nextgen;
		self->gci_obj = self->gci_gen->gg_objects;
	}
	if (!gc_tryincref(self->gci_obj))
		gciter_find_next_object(self);
done:
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
	Dee_atomic_lock_init(&self->gci_lck);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gciter_init(GCIter *__restrict self) {
	gciter_setup(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gciter_copy(GCIter *__restrict self,
            GCIter *__restrict other) {
	gc_iter_acquire(other);
	self->gci_gen = other->gci_gen;
	self->gci_obj = other->gci_obj;
	if (self->gci_gen) {
		ASSERT(self->gci_obj);
		Dee_Incref(self->gci_obj);
		gc_generation_incref(self->gci_gen);
	}
	gc_iter_release(other);
	Dee_atomic_lock_init(&self->gci_lck);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
gciter_fini(GCIter *__restrict self) {
	if (self->gci_gen) {
		Dee_Decref(self->gci_obj);
		gc_generation_decref(self->gci_gen);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
gciter_visit(GCIter *__restrict self, Dee_visit_t proc, void *arg) {
	gc_iter_acquire(self);
	Dee_XVisit(self->gci_obj);
	gc_iter_release(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gciter_serialize(GCIter *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(GCIter, field))
	DREF struct gc_generation *out__gci_gen;
	DREF DeeObject *out__gci_obj;
	gc_iter_acquire(self);
	out__gci_gen = self->gci_gen;
	out__gci_obj = self->gci_obj;
	if (out__gci_gen) {
		gc_generation_incref(out__gci_gen);
		Dee_Incref(out__gci_obj);
	}
	gc_iter_release(self);
	if (!out__gci_gen) {
		GCIter *out;
		out = DeeSerial_Addr2Mem(writer, addr, GCIter);
		out->gci_gen = NULL;
		out->gci_obj = NULL;
		Dee_atomic_lock_init(&out->gci_lck);
		return 0;
	}
	if (DeeSerial_PutObjectInherited(writer, ADDROF(gci_obj), out__gci_obj))
		goto err_gen;
	/* XXX: This here assumes that "struct gc_generation" are statically allocated */
	return DeeSerial_PutPointer(writer, ADDROF(gci_gen), out__gci_gen);
err_gen:
	gc_generation_decref(out__gci_gen);
/*err:*/
	return -1;
#undef ADDROF
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gciter_next(GCIter *__restrict self) {
	DREF DeeObject *result;
	gc_iter_acquire(self);
	if (!self->gci_gen) {
		gc_iter_release(self);
		return ITER_DONE;
	}
	result = self->gci_obj; /* Inherit reference */
	gciter_find_next_object(self);
	gc_iter_release(self);
	return result;
}


INTERN DeeTypeObject GCIter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCIter",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ GCIter,
			/* tp_ctor:        */ &gciter_init,
			/* tp_copy_ctor:   */ &gciter_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &gciter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gciter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&gciter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gciter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};





PRIVATE WUNUSED NONNULL((1)) DREF GCIter *DCALL
gcenum_iter(DeeObject *__restrict UNUSED(self)) {
	DREF GCIter *result = DeeObject_MALLOC(GCIter);
	if unlikely(!result)
		goto err;
	gciter_setup(result);
	DeeObject_Init(result, &GCIter_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
gcenum_size(DeeObject *__restrict UNUSED(self)) {
	size_t result;
	struct gc_generation *gen;
	gc_lock_acquire_and_reap(DeeGC_TRACK_F_NORMAL);
	result = gc_gen0.gg_count;
	for (gen = gc_gen0.gg_next; gen; gen = gen->gg_next) {
		gc_generation_fixcount(gen);
		result += gen->gg_count;
	}
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_contains(DeeObject *__restrict UNUSED(self),
                DeeObject *__restrict ob) {
	/* Contains **all** instances of GC objects (except for non-heap types) */
	DeeTypeObject *tp = Dee_TYPE(ob);
	if (!DeeType_IsGC(tp))
		goto nope;
	if (DeeType_Extends(tp, &DeeType_Type)) {
		DeeTypeObject *me = (DeeTypeObject *)ob;
		if (!(me->tp_flags & TP_FHEAP))
			goto nope; /* Statically allocated types aren't GC-tracked */
	}
	return_true;
nope:
	return_false;
}

PRIVATE struct type_seq gcenum_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcenum_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&gcenum_size,
	/* .tp_size_fast                  = */ NULL,
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
};

PRIVATE struct type_member tpconst gcenum_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &GCIter_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_collect(DeeObject *UNUSED(self),
               size_t argc, DeeObject *const *argv) {
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("collect", params: """
	size_t max = (size_t)-1;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_collect_params "max=!-1"
	struct {
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	DeeArg_Unpack0Or1X(err, argc, argv, "collect", &args.max_, UNPxSIZ, DeeObject_AsSizeM1);
/*[[[end]]]*/
	result = DeeGC_Collect(args.max_);
	if (result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF GCCollection *DCALL
gcenum_reachable(DeeObject *UNUSED(self), size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("reachable", params: """
	DeeObject *from;
	bool transitive = true;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_reachable_params "from,transitive=!t"
	struct {
		DeeObject *from;
		bool transitive;
	} args;
	args.transitive = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__from_transitive, "o|b:reachable", &args))
		goto err;
/*[[[end]]]*/
	return GCCollection_New(args.from,
	                        args.transitive ? &GCCollection_PopulateReachableTransitive
	                                        : &GCCollection_PopulateReachableDirect);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF GCCollection *DCALL
gcenum_referring(DeeObject *UNUSED(self), size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("referring", params: """
	DeeObject *to;
	bool transitive = true;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_referring_params "to,transitive=!t"
	struct {
		DeeObject *to;
		bool transitive;
	} args;
	args.transitive = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__to_transitive, "o|b:referring", &args))
		goto err;
/*[[[end]]]*/
	return GCCollection_New(args.to,
	                        args.transitive ? &GCCollection_PopulateReferringTransitive
	                                        : &GCCollection_PopulateReferringDirect);
err:
	return NULL;
}


PRIVATE struct type_method tpconst gcenum_methods[] = {
	TYPE_METHOD_F("collect", &gcenum_collect, METHOD_FNOREFESCAPE,
	              "(" gcenum_collect_params ")->?Dint\n"
	              "Try to collect at most @max GC objects and return the actual number collected\n"
	              "Note that more than @max objects may be collected if sufficiently large reference cycles exist"),
	TYPE_KWMETHOD_F("reachable", &gcenum_reachable, METHOD_FNOREFESCAPE,
	                "(" gcenum_reachable_params ")->?DSet\n"
	                "Returns a set of objects that are reachable from @from. "
	                /**/ "When @transitive is !f, only direct references are included"),
	TYPE_KWMETHOD_F("referring", &gcenum_referring, METHOD_FNOREFESCAPE,
	                "(" gcenum_referring_params ")->?DSet\n"
	                "Returns a set of GC objects (acting as roots) and normal objects that "
	                /**/ "somehow end up referencing @to. When @transitive is !f, only GC "
	                /**/ "objects that directly reference @to are included.\n"
	                "It is a technical impossibility to find #Ball objects or places that "
	                /**/ "may hold a reference to some object, since only GC objects are "
	                /**/ "tracked. Additionally, this sort of #I{reverse search} is fairly "
	                /**/ "expensive, meaning this method should be avoided if possible."),
	TYPE_METHOD_END
};

PRIVATE DeeTypeObject GCEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCEnum",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_SINGLETON,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Static singleton, so no serial needed */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_sizeob),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__96C7A3207FAE93E2),
	/* .tp_seq           = */ &gcenum_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ gcenum_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ gcenum_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


/* An generic sequence singleton that can be
 * iterated to yield all tracked GC objects.
 * This object also offers a hand full of member functions
 * that user-space an invoke to trigger various GC-related
 * functionality:
 *   - collect(max: int = -1): int;
 * Also: remember that this derives from `Sequence', so you
 *       can use all its attributes, like `empty', etc.
 * NOTE: This object is exported as `gc from deemon' */
PUBLIC DeeObject DeeGCEnumTracked_Singleton = {
	OBJECT_HEAD_INIT(&GCEnum_Type)
};

DECL_END
#endif /* CONFIG_EXPERIMENTAL_REWORKED_GC */

#endif /* !GUARD_DEEMON_RUNTIME_GC_INSPECT_C_INL */
