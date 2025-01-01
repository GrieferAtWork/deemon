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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FLAT_H
#define GUARD_DEEMON_OBJECTS_SEQ_FLAT_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/util/lock.h>
/**/

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct { /* GC Object */
	PROXY_OBJECT_HEAD2(sfi_baseiter, /* [1..1][const] Iterator for the base sequence */
	                   sfi_curriter) /* [1..1][lock(sfi_currlock)] Iterator for the current sub-sequence */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t  sfi_currlock; /* Lock for `sfi_curriter' */
#endif /* !CONFIG_NO_THREADS */
} SeqFlatIterator;
#define SeqFlatIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->sfi_currlock)
#define SeqFlatIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->sfi_currlock)
#define SeqFlatIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->sfi_currlock)
#define SeqFlatIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->sfi_currlock)
#define SeqFlatIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->sfi_currlock)
#define SeqFlatIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->sfi_currlock)


typedef struct {
	PROXY_OBJECT_HEAD(sf_seq) /* [1..1][const] The function that is being flattened. */
} SeqFlat;

INTDEF DeeTypeObject SeqFlat_Type;
INTDEF DeeTypeObject SeqFlatIterator_Type;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Flat(DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FLAT_H */
