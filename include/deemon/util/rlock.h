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
#ifndef GUARD_DEEMON_UTIL_RLOCK_H
#define GUARD_DEEMON_UTIL_RLOCK_H 1

#include "../api.h"

/*
 * Dee_ratomic_lock_t:   Recursive Atomic Lock (sched_yield() until available; no interrupt checks)
 * Dee_rshared_lock_t:   Recursive Shared Lock (blocking wait; w/ interrupt checks)
 * Dee_ratomic_rwlock_t: Recursive Atomic RWLock (sched_yield() until available; no interrupt checks)
 * Dee_rshared_rwlock_t: Recursive Shared RWLock (blocking wait; w/ interrupt checks)
 */

#ifdef CONFIG_NO_THREADS
DECL_BEGIN

typedef char Dee_ratomic_lock_t;
#define DEE_RATOMIC_LOCK_INIT                     0
#define Dee_ratomic_lock_init(self)               (void)0
#define Dee_ratomic_lock_cinit(self)              (void)0
#define Dee_ratomic_lock_available(self)          1
#define Dee_ratomic_lock_acquired(self)           1
#define Dee_ratomic_lock_tryacquire(self)         1
#define Dee_ratomic_lock_acquire(self)            (void)0
#define Dee_ratomic_lock_waitfor(self)            (void)0
#define _Dee_ratomic_lock_release_NDEBUG(self)    (void)0
#define _Dee_ratomic_lock_release_ex_NDEBUG(self) 1
#define Dee_ratomic_lock_release(self)            (void)0
#define Dee_ratomic_lock_release_ex(self)         1

typedef char Dee_rshared_lock_t;
#define DEE_RSHARED_LOCK_INIT                                     0
#define Dee_rshared_lock_init(self)                               (void)0
#define Dee_rshared_lock_cinit(self)                              (void)0
#define Dee_rshared_lock_available(self)                          1
#define Dee_rshared_lock_acquired(self)                           1
#define Dee_rshared_lock_tryacquire(self)                         1
#define Dee_rshared_lock_acquire(self)                            0
#define Dee_rshared_lock_acquire_noint(self)                      (void)0
#define Dee_rshared_lock_waitfor(self)                            0
#define Dee_rshared_lock_waitfor_noint(self)                      (void)0
#define Dee_rshared_lock_acquire_timed(self, timeout_nanoseconds) 0
#define Dee_rshared_lock_waitfor_timed(self, timeout_nanoseconds) 0
#define _Dee_rshared_lock_release_NDEBUG(self)                    (void)0
#define _Dee_rshared_lock_release_ex_NDEBUG(self)                 1
#define Dee_rshared_lock_release(self)                            (void)0
#define Dee_rshared_lock_release_ex(self)                         1

typedef char Dee_ratomic_rwlock_t;
#define DEE_RATOMIC_RWLOCK_INIT                      0
#define Dee_ratomic_rwlock_init(self)                (void)0
#define Dee_ratomic_rwlock_cinit(self)               (void)0
#define Dee_ratomic_rwlock_tryread(self)             1
#define Dee_ratomic_rwlock_waitread(self)            (void)0
#define Dee_ratomic_rwlock_read(self)                (void)0
#define Dee_ratomic_rwlock_endread(self)             (void)0
#define _Dee_ratomic_rwlock_endread_NDEBUG(self)     (void)0
#define Dee_ratomic_rwlock_endread_ex(self)          1
#define _Dee_ratomic_rwlock_endread_ex_NDEBUG(self)  1
#define Dee_ratomic_rwlock_trywrite(self)            1
#define Dee_ratomic_rwlock_reading(self)             1
#define Dee_ratomic_rwlock_writing(self)             1
#define Dee_ratomic_rwlock_canread(self)             1
#define Dee_ratomic_rwlock_canwrite(self)            1
#define Dee_ratomic_rwlock_canendread(self)          1
#define Dee_ratomic_rwlock_canend(self)              1
#define Dee_ratomic_rwlock_canendwrite(self)         1
#define Dee_ratomic_rwlock_write(self)               (void)0
#define Dee_ratomic_rwlock_waitwrite(self)           (void)0
#define Dee_ratomic_rwlock_tryupgrade(self)          (void)0
#define Dee_ratomic_rwlock_upgrade(self)             1
#define Dee_ratomic_rwlock_downgrade(self)           (void)0
#define _Dee_ratomic_rwlock_downgrade_NDEBUG(self)   (void)0
#define _Dee_ratomic_rwlock_endwrite_NDEBUG(self)    (void)0
#define _Dee_ratomic_rwlock_endwrite_ex_NDEBUG(self) 1
#define Dee_ratomic_rwlock_endwrite(self)            (void)0
#define Dee_ratomic_rwlock_endwrite_ex(self)         1
#define Dee_ratomic_rwlock_end(self)                 (void)0
#define Dee_ratomic_rwlock_end_ex(self)              1
#define _Dee_ratomic_rwlock_end_NDEBUG(self)         (void)0
#define _Dee_ratomic_rwlock_end_ex_NDEBUG(self)      1

typedef char Dee_rshared_rwlock_t;
#define DEE_RSHARED_RWLOCK_INIT                                       0
#define Dee_rshared_rwlock_init(self)                                 (void)0
#define Dee_rshared_rwlock_cinit(self)                                (void)0
#define Dee_rshared_rwlock_reading(self)                              1
#define Dee_rshared_rwlock_writing(self)                              1
#define Dee_rshared_rwlock_tryread(self)                              1
#define Dee_rshared_rwlock_trywrite(self)                             1
#define Dee_rshared_rwlock_canread(self)                              1
#define Dee_rshared_rwlock_canwrite(self)                             1
#define Dee_rshared_rwlock_canendread(self)                           1
#define Dee_rshared_rwlock_canendwrite(self)                          1
#define Dee_rshared_rwlock_canend(self)                               1
#define Dee_rshared_rwlock_tryupgrade(self)                           1
#define Dee_rshared_rwlock_downgrade(self)                            (void)0
#define _Dee_rshared_rwlock_downgrade_NDEBUG(self)                    (void)0
#define Dee_rshared_rwlock_endwrite_ex(self)                          1
#define Dee_rshared_rwlock_endwrite(self)                             (void)0
#define _Dee_rshared_rwlock_endwrite_ex_NDEBUG(self)                  1
#define _Dee_rshared_rwlock_endwrite_NDEBUG(self)                     (void)0
#define Dee_rshared_rwlock_endread(self)                              (void)0
#define Dee_rshared_rwlock_endread_ex(self)                           1
#define _Dee_rshared_rwlock_endread_NDEBUG(self)                      (void)0
#define _Dee_rshared_rwlock_endread_ex_NDEBUG(self)                   1
#define Dee_rshared_rwlock_end(self)                                  (void)0
#define Dee_rshared_rwlock_end_ex(self)                               1
#define _Dee_rshared_rwlock_end_NDEBUG(self)                          (void)0
#define _Dee_rshared_rwlock_end_ex_NDEBUG(self)                       1
#define Dee_rshared_rwlock_read(self)                                 0
#define Dee_rshared_rwlock_read_noint(self)                           (void)0
#define Dee_rshared_rwlock_write(self)                                0
#define Dee_rshared_rwlock_write_noint(self)                          (void)0
#define Dee_rshared_rwlock_waitread(self)                             0
#define Dee_rshared_rwlock_waitread_noint(self)                       (void)0
#define Dee_rshared_rwlock_waitwrite(self)                            0
#define Dee_rshared_rwlock_waitwrite_noint(self)                      (void)0
#define Dee_rshared_rwlock_read_timed(self, timeout_nanoseconds)      0
#define Dee_rshared_rwlock_write_timed(self, timeout_nanoseconds)     0
#define Dee_rshared_rwlock_waitread_timed(self, timeout_nanoseconds)  0
#define Dee_rshared_rwlock_waitwrite_timed(self, timeout_nanoseconds) 0
#define Dee_rshared_rwlock_upgrade(self)                              1
#define Dee_rshared_rwlock_upgrade_noint(self)                        1

DECL_END
#else /* CONFIG_NO_THREADS */

#include "../system-features.h"
#include "../thread.h"
#include "futex.h"
#include "lock.h"
/**/

#include <hybrid/__atomic.h>
#include <hybrid/sched/__gettid.h>
#include <hybrid/sched/__yield.h>
/**/

#include <stdbool.h> /* bool */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

/************************************************************************/
/* Recursive atomic lock                                                */
/************************************************************************/
typedef struct {
	unsigned int   ra_lock; /* Lock word (when non-zero, # of recursive locks held by `ra_tid') */
	__hybrid_tid_t ra_tid;  /* [valid_if(ra_lock > 0)] Lock owner (set to `__HYBRID_GETTID_INVALID' after fully releasing lock) */
} Dee_ratomic_lock_t;

#define DEE_RATOMIC_LOCK_INIT        { 0, __HYBRID_GETTID_INVALID }
#define Dee_ratomic_lock_init(self)  (void)((self)->ra_lock = 0, (self)->ra_tid = __HYBRID_GETTID_INVALID)
#ifdef __HYBRID_GETTID_INVALID_IS_ZERO
#define Dee_ratomic_lock_cinit(self) (void)(Dee_ASSERT((self)->ra_lock == 0), Dee_ASSERT((self)->ra_tid == __HYBRID_GETTID_INVALID))
#else /* __HYBRID_GETTID_INVALID_IS_ZERO */
#define Dee_ratomic_lock_cinit(self) (void)(Dee_ASSERT((self)->ra_lock == 0), (self)->ra_tid = __HYBRID_GETTID_INVALID)
#endif /* !__HYBRID_GETTID_INVALID_IS_ZERO */
#define Dee_ratomic_lock_available(self) \
	(__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) == 0 || __hybrid_gettid_iscaller((self)->ra_tid))
#define Dee_ratomic_lock_acquired(self) \
	(__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) != 0 && __hybrid_gettid_iscaller((self)->ra_tid))

/* Try to acquire a recursive atomic lock. */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_lock_tryacquire(Dee_ratomic_lock_t *__restrict self) {
	if (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) == 0) {
		if (!__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			return false;
		self->ra_tid = __hybrid_gettid();
		return true;
	}
	if (__hybrid_gettid_iscaller(self->ra_tid)) {
		__hybrid_atomic_inc(&self->ra_lock, __ATOMIC_ACQUIRE);
		return true;
	}
	return false;
}

/* Block until successfully acquired a recursive atomic lock. */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_lock_acquire(Dee_ratomic_lock_t *__restrict self) {
	if (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) == 0) {
		if (!__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto waitfor;
settid:
		self->ra_tid = __hybrid_gettid();
		return;
	}
	if (__hybrid_gettid_iscaller(self->ra_tid)) {
		__hybrid_atomic_inc(&self->ra_lock, __ATOMIC_ACQUIRE);
		return;
	}
waitfor:
	while (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) != 0)
		__hybrid_yield();
	if (__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		goto settid;
	goto waitfor;
}

/* Wait until acquiring a recursive atomic lock becomes non-blocking. */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_lock_waitfor(Dee_ratomic_lock_t *__restrict self) {
	if (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) == 0)
		return;
	if (__hybrid_gettid_iscaller(self->ra_tid))
		return;
	while (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) != 0)
		__hybrid_yield();
}

/* Release a recursive atomic lock. */
#define _Dee_ratomic_lock_release_NDEBUG(self)                 \
	((self)->ra_lock > 1                                       \
	 ? __hybrid_atomic_dec(&(self)->ra_lock, __ATOMIC_RELEASE) \
	 : ((self)->ra_tid = __HYBRID_GETTID_INVALID,              \
	    __hybrid_atomic_store(&(self)->ra_lock, 0, __ATOMIC_RELEASE)))
#define _Dee_ratomic_lock_release_ex_NDEBUG(self)                   \
	((self)->ra_lock > 1                                            \
	 ? (__hybrid_atomic_dec(&(self)->ra_lock, __ATOMIC_RELEASE), 0) \
	 : ((self)->ra_tid = __HYBRID_GETTID_INVALID,                   \
	    __hybrid_atomic_store(&(self)->ra_lock, 0, __ATOMIC_RELEASE), 1))
#define Dee_ratomic_lock_release(self)                                                    \
	(Dee_ASSERTF((self)->ra_lock > 0, "No locks held"),                                   \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->ra_tid), "You're not the lock holder"), \
	 _Dee_ratomic_lock_release_NDEBUG(self))
#define Dee_ratomic_lock_release_ex(self)                                                 \
	(Dee_ASSERTF((self)->ra_lock > 0, "No locks held"),                                   \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->ra_tid), "You're not the lock holder"), \
	 _Dee_ratomic_lock_release_ex_NDEBUG(self))






/************************************************************************/
/* Recursive shared lock (scheduler-level blocking lock)                */
/************************************************************************/
typedef struct {
	Dee_ratomic_lock_t rs_lock;    /* Underlying atomic lock */
	unsigned int       rs_waiting; /* # of threads waiting for `rs_lock' (controlled by the waiting threads themselves) */
} Dee_rshared_lock_t;

#define _Dee_rshared_lock_waiting_start(self) __hybrid_atomic_inc(&(self)->rs_waiting, __ATOMIC_ACQUIRE)
#define _Dee_rshared_lock_waiting_end(self)   __hybrid_atomic_dec(&(self)->rs_waiting, __ATOMIC_RELEASE)

#define DEE_RSHARED_LOCK_INIT             { DEE_RATOMIC_LOCK_INIT, 0 }
#define Dee_rshared_lock_init(self)       (void)(Dee_ratomic_lock_init(&(self)->rs_lock), (self)->rs_waiting = 0)
#define Dee_rshared_lock_cinit(self)      (void)(Dee_ratomic_lock_cinit(&(self)->rs_lock), Dee_ASSERT((self)->rs_waiting == 0))
#define Dee_rshared_lock_available(self)  Dee_ratomic_lock_available(&(self)->rs_lock)
#define Dee_rshared_lock_acquired(self)   Dee_ratomic_lock_acquired(&(self)->rs_lock)
#define Dee_rshared_lock_tryacquire(self) Dee_ratomic_lock_tryacquire(&(self)->rs_lock)
#define _Dee_rshared_lock_wakeone(self)   (void)(__hybrid_atomic_load(&(self)->rs_waiting, __ATOMIC_ACQUIRE) && (DeeFutex_WakeOne(&(self)->rs_lock.ra_lock), 1))

/* Block until successfully acquired a recursive shared lock. (does not check for interrupts) */
DFUNDEF NONNULL((1)) void DCALL
Dee_rshared_lock_acquire_noint(Dee_rshared_lock_t *__restrict self);

/* Block until acquiring a recursive shared lock would no longer block. (does not check for interrupts) */
LOCAL NONNULL((1)) void DCALL
Dee_rshared_lock_waitfor_noint(Dee_rshared_lock_t *__restrict self) {
	unsigned int lockword;
	if (__hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE) == 0)
		return;
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid))
		return;
	_Dee_rshared_lock_waiting_start(self);
	while ((lockword = __hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE)) != 0)
		DeeFutex_WaitIntNoInt(&self->rs_lock.ra_lock, lockword);
	_Dee_rshared_lock_waiting_end(self);
}

/* Block until successfully acquired a recursive shared lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_lock_acquire(Dee_rshared_lock_t *__restrict self);

/* Block until acquiring a recursive shared lock would no longer block.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_lock_waitfor(Dee_rshared_lock_t *__restrict self) {
	unsigned int lockword;
	if (__hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE) == 0)
		return 0;
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid))
		return 0;
	_Dee_rshared_lock_waiting_start(self);
	while ((lockword = __hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE)) != 0) {
		int result = DeeFutex_WaitInt(&self->rs_lock.ra_lock, lockword);
		if unlikely(result != 0) {
			_Dee_rshared_lock_waiting_end(self);
			return result;
		}
	}
	_Dee_rshared_lock_waiting_end(self);
	return 0;
}

/* Block until successfully acquired a recursive shared lock.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_lock_acquire_timed(Dee_rshared_lock_t *__restrict self,
                               uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_lock_waitfor_timed(Dee_rshared_lock_t *__restrict self,
                               uint64_t timeout_nanoseconds);


/* Release a recursive shared lock. */
#define _Dee_rshared_lock_release_NDEBUG(self)                                \
	((self)->rs_lock.ra_lock > 1                                              \
	 ? __hybrid_atomic_dec(&(self)->rs_lock.ra_lock, __ATOMIC_RELEASE)        \
	 : ((self)->rs_lock.ra_tid = __HYBRID_GETTID_INVALID,                     \
	    __hybrid_atomic_store(&(self)->rs_lock.ra_lock, 0, __ATOMIC_RELEASE), \
	    _Dee_rshared_lock_wakeone(self)))
#define _Dee_rshared_lock_release_ex_NDEBUG(self)                             \
	((self)->rs_lock.ra_lock > 1                                              \
	 ? (__hybrid_atomic_dec(&(self)->rs_lock.ra_lock, __ATOMIC_RELEASE), 0)   \
	 : ((self)->rs_lock.ra_tid = __HYBRID_GETTID_INVALID,                     \
	    __hybrid_atomic_store(&(self)->rs_lock.ra_lock, 0, __ATOMIC_RELEASE), \
	    _Dee_rshared_lock_wakeone(self), 1))
#define Dee_rshared_lock_release(self)                                                            \
	(Dee_ASSERTF((self)->rs_lock.ra_lock > 0, "No locks held"),                                   \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->rs_lock.ra_tid), "You're not the lock holder"), \
	 _Dee_rshared_lock_release_NDEBUG(self))
#define Dee_rshared_lock_release_ex(self)                                                         \
	(Dee_ASSERTF((self)->rs_lock.ra_lock > 0, "No locks held"),                                   \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->rs_lock.ra_tid), "You're not the lock holder"), \
	 _Dee_rshared_lock_release_ex_NDEBUG(self))




/************************************************************************/
/* Recursive atomic rwlock                                              */
/************************************************************************/
typedef struct {
	Dee_atomic_rwlock_t rarw_lock;   /* Underlying atomic read/write lock */
	__hybrid_tid_t      rarw_tid;    /* [valid_if(rarw_lock.arw_lock == (uintptr_t)-1)] Write-lock owner (set to `__HYBRID_GETTID_INVALID' after fully releasing lock) */
	unsigned int        rarw_nwrite; /* [valid_if(rarw_lock.arw_lock == (uintptr_t)-1)] Number of extra write-locks (`0' means that the next `endwrite()' will release the primary write-lock) */
} Dee_ratomic_rwlock_t;
#define DEE_RATOMIC_RWLOCK_INIT \
	{ DEE_ATOMIC_RWLOCK_INIT, __HYBRID_GETTID_INVALID, 0 }
#define Dee_ratomic_rwlock_init(self)                     \
	(void)(Dee_atomic_rwlock_init(&(self)->rarw_lock),    \
	       (self)->rarw_tid    = __HYBRID_GETTID_INVALID, \
	       (self)->rarw_nwrite = 0)
#ifdef __HYBRID_GETTID_INVALID_IS_ZERO
#define Dee_ratomic_rwlock_cinit(self)                              \
	(void)(Dee_atomic_rwlock_cinit(&(self)->rarw_lock),             \
	       Dee_ASSERT((self)->rarw_tid == __HYBRID_GETTID_INVALID), \
	       Dee_ASSERT((self)->rarw_nwrite == 0))
#else /* __HYBRID_GETTID_INVALID_IS_ZERO */
#define Dee_ratomic_rwlock_cinit(self)                  \
	(void)(Dee_atomic_rwlock_cinit(&(self)->rarw_lock), \
	       (self)->rarw_tid = __HYBRID_GETTID_INVALID,  \
	       Dee_ASSERT((self)->rarw_nwrite == 0))
#endif /* !__HYBRID_GETTID_INVALID_IS_ZERO */

/* Try to acquire a read-lock to `self' */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_tryread(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite; /* Read-after-write */
			return true;
		}
		return false;
	}
	if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock,
	                                 lockword, lockword + 1,
	                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		goto again;
	return true;
}

/* Wait until reading becomes possible */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_waitread(Dee_ratomic_rwlock_t *__restrict self) {
	if (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid))
			return; /* Read-after-write */
		while (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == (uintptr_t)-1)
			__hybrid_yield();
	}
}

/* Blocking acquire a read-lock */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_read(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite; /* Read-after-write */
			return;
		}
		do {
			__hybrid_yield();
		} while ((lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock,
		                                          __ATOMIC_ACQUIRE)) == (uintptr_t)-1);
	}
	if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock,
	                                 lockword, lockword + 1,
	                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		goto again;
}

/* Release a read-lock */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_endread(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->rarw_tid),
		            "You're not the write-holder, so you couldn't have done read-after-write");
		Dee_ASSERTF(self->rarw_nwrite > 0,
		            "No recursive write-locks, so this can't be read-after-write");
		--self->rarw_nwrite;
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
	}
}

/* Release a read-lock */
#define _Dee_ratomic_rwlock_endread_NDEBUG(self)                                            \
	((__hybrid_atomic_load(&(self)->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == (uintptr_t)-1) \
	 ? (void)--(self)->rarw_nwrite                                                          \
	 : (void)__hybrid_atomic_dec(&(self)->rarw_lock.arw_lock, __ATOMIC_RELEASE))
LOCAL NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_endread_ex(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->rarw_tid),
		            "You're not the write-holder, so you couldn't have done read-after-write");
		Dee_ASSERTF(self->rarw_nwrite > 0,
		            "No recursive write-locks, so this can't be read-after-write");
		--self->rarw_nwrite;
		return false;
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
		return lockword == 1;
	}
}
LOCAL NONNULL((1)) bool DCALL
_Dee_ratomic_rwlock_endread_ex_NDEBUG(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		--self->rarw_nwrite;
		return false;
	} else {
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
		return lockword == 1;
	}
}


/* Try to acquire a write-lock to `self' */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_trywrite(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
		self->rarw_tid = __hybrid_gettid();
		return true;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return true;
		}
	}
	return false;
}

LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_reading(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return false;
	if (lockword == (uintptr_t)-1)
		return __hybrid_gettid_iscaller(self->rarw_tid);
	return true;
}
#define Dee_ratomic_rwlock_writing(self)                                                     \
	(__hybrid_atomic_load(&(self)->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == (uintptr_t)-1 && \
	 __hybrid_gettid_iscaller((self)->rarw_tid))

/* Check if trying to start reading right now is non-blocking */
#define Dee_ratomic_rwlock_canread(self)                                                     \
	(__hybrid_atomic_load(&(self)->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != (uintptr_t)-1 || \
	 __hybrid_gettid_iscaller((self)->rarw_tid))

/* Check if trying to start writing right now is non-blocking */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_canwrite(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return true;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid))
			return true;
	}
	return false;
}

/* Check if reading/writing/either can be ended right now */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_canendread(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return false;
	if (lockword == (uintptr_t)-1) {
		return self->rarw_nwrite > 0 && /* Only recursive read-after-write can be released with `endread()' */
		       __hybrid_gettid_iscaller(self->rarw_tid);
	}
	return true;
}
#define Dee_ratomic_rwlock_canend(self)      Dee_ratomic_rwlock_reading(self)
#define Dee_ratomic_rwlock_canendwrite(self) Dee_ratomic_rwlock_writing(self)



/* Acquire a write-lock to `self' */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_write(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
settid:
		self->rarw_tid = __hybrid_gettid();
		return;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return;
		}
	}
	while (!__hybrid_atomic_cmpxch(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
	                               __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		__hybrid_yield();
	goto settid;
}

/* Wait until acquiring a write-lock to `self' is non-blocking */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_waitwrite(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return;
		}
	}
	while (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != 0)
		__hybrid_yield();
}

/* Try to upgrade a read-lock into a write-lock */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_tryupgrade(Dee_ratomic_rwlock_t *__restrict self) {
#ifndef __OPTIMIZE_SIZE__
	if (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != 1)
		return false;
#endif /* !__OPTIMIZE_SIZE__ */
	if (!__hybrid_atomic_cmpxch(&self->rarw_lock.arw_lock, 1, (uintptr_t)-1,
	                            __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		return false;
	self->rarw_tid = __hybrid_gettid();
	return true;
}

/* Upgrade a read-lock into a write-lock */
#define Dee_ratomic_rwlock_upgrade(self) \
	(Dee_ratomic_rwlock_tryupgrade(self) ? 1 : (Dee_ratomic_rwlock_endread(self), Dee_ratomic_rwlock_write(self), 0))

/* Downgrade a write-lock into a read-lock */
#define Dee_ratomic_rwlock_downgrade(self)                                                   \
	(Dee_ASSERTF((self)->rarw_lock.arw_lock == (uintptr_t)-1, "Lock not in write-mode"),     \
	 Dee_ASSERTF((self)->rarw_nwrite == 0, "Recursive write-locks are present"),             \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->rarw_tid), "You're not the write-holder"), \
	 _Dee_ratomic_rwlock_downgrade_NDEBUG(self))
#define _Dee_ratomic_rwlock_downgrade_NDEBUG(self) \
	((self)->rarw_tid = __HYBRID_GETTID_INVALID,   \
	 __hybrid_atomic_store(&(self)->rarw_lock.arw_lock, 1, __ATOMIC_RELEASE))

/* Release a write-lock */
#define _Dee_ratomic_rwlock_endwrite_NDEBUG(self)   \
	((self)->rarw_nwrite > 0                        \
	 ? (void)--(self)->rarw_nwrite                  \
	 : ((self)->rarw_tid = __HYBRID_GETTID_INVALID, \
	    __hybrid_atomic_store(&(self)->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE)))
#define _Dee_ratomic_rwlock_endwrite_ex_NDEBUG(self) \
	((self)->rarw_nwrite > 0                         \
	 ? (--(self)->rarw_nwrite, 0)                    \
	 : ((self)->rarw_tid = __HYBRID_GETTID_INVALID,  \
	    __hybrid_atomic_store(&(self)->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE), 1))
#define Dee_ratomic_rwlock_endwrite(self)                                                    \
	(Dee_ASSERTF((self)->rarw_lock.arw_lock == (uintptr_t)-1, "Lock not in write-mode"),     \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->rarw_tid), "You're not the write-holder"), \
	 _Dee_ratomic_rwlock_endwrite_NDEBUG(self))
#define Dee_ratomic_rwlock_endwrite_ex(self)                                                 \
	(Dee_ASSERTF((self)->rarw_lock.arw_lock == (uintptr_t)-1, "Lock not in write-mode"),     \
	 Dee_ASSERTF(__hybrid_gettid_iscaller((self)->rarw_tid), "You're not the write-holder"), \
	 _Dee_ratomic_rwlock_endwrite_ex_NDEBUG(self))

/* Release a read- or write-lock */
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_end(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->rarw_tid), "You're not the write-holder");
		if (self->rarw_nwrite > 0) {
			--self->rarw_nwrite;
			return;
		}
		self->rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
	}
}
LOCAL NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_end_ex(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->rarw_tid), "You're not the write-holder");
		if (self->rarw_nwrite > 0) {
			--self->rarw_nwrite;
			return false;
		}
		self->rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
		return true;
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
		return lockword == 1;
	}
}
LOCAL NONNULL((1)) void DCALL
_Dee_ratomic_rwlock_end_NDEBUG(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (self->rarw_nwrite > 0) {
			--self->rarw_nwrite;
			return;
		}
		self->rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
	} else {
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
	}
}
LOCAL NONNULL((1)) bool DCALL
_Dee_ratomic_rwlock_end_ex_NDEBUG(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (self->rarw_nwrite > 0) {
			--self->rarw_nwrite;
			return false;
		}
		self->rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
		return true;
	} else {
		__hybrid_atomic_dec(&self->rarw_lock.arw_lock, __ATOMIC_RELEASE);
		return lockword == 1;
	}
}




/************************************************************************/
/* Recursive shared rwlock (scheduler-level blocking lock)              */
/************************************************************************/
typedef struct {
	Dee_ratomic_rwlock_t rsrw_lock;    /* Underlying recursive atomic read/write lock */
	unsigned int         rsrw_waiting; /* non-zero if threads may be waiting on `rsrw_lock.rarw_lock.arw_lock' */
} Dee_rshared_rwlock_t;
#define _Dee_rshared_rwlock_mark_waiting(self) \
	__hybrid_atomic_store(&(self)->rsrw_waiting, 1, __ATOMIC_RELEASE)
#define _Dee_rshared_rwlock_wake(self)                                     \
	(__hybrid_atomic_load(&(self)->rsrw_waiting, __ATOMIC_ACQUIRE)         \
	 ? (__hybrid_atomic_store(&(self)->rsrw_waiting, 0, __ATOMIC_RELEASE), \
	    DeeFutex_WakeAll(&(self)->rsrw_lock.rarw_lock.arw_lock))           \
	 : (void)0)
#define DEE_RSHARED_RWLOCK_INIT        { DEE_RATOMIC_RWLOCK_INIT, 0 }
#define Dee_rshared_rwlock_init(self)  (void)(Dee_ratomic_rwlock_init(&(self)->rsrw_lock), (self)->rsrw_waiting = 0)
#define Dee_rshared_rwlock_cinit(self) (void)(Dee_ratomic_rwlock_cinit(&(self)->rsrw_lock), Dee_ASSERT((self)->rsrw_waiting == 0))
#define Dee_rshared_rwlock_reading(self)             Dee_ratomic_rwlock_reading(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_writing(self)             Dee_ratomic_rwlock_writing(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_tryread(self)             Dee_ratomic_rwlock_tryread(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_trywrite(self)            Dee_ratomic_rwlock_trywrite(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_canread(self)             Dee_ratomic_rwlock_canread(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_canwrite(self)            Dee_ratomic_rwlock_canwrite(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_canendread(self)          Dee_ratomic_rwlock_canendread(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_canendwrite(self)         Dee_ratomic_rwlock_canendwrite(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_canend(self)              Dee_ratomic_rwlock_canend(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_tryupgrade(self)          Dee_ratomic_rwlock_tryupgrade(&(self)->rsrw_lock)
#define Dee_rshared_rwlock_downgrade(self)           (Dee_ratomic_rwlock_downgrade(&(self)->rsrw_lock), _Dee_rshared_rwlock_wake(self))
#define _Dee_rshared_rwlock_downgrade_NDEBUG(self)   (_Dee_ratomic_rwlock_downgrade_NDEBUG(&(self)->rsrw_lock), _Dee_rshared_rwlock_wake(self))
#define _Dee_rshared_rwlock_endwrite_ex_NDEBUG(self) (_Dee_ratomic_rwlock_endwrite_ex_NDEBUG(&(self)->rsrw_lock) && (_Dee_rshared_rwlock_wake(self), 1))
#define _Dee_rshared_rwlock_endwrite_NDEBUG(self)    (void)_Dee_rshared_rwlock_endwrite_ex_NDEBUG(self)
#define Dee_rshared_rwlock_endwrite(self)            (void)Dee_rshared_rwlock_endwrite_ex(self)

#if 0
/* Release a write-lock
 * @return: true:  All locks have now been released
 * @return: false: You're still holding more write-locks */
#define Dee_rshared_rwlock_endwrite_ex(self)               \
	(Dee_ratomic_rwlock_endwrite_ex(&(self)->rsrw_lock) && \
	 (_Dee_rshared_rwlock_wake(self), 1))
#else
/* Release a write-lock
 * @return: true:  All locks have now been released
 * @return: false: You're still holding more write-locks */
DFUNDEF NONNULL((1)) bool DCALL
Dee_rshared_rwlock_endwrite_ex(Dee_rshared_rwlock_t *__restrict self);
#endif

/* Release a read-lock */
#define Dee_rshared_rwlock_endread (void)Dee_rshared_rwlock_endread_ex

/* Release a read-lock
 * @return: true:  All locks have now been released
 * @return: false: You're still holding more read-locks */
DFUNDEF NONNULL((1)) bool DCALL
Dee_rshared_rwlock_endread_ex(Dee_rshared_rwlock_t *__restrict self);

#define _Dee_rshared_rwlock_endread_NDEBUG (void)_Dee_rshared_rwlock_endread_ex_NDEBUG
LOCAL NONNULL((1)) bool DCALL
_Dee_rshared_rwlock_endread_ex_NDEBUG(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		--self->rsrw_lock.rarw_nwrite;
	} else {
		lockword = __hybrid_atomic_fetchdec(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_RELEASE);
		if (lockword == 1) {
			_Dee_rshared_rwlock_wake(self); /* Last read-lock went away */
			return true;
		}
	}
	return false;
}

/* Release a read- or write-lock */
#define Dee_rshared_rwlock_end (void)Dee_rshared_rwlock_end_ex
LOCAL NONNULL((1)) bool DCALL
Dee_rshared_rwlock_end_ex(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid), "You're not the write-holder");
		if (self->rsrw_lock.rarw_nwrite > 0) {
			--self->rsrw_lock.rarw_nwrite;
			return false;
		}
		self->rsrw_lock.rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->rsrw_lock.rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
		_Dee_rshared_rwlock_wake(self); /* Last write-lock went away */
		return true;
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		lockword = __hybrid_atomic_fetchdec(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_RELEASE);
		Dee_ASSERTF(lockword != 0, "No lock are held (race)");
		if (lockword == 1) {
			_Dee_rshared_rwlock_wake(self); /* Last read-lock went away */
			return true;
		}
		return false;
	}
	__builtin_unreachable();
}

#define _Dee_rshared_rwlock_end_NDEBUG (void)_Dee_rshared_rwlock_end_ex_NDEBUG
LOCAL NONNULL((1)) bool DCALL
_Dee_rshared_rwlock_end_ex_NDEBUG(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (self->rsrw_lock.rarw_nwrite > 0) {
			--self->rsrw_lock.rarw_nwrite;
			return false;
		}
		self->rsrw_lock.rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->rsrw_lock.rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
		_Dee_rshared_rwlock_wake(self); /* Last write-lock went away */
		return true;
	} else {
		lockword = __hybrid_atomic_fetchdec(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_RELEASE);
		if (lockword == 1) {
			_Dee_rshared_rwlock_wake(self); /* Last read-lock went away */
			return true;
		}
		return false;
	}
	__builtin_unreachable();
}


/* Acquire a read-lock to `self' (does not check for interrupts) */
DFUNDEF NONNULL((1)) void DCALL
Dee_rshared_rwlock_read_noint(Dee_rshared_rwlock_t *__restrict self);

/* Acquire a write-lock to `self' (does not check for interrupts) */
DFUNDEF NONNULL((1)) void DCALL
Dee_rshared_rwlock_write_noint(Dee_rshared_rwlock_t *__restrict self);

/* Acquire a read-lock to `self'
 * @return: 0 : Success
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_read(Dee_rshared_rwlock_t *__restrict self);

/* Acquire a write-lock to `self'
 * @return: 0 : Success
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_write(Dee_rshared_rwlock_t *__restrict self);

/* Wait until acquiring a read-lock to `self' no longer blocks */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_waitread(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
			/* Special case for read-after-write */
			return 0;
		}
		do {
			int result;
			_Dee_rshared_rwlock_mark_waiting(self);
			result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
			if unlikely(result != 0)
				return result;
		} while ((lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock,
		                                          __ATOMIC_ACQUIRE)) == (uintptr_t)-1);
	}
	return 0;
}

/* Wait until acquiring a write-lock to `self' no longer blocks */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_waitwrite(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return 0;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid))
			return 0;
	}
	do {
		int result;
		_Dee_rshared_rwlock_mark_waiting(self);
		result = DeeFutex_WaitPtr(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
		if unlikely(result != 0)
			return result;
	} while ((lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE)) != 0);
	return 0;
}

/* Wait until acquiring a read-lock to `self' no longer blocks (does not check for interrupts) */
LOCAL NONNULL((1)) void DCALL
Dee_rshared_rwlock_waitread_noint(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid)) {
			/* Special case for read-after-write */
			return;
		}
		do {
			_Dee_rshared_rwlock_mark_waiting(self);
			DeeFutex_WaitPtrNoInt(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
		} while ((lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock,
		                                          __ATOMIC_ACQUIRE)) == (uintptr_t)-1);
	}
}

/* Wait until acquiring a write-lock to `self' no longer blocks (does not check for interrupts) */
LOCAL NONNULL((1)) void DCALL
Dee_rshared_rwlock_waitwrite_noint(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rsrw_lock.rarw_tid))
			return;
	}
	do {
		_Dee_rshared_rwlock_mark_waiting(self);
		DeeFutex_WaitPtrNoInt(&self->rsrw_lock.rarw_lock.arw_lock, lockword);
	} while ((lockword = __hybrid_atomic_load(&self->rsrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE)) != 0);
}

/* Block until successfully acquired a recursive shared lock.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_read_timed(Dee_rshared_rwlock_t *__restrict self,
                              uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_write_timed(Dee_rshared_rwlock_t *__restrict self,
                               uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_waitread_timed(Dee_rshared_rwlock_t *__restrict self,
                                  uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_waitwrite_timed(Dee_rshared_rwlock_t *__restrict self,
                                   uint64_t timeout_nanoseconds);


/* Upgrade a read-lock into a write-lock */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_upgrade(Dee_rshared_rwlock_t *__restrict self) {
	if (Dee_rshared_rwlock_tryupgrade(self))
		return 1;
	Dee_rshared_rwlock_endread(self);
	return Dee_rshared_rwlock_write(self);
}

LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_rshared_rwlock_upgrade_noint(Dee_rshared_rwlock_t *__restrict self) {
	if (Dee_rshared_rwlock_tryupgrade(self))
		return 1;
	Dee_rshared_rwlock_endread(self);
	Dee_rshared_rwlock_write_noint(self);
	return 0;
}


DECL_END

#endif /* !CONFIG_NO_THREADS */

#endif /* !GUARD_DEEMON_UTIL_RLOCK_H */
