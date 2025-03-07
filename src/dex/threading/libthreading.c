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
#include <deemon/types.h>

DECL_BEGIN

INTDEF DeeCMethodObject libthreading_lockunion_all;

PRIVATE struct dex_symbol symbols[] = {
	/* Normal locks */
	{ "Lock", (DeeObject *)&DeeLock_Type, MODSYM_FREADONLY },
	{ "AtomicLock", (DeeObject *)&DeeAtomicLock_Type, MODSYM_FREADONLY },
	{ "SharedLock", (DeeObject *)&DeeSharedLock_Type, MODSYM_FREADONLY },
	{ "RAtomicLock", (DeeObject *)&DeeRAtomicLock_Type, MODSYM_FREADONLY },
	{ "RSharedLock", (DeeObject *)&DeeRSharedLock_Type, MODSYM_FREADONLY },

	/* Read/write locks */
	{ "RWLock", (DeeObject *)&DeeRWLock_Type, MODSYM_FREADONLY },
	{ "RWLockReadLock", (DeeObject *)&DeeRWLockReadLock_Type, MODSYM_FREADONLY },
	{ "RWLockWriteLock", (DeeObject *)&DeeRWLockWriteLock_Type, MODSYM_FREADONLY },
	{ "AtomicRWLock", (DeeObject *)&DeeAtomicRWLock_Type, MODSYM_FREADONLY },
	{ "SharedRWLock", (DeeObject *)&DeeSharedRWLock_Type, MODSYM_FREADONLY },
	{ "RAtomicRWLock", (DeeObject *)&DeeRAtomicRWLock_Type, MODSYM_FREADONLY },
	{ "RSharedRWLock", (DeeObject *)&DeeRSharedRWLock_Type, MODSYM_FREADONLY },

	/* LockUnion */
	{ "LockUnion", (DeeObject *)&DeeLockUnion_Type },
	{ "all", (DeeObject *)&libthreading_lockunion_all, MODSYM_FREADONLY,
	  DOC("(locks!:?GLock)->?GLock\n"
	      "#tValueError{No @locks specified (a lock union must contain at least 1 lock)}"
	      "Return a ?GLockUnion for all of the given @locks, or re-returns ${locks.first} "
	      /**/ "when only a single lock was given\n"
	      "Lock unions can be used to (safely) acquire multiple locks at the same time, whilst "
	      /**/ "ensuring that doing so doesn't result in a dead-lock (as would normally be the "
	      /**/ "case when 2 threads acquire multiple locks at the same time, but not in the same "
	      /**/ "order). For more details on how this is done, see ?GLockUnion") },

	/* Semaphore */
	{ "Semaphore", (DeeObject *)&DeeSemaphore_Type, MODSYM_FREADONLY },

	/* Event */
	{ "Event", (DeeObject *)&DeeEvent_Type, MODSYM_FREADONLY },

	/* Once */
	{ "Once", (DeeObject *)&DeeOnce_Type, MODSYM_FREADONLY },

	/* ThreadLocalStorage */
	{ "TLS", (DeeObject *)&DeeTLS_Type, MODSYM_FREADONLY },
	{ NULL }
};



#ifndef CONFIG_NO_THREADS
PRIVATE struct tls_callback_hooks orig_hooks;

PRIVATE WUNUSED NONNULL((1)) int DCALL
libthreading_init(DeeDexObject *__restrict UNUSED(self)) {
	/* Install our custom TLS callback hooks. */
	memcpy(&orig_hooks, &_DeeThread_TlsCallbacks, sizeof(struct tls_callback_hooks));
	_DeeThread_TlsCallbacks.tc_fini = (void(DCALL *)(void *__restrict))&thread_tls_fini;
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
libthreading_fini(DeeDexObject *__restrict UNUSED(self)) {
	/* Restore the original TLS callback hooks. */
	memcpy(&_DeeThread_TlsCallbacks, &orig_hooks, sizeof(struct tls_callback_hooks));
}

#endif /* !CONFIG_NO_THREADS */


PUBLIC struct dex DEX = {
	/* .d_symbols      = */ symbols,
#ifndef CONFIG_NO_THREADS
	/* .d_init         = */ &libthreading_init,
	/* .d_fini         = */ &libthreading_fini
#endif /* !CONFIG_NO_THREADS */
};

DECL_END

#endif /* !GUARD_DEX_THREADING_LIBTHREADING_C */
