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
/*!export **/
/*!export Dee_simple_hashset_**/
/*!export Dee_SIMPLE_HASHSET_**/
/*!export Dee_simple_hashset_with_lock_**/
/*!export Dee_SIMPLE_HASHSET_WITH_LOCK_**/
#ifndef GUARD_DEEMON_UTIL_SIMPLE_HASHSET_H
#define GUARD_DEEMON_UTIL_SIMPLE_HASHSET_H 1 /*!export-*/

#include "../api.h"

#include <hybrid/typecore.h> /* __UINTPTR_TYPE__ */

#include "../alloc.h"           /* Dee_Free, Dee_Mallocc, Dee_TryMallocc */
#include "../object.h"          /* DREF, DeeObject, Dee_XDecref, Dee_XIncref, Dee_hash_t */
#include "../system-features.h" /* memcpyc */
#include "../type.h"            /* Dee_XVisit, Dee_visit_t */
#include "lock.h"               /* Dee_ATOMIC_LOCK_INIT, Dee_atomic_lock_* */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

struct Dee_simple_hashset_item {
	DREF DeeObject *shsi_key;  /* [0..1] Set item key. */
	Dee_hash_t      shsi_hash; /* [valis_if(shsi_key)] Hash of `shsi_key'. */
};

struct Dee_simple_hashset {
	struct Dee_simple_hashset_item *shs_elem; /* [0..hs_size|ALLOC(hs_mask+1)] Set keys. */
	size_t                          shs_mask; /* [>= shs_size] Allocated set size minus 1. */
	size_t                          shs_size; /* [<= shs_mask] Amount of non-NULL keys. */
};

#define Dee_SIMPLE_HASHSET_INIT { NULL, 0, 0 }
#define Dee_simple_hashset_init(self) \
	(void)((self)->shs_elem = NULL, (self)->shs_mask = (self)->shs_size = 0)
#define Dee_simple_hashset_cinit(self)           \
	(void)(Dee_ASSERT((self)->shs_elem == NULL), \
	       Dee_ASSERT((self)->shs_mask == 0),    \
	       Dee_ASSERT((self)->shs_size == 0))

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_copy(struct Dee_simple_hashset *__restrict self,
                        struct Dee_simple_hashset const *__restrict other) {
	self->shs_mask = other->shs_mask;
	self->shs_size = other->shs_size;
	self->shs_elem = other->shs_elem;
	if (self->shs_elem) {
		size_t i;
		struct Dee_simple_hashset_item *copy;
		copy = (struct Dee_simple_hashset_item *)Dee_Mallocc(self->shs_mask + 1,
		                                                     sizeof(struct Dee_simple_hashset_item));
		if unlikely(!copy)
			goto err;
		copy = (struct Dee_simple_hashset_item *)memcpyc(copy, self->shs_elem, self->shs_mask + 1,
		                                                 sizeof(struct Dee_simple_hashset_item));
		self->shs_elem = copy;
		for (i = 0; i <= self->shs_mask; ++i)
			Dee_XIncref(copy[i].shsi_key);
	}
	return 0;
err:
	return -1;
}

#define Dee_simple_hashset_fini_nodecref(self) Dee_Free((self)->shs_elem)
LOCAL NONNULL((1)) void DCALL
Dee_simple_hashset_fini(struct Dee_simple_hashset *__restrict self) {
	if likely(self->shs_elem) {
		size_t i;
		for (i = 0; i <= self->shs_mask; ++i)
			Dee_XDecref(self->shs_elem[i].shsi_key);
		Dee_Free(self->shs_elem);
	}
}

LOCAL NONNULL((1)) void DCALL
Dee_simple_hashset_clear(struct Dee_simple_hashset *__restrict self) {
	if likely(self->shs_elem) {
		size_t i;
		for (i = 0; i <= self->shs_mask; ++i)
			Dee_XDecref(self->shs_elem[i].shsi_key);
		Dee_Free(self->shs_elem);
		self->shs_elem = NULL;
		self->shs_mask = 0;
		self->shs_size = 0;
	}
}

LOCAL NONNULL((1, 2)) void DCALL
Dee_simple_hashset_visit(struct Dee_simple_hashset *__restrict self,
                         Dee_visit_t proc, void *arg) {
	size_t i;
	if likely(self->shs_elem) {
		for (i = 0; i <= self->shs_mask; ++i)
			Dee_XVisit(self->shs_elem[i].shsi_key);
	}
}

/* Hash config for `struct Dee_simple_hashset' */
#define Dee_simple_hashset_hashst(self, hash)  ((hash) & (self)->shs_mask)
#define Dee_simple_hashset_hashnx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_simple_hashset_hashit(self, i)     ((self)->shs_elem + ((i) & (self)->shs_mask))

/* Insert `item' into `self' (if it wasn't present already)
 * @return:  1: Successfully inserted the object.
 * @return:  0: An identical object already exists.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_insert(struct Dee_simple_hashset *__restrict self,
                          DeeObject *item);


struct Dee_simple_hashset_with_lock {
	struct Dee_simple_hashset shswl_set;  /* Underlying hash-set */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t         shswl_lock; /* Lock for the hashset */
#endif /* !CONFIG_NO_THREADS */
};

#ifndef CONFIG_NO_THREADS
#define Dee_SIMPLE_HASHSET_WITH_LOCK_INIT { Dee_SIMPLE_HASHSET_INIT, Dee_ATOMIC_LOCK_INIT }
#else /* !CONFIG_NO_THREADS */
#define Dee_SIMPLE_HASHSET_WITH_LOCK_INIT { Dee_SIMPLE_HASHSET_INIT }
#endif /* CONFIG_NO_THREADS */
#define Dee_simple_hashset_with_lock_init(self) \
	(Dee_simple_hashset_init(&(self)->shswl_set), Dee_atomic_lock_init(&(self)->shswl_lock))
#define Dee_simple_hashset_with_lock_cinit(self) \
	(Dee_simple_hashset_cinit(&(self)->shswl_set), Dee_atomic_lock_cinit(&(self)->shswl_lock))


/* Locking helpers. */
#define Dee_simple_hashset_with_lock_available(self)  Dee_atomic_lock_available(&(self)->shswl_lock)
#define Dee_simple_hashset_with_lock_acquired(self)   Dee_atomic_lock_acquired(&(self)->shswl_lock)
#define Dee_simple_hashset_with_lock_tryacquire(self) Dee_atomic_lock_tryacquire(&(self)->shswl_lock)
#define Dee_simple_hashset_with_lock_acquire(self)    Dee_atomic_lock_acquire(&(self)->shswl_lock)
#define Dee_simple_hashset_with_lock_waitfor(self)    Dee_atomic_lock_waitfor(&(self)->shswl_lock)
#define Dee_simple_hashset_with_lock_release(self)    Dee_atomic_lock_release(&(self)->shswl_lock)


#define Dee_simple_hashset_with_lock_fini_nodecref(self) \
	Dee_simple_hashset_fini_nodecref(&(self)->shswl_set)
#define Dee_simple_hashset_with_lock_fini(self) \
	Dee_simple_hashset_fini(&(self)->shswl_set)
#define Dee_simple_hashset_with_lock_visit(self, proc, arg)   \
	(Dee_simple_hashset_with_lock_acquire(self),              \
	 Dee_simple_hashset_visit(&(self)->shswl_set, proc, arg), \
	 Dee_simple_hashset_with_lock_release(self))

struct Dee_serial;
#ifndef Dee_seraddr_t_DEFINED
#define Dee_seraddr_t_DEFINED           /*!export-*/
typedef __UINTPTR_TYPE__ Dee_seraddr_t; /*!export-*/ /* Should `#include <deemon/serial.h>' for this one... */
#endif /* !Dee_seraddr_t_DEFINED */

DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_with_lock_serialize(struct Dee_simple_hashset_with_lock *__restrict self,
                                       struct Dee_serial *__restrict writer, Dee_seraddr_t addr);

#ifdef CONFIG_NO_THREADS
#define Dee_simple_hashset_with_lock_insert(self, item) \
	Dee_simple_hashset_insert(&(self)->shswl_set, item)
#define Dee_simple_hashset_with_lock_copy(self, other) \
	Dee_simple_hashset_copy(&(self)->shswl_set, &(other)->shswl_set)
#define Dee_simple_hashset_with_lock_clear(self) \
	Dee_simple_hashset_clear(&(self)->shswl_set)
#else /* CONFIG_NO_THREADS */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_with_lock_insert(struct Dee_simple_hashset_with_lock *__restrict self,
                                    DeeObject *item);
LOCAL WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_with_lock_copy(struct Dee_simple_hashset_with_lock *__restrict self,
                                  struct Dee_simple_hashset_with_lock *__restrict other) {
again:
	Dee_simple_hashset_with_lock_acquire(other);
	self->shswl_set.shs_mask = other->shswl_set.shs_mask;
	self->shswl_set.shs_size = other->shswl_set.shs_size;
	self->shswl_set.shs_elem = other->shswl_set.shs_elem;
	if (self->shswl_set.shs_elem) {
		size_t i;
		struct Dee_simple_hashset_item *copy;
		copy = (struct Dee_simple_hashset_item *)Dee_TryMallocc(self->shswl_set.shs_mask + 1,
		                                                        sizeof(struct Dee_simple_hashset_item));
		if unlikely(!copy) {
			Dee_simple_hashset_with_lock_release(other);
			copy = (struct Dee_simple_hashset_item *)Dee_Mallocc(self->shswl_set.shs_mask + 1,
			                                                     sizeof(struct Dee_simple_hashset_item));
			if unlikely(!copy)
				goto err;
			Dee_simple_hashset_with_lock_acquire(other);
			if unlikely(self->shswl_set.shs_mask != other->shswl_set.shs_mask ||
			            self->shswl_set.shs_size != other->shswl_set.shs_size ||
			            self->shswl_set.shs_elem != other->shswl_set.shs_elem) {
				Dee_Free(copy);
				Dee_simple_hashset_with_lock_release(other);
				goto again;
			}
		}
		copy = (struct Dee_simple_hashset_item *)memcpyc(copy, self->shswl_set.shs_elem,
		                                                 self->shswl_set.shs_mask + 1,
		                                                 sizeof(struct Dee_simple_hashset_item));
		self->shswl_set.shs_elem = copy;
		for (i = 0; i <= self->shswl_set.shs_mask; ++i)
			Dee_XIncref(copy[i].shsi_key);
	}
	Dee_simple_hashset_with_lock_release(other);
	Dee_atomic_lock_init(&self->shswl_lock);
	return 0;
err:
	return -1;
}

LOCAL NONNULL((1)) void DCALL
Dee_simple_hashset_with_lock_clear(struct Dee_simple_hashset_with_lock *__restrict self) {
	Dee_simple_hashset_with_lock_acquire(self);
	if likely(self->shswl_set.shs_elem) {
		size_t i, mask;
		struct Dee_simple_hashset_item *elem;
		mask = self->shswl_set.shs_mask;
		elem = self->shswl_set.shs_elem;
		self->shswl_set.shs_elem = NULL;
		self->shswl_set.shs_mask = 0;
		self->shswl_set.shs_size = 0;
		Dee_simple_hashset_with_lock_release(self);
		for (i = 0; i <= mask; ++i)
			Dee_XDecref(elem[i].shsi_key);
		Dee_Free(elem);
	} else {
		Dee_simple_hashset_with_lock_release(self);
	}
}
#endif /* !CONFIG_NO_THREADS */


DECL_END

#endif /* !GUARD_DEEMON_UTIL_SIMPLE_HASHSET_H */
