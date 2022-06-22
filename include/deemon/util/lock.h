/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_LOCK_H
#define GUARD_DEEMON_UTIL_LOCK_H 1

#include "../api.h"

/**/
#include <hybrid/sched/atomic-lock.h>
#include <hybrid/sched/atomic-rwlock.h>

/*
 * Dee_atomic_lock_t: Atomic lock (sched_yield() until available; no interrupt checks)
 * Dee_shared_lock_t: Shared lock (blocking wait; w/ interrupt checks)
 * Dee_atomic_rwlock_t: Like Dee_atomic_lock_t, but allows for read- and write-locks
 * Dee_shared_rwlock_t: Like Dee_shared_lock_t, but allows for read- and write-locks
 *
 */

DECL_BEGIN

#ifdef CONFIG_NO_THREADS
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
#else /* CONFIG_NO_THREADS */
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

/* TODO: OS-specific optimized implementation for shared_lock (e.g. KOS's <kos/sched/shared-lock.h>)
 *       same for shared_rwlock (e.g. KOS's <kos/sched/shared-rwlock.h>) */

#endif /* !CONFIG_NO_THREADS */

/************************************************************************/
/* Shared lock (scheduler-level blocking lock)                          */
/************************************************************************/
#ifndef DEE_SHARED_LOCK_INIT
#define DEE_CONFIG_SHARED_LOCK_USES_ATOMIC_LOCK
typedef Dee_atomic_lock_t Dee_shared_lock_t; /* Fallback: Use the atomic-lock implementation */
#define DEE_SHARED_LOCK_INIT       DEE_ATOMIC_LOCK_INIT
#define Dee_shared_lock_cinit      Dee_atomic_lock_cinit
#define Dee_shared_lock_init       Dee_atomic_lock_init
#define Dee_shared_lock_available  Dee_atomic_lock_available
#define Dee_shared_lock_acquired   Dee_atomic_lock_acquired
#define Dee_shared_lock_tryacquire Dee_atomic_lock_tryacquire
#define Dee_shared_lock_release    Dee_atomic_lock_release
#endif /* !DEE_SHARED_LOCK_INIT */

/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_acquire)(Dee_shared_lock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_lock_waitfor)(Dee_shared_lock_t *__restrict self);

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

#ifdef CONFIG_NO_THREADS
#undef Dee_shared_lock_acquire
#undef Dee_shared_lock_waitfor
#define Dee_shared_lock_acquire(self) 0
#define Dee_shared_lock_waitfor(self) 0
#endif /* CONFIG_NO_THREADS */




/************************************************************************/
/* Shared r/w-lock (scheduler-level blocking lock)                      */
/************************************************************************/
#ifndef DEE_SHARED_RWLOCK_INIT
#define DEE_CONFIG_SHARED_RWLOCK_USES_ATOMIC_RWLOCK
typedef Dee_atomic_rwlock_t Dee_shared_rwlock_t; /* Fallback: Use the atomic-rwlock implementation */
#define DEE_SHARED_RWLOCK_INIT       DEE_ATOMIC_RWLOCK_INIT
#define Dee_shared_rwlock_cinit      Dee_atomic_rwlock_cinit
#define Dee_shared_rwlock_init       Dee_atomic_rwlock_init
#define Dee_shared_rwlock_reading    Dee_atomic_rwlock_reading
#define Dee_shared_rwlock_writing    Dee_atomic_rwlock_writing
#define Dee_shared_rwlock_tryread    Dee_atomic_rwlock_tryread
#define Dee_shared_rwlock_trywrite   Dee_atomic_rwlock_trywrite
#define Dee_shared_rwlock_canread    Dee_atomic_rwlock_canread
#define Dee_shared_rwlock_canwrite   Dee_atomic_rwlock_canwrite
#define Dee_shared_rwlock_tryupgrade Dee_atomic_rwlock_tryupgrade
#define Dee_shared_rwlock_upgrade    Dee_atomic_rwlock_upgrade
#define Dee_shared_rwlock_downgrade  Dee_atomic_rwlock_downgrade
#define Dee_shared_rwlock_endwrite   Dee_atomic_rwlock_endwrite
#define Dee_shared_rwlock_endread    Dee_atomic_rwlock_endread
#define Dee_shared_rwlock_end        Dee_atomic_rwlock_end
#endif /* !DEE_SHARED_RWLOCK_INIT */

/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_read)(Dee_shared_rwlock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_write)(Dee_shared_rwlock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitread)(Dee_shared_rwlock_t *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_shared_rwlock_waitwrite)(Dee_shared_rwlock_t *__restrict self);

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

#ifdef CONFIG_NO_THREADS
#undef Dee_shared_rwlock_read
#undef Dee_shared_rwlock_write
#undef Dee_shared_rwlock_waitread
#undef Dee_shared_rwlock_waitwrite
#define Dee_shared_rwlock_read(self)      0
#define Dee_shared_rwlock_write(self)     0
#define Dee_shared_rwlock_waitread(self)  0
#define Dee_shared_rwlock_waitwrite(self) 0
#endif /* CONFIG_NO_THREADS */




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
#if !defined(SHARED_LOCK_INIT) || defined(CONFIG_NO_THREADS)
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
#endif /* !SHARED_LOCK_INIT || CONFIG_NO_THREADS */

typedef Dee_shared_rwlock_t shared_rwlock_t;
#if !defined(SHARED_RWLOCK_INIT) || defined(CONFIG_NO_THREADS)
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
#endif /* !SHARED_RWLOCK_INIT || CONFIG_NO_THREADS */
#endif /* DEE_SOURCE */

DECL_END

#endif /* !GUARD_DEEMON_UTIL_LOCK_H */
