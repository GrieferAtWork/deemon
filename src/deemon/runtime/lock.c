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
#ifndef GUARD_DEEMON_RUNTIME_LOCK_C
#define GUARD_DEEMON_RUNTIME_LOCK_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

DECL_BEGIN

/************************************************************************/
/* Shared lock (scheduler-level blocking lock)                          */
/************************************************************************/

/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_acquire)(Dee_shared_lock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
	while ((lockword = atomic_xch_explicit(&self->s_lock, 1, __ATOMIC_ACQUIRE)) != 0) {
		int error;
		atomic_inc(&self->s_waiting);
		error = DeeFutex_WaitInt(&self->s_lock, lockword);
		atomic_dec(&self->s_waiting);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_waitfor)(Dee_shared_lock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
	while ((lockword = atomic_read(&self->s_lock)) != 0) {
		int error;
		atomic_inc(&self->s_waiting);
		error = DeeFutex_WaitInt(&self->s_lock, lockword);
		atomic_dec(&self->s_waiting);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

/* Same as `Dee_shared_lock_acquire()' / `Dee_shared_lock_waitfor()',
 * but also takes an additional timeout in nano-seconds. The special
 * values `0' (try-acquire) and `(uint64_t)-1' (infinite timeout) are
 * also recognized for `timeout_nanoseconds'.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_acquire_timed)(Dee_shared_lock_t *__restrict self,
                                      uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
again:
	if ((lockword = atomic_xch_explicit(&self->s_lock, 1, __ATOMIC_ACQUIRE)) != 0) {
		int error;
		uint64_t now_microseconds, then_microseconds;
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			atomic_inc(&self->s_waiting);
			error = DeeFutex_WaitInt(&self->s_lock, lockword);
			atomic_dec(&self->s_waiting);
			if unlikely(error != 0)
				return error;
			goto again;
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		atomic_inc(&self->s_waiting);
		error = DeeFutex_WaitIntTimed(&self->s_lock, lockword, timeout_nanoseconds);
		atomic_dec(&self->s_waiting);
		if unlikely(error != 0)
			return error;
		if ((lockword = atomic_fetchinc_explicit(&self->s_lock, __ATOMIC_ACQUIRE)) != 0) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_waitfor_timed)(Dee_shared_lock_t *__restrict self,
                                      uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
again:
	while ((lockword = atomic_read(&self->s_lock)) != 0) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			atomic_inc(&self->s_waiting);
			error = DeeFutex_WaitInt(&self->s_lock, lockword);
			atomic_dec(&self->s_waiting);
			if unlikely(error != 0)
				return error;
			goto again;
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		atomic_inc(&self->s_waiting);
		error = DeeFutex_WaitIntTimed(&self->s_lock, lockword, timeout_nanoseconds);
		atomic_dec(&self->s_waiting);
		if unlikely(error != 0)
			return error;
		if ((lockword = atomic_read(&self->s_lock)) != 0) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}






/************************************************************************/
/* Shared r/w-lock (scheduler-level blocking lock)                      */
/************************************************************************/

PUBLIC NONNULL((1)) void
(DCALL Dee_shared_rwlock_end)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	if (self->srw_lock != (uintptr_t)-1) {
		/* Read-lock */
		uintptr_t temp;
		Dee_ASSERTF(self->srw_lock != 0, "No remaining read-locks");
		temp = atomic_decfetch_explicit(&self->srw_lock, __ATOMIC_RELEASE);
		if (temp == 0)
			_Dee_shared_rwlock_wake(self);
	} else {
		/* Write-lock */
		atomic_write(&self->srw_lock, 0);
		_Dee_shared_rwlock_wake(self);
	}
#endif /* !CONFIG_NO_THREADS */
}


/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_read)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	while (!Dee_shared_rwlock_tryread(self)) {
		int error;
		atomic_write(&self->srw_waiting, 1);
		error = DeeFutex_WaitPtr(&self->srw_lock, (uintptr_t)-1);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_write)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		uintptr_t lockword = atomic_read(&self->srw_lock);
		if (lockword == 0) {
			if (atomic_cmpxch_explicit(&self->srw_lock, 0, (uintptr_t)-1,
			                           __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				break;
		} else {
			int error;
			atomic_write(&self->srw_waiting, 1);
			error = DeeFutex_WaitPtr(&self->srw_lock, lockword);
			if unlikely(error != 0)
				return error;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitread)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	while (!Dee_shared_rwlock_canread(self)) {
		int error;
		atomic_write(&self->srw_waiting, 1);
		error = DeeFutex_WaitPtr(&self->srw_lock, (uintptr_t)-1);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitwrite)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		int error;
		uintptr_t lockword = atomic_read(&self->srw_lock);
		if (lockword == 0)
			break;
		atomic_write(&self->srw_waiting, 1);
		error = DeeFutex_WaitPtr(&self->srw_lock, lockword);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_read_timed)(Dee_shared_rwlock_t *__restrict self,
                                     uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	if (!Dee_shared_rwlock_tryread(self)) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			return (Dee_shared_rwlock_read)(self);
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		atomic_write(&self->srw_waiting, 1);
		error = DeeFutex_WaitPtrTimed(&self->srw_lock, (uintptr_t)-1, timeout_nanoseconds);
		if unlikely(error != 0)
			return error;
		if (!Dee_shared_rwlock_tryread(self)) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_write_timed)(Dee_shared_rwlock_t *__restrict self,
                                      uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		uintptr_t lockword = atomic_read(&self->srw_lock);
		if (lockword == 0) {
			if (atomic_cmpxch_explicit(&self->srw_lock, 0, (uintptr_t)-1,
			                           __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				break;
		} else {
			uint64_t now_microseconds, then_microseconds;
			int error;
			if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
				return (Dee_shared_rwlock_write)(self);
			}
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
				goto do_infinite_timeout;
do_wait_with_timeout:
			atomic_write(&self->srw_waiting, 1);
			error = DeeFutex_WaitPtrTimed(&self->srw_lock, lockword, timeout_nanoseconds);
			if unlikely(error != 0)
				return error;
			lockword = atomic_read(&self->srw_lock);
			if (lockword == 0 &&
			    atomic_cmpxch_explicit(&self->srw_lock, 0, (uintptr_t)-1,
			                           __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				break;
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitread_timed)(Dee_shared_rwlock_t *__restrict self,
                                         uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	if (!Dee_shared_rwlock_canread(self)) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			return (Dee_shared_rwlock_waitread)(self);
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		atomic_write(&self->srw_waiting, 1);
		error = DeeFutex_WaitPtrTimed(&self->srw_lock, (uintptr_t)-1, timeout_nanoseconds);
		if unlikely(error != 0)
			return error;
		if (!Dee_shared_rwlock_canread(self)) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitwrite_timed)(Dee_shared_rwlock_t *__restrict self,
                                          uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	int error;
	uint64_t now_microseconds, then_microseconds;
	uintptr_t lockword = atomic_read(&self->srw_lock);
	if (lockword == 0)
		return 0;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return (Dee_shared_rwlock_waitwrite)(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	atomic_write(&self->srw_waiting, 1);
	error = DeeFutex_WaitPtrTimed(&self->srw_lock, lockword, timeout_nanoseconds);
	if unlikely(error != 0)
		return error;
	lockword = atomic_read(&self->srw_lock);
	if (lockword == 0)
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}






/************************************************************************/
/* Shared semaphore (scheduler-level blocking)                          */
/************************************************************************/

#ifndef CONFIG_NO_THREADS
PRIVATE WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_do_waitfor)(Dee_semaphore_t *__restrict self) {
	int result;
	atomic_inc(&self->se_waiting);
	result = DeeFutex_WaitPtr(&self->se_tickets, 0);
	atomic_dec(&self->se_waiting);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_do_waitfor_timed)(Dee_semaphore_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
	int result;
	atomic_inc(&self->se_waiting);
	result = DeeFutex_WaitPtrTimed(&self->se_tickets, 0, timeout_nanoseconds);
	atomic_dec(&self->se_waiting);
	return result;
}
#endif /* !CONFIG_NO_THREADS */


/* Blocking acquire a semaphore ticket, or wait for one to become available.
 * @return: 1 : Timeout expired. (`*_timed' only)
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_waitfor)(Dee_semaphore_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return 0;
#else /* CONFIG_NO_THREADS */
	while (!Dee_semaphore_hastickets(self)) {
		int result = Dee_semaphore_do_waitfor(self);
		if unlikely(result != 0)
			return result;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_waitfor_timed)(Dee_semaphore_t *__restrict self,
                                    uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int error;
	uint64_t now_microseconds, then_microseconds;
	if (Dee_semaphore_hastickets(self))
		return 0;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return (Dee_semaphore_waitfor)(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = Dee_semaphore_do_waitfor_timed(self, timeout_nanoseconds);
	if unlikely(error != 0)
		return error;
	if (Dee_semaphore_hastickets(self))
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_acquire)(Dee_semaphore_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return 0;
#else /* CONFIG_NO_THREADS */
	uintptr_t temp;
	do {
again_read_tickets:
		temp = atomic_read(&self->se_tickets);
		if (temp == 0) {
			int result = Dee_semaphore_do_waitfor(self);
			if likely(result == 0)
				goto again_read_tickets;
			return result;
		}
	} while (!atomic_cmpxch_weak_explicit(&self->se_tickets, temp, temp - 1,
	                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED));
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_acquire_timed)(Dee_semaphore_t *__restrict self,
                                    uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int error;
	uint64_t now_microseconds, then_microseconds;
	if (Dee_semaphore_tryacquire(self))
		return 0;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return (Dee_semaphore_acquire)(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = Dee_semaphore_do_waitfor_timed(self, timeout_nanoseconds);
	if unlikely(error != 0)
		return error;
	if (Dee_semaphore_tryacquire(self))
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}






/************************************************************************/
/* Shared event (scheduler-level blocking)                              */
/************************************************************************/

/* Blocking wait for an event to become set.
 * @return: 1 : Timeout expired. (`Dee_event_waitfor_timed' only)
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_event_waitfor)(Dee_event_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	if (Dee_event_get(self))
		return 0;
	atomic_cmpxch(&self->ev_state, 1, 2);
	do {
		result = DeeFutex_WaitInt(&self->ev_state, 2);
	} while (result == 0 && !Dee_event_get(self));
	return result;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_event_waitfor_timed)(Dee_event_t *__restrict self,
                                uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int error;
	uint64_t now_microseconds, then_microseconds;
	if (Dee_event_get(self))
		return 0;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return (Dee_event_waitfor)(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	atomic_cmpxch(&self->ev_state, 1, 2);
	error = DeeFutex_WaitIntTimed(&self->ev_state, 2, timeout_nanoseconds);
	if unlikely(error != 0)
		return error;
	if (Dee_event_get(self))
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_LOCK_C */