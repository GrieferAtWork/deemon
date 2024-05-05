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
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>

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
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
	                                              (self->fl_size * sizeof(DREF DeeObject *)));
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

PRIVATE WUNUSED NONNULL((1)) DREF FixedList *DCALL
fl_init_iterator(DeeObject *__restrict iterator) {
	DREF FixedList *result, *new_result;
	DREF DeeObject *elem;
	DREF DeeObject *next;
	size_t itema, itemc;
	elem = DeeObject_IterNext(iterator);
	if (!ITER_ISOK(elem)) {
		if (elem == ITER_DONE)
			return fl_ctor();
		goto err;
	}
	next = DeeObject_IterNext(iterator);
	if (!ITER_ISOK(next)) {
		if (next == ITER_DONE) {
			result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
			                                              (1 * sizeof(DREF DeeObject *)));
			if unlikely(!result)
				goto err;
			itemc              = 1;
			result->fl_elem[0] = elem;
			goto done;
		}
		goto err;
	}
	itemc = 2, itema = 4;
	result = (DREF FixedList *)DeeGCObject_TryMalloc(offsetof(FixedList, fl_elem) +
	                                                 (4 * sizeof(DREF DeeObject *)));
	if unlikely(!result) {
		itema  = 2;
		result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
		                                              (2 * sizeof(DREF DeeObject *)));
		if unlikely(!result)
			goto err;
	}
	result->fl_elem[0] = elem;
	result->fl_elem[1] = next;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		ASSERT(itemc <= itema);
		if (itemc >= itema) {
			itema *= 2;
			new_result = (DREF FixedList *)DeeGCObject_TryRealloc(result,
			                                                      offsetof(FixedList, fl_elem) +
			                                                      (itema * sizeof(DREF DeeObject *)));
			if unlikely(!new_result) {
				itema      = itemc + 1;
				new_result = (DREF FixedList *)DeeGCObject_Realloc(result,
				                                                   offsetof(FixedList, fl_elem) +
				                                                   (itema * sizeof(DREF DeeObject *)));
				if unlikely(!new_result)
					goto err_r;
			}
			result = new_result;
		}
		result->fl_elem[itemc] = elem; /* Inherit reference. */
		++itemc;
		if (DeeThread_CheckInterrupt())
			goto err_r;
	}
	if unlikely(!elem)
		goto err_r;
	if (itema > itemc) {
		new_result = (DREF FixedList *)DeeGCObject_TryRealloc(result,
		                                                      offsetof(FixedList, fl_elem) +
		                                                      (itemc * sizeof(DREF DeeObject *)));
		if likely(new_result)
			result = new_result;
	}
done:
	Dee_atomic_rwlock_init(&result->fl_lock);
	result->fl_size = itemc;
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return (DREF FixedList *)DeeGC_Track((DeeObject *)result);
err_r:
	Dee_Decrefv(result->fl_elem, itemc);
	DeeGCObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF FixedList *DCALL
fl_init_getitem(DREF DeeObject *(DCALL *getitem)(DeeObject *__restrict self,
                                                 DeeObject *__restrict index),
                DeeObject *__restrict sequence, size_t length) {
	DREF FixedList *result;
	size_t i;
	DREF DeeObject *index_ob, *elem;
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
	                                              (length * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto err;
	for (i = 0; i < length; ++i) {
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err_r;
		elem = (*getitem)(sequence, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err_r;
		}
		result->fl_elem[i] = elem;
	}
	Dee_atomic_rwlock_init(&result->fl_lock);
	result->fl_size = length;
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return (DREF FixedList *)DeeGC_Track((DeeObject *)result);
err_r:
	Dee_Decrefv(result->fl_elem, i);
	DeeGCObject_Free(result);
err:
	return NULL;
}


PRIVATE WUNUSED DREF FixedList *DCALL
fl_init(size_t argc, DeeObject *const *argv) {
	DREF FixedList *result;
	DeeObject *size_ob, *init = NULL;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o|o:FixedList", &size_ob, &init))
		goto err;
	if (init) {
		if (DeeObject_AsSize(size_ob, &size))
			goto err;
		result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
		                                              (size * sizeof(DREF DeeObject *)));
		if unlikely(!result)
			goto err;
		Dee_atomic_rwlock_init(&result->fl_lock);
		result->fl_size = size;
		Dee_Incref_n(init, size);
		memsetp(result->fl_elem, init, size);
	} else if (DeeInt_Check(size_ob)) {
		if (DeeObject_AsSize(size_ob, &size))
			goto err;
		result = (DREF FixedList *)DeeGCObject_Calloc(offsetof(FixedList, fl_elem) +
		                                              (size * sizeof(DREF DeeObject *)));
		if unlikely(!result)
			goto err;
		Dee_atomic_rwlock_cinit(&result->fl_lock);
		result->fl_size = size;
	} else {
		size_t i;
		if (DeeMapping_Check(size_ob))
			goto init_from_iterator;

		/* Initialize from sequence. */
		size = DeeFastSeq_GetSize_deprecated(size_ob);
		if (size == (size_t)-1) {
			DeeTypeObject *iter;
			DREF DeeObject *iterator;
			DeeTypeMRO mro;
			iter = DeeTypeMRO_Init(&mro, Dee_TYPE(size_ob));
			for (;;) {
				DeeTypeObject *base;
				base = DeeTypeMRO_Next(&mro, iter);
				if (iter->tp_seq &&
				    (!base || iter->tp_seq != base->tp_seq)) {
					if (iter->tp_seq->tp_getitem &&
					    (!base || !base->tp_seq || base->tp_seq->tp_getitem != iter->tp_seq->tp_getitem)) {
						size = DeeObject_Size(size_ob);
						if unlikely(size == (size_t)-1)
							goto err;
						return fl_init_getitem(iter->tp_seq->tp_getitem, size_ob, size);
					}
					if (iter->tp_seq->tp_iter &&
					    (!base || !base->tp_seq || base->tp_seq->tp_iter != iter->tp_seq->tp_iter)) {
						iterator = (*iter->tp_seq->tp_iter)(size_ob);
						if unlikely(!iterator)
							goto err;
						result = fl_init_iterator(iterator);
						Dee_Decref(iterator);
						return result;
					}
				}
				if (!base)
					break;
				iter = base;
			}
init_from_iterator:
			/* Initialize from iterators. */
			iterator = DeeObject_Iter(size_ob);
			if unlikely(!iterator)
				goto err;
			result = fl_init_iterator(iterator);
			Dee_Decref(iterator);
			return result;
		}
		/* Initialize from a fast sequence */
		result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
		                                              (size * sizeof(DREF DeeObject *)));
		if unlikely(!result)
			goto err;
		Dee_atomic_rwlock_init(&result->fl_lock);
		result->fl_size = size;
		for (i = 0; i < size; ++i) {
			DREF DeeObject *elem;
			elem = DeeFastSeq_GetItemUnbound_deprecated(size_ob, i);
			if (elem == ITER_DONE)
				goto err_r;
			result->fl_elem[i] = elem; /* Inherit reference. */
		}
	}
/*done:*/
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	return (DREF FixedList *)DeeGC_Track((DeeObject *)result);
err_r:
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
	if (DeeObject_Unpack(other, self->fl_size, items))
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
		if (DeeObject_XInplaceDeepCopyWithLock(&self->fl_elem[i],
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
fl_visit(FixedList *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	FixedList_LockRead(self);
	for (i = 0; i < self->fl_size; ++i)
		Dee_XVisit(self->fl_elem[i]);
	FixedList_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
fl_clear(FixedList *__restrict self) {
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
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&fl_clear,
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
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		err_unbound_index((DeeObject *)self, index);
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
		return -2;
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		return 0;
	}
	Dee_Incref(result);
	FixedList_LockEndRead(self);
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_hasitem_index(FixedList *__restrict self, size_t index) {
	return index < self->fl_size ? 1 : 0;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
fl_delitem_index(FixedList *__restrict self, size_t index) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->fl_size) {
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
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
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
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

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
fl_nsi_xchitem(FixedList *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	if unlikely(index >= self->fl_size) {
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	FixedList_LockRead(self);
	result = self->fl_elem[index];
	if unlikely(!result) {
		FixedList_LockEndRead(self);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	self->fl_elem[index] = value;
	FixedList_LockEndRead(self);
	return result;
err:
	return NULL;
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
		temp = DeeObject_CompareEq(value, elem);
		Dee_Decref(elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return_true;
		}
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
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
	                                              range_size * sizeof(DREF DeeObject *));
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
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
	                                              range_size * sizeof(DREF DeeObject *));
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
	if (DeeObject_UnpackWithUnbound(values, range_size, values_buf))
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
	if (DeeObject_UnpackWithUnbound(values, range_size, values_buf))
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

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
fl_nsi_find(FixedList *self, size_t start, size_t end,
            DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	if (end > self->fl_size)
		end = self->fl_size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		FixedList_LockRead(self);
		elem = self->fl_elem[i];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (elem) {
			int error;
			error = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err;
			if (error)
				return i; /* Found it! */
		}
	}
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
fl_nsi_rfind(FixedList *self, size_t start, size_t end,
             DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	if (end > self->fl_size)
		end = self->fl_size;
	for (i = end; i > start;) {
		DREF DeeObject *elem;
		--i;
		FixedList_LockRead(self);
		elem = self->fl_elem[i];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (elem) {
			int error;
			error = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err;
			if (error)
				return i; /* Found it! */
		}
	}
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_nsi_pop(FixedList *__restrict self, Dee_ssize_t index) {
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
		err_unbound_index((DeeObject *)self,
		                  (size_t)index);
	}
	return result;
err_bounds:
	err_index_out_of_bounds((DeeObject *)self,
	                        (size_t)index,
	                        self->fl_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
fl_nsi_remove(FixedList *self, size_t start, size_t end,
              DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
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
			error = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err;
			if (error) {
				/* Found it! */
				FixedList_LockWrite(self);
				if unlikely(self->fl_elem[i] != elem) {
					FixedList_LockEndWrite(self);
					goto again_elem;
				}
				self->fl_elem[i] = NULL;
				FixedList_LockEndWrite(self);
				Dee_Decref(elem);
				return 1;
			}
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
fl_nsi_rremove(FixedList *self, size_t start, size_t end,
               DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	if (end > self->fl_size)
		end = self->fl_size;
	for (i = end; i > start;) {
		DREF DeeObject *elem;
		--i;
again_elem:
		FixedList_LockRead(self);
		elem = self->fl_elem[i];
		Dee_XIncref(elem);
		FixedList_LockEndRead(self);
		if (elem) {
			int error;
			error = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err;
			if (error) {
				/* Found it! */
				FixedList_LockWrite(self);
				if unlikely(self->fl_elem[i] != elem) {
					FixedList_LockEndWrite(self);
					goto again_elem;
				}
				self->fl_elem[i] = NULL;
				FixedList_LockEndWrite(self);
				Dee_Decref(elem);
				return 1;
			}
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
fl_nsi_removeall(FixedList *self, size_t start, size_t end,
                 DeeObject *keyed_search_item, DeeObject *key) {
	size_t result = 0;
	size_t i;
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
			error = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err;
			if (error) {
				/* Found one! */
				FixedList_LockWrite(self);
				if unlikely(self->fl_elem[i] != elem) {
					FixedList_LockEndWrite(self);
					goto again_elem;
				}
				self->fl_elem[i] = NULL;
				FixedList_LockEndWrite(self);
				Dee_Decref(elem);
				++result;
			}
		}
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
fl_nsi_removeif(FixedList *self, size_t start,
                size_t end, DeeObject *should) {
	size_t result = 0;
	size_t i;
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
			error = DeeObject_Bool(bshould);
			Dee_Decref(bshould);
			if unlikely(error < 0)
				goto err;
			if (error) {
				/* Found one! */
				FixedList_LockWrite(self);
				if unlikely(self->fl_elem[i] != elem) {
					FixedList_LockEndWrite(self);
					goto again_elem;
				}
				self->fl_elem[i] = NULL;
				FixedList_LockEndWrite(self);
				Dee_Decref(elem);
				++result;
			}
		}
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
fl_foreach(FixedList *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *elem;
		FixedList_LockRead(self);
		while ((elem = self->fl_elem[i]) == NULL) {
			++i;
			if (i >= self->fl_size) {
				FixedList_LockEndRead(self);
				goto done;
			}
		}
		Dee_Incref(elem);
		FixedList_LockEndRead(self);
		temp = (*proc)(arg, elem);
		Dee_Decref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
done:
	return result;
err_temp:
	return temp;
}


PRIVATE struct type_nsi tpconst fl_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&fl_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&fl_size,
			/* .nsi_getitem      = */ (dfunptr_t)&fl_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)&fl_delitem_index,
			/* .nsi_setitem      = */ (dfunptr_t)&fl_setitem_index,
			/* .nsi_getitem_fast = */ (dfunptr_t)&fl_getitem_index_fast,
			/* .nsi_getrange     = */ (dfunptr_t)&fl_getrange_index,
			/* .nsi_getrange_n   = */ (dfunptr_t)&fl_getrange_index_n,
			/* .nsi_delrange     = */ (dfunptr_t)&fl_delrange_index,
			/* .nsi_delrange_n   = */ (dfunptr_t)&fl_delrange_index_n,
			/* .nsi_setrange     = */ (dfunptr_t)&fl_setrange_index,
			/* .nsi_setrange_n   = */ (dfunptr_t)&fl_setrange_index_n,
			/* .nsi_find         = */ (dfunptr_t)&fl_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&fl_nsi_rfind,
			/* .nsi_xch          = */ (dfunptr_t)&fl_nsi_xchitem,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)&fl_nsi_pop,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)&fl_nsi_remove,
			/* .nsi_rremove      = */ (dfunptr_t)&fl_nsi_rremove,
			/* .nsi_removeall    = */ (dfunptr_t)&fl_nsi_removeall,
			/* .nsi_removeif     = */ (dfunptr_t)&fl_nsi_removeif
		}
	}
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
	/* .tp_nsi                        = */ &fl_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&fl_foreach,
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
fl_clear_meth(FixedList *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	fl_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fl_sizeof(FixedList *self) {
	return DeeInt_NewSize(offsetof(FixedList, fl_elem) +
	                      (self->fl_size * sizeof(DREF DeeObject *)));
}

PRIVATE struct type_method tpconst fl_methods[] = {
	TYPE_METHOD_F("clear", &fl_clear_meth, METHOD_FNOREFESCAPE, "()"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst fl_getsets[] = {
	TYPE_GETTER_F("__sizeof__", &fl_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
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
				/* .tp_ctor      = */ (dfunptr_t)&fl_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&fl_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&fl_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&fl_init,
				/* .tp_free      = */ (dfunptr_t)NULL
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&fl_visit,
	/* .tp_gc            = */ &fl_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ &fl_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ fl_methods,
	/* .tp_getsets       = */ fl_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ fl_class_members,
	/* .tp_call_kw       = */ NULL,
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
fli_visit(FixedListIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->li_list);
}

#define DEFINE_FIXEDLISTITERATOR_COMPARE(name, op)                     \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL              \
	name(FixedListIterator *self, FixedListIterator *other) {          \
		if (DeeObject_AssertTypeExact(other, &FixedListIterator_Type)) \
			goto err;                                                  \
		return_bool(FLI_GETITER(self) op FLI_GETITER(other));          \
	err:                                                               \
		return NULL;                                                   \
	}
DEFINE_FIXEDLISTITERATOR_COMPARE(fli_eq, ==)
DEFINE_FIXEDLISTITERATOR_COMPARE(fli_ne, !=)
DEFINE_FIXEDLISTITERATOR_COMPARE(fli_lo, <)
DEFINE_FIXEDLISTITERATOR_COMPARE(fli_le, <=)
DEFINE_FIXEDLISTITERATOR_COMPARE(fli_gr, >)
DEFINE_FIXEDLISTITERATOR_COMPARE(fli_ge, >=)
#undef DEFINE_FIXEDLISTITERATOR_COMPARE

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
			/* .nii_getseq   = */ (dfunptr_t)&fli_getseq,
			/* .nii_getindex = */ (dfunptr_t)&fli_getindex,
			/* .nii_setindex = */ (dfunptr_t)&fli_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&fli_rewind,
			/* .nii_revert   = */ (dfunptr_t)&fli_revert,
			/* .nii_advance  = */ (dfunptr_t)&fli_advance,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)&fli_peek,
		}
	}
};

PRIVATE struct type_cmp fli_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fli_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fli_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fli_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fli_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fli_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&fli_ge,
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
				/* .tp_ctor      = */ (dfunptr_t)&fli_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&fli_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&fli_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&fli_init,
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&fli_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &fli_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&fli_next,
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
