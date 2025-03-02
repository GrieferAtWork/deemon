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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_CONCAT_H
#define GUARD_DEEMON_OBJECTS_SEQ_CONCAT_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

DECL_BEGIN

typedef DeeTupleObject Cat;

INTDEF DeeTypeObject SeqConcat_Type;
INTDEF DeeTypeObject SeqConcatIterator_Type;
#define SeqConcat_Check(ob) DeeObject_InstanceOfExact(ob, &SeqConcat_Type)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject     *cti_curr; /* [1..1][lock(cti_lock)] The current iterator. */
	DeeObject   *const *cti_pseq; /* [1..1][1..1][lock(cti_lock)][in(cti_cat)] The current sequence. */
	DREF Cat           *cti_cat;  /* [1..1][const] The underly sequence cat. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t cti_lock; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
} CatIterator;

#define CatIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->cti_lock)
#define CatIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->cti_lock)
#define CatIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->cti_lock)
#define CatIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->cti_lock)
#define CatIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->cti_lock)
#define CatIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->cti_lock)
#define CatIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->cti_lock)
#define CatIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->cti_lock)
#define CatIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->cti_lock)
#define CatIterator_LockRead2(a, b)      Dee_atomic_rwlock_read_2(&(a)->cti_lock, &(b)->cti_lock)
#define CatIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->cti_lock)
#define CatIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->cti_lock)
#define CatIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->cti_lock)
#define CatIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->cti_lock)
#define CatIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->cti_lock)
#define CatIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->cti_lock)
#define CatIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->cti_lock)

/* Construct new concat-proxy-sequence objects. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Concat(DeeObject *self, DeeObject *other);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_CONCAT_H */
