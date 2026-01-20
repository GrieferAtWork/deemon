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
#ifdef __INTELLISENSE__
#include "futex.c"
//#define DEFINE_futex_ataddr_create
#define DEFINE_futex_ataddr_trycreate
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/system.h>      /* DeeNTSystem_ThrowErrorf, DeeUnixSystem_ThrowErrorf */
#include <deemon/types.h>
#include <deemon/util/atomic.h>

#include <hybrid/sequence/list.h>

#include <stddef.h> /* NULL */
#include <stdint.h> /* INT32_MAX, uintptr_t */

#ifndef INT32_MAX
#include <hybrid/limitcore.h>
#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#endif /* !INT32_MAX */

#if (defined(DEFINE_futex_ataddr_create) + \
     defined(DEFINE_futex_ataddr_trycreate)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DeeFutex_USES_CONTROL_STRUCTURE
DECL_BEGIN

#ifdef DEFINE_futex_ataddr_create
#define LOCAL_HAVE_EXCEPTIONS
#define LOCAL_futex_controller_alloc       futex_controller_alloc
#define LOCAL_futex_controller_do_new_impl futex_controller_do_new_impl
#define LOCAL_futex_controller_new         futex_controller_new
#define LOCAL_futex_ataddr_create          futex_ataddr_create
#elif defined(DEFINE_futex_ataddr_trycreate)
#undef LOCAL_HAVE_EXCEPTIONS
#define LOCAL_futex_controller_alloc       futex_controller_tryalloc
#define LOCAL_futex_controller_do_new_impl futex_controller_do_trynew_impl
#define LOCAL_futex_controller_new         futex_controller_trynew
#define LOCAL_futex_ataddr_create          futex_ataddr_trycreate
#else /* ... */
#error "Invalid configuration"
#endif /* !... */


/* Raw, low-level allocate a new futex controller. */
PRIVATE WUNUSED struct futex_controller *DCALL
LOCAL_futex_controller_do_new_impl(void) {
	DREF struct futex_controller *result;
	result = LOCAL_futex_controller_alloc();
	if unlikely(!result)
		goto err;

	/* Initialize the os-specific part of `result' */
#ifdef DeeFutex_USE_os_futex_32_only
	result->fc_word = 0;
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	switch (nt_futex_implementation) {
	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT:
		/* Neither CONDITION_VARIABLEs, nor SWRLOCKs have destructors. */
		InitializeConditionVariable(&result->fc_nt_cond_crit.cc_cond);
		InitializeSRWLock(&result->fc_nt_cond_crit.cc_lock);
		break;
	case NT_FUTEX_IMPLEMENTATION_SEMAPHORE: {
		HANDLE hSem = CreateSemaphoreW(NULL, 0, INT32_MAX, NULL);
		if unlikely(hSem == NULL || hSem == INVALID_HANDLE_VALUE) {
#ifdef LOCAL_HAVE_EXCEPTIONS
			DWORD dwError = GetLastError();
#endif /* LOCAL_HAVE_EXCEPTIONS */
			futex_controller_free(result);
#ifdef LOCAL_HAVE_EXCEPTIONS
			DeeNTSystem_ThrowErrorf(NULL, dwError, "Failed to allocate semaphore");
#endif /* LOCAL_HAVE_EXCEPTIONS */
			goto err;
		}
		result->fc_nt_sem.sm_hSemaphore = hSem;
		result->fc_nt_sem.sm_dwThreads  = 0;
	}	break;
	default: __builtin_unreachable();
	}
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)
	{
		int error;
		error = pthread_mutex_init(&result->fc_mutx, NULL);
		if likely(error == 0) {
			error = pthread_cond_init(&result->fc_cond, NULL);
			if unlikely(error != 0) {
				(void)pthread_mutex_destroy(&result->fc_mutx);
			}
		}
		if unlikely(error != 0) {
			futex_controller_free(result);
#ifdef LOCAL_HAVE_EXCEPTIONS
			DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to initialize mutex and condition variable");
#endif /* LOCAL_HAVE_EXCEPTIONS */
			goto err;
		}
	}
#elif defined(DeeFutex_USE_cnd_t_AND_mtx_t)
	{
		int error;
		error = mtx_init(&result->fc_mutx, mtx_plain);
		if likely(error == 0) {
			error = cnd_init(&result->fc_cond);
			if unlikely(error != 0) {
				(void)mtx_destroy(&result->fc_mutx);
			}
		}
		if unlikely(error != 0) {
			futex_controller_free(result);
#ifdef LOCAL_HAVE_EXCEPTIONS
#ifdef ENOMEM
			error = ENOMEM;
#else /* ENOMEM */
			error = 1;
#endif /* !ENOMEM */
			DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to initialize mutex and condition variable");
#endif /* LOCAL_HAVE_EXCEPTIONS */
			goto err;
		}
	}
#elif defined(DeeFutex_USE_sem_t)
	if unlikely(sem_init(&result->fc_sem) != 0) {
#ifdef LOCAL_HAVE_EXCEPTIONS
		int error = DeeSystem_GetErrno();
#endif /* LOCAL_HAVE_EXCEPTIONS */
		futex_controller_free(result);
#ifdef LOCAL_HAVE_EXCEPTIONS
		DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to initialize semaphore");
#endif /* LOCAL_HAVE_EXCEPTIONS */
		goto err;
	}
	result->fc_n_threads = 0;
#endif /* ... */

	return result;
err:
	return NULL;
}


/* Similar to `LOCAL_futex_controller_do_new_impl()', but try
 * to take a pre-existing object from the free-list, as
 * well as also set the reference counter to `1'.
 *
 * @return: * :   The new controller.
 * @return: NULL: Alloc failed (an error was thrown) */
PRIVATE WUNUSED DREF struct futex_controller *DCALL
LOCAL_futex_controller_new(void) {
	DREF struct futex_controller *result;
	if (atomic_read(&fcont_freesize) != 0) {
		fcont_lock_write();
		result = SLIST_FIRST(&fcont_freelist);
		if likely(result != NULL) {
			SLIST_REMOVE_HEAD(&fcont_freelist, fc_free);
			--fcont_freesize;
			fcont_lock_endwrite();
			goto set_refcnt;
		}
		fcont_lock_endwrite();
	}
	result = LOCAL_futex_controller_do_new_impl();
	if likely(result) {
set_refcnt:
		result->fc_refcnt = 1;
	}
	return result;
}

/* Lookup a futex controller at a given address,
 * or create one at said address if there wasn't
 * one there already.
 *
 * @return: * :   Reference to the controller at `addr'
 * @return: NULL: Failed to create a new controller (an error was thrown) */
PRIVATE WUNUSED DREF struct futex_controller *DCALL
LOCAL_futex_ataddr_create(uintptr_t addr) {
	DREF struct futex_controller *result;
	result = futex_ataddr_get(addr);
	if (result == NULL) {
		/* Must create a new controller. */
		result = LOCAL_futex_controller_new();
		if likely(result != NULL) {
			DREF struct futex_controller *existing_result;

			/* Initialize the new controller's address. */
			result->fc_addr = addr;

			/* Inject the new controller into the tree. */
			fcont_lock_write();
			existing_result = futex_tree_locate(fcont_tree, addr);
			if unlikely(existing_result) {
				if (futex_controller_tryincref(existing_result)) {
					/* Race condition: another thread created the controller before we could. */
					fcont_lock_endwrite();
					futex_controller_destroy(result);
					return existing_result;
				}

				/* There is a dead controller at our address.
				 * -> just remove it from the tree so we can move on. */
				futex_tree_removenode(&fcont_tree, existing_result);
			}

			/* Insert our newly created controller into the tree. */
			futex_tree_insert(&fcont_tree, result);
			fcont_lock_endwrite();
		}
	}
	return result;
}

#undef LOCAL_HAVE_EXCEPTIONS
#undef LOCAL_futex_controller_alloc
#undef LOCAL_futex_controller_do_new_impl
#undef LOCAL_futex_controller_new
#undef LOCAL_futex_ataddr_create

DECL_END
#endif /* DeeFutex_USES_CONTROL_STRUCTURE */

#undef DEFINE_futex_ataddr_trycreate
#undef DEFINE_futex_ataddr_create
