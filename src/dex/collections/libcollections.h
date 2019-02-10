/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
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
#define DEQUEBUCKET_PREV(x)          COMPILER_CONTAINER_OF((x)->db_pself,DequeBucket,db_next)
#define DEQUEBUCKET_NEXT(x)          ((x)->db_next)
#define DEQUEBUCKET_HASPREV(deq,x)   ((x) != (deq)->d_head)
#define DEQUEBUCKET_HASNEXT(deq,x)   ((x) != (deq)->d_tail)

#define SIZEOF_BUCKET(bucket_size) \
   (offsetof(DequeBucket,db_items)+(bucket_size)*sizeof(DREF DeeObject *))
#define NEW_BUCKET(bucket_size) \
   ((DequeBucket *)Dee_Malloc(SIZEOF_BUCKET(bucket_size)))
#define TRY_NEW_BUCKET(bucket_size) \
   ((DequeBucket *)Dee_TryMalloc(SIZEOF_BUCKET(bucket_size)))


typedef struct deque {
    OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
    rwlock_t     d_lock;      /* Lock for this deque. */
#endif
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
    WEAKREF_SUPPORT
} Deque;

#define Deque_LockRead(self)      rwlock_read(&(self)->d_lock)
#define Deque_LockWrite(self)     rwlock_write(&(self)->d_lock)
#define Deque_LockEndRead(self)   rwlock_endread(&(self)->d_lock)
#define Deque_LockEndWrite(self)  rwlock_endwrite(&(self)->d_lock)
#define Deque_LockDowngrade(self) rwlock_downgrade(&(self)->d_lock)

#define DEQUE_BUCKETSZ(x)  ((x)->d_bucket_sz)
#define DEQUE_HEADFREE(x)  ((x)->d_head_idx)
#define DEQUE_HEADUSED(x)  ((x)->d_head_use)
#define DEQUE_TAILUSED(x)  ((x)->d_tail_sz)
#define DEQUE_TAILFREE(x)  (DEQUE_BUCKETSZ(x)-DEQUE_TAILUSED(x))

/* Returns the base-relative indices at which the head/tail bucket start at. */
#define DEQUE_HEADSTART(x)   0
#define DEQUE_HEADEND(x)     DEQUE_HEADUSED(x)
#define DEQUE_TAILSTART(x) ((x)->d_size-(x)->d_tail_sz)
#define DEQUE_TAILEND(x)    (x)->d_size

/* Returns l-values for the front, back or base-relative i'th item of the given deque. */
#define DEQUE_HEAD(x)   (ASSERT((x)->d_size),(x)->d_head->db_items[DEQUE_HEADFREE(x)])
#define DEQUE_TAIL(x)   (ASSERT((x)->d_size),(x)->d_head == (x)->d_tail \
                                           ? (x)->d_tail->db_items[(x)->d_head_idx+(x)->d_head_use-1] \
                                           : (x)->d_tail->db_items[DEQUE_TAILUSED(x)-1])
#define DEQUE_ITEM(x,i) (*Deque_ItemPointer(x,i))


/* Returns a pointer to the base-relative index'th item. */
LOCAL DeeObject **DCALL
Deque_ItemPointer(Deque *__restrict self, size_t index) {
 DequeBucket *iter = self->d_head;
 ASSERT(index < self->d_size);
 if (index < DEQUE_HEADUSED(self))
     return &iter->db_items[DEQUE_HEADFREE(self) + index];
 if (index >= self->d_size/2) {
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
INTDEF int DCALL Deque_PushFront(Deque *__restrict self, DeeObject *__restrict item);
INTDEF int DCALL Deque_PushBack(Deque *__restrict self, DeeObject *__restrict item);
INTDEF DREF DeeObject *DCALL Deque_PopFront(Deque *__restrict self);
INTDEF DREF DeeObject *DCALL Deque_PopBack(Deque *__restrict self);

/* Deque rotation */
INTDEF void DCALL Deque_llrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the first `num_objects' left. */
INTDEF void DCALL Deque_lrrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the first `num_objects' right. */
INTDEF void DCALL Deque_rlrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the last `num_objects' left. */
INTDEF void DCALL Deque_rrrot_unlocked(Deque *__restrict self, size_t num_objects); /* Rotate the last `num_objects' right. */

INTDEF int DCALL Deque_llrot(Deque *__restrict self, size_t num_objects);
INTDEF int DCALL Deque_lrrot(Deque *__restrict self, size_t num_objects);
INTDEF int DCALL Deque_rlrot(Deque *__restrict self, size_t num_objects);
INTDEF int DCALL Deque_rrrot(Deque *__restrict self, size_t num_objects);


/* @return: true:  Successfully inserted the given item.
 * @return: false: Insertion failed. - Unlock the deque and collect
 *                `SIZEOF_BUCKET(self->d_bucket_sz)' bytes of memory. */
INTDEF bool DCALL Deque_PushFront_unlocked(Deque *__restrict self, DeeObject *__restrict item);
INTDEF bool DCALL Deque_PushBack_unlocked(Deque *__restrict self, DeeObject *__restrict item);
INTDEF bool DCALL Deque_Insert_unlocked(Deque *__restrict self, size_t index, DeeObject *__restrict item);

/* @return: * : The item that was popped. */
INTDEF ATTR_RETNONNULL DREF DeeObject *DCALL Deque_PopFront_unlocked(Deque *__restrict self);
INTDEF ATTR_RETNONNULL DREF DeeObject *DCALL Deque_PopBack_unlocked(Deque *__restrict self);
INTDEF ATTR_RETNONNULL DREF DeeObject *DCALL Deque_Pop_unlocked(Deque *__restrict self, size_t index);


/* Insert/delete an item at a given index. */
INTDEF int DCALL Deque_Insert(Deque *__restrict self, size_t index, DeeObject *__restrict item);
INTDEF size_t DCALL Deque_Erase(Deque *__restrict self, size_t index, size_t num_items);
INTDEF DREF DeeObject *DCALL Deque_Pop(Deque *__restrict self, size_t index);
INTDEF DREF DeeObject *DCALL Deque_Pops(Deque *__restrict self, dssize_t index);

typedef struct {
    DequeBucket *di_bucket;
    size_t       di_index;
} DequeIterator;

/* C-level deque iteration helpers. */
#define DequeIterator_ITEM(self)         ((self)->di_bucket->db_items[(self)->di_index])
#define DequeIterator_PREVITEM(self,deq) (*DequeIterator_PrevItemPointer(self,deq))
#define DequeIterator_NEXTITEM(self,deq) (*DequeIterator_NextItemPointer(self,deq))

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
 ASSERT(index < deq->d_size);
 if (index < DEQUE_HEADUSED(deq)) {
  self->di_bucket = iter;
  self->di_index = DEQUE_HEADFREE(deq) + index;
  return;
 }
 if (index >= deq->d_size/2) {
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
  ASSERT(deq->d_head_use != 0);
  self->di_index = deq->d_head_idx+deq->d_head_use-1;
 } else {
  ASSERT(deq->d_tail_sz != 0);
  self->di_index = deq->d_tail_sz-1;
 }
}
LOCAL void DCALL
DequeIterator_InitEnd(DequeIterator *__restrict self,
                      Deque *__restrict deq) {
 self->di_bucket = deq->d_tail;
 if (deq->d_tail == deq->d_head) {
  ASSERT(deq->d_head_use != 0);
  self->di_index = deq->d_head_idx+deq->d_head_use;
 } else {
  ASSERT(deq->d_tail_sz != 0);
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
 if (self->di_bucket != deq->d_tail) return true;
 if (deq->d_head != deq->d_tail)
     return self->di_index+1 < deq->d_tail_sz;
 return self->di_index+1 < deq->d_head_idx+deq->d_head_use;
}
LOCAL void DCALL
DequeIterator_Prev(DequeIterator *__restrict self,
                   Deque *__restrict deq) {
 if (!self->di_index--) {
  self->di_bucket = DEQUEBUCKET_PREV(self->di_bucket);
  self->di_index  = deq->d_bucket_sz-1;
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
LOCAL DREF DeeObject **DCALL
DequeIterator_PrevItemPointer(DequeIterator *__restrict self,
                              Deque *__restrict deq) {
 ASSERT(DequeIterator_HasPrev(self,deq));
 if (self->di_index != 0)
     return &self->di_bucket->db_items[self->di_index-1];
 return &DEQUEBUCKET_PREV(self->di_bucket)->db_items[deq->d_bucket_sz-1];
}
LOCAL DREF DeeObject **DCALL
DequeIterator_NextItemPointer(DequeIterator *__restrict self,
                              Deque *__restrict deq) {
 ASSERT(DequeIterator_HasNext(self,deq));
 if (self->di_index+1 < deq->d_bucket_sz)
     return &self->di_bucket->db_items[self->di_index+1];
 return &DEQUEBUCKET_NEXT(self->di_bucket)->db_items[0];
}


typedef struct {
    OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
    rwlock_t      di_lock; /* Lock for this iterator. */
#endif
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
    OBJECT_HEAD /* GC object */
    WEAKREF_SUPPORT
#ifndef CONFIG_NO_THREADS
    rwlock_t        fl_lock;       /* Lock for this list. */
#endif
    size_t          fl_size;       /* [const] Length of the list. */
    DREF DeeObject *fl_elem[1024]; /* [0..1][fl_size] List items. */
} FixedList;

typedef struct {
    OBJECT_HEAD
    DREF FixedList    *li_list; /* [1..1][const] The list being iterated. */
    ATOMIC_DATA size_t li_iter; /* Iterator position. */
} FixedListIterator;


INTDEF DeeTypeObject FixedList_Type;
INTDEF DeeTypeObject FixedListIterator_Type;




INTDEF ATTR_COLD int DCALL err_empty_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD int DCALL err_index_out_of_bounds(DeeObject *__restrict self, size_t index, size_t size);
INTDEF ATTR_COLD int DCALL err_unbound_index(DeeObject *__restrict self, size_t index);


DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_H */
