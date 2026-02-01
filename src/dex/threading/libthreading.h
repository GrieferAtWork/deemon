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
#ifndef GUARD_DEX_THREADING_LIBTHREADING_H
#define GUARD_DEX_THREADING_LIBTHREADING_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Lock objects */
INTDEF DeeTypeObject DeeLock_Type;
INTDEF DeeTypeObject DeeAtomicLock_Type;
INTDEF DeeTypeObject DeeSharedLock_Type;
INTDEF DeeTypeObject DeeRLock_Type;
INTDEF DeeTypeObject DeeRAtomicLock_Type;
INTDEF DeeTypeObject DeeRSharedLock_Type;

/* RWLock objects */
INTDEF DeeTypeObject DeeRWLock_Type;
INTDEF DeeTypeObject DeeRWLockReadLock_Type;
INTDEF DeeTypeObject DeeRWLockWriteLock_Type;
INTDEF DeeTypeObject DeeAtomicRWLock_Type;
INTDEF DeeTypeObject DeeAtomicRWLockReadLock_Type;
INTDEF DeeTypeObject DeeAtomicRWLockWriteLock_Type;
INTDEF DeeTypeObject DeeSharedRWLock_Type;
INTDEF DeeTypeObject DeeSharedRWLockReadLock_Type;
INTDEF DeeTypeObject DeeSharedRWLockWriteLock_Type;
INTDEF DeeTypeObject DeeRAtomicRWLock_Type;
INTDEF DeeTypeObject DeeRAtomicRWLockReadLock_Type;
INTDEF DeeTypeObject DeeRAtomicRWLockWriteLock_Type;
INTDEF DeeTypeObject DeeRSharedRWLock_Type;
INTDEF DeeTypeObject DeeRSharedRWLockReadLock_Type;
INTDEF DeeTypeObject DeeRSharedRWLockWriteLock_Type;

/* Semaphore */
INTDEF DeeTypeObject DeeSemaphore_Type;

/* Event */
INTDEF DeeTypeObject DeeEvent_Type;

/* Once */
INTDEF DeeTypeObject DeeOnce_Type;

/* >> all(args...: Lock): Lock
 * Proxy lock which can be used to acquire multiple locks at the same time,
 * without running the risk of a dead-lock (assuming that the caller isn't
 * already holding at least one of the given locks)
 *
 * >> local a = AtomicLock();
 * >> local b = SharedLock();
 * >> local c = SharedRWLock();
 * >> with (all(a, b, c.writelock)) {
 * >>     ... // At this point, all 3 locks are held (and were acquired in a safe manner)
 * >> } */
INTDEF DeeTypeObject DeeLockUnion_Type;


#ifndef CONFIG_NO_THREADS
struct tls_descriptor {
	/* This is the actual data structure that is being pointed
	 * to by the `t_tlsdata' field of every existing thread once
	 * `libthreading' has been loaded. */
	size_t                                    td_size;  /* The amount of TLS instances allocated for this thread. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, td_elem); /* [0..1][td_size] Vector of TLS instances allocated for this thread.
	                                                     * NOTE: Individual items are set to ITER_DONE if the user
	                                                     *       deletes the following their factory initialization.
	                                                     *       NULL-values however are lazily allocated by the factory. */
};


/* TLS controller callbacks for libthreading's TLS implementation. */
INTDEF NONNULL((1)) void DCALL
thread_tls_fini(struct tls_descriptor *__restrict data);

#endif /* !CONFIG_NO_THREADS */


INTDEF DeeTypeObject DeeTLS_Type;

DECL_END

#endif /* !GUARD_DEX_THREADING_LIBTHREADING_H */
