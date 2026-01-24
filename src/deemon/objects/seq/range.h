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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_RANGE_H
#define GUARD_DEEMON_OBJECTS_SEQ_RANGE_H 1

#include <deemon/api.h>

#include <deemon/object.h>
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_* */

#include <stdbool.h> /* bool */

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	/* NOTE: Iteration stops only when `index >= ir_end'.
	 *       This is feasible because of our infinite-precision integer library,
	 *       meaning that user-code shouldn't need to worry about overflows in
	 *       standard arithmetical operations. */
	DREF DeeObject *r_start; /* [1..1][const] Starting index. */
	DREF DeeObject *r_end;   /* [1..1][const] Ending index. */
	DREF DeeObject *r_step;  /* [0..1][const] Step size (or NULL when `tp_inc()' should be used). */
	bool            r_rev;   /* [const] True if `r_step' is non-NULL and negative. */
} Range;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject     *ri_index; /* [1..1][lock(ri_lock)] The current index operated on using using `tp_inplace_add()' or `tp_inc()'. */
	DREF DeeObject     *ri_end;   /* [1..1][const][== ri_range->r_end] Ending index. */
	DREF DeeObject     *ri_step;  /* [0..1][const][== ri_range->r_step] Step size (or NULL when `tp_inc()' should be used). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t ri_lock;  /* Lock for synchronizing access to ri_index. */
#endif /* !CONFIG_NO_THREADS */
	bool                ri_first; /* [lock(ri_lock)] Only true during the first iteration to skip the initial modification. */
	bool                ri_rev;   /* [const][== ri_range->r_rev] True if `r_step' is non-NULL and negative. */
} RangeIterator;

#define RangeIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->ri_lock)
#define RangeIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->ri_lock)
#define RangeIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->ri_lock)
#define RangeIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->ri_lock)
#define RangeIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->ri_lock)
#define RangeIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->ri_lock)
#define RangeIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->ri_lock)
#define RangeIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->ri_lock)
#define RangeIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->ri_lock)
#define RangeIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->ri_lock)
#define RangeIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->ri_lock)
#define RangeIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->ri_lock)
#define RangeIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->ri_lock)
#define RangeIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->ri_lock)
#define RangeIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->ri_lock)
#define RangeIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->ri_lock)

INTDEF DeeTypeObject SeqRangeIterator_Type;
INTDEF DeeTypeObject SeqRange_Type;




typedef struct {
	OBJECT_HEAD
	/* NOTE: Iteration stops when `index >= ir_end' (ir_step > 0) / `index <= ir_end' (ir_step < 0) or
	 *      `index += ir_step' would roll over. (returning `ITER_DONE' immediately) */
	Dee_ssize_t ir_start; /* [const] Starting index. */
	Dee_ssize_t ir_end;   /* [const] Ending index. */
	Dee_ssize_t ir_step;  /* [const][!0] Step size (may be negative). */
} IntRange;

typedef struct {
	OBJECT_HEAD
	Dee_ssize_t iri_index; /* [atomic] The next index to yield. */
	Dee_ssize_t iri_end;   /* [weak(const)] Ending index. */
	Dee_ssize_t iri_step;  /* [weak(const)] Step size (may be negative). */
} IntRangeIterator;

INTDEF DeeTypeObject SeqIntRangeIterator_Type;
INTDEF DeeTypeObject SeqIntRange_Type;


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_RANGE_H */
