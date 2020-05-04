/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_TUPLE_C
#define GUARD_DEEMON_OBJECTS_TUPLE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/string.h>

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <stddef.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#if defined(CONFIG_NO_CACHES) || defined(CONFIG_NO_TUPLE_CACHES)
#undef CONFIG_TUPLE_CACHE_MAXSIZE
#define CONFIG_TUPLE_CACHE_MAXSIZE  0
#undef CONFIG_TUPLE_CACHE_MAXCOUNT
#define CONFIG_TUPLE_CACHE_MAXCOUNT 0
#endif /* CONFIG_NO_CACHES || CONFIG_NO_TUPLE_CACHES */

/* The max amount of tuples per cache */
#ifndef CONFIG_TUPLE_CACHE_MAXSIZE
#define CONFIG_TUPLE_CACHE_MAXSIZE   64
#endif /* !CONFIG_TUPLE_CACHE_MAXSIZE */

/* The max tuple length for which a cache is kept */
#ifndef CONFIG_TUPLE_CACHE_MAXCOUNT
#define CONFIG_TUPLE_CACHE_MAXCOUNT  8
#endif /* !CONFIG_TUPLE_CACHE_MAXCOUNT */

#if !CONFIG_TUPLE_CACHE_MAXSIZE
#undef CONFIG_TUPLE_CACHE_MAXCOUNT
#define CONFIG_TUPLE_CACHE_MAXCOUNT 0
#endif /* !CONFIG_TUPLE_CACHE_MAXSIZE */

#include <hybrid/minmax.h>
#if CONFIG_TUPLE_CACHE_MAXCOUNT && !defined(CONFIG_NO_THREADS)
#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT && !CONFIG_NO_THREADS */

DECL_BEGIN

typedef DeeTupleObject Tuple;



#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
struct cached_object {
	struct cached_object *co_next;
};

struct tuple_cache {
	size_t                c_count; /* [lock(c_lock)][<= CONFIG_TUPLE_CACHE_MAXSIZE] Amount of cached objects int `c_head' */
	struct cached_object *c_head;  /* [lock(c_lock)][0..1] Linked list of cached tuple objects. */
#ifndef CONFIG_NO_THREADS
	rwlock_t              c_lock;  /* Lock for this tuple cache. */
#endif /* !CONFIG_NO_THREADS */
};

#ifdef CONFIG_NO_THREADS
#define LOCK(x)   (void)0
#define UNLOCK(x) (void)0
#else /* CONFIG_NO_THREADS */
#define LOCK(x)   rwlock_write(&(x))
#define UNLOCK(x) rwlock_endwrite(&(x))
#endif /* !CONFIG_NO_THREADS */

PRIVATE struct tuple_cache cache[CONFIG_TUPLE_CACHE_MAXCOUNT];

INTERN size_t DCALL
tuplecache_clear(size_t max_clear) {
	size_t result = 0;
	struct tuple_cache *iter;
	for (iter = cache;
	     iter != COMPILER_ENDOF(cache); ++iter) {
		struct cached_object *elem;
		LOCK(iter->c_lock);
		while ((elem = iter->c_head) != NULL && result < max_clear) {
			iter->c_head = elem->co_next;
			ASSERT(iter->c_count);
			--iter->c_count;
			result += (offsetof(DeeTupleObject, t_elem) +
			           ((size_t)(iter - cache + 1) * sizeof(DeeObject *)));
			DeeObject_Free(elem);
		}
		ASSERT((iter->c_head != NULL) ==
		       (iter->c_count != 0));
		UNLOCK(iter->c_lock);
		if (result >= max_clear)
			break;
	}
	return result;
}

#else /* CONFIG_TUPLE_CACHE_MAXCOUNT */

INTERN size_t DCALL
tuplecache_clear(size_t UNUSED(max_clear)) {
	return 0;
}

#endif /* !CONFIG_TUPLE_CACHE_MAXCOUNT */


PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_NewUninitialized(size_t n) {
	DREF DeeTupleObject *result;
	if unlikely(!n)
		return_empty_tuple;
#if CONFIG_TUPLE_CACHE_MAXCOUNT
	if (n < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[n - 1];
		if (c->c_count) {
			LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (c->c_head)
#endif /* !CONFIG_NO_THREADS */
			{
				ASSERT(c->c_head != NULL);
				result    = (DREF DeeTupleObject *)c->c_head;
				c->c_head = ((struct cached_object *)result)->co_next;
				--c->c_count;
				ASSERT((c->c_count == 0) == (c->c_head == NULL));
				ASSERT(result->t_size == n);
				UNLOCK(c->c_lock);
				goto got_result;
			}
			UNLOCK(c->c_lock);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	result = (DREF DeeTupleObject *)DeeObject_Malloc(offsetof(DeeTupleObject, t_elem) +
	                                                 (n * sizeof(DeeObject *)));
	if unlikely(!result)
		goto done;
	result->t_size = n;
#if CONFIG_TUPLE_CACHE_MAXCOUNT
got_result:
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	DeeObject_Init(result, &DeeTuple_Type);
#ifndef NDEBUG
	MEMSET_PTR(result->t_elem, 0xcc, n);
#endif /* !NDEBUG */
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_TryNewUninitialized(size_t n) {
	DREF DeeTupleObject *result;
	if unlikely(!n)
		return_empty_tuple;
#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
	if (n < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[n - 1];
		if (c->c_count) {
			LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (c->c_head)
#endif /* !CONFIG_NO_THREADS */
			{
				ASSERT(c->c_head != NULL);
				result    = (DREF DeeTupleObject *)c->c_head;
				c->c_head = ((struct cached_object *)result)->co_next;
				--c->c_count;
				ASSERT((c->c_count == 0) == (c->c_head == NULL));
				ASSERT(result->t_size == n);
				UNLOCK(c->c_lock);
				goto got_result;
			}
			UNLOCK(c->c_lock);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */
	result = (DREF DeeTupleObject *)DeeObject_TryMalloc(offsetof(DeeTupleObject, t_elem) +
	                                                    (n * sizeof(DeeObject *)));
	if unlikely(!result)
		goto done;
	result->t_size = n;
#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
got_result:
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */
	DeeObject_Init(result, &DeeTuple_Type);
#ifndef NDEBUG
	MEMSET_PTR(result->t_elem, 0xcc, n);
#endif /* !NDEBUG */
done:
	return (DREF DeeObject *)result;
}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
INTERN NONNULL((1)) void DCALL
tuple_tp_free(void *__restrict ob) {
	ASSERT(ob);
	ASSERT(!DeeTuple_IsEmpty((DeeObject *)ob));
	ASSERT(DeeTuple_SIZE((DeeObject *)ob) != 0);
#ifndef NDEBUG
	MEMSET_PTR(DeeTuple_ELEM((DeeObject *)ob), 0xcc,
	           DeeTuple_SIZE((DeeObject *)ob));
#endif /* !NDEBUG */
	if (DeeTuple_SIZE((DeeObject *)ob) < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[DeeTuple_SIZE((DeeObject *)ob) - 1];
		if (c->c_count < CONFIG_TUPLE_CACHE_MAXSIZE) {
			LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (c->c_count < CONFIG_TUPLE_CACHE_MAXSIZE)
#endif /* !CONFIG_NO_THREADS */
			{
				struct cached_object *cob;
				cob = (struct cached_object *)ob;
				cob->co_next = c->c_head;
				c->c_head    = cob;
				++c->c_count;
				UNLOCK(c->c_lock);
				return;
			}
			UNLOCK(c->c_lock);
		}
	}
	DeeObject_Free(ob);
}
#else /* CONFIG_TUPLE_CACHE_MAXCOUNT */
#define tuple_tp_free(ob) DeeObject_Free(ob)
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */

PUBLIC NONNULL((1)) void DCALL
DeeTuple_FreeUninitialized(DREF DeeObject *__restrict self) {
	ASSERT(self->ob_refcnt == 1);
	ASSERT(self->ob_type == &DeeTuple_Type);
	tuple_tp_free(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_ResizeUninitialized(DREF DeeObject *__restrict self,
                             size_t new_size) {
	DREF DeeTupleObject *new_tuple;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	if unlikely(DeeTuple_SIZE(self) == new_size)
		return self;
	if unlikely(DeeTuple_IsEmpty(self)) {
		/* Special case: Must not resize the empty tuple. */
		new_tuple = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(new_size);
		if likely(new_tuple)
			Dee_DecrefNokill(self);
		return (DREF DeeObject *)new_tuple;
	}
	ASSERT(DeeTuple_SIZE(self) != 0);
	ASSERTF(self->ob_refcnt == 1, "The tuple is being shared");
	if unlikely(!new_size) {
		/* Special case: Resize to an empty tuple. */
		Dee_DecrefNokill(&DeeTuple_Type);
		tuple_tp_free(self);
		return_empty_tuple;
	}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
	/* Check if we can use a cached tuple. */
	if (new_size < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[new_size - 1];
		if (c->c_count) {
			LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (c->c_head)
#endif /* !CONFIG_NO_THREADS */
			{
				ASSERT(c->c_head);
				new_tuple = (DREF DeeTupleObject *)c->c_head;
				c->c_head = ((struct cached_object *)new_tuple)->co_next;
				--c->c_count;
				UNLOCK(c->c_lock);
				/* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
				MEMCPY_PTR(new_tuple, self,
				           (offsetof(DeeTupleObject, t_elem) / sizeof(void *)) +
				           MIN(DeeTuple_SIZE(self), new_size));
#ifndef NDEBUG
				if (new_size > DeeTuple_SIZE(self)) {
					MEMSET_PTR(&new_tuple->t_elem[DeeTuple_SIZE(self)], 0xcc,
					           new_size - DeeTuple_SIZE(self));
				}
#endif /* !NDEBUG */
				tuple_tp_free(self);
				new_tuple->t_size = new_size;
				return (DREF DeeObject *)new_tuple;
			}
			UNLOCK(c->c_lock);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	/* Resize the old tuple. */
	new_tuple = (DREF DeeTupleObject *)DeeObject_Realloc(self,
	                                                     offsetof(DeeTupleObject, t_elem) +
	                                                     new_size * sizeof(DREF DeeObject *));
	if unlikely(!new_tuple)
		return NULL;
#ifndef NDEBUG
	if (new_size > new_tuple->t_size) {
		MEMSET_PTR(&new_tuple->t_elem[new_tuple->t_size],
		           0xcc, new_size - new_tuple->t_size);
	}
#endif /* !NDEBUG */
	new_tuple->t_size = new_size;
	return (DREF DeeObject *)new_tuple;
}

PUBLIC WUNUSED ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_TruncateUninitialized(DREF DeeObject *__restrict self,
                               size_t new_size) {
	DREF DeeTupleObject *new_tuple;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(new_size <= DeeTuple_SIZE(self));
	if unlikely(DeeTuple_SIZE(self) == new_size)
		return self;
	ASSERT(!DeeTuple_IsEmpty(self));
	ASSERT(DeeTuple_SIZE(self) != 0);
	ASSERTF(self->ob_refcnt == 1, "The tuple is being shared");
	if unlikely(!new_size) {
		/* Special case: Resize to an empty tuple. */
		Dee_DecrefNokill(&DeeTuple_Type);
		tuple_tp_free(self);
		return_empty_tuple;
	}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
	/* Check if we can use a cached tuple. */
	if (new_size < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[new_size - 1];
		if (c->c_count) {
			LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (c->c_head)
#endif /* !CONFIG_NO_THREADS */
			{
				ASSERT(c->c_head);
				new_tuple = (DREF DeeTupleObject *)c->c_head;
				c->c_head = ((struct cached_object *)new_tuple)->co_next;
				--c->c_count;
				UNLOCK(c->c_lock);
				/* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
				MEMCPY_PTR(new_tuple, self,
				           (offsetof(DeeTupleObject, t_elem) / sizeof(void *)) +
				           MIN(DeeTuple_SIZE(self), new_size));
#ifndef NDEBUG
				if (new_size > DeeTuple_SIZE(self)) {
					MEMSET_PTR(&new_tuple->t_elem[DeeTuple_SIZE(self)], 0xcc,
					           new_size - DeeTuple_SIZE(self));
				}
#endif /* !NDEBUG */
				tuple_tp_free(self);
				new_tuple->t_size = new_size;
				return (DREF DeeObject *)new_tuple;
			}
			UNLOCK(c->c_lock);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	/* Try to resize the old tuple. */
	new_tuple = (DREF DeeTupleObject *)DeeObject_TryRealloc(self, offsetof(DeeTupleObject, t_elem) +
	                                                              new_size * sizeof(DREF DeeObject *));
	if unlikely(!new_tuple)
		new_tuple = (DREF DeeTupleObject *)self;
	new_tuple->t_size = new_size;
	return (DREF DeeObject *)new_tuple;
}


PUBLIC WUNUSED NONNULL((2)) DREF DeeObject *DCALL
DeeTuple_NewVector(size_t objc, DeeObject *const *__restrict objv) {
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto done;
	Dee_Movrefv(DeeTuple_ELEM(result), objv, objc);
done:
	return result;
}

PUBLIC WUNUSED NONNULL((2)) DREF DeeObject *DCALL
DeeTuple_NewVectorSymbolic(size_t objc, DeeObject *const *__restrict objv) {
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto done;
	MEMCPY_PTR(DeeTuple_ELEM(result), objv, objc);
done:
	return result;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_FromSequence(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t i, seq_length;
	ASSERT_OBJECT(self);
	/* Optimizations for specific types such as `tuple' and `list' */
	if (DeeTuple_CheckExact(self))
		return_reference_(self);
	if (DeeList_CheckExact(self)) {
		seq_length = ATOMIC_READ(DeeList_SIZE(self));
list_size_changed:
		result = DeeTuple_NewUninitialized(seq_length);
		if unlikely(!result)
			goto err;
		COMPILER_READ_BARRIER();
		DeeList_LockRead(self);
		if unlikely(seq_length != DeeList_SIZE(self)) {
			seq_length = DeeList_SIZE(self);
			DeeList_LockEndRead(self);
			DeeTuple_FreeUninitialized(result);
			goto list_size_changed;
		}
		Dee_Movrefv(DeeTuple_ELEM(result),
		            DeeList_ELEM(self),
		            seq_length);
		DeeList_LockEndRead(self);
		goto done;
	}
	/* Optimization for fast-sequence compatible objects. */
	seq_length = DeeFastSeq_GetSize(self);
	if (seq_length != DEE_FASTSEQ_NOTFAST) {
		DREF DeeObject *elem;
		if (seq_length == 0)
			return_empty_tuple;
		result = DeeTuple_NewUninitialized(seq_length);
		if unlikely(!result)
			goto err;
		for (i = 0; i < seq_length; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err_r;
			DeeTuple_SET(result, i, elem); /* Inherit reference. */
		}
		goto done;
	}
	/* Use general-purpose iterators to create a new tuple. */
	self = DeeObject_IterSelf(self);
	if unlikely(!self)
		goto err;
	result = DeeTuple_FromIterator(self);
	Dee_Decref(self);
done:
	return result;
err_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_FromIterator(DeeObject *__restrict self) {
	DREF DeeObject *elem, *next;
	size_t used_size;
	DREF DeeTupleObject *result;
	/* Optimizations for empty- and single-element iterators. */
	elem = DeeObject_IterNext(self);
	if unlikely(!elem)
		goto err;
	if (elem == ITER_DONE)
		return_empty_tuple;
	next = DeeObject_IterNext(self);
	if unlikely(!next)
		goto err_elem;
	if (next == ITER_DONE) {
		result = (DREF DeeTupleObject *)DeeTuple_PackSymbolic(1, elem);
		if unlikely(!result)
			goto err_elem;
		return (DREF DeeObject *)result;
	}
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(2);
	if unlikely(!result) {
		Dee_Decref(next);
		goto err_elem;
	}
	result->t_elem[0] = elem;
	result->t_elem[1] = next;
	used_size         = 2;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		ASSERT(used_size <= result->t_size);
		if (used_size >= result->t_size) {
			/* Allocate more memory. */
			DREF DeeObject *new_result;
			new_result = DeeTuple_ResizeUninitialized((DeeObject *)result,
			                                          result->t_size * 2);
			if unlikely(!new_result) {
				Dee_Decref(elem);
				goto err_cleanup;
			}
			result = (DREF DeeTupleObject *)new_result;
		}
		result->t_elem[used_size++] = elem; /* Inherit reference. */
		if (DeeThread_CheckInterrupt())
			goto err_cleanup;
	}
	if unlikely(!elem) {
err_cleanup:
		/* Cleanup elements we've already assigned. */
		while (used_size--)
			Dee_Decref(result->t_elem[used_size]);
		Dee_DecrefNokill(&DeeTuple_Type);
		tuple_tp_free(result);
		result = NULL;
	} else if (used_size != result->t_size) {
		/* Fix the actually used size. */
#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
		if (used_size < CONFIG_TUPLE_CACHE_MAXCOUNT) {
			DREF DeeTupleObject *new_tuple;
			struct tuple_cache *c = &cache[used_size - 1];
			if (c->c_count) {
				LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
				COMPILER_READ_BARRIER();
				if (c->c_head)
#endif /* !CONFIG_NO_THREADS */
				{
					ASSERT(c->c_head);
					new_tuple = (DREF DeeTupleObject *)c->c_head;
					c->c_head = ((struct cached_object *)new_tuple)->co_next;
					--c->c_count;
					UNLOCK(c->c_lock);
					/* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
					ASSERT(used_size < DeeTuple_SIZE(result));
					MEMCPY_PTR(new_tuple, result,
					           (offsetof(DeeTupleObject, t_elem) / sizeof(void *)) +
					           used_size);
					tuple_tp_free(result);
					new_tuple->t_size = used_size;
					ASSERT(new_tuple->ob_refcnt == 1);
					return (DREF DeeObject *)new_tuple;
				}
				UNLOCK(c->c_lock);
			}
		}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
		ASSERT(result->ob_refcnt == 1);
		next = (DREF DeeObject *)DeeObject_TryRealloc(result, offsetof(DeeTupleObject, t_elem) +
		                                                      used_size * sizeof(DREF DeeObject *));
		if likely(next)
			result = (DREF DeeTupleObject *)next;
		result->t_size = used_size;
	}
	return (DREF DeeObject *)result;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_Types(DeeObject *__restrict self) {
	size_t i, count;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	ASSERT(DeeTuple_Check(self));
	count  = DeeTuple_SIZE(self);
	result = DeeTuple_NewUninitialized(count);
	if unlikely(!result)
		goto done;
	for (i = 0; i < count; ++i) {
		DeeTypeObject *tp;
		tp = Dee_TYPE(DeeTuple_GET(self, i));
		Dee_Incref(tp);
		DeeTuple_SET(result, i, (DeeObject *)tp);
	}
done:
	return result;
}

#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeTuple_VPack, 8),
                    ASSEMBLY_NAME(DeeTuple_NewVector, 8));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeTuple_VPackSymbolic, 8),
                    ASSEMBLY_NAME(DeeTuple_NewVectorSymbolic, 8));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPack(size_t n, va_list args) {
	return DeeTuple_NewVector(n, (DeeObject **)args);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPackSymbolic(size_t n, va_list args) {
	return DeeTuple_NewVectorSymbolic(n, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPack(size_t n, va_list args) {
	size_t i;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(n);
	if unlikely(!result)
		goto done;
	for (i = 0; i < n; ++i) {
		DREF DeeObject *elem;
		elem = va_arg(args, DeeObject *);
		Dee_Incref(elem);
		DeeTuple_SET(result, i, elem);
	}
done:
	return result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPackSymbolic(size_t n, va_list args) {
	size_t i;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(n);
	if unlikely(!result)
		goto done;
	for (i = 0; i < n; ++i) {
		DREF DeeObject *elem;
		elem = va_arg(args, DeeObject *);
		DeeTuple_SET(result, i, elem);
	}
done:
	return result;
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */

PUBLIC WUNUSED DREF DeeObject *DeeTuple_Pack(size_t n, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, n);
	result = DeeTuple_VPack(n, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED DREF DeeObject *DeeTuple_PackSymbolic(size_t n, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, n);
	result = DeeTuple_VPackSymbolic(n, args);
	va_end(args);
	return result;
}


PUBLIC NONNULL((1)) void DCALL
DeeTuple_DecrefSymbolic(DeeObject *__restrict self) {
	ASSERT_OBJECT(self);
	ASSERT(DeeTuple_Check(self));
	if (!DeeObject_IsShared(self)) {
		DeeTuple_FreeUninitialized(self);
	} else {
		Dee_Increfv(DeeTuple_ELEM(self),
		            DeeTuple_SIZE(self));
		Dee_Decref(self);
	}
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_AppendIterator(DREF DeeObject *__restrict self,
                        DeeObject *__restrict iterator) {
	DREF DeeObject *elem, *result;
	size_t incfactor = 2;
	size_t used_size = DeeTuple_SIZE(self);
	ASSERT(self != iterator);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (used_size == DeeTuple_SIZE(self)) {
			/* Must increase the tuple's size. */
			result = DeeTuple_ResizeUninitialized(self, used_size + incfactor);
			if unlikely(!result) {
				Dee_Decref(elem);
				goto err;
			}
			self = result;
			incfactor *= 2;
		}
		DeeTuple_SET(self, used_size, elem); /* Inherit reference. */
		++used_size;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	self = DeeTuple_TruncateUninitialized(self, used_size);
	if unlikely(!elem)
		goto err;
	return self;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_Append(DREF DeeObject *__restrict self,
                DeeObject *__restrict item) {
	DREF DeeObject *result;
	size_t index;
	ASSERT(self != item);
	/* Must increase the tuple's size. */
	index  = DeeTuple_SIZE(self);
	result = DeeTuple_ResizeUninitialized(self, index + 1);
	if likely(result) {
		DeeTuple_SET(result, index, item);
		Dee_Incref(item);
	}
	return result;
}


PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeTuple_ExtendInherited(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc,
                         /*inherit(on_success)*/ DREF DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	ASSERT_OBJECT(self);
	ASSERT(DeeTuple_Check(self));
	if (!DeeObject_IsShared(self)) {
		size_t old_size; /* Optimization: The old object can be re-used. */
		old_size = ((DREF DeeTupleObject *)self)->t_size;
		result   = (DREF DeeTupleObject *)DeeTuple_ResizeUninitialized(self, old_size + argc);
		if unlikely(!result)
			goto err;
		MEMCPY_PTR(result->t_elem + old_size, argv, argc);
	} else if unlikely(!argc) {
		result = (DREF DeeTupleObject *)self;
	} else {
		size_t mylen = DeeTuple_SIZE(self);
		result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(mylen + argc);
		if unlikely(!result)
			goto err;
		Dee_Movrefv(DeeTuple_ELEM(result), DeeTuple_ELEM(self), mylen);
		Dee_Decref_unlikely(self);
		MEMCPY_PTR(DeeTuple_ELEM(result) + mylen, argv, argc);
	}
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_ConcatInherited(/*inherit(on_success)*/ DREF DeeObject *self, DeeObject *sequence) {
	DREF DeeTupleObject *result;
	size_t oldsize;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(sequence);
	ASSERT(DeeTuple_Check(self));
	oldsize = DeeTuple_SIZE(self);
	if (!DeeObject_IsShared(self)) {
		/* Re-use `self'. */
		result = (DREF DeeTupleObject *)self;
		/* Optimizations for specific types. */
#if 0
		if (DeeTuple_CheckExact(sequence)) {
			DREF DeeTupleObject *new_result;
			size_t addsize = DeeTuple_SIZE(sequence);
			new_result = (DREF DeeTupleObject *)DeeTuple_ResizeUninitialized(result, oldsize + addsize);
			if unlikely(!new_result)
				goto err_inherit;
			result = new_result;
			Dee_Movrefv(result->t_elem, DeeTuple_ELEM(sequence), addsize);
			goto done;
		}
#endif
		if (DeeList_CheckExact(sequence)) {
			DREF DeeTupleObject *new_result;
			size_t lstsize = ATOMIC_READ(DeeList_SIZE(sequence));
			if unlikely(!lstsize)
				goto done;
handle_list_size:
			new_result = (DREF DeeTupleObject *)DeeTuple_ResizeUninitialized((DeeObject *)result,
			                                                                 oldsize + lstsize);
			if unlikely(!new_result) {
				ASSERT(!DeeObject_IsShared(result));
				Dee_Decrefv(DeeTuple_ELEM(result), oldsize);
				Dee_DecrefNokill(&DeeTuple_Type);
				tuple_tp_free(result);
				return NULL;
			}
			result = new_result;
			COMPILER_READ_BARRIER();
			DeeList_LockRead(sequence);
			/* Check if the list's size has changes in the mean time. */
			if unlikely(lstsize != DeeList_SIZE(sequence)) {
				lstsize = DeeList_SIZE(sequence);
				DeeList_LockEndRead(sequence);
				goto handle_list_size;
			}
			/* Copy elements from the list. */
			Dee_Movrefv(result->t_elem + oldsize,
			            DeeList_ELEM(sequence),
			            lstsize);
			DeeList_LockEndRead(sequence);
			goto done;
		}
	} else {
#if 0
		if (DeeTuple_CheckExact(sequence)) {
			size_t addsize = DeeTuple_SIZE(sequence);
			result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(oldsize +
			                                                          addsize);
			if unlikely(!result)
				goto err_inherit;
			Dee_Movrefv(DeeTuple_ELEM(result),
			            DeeTuple_ELEM(self),
			            oldsize);
			Dee_Movrefv(DeeTuple_ELEM(result) + oldsize,
			            DeeTuple_ELEM(sequence),
			            addsize);
			Dee_Decref_unlikely(self);
			goto done;
		}
#endif
		if (DeeList_CheckExact(sequence)) {
			size_t lstsize;
			lstsize = ATOMIC_READ(DeeList_SIZE(sequence));
handle_list_size2:
			result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(oldsize + lstsize);
			if unlikely(!result)
				goto err;
			COMPILER_READ_BARRIER();
			DeeList_LockRead(sequence);
			/* Make sure the list didn't change size in he mean time. */
			if unlikely(lstsize != DeeList_SIZE(sequence)) {
				lstsize = DeeList_SIZE(sequence);
				goto handle_list_size2;
			}
			Dee_Movrefv(DeeTuple_ELEM(result) + oldsize,
			            DeeList_ELEM(sequence), lstsize);
			DeeList_LockEndRead(sequence);
			Dee_Movrefv(DeeTuple_ELEM(result),
			            DeeTuple_ELEM(self), oldsize);
			Dee_Decref_unlikely(self);
			goto done;
		}

		/* Copy the `self' tuple, so-as to allow us to modify it. */
		result = (DREF DeeTupleObject *)DeeTuple_NewVector(oldsize, DeeTuple_ELEM(self));
		Dee_Decref_unlikely(self);
		if unlikely(!result)
			return NULL;
	}
	sequence = DeeObject_IterSelf(sequence);
	if unlikely(!sequence)
		goto err_result;
	{
		DREF DeeTupleObject *new_result;
		new_result = (DREF DeeTupleObject *)DeeTuple_AppendIterator((DeeObject *)result, sequence);
		Dee_Decref(sequence);
		if unlikely(!new_result)
			goto err_result;
		result = new_result;
	}
done:
	return (DREF DeeObject *)result;
err_result:
	if (result != (DREF DeeTupleObject *)self)
		Dee_DecrefDokill(result);
err:
	return NULL;
}










/*  ====== `Tuple.Iterator' type implementation ======  */
typedef struct {
	OBJECT_HEAD
	DREF DeeTupleObject *ti_tuple; /* [1..1][const] Referenced tuple. */
	DWEAK size_t         ti_index; /* [<= ti_tuple->t_size] Next-element index. */
} TupleIterator;

#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)            ((x)->ti_index)
#else /* CONFIG_NO_THREADS */
#define READ_INDEX(x) ATOMIC_READ((x)->ti_index)
#endif /* !CONFIG_NO_THREADS */

INTDEF DeeTypeObject DeeTupleIterator_Type;

PRIVATE NONNULL((1)) int DCALL
tuple_iterator_ctor(TupleIterator *__restrict self) {
	self->ti_tuple = (DREF DeeTupleObject *)Dee_EmptyTuple;
	self->ti_index = 0;
	Dee_Incref(Dee_EmptyTuple);
	return 0;
}

PRIVATE NONNULL((1, 2)) int DCALL
tuple_iterator_copy(TupleIterator *__restrict self,
                    TupleIterator *__restrict other) {
	self->ti_tuple = other->ti_tuple;
	Dee_Incref(self->ti_tuple);
	self->ti_index = READ_INDEX(other);
	return 0;
}

PRIVATE NONNULL((1, 2)) int DCALL
tuple_iterator_deep(TupleIterator *__restrict self,
                    TupleIterator *__restrict other) {
	self->ti_tuple = (DREF DeeTupleObject *)DeeObject_DeepCopy((DeeObject *)other->ti_tuple);
	if unlikely(!self->ti_tuple)
		goto err;
	self->ti_index = READ_INDEX(other);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 3)) int DCALL
tuple_iterator_init(TupleIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	self->ti_tuple = (DREF DeeTupleObject *)Dee_EmptyTuple;
	self->ti_index = 0;
	if (DeeArg_Unpack(argc, argv, "|oIu:_TupleIterator",
	                  &self->ti_tuple, &self->ti_index))
		goto err;
	if (DeeObject_AssertTypeExact(self->ti_tuple, &DeeTuple_Type))
		goto err;
	if (self->ti_index >= DeeTuple_SIZE(self->ti_tuple))
		goto err_bounds;
	Dee_Incref(self->ti_tuple);
	return 0;
err_bounds:
	err_index_out_of_bounds((DeeObject *)self->ti_tuple,
	                        self->ti_index,
	                        DeeTuple_SIZE(self->ti_tuple));
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
tuple_iterator_fini(TupleIterator *__restrict self) {
	Dee_Decref(self->ti_tuple);
}

PRIVATE NONNULL((1, 2)) void DCALL
tuple_iterator_visit(TupleIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ti_tuple);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_iterator_next(TupleIterator *__restrict self) {
	DREF DeeObject *result;
#ifdef CONFIG_NO_THREADS
	if (self->ti_index >= DeeTuple_SIZE(self->ti_tuple))
		return ITER_DONE;
	result = DeeTuple_GET(self->ti_tuple, self->ti_index);
	ASSERT_OBJECT(result);
	++self->ti_index;
	Dee_Incref(result);
#else /* CONFIG_NO_THREADS */
	size_t index;
	do {
		index = ATOMIC_READ(self->ti_index);
		if (index >= DeeTuple_SIZE(self->ti_tuple))
			return ITER_DONE;
	} while (!ATOMIC_CMPXCH(self->ti_index, index, index + 1));
	result = DeeTuple_GET(self->ti_tuple, index);
	ASSERT_OBJECT(result);
	Dee_Incref(result);
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE struct type_member tuple_iterator_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(TupleIterator, ti_tuple)),
	TYPE_MEMBER_FIELD("index", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(TupleIterator, ti_index)),
	TYPE_MEMBER_END
};

#define DEFINE_TUPLE_ITERATOR_COMPARE(name, op)                       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL             \
	name(TupleIterator *self, TupleIterator *other) {                 \
		if (DeeObject_AssertTypeExact(other, &DeeTupleIterator_Type)) \
			goto err;                                                 \
		return_bool(READ_INDEX(self) op READ_INDEX(other));           \
	err:                                                              \
		return NULL;                                                  \
	}
DEFINE_TUPLE_ITERATOR_COMPARE(tuple_iterator_eq, ==)
DEFINE_TUPLE_ITERATOR_COMPARE(tuple_iterator_ne, !=)
DEFINE_TUPLE_ITERATOR_COMPARE(tuple_iterator_lo, <)
DEFINE_TUPLE_ITERATOR_COMPARE(tuple_iterator_le, <=)
DEFINE_TUPLE_ITERATOR_COMPARE(tuple_iterator_gr, >)
DEFINE_TUPLE_ITERATOR_COMPARE(tuple_iterator_ge, >=)
#undef DEFINE_TUPLE_ITERATOR_COMPARE

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
tuple_iterator_nii_getseq(TupleIterator *__restrict self) {
	return_reference_(self->ti_tuple);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
tuple_iterator_nii_getindex(TupleIterator *__restrict self) {
	return READ_INDEX(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_setindex(TupleIterator *__restrict self, size_t new_index) {
#ifdef CONFIG_NO_THREADS
	self->ti_index = new_index;
#else /* CONFIG_NO_THREADS */
	ATOMIC_WRITE(self->ti_index, new_index);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_rewind(TupleIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	self->ti_index = 0;
#else /* CONFIG_NO_THREADS */
	ATOMIC_WRITE(self->ti_index, 0);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_revert(TupleIterator *__restrict self, size_t step) {
#ifdef CONFIG_NO_THREADS
	if (OVERFLOW_USUB(self->ti_index, step, &self->ti_index))
		self->ti_index = 0;
#else /* CONFIG_NO_THREADS */
	size_t old_index, new_index;
	do {
		old_index = ATOMIC_READ(self->ti_index);
		if (OVERFLOW_USUB(old_index, step, &new_index))
			new_index = 0;
	} while (!ATOMIC_CMPXCH_WEAK(self->ti_index, old_index, new_index));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_advance(TupleIterator *__restrict self, size_t step) {
#ifdef CONFIG_NO_THREADS
	if (OVERFLOW_UADD(self->ti_index, step, &self->ti_index))
		self->ti_index = (size_t)-1;
#else /* CONFIG_NO_THREADS */
	size_t old_index, new_index;
	do {
		old_index = ATOMIC_READ(self->ti_index);
		if (OVERFLOW_UADD(old_index, step, &new_index))
			new_index = (size_t)-1;
	} while (!ATOMIC_CMPXCH_WEAK(self->ti_index, old_index, new_index));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_prev(TupleIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (!self->ti_index)
		return 1;
	--self->ti_index;
#else /* CONFIG_NO_THREADS */
	size_t old_index;
	do {
		old_index = ATOMIC_READ(self->ti_index);
		if (!old_index)
			return 1;
	} while (!ATOMIC_CMPXCH_WEAK(self->ti_index, old_index, old_index - 1));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_next(TupleIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (self->ti_index >= DeeTuple_SIZE(self->ti_tuple))
		return 1;
	++self->ti_index;
#else /* CONFIG_NO_THREADS */
	size_t old_index;
	do {
		old_index = ATOMIC_READ(self->ti_index);
		if (old_index >= DeeTuple_SIZE(self->ti_tuple))
			return 1;
	} while (!ATOMIC_CMPXCH_WEAK(self->ti_index, old_index, old_index + 1));
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_hasprev(TupleIterator *__restrict self) {
	return READ_INDEX(self) != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_iterator_nii_peek(TupleIterator *__restrict self) {
	DREF DeeObject *result;
	size_t index = READ_INDEX(self);
	result       = DeeTuple_GET(self->ti_tuple, index);
	ASSERT_OBJECT(result);
	Dee_Incref(result);
	return result;
}


PRIVATE struct type_nii tuple_iterator_nii = {
	/* .nii_class = */TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (void *)&tuple_iterator_nii_getseq,
			/* .nii_getindex = */ (void *)&tuple_iterator_nii_getindex,
			/* .nii_setindex = */ (void *)&tuple_iterator_nii_setindex,
			/* .nii_rewind   = */ (void *)&tuple_iterator_nii_rewind,
			/* .nii_revert   = */ (void *)&tuple_iterator_nii_revert,
			/* .nii_advance  = */ (void *)&tuple_iterator_nii_advance,
			/* .nii_prev     = */ (void *)&tuple_iterator_nii_prev,
			/* .nii_next     = */ (void *)&tuple_iterator_nii_next,
			/* .nii_hasprev  = */ (void *)&tuple_iterator_nii_hasprev,
			/* .nii_peek     = */ (void *)&tuple_iterator_nii_peek
		}
	}
};

PRIVATE struct type_cmp tuple_iterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_ge,
	/* .tp_nii  = */ &tuple_iterator_nii
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_bool(TupleIterator *__restrict self) {
	return READ_INDEX(self) < DeeTuple_SIZE(self->ti_tuple);
}

INTERN DeeTypeObject DeeTupleIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TupleIterator",
	/* .tp_doc      = */ DOC("(seq=!T0,index=!0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&tuple_iterator_ctor,
				/* .tp_copy_ctor = */ (void *)&tuple_iterator_copy,
				/* .tp_deep_ctor = */ (void *)&tuple_iterator_deep,
				/* .tp_any_ctor  = */ (void *)&tuple_iterator_init,
				TYPE_FIXED_ALLOCATOR(TupleIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&tuple_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&tuple_iterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&tuple_iterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &tuple_iterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_iterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ tuple_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};











/*  ====== `Tuple' type implementation ======  */

PRIVATE WUNUSED DREF Tuple *DCALL tuple_ctor(void) {
	return_reference_((DREF Tuple *)Dee_EmptyTuple);
}

INTERN WUNUSED NONNULL((1)) DREF Tuple *DCALL
tuple_deepcopy(Tuple *__restrict self) {
	DREF Tuple *result;
	size_t i, size = DeeTuple_SIZE(self);
	result = (DREF Tuple *)DeeTuple_NewUninitialized(size);
	if unlikely(!result)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *temp;
		temp = DeeObject_DeepCopy(DeeTuple_GET(self, i));
		if unlikely(!temp)
			goto err_r;
		DeeTuple_SET(result, i, temp); /* Inherit reference. */
	}
	return result;
err_r:
	Dee_Decrefv_likely(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Tuple *DCALL
tuple_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:Tuple", &seq))
		goto err;
	return (DREF Tuple *)DeeTuple_FromSequence(seq);
err:
	return NULL;
}

INTERN NONNULL((1)) void DCALL
tuple_fini(Tuple *__restrict self) {
	Dee_Decrefv(DeeTuple_ELEM(self),
	            DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_iter(Tuple *__restrict self) {
	DREF TupleIterator *result;
	result = DeeObject_MALLOC(TupleIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTupleIterator_Type);
	result->ti_index = 0;
	result->ti_tuple = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_size(Tuple *__restrict self) {
	return DeeInt_NewSize(DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_contains(Tuple *self, DeeObject *item) {
	int error;
	size_t i, mylen;
	mylen = DeeTuple_SIZE(self);
	for (i = 0; i < mylen; ++i) {
		DeeObject *ob;
		ob = DeeTuple_GET(self, i);
		error = DeeObject_CompareEq(item, ob);
		if unlikely(error < 0)
			goto err;
		if (error)
			return_true;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_getitem(Tuple *self, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	if unlikely(i >= DeeTuple_SIZE(self))
		goto err_bounds;
	return_reference(DeeTuple_GET(self, i));
err_bounds:
	err_index_out_of_bounds((DeeObject *)self,
	                        i, DeeTuple_SIZE(self));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getrange_i(Tuple *__restrict self,
                 dssize_t begin, dssize_t end) {
	if unlikely(begin < 0)
		begin += DeeTuple_SIZE(self);
	if unlikely(end < 0)
		end += DeeTuple_SIZE(self);
	if unlikely((size_t)begin >= DeeTuple_SIZE(self) ||
	            (size_t)begin >= (size_t)end)
		return_empty_tuple;
	if unlikely((size_t)end > DeeTuple_SIZE(self))
		end = (dssize_t)DeeTuple_SIZE(self);
	return DeeTuple_NewVector((size_t)(end - begin),
	                          DeeTuple_ELEM(self) + begin);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getrange_in(Tuple *__restrict self,
                  dssize_t begin) {
	if unlikely(begin < 0)
		begin += DeeTuple_SIZE(self);
	if unlikely((size_t)begin >= DeeTuple_SIZE(self))
		return_empty_tuple;
	return DeeTuple_NewVector(DeeTuple_SIZE(self) - (size_t)begin,
	                          DeeTuple_ELEM(self) + (size_t)begin);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tuple_getrange(Tuple *__restrict self,
               DeeObject *__restrict begin,
               DeeObject *__restrict end) {
	dssize_t i_begin, i_end = DeeTuple_SIZE(self);
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (!DeeNone_Check(end) && DeeObject_AsSSize(end, &i_end))
		goto err;
	return tuple_getrange_i(self, i_begin, i_end);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) size_t DCALL
tuple_nsi_getsize(Tuple *__restrict self) {
	ASSERT(self->t_size != (size_t)-1);
	return self->t_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_nsi_getitem(Tuple *__restrict self, size_t index) {
	if unlikely(index >= self->t_size)
		goto err_bounds;
	return_reference(self->t_elem[index]);
err_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, self->t_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_nsi_getitem_fast(Tuple *__restrict self, size_t index) {
	ASSERT(index < self->t_size);
	return_reference(self->t_elem[index]);
}

PRIVATE size_t DCALL
tuple_nsi_find(Tuple *__restrict self, size_t start, size_t end,
               DeeObject *__restrict keyed_search_item,
               DeeObject *key) {
	size_t i;
	int temp;
	if (end > self->t_size)
		end = self->t_size;
	for (i = start; i < end; ++i) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, self->t_elem[i], key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i;
		}
	}
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE size_t DCALL
tuple_nsi_rfind(Tuple *__restrict self, size_t start, size_t end,
                DeeObject *__restrict keyed_search_item,
                DeeObject *key) {
	size_t i;
	int temp;
	if (end > self->t_size)
		end = self->t_size;
	i = end;
	while (i > start) {
		--i;
		temp = DeeObject_CompareKeyEq(keyed_search_item, self->t_elem[i], key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i;
		}
	}
	return (size_t)-1;
err:
	return (size_t)-2;
}


PRIVATE struct type_nsi tuple_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&tuple_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&tuple_nsi_getsize,
			/* .nsi_getitem      = */ (void *)&tuple_nsi_getitem,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)&tuple_nsi_getitem_fast,
			/* .nsi_getrange     = */ (void *)&tuple_getrange_i,
			/* .nsi_getrange_n   = */ (void *)&tuple_getrange_in,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)&tuple_nsi_find,
			/* .nsi_rfind        = */ (void *)&tuple_nsi_rfind,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL
		}
	}
};


PRIVATE struct type_seq tuple_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&tuple_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &tuple_nsi
};

PRIVATE WUNUSED DREF DeeObject *DCALL
tuple_unpack(DeeObject *__restrict UNUSED(self),
             size_t argc, DeeObject *const *argv) {
	size_t num_items;
	DeeObject *init;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "Iuo:unpack", &num_items, &init))
		goto err;
	result = DeeTuple_NewUninitialized(num_items);
	if unlikely(!result)
		goto done;
	if unlikely(DeeObject_Unpack(init, num_items, DeeTuple_ELEM(result))) {
		DeeTuple_FreeUninitialized(result);
err:
		result = NULL;
	}
done:
	return result;
}

PRIVATE struct type_method tuple_class_methods[] = {
	{ "unpack",
	  &tuple_unpack,
	  DOC("(num:?Dint,seq:?S?O)->?.\n"
	      "@throw UnpackError The given @seq doesn't contain exactly @num elements\n"
	      "Unpack the given sequence @seq into a Tuple consisting of @num elements") },
	{ NULL }
};

PRIVATE struct type_member tuple_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeTupleIterator_Type),
	TYPE_MEMBER_END
};

INTERN NONNULL((1, 2)) void DCALL
tuple_visit(Tuple *__restrict self,
            dvisit_t proc, void *arg) {
	Dee_Visitv(DeeTuple_ELEM(self),
	           DeeTuple_SIZE(self));
}

/* Print all bytes from `self' encoded as UTF-8.
 * In other words, bytes that are non-ASCII (aka. 80-FF) are
 * encoded as 2-byte UTF-8 sequences (aka: as LATIN-1), allowing
 * them to be properly interpreted by the given `printer' */
INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeTuple_Print(DeeObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	size_t i;
	dssize_t temp, result = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		DeeObject *elem;
		elem = DeeTuple_GET(self, i);
		temp = DeeObject_Print(elem, printer, arg);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_str(Tuple *__restrict self) {
	/* Special case to facilitate function-like use of `print':
	 * >> print "foo", "bar";   // Prints "foo bar\n"
	 * >> print("foo", "bar");  // Prints "foobar\n"
	 * >> print "foo", "bar",;  // Prints "foo bar"
	 * >> print("foo", "bar"),; // Prints "foobar"
	 * Using this mechanic, printing multiple objects without having
	 * them separated by spaces becomes much simpler, especially since
	 * this way of implementing this mechanism is entirely opaque to
	 * user-code and fully compliant with Sequence requirements (since
	 * `Sequence' only mandates a proper implementation of `operator repr',
	 * but leaves `operator str' unspecified)
	 * Note that the compiler knows about this and will optimize away
	 * attempts of printing Tuple sequences at compile-time, reducing
	 * them to `print' instructions that ever actually create a tuple.
	 * >> print;                         // Prints "\n"
	 * >> print();                       // Prints "\n"
	 * >> print("foo", "bar");           // Prints "foobar\n"
	 * >> print Tuple { "foo", "bar" };  // Prints "foobar\n"
	 */
	size_t i;
	struct unicode_printer p = UNICODE_PRINTER_INIT;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		DeeObject *elem;
		elem = DeeTuple_GET(self, i);
		if (unicode_printer_printobject(&p, elem) < 0)
			goto err;
	}
	return unicode_printer_pack(&p);
err:
	unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_repr(Tuple *__restrict self) {
	size_t i, count;
	/* Special case: single-item tuples are
	 * must be encoded with a trailing comma. */
	if (DeeTuple_SIZE(self) == 1) {
		return DeeString_Newf("(%r,)", DeeTuple_GET(self, 0));
	} else {
		struct unicode_printer p = UNICODE_PRINTER_INIT;
		if (unicode_printer_putascii(&p, '(') < 0)
			goto err;
		count = DeeTuple_SIZE(self);
		for (i = 0; i < count; ++i) {
			/* Print this item. */
			if (i && UNICODE_PRINTER_PRINT(&p, ", ") < 0)
				goto err;
			if (unicode_printer_printobjectrepr(&p, DeeTuple_GET(self, i)) < 0)
				goto err;
		}
		if (unicode_printer_putascii(&p, ')') < 0)
			goto err;
		return unicode_printer_pack(&p);
err:
		unicode_printer_fini(&p);
	}
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_bool(Tuple *__restrict self) {
	return !DeeTuple_IsEmpty(self);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
tuple_hash(Tuple *__restrict self) {
	return DeeObject_Hashv(DeeTuple_ELEM(self),
	                       DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_sizeof(Tuple *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(offsetof(Tuple, t_elem) +
	                      (self->t_size * sizeof(DeeObject *)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_first(Tuple *__restrict self) {
	if unlikely(DeeTuple_IsEmpty(self))
		goto err_empty;
	return_reference_(DeeTuple_GET(self, 0));
err_empty:
	err_empty_sequence((DeeObject *)self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_last(Tuple *__restrict self) {
	if unlikely(DeeTuple_IsEmpty(self))
		goto err_empty;
	return_reference_(DeeTuple_GET(self, DeeTuple_SIZE(self) - 1));
err_empty:
	err_empty_sequence((DeeObject *)self);
	return NULL;
}


PRIVATE struct type_method tuple_methods[] = {
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&tuple_sizeof,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_getset tuple_getsets[] = {
	{ DeeString_STR(&str_first),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_first,
	  NULL,
	  NULL },
	{ DeeString_STR(&str_last),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_last,
	  NULL,
	  NULL },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_eq(Tuple *self, DeeObject *other) {
	int result;
	result = DeeSeq_EqVS(DeeTuple_ELEM(self),
	                     DeeTuple_SIZE(self),
	                     other);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_ne(Tuple *self, DeeObject *other) {
	int result;
	result = DeeSeq_EqVS(DeeTuple_ELEM(self),
	                     DeeTuple_SIZE(self),
	                     other);
	if unlikely(result < 0)
		return NULL;
	return_bool_(!result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_lo(Tuple *self, DeeObject *other) {
	int result;
	result = DeeSeq_LoVS(DeeTuple_ELEM(self),
	                     DeeTuple_SIZE(self),
	                     other);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_le(Tuple *self, DeeObject *other) {
	int result;
	result = DeeSeq_LeVS(DeeTuple_ELEM(self),
	                     DeeTuple_SIZE(self),
	                     other);
	if unlikely(result < 0)
		return NULL;
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_gr(Tuple *self, DeeObject *other) {
	int result;
	result = DeeSeq_LeVS(DeeTuple_ELEM(self),
	                     DeeTuple_SIZE(self),
	                     other);
	if unlikely(result < 0)
		return NULL;
	return_bool_(!result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_ge(Tuple *self, DeeObject *other) {
	int result;
	result = DeeSeq_LoVS(DeeTuple_ELEM(self),
	                     DeeTuple_SIZE(self),
	                     other);
	if unlikely(result < 0)
		return NULL;
	return_bool_(!result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_concat(Tuple *self,
             DeeObject *other) {
	DeeObject **dst;
	DREF DeeObject *result, *new_result;
	DREF DeeObject *elem, *iterator;
	size_t i, my_length, ot_length;
	my_length = DeeTuple_SIZE(self);
	if (my_length == 0)
		return DeeTuple_FromSequence(other);
	if (DeeTuple_Check(other)) {
		ot_length = DeeTuple_SIZE(other);
		if (ot_length == 0)
			goto return_self;
		result = DeeTuple_NewUninitialized(my_length + ot_length);
		if unlikely(!result)
			goto err;
		Dee_Movrefv(DeeTuple_ELEM(result),
		            DeeTuple_ELEM(self),
		            my_length);
		Dee_Movrefv(DeeTuple_ELEM(result) + my_length,
		            DeeTuple_ELEM(other),
		            ot_length);
		goto done;
	}
	ot_length = DeeFastSeq_GetSize(other);
	if (ot_length != DEE_FASTSEQ_NOTFAST) {
		if (ot_length == 0)
			goto return_self;
		result = DeeTuple_NewUninitialized(my_length + ot_length);
		if unlikely(!result)
			goto err;
		dst = Dee_Movprefv(DeeTuple_ELEM(result),
		                   DeeTuple_ELEM(self),
		                   my_length);
		for (i = 0; i < ot_length; ++i) {
			elem = DeeFastSeq_GetItem(other, i);
			if unlikely(!elem)
				goto err_r;
			*dst++ = elem; /* Inherit reference. */
		}
		goto done;
	}
	/* Fallback: use iterator. */
	result = DeeTuple_NewUninitialized(my_length + 8);
	if unlikely(!result)
		goto err;
	dst = Dee_Movprefv(DeeTuple_ELEM(result),
	                   DeeTuple_ELEM(self),
	                   my_length);
	iterator = DeeObject_IterSelf(other);
	if unlikely(!iterator)
		goto err_r;
	ot_length = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		ASSERT(my_length <= DeeTuple_SIZE(result));
		if (my_length >= DeeTuple_SIZE(result)) {
			new_result = DeeTuple_ResizeUninitialized(result, my_length * 2);
			if unlikely(!new_result)
				goto err_r;
			result = new_result;
		}
		ASSERT(my_length < DeeTuple_SIZE(result));
		DeeTuple_SET(result, my_length, elem); /* Inherit reference. */
		++my_length;
	}
	if unlikely(!elem)
		goto err_r_iterator;
	Dee_Decref(iterator);
	result = DeeTuple_TruncateUninitialized(result, my_length);
done:
	return result;
return_self:
	return_reference_((DeeObject *)self);
err_r_iterator:
	Dee_Decref(iterator);
err_r:
	Dee_Decrefv(DeeTuple_ELEM(result),
	            (size_t)(dst - DeeTuple_ELEM(result)));
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_repeat(Tuple *self,
             DeeObject *other) {
	size_t i, count, total_length, my_length;
	DREF DeeObject *result, **dst;
	if (DeeObject_AsSize(other, &count))
		goto err;
	if (!count)
		goto return_empty;
	if (count == 1)
		return_reference_((DeeObject *)self);
	/* Repeat `self' `count' number of times. */
	my_length = DeeTuple_SIZE(self);
	if (my_length == 0)
		goto return_empty;
	if (OVERFLOW_UMUL(my_length, count, &total_length))
		goto err_overflow;
	result = DeeTuple_NewUninitialized(total_length);
	if unlikely(!result)
		goto err;
	/* Create all the new references that will be contained in the new tuple. */
	for (i = 0; i < my_length; ++i)
		Dee_Incref_n(DeeTuple_GET(self, i), count);
	/* Fill in the resulting tuple with repetitions of ourself. */
	dst = DeeTuple_ELEM(result);
	while (count--) {
		MEMCPY_PTR(dst, DeeTuple_ELEM(self), my_length);
		dst += my_length;
	}
	return result;
return_empty:
	return_empty_tuple;
err_overflow:
	err_integer_overflow(other, sizeof(size_t) * 8, true);
err:
	return NULL;
}

PRIVATE struct type_math tuple_math = {
	/* .tp_int32  = */ NULL,
	/* .tp_int64  = */ NULL,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ NULL,
	/* .tp_neg    = */ NULL,
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_concat,
	/* .tp_sub    = */ NULL,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_repeat,
	/* .tp_div    = */ NULL,
	/* .tp_mod    = */ NULL,
	/* .tp_shl    = */ NULL,
	/* .tp_shr    = */ NULL,
	/* .tp_and    = */ NULL,
	/* .tp_or     = */ NULL,
	/* .tp_xor    = */ NULL,
	/* .tp_pow    = */ NULL,
};

PRIVATE struct type_cmp tuple_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&tuple_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_ge,
};


PUBLIC DeeTypeObject DeeTuple_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Tuple),
	/* .tp_doc      = */ DOC("A builtin type that is similar to :list, however represents a fixed-length, "
	                         "immutable sequence of objects. Tuples are fast, low-level :Sequence-like objects "
	                         "that are written as ${(elem1, elem2, etc)}, with the exception of single-element "
	                         "tuples being written as ${(single_element,)}\n"

	                         "\n"
	                         "()\n"
	                         "Construct an empty Tuple\n"

	                         "\n"
	                         "(items:?S?O)\n"
	                         "Construct a new Tuple that is pre-initializes with the elements from @items\n"

	                         "\n"
	                         "str->\n"
	                         "Returns the concatenation of all of @this Tuple's elements converted to strings:\n"
	                         "${"
	                         "operator str() {\n"
	                         "	return \"\".join(this);\n"
	                         "}}\n"

	                         "\n"
	                         "repr->\n"
	                         "Returns a representation of @this Tuple:\n"
	                         "${"
	                         "operator repr() {\n"
	                         "	if (#this == 1)\n"
	                         "		return \"({!r},)\".format({ this[0] });\n"
	                         "	return \"(\" + \", \".join(for (local x: this) repr x) + \")\";\n"
	                         "}}\n"

	                         "\n"
	                         "bool->\n"
	                         "Returns ?t if @this Tuple is non-empty\n"

	                         "\n"
	                         "+->\n"
	                         "+(other:?S?O)->\n"
	                         "@throw NotImplemented The given @other isn't iterable\n"
	                         "Returns a new Tuple consisting of the elements from @this, followed by "
	                         "those from @other, which may be another Tuple, or a generic sequence\n"

	                         "\n"
	                         "*(count:?Dint)->\n"
	                         "@throw IntegerOverflow The given @count is negative, or too large\n"
	                         "Return a new Tuple consisting of the elements from @this, repeated @count times\n"
	                         "When @count is $0, an empty Tuple is returned. When @count is $1, @this Tuple is re-returned\n"

	                         "\n"
	                         "==->\n"
	                         "!=->\n"
	                         "<->\n"
	                         "<=->\n"
	                         ">->\n"
	                         ">=->\n"
	                         "Perform a lexicographical comparison between the elements of @this "
	                         "Tuple and the given @other sequence\n"

	                         "\n"
	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this Tuple\n"

	                         "\n"
	                         "#->\n"
	                         "Returns the number of elements contained inside of @this Tuple\n"

	                         "\n"
	                         "contains->\n"
	                         "Returns ?t if @elem is apart of @this Tuple, or @false otherwise\n"

	                         "\n"
	                         "[]->\n"
	                         "@throw IntegerOverflow The given @index is negative, or too large\n"
	                         "@throw IndexError The given @index is out of bounds\n"
	                         "Returns the @index'th item of @this Tuple\n"

	                         "\n"
	                         "[:]->?.\n"
	                         "Returns a new Tuple for the given subrange, following the usual rules for "
	                         "negative @start or @end values, as well as ?N being passed for either "
	                         "(s.a. :Sequence.op:getrange)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)&tuple_ctor,
				/* .tp_copy_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (void *)&tuple_deepcopy,
				/* .tp_any_ctor  = */ (void *)&tuple_init,
#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
				/* .tp_free      = */ (void *)&tuple_tp_free
#else /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */
				/* .tp_free      = */ (void *)NULL
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT == 0 */
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&tuple_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&tuple_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&tuple_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &tuple_math,
	/* .tp_cmp           = */ &tuple_cmp,
	/* .tp_seq           = */ &tuple_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ tuple_methods,
	/* .tp_getsets       = */ tuple_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ tuple_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ tuple_class_members
};

PUBLIC struct empty_tuple_object DeeTuple_Empty = {
	4, /* +1 in contrast to OBJECT_HEAD_INIT for the reference
	    * saved as the original value for `Dee_GetArgv()' */
	&DeeTuple_Type,
#ifdef CONFIG_TRACE_REFCHANGES
	DEE_REFTRACKER_UNTRACKED,
#endif
	0
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TUPLE_C */
