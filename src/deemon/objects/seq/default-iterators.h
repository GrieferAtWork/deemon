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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H 1

#include <deemon/api.h>

#include <deemon/object.h>
#include <deemon/util/lock.h> /* Dee_atomic_lock_* */

#include "../generic-proxy.h"

#include <stddef.h> /* size_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(digi_seq) /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `digi_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *digi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          digi_index; /* [lock(ATOMIC)] Next index to enumerate. */
} DefaultIterator_WithGetItemIndex;

typedef struct {
	PROXY_OBJECT_HEAD(disgi_seq) /* [1..1][const] The sequence being iterated. */
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
	DREF DeeObject   *disg_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `disg_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *disg_tp_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject   *disg_index; /* [1..1][lock(disg_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t disg_lock;  /* Lock for `disg_index' */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject   *disg_end;   /* [1..1][const] Iteration stop index. */
} DefaultIterator_WithSizeObAndGetItem;
#define DefaultIterator_WithSizeAndGetItem_LockAvailable(self)  Dee_atomic_lock_available(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->disg_lock)
#define DefaultIterator_WithSizeAndGetItem_LockRelease(self)    Dee_atomic_lock_release(&(self)->disg_lock)

typedef struct {
	PROXY_OBJECT_HEAD(dinl_iter)   /* [1..1][const] The underlying iterator */
	/* [1..1][const] Callback to load the next element from `dinl_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dinl_tp_next)(DeeObject *self);
	size_t            dinl_limit;  /* [lock(ATOMIC)] Max # of elements to still enumerate. */
} DefaultIterator_WithNextAndLimit;

typedef struct {
	PROXY_OBJECT_HEAD2(diikgi_iter, /* [1..1][const] Sequence to read key values from */
	                   diikgi_seq)  /* [1..1][const] Key iterator */
	/* [1..1][const] Callback to load the next key from `diikgi_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *diikgi_tp_next)(DeeObject *self);
	/* [1..1][const] Callback to load a key value from `diikgi_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *diikgi_tp_getitem)(DeeObject *self, DeeObject *key);
} DefaultIterator_WithIterKeysAndGetItem;

INTDEF DeeTypeObject DefaultIterator_WithGetItemIndex_Type;                /* DefaultIterator_WithGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithGetItemIndexPair_Type;            /* DefaultIterator_WithGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndex_Type;         /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexPair_Type;     /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFast_Type;     /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFastPair_Type; /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndTryGetItemIndex_Type;      /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndTryGetItemIndexPair_Type;  /* DefaultIterator_WithSizeAndGetItemIndex */

INTDEF DeeTypeObject DefaultIterator_WithGetItem_Type;     /* DefaultIterator_WithGetItem */
INTDEF DeeTypeObject DefaultIterator_WithGetItemPair_Type; /* DefaultIterator_WithGetItem */

INTDEF DeeTypeObject DefaultIterator_WithSizeObAndGetItem_Type;     /* DefaultIterator_WithSizeObAndGetItem */
INTDEF DeeTypeObject DefaultIterator_WithSizeObAndGetItemPair_Type; /* DefaultIterator_WithSizeObAndGetItem */

INTDEF DeeTypeObject DefaultIterator_WithNextAndLimit_Type; /* DefaultIterator_WithNextAndLimit */

INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndGetItemMap_Type;     /* DefaultIterator_WithIterKeysAndGetItem (yields 2-element (key, value) tuples) */
INTDEF DeeTypeObject DefaultIterator_WithIterKeysAndTryGetItemMap_Type;  /* DefaultIterator_WithIterKeysAndGetItem (yields 2-element (key, value) tuples) */

INTDEF DeeTypeObject DefaultIterator_WithForeach_Type;           /* DefaultIterator_WithForeach */
INTDEF DeeTypeObject DefaultIterator_WithForeachPair_Type;       /* DefaultIterator_WithForeachPair */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateSeq_Type;      /* DefaultIterator_WithEnumerate */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateMap_Type;      /* DefaultIterator_WithEnumerate */
INTDEF DeeTypeObject DefaultIterator_WithEnumerateIndexSeq_Type; /* DefaultIterator_WithEnumerateIndex */


/************************************************************************/
/* Extra iterators for default enumeration sequence types               */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD(dinc_iter)    /* [1..1][const] The underlying iterator */
	/* [1..1][const] Callback to load the next element from `dinc_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dinc_tp_next)(DeeObject *self);
	size_t            dinc_counter; /* [lock(ATOMIC)] Index of the next element to-be read from "dinc_iter". */
} DefaultIterator_WithNextAndCounter;

typedef struct {
	PROXY_OBJECT_HEAD(dincl_iter)    /* [1..1][const] The underlying iterator */
	/* [1..1][const] Callback to load the next element from `dincl_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dincl_tp_next)(DeeObject *self);
	size_t            dincl_counter; /* [lock(ATOMIC)] Index of the next element to-be read from "dincl_iter". */
	size_t            dincl_limit;   /* [const] First value for `dincl_counter' that must not be yielded. */
} DefaultIterator_WithNextAndCounterAndLimit;

typedef struct {
	/* Enumerate stuff from "dinuf_iter", whose items are unpacked as a pair,
	 * and then filtered by only re-yielding those items where the first element
	 * "key" of the item-pair matches `dinuf_start <= key && dinuf_end > key'. */
	OBJECT_HEAD
	DREF DeeObject   *dinuf_iter;    /* [1..1][const] The underlying iterator */
	/* [1..1][const] Callback to load the next element from `dinuf_iter'.
	 * This must *ALWAYS* be `DeeType_RequireSupportedNativeOperator(Dee_TYPE(dinuf_iter), iter_next)' */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dinuf_tp_next)(DeeObject *self);
	DREF DeeObject   *dinuf_start;   /* [1..1][const] Range start for unpack pair key. */
	DREF DeeObject   *dinuf_end;     /* [1..1][const] Range end for unpack pair key. */
} DefaultIterator_WithNextAndUnpackFilter;

INTDEF DeeTypeObject DefaultIterator_WithNextAndCounterPair_Type;         /* DefaultIterator_WithNextAndCounter */
INTDEF DeeTypeObject DefaultIterator_WithNextAndCounterAndLimitPair_Type; /* DefaultIterator_WithNextAndCounterAndLimit */
INTDEF DeeTypeObject DefaultIterator_WithNextAndUnpackFilter_Type;        /* DefaultIterator_WithNextAndUnpackFilter */


typedef struct {
	PROXY_OBJECT_HEAD(dipsi_iter) /* [1..1][const] The underlying iterator. */
	/* [1..1] The get-next-item function to use on `dipsi_iter' in order to load the next item.
	 *        This is usually the `tp_nextkey' or `tp_nextvalue' operator of `dipsi_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dipsi_next)(DeeObject *self);
} DefaultIterator_PairSubItem;

INTDEF DeeTypeObject DefaultIterator_WithNextKey;   /* DefaultIterator_PairSubItem */
INTDEF DeeTypeObject DefaultIterator_WithNextValue; /* DefaultIterator_PairSubItem */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H */
