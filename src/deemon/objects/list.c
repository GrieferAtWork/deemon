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
#ifndef GUARD_DEEMON_OBJECTS_LIST_C
#define GUARD_DEEMON_OBJECTS_LIST_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FreeTracker, DeeObject_MALLOC, Dee_*alloc*, Dee_BadAlloc, Dee_CollectMemory*, Dee_Freea, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_XFreea */
#include <deemon/arg.h>                /* DeeArg_Unpack0, DeeArg_UnpackStruct*, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>               /* Dee_True, return_bool, return_false, return_true */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_Throwf, DeeError_ValueError */
#include <deemon/format.h>             /* PRFuSIZ */
#include <deemon/gc.h>                 /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK, DeeGC_Track, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/int.h>                /* DeeInt_Check, DeeInt_NewSize */
#include <deemon/list.h>               /* DeeListObject, DeeList_*, _DeeList_SetAlloc */
#include <deemon/method-hints.h>       /* DeeMA_*, Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* DeeNone_Check, Dee_None, return_none */
#include <deemon/object.h>             /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_FROMBOOL, Dee_COMPARE_*, Dee_Clear, Dee_Compare, Dee_Decref*, Dee_Incref*, Dee_Movrefv, Dee_Setrefv, Dee_TYPE, Dee_WEAKREF_SUPPORT_ADDR, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compareT, Dee_return_compare_if_ne, Dee_ssize_t, Dee_weakref_support_fini, Dee_weakref_support_init, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_*, Dee_seq_range */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/system-features.h>    /* memcpyc, memmovec, memmovedownc, memmoveupc, mempcpyc */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/type.h>               /* DeeObject_Init, DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_Visitv, Dee_visit_t, METHOD_FNOREFESCAPE, METHOD_FNOTHROW, OPERATOR_*, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */
#include <deemon/util/hash.h>          /* Dee_HASHOF_EMPTY_SEQUENCE, Dee_HashCombine, Dee_HashPointer */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */
#include <deemon/util/objectlist.h>    /* Dee_OBJECTLIST_*, Dee_objectlist, Dee_objectlist_* */

#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */
#include <hybrid/overflow.h>  /* OVERFLOW_* */

#include "../runtime/kwlist.h"
#include "../runtime/method-hint-defaults.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/sort.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

/* Returns true if `tp_asvector_nothrow' of `ob' may
 * attempt to acquire a lock to `DeeList_Lock*(list)'. */
#define DeeObject_IsAsVectorNoThrowUsingLockOfList(ob, list) \
	((DeeObject *)(list) == (DeeObject *)(ob))


typedef DeeListObject List;

typedef struct {
	PROXY_OBJECT_HEAD_EX(List, li_list); /* [1..1][const] The list being iterated. */
	DWEAK size_t               li_index; /* The current iteration index. */
} ListIterator;

INTDEF DeeTypeObject DeeListIterator_Type;


PRIVATE NONNULL((1)) void DCALL
list_fini(List *__restrict me) {
	Dee_weakref_support_fini(me);
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
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
	newlist.ol_elemc = DeeSeq_AsHeapVectorWithAllocReuse(other, &newlist.ol_elemv, &newlist.ol_elema);
#else /* Dee_OBJECTLIST_HAVE_ELEMA */
	newlist.ol_elemc = DeeSeq_AsHeapVectorWithAllocReuse2(other, &newlist.ol_elemv);
#endif /* !Dee_OBJECTLIST_HAVE_ELEMA */

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
		Dee_objectlist_elemv_free(old_elemv);
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
		Dee_objectlist_elemv_free(old_elemv);
	}
done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_ctor(List *__restrict self) {
	Dee_weakref_support_init(self);
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
	Dee_weakref_support_init(me);
	Dee_atomic_rwlock_init(&me->l_lock);
again:
	DeeList_LockRead(other);
	count  = other->l_list.ol_elemc;
	new_elemv = Dee_objectlist_elemv_trymalloc(count);
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

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewWithHint(size_t n_prealloc) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeList_Type);
	_DeeList_SetAlloc(result, n_prealloc);
	result->l_list.ol_elemc  = 0;
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	if likely(n_prealloc) {
		result->l_list.ol_elemv = Dee_objectlist_elemv_trymalloc_safe(n_prealloc);
		if unlikely(!result->l_list.ol_elemv)
			_DeeList_SetAlloc(result, 0);
	} else {
		result->l_list.ol_elemv = NULL;
	}
	return Dee_AsObject(DeeGC_TRACK(List, result));
err:
	return NULL;
}

PUBLIC WUNUSED DREF List *DCALL
DeeList_NewUninitialized(size_t n_elem) {
	DREF List *result;
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto done;
	result->l_list.ol_elemv = Dee_objectlist_elemv_malloc_safe(n_elem);
	if unlikely(!result->l_list.ol_elemv)
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	_DeeList_SetAlloc(result, n_elem);
	result->l_list.ol_elemc  = n_elem;
	Dee_weakref_support_init(result);
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
	Dee_objectlist_elemv_free(self->l_list.ol_elemv);
	DeeObject_FreeTracker(Dee_AsObject(self));
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
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	DeeObject_Init(result, &DeeList_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
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
	elemv = Dee_objectlist_elemv_malloc(elemc);
	if unlikely(!elemv)
		goto err_r;
	elemv = Dee_Movrefv(elemv, DeeTuple_ELEM(self), elemc);
	result->l_list.ol_elemv = elemv;
	result->l_list.ol_elemc = elemc;
	_DeeList_SetAlloc(result, elemc);
	DeeObject_Init(result, &DeeList_Type);
	Dee_weakref_support_init(result);
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
	return Dee_AsObject(result);
}

/* Inherit the entire vector, which must have been allocated using `Dee_Malloc()' and friends. */
#ifndef Dee_OBJECTLIST_HAVE_ELEMA
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(/*inherit(on_success)*/ DREF DeeObject **objv,
                               size_t objc, size_t obja) {
	ASSERT(obja <= Dee_objectlist_elemv_usable_size(objv));
	(void)obja;
	return DeeList_NewVectorInheritedHeap2(objv, objc);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap2(/*inherit(on_success)*/ DREF DeeObject **objv, size_t objc)
#else /* !Dee_OBJECTLIST_HAVE_ELEMA */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap2(/*inherit(on_success)*/ DREF DeeObject **objv, size_t objc) {
	/* For binary compatibility... */
	return DeeList_NewVectorInheritedHeap(objv, objc, objc);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(/*inherit(on_success)*/ DREF DeeObject **objv,
                               size_t objc, size_t obja)
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */
{
	DREF List *result;
#ifndef Dee_OBJECTLIST_HAVE_ELEMA
#ifdef Dee_MallocUsableSize /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	ASSERT(objc <= Dee_objectlist_elemv_usable_size(objv));
#endif /* Dee_MallocUsableSize */
	ASSERT(objv || !objc);
#else /* !Dee_OBJECTLIST_HAVE_ELEMA */
	ASSERT(objc <= obja);
	ASSERT(objv || (!obja && !objc));
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	result->l_list.ol_elemv = objv; /* Inherit */
	result->l_list.ol_elemc = objc;
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
	_DeeList_SetAlloc(result, obja);
#elif defined(Dee_MallocUsableSize) /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	_DeeList_SetAlloc(result, Dee_objectlist_elemv_usable_size(objv));
#else /* ... */
	_DeeList_SetAlloc(result, objc);
#endif /* !... */
	DeeObject_Init(result, &DeeList_Type);
	Dee_weakref_support_init(result);
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

INTERN WUNUSED DREF List *DCALL /* Used in "../runtime/accu.c" */
DeeList_Copy(List *__restrict self) {
	DREF List *result;
	ASSERT_OBJECT_TYPE(self, &DeeList_Type);
	result = DeeGCObject_MALLOC(List);
	if unlikely(!result)
		goto err;
	if unlikely(list_copy(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeList_Type);
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	return DeeGC_TRACK(List, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


/* Concat a list and some generic sequence,
 * inheriting a reference from `self' in the process. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeList_ConcatInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *sequence) {
	DREF DeeObject *result;
	if (!DeeObject_IsShared(self)) {
		/* Simple case: can append onto the original list. */
		if unlikely(DeeList_AppendSequence(self, sequence))
			goto err_self;
		return self;
	}

	/* Fallback: Copy the list and append. */
	/* XXX: We can do better here: by combining `DeeList_SIZE_ATOMIC(self)' and
	 *      `DeeObject_SizeFast(sequence)', we can get a snapshot of what the final
	 *      list's length will be, which can then safe us one realloc() that's
	 *      needed in `DeeList_AppendSequence()' at the moment to resize the list
	 *      before appending "sequence".
	 * This doesn't even need to be complicated; we can just use the prealloc
	 * mechanism and have `DeeList_Copy()' prealloc sufficient space for at least
	 * `DeeObject_SizeFast(sequence)' trailing objects. */
	result = Dee_AsObject(DeeList_Copy((List *)self));
	Dee_Decref_unlikely(self);
	if unlikely(!result)
		goto err;
	if unlikely(DeeList_AppendSequence(result, sequence))
		Dee_Clear(result);
	return result;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_ExtendInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc,
                        /*inherit(always)*/ DREF DeeObject *const *argv) {
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
			size_t new_elema = Dee_OBJECTLIST_MOREALLOC(old_elema);
			if (new_elema < req_alloc)
				new_elema = req_alloc;
			if (new_elema < Dee_OBJECTLIST_MINALLOC)
				new_elema = Dee_OBJECTLIST_MINALLOC;
do_realloc_vector:
			new_elemv = Dee_objectlist_elemv_tryrealloc(result->l_list.ol_elemv, new_elema);
			if unlikely(!new_elemv) {
				if (new_elema > req_alloc) {
					new_elema = req_alloc;
					goto do_realloc_vector;
				}
				if (Dee_CollectMemoryc(new_elema, sizeof(DREF DeeObject *)))
					goto do_realloc_vector;
				goto err_me_argv;
			}
			result->l_list.ol_elemv  = new_elemv;
			_DeeList_SetAlloc(result, new_elema);
		}
		memcpyc(result->l_list.ol_elemv + result->l_list.ol_elemc,
		        argv, argc, sizeof(DREF DeeObject *));
		result->l_list.ol_elemc += argc;
	} else {
		DREF DeeObject **new_elemv;
		size_t list_size;
		list_size = DeeList_SIZE_ATOMIC(me);
allocate_new_vector:
		new_elemv = Dee_objectlist_elemv_malloc(list_size + argc);
		if unlikely(!new_elemv)
			goto err_me_argv;
		DeeList_LockRead(me);
		if unlikely(DeeList_SIZE(me) != list_size) {
			list_size = DeeList_SIZE(me);
			DeeList_LockEndRead(me);
			Dee_objectlist_elemv_free(new_elemv);
			goto allocate_new_vector;
		}
		Dee_Movrefv(new_elemv, DeeList_ELEM(me), list_size);
		DeeList_LockEndRead(me);

		/* Create the new list descriptor. */
		result = DeeGCObject_MALLOC(List);
		if unlikely(!result) {
			Dee_Decrefv(new_elemv, list_size);
			Dee_objectlist_elemv_free(new_elemv);
			goto err_me_argv;
		}
		memcpyc(new_elemv + list_size, argv,
		        argc, sizeof(DREF DeeObject *));
		DeeObject_Init(result, &DeeList_Type);
		result->l_list.ol_elemv = new_elemv;
		result->l_list.ol_elemc = list_size + argc;
		_DeeList_SetAlloc(result, list_size + argc);
		Dee_weakref_support_init(result);
		Dee_atomic_rwlock_init(&result->l_lock);
		result = DeeGC_TRACK(List, result);
	}
	return Dee_AsObject(result);
err_me_argv:
	Dee_Decrefv(argv, argc);
	Dee_Decref(me);
	return NULL;
}


/* @return: * :   The popped element.
 * @return: NULL: The given index was out-of-bounds and an IndexError was thrown. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_Pop(DeeObject *__restrict self, Dee_ssize_t index) {
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
		DeeRT_ErrIndexOutOfBounds((DeeObject *)me, (size_t)index, length);
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

PUBLIC WUNUSED NONNULL((1)) int DCALL
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
			goto err;
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
	return 0;
err:
	return -1;
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
			new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, newsize);
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
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
		size_t old_elema;
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */

		/* Remove everything */
		old_elemv = me->l_list.ol_elemv;
		old_elemc = me->l_list.ol_elemc;
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
		old_elema = DeeList_GetAlloc(me);
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */
		Dee_objectlist_init(&me->l_list);
		DeeList_LockEndWrite(me);
		Dee_Decrefv(old_elemv, old_elemc);
		DeeList_LockWrite(me);
		if likely(!me->l_list.ol_elemv) {
			ASSERT(me->l_list.ol_elemc == 0);

			/* Allow the list to re-use its old vector. */
			me->l_list.ol_elemv = old_elemv;
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
			_DeeList_SetAlloc(me, old_elema);
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */
			old_elemv = NULL; /* Inherit the old vector. */
		}
		DeeList_LockEndWrite(me);
		/* Free the old vector. */
		Dee_objectlist_elemv_free(old_elemv);
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
PUBLIC WUNUSED NONNULL((1, 2)) size_t DCALL
DeeList_RemoveIf(DeeObject *self, DeeObject *should,
                 size_t start, size_t end, size_t max) {
	List *me = (List *)self;
	DeeObject **vector;
	size_t i, length, result = 0;
	ASSERT_OBJECT_TYPE(me, &DeeList_Type);
	if unlikely(!max)
		goto done;
	DeeList_LockRead(me);
again:
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
		temp = DeeObject_BoolInherited(callback_result);
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
			DeeList_LockEndWrite(me);
			++result;

			/* Drop the reference previously held by the list. */
			Dee_Decref(this_elem);
			if unlikely(result >= max)
				goto done;
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
done:
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
	DREF DeeObject **new_elemv;
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
		new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
		if (new_elema < Dee_OBJECTLIST_MINALLOC)
			new_elema = Dee_OBJECTLIST_MINALLOC;
		new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
		if unlikely(!new_elemv) {
			/* Try again, but only attempt to allocate for a single object. */
			new_elema = me->l_list.ol_elemc + 1;
			new_elemv = Dee_objectlist_elemv_realloc(me->l_list.ol_elemv, new_elema);
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
	DREF DeeObject **new_elemv;
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
		if (new_elema < Dee_OBJECTLIST_MINALLOC)
			new_elema = Dee_OBJECTLIST_MINALLOC;
		while (new_elema < old_elemc + objc)
			new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
		new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
		if unlikely(!new_elemv) {
			/* Try again, but only attempt to allocate what we need. */
			new_elema = old_elemc + objc;
			new_elemv = Dee_objectlist_elemv_realloc(me->l_list.ol_elemv, new_elema);
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
		size_t new_elema = Dee_OBJECTLIST_MOREALLOC(old_elema);
		DREF DeeObject **new_elemv;
		if (new_elema < Dee_OBJECTLIST_MINALLOC)
			new_elema = Dee_OBJECTLIST_MINALLOC;
do_realloc:
		new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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
		size_t new_elema = Dee_OBJECTLIST_MOREALLOC(old_elema);
		DREF DeeObject **new_elemv;
		if (new_elema < Dee_OBJECTLIST_MINALLOC)
			new_elema = Dee_OBJECTLIST_MINALLOC;
		while (new_elema < me->l_list.ol_elemc + objc)
			new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
do_realloc:
		new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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


PRIVATE WUNUSED NONNULL((1)) int DCALL
list_init(List *__restrict self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("List", params: "
	DeeObject *sequence_or_length: ?X2?S?O?Dint;
	DeeObject *filler = NULL;
");]]]*/
	struct {
		DeeObject *sequence_or_length;
		DeeObject *filler;
	} args;
	args.filler = NULL;
	DeeArg_UnpackStruct1Or2(err, argc, argv, "List", &args, &args.sequence_or_length, &args.filler);
/*[[[end]]]*/
	Dee_weakref_support_init(self);
	Dee_atomic_rwlock_init(&self->l_lock);
	if (args.filler || DeeInt_Check(args.sequence_or_length)) {
		size_t list_size;
		if (DeeObject_AsSize(args.sequence_or_length, &list_size))
			goto err;
		if (!list_size) {
			self->l_list.ol_elemv = NULL;
		} else {
			if (args.filler == NULL)
				args.filler = Dee_None;
			self->l_list.ol_elemv = Dee_objectlist_elemv_malloc_safe(list_size);
			if unlikely(!self->l_list.ol_elemv)
				goto err;
			Dee_Setrefv(self->l_list.ol_elemv, args.filler, list_size);
		}
		self->l_list.ol_elemc = list_size;
		_DeeList_SetAlloc(self, list_size);
		return 0;
	}

	/* Fallback: initialize from a generic sequence. */
	return Dee_objectlist_init_fromseq(&self->l_list, args.sequence_or_length);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_serialize(List *__restrict self,
               DeeSerial *__restrict writer,
               Dee_seraddr_t addr) {
	List *out;
	size_t sizeof_list;
	size_t out__l_list_ol_elemc;
	size_t addrof_out__l_list_ol_elemv;
	DREF DeeObject **in__l_list_ol_elemv;
	DREF DeeObject **out__l_list_ol_elemv;
again:
	out = DeeSerial_Addr2Mem(writer, addr, List);
	Dee_weakref_support_init(out);
	DeeList_LockRead(self);
	out->l_list.ol_elemc = self->l_list.ol_elemc;
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
	out->l_list.ol_elema = out->l_list.ol_elemc;
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */
	Dee_atomic_rwlock_init(&out->l_lock);
	if (self->l_list.ol_elemv == NULL) {
		DeeList_LockEndRead(self);
		out->l_list.ol_elemv = NULL;
		return 0;
	}
	out__l_list_ol_elemc = out->l_list.ol_elemc;
	sizeof_list = out__l_list_ol_elemc * sizeof(DREF DeeObject *);
	addrof_out__l_list_ol_elemv = DeeSerial_TryMalloc(writer, sizeof_list, NULL);
	if (!Dee_SERADDR_ISOK(addrof_out__l_list_ol_elemv)) {
		DeeList_LockEndRead(self);
		addrof_out__l_list_ol_elemv = DeeSerial_Malloc(writer, sizeof_list, NULL);
		if (!Dee_SERADDR_ISOK(addrof_out__l_list_ol_elemv))
			goto err;
		DeeList_LockRead(self);
		if unlikely(out__l_list_ol_elemc != self->l_list.ol_elemc) {
			DeeList_LockEndRead(self);
			goto again;
		}
	}
//	out = DeeSerial_Addr2Mem(writer, addr, List);
	out__l_list_ol_elemv = DeeSerial_Addr2Mem(writer, addrof_out__l_list_ol_elemv, DREF DeeObject *);
	in__l_list_ol_elemv  = self->l_list.ol_elemv;
	Dee_Movrefv(out__l_list_ol_elemv, in__l_list_ol_elemv, out__l_list_ol_elemc);
	DeeList_LockEndRead(self);
	if (DeeSerial_InplacePutObjectv(writer,
	                                addrof_out__l_list_ol_elemv,
	                                out__l_list_ol_elemc))
		goto err;
	return DeeSerial_PutAddr(writer,
	                         addr + offsetof(List, l_list.ol_elemv),
	                         addrof_out__l_list_ol_elemv);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
list_visit(List *__restrict self,
           Dee_visit_t proc, void *arg) {
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
	Dee_objectlist_elemv_free(elemv);
	return elemc != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
list_printrepr(List *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	temp = (*printer)(arg, "[", 1);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	DeeList_LockRead(self);
	for (i = 0; i < self->l_list.ol_elemc; ++i) {
		DREF DeeObject *elem = self->l_list.ol_elemv[i];
		Dee_Incref(elem);
		DeeList_LockEndRead(self);
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
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
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
		if (Dee_COMPARE_ISEQ_NO_ERR(error))
			return_true;
		if (Dee_COMPARE_ISERR(error))
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
	if unlikely(DeeObject_AsSize(index, &i)) {
		DeeRT_ErrIndexOverflow((DeeObject *)me);
		goto err;
	}
	DeeList_LockRead(me);
	if unlikely(i >= DeeList_SIZE(me)) {
		size_t list_size = DeeList_SIZE(me);
		DeeList_LockEndRead(me);
		DeeRT_ErrIndexOutOfBounds((DeeObject *)me, i, list_size);
		goto err;
	}
	result = DeeList_GET(me, i);
	Dee_Incref(result);
	DeeList_LockEndRead(me);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_index(List *__restrict me, Dee_ssize_t i_begin, Dee_ssize_t i_end) {
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
	new_elemv = Dee_objectlist_elemv_trymalloc(range_size);
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
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	result->l_list.ol_elemv = new_elemv;

	/* Start tracking it as a GC object. */
	return DeeGC_Track((DeeObject *)result);

err_elemv:
	/* Cleanup on error. */
	Dee_Decrefv(new_elemv, range_size);
	Dee_objectlist_elemv_free(new_elemv);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_index_n(List *__restrict me, Dee_ssize_t i_begin) {
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
	new_elemv = Dee_objectlist_elemv_trymalloc(range_size);
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
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	result->l_list.ol_elemv = new_elemv;

	/* Start tracking it as a GC object. */
	return DeeGC_Track((DeeObject *)result);

err_elemv:
	/* Cleanup on error. */
	Dee_Decrefv(new_elemv, range_size);
	Dee_objectlist_elemv_free(new_elemv);
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
list_getrange(List *me, DeeObject *begin, DeeObject *end) {
	Dee_ssize_t i_begin, i_end;
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
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
	return DeeRT_ErrIndexOutOfBounds((DeeObject *)me, index, length);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_delitem(List *me, DeeObject *index) {
	size_t i;
	if unlikely(DeeObject_AsSize(index, &i)) {
		DeeRT_ErrIndexOverflow((DeeObject *)me);
		goto err;
	}
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
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)me, index, length);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_hasitem_index(List *me, size_t index) {
	return index < DeeList_SIZE_ATOMIC(me) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
list_setitem(List *me, DeeObject *index, DeeObject *value) {
	size_t i;
	if unlikely(DeeObject_AsSize(index, &i)) {
		DeeRT_ErrIndexOverflow((DeeObject *)me);
		goto err;
	}
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
list_delrange_index(List *__restrict me, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject **delete_v;
	struct Dee_seq_range range;
	size_t delete_c;
again:
	DeeList_LockWrite(me);
	DeeSeqRange_Clamp(&range, start, end, DeeList_SIZE(me));
	delete_c = range.sr_end - range.sr_start;
	if unlikely(delete_c <= 0)
		goto done_noop;
	delete_v = (DREF DeeObject **)Dee_TryMallocac(delete_c, sizeof(DREF DeeObject *));
	if unlikely(!delete_v) {
		DeeList_LockEndWrite(me);
		if (Dee_CollectMemoryc(delete_c, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}

	/* Move all items to-be deleted into the delete-vector. */
	memcpyc(delete_v, DeeList_ELEM(me) + range.sr_start,
	        delete_c, sizeof(DREF DeeObject *));
	memmovedownc(DeeList_ELEM(me) + range.sr_start,
	             DeeList_ELEM(me) + range.sr_end,
	             DeeList_SIZE(me) - range.sr_end,
	             sizeof(DREF DeeObject *));
	me->l_list.ol_elemc -= delete_c;
	DeeList_LockEndWrite(me);

	/* Drop object references. */
	Dee_Decrefv(delete_v, delete_c);

	/* Free the temporary del-item vector. */
	Dee_Freea(delete_v);
done:
	return 0;
err:
	return -1;
done_noop:
	DeeList_LockEndWrite(me);
	goto done;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_delrange_index_n(List *__restrict me, Dee_ssize_t start) {
#ifdef __OPTIMIZE_SIZE__
	return list_delrange_index(me, start, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject **delete_v;
	size_t start_index, delete_c;
again:
	DeeList_LockWrite(me);
	start_index = DeeSeqRange_Clamp_n(start, DeeList_SIZE(me));
	delete_c = DeeList_SIZE(me) - start_index;
	if unlikely(delete_c <= 0)
		goto done_noop;
	delete_v = (DREF DeeObject **)Dee_TryMallocac(delete_c, sizeof(DREF DeeObject *));
	if unlikely(!delete_v) {
		DeeList_LockEndWrite(me);
		if (Dee_CollectMemoryc(delete_c, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}

	/* Move all items to-be deleted into the delete-vector. */
	memcpyc(delete_v, DeeList_ELEM(me) + start_index,
	        delete_c, sizeof(DREF DeeObject *));
	me->l_list.ol_elemc -= delete_c;
	DeeList_LockEndWrite(me);

	/* Drop object references. */
	Dee_Decrefv(delete_v, delete_c);

	/* Free the temporary del-item vector. */
	Dee_Freea(delete_v);
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
list_setrange_index(List *me, Dee_ssize_t start,
                    Dee_ssize_t end, DeeObject *items) {
	DREF DeeObject **delete_v;
	DREF DeeObject **insert_v;
	size_t delete_c, insert_c;
	struct Dee_seq_range range;

	/* Check if "items" can be appended in-place. */
	if (Dee_TYPE(items)->tp_seq) {
		size_t items_size_fast;
		size_t (DCALL *tp_asvector_nothrow)(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
		tp_asvector_nothrow = Dee_TYPE(items)->tp_seq->tp_asvector_nothrow;
		if (tp_asvector_nothrow && (items_size_fast = DeeObject_SizeFast(items)) != (size_t)-1) {
			if (items_size_fast == 0)
				return list_delrange_index(me, start, end);

			if (tp_asvector_nothrow != NULL && !DeeObject_IsAsVectorNoThrowUsingLockOfList(me, items)) {
				/* Super-fast case: can transfer objects directly while holding a lock to "me" */
				size_t items_size, num_trailing;
				size_t cur_elema, min_elema;
again_with_tp_asvector_nothrow:
				DeeList_LockWrite(me);
				DeeSeqRange_Clamp(&range, start, end, me->l_list.ol_elemc);
				delete_c = range.sr_end - range.sr_start;
				if (OVERFLOW_UADD(me->l_list.ol_elemc - delete_c, items_size_fast, &min_elema))
					goto err_badalloc_overflow_unlock;
				cur_elema = DeeList_GetAlloc(me);
				if unlikely(min_elema > cur_elema) {
					/* Need a larger list buffer. */
					DREF DeeObject **new_listv;
					size_t new_elema = cur_elema;
					if (new_elema < Dee_OBJECTLIST_MINALLOC)
						new_elema = Dee_OBJECTLIST_MINALLOC;
					do {
						new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
					} while (new_elema < min_elema);
					new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
					if unlikely(!new_listv) {
						new_elema = min_elema;
						new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
						if unlikely(!new_listv) {
							DREF DeeObject **old_listv;
							DeeList_LockEndWrite(me);
							new_listv = Dee_objectlist_elemv_malloc(new_elema);
							if unlikely(!new_listv)
								goto err;
							DeeList_LockWrite(me);
							cur_elema = DeeList_GetAlloc(me);
							if likely(new_elema > cur_elema) {
								old_listv = me->l_list.ol_elemv;
								me->l_list.ol_elemv = (DREF DeeObject **)memcpyc(new_listv, old_listv,
								                                                 me->l_list.ol_elemc,
								                                                 sizeof(DREF DeeObject *));
							} else {
								old_listv = new_listv;
							}
							DeeList_LockEndWrite(me);
							Dee_objectlist_elemv_free(old_listv);
							goto again_with_tp_asvector_nothrow;
						}
					}
					me->l_list.ol_elemv = new_listv;
					_DeeList_SetAlloc(me, new_elema);
				}
	
				/* If elements should be removed, remember their references now */
				if (delete_c > 0) {
					delete_v = (DREF DeeObject **)Dee_TryMallocac(delete_c, sizeof(DREF DeeObject *));
					if unlikely(!delete_v) {
						struct Dee_seq_range new_range;
						DeeList_LockEndWrite(me);
						delete_v = (DREF DeeObject **)Dee_Mallocac(delete_c, sizeof(DREF DeeObject *));
						if unlikely(!delete_v)
							goto err;
						DeeList_LockWrite(me);
						/* Validate that nothing about "me" changed. */
						DeeSeqRange_Clamp(&new_range, start, end, me->l_list.ol_elemc);
						if unlikely(range.sr_start != new_range.sr_start ||
						            range.sr_end != new_range.sr_end ||
						            cur_elema != DeeList_GetAlloc(me) ||
						            delete_c != (range.sr_end - range.sr_start) ||
						            min_elema != (me->l_list.ol_elemc - delete_c + items_size_fast)) {
							DeeList_LockEndWrite(me);
							Dee_Freea(delete_v);
							goto again_with_tp_asvector_nothrow;
						}
					}
					memcpyc(delete_v, me->l_list.ol_elemv + range.sr_start,
					        delete_c, sizeof(DREF DeeObject *));
				} else {
					delete_v = NULL;
				}

				/* Shift trailing objects such that they appear at their final positions. */
				num_trailing = me->l_list.ol_elemc - range.sr_end;
				if (/*num_trailing &&*/ delete_c != items_size_fast) {
					memmovec(me->l_list.ol_elemv + range.sr_start + items_size_fast,
					         me->l_list.ol_elemv + range.sr_end,
					         num_trailing, sizeof(DREF DeeObject *));
				}
	
				/* Buffer should have sufficient size now. */
				items_size = (*tp_asvector_nothrow)(items, items_size_fast,
				                                    me->l_list.ol_elemv + range.sr_start);
				if unlikely(items_size > items_size_fast) {
					/* Buffer is too small, and we didn't get references :( */
					if (/*num_trailing &&*/ delete_c != items_size_fast) {
						/* Restore position of trailing objects (because we need to unlock) */
						memmovec(me->l_list.ol_elemv + range.sr_end,
						         me->l_list.ol_elemv + range.sr_start + items_size_fast,
						         num_trailing, sizeof(DREF DeeObject *));
					}
					items_size_fast = items_size;
					DeeList_LockEndWrite(me);
					Dee_XFreea(delete_v);
					goto again_with_tp_asvector_nothrow;
				}
				me->l_list.ol_elemc = range.sr_start + items_size + num_trailing;
				DeeList_LockEndWrite(me);
	
				/* Drop references to deleted list items. */
				if (delete_v) {
					Dee_Decrefv(delete_v, delete_c);
					Dee_Freea(delete_v);
				}
				return 0;
			}
		}
	}

	/* General case: load the elements of "items" into a temporary vector buffer. */
	insert_v = DeeSeq_AsHeapVector(items, &insert_c);
	if unlikely(!insert_v)
		goto err;
	if unlikely(!insert_c) {
		Dee_objectlist_elemv_free(insert_v);
		return list_delrange_index(me, start, end);
	}
again:
	DeeList_LockWrite(me);
	DeeSeqRange_Clamp(&range, start, end, me->l_list.ol_elemc);
	delete_c = range.sr_end - range.sr_start;
	ASSERT(delete_c <= DeeList_SIZE(me));
	if (delete_c == me->l_list.ol_elemc) {
		/* Special case: assign to the entirety of the list
		 * -> Here, we can simply swap vectors. */
		DeeObject **old_elemv;
		size_t old_elemc;
		old_elemv = me->l_list.ol_elemv;
		old_elemc = me->l_list.ol_elemc;
		me->l_list.ol_elemv = insert_v; /* Inherit */
		me->l_list.ol_elemc = insert_c;
		_DeeList_SetAlloc(me, insert_c);
		DeeList_LockEndWrite(me);
		old_elemv = Dee_Decrefv(old_elemv, old_elemc);
		Dee_objectlist_elemv_free(old_elemv);
		goto done;
	}
	if (insert_c > delete_c) {
		/* Make sure the list has enough available memory. */
		size_t min_elema = (DeeList_SIZE(me) - delete_c) + insert_c;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < Dee_OBJECTLIST_MINALLOC)
				new_elema = Dee_OBJECTLIST_MINALLOC;
			do {
				new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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
	if (!delete_c) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(me) + range.sr_start + insert_c,
		           DeeList_ELEM(me) + range.sr_start,
		           DeeList_SIZE(me) - range.sr_start,
		           sizeof(DREF DeeObject *));

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + range.sr_start, insert_v,
		        insert_c, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc += insert_c;
		DeeList_LockEndWrite(me);
		Dee_objectlist_elemv_free(insert_v);
	} else {
		delete_v = (DREF DeeObject **)Dee_TryMallocac(delete_c, sizeof(DREF DeeObject *));
		if unlikely(!delete_v) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(delete_c, sizeof(DREF DeeObject *)))
				goto again;
			goto err_insertv;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delete_v, DeeList_ELEM(me) + range.sr_start,
		        delete_c, sizeof(DREF DeeObject *));

		/* Move following items to their proper places. */
		if (range.sr_start + insert_c != range.sr_end) {
			memmovec(DeeList_ELEM(me) + range.sr_start + insert_c,
			         DeeList_ELEM(me) + range.sr_end,
			         DeeList_SIZE(me) - range.sr_end,
			         sizeof(DREF DeeObject *));
		}

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + range.sr_start, insert_v,
		        insert_c, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc -= delete_c;
		me->l_list.ol_elemc += insert_c;
		DeeList_LockEndWrite(me);
		Dee_objectlist_elemv_free(insert_v);

		/* Drop object references. */
		Dee_Decrefv(delete_v, delete_c);

		/* Free the temporary del-item vector. */
		Dee_Freea(delete_v);
	}
done:
	return 0;
err_badalloc_overflow_unlock:
	DeeList_LockEndWrite(me);
	return Dee_BadAlloc((size_t)-1);
err_insertv:
	Dee_Decrefv(insert_v, insert_c);
	Dee_objectlist_elemv_free(insert_v);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeList_InsertSequence)(DeeObject *self, size_t index,
                               DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
	return list_setrange_index((List *)self,
	                           (Dee_ssize_t)index,
	                           (Dee_ssize_t)index,
	                           items);
#else /* __OPTIMIZE_SIZE__ */
	List *me = (List *)self;
	DREF DeeObject **insert_v;
	size_t insert_c;
	size_t used_index;

	/* Check if "items" can be appended in-place. */
	if (Dee_TYPE(items)->tp_seq) {
		size_t items_size_fast;
		size_t (DCALL *tp_asvector_nothrow)(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
		tp_asvector_nothrow = Dee_TYPE(items)->tp_seq->tp_asvector_nothrow;
		if (tp_asvector_nothrow && (items_size_fast = DeeObject_SizeFast(items)) != (size_t)-1) {
			if (items_size_fast == 0)
				return 0;

			if (tp_asvector_nothrow != NULL && !DeeObject_IsAsVectorNoThrowUsingLockOfList(me, items)) {
				/* Super-fast case: can transfer objects directly while holding a lock to "me" */
				size_t items_size, num_trailing;
				size_t cur_elema, min_elema;
again_with_tp_asvector_nothrow:
				DeeList_LockWrite(me);
				used_index = index;
				if (used_index > me->l_list.ol_elemc)
					used_index = me->l_list.ol_elemc;
				if (OVERFLOW_UADD(me->l_list.ol_elemc, items_size_fast, &min_elema))
					goto err_badalloc_overflow_unlock;
				cur_elema = DeeList_GetAlloc(me);
				if unlikely(min_elema > cur_elema) {
					/* Need a larger list buffer. */
					DREF DeeObject **new_listv;
					size_t new_elema = cur_elema;
					if (new_elema < Dee_OBJECTLIST_MINALLOC)
						new_elema = Dee_OBJECTLIST_MINALLOC;
					do {
						new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
					} while (new_elema < min_elema);
					new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
					if unlikely(!new_listv) {
						new_elema = min_elema;
						new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
						if unlikely(!new_listv) {
							DREF DeeObject **old_listv;
							DeeList_LockEndWrite(me);
							new_listv = Dee_objectlist_elemv_malloc(new_elema);
							if unlikely(!new_listv)
								goto err;
							DeeList_LockWrite(me);
							cur_elema = DeeList_GetAlloc(me);
							if likely(new_elema > cur_elema) {
								old_listv = me->l_list.ol_elemv;
								me->l_list.ol_elemv = (DREF DeeObject **)memcpyc(new_listv, old_listv,
								                                                 me->l_list.ol_elemc,
								                                                 sizeof(DREF DeeObject *));
							} else {
								old_listv = new_listv;
							}
							DeeList_LockEndWrite(me);
							Dee_objectlist_elemv_free(old_listv);
							goto again_with_tp_asvector_nothrow;
						}
					}
					me->l_list.ol_elemv = new_listv;
					_DeeList_SetAlloc(me, new_elema);
				}
	
				/* Shift trailing objects such that they appear at their final positions. */
				num_trailing = me->l_list.ol_elemc - used_index;
				if (/*num_trailing &&*/ items_size_fast) {
					memmoveupc(me->l_list.ol_elemv + used_index + items_size_fast,
					           me->l_list.ol_elemv + used_index,
					           num_trailing, sizeof(DREF DeeObject *));
				}
	
				/* Buffer should have sufficient size now. */
				items_size = (*tp_asvector_nothrow)(items, items_size_fast,
				                                    me->l_list.ol_elemv + used_index);
				if unlikely(items_size > items_size_fast) {
					/* Buffer is too small, and we didn't get references :( */
					if (/*num_trailing &&*/ items_size_fast) {
						/* Restore position of trailing objects (because we need to unlock) */
						memmovedownc(me->l_list.ol_elemv + used_index,
						             me->l_list.ol_elemv + used_index + items_size_fast,
						             num_trailing, sizeof(DREF DeeObject *));
					}
					items_size_fast = items_size;
					DeeList_LockEndWrite(me);
					goto again_with_tp_asvector_nothrow;
				}
				me->l_list.ol_elemc = used_index + items_size + num_trailing;
				DeeList_LockEndWrite(me);
				return 0;
			}
		}
	}

	/* General case: load the elements of "items" into a temporary vector buffer. */
	insert_v = DeeSeq_AsHeapVector(items, &insert_c);
	if unlikely(!insert_v)
		goto err;
	if unlikely(!insert_c) {
		Dee_objectlist_elemv_free(insert_v);
		return 0;
	}
again:
	DeeList_LockWrite(me);
	used_index = index;
	if (used_index > me->l_list.ol_elemc)
		used_index = me->l_list.ol_elemc;
	if (me->l_list.ol_elemc == 0) {
		/* Special case: assign to the entirety of the list
		 * -> Here, we can simply swap vectors. */
		DeeObject **old_elemv;
		old_elemv = me->l_list.ol_elemv;
		me->l_list.ol_elemv = insert_v; /* Inherit */
		me->l_list.ol_elemc = insert_c;
		_DeeList_SetAlloc(me, insert_c);
		DeeList_LockEndWrite(me);
		Dee_objectlist_elemv_free(old_elemv);
		goto done;
	}

	/* Make sure the list has enough available memory. */
	{
		size_t min_elema = DeeList_SIZE(me) + insert_c;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < Dee_OBJECTLIST_MINALLOC)
				new_elema = Dee_OBJECTLIST_MINALLOC;
			do {
				new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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

	/* Move following items to their proper places. */
	memmoveupc(DeeList_ELEM(me) + used_index + insert_c,
	           DeeList_ELEM(me) + used_index,
	           DeeList_SIZE(me) - used_index,
	           sizeof(DREF DeeObject *));

	/* Copy new items into the list. */
	memcpyc(DeeList_ELEM(me) + used_index, insert_v,
	        insert_c, sizeof(DREF DeeObject *));
	me->l_list.ol_elemc += insert_c;
	DeeList_LockEndWrite(me);
	Dee_objectlist_elemv_free(insert_v);
done:
	return 0;
err_badalloc_overflow_unlock:
	DeeList_LockEndWrite(me);
	return Dee_BadAlloc((size_t)-1);
err_insertv:
	Dee_Decrefv(insert_v, insert_c);
	Dee_objectlist_elemv_free(insert_v);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}



PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
list_setrange_index_n(List *me, Dee_ssize_t start, DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
	return list_setrange_index(me, start, SSIZE_MAX, items);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject **delete_v;
	DREF DeeObject **insert_v;
	size_t delete_c, insert_c;
	size_t start_index;

	/* Check if "items" can be appended in-place. */
	if (Dee_TYPE(items)->tp_seq) {
		size_t items_size_fast;
		size_t (DCALL *tp_asvector_nothrow)(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
		tp_asvector_nothrow = Dee_TYPE(items)->tp_seq->tp_asvector_nothrow;
		if (tp_asvector_nothrow && (items_size_fast = DeeObject_SizeFast(items)) != (size_t)-1) {
			if (items_size_fast == 0)
				return list_delrange_index_n(me, start);
			if (tp_asvector_nothrow != NULL && !DeeObject_IsAsVectorNoThrowUsingLockOfList(me, items)) {
				/* Super-fast case: can transfer objects directly while holding a lock to "me" */
				size_t items_size;
				size_t cur_elema, min_elema;
again_with_tp_asvector_nothrow:
				DeeList_LockWrite(me);
				start_index = DeeSeqRange_Clamp_n(start, me->l_list.ol_elemc);
				if (OVERFLOW_UADD(start_index, items_size_fast, &min_elema))
					goto err_badalloc_overflow_unlock;
				ASSERT(start_index <= me->l_list.ol_elemc);
				cur_elema = DeeList_GetAlloc(me);
				if unlikely(min_elema > cur_elema) {
					/* Need a larger list buffer. */
					DREF DeeObject **new_listv;
					size_t new_elema = cur_elema;
					if (new_elema < Dee_OBJECTLIST_MINALLOC)
						new_elema = Dee_OBJECTLIST_MINALLOC;
					do {
						new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
					} while (new_elema < min_elema);
					new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
					if unlikely(!new_listv) {
						new_elema = min_elema;
						new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
						if unlikely(!new_listv) {
							DREF DeeObject **old_listv;
							DeeList_LockEndWrite(me);
							new_listv = Dee_objectlist_elemv_malloc(new_elema);
							if unlikely(!new_listv)
								goto err;
							DeeList_LockWrite(me);
							cur_elema = DeeList_GetAlloc(me);
							if likely(new_elema > cur_elema) {
								old_listv = me->l_list.ol_elemv;
								me->l_list.ol_elemv = (DREF DeeObject **)memcpyc(new_listv, old_listv,
								                                                 me->l_list.ol_elemc,
								                                                 sizeof(DREF DeeObject *));
							} else {
								old_listv = new_listv;
							}
							DeeList_LockEndWrite(me);
							Dee_objectlist_elemv_free(old_listv);
							goto again_with_tp_asvector_nothrow;
						}
					}
					me->l_list.ol_elemv = new_listv;
					_DeeList_SetAlloc(me, new_elema);
				}
	
				/* If trailing elements should be removed, remember their references now */
				if (start_index < me->l_list.ol_elemc) {
					delete_c = me->l_list.ol_elemc - start_index;
					delete_v = (DREF DeeObject **)Dee_TryMallocac(delete_c, sizeof(DREF DeeObject *));
					if unlikely(!delete_v) {
						DeeList_LockEndWrite(me);
						delete_v = (DREF DeeObject **)Dee_Mallocac(delete_c, sizeof(DREF DeeObject *));
						if unlikely(!delete_v)
							goto err;
						DeeList_LockWrite(me);
						/* Validate that nothing about "me" changed. */
						if unlikely(start_index != DeeSeqRange_Clamp_n(start_index, me->l_list.ol_elemc) ||
						            cur_elema != DeeList_GetAlloc(me) ||
						            me->l_list.ol_elemc != (start_index + delete_c)) {
							DeeList_LockEndWrite(me);
							Dee_Freea(delete_v);
							goto again_with_tp_asvector_nothrow;
						}
					}
					memcpyc(delete_v, me->l_list.ol_elemv + start_index,
					        delete_c, sizeof(DREF DeeObject *));
				} else {
					delete_c = 0;
					delete_v = NULL;
				}
	
				/* Buffer should have sufficient size now. */
				items_size = (*tp_asvector_nothrow)(items, items_size_fast,
				                                    me->l_list.ol_elemv + start_index);
				if unlikely(items_size > items_size_fast) {
					/* Buffer is too small, and we didn't get references :( */
					items_size_fast = items_size;
					DeeList_LockEndWrite(me);
					Dee_XFreea(delete_v);
					goto again_with_tp_asvector_nothrow;
				}
				me->l_list.ol_elemc = start_index + items_size;
				DeeList_LockEndWrite(me);
	
				/* Drop references to deleted list items. */
				if (delete_v) {
					Dee_Decrefv(delete_v, delete_c);
					Dee_Freea(delete_v);
				}
				return 0;
			}
		}
	}

	/* General case: load the elements of "items" into a temporary vector buffer. */
	insert_v = DeeSeq_AsHeapVector(items, &insert_c);
	if unlikely(!insert_v)
		goto err;
	if unlikely(!insert_c) {
		Dee_objectlist_elemv_free(insert_v);
		return list_delrange_index_n(me, start);
	}
again:
	DeeList_LockWrite(me);
	start_index = DeeSeqRange_Clamp_n(start, me->l_list.ol_elemc);
	if (start_index == 0) {
		/* Special case: assign to the entirety of the list
		 * -> Here, we can simply swap vectors. */
		DeeObject **old_elemv;
		size_t old_elemc;
		old_elemv = me->l_list.ol_elemv;
		old_elemc = me->l_list.ol_elemc;
		me->l_list.ol_elemv = insert_v; /* Inherit */
		me->l_list.ol_elemc = insert_c;
		_DeeList_SetAlloc(me, insert_c);
		DeeList_LockEndWrite(me);
		old_elemv = Dee_Decrefv(old_elemv, old_elemc);
		Dee_objectlist_elemv_free(old_elemv);
		goto done;
	}
	delete_c = me->l_list.ol_elemc - start_index;
	ASSERT(delete_c <= DeeList_SIZE(me));
	if (insert_c > delete_c) {
		/* Make sure the list has enough available memory. */
		size_t min_elema = (DeeList_SIZE(me) - delete_c) + insert_c;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < Dee_OBJECTLIST_MINALLOC)
				new_elema = Dee_OBJECTLIST_MINALLOC;
			do {
				new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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
	if (!delete_c) {
		/* Move following items to their proper places. */
		memmoveupc(DeeList_ELEM(me) + start_index + insert_c,
		           DeeList_ELEM(me) + start_index,
		           DeeList_SIZE(me) - start_index,
		           sizeof(DREF DeeObject *));

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + start_index, insert_v,
		        insert_c, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc += insert_c;
		DeeList_LockEndWrite(me);
		Dee_objectlist_elemv_free(insert_v);
	} else {
		delete_v = (DREF DeeObject **)Dee_TryMallocac(delete_c, sizeof(DREF DeeObject *));
		if unlikely(!delete_v) {
			DeeList_LockEndWrite(me);
			if (Dee_CollectMemoryc(delete_c, sizeof(DREF DeeObject *)))
				goto again;
			goto err_insertv;
		}

		/* Move all items to-be deleted into the delete-vector. */
		memcpyc(delete_v, DeeList_ELEM(me) + start_index,
		        delete_c, sizeof(DREF DeeObject *));

		/* Copy new items into the list. */
		memcpyc(DeeList_ELEM(me) + start_index, insert_v,
		        insert_c, sizeof(DREF DeeObject *));
		me->l_list.ol_elemc -= delete_c;
		me->l_list.ol_elemc += insert_c;
		DeeList_LockEndWrite(me);
		Dee_objectlist_elemv_free(insert_v);

		/* Drop object references. */
		Dee_Decrefv(delete_v, delete_c);

		/* Free the temporary del-item vector. */
		Dee_Freea(delete_v);
	}
done:
	return 0;
err_badalloc_overflow_unlock:
	DeeList_LockEndWrite(me);
	return Dee_BadAlloc((size_t)-1);
err_insertv:
	Dee_Decrefv(insert_v, insert_c);
	Dee_objectlist_elemv_free(insert_v);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeList_AppendSequence)(DeeObject *self, DeeObject *items) {
#ifdef __OPTIMIZE_SIZE__
#if 1
	return list_setrange_index((List *)self, SSIZE_MAX, SSIZE_MAX, items);
#else
	return list_setrange_index_n((List *)self, SSIZE_MAX, items);
#endif
#else /* __OPTIMIZE_SIZE__ */
	List *me = (List *)self;
	DREF DeeObject **insert_v;
	size_t old_size, insert_c;

	/* Check if "items" can be appended in-place. */
	if (Dee_TYPE(items)->tp_seq) {
		size_t items_size_fast;
		size_t (DCALL *tp_asvector_nothrow)(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
		tp_asvector_nothrow = Dee_TYPE(items)->tp_seq->tp_asvector_nothrow;
		if (tp_asvector_nothrow && (items_size_fast = DeeObject_SizeFast(items)) != (size_t)-1) {
			if (items_size_fast == 0)
				return 0;
			if (tp_asvector_nothrow != NULL && !DeeObject_IsAsVectorNoThrowUsingLockOfList(me, items)) {
				/* Super-fast case: can transfer objects directly while holding a lock to "me" */
				size_t items_size;
				size_t cur_elema, min_elema;
again_with_tp_asvector_nothrow:
				DeeList_LockWrite(me);
				old_size = me->l_list.ol_elemc;
				if (OVERFLOW_UADD(old_size, items_size_fast, &min_elema))
					goto err_badalloc_overflow_unlock;
				cur_elema = DeeList_GetAlloc(me);
				if unlikely(min_elema > cur_elema) {
					/* Need a larger list buffer. */
					DREF DeeObject **new_listv;
					size_t new_elema = cur_elema;
					if (new_elema < Dee_OBJECTLIST_MINALLOC)
						new_elema = Dee_OBJECTLIST_MINALLOC;
					do {
						new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
					} while (new_elema < min_elema);
					new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
					if unlikely(!new_listv) {
						new_elema = min_elema;
						new_listv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
						if unlikely(!new_listv) {
							DREF DeeObject **old_listv;
							DeeList_LockEndWrite(me);
							new_listv = Dee_objectlist_elemv_malloc(new_elema);
							if unlikely(!new_listv)
								goto err;
							DeeList_LockWrite(me);
							cur_elema = DeeList_GetAlloc(me);
							if likely(new_elema > cur_elema) {
								old_listv = me->l_list.ol_elemv;
								me->l_list.ol_elemv = (DREF DeeObject **)memcpyc(new_listv, old_listv,
								                                                 me->l_list.ol_elemc,
								                                                 sizeof(DREF DeeObject *));
							} else {
								old_listv = new_listv;
							}
							DeeList_LockEndWrite(me);
							Dee_objectlist_elemv_free(old_listv);
							goto again_with_tp_asvector_nothrow;
						}
					}
					me->l_list.ol_elemv = new_listv;
					_DeeList_SetAlloc(me, new_elema);
				}
	
				/* Buffer should have sufficient size now. */
				items_size = (*tp_asvector_nothrow)(items, items_size_fast,
				                                    me->l_list.ol_elemv + old_size);
				if unlikely(items_size > items_size_fast) {
					/* Buffer is too small, and we didn't get references :( */
					items_size_fast = items_size;
					DeeList_LockEndWrite(me);
					goto again_with_tp_asvector_nothrow;
				}
				me->l_list.ol_elemc = old_size + items_size;
				DeeList_LockEndWrite(me);
				return 0;
			}
		}
	}

	/* General case: load the elements of "items" into a temporary vector buffer. */
	insert_v = DeeSeq_AsHeapVector(items, &insert_c);
	if unlikely(!insert_v)
		goto err;
	if unlikely(!insert_c) {
		Dee_objectlist_elemv_free(insert_v);
		return 0;
	}
again:
	DeeList_LockWrite(me);
	old_size = me->l_list.ol_elemc;
	if (old_size == 0) {
		/* Special case: assign to the entirety of the list
		 * -> Here, we can simply swap vectors. */
		DeeObject **old_elemv;
		old_elemv = me->l_list.ol_elemv;
		me->l_list.ol_elemv = insert_v; /* Inherit */
		me->l_list.ol_elemc = insert_c;
		_DeeList_SetAlloc(me, insert_c);
		DeeList_LockEndWrite(me);
		Dee_objectlist_elemv_free(old_elemv);
		goto done;
	}

	{
		/* Make sure the list has enough available memory. */
		size_t min_elema = old_size + insert_c;
		size_t old_elema = DeeList_GetAlloc(me);
		if (min_elema > old_elema) {
			size_t new_elema = old_elema;
			DREF DeeObject **new_elemv;
			if (new_elema < Dee_OBJECTLIST_MINALLOC)
				new_elema = Dee_OBJECTLIST_MINALLOC;
			do {
				new_elema = Dee_OBJECTLIST_MOREALLOC(new_elema);
			} while (new_elema < min_elema);
			new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
			if unlikely(!new_elemv) {
				new_elema = min_elema;
				new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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

	/* Copy new items into the list. */
	memcpyc(DeeList_ELEM(me) + old_size, insert_v,
	        insert_c, sizeof(DREF DeeObject *));
	me->l_list.ol_elemc += insert_c;
	DeeList_LockEndWrite(me);
	Dee_objectlist_elemv_free(insert_v);
done:
	return 0;
err_badalloc_overflow_unlock:
	DeeList_LockEndWrite(me);
	return Dee_BadAlloc((size_t)-1);
err_insertv:
	Dee_Decrefv(insert_v, insert_c);
	Dee_objectlist_elemv_free(insert_v);
err:
	return -1;
#endif /* !__OPTIMIZE_SIZE__ */
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
list_delrange(List *me, DeeObject *begin, DeeObject *end) {
	Dee_ssize_t i_begin, i_end;
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
	Dee_ssize_t i_begin, i_end;
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
		DeeRT_ErrIndexOutOfBounds((DeeObject *)me, index, list_size);
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

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
list_xchitem_index(List *me, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	DeeList_LockWrite(me);
	if unlikely(index >= DeeList_SIZE(me)) {
		size_t my_length = DeeList_SIZE(me);
		DeeList_LockEndWrite(me);
		DeeRT_ErrIndexOutOfBounds((DeeObject *)me, index, my_length);
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
list_mh_foreach_reverse(List *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	DeeList_LockRead(self);
	i = self->l_list.ol_elemc;
	while (i) {
		DREF DeeObject *list_elem;
		--i;
		list_elem = self->l_list.ol_elemv[i];
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		temp = (*proc)(arg, list_elem);
		Dee_Decref_unlikely(list_elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		DeeList_LockRead(self);
		if (i > self->l_list.ol_elemc)
			i = self->l_list.ol_elemc;
	}
	DeeList_LockEndRead(self);
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
list_mh_seq_enumerate_index(List *self, Dee_seq_enumerate_index_t proc,
                            void *arg, size_t start, size_t end) {
	size_t i = start;
	Dee_ssize_t temp, result = 0;
	DeeList_LockRead(self);
	for (; i < end; ++i) {
		DREF DeeObject *list_elem;
		if (i >= self->l_list.ol_elemc)
			break;
		list_elem = self->l_list.ol_elemv[i];
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		temp = (*proc)(arg, i, list_elem);
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
list_mh_seq_enumerate_index_reverse(List *self, Dee_seq_enumerate_index_t proc,
                                    void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	DeeList_LockRead(self);
	for (;;) {
		DREF DeeObject *list_elem;
		if (end > self->l_list.ol_elemc)
			end = self->l_list.ol_elemc;
		if (end <= start)
			break;
		--end;
		list_elem = self->l_list.ol_elemv[end];
		Dee_Incref(list_elem);
		DeeList_LockEndRead(self);
		temp = (*proc)(arg, end, list_elem);
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_asvector_nothrow(List *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t realsize;
	DeeList_LockRead(self);
	realsize = self->l_list.ol_elemc;
	if likely(dst_length >= realsize)
		Dee_Movrefv(dst, self->l_list.ol_elemv, realsize);
	DeeList_LockEndRead(self);
	return realsize;
}

PRIVATE struct type_seq list_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&list_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_setitem,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&list_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&list_setrange,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&list_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast), /* default */
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index), /* default */
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&list_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&list_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&list_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&list_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&list_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&list_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&list_hasitem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&list_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&list_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&list_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&list_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&list_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&list_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&list_setrange_index_n,
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&list_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&list_asvector_nothrow,
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_mh_remove(List *me, DeeObject *item, size_t start, size_t end) {
	DeeObject **vector;
	size_t i, length;
	ASSERT_OBJECT(item);
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
		temp = DeeObject_TryCompareEq(item, this_elem);
		Dee_Decref(this_elem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp)) {
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
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
list_mh_remove_with_key(List *me, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DeeObject **vector;
	size_t i, length;
	ASSERT_OBJECT(item);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	DeeList_LockRead(me);
again_locked:
	vector = me->l_list.ol_elemv;
	length = me->l_list.ol_elemc;
	for (i = start; i < length && i < end; ++i) {
		DREF DeeObject *this_elem;
		int temp;
		this_elem = DeeList_GET(me, i);
		Dee_Incref(this_elem);
		DeeList_LockEndRead(me);
		temp = DeeObject_TryCompareKeyEq(item, this_elem, key);
		Dee_Decref(this_elem);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
			/* This is the element we're supposed to remove. */
			DeeList_LockWrite(me);

			/* Check if the list was changed. */
			if (me->l_list.ol_elemv != vector ||
			    me->l_list.ol_elemc != length ||
			    DeeList_GET(me, i) != this_elem) {
				DeeList_LockDowngrade(me);
				goto again_locked;
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
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
			Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
			return 1;
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again_locked;
	}
	DeeList_LockEndRead(me);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return 0;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_mh_rremove(List *me, DeeObject *item, size_t start, size_t end) {
	DeeObject **vector;
	size_t i, length;
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
		temp = DeeObject_TryCompareEq(item, this_elem);
		Dee_Decref(this_elem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp)) {
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
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
list_mh_rremove_with_key(List *me, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DeeObject **vector;
	size_t i, length;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
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
		temp = DeeObject_TryCompareKeyEq(item, this_elem, key);
		Dee_Decref(this_elem);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
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
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
			Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
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
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return 0;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
list_mh_removeall(List *me, DeeObject *item, size_t start, size_t end, size_t max) {
	DeeObject **vector;
	size_t i, length, result = 0;
	if unlikely(!max)
		return 0;
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

		/* Invoke a predicate. */
		temp = DeeObject_TryCompareEq(item, this_elem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp)) {
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
			++result;
			if (result >= max)
				goto done;
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
done:
	ASSERT(result != (size_t)-1);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2, 6)) size_t DCALL
list_mh_removeall_with_key(List *me, DeeObject *item, size_t start,
                           size_t end, size_t max, DeeObject *key) {
	DeeObject **vector;
	size_t i, length, result = 0;
	if unlikely(!max)
		return 0;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
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

		/* Invoke a predicate. */
		temp = DeeObject_TryCompareKeyEq(item, this_elem, key);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
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
			++result;
			if (result >= max)
				goto done;
		}

		/* Continue onwards. */
		DeeList_LockRead(me);

		/* Check if the list was changed. */
		if (me->l_list.ol_elemv != vector ||
		    me->l_list.ol_elemc != length)
			goto again;
	}
	DeeList_LockEndRead(me);
done:
	ASSERT(result != (size_t)-1);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return result;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)-1;
}

PRIVATE ATTR_NOINLINE NONNULL((1, 4)) void DCALL
list_mh_fill_fallback(List *me, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject *old_values[32];
	size_t old_values_count;
again:
	old_values_count = 0;
	DeeList_LockWrite(me);
	for (; start < end && start < me->l_list.ol_elemc; ++start) {
		if unlikely(old_values_count >= COMPILER_LENOF(old_values)) {
			DeeList_LockEndWrite(me);
			Dee_Decrefv(old_values, COMPILER_LENOF(old_values));
			goto again;
		}
		old_values[old_values_count++] = me->l_list.ol_elemv[start]; /* Inherit reference */
		me->l_list.ol_elemv[start] = filler;
		Dee_Incref(filler);
	}
	DeeList_LockEndWrite(me);
	Dee_Decrefv(old_values, old_values_count);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
list_mh_fill(List *me, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject **old_values;
	size_t length = DeeList_SIZE_ATOMIC(me);
	if (end > length)
		end = length;
	if unlikely(start >= end)
		return 0;
	length = end - start;
	old_values = (DREF DeeObject **)Dee_TryMallocac(end - start, sizeof(DREF DeeObject *));
	if likely(old_values) {
		DeeList_LockWrite(me);
		if unlikely(end > me->l_list.ol_elemc) {
			end = me->l_list.ol_elemc;
			if unlikely(start >= end) {
				DeeList_LockEndWrite(me);
				return 0;
			}
			length = end - start;
		}
		memcpyc(old_values, me->l_list.ol_elemv + start, length, sizeof(DREF DeeObject *));
		Dee_Setrefv(me->l_list.ol_elemv + start, filler, length);
		DeeList_LockEndWrite(me);
		Dee_Decrefv(old_values, length);
		Dee_Freea(old_values);
	} else {
		/* Must fill values non-atomically. */
		list_mh_fill_fallback(me, start, end, filler);
	}
	return 0;
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
list_mh_find(List *self, DeeObject *item, size_t start, size_t end) {
	size_t i = start;
	DeeList_LockRead(self);
	for (; i < end && i < self->l_list.ol_elemc; ++i) {
		DREF DeeObject *myitem;
		int temp;
		myitem = self->l_list.ol_elemv[i];
		Dee_Incref(myitem);
		DeeList_LockEndRead(self);
		temp = DeeObject_TryCompareEq(item, myitem);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return i;
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
list_mh_find_with_key(List *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t i = start;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	DeeList_LockRead(self);
	for (; i < end && i < self->l_list.ol_elemc; ++i) {
		DREF DeeObject *myitem;
		int temp;
		myitem = self->l_list.ol_elemv[i];
		Dee_Incref(myitem);
		DeeList_LockEndRead(self);
		temp = DeeObject_TryCompareKeyEq(item, myitem, key);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
			Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
			return i;
		}
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)-1;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
list_mh_rfind(List *self, DeeObject *item, size_t start, size_t end) {
	size_t i = end;
	DeeList_LockRead(self);
	for (;;) {
		DREF DeeObject *myitem;
		int temp;
		if (i > self->l_list.ol_elemc)
			i = self->l_list.ol_elemc;
		if (i <= start)
			break;
		--i;
		myitem = self->l_list.ol_elemv[i];
		Dee_Incref(myitem);
		DeeList_LockEndRead(self);
		temp = DeeObject_TryCompareEq(item, myitem);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return i;
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
list_mh_rfind_with_key(List *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t i = end;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	DeeList_LockRead(self);
	for (;;) {
		DREF DeeObject *myitem;
		int temp;
		if (i > self->l_list.ol_elemc)
			i = self->l_list.ol_elemc;
		if (i <= start)
			break;
		--i;
		myitem = self->l_list.ol_elemv[i];
		Dee_Incref(myitem);
		DeeList_LockEndRead(self);
		temp = DeeObject_TryCompareKeyEq(item, myitem, key);
		Dee_Decref(myitem);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp)) {
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
			Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
			return i;
		}
		DeeList_LockRead(self);
	}
	DeeList_LockEndRead(self);
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)-1;
err_item:
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	Dee_Decref(item);
err:
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_mh_clear(List *__restrict self) {
	DeeList_Clear(Dee_AsObject(self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_mh_sort(List *__restrict self, size_t start, size_t end) {
	return DeeList_Sort(Dee_AsObject(self), start, end, Dee_None);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_mh_reverse(List *__restrict self, size_t start, size_t end) {
	DeeList_Reverse(Dee_AsObject(self), start, end);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_append(List *me, size_t argc, DeeObject *const *argv) {
	/* Optimize for the case of a single object to-be appended. */
#ifndef __OPTIMIZE_SIZE__
	if likely(argc == 1) {
		if (DeeList_Append((DeeObject *)me, argv[0]))
			goto err;
	} else
#endif /* !__OPTIMIZE_SIZE__ */
	{
		if (DeeList_AppendVector((DeeObject *)me, argc, argv))
			goto err;
	}
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
list_tryget_first(List *__restrict me) {
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
	return ITER_DONE;
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
	return DeeRT_ErrTUnboundAttr(&DeeList_Type, me, &str_first);
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
	return DeeRT_ErrEmptySequence(me);
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
	return DeeRT_ErrEmptySequence(me);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_tryget_last(List *__restrict me) {
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
	return ITER_DONE;
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
	return DeeRT_ErrTUnboundAttr(&DeeList_Type, me, &str_last);
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
	return DeeRT_ErrEmptySequence(me);
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
	return DeeRT_ErrEmptySequence(me);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
list_nonempty_as_bound(List *__restrict me) {
	return Dee_BOUND_FROMBOOL(DeeList_SIZE_ATOMIC(me) != 0);
}

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

PRIVATE NONNULL((1)) bool DCALL
list_do_shrink(List *__restrict me) {
	bool result = false;
	size_t old_elema;
	DeeList_LockWrite(me);
	old_elema = DeeList_GetAlloc(me);
	if (me->l_list.ol_elemc < old_elema) {
		size_t new_elema = me->l_list.ol_elemc;
		DREF DeeObject **new_elemv;
		new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
		if likely(new_elemv) {
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, new_elema);
			result = true;
		}
	}
	DeeList_LockEndWrite(me);
	return result;
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
		new_elemv = Dee_objectlist_elemv_tryrealloc(me->l_list.ol_elemv, new_elema);
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
list_reserve(List *me, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	bool result = false;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("reserve", params: "
	size_t total = 0;
	size_t more = 0;
", docStringPrefix: "list");]]]*/
#define list_reserve_params "total=!0,more=!0"
	struct {
		size_t total;
		size_t more;
	} args;
	args.total = 0;
	args.more = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__total_more, "|" UNPuSIZ UNPuSIZ ":reserve", &args))
		goto err;
/*[[[end]]]*/
	DeeList_LockWrite(me);
	if (args.total < me->l_list.ol_elemc)
		args.total = me->l_list.ol_elemc;
	if (OVERFLOW_UADD(args.total, args.more, &args.total))
		args.total = (size_t)-1;
	if (args.total <= DeeList_GetAlloc(me)) {
		result = true;
	} else {
		/* Try to allocate more memory for this List. */
		DREF DeeObject **new_elemv;
		new_elemv = Dee_objectlist_elemv_tryrealloc_safe(me->l_list.ol_elemv, args.total);
		if likely(new_elemv) {
			me->l_list.ol_elemv = new_elemv;
			_DeeList_SetAlloc(me, args.total);
			result = true;
		}
	}
	DeeList_LockEndWrite(me);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_shrink(List *me, size_t argc, DeeObject *const *argv) {
	bool result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("shrink", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "shrink");
/*[[[end]]]*/
	result = list_do_shrink(me);
	return_bool(result);
err:
	return NULL;
}

/* Reverse the order of the elements of `self' */
PUBLIC NONNULL((1)) void DCALL
DeeList_Reverse(DeeObject *__restrict self, size_t start, size_t end) {
	List *me = (List *)self;
	DeeObject **lo, **hi;
	DeeList_LockWrite(me);
	if (end > me->l_list.ol_elemc)
		end = me->l_list.ol_elemc;
	if unlikely(start > end)
		start = end;
	lo = DeeList_ELEM(me) + start;
	hi = DeeList_ELEM(me) + end;
	while (lo < hi) {
		DeeObject *temp;
		temp  = *lo;
		*lo++ = *--hi;
		*hi   = temp;
	}
	DeeList_LockEndWrite(me);
}



/* Sort the given list ascendingly, or according to `key'
 * To use default sorting, pass `Dee_None' for `key' */
PUBLIC WUNUSED NONNULL((1, 4)) int DCALL
DeeList_Sort(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	List *me = (List *)self;
	DeeObject **oldv, **newv;
	size_t oldc, objc, list_objc;
	size_t used_start = start;
	size_t used_end = end;
	list_objc = DeeList_SIZE_ATOMIC(me);
	if (used_end > list_objc)
		used_end = list_objc;
	if unlikely(used_start > used_end)
		used_start = used_end;
	objc = used_end - used_start;
	oldv = Dee_objectlist_elemv_malloc(objc);
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(me);
	if unlikely(me->l_list.ol_elemc != list_objc) {
		size_t new_used_start = start;
		size_t new_used_end   = end;
		list_objc = me->l_list.ol_elemc;
		if (new_used_end > list_objc)
			new_used_end = list_objc;
		if unlikely(new_used_start > new_used_end)
			new_used_start = new_used_end;
		if (new_used_start != used_start || new_used_end != used_end) {
			DeeObject **new_objv;
			DeeList_LockEndRead(me);
			new_objv = Dee_objectlist_elemv_realloc(oldv, list_objc);
			if unlikely(!new_objv)
				goto err_oldv;
			oldv       = new_objv;
			used_start = new_used_start;
			used_end   = new_used_end;
			objc       = used_end - used_start;
			goto again;
		}
	}

	/* Read all the old elements from the list. */
	Dee_Movrefv(oldv, me->l_list.ol_elemv + used_start, objc);
	DeeList_LockEndRead(me);

	/* Allocate the new list */
	newv = Dee_objectlist_elemv_malloc(objc);
	if unlikely(!newv)
		goto err_oldv_elem;
	/* Do the actual sorting. */
	if unlikely(!DeeNone_Check(key)
	            ? DeeSeq_SortVectorWithKey(objc, newv, oldv, key)
	            : DeeSeq_SortVector(objc, newv, oldv))
		goto err_newv;
	if likely(objc == list_objc) {
		/* Likely case: sort the whole list (replace the entire vector) */
		Dee_objectlist_elemv_free(oldv);
		DeeList_LockWrite(me);
		oldv = me->l_list.ol_elemv;
		oldc = me->l_list.ol_elemc;
		me->l_list.ol_elemc = objc;
		me->l_list.ol_elemv = newv;
		_DeeList_SetAlloc(me, objc);
		DeeList_LockEndWrite(me);
		Dee_Decrefv(oldv, oldc);
		Dee_objectlist_elemv_free(oldv);
	} else {
		/* Special case: only a sub-range of the list was sorted; must override that sub-range. */
		DeeList_LockWrite(me);
		if unlikely(me->l_list.ol_elemc < (used_start + objc)) {
			DeeList_LockEndWrite(me);
			Dee_Decrefv(newv, objc);
			Dee_objectlist_elemv_free(newv);
			goto again;
		}
		memcpyc(oldv, me->l_list.ol_elemv + used_start, objc, sizeof(DREF DeeObject *));
		memcpyc(me->l_list.ol_elemv + used_start, newv, objc, sizeof(DREF DeeObject *));
		DeeList_LockEndWrite(me);
		Dee_Decrefv(oldv, objc);
		Dee_objectlist_elemv_free(oldv);
		Dee_objectlist_elemv_free(newv);
	}
	return 0;
err_newv:
	Dee_objectlist_elemv_free(newv);
err_oldv_elem:
	Dee_Decrefv(oldv, objc);
err_oldv:
	Dee_objectlist_elemv_free(oldv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
list_mh_sorted(List *__restrict me, size_t start, size_t end) {
	DREF DeeTupleObject *result;
	DeeObject **oldv;
	size_t objc, list_objc;
	size_t used_start = start;
	size_t used_end = end;
	list_objc = DeeList_SIZE_ATOMIC(me);
	if (used_end > list_objc)
		used_end = list_objc;
	if unlikely(used_start > used_end)
		used_start = used_end;
	objc = used_end - used_start;
	oldv = Dee_objectlist_elemv_malloc(objc);
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(me);
	if unlikely(me->l_list.ol_elemc != list_objc) {
		size_t new_used_start = start;
		size_t new_used_end   = end;
		list_objc = me->l_list.ol_elemc;
		if (new_used_end > list_objc)
			new_used_end = list_objc;
		if unlikely(new_used_start > new_used_end)
			new_used_start = new_used_end;
		if (new_used_start != used_start || new_used_end != used_end) {
			DeeObject **new_objv;
			DeeList_LockEndRead(me);
			new_objv = Dee_objectlist_elemv_realloc(oldv, list_objc);
			if unlikely(!new_objv)
				goto err_oldv;
			oldv       = new_objv;
			used_start = new_used_start;
			used_end   = new_used_end;
			objc       = used_end - used_start;
			goto again;
		}
	}

	/* Read all the old elements from the list. */
	Dee_Movrefv(oldv, me->l_list.ol_elemv + used_start, objc);
	DeeList_LockEndRead(me);

	/* Allocate the new tuple */
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto err_oldv_elem;

	/* Do the actual sorting. */
	if unlikely(DeeSeq_SortVector(objc, result->t_elem, oldv))
		goto err_oldv_elem_result;
	Dee_objectlist_elemv_free(oldv);
	return result;
err_oldv_elem_result:
	DeeTuple_FreeUninitialized(result);
err_oldv_elem:
	Dee_Decrefv(oldv, objc);
err_oldv:
	Dee_objectlist_elemv_free(oldv);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
list_mh_sorted_with_key(List *__restrict me, size_t start, size_t end, DeeObject *key) {
	DREF DeeTupleObject *result;
	DeeObject **oldv;
	size_t objc, list_objc;
	size_t used_start = start;
	size_t used_end = end;
	list_objc = DeeList_SIZE_ATOMIC(me);
	if (used_end > list_objc)
		used_end = list_objc;
	if unlikely(used_start > used_end)
		used_start = used_end;
	objc = used_end - used_start;
	oldv = Dee_objectlist_elemv_malloc(objc);
	if unlikely(!oldv)
		goto err;
again:
	DeeList_LockRead(me);
	if unlikely(me->l_list.ol_elemc != list_objc) {
		size_t new_used_start = start;
		size_t new_used_end   = end;
		list_objc = me->l_list.ol_elemc;
		if (new_used_end > list_objc)
			new_used_end = list_objc;
		if unlikely(new_used_start > new_used_end)
			new_used_start = new_used_end;
		if (new_used_start != used_start || new_used_end != used_end) {
			DeeObject **new_objv;
			DeeList_LockEndRead(me);
			new_objv = Dee_objectlist_elemv_realloc(oldv, list_objc);
			if unlikely(!new_objv)
				goto err_oldv;
			oldv       = new_objv;
			used_start = new_used_start;
			used_end   = new_used_end;
			objc       = used_end - used_start;
			goto again;
		}
	}

	/* Read all the old elements from the list. */
	Dee_Movrefv(oldv, me->l_list.ol_elemv + used_start, objc);
	DeeList_LockEndRead(me);

	/* Allocate the new tuple */
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto err_oldv_elem;

	/* Do the actual sorting. */
	if unlikely(DeeSeq_SortVectorWithKey(objc, result->t_elem, oldv, key))
		goto err_oldv_elem_result;
	Dee_objectlist_elemv_free(oldv);
	return result;
err_oldv_elem_result:
	DeeTuple_FreeUninitialized(result);
err_oldv_elem:
	Dee_Decrefv(oldv, objc);
err_oldv:
	Dee_objectlist_elemv_free(oldv);
err:
	return NULL;
}


/* Deprecated functions. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
IteratorFuture_Of(DeeObject *__restrict self); /* From "./iterator.c" */
PRIVATE WUNUSED DREF DeeObject *DCALL
list_insertiter_deprecated(List *me, size_t argc, DeeObject *const *argv) {
	int temp;
	DREF DeeObject *future;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("insert_iter", params: "
	size_t index;
	DeeObject *iter: ?DIterator;
", docStringPrefix: "list");]]]*/
#define list_insert_iter_params "index:?Dint,iter:?DIterator"
	struct {
		size_t index;
		DeeObject *iter;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "insert_iter", &args, &args.index, UNPuSIZ, DeeObject_AsSize, &args.iter, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	future = IteratorFuture_Of(args.iter);
	if unlikely(!future)
		goto err;
	temp = DeeList_InsertSequence((DeeObject *)me, args.index, future);
	Dee_Decref(future);
	if unlikely(temp)
		goto err;
	return_none;
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */


PRIVATE struct type_getset tpconst list_getsets[] = {
	TYPE_GETSET_AB_F("allocated", &list_getallocated, &list_delallocated, &list_setallocated, METHOD_FNOREFESCAPE,
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
	TYPE_GETSET_BOUND_F(STR_first,
	                    &list_get_first,
	                    &list_del_first,
	                    &list_set_first,
	                    &list_nonempty_as_bound,
	                    METHOD_FNOREFESCAPE,
	                    "->\n"
	                    "#r{The first item from @this List}"),
	TYPE_GETSET_BOUND_F(STR_last,
	                    &list_get_last,
	                    &list_del_last,
	                    &list_set_last,
	                    &list_nonempty_as_bound,
	                    METHOD_FNOREFESCAPE,
	                    "->\n"
	                    "#r{The last item from @this List}"),
	TYPE_GETTER_AB_F(STR_frozen, &DeeTuple_FromList, METHOD_FNOREFESCAPE,
	                 "->?DTuple\n"
	                 "Return a copy of the contents of @this List as an immutable sequence"),
	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &list_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* From `seq.c' */
seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* From `seq.c' */
seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv);

#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_binsert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distinct(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */


PRIVATE struct type_method tpconst list_methods[] = {
	/* `List.append()' is a little different from `Sequence.append', in that
	 * it is a varargs function, and will append *all* given arguments to the
	 * list in order.
	 *
	 * Note that behavior is considered deprecated, but will be maintained
	 * for the sake of backwards-compatibility.
	 *
	 * Additionally, `(myList as Sequence).append(1, 2, 3)' will cause an
	 * error, meaning that only `myList.append(1, 2, 3)' works. */
	TYPE_METHOD_F(STR_append, &list_append, METHOD_FNOREFESCAPE,
	              "(items!)\n"
	              "Append all the given @items at the end of @this List"),

	/* List buffer functions. */
	TYPE_KWMETHOD_F("reserve", &list_reserve, METHOD_FNOREFESCAPE,
	                "(" list_reserve_params ")->?Dbool\n"
	                "#r{Indicative of the ?. having sufficient preallocated space on return}"
	                "Reserve (preallocate) memory for at least ${({#this, total} > ...) + more} items\n"
	                "Failures to pre-allocate memory are silently ignored, "
	                /**/ "in which case ?#allocated will remain unchanged\n"
	                "If the effective new size is less than the current ?#allocated, the function becomes a no-op"),
	TYPE_METHOD_F("shrink", &list_shrink, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#r{Returns !t if memory was freed}"
	              "Release any pre-allocated, but unused memory, setting "
	              /**/ "?#allocated to the length of @this List"),

	TYPE_METHOD_HINTREF(Sequence_extend),
	TYPE_METHOD_HINTREF(Sequence_resize),
	TYPE_METHOD_HINTREF(Sequence_insert),
	TYPE_METHOD_HINTREF(Sequence_insertall),
	TYPE_METHOD_HINTREF(Sequence_erase),
	TYPE_METHOD_HINTREF(Sequence_pop),
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_HINTREF(Sequence_find),
	TYPE_METHOD_HINTREF(Sequence_rfind),
	TYPE_METHOD_HINTREF(Sequence_remove),
	TYPE_METHOD_HINTREF(Sequence_rremove),
	TYPE_METHOD_HINTREF(Sequence_removeall),
	TYPE_METHOD_HINTREF(Sequence_removeif),
	TYPE_METHOD_HINTREF(Sequence_fill),
	TYPE_METHOD_HINTREF(Sequence_reverse),
	TYPE_METHOD_HINTREF(Sequence_sort),
	TYPE_METHOD_HINTREF(Sequence_sorted),
	TYPE_METHOD_HINTREF(__seq_append__),
	TYPE_METHOD_HINTREF(__seq_enumerate__),

	/* Deprecated aliases / functions. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_KWMETHOD_F("remove_if", &DeeMA_Sequence_removeif, METHOD_FNOREFESCAPE,
	                "(should:?DCallable,start=!0,end=!-1)->?Dint\n"
	                "Deprecated alias for ?#removeif"),
	TYPE_KWMETHOD_F("insert_list", &DeeMA_Sequence_insertall, METHOD_FNOREFESCAPE,
	                "(index:?Dint,items:?S?O)\n"
	                "Deprecated alias for ?#insertall"),
	TYPE_METHOD_F("insert_iter", &list_insertiter_deprecated, METHOD_FNOREFESCAPE,
	              "(" list_insert_iter_params ")\n"
	              "Deprecated alias for ${this.insertall(index, (iter as iterator from deemon).future)}"),
	TYPE_METHOD_F("push_front", &DeeMA_Sequence_pushfront, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#pushfront"),
	TYPE_METHOD_F("push_back", &DeeMA_Sequence_append, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#pushback"),
	TYPE_METHOD_F("pop_front", &seq_popfront, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#popfront"),
	TYPE_METHOD_F("pop_back", &seq_popback, METHOD_FNOREFESCAPE,
	              "(item)\n"
	              "Deprecated alias for ?#popback"),
	TYPE_METHOD_F("shrink_to_fit", &list_shrink, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Deprecated alias for ?#shrink"),

	TYPE_KWMETHOD("sorted_insert", &seq_binsert,
	              "(item,start=!0,end=!-1)\n"
	              "Deprecated alias for ?Abinsert?DSequence present only in ?."),
	TYPE_KWMETHOD("tounique", &seq_distinct,
	              "(key?:?DCallable)->?DSet\n"
	              "Deprecated alias for ?Adistinct?DSequence present only in ?."),
	/* TODO: DEE_METHODDEF_v100("unique", member(&_deelist_unique), DEE_DOC_AUTO),
	 * TODO: DEE_METHODDEF_CONST_v100("extend_unique", member(&_deelist_extend_unique), DEE_DOC_AUTO), */
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst list_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &list_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &list_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &list_mh_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &list_tryget_first, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &list_tryget_last, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_append, &DeeList_Append, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_extend, &DeeList_AppendSequence, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_resize, &DeeList_Resize, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insert, &DeeList_Insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insertall, &DeeList_InsertSequence, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_erase, &DeeList_Erase, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pop, &DeeList_Pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &list_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_clear, &list_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find, &list_mh_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &list_mh_find_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &list_mh_rfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &list_mh_rfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_remove, &list_mh_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_remove_with_key, &list_mh_remove_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rremove, &list_mh_rremove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rremove_with_key, &list_mh_rremove_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeall, &list_mh_removeall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeall_with_key, &list_mh_removeall_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_fill, &list_mh_fill, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeif, &DeeList_RemoveIf, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reverse, &list_mh_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sort, &list_mh_sort, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sort_with_key, &DeeList_Sort, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted, &list_mh_sorted, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted_with_key, &list_mh_sorted_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_gc tpconst list_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&DeeList_Clear
};

INTERN WUNUSED NONNULL((1, 2)) DREF List *DCALL
list_add(List *me, DeeObject *other) {
	DREF List *result = DeeList_Copy(me);
	if unlikely(!result)
		goto err;
	if unlikely(DeeList_AppendSequence((DeeObject *)result, other))
		goto err_r;
	return result;
err_r:
	DeeList_Destroy(result);
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
		DeeRT_ErrIntegerOverflowUMul(my_elemc, multiplier);
		goto err;
	}
	if unlikely(res_elemc == 0) {
		DeeList_LockEndRead(me);
		return (DREF List *)DeeList_New();
	}
	res_elemv = Dee_objectlist_elemv_trymalloc_safe(res_elemc);
	if unlikely(!res_elemv) {
		DeeList_LockEndRead(me);
		if (Dee_CollectMemorycSafe(res_elemc, sizeof(DREF DeeObject *)))
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
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->l_lock);
	DeeObject_Init(result, &DeeList_Type);
	return DeeGC_TRACK(List, result);
err_elem:
	Dee_Decrefv(res_elemv, res_elemc);
	Dee_objectlist_elemv_free(res_elemv);
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
		DeeRT_ErrIntegerOverflowUMul(my_length, multiplier);
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
		Dee_objectlist_elemv_free(elemv);
		goto done;
	}

	/* Make sure sufficient memory has been allocated. */
	if (result_length > DeeList_GetAlloc(me)) {
		elemv = Dee_objectlist_elemv_tryrealloc(elemv, result_length);
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
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ DEFIMPL(&default__set_operator_inv),
	/* .tp_pos         = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg         = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_add,
	/* .tp_sub         = */ DEFIMPL(&default__set_operator_sub),
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_mul,
	/* .tp_div         = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod         = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl         = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr         = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and         = */ DEFIMPL(&default__set_operator_and),
	/* .tp_or          = */ DEFIMPL(&default__set_operator_add),
	/* .tp_xor         = */ DEFIMPL(&default__set_operator_xor),
	/* .tp_pow         = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__inplace_add),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&list_inplace_add,
	/* .tp_inplace_sub = */ DEFIMPL(&default__set_operator_inplace_sub),
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&list_inplace_mul,
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ DEFIMPL(&default__set_operator_inplace_and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__set_operator_inplace_add),
	/* .tp_inplace_xor = */ DEFIMPL(&default__set_operator_inplace_xor),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

PRIVATE struct type_member tpconst list_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeListIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeTuple_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
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
		if (temp != Dee_COMPARE_EQ)
			return temp;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return Dee_COMPARE_EQ;
nope:
	DeeList_LockEndRead(lhs);
	return Dee_COMPARE_NE;
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
			return Dee_Compare(lhsc, rhsc);
		}
		if (i >= rhsc)
			break;
		lhs_elem = DeeList_GET(lhs, i);
		Dee_Incref(lhs_elem);
		DeeList_LockEndRead(lhs);
		diff = DeeObject_Compare(lhs_elem, rhsv[i]);
		Dee_Decref(lhs_elem);
		if (diff != Dee_COMPARE_EQ)
			return diff;
		DeeList_LockRead(lhs);
	}
	DeeList_LockEndRead(lhs);
	return Dee_COMPARE_GR; /* COUNT(lhs) > COUNT(rhs) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_compare_eq(List *lhs, DeeObject *rhs) {
	if (DeeList_CheckExact(lhs)) {
		if (DeeTuple_Check(rhs))
			return list_compare_eq_v(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
		return default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast((DeeObject *)lhs, rhs);
	}
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index((DeeObject *)lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
list_compare(List *lhs, DeeObject *rhs) {
	if (DeeList_CheckExact(lhs)) {
		if (DeeTuple_Check(rhs))
			return list_compare_v(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
		return default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast((DeeObject *)lhs, rhs);
	}
	return default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index((DeeObject *)lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
list_hash(List *__restrict me) {
	size_t i;
	Dee_hash_t result;
	DREF DeeObject *elem;
	DeeList_LockRead(me);
	if unlikely(!me->l_list.ol_elemc) {
		DeeList_LockEndRead(me);
		return Dee_HASHOF_EMPTY_SEQUENCE;
	}
	elem = me->l_list.ol_elemv[0];
	Dee_Incref(elem);
	DeeList_LockEndRead(me);
	result = DeeObject_HashInherited(elem);
	DeeList_LockRead(me);
	for (i = 1; i < me->l_list.ol_elemc; ++i) {
		elem = me->l_list.ol_elemv[i];
		Dee_Incref(elem);
		DeeList_LockEndRead(me);
		result = Dee_HashCombine(result, DeeObject_HashInherited(elem));
		DeeList_LockRead(me);
	}
	DeeList_LockEndRead(me);
	return result;
}


PRIVATE struct type_cmp list_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&list_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE struct type_operator const list_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
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
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(List),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ List,
			/* tp_ctor:        */ &list_ctor,
			/* tp_copy_ctor:   */ &list_copy,
			/* tp_any_ctor:    */ &list_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &list_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&list_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&list_moveassign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&list_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&list_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&list_visit,
	/* .tp_gc            = */ &list_gc,
	/* .tp_math          = */ &list_math,
	/* .tp_cmp           = */ &list_cmp,
	/* .tp_seq           = */ &list_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ list_methods,
	/* .tp_getsets       = */ list_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ list_class_members,
	/* .tp_method_hints  = */ list_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ list_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(list_operators),
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
li_init(ListIterator *__restrict self,
        size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("List", params: "
	DeeListObject *list;
	size_t index = 0;
", docStringPrefix: "li");]]]*/
#define li_List_params "list:?DList,index=!0"
	struct {
		DeeListObject *list;
		size_t index;
	} args;
	args.index = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "List", &args, &args.list, "o", _DeeArg_AsObject, &args.index, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AssertType(args.list, &DeeList_Type))
		goto err;
	Dee_Incref(args.list);
	self->li_index = args.index;
	self->li_list  = args.list;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
li_serialize(ListIterator *__restrict self,
             DeeSerial *__restrict writer,
             Dee_seraddr_t addr) {
	ListIterator *out = DeeSerial_Addr2Mem(writer, addr, ListIterator);
	out->li_index = LI_GETINDEX(self);
	return generic_proxy__serialize((ProxyObject *)self, writer, addr);
}

STATIC_ASSERT(offsetof(ListIterator, li_list) == offsetof(ProxyObject, po_obj));
#define li_fini  generic_proxy__fini_unlikely /* Lists are usually referenced externally; else you'd use a Tuple */
#define li_visit generic_proxy__visit

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
		Dee_Decref_unlikely(result);
		goto again;
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
li_hash(ListIterator *self) {
	return Dee_HashCombine(Dee_HashPointer(self->li_list),
	                       LI_GETINDEX(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
li_compare(ListIterator *self, ListIterator *other) {
	if (DeeObject_AssertTypeExact(other, &DeeListIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->li_list, other->li_list);
	Dee_return_compareT(size_t, LI_GETINDEX(self),
	                    /*   */ LI_GETINDEX(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_iterator_mh_iter_revert(ListIterator *__restrict self, size_t step) {
	size_t old_index, new_index;
	do {
		old_index = atomic_read(&self->li_index);
		if (OVERFLOW_USUB(old_index, step, &new_index))
			new_index = 0;
	} while (!atomic_cmpxch_weak_or_write(&self->li_index, old_index, new_index));
	return old_index - new_index;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
list_iterator_mh_iter_advance(ListIterator *__restrict self, size_t step) {
	size_t old_index, new_index, list_size;
	do {
		old_index = atomic_read(&self->li_index);
		if (OVERFLOW_UADD(old_index, step, &new_index))
			new_index = (size_t)-1;
		list_size = DeeList_SIZE_ATOMIC(self->li_list);
		if (new_index > list_size)
			new_index = list_size;
		if (new_index <= old_index)
			return 0;
	} while (!atomic_cmpxch_weak_or_write(&self->li_index, old_index, new_index));
	return new_index - old_index;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_iterator_mh_iter_peek(ListIterator *__restrict self) {
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

PRIVATE struct type_cmp li_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&li_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&li_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_method tpconst list_iterator_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_HINTREF(Iterator_revert),
	TYPE_METHOD_HINTREF(Iterator_peek),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst list_iterator_method_hints[] = {
	TYPE_METHOD_HINT(iter_revert, &list_iterator_mh_iter_revert),
	TYPE_METHOD_HINT(iter_advance, &list_iterator_mh_iter_advance),
	TYPE_METHOD_HINT(iter_peek, &list_iterator_mh_iter_peek),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst list_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(ListIterator, li_list), "->?DList"),
	TYPE_MEMBER_FIELD(STR_index, STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(ListIterator, li_index)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeListIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ListIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(" li_List_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ListIterator,
			/* tp_ctor:        */ &li_ctor,
			/* tp_copy_ctor:   */ &li_copy,
			/* tp_any_ctor:    */ &li_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &li_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&li_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&li_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&li_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &li_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&li_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ list_iterator_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ list_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ list_iterator_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_LIST_C */
