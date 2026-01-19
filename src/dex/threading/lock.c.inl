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
#ifdef __INTELLISENSE__
#include "lock.c"
#define DEE_SOURCE
//#define DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
//#define DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type
//#define DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type
#define DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type
#endif /* __INTELLISENSE__ */

#include "libthreading.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock-utils.h>
#include <deemon/util/lock.h>
#include <deemon/util/rlock-utils.h>
#include <deemon/util/rlock.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>

#include <stdbool.h> /* bool, false, true */
#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uint64_t, uintptr_t */

#if (defined(DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type) +   \
     defined(DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type) +   \
     defined(DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type) + \
     defined(DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type)) != 1
#error "Must #define exactly one of these macros."
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
/* ... */
#elif defined(DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type)
#define LOCAL_IS_SHARED
#elif defined(DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type)
#define LOCAL_IS_RECURSIVE
#else /* #elif defined(DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type) */
#define LOCAL_IS_SHARED
#define LOCAL_IS_RECURSIVE
#endif /* ... */

#ifdef LOCAL_IS_SHARED
#define LOCAL_S_Atomic_or_Shared "Shared"
#else /* LOCAL_IS_SHARED */
#define LOCAL_S_Atomic_or_Shared "Atomic"
#endif /* !LOCAL_IS_SHARED */

#ifdef LOCAL_IS_RECURSIVE
#define LOCAL_S_MaybeRecursiveR "R"
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_S_MaybeRecursiveR ""
#endif /* !LOCAL_IS_RECURSIVE */

#define LOCAL_S_Lock            LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "Lock"
#define LOCAL_S_RWLock          LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "RWLock"
#define LOCAL_S_RWLockReadLock  "_" LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "RWLockReadLock"
#define LOCAL_S_RWLockWriteLock "_" LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "RWLockWriteLock"

#if !defined(LOCAL_IS_SHARED) && defined(CONFIG_NO_THREADS)
#define LOCAL_IS_ATOMIC_AS_SHARED
#endif /* !LOCAL_IS_SHARED && CONFIG_NO_THREADS */

/* Select C-level API for Lock-type */
#ifdef LOCAL_IS_SHARED
#ifdef LOCAL_IS_RECURSIVE
#define LOCAL_lockapi_func(x)      drshared_lock_##x
#define LOCAL_DeeLockObject        DeeRSharedLockObject
#define LOCAL_DeeLock_Type         DeeRSharedLock_Type
#define LOCAL_lock_t               Dee_rshared_lock_t
#define LOCAL_lock_init            Dee_rshared_lock_init
#define LOCAL_lock_cinit           Dee_rshared_lock_cinit
#define LOCAL_lock_available       Dee_rshared_lock_available
#define LOCAL_lock_acquired        Dee_rshared_lock_acquired
#define LOCAL_lock_tryacquire      Dee_rshared_lock_tryacquire
#define LOCAL_lock_acquire         Dee_rshared_lock_acquire
#define LOCAL_lock_waitfor         Dee_rshared_lock_waitfor
#define LOCAL_lock_acquire_timed   Dee_rshared_lock_acquire_timed
#define LOCAL_lock_waitfor_timed   Dee_rshared_lock_waitfor_timed
#define _LOCAL_lock_release_NDEBUG _Dee_rshared_lock_release_NDEBUG
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_lockapi_func(x)      dshared_lock_##x
#define LOCAL_DeeLockObject        DeeSharedLockObject
#define LOCAL_DeeLock_Type         DeeSharedLock_Type
#define LOCAL_lock_t               Dee_shared_lock_t
#define LOCAL_lock_init            Dee_shared_lock_init
#define LOCAL_lock_init_acquired   Dee_shared_lock_init_acquired
#define LOCAL_lock_cinit           Dee_shared_lock_cinit
#define LOCAL_lock_cinit_acquired  Dee_shared_lock_cinit_acquired
#define LOCAL_lock_available       Dee_shared_lock_available
#define LOCAL_lock_acquired        Dee_shared_lock_acquired
#define LOCAL_lock_tryacquire      Dee_shared_lock_tryacquire
#define LOCAL_lock_acquire         Dee_shared_lock_acquire
#define LOCAL_lock_waitfor         Dee_shared_lock_waitfor
#define LOCAL_lock_acquire_timed   Dee_shared_lock_acquire_timed
#define LOCAL_lock_waitfor_timed   Dee_shared_lock_waitfor_timed
#define _LOCAL_lock_release_NDEBUG _Dee_shared_lock_release_NDEBUG
#endif /* !LOCAL_IS_RECURSIVE */
#else  /* LOCAL_IS_SHARED */
#ifdef LOCAL_IS_RECURSIVE
#define LOCAL_lockapi_func(x)      dratomic_lock_##x
#define LOCAL_DeeLockObject        DeeRAtomicLockObject
#define LOCAL_DeeLock_Type         DeeRAtomicLock_Type
#define LOCAL_lock_t               Dee_ratomic_lock_t
#define LOCAL_lock_init            Dee_ratomic_lock_init
#define LOCAL_lock_cinit           Dee_ratomic_lock_cinit
#define LOCAL_lock_available       Dee_ratomic_lock_available
#define LOCAL_lock_acquired        Dee_ratomic_lock_acquired
#define LOCAL_lock_tryacquire      Dee_ratomic_lock_tryacquire
#define _LOCAL_lock_release_NDEBUG _Dee_ratomic_lock_release_NDEBUG
#define LOCAL_lock_acquire_p       Dee_ratomic_lock_acquire_p
#define LOCAL_lock_waitfor_p       Dee_ratomic_lock_waitfor_p
#define LOCAL_lock_acquire_timed_p Dee_ratomic_lock_acquire_timed_p
#define LOCAL_lock_waitfor_timed_p Dee_ratomic_lock_waitfor_timed_p
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_lockapi_func(x)      datomic_lock_##x
#define LOCAL_DeeLockObject        DeeAtomicLockObject
#define LOCAL_DeeLock_Type         DeeAtomicLock_Type
#define LOCAL_lock_t               Dee_atomic_lock_t
#define LOCAL_lock_init            Dee_atomic_lock_init
#define LOCAL_lock_init_acquired   Dee_atomic_lock_init_acquired
#define LOCAL_lock_cinit           Dee_atomic_lock_cinit
#define LOCAL_lock_cinit_acquired  Dee_atomic_lock_cinit_acquired
#define LOCAL_lock_available       Dee_atomic_lock_available
#define LOCAL_lock_acquired        Dee_atomic_lock_acquired
#define LOCAL_lock_tryacquire      Dee_atomic_lock_tryacquire
#define _LOCAL_lock_release_NDEBUG _Dee_atomic_lock_release_NDEBUG
#define LOCAL_lock_acquire_p       Dee_atomic_lock_acquire_p
#define LOCAL_lock_waitfor_p       Dee_atomic_lock_waitfor_p
#define LOCAL_lock_acquire_timed_p Dee_atomic_lock_acquire_timed_p
#define LOCAL_lock_waitfor_timed_p Dee_atomic_lock_waitfor_timed_p
#endif /* !LOCAL_IS_RECURSIVE */
#endif /* !LOCAL_IS_SHARED */
#ifndef LOCAL_lock_acquire_p
#define LOCAL_lock_acquire_p(self, err_label) \
	do {                                      \
		if unlikely(LOCAL_lock_acquire(self)) \
			goto err_label;                   \
	}	__WHILE0
#endif /* !LOCAL_lock_acquire_p */
#ifndef LOCAL_lock_waitfor_p
#define LOCAL_lock_waitfor_p(self, err_label) \
	do {                                      \
		if unlikely(LOCAL_lock_waitfor(self)) \
			goto err_label;                   \
	}	__WHILE0
#endif /* !LOCAL_lock_waitfor_p */
#ifndef LOCAL_lock_acquire_timed_p
#define LOCAL_lock_acquire_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                \
		int _status = LOCAL_lock_acquire_timed(self, timeout_nanoseconds);              \
		if unlikely(_status != 0) {                                                     \
			if unlikely(_status < 0)                                                    \
				goto err_label;                                                         \
			goto timeout_label;                                                         \
		}                                                                               \
	}	__WHILE0
#endif /* !LOCAL_lock_acquire_p */
#ifndef LOCAL_lock_waitfor_timed_p
#define LOCAL_lock_waitfor_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                \
		int _status = LOCAL_lock_waitfor_timed(self, timeout_nanoseconds);              \
		if unlikely(_status != 0) {                                                     \
			if unlikely(_status < 0)                                                    \
				goto err_label;                                                         \
			goto timeout_label;                                                         \
		}                                                                               \
	}	__WHILE0
#endif /* !LOCAL_lock_waitfor_p */


/* Select C-level API for RWLock-type */
#ifdef LOCAL_IS_SHARED
#ifdef LOCAL_IS_RECURSIVE
#define LOCAL_rwlockapi_func(x)         drshared_rwlock_##x
#define LOCAL_DeeRWLockObject           DeeRSharedRWLockObject
#define LOCAL_DeeRWLock_Type            DeeRSharedRWLock_Type
#define LOCAL_DeeRWLockReadLock_Type    DeeRSharedRWLockReadLock_Type
#define LOCAL_DeeRWLockWriteLock_Type   DeeRSharedRWLockWriteLock_Type
#define LOCAL_rwlock_t                  Dee_rshared_rwlock_t
#define LOCAL_rwlock_cinit              Dee_rshared_rwlock_cinit
#define LOCAL_rwlock_init               Dee_rshared_rwlock_init
#define LOCAL_rwlock_reading            Dee_rshared_rwlock_reading
#define LOCAL_rwlock_writing            Dee_rshared_rwlock_writing
#define LOCAL_rwlock_tryread            Dee_rshared_rwlock_tryread
#define LOCAL_rwlock_trywrite           Dee_rshared_rwlock_trywrite
#define LOCAL_rwlock_canread            Dee_rshared_rwlock_canread
#define LOCAL_rwlock_canwrite           Dee_rshared_rwlock_canwrite
#define LOCAL_rwlock_canendread         Dee_rshared_rwlock_canendread
#define LOCAL_rwlock_canendwrite        Dee_rshared_rwlock_canendwrite
#define LOCAL_rwlock_canend             Dee_rshared_rwlock_canend
#define LOCAL_rwlock_tryupgrade         Dee_rshared_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG  _Dee_rshared_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG   _Dee_rshared_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG    _Dee_rshared_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG        _Dee_rshared_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG _Dee_rshared_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG     _Dee_rshared_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread           Dee_rshared_rwlock_waitread
#define LOCAL_rwlock_waitwrite          Dee_rshared_rwlock_waitwrite
#define LOCAL_rwlock_waitread_timed     Dee_rshared_rwlock_waitread_timed
#define LOCAL_rwlock_waitwrite_timed    Dee_rshared_rwlock_waitwrite_timed
#define LOCAL_rwlock_read               Dee_rshared_rwlock_read
#define LOCAL_rwlock_write              Dee_rshared_rwlock_write
#define LOCAL_rwlock_read_timed         Dee_rshared_rwlock_read_timed
#define LOCAL_rwlock_write_timed        Dee_rshared_rwlock_write_timed
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_rwlockapi_func(x)         dshared_rwlock_##x
#define LOCAL_DeeRWLockObject           DeeSharedRWLockObject
#define LOCAL_DeeRWLock_Type            DeeSharedRWLock_Type
#define LOCAL_DeeRWLockReadLock_Type    DeeSharedRWLockReadLock_Type
#define LOCAL_DeeRWLockWriteLock_Type   DeeSharedRWLockWriteLock_Type
#define LOCAL_rwlock_t                  Dee_shared_rwlock_t
#define LOCAL_RWLOCK_MAX_READERS        Dee_SHARED_RWLOCK_MAX_READERS
#define LOCAL_rwlock_cinit              Dee_shared_rwlock_cinit
#define LOCAL_rwlock_cinit_read         Dee_shared_rwlock_cinit_read
#define LOCAL_rwlock_cinit_write        Dee_shared_rwlock_cinit_write
#define LOCAL_rwlock_init               Dee_shared_rwlock_init
#define LOCAL_rwlock_init_read          Dee_shared_rwlock_init_read
#define LOCAL_rwlock_init_write         Dee_shared_rwlock_init_write
#define LOCAL_rwlock_reading            Dee_shared_rwlock_reading
#define LOCAL_rwlock_writing            Dee_shared_rwlock_writing
#define LOCAL_rwlock_tryread            Dee_shared_rwlock_tryread
#define LOCAL_rwlock_trywrite           Dee_shared_rwlock_trywrite
#define LOCAL_rwlock_canread            Dee_shared_rwlock_canread
#define LOCAL_rwlock_canwrite           Dee_shared_rwlock_canwrite
#define LOCAL_rwlock_canendread         Dee_shared_rwlock_canendread
#define LOCAL_rwlock_canendwrite        Dee_shared_rwlock_canendwrite
#define LOCAL_rwlock_canend             Dee_shared_rwlock_canend
#define LOCAL_rwlock_tryupgrade         Dee_shared_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG  _Dee_shared_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG   _Dee_shared_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG    _Dee_shared_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG        _Dee_shared_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG _Dee_shared_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG     _Dee_shared_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread           Dee_shared_rwlock_waitread
#define LOCAL_rwlock_waitwrite          Dee_shared_rwlock_waitwrite
#define LOCAL_rwlock_waitread_timed     Dee_shared_rwlock_waitread_timed
#define LOCAL_rwlock_waitwrite_timed    Dee_shared_rwlock_waitwrite_timed
#define LOCAL_rwlock_read               Dee_shared_rwlock_read
#define LOCAL_rwlock_write              Dee_shared_rwlock_write
#define LOCAL_rwlock_read_timed         Dee_shared_rwlock_read_timed
#define LOCAL_rwlock_write_timed        Dee_shared_rwlock_write_timed
#endif /* !LOCAL_IS_RECURSIVE */
#else  /* LOCAL_IS_SHARED */
#ifdef LOCAL_IS_RECURSIVE
#define LOCAL_rwlockapi_func(x)         dratomic_rwlock_##x
#define LOCAL_DeeRWLockObject           DeeRAtomicRWLockObject
#define LOCAL_DeeRWLock_Type            DeeRAtomicRWLock_Type
#define LOCAL_DeeRWLockReadLock_Type    DeeRAtomicRWLockReadLock_Type
#define LOCAL_DeeRWLockWriteLock_Type   DeeRAtomicRWLockWriteLock_Type
#define LOCAL_rwlock_t                  Dee_ratomic_rwlock_t
#define LOCAL_rwlock_cinit              Dee_ratomic_rwlock_cinit
#define LOCAL_rwlock_init               Dee_ratomic_rwlock_init
#define LOCAL_rwlock_reading            Dee_ratomic_rwlock_reading
#define LOCAL_rwlock_writing            Dee_ratomic_rwlock_writing
#define LOCAL_rwlock_tryread            Dee_ratomic_rwlock_tryread
#define LOCAL_rwlock_trywrite           Dee_ratomic_rwlock_trywrite
#define LOCAL_rwlock_canread            Dee_ratomic_rwlock_canread
#define LOCAL_rwlock_canwrite           Dee_ratomic_rwlock_canwrite
#define LOCAL_rwlock_canendread         Dee_ratomic_rwlock_canendread
#define LOCAL_rwlock_canendwrite        Dee_ratomic_rwlock_canendwrite
#define LOCAL_rwlock_canend             Dee_ratomic_rwlock_canend
#define LOCAL_rwlock_tryupgrade         Dee_ratomic_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG  _Dee_ratomic_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG   _Dee_ratomic_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG    _Dee_ratomic_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG        _Dee_ratomic_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG _Dee_ratomic_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG     _Dee_ratomic_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread_p         Dee_ratomic_rwlock_waitread_p
#define LOCAL_rwlock_waitwrite_p        Dee_ratomic_rwlock_waitwrite_p
#define LOCAL_rwlock_waitread_timed_p   Dee_ratomic_rwlock_waitread_timed_p
#define LOCAL_rwlock_waitwrite_timed_p  Dee_ratomic_rwlock_waitwrite_timed_p
#define LOCAL_rwlock_read_p             Dee_ratomic_rwlock_read_p
#define LOCAL_rwlock_write_p            Dee_ratomic_rwlock_write_p
#define LOCAL_rwlock_read_timed_p       Dee_ratomic_rwlock_read_timed_p
#define LOCAL_rwlock_write_timed_p      Dee_ratomic_rwlock_write_timed_p
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_rwlockapi_func(x)         datomic_rwlock_##x
#define LOCAL_DeeRWLockObject           DeeAtomicRWLockObject
#define LOCAL_DeeRWLock_Type            DeeAtomicRWLock_Type
#define LOCAL_DeeRWLockReadLock_Type    DeeAtomicRWLockReadLock_Type
#define LOCAL_DeeRWLockWriteLock_Type   DeeAtomicRWLockWriteLock_Type
#define LOCAL_rwlock_t                  Dee_atomic_rwlock_t
#define LOCAL_RWLOCK_MAX_READERS        Dee_ATOMIC_RWLOCK_MAX_READERS
#define LOCAL_rwlock_cinit              Dee_atomic_rwlock_cinit
#define LOCAL_rwlock_cinit_read         Dee_atomic_rwlock_cinit_read
#define LOCAL_rwlock_cinit_write        Dee_atomic_rwlock_cinit_write
#define LOCAL_rwlock_init               Dee_atomic_rwlock_init
#define LOCAL_rwlock_init_read          Dee_atomic_rwlock_init_read
#define LOCAL_rwlock_init_write         Dee_atomic_rwlock_init_write
#define LOCAL_rwlock_reading            Dee_atomic_rwlock_reading
#define LOCAL_rwlock_writing            Dee_atomic_rwlock_writing
#define LOCAL_rwlock_tryread            Dee_atomic_rwlock_tryread
#define LOCAL_rwlock_trywrite           Dee_atomic_rwlock_trywrite
#define LOCAL_rwlock_canread            Dee_atomic_rwlock_canread
#define LOCAL_rwlock_canwrite           Dee_atomic_rwlock_canwrite
#define LOCAL_rwlock_canendread         Dee_atomic_rwlock_canendread
#define LOCAL_rwlock_canendwrite        Dee_atomic_rwlock_canendwrite
#define LOCAL_rwlock_canend             Dee_atomic_rwlock_canend
#define LOCAL_rwlock_tryupgrade         Dee_atomic_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG  _Dee_atomic_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG   _Dee_atomic_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG    _Dee_atomic_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG        _Dee_atomic_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG _Dee_atomic_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG     _Dee_atomic_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread_p         Dee_atomic_rwlock_waitread_p
#define LOCAL_rwlock_waitwrite_p        Dee_atomic_rwlock_waitwrite_p
#define LOCAL_rwlock_waitread_timed_p   Dee_atomic_rwlock_waitread_timed_p
#define LOCAL_rwlock_waitwrite_timed_p  Dee_atomic_rwlock_waitwrite_timed_p
#define LOCAL_rwlock_read_p             Dee_atomic_rwlock_read_p
#define LOCAL_rwlock_write_p            Dee_atomic_rwlock_write_p
#define LOCAL_rwlock_read_timed_p       Dee_atomic_rwlock_read_timed_p
#define LOCAL_rwlock_write_timed_p      Dee_atomic_rwlock_write_timed_p
#endif /* !LOCAL_IS_RECURSIVE */
#endif /* !LOCAL_IS_SHARED */

#ifndef LOCAL_rwlock_waitread_p
#define LOCAL_rwlock_waitread_p(self, err_label) \
	do {                                         \
		if unlikely(LOCAL_rwlock_waitread(self)) \
			goto err_label;                      \
	}	__WHILE0
#endif /* !LOCAL_rwlock_waitread_p */
#ifndef LOCAL_rwlock_waitwrite_p
#define LOCAL_rwlock_waitwrite_p(self, err_label) \
	do {                                          \
		if unlikely(LOCAL_rwlock_waitwrite(self)) \
			goto err_label;                       \
	}	__WHILE0
#endif /* !LOCAL_rwlock_waitwrite_p */
#ifndef LOCAL_rwlock_waitread_timed_p
#define LOCAL_rwlock_waitread_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                   \
		int _status = LOCAL_rwlock_waitread_timed(self, timeout_nanoseconds);              \
		if unlikely(_status != 0) {                                                        \
			if unlikely(_status < 0)                                                       \
				goto err_label;                                                            \
			goto timeout_label;                                                            \
		}                                                                                  \
	}	__WHILE0
#endif /* !LOCAL_rwlock_waitread_timed_p */
#ifndef LOCAL_rwlock_waitwrite_timed_p
#define LOCAL_rwlock_waitwrite_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                    \
		int _status = LOCAL_rwlock_waitwrite_timed(self, timeout_nanoseconds);              \
		if unlikely(_status != 0) {                                                         \
			if unlikely(_status < 0)                                                        \
				goto err_label;                                                             \
			goto timeout_label;                                                             \
		}                                                                                   \
	}	__WHILE0
#endif /* !LOCAL_rwlock_waitwrite_timed_p */
#ifndef LOCAL_rwlock_read_p
#define LOCAL_rwlock_read_p(self, err_label) \
	do {                                     \
		if unlikely(LOCAL_rwlock_read(self)) \
			goto err_label;                  \
	}	__WHILE0
#endif /* !LOCAL_rwlock_read_p */
#ifndef LOCAL_rwlock_write_p
#define LOCAL_rwlock_write_p(self, err_label) \
	do {                                      \
		if unlikely(LOCAL_rwlock_write(self)) \
			goto err_label;                   \
	}	__WHILE0
#endif /* !LOCAL_rwlock_write_p */
#ifndef LOCAL_rwlock_read_timed_p
#define LOCAL_rwlock_read_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                               \
		int _status = LOCAL_rwlock_read_timed(self, timeout_nanoseconds);              \
		if unlikely(_status != 0) {                                                    \
			if unlikely(_status < 0)                                                   \
				goto err_label;                                                        \
			goto timeout_label;                                                        \
		}                                                                              \
	}	__WHILE0
#endif /* !LOCAL_rwlock_read_timed_p */
#ifndef LOCAL_rwlock_write_timed_p
#define LOCAL_rwlock_write_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                \
		int _status = LOCAL_rwlock_write_timed(self, timeout_nanoseconds);              \
		if unlikely(_status != 0) {                                                     \
			if unlikely(_status < 0)                                                    \
				goto err_label;                                                         \
			goto timeout_label;                                                         \
		}                                                                               \
	}	__WHILE0
#endif /* !LOCAL_rwlock_write_timed_p */


/* Local API function selection (lock) */
#define LOCAL_lockapi_ctor          LOCAL_lockapi_func(ctr)
#define LOCAL_lockapi_init_kw       LOCAL_lockapi_func(init_kw)
#define LOCAL_lockapi_printrepr     LOCAL_lockapi_func(printrepr)
#define LOCAL_lockapi_enter         LOCAL_lockapi_func(enter)
#define LOCAL_lockapi_leave         LOCAL_lockapi_func(leave)
#define LOCAL_lockapi_with          LOCAL_lockapi_func(with)
#define LOCAL_lockapi_tryacquire    LOCAL_lockapi_func(tryacquire)
#define LOCAL_lockapi_acquire       LOCAL_lockapi_func(acquire)
#define LOCAL_lockapi_timedacquire  LOCAL_lockapi_func(timedacquire)
#define LOCAL_lockapi_do_waitfor    LOCAL_lockapi_func(do_waitfor)
#define LOCAL_lockapi_waitfor       LOCAL_lockapi_func(waitfor)
#define LOCAL_lockapi_timedwaitfor  LOCAL_lockapi_func(timedwaitfor)
#define LOCAL_lockapi_release       LOCAL_lockapi_func(release)
#define LOCAL_lockapi_available_get LOCAL_lockapi_func(available_get)
#define LOCAL_lockapi_acquired_get  LOCAL_lockapi_func(acquired_get)
#define LOCAL_lockapi_methods       LOCAL_lockapi_func(methods)
#define LOCAL_lockapi_getsets       LOCAL_lockapi_func(getsets)
#define LOCAL_lockapi_members       LOCAL_lockapi_func(members)

/* Local API function selection (rwlock) */
#define LOCAL_rwlockapi_ctor                    LOCAL_rwlockapi_func(ctor)
#define LOCAL_rwlockapi_init_kw                 LOCAL_rwlockapi_func(init_kw)
#define LOCAL_rwlockapi_printrepr               LOCAL_rwlockapi_func(printrepr)
#define LOCAL_rwlockapi_tryread                 LOCAL_rwlockapi_func(tryread)
#define LOCAL_rwlockapi_trywrite                LOCAL_rwlockapi_func(trywrite)
#define LOCAL_rwlockapi_tryupgrade              LOCAL_rwlockapi_func(tryupgrade)
#define LOCAL_rwlockapi_endread                 LOCAL_rwlockapi_func(endread)
#define LOCAL_rwlockapi_endwrite                LOCAL_rwlockapi_func(endwrite)
#define LOCAL_rwlockapi_downgrade               LOCAL_rwlockapi_func(downgrade)
#define LOCAL_rwlockapi_read                    LOCAL_rwlockapi_func(read)
#define LOCAL_rwlockapi_write                   LOCAL_rwlockapi_func(write)
#define LOCAL_rwlockapi_end                     LOCAL_rwlockapi_func(end)
#define LOCAL_rwlockapi_upgrade                 LOCAL_rwlockapi_func(upgrade)
#define LOCAL_rwlockapi_timedread               LOCAL_rwlockapi_func(timedread)
#define LOCAL_rwlockapi_timedwrite              LOCAL_rwlockapi_func(timedwrite)
#define LOCAL_rwlockapi_waitread                LOCAL_rwlockapi_func(waitread)
#define LOCAL_rwlockapi_waitwrite               LOCAL_rwlockapi_func(waitwrite)
#define LOCAL_rwlockapi_timedwaitread           LOCAL_rwlockapi_func(timedwaitread)
#define LOCAL_rwlockapi_timedwaitwrite          LOCAL_rwlockapi_func(timedwaitwrite)
#define LOCAL_rwlockapi_canread_get             LOCAL_rwlockapi_func(canread_get)
#define LOCAL_rwlockapi_canwrite_get            LOCAL_rwlockapi_func(canwrite_get)
#define LOCAL_rwlockapi_reading_get             LOCAL_rwlockapi_func(reading_get)
#define LOCAL_rwlockapi_writing_get             LOCAL_rwlockapi_func(writing_get)
#define LOCAL_rwlockapi_readlock_get            LOCAL_rwlockapi_func(readlock_get)
#define LOCAL_rwlockapi_writelock_get           LOCAL_rwlockapi_func(writelock_get)
#define LOCAL_rwlockapi_methods                 LOCAL_rwlockapi_func(methods)
#define LOCAL_rwlockapi_getsets                 LOCAL_rwlockapi_func(getsets)
#define LOCAL_rwlockapi_readlock_init           LOCAL_rwlockapi_func(readlock_init)
#define LOCAL_rwlockapi_writelock_init          LOCAL_rwlockapi_func(writelock_init)
#define LOCAL_rwlockapi_readlock_enter          LOCAL_rwlockapi_func(readlock_enter)
#define LOCAL_rwlockapi_writelock_enter         LOCAL_rwlockapi_func(writelock_enter)
#define LOCAL_rwlockapi_readlock_leave          LOCAL_rwlockapi_func(readlock_leave)
#define LOCAL_rwlockapi_writelock_leave         LOCAL_rwlockapi_func(writelock_leave)
#define LOCAL_rwlockapi_readlock_tryacquire     LOCAL_rwlockapi_func(readlock_tryacquire)
#define LOCAL_rwlockapi_writelock_tryacquire    LOCAL_rwlockapi_func(writelock_tryacquire)
#define LOCAL_rwlockapi_readlock_acquire        LOCAL_rwlockapi_func(readlock_acquire)
#define LOCAL_rwlockapi_writelock_acquire       LOCAL_rwlockapi_func(writelock_acquire)
#define LOCAL_rwlockapi_readlock_release        LOCAL_rwlockapi_func(readlock_release)
#define LOCAL_rwlockapi_writelock_release       LOCAL_rwlockapi_func(writelock_release)
#define LOCAL_rwlockapi_readlock_timedacquire   LOCAL_rwlockapi_func(readlock_timedacquire)
#define LOCAL_rwlockapi_writelock_timedacquire  LOCAL_rwlockapi_func(writelock_timedacquire)
#define LOCAL_rwlockapi_readlock_waitfor        LOCAL_rwlockapi_func(readlock_waitfor)
#define LOCAL_rwlockapi_writelock_waitfor       LOCAL_rwlockapi_func(writelock_waitfor)
#define LOCAL_rwlockapi_readlock_timedwaitfor   LOCAL_rwlockapi_func(readlock_timedwaitfor)
#define LOCAL_rwlockapi_writelock_timedwaitfor  LOCAL_rwlockapi_func(writelock_timedwaitfor)
#define LOCAL_rwlockapi_readlock_available_get  LOCAL_rwlockapi_func(readlock_available_get)
#define LOCAL_rwlockapi_writelock_available_get LOCAL_rwlockapi_func(writelock_available_get)
#define LOCAL_rwlockapi_readlock_acquired_get   LOCAL_rwlockapi_func(readlock_acquired_get)
#define LOCAL_rwlockapi_writelock_acquired_get  LOCAL_rwlockapi_func(writelock_acquired_get)
#define LOCAL_rwlockapi_readlock_with           LOCAL_rwlockapi_func(readlock_with)
#define LOCAL_rwlockapi_writelock_with          LOCAL_rwlockapi_func(writelock_with)
#define LOCAL_rwlockapi_readlock_methods        LOCAL_rwlockapi_func(readlock_methods)
#define LOCAL_rwlockapi_writelock_methods       LOCAL_rwlockapi_func(writelock_methods)
#define LOCAL_rwlockapi_readlock_getsets        LOCAL_rwlockapi_func(readlock_getsets)
#define LOCAL_rwlockapi_writelock_getsets       LOCAL_rwlockapi_func(writelock_getsets)

/* Alias atomic type operators/functions for shared operators/functions */
/*[[[deemon
import * from deemon;
local names_lockapi = [];
local names_rwlockapi = [];
for (local line: File.open(__FILE__)) {
	local name_lockapi   = try line.rescanf(r"#define LOCAL_lockapi_[^ ]* +LOCAL_lockapi_func\(([^)]+)\)")[0] catch (...) none;
	local name_rwlockapi = try line.rescanf(r"#define LOCAL_rwlockapi_[^ ]* +LOCAL_rwlockapi_func\(([^)]+)\)")[0] catch (...) none;
	if (name_lockapi !is none)
		names_lockapi.append(name_lockapi);
	if (name_rwlockapi !is none)
		names_rwlockapi.append(name_rwlockapi);
}
for (local name: { "init_kw", "printrepr", "readlock_init", "writelock_init" }) {
	names_lockapi.remove(name);
	names_rwlockapi.remove(name);
}
print("#ifdef LOCAL_IS_ATOMIC_AS_SHARED");
print("#ifndef DEEMON_LIBTHREADING_LOCKS_ATOMIC_AS_SHARED_DEFINED");
print("#define DEEMON_LIBTHREADING_LOCKS_ATOMIC_AS_SHARED_DEFINED");
for (local name: names_lockapi) {
	print("#define datomic_lock_", name, " dshared_lock_", name);
	print("#define dratomic_lock_", name, " drshared_lock_", name);
}
for (local name: names_rwlockapi) {
	print("#define datomic_rwlock_", name, " dshared_rwlock_", name);
	print("#define dratomic_rwlock_", name, " drshared_rwlock_", name);
}
print("#endif /" "* !DEEMON_LIBTHREADING_LOCKS_ATOMIC_AS_SHARED_DEFINED *" "/");
print("#endif /" "* LOCAL_IS_ATOMIC_AS_SHARED *" "/");
]]]*/
#ifdef LOCAL_IS_ATOMIC_AS_SHARED
#ifndef DEEMON_LIBTHREADING_LOCKS_ATOMIC_AS_SHARED_DEFINED
#define DEEMON_LIBTHREADING_LOCKS_ATOMIC_AS_SHARED_DEFINED
#define datomic_lock_ctr dshared_lock_ctr
#define dratomic_lock_ctr drshared_lock_ctr
#define datomic_lock_enter dshared_lock_enter
#define dratomic_lock_enter drshared_lock_enter
#define datomic_lock_leave dshared_lock_leave
#define dratomic_lock_leave drshared_lock_leave
#define datomic_lock_with dshared_lock_with
#define dratomic_lock_with drshared_lock_with
#define datomic_lock_tryacquire dshared_lock_tryacquire
#define dratomic_lock_tryacquire drshared_lock_tryacquire
#define datomic_lock_acquire dshared_lock_acquire
#define dratomic_lock_acquire drshared_lock_acquire
#define datomic_lock_timedacquire dshared_lock_timedacquire
#define dratomic_lock_timedacquire drshared_lock_timedacquire
#define datomic_lock_do_waitfor dshared_lock_do_waitfor
#define dratomic_lock_do_waitfor drshared_lock_do_waitfor
#define datomic_lock_waitfor dshared_lock_waitfor
#define dratomic_lock_waitfor drshared_lock_waitfor
#define datomic_lock_timedwaitfor dshared_lock_timedwaitfor
#define dratomic_lock_timedwaitfor drshared_lock_timedwaitfor
#define datomic_lock_release dshared_lock_release
#define dratomic_lock_release drshared_lock_release
#define datomic_lock_available_get dshared_lock_available_get
#define dratomic_lock_available_get drshared_lock_available_get
#define datomic_lock_acquired_get dshared_lock_acquired_get
#define dratomic_lock_acquired_get drshared_lock_acquired_get
#define datomic_lock_methods dshared_lock_methods
#define dratomic_lock_methods drshared_lock_methods
#define datomic_lock_getsets dshared_lock_getsets
#define dratomic_lock_getsets drshared_lock_getsets
#define datomic_lock_members dshared_lock_members
#define dratomic_lock_members drshared_lock_members
#define datomic_rwlock_ctor dshared_rwlock_ctor
#define dratomic_rwlock_ctor drshared_rwlock_ctor
#define datomic_rwlock_tryread dshared_rwlock_tryread
#define dratomic_rwlock_tryread drshared_rwlock_tryread
#define datomic_rwlock_trywrite dshared_rwlock_trywrite
#define dratomic_rwlock_trywrite drshared_rwlock_trywrite
#define datomic_rwlock_tryupgrade dshared_rwlock_tryupgrade
#define dratomic_rwlock_tryupgrade drshared_rwlock_tryupgrade
#define datomic_rwlock_endread dshared_rwlock_endread
#define dratomic_rwlock_endread drshared_rwlock_endread
#define datomic_rwlock_endwrite dshared_rwlock_endwrite
#define dratomic_rwlock_endwrite drshared_rwlock_endwrite
#define datomic_rwlock_downgrade dshared_rwlock_downgrade
#define dratomic_rwlock_downgrade drshared_rwlock_downgrade
#define datomic_rwlock_read dshared_rwlock_read
#define dratomic_rwlock_read drshared_rwlock_read
#define datomic_rwlock_write dshared_rwlock_write
#define dratomic_rwlock_write drshared_rwlock_write
#define datomic_rwlock_end dshared_rwlock_end
#define dratomic_rwlock_end drshared_rwlock_end
#define datomic_rwlock_upgrade dshared_rwlock_upgrade
#define dratomic_rwlock_upgrade drshared_rwlock_upgrade
#define datomic_rwlock_timedread dshared_rwlock_timedread
#define dratomic_rwlock_timedread drshared_rwlock_timedread
#define datomic_rwlock_timedwrite dshared_rwlock_timedwrite
#define dratomic_rwlock_timedwrite drshared_rwlock_timedwrite
#define datomic_rwlock_waitread dshared_rwlock_waitread
#define dratomic_rwlock_waitread drshared_rwlock_waitread
#define datomic_rwlock_waitwrite dshared_rwlock_waitwrite
#define dratomic_rwlock_waitwrite drshared_rwlock_waitwrite
#define datomic_rwlock_timedwaitread dshared_rwlock_timedwaitread
#define dratomic_rwlock_timedwaitread drshared_rwlock_timedwaitread
#define datomic_rwlock_timedwaitwrite dshared_rwlock_timedwaitwrite
#define dratomic_rwlock_timedwaitwrite drshared_rwlock_timedwaitwrite
#define datomic_rwlock_canread_get dshared_rwlock_canread_get
#define dratomic_rwlock_canread_get drshared_rwlock_canread_get
#define datomic_rwlock_canwrite_get dshared_rwlock_canwrite_get
#define dratomic_rwlock_canwrite_get drshared_rwlock_canwrite_get
#define datomic_rwlock_reading_get dshared_rwlock_reading_get
#define dratomic_rwlock_reading_get drshared_rwlock_reading_get
#define datomic_rwlock_writing_get dshared_rwlock_writing_get
#define dratomic_rwlock_writing_get drshared_rwlock_writing_get
#define datomic_rwlock_readlock_get dshared_rwlock_readlock_get
#define dratomic_rwlock_readlock_get drshared_rwlock_readlock_get
#define datomic_rwlock_writelock_get dshared_rwlock_writelock_get
#define dratomic_rwlock_writelock_get drshared_rwlock_writelock_get
#define datomic_rwlock_methods dshared_rwlock_methods
#define dratomic_rwlock_methods drshared_rwlock_methods
#define datomic_rwlock_getsets dshared_rwlock_getsets
#define dratomic_rwlock_getsets drshared_rwlock_getsets
#define datomic_rwlock_readlock_enter dshared_rwlock_readlock_enter
#define dratomic_rwlock_readlock_enter drshared_rwlock_readlock_enter
#define datomic_rwlock_writelock_enter dshared_rwlock_writelock_enter
#define dratomic_rwlock_writelock_enter drshared_rwlock_writelock_enter
#define datomic_rwlock_readlock_leave dshared_rwlock_readlock_leave
#define dratomic_rwlock_readlock_leave drshared_rwlock_readlock_leave
#define datomic_rwlock_writelock_leave dshared_rwlock_writelock_leave
#define dratomic_rwlock_writelock_leave drshared_rwlock_writelock_leave
#define datomic_rwlock_readlock_tryacquire dshared_rwlock_readlock_tryacquire
#define dratomic_rwlock_readlock_tryacquire drshared_rwlock_readlock_tryacquire
#define datomic_rwlock_writelock_tryacquire dshared_rwlock_writelock_tryacquire
#define dratomic_rwlock_writelock_tryacquire drshared_rwlock_writelock_tryacquire
#define datomic_rwlock_readlock_acquire dshared_rwlock_readlock_acquire
#define dratomic_rwlock_readlock_acquire drshared_rwlock_readlock_acquire
#define datomic_rwlock_writelock_acquire dshared_rwlock_writelock_acquire
#define dratomic_rwlock_writelock_acquire drshared_rwlock_writelock_acquire
#define datomic_rwlock_readlock_release dshared_rwlock_readlock_release
#define dratomic_rwlock_readlock_release drshared_rwlock_readlock_release
#define datomic_rwlock_writelock_release dshared_rwlock_writelock_release
#define dratomic_rwlock_writelock_release drshared_rwlock_writelock_release
#define datomic_rwlock_readlock_timedacquire dshared_rwlock_readlock_timedacquire
#define dratomic_rwlock_readlock_timedacquire drshared_rwlock_readlock_timedacquire
#define datomic_rwlock_writelock_timedacquire dshared_rwlock_writelock_timedacquire
#define dratomic_rwlock_writelock_timedacquire drshared_rwlock_writelock_timedacquire
#define datomic_rwlock_readlock_waitfor dshared_rwlock_readlock_waitfor
#define dratomic_rwlock_readlock_waitfor drshared_rwlock_readlock_waitfor
#define datomic_rwlock_writelock_waitfor dshared_rwlock_writelock_waitfor
#define dratomic_rwlock_writelock_waitfor drshared_rwlock_writelock_waitfor
#define datomic_rwlock_readlock_timedwaitfor dshared_rwlock_readlock_timedwaitfor
#define dratomic_rwlock_readlock_timedwaitfor drshared_rwlock_readlock_timedwaitfor
#define datomic_rwlock_writelock_timedwaitfor dshared_rwlock_writelock_timedwaitfor
#define dratomic_rwlock_writelock_timedwaitfor drshared_rwlock_writelock_timedwaitfor
#define datomic_rwlock_readlock_available_get dshared_rwlock_readlock_available_get
#define dratomic_rwlock_readlock_available_get drshared_rwlock_readlock_available_get
#define datomic_rwlock_writelock_available_get dshared_rwlock_writelock_available_get
#define dratomic_rwlock_writelock_available_get drshared_rwlock_writelock_available_get
#define datomic_rwlock_readlock_acquired_get dshared_rwlock_readlock_acquired_get
#define dratomic_rwlock_readlock_acquired_get drshared_rwlock_readlock_acquired_get
#define datomic_rwlock_writelock_acquired_get dshared_rwlock_writelock_acquired_get
#define dratomic_rwlock_writelock_acquired_get drshared_rwlock_writelock_acquired_get
#define datomic_rwlock_readlock_with dshared_rwlock_readlock_with
#define dratomic_rwlock_readlock_with drshared_rwlock_readlock_with
#define datomic_rwlock_writelock_with dshared_rwlock_writelock_with
#define dratomic_rwlock_writelock_with drshared_rwlock_writelock_with
#define datomic_rwlock_readlock_methods dshared_rwlock_readlock_methods
#define dratomic_rwlock_readlock_methods drshared_rwlock_readlock_methods
#define datomic_rwlock_writelock_methods dshared_rwlock_writelock_methods
#define dratomic_rwlock_writelock_methods drshared_rwlock_writelock_methods
#define datomic_rwlock_readlock_getsets dshared_rwlock_readlock_getsets
#define dratomic_rwlock_readlock_getsets drshared_rwlock_readlock_getsets
#define datomic_rwlock_writelock_getsets dshared_rwlock_writelock_getsets
#define dratomic_rwlock_writelock_getsets drshared_rwlock_writelock_getsets
#endif /* !DEEMON_LIBTHREADING_LOCKS_ATOMIC_AS_SHARED_DEFINED */
#endif /* LOCAL_IS_ATOMIC_AS_SHARED */
/*[[[end]]]*/

/************************************************************************/
/* Lock                                                                 */
/************************************************************************/
#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_lockapi_ctor(LOCAL_DeeLockObject *__restrict self) {
	LOCAL_lock_init(&self->l_lock);
	return 0;
}
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */

#ifdef LOCAL_lock_init_acquired
#ifndef DEFINED_lock_init_acquired_kwlist
#define DEFINED_lock_init_acquired_kwlist
PRIVATE DEFINE_KWLIST(lock_init_acquired_kwlist, { K(acquired), KEND });
#endif /* !DEFINED_lock_init_acquired_kwlist */

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_lockapi_init_kw(LOCAL_DeeLockObject *__restrict self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	bool acquired = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, lock_init_acquired_kwlist,
	                          "|b:" LOCAL_S_Lock, &acquired))
		goto err;
	if (acquired) {
		LOCAL_lock_init_acquired(&self->l_lock);
	} else {
		LOCAL_lock_init(&self->l_lock);
	}
	return 0;
err:
	return -1;
}
#else /* LOCAL_lock_init_acquired */
#undef LOCAL_lockapi_init_kw
#endif /* !LOCAL_lock_init_acquired */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_lockapi_printrepr(LOCAL_DeeLockObject *__restrict self,
                        Dee_formatprinter_t printer, void *arg) {
	(void)self;
#ifdef LOCAL_lock_init_kw
	return DeeFormat_Printf(printer, arg,
	                        LOCAL_S_Lock "(acquired: %s)",
	                        LOCAL_lock_acquired(&self->l_lock)
	                        ? "true"
	                        : "false");
#else /* LOCAL_lock_init_kw */
	return DeeFormat_PRINT(printer, arg, LOCAL_S_Lock "()");
#endif /* !LOCAL_lock_init_kw */
}

#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_lockapi_enter(LOCAL_DeeLockObject *__restrict self) {
	LOCAL_lock_acquire_p(&self->l_lock, err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_lockapi_leave(LOCAL_DeeLockObject *__restrict self) {
	if unlikely(!LOCAL_lock_acquired(&self->l_lock))
		goto err_not_acquired;
	_LOCAL_lock_release_NDEBUG(&self->l_lock);
	return 0;
err_not_acquired:
	return err_lock_not_acquired(Dee_AsObject(self));
}

PRIVATE struct type_with LOCAL_lockapi_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_lockapi_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_lockapi_leave
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_tryacquire(LOCAL_DeeLockObject *self,
                         size_t argc, DeeObject *const *argv) {
	bool result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tryacquire");]]]*/
	DeeArg_Unpack0(err, argc, argv, "tryacquire");
/*[[[end]]]*/
	result = LOCAL_lock_tryacquire(&self->l_lock);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_acquire(LOCAL_DeeLockObject *self,
                      size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("acquire");]]]*/
	DeeArg_Unpack0(err, argc, argv, "acquire");
/*[[[end]]]*/
	LOCAL_lock_acquire_p(&self->l_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_timedacquire(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedacquire", params: "uint64_t timeout_nanoseconds");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedacquire", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	LOCAL_lock_acquire_timed_p(&self->l_lock, args.timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_waitfor(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("waitfor");]]]*/
	DeeArg_Unpack0(err, argc, argv, "waitfor");
/*[[[end]]]*/
	LOCAL_lock_waitfor_p(&self->l_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_timedwaitfor(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitfor", params: "uint64_t timeout_nanoseconds");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitfor", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	LOCAL_lock_waitfor_timed_p(&self->l_lock, args.timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_release(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("release");]]]*/
	DeeArg_Unpack0(err, argc, argv, "release");
/*[[[end]]]*/
	if unlikely(LOCAL_lockapi_leave(self) != 0)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_available_get(LOCAL_DeeLockObject *self) {
	return_bool(LOCAL_lock_available(&self->l_lock));
}

PRIVATE struct type_method tpconst LOCAL_lockapi_methods[] = {
	TYPE_METHOD_F(STR_tryacquire, &LOCAL_lockapi_tryacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD_F(STR_acquire, &LOCAL_lockapi_acquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD_F(STR_release, &LOCAL_lockapi_release, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_release)),
	TYPE_METHOD_F(STR_timedacquire, &LOCAL_lockapi_timedacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD_F(STR_waitfor, &LOCAL_lockapi_waitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD_F(STR_timedwaitfor, &LOCAL_lockapi_timedwaitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */

#ifdef LOCAL_IS_RECURSIVE
#undef LOCAL_lockapi_members
#else /* LOCAL_IS_RECURSIVE */
#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE struct type_member tpconst LOCAL_lockapi_members[] = {
#ifdef CONFIG_NO_THREADS
	TYPE_MEMBER_FIELD_DOC(STR_acquired,
	                      STRUCT_CONST | STRUCT_ATOMIC | STRUCT_BOOL(1),
	                      offsetof(LOCAL_DeeLockObject, l_lock),
	                      DOC_GET(doc_lock_acquired)),
#else /* CONFIG_NO_THREADS */
#ifdef LOCAL_IS_SHARED
	TYPE_MEMBER_FIELD_DOC(STR_acquired,
	                      STRUCT_CONST | STRUCT_ATOMIC | STRUCT_BOOL(Dee_SIZEOF_ATOMIC_LOCK),
	                      offsetof(LOCAL_DeeLockObject, l_lock.s_lock.a_lock),
	                      DOC_GET(doc_lock_acquired)),
#else /* LOCAL_IS_SHARED */
	TYPE_MEMBER_FIELD_DOC(STR_acquired,
	                      STRUCT_CONST | STRUCT_ATOMIC | STRUCT_BOOL(Dee_SIZEOF_ATOMIC_LOCK),
	                      offsetof(LOCAL_DeeLockObject, l_lock.a_lock),
	                      DOC_GET(doc_lock_acquired)),
#endif /* !LOCAL_IS_SHARED */
#endif /* !CONFIG_NO_THREADS */
	TYPE_MEMBER_END
};
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */
#endif /* !LOCAL_IS_RECURSIVE */

#ifdef LOCAL_lockapi_members
#undef LOCAL_lockapi_acquired_get
#else /* LOCAL_lockapi_members */
#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_acquired_get(LOCAL_DeeLockObject *self) {
	return_bool(LOCAL_lock_acquired(&self->l_lock));
}
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */
#endif /* !LOCAL_lockapi_members */


#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE struct type_getset tpconst LOCAL_lockapi_getsets[] = {
	TYPE_GETTER_F(STR_available, &LOCAL_lockapi_available_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_available)),
#ifdef LOCAL_lockapi_acquired_get
	TYPE_GETTER_F(STR_acquired, &LOCAL_lockapi_acquired_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquired)),
#endif /* LOCAL_lockapi_acquired_get */
	TYPE_GETSET_END
};
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */


INTERN DeeTypeObject LOCAL_DeeLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_Lock,
#ifdef CONFIG_NO_DOC
	/* .tp_doc      = */ NULL,
#else /* CONFIG_NO_DOC */
	/* .tp_doc      = */ ""
#ifdef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
	                     "Atomic (spin) lock using ?Ayield?DThread to block until the lock becomes available. "
	                     /**/ "The recursive version of this lock is ?GRAtomicLock, and the preemptive version is ?GSharedLock.\n"
#elif defined(DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type)
	                     "Shared (preemptive) lock using ?Ectypes:futex_wait and ?Ectypes:futex_timedwait to block until the lock becomes available. "
	                     /**/ "The recursive version of this lock is ?GRSharedLock, and the atomic version is ?GAtomicLock.\n"
#elif defined(DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type)
	                     "Recursive, atomic (spin) lock using ?Ayield?DThread to block until the lock becomes available. "
	                     /**/ "The non-recursive version of this lock is ?GAtomicLock, and the preemptive version is ?GRSharedLock.\n"
#elif defined(DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type)
	                     "Recursive, shared (preemptive) lock using ?Ectypes:futex_wait and ?Ectypes:futex_timedwait to block until the lock becomes available. "
	                     /**/ "The non-recursive version of this lock is ?GSharedLock, and the atomic version is ?GRAtomicLock.\n"
#endif /* ... */
	                     "\n"
#ifdef LOCAL_lockapi_init_kw
	                     "(acquired=!f)"
#endif /* LOCAL_lockapi_init_kw */
	                     "",
#endif /* !CONFIG_NO_DOC */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
#ifdef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
	/* .tp_base     = */ &DeeLock_Type,
#elif defined(DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type)
	/* .tp_base     = */ &DeeAtomicLock_Type,
#elif defined(DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type)
	/* .tp_base     = */ &DeeLock_Type,
#elif defined(DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type)
	/* .tp_base     = */ &DeeRAtomicLock_Type,
#else /* ... */
	/* .tp_base     = */ &DeeLock_Type,
#endif /* !... */
	/* .tp_init = */ {
#ifdef LOCAL_lockapi_init_kw
#define LOCAL_lockapi_init_kw_PTR &LOCAL_lockapi_init_kw
#else /* LOCAL_lockapi_init_kw */
#define LOCAL_lockapi_init_kw_PTR NULL
#endif /* !LOCAL_lockapi_init_kw */
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ LOCAL_DeeLockObject,
			/* tp_ctor:        */ &LOCAL_lockapi_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ LOCAL_lockapi_init_kw_PTR,
			/* tp_serialize:   */ NULL /* TODO (writes the output lock matching the "acquired" ctor argument (or unlock if no such argument)) */
		),
#undef LOCAL_lockapi_init_kw_PTR
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&LOCAL_lockapi_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &LOCAL_lockapi_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ LOCAL_lockapi_methods,
	/* .tp_getsets       = */ LOCAL_lockapi_getsets,
#ifdef LOCAL_lockapi_members
	/* .tp_members       = */ LOCAL_lockapi_members,
#else /* LOCAL_lockapi_members */
	/* .tp_members       = */ NULL,
#endif /* !LOCAL_lockapi_members */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};







/************************************************************************/
/* RWLock                                                               */
/************************************************************************/
#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_ctor(LOCAL_DeeRWLockObject *__restrict self) {
	LOCAL_rwlock_init(&self->rwl_lock);
	return 0;
}
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */

#ifndef DEFINED_rwlock_init_readers_writing_kwlist
#define DEFINED_rwlock_init_readers_writing_kwlist
PRIVATE DEFINE_KWLIST(rwlock_init_readers_writing_kwlist, { K(readers), K(writing), KEND });
#endif /* !DEFINED_rwlock_init_readers_writing_kwlist */

#if !defined(LOCAL_rwlock_init_write) || !defined(LOCAL_rwlock_init_read)
#undef LOCAL_rwlockapi_init_kw
#else /* !LOCAL_rwlock_init_write || !LOCAL_rwlock_init_read */
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_init_kw(LOCAL_DeeRWLockObject *__restrict self,
                        size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		uintptr_t readers;
		bool writing;
	} args;
	args.readers = 0;
	args.writing = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, rwlock_init_readers_writing_kwlist,
	                          "|" UNPuPTR "b:" LOCAL_S_RWLock, &args))
		goto err;
	if (args.writing) {
		if unlikely(args.readers != 0)
			goto err_cannot_initialize_readers_with_writers;

		/* Initialize in write-mode */
		LOCAL_rwlock_init_write(&self->rwl_lock);
	} else {
		if unlikely(args.readers > LOCAL_RWLOCK_MAX_READERS)
			goto err_readers_counter_is_too_large;
		LOCAL_rwlock_init_read(&self->rwl_lock, args.readers);
	}
	return 0;
err_cannot_initialize_readers_with_writers:
	return err_rwlock_with_readers_and_writers();
err_readers_counter_is_too_large:
	return err_rwlock_too_many_readers(args.readers);
err:
	return -1;
}
#endif /* LOCAL_rwlock_init_write && LOCAL_rwlock_init_read */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_rwlockapi_printrepr(LOCAL_DeeRWLockObject *__restrict self,
                          Dee_formatprinter_t printer, void *arg) {
#ifdef LOCAL_rwlockapi_init_kw
#ifdef CONFIG_NO_THREADS
	uintptr_t status = atomic_read(&self->rwl_lock);
#elif defined(LOCAL_IS_SHARED)
	uintptr_t status = atomic_read(&self->rwl_lock.srw_lock.arw_lock);
#else /* LOCAL_IS_SHARED */
	uintptr_t status = atomic_read(&self->rwl_lock.arw_lock);
#endif /* !LOCAL_IS_SHARED */
	if (status == (uintptr_t)-1)
		return DeeFormat_PRINT(printer, arg, LOCAL_S_RWLock "(writing: true)");
	if (status == 0)
		return DeeFormat_PRINT(printer, arg, LOCAL_S_RWLock "()");
	return DeeFormat_Printf(printer, arg,
	                        LOCAL_S_RWLock "(readers: %" PRFuPTR ")",
	                        status);
#else /* LOCAL_rwlockapi_init_kw */
	(void)self;
	return DeeFormat_PRINT(printer, arg, LOCAL_S_RWLock "()");
#endif /* !LOCAL_rwlockapi_init_kw */
}

#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_tryread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tryread");]]]*/
	DeeArg_Unpack0(err, argc, argv, "tryread");
/*[[[end]]]*/
	return_bool(LOCAL_rwlock_tryread(&self->rwl_lock));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_trywrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("trywrite");]]]*/
	DeeArg_Unpack0(err, argc, argv, "trywrite");
/*[[[end]]]*/
	return_bool(LOCAL_rwlock_trywrite(&self->rwl_lock));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_tryupgrade(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tryupgrade");]]]*/
	DeeArg_Unpack0(err, argc, argv, "tryupgrade");
/*[[[end]]]*/
	return_bool(LOCAL_rwlock_tryupgrade(&self->rwl_lock));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_endread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("endread");]]]*/
	DeeArg_Unpack0(err, argc, argv, "endread");
/*[[[end]]]*/
	if unlikely(!LOCAL_rwlock_canendread(&self->rwl_lock))
		goto err_no_read_lock;
	_LOCAL_rwlock_endread_NDEBUG(&self->rwl_lock);
	return_none;
err_no_read_lock:
	err_read_lock_not_acquired(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_endwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("endwrite");]]]*/
	DeeArg_Unpack0(err, argc, argv, "endwrite");
/*[[[end]]]*/
	if unlikely(!LOCAL_rwlock_canendwrite(&self->rwl_lock))
		goto err_no_write_lock;
	_LOCAL_rwlock_endwrite_NDEBUG(&self->rwl_lock);
	return_none;
err_no_write_lock:
	err_write_lock_not_acquired(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_end(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("end");]]]*/
	DeeArg_Unpack0(err, argc, argv, "end");
/*[[[end]]]*/
	if unlikely(!LOCAL_rwlock_canend(&self->rwl_lock))
		goto err_nolock;
	_LOCAL_rwlock_end_NDEBUG(&self->rwl_lock);
	return_none;
err_nolock:
	err_read_or_write_lock_not_acquired(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_downgrade(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("downgrade");]]]*/
	DeeArg_Unpack0(err, argc, argv, "downgrade");
/*[[[end]]]*/
	if unlikely(!LOCAL_rwlock_canendwrite(&self->rwl_lock))
		goto err_no_write_lock;
	_LOCAL_rwlock_downgrade_NDEBUG(&self->rwl_lock);
	return_none;
err_no_write_lock:
	err_write_lock_not_acquired(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_read(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("read");]]]*/
	DeeArg_Unpack0(err, argc, argv, "read");
/*[[[end]]]*/
	LOCAL_rwlock_read_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_write(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("write");]]]*/
	DeeArg_Unpack0(err, argc, argv, "write");
/*[[[end]]]*/
	LOCAL_rwlock_write_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_upgrade(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("upgrade");]]]*/
	DeeArg_Unpack0(err, argc, argv, "upgrade");
/*[[[end]]]*/
	if (LOCAL_rwlock_tryupgrade(&self->rwl_lock))
		return_true;
	if unlikely(!LOCAL_rwlock_canendread(&self->rwl_lock))
		goto err_no_read_lock;
	_LOCAL_rwlock_endread_NDEBUG(&self->rwl_lock);
	LOCAL_rwlock_write_p(&self->rwl_lock, err);
	return_false;
err_no_read_lock:
	err_read_lock_not_acquired(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedread", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedread", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	LOCAL_rwlock_read_timed_p(&self->rwl_lock, args.timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwrite", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwrite", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	LOCAL_rwlock_write_timed_p(&self->rwl_lock, args.timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_waitread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("waitread");]]]*/
	DeeArg_Unpack0(err, argc, argv, "waitread");
/*[[[end]]]*/
	LOCAL_rwlock_waitread_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_waitwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("waitwrite");]]]*/
	DeeArg_Unpack0(err, argc, argv, "waitwrite");
/*[[[end]]]*/
	LOCAL_rwlock_waitwrite_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedwaitread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitread", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitread", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	LOCAL_rwlock_waitread_timed_p(&self->rwl_lock, args.timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedwaitwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitwrite", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitwrite", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	LOCAL_rwlock_waitwrite_timed_p(&self->rwl_lock, args.timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_canread_get(LOCAL_DeeRWLockObject *__restrict self) {
	return_bool(LOCAL_rwlock_canread(&self->rwl_lock));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_canwrite_get(LOCAL_DeeRWLockObject *__restrict self) {
	return_bool(LOCAL_rwlock_canwrite(&self->rwl_lock));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_reading_get(LOCAL_DeeRWLockObject *__restrict self) {
	return_bool(LOCAL_rwlock_reading(&self->rwl_lock));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writing_get(LOCAL_DeeRWLockObject *__restrict self) {
	return_bool(LOCAL_rwlock_writing(&self->rwl_lock));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeGenericRWLockProxyObject *DCALL
LOCAL_rwlockapi_readlock_get(LOCAL_DeeRWLockObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = Dee_AsObject(self);
	Dee_Incref(self);
	DeeObject_Init(result, &LOCAL_DeeRWLockReadLock_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeGenericRWLockProxyObject *DCALL
LOCAL_rwlockapi_writelock_get(LOCAL_DeeRWLockObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = Dee_AsObject(self);
	Dee_Incref(self);
	DeeObject_Init(result, &LOCAL_DeeRWLockWriteLock_Type);
done:
	return result;
}

PRIVATE struct type_method tpconst LOCAL_rwlockapi_methods[] = {
	TYPE_METHOD_F(STR_tryread, &LOCAL_rwlockapi_tryread, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_tryread)),
	TYPE_METHOD_F(STR_trywrite, &LOCAL_rwlockapi_trywrite, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_trywrite)),
	TYPE_METHOD_F(STR_tryupgrade, &LOCAL_rwlockapi_tryupgrade, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_tryupgrade)),
	TYPE_METHOD_F(STR_endread, &LOCAL_rwlockapi_endread, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_endread)),
	TYPE_METHOD_F(STR_endwrite, &LOCAL_rwlockapi_endwrite, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_endwrite)),
	TYPE_METHOD_F(STR_downgrade, &LOCAL_rwlockapi_downgrade, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_downgrade)),
	TYPE_METHOD_F(STR_read, &LOCAL_rwlockapi_read, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_read)),
	TYPE_METHOD_F(STR_write, &LOCAL_rwlockapi_write, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_write)),
	TYPE_METHOD_F(STR_end, &LOCAL_rwlockapi_end, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_end)),
	TYPE_METHOD_F(STR_upgrade, &LOCAL_rwlockapi_upgrade, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_upgrade)),
	TYPE_METHOD_F(STR_timedread, &LOCAL_rwlockapi_timedread, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_timedread)),
	TYPE_METHOD_F(STR_timedwrite, &LOCAL_rwlockapi_timedwrite, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_timedwrite)),
	TYPE_METHOD_F(STR_waitread, &LOCAL_rwlockapi_waitread, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_waitread)),
	TYPE_METHOD_F(STR_waitwrite, &LOCAL_rwlockapi_waitwrite, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_waitwrite)),
	TYPE_METHOD_F(STR_timedwaitread, &LOCAL_rwlockapi_timedwaitread, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_timedwaitread)),
	TYPE_METHOD_F(STR_timedwaitwrite, &LOCAL_rwlockapi_timedwaitwrite, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_timedwaitwrite)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst LOCAL_rwlockapi_getsets[] = {
	TYPE_GETTER_F(STR_reading, &LOCAL_rwlockapi_reading_get, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_reading)),
	TYPE_GETTER_F(STR_writing, &LOCAL_rwlockapi_writing_get, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_writing)),
	TYPE_GETTER_F(STR_canread, &LOCAL_rwlockapi_canread_get, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_canread)),
	TYPE_GETTER_F(STR_canwrite, &LOCAL_rwlockapi_canwrite_get, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_canwrite)),
	TYPE_GETTER_F(STR_readlock, &LOCAL_rwlockapi_readlock_get, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_readlock)),
	TYPE_GETTER_F(STR_writelock, &LOCAL_rwlockapi_writelock_get, METHOD_FNOREFESCAPE, DOC_GET(doc_rwlock_writelock)),
	TYPE_GETSET_END
};
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */

INTERN DeeTypeObject LOCAL_DeeRWLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_RWLock,
#ifdef CONFIG_NO_DOC
	/* .tp_doc      = */ NULL,
#else /* CONFIG_NO_DOC */
	/* .tp_doc      = */ ""
#ifdef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
	                     "Atomic (spin) read/write lock using ?Ayield?DThread to block until the lock becomes available. "
	                     /**/ "The recursive version of this lock is ?GRAtomicRWLock, and the preemptive version is ?GSharedRWLock.\n"
#elif defined(DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type)
	                     "Shared (preemptive) read/write lock using ?Ectypes:futex_wait and ?Ectypes:futex_timedwait to block until the lock becomes available. "
	                     /**/ "The recursive version of this lock is ?GRSharedRWLock, and the atomic version is ?GAtomicRWLock.\n"
#elif defined(DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type)
	                     "Recursive, Atomic (spin) read/write lock using ?Ayield?DThread to block until the lock becomes available. "
	                     /**/ "The non-recursive version of this lock is ?GAtomicRWLock, and the preemptive version is ?GRSharedRWLock.\n"
#elif defined(DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type)
	                     "Recursive, Shared (preemptive) read/write lock using ?Ectypes:futex_wait and ?Ectypes:futex_timedwait to block until the lock becomes available. "
	                     /**/ "The non-recursive version of this lock is ?GSharedRWLock, and the atomic version is ?GRAtomicRWLock.\n"
#endif /* ... */
	                     "\n"
#ifdef LOCAL_rwlockapi_init_kw
	                     "(readers=!0,writing=!f)"
#endif /* LOCAL_rwlockapi_init_kw */
	                     "",
#endif /* !CONFIG_NO_DOC */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
#ifdef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
	/* .tp_base     = */ &DeeRWLock_Type,
#elif defined(DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type)
	/* .tp_base     = */ &DeeAtomicRWLock_Type,
#elif defined(DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type)
	/* .tp_base     = */ &DeeRWLock_Type,
#elif defined(DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type)
	/* .tp_base     = */ &DeeRAtomicRWLock_Type,
#else /* ... */
	/* .tp_base     = */ &DeeRWLock_Type,
#endif /* !... */
	/* .tp_init = */ {
#ifdef LOCAL_rwlockapi_init_kw
#define LOCAL_rwlockapi_init_kw_PTR &LOCAL_rwlockapi_init_kw
#else /* LOCAL_rwlockapi_init_kw */
#define LOCAL_rwlockapi_init_kw_PTR NULL
#endif /* !LOCAL_rwlockapi_init_kw */
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ LOCAL_DeeRWLockObject,
			/* tp_ctor:        */ &LOCAL_rwlockapi_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ LOCAL_rwlockapi_init_kw_PTR,
			/* tp_serialize:   */ NULL /* TODO (writes the output lock as unlocked) */
		),
#undef LOCAL_rwlockapi_init_kw_PTR
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&LOCAL_rwlockapi_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ LOCAL_rwlockapi_methods,
	/* .tp_getsets       = */ LOCAL_rwlockapi_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_readlock_init(DeeGenericRWLockProxyObject *__restrict self,
                              size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, LOCAL_S_RWLockReadLock, &self->grwl_lock);
	if (DeeObject_AssertType(self->grwl_lock, &LOCAL_DeeRWLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_writelock_init(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, LOCAL_S_RWLockWriteLock, &self->grwl_lock);
	if (DeeObject_AssertType(self->grwl_lock, &LOCAL_DeeRWLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

#ifndef LOCAL_IS_ATOMIC_AS_SHARED
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_readlock_enter(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	LOCAL_rwlock_read_p(&rwlock->rwl_lock, err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_readlock_leave(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	if unlikely(!LOCAL_rwlock_canendread(&rwlock->rwl_lock))
		goto err_no_read_lock;
	_LOCAL_rwlock_endread_NDEBUG(&rwlock->rwl_lock);
	return 0;
err_no_read_lock:
	return err_read_lock_not_acquired((DeeObject *)rwlock);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_writelock_enter(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	LOCAL_rwlock_write_p(&rwlock->rwl_lock, err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_writelock_leave(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	if unlikely(!LOCAL_rwlock_canendwrite(&rwlock->rwl_lock))
		goto err_no_write_lock;
	_LOCAL_rwlock_endwrite_NDEBUG(&rwlock->rwl_lock);
	return 0;
err_no_write_lock:
	return err_write_lock_not_acquired((DeeObject *)rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_tryacquire(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_tryread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_tryacquire(DeeGenericRWLockProxyObject *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_trywrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_acquire(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_read(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_acquire(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_write(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_release(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_endread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_release(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_endwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_timedacquire(DeeGenericRWLockProxyObject *__restrict self,
                                    size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_timedacquire(DeeGenericRWLockProxyObject *__restrict self,
                                       size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_waitfor(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_waitread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_waitfor(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_waitwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_timedwaitfor(DeeGenericRWLockProxyObject *__restrict self,
                                    size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedwaitread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_timedwaitfor(DeeGenericRWLockProxyObject *__restrict self,
                                       size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedwaitwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_canread_get(rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_canwrite_get(rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_readlock_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_reading_get(rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_writelock_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_writing_get(rwlock);
}

PRIVATE struct type_with LOCAL_rwlockapi_readlock_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_readlock_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_readlock_leave
};

PRIVATE struct type_with LOCAL_rwlockapi_writelock_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_writelock_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_writelock_leave
};

PRIVATE struct type_method tpconst LOCAL_rwlockapi_readlock_methods[] = {
	TYPE_METHOD_F(STR_tryacquire, &LOCAL_rwlockapi_readlock_tryacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD_F(STR_acquire, &LOCAL_rwlockapi_readlock_acquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD_F(STR_release, &LOCAL_rwlockapi_readlock_release, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_release)),
	TYPE_METHOD_F(STR_timedacquire, &LOCAL_rwlockapi_readlock_timedacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD_F(STR_waitfor, &LOCAL_rwlockapi_readlock_waitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD_F(STR_timedwaitfor, &LOCAL_rwlockapi_readlock_timedwaitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_method tpconst LOCAL_rwlockapi_writelock_methods[] = {
	TYPE_METHOD_F(STR_tryacquire, &LOCAL_rwlockapi_writelock_tryacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD_F(STR_acquire, &LOCAL_rwlockapi_writelock_acquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD_F(STR_release, &LOCAL_rwlockapi_writelock_release, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_release)),
	TYPE_METHOD_F(STR_timedacquire, &LOCAL_rwlockapi_writelock_timedacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD_F(STR_waitfor, &LOCAL_rwlockapi_writelock_waitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD_F(STR_timedwaitfor, &LOCAL_rwlockapi_writelock_timedwaitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst LOCAL_rwlockapi_readlock_getsets[] = {
	TYPE_GETTER_F(STR_available, &LOCAL_rwlockapi_readlock_available_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_available)),
	TYPE_GETTER_F(STR_acquired, &LOCAL_rwlockapi_readlock_acquired_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst LOCAL_rwlockapi_writelock_getsets[] = {
	TYPE_GETTER_F(STR_available, &LOCAL_rwlockapi_writelock_available_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_available)),
	TYPE_GETTER_F(STR_acquired, &LOCAL_rwlockapi_writelock_acquired_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};
#endif /* !LOCAL_IS_ATOMIC_AS_SHARED */

INTERN DeeTypeObject LOCAL_DeeRWLockReadLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_RWLockReadLock,
	/* .tp_doc      = */ DOC("Wrapper for creating/holding read-locks for ?G" LOCAL_S_RWLock "\n"
	                         "\n"
	                         "(lock:?G" LOCAL_S_RWLock ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeRWLockReadLock_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeGenericRWLockProxyObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &LOCAL_rwlockapi_readlock_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &LOCAL_rwlockapi_readlock_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ LOCAL_rwlockapi_readlock_methods,
	/* .tp_getsets       = */ LOCAL_rwlockapi_readlock_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
INTERN DeeTypeObject LOCAL_DeeRWLockWriteLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_RWLockWriteLock,
	/* .tp_doc      = */ DOC("Wrapper for creating/holding write-locks for ?G" LOCAL_S_RWLock "\n"
	                         "\n"
	                         "(lock:?G" LOCAL_S_RWLock ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeRWLockWriteLock_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeGenericRWLockProxyObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &LOCAL_rwlockapi_writelock_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &LOCAL_rwlockapi_writelock_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ LOCAL_rwlockapi_writelock_methods,
	/* .tp_getsets       = */ LOCAL_rwlockapi_writelock_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


#undef LOCAL_IS_ATOMIC_AS_SHARED

#undef LOCAL_S_Atomic_or_Shared
#undef LOCAL_S_MaybeRecursiveR
#undef LOCAL_S_Lock
#undef LOCAL_S_RWLock
#undef LOCAL_S_RWLockReadLock
#undef LOCAL_S_RWLockWriteLock

#undef LOCAL_lockapi_func
#undef LOCAL_DeeLockObject
#undef LOCAL_DeeLock_Type
#undef LOCAL_lock_t
#undef LOCAL_lock_init
#undef LOCAL_lock_init_acquired
#undef LOCAL_lock_cinit
#undef LOCAL_lock_cinit_acquired
#undef LOCAL_lock_available
#undef LOCAL_lock_acquired
#undef LOCAL_lock_tryacquire
#undef LOCAL_lock_acquire
#undef LOCAL_lock_waitfor
#undef LOCAL_lock_acquire_timed
#undef LOCAL_lock_waitfor_timed
#undef _LOCAL_lock_release_NDEBUG
#undef LOCAL_lock_acquire_p
#undef LOCAL_lock_waitfor_p
#undef LOCAL_lock_acquire_timed_p
#undef LOCAL_lock_waitfor_timed_p

#undef LOCAL_rwlockapi_func
#undef LOCAL_DeeRWLockObject
#undef LOCAL_DeeRWLock_Type
#undef LOCAL_DeeRWLockReadLock_Type
#undef LOCAL_DeeRWLockWriteLock_Type
#undef LOCAL_rwlock_t
#undef LOCAL_RWLOCK_MAX_READERS
#undef LOCAL_rwlock_cinit
#undef LOCAL_rwlock_cinit_read
#undef LOCAL_rwlock_cinit_write
#undef LOCAL_rwlock_init
#undef LOCAL_rwlock_init_read
#undef LOCAL_rwlock_init_write
#undef LOCAL_rwlock_reading
#undef LOCAL_rwlock_writing
#undef LOCAL_rwlock_tryread
#undef LOCAL_rwlock_trywrite
#undef LOCAL_rwlock_canread
#undef LOCAL_rwlock_canwrite
#undef LOCAL_rwlock_canendread
#undef LOCAL_rwlock_canendwrite
#undef LOCAL_rwlock_canend
#undef LOCAL_rwlock_tryupgrade
#undef _LOCAL_rwlock_downgrade_NDEBUG
#undef _LOCAL_rwlock_endwrite_NDEBUG
#undef _LOCAL_rwlock_endread_NDEBUG
#undef _LOCAL_rwlock_end_NDEBUG
#undef _LOCAL_rwlock_endread_ex_NDEBUG
#undef _LOCAL_rwlock_end_ex_NDEBUG
#undef LOCAL_rwlock_waitread
#undef LOCAL_rwlock_waitwrite
#undef LOCAL_rwlock_waitread_timed
#undef LOCAL_rwlock_waitwrite_timed
#undef LOCAL_rwlock_read
#undef LOCAL_rwlock_write
#undef LOCAL_rwlock_read_timed
#undef LOCAL_rwlock_write_timed
#undef LOCAL_rwlock_waitread_p
#undef LOCAL_rwlock_waitwrite_p
#undef LOCAL_rwlock_waitread_timed_p
#undef LOCAL_rwlock_waitwrite_timed_p
#undef LOCAL_rwlock_read_p
#undef LOCAL_rwlock_write_p
#undef LOCAL_rwlock_read_timed_p
#undef LOCAL_rwlock_write_timed_p

#undef LOCAL_lockapi_ctor
#undef LOCAL_lockapi_init_kw
#undef LOCAL_lockapi_printrepr
#undef LOCAL_lockapi_enter
#undef LOCAL_lockapi_leave
#undef LOCAL_lockapi_with
#undef LOCAL_lockapi_tryacquire
#undef LOCAL_lockapi_acquire
#undef LOCAL_lockapi_timedacquire
#undef LOCAL_lockapi_do_waitfor
#undef LOCAL_lockapi_waitfor
#undef LOCAL_lockapi_timedwaitfor
#undef LOCAL_lockapi_release
#undef LOCAL_lockapi_available_get
#undef LOCAL_lockapi_acquired_get
#undef LOCAL_lockapi_methods
#undef LOCAL_lockapi_getsets
#undef LOCAL_lockapi_members

#undef LOCAL_rwlockapi_ctor
#undef LOCAL_rwlockapi_init_kw
#undef LOCAL_rwlockapi_printrepr
#undef LOCAL_rwlockapi_tryread
#undef LOCAL_rwlockapi_trywrite
#undef LOCAL_rwlockapi_tryupgrade
#undef LOCAL_rwlockapi_endread
#undef LOCAL_rwlockapi_endwrite
#undef LOCAL_rwlockapi_downgrade
#undef LOCAL_rwlockapi_read
#undef LOCAL_rwlockapi_write
#undef LOCAL_rwlockapi_end
#undef LOCAL_rwlockapi_upgrade
#undef LOCAL_rwlockapi_timedread
#undef LOCAL_rwlockapi_timedwrite
#undef LOCAL_rwlockapi_waitread
#undef LOCAL_rwlockapi_waitwrite
#undef LOCAL_rwlockapi_timedwaitread
#undef LOCAL_rwlockapi_timedwaitwrite
#undef LOCAL_rwlockapi_canread_get
#undef LOCAL_rwlockapi_canwrite_get
#undef LOCAL_rwlockapi_reading_get
#undef LOCAL_rwlockapi_writing_get
#undef LOCAL_rwlockapi_readlock_get
#undef LOCAL_rwlockapi_writelock_get
#undef LOCAL_rwlockapi_methods
#undef LOCAL_rwlockapi_getsets
#undef LOCAL_rwlockapi_readlock_init
#undef LOCAL_rwlockapi_writelock_init
#undef LOCAL_rwlockapi_readlock_enter
#undef LOCAL_rwlockapi_readlock_leave
#undef LOCAL_rwlockapi_writelock_enter
#undef LOCAL_rwlockapi_writelock_leave
#undef LOCAL_rwlockapi_readlock_tryacquire
#undef LOCAL_rwlockapi_writelock_tryacquire
#undef LOCAL_rwlockapi_readlock_acquire
#undef LOCAL_rwlockapi_writelock_acquire
#undef LOCAL_rwlockapi_readlock_release
#undef LOCAL_rwlockapi_writelock_release
#undef LOCAL_rwlockapi_readlock_timedacquire
#undef LOCAL_rwlockapi_writelock_timedacquire
#undef LOCAL_rwlockapi_readlock_waitfor
#undef LOCAL_rwlockapi_writelock_waitfor
#undef LOCAL_rwlockapi_readlock_timedwaitfor
#undef LOCAL_rwlockapi_writelock_timedwaitfor
#undef LOCAL_rwlockapi_readlock_available_get
#undef LOCAL_rwlockapi_writelock_available_get
#undef LOCAL_rwlockapi_readlock_acquired_get
#undef LOCAL_rwlockapi_writelock_acquired_get
#undef LOCAL_rwlockapi_readlock_with
#undef LOCAL_rwlockapi_writelock_with
#undef LOCAL_rwlockapi_readlock_methods
#undef LOCAL_rwlockapi_writelock_methods
#undef LOCAL_rwlockapi_readlock_getsets
#undef LOCAL_rwlockapi_writelock_getsets

#undef LOCAL_IS_RECURSIVE
#undef LOCAL_IS_SHARED

DECL_END

#undef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
#undef DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type
#undef DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type
#undef DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type
