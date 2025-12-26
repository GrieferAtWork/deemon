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
#ifndef GUARD_DEEMON_OBJECTS_CACHED_SEQ_C
#define GUARD_DEEMON_OBJECTS_CACHED_SEQ_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
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
/**/

#include <stddef.h> /* size_t, offsetof */

DECL_BEGIN

/************************************************************************/
/* ITERATOR-BASED CACHE                                                 */
/************************************************************************/
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
	Dee_atomic_lock_init(&self->cswi_lock);
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
		self->cswi_iter = DeeObject_CopyInherited(self->cswi_iter);
		if unlikely(!self->cswi_iter)
			goto err_cache;
	}
	return 0;
err_cache:
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
		newval = DeeObject_DeepCopyInherited(oldval);
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
	return DeeObject_XInplaceDeepCopyWithLock(&self->cswi_iter,
	                                          &self->cswi_lock);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_init(CachedSeq_WithIter *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_CachedSeqWithIter", &self->cswi_iter);
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
cswi_visit(CachedSeq_WithIter *__restrict self, Dee_visit_t proc, void *arg) {
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

PRIVATE struct type_gc tpconst cswi_gc = {
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
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, size);
	}
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_bounditem_index(CachedSeq_WithIter *__restrict self, size_t index) {
	int result = cswi_ensure_loaded(self, index);
	if unlikely(result < 0)
		return Dee_BOUND_ERR;
	if (result > 0)
		return Dee_BOUND_MISSING;
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cswi_hasitem_index(CachedSeq_WithIter *__restrict self, size_t index) {
	int result = cswi_ensure_loaded(self, index);
	if likely(result >= 0)
		result = result ? 0 : 1;
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
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&cswi_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cswi_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&cswi_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cswi_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&cswi_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&cswi_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cswi_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
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
	return DeeRT_ErrUnboundAttrCStr(self, "__iter__");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswi_populate(CachedSeq_WithIter *__restrict self,
              size_t argc, DeeObject *const *argv) {
	size_t count = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "|" UNPxSIZ ":populate", &count))
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
	            "(count:?Dint=!-1)\n"
	            "Ensure that the first @count elements of ?#__seq__ have been cached"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst cswi_getsets[] = {
	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_frozen, &cswi_getfrozen, "->?.\nFully populate the cache, then re-return it"),
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
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CachedSeq_WithIter,
			/* tp_ctor:        */ &cswi_ctor,
			/* tp_copy_ctor:   */ &cswi_copy,
			/* tp_deep_ctor:   */ &cswi_deep,
			/* tp_any_ctor:    */ &cswi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cswi_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cswi_visit,
	/* .tp_gc            = */ &cswi_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &cswi_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cswi_methods,
	/* .tp_getsets       = */ cswi_getsets,
	/* .tp_members       = */ cswi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cswi_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


STATIC_ASSERT(offsetof(CachedSeq_WithIter_Iterator, cswii_cache) == offsetof(ProxyObject, po_obj));
#define cswiiter_fini  generic_proxy__fini
#define cswiiter_visit generic_proxy__visit

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
	return generic_proxy__copy_alias((ProxyObject *)self, (ProxyObject *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswiiter_deep(CachedSeq_WithIter_Iterator *__restrict self,
              CachedSeq_WithIter_Iterator *__restrict other) {
	self->cswii_index = atomic_read(&other->cswii_index);
	return generic_proxy__deepcopy((ProxyObject *)self, (ProxyObject *)other);
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
	/* .tp_nextpair  = */ DEFIMPL(&default__nextpair__with__iter_next),
	/* .tp_nextkey   = */ DEFIMPL(&default__nextkey__with__iter_next),
	/* .tp_nextvalue = */ DEFIMPL(&default__nextvalue__with__iter_next),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ CachedSeq_WithIter_Iterator,
			/* tp_ctor:        */ &cswiiter_ctor,
			/* tp_copy_ctor:   */ &cswiiter_copy,
			/* tp_deep_ctor:   */ &cswiiter_deep,
			/* tp_any_ctor:    */ &cswiiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswiiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswiiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cswiiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cswiiter_next,
	/* .tp_iterator      = */ &cswiiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cswiiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

















/************************************************************************/
/* INDEX-BASED CACHE                                                    */
/************************************************************************/

#ifdef HAVE_CachedSeq_WithGetItem
PRIVATE struct {
	Dee_OBJECT_HEAD
	Dee_ssize_t ob_size;
	Dee_digit_t ob_digit[CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS)];
} int_SIZE_MAX_plus_1 = {
	OBJECT_HEAD_INIT(&DeeInt_Type),
	CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS),
	{
#if CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 2
#if CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 3
#if CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 4
#if CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 5
#if CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 6
#if CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 7
#error "Unsupported __SIZEOF_SIZE_T__/DIGIT_BITS combination"
#endif /* CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 7 */
		0,
#endif /* CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 6 */
		0,
#endif /* CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 5 */
		0,
#endif /* CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 4 */
		0,
#endif /* CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 3 */
		0,
#endif /* CEILDIV(__SIZEOF_SIZE_T__ * __CHAR_BIT__, DIGIT_BITS) >= 2 */
		((Dee_digit_t)1 << ((__SIZEOF_SIZE_T__ * __CHAR_BIT__) % DIGIT_BITS))
	}
};

INTERN WUNUSED NONNULL((1)) int DCALL
cachedseq_index_inc(struct cachedseq_index *__restrict self) {
	if (self->csi_indexob)
		return int_inc(&self->csi_indexob);
	if unlikely(OVERFLOW_UADD(self->csi_index, 1, &self->csi_index)) {
		Dee_Incref(&int_SIZE_MAX_plus_1);
		self->csi_indexob = (DREF DeeIntObject *)&int_SIZE_MAX_plus_1;
	}
	return 0;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../int.c" */
int_compareint(DeeIntObject const *a, DeeIntObject const *b);

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cachedseq_index_compare(struct cachedseq_index const *__restrict lhs,
                        struct cachedseq_index const *__restrict rhs) {
	size_t lhs_asint;
	size_t rhs_asint;
	if (lhs->csi_indexob) {
		if (rhs->csi_indexob)
			return int_compareint(lhs->csi_indexob, rhs->csi_indexob);
		rhs_asint = rhs->csi_index;
		if (!DeeInt_TryAsSize((DeeObject *)lhs->csi_indexob, &lhs_asint))
			return Dee_COMPARE_GR; /* lhs > rhs  (reason: lhs > (size_t)-1) */
	} else {
		lhs_asint = lhs->csi_index;
		if (rhs->csi_indexob) {
			if (!DeeInt_TryAsSize((DeeObject *)rhs->csi_indexob, &rhs_asint))
				return Dee_COMPARE_LO; /* lhs < rhs  (reason: rhs > (size_t)-1) */
		} else {
			rhs_asint = rhs->csi_index;
		}
	}
	return Dee_Compare(lhs_asint, rhs_asint);
}


#define cswsogi_ctor cswgi_ctor
#define cswsgi_ctor cswgi_ctor
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_ctor(CachedSeq_WithGetItem *__restrict self) {
	Dee_atomic_lock_init(&self->cswgi_lock);
	self->cswgi_seq                 = NULL;
	self->cswgi_size.csi_indexob    = NULL;
	self->cswgi_size.csi_index      = 0;
	self->cswgi_maxsize.csi_indexob = NULL;
	self->cswgi_maxsize.csi_index   = 0;
	self->cswgi_loaded.csi_indexob  = NULL;
	self->cswgi_loaded.csi_index    = 0;
	objectlist_init(&self->cswgi_vector);
	indexbtab_init(&self->cswgi_btab);
	return 0;
}

#define cswsogi_init cswgi_init
#define cswsgi_init  cswgi_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_init(CachedSeq_WithGetItem *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if unlikely(argc != 1)
		return err_invalid_argc(DeeType_GetName(Dee_TYPE(self)), argc, 1, 1);
	self->cswgi_seq = argv[0];
	Dee_Incref(self->cswgi_seq);
	self->cswgi_size.csi_indexob    = NULL;
	self->cswgi_size.csi_index      = (size_t)-1;
	self->cswgi_maxsize.csi_indexob = NULL;
	self->cswgi_maxsize.csi_index   = (size_t)-1;
	self->cswgi_loaded.csi_indexob  = NULL;
	self->cswgi_loaded.csi_index    = 0;
	Dee_atomic_lock_init(&self->cswgi_lock);
	objectlist_init(&self->cswgi_vector);
	indexbtab_init(&self->cswgi_btab);
	return 0;
}

#define cswsogi_fini cswgi_fini
#define cswsgi_fini  cswgi_fini
PRIVATE NONNULL((1)) void DCALL
cswgi_fini(CachedSeq_WithGetItem *__restrict self) {
	size_t i;
	Dee_XDecref(self->cswgi_seq);
	cachedseq_index_fini(&self->cswgi_size);
	cachedseq_index_fini(&self->cswgi_maxsize);
	cachedseq_index_fini(&self->cswgi_loaded);
	Dee_XDecrefv(self->cswgi_vector.ol_elemv,
	             self->cswgi_vector.ol_elemc);
	Dee_objectlist_elemv_free(self->cswgi_vector.ol_elemv);
	for (i = 0; i < self->cswgi_btab.ibt_size; ++i) {
		Dee_Decref(self->cswgi_btab.ibt_elem[i].ibti_index);
		Dee_XDecref(self->cswgi_btab.ibt_elem[i].ibti_value);
	}
	Dee_Free(self->cswgi_btab.ibt_elem);
}

#define cswsogi_visit cswgi_visit
#define cswsgi_visit  cswgi_visit
PRIVATE NONNULL((1, 2)) void DCALL
cswgi_visit(CachedSeq_WithGetItem *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	CachedSeq_WithGetItem_LockAcquire(self);
	Dee_XVisit(self->cswgi_seq);
	/*Dee_XVisit(self->cswgi_size.csi_indexob);*/
	/*Dee_XVisit(self->cswgi_maxsize.csi_indexob);*/
	/*Dee_XVisit(self->cswgi_loaded.csi_indexob);*/
	Dee_XVisitv(self->cswgi_vector.ol_elemv,
	            self->cswgi_vector.ol_elemc);
	for (i = 0; i <= self->cswgi_btab.ibt_size; ++i) {
		/*Dee_Visit(self->cswgi_btab.ibt_elem[i].ibti_index);*/
		Dee_XVisit(self->cswgi_btab.ibt_elem[i].ibti_value);
	}
	CachedSeq_WithGetItem_LockRelease(self);
}

#define cswgi_deep   cswgi_copy
#define cswsogi_copy cswgi_copy
#define cswsogi_deep cswgi_copy
#define cswsgi_copy  cswgi_copy
#define cswsgi_deep  cswgi_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswgi_copy(CachedSeq_WithGetItem *__restrict self,
           CachedSeq_WithGetItem *__restrict other) {
	size_t i;
	DREF DeeObject **vector_copy;
	size_t vector_size;
	struct indexbtab_item *indexbtab_copy;
	size_t indexbtab_size;
again:
	CachedSeq_WithGetItem_LockAcquire(other);
	vector_size = other->cswgi_vector.ol_elemc;
	if (vector_size) {
		vector_copy = Dee_objectlist_elemv_trymalloc(vector_size);
		if unlikely(!vector_copy) {
			CachedSeq_WithGetItem_LockRelease(other);
			vector_copy = Dee_objectlist_elemv_malloc(vector_size);
			if unlikely(!vector_copy)
				goto err;
			CachedSeq_WithGetItem_LockAcquire(other);
			if unlikely(other->cswgi_vector.ol_elemc != vector_size) {
unlock_other_and_free_vector_copy_and_again:
				CachedSeq_WithGetItem_LockRelease(other);
				Dee_objectlist_elemv_free(vector_copy);
				goto again;
			}
		}
	}
again_copy_indexbtab_locked:
	indexbtab_size = other->cswgi_btab.ibt_size;
	indexbtab_copy = NULL;
	if (indexbtab_size) {
		indexbtab_copy = (struct indexbtab_item *)Dee_TryMallocc(indexbtab_size,
		                                                         sizeof(struct indexbtab_item));
		if unlikely(!indexbtab_copy) {
			CachedSeq_WithGetItem_LockRelease(other);
			indexbtab_copy = (struct indexbtab_item *)Dee_Mallocc(indexbtab_size,
			                                                      sizeof(struct indexbtab_item));
			if unlikely(!indexbtab_copy)
				goto err_vector_copy;
			CachedSeq_WithGetItem_LockAcquire(other);
			if unlikely(other->cswgi_btab.ibt_size != indexbtab_size - 1) {
				CachedSeq_WithGetItem_LockRelease(other);
				Dee_Free(indexbtab_copy);
				CachedSeq_WithGetItem_LockAcquire(other);
				goto again_copy_indexbtab_locked;
			}
			if unlikely(other->cswgi_vector.ol_elemc != vector_size)
				goto unlock_other_and_free_vector_copy_and_again;
		}
	}

	/* Copy indexbtab cache. */
	self->cswgi_btab.ibt_elem = indexbtab_copy;
	self->cswgi_btab.ibt_size = other->cswgi_btab.ibt_size;
	for (i = 0; i < self->cswgi_btab.ibt_size; ++i) {
		struct indexbtab_item *dst = &indexbtab_copy[i];
		struct indexbtab_item *src = &other->cswgi_btab.ibt_elem[i];
		dst->ibti_index = src->ibti_index;
		Dee_Incref(dst->ibti_index);
		dst->ibti_value = src->ibti_value;
		Dee_XIncref(dst->ibti_value);
	}

	/* Copy vector cache. */
	vector_copy = Dee_XMovrefv(vector_copy, other->cswgi_vector.ol_elemv, vector_size);
	self->cswgi_vector.ol_elemc = vector_size;
	self->cswgi_vector.ol_elemv = vector_copy;
	_Dee_objectlist_setalloc(&self->cswgi_vector, vector_size);

	/* Copy other caches/fields. */
	cachedseq_index_copy(&self->cswgi_size, &other->cswgi_size);
	cachedseq_index_copy(&self->cswgi_maxsize, &other->cswgi_maxsize);
	cachedseq_index_copy(&self->cswgi_loaded, &other->cswgi_loaded);
	self->cswgi_seq = other->cswgi_seq;
	Dee_XIncref(self->cswgi_seq);
	CachedSeq_WithGetItem_LockRelease(other);
	objectlist_init(&self->cswgi_vector);
	indexbtab_init(&self->cswgi_btab);
	Dee_atomic_lock_init(&self->cswgi_lock);
	return 0;
err_vector_copy:
	Dee_objectlist_elemv_free(vector_copy);
err:
	return -1;
}

#define cswsogi_deepload cswgi_deepload
#define cswsgi_deepload  cswgi_deepload
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_deepload(CachedSeq_WithGetItem *__restrict self) {
	size_t i;
	CachedSeq_WithGetItem_LockAcquire(self);
	for (i = 0; i < self->cswgi_vector.ol_elemc; ++i) {
		DREF DeeObject *new_vector_elem;
		DREF DeeObject *old_vector_elem;
		old_vector_elem = self->cswgi_vector.ol_elemv[i];
		if (!old_vector_elem)
			continue;
		CachedSeq_WithGetItem_LockRelease(self);
		new_vector_elem = DeeObject_DeepCopyInherited(old_vector_elem);
		if unlikely(!new_vector_elem)
			goto err;
		CachedSeq_WithGetItem_LockAcquire(self);
		old_vector_elem = self->cswgi_vector.ol_elemv[i]; /* Inherit reference */
		self->cswgi_vector.ol_elemv[i] = new_vector_elem; /* Inherit reference */
		CachedSeq_WithGetItem_LockRelease(self);
		Dee_XDecref(old_vector_elem);
		CachedSeq_WithGetItem_LockAcquire(self);
	}
	for (i = 0; i < self->cswgi_btab.ibt_size; ++i) {
		DREF DeeObject *old_indexbtab_elem;
		DREF DeeObject *new_indexbtab_elem;
		old_indexbtab_elem = self->cswgi_btab.ibt_elem[i].ibti_value;
		if (!old_indexbtab_elem)
			continue;
		Dee_Incref(old_indexbtab_elem);
		CachedSeq_WithGetItem_LockRelease(self);
		new_indexbtab_elem = DeeObject_DeepCopyInherited(old_indexbtab_elem);
		if unlikely(!new_indexbtab_elem)
			goto err;
		CachedSeq_WithGetItem_LockAcquire(self);
		old_indexbtab_elem = self->cswgi_btab.ibt_elem[i].ibti_value; /* Inherit reference */
		self->cswgi_btab.ibt_elem[i].ibti_value = new_indexbtab_elem; /* Inherit reference */
		CachedSeq_WithGetItem_LockRelease(self);
		Dee_XDecref(old_indexbtab_elem);
		CachedSeq_WithGetItem_LockAcquire(self);
	}
	CachedSeq_WithGetItem_LockRelease(self);
	return DeeObject_XInplaceDeepCopyWithLock(&self->cswgi_seq, &self->cswgi_lock);
err:
	return -1;
}


#define cswsogi_clear cswgi_clear
#define cswsgi_clear  cswgi_clear
PRIVATE NONNULL((1, 2)) void DCALL
cswgi_clear(CachedSeq_WithGetItem *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	DREF DeeObject *old_seq;
	DREF DeeIntObject *old_sizeob;
	DREF DeeIntObject *old_maxsizeob;
	DREF DeeIntObject *old_loadedob;
	size_t old_map_mask;
	struct indexbtab_item *old_map_elem;
	size_t old_vector_size;
	DREF DeeObject **old_vector_elem;
	CachedSeq_WithGetItem_LockAcquire(self);
	old_seq = self->cswgi_seq;
	self->cswgi_seq = NULL;
	old_sizeob = self->cswgi_size.csi_indexob;
	cachedseq_index_init_index(&self->cswgi_size, 0);
	old_maxsizeob = self->cswgi_maxsize.csi_indexob;
	cachedseq_index_init_index(&self->cswgi_maxsize, 0);
	old_loadedob = self->cswgi_loaded.csi_indexob;
	cachedseq_index_init_index(&self->cswgi_loaded, 0);
	old_vector_elem = self->cswgi_vector.ol_elemv;
	old_vector_size = self->cswgi_vector.ol_elemc;
	self->cswgi_vector.ol_elemv = NULL;
	self->cswgi_vector.ol_elemc = 0;
	old_map_elem = self->cswgi_btab.ibt_elem;
	old_map_mask = self->cswgi_btab.ibt_size;
	self->cswgi_btab.ibt_elem = NULL;
	self->cswgi_btab.ibt_size = 0;
	CachedSeq_WithGetItem_LockRelease(self);
	Dee_XDecref(old_seq);
	Dee_XDecref(old_sizeob);
	Dee_XDecref(old_maxsizeob);
	Dee_XDecref(old_loadedob);
	Dee_XDecrefv(old_vector_elem, old_vector_size);
	Dee_objectlist_elemv_free(old_vector_elem);
	for (i = 0; i <= old_map_mask; ++i) {
		if (!old_map_elem[i].ibti_index)
			continue;
		Dee_Visit(old_map_elem[i].ibti_index);
		Dee_XVisit(old_map_elem[i].ibti_value);
	}
	Dee_Free(old_map_elem);
}


#define cswsogi_gc cswgi_gc
#define cswsgi_gc  cswgi_gc
PRIVATE struct type_gc tpconst cswgi_gc = {
	/* .tp_clear */ (void (DCALL *)(DeeObject *__restrict))&cswgi_clear
};



#include <deemon/error.h>

/* @return: 0 : Size has been cached
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_loadsize(CachedSeq_WithGetItem *__restrict self) {
	/* TODO */
	(void)self;
	return DeeError_NOTIMPLEMENTED();
}

/* Returns the next (bound) index that is `>= min_index [&& <= end_index]'
 * NOTE: Initializes `result' on success (return == 0)
 * @param: end_index: when non-NULL, never return indices `>= end_index'.
 *                    Instead, temporarily act as if "self" ended at "end_index".
 * @return: 1 : No such index
 * @return: 0 : Success 
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
cswgi_indexafter(CachedSeq_WithGetItem *__restrict self,
                 /*out[1..1]*/ struct cachedseq_index *result,
                 /*in[1..1]*/ struct cachedseq_index const *min_index,
                 /*in[0..1]*/ struct cachedseq_index const *end_index) {
	/* TODO */
	(void)self;
	(void)result;
	(void)min_index;
	(void)end_index;
	return DeeError_NOTIMPLEMENTED();
}

#define CSWGI_GETITEM_ERROR   NULL
#define CSWGI_GETITEM_UNBOUND ITER_DONE
#define CSWGI_GETITEM_OOB     ((DeeObject *)-2l)
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswgi_getitem_index_ex(CachedSeq_WithGetItem *__restrict self, size_t index) {
	/* TODO */
	(void)self;
	(void)index;
	DeeError_NOTIMPLEMENTED();
	return CSWGI_GETITEM_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cswgi_getitem_object_ex(CachedSeq_WithGetItem *self, DeeObject *index) {
	/* TODO */
	(void)self;
	(void)index;
	DeeError_NOTIMPLEMENTED();
	return CSWGI_GETITEM_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cswgi_getitem_ex(CachedSeq_WithGetItem *self,
                 struct cachedseq_index const *index) {
	return index->csi_indexob ? cswgi_getitem_object_ex(self, (DeeObject *)index->csi_indexob)
	                          : cswgi_getitem_index_ex(self, index->csi_index);
}

#define cswsogi_sizeob cswgi_sizeob
#define cswsgi_sizeob  cswgi_sizeob
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswgi_sizeob(CachedSeq_WithGetItem *__restrict self) {
	struct cachedseq_index result;
	if unlikely(cswgi_loadsize(self))
		goto err;
	CachedSeq_WithGetItem_LockAcquire(self);
	cachedseq_index_copy(&result, &self->cswgi_size);
	CachedSeq_WithGetItem_LockRelease(self);
	if (result.csi_indexob != NULL)
		return Dee_AsObject(result).csi_indexob;
	return DeeInt_NewSize(result.csi_index);
err:
	return NULL;
}

#define cswsogi_size cswgi_size
#define cswsgi_size  cswgi_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cswgi_size(CachedSeq_WithGetItem *__restrict self) {
	struct cachedseq_index result;
	if unlikely(cswgi_loadsize(self))
		goto err;
	CachedSeq_WithGetItem_LockAcquire(self);
	cachedseq_index_copy(&result, &self->cswgi_size);
	CachedSeq_WithGetItem_LockRelease(self);
	if (result.csi_indexob != NULL) {
		int temp;
		temp = DeeInt_AsSize(Dee_AsObject(result.csi_indexob), &result.csi_index);
		Dee_Decref(result.csi_indexob);
		if unlikely(temp)
			goto err;
	}
	if unlikely(result.csi_index == (size_t)-1)
		DeeRT_ErrIntegerOverflowU(result.csi_index, (size_t)-2);
	return result.csi_index;
err:
	return (size_t)-1;
}

#define cswsogi_size_fast cswgi_size_fast
#define cswsgi_size_fast  cswgi_size_fast
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cswgi_size_fast(CachedSeq_WithGetItem *__restrict self) {
	size_t result = (size_t)-1;
	CachedSeq_WithGetItem_LockAcquire(self);
	if (!self->cswgi_seq && !self->cswgi_size.csi_indexob)
		result = self->cswgi_size.csi_index;
	CachedSeq_WithGetItem_LockRelease(self);
	return result;
}


#define cswsogi_getitem cswgi_getitem
#define cswsgi_getitem  cswgi_getitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cswgi_getitem(CachedSeq_WithGetItem *self, DeeObject *index) {
	DREF DeeObject *result = cswgi_getitem_object_ex(self, index);
	if unlikely(result == CSWGI_GETITEM_UNBOUND) {
		DeeRT_ErrUnboundIndexObj(Dee_AsObject(self), index);
		goto err;
	}
	if unlikely(result == CSWGI_GETITEM_OOB) {
		DREF DeeObject *sizeob = cswgi_sizeob(self);
		if unlikely(!sizeob)
			goto err;
		DeeRT_ErrIndexOutOfBoundsObj(Dee_AsObject(self), index, sizeob);
		Dee_Decref_unlikely(sizeob);
		goto err;
	}
	return result;
err:
	return NULL;
}

#define cswsogi_bounditem cswgi_bounditem
#define cswsgi_bounditem  cswgi_bounditem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswgi_bounditem(CachedSeq_WithGetItem *self, DeeObject *index) {
	DREF DeeObject *result = cswgi_getitem_object_ex(self, index);
	if (result == CSWGI_GETITEM_UNBOUND)
		return Dee_BOUND_NO;
	if (result == CSWGI_GETITEM_OOB)
		return Dee_BOUND_MISSING;
	if (result == CSWGI_GETITEM_ERROR)
		return Dee_BOUND_ERR;
	Dee_Decref_unlikely(result);
	return Dee_BOUND_YES;
}

#define cswsogi_bounditem_index cswgi_bounditem_index
#define cswsgi_bounditem_index  cswgi_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_bounditem_index(CachedSeq_WithGetItem *self, size_t index) {
	DREF DeeObject *result = cswgi_getitem_index_ex(self, index);
	if (result == CSWGI_GETITEM_UNBOUND)
		return Dee_BOUND_NO;
	if (result == CSWGI_GETITEM_OOB)
		return Dee_BOUND_MISSING;
	if (result == CSWGI_GETITEM_ERROR)
		return Dee_BOUND_ERR;
	Dee_Decref_unlikely(result);
	return Dee_BOUND_YES;
}

#define cswsogi_hasitem cswgi_hasitem
#define cswsgi_hasitem  cswgi_hasitem
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define cswgi_hasitem cswgi_bounditem
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswgi_hasitem(CachedSeq_WithGetItem *self, DeeObject *index) {
	DREF DeeObject *result = cswgi_getitem_object_ex(self, index);
	if (result == CSWGI_GETITEM_UNBOUND)
		return Dee_HAS_NO;
	if (result == CSWGI_GETITEM_OOB)
		return Dee_HAS_NO;
	if (result == CSWGI_GETITEM_ERROR)
		return Dee_HAS_ERR;
	Dee_Decref_unlikely(result);
	return Dee_HAS_YES;
}
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

#define cswsogi_hasitem_index cswgi_hasitem_index
#define cswsgi_hasitem_index  cswgi_hasitem_index
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define cswgi_hasitem_index cswgi_bounditem_index
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_hasitem_index(CachedSeq_WithGetItem *self, size_t index) {
	DREF DeeObject *result = cswgi_getitem_index_ex(self, index);
	if (result == CSWGI_GETITEM_UNBOUND)
		return Dee_HAS_NO;
	if (result == CSWGI_GETITEM_OOB)
		return Dee_HAS_NO;
	if (result == CSWGI_GETITEM_ERROR)
		return Dee_HAS_ERR;
	Dee_Decref_unlikely(result);
	return Dee_HAS_YES;
}
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

#define cswsogi_trygetitem cswgi_trygetitem
#define cswsgi_trygetitem  cswgi_trygetitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cswgi_trygetitem(CachedSeq_WithGetItem *self, DeeObject *index) {
	DREF DeeObject *result = cswgi_getitem_object_ex(self, index);
	STATIC_ASSERT((uintptr_t)CSWGI_GETITEM_UNBOUND == (uintptr_t)ITER_DONE);
	if (result == CSWGI_GETITEM_OOB)
		result = ITER_DONE;
	return result;
}

#define cswsogi_getitem_index cswgi_getitem_index
#define cswsgi_getitem_index  cswgi_getitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswgi_getitem_index(CachedSeq_WithGetItem *self, size_t index) {
	DREF DeeObject *result = cswgi_getitem_index_ex(self, index);
	if unlikely(result == CSWGI_GETITEM_UNBOUND) {
		DeeRT_ErrUnboundIndex(self, index);
		goto err;
	}
	if unlikely(result == CSWGI_GETITEM_OOB) {
		DREF DeeObject *indexob;
		DREF DeeObject *sizeob = cswgi_sizeob(self);
		if unlikely(!sizeob)
			goto err;
		indexob = DeeInt_NewSize(index);
		if likely(indexob) {
			DeeRT_ErrIndexOutOfBoundsObj(Dee_AsObject(self), indexob, sizeob);
			Dee_Decref_likely(indexob);
		}
		Dee_Decref_unlikely(sizeob);
		goto err;
	}
	return result;
err:
	return NULL;
}

#define cswsogi_trygetitem_index cswgi_trygetitem_index
#define cswsgi_trygetitem_index  cswgi_trygetitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswgi_trygetitem_index(CachedSeq_WithGetItem *self, size_t index) {
	DREF DeeObject *result = cswgi_getitem_index_ex(self, index);
	STATIC_ASSERT((uintptr_t)CSWGI_GETITEM_UNBOUND == (uintptr_t)ITER_DONE);
	if (result == CSWGI_GETITEM_OOB)
		result = ITER_DONE;
	return result;
}


#define cswsogi_foreach cswgi_foreach
#define cswsgi_foreach  cswgi_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cswgi_foreach(CachedSeq_WithGetItem *self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct cachedseq_index prev_index;
	cachedseq_index_init_index(&prev_index, 0);
	for (;;) {
		int status;
		DREF DeeObject *value;
		struct cachedseq_index next_index;
		status = cswgi_indexafter(self, &prev_index, &next_index, NULL);
		if unlikely(status < 0)
			goto err_prev_index;
		if (status > 0)
			break;
		cachedseq_index_fini(&prev_index);
		cachedseq_index_move(&prev_index, &next_index);
		value = cswgi_getitem_ex(self, &prev_index);
		if unlikely(value == CSWGI_GETITEM_ERROR)
			goto err_prev_index;
		if unlikely(value == CSWGI_GETITEM_OOB)
			break;
		if likely(value != CSWGI_GETITEM_UNBOUND) {
			temp = (*cb)(arg, value);
			Dee_Decref(value);
			if unlikely(temp < 0)
				goto err_prev_index_temp;
			result += temp;
		}
		if (cachedseq_index_inc(&prev_index))
			goto err_prev_index;
	}
	cachedseq_index_fini(&prev_index);
	return result;
err_prev_index_temp:
	cachedseq_index_fini(&prev_index);
	return temp;
err_prev_index:
	cachedseq_index_fini(&prev_index);
/*err:*/
	return -1;
}


#define cswsogi_mh_seq_enumerate cswgi_mh_seq_enumerate
#define cswsgi_mh_seq_enumerate  cswgi_mh_seq_enumerate
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cswgi_mh_seq_enumerate(CachedSeq_WithGetItem *self, Dee_seq_enumerate_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct cachedseq_index prev_index;
	cachedseq_index_init_index(&prev_index, 0);
	for (;;) {
		int status;
		DREF DeeObject *value;
		struct cachedseq_index next_index;
		status = cswgi_indexafter(self, &prev_index, &next_index, NULL);
		if unlikely(status < 0)
			goto err_prev_index;
		if (status > 0)
			break;
		cachedseq_index_fini(&prev_index);
		cachedseq_index_move(&prev_index, &next_index);
		value = cswgi_getitem_ex(self, &prev_index);
		if unlikely(value == CSWGI_GETITEM_ERROR)
			goto err_prev_index;
		if unlikely(value == CSWGI_GETITEM_OOB)
			break;
		if likely(value != CSWGI_GETITEM_UNBOUND) {
			if (prev_index.csi_indexob) {
				temp = (*cb)(arg, (DeeObject *)prev_index.csi_indexob, value);
			} else {
				DREF DeeObject *indexob;
				indexob = DeeInt_NewSize(prev_index.csi_index);
				if likely(indexob) {
					temp = (*cb)(arg, indexob, value);
					Dee_Decref(indexob);
				} else {
					temp = -1;
				}
			}
			Dee_Decref(value);
			if unlikely(temp < 0)
				goto err_prev_index_temp;
			result += temp;
		}
		if (cachedseq_index_inc(&prev_index))
			goto err_prev_index;
	}
	cachedseq_index_fini(&prev_index);
	return result;
err_prev_index_temp:
	cachedseq_index_fini(&prev_index);
	return temp;
err_prev_index:
	cachedseq_index_fini(&prev_index);
/*err:*/
	return -1;
}

#define cswsogi_mh_seq_enumerate_index cswgi_mh_seq_enumerate_index
#define cswsgi_mh_seq_enumerate_index  cswgi_mh_seq_enumerate_index
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cswgi_mh_seq_enumerate_index(CachedSeq_WithGetItem *self, Dee_seq_enumerate_index_t cb,
                             void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct cachedseq_index prev_index;
	struct cachedseq_index end_index;
	cachedseq_index_init_index(&prev_index, start);
	cachedseq_index_init_index(&end_index, end);
	for (;;) {
		int status;
		DREF DeeObject *value;
		struct cachedseq_index next_index;
		status = cswgi_indexafter(self, &prev_index, &next_index, &end_index);
		if unlikely(status < 0)
			goto err_prev_index;
		if (status > 0)
			break;
		cachedseq_index_fini(&prev_index);
		cachedseq_index_move(&prev_index, &next_index);
		value = cswgi_getitem_ex(self, &prev_index);
		if unlikely(value == CSWGI_GETITEM_ERROR)
			goto err_prev_index;
		if unlikely(value == CSWGI_GETITEM_OOB)
			break;
		if likely(value != CSWGI_GETITEM_UNBOUND) {
			if (!prev_index.csi_indexob) {
				temp = (*cb)(arg, prev_index.csi_index, value);
			} else {
				size_t int_index;
				if unlikely(!DeeInt_TryAsSize((DeeObject *)prev_index.csi_indexob, &int_index)) {
					Dee_Decref(value);
					break;
				}
				temp = (*cb)(arg, int_index, value);
			}
			Dee_Decref(value);
			if unlikely(temp < 0)
				goto err_prev_index_temp;
			result += temp;
		}
		if (cachedseq_index_inc(&prev_index))
			goto err_prev_index;
	}
	cachedseq_index_fini(&prev_index);
	return result;
err_prev_index_temp:
	cachedseq_index_fini(&prev_index);
	return temp;
err_prev_index:
	cachedseq_index_fini(&prev_index);
/*err:*/
	return -1;
}

#define cswsogi_iter cswgi_iter
#define cswsgi_iter  cswgi_iter
PRIVATE WUNUSED NONNULL((1)) DREF CachedSeq_WithGetItem_Iterator *DCALL
cswgi_iter(CachedSeq_WithGetItem *__restrict self) {
	DREF CachedSeq_WithGetItem_Iterator *result;
	result = DeeObject_MALLOC(CachedSeq_WithGetItem_Iterator);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->cswgii_cache = self;
	cachedseq_index_init_index(&result->cswgii_nextindex, 0);
	Dee_atomic_lock_init(&result->cswgii_lock);
	DeeObject_Init(result, &CachedSeq_WithGetItem_Iterator_Type);
	return result;
err:
	return NULL;
}

#define cswsogi_bool cswgi_bool
#define cswsgi_bool  cswgi_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
cswgi_bool(CachedSeq_WithGetItem *__restrict self) {
	int status;
	struct cachedseq_index prev_index;
	struct cachedseq_index next_index;
	cachedseq_index_init_index(&prev_index, 0);
	status = cswgi_indexafter(self, &prev_index, &next_index, NULL);
	cachedseq_index_fini(&next_index);
	ASSERT(prev_index.csi_indexob == NULL); /*cachedseq_index_fini(&prev_index);*/
	if (status >= 0)
		status = !status;
	return status;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cswgi_populate_impl(CachedSeq_WithGetItem *__restrict self,
                    struct cachedseq_index *__restrict start,
                    /*0..1*/ struct cachedseq_index *end) {
	int status = 0;
	struct cachedseq_index prev_index;
	struct cachedseq_index next_index;
	CachedSeq_WithGetItem_LockAcquire(self);
	cachedseq_index_copy(&prev_index, &self->cswgi_loaded);
	CachedSeq_WithGetItem_LockRelease(self);
	if (cachedseq_index_compare(&prev_index, start) < 0) {
		cachedseq_index_fini(&prev_index);
		cachedseq_index_copy(&prev_index, start);
	}
	for (;;) {
		CachedSeq_WithGetItem_LockAcquire(self);
		if (self->cswgi_seq == NULL) {
			/* Everything has been cached */
			CachedSeq_WithGetItem_LockRelease(self);
			break;
		}
		CachedSeq_WithGetItem_LockRelease(self);
		status = cswgi_indexafter(self, &next_index, &prev_index, end);
		if (status != 0)
			break;
		cachedseq_index_fini(&prev_index);
		cachedseq_index_move(&prev_index, &next_index);
	}
	cachedseq_index_fini(&prev_index);
	return status;
}


#define cswsogi_populate cswgi_populate
#define cswsgi_populate  cswgi_populate
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cswgi_populate(CachedSeq_WithGetItem *__restrict self,
               size_t argc, DeeObject *const *argv) {
	int status;
	struct cachedseq_index start, end;
	if (argc == 0) {
		cachedseq_index_init_index(&start, 0);
		status = cswgi_populate_impl(self, &start, NULL);
	} else if (argc == 1) {
		end.csi_indexob = (DREF DeeIntObject *)DeeObject_Int(argv[0]);
		if unlikely(!end.csi_indexob)
			goto err;
		cachedseq_index_init_index(&start, 0);
		status = cswgi_populate_impl(self, &start, &end);
		cachedseq_index_fini(&end);
	} else if (argc == 2) {
		start.csi_indexob = (DREF DeeIntObject *)DeeObject_Int(argv[0]);
		if unlikely(!start.csi_indexob)
			goto err;
		end.csi_indexob = (DREF DeeIntObject *)DeeObject_Int(argv[1]);
		if unlikely(!end.csi_indexob)
			goto err_start;
		status = cswgi_populate_impl(self, &start, &end);
		cachedseq_index_fini(&end);
		cachedseq_index_fini(&start);
	} else {
		err_invalid_argc("populate", argc, 0, 2);
		goto err;
	}
	if unlikely(status < 0)
		goto err;
	return_none;
err_start:
	cachedseq_index_fini(&start);
err:
	return NULL;
}

#define cswsogi_methods cswgi_methods
#define cswsgi_methods  cswgi_methods
PRIVATE struct type_method cswgi_methods[] = {
	TYPE_METHOD("populate", &cswgi_populate,
	            "()\n"
	            "(end:?Dint)\n"
	            "(start:?Dint,end:?Dint)\n"
	            "Populate the entire cache, or the specified range of indices"),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

#define cswsogi_getsets cswgi_getsets
#define cswsgi_getsets  cswgi_getsets
PRIVATE struct type_getset cswgi_getsets[] = {
	/* TODO */
	TYPE_GETSET_END
};

#define cswsogi_members cswgi_members
#define cswsgi_members  cswgi_members
PRIVATE struct type_member cswgi_members[] = {
	TYPE_MEMBER_FIELD("__vector_cache_size__", STRUCT_SIZE_T | STRUCT_ATOMIC | STRUCT_CONST,
	                  offsetof(CachedSeq_WithGetItem, cswgi_vector.ol_elemc)),
	TYPE_MEMBER_FIELD("__btab_cache_size__", STRUCT_SIZE_T | STRUCT_ATOMIC | STRUCT_CONST,
	                  offsetof(CachedSeq_WithGetItem, cswgi_btab.ibt_size)),
	TYPE_MEMBER_END
};

#define cswsogi_class_members cswgi_class_members
#define cswsgi_class_members  cswgi_class_members
PRIVATE struct type_member cswgi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &CachedSeq_WithGetItem_Iterator_Type),
	TYPE_MEMBER_END
};


#define cswsogi_seq cswgi_seq
#define cswsgi_seq  cswgi_seq
PRIVATE struct type_seq cswgi_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cswgi_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cswgi_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cswgi_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&cswgi_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&cswgi_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cswgi_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cswgi_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&cswgi_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cswgi_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&cswgi_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&cswgi_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cswgi_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cswgi_trygetitem_index,
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
	/* .tp_asvector                   = */ NULL
};

#define cswsogi_method_hints cswgi_method_hints
#define cswsgi_method_hints  cswgi_method_hints
PRIVATE struct type_method_hint tpconst cswgi_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate, &cswgi_mh_seq_enumerate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &cswgi_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

INTERN DeeTypeObject CachedSeq_WithGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedSeqWithGetItem",
	/* .tp_doc      = */ DOC("()\n"
	                         "(objWithGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CachedSeq_WithGetItem,
			/* tp_ctor:        */ &cswgi_ctor,
			/* tp_copy_ctor:   */ &cswgi_copy,
			/* tp_deep_ctor:   */ &cswgi_deep,
			/* tp_any_ctor:    */ &cswgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cswgi_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswgi_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cswgi_visit,
	/* .tp_gc            = */ &cswgi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cswgi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cswgi_methods,
	/* .tp_getsets       = */ cswgi_getsets,
	/* .tp_members       = */ cswgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cswgi_class_members,
	/* .tp_method_hints  = */ cswgi_method_hints,
};

INTERN DeeTypeObject CachedSeq_WithSizeObAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedSeqWithSizeObAndGetItem",
	/* .tp_doc      = */ DOC("()\n"
	                         "(objWithSizeAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CachedSeq_WithIter,
			/* tp_ctor:        */ &cswsogi_ctor,
			/* tp_copy_ctor:   */ &cswsogi_copy,
			/* tp_deep_ctor:   */ &cswsogi_deep,
			/* tp_any_ctor:    */ &cswsogi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswsogi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cswsogi_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswsogi_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cswsogi_visit,
	/* .tp_gc            = */ &cswsogi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cswsogi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cswsogi_methods,
	/* .tp_getsets       = */ cswsogi_getsets,
	/* .tp_members       = */ cswsogi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cswsogi_class_members,
	/* .tp_method_hints  = */ cswsogi_method_hints,
};

INTERN DeeTypeObject CachedSeq_WithSizeAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedSeqWithSizeAndGetItemIndex",
	/* .tp_doc      = */ DOC("()\n"
	                         "(objWithSizeAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CachedSeq_WithIter,
			/* tp_ctor:        */ &cswsgi_ctor,
			/* tp_copy_ctor:   */ &cswsgi_copy,
			/* tp_deep_ctor:   */ &cswsgi_deep,
			/* tp_any_ctor:    */ &cswsgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswsgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cswsgi_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswsgi_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cswsgi_visit,
	/* .tp_gc            = */ &cswsgi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cswsgi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cswsgi_methods,
	/* .tp_getsets       = */ cswsgi_getsets,
	/* .tp_members       = */ cswsgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cswsgi_class_members,
	/* .tp_method_hints  = */ cswsgi_method_hints,
};





INTERN DeeTypeObject CachedSeq_WithGetItem_Iterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CachedSeqWithGetItemIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(base:?X3"
	                         /**/ "?Ert:CachedSeqWithGetItem"
	                         /**/ "?Ert:CachedSeqWithSizeObAndGetItem"
	                         /**/ "?Ert:CachedSeqWithSizeAndGetItem"
	                         ",index=!0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL, /* Not GC because "cswgii_nextindex" can only reference int-objects. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ CachedSeq_WithGetItem_Iterator,
			/* tp_ctor:        */ &cswgiiter_ctor,
			/* tp_copy_ctor:   */ &cswgiiter_copy,
			/* tp_deep_ctor:   */ &cswgiiter_deep,
			/* tp_any_ctor:    */ &cswgiiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cswgiiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cswgiiter_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cswgiiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cswgiiter_next,
	/* .tp_iterator      = */ &cswgiiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cswgiiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
#endif /* HAVE_CachedSeq_WithGetItem */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CACHED_SEQ_C */
