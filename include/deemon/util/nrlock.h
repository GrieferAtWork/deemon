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
#ifndef GUARD_DEEMON_UTIL_NRLOCK_H
#define GUARD_DEEMON_UTIL_NRLOCK_H 1

#include "../api.h"

/*
 * NR-locks are like regular locks (they are non-recursive), but they still
 * check which thread has acquired a lock, and if a thread tries to acquire
 * a lock that it is already holding, rather than entering a dead-lock state,
 * a special value indicative of the lock already being acquired is returned.
 *
 * Dee_nratomic_lock_t: Non-Recursive Atomic Lock (sched_yield() until available; no interrupt checks)
 * Dee_nrshared_lock_t: Non-Recursive Shared Lock (blocking wait; w/ interrupt checks)
 */


#define Dee_NRLOCK_OK      0    /* Operation succeeded */
#define Dee_NRLOCK_FAIL    1    /* Operation failed */
#define Dee_NRLOCK_ALREADY 2    /* Operation failed because you are already the lock owner */
#define Dee_NRLOCK_ERR     (-1) /* An error was thrown */

#ifdef CONFIG_NO_THREADS
DECL_BEGIN

typedef char Dee_nratomic_lock_t;
#define DEE_NRATOMIC_LOCK_INIT             0
#define Dee_nratomic_lock_init(self)       (void)(*(self) = 0)
#define Dee_nratomic_lock_cinit(self)      (void)Dee_ASSERT(*(self) == 0)
#define Dee_nratomic_lock_available(self)  (*(self) == 0)
#define Dee_nratomic_lock_acquired(self)   (*(self) != 0)
#define Dee_nratomic_lock_owned(self)      (*(self) != 0)
#define Dee_nratomic_lock_release(self)    (void)(*(self) = 0)
#define Dee_nratomic_lock_tryacquire(self) (*(self) ? Dee_NRLOCK_ALREADY : (*(self) = 1, Dee_NRLOCK_OK))
#define Dee_nratomic_lock_acquire(self)    Dee_nratomic_lock_tryacquire(self)

typedef Dee_nratomic_lock_t Dee_nrshared_lock_t;
#define DEE_NRSHARED_LOCK_INIT       DEE_NRATOMIC_LOCK_INIT
#define Dee_nrshared_lock_init       Dee_nratomic_lock_init
#define Dee_nrshared_lock_cinit      Dee_nratomic_lock_cinit
#define Dee_nrshared_lock_available  Dee_nratomic_lock_available
#define Dee_nrshared_lock_acquired   Dee_nratomic_lock_acquired
#define Dee_nrshared_lock_owned      Dee_nratomic_lock_owned
#define Dee_nrshared_lock_release    Dee_nratomic_lock_release
#define Dee_nrshared_lock_tryacquire Dee_nratomic_lock_tryacquire
#define Dee_nrshared_lock_acquire    Dee_nratomic_lock_acquire
#define Dee_nrshared_lock_acquire_timed(self, timeout_nanoseconds) \
	Dee_nratomic_lock_acquire(self)
#define Dee_nrshared_lock_acquire_noint       Dee_nrshared_lock_acquire
#define Dee_nrshared_lock_acquire_noint_timed Dee_nrshared_lock_acquire_timed

DECL_END
#else /* CONFIG_NO_THREADS */

#include <hybrid/__atomic.h>
#include <hybrid/sched/__gettid.h>
#include <hybrid/sched/__yield.h>

#include "futex.h"

#include <stddef.h> /* NULL */
#include <stdint.h> /* uint32_t, uint64_t */

#if (defined(__hybrid_tid_t_IS_NUMERIC) && \
     (__HYBRID_SIZEOF_TID__ == 4 ||        \
      __HYBRID_SIZEOF_TID__ == __SIZEOF_POINTER__))
#define _Dee_SIZEOF_NRLOCK_TID_T    __HYBRID_SIZEOF_TID__
#define _Dee_nrlock_tid_t           __hybrid_tid_t
#define _Dee_nrlock_gettid          __hybrid_gettid
#define _Dee_nrlock_gettid_iscaller __hybrid_gettid_iscaller
#define _Dee_NRLOCK_TID_INVALID     __HYBRID_GETTID_INVALID
#ifdef __HYBRID_GETTID_INVALID_IS_ZERO
#define _Dee_NRLOCK_TID_INVALID_IS_ZERO
#endif /* __HYBRID_GETTID_INVALID_IS_ZERO */
#else /* __hybrid_tid_t_IS_NUMERIC && __HYBRID_SIZEOF_TID__ == ... */
#include "../thread.h"
#define _Dee_SIZEOF_NRLOCK_TID_T __SIZEOF_POINTER__
#define _Dee_nrlock_tid_t        DeeThreadObject *
#define _Dee_nrlock_gettid       DeeThread_Self
#define _Dee_NRLOCK_TID_INVALID  NULL
#define _Dee_NRLOCK_TID_INVALID_IS_ZERO
#endif /* !__hybrid_tid_t_IS_NUMERIC || __HYBRID_SIZEOF_TID__ != ... */
#ifndef _Dee_nrlock_gettid_iscaller
#define _Dee_nrlock_gettid_iscaller(x) ((x) == _Dee_nrlock_gettid())
#endif /* !_Dee_nrlock_gettid_iscaller */
#if _Dee_SIZEOF_NRLOCK_TID_T == 4
#define _Dee_nrlock_gettid_futex_wait(addr, expected)                                  DeeFutex_Wait32(addr, (uint32_t)(expected))
#define _Dee_nrlock_gettid_futex_wait_timed(addr, expected, timeout_nanoseconds)       DeeFutex_Wait32Timed(addr, (uint32_t)(expected), timeout_nanoseconds)
#define _Dee_nrlock_gettid_futex_wait_noint(addr, expected)                            DeeFutex_Wait32NoInt(addr, (uint32_t)(expected))
#define _Dee_nrlock_gettid_futex_wait_noint_timed(addr, expected, timeout_nanoseconds) DeeFutex_Wait32NoIntTimed(addr, (uint32_t)(expected), timeout_nanoseconds)
#else /* _Dee_SIZEOF_NRLOCK_TID_T == 4 */
#define _Dee_nrlock_gettid_futex_wait(addr, expected)                                  DeeFutex_Wait64(addr, (uint64_t)(expected))
#define _Dee_nrlock_gettid_futex_wait_timed(addr, expected, timeout_nanoseconds)       DeeFutex_Wait64Timed(addr, (uint64_t)(expected), timeout_nanoseconds)
#define _Dee_nrlock_gettid_futex_wait_noint(addr, expected)                            DeeFutex_Wait64NoInt(addr, (uint64_t)(expected))
#define _Dee_nrlock_gettid_futex_wait_noint_timed(addr, expected, timeout_nanoseconds) DeeFutex_Wait64NoIntTimed(addr, (uint64_t)(expected), timeout_nanoseconds)
#endif /* _Dee_SIZEOF_NRLOCK_TID_T != 4 */

DECL_BEGIN

/************************************************************************/
/* Non-recursive atomic lock                                            */
/************************************************************************/
typedef struct {
	_Dee_nrlock_tid_t nra_tid; /* Lock owner (`_Dee_NRLOCK_TID_INVALID' if not held) */
} Dee_nratomic_lock_t;

#define DEE_NRATOMIC_LOCK_INIT        { _Dee_NRLOCK_TID_INVALID }
#define Dee_nratomic_lock_init(self)  (void)((self)->nra_tid = _Dee_NRLOCK_TID_INVALID)
#ifdef _Dee_NRLOCK_TID_INVALID_IS_ZERO
#define Dee_nratomic_lock_cinit(self) Dee_ASSERT((self)->nra_tid == _Dee_NRLOCK_TID_INVALID)
#else /* _Dee_NRLOCK_TID_INVALID_IS_ZERO */
#define Dee_nratomic_lock_cinit(self) (void)((self)->nra_tid = _Dee_NRLOCK_TID_INVALID)
#endif /* !_Dee_NRLOCK_TID_INVALID_IS_ZERO */
#define _Dee_nratomic_lock_gettid(self)   __hybrid_atomic_load(&(self)->nra_tid, __ATOMIC_ACQUIRE)
#define Dee_nratomic_lock_available(self) (_Dee_nratomic_lock_gettid(self) == _Dee_NRLOCK_TID_INVALID)
#define Dee_nratomic_lock_acquired(self)  (_Dee_nratomic_lock_gettid(self) != _Dee_NRLOCK_TID_INVALID)
#define Dee_nratomic_lock_owned(self)     _Dee_nrlock_gettid_iscaller(_Dee_nratomic_lock_gettid(self))

#define Dee_nratomic_lock_release(self) \
	__hybrid_atomic_store(&(self)->nra_tid, _Dee_NRLOCK_TID_INVALID, __ATOMIC_RELEASE)
/* @return: Dee_NRLOCK_OK:      Success
 * @return: Dee_NRLOCK_ALREADY: Lock already acquired
 * @return: Dee_NRLOCK_FAIL:    Try again */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_nratomic_lock_tryacquire(Dee_nratomic_lock_t *__restrict self) {
	_Dee_nrlock_tid_t owner;
	_Dee_nrlock_tid_t caller = _Dee_nrlock_gettid();
	owner = __hybrid_atomic_cmpxch_val(&self->nra_tid, _Dee_NRLOCK_TID_INVALID, caller,
	                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
	if (owner == NULL)
		return Dee_NRLOCK_OK;
	if (owner == caller)
		return Dee_NRLOCK_ALREADY;
	return Dee_NRLOCK_FAIL;
}

/* @return: Dee_NRLOCK_OK:      Success
 * @return: Dee_NRLOCK_ALREADY: Lock already acquired */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_nratomic_lock_acquire(Dee_nratomic_lock_t *__restrict self) {
	_Dee_nrlock_tid_t owner;
	_Dee_nrlock_tid_t caller = _Dee_nrlock_gettid();
	for (;;) {
		owner = __hybrid_atomic_cmpxch_val(&self->nra_tid, _Dee_NRLOCK_TID_INVALID, caller,
		                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
		if (owner == NULL)
			return Dee_NRLOCK_OK;
		if (owner == caller)
			return Dee_NRLOCK_ALREADY;
		__hybrid_yield();
	}
}


/************************************************************************/
/* Non-recursive shared lock                                            */
/************************************************************************/
typedef struct {
	Dee_nratomic_lock_t nrs_lock;    /* Lock owner and futex word (`_Dee_NRLOCK_TID_INVALID' if not held) */
	unsigned int        nrs_waiting; /* non-zero if threads may be waiting on `nrs_lock.nra_tid' */
} Dee_nrshared_lock_t;

#define _Dee_nrshared_lock_mark_waiting(self) \
	__hybrid_atomic_store(&(self)->nrs_waiting, 1, __ATOMIC_RELEASE)
#define _Dee_nrshared_lock_wake(self)                                     \
	(__hybrid_atomic_load(&(self)->nrs_waiting, __ATOMIC_ACQUIRE)         \
	 ? (__hybrid_atomic_store(&(self)->nrs_waiting, 0, __ATOMIC_RELEASE), \
	    DeeFutex_WakeAll(&(self)->nrs_lock.nra_tid))                      \
	 : (void)0)
#define DEE_NRSHARED_LOCK_INIT            { DEE_NRATOMIC_LOCK_INIT, 0 }
#define Dee_nrshared_lock_init(self)      (void)(Dee_nratomic_lock_init(&(self)->nrs_lock), (self)->nrs_waiting = 0)
#define Dee_nrshared_lock_cinit(self)     (void)(Dee_nratomic_lock_cinit(&(self)->nrs_lock), Dee_ASSERT((self)->nrs_waiting == 0))
#define Dee_nrshared_lock_available(self) Dee_nratomic_lock_available(&(self)->nrs_lock)
#define Dee_nrshared_lock_acquired(self)  Dee_nratomic_lock_acquired(&(self)->nrs_lock)
#define Dee_nrshared_lock_owned(self)     Dee_nratomic_lock_owned(&(self)->nrs_lock)

#define Dee_nrshared_lock_release(self) \
	(Dee_nratomic_lock_release(&(self)->nrs_lock), _Dee_nrshared_lock_wake(self))
#define Dee_nrshared_lock_tryacquire(self) \
	Dee_nratomic_lock_tryacquire(&(self)->nrs_lock)

/* @return: Dee_NRLOCK_OK:      Success
 * @return: Dee_NRLOCK_ALREADY: Lock already acquired
 * @return: Dee_NRLOCK_ERR:     Error was thrown */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_nrshared_lock_acquire(Dee_nrshared_lock_t *__restrict self) {
	_Dee_nrlock_tid_t owner;
	_Dee_nrlock_tid_t caller = _Dee_nrlock_gettid();
	for (;;) {
		owner = __hybrid_atomic_cmpxch_val(&self->nrs_lock.nra_tid,
		                                   _Dee_NRLOCK_TID_INVALID, caller,
		                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
		if (owner == NULL)
			return Dee_NRLOCK_OK;
		if (owner == caller)
			return Dee_NRLOCK_ALREADY;
		_Dee_nrshared_lock_mark_waiting(self);
		if unlikely(_Dee_nrlock_gettid_futex_wait(&self->nrs_lock.nra_tid, owner))
			return Dee_NRLOCK_ERR;
	}
}

/* @return: Dee_NRLOCK_OK:      Success
 * @return: Dee_NRLOCK_ALREADY: Lock already acquired
 * @return: Dee_NRLOCK_FAIL:    Timeout expired
 * @return: Dee_NRLOCK_ERR:     Error was thrown */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_nrshared_lock_acquire_timed(Dee_nrshared_lock_t *__restrict self,
                                uint64_t timeout_nanoseconds) {
	_Dee_nrlock_tid_t owner;
	_Dee_nrlock_tid_t caller = _Dee_nrlock_gettid();
	for (;;) {
		int error;
		owner = __hybrid_atomic_cmpxch_val(&self->nrs_lock.nra_tid,
		                                   _Dee_NRLOCK_TID_INVALID, caller,
		                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
		if (owner == NULL)
			return Dee_NRLOCK_OK;
		if (owner == caller)
			return Dee_NRLOCK_ALREADY;
		_Dee_nrshared_lock_mark_waiting(self);
		error = _Dee_nrlock_gettid_futex_wait_timed(&self->nrs_lock.nra_tid,
		                                            owner, timeout_nanoseconds);
		if unlikely(error != 0) {
			if (error > 0)
				return Dee_NRLOCK_FAIL;
			return Dee_NRLOCK_ERR;
		}
	}
}

/* @return: Dee_NRLOCK_OK:      Success
 * @return: Dee_NRLOCK_ALREADY: Lock already acquired */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_nrshared_lock_acquire_noint(Dee_nrshared_lock_t *__restrict self) {
	_Dee_nrlock_tid_t owner;
	_Dee_nrlock_tid_t caller = _Dee_nrlock_gettid();
	for (;;) {
		owner = __hybrid_atomic_cmpxch_val(&self->nrs_lock.nra_tid,
		                                   _Dee_NRLOCK_TID_INVALID, caller,
		                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
		if (owner == NULL)
			return Dee_NRLOCK_OK;
		if (owner == caller)
			return Dee_NRLOCK_ALREADY;
		_Dee_nrshared_lock_mark_waiting(self);
		_Dee_nrlock_gettid_futex_wait_noint(&self->nrs_lock.nra_tid, owner);
	}
}

/* @return: Dee_NRLOCK_OK:      Success
 * @return: Dee_NRLOCK_ALREADY: Lock already acquired
 * @return: Dee_NRLOCK_FAIL:    Timeout expired */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_nrshared_lock_acquire_noint_timed(Dee_nrshared_lock_t *__restrict self,
                                      uint64_t timeout_nanoseconds) {
	_Dee_nrlock_tid_t owner;
	_Dee_nrlock_tid_t caller = _Dee_nrlock_gettid();
	for (;;) {
		owner = __hybrid_atomic_cmpxch_val(&self->nrs_lock.nra_tid,
		                                   _Dee_NRLOCK_TID_INVALID, caller,
		                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
		if (owner == NULL)
			return Dee_NRLOCK_OK;
		if (owner == caller)
			return Dee_NRLOCK_ALREADY;
		_Dee_nrshared_lock_mark_waiting(self);
		if unlikely(_Dee_nrlock_gettid_futex_wait_noint_timed(&self->nrs_lock.nra_tid,
		                                                      owner, timeout_nanoseconds))
			return Dee_NRLOCK_FAIL;
	}
}

DECL_END

#endif /* !CONFIG_NO_THREADS */

#endif /* !GUARD_DEEMON_UTIL_NRLOCK_H */
