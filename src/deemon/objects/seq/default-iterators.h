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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/util/lock.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *digi_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `digi_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *digi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          digi_index; /* [lock(ATOMIC)] Next index to enumerate. */
} DefaultIterator_WithGetItemIndex;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *disgi_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `disgi_seq'.
	 * This is either a `tp_getitem_index' or `tp_getitem_index_fast' operator. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *disgi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          disgi_index; /* [lock(ATOMIC)] Next index to enumerate. */
	size_t          disgi_end;   /* [const] Iteration stop index. */
} DefaultIterator_WithSizeAndGetItemIndex;

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject   *dig_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `dig_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *dig_tp_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject   *dig_index; /* [1..1][lock(dig_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t dig_lock;  /* Lock for `dig_index' */
#endif /* !CONFIG_NO_THREADS */
} DefaultIterator_WithGetItem;
#define DefaultIterator_WithGetItem_LockAvailable(self)  Dee_atomic_lock_available(&(self)->dig_lock)
#define DefaultIterator_WithGetItem_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->dig_lock)
#define DefaultIterator_WithGetItem_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->dig_lock)
#define DefaultIterator_WithGetItem_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->dig_lock)
#define DefaultIterator_WithGetItem_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->dig_lock)
#define DefaultIterator_WithGetItem_LockRelease(self)    Dee_atomic_lock_release(&(self)->dig_lock)

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject   *ditg_seq;    /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `ditg_seq'. */
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *ditg_tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
	DREF DeeObject   *ditg_index;  /* [1..1][lock(ditg_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t ditg_lock;   /* Lock for `ditg_index' */
#endif /* !CONFIG_NO_THREADS */
	DeeTypeObject    *ditg_tp_seq; /* [1..1][const] The type to pass to `ditg_tp_tgetitem'. */
} DefaultIterator_WithTGetItem;
#define DefaultIterator_WithTGetItem_LockAvailable(self)  Dee_atomic_lock_available(&(self)->ditg_lock)
#define DefaultIterator_WithTGetItem_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->ditg_lock)
#define DefaultIterator_WithTGetItem_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->ditg_lock)
#define DefaultIterator_WithTGetItem_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->ditg_lock)
#define DefaultIterator_WithTGetItem_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->ditg_lock)
#define DefaultIterator_WithTGetItem_LockRelease(self)    Dee_atomic_lock_release(&(self)->ditg_lock)

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject   *disg_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `disg_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *disg_tp_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject   *disg_index; /* [1..1][lock(disg_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t disg_lock;  /* Lock for `disg_index' */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject   *disg_end;   /* [1..1][const] Iteration stop index. */
} DefaultIterator_WithSizeAndGetItem;
#define DefaultIterator_WithSizeAndGetItem_LockAvailable(self)  Dee_atomic_lock_available(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockRelease(self)    Dee_atomic_lock_release(&(self)->disg_lock)

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject   *ditsg_seq;    /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `ditsg_seq'. */
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *ditsg_tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
	DREF DeeObject   *ditsg_index;  /* [1..1][lock(ditsg_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t ditsg_lock;   /* Lock for `ditsg_index' */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject   *ditsg_end;    /* [1..1][const] Iteration stop index. */
	DeeTypeObject    *ditsg_tp_seq; /* [1..1][const] The type to pass to `ditsg_tp_tgetitem'. */
} DefaultIterator_WithTSizeAndGetItem;
#define DefaultIterator_WithTSizeAndGetItem_LockAvailable(self)  Dee_atomic_lock_available(&(self)->ditsg_lock)
#define DefaultIterator_WithTSizeAndGetItem_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->ditsg_lock)
#define DefaultIterator_WithTSizeAndGetItem_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->ditsg_lock)
#define DefaultIterator_WithTSizeAndGetItem_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->ditsg_lock)
#define DefaultIterator_WithTSizeAndGetItem_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->ditsg_lock)
#define DefaultIterator_WithTSizeAndGetItem_LockRelease(self)    Dee_atomic_lock_release(&(self)->ditsg_lock)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject   *dinl_iter;   /* [1..1][const] The underlying iterator */
	/* [1..1][const] Callback to load the next element from `dinl_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dinl_tp_next)(DeeObject *self);
	size_t            dinl_limit;  /* [lock(ATOMIC)] Max # of elements to still enumerate. */
} DefaultIterator_WithNextAndLimit;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject   *diikgi_seq;  /* [1..1][const] Sequence to read key values from */
	DREF DeeObject   *diikgi_iter; /* [1..1][const] Key iterator */
	/* [1..1][const] Callback to load the next key from `diikgi_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *diikgi_tp_next)(DeeObject *self);
	/* [1..1][const] Callback to load a key value from `diikgi_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *diikgi_tp_getitem)(DeeObject *self, DeeObject *key);
} DefaultIterator_WithIterKeysAndGetItem;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject   *diiktgi_seq;  /* [1..1][const] Sequence to read key values from */
	DREF DeeObject   *diiktgi_iter; /* [1..1][const] Key iterator */
	/* [1..1][const] Callback to load the next key from `diiktgi_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *diiktgi_tp_next)(DeeObject *self);
	/* [1..1][const] Callback to load a key value from `diiktgi_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *diiktgi_tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
	DeeTypeObject    *diiktgi_tp_seq; /* [1..1][const] The type to pass to `diiktgi_tp_tgetitem'. */
} DefaultIterator_WithIterKeysAndTGetItem;

INTDEF DeeTypeObject DefaultIterator_WithGetItemIndex_Type;            /* DefaultIterator_WithGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndex_Type;     /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFast_Type; /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndTryGetItemIndex_Type;  /* DefaultIterator_WithSizeAndGetItemIndex */

INTDEF DeeTypeObject DefaultIterator_WithGetItem_Type;  /* DefaultIterator_WithGetItem */
INTDEF DeeTypeObject DefaultIterator_WithTGetItem_Type; /* DefaultIterator_WithTGetItem */

INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItem_Type;  /* DefaultIterator_WithSizeAndGetItem */
INTDEF DeeTypeObject DefaultIterator_WithTSizeAndGetItem_Type; /* DefaultIterator_WithTSizeAndGetItem */

INTDEF DeeTypeObject DefaultIterator_WithNextAndLimit_Type; /* DefaultIterator_WithNextAndLimit */

INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndGetItemSeq_Type;     /* DefaultIterator_WithIterKeysAndGetItem (yields only the value-part) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTGetItemSeq_Type;    /* DefaultIterator_WithIterKeysAndTGetItem (yields only the value-part) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTryGetItemSeq_Type;  /* DefaultIterator_WithIterKeysAndGetItem (yields only the value-part) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTTryGetItemSeq_Type; /* DefaultIterator_WithIterKeysAndTGetItem (yields only the value-part) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndGetItemMap_Type;     /* DefaultIterator_WithIterKeysAndGetItem (yields 2-element (key, value) tuples) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTGetItemMap_Type;    /* DefaultIterator_WithIterKeysAndTGetItem (yields 2-element (key, value) tuples) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTryGetItemMap_Type;  /* DefaultIterator_WithIterKeysAndGetItem (yields 2-element (key, value) tuples) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTTryGetItemMap_Type; /* DefaultIterator_WithIterKeysAndTGetItem (yields 2-element (key, value) tuples) */

INTDEF DeeTypeObject DefaultIterator_WithForeach_Type;           /* DefaultIterator_WithForeach */
INTDEF DeeTypeObject DefaultIterator_WithForeachPair_Type;       /* DefaultIterator_WithForeachPair */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateSeq_Type;      /* DefaultIterator_WithEnumerate */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateMap_Type;      /* DefaultIterator_WithEnumerate */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateIndexSeq_Type; /* DefaultIterator_WithEnumerateIndex */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateIndexMap_Type; /* DefaultIterator_WithEnumerateIndex */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H */
