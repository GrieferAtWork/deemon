/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_COLLECTIONS_DEQUE_C
#define GUARD_DEX_COLLECTIONS_DEQUE_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

/**/
#include "kwlist.h"

DECL_BEGIN


INTERN WUNUSED NONNULL((1, 2)) bool DCALL
Deque_PushFront_unlocked(Deque *self, DeeObject *item) {
	DequeBucket *new_bucket;
	ASSERT(self->d_head_idx < self->d_bucket_sz);
	ASSERT(self->d_head_use <= self->d_bucket_sz);
	ASSERT(self->d_tail_sz <= self->d_bucket_sz);
	if (self->d_head_idx != 0) {
		/* Pre-allocated space is available. */
		--self->d_head_idx;
		++self->d_size;
		++self->d_head_use;
		self->d_head->db_items[self->d_head_idx] = item;
		Dee_Incref(item);
		++self->d_version;
		return true;
	}
	/* Need a new bucket. */
	new_bucket = TRY_NEW_BUCKET(self->d_bucket_sz);
	if unlikely(!new_bucket)
		return false;

	/* Insert the new bucket. */
	self->d_head_use     = 1;
	new_bucket->db_pself = &self->d_head;
	if ((new_bucket->db_next = self->d_head) != NULL) {
		/* New bucket in the front (insert item at the end) */
		self->d_head_idx = self->d_bucket_sz - 1;
		if (self->d_head == self->d_tail) {
			ASSERT(self->d_size <= self->d_bucket_sz);
			/* Second bucket created (enter split head-tail-mode) */
			self->d_tail_sz = self->d_size;
		}
		new_bucket->db_next->db_pself = &new_bucket->db_next;
	} else {
		ASSERT(self->d_size == 0);
		ASSERT(self->d_tail_sz == 0);
		/* First bucket (start in the middle). */
		self->d_head_idx = self->d_bucket_sz / 2;
		self->d_tail     = new_bucket;
	}
	self->d_head                           = new_bucket;
	new_bucket->db_items[self->d_head_idx] = item;
	Dee_Incref(item);
	++self->d_size;
	++self->d_version;
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
Deque_PushBack_unlocked(Deque *self, DeeObject *item) {
	DequeBucket *new_bucket;
	ASSERT(self->d_head_idx < self->d_bucket_sz);
	ASSERT(self->d_head_use <= self->d_bucket_sz);
	ASSERT(self->d_tail_sz <= self->d_bucket_sz);
	if (self->d_head != self->d_tail) {
		/* Append at the end of a secondary bucket. */
		if (self->d_tail_sz < self->d_bucket_sz) {
			self->d_tail->db_items[self->d_tail_sz] = item;
			++self->d_tail_sz;
			++self->d_size;
			Dee_Incref(item);
			++self->d_version;
			return true;
		}
	} else if (self->d_tail) {
		size_t tail_free;
		/* Single-bucket, non-empty mode. */
		ASSERT(self->d_tail_sz == 0);
		ASSERT(self->d_size != 0);
		tail_free = self->d_head_idx + self->d_head_use;
		ASSERT(tail_free <= self->d_bucket_sz);
		if (tail_free < self->d_bucket_sz) {
			/* Append at the end of the first bucket. */
			++self->d_head_use;
			++self->d_size;
			self->d_head->db_items[tail_free] = item;
			Dee_Incref(item);
			++self->d_version;
			return true;
		}
	}
	/* Need a new bucket. */
	new_bucket = TRY_NEW_BUCKET(self->d_bucket_sz);
	if unlikely(!new_bucket)
		return false;
	new_bucket->db_next = NULL;

	if (!self->d_tail) {
		/* First bucket (start in the middle) */
		ASSERT(self->d_tail_sz == 0);
		new_bucket->db_pself                   = &self->d_head;
		self->d_head                           = new_bucket;
		self->d_head_idx                       = self->d_bucket_sz / 2;
		self->d_head_use                       = 1;
		new_bucket->db_items[self->d_head_idx] = item;
	} else {
		ASSERT(self->d_tail->db_next == NULL);
		self->d_tail->db_next   = new_bucket;
		new_bucket->db_pself    = &self->d_tail->db_next;
		new_bucket->db_items[0] = item;
		self->d_tail_sz         = 1;
	}
	self->d_tail = new_bucket;
	Dee_Incref(item);
	++self->d_size;
	++self->d_version;
	return true;
}


INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_PopFront_unlocked(Deque *__restrict self) {
	DREF DeeObject *result;
	DequeBucket *head;
	ASSERT(self->d_size != 0);
	head   = self->d_head;
	result = head->db_items[self->d_head_idx]; /* Inherit reference. */
	--self->d_size;
	--self->d_head_use;
	if (!self->d_head_use) {
		/* Head-bucket has become empty. */
		self->d_head_idx = 0;
		self->d_head     = head->db_next;
		ASSERT((self->d_size == 0) == (self->d_tail == head));
		if (self->d_tail == head) {
			/* It was the last bucket that we just removed. */
			self->d_tail = NULL;
		} else if (self->d_head == self->d_tail) {
			/* Only one bucket remains. */
			/*self->d_head_idx = 0;*/
			ASSERT(self->d_size <= self->d_bucket_sz);
			self->d_head_use = self->d_tail_sz;
			self->d_tail_sz  = 0;
		} else {
			/* More that one bucket remains. */
			/*self->d_head_idx = 0;*/
			self->d_head_use = self->d_bucket_sz;
		}
		Dee_Free(head);
	} else {
		/* Advance the head-used index to indicate that we took an object. */
		++self->d_head_idx;
	}
	++self->d_version;
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_PopBack_unlocked(Deque *__restrict self) {
	DREF DeeObject *result;
	DequeBucket *tail;
	tail = self->d_tail;
	ASSERT(tail != NULL);
	if (tail == self->d_head) {
		/* Only one bucket is being used. */
		ASSERT(self->d_size <= self->d_bucket_sz);
		ASSERT(self->d_size == self->d_head_use);
		ASSERT(self->d_tail_sz == 0);
		--self->d_size;
		--self->d_head_use;
		result = tail->db_items[self->d_head_idx + self->d_head_use];
		if (!self->d_head_use) {
			/* The last bucket has gone away. */
			self->d_head_idx = 0;
			self->d_head     = NULL;
			self->d_tail     = NULL;
			Dee_Free(tail);
		}
	} else {
		/* More than one bucket is being used. */
		ASSERT(self->d_tail_sz != 0);
		--self->d_size;
		--self->d_tail_sz;
		result = tail->db_items[self->d_tail_sz];
		if (!self->d_tail_sz) {
			/* The tail-bucket needs to go away. */
			self->d_tail          = DEQUEBUCKET_PREV(tail);
			self->d_tail->db_next = NULL;
			if (self->d_tail == self->d_head) {
				ASSERT(self->d_size <= self->d_bucket_sz);
				/* Only one bucket will remain. */
				ASSERT(self->d_head_idx + self->d_head_use ==
				       self->d_bucket_sz);
				ASSERT(self->d_size == self->d_head_use);
				/*self->d_tail_sz = 0;*/
			} else {
				/* More than one bucket remains. */
				self->d_tail_sz = self->d_bucket_sz;
			}
			Dee_Free(tail);
		}
	}
	++self->d_version;
	return result;
}




/* Rotate the first `num_objects' left. */
INTERN NONNULL((1)) void DCALL
Deque_llrot_unlocked(Deque *__restrict self, size_t num_objects) {
	DequeIterator iter;
	DREF DeeObject *lobj;
	if unlikely(num_objects <= 1)
		return;
	ASSERT(num_objects <= self->d_size);
	DequeIterator_InitBegin(&iter, self);
	lobj = DequeIterator_ITEM(&iter);
	num_objects -= 2;
	while (num_objects--) {
		DequeIterator_ITEM(&iter) = DequeIterator_NEXTITEM(&iter, self);
		DequeIterator_Next(&iter, self);
	}
	DequeIterator_ITEM(&iter) = lobj;
	++self->d_version;
}

/* Rotate the first `num_objects' right. */
INTERN NONNULL((1)) void DCALL
Deque_lrrot_unlocked(Deque *__restrict self, size_t num_objects) {
	DequeIterator iter;
	DREF DeeObject *temp, *next;
	if unlikely(num_objects <= 1)
		return;
	ASSERT(num_objects <= self->d_size);
	DequeIterator_InitBegin(&iter, self);
	num_objects -= 2;
	temp = DequeIterator_ITEM(&iter);
	while (num_objects--) {
		DequeIterator_Next(&iter, self);
		next = DequeIterator_ITEM(&iter);
		DequeIterator_ITEM(&iter) = temp;
		temp = next;
	}
	DEQUE_HEAD(self) = temp;
	++self->d_version;
}

/* Rotate the last `num_objects' left. */
INTERN NONNULL((1)) void DCALL
Deque_rlrot_unlocked(Deque *__restrict self, size_t num_objects) {
	DequeIterator iter;
	DREF DeeObject *temp, *next;
	if unlikely(num_objects <= 1)
		return;
	ASSERT(num_objects <= self->d_size);
	DequeIterator_InitRBegin(&iter, self);
	num_objects -= 2;
	temp = DequeIterator_ITEM(&iter);
	while (num_objects--) {
		DequeIterator_Prev(&iter, self);
		next                      = DequeIterator_ITEM(&iter);
		DequeIterator_ITEM(&iter) = temp;
		temp                      = next;
	}
	DEQUE_TAIL(self) = temp;
	++self->d_version;
}

/* Rotate the last `num_objects' right. */
INTERN NONNULL((1)) void DCALL
Deque_rrrot_unlocked(Deque *__restrict self, size_t num_objects) {
	DequeIterator iter;
	DREF DeeObject *robj;
	if unlikely(num_objects <= 1)
		return;
	ASSERT(num_objects <= self->d_size);
	DequeIterator_InitRBegin(&iter, self);
	num_objects -= 2;
	robj = DequeIterator_ITEM(&iter);
	while (num_objects--) {
		DequeIterator_Prev(&iter, self);
		DequeIterator_NEXTITEM(&iter, self) = DequeIterator_ITEM(&iter);
	}
	DequeIterator_ITEM(&iter) = robj;
	++self->d_version;
}



INTERN WUNUSED NONNULL((1, 3)) bool DCALL
Deque_Insert_unlocked(Deque *self, size_t index, DeeObject *item) {
	ASSERT(index <= self->d_size);
	/* Optimize for special cases: front/back */
	if (index == 0)
		return Deque_PushFront_unlocked(self, item);
	if (index == self->d_size - 1)
		return Deque_PushBack_unlocked(self, item);

	if (index < self->d_size / 2) {
		/* Insert from the left */
		if (!Deque_PushFront_unlocked(self, item))
			return false;
		Deque_llrot_unlocked(self, index + 1);
	} else {
		/* Insert from the right */
		if (!Deque_PushBack_unlocked(self, item))
			return false;
		Deque_rrrot_unlocked(self, self->d_size - index);
	}
	return true;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_Pop_unlocked(Deque *__restrict self, size_t index) {
	ASSERT(index < self->d_size);
	/* Optimize for special cases: front/back */
	if (index == 0)
		return Deque_PopFront_unlocked(self);
	if (index == self->d_size - 1)
		return Deque_PopBack_unlocked(self);
	if (index < self->d_size / 2) {
		/* Pop from the left */
		Deque_llrot_unlocked(self, index + 1);
		return Deque_PopFront_unlocked(self);
	} else {
		/* Pop from the right */
		Deque_rrrot_unlocked(self, self->d_size - index);
		return Deque_PopBack_unlocked(self);
	}
}




INTERN WUNUSED NONNULL((1, 2)) int DCALL
Deque_PushFront(Deque *self, DeeObject *item) {
again:
	Deque_LockWrite(self);
	if (!Deque_PushFront_unlocked(self, item)) {
		size_t bucket_size;
		bucket_size = self->d_bucket_sz;
		Deque_LockEndWrite(self);
		if (!Dee_CollectMemory(SIZEOF_BUCKET(bucket_size)))
			goto err;
		goto again;
	}
	Deque_LockEndWrite(self);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Deque_PushBack(Deque *self, DeeObject *item) {
again:
	Deque_LockWrite(self);
	if (!Deque_PushBack_unlocked(self, item)) {
		size_t bucket_size;
		bucket_size = self->d_bucket_sz;
		Deque_LockEndWrite(self);
		if (!Dee_CollectMemory(SIZEOF_BUCKET(bucket_size)))
			goto err;
		goto again;
	}
	Deque_LockEndWrite(self);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_PopFront(Deque *__restrict self) {
	DREF DeeObject *result;
	Deque_LockWrite(self);
	if unlikely(!self->d_size) {
		Deque_LockEndWrite(self);
		err_empty_sequence((DeeObject *)self);
		return NULL;
	}
	result = Deque_PopFront_unlocked(self);
	Deque_LockEndWrite(self);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_PopBack(Deque *__restrict self) {
	DREF DeeObject *result;
	Deque_LockWrite(self);
	if unlikely(!self->d_size) {
		Deque_LockEndWrite(self);
		err_empty_sequence((DeeObject *)self);
		return NULL;
	}
	result = Deque_PopBack_unlocked(self);
	Deque_LockEndWrite(self);
	return result;
}

/* Insert/delete items at a given index. */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
Deque_Insert(Deque *self, size_t index, DeeObject *item) {
again:
	Deque_LockWrite(self);
	if (index > self->d_size)
		index = self->d_size;
	if (!Deque_Insert_unlocked(self, (size_t)index, item)) {
		size_t bucket_size;
		bucket_size = self->d_bucket_sz;
		Deque_LockEndWrite(self);
		if (!Dee_CollectMemory(SIZEOF_BUCKET(bucket_size)))
			goto err;
		goto again;
	}
	Deque_LockEndWrite(self);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Deque_Erase(Deque *__restrict self, size_t index, size_t count) {
	size_t result;
	size_t i;
	DREF DeeObject **pop_objv;
again:
	Deque_LockWrite(self);
	if (index > self->d_size)
		index = self->d_size;
	result = self->d_size - (size_t)index;
	if (result > count)
		result = count;
	pop_objv = (DREF DeeObject **)Dee_TryMallocac(result, sizeof(DREF DeeObject *));
	if unlikely(!pop_objv) {
		Deque_LockEndWrite(self);
		if (Dee_CollectMemoryc(result, sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	if ((size_t)index < (self->d_size - (size_t)index + result)) {
		/* Pop from the left. */
		for (i = 0; i < result; ++i)
			pop_objv[i] = Deque_Pop_unlocked(self, (size_t)index);
	} else {
		/* Pop from the right. */
		for (i = 0; i < result; ++i)
			pop_objv[(result - 1) - i] = Deque_Pop_unlocked(self, (size_t)index + i - 1);
	}
	Deque_LockEndWrite(self);
	/* Drop references to popped objects. */
	Dee_Decrefv(pop_objv, result);
	Dee_Freea(pop_objv);
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_Pop(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	Deque_LockWrite(self);
	if unlikely(index >= self->d_size) {
		index = self->d_size;
		if (!index) {
			Deque_LockEndWrite(self);
			err_empty_sequence((DeeObject *)self);
			return NULL;
		}
		--index;
	}
	result = Deque_Pop_unlocked(self, (size_t)index);
	Deque_LockEndWrite(self);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_Pops(Deque *__restrict self, Dee_ssize_t index) {
	DREF DeeObject *result;
	Deque_LockWrite(self);
	if unlikely(index < 0)
		index += self->d_size;
	if unlikely((size_t)index >= self->d_size) {
		index = self->d_size;
		if (!index) {
			Deque_LockEndWrite(self);
			err_empty_sequence((DeeObject *)self);
			return NULL;
		}
	}
	result = Deque_Pop_unlocked(self, (size_t)index);
	Deque_LockEndWrite(self);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Deque_llrot(Deque *__restrict self, size_t num_objects) {
	Deque_LockWrite(self);
	if unlikely(num_objects > self->d_size) {
		size_t my_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndWrite(self);
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)self, num_objects, my_size);
	}
	Deque_llrot_unlocked(self, num_objects);
	Deque_LockEndWrite(self);
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Deque_lrrot(Deque *__restrict self, size_t num_objects) {
	Deque_LockWrite(self);
	if unlikely(num_objects > self->d_size) {
		size_t my_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndWrite(self);
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)self, num_objects, my_size);
	}
	Deque_lrrot_unlocked(self, num_objects);
	Deque_LockEndWrite(self);
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Deque_rlrot(Deque *__restrict self, size_t num_objects) {
	Deque_LockWrite(self);
	if unlikely(num_objects > self->d_size) {
		size_t my_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndWrite(self);
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)self, num_objects, my_size);
	}
	Deque_rlrot_unlocked(self, num_objects);
	Deque_LockEndWrite(self);
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Deque_rrrot(Deque *__restrict self, size_t num_objects) {
	Deque_LockWrite(self);
	if unlikely(num_objects > self->d_size) {
		size_t my_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndWrite(self);
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)self, num_objects, my_size);
	}
	Deque_rrrot_unlocked(self, num_objects);
	Deque_LockEndWrite(self);
	return 0;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_ctor(Deque *__restrict self) {
	Dee_atomic_rwlock_init(&self->d_lock);
	self->d_head      = NULL;
	self->d_tail      = NULL;
	self->d_size      = 0;
	self->d_head_idx  = 0;
	self->d_head_use  = 0;
	self->d_tail_sz   = 0;
	self->d_bucket_sz = DEQUE_BUCKET_DEFAULT_SIZE;
	self->d_version   = 0;
	weakref_support_init(self);
	return 0;
}

LOCAL WUNUSED NONNULL((1)) DequeBucket *DCALL
copy_bucket(DequeBucket *__restrict self,
            size_t bucket_size,
            size_t start, size_t used_size) {
	DequeBucket *result;
	ASSERT(start + used_size >= start);
	ASSERT(start + used_size <= bucket_size);
	result = TRY_NEW_BUCKET(bucket_size);
	if unlikely(!result)
		goto done;
	Dee_Movrefv(result->db_items + start,
	            self->db_items + start,
	            used_size);
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deq_copy(Deque *__restrict self,
         Deque *__restrict other) {
	DequeBucket *iter;
	DequeBucket *copy;
	DequeBucket *copy_tail;
	DequeBucket *next;
	Dee_atomic_rwlock_init(&self->d_lock);
	self->d_version = 0;
again:
	Deque_LockRead(other);
	self->d_bucket_sz = other->d_bucket_sz;
	self->d_size      = other->d_size;
	self->d_head_idx  = other->d_head_idx;
	self->d_head_use  = other->d_head_use;
	self->d_tail_sz   = other->d_tail_sz;
	if (other->d_head) {
		iter = other->d_head;
		copy = copy_tail = copy_bucket(iter,
		                               other->d_bucket_sz,
		                               other->d_head_idx,
		                               other->d_head_use);
		if unlikely(!copy)
			goto err_restart_collect;
		copy->db_pself = &self->d_head;
		/* Copy all the inner buckets of the other deque. */
		while (iter != other->d_tail) {
			size_t copy_count;
			iter       = iter->db_next;
			copy_count = other->d_bucket_sz;
			if (iter == other->d_tail) /* Special case for the last bucket. */
				copy_count = other->d_tail_sz;
			next = copy_bucket(iter, other->d_bucket_sz, 0, copy_count);
			if unlikely(!next)
				goto err_restart_collect;
			copy_tail->db_next = next;
			next->db_pself     = &copy_tail->db_next;
			copy_tail          = next;
		}
		self->d_head       = copy;
		self->d_tail       = copy_tail;
		copy_tail->db_next = NULL;
	} else {
		self->d_head = NULL;
		self->d_tail = NULL;
	}
	Deque_LockEndRead(other);
	weakref_support_init(self);
	return 0;
err_restart_collect:
	COMPILER_READ_BARRIER();
	Deque_LockEndRead(other);
	iter = copy;
	/* Free memory for all buckets already allocated. */
	if (iter) {
		for (;;) {
			size_t i, count;
			next = iter->db_next;
			if (iter == copy) {
				i     = self->d_head_idx;
				count = self->d_head_use;
			} else {
				i     = 0;
				count = self->d_bucket_sz;
			}
			Dee_Decrefv(iter->db_items + i, count);
			Dee_Free(iter);
			if (iter == copy_tail)
				break;
			iter = next;
		}
	}
	/* Collect memory for the missing bucket. */
	if (!Dee_CollectMemory(SIZEOF_BUCKET(self->d_bucket_sz)))
		goto err;
	goto again;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_deepload(Deque *__restrict self) {
	Deque_LockRead(self);
	if (self->d_size) {
		DequeIterator iter;
		size_t version;
		version = self->d_version;
		DequeIterator_InitBegin(&iter, self);
		for (;;) {
			DREF DeeObject *orig_ob, *copy;
			orig_ob = DequeIterator_ITEM(&iter);
			Dee_Incref(orig_ob);
			Deque_LockEndRead(self);
			/* Create a deep copy of this item. */
			copy = DeeObject_DeepCopy(orig_ob);
			Dee_Decref(orig_ob);
			if unlikely(!copy)
				goto err;
			Deque_LockWrite(self);
			/* Check if the deque changed in the mean time. */
			if (version != self->d_version) {
				Deque_LockEndWrite(self);
				Dee_Decref(copy);
				return 0;
			}
			orig_ob = DequeIterator_ITEM(&iter); /* Inherit reference. */
			DequeIterator_ITEM(&iter) = copy;    /* Inherit reference. */
			Deque_LockDowngrade(self);
			/* Drop a reference from the exchanged object. */
			if (!Dee_DecrefIfNotOne(orig_ob)) {
				Deque_LockEndRead(self);
				Dee_Decref(orig_ob);
				Deque_LockRead(self);
				if (version != self->d_version)
					break;
			}
			if (!DequeIterator_HasNext(&iter, self))
				break;
			DequeIterator_Next(&iter, self);
		}
	}
	Deque_LockEndRead(self);
	return 0;
err:
	return -1;
}



PRIVATE NONNULL((1)) void DCALL
deq_fini(Deque *__restrict self) {
	DequeBucket *iter, *next;
	weakref_support_fini(self);
	if (!self->d_size)
		return;
	iter = self->d_head;
	Dee_Decrefv(iter->db_items + self->d_head_idx, self->d_head_use);
	if (iter != self->d_tail) {
		for (;;) {
			next = iter->db_next;
			Dee_Free(iter);
			iter = next;
			if (iter == self->d_tail)
				break;
			Dee_Decrefv(iter->db_items, self->d_bucket_sz);
		}
		Dee_Decrefv(iter->db_items, self->d_tail_sz);
	}
	Dee_Free(iter);
}


#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
PRIVATE NONNULL((1, 2)) Dee_ssize_t DCALL
deque_push_back(void *self, DeeObject *__restrict ob) {
	return (Dee_ssize_t)Deque_PushBack((Deque *)self, ob);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */


PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_init(Deque *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *init   = Dee_EmptySeq;
	self->d_bucket_sz = DEQUE_BUCKET_DEFAULT_SIZE;
	if (DeeArg_Unpack(argc, argv, "|o" UNPuSIZ ":Deque", &init, &self->d_bucket_sz))
		goto err;
	if unlikely(!self->d_bucket_sz) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid bucket size: 0");
		goto err;
	}
	Dee_atomic_rwlock_init(&self->d_lock);
	self->d_head     = NULL;
	self->d_tail     = NULL;
	self->d_size     = 0;
	self->d_head_idx = 0;
	self->d_head_use = 0;
	self->d_tail_sz  = 0;
	self->d_version  = 0;
	weakref_support_init(self);
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
	if (DeeObject_Foreach(init, (Dee_foreach_t)&Deque_PushBack, self) < 0)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
	if (DeeObject_Foreach(init, &deque_push_back, self) < 0)
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
	{
		deq_fini(self);
		goto err;
	}
	return 0;
err:
	return -1;
}


#define DEQUE_BUFFER_STARTFIELD d_head
#define DEQUE_BUFFER_ENDFIELD   d_version
#define DEQUE_BUFFER_OF(self)   (&(self)->DEQUE_BUFFER_STARTFIELD)
#define DEQUE_BUFFER_SIZE       (offsetof(Deque, DEQUE_BUFFER_ENDFIELD) - offsetof(Deque, DEQUE_BUFFER_STARTFIELD))


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deq_assign(Deque *self, DeeObject *seq) {
	Deque bTemp, bOld;
	weakref_support_init(&bTemp);
	if (DeeObject_InstanceOfExact(seq, &Deque_Type)) {
		if (deq_copy(&bTemp, (Deque *)seq))
			goto err;
	} else {
		bTemp.d_head      = NULL;
		bTemp.d_tail      = NULL;
		bTemp.d_size      = 0;
		bTemp.d_head_idx  = 0;
		bTemp.d_head_use  = 0;
		bTemp.d_tail_sz   = 0;
		bTemp.d_bucket_sz = DEQUE_BUCKET_DEFAULT_SIZE;
		Dee_atomic_rwlock_init(&bTemp.d_lock);
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
		if (DeeObject_Foreach(seq, (Dee_foreach_t)&Deque_PushBack, &bTemp) < 0)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
		if (DeeObject_Foreach(seq, &deque_push_back, &bTemp) < 0)
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
		{
			deq_fini(&bTemp);
			goto err;
		}
	}
	/* Save the new deque into the current. */
	Deque_LockWrite(self);
	memcpy(DEQUE_BUFFER_OF(&bOld), DEQUE_BUFFER_OF(self), DEQUE_BUFFER_SIZE);
	memcpy(DEQUE_BUFFER_OF(self), DEQUE_BUFFER_OF(&bTemp), DEQUE_BUFFER_SIZE);
	++self->d_version;
	Deque_LockEndWrite(self);
	weakref_support_init(&bOld);

	/* Free the old deque state. */
	deq_fini(&bOld);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deq_moveassign(Deque *__restrict self,
               Deque *__restrict other) {
	Deque bNew, bOld;
	if (self == other)
		return 0;
	Deque_LockWrite(other);
	memcpy(DEQUE_BUFFER_OF(&bNew), DEQUE_BUFFER_OF(other), DEQUE_BUFFER_SIZE);
	/* Clear out the state of the other deque, thus stealing all of its references. */
	bzero(DEQUE_BUFFER_OF(other), DEQUE_BUFFER_SIZE);
	++other->d_version;
	Deque_LockEndWrite(other);
	/* Override the state of our own deque. */
	Deque_LockWrite(self);
	memcpy(DEQUE_BUFFER_OF(&bOld), DEQUE_BUFFER_OF(self), DEQUE_BUFFER_SIZE);
	memcpy(DEQUE_BUFFER_OF(self), DEQUE_BUFFER_OF(&bNew), DEQUE_BUFFER_SIZE);
	++self->d_version;
	Deque_LockEndWrite(self);
	weakref_support_init(&bOld);
	/* Free the old state of our own deque. */
	deq_fini(&bOld);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_bool(Deque *__restrict self) {
	return atomic_read(&self->d_size) != 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
deq_visit(Deque *__restrict self, dvisit_t proc, void *arg) {
	DequeBucket *iter;
	if (!self->d_size)
		return;
	Deque_LockRead(self);
	iter = self->d_head;
	Dee_Visitv(iter->db_items + self->d_head_idx, self->d_head_use);
	if (iter != self->d_tail) {
		while ((iter = iter->db_next) != self->d_tail) {
			Dee_Visitv(iter->db_items, self->d_bucket_sz);
		}
		Dee_Visitv(iter->db_items, self->d_tail_sz);
	}
	Deque_LockEndRead(self);
}


PRIVATE NONNULL((1)) void DCALL
deq_clear(Deque *__restrict self) {
	Deque bData;
	Deque_LockWrite(self);
	memcpy(DEQUE_BUFFER_OF(&bData), DEQUE_BUFFER_OF(self), DEQUE_BUFFER_SIZE);
	/* Clear out the state of the other deque,
	 * thus stealing all of its references. */
	bzero(DEQUE_BUFFER_OF(self), DEQUE_BUFFER_SIZE);
	++self->d_version;
	Deque_LockEndWrite(self);
	weakref_support_init(&bData);
	/* Destroy the cloned buffer. */
	deq_fini(&bData);
}

PRIVATE NONNULL((1)) void DCALL
deq_pclear(Deque *__restrict self, unsigned int gc_priority) {
	Deque_LockWrite(self);
	if (self->d_size) {
		DequeIterator iter;
		size_t version;
		version = self->d_version;
		DequeIterator_InitBegin(&iter, self);
		for (;;) {
			DREF DeeObject *orig_ob;
			orig_ob = DequeIterator_ITEM(&iter);
			if (DeeObject_GCPriority(orig_ob) >= gc_priority) {
				/* Inherit reference to `orig_ob' */
				DequeIterator_ITEM(&iter) = DeeNone_NewRef();
				/* Drop a reference from the cleared object. */
				if (!Dee_DecrefIfNotOne(orig_ob)) {
					Deque_LockEndWrite(self);
					Dee_Decref(orig_ob);
					Deque_LockWrite(self);
					if (version != self->d_version)
						break;
				}
			}
			if (!DequeIterator_HasNext(&iter, self))
				break;
			DequeIterator_Next(&iter, self);
		}
	}
	Deque_LockEndWrite(self);
}


PRIVATE struct type_gc tpconst deq_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&deq_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&deq_pclear
};


PRIVATE WUNUSED NONNULL((1)) DREF DequeIteratorObject *DCALL
deq_iter(Deque *__restrict self) {
	DREF DequeIteratorObject *result;
	result = DeeObject_MALLOC(DequeIteratorObject);
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_init(&result->di_lock);
	result->di_ver = self->d_version;
	COMPILER_READ_BARRIER();
	DequeIterator_InitBegin(&result->di_iter, self);
	if (!self->d_size)
		--result->di_ver;
	result->di_deq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DequeIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
deq_contains(Deque *self, DeeObject *item) {
	Deque_LockRead(self);
	if (self->d_size) {
		DequeIterator iter;
		size_t version;
		version = self->d_version;
		DequeIterator_InitBegin(&iter, self);
		for (;;) {
			DREF DeeObject *elem;
			int temp;
			elem = DequeIterator_ITEM(&iter);
			Dee_Incref(elem);
			Deque_LockEndRead(self);
			temp = DeeObject_TryCompareEq(item, elem);
			Dee_Decref(elem);
			if unlikely(temp == Dee_COMPARE_ERR)
				goto err;
			if (temp == 0)
				return_true; /* Found it! */
			Deque_LockRead(self);
			if (self->d_version != version)
				break;
			if (!DequeIterator_HasNext(&iter, self))
				break;
			DequeIterator_Next(&iter, self);
		}
	}
	Deque_LockEndRead(self);
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
deq_size(Deque *__restrict self) {
	return atomic_read(&self->d_size);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_getitem_index(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, deq_size);
		goto err;
	}
	result = DEQUE_ITEM(self, index);
	Dee_Incref(result);
	Deque_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_trygetitem_index(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	Deque_LockRead(self);
	if unlikely(index >= self->d_size) {
		Deque_LockEndRead(self);
		return ITER_DONE;
	}
	result = DEQUE_ITEM(self, index);
	Dee_Incref(result);
	Deque_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_getitem_index_fast(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	Deque_LockRead(self);
	if unlikely(index >= self->d_size) {
		Deque_LockEndRead(self);
		return NULL; /* >> The sequence is resizable and `index >= CURRENT_SIZE' */
	}
	result = DEQUE_ITEM(self, index);
	Dee_Incref(result);
	Deque_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_delitem_index(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = Deque_Pop(self, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
deq_setitem_index(Deque *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldval;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, deq_size);
		goto err;
	}
	/* Exchange the stored item. */
	Dee_Incref(value);
	oldval                  = DEQUE_ITEM(self, index);
	DEQUE_ITEM(self, index) = value;
	Deque_LockEndRead(self);
	Dee_Decref(oldval);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
deq_foreach(Deque *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	Deque_LockRead(self);
	for (i = 0; i < self->d_size; ++i) {
		DREF DeeObject *elem;
		elem = DEQUE_ITEM(self, i);
		Dee_Incref(elem);
		Deque_LockEndRead(self);
		temp = (*proc)(arg, elem);
		Dee_Decref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		Deque_LockRead(self);
	}
	Deque_LockEndRead(self);
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_seq deq_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&deq_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&deq_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&deq_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&deq_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&deq_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&deq_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&deq_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&deq_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&deq_setitem_index,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* XXX: range operators? */
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))NULL,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))NULL,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&deq_trygetitem_index,
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

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
deq_xchitem_index(Deque *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldval;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, deq_size);
		goto err;
	}
	/* Exchange the stored item. */
	Dee_Incref(value);
	oldval = DEQUE_ITEM(self, index);
	DEQUE_ITEM(self, index) = value;
	Deque_LockEndRead(self);
	return oldval;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_popfront(Deque *self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "popfront");
	return Deque_PopFront(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_popback(Deque *self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "popback");
	return Deque_PopBack(self);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_llrot(Deque *self, size_t argc, DeeObject *const *argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":llrot", &num_items))
		goto err;
	if (Deque_llrot(self, num_items))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_lrrot(Deque *self, size_t argc, DeeObject *const *argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":lrrot", &num_items))
		goto err;
	if (Deque_lrrot(self, num_items))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_rlrot(Deque *self, size_t argc, DeeObject *const *argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":rlrot", &num_items))
		goto err;
	if (Deque_rlrot(self, num_items))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_rrrot(Deque *self, size_t argc, DeeObject *const *argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":rrrot", &num_items))
		goto err;
	if (Deque_rrrot(self, num_items))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_sizeof(Deque *self) {
	size_t result = sizeof(Deque);
	Deque_LockRead(self);
	ASSERT((self->d_head != NULL) == (self->d_tail != NULL));
	if (self->d_head) {
		size_t count = 1;
		DequeBucket *iter = self->d_head;
		for (; iter != self->d_tail; iter = iter->db_next)
			++count;
		result += count * SIZEOF_BUCKET(self->d_bucket_sz);
	}
	Deque_LockEndRead(self);
	return DeeInt_NewSize(result);
}


PRIVATE struct type_method tpconst deq_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(Sequence_insert),
	TYPE_METHOD_HINTREF(Sequence_erase),
	TYPE_METHOD_HINTREF(Sequence_pop),
	TYPE_METHOD_HINTREF(Sequence_pushfront),
	TYPE_METHOD_HINTREF(Sequence_append),
	TYPE_METHOD_F("popfront", &deq_popfront, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#tValueError{@this deque is empty}"
	              "Pop and return an item from the front of @this deque"),
	TYPE_METHOD_F("popback", &deq_popback, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#tValueError{@this deque is empty}"
	              "Pop and return an item from the back of @this deque"),
	TYPE_METHOD_F("llrot", &deq_llrot, METHOD_FNOREFESCAPE,
	              "(num_items:?Dint)\n"
	              "#tIndexError{@this deque contain less than @num_items items}"
	              "Rotate the first @num_items items left by 1:\n"
	              "${"
	              /**/ "import deque from collections;\n"
	              /**/ "local x = deque({ 10, 20, 30, 40, 50 });\n"
	              /**/ "x.llrot(3);\n"
	              /**/ "print repr x; /* { 20, 30, 10, 40, 50 } */"
	              "}"),
	TYPE_METHOD_F("lrrot", &deq_lrrot, METHOD_FNOREFESCAPE,
	              "(num_items:?Dint)\n"
	              "#tIndexError{@this deque contain less than @num_items items}"
	              "Rotate the first @num_items items right by 1:\n"
	              "${"
	              /**/ "import deque from collections;\n"
	              /**/ "local x = deque({ 10, 20, 30, 40, 50 });\n"
	              /**/ "x.lrrot(3);\n"
	              /**/ "print repr x; /* { 30, 10, 20, 40, 50 } */"
	              "}"),
	TYPE_METHOD_F("rlrot", &deq_rlrot, METHOD_FNOREFESCAPE,
	              "(num_items:?Dint)\n"
	              "#tIndexError{@this deque contain less than @num_items items}"
	              "Rotate the last @num_items items left by 1:\n"
	              "${"
	              /**/ "import deque from collections;\n"
	              /**/ "local x = deque({ 10, 20, 30, 40, 50 });\n"
	              /**/ "x.rlrot(3);\n"
	              /**/ "print repr x; /* { 10, 20, 40, 50, 30 } */"
	              "}"),
	TYPE_METHOD_F("rrrot", &deq_rrrot, METHOD_FNOREFESCAPE,
	              "(num_items:?Dint)\n"
	              "#tIndexError{@this deque contain less than @num_items items}"
	              "Rotate the last @num_items items right by 1:\n"
	              "${"
	              /**/ "import deque from collections;\n"
	              /**/ "local x = deque({ 10, 20, 30, 40, 50 });\n"
	              /**/ "x.rrrot(3);\n"
	              /**/ "print repr x; /* { 10, 20, 50, 30, 40 } */"
	              "}"),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst deq_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_xchitem_index, &deq_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insert, &Deque_Insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_erase, &Deque_Erase, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pop, &Deque_Pops, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pushfront, &Deque_PushFront, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_append, &Deque_PushBack, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst deq_getsets[] = {
	TYPE_GETTER_AB_F("__sizeof__", &deq_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst deq_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DequeIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject Deque_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Deque",
	/* .tp_doc      = */ DOC("A double-ended queue that allows for O(1) insertion and removal at the front "
	                         /**/ "and back, while still providing a decently fast index-based item lookup, "
	                         /**/ "as well as fast iteration both in forward, as well as in reverse\n"
	                         "A Deque in its purest form (with a ?#bucketsize of $1) is really nothing "
	                         /**/ "but a doubly-linked list. However instead of every node only ever containing "
	                         /**/ "a single item, each is a pre-allocated vector of up to ?#bucketsize objects, "
	                         /**/ "meaning that an index-based lookup takes at most O(floor((n / bucketsize) / 2).\n"
	                         "The second division by 2 is done because indices closer to the end of the Deque "
	                         /**/ "are searched for from its end, rather than its start\n"
	                         "However, inserting or erasure anywhere else is quite expensive, "
	                         /**/ "as doing so requires items to be moved around a lot\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty Deque\n"
	                         "\n"

	                         "(seq:?S?O)\n"
	                         "(seq:?S?O,bucketsize:?Dint)\n"
	                         "Construct a Deque pre-initialized with items from "
	                         /**/ "@seq, and set the Deque's bucket size to @bucketsize\n"
	                         "When @bucketsize is omitted, an implementation-specific "
	                         /**/ "default value is used, which may also depend on other "
	                         /**/ "environmental factors selected to maximize performance\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a shallow copy of all elements of @this Deque\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Returns a deep copy of all elements of @this Deque\n"
	                         "\n"

	                         ":=(other:?S?O)->\n"
	                         "Assign all the elements from @other to @this Deque\n"
	                         "\n"

	                         "move:=->\n"
	                         "Move all the elements from @other into @this Deque, clearing @other in the process\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this Deque is non-empty. ?f otherwise\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this Deque in ascending order\n"
	                         "\n"

	                         "[]->\n"
	                         "#tIndexError{@index is greater that the length of @this Deque}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Returns the @index'th item of @this Deque\n"
	                         "\n"

	                         "[]=->\n"
	                         "#tIndexError{@index is greater that the length of @this Deque}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Set the @index'th item of @this Deque to @item\n"
	                         "\n"

	                         "del[]->\n"
	                         "#tIndexError{@index is greater that the length of @this Deque}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "Same as ${this.pop(index)} for positive values for @index\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @item is apart of this Deque, ?f otherwise"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(Deque),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&deq_ctor, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (Dee_funptr_t)&deq_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&deq_copy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&deq_init,
				TYPE_FIXED_ALLOCATOR_GC(Deque)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&deq_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&deq_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&deq_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&deq_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&deq_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&deq_visit,
	/* .tp_gc            = */ &deq_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &deq_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ deq_methods,
	/* .tp_getsets       = */ deq_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ deq_class_members,
	/* .tp_method_hints  = */ deq_method_hints,
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
deqiter_ctor(DequeIteratorObject *__restrict self) {
	Dee_atomic_rwlock_init(&self->di_lock);
	self->di_deq = (DREF Deque *)DeeObject_NewDefault(&Deque_Type);
	if unlikely(!self->di_deq)
		goto err;
	self->di_ver = 0;
	DequeIterator_InitBegin(&self->di_iter, self->di_deq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deqiter_copy(DequeIteratorObject *__restrict self,
             DequeIteratorObject *__restrict other) {
	Dee_atomic_rwlock_init(&self->di_lock);
	self->di_deq = other->di_deq;
	Dee_Incref(self->di_deq);
	DequeIterator_LockRead(other);
	self->di_ver  = other->di_ver;
	self->di_iter = other->di_iter;
	DequeIterator_LockEndRead(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
deqiter_getindex(DequeIteratorObject *__restrict self) {
	size_t result;
	DequeIterator_LockRead(self);
	Deque_LockRead(self->di_deq);
	result = unlikely(self->di_ver != self->di_deq->d_version)
	         ? (size_t)-1
	         : DequeIterator_GetIndex(&self->di_iter, self->di_deq);
	Deque_LockEndRead(self->di_deq);
	DequeIterator_LockEndRead(self);
	return result;
}

PRIVATE NONNULL((1)) void DCALL
deqiter_setindex(DequeIteratorObject *__restrict self, size_t index) {
	DequeIterator_LockWrite(self);
	Deque_LockRead(self->di_deq);
	self->di_ver = self->di_deq->d_version;
	if unlikely(index >= self->di_deq->d_size) {
		--self->di_ver;
	} else {
		DequeIterator_InitAt(&self->di_iter,
		                     self->di_deq,
		                     index);
	}
	Deque_LockEndRead(self->di_deq);
	DequeIterator_LockEndWrite(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deqiter_deep(DequeIteratorObject *__restrict self,
             DequeIteratorObject *__restrict other) {
	size_t index;
	self->di_deq = (DREF Deque *)DeeObject_DeepCopy((DeeObject *)other->di_deq);
	if unlikely(!self->di_deq)
		goto err;
	index = deqiter_getindex(other);
	Deque_LockRead(self->di_deq);
	self->di_ver = self->di_deq->d_version;
	if unlikely(index >= self->di_deq->d_size) {
		--self->di_ver;
	} else {
		DequeIterator_InitAt(&self->di_iter,
		                     self->di_deq,
		                     index);
	}
	Deque_LockEndRead(self->di_deq);
	Dee_atomic_rwlock_init(&self->di_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
deqiter_init(DequeIteratorObject *__restrict self,
             size_t argc, DeeObject *const *argv) {
	size_t index = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":DequeIterator", &self->di_deq, &index))
		goto err;
	if (DeeObject_AssertType(self->di_deq, &Deque_Type))
		goto err;
	Dee_atomic_rwlock_init(&self->di_lock);
	Dee_Incref(self->di_deq);
	Deque_LockRead(self->di_deq);
	self->di_ver = self->di_deq->d_version;
	COMPILER_READ_BARRIER();
	if unlikely(index >= self->di_deq->d_size) {
		--self->di_ver;
	} else {
		DequeIterator_InitAt(&self->di_iter, self->di_deq, index);
	}
	Deque_LockEndRead(self->di_deq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
deqiter_fini(DequeIteratorObject *__restrict self) {
	Dee_Decref(self->di_deq);
}

PRIVATE NONNULL((1, 2)) void DCALL
deqiter_visit(DequeIteratorObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->di_deq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
deqiter_bool(DequeIteratorObject *__restrict self) {
	int result;
	DequeIterator_LockRead(self);
	result = (self->di_ver == self->di_deq->d_version);
	DequeIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deqiter_next(DequeIteratorObject *__restrict self) {
	DREF DeeObject *result;
	DequeIterator_LockWrite(self);
	Deque_LockRead(self->di_deq);
	if (self->di_ver != self->di_deq->d_version) {
		result = ITER_DONE;
	} else {
		result = DequeIterator_ITEM(&self->di_iter);
		Dee_Incref(result);
		if (DequeIterator_HasNext(&self->di_iter, self->di_deq)) {
			DequeIterator_Next(&self->di_iter, self->di_deq);
		} else {
			--self->di_ver; /* Invalidate the version number. */
		}
	}
	Deque_LockEndRead(self->di_deq);
	DequeIterator_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deqiter_getindex_ob(DequeIteratorObject *__restrict self) {
	size_t result;
	result = deqiter_getindex(self);
	if (result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deqiter_setindex_ob(DequeIteratorObject *self, DeeObject *value) {
	size_t newindex = (size_t)-1;
	if (!DeeNone_Check(value) &&
	    DeeObject_AsSize(value, &newindex))
		goto err;
	deqiter_setindex(self, newindex);
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset tpconst deqiter_getsets[] = {
	TYPE_GETSET_F("index", &deqiter_getindex_ob, NULL, &deqiter_setindex_ob, METHOD_FNOREFESCAPE,
	              "->?X2?Dint?N\n"
	              "Get/set the index of @this iterator within its associated ?GDeque\n"
	              "When ?N, the iterator has been invalidated, possibly due to the "
	              /**/ "deque having changed"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst deqiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(DequeIteratorObject, di_deq), "->?GDeque"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DequeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "DequeIterator",
	/* .tp_doc      = */ DOC("(seq?:?GDeque)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&deqiter_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&deqiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&deqiter_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&deqiter_init,
				TYPE_FIXED_ALLOCATOR(DequeIteratorObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&deqiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&deqiter_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&deqiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&deqiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ deqiter_getsets,
	/* .tp_members       = */ deqiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END


#endif /* !GUARD_DEX_COLLECTIONS_DEQUE_C */
