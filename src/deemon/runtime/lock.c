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
#ifndef GUARD_DEEMON_RUNTIME_LOCK_C
#define GUARD_DEEMON_RUNTIME_LOCK_C 1

#include <deemon/api.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/once.h>
#include <deemon/util/futex.h>
#include <deemon/util/rlock.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/__gettid.h>
/**/

#include <stddef.h> /* uintptr_t */
#include <stdint.h> /* uint64_t */

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
	while ((lockword = atomic_xch_explicit(&self->s_lock.a_lock, 1, Dee_ATOMIC_ACQUIRE)) != 0) {
		int error;
		_Dee_shared_lock_waiting_start(self);
		error = DeeFutex_WaitInt(&self->s_lock, lockword);
		_Dee_shared_lock_waiting_end(self);
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
	while ((lockword = atomic_read(&self->s_lock.a_lock)) != 0) {
		int error;
		_Dee_shared_lock_waiting_start(self);
		error = DeeFutex_WaitInt(&self->s_lock.a_lock, lockword);
		_Dee_shared_lock_waiting_end(self);
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
	if ((lockword = atomic_xch_explicit(&self->s_lock.a_lock, 1, Dee_ATOMIC_ACQUIRE)) != 0) {
		int error;
		uint64_t now_microseconds, then_microseconds;
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			_Dee_shared_lock_waiting_start(self);
			error = DeeFutex_WaitInt(&self->s_lock.a_lock, lockword);
			_Dee_shared_lock_waiting_end(self);
			if unlikely(error != 0)
				return error;
			goto again;
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		_Dee_shared_lock_waiting_start(self);
		error = DeeFutex_WaitIntTimed(&self->s_lock.a_lock, lockword, timeout_nanoseconds);
		_Dee_shared_lock_waiting_end(self);
		if unlikely(error != 0)
			return error;
		if ((lockword = atomic_fetchinc_explicit(&self->s_lock.a_lock, Dee_ATOMIC_ACQUIRE)) != 0) {
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
	while ((lockword = atomic_read(&self->s_lock.a_lock)) != 0) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			_Dee_shared_lock_waiting_start(self);
			error = DeeFutex_WaitInt(&self->s_lock.a_lock, lockword);
			_Dee_shared_lock_waiting_end(self);
			if unlikely(error != 0)
				return error;
			goto again;
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		_Dee_shared_lock_waiting_start(self);
		error = DeeFutex_WaitIntTimed(&self->s_lock.a_lock, lockword, timeout_nanoseconds);
		_Dee_shared_lock_waiting_end(self);
		if unlikely(error != 0)
			return error;
		if ((lockword = atomic_read(&self->s_lock.a_lock)) != 0) {
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
		_Dee_shared_rwlock_mark_waiting(self);
		error = DeeFutex_WaitPtr(&self->srw_lock.arw_lock, (uintptr_t)-1);
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
		uintptr_t lockword = atomic_read(&self->srw_lock.arw_lock);
		if (lockword == 0) {
			if (atomic_cmpxch_explicit(&self->srw_lock.arw_lock, 0, (uintptr_t)-1,
			                           Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_RELAXED))
				break;
		} else {
			int error;
			_Dee_shared_rwlock_mark_waiting(self);
			error = DeeFutex_WaitPtr(&self->srw_lock.arw_lock, lockword);
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
		_Dee_shared_rwlock_mark_waiting(self);
		error = DeeFutex_WaitPtr(&self->srw_lock.arw_lock, (uintptr_t)-1);
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
		uintptr_t lockword = atomic_read(&self->srw_lock.arw_lock);
		if (lockword == 0)
			break;
		_Dee_shared_rwlock_mark_waiting(self);
		error = DeeFutex_WaitPtr(&self->srw_lock.arw_lock, lockword);
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
		_Dee_shared_rwlock_mark_waiting(self);
		error = DeeFutex_WaitPtrTimed(&self->srw_lock.arw_lock, (uintptr_t)-1, timeout_nanoseconds);
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
		uintptr_t lockword = atomic_read(&self->srw_lock.arw_lock);
		if (lockword == 0) {
			if (atomic_cmpxch_explicit(&self->srw_lock.arw_lock, 0, (uintptr_t)-1,
			                           Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_RELAXED))
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
			_Dee_shared_rwlock_mark_waiting(self);
			error = DeeFutex_WaitPtrTimed(&self->srw_lock.arw_lock, lockword, timeout_nanoseconds);
			if unlikely(error != 0)
				return error;
			lockword = atomic_read(&self->srw_lock.arw_lock);
			if (lockword == 0 &&
			    atomic_cmpxch_explicit(&self->srw_lock.arw_lock, 0, (uintptr_t)-1,
			                           Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_RELAXED))
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
		_Dee_shared_rwlock_mark_waiting(self);
		error = DeeFutex_WaitPtrTimed(&self->srw_lock.arw_lock, (uintptr_t)-1, timeout_nanoseconds);
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
	uintptr_t lockword = atomic_read(&self->srw_lock.arw_lock);
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
	_Dee_shared_rwlock_mark_waiting(self);
	error = DeeFutex_WaitPtrTimed(&self->srw_lock.arw_lock, lockword, timeout_nanoseconds);
	if unlikely(error != 0)
		return error;
	lockword = atomic_read(&self->srw_lock.arw_lock);
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
	_Dee_semaphore_waiting_start(self);
	result = DeeFutex_WaitPtr(&self->se_tickets, 0);
	_Dee_semaphore_waiting_end(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_do_waitfor_timed)(Dee_semaphore_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
	int result;
	_Dee_semaphore_waiting_start(self);
	result = DeeFutex_WaitPtrTimed(&self->se_tickets, 0, timeout_nanoseconds);
	_Dee_semaphore_waiting_end(self);
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
	size_t temp;
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
	                                      Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_RELAXED));
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












/************************************************************************/
/* Recursive shared lock (scheduler-level blocking lock)                */
/************************************************************************/

/* Block until successfully acquired a recursive shared lock. (does not check for interrupts) */
PUBLIC NONNULL((1)) void
(DCALL Dee_rshared_lock_acquire_noint)(Dee_rshared_lock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
again:
	lockword = atomic_read(&self->rs_lock.ra_lock);
	if (lockword == 0) {
		if (!atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto again;
settid:
		self->rs_lock.ra_tid = __hybrid_gettid();
		return;
	}
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid)) {
		atomic_inc_explicit(&self->rs_lock.ra_lock, Dee_ATOMIC_ACQUIRE);
		return;
	}
	_Dee_rshared_lock_waiting_start(self);
	do {
		DeeFutex_WaitIntNoInt(&self->rs_lock.ra_lock, lockword);
	} while (!atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE));
	_Dee_rshared_lock_waiting_end(self);
	goto settid;
#endif /* !CONFIG_NO_THREADS */
}

/* Block until successfully acquired a recursive shared lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_lock_acquire)(Dee_rshared_lock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	unsigned int lockword;
again:
	lockword = atomic_read(&self->rs_lock.ra_lock);
	if (lockword == 0) {
		if (!atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto again;
settid:
		self->rs_lock.ra_tid = __hybrid_gettid();
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid)) {
		atomic_inc_explicit(&self->rs_lock.ra_lock, Dee_ATOMIC_ACQUIRE);
		return 0;
	}
	_Dee_rshared_lock_waiting_start(self);
	do {
		result = DeeFutex_WaitInt(&self->rs_lock.ra_lock, lockword);
		if unlikely(result != 0) {
			_Dee_rshared_lock_waiting_end(self);
			return result;
		}
	} while (!atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE));
	_Dee_rshared_lock_waiting_end(self);
	goto settid;
#endif /* !CONFIG_NO_THREADS */
}

/* Block until successfully acquired a recursive shared lock.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_lock_acquire_timed)(Dee_rshared_lock_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	unsigned int lockword;
	uint64_t now_microseconds, then_microseconds;
again:
	lockword = atomic_read(&self->rs_lock.ra_lock);
	if (lockword == 0) {
		if (!atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto again;
settid:
		self->rs_lock.ra_tid = __hybrid_gettid();
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid)) {
		atomic_inc_explicit(&self->rs_lock.ra_lock, Dee_ATOMIC_ACQUIRE);
		return 0;
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		_Dee_rshared_lock_waiting_start(self);
		do {
			result = DeeFutex_WaitInt(&self->rs_lock.ra_lock, lockword);
			if unlikely(result != 0) {
				_Dee_rshared_lock_waiting_end(self);
				return result;
			}
		} while ((lockword = atomic_read(&self->rs_lock.ra_lock)) != 0);
		_Dee_rshared_lock_waiting_end(self);
		do {
			if (atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
				goto settid;
		} while ((lockword = atomic_read(&self->rs_lock.ra_lock)) == 0);
		goto do_infinite_timeout;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	_Dee_rshared_lock_waiting_start(self);
	result = DeeFutex_WaitIntTimed(&self->rs_lock.ra_lock, lockword, timeout_nanoseconds);
	_Dee_rshared_lock_waiting_end(self);
	if unlikely(result != 0)
		return result;
	do {
		if (atomic_cmpxch_explicit(&self->rs_lock.ra_lock, 0, 1, Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto settid;
	} while ((lockword = atomic_read(&self->rs_lock.ra_lock)) == 0);
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_lock_waitfor_timed)(Dee_rshared_lock_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	unsigned int lockword;
	uint64_t now_microseconds, then_microseconds;
	lockword = atomic_read(&self->rs_lock.ra_lock);
	if (lockword == 0)
		return 0;
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid))
		return 0;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		_Dee_rshared_lock_waiting_start(self);
		do {
			result = DeeFutex_WaitInt(&self->rs_lock.ra_lock, lockword);
			if unlikely(result != 0) {
				_Dee_rshared_lock_waiting_end(self);
				return result;
			}
		} while ((lockword = atomic_read(&self->rs_lock.ra_lock)) != 0);
		_Dee_rshared_lock_waiting_end(self);
		return 0;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	_Dee_rshared_lock_waiting_start(self);
	result = DeeFutex_WaitIntTimed(&self->rs_lock.ra_lock, lockword, timeout_nanoseconds);
	_Dee_rshared_lock_waiting_end(self);
	if unlikely(result != 0)
		return result;
	lockword = atomic_read(&self->rs_lock.ra_lock);
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
/* Recursive shared rwlock (scheduler-level blocking lock)              */
/************************************************************************/

/* Release a write-lock
 * @return: true:  All locks have now been released
 * @return: false: You're still holding more write-locks */
PUBLIC NONNULL((1)) bool
(DCALL Dee_rshared_rwlock_endwrite_ex)(Dee_rshared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return true;
#else /* CONFIG_NO_THREADS */
	if (Dee_ratomic_rwlock_endwrite_ex(&self->rsrw_lock)) {
		_Dee_rshared_rwlock_wake(self);
		return true;
	}
	return false;
#endif /* !CONFIG_NO_THREADS */
}

/* Release a read-lock
 * @return: true:  All locks have now been released
 * @return: false: You're still holding more read-locks */
PUBLIC NONNULL((1)) bool
(DCALL Dee_rshared_rwlock_endread_ex)(Dee_rshared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return true;
#else /* CONFIG_NO_THREADS */
	uintptr_t lockword;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid),
		            "You're not the write-holder, so you couldn't have done read-after-write");
		Dee_ASSERTF(self->rsrw_lock.rarw_nwrite > 0,
		            "No recursive write-locks, so this can't be read-after-write");
		--self->rsrw_lock.rarw_nwrite;
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		lockword = atomic_fetchdec_explicit(&self->rsrw_lock.rarw_lock.arw_lock, Dee_ATOMIC_RELEASE);
		Dee_ASSERTF(lockword != 0, "No lock are held (race)");
		if (lockword == 1) {
			_Dee_rshared_rwlock_wake(self); /* Last read-lock went away */
			return true;
		}
	}
	return false;
#endif /* !CONFIG_NO_THREADS */
}

/* Acquire a read-lock to `self' (does not check for interrupts) */
PUBLIC NONNULL((1)) void
(DCALL Dee_rshared_rwlock_read_noint)(Dee_rshared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
#else /* CONFIG_NO_THREADS */
	uintptr_t lockword;
again:
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword != (uintptr_t)-1) {
again_lockword_not_UINTPTR_MAX:
		Dee_ASSERTF(lockword != (uintptr_t)-2, "Too many read-locks");
		if (atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock,
		                                lockword, lockword + 1,
		                                Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			return;
		goto again;
	}
	if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
		/* Special case for read-after-write */
		++self->rsrw_lock.rarw_nwrite;
		return;
	}
	do {
		_Dee_rshared_rwlock_mark_waiting(self);
		DeeFutex_WaitPtrNoInt(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
	} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) == (uintptr_t)-1);
	goto again_lockword_not_UINTPTR_MAX;
#endif /* !CONFIG_NO_THREADS */
}

/* Acquire a write-lock to `self' (does not check for interrupts) */
PUBLIC NONNULL((1)) void
(DCALL Dee_rshared_rwlock_write_noint)(Dee_rshared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
#else /* CONFIG_NO_THREADS */
	uintptr_t lockword;
again:
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword == 0) {
again_lockword_zero:
		if (!atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto again;
		self->rsrw_lock.rarw_tid = __hybrid_gettid();
		return;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
			++self->rsrw_lock.rarw_nwrite;
			return;
		}
	}
	do {
		_Dee_rshared_rwlock_mark_waiting(self);
		DeeFutex_WaitPtrNoInt(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
	} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) != 0);
	goto again_lockword_zero;
#endif /* !CONFIG_NO_THREADS */
}

/* Acquire a read-lock to `self'
 * @return: 0 : Success
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_rwlock_read)(Dee_rshared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return 0;
#else /* CONFIG_NO_THREADS */
	uintptr_t lockword;
again:
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword != (uintptr_t)-1) {
again_lockword_not_UINTPTR_MAX:
		Dee_ASSERTF(lockword != (uintptr_t)-2, "Too many read-locks");
		if (atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock,
		                                lockword, lockword + 1,
		                                Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			return 0;
		goto again;
	}
	if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
		/* Special case for read-after-write */
		++self->rsrw_lock.rarw_nwrite;
		return 0;
	}
	do {
		int result;
		_Dee_rshared_rwlock_mark_waiting(self);
		result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
		if unlikely(result != 0)
			return result;
	} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) == (uintptr_t)-1);
	goto again_lockword_not_UINTPTR_MAX;
#endif /* !CONFIG_NO_THREADS */
}

/* Acquire a write-lock to `self'
 * @return: 0 : Success
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_rwlock_write)(Dee_rshared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	return 0;
#else /* CONFIG_NO_THREADS */
	uintptr_t lockword;
again:
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword == 0) {
again_lockword_zero:
		if (!atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto again;
		self->rsrw_lock.rarw_tid = __hybrid_gettid();
		return 0;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
			++self->rsrw_lock.rarw_nwrite;
			return 0;
		}
	}
	do {
		int result;
		_Dee_rshared_rwlock_mark_waiting(self);
		result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
		if unlikely(result != 0)
			return result;
	} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) != 0);
	goto again_lockword_zero;
#endif /* !CONFIG_NO_THREADS */
}

/* Block until successfully acquired a recursive shared lock.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_rwlock_read_timed)(Dee_rshared_rwlock_t *__restrict self,
                                      uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	uintptr_t lockword;
	uint64_t now_microseconds, then_microseconds;
again:
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword != (uintptr_t)-1) {
again_lockword_not_UINTPTR_MAX:
		Dee_ASSERTF(lockword != (uintptr_t)-2, "Too many read-locks");
		if (atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock,
		                                lockword, lockword + 1,
		                                Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			return 0;
		goto again;
	}
	if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
		/* Special case for read-after-write */
		++self->rsrw_lock.rarw_nwrite;
		return 0;
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		do {
			_Dee_rshared_rwlock_mark_waiting(self);
			result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
			if unlikely(result != 0)
				return result;
		} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) == (uintptr_t)-1);
		goto again_lockword_not_UINTPTR_MAX;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	_Dee_rshared_rwlock_mark_waiting(self);
	result = DeeFutex_WaitPtrTimed(&self->rsrw_lock.rarw_lock.arw_lock,
	                               lockword, timeout_nanoseconds);
	if unlikely(result != 0)
		return result;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword != (uintptr_t)-1) {
		Dee_ASSERTF(lockword != (uintptr_t)-2, "Too many read-locks");
		if (atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock,
		                                lockword, lockword + 1,
		                                Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			return 0;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_rwlock_write_timed)(Dee_rshared_rwlock_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	uintptr_t lockword;
	uint64_t now_microseconds, then_microseconds;
again:
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword == 0) {
again_lockword_zero:
		if (!atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE))
			goto again;
		self->rsrw_lock.rarw_tid = __hybrid_gettid();
		return 0;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
			++self->rsrw_lock.rarw_nwrite;
			return 0;
		}
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		do {
			_Dee_rshared_rwlock_mark_waiting(self);
			result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
			if unlikely(result != 0)
				return result;
		} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) != 0);
		goto again_lockword_zero;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	_Dee_rshared_rwlock_mark_waiting(self);
	result = DeeFutex_WaitPtrTimed(&self->rsrw_lock.rarw_lock.arw_lock,
	                               lockword, timeout_nanoseconds);
	if unlikely(result != 0)
		return result;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword == 0) {
		if (atomic_cmpxch_weak_explicit(&self->rsrw_lock.rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                Dee_ATOMIC_ACQUIRE, Dee_ATOMIC_ACQUIRE)) {
			self->rsrw_lock.rarw_tid = __hybrid_gettid();
			return 0;
		}
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_rwlock_waitread_timed)(Dee_rshared_rwlock_t *__restrict self,
                                          uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	uintptr_t lockword;
	uint64_t now_microseconds, then_microseconds;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword != (uintptr_t)-1)
		return 0;
	if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid))
		return 0; /* Special case for read-after-write */
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		do {
			_Dee_rshared_rwlock_mark_waiting(self);
			result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
			if unlikely(result != 0)
				return result;
		} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) == (uintptr_t)-1);
		return 0;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	_Dee_rshared_rwlock_mark_waiting(self);
	result = DeeFutex_WaitPtrTimed(&self->rsrw_lock.rarw_lock.arw_lock,
	                               lockword, timeout_nanoseconds);
	if unlikely(result != 0)
		return result;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword != (uintptr_t)-1)
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_rshared_rwlock_waitwrite_timed)(Dee_rshared_rwlock_t *__restrict self,
                                           uint64_t timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	COMPILER_IMPURE();
	(void)self;
	(void)timeout_nanoseconds;
	return 0;
#else /* CONFIG_NO_THREADS */
	int result;
	uintptr_t lockword;
	uint64_t now_microseconds, then_microseconds;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
	if (lockword == 0)
		return 0;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid))
			return 0;
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		do {
			_Dee_rshared_rwlock_mark_waiting(self);
			result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
			if unlikely(result != 0)
				return result;
		} while ((lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock)) != 0);
		return 0;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	_Dee_rshared_rwlock_mark_waiting(self);
	result = DeeFutex_WaitPtrTimed(&self->rsrw_lock.rarw_lock.arw_lock,
	                               lockword, timeout_nanoseconds);
	if unlikely(result != 0)
		return result;
	lockword = atomic_read(&self->rsrw_lock.rarw_lock.arw_lock);
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
/* Dee_once_t                                                           */
/************************************************************************/


/* Enter the once-block
 * @return: 1 : You're now responsible for executing the once-function
 * @return: 0 : The once-function has already been executed
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_once_begin)(Dee_once_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (*self == _DEE_ONCE_PENDING) {
		*self = _DEE_ONCE_RUNNING;
		return 1; /* You're now responsible for executing the once-function */
	}
	if unlikely(*self != _DEE_ONCE_DONE) {
		return DeeError_Throwf(&DeeError_RuntimeError,
		                       "Deadlock detected: Once function is already "
		                       "being run by the calling thread");
	}
	return 0; /* The once-function has already been executed */
#else /* CONFIG_NO_THREADS */
	uint32_t state;
	state = atomic_read(&self->oc_didrun);
	if (state >= _DEE_ONCE_COMPLETED_THRESHOLD)
		return 0; /* Already executed */

	/* Start trying to run the once-controller. */
again_start_waiting:
	state = atomic_fetchinc(&self->oc_didrun);
	if unlikely(state >= _DEE_ONCE_COMPLETED_THRESHOLD) {
		/* Race condition: the once-block finished in
		 * another thread while we were trying to start it. */
		atomic_write(&self->oc_didrun, _DEE_ONCE_COMPLETED_THRESHOLD);
		return 0;
	}

	if (state == 0)
		return 1; /* Caller must run the block. */

	/* Construct the (presumed) current controller value. */
	++state;

	/* If there are already some other threads waiting for completion. */
	if (state >= 3) {
		atomic_cmpxch(&self->oc_didrun, state, 2);
		state = 2;
	}

	/* Wait for the block to finish or be aborted */
	for (;;) {
		int status;
		status = DeeFutex_Wait32(&self->oc_didrun, state);
		ASSERT(status <= 0);
		if unlikely(status != 0)
			return status;
		state = atomic_read(&self->oc_didrun);
		if (state >= _DEE_ONCE_COMPLETED_THRESHOLD)
			return 0; /* Once-block completed. */

		/* Check for another case: once-block was aborted. */
		if (state <= 1)
			goto again_start_waiting;
	}
#endif /* !CONFIG_NO_THREADS */
}

/* Enter the once-block
 * @return: true:  You're now responsible for executing the once-function
 * @return: false: The once-function has already been executed */
PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_once_begin_noint)(Dee_once_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (*self == _DEE_ONCE_PENDING) {
		*self = _DEE_ONCE_RUNNING;
		return true; /* You're now responsible for executing the once-function */
	}
	if unlikely(*self != _DEE_ONCE_DONE) {
		Dee_Fatalf("Deadlock detected: Once function %p is "
		           "already being run by the calling thread",
		           self);
	}
	return false; /* The once-function has already been executed */
#else /* CONFIG_NO_THREADS */
	uint32_t state;
	state = atomic_read(&self->oc_didrun);
	if (state >= _DEE_ONCE_COMPLETED_THRESHOLD)
		return false; /* Already executed */

	/* Start trying to run the once-controller. */
again_start_waiting:
	state = atomic_fetchinc(&self->oc_didrun);
	if unlikely(state >= _DEE_ONCE_COMPLETED_THRESHOLD) {
		/* Race condition: the once-block finished in
		 * another thread while we were trying to start it. */
		atomic_write(&self->oc_didrun, _DEE_ONCE_COMPLETED_THRESHOLD);
		return false;
	}

	if (state == 0)
		return true; /* Caller must run the block. */

	/* Construct the (presumed) current controller value. */
	++state;

	/* If there are already some other threads waiting for completion. */
	if (state >= 3) {
		atomic_cmpxch(&self->oc_didrun, state, 2);
		state = 2;
	}

	/* Wait for the block to finish or be aborted */
	for (;;) {
		DeeFutex_Wait32NoInt(&self->oc_didrun, state);
		state = atomic_read(&self->oc_didrun);
		if (state >= _DEE_ONCE_COMPLETED_THRESHOLD)
			return false; /* Once-block completed. */

		/* Check for another case: once-block was aborted. */
		if (state <= 1)
			goto again_start_waiting;
	}
#endif /* !CONFIG_NO_THREADS */
}

/* Try to begin execution of a once-block
 * @return: 0 : The once-function has already been executed
 * @return: 1 : You're now responsible for executing the once-function
 * @return: 2 : Another thread is currently executing the once-function (NO ERROR WAS THROWN) */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_once_trybegin)(Dee_once_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return Dee_once_trybegin(self);
#else /* CONFIG_NO_THREADS */
	uint32_t state;
	state = atomic_read(&self->oc_didrun);
	if (state >= _DEE_ONCE_COMPLETED_THRESHOLD)
		return 0; /* Already executed */

	/* Start trying to run the once-controller. */
	state = atomic_fetchinc(&self->oc_didrun);
	if unlikely(state >= _DEE_ONCE_COMPLETED_THRESHOLD) {
		/* Race condition: the once-block finished in
		 * another thread while we were trying to start it. */
		atomic_write(&self->oc_didrun, _DEE_ONCE_COMPLETED_THRESHOLD);
		return 0;
	}

	if (state == 0)
		return 1; /* Caller must run the block. */

	/* Construct the (presumed) current controller value. */
	++state;

	/* If there are already some other threads waiting for completion. */
	if (state >= 3) {
		atomic_cmpxch(&self->oc_didrun, state, 2);
		state = 2;
	}

	/* Another thread is currently executing the once-function. */
	return -1;
#endif /* !CONFIG_NO_THREADS */
}

/* Finish the once-block successfully */
PUBLIC NONNULL((1)) void
(DCALL Dee_once_commit)(Dee_once_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	Dee_once_commit(self);
#else /* CONFIG_NO_THREADS */
	/* Change state to COMPLETED */
	uint32_t old_state;
	old_state = atomic_xch(&self->oc_didrun, _DEE_ONCE_COMPLETED_THRESHOLD);
	ASSERTF(old_state != 0, "Once-block committed, but never stated?");
	if (old_state >= 2)
		DeeFutex_WakeAll(&self->oc_didrun);
#endif /* !CONFIG_NO_THREADS */
}

/* Finish the once-block with an error (causing it to be re-attempted the next time) */
PUBLIC NONNULL((1)) void
(DCALL Dee_once_abort)(Dee_once_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	Dee_once_commit(self);
#else /* CONFIG_NO_THREADS */
	/* Change state to NOT-RUN */
	uint32_t old_state;
	old_state = atomic_xch(&self->oc_didrun, 0);
	ASSERTF(old_state != 0, "Once-block aborted, but never stated?");
	if (old_state >= 2)
		DeeFutex_WakeAll(&self->oc_didrun);
#endif /* !CONFIG_NO_THREADS */
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_LOCK_C */
