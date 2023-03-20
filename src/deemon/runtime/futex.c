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
#ifndef GUARD_DEEMON_RUNTIME_FUTEX_C
#define GUARD_DEEMON_RUNTIME_FUTEX_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/system-features.h> /* memcpy(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/rwlock.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/atomic-once.h>
#include <hybrid/sched/yield.h>

#include <stdbool.h>

#ifdef CONFIG_HAVE_LINUX_FUTEX_H
#include <linux/futex.h>
#endif /* CONFIG_HAVE_LINUX_FUTEX_H */

#ifdef CONFIG_HAVE_TIME_H
#include <time.h>
#endif /* CONFIG_HAVE_TIME_H */

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#ifdef CONFIG_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* CONFIG_HAVE_SYS_TIME_H */

#ifndef INT32_MAX
#include <hybrid/limitcore.h>
#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#endif /* !INT32_MAX */

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

#if __SIZEOF_POINTER__ == 4 && defined(CONFIG_HAVE_futex_waitwhile) && defined(CONFIG_HAVE_futex_timedwaitwhile)
#define os_futex_wait32(uaddr, expected) futex_waitwhile((lfutex_t *)(uaddr), expected)
#define os_futex_wait32_timed            os_futex_wait32_timed
LOCAL int DCALL os_futex_wait32_timed(void *uaddr, uint32_t expected,
                                     uint64_t timeout_nanoseconds) {
	struct timespec ts;
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return futex_timedwaitwhile(uaddr, expected, &ts);
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
#endif /* linux_futex && FUTEX_WAIT && __SIZEOF_INT__ == 4 */

#if __SIZEOF_POINTER__ == 8 && defined(CONFIG_HAVE_futex_waitwhile) && defined(CONFIG_HAVE_futex_timedwaitwhile)
#define os_futex_wait64(uaddr, expected) futex_waitwhile((lfutex_t *)(uaddr), expected)
#define os_futex_wait64_timed            os_futex_wait64_timed
LOCAL int DCALL os_futex_wait64_timed(void *uaddr, uint64_t expected,
                                      uint64_t timeout_nanoseconds) {
	struct timespec ts;
	if (sizeof(ts.tv_sec) < 4 && timeout_nanoseconds == (uint64_t)-1)
		return os_futex_wait64(uaddr, expected);
	ts.tv_sec  = timeout_nanoseconds / UINT32_C(1000000000);
	ts.tv_nsec = timeout_nanoseconds % UINT32_C(1000000000);
	return futex_timedwaitwhile(uaddr, expected, &ts);
}
#endif /* __SIZEOF_POINTER__ == 8 && CONFIG_HAVE_futex_waitwhile && CONFIG_HAVE_futex_timedwaitwhile */


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
#if (defined(CONFIG_HAVE_pthread_cond_signal) &&                                            \
     defined(CONFIG_HAVE_pthread_cond_broadcast) &&                                         \
     defined(CONFIG_HAVE_pthread_cond_wait) &&                                              \
     ((defined(CONFIG_HAVE_pthread_cond_timedwait) && defined(CONFIG_HAVE_gettimeofday)) || \
      defined(CONFIG_HAVE_pthread_cond_reltimedwait_np)))
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
#if (defined(CONFIG_HAVE_SEMAPHORE_H) && defined(CONFIG_HAVE_sem_init) && \
     defined(CONFIG_HAVE_sem_wait) && defined(CONFIG_HAVE_sem_post) &&    \
     (defined(CONFIG_HAVE_sem_timedwait) && defined(CONFIG_HAVE_gettimeofday)))
#define CONFIG_HAVE_sem_t
#endif /* ... */


/* Figure out how we want to implement the deemon Futex API.
 *
 * NOTE: All implementations except for `DeeFutex_USE_os_futex'
 *       use dynamically allocated structures and a hash-table
 *       to translate wake/wait-addresses into those structures.
 */
#undef DeeFutex_USE_os_futex
#undef DeeFutex_USE_os_futex_32_only
#undef DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#undef DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t
#undef DeeFutex_USE_sem_t
#undef DeeFutex_USE_yield
#undef DeeFutex_USE_stub
#ifdef CONFIG_NO_THREADS
#define DeeFutex_USE_stub
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
#elif defined(CONFIG_HOST_WINDOWS) || defined(__CYGWIN__)
/* Windows 8+:     WaitOnAddress is pretty much the same as futex()
 * Windows Vista+: SRWLOCK + CRITICAL_SECTION (same as `pthread_cond_t' + `pthread_mutex_t')
 * Windows XP+:    CreateSemaphoreW (same as `sem_t') */
#define DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW
#elif defined(CONFIG_HAVE_pthread_cond_t) && defined(CONFIG_HAVE_pthread_mutex_t)
/* Waiting is implemented by blocking on a condition-variable.
 * Should-wait checking is done while holding a mutex.
 * Wake-up happen while holding a lock to that same mutex. */
#define DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t
#elif defined(CONFIG_HAVE_sem_t)
/* Use a semaphore to keep track of how many threads are blocking as an
 * upper count-limit, we can implement WakeOne as sem_wake*1, and WakeAll
 * as sem_wake*numWaitingThreads.
 * - This doesn't race so-long as blocking threads re-check the wait condition one last
 *   time _after_ `++numWaitingThreads', since in that case it will be guarantied that
 *   the blocking thread will be woken in case of a WakeAll()
 * - The situation where more threads are woken that are actually waiting is OK, since
 *   that case will simply be handled as sporadic wake-ups the next time a wait happens */
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
     defined(DeeFutex_USE_sem_t))
#define DeeFutex_USES_CONTROL_STRUCTURE
#endif /* ... */

#ifdef DeeFutex_USES_CONTROL_STRUCTURE
#include <hybrid/sequence/list.h>
#include <hybrid/sequence/rbtree.h>

struct futex_controller;
SLIST_HEAD(futex_controller_slist, futex_controller);
struct futex_controller {
	union {
		SLIST_ENTRY(futex_controller)   fc_free; /* Link in list of free futex controllers. */
		LLRBTREE_NODE(futex_controller) fc_node; /* Node in tree of futex controllers. */
	};
	uintptr_t fc_addr;   /* [const] Futex address. */
	bool      fc_isred;  /* Status bit indicating of this being a "red" node. */
	uintptr_t fc_refcnt; /* [lock(ATOMIC)] Reference counter for the controller. */

	/* OS-specific futex control data. */
#ifdef DeeFutex_USE_os_futex_32_only
	uint32_t fc_word; /* Futex status word (incremented each time a wake happens) */
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	union {
		/* ==== Rant on WaitOnAddress ====
		 *
		 * You would that that we'd be able to just directly use `WaitOnAddress' on the
		 * actual futex address in order to implement fully support for futex operation.
		 * 
		 * And you'd be correct (but sadly only kind-of)
		 * 
		 * The problem here lies in the fact that `WaitOnAddress' IS NOT INTERRUPTIBLE,
		 * and there is no `WaitOnAddressEx' function with `BOOL bAlertable' which we
		 * could then set to `TRUE'
		 * 
		 * As such, `DeeThread_Wake()' would have no way to interrupt/wake-up a thread
		 * which is currently being blocked in a call to `WaitOnAddress'.
		 * 
		 * Now you may think this could be solved by simply keeping a singly-linked list
		 * of all threads that are currently inside of calls to `WaitOnAddress', so then
		 * we could simply call `WakeByAddressAll' on all of those thread.
		 * But the problem with this would be the race condition between the thread that
		 * has yet to reach kernel-space and enter its truly-sleeping-state. If we were
		 * to try and wake up the thread *before* then, it wouldn't get the memo and also
		 * would not receive the interrupt.
		 * This *could* be rectified if we had a method `IsThreadBlockedOnSomething()`,
		 * but alas, no such method exists (and the only way to get information about
		 * the actual sleep-state of threads doesn't let you specify a specific thread,
		 * but needs *all* threads: https://stackoverflow.com/a/22949726/3296587). If
		 * such a function existed, we'd be able to briefly wait until a thread that
		 * already appears in the list of threads calling `WaitOnAddress' actually has
		 * entered its sleep-state (so it'll be able to receive `WakeByAddressAll').
		 * 
		 * So with all of this in mind, the only *real* way we have is to use a futex
		 * control word like we also do under `DeeFutex_USE_os_futex_32_only', and then
		 * have `DeeThread_Wake()' increment that control word for all futexes, before
		 * then broadcasting to all threads which may be waiting for on some futex, thus
		 * ensuring that *any* thread currently inside of a futex-wait call will act as
		 * though it was woken by `DeeFutex_WakeAll()'. */
		uint32_t fc_nt_word; /* NT_FUTEX_IMPLEMENTATION_WAITONADDRESS */

		struct {
			SRWLOCK            cc_lock; /* Lock (only ever used in its exclusive-mode) */
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
#define futex_controller_free(self) DeeObject_FREE(self)

/* Define the futex-controller tree access API */
#define RBTREE_LEFT_LEANING
#define RBTREE(name)            futex_tree_##name
#define RBTREE_T                struct futex_controller
#define RBTREE_Tkey             uintptr_t
#define RBTREE_GETNODE(self)    (self)->fc_node
#define RBTREE_GETKEY(self)     (self)->fc_addr
#define RBTREE_REDFIELD         fc_isred
#define RBTREE_CC               FCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#include <hybrid/sequence/rbtree-abi.h>


/* Lock for the Futex tree controls below. */
PRIVATE struct atomic_rwlock fcont_lock = ATOMIC_RWLOCK_INIT;

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

#define LOAD_PROC_ADDRESS(err, pdyn_Foo, LPFOO, hModule, name) \
	do {                                                       \
		pdyn_Foo = (LPFOO)GetProcAddress(hModule, name);       \
		if unlikely(!pdyn_Foo)                                 \
			goto err;                                          \
	}	__WHILE0

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



#ifdef DeeFutex_USES_CONTROL_STRUCTURE

PRIVATE NONNULL((1)) void DCALL
futex_controller_do_destroy(struct futex_controller *__restrict self) {
#ifdef DeeFutex_USE_os_futex_32_only
	/* No OS-specific cleanup necessary */
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	/* NOTE: NO need to handle `NT_FUTEX_IMPLEMENTATION_UNINITIALIZED' here, since
	 *       the futex controller couldn't have been created in the first place if
	 *       the implementation wasn't known. */
	switch (nt_futex_implementation) {
	case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS:
		/* Nothing to do here. */
		break;
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
#elif defined(DeeFutex_USE_sem_t)
#ifdef CONFIG_HAVE_sem_destroy
	sem_destroy(&self->fc_sem);
#endif /* CONFIG_HAVE_sem_destroy */
#endif /* ... */

	/* Finally, free the actual control object itself */
	futex_controller_free(self);
}

/* Raw, low-level allocate a new futex controller. */
PRIVATE WUNUSED struct futex_controller *DCALL
futex_controller_do_new_impl(void) {
	DREF struct futex_controller *result;
	result = futex_controller_alloc();
	if unlikely(!result)
		goto err;

	/* Initialize the os-specific part of `result' */
#ifdef DeeFutex_USE_os_futex_32_only
	result->fc_word = 0;
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	switch (nt_futex_implementation) {
	case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS:
		result->fc_nt_word = 0;
		break;
	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT:
		/* Neither CONDITION_VARIABLEs, nor SWRLOCKs have destructors. */
		InitializeConditionVariable(&result->fc_nt_cond_crit.cc_cond);
		InitializeSRWLock(&result->fc_nt_cond_crit.cc_lock);
		break;
	case NT_FUTEX_IMPLEMENTATION_SEMAPHORE: {
		HANDLE hSem = CreateSemaphoreW(NULL, 0, INT32_MAX, NULL);
		if unlikely(hSem == NULL || hSem == INVALID_HANDLE_VALUE) {
			DWORD dwError = GetLastError();
			futex_controller_free(result);
			DeeNTSystem_ThrowErrorf(NULL, dwError, "Failed to allocate semaphore");
			goto err;
		}
		result->fc_nt_sem.sm_hSemaphore = hSem;
		result->fc_nt_sem.sm_dwThreads  = 0;
	}	break;
	default: __builtin_unreachable();
	}
#elif defined(DeeFutex_USE_pthread_cond_t_AND_pthread_mutex_t)
	{
		int error;
		error = pthread_mutex_init(&result->fc_mutx);
		if likely(error == 0) {
			error = pthread_cond_init(&result->fc_cond);
			if unlikely(error != 0) {
				(void)pthread_mutex_destroy(&result->fc_mutx);
			}
		}
		if unlikely(error != 0) {
			futex_controller_free(result);
			DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to initialize mutex and condition variable");
			goto err;
		}
	}
#elif defined(DeeFutex_USE_sem_t)
	if unlikely(sem_init(&result->fc_sem) != 0) {
		int error = DeeSystem_GetErrno();
		futex_controller_free(result);
		DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to initialize semaphore");
		goto err;
	}
	result->fc_n_threads = 0;
#endif /* ... */

	return result;
err:
	return NULL;
}


/* Similar to `futex_controller_do_new_impl()', but try
 * to take a pre-existing object from the free-list, as
 * well as also set the reference counter to `1'.
 *
 * @return: * :   The new controller.
 * @return: NULL: Alloc failed (an error was thrown) */
PRIVATE WUNUSED DREF struct futex_controller *DCALL
futex_controller_new(void) {
	DREF struct futex_controller *result;
	if (atomic_read(&fcont_freesize) != 0) {
		atomic_rwlock_write(&fcont_lock);
		result = SLIST_FIRST(&fcont_freelist);
		if likely(result != NULL) {
			SLIST_REMOVE_HEAD(&fcont_freelist, fc_free);
			--fcont_freesize;
			atomic_rwlock_endwrite(&fcont_lock);
			goto set_refcnt;
		}
		atomic_rwlock_endwrite(&fcont_lock);
	}
	result = futex_controller_do_new_impl();
	if likely(result) {
set_refcnt:
		result->fc_refcnt = 1;
	}
	return result;
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
	                                      __ATOMIC_RELEASE, __ATOMIC_RELEASE));
	return true;
}


/* Handler for when the controller's reference counter reaches `0' */
PRIVATE NONNULL((1)) void DCALL
futex_controller_destroy(struct futex_controller *__restrict self) {
	atomic_rwlock_write(&fcont_lock);

	/* Remove ourselves from the free (but only if that hasn't been done already) */
	{
		struct futex_controller *removed_node;
		removed_node = futex_tree_remove(&fcont_tree, self->fc_addr);
		if unlikely(removed_node != self && removed_node != NULL)
			futex_tree_insert(&fcont_tree, removed_node);
	}

	/* Check if we should add ourselves to the list of free controllers. */
	if (fcont_freesize < FCONT_FREELIST_MAXSIZE) {
		SLIST_INSERT_HEAD(&fcont_freelist, self, fc_free);
		++fcont_freesize;
		atomic_rwlock_endwrite(&fcont_lock);
		return;
	}
	atomic_rwlock_endwrite(&fcont_lock);

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
	atomic_rwlock_read(&fcont_lock);
	result = futex_tree_locate(fcont_tree, addr);
	if (result && !futex_controller_tryincref(result))
		result = NULL;
	atomic_rwlock_endread(&fcont_lock);
	return result;
}


/* Lookup a futex controller at a given address,
 * or create one at said address if there wasn't
 * one there already.
 *
 * @return: * :   Reference to the controller at `addr'
 * @return: NULL: Failed to create a new controller (an error was thrown) */
PRIVATE WUNUSED DREF struct futex_controller *DCALL
futex_ataddr_create(uintptr_t addr) {
	DREF struct futex_controller *result;
	result = futex_ataddr_get(addr);
	if (result == NULL) {
		/* Must create a new controller. */
		result = futex_controller_new();
		if likely(result != NULL) {
			DREF struct futex_controller *existing_result;

			/* Initialize the new controller's address. */
			result->fc_addr = addr;

			/* Inject the new controller into the tree. */
			atomic_rwlock_write(&fcont_lock);
			existing_result = futex_tree_locate(fcont_tree, addr);
			if unlikely(existing_result) {
				if (futex_controller_tryincref(existing_result)) {
					/* Race condition: another thread created the controller before we could. */
					atomic_rwlock_endwrite(&fcont_lock);
					futex_controller_destroy(result);
					return existing_result;
				}
				/* There is a dead controller at our address.
				 * -> just remove it from the tree so we can move on. */
				futex_tree_removenode(&fcont_tree, existing_result);
			}

			/* Insert our newly created controller into the tree. */
			futex_tree_insert(&fcont_tree, result);
			atomic_rwlock_endwrite(&fcont_lock);
		}
	}
	return result;
}

PRIVATE NONNULL((1)) void DCALL
futex_controller_wakeall(struct futex_controller *__restrict self) {
#ifdef DeeFutex_USE_os_futex_32_only
	atomic_inc(&self->fc_word);
	(void)os_futex_wakeall(&self->fc_word);
#elif defined(DeeFutex_USE_WaitOnAddress_OR_CONDITION_VARIABLE_AND_SRWLOCK_OR_CreateSemaphoreW)
	switch (nt_futex_implementation) {

	case NT_FUTEX_IMPLEMENTATION_WAITONADDRESS:
		atomic_inc(&self->fc_nt_word);
		WakeByAddressAll(&self->fc_nt_word);
		break;

	case NT_FUTEX_IMPLEMENTATION_COND_AND_CRIT:
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
	(void)pthread_mutex_lock(&self->fc_mutx);
	(void)pthread_cond_broadcast(&self->fc_cond);
	(void)pthread_mutex_unlock(&self->fc_mutx);
#elif defined(DeeFutex_USE_sem_t)
	size_t n_threads;
	n_threads = atomic_read(&self->fc_n_threads);
	while (n_threads > 0) {
		(void)sem_post(&self->fc_sem);
		--n_threads;
	}
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

INTERN void DCALL DeeFutex_WakeGlobal(void) {
	atomic_rwlock_read(&fcont_lock);
	if (fcont_tree != NULL)
		futex_controller_wakeall_tree(fcont_tree);
	atomic_rwlock_endread(&fcont_lock);
}

#else /* DeeFutex_USES_CONTROL_STRUCTURE */
INTERN void DCALL DeeFutex_WakeGlobal(void) {
	/* No-op */
	COMPILER_IMPURE();
}
#endif /* !DeeFutex_USES_CONTROL_STRUCTURE */

/* Define the high-level implementations for futex operations
 * (using multi-include source files to prevent redundancy) */
#ifndef __INTELLISENSE__
DECL_END

#define DEFINE_DeeFutex_WakeOne
#include "futex-wake.c.inl"
#define DEFINE_DeeFutex_WakeAll
#include "futex-wake.c.inl"

#define DEFINE_DeeFutex_Wait32
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait32Timed
#include "futex-wait.c.inl"

#if __SIZEOF_POINTER__ >= 8
#define DEFINE_DeeFutex_Wait64
#include "futex-wait.c.inl"
#define DEFINE_DeeFutex_Wait64Timed
#include "futex-wait.c.inl"
#endif /* __SIZEOF_POINTER__ >= 8 */

DECL_BEGIN
#endif /* !__INTELLISENSE__ */





/************************************************************************/
/* Shared lock (scheduler-level blocking lock)                          */
/************************************************************************/

/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_acquire)(Dee_shared_lock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	uint32_t lockword;
again:
	/* NOTE: If there suddenly were more than UINT_MAX threads trying to acquire the same
	 *       lock  all at the same time, this could overflow. -- But I think that's not a
	 *       thing that could ever happen... */
	while ((lockword = atomic_fetchinc_explicit(&self->sl_lock, __ATOMIC_ACQUIRE)) != 0) {
		int error;
		if unlikely(lockword != 1) {
			/* This can happen if multiple threads try to acquire the lock at the same time.
			 * In  this case, we must normalize the  lock-word back to `state = 2', but only
			 * for as long as the lock itself remains acquired by some-one.
			 *
			 * This code right here is also carefully written such that it always does
			 * the  right thing, no  matter how many  threads execute it concurrently. */
			++lockword;
			while (!atomic_cmpxch_explicit(&self->sl_lock, lockword, 2,
			                               __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
				lockword = atomic_read(&self->sl_lock);
				if unlikely(lockword == 0)
					goto again; /* Lock suddenly become available */
				if unlikely(lockword == 2)
					break; /* Some other thread did the normalize for us! */
			}
		}
		error = DeeFutex_Wait32(&self->sl_lock, 2);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_waitfor)(Dee_shared_lock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
	while ((lockword = atomic_read(&self->sl_lock)) != 0) {
		int error;
		if (lockword == 1)
			atomic_cmpxch(&self->sl_lock, 1, 2);
		error = DeeFutex_Wait32(&self->sl_lock, 2);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

/* Same as `Dee_shared_lock_acquire()' / `Dee_shared_lock_waitfor()',
 * but also takes an additional timeout in nano-seconds. The special
 * values `0' (try-acquire) and `(uint64_t)-1' (infinite timeout) are
 * also recognized for `timeout_nanoseconds'.
 * @return: 1 : Timeout expired.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_acquire_timed)(Dee_shared_lock_t *__restrict self,
                                      __UINT64_TYPE__ timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	uint32_t lockword;
again:
	/* NOTE: If there suddenly were more than UINT_MAX threads trying to acquire the same
	 *       lock  all at the same time, this could overflow. -- But I think that's not a
	 *       thing that could ever happen... */
	if ((lockword = atomic_fetchinc_explicit(&self->sl_lock, __ATOMIC_ACQUIRE)) != 0) {
		int error;
		uint64_t now_microseconds, then_microseconds;
		if unlikely(lockword != 1) {
			/* This can happen if multiple threads try to acquire the lock at the same time.
			 * In  this case, we must normalize the  lock-word back to `state = 2', but only
			 * for as long as the lock itself remains acquired by some-one.
			 *
			 * This code right here is also carefully written such that it always does
			 * the  right thing, no  matter how many  threads execute it concurrently. */
			++lockword;
			while (!atomic_cmpxch_explicit(&self->sl_lock, lockword, 2,
			                               __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
				lockword = atomic_read(&self->sl_lock);
				if unlikely(lockword == 0)
					goto again; /* Lock suddenly become available */
				if unlikely(lockword == 2)
					break; /* Some other thread did the normalize for us! */
			}
		}
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			error = DeeFutex_Wait32(&self->sl_lock, 2);
			if unlikely(error != 0)
				return error;
			goto again;
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		error = DeeFutex_Wait32Timed(&self->sl_lock, 2, timeout_nanoseconds);
		if unlikely(error != 0)
			return error;
		if ((lockword = atomic_fetchinc_explicit(&self->sl_lock, __ATOMIC_ACQUIRE)) != 0) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;

			if unlikely(lockword != 1) {
				/* This can happen if multiple threads try to acquire the lock at the same time.
				 * In  this case, we must normalize the  lock-word back to `state = 2', but only
				 * for as long as the lock itself remains acquired by some-one.
				 *
				 * This code right here is also carefully written such that it always does
				 * the  right thing, no  matter how many  threads execute it concurrently. */
				++lockword;
				while (!atomic_cmpxch_explicit(&self->sl_lock, lockword, 2,
				                               __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
					lockword = atomic_read(&self->sl_lock);
					if unlikely(lockword == 0)
						goto again; /* Lock suddenly become available */
					if unlikely(lockword == 2)
						break; /* Some other thread did the normalize for us! */
				}
			}
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_lock_waitfor_timed)(Dee_shared_lock_t *__restrict self,
                                      __UINT64_TYPE__ timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	unsigned int lockword;
again:
	while ((lockword = atomic_read(&self->sl_lock)) != 0) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (lockword == 1)
			atomic_cmpxch(&self->sl_lock, 1, 2);
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			error = DeeFutex_Wait32(&self->sl_lock, 2);
			if unlikely(error != 0)
				return error;
			goto again;
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		error = DeeFutex_Wait32Timed(&self->sl_lock, 2, timeout_nanoseconds);
		if unlikely(error != 0)
			return error;
		if ((lockword = atomic_read(&self->sl_lock)) != 0) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			if (lockword == 1)
				atomic_cmpxch(&self->sl_lock, 1, 2);
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}






/************************************************************************/
/* Shared r/w-lock (scheduler-level blocking lock)                      */
/************************************************************************/

PUBLIC NONNULL((1)) void
(DCALL Dee_shared_rwlock_end)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	if (self->sl_lock != (uintptr_t)-1) {
		/* Read-lock */
		uintptr_t temp;
		Dee_ASSERTF(self->sl_lock != 0, "No remaining read-locks");
		temp = atomic_decfetch_explicit(&self->sl_lock, __ATOMIC_RELEASE);
		if (temp == 0)
			_Dee_shared_rwlock_wake(self);
	} else {
		/* Write-lock */
		atomic_write(&self->sl_lock, 0);
		_Dee_shared_rwlock_wake(self);
	}
#endif /* !CONFIG_NO_THREADS */
}


/* Blocking acquire/wait-for a given lock.
 * @return: 0 : Success.
 * @return: -1: An exception was thrown. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_read)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	while (!Dee_shared_rwlock_tryread(self)) {
		int error;
		atomic_write(&self->sl_waiting, 1);
		error = DeeFutex_WaitPtr(&self->sl_lock, (uintptr_t)-1);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_write)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		uintptr_t lockword = atomic_read(&self->sl_lock);
		if (lockword == 0) {
			if (atomic_cmpxch_explicit(&self->sl_lock, 0, (uintptr_t)-1,
			                           __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				break;
		} else {
			int error;
			atomic_write(&self->sl_waiting, 1);
			error = DeeFutex_WaitPtr(&self->sl_lock, lockword);
			if unlikely(error != 0)
				return error;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitread)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	while (!Dee_shared_rwlock_canread(self)) {
		int error;
		atomic_write(&self->sl_waiting, 1);
		error = DeeFutex_WaitPtr(&self->sl_lock, (uintptr_t)-1);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitwrite)(Dee_shared_rwlock_t *__restrict self) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		int error;
		uintptr_t lockword = atomic_read(&self->sl_lock);
		if (lockword == 0)
			break;
		atomic_write(&self->sl_waiting, 1);
		error = DeeFutex_WaitPtr(&self->sl_lock, lockword);
		if unlikely(error != 0)
			return error;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_read_timed)(Dee_shared_rwlock_t *__restrict self,
                                     __UINT64_TYPE__ timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	if (!Dee_shared_rwlock_tryread(self)) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (timeout_nanoseconds == (__UINT64_TYPE__)-1) {
do_infinite_timeout:
			return (Dee_shared_rwlock_read)(self);
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		atomic_write(&self->sl_waiting, 1);
		error = DeeFutex_WaitPtrTimed(&self->sl_lock, (uintptr_t)-1, timeout_nanoseconds);
		if unlikely(error != 0)
			return error;
		if (!Dee_shared_rwlock_tryread(self)) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_write_timed)(Dee_shared_rwlock_t *__restrict self,
                                      __UINT64_TYPE__ timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		uintptr_t lockword = atomic_read(&self->sl_lock);
		if (lockword == 0) {
			if (atomic_cmpxch_explicit(&self->sl_lock, 0, (uintptr_t)-1,
			                           __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				break;
		} else {
			uint64_t now_microseconds, then_microseconds;
			int error;
			if (timeout_nanoseconds == (__UINT64_TYPE__)-1) {
do_infinite_timeout:
				return (Dee_shared_rwlock_write)(self);
			}
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
				goto do_infinite_timeout;
do_wait_with_timeout:
			atomic_write(&self->sl_waiting, 1);
			error = DeeFutex_WaitPtr(&self->sl_lock, lockword);
			if unlikely(error != 0)
				return error;
			lockword = atomic_read(&self->sl_lock);
			if (lockword == 0 &&
			    atomic_cmpxch_explicit(&self->sl_lock, 0, (uintptr_t)-1,
			                           __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				break;
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitread_timed)(Dee_shared_rwlock_t *__restrict self,
                                         __UINT64_TYPE__ timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	if (!Dee_shared_rwlock_canread(self)) {
		uint64_t now_microseconds, then_microseconds;
		int error;
		if (timeout_nanoseconds == (__UINT64_TYPE__)-1) {
do_infinite_timeout:
			return (Dee_shared_rwlock_waitread)(self);
		}
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
do_wait_with_timeout:
		atomic_write(&self->sl_waiting, 1);
		error = DeeFutex_WaitPtrTimed(&self->sl_lock, (uintptr_t)-1, timeout_nanoseconds);
		if unlikely(error != 0)
			return error;
		if (!Dee_shared_rwlock_canread(self)) {
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 1; /* Timeout */
			timeout_nanoseconds *= 1000;
			goto do_wait_with_timeout;
		}
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_shared_rwlock_waitwrite_timed)(Dee_shared_rwlock_t *__restrict self,
                                          __UINT64_TYPE__ timeout_nanoseconds) {
#ifdef CONFIG_NO_THREADS
	/* For binary compatibility */
	(void)self;
	(void)timeout_nanoseconds;
	COMPILER_IMPURE();
	return 0;
#else /* CONFIG_NO_THREADS */
	int error;
	uint64_t now_microseconds, then_microseconds;
	uintptr_t lockword = atomic_read(&self->sl_lock);
	if (lockword == 0)
		return 0;
	if (timeout_nanoseconds == (__UINT64_TYPE__)-1) {
do_infinite_timeout:
		return (Dee_shared_rwlock_waitwrite)(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	atomic_write(&self->sl_waiting, 1);
	error = DeeFutex_WaitPtr(&self->sl_lock, lockword);
	if unlikely(error != 0)
		return error;
	lockword = atomic_read(&self->sl_lock);
	if (lockword == 0)
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	goto do_wait_with_timeout;
#endif /* !CONFIG_NO_THREADS */
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_FUTEX_C */
