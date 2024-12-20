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
#ifndef GUARD_DEEMON_OBJECTS_CACHED_SEQ_C
#define GUARD_DEEMON_OBJECTS_CACHED_SEQ_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/gc.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/objectlist.h>

#include <hybrid/overflow.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"

/**/
#include "cached-seq.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_ctor(CachedSeq_WithIter *__restrict self) {
	self->cswi_iter = NULL;
	Dee_atomic_lock_init(&self->cswi_lock);
	objectlist_init(&self->cswi_cache);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswi_copycache(CachedSeq_WithIter *__restrict self,
               CachedSeq_WithIter *__restrict other) {
	DREF DeeObject **cache_copy;
	size_t cache_size;
again:
	CachedSeq_WithIter_LockAcquire(other);
	cache_size = other->cswi_cache.ol_elemc;
	cache_copy = Dee_objectlist_elemv_trymalloc(cache_size);
	if unlikely(!cache_copy) {
		CachedSeq_WithIter_LockRelease(other);
		cache_copy = Dee_objectlist_elemv_malloc(cache_size);
		if unlikely(!cache_copy)
			goto err;
		CachedSeq_WithIter_LockAcquire(other);
		if unlikely(cache_size != other->cswi_cache.ol_elemc) {
			CachedSeq_WithIter_LockRelease(other);
			Dee_objectlist_elemv_free(cache_copy);
			goto again;
		}
	}
	cache_copy = Dee_Movrefv(cache_copy, other->cswi_cache.ol_elemv, cache_size);
	_Dee_objectlist_setalloc(&self->cswi_cache, cache_size);
	self->cswi_iter = other->cswi_iter;
	Dee_XIncref(self->cswi_iter);
	CachedSeq_WithIter_LockRelease(other);
	self->cswi_cache.ol_elemc = cache_size;
	self->cswi_cache.ol_elemv = cache_copy;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswi_copy(CachedSeq_WithIter *__restrict self,
          CachedSeq_WithIter *__restrict other) {
	if unlikely(cswi_copycache(self, other))
		goto err;
	if (self->cswi_iter) {
		DREF DeeObject *iter_copy;
		iter_copy = DeeObject_Copy(self->cswi_iter);
		if unlikely(!iter_copy)
			goto err_cache_iter;
		Dee_Decref_unlikely(self->cswi_iter);
		self->cswi_iter = iter_copy;
	}
	return 0;
err_cache_iter:
	Dee_Decref(self->cswi_iter);
	objectlist_fini(&self->cswi_cache);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswi_deep(CachedSeq_WithIter *__restrict self,
          CachedSeq_WithIter *__restrict other) {
	return cswi_copycache(self, other);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_deepload(CachedSeq_WithIter *__restrict self) {
	size_t i;
	for (i = 0;; ++i) {
		DREF DeeObject *oldval, *newval;
again_copy_at_i:
		CachedSeq_WithIter_LockAcquire(self);
		if (i >= self->cswi_cache.ol_elemc) {
			CachedSeq_WithIter_LockRelease(self);
			break;
		}
		oldval = self->cswi_cache.ol_elemv[i];
		Dee_Incref(oldval);
		CachedSeq_WithIter_LockRelease(self);
		newval = DeeObject_DeepCopy(oldval);
		Dee_Decref_unlikely(oldval);
		if unlikely(!newval)
			goto err;
		CachedSeq_WithIter_LockAcquire(self);
		if unlikely(i >= self->cswi_cache.ol_elemc) {
			CachedSeq_WithIter_LockRelease(self);
			Dee_Decref(newval);
			break;
		}
		if unlikely(self->cswi_cache.ol_elemv[i] != oldval) {
			CachedSeq_WithIter_LockRelease(self);
			Dee_Decref(newval);
			goto again_copy_at_i;
		}
		self->cswi_cache.ol_elemv[i] = newval; /* Inherit reference (x2) */
		CachedSeq_WithIter_LockRelease(self);
		Dee_Decref(oldval);
	}
	return DeeObject_InplaceDeepCopyWithLock(&self->cswi_iter, &self->cswi_lock);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_init(CachedSeq_WithIter *__restrict self,
          size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_CachedSeqWithIter", &self->cswi_iter))
		goto err;
	Dee_Incref(self->cswi_iter);
	Dee_atomic_lock_init(&self->cswi_lock);
	objectlist_init(&self->cswi_cache);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
cswi_fini(CachedSeq_WithIter *__restrict self) {
	objectlist_fini(&self->cswi_cache);
	Dee_XDecref(self->cswi_iter);
}

PRIVATE NONNULL((1, 2)) void DCALL
cswi_visit(CachedSeq_WithIter *__restrict self, dvisit_t proc, void *arg) {
	CachedSeq_WithIter_LockAcquire(self);
	Dee_Visitv(self->cswi_cache.ol_elemv,
	           self->cswi_cache.ol_elemc);
	Dee_XVisit(self->cswi_iter);
	CachedSeq_WithIter_LockRelease(self);
}

PRIVATE NONNULL((1)) void DCALL
cswi_clear(CachedSeq_WithIter *__restrict self) {
	size_t old_elemc;
	DREF DeeObject **old_elemv;
	CachedSeq_WithIter_LockAcquire(self);
	old_elemc = self->cswi_cache.ol_elemc;
	old_elemv = self->cswi_cache.ol_elemv;
	self->cswi_cache.ol_elemc = 0;
	self->cswi_cache.ol_elemv = NULL;
	_Dee_objectlist_setalloc(&self->cswi_cache, 0);
	CachedSeq_WithIter_LockRelease(self);
	Dee_Decrefv(old_elemv, old_elemc);
	Dee_objectlist_elemv_free(old_elemv);
}

PRIVATE struct type_gc cswi_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&cswi_clear
};

/* @return: 1 : Failure (`index' is out-of-bounds, but the cache is now fully loaded)
 * @return: 0 : Success (`index' is now loaded within the cache)
 * @return: -1: Error * */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_ensure_loaded(CachedSeq_WithIter *__restrict self, size_t index) {
	DREF DeeObject *nextitem;
	CachedSeq_WithIter_LockAcquire(self);
	while (index >= self->cswi_cache.ol_elemc) {
		size_t alloc;
		DREF DeeObject *iter;
		iter = self->cswi_iter;
		if (!iter) {
			CachedSeq_WithIter_LockRelease(self);
			return 1; /* Out-of-bounds */
		}
		Dee_Incref(iter);
		CachedSeq_WithIter_LockRelease(self);

		/* Load 1 additional element. */
		nextitem = DeeObject_IterNext(iter);
		Dee_Decref_unlikely(iter);
		if (!ITER_ISOK(nextitem)) {
			if unlikely(!nextitem)
				goto err;
			CachedSeq_WithIter_LockAcquire(self);
			iter = self->cswi_iter;
			self->cswi_iter = NULL;
			CachedSeq_WithIter_LockRelease(self);
			if likely(iter)
				Dee_Decref(iter);
			return 1;
		}

		/* Store "nextitem" within the cache. */
lock_and_append_to_cache:
		CachedSeq_WithIter_LockAcquire(self);
		alloc = Dee_objectlist_getalloc(&self->cswi_cache);
		ASSERT(alloc >= self->cswi_cache.ol_elemc);
		if unlikely(alloc <= self->cswi_cache.ol_elemc) {
			DREF DeeObject **new_elemv;
			size_t new_alloc = DEE_OBJECTLIST_MOREALLOC(alloc);
			if (new_alloc < DEE_OBJECTLIST_MINALLOC)
				new_alloc = DEE_OBJECTLIST_MINALLOC;
			new_elemv = Dee_objectlist_elemv_tryrealloc_safe(self->cswi_cache.ol_elemv, new_alloc);
			if unlikely(!new_elemv) {
				size_t min_alloc = self->cswi_cache.ol_elemc;
				new_alloc = min_alloc + 1;
				new_elemv = Dee_objectlist_elemv_tryrealloc_safe(self->cswi_cache.ol_elemv, new_alloc);
				if unlikely(!new_elemv) {
					/* Cannot realloc -> try again while not holding any locks. */
					CachedSeq_WithIter_LockRelease(self);
					new_alloc = DEE_OBJECTLIST_MOREALLOC(alloc);
					if (new_alloc < DEE_OBJECTLIST_MINALLOC)
						new_alloc = DEE_OBJECTLIST_MINALLOC;
					new_elemv = Dee_objectlist_elemv_trymalloc_safe(new_alloc);
					if unlikely(!new_elemv) {
						new_alloc = min_alloc + 1;
						new_elemv = Dee_objectlist_elemv_malloc_safe(new_alloc);
						if unlikely(!new_elemv)
							goto err_nextitem;
					}
					CachedSeq_WithIter_LockAcquire(self);
					if unlikely(self->cswi_cache.ol_elemc > min_alloc) {
						CachedSeq_WithIter_LockRelease(self);
						Dee_objectlist_elemv_free(new_elemv);
						goto lock_and_append_to_cache;
					}
					/* Copy into the new cache-buffer. */
					new_elemv = (DREF DeeObject **)memcpyc(new_elemv, self->cswi_cache.ol_elemv,
					                                       self->cswi_cache.ol_elemc,
					                                       sizeof(DREF DeeObject *));
					Dee_objectlist_elemv_free(self->cswi_cache.ol_elemv);
					self->cswi_cache.ol_elemv = new_elemv;
					_Dee_objectlist_setalloc(&self->cswi_cache, new_alloc);
				}
			}
			self->cswi_cache.ol_elemv = new_elemv;
			_Dee_objectlist_setalloc(&self->cswi_cache, new_alloc);
		}
		self->cswi_cache.ol_elemv[self->cswi_cache.ol_elemc] = nextitem; /* Inherit reference */
		++self->cswi_cache.ol_elemc;
	}
	CachedSeq_WithIter_LockRelease(self);
	return 0; /* Already in cache! */
err_nextitem:
	Dee_Decref(nextitem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswi_trygetitem_index(CachedSeq_WithIter *__restrict self, size_t index) {
	DREF DeeObject *result;
	int status;
#ifndef CONFIG_NO_THREADS
again:
#endif /* !CONFIG_NO_THREADS */
	status = cswi_ensure_loaded(self, index);
	if unlikely(status < 0)
		goto err;
	if (status > 0)
		return ITER_DONE;
	CachedSeq_WithIter_LockAcquire(self);
#ifndef CONFIG_NO_THREADS
	if unlikely(index >= self->cswi_cache.ol_elemc) {
		CachedSeq_WithIter_LockRelease(self);
		goto again;
	}
#endif /* !CONFIG_NO_THREADS */
	ASSERT(index < self->cswi_cache.ol_elemc);
	result = self->cswi_cache.ol_elemv[index];
	Dee_Incref(result);
	CachedSeq_WithIter_LockRelease(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswi_getitem_index(CachedSeq_WithIter *__restrict self, size_t index) {
	DREF DeeObject *result = cswi_trygetitem_index(self, index);
	if unlikely(result == ITER_DONE)
		goto err_oob;
	return result;
	{
		size_t size;
err_oob:
		CachedSeq_WithIter_LockAcquire(self);
		size = self->cswi_cache.ol_elemc;
		CachedSeq_WithIter_LockRelease(self);
		err_index_out_of_bounds((DeeObject *)self, index, size);
	}
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_bounditem_index(CachedSeq_WithIter *__restrict self, size_t index) {
	int result = cswi_ensure_loaded(self, index);
	if likely(result >= 0)
		result = result ? -2 : 1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_hasitem_index(CachedSeq_WithIter *__restrict self, size_t index) {
	int result = cswi_ensure_loaded(self, index);
	if likely(result >= 0)
		result = !result;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cswi_size(CachedSeq_WithIter *__restrict self) {
	size_t result;
	if unlikely(cswi_ensure_loaded(self, (size_t)-1) < 0)
		goto err;
	CachedSeq_WithIter_LockAcquire(self);
	result = self->cswi_cache.ol_elemc;
	CachedSeq_WithIter_LockRelease(self);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cswi_size_fast(CachedSeq_WithIter *__restrict self) {
	size_t result;
	CachedSeq_WithIter_LockAcquire(self);
	result = self->cswi_cache.ol_elemc;
	if (self->cswi_iter != NULL)
		result = (size_t)-1;
	CachedSeq_WithIter_LockRelease(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF CachedSeq_WithIter_Iterator *DCALL
cswi_iter(CachedSeq_WithIter *__restrict self) {
	DREF CachedSeq_WithIter_Iterator *result;
	result = DeeObject_MALLOC(CachedSeq_WithIter_Iterator);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->cswii_cache = self;
	result->cswii_index = 0;
	DeeObject_Init(result, &CachedSeq_WithIter_Iterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
cswi_foreach(CachedSeq_WithIter *__restrict self, Dee_foreach_t cb, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0;; ++i) {
		DREF DeeObject *item;
		item = cswi_trygetitem_index(self, i);
		if unlikely(!ITER_ISOK(item)) {
			if unlikely(!item)
				goto err;
			break;
		}
		temp = (*cb)(arg, item);
		Dee_Decref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cswi_asvector(CachedSeq_WithIter *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t result;
	if unlikely(cswi_ensure_loaded(self, (size_t)-1) < 0)
		goto err;
	CachedSeq_WithIter_LockAcquire(self);
	result = self->cswi_cache.ol_elemc;
	if likely(dst_length >= result)
		Dee_Movrefv(dst, self->cswi_cache.ol_elemv, result);
	CachedSeq_WithIter_LockRelease(self);
	return result;
err:
	return (size_t)-1;
}



PRIVATE struct type_seq cswi_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cswi_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&cswi_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cswi_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&cswi_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cswi_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&cswi_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&cswi_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cswi_trygetitem_index,
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
	/* NOTE: "tp_asvector" explicitly states that "repeated" invocations mustn't produce any
	 *       (additional) side-effects. As such, it *does* allow side-effects on the first
	 *       invocation, meaning we're perfectly fine to implement it! */
	/* .tp_asvector = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&cswi_asvector,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_bool(CachedSeq_WithIter *__restrict self) {
	return cswi_hasitem_index(self, 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF CachedSeq_WithIter *DCALL
cswi_getfrozen(CachedSeq_WithIter *__restrict self) {
	/* Once fully loaded, a cache can be considered as frozen! */
	if unlikely(cswi_ensure_loaded(self, (size_t)-1) < 0)
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswi_getiter(CachedSeq_WithIter *__restrict self) {
	DREF DeeObject *result;
	CachedSeq_WithIter_LockAcquire(self);
	result = self->cswi_iter;
	if likely(result) {
		Dee_Incref(result);
		CachedSeq_WithIter_LockRelease(self);
		return result;
	}
	CachedSeq_WithIter_LockRelease(self);
	err_unbound_attribute_string(&CachedSeq_WithIter_Type, "__iter__");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswi_populate(CachedSeq_WithIter *__restrict self,
              size_t argc, DeeObject *const *argv) {
	size_t count = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, ":populate", &count))
		goto err;
	if likely(count) {
		--count;
		if unlikely(cswi_ensure_loaded(self, count) < 0)
			goto err;
	}
	return_none;
err:
	return NULL;
}


PRIVATE struct type_method tpconst cswi_methods[] = {
	TYPE_METHOD("populate", &cswi_populate,
	            "(count:?Dint=!ASIZE_MAX?Dint)\n"
	            "Ensure that the first @count elements of ?#__seq__ have been cached"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst cswi_getsets[] = {
	TYPE_GETTER(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER(STR_frozen, &cswi_getfrozen, "->?.\nFully populate the cache, then re-return it"),
	TYPE_GETTER("__iter__", &cswi_getiter, "->?.\nThe iterator acting as cache source (throws :UnboundAttribute once exhausted)"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cswi_members[] = {
	TYPE_MEMBER_FIELD("__cache_size__", STRUCT_SIZE_T | STRUCT_CONST | STRUCT_ATOMIC,
	                  offsetof(CachedSeq_WithIter, cswi_cache.ol_elemc)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst cswi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &CachedSeq_WithIter_Iterator_Type),
	TYPE_MEMBER_END
};



/* Uses an auto-growing vector for elements, that is fed by an iterator. */
INTERN DeeTypeObject CachedSeq_WithIter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedSeqWithIter",
	/* .tp_doc      = */ DOC("()\n"
	                         "(iter:?DIterator)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&cswi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&cswi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&cswi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&cswi_init,
				TYPE_FIXED_ALLOCATOR_GC(CachedSeq_WithIter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cswi_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cswi_visit,
	/* .tp_gc            = */ &cswi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cswi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cswi_methods,
	/* .tp_getsets       = */ cswi_getsets,
	/* .tp_members       = */ cswi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cswi_class_members
};


STATIC_ASSERT(offsetof(CachedSeq_WithIter_Iterator, cswii_cache) == offsetof(ProxyObject, po_obj));
#define cswiiter_fini  generic_proxy_fini
#define cswiiter_visit generic_proxy_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswiiter_ctor(CachedSeq_WithIter_Iterator *__restrict self) {
	self->cswii_cache = (DREF CachedSeq_WithIter *)DeeObject_NewDefault(&CachedSeq_WithIter_Type);
	if unlikely(!self->cswii_cache)
		goto err;
	self->cswii_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswiiter_copy(CachedSeq_WithIter_Iterator *__restrict self,
              CachedSeq_WithIter_Iterator *__restrict other) {
	self->cswii_index = atomic_read(&other->cswii_index);
	return generic_proxy_copy_alias((ProxyObject *)self, (ProxyObject *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswiiter_deep(CachedSeq_WithIter_Iterator *__restrict self,
              CachedSeq_WithIter_Iterator *__restrict other) {
	self->cswii_index = atomic_read(&other->cswii_index);
	return generic_proxy_deepcopy((ProxyObject *)self, (ProxyObject *)other);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswiiter_init(CachedSeq_WithIter_Iterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	self->cswii_index = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":_CachedSeqWithIterIterator",
	                  &self->cswii_cache, &self->cswii_index))
		goto err;
	if (DeeObject_AssertTypeExact(self->cswii_cache, &CachedSeq_WithIter_Type))
		goto err;
	Dee_Incref(self->cswii_cache);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswiiter_bool(CachedSeq_WithIter_Iterator *__restrict self) {
	size_t index = atomic_read(&self->cswii_index);
	return cswi_hasitem_index(self->cswii_cache, index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswiiter_next(CachedSeq_WithIter_Iterator *__restrict self) {
	DREF DeeObject *result;
	for (;;) {
		size_t index;
		index  = atomic_read(&self->cswii_index);
		result = cswi_trygetitem_index(self->cswii_cache, index);
		if unlikely(!ITER_ISOK(result))
			break;
		if likely(atomic_cmpxch_or_write(&self->cswii_index, index, index + 1))
			break;
		Dee_Decref(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cswiiter_advance(CachedSeq_WithIter_Iterator *__restrict self, size_t step) {
	size_t old_index, new_index;
	do {
		old_index = atomic_read(&self->cswii_index);
		if (OVERFLOW_UADD(old_index, step, &new_index))
			new_index = (size_t)-1;
		if likely(new_index > 0) {
			int load_status;
			load_status = cswi_ensure_loaded(self->cswii_cache, new_index - 1);
			if unlikely(load_status < 0)
				goto err;
			if (load_status > 0) {
				/* Clap "new_index" to the max possible value */
				CachedSeq_WithIter_LockAcquire(self->cswii_cache);
				new_index = self->cswii_cache->cswi_cache.ol_elemc;
				CachedSeq_WithIter_LockRelease(self->cswii_cache);
			}
		}
	} while (!atomic_cmpxch_or_write(&self->cswii_index, old_index, new_index));
	return new_index - old_index;
err:
	return (size_t)-1;
}

PRIVATE struct type_iterator cswiiter_iterator = {
	/* .tp_nextpair  = */ NULL,
	/* .tp_nextkey   = */ NULL,
	/* .tp_nextvalue = */ NULL,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&cswiiter_advance,
};

PRIVATE struct type_member tpconst cswiiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT,
	                      offsetof(CachedSeq_WithIter_Iterator, cswii_cache),
	                      "->?Ert:CachedSeqWithIter"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T | STRUCT_ATOMIC,
	                  offsetof(CachedSeq_WithIter_Iterator, cswii_index)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject CachedSeq_WithIter_Iterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedSeqWithIterIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(base:?Ert:CachedSeqWithIter,index=!0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&cswiiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&cswiiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&cswiiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&cswiiter_init,
				TYPE_FIXED_ALLOCATOR(CachedSeq_WithIter_Iterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswiiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswiiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cswiiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cswiiter_next,
	/* .tp_iterator      = */ &cswiiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cswiiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CACHED_SEQ_C */
