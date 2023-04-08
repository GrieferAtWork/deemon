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

typedef int Dee_semaphore_t;
#define DEE_SEMAPHORE_INIT(n_tickets)                          0
#define Dee_semaphore_init(self, n_tickets)                    (void)0
#define Dee_semaphore_cinit(self, n_tickets)                   (void)0
#define Dee_semaphore_haswaiting(self)                         0
#define Dee_semaphore_hastickets(self)                         1
#define Dee_semaphore_gettickets(self)                         1
#define Dee_semaphore_release(self, count)                     (void)0
#define Dee_semaphore_tryacquire(self)                         1
#define Dee_semaphore_waitfor(self)                            0
#define Dee_semaphore_waitfor_timed(self, timeout_nanoseconds) 0
#define Dee_semaphore_acquire(self)                            0
#define Dee_semaphore_acquire_timed(self, timeout_nanoseconds) 0

#define DeeLock_Lock2(lock_a, trylock_a, unlock_a, \
                      lock_b, trylock_b, unlock_b) \
	(void)0
#define DeeLock_Lock3(lock_a, trylock_a, unlock_a, \
                      lock_b, trylock_b, unlock_b, \
                      lock_c, trylock_c, unlock_c) \
	(void)0

DECL_END

#else /* CONFIG_NO_THREADS */

#include "futex.h"
/**/

#include <hybrid/__atomic.h>
#include <hybrid/sched/atomic-lock.h>
#include <hybrid/sched/atomic-rwlock.h>

#include <stdbool.h>
#include <stdint.h>

DECL_BEGIN

/* Simply implement atomic locks using the hybrid-API */
typedef struct atomic_lock Dee_atomic_lock_t;
#define DEE_ATOMIC_LOCK_INIT            ATOMIC_LOCK_INIT
#define DEE_ATOMIC_LOCK_INIT_ACQUIRED   ATOMIC_LOCK_INIT_ACQUIRED
#define Dee_atomic_lock_init            atomic_lock_init
#define Dee_atomic_lock_init_acquired   atomic_lock_init_acquired
#define Dee_atomic_lock_cinit           atomic_lock_cinit
#define Dee_atomic_lock_cinit_acquired  atomic_lock_cinit_acquired
#define Dee_atomic_lock_available       atomic_lock_available
#define Dee_atomic_lock_acquired        atomic_lock_acquired
#define Dee_atomic_lock_tryacquire      atomic_lock_tryacquire
#define Dee_atomic_lock_acquire         atomic_lock_acquire
#define Dee_atomic_lock_waitfor         atomic_lock_waitfor
#define Dee_atomic_lock_release         atomic_lock_release
#define _Dee_atomic_lock_release_NDEBUG _atomic_lock_release_NDEBUG

/* Simply implement atomic R/W-locks using the hybrid-API */
typedef struct atomic_rwlock Dee_atomic_rwlock_t;
#define DEE_ATOMIC_RWLOCK_MAX_READERS        ATOMIC_RWLOCK_MAX_READERS
#define DEE_ATOMIC_RWLOCK_INIT               ATOMIC_RWLOCK_INIT
#define DEE_ATOMIC_RWLOCK_INIT_READ          ATOMIC_RWLOCK_INIT_READ
#define DEE_ATOMIC_RWLOCK_INIT_WRITE         ATOMIC_RWLOCK_INIT_WRITE
#define Dee_atomic_rwlock_cinit              atomic_rwlock_cinit
#define Dee_atomic_rwlock_cinit_read         atomic_rwlock_cinit_read
#define Dee_atomic_rwlock_cinit_write        atomic_rwlock_cinit_write
#define Dee_atomic_rwlock_init               atomic_rwlock_init
#define Dee_atomic_rwlock_init_read          atomic_rwlock_init_read
#define Dee_atomic_rwlock_init_write         atomic_rwlock_init_write
#define Dee_atomic_rwlock_reading            atomic_rwlock_reading
#define Dee_atomic_rwlock_writing            atomic_rwlock_writing
#define Dee_atomic_rwlock_tryread            atomic_rwlock_tryread
#define Dee_atomic_rwlock_trywrite           atomic_rwlock_trywrite
#define Dee_atomic_rwlock_canread            atomic_rwlock_canread
#define Dee_atomic_rwlock_canwrite           atomic_rwlock_canwrite
#define Dee_atomic_rwlock_canendread         atomic_rwlock_canendread
#define Dee_atomic_rwlock_canendwrite        atomic_rwlock_canendwrite
#define Dee_atomic_rwlock_canend             atomic_rwlock_canend
#define Dee_atomic_rwlock_waitread           atomic_rwlock_waitread
#define Dee_atomic_rwlock_waitwrite          atomic_rwlock_waitwrite
#define Dee_atomic_rwlock_read               atomic_rwlock_read
#define Dee_atomic_rwlock_write              atomic_rwlock_write
#define Dee_atomic_rwlock_tryupgrade         atomic_rwlock_tryupgrade
#define Dee_atomic_rwlock_upgrade            atomic_rwlock_upgrade
#define Dee_atomic_rwlock_downgrade          atomic_rwlock_downgrade
#define _Dee_atomic_rwlock_downgrade_NDEBUG  _atomic_rwlock_downgrade_NDEBUG
#define Dee_atomic_rwlock_endwrite           atomic_rwlock_endwrite
#define Dee_atomic_rwlock_endread            atomic_rwlock_endread
#define Dee_atomic_rwlock_end                atomic_rwlock_end
#define Dee_atomic_rwlock_endread_ex         atomic_rwlock_endread_ex
#define Dee_atomic_rwlock_end_ex             atomic_rwlock_end_ex
#define _Dee_atomic_rwlock_endwrite_NDEBUG   _atomic_rwlock_endwrite_NDEBUG
#define _Dee_atomic_rwlock_endread_NDEBUG    _atomic_rwlock_endread_NDEBUG
#define _Dee_atomic_rwlock_end_NDEBUG        _atomic_rwlock_end_NDEBUG
#define _Dee_atomic_rwlock_endread_ex_NDEBUG _atomic_rwlock_endread_ex_NDEBUG
#define _Dee_atomic_rwlock_end_ex_NDEBUG     _atomic_rwlock_end_ex_NDEBUG

/************************************************************************/
/* Shared lock (scheduler-level blocking lock)                          */
/************************************************************************/
typedef struct {
	Dee_atomic_lock_t s_lock;    /* Lock word (== 0: available, != 0: held) */
	unsigned int      s_waiting; /* # of waiting threads */
} Dee_shared_lock_t;
#define DEE_SHARED_LOCK_INIT                 { DEE_ATOMIC_LOCK_INIT, 0 }
#define DEE_SHARED_LOCK_INIT_ACQUIRED        { DEE_ATOMIC_LOCK_INIT_ACQUIRED, 0 }
#define Dee_shared_lock_cinit(self)          (void)(Dee_atomic_lock_cinit(&(self)->s_lock), Dee_ASSERT((self)->s_waiting == 0))
#define Dee_shared_lock_init(self)           (void)(Dee_atomic_lock_init(&(self)->s_lock), (self)->s_waiting = 0)
#define Dee_shared_lock_cinit_acquired(self) (void)(Dee_atomic_lock_cinit_acquired(&(self)->s_lock), Dee_ASSERT((self)->s_waiting == 0))
#define Dee_shared_lock_init_acquired(self)  (void)(Dee_atomic_lock_init_acquired(&(self)->s_lock), (self)->s_waiting = 0)
#define Dee_shared_lock_available(self)      Dee_atomic_lock_available(&(self)->s_lock)
#define Dee_shared_lock_acquired(self)       Dee_atomic_lock_acquired(&(self)->s_lock)
#define Dee_shared_lock_tryacquire(self)     Dee_atomic_lock_tryacquire(&(self)->s_lock)
#define _Dee_shared_lock_wake(self)                             \
	(__hybrid_atomic_load(&(self)->s_waiting, __ATOMIC_ACQUIRE) \
	 ? DeeFutex_WakeOne(&(self)->s_lock)                        \
	 : (void)0)

/* Release a shared lock. */
#define Dee_shared_lock_release(self) \
	(Dee_atomic_lock_release(&(self)->s_lock), _Dee_shared_lock_wake(self))
#define _Dee_shared_lock_release_NDEBUG(self) \
	(_Dee_atomic_lock_release_NDEBUG(&(self)->s_lock), _Dee_shared_lock_wake(self))

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
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_acquire_timed)(Dee_shared_lock_t *__restrict self, uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_waitfor_timed)(Dee_shared_lock_t *__restrict self, uint64_t timeout_nanoseconds);

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
	Dee_atomic_rwlock_t srw_lock;    /* Underlying R/W-lock */
	unsigned int        srw_waiting; /* non-zero if threads may be waiting on `srw_lock' */
} Dee_shared_rwlock_t;

#define _Dee_shared_rwlock_wake(self)                                     \
	(__hybrid_atomic_load(&(self)->srw_waiting, __ATOMIC_ACQUIRE)         \
	 ? (__hybrid_atomic_store(&(self)->srw_waiting, 0, __ATOMIC_RELEASE), \
	    DeeFutex_WakeAll(&(self)->srw_lock.arw_lock))                     \
	 : (void)0)

#define DEE_SHARED_RWLOCK_MAX_READERS        DEE_ATOMIC_RWLOCK_MAX_READERS
#define DEE_SHARED_RWLOCK_INIT               { DEE_ATOMIC_RWLOCK_INIT, 0 }
#define DEE_SHARED_RWLOCK_INIT_READ(n)       { DEE_ATOMIC_RWLOCK_INIT_READ(n), 0 }
#define DEE_SHARED_RWLOCK_INIT_WRITE         { DEE_ATOMIC_RWLOCK_INIT_WRITE, 0 }
#define Dee_shared_rwlock_init(self)         (void)(Dee_atomic_rwlock_init(&(self)->srw_lock), (self)->srw_waiting = 0)
#define Dee_shared_rwlock_init_read(self, n) (void)(Dee_atomic_rwlock_init_read(&(self)->srw_lock, n), (self)->srw_waiting = 0)
#define Dee_shared_rwlock_init_write(self)   (void)(Dee_atomic_rwlock_init_write(&(self)->srw_lock), (self)->srw_waiting = 0)
#define Dee_shared_rwlock_cinit(self)        (void)(Dee_atomic_rwlock_cinit(&(self)->srw_lock), Dee_ASSERT((self)->srw_waiting == 0))
#define Dee_shared_rwlock_cinit_read(self)   (void)(Dee_atomic_rwlock_cinit_read(&(self)->srw_lock, n), Dee_ASSERT((self)->srw_waiting == 0))
#define Dee_shared_rwlock_cinit_write(self)  (void)(Dee_atomic_rwlock_cinit_write(&(self)->srw_lock), Dee_ASSERT((self)->srw_waiting == 0))
#define Dee_shared_rwlock_reading(self)      Dee_atomic_rwlock_reading(&(self)->srw_lock)
#define Dee_shared_rwlock_writing(self)      Dee_atomic_rwlock_writing(&(self)->srw_lock)
#define Dee_shared_rwlock_canread(self)      Dee_atomic_rwlock_canread(&(self)->srw_lock)
#define Dee_shared_rwlock_canwrite(self)     Dee_atomic_rwlock_canwrite(&(self)->srw_lock)
#define Dee_shared_rwlock_canendread(self)   Dee_atomic_rwlock_canendread(&(self)->srw_lock)
#define Dee_shared_rwlock_canendwrite(self)  Dee_atomic_rwlock_canendwrite(&(self)->srw_lock)
#define Dee_shared_rwlock_canend(self)       Dee_atomic_rwlock_canend(&(self)->srw_lock)
#define Dee_shared_rwlock_tryupgrade(self)   Dee_atomic_rwlock_tryupgrade(&(self)->srw_lock)
#define Dee_shared_rwlock_trywrite(self)     Dee_atomic_rwlock_trywrite(&(self)->srw_lock)
#define Dee_shared_rwlock_tryread(self)      Dee_atomic_rwlock_tryread(&(self)->srw_lock)
#define _Dee_shared_rwlock_downgrade_NDEBUG(self)                  \
	(void)(_Dee_atomic_rwlock_downgrade_NDEBUG(&(self)->srw_lock), \
	       _Dee_shared_rwlock_wake(self)) /* Allow for more readers. */
#define Dee_shared_rwlock_downgrade(self)                  \
	(void)(Dee_atomic_rwlock_downgrade(&(self)->srw_lock), \
	       _Dee_shared_rwlock_wake(self)) /* Allow for more readers. */

/* Upgrade from a read- to a write-lock.
 * @return: 1 : Success.
 * @return: 0 : Success, but the read-lock had to be dropped temporarily (i.e. the upgrade wasn't atomic).
 * @return: -1: An exception was thrown (in this case, the old read-lock was lost). */
#define Dee_shared_rwlock_upgrade(self) \
	(Dee_shared_rwlock_tryupgrade(self) ? 1 : (Dee_shared_rwlock_endread(self), Dee_shared_rwlock_write(self)))
#define _Dee_shared_rwlock_upgrade_NDEBUG(self) \
	(Dee_shared_rwlock_tryupgrade(self) ? 1 : (_Dee_shared_rwlock_endread_NDEBUG(self), Dee_shared_rwlock_write(self)))

/* Release a lock of the indicated type. */
#define Dee_shared_rwlock_endread(self)          (void)Dee_shared_rwlock_endread_ex(self)
#define Dee_shared_rwlock_endwrite(self)         (void)(Dee_atomic_rwlock_endwrite(&(self)->srw_lock), _Dee_shared_rwlock_wake(self))
#define Dee_shared_rwlock_end(self)              (void)Dee_shared_rwlock_end_ex(self)
#define _Dee_shared_rwlock_endread_NDEBUG(self)  (void)_Dee_shared_rwlock_endread_ex_NDEBUG(self)
#define _Dee_shared_rwlock_endwrite_NDEBUG(self) (void)(_Dee_atomic_rwlock_endwrite_NDEBUG(&(self)->srw_lock), _Dee_shared_rwlock_wake(self))
#define _Dee_shared_rwlock_end_NDEBUG(self)      (void)_Dee_shared_rwlock_end_ex_NDEBUG(self)

/* Release a lock of the indicated type.
 * @return: true:  The lock may have become free.
 * @return: false: The lock is still held by someone. */
#define Dee_shared_rwlock_endread_ex(self)         (Dee_atomic_rwlock_endread_ex(&(self)->srw_lock) && (_Dee_shared_rwlock_wake(self), 1))
#define Dee_shared_rwlock_end_ex(self)             (Dee_atomic_rwlock_end_ex(&(self)->srw_lock) && (_Dee_shared_rwlock_wake(self), 1))
#define _Dee_shared_rwlock_endread_ex_NDEBUG(self) (_Dee_atomic_rwlock_endread_ex_NDEBUG(&(self)->srw_lock) && (_Dee_shared_rwlock_wake(self), 1))
#define _Dee_shared_rwlock_end_ex_NDEBUG(self)     (_Dee_atomic_rwlock_end_ex_NDEBUG(&(self)->srw_lock) && (_Dee_shared_rwlock_wake(self), 1))


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
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_read_timed)(Dee_shared_rwlock_t *__restrict self, uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_write_timed)(Dee_shared_rwlock_t *__restrict self, uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitread_timed)(Dee_shared_rwlock_t *__restrict self, uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitwrite_timed)(Dee_shared_rwlock_t *__restrict self, uint64_t timeout_nanoseconds);

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




/************************************************************************/
/* Shared semaphore (scheduler-level blocking)                          */
/************************************************************************/
typedef struct {
	uintptr_t se_tickets; /* # of tickets currently available (atomic + futex word) */
	uintptr_t se_waiting; /* # of threads waiting for tickets to become available. */
} Dee_semaphore_t;

#define DEE_SEMAPHORE_INIT(n_tickets)        { n_tickets, 0 }
#define Dee_semaphore_init(self, n_tickets)  (void)((self)->se_tickets = n_tickets, (self)->se_waiting = 0)
#define Dee_semaphore_cinit(self, n_tickets) (void)((self)->se_tickets = n_tickets, Dee_ASSERT((self)->se_waiting == 0))
#define Dee_semaphore_haswaiting(self)       (__hybrid_atomic_load(&(self)->se_waiting, __ATOMIC_ACQUIRE) != 0)
#define Dee_semaphore_hastickets(self)       (__hybrid_atomic_load(&(self)->se_tickets, __ATOMIC_ACQUIRE) != 0)
#define Dee_semaphore_gettickets(self)       __hybrid_atomic_load(&(self)->se_tickets, __ATOMIC_ACQUIRE)
#define Dee_semaphore_release(self)                                         \
	(void)(__hybrid_atomic_fetchinc(&(self)->se_tickets, __ATOMIC_RELEASE), \
	       Dee_semaphore_haswaiting(self) && (DeeFutex_WakeOne(&(self)->se_tickets), 0))
#define Dee_semaphore_tryacquire Dee_semaphore_tryacquire
LOCAL WUNUSED ATTR_INOUT(1) bool
(DCALL Dee_semaphore_tryacquire)(Dee_semaphore_t *__restrict self) {
	uintptr_t temp;
	do {
		temp = __hybrid_atomic_load(&self->se_tickets, __ATOMIC_ACQUIRE);
		if (temp == 0)
			return false;
	} while (!__hybrid_atomic_cmpxch_weak(&self->se_tickets, temp, temp - 1,
	                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED));
	return true;
}

/* Blocking acquire a semaphore ticket, or wait for one to become available.
 * @return: 1 : Timeout expired. (`*_timed' only)
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_waitfor)(Dee_semaphore_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_waitfor_timed)(Dee_semaphore_t *__restrict self,
                                    uint64_t timeout_nanoseconds);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_acquire)(Dee_semaphore_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_semaphore_acquire_timed)(Dee_semaphore_t *__restrict self,
                                    uint64_t timeout_nanoseconds);




/************************************************************************/
/* Shared semaphore (scheduler-level blocking)                          */
/************************************************************************/
typedef struct {
	unsigned int ev_state; /* 0: event has been triggered
	                        * 1: event has NOT been triggered
	                        * 2: event has NOT been triggered, and there are waiting threads */
} Dee_event_t;

#define DEE_EVENT_INIT_SET        { 0 }
#define DEE_EVENT_INIT            { 1 }
#define Dee_event_init_set(self)  (void)((self)->ev_state = 0)
#define Dee_event_init(self)      (void)((self)->ev_state = 1)
#define Dee_event_cinit_set(self) (void)(Dee_ASSERT((self)->ev_state == 0))
#define Dee_event_cinit(self)     (void)(Dee_ASSERT((self)->ev_state == 0), (self)->ev_state = 1)
#define Dee_event_get(self)       (__hybrid_atomic_load(&(self)->ev_state, __ATOMIC_ACQUIRE) == 0)
#define Dee_event_set(self)                                          \
	(__hybrid_atomic_xch(&(self)->ev_state, 0, __ATOMIC_SEQ_CST) > 1 \
	 ? DeeFutex_WakeAll(&(self)->ev_state)                           \
	 : (void)0)
#define Dee_event_clear(self) \
	(void)__hybrid_atomic_cmpxch(&(self)->ev_state, 0, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

/* Blocking wait for an event to become set.
 * @return: 1 : Timeout expired. (`Dee_event_waitfor_timed' only)
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_event_waitfor)(Dee_event_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_event_waitfor_timed)(Dee_event_t *__restrict self,
                                uint64_t timeout_nanoseconds);

DECL_END

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

#endif /* !GUARD_DEEMON_UTIL_LOCK_H */
