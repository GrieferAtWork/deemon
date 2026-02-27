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
/*!export **/
/*!export DeeRCU_**/
/*!export Dee_RCU_**/
/*!export Dee_rcu_**/
#ifndef GUARD_DEEMON_UTIL_RCU_H
#define GUARD_DEEMON_UTIL_RCU_H 1 /*!export-*/

#include "../api.h"

#include <hybrid/__atomic.h> /* __ATOMIC_ACQUIRE, __ATOMIC_RELEASE, __hybrid_atomic_load, __hybrid_atomic_store */
#include <hybrid/typecore.h> /* __SIZEOF_POINTER__, __UINTPTR_C */

#include "../thread.h" /* CONFIG_PER_OBJECT_RCU_LOCKS, DeeThreadObject, DeeThread_Self, Dee_THREAD_RCU_INACTIVE, Dee_thread_rcuvers_t */

#include <stddef.h> /* NULL */

DECL_BEGIN

/*
 * RCU usage:
 *
 * >> DREF DeeObject *getref() {
 * >>     DREF DeeObject *result;
 * >>     DeeRCU_LockDefault();
 * >>     result = atomic_read(&global_ref);
 * >>     Dee_Incref(result);
 * >>     DeeRCU_UnlockDefault();
 * >>     return result;
 * >> }
 * >>
 * >> void setref(DREF DeeObject *ob) {
 * >>     DREF DeeObject *old;
 * >>     old = atomic_xch(&global_ref, ob);
 * >>     DeeRCU_SynchronizeDefault();
 * >>     Dee_Decref(old);
 * >> }
 *
 *
 * NOTES:
 * - It's basically the same as the idea of an "in-use" counter to
 *   prevent premature destruction of references, only that instead
 *   of having a global (and thus prone to cache conflicts) counter
 *   of in-use threads, every reader thread has only a single thread-
 *   local "rcu version" field.
 * - As such, if you already understood the idea of in-use counters:
 *   - "atomic_inc(&in_use);"                   -- Replace with "DeeRCU_LockDefault()"
 *   - "atomic_dec(&in_use);"                   -- Replace with "DeeRCU_UnlockDefault()"
 *   - "while (atomic_read(&in_use)) yield();"  -- Replace with "DeeRCU_SynchronizeDefault()"
 */

/* XXX: Have a configuration where "DeeRCU_LockDefault()" (and the other methods) all take an
 *      additional parameter "struct Dee_rculock" that encapsulates what is currently
 *      done by `_DeeRCU_Default' -- in that version, every RCU use-case must then
 *      supply its own "struct Dee_rculock" (though there can also still be some
 *      default, global RCU lock, too) */


/* Enter/leave an RCU section (where references
 * read are always either the old, or new state) */
DFUNDEF void (DCALL DeeRCU_LockDefault)(void);
DFUNDEF void (DCALL DeeRCU_UnlockDefault)(void);

/* Synchronize RCU, blocking until all threads that
 * locked an older RCU version will have left their
 * RCU section (by calling `DeeRCU_UnlockDefault()')
 *
 * This function must be called before the old state
 * of some variable protected by RCU may be destroyed */
DFUNDEF void (DCALL DeeRCU_SynchronizeDefault)(void);


#if __SIZEOF_POINTER__ == 4
#define _DeeRCU_INVALID_LOCK ((struct Dee_rcu_lock *)__UINTPTR_C(0xcccccccc))
#elif __SIZEOF_POINTER__ == 8
#define _DeeRCU_INVALID_LOCK ((struct Dee_rcu_lock *)__UINTPTR_C(0xcccccccccccccccc))
#elif __SIZEOF_POINTER__ == 2
#define _DeeRCU_INVALID_LOCK ((struct Dee_rcu_lock *)__UINTPTR_C(0xcccc))
#elif __SIZEOF_POINTER__ == 1
#define _DeeRCU_INVALID_LOCK ((struct Dee_rcu_lock *)__UINTPTR_C(0xcc))
#else /* __SIZEOF_POINTER__ == ... */
#define _DeeRCU_INVALID_LOCK ((struct Dee_rcu_lock *)NULL)
#endif /* __SIZEOF_POINTER__ != ... */

/* Support for per-object RCU locks. */
#ifdef CONFIG_PER_OBJECT_RCU_LOCKS
typedef struct Dee_rcu_lock {
	Dee_thread_rcuvers_t rcul_version; /* [lock(ATOMIC)] RCU "version" number */
} Dee_rcu_lock_t;

/* [lock(ATOMIC)] Global RCU "version" number (only here for reading;
 * only `DeeRCU_SynchronizeDefault()' is allowed to write this!) */
DDATDEF Dee_rcu_lock_t _DeeRCU_Default;

#define Dee_RCU_LOCK_INIT        {1}
#define Dee_RCU_LOCK__INIT       ,{1}
#define Dee_RCU_LOCK_INIT_       {1},
#define Dee_rcu_lock_init(self)  (void)((self)->rcul_version = 1)
#define Dee_rcu_lock_cinit(self) (void)((self)->rcul_version = 1)
#define Dee_RCU_LOCK_DECLARE(n)  Dee_rcu_lock_t n;

#else /* CONFIG_PER_OBJECT_RCU_LOCKS */
typedef void Dee_rcu_lock_t;

#ifndef CONFIG_NO_THREADS
/* [lock(ATOMIC)] Global RCU "version" number (only here for reading;
 * only `DeeRCU_SynchronizeDefault()' is allowed to write this!) */
DDATDEF struct Dee_rcu_lock {
	Dee_thread_rcuvers_t rcul_version;
} _DeeRCU_Default;
#endif /* !CONFIG_NO_THREADS */

#define Dee_RCU_LOCK_INIT        /* nothing */
#define Dee_RCU_LOCK__INIT       /* nothing */
#define Dee_RCU_LOCK_INIT_       /* nothing */
#define Dee_rcu_lock_init(self)  (void)0
#define Dee_rcu_lock_cinit(self) (void)0
#define Dee_RCU_LOCK_DECLARE(n)  /* nothing */

#define DeeRCU_Lock(self)        DeeRCU_LockDefault()
#define DeeRCU_Unlock(self)      DeeRCU_UnlockDefault()
#define DeeRCU_Synchronize(self) DeeRCU_SynchronizeDefault()
#endif /* !CONFIG_PER_OBJECT_RCU_LOCKS */

DFUNDEF NONNULL((1)) void (DFCALL DeeRCU_Lock)(Dee_rcu_lock_t *__restrict self);
DFUNDEF NONNULL((1)) void (DFCALL DeeRCU_Unlock)(Dee_rcu_lock_t *__restrict self);
DFUNDEF NONNULL((1)) void (DFCALL DeeRCU_Synchronize)(Dee_rcu_lock_t *__restrict self);



#ifdef CONFIG_NO_THREADS
#undef DeeRCU_Lock
#undef DeeRCU_Unlock
#undef DeeRCU_Synchronize
#undef DeeRCU_LockDefault
#undef DeeRCU_UnlockDefault
#undef DeeRCU_SynchronizeDefault
#define DeeRCU_Lock(self)           (void)0
#define DeeRCU_Unlock(self)         (void)0
#define DeeRCU_Synchronize(self)    (void)0
#define DeeRCU_LockDefault()        (void)0
#define DeeRCU_UnlockDefault()      (void)0
#define DeeRCU_SynchronizeDefault() (void)0
#elif !defined(__OPTIMIZE_SIZE__)
#ifdef CONFIG_PER_OBJECT_RCU_LOCKS
#define DeeRCU_LockSelf(self, caller)                                      \
	(__hybrid_atomic_store(&(caller)->t_rcu_lock, self, __ATOMIC_RELEASE), \
	 __hybrid_atomic_store(&(caller)->t_rcu_vers, __hybrid_atomic_load(&(self)->rcul_version, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE))
#ifndef NDEBUG
#define DeeRCU_UnlockSelf(self, caller)                                                             \
	(void)(__hybrid_atomic_store(&(caller)->t_rcu_vers, Dee_THREAD_RCU_INACTIVE, __ATOMIC_RELEASE), \
	       (caller)->t_rcu_lock = _DeeRCU_INVALID_LOCK)
#endif /* !NDEBUG */
#else /* CONFIG_PER_OBJECT_RCU_LOCKS */
#define DeeRCU_LockSelf(self, caller) \
	__hybrid_atomic_store(&(caller)->t_rcu_vers, __hybrid_atomic_load(&(self)->rcul_version, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE)
#endif /* !CONFIG_PER_OBJECT_RCU_LOCKS */
#undef DeeRCU_Unlock
#ifndef DeeRCU_UnlockSelf
#define DeeRCU_UnlockSelf(self, caller) \
	__hybrid_atomic_store(&(caller)->t_rcu_vers, Dee_THREAD_RCU_INACTIVE, __ATOMIC_RELEASE)
#define DeeRCU_Unlock(self) DeeRCU_UnlockSelf(self, DeeThread_Self())
#endif /* !DeeRCU_UnlockSelf */

#undef DeeRCU_Lock
#ifdef CONFIG_PER_OBJECT_RCU_LOCKS
#define DeeRCU_Lock(self)                                         \
	do {                                                          \
		DeeThreadObject *const _drculd_caller = DeeThread_Self(); \
		DeeRCU_LockSelf(self, _drculd_caller);                    \
	}	__WHILE0
#ifndef DeeRCU_Unlock
#define DeeRCU_Unlock(self)                                       \
	do {                                                          \
		DeeThreadObject *const _drcuud_caller = DeeThread_Self(); \
		DeeRCU_UnlockSelf(self, _drcuud_caller);                  \
	}	__WHILE0
#endif /* !DeeRCU_Unlock */
#else /* CONFIG_PER_OBJECT_RCU_LOCKS */
#define DeeRCU_Lock(self) DeeRCU_LockSelf(self, DeeThread_Self())
#ifndef DeeRCU_Unlock
#define DeeRCU_Unlock(self) DeeRCU_UnlockSelf(self, DeeThread_Self())
#endif /* !DeeRCU_Unlock */
#endif /* !CONFIG_PER_OBJECT_RCU_LOCKS */

/* "fast" RCU locking macros (what makes these "fast" is just that they pre-cache the calling thread) */
#define DeeRCU_FAST_SETUP DeeThreadObject *const _rcu_fast_caller = DeeThread_Self();
#if defined(__NO_builtin_assume) || defined(__builtin_assume_has_sideeffects)
#define DeeRCU_FAST_Lock(self)   DeeRCU_LockSelf(self, _rcu_fast_caller)
#define DeeRCU_FAST_Unlock(self) DeeRCU_UnlockSelf(self, _rcu_fast_caller)
#else /* __NO_builtin_assume || __builtin_assume_has_sideeffects */
#define DeeRCU_FAST_Lock(self)   (__builtin_assume(_rcu_fast_caller == DeeThread_Self()), DeeRCU_LockSelf(self, _rcu_fast_caller))
#define DeeRCU_FAST_Unlock(self) (__builtin_assume(_rcu_fast_caller == DeeThread_Self()), DeeRCU_UnlockSelf(self, _rcu_fast_caller))
#endif /* !__NO_builtin_assume && !__builtin_assume_has_sideeffects */

#undef DeeRCU_LockDefault
#undef DeeRCU_UnlockDefault
#define DeeRCU_LockDefault()             DeeRCU_Lock(&_DeeRCU_Default)
#define DeeRCU_UnlockDefault()           DeeRCU_Unlock(&_DeeRCU_Default)
#define DeeRCU_LockDefaultSelf(caller)   DeeRCU_LockSelf(&_DeeRCU_Default, caller)
#define DeeRCU_UnlockDefaultSelf(caller) DeeRCU_UnlockSelf(&_DeeRCU_Default, caller)
#define DeeRCU_FAST_LockDefault()        DeeRCU_FAST_Lock(&_DeeRCU_Default)
#define DeeRCU_FAST_UnlockDefault()      DeeRCU_FAST_Unlock(&_DeeRCU_Default)
#endif /* !CONFIG_NO_THREADS && !__OPTIMIZE_SIZE__ */


#ifndef DeeRCU_LockSelf
#define DeeRCU_LockSelf(self, caller)    DeeRCU_Lock(self)
#define DeeRCU_UnlockSelf(self, caller)  DeeRCU_Unlock(self)
#define DeeRCU_LockDefaultSelf(caller)   DeeRCU_LockDefault()
#define DeeRCU_UnlockDefaultSelf(caller) DeeRCU_UnlockDefault()
#endif /* !DeeRCU_LockSelf */

#ifndef DeeRCU_FAST_SETUP
#define DeeRCU_FAST_SETUP           /* nothing */
#define DeeRCU_FAST_Lock(self)      DeeRCU_Lock(self)
#define DeeRCU_FAST_Unlock(self)    DeeRCU_Unlock(self)
#define DeeRCU_FAST_LockDefault()   DeeRCU_LockDefault()
#define DeeRCU_FAST_UnlockDefault() DeeRCU_UnlockDefault()
#endif /* !DeeRCU_FAST_SETUP */

DECL_END

#endif /* !GUARD_DEEMON_UTIL_RCU_H */
