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

#include <stdbool.h>
#include <stdint.h>

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
#define Dee_ratomic_lock_tryacquire Dee_ratomic_lock_tryacquire
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
#define Dee_ratomic_lock_acquire Dee_ratomic_lock_acquire
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
#define Dee_ratomic_lock_waitfor Dee_ratomic_lock_waitfor
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
#define Dee_ratomic_lock_release Dee_ratomic_lock_release
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_lock_release(Dee_ratomic_lock_t *__restrict self) {
	Dee_ASSERT(self->ra_lock > 0);
	Dee_ASSERT(__hybrid_gettid_iscaller(self->ra_tid));
	if (self->ra_lock > 1) {
		__hybrid_atomic_dec(&self->ra_lock, __ATOMIC_RELEASE);
		return;
	}
	self->ra_tid = __HYBRID_GETTID_INVALID;
	__hybrid_atomic_store(&self->ra_lock, 0, __ATOMIC_RELEASE);
}





/************************************************************************/
/* Recursive shared lock (scheduler-level blocking lock)                */
/************************************************************************/
typedef struct {
	Dee_ratomic_lock_t rs_lock;    /* Underlying atomic lock */
	unsigned int       rs_waiting; /* non-zero if threads may be waiting on `rs_lock' */
} Dee_rshared_lock_t;

#define DEE_RSHARED_LOCK_INIT             { DEE_RATOMIC_LOCK_INIT, 0 }
#define Dee_rshared_lock_init(self)       (void)(Dee_ratomic_lock_init(&(self)->rs_lock), (self)->rs_waiting = 0)
#define Dee_rshared_lock_cinit(self)      (void)(Dee_ratomic_lock_cinit(&(self)->rs_lock), Dee_ASSERT((self)->rs_waiting == 0))
#define Dee_rshared_lock_available(self)  Dee_ratomic_lock_available(&(self)->rs_lock)
#define Dee_rshared_lock_acquired(self)   Dee_ratomic_lock_acquired(&(self)->rs_lock)
#define Dee_rshared_lock_tryacquire(self) Dee_ratomic_lock_tryacquire(&(self)->rs_lock)

/* Block until successfully acquired a recursive shared lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_lock_acquire(Dee_rshared_lock_t *__restrict self) {
	int result;
	unsigned int lockword;
	lockword = __hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
		if (!__hybrid_atomic_cmpxch(&self->rs_lock.ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto waitfor;
settid:
		self->rs_lock.ra_tid = __hybrid_gettid();
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid)) {
		__hybrid_atomic_inc(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE);
		return 0;
	}
waitfor:
	result = DeeFutex_WaitInt(&self->rs_lock.ra_lock, lockword);
	if unlikely(result != 0)
		return result;
	if (__hybrid_atomic_cmpxch(&self->rs_lock.ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		goto settid;
	goto waitfor;
}

LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_lock_waitfor(Dee_rshared_lock_t *__restrict self) {
	unsigned int lockword;
	if (__hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE) == 0)
		return 0;
	if (__hybrid_gettid_iscaller(self->rs_lock.ra_tid))
		return 0;
	while ((lockword = __hybrid_atomic_load(&self->rs_lock.ra_lock, __ATOMIC_ACQUIRE)) != 0) {
		int result = DeeFutex_WaitInt(&self->rs_lock.ra_lock, lockword);
		if unlikely(result != 0)
			return result;
	}
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
#define Dee_rshared_lock_release Dee_rshared_lock_release
LOCAL NONNULL((1)) void DCALL
Dee_rshared_lock_release(Dee_rshared_lock_t *__restrict self) {
	Dee_ASSERT(self->rs_lock.ra_lock > 0);
	Dee_ASSERT(__hybrid_gettid_iscaller(self->rs_lock.ra_tid));
	if (self->rs_lock.ra_lock > 1) {
		__hybrid_atomic_dec(&self->rs_lock.ra_lock, __ATOMIC_RELEASE);
		return;
	}
	self->rs_lock.ra_tid = __HYBRID_GETTID_INVALID;
	__hybrid_atomic_store(&self->rs_lock.ra_lock, 0, __ATOMIC_RELEASE);
	if (__hybrid_atomic_load(&self->rs_waiting, __ATOMIC_ACQUIRE) > 0)
		DeeFutex_WakeOne(&self->rs_lock.ra_lock);
}




/************************************************************************/
/* Recursive atomic rwlock                                              */
/************************************************************************/
typedef struct {
	Dee_atomic_rwlock_t rarw_lock;   /* Underlying atomic read/write lock */
	__hybrid_tid_t      rarw_tid;    /* [valid_if(rarw_lock.arw_lock == (uintptr_t)-1)] Write-lock owner (set to `__HYBRID_GETTID_INVALID' after fully releasing lock) */
	unsigned int        rarw_nwrite; /* [valid_if(rarw_lock.arw_lock == (uintptr_t)-1)] Number of extra write-locks (`0' means that the next `endwrite()' will release the primary write-lock) */
} Dee_ratomic_rwlock_t;
#define DEE_RATOMIC_RWLOCK_INIT        { DEE_ATOMIC_RWLOCK_INIT, __HYBRID_GETTID_INVALID, 0 }
#define Dee_ratomic_rwlock_init(self)  (void)(atomic_rwlock_init(&(self)->rarw_lock), (self)->rarw_tid = __HYBRID_GETTID_INVALID, (self)->rarw_nwrite = 0)
#ifdef __HYBRID_GETTID_INVALID_IS_ZERO
#define Dee_ratomic_rwlock_cinit(self) (void)(atomic_rwlock_cinit(&(self)->rarw_lock), Dee_ASSERT((self)->rarw_tid == __HYBRID_GETTID_INVALID), Dee_ASSERT((self)->rarw_nwrite == 0))
#else /* __HYBRID_GETTID_INVALID_IS_ZERO */
#define Dee_ratomic_rwlock_cinit(self) (void)(atomic_rwlock_cinit(&(self)->rarw_lock), (self)->rarw_tid = __HYBRID_GETTID_INVALID, Dee_ASSERT((self)->rarw_nwrite == 0))
#endif /* !__HYBRID_GETTID_INVALID_IS_ZERO */
#define Dee_ratomic_rwlock_reading(self)  Dee_atomic_rwlock_reading(&(self)->rarw_lock)
#define Dee_ratomic_rwlock_writing(self)  Dee_atomic_rwlock_writing(&(self)->rarw_lock)
#define Dee_ratomic_rwlock_tryread(self)  Dee_atomic_rwlock_tryread(&(self)->rarw_lock)
#define Dee_ratomic_rwlock_canread(self)  Dee_atomic_rwlock_canread(&(self)->rarw_lock)
#define Dee_ratomic_rwlock_waitread(self) Dee_atomic_rwlock_waitread(&(self)->rarw_lock)
#define Dee_ratomic_rwlock_read(self)     Dee_atomic_rwlock_read(&(self)->rarw_lock)
#define Dee_ratomic_rwlock_endread(self)  Dee_atomic_rwlock_endread(&(self)->rarw_lock)

/* Try to acquire a write-lock to `self' */
#define Dee_ratomic_rwlock_trywrite Dee_ratomic_rwlock_trywrite
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

/* Check if trying to start writing right now is non-blocking */
#define Dee_ratomic_rwlock_canwrite Dee_ratomic_rwlock_canwrite
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

/* Acquire a write-lock to `self' */
#define Dee_ratomic_rwlock_write Dee_ratomic_rwlock_write
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_write(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
again_lockword_zero:
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
		self->rarw_tid = __hybrid_gettid();
		return;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return;
		}
	}
	while ((lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE)) != 0)
		__hybrid_yield();
	goto again_lockword_zero;
}

/* Wait until acquiring a write-lock to `self' is non-blocking */
#define Dee_ratomic_rwlock_waitwrite Dee_ratomic_rwlock_waitwrite
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
#define Dee_ratomic_rwlock_tryupgrade Dee_ratomic_rwlock_tryupgrade
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_tryupgrade(Dee_ratomic_rwlock_t *__restrict self) {
#ifndef __OPTIMIZE_SIZE__
	if (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != 1)
		return false;
#endif /* !__OPTIMIZE_SIZE__ */
	if (!__hybrid_atomic_cmpxch(&self->rarw_lock.arw_lock, 1, (uintptr_t)-1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		return false;
	self->rarw_tid = __hybrid_gettid();
	return true;
}

/* Upgrade a read-lock into a write-lock */
#define Dee_ratomic_rwlock_upgrade Dee_ratomic_rwlock_upgrade
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_ratomic_rwlock_upgrade(Dee_ratomic_rwlock_t *__restrict self) {
	if (Dee_ratomic_rwlock_tryupgrade(self))
		return 1;
	Dee_ratomic_rwlock_endread(self);
	Dee_ratomic_rwlock_write(self);
	return 0;
}

/* Downgrade a write-lock into a read-lock */
#define Dee_ratomic_rwlock_downgrade Dee_ratomic_rwlock_downgrade
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_downgrade(Dee_ratomic_rwlock_t *__restrict self) {
	Dee_ASSERTF(self->rarw_lock.arw_lock == (uintptr_t)-1, "Lock not in write-mode");
	Dee_ASSERTF(self->rarw_nwrite == 0, "Recursive write-locks are present");
	Dee_ASSERTF(__hybrid_gettid_iscaller(self->rarw_tid), "You're not the write-holder");
	self->rarw_tid = __HYBRID_GETTID_INVALID;
	__hybrid_atomic_store(&self->rarw_lock.arw_lock, 1, __ATOMIC_RELEASE);
}

/* Release a write-lock */
#define Dee_ratomic_rwlock_endwrite Dee_ratomic_rwlock_endwrite
LOCAL NONNULL((1)) void DCALL
Dee_ratomic_rwlock_endwrite(Dee_ratomic_rwlock_t *__restrict self) {
	Dee_ASSERTF(self->rarw_lock.arw_lock == (uintptr_t)-1, "Lock not in write-mode");
	Dee_ASSERTF(__hybrid_gettid_iscaller(self->rarw_tid), "You're not the write-holder");
	if (self->rarw_nwrite > 0) {
		--self->rarw_nwrite;
		return;
	}
	self->rarw_tid = __HYBRID_GETTID_INVALID;
	__hybrid_atomic_store(&self->rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
}

/* Release a read- or write-lock */
#define Dee_ratomic_rwlock_end Dee_ratomic_rwlock_end
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




/************************************************************************/
/* Recursive shared rwlock (scheduler-level blocking lock)              */
/************************************************************************/
typedef struct {
	Dee_ratomic_rwlock_t srrw_lock;    /* Underlying recursive atomic read/write lock */
	unsigned int         srrw_waiting; /* non-zero if threads may be waiting on `srrw_lock.rarw_lock.arw_lock' */
} Dee_rshared_rwlock_t;
#define DEE_RSHARED_RWLOCK_INIT        { DEE_RATOMIC_RWLOCK_INIT, 0 }
#define Dee_rshared_rwlock_init(self)  (void)(Dee_ratomic_rwlock_init(&(self)->srrw_lock), (self)->srrw_waiting = 0)
#define Dee_rshared_rwlock_cinit(self) (void)(Dee_ratomic_rwlock_cinit(&(self)->srrw_lock), Dee_ASSERT((self)->srrw_waiting == 0))
#define _Dee_rshared_rwlock_wake(self)                                     \
	(__hybrid_atomic_load(&(self)->srrw_waiting, __ATOMIC_ACQUIRE)         \
	 ? (__hybrid_atomic_store(&(self)->srrw_waiting, 0, __ATOMIC_RELEASE), \
	    DeeFutex_WakeAll(&(self)->srrw_lock.rarw_lock.arw_lock))           \
	 : (void)0)
#define Dee_rshared_rwlock_reading(self)    Dee_ratomic_rwlock_reading(&(self)->srrw_lock)
#define Dee_rshared_rwlock_writing(self)    Dee_ratomic_rwlock_writing(&(self)->srrw_lock)
#define Dee_rshared_rwlock_tryread(self)    Dee_ratomic_rwlock_tryread(&(self)->srrw_lock)
#define Dee_rshared_rwlock_trywrite(self)   Dee_ratomic_rwlock_trywrite(&(self)->srrw_lock)
#define Dee_rshared_rwlock_canread(self)    Dee_ratomic_rwlock_canread(&(self)->srrw_lock)
#define Dee_rshared_rwlock_canwrite(self)   Dee_ratomic_rwlock_canwrite(&(self)->srrw_lock)
#define Dee_rshared_rwlock_tryupgrade(self) Dee_ratomic_rwlock_tryupgrade(&(self)->srrw_lock)
#define Dee_rshared_rwlock_downgrade(self)  (Dee_ratomic_rwlock_downgrade(&(self)->srrw_lock), _Dee_rshared_rwlock_wake(self))
#define Dee_rshared_rwlock_endwrite(self)   (Dee_ratomic_rwlock_endwrite(&(self)->srrw_lock), _Dee_rshared_rwlock_wake(self))

/* Acquire a read-lock to `self' */
#define Dee_rshared_rwlock_read Dee_rshared_rwlock_read
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_read(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword != (uintptr_t)-1) {
again_lockword_not_UINTPTR_MAX:
		Dee_ASSERTF(lockword != (uintptr_t)-2, "Too many read-locks");
		if (__hybrid_atomic_cmpxch_weak(&self->srrw_lock.rarw_lock.arw_lock,
		                                lockword, lockword + 1,
		                                __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			return 0;
		goto again;
	}
	if (__hybrid_gettid_iscaller(self->srrw_lock.rarw_tid)) {
		/* Special case for read-after-write */
		++self->srrw_lock.rarw_nwrite;
		return 0;
	}
	do {
		int result = DeeFutex_WaitPtr(&self->srrw_lock.rarw_lock.arw_lock, lockword);
		if unlikely(result != 0)
			return result;
	} while ((lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock,
	                                          __ATOMIC_ACQUIRE)) == (uintptr_t)-1);
	goto again_lockword_not_UINTPTR_MAX;
}

/* Acquire a write-lock to `self' */
#define Dee_rshared_rwlock_write Dee_rshared_rwlock_write
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_write(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
again_lockword_zero:
		if (!__hybrid_atomic_cmpxch_weak(&self->srrw_lock.rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
		self->srrw_lock.rarw_tid = __hybrid_gettid();
		return 0;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->srrw_lock.rarw_tid)) {
			++self->srrw_lock.rarw_nwrite;
			return 0;
		}
	}
	do {
		int result = DeeFutex_WaitPtr(&self->srrw_lock.rarw_lock.arw_lock, lockword);
		if unlikely(result != 0)
			return result;
	} while ((lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock,
	                                          __ATOMIC_ACQUIRE)) != 0);
	goto again_lockword_zero;
}

/* Wait until acquiring a read-lock to `self' no longer blocks */
#define Dee_rshared_rwlock_waitread Dee_rshared_rwlock_waitread
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_waitread(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->srrw_lock.rarw_tid)) {
			/* Special case for read-after-write */
			return 0;
		}
		do {
			int result = DeeFutex_WaitPtr(&self->srrw_lock.rarw_lock.arw_lock, lockword);
			if unlikely(result != 0)
				return result;
		} while ((lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock,
		                                          __ATOMIC_ACQUIRE)) == (uintptr_t)-1);
	}
	return 0;
}

/* Wait until acquiring a write-lock to `self' no longer blocks */
#define Dee_rshared_rwlock_waitwrite Dee_rshared_rwlock_waitwrite
LOCAL WUNUSED NONNULL((1)) int DCALL
Dee_rshared_rwlock_waitwrite(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return 0;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->srrw_lock.rarw_tid))
			return 0;
	}
	do {
		int result = DeeFutex_WaitPtr(&self->srrw_lock.rarw_lock.arw_lock, lockword);
		if unlikely(result != 0)
			return result;
	} while ((lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE)) != 0);
	return 0;
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


/* Release a read-lock */
#define Dee_rshared_rwlock_endread Dee_rshared_rwlock_endread
LOCAL NONNULL((1)) void DCALL
Dee_rshared_rwlock_endread(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->srrw_lock.rarw_tid),
		            "You're not the write-holder, so you couldn't have done read-after-write");
		Dee_ASSERTF(self->srrw_lock.rarw_nwrite > 0,
		            "No recursive write-locks, so this can't be read-after-write");
		--self->srrw_lock.rarw_nwrite;
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		__hybrid_atomic_dec(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_RELEASE);
		if (lockword == 1)
			_Dee_rshared_rwlock_wake(self); /* Last read-lock went away */
	}
}

/* Release a read- or write-lock */
#define Dee_rshared_rwlock_end Dee_rshared_rwlock_end
LOCAL NONNULL((1)) void DCALL
Dee_rshared_rwlock_end(Dee_rshared_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == (uintptr_t)-1) {
		Dee_ASSERTF(__hybrid_gettid_iscaller(self->srrw_lock.rarw_tid), "You're not the write-holder");
		if (self->srrw_lock.rarw_nwrite > 0) {
			--self->srrw_lock.rarw_nwrite;
			return;
		}
		self->srrw_lock.rarw_tid = __HYBRID_GETTID_INVALID;
		__hybrid_atomic_store(&self->srrw_lock.rarw_lock.arw_lock, 0, __ATOMIC_RELEASE);
		_Dee_rshared_rwlock_wake(self); /* Last write-lock went away */
	} else {
		Dee_ASSERTF(lockword != 0, "No lock are held");
		__hybrid_atomic_dec(&self->srrw_lock.rarw_lock.arw_lock, __ATOMIC_RELEASE);
		if (lockword == 1)
			_Dee_rshared_rwlock_wake(self); /* Last read-lock went away */
	}
}

/* Upgrade a read-lock into a write-lock */
#define Dee_rshared_rwlock_upgrade Dee_rshared_rwlock_upgrade
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_rshared_rwlock_upgrade(Dee_rshared_rwlock_t *__restrict self) {
	if (Dee_rshared_rwlock_tryupgrade(self))
		return 1;
	Dee_rshared_rwlock_endread(self);
	Dee_rshared_rwlock_write(self);
	return 0;
}


DECL_END

#endif /* !CONFIG_NO_THREADS */

#endif /* !GUARD_DEEMON_UTIL_RLOCK_H */
