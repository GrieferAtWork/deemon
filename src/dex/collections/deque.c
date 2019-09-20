/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEX_COLLECTIONS_DEQUE_C
#define GUARD_DEX_COLLECTIONS_DEQUE_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>

#include <hybrid/atomic.h>
#include <hybrid/typecore.h>

DECL_BEGIN


INTERN WUNUSED NONNULL((1, 2)) bool DCALL
Deque_PushFront_unlocked(Deque *__restrict self,
                         DeeObject *__restrict item) {
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
Deque_PushBack_unlocked(Deque *__restrict self,
                        DeeObject *__restrict item) {
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


INTERN ATTR_RETNONNULL DREF DeeObject *DCALL
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

INTERN ATTR_RETNONNULL DREF DeeObject *DCALL
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
		next                      = DequeIterator_ITEM(&iter);
		DequeIterator_ITEM(&iter) = temp;
		temp                      = next;
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



INTERN bool DCALL
Deque_Insert_unlocked(Deque *__restrict self, size_t index,
                      DeeObject *__restrict item) {
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

INTERN ATTR_RETNONNULL DREF DeeObject *DCALL
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
Deque_PushFront(Deque *__restrict self, DeeObject *__restrict item) {
again:
	Deque_LockWrite(self);
	if (!Deque_PushFront_unlocked(self, item)) {
		size_t bucket_size;
		bucket_size = self->d_bucket_sz;
		Deque_LockEndWrite(self);
		if (!Dee_CollectMemory(SIZEOF_BUCKET(bucket_size)))
			return -1;
		goto again;
	}
	Deque_LockEndWrite(self);
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Deque_PushBack(Deque *__restrict self, DeeObject *__restrict item) {
again:
	Deque_LockWrite(self);
	if (!Deque_PushBack_unlocked(self, item)) {
		size_t bucket_size;
		bucket_size = self->d_bucket_sz;
		Deque_LockEndWrite(self);
		if (!Dee_CollectMemory(SIZEOF_BUCKET(bucket_size)))
			return -1;
		goto again;
	}
	Deque_LockEndWrite(self);
	return 0;
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
INTERN int DCALL
Deque_Insert(Deque *__restrict self,
             size_t index,
             DeeObject *__restrict item) {
again:
	Deque_LockWrite(self);
	if (index > self->d_size)
		index = self->d_size;
	if (!Deque_Insert_unlocked(self, (size_t)index, item)) {
		size_t bucket_size;
		bucket_size = self->d_bucket_sz;
		Deque_LockEndWrite(self);
		if (!Dee_CollectMemory(SIZEOF_BUCKET(bucket_size)))
			return -1;
		goto again;
	}
	Deque_LockEndWrite(self);
	return 0;
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
Deque_Erase(Deque *__restrict self,
            size_t index, size_t num_items) {
	size_t result;
	size_t i;
	DREF DeeObject **pop_objv;
again:
	Deque_LockWrite(self);
	if (index > self->d_size)
		index = self->d_size;
	result = self->d_size - (size_t)index;
	if (result > num_items)
		result = num_items;
	pop_objv = (DREF DeeObject **)Dee_ATryMalloc(result * sizeof(DREF DeeObject *));
	if unlikely(!pop_objv) {
		Deque_LockEndWrite(self);
		if (Dee_CollectMemory(result * sizeof(DREF DeeObject *)))
			goto again;
		return (size_t)-1;
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
	for (i = 0; i < result; ++i)
		Dee_Decref(pop_objv[i]);
	Dee_AFree(pop_objv);
	return (dssize_t)result;
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
	}
	result = Deque_Pop_unlocked(self, (size_t)index);
	Deque_LockEndWrite(self);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Deque_Pops(Deque *__restrict self, dssize_t index) {
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
		err_index_out_of_bounds((DeeObject *)self, num_objects, my_size);
		return -1;
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
		err_index_out_of_bounds((DeeObject *)self, num_objects, my_size);
		return -1;
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
		err_index_out_of_bounds((DeeObject *)self, num_objects, my_size);
		return -1;
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
		err_index_out_of_bounds((DeeObject *)self, num_objects, my_size);
		return -1;
	}
	Deque_rrrot_unlocked(self, num_objects);
	Deque_LockEndWrite(self);
	return 0;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_ctor(Deque *__restrict self) {
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
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

LOCAL DequeBucket *DCALL
copy_bucket(DequeBucket *__restrict self, size_t bucket_size,
            size_t start, size_t used_size) {
	DequeBucket *result;
	size_t i;
	ASSERT(start + used_size <= bucket_size);
	result = TRY_NEW_BUCKET(bucket_size);
	if unlikely(!result)
		goto done;
	for (i = start; i < start + used_size; ++i) {
		result->db_items[i] = self->db_items[i];
		Dee_Incref(result->db_items[i]);
	}
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
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
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
	if (iter)
		for (;;) {
			size_t i, count;
			next = iter->db_next;
			if (iter == copy)
				i     = self->d_head_idx,
				count = self->d_head_use;
			else {
				i = 0, count = self->d_bucket_sz;
			}
			while (count--) {
				Dee_Decref(iter->db_items[i]);
				++i;
			}
			Dee_Free(iter);
			if (iter == copy_tail)
				break;
			iter = next;
		}
	/* Collect memory for the missing bucket. */
	if (!Dee_CollectMemory(SIZEOF_BUCKET(self->d_bucket_sz)))
		return -1;
	goto again;
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
				return -1;
			Deque_LockWrite(self);
			/* Check if the deque changed in the mean time. */
			if (version != self->d_version) {
				Deque_LockEndWrite(self);
				Dee_Decref(copy);
				return 0;
			}
			orig_ob                   = DequeIterator_ITEM(&iter); /* Inherit reference. */
			DequeIterator_ITEM(&iter) = copy;                      /* Inherit reference. */
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
}



PRIVATE NONNULL((1)) void DCALL
deq_fini(Deque *__restrict self) {
	size_t i;
	DequeBucket *iter, *next;
	weakref_support_fini(self);
	if (!self->d_size)
		return;
	iter = self->d_head;
	for (i = 0; i < self->d_head_use; ++i)
		Dee_Decref(iter->db_items[i + self->d_head_idx]);
	if (iter != self->d_tail) {
		for (;;) {
			next = iter->db_next;
			Dee_Free(iter);
			iter = next;
			if (iter == self->d_tail)
				break;
			for (i = 0; i < self->d_bucket_sz; ++i)
				Dee_Decref(iter->db_items[i]);
		}
		for (i = 0; i < self->d_tail_sz; ++i)
			Dee_Decref(iter->db_items[i]);
	}
	Dee_Free(iter);
}


#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
PRIVATE dssize_t DCALL
deque_push_back(void *self, DeeObject *__restrict ob) {
	return (dssize_t)Deque_PushBack((Deque *)self, ob);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */


PRIVATE int DCALL
deq_init(Deque *__restrict self,
         size_t argc, DeeObject **argv) {
	DeeObject *init   = Dee_EmptySeq;
	self->d_bucket_sz = DEQUE_BUCKET_DEFAULT_SIZE;
	if (DeeArg_Unpack(argc, argv, "|oIu:Deque", &init, &self->d_bucket_sz))
		return -1;
	if unlikely(!self->d_bucket_sz) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid bucket size: 0");
		return -1;
	}
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
	self->d_head     = NULL;
	self->d_tail     = NULL;
	self->d_size     = 0;
	self->d_head_idx = 0;
	self->d_head_use = 0;
	self->d_tail_sz  = 0;
	self->d_version  = 0;
	weakref_support_init(self);
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
	if (DeeObject_Foreach(init, (dforeach_t)&Deque_PushBack, self) < 0)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
	if (DeeObject_Foreach(init, &deque_push_back, self) < 0)
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
	{
		deq_fini(self);
		return -1;
	}
	return 0;
}



#ifndef CONFIG_NO_THREADS
#define DEQUE_BUFFER_START d_lock
#else /* CONFIG_NO_THREADS */
#define DEQUE_BUFFER_START d_head
#endif /* !CONFIG_NO_THREADS */

#if defined(__GNUC__) && __GNUC__ >= 9
/* Get rid of:
 *   array subscript 0 is outside array bounds of 'DEQUE_BUFFER_TYPE' {aka 'unsigned char[40]'}
 * Either I'm not seeing the bug in my code, or the fact that GCC apparently claims that `0'
 * is outside the bounds of [0,40) is incorrect, because 0 is _very_ _well_ within those bounds! */
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif /* __GNUC__ && __GNUC__ >= 9 */


#define DEQUE_BUFFER_SIZE \
	(sizeof(Deque) - COMPILER_OFFSETOF(Deque, DEQUE_BUFFER_START))
#define DEQUE_BUFFER_SIZE_NOVERSION \
	(COMPILER_OFFSETOF(Deque, d_version) - COMPILER_OFFSETOF(Deque, DEQUE_BUFFER_START))
#define DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION \
	(COMPILER_OFFSETOF(Deque, d_version) - COMPILER_OFFSETOF(Deque, d_head))
typedef __BYTE_TYPE__ DEQUE_BUFFER_TYPE[DEQUE_BUFFER_SIZE];
typedef __BYTE_TYPE__ DEQUE_BUFFER_TYPE_NOLOCK_NOVERSION[DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION];
#define DEQUE_BUFFER_GET(x)        ((Deque *)((__BYTE_TYPE__ *)(x) - COMPILER_OFFSETOF(Deque, DEQUE_BUFFER_START)))
#define DEQUE_BUFFER_FOR(x)        ((__BYTE_TYPE__ *)&(x)->DEQUE_BUFFER_START)
#define DEQUE_BUFFER_GET_NOLOCK(x) ((Deque *)((__BYTE_TYPE__ *)(x) - COMPILER_OFFSETOF(Deque, d_head)))
#define DEQUE_BUFFER_FOR_NOLOCK(x) ((__BYTE_TYPE__ *)&(x)->d_head)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deq_assign(Deque *__restrict self,
           DeeObject *__restrict seq) {
	DEQUE_BUFFER_TYPE bTemp, bOld;
	Deque *temp = DEQUE_BUFFER_GET(bTemp);
	weakref_support_init(temp);
	if (DeeObject_InstanceOfExact(seq, &Deque_Type)) {
		if (deq_copy(temp, (Deque *)seq))
			return -1;
	} else {
		temp->d_head      = NULL;
		temp->d_tail      = NULL;
		temp->d_size      = 0;
		temp->d_head_idx  = 0;
		temp->d_head_use  = 0;
		temp->d_tail_sz   = 0;
		temp->d_bucket_sz = DEQUE_BUCKET_DEFAULT_SIZE;
		rwlock_init(&temp->d_lock);
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
		if (DeeObject_Foreach(seq, (dforeach_t)&Deque_PushBack, temp) < 0)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
		if (DeeObject_Foreach(seq, &deque_push_back, temp) < 0)
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
		{
			deq_fini(temp);
			return -1;
		}
	}
	/* Save the new deque into the current. */
	Deque_LockWrite(self);
	memcpy(DEQUE_BUFFER_FOR_NOLOCK(DEQUE_BUFFER_GET(bOld)),
	       DEQUE_BUFFER_FOR_NOLOCK(self),
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	memcpy(DEQUE_BUFFER_FOR_NOLOCK(self),
	       DEQUE_BUFFER_FOR_NOLOCK(temp),
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	++self->d_version;
	Deque_LockEndWrite(self);
	weakref_support_init(DEQUE_BUFFER_GET(bOld));

	/* Free the old deque state. */
	deq_fini(DEQUE_BUFFER_GET(bOld));
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deq_moveassign(Deque *__restrict self,
               Deque *__restrict other) {
	DEQUE_BUFFER_TYPE bNew, bOld;
	if (self == other)
		return 0;
	Deque_LockWrite(other);
	memcpy(DEQUE_BUFFER_FOR_NOLOCK(DEQUE_BUFFER_GET(bNew)),
	       DEQUE_BUFFER_FOR_NOLOCK(other),
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	/* Clear out the state of the other deque, thus stealing all of its references. */
	memset(DEQUE_BUFFER_FOR_NOLOCK(other), 0,
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	++other->d_version;
	Deque_LockEndWrite(other);
	/* Override the state of our own deque. */
	Deque_LockWrite(self);
	memcpy(DEQUE_BUFFER_FOR_NOLOCK(DEQUE_BUFFER_GET(bOld)),
	       DEQUE_BUFFER_FOR_NOLOCK(self),
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	memcpy(DEQUE_BUFFER_FOR_NOLOCK(self),
	       DEQUE_BUFFER_FOR_NOLOCK(DEQUE_BUFFER_GET(bNew)),
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	++self->d_version;
	Deque_LockEndWrite(self);
	weakref_support_init(DEQUE_BUFFER_GET(bOld));
	/* Free the old state of our own deque. */
	deq_fini(DEQUE_BUFFER_GET(bOld));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL deq_bool(Deque *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->d_size != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->d_size) != 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE NONNULL((1, 2)) void DCALL
deq_visit(Deque *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	DequeBucket *iter;
	if (!self->d_size)
		return;
	Deque_LockRead(self);
	iter = self->d_head;
	for (i = 0; i < self->d_head_use; ++i)
		Dee_Visit(iter->db_items[i + self->d_head_idx]);
	if (iter != self->d_tail) {
		while ((iter = iter->db_next) != self->d_tail) {
			for (i = 0; i < self->d_bucket_sz; ++i)
				Dee_Visit(iter->db_items[i]);
		}
		for (i = 0; i < self->d_tail_sz; ++i)
			Dee_Visit(iter->db_items[i]);
	}
	Deque_LockEndRead(self);
}


PRIVATE NONNULL((1)) void DCALL
deq_clear(Deque *__restrict self) {
	DEQUE_BUFFER_TYPE bData;
	Deque_LockWrite(self);
	memcpy(DEQUE_BUFFER_FOR_NOLOCK(DEQUE_BUFFER_GET(bData)),
	       DEQUE_BUFFER_FOR_NOLOCK(self),
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	/* Clear out the state of the other deque,
	 * thus stealing all of its references. */
	memset(DEQUE_BUFFER_FOR_NOLOCK(self), 0,
	       DEQUE_BUFFER_SIZE_NOLOCK_NOVERSION);
	++self->d_version;
	Deque_LockEndWrite(self);
	weakref_support_init(DEQUE_BUFFER_GET(bData));
	/* Destroy the cloned buffer. */
	deq_fini(DEQUE_BUFFER_GET(bData));
}

PRIVATE void DCALL
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
				DequeIterator_ITEM(&iter) = Dee_None;
				Dee_Incref(Dee_None);
				/* Drop a reference from the cleared object. */
				if (!Dee_DecrefIfNotOne(orig_ob)) {
					Deque_LockEndWrite(self);
					Dee_Decref(orig_ob);
					Deque_LockWrite(self);
					if (version != self->d_version)
						break;
				}
			}
		}
	}
	Deque_LockEndWrite(self);
}


PRIVATE struct type_gc deq_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&deq_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&deq_pclear
};


PRIVATE WUNUSED NONNULL((1)) DREF DequeIteratorObject *DCALL
deq_iter(Deque *__restrict self) {
	DREF DequeIteratorObject *result;
	result = DeeObject_MALLOC(DequeIteratorObject);
	if unlikely(!result)
		goto done;
#ifndef CONFIG_NO_THREADS
	rwlock_init(&result->di_lock);
#endif /* !CONFIG_NO_THREADS */
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_size(Deque *__restrict self) {
#ifndef CONFIG_NO_THREADS
	return DeeInt_NewSize(ATOMIC_READ(self->d_size));
#else /* !CONFIG_NO_THREADS */
	return DeeInt_NewSize(self->d_size);
#endif /* CONFIG_NO_THREADS */
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
			temp = DeeObject_CompareEq(item, elem);
			Dee_Decref(elem);
			if (temp != 0) {
				if unlikely(temp < 0)
					goto err;
				return_true; /* Found it! */
			}
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
deq_get(Deque *self, DeeObject *index_ob) {
	size_t index;
	DREF DeeObject *result;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, deq_size);
		goto err;
	}
	result = DEQUE_ITEM(self, index);
	Dee_Incref(result);
	Deque_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deq_del(Deque *__restrict self, DeeObject *__restrict index_ob) {
	size_t index;
	DREF DeeObject *result;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	result = Deque_Pop(self, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
deq_set(Deque *__restrict self,
        DeeObject *__restrict index_ob,
        DeeObject *__restrict value) {
	size_t index;
	DREF DeeObject *oldval;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, deq_size);
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
deq_nsi_size(Deque *__restrict self) {
#ifndef CONFIG_NO_THREADS
	return ATOMIC_READ(self->d_size);
#else /* !CONFIG_NO_THREADS */
	return self->d_size;
#endif /* CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_nsi_getitem(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, deq_size);
		goto err;
	}
	result = DEQUE_ITEM(self, index);
	Dee_Incref(result);
	Deque_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
deq_nsi_delitem(Deque *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = Deque_Pop(self, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
deq_nsi_setitem(Deque *__restrict self,
                size_t index,
                DeeObject *__restrict value) {
	DREF DeeObject *oldval;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, deq_size);
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

PRIVATE WUNUSED DREF DeeObject *DCALL
deq_nsi_xchitem(Deque *__restrict self,
                size_t index,
                DeeObject *__restrict value) {
	DREF DeeObject *oldval;
	Deque_LockRead(self);
	if (index >= self->d_size) {
		size_t deq_size = self->d_size;
		COMPILER_READ_BARRIER();
		Deque_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, deq_size);
		goto err;
	}
	/* Exchange the stored item. */
	Dee_Incref(value);
	oldval                  = DEQUE_ITEM(self, index);
	DEQUE_ITEM(self, index) = value;
	Deque_LockEndRead(self);
	ASSERT(oldval);
	return oldval;
err:
	return NULL;
}



PRIVATE struct type_nsi deq_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&deq_nsi_size,
			/* .nsi_getsize_fast = */ (void *)&deq_nsi_size,
			/* .nsi_getitem      = */ (void *)&deq_nsi_getitem,
			/* .nsi_delitem      = */ (void *)&deq_nsi_delitem,
			/* .nsi_setitem      = */ (void *)&deq_nsi_setitem,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)NULL,
			/* .nsi_rfind        = */ (void *)NULL,
			/* .nsi_xch          = */ (void *)&deq_nsi_xchitem,
			/* .nsi_insert       = */ (void *)&Deque_Insert,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)&Deque_Pops,
			/* .nsi_erase        = */ (void *)&Deque_Erase,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL,
			/* .nsi_removeif     = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq deq_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&deq_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&deq_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&deq_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&deq_get,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&deq_del,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&deq_set,
	/* XXX: range operators? (Also: Add a fallback delrange / setrange in `sequence' that calls
	 *                              forward to member functions `insert(index:?Dint, ob)',
	 *                             `erase(index:?Dint, count:?Dint = 1)') */
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_nsi       = */ &deq_nsi
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_pushfront(Deque *self, size_t argc, DeeObject **argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &item))
		goto err;
	if (Deque_PushFront(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_pushback(Deque *self, size_t argc, DeeObject **argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushback", &item))
		goto err;
	if (Deque_PushBack(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_popfront(Deque *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return Deque_PopFront(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_popback(Deque *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	return Deque_PopBack(self);
err:
	return NULL;
}

INTERN struct keyword seq_insert_kwlist[] = { K(index), K(item), KEND };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_insert(Deque *self, size_t argc,
           DeeObject **argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insert_kwlist, "Ido:insert", &index, &item))
		goto err;
	if (Deque_Insert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN struct keyword seq_erase_kwlist[] = { K(index), K(count), KEND };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_erase(Deque *self, size_t argc,
          DeeObject **argv, DeeObject *kw) {
	size_t index, result;
	size_t num_items = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_erase_kwlist, "Id|Iu:erase", &index, &num_items))
		goto err;
	result = Deque_Erase(self, index, num_items);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN struct keyword seq_pop_kwlist[] = { K(index), KEND };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_pop(Deque *self, size_t argc,
        DeeObject **argv, DeeObject *kw) {
	size_t index;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_pop_kwlist, "Id:pop", &index))
		return NULL;
	return Deque_Pop(self, index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_llrot(Deque *self, size_t argc, DeeObject **argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, "Iu:llrot", &num_items) ||
	    Deque_llrot(self, num_items))
		return NULL;
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_lrrot(Deque *self, size_t argc, DeeObject **argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, "Iu:lrrot", &num_items) ||
	    Deque_lrrot(self, num_items))
		return NULL;
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_rlrot(Deque *self, size_t argc, DeeObject **argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, "Iu:rlrot", &num_items) ||
	    Deque_rlrot(self, num_items))
		return NULL;
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_rrrot(Deque *self, size_t argc, DeeObject **argv) {
	size_t num_items;
	if (DeeArg_Unpack(argc, argv, "Iu:rrrot", &num_items) ||
	    Deque_rrrot(self, num_items))
		return NULL;
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deq_sizeof(Deque *self, size_t argc, DeeObject **argv) {
	size_t result;
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	result = sizeof(Deque);
	Deque_LockRead(self);
	ASSERT((self->d_head != NULL) == (self->d_tail != NULL));
	if (self->d_head) {
		size_t count      = 1;
		DequeBucket *iter = self->d_head;
		for (; iter != self->d_tail; iter = iter->db_next)
			++count;
		result += count * SIZEOF_BUCKET(self->d_bucket_sz);
	}
	Deque_LockEndRead(self);
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE struct type_method deq_methods[] = {
	{ "insert", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_insert,
	  DOC("(index:?Dint,ob)\n"
	      "@throw IntegerOverflow @index is negative or too large\n"
	      "Insert the given object @ob at @index"),
	  TYPE_METHOD_FKWDS },
	{ "erase", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_erase,
	  DOC("(index:?Dint,num_items=!1)->?Dint\n"
	      "@throw IntegerOverflow @index is negative or too large\n"
	      "@return The actual number of erased items\n"
	      "Erase up to @num_items objects from @this deque, starting at @index"),
	  TYPE_METHOD_FKWDS },
	{ "pop", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_pop,
	  DOC("(index=!-1)->\n"
	      "@throw IntegerOverflow @index is negative or too large\n"
	      "@return The item that got removed\n"
	      "Pop (erase) the item located at @index and return it"),
	  TYPE_METHOD_FKWDS },
	{ "pushfront", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_pushfront,
	  DOC("(ob)\n"
	      "Insert the given object @ob at the front of @this deque") },
	{ "pushback", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_pushback,
	  DOC("(ob)\n"
	      "Insert the given object @ob at the back of @this deque") },
	{ "popfront", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_popfront,
	  DOC("->\n"
	      "@throw ValueError @this deque is empty\n"
	      "Pop and return an item from the front of @this deque") },
	{ "popback", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_popback,
	  DOC("->\n"
	      "@throw ValueError @this deque is empty\n"
	      "Pop and return an item from the back of @this deque") },
	{ "llrot", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_llrot,
	  DOC("(num_items:?Dint)\n"
	      "@throw IndexError @this deque contain less than @num_items items\n"
	      "Rotate the first @num_items items left by 1:\n"
	      ">import deque from collections;\n"
	      ">local x = deque({ 10, 20, 30, 40, 50 });\n"
	      ">x.llrot(3);\n"
	      ">print repr x; /* { 20, 30, 10, 40, 50 } */") },
	{ "lrrot", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_lrrot,
	  DOC("(num_items:?Dint)\n"
	      "@throw IndexError @this deque contain less than @num_items items\n"
	      "Rotate the first @num_items items right by 1:\n"
	      ">import deque from collections;\n"
	      ">local x = deque({ 10, 20, 30, 40, 50 });\n"
	      ">x.lrrot(3);\n"
	      ">print repr x; /* { 30, 10, 20, 40, 50 } */") },
	{ "rlrot", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_rlrot,
	  DOC("(num_items:?Dint)\n"
	      "@throw IndexError @this deque contain less than @num_items items\n"
	      "Rotate the last @num_items items left by 1:\n"
	      ">import deque from collections;\n"
	      ">local x = deque({ 10, 20, 30, 40, 50 });\n"
	      ">x.rlrot(3);\n"
	      ">print repr x; /* { 10, 20, 40, 50, 30 } */") },
	{ "rrrot", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_rrrot,
	  DOC("(num_items:?Dint)\n"
	      "@throw IndexError @this deque contain less than @num_items items\n"
	      "Rotate the last @num_items items right by 1:\n"
	      ">import deque from collections;\n"
	      ">local x = deque({ 10, 20, 30, 40, 50 });\n"
	      ">x.rrrot(3);\n"
	      ">print repr x; /* { 10, 20, 50, 30, 40 } */") },
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&deq_sizeof,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_member deq_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DequeIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject Deque_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Deque",
	/* .tp_doc      = */ DOC("A double-ended queue that allows for O(1) insertion and removal at the front "
	                         "and back, while still providing a decently fast index-based item lookup, "
	                         "as well as fast iteration both in forward, as well as in reverse\n"
	                         "A Deque in its purest form (with a #bucketsize of $1) is really nothing "
	                         "but a doubly-linked list. However instead of every node only ever containing "
	                         "a single item, each is a pre-allocated vector of up to #bucketsize objects, "
	                         "meaning that an index-based lookup takes at most O(floor((n / bucketsize) / 2).\n"
	                         "The second division by 2 is done because indices closer to the end of the Deque "
	                         "are searched for from its end, rather than its start\n"
	                         "However, inserting or erasure anywhere else is quite expensive, "
	                         "as doing so requires items to be moved around a lot\n"
	                         "\n"
	                         "()\n"
	                         "Construct an empty Deque\n"
	                         "\n"
	                         "(seq:?S?O)\n"
	                         "(seq:?S?O,bucketsize:?Dint)\n"
	                         "Construct a Deque pre-initialized with items from "
	                         "@seq, and set the Deque's bucket size to @bucketsize\n"
	                         "When @bucketsize is omitted, an implementation-specific "
	                         "default value is used, which may also depend on other "
	                         "environmental factors selected to maximize performance\n"
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
	                         "Returns :true if @this Deque is non-empty. :false otherwise\n"
	                         "\n"
	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this Deque in ascending order\n"
	                         "\n"
	                         "[]->\n"
	                         "@throw IndexError @index is greater that the length of @this Deque\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Returns the @index'th item of @this Deque\n"
	                         "\n"
	                         "[]=->\n"
	                         "@throw IndexError @index is greater that the length of @this Deque\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Set the @index'th item of @this Deque to @item\n"
	                         "\n"
	                         "del[]->\n"
	                         "@throw IndexError @index is greater that the length of @this Deque\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Same as ${this.pop(index)} for positive values for @index\n"
	                         "\n"
	                         "contains->\n"
	                         "Returns :true if @item is apart of this Deque, :false otherwise"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(Deque),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&deq_ctor, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (void *)&deq_copy,
				/* .tp_deep_ctor = */ (void *)&deq_copy,
				/* .tp_any_ctor  = */ (void *)&deq_init,
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&deq_visit,
	/* .tp_gc            = */ &deq_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &deq_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ deq_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ deq_class_members
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
deqiter_ctor(DequeIteratorObject *__restrict self) {
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->di_lock);
#endif /* !CONFIG_NO_THREADS */
	self->di_deq = (DREF Deque *)DeeObject_NewDefault(&Deque_Type);
	if unlikely(!self->di_deq)
		return -1;
	self->di_ver = 0;
	DequeIterator_InitBegin(&self->di_iter, self->di_deq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deqiter_copy(DequeIteratorObject *__restrict self,
             DequeIteratorObject *__restrict other) {
	rwlock_init(&self->di_lock);
	self->di_deq = other->di_deq;
	Dee_Incref(self->di_deq);
	rwlock_read(&other->di_lock);
	self->di_ver  = other->di_ver;
	self->di_iter = other->di_iter;
	rwlock_endread(&other->di_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
deqiter_getindex(DequeIteratorObject *__restrict self) {
	size_t result;
	rwlock_read(&self->di_lock);
	Deque_LockRead(self->di_deq);
	result = (unlikely(self->di_ver != self->di_deq->d_version))
	         ? (size_t)-1
	         : DequeIterator_GetIndex(&self->di_iter, self->di_deq);
	Deque_LockEndRead(self->di_deq);
	rwlock_endread(&self->di_lock);
	return result;
}

PRIVATE NONNULL((1)) void DCALL
deqiter_setindex(DequeIteratorObject *__restrict self, size_t index) {
	rwlock_write(&self->di_lock);
	Deque_LockRead(self->di_deq);
	self->di_ver = self->di_deq->d_version;
	if unlikely(index >= self->di_deq->d_size)
		--self->di_ver;
	else {
		DequeIterator_InitAt(&self->di_iter,
		                     self->di_deq,
		                     index);
	}
	Deque_LockEndRead(self->di_deq);
	rwlock_endwrite(&self->di_lock);
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
	rwlock_init(&self->di_lock);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
deqiter_init(DequeIteratorObject *__restrict self,
             size_t argc, DeeObject **argv) {
	size_t index = 0;
	if (DeeArg_Unpack(argc, argv, "o|Iu:DequeIterator", &self->di_deq, &index))
		goto err;
	if (DeeObject_AssertType((DeeObject *)self->di_deq, &Deque_Type))
		goto err;
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->di_lock);
#endif /* !CONFIG_NO_THREADS */
	Dee_Incref(self->di_deq);
	Deque_LockRead(self->di_deq);
	self->di_ver = self->di_deq->d_version;
	COMPILER_READ_BARRIER();
	if unlikely(index >= self->di_deq->d_size)
		--self->di_ver;
	else {
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
	rwlock_read(&self->di_lock);
	result = (self->di_ver == self->di_deq->d_version);
	rwlock_endread(&self->di_lock);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deqiter_next(DequeIteratorObject *__restrict self) {
	DREF DeeObject *result;
	rwlock_read(&self->di_lock);
	Deque_LockRead(self->di_deq);
	if (self->di_ver != self->di_deq->d_version)
		result = ITER_DONE;
	else {
		result = DequeIterator_ITEM(&self->di_iter);
		Dee_Incref(result);
		if (DequeIterator_HasNext(&self->di_iter, self->di_deq))
			DequeIterator_Next(&self->di_iter, self->di_deq);
		else {
			--self->di_ver; /* Invalidate the version number. */
		}
	}
	Deque_LockEndRead(self->di_deq);
	rwlock_endread(&self->di_lock);
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
deqiter_setindex_ob(DequeIteratorObject *__restrict self,
                    DeeObject *__restrict value) {
	size_t newindex = (size_t)-1;
	if (!DeeNone_Check(value) &&
	    DeeObject_AsSize(value, &newindex))
		goto err;
	deqiter_setindex(self, newindex);
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset deqiter_getsets[] = {
	{ "index",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&deqiter_getindex_ob,
	  NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&deqiter_setindex_ob,
	  DOC("->?X2?Dint?N\n"
	      "Get/set the index of @this iterator within its associated :Deque\n"
	      "When :none, the iterator has been invalidated, possibly due to the "
	      "deque having changed") },
	{ NULL }
};

PRIVATE struct type_member deqiter_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(DequeIteratorObject, di_deq)),
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
				/* .tp_ctor      = */ (void *)&deqiter_ctor,
				/* .tp_copy_ctor = */ (void *)&deqiter_copy,
				/* .tp_deep_ctor = */ (void *)&deqiter_deep,
				/* .tp_any_ctor  = */ (void *)&deqiter_init,
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&deqiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&deqiter_next,
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
