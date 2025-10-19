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
#ifndef GUARD_DEX_COLLECTIONS_FIXEDLIST_C
#define GUARD_DEX_COLLECTIONS_FIXEDLIST_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>

/**/
#include "kwlist.h"

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

PRIVATE WUNUSED DREF FixedList *DCALL fl_ctor(void) {
	DREF FixedList *result;
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem));
	if unlikely(!result)
		goto err;
	Dee_atomic_rwlock_init(&result->fl_lock);
	result->fl_size = 0;
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return (DREF FixedList *)DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF FixedList *DCALL
fl_copy(FixedList *__restrict self) {
	DREF FixedList *result;
	result = (DREF FixedList *)DeeGCObject_Mallocc(offsetof(FixedList, fl_elem),
	                                               self->fl_size, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	Dee_atomic_rwlock_init(&result->fl_lock);
	result->fl_size = self->fl_size;
	FixedList_LockRead(self);
	Dee_XMovrefv(result->fl_elem, self->fl_elem, self->fl_size);
	FixedList_LockEndRead(self);
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return (DREF FixedList *)DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

#define FixedList_InitEnumerate_GetAllocatedSize(self) \
	((size_t)(uintptr_t)(self)->ob_weakrefs.wl_nodes)
#define FixedList_InitEnumerate_SetAllocatedSize(self, v) \
	(void)((self)->ob_weakrefs.wl_nodes = (struct Dee_weakref *)(uintptr_t)(size_t)(v))

PRIVATE WUNUSED Dee_ssize_t DCALL
fl_init_enumerate_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	DREF FixedList *self = *(DREF FixedList **)arg;
	if unlikely(index >= self->fl_size) {
		size_t alloc = FixedList_InitEnumerate_GetAllocatedSize(self);
		ASSERT(self->fl_size <= alloc);
		if (index >= alloc) {
			/* Increase allocated size */
			DREF FixedList *new_self;
			size_t new_alloc = alloc * 2;
			if unlikely(new_alloc <= index)
				new_alloc = index + 1;
			new_self = (DREF FixedList *)DeeGCObject_TryReallocc(self, offsetof(FixedList, fl_elem),
			                                                     new_alloc, sizeof(DREF DeeObject *));
			if unlikely(!new_self) {
				new_alloc = index + 1;
				new_self = (DREF FixedList *)DeeGCObject_Reallocc(self, offsetof(FixedList, fl_elem),
				                                                  new_alloc, sizeof(DREF DeeObject *));
				if unlikely(!new_self)
					goto err;
			}
			*(DREF FixedList **)arg = self = new_self;
			FixedList_InitEnumerate_SetAllocatedSize(self, new_alloc);
		}
		self->fl_size = index + 1;
	}
	if (!value)
		return 0;
	self->fl_elem[index] = value;
	Dee_Incref(value);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED DREF FixedList *DCALL
fl_init(size_t argc, DeeObject *const *argv) {
	DREF FixedList *result;
	DeeObject *sizeob_or_seq, *init = NULL;
	DeeArg_Unpack1Or2(err, argc, argv, "FixedList", &sizeob_or_seq, &init);
	if (init) {
		size_t size;
		if (DeeObject_AsSize(sizeob_or_seq, &size))
			goto err;
		result = (DREF FixedList *)DeeGCObject_MalloccSafe(offsetof(FixedList, fl_elem),
		                                                   size, sizeof(DREF DeeObject *));
		if unlikely(!result)
			goto err;
		Dee_atomic_rwlock_init(&result->fl_lock);
		result->fl_size = size;
		Dee_Incref_n(init, size);
		memsetp(result->fl_elem, init, size);
	} else if (DeeInt_Check(sizeob_or_seq)) {
		size_t size;
		if (DeeObject_AsSize(sizeob_or_seq, &size))
			goto err;
		result = (DREF FixedList *)DeeGCObject_CalloccSafe(offsetof(FixedList, fl_elem),
		                                                   size, sizeof(DREF DeeObject *));
		if unlikely(!result)
			goto err;
		Dee_atomic_rwlock_cinit(&result->fl_lock);
		result->fl_size = size;
	} else {
		size_t size;
		/* (x as Sequence).operator # () */
		size = (*DeeSeq_Type.tp_seq->tp_size)(sizeob_or_seq);
		if unlikely(size == (size_t)-1)
			goto err;
		result = (DREF FixedList *)DeeGCObject_CalloccSafe(offsetof(FixedList, fl_elem),
		                                                   size, sizeof(DREF DeeObject *));
		if unlikely(!result)
			goto err;
		Dee_atomic_rwlock_cinit(&result->fl_lock);
		FixedList_InitEnumerate_SetAllocatedSize(result, size);
		/* (x as Sequence).enumerate((index, value?) -> fl_init_enumerate_cb(index, value)) */
		if unlikely(DeeObject_InvokeMethodHint(seq_enumerate_index, sizeob_or_seq,
		                                       &fl_init_enumerate_cb,
		                                       &result, 0, (size_t)-1))
			goto err_r_elem;
		result->fl_size = size;
	}
/*done:*/
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return (DREF FixedList *)DeeGC_Track((DeeObject *)result);
err_r_elem:
	Dee_XDecrefv(result->fl_elem, result->fl_size);
/*err_r:*/
	DeeGCObject_Free(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
fl_fini(FixedList *__restrict self) {
	weakref_support_fini(self);
	Dee_XDecrefv(self->fl_elem, self->fl_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fl_assign(FixedList *__restrict self, DeeObject *__restrict other) {
	DREF DeeObject **items, *temp;
	size_t i;
	items = (DREF DeeObject **)Dee_Mallocac(self->fl_size,
	                                        sizeof(DREF DeeObject *));
	if unlikely(!items)
		goto err;
	if (DeeSeq_Unpack(other, self->fl_size, items))
		goto err_items;
	FixedList_LockWrite(self);
	/* Exchange all stored items. */
	for (i = 0; i < self->fl_size; ++i) {
		temp             = self->fl_elem[i];
		self->fl_elem[i] = items[i];
		items[i]         = temp;
	}
	FixedList_LockEndWrite(self);
	/* Drop references to all of the old items. */
	Dee_XDecrefv(items, self->fl_size);
	Dee_Freea(items);
	return 0;
err_items:
	Dee_Freea(items);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fl_moveassign(FixedList *__restrict self,
              FixedList *__restrict other) {
	DREF DeeObject **items;
	if (self == other)
		return 0;
	if unlikely(self->fl_size != other->fl_size) {
		return DeeError_Throwf(&DeeError_UnpackError,
		                       "Expected a fixed list containing %" PRFuSIZ " object%s "
		                       "when one containing %" PRFuSIZ " was given",
		                       self->fl_size, self->fl_size > 1 ? "s" : "", other->fl_size);
	}
	items = (DREF DeeObject **)Dee_Mallocac(self->fl_size,
	                                        sizeof(DREF DeeObject *));
	if unlikely(!items)
		goto err;
#ifndef CONFIG_NO_THREADS
write_self_again:
	FixedList_LockWrite(self);
	if (!FixedList_LockTryWrite(other)) {
		FixedList_LockEndWrite(self);
		FixedList_LockWrite(other);
		if (!FixedList_LockTryWrite(self)) {
			FixedList_LockEndWrite(other);
			goto write_self_again;
		}
	}
#endif /* !CONFIG_NO_THREADS */

	/* Transfer objects. */
	memcpyc(items, self->fl_elem, self->fl_size, sizeof(DREF DeeObject *));          /* Backup old objects. */
	memcpyc(self->fl_elem, other->fl_elem, self->fl_size, sizeof(DREF DeeObject *)); /* Copy new objects. */
	bzeroc(other->fl_elem, self->fl_size, sizeof(DREF DeeObject *));                 /* Steal references. */
	FixedList_LockEndWrite(other);
	FixedList_LockEndWrite(self);

	/* Drop references to all of the old items. */
	Dee_XDecrefv(items, self->fl_size);
	Dee_Freea(items);
	return 0;
/*
err_items:
	Dee_Freea(items);*/
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_deepload(FixedList *__restrict self) {
	size_t i;
	for (i = 0; i < self->fl_size; ++i) {
		if (DeeObject_XInplaceDeepCopyWithRWLock(&self->fl_elem[i],
		                                         &self->fl_lock))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL fl_bool(FixedList *__restrict self) {
	return self->fl_size != 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
fl_visit(FixedList *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	FixedList_LockRead(self);
	for (i = 0; i < self->fl_size; ++i)
		Dee_XVisit(self->fl_elem[i]);
	FixedList_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
fl_clear_impl(FixedList *__restrict self) {
	size_t i, buflen = 0;
	DREF DeeObject *buf[16];
again:
	FixedList_LockWrite(self);
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *ob;
		ob = self->fl_elem[i];
		if (!ob)
			continue;
		self->fl_elem[i] = NULL;
		if (buflen >= COMPILER_LENOF(buf)) {
			FixedList_LockEndWrite(self);
			Dee_Decref(ob);
			Dee_Decrefv(buf, buflen);
			goto again;
		}
		buf[buflen++] = ob; /* Inherit reference. */
	}
	FixedList_LockEndWrite(self);
	/* Drop all of the references we took. */
	Dee_Decrefv(buf, buflen);
}

PRIVATE void DCALL
fl_pclear(FixedList *__restrict self, unsigned int gc_priority) {
	size_t i, buflen = 0;
	DREF DeeObject *buf[16];
again:
	FixedList_LockWrite(self);
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *ob;
		ob = self->fl_elem[i];
		if (!ob)
			continue;
		if (DeeObject_GCPriority(ob) < gc_priority)
			continue; /* Ignore this object (for now) */
		self->fl_elem[i] = NULL;
		if (buflen >= COMPILER_LENOF(buf)) {
			FixedList_LockEndWrite(self);
			Dee_Decref(ob);
			Dee_Decrefv(buf, buflen);
			goto again;
		}
		buf[buflen++] = ob; /* Inherit reference. */
	}
	FixedList_LockEndWrite(self);
	/* Drop all of the references we took. */
	Dee_Decrefv(buf, buflen);
}


PRIVATE struct type_gc tpconst fl_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&fl_clear_impl,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&fl_pclear
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
fl_size(FixedList *__restrict self) {
	return self->fl_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_getitem_index(FixedList *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->fl_size) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(result);
	FixedList_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_getitem_index_fast(FixedList *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->fl_size);
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	Dee_XIncref(result);
	FixedList_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_trygetitem_index(FixedList *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->fl_size)
		goto nope;
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		goto nope;
	}
	Dee_Incref(result);
	FixedList_LockEndRead(self);
	return result;
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_bounditem_index(FixedList *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->fl_size)
		return Dee_BOUND_MISSING;
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		return Dee_BOUND_NO;
	}
	Dee_Incref(result);
	FixedList_LockEndRead(self);
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_hasitem_index(FixedList *__restrict self, size_t index) {
	return index < self->fl_size ? 1 : 0;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_delitem_index(FixedList *__restrict self, size_t index) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->fl_size) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	FixedList_LockRead(self);
	oldval = self->fl_elem[index];
	self->fl_elem[index] = NULL;
	FixedList_LockEndRead(self);
	Dee_XDecref(oldval);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
fl_setitem_index(FixedList *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->fl_size) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	Dee_Incref(value);
	FixedList_LockRead(self);
	oldval               = self->fl_elem[index];
	self->fl_elem[index] = value;
	FixedList_LockEndRead(self);
	Dee_XDecref(oldval);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
fl_contains(FixedList *self, DeeObject *value) {
	size_t i;
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *elem;
		int temp;
		FixedList_LockRead(self);
		elem = self->fl_elem[i];
		Dee_Incref(elem);
		FixedList_LockEndRead(self);
		temp = DeeObject_TryCompareEq(value, elem);
		Dee_Decref(elem);
		if unlikely(temp == Dee_COMPARE_ERR)
			goto err;
		if (temp == 0)
			return_true;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF FixedListIterator *DCALL
fl_iter(FixedList *__restrict self) {
	DREF FixedListIterator *result;
	result = DeeObject_MALLOC(FixedListIterator);
	if unlikely(!result)
		goto done;
	result->li_list = self;
	result->li_iter = 0;
	Dee_Incref(self);
	DeeObject_Init(result, &FixedListIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_getrange_index(FixedList *__restrict self, Dee_ssize_t i_begin, Dee_ssize_t i_end) {
	DREF FixedList *result;
	struct Dee_seq_range range;
	size_t range_size;
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->fl_size);
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size <= 0)
		return_reference_(Dee_EmptySeq);
	result = (DREF FixedList *)DeeGCObject_Mallocc(offsetof(FixedList, fl_elem),
	                                               range_size, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	Dee_atomic_rwlock_init(&result->fl_lock);
	result->fl_size = range_size;
	FixedList_LockRead(self);
	Dee_XMovrefv(result->fl_elem,
	             self->fl_elem + range.sr_start,
	             range_size);
	FixedList_LockEndRead(self);
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_getrange_index_n(FixedList *__restrict self, Dee_ssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return fl_getrange_index(self, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	DREF FixedList *result;
	size_t start, range_size;
	start = DeeSeqRange_Clamp_n(i_begin, self->fl_size);
	range_size = self->fl_size - start;
	if unlikely(range_size <= 0)
		return_reference_(Dee_EmptySeq);
	result = (DREF FixedList *)DeeGCObject_Mallocc(offsetof(FixedList, fl_elem),
	                                               range_size, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	Dee_atomic_rwlock_init(&result->fl_lock);
	result->fl_size = range_size;
	FixedList_LockRead(self);
	Dee_XMovrefv(result->fl_elem,
	             self->fl_elem + start,
	             range_size);
	FixedList_LockEndRead(self);
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_delrange_index(FixedList *__restrict self, Dee_ssize_t i_begin, Dee_ssize_t i_end) {
	struct Dee_seq_range range;
	size_t range_size;
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->fl_size);
	range_size = range.sr_end - range.sr_start;
	if (range_size > 0) {
		size_t i;
		DREF DeeObject **values_buf;
		values_buf = (DREF DeeObject **)Dee_Mallocac(range_size, sizeof(DREF DeeObject *));
		if unlikely(!values_buf)
			goto err;
		FixedList_LockWrite(self);
		for (i = 0; i < range_size; ++i) {
			DREF DeeObject **p_slot;
			p_slot = &self->fl_elem[(size_t)i_begin + i];
			/* Exchange objects. */
			values_buf[i] = *p_slot;
			*p_slot       = NULL;
		}
		FixedList_LockEndWrite(self);
		Dee_XDecrefv(values_buf, range_size);
		Dee_Freea(values_buf);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_delrange_index_n(FixedList *__restrict self, Dee_ssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return fl_delrange_index(self, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t start, range_size;
	start = DeeSeqRange_Clamp_n(i_begin, self->fl_size);
	range_size = self->fl_size - start;
	if (range_size > 0) {
		size_t i;
		DREF DeeObject **values_buf;
		values_buf = (DREF DeeObject **)Dee_Mallocac(range_size, sizeof(DREF DeeObject *));
		if unlikely(!values_buf)
			goto err;
		FixedList_LockWrite(self);
		for (i = 0; i < range_size; ++i) {
			DREF DeeObject **p_slot;
			p_slot = &self->fl_elem[(size_t)i_begin + i];
			/* Exchange objects. */
			values_buf[i] = *p_slot;
			*p_slot       = NULL;
		}
		FixedList_LockEndWrite(self);
		Dee_XDecrefv(values_buf, range_size);
		Dee_Freea(values_buf);
	}
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
fl_setrange_index(FixedList *self, Dee_ssize_t i_begin,
                  Dee_ssize_t i_end, DeeObject *values) {
	struct Dee_seq_range range;
	size_t i, range_size;
	DREF DeeObject **values_buf;
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->fl_size);
	range_size = range.sr_end - range.sr_start;
	values_buf = (DREF DeeObject **)Dee_Mallocac(range_size, sizeof(DREF DeeObject *));
	if unlikely(!values_buf)
		goto err;
	if (DeeObject_InvokeMethodHint(seq_unpack_ub, values, range_size, range_size, values_buf) == (size_t)-1)
		goto err_values_buf;
	FixedList_LockWrite(self);
	for (i = 0; i < range_size; ++i) {
		DREF DeeObject **p_slot, *temp;
		p_slot = &self->fl_elem[(size_t)i_begin + i];
		/* Exchange objects. */
		temp          = *p_slot;
		*p_slot       = values_buf[i];
		values_buf[i] = temp;
	}
	FixedList_LockEndWrite(self);
	Dee_XDecrefv(values_buf, range_size);
	Dee_Freea(values_buf);
	return 0;
err_values_buf:
	Dee_Freea(values_buf);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
fl_setrange_index_n(FixedList *self, Dee_ssize_t i_begin, DeeObject *values) {
#ifdef __OPTIMIZE_SIZE__
	return fl_setrange_index(self, i_begin, SSIZE_MAX, values);
#else /* __OPTIMIZE_SIZE__ */
	size_t i, start, range_size;
	DREF DeeObject **values_buf;
	start = DeeSeqRange_Clamp_n(i_begin, self->fl_size);
	range_size = self->fl_size - start;
	values_buf = (DREF DeeObject **)Dee_Mallocac(range_size, sizeof(DREF DeeObject *));
	if unlikely(!values_buf)
		goto err;
	if (DeeObject_InvokeMethodHint(seq_unpack_ub, values, range_size, range_size, values_buf) == (size_t)-1)
		goto err_values_buf;
	FixedList_LockWrite(self);
	for (i = 0; i < range_size; ++i) {
		DREF DeeObject **p_slot, *temp;
		p_slot = &self->fl_elem[(size_t)i_begin + i];
		/* Exchange objects. */
		temp          = *p_slot;
		*p_slot       = values_buf[i];
		values_buf[i] = temp;
	}
	FixedList_LockEndWrite(self);
	Dee_XDecrefv(values_buf, range_size);
	Dee_Freea(values_buf);
	return 0;
err_values_buf:
	Dee_Freea(values_buf);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
fl_mh_seq_enumerate_index(FixedList *self, Dee_seq_enumerate_index_t proc,
                   void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->fl_size)
		end = self->fl_size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		FixedList_LockRead(self);
		elem = self->fl_elem[i];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		temp = (*proc)(arg, i, elem);
		Dee_XDecref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
fl_xchitem_index(FixedList *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	if unlikely(index >= self->fl_size) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	self->fl_elem[index] = value;
	FixedList_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_mh_pop(FixedList *__restrict self, Dee_ssize_t index) {
	DREF DeeObject *result;
	if (index < 0)
		index += self->fl_size;
	if unlikely((size_t)index >= self->fl_size)
		goto err_bounds;
	FixedList_LockRead(self);
	result = self->fl_elem[(size_t)index]; /* Steal reference */
	self->fl_elem[(size_t)index] = NULL;
	FixedList_LockEndRead(self);
	if unlikely(!result) {
		DeeRT_ErrUnboundIndex((DeeObject *)self,
		                      (size_t)index);
	}
	return result;
err_bounds:
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self,
	                          (size_t)index,
	                          self->fl_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
fl_mh_find(FixedList *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->fl_size)
		end = self->fl_size;
	for (; start < end; ++start) {
		int error;
		DREF DeeObject *elem;
		FixedList_LockRead(self);
		elem = self->fl_elem[start];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareEq(item, elem);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error == 0)
			return start;
	}
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
fl_mh_find_with_key(FixedList *self, DeeObject *item,
                      size_t start, size_t end, DeeObject *key) {
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->fl_size)
		end = self->fl_size;
	for (; start < end; ++start) {
		int error;
		DREF DeeObject *elem;
		FixedList_LockRead(self);
		elem = self->fl_elem[start];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareKeyEq(item, elem, key);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_item;
		if (error == 0) {
			Dee_Decref(item);
			return start;
		}
	}
	Dee_Decref(item);
	return (size_t)-1;
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
fl_mh_rfind(FixedList *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->fl_size)
		end = self->fl_size;
	while (end > start) {
		int error;
		DREF DeeObject *elem;
		--end;
		FixedList_LockRead(self);
		elem = self->fl_elem[end];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareEq(item, elem);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error == 0)
			return end;
	}
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
fl_mh_rfind_with_key(FixedList *self, DeeObject *item,
                     size_t start, size_t end, DeeObject *key) {
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->fl_size)
		end = self->fl_size;
	while (end > start) {
		int error;
		DREF DeeObject *elem;
		--end;
		FixedList_LockRead(self);
		elem = self->fl_elem[end];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareKeyEq(item, elem, key);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_item;
		if (error == 0) {
			/* Found it! */
			Dee_Decref(item);
			return end;
		}
	}
	Dee_Decref(item);
	return (size_t)-1;
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fl_mh_remove(FixedList *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->fl_size)
		end = self->fl_size;
	for (; start < end; ++start) {
		int error;
		DREF DeeObject *elem;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[start];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareEq(item, elem);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error == 0) {
			/* Found it! */
			FixedList_LockWrite(self);
			if unlikely(self->fl_elem[start] != elem) {
				FixedList_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again_elem;
			}
			self->fl_elem[start] = NULL;
			FixedList_LockEndWrite(self);
			Dee_Decref(elem);
			return 1;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
fl_mh_remove_with_key(FixedList *self, DeeObject *item,
                      size_t start, size_t end, DeeObject *key) {
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->fl_size)
		end = self->fl_size;
	for (; start < end; ++start) {
		int error;
		DREF DeeObject *elem;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[start];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareKeyEq(item, elem, key);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_item;
		if (error == 0) {
			/* Found it! */
			FixedList_LockWrite(self);
			if unlikely(self->fl_elem[start] != elem) {
				FixedList_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err_item;
				goto again_elem;
			}
			self->fl_elem[start] = NULL;
			FixedList_LockEndWrite(self);
			Dee_Decref(elem);
			Dee_Decref(item);
			return 1;
		}
	}
	Dee_Decref(item);
	return 0;
err_item:
	Dee_Decref(item);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fl_mh_rremove(FixedList *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->fl_size)
		end = self->fl_size;
	while (end > start) {
		int error;
		DREF DeeObject *elem;
		--end;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[end];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareEq(item, elem);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error == 0) {
			/* Found it! */
			FixedList_LockWrite(self);
			if unlikely(self->fl_elem[end] != elem) {
				FixedList_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again_elem;
			}
			self->fl_elem[end] = NULL;
			FixedList_LockEndWrite(self);
			Dee_Decref(elem);
			return 1;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
fl_mh_rremove_with_key(FixedList *self, DeeObject *item,
                       size_t start, size_t end, DeeObject *key) {
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->fl_size)
		end = self->fl_size;
	while (end > start) {
		int error;
		DREF DeeObject *elem;
		--end;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[end];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareKeyEq(item, elem, key);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_item;
		if (error == 0) {
			/* Found it! */
			FixedList_LockWrite(self);
			if unlikely(self->fl_elem[end] != elem) {
				FixedList_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err_item;
				goto again_elem;
			}
			self->fl_elem[end] = NULL;
			FixedList_LockEndWrite(self);
			Dee_Decref(elem);
			Dee_Decref(item);
			return 1;
		}
	}
	Dee_Decref(item);
	return 0;
err_item:
	Dee_Decref(item);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
fl_mh_removeall(FixedList *self, DeeObject *item,
                size_t start, size_t end, size_t max) {
	size_t result = 0;
	if unlikely(!max)
		return 0;
	if (end > self->fl_size)
		end = self->fl_size;
	for (; start < end; ++start) {
		int error;
		DREF DeeObject *elem;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[start];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareEq(item, elem);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error == 0) {
			/* Found it! */
			FixedList_LockWrite(self);
			if unlikely(self->fl_elem[start] != elem) {
				FixedList_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again_elem;
			}
			self->fl_elem[start] = NULL;
			FixedList_LockEndWrite(self);
			Dee_Decref(elem);
			++result;
			if (result >= max)
				break;
		}
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2, 6)) size_t DCALL
fl_mh_removeall_with_key(FixedList *self, DeeObject *item, size_t start,
                         size_t end, size_t max, DeeObject *key) {
	size_t result = 0;
	if unlikely(!max)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->fl_size)
		end = self->fl_size;
	for (; start < end; ++start) {
		int error;
		DREF DeeObject *elem;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[start];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (!elem)
			continue;
		error = DeeObject_TryCompareKeyEq(item, elem, key);
		Dee_Decref(elem);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_item;
		if (error == 0) {
			/* Found it! */
			FixedList_LockWrite(self);
			if unlikely(self->fl_elem[start] != elem) {
				FixedList_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err_item;
				goto again_elem;
			}
			self->fl_elem[start] = NULL;
			FixedList_LockEndWrite(self);
			Dee_Decref(elem);
			++result;
			if (result >= max)
				break;
		}
	}
	Dee_Decref(item);
	return result;
err_item:
	Dee_Decref(item);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
fl_mh_removeif(FixedList *self, DeeObject *should,
               size_t start, size_t end, size_t max) {
	size_t result = 0;
	size_t i;
	if unlikely(!max)
		return 0;
	if (end > self->fl_size)
		end = self->fl_size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[i];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (elem) {
			int error;
			DREF DeeObject *bshould;
			bshould = DeeObject_Call(should, 1, &elem);
			Dee_Decref(elem);
			if unlikely(!bshould)
				goto err;
			error = DeeObject_BoolInherited(bshould);
			if unlikely(error < 0)
				goto err;
			if (error) {
				/* Found one! */
				FixedList_LockWrite(self);
				if unlikely(self->fl_elem[i] != elem) {
					FixedList_LockEndWrite(self);
					if (DeeThread_CheckInterrupt())
						goto err;
					goto again_elem;
				}
				self->fl_elem[i] = NULL;
				FixedList_LockEndWrite(self);
				Dee_Decref(elem);
				++result;
				if (result >= max)
					break;
			}
		}
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE ATTR_NOINLINE NONNULL((1, 4)) void DCALL
fl_mh_fill_fallback(FixedList *self, size_t start, size_t count, DeeObject *filler) {
	DREF DeeObject *old_values[32];
	size_t old_values_count;
again:
	old_values_count = 0;
	FixedList_LockWrite(self);
	for (; count; --count, ++start) {
		if unlikely(old_values_count >= COMPILER_LENOF(old_values)) {
			FixedList_LockEndWrite(self);
			Dee_Decrefv(old_values, COMPILER_LENOF(old_values));
			goto again;
		}
		old_values[old_values_count++] = self->fl_elem[start]; /* Inherit reference */
		self->fl_elem[start] = filler;
	}
	FixedList_LockEndWrite(self);
	Dee_Decrefv(old_values, old_values_count);
}

PRIVATE NONNULL((1, 4)) int DCALL
fl_mh_fill(FixedList *self, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject **saved;
	if (end > self->fl_size)
		end = self->fl_size;
	if unlikely(start >= end)
		return 0;
	end -= start;
	saved = (DREF DeeObject **)Dee_TryMallocac(end, sizeof(DREF DeeObject *));
	Dee_Incref_n(filler, end);
	if likely(saved) {
		FixedList_LockWrite(self);
		memcpyc(saved, self->fl_elem, end, sizeof(DREF DeeObject *));
		memsetp(self->fl_elem, filler, end);
		FixedList_LockEndWrite(self);
		Dee_Decrefv(saved, end);
		Dee_Freea(saved);
	} else {
		fl_mh_fill_fallback(self, start, end, filler);
	}
	return 0;
}

PRIVATE NONNULL((1)) int DCALL
fl_mh_reverse(FixedList *self, size_t start, size_t end) {
	DeeObject **lo, **hi;
	if (end > self->fl_size)
		end = self->fl_size;
	if unlikely(start > end)
		start = end;
	FixedList_LockWrite(self);
	lo = self->fl_elem + start;
	hi = self->fl_elem + end;
	while (lo < hi) {
		DeeObject *temp;
		temp  = *lo;
		*lo++ = *--hi;
		*hi   = temp;
	}
	FixedList_LockEndWrite(self);
	return 0;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_mh_clear(FixedList *self) {
	fl_clear_impl(self);
	return 0;
}

PRIVATE struct type_method tpconst fl_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_find),
	TYPE_METHOD_HINTREF(Sequence_rfind),
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(Sequence_remove),
	TYPE_METHOD_HINTREF(Sequence_rremove),
	TYPE_METHOD_HINTREF(Sequence_removeall),
	TYPE_METHOD_HINTREF(Sequence_removeif),
	TYPE_METHOD_HINTREF(Sequence_fill),
	TYPE_METHOD_HINTREF(Sequence_reverse),
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst fl_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_find, &fl_mh_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &fl_mh_find_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &fl_mh_rfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &fl_mh_rfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &fl_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pop, &fl_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_remove, &fl_mh_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_remove_with_key, &fl_mh_remove_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rremove, &fl_mh_rremove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rremove_with_key, &fl_mh_rremove_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeall, &fl_mh_removeall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeall_with_key, &fl_mh_removeall_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeif, &fl_mh_removeif, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_fill, &fl_mh_fill, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reverse, &fl_mh_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_clear, &fl_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &fl_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq fl_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&fl_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fl_contains,
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
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&fl_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&fl_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&fl_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&fl_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&fl_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&fl_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&fl_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&fl_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&fl_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&fl_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&fl_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&fl_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&fl_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&fl_setrange_index_n,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&fl_trygetitem_index,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_sizeof(FixedList *self) {
	size_t result;
	result = _Dee_MallococBufsize(offsetof(FixedList, fl_elem),
	                              self->fl_size, sizeof(DREF DeeObject *));
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset tpconst fl_getsets[] = {
	TYPE_GETTER_AB_F("__sizeof__", &fl_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst fl_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &FixedListIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE struct type_operator const fl_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FNOREFESCAPE),
};

INTERN DeeTypeObject FixedList_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "FixedList",
	/* .tp_doc      = */ DOC("A mutable, but fixed-length sequence type, functioning "
	                         /**/ "as a sort-of hybrid between ?DList and ?DTuple\n"
	                         "\n"

	                         "()\n"
	                         "Construct a fixed list that is empty\n"
	                         "\n"

	                         "(size:?Dint,init?)\n"
	                         "Create an pre-sized list of @size elements, all initialized "
	                         /**/ "as @init, or as unbound when no @init is given\n"
	                         "\n"

	                         "(seq:?S?O)\n"
	                         "Construct a FixedList from items taken from the given @seq\n"
	                         "\n"

	                         "(seq:?M?Dint?O)\n"
	                         "Construct a FixedList by assigning values to indices\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a shallow copy of @this FixedList\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Returns a deep copy of @this FixedList\n"
	                         "\n"

	                         ":=(other:?S?O)->\n"
	                         "#tUnpackError{@other has a different length than @this}"
	                         "Assign all the elements from @other to @this FixedList\n"
	                         "\n"

	                         "move:=->\n"
	                         "#tUnpackError{@other has a different length than @this}"
	                         "Move all the elements from @other into @this FixedList, "
	                         /**/ "changing all of them to unbound in @other\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this FixedList is non-empty. ?f otherwise\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this FixedList in ascending order\n"
	                         "\n"

	                         "[]->\n"
	                         "#tIndexError{@index is greater that the length of @this FixedList}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Returns the @index'th item of @this FixedList\n"
	                         "\n"

	                         "[]=->\n"
	                         "#tIndexError{@index is greater that the length of @this FixedList}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Set the @index'th item of @this FixedList to @item\n"
	                         "\n"

	                         "del[]->\n"
	                         "#tIndexError{@index is greater that the length of @this FixedList}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Mark the @index'th element of @this FixedList as unbound\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @item is apart of @this FixedList, ?f otherwise"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(FixedList),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&fl_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&fl_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&fl_copy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&fl_init,
				/* .tp_free      = */ (Dee_funptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&fl_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&fl_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&fl_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&fl_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&fl_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&fl_visit,
	/* .tp_gc            = */ &fl_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ &fl_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ fl_methods,
	/* .tp_getsets       = */ fl_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ fl_class_members,
	/* .tp_method_hints  = */ fl_method_hints,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ fl_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(fl_operators)
};


#define FLI_GETITER(x)  atomic_read(&(x)->li_iter)

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_ctor(FixedListIterator *__restrict self) {
	self->li_iter = 0;
	self->li_list = fl_ctor();
	return likely(self->li_list) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fli_copy(FixedListIterator *__restrict self,
         FixedListIterator *__restrict other) {
	self->li_list = other->li_list;
	self->li_iter = FLI_GETITER(other);
	Dee_Incref(self->li_list);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fli_deep(FixedListIterator *__restrict self,
         FixedListIterator *__restrict other) {
	self->li_iter = FLI_GETITER(other);
	self->li_list = (DREF FixedList *)DeeObject_DeepCopy((DeeObject *)other->li_list);
	if unlikely(!self->li_list)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_init(FixedListIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->li_iter = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":FixedListIterator", &self->li_list, &self->li_iter))
		goto err;
	if (DeeObject_AssertTypeExact(self->li_list, &FixedList_Type))
		goto err;
	Dee_Incref(self->li_list);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
fli_fini(FixedListIterator *__restrict self) {
	Dee_Decref(self->li_list);
}

PRIVATE NONNULL((1, 2)) void DCALL
fli_visit(FixedListIterator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->li_list);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
fli_hash(FixedListIterator *self) {
	return FLI_GETITER(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fli_compare(FixedListIterator *self, FixedListIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FixedListIterator_Type))
		goto err;
	Dee_return_compareT(size_t, FLI_GETITER(self),
	                    /*   */ FLI_GETITER(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF FixedList *DCALL
fli_getseq(FixedListIterator *__restrict self) {
	return_reference_(self->li_list);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
fli_getindex(FixedListIterator *__restrict self) {
	return FLI_GETITER(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_setindex(FixedListIterator *__restrict self, size_t new_index) {
	atomic_write(&self->li_iter, new_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_rewind(FixedListIterator *__restrict self) {
	atomic_write(&self->li_iter, 0);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_revert(FixedListIterator *__restrict self, size_t step) {
	size_t oldpos, newpos;
	do {
		oldpos = atomic_read(&self->li_iter);
		if (OVERFLOW_USUB(oldpos, step, &newpos))
			newpos = 0;
	} while (!atomic_cmpxch_weak_or_write(&self->li_iter, oldpos, newpos));
	if (newpos == 0)
		return 1;
	return 2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_advance(FixedListIterator *__restrict self, size_t step) {
	size_t oldpos, newpos;
	do {
		oldpos = atomic_read(&self->li_iter);
		if (OVERFLOW_UADD(oldpos, step, &newpos))
			newpos = (size_t)-1;
	} while (!atomic_cmpxch_weak_or_write(&self->li_iter, oldpos, newpos));
	if (newpos >= self->li_list->fl_size)
		return 1;
	return 2;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fli_peek(FixedListIterator *__restrict self) {
	DREF DeeObject *result;
	size_t iter, newiter;
	FixedList *list = self->li_list;
	iter = FLI_GETITER(self);
	if (iter >= list->fl_size)
		return ITER_DONE;
	newiter = iter;
	FixedList_LockRead(list);
	for (;; ++newiter) {
		if (newiter >= list->fl_size) {
			FixedList_LockEndRead(list);
			return ITER_DONE;
		}
		result = list->fl_elem[newiter];
		if (result)
			break;
	}
	Dee_Incref(result);
	FixedList_LockEndRead(list);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fli_bool(FixedListIterator *__restrict self) {
	size_t pos = FLI_GETITER(self);
	return pos < self->li_list->fl_size;
}


PRIVATE struct type_nii tpconst fli_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&fli_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&fli_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&fli_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&fli_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)&fli_revert,
			/* .nii_advance  = */ (Dee_funptr_t)&fli_advance,
			/* .nii_prev     = */ (Dee_funptr_t)NULL,
			/* .nii_next     = */ (Dee_funptr_t)NULL,
			/* .nii_hasprev  = */ (Dee_funptr_t)NULL,
			/* .nii_peek     = */ (Dee_funptr_t)&fli_peek,
		}
	}
};

PRIVATE struct type_cmp fli_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&fli_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&fli_compare,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &fli_nii,
};


PRIVATE struct type_member tpconst fli_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(FixedListIterator, li_list), "->?GFixedList"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_OBJECT, offsetof(FixedListIterator, li_iter)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fli_next(FixedListIterator *__restrict self) {
	DREF DeeObject *result;
	size_t iter, newiter;
	FixedList *list = self->li_list;
again:
	iter = FLI_GETITER(self);
	if (iter >= list->fl_size)
		return ITER_DONE;
	newiter = iter;
	FixedList_LockRead(list);
	for (;; ++newiter) {
		if (newiter >= list->fl_size) {
			FixedList_LockEndRead(list);
			if (!atomic_cmpxch_weak_or_write(&self->li_iter, iter, newiter))
				goto again;
			return ITER_DONE;
		}
		result = list->fl_elem[newiter];
		if (result)
			break;
	}
	Dee_Incref(result);
	FixedList_LockEndRead(list);
	if (!atomic_cmpxch_weak_or_write(&self->li_iter, iter, newiter + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
}


INTERN DeeTypeObject FixedListIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "FixedListIterator",
	/* .tp_doc      = */ DOC("(seq?:?GFixedList)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&fli_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&fli_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&fli_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&fli_init,
				TYPE_FIXED_ALLOCATOR(FixedListIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&fli_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&fli_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&fli_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &fli_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&fli_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ fli_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_FIXEDLIST_C */
