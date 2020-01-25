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
#ifndef GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H
#define GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/util/rwlock.h>

DECL_BEGIN

#define DEQUE_BUCKET_DEFAULT_SIZE  62 /* 64-2 */

typedef struct deque_bucket {
	struct deque_bucket **db_pself;    /* [1..1][== self][1..1] Deque bucket self-pointer. */
	struct deque_bucket  *db_next;     /* [0..1][owned] Next deque bucket. */
	DREF DeeObject       *db_items[1]; /* [1..1][valid_if(IN_BOUNDS)] Items of this bucket. */
} DequeBucket;

#define DEQUEBUCKET_PREV(x)         COMPILER_CONTAINER_OF((x)->db_pself, DequeBucket, db_next)
#define DEQUEBUCKET_NEXT(x)         ((x)->db_next)
#define DEQUEBUCKET_HASPREV(deq, x) ((x) != (deq)->d_head)
#define DEQUEBUCKET_HASNEXT(deq, x) ((x) != (deq)->d_tail)

#define SIZEOF_BUCKET(bucket_size) \
	(offsetof(DequeBucket, db_items) + (bucket_size) * sizeof(DREF DeeObject *))
#define NEW_BUCKET(bucket_size) \
	((DequeBucket *)Dee_Malloc(SIZEOF_BUCKET(bucket_size)))
#define TRY_NEW_BUCKET(bucket_size) \
	((DequeBucket *)Dee_TryMalloc(SIZEOF_BUCKET(bucket_size)))


typedef struct deque {
	Dee_OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_rwlock_t d_lock;      /* Lock for this deque. */
#endif /* !CONFIG_NO_THREADS */
	DequeBucket *d_head;      /* [lock(d_lock)][0..1] Head bucket. */
	DequeBucket *d_tail;      /* [lock(d_lock)][0..1] Tail bucket. */
	size_t       d_size;      /* [lock(d_lock)] Total number of objects stored in this deque. */
	size_t       d_head_idx;  /* [lock(d_lock)][< d_bucket_sz][if(d_size == 0,[== 0])]
	                           * Absolute index where the head starts. */
	size_t       d_head_use;  /* [lock(d_lock)][<= d_bucket_sz][if(d_size == 0,[== 0])]
	                           * Amount of indices in use in `d_head'. */
	size_t       d_tail_sz;   /* [lock(d_lock)][if(d_head == d_tail,[== 0])]
	                           * [if(d_head && d_head != d_tail,[!= 0])][<= d_bucket_sz]
	                           * Number of items in use by the tail-bucket (only when there are more than 2 buckets). */
	size_t       d_bucket_sz; /* [lock(d_lock)][!0] Size of a bucket (in objects) */
	size_t       d_version;   /* [lock(d_lock)] Incremented every time the deque changes. */
	Dee_WEAKREF_SUPPORT
} Deque;

#define Deque_LockRead(self)      rwlock_read(&(self)->d_lock)
#define Deque_LockWrite(self)     rwlock_write(&(self)->d_lock)
#define Deque_LockEndRead(self)   rwlock_endread(&(self)->d_lock)
#define Deque_LockEndWrite(self)  rwlock_endwrite(&(self)->d_lock)
#define Deque_LockDowngrade(self) rwlock_downgrade(&(self)->d_lock)

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
LOCAL DeeObject **DCALL
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Deque_PushFront(Deque *__restrict self, DeeObject *__restrict item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL Deque_PushBack(Deque *__restrict self, DeeObject *__restrict item);
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
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL Deque_PushFront_unlocked(Deque *__restrict self, DeeObject *__restrict item);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL Deque_PushBack_unlocked(Deque *__restrict self, DeeObject *__restrict item);
INTDEF bool DCALL Deque_Insert_unlocked(Deque *__restrict self, size_t index, DeeObject *__restrict item);

/* @return: * : The item that was popped. */
INTDEF ATTR_RETNONNULL DREF DeeObject *DCALL Deque_PopFront_unlocked(Deque *__restrict self);
INTDEF ATTR_RETNONNULL DREF DeeObject *DCALL Deque_PopBack_unlocked(Deque *__restrict self);
INTDEF ATTR_RETNONNULL DREF DeeObject *DCALL Deque_Pop_unlocked(Deque *__restrict self, size_t index);


/* Insert/delete an item at a given index. */
INTDEF int DCALL Deque_Insert(Deque *__restrict self, size_t index, DeeObject *__restrict item);
INTDEF WUNUSED NONNULL((1)) size_t DCALL Deque_Erase(Deque *__restrict self, size_t index, size_t num_items);
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
		result            = self->di_index - DEQUE_HEADFREE(deq);
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
	Dee_rwlock_t  di_lock; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
	DequeIterator di_iter; /* [lock(di_lock)] The C-level iterator used to implement this one. */
	DREF Deque   *di_deq;  /* [1..1][const] The deque in question. */
	size_t        di_ver;  /* [lock(di_lock)] The deque version for this this iterator was created.
	                        * When this number doesn't match `di_deq->d_version', then the
	                        * iterator behaves as though it was exhausted. */
} DequeIteratorObject;

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
	Dee_rwlock_t    fl_lock;       /* Lock for this list. */
#endif /* !CONFIG_NO_THREADS */
	size_t          fl_size;       /* [const] Length of the list. */
	DREF DeeObject *fl_elem[1024]; /* [0..1][fl_size] List items. */
} FixedList;

typedef struct {
	Dee_OBJECT_HEAD
	DREF FixedList    *li_list; /* [1..1][const] The list being iterated. */
	ATOMIC_DATA size_t li_iter; /* Iterator position. */
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
	DREF DeeObject *di_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
	DREF DeeObject *di_value; /* [1..1|if(di_key == dummy,0..0)][valid_if(di_key)]
	                           * [lock(:d_lock)] Dictionary item value. */
};

struct udict_object {
	OBJECT_HEAD /* GC Object */
	size_t             d_mask; /* [lock(d_lock)][> d_size || d_mask == 0] Allocated dictionary size. */
	size_t             d_size; /* [lock(d_lock)][< d_mask || d_mask == 0] Amount of non-NULL key-item pairs. */
	size_t             d_used; /* [lock(d_lock)][<= d_size] Amount of key-item pairs actually in use.
	                            *  HINT: The difference to `d_size' is the number of dummy keys currently in use. */
	struct udict_item *d_elem; /* [1..d_size|ALLOC(d_mask+1)][lock(d_lock)]
	                            * [owned_if(!= INTERNAL(empty_dict_items))] Dict key-item pairs (items). */
#ifndef CONFIG_NO_THREADS
	rwlock_t           d_lock; /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	WEAKREF_SUPPORT
};

struct urodict_object {
	OBJECT_HEAD
	size_t            rd_mask;    /* [const][!0] Allocated dictionary mask. */
	size_t            rd_size;    /* [const][< rd_mask] Amount of non-NULL key-item pairs. */
	struct udict_item rd_elem[1]; /* [rd_mask+1] Dict key-item pairs. */
};



/* USET */
struct uset_item {
	DREF DeeObject *si_key; /* [0..1][lock(:s_lock)] Set item key. */
};

struct uset_iterator_object {
	OBJECT_HEAD
	DREF USet        *si_set;  /* [1..1][const] The set that is being iterated. */
	struct uset_item *si_next; /* [?..1][MAYBE(in(si_set->s_elem))][atomic]
	                            *   The first candidate for the next item.
	                            *   NOTE: Before being dereferenced, this pointer is checked
	                            *         for being located inside the set's element vector.
	                            *         In the event that it is located at its end, `ITER_DONE'
	                            *         is returned, though in the event that it is located
	                            *         outside, an error is thrown (`err_changed_sequence()'). */
};

struct uset_object {
	OBJECT_HEAD /* GC Object */
	size_t            s_mask; /* [lock(s_lock)][> s_size || s_mask == 0] Allocated set size. */
	size_t            s_size; /* [lock(s_lock)][< s_mask || s_mask == 0] Amount of non-NULL keys. */
	size_t            s_used; /* [lock(s_lock)][<= s_size] Amount of keys actually in use.
	                           * HINT: The difference to `s_size' is the number of dummy keys currently in use. */
	struct uset_item *s_elem; /* [1..s_size|ALLOC(s_mask+1)][lock(s_lock)]
	                           * [ownes_if(!= INTERNAL(empty_set_items))] Set keys. */
#ifndef CONFIG_NO_THREADS
	rwlock_t          s_lock; /* Lock used for accessing this set. */
#endif /* !CONFIG_NO_THREADS */
	WEAKREF_SUPPORT
};

struct uroset_iterator_object {
	OBJECT_HEAD
	DREF URoSet      *si_set;  /* [1..1][const] The set that is being iterated. */
	struct uset_item *si_next; /* [?..1][MAYBE(in(si_set->s_elem))][atomic]
	                            *   The first candidate for the next item.
	                            *   NOTE: Before being dereferenced, this pointer is checked
	                            *         for being located inside the set's element vector.
	                            *         In the event that it is located at its end, `ITER_DONE'
	                            *         is returned, though in the event that it is located
	                            *         outside, an error is thrown (`err_changed_sequence()'). */
};

struct uroset_object {
	OBJECT_HEAD
	size_t           rs_mask;    /* [> rs_size] Allocated set size. */
	size_t           rs_size;    /* [< rs_mask] Amount of non-NULL keys. */
	struct uset_item rs_elem[1]; /* [1..rs_mask+1] Set key hash-vector. */
};



INTDEF struct udict_item empty_dict_items[1];
#ifdef __INTELLISENSE__
INTDEF struct uset_item empty_set_items[1];
#else /* __INTELLISENSE__ */
#define empty_set_items ((struct uset_item *)empty_dict_items)
#endif /* !__INTELLISENSE__ */

INTDEF WUNUSED NONNULL((1)) DREF USet *DCALL USet_FromSequence(DeeObject *__restrict sequence);
INTDEF WUNUSED NONNULL((1)) DREF UDict *DCALL UDict_FromSequence(DeeObject *__restrict sequence);
INTDEF WUNUSED NONNULL((1)) DREF URoSet *DCALL URoSet_FromIterator(DeeObject *__restrict iterator);
INTDEF WUNUSED NONNULL((1)) DREF URoSet *DCALL URoSet_FromSequence(DeeObject *__restrict sequence);
INTDEF WUNUSED NONNULL((1)) DREF URoDict *DCALL URoDict_FromIterator(DeeObject *__restrict iterator);
INTDEF WUNUSED NONNULL((1)) DREF URoDict *DCALL URoDict_FromSequence(DeeObject *__restrict sequence);

/* Unique map/set types.
 * These function identical to the normal Dict/HashSet, but instead
 * of using `x.operator hash()' + `x == y' to check for duplicates,
 * these types use `Object.id(x)' + `x === y', meaning that they don't
 * rely on any user-defined operator, or on hashing being implemented. */
INTDEF DeeTypeObject USet_Type;
INTDEF DeeTypeObject USetIterator_Type;
INTDEF DeeTypeObject UDict_Type;
INTDEF DeeTypeObject UDictIterator_Type;
INTDEF DeeTypeObject URoSet_Type;
INTDEF DeeTypeObject URoSetIterator_Type;
INTDEF DeeTypeObject URoDict_Type;
INTDEF DeeTypeObject URoDictIterator_Type;



INTDEF ATTR_COLD int DCALL err_empty_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD int DCALL err_index_out_of_bounds(DeeObject *__restrict self, size_t index, size_t size);
INTDEF ATTR_COLD int DCALL err_unbound_index(DeeObject *__restrict self, size_t index);
INTDEF ATTR_COLD int DCALL err_changed_sequence(DeeObject *__restrict seq);


DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H */
