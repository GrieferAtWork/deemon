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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SET_H
#define GUARD_DEEMON_OBJECTS_SEQ_SET_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/util/lock.h>

DECL_BEGIN

/* Set proxy types used to implement set operations on the C-level. */

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *su_a; /* [1..1][const] The first set of the union. */
	DREF DeeObject *su_b; /* [1..1][const] The second set of the union. */
} SetUnion;

typedef struct {
	OBJECT_HEAD
	DREF SetUnion      *sui_union; /* [1..1][const] The underlying union-set. */
	DREF DeeObject     *sui_iter;  /* [1..1][lock(sui_lock)] The current iterator. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t sui_lock;  /* Lock for `sui_iter' and `sui_in2nd' */
#endif /* !CONFIG_NO_THREADS */
	bool                sui_in2nd; /* [lock(sui_lock)] The second set is being iterated. */
} SetUnionIterator;

#define SetUnionIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->sui_lock)
#define SetUnionIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->sui_lock)
#define SetUnionIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->sui_lock)
#define SetUnionIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->sui_lock)
#define SetUnionIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->sui_lock)
#define SetUnionIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->sui_lock)
#define SetUnionIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->sui_lock)
#define SetUnionIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->sui_lock)
#define SetUnionIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->sui_lock)
#define SetUnionIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->sui_lock)
#define SetUnionIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->sui_lock)
#define SetUnionIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->sui_lock)
#define SetUnionIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->sui_lock)
#define SetUnionIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->sui_lock)
#define SetUnionIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->sui_lock)
#define SetUnionIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->sui_lock)

INTDEF DeeTypeObject SetUnion_Type;
INTDEF DeeTypeObject SetUnionIterator_Type;



typedef struct {
	OBJECT_HEAD
	DREF DeeObject *si_a; /* [1..1][const] The first set of the intersection. */
	DREF DeeObject *si_b; /* [1..1][const] The second set of the intersection. */
} SetIntersection;

typedef struct {
	OBJECT_HEAD
	DREF SetIntersection *sii_intersect; /* [1..1][const] The underlying intersection-set. */
	DREF DeeObject       *sii_iter;      /* [1..1][const] An iterator for `sii_intersect->si_a' */
	DREF DeeObject       *sii_other;     /* [1..1][const][== sii_intersect->si_b]. */
} SetIntersectionIterator;

INTDEF DeeTypeObject SetIntersection_Type;
INTDEF DeeTypeObject SetIntersectionIterator_Type;



typedef struct {
	OBJECT_HEAD
	DREF DeeObject *sd_a; /* [1..1][const] The primary set. */
	DREF DeeObject *sd_b; /* [1..1][const] The set of objects removed from `sd_a'. */
} SetDifference;

typedef struct {
	OBJECT_HEAD
	DREF SetDifference *sdi_diff;  /* [1..1][const] The underlying difference-set. */
	DREF DeeObject     *sdi_iter;  /* [1..1][const] An iterator for `sdi_diff->sd_a' */
	DREF DeeObject     *sdi_other; /* [1..1][const][== sdi_diff->sd_b]. */
} SetDifferenceIterator;

INTDEF DeeTypeObject SetDifference_Type;
INTDEF DeeTypeObject SetDifferenceIterator_Type;



typedef struct {
	OBJECT_HEAD
	DREF DeeObject *ssd_a; /* [1..1][const] The first set. */
	DREF DeeObject *ssd_b; /* [1..1][const] The second set. */
} SetSymmetricDifference;

typedef struct {
	OBJECT_HEAD
	DREF SetSymmetricDifference *ssd_set;   /* [1..1][const] The underlying set. */
	DREF DeeObject              *ssd_iter;  /* [1..1][lock(ssd_lock)] The current iterator. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t          ssd_lock;  /* Lock for `ssd_iter' and `ssd_in2nd' */
#endif /* !CONFIG_NO_THREADS */
	bool                         ssd_in2nd; /* [lock(ssd_lock)] The second set is being iterated. */
} SetSymmetricDifferenceIterator;

#define SetSymmetricDifferenceIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->ssd_lock)

INTDEF DeeTypeObject SetSymmetricDifference_Type;
INTDEF DeeTypeObject SetSymmetricDifferenceIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SET_H */
