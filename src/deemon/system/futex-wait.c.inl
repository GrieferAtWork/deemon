/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "futex.c"
//#define DEFINE_DeeFutex_Wait32
#define DEFINE_DeeFutex_Wait64
//#define DEFINE_DeeFutex_Wait32Timed
//#define DEFINE_DeeFutex_Wait64Timed
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#if (defined(DEFINE_DeeFutex_Wait32) + defined(DEFINE_DeeFutex_Wait64) + \
     defined(DEFINE_DeeFutex_Wait32Timed) + defined(DEFINE_DeeFutex_Wait64Timed)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeFutex_Wait32
#define LOCAL_sizeof_expected 4
#define LOCAL_DeeFutex_WaitX  DeeFutex_Wait32
#elif defined(DEFINE_DeeFutex_Wait64)
#define LOCAL_sizeof_expected 8
#define LOCAL_DeeFutex_WaitX  DeeFutex_Wait64
#elif defined(DEFINE_DeeFutex_Wait32Timed)
#define LOCAL_sizeof_expected 4
#define LOCAL_HAVE_timeout_nanoseconds
#define LOCAL_DeeFutex_WaitX DeeFutex_Wait32Timed
#else /* ... */
#define LOCAL_sizeof_expected 8
#define LOCAL_HAVE_timeout_nanoseconds
#define LOCAL_DeeFutex_WaitX DeeFutex_Wait64Timed
#endif /* !... */

#if LOCAL_sizeof_expected == 4
#define LOCAL_expected_t           uint32_t
#define LOCAL_os_futex_waitX       os_futex_wait32
#define LOCAL_os_futex_waitX_timed os_futex_wait32_timed
#else /* LOCAL_sizeof_expected == 4 */
#define LOCAL_expected_t           uint64_t
#define LOCAL_os_futex_waitX       os_futex_wait64
#define LOCAL_os_futex_waitX_timed os_futex_wait64_timed
#endif /* LOCAL_sizeof_expected != 4 */

#ifndef NANOSECONDS_PER_SECOND
#define NANOSECONDS_PER_SECOND 1000000000
#endif /* !NANOSECONDS_PER_SECOND */

/* Blocking wait if `*(uint32_t *)addr == expected', until someone calls `DeeFutex_Wake*(addr)'
 * @return: 1 : [DeeFutex_Wait32Timed] The given `timeout_nanoseconds' expired.
 * @return: 0 : Success (someone called `DeeFutex_Wake*(addr)', or `*addr != expected', or spurious wake-up)
 * @return: -1: Error (an error was thrown) */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL LOCAL_DeeFutex_WaitX)(void *addr,
                             LOCAL_expected_t expected
#ifdef LOCAL_HAVE_timeout_nanoseconds
                             , uint64_t timeout_nanoseconds
#endif /* LOCAL_HAVE_timeout_nanoseconds */
                             )
{
#define LOCAL_should_wait() (*(LOCAL_expected_t const *)addr == expected)
#ifdef DeeFutex_USE_stub

	/************************************************************************/
	/* STUB IMPLEMENTATION                                                  */
	/************************************************************************/
	COMPILER_IMPURE();
	(void)addr;
	(void)expected;
#ifdef LOCAL_HAVE_timeout_nanoseconds
	(void)timeout_nanoseconds;
#endif /* LOCAL_HAVE_timeout_nanoseconds */
	return 0;

#elif defined(DeeFutex_USE_yield)

	/************************************************************************/
	/* FALLBACK YIELD IMPLEMENTATION                                        */
	/************************************************************************/
	(void)addr;
	(void)expected;
#ifdef LOCAL_HAVE_timeout_nanoseconds
	(void)timeout_nanoseconds;
#endif /* LOCAL_HAVE_timeout_nanoseconds */
	int result = DeeThread_CheckInterrupt();
	if likely(result == 0)
		SCHED_YIELD();
	return result;

#elif (defined(DeeFutex_USE_os_futex) || \
       (defined(DeeFutex_USE_os_futex_32_only) && LOCAL_sizeof_expected == 4))

	/************************************************************************/
	/* LINUX FUTEX IMPLEMENTATION                                           */
	/************************************************************************/
	int error;
#if defined(EINTR) || defined(ENOMEM)
again_futex_wait:
#endif /* EINTR || ENOMEM */
	os_futex_wait_begin(addr) {

		/* Check for interrupts _while_ our thread is registered as being inside of a futex operation. */
		if (DeeThread_CheckInterrupt()) {
			os_futex_wait_break();
			(void)os_futex_wakeall(addr);
			return -1;
		}
		
		/* Do the actual futex operation */
#ifdef LOCAL_HAVE_timeout_nanoseconds
		error = LOCAL_os_futex_waitX_timed(addr, expected, timeout_nanoseconds);
#else /* LOCAL_HAVE_timeout_nanoseconds */
		error = LOCAL_os_futex_waitX(addr, expected);
#endif /* !LOCAL_HAVE_timeout_nanoseconds */
	}
	os_futex_wait_end();

	/* Handle system errors */
	if (error < 0) {
		error = DeeSystem_GetErrno();
		DeeSystem_IF_E1(error, EINTR, goto again_futex_wait);
#ifdef LOCAL_HAVE_timeout_nanoseconds
		DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
			return 1;
		});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_futex_wait;
			return -1;
		});
		return DeeUnixSystem_ThrowErrorf(NULL, error, "Futex wait operation failed");
	}
	return 0;

#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)

	/************************************************************************/
	/* WINDOWS IMPLEMENTATION                                               */
	/************************************************************************/
#ifdef LOCAL_HAVE_timeout_nanoseconds
#define LOCAL_dwTimeout dwTimeout
	DWORD dwTimeout;
	if (OVERFLOW_UCAST(timeout_nanoseconds / 1000, &dwTimeout))
		dwTimeout = INFINITE;
#else /* LOCAL_HAVE_timeout_nanoseconds */
#define LOCAL_dwTimeout INFINITE
#endif /* !LOCAL_HAVE_timeout_nanoseconds */

again_switch_nt_futex_implementation:
	switch (nt_futex_implementation) {

	case NT_FUTEX_IMPLEMENTATION_UNINITIALIZED:
		nt_futex_initialize_subsystem();
		goto again_switch_nt_futex_implementation;

	case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS: {
		BOOL bOK;
again_futex_wait:
		os_futex_wait_begin(addr) {
			/* Check for interrupts _while_ our thread is registered as being inside of a futex operation. */
			if (DeeThread_CheckInterrupt()) {
				os_futex_wait_break();
				(void)WakeByAddressAll(addr);
				return -1;
			}
		
			bOK = WaitOnAddress(addr, &expected, LOCAL_sizeof_expected, LOCAL_dwTimeout);
		}
		os_futex_wait_end();
		if (!bOK) {
			DWORD dwError = GetLastError();
#ifdef LOCAL_HAVE_timeout_nanoseconds
			if (dwError == ERROR_TIMEOUT)
				return 1;
#endif /* LOCAL_HAVE_timeout_nanoseconds */
			if (DeeNTSystem_IsIntr(dwError))
				goto again_futex_wait;
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again_futex_wait;
				return -1;
			}
			return DeeNTSystem_ThrowErrorf(NULL, dwError, "WaitOnAddress failed");
		}
		return 0;
	}	break;

	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT: {
		BOOL bCondOk;
		DREF struct futex_controller *ctrl;
		ctrl = futex_ataddr_create((uintptr_t)addr);
		if unlikely(!ctrl)
			return -1;

again_call_AcquireSRWLockShared:
		AcquireSRWLockShared(&ctrl->fc_nt_cond_crit.cc_lock);
		/* Check for interrupts _while_ we're holding the SRW-lock */
		if (DeeThread_CheckInterrupt()) {
			ReleaseSRWLockShared(&ctrl->fc_nt_cond_crit.cc_lock);
			WakeAllConditionVariable(&ctrl->fc_nt_cond_crit.cc_cond);
			futex_controller_decref(ctrl);
			return -1;
		}

		/* Check the condition which we're supposed to wait on. */
		if (!LOCAL_should_wait()) {
			ReleaseSRWLockShared(&ctrl->fc_nt_cond_crit.cc_lock);
			futex_controller_decref(ctrl);
			return 0;
		}
#ifndef CONDITION_VARIABLE_LOCKMODE_SHARED
#define CONDITION_VARIABLE_LOCKMODE_SHARED 0x1
#endif /* !CONDITION_VARIABLE_LOCKMODE_SHARED */
		bCondOk = SleepConditionVariableSRW(&ctrl->fc_nt_cond_crit.cc_cond,
		                                    &ctrl->fc_nt_cond_crit.cc_lock,
		                                    LOCAL_dwTimeout,
		                                    CONDITION_VARIABLE_LOCKMODE_SHARED);
		ReleaseSRWLockShared(&ctrl->fc_nt_cond_crit.cc_lock);

		if (!bCondOk) {
			DWORD dwError = GetLastError();
#ifdef LOCAL_HAVE_timeout_nanoseconds
			if (dwError == ERROR_TIMEOUT) {
				futex_controller_decref(ctrl);
				return 1;
			}
#endif /* LOCAL_HAVE_timeout_nanoseconds */
			if (DeeNTSystem_IsIntr(dwError)) {
				futex_controller_decref(ctrl);
				goto again_call_AcquireSRWLockShared;
			}
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again_call_AcquireSRWLockShared;
				futex_controller_decref(ctrl);
				return -1;
			}
			futex_controller_decref(ctrl);
			return DeeNTSystem_ThrowErrorf(NULL, dwError, "SleepConditionVariableSRW failed");
		}
		futex_controller_decref(ctrl);
		return 0;
	}	break;

	case NT_FUTEX_IMPLEMENTATION_SEMAPHORE: {
		DWORD dwWaitStatus;
		DREF struct futex_controller *ctrl;
		ctrl = futex_ataddr_create((uintptr_t)addr);
		if unlikely(!ctrl)
			return -1;
again_call_inc_dwThreads:
		atomic_inc(&ctrl->fc_nt_sem.sm_dwThreads);

		/* Check for interrupts _while_ we're registered as a receiver */
		if (DeeThread_CheckInterrupt()) {
			atomic_dec(&ctrl->fc_nt_sem.sm_dwThreads);
			futex_controller_decref(ctrl);
			return -1;
		}

		/* Check the condition which we're supposed to wait on. */
		if (!LOCAL_should_wait()) {
			atomic_dec(&ctrl->fc_nt_sem.sm_dwThreads);
			futex_controller_decref(ctrl);
			return 0;
		}
		dwWaitStatus = WaitForSingleObjectEx(ctrl->fc_nt_sem.sm_hSemaphore,
		                                     LOCAL_dwTimeout, TRUE);
		atomic_dec(&ctrl->fc_nt_sem.sm_dwThreads);

		switch (dwWaitStatus) {

		case WAIT_IO_COMPLETION:
			futex_controller_decref(ctrl);
			goto again_call_inc_dwThreads;

#ifdef LOCAL_HAVE_timeout_nanoseconds
		case WAIT_TIMEOUT:
			futex_controller_decref(ctrl);
			return 1;
#endif /* LOCAL_HAVE_timeout_nanoseconds */

		case WAIT_FAILED: {
			DWORD dwError = GetLastError();
			if (DeeNTSystem_IsIntr(dwError)) {
				futex_controller_decref(ctrl);
				goto again_call_inc_dwThreads;
			}
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again_call_inc_dwThreads;
				futex_controller_decref(ctrl);
				return -1;
			}
			futex_controller_decref(ctrl);
			return DeeNTSystem_ThrowErrorf(NULL, dwError, "WaitForSingleObjectEx failed");
		}	break;

		default:
			break;
		}
		futex_controller_decref(ctrl);
		return 0;
	}	break;

	default: __builtin_unreachable();
	}
	__builtin_unreachable();
#undef LOCAL_dwTimeout

#elif defined(DeeFutex_USES_CONTROL_STRUCTURE)

	/************************************************************************/
	/* CONTROL-STRUCTURE-BASED IMPLEMENTATION                               */
	/************************************************************************/
#ifdef DeeFutex_USE_os_futex_32_only
	uint32_t ctrl_word;
	int error;
#endif /* DeeFutex_USE_os_futex_32_only */
#ifdef DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t
	int error;
#endif /* DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t */
#ifdef DeeFutex_USE_cnd_t_AND_mtx_t
	int error;
#endif /* DeeFutex_USE_cnd_t_AND_mtx_t */
#ifdef DeeFutex_USE_sem_t
	int error;
#endif /* DeeFutex_USE_sem_t */
	DREF struct futex_controller *ctrl;
	ctrl = futex_ataddr_create((uintptr_t)addr);
	if unlikely(!ctrl)
		return -1;

#ifdef DeeFutex_USE_os_futex_32_only
#if defined(EINTR) || defined(ENOMEM)
again_read_ctrl_word:
#endif /* EINTR || ENOMEM */
	ctrl_word = atomic_read(&ctrl->fc_word);
	/* NOTE: No need to set-up a OS futex wait list entry. Because we're using the
	 *       control word of the futex controller for synchronization, an interrupting
	 *       thread is able to just increment that word in order to force a sporadic
	 *       wake-up.
	 * However, that is also the reason why the read from `fc_word' _MUST_ happen
	 * *before* we check if our thread got interrupted (though if we get interrupted
	 * *after* having checked for that, we'll still get re-awoken as a result of the
	 * sender incrementing `fc_word')! */

	/* Check for interrupts _while_ our thread is registered as being inside of a futex operation. */
	if (DeeThread_CheckInterrupt()) {
		(void)os_futex_wakeall(&ctrl->fc_word);
		futex_controller_decref(ctrl);
		return -1;
	}

	/* Check if we're actually supposed to wait */
	if (!LOCAL_should_wait()) {
		futex_controller_decref(ctrl);
		return 0;
	}
	
	/* Do the actual futex operation */
#ifdef LOCAL_HAVE_timeout_nanoseconds
	error = os_futex_wait32_timed(&ctrl->fc_word, ctrl_word, timeout_nanoseconds);
#else /* LOCAL_HAVE_timeout_nanoseconds */
	error = os_futex_wait32(&ctrl->fc_word, ctrl_word);
#endif /* !LOCAL_HAVE_timeout_nanoseconds */

	/* Handle system errors */
	if (error < 0) {
		error = DeeSystem_GetErrno();
		DeeSystem_IF_E1(error, EINTR, {
			futex_controller_decref(ctrl);
			goto again_read_ctrl_word;
		});
#ifdef LOCAL_HAVE_timeout_nanoseconds
		DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
			futex_controller_decref(ctrl);
			return 1;
		});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_read_ctrl_word;
			futex_controller_decref(ctrl);
			return -1;
		});
		futex_controller_decref(ctrl);
		return DeeUnixSystem_ThrowErrorf(NULL, error, "Futex wait operation failed");
	}
#endif /* DeeFutex_USE_os_futex_32_only */

#ifdef DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t
#if defined(EINTR) || defined(ENOMEM)
again_pthread_mutex_lock:
#endif /* EINTR || ENOMEM */
	(void)pthread_mutex_lock(&ctrl->fc_mutx);

	/* Check for interrupts _while_ we're holding a lock to `fc_mutx'. */
	if (DeeThread_CheckInterrupt()) {
		(void)pthread_mutex_unlock(&ctrl->fc_mutx);
		(void)pthread_cond_broadcast(&ctrl->fc_cond);
		futex_controller_decref(ctrl);
		return -1;
	}

	/* Check if we're actually supposed to wait */
	if (!LOCAL_should_wait()) {
		futex_controller_decref(ctrl);
		return 0;
	}
	
#ifdef LOCAL_HAVE_timeout_nanoseconds
	if (timeout_nanoseconds != (uint64_t)-1) {
#ifdef CONFIG_HAVE_pthread_cond_reltimedwait64_np
		struct timespec64 ts;
		ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
		error = pthread_cond_reltimedwait64_np(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
#elif defined(CONFIG_HAVE_pthread_cond_reltimedwait_np)
		struct timespec ts;
		ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
		error = pthread_cond_reltimedwait_np(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
#elif defined(CONFIG_HAVE_gettimeofday64) && defined(CONFIG_HAVE_pthread_cond_timedwait64)
		struct timespec64 ts;
		error = gettimeofday64(NULL, &ts);
		if unlikely(error != 0) {
			error = DeeSystem_GetErrno();
		} else {
			ts.tv_sec  += timeout_nanoseconds / NANOSECONDS_PER_SECOND;
			ts.tv_nsec += timeout_nanoseconds % NANOSECONDS_PER_SECOND;
			if (ts.tv_nsec > NANOSECONDS_PER_SECOND) {
				++ts.tv_sec;
				ts.tv_nsec -= NANOSECONDS_PER_SECOND;
			}
			error = pthread_cond_timedwait64(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
		}
#else /* ... */
		struct timespec ts;
		error = gettimeofday(NULL, &ts);
		if unlikely(error != 0) {
			error = DeeSystem_GetErrno();
		} else {
			ts.tv_sec  += timeout_nanoseconds / NANOSECONDS_PER_SECOND;
			ts.tv_nsec += timeout_nanoseconds % NANOSECONDS_PER_SECOND;
			if (ts.tv_nsec > NANOSECONDS_PER_SECOND) {
				++ts.tv_sec;
				ts.tv_nsec -= NANOSECONDS_PER_SECOND;
			}
			error = pthread_cond_timedwait(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
		}
#endif /* !... */
	} else
#endif /* LOCAL_HAVE_timeout_nanoseconds */
	{
		error = pthread_cond_wait(&ctrl->fc_cond, &ctrl->fc_mutx);
	}
	(void)pthread_mutex_unlock(&ctrl->fc_mutx);

	/* Handle system errors */
	if (error != 0) {
		DeeSystem_IF_E1(error, EINTR, {
			futex_controller_decref(ctrl);
			goto again_pthread_mutex_lock;
		});
#ifdef LOCAL_HAVE_timeout_nanoseconds
		DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
			futex_controller_decref(ctrl);
			return 1;
		});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_pthread_mutex_lock;
			futex_controller_decref(ctrl);
			return -1;
		});
		futex_controller_decref(ctrl);
		return DeeUnixSystem_ThrowErrorf(NULL, error, "pthread_cond_wait failed");
	}
#endif /* DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t */

#ifdef DeeFutex_USE_cnd_t_AND_mtx_t
#if defined(CONFIG_HAVE_thrd_nomem) || (defined(CONFIG_HAVE_thrd_error) && (defined(EINTR) || defined(ENOMEM)))
again_mtx_lock:
#endif /* CONFIG_HAVE_thrd_nomem || (CONFIG_HAVE_thrd_error && (EINTR || ENOMEM)) */
	(void)mtx_lock(&ctrl->fc_mutx);

	/* Check for interrupts _while_ we're holding a lock to `fc_mutx'. */
	if (DeeThread_CheckInterrupt()) {
		(void)mtx_unlock(&ctrl->fc_mutx);
		(void)cnd_broadcast(&ctrl->fc_cond);
		futex_controller_decref(ctrl);
		return -1;
	}

	/* Check if we're actually supposed to wait */
	if (!LOCAL_should_wait()) {
		futex_controller_decref(ctrl);
		return 0;
	}
	
#ifdef LOCAL_HAVE_timeout_nanoseconds
	if (timeout_nanoseconds != (uint64_t)-1) {
#ifdef CONFIG_HAVE_cnd_reltimedwait64_np
		struct timespec64 ts;
		ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
		error = cnd_reltimedwait64_np(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
#elif defined(CONFIG_HAVE_cnd_reltimedwait_np)
		struct timespec ts;
		ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
		error = cnd_reltimedwait_np(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
#elif defined(CONFIG_HAVE_gettimeofday64) && defined(CONFIG_HAVE_cnd_timedwait64)
		struct timespec64 ts;
		error = gettimeofday64(NULL, &ts);
		if unlikely(error != 0) {
#ifdef CONFIG_HAVE_thrd_error
			error = thrd_error;
#else /* CONFIG_HAVE_thrd_error */
			error = thrd_success + 1;
#endif /* !CONFIG_HAVE_thrd_error */
		} else {
			ts.tv_sec  += timeout_nanoseconds / NANOSECONDS_PER_SECOND;
			ts.tv_nsec += timeout_nanoseconds % NANOSECONDS_PER_SECOND;
			if (ts.tv_nsec > NANOSECONDS_PER_SECOND) {
				++ts.tv_sec;
				ts.tv_nsec -= NANOSECONDS_PER_SECOND;
			}
			error = cnd_timedwait64(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
		}
#else /* ... */
		struct timespec ts;
		error = gettimeofday(NULL, &ts);
		if unlikely(error != 0) {
#ifdef CONFIG_HAVE_thrd_error
			error = thrd_error;
#else /* CONFIG_HAVE_thrd_error */
			error = thrd_success + 1;
#endif /* !CONFIG_HAVE_thrd_error */
		} else {
			ts.tv_sec  += timeout_nanoseconds / NANOSECONDS_PER_SECOND;
			ts.tv_nsec += timeout_nanoseconds % NANOSECONDS_PER_SECOND;
			if (ts.tv_nsec > NANOSECONDS_PER_SECOND) {
				++ts.tv_sec;
				ts.tv_nsec -= NANOSECONDS_PER_SECOND;
			}
			error = cnd_timedwait(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
		}
#endif /* !... */
	} else
#endif /* LOCAL_HAVE_timeout_nanoseconds */
	{
		error = cnd_wait(&ctrl->fc_cond, &ctrl->fc_mutx);
	}
	(void)mtx_unlock(&ctrl->fc_mutx);

	/* Handle system errors */
	if (error != thrd_success) {
#ifdef LOCAL_HAVE_timeout_nanoseconds
		if (error == thrd_timedout) {
			futex_controller_decref(ctrl);
			return 1;
		}
#endif /* LOCAL_HAVE_timeout_nanoseconds */
#ifdef CONFIG_HAVE_thrd_nomem
		if (error == thrd_nomem) {
			if (Dee_CollectMemory(1))
				goto again_mtx_lock;
			futex_controller_decref(ctrl);
			return -1;
		}
#endif /* CONFIG_HAVE_thrd_nomem */
#ifdef CONFIG_HAVE_thrd_error
		if (error == thrd_error) {
			error = DeeSystem_GetErrno();
#ifdef EINTR
			if (error == EINTR)
				goto again_mtx_lock;
#endif /* EINTR */
#ifdef ENOMEM
			if (error == ENOMEM) {
				if (Dee_CollectMemory(1))
					goto again_mtx_lock;
				futex_controller_decref(ctrl);
				return -1;
			}
#endif /* ENOMEM */
		} else
#endif /* !CONFIG_HAVE_thrd_error */
		{
#ifdef EINVAL
			error = EINVAL;
#else /* EINVAL */
			error = 1;
#endif /* !EINVAL */
		}
		futex_controller_decref(ctrl);
		return DeeUnixSystem_ThrowErrorf(NULL, error, "cnd_wait failed");
	}
#endif /* DeeFutex_USE_cnd_t_AND_mtx_t */

#ifdef DeeFutex_USE_sem_t
#if defined(EINTR) || defined(ENOMEM)
again_inc_n_threads:
#endif /* EINTR || ENOMEM */
	atomic_inc(&ctrl->fc_n_threads);

	/* Check for interrupts _while_ we're registered as a receiver */
	if (DeeThread_CheckInterrupt()) {
		atomic_dec(&ctrl->fc_n_threads);
		futex_controller_decref(ctrl);
		return -1;
	}

	/* Check the condition which we're supposed to wait on. */
	if (!LOCAL_should_wait()) {
		atomic_dec(&ctrl->fc_n_threads);
		futex_controller_decref(ctrl);
		return 0;
	}

#ifdef LOCAL_HAVE_timeout_nanoseconds
	if (timeout_nanoseconds != (uint64_t)-1) {
#ifdef CONFIG_HAVE_sem_reltimedwait64_np
		struct timespec64 ts;
		ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
		error = sem_reltimedwait64_np(&ctrl->fc_sem, &ts);
#elif defined(CONFIG_HAVE_sem_reltimedwait_np)
		struct timespec ts;
		ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
		ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
		error = sem_reltimedwait_np(&ctrl->fc_sem, &ts);
#elif defined(CONFIG_HAVE_sem_timedwait64) && defined(CONFIG_HAVE_gettimeofday64)
		struct timespec64 ts;
		error = gettimeofday64(NULL, &ts);
		if likely(error == 0) {
			ts.tv_sec  += timeout_nanoseconds / NANOSECONDS_PER_SECOND;
			ts.tv_nsec += timeout_nanoseconds % NANOSECONDS_PER_SECOND;
			if (ts.tv_nsec > NANOSECONDS_PER_SECOND) {
				++ts.tv_sec;
				ts.tv_nsec -= NANOSECONDS_PER_SECOND;
			}
			error = sem_timedwait64(&ctrl->fc_sem, &ts);
		}
#else /* ... */
		struct timespec ts;
		error = gettimeofday(NULL, &ts);
		if likely(error == 0) {
			ts.tv_sec  += timeout_nanoseconds / NANOSECONDS_PER_SECOND;
			ts.tv_nsec += timeout_nanoseconds % NANOSECONDS_PER_SECOND;
			if (ts.tv_nsec > NANOSECONDS_PER_SECOND) {
				++ts.tv_sec;
				ts.tv_nsec -= NANOSECONDS_PER_SECOND;
			}
			error = sem_timedwait(&ctrl->fc_sem, &ts);
		}
#endif /* !... */
	} else
#endif /* LOCAL_HAVE_timeout_nanoseconds */
	{
		error = sem_wait(&ctrl->fc_sem);
	}
	atomic_dec(&ctrl->fc_n_threads);

	/* Handle system errors */
	if (error != 0) {
		error = DeeSystem_GetErrno();
		DeeSystem_IF_E1(error, EINTR, {
			futex_controller_decref(ctrl);
			goto again_inc_n_threads;
		});
#ifdef LOCAL_HAVE_timeout_nanoseconds
		DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
			futex_controller_decref(ctrl);
			return 1;
		});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_inc_n_threads;
			futex_controller_decref(ctrl);
			return -1;
		});
		futex_controller_decref(ctrl);
		return DeeUnixSystem_ThrowErrorf(NULL, error, "sem_wait failed");
	}
#endif /* DeeFutex_USE_sem_t */

	/* Cleanup and indicate to our caller that we received a wake-up */
	futex_controller_decref(ctrl);
	return 0;
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#undef LOCAL_should_wait
}

#undef LOCAL_os_futex_waitX_timed
#undef LOCAL_os_futex_waitX
#undef LOCAL_expected_t
#undef LOCAL_DeeFutex_WaitX
#undef LOCAL_HAVE_timeout_nanoseconds
#undef LOCAL_sizeof_expected

DECL_END

#undef DEFINE_DeeFutex_Wait32
#undef DEFINE_DeeFutex_Wait64
#undef DEFINE_DeeFutex_Wait32Timed
#undef DEFINE_DeeFutex_Wait64Timed