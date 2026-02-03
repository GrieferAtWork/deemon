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
#ifndef GUARD_DEEMON_SYSTEM_FUTEX_C
#define GUARD_DEEMON_SYSTEM_FUTEX_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_* */
#include <deemon/object.h>          /* DREF */
#include <deemon/system-features.h> /* CONFIG_HAVE_*, bzero, cnd_broadcast, futex_timedwaitwhile, futex_timedwaitwhile64, futex_waitwhile, futex_wake, futex_wakeall, mtx_lock, mtx_unlock, pthread_cond_broadcast, pthread_mutex_lock, pthread_mutex_unlock, sem_destroy, sem_post, sem_post_multiple, syscall, thrd_success */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_Self */
#include <deemon/util/atomic.h>     /* Dee_ATOMIC_RELAXED, Dee_ATOMIC_SEQ_CST, atomic_* */
#include <deemon/util/lock.h>       /* Dee_atomic_lock_*, Dee_atomic_rwlock_* */

#include <hybrid/sched/atomic-once.h> /* atomic_once */
#include <hybrid/sched/yield.h>       /* SCHED_YIELD */
#include <hybrid/sequence/list.h>     /* LIST_*, SLIST_* */
#include <hybrid/typecore.h>          /* __SIZEOF_INT__, __SIZEOF_POINTER__, __ULONGPTR_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* UINT32_C, uint32_t, uint64_t, uintptr_t */

#ifdef CONFIG_HAVE_LINUX_FUTEX_H
#include <linux/futex.h>
#endif /* CONFIG_HAVE_LINUX_FUTEX_H */

#ifdef CONFIG_HAVE_KOS_FUTEX_H
#include <kos/futex.h>
#endif /* CONFIG_HAVE_KOS_FUTEX_H */

#ifdef CONFIG_HAVE_TIME_H
#include <time.h>
#endif /* CONFIG_HAVE_TIME_H */

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#ifdef CONFIG_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* CONFIG_HAVE_SYS_TIME_H */

#if !defined(SYS_futex) && defined(__NR_futex)
#define SYS_futex __NR_futex
#endif /* !SYS_futex && __NR_futex */

#if !defined(SYS_lfutex) && defined(__NR_lfutex)
#define SYS_lfutex __NR_lfutex
#endif /* !SYS_lfutex && __NR_lfutex */

#undef linux_futex
#if defined(CONFIG_HAVE_syscall) && defined(SYS_futex)
#define linux_futex(uaddr, futex_op, val, timeout_or_val2, uaddr2, val3) \
	syscall(SYS_futex, uaddr, futex_op, val, timeout_or_val2, uaddr2, val3)
#endif /* ... */

#undef os_futex_wakeone
#if defined(CONFIG_HAVE_futex_wake)
#define os_futex_wakeone(addr) futex_wake((lfutex_t *)(addr), 1)
#elif defined(linux_futex) && defined(FUTEX_WAKE)
#define os_futex_wakeone(addr) linux_futex(addr, FUTEX_WAKE, 1, NULL, NULL, 0)
#endif /* ... */

#undef os_futex_wakeall
#if defined(CONFIG_HAVE_futex_wakeall)
#define os_futex_wakeall(addr) futex_wakeall((lfutex_t *)(addr))
#elif defined(linux_futex) && defined(FUTEX_WAKE)
#define os_futex_wakeall(addr) linux_futex(addr, FUTEX_WAKE, (uint32_t)-1, NULL, NULL, 0)
#endif /* ... */

#if (__SIZEOF_POINTER__ == 4 && defined(CONFIG_HAVE_futex_waitwhile) && \
     (defined(CONFIG_HAVE_futex_timedwaitwhile64) || defined(CONFIG_HAVE_futex_timedwaitwhile)))
#define os_futex_wait32(uaddr, expected) futex_waitwhile((lfutex_t *)(uaddr), expected)
#define os_futex_wait32_timed            os_futex_wait32_timed
LOCAL int DCALL os_futex_wait32_timed(void *uaddr, uint32_t expected,
                                      uint64_t timeout_nanoseconds) {
#ifdef CONFIG_HAVE_futex_timedwaitwhile64
	struct timespec64 ts;
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return futex_timedwaitwhile64(uaddr, expected, &ts);
#else /* CONFIG_HAVE_futex_timedwaitwhile64 */
	struct timespec ts;
	if (sizeof(ts.tv_sec) < 4 && timeout_nanoseconds == (uint64_t)-1)
		return os_futex_wait32(uaddr, expected);
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return futex_timedwaitwhile(uaddr, expected, &ts);
#endif /* !CONFIG_HAVE_futex_timedwaitwhile64 */
}
#elif defined(linux_futex) && defined(FUTEX_WAIT) && __SIZEOF_INT__ == 4
#define os_futex_wait32(uaddr, expected) \
	linux_futex(uaddr, FUTEX_WAIT, expected, NULL, NULL, 0)
#ifndef __syscall_ulong_t
#define __syscall_ulong_t __ULONGPTR_TYPE__
#endif /* !__syscall_ulong_t */
#ifndef __time32_t
#define __time32_t __syscall_ulong_t
#endif /* !__time32_t */
struct _dee_timespec32 {
	__time32_t        tv_sec;  /* Seconds */
	__syscall_ulong_t tv_nsec; /* Nanoseconds */
};
#define os_futex_wait32_timed os_futex_wait32_timed
LOCAL int DCALL os_futex_wait32_timed(void *uaddr, uint32_t expected,
                                      uint64_t timeout_nanoseconds) {
	struct _dee_timespec32 ts;
	if (sizeof(ts.tv_sec) < 4 && timeout_nanoseconds == (uint64_t)-1)
		return os_futex_wait32(uaddr, expected);
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return linux_futex(uaddr, FUTEX_WAIT, expected, &ts, NULL, 0);
}
#endif /* ... */

#if (__SIZEOF_POINTER__ == 8 && defined(CONFIG_HAVE_futex_waitwhile) && \
     (defined(CONFIG_HAVE_futex_timedwaitwhile64) || defined(CONFIG_HAVE_futex_timedwaitwhile)))
#define os_futex_wait64(uaddr, expected) futex_waitwhile((lfutex_t *)(uaddr), expected)
#define os_futex_wait64_timed            os_futex_wait64_timed
LOCAL int DCALL os_futex_wait64_timed(void *uaddr, uint64_t expected,
                                      uint64_t timeout_nanoseconds) {
#ifdef CONFIG_HAVE_futex_timedwaitwhile64
	struct timespec64 ts;
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return futex_timedwaitwhile64(uaddr, expected, &ts);
#else /* CONFIG_HAVE_futex_timedwaitwhile64 */
	struct timespec ts;
	if (sizeof(ts.tv_sec) < 4 && timeout_nanoseconds == (uint64_t)-1)
		return os_futex_wait64(uaddr, expected);
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return futex_timedwaitwhile(uaddr, expected, &ts);
#endif /* !CONFIG_HAVE_futex_timedwaitwhile64 */
}
#endif /* ... */


/************************************************************************/
/* Check for `cnd_t' support                                            */
/************************************************************************/
#ifndef CONFIG_HAVE_cnd_init
#undef cnd_init
#define cnd_init(self) (bzero(self, sizeof(pthread_cond_t)), thrd_success)
#endif /* !CONFIG_HAVE_cnd_init */
#ifndef CONFIG_HAVE_cnd_destroy
#undef cnd_destroy
#define cnd_destroy(self) (void)0
#endif /* !CONFIG_HAVE_cnd_destroy */
#undef CONFIG_HAVE_cnd_t
#if (defined(CONFIG_HAVE_cnd_signal) &&                                                \
     defined(CONFIG_HAVE_cnd_broadcast) &&                                             \
     defined(CONFIG_HAVE_cnd_wait) &&                                                  \
     ((defined(CONFIG_HAVE_cnd_timedwait64) && defined(CONFIG_HAVE_gettimeofday64)) || \
      (defined(CONFIG_HAVE_cnd_timedwait) && defined(CONFIG_HAVE_gettimeofday)) ||     \
      defined(CONFIG_HAVE_cnd_reltimedwait_np) || \
      defined(CONFIG_HAVE_cnd_reltimedwait64_np)))
#define CONFIG_HAVE_cnd_t
#endif /* ... */


/************************************************************************/
/* Check for `mtx_t' support                                            */
/************************************************************************/
#ifndef CONFIG_HAVE_mtx_init
#undef mtx_init
#define mtx_init(self, typ) (bzero(self, sizeof(mtx_t)), thrd_success)
#endif /* !CONFIG_HAVE_mtx_init */
#ifndef CONFIG_HAVE_mtx_destroy
#undef mtx_destroy
#define mtx_destroy(self) (void)0
#endif /* !CONFIG_HAVE_mtx_destroy */
#undef CONFIG_HAVE_mtx_t
#if (defined(CONFIG_HAVE_mtx_lock) && \
     defined(CONFIG_HAVE_mtx_unlock))
#define CONFIG_HAVE_mtx_t
#endif /* ... */


/************************************************************************/
/* Check for `pthread_cond_t' support                                   */
/************************************************************************/
#ifndef CONFIG_HAVE_pthread_cond_init
#undef pthread_cond_init
#define pthread_cond_init(self, at) (bzero(self, sizeof(pthread_cond_t)), 0)
#endif /* !CONFIG_HAVE_pthread_cond_init */
#ifndef CONFIG_HAVE_pthread_cond_destroy
#undef pthread_cond_destroy
#define pthread_cond_destroy(self) (void)0
#endif /* !CONFIG_HAVE_pthread_cond_destroy */
#undef CONFIG_HAVE_pthread_cond_t
#if (defined(CONFIG_HAVE_pthread_cond_signal) &&                                                \
     defined(CONFIG_HAVE_pthread_cond_broadcast) &&                                             \
     defined(CONFIG_HAVE_pthread_cond_wait) &&                                                  \
     ((defined(CONFIG_HAVE_pthread_cond_timedwait) && defined(CONFIG_HAVE_gettimeofday)) ||     \
      (defined(CONFIG_HAVE_pthread_cond_timedwait64) && defined(CONFIG_HAVE_gettimeofday64)) || \
      defined(CONFIG_HAVE_pthread_cond_reltimedwait_np) ||                                      \
      defined(CONFIG_HAVE_pthread_cond_reltimedwait64_np)))
#define CONFIG_HAVE_pthread_cond_t
#endif /* ... */


/************************************************************************/
/* Check for `pthread_mutex_t' support                                  */
/************************************************************************/
#ifndef CONFIG_HAVE_pthread_mutex_init
#undef pthread_mutex_init
#define pthread_mutex_init(self, at) (bzero(self, sizeof(pthread_mutex_t)), 0)
#endif /* !CONFIG_HAVE_pthread_mutex_init */
#ifndef CONFIG_HAVE_pthread_mutex_destroy
#undef pthread_mutex_destroy
#define pthread_mutex_destroy(self) (void)0
#endif /* !CONFIG_HAVE_pthread_mutex_destroy */
#undef CONFIG_HAVE_pthread_mutex_t
#if (defined(CONFIG_HAVE_pthread_mutex_lock) && \
     defined(CONFIG_HAVE_pthread_mutex_unlock))
#define CONFIG_HAVE_pthread_mutex_t
#endif /* ... */


/************************************************************************/
/* Check for `sem_t' support                                            */
/************************************************************************/
#undef CONFIG_HAVE_sem_t
#if (defined(CONFIG_HAVE_SEMAPHORE_H) && defined(CONFIG_HAVE_sem_init) &&              \
     defined(CONFIG_HAVE_sem_wait) && defined(CONFIG_HAVE_sem_post) &&                 \
     ((defined(CONFIG_HAVE_sem_timedwait) && defined(CONFIG_HAVE_gettimeofday)) ||     \
      (defined(CONFIG_HAVE_sem_timedwait64) && defined(CONFIG_HAVE_gettimeofday64)) || \
      defined(CONFIG_HAVE_sem_reltimedwait_np) || defined(CONFIG_HAVE_sem_reltimedwait64_np)))
#define CONFIG_HAVE_sem_t
#endif /* ... */



/************************************************************************/
/* MASTER CONTROL FOR HOW FUTEXES ARE IMPLEMENTED                       */
/************************************************************************/
/* Figure out how we want to implement the deemon Futex API.
 *
 * NOTE: All implementations except for `DeeFutex_USE_os_futex'
 *       use dynamically allocated structures and a binary tree
 *       to translate wake/wait-addresses into those structures. */
#undef DeeFutex_USE_os_futex
#undef DeeFutex_USE_os_futex_32_only
#undef DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#undef DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t
#undef DeeFutex_USE_cnd_t_AND_mtx_t
#undef DeeFutex_USE_sem_t
#undef DeeFutex_USE_yield
/* XXX: DeeFutex_USE_cxx_atomic_wait -- But only if "std::atomic<uint(32|64)_t>" is a
 *      wrapper around "uint(32|64)_t" -- iow: can't be used if futex data is stored
 *      in-band alongside the actual atomic word. -- C++ allows something like:
 * >> template<class T> class atomic {
 * >> private:
 * >>     T                      m_value;
 * >>     MAGIC_OS_FUTEX_CONTEXT m_futex; // Ooops :(
 * >> };
 *
 * Or even this, which we can't even detect with "sizeof(std::atomic<T>) != sizeof(T)"
 * >> template<class T> class atomic;
 * >> template<> class atomic<uint32_t> {
 * >> private:
 * >>     uint16_t m_value_index; // Index into some magic table to store atomic value
 * >>     uint16_t m_futex_index; // again: futex is somehow part of atomic<T>
 * >> };
 */
/*#undef DeeFutex_USE_cxx_atomic_wait*/
#undef DeeFutex_USE_STUB
#ifdef CONFIG_NO_THREADS
#define DeeFutex_USE_STUB
#elif (defined(os_futex_wakeone) && defined(os_futex_wakeall) &&     \
       defined(os_futex_wait32) && defined(os_futex_wait32_timed) && \
       (__SIZEOF_POINTER__ < 8 || (defined(os_futex_wait64) && defined(os_futex_wait64_timed))))
/* Use linux/kos futex APIs */
#define DeeFutex_USE_os_futex
#elif (defined(os_futex_wakeone) && defined(os_futex_wakeall) && \
       defined(os_futex_wait32) && defined(os_futex_wait32_timed))
/* Only use linux-compatible futex APIs (in order to facilitate 64-bit futex operations,
 * we pipe all futex requests through an indirect table of futex status words, where we
 * then increment those status words during wake-up):
 * >> wait:
 * >>     status = atomic_read(&STATUS);
 * >>     if (!SHOULD_WAIT())
 * >>         return;
 * >>     WAIT_WHILE(&STATUS, status);
 * >>
 * >> wake:
 * >>     atomic_inc(&STATUS);
 * >>     WAKE(&STATUS); */
#define DeeFutex_USE_os_futex_32_only
#elif defined(CONFIG_HOST_WINDOWS)
/* Windows 8+:     WaitOnAddress is pretty much the same as linux's `sys_futex(2)'
 * Windows Vista+: SRWLOCK + CONDITION_VARIABLE (same as `pthread_cond_t' + `pthread_mutex_t')
 * Windows XP+:    CreateSemaphoreW (same as `sem_t') */
#define DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#elif defined(__CYGWIN__) && 0
/* TODO: This would work, but breaks because we don't define `DeeNTSystem_ThrowErrorf()' on cygwin... */
#define DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#elif defined(CONFIG_HAVE_pthread_cond_t) && defined(CONFIG_HAVE_pthread_mutex_t)
/* Waiting is implemented by blocking on a condition-variable.
 * Should-wait checking is done while holding a mutex. */
#define DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t
#elif (defined(CONFIG_HAVE_cnd_t) && defined(CONFIG_HAVE_mtx_t) && \
       defined(CONFIG_HAVE_thrd_success) && defined(CONFIG_HAVE_thrd_timedout))
/* Same as `DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t', but using the STDC API. */
#define DeeFutex_USE_cnd_t_AND_mtx_t
#elif defined(CONFIG_HAVE_sem_t)
/* Use a semaphore to keep track of how many threads are blocking as an
 * upper count-limit. We then implement WakeOne as sem_wake*1, and WakeAll
 * as sem_wake*numWaitingThreads.
 * - This doesn't race so-long as blocking threads re-check the wait condition one last
 *   time _after_ `++numWaitingThreads', since in that case it will be guarantied that
 *   the blocking thread will be woken in case of a WakeAll()
 * - The situation where more threads are woken than are actually waiting is OK, since
 *   that case will simply be handled as sporadic wake-ups the next time a wait happens
 *   All-too-many sporadic wake-ups should never happen, since any extra wake-ups will
 *   just go away when the futex controller is destroyed and re-initialized (which then
 *   happens whenever no-one is waiting on some given address anymore) */
#define DeeFutex_USE_sem_t
#else /* ... */
/* The stub implementation does nothing on wake,
 * and simply checks interrupts+yields on wait. */
#define DeeFutex_USE_yield
#endif /* !... */

#ifdef DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#include <Windows.h>

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */
#endif /* DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW */

#ifdef DeeFutex_USE_cnd_t_AND_mtx_t
#include <threads.h>
#endif /* DeeFutex_USE_cnd_t_AND_mtx_t */

#ifdef DeeFutex_USE_sem_t
#include <semaphore.h>
#endif /* DeeFutex_USE_sem_t */

DECL_BEGIN

/************************************************************************/
/* Futex API                                                            */
/************************************************************************/


/* Figure out if we need to provide futex control structures. */
#undef DeeFutex_USES_CONTROL_STRUCTURE
#if (defined(DeeFutex_USE_os_futex_32_only) ||                                                    \
     defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW) || \
     defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t) ||                                  \
     defined(DeeFutex_USE_cnd_t_AND_mtx_t) ||                                                     \
     defined(DeeFutex_USE_sem_t))
#define DeeFutex_USES_CONTROL_STRUCTURE
#endif /* ... */

/* Figure out if we need the OS futex wait list. */
#undef DeeFutex_USES_OS_FUTEX_WAIT_LIST
#if (defined(DeeFutex_USE_os_futex_32_only) || defined(DeeFutex_USE_os_futex) || \
     defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW))
#define DeeFutex_USES_OS_FUTEX_WAIT_LIST
#endif /* ... */

#ifdef DeeFutex_USES_CONTROL_STRUCTURE
#include <hybrid/sequence/rbtree.h> /* LLRBTREE_NODE, LLRBTREE_ROOT */

struct futex_controller;
SLIST_HEAD(futex_controller_slist, futex_controller);
struct futex_controller {
	uintptr_t fc_refcnt; /* [lock(ATOMIC)] Reference counter for the controller. */
	uintptr_t fc_addr;   /* [const] Futex address. */
	union {
		SLIST_ENTRY(futex_controller)   fc_free; /* Link in list of free futex controllers. */
		LLRBTREE_NODE(futex_controller) fc_node; /* Node in tree of futex controllers. */
	};
	bool      fc_isred;  /* Status bit indicating of this being a "red" node. */

	/* OS-specific futex control data. */
#ifdef DeeFutex_USE_os_futex_32_only
	uint32_t fc_word; /* Futex status word (incremented each time a wake happens) */
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	union {
		struct {
			SRWLOCK            cc_lock; /* Lock */
			CONDITION_VARIABLE cc_cond; /* Condition variable */
		} fc_nt_cond_crit; /* NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT */

		struct {
			HANDLE sm_hSemaphore; /* [const] Semaphore handle */
			DWORD  sm_dwThreads;  /* [lock(atomic)] Upper bound for # of threads waiting on `sm_hSemaphore' */
		} fc_nt_sem; /* NT_FUTEX_IMPLEMENTATION_SEMAPHORE */
	};
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)
	pthread_mutex_t fc_mutx; /* Mutex */
	pthread_cond_t  fc_cond; /* Condition variable */
#elif defined(DeeFutex_USE_cnd_t_AND_mtx_t)
	mtx_t fc_mutx; /* Mutex */
	cnd_t fc_cond; /* Condition variable */
#elif defined(DeeFutex_USE_sem_t)
	sem_t  fc_sem;       /* Semaphore */
	size_t fc_n_threads; /* [lock(atomic)] Upper bound for # of threads waiting on `fc_sem' */
#endif /* ... */
};

/* Helpers to allocate/free futex controllers.
 * NOTE: We use the deemon object heap for this since that one has extra optimizations
 *       for fixed-length objects (which `struct futex_controller' is), so that we're
 *       able to make use of the (extremely fast) slab allocator. */
#define futex_controller_alloc()    DeeObject_MALLOC(struct futex_controller)
#define futex_controller_tryalloc() DeeObject_TRYMALLOC(struct futex_controller)
#define futex_controller_free(self) DeeObject_FREE(self)

/* Define the futex-controller tree access API */
DECL_END
#define RBTREE_LEFT_LEANING
#define RBTREE(name)            futex_tree_##name
#define RBTREE_T                struct futex_controller
#define RBTREE_Tkey             uintptr_t
#define RBTREE_GETNODE(self)    (self)->fc_node
#define RBTREE_GETKEY(self)     (self)->fc_addr
#define RBTREE_REDFIELD         fc_isred
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN


/* Lock for the Futex tree controls below. */
PRIVATE Dee_atomic_rwlock_t fcont_lock = Dee_ATOMIC_RWLOCK_INIT;
#define fcont_lock_reading()    Dee_atomic_rwlock_reading(&fcont_lock)
#define fcont_lock_writing()    Dee_atomic_rwlock_writing(&fcont_lock)
#define fcont_lock_tryread()    Dee_atomic_rwlock_tryread(&fcont_lock)
#define fcont_lock_trywrite()   Dee_atomic_rwlock_trywrite(&fcont_lock)
#define fcont_lock_canread()    Dee_atomic_rwlock_canread(&fcont_lock)
#define fcont_lock_canwrite()   Dee_atomic_rwlock_canwrite(&fcont_lock)
#define fcont_lock_waitread()   Dee_atomic_rwlock_waitread(&fcont_lock)
#define fcont_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&fcont_lock)
#define fcont_lock_read()       Dee_atomic_rwlock_read(&fcont_lock)
#define fcont_lock_write()      Dee_atomic_rwlock_write(&fcont_lock)
#define fcont_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&fcont_lock)
#define fcont_lock_upgrade()    Dee_atomic_rwlock_upgrade(&fcont_lock)
#define fcont_lock_downgrade()  Dee_atomic_rwlock_downgrade(&fcont_lock)
#define fcont_lock_endwrite()   Dee_atomic_rwlock_endwrite(&fcont_lock)
#define fcont_lock_endread()    Dee_atomic_rwlock_endread(&fcont_lock)
#define fcont_lock_end()        Dee_atomic_rwlock_end(&fcont_lock)

/* [0..n][lock(fcont_lock)] List of free (but already-initialized) futex controllers. */
PRIVATE struct futex_controller_slist fcont_freelist = SLIST_HEAD_INITIALIZER(fcont_freelist);
PRIVATE size_t /*                  */ fcont_freesize = 0;

/* [0..n][lock(fcont_lock)] Tree of futex objects, ordered by the address they affect.
 * NOTE: This tree does _NOT_ hold references to the individual futex controllers!
 *       When a controller's reference counter hits `0', it will automatically remove
 *       itself from this tree, but there is a short period of time where it will still
 *       be present in this tree, so you have to do tryincref() when wanting to get refs
 *       to objects from this tree! */
PRIVATE LLRBTREE_ROOT(futex_controller) fcont_tree = NULL;

/* Max number of elements in `fcont_freelist' before further controllers are *actually* free'd */
#define FCONT_FREELIST_MAXSIZE 8


#define Dee_futex_clearall_DEFINED
INTERN size_t DCALL Dee_futex_clearall(size_t max_clear) {
	size_t result = 0;
	struct futex_controller_slist freelist;
	struct futex_controller *iter, *tvar;
	fcont_lock_write();
	freelist = fcont_freelist;
	SLIST_CLEAR(&fcont_freelist);
	fcont_freesize = 0;
	fcont_lock_endwrite();

	(void)max_clear;
	SLIST_FOREACH_SAFE (iter, &freelist, fc_free, tvar) {
		result += sizeof(*iter);
		futex_controller_free(iter);
	}
	return result;
}
#endif /* DeeFutex_USES_CONTROL_STRUCTURE */



/* Since on NT we have 3 different ways of implementing the futex system, we need an
 * initializer, as well as a controller for which implementation is chosen (for the
 * sake of performance, we define a 4th implementation that means "not-yet-initialized") */
#ifdef DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#define NT_FUTEX_IMPLEMENTATION_UNINITIALIZED 0
#define NT_FUTEX_IMPLEMENTATION_WAITONADDRESS 1
#define NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT 2
#define NT_FUTEX_IMPLEMENTATION_SEMAPHORE     3

/* [lock(LAZY_ATOMIC, WRITE_ONCE)] The chosen implementation for futex objects on NT */
PRIVATE unsigned int nt_futex_implementation = NT_FUTEX_IMPLEMENTATION_UNINITIALIZED;

#define LOAD_PROC_ADDRESS(err, pdyn_Foo, LPFOO, hModule, name) \
	do {                                                       \
		pdyn_Foo = (LPFOO)GetProcAddress(hModule, name);       \
		if unlikely(!pdyn_Foo)                                 \
			goto err;                                          \
	}	__WHILE0

typedef BOOL (WINAPI *LPWAITONADDRESS)(void volatile *Address, PVOID CompareAddress, SIZE_T AddressSize, DWORD dwMilliseconds);
typedef void (WINAPI *LPWAKEBYADDRESSSINGLE)(PVOID Address);
typedef void (WINAPI *LPWAKEBYADDRESSALL)(PVOID Address);
PRIVATE LPWAITONADDRESS /*      */ pdyn_WaitOnAddress       = NULL;
PRIVATE LPWAKEBYADDRESSSINGLE /**/ pdyn_WakeByAddressSingle = NULL;
PRIVATE LPWAKEBYADDRESSALL /*   */ pdyn_WakeByAddressAll    = NULL;

#undef WaitOnAddress
#undef WakeByAddressSingle
#undef WakeByAddressAll
#define WaitOnAddress       (*pdyn_WaitOnAddress)
#define WakeByAddressSingle (*pdyn_WakeByAddressSingle)
#define WakeByAddressAll    (*pdyn_WakeByAddressAll)

/* L"KernelBase.dll" */
PRIVATE WCHAR const wKernelBaseDll[] = { 'K', 'e', 'r', 'n', 'e', 'l', 'B', 'a', 's', 'e', '.', 'd', 'l', 'l', 0 };

PRIVATE bool DCALL nt_futex_try_initialize_WaitOnAddress(void) {
	HMODULE hKernelBase;
	hKernelBase = LoadLibraryW(wKernelBaseDll);
	if unlikely(!hKernelBase)
		goto err;
	LOAD_PROC_ADDRESS(err_kbase, pdyn_WaitOnAddress, LPWAITONADDRESS, hKernelBase, "WaitOnAddress");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_WakeByAddressSingle, LPWAKEBYADDRESSSINGLE, hKernelBase, "WakeByAddressSingle");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_WakeByAddressAll, LPWAKEBYADDRESSALL, hKernelBase, "WakeByAddressAll");
	return true;
err_kbase:
	FreeLibrary(hKernelBase);
err:
	return false;
}


typedef void (WINAPI *LPINITIALIZECONDITIONVARIABLE)(PCONDITION_VARIABLE ConditionVariable);
typedef void (WINAPI *LPWAKECONDITIONVARIABLE)(PCONDITION_VARIABLE ConditionVariable);
typedef void (WINAPI *LPWAKEALLCONDITIONVARIABLE)(PCONDITION_VARIABLE ConditionVariable);
typedef BOOL (WINAPI *LPSLEEPCONDITIONVARIABLESRW)(PCONDITION_VARIABLE ConditionVariable, PSRWLOCK SRWLock, DWORD dwMilliseconds, ULONG Flags);
typedef void (WINAPI *LPINITIALIZESRWLOCK)(PSRWLOCK SRWLock);
typedef void (WINAPI *LPRELEASESRWLOCKEXCLUSIVE)(PSRWLOCK SRWLock);
typedef void (WINAPI *LPACQUIRESRWLOCKEXCLUSIVE)(PSRWLOCK SRWLock);
typedef void (WINAPI *LPRELEASESRWLOCKSHARED)(PSRWLOCK SRWLock);
typedef void (WINAPI *LPACQUIRESRWLOCKSHARED)(PSRWLOCK SRWLock);

PRIVATE LPINITIALIZECONDITIONVARIABLE /**/ pdyn_InitializeConditionVariable = NULL;
PRIVATE LPWAKECONDITIONVARIABLE /*      */ pdyn_WakeConditionVariable       = NULL;
PRIVATE LPWAKEALLCONDITIONVARIABLE /*   */ pdyn_WakeAllConditionVariable    = NULL;
PRIVATE LPSLEEPCONDITIONVARIABLESRW /*  */ pdyn_SleepConditionVariableSRW   = NULL;
PRIVATE LPINITIALIZESRWLOCK /*          */ pdyn_InitializeSRWLock           = NULL;
PRIVATE LPRELEASESRWLOCKEXCLUSIVE /*    */ pdyn_ReleaseSRWLockExclusive     = NULL;
PRIVATE LPACQUIRESRWLOCKEXCLUSIVE /*    */ pdyn_AcquireSRWLockExclusive     = NULL;
PRIVATE LPRELEASESRWLOCKSHARED /*       */ pdyn_ReleaseSRWLockShared        = NULL;
PRIVATE LPACQUIRESRWLOCKSHARED /*       */ pdyn_AcquireSRWLockShared        = NULL;

#undef InitializeConditionVariable
#undef WakeConditionVariable
#undef WakeAllConditionVariable
#undef SleepConditionVariableSRW
#undef InitializeSRWLock
#undef ReleaseSRWLockExclusive
#undef AcquireSRWLockExclusive
#undef ReleaseSRWLockShared
#undef AcquireSRWLockShared
#define InitializeConditionVariable (*pdyn_InitializeConditionVariable)
#define WakeConditionVariable       (*pdyn_WakeConditionVariable)
#define WakeAllConditionVariable    (*pdyn_WakeAllConditionVariable)
#define SleepConditionVariableSRW   (*pdyn_SleepConditionVariableSRW)
#define InitializeSRWLock           (*pdyn_InitializeSRWLock)
#define ReleaseSRWLockExclusive     (*pdyn_ReleaseSRWLockExclusive)
#define AcquireSRWLockExclusive     (*pdyn_AcquireSRWLockExclusive)
#define ReleaseSRWLockShared        (*pdyn_ReleaseSRWLockShared)
#define AcquireSRWLockShared        (*pdyn_AcquireSRWLockShared)

PRIVATE bool DCALL nt_futex_try_initialize_CONDITION_VARIABLE(void) {
	HMODULE hKernelBase;
	hKernelBase = LoadLibraryW(wKernelBaseDll);
	if unlikely(!hKernelBase)
		goto err;
	LOAD_PROC_ADDRESS(err_kbase, pdyn_InitializeConditionVariable, LPINITIALIZECONDITIONVARIABLE, hKernelBase, "InitializeConditionVariable");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_WakeConditionVariable, LPWAKECONDITIONVARIABLE, hKernelBase, "WakeConditionVariable");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_WakeAllConditionVariable, LPWAKEALLCONDITIONVARIABLE, hKernelBase, "WakeAllConditionVariable");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_SleepConditionVariableSRW, LPSLEEPCONDITIONVARIABLESRW, hKernelBase, "SleepConditionVariableSRW");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_InitializeSRWLock, LPINITIALIZESRWLOCK, hKernelBase, "InitializeSRWLock");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_ReleaseSRWLockExclusive, LPRELEASESRWLOCKEXCLUSIVE, hKernelBase, "ReleaseSRWLockExclusive");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_AcquireSRWLockExclusive, LPACQUIRESRWLOCKEXCLUSIVE, hKernelBase, "AcquireSRWLockExclusive");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_ReleaseSRWLockShared, LPRELEASESRWLOCKSHARED, hKernelBase, "ReleaseSRWLockShared");
	LOAD_PROC_ADDRESS(err_kbase, pdyn_AcquireSRWLockShared, LPACQUIRESRWLOCKSHARED, hKernelBase, "AcquireSRWLockShared");
	return true;
err_kbase:
	FreeLibrary(hKernelBase);
err:
	return false;
}

PRIVATE void DCALL nt_futex_do_initialize_subsystem(void) {
	/* First up: try to load what we need to use `WaitOnAddress()' */
	if (nt_futex_try_initialize_WaitOnAddress()) {
		atomic_write(&nt_futex_implementation, NT_FUTEX_IMPLEMENTATION_WAITONADDRESS);
		return;
	}

	/* Next up: try to use Vista+ CONDITION_VARIABLE+CRITICAL_SECTION */
	if (nt_futex_try_initialize_CONDITION_VARIABLE()) {
		atomic_write(&nt_futex_implementation, NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT);
		return;
	}

	/* Fallback: have to use XP+ Semaphores */
	atomic_write(&nt_futex_implementation, NT_FUTEX_IMPLEMENTATION_SEMAPHORE);
}

/* Call this function (`nt_futex_initialize_subsystem()') to initialize the futex-subsystem on NT */
PRIVATE struct atomic_once nt_futex_subsystem_initialized = ATOMIC_ONCE_INIT;
PRIVATE void DCALL nt_futex_initialize_subsystem(void) {
	ATOMIC_ONCE_RUN(&nt_futex_subsystem_initialized, {
		nt_futex_do_initialize_subsystem();
	});
}
#endif /* DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW */




/* Even when the OS is providing a native futex API, we still need a way to keep track of
 * threads that are currently inside blocking system calls to futex functions. This is because
 * we need a way to know which thread is blocked-on (or about to be blocked-on) which address,
 * so we're able to send sporadic wake-up signals to blocking threads in `DeeThread_Interrupt'
 *
 * This is needed because, while we're able to interrupt blocking threads through various
 * different OS-specific means, all of these means only work to interrupt a thread that is
 * already in kernel-space.
 *
 * NOTE: The race condition where the thread in question may yet to have reached the point
 *       where it is actually able to receive `os_futex_wakeall()' signals (but is already
 *       apart of the wait-list) is solved because we simply keep on waking all threads on
 *       the to-be woken thread's address until the thread we are trying to wake no longer
 *       appears in the global list of waiting threads. */
#ifdef DeeFutex_USES_OS_FUTEX_WAIT_LIST
struct os_futex_wait_entry;
LIST_HEAD(os_futex_wait_list_struct, os_futex_wait_entry);
struct os_futex_wait_entry {
	LIST_ENTRY(os_futex_wait_entry) ofwe_link; /* [1..1][lock(os_futex_wait_lock)] Link in list of waiting threads */
	DeeThreadObject                *ofwe_thrd; /* [1..1][const] The thread that is waiting. */
	uintptr_t                       ofwe_addr; /* [const] Address that this thread is waiting on */
};

/* [lock(os_futex_wait_lock)][0..n] List of threads that are currently waiting on futex addresses. */
PRIVATE struct os_futex_wait_list_struct os_futex_wait_list = LIST_HEAD_INITIALIZER(os_futex_wait_list);
PRIVATE Dee_atomic_lock_t /*          */ os_futex_wait_lock = Dee_ATOMIC_LOCK_INIT;

#define os_futex_wait_begin(addr)                                       \
	do {                                                                \
		struct os_futex_wait_entry _ofwe_entry;                         \
		_ofwe_entry.ofwe_addr = (uintptr_t)(addr);                      \
		_ofwe_entry.ofwe_thrd = DeeThread_Self();                       \
		Dee_atomic_lock_acquire(&os_futex_wait_lock);                   \
		LIST_INSERT_HEAD(&os_futex_wait_list, &_ofwe_entry, ofwe_link); \
		Dee_atomic_lock_release(&os_futex_wait_lock);                   \
		do
#define os_futex_wait_break()                         \
	do {                                              \
		Dee_atomic_lock_acquire(&os_futex_wait_lock); \
		LIST_REMOVE(&_ofwe_entry, ofwe_link);         \
		Dee_atomic_lock_release(&os_futex_wait_lock); \
	}	__WHILE0
#define os_futex_wait_end()    \
		__WHILE0;              \
		os_futex_wait_break(); \
	}	__WHILE0

PRIVATE NONNULL((1)) void DCALL
os_futex_wait_list_wakethread(DeeThreadObject *thread) {
	/* Search for entries regarding `thread' */
	for (;;) {
		struct os_futex_wait_entry *ent;
		bool found_thread = false;
		Dee_atomic_lock_acquire(&os_futex_wait_lock);
		LIST_FOREACH (ent, &os_futex_wait_list, ofwe_link) {
			if (ent->ofwe_thrd == thread) {
				void *addr = (void *)ent->ofwe_addr;
				Dee_atomic_lock_release(&os_futex_wait_lock);
#ifdef DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
				WakeByAddressAll(addr);
#else /* DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW */
				os_futex_wakeall(addr);
#endif /* !DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW */
				found_thread = true;
				break;
			}
		}
		if (!found_thread) {
			Dee_atomic_lock_release(&os_futex_wait_lock);
			break;
		}

		/* Yield a bit so `thread' will hopefully get a quantum. */
		SCHED_YIELD();
		SCHED_YIELD();
		SCHED_YIELD();
	}
}
#endif /* DeeFutex_USES_OS_FUTEX_WAIT_LIST */



#ifdef DeeFutex_USES_CONTROL_STRUCTURE
PRIVATE NONNULL((1)) void DCALL
futex_controller_do_destroy(struct futex_controller *__restrict self) {
#ifdef DeeFutex_USE_os_futex_32_only
	/* No OS-specific cleanup necessary */
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	/* NOTE: No need to handle `NT_FUTEX_IMPLEMENTATION_UNINITIALIZED' here, since
	 *       the futex controller couldn't have been created in the first place if
	 *       the implementation wasn't known. */
	switch (nt_futex_implementation) {
	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT:
		/* Neither CONDITION_VARIABLEs, nor SWRLOCKs have destructors. */
		break;
	case NT_FUTEX_IMPLEMENTATION_SEMAPHORE:
		(void)CloseHandle(self->fc_nt_sem.sm_hSemaphore);
		break;
	default: __builtin_unreachable();
	}
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)
	(void)pthread_mutex_destroy(&self->fc_mutx);
	(void)pthread_cond_destroy(&self->fc_cond);
#elif defined(DeeFutex_USE_cnd_t_AND_mtx_t)
	(void)mtx_destroy(&self->fc_mutx);
	(void)cnd_destroy(&self->fc_cond);
#elif defined(DeeFutex_USE_sem_t)
#ifdef CONFIG_HAVE_sem_destroy
	(void)sem_destroy(&self->fc_sem);
#endif /* CONFIG_HAVE_sem_destroy */
#endif /* ... */

	/* Finally, free the actual control object itself */
	futex_controller_free(self);
}

/* incref/decref operations for `struct futex_controller' */
#define futex_controller_incref(self) atomic_inc(&(self)->fc_refcnt)
#define futex_controller_decref(self) (void)(atomic_decfetch(&(self)->fc_refcnt) || (futex_controller_destroy(self), 0))

LOCAL WUNUSED NONNULL((1)) bool DCALL
futex_controller_tryincref(struct futex_controller *__restrict self) {
	uintptr_t refcnt;
	do {
		refcnt = atomic_read(&self->fc_refcnt);
		if unlikely(refcnt == 0)
			return false; /* It's already dead... */
	} while (!atomic_cmpxch_weak_explicit(&self->fc_refcnt, refcnt, refcnt + 1,
	                                      Dee_ATOMIC_SEQ_CST, Dee_ATOMIC_RELAXED));
	return true;
}


/* Handler for when the controller's reference counter reaches `0' */
PRIVATE NONNULL((1)) void DCALL
futex_controller_destroy(struct futex_controller *__restrict self) {
	fcont_lock_write();

	/* Remove ourselves from the tree (but only if that hasn't been done already) */
	{
		struct futex_controller *removed_node;
		removed_node = futex_tree_remove(&fcont_tree, self->fc_addr);
		if unlikely(removed_node != self && removed_node)
			futex_tree_insert(&fcont_tree, removed_node);
	}

	/* Check if we should add ourselves to the list of free controllers. */
	if (fcont_freesize < FCONT_FREELIST_MAXSIZE) {
		SLIST_INSERT_HEAD(&fcont_freelist, self, fc_free);
		++fcont_freesize;
		fcont_lock_endwrite();
		return;
	}
	fcont_lock_endwrite();

	/* Must *actually* destroy the controller. */
	futex_controller_do_destroy(self);
}


/* Lookup a futex controller at a given address
 * (but don't create one there if there is none)
 *
 * @return: * :   Reference to the controller at `addr'
 * @return: NULL: There is no controller at `addr' (no error was thrown) */
PRIVATE WUNUSED DREF struct futex_controller *DCALL
futex_ataddr_get(uintptr_t addr) {
	DREF struct futex_controller *result;
	fcont_lock_read();
	result = futex_tree_locate(fcont_tree, addr);
	if (result && !futex_controller_tryincref(result))
		result = NULL;
	fcont_lock_endread();
	return result;
}

/* Lookup a futex controller at a given address,
 * or create one at said address if there wasn't
 * one there already.
 *
 * @return: * :   Reference to the controller at `addr'
 * @return: NULL: Failed to create a new controller (an error was thrown)
 *                Note that `futex_ataddr_trycreate' doesn't throw an error */
PRIVATE WUNUSED DREF struct futex_controller *DCALL futex_ataddr_create(uintptr_t addr);
PRIVATE WUNUSED DREF struct futex_controller *DCALL futex_ataddr_trycreate(uintptr_t addr);

#ifndef __INTELLISENSE__
#define DEFINE_futex_ataddr_create
#include "futex-controller-new.c.inl"
#define DEFINE_futex_ataddr_trycreate
#include "futex-controller-new.c.inl"
#endif /* !__INTELLISENSE__ */


PRIVATE NONNULL((1)) void DCALL
futex_controller_wakeall(struct futex_controller *__restrict self) {
#ifdef DeeFutex_USE_os_futex_32_only
	atomic_inc(&self->fc_word);
	(void)os_futex_wakeall(&self->fc_word);
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	switch (nt_futex_implementation) {

	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT:
		/* Need an exclusive lock here to ensure that no thread that is about to
		 * wait for the mutex is still busy checking if it *should* wait (or if
		 * it has pending interrupts) */
		AcquireSRWLockExclusive(&self->fc_nt_cond_crit.cc_lock);
		WakeAllConditionVariable(&self->fc_nt_cond_crit.cc_cond);
		ReleaseSRWLockExclusive(&self->fc_nt_cond_crit.cc_lock);
		break;

	case NT_FUTEX_IMPLEMENTATION_SEMAPHORE: {
		DWORD dwWaitingThreads;
		dwWaitingThreads = atomic_read(&self->fc_nt_sem.sm_dwThreads);
		if likely(dwWaitingThreads > 0)
			(void)ReleaseSemaphore(self->fc_nt_sem.sm_hSemaphore, dwWaitingThreads, NULL);
	}	break;

	default: __builtin_unreachable();
	}
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)
	/* Need to lock the mutex to inter-lock with a thread that
	 * might still be checking the should-wait-condition (iow:
	 * need to make sure that everyone is truly waiting, and
	 * not still busy checking if they *should* wait) */
	(void)pthread_mutex_lock(&self->fc_mutx);
	(void)pthread_cond_broadcast(&self->fc_cond);
	(void)pthread_mutex_unlock(&self->fc_mutx);
#elif defined(DeeFutex_USE_cnd_t_AND_mtx_t)
	/* *ditto* for stdc-based mutex/condition-variable implementation */
	(void)mtx_lock(&self->fc_mutx);
	(void)cnd_broadcast(&self->fc_cond);
	(void)mtx_unlock(&self->fc_mutx);
#elif defined(DeeFutex_USE_sem_t)
	size_t n_threads = atomic_read(&self->fc_n_threads);
#ifdef CONFIG_HAVE_sem_post_multiple
	(void)sem_post_multiple(&self->fc_sem, n_threads);
#else /* CONFIG_HAVE_sem_post_multiple */
	while (n_threads > 0) {
		(void)sem_post(&self->fc_sem);
		--n_threads;
	}
#endif /* !CONFIG_HAVE_sem_post_multiple */
#endif /* ... */
}

PRIVATE NONNULL((1)) void DCALL
futex_controller_wakeall_tree(struct futex_controller *__restrict self) {
again:
	futex_controller_wakeall(self);
	if (self->fc_node.rb_lhs) {
		if (self->fc_node.rb_rhs)
			futex_controller_wakeall_tree(self->fc_node.rb_rhs);
		self = self->fc_node.rb_lhs;
		goto again;
	}
	if (self->fc_node.rb_rhs) {
		self = self->fc_node.rb_rhs;
		goto again;
	}
}
#endif /* DeeFutex_USES_CONTROL_STRUCTURE */


#if (defined(DeeFutex_USES_OS_FUTEX_WAIT_LIST) || \
     defined(DeeFutex_USES_CONTROL_STRUCTURE))
INTERN NONNULL((1)) void DCALL
DeeFutex_WakeGlobal(DeeThreadObject *thread) {
	(void)thread;

	/* Search for the thread in the OS futex wait list */
#ifdef DeeFutex_USES_OS_FUTEX_WAIT_LIST
#ifdef DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
	switch (nt_futex_implementation) {

	case NT_FUTEX_IMPLEMENTATION_UNINITIALIZED:
		/* If not yet initialized, there can't be *any* threads waiting *anywhere* */
		return;

	case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS:
		/* If the WaitOnAddress implementation is selected, futex controllers
		 * below go completely unused. Instead, we must search for the thread
		 * in question within the global wait-list. */
		os_futex_wait_list_wakethread(thread);
		return;

	default: break;
	}
#else /* DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW */
	os_futex_wait_list_wakethread(thread);
	/* Fallthru to the futex-tree method. When `DeeFutex_USE_os_futex_32_only'
	 * is selected, we can have both wait lists, **as well as** futex controllers,
	 * where the wait list is used for 32-bit futex operations, and controllers
	 * are used for 64-bit futex operations. */
#endif /* !DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW */
#endif /* DeeFutex_USES_OS_FUTEX_WAIT_LIST */

	/* Wake all threads connected to a futex control structure. */
#ifdef DeeFutex_USES_CONTROL_STRUCTURE
	fcont_lock_read();
	if (fcont_tree != NULL)
		futex_controller_wakeall_tree(fcont_tree);
	fcont_lock_endread();
#endif /* DeeFutex_USES_CONTROL_STRUCTURE */
}
#else /* ... */
INTERN NONNULL((1)) void DCALL
DeeFutex_WakeGlobal(DeeThreadObject *thread) {
	(void)thread;
	/* No-op */
	COMPILER_IMPURE();
}
#endif /* !... */

#ifndef CONFIG_NO_THREADS
#ifndef Dee_futex_clearall_DEFINED
#define Dee_futex_clearall_DEFINED
INTERN size_t DCALL Dee_futex_clearall(size_t max_clear) {
	(void)max_clear;
	return 0;
}
#endif /* !Dee_futex_clearall_DEFINED */
#endif /* !CONFIG_NO_THREADS */


DECL_END

/* Define the high-level implementations for futex operations
 * (using multi-include source files to prevent redundancy) */
#ifndef __INTELLISENSE__
#define DEFINE_DeeFutex_WakeOne
#include "futex-wake.c.inl"
#define DEFINE_DeeFutex_WakeAll
#include "futex-wake.c.inl"

#define DEFINE_DeeFutex_Wait32
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait32Timed
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait32NoInt
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait32NoIntTimed
#include "futex-wait.c.inl"

#if __SIZEOF_POINTER__ >= 8
#define DEFINE_DeeFutex_Wait64
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait64Timed
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait64NoInt
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait64NoIntTimed
#include "futex-wait.c.inl"
#endif /* __SIZEOF_POINTER__ >= 8 */
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_SYSTEM_FUTEX_C */
