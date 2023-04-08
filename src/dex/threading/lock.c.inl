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
#include "lock.c"
#define DEE_SOURCE
//#define DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
//#define DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type
#define DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type
//#define DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/api.h>
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

#include <stdbool.h>
#include <stddef.h>

#include "libthreading.h"

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

#define LOCAL_S_Lock                LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "Lock"
#define LOCAL_S_RWLock              LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "RWLock"
#define LOCAL_S_RWLockSharedLock    "_" LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "RWLockSharedLock"
#define LOCAL_S_RWLockExclusiveLock "_" LOCAL_S_MaybeRecursiveR LOCAL_S_Atomic_or_Shared "RWLockExclusiveLock"

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
#define LOCAL_rwlockapi_func(x)           drshared_rwlock_##x
#define LOCAL_DeeRWLockObject             DeeRSharedRWLockObject
#define LOCAL_DeeRWLock_Type              DeeRSharedRWLock_Type
#define LOCAL_DeeRWLockSharedLock_Type    DeeRSharedRWLockSharedLock_Type
#define LOCAL_DeeRWLockExclusiveLock_Type DeeRSharedRWLockExclusiveLock_Type
#define LOCAL_rwlock_t                    Dee_rshared_rwlock_t
#define LOCAL_rwlock_cinit                Dee_rshared_rwlock_cinit
#define LOCAL_rwlock_init                 Dee_rshared_rwlock_init
#define LOCAL_rwlock_reading              Dee_rshared_rwlock_reading
#define LOCAL_rwlock_writing              Dee_rshared_rwlock_writing
#define LOCAL_rwlock_tryread              Dee_rshared_rwlock_tryread
#define LOCAL_rwlock_trywrite             Dee_rshared_rwlock_trywrite
#define LOCAL_rwlock_canread              Dee_rshared_rwlock_canread
#define LOCAL_rwlock_canwrite             Dee_rshared_rwlock_canwrite
#define LOCAL_rwlock_canendread           Dee_rshared_rwlock_canendread
#define LOCAL_rwlock_canendwrite          Dee_rshared_rwlock_canendwrite
#define LOCAL_rwlock_canend               Dee_rshared_rwlock_canend
#define LOCAL_rwlock_tryupgrade           Dee_rshared_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG    _Dee_rshared_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG     _Dee_rshared_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG      _Dee_rshared_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG          _Dee_rshared_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG   _Dee_rshared_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG       _Dee_rshared_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread             Dee_rshared_rwlock_waitread
#define LOCAL_rwlock_waitwrite            Dee_rshared_rwlock_waitwrite
#define LOCAL_rwlock_waitread_timed       Dee_rshared_rwlock_waitread_timed
#define LOCAL_rwlock_waitwrite_timed      Dee_rshared_rwlock_waitwrite_timed
#define LOCAL_rwlock_read                 Dee_rshared_rwlock_read
#define LOCAL_rwlock_write                Dee_rshared_rwlock_write
#define LOCAL_rwlock_read_timed           Dee_rshared_rwlock_read_timed
#define LOCAL_rwlock_write_timed          Dee_rshared_rwlock_write_timed
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_rwlockapi_func(x)           dshared_rwlock_##x
#define LOCAL_DeeRWLockObject             DeeSharedRWLockObject
#define LOCAL_DeeRWLock_Type              DeeSharedRWLock_Type
#define LOCAL_DeeRWLockSharedLock_Type    DeeSharedRWLockSharedLock_Type
#define LOCAL_DeeRWLockExclusiveLock_Type DeeSharedRWLockExclusiveLock_Type
#define LOCAL_rwlock_t                    Dee_shared_rwlock_t
#define LOCAL_RWLOCK_MAX_READERS          DEE_SHARED_RWLOCK_MAX_READERS
#define LOCAL_rwlock_cinit                Dee_shared_rwlock_cinit
#define LOCAL_rwlock_cinit_read           Dee_shared_rwlock_cinit_read
#define LOCAL_rwlock_cinit_write          Dee_shared_rwlock_cinit_write
#define LOCAL_rwlock_init                 Dee_shared_rwlock_init
#define LOCAL_rwlock_init_read            Dee_shared_rwlock_init_read
#define LOCAL_rwlock_init_write           Dee_shared_rwlock_init_write
#define LOCAL_rwlock_reading              Dee_shared_rwlock_reading
#define LOCAL_rwlock_writing              Dee_shared_rwlock_writing
#define LOCAL_rwlock_tryread              Dee_shared_rwlock_tryread
#define LOCAL_rwlock_trywrite             Dee_shared_rwlock_trywrite
#define LOCAL_rwlock_canread              Dee_shared_rwlock_canread
#define LOCAL_rwlock_canwrite             Dee_shared_rwlock_canwrite
#define LOCAL_rwlock_canendread           Dee_shared_rwlock_canendread
#define LOCAL_rwlock_canendwrite          Dee_shared_rwlock_canendwrite
#define LOCAL_rwlock_canend               Dee_shared_rwlock_canend
#define LOCAL_rwlock_tryupgrade           Dee_shared_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG    _Dee_shared_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG     _Dee_shared_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG      _Dee_shared_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG          _Dee_shared_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG   _Dee_shared_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG       _Dee_shared_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread             Dee_shared_rwlock_waitread
#define LOCAL_rwlock_waitwrite            Dee_shared_rwlock_waitwrite
#define LOCAL_rwlock_waitread_timed       Dee_shared_rwlock_waitread_timed
#define LOCAL_rwlock_waitwrite_timed      Dee_shared_rwlock_waitwrite_timed
#define LOCAL_rwlock_read                 Dee_shared_rwlock_read
#define LOCAL_rwlock_write                Dee_shared_rwlock_write
#define LOCAL_rwlock_read_timed           Dee_shared_rwlock_read_timed
#define LOCAL_rwlock_write_timed          Dee_shared_rwlock_write_timed
#endif /* !LOCAL_IS_RECURSIVE */
#else  /* LOCAL_IS_SHARED */
#ifdef LOCAL_IS_RECURSIVE
#define LOCAL_rwlockapi_func(x)           dratomic_rwlock_##x
#define LOCAL_DeeRWLockObject             DeeRAtomicRWLockObject
#define LOCAL_DeeRWLock_Type              DeeRAtomicRWLock_Type
#define LOCAL_DeeRWLockSharedLock_Type    DeeRAtomicRWLockSharedLock_Type
#define LOCAL_DeeRWLockExclusiveLock_Type DeeRAtomicRWLockExclusiveLock_Type
#define LOCAL_rwlock_t                    Dee_ratomic_rwlock_t
#define LOCAL_rwlock_cinit                Dee_ratomic_rwlock_cinit
#define LOCAL_rwlock_init                 Dee_ratomic_rwlock_init
#define LOCAL_rwlock_reading              Dee_ratomic_rwlock_reading
#define LOCAL_rwlock_writing              Dee_ratomic_rwlock_writing
#define LOCAL_rwlock_tryread              Dee_ratomic_rwlock_tryread
#define LOCAL_rwlock_trywrite             Dee_ratomic_rwlock_trywrite
#define LOCAL_rwlock_canread              Dee_ratomic_rwlock_canread
#define LOCAL_rwlock_canwrite             Dee_ratomic_rwlock_canwrite
#define LOCAL_rwlock_canendread           Dee_ratomic_rwlock_canendread
#define LOCAL_rwlock_canendwrite          Dee_ratomic_rwlock_canendwrite
#define LOCAL_rwlock_canend               Dee_ratomic_rwlock_canend
#define LOCAL_rwlock_tryupgrade           Dee_ratomic_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG    _Dee_ratomic_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG     _Dee_ratomic_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG      _Dee_ratomic_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG          _Dee_ratomic_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG   _Dee_ratomic_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG       _Dee_ratomic_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread_p           Dee_ratomic_rwlock_waitread_p
#define LOCAL_rwlock_waitwrite_p          Dee_ratomic_rwlock_waitwrite_p
#define LOCAL_rwlock_waitread_timed_p     Dee_ratomic_rwlock_waitread_timed_p
#define LOCAL_rwlock_waitwrite_timed_p    Dee_ratomic_rwlock_waitwrite_timed_p
#define LOCAL_rwlock_read_p               Dee_ratomic_rwlock_read_p
#define LOCAL_rwlock_write_p              Dee_ratomic_rwlock_write_p
#define LOCAL_rwlock_read_timed_p         Dee_ratomic_rwlock_read_timed_p
#define LOCAL_rwlock_write_timed_p        Dee_ratomic_rwlock_write_timed_p
#else /* LOCAL_IS_RECURSIVE */
#define LOCAL_rwlockapi_func(x)           datomic_rwlock_##x
#define LOCAL_DeeRWLockObject             DeeAtomicRWLockObject
#define LOCAL_DeeRWLock_Type              DeeAtomicRWLock_Type
#define LOCAL_DeeRWLockSharedLock_Type    DeeAtomicRWLockSharedLock_Type
#define LOCAL_DeeRWLockExclusiveLock_Type DeeAtomicRWLockExclusiveLock_Type
#define LOCAL_rwlock_t                    Dee_atomic_rwlock_t
#define LOCAL_RWLOCK_MAX_READERS          DEE_ATOMIC_RWLOCK_MAX_READERS
#define LOCAL_rwlock_cinit                Dee_atomic_rwlock_cinit
#define LOCAL_rwlock_cinit_read           Dee_atomic_rwlock_cinit_read
#define LOCAL_rwlock_cinit_write          Dee_atomic_rwlock_cinit_write
#define LOCAL_rwlock_init                 Dee_atomic_rwlock_init
#define LOCAL_rwlock_init_read            Dee_atomic_rwlock_init_read
#define LOCAL_rwlock_init_write           Dee_atomic_rwlock_init_write
#define LOCAL_rwlock_reading              Dee_atomic_rwlock_reading
#define LOCAL_rwlock_writing              Dee_atomic_rwlock_writing
#define LOCAL_rwlock_tryread              Dee_atomic_rwlock_tryread
#define LOCAL_rwlock_trywrite             Dee_atomic_rwlock_trywrite
#define LOCAL_rwlock_canread              Dee_atomic_rwlock_canread
#define LOCAL_rwlock_canwrite             Dee_atomic_rwlock_canwrite
#define LOCAL_rwlock_canendread           Dee_atomic_rwlock_canendread
#define LOCAL_rwlock_canendwrite          Dee_atomic_rwlock_canendwrite
#define LOCAL_rwlock_canend               Dee_atomic_rwlock_canend
#define LOCAL_rwlock_tryupgrade           Dee_atomic_rwlock_tryupgrade
#define _LOCAL_rwlock_downgrade_NDEBUG    _Dee_atomic_rwlock_downgrade_NDEBUG
#define _LOCAL_rwlock_endwrite_NDEBUG     _Dee_atomic_rwlock_endwrite_NDEBUG
#define _LOCAL_rwlock_endread_NDEBUG      _Dee_atomic_rwlock_endread_NDEBUG
#define _LOCAL_rwlock_end_NDEBUG          _Dee_atomic_rwlock_end_NDEBUG
#define _LOCAL_rwlock_endread_ex_NDEBUG   _Dee_atomic_rwlock_endread_ex_NDEBUG
#define _LOCAL_rwlock_end_ex_NDEBUG       _Dee_atomic_rwlock_end_ex_NDEBUG
#define LOCAL_rwlock_waitread_p           Dee_atomic_rwlock_waitread_p
#define LOCAL_rwlock_waitwrite_p          Dee_atomic_rwlock_waitwrite_p
#define LOCAL_rwlock_waitread_timed_p     Dee_atomic_rwlock_waitread_timed_p
#define LOCAL_rwlock_waitwrite_timed_p    Dee_atomic_rwlock_waitwrite_timed_p
#define LOCAL_rwlock_read_p               Dee_atomic_rwlock_read_p
#define LOCAL_rwlock_write_p              Dee_atomic_rwlock_write_p
#define LOCAL_rwlock_read_timed_p         Dee_atomic_rwlock_read_timed_p
#define LOCAL_rwlock_write_timed_p        Dee_atomic_rwlock_write_timed_p
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
#define LOCAL_rwlockapi_shared_get              LOCAL_rwlockapi_func(shared_get)
#define LOCAL_rwlockapi_exclusive_get           LOCAL_rwlockapi_func(exclusive_get)
#define LOCAL_rwlockapi_methods                 LOCAL_rwlockapi_func(methods)
#define LOCAL_rwlockapi_getsets                 LOCAL_rwlockapi_func(getsets)
#define LOCAL_rwlockapi_shared_init             LOCAL_rwlockapi_func(shared_init)
#define LOCAL_rwlockapi_exclusive_init          LOCAL_rwlockapi_func(exclusive_init)
#define LOCAL_rwlockapi_shared_enter            LOCAL_rwlockapi_func(shared_enter)
#define LOCAL_rwlockapi_shared_leave            LOCAL_rwlockapi_func(shared_leave)
#define LOCAL_rwlockapi_exclusive_enter         LOCAL_rwlockapi_func(exclusive_enter)
#define LOCAL_rwlockapi_exclusive_leave         LOCAL_rwlockapi_func(exclusive_leave)
#define LOCAL_rwlockapi_shared_tryacquire       LOCAL_rwlockapi_func(shared_tryacquire)
#define LOCAL_rwlockapi_exclusive_tryacquire    LOCAL_rwlockapi_func(exclusive_tryacquire)
#define LOCAL_rwlockapi_shared_acquire          LOCAL_rwlockapi_func(shared_acquire)
#define LOCAL_rwlockapi_exclusive_acquire       LOCAL_rwlockapi_func(exclusive_acquire)
#define LOCAL_rwlockapi_shared_release          LOCAL_rwlockapi_func(shared_release)
#define LOCAL_rwlockapi_exclusive_release       LOCAL_rwlockapi_func(exclusive_release)
#define LOCAL_rwlockapi_shared_timedacquire     LOCAL_rwlockapi_func(shared_timedacquire)
#define LOCAL_rwlockapi_exclusive_timedacquire  LOCAL_rwlockapi_func(exclusive_timedacquire)
#define LOCAL_rwlockapi_shared_waitfor          LOCAL_rwlockapi_func(shared_waitfor)
#define LOCAL_rwlockapi_exclusive_waitfor       LOCAL_rwlockapi_func(exclusive_waitfor)
#define LOCAL_rwlockapi_shared_timedwaitfor     LOCAL_rwlockapi_func(shared_timedwaitfor)
#define LOCAL_rwlockapi_exclusive_timedwaitfor  LOCAL_rwlockapi_func(exclusive_timedwaitfor)
#define LOCAL_rwlockapi_shared_available_get    LOCAL_rwlockapi_func(shared_available_get)
#define LOCAL_rwlockapi_exclusive_available_get LOCAL_rwlockapi_func(exclusive_available_get)
#define LOCAL_rwlockapi_shared_acquired_get     LOCAL_rwlockapi_func(shared_acquired_get)
#define LOCAL_rwlockapi_exclusive_acquired_get  LOCAL_rwlockapi_func(exclusive_acquired_get)
#define LOCAL_rwlockapi_shared_with             LOCAL_rwlockapi_func(shared_with)
#define LOCAL_rwlockapi_exclusive_with          LOCAL_rwlockapi_func(exclusive_with)
#define LOCAL_rwlockapi_shared_methods          LOCAL_rwlockapi_func(shared_methods)
#define LOCAL_rwlockapi_exclusive_methods       LOCAL_rwlockapi_func(exclusive_methods)
#define LOCAL_rwlockapi_shared_getsets          LOCAL_rwlockapi_func(shared_getsets)
#define LOCAL_rwlockapi_exclusive_getsets       LOCAL_rwlockapi_func(exclusive_getsets)


/************************************************************************/
/* Lock                                                                 */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_lockapi_ctor(LOCAL_DeeLockObject *__restrict self) {
	LOCAL_lock_init(&self->l_lock);
	return 0;
}

#ifdef LOCAL_lock_init_acquired
#ifndef DEFINED_lock_init_acquired_kwlist
#define DEFINED_lock_init_acquired_kwlist
PRIVATE DEFINE_KWLIST(lock_init_acquired_kwlist, { K(acquired), KEND });
#endif /* !DEFINED_lock_init_acquired_kwlist */

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_lockapi_init_kw(LOCAL_DeeLockObject *__restrict self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	bool acquired = false;
	if (DeeArg_UnpackKw(argc, argv, kw, lock_init_acquired_kwlist,
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
                        dformatprinter printer, void *arg) {
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
	return err_lock_not_acquired((DeeObject *)self);
}

PRIVATE struct type_with LOCAL_lockapi_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_lockapi_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_lockapi_leave
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_tryacquire(LOCAL_DeeLockObject *self,
                         size_t argc, DeeObject *const *argv) {
	bool result;
	if (DeeArg_Unpack(argc, argv, ":tryacquire"))
		goto err;
	result = LOCAL_lock_tryacquire(&self->l_lock);
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_acquire(LOCAL_DeeLockObject *self,
                      size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":acquire"))
		goto err;
	LOCAL_lock_acquire_p(&self->l_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_timedacquire(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedacquire", &timeout_nanoseconds))
		goto err;
	LOCAL_lock_acquire_timed_p(&self->l_lock, timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_waitfor(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitfor"))
		goto err;
	LOCAL_lock_waitfor_p(&self->l_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_timedwaitfor(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitfor", &timeout_nanoseconds))
		goto err;
	LOCAL_lock_waitfor_timed_p(&self->l_lock, timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_release(LOCAL_DeeLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":release"))
		goto err;
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

PRIVATE struct type_method LOCAL_lockapi_methods[] = {
	TYPE_METHOD(STR_tryacquire, &LOCAL_lockapi_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &LOCAL_lockapi_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &LOCAL_lockapi_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &LOCAL_lockapi_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &LOCAL_lockapi_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &LOCAL_lockapi_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

#ifdef LOCAL_IS_RECURSIVE
#undef LOCAL_lockapi_members
#else /* LOCAL_IS_RECURSIVE */
PRIVATE struct type_member LOCAL_lockapi_members[] = {
#ifdef LOCAL_IS_SHARED
	TYPE_MEMBER_FIELD_DOC(STR_acquired,
	                      STRUCT_CONST | STRUCT_ATOMIC | STRUCT_BOOL(__SIZEOF_ATOMIC_LOCK),
	                      offsetof(LOCAL_DeeLockObject, l_lock.s_lock.a_lock),
	                      DOC_GET(doc_lock_acquired)),
#else /* LOCAL_IS_SHARED */
	TYPE_MEMBER_FIELD_DOC(STR_acquired,
	                      STRUCT_CONST | STRUCT_ATOMIC | STRUCT_BOOL(__SIZEOF_ATOMIC_LOCK),
	                      offsetof(LOCAL_DeeLockObject, l_lock.a_lock),
	                      DOC_GET(doc_lock_acquired)),
#endif /* !LOCAL_IS_SHARED */
	TYPE_MEMBER_END
};
#endif /* !LOCAL_IS_RECURSIVE */

#ifdef LOCAL_lockapi_members
#undef LOCAL_lockapi_acquired_get
#else /* LOCAL_lockapi_members */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_lockapi_acquired_get(LOCAL_DeeLockObject *self) {
	return_bool(LOCAL_lock_acquired(&self->l_lock));
}
#endif /* !LOCAL_lockapi_members */


PRIVATE struct type_getset LOCAL_lockapi_getsets[] = {
	TYPE_GETTER(STR_available, &LOCAL_lockapi_available_get, DOC_GET(doc_lock_available)),
#ifdef LOCAL_lockapi_acquired_get
	TYPE_GETTER(STR_acquired, &LOCAL_lockapi_acquired_get, DOC_GET(doc_lock_acquired)),
#endif /* LOCAL_lockapi_acquired_get */
	TYPE_GETSET_END
};


INTERN DeeTypeObject LOCAL_DeeLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_Lock,
#ifdef LOCAL_lockapi_init_kw
	/* .tp_doc      = */ DOC("(acquired=!f)"),
#else /* LOCAL_lockapi_init_kw */
	/* .tp_doc      = */ NULL,
#endif /* !LOCAL_lockapi_init_kw */
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&LOCAL_lockapi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(LOCAL_DeeLockObject),
#ifdef LOCAL_lockapi_init_kw
				/* .tp_any_ctor_kw = */ (dfunptr_t)&LOCAL_lockapi_init_kw
#else /* LOCAL_lockapi_init_kw */
				/* .tp_any_ctor_kw = */ (dfunptr_t)NULL
#endif /* !LOCAL_lockapi_init_kw */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&LOCAL_lockapi_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_ctor(LOCAL_DeeRWLockObject *__restrict self) {
	LOCAL_rwlock_init(&self->rwl_lock);
	return 0;
}

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
	uintptr_t readers = 0;
	bool writing = false;
	if (DeeArg_UnpackKw(argc, argv, kw, rwlock_init_readers_writing_kwlist,
	                    "|" UNPuPTR "b:" LOCAL_S_RWLock, &readers, &writing))
		goto err;
	if (writing) {
		if unlikely(readers != 0)
			goto err_cannot_initialize_readers_with_writers;

		/* Initialize in write-mode */
		LOCAL_rwlock_init_write(&self->rwl_lock);
	} else {
		if unlikely(readers > LOCAL_RWLOCK_MAX_READERS)
			goto err_readers_counter_is_too_large;
		LOCAL_rwlock_init_read(&self->rwl_lock, readers);
	}
	return 0;
err_cannot_initialize_readers_with_writers:
	return err_rwlock_with_readers_and_writers();
err_readers_counter_is_too_large:
	return err_rwlock_too_many_readers(readers);
err:
	return -1;
}
#endif /* LOCAL_rwlock_init_write && LOCAL_rwlock_init_read */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_rwlockapi_printrepr(LOCAL_DeeRWLockObject *__restrict self,
                          dformatprinter printer, void *arg) {
#ifdef LOCAL_rwlockapi_init_kw
#ifdef LOCAL_IS_SHARED
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_tryread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":tryread"))
		goto err;
	return_bool(LOCAL_rwlock_tryread(&self->rwl_lock));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_trywrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":trywrite"))
		goto err;
	return_bool(LOCAL_rwlock_trywrite(&self->rwl_lock));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_tryupgrade(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":tryupgrade"))
		goto err;
	return_bool(LOCAL_rwlock_tryupgrade(&self->rwl_lock));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_endread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":endread"))
		goto err;
	if unlikely(!LOCAL_rwlock_canendread(&self->rwl_lock))
		goto err_no_read_lock;
	_LOCAL_rwlock_endread_NDEBUG(&self->rwl_lock);
	return_none;
err_no_read_lock:
	err_read_lock_not_acquired((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_endwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":endwrite"))
		goto err;
	if unlikely(!LOCAL_rwlock_canendwrite(&self->rwl_lock))
		goto err_no_write_lock;
	_LOCAL_rwlock_endwrite_NDEBUG(&self->rwl_lock);
	return_none;
err_no_write_lock:
	err_write_lock_not_acquired((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_end(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":end"))
		goto err;
	if unlikely(!LOCAL_rwlock_canend(&self->rwl_lock))
		goto err_nolock;
	_LOCAL_rwlock_end_NDEBUG(&self->rwl_lock);
	return_none;
err_nolock:
	err_read_or_write_lock_not_acquired((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_downgrade(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":downgrade"))
		goto err;
	if unlikely(!LOCAL_rwlock_canendwrite(&self->rwl_lock))
		goto err_no_write_lock;
	_LOCAL_rwlock_downgrade_NDEBUG(&self->rwl_lock);
	return_none;
err_no_write_lock:
	err_write_lock_not_acquired((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_read(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":read"))
		goto err;
	LOCAL_rwlock_read_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_write(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":write"))
		goto err;
	LOCAL_rwlock_write_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_upgrade(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":upgrade"))
		goto err;
	if (LOCAL_rwlock_tryupgrade(&self->rwl_lock))
		return_true;
	if unlikely(!LOCAL_rwlock_canendread(&self->rwl_lock))
		goto err_no_read_lock;
	_LOCAL_rwlock_endread_NDEBUG(&self->rwl_lock);
	LOCAL_rwlock_write_p(&self->rwl_lock, err);
	return_false;
err_no_read_lock:
	err_read_lock_not_acquired((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedread", &timeout_nanoseconds))
		goto err;
	LOCAL_rwlock_read_timed_p(&self->rwl_lock, timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwrite", &timeout_nanoseconds))
		goto err;
	LOCAL_rwlock_write_timed_p(&self->rwl_lock, timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_waitread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitread"))
		goto err;
	LOCAL_rwlock_waitread_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_waitwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitwrite"))
		goto err;
	LOCAL_rwlock_waitwrite_p(&self->rwl_lock, err);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedwaitread(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitread", &timeout_nanoseconds))
		goto err;
	LOCAL_rwlock_waitread_timed_p(&self->rwl_lock, timeout_nanoseconds, err, timeout);
	return_true;
timeout:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_timedwaitwrite(LOCAL_DeeRWLockObject *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitwrite", &timeout_nanoseconds))
		goto err;
	LOCAL_rwlock_waitwrite_timed_p(&self->rwl_lock, timeout_nanoseconds, err, timeout);
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
LOCAL_rwlockapi_shared_get(LOCAL_DeeRWLockObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = (DREF DeeObject *)self;
	Dee_Incref(self);
	DeeObject_Init(result, &LOCAL_DeeRWLockSharedLock_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeGenericRWLockProxyObject *DCALL
LOCAL_rwlockapi_exclusive_get(LOCAL_DeeRWLockObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = (DREF DeeObject *)self;
	Dee_Incref(self);
	DeeObject_Init(result, &LOCAL_DeeRWLockExclusiveLock_Type);
done:
	return result;
}

PRIVATE struct type_method LOCAL_rwlockapi_methods[] = {
	TYPE_METHOD(STR_tryread, &LOCAL_rwlockapi_tryread, DOC_GET(doc_rwlock_tryread)),
	TYPE_METHOD(STR_trywrite, &LOCAL_rwlockapi_trywrite, DOC_GET(doc_rwlock_trywrite)),
	TYPE_METHOD(STR_tryupgrade, &LOCAL_rwlockapi_tryupgrade, DOC_GET(doc_rwlock_tryupgrade)),
	TYPE_METHOD(STR_endread, &LOCAL_rwlockapi_endread, DOC_GET(doc_rwlock_endread)),
	TYPE_METHOD(STR_endwrite, &LOCAL_rwlockapi_endwrite, DOC_GET(doc_rwlock_endwrite)),
	TYPE_METHOD(STR_downgrade, &LOCAL_rwlockapi_downgrade, DOC_GET(doc_rwlock_downgrade)),
	TYPE_METHOD(STR_read, &LOCAL_rwlockapi_read, DOC_GET(doc_rwlock_read)),
	TYPE_METHOD(STR_write, &LOCAL_rwlockapi_write, DOC_GET(doc_rwlock_write)),
	TYPE_METHOD(STR_end, &LOCAL_rwlockapi_end, DOC_GET(doc_rwlock_end)),
	TYPE_METHOD(STR_upgrade, &LOCAL_rwlockapi_upgrade, DOC_GET(doc_rwlock_upgrade)),
	TYPE_METHOD(STR_timedread, &LOCAL_rwlockapi_timedread, DOC_GET(doc_rwlock_timedread)),
	TYPE_METHOD(STR_timedwrite, &LOCAL_rwlockapi_timedwrite, DOC_GET(doc_rwlock_timedwrite)),
	TYPE_METHOD(STR_waitread, &LOCAL_rwlockapi_waitread, DOC_GET(doc_rwlock_waitread)),
	TYPE_METHOD(STR_waitwrite, &LOCAL_rwlockapi_waitwrite, DOC_GET(doc_rwlock_waitwrite)),
	TYPE_METHOD(STR_timedwaitread, &LOCAL_rwlockapi_timedwaitread, DOC_GET(doc_rwlock_timedwaitread)),
	TYPE_METHOD(STR_timedwaitwrite, &LOCAL_rwlockapi_timedwaitwrite, DOC_GET(doc_rwlock_timedwaitwrite)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset LOCAL_rwlockapi_getsets[] = {
	TYPE_GETTER(STR_reading, &LOCAL_rwlockapi_reading_get, DOC_GET(doc_rwlock_reading)),
	TYPE_GETTER(STR_writing, &LOCAL_rwlockapi_writing_get, DOC_GET(doc_rwlock_writing)),
	TYPE_GETTER(STR_canread, &LOCAL_rwlockapi_canread_get, DOC_GET(doc_rwlock_canread)),
	TYPE_GETTER(STR_canwrite, &LOCAL_rwlockapi_canwrite_get, DOC_GET(doc_rwlock_canwrite)),
	TYPE_GETTER(STR_shared, &LOCAL_rwlockapi_shared_get, DOC_GET(doc_rwlock_shared)),
	TYPE_GETTER(STR_exclusive, &LOCAL_rwlockapi_exclusive_get, DOC_GET(doc_rwlock_exclusive)),
	TYPE_GETSET_END
};

INTERN DeeTypeObject LOCAL_DeeRWLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_RWLock,
#ifdef LOCAL_rwlockapi_init_kw
	/* .tp_doc      = */ DOC("(readers=!0,writing=!f)"),
#else /* LOCAL_rwlockapi_init_kw */
	/* .tp_doc      = */ NULL,
#endif /* !LOCAL_rwlockapi_init_kw */
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&LOCAL_rwlockapi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(LOCAL_DeeRWLockObject),
#ifdef LOCAL_rwlockapi_init_kw
				/* .tp_any_ctor_kw = */ (dfunptr_t)&LOCAL_rwlockapi_init_kw
#else /* LOCAL_rwlockapi_init_kw */
				/* .tp_any_ctor_kw = */ (dfunptr_t)NULL
#endif /* !LOCAL_rwlockapi_init_kw */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&LOCAL_rwlockapi_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
LOCAL_rwlockapi_shared_init(DeeGenericRWLockProxyObject *__restrict self,
                            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:" LOCAL_S_RWLockSharedLock, &self->grwl_lock))
		goto err;
	if (DeeObject_AssertType(self->grwl_lock, &LOCAL_DeeRWLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_exclusive_init(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:" LOCAL_S_RWLockExclusiveLock, &self->grwl_lock))
		goto err;
	if (DeeObject_AssertType(self->grwl_lock, &LOCAL_DeeRWLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_shared_enter(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	LOCAL_rwlock_read_p(&rwlock->rwl_lock, err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_shared_leave(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	if unlikely(!LOCAL_rwlock_canendread(&rwlock->rwl_lock))
		goto err_no_read_lock;
	_LOCAL_rwlock_endread_NDEBUG(&rwlock->rwl_lock);
	return 0;
err_no_read_lock:
	return err_read_lock_not_acquired((DeeObject *)rwlock);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_exclusive_enter(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	LOCAL_rwlock_write_p(&rwlock->rwl_lock, err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_rwlockapi_exclusive_leave(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	if unlikely(!LOCAL_rwlock_canendwrite(&rwlock->rwl_lock))
		goto err_no_write_lock;
	_LOCAL_rwlock_endwrite_NDEBUG(&rwlock->rwl_lock);
	return 0;
err_no_write_lock:
	return err_write_lock_not_acquired((DeeObject *)rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_tryacquire(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_tryread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_tryacquire(DeeGenericRWLockProxyObject *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_trywrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_acquire(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_read(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_acquire(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_write(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_release(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_endread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_release(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_endwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_timedacquire(DeeGenericRWLockProxyObject *__restrict self,
                                    size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_timedacquire(DeeGenericRWLockProxyObject *__restrict self,
                                       size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_waitfor(DeeGenericRWLockProxyObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_waitread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_waitfor(DeeGenericRWLockProxyObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_waitwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_timedwaitfor(DeeGenericRWLockProxyObject *__restrict self,
                                    size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedwaitread(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_timedwaitfor(DeeGenericRWLockProxyObject *__restrict self,
                                       size_t argc, DeeObject *const *argv) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_timedwaitwrite(rwlock, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_canread_get(rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_canwrite_get(rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_shared_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_reading_get(rwlock);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_rwlockapi_exclusive_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	LOCAL_DeeRWLockObject *rwlock = (LOCAL_DeeRWLockObject *)self->grwl_lock;
	return LOCAL_rwlockapi_writing_get(rwlock);
}

PRIVATE struct type_with LOCAL_rwlockapi_shared_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_shared_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_shared_leave
};

PRIVATE struct type_with LOCAL_rwlockapi_exclusive_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_exclusive_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_rwlockapi_exclusive_leave
};

PRIVATE struct type_method LOCAL_rwlockapi_shared_methods[] = {
	TYPE_METHOD(STR_tryacquire, &LOCAL_rwlockapi_shared_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &LOCAL_rwlockapi_shared_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &LOCAL_rwlockapi_shared_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &LOCAL_rwlockapi_shared_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &LOCAL_rwlockapi_shared_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &LOCAL_rwlockapi_shared_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_method LOCAL_rwlockapi_exclusive_methods[] = {
	TYPE_METHOD(STR_tryacquire, &LOCAL_rwlockapi_exclusive_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &LOCAL_rwlockapi_exclusive_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &LOCAL_rwlockapi_exclusive_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &LOCAL_rwlockapi_exclusive_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &LOCAL_rwlockapi_exclusive_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &LOCAL_rwlockapi_exclusive_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset LOCAL_rwlockapi_shared_getsets[] = {
	TYPE_GETTER(STR_available, &LOCAL_rwlockapi_shared_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &LOCAL_rwlockapi_shared_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

PRIVATE struct type_getset LOCAL_rwlockapi_exclusive_getsets[] = {
	TYPE_GETTER(STR_available, &LOCAL_rwlockapi_exclusive_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &LOCAL_rwlockapi_exclusive_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

INTERN DeeTypeObject LOCAL_DeeRWLockSharedLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_RWLockSharedLock,
	/* .tp_doc      = */ DOC("(lock:?G" LOCAL_S_RWLock ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeRWLockSharedLock_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&LOCAL_rwlockapi_shared_init,
				TYPE_FIXED_ALLOCATOR(DeeGenericRWLockProxyObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &LOCAL_rwlockapi_shared_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ LOCAL_rwlockapi_shared_methods,
	/* .tp_getsets       = */ LOCAL_rwlockapi_shared_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
INTERN DeeTypeObject LOCAL_DeeRWLockExclusiveLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ LOCAL_S_RWLockExclusiveLock,
	/* .tp_doc      = */ DOC("(lock:?G" LOCAL_S_RWLock ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeRWLockExclusiveLock_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&LOCAL_rwlockapi_exclusive_init,
				TYPE_FIXED_ALLOCATOR(DeeGenericRWLockProxyObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &LOCAL_rwlockapi_exclusive_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ LOCAL_rwlockapi_exclusive_methods,
	/* .tp_getsets       = */ LOCAL_rwlockapi_exclusive_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


#undef LOCAL_S_Atomic_or_Shared
#undef LOCAL_S_MaybeRecursiveR
#undef LOCAL_S_Lock
#undef LOCAL_S_RWLock
#undef LOCAL_S_RWLockSharedLock
#undef LOCAL_S_RWLockExclusiveLock

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
#undef LOCAL_DeeRWLockSharedLock_Type
#undef LOCAL_DeeRWLockExclusiveLock_Type
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
#undef LOCAL_rwlockapi_shared_get
#undef LOCAL_rwlockapi_exclusive_get
#undef LOCAL_rwlockapi_methods
#undef LOCAL_rwlockapi_getsets
#undef LOCAL_rwlockapi_shared_init
#undef LOCAL_rwlockapi_exclusive_init
#undef LOCAL_rwlockapi_shared_enter
#undef LOCAL_rwlockapi_shared_leave
#undef LOCAL_rwlockapi_exclusive_enter
#undef LOCAL_rwlockapi_exclusive_leave
#undef LOCAL_rwlockapi_shared_tryacquire
#undef LOCAL_rwlockapi_exclusive_tryacquire
#undef LOCAL_rwlockapi_shared_acquire
#undef LOCAL_rwlockapi_exclusive_acquire
#undef LOCAL_rwlockapi_shared_release
#undef LOCAL_rwlockapi_exclusive_release
#undef LOCAL_rwlockapi_shared_timedacquire
#undef LOCAL_rwlockapi_exclusive_timedacquire
#undef LOCAL_rwlockapi_shared_waitfor
#undef LOCAL_rwlockapi_exclusive_waitfor
#undef LOCAL_rwlockapi_shared_timedwaitfor
#undef LOCAL_rwlockapi_exclusive_timedwaitfor
#undef LOCAL_rwlockapi_shared_available_get
#undef LOCAL_rwlockapi_exclusive_available_get
#undef LOCAL_rwlockapi_shared_acquired_get
#undef LOCAL_rwlockapi_exclusive_acquired_get
#undef LOCAL_rwlockapi_shared_with
#undef LOCAL_rwlockapi_exclusive_with
#undef LOCAL_rwlockapi_shared_methods
#undef LOCAL_rwlockapi_exclusive_methods
#undef LOCAL_rwlockapi_shared_getsets
#undef LOCAL_rwlockapi_exclusive_getsets

#undef LOCAL_IS_RECURSIVE
#undef LOCAL_IS_SHARED

DECL_END

#undef DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
#undef DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type
#undef DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type
#undef DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type
