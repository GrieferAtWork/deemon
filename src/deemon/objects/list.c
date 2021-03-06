/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
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

#include <hybrid/atomic.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#ifndef CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

typedef DeeListObject List;

typedef struct {
	OBJECT_HEAD
	DREF List   *li_list;  /* [1..1][const] The list being iterated. */
	DWEAK size_t li_index; /* The current iteration index. */
} ListIterator;

INTDEF DeeTypeObject DeeListIterator_Type;


PRIVATE NONNULL((1)) void DCALL
list_fini(List *__restrict self) {
	DeeObject **vector;
	weakref_support_fini(self);
	vector = Dee_Decrefv(DeeList_ELEM(self),
	                     DeeList_SIZE(self));
	Dee_Free(vector);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_assign(List *self, DeeObject *other) {
	DREF DeeObject **elemv, **old_elemv;
	size_t elemc, elema, old_elemc;
	if ((List *)other == self)
		return 0;
	/* Steal the current list state to update it! */
	DeeList_LockWrite(self);
	elemv         = self->l_elem;
	elemc         = self->l_size;
	elema         = self->l_alloc;
	self->l_elem  = NULL;
	self->l_size  = 0;
	self->l_alloc = 0;
	DeeList_LockEndWrite(self);
	/* Delete all of the old list elements. */
	while (elemc--)
		Dee_Decref(elemv[elemc]);
	elemc = DeeSeq_AsHeapVectorWithAllocReuse(other,
	                                          &elemv,
	                                          &elema);
	/* Save the new list buffer. */
	DeeList_LockWrite(self);
	old_elemv     = self->l_elem;
	old_elemc     = self->l_size;
	self->l_elem  = elemv;
	self->l_size  = elemc == (size_t)-1 ? 0 : elemc;
	self->l_alloc = elema;
	DeeList_LockEndWrite(self);
	if unlikely(old_elemv) {
		/* Free the list state that got created while we loaded `other' */
		while (old_elemc--)
			Dee_Decref(old_elemv[old_elemc]);
		Dee_Free(old_elemv);
	}
	/* With the updated list state now saved, check for errors. */
	if unlikely(elemc == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_moveassign(List *self, List *other) {
	DREF DeeObject **elemv, **old_elemv;
	size_t elemc, elema, old_elemc;
	if (self == other)
		goto done;
	DeeList_LockWrite(other);
	elemv          = other->l_elem;
	elemc          = other->l_size;
	elema          = other->l_alloc;
	other->l_elem  = NULL;
	other->l_size  = 0;
	other->l_alloc = 0;
	DeeList_LockEndWrite(other);
	DeeList_LockWrite(self);
	old_elemv     = self->l_elem;
	old_elemc     = self->l_size;
	self->l_elem  = elemv;
	self->l_size  = elemc;
	self->l_alloc = elema;
	DeeList_LockEndWrite(self);
	if (old_elemc) {
		while (old_elemc--)
			Dee_Decref(old_elemv[old_elemc]);
		Dee_Free(old_elemv);
	}
done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_ctor(List *__restrict self) {
	weakref_support_init(self);
	self->l_alloc = 0;
	self->l_size  = 0;
	self->l_elem  = NULL;
	rwlock_init(&self->l_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_copy(List *__restrict self,
          List *__restrict other) {
	DREF DeeObject **newvec;
	size_t count;
	ASSERT(self != other);
	weakref_support_init(self);
	rwlock_init(&self->l_lock);
again:
	DeeList_LockRead(other);
	count = other->l_size;
	newvec = (DREF DeeObject **)Dee_TryMalloc(count * sizeof(DREF DeeObject *));
	if unlikely(!newvec) {
		DeeList_LockEndRead(other);
		if (Dee_CollectMemory(count * sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	/* Copy references */
	self->l_elem  = Dee_Movrefv(newvec, DeeList_ELEM(other), count);
	self->l_alloc = count;
	self->l_size  = count;
	DeeList_LockEndRead(other);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_deepload(List *__restrict self) {
	DREF DeeObject *temp, *item;
	size_t i = 0;
	weakref_support_init(self);
	DeeList_LockRead(self);
	for (; i < self->l_size; ++i) {
		item = self->l_elem[i];
		Dee_Incref(item);
		DeeList_LockEndRead(self);
		/* Create the deep copy. */
		temp = DeeObject_DeepCopy(item);
		Dee_Decref(item);
		if unlikely(!temp)
			goto err;
		DeeList_LockWrite(self);
		/* Must re-check that the list hasn't been shrunk in the mean time. */
		if unlikely(i >= self->l_size)
			goto stop_on_end;
		/* Write the duplicated object back into the list's vector. */
		item            = self->l_elem[i]; /* Inherit */
		self->l_elem[i] = temp;            /* Inherit */
		DeeList_LockEndWrite(self);
		/* Drop the old object. */
		Dee_Decref(item);
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
done:
	return 0;
err:
	return -1;
stop_on_end:
	DeeList_LockEndWrite(self);
	Dee_Decref(temp);
	goto done;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_init_iterator(List *__restrict self,
                   DeeObject *__restrict iterator) {
	DREF DeeObject *elem;
	size_t size, alloc, new_alloc;
	DREF DeeObject **vector, **new_vector;
	ASSERT_OBJECT(iterator);
	ASSERT((DeeObject *)self != iterator);
	size = alloc = 0;
	vector       = NULL;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (size == alloc) {
			new_alloc = alloc * 2;
			if (!new_alloc)
				new_alloc = 2;
do_realloc:
			new_vector = (DREF DeeObject **)Dee_TryRealloc(vector, new_alloc *
			                                                       sizeof(DREF DeeObject *));
			if unlikely(!new_vector) {
				if (new_alloc != size + 1) {
					new_alloc = size + 1;
					goto do_realloc;
				}
				if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
					goto do_realloc;
				Dee_Decref(elem);
				goto err;
			}
			vector = new_vector;
			alloc  = new_alloc;
		}
		/* Store all elements in the vector. */
		vector[size++] = elem;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	/* Save the allocated vector in the list.
	 * NOTE: Since lists are capable of being over-allocated, we don't truncate
	 *       our allocation but store that information in the list, too. */
	self->l_alloc = alloc;
	self->l_elem  = vector;
	self->l_size  = size;
	weakref_support_init(self);
	rwlock_init(&self->l_lock);
	return 0;
err:
	while (size--)
		Dee_Decref(vector[size]);
	Dee_Free(vector);
	return -1;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewHint(size_t n_prealloc) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeList_Type);
	result->l_alloc = n_prealloc;
	result->l_size  = 0;
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	if likely(n_prealloc) {
		result->l_elem = (DREF DeeObject **)Dee_TryMalloc(n_prealloc *
		                                                  sizeof(DREF DeeObject *));
		if unlikely(!result->l_elem)
			result->l_alloc = 0;
	} else {
		result->l_elem = NULL;
	}
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewUninitialized(size_t n_elem) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	result->l_elem = (DREF DeeObject **)Dee_Malloc(n_elem * sizeof(DREF DeeObject *));
	if unlikely(!result->l_elem)
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	result->l_alloc = n_elem;
	result->l_size  = n_elem;
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	/*DeeGC_Track((DeeObject *)result);*/ /* The caller must do this */
done:
	return (DREF DeeObject *)result;
err_r:
	DeeGCObject_FREE(result);
	return NULL;
}

PUBLIC NONNULL((1)) void DCALL
DeeList_FreeUninitialized(DeeObject *__restrict self) {
	ASSERT(!DeeObject_IsShared(self));
	ASSERT(Dee_TYPE(self) == &DeeList_Type);
	Dee_DecrefNokill(&DeeList_Type);
	Dee_Free(DeeList_ELEM(self));
	DeeObject_FreeTracker((DeeObject *)self);
	DeeGCObject_FREE(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_FromIterator(DeeObject *__restrict self) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	if unlikely(list_init_iterator(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeGCObject_FREE(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_FromSequence(DeeObject *__restrict self) {
	DREF List *result;
	if (DeeList_CheckExact(self)) {
		if (!DeeObject_IsShared(self))
			return_reference_(self);
	}
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	result->l_elem = DeeSeq_AsHeapVectorWithAlloc(self,
	                                              &result->l_size,
	                                              &result->l_alloc);
	if unlikely(!result->l_elem)
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	DeeGC_Track((DeeObject *)result);
	return (DREF DeeObject *)result;
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInherited(size_t objc, DREF DeeObject *const *objv) {
	DREF DeeObject *result;
	ASSERT(objv || !objc);
	result = DeeList_NewUninitialized(objc);
	if unlikely(!result)
		goto done;
	memcpyc(DeeList_ELEM(result), objv, objc,
	        sizeof(DREF DeeObject *));
	COMPILER_WRITE_BARRIER();
	/* Now that the list's been filled with data,
	 * we can start tracking it as a GC object. */
	DeeGC_Track(result);
done:
	return result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(size_t obja, size_t objc,
                               /*inherit(on_success)*/ DREF DeeObject **objv) {
	DREF List *result;
	ASSERT(objc <= obja);
	ASSERT(objv || (!obja && !objc));
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	result->l_elem  = objv; /* Inherit */
	result->l_alloc = obja;
	result->l_size  = objc;
	DeeObject_Init(result, &DeeList_Type);
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVector(size_t objc, DeeObject *const *objv) {
	DREF DeeObject *result;
	size_t i;
	ASSERT(objv || !objc);
	for (i = 0; i < objc; ++i)
		Dee_Incref(objv[i]);
	result = DeeList_NewVectorInherited(objc, objv);
	if unlikely(!result)
		goto err;
	return result;
err:
	for (i = 0; i < objc; ++i)
		Dee_Decref(objv[i]);
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
DeeList_Copy(DeeObject *__restrict self) {
	DREF List *result;
	ASSERT_OBJECT(self);
	ASSERT(DeeList_Check(self));
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	if unlikely(list_copy(result, (List *)self))
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeGCObject_FREE(result);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeList_Concat(/*inherit(on_success)*/ DREF DeeObject *self,
               DeeObject *sequence) {
	DREF DeeObject *result;
	size_t fast_seqlen;
	if (!DeeObject_IsShared(self)) {
		/* Simple case: can append onto the original list. */
		if unlikely(DeeList_AppendSequence(self, sequence))
			goto err;
		return self;
	}
	fast_seqlen = DeeFastSeq_GetSize(sequence);
	if (fast_seqlen != DEE_FASTSEQ_NOTFAST) {
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_ExtendInherited(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc,
                        /*inherit(on_success)*/ DREF DeeObject *const *argv) {
	DREF List *result;
	if (!DeeObject_IsShared(self)) {
		size_t req_alloc;
		result    = (DREF List *)self;
		req_alloc = result->l_size + argc;
		/* Make sure there are sufficient buffers. */
		if (req_alloc > result->l_alloc) {
			DREF DeeObject **new_vector;
			size_t new_alloc = result->l_alloc * 2;
			if (new_alloc < req_alloc)
				new_alloc = req_alloc;
do_realloc_vector:
			new_vector = (DREF DeeObject **)Dee_TryRealloc(result->l_elem, new_alloc *
			                                                               sizeof(DREF DeeObject *));
			if unlikely(!new_vector) {
				if (new_alloc != req_alloc) {
					new_alloc = req_alloc;
					goto do_realloc_vector;
				}
				if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
					goto do_realloc_vector;
				goto err;
			}
			result->l_elem  = new_vector;
			result->l_alloc = new_alloc;
		}
		memcpyc(result->l_elem + result->l_size, argv,
		        argc, sizeof(DREF DeeObject *));
		result->l_size += argc;
	} else {
		DREF DeeObject **new_vector;
		size_t list_size, i;
		list_size = DeeList_SIZE(self);
allocate_new_vector:
		new_vector = (DREF DeeObject **)Dee_Malloc((list_size + argc) *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!new_vector)
			goto err;
		DeeList_LockRead(self);
		if unlikely(DeeList_SIZE(self) != list_size) {
			list_size = DeeList_SIZE(self);
			DeeList_LockEndRead(self);
			Dee_Free(new_vector);
			goto allocate_new_vector;
		}
		for (i = 0; i < list_size; ++i) {
			new_vector[i] = DeeList_GET(self, i);
			Dee_Incref(new_vector[i]);
		}
		DeeList_LockEndRead(self);
		/* Create the new list descriptor. */
		result = DeeGCObject_MALLOC(List);
		if unlikely(!result) {
			while (i--)
				Dee_Decref(new_vector[i]);
			Dee_Free(new_vector);
			goto err;
		}
		memcpyc(new_vector + list_size, argv, argc,
		        sizeof(DREF DeeObject *));
		DeeObject_Init(result, &DeeList_Type);
		result->l_alloc = result->l_size = list_size + argc;
		result->l_elem                   = new_vector;
		weakref_support_init(result);
		rwlock_init(&result->l_lock);
		DeeGC_Track((DeeObject *)result);
	}
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_Pop(DeeObject *__restrict self, dssize_t index) {
	DREF DeeObject *delob;
	size_t length;
	ASSERT_OBJECT(self);
	ASSERT(DeeList_Check(self));
	DeeList_LockWrite(self);
	length = DeeList_SIZE(self);
	if unlikely(index < 0)
		index += length;
	if unlikely((size_t)index >= length) {
		DeeList_LockEndWrite(self);
		err_index_out_of_bounds(self,
		                        (size_t)index,
		                        length);
		return NULL;
	}
	delob = DeeList_GET(self, (size_t)index);
	DeeList_SIZE(self) = --length;
	/* Adjust to shift following elements downwards. */
	memmovedownc(DeeList_ELEM(self) + (size_t)index,
	             DeeList_ELEM(self) + (size_t)index + 1,
	             length - (size_t)index,
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(self);
	return delob;
}

PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeList_Erase(DeeObject *__restrict self,
              size_t index, size_t count) {
	DREF DeeObject **delobv;
	size_t i;
	size_t delete_count;
	ASSERT_OBJECT(self);
	ASSERT(DeeList_Check(self));
again:
	DeeList_LockWrite(self);
	if unlikely(index >= DeeList_SIZE(self)) {
		DeeList_LockEndWrite(self);
		return 0;
	}
	delete_count = count;
	if (index + count > DeeList_SIZE(self))
		delete_count = DeeList_SIZE(self) - index;
	delobv = (DREF DeeObject **)Dee_ATryMalloc(delete_count * sizeof(DREF DeeObject *));
	if unlikely(!delobv) {
		DeeList_LockEndWrite(self);
		if (!Dee_CollectMemory(delete_count * sizeof(DREF DeeObject *)))
			return (size_t)-1;
		goto again;
	}
	/* Adjust to shift following elements downwards. */
	memcpyc(delobv, DeeList_ELEM(self) + index, count,
	        sizeof(DREF DeeObject *));
	memmovedownc(DeeList_ELEM(self) + index,
	             DeeList_ELEM(self) + index + count,
	             ((DeeList_SIZE(self) -= count) - index),
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(self);
	for (i = 0; i < count; ++i)
		Dee_Decref(delobv[i]);
	Dee_AFree(delobv);
	ASSERT(count != (size_t)-1);
	return count;
}

/* @return: 0 : The given `keyed_search_item' count not found found.
 * @return: 1 : The given `keyed_search_item' was deleted once.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
DeeList_Remove(DeeObject *self, size_t start, size_t end,
               DeeObject *keyed_search_item, DeeObject *key) {
	DeeObject **vector;
	size_t i, length;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	ASSERT_OBJECT_OPT(key);
	ASSERT_OBJECT(keyed_search_item);
	DeeList_LockRead(self);
again:
	vector = DeeList_ELEM(self);
	length = DeeList_SIZE(self);
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(self, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(self);
		temp = DeeObject_CompareKeyEq(keyed_search_item, this_elem, key);
		Dee_Decref(this_elem);
		if unlikely(temp < 0)
			return temp;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(self);
			/* Check if the list was changed. */
			if (DeeList_ELEM(self) != vector ||
			    DeeList_SIZE(self) != length ||
			    DeeList_GET(self, i) != this_elem) {
				DeeList_LockDowngrade(self);
				goto again;
			}
			/* Override the element with its successors. */
			DeeList_SIZE(self) = --length;
			memmovedownc(DeeList_ELEM(self) + i,
			             DeeList_ELEM(self) + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			DeeList_LockEndWrite(self);
			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
			return 1;
		}
		/* Continue onwards. */
		DeeList_LockRead(self);
		/* Check if the list was changed. */
		if (DeeList_ELEM(self) != vector ||
		    DeeList_SIZE(self) != length)
			goto again;
	}
	DeeList_LockEndRead(self);
	return 0;
}

/* @return: 0 : The given `keyed_search_item' count not found found.
 * @return: 1 : The given `keyed_search_item' was deleted once.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
DeeList_RRemove(DeeObject *self, size_t start, size_t end,
                DeeObject *keyed_search_item, DeeObject *key) {
	DeeObject **vector;
	size_t i, length;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	ASSERT_OBJECT_OPT(key);
	ASSERT_OBJECT(keyed_search_item);
	DeeList_LockRead(self);
again:
	vector = DeeList_ELEM(self);
	length = DeeList_SIZE(self);
	i      = end;
	if (i > length)
		i = length;
	for (;;) {
		DREF DeeObject *this_elem;
		int temp;
		if (i <= start)
			break;
		--i;
		this_elem = DeeList_GET(self, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(self);
		temp = DeeObject_CompareKeyEq(keyed_search_item, this_elem, key);
		Dee_Decref(this_elem);
		if unlikely(temp < 0)
			return temp;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(self);
			/* Check if the list was changed. */
			if (DeeList_ELEM(self) != vector ||
			    DeeList_SIZE(self) != length ||
			    DeeList_GET(self, i) != this_elem) {
				DeeList_LockDowngrade(self);
				goto again;
			}
			/* Override the element with its successors. */
			DeeList_SIZE(self) = --length;
			memmovedownc(DeeList_ELEM(self) + i,
			             DeeList_ELEM(self) + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			DeeList_LockEndWrite(self);
			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
			return 1;
		}
		/* Continue onwards. */
		DeeList_LockRead(self);
		/* Check if the list was changed. */
		if (DeeList_ELEM(self) != vector ||
		    DeeList_SIZE(self) != length)
			goto again;
	}
	DeeList_LockEndRead(self);
	return 0;
}


/* Remove all items matching `!!should(item)'
 * @return: * : The number of removed items.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
DeeList_RemoveIf(List *self, size_t start,
                 size_t end, DeeObject *should) {
	DeeObject **vector;
	size_t i, length, result;
	DeeList_LockRead(self);
again:
	result = 0;
	vector = DeeList_ELEM(self);
	length = DeeList_SIZE(self);
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *callback_result;
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(self, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(self);
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
			DeeList_LockWrite(self);
			/* Check if the list was changed. */
			if (DeeList_ELEM(self) != vector ||
			    DeeList_SIZE(self) != length ||
			    DeeList_GET(self, i) != this_elem) {
				DeeList_LockDowngrade(self);
				goto again;
			}
			/* Override the element with its successors. */
			DeeList_SIZE(self) = --length;
			memmovedownc(DeeList_ELEM(self) + i,
			             DeeList_ELEM(self) + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			++result;
			DeeList_LockEndWrite(self);
			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
		}
		/* Continue onwards. */
		DeeList_LockRead(self);
		/* Check if the list was changed. */
		if (DeeList_ELEM(self) != vector ||
		    DeeList_SIZE(self) != length)
			goto again;
	}
	DeeList_LockEndRead(self);
	ASSERT(result != (size_t)-1);
	return result;
err:
	return (size_t)-1;
}

/* Remove all items matching `!!should(item)'
 * @return: * : The number of removed items.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
DeeList_RemoveAll(List *self, size_t start, size_t end,
                  DeeObject *keyed_search_item, DeeObject *key) {
	DeeObject **vector;
	size_t i, length, result;
	DeeList_LockRead(self);
again:
	result = 0;
	vector = DeeList_ELEM(self);
	length = DeeList_SIZE(self);
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(self, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(self);
		/* Invoke a predicate. */
		temp = DeeObject_CompareKeyEq(keyed_search_item, this_elem, key);
		if unlikely(temp < 0)
			goto err;
		if (temp) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(self);
			/* Check if the list was changed. */
			if (DeeList_ELEM(self) != vector ||
			    DeeList_SIZE(self) != length ||
			    DeeList_GET(self, i) != this_elem) {
				DeeList_LockDowngrade(self);
				goto again;
			}
			/* Override the element with its successors. */
			DeeList_SIZE(self) = --length;
			memmovedownc(DeeList_ELEM(self) + i,
			             DeeList_ELEM(self) + i + 1,
			             length - i,
			             sizeof(DREF DeeObject *));
			++result;
			DeeList_LockEndWrite(self);
			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
		}
		/* Continue onwards. */
		DeeList_LockRead(self);
		/* Check if the list was changed. */
		if (DeeList_ELEM(self) != vector ||
		    DeeList_SIZE(self) != length)
			goto again;
	}
	DeeList_LockEndRead(self);
	ASSERT(result != (size_t)-1);
	return result;
err:
	return (size_t)-1;
}

/* Append objects to a given list. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeList_Append)(DeeObject *self, DeeObject *elem) {
	int result = 0;
	DeeObject **newvec;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(elem);
	ASSERT(DeeList_Check(self));
retry:
	DeeList_LockWrite(self);
	ASSERT(DeeList_CAPACITY(self) >= DeeList_SIZE(self));
	if (DeeList_CAPACITY(self) == DeeList_SIZE(self)) {
		size_t newcap = DeeList_CAPACITY(self);
		if (!newcap)
			newcap = 1;
		newcap *= 2; /* Must increase the list's capacity. */
		newvec = (DeeObject **)Dee_TryRealloc(DeeList_ELEM(self),
		                                      newcap * sizeof(DeeObject *));
		if unlikely(!newvec) {
			/* Try again, but only attempt to allocate for a single object. */
			newcap = DeeList_SIZE(self) + 1;
			newvec = (DeeObject **)Dee_TryRealloc(DeeList_ELEM(self),
			                                      newcap * sizeof(DeeObject *));
			if unlikely(!newvec) {
				DeeList_LockEndWrite(self);
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(sizeof(DeeObject *)))
					goto retry;
				goto err;
			}
		}
		DeeList_CAPACITY(self) = newcap;
		DeeList_ELEM(self)     = newvec;
	}
	DeeList_ELEM(self)[DeeList_SIZE(self)++] = elem;
	Dee_Incref(elem);
	DeeList_LockEndWrite(self);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeList_AppendVector)(DeeObject *self, size_t objc,
                             DeeObject *const *objv) {
	int result = 0;
	DeeObject **newvec;
	DeeListObject *me;
	size_t oldlen;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	me = (DeeListObject *)self;
retry:
	DeeList_LockWrite(me);
	oldlen = me->l_size;
	ASSERT(me->l_alloc >= oldlen);
	if (me->l_alloc < oldlen + objc) {
		size_t newcap = me->l_alloc;
		if (!newcap)
			newcap = 1; /* Must increase the list's capacity. */
		while (newcap < oldlen + objc)
			newcap *= 2;
		newvec = (DeeObject **)Dee_TryRealloc(me->l_elem,
		                                      newcap * sizeof(DeeObject *));
		if unlikely(!newvec) {
			/* Try again, but only attempt to allocate what we need. */
			newcap = oldlen + objc;
			newvec = (DeeObject **)Dee_TryRealloc(me->l_elem,
			                                      newcap * sizeof(DeeObject *));
			if unlikely(!newvec) {
				DeeList_LockEndWrite(me);
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(objc * sizeof(DeeObject *)))
					goto retry;
				goto err;
			}
		}
		me->l_alloc = newcap;
		me->l_elem  = newvec;
	}
	/* Copy references */
	Dee_Movrefv(me->l_elem + oldlen, objv, objc);
	me->l_size = oldlen + objc;
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
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(iterator);
	ASSERT(DeeList_Check(self));
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		error = DeeList_Append(self, elem);
		Dee_Decref(elem);
		if (error)
			return error;
		if (DeeThread_CheckInterrupt())
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
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(sequence);
	ASSERT(DeeList_Check(self));
	if (DeeTuple_CheckExact(sequence))
		return DeeList_AppendVector(self, DeeTuple_SIZE(sequence), DeeTuple_ELEM(sequence));
	fast_seqlen = DeeFastSeq_GetSize(sequence);
	if (fast_seqlen != DEE_FASTSEQ_NOTFAST) {
		/* TODO: Special optimization. */
	}
	/* Fallback: Append elements from a general-purpose iterator. */
	sequence = DeeObject_IterSelf(sequence);
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
	List *me = (List *)self;
	ASSERT_OBJECT(elem);
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	DeeList_LockWrite(me);
	if (index > me->l_size)
		index = me->l_size;
	ASSERT(me->l_size <= me->l_alloc);
	if (me->l_size == me->l_alloc) {
		size_t new_alloc = me->l_alloc * 2;
		DREF DeeObject **new_vector;
		if (!new_alloc)
			new_alloc = 2;
do_realloc:
		new_vector = (DREF DeeObject **)Dee_TryRealloc(me->l_elem,
		                                               new_alloc *
		                                               sizeof(DREF DeeObject *));
		if unlikely(!new_vector) {
			if (new_alloc != me->l_size + 1) {
				new_alloc = me->l_size + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
				goto do_realloc;
			DeeList_LockEndWrite(me);
			goto err;
		}
		me->l_alloc = new_alloc;
		me->l_elem  = new_vector;
	}
	/* Move objects above the point of insertion. */
	memmoveupc(me->l_elem + index + 1,
	           me->l_elem + index,
	           me->l_size - index,
	           sizeof(DREF DeeObject *));
	/* Store the given element. */
	me->l_elem[index] = elem;
	++me->l_size;
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
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	DeeList_LockWrite(me);
	if (index > me->l_size)
		index = me->l_size;
	ASSERT(me->l_size <= me->l_alloc);
	if (me->l_size + objc >= me->l_alloc) {
		size_t new_alloc = me->l_alloc * 2;
		DREF DeeObject **new_vector;
		if (!new_alloc)
			new_alloc = 2;
		while (new_alloc < me->l_size + objc)
			new_alloc *= 2;
do_realloc:
		new_vector = (DREF DeeObject **)Dee_TryRealloc(me->l_elem,
		                                               new_alloc *
		                                               sizeof(DREF DeeObject *));
		if unlikely(!new_vector) {
			if (new_alloc != me->l_size + objc) {
				new_alloc = me->l_size + objc;
				goto do_realloc;
			}
			if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
				goto do_realloc;
			DeeList_LockEndWrite(me);
			goto err;
		}
		me->l_alloc = new_alloc;
		me->l_elem  = new_vector;
	}
	/* Move objects above the point of insertion. */
	memmoveupc(me->l_elem + index + objc,
	           me->l_elem + index,
	           me->l_size - index,
	           sizeof(DREF DeeObject *));
	/* Store the given elements. */
	me->l_size += objc;
	while (objc--) {
		DeeObject *ob = *objv++;
		Dee_Incref(ob);
		me->l_elem[index++] = ob;
	}
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
	fast_seqlen = DeeFastSeq_GetSize(sequence);
	if (fast_seqlen != DEE_FASTSEQ_NOTFAST) {
		/* TODO: Special optimization. */
	}
	iterator = DeeObject_IterSelf(sequence);
	if unlikely(!iterator)
		goto err;
	result = DeeList_InsertIterator(self, index, iterator);
	Dee_Decref(iterator);
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_init_sequence(List *__restrict self,
                   DeeObject *__restrict seq) {
	int result;
	DeeObject *iter;
	size_t fast_size;
	ASSERT((DeeObject *)self != seq);
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		DREF DeeObject **elem;
		if (!fast_size) {
			elem = NULL;
		} else {
			/* Optimizations for fast-sequence types. */
			elem = (DREF DeeObject **)Dee_Malloc(fast_size *
			                                     sizeof(DREF DeeObject *));
			if unlikely(!elem)
				goto err;
			for (i = 0; i < fast_size; ++i) {
				DREF DeeObject *item;
				item = DeeFastSeq_GetItem(seq, i);
				if unlikely(!item)
					goto err_elem;
				elem[i] = item; /* Inherit reference. */
			}
		}
		self->l_elem = elem;
		self->l_size = self->l_alloc = fast_size;
		weakref_support_init(self);
		rwlock_init(&self->l_lock);
		return 0;
err_elem:
		while (i--)
			Dee_Decref(elem[i]);
		Dee_Free(elem);
		goto err;
	}
	iter = DeeObject_IterSelf(seq);
	if unlikely(!iter)
		goto err;
	result = list_init_iterator(self, iter);
	Dee_Decref(iter);
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
	if (filler || DeeInt_Check(sequence)) {
		size_t list_size;
		if (DeeObject_AsSize(sequence, &list_size))
			goto err;
		if (!list_size) {
			self->l_elem = NULL;
		} else {
			if (!filler)
				filler = Dee_None;
			self->l_elem = (DREF DeeObject **)Dee_Malloc(list_size *
			                                             sizeof(DREF DeeObject *));
			if unlikely(!self->l_elem)
				goto err;
			memsetp(self->l_elem, filler, list_size);
			Dee_Incref_n(filler, list_size);
		}
		self->l_alloc = self->l_size = list_size;
		weakref_support_init(self);
		rwlock_init(&self->l_lock);
		return 0;
	}
	return list_init_sequence(self, sequence);
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

PUBLIC NONNULL((1)) bool DCALL
DeeList_Clear(DeeObject *__restrict self) {
	List *me = (List *)self;
	DREF DeeObject **vec = NULL;
	size_t count;
	DeeList_LockWrite(me);
	count = me->l_size;
	if (count) {
		vec = me->l_elem;
		me->l_alloc = 0;
		me->l_size  = 0;
		me->l_elem  = NULL;
	}
	DeeList_LockEndWrite(me);
	Dee_Decrefv(vec, count);
	Dee_Free(vec);
	return count != 0;
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeList_PrintRepr(DeeObject *__restrict self,
                  dformatprinter printer, void *arg) {
	size_t i;
	dssize_t temp, result = 0;
	List *me = (List *)self;
	temp = (*printer)(arg, "[", 1);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	DeeList_LockRead(me);
	for (i = 0; i < me->l_size; ++i) {
		DREF DeeObject *elem = me->l_elem[i];
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_repr(List *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeList_PrintRepr((DeeObject *)self,
	                              &unicode_printer_print,
	                              &printer) < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL list_bool(List *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->l_size != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->l_size) != 0;
#endif /* !CONFIG_NO_THREADS */
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_size(List *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return DeeInt_NewSize(self->l_size);
#else /* CONFIG_NO_THREADS */
	return DeeInt_NewSize(ATOMIC_READ(self->l_size));
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_contains(List *self, DeeObject *elem) {
	DeeObject **iter, **end, *list_elem;
	DeeList_LockRead(self);
again:
	end = (iter = self->l_elem) + self->l_size;
	for (; iter != end; ++iter) {
		int error;
		list_elem = *iter;
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		error = DeeObject_CompareEq(elem, list_elem);
		Dee_Decref(list_elem);
		if unlikely(error < 0)
			goto err;
		if (error)
			return_true;
		DeeList_LockRead(self);
		/* Check if the list was changed. */
		if unlikely(end != self->l_elem + self->l_size ||
		            iter < self->l_elem)
			goto again;
	}
	DeeList_LockEndRead(self);
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_getitem(List *self, DeeObject *index) {
	size_t i;
	DREF DeeObject *result;
	if (DeeObject_AsSize(index, &i))
		goto err;
	DeeList_LockRead(self);
	if unlikely(i >= DeeList_SIZE(self)) {
		size_t list_size = DeeList_SIZE(self);
		DeeList_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, i, list_size);
		goto err;
	}
	result = DeeList_GET(self, i);
	Dee_Incref(result);
	DeeList_LockEndRead(self);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_i(List *__restrict self,
                dssize_t begin, dssize_t end) {
	DREF DeeObject **new_vector;
	DREF List *result;
	size_t i;
again:
	DeeList_LockRead(self);
	if unlikely(begin < 0)
		begin += self->l_size;
	if unlikely(end < 0)
		end += self->l_size;
	if unlikely((size_t)begin >= self->l_size ||
	            (size_t)begin >= (size_t)end) {
		/* Empty list. */
		DeeList_LockEndRead(self);
		return DeeList_New();
	}
	if unlikely((size_t)end > self->l_size)
		end = (dssize_t)self->l_size;
	end -= begin;
	ASSERT(end != 0);
	new_vector = (DREF DeeObject **)Dee_TryMalloc((size_t)end * sizeof(DREF DeeObject *));
	if unlikely(!new_vector) {
		DeeList_LockEndRead(self);
		if (Dee_CollectMemory((size_t)end * sizeof(DREF DeeObject *)))
			goto again;
		return NULL;
	}
	/* Copy vector elements. */
	for (i = 0; i < (size_t)end; ++i) {
		new_vector[i] = self->l_elem[(size_t)begin + i];
		Dee_Incref(new_vector[i]);
	}
	DeeList_LockEndRead(self);
	/* Create the new list descriptor. */
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err_elemv;
	/* Fill in the descriptor. */
	DeeObject_Init(result, &DeeList_Type);
	result->l_size = result->l_alloc = (size_t)end;
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	result->l_elem = new_vector;
	/* Start tracking it as a GC object. */
	DeeGC_Track((DeeObject *)result);
	return (DREF DeeObject *)result;
err_elemv:
	/* Cleanup on error. */
	while ((size_t)end--)
		Dee_Decref(new_vector[(size_t)end]);
	Dee_Free(new_vector);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_in(List *__restrict self, dssize_t begin) {
	DREF DeeObject **new_vector;
	DREF List *result;
	size_t i, new_size;
again:
	DeeList_LockRead(self);
	if unlikely(begin < 0)
		begin += self->l_size;
	if unlikely((size_t)begin >= self->l_size) {
		/* Empty list. */
		DeeList_LockEndRead(self);
		return DeeList_New();
	}
	new_size = self->l_size - begin;
	ASSERT(new_size != 0);
	new_vector = (DREF DeeObject **)Dee_TryMalloc((size_t)new_size *
	                                              sizeof(DREF DeeObject *));
	if unlikely(!new_vector) {
		DeeList_LockEndRead(self);
		if (Dee_CollectMemory((size_t)new_size *
		                      sizeof(DREF DeeObject *)))
			goto again;
		return NULL;
	}
	/* Copy vector elements. */
	for (i = 0; i < (size_t)new_size; ++i) {
		new_vector[i] = self->l_elem[(size_t)begin + i];
		Dee_Incref(new_vector[i]);
	}
	DeeList_LockEndRead(self);
	/* Create the new list descriptor. */
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err_elemv;
	/* Fill in the descriptor. */
	DeeObject_Init(result, &DeeList_Type);
	result->l_size = result->l_alloc = (size_t)new_size;
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	result->l_elem = new_vector;
	/* Start tracking it as a GC object. */
	DeeGC_Track((DeeObject *)result);
	return (DREF DeeObject *)result;
err_elemv:
	/* Cleanup on error. */
	while ((size_t)new_size--)
		Dee_Decref(new_vector[(size_t)new_size]);
	Dee_Free(new_vector);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
list_getrange(List *self, DeeObject *begin, DeeObject *end) {
	dssize_t i_begin, i_end = SSIZE_MAX;
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (DeeNone_Check(end))
		return list_getrange_in(self, i_begin);
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	return list_getrange_i(self, i_begin, i_end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
list_delitem_index(List *__restrict self, size_t index) {
	DREF DeeObject *delob;
	size_t length;
	ASSERT_OBJECT(self);
	ASSERT(DeeList_Check(self));
	DeeList_LockWrite(self);
	length = self->l_size;
	if unlikely(index >= length)
		goto unlock_and_err_index;
	/* Adjust to shift following elements downwards. */
	delob = DeeList_GET(self, index);
	self->l_size = --length;
	memmovedownc(DeeList_ELEM(self) + index,
	             DeeList_ELEM(self) + index + 1,
	             length - index,
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(self);
	Dee_Decref(delob);
	return 0;
unlock_and_err_index:
	DeeList_LockEndWrite(self);
	return err_index_out_of_bounds((DeeObject *)self,
	                               index,
	                               length);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_delitem(List *self, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return list_delitem_index(self, i);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
list_setitem_index(List *self, size_t index, DeeObject *value) {
	DREF DeeObject *old_item;
	DeeList_LockWrite(self);
	if unlikely(index >= DeeList_SIZE(self))
		goto unlock_and_err_index;
	old_item = DeeList_GET(self, index);
	Dee_Incref(value);
	DeeList_SET(self, index, value);
	DeeList_LockEndWrite(self);
	Dee_Decref(old_item);
	return 0;
	{
		size_t length;
unlock_and_err_index:
		length = DeeList_SIZE(self);
		DeeList_LockEndWrite(self);
		return err_index_out_of_bounds((DeeObject *)self,
		                               index,
		                               length);
	}
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
list_setitem(List *self, DeeObject *index, DeeObject *value) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return list_setitem_index(self, i, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF ListIterator *DCALL
list_iter(List *__restrict self) {
	DREF ListIterator *result;
	result = DeeObject_MALLOC(ListIterator);
	if unlikely(!result)
		goto done;
	result->li_list  = self;
	result->li_index = 0;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeListIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delrange_i(List *__restrict self,
                dssize_t start,
                dssize_t end) {
	DREF DeeObject **delobv;
	size_t count;
	dssize_t start_index, end_index;
again:
	start_index = start;
	end_index   = end;
	DeeList_LockWrite(self);
	if (end_index < 0) {
		end_index += DeeList_SIZE(self);
	} else if ((size_t)end_index > DeeList_SIZE(self)) {
		end_index = (dssize_t)DeeList_SIZE(self);
	}
	if (start_index < 0) {
		start_index += DeeList_SIZE(self);
	} else if ((size_t)start_index >= DeeList_SIZE(self)) {
		goto done_noop;
	}
	if ((size_t)start_index >= (size_t)end_index)
		goto done_noop;
	count  = (size_t)end_index - (size_t)start_index;
	delobv = (DREF DeeObject **)Dee_ATryMalloc((size_t)count *
	                                           sizeof(DREF DeeObject *));
	if unlikely(!delobv) {
		DeeList_LockEndWrite(self);
		if (Dee_CollectMemory((size_t)count * sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	/* Move all items to-be deleted into the delete-vector. */
	memcpyc(delobv, DeeList_ELEM(self) + (size_t)start_index,
	        (size_t)count, sizeof(DREF DeeObject *));
	memmovedownc(DeeList_ELEM(self) + (size_t)start_index,
	             DeeList_ELEM(self) + (size_t)end_index,
	             DeeList_SIZE(self) - (size_t)end_index,
	             sizeof(DREF DeeObject *));
	self->l_size -= count;
	DeeList_LockEndWrite(self);
	/* Drop object references. */
	while (count--)
		Dee_Decref(delobv[count]);
	/* Free the temporary del-item vector. */
	Dee_AFree(delobv);
done:
	return 0;
err:
	return -1;
done_noop:
	DeeList_LockEndWrite(self);
	goto done;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delrange_in(List *__restrict self,
                 dssize_t start) {
	DREF DeeObject **delobv;
	size_t count;
	dssize_t start_index;
again:
	start_index = start;
	DeeList_LockWrite(self);
	if (start_index < 0) {
		start_index += DeeList_SIZE(self);
	} else if ((size_t)start_index >= DeeList_SIZE(self)) {
		goto done_noop;
	}
	if ((size_t)start_index >= DeeList_SIZE(self))
		goto done_noop;
	count  = DeeList_SIZE(self) - (size_t)start_index;
	delobv = (DREF DeeObject **)Dee_ATryMalloc((size_t)count *
	                                           sizeof(DREF DeeObject *));
	if unlikely(!delobv) {
		DeeList_LockEndWrite(self);
		if (Dee_CollectMemory((size_t)count * sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	/* Move all items to-be deleted into the delete-vector. */
	memcpyc(delobv, DeeList_ELEM(self) + (size_t)start_index,
	        (size_t)count, sizeof(DREF DeeObject *));
	self->l_size -= count;
	DeeList_LockEndWrite(self);
	/* Drop object references. */
	while (count--)
		Dee_Decref(delobv[count]);
	/* Free the temporary del-item vector. */
	Dee_AFree(delobv);
done:
	return 0;
err:
	return -1;
done_noop:
	DeeList_LockEndWrite(self);
	goto done;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
list_setrange_fast_i(List *self,
                     dssize_t start, dssize_t end,
                     DeeObject *items,
                     size_t insert_count) {
	DREF DeeObject **delobv;
	size_t delete_count, i;
	dssize_t start_index, end_index;
	if unlikely(!insert_count)
		return list_delrange_i(self, start, end);
again:
	DeeList_LockWrite(self);
	start_index = start;
	end_index   = end;
	if (start_index < 0) {
		start_index += DeeList_SIZE(self);
	} else if ((size_t)start_index > DeeList_SIZE(self)) {
		start_index = (dssize_t)DeeList_SIZE(self);
	}
	if ((size_t)start_index >= (size_t)end_index) {
		/* Insert-only */
		delete_count = 0;
	} else {
		delete_count = (size_t)end_index - (size_t)start_index;
	}
	ASSERT(delete_count <= DeeList_SIZE(self));
	if (insert_count > delete_count) {
		/* Make sure the list has enough available memory. */
		size_t min_alloc = (DeeList_SIZE(self) - delete_count) + insert_count;
		if (min_alloc > self->l_alloc) {
			size_t new_alloc = self->l_alloc;
			DREF DeeObject **new_elem;
			if unlikely(!new_alloc)
				new_alloc = 16;
			do {
				new_alloc *= 2;
			} while (new_alloc < min_alloc);
			new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
			                                                           sizeof(DREF DeeObject *));
			if unlikely(!new_elem) {
				new_alloc = min_alloc;
				new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
				                                                           sizeof(DREF DeeObject *));
				if unlikely(!new_elem) {
					DeeList_LockEndWrite(self);
					/* Collect memory and try again. */
					if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
						goto again;
					goto err;
				}
			}
			self->l_alloc = new_alloc;
			self->l_elem  = new_elem;
		}
	}
	if (!delete_count) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(self) + (size_t)start_index + insert_count,
		           DeeList_ELEM(self) + (size_t)start_index,
		           DeeList_SIZE(self) - (size_t)start_index,
		           sizeof(DREF DeeObject *));
		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(self, (size_t)start_index + i, DeeFastSeq_GetItemNB(items, i));
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
	} else {
		delobv = (DREF DeeObject **)Dee_ATryMalloc(delete_count *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(self);
			if (Dee_CollectMemory(delete_count * sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(self) + (size_t)start_index,
		        delete_count, sizeof(DREF DeeObject *));
		/* Move following items to their proper places. */
		if ((size_t)start_index + insert_count != (size_t)end_index) {
			memmoveupc(DeeList_ELEM(self) + (size_t)start_index + insert_count,
			           DeeList_ELEM(self) + (size_t)end_index,
			           DeeList_SIZE(self) - (size_t)end_index,
			           sizeof(DREF DeeObject *));
		}
		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(self, (size_t)start_index + i, DeeFastSeq_GetItemNB(items, i));
		self->l_size -= delete_count;
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
		/* Drop object references. */
		while (delete_count--)
			Dee_Decref(delobv[delete_count]);
		/* Free the temporary del-item vector. */
		Dee_AFree(delobv);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
list_setrange_fast_in(List *self,
                      dssize_t start,
                      DeeObject *items,
                      size_t insert_count) {
	DREF DeeObject **delobv;
	size_t delete_count, i;
	dssize_t start_index;
again:
	DeeList_LockWrite(self);
	start_index = start;
	if (start_index < 0)
		start_index += DeeList_SIZE(self);
	if ((size_t)start_index >= DeeList_SIZE(self)) {
		/* Insert-only */
		start_index  = (dssize_t)DeeList_SIZE(self);
		delete_count = 0;
	} else {
		delete_count = DeeList_SIZE(self) - (size_t)start_index;
	}
	ASSERT(delete_count <= DeeList_SIZE(self));
	if (insert_count > delete_count) {
		/* Make sure the list has enough available memory. */
		size_t min_alloc = (DeeList_SIZE(self) - delete_count) + insert_count;
		if (min_alloc > self->l_alloc) {
			size_t new_alloc = self->l_alloc;
			DREF DeeObject **new_elem;
			if unlikely(!new_alloc)
				new_alloc = 16;
			do {
				new_alloc *= 2;
			} while (new_alloc < min_alloc);
			new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
			                                                           sizeof(DREF DeeObject *));
			if unlikely(!new_elem) {
				new_alloc = min_alloc;
				new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
				                                                           sizeof(DREF DeeObject *));
				if unlikely(!new_elem) {
					DeeList_LockEndWrite(self);
					/* Collect memory and try again. */
					if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
						goto again;
					goto err;
				}
			}
			self->l_alloc = new_alloc;
			self->l_elem  = new_elem;
		}
	}
	if (!delete_count) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(self) + (size_t)start_index + insert_count,
		           DeeList_ELEM(self) + (size_t)start_index,
		           DeeList_SIZE(self) - (size_t)start_index,
		           sizeof(DREF DeeObject *));
		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(self, (size_t)start_index + i, DeeFastSeq_GetItemNB(items, i));
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
	} else {
		delobv = (DREF DeeObject **)Dee_ATryMalloc(delete_count *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(self);
			if (Dee_CollectMemory(delete_count * sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(self) + (size_t)start_index,
		        delete_count, sizeof(DREF DeeObject *));
		/* Fill in the new items. */
		for (i = 0; i < insert_count; ++i)
			DeeList_SET(self, (size_t)start_index + i, DeeFastSeq_GetItemNB(items, i));
		self->l_size -= delete_count;
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
		/* Drop object references. */
		while (delete_count--)
			Dee_Decref(delobv[delete_count]);
		/* Free the temporary del-item vector. */
		Dee_AFree(delobv);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
list_setrange_i(List *self,
                dssize_t start, dssize_t end,
                DeeObject *items) {
	DREF DeeObject **delobv, **insertv;
	size_t delete_count, insert_count;
	dssize_t start_index, end_index;
	/* Special case: none -> delrange. */
	if (DeeNone_Check(items))
		return list_delrange_i(self, start, end);
	/* Check for special case: Fast-insert */
	insert_count = DeeFastSeq_GetSizeNB(items);
	if (insert_count != DEE_FASTSEQ_NOTFAST)
		return list_setrange_fast_i(self, start, end, items, insert_count);
	insertv = DeeSeq_AsHeapVector(items, &insert_count);
	if unlikely(!insertv)
		goto err;
	if unlikely(!insert_count) {
		Dee_Free(insertv);
		return list_delrange_i(self, start, end);
	}
again:
	start_index = start;
	end_index   = end;
	DeeList_LockWrite(self);
	if (end_index < 0) {
		end_index += DeeList_SIZE(self);
	} else if ((size_t)end_index > DeeList_SIZE(self)) {
		end_index = (dssize_t)DeeList_SIZE(self);
	}
	if (start_index < 0) {
		start_index += DeeList_SIZE(self);
	} else if ((size_t)start_index > DeeList_SIZE(self)) {
		start_index = (dssize_t)DeeList_SIZE(self);
	}
	if ((size_t)start_index >= (size_t)end_index) {
		if (!self->l_size) {
			/* Special case: The list didn't contain anything, yet
			 * -> Here, we can simply invert the insert-vector and call it a day! */
			Dee_Free(self->l_elem);
			self->l_alloc = insert_count;
			self->l_size  = insert_count;
			self->l_elem  = insertv; /* Inherit */
			DeeList_LockEndWrite(self);
			goto done;
		}
		/* Insert-only */
		delete_count = 0;
	} else {
		delete_count = (size_t)end_index - (size_t)start_index;
	}
	ASSERT(delete_count <= DeeList_SIZE(self));
	if (insert_count > delete_count) {
		/* Make sure the list has enough available memory. */
		size_t min_alloc = (DeeList_SIZE(self) - delete_count) + insert_count;
		if (min_alloc > self->l_alloc) {
			size_t new_alloc = self->l_alloc;
			DREF DeeObject **new_elem;
			if unlikely(!new_alloc)
				new_alloc = 16;
			do {
				new_alloc *= 2;
			} while (new_alloc < min_alloc);
			new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
			                                                           sizeof(DREF DeeObject *));
			if unlikely(!new_elem) {
				new_alloc = min_alloc;
				new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
				                                                           sizeof(DREF DeeObject *));
				if unlikely(!new_elem) {
					DeeList_LockEndWrite(self);
					/* Collect memory and try again. */
					if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
						goto again;
					goto err_insv;
				}
			}
			self->l_alloc = new_alloc;
			self->l_elem  = new_elem;
		}
	}
	if (!delete_count) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(self) + (size_t)start_index + insert_count,
		           DeeList_ELEM(self) + (size_t)start_index,
		           DeeList_SIZE(self) - (size_t)start_index,
		           sizeof(DREF DeeObject *));
		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(self) + (size_t)start_index, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
		Dee_Free(insertv);
	} else {
		delobv = (DREF DeeObject **)Dee_ATryMalloc(delete_count *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(self);
			if (Dee_CollectMemory(delete_count * sizeof(DREF DeeObject *)))
				goto again;
			goto err_insv;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(self) + (size_t)start_index,
		        delete_count, sizeof(DREF DeeObject *));
		/* Move following items to their proper places. */
		if ((size_t)start_index + insert_count != (size_t)end_index) {
			memmoveupc(DeeList_ELEM(self) + (size_t)start_index + insert_count,
			           DeeList_ELEM(self) + (size_t)end_index,
			           DeeList_SIZE(self) - (size_t)end_index,
			           sizeof(DREF DeeObject *));
		}
		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(self) + (size_t)start_index, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		self->l_size -= delete_count;
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
		Dee_Free(insertv);
		/* Drop object references. */
		while (delete_count--)
			Dee_Decref(delobv[delete_count]);
		/* Free the temporary del-item vector. */
		Dee_AFree(delobv);
	}
done:
	return 0;
err_insv:
	while (insert_count--)
		Dee_Decref(insertv[insert_count]);
	Dee_Free(insertv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
list_setrange_in(List *self, dssize_t start, DeeObject *items) {
	DREF DeeObject **delobv, **insertv;
	size_t delete_count, insert_count;
	dssize_t start_index;
	/* Special case: none -> delrange. */
	if (DeeNone_Check(items))
		return list_delrange_in(self, start);
	/* Check for special case: Fast-insert */
	insert_count = DeeFastSeq_GetSizeNB(items);
	if (insert_count != DEE_FASTSEQ_NOTFAST) {
		if (!insert_count)
			return list_delrange_in(self, start);
		return list_setrange_fast_in(self, start, items, insert_count);
	}
	insertv = DeeSeq_AsHeapVector(items, &insert_count);
	if unlikely(!insertv)
		goto err;
	if unlikely(!insert_count) {
		Dee_Free(insertv);
		return list_delrange_in(self, start);
	}
again:
	start_index = start;
	DeeList_LockWrite(self);
	if (start_index < 0)
		start_index += DeeList_SIZE(self);
	if ((size_t)start_index >= DeeList_SIZE(self)) {
		start_index = (dssize_t)DeeList_SIZE(self);
		if (!self->l_size) {
			/* Special case: The list didn't contain anything, yet
			 * -> Here, we can simply invert the insert-vector and call it a day! */
			Dee_Free(self->l_elem);
			self->l_alloc = insert_count;
			self->l_size  = insert_count;
			self->l_elem  = insertv; /* Inherit */
			DeeList_LockEndWrite(self);
			goto done;
		}
		/* Insert-only */
		delete_count = 0;
	} else {
		delete_count = DeeList_SIZE(self) - (size_t)start_index;
	}
	ASSERT(delete_count <= DeeList_SIZE(self));
	if (insert_count > delete_count) {
		/* Make sure the list has enough available memory. */
		size_t min_alloc = (DeeList_SIZE(self) - delete_count) + insert_count;
		if (min_alloc > self->l_alloc) {
			size_t new_alloc = self->l_alloc;
			DREF DeeObject **new_elem;
			if unlikely(!new_alloc)
				new_alloc = 16;
			do {
				new_alloc *= 2;
			} while (new_alloc < min_alloc);
			new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
			                                                           sizeof(DREF DeeObject *));
			if unlikely(!new_elem) {
				new_alloc = min_alloc;
				new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
				                                                           sizeof(DREF DeeObject *));
				if unlikely(!new_elem) {
					DeeList_LockEndWrite(self);
					/* Collect memory and try again. */
					if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeObject *)))
						goto again;
					goto err_insv;
				}
			}
			self->l_alloc = new_alloc;
			self->l_elem  = new_elem;
		}
	}
	if (!delete_count) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(self) + (size_t)start_index + insert_count,
		           DeeList_ELEM(self) + (size_t)start_index,
		           DeeList_SIZE(self) - (size_t)start_index,
		           sizeof(DREF DeeObject *));
		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(self) + (size_t)start_index, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
		Dee_Free(insertv);
	} else {
		delobv = (DREF DeeObject **)Dee_ATryMalloc(delete_count *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!delobv) {
			DeeList_LockEndWrite(self);
			if (Dee_CollectMemory(delete_count * sizeof(DREF DeeObject *)))
				goto again;
			goto err_insv;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delobv, DeeList_ELEM(self) + (size_t)start_index,
		        delete_count, sizeof(DREF DeeObject *));
		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(self) + (size_t)start_index, insertv,
		        insert_count, sizeof(DREF DeeObject *));
		self->l_size -= delete_count;
		self->l_size += insert_count;
		DeeList_LockEndWrite(self);
		Dee_Free(insertv);
		/* Drop object references. */
		while (delete_count--)
			Dee_Decref(delobv[delete_count]);
		/* Free the temporary del-item vector. */
		Dee_AFree(delobv);
	}
done:
	return 0;
err_insv:
	while (insert_count--)
		Dee_Decref(insertv[insert_count]);
	Dee_Free(insertv);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
list_delrange(List *self, DeeObject *start_ob, DeeObject *end_ob) {
	dssize_t start_index, end_index;
	if (DeeObject_AsSSize(start_ob, &start_index))
		goto err;
	if (DeeNone_Check(end_ob))
		return list_delrange_in(self, start_index);
	if (DeeObject_AsSSize(end_ob, &end_index))
		goto err;
	return list_delrange_i(self, start_index, end_index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
list_setrange(List *self, DeeObject *start_ob,
              DeeObject *end_ob, DeeObject *items) {
	dssize_t start_index, end_index;
	if (DeeObject_AsSSize(start_ob, &start_index))
		goto err;
	if (DeeNone_Check(end_ob))
		return list_setrange_in(self, start_index, items);
	if (DeeObject_AsSSize(end_ob, &end_index))
		goto err;
	return list_setrange_i(self, start_index, end_index, items);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_nsi_getsize(List *__restrict self) {
	ASSERT(self->l_size != (size_t)-1);
#ifdef CONFIG_NO_THREADS
	return self->l_size;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->l_size);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_nsi_getitem(List *__restrict self, size_t index) {
	DREF DeeObject *result;
	DeeList_LockRead(self);
	if unlikely(index >= DeeList_SIZE(self)) {
		size_t list_size = DeeList_SIZE(self);
		DeeList_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, list_size);
		return NULL;
	}
	result = DeeList_GET(self, index);
	Dee_Incref(result);
	DeeList_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
list_nsi_find(List *self,
              size_t start, size_t end,
              DeeObject *keyed_search_item,
              DeeObject *key) {
	DREF DeeObject *list_elem;
	size_t i;
	int temp;
	DeeList_LockRead(self);
	for (i = start; i < DeeList_SIZE(self) && i < end; ++i) {
		list_elem = DeeList_GET(self, i);
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		temp = DeeObject_CompareKeyEq(keyed_search_item, list_elem, key);
		Dee_Decref(list_elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i; /* Found it! */
		}
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
list_nsi_rfind(List *self, size_t start, size_t end,
               DeeObject *keyed_search_item,
               DeeObject *key) {
	DREF DeeObject *list_elem;
	size_t i;
	int temp;
	DeeList_LockRead(self);
	i = end;
	for (;;) {
		if (i > DeeList_SIZE(self))
			i = DeeList_SIZE(self);
		if (i <= start)
			break;
		--i;
		list_elem = DeeList_GET(self, i);
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		temp = DeeObject_CompareKeyEq(keyed_search_item, list_elem, key);
		Dee_Decref(list_elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i; /* Found it! */
		}
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
list_nsi_xch(List *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	DeeList_LockWrite(self);
	if (index >= DeeList_SIZE(self)) {
		size_t my_length = DeeList_SIZE(self);
		DeeList_LockEndWrite(self);
		err_index_out_of_bounds((DeeObject *)self, index, my_length);
		return NULL;
	}
	Dee_Incref(value);
	result = DeeList_GET(self, index); /* Inherit reference. */
	DeeList_SET(self, index, value);   /* Inherit reference. */
	DeeList_LockEndWrite(self);
	return result;
}


PRIVATE struct type_nsi tpconst list_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&list_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&list_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&list_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)&list_delitem_index,
			/* .nsi_setitem      = */ (dfunptr_t)&list_setitem_index,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)&list_getrange_i,
			/* .nsi_getrange_n   = */ (dfunptr_t)&list_getrange_in,
			/* .nsi_setrange     = */ (dfunptr_t)&list_setrange_i,
			/* .nsi_setrange_n   = */ (dfunptr_t)&list_setrange_in,
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
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_setitem,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_getrange,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_delrange,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&list_setrange,
	/* .tp_nsi       = */ &list_nsi
};


INTDEF struct keyword seq_insert_kwlist[];
INTDEF struct keyword seq_insertall_kwlist[];
INTDEF struct keyword seq_erase_kwlist[];
INTDEF struct keyword seq_xch_kwlist[];
INTDEF struct keyword seq_removeif_kwlist[];
INTDEF struct keyword seq_pop_kwlist[];
INTDEF struct keyword seq_resize_kwlist[];

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_append(List *self, size_t argc, DeeObject *const *argv) {
	/* Optimize for the case of a single object to-be appended. */
	if likely(argc == 1) {
		if (DeeList_Append((DeeObject *)self, argv[0]))
			goto err;
	} else {
		if (DeeList_AppendVector((DeeObject *)self, argc, argv))
			goto err;
	}
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_extend(List *self, size_t argc, DeeObject *const *argv) {
	DeeObject *extension;
	if (DeeArg_Unpack(argc, argv, "o:extend", &extension))
		goto err;
	if (DeeList_AppendSequence((DeeObject *)self, extension))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_insert(List *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insert_kwlist, "Iuo:insert", &index, &item))
		goto err;
	if (DeeList_Insert((DeeObject *)self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_insertall(List *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insertall_kwlist, "Ido:insertall", &index, &items))
		goto err;
	if (DeeList_InsertSequence((DeeObject *)self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_removeif(List *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *should;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_removeif_kwlist, "o|IdId:removeif", &should, &start, &end))
		goto err;
	result = DeeList_RemoveIf(self, start, end, should);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
list_insertiter_deprecated(List *self,
                           size_t argc, DeeObject *const *argv) {
	size_t index;
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "Ido:insert_iter", &index, &seq))
		goto err;
	if (DeeList_InsertIterator((DeeObject *)self, index, seq))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_erase(List *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_erase_kwlist, "Iu|Iu:erase", &index, &count))
		goto err;
	count = DeeList_Erase((DeeObject *)self, index, count);
	if unlikely(count == (size_t)-1)
		goto err;
	return DeeInt_NewSize(count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_xch(List *self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *value;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_xch_kwlist, "Iuo:xch", &index, &value))
		goto err;
	return list_nsi_xch(self, index, value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_pop(List *self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	dssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_pop_kwlist, "|Id:pop", &index))
		goto err;
	return DeeList_Pop((DeeObject *)self, index);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
list_clear(List *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	DeeList_Clear((DeeObject *)self);
	return_none;
err:
	return NULL;
}

#ifdef CONFIG_NO_THREADS
#define WEAK_READSIZE(x) ((x)->l_size)
#else /* CONFIG_NO_THREADS */
#define WEAK_READSIZE(x) ATOMIC_READ((x)->l_size)
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sizeof(List *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(sizeof(List) +
	                      (WEAK_READSIZE(self) * sizeof(DeeObject *)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_get_first(List *__restrict self) {
	DREF DeeObject *result;
	DeeList_LockRead(self);
	if unlikely(DeeList_IsEmpty(self))
		goto err_empty;
	result = DeeList_GET(self, 0);
	Dee_Incref(result);
	DeeList_LockEndRead(self);
	return result;
err_empty:
	DeeList_LockEndRead(self);
	err_empty_sequence((DeeObject *)self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_del_first(List *__restrict self) {
	DREF DeeObject *delob;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	DeeList_LockWrite(self);
	if unlikely(DeeList_IsEmpty(self))
		goto err_empty;
	/* Adjust to shift following elements downwards. */
	delob = DeeList_GET(self, 0);
	DeeList_SIZE(self) -= 1;
	memmovedownc(DeeList_ELEM(self),
	             DeeList_ELEM(self) + 1,
	             DeeList_SIZE(self),
	             sizeof(DREF DeeObject *));
	DeeList_LockEndWrite(self);
	Dee_Decref(delob);
	return 0;
err_empty:
	DeeList_LockEndRead(self);
	return err_empty_sequence((DeeObject *)self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_set_first(List *self, DeeObject *value) {
	DREF DeeObject *oldob;
	DeeList_LockWrite(self);
	if unlikely(DeeList_IsEmpty(self))
		goto err_empty;
	Dee_Incref(value);
	oldob = DeeList_GET(self, 0);
	DeeList_SET(self, 0, value);
	DeeList_LockEndWrite(self);
	Dee_Decref(oldob);
	return 0;
err_empty:
	DeeList_LockEndWrite(self);
	return err_empty_sequence((DeeObject *)self);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_get_last(List *__restrict self) {
	DREF DeeObject *result;
	DeeList_LockRead(self);
	if unlikely(DeeList_IsEmpty(self))
		goto err_empty;
	result = DeeList_GET(self, DeeList_SIZE(self) - 1);
	Dee_Incref(result);
	DeeList_LockEndRead(self);
	return result;
err_empty:
	DeeList_LockEndRead(self);
	err_empty_sequence((DeeObject *)self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_del_last(List *__restrict self) {
	DREF DeeObject *delob;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	DeeList_LockWrite(self);
	if unlikely(DeeList_IsEmpty(self))
		goto err_empty;
	DeeList_SIZE(self) -= 1;
	delob = DeeList_GET(self, DeeList_SIZE(self));
	DeeList_LockEndWrite(self);
	Dee_Decref(delob);
	return 0;
err_empty:
	DeeList_LockEndRead(self);
	return err_empty_sequence((DeeObject *)self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_set_last(List *self, DeeObject *value) {
	DREF DeeObject *oldob;
	size_t index;
	DeeList_LockWrite(self);
	if unlikely(DeeList_IsEmpty(self))
		goto err_empty;
	index = DeeList_SIZE(self) - 1;
	Dee_Incref(value);
	oldob = DeeList_GET(self, index);
	DeeList_SET(self, index, value);
	DeeList_LockEndWrite(self);
	Dee_Decref(oldob);
	return 0;
err_empty:
	DeeList_LockEndWrite(self);
	return err_empty_sequence((DeeObject *)self);
}

#ifdef __OPTIMIZE_SIZE__
#define list_get_frozen DeeTuple_FromSequence
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_get_frozen(List *__restrict self) {
	size_t i, count;
	DREF DeeObject *result;
again:
	DeeList_LockWrite(self);
	count = DeeList_SIZE(self);
	result = DeeTuple_TryNewUninitialized(count);
	if unlikely(!result) {
		DeeList_LockEndWrite(self);
		if (Dee_CollectMemory(DeeTuple_SIZEOF(count)))
			goto again;
		return NULL;
	}
	/* Copy elements. */
	memcpyc(DeeTuple_ELEM(result), DeeList_ELEM(self),
	        count, sizeof(DREF DeeObject *));
	for (i = 0; i < count; ++i)
		Dee_Incref(DeeTuple_GET(result, i));
	DeeList_LockEndWrite(self);
	return result;
}
#endif /* !__OPTIMIZE_SIZE__ */



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getallocated(List *__restrict self) {
	size_t result;
	DeeList_LockRead(self);
	result = self->l_alloc;
	DeeList_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE NONNULL((1)) void DCALL
list_do_shrink(List *__restrict self) {
	DeeList_LockWrite(self);
	if (self->l_size < self->l_alloc) {
		DREF DeeObject **new_elem;
		new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, self->l_size *
		                                                           sizeof(DREF DeeObject *));
		if likely(new_elem) {
			self->l_elem  = new_elem;
			self->l_alloc = self->l_size;
		}
	}
	DeeList_LockEndWrite(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delallocated(List *__restrict self) {
	list_do_shrink(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_setallocated(List *self, DeeObject *value) {
	size_t new_alloc;
	DREF DeeObject **new_elem;
	if (DeeObject_AsSize(value, &new_alloc))
		goto err;
	DeeList_LockWrite(self);
	ASSERT(self->l_alloc >= self->l_size);
	if (new_alloc != self->l_alloc) {
		/* Make sure that the new allocation isn't too low */
		if unlikely(new_alloc < self->l_size) {
			size_t my_size = self->l_size;
			DeeList_LockEndWrite(self);
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot lower list allocation to %Iu when size is %Iu",
			                new_alloc, my_size);
			goto err;
		}
		/* Release / allocate memory. */
		new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, new_alloc *
		                                                           sizeof(DREF DeeObject *));
		if unlikely(!new_elem)
			goto done_unlock;
		self->l_elem  = new_elem;
		self->l_alloc = new_alloc;
	}
	ASSERT(self->l_alloc >= self->l_size);
done_unlock:
	DeeList_LockEndWrite(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_pushfront(List *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &item))
		goto err;
	if (DeeList_Insert((DeeObject *)self, 0, item))
		goto err;
	return_none;
err:
	return NULL;
}
#define list_pushback list_append
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_popfront(List *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return DeeList_Pop((DeeObject *)self, 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_popback(List *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	return DeeList_Pop((DeeObject *)self, -1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_reserve(List *self, size_t argc, DeeObject *const *argv) {
	size_t size;
	if (DeeArg_Unpack(argc, argv, "Iu:reserve", &size))
		goto err;
	DeeList_LockWrite(self);
	if (size > self->l_alloc) {
		/* Try to allocate more memory for this List. */
		DREF DeeObject **new_elem;
		new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, size *
		                                                           sizeof(DREF DeeObject *));
		if likely(new_elem) {
			self->l_elem  = new_elem;
			self->l_alloc = size;
		}
	}
	DeeList_LockEndWrite(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_shrink(List *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":shrink"))
		goto err;
	list_do_shrink(self);
	return_none;
err:
	return NULL;
}

/* Reverse the order of the elements of `self' */
PUBLIC NONNULL((1)) void DCALL
DeeList_Reverse(DeeObject *__restrict self) {
	DeeObject **lo, **hi;
	DeeList_LockWrite(self);
	hi = (lo = DeeList_ELEM(self)) + DeeList_SIZE(self);
	while (lo < hi) {
		DeeObject *temp;
		temp  = *lo;
		*lo++ = *--hi;
		*hi   = temp;
	}
	DeeList_LockEndWrite(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_reverse(List *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":reverse"))
		goto err;
	DeeList_Reverse((DeeObject *)self);
	return_none;
err:
	return NULL;
}



/* Sort the given list ascendingly, or according to `key' */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeList_Sort(DeeObject *self, DeeObject *key) {
	DeeObject **oldv, **newv;
	size_t i, objc;
	objc = DeeList_SIZE(self);
	oldv = (DeeObject **)Dee_Malloc(objc * sizeof(DeeObject *));
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(self);
	if unlikely(DeeList_SIZE(self) > objc) {
		DeeObject **new_objv;
		objc = DeeList_SIZE(self);
		DeeList_LockEndRead(self);
		new_objv = (DeeObject **)Dee_Realloc(oldv, objc * sizeof(DeeObject *));
		if unlikely(!new_objv)
			goto err_oldv;
		oldv = new_objv;
		goto again;
	}
	/* Read all the old elements from the list. */
	for (i = 0; i < objc; ++i) {
		oldv[i] = DeeList_GET(self, i);
		Dee_Incref(oldv[i]);
	}
	DeeList_LockEndRead(self);
	/* Allocate the new list */
	newv = (DeeObject **)Dee_Malloc(objc * sizeof(DeeObject *));
	if unlikely(!newv)
		goto err_oldv_elem;
	/* Do the actual sorting. */
	if unlikely(DeeSeq_MergeSort(newv, oldv, objc, key))
		goto err_newv;
	Dee_Free(oldv);
	DeeList_LockWrite(self);
	oldv = ((List *)self)->l_elem;
	i    = ((List *)self)->l_size;
	((List *)self)->l_size  = objc;
	((List *)self)->l_elem  = newv;
	((List *)self)->l_alloc = objc;
	DeeList_LockEndWrite(self);
	while (i--)
		Dee_Decref(oldv[i]);
	Dee_Free(oldv);
	return 0;
err_newv:
	Dee_Free(newv);
err_oldv_elem:
	for (i = 0; i < objc; ++i)
		Dee_Decref(oldv[i]);
err_oldv:
	Dee_Free(oldv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_Sorted(DeeObject *self, DeeObject *key) {
	DeeObject **oldv;
	size_t i, objc;
	DeeTupleObject *result;
	objc = DeeList_SIZE(self);
	oldv = (DeeObject **)Dee_Malloc(objc * sizeof(DeeObject *));
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(self);
	if unlikely(DeeList_SIZE(self) > objc) {
		DeeObject **new_objv;
		objc = DeeList_SIZE(self);
		DeeList_LockEndRead(self);
		new_objv = (DeeObject **)Dee_Realloc(oldv, objc * sizeof(DeeObject *));
		if unlikely(!new_objv)
			goto err_oldv;
		oldv = new_objv;
		goto again;
	}
	/* Read all the old elements from the list. */
	for (i = 0; i < objc; ++i) {
		oldv[i] = DeeList_GET(self, i);
		Dee_Incref(oldv[i]);
	}
	DeeList_LockEndRead(self);
	/* Allocate the new list */
	result = (DeeTupleObject *)DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto err_oldv_elem;
	/* Do the actual sorting. */
	if (DeeSeq_MergeSort(DeeTuple_ELEM(result), oldv, objc, key))
		goto err_result;
	Dee_Free(oldv);
	return (DeeObject *)result;
err_result:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err_oldv_elem:
	for (i = 0; i < objc; ++i)
		Dee_Decref(oldv[i]);
err_oldv:
	Dee_Free(oldv);
err:
	return NULL;
}


INTDEF struct keyword seq_sort_kwlist[];
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sort(List *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:sort", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	if (DeeList_Sort((DeeObject *)self, key))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_sorted(List *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:sorted", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	return DeeList_Sorted((DeeObject *)self, key);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_resize(List *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t newsize;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_resize_kwlist, "Iu|o:resize", &newsize, &filler))
		goto err;
again:
	DeeList_LockWrite(self);
	if (newsize > self->l_size) {
		size_t num_more;
		if (self->l_alloc < newsize) {
			/* Try to resize the list to make it bigger. */
			DREF DeeObject **new_elem;
			new_elem = (DREF DeeObject **)Dee_TryRealloc(self->l_elem, newsize *
			                                                           sizeof(DREF DeeObject *));
			if unlikely(!new_elem) {
				DeeList_LockEndWrite(self);
				if (Dee_CollectMemory(newsize * sizeof(DREF DeeObject *)))
					goto again;
				goto err;
			}
			self->l_elem  = new_elem;
			self->l_alloc = newsize;
		}
		/* Fill in the new items. */
		num_more = newsize - self->l_size;
		memsetp(self->l_elem + self->l_size, filler, num_more);
		Dee_Incref_n(filler, num_more);
		self->l_size = newsize;
		DeeList_LockEndWrite(self);
	} else if (newsize == self->l_size) {
		/* Size didn't change */
		DeeList_LockEndWrite(self);
	} else if (!newsize) {
		/* Clear the list of all items. */
		DREF DeeObject **old_elem;
		size_t i, old_alloc, old_size;
		/* Remove everything */
		old_elem      = self->l_elem;
		old_alloc     = self->l_alloc;
		old_size      = self->l_size;
		self->l_elem  = NULL;
		self->l_alloc = 0;
		self->l_size  = 0;
		DeeList_LockEndWrite(self);
		for (i = 0; i < old_size; ++i)
			Dee_Decref(old_elem[i]);
		DeeList_LockWrite(self);
		if (!self->l_elem) {
			ASSERT(self->l_alloc == 0);
			ASSERT(self->l_size == 0);
			/* Allow the list to re-use its old vector. */
			self->l_elem  = old_elem;
			self->l_alloc = old_alloc;
			old_elem      = NULL; /* Inherit the old vector. */
		}
		DeeList_LockEndWrite(self);
		/* Free the old vector. */
		Dee_Free(old_elem);
	} else {
		/* Must remove items. */
		DREF DeeObject **old_obj;
		size_t num_del;
		num_del = self->l_size - newsize;
		old_obj = (DREF DeeObject **)Dee_ATryMalloc(num_del * sizeof(DREF DeeObject *));
		if unlikely(!old_obj) {
			DeeList_LockEndWrite(self);
			if (Dee_CollectMemory(newsize * sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}
		memcpyc(old_obj, &self->l_elem[newsize],
		        num_del, sizeof(DREF DeeObject *));
		self->l_size = newsize;
		DeeList_LockEndWrite(self);
		/* Drop references from all objects that were deleted. */
		while (num_del--)
			Dee_Decref(old_obj[num_del]);
		Dee_AFree(old_obj);
	}
	return_none;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst list_getsets[] = {
	{ "allocated",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_getallocated,
	  (int (DCALL *)(DeeObject *__restrict))&list_delallocated,
	  (int (DCALL *)(DeeObject *, DeeObject *))&list_setallocated,
	  DOC("->?Dint\n"
	      "@throw ValueError Attmpted to set the List preallocation size to a value lower than ${##this}\n"
	      "The number of allocated items\n"
	      "When using performing a del-operation on this property, the allocation will "
	      "be set to use the least amount of memory, which is achived by setting it to ${##this}.\n"
	      "Note however that when lowering the amount of allocated vector space, failure to "
	      "reallocate the internal List vector is ignored, and the allocated List size will "
	      "not be modified\n"
	      "Similarly, failure to allocate more memory when increasing the allocated size "
	      "of a List is ignored, with the previously allocated size remaining unchanged.\n"
	      "${"
	      "del mylist.allocated;\n"
	      "/* Same as this: */\n"
	      "mylist.shrink();\n"
	      "/* And same as an atomic variant of: */\n"
	      "mylist.allocated = ##mylist;"
	      "}") },
	{ DeeString_STR(&str_first),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_get_first,
	  (int (DCALL *)(DeeObject *__restrict))&list_del_first,
	  (int (DCALL *)(DeeObject *, DeeObject *))&list_set_first,
	  DOC("->\n"
	      "@return The first item from @this List") },
	{ DeeString_STR(&str_last),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_get_last,
	  (int (DCALL *)(DeeObject *__restrict))&list_del_last,
	  (int (DCALL *)(DeeObject *, DeeObject *))&list_set_last,
	  DOC("->\n"
	      "@return The last item from @this List") },
	{ "frozen",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_get_frozen,
	  NULL,
	  NULL,
	  DOC("->?DTuple\n"
	      "Return a copy of the contents of @this List as an immutable sequence") },
	{ NULL }
};

PRIVATE struct type_method tpconst list_methods[] = {
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_sizeof },
	{ DeeString_STR(&str_append),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_append,
	  DOC("(items!)\n"
	      "Append all the given @items at the end of @this List") },
	{ DeeString_STR(&str_extend),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_extend,
	  DOC("(items:?S?O)\n"
	      "Append all elements from @items to the end of @this List") },
	{ DeeString_STR(&str_resize),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_resize,
	  DOC("(newsize:?Dint,filler=!N)\n"
	      "@throw IntegerOverflow The given @newsize is lower than $0\n"
	      "Resize the size of @this List to match @newsize, using @filler to initialize new items"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_insert),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_insert,
	  DOC("(index:?Dint,item)\n"
	      "@throw IntegerOverflow The given @index is too large\n"
	      "Insert the given @item at the specified @index\n"
	      "When @index is lower than $0, or greater thatn ${##this}, append items at the end"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_insertall),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_insertall,
	  DOC("(index:?Dint,seq:?S?O)\n"
	      "@throw IntegerOverflow The given @index is too large\n"
	      "Insert all items from a sequence @seq at the specified @index\n"
	      "When @index is lower than $0, or greater thatn ${##this}, append items at the end"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_erase),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_erase,
	  DOC("(index:?Dint,count=!1)->?Dint\n"
	      "@throw IntegerOverflow The given @index or @count are lower than $0, or too large\n"
	      "@return The number of erased items (Zero when @index is out of bounds)\n"
	      "Erase up to @count items starting at the given @index"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_pop),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_pop,
	  DOC("(index=!-1)->\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "@return Items that was removed\n"
	      "Pops an item at the given @{index}. When @index is lower "
	      "than $0, add ${##this} prior to evaluation, meaning that "
	      "negative numbers pop items relative to the end of the List"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_xch),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_xch,
	  DOC("(index:?Dint,value)->\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Exchange the @index'th item of @this List with @value, returning the item's old value"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_clear),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_clear,
	  DOC("()\n"
	      "Clear all items from @this List") },
	{ DeeString_STR(&str_removeif),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_removeif,
	  DOC("(should:?DCallable,start=!0,end=!-1)->?Dint\n"
	      "@return The number of removed items"
	      "Remove all elements within the given sub-range for which ${!!should(elem)} is true"),
	  TYPE_METHOD_FKWDS },

	{ DeeString_STR(&str_pushfront),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_pushfront,
	  DOC("(item)\nInserts @item at the fron of this @this List. Same as ${this.insert(0,item)}") },
	{ DeeString_STR(&str_pushback),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_pushback,
	  DOC("(item)\nInserts @item at the end of this @this List. Same as ?#append") },
	{ DeeString_STR(&str_popfront),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_popfront,
	  DOC("->\nSame as ${this.pop(0)}") },
	{ DeeString_STR(&str_popback),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_popback,
	  DOC("->\nSame as ${this.pop(-1)}") },

	/* List ordering functions. */
	{ "reverse",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_reverse,
	  DOC("()\n"
	      "Reverse the order of all the elements of @this List") },
	{ "sort",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_sort,
	  DOC("()\n"
	      "(key:?DCallable)\n"
	      "Sort the elements of @this List in ascending order, or in accordance to @key"),
	  TYPE_METHOD_FKWDS },
	{ "sorted",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_sorted,
	  DOC("->?S?O\n"
	      "(key:?DCallable)->?S?O\n"
	      "Return a sequence that contains all elements from @this sequence, "
	      "but sorted in ascending order, or in accordance to @key\n"
	      "The type of sequence returned is implementation-defined"),
	  TYPE_METHOD_FKWDS },

	/* List buffer functions. */
	{ "reserve", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_reserve,
	  DOC("(size:?Dint)\n"
	      "Reserve (preallocate) memory for @size items\n"
	      "Failures to pre-allocate memory are silently ignored, in which case ?#allocated will remain unchanged\n"
	      "If @size is lower than the currently ?#allocated size, the function becomes a no-op") },
	{ "shrink", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_shrink,
	  DOC("()\n"
	      "Release any pre-allocated, but unused memory, setting ?#allocated to the length of @this List") },

	/* Deprecated aliases / functions. */
	{ "remove_if",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_removeif,
	  DOC("(should:?DCallable,start=!0,end=!-1)->?Dint\n"
	      "Deprecated alias for ?#removeif"),
	  TYPE_METHOD_FKWDS },
	{ "insert_list",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_insertall,
	  DOC("(index:?Dint,items:?S?O)\n"
	      "Deprecated alias for ?#insertall"),
	  TYPE_METHOD_FKWDS },
	{ "insert_iter",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_insertiter_deprecated,
	  DOC("(index:?Dint,iter:?DIterator)\n"
	      "Deprecated alias for ${this.insertall(index, (iter as iterator from deemon).future)}") },
	{ "push_front",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_pushfront,
	  DOC("(item)\n"
	      "Deprecated alias for ?#pushfront") },
	{ "push_back",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_pushback,
	  DOC("(item)\n"
	      "Deprecated alias for ?#pushback") },
	{ "pop_front",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_popfront,
	  DOC("(item)\n"
	      "Deprecated alias for ?#popfront") },
	{ "pop_back",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_popback,
	  DOC("(item)\n"
	      "Deprecated alias for ?#popback") },
	{ "shrink_to_fit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&list_shrink,
	  DOC("()\n"
	      "Deprecated alias for ?#shrink") },
	/* TODO: DEE_METHODDEF_v100("sorted_insert", member(&_deelist_sorted_insert), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_v100("fill", member(&_deelist_fill), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_v100("unique", member(&_deelist_unique), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_CONST_v100("tounique", member(&_deelist_tounique), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_CONST_v100("extend_unique", member(&_deelist_extend_unique), DEE_DOC_AUTO), */
	{ NULL }
};

PRIVATE struct type_gc tpconst list_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&DeeList_Clear
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF List *DCALL
list_add(List *self, DeeObject *other) {
	DREF List *result;
	result = (DREF List *)DeeList_Copy((DeeObject *)self);
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
list_inplace_add(List **__restrict pself,
                 DeeObject *other) {
	return DeeList_AppendSequence((DeeObject *)*pself, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF List *DCALL
list_mul(List *self, DeeObject *other) {
	size_t i, my_length, result_length, multiplier;
	DREF List *result;
	DREF DeeObject **elemv, **dst;
	if (DeeObject_AsSize(other, &multiplier))
		goto err;
	if unlikely(!multiplier)
		return (DREF List *)DeeList_New();
again:
	DeeList_LockRead(self);
	my_length = self->l_size;
	if (OVERFLOW_UMUL(my_length, multiplier, &result_length)) {
		DeeList_LockEndRead(self);
		err_integer_overflow_i(sizeof(size_t) * 8, true);
		goto err;
	}
	elemv = (DREF DeeObject **)Dee_TryMalloc(result_length * sizeof(DREF DeeObject *));
	if unlikely(!elemv) {
		DeeList_LockEndRead(self);
		if (Dee_CollectMemory(result_length * sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	for (i = 0; i < my_length; ++i) {
		DeeObject *obj;
		obj = DeeList_GET(self, i);
		Dee_Incref_n(obj, multiplier);
	}
	dst = elemv;
	for (i = 0; i < multiplier; ++i) {
		memcpyc(dst, DeeList_ELEM(self),
		        my_length, sizeof(DREF DeeObject *));
		dst += my_length;
	}
	DeeList_LockEndRead(self);
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err_elem;
	result->l_elem  = elemv;
	result->l_alloc = result_length;
	result->l_size  = result_length;
	weakref_support_init(result);
	rwlock_init(&result->l_lock);
	DeeObject_Init(result, &DeeList_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_elem:
	while (result_length--)
		Dee_Decref(elemv[result_length]);
	Dee_Free(elemv);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_inplace_mul(List **__restrict pself,
                 DeeObject *other) {
	List *self = *pself;
	DREF DeeObject **elemv, **dst;
	size_t i, my_length, result_length, multiplier;
	if (DeeObject_AsSize(other, &multiplier))
		goto err;
	if unlikely(!multiplier) {
		DeeList_Clear((DeeObject *)self);
		goto done;
	}
	if unlikely(multiplier == 1)
		goto done;
again:
	DeeList_LockWrite(self);
	my_length = self->l_size;
	if (OVERFLOW_UMUL(my_length, multiplier, &result_length)) {
		DeeList_LockEndWrite(self);
		err_integer_overflow_i(sizeof(size_t) * 8, true);
		goto err;
	}
	elemv = self->l_elem;
	/* Make sure sufficient memory has been allocated. */
	if (result_length > self->l_alloc) {
		elemv = (DREF DeeObject **)Dee_TryRealloc(elemv,
		                                          result_length *
		                                          sizeof(DREF DeeObject *));
		if unlikely(!elemv) {
			DeeList_LockEndWrite(self);
			if (Dee_CollectMemory(result_length * sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}
		self->l_elem  = elemv;
		self->l_alloc = result_length;
	}
	/* Create new references. */
	--multiplier;
	for (i = 0; i < my_length; ++i)
		Dee_Incref_n(elemv[i], multiplier);
	/* Copy objects to fill the new vector area. */
	dst = elemv + my_length;
	while (multiplier--) {
		memcpyc(dst, elemv, my_length,
		        sizeof(DREF DeeObject *));
		dst += my_length;
	}
	self->l_size = result_length;
	DeeList_LockEndWrite(self);
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
	TYPE_MEMBER_CONST("Iterator", &DeeListIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeTuple_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeList_EqV(List *lhs, DeeObject *const *rhsv, size_t elemc) {
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
		temp = DeeObject_CompareEq(lhs_elem, rhsv[i]);
		Dee_Decref(lhs_elem);
		if (temp <= 0)
			goto err;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 1;
nope:
	DeeList_LockEndRead(lhs);
	return 0;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_EqF(List *lhs, DeeObject *rhs, size_t elemc) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	size_t i;
	int temp;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		if (elemc != DeeList_SIZE(lhs))
			goto nope;
		if (i >= elemc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			return -1;
		}
		temp = DeeObject_CompareEq(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp <= 0)
			goto err;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 1;
nope:
	DeeList_LockEndRead(lhs);
	return 0;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_EqI(List *lhs, DeeObject *rhs) {
	size_t i;
	int temp;
	DREF DeeObject *lhs_elem, *rhs_elem;
	DeeList_LockRead(lhs);
	for (i = 0; i < DeeList_SIZE(lhs); ++i) {
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		rhs_elem = DeeObject_IterNext(rhs);
		if (!ITER_ISOK(rhs_elem)) {
			Dee_Decref(lhs_elem);
			return rhs_elem ? 0 : -1;
		}
		temp = DeeObject_CompareEq(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp <= 0)
			goto return_temp;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	rhs_elem = DeeObject_IterNext(rhs);
	if (rhs_elem != ITER_DONE) {
		if unlikely(!rhs_elem)
			goto err;
		Dee_Decref(rhs_elem);
		return 0;
	}
	return 1;
err:
	temp = -1;
return_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_EqS(List *lhs, DeeObject *seq) {
	int result;
	size_t fast_size;
	if (DeeTuple_Check(seq))
		return DeeList_EqV(lhs, DeeTuple_ELEM(seq), DeeTuple_SIZE(seq));
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST)
		return DeeList_EqF(lhs, seq, fast_size);
	seq = DeeObject_IterSelf(seq);
	if unlikely(!seq)
		goto err;
	result = DeeList_EqI(lhs, seq);
	Dee_Decref(seq);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeList_LoV(List *lhs, DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_elem;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		int temp;
		if (i >= DeeList_SIZE(lhs)) {
			size_t my_size = DeeList_SIZE(lhs);
			DeeList_LockEndRead(lhs);
			return my_size < rhsc;
		}
		if (i >= rhsc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		temp = DeeObject_CompareLo(lhs_elem, rhsv[i]);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0) {
				Dee_Decref(lhs_elem);
				return temp;
			}
			temp = DeeObject_CompareLo(rhsv[i], lhs_elem);
			if (temp != 0) { /* *rhs < *lhs : true */
				Dee_Decref(lhs_elem);
				if unlikely(temp < 0)
					return temp;
				return 0; /* return false */
			}
		}
		Dee_Decref(lhs_elem);
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 0; /* size:greater */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_LoF(List *lhs, DeeObject *rhs, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_elem, *rhs_elem;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		int temp;
		if (i >= DeeList_SIZE(lhs)) {
			size_t my_size = DeeList_SIZE(lhs);
			DeeList_LockEndRead(lhs);
			return my_size < rhsc;
		}
		if (i >= rhsc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			goto err;
		}
		temp = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0) {
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return temp;
			}
			temp = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (temp != 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				if unlikely(temp < 0)
					return temp;
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 0; /* size:greater */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_LoI(List *lhs, DeeObject *rhs) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	size_t i;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		if (i >= DeeList_SIZE(lhs))
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		result = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (result != 0)
			return result;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
		return (unlikely(!rhs_elem))
		       ? -1
		       : 0; /* size:equal */
	Dee_Decref(rhs_elem);
	return 1; /* size:lower */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_LoS(List *lhs, DeeObject *seq) {
	int result;
	size_t fast_size;
	if (DeeTuple_Check(seq))
		return DeeList_LoV(lhs, DeeTuple_ELEM(seq), DeeTuple_SIZE(seq));
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST)
		return DeeList_LoF(lhs, seq, fast_size);
	seq = DeeObject_IterSelf(seq);
	if unlikely(!seq)
		goto err;
	result = DeeList_LoI(lhs, seq);
	Dee_Decref(seq);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeList_LeV(List *lhs, DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_elem;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		int temp;
		if (i >= DeeList_SIZE(lhs)) {
			size_t my_size = DeeList_SIZE(lhs);
			DeeList_LockEndRead(lhs);
			return my_size <= rhsc;
		}
		if (i >= rhsc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		temp = DeeObject_CompareLo(lhs_elem, rhsv[i]);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0) {
				Dee_Decref(lhs_elem);
				return temp;
			}
			temp = DeeObject_CompareLo(rhsv[i], lhs_elem);
			if (temp != 0) { /* *rhs < *lhs : true */
				Dee_Decref(lhs_elem);
				if unlikely(temp < 0)
					return temp;
				return 0; /* return false */
			}
		}
		Dee_Decref(lhs_elem);
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 0; /* size:greater */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_LeF(List *lhs, DeeObject *rhs, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_elem, *rhs_elem;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		int temp;
		if (i >= DeeList_SIZE(lhs)) {
			size_t my_size = DeeList_SIZE(lhs);
			DeeList_LockEndRead(lhs);
			return my_size <= rhsc;
		}
		if (i >= rhsc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			goto err;
		}
		temp = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0) {
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return temp;
			}
			temp = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (temp != 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				if unlikely(temp < 0)
					return temp;
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 0; /* size:greater */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_LeI(List *lhs, DeeObject *rhs) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	size_t i;
	DeeList_LockRead(lhs);
	for (i = 0;; ++i) {
		if (i >= DeeList_SIZE(lhs))
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		result = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (result != 0)
			return result;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return 1; /* size:lower_or_equal */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeList_LeS(List *lhs, DeeObject *seq) {
	int result;
	size_t fast_size;
	if (DeeTuple_Check(seq))
		return DeeList_LeV(lhs, DeeTuple_ELEM(seq), DeeTuple_SIZE(seq));
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST)
		return DeeList_LeF(lhs, seq, fast_size);
	seq = DeeObject_IterSelf(seq);
	if unlikely(!seq)
		goto err;
	result = DeeList_LeI(lhs, seq);
	Dee_Decref(seq);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_eq(List *self, DeeObject *other) {
	int result = DeeList_EqS(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_ne(List *self, DeeObject *other) {
	int result = DeeList_EqS(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_lo(List *self, DeeObject *other) {
	int result = DeeList_LoS(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_le(List *self, DeeObject *other) {
	int result = DeeList_LeS(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_gr(List *self, DeeObject *other) {
	int result = DeeList_LeS(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
list_ge(List *self, DeeObject *other) {
	int result = DeeList_LoS(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}



PRIVATE struct type_cmp list_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_ge,
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
	                         "Create a new List consiting of @size elements, all initialized to @filler\n"

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
	                         "local x = [];\n"
	                         "x.append(10);\n"
	                         "x.append(20);\n"
	                         "x.append(30);\n"
	                         "print repr x; /* `[10, 20, 30]' */"
	                         "}\n"

	                         "\n"
	                         "+(other:?X2?.?S?O)->\n"
	                         "Returns a new List that is the concatenation of @this List and @other\n"

	                         "\n"
	                         "+=(other:?X2?.?S?O)->\n"
	                         "Appends elements from @other to @this List. (Same as ?#extend)\n"

	                         "\n"
	                         "*(count:?Dint)->\n"
	                         "@throw IntegerOverflow @count is negative\n"
	                         "Return a new List containing all the elements of @this one, repeated @count times\n"
	                         "When @count is equal to $0, an empty List is returned\n"

	                         "\n"
	                         "*=(count:?Dint)->\n"
	                         "@throw IntegerOverflow @count is negative\n"
	                         "Extend @this List to contain @count as many items, filling the new slots with repetitions of all pre-existing items\n"
	                         "When @count is equal to $0, the operator behaves the same as ?#clear\n"

	                         "\n"
	                         "#->\n"
	                         "Returns the number of items contained within @this List\n"

	                         "\n"
	                         "[](index:?Dint)->\n"
	                         "@throw IndexError @index is out of bounds\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Return the @index'th item of @this List\n"

	                         "\n"
	                         "del[](index:?Dint)->\n"
	                         "@throw IndexError @index is out of bounds\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Delete the @index'th item from @this List. (same as ${this.erase(index, 1)})\n"

	                         "\n"
	                         "[]=(index:?Dint,ob)->\n"
	                         "@throw IndexError @index is out of bounds\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Replace the @index'th item of @this List with @ob\n"

	                         "\n"
	                         "[:](start:?Dint,end:?Dint)->\n"
	                         "@throw IntegerOverflow @start or @end are too large\n"
	                         "Return a new List containing the elements within the range @{start}...@end\n"
	                         "If either @start or @end are negative, ${##this} is added first.\n"
	                         "If following this, either is greater than ${##this}, it is clampled to that value\n"

	                         "\n"
	                         "del[:](start:?Dint,end:?Dint)->\n"
	                         "@throw IntegerOverflow @start or @end are too large\n"
	                         "Using the same index-rules as for ?#{op:getrange}, delete all items from that range\n"

	                         "\n"
	                         "[:]=(start:?Dint,end:?Dint,items:?S?O)->\n"
	                         "@throw NotImplemented The given @items cannot be iterated\n"
	                         "@throw IntegerOverflow @start or @end are too large\n"
	                         "Using the same index-rules as for ?#{op:getrange}, delete all items from that range "
	                         "before inserting all elements from @items at the range's start. - This operation "
	                         "is performed atomically, and @this List is not modified if @items cannot be iterated\n"

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
	                         "@throw NotImplemented The given @other cannot be iterated\n"
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
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&list_bool
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
	/* .tp_class_members = */ list_class_members
};



#ifdef CONFIG_NO_THREADS
#define LI_GETINDEX(x)            ((x)->li_index)
#else /* CONFIG_NO_THREADS */
#define LI_GETINDEX(x) ATOMIC_READ((x)->li_index)
#endif /* !CONFIG_NO_THREADS */


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
	if (DeeArg_Unpack(argc, argv, "o|Iu:_ListIterator", &self->li_list, &self->li_index))
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
#ifdef CONFIG_NO_THREADS
	DREF DeeObject *result;
	if (self->li_index >= self->li_list->l_size)
		return ITER_DONE;
	result = self->li_list->l_elem[self->li_index++];
	Dee_Incref(result);
	return result;
#else /* CONFIG_NO_THREADS */
	size_t list_index;
	DREF DeeObject *result;
again:
	list_index = ATOMIC_READ(self->li_index);
	DeeList_LockRead(self->li_list);
	if (list_index >= self->li_list->l_size) {
		DeeList_LockEndRead(self->li_list);
		return ITER_DONE;
	}
	result = self->li_list->l_elem[list_index];
	Dee_Incref(result);
	DeeList_LockEndRead(self->li_list);
	if (!ATOMIC_CMPXCH(self->li_index, list_index, list_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
#endif /* !CONFIG_NO_THREADS */
}


#define DEFINE_LIST_ITERATOR_COMPARE(name, expr)                                  \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                         \
	name(ListIterator *self, ListIterator *other) {                               \
		if (DeeObject_AssertTypeExact(other, &DeeListIterator_Type)) \
			goto err;                                                             \
		return_bool(expr);                                                        \
	err:                                                                          \
		return NULL;                                                              \
	}
DEFINE_LIST_ITERATOR_COMPARE(li_eq,
                             self->li_list == other->li_list &&
                             LI_GETINDEX(self) == LI_GETINDEX(other))
DEFINE_LIST_ITERATOR_COMPARE(li_ne,
                             self->li_list != other->li_list ||
                             LI_GETINDEX(self) != LI_GETINDEX(other))
DEFINE_LIST_ITERATOR_COMPARE(li_lo,
                             self->li_list < other->li_list ||
                             (self->li_list == other->li_list &&
                                     LI_GETINDEX(self) < LI_GETINDEX(other)))
DEFINE_LIST_ITERATOR_COMPARE(li_le,
                             self->li_list < other->li_list ||
                             (self->li_list == other->li_list &&
                                     LI_GETINDEX(self) <= LI_GETINDEX(other)))
DEFINE_LIST_ITERATOR_COMPARE(li_gr,
                             self->li_list > other->li_list ||
                             (self->li_list == other->li_list &&
                                     LI_GETINDEX(self) > LI_GETINDEX(other)))
DEFINE_LIST_ITERATOR_COMPARE(li_ge,
                             self->li_list > other->li_list ||
                             (self->li_list == other->li_list &&
                                     LI_GETINDEX(self) >= LI_GETINDEX(other)))
#undef DEFINE_LIST_ITERATOR_COMPARE

PRIVATE WUNUSED NONNULL((1)) DREF DeeListObject *DCALL
list_iterator_nii_getseq(ListIterator *__restrict self) {
	return_reference_(self->li_list);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_iterator_nii_getindex(ListIterator *__restrict self) {
	return LI_GETINDEX(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_setindex(ListIterator *__restrict self, size_t new_index) {
#ifdef CONFIG_NO_THREADS
	self->li_index = new_index;
#else /* CONFIG_NO_THREADS */
	ATOMIC_WRITE(self->li_index, new_index);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_rewind(ListIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	self->li_index = 0;
#else /* CONFIG_NO_THREADS */
	ATOMIC_WRITE(self->li_index, 0);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_revert(ListIterator *__restrict self, size_t step) {
#ifdef CONFIG_NO_THREADS
	if (OVERFLOW_USUB(self->li_index, step, &self->li_index))
		self->li_index = 0;
#else /* CONFIG_NO_THREADS */
	size_t old_index, new_index;
	do {
		old_index = ATOMIC_READ(self->li_index);
		if (OVERFLOW_USUB(old_index, step, &new_index))
			new_index = 0;
	} while (!ATOMIC_CMPXCH_WEAK(self->li_index, old_index, new_index));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_advance(ListIterator *__restrict self, size_t step) {
#ifdef CONFIG_NO_THREADS
	if (OVERFLOW_UADD(self->li_index, step, &self->li_index))
		self->li_index = (size_t)-1;
#else /* CONFIG_NO_THREADS */
	size_t old_index, new_index;
	do {
		old_index = ATOMIC_READ(self->li_index);
		if (OVERFLOW_UADD(old_index, step, &new_index))
			new_index = (size_t)-1;
	} while (!ATOMIC_CMPXCH_WEAK(self->li_index, old_index, new_index));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_prev(ListIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (!self->li_index)
		return 1;
	--self->li_index;
#else /* CONFIG_NO_THREADS */
	size_t old_index;
	do {
		old_index = ATOMIC_READ(self->li_index);
		if (!old_index)
			return 1;
	} while (!ATOMIC_CMPXCH_WEAK(self->li_index, old_index, old_index - 1));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_next(ListIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (self->li_index >= DeeList_SIZE(self->li_list))
		return 1;
	++self->li_index;
#else /* CONFIG_NO_THREADS */
	size_t old_index;
	do {
		old_index = ATOMIC_READ(self->li_index);
		if (old_index >= DeeList_SIZE(self->li_list))
			return 1;
	} while (!ATOMIC_CMPXCH_WEAK(self->li_index, old_index, old_index + 1));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_iterator_nii_hasprev(ListIterator *__restrict self) {
	return LI_GETINDEX(self) != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_iterator_nii_peek(ListIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	DREF DeeObject *result;
	if (self->li_index >= self->li_list->l_size)
		return ITER_DONE;
	result = self->li_list->l_elem[self->li_index];
	Dee_Incref(result);
	return result;
#else /* CONFIG_NO_THREADS */
	size_t list_index;
	DREF DeeObject *result;
	list_index = ATOMIC_READ(self->li_index);
	DeeList_LockRead(self->li_list);
	if (list_index >= self->li_list->l_size) {
		DeeList_LockEndRead(self->li_list);
		return ITER_DONE;
	}
	result = self->li_list->l_elem[list_index];
	Dee_Incref(result);
	DeeList_LockEndRead(self->li_list);
	return result;
#endif /* !CONFIG_NO_THREADS */
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
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&li_ge,
	/* .tp_nii  = */ &list_iterator_nii
};

PRIVATE struct type_member tpconst li_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(ListIterator, li_list), "->?DList"),
	TYPE_MEMBER_FIELD("index", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(ListIterator, li_index)),
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
