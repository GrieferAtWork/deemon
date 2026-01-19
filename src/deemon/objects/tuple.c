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
#ifndef GUARD_DEEMON_OBJECTS_TUPLE_C
#define GUARD_DEEMON_OBJECTS_TUPLE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/host.h> /* __ARCH_VA_LIST_IS_STACK_POINTER */
#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/default-compare.h"
#include "seq/sort.h"

#include <stdarg.h> /* va_arg, va_end, va_list, va_start */
#include <stddef.h> /* NULL, offsetof, size_t */

#undef SSIZE_MIN
#undef SSIZE_MAX
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

#if defined(CONFIG_NO_CACHES) || defined(CONFIG_NO_TUPLE_CACHES)
#undef CONFIG_TUPLE_CACHE_MAXSIZE
#define CONFIG_TUPLE_CACHE_MAXSIZE  0
#undef CONFIG_TUPLE_CACHE_MAXCOUNT
#define CONFIG_TUPLE_CACHE_MAXCOUNT 0
#endif /* CONFIG_NO_CACHES || CONFIG_NO_TUPLE_CACHES */

/* In order for it to be possible to write "Tuple" to a dec file,
 * there mustn't be a custom "tp_free" operator. Since the whole
 * point of mmap-able .dec files is to use pointers into a file
 * mapping of the .dec file as the actual DeeObject, that object
 * must not use a custom allocation mechanism (since it _needs_
 * to be able to live within a `struct Dee_heapregion') */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#undef CONFIG_TUPLE_CACHE_MAXSIZE
#define CONFIG_TUPLE_CACHE_MAXSIZE 0
#undef CONFIG_TUPLE_CACHE_MAXCOUNT
#define CONFIG_TUPLE_CACHE_MAXCOUNT 0
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */


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

#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
#include <deemon/util/lock.h>

#include <hybrid/minmax.h>
#include <hybrid/sequence/list.h>
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeTupleObject Tuple;

#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
struct tuple_cache_item {
	SLIST_ENTRY(tuple_cache_item) tci_link;
};

SLIST_HEAD(tuple_cache_item_slist, tuple_cache_item);
struct tuple_cache {
	size_t                        tuc_count; /* [lock(tuc_lock)][<= CONFIG_TUPLE_CACHE_MAXSIZE] Amount of cached objects int `tuc_list' */
	struct tuple_cache_item_slist tuc_list;  /* [0..n][lock(tuc_lock)] Linked list of cached tuple objects. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t             tuc_lock;  /* Lock for this tuple cache. */
#endif /* !CONFIG_NO_THREADS */
};

#define tuple_cache_lock_available(self)  Dee_atomic_lock_available(&(self)->tuc_lock)
#define tuple_cache_lock_acquired(self)   Dee_atomic_lock_acquired(&(self)->tuc_lock)
#define tuple_cache_lock_tryacquire(self) Dee_atomic_lock_tryacquire(&(self)->tuc_lock)
#define tuple_cache_lock_acquire(self)    Dee_atomic_lock_acquire(&(self)->tuc_lock)
#define tuple_cache_lock_waitfor(self)    Dee_atomic_lock_waitfor(&(self)->tuc_lock)
#define tuple_cache_lock_release(self)    Dee_atomic_lock_release(&(self)->tuc_lock)

PRIVATE struct tuple_cache cache[CONFIG_TUPLE_CACHE_MAXCOUNT] = { {} };

INTERN size_t DCALL
Dee_tuplecache_clearall(size_t max_clear) {
	size_t result = 0;
	struct tuple_cache *iter;
	for (iter = cache; iter < COMPILER_ENDOF(cache); ++iter) {
		struct tuple_cache_item *elem;
		tuple_cache_lock_acquire(iter);
		while ((elem = SLIST_FIRST(&iter->tuc_list)) != NULL && result < max_clear) {
			SLIST_REMOVE_HEAD(&iter->tuc_list, tci_link);
			ASSERT(iter->tuc_count != 0);
			--iter->tuc_count;
			result += _Dee_MallococBufsize(offsetof(Tuple, t_elem),
			                               (size_t)(iter - cache + 1),
			                               sizeof(DeeObject *));
			DeeObject_Free(elem);
		}
		ASSERT(!!SLIST_EMPTY(&iter->tuc_list) == (iter->tuc_count == 0));
		tuple_cache_lock_release(iter);
		if (result >= max_clear)
			break;
	}
	return result;
}

#else /* CONFIG_TUPLE_CACHE_MAXCOUNT */

INTERN size_t DCALL
Dee_tuplecache_clearall(size_t UNUSED(max_clear)) {
	return 0;
}

#endif /* !CONFIG_TUPLE_CACHE_MAXCOUNT */


/* Create new tuple objects. */
PUBLIC WUNUSED DREF Tuple *DCALL
DeeTuple_NewUninitialized(size_t n) {
	DREF Tuple *result;
	if unlikely(!n)
		return (Tuple *)DeeTuple_NewEmpty();
#if CONFIG_TUPLE_CACHE_MAXCOUNT
	if (n < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[n - 1];
		if (c->tuc_count != 0) {
			tuple_cache_lock_acquire(c);
#ifndef CONFIG_NO_THREADS
			if (!SLIST_EMPTY(&c->tuc_list))
#endif /* !CONFIG_NO_THREADS */
			{
				ASSERT(!SLIST_EMPTY(&c->tuc_list));
				result = (DREF Tuple *)SLIST_FIRST(&c->tuc_list);
				SLIST_REMOVE_HEAD(&c->tuc_list, tci_link);
				--c->tuc_count;
				ASSERT(!!SLIST_EMPTY(&c->tuc_list) == (c->tuc_count == 0));
				ASSERT(result->t_size == n);
				tuple_cache_lock_release(c);
				goto got_result;
			}
			tuple_cache_lock_release(c);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	result = (DREF Tuple *)DeeObject_Malloc(DeeTuple_SIZEOF_SAFE(n));
	if unlikely(!result)
		goto done;
	result->t_size = n;
#if CONFIG_TUPLE_CACHE_MAXCOUNT
got_result:
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	DeeObject_Init(result, &DeeTuple_Type);
	DBG_memset(result->t_elem, 0xcc, n * sizeof(DREF DeeObject *));
done:
	return result;
}

/* Same as `DeeTuple_NewUninitialized()', but
 * doesn't throw an exception when returning `NULL' */
PUBLIC WUNUSED DREF Tuple *DCALL
DeeTuple_TryNewUninitialized(size_t n) {
	DREF Tuple *result;
	if unlikely(!n)
		return (Tuple *)DeeTuple_NewEmpty();
#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
	if (n < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[n - 1];
		if (c->tuc_count) {
			tuple_cache_lock_acquire(c);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (!SLIST_EMPTY(&c->tuc_list))
#endif /* !CONFIG_NO_THREADS */
			{
				ASSERT(!SLIST_EMPTY(&c->tuc_list));
				result = (DREF Tuple *)SLIST_FIRST(&c->tuc_list);
				SLIST_REMOVE_HEAD(&c->tuc_list, tci_link);
				--c->tuc_count;
				ASSERT(!!SLIST_EMPTY(&c->tuc_list) == (c->tuc_count == 0));
				ASSERT(result->t_size == n);
				tuple_cache_lock_release(c);
				goto got_result;
			}
			tuple_cache_lock_release(c);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */
	result = (DREF Tuple *)DeeObject_TryMalloc(DeeTuple_SIZEOF_SAFE(n));
	if unlikely(!result)
		goto done;
	result->t_size = n;
#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
got_result:
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */
	DeeObject_Init(result, &DeeTuple_Type);
	DBG_memset(result->t_elem, 0xcc, n * sizeof(DREF DeeObject *));
done:
	return result;
}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
INTERN NONNULL((1)) void DCALL
tuple_tp_free(void *__restrict ptr) {
	Tuple *self = (Tuple *)ptr;
	ASSERT(!DeeTuple_IsEmpty(self));
	ASSERT(DeeTuple_SIZE(self) != 0);
	DBG_memset(DeeTuple_ELEM(self), 0xcc,
	           DeeTuple_SIZE(self) * sizeof(DREF DeeObject *));
	if (DeeTuple_SIZE(self) < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[DeeTuple_SIZE(self) - 1];
		if (c->tuc_count < CONFIG_TUPLE_CACHE_MAXSIZE) {
			tuple_cache_lock_acquire(c);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if unlikely(c->tuc_count >= CONFIG_TUPLE_CACHE_MAXSIZE) {
				tuple_cache_lock_release(c);
			} else
#endif /* !CONFIG_NO_THREADS */
			{
				struct tuple_cache_item *cob;
				cob = (struct tuple_cache_item *)self;
				SLIST_INSERT(&c->tuc_list, cob, tci_link);
				++c->tuc_count;
				tuple_cache_lock_release(c);
				return;
			}
		}
	}
	DeeObject_Free(self);
}
#else /* CONFIG_TUPLE_CACHE_MAXCOUNT */
#define tuple_tp_free(ptr) DeeObject_Free(ptr)
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */

PUBLIC NONNULL((1)) void DCALL
DeeTuple_FreeUninitialized(DREF Tuple *__restrict self) {
	if likely(self != (Tuple *)&DeeTuple_Empty) {
		ASSERT(self->ob_refcnt == 1);
		ASSERT(self->ob_type == &DeeTuple_Type);
		tuple_tp_free(self);
	} else {
		Dee_DecrefNokill(self);
	}
}

PUBLIC WUNUSED NONNULL((1)) DREF Tuple *DCALL
DeeTuple_ResizeUninitialized(/*inherit(on_success)*/ DREF Tuple *__restrict self,
                             size_t new_size) {
	DREF Tuple *new_tuple;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	if unlikely(DeeTuple_SIZE(self) == new_size)
		return self;
	if unlikely(DeeTuple_IsEmpty(self)) {
		/* Special case: Must not resize the empty tuple. */
		new_tuple = DeeTuple_NewUninitialized(new_size);
		if likely(new_tuple)
			Dee_DecrefNokill(self);
		return new_tuple;
	}
	ASSERT(DeeTuple_SIZE(self) != 0);
	ASSERTF(self->ob_refcnt == 1, "The tuple is being shared");
	if unlikely(!new_size) {
		/* Special case: Resize to an empty tuple. */
		Dee_DecrefNokill(&DeeTuple_Type);
		tuple_tp_free(self);
		return (Tuple *)DeeTuple_NewEmpty();
	}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
	/* Check if we can use a cached tuple. */
	if (new_size < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[new_size - 1];
		if (c->tuc_count) {
			tuple_cache_lock_acquire(c);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (!SLIST_EMPTY(&c->tuc_list))
#endif /* !CONFIG_NO_THREADS */
			{
				size_t common_length;
				STATIC_ASSERT((offsetof(Tuple, t_elem) %
				               sizeof(DREF DeeObject *)) == 0);
				ASSERT(!SLIST_EMPTY(&c->tuc_list));
				new_tuple = (DREF Tuple *)SLIST_FIRST(&c->tuc_list);
				SLIST_REMOVE_HEAD(&c->tuc_list, tci_link);
				--c->tuc_count;
				tuple_cache_lock_release(c);
				/* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
				common_length = MIN(DeeTuple_SIZE(self), new_size);
				memcpyc(new_tuple, self,
				        (offsetof(Tuple, t_elem) / sizeof(DREF DeeObject *)) +
				        common_length, sizeof(DREF DeeObject *));
#ifndef NDEBUG
				if (new_size > DeeTuple_SIZE(self)) {
					DBG_memset(&new_tuple->t_elem[DeeTuple_SIZE(self)], 0xcc,
					           (new_size - DeeTuple_SIZE(self)) * sizeof(DREF DeeObject *));
				}
#endif /* !NDEBUG */
				tuple_tp_free(self);
				new_tuple->t_size = new_size;
				return new_tuple;
			}
			tuple_cache_lock_release(c);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */

	/* Resize the old tuple. */
	new_tuple = (DREF Tuple *)DeeObject_Realloc(self, DeeTuple_SIZEOF_SAFE(new_size));
	if unlikely(!new_tuple)
		goto err;
#ifndef NDEBUG
	if (new_size > new_tuple->t_size) {
		DBG_memset(&new_tuple->t_elem[new_tuple->t_size], 0xcc,
		           (new_size - new_tuple->t_size) * sizeof(DREF DeeObject *));
	}
#endif /* !NDEBUG */
	new_tuple->t_size = new_size;
	return new_tuple;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF Tuple *DCALL
DeeTuple_TryResizeUninitialized(/*inherit(on_success)*/ DREF Tuple *__restrict self,
                                size_t new_size) {
	DREF Tuple *new_tuple;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	if unlikely(DeeTuple_SIZE(self) == new_size)
		return self;
	if unlikely(DeeTuple_IsEmpty(self)) {
		/* Special case: Must not resize the empty tuple. */
		new_tuple = (DREF Tuple *)DeeTuple_TryNewUninitialized(new_size);
		if likely(new_tuple)
			Dee_DecrefNokill(self);
		return new_tuple;
	}
	ASSERT(DeeTuple_SIZE(self) != 0);
	ASSERTF(self->ob_refcnt == 1, "The tuple is being shared");
	if unlikely(!new_size) {
		/* Special case: Resize to an empty tuple. */
		Dee_DecrefNokill(&DeeTuple_Type);
		tuple_tp_free(self);
		return (Tuple *)DeeTuple_NewEmpty();
	}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
	/* Check if we can use a cached tuple. */
	if (new_size < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[new_size - 1];
		if (c->tuc_count) {
			tuple_cache_lock_acquire(c);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (!SLIST_EMPTY(&c->tuc_list))
#endif /* !CONFIG_NO_THREADS */
			{
				size_t common_length;
				STATIC_ASSERT((offsetof(Tuple, t_elem) %
				               sizeof(DREF DeeObject *)) == 0);
				ASSERT(!SLIST_EMPTY(&c->tuc_list));
				new_tuple = (DREF Tuple *)SLIST_FIRST(&c->tuc_list);
				SLIST_REMOVE_HEAD(&c->tuc_list, tci_link);
				--c->tuc_count;
				tuple_cache_lock_release(c);
				/* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
				common_length = MIN(DeeTuple_SIZE(self), new_size);
				memcpyc(new_tuple, self,
				        (offsetof(Tuple, t_elem) / sizeof(DREF DeeObject *)) +
				        common_length, sizeof(DREF DeeObject *));
#ifndef NDEBUG
				if (new_size > DeeTuple_SIZE(self)) {
					DBG_memset(&new_tuple->t_elem[DeeTuple_SIZE(self)], 0xcc,
					           (new_size - DeeTuple_SIZE(self)) * sizeof(DREF DeeObject *));
				}
#endif /* !NDEBUG */
				tuple_tp_free(self);
				new_tuple->t_size = new_size;
				return new_tuple;
			}
			tuple_cache_lock_release(c);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */

	/* Try to resize the old tuple. */
	new_tuple = (DREF Tuple *)DeeObject_TryRealloc(self, DeeTuple_SIZEOF_SAFE(new_size));
	if unlikely(!new_tuple)
		goto err;
#ifndef NDEBUG
	if (new_size > new_tuple->t_size) {
		DBG_memset(&new_tuple->t_elem[new_tuple->t_size], 0xcc,
		           (new_size - new_tuple->t_size) * sizeof(DREF DeeObject *));
	}
#endif /* !NDEBUG */
	new_tuple->t_size = new_size;
	return new_tuple;
err:
	return NULL;
}

PUBLIC WUNUSED ATTR_RETNONNULL NONNULL((1)) DREF Tuple *DCALL
DeeTuple_TruncateUninitialized(/*inherit(always)*/ DREF Tuple *__restrict self,
                               size_t new_size) {
	DREF Tuple *new_tuple;
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
		return (Tuple *)DeeTuple_NewEmpty();
	}

#if CONFIG_TUPLE_CACHE_MAXCOUNT
	/* Check if we can use a cached tuple. */
	if (new_size < CONFIG_TUPLE_CACHE_MAXCOUNT) {
		struct tuple_cache *c = &cache[new_size - 1];
		if (c->tuc_count) {
			tuple_cache_lock_acquire(c);
#ifndef CONFIG_NO_THREADS
			COMPILER_READ_BARRIER();
			if (!SLIST_EMPTY(&c->tuc_list))
#endif /* !CONFIG_NO_THREADS */
			{
				size_t common_length;
				ASSERT(!SLIST_EMPTY(&c->tuc_list));
				new_tuple = (DREF Tuple *)SLIST_FIRST(&c->tuc_list);
				SLIST_REMOVE_HEAD(&c->tuc_list, tci_link);
				--c->tuc_count;
				tuple_cache_lock_release(c);
				/* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
				common_length = MIN(DeeTuple_SIZE(self), new_size);
				memcpyc(new_tuple, self,
				        (offsetof(Tuple, t_elem) / sizeof(DREF DeeObject *)) +
				        common_length, sizeof(DREF DeeObject *));
#ifndef NDEBUG
				if (new_size > DeeTuple_SIZE(self)) {
					DBG_memset(&new_tuple->t_elem[DeeTuple_SIZE(self)], 0xcc,
					           (new_size - DeeTuple_SIZE(self)) * sizeof(DREF DeeObject *));
				}
#endif /* !NDEBUG */
				tuple_tp_free(self);
				new_tuple->t_size = new_size;
				return new_tuple;
			}
			tuple_cache_lock_release(c);
		}
	}
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT */
	/* Try to resize the old tuple. */
	new_tuple = (DREF Tuple *)DeeObject_TryRealloc(self, DeeTuple_SIZEOF(new_size));
	if unlikely(!new_tuple)
		new_tuple = (DREF Tuple *)self;
	new_tuple->t_size = new_size;
	return new_tuple;
}


/* Create a new tuple from a given vector. */
PUBLIC WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL
DeeTuple_NewVector(size_t objc, DeeObject *const *__restrict objv) {
	DREF Tuple *result;
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto done;
	Dee_Movrefv(DeeTuple_ELEM(result), objv, objc);
done:
	return Dee_AsObject(result);
}

PUBLIC WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL
DeeTuple_NewVectorSymbolic(size_t objc, DeeObject *const *__restrict objv) {
	DREF Tuple *result;
	result = DeeTuple_NewUninitialized(objc);
	if unlikely(!result)
		goto done;
	memcpyc(DeeTuple_ELEM(result), objv,
	        objc, sizeof(DREF DeeObject *));
done:
	return Dee_AsObject(result);
}

PUBLIC WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL
DeeTuple_TryNewVector(size_t objc, DeeObject *const *__restrict objv) {
	DREF Tuple *result;
	result = DeeTuple_TryNewUninitialized(objc);
	if unlikely(!result)
		goto done;
	Dee_Movrefv(DeeTuple_ELEM(result), objv, objc);
done:
	return Dee_AsObject(result);
}

PUBLIC WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL
DeeTuple_TryNewVectorSymbolic(size_t objc, DeeObject *const *__restrict objv) {
	DREF Tuple *result;
	result = DeeTuple_TryNewUninitialized(objc);
	if unlikely(!result)
		goto done;
	memcpyc(DeeTuple_ELEM(result), objv,
	        objc, sizeof(DREF DeeObject *));
done:
	return Dee_AsObject(result);
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_FromList(DeeObject *__restrict self) {
	DREF DeeTupleObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeList_Type);
	DeeListObject *me = (DeeListObject *)self;
	size_t seq_length = DeeList_SIZE_ATOMIC(me);
list_size_changed:
	result = DeeTuple_NewUninitialized(seq_length);
	if unlikely(!result)
		goto err;
	COMPILER_READ_BARRIER();
	DeeList_LockRead(me);
	if unlikely(seq_length != DeeList_SIZE(me)) {
		seq_length = DeeList_SIZE(me);
		DeeList_LockEndRead(me);
		DeeTuple_FreeUninitialized(result);
		goto list_size_changed;
	}
	Dee_Movrefv(DeeTuple_ELEM(result),
	            DeeList_ELEM(me),
	            seq_length);
	DeeList_LockEndRead(me);
	return Dee_AsObject(result);
err:
	return NULL;
}

/* Create a new tuple object from a sequence or iterator. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_FromSequence(DeeObject *__restrict self) {
	DeeTypeObject *tp_seq = Dee_TYPE(self);

	/* Optimizations for specific types such as `Tuple' and `List' */
	if (tp_seq == &DeeTuple_Type) {
		return_reference_(self);
	} else if (tp_seq == &DeeList_Type) {
		return DeeTuple_FromList(self);
	} else {
		/* Fallback: use the generic tuple builder */
		struct Dee_tuple_builder builder;
		size_t hint = DeeObject_SizeFast(self);
		if (hint != (size_t)-1) {
			Dee_tuple_builder_init_with_hint(&builder, hint);
		} else {
			Dee_tuple_builder_init(&builder);
		}
		if unlikely(DeeObject_Foreach(self, &Dee_tuple_builder_append, &builder))
			goto err_builder;
		return Dee_tuple_builder_pack(&builder);
err_builder:
		Dee_tuple_builder_fini(&builder);
		goto err;
	}
	__builtin_unreachable();
err:
	return NULL;
}

/* Return a new tuple object containing the types of each object of the given tuple. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_Types(DeeObject *__restrict self) {
	size_t i, count;
	DREF Tuple *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	count  = DeeTuple_SIZE(self);
	result = DeeTuple_NewUninitialized(count);
	if unlikely(!result)
		goto done;
	for (i = 0; i < count; ++i) {
		DeeTypeObject *tp;
		tp = Dee_TYPE(DeeTuple_GET(self, i));
		Dee_Incref(tp);
		DeeTuple_SET(result, i, tp);
	}
done:
	return Dee_AsObject(result);
}

#ifdef __ARCH_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeTuple_VPack, 8),
                    DCALL_ASSEMBLY_NAME(DeeTuple_NewVector, 8));
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeTuple_VPackSymbolic, 8),
                    DCALL_ASSEMBLY_NAME(DeeTuple_NewVectorSymbolic, 8));
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeTuple_VTryPack, 8),
                    DCALL_ASSEMBLY_NAME(DeeTuple_TryNewVector, 8));
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeTuple_VTryPackSymbolic, 8),
                    DCALL_ASSEMBLY_NAME(DeeTuple_TryNewVectorSymbolic, 8));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPack(size_t n, va_list args) {
	return DeeTuple_NewVector(n, (DeeObject **)args);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ va_list args) {
	return DeeTuple_NewVectorSymbolic(n, (DeeObject **)args);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VTryPack(size_t n, va_list args) {
	return DeeTuple_TryNewVector(n, (DeeObject **)args);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VTryPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ va_list args) {
	return DeeTuple_TryNewVectorSymbolic(n, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* __ARCH_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPack(size_t n, va_list args) {
	size_t i;
	DREF Tuple *result;
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
	return Dee_AsObject(result);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ va_list args) {
	size_t i;
	DREF Tuple *result;
	result = DeeTuple_NewUninitialized(n);
	if unlikely(!result)
		goto done;
	for (i = 0; i < n; ++i) {
		DREF DeeObject *elem;
		elem = va_arg(args, DeeObject *);
		DeeTuple_SET(result, i, elem);
	}
done:
	return Dee_AsObject(result);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VTryPack(size_t n, va_list args) {
	size_t i;
	DREF Tuple *result;
	result = DeeTuple_TryNewUninitialized(n);
	if unlikely(!result)
		goto done;
	for (i = 0; i < n; ++i) {
		DREF DeeObject *elem;
		elem = va_arg(args, DeeObject *);
		Dee_Incref(elem);
		DeeTuple_SET(result, i, elem);
	}
done:
	return Dee_AsObject(result);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTuple_VTryPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ va_list args) {
	size_t i;
	DREF Tuple *result;
	result = DeeTuple_TryNewUninitialized(n);
	if unlikely(!result)
		goto done;
	for (i = 0; i < n; ++i) {
		DREF DeeObject *elem;
		elem = va_arg(args, DeeObject *);
		DeeTuple_SET(result, i, elem);
	}
done:
	return Dee_AsObject(result);
}
#endif /* !__ARCH_VA_LIST_IS_STACK_POINTER */

/* Create new tuple objects. */
PUBLIC WUNUSED DREF DeeObject *
DeeTuple_Pack(size_t n, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, n);
	result = DeeTuple_VPack(n, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED DREF DeeObject *
DeeTuple_TryPack(size_t n, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, n);
	result = DeeTuple_VTryPack(n, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED DREF DeeObject *
DeeTuple_PackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, n);
	result = DeeTuple_VPackSymbolic(n, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED DREF DeeObject *
DeeTuple_TryPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, n);
	result = DeeTuple_VTryPackSymbolic(n, args);
	va_end(args);
	return result;
}


/* Decrement the reference counter of a tuple object filled with symbolic references.
 * >> If the reference counter hits ZERO(0), simply free() the tuple object
 *    without decrementing the reference counters of contained objects.
 *    Otherwise (In case the tuple is being used elsewhere), increment
 *    the reference counters of all contained objects.
 * >> This function is used to safely clean up temporary, local
 *    tuples that are not initialized to contain ~real~ references.
 *    Using this function such tuples can be released with regards
 *    to fixing incorrect reference counters of contained objects.
 *    NOTE: Doing this is still ok, because somewhere further up
 *          the call chain, a caller owns another reference to each
 *          contained object, even before we fix reference counters.
 * Semantically speaking, you can think of this function as:
 * >> Dee_Increfv(DeeTuple_ELEM(self), DeeTuple_SIZE(self));
 * >> Dee_Decref(self); */
PUBLIC NONNULL((1)) void DCALL
DeeTuple_DecrefSymbolic(DeeObject *__restrict self) {
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	if (!DeeObject_IsShared(self)) {
		DeeTuple_FreeUninitialized((Tuple *)self);
	} else {
		Dee_Increfv(DeeTuple_ELEM(self),
		            DeeTuple_SIZE(self));
		Dee_Decref_unlikely(self);
	}
}









/*  ====== `Tuple.Iterator' type implementation ======  */
typedef struct {
	PROXY_OBJECT_HEAD_EX(Tuple, ti_tuple); /* [1..1][const] Referenced tuple. */
	DWEAK size_t                ti_index;  /* [<= ti_tuple->t_size] Next-element index. */
} TupleIterator;
#define READ_INDEX(x) atomic_read(&(x)->ti_index)

INTDEF DeeTypeObject DeeTupleIterator_Type;

PRIVATE NONNULL((1)) int DCALL
tuple_iterator_ctor(TupleIterator *__restrict self) {
	self->ti_tuple = (DREF Tuple *)DeeTuple_NewEmpty();
	self->ti_index = 0;
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
	self->ti_tuple = (DREF Tuple *)DeeObject_DeepCopy((DeeObject *)other->ti_tuple);
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
	self->ti_tuple = (DREF Tuple *)Dee_EmptyTuple;
	self->ti_index = 0;
	if (DeeArg_Unpack(argc, argv, "|o" UNPuSIZ ":_TupleIterator",
	                  &self->ti_tuple, &self->ti_index))
		goto err;
	if (DeeObject_AssertTypeExact(self->ti_tuple, &DeeTuple_Type))
		goto err;
	if (self->ti_index >= DeeTuple_SIZE(self->ti_tuple))
		goto err_bounds;
	Dee_Incref(self->ti_tuple);
	return 0;
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self->ti_tuple),
	                          self->ti_index,
	                          DeeTuple_SIZE(self->ti_tuple));
err:
	return -1;
}

STATIC_ASSERT(offsetof(TupleIterator, ti_tuple) == offsetof(ProxyObject, po_obj));
#define tuple_iterator_serialize generic_proxy__serialize_and_wordcopy_atomic(__SIZEOF_SIZE_T__)
#define tuple_iterator_fini      generic_proxy__fini
#define tuple_iterator_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_iterator_next(TupleIterator *__restrict self) {
	DREF DeeObject *result;
	size_t index;
	do {
		index = atomic_read(&self->ti_index);
		if (index >= DeeTuple_SIZE(self->ti_tuple))
			return ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_index, index, index + 1));
	result = DeeTuple_GET(self->ti_tuple, index);
	ASSERT_OBJECT(result);
	Dee_Incref(result);
	return result;
}

PRIVATE struct type_member tpconst tuple_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(TupleIterator, ti_tuple), "->?DTuple"),
	TYPE_MEMBER_FIELD(STR_index, STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(TupleIterator, ti_index)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
tuple_iterator_hash(TupleIterator *self) {
	return READ_INDEX(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tuple_iterator_compare(TupleIterator *self, TupleIterator *other) {
	if (DeeObject_AssertTypeExact(other, &DeeTupleIterator_Type))
		goto err;
	Dee_return_compareT(size_t, READ_INDEX(self),
	                    /*   */ READ_INDEX(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF Tuple *DCALL
tuple_iterator_nii_getseq(TupleIterator *__restrict self) {
	return_reference_(self->ti_tuple);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
tuple_iterator_nii_getindex(TupleIterator *__restrict self) {
	return READ_INDEX(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_setindex(TupleIterator *__restrict self, size_t new_index) {
	atomic_write(&self->ti_index, new_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_rewind(TupleIterator *__restrict self) {
	atomic_write(&self->ti_index, 0);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_revert(TupleIterator *__restrict self, size_t step) {
	size_t old_index, new_index;
	do {
		old_index = atomic_read(&self->ti_index);
		if (OVERFLOW_USUB(old_index, step, &new_index))
			new_index = 0;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_index, old_index, new_index));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_advance(TupleIterator *__restrict self, size_t step) {
	size_t old_index, new_index;
	do {
		old_index = atomic_read(&self->ti_index);
		if (OVERFLOW_UADD(old_index, step, &new_index))
			new_index = (size_t)-1;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_index, old_index, new_index));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_prev(TupleIterator *__restrict self) {
	size_t old_index;
	do {
		old_index = atomic_read(&self->ti_index);
		if (!old_index)
			return 1;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_index, old_index, old_index - 1));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_iterator_nii_next(TupleIterator *__restrict self) {
	size_t old_index;
	do {
		old_index = atomic_read(&self->ti_index);
		if (old_index >= DeeTuple_SIZE(self->ti_tuple))
			return 1;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_index, old_index, old_index + 1));
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


PRIVATE struct type_nii tpconst tuple_iterator_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&tuple_iterator_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&tuple_iterator_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&tuple_iterator_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&tuple_iterator_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)&tuple_iterator_nii_revert,
			/* .nii_advance  = */ (Dee_funptr_t)&tuple_iterator_nii_advance,
			/* .nii_prev     = */ (Dee_funptr_t)&tuple_iterator_nii_prev,
			/* .nii_next     = */ (Dee_funptr_t)&tuple_iterator_nii_next,
			/* .nii_hasprev  = */ (Dee_funptr_t)&tuple_iterator_nii_hasprev,
			/* .nii_peek     = */ (Dee_funptr_t)&tuple_iterator_nii_peek
		}
	}
};

PRIVATE struct type_cmp tuple_iterator_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&tuple_iterator_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&tuple_iterator_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &tuple_iterator_nii,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TupleIterator,
			/* tp_ctor:        */ &tuple_iterator_ctor,
			/* tp_copy_ctor:   */ &tuple_iterator_copy,
			/* tp_deep_ctor:   */ &tuple_iterator_deep,
			/* tp_any_ctor:    */ &tuple_iterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &tuple_iterator_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&tuple_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&tuple_iterator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&tuple_iterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &tuple_iterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_iterator_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ tuple_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};











/*  ====== `Tuple' type implementation ======  */

PRIVATE WUNUSED DREF Tuple *DCALL tuple_ctor(void) {
	return (DREF Tuple *)DeeTuple_NewEmpty();
}

INTERN WUNUSED NONNULL((1)) DREF Tuple *DCALL
tuple_deepcopy(Tuple *__restrict self) {
	DREF Tuple *result;
	size_t i, size = DeeTuple_SIZE(self);
	result = DeeTuple_NewUninitialized(size);
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
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Tuple *DCALL
tuple_init(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("Tuple", params: "
	DeeObject *items: ?S?O;
", docStringPrefix: "tuple");]]]*/
#define tuple_Tuple_params "items:?S?O"
	struct {
		DeeObject *items;
	} args;
	DeeArg_Unpack1(err, argc, argv, "Tuple", &args.items);
/*[[[end]]]*/
	return (DREF Tuple *)DeeTuple_FromSequence(args.items);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
tuple_serialize(Tuple *__restrict self, DeeSerial *__restrict writer) {
	Tuple *out;
	size_t i, sizeof_tuple = offsetof(Tuple, t_elem) + (self->t_size * sizeof(DREF DeeObject *));
	Dee_seraddr_t addr = DeeSerial_ObjectMalloc(writer, sizeof_tuple, self);
	if (!Dee_SERADDR_ISOK(addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, Tuple);
	out->t_size = self->t_size;
	for (i = 0; i < self->t_size; ++i) {
		DeeObject *item = self->t_elem[i];
		Dee_seraddr_t addrof_item = addr + offsetof(Tuple, t_elem) + (i * sizeof(DREF DeeObject *));
		if (DeeSerial_PutObject(writer, addrof_item, item))
			goto err;
	}
	return addr;
err:
	return Dee_SERADDR_INVALID;
}

INTERN NONNULL((1)) void DCALL
tuple_fini(Tuple *__restrict self) {
	Dee_Decrefv(DeeTuple_ELEM(self),
	            DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF TupleIterator *DCALL
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
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_sizeob(Tuple *__restrict self) {
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
		error = DeeObject_TryCompareEq(item, ob);
		if (Dee_COMPARE_ISERR(error))
			goto err;
		if (Dee_COMPARE_ISEQ(error))
			return_true;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tuple_getitem(Tuple *self, DeeObject *index) {
	size_t i;
	if unlikely(DeeObject_AsSize(index, &i))
		goto err_maybe_overflow;
	if unlikely(i >= DeeTuple_SIZE(self))
		goto err_bounds;
	return_reference(DeeTuple_GET(self, i));
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), i,
	                          DeeTuple_SIZE(self));
err:
	return NULL;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
	goto err;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getrange_index(Tuple *__restrict self,
                     Dee_ssize_t begin, Dee_ssize_t end) {
	size_t range_size;
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, begin, end, self->t_size);
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size == self->t_size)
		return_reference(Dee_AsObject(self));
	return DeeTuple_NewVector(range_size, self->t_elem + range.sr_start);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getrange_index_n(Tuple *__restrict self,
                       Dee_ssize_t begin) {
#ifdef __OPTIMIZE_SIZE__
	return tuple_getrange_index(self, begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t start;
	start = DeeSeqRange_Clamp_n(begin, self->t_size);
	if unlikely(start == 0)
		return_reference(Dee_AsObject(self));
	return DeeTuple_NewVector(DeeTuple_SIZE(self) - start,
	                          DeeTuple_ELEM(self) + start);
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tuple_getrange(Tuple *self, DeeObject *begin, DeeObject *end) {
	Dee_ssize_t i_begin, i_end;
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (DeeNone_Check(end))
		return tuple_getrange_index_n(self, i_begin);
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	return tuple_getrange_index(self, i_begin, i_end);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) size_t DCALL
tuple_size(Tuple *__restrict self) {
	ASSERT(self->t_size != (size_t)-1);
	return self->t_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getitem_index(Tuple *__restrict self, size_t index) {
	if unlikely(index >= self->t_size)
		goto err_bounds;
	return_reference(self->t_elem[index]);
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->t_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getitem_index_fast(Tuple *__restrict self, size_t index) {
	ASSERT(index < self->t_size);
	return_reference(self->t_elem[index]);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_trygetitem_index(Tuple *__restrict self, size_t index) {
	if unlikely(index >= self->t_size)
		return ITER_DONE;
	return_reference(self->t_elem[index]);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tuple_foreach(Tuple *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i < self->t_size; ++i) {
		temp = (*proc)(arg, self->t_elem[i]);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tuple_mh_enumerate_index(Tuple *self, Dee_seq_enumerate_index_t proc,
                         void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->t_size)
		end = self->t_size;
	for (i = start; i < end; ++i) {
		temp = (*proc)(arg, i, self->t_elem[i]);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
tuple_asvector_nothrow(Tuple *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t result = self->t_size;
	if likely(dst_length >= result)
		Dee_Movrefv(dst, self->t_elem, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_mh_seq_unpack(Tuple *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if unlikely(self->t_size != dst_length)
		return DeeRT_ErrUnpackError(self, dst_length, self->t_size);
	Dee_Movrefv(dst, self->t_elem, dst_length);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
tuple_mh_seq_unpack_ex(Tuple *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst) {
	if unlikely(self->t_size < dst_length_min || self->t_size > dst_length_max)
		return (size_t)DeeRT_ErrUnpackErrorEx(self, dst_length_min, dst_length_max, self->t_size);
	Dee_Movrefv(dst, self->t_elem, self->t_size);
	return self->t_size;
}

PRIVATE struct type_seq tuple_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&tuple_getrange,
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&tuple_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&tuple_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&tuple_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&tuple_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&tuple_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&tuple_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&tuple_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&tuple_trygetitem_index,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&tuple_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&tuple_asvector_nothrow,
};

PRIVATE WUNUSED NONNULL((1)) DREF Tuple *DCALL
tuple_unpack_method(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
	DREF Tuple *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("unpack", params: "
	size_t num, seq:?S?O
", docStringPrefix: "tuple");]]]*/
#define tuple_unpack_params "num:?Dint,seq:?S?O"
	struct {
		size_t num;
		DeeObject *seq;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "unpack", &args, &args.num, UNPuSIZ, DeeObject_AsSize, &args.seq, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	result = DeeTuple_NewUninitialized(args.num);
	if unlikely(!result)
		goto done;
	if unlikely(DeeSeq_Unpack(args.seq, args.num, DeeTuple_ELEM(result))) {
		DeeTuple_FreeUninitialized(result);
err:
		result = NULL;
	}
done:
	return result;
}

PRIVATE struct type_method tpconst tuple_class_methods[] = {
	TYPE_METHOD("unpack", &tuple_unpack_method,
	            "(" tuple_unpack_params ")->?.\n"
	            "#tUnpackError{The given @seq doesn't contain exactly @num elements}"
	            "Unpack the given sequence @seq into a Tuple consisting of @num elements\n"
	            "Deprecated alias for ${Tuple((seq as Sequence).unpack(num))}"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst tuple_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeTupleIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeTuple_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

INTERN NONNULL((1, 2)) void DCALL
tuple_visit(Tuple *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visitv(DeeTuple_ELEM(self),
	           DeeTuple_SIZE(self));
}

/* Print all elements of the given tuple without any separators in-between
 * elements. This is equivalent to `Tuple.operator str' and is related to
 * the change introduced for handling `print("foo", "bar");'-like statements */
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tuple_print(Tuple *__restrict self,
            Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
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
	 * them to `print' instructions that never actually create a tuple.
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tuple_printrepr(Tuple *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	/* Special case: single-item tuples are
	 * must be encoded with a trailing comma. */
	if (DeeTuple_SIZE(self) == 1) {
		result = DeeFormat_Printf(printer, arg, "(%r,)", DeeTuple_GET(self, 0));
	} else {
		size_t i, count;
		result = DeeFormat_PRINT(printer, arg, "(");
		if unlikely(result < 0)
			goto done;
		count = DeeTuple_SIZE(self);
		for (i = 0; i < count; ++i) {
			/* Print this item. */
			if (i) {
				temp = DeeFormat_PRINT(printer, arg, ", ");
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			temp = DeeObject_PrintRepr(DeeTuple_GET(self, i), printer, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		temp = DeeFormat_PRINT(printer, arg, ")");
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
done:
	return result;
err:
	return temp;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_sizeof(Tuple *self) {
	return DeeInt_NewSize(DeeTuple_SIZEOF(self->t_size));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getfirst(Tuple *__restrict self) {
	if unlikely(DeeTuple_IsEmpty(self))
		goto err_empty;
	return_reference_(DeeTuple_GET(self, 0));
err_empty:
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getlast(Tuple *__restrict self) {
	if unlikely(DeeTuple_IsEmpty(self))
		goto err_empty;
	return_reference_(DeeTuple_GET(self, DeeTuple_SIZE(self) - 1));
err_empty:
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tuple_nonempty_as_bound(Tuple *__restrict self) {
	return Dee_BOUND_FROMPRESENT_BOUND(self->t_size != 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_trygetfirst(Tuple *__restrict self) {
	if unlikely(DeeTuple_IsEmpty(self))
		return ITER_DONE;
	return_reference_(DeeTuple_GET(self, 0));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_trygetlast(Tuple *__restrict self) {
	if unlikely(DeeTuple_IsEmpty(self))
		return ITER_DONE;
	return_reference_(DeeTuple_GET(self, DeeTuple_SIZE(self) - 1));
}



PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
tuple_mh_find(Tuple *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->t_size)
		end = self->t_size;
	for (; start < end; ++start) {
		int temp = DeeObject_TryCompareEq(item, self->t_elem[start]);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return start;
	}
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
tuple_mh_find_with_key(Tuple *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->t_size)
		end = self->t_size;
	for (; start < end; ++start) {
		int temp = DeeObject_TryCompareKeyEq(item, self->t_elem[start], key);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp))
			return start;
	}
	Dee_Decref(item);
	return (size_t)-1;
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
tuple_mh_rfind(Tuple *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->t_size)
		end = self->t_size;
	while (end > start) {
		int temp;
		--end;
		temp = DeeObject_TryCompareEq(item, self->t_elem[end]);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		if (Dee_COMPARE_ISEQ(temp))
			return end;
	}
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
tuple_mh_rfind_with_key(Tuple *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	if (end > self->t_size)
		end = self->t_size;
	while (end > start) {
		int temp;
		--end;
		temp = DeeObject_TryCompareKeyEq(item, self->t_elem[end], key);
		if (Dee_COMPARE_ISERR(temp))
			goto err_item;
		if (Dee_COMPARE_ISEQ(temp))
			return end;
	}
	Dee_Decref(item);
	return (size_t)-1;
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF Tuple *DCALL
tuple_mh_sorted(Tuple *__restrict self, size_t start, size_t end) {
	DREF Tuple *result;
	if (end > self->t_size)
		end = self->t_size;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortVector(result->t_size, result->t_elem, self->t_elem + start))
		goto err_r;
	Dee_Increfv(result->t_elem, result->t_size);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF Tuple *DCALL
tuple_mh_sorted_with_key(Tuple *self, size_t start, size_t end, DeeObject *key) {
	DREF Tuple *result;
	if (end > self->t_size)
		end = self->t_size;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortVectorWithKey(result->t_size, result->t_elem, self->t_elem + start, key))
		goto err_r;
	Dee_Increfv(result->t_elem, result->t_size);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* Needed by "seq/flat.c" */
tuple_mh_foreach_reverse(DeeTupleObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* Needed by "seq/flat.c" */
tuple_mh_enumerate_index_reverse(DeeTupleObject *__restrict self, Dee_seq_enumerate_index_t proc,
                                 void *arg, size_t start, size_t end);


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tuple_mh_foreach_reverse(DeeTupleObject *__restrict self,
                         Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i = self->t_size;
	while (i) {
		--i;
		temp = (*proc)(arg, self->t_elem[i]);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

#define nullable_tuple_mh_enumerate_index_reverse tuple_mh_enumerate_index_reverse
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tuple_mh_enumerate_index_reverse(DeeTupleObject *__restrict self, Dee_seq_enumerate_index_t proc,
                                 void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->t_size)
		end = self->t_size;
	while (end > start) {
		--end;
		temp = (*proc)(arg, start, self->t_elem[end]);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_method tpconst tuple_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_find),
	TYPE_METHOD_HINTREF(Sequence_rfind),
	TYPE_METHOD_HINTREF(Sequence_sorted),
	TYPE_METHOD_HINTREF(Sequence_unpack),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst tuple_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &tuple_mh_foreach_reverse, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &tuple_mh_enumerate_index, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &tuple_mh_enumerate_index_reverse, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_find, &tuple_mh_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &tuple_mh_find_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &tuple_mh_rfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &tuple_mh_rfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted, &tuple_mh_sorted, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted_with_key, &tuple_mh_sorted_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &tuple_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_trygetlast, &tuple_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_unpack, &tuple_mh_seq_unpack, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_unpack_ex, &tuple_mh_seq_unpack_ex, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst tuple_getsets[] = {
	TYPE_GETTER_BOUND_F_NODOC(STR_first, &tuple_getfirst, &tuple_nonempty_as_bound,
	                          METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_GETTER_BOUND_F_NODOC(STR_last, &tuple_getlast, &tuple_nonempty_as_bound,
	                          METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
#define nullable_tuple_getsets (tuple_getsets + 2)
	TYPE_GETTER_F(STR_frozen, &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_F(STR_cached, &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_F("__sizeof__", &tuple_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

struct tuple_concat_fe_data {
	DREF Tuple *tcfed_result; /* [1..1] The resulting tuple. */
	size_t      tcfed_offset; /* Offset of next element to write in `tcfed_result' */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
tuple_concat_fe_cb(void *arg, DeeObject *item) {
	struct tuple_concat_fe_data *data;
	data = (struct tuple_concat_fe_data *)arg;
	if unlikely(data->tcfed_offset >= data->tcfed_result->t_size) {
		/* Allocate a larger buffer. */
		size_t new_size;
		DREF Tuple *new_result;
		if (OVERFLOW_UMUL(data->tcfed_result->t_size, 2, &new_size)) {
			if (OVERFLOW_UADD(data->tcfed_result->t_size, 1, &new_size))
				new_size = (size_t)-1;
		}
		new_result = DeeTuple_TryResizeUninitialized(data->tcfed_result, new_size);
		if unlikely(!new_result) {
			if (OVERFLOW_UADD(data->tcfed_result->t_size, 1, &new_size))
				new_size = (size_t)-1;
			new_result = DeeTuple_ResizeUninitialized(data->tcfed_result, new_size);
			if unlikely(!new_result)
				goto err;
		}
		data->tcfed_result = new_result;
	}
	data->tcfed_result->t_elem[data->tcfed_offset] = item;
	Dee_Incref(item);
	++data->tcfed_offset;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF Tuple *DCALL
tuple_concat(Tuple *self, DeeObject *other) {
	DREF Tuple *result;
	size_t other_sizehint, total_size;
	DeeTypeObject *tp_other          = Dee_TYPE(other);
	DeeNO_foreach_t tp_other_foreach = DeeType_RequireNativeOperator(tp_other, foreach);

	/* Try to get an idea of how large the final sequence will be. */
	if (tp_other->tp_seq && tp_other->tp_seq->tp_size_fast) {
		other_sizehint = (*tp_other->tp_seq->tp_size_fast)(other);
		if unlikely(other_sizehint == (size_t)-1)
			other_sizehint = 8;
	} else {
		other_sizehint = 8;
	}

	/* Allocate an initial buffer. */
	if (OVERFLOW_UADD(self->t_size, other_sizehint, &total_size))
		total_size = (size_t)-1;
	result = DeeTuple_TryNewUninitialized(total_size);
	if unlikely(!result) {
		other_sizehint = 0;
		total_size     = self->t_size;
		result = DeeTuple_NewUninitialized(total_size);
		if unlikely(!result)
			goto err;
	}

	/* Check if the type supports the "tp_asvector" extension. */
	if (tp_other->tp_seq && tp_other->tp_seq->tp_asvector) {
		size_t other_size;
		for (;;) {
			DREF Tuple *new_result;
			other_size = (*tp_other->tp_seq->tp_asvector)(other, other_sizehint,
			                                              result->t_elem + self->t_size);
			if (other_size <= other_sizehint)
				break; /* Success! got the entire sequence */
			/* Must allocate a larger buffer. */
			if (OVERFLOW_UADD(self->t_size, other_size, &total_size))
				total_size = (size_t)-1; /* Force downstream OOM */
			new_result = DeeTuple_ResizeUninitialized(result, total_size);
			if unlikely(!new_result)
				goto err_r;
			result         = new_result;
			other_sizehint = other_size;
		}
		if (other_size < other_sizehint) {
			total_size = self->t_size + other_size;
			result = DeeTuple_TruncateUninitialized(result, total_size);
		}
	} else {
		Dee_ssize_t fe_status;
		struct tuple_concat_fe_data data;
		data.tcfed_result = result;
		data.tcfed_offset = self->t_size;
		fe_status = (*tp_other_foreach)(other, &tuple_concat_fe_cb, &data);
		ASSERT(data.tcfed_offset >= self->t_size);
		result = data.tcfed_result;
		ASSERT(fe_status <= 0);
		if unlikely(fe_status) {
			Dee_Decrefv(result->t_elem + self->t_size,
			            data.tcfed_offset - self->t_size);
			goto err_r;
		}
		total_size = data.tcfed_offset;
		result = DeeTuple_TruncateUninitialized(result, total_size);
	}

	/* Fill in elements inherited from `self' */
	Dee_Movrefv(result->t_elem, self->t_elem, self->t_size);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

/* Concat a tuple and some generic sequence,
 * inheriting a reference from `self' in the process. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_ConcatInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *sequence) {
	DREF Tuple *me = (DREF Tuple *)self;
	DREF Tuple *result;
	size_t lhs_size, other_sizehint, total_size;
	DeeTypeObject *tp_sequence;
	DeeNO_foreach_t tp_sequence_foreach;
	if unlikely(DeeObject_IsShared(me)) {
		result = tuple_concat(me, sequence);
		Dee_Decref_unlikely(me);
		return Dee_AsObject(result);
	}

	tp_sequence         = Dee_TYPE(sequence);
	tp_sequence_foreach = DeeType_RequireNativeOperator(tp_sequence, foreach);

	/* Try to get an idea of how large the final sequence will be. */
	if (tp_sequence->tp_seq && tp_sequence->tp_seq->tp_size_fast) {
		other_sizehint = (*tp_sequence->tp_seq->tp_size_fast)(sequence);
		if unlikely(other_sizehint == (size_t)-1)
			other_sizehint = 8;
	} else {
		other_sizehint = 8;
	}

	/* Allocate an initial buffer. */
	lhs_size = me->t_size;
	if (OVERFLOW_UADD(lhs_size, other_sizehint, &total_size))
		total_size = (size_t)-1;
	result = DeeTuple_TryResizeUninitialized(me, total_size);
	if unlikely(!result) {
		other_sizehint = 0;
		total_size     = lhs_size;
		result = DeeTuple_ResizeUninitialized(me, total_size);
		if unlikely(!result)
			goto err_me;
	}

	/* Check if the type supports the "tp_asvector" extension. */
	if (tp_sequence->tp_seq && tp_sequence->tp_seq->tp_asvector) {
		size_t other_size;
		for (;;) {
			DREF Tuple *new_result;
			other_size = (*tp_sequence->tp_seq->tp_asvector)(sequence, other_sizehint,
			                                                 result->t_elem + lhs_size);
			if (other_size <= other_sizehint)
				break; /* Success! got the entire sequence */
			/* Must allocate a larger buffer. */
			if (OVERFLOW_UADD(lhs_size, other_size, &total_size))
				total_size = (size_t)-1; /* Force downstream OOM */
			new_result = DeeTuple_ResizeUninitialized(result, total_size);
			if unlikely(!new_result) {
				Dee_Decrefv(result->t_elem, lhs_size);
				goto err_r;
			}
			result         = new_result;
			other_sizehint = other_size;
		}
		if (other_size < other_sizehint) {
			total_size = lhs_size + other_size;
			result = DeeTuple_TruncateUninitialized(result, total_size);
		}
	} else {
		Dee_ssize_t fe_status;
		struct tuple_concat_fe_data data;
		data.tcfed_result = result;
		data.tcfed_offset = lhs_size;
		fe_status = (*tp_sequence_foreach)(sequence, &tuple_concat_fe_cb, &data);
		ASSERTF(data.tcfed_offset >= lhs_size, "%Iu >= %Iu", data.tcfed_offset, lhs_size);
		result = data.tcfed_result;
		ASSERT(fe_status <= 0);
		if unlikely(fe_status) {
			Dee_Decrefv(result->t_elem, data.tcfed_offset);
			goto err_r;
		}
		total_size = data.tcfed_offset;
		result = DeeTuple_TruncateUninitialized(result, total_size);
	}
	return Dee_AsObject(result);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
err_me:
	Dee_Decref(me);
	goto err;
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1, 3)) DREF DeeObject *DCALL
DeeTuple_ExtendInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc,
                         /*inherit(always)*/ DREF DeeObject *const *argv) {
	DREF Tuple *me = (DREF Tuple *)self;
	DREF Tuple *result;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeTuple_Type);
	if (!DeeObject_IsShared(me)) {
		size_t old_size; /* Optimization: The old object can be re-used. */
		old_size = DeeTuple_SIZE(me);
		result   = DeeTuple_ResizeUninitialized(me, old_size + argc);
		if unlikely(!result)
			goto err_me_argv;
		memcpyc(result->t_elem + old_size, argv,
		        argc, sizeof(DREF DeeObject *));
	} else if unlikely(!argc) {
		result = me; /* Inherit reference */
	} else {
		DREF DeeObject **iter;
		size_t mylen = DeeTuple_SIZE(me);
		result = DeeTuple_NewUninitialized(mylen + argc);
		if unlikely(!result)
			goto err_me_argv;
		iter = Dee_Movprefv(DeeTuple_ELEM(result), DeeTuple_ELEM(me), mylen);
		memcpyc(iter, argv, argc, sizeof(DREF DeeObject *));
		Dee_Decref_unlikely(me);
	}
	return Dee_AsObject(result);
err_me_argv:
	Dee_Decrefv(argv, argc);
	Dee_Decref(me);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Tuple *DCALL
tuple_repeat(Tuple *self, DeeObject *other) {
	size_t i, count, total_length, my_length;
	DREF Tuple *result;
	DREF DeeObject **dst;
	if (DeeObject_AsSize(other, &count))
		goto err;
	if (count == 1)
		return_reference_(self);

	/* Repeat `self' `count' number of times. */
	my_length = DeeTuple_SIZE(self);
	if (OVERFLOW_UMUL(my_length, count, &total_length))
		goto err_overflow;
	if unlikely(total_length == 0)
		goto return_empty;
	result = DeeTuple_NewUninitialized(total_length);
	if unlikely(!result)
		goto err;

	/* Fill in the resulting tuple with repetitions of ourself. */
	dst = DeeTuple_ELEM(result);
#ifndef __OPTIMIZE_SIZE__
	if (my_length == 1) {
		Dee_Setrefv(dst, DeeTuple_GET(self, 0), count);
	} else
#endif /* !__OPTIMIZE_SIZE__ */
	{
		/* Create all the new references that will be contained in the new tuple. */
		for (i = 0; i < my_length; ++i)
			Dee_Incref_n(DeeTuple_GET(self, i), count);
		while (count--) {
			dst = (DREF DeeObject **)mempcpyc(dst, DeeTuple_ELEM(self),
			                                  my_length, sizeof(DREF DeeObject *));
		}
	}
	return result;
return_empty:
	return (Tuple *)DeeTuple_NewEmpty();
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(my_length, count);
err:
	return NULL;
}

PRIVATE struct type_math tuple_math = {
	/* .tp_int32  = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64  = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int    = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv    = */ DEFIMPL(&default__set_operator_inv),
	/* .tp_pos    = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg    = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_concat,
	/* .tp_sub    = */ DEFIMPL(&default__set_operator_sub),
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_repeat,
	/* .tp_div    = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod    = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl    = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr    = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and    = */ DEFIMPL(&default__set_operator_and),
	/* .tp_or     = */ DEFIMPL(&default__set_operator_add),
	/* .tp_xor    = */ DEFIMPL(&default__set_operator_xor),
	/* .tp_pow    = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__set_operator_inplace_sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ DEFIMPL(&default__set_operator_inplace_and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__set_operator_inplace_add),
	/* .tp_inplace_xor = */ DEFIMPL(&default__set_operator_inplace_xor),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
tuple_hash(Tuple *__restrict self) {
	return DeeObject_Hashv(DeeTuple_ELEM(self),
	                       DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tuple_compare(Tuple *self, DeeObject *other) {
	return seq_docompare__lhs_vector(self->t_elem, self->t_size, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tuple_compare_eq(Tuple *self, DeeObject *other) {
	return seq_docompareeq__lhs_vector(self->t_elem, self->t_size, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tuple_trycompare_eq(Tuple *self, DeeObject *other) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(other), foreach))
		return Dee_COMPARE_NE;
	return tuple_compare_eq(self, other);
}

PRIVATE struct type_cmp tuple_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&tuple_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&tuple_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&tuple_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&tuple_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_operator const tuple_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTSTR | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
};

#if CONFIG_TUPLE_CACHE_MAXCOUNT != 0
#define tuple_tp_free_PTR &tuple_tp_free
#else /* CONFIG_TUPLE_CACHE_MAXCOUNT != 0 */
#define tuple_tp_free_PTR NULL
#endif /* CONFIG_TUPLE_CACHE_MAXCOUNT == 0 */


PUBLIC DeeTypeObject DeeTuple_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Tuple),
	/* .tp_doc      = */ DOC("A builtin type that is similar to ?DList, however represents a fixed-length, "
	                         /**/ "immutable sequence of objects. Tuples are fast, low-level ?DSequence-like objects "
	                         /**/ "that are written as ${(elem1, elem2, etc)}, with the exception of single-element "
	                         /**/ "tuples being written as ${(single_element,)}\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?.\n"
	                         "\n"

	                         "(" tuple_Tuple_params ")\n"
	                         "Construct a new ?. that is pre-initializes with the elements from @items\n"
	                         "\n"

	                         "str->\n"
	                         "Returns the concatenation of all of @this ?.'s elements converted to strings:\n"
	                         "${"
	                         /**/ "operator str() {\n"
	                         /**/ "	return \"\".join(this);\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns a representation of @this ?.:\n"
	                         "${"
	                         /**/ "operator repr() {\n"
	                         /**/ "	if (#this == 1)\n"
	                         /**/ "		return f\"({repr this[0]},)\";\n"
	                         /**/ "	return f\"({\", \".join(for (local x: this) repr x)})\";\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. is non-empty\n"
	                         "\n"

	                         "+->\n"
	                         "+(other:?S?O)->\n"
	                         "#tNotImplemented{The given @other isn't iterable}"
	                         "Returns a new ?. consisting of the elements from @this, followed by "
	                         /**/ "those from @other, which may be another ?., or a generic sequence\n"
	                         "\n"

	                         "*(count:?Dint)->\n"
	                         "#tIntegerOverflow{The given @count is negative, or too large}"
	                         "Return a new ?. consisting of the elements from @this, repeated @count times\n"
	                         "When @count is $0, an empty ?. is returned. When @count is $1, @this ?. is re-returned\n"
	                         "\n"

	                         "==->\n"
	                         "!=->\n"
	                         "<->\n"
	                         "<=->\n"
	                         ">->\n"
	                         ">=->\n"
	                         "Perform a lexicographical comparison between the elements of @this "
	                         /**/ "?. and the given @other sequence\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this ?.\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of elements contained inside of @this ?.\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @elem is apart of @this ?., or @false otherwise\n"
	                         "\n"

	                         "[]->\n"
	                         "#tIntegerOverflow{The given @index is negative, or too large}"
	                         "#tIndexError{The given @index is out of bounds}"
	                         "Returns the @index'th item of @this ?.\n"
	                         "\n"

	                         "[:]->?.\n"
	                         "Returns a new ?. for the given subrange, following the usual rules for "
	                         /**/ "negative @start or @end values, as well as ?N being passed for "
	                         /**/ "either (s.a. ?A{op:getrange}?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0, /* !!! DeeTuple_ConcatInherited assumes that tuples can't have weakrefs! */
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &tuple_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &tuple_deepcopy,
			/* tp_any_ctor:    */ &tuple_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &tuple_serialize,
			/* tp_free:        */ tuple_tp_free_PTR
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&tuple_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_str,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&tuple_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&tuple_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&tuple_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&tuple_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &tuple_math,
	/* .tp_cmp           = */ &tuple_cmp,
	/* .tp_seq           = */ &tuple_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ tuple_methods,
	/* .tp_getsets       = */ tuple_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ tuple_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ tuple_class_members,
	/* .tp_method_hints  = */ tuple_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ tuple_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(tuple_operators),
};

PUBLIC struct Dee_empty_tuple_struct DeeTuple_Empty = {
	4, /* +1 in contrast to OBJECT_HEAD_INIT for the reference
	    * saved as the original value for `Dee_GetArgv()' */
	&DeeTuple_Type,
#ifdef CONFIG_TRACE_REFCHANGES
	DEE_REFTRACKER_UNTRACKED,
#endif /* CONFIG_TRACE_REFCHANGES */
	0
};

PUBLIC struct Dee_empty_tuple_struct DeeNullableTuple_Empty = {
	OBJECT_HEAD_INIT(&DeeNullableTuple_Type),
	0
};

LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
make_nullable(DREF DeeTupleObject *__restrict self) {
	if unlikely(self == (DREF DeeTupleObject *)&DeeTuple_Empty) {
		Dee_DecrefNokill(&DeeTuple_Empty);
		Dee_Incref(&DeeNullableTuple_Empty);
		return (DREF DeeTupleObject *)&DeeNullableTuple_Empty;
	}
	Dee_DecrefNokill(&DeeTuple_Type);
	Dee_Incref(&DeeNullableTuple_Type);
	self->ob_type = &DeeNullableTuple_Type;
	return self;
}

PRIVATE WUNUSED NONNULL((1)) DREF Tuple *DCALL
nullable_tuple_unpack(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
	DREF Tuple *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("unpack", params: "
	size_t num, seq:?S?O
", docStringPrefix: "nullable_tuple");]]]*/
#define nullable_tuple_unpack_params "num:?Dint,seq:?S?O"
	struct {
		size_t num;
		DeeObject *seq;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "unpack", &args, &args.num, UNPuSIZ, DeeObject_AsSize, &args.seq, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	result = DeeTuple_NewUninitialized(args.num);
	if unlikely(!result)
		goto done;
	if unlikely(DeeObject_InvokeMethodHint(seq_unpack_ub, args.seq, args.num,
	                                       args.num, DeeTuple_ELEM(result)) == (size_t)-1) {
		DeeTuple_FreeUninitialized(result);
err:
		result = NULL;
	} else {
		result = make_nullable(result);
	}
done:
	return result;
}


PRIVATE WUNUSED DREF Tuple *DCALL nullable_tuple_ctor(void) {
	Dee_Incref(&DeeNullableTuple_Empty);
	return (DREF DeeTupleObject *)&DeeNullableTuple_Empty;
}

INTERN WUNUSED NONNULL((1)) DREF Tuple *DCALL
nullable_tuple_deepcopy(Tuple *__restrict self) {
	DREF Tuple *result;
	size_t i, size = DeeTuple_SIZE(self);
	result = DeeTuple_NewUninitialized(size);
	if unlikely(!result)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *temp;
		temp = DeeTuple_GET(self, i);
		if (temp) {
			temp = DeeObject_DeepCopy(DeeTuple_GET(self, i));
			if unlikely(!temp)
				goto err_r;
		}
		DeeTuple_SET(result, i, temp); /* Inherit reference. */
	}
	return make_nullable(result);
err_r:
	Dee_XDecrefv_likely(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Tuple *DCALL
nullable_tuple_init(size_t argc, DeeObject *const *argv) {
	DREF Tuple *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("NullableTuple", params: "
	DeeObject *items: ?S?O;
", docStringPrefix: "nullable_tuple");]]]*/
#define nullable_tuple_NullableTuple_params "items:?S?O"
	struct {
		DeeObject *items;
	} args;
	DeeArg_Unpack1(err, argc, argv, "NullableTuple", &args.items);
/*[[[end]]]*/
	result = (DREF Tuple *)DeeTuple_FromSequence(args.items);
	if likely(result)
		result = make_nullable(result);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
nullable_tuple_serialize(Tuple *__restrict self, DeeSerial *__restrict writer) {
	Tuple *out;
	size_t i, sizeof_tuple = offsetof(Tuple, t_elem) + (self->t_size * sizeof(DREF DeeObject *));
	Dee_seraddr_t addr = DeeSerial_ObjectMalloc(writer, sizeof_tuple, self);
	if (!Dee_SERADDR_ISOK(addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, Tuple);
	out->t_size = self->t_size;
	for (i = 0; i < self->t_size; ++i) {
		DeeObject *item = self->t_elem[i];
		Dee_seraddr_t addrof_item = addr + offsetof(Tuple, t_elem) + (i * sizeof(DREF DeeObject *));
		if (DeeSerial_XPutObject(writer, addrof_item, item))
			goto err;
	}
	return addr;
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE NONNULL((1)) void DCALL
nullable_tuple_fini(Tuple *__restrict self) {
	Dee_XDecrefv(DeeTuple_ELEM(self),
	             DeeTuple_SIZE(self));
}

PRIVATE NONNULL((1, 2)) void DCALL
nullable_tuple_visit(Tuple *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisitv(DeeTuple_ELEM(self),
	            DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
nullable_tuple_contains(Tuple *self, DeeObject *item) {
	int error;
	size_t i, mylen;
	mylen = DeeTuple_SIZE(self);
	for (i = 0; i < mylen; ++i) {
		DeeObject *ob;
		ob = DeeTuple_GET(self, i);
		if (!ob)
			continue;
		error = DeeObject_TryCompareEq(item, ob);
		if (Dee_COMPARE_ISERR(error))
			goto err;
		if (Dee_COMPARE_ISEQ(error))
			return_true;
	}
	return_false;
err:
	return NULL;
}

#define nullable_tuple_mh_enumerate_index \
	tuple_mh_enumerate_index /* Actually works the same! */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nullable_tuple_getitem_index(Tuple *__restrict self, size_t index) {
	if unlikely(index >= self->t_size)
		goto err_bounds;
	if (!self->t_elem[index])
		goto err_unbound;
	return_reference(self->t_elem[index]);
err_unbound:
	DeeRT_ErrUnboundIndex(self, index);
	return NULL;
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->t_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nullable_tuple_getitem_index_fast(Tuple *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->t_size);
	result = self->t_elem[index];
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nullable_tuple_trygetitem_index(Tuple *__restrict self, size_t index) {
	if unlikely(index >= self->t_size)
		return ITER_DONE;
	if (!self->t_elem[index])
		return ITER_DONE;
	return_reference(self->t_elem[index]);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
nullable_tuple_mh_seq_unpack_ub(Tuple *self, size_t min_count, size_t max_count, /*out*/ DREF DeeObject **dst) {
	if unlikely(self->t_size < min_count || self->t_size > max_count)
		return DeeRT_ErrUnpackErrorEx(self, min_count, max_count, self->t_size);
	Dee_XMovrefv(dst, self->t_elem, self->t_size);
	return self->t_size;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
nullable_tuple_asvector(Tuple *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t i, result = 0;
	for (i = 0; i < self->t_size; ++i) {
		DeeObject *item = self->t_elem[i];
		if (!item)
			continue;
		if (result == dst_length) {
			size_t j;
			for (j = 0; j < dst_length; ++j)
				Dee_DecrefNokill(dst[j]);
		} else if (result < dst_length) {
			dst[result] = item;
			Dee_Incref(item);
		}
		++result;
	}
	return result;
}


PRIVATE struct type_seq nullable_tuple_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&nullable_tuple_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&tuple_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&tuple_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&nullable_tuple_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&nullable_tuple_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&nullable_tuple_trygetitem_index,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, /*out*/ DREF DeeObject **))&nullable_tuple_asvector,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, /*out*/ DREF DeeObject **))&nullable_tuple_asvector,
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
nullable_tuple_mh_foreach_reverse(DeeTupleObject *__restrict self,
                                  Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i = self->t_size;
	while (i) {
		--i;
		if (self->t_elem[i]) {
			temp = (*proc)(arg, self->t_elem[i]);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_method_hint tpconst nullable_tuple_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &nullable_tuple_mh_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &nullable_tuple_mh_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &nullable_tuple_mh_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack_ub, &nullable_tuple_mh_seq_unpack_ub, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method tpconst nullable_tuple_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_unpackub),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method tpconst nullable_tuple_class_methods[] = {
	TYPE_METHOD("unpack", &nullable_tuple_unpack,
	            "(" nullable_tuple_unpack_params ")->?.\n"
	            "#tUnpackError{The given @seq doesn't contain exactly @num elements}"
	            "Unpack the given sequence @seq into a Tuple consisting of @num elements.\n"
	            "Same as ?Aunpack?DTuple, except that unbound items are not silently skipped"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst nullable_tuple_class_members[] = {
	TYPE_MEMBER_CONST(STR_Frozen, &DeeNullableTuple_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
nullable_tuple_hash(Tuple *__restrict self) {
	return DeeObject_XHashv(DeeTuple_ELEM(self),
	                        DeeTuple_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
nullable_tuple_compare(Tuple *self, DeeObject *other) {
	return seq_docompare__lhs_xvector(self->t_elem, self->t_size, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
nullable_tuple_compare_eq(Tuple *self, DeeObject *other) {
	return seq_docompareeq__lhs_xvector(self->t_elem, self->t_size, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
nullable_tuple_trycompare_eq(Tuple *self, DeeObject *other) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(other), foreach))
		return Dee_COMPARE_NE;
	return nullable_tuple_compare_eq(self, other);
}

PRIVATE struct type_cmp nullable_tuple_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&nullable_tuple_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&nullable_tuple_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&nullable_tuple_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&nullable_tuple_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_operator const nullable_tuple_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
};

PUBLIC DeeTypeObject DeeNullableTuple_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "NullableTuple",
	/* .tp_doc      = */ DOC("Same as ?DTuple, but able to represent unbound elements\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?.\n"
	                         "\n"

	                         "(" nullable_tuple_NullableTuple_params ")\n"
	                         "Construct a new ?. that is pre-initializes with the elements from @items"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &nullable_tuple_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &nullable_tuple_deepcopy,
			/* tp_any_ctor:    */ &nullable_tuple_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &nullable_tuple_serialize,
			/* tp_free:        */ tuple_tp_free_PTR
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&nullable_tuple_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&tuple_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&nullable_tuple_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ &nullable_tuple_cmp,
	/* .tp_seq           = */ &nullable_tuple_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ nullable_tuple_methods,
	/* .tp_getsets       = */ nullable_tuple_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ nullable_tuple_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ nullable_tuple_class_members,
	/* .tp_method_hints  = */ nullable_tuple_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ nullable_tuple_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(nullable_tuple_operators),
};





/************************************************************************/
/* TUPLE_BUILDER                                                        */
/************************************************************************/
PUBLIC ATTR_RETNONNULL WUNUSED DREF /*Tuple*/DeeObject *DCALL
Dee_tuple_builder_pack(struct Dee_tuple_builder *__restrict self) {
	DeeTupleObject *result = self->tb_tuple;
	if unlikely(!result) {
		ASSERT(self->tb_size == 0);
		return DeeTuple_NewEmpty();
	}
	return Dee_AsObject(DeeTuple_TruncateUninitialized(result, self->tb_size));
}

PUBLIC WUNUSED NONNULL((2)) Dee_ssize_t DCALL
Dee_tuple_builder_append(/*struct Dee_tuple_builder*/ void *self,
                         DeeObject *item) {
	struct Dee_tuple_builder *me = (struct Dee_tuple_builder *)self;
	DeeObject **buf = Dee_tuple_builder_alloc1(me);
	if unlikely(!buf)
		goto err;
	Dee_Incref(item);
	*buf = item; /* Inherit reference */
	Dee_tuple_builder_commit1(me);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((2)) Dee_ssize_t DCALL
Dee_tuple_builder_append_inherited(/*struct Dee_tuple_builder*/ void *self,
                                   /*inherit(always)*/ DREF DeeObject *item) {
	struct Dee_tuple_builder *me = (struct Dee_tuple_builder *)self;
	DeeObject **buf = Dee_tuple_builder_alloc1(me);
	if unlikely(!buf)
		goto err;
	*buf = item; /* Inherit reference */
	Dee_tuple_builder_commit1(me);
	return 0;
err:
	Dee_Decref(item);
	return -1;
}

PUBLIC WUNUSED NONNULL((2)) Dee_ssize_t DCALL
Dee_tuple_builder_appenditems(/*struct Dee_tuple_builder*/ void *self, DeeObject *items) {
	struct Dee_tuple_builder *me = (struct Dee_tuple_builder *)self;
	size_t hint = DeeObject_SizeFast(items);
	if (hint != (size_t)-1) {
		Dee_tuple_builder_reserve(me, hint);
		/* TODO: Use tp_asvector. */
	}
	return DeeObject_Foreach(items, &Dee_tuple_builder_append, me);
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) int DCALL
Dee_tuple_builder_extend(struct Dee_tuple_builder *self, size_t objc,
                         DeeObject *const *__restrict objv) {
	DeeObject **buf = Dee_tuple_builder_alloc(self, objc);
	if unlikely(!buf)
		goto err;
	Dee_Movrefv(buf, objv, objc);
	Dee_tuple_builder_commit(self, objc);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) int DCALL
Dee_tuple_builder_extend_inherited(struct Dee_tuple_builder *self, size_t objc,
                                   /*inherit(always)*/ DREF DeeObject *const *__restrict objv) {
	DeeObject **buf = Dee_tuple_builder_alloc(self, objc);
	if unlikely(!buf)
		goto err;
	memcpyp(buf, objv, objc); /* Inherit reference */
	Dee_tuple_builder_commit(self, objc);
	return 0;
err:
	return -1;
}

/* Ensure that space for at least `n' items is allocated, and return
 * a pointer to a buffer where those `n' items can be written. Once
 * written, commit the write using `Dee_tuple_builder_commit' */
PUBLIC WUNUSED NONNULL((1)) DeeObject **DCALL
Dee_tuple_builder_alloc(struct Dee_tuple_builder *__restrict self, size_t n) {
	size_t sizeof_tuple;
	do {
		if likely(Dee_tuple_builder_reserve(self, n)) {
			ASSERT(self->tb_tuple);
			ASSERT(self->tb_tuple->t_size >= self->tb_size + n);
			return self->tb_tuple->t_elem + self->tb_size;
		}
		if (OVERFLOW_UADD(self->tb_size, n, &sizeof_tuple))
			sizeof_tuple = (size_t)-1;
		if (OVERFLOW_UMUL(sizeof_tuple, sizeof(DREF DeeObject *), &sizeof_tuple))
			sizeof_tuple = (size_t)-1;
		if (OVERFLOW_UADD(sizeof_tuple, offsetof(DeeTupleObject, t_elem), &sizeof_tuple))
			sizeof_tuple = (size_t)-1;
	} while (Dee_CollectMemory(sizeof_tuple));
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DeeObject **DCALL
Dee_tuple_builder_alloc1(struct Dee_tuple_builder *__restrict self) {
	size_t sizeof_tuple;
	DeeTupleObject *tuple = self->tb_tuple;
	if likely(tuple) {
		size_t avail = tuple->t_size;
		ASSERT(self->tb_size <= avail);
		if likely(self->tb_size < avail)
			return tuple->t_elem + self->tb_size;
	}
	do {
		if likely(Dee_tuple_builder_reserve(self, 1))
			return self->tb_tuple->t_elem + self->tb_size;
		sizeof_tuple = (self->tb_size + 1) * sizeof(DREF DeeObject *);
		sizeof_tuple += offsetof(DeeTupleObject, t_elem);
	} while (Dee_CollectMemory(sizeof_tuple));
	return NULL;
}

/* Try to ensure that space for at least `n' extra items is available.
 * Returns indicate of that much space now being pre-allocated. */
PUBLIC NONNULL((1)) bool DCALL
Dee_tuple_builder_reserve(struct Dee_tuple_builder *__restrict self, size_t n) {
	size_t min_alloc;
	size_t new_alloc;
	DeeTupleObject *tuple;
	if (OVERFLOW_UADD(self->tb_size, n, &min_alloc))
		min_alloc = (size_t)-1;
	tuple = self->tb_tuple;
	if (tuple) {
		size_t old_alloc = tuple->t_size;
		if likely(old_alloc >= min_alloc)
			return true;
		new_alloc = tuple->t_size * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		tuple = DeeTuple_TryResizeUninitialized(tuple, new_alloc);
		if unlikely(!tuple) {
			new_alloc = min_alloc;
			tuple = DeeTuple_TryResizeUninitialized(self->tb_tuple, new_alloc);
		}
	} else {
		new_alloc = min_alloc * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		tuple = DeeTuple_TryNewUninitialized(new_alloc);
		if unlikely(!tuple) {
			new_alloc = min_alloc;
			tuple = DeeTuple_TryNewUninitialized(new_alloc);
		}
	}
	if unlikely(!tuple)
		return false;
	self->tb_tuple = tuple;
	ASSERT(tuple->t_size >= self->tb_size + n);
	return true;
}


PUBLIC ATTR_RETNONNULL WUNUSED DREF /*NullableTuple*/DeeObject *DCALL
Dee_nullable_tuple_builder_pack(struct Dee_tuple_builder *__restrict self) {
	DeeTupleObject *result = self->tb_tuple;
	if unlikely(!result) {
		ASSERT(self->tb_size == 0);
		return DeeNullableTuple_NewEmpty();
	}
	result = DeeTuple_TruncateUninitialized(result, self->tb_size);
	if likely(result)
		result = make_nullable(result);
	return Dee_AsObject(result);
}

/* Ensure that at least "index + 1" elements are allocated (default-initializing
 * previously unallocated items to "NULL"; iow: unbound), and set the index'th
 * element to "item" (which is also allowed to be "NULL")
 *
 * HINT: This function is binary-compatible with `Dee_seq_enumerate_index_t'
 *
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED Dee_ssize_t DCALL
Dee_nullable_tuple_builder_setitem_index(/*struct Dee_nullable_tuple_builder **/void *self,
                                         size_t index, /*0..1*/ DeeObject *item) {
	DeeTupleObject *tuple;
	struct Dee_nullable_tuple_builder *me;
	me = (struct Dee_nullable_tuple_builder *)self;
	if (index >= me->tb_size) {
		if (!me->tb_tuple || index >= me->tb_tuple->t_size) {
			size_t req = index - me->tb_size;
			if (OVERFLOW_UADD(req, 1, &req))
				req = (size_t)-1;
			if unlikely(!Dee_nullable_tuple_builder_alloc(me, req))
				goto err;
		}
	}
	tuple = me->tb_tuple;
	ASSERT(tuple);
	ASSERT(index < tuple->t_size);
	if likely(index >= me->tb_size) {
		bzeroc(tuple->t_elem + me->tb_size,
		       (index + 1) - me->tb_size,
		       sizeof(DeeObject *));
		me->tb_size = index + 1;
	} else {
		DeeObject *old_item;
		old_item = tuple->t_elem[index];
		if unlikely(old_item)
			Dee_Decref(old_item);
	}
	tuple->t_elem[index] = item;
	Dee_XIncref(item);
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TUPLE_C */
