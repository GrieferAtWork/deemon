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
#ifndef GUARD_DEEMON_UTIL_LOCK_H
#define GUARD_DEEMON_UTIL_LOCK_H 1

#include "../api.h"

/*
 * Dee_atomic_lock_t: Atomic lock (sched_yield() until available; no interrupt checks)
 * Dee_shared_lock_t: Shared lock (blocking wait; w/ interrupt checks)
 * Dee_atomic_rwlock_t: Like Dee_atomic_lock_t, but allows for read- and write-locks
 * Dee_shared_rwlock_t: Like Dee_shared_lock_t, but allows for read- and write-locks
 */

#ifdef CONFIG_NO_THREADS
DECL_BEGIN

typedef int Dee_atomic_lock_t;
#define DEE_ATOMIC_LOCK_INIT             0
#define Dee_atomic_lock_cinit(self)      (void)0
#define Dee_atomic_lock_init(self)       (void)0
#define Dee_atomic_lock_available(x)     1
#define Dee_atomic_lock_acquired(x)      1
#define Dee_atomic_lock_tryacquire(self) 1
#define Dee_atomic_lock_acquire(self)    (void)0
#define Dee_atomic_lock_waitfor(self)    (void)0
#define Dee_atomic_lock_release(self)    (void)0

typedef int Dee_atomic_rwlock_t;
#define DEE_ATOMIC_RWLOCK_INIT             0
#define Dee_atomic_rwlock_cinit(self)      (void)0
#define Dee_atomic_rwlock_init(self)       (void)0
#define Dee_atomic_rwlock_reading(x)       1
#define Dee_atomic_rwlock_writing(x)       1
#define Dee_atomic_rwlock_tryread(self)    1
#define Dee_atomic_rwlock_trywrite(self)   1
#define Dee_atomic_rwlock_canread(self)    1
#define Dee_atomic_rwlock_canwrite(self)   1
#define Dee_atomic_rwlock_waitread(self)   1
#define Dee_atomic_rwlock_waitwrite(self)  1
#define Dee_atomic_rwlock_read(self)       (void)0
#define Dee_atomic_rwlock_write(self)      (void)0
#define Dee_atomic_rwlock_tryupgrade(self) 1
#define Dee_atomic_rwlock_upgrade(self)    1
#define Dee_atomic_rwlock_downgrade(self)  (void)0
#define Dee_atomic_rwlock_endwrite(self)   (void)0
#define Dee_atomic_rwlock_endread(self)    (void)0
#define Dee_atomic_rwlock_end(self)        (void)0

/* No-op out the futex API (with only a single thread, the below is
 * actually equivalent, so-long as you assume that dead-locks are
 * impossible, too) */
#define DeeFutex_WakeOne(addr)                                     (void)0
#define DeeFutex_WakeAll(addr)                                     (void)0
#define DeeFutex_Wait32(addr, expected)                            0
#define DeeFutex_Wait32Timed(addr, expected, timeout_nanoseconds)  0
#define DeeFutex_WaitPtr(addr, expected)                           0
#define DeeFutex_WaitPtrTimed(addr, expected, timeout_nanoseconds) 0
#if __SIZEOF_POINTER__ >= 8
#define DeeFutex_Wait64(addr, expected)                           0
#define DeeFutex_Wait64Timed(addr, expected, timeout_nanoseconds) 0
#endif /* __SIZEOF_POINTER__ >= 8 */

typedef int Dee_shared_lock_t;
#define DEE_SHARED_LOCK_INIT                                     0
#define DEE_SHARED_LOCK_INIT_LOCKED                              0
#define Dee_shared_lock_cinit(self)                              (void)0
#define Dee_shared_lock_init(self)                               (void)0
#define Dee_shared_lock_cinit_locked(self)                       (void)0
#define Dee_shared_lock_init_locked(self)                        (void)0
#define Dee_shared_lock_available(self)                          1
#define Dee_shared_lock_acquired(self)                           1
#define Dee_shared_lock_tryacquire(self)                         1
#define Dee_shared_lock_release(self)                            (void)0
#define Dee_shared_lock_acquire(self)                            0
#define Dee_shared_lock_waitfor(self)                            0
#define Dee_shared_lock_acquire_timed(self, timeout_nanoseconds) 0
#define Dee_shared_lock_waitfor_timed(self, timeout_nanoseconds) 0
#define Dee_shared_lock_acquire(self)                            0
#define Dee_shared_lock_waitfor(self)                            0

typedef int Dee_shared_rwlock_t;
#define DEE_SHARED_RWLOCK_INIT                                       0
#define DEE_SHARED_RWLOCK_INIT_READ                                  0
#define DEE_SHARED_RWLOCK_INIT_WRITE                                 0
#define Dee_shared_rwlock_init(self)                                 (void)0
#define Dee_shared_rwlock_init_read(self)                            (void)0
#define Dee_shared_rwlock_init_write(self)                           (void)0
#define Dee_shared_rwlock_cinit(self)                                (void)0
#define Dee_shared_rwlock_cinit_read(self)                           (void)0
#define Dee_shared_rwlock_cinit_write(self)                          (void)0
#define Dee_shared_rwlock_reading(self)                              1
#define Dee_shared_rwlock_writing(self)                              1
#define Dee_shared_rwlock_tryread(self)                              1
#define Dee_shared_rwlock_trywrite(self)                             1
#define Dee_shared_rwlock_canread(self)                              1
#define Dee_shared_rwlock_canwrite(self)                             1
#define Dee_shared_rwlock_tryupgrade(self)                           1
#define Dee_shared_rwlock_upgrade(self)                              1
#define Dee_shared_rwlock_downgrade(self)                            (void)0
#define Dee_shared_rwlock_endread(self)                              (void)0
#define Dee_shared_rwlock_endwrite(self)                             (void)0
#define Dee_shared_rwlock_end(self)                                  (void)0
#define Dee_shared_rwlock_read(self)                                 0
#define Dee_shared_rwlock_write(self)                                0
#define Dee_shared_rwlock_waitread(self)                             0
#define Dee_shared_rwlock_waitwrite(self)                            0
#define Dee_shared_rwlock_read_timed(self, timeout_nanoseconds)      0
#define Dee_shared_rwlock_write_timed(self, timeout_nanoseconds)     0
#define Dee_shared_rwlock_waitread_timed(self, timeout_nanoseconds)  0
#define Dee_shared_rwlock_waitwrite_timed(self, timeout_nanoseconds) 0
#define Dee_shared_rwlock_read(self)                                 0
#define Dee_shared_rwlock_write(self)                                0
#define Dee_shared_rwlock_waitread(self)                             0
#define Dee_shared_rwlock_waitwrite(self)                            0

#define DeeLock_Lock2(lock_a, trylock_a, unlock_a, \
                      lock_b, trylock_b, unlock_b) \
	(void)0
#define DeeLock_Lock3(lock_a, trylock_a, unlock_a, \
                      lock_b, trylock_b, unlock_b, \
                      lock_c, trylock_c, unlock_c) \
	(void)0

DECL_END

#else /* CONFIG_NO_THREADS */

#include <hybrid/__atomic.h>
#include <hybrid/sched/atomic-lock.h>
#include <hybrid/sched/atomic-rwlock.h>
#include <hybrid/typecore.h>

#include <stdbool.h>

DECL_BEGIN

/* Simply implement atomic locks using the hybrid-API */
typedef struct atomic_lock Dee_atomic_lock_t;
#define DEE_ATOMIC_LOCK_INIT       ATOMIC_LOCK_INIT
#define Dee_atomic_lock_cinit      atomic_lock_cinit
#define Dee_atomic_lock_init       atomic_lock_init
#define Dee_atomic_lock_available  atomic_lock_available
#define Dee_atomic_lock_acquired   atomic_lock_acquired
#define Dee_atomic_lock_tryacquire atomic_lock_tryacquire
#define Dee_atomic_lock_acquire    atomic_lock_acquire
#define Dee_atomic_lock_waitfor    atomic_lock_waitfor
#define Dee_atomic_lock_release    atomic_lock_release

/* Simply implement atomic R/W-locks using the hybrid-API */
typedef struct atomic_rwlock Dee_atomic_rwlock_t;
#define DEE_ATOMIC_RWLOCK_INIT       ATOMIC_RWLOCK_INIT
#define Dee_atomic_rwlock_cinit      atomic_rwlock_cinit
#define Dee_atomic_rwlock_init       atomic_rwlock_init
#define Dee_atomic_rwlock_reading    atomic_rwlock_reading
#define Dee_atomic_rwlock_writing    atomic_rwlock_writing
#define Dee_atomic_rwlock_tryread    atomic_rwlock_tryread
#define Dee_atomic_rwlock_trywrite   atomic_rwlock_trywrite
#define Dee_atomic_rwlock_canread    atomic_rwlock_canread
#define Dee_atomic_rwlock_canwrite   atomic_rwlock_canwrite
#define Dee_atomic_rwlock_waitread   atomic_rwlock_waitread
#define Dee_atomic_rwlock_waitwrite  atomic_rwlock_waitwrite
#define Dee_atomic_rwlock_read       atomic_rwlock_read
#define Dee_atomic_rwlock_write      atomic_rwlock_write
#define Dee_atomic_rwlock_tryupgrade atomic_rwlock_tryupgrade
#define Dee_atomic_rwlock_upgrade    atomic_rwlock_upgrade
#define Dee_atomic_rwlock_downgrade  atomic_rwlock_downgrade
#define Dee_atomic_rwlock_endwrite   atomic_rwlock_endwrite
#define Dee_atomic_rwlock_endread    atomic_rwlock_endread
#define Dee_atomic_rwlock_end        atomic_rwlock_end

/************************************************************************/
/* Low-level, futex-based wait/wake scheduling.                         */
/************************************************************************/

/* Wake up 1, or all waiting threads at a given address. */
DFUNDEF NONNULL((1)) void (DCALL DeeFutex_WakeOne)(void *addr);
DFUNDEF NONNULL((1)) void (DCALL DeeFutex_WakeAll)(void *addr);

/* Blocking wait if `*(uint32_t *)addr == expected', until someone calls `DeeFutex_Wake*(addr)'
 * @return: 1 : [DeeFutex_Wait32Timed] The given `timeout_nanoseconds' expired.
 * @return: 0 : Success (someone called `DeeFutex_Wake*(addr)', or `*addr != expected', or spurious wake-up)
 * @return: -1: Error (an error was thrown) */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait32)(void *addr, __UINT32_TYPE__ expected);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait32Timed)(void *addr, __UINT32_TYPE__ expected,
                             __UINT64_TYPE__ timeout_nanoseconds);

#if __SIZEOF_POINTER__ >= 8
/* Same as above, but do a 64-bit equals-comparison test. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait64)(void *addr, __UINT64_TYPE__ expected);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait64Timed)(void *addr, __UINT64_TYPE__ expected,
                             __UINT64_TYPE__ timeout_nanoseconds);
#define DeeFutex_WaitPtr      DeeFutex_Wait64
#define DeeFutex_WaitPtrTimed DeeFutex_Wait64Timed
#else /* __SIZEOF_POINTER__ >= 8 */
#define DeeFutex_WaitPtr      DeeFutex_Wait32
#define DeeFutex_WaitPtrTimed DeeFutex_Wait32Timed
#endif /* __SIZEOF_POINTER__ < 8 */


/************************************************************************/
/* Shared lock (scheduler-level blocking lock)                          */
/************************************************************************/
typedef struct {
	__UINT32_TYPE__ sl_lock; /* Lock word (== 0: available, == 1: held, >= 2: someone is waiting) */
} Dee_shared_lock_t;
#define DEE_SHARED_LOCK_INIT               { 0 }
#define DEE_SHARED_LOCK_INIT_LOCKED        { 1 }
#define Dee_shared_lock_cinit(self)        (void)(Dee_ASSERT((self)->sl_lock == 0))
#define Dee_shared_lock_init(self)         (void)((self)->sl_lock = 0)
#define Dee_shared_lock_cinit_locked(self) (void)(Dee_ASSERT((self)->sl_lock == 0), (self)->sl_lock = 1)
#define Dee_shared_lock_init_locked(self)  (void)((self)->sl_lock = 1)
#define Dee_shared_lock_available(self)    (__hybrid_atomic_load(&(self)->sl_lock, __ATOMIC_ACQUIRE) == 0)
#define Dee_shared_lock_acquired(self)     (__hybrid_atomic_load(&(self)->sl_lock, __ATOMIC_ACQUIRE) != 0)
#define Dee_shared_lock_tryacquire(self)   __hybrid_atomic_cmpxch(&(self)->sl_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)

/* Release a shared lock. */
#define Dee_shared_lock_release(self)                                \
	(Dee_ASSERT((self)->sl_lock != 0),                               \
	 __hybrid_atomic_xch(&(self)->sl_lock, 0, __ATOMIC_RELEASE) >= 2 \
	 ? DeeFutex_WakeOne(&(self)->sl_lock)                            \
	 : (void)0)

/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_acquire)(Dee_shared_lock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_waitfor)(Dee_shared_lock_t *__restrict self);

/* Same as `Dee_shared_lock_acquire()' / `Dee_shared_lock_waitfor()',
 * but also takes an additional timeout in nano-seconds. The special
 * values `0' (try-acquire) and `(uint64_t)-1' (infinite timeout) are
 * also recognized for `timeout_nanoseconds'.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_acquire_timed)(Dee_shared_lock_t *__restrict self, __UINT64_TYPE__ timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_waitfor_timed)(Dee_shared_lock_t *__restrict self, __UINT64_TYPE__ timeout_nanoseconds);

#if !defined(__NO_builtin_expect) && !defined(__OPTIMIZE_SIZE__)
#define Dee_shared_lock_acquire(self) __builtin_expect(Dee_shared_lock_tryacquire(self) ? 0 : (Dee_shared_lock_acquire)(self), 0)
#define Dee_shared_lock_waitfor(self) __builtin_expect(Dee_shared_lock_available(self) ? 0 : (Dee_shared_lock_waitfor)(self), 0)
#elif !defined(__NO_builtin_expect)
#define Dee_shared_lock_acquire(self) __builtin_expect((Dee_shared_lock_acquire)(self), 0)
#define Dee_shared_lock_waitfor(self) __builtin_expect((Dee_shared_lock_waitfor)(self), 0)
#elif !defined(__OPTIMIZE_SIZE__)
#define Dee_shared_lock_acquire(self) (Dee_shared_lock_tryacquire(self) ? 0 : (Dee_shared_lock_acquire)(self))
#define Dee_shared_lock_waitfor(self) (Dee_shared_lock_available(self) ? 0 : (Dee_shared_lock_waitfor)(self))
#endif /* !__NO_builtin_expect */




/************************************************************************/
/* Shared r/w-lock (scheduler-level blocking lock)                      */
/************************************************************************/
typedef struct {
	__UINTPTR_TYPE__ sl_lock;    /* # of read-locks, or (uintptr_t)-1 if a write-lock is active. */
	__UINT32_TYPE__  sl_waiting; /* non-zero if threads may be waiting on `sl_lock' */
} Dee_shared_rwlock_t;

#define _Dee_shared_rwlock_wake(self)                                    \
	((self)->sl_waiting                                                  \
	 ? (__hybrid_atomic_store(&(self)->sl_waiting, 0, __ATOMIC_RELEASE), \
	    DeeFutex_WakeAll(&(self)->sl_lock))                              \
	 : (void)0)

#define DEE_SHARED_RWLOCK_INIT              { 0, 0 }
#define DEE_SHARED_RWLOCK_INIT_READ         { 1, 0 }
#define DEE_SHARED_RWLOCK_INIT_WRITE        { (__UINTPTR_TYPE__)-1, 0 }
#define Dee_shared_rwlock_init(self)        (void)((self)->sl_lock = 0, (self)->sl_waiting = 0)
#define Dee_shared_rwlock_init_read(self)   (void)((self)->sl_lock = 1, (self)->sl_waiting = 0)
#define Dee_shared_rwlock_init_write(self)  (void)((self)->sl_lock = (__UINTPTR_TYPE__)-1, (self)->sl_waiting = 0)
#define Dee_shared_rwlock_cinit(self)       (Dee_ASSERT((self)->sl_lock == 0), Dee_ASSERT((self)->sl_waiting == 0))
#define Dee_shared_rwlock_cinit_read(self)  (Dee_ASSERT((self)->sl_lock == 0), (self)->sl_lock = 1, Dee_ASSERT((self)->sl_waiting == 0))
#define Dee_shared_rwlock_cinit_write(self) (Dee_ASSERT((self)->sl_lock == 0), (self)->sl_lock = (__UINTPTR_TYPE__)-1, Dee_ASSERT((self)->sl_waiting == 0))
#define Dee_shared_rwlock_reading(self)     (__hybrid_atomic_load(&(self)->sl_lock, __ATOMIC_ACQUIRE) != 0)
#define Dee_shared_rwlock_writing(self)     (__hybrid_atomic_load(&(self)->sl_lock, __ATOMIC_ACQUIRE) == (__UINTPTR_TYPE__)-1)
#define Dee_shared_rwlock_canread(self)     (__hybrid_atomic_load(&(self)->sl_lock, __ATOMIC_ACQUIRE) != (__UINTPTR_TYPE__)-1)
#define Dee_shared_rwlock_canwrite(self)    (__hybrid_atomic_load(&(self)->sl_lock, __ATOMIC_ACQUIRE) == 0)

#define Dee_shared_rwlock_tryupgrade(self) \
	__hybrid_atomic_cmpxch(&(self)->sl_lock, 1, (__UINTPTR_TYPE__)-1, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)
#define Dee_shared_rwlock_trywrite(self) \
	__hybrid_atomic_cmpxch(&(self)->sl_lock, 0, (__UINTPTR_TYPE__)-1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)
#define Dee_shared_rwlock_tryread Dee_shared_rwlock_tryread
LOCAL WUNUSED ATTR_INOUT(1) bool
(DCALL Dee_shared_rwlock_tryread)(Dee_shared_rwlock_t *__restrict self) {
	__UINTPTR_TYPE__ temp;
	do {
		temp = __hybrid_atomic_load(&self->sl_lock, __ATOMIC_ACQUIRE);
		if (temp == (__UINTPTR_TYPE__)-1)
			return false;
		Dee_ASSERT(temp != (__UINTPTR_TYPE__)-2);
	} while (!__hybrid_atomic_cmpxch_weak(&self->sl_lock, temp, temp + 1,
	                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED));
	__COMPILER_READ_BARRIER();
	return true;
}

#define Dee_shared_rwlock_downgrade(self)                                                                          \
	(void)(Dee_ASSERTF((self)->sl_lock == (__UINTPTR_TYPE__)-1, "Lock isn't in write-mode (%x)", (self)->sl_lock), \
	       __hybrid_atomic_store(&(self)->sl_lock, 1, __ATOMIC_RELEASE),                                           \
	       _Dee_shared_rwlock_rdwait_broadcast(self)) /* Allow for more readers. */

/* Upgrade from a read- to a write-lock.
 * @return: 1 : Success, but the read-lock had to be dropped temporarily (i.e. the upgrade wasn't atomic).
 * @return: 0 : Success.
 * @return: -1: An exception was thrown (in this case, the old read-lock was lost). */
#define Dee_shared_rwlock_upgrade(self) \
	(Dee_shared_rwlock_tryupgrade(self) ? 1 : (Dee_shared_rwlock_endread(self), Dee_shared_rwlock_write(self)))

/* Release a lock of the indicated type. */
#define Dee_shared_rwlock_endread(self)                                                                   \
	(Dee_ASSERTF((self)->sl_lock != (__UINTPTR_TYPE__)-1, "Lock is in write-mode (%x)", (self)->sl_lock), \
	 Dee_ASSERTF((self)->sl_lock != 0, "Lock isn't held by anyone"),                                      \
	 __hybrid_atomic_decfetch(&(self)->sl_lock, __ATOMIC_RELEASE) == 0                                    \
	 ? _Dee_shared_rwlock_wake(self)                                                                      \
	 : (void)0)
#define Dee_shared_rwlock_endwrite(self)                                                                           \
	(void)(Dee_ASSERTF((self)->sl_lock == (__UINTPTR_TYPE__)-1, "Lock isn't in write-mode (%x)", (self)->sl_lock), \
	       __hybrid_atomic_store(&(self)->sl_lock, 0, __ATOMIC_RELEASE),                                           \
	       _Dee_shared_rwlock_wake(self))
DFUNDEF NONNULL((1)) void(DCALL Dee_shared_rwlock_end)(Dee_shared_rwlock_t *__restrict self);

/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_read)(Dee_shared_rwlock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_write)(Dee_shared_rwlock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitread)(Dee_shared_rwlock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitwrite)(Dee_shared_rwlock_t *__restrict self);


/* Same as `Dee_shared_rwlock_*', but also takes an additional timeout in nano-seconds.
 * The special values `0' (try-acquire) and `(uint64_t)-1' (infinite timeout) are also
 * recognized for `timeout_nanoseconds'.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_read_timed)(Dee_shared_rwlock_t *__restrict self, __UINT64_TYPE__ timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_write_timed)(Dee_shared_rwlock_t *__restrict self, __UINT64_TYPE__ timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitread_timed)(Dee_shared_rwlock_t *__restrict self, __UINT64_TYPE__ timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitwrite_timed)(Dee_shared_rwlock_t *__restrict self, __UINT64_TYPE__ timeout_nanoseconds);

#if !defined(__NO_builtin_expect) && !defined(__OPTIMIZE_SIZE__)
#define Dee_shared_rwlock_read(self)      __builtin_expect(Dee_shared_rwlock_tryread(self) ? 0 : (Dee_shared_rwlock_read)(self), 0)
#define Dee_shared_rwlock_write(self)     __builtin_expect(Dee_shared_rwlock_trywrite(self) ? 0 : (Dee_shared_rwlock_write)(self), 0)
#define Dee_shared_rwlock_waitread(self)  __builtin_expect(Dee_shared_rwlock_canread(self) ? 0 : (Dee_shared_rwlock_waitread)(self), 0)
#define Dee_shared_rwlock_waitwrite(self) __builtin_expect(Dee_shared_rwlock_canwrite(self) ? 0 : (Dee_shared_rwlock_waitwrite)(self), 0)
#elif !defined(__NO_builtin_expect)
#define Dee_shared_rwlock_read(self)      __builtin_expect((Dee_shared_rwlock_read)(self), 0)
#define Dee_shared_rwlock_write(self)     __builtin_expect((Dee_shared_rwlock_write)(self), 0)
#define Dee_shared_rwlock_waitread(self)  __builtin_expect((Dee_shared_rwlock_waitread)(self), 0)
#define Dee_shared_rwlock_waitwrite(self) __builtin_expect((Dee_shared_rwlock_waitwrite)(self), 0)
#elif !defined(__OPTIMIZE_SIZE__)
#define Dee_shared_rwlock_read(self)      (Dee_shared_rwlock_tryread(self) ? 0 : (Dee_shared_rwlock_read)(self))
#define Dee_shared_rwlock_write(self)     (Dee_shared_rwlock_trywrite(self) ? 0 : (Dee_shared_rwlock_write)(self))
#define Dee_shared_rwlock_waitread(self)  (Dee_shared_rwlock_canread(self) ? 0 : (Dee_shared_rwlock_waitread)(self))
#define Dee_shared_rwlock_waitwrite(self) (Dee_shared_rwlock_canwrite(self) ? 0 : (Dee_shared_rwlock_waitwrite)(self))
#endif /* !__NO_builtin_expect */

/* Helper macros to safely (i.e. without any chance of dead-locking
 * (so-long as locks are distinct)) acquire multiple locks at once. */
#define DeeLock_Lock2(lock_a, trylock_a, unlock_a, \
                      lock_b, trylock_b, unlock_b) \
	do {                                           \
		lock_a;                                    \
		if likely(trylock_b)                       \
			break;                                 \
		unlock_a;                                  \
		lock_b;                                    \
		if likely(trylock_a)                       \
			break;                                 \
		unlock_b;                                  \
	}	__WHILE1
#define DeeLock_Lock3(lock_a, trylock_a, unlock_a,  \
                      lock_b, trylock_b, unlock_b,  \
                      lock_c, trylock_c, unlock_c)  \
	do {                                            \
		DeeLock_Lock2(lock_a, trylock_a, unlock_a,  \
		              lock_b, trylock_b, unlock_b); \
		if likely(trylock_c)                        \
			break;                                  \
		unlock_b;                                   \
		unlock_a;                                   \
		lock_c;                                     \
		if unlikely(!(trylock_b)) {                 \
			unlock_c;                               \
			continue;                               \
		}                                           \
		if likely(trylock_a)                        \
			break;                                  \
		unlock_b;                                   \
		unlock_c;                                   \
	}	__WHILE1
#endif /* !CONFIG_NO_THREADS */

/* Helpers to (safely) acquire multiple atomic [rw]locks at the same time. */
#define Dee_atomic_lock_acquire_2(a, b)                                                                  \
	DeeLock_Lock2(Dee_atomic_lock_acquire(a), Dee_atomic_lock_tryacquire(a), Dee_atomic_lock_release(a), \
	              Dee_atomic_lock_acquire(b), Dee_atomic_lock_tryacquire(b), Dee_atomic_lock_release(b))
#define Dee_atomic_rwlock_read_2(a, b)                                                                   \
	DeeLock_Lock2(Dee_atomic_rwlock_read(a), Dee_atomic_rwlock_tryread(a), Dee_atomic_rwlock_endread(a), \
	              Dee_atomic_rwlock_read(b), Dee_atomic_rwlock_tryread(b), Dee_atomic_rwlock_endread(b))
#define Dee_atomic_rwlock_write_2(a, b)                                                                     \
	DeeLock_Lock2(Dee_atomic_rwlock_write(a), Dee_atomic_rwlock_trywrite(a), Dee_atomic_rwlock_endwrite(a), \
	              Dee_atomic_rwlock_write(b), Dee_atomic_rwlock_trywrite(b), Dee_atomic_rwlock_endwrite(b))
#define Dee_atomic_lock_acquire_3(a, b, c)                                                               \
	DeeLock_Lock3(Dee_atomic_lock_acquire(a), Dee_atomic_lock_tryacquire(a), Dee_atomic_lock_release(a), \
	              Dee_atomic_lock_acquire(b), Dee_atomic_lock_tryacquire(b), Dee_atomic_lock_release(b), \
	              Dee_atomic_lock_acquire(c), Dee_atomic_lock_tryacquire(c), Dee_atomic_lock_release(c))
#define Dee_atomic_rwlock_read_3(a, b, c)                                                                \
	DeeLock_Lock3(Dee_atomic_rwlock_read(a), Dee_atomic_rwlock_tryread(a), Dee_atomic_rwlock_endread(a), \
	              Dee_atomic_rwlock_read(b), Dee_atomic_rwlock_tryread(b), Dee_atomic_rwlock_endread(b), \
	              Dee_atomic_rwlock_read(c), Dee_atomic_rwlock_tryread(c), Dee_atomic_rwlock_endread(c))
#define Dee_atomic_rwlock_write_3(a, b, c)                                                                  \
	DeeLock_Lock3(Dee_atomic_rwlock_write(a), Dee_atomic_rwlock_trywrite(a), Dee_atomic_rwlock_endwrite(a), \
	              Dee_atomic_rwlock_write(b), Dee_atomic_rwlock_trywrite(b), Dee_atomic_rwlock_endwrite(b), \
	              Dee_atomic_rwlock_write(c), Dee_atomic_rwlock_trywrite(c), Dee_atomic_rwlock_endwrite(c))




/* Unescaped symbol aliases */
#ifdef DEE_SOURCE
typedef Dee_atomic_lock_t atomic_lock_t;
#if !defined(ATOMIC_LOCK_INIT) || defined(CONFIG_NO_THREADS)
#undef ATOMIC_LOCK_INIT
#undef atomic_lock_cinit
#undef atomic_lock_init
#undef atomic_lock_available
#undef atomic_lock_acquired
#undef atomic_lock_tryacquire
#undef atomic_lock_acquire
#undef atomic_lock_waitfor
#undef atomic_lock_release
#define ATOMIC_LOCK_INIT       DEE_ATOMIC_LOCK_INIT
#define atomic_lock_cinit      Dee_atomic_lock_cinit
#define atomic_lock_init       Dee_atomic_lock_init
#define atomic_lock_available  Dee_atomic_lock_available
#define atomic_lock_acquired   Dee_atomic_lock_acquired
#define atomic_lock_tryacquire Dee_atomic_lock_tryacquire
#define atomic_lock_acquire    Dee_atomic_lock_acquire
#define atomic_lock_waitfor    Dee_atomic_lock_waitfor
#define atomic_lock_release    Dee_atomic_lock_release
#endif /* !ATOMIC_LOCK_INIT || CONFIG_NO_THREADS */

typedef Dee_atomic_rwlock_t atomic_rwlock_t;
#if !defined(ATOMIC_RWLOCK_INIT) || defined(CONFIG_NO_THREADS)
#undef ATOMIC_RWLOCK_INIT
#undef atomic_rwlock_cinit
#undef atomic_rwlock_init
#undef atomic_rwlock_reading
#undef atomic_rwlock_writing
#undef atomic_rwlock_tryread
#undef atomic_rwlock_trywrite
#undef atomic_rwlock_canread
#undef atomic_rwlock_canwrite
#undef atomic_rwlock_waitread
#undef atomic_rwlock_waitwrite
#undef atomic_rwlock_read
#undef atomic_rwlock_write
#undef atomic_rwlock_tryupgrade
#undef atomic_rwlock_upgrade
#undef atomic_rwlock_downgrade
#undef atomic_rwlock_endwrite
#undef atomic_rwlock_endread
#undef atomic_rwlock_end
#define ATOMIC_RWLOCK_INIT       DEE_ATOMIC_RWLOCK_INIT
#define atomic_rwlock_cinit      Dee_atomic_rwlock_cinit
#define atomic_rwlock_init       Dee_atomic_rwlock_init
#define atomic_rwlock_reading    Dee_atomic_rwlock_reading
#define atomic_rwlock_writing    Dee_atomic_rwlock_writing
#define atomic_rwlock_tryread    Dee_atomic_rwlock_tryread
#define atomic_rwlock_trywrite   Dee_atomic_rwlock_trywrite
#define atomic_rwlock_canread    Dee_atomic_rwlock_canread
#define atomic_rwlock_canwrite   Dee_atomic_rwlock_canwrite
#define atomic_rwlock_waitread   Dee_atomic_rwlock_waitread
#define atomic_rwlock_waitwrite  Dee_atomic_rwlock_waitwrite
#define atomic_rwlock_read       Dee_atomic_rwlock_read
#define atomic_rwlock_write      Dee_atomic_rwlock_write
#define atomic_rwlock_tryupgrade Dee_atomic_rwlock_tryupgrade
#define atomic_rwlock_upgrade    Dee_atomic_rwlock_upgrade
#define atomic_rwlock_downgrade  Dee_atomic_rwlock_downgrade
#define atomic_rwlock_endwrite   Dee_atomic_rwlock_endwrite
#define atomic_rwlock_endread    Dee_atomic_rwlock_endread
#define atomic_rwlock_end        Dee_atomic_rwlock_end
#endif /* !ATOMIC_RWLOCK_INIT || CONFIG_NO_THREADS */

typedef Dee_shared_lock_t shared_lock_t;
#undef SHARED_LOCK_INIT
#undef shared_lock_cinit
#undef shared_lock_init
#undef shared_lock_available
#undef shared_lock_acquired
#undef shared_lock_tryacquire
#undef shared_lock_acquire
#undef shared_lock_waitfor
#undef shared_lock_release
#define SHARED_LOCK_INIT       DEE_SHARED_LOCK_INIT
#define shared_lock_cinit      Dee_shared_lock_cinit
#define shared_lock_init       Dee_shared_lock_init
#define shared_lock_available  Dee_shared_lock_available
#define shared_lock_acquired   Dee_shared_lock_acquired
#define shared_lock_tryacquire Dee_shared_lock_tryacquire
#define shared_lock_acquire    Dee_shared_lock_acquire
#define shared_lock_waitfor    Dee_shared_lock_waitfor
#define shared_lock_release    Dee_shared_lock_release

typedef Dee_shared_rwlock_t shared_rwlock_t;
#undef SHARED_RWLOCK_INIT
#undef shared_rwlock_cinit
#undef shared_rwlock_init
#undef shared_rwlock_reading
#undef shared_rwlock_writing
#undef shared_rwlock_tryread
#undef shared_rwlock_trywrite
#undef shared_rwlock_canread
#undef shared_rwlock_canwrite
#undef shared_rwlock_waitread
#undef shared_rwlock_waitwrite
#undef shared_rwlock_read
#undef shared_rwlock_write
#undef shared_rwlock_tryupgrade
#undef shared_rwlock_upgrade
#undef shared_rwlock_downgrade
#undef shared_rwlock_endwrite
#undef shared_rwlock_endread
#undef shared_rwlock_end
#define SHARED_RWLOCK_INIT       DEE_SHARED_RWLOCK_INIT
#define shared_rwlock_cinit      Dee_shared_rwlock_cinit
#define shared_rwlock_init       Dee_shared_rwlock_init
#define shared_rwlock_reading    Dee_shared_rwlock_reading
#define shared_rwlock_writing    Dee_shared_rwlock_writing
#define shared_rwlock_tryread    Dee_shared_rwlock_tryread
#define shared_rwlock_trywrite   Dee_shared_rwlock_trywrite
#define shared_rwlock_canread    Dee_shared_rwlock_canread
#define shared_rwlock_canwrite   Dee_shared_rwlock_canwrite
#define shared_rwlock_waitread   Dee_shared_rwlock_waitread
#define shared_rwlock_waitwrite  Dee_shared_rwlock_waitwrite
#define shared_rwlock_read       Dee_shared_rwlock_read
#define shared_rwlock_write      Dee_shared_rwlock_write
#define shared_rwlock_tryupgrade Dee_shared_rwlock_tryupgrade
#define shared_rwlock_upgrade    Dee_shared_rwlock_upgrade
#define shared_rwlock_downgrade  Dee_shared_rwlock_downgrade
#define shared_rwlock_endwrite   Dee_shared_rwlock_endwrite
#define shared_rwlock_endread    Dee_shared_rwlock_endread
#define shared_rwlock_end        Dee_shared_rwlock_end

DECL_END
#endif /* DEE_SOURCE */

#endif /* !GUARD_DEEMON_UTIL_LOCK_H */
