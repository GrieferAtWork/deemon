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
#ifndef GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H
#define GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>     /* Dee_Malloc, Dee_TryMalloc, _Dee_MallococBufsize */
#include <deemon/dex.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_* */

#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* offsetof, size_t */

DECL_BEGIN

#define DEQUE_BUCKET_DEFAULT_SIZE  62 /* 64-2 */

typedef struct deque_bucket {
	struct deque_bucket                     **db_pself;  /* [1..1][== self][1..1] Deque bucket self-pointer. */
	struct deque_bucket                      *db_next;   /* [0..1][owned] Next deque bucket. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, db_items); /* [1..1][valid_if(IN_BOUNDS)] Items of this bucket. */
} DequeBucket;

#define DEQUEBUCKET_PREV(x)         COMPILER_CONTAINER_OF((x)->db_pself, DequeBucket, db_next)
#define DEQUEBUCKET_NEXT(x)         ((x)->db_next)
#define DEQUEBUCKET_HASPREV(deq, x) ((x) != (deq)->d_head)
#define DEQUEBUCKET_HASNEXT(deq, x) ((x) != (deq)->d_tail)

#define SIZEOF_BUCKET(bucket_size) \
	_Dee_MallococBufsize(offsetof(DequeBucket, db_items), bucket_size, sizeof(DREF DeeObject *))
#define NEW_BUCKET(bucket_size) \
	((DequeBucket *)Dee_Malloc(SIZEOF_BUCKET(bucket_size)))
#define TRY_NEW_BUCKET(bucket_size) \
	((DequeBucket *)Dee_TryMalloc(SIZEOF_BUCKET(bucket_size)))


typedef struct {
	Dee_OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t d_lock;      /* Lock for this deque. */
#endif /* !CONFIG_NO_THREADS */
	DequeBucket        *d_head;      /* [lock(d_lock)][0..1] Head bucket. */
	DequeBucket        *d_tail;      /* [lock(d_lock)][0..1] Tail bucket. */
	size_t              d_size;      /* [lock(d_lock)] Total number of objects stored in this deque. */
	size_t              d_head_idx;  /* [lock(d_lock)][< d_bucket_sz][if(d_size == 0, [== 0])]
	                                  * Absolute index where the head starts. */
	size_t              d_head_use;  /* [lock(d_lock)][<= d_bucket_sz][if(d_size == 0, [== 0])]
	                                  * Amount of indices in use in `d_head'. */
	size_t              d_tail_sz;   /* [lock(d_lock)][if(d_head == d_tail, [== 0])]
	                                  * [if(d_head && d_head != d_tail, [!= 0])][<= d_bucket_sz]
	                                  * Number of items in use by the tail-bucket (only when there are more than 2 buckets). */
	size_t              d_bucket_sz; /* [lock(d_lock)][!0] Size of a bucket (in objects) */
	size_t              d_version;   /* [lock(d_lock)] Incremented every time the deque changes. */
	Dee_WEAKREF_SUPPORT
} Deque;

#define Deque_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->d_lock)
#define Deque_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->d_lock)
#define Deque_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->d_lock)
#define Deque_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->d_lock)
#define Deque_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->d_lock)
#define Deque_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->d_lock)
#define Deque_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->d_lock)
#define Deque_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->d_lock)
#define Deque_LockRead(self)       Dee_atomic_rwlock_read(&(self)->d_lock)
#define Deque_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->d_lock)
#define Deque_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->d_lock)
#define Deque_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->d_lock)
#define Deque_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->d_lock)
#define Deque_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->d_lock)
#define Deque_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->d_lock)
#define Deque_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->d_lock)


#define DEQUE_BUCKETSZ(x) ((x)->d_bucket_sz)
#define DEQUE_HEADFREE(x) ((x)->d_head_idx)
#define DEQUE_HEADUSED(x) ((x)->d_head_use)
#define DEQUE_TAILUSED(x) ((x)->d_tail_sz)
#define DEQUE_TAILFREE(x) (DEQUE_BUCKETSZ(x) - DEQUE_TAILUSED(x))

/* Returns the base-relative indices at which the head/tail bucket start at. */
#define DEQUE_HEADSTART(x) 0
#define DEQUE_HEADEND(x)   DEQUE_HEADUSED(x)
#define DEQUE_TAILSTART(x) ((x)->d_size - (x)->d_tail_sz)
#define DEQUE_TAILEND(x)   (x)->d_size

/* Returns l-values for the front, back or base-relative i'th item of the given deque. */
#define DEQUE_HEAD(x) \
	(*(ASSERT((x)->d_size), &(x)->d_head->db_items[DEQUE_HEADFREE(x)]))
#define DEQUE_TAIL(x)                                                                       \
	(*(ASSERT((x)->d_size), (x)->d_head == (x)->d_tail                                      \
	                        ? &(x)->d_tail->db_items[(x)->d_head_idx + (x)->d_head_use - 1] \
	                        : &(x)->d_tail->db_items[DEQUE_TAILUSED(x) - 1]))
#define DEQUE_ITEM(x, i) (*Deque_ItemPointer(x, i))


/* Returns a pointer to the base-relative index'th item. */
LOCAL ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeObject **DCALL
Deque_ItemPointer(Deque *__restrict self, size_t index) {
	DequeBucket *iter = self->d_head;
	Dee_ASSERT(index < self->d_size);
	if (index < DEQUE_HEADUSED(self))
		return &iter->db_items[DEQUE_HEADFREE(self) + index];
	if (index >= self->d_size / 2) {
		size_t rindex;
		/* Search for the right. */
		iter   = self->d_tail;
		rindex = self->d_size - index;
		if (rindex <= DEQUE_TAILUSED(self))
			return &iter->db_items[DEQUE_TAILUSED(self) - rindex];
		rindex -= DEQUE_TAILUSED(self);
		for (;;) {
			iter = DEQUEBUCKET_PREV(iter);
			if (rindex <= DEQUE_BUCKETSZ(self))
				break;
			rindex -= DEQUE_BUCKETSZ(self);
		}
		return &iter->db_items[DEQUE_BUCKETSZ(self) - rindex];
	}
	/* Search for the left. */
	index -= DEQUE_HEADUSED(self);
	for (;;) {
		iter = DEQUEBUCKET_NEXT(iter);
		if (index < DEQUE_BUCKETSZ(self))
			break;
		index -= DEQUE_BUCKETSZ(self);
	}
	return &iter->db_items[index];
}




/* Push/pop items from the front/back. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Deque_PushFront(Deque *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Deque_PushBack(Deque *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_PopFront(Deque *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_PopBack(Deque *__restrict self);

/* Deque rotation */
INTDEF NONNULL((1)) void DCALL Deque_llrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the first `num_objects' left. */
INTDEF NONNULL((1)) void DCALL Deque_lrrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the first `num_objects' right. */
INTDEF NONNULL((1)) void DCALL Deque_rlrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the last `num_objects' left. */
INTDEF NONNULL((1)) void DCALL Deque_rrrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the last `num_objects' right. */

INTDEF WUNUSED NONNULL((1)) int DCALL Deque_llrot(Deque *__restrict self, size_t num_objects);
INTDEF WUNUSED NONNULL((1)) int DCALL Deque_lrrot(Deque *__restrict self, size_t num_objects);
INTDEF WUNUSED NONNULL((1)) int DCALL Deque_rlrot(Deque *__restrict self, size_t num_objects);
INTDEF WUNUSED NONNULL((1)) int DCALL Deque_rrrot(Deque *__restrict self, size_t num_objects);


/* @return: true:  Successfully inserted the given item.
 * @return: false: Insertion failed. - Unlock the deque and collect
 *                `SIZEOF_BUCKET(self->d_bucket_sz)' bytes of memory. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL Deque_PushFront_unlocked(Deque *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL Deque_PushBack_unlocked(Deque *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) bool DCALL Deque_Insert_unlocked(Deque *self, size_t index, DeeObject *item);

/* @return: * : The item that was popped. */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_PopFront_unlocked(Deque *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_PopBack_unlocked(Deque *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_Pop_unlocked(Deque *__restrict self, size_t index);


/* Insert/delete an item at a given index. */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL Deque_Insert(Deque *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1)) int DCALL Deque_Erase(Deque *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_Pop(Deque *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL Deque_Pops(Deque *__restrict self, Dee_ssize_t index);

typedef struct {
	DequeBucket *di_bucket;
	size_t       di_index;
} DequeIterator;

/* C-level deque iteration helpers. */
#define DequeIterator_ITEM(self)          ((self)->di_bucket->db_items[(self)->di_index])
#define DequeIterator_PREVITEM(self, deq) (*DequeIterator_PrevItemPointer(self, deq))
#define DequeIterator_NEXTITEM(self, deq) (*DequeIterator_NextItemPointer(self, deq))

LOCAL size_t DCALL
DequeIterator_GetIndex(DequeIterator *__restrict self,
                       Deque *__restrict deq) {
	size_t result;
	if (self->di_bucket == deq->d_tail) {
		result = DEQUE_TAILSTART(deq) + self->di_index;
	} else {
		DequeBucket *iter = deq->d_head;
		result = self->di_index - DEQUE_HEADFREE(deq);
		for (; iter != self->di_bucket; iter = iter->db_next)
			result += deq->d_bucket_sz;
	}
	return result;
}

LOCAL void DCALL
DequeIterator_InitAt(DequeIterator *__restrict self,
                     Deque *__restrict deq, size_t index) {
	DequeBucket *iter = deq->d_head;
	Dee_ASSERT(index < deq->d_size);
	if (index < DEQUE_HEADUSED(deq)) {
		self->di_bucket = iter;
		self->di_index  = DEQUE_HEADFREE(deq) + index;
		return;
	}
	if (index >= deq->d_size / 2) {
		size_t rindex;
		/* Search for the right. */
		iter   = deq->d_tail;
		rindex = deq->d_size - index;
		if (rindex <= DEQUE_TAILUSED(deq)) {
			self->di_bucket = iter;
			self->di_index  = DEQUE_TAILUSED(deq) - rindex;
			return;
		}
		rindex -= DEQUE_TAILUSED(deq);
		for (;;) {
			iter = DEQUEBUCKET_PREV(iter);
			if (rindex <= DEQUE_BUCKETSZ(deq))
				break;
			rindex -= DEQUE_BUCKETSZ(deq);
		}
		self->di_bucket = iter;
		self->di_index  = DEQUE_BUCKETSZ(deq) - rindex;
		return;
	}
	/* Search for the left. */
	index -= DEQUE_HEADUSED(deq);
	for (;;) {
		iter = DEQUEBUCKET_NEXT(iter);
		if (index < DEQUE_BUCKETSZ(deq))
			break;
		index -= DEQUE_BUCKETSZ(deq);
	}
	self->di_bucket = iter;
	self->di_index  = index;
}

LOCAL void DCALL
DequeIterator_InitBegin(DequeIterator *__restrict self,
                        Deque *__restrict deq) {
	self->di_bucket = deq->d_head;
	self->di_index  = deq->d_head_idx;
}

LOCAL void DCALL
DequeIterator_InitRBegin(DequeIterator *__restrict self,
                         Deque *__restrict deq) {
	self->di_bucket = deq->d_tail;
	if (deq->d_tail == deq->d_head) {
		Dee_ASSERT(deq->d_head_use != 0);
		self->di_index = deq->d_head_idx + deq->d_head_use - 1;
	} else {
		Dee_ASSERT(deq->d_tail_sz != 0);
		self->di_index = deq->d_tail_sz - 1;
	}
}

LOCAL void DCALL
DequeIterator_InitEnd(DequeIterator *__restrict self,
                      Deque *__restrict deq) {
	self->di_bucket = deq->d_tail;
	if (deq->d_tail == deq->d_head) {
		Dee_ASSERT(deq->d_head_use != 0);
		self->di_index = deq->d_head_idx + deq->d_head_use;
	} else {
		Dee_ASSERT(deq->d_tail_sz != 0);
		self->di_index = deq->d_tail_sz;
	}
}

LOCAL bool DCALL
DequeIterator_HasPrev(DequeIterator *__restrict self,
                      Deque *__restrict deq) {
	return self->di_bucket != deq->d_head || self->di_index > deq->d_head_idx;
}

LOCAL bool DCALL
DequeIterator_HasNext(DequeIterator *__restrict self,
                      Deque *__restrict deq) {
	if (self->di_bucket != deq->d_tail)
		return true;
	if (deq->d_head != deq->d_tail)
		return self->di_index + 1 < deq->d_tail_sz;
	return self->di_index + 1 < deq->d_head_idx + deq->d_head_use;
}

LOCAL void DCALL
DequeIterator_Prev(DequeIterator *__restrict self,
                   Deque *__restrict deq) {
	if (!self->di_index--) {
		self->di_bucket = DEQUEBUCKET_PREV(self->di_bucket);
		self->di_index  = deq->d_bucket_sz - 1;
	}
}

LOCAL void DCALL
DequeIterator_Next(DequeIterator *__restrict self,
                   Deque *__restrict deq) {
	if (++self->di_index >= deq->d_bucket_sz) {
		self->di_bucket = DEQUEBUCKET_NEXT(self->di_bucket);
		self->di_index  = 0;
	}
}

LOCAL WUNUSED DREF DeeObject **DCALL
DequeIterator_PrevItemPointer(DequeIterator *__restrict self,
                              Deque *__restrict deq) {
	Dee_ASSERT(DequeIterator_HasPrev(self, deq));
	if (self->di_index != 0)
		return &self->di_bucket->db_items[self->di_index - 1];
	return &DEQUEBUCKET_PREV(self->di_bucket)->db_items[deq->d_bucket_sz - 1];
}

LOCAL WUNUSED DREF DeeObject **DCALL
DequeIterator_NextItemPointer(DequeIterator *__restrict self,
                              Deque *__restrict deq) {
	Dee_ASSERT(DequeIterator_HasNext(self, deq));
	if (self->di_index + 1 < deq->d_bucket_sz)
		return &self->di_bucket->db_items[self->di_index + 1];
	return &DEQUEBUCKET_NEXT(self->di_bucket)->db_items[0];
}


typedef struct {
	Dee_OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t di_lock; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
	DequeIterator       di_iter; /* [lock(di_lock)] The C-level iterator used to implement this one. */
	DREF Deque         *di_deq;  /* [1..1][const] The deque in question. */
	size_t              di_ver;  /* [lock(di_lock)] The deque version for this this iterator was created.
	                              * When this number doesn't match `di_deq->d_version', then the
	                              * iterator behaves as though it was exhausted. */
} DequeIteratorObject;

#define DequeIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->di_lock)
#define DequeIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->di_lock)
#define DequeIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->di_lock)
#define DequeIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->di_lock)
#define DequeIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->di_lock)
#define DequeIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->di_lock)
#define DequeIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->di_lock)
#define DequeIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->di_lock)
#define DequeIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->di_lock)
#define DequeIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->di_lock)
#define DequeIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->di_lock)
#define DequeIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->di_lock)
#define DequeIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->di_lock)
#define DequeIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->di_lock)
#define DequeIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->di_lock)
#define DequeIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->di_lock)

/* The deque type, and its iterator. */
INTDEF DeeTypeObject Deque_Type;
INTDEF DeeTypeObject DequeIterator_Type;




typedef struct {
	/* A fixed-length list who's elements may be modified
	 * Basically a tuple, but its elements can change.
	 * Additionally, elements may either be bound or unbound,
	 * with `del this[x]' causing the x'th item to become unbound. */
	Dee_OBJECT_HEAD /* GC object */
	Dee_WEAKREF_SUPPORT
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                       fl_lock;  /* Lock for this list. */
#endif /* !CONFIG_NO_THREADS */
	size_t                                    fl_size;  /* [const] Length of the list. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, fl_elem); /* [0..1][fl_size] List items. */
} FixedList;

#define FixedList_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->fl_lock)
#define FixedList_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->fl_lock)
#define FixedList_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->fl_lock)
#define FixedList_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->fl_lock)
#define FixedList_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->fl_lock)
#define FixedList_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->fl_lock)
#define FixedList_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->fl_lock)
#define FixedList_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->fl_lock)
#define FixedList_LockRead(self)       Dee_atomic_rwlock_read(&(self)->fl_lock)
#define FixedList_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->fl_lock)
#define FixedList_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->fl_lock)
#define FixedList_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->fl_lock)
#define FixedList_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->fl_lock)
#define FixedList_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->fl_lock)
#define FixedList_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->fl_lock)
#define FixedList_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->fl_lock)

typedef struct {
	Dee_OBJECT_HEAD
	DREF FixedList *li_list; /* [1..1][const] The list being iterated. */
	size_t          li_iter; /* [lock(ATOMIC)] Iterator position. */
} FixedListIterator;


INTDEF DeeTypeObject FixedList_Type;
INTDEF DeeTypeObject FixedListIterator_Type;









#define UHASH(ob)   DeeObject_HashGeneric(ob)
#define USAME(a, b) ((a) == (b))

typedef struct uset_object USet;
typedef struct udict_object UDict;
typedef struct uroset_object URoSet;
typedef struct urodict_object URoDict;
typedef struct uset_iterator_object USetIterator;
typedef struct udict_iterator_object UDictIterator;
typedef struct uroset_iterator_object URoSetIterator;
typedef struct urodict_iterator_object URoDictIterator;

/* UDICT */
struct udict_item {
	DREF DeeObject *di_key;   /* [0..1][lock(:ud_lock)] Dictionary item key. */
	DREF DeeObject *di_value; /* [1..1|if(di_key == dummy, 0..0)][valid_if(di_key)]
	                           * [lock(:ud_lock)] Dictionary item value. */
};

struct udict_object {
	OBJECT_HEAD /* GC Object */
	size_t              ud_mask; /* [lock(ud_lock)][> ud_size || ud_mask == 0] Allocated dictionary size. */
	size_t              ud_used; /* [lock(ud_lock)][<= ud_size] Amount of key-item pairs actually in use.
	                             *  HINT: The difference to `ud_size' is the number of dummy keys currently in use. */
	size_t              ud_size; /* [lock(ud_lock)][< ud_mask || ud_mask == 0] Amount of non-NULL key-item pairs. */
	struct udict_item  *ud_elem; /* [1..ud_size|ALLOC(ud_mask+1)][lock(ud_lock)]
	                             * [owned_if(!= INTERNAL(empty_dict_items))] Dict key-item pairs (items). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t ud_lock; /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	WEAKREF_SUPPORT
};

#define UDict_HashSt(self, hash)  ((hash) & Dee_REQUIRES_OBJECT(UDict, self)->ud_mask)
#define UDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define UDict_HashIt(self, i)     (Dee_REQUIRES_OBJECT(UDict, self)->ud_elem + ((i) & ((UDict *)(self))->ud_mask))

#define UDict_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->ud_lock)
#define UDict_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->ud_lock)
#define UDict_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->ud_lock)
#define UDict_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->ud_lock)
#define UDict_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->ud_lock)
#define UDict_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->ud_lock)
#define UDict_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->ud_lock)
#define UDict_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->ud_lock)
#define UDict_LockRead(self)       Dee_atomic_rwlock_read(&(self)->ud_lock)
#define UDict_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->ud_lock)
#define UDict_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->ud_lock)
#define UDict_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->ud_lock)
#define UDict_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->ud_lock)
#define UDict_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->ud_lock)
#define UDict_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->ud_lock)
#define UDict_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->ud_lock)

struct urodict_object {
	OBJECT_HEAD
	size_t                                     urd_mask;  /* [const][!0] Allocated dictionary mask. */
	size_t                                     urd_size;  /* [const][< urd_mask] Amount of non-NULL key-item pairs. */
	COMPILER_FLEXIBLE_ARRAY(struct udict_item, urd_elem); /* [urd_mask+1] Dict key-item pairs. */
};

#define URoDict_HashSt(self, hash)  ((hash) & Dee_REQUIRES_OBJECT(URoDict, self)->urd_mask)
#define URoDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define URoDict_HashIt(self, i)     (Dee_REQUIRES_OBJECT(URoDict, self)->urd_elem + ((i) & ((URoDict *)(self))->urd_mask))


struct udict_iterator_object {
	OBJECT_HEAD
	DREF UDict        *udi_dict; /* [1..1][const] The dict that is being iterated. */
	struct udict_item *udi_next; /* [?..1][MAYBE(in(udi_dict->ud_elem))][atomic]
	                              * The first candidate for the next item.
	                              * NOTE: Before being dereferenced, this pointer is checked
	                              *       for being located inside the dict's element vector.
	                              *       In the event that it is located at its end, `ITER_DONE'
	                              *       is returned, though in the event that it is located
	                              *       outside, an error is thrown (`err_changed_sequence()'). */
};

struct urodict_iterator_object {
	OBJECT_HEAD
	DREF URoDict      *urdi_dict; /* [1..1][const] The dict that is being iterated. */
	struct udict_item *urdi_next; /* [?..1][MAYBE(in(urdi_dict->urd_elem))][atomic]
	                               * The first candidate for the next item. */
};





/* USET */
struct uset_item {
	DREF DeeObject *usi_key; /* [0..1][lock(:us_lock)] Set item key. */
};

struct uset_iterator_object {
	OBJECT_HEAD
	DREF USet        *usi_set;  /* [1..1][const] The set that is being iterated. */
	struct uset_item *usi_next; /* [?..1][MAYBE(in(usi_set->us_elem))][atomic]
	                             * The first candidate for the next item.
	                             * NOTE: Before being dereferenced, this pointer is checked
	                             *       for being located inside the set's element vector.
	                             *       In the event that it is located at its end, `ITER_DONE'
	                             *       is returned, though in the event that it is located
	                             *       outside, an error is thrown (`err_changed_sequence()'). */
};

struct uset_object {
	OBJECT_HEAD /* GC Object */
	size_t              us_mask; /* [lock(us_lock)][> us_size || us_mask == 0] Allocated set size. */
	size_t              us_used; /* [lock(us_lock)][<= us_size] Amount of keys actually in use.
	                              * HINT: The difference to `us_size' is the number of dummy keys currently in use. */
	size_t              us_size; /* [lock(us_lock)][< us_mask || us_mask == 0] Amount of non-NULL keys. */
	struct uset_item   *us_elem; /* [1..us_size|ALLOC(us_mask+1)][lock(us_lock)]
	                              * [ownes_if(!= INTERNAL(empty_set_items))] Set keys. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t us_lock; /* Lock used for accessing this set. */
#endif /* !CONFIG_NO_THREADS */
	WEAKREF_SUPPORT
};

#define USet_HashSt(self, hash)  ((hash) & Dee_REQUIRES_OBJECT(USet, self)->us_mask)
#define USet_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define USet_HashIt(self, i)     (Dee_REQUIRES_OBJECT(USet, self)->us_elem + ((i) & ((USet *)(self))->us_mask))

#define USet_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->us_lock)
#define USet_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->us_lock)
#define USet_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->us_lock)
#define USet_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->us_lock)
#define USet_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->us_lock)
#define USet_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->us_lock)
#define USet_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->us_lock)
#define USet_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->us_lock)
#define USet_LockRead(self)       Dee_atomic_rwlock_read(&(self)->us_lock)
#define USet_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->us_lock)
#define USet_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->us_lock)
#define USet_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->us_lock)
#define USet_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->us_lock)
#define USet_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->us_lock)
#define USet_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->us_lock)
#define USet_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->us_lock)


struct uroset_iterator_object {
	OBJECT_HEAD
	DREF URoSet      *ursi_set;  /* [1..1][const] The set that is being iterated. */
	struct uset_item *ursi_next; /* [?..1][MAYBE(in(ursi_set->urs_elem))][atomic]
	                              * The first candidate for the next item. */
};

struct uroset_object {
	OBJECT_HEAD
	size_t                                    urs_mask;  /* [> urs_size] Allocated set size. */
	size_t                                    urs_size;  /* [< urs_mask] Amount of non-NULL keys. */
	COMPILER_FLEXIBLE_ARRAY(struct uset_item, urs_elem); /* [1..urs_mask+1] Set key hash-vector. */
};



INTDEF struct udict_item empty_dict_items[1];
#ifdef __INTELLISENSE__
INTDEF struct uset_item empty_set_items[1];
#else /* __INTELLISENSE__ */
#define empty_set_items ((struct uset_item *)empty_dict_items)
#endif /* !__INTELLISENSE__ */

INTDEF WUNUSED NONNULL((1)) DREF USet *DCALL USet_FromSequence(DeeObject *__restrict sequence);
INTDEF WUNUSED NONNULL((1)) DREF UDict *DCALL UDict_FromSequence(DeeObject *__restrict sequence);

INTDEF WUNUSED DREF URoSet *DCALL URoSet_New(void);
INTDEF WUNUSED NONNULL((1)) DREF URoSet *DCALL URoSet_FromSequence(DeeObject *__restrict sequence);
INTDEF WUNUSED NONNULL((1)) DREF URoSet *DCALL URoSet_FromUSet(USet *__restrict self);

INTDEF WUNUSED DREF URoDict *DCALL URoDict_New(void);
INTDEF WUNUSED NONNULL((1)) DREF URoDict *DCALL URoDict_FromSequence(DeeObject *__restrict sequence);
INTDEF WUNUSED NONNULL((1)) DREF URoDict *DCALL URoDict_FromUDict(UDict *__restrict self);

/* Unique map/set types.
 * These function identical to the normal Dict/HashSet, but instead
 * of using `x.operator hash()' + `x == y' to check for duplicates,
 * these types use `Object.id(x)' + `x === y', meaning that they don't
 * rely on any user-defined operator, or on hashing being implemented. */
INTDEF DeeTypeObject USet_Type;
INTDEF DeeTypeObject USetIterator_Type;
INTDEF DeeTypeObject URoSet_Type;
INTDEF DeeTypeObject URoSetIterator_Type;

INTDEF DeeTypeObject UDict_Type;
INTDEF DeeTypeObject UDictIterator_Type;
INTDEF DeeTypeObject URoDict_Type;
INTDEF DeeTypeObject URoDictIterator_Type;


/* Base class for range-based mapping types.
 *
 * These sort of types behave similar to (and are derived from) deemon.Mapping,
 * except that rather than operating on the basis of a singular key for every
 * contained value, each contained value is mapped to a range of keys (where
 * this range is a closed range, i.e. include both its lower and upper bound)
 *
 * In order to qualify as a RangeMap, a type must implement at least:
 * >> operator for(): {(Object, Object, Object)...};
 *
 * In order to be mutable, sub-classes of RangeMap must implement at least:
 * >> operator del[:] (minkey: Object, maxkey: Object);
 * >> operator [:]= (minkey: Object, maxkey: Object, value: Object);
 *
 * A mapping of this type behaves as follows:
 * >>
 * >> //// Overwritten operators
 * >>
 * >> @@Construct an empty @RangeMap object
 * >> this();
 * >>
 * >> @@Return the value of the node containing @key
 * >> operator [] (key: Object): Object;
 * >>
 * >> @@(only if writable)
 * >> @@Alias for @(del this[key:key])
 * >> operator del[] (key: Object);
 * >>
 * >> @@(only if writable)
 * >> @@Alias for @(this[key:key] = value)
 * >> operator []= (key: Object, value: Object);
 * >>
 * >> @@(only if writable)
 * >> @@Delete all nodes that fully overlap with @[minkey:maxkey], and
 * >> @@trim/split any node (of the 0-2) that partially overlap with it.
 * >> operator del[:] (minkey: Object, maxkey: Object);
 * >>
 * >> @@(only if writable)
 * >> @@Delete all nodes that fully overlap with @[minkey:maxkey], and
 * >> @@trim/split any node (of the 0-2) that partially overlap with it,
 * >> @@then construct a new node mapping this node to @value. If a
 * >> @@simple, primitive key-type is used, and @value is identic to
 * >> @@that of an adjacent node (as per @===), nodes *may* be merged.
 * >> operator [:]= (minkey: Object, maxkey: Object, value: Object);
 * >>
 * >> @@Return the number of nodes within @this @RangeMap
 * >> operator # (): int;
 * >>
 * >> @@Evaluate to true when @this @RangeMap contains at least @1 node.
 * >> operator bool(): bool;
 * >>
 * >> @@Generate a string describing the look of @this @RangeMap as follows:
 * >> @@>> { [minkey1:maxkey1]: value1, [minkey2:maxkey2]: value1, ... [minkeyN:maxkeyN]: valueN }
 * >> operator repr(): string;
 * >>
 * >> @@Check if a range exists that contains @key
 * >> operator contains(key: Object): bool;
 * >>
 * >> @@Return a proxy describing the union of @this @RangeMap and @other
 * >> @@Where both @RangeMap contain overlapping nodes, it is undefined
 * >> @@which value will be bound in the resulting @RangeMap
 * >> operator | (other: RangeMap): RangeMap;
 * >> operator | (other: {(Object, Object, Object)...}): RangeMap;
 * >>
 * >> @@Return a proxy describing the itersection of @this @RangeMap and @other
 * >> @@Only ranges that exist in both maps will be included in the resulting @RangeMap,
 * >> @@though if @this and @other map different values, it is undefined which will
 * >> @@appear in the resulting @RangeMap.
 * >> operator & (other: RangeMap): RangeMap;
 * >> operator & (other: {(Object, Object, Object)...}): RangeMap;
 * >>
 * >> @@Return a proxy describing all ranges that appear in only one of the given @(RangeMap)s
 * >> operator ^ (other: RangeMap): RangeMap;
 * >> operator ^ (other: {(Object, Object, Object)...}): RangeMap;
 * >>
 * >> @@Produce a hash of all minkey/maxkey/value nodes contained within the RangeMap.
 * >> @@This hash is equal to:
 * >> @@>> local result = 0;
 * >> @@>> for (local minkey, maxkey, value: this)
 * >> @@>>     result ^= deemon.hash(minkey, maxkey, value);
 * >> @@>> return result;
 * >> operator hash(): int;
 * >>
 * >> @@Check for equality / inequality between @(RangeMap)s
 * >> operator == (other: RangeMap): bool;
 * >> operator == (other: {(Object, Object, Object)...}): bool;
 * >> operator != (other: RangeMap): bool;
 * >> operator != (other: {(Object, Object, Object)...}): bool;
 * >>
 * >>
 * >>
 * >> //// Overwritten properties
 * >>
 * >> @@The first element of the @RangeMap as a @(minkey, maxkey, value)-tuple
 * >> @@@throws ValueError @this @RangeMap is empty
 * >> property first: (Object, Object, Object) = { get() { ... } };
 * >>
 * >> @@The last element of the @RangeMap as a @(minkey, maxkey, value)-tuple
 * >> @@@throws ValueError @this @RangeMap is empty
 * >> property last: (Object, Object, Object) = { get() { ... } };
 * >>
 * >> @@Return a set object that contains all keys that are apart
 * >> @@of any of the nodes of @this @RangeMap. This set can only
 * >> @@be enumerated if all used keys implement @(operator copy)
 * >> @@and @(operator ++). Otherwise, it is still possible to check
 * >> @@for membership using @(operator contains) (which behaves the
 * >> @@same as @(RangeMap.operator contains))
 * >> property keys: {Object...} = { get() { ... } };
 * >> property iterkeys: Keys.Iterator = { get() { ... } };
 * >>
 * >> @@Return the set-type of @keys
 * >> static property Keys: Type = { get() { ... } };
 * >>
 * >> @@Same as @Mapping.values
 * >> property values: {Object...} = { get() { ... } };
 * >> property itervalues: Values.Iterator = { get() { ... } };
 * >>
 * >> @@Return the sequence-type of @values
 * >> static property Values: Type = { get() { ... } };
 * >>
 * >> @@Return a sequence proxy to enumerate the min/max/value of
 * >> @@each defined node.
 * >> property items: {(Object, Object, Object)...} = { get() { ... } };
 * >> property iteritems: Items.Iterator = { get() { ... } };
 * >>
 * >> @@Return the sequence-type of @items
 * >> static property Items: Type = { get() { ... } };
 * >>
 * >> @@Common sub-class of @Keys, @Values, @Items, @AsMap, @AsMap.Keys, @AsMap.Values and @AsMap.Items
 * >> static property Proxy: Type = { get() { ... } };
 * >>
 * >> @@Return the effective iterator type
 * >> static property Iterator: Type = { get() { ... } };
 * >>
 * >>
 * >>
 * >> //// Overwritten functions
 * >>
 * >> @@Delete all nodes
 * >> function clear();
 * >>
 * >> @@Pop a random node and return it as a @(minkey, maxkey, value)-tuple
 * >> @@@throws ValueError @this @RangeMap is empty
 * >> function popitem(): (Object, Object, Object);
 * >>
 * >> @@Return the value of the node containing @key, or @def if no such node exists
 * >> function get(key: Object, def: Object = none): Object;
 * >>
 * >> @@Return the value of the node containing @key, or create a new node:
 * >> @@>> try {
 * >> @@>>     return this[key];
 * >> @@>> } catch (KeyError) {
 * >> @@>> }
 * >> @@>> this[key] = def;
 * >> @@>> return def;
 * >> function setdefault(key: Object, def: Object = none): Object;
 * >>
 * >> @@Remove a singular @key without removing the containing node. Same as:
 * >> @@>> local result = this[key];
 * >> @@>> del this[key];
 * >> @@>> return result;
 * >> @@@throws KeyError No node contains @key
 * >> function pop(key: Object): Object;
 * >> function pop(key: Object, def: Object): Object;
 * >>
 * >> @@Override from @Mapping.update, taking a sequence of @(minkey, maxkey, value)-tuples
 * >> function update(items: {(Object, Object, Object)...});
 * >>
 * >> @@Remove and return the @first node
 * >> @@@throws ValueError @this @RangeMap is empty
 * >> function popfront(): (Object, Object, Object);
 * >>
 * >> @@Remove and return the @last node
 * >> @@@throws ValueError @this @RangeMap is empty
 * >> function popback(): (Object, Object, Object);
 * >>
 * >>
 * >>
 * >> //// New properties & functions
 * >>
 * >> @@Return a sequence proxy to enumerate the key-ranges of @this @RangeMap
 * >> property ranges: {(Object, Object)...} = { get() { ... } };
 * >> property iterranges: Ranges.Iterator = { get() { ... } };
 * >>
 * >> @@Return the sequence-type of @ranges
 * >> static property Ranges: Type = { get() { ... } };
 * >>
 * >> @@Same as @this.asmap.items
 * >> property mapitems: {(Object, Object)...} = { get() { ... } };
 * >> property itermapitems: MapItems.Iterator = { get() { ... } };
 * >>
 * >> @@Return the sequence-type of @mapitems
 * >> static property MapItems: Type = { get() { ... } };
 * >>
 * >> @@Return a sequence proxy to enumerate the nodes of @this @RangeMap
 * >> property nodes: {Iterator...} = { get() { ... } };
 * >> property iternodes: Nodes.Iterator = { get() { ... } };
 * >>
 * >> @@Return the sequence-type of @nodes
 * >> static property Nodes: Type = { get() { ... } };
 * >>
 * >> @@Return a map-proxy that allows the node-map to be used as
 * >> @@though it were a regular map. Any operation performed on the
 * >> @@returned map-like object is forwarded to @this @RangeMap
 * >> property asmap: {Object: Object} = { get() { ... } };
 * >> property iterasmap: AsMap.Iterator = { get() { ... } };
 * >>
 * >> @@Return the map-type of @asmap
 * >> static property AsMap: Type = { get() { ... } };
 * >>
 * >> @@Return a @(minkey, maxkey, value)-tuple for the node
 * >> @@containing @key, or @none if no such node exists
 * >> function locate(key: Object): (Object, Object, Object) | none;
 * >>
 * >> @@Return an iterator pointing to the node
 * >> @@containing @key, or @none if no such node exists
 * >> function itlocate(key: Object): Iterator | none;
 * >>
 * >> @@Return a sequence of @(minkey, maxkey, value)-tuple describing
 * >> @@all nodes that overlap with the given @[minkey:maxkey] node.
 * >> function rlocate(minkey: Object, maxkey: Object): {(Object, Object, Object)...};
 * >>
 * >> @@Remove and return the node containing @key, or @none if no such node exists
 * >> function remove(key: Object): (Object, Object, Object) | none;
 * >>
 * >> @@Remove all nodes that overlap with @[minkey:maxkey]
 * >> function rremove(minkey: Object, maxkey: Object): {(Object, Object, Object)...};
 * >>
 * >> @@Return the node that precedes the one containing @key, or @none if no such node exists
 * >> function prevnode(key: Object): (Object, Object, Object) | none;
 * >>
 * >> @@Return the node that succeeds the one containing @key, or @none if no such node exists
 * >> function nextnode(key: Object): (Object, Object, Object) | none;
 * >>
 * >> @@Return an iterator to the node that precedes the one containing @key, or @none if no such node exists
 * >> function itprevnode(key: Object): Iterator | none;
 * >>
 * >> @@Return an iterator to the node that succeeds the one containing @key, or @none if no such node exists
 * >> function itnextnode(key: Object): Iterator | none;
 * >>
 * >> @@Try to merge adjacent nodes
 * >> function optimize();
 *
 * And its iterator behaves as follows:
 * >> @@Remove the node selected by @this iterator. Same as @seq.remove(minkey)
 * >> @@@throws ValueError No node selected
 * >> function removenode();
 * >>
 * >> @@@throws UnboundAttribute No node selected
 * >> property minkey: Object = { get() { ... } };
 * >> @@@throws UnboundAttribute No node selected
 * >> property maxkey: Object = { get() { ... } };
 * >> @@@throws UnboundAttribute No node selected
 * >> property value: Object = { get() { ... } };
 */
INTDEF DeeTypeObject RangeMap_Type;
INTDEF DeeTypeObject RangeMapProxy_Type;
INTDEF DeeTypeObject RangeMapProxyIterator_Type;
INTDEF DeeTypeObject RangeMapKeys_Type; /* {Key...} */
INTDEF DeeTypeObject RangeMapKeysIterator_Type;
INTDEF DeeTypeObject RangeMapValues_Type; /* {Value...} */
INTDEF DeeTypeObject RangeMapValuesIterator_Type;
INTDEF DeeTypeObject RangeMapItems_Type; /* {(MinKey, MaxKey, Value)...} */
INTDEF DeeTypeObject RangeMapItemsIterator_Type;
INTDEF DeeTypeObject RangeMapNodes_Type; /* {Iterator...} */
INTDEF DeeTypeObject RangeMapNodesIterator_Type;
INTDEF DeeTypeObject RangeMapRanges_Type; /* {(MinKey, MaxKey)...} */
INTDEF DeeTypeObject RangeMapRangesIterator_Type;
INTDEF DeeTypeObject RangeMapMapItems_Type; /* {(Key, Value)...} */
INTDEF DeeTypeObject RangeMapMapItemsIterator_Type;
INTDEF DeeTypeObject RangeMapAsMap_Type; /* {Key: Value} */
INTDEF DeeTypeObject RangeMapAsMapIterator_Type;


/* R/B tree types:
 * >> import RBTree from collections;
 * >> local rb = RBTree();
 * >> rb[10:20] = "foo";    // XXX: This goes against conventions: The upper-bound is conventionally
 * >>                       //      1-PAST the last element, but here it would *BE* the last element!
 * >>                       // However, I don't have any real idea of how to fix this...
 * >> print rb.get(9);  // none
 * >> print rb.get(10); // "foo"
 * >> print rb.get(15); // "foo"
 * >> print rb.get(20); // "foo"
 * >> print rb.get(21); // none
 *
 * A special map type that uses an R/B-trees to facilitate the use
 * of red/black trees in order to reference not just simple keys,
 * but ranges of keys.
 *
 * Unlike normal mappings (which behave like `{(Key, Value)...}'),
 * RBTree mappings behave like `{(MinKey, MaxKey, Value)...}' when
 * iterated or used with some other sequence-style function.
 *
 * However, it is also possible to initialize this mapping like this:
 * >> import RBTree from collections;
 * >> local rm = RBTree({
 * >>     [10:20]: "foo",
 * >>     [21:30]: "bar",
 * >> });
 * iow: by using a mapping where the keys are `rt.SeqRange' objects.
 * This representation is also the *native* representation of an
 * RBTree object, and is what's generated by `operator repr()'
 */
INTDEF DeeTypeObject RBTree_Type;
INTDEF DeeTypeObject RBTreeIterator_Type;



/* Bitset-related types. */
INTDEF DeeTypeObject Bitset_Type;
INTDEF DeeTypeObject RoBitset_Type;
INTDEF DeeTypeObject BitsetView_Type;
INTDEF DeeTypeObject BitsetIterator_Type;



INTDEF ATTR_COLD NONNULL((1)) int (DCALL err_changed_sequence)(DeeObject *__restrict seq);
INTDEF ATTR_COLD NONNULL((1)) int (DCALL err_unimplemented_operator)(DeeTypeObject const *__restrict tp, Dee_operator_t operator_name);


/* Common error documentation strings */
#define DOC_ERROR_ValueError_EMPTY_SEQUENCE \
	"#tValueError{@this ?. is empty}"
#define DOC_ERROR_KeyError_NO_SUCH_KEY \
	"#tKeyError{No such @key}"
#define DOC_ERROR_NotImplemented_CANNOT_SPLIT \
	"#tNotImplemented{Tried to split a node with a key-type that does not support predecessor/successor semantics}"

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H */
