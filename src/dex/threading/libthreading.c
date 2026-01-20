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
#ifndef GUARD_DEX_THREADING_LIBTHREADING_C
#define GUARD_DEX_THREADING_LIBTHREADING_C 1
#define CONFIG_BUILDING_LIBTHREADING
#define DEE_SOURCE

#include "libthreading.h"
/**/

#include <deemon/api.h>

#include <deemon/dex.h>
#include <deemon/module.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h> /* memcpy */
#include <deemon/thread.h>

#include <stddef.h> /* NULL */

DECL_BEGIN

INTDEF DeeCMethodObject libthreading_lockunion_all;

#ifndef CONFIG_NO_THREADS
PRIVATE struct tls_callback_hooks orig_hooks;

#define PTR_libthreading_init &libthreading_init
PRIVATE WUNUSED int DCALL libthreading_init(void) {
	/* Install our custom TLS callback hooks. */
	memcpy(&orig_hooks, &_DeeThread_TlsCallbacks, sizeof(struct tls_callback_hooks));
	_DeeThread_TlsCallbacks.tc_fini = (void(DCALL *)(void *__restrict))&thread_tls_fini;
	return 0;
}

#define PTR_libthreading_fini &libthreading_fini
PRIVATE void DCALL libthreading_fini(void) {
	/* Restore the original TLS callback hooks. */
	memcpy(&_DeeThread_TlsCallbacks, &orig_hooks, sizeof(struct tls_callback_hooks));
}
#endif /* !CONFIG_NO_THREADS */

DEX_BEGIN

/* Normal locks */
DEX_MEMBER_F_NODOC("Lock", &DeeLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("AtomicLock", &DeeAtomicLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("SharedLock", &DeeSharedLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("RAtomicLock", &DeeRAtomicLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("RSharedLock", &DeeRSharedLock_Type, MODSYM_FREADONLY),

/* Read/write locks */
DEX_MEMBER_F_NODOC("RWLock", &DeeRWLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("RWLockReadLock", &DeeRWLockReadLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("RWLockWriteLock", &DeeRWLockWriteLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("AtomicRWLock", &DeeAtomicRWLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("SharedRWLock", &DeeSharedRWLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("RAtomicRWLock", &DeeRAtomicRWLock_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("RSharedRWLock", &DeeRSharedRWLock_Type, MODSYM_FREADONLY),

/* LockUnion */
DEX_MEMBER_F_NODOC("LockUnion", &DeeLockUnion_Type, MODSYM_FREADONLY),
DEX_MEMBER_F("all", &libthreading_lockunion_all, MODSYM_FREADONLY,
             "(locks!:?GLock)->?GLock\n"
             "#tValueError{No @locks specified (a lock union must contain at least 1 lock)}"
             "Return a ?GLockUnion for all of the given @locks, or re-returns ${locks.first} "
             /**/ "when only a single lock was given\n"
             "Lock unions can be used to (safely) acquire multiple locks at the same time, whilst "
             /**/ "ensuring that doing so doesn't result in a dead-lock (as would normally be the "
             /**/ "case when 2 threads acquire multiple locks at the same time, but not in the same "
             /**/ "order). For more details on how this is done, see ?GLockUnion"),

/* Semaphore */
DEX_MEMBER_F_NODOC("Semaphore", &DeeSemaphore_Type, MODSYM_FREADONLY),

/* Event */
DEX_MEMBER_F_NODOC("Event", &DeeEvent_Type, MODSYM_FREADONLY),

/* Once */
DEX_MEMBER_F_NODOC("Once", &DeeOnce_Type, MODSYM_FREADONLY),

/* ThreadLocalStorage */
DEX_MEMBER_F_NODOC("TLS", &DeeTLS_Type, MODSYM_FREADONLY),

#ifndef PTR_libthreading_init
#define PTR_libthreading_init NULL
#endif /* !PTR_libthreading_init */
#ifndef PTR_libthreading_fini
#define PTR_libthreading_fini NULL
#endif /* !PTR_libthreading_fini */

/* clang-format off */
DEX_END(
	/* init:  */ PTR_libthreading_init,
	/* fini:  */ PTR_libthreading_fini,
	/* clear: */ NULL
);
/* clang-format on */

DECL_END

#endif /* !GUARD_DEX_THREADING_LIBTHREADING_C */
