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
	COMPILER_IMPURE();
	(void)addr;
	(void)expected;
#ifdef LOCAL_HAVE_timeout_nanoseconds
	(void)timeout_nanoseconds;
#endif /* LOCAL_HAVE_timeout_nanoseconds */
	return 0;
#else /* DeeFutex_USE_stub */
	int result;
again:
	result = DeeThread_CheckInterrupt();
	if unlikely(result != 0) {
		/* Prevent dead-lock in case we got here after a race
		 * between `DeeFutex_WakeOne()' and `DeeThread_Interrupt()'
		 *
		 * -> Wake up anyone else that might be waiting on this futx. */
		DeeFutex_WakeAll(addr);
	} else {
#if (defined(DeeFutex_USE_os_futex) || \
     (defined(DeeFutex_USE_os_futex_32_only) && LOCAL_sizeof_expected == 4))
#ifdef LOCAL_HAVE_timeout_nanoseconds
		if (LOCAL_os_futex_waitX_timed(addr, expected, timeout_nanoseconds) < 0)
#else /* LOCAL_HAVE_timeout_nanoseconds */
		if (LOCAL_os_futex_waitX(addr, expected) < 0)
#endif /* !LOCAL_HAVE_timeout_nanoseconds */
		{
			int error = DeeSystem_GetErrno();
			DeeSystem_IF_E1(error, EINTR, goto again);
#ifdef LOCAL_HAVE_timeout_nanoseconds
			DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
				return 1;
			});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
			DeeSystem_IF_E1(error, ENOMEM, {
				if (Dee_CollectMemory(1))
					goto again;
				return -1;
			});
			return DeeUnixSystem_ThrowErrorf(NULL, error, "Futex wait operation failed");
		}
#elif defined(DeeFutex_USE_yield)
		(void)addr;
		(void)expected;
#ifdef LOCAL_HAVE_timeout_nanoseconds
		(void)timeout_nanoseconds;
#endif /* LOCAL_HAVE_timeout_nanoseconds */
		SCHED_YIELD();
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
#ifdef LOCAL_HAVE_timeout_nanoseconds
#define LOCAL_dwTimeout dwTimeout
		DWORD dwTimeout;
		if (OVERFLOW_UCAST(timeout_nanoseconds / 1000, &dwTimeout))
			dwTimeout = INFINITE;
#else /* LOCAL_HAVE_timeout_nanoseconds */
#define LOCAL_dwTimeout INFINITE
#endif /* !LOCAL_HAVE_timeout_nanoseconds */

		/* Switch based on chosen implementation */
again_switch_nt_futex_implementation:
		switch (nt_futex_implementation) {

		case NT_FUTEX_IMPLEMENTATION_UNINITIALIZED:
			nt_futex_initialize_subsystem();
			goto again_switch_nt_futex_implementation;

		case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS: {
			uint32_t word;
			DREF struct futex_controller *ctrl;
			ctrl = futex_ataddr_create((uintptr_t)addr);
			if unlikely(!ctrl)
				return -1;

			/* Do the wait using windows's system futex API */
again_call_WaitOnAddress:
			word = atomic_read(&ctrl->fc_nt_word);
			if (LOCAL_should_wait()) {
				if (!WaitOnAddress(&ctrl->fc_nt_word, &word, 4, LOCAL_dwTimeout)) {
					DWORD dwError = GetLastError();
#ifdef LOCAL_HAVE_timeout_nanoseconds
					if (dwError == ERROR_TIMEOUT) {
						futex_controller_decref(ctrl);
						return 1;
					}
#endif /* LOCAL_HAVE_timeout_nanoseconds */
					if (DeeNTSystem_IsIntr(dwError)) {
						futex_controller_decref(ctrl);
						goto again;
					}
					if (DeeNTSystem_IsBadAllocError(dwError)) {
						if (Dee_CollectMemory(1))
							goto again_call_WaitOnAddress;
						futex_controller_decref(ctrl);
						return -1;
					}
					futex_controller_decref(ctrl);
					return DeeNTSystem_ThrowErrorf(NULL, dwError, "WaitOnAddress failed");
				}
			}
			futex_controller_decref(ctrl);
		}	break;

		case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT: {
			BOOL bCondOk;
			DREF struct futex_controller *ctrl;
			ctrl = futex_ataddr_create((uintptr_t)addr);
			if unlikely(!ctrl)
				return -1;

again_call_AcquireSRWLockExclusive:
			AcquireSRWLockShared(&ctrl->fc_nt_cond_crit.cc_lock);
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
					goto again;
				}
				if (DeeNTSystem_IsBadAllocError(dwError)) {
					if (Dee_CollectMemory(1))
						goto again_call_AcquireSRWLockExclusive;
					futex_controller_decref(ctrl);
					return -1;
				}
				futex_controller_decref(ctrl);
				return DeeNTSystem_ThrowErrorf(NULL, dwError, "SleepConditionVariableSRW failed");
			}
			futex_controller_decref(ctrl);
		}	break;

		case NT_FUTEX_IMPLEMENTATION_SEMAPHORE: {
			DWORD dwWaitStatus;
			DREF struct futex_controller *ctrl;
			ctrl = futex_ataddr_create((uintptr_t)addr);
			if unlikely(!ctrl)
				return -1;
again_call_inc_dwThreads:
			atomic_inc(&ctrl->fc_nt_sem.sm_dwThreads);

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
				goto again;

#ifdef LOCAL_HAVE_timeout_nanoseconds
			case WAIT_TIMEOUT:
				futex_controller_decref(ctrl);
				return 1;
#endif /* LOCAL_HAVE_timeout_nanoseconds */

			case WAIT_FAILED: {
				DWORD dwError = GetLastError();
				if (DeeNTSystem_IsIntr(dwError)) {
					futex_controller_decref(ctrl);
					goto again;
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
		}	break;

		default: __builtin_unreachable();
		}
#undef LOCAL_dwTimeout
#elif defined(DeeFutex_USES_CONTROL_STRUCTURE)
		DREF struct futex_controller *ctrl;
		ctrl = futex_ataddr_create((uintptr_t)addr);
		if unlikely(!ctrl)
			return -1;

#ifdef DeeFutex_USE_os_futex_32_only
		{
			uint32_t ctrl_word;
again_read_ctrl_word:
			ctrl_word = atomic_read(&ctrl->fc_word);
			if (LOCAL_should_wait()) {
#ifdef LOCAL_HAVE_timeout_nanoseconds
				if (os_futex_wait32_timed(&ctrl->fc_word, ctrl_word, timeout_nanoseconds) < 0)
#else /* LOCAL_HAVE_timeout_nanoseconds */
				if (os_futex_wait32(&ctrl->fc_word, ctrl_word) < 0)
#endif /* !LOCAL_HAVE_timeout_nanoseconds */
				{
					int error = DeeSystem_GetErrno();
					DeeSystem_IF_E1(error, EINTR, {
						futex_controller_decref(ctrl);
						goto again;
					});
#ifdef LOCAL_HAVE_timeout_nanoseconds
					DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
						futex_controller_decref(ctrl);
						return 1;
					});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
					DeeSystem_IF_E1(error, ENOMEM, {
						if (Dee_CollectMemory(1))
							goto again;
						futex_controller_decref(ctrl);
						goto again_read_ctrl_word;
					});
					futex_controller_decref(ctrl);
					return DeeUnixSystem_ThrowErrorf(NULL, error, "Futex wait operation failed");
				}
			}
		}
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)
again_pthread_mutex_lock:
		(void)pthread_mutex_lock(&ctrl->fc_mutx);
		if (!LOCAL_should_wait()) {
			(void)pthread_mutex_unlock(&ctrl->fc_mutx);
		} else {
			int error;
#ifdef LOCAL_HAVE_timeout_nanoseconds
			struct timespec ts;
			if (timeout_nanoseconds != (uint64_t)-1) {
#ifdef CONFIG_HAVE_pthread_cond_reltimedwait_np
				ts.tv_sec  = timeout_nanoseconds / NANOSECONDS_PER_SECOND;
				ts.tv_nsec = timeout_nanoseconds % NANOSECONDS_PER_SECOND;
				error = pthread_cond_reltimedwait_np(&ctrl->fc_cond, &ctrl->fc_mutx, &ts);
#else /* CONFIG_HAVE_pthread_cond_reltimedwait_np */
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
#endif /* !CONFIG_HAVE_pthread_cond_reltimedwait_np */
			} else
#endif /* LOCAL_HAVE_timeout_nanoseconds */
			{
				error = pthread_cond_wait(&ctrl->fc_cond, &ctrl->fc_mutx);
			}
			(void)pthread_mutex_unlock(&ctrl->fc_mutx);
			if (error != 0) {
				DeeSystem_IF_E1(error, EINTR, {
					futex_controller_decref(ctrl);
					goto again;
				});
#ifdef LOCAL_HAVE_timeout_nanoseconds
				DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
					futex_controller_decref(ctrl);
					return 1;
				});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
				DeeSystem_IF_E1(error, ENOMEM, {
					if (Dee_CollectMemory(1))
						goto again;
					futex_controller_decref(ctrl);
					goto again_pthread_mutex_lock;
				});
				futex_controller_decref(ctrl);
				return DeeUnixSystem_ThrowErrorf(NULL, error, "pthread_cond_wait failed");
			}
		}
#elif defined(DeeFutex_USE_sem_t)
again_inc_n_threads:
		atomic_inc(&ctrl->fc_n_threads);

		/* Check the condition which we're supposed to wait on. */
		if (!LOCAL_should_wait()) {
			atomic_dec(&ctrl->fc_n_threads);
			futex_controller_decref(ctrl);
			return 0;
		}
		{
			int error;
#ifdef LOCAL_HAVE_timeout_nanoseconds
			if (timeout_nanoseconds != (uint64_t)-1) {
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
			} else
#endif /* LOCAL_HAVE_timeout_nanoseconds */
			{
				error = sem_wait(&ctrl->fc_sem);
			}
			atomic_dec(&ctrl->fc_n_threads);
			if (error != 0) {
				error = DeeSystem_GetErrno();
				DeeSystem_IF_E1(error, EINTR, {
					futex_controller_decref(ctrl);
					goto again;
				});
#ifdef LOCAL_HAVE_timeout_nanoseconds
				DeeSystem_IF_E3(error, ETIMEDOUT, EAGAIN, EWOULDBLOCK, {
					futex_controller_decref(ctrl);
					return 1;
				});
#endif /* LOCAL_HAVE_timeout_nanoseconds */
				DeeSystem_IF_E1(error, ENOMEM, {
					if (Dee_CollectMemory(1))
						goto again;
					futex_controller_decref(ctrl);
					goto again_inc_n_threads;
				});
				futex_controller_decref(ctrl);
				return DeeUnixSystem_ThrowErrorf(NULL, error, "sem_wait failed");
			}
		} /* Scope... */
#endif /* ... */

		futex_controller_decref(ctrl);
#endif /* ... */
	}
	return result;
#endif /* !DeeFutex_USE_stub */
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
