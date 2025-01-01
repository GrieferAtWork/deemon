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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_REPEAT_H
#define GUARD_DEEMON_OBJECTS_SEQ_REPEAT_H 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/lock.h>

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(rp_seq) /* [1..1][const] The sequence being repeated. */
	size_t            rp_num; /* [!0][const] The number of times by which to repeat the sequence. */
} Repeat;

typedef struct {
	PROXY_OBJECT_HEAD(rpit_obj) /* [1..1][const] The object being repeated. */
	size_t            rpit_num; /* [const] The number of times by which to repeat the object. */
} RepeatItem;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(Repeat,    rpi_rep,  /* [1..1][const] The underlying repeat-proxy-sequence. */
	                      DeeObject, rpi_iter) /* [1..1][lock(rpi_lock)] The current repeat-iterator. */
	size_t                           rpi_num;  /* [lock(rpi_lock)] The remaining number of times to repeat the sequence. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t              rpi_lock; /* Lock for accessing the variable fields above. */
#endif /* !CONFIG_NO_THREADS */
} RepeatIterator;

#define RepeatIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->rpi_lock)
#define RepeatIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->rpi_lock)
#define RepeatIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->rpi_lock)
#define RepeatIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->rpi_lock)
#define RepeatIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->rpi_lock)
#define RepeatIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->rpi_lock)
#define RepeatIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->rpi_lock)
#define RepeatIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->rpi_lock)
#define RepeatIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->rpi_lock)
#define RepeatIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->rpi_lock)
#define RepeatIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->rpi_lock)
#define RepeatIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->rpi_lock)
#define RepeatIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->rpi_lock)
#define RepeatIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->rpi_lock)
#define RepeatIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->rpi_lock)
#define RepeatIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->rpi_lock)

typedef struct {
	PROXY_OBJECT_HEAD_EX(RepeatItem, rii_rep); /* [1..1][const] The underlying repeat-proxy-sequence. */
	DeeObject                       *rii_obj;  /* [1..1][const][== rii_rep->rpit_obj] The object being repeated. */
	DWEAK size_t                     rii_num;  /* The remaining number of repetitions. */
} RepeatItemIterator;

INTDEF DeeTypeObject SeqRepeat_Type;
INTDEF DeeTypeObject SeqItemRepeat_Type;
INTDEF DeeTypeObject SeqRepeatIterator_Type;
INTDEF DeeTypeObject SeqItemRepeatIterator_Type;

/* Construct new repetition-proxy-sequence objects. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Repeat(DeeObject *__restrict self, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatItem(DeeObject *__restrict item, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatItemForever(DeeObject *__restrict item);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_REPEAT_H */
