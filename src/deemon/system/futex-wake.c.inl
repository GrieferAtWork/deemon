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
#ifdef __INTELLISENSE__
#include "futex.c"
#define DEFINE_DeeFutex_WakeOne
//#define DEFINE_DeeFutex_WakeAll
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#if (defined(DEFINE_DeeFutex_WakeOne) + defined(DEFINE_DeeFutex_WakeAll)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeFutex_WakeOne
#define LOCAL_IS_ONE
#define LOCAL_DeeFutex_Wake DeeFutex_WakeOne
#else /* DEFINE_DeeFutex_WakeOne */
#define LOCAL_IS_ALL
#define LOCAL_DeeFutex_Wake DeeFutex_WakeAll
#endif /* !DEFINE_DeeFutex_WakeOne */

/* Select local wake-up functions */
#ifdef LOCAL_IS_ONE
#define LOCAL_os_futex_wake         os_futex_wakeone
#define LOCAL_pthread_cond_signal   pthread_cond_signal
#define LOCAL_cnd_signal            cnd_signal
#define LOCAL_WakeByAddress         WakeByAddressSingle
#define LOCAL_WakeConditionVariable WakeConditionVariable
#else /* LOCAL_IS_ONE */
#define LOCAL_os_futex_wake         os_futex_wakeall
#define LOCAL_pthread_cond_signal   pthread_cond_broadcast
#define LOCAL_cnd_signal            cnd_broadcast
#define LOCAL_WakeByAddress         WakeByAddressAll
#define LOCAL_WakeConditionVariable WakeAllConditionVariable
#endif /* !LOCAL_IS_ONE */



/* Wake up 1, or all waiting threads at a given address. */
PUBLIC NONNULL((1)) void
(DCALL LOCAL_DeeFutex_Wake)(void *addr) {
#ifdef DeeFutex_USE_os_futex
	(void)LOCAL_os_futex_wake(addr);
#elif defined(DeeFutex_USE_STUB) || defined(DeeFutex_USE_yield)
	(void)addr;
	COMPILER_IMPURE();
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	DREF struct futex_controller *ctrl;
	switch (nt_futex_implementation) {

	case NT_FUTEX_IMPLEMENTATION_UNINITIALIZED:
		return; /* Not initialized -> no-one can be waiting *anywhere* */

	case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS:
		LOCAL_WakeByAddress(addr);
		return;

	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT: {
		ctrl = futex_ataddr_get((uintptr_t)addr);
		if (!ctrl)
			return; /* No-one is waiting here... */

		/* Must lock the mutex to ensure that no thread is between
		 * the point where it checks if it should wait, and actually
		 * starts waiting */
		AcquireSRWLockExclusive(&ctrl->fc_nt_cond_crit.cc_lock);
		LOCAL_WakeConditionVariable(&ctrl->fc_nt_cond_crit.cc_cond);
		ReleaseSRWLockExclusive(&ctrl->fc_nt_cond_crit.cc_lock);
	}	break;

	case NT_FUTEX_IMPLEMENTATION_SEMAPHORE: {
		DWORD dwWaitingThreads;
		ctrl = futex_ataddr_get((uintptr_t)addr);
		if (!ctrl)
			return; /* No-one is waiting here... */
		dwWaitingThreads = atomic_read(&ctrl->fc_nt_sem.sm_dwThreads);
		if likely(dwWaitingThreads > 0) {
#ifdef LOCAL_IS_ONE
			(void)ReleaseSemaphore(ctrl->fc_nt_sem.sm_hSemaphore, 1, NULL);
#else /* LOCAL_IS_ONE */
			(void)ReleaseSemaphore(ctrl->fc_nt_sem.sm_hSemaphore, dwWaitingThreads, NULL);
#endif /* !LOCAL_IS_ONE */
		}
	}	break;

	default: __builtin_unreachable();
	}

	/* Drop the reference we got from `futex_ataddr_get' */
	futex_controller_decref(ctrl);
#elif defined(DeeFutex_USES_CONTROL_STRUCTURE)
	DREF struct futex_controller *ctrl;
#ifdef DeeFutex_USE_os_futex_32_only
	/* In this impl, we also need to wake-up anyone listening on the original address.
	 * This is because only 8-byte wait operations make use of futex controllers, while
	 * 4-byte wait operations use the OS's native API.
	 *
	 * As such, we have to do extra work to ensure that both types of threads get woken. */
#ifdef LOCAL_IS_ONE
	if (LOCAL_os_futex_wake(addr) > 0) {
		/* Someone was woken up -> since we're only supposed to wake 1 thread, we don't
		 * have to go through all of the hassle of checking for futex controllers! */
		return;
	}
#else /* LOCAL_IS_ONE */
	(void)LOCAL_os_futex_wake(addr);
#endif /* !LOCAL_IS_ONE */
#endif /* DeeFutex_USE_os_futex_32_only */

	ctrl = futex_ataddr_get((uintptr_t)addr);
	if (!ctrl)
		return; /* No-one is waiting here... */

	/* Wake up threads that are waiting on this controller. */
#ifdef DeeFutex_USE_os_futex_32_only
	atomic_inc(&ctrl->fc_word);
	(void)LOCAL_os_futex_wake(&ctrl->fc_word);
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)

	/* Must lock the mutex to ensure that no thread is between
	 * the point where it checks if it should wait, and actually
	 * starts waiting */
	(void)pthread_mutex_lock(&ctrl->fc_mutx);
	(void)LOCAL_pthread_cond_signal(&ctrl->fc_cond);
	(void)pthread_mutex_unlock(&ctrl->fc_mutx);

#elif defined(DeeFutex_USE_cnd_t_AND_mtx_t)

	/* Must lock the mutex to ensure that no thread is between
	 * the point where it checks if it should wait, and actually
	 * starts waiting */
	(void)mtx_lock(&ctrl->fc_mutx);
	(void)LOCAL_cnd_signal(&ctrl->fc_cond);
	(void)mtx_unlock(&ctrl->fc_mutx);
#elif defined(DeeFutex_USE_sem_t)
	{
		size_t n_threads;
		n_threads = atomic_read(&ctrl->fc_n_threads);
#ifdef LOCAL_IS_ONE
		if (n_threads > 0)
			(void)sem_post(&ctrl->fc_sem);
#else /* LOCAL_IS_ONE */
		/* We have to wake at least `n_threads' that may be waiting.
		 * We don't have to worry about any extra threads that might
		 * appear while we're waking existing threads, since this is
		 * only about waking threads that were there at the time of
		 * the signal being delivered.
		 * 
		 * We also don't have to worry about any extra pending thread
		 * wake-ups remaining after we're doing waking threads, since
		 * 1: Futex controllers are constantly finalized and thrown
		 *    away, so extra pending tickets in the semaphore will
		 *    probably just get reset
		 * 2: Those tickets that don't get reset will just result in
		 *    some receiving thread to be woken up sporadically,
		 *    which is also fine. */
#ifdef CONFIG_HAVE_sem_post_multiple
		(void)sem_post_multiple(&ctrl->fc_sem, n_threads);
#else /* CONFIG_HAVE_sem_post_multiple */
		while (n_threads > 0) {
			(void)sem_post(&ctrl->fc_sem);
			--n_threads;
		}
#endif /* !CONFIG_HAVE_sem_post_multiple */
#endif /* !LOCAL_IS_ONE */
	}
#endif /* ... */

	/* Drop the reference we got from `futex_ataddr_get' */
	futex_controller_decref(ctrl);
#endif /* !... */
}

#undef LOCAL_os_futex_wake
#undef LOCAL_pthread_cond_signal
#undef LOCAL_cnd_signal
#undef LOCAL_WakeByAddress
#undef LOCAL_WakeConditionVariable

#undef LOCAL_IS_ALL
#undef LOCAL_IS_ONE
#undef LOCAL_DeeFutex_Wake

DECL_END

#undef DEFINE_DeeFutex_WakeOne
#undef DEFINE_DeeFutex_WakeAll
