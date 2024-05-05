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
#ifndef GUARD_DEEMON_OBJECTS_LIST_C
#define GUARD_DEEMON_OBJECTS_LIST_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/limitcore.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

typedef DeeListObject List;

typedef struct {
	OBJECT_HEAD
	DREF List   *li_list;  /* [1..1][const] The list being iterated. */
	DWEAK size_t li_index; /* The current iteration index. */
} ListIterator;

INTDEF DeeTypeObject DeeListIterator_Type;


PRIVATE NONNULL((1)) void DCALL
list_fini(List *__restrict me) {
	weakref_support_fini(me);
	Dee_objectlist_fini(&me->l_list);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_assign(List *me, DeeObject *other) {
	struct Dee_objectlist newlist;
	DREF DeeObject **old_elemv;
	size_t old_elemc;
	if ((List *)other == me)
		return 0;

	/* Steal the current list state to update it! */
	DeeList_LockWrite(me);
	newlist = me->l_list;
	Dee_objectlist_init(&me->l_list);
	DeeList_LockEndWrite(me);

	/* Delete all of the old list elements. */
	Dee_Decrefv(newlist.ol_elemv, newlist.ol_elemc);
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
	newlist.ol_elemc = DeeSeq_AsHeapVectorWithAllocReuse(other, &newlist.ol_elemv, &newlist.ol_elema);
#else /* DEE_OBJECTLIST_HAVE_ELEMA */
	newlist.ol_elemc = DeeSeq_AsHeapVectorWithAllocReuse2(other, &newlist.ol_elemv);
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */

	/* Save the new list buffer. */
	DeeList_LockWrite(me);
	old_elemv  = me->l_list.ol_elemv;
	old_elemc  = me->l_list.ol_elemc;
	me->l_list = newlist;
	if (me->l_list.ol_elemc == (size_t)-1)
		me->l_list.ol_elemc = 0;
	DeeList_LockEndWrite(me);

	/* Free the list state that got created while we loaded `other' */
#ifndef __OPTIMIZE_SIZE__
	if unlikely(old_elemv)
#endif /* !__OPTIMIZE_SIZE__ */
	{
		Dee_Decrefv(old_elemv, old_elemc);
		Dee_Free(old_elemv);
	}

	/* With the updated list state now saved, check for errors. */
	if unlikely(newlist.ol_elemc == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_moveassign(List *me, List *other) {
	DREF DeeObject **old_elemv;
	size_t old_elemc;
	if (me == other)
		goto done;
	DeeList_LockWrite2(me, other);
	old_elemv  = me->l_list.ol_elemv;
	old_elemc  = me->l_list.ol_elemc;
	me->l_list = other->l_list;
	Dee_objectlist_init(&other->l_list);
	DeeList_LockEndWrite(other);
	DeeList_LockEndWrite(me);
	if (old_elemv) {
		Dee_Decrefv(old_elemv, old_elemc);
		Dee_Free(old_elemv);
	}
done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_ctor(List *__restrict self) {
	weakref_support_init(self);
	Dee_objectlist_init(&self->l_list);
	Dee_atomic_rwlock_init(&self->l_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_copy(List *__restrict me,
          List *__restrict other) {
	DREF DeeObject **new_elemv;
	size_t count;
	ASSERT(me != other);
	weakref_support_init(me);
	Dee_atomic_rwlock_init(&me->l_lock);
again:
	DeeList_LockRead(other);
	count  = other->l_list.ol_elemc;
	new_elemv = (DREF DeeObject **)Dee_TryMallocc(count, sizeof(DREF DeeObject *));
	if unlikely(!new_elemv) {
		DeeList_LockEndRead(other);
		if (Dee_CollectMemoryc(count, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}

	/* Copy references */
	me->l_list.ol_elemv = Dee_Movrefv(new_elemv, DeeList_ELEM(other), count);
	me->l_list.ol_elemc = count;
	_DeeList_SetAlloc(me, count);
	DeeList_LockEndRead(other);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_deepload(List *__restrict me) {
	DREF DeeObject *temp, *item;
	size_t i = 0;
	weakref_support_init(me);
	DeeList_LockRead(me);
	for (; i < me->l_list.ol_elemc; ++i) {
		item = me->l_list.ol_elemv[i];
		Dee_Incref(item);
		DeeList_LockEndRead(me);

		/* Create the deep copy. */
		temp = DeeObject_DeepCopy(item);
		Dee_Decref(item);
		if unlikely(!temp)
			goto err;
		DeeList_LockWrite(me);

		/* Must re-check that the list hasn't been shrunk in the mean time. */
		if unlikely(i >= me->l_list.ol_elemc)
			goto stop_on_end;

		/* Write the duplicated object back into the list's vector. */
		item = me->l_list.ol_elemv[i]; /* Inherit */
		me->l_list.ol_elemv[i] = temp; /* Inherit */
		DeeList_LockEndWrite(me);

		/* Drop the old object. */
		Dee_Decref(item);
		DeeList_LockRead(me);
	}
	DeeList_LockEndRead(me);
done:
	return 0;
err:
	return -1;
stop_on_end:
	DeeList_LockEndWrite(me);
	Dee_Decref(temp);
	goto done;
}


PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewWithHint(size_t n_prealloc) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeList_Type);
	_DeeList_SetAlloc(result, n_prealloc);
	result->l_list.ol_elemc  = 0;
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	if likely(n_prealloc) {
		result->l_list.ol_elemv = (DREF DeeObject **)Dee_TryMallocc(n_prealloc, sizeof(DREF DeeObject *));
		if unlikely(!result->l_list.ol_elemv)
			_DeeList_SetAlloc(result, 0);
	} else {
		result->l_list.ol_elemv = NULL;
	}
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PUBLIC WUNUSED DREF List *DCALL
DeeList_NewUninitialized(size_t n_elem) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	result->l_list.ol_elemv = (DREF DeeObject **)Dee_Mallocc(n_elem, sizeof(DREF DeeObject *));
	if unlikely(!result->l_list.ol_elemv)
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	_DeeList_SetAlloc(result, n_elem);
	result->l_list.ol_elemc  = n_elem;
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	/*DeeGC_Track((DeeObject *)result);*/ /* The caller must do this */
done:
	return result;
err_r:
	DeeGCObject_FREE(result);
	return NULL;
}

PUBLIC NONNULL((1)) void DCALL
DeeList_FreeUninitialized(DREF List *__restrict self) {
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeList_Type);
	ASSERT(!DeeObject_IsShared(self));
	Dee_DecrefNokill(&DeeList_Type);
	Dee_Free(DeeList_ELEM(self));
	DeeObject_FreeTracker((DeeObject *)self);
	DeeGCObject_FREE(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_FromSequence(DeeObject *__restrict self) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	if (Dee_objectlist_init_fromseq(&result->l_list, self) != 0)
		goto err_r;
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	DeeObject_Init(result, &DeeList_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_FromSequenceInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	if (DeeList_CheckExact(self)) {
		if (!DeeObject_IsShared(self))
			return self; /* Can re-use existing List object. */
	}
	result = DeeList_FromSequence(self);
	if likely(result)
		Dee_Decref(self);
	return result;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_FromTuple(DeeObject *__restrict self) {
	DeeObject **elemv;
	DREF List *result;
	size_t elemc;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	elemc = DeeTuple_SIZE(self);
	elemv = (DREF DeeObject **)Dee_Mallocc(elemc, sizeof(DREF DeeObject *));
	if unlikely(!elemv)
		goto err_r;
	elemv = Dee_Movrefv(elemv, DeeTuple_ELEM(self), elemc);
	result->l_list.ol_elemv = elemv;
	result->l_list.ol_elemc = elemc;
	_DeeList_SetAlloc(result, elemc);
	DeeObject_Init(result, &DeeList_Type);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInherited(size_t objc, /*inherit(on_success)*/ DREF DeeObject *const *objv) {
	DREF List *result;
	ASSERT(objv || !objc);
	result = DeeList_NewUninitialized(objc);
	if unlikely(!result)
		goto done;
	memcpyc(DeeList_ELEM(result), objv, objc,
	        sizeof(DREF DeeObject *));

	/* Now that the list's been filled with data,
	 * we can start tracking it as a GC object. */
	result = (DREF List *)DeeList_FinalizeUninitialized(result);
done:
	return (DREF DeeObject *)result;
}

/* Inherit the entire vector, which must have been allocated using `Dee_Malloc()' and friends. */
#ifndef DEE_OBJECTLIST_HAVE_ELEMA
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(/*inherit(on_success)*/ DREF DeeObject **objv,
                               size_t objc, size_t obja) {
	ASSERT(obja <= Dee_MallocUsableSize(objv) / sizeof(DREF DeeObject *));
	(void)obja;
	return DeeList_NewVectorInheritedHeap2(objv, objc);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap2(/*inherit(on_success)*/ DREF DeeObject **objv, size_t objc)
#else /* !DEE_OBJECTLIST_HAVE_ELEMA */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap2(/*inherit(on_success)*/ DREF DeeObject **objv, size_t objc) {
	/* For binary compatibility... */
	return DeeList_NewVectorInheritedHeap(objv, objc, objc);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(/*inherit(on_success)*/ DREF DeeObject **objv,
                               size_t objc, size_t obja)
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */
{
	DREF List *result;
#ifndef DEE_OBJECTLIST_HAVE_ELEMA
#ifdef Dee_MallocUsableSize
	ASSERT(objc <= Dee_MallocUsableSize(objv) / sizeof(DREF DeeObject *));
#endif /* Dee_MallocUsableSize */
	ASSERT(objv || !objc);
#else /* !DEE_OBJECTLIST_HAVE_ELEMA */
	ASSERT(objc <= obja);
	ASSERT(objv || (!obja && !objc));
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	result->l_list.ol_elemv = objv; /* Inherit */
	result->l_list.ol_elemc = objc;
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
	_DeeList_SetAlloc(result, obja);
#elif defined(Dee_MallocUsableSize)
	_DeeList_SetAlloc(result, Dee_MallocUsableSize(objv) / sizeof(DREF DeeObject *));
#else /* ... */
	_DeeList_SetAlloc(result, objc);
#endif /* !... */
	DeeObject_Init(result, &DeeList_Type);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

/* Create a new list object from a vector. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVector(size_t objc, DeeObject *const *objv) {
	DREF DeeObject *result;
	ASSERT(objv || !objc);
	Dee_Increfv(objv, objc);
	result = DeeList_NewVectorInherited(objc, objv);
	if unlikely(!result)
		goto err;
	return result;
err:
	Dee_Decrefv(objv, objc);
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
DeeList_Copy(DeeObject *__restrict self) {
	DREF List *result;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	if unlikely(list_copy(result, (List *)self))
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


/* Concat a list and some generic sequence,
 * inheriting a reference from `self' in the process. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeList_ConcatInherited(/*inherit(on_success)*/ DREF DeeObject *self,
                        DeeObject *sequence) {
	DREF DeeObject *result;
	size_t fast_seqlen;
	if (!DeeObject_IsShared(self)) {
		/* Simple case: can append onto the original list. */
		if unlikely(DeeList_AppendSequence(self, sequence))
			goto err;
		return self;
	}
	fast_seqlen = DeeFastSeq_GetSize_deprecated(sequence);
	if (fast_seqlen != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		/* TODO: Special optimization. */
	}

	/* Fallback: Copy the list and append an iterator. */
	result = DeeList_Copy(self);
	Dee_Decref(self);
	if unlikely(!result)
		goto err;
	if unlikely(DeeList_AppendSequence(result, sequence))
		Dee_Clear(result);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_ExtendInherited(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc,
                        /*inherit(on_success)*/ DREF DeeObject *const *argv) {
	List *me = (List *)self;
	DREF List *result;
	if (!DeeObject_IsShared(me)) {
		size_t req_alloc, old_elema;
		result    = me;
		req_alloc = result->l_list.ol_elemc + argc;
		old_elema = DeeList_GetAlloc(result);
		/* Make sure there are sufficient buffers. */
		if (req_alloc > old_elema) {
			DREF DeeObject **new_elemv;
			size_t new_elema = DEE_OBJECTLIST_MOREALLOC(old_elema);
			if (new_elema < req_alloc)
				new_elema = req_alloc;
			if (new_elema < DEE_OBJECTLIST_MINALLOC)
				new_elema = DEE_OBJECTLIST_MINALLOC;
do_realloc_vector:
			new_elemv = (DREF DeeObject **)Dee_TryReallocc(result->l_list.ol_elemv, new_elema,
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_elemv) {
				if (new_elema > req_alloc) {
					new_elema = req_alloc;
					goto do_realloc_vector;
				}
				if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
					goto do_realloc_vector;
				goto err;
			}
			result->l_list.ol_elemv  = new_elemv;
			_DeeList_SetAlloc(result, new_elema);
		}
		memcpyc(result->l_list.ol_elemv + result->l_list.ol_elemc, argv,
		        argc, sizeof(DREF DeeObject *));
		result->l_list.ol_elemc += argc;
	} else {
		DREF DeeObject **new_elemv;
		size_t list_size;
		list_size = DeeList_SIZE_ATOMIC(me);
allocate_new_vector:
		new_elemv = (DREF DeeObject **)Dee_Mallocc(list_size + argc,
		                                            sizeof(DREF DeeObject *));
		if unlikely(!new_elemv)
			goto err;
		DeeList_LockRead(me);
		if unlikely(DeeList_SIZE(me) != list_size) {
			list_size = DeeList_SIZE(me);
			DeeList_LockEndRead(me);
			Dee_Free(new_elemv);
			goto allocate_new_vector;
		}
		Dee_Movrefv(new_elemv, DeeList_ELEM(me), list_size);
		DeeList_LockEndRead(me);

		/* Create the new list descriptor. */
		result = DeeGCObject_MALLOC(List);
		if unlikely(!result) {
			Dee_Decrefv(new_elemv, list_size);
			Dee_Free(new_elemv);
			goto err;
		}
		memcpyc(new_elemv + list_size, argv, argc,
		        sizeof(DREF DeeObject *));
		DeeObject_Init(result, &DeeList_Type);
		result->l_list.ol_elemv = new_elemv;
		result->l_list.ol_elemc = list_size + argc;
		_DeeList_SetAlloc(result, list_size + argc);
		weakref_support_init(result);
		Dee_atomic_rwlock_init(&result->l_lock);
		result = (DREF List *)DeeGC_Track((DeeObject *)result);
	}
	return (DREF DeeObject *)result;
err:
	return NULL;
}


/* @return: * :   The popped element.
 * @return: NULL: The given index was out-of-bounds and an IndexError was thrown. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_Pop(DeeObject *__restrict self, dssize_t index) {
	List *me = (List *)self;
	DREF DeeObject *delob;
	size_t length;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	DeeList_LockWrite(me);
	length = me->l_list.ol_elemc;
	if unlikely(index < 0)
		index += length;
	if unlikely((size_t)index >= length) {
		DeeList_LockEndWrite(me);
		err_index_out_of_bounds((DeeObject *)me, (size_t)index, length);
		return NULL;
	}
	delob = DeeList_GET(me, (size_t)index);
	--length;
	me->l_list.ol_elemc = length;

	/* Adjust to shift following elements downwards. */
	memmovedownc(me->l_list.ol_elemv + (size_t)index,
	             me->l_list.ol_elemv + (size_t)index + 1,
	             length - (size_t)index,
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(me);
	return delob;
}

/* @return: * : The actual number of deleted items.
 * @return: (size_t)-1: Error. */
PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeList_Erase(DeeObject *__restrict self,
              size_t index, size_t count) {
	size_t max_count;
	List *me = (List *)self;
	DREF DeeObject **delobv;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
again:
	DeeList_LockWrite(me);
	if unlikely(index >= me->l_list.ol_elemc) {
		DeeList_LockEndWrite(me);
		return 0;
	}
	max_count = me->l_list.ol_elemc - index;
	if (count > max_count)
		count = max_count;
	delobv = (DREF DeeObject **)Dee_TryMallocac(count, sizeof(DREF DeeObject *));
	if unlikely(!delobv) {
		DeeList_LockEndWrite(me);
		if (!Dee_CollectMemoryc(count, sizeof(DREF DeeObject *)))
			return (size_t)-1;
		goto again;
	}

	/* Adjust to shift following elements downwards. */
	memcpyc(delobv, me->l_list.ol_elemv + index,
	        count, sizeof(DREF DeeObject *));
	me->l_list.ol_elemc -= count;
	memmovedownc(me->l_list.ol_elemv + index,
	             me->l_list.ol_elemv + index + count,
	             me->l_list.ol_elemc - index,
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(me);
	Dee_Decrefv(delobv, count);
	Dee_Freea(delobv);
	ASSERT(count != (size_t)-1);
	return count;
}

/* @return: 0 : The given `keyed_search_item' count not found found.
 * @return: 1 : The given `keyed_search_item' was deleted once.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
DeeList_Remove(DeeObject *self, size_t start, size_t end,
               DeeObject *keyed_search_item, DeeObject *key) {
	List *me = (List *)self;
	DeeObject **vector;
	size_t i, length;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	ASSERT_OBJECT_OPT(key);
	ASSERT_OBJECT(keyed_search_item);
	DeeList_LockRead(me);
again:
	vector = me->l_list.ol_elemv;
	length = me->l_list.ol_elemc;
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(me, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(me);
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, this_elem, key);
		Dee_Decref(this_elem);
		if unlikely(temp < 0)
			return temp;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(me);

			/* Check if the list was changed. */
			if (me->l_list.ol_elemv != vector ||
			    me->l_list.ol_elemc != length ||
			    DeeList_GET(me, i) != this_elem) {
				DeeList_LockDowngrade(me);
				goto again;
			}

			/* Override the element with its successors. */
			--length;
			me->l_list.ol_elemc = length;
			memmovedownc(me->l_list.ol_elemv + i,
			             me->l_list.ol_elemv + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			DeeList_LockEndWrite(me);

			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
			return 1;
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
	return 0;
}

/* @return: 0 : The given `keyed_search_item' count not found found.
 * @return: 1 : The given `keyed_search_item' was deleted once.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
DeeList_RRemove(DeeObject *self, size_t start, size_t end,
                DeeObject *keyed_search_item, DeeObject *key) {
	List *me = (List *)self;
	DeeObject **vector;
	size_t i, length;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	ASSERT_OBJECT_OPT(key);
	ASSERT_OBJECT(keyed_search_item);
	DeeList_LockRead(me);
again:
	vector = me->l_list.ol_elemv;
	length = me->l_list.ol_elemc;
	i      = end;
	if (i > length)
		i = length;
	for (;;) {
		DREF DeeObject *this_elem;
		int temp;
		if (i <= start)
			break;
		--i;
		this_elem = DeeList_GET(me, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(me);
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, this_elem, key);
		Dee_Decref(this_elem);
		if unlikely(temp < 0)
			return temp;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(me);

			/* Check if the list was changed. */
			if (me->l_list.ol_elemv != vector ||
			    me->l_list.ol_elemc != length ||
			    DeeList_GET(me, i) != this_elem) {
				DeeList_LockDowngrade(me);
				goto again;
			}

			/* Override the element with its successors. */
			--length;
			me->l_list.ol_elemc = length;
			memmovedownc(me->l_list.ol_elemv + i,
			             me->l_list.ol_elemv + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			DeeList_LockEndWrite(me);

			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
			return 1;
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
	return 0;
}


/* Remove all items matching `!!should(item)'
 * @return: * : The number of removed items.
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 4)) size_t DCALL
DeeList_RemoveIf(DeeObject *self, size_t start,
                 size_t end, DeeObject *should) {
	List *me = (List *)self;
	DeeObject **vector;
	size_t i, length, result;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	DeeList_LockRead(me);
again:
	result = 0;
	vector = me->l_list.ol_elemv;
	length = me->l_list.ol_elemc;
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *callback_result;
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(me, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(me);

		/* Invoke a predicate. */
		callback_result = DeeObject_Call(should, 1, &this_elem);
		Dee_Decref(this_elem);
		if unlikely(!callback_result)
			goto err;
		temp = DeeObject_Bool(callback_result);
		Dee_Decref(callback_result);
		if unlikely(temp < 0)
			goto err;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(me);

			/* Check if the list was changed. */
			if (me->l_list.ol_elemv != vector ||
			    me->l_list.ol_elemc != length ||
			    DeeList_GET(me, i) != this_elem) {
				DeeList_LockDowngrade(me);
				goto again;
			}

			/* Override the element with its successors. */
			--length;
			me->l_list.ol_elemc = length;
			memmovedownc(me->l_list.ol_elemv + i,
			             me->l_list.ol_elemv + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			++result;
			DeeList_LockEndWrite(me);

			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
	ASSERT(result != (size_t)-1);
	return result;
err:
	return (size_t)-1;
}

/* Resize `self' to have a length of `newsize'.
 * If the size increases, use `filler' for new items.
 * @return: 0 : Success.
 * @return: -1: Error. */
PUBLIC WUNUSED NONNULL((1, 3)) int DCALL
DeeList_Resize(DeeObject *self, size_t newsize, DeeObject *filler) {
	List *me = (List *)self;
again:
	DeeList_LockWrite(me);
	if (newsize > DeeList_SIZE(me)) {
		size_t old_elema;
		size_t num_more;
		old_elema = DeeList_GetAlloc(me);
		if (old_elema < newsize) {
			/* Try to resize the list to make it bigger. */
			DREF DeeObject **new_elemv;
			new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, newsize,
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_elemv) {
				DeeList_LockEndWrite(me);
				if (Dee_CollectMemoryc(newsize, sizeof(DREF DeeObject *)))
					goto again;
				goto err;
			}
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, newsize);
		}

		/* Fill in the new items. */
		num_more = newsize - me->l_list.ol_elemc;
		Dee_Setrefv(me->l_list.ol_elemv + me->l_list.ol_elemc, filler, num_more);
		me->l_list.ol_elemc = newsize;
		DeeList_LockEndWrite(me);
	} else if (newsize == me->l_list.ol_elemc) {
		/* Size didn't change */
		DeeList_LockEndWrite(me);
	} else if (!newsize) {
		/* Clear the list of all items. */
		DREF DeeObject **old_elemv;
		size_t old_elemc;
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
		size_t old_elema;
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */

		/* Remove everything */
		old_elemv = me->l_list.ol_elemv;
		old_elemc = me->l_list.ol_elemc;
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
		old_elema = DeeList_GetAlloc(me);
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */
		Dee_objectlist_init(&me->l_list);
		DeeList_LockEndWrite(me);
		Dee_Decrefv(old_elemv, old_elemc);
		DeeList_LockWrite(me);
		if likely(!me->l_list.ol_elemv) {
			ASSERT(me->l_list.ol_elemc == 0);

			/* Allow the list to re-use its old vector. */
			me->l_list.ol_elemv = old_elemv;
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
			_DeeList_SetAlloc(me, old_elema);
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */
			old_elemv = NULL; /* Inherit the old vector. */
		}
		DeeList_LockEndWrite(me);
		/* Free the old vector. */
		Dee_Free(old_elemv);
	} else {
		/* Must remove items. */
		DREF DeeObject **old_obj;
		size_t num_del;
		num_del = me->l_list.ol_elemc - newsize;
		old_obj = (DREF DeeObject **)Dee_TryMallocac(num_del, sizeof(DREF DeeObject *));
		if unlikely(!old_obj) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(newsize, sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}
		memcpyc(old_obj, &me->l_list.ol_elemv[newsize],
		        num_del, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc = newsize;
		DeeList_LockEndWrite(me);

		/* Drop references from all objects that were deleted. */
		Dee_Decrefv(old_obj, num_del);
		Dee_Freea(old_obj);
	}
	return 0;
err:
	return -1;
}


/* Remove all items matching `!!should(item)'
 * @return: * : The number of removed items.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
DeeList_RemoveAll(List *self, size_t start, size_t end,
                  DeeObject *keyed_search_item, DeeObject *key) {
	List *me = (List *)self;
	DeeObject **vector;
	size_t i, length, result;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	DeeList_LockRead(me);
again:
	result = 0;
	vector = me->l_list.ol_elemv;
	length = me->l_list.ol_elemc;
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(me, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(me);

		/* Invoke a predicate. */
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, this_elem, key);
		if unlikely(temp < 0)
			goto err;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(me);

			/* Check if the list was changed. */
			if (me->l_list.ol_elemv != vector ||
			    me->l_list.ol_elemc != length ||
			    DeeList_GET(me, i) != this_elem) {
				DeeList_LockDowngrade(me);
				goto again;
			}

			/* Override the element with its successors. */
			--length;
			me->l_list.ol_elemc = length;
			memmovedownc(me->l_list.ol_elemv + i,
			             me->l_list.ol_elemv + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			++result;
			DeeList_LockEndWrite(me);

			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
	ASSERT(result != (size_t)-1);
	return result;
err:
	return (size_t)-1;
}

/* Append objects to a given list. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeList_Append)(DeeObject *self, DeeObject *elem) {
	List *me = (List *)self;
	int result = 0;
	DeeObject **new_elemv;
	size_t old_elema;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	ASSERT_OBJECT(elem);
retry:
	DeeList_LockWrite(me);
	old_elema = DeeList_GetAlloc(me);
	ASSERT(old_elema >= me->l_list.ol_elemc);
	if (old_elema <= me->l_list.ol_elemc) {
		/* Must increase the list's capacity. */
		size_t new_elema = old_elema;
		new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
		if (new_elema < DEE_OBJECTLIST_MINALLOC)
			new_elema = DEE_OBJECTLIST_MINALLOC;
		new_elemv = (DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv,
		                                          new_elema, sizeof(DeeObject *));
		if unlikely(!new_elemv) {
			/* Try again, but only attempt to allocate for a single object. */
			new_elema = me->l_list.ol_elemc + 1;
			new_elemv = (DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv,
			                                          new_elema, sizeof(DeeObject *));
			if unlikely(!new_elemv) {
				DeeList_LockEndWrite(me);

				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(sizeof(DeeObject *)))
					goto retry;
				goto err;
			}
		}
		_DeeList_SetAlloc(me, new_elema);
		me->l_list.ol_elemv = new_elemv;
	}
	me->l_list.ol_elemv[me->l_list.ol_elemc++] = elem;
	Dee_Incref(elem);
	DeeList_LockEndWrite(me);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeList_AppendVector)(DeeObject *self, size_t objc,
                             DeeObject *const *objv) {
	List *me = (List *)self;
	int result = 0;
	DeeObject **new_elemv;
	size_t old_elemc, old_elema;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
retry:
	DeeList_LockWrite(me);
	old_elemc = me->l_list.ol_elemc;
	old_elema = DeeList_GetAlloc(me);
	ASSERT(old_elema >= old_elemc);
	if (old_elema < old_elemc + objc) {
		/* Must increase the list's capacity. */
		size_t new_elema = old_elema;
		if (new_elema < DEE_OBJECTLIST_MINALLOC)
			new_elema = DEE_OBJECTLIST_MINALLOC;
		while (new_elema < old_elemc + objc)
			new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
		new_elemv = (DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
		                                          sizeof(DeeObject *));
		if unlikely(!new_elemv) {
			/* Try again, but only attempt to allocate what we need. */
			new_elema = old_elemc + objc;
			new_elemv = (DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
			                                          sizeof(DeeObject *));
			if unlikely(!new_elemv) {
				DeeList_LockEndWrite(me);

				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemoryc(objc, sizeof(DeeObject *)))
					goto retry;
				goto err;
			}
		}
		me->l_list.ol_elemv = new_elemv;
		_DeeList_SetAlloc(me, new_elema);
	}

	/* Copy references */
	Dee_Movrefv(me->l_list.ol_elemv + old_elemc, objv, objc);
	me->l_list.ol_elemc = old_elemc + objc;
	DeeList_LockEndWrite(me);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeList_AppendIterator)(DeeObject *self,
                               DeeObject *iterator) {
	DREF DeeObject *elem;
	int error;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	ASSERT_OBJECT(iterator);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		error = DeeList_Append(self, elem);
		Dee_Decref(elem);
		if unlikely(error)
			return error;
		if unlikely(DeeThread_CheckInterrupt())
			goto err;
	}
	if likely(elem)
		return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeList_AppendSequence)(DeeObject *self,
                               DeeObject *sequence) {
	int error;
	size_t fast_seqlen;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	ASSERT_OBJECT(sequence);
	if (DeeTuple_CheckExact(sequence))
		return DeeList_AppendVector(self, DeeTuple_SIZE(sequence), DeeTuple_ELEM(sequence));
	fast_seqlen = DeeFastSeq_GetSize_deprecated(sequence);
	if (fast_seqlen != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		/* TODO: Special optimization. */
	}

	/* Fallback: Append elements from a general-purpose iterator. */
	sequence = DeeObject_Iter(sequence);
	if unlikely(!sequence)
		goto err;
	error = DeeList_AppendIterator(self, sequence);
	Dee_Decref(sequence);
	return error;
err:
	return -1;
}

/* Insert objects into a given list. */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeList_Insert)(DeeObject *self, size_t index,
                       DeeObject *elem) {
	size_t old_elema;
	List *me = (List *)self;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	ASSERT_OBJECT(elem);
	DeeList_LockWrite(me);
	if (index > me->l_list.ol_elemc)
		index = me->l_list.ol_elemc;
	old_elema = DeeList_GetAlloc(me);
	ASSERT(me->l_list.ol_elemc <= old_elema);
	if (me->l_list.ol_elemc == old_elema) {
		size_t new_elema = DEE_OBJECTLIST_MOREALLOC(old_elema);
		DREF DeeObject **new_elemv;
		if (new_elema < DEE_OBJECTLIST_MINALLOC)
			new_elema = DEE_OBJECTLIST_MINALLOC;
do_realloc:
		new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
		                                                sizeof(DREF DeeObject *));
		if unlikely(!new_elemv) {
			if (new_elema > me->l_list.ol_elemc + 1) {
				new_elema = me->l_list.ol_elemc + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
				goto do_realloc;
			DeeList_LockEndWrite(me);
			goto err;
		}
		me->l_list.ol_elemv = new_elemv;
		_DeeList_SetAlloc(me, new_elema);
	}

	/* Move objects above the point of insertion. */
	memmoveupc(me->l_list.ol_elemv + index + 1,
	           me->l_list.ol_elemv + index,
	           me->l_list.ol_elemc - index,
	           sizeof(DREF DeeObject *));

	/* Store the given element. */
	++me->l_list.ol_elemc;
	me->l_list.ol_elemv[index] = elem;
	Dee_Incref(elem);
	DeeList_LockEndWrite(me);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeList_InsertVector)(DeeObject *self,
                             size_t index, size_t objc,
                             DeeObject *const *objv) {
	List *me = (List *)self;
	size_t old_elema;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	DeeList_LockWrite(me);
	if (index > me->l_list.ol_elemc)
		index = me->l_list.ol_elemc;
	old_elema = DeeList_GetAlloc(me);
	ASSERT(me->l_list.ol_elemc <= old_elema);
	if (me->l_list.ol_elemc + objc >= old_elema) {
		size_t new_elema = DEE_OBJECTLIST_MOREALLOC(old_elema);
		DREF DeeObject **new_elemv;
		if (new_elema < DEE_OBJECTLIST_MINALLOC)
			new_elema = DEE_OBJECTLIST_MINALLOC;
		while (new_elema < me->l_list.ol_elemc + objc)
			new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
do_realloc:
		new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
		                                                sizeof(DREF DeeObject *));
		if unlikely(!new_elemv) {
			if (new_elema != me->l_list.ol_elemc + objc) {
				new_elema = me->l_list.ol_elemc + objc;
				goto do_realloc;
			}
			if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
				goto do_realloc;
			DeeList_LockEndWrite(me);
			goto err;
		}
		me->l_list.ol_elemv = new_elemv;
		_DeeList_SetAlloc(me, new_elema);
	}

	/* Move objects above the point of insertion. */
	memmoveupc(me->l_list.ol_elemv + index + objc,
	           me->l_list.ol_elemv + index,
	           me->l_list.ol_elemc - index,
	           sizeof(DREF DeeObject *));

	/* Store the given elements. */
	me->l_list.ol_elemc += objc;
	Dee_Movrefv(me->l_list.ol_elemv + index, objv, objc);
	DeeList_LockEndWrite(me);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeList_InsertIterator)(DeeObject *self, size_t index,
                               DeeObject *iterator) {
	DeeObject *elem;
	int error;
	ASSERT_OBJECT(iterator);
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		error = DeeList_Insert(self, index, elem);
		Dee_Decref(elem);
		if unlikely(error)
			return error;
		++index;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if likely(elem)
		return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeList_InsertSequence)(DeeObject *self, size_t index,
                               DeeObject *sequence) {
	DeeObject *iterator;
	int result;
	size_t fast_seqlen;
	ASSERT_OBJECT(sequence);
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);

	/* Optimization for known types. */
	if (DeeTuple_CheckExact(sequence))
		return DeeList_InsertVector(self, index, DeeTuple_SIZE(sequence), DeeTuple_ELEM(sequence));
	fast_seqlen = DeeFastSeq_GetSize_deprecated(sequence);
	if (fast_seqlen != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		/* TODO: Special optimization. */
	}
	iterator = DeeObject_Iter(sequence);
	if unlikely(!iterator)
		goto err;
	result = DeeList_InsertIterator(self, index, iterator);
	Dee_Decref(iterator);
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
list_init(List *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *sequence;
	DeeObject *filler = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:List", &sequence, &filler))
		goto err;
	weakref_support_init(self);
	Dee_atomic_rwlock_init(&self->l_lock);
	if (filler || DeeInt_Check(sequence)) {
		size_t list_size;
		if (DeeObject_AsSize(sequence, &list_size))
			goto err;
		if (!list_size) {
			self->l_list.ol_elemv = NULL;
		} else {
			if (filler == NULL)
				filler = Dee_None;
			self->l_list.ol_elemv = (DREF DeeObject **)Dee_Mallocc(list_size,
			                                                       sizeof(DREF DeeObject *));
			if unlikely(!self->l_list.ol_elemv)
				goto err;
			Dee_Setrefv(self->l_list.ol_elemv, filler, list_size);
		}
		self->l_list.ol_elemc = list_size;
		_DeeList_SetAlloc(self, list_size);
		return 0;
	}

	/* Fallback: initialize from a generic sequence. */
	return Dee_objectlist_init_fromseq(&self->l_list, sequence);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
list_visit(List *__restrict self,
           dvisit_t proc, void *arg) {
	DeeList_LockRead(self);
	Dee_Visitv(DeeList_ELEM(self),
	           DeeList_SIZE(self));
	DeeList_LockEndRead(self);
}

/* Clear the given list.
 * Returns `true' if the list wasn't empty before. */
PUBLIC NONNULL((1)) bool DCALL
DeeList_Clear(DeeObject *__restrict self) {
	List *me = (List *)self;
	DREF DeeObject **elemv = NULL;
	size_t elemc;
	DeeList_LockWrite(me);
	elemc = me->l_list.ol_elemc;
	if (elemc) {
		elemv = me->l_list.ol_elemv;
		Dee_objectlist_init(&me->l_list);
	}
	DeeList_LockEndWrite(me);
	Dee_Decrefv(elemv, elemc);
	Dee_Free(elemv);
	return elemc != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
list_printrepr(DeeObject *__restrict self,
               dformatprinter printer, void *arg) {
	List *me = (List *)self;
	size_t i;
	dssize_t temp, result = 0;
	temp = (*printer)(arg, "[", 1);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	DeeList_LockRead(me);
	for (i = 0; i < me->l_list.ol_elemc; ++i) {
		DREF DeeObject *elem = me->l_list.ol_elemv[i];
		Dee_Incref(elem);
		DeeList_LockEndRead(me);
		/* Print this item. */
		if (i) {
			temp = (*printer)(arg, ", ", 2);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		temp = DeeObject_PrintRepr(elem, printer, arg);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		DeeList_LockRead(me);
	}
	DeeList_LockEndRead(me);
	temp = (*printer)(arg, "]", 1);
	if unlikely(temp < 0)
		goto err;
	return result + temp;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_bool(List *__restrict me) {
	return DeeList_SIZE_ATOMIC(me) != 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sizeob(List *__restrict me) {
	return DeeInt_NewSize(DeeList_SIZE_ATOMIC(me));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_contains(List *me, DeeObject *elem) {
	DeeObject **iter, **end, *list_elem;
	DeeList_LockRead(me);
again:
	iter = me->l_list.ol_elemv;
	end  = iter + me->l_list.ol_elemc;
	for (; iter < end; ++iter) {
		int error;
		list_elem = *iter;
		Dee_Incref(list_elem);
		DeeList_LockEndRead(me);
		error = DeeObject_TryCompareEq(elem, list_elem);
		Dee_Decref_unlikely(list_elem);
		if (error == 0)
			return_true;
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		DeeList_LockRead(me);
		/* Check if the list was changed. */
		if unlikely(end != me->l_list.ol_elemv + me->l_list.ol_elemc ||
		            iter < me->l_list.ol_elemv)
			goto again;
	}
	DeeList_LockEndRead(me);
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_getitem(List *me, DeeObject *index) {
	size_t i;
	DREF DeeObject *result;
	if (DeeObject_AsSize(index, &i))
		goto err;
	DeeList_LockRead(me);
	if unlikely(i >= DeeList_SIZE(me)) {
		size_t list_size = DeeList_SIZE(me);
		DeeList_LockEndRead(me);
		err_index_out_of_bounds((DeeObject *)me, i, list_size);
		goto err;
	}
	result = DeeList_GET(me, i);
	Dee_Incref(result);
	DeeList_LockEndRead(me);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_index(List *__restrict me, dssize_t i_begin, dssize_t i_end) {
	struct Dee_seq_range range;
	size_t range_size;
	DREF DeeObject **new_elemv;
	DREF List *result;
again:
	DeeList_LockRead(me);
	DeeSeqRange_Clamp(&range, i_begin, i_end, me->l_list.ol_elemc);
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size <= 0) {
		/* Empty list. */
		DeeList_LockEndRead(me);
		return DeeList_New();
	}
	new_elemv = (DREF DeeObject **)Dee_TryMallocc(range_size, sizeof(DREF DeeObject *));
	if unlikely(!new_elemv) {
		DeeList_LockEndRead(me);
		if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
			goto again;
		return NULL;
	}

	/* Copy vector elements. */
	Dee_Movrefv(new_elemv,
	            me->l_list.ol_elemv + range.sr_start,
	            range_size);
	DeeList_LockEndRead(me);

	/* Create the new list descriptor. */
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err_elemv;

	/* Fill in the descriptor. */
	DeeObject_Init(result, &DeeList_Type);
	result->l_list.ol_elemc = range_size;
	_DeeList_SetAlloc(result, range_size);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	result->l_list.ol_elemv = new_elemv;

	/* Start tracking it as a GC object. */
	return DeeGC_Track((DeeObject *)result);

err_elemv:
	/* Cleanup on error. */
	Dee_Decrefv(new_elemv, range_size);
	Dee_Free(new_elemv);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_index_n(List *__restrict me, dssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return list_getrange_index(me, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject **new_elemv;
	DREF List *result;
	size_t start, range_size;
again:
	DeeList_LockRead(me);
	start = DeeSeqRange_Clamp_n(i_begin, me->l_list.ol_elemc);
	range_size = me->l_list.ol_elemc - start;
	if unlikely(range_size <= 0) {
		/* Empty list. */
		DeeList_LockEndRead(me);
		return DeeList_New();
	}
	ASSERT(range_size != 0);
	new_elemv = (DREF DeeObject **)Dee_TryMallocc(range_size, sizeof(DREF DeeObject *));
	if unlikely(!new_elemv) {
		DeeList_LockEndRead(me);
		if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
			goto again;
		return NULL;
	}

	/* Copy vector elements. */
	Dee_Movrefv(new_elemv, me->l_list.ol_elemv + start, range_size);
	DeeList_LockEndRead(me);

	/* Create the new list descriptor. */
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err_elemv;

	/* Fill in the descriptor. */
	DeeObject_Init(result, &DeeList_Type);
	result->l_list.ol_elemc = range_size;
	_DeeList_SetAlloc(result, range_size);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	result->l_list.ol_elemv = new_elemv;

	/* Start tracking it as a GC object. */
	return DeeGC_Track((DeeObject *)result);

err_elemv:
	/* Cleanup on error. */
	Dee_Decrefv(new_elemv, range_size);
	Dee_Free(new_elemv);
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
list_getrange(List *me, DeeObject *begin, DeeObject *end) {
	dssize_t i_begin, i_end;
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (DeeNone_Check(end))
		return list_getrange_index_n(me, i_begin);
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	return list_getrange_index(me, i_begin, i_end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
list_delitem_index(List *__restrict me, size_t index) {
	DREF DeeObject *delob;
	size_t length;
	DeeList_LockWrite(me);
	length = me->l_list.ol_elemc;
	if unlikely(index >= length)
		goto unlock_and_err_index;

	/* Adjust to shift following elements downwards. */
	delob = DeeList_GET(me, index);
	--length;
	me->l_list.ol_elemc = length;
	memmovedownc(DeeList_ELEM(me) + index,
	             DeeList_ELEM(me) + index + 1,
	             length - index,
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(me);
	Dee_Decref(delob);
	return 0;
unlock_and_err_index:
	DeeList_LockEndWrite(me);
	return err_index_out_of_bounds((DeeObject *)me,
	                               index,
	                               length);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_delitem(List *me, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return list_delitem_index(me, i);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
list_setitem_index(List *me, size_t index, DeeObject *value) {
	DREF DeeObject *old_item;
	DeeList_LockWrite(me);
	if unlikely(index >= DeeList_SIZE(me))
		goto unlock_and_err_index;
	old_item = DeeList_GET(me, index);
	Dee_Incref(value);
	DeeList_SET(me, index, value);
	DeeList_LockEndWrite(me);
	Dee_Decref(old_item);
	return 0;
	{
		size_t length;
unlock_and_err_index:
		length = DeeList_SIZE(me);
		DeeList_LockEndWrite(me);
		return err_index_out_of_bounds((DeeObject *)me,
		                               index,
		                               length);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_bounditem_index(List *me, size_t index) {
	return index < DeeList_SIZE_ATOMIC(me) ? 1 : -2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_hasitem_index(List *me, size_t index) {
	return index < DeeList_SIZE_ATOMIC(me) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
list_setitem(List *me, DeeObject *index, DeeObject *value) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return list_setitem_index(me, i, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF ListIterator *DCALL
list_iter(List *__restrict me) {
	DREF ListIterator *result;
	result = DeeObject_MALLOC(ListIterator);
	if unlikely(!result)
		goto done;
	result->li_list  = me;
	result->li_index = 0;
	Dee_Incref(me);
	DeeObject_Init(result, &DeeListIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delrange_index(List *__restrict me, dssize_t i_begin, dssize_t i_end) {
	DREF DeeObject **delobv;
	struct Dee_seq_range range;
	size_t range_size;
again:
	DeeList_LockWrite(me);
	DeeSeqRange_Clamp(&range, i_begin, i_end, DeeList_SIZE(me));
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size <= 0)
		goto done_noop;
	delobv = (DREF DeeObject **)Dee_TryMallocac(range_size,
	                                            sizeof(DREF DeeObject *));
	if unlikely(!delobv) {
		DeeList_LockEndWrite(me);
		if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}

	/* Move all items to-be deleted into the delete-vector. */
	memcpyc(delobv, DeeList_ELEM(me) + range.sr_start,
	        range_size, sizeof(DREF DeeObject *));
	memmovedownc(DeeList_ELEM(me) + range.sr_start,
	             DeeList_ELEM(me) + range.sr_end,
	             DeeList_SIZE(me) - range.sr_end,
	             sizeof(DREF DeeObject *));
	me->l_list.ol_elemc -= range_size;
	DeeList_LockEndWrite(me);

	/* Drop object references. */
	Dee_Decrefv(delobv, range_size);

	/* Free the temporary del-item vector. */
	Dee_Freea(delobv);
done:
	return 0;
err:
	return -1;
done_noop:
	DeeList_LockEndWrite(me);
	goto done;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delrange_index_n(List *__restrict me, dssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return list_delrange_index(me, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject **delobv;
	size_t start, range_size;
again:
	DeeList_LockWrite(me);
	start = DeeSeqRange_Clamp_n(i_begin, DeeList_SIZE(me));
	range_size = DeeList_SIZE(me) - start;
	if unlikely(range_size <= 0)
		goto done_noop;
	delobv = (DREF DeeObject **)Dee_TryMallocac(range_size, sizeof(DREF DeeObject *));
	if unlikely(!delobv) {
		DeeList_LockEndWrite(me);
		if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}

	/* Move all items to-be deleted into the delete-vector. */
	memcpyc(delobv, DeeList_ELEM(me) + start,
	        range_size, sizeof(DREF DeeObject *));
	me->l_list.ol_elemc -= range_size;
	DeeList_LockEndWrite(me);

	/* Drop object references. */
	Dee_Decrefv(delobv, range_size);

	/* Free the temporary del-item vector. */
	Dee_Freea(delobv);
done:
	return 0;
err:
	return -1;
done_noop:
	DeeList_LockEndWrite(me);
	goto done;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
list_setrange_fast(List *me, dssize_t i_begin, dssize_t i_end,
                   DeeObject *items, size_t insert_count) {
	DREF DeeObject **delobv;
	struct Dee_seq_range range;
	size_t range_size, i;
	if unlikely(!insert_count)
		return list_delrange_index(me, i_begin, i_end);
again:
	DeeList_LockWrite(me);
	DeeSeqRange_Clamp(&range, i_begin, i_end, me->l_list.ol_elemc);
	range_size = range.sr_end - range.sr_start;
	ASSERT(range_size <= DeeList_SIZE(me));
	if (insert_count > range_size) {
		/* Make sure the list has enough available memory. */
		size_t min_elema = (DeeList_SIZE(me) - range_size) + insert_count;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < DEE_OBJECTLIST_MINALLOC)
				new_elema = DEE_OBJECTLIST_MINALLOC;
			do {
				new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
				                                               sizeof(DREF DeeObject *));
				if unlikely(!new_elemv) {
					DeeList_LockEndWrite(me);
					/* Collect memory and try again. */
					if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
						goto again;
					goto err;
				}
			}
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
		}
	}
	if (range_size <= 0) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(me) + range.sr_start + insert_count,
		           DeeList_ELEM(me) + range.sr_start,
		           DeeList_SIZE(me) - range.sr_start,
		           sizeof(DREF DeeObject *));

		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(me, range.sr_start + i, DeeFastSeq_GetItemNB_deprecated(items, i));
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);
	} else {
		delobv = (DREF DeeObject **)Dee_TryMallocac(range_size,
		                                            sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(me) + range.sr_start,
		        range_size, sizeof(DREF DeeObject *));

		/* Move following items to their proper places. */
		if (range.sr_start + insert_count != range.sr_end) {
			memmovec(DeeList_ELEM(me) + range.sr_start + insert_count,
			         DeeList_ELEM(me) + range.sr_end,
			         DeeList_SIZE(me) - range.sr_end,
			         sizeof(DREF DeeObject *));
		}

		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(me, range.sr_start + i, DeeFastSeq_GetItemNB_deprecated(items, i));
		me->l_list.ol_elemc -= range_size;
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);

		/* Drop object references. */
		Dee_Decrefv(delobv, range_size);

		/* Free the temporary del-item vector. */
		Dee_Freea(delobv);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
list_setrange_fast_n(List *me, dssize_t i_begin,
                     DeeObject *items, size_t insert_count) {
#ifdef __OPTIMIZE_SIZE__
	return list_setrange_fast(me, i_begin, SSIZE_MAX, items, insert_count);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject **delobv;
	size_t range_size, i, start;
again:
	DeeList_LockWrite(me);
	start = DeeSeqRange_Clamp_n(i_begin, me->l_list.ol_elemc);
	range_size = me->l_list.ol_elemc - start;
	ASSERT(range_size <= DeeList_SIZE(me));
	if (insert_count > range_size) {
		/* Make sure the list has enough available memory. */
		size_t min_elema = (DeeList_SIZE(me) - range_size) + insert_count;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < DEE_OBJECTLIST_MINALLOC)
				new_elema = DEE_OBJECTLIST_MINALLOC;
			do {
				new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
				                                               sizeof(DREF DeeObject *));
				if unlikely(!new_elemv) {
					DeeList_LockEndWrite(me);
					/* Collect memory and try again. */
					if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
						goto again;
					goto err;
				}
			}
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
		}
	}
	if (!range_size) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(me) + start + insert_count,
		           DeeList_ELEM(me) + start,
		           DeeList_SIZE(me) - start,
		           sizeof(DREF DeeObject *));

		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(me, start + i, DeeFastSeq_GetItemNB_deprecated(items, i));
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);
	} else {
		delobv = (DREF DeeObject **)Dee_TryMallocac(range_size, sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(me) + start,
		        range_size, sizeof(DREF DeeObject *));

		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(me, start + i, DeeFastSeq_GetItemNB_deprecated(items, i));
		me->l_list.ol_elemc -= range_size;
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);

		/* Drop object references. */
		Dee_Decrefv(delobv, range_size);

		/* Free the temporary del-item vector. */
		Dee_Freea(delobv);
	}
	return 0;
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
list_setrange_index(List *me, dssize_t i_begin,
                  dssize_t i_end, DeeObject *items) {
	DREF DeeObject **delobv, **insertv;
	size_t range_size, insert_count;
	struct Dee_seq_range range;

	/* Check for special case: Fast-insert */
	insert_count = DeeFastSeq_GetSizeNB_deprecated(items);
	if (insert_count != DEE_FASTSEQ_NOTFAST_DEPRECATED)
		return list_setrange_fast(me, i_begin, i_end, items, insert_count);
	insertv = DeeSeq_AsHeapVector(items, &insert_count);
	if unlikely(!insertv)
		goto err;
	if unlikely(!insert_count) {
		Dee_Free(insertv);
		return list_delrange_index(me, i_begin, i_end);
	}
again:
	DeeList_LockWrite(me);
	DeeSeqRange_Clamp(&range, i_begin, i_end, me->l_list.ol_elemc);
	range_size = range.sr_end - range.sr_start;
	ASSERT(range_size <= DeeList_SIZE(me));
	if (range_size == me->l_list.ol_elemc) {
		/* Special case: assign to the entirety of the list
		 * -> Here, we can simply swap vectors. */
		DeeObject **old_elemv;
		size_t old_elemc;
		old_elemv = me->l_list.ol_elemv;
		old_elemc = me->l_list.ol_elemc;
		me->l_list.ol_elemv = insertv; /* Inherit */
		me->l_list.ol_elemc = insert_count;
		_DeeList_SetAlloc(me, insert_count);
		DeeList_LockEndWrite(me);
		old_elemv = Dee_Decrefv(old_elemv, old_elemc);
		Dee_Free(old_elemv);
		goto done;
	}
	if (insert_count > range_size) {
		/* Make sure the list has enough available memory. */
		size_t min_elema = (DeeList_SIZE(me) - range_size) + insert_count;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < DEE_OBJECTLIST_MINALLOC)
				new_elema = DEE_OBJECTLIST_MINALLOC;
			do {
				new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
				                                               sizeof(DREF DeeObject *));
				if unlikely(!new_elemv) {
					DeeList_LockEndWrite(me);
					/* Collect memory and try again. */
					if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
						goto again;
					goto err_insertv;
				}
			}
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
		}
	}
	if (!range_size) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(me) + range.sr_start + insert_count,
		           DeeList_ELEM(me) + range.sr_start,
		           DeeList_SIZE(me) - range.sr_start,
		           sizeof(DREF DeeObject *));

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + range.sr_start, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);
		Dee_Free(insertv);
	} else {
		delobv = (DREF DeeObject **)Dee_TryMallocac(range_size, sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
				goto again;
			goto err_insertv;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(me) + range.sr_start,
		        range_size, sizeof(DREF DeeObject *));

		/* Move following items to their proper places. */
		if (range.sr_start + insert_count != range.sr_end) {
			memmovec(DeeList_ELEM(me) + range.sr_start + insert_count,
			         DeeList_ELEM(me) + range.sr_end,
			         DeeList_SIZE(me) - range.sr_end,
			         sizeof(DREF DeeObject *));
		}

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + range.sr_start, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc -= range_size;
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);
		Dee_Free(insertv);

		/* Drop object references. */
		Dee_Decrefv(delobv, range_size);

		/* Free the temporary del-item vector. */
		Dee_Freea(delobv);
	}
done:
	return 0;
err_insertv:
	Dee_Decrefv(insertv, insert_count);
	Dee_Free(insertv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
list_setrange_index_n(List *me, dssize_t i_begin, DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
	return list_setrange_index(me, i_begin, SSIZE_MAX, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject **delobv, **insertv;
	size_t range_size, insert_count, start;

	/* Check for special case: Fast-insert */
	insert_count = DeeFastSeq_GetSizeNB_deprecated(items);
	if (insert_count != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (!insert_count)
			return list_delrange_index_n(me, i_begin);
		return list_setrange_fast_n(me, i_begin, items, insert_count);
	}
	insertv = DeeSeq_AsHeapVector(items, &insert_count);
	if unlikely(!insertv)
		goto err;
	if unlikely(!insert_count) {
		Dee_Free(insertv);
		return list_delrange_index_n(me, i_begin);
	}
again:
	DeeList_LockWrite(me);
	start = DeeSeqRange_Clamp_n(i_begin, me->l_list.ol_elemc);
	if (start == 0) {
		/* Special case: assign to the entirety of the list
		 * -> Here, we can simply swap vectors. */
		DeeObject **old_elemv;
		size_t old_elemc;
		old_elemv = me->l_list.ol_elemv;
		old_elemc = me->l_list.ol_elemc;
		me->l_list.ol_elemv = insertv; /* Inherit */
		me->l_list.ol_elemc = insert_count;
		_DeeList_SetAlloc(me, insert_count);
		DeeList_LockEndWrite(me);
		old_elemv = Dee_Decrefv(old_elemv, old_elemc);
		Dee_Free(old_elemv);
		goto done;
	}
	range_size = me->l_list.ol_elemc - start;
	ASSERT(range_size <= DeeList_SIZE(me));
	if (insert_count > range_size) {
		/* Make sure the list has enough available memory. */
		size_t min_elema = (DeeList_SIZE(me) - range_size) + insert_count;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < DEE_OBJECTLIST_MINALLOC)
				new_elema = DEE_OBJECTLIST_MINALLOC;
			do {
				new_elema = DEE_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
				                                               sizeof(DREF DeeObject *));
				if unlikely(!new_elemv) {
					DeeList_LockEndWrite(me);
					/* Collect memory and try again. */
					if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
						goto again;
					goto err_insertv;
				}
			}
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
		}
	}
	if (!range_size) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(me) + start + insert_count,
		           DeeList_ELEM(me) + start,
		           DeeList_SIZE(me) - start,
		           sizeof(DREF DeeObject *));

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + start, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);
		Dee_Free(insertv);
	} else {
		delobv = (DREF DeeObject **)Dee_TryMallocac(range_size, sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(range_size, sizeof(DREF DeeObject *)))
				goto again;
			goto err_insertv;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(me) + start,
		        range_size, sizeof(DREF DeeObject *));

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + start, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc -= range_size;
		me->l_list.ol_elemc += insert_count;
		DeeList_LockEndWrite(me);
		Dee_Free(insertv);

		/* Drop object references. */
		Dee_Decrefv(delobv, range_size);

		/* Free the temporary del-item vector. */
		Dee_Freea(delobv);
	}
done:
	return 0;
err_insertv:
	Dee_Decrefv(insertv, insert_count);
	Dee_Free(insertv);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}



PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
list_delrange(List *me, DeeObject *begin, DeeObject *end) {
	dssize_t i_begin, i_end;
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (DeeNone_Check(end))
		return list_delrange_index_n(me, i_begin);
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	return list_delrange_index(me, i_begin, i_end);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
list_setrange(List *me, DeeObject *begin,
              DeeObject *end, DeeObject *items) {
	dssize_t i_begin, i_end;
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (DeeNone_Check(end))
		return list_setrange_index_n(me, i_begin, items);
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	return list_setrange_index(me, i_begin, i_end, items);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_size(List *__restrict me) {
	ASSERT(me->l_list.ol_elemc != (size_t)-1);
	return atomic_read(&me->l_list.ol_elemc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getitem_index(List *__restrict me, size_t index) {
	DREF DeeObject *result;
	DeeList_LockRead(me);
	if unlikely(index >= DeeList_SIZE(me)) {
		size_t list_size = DeeList_SIZE(me);
		DeeList_LockEndRead(me);
		err_index_out_of_bounds((DeeObject *)me, index, list_size);
		return NULL;
	}
	result = DeeList_GET(me, index);
	Dee_Incref(result);
	DeeList_LockEndRead(me);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getitem_index_fast(List *__restrict me, size_t index) {
	DREF DeeObject *result;
	DeeList_LockRead(me);
	if unlikely(index >= DeeList_SIZE(me)) {
		DeeList_LockEndRead(me);
		return NULL;
	}
	result = DeeList_GET(me, index);
	Dee_Incref(result);
	DeeList_LockEndRead(me);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
list_nsi_find(List *me,
              size_t start, size_t end,
              DeeObject *keyed_search_item,
              DeeObject *key) {
	DREF DeeObject *list_elem;
	size_t i;
	int temp;
	DeeList_LockRead(me);
	for (i = start; i < DeeList_SIZE(me) && i < end; ++i) {
		list_elem = DeeList_GET(me, i);
		Dee_Incref(list_elem);
		DeeList_LockEndRead(me);
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, list_elem, key);
		Dee_Decref_unlikely(list_elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i; /* Found it! */
		}
		DeeList_LockRead(me);
	}
	DeeList_LockEndRead(me);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
list_nsi_rfind(List *me, size_t start, size_t end,
               DeeObject *keyed_search_item,
               DeeObject *key) {
	DREF DeeObject *list_elem;
	size_t i;
	int temp;
	DeeList_LockRead(me);
	i = end;
	for (;;) {
		if (i > DeeList_SIZE(me))
			i = DeeList_SIZE(me);
		if (i <= start)
			break;
		--i;
		list_elem = DeeList_GET(me, i);
		Dee_Incref(list_elem);
		DeeList_LockEndRead(me);
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, list_elem, key);
		Dee_Decref_unlikely(list_elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i; /* Found it! */
		}
		DeeList_LockRead(me);
	}
	DeeList_LockEndRead(me);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
list_nsi_xch(List *me, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	DeeList_LockWrite(me);
	if (index >= DeeList_SIZE(me)) {
		size_t my_length = DeeList_SIZE(me);
		DeeList_LockEndWrite(me);
		err_index_out_of_bounds((DeeObject *)me, index, my_length);
		return NULL;
	}
	Dee_Incref(value);
	result = DeeList_GET(me, index); /* Inherit reference. */
	DeeList_SET(me, index, value);   /* Inherit reference. */
	DeeList_LockEndWrite(me);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
list_foreach(List *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	DeeList_LockRead(self);
	for (i = 0; i < self->l_list.ol_elemc; ++i) {
		DREF DeeObject *list_elem;
		list_elem = self->l_list.ol_elemv[i];
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		temp = (*proc)(arg, list_elem);
		Dee_Decref_unlikely(list_elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
	return result;
err:
	return temp;
}


PRIVATE struct type_nsi tpconst list_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&list_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&list_size,
			/* .nsi_getitem      = */ (dfunptr_t)&list_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)&list_delitem_index,
			/* .nsi_setitem      = */ (dfunptr_t)&list_setitem_index,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)&list_getrange_index,
			/* .nsi_getrange_n   = */ (dfunptr_t)&list_getrange_index_n,
			/* .nsi_delrange     = */ (dfunptr_t)&list_delrange_index,
			/* .nsi_delrange_n   = */ (dfunptr_t)&list_delrange_index_n,
			/* .nsi_setrange     = */ (dfunptr_t)&list_setrange_index,
			/* .nsi_setrange_n   = */ (dfunptr_t)&list_setrange_index_n,
			/* .nsi_find         = */ (dfunptr_t)&list_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&list_nsi_rfind,
			/* .nsi_xch          = */ (dfunptr_t)&list_nsi_xch,
			/* .nsi_insert       = */ (dfunptr_t)&DeeList_Insert,
			/* .nsi_insertall    = */ (dfunptr_t)&DeeList_InsertSequence,
			/* .nsi_insertvec    = */ (dfunptr_t)&DeeList_InsertVector,
			/* .nsi_pop          = */ (dfunptr_t)&DeeList_Pop,
			/* .nsi_erase        = */ (dfunptr_t)&DeeList_Erase,
			/* .nsi_remove       = */ (dfunptr_t)&DeeList_Remove,
			/* .nsi_rremove      = */ (dfunptr_t)&DeeList_RRemove,
			/* .nsi_removeall    = */ (dfunptr_t)&DeeList_RemoveAll,
			/* .nsi_removeif     = */ (dfunptr_t)&DeeList_RemoveIf
		}
	}
};

PRIVATE struct type_seq list_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_setitem,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&list_setrange,
	/* .tp_nsi                        = */ &list_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&list_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL, /* default */
	/* .tp_hasitem                    = */ NULL, /* default */
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&list_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&list_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&list_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&list_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&list_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&list_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&list_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&list_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&list_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&list_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&list_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&list_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&list_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&list_setrange_index_n,
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


INTDEF struct keyword seq_insert_kwlist[];
INTDEF struct keyword seq_insertall_kwlist[];
INTDEF struct keyword seq_erase_kwlist[];
INTDEF struct keyword seq_xch_kwlist[];
INTDEF struct keyword seq_removeif_kwlist[];
INTDEF struct keyword seq_pop_kwlist[];
INTDEF struct keyword seq_resize_kwlist[];

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_append(List *me, size_t argc, DeeObject *const *argv) {
	/* Optimize for the case of a single object to-be appended. */
	if likely(argc == 1) {
		if (DeeList_Append((DeeObject *)me, argv[0]))
			goto err;
	} else {
		if (DeeList_AppendVector((DeeObject *)me, argc, argv))
			goto err;
	}
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_extend(List *me, size_t argc, DeeObject *const *argv) {
	DeeObject *extension;
	if (DeeArg_Unpack(argc, argv, "o:extend", &extension))
		goto err;
	if (DeeList_AppendSequence((DeeObject *)me, extension))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_insert(List *me, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insert_kwlist,
	                    UNPuSIZ "o:insert", &index, &item))
		goto err;
	if (DeeList_Insert((DeeObject *)me, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_insertall(List *me, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insertall_kwlist,
	                    UNPuSIZ "o:insertall", &index, &items))
		goto err;
	if (DeeList_InsertSequence((DeeObject *)me, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_removeif(List *me, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *should;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_removeif_kwlist,
	                    "o|" UNPuSIZ UNPuSIZ ":removeif",
	                    &should, &start, &end))
		goto err;
	result = DeeList_RemoveIf((DeeObject *)me, start, end, should);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
list_insertiter_deprecated(List *me,
                           size_t argc, DeeObject *const *argv) {
	size_t index;
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "o:insert_iter", &index, &seq))
		goto err;
	if (DeeList_InsertIterator((DeeObject *)me, index, seq))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_erase(List *me, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_erase_kwlist,
	                    UNPuSIZ "|" UNPuSIZ ":erase",
	                    &index, &count))
		goto err;
	count = DeeList_Erase((DeeObject *)me, index, count);
	if unlikely(count == (size_t)-1)
		goto err;
	return DeeInt_NewSize(count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_xch(List *me, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *value;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_xch_kwlist,
	                    UNPuSIZ "o:xch", &index, &value))
		goto err;
	return list_nsi_xch(me, index, value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_pop(List *me, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	dssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_pop_kwlist,
	                    "|" UNPdSIZ ":pop", &index))
		goto err;
	return DeeList_Pop((DeeObject *)me, index);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
list_clear(List *me, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	DeeList_Clear((DeeObject *)me);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sizeof(List *me) {
	size_t result = sizeof(List);
	result += DeeList_SIZE_ATOMIC(me) * sizeof(DeeObject *);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_get_first(List *__restrict me) {
	DREF DeeObject *result;
	DeeList_LockRead(me);
	if unlikely(DeeList_IsEmpty(me))
		goto err_empty_endread;
	result = DeeList_GET(me, 0);
	Dee_Incref(result);
	DeeList_LockEndRead(me);
	return result;
err_empty_endread:
	DeeList_LockEndRead(me);
	err_empty_sequence((DeeObject *)me); /* TODO: Unbound attribute */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_del_first(List *__restrict me) {
	DREF DeeObject *delob;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	DeeList_LockWrite(me);
	if unlikely(DeeList_IsEmpty(me))
		goto err_empty_endwrite;

	/* Adjust to shift following elements downwards. */
	delob = DeeList_GET(me, 0);
	--me->l_list.ol_elemc;
	memmovedownc(DeeList_ELEM(me),
	             DeeList_ELEM(me) + 1,
	             DeeList_SIZE(me),
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(me);
	Dee_Decref(delob);
	return 0;
err_empty_endwrite:
	DeeList_LockEndWrite(me);
	return err_empty_sequence((DeeObject *)me);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_set_first(List *me, DeeObject *value) {
	DREF DeeObject *old_elem;
	DeeList_LockWrite(me);
	if unlikely(DeeList_IsEmpty(me))
		goto err_empty_endwrite;
	Dee_Incref(value);
	old_elem = DeeList_GET(me, 0);
	DeeList_SET(me, 0, value);
	DeeList_LockEndWrite(me);
	Dee_Decref(old_elem);
	return 0;
err_empty_endwrite:
	DeeList_LockEndWrite(me);
	return err_empty_sequence((DeeObject *)me);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_get_last(List *__restrict me) {
	DREF DeeObject *result;
	DeeList_LockRead(me);
	if unlikely(DeeList_IsEmpty(me))
		goto err_empty_endread;
	result = DeeList_GET(me, DeeList_SIZE(me) - 1);
	Dee_Incref(result);
	DeeList_LockEndRead(me);
	return result;
err_empty_endread:
	DeeList_LockEndRead(me);
	err_empty_sequence((DeeObject *)me); /* TODO: Unbound attribute */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_del_last(List *__restrict me) {
	DREF DeeObject *delob;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	DeeList_LockWrite(me);
	if unlikely(DeeList_IsEmpty(me))
		goto err_empty_endwrite;
	--me->l_list.ol_elemc;
	delob = DeeList_GET(me, DeeList_SIZE(me));
	DeeList_LockEndWrite(me);
	Dee_Decref(delob);
	return 0;
err_empty_endwrite:
	DeeList_LockEndWrite(me);
	return err_empty_sequence((DeeObject *)me);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_set_last(List *me, DeeObject *value) {
	DREF DeeObject *old_elem;
	size_t index;
	DeeList_LockWrite(me);
	if unlikely(DeeList_IsEmpty(me))
		goto err_empty_endwrite;
	index = DeeList_SIZE(me) - 1;
	Dee_Incref(value);
	old_elem = DeeList_GET(me, index);
	DeeList_SET(me, index, value);
	DeeList_LockEndWrite(me);
	Dee_Decref(old_elem);
	return 0;
err_empty_endwrite:
	DeeList_LockEndWrite(me);
	return err_empty_sequence((DeeObject *)me);
}

#ifdef __OPTIMIZE_SIZE__
#define list_get_frozen DeeTuple_FromSequence
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
list_get_frozen(List *__restrict me) {
	size_t count;
	DREF DeeTupleObject *result;
again:
	DeeList_LockWrite(me);
	count  = DeeList_SIZE(me);
	result = DeeTuple_TryNewUninitialized(count);
	if unlikely(!result) {
		DeeList_LockEndWrite(me);
		if (Dee_CollectMemory(DeeTuple_SIZEOF(count)))
			goto again;
		return NULL;
	}

	/* Copy elements and create new references. */
	Dee_Movrefv(DeeTuple_ELEM(result), DeeList_ELEM(me), count);
	DeeList_LockEndWrite(me);
	return result;
}
#endif /* !__OPTIMIZE_SIZE__ */



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getallocated(List *__restrict me) {
	size_t result;
#ifdef DeeList_GetAlloc_ATOMIC
	result = DeeList_GetAlloc_ATOMIC(me);
#else /* DeeList_GetAlloc_ATOMIC */
	DeeList_LockRead(me);
	result = DeeList_GetAlloc(me);
	DeeList_LockEndRead(me);
#endif /* !DeeList_GetAlloc_ATOMIC */
	return DeeInt_NewSize(result);
}

PRIVATE NONNULL((1)) void DCALL
list_do_shrink(List *__restrict me) {
	size_t old_elema;
	DeeList_LockWrite(me);
	old_elema = DeeList_GetAlloc(me);
	if (me->l_list.ol_elemc < old_elema) {
		size_t new_elema = me->l_list.ol_elemc;
		DREF DeeObject **new_elemv;
		new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
		                                               sizeof(DREF DeeObject *));
		if likely(new_elemv) {
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
		}
	}
	DeeList_LockEndWrite(me);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delallocated(List *__restrict me) {
	list_do_shrink(me);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_setallocated(List *me, DeeObject *value) {
	size_t old_elema;
	size_t new_elema;
	if (DeeObject_AsSize(value, &new_elema))
		goto err;
	DeeList_LockWrite(me);
	old_elema = DeeList_GetAlloc(me);
	ASSERT(old_elema >= DeeList_SIZE(me));
	if (new_elema != old_elema) {
		DREF DeeObject **new_elemv;

		/* Make sure that the new allocation isn't too low */
		if unlikely(new_elema < DeeList_SIZE(me)) {
			size_t my_size = DeeList_SIZE(me);
			DeeList_LockEndWrite(me);
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot lower list allocation to %" PRFuSIZ " when size is %" PRFuSIZ,
			                new_elema, my_size);
			goto err;
		}

		/* Release / allocate memory. */
		new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
		                                               sizeof(DREF DeeObject *));
		if unlikely(!new_elemv)
			goto done_unlock;
		me->l_list.ol_elemv  = new_elemv;
		_DeeList_SetAlloc(me, new_elema);
	}
	ASSERT(DeeList_GetAlloc(me) >= DeeList_SIZE(me));
done_unlock:
	DeeList_LockEndWrite(me);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_pushfront(List *me, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &item))
		goto err;
	if (DeeList_Insert((DeeObject *)me, 0, item))
		goto err;
	return_none;
err:
	return NULL;
}
#define list_pushback list_append
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_popfront(List *me, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return DeeList_Pop((DeeObject *)me, 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_popback(List *me, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	return DeeList_Pop((DeeObject *)me, -1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_reserve(List *me, size_t argc, DeeObject *const *argv) {
	size_t old_elema;
	size_t new_elema;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":reserve", &new_elema))
		goto err;
	DeeList_LockWrite(me);
	old_elema = DeeList_GetAlloc(me);
	if (new_elema > old_elema) {
		/* Try to allocate more memory for this List. */
		DREF DeeObject **new_elemv;
		new_elemv = (DREF DeeObject **)Dee_TryReallocc(me->l_list.ol_elemv, new_elema,
		                                               sizeof(DREF DeeObject *));
		if likely(new_elemv) {
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
		}
	}
	DeeList_LockEndWrite(me);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_shrink(List *me, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":shrink"))
		goto err;
	list_do_shrink(me);
	return_none;
err:
	return NULL;
}

/* Reverse the order of the elements of `self' */
PUBLIC NONNULL((1)) void DCALL
DeeList_Reverse(DeeObject *__restrict self) {
	List *me = (List *)self;
	DeeObject **lo, **hi;
	DeeList_LockWrite(me);
	lo = DeeList_ELEM(me);
	hi = lo + DeeList_SIZE(me);
	while (lo < hi) {
		DeeObject *temp;
		temp  = *lo;
		*lo++ = *--hi;
		*hi   = temp;
	}
	DeeList_LockEndWrite(me);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_reverse(List *me, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":reverse"))
		goto err;
	DeeList_Reverse((DeeObject *)me);
	return_none;
err:
	return NULL;
}



/* Sort the given list ascendingly, or according to `key' */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeList_Sort(DeeObject *self, DeeObject *key) {
	List *me = (List *)self;
	DeeObject **oldv, **newv;
	size_t oldc, objc;
	objc = DeeList_SIZE_ATOMIC(me);
	oldv = (DeeObject **)Dee_Mallocc(objc, sizeof(DeeObject *));
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(me);
	if unlikely(me->l_list.ol_elemc > objc) {
		DeeObject **new_objv;
		objc = me->l_list.ol_elemc;
		DeeList_LockEndRead(me);
		new_objv = (DeeObject **)Dee_Reallocc(oldv, objc, sizeof(DeeObject *));
		if unlikely(!new_objv)
			goto err_oldv;
		oldv = new_objv;
		goto again;
	}

	/* Read all the old elements from the list. */
	Dee_Movrefv(oldv, me->l_list.ol_elemv, objc);
	DeeList_LockEndRead(me);

	/* Allocate the new list */
	newv = (DeeObject **)Dee_Mallocc(objc, sizeof(DeeObject *));
	if unlikely(!newv)
		goto err_oldv_elem;
	/* Do the actual sorting. */
	if unlikely(DeeSeq_MergeSort(newv, oldv, objc, key))
		goto err_newv;
	Dee_Free(oldv);
	DeeList_LockWrite(me);
	oldv = me->l_list.ol_elemv;
	oldc = me->l_list.ol_elemc;
	me->l_list.ol_elemc = objc;
	me->l_list.ol_elemv = newv;
	_DeeList_SetAlloc(me, objc);
	DeeList_LockEndWrite(me);
	Dee_Decrefv(oldv, oldc);
	Dee_Free(oldv);
	return 0;
err_newv:
	Dee_Free(newv);
err_oldv_elem:
	Dee_Decrefv(oldv, objc);
err_oldv:
	Dee_Free(oldv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_Sorted(DeeObject *self, DeeObject *key) {
	List *me = (List *)self;
	DeeObject **oldv;
	size_t objc;
	DeeTupleObject *result;
	objc = DeeList_SIZE_ATOMIC(me);
	oldv = (DeeObject **)Dee_Mallocc(objc, sizeof(DeeObject *));
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(me);
	if unlikely(DeeList_SIZE(me) > objc) {
		DeeObject **new_objv;
		objc = DeeList_SIZE(me);
		DeeList_LockEndRead(me);
		new_objv = (DeeObject **)Dee_Reallocc(oldv, objc, sizeof(DeeObject *));
		if unlikely(!new_objv)
			goto err_oldv;
		oldv = new_objv;
		goto again;
	}

	/* Read all the old elements from the list. */
	Dee_Movrefv(oldv, DeeList_ELEM(me), objc);
	DeeList_LockEndRead(me);

	/* Allocate the new list */
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto err_oldv_elem;

	/* Do the actual sorting. */
	if (DeeSeq_MergeSort(DeeTuple_ELEM(result), oldv, objc, key))
		goto err_result;
	Dee_Free(oldv);
	return (DeeObject *)result;
err_result:
	DeeTuple_FreeUninitialized(result);
err_oldv_elem:
	Dee_Decrefv(oldv, objc);
err_oldv:
	Dee_Free(oldv);
err:
	return NULL;
}


INTDEF struct keyword seq_sort_kwlist[];
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sort(List *me, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:sort", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	if (DeeList_Sort((DeeObject *)me, key))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sorted(List *me, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:sorted", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	return DeeList_Sorted((DeeObject *)me, key);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_resize(List *me, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t newsize;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_resize_kwlist,
	                    UNPuSIZ "|o:resize",
	                    &newsize, &filler))
		goto err;
	if unlikely(DeeList_Resize((DeeObject *)me, newsize, filler))
		goto err;
	return_none;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst list_getsets[] = {
	TYPE_GETSET_F("allocated", &list_getallocated, &list_delallocated, &list_setallocated, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{Attmpted to set the List preallocation size to a value lower than ${##this}}"
	              "The number of allocated items\n"
	              "When using performing a del-operation on this property, the allocation will "
	              /**/ "be set to use the least amount of memory, which is achived by setting it to ${##this}.\n"
	              "Note however that when lowering the amount of allocated vector space, failure to "
	              /**/ "reallocate the internal List vector is ignored, and the allocated List size will "
	              /**/ "not be modified\n"
	              "Similarly, failure to allocate more memory when increasing the allocated size "
	              /**/ "of a List is ignored, with the previously allocated size remaining unchanged.\n"
	              "${"
	              /**/ "del mylist.allocated;\n"
	              /**/ "/* Same as this: */\n"
	              /**/ "mylist.shrink();\n"
	              /**/ "/* And same as an atomic variant of: */\n"
	              /**/ "mylist.allocated = ##mylist;"
	              "}"),
	TYPE_GETSET_F(STR_first, &list_get_first, &list_del_first, &list_set_first, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#r{The first item from @this List}"),
	TYPE_GETSET_F(STR_last, &list_get_last, &list_del_last, &list_set_last, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#r{The last item from @this List}"),
	TYPE_GETTER_F(STR_frozen, &list_get_frozen, METHOD_FNOREFESCAPE,
	              "->?DTuple\n"
	              "Return a copy of the contents of @this List as an immutable sequence"),
	TYPE_GETTER_F("__sizeof__", &list_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst list_methods[] = {
	TYPE_METHOD_F(STR_append, &list_append, METHOD_FNOREFESCAPE,
	              "(items!)\n"
	              "Append all the given @items at the end of @this List"),
	TYPE_METHOD_F(STR_extend, &list_extend, METHOD_FNOREFESCAPE,
	              "(items:?S?O)\n"
	              "Append all elements from @items to the end of @this List"),
	TYPE_KWMETHOD_F(STR_resize, &list_resize, METHOD_FNOREFESCAPE,
	                "(newsize:?Dint,filler=!N)\n"
	                "#tIntegerOverflow{The given @newsize is lower than $0}"
	                "Resize the size of @this List to match @newsize, using @filler to initialize new items"),
	TYPE_KWMETHOD_F(STR_insert, &list_insert, METHOD_FNOREFESCAPE,
	                "(index:?Dint,item)\n"
	                "#tIntegerOverflow{The given @index is too large}"
	                "Insert the given @item at the specified @index\n"
	                "When @index is lower than $0, or greater thatn ${##this}, append items at the end"),
	TYPE_KWMETHOD_F(STR_insertall, &list_insertall, METHOD_FNOREFESCAPE,
	                "(index:?Dint,seq:?S?O)\n"
	                "#tIntegerOverflow{The given @index is too large}"
	                "Insert all items from a sequence @seq at the specified @index\n"
	                "When @index is lower than $0, or greater thatn ${##this}, append items at the end"),
	TYPE_KWMETHOD_F(STR_erase, &list_erase, METHOD_FNOREFESCAPE,
	                "(index:?Dint,count=!1)->?Dint\n"
	                "#tIntegerOverflow{The given @index or @count are lower than $0, or too large}"
	                "#r{The number of erased items (Zero when @index is out of bounds)}"
	                "Erase up to @count items starting at the given @index"),
	TYPE_KWMETHOD_F(STR_pop, &list_pop, METHOD_FNOREFESCAPE,
	                "(index=!-1)->\n"
	                "#tIndexError{The given @index is out of bounds}"
	                "#r{Items that was removed}"
	                "Pops an item at the given @{index}. When @index is lower "
	                "than $0, add ${##this} prior to evaluation, meaning that "
	                "negative numbers pop items relative to the end of the List"),
	TYPE_KWMETHOD_F(STR_xch, &list_xch, METHOD_FNOREFESCAPE,
	                "(index:?Dint,value)->\n"
	                "#tIndexError{The given @index is out of bounds}"
	                "Exchange the @index'th item of @this List with @value, returning the item's old value"),
	TYPE_METHOD_F(STR_clear, &list_clear, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear all items from @this List"),
	TYPE_KWMETHOD_F(STR_removeif, &list_removeif, METHOD_FNOREFESCAPE,
	                "(should:?DCallable,start=!0,end=!-1)->?Dint\n"
	                "#r{The number of removed items}"
	                "Remove all elements within the given sub-range for which ${!!should(elem)} is true"),
	TYPE_METHOD_F(STR_pushfront, &list_pushfront, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Inserts @item at the fron of this @this List. Same as ${this.insert(0,item)}"),
	TYPE_METHOD_F(STR_pushback, &list_pushback, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Inserts @item at the end of this @this List. Same as ?#append"),
	TYPE_METHOD_F(STR_popfront, &list_popfront, METHOD_FNOREFESCAPE,
	              "->\n"
	              "Same as ${this.pop(0)}"),
	TYPE_METHOD_F(STR_popback, &list_popback, METHOD_FNOREFESCAPE,
	              "->\n"
	              "Same as ${this.pop(-1)}"),

	/* List ordering functions. */
	TYPE_METHOD_F("reverse", &list_reverse, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Reverse the order of all the elements of @this List"),
	TYPE_KWMETHOD_F("sort", &list_sort, METHOD_FNOREFESCAPE,
	                "()\n"
	                "(key:?DCallable)\n"
	                "Sort the elements of @this List in ascending order, or in accordance to @key"),
	TYPE_KWMETHOD_F("sorted", &list_sorted, METHOD_FNOREFESCAPE,
	                "->?S?O\n"
	                "(key:?DCallable)->?S?O\n"
	                "Return a sequence that contains all elements from @this sequence, "
	                /**/ "but sorted in ascending order, or in accordance to @key\n"
	                "The type of sequence returned is implementation-defined"),

	/* List buffer functions. */
	TYPE_METHOD_F("reserve", &list_reserve, METHOD_FNOREFESCAPE,
	              "(size:?Dint)\n"
	              "Reserve (preallocate) memory for @size items\n"
	              "Failures to pre-allocate memory are silently ignored, in which case ?#allocated will remain unchanged\n"
	              "If @size is lower than the currently ?#allocated size, the function becomes a no-op"),
	TYPE_METHOD_F("shrink", &list_shrink, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Release any pre-allocated, but unused memory, setting ?#allocated to the length of @this List"),

	/* Deprecated aliases / functions. */
	TYPE_KWMETHOD_F("remove_if", &list_removeif, METHOD_FNOREFESCAPE,
	                "(should:?DCallable,start=!0,end=!-1)->?Dint\n"
	                "Deprecated alias for ?#removeif"),
	TYPE_KWMETHOD_F("insert_list", &list_insertall, METHOD_FNOREFESCAPE,
	                "(index:?Dint,items:?S?O)\n"
	                "Deprecated alias for ?#insertall"),
	TYPE_METHOD_F("insert_iter", &list_insertiter_deprecated, METHOD_FNOREFESCAPE,
	              "(index:?Dint,iter:?DIterator)\n"
	              "Deprecated alias for ${this.insertall(index, (iter as iterator from deemon).future)}"),
	TYPE_METHOD_F("push_front", &list_pushfront, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#pushfront"),
	TYPE_METHOD_F("push_back", &list_pushback, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#pushback"),
	TYPE_METHOD_F("pop_front", &list_popfront, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#popfront"),
	TYPE_METHOD_F("pop_back", &list_popback, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#popback"),
	TYPE_METHOD_F("shrink_to_fit", &list_shrink, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Deprecated alias for ?#shrink"),
	/* TODO: DEE_METHODDEF_v100("sorted_insert", member(&_deelist_sorted_insert), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_v100("fill", member(&_deelist_fill), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_v100("unique", member(&_deelist_unique), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_CONST_v100("tounique", member(&_deelist_tounique), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_CONST_v100("extend_unique", member(&_deelist_extend_unique), DEE_DOC_AUTO), */
	TYPE_METHOD_END
};

PRIVATE struct type_gc tpconst list_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&DeeList_Clear
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF List *DCALL
list_add(List *me, DeeObject *other) {
	DREF List *result;
	result = (DREF List *)DeeList_Copy((DeeObject *)me);
	if unlikely(!result)
		goto err;
	if unlikely(DeeList_AppendSequence((DeeObject *)result, other))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_inplace_add(List **__restrict p_self,
                 DeeObject *other) {
	return DeeList_AppendSequence((DeeObject *)*p_self, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF List *DCALL
list_mul(List *me, DeeObject *other) {
	size_t i, my_elemc, res_elemc, multiplier;
	DREF List *result;
	DREF DeeObject **res_elemv, **dst;
	if (DeeObject_AsSize(other, &multiplier))
		goto err;
again:
	DeeList_LockRead(me);
	my_elemc = me->l_list.ol_elemc;
	if (OVERFLOW_UMUL(my_elemc, multiplier, &res_elemc)) {
		DeeList_LockEndRead(me);
		err_integer_overflow_i(sizeof(size_t) * 8, true);
		goto err;
	}
	if unlikely(res_elemc == 0) {
		DeeList_LockEndRead(me);
		return (DREF List *)DeeList_New();
	}
	res_elemv = (DREF DeeObject **)Dee_TryMallocc(res_elemc, sizeof(DREF DeeObject *));
	if unlikely(!res_elemv) {
		DeeList_LockEndRead(me);
		if (Dee_CollectMemoryc(res_elemc, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
#ifndef __OPTIMIZE_SIZE__
	if (my_elemc == 1) {
		DeeObject *obj = DeeList_GET(me, 0);
		Dee_Setrefv(res_elemv, obj, multiplier);
	} else
#endif /* !__OPTIMIZE_SIZE__ */
	{
		for (i = 0; i < my_elemc; ++i) {
			DeeObject *obj;
			obj = DeeList_GET(me, i);
			Dee_Incref_n(obj, multiplier);
		}
		for (dst = res_elemv, i = 0; i < multiplier; ++i) {
			memcpyc(dst, DeeList_ELEM(me),
					my_elemc, sizeof(DREF DeeObject *));
			dst += my_elemc;
		}
	}
	DeeList_LockEndRead(me);
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err_elem;
	result->l_list.ol_elemv = res_elemv;
	result->l_list.ol_elemc = res_elemc;
	_DeeList_SetAlloc(result, res_elemc);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	DeeObject_Init(result, &DeeList_Type);
	return (DREF List *)DeeGC_Track((DeeObject *)result);
err_elem:
	Dee_Decrefv(res_elemv, res_elemc);
	Dee_Free(res_elemv);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_inplace_mul(List **__restrict p_self,
                 DeeObject *other) {
	List *me = *p_self;
	DREF DeeObject **elemv, **dst;
	size_t i, my_length, result_length, multiplier;
	if (DeeObject_AsSize(other, &multiplier))
		goto err;
	if unlikely(multiplier == 1)
		goto done;
again:
	DeeList_LockWrite(me);
	my_length = me->l_list.ol_elemc;
	if (OVERFLOW_UMUL(my_length, multiplier, &result_length)) {
		DeeList_LockEndWrite(me);
		err_integer_overflow_i(sizeof(size_t) * 8, true);
		goto err;
	}
	elemv = me->l_list.ol_elemv;
	if unlikely(result_length <= 0) {
		/* Special case: list is being cleared. */
		size_t elemc;
		elemc = me->l_list.ol_elemc;
		elemv = me->l_list.ol_elemv;
		Dee_objectlist_init(&me->l_list);
		DeeList_LockEndWrite(me);
		Dee_Decrefv(elemv, elemc);
		Dee_Free(elemv);
		goto done;
	}

	/* Make sure sufficient memory has been allocated. */
	if (result_length > DeeList_GetAlloc(me)) {
		elemv = (DREF DeeObject **)Dee_TryReallocc(elemv, result_length,
		                                           sizeof(DREF DeeObject *));
		if unlikely(!elemv) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(result_length, sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}
		me->l_list.ol_elemv = elemv;
		_DeeList_SetAlloc(me, result_length);
	}

	/* Create new references. */
	--multiplier;
	for (i = 0; i < my_length; ++i)
		Dee_Incref_n(elemv[i], multiplier);

	/* Copy objects to fill the new vector area. */
	dst = elemv + my_length;
	while (multiplier--) {
		dst = (DREF DeeObject **)mempcpyc(dst, elemv, my_length,
		                                  sizeof(DREF DeeObject *));
	}
	me->l_list.ol_elemc = result_length;
	DeeList_LockEndWrite(me);
done:
	return 0;
err:
	return -1;
}


PRIVATE struct type_math list_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ NULL,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_add,
	/* .tp_sub         = */ NULL,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_mul,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ NULL,
	/* .tp_or          = */ NULL,
	/* .tp_xor         = */ NULL,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&list_inplace_add,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&list_inplace_mul,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL,
};

PRIVATE struct type_member tpconst list_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeListIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeTuple_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
list_compare_eq_v(List *lhs, DeeObject *const *rhsv, size_t elemc) {
	size_t i;
	int temp;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		DREF DeeObject *lhs_elem;
		if (DeeList_SIZE(lhs) != elemc)
			goto nope;
		if (i >= elemc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		temp = DeeObject_TryCompareEq(lhs_elem, rhsv[i]);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 0;
nope:
	DeeList_LockEndRead(lhs);
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_compare_v(List *lhs, DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		int diff;
		DREF DeeObject *lhs_elem;
		if (i >= DeeList_SIZE(lhs)) {
			size_t lhsc = DeeList_SIZE(lhs);
			DeeList_LockEndRead(lhs);
			return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
		}
		if (i >= rhsc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		diff = DeeObject_Compare(lhs_elem, rhsv[i]);
		Dee_Decref(lhs_elem);
		if (diff != 0)
			return diff;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 1; /* COUNT(lhs) > COUNT(rhs) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_compare_eq(List *lhs, DeeObject *rhs) {
	if (DeeTuple_Check(rhs))
		return list_compare_eq_v(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	return DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndexFast(&DeeList_Type, (DeeObject *)lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_compare(List *lhs, DeeObject *rhs) {
	if (DeeTuple_Check(rhs))
		return list_compare_v(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	return DeeSeq_TDefaultCompareWithSizeAndGetItemIndexFast(&DeeList_Type, (DeeObject *)lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
list_hash(List *__restrict me) {
	size_t i;
	dhash_t result;
	DREF DeeObject *elem;
	DeeList_LockRead(me);
	if unlikely(!me->l_list.ol_elemc) {
		DeeList_LockEndRead(me);
		return DEE_HASHOF_EMPTY_SEQUENCE;
	}
	elem = me->l_list.ol_elemv[0];
	Dee_Incref(elem);
	DeeList_LockEndRead(me);
	result = DeeObject_Hash(elem);
	Dee_Decref(elem);
	DeeList_LockRead(me);
	for (i = 1; i < me->l_list.ol_elemc; ++i) {
		elem = me->l_list.ol_elemv[i];
		Dee_Incref(elem);
		DeeList_LockEndRead(me);
		result = Dee_HashCombine(result, DeeObject_Hash(elem));
		Dee_Decref(elem);
		DeeList_LockRead(me);
	}
	DeeList_LockEndRead(me);
	return result;
}


PRIVATE struct type_cmp list_cmp = {
	/* .tp_hash          = */ (dhash_t (DCALL *)(DeeObject *__restrict))&list_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_compare,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
};


PRIVATE struct type_operator const list_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOREFESCAPE | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0036_DELRANGE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0037_SETRANGE, METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeList_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_List),
	/* .tp_doc      = */ DOC("An index-based vector sequence, capable of holding any number of objects\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty List\n"
	                         "\n"

	                         "(items:?S?O)\n"
	                         "Create a new List, using objects enumerated by iterating @items\n"
	                         "\n"

	                         "(size:?Dint,filler=!N)\n"
	                         "Create a new List consisting of @size elements, all initialized to @filler\n"
	                         "\n"

	                         "copy->\n"
	                         "Creates a shallow copy of @this List\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Creates a deep copy of @this List\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this List is non-empty\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the items of @this List, following List syntax rather than abstract sequence syntax:\n"
	                         "${"
	                         /**/ "local x = [];\n"
	                         /**/ "x.append(10);\n"
	                         /**/ "x.append(20);\n"
	                         /**/ "x.append(30);\n"
	                         /**/ "print repr x; /* `[10, 20, 30]' */"
	                         "}\n"
	                         "\n"

	                         "+(other:?X2?.?S?O)->\n"
	                         "Returns a new List that is the concatenation of @this List and @other\n"
	                         "\n"

	                         "+=(other:?X2?.?S?O)->\n"
	                         "Appends elements from @other to @this List. (Same as ?#extend)\n"
	                         "\n"

	                         "*(count:?Dint)->\n"
	                         "#tIntegerOverflow{@count is negative}"
	                         "Return a new List containing all the elements of @this one, repeated @count times\n"
	                         "When @count is equal to $0, an empty List is returned\n"
	                         "\n"

	                         "*=(count:?Dint)->\n"
	                         "#tIntegerOverflow{@count is negative}"
	                         "Extend @this List to contain @count as many items, filling the new slots with repetitions of all pre-existing items\n"
	                         "When @count is equal to $0, the operator behaves the same as ?#clear\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of items contained within @this List\n"
	                         "\n"

	                         "[](index:?Dint)->\n"
	                         "#tIndexError{@index is out of bounds}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Return the @index'th item of @this List\n"
	                         "\n"

	                         "del[](index:?Dint)->\n"
	                         "#tIndexError{@index is out of bounds}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Delete the @index'th item from @this List. (same as ${this.erase(index, 1)})\n"
	                         "\n"

	                         "[]=(index:?Dint,ob)->\n"
	                         "#tIndexError{@index is out of bounds}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Replace the @index'th item of @this List with @ob\n"
	                         "\n"

	                         "[:](start:?Dint,end:?Dint)->\n"
	                         "#tIntegerOverflow{@start or @end are too large}"
	                         "Return a new List containing the elements within the range @{start}...@end\n"
	                         "If either @start or @end are negative, ${##this} is added first.\n"
	                         "If following this, either is greater than ${##this}, it is clampled to that value\n"
	                         "\n"

	                         "del[:](start:?Dint,end:?Dint)->\n"
	                         "#tIntegerOverflow{@start or @end are too large}"
	                         "Using the same index-rules as for ?#{op:getrange}, delete all items from that range\n"
	                         "\n"

	                         "[:]=(start:?Dint,end:?Dint,items:?S?O)->\n"
	                         "#tNotImplemented{The given @items cannot be iterated}"
	                         "#tIntegerOverflow{@start or @end are too large}"
	                         "Using the same index-rules as for ?#{op:getrange}, delete all items from that range "
	                         /**/ "before inserting all elements from @items at the range's start. - This operation "
	                         /**/ "is performed atomically, and @this List is not modified if @items cannot be iterated\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this List\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @elem is apart of @this List\n"
	                         "\n"

	                         "<(other:?X2?.?S?O)->\n"
	                         "<=(other:?X2?.?S?O)->\n"
	                         "==(other:?X2?.?S?O)->\n"
	                         "!=(other:?X2?.?S?O)->\n"
	                         ">(other:?X2?.?S?O)->\n"
	                         ">=(other:?X2?.?S?O)->\n"
	                         "#tNotImplemented{The given @other cannot be iterated}"
	                         "Perform a lexicographical comparison between @this List and the given @other sequence"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(List),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&list_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&list_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&list_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&list_init,
				TYPE_FIXED_ALLOCATOR_GC(List)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&list_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&list_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&list_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ &list_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&list_visit,
	/* .tp_gc            = */ &list_gc,
	/* .tp_math          = */ &list_math,
	/* .tp_cmp           = */ &list_cmp,
	/* .tp_seq           = */ &list_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ list_methods,
	/* .tp_getsets       = */ list_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ list_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ list_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(list_operators)
};

#define LI_GETINDEX(x) atomic_read(&(x)->li_index)

PRIVATE WUNUSED NONNULL((1)) int DCALL
li_ctor(ListIterator *__restrict self) {
	self->li_list = (List *)DeeList_New();
	if unlikely(!self->li_list)
		goto err;
	self->li_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
li_copy(ListIterator *__restrict self,
        ListIterator *__restrict other) {
	self->li_list  = other->li_list;
	self->li_index = LI_GETINDEX(other);
	Dee_Incref(self->li_list);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
li_deep(ListIterator *__restrict self,
        ListIterator *__restrict other) {
	self->li_list = (DREF List *)DeeObject_DeepCopy((DeeObject *)other->li_list);
	if unlikely(!self->li_list)
		goto err;
	self->li_index = LI_GETINDEX(other);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
li_init(ListIterator *__restrict self,
        size_t argc, DeeObject *const *argv) {
	self->li_index = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":_ListIterator", &self->li_list, &self->li_index))
		goto err;
	if (DeeObject_AssertType(self->li_list, &DeeList_Type))
		goto err;
	Dee_Incref(self->li_list);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
li_dtor(ListIterator *__restrict self) {
	Dee_Decref(self->li_list);
}

PRIVATE NONNULL((1, 2)) void DCALL
li_visit(ListIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->li_list);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
li_bool(ListIterator *__restrict self) {
	return LI_GETINDEX(self) < DeeList_SIZE(self->li_list);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
li_next(ListIterator *__restrict self) {
	size_t list_index;
	DREF DeeObject *result;
again:
	list_index = atomic_read(&self->li_index);
	DeeList_LockRead(self->li_list);
	if (list_index >= self->li_list->l_list.ol_elemc) {
		DeeList_LockEndRead(self->li_list);
		return ITER_DONE;
	}
	result = self->li_list->l_list.ol_elemv[list_index];
	Dee_Incref(result);
	DeeList_LockEndRead(self->li_list);
	if (!atomic_cmpxch_weak_or_write(&self->li_index, list_index, list_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
}


#define DEFINE_LISTITERATOR_COMPARE(name, expr)                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL            \
	name(ListIterator *self, ListIterator *other) {                  \
		if (DeeObject_AssertTypeExact(other, &DeeListIterator_Type)) \
			goto err;                                                \
		return_bool(expr);                                           \
	err:                                                             \
		return NULL;                                                 \
	}
DEFINE_LISTITERATOR_COMPARE(li_eq,
                            self->li_list == other->li_list &&
                            LI_GETINDEX(self) == LI_GETINDEX(other))
DEFINE_LISTITERATOR_COMPARE(li_ne,
                            self->li_list != other->li_list ||
                            LI_GETINDEX(self) != LI_GETINDEX(other))
DEFINE_LISTITERATOR_COMPARE(li_lo,
                            self->li_list < other->li_list ||
                            (self->li_list == other->li_list &&
                             LI_GETINDEX(self) < LI_GETINDEX(other)))
DEFINE_LISTITERATOR_COMPARE(li_le,
                            self->li_list < other->li_list ||
                            (self->li_list == other->li_list &&
                             LI_GETINDEX(self) <= LI_GETINDEX(other)))
DEFINE_LISTITERATOR_COMPARE(li_gr,
                            self->li_list > other->li_list ||
                            (self->li_list == other->li_list &&
                             LI_GETINDEX(self) > LI_GETINDEX(other)))
DEFINE_LISTITERATOR_COMPARE(li_ge,
                            self->li_list > other->li_list ||
                            (self->li_list == other->li_list &&
                             LI_GETINDEX(self) >= LI_GETINDEX(other)))
#undef DEFINE_LISTITERATOR_COMPARE

PRIVATE WUNUSED NONNULL((1)) DREF List *DCALL
list_iterator_nii_getseq(ListIterator *__restrict self) {
	return_reference_(self->li_list);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_iterator_nii_getindex(ListIterator *__restrict self) {
	return LI_GETINDEX(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_setindex(ListIterator *__restrict self, size_t new_index) {
	atomic_write(&self->li_index, new_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_rewind(ListIterator *__restrict self) {
	atomic_write(&self->li_index, 0);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_revert(ListIterator *__restrict self, size_t step) {
	size_t old_index, new_index;
	do {
		old_index = atomic_read(&self->li_index);
		if (OVERFLOW_USUB(old_index, step, &new_index))
			new_index = 0;
	} while (!atomic_cmpxch_weak_or_write(&self->li_index, old_index, new_index));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_advance(ListIterator *__restrict self, size_t step) {
	size_t old_index, new_index;
	do {
		old_index = atomic_read(&self->li_index);
		if (OVERFLOW_UADD(old_index, step, &new_index))
			new_index = (size_t)-1;
	} while (!atomic_cmpxch_weak_or_write(&self->li_index, old_index, new_index));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_prev(ListIterator *__restrict self) {
	size_t old_index;
	do {
		old_index = atomic_read(&self->li_index);
		if (!old_index)
			return 1;
	} while (!atomic_cmpxch_weak_or_write(&self->li_index, old_index, old_index - 1));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_next(ListIterator *__restrict self) {
	size_t old_index;
	do {
		old_index = atomic_read(&self->li_index);
		if (old_index >= DeeList_SIZE(self->li_list))
			return 1;
	} while (!atomic_cmpxch_weak_or_write(&self->li_index, old_index, old_index + 1));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_hasprev(ListIterator *__restrict self) {
	return LI_GETINDEX(self) != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_iterator_nii_peek(ListIterator *__restrict self) {
	size_t list_index;
	DREF DeeObject *result;
	list_index = atomic_read(&self->li_index);
	DeeList_LockRead(self->li_list);
	if (list_index >= self->li_list->l_list.ol_elemc) {
		DeeList_LockEndRead(self->li_list);
		return ITER_DONE;
	}
	result = self->li_list->l_list.ol_elemv[list_index];
	Dee_Incref(result);
	DeeList_LockEndRead(self->li_list);
	return result;
}

PRIVATE struct type_nii tpconst list_iterator_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&list_iterator_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&list_iterator_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&list_iterator_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&list_iterator_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)&list_iterator_nii_revert,
			/* .nii_advance  = */ (dfunptr_t)&list_iterator_nii_advance,
			/* .nii_prev     = */ (dfunptr_t)&list_iterator_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&list_iterator_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&list_iterator_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&list_iterator_nii_peek
		}
	}
};


PRIVATE struct type_cmp li_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_ge,
	/* .tp_nii           = */ &list_iterator_nii
};

PRIVATE struct type_member tpconst li_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(ListIterator, li_list), "->?DList"),
	TYPE_MEMBER_FIELD(STR_index, STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(ListIterator, li_index)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeListIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ListIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&li_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&li_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&li_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&li_init,
				TYPE_FIXED_ALLOCATOR(ListIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&li_dtor,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&li_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&li_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &li_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&li_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ li_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_LIST_C */
