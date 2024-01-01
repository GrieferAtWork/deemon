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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SVEC_H
#define GUARD_DEEMON_OBJECTS_SEQ_SVEC_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/lock.h>

DECL_BEGIN

/* A type `RefVector' that acts and works very much the same as `SharedVector',
 * but instead allows other objects to enumerate private vectors, such as the
 * global objects of modules, or their imports, as well as static variables of
 * code objects, etc. etc. etc... */
typedef struct {
	OBJECT_HEAD
	DREF DeeObject      *rv_owner;    /* [1..1] The object that is actually owning the vector. */
	size_t               rv_length;   /* [const] The number of items in this vector. */
	DREF DeeObject     **rv_vector;   /* [0..1][lock(*rv_plock)][0..rv_length][lock(*rv_plock)][const]
	                                   * The vector of objects that is being referenced.
	                                   * NOTE: Elements of this vector must not be changed. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t *rv_plock;    /* [0..1][const] An optional lock that must be held when accessing the list.
	                                   * Also: when non-NULL, items of the vector can be modified. */
#define RefVector_IsWritable(self) ((self)->rv_plock != NULL)
#else /* !CONFIG_NO_THREADS */
	bool                 rv_writable; /* [const] Set to true if items of the vector can be modified. */
#define RefVector_IsWritable(self) ((self)->rv_writable)
#endif /* CONFIG_NO_THREADS */
} RefVector;

#define RefVector_LockReading(self)    Dee_atomic_rwlock_reading((self)->rv_plock)
#define RefVector_LockWriting(self)    Dee_atomic_rwlock_writing((self)->rv_plock)
#define RefVector_LockTryRead(self)    Dee_atomic_rwlock_tryread((self)->rv_plock)
#define RefVector_LockTryWrite(self)   Dee_atomic_rwlock_trywrite((self)->rv_plock)
#define RefVector_LockCanRead(self)    Dee_atomic_rwlock_canread((self)->rv_plock)
#define RefVector_LockCanWrite(self)   Dee_atomic_rwlock_canwrite((self)->rv_plock)
#define RefVector_LockWaitRead(self)   Dee_atomic_rwlock_waitread((self)->rv_plock)
#define RefVector_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite((self)->rv_plock)
#define RefVector_LockRead(self)       Dee_atomic_rwlock_read((self)->rv_plock)
#define RefVector_LockWrite(self)      Dee_atomic_rwlock_write((self)->rv_plock)
#define RefVector_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade((self)->rv_plock)
#define RefVector_LockUpgrade(self)    Dee_atomic_rwlock_upgrade((self)->rv_plock)
#define RefVector_LockDowngrade(self)  Dee_atomic_rwlock_downgrade((self)->rv_plock)
#define RefVector_LockEndWrite(self)   Dee_atomic_rwlock_endwrite((self)->rv_plock)
#define RefVector_LockEndRead(self)    Dee_atomic_rwlock_endread((self)->rv_plock)
#define RefVector_LockEnd(self)        Dee_atomic_rwlock_end((self)->rv_plock)

#ifdef CONFIG_NO_THREADS
#define RefVector_XLockReading(self)    1
#define RefVector_XLockWriting(self)    0
#define RefVector_XLockTryRead(self)    1
#define RefVector_XLockTryWrite(self)   1
#define RefVector_XLockCanRead(self)    1
#define RefVector_XLockCanWrite(self)   0
#define RefVector_XLockWaitRead(self)   (void)0
#define RefVector_XLockWaitWrite(self)  (void)0
#define RefVector_XLockRead(self)       (void)0
#define RefVector_XLockWrite(self)      (void)0
#define RefVector_XLockTryUpgrade(self) 1
#define RefVector_XLockUpgrade(self)    1
#define RefVector_XLockDowngrade(self)  (void)0
#define RefVector_XLockEndWrite(self)   (void)0
#define RefVector_XLockEndRead(self)    (void)0
#define RefVector_XLockEnd(self)        (void)0
#else /* CONFIG_NO_THREADS */
#define RefVector_XLockReading(self)    ((self)->rv_plock ? RefVector_LockReading(self) : 1)
#define RefVector_XLockWriting(self)    ((self)->rv_plock ? RefVector_LockWriting(self) : 0)
#define RefVector_XLockTryRead(self)    ((self)->rv_plock ? RefVector_LockTryRead(self) : 1)
#define RefVector_XLockTryWrite(self)   ((self)->rv_plock ? RefVector_LockTryWrite(self) : 1)
#define RefVector_XLockCanRead(self)    ((self)->rv_plock ? RefVector_LockCanRead(self) : 1)
#define RefVector_XLockCanWrite(self)   ((self)->rv_plock ? RefVector_LockCanWrite(self) : 0)
#define RefVector_XLockWaitRead(self)   ((self)->rv_plock ? RefVector_LockWaitRead(self) : (void)0)
#define RefVector_XLockWaitWrite(self)  ((self)->rv_plock ? RefVector_LockWaitWrite(self) : (void)0)
#define RefVector_XLockRead(self)       ((self)->rv_plock ? RefVector_LockRead(self) : (void)0)
#define RefVector_XLockWrite(self)      ((self)->rv_plock ? RefVector_LockWrite(self) : (void)0)
#define RefVector_XLockTryUpgrade(self) ((self)->rv_plock ? RefVector_LockTryUpgrade(self) : 1)
#define RefVector_XLockUpgrade(self)    ((self)->rv_plock ? RefVector_LockUpgrade(self) : 1)
#define RefVector_XLockDowngrade(self)  ((self)->rv_plock ? RefVector_LockDowngrade(self) : (void)0)
#define RefVector_XLockEndWrite(self)   ((self)->rv_plock ? RefVector_LockEndWrite(self) : (void)0)
#define RefVector_XLockEndRead(self)    ((self)->rv_plock ? RefVector_LockEndRead(self) : (void)0)
#define RefVector_XLockEnd(self)        ((self)->rv_plock ? RefVector_LockEnd(self) : (void)0)
#endif /* !CONFIG_NO_THREADS */


typedef struct {
	OBJECT_HEAD
	DREF RefVector  *rvi_vector; /* [1..1][const] The underlying vector being iterated. */
	DREF DeeObject **rvi_pos;    /* [0..1][lock(*rvi_vector->rv_plock)][1..1][in(rvi_vector->rv_vector)][atomic]
	                              * The current iterator position. */
} RefVectorIterator;


INTDEF DeeTypeObject RefVector_Type;
INTDEF DeeTypeObject RefVectorIterator_Type;


typedef struct {
	OBJECT_HEAD
	size_t                 sv_length; /* [lock(sv_lock)] The number of items in this vector. */
	DREF DeeObject *const *sv_vector; /* [1..1][const][0..sv_length][lock(sv_lock)][owned]
	                                   * The vector of objects that is being referenced.
	                                   * NOTE: Elements of this vector must not be changed. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t    sv_lock;   /* Lock for this shared-vector. */
#endif /* !CONFIG_NO_THREADS */
} SharedVector;

#define SharedVector_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->sv_lock)
#define SharedVector_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->sv_lock)
#define SharedVector_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->sv_lock)
#define SharedVector_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->sv_lock)
#define SharedVector_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->sv_lock)
#define SharedVector_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->sv_lock)
#define SharedVector_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->sv_lock)
#define SharedVector_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->sv_lock)
#define SharedVector_LockRead(self)       Dee_atomic_rwlock_read(&(self)->sv_lock)
#define SharedVector_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->sv_lock)
#define SharedVector_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->sv_lock)
#define SharedVector_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->sv_lock)
#define SharedVector_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->sv_lock)
#define SharedVector_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->sv_lock)
#define SharedVector_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->sv_lock)
#define SharedVector_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->sv_lock)

INTDEF DeeTypeObject SharedVector_Type;


typedef struct {
	OBJECT_HEAD
	DREF SharedVector *si_seq;   /* [1..1][const] The shared-vector that is being iterated. */
	size_t             si_index; /* [atomic] The current sequence index.
	                              * Should this value be `>= si_seq->sv_length',
	                              * then the iterator has been exhausted. */
} SharedVectorIterator;

INTDEF DeeTypeObject SharedVectorIterator_Type;


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SVEC_H */
