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
#ifndef GUARD_DEEMON_RUNTIME_THREAD_C
#define GUARD_DEEMON_RUNTIME_THREAD_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-error.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/futex.h>
#include <deemon/util/lock.h>
#include <deemon/util/once.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/sequence/list.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

#define MICROSECONDS_PER_SECOND     UINT32_C(1000000)
#define NANOSECONDS_PER_MICROSECOND UINT16_C(1000)
#define NANOSECONDS_PER_MILLISECOND UINT32_C(1000000)
#define NANOSECONDS_PER_SECOND      UINT32_C(1000000000)

/* Delay to wait before repeating a thread-wake operation.
 *
 * Thread-wake operations need to be repeated until the thread
 * that is being woken acknowledges the wake-up, because of how
 * wake-ups are implemented as sporadic interrupts that will
 * only interrupt an in-progress system call, but not one that
 * hasn't started yet:
 *
 * Thread #1: <about-to-call-read(2)>
 * Thread #2: DeeThread_Wake(<Thread #1>)
 * Thread #1: Enters `DeeThread_SporadicInterruptHandler'
 * Thread #1: Leaves `DeeThread_SporadicInterruptHandler'
 * Thread #1: Calls `read(2)' (only from this point forth will `DeeThread_Wake()' interrupt the system call)
 * Thread #1: Starts blocking
 * Thread #2: After `THREAD_WAKE_DELAY', send another wake to <Thread #1>
 * Thread #1: Returns from `read(2)' with `errno=EINTR'
 * Thread #1: Finally checks for interrupts
 * Thread #1: Clears `Dee_THREAD_STATE_INTERRUPTED'
 * Thread #1: Wakes up <Thread #2> via a futex operation on the thread status
 * Thread #2: Notices that `Dee_THREAD_STATE_INTERRUPTED' is now clear
 * ->> Thread #1 was forced to check for interrupts, even though it didn't receive the initial wake-up
 */
#define THREAD_WAKE_DELAY (NANOSECONDS_PER_MILLISECOND * 10)


/* Figure out how we want to implement threads. */
#undef DeeThread_USE_SINGLE_THREADED
#undef DeeThread_USE_CreateThread
#undef DeeThread_USE_pthread_create
#undef DeeThread_USE_thrd_create
#if 0
/* ... */
#elif defined(CONFIG_NO_THREADS)
#define DeeThread_USE_SINGLE_THREADED
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeThread_USE_CreateThread
#elif defined(CONFIG_HAVE_pthread_create)
#define DeeThread_USE_pthread_create
#elif defined(CONFIG_HAVE_thrd_create)
#define DeeThread_USE_thrd_create
#else /* ... */
#define DeeThread_USE_SINGLE_THREADED
#endif /* !... */

#ifdef DeeThread_USE_CreateThread
#include <Windows.h>
#endif /* DeeThread_USE_CreateThread */

#ifdef DeeThread_USE_pthread_create
#include <pthread.h>
#endif /* DeeThread_USE_pthread_create */

#ifdef DeeThread_USE_thrd_create
#include <threads.h>
#endif /* DeeThread_USE_thrd_create */


#if defined(CONFIG_HAVE_pthread_setname_np_2ARG) && !defined(CONFIG_HAVE_pthread_setname_2ARG)
#define CONFIG_HAVE_pthread_setname_2ARG
#undef pthread_setname
#define pthread_setname pthread_setname_np
#endif /* pthread_setname = pthread_setname_np */

#if defined(CONFIG_HAVE_pthread_setname_np_3ARG) && !defined(CONFIG_HAVE_pthread_setname_3ARG)
#define CONFIG_HAVE_pthread_setname_3ARG
#undef pthread_setname
#define pthread_setname pthread_setname_np
#endif /* pthread_setname = pthread_setname_np */


/* Figure out extra stuff to implement `DeeThread_Wake()' */
#undef DeeThread_Wake_USE_QueueUserAPC__AND__CancelSynchronousIo
#undef DeeThread_Wake_USE_pthread_kill
#undef DeeThread_Wake_USE_pthread_sigqueue
#undef DeeThread_Wake_USE_tgkill
#undef DeeThread_Wake_USE_kill
#ifdef DeeThread_USE_CreateThread
#define DeeThread_Wake_USE_QueueUserAPC__AND__CancelSynchronousIo
#endif /* DeeThread_USE_CreateThread */
#ifdef DeeThread_USE_pthread_create
#ifdef CONFIG_HAVE_pthread_kill
#define DeeThread_Wake_USE_pthread_kill
#elif defined(CONFIG_HAVE_pthread_sigqueue)
#define DeeThread_Wake_USE_pthread_sigqueue
#endif /* CONFIG_HAVE_pthread_sigqueue */
#endif /* DeeThread_USE_pthread_create */
#ifdef Dee_pid_t
#if defined(CONFIG_HAVE_tgkill) && defined(CONFIG_HAVE_getpid)
#define DeeThread_Wake_USE_tgkill
#elif defined(CONFIG_HAVE_kill)
#define DeeThread_Wake_USE_kill
#endif /* ... */
#endif /* Dee_pid_t */

#undef DeeThread_Wake_NEEDS_SIGNAL
#if (defined(DeeThread_Wake_USE_pthread_kill) ||     \
     defined(DeeThread_Wake_USE_pthread_sigqueue) || \
     defined(DeeThread_Wake_USE_tgkill) ||           \
     defined(DeeThread_Wake_USE_kill))
#define DeeThread_Wake_NEEDS_SIGNAL
#endif /* DeeThread_Wake_USE_... */

#ifdef DeeThread_Wake_NEEDS_SIGNAL
#ifdef SIGUSR1
#define DeeThread_Wake_USED_SIGNAL SIGUSR1
#elif defined(SIGUSR2)
#define DeeThread_Wake_USED_SIGNAL SIGUSR2
#elif defined(SIGRTMIN)
#define DeeThread_Wake_USED_SIGNAL SIGRTMIN
#endif /* SIGUSR1 */

/* Figure out how to do the necessary signal setup (`sigaction(2)', `signal(2)', ...). */
#undef DeeThread_SetupSignalHandlers_USE_sigaction
#undef DeeThread_SetupSignalHandlers_USE_bsd_signal
#undef DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal
#ifdef CONFIG_HAVE_sigaction
#define DeeThread_SetupSignalHandlers_USE_sigaction
#elif defined(CONFIG_HAVE_bsd_signal)
#define DeeThread_SetupSignalHandlers_USE_bsd_signal
#elif defined(CONFIG_HAVE_sysv_signal) || defined(CONFIG_HAVE_signal)
#define DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal
#endif /* ... */

#undef DeeThread_SetupSignalHandlers_NEEDED
#if (defined(DeeThread_SetupSignalHandlers_USE_sigaction) ||  \
     defined(DeeThread_SetupSignalHandlers_USE_bsd_signal) || \
     defined(DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal))
#define DeeThread_SetupSignalHandlers_NEEDED
#endif /* ... */
#endif /* DeeThread_Wake_NEEDS_SIGNAL */

#if (defined(DeeThread_Wake_NEEDS_SIGNAL) &&  \
     (!defined(DeeThread_Wake_USED_SIGNAL) || \
      !defined(DeeThread_SetupSignalHandlers_NEEDED)))
/* Unable to establish signal handlers, so won't be able to send signals, either. */
#undef DeeThread_Wake_NEEDS_SIGNAL
#undef DeeThread_Wake_USE_pthread_kill
#undef DeeThread_Wake_USE_pthread_sigqueue
#undef DeeThread_Wake_USE_tgkill
#undef DeeThread_Wake_USE_kill
#endif /* ... */


/* Delete thread-state flags that don't make sense in the single-threaded configuration. */
#ifdef DeeThread_USE_SINGLE_THREADED
#undef Dee_THREAD_STATE_SUSPENDING
#undef Dee_THREAD_STATE_SUSPENDED
#undef Dee_THREAD_STATE_TERMINATED
#undef Dee_THREAD_STATE_STARTING
/*#undef Dee_THREAD_STATE_STARTED*/ /* Always set */
#undef DeeThread_HasCrashed
#undef DeeThread_HasStarted
#define DeeThread_HasCrashed(self) ((self) == DeeThread_Self() && (self)->t_exceptsz > 0)
#define DeeThread_HasStarted(self) 1
#endif /* DeeThread_USE_SINGLE_THREADED */
#ifndef Dee_pid_t
#undef Dee_THREAD_STATE_HASTID
#endif /* !Dee_pid_t */


#undef Dee_THREAD_STATE_HASOSCTX
#if defined(Dee_THREAD_STATE_HASTHREAD) && defined(Dee_THREAD_STATE_HASTID)
#define Dee_THREAD_STATE_HASOSCTX (Dee_THREAD_STATE_HASTHREAD | Dee_THREAD_STATE_HASTID)
#elif defined(Dee_THREAD_STATE_HASTHREAD)
#define Dee_THREAD_STATE_HASOSCTX Dee_THREAD_STATE_HASTHREAD
#elif defined(Dee_THREAD_STATE_HASTID)
#define Dee_THREAD_STATE_HASOSCTX Dee_THREAD_STATE_HASTID
#endif /* ... */
#ifndef Dee_THREAD_STATE_HASOSCTX
#undef Dee_THREAD_STATE_DETACHING
#endif /* !Dee_THREAD_STATE_HASOSCTX */



DECL_BEGIN


/* >> Dee_pid_t DeeThread_GetCurrentTid(void); */
#undef DeeThread_GetCurrentTid
#ifdef Dee_pid_t
#ifdef CONFIG_HOST_WINDOWS
#define DeeThread_GetCurrentTid() (Dee_pid_t)GetCurrentThreadId()
#elif defined(CONFIG_HAVE_gettid)
#define DeeThread_GetCurrentTid() gettid()
#elif defined(CONFIG_HAVE_pthread_gettid_np) && defined(CONFIG_HAVE_pthread_self)
#define DeeThread_GetCurrentTid() pthread_gettid_np(pthread_self())
#elif defined(CONFIG_HAVE_syscall) && defined(SYS_gettid)
#define DeeThread_GetCurrentTid() (Dee_pid_t)syscall(SYS_gettid)
#elif defined(CONFIG_HAVE_syscall) && defined(__NR_gettid)
#define DeeThread_GetCurrentTid() (Dee_pid_t)syscall(__NR_gettid)
#endif /* ... */
#endif /* Dee_pid_t */


#ifndef DeeThread_USE_SINGLE_THREADED
#if defined(CONFIG_HOST_WINDOWS) && defined(_MSC_VER)
#ifdef _PREFAST_
#pragma warning(push)
#pragma warning(disable: 6320)
#pragma warning(disable: 6322)
#endif /* _PREFAST_ */
#pragma pack(push, 8)
#define DeeThread_SetName__406D1388 DeeThread_SetName__406D1388
PRIVATE NONNULL((1)) void DCALL
DeeThread_SetName__406D1388(char const *__restrict name) {
	typedef struct THREADNAME_INFO {
		DWORD dwType;     // Must be 0x1000.
		LPCSTR szName;    // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags;    // Reserved for future use, must be zero.
	} THREADNAME_INFO;
	THREADNAME_INFO info;
	DBG_ALIGNMENT_DISABLE();
	info.dwType     = 0x1000;
	info.szName     = name;
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags    = 0;
	__try {
		RaiseException(0x406D1388, 0,
		               sizeof(info) / sizeof(void *),
		               (ULONG_PTR const *)&info);
	} __except (1) {
	}
	DBG_ALIGNMENT_ENABLE();
}
#pragma pack(pop)
#ifdef _PREFAST_
#pragma warning(pop)
#endif /* _PREFAST_ */
#endif /* CONFIG_HOST_WINDOWS && _MSC_VER */

/* >> void DeeThread_SetName(char const *name); */
#ifdef CONFIG_HOST_WINDOWS
PRIVATE WUNUSED NONNULL((1)) uint16_t *DCALL
try_utf8_to_wide(char const *__restrict name) {
	uint16_t *result, *dst;
	size_t namelen;
	uint32_t ch;
	namelen = strlen(name);
	result  = (uint16_t *)Dee_TryMallocc(namelen + 1, sizeof(uint16_t));
	if unlikely(!result)
		return NULL;
	dst = result;
	while ((ch = unicode_readutf8(&name)) != 0) {
		if likely(ch <= 0xffff && (ch < 0xd800 || ch > 0xdfff)) {
			*dst++ = (uint16_t)ch;
		} else {
			ch -= 0x10000;
			*dst++ = 0xd800 + (uint16_t)(ch >> 10);
			*dst++ = 0xdc00 + (uint16_t)(ch & 0x3ff);
		}
	}
	*dst = (uint16_t)'\0';
	return result;
}

PRIVATE NONNULL((1)) bool DCALL
DeeThread_SetName__SetThreadDescription(char const *__restrict name) {
	typedef HRESULT (WINAPI *LPSETTHREADDESCRIPTION)(HANDLE hThread, PCWSTR lpThreadDescription);
	PRIVATE WCHAR const wKernelBase_dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', 'B', 'a', 's', 'e', '.', 'd', 'l', 'l', 0 };
	PRIVATE LPSETTHREADDESCRIPTION pdyn_SetThreadDescription = NULL;
	HRESULT hr;
	PWSTR lpwName;
	Dee_ONCE({
		HMODULE hm;
		DBG_ALIGNMENT_DISABLE();
		hm = LoadLibraryW(wKernelBase_dll);
		if (hm) {
			pdyn_SetThreadDescription = (LPSETTHREADDESCRIPTION)GetProcAddress(hm, "SetThreadDescription");
			if (!pdyn_SetThreadDescription)
				(void)FreeLibrary(hm);
		}
		DBG_ALIGNMENT_ENABLE();
	});
	if (!pdyn_SetThreadDescription)
		return false;
	lpwName = (PWSTR)try_utf8_to_wide(name);
	if unlikely(!lpwName)
		return false;
	hr = (*pdyn_SetThreadDescription)(GetCurrentThread(), lpwName);
	Dee_Free(lpwName);
	return !FAILED(hr);
}

#define DeeThread_SetName DeeThread_SetName
PRIVATE NONNULL((1)) void DCALL
DeeThread_SetName(char const *__restrict name) {
	if (!DeeThread_SetName__SetThreadDescription(name)) {
#ifdef DeeThread_SetName__406D1388
		DeeThread_SetName__406D1388(name);
#endif /* DeeThread_SetName__406D1388 */
	}
}
#elif defined(DeeThread_USE_pthread_create) && defined(CONFIG_HAVE_pthread_self) && defined(CONFIG_HAVE_pthread_setname_np_2ARG)
#define DeeThread_SetName(name) (void)pthread_setname_np(pthread_self(), name)
#elif defined(DeeThread_USE_pthread_create) && defined(CONFIG_HAVE_pthread_self) && defined(CONFIG_HAVE_pthread_setname_np_23RG)
#define DeeThread_SetName(name) (void)pthread_setname_np(pthread_self(), name, strlen(name))
#endif /* ... */
#endif /* !DeeThread_USE_SINGLE_THREADED */



/* Figure out how to implement `DeeThread_GetTimeMicroSeconds()' */
#undef DeeThread_GetTimeMicroSeconds_USE_QueryPerformanceCounter__OR__GetTickCount
#undef DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_MONOTONIC
#undef DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_REALTIME
#undef DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_MONOTONIC
#undef DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_REALTIME
#undef DeeThread_GetTimeMicroSeconds_USE_gettimeofday64
#undef DeeThread_GetTimeMicroSeconds_USE_gettimeofday
#undef DeeThread_GetTimeMicroSeconds_USE_time
#undef DeeThread_GetTimeMicroSeconds_USE_COUNTER
#if 0
/* ... */
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeThread_GetTimeMicroSeconds_USE_QueryPerformanceCounter__OR__GetTickCount
#elif defined(CONFIG_HAVE_clock_gettime64) && defined(CONFIG_HAVE_CLOCK_MONOTONIC)
#define DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_MONOTONIC
#elif defined(CONFIG_HAVE_clock_gettime64) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_REALTIME
#elif defined(CONFIG_HAVE_clock_gettime) && defined(CONFIG_HAVE_CLOCK_MONOTONIC)
#define DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_MONOTONIC
#elif defined(CONFIG_HAVE_clock_gettime) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_REALTIME
#elif defined(CONFIG_HAVE_gettimeofday64)
#define DeeThread_GetTimeMicroSeconds_USE_gettimeofday64
#elif defined(CONFIG_HAVE_gettimeofday)
#define DeeThread_GetTimeMicroSeconds_USE_gettimeofday
#elif defined(CONFIG_HAVE_time64)
#undef DeeThread_GetTimeMicroSeconds_USE_time64
#elif defined(CONFIG_HAVE_time)
#undef DeeThread_GetTimeMicroSeconds_USE_time
#else /* ... */
#undef DeeThread_GetTimeMicroSeconds_USE_COUNTER
#endif /* !... */


/* Get the current time (offset from some undefined point) in microseconds. */
PUBLIC WUNUSED uint64_t
(DCALL DeeThread_GetTimeMicroSeconds)(void) {
#ifdef DeeThread_GetTimeMicroSeconds_USE_QueryPerformanceCounter__OR__GetTickCount
	static uint64_t performance_freq = 0;
	static uint64_t performance_freq_div_1000000 = 0;
	static uint64_t performance_1000000_div_freq = 0;
	static WCHAR const wKernel32[] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
	static ULONGLONG (WINAPI *lp_GetTickCount64)(void) = NULL;
	uint64_t result;
	if (performance_freq == 0) {
		uint64_t new_freq;
		DBG_ALIGNMENT_DISABLE();
		if unlikely(!QueryPerformanceFrequency((LARGE_INTEGER *)&new_freq))
			goto do_tickcount;
		DBG_ALIGNMENT_ENABLE();
		if unlikely(!new_freq)
			new_freq = 1;
		performance_freq             = new_freq;
		performance_freq_div_1000000 = new_freq / 1000000;
		performance_1000000_div_freq = 1000000 / new_freq;
	}
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!QueryPerformanceCounter((LARGE_INTEGER *)&result)) {
do_tickcount:
		if (!lp_GetTickCount64) {
			*(FARPROC *)&lp_GetTickCount64 = GetProcAddress(GetModuleHandleW(wKernel32), "GetTickCount64");
			if (!lp_GetTickCount64) {
				result = (uint64_t)GetTickCount();
				DBG_ALIGNMENT_ENABLE();
				return result * 1000;
			}
		}
		result = (*lp_GetTickCount64)();
		DBG_ALIGNMENT_ENABLE();
		return result * 1000;
	}
	DBG_ALIGNMENT_ENABLE();
#if 1
	if (!OVERFLOW_UMUL(result, 1000000, &result)) {
		result /= performance_freq;
	} else
#endif
	if (performance_freq >= 1000000) {
		result /= performance_freq_div_1000000;
	} else {
		result *= performance_1000000_div_freq;
	}
	return result;
#endif /* DeeThread_GetTimeMicroSeconds_USE_QueryPerformanceCounter__OR__GetTickCount */

#if (defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_MONOTONIC) || \
     defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_REALTIME) ||  \
     defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_MONOTONIC) ||   \
     defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_REALTIME) ||    \
     defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday64) ||                  \
     defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday))
#undef LOCAL_use_time64
#undef LOCAL_need_baseline
#if (defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_MONOTONIC) || \
     defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_REALTIME) ||  \
     defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday64))
#define LOCAL_use_time64
#endif /* ... */
#if (!defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_REALTIME) && \
     !defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_REALTIME))
#define LOCAL_need_baseline
#endif /* ... */
#ifdef LOCAL_use_time64
#define LOCAL_struct_timespec struct timespec64
#define LOCAL_struct_timeval  struct timeval64
#else /* LOCAL_use_time64 */
#define LOCAL_struct_timespec struct timespec
#define LOCAL_struct_timeval  struct timeval
#endif /* !LOCAL_use_time64 */
#if (defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday) || \
     defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday64))
#define LOCAL_struct_timespec_OR_timeval_IS_timeval
#define LOCAL_struct_timespec_OR_timeval LOCAL_struct_timeval
#else /* ... */
#define LOCAL_struct_timespec_OR_timeval_IS_timespec
#define LOCAL_struct_timespec_OR_timeval LOCAL_struct_timespec
#endif /* !... */
#ifdef LOCAL_need_baseline
	static uint64_t baseline_tv_sec  = 0;
	static uint32_t baseline_tv_usec = 0;
#endif /* LOCAL_need_baseline */
	LOCAL_struct_timespec_OR_timeval ts;
	uint64_t used_tv_sec;
	uint32_t used_tv_usec;

	/* Retrieve the current time. */
#if defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_MONOTONIC)
	(void)clock_gettime64(CLOCK_MONOTONIC, &ts);
#elif defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_MONOTONIC)
	(void)clock_gettime(CLOCK_MONOTONIC, &ts);
#else /* ... */
#if defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime64_CLOCK_REALTIME)
	if (clock_gettime64(CLOCK_REALTIME, &ts) != 0)
#elif defined(DeeThread_GetTimeMicroSeconds_USE_clock_gettime_CLOCK_REALTIME)
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
#elif defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday64)
	if (gettimeofday64(&ts, NULL) != 0)
#elif defined(DeeThread_GetTimeMicroSeconds_USE_gettimeofday)
	if (gettimeofday(&ts, NULL) != 0)
#endif
	{
		/* Fallback error handling */
#if defined(LOCAL_use_time64) && defined(CONFIG_HAVE_time64)
		ts.tv_sec = time64(NULL);
#elif defined(CONFIG_HAVE_time)
		ts.tv_sec = time(NULL);
#elif defined(CONFIG_HAVE_time64)
		ts.tv_sec = time64(NULL);
#else /* ... */
		ts.tv_sec = 0;
#endif /* !... */
#ifdef LOCAL_struct_timespec_OR_timeval_IS_timespec
		ts.tv_nsec = 0;
#endif /* LOCAL_struct_timespec_OR_timeval_IS_timespec */
#ifdef LOCAL_struct_timespec_OR_timeval_IS_timeval
		ts.tv_usec = 0;
#endif /* LOCAL_struct_timespec_OR_timeval_IS_timeval */
	}
#endif /* !... */

	/* Load the effective current time. */
	used_tv_sec  = (uint64_t)ts.tv_sec;
#ifdef LOCAL_struct_timespec_OR_timeval_IS_timespec
	used_tv_usec = (uint32_t)(ts.tv_nsec / 1000);
#endif /* LOCAL_struct_timespec_OR_timeval_IS_timespec */
#ifdef LOCAL_struct_timespec_OR_timeval_IS_timeval
	used_tv_usec = (uint32_t)ts.tv_usec;
#endif /* LOCAL_struct_timespec_OR_timeval_IS_timeval */

	/* Special case for first call when we use a baseline */
#ifdef LOCAL_need_baseline
	if unlikely(baseline_tv_sec == 0) {
		baseline_tv_sec  = used_tv_sec;
		baseline_tv_usec = used_tv_usec;
		return 0;
	}
	used_tv_sec -= baseline_tv_sec;
	if (OVERFLOW_USUB(used_tv_usec, baseline_tv_usec, &used_tv_usec)) {
		--used_tv_sec;
		used_tv_usec += MICROSECONDS_PER_SECOND;
	}
#endif /* LOCAL_need_baseline */

	return used_tv_usec + (used_tv_sec * MICROSECONDS_PER_SECOND);
#endif /* ... */

#ifdef DeeThread_GetTimeMicroSeconds_USE_time64
	time64_t t = time64(NULL);
	return (uint64_t)t * MICROSECONDS_PER_SECOND;
#endif /* DeeThread_GetTimeMicroSeconds_USE_time64 */

#ifdef DeeThread_GetTimeMicroSeconds_USE_time
	time_t t = time(NULL);
	return (uint64_t)t * MICROSECONDS_PER_SECOND;
#endif /* DeeThread_GetTimeMicroSeconds_USE_time */

#ifdef DeeThread_GetTimeMicroSeconds_USE_COUNTER
	static uint64_t counter = 0;
	return atomic_fetchinc(&counter);
#endif /* DeeThread_GetTimeMicroSeconds_USE_COUNTER */
}


/* Figure out how to implement `DeeThread_Sleep()' */
#undef DeeThread_Sleep_USE_SleepEx
#undef DeeThread_Sleep_USE_nanosleep
#undef DeeThread_Sleep_USE_usleep
#undef DeeThread_Sleep_USE_select
#undef DeeThread_Sleep_USE_pselect
#undef DeeThread_Sleep_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define DeeThread_Sleep_USE_SleepEx
#elif defined(CONFIG_HAVE_nanosleep64) || defined(CONFIG_HAVE_nanosleep)
#define DeeThread_Sleep_USE_nanosleep
#elif defined(CONFIG_HAVE_usleep)
#define DeeThread_Sleep_USE_usleep
#elif defined(CONFIG_HAVE_select) || defined(CONFIG_HAVE_select64)
#define DeeThread_Sleep_USE_select
#elif defined(CONFIG_HAVE_pselect) || defined(CONFIG_HAVE_pselect64)
#define DeeThread_Sleep_USE_pselect
#else /* ... */
#define DeeThread_Sleep_USE_STUB
#endif /* !... */

#ifdef DeeThread_Sleep_USE_usleep
#ifndef CONFIG_HAVE_useconds_t
#define useconds_t uint64_t
#endif /* !CONFIG_HAVE_useconds_t */
#endif /* DeeThread_Sleep_USE_usleep */

/* Sleep for the specified number of microseconds (1/1000000 seconds). */
PUBLIC WUNUSED int (DCALL DeeThread_Sleep)(uint64_t microseconds) {
	/* TODO: Change this function to use nano-seconds instead! */
#ifdef DeeThread_Sleep_USE_SleepEx
	uint64_t end_time;
	end_time = DeeThread_GetTimeMicroSeconds() + microseconds;
	if (DeeThread_CheckInterrupt())
		goto err;
again:
	/* XXX: More precision? */
	DBG_ALIGNMENT_DISABLE();
	if (SleepEx((DWORD)(microseconds / 1000), TRUE)) {
		uint64_t now;
		DBG_ALIGNMENT_ENABLE();
		now = DeeThread_GetTimeMicroSeconds();
		if (DeeThread_CheckInterrupt())
			goto err;
		/* Continue sleeping for the remainder of the given time. */
		if (now < end_time) {
			microseconds = end_time - now;
			goto again;
		}
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
#endif /* DeeThread_Sleep_USE_SleepEx */

#ifdef DeeThread_Sleep_USE_nanosleep
#ifdef CONFIG_HAVE_nanosleep64
	struct timespec64 sleep_time, rem;
	sleep_time.tv_sec  = (time64_t)(microseconds / 1000000);
#else /* CONFIG_HAVE_nanosleep64 */
	struct timespec sleep_time, rem;
	sleep_time.tv_sec  = (time_t)(microseconds / 1000000);
#endif /* !CONFIG_HAVE_nanosleep64 */
	sleep_time.tv_nsec = (long)(microseconds % 1000000) * 1000;
again:
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_nanosleep64
	if (nanosleep64(&sleep_time, &rem))
#else /* CONFIG_HAVE_nanosleep64 */
	if (nanosleep(&sleep_time, &rem))
#endif /* !CONFIG_HAVE_nanosleep64 */
	{
		DBG_ALIGNMENT_ENABLE();
		if (DeeThread_CheckInterrupt())
			goto err;
#ifdef EINTR
		if (DeeSystem_GetErrno() == EINTR) {
			memcpy(&sleep_time, &rem, sizeof(struct timespec));
			goto again;
		}
#endif /* EINTR */
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
#endif /* DeeThread_Sleep_USE_nanosleep */

#ifdef DeeThread_Sleep_USE_usleep
again:
	DBG_ALIGNMENT_DISABLE();
	if (usleep((useconds_t)microseconds)) {
		DBG_ALIGNMENT_ENABLE();
		if (DeeThread_CheckInterrupt())
			goto err;
#ifdef EINTR
		if (DeeSystem_GetErrno() == EINTR)
			goto again;
#endif /* EINTR */
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
#endif /* DeeThread_Sleep_USE_usleep */

#ifdef DeeThread_Sleep_USE_select
#ifdef CONFIG_HAVE_select64
	struct timeval64 tv;
#else /* CONFIG_HAVE_select64 */
	struct timeval tv;
#endif /* !CONFIG_HAVE_select64 */
	tv.tv_sec  = microseconds / MICROSECONDS_PER_SECOND;
	tv.tv_usec = microseconds % MICROSECONDS_PER_SECOND;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_select64
	if (select64(0, NULL, NULL, NULL, &tv))
#else /* CONFIG_HAVE_select64 */
	if (select(0, NULL, NULL, NULL, &tv))
#endif /* !CONFIG_HAVE_select64 */
	{
		DBG_ALIGNMENT_ENABLE();
		if (DeeThread_CheckInterrupt())
			goto err;
#ifdef EINTR
		if (DeeSystem_GetErrno() == EINTR)
			goto again;
#endif /* EINTR */
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
#endif /* DeeThread_Sleep_USE_select */

#ifdef DeeThread_Sleep_USE_pselect
#ifdef CONFIG_HAVE_pselect64
	struct timespec64 ts;
#else /* CONFIG_HAVE_pselect64 */
	struct timespec ts;
#endif /* !CONFIG_HAVE_pselect64 */
	ts.tv_sec  = microseconds / MICROSECONDS_PER_SECOND;
	ts.tv_nsec = (microseconds % MICROSECONDS_PER_SECOND) * 1000;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_pselect64
	if (pselect64(0, NULL, NULL, NULL, &ts, NULL))
#else /* CONFIG_HAVE_pselect64 */
	if (pselect(0, NULL, NULL, NULL, &ts, NULL))
#endif /* !CONFIG_HAVE_pselect64 */
	{
		DBG_ALIGNMENT_ENABLE();
		if (DeeThread_CheckInterrupt())
			goto err;
#ifdef EINTR
		if (DeeSystem_GetErrno() == EINTR)
			goto again;
#endif /* EINTR */
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
#endif /* DeeThread_Sleep_USE_pselect */

#ifdef DeeThread_Sleep_USE_STUB
	if (DeeThread_CheckInterrupt())
		goto err;
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "The host does not implement a way of sleeping");
err:
	return -1;
#endif /* DeeThread_Sleep_USE_STUB */
}

PUBLIC void DCALL
DeeThread_SleepNoInt(uint64_t microseconds) {
#ifdef DeeThread_Sleep_USE_SleepEx
	SleepEx((DWORD)(microseconds / 1000), TRUE);
#endif /* DeeThread_Sleep_USE_SleepEx */

#ifdef DeeThread_Sleep_USE_nanosleep
#ifdef CONFIG_HAVE_nanosleep64
	struct timespec64 sleep_time;
	sleep_time.tv_sec  = (time64_t)(microseconds / 1000000);
#else /* CONFIG_HAVE_nanosleep64 */
	struct timespec sleep_time;
	sleep_time.tv_sec  = (time_t)(microseconds / 1000000);
#endif /* !CONFIG_HAVE_nanosleep64 */
	sleep_time.tv_nsec = (long)(microseconds % 1000000) * 1000;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_nanosleep64
	nanosleep64(&sleep_time, NULL);
#else /* CONFIG_HAVE_nanosleep64 */
	nanosleep(&sleep_time, NULL);
#endif /* !CONFIG_HAVE_nanosleep64 */
	DBG_ALIGNMENT_ENABLE();
#endif /* DeeThread_Sleep_USE_nanosleep */

#ifdef DeeThread_Sleep_USE_usleep
	DBG_ALIGNMENT_DISABLE();
	usleep((useconds_t)microseconds);
	DBG_ALIGNMENT_ENABLE();
#endif /* DeeThread_Sleep_USE_usleep */

#ifdef DeeThread_Sleep_USE_select
#ifdef CONFIG_HAVE_select64
	struct timeval64 tv;
#else /* CONFIG_HAVE_select64 */
	struct timeval tv;
#endif /* !CONFIG_HAVE_select64 */
	tv.tv_sec  = microseconds / MICROSECONDS_PER_SECOND;
	tv.tv_usec = microseconds % MICROSECONDS_PER_SECOND;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_select64
	select64(0, NULL, NULL, NULL, &tv);
#else /* CONFIG_HAVE_select64 */
	select(0, NULL, NULL, NULL, &tv);
#endif /* !CONFIG_HAVE_select64 */
	DBG_ALIGNMENT_ENABLE();
#endif /* DeeThread_Sleep_USE_select */

#ifdef DeeThread_Sleep_USE_pselect
#ifdef CONFIG_HAVE_pselect64
	struct timespec64 ts;
#else /* CONFIG_HAVE_pselect64 */
	struct timespec ts;
#endif /* !CONFIG_HAVE_pselect64 */
	ts.tv_sec  = microseconds / MICROSECONDS_PER_SECOND;
	ts.tv_nsec = (microseconds % MICROSECONDS_PER_SECOND) * 1000;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_pselect64
	pselect64(0, NULL, NULL, NULL, &ts, NULL);
#else /* CONFIG_HAVE_pselect64 */
	pselect(0, NULL, NULL, NULL, &ts, NULL);
#endif /* !CONFIG_HAVE_pselect64 */
	DBG_ALIGNMENT_ENABLE();
#endif /* DeeThread_Sleep_USE_pselect */

#ifdef DeeThread_Sleep_USE_STUB
	(void)microseconds;
#endif /* DeeThread_Sleep_USE_STUB */
}









/* Library hooks for implementing thread-local storage. */
#define fini_tls_data(data) (*_DeeThread_TlsCallbacks.tc_fini)(data)
PRIVATE void DCALL
stub_tc_fini(void *__restrict UNUSED(data)) {
}

PUBLIC struct tls_callback_hooks _DeeThread_TlsCallbacks = {
	/* .tc_fini  = */ &stub_tc_fini,
};



PRIVATE DEFINE_STRING(main_thread_name, "MainThread");
PUBLIC uint16_t DeeExec_StackLimit = DEE_CONFIG_DEFAULT_STACK_LIMIT;


PRIVATE struct deep_assoc_entry empty_deep_assoc[] = {
	{ NULL, NULL }
};


PRIVATE NONNULL((1)) bool DCALL
deepassoc_rehash(DeeThreadObject *__restrict self) {
	struct deep_assoc_entry *new_vector, *iter, *end;
	size_t new_mask;
	new_mask = self->t_deepassoc.da_mask;
	new_mask = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 64 - 1; /* Start out bigger than 2. */
	ASSERT(self->t_deepassoc.da_used < new_mask);
	new_vector = (struct deep_assoc_entry *)Dee_TryCallocc(new_mask + 1, sizeof(struct deep_assoc_entry));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->t_deepassoc.da_list == empty_deep_assoc) == (self->t_deepassoc.da_used == 0));
	ASSERT((self->t_deepassoc.da_list == empty_deep_assoc) == (self->t_deepassoc.da_mask == 0));
	if (self->t_deepassoc.da_list != empty_deep_assoc) {

		/* Re-insert all existing items into the new table vector. */
		end = (iter = self->t_deepassoc.da_list) + (self->t_deepassoc.da_mask + 1);
		for (; iter < end; ++iter) {
			struct deep_assoc_entry *item;
			dhash_t i, perturb;

			/* Skip NULL entries. */
			if (!iter->de_old)
				continue;
			perturb = i = (Dee_HashPointer(iter->de_old) ^
			               Dee_HashPointer(Dee_TYPE(iter->de_new))) &
			              new_mask;
			for (;; DEEPASSOC_HASHNX(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->de_old)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			memcpy(item, iter, sizeof(struct deep_assoc_entry));
		}
		Dee_Free(self->t_deepassoc.da_list);
	}
	self->t_deepassoc.da_mask = new_mask;
	self->t_deepassoc.da_list = new_vector;
	return true;
}



/* Implementation detail required to implement recursive deepcopy.
 * To see how this function must be used, look at the documentation for `tp_deepload'
 * WARNING: THIS FUNCTION MUST NOT BE CALLED BY THE IMPLEMENTING
 *          TYPE WHEN `tp_deepload' IS BEING IMPLEMENTED! */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL Dee_DeepCopyAddAssoc)(DeeObject *new_object,
                             DeeObject *old_object) {
	size_t mask;
	dhash_t i, perturb, hash;
	DeeThreadObject *self = DeeThread_Self();
	ASSERT_OBJECT(old_object);
	ASSERT_OBJECT(new_object);

	/* Make sure that the type of the new object is equal to, or a base of that of the old object,
	 * as if this wasn't the case in all situations, then `old_object' could never have been used to
	 * construct `new_object' as a copy.
	 * HINT: This also further ensures a valid pre-initialization (if that wasn't asserted enough already).
	 * Note however that the other way around may not necessarily be true, as constructing
	 * a deep-copy of a super-class `A' from an instances of `B' derived from `A' will produce
	 * an association from an instance of `B' to an instance of `A'. */
	ASSERT(DeeObject_InstanceOf(old_object, Dee_TYPE(new_object)));

	hash = (Dee_HashPointer(old_object) ^
	        Dee_HashPointer(Dee_TYPE(new_object)));
again:
	mask    = self->t_deepassoc.da_mask;
	perturb = i = hash & mask;
	for (;; DEEPASSOC_HASHNX(i, perturb)) {
		struct deep_assoc_entry *item = &self->t_deepassoc.da_list[i & mask];
		if (!item->de_old) {
			if (self->t_deepassoc.da_used + 1 >= self->t_deepassoc.da_mask)
				break; /* Rehash the table and try again. */

			/* Not found. - Use this empty slot. */
			item->de_old = old_object;
			item->de_new = new_object;
			Dee_Incref(old_object);
			Dee_Incref(new_object);
			++self->t_deepassoc.da_used;

			/* Try to keep the table vector big at least twice as big as the element count. */
			if (self->t_deepassoc.da_used * 2 > self->t_deepassoc.da_mask)
				deepassoc_rehash(self);
			return 0;
		}

		/* Check for illegal duplicate entries. */
		ASSERT_OBJECT(item->de_new);

		/* Check if this association was already made */
		if (item->de_old == old_object &&
		    Dee_TYPE(item->de_new) == Dee_TYPE(new_object))
			return 0;
	}

	/* Rehash the table and try again. */
	if (deepassoc_rehash(self))
		goto again;

	/* If that failed, collect memory. */
	if (Dee_CollectMemory(1))
		goto again;

	/* If that failed, we've failed... */
	return -1;
}

/* Lookup a GC association of `old_object', who's
 * new object is an exact instance of `new_type' */
INTERN WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
deepcopy_lookup(DeeThreadObject *thread_self, DeeObject *old_object,
                DeeTypeObject *new_type) {
	uintptr_t i, perturb;
	uintptr_t hash = (Dee_HashPointer(old_object) ^
	                  Dee_HashPointer(new_type));
	perturb = i = DEEPASSOC_HASHST(&thread_self->t_deepassoc, hash);
	for (;; DEEPASSOC_HASHNX(i, perturb)) {
		struct deep_assoc_entry *item;
		item = DEEPASSOC_HASHIT(&thread_self->t_deepassoc, i);
		if (item->de_old != old_object) {
			if (!item->de_old)
				break;
			continue;
		}
		if unlikely(Dee_TYPE(item->de_new) != new_type)
			continue;
		return item->de_new;
	}
	return NULL;
}

INTERN NONNULL((1)) void DCALL
deepcopy_clear(DeeThreadObject *__restrict thread_self) {
	/* NOTE: Everything here is synchronized by lock(PRIVATE(DeeThread_Self())) */
	struct deep_assoc_entry *begin;
	size_t mask;
	struct deep_assoc_entry *iter, *end;
	begin = thread_self->t_deepassoc.da_list;
	mask  = thread_self->t_deepassoc.da_mask;
	thread_self->t_deepassoc.da_list = empty_deep_assoc;
	thread_self->t_deepassoc.da_mask = 0;
	thread_self->t_deepassoc.da_used = 0;
	end = (iter = begin) + (mask + 1);

	/* Go through and clear out all the generated mappings. */
	for (; iter < end; ++iter) {
		if (!iter->de_old)
			continue;
		Dee_Decref(iter->de_old);
		Dee_Decref(iter->de_new);
		iter->de_old = NULL;
	}
	if (thread_self->t_deepassoc.da_list == empty_deep_assoc && mask <= 0xff) {
		/* Return the pre-allocated (but empty) map to the thread.
		 * NOTE: So-as not to clobber member, only do this for small masks. */
		thread_self->t_deepassoc.da_list = begin;
		thread_self->t_deepassoc.da_mask = mask;
	} else {
		/* Some destructor call initiated another deepcopy.
		 * -> Can't replace the existing map with the new one. */
		Dee_Free(begin);
	}
}


struct os_thread_object {
	DeeThreadObject ot_thread;     /* Underlying thread */
#ifdef Dee_pid_t
	Dee_pid_t       ot_tid;        /* [valid_if(Dee_THREAD_STATE_HASTID)] */
#endif /* Dee_pid_t */

#ifndef DeeThread_USE_SINGLE_THREADED
	/* OS-specific thread data */
#ifdef DeeThread_USE_CreateThread
	HANDLE          ot_hThread;    /* [valid_if(Dee_THREAD_STATE_HASTHREAD)] */
#endif /* DeeThread_USE_CreateThread */

#ifdef DeeThread_USE_pthread_create
	pthread_t       ot_pthread;    /* [valid_if(Dee_THREAD_STATE_HASTHREAD)] */
#endif /* DeeThread_USE_pthread_create */

#ifdef DeeThread_USE_thrd_create
	thrd_t          ot_thrd;       /* [valid_if(Dee_THREAD_STATE_HASTHREAD)] */
#endif /* DeeThread_USE_thrd_create */
#endif /* !DeeThread_USE_SINGLE_THREADED */
};

typedef struct os_thread_object DeeOSThreadObject;
LIST_HEAD(thread_object_list, thread_object);
#define DeeThread_AsOSThread(self) COMPILER_CONTAINER_OF(self, DeeOSThreadObject, ot_thread)
#ifndef DeeThread_USE_SINGLE_THREADED
#define DeeThread_GetHThread(self) DeeThread_AsOSThread(self)->ot_hThread
#define DeeThread_GetPThread(self) DeeThread_AsOSThread(self)->ot_pthread
#define DeeThread_GetThrd(self)    DeeThread_AsOSThread(self)->ot_thrd
#endif /* !DeeThread_USE_SINGLE_THREADED */

/* Do the necessary system calls to detach the thread's handle */
#ifdef DeeThread_USE_CreateThread
#define DeeThread_Detach_system_impl(self) (void)CloseHandle(DeeThread_GetHThread(self))
#elif defined(DeeThread_USE_pthread_create)
#ifdef CONFIG_HAVE_pthread_detach
#define DeeThread_Detach_system_impl(self) (void)pthread_detach(DeeThread_GetPThread(self))
#endif /* !CONFIG_HAVE_pthread_detach */
#ifdef CONFIG_HAVE_pthread_join
#define DeeThread_Join_system_impl(self) \
	do { void *_th_res; pthread_join(DeeThread_GetPThread(self), &_th_res); } __WHILE0
#endif /* !CONFIG_HAVE_pthread_join */
#elif defined(DeeThread_USE_thrd_create)
#ifdef CONFIG_HAVE_thrd_detach
#define DeeThread_Detach_system_impl(self) (void)thrd_detach(DeeThread_GetThrd(self))
#endif /* !CONFIG_HAVE_thrd_detach */
#ifdef CONFIG_HAVE_thrd_join
#define DeeThread_Join_system_impl(self) \
	do { void *_th_res; thrd_join(DeeThread_GetThrd(self), &_th_res); } __WHILE0
#endif /* !CONFIG_HAVE_thrd_join */
#endif /* ... */


#ifdef DeeThread_USE_CreateThread
#define DeeThread_HAVE_GetCurrentXThread
#define DeeThread_GetCurrentHThread DeeThread_GetCurrentHThread
PRIVATE HANDLE DCALL DeeThread_GetCurrentHThread(void) {
	HANDLE hResult;
	DBG_ALIGNMENT_DISABLE();
	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
	                     GetCurrentProcess(), &hResult,
	                     0, TRUE, DUPLICATE_SAME_ACCESS))
		hResult = GetCurrentThread();
	DBG_ALIGNMENT_ENABLE();
	return hResult;
}
#endif /* DeeThread_USE_CreateThread */
#if defined(DeeThread_USE_pthread_create) && defined(CONFIG_HAVE_pthread_self)
#define DeeThread_HAVE_GetCurrentXThread
#define DeeThread_GetCurrentPThread() pthread_self()
#endif /* DeeThread_USE_pthread_create && CONFIG_HAVE_pthread_self */
#if defined(DeeThread_USE_thrd_create) && defined(CONFIG_HAVE_thrd_current)
#define DeeThread_HAVE_GetCurrentXThread
#define DeeThread_GetCurrentThrd() thrd_current()
#endif /* DeeThread_USE_thrd_create && CONFIG_HAVE_thrd_current */


/* [0..n][lock(thread_list_lock)] List of running threads, except for the main thread */
#ifndef DeeThread_USE_SINGLE_THREADED
#define thread_list (*(struct thread_object_list *)&DeeThread_Main.ot_thread.t_global.le_next)
#endif /* !DeeThread_USE_SINGLE_THREADED */

/* Lock for the global list of threads */
#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE Dee_shared_lock_t thread_list_lock = DEE_SHARED_LOCK_INIT;
#define thread_list_lock_available()     Dee_shared_lock_available(&thread_list_lock)
#define thread_list_lock_acquired()      Dee_shared_lock_acquired(&thread_list_lock)
#define thread_list_lock_tryacquire()    Dee_shared_lock_tryacquire(&thread_list_lock)
#define thread_list_lock_acquire()       Dee_shared_lock_acquire(&thread_list_lock)
#define thread_list_lock_acquire_noint() Dee_shared_lock_acquire_noint(&thread_list_lock)
#define thread_list_lock_waitfor()       Dee_shared_lock_waitfor(&thread_list_lock)
#define thread_list_lock_release()       Dee_shared_lock_release(&thread_list_lock)
#else /* !DeeThread_USE_SINGLE_THREADED */
#define thread_list_lock_available()     true
#define thread_list_lock_acquired()      true
#define thread_list_lock_tryacquire()    true
#define thread_list_lock_acquire()       0
#define thread_list_lock_acquire_noint() (void)0
#define thread_list_lock_waitfor()       0
#define thread_list_lock_release()       (void)0
#endif /* DeeThread_USE_SINGLE_THREADED */


/* Controller for the initial/main thread object. */
INTERN DeeOSThreadObject DeeThread_Main = {
	/* .ot_thread = */ {
		OBJECT_HEAD_INIT(&DeeThread_Type),
		/* .t_str_curr   = */ NULL,
		/* .t_repr_curr  = */ NULL,
		/* .t_hash_curr  = */ NULL,
		/* .t_deepassoc  = */ { 0, 0, empty_deep_assoc, 0 },
		/* .t_exec       = */ NULL,
		/* .t_except     = */ NULL,
		/* .t_execsz     = */ 0,
		/* .t_exceptsz   = */ 0,
		/* .t_state      = */ Dee_THREAD_STATE_STARTED |
#ifdef DeeThread_HAVE_GetCurrentXThread
		                      Dee_THREAD_STATE_HASTHREAD |
#endif /* DeeThread_HAVE_GetCurrentXThread */
#ifdef DeeThread_GetCurrentTid
		                      Dee_THREAD_STATE_HASTID |
#endif /* DeeThread_GetCurrentTid */
		                      0,
		/* .t_interrupt  = */ { NULL, NULL, NULL },
#ifndef CONFIG_NO_THREADS
		/* .t_int_vers   = */ 0,
		/* .t_global     = */ LIST_ENTRY_UNBOUND_INITIALIZER,
		/* .t_threadname = */ (DeeStringObject *)&main_thread_name,
		/* .t_inout      = */ { NULL },
		/* .t_context    = */ { NULL },
#endif /* !CONFIG_NO_THREADS */
	},
#ifdef Dee_pid_t
	/* .ot_tid = */ 0,
#endif /* Dee_pid_t */
#ifndef DeeThread_USE_SINGLE_THREADED
#ifdef DeeThread_USE_CreateThread
	/* .ot_hThread = */ INVALID_HANDLE_VALUE,
#endif /* DeeThread_USE_CreateThread */
#endif /* !DeeThread_USE_SINGLE_THREADED */
};





/* Suspend/resume execution of the given thread.
 * WARNING: Do _NOT_ expose these functions to user-code.
 * WARNING: Do not attempt to suspend more than a single thread at once using this
 *          method. If you need to suspend more, use `DeeThread_SuspendAll()' instead!
 * NOTE: This function (`DeeThread_Suspend') synchronously waits for the thread to
 *       actually become suspended, meaning that once it returns, the caller is allowed
 *       to assume that the given thread is no longer capable of executing instructions.
 * NOTE: Trying to suspend yourself results in undefined behavior
 * @return: 1 : The specified thread could not be suspended because it has already terminated
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeThread_Suspend(DeeThreadObject *__restrict self) {
#ifdef DeeThread_USE_SINGLE_THREADED
	ASSERT(self != DeeThread_Self());
#else /* DeeThread_USE_SINGLE_THREADED */
	uint32_t state;
	ASSERT(self != DeeThread_Self());
	if (thread_list_lock_acquire())
		goto err;

	/* Indicate a request for the thread to become suspended */
	for (;;) {
		state = atomic_read(&self->t_state);
		ASSERT(!(state & Dee_THREAD_STATE_SUSPENDING));
		if (state & (Dee_THREAD_STATE_TERMINATING | Dee_THREAD_STATE_UNMANAGED)) {
			/* Cannot suspend a thread that is terminating. */
			thread_list_lock_release();
			if unlikely(state & Dee_THREAD_STATE_UNMANAGED)
				goto err_unmanaged_thread;
			return 1;
		}
		if (atomic_cmpxch_weak(&self->t_state, state,
		                       state |
		                       Dee_THREAD_STATE_SUSPENDING |
		                       Dee_THREAD_STATE_INTERRUPTED))
			break;
	}
	for (;;) {
		state = atomic_read(&self->t_state);
		if (state & Dee_THREAD_STATE_SUSPENDED)
			break;

		/* Wake up the thread (in case it's currently inside of a blocking system call) */
		DeeThread_Wake((DeeObject *)self);

		/* Futex-wait for the thread to become suspended (with a short timeout so we're
		 * able to catch threads that were just on their way into a blocking system call). */
		if (DeeFutex_Wait32Timed(&self->t_state, state, THREAD_WAKE_DELAY) < 0)
			goto err;
	}

	/* Success! The thread is now suspended. */
	return 0;
err:
	atomic_and(&self->t_state, ~(Dee_THREAD_STATE_SUSPENDING |
	                             Dee_THREAD_STATE_WAITING));
	thread_list_lock_release();
	DeeFutex_WakeAll(&self->t_state);
	return -1;
err_unmanaged_thread:
#endif /* !DeeThread_USE_SINGLE_THREADED */
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot suspend unmanaged thread %k",
	                       self);
}

PUBLIC NONNULL((1)) void DCALL
DeeThread_Resume(DeeThreadObject *__restrict self) {
#ifdef DeeThread_USE_SINGLE_THREADED
	(void)self;
	COMPILER_IMPURE();
	Dee_Fatalf("Suspending threads is impossible, so there should "
	           "be no situation where this function gets called.");
#else /* DeeThread_USE_SINGLE_THREADED */
	ASSERT(thread_list_lock_acquired());
	ASSERT(atomic_read(&self->t_state) & Dee_THREAD_STATE_SUSPENDED);
	ASSERT(atomic_read(&self->t_state) & Dee_THREAD_STATE_SUSPENDING);
	atomic_and(&self->t_state, ~(Dee_THREAD_STATE_SUSPENDING |
	                             Dee_THREAD_STATE_WAITING));
	thread_list_lock_release();
	DeeFutex_WakeAll(&self->t_state);
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

/* Safely suspend/resume all threads but the calling.
 * The same restrictions that apply to `DeeThread_Suspend()'
 * and `DeeThread_Resume()' also apply to this function pair.
 * @return: * :   Start of thread list
 * @return: NULL: An error was thrown */
PUBLIC WUNUSED DeeThreadObject *DCALL DeeThread_SuspendAll(void) {
#ifdef DeeThread_USE_SINGLE_THREADED
	COMPILER_IMPURE();
	return &DeeThread_Main.ot_thread;
#else /* DeeThread_USE_SINGLE_THREADED */
	DeeThreadObject *iter, *caller = DeeThread_Self();

	/* NOTE: Acquire (and keep) a lock to ensure that only
	 *       a single thread is ever able to suspend all others. */
	if (thread_list_lock_acquire())
		return NULL;
	iter = &DeeThread_Main.ot_thread;
	do {
		/* Send suspend request to all threads but the caller themselves. */
		if (iter != caller && !(atomic_read(&iter->t_state) & Dee_THREAD_STATE_TERMINATED)) {
			ASSERT(!(atomic_read(&iter->t_state) & Dee_THREAD_STATE_SUSPENDING));
			atomic_or(&iter->t_state, Dee_THREAD_STATE_SUSPENDING |
			                          Dee_THREAD_STATE_INTERRUPTED);
			DeeThread_Wake((DeeObject *)iter);
		}
	} while ((iter = iter->t_global.le_next) != NULL);

	/* Wait for all threads to become suspended. */
again_wait_for_suspend:
	iter = &DeeThread_Main.ot_thread;
	do {
		if (iter != caller) {
			if (atomic_read(&iter->t_state) & Dee_THREAD_STATE_TERMINATED) {
				ASSERT(iter != &DeeThread_Main.ot_thread);
				LIST_UNBIND(iter, t_global);
				goto again_wait_for_suspend;
			}
			for (;;) {
				uint32_t state = atomic_read(&iter->t_state);
				if (state & Dee_THREAD_STATE_SUSPENDED)
					break;
				if (DeeFutex_Wait32Timed(&iter->t_state, state, THREAD_WAKE_DELAY) < 0)
					goto err;

				/* Wake up the thread (in case it's currently inside of a blocking system call) */
				DeeThread_Wake((DeeObject *)iter);
			}
		}
	} while ((iter = iter->t_global.le_next) != NULL);

	/* Expose the global thread list to allow the caller to enumerate it. */
	return &DeeThread_Main.ot_thread;
err:
	/* Resume execution in all threads. */
	iter = &DeeThread_Main.ot_thread;
	do {
		if (iter != caller && !(atomic_read(&iter->t_state) & Dee_THREAD_STATE_TERMINATED)) {
			ASSERT(atomic_read(&iter->t_state) & Dee_THREAD_STATE_SUSPENDING);
			atomic_and(&iter->t_state, ~(Dee_THREAD_STATE_SUSPENDING |
			                             Dee_THREAD_STATE_WAITING));
			DeeFutex_WakeAll(&iter->t_state);
		}
	} while ((iter = iter->t_global.le_next) != NULL);
	thread_list_lock_release();
	return NULL;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

PUBLIC void DCALL DeeThread_ResumeAll(void) {
#ifdef DeeThread_USE_SINGLE_THREADED
	COMPILER_IMPURE();
#else /* DeeThread_USE_SINGLE_THREADED */
	DeeThreadObject *iter, *caller = DeeThread_Self();
	ASSERT(thread_list_lock_acquired());
	iter = &DeeThread_Main.ot_thread;
	do {
		if (iter != caller && !(atomic_read(&iter->t_state) & Dee_THREAD_STATE_TERMINATED)) {
			ASSERT(atomic_read(&iter->t_state) & Dee_THREAD_STATE_SUSPENDED);
			ASSERT(atomic_read(&iter->t_state) & Dee_THREAD_STATE_SUSPENDING);
			atomic_and(&iter->t_state, ~(Dee_THREAD_STATE_SUSPENDING |
			                             Dee_THREAD_STATE_WAITING));
			DeeFutex_WakeAll(&iter->t_state);
		}
	} while ((iter = iter->t_global.le_next) != NULL);

	/* Release the lock acquired in `DeeThread_SuspendAll()' */
	thread_list_lock_release();
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

#ifdef DeeThread_Detach_system_impl
PRIVATE NONNULL((1)) void DCALL
DeeThread_Detach_impl(DeeThreadObject *__restrict self) {
	uint32_t state;
again:
	state = atomic_fetchor(&self->t_state, Dee_THREAD_STATE_DETACHING);
	if (state & Dee_THREAD_STATE_DETACHING) {
		if (!(state & Dee_THREAD_STATE_HASTHREAD))
			return; /* Already detached */
		if (state & Dee_THREAD_STATE_UNMANAGED)
			return; /* Not allowed to detach */
		SCHED_YIELD();
		goto again;
	}
	if (state & Dee_THREAD_STATE_HASTHREAD) {
		if (state & Dee_THREAD_STATE_UNMANAGED)
			return; /* Not allowed to detach */
		DeeThread_Detach_system_impl(self);
	}
	atomic_and(&self->t_state, ~(Dee_THREAD_STATE_DETACHING |
	                             Dee_THREAD_STATE_HASOSCTX));
}
#endif /* DeeThread_Detach_system_impl */

PRIVATE NONNULL((1)) void DCALL
DeeThread_DiscardAllInterrupts(DeeThreadObject *__restrict self) {
	do {
		DREF DeeObject *intr;
		DREF DeeTupleObject *args;
		intr = self->t_interrupt.ti_intr;
		args = self->t_interrupt.ti_args;
		if (self->t_interrupt.ti_next) {
			struct thread_interrupt *interrupt;
			interrupt = self->t_interrupt.ti_next;
			memcpy(&self->t_interrupt, interrupt,
			       sizeof(struct thread_interrupt));
			Dee_thread_interrupt_free(interrupt);
		} else {
			self->t_interrupt.ti_intr = NULL;
			self->t_interrupt.ti_args = NULL;
		}
		if (ITER_ISOK(args))
			Dee_Decref(args);
		Dee_Decref(intr);
	} while (self->t_interrupt.ti_intr != NULL);
}


PRIVATE void DCALL DeeThread_Main_SetTerminating(void) {
	if (atomic_read(&DeeThread_Main.ot_thread.t_state) & Dee_THREAD_STATE_TERMINATING)
		return;
	_DeeThread_AcquireInterrupt(&DeeThread_Main.ot_thread);
	atomic_or(&DeeThread_Main.ot_thread.t_state, Dee_THREAD_STATE_TERMINATING);
	_DeeThread_ReleaseInterrupt(&DeeThread_Main.ot_thread);
	if (DeeThread_Main.ot_thread.t_interrupt.ti_intr != NULL) {
		/* XXX: Maybe we should actually execute interrupts here (and discard exceptions)? */
		DeeThread_DiscardAllInterrupts(&DeeThread_Main.ot_thread);
	}
	DBG_memset(&DeeThread_Main.ot_thread.t_interrupt, 0xcc,
	           sizeof(DeeThread_Main.ot_thread.t_interrupt));
}

/* Join all threads that are still running
 * after sending an interrupt signal to each.
 * Returns true if at least one thread was joined. */
INTERN bool DCALL DeeThread_InterruptAndJoinAll(void) {
#ifdef DeeThread_USE_SINGLE_THREADED
	DeeThread_Main_SetTerminating();
	return false;
#else /* DeeThread_USE_SINGLE_THREADED */
	DeeThreadObject *thread;
	bool result = false;
again:
	thread_list_lock_acquire_noint();

	/* Mark all threads for shutdown, and wake them all */
	for (thread = DeeThread_Main.ot_thread.t_global.le_next;
	     thread != NULL; thread = thread->t_global.le_next) {
		atomic_or(&thread->t_state, Dee_THREAD_STATE_SHUTDOWNINTR |
		                            Dee_THREAD_STATE_INTERRUPTED);
		DeeThread_Wake((DeeObject *)thread);
		result = true;
	}

	/* Detach threads, unlink ones that have already terminated, and wait for one that hasn't */
again_find_running_thread:
	while ((thread = DeeThread_Main.ot_thread.t_global.le_next) != NULL) {
		uint32_t state = atomic_read(&thread->t_state);
#ifdef DeeThread_Detach_system_impl
		if (state & Dee_THREAD_STATE_HASTHREAD) {
			DeeThread_Detach_impl(thread);
			state = atomic_read(&thread->t_state);
		}
#endif /* DeeThread_Detach_system_impl */

		if (!(state & Dee_THREAD_STATE_TERMINATED))
			break;
		LIST_UNBIND(thread, t_global);
	}
	if (thread == NULL) {
		thread_list_lock_release();
		if (!result) {
			/* At this point, the main thread is once again the only remaining thread.
			 * With this in mind, mark it as TERMINATING and handle any remaining
			 * interrupts that may be pending (this is needed since those interrupts
			 * are likely to be other threads that need to be decref'd)
			 *
			 * Note that we only do this the first time we set TERMINATING, since after
			 * that, the interrupt list itself becomes invalid. */
			DeeThread_Main_SetTerminating();
		}
		return result; /* Done! */
	}

	ASSERT(atomic_read(&thread->t_state) & Dee_THREAD_STATE_SHUTDOWNINTR);
	if (!Dee_IncrefIfNotZero(thread)) {
		ASSERTF(atomic_read(&thread->t_state) & Dee_THREAD_STATE_TERMINATED,
		        "STARTED thread with 0 references, but no TERMINATED flag?");
		LIST_UNBIND(thread, t_global);
		goto again_find_running_thread;
	}
	thread_list_lock_release();

	/* Wait for this thread to terminate */
	for (;;) {
		uint32_t state = atomic_read(&thread->t_state);
		if (state & Dee_THREAD_STATE_TERMINATED)
			break;

		/* Wait a bit for the thread to finish (but keep sending wake-ups
		 * in case the thread is currently inside of a blocking system call) */
		if (!(state & Dee_THREAD_STATE_WAITING)) {
			atomic_or(&thread->t_state, Dee_THREAD_STATE_WAITING);
			state |= Dee_THREAD_STATE_WAITING;
		}
		DeeFutex_Wait32NoIntTimed(&thread->t_state, state, THREAD_WAKE_DELAY);
		DeeThread_Wake((DeeObject *)thread);
	}

	/* Assert that the thread has no exited. */
	ASSERT(atomic_read(&thread->t_state) & Dee_THREAD_STATE_TERMINATED);
	Dee_Decref(thread);

	/* Continue joining all the other threads. */
	goto again;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}



/* Check if `pthread_key_t' is supported */
#undef CONFIG_HAVE_pthread_key_t
#if (defined(CONFIG_HAVE_pthread_key_create) &&  \
     defined(CONFIG_HAVE_pthread_getspecific) && \
     defined(CONFIG_HAVE_pthread_setspecific))
#define CONFIG_HAVE_pthread_key_t
#endif /* ... */

/* Check if `tss_t' is supported */
#undef CONFIG_HAVE_tss_t
#if (defined(CONFIG_HAVE_tss_create) && \
     defined(CONFIG_HAVE_tss_get) &&    \
     defined(CONFIG_HAVE_tss_set))
#define CONFIG_HAVE_tss_t
#endif /* ... */


/* Figure out how to implement the `thread_self_tls' */
#undef thread_self_tls_USE_DeeThread_Main
#undef thread_self_tls_USE_ATTR_THREAD
#undef thread_self_tls_USE_TlsAlloc
#undef thread_self_tls_USE_pthread_key_t
#undef thread_self_tls_USE_tss_t
#undef thread_self_tls_USE_errno_address
#undef thread_self_tls_USE_sp_address
#ifdef DeeThread_USE_SINGLE_THREADED
#define thread_self_tls_USE_DeeThread_Main
#elif !defined(__NO_ATTR_THREAD)
#define thread_self_tls_USE_ATTR_THREAD
#elif defined(DeeThread_USE_CreateThread)
#define thread_self_tls_USE_TlsAlloc
#elif defined(DeeThread_USE_pthread_create) && defined(CONFIG_HAVE_pthread_key_t)
#define thread_self_tls_USE_pthread_key_t
#elif defined(DeeThread_USE_thrd_create) && defined(CONFIG_HAVE_tss_t)
#define thread_self_tls_USE_tss_t
#elif defined(CONFIG_HOST_WINDOWS)
#define thread_self_tls_USE_TlsAlloc
#elif defined(CONFIG_HAVE_pthread_key_t)
#define thread_self_tls_USE_pthread_key_t
#elif defined(CONFIG_HAVE_tss_t)
#define thread_self_tls_USE_tss_t
#elif defined(CONFIG_HAVE_errno)
#define thread_self_tls_USE_errno_address
#else /* ... */
#ifdef PTHREAD_STACK_MIN
#define SYSTEM_STACK_SIZE PTHREAD_STACK_MIN
#else /* PTHREAD_STACK_MIN */
#define SYSTEM_STACK_SIZE 16384
#endif /* !PTHREAD_STACK_MIN */
#define thread_self_tls_USE_sp_address_MASK (~(SYSTEM_STACK_SIZE - 1))
#define thread_self_tls_USE_sp_address
#endif /* !... */




#ifdef thread_self_tls_USE_DeeThread_Main
#define thread_tls_get() (&DeeThread_Main.ot_thread)
#endif /* thread_self_tls_USE_DeeThread_Main */

#ifdef thread_self_tls_USE_ATTR_THREAD
PRIVATE ATTR_THREAD DREF DeeThreadObject *thread_self_tls = NULL;
#define thread_tls_get()  (thread_self_tls)
#define thread_tls_set(v) (void)(thread_self_tls = (v))
#endif /* thread_self_tls_USE_ATTR_THREAD */

#ifdef thread_self_tls_USE_TlsAlloc
PRIVATE DWORD thread_self_tls;

#ifdef __i386__
/* Considering how often we need to read this TLS, here's
 * an inline implementation that does the same.
 * Microsoft doesn't guaranty that this will work forever, but
 * I don't see a reason why this should ever break short of them
 * intentionally breaking it so only their stuff can continue to
 * work with whatever offsets they choose to go for them.
 * https://en.wikipedia.org/wiki/Win32_Thread_Information_Block */
#ifdef _MSC_VER
#define thread_tls_get() thread_tls_get()
FORCELOCAL DeeThreadObject *(thread_tls_get)(void) {
	void *result;
	__asm {
		MOV EAX, thread_self_tls
		MOV EAX, DWORD PTR FS:[0xe10 + EAX * 4]
		MOV result, EAX
	}
	return (DeeThreadObject *)result;
}
#define thread_tls_set(value) thread_tls_set(value)
FORCELOCAL void (thread_tls_set)(void *value) {
	__asm {
		MOV ECX, value
		MOV EAX, thread_self_tls
		MOV DWORD PTR FS:[0xe10 + EAX * 4], ECX
	}
}
#elif defined(__COMPILER_HAVE_GCC_ASM)
#define thread_tls_get()  thread_tls_get()
FORCELOCAL DeeThreadObject *(thread_tls_get)(void) {
	register void *result;
	__asm__("movl %%fs:0xe10(,%1,4), %0\n"
	        : "=r" (result)
	        : "r" (thread_self_tls));
	return (DeeThreadObject *)result;
}
#define thread_tls_set(value) thread_tls_set(value)
FORCELOCAL void(thread_tls_set)(void *value) {
	__asm__("movl %1, %%fs:0xe10(,%0,4)\n"
	        :
	        : "r" (thread_self_tls)
	        , "r" (value));
}
#endif /* ... */
#endif /* __i386__ */

#ifndef thread_tls_get
#define thread_tls_get()  (DeeThreadObject *)TlsGetValue(thread_self_tls)
#define thread_tls_set(v) (void)TlsSetValue(thread_self_tls, (LPVOID)(v))
#endif /* !thread_tls_get */
#endif /* thread_self_tls_USE_TlsAlloc */


#ifdef thread_self_tls_USE_pthread_key_t
PRIVATE pthread_key_t thread_self_tls;
#define thread_tls_get()  (DeeThreadObject *)pthread_getspecific(thread_self_tls)
#define thread_tls_set(v) (void)pthread_setspecific(thread_self_tls, (void *)(v))
#endif /* thread_self_tls_USE_pthread_key_t */

#ifdef thread_self_tls_USE_tss_t
PRIVATE tss_t thread_self_tls;
#define thread_tls_get()  (DeeThreadObject *)tss_get(thread_self_tls)
#define thread_tls_set(v) (void)tss_set(thread_self_tls, (void *)(v))
#endif /* thread_self_tls_USE_tss_t */

#if (defined(thread_self_tls_USE_errno_address) || \
     defined(thread_self_tls_USE_sp_address))
/* TODO */
#error "TODO: Unimplemented TLS method"
#endif /* ... */


/* Return the thread controller object for the calling thread.
 * If the calling thread wasn't created by `DeeThread_Start()',
 * the caller must call `DeeThread_Accede()' at least once in
 * order to affiliate their thread with deemon. */
PUBLIC WUNUSED ATTR_CONST ATTR_RETNONNULL
DeeThreadObject *DCALL DeeThread_Self(void) {
#ifdef thread_self_tls_USE_DeeThread_Main
	return &DeeThread_Main.ot_thread;
#else /* thread_self_tls_USE_DeeThread_Main */
	DeeThreadObject *result;
	DBG_ALIGNMENT_DISABLE();
	result = thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	ASSERTF(result, "Your thread is not affiliated with deemon. You have to call `DeeThread_Accede()' first");
	return result;
#endif /* !thread_self_tls_USE_DeeThread_Main */
}




struct acceded_thread_object {
	struct os_thread_object at_os_thread; /* Underlying OS-thread */
#ifdef DeeThread_SetupSignalHandlers_USE_sigaction
	struct sigaction at_old_signal;
#elif (defined(DeeThread_SetupSignalHandlers_USE_bsd_signal) || \
       defined(DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal))
	void (*at_old_signal)(int signo);
#endif /* ... */
};



/* Initialize signal handlers (for sporadic interrupts) */
#ifdef DeeThread_Wake_NEEDS_SIGNAL
PRIVATE void DCALL _DeeThread_SetupSignalHandlersForCurrentThread(void);

PRIVATE void DeeThread_SporadicInterruptHandler(int signo) {
	(void)signo;
	Dee_DPRINTF("DeeThread_SporadicInterruptHandler called\n");
#ifdef DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal
	/* In this configuration, signal handlers are one-shot (so we need to restore them) */
	_DeeThread_SetupSignalHandlersForCurrentThread();
#endif /* DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal */
}

#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE NONNULL((1)) void DCALL
DeeThread_SetupSignalHandlers(struct acceded_thread_object *__restrict thread) {
#ifdef DeeThread_SetupSignalHandlers_USE_sigaction
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = &DeeThread_SporadicInterruptHandler;
	if (sigaction(DeeThread_Wake_USED_SIGNAL, &act, &thread->at_old_signal) != 0)
		bzero(&thread->at_old_signal, sizeof(thread->at_old_signal));
#elif (defined(DeeThread_SetupSignalHandlers_USE_bsd_signal) || \
       defined(DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal))
#ifdef DeeThread_SetupSignalHandlers_USE_bsd_signal
	thread->at_old_signal = bsd_signal(DeeThread_Wake_USED_SIGNAL, &DeeThread_SporadicInterruptHandler);
#elif defined(CONFIG_HAVE_sysv_signal)
	thread->at_old_signal = sysv_signal(DeeThread_Wake_USED_SIGNAL, &DeeThread_SporadicInterruptHandler);
#else /* ... */
	thread->at_old_signal = signal(DeeThread_Wake_USED_SIGNAL, &DeeThread_SporadicInterruptHandler);
#endif /* !... */
#endif /* ... */
}

PRIVATE NONNULL((1)) void DCALL
DeeThread_RestoreSignalHandlers(struct acceded_thread_object *__restrict thread) {
#ifdef DeeThread_SetupSignalHandlers_USE_sigaction
	(void)sigaction(DeeThread_Wake_USED_SIGNAL, &thread->at_old_signal, NULL);
#elif (defined(DeeThread_SetupSignalHandlers_USE_bsd_signal) || \
       defined(DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal))
#ifdef DeeThread_SetupSignalHandlers_USE_bsd_signal
	(void)bsd_signal(DeeThread_Wake_USED_SIGNAL, thread->at_old_signal);
#elif defined(CONFIG_HAVE_sysv_signal)
	(void)sysv_signal(DeeThread_Wake_USED_SIGNAL, thread->at_old_signal);
#else /* ... */
	(void)signal(DeeThread_Wake_USED_SIGNAL, thread->at_old_signal);
#endif /* !... */
#endif /* ... */
}
#endif /* !DeeThread_USE_SINGLE_THREADED */

PRIVATE void DCALL
_DeeThread_SetupSignalHandlersForCurrentThread(void) {
#ifdef DeeThread_SetupSignalHandlers_USE_sigaction
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = &DeeThread_SporadicInterruptHandler;
	(void)sigaction(DeeThread_Wake_USED_SIGNAL, &act, NULL);
#elif (defined(DeeThread_SetupSignalHandlers_USE_bsd_signal) || \
       defined(DeeThread_SetupSignalHandlers_USE_sysv_signal_OR_signal))
#ifdef DeeThread_SetupSignalHandlers_USE_bsd_signal
	(void)bsd_signal(DeeThread_Wake_USED_SIGNAL, &DeeThread_SporadicInterruptHandler);
#elif defined(CONFIG_HAVE_sysv_signal)
	(void)sysv_signal(DeeThread_Wake_USED_SIGNAL, &DeeThread_SporadicInterruptHandler);
#else /* ... */
	(void)signal(DeeThread_Wake_USED_SIGNAL, &DeeThread_SporadicInterruptHandler);
#endif /* !... */
#endif /* ... */
}
#else /* DeeThread_Wake_NEEDS_SIGNAL */
#ifndef DeeThread_USE_SINGLE_THREADED
#define DeeThread_SetupSignalHandlers(thread)   (void)0
#define DeeThread_RestoreSignalHandlers(thread) (void)0
#endif /* !DeeThread_USE_SINGLE_THREADED */
#define _DeeThread_SetupSignalHandlersForCurrentThread() (void)0
#endif /* !DeeThread_Wake_NEEDS_SIGNAL */



#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE WUNUSED DREF DeeThreadObject *DCALL
DeeThread_AllocateCurrentThread(void) {
	DREF struct acceded_thread_object *result;
	result = DeeObject_TRYCALLOC(struct acceded_thread_object);
	if unlikely(result == NULL)
		return NULL;
	DeeObject_Init(&result->at_os_thread.ot_thread, &DeeThread_Type);
	result->at_os_thread.ot_thread.t_deepassoc.da_list = empty_deep_assoc;

	/* Set expected thread flags. */
	result->at_os_thread.ot_thread.t_state = 0 |
#ifdef DeeThread_GetCurrentTid
	                                         Dee_THREAD_STATE_HASTID |
#endif /* DeeThread_GetCurrentTid */
#ifdef DeeThread_HAVE_GetCurrentXThread
#ifndef DeeThread_USE_CreateThread /* NOTE: `DeeThread_GetCurrentHThread()' returns a proper handle, so don't set this flag in that case! */
	                                         Dee_THREAD_STATE_UNMANAGED | /* Don't allow deemon to detach the OS-handle of this thread */
	/* FIXME: Setting `Dee_THREAD_STATE_UNMANAGED' here breaks a whole bunch of stuff (like `DeeThread_Suspend()') */
#endif /* !DeeThread_USE_CreateThread */
	                                         Dee_THREAD_STATE_HASTHREAD |
#endif /* DeeThread_HAVE_GetCurrentXThread */
	                                         Dee_THREAD_STATE_STARTED;

	/* Lookup descriptor numbers for the calling thread. */
	DBG_ALIGNMENT_DISABLE();
#ifdef DeeThread_GetCurrentTid
	result->at_os_thread.ot_tid = DeeThread_GetCurrentTid();
#endif /* DeeThread_GetCurrentTid */
#ifdef DeeThread_HAVE_GetCurrentXThread
#ifdef DeeThread_USE_CreateThread
	result->at_os_thread.ot_hThread = DeeThread_GetCurrentHThread();
#endif /* DeeThread_USE_CreateThread */
#ifdef DeeThread_USE_pthread_create
	result->at_os_thread.ot_pthread = DeeThread_GetCurrentPThread();
#endif /* DeeThread_USE_pthread_create */
#ifdef DeeThread_USE_thrd_create
	result->at_os_thread.ot_thrd = DeeThread_GetCurrentThrd();
#endif /* DeeThread_USE_thrd_create */
#endif /* DeeThread_HAVE_GetCurrentXThread */
	DBG_ALIGNMENT_ENABLE();

	/* Setup signal handlers. */
	DeeThread_SetupSignalHandlers(result);

	/* Add the thread to the global list of threads */
	thread_list_lock_acquire_noint();
	LIST_INSERT_HEAD(&thread_list, &result->at_os_thread.ot_thread, t_global);
	thread_list_lock_release();

	return &result->at_os_thread.ot_thread;
}
#endif /* !DeeThread_USE_SINGLE_THREADED */


/* Hand over control of the calling thread to deemon until a call is
 * made to `DeeThread_Secede'. Note however that (since the caller's
 * stack doesn't end with deemon's thread bootstrap stub), the caller
 * must eventually call `DeeThread_Secede()' in order to secede their
 * deemon thread context once they are done executing deemon code.
 *
 * NOTE: When the caller's thread already has a deemon context, this
 *       function behaves the same as `DeeThread_Self()'
 *
 * @return: * :   The caller's thread controller.
 * @return: NULL: Failed to allocate a thread controller for the caller (out-of-memory)
 *                Note that in this case, no exception is thrown (because none can be
 *                thrown, as the caller doesn't have a deemon thread-context) */
PUBLIC WUNUSED DeeThreadObject *DCALL DeeThread_Accede(void) {
#ifdef DeeThread_USE_SINGLE_THREADED
	Dee_Fatalf("Threading support is disabled");
	return NULL;
#else /* DeeThread_USE_SINGLE_THREADED */
	DeeThreadObject *result;

	/* Check if the caller already has a thread context. */
	DBG_ALIGNMENT_DISABLE();
	result = thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if (result)
		return result;

	/* Lazily create the thread-self descriptor. */
	DeeSystemError_Push();
	result = DeeThread_AllocateCurrentThread();
	if likely(result) {
		/* Save the generated thread object in the TLS slot. */
		DBG_ALIGNMENT_DISABLE();
		thread_tls_set(result);
		DBG_ALIGNMENT_ENABLE();
	}
	DeeSystemError_Pop();
	return result;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}


/* Drop a reference from `self' in the context of another thread.
 * This function is called to get rid of the final reference that
 * a thread used to hold to itself */
#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE NONNULL((1)) void DCALL
DeeThread_DecrefInOtherThread(DREF DeeThreadObject *self) {
	ASSERT(self != &DeeThread_Main.ot_thread);
	ASSERT(self->t_interrupt.ti_intr == NULL);
	ASSERT(self->t_interrupt.ti_args == NULL);
	ASSERT(self->t_interrupt.ti_next == NULL);

	/* If someone else is still holding a reference to our thread,
	 * we can just let *them* deal with destroying our thread's
	 * controller! */
	if (Dee_DecrefIfNotOne(self))
		return;

	/* Use a custom interrupt descriptor to decref `self' in the main thread. */
	self->t_interrupt.ti_intr = (DREF DeeObject *)self;               /* Inherit reference */
	self->t_interrupt.ti_args = (struct Dee_tuple_object *)ITER_DONE; /* Decref marker (& prevent descriptor free) */

	/* Let the main thread inherit a reference to our thread. */
	_DeeThread_AcquireInterrupt(&DeeThread_Main.ot_thread);
	if (DeeThread_Main.ot_thread.t_interrupt.ti_intr != NULL) {
		/* Secondary interrupt */
		self->t_interrupt.ti_next = DeeThread_Main.ot_thread.t_interrupt.ti_next;
		DeeThread_Main.ot_thread.t_interrupt.ti_next = &self->t_interrupt; /* Inherit reference */
	} else {
		/* Initial interrupt */
		DeeThread_Main.ot_thread.t_interrupt.ti_intr = (DREF DeeObject *)self; /* Inherit reference */
		DeeThread_Main.ot_thread.t_interrupt.ti_args = (DREF struct Dee_tuple_object *)ITER_DONE;
	}
	_DeeThread_ReleaseInterrupt(&DeeThread_Main.ot_thread);

	/* Tell the main thread that it's got some interrupts to deal with. */
	atomic_or(&DeeThread_Main.ot_thread.t_state, Dee_THREAD_STATE_INTERRUPTED);
	DeeThread_Wake((DeeObject *)&DeeThread_Main.ot_thread);
}
#endif /* !DeeThread_USE_SINGLE_THREADED */


/* Secede deemon's control over the calling thread by simulating said
 * thread's termination in the eyes of user-code. Following this, the
 * calling thread is no longer considered as being managed by deemon.
 *
 * However, the calling thread is allowed to accede to deemon once
 * again in the future by making another call to `DeeThread_Accede()'
 *
 * @param: thread_result: When non-NULL, the result of the thread if
 *                        someone tries to join it. Else, if a deemon
 *                        exception is currently thrown, the thread
 *                        will have exited with that exception. Else,
 *                        the thread will simply return `Dee_None'. */
PUBLIC void DCALL
DeeThread_Secede(DREF DeeObject *thread_result) {
#ifdef DeeThread_USE_SINGLE_THREADED
	(void)thread_result;
	COMPILER_IMPURE();
	Dee_Fatalf("Threading support is disabled");
#else /* DeeThread_USE_SINGLE_THREADED */
	DREF DeeThreadObject *self;
	DBG_ALIGNMENT_DISABLE();
	self = thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if (!self) {
		ASSERTF(!thread_result, "Thread was never allocated, but we've been given an object reference?");
		return;
	}

	/* Assert that the thread is in a state where it can be detached from deemon. */
	ASSERTF(Dee_TYPE(self) == &DeeThread_Type, "Custom thread types cannot be seceded");
	ASSERTF(self != &DeeThread_Main.ot_thread, "The main thread cannot secede control");
#if defined(DeeThread_HAVE_GetCurrentXThread) && !defined(DeeThread_USE_CreateThread)
	ASSERTF(self->t_state & Dee_THREAD_STATE_UNMANAGED, "Calling thread was created by deemon");
#endif /* DeeThread_HAVE_GetCurrentXThread && !DeeThread_USE_CreateThread */
	ASSERTF(self->t_inout.io_result == NULL, "Calling thread was created by deemon");
	ASSERTF(self->t_exec == NULL && self->t_execsz == 0, "Calling thread is still executing deemon code");
	ASSERTF(self->t_str_curr == NULL, "Calling thread still has active calls to `DeeObject_Str'");
	ASSERTF(self->t_repr_curr == NULL, "Calling thread still has active calls to `DeeObject_Repr'");
	ASSERTF(self->t_hash_curr == NULL, "Calling thread still has active calls to `DeeObject_Hash'");
	ASSERTF(self->t_deepassoc.da_used == 0, "Calling thread still has active calls to `DeeObject_DeepCopy'");
	ASSERT((self->t_deepassoc.da_mask != 0) ==
	       (self->t_deepassoc.da_list != empty_deep_assoc));
	ASSERT(!self->t_threadname || DeeString_Check(self->t_threadname));

	/* Set the TERMINATING flag to prevent further interrupts from being scheduled. */
	_DeeThread_AcquireInterrupt(self);
	atomic_or(&self->t_state, Dee_THREAD_STATE_TERMINATING);
	_DeeThread_ReleaseInterrupt(self);

	/* Try to handle interrupts synchronously (but only once; if an
	 * error happens, all other interrupts are simply discarded). */
	if (DeeThread_CheckInterruptSelf(self))
		DeeError_Handled(ERROR_HANDLED_INTERRUPT);

again_cleanup:
	/* Clean-up TLS data (needs to happen in the original thread's context) */
	if (self->t_context.d_tls != NULL) {
		void *tls_data;
		tls_data = self->t_context.d_tls;
		self->t_context.d_tls = NULL;
		fini_tls_data(tls_data);
		goto again_cleanup;
	}

	/* Discard all interrupts that are still pending */
	if (self->t_interrupt.ti_intr) {
		DeeThread_DiscardAllInterrupts(self);
		goto again_cleanup;
	}

	if (thread_result != NULL && self->t_except != NULL) {
		do {
			struct Dee_except_frame *frame;
			/* Must discard exceptions. */
			frame = self->t_except;
			self->t_except = frame->ef_prev;
			Dee_Decref(frame->ef_error);
			if (ITER_ISOK(frame->ef_trace))
				Dee_Decref(frame->ef_trace);
			Dee_except_frame_free(frame);
			ASSERT(self->t_exceptsz != 0);
			--self->t_exceptsz;
		} while (self->t_except != NULL);
		ASSERT(self->t_exceptsz == 0);
		goto again_cleanup;
	}

	/* ==== POINT OF NO RETURN ====
	 *
	 * from this point forth, no deemon code may be executed by the thread anymore */

	/* Must act as though the thread had exited. */
	_DeeThread_AcquireSetup(self);
	if (thread_result != NULL) {
		ASSERT(self->t_exceptsz == 0);
		ASSERT(self->t_except == NULL);
		self->t_inout.io_result = thread_result; /* Inherit reference */
	} else if (self->t_except != NULL) {
		/* Return with an exception */
		ASSERT(self->t_exceptsz != 0);
	} else {
		/* Simply return `Dee_None' */
		ASSERT(self->t_exceptsz == 0);
		ASSERT(self->t_except == NULL);
		self->t_inout.io_result = DeeNone_NewRef();
	}
	atomic_or(&self->t_state, Dee_THREAD_STATE_TERMINATED);
	_DeeThread_ReleaseSetup(self);

	_DeeThread_AcquireDetaching(self);
	atomic_and(&self->t_state, ~(Dee_THREAD_STATE_DETACHING |
	                             Dee_THREAD_STATE_HASOSCTX |
	                             Dee_THREAD_STATE_INTERRUPTED));

	/* Wake up threads that may be waiting for our thread to terminate */
	_DeeThread_WakeWaiting(self);

	if (LIST_ISBOUND(self, t_global)) {
		if (thread_list_lock_tryacquire()) {
			if (LIST_ISBOUND(self, t_global))
				LIST_UNBIND(self, t_global);
			thread_list_lock_release();
		}
	}

	/* Restore signal handlers. */
	{
		struct acceded_thread_object *athread;
		athread = COMPILER_CONTAINER_OF(self, struct acceded_thread_object, at_os_thread.ot_thread);
		DeeThread_RestoreSignalHandlers(athread);
	}

	/* Drop the reference previously held by the TLS variable */
	DeeThread_DecrefInOtherThread(self);

	/* Clear our own TLS context. */
	DBG_ALIGNMENT_DISABLE();
	thread_tls_set(NULL);
	DBG_ALIGNMENT_ENABLE();
#endif /* !DeeThread_USE_SINGLE_THREADED */
}



#if defined(DeeThread_HAVE_GetCurrentXThread) && defined(DeeThread_USE_CreateThread)
#define _DeeThread_FiniMainThread() \
	(void)CloseHandle(DeeThread_Main.ot_hThread)
#else /* DeeThread_HAVE_GetCurrentXThread && DeeThread_USE_CreateThread */
#define _DeeThread_FiniMainThread() (void)0
#endif /* !DeeThread_HAVE_GetCurrentXThread || !DeeThread_USE_CreateThread */


#if !defined(DeeThread_GetCurrentTid) && !defined(DeeThread_HAVE_GetCurrentXThread)
#define _DeeThread_InitMainThread() (void)0
#else /* !DeeThread_GetCurrentTid && !DeeThread_HAVE_GetCurrentXThread */
PRIVATE void DCALL _DeeThread_InitMainThread(void) {
	/* Collect information on the main thread. */
	DBG_ALIGNMENT_DISABLE();
#ifdef DeeThread_GetCurrentTid
	DeeThread_Main.ot_tid = DeeThread_GetCurrentTid();
#endif /* DeeThread_GetCurrentTid */

#ifdef DeeThread_HAVE_GetCurrentXThread
#ifdef DeeThread_USE_CreateThread
	DeeThread_Main.ot_hThread = DeeThread_GetCurrentHThread();
#endif /* DeeThread_USE_CreateThread */
#ifdef DeeThread_USE_pthread_create
	DeeThread_Main.ot_pthread = DeeThread_GetCurrentPThread();
#endif /* DeeThread_USE_pthread_create */
#ifdef DeeThread_USE_thrd_create
	DeeThread_Main.ot_thrd = DeeThread_GetCurrentThrd();
#endif /* DeeThread_USE_thrd_create */
#endif /* DeeThread_HAVE_GetCurrentXThread */
	DBG_ALIGNMENT_ENABLE();
}
#endif /* DeeThread_GetCurrentTid || DeeThread_HAVE_GetCurrentXThread */


PRIVATE void DCALL _DeeThread_SelfTlsInit(void) {
#ifdef thread_self_tls_USE_TlsAlloc
	thread_self_tls = TlsAlloc();
	if unlikely(thread_self_tls == TLS_OUT_OF_INDEXES) {
#if defined(CONFIG_HAVE_fprintf) && defined(CONFIG_HAVE_stderr)
		fprintf(stderr, "Failed to initialize deemon thread subsystem: "
		                "Couldn't allocate Thread.current TLS: %u\n",
		        (unsigned int)GetLastError());
#endif /* CONFIG_HAVE_fprintf && CONFIG_HAVE_stderr */
		abort();
	}
#endif /* thread_self_tls_USE_TlsAlloc */

#ifdef thread_self_tls_USE_pthread_key_t
	{
		int error;
		error = pthread_key_create(&thread_self_tls, NULL);
		if unlikely(error) {
#if defined(CONFIG_HAVE_fprintf) && defined(CONFIG_HAVE_stderr)
			fprintf(stderr, "Failed to initialize deemon thread subsystem: "
			                "Couldn't allocate Thread.current TLS: %d - %s\n",
			        error,
#ifdef CONFIG_HAVE_strerror
			        strerror(error)
#else /* CONFIG_HAVE_strerror */
			        "?"
#endif /* !CONFIG_HAVE_strerror */
			        );
#endif /* CONFIG_HAVE_fprintf && CONFIG_HAVE_stderr */
			abort();
		}
	}
#endif /* thread_self_tls_USE_pthread_key_t */

#ifdef thread_self_tls_USE_tss_t
	if unlikely(tss_create(&thread_self_tls, NULL) != thrd_success) {
#if defined(CONFIG_HAVE_fprintf) && defined(CONFIG_HAVE_stderr)
		fprintf(stderr, "Failed to initialize deemon thread subsystem: "
		                "Couldn't allocate Thread.current TLS\n");
#endif /* CONFIG_HAVE_fprintf && CONFIG_HAVE_stderr */
		abort();
	}
#endif /* thread_self_tls_USE_tss_t */

	/* Set the thread-self TLS value of the main thread. */
#ifdef thread_tls_set
	thread_tls_set(&DeeThread_Main.ot_thread);
#endif /* thread_tls_set */
}


PRIVATE void DCALL _DeeThread_SelfTlsFini(void) {
#ifdef thread_self_tls_USE_TlsAlloc
	TlsFree(thread_self_tls);
#endif /* thread_self_tls_USE_TlsAlloc */
#if defined(thread_self_tls_USE_pthread_key_t) && defined(CONFIG_HAVE_pthread_key_delete)
	pthread_key_delete(thread_self_tls);
#endif /* thread_self_tls_USE_pthread_key_t && CONFIG_HAVE_pthread_key_delete */
#if defined(thread_self_tls_USE_tss_t) && defined(CONFIG_HAVE_tss_delete)
	tss_delete(thread_self_tls);
#endif /* thread_self_tls_USE_tss_t && CONFIG_HAVE_tss_delete */
}


/* Initialize/Finalize the threading sub-system.
 * NOTE: `DeeThread_SubSystemInit()' must be called by the
 *       thread main before any other part of deemon's API,
 *       whilst `DeeThread_SubSystemFini()' can optionally be called
 *       (if only to prevent resource leaks cleaned up by the OS in
 *       any case) once no other API function is going to run. */
INTERN void DCALL DeeThread_SubSystemInit(void) {
	DBG_ALIGNMENT_DISABLE();

	/* Initialize the main-thread object */
	_DeeThread_InitMainThread();

	/* Setup signal handlers for the main thread. */
	_DeeThread_SetupSignalHandlersForCurrentThread();

	/* Initialize the thread-self TLS variable. */
	_DeeThread_SelfTlsInit();

	DBG_ALIGNMENT_ENABLE();
}

INTERN void DCALL DeeThread_SubSystemFini(void) {
	DBG_ALIGNMENT_DISABLE();

	/* Finalize the TLS variable used to track `DeeThread_Self()' */
	_DeeThread_SelfTlsFini();

	/* Finalize the main-thread object */
	_DeeThread_FiniMainThread();

	/* Do some cleanup on the main-thread object */
	if (DeeThread_Main.ot_thread.t_deepassoc.da_list != empty_deep_assoc)
		Dee_Free(DeeThread_Main.ot_thread.t_deepassoc.da_list);
	DeeThread_Main.ot_thread.t_deepassoc.da_used = 0;
	DeeThread_Main.ot_thread.t_deepassoc.da_mask = 0;
	DeeThread_Main.ot_thread.t_deepassoc.da_list = empty_deep_assoc;

	DBG_ALIGNMENT_ENABLE();
}




#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
INTERN uint8_t keyboard_interrupt_counter = 0;
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */


/* Same as `DeeThread_CheckInterrupt()', but faster
 * if the caller already knows their own thread object. */
INTERN WUNUSED NONNULL((1)) int
(DCALL DeeThread_CheckInterruptSelf)(DeeThreadObject *__restrict self) {
	uint32_t state;
again_read_state:
	state = atomic_read(&self->t_state);

	/* Check if we've been interrupted. */
	if (!(state & Dee_THREAD_STATE_INTERRUPTED))
		return 0;
#ifndef DeeThread_USE_SINGLE_THREADED
	atomic_inc(&self->t_int_vers); /* Indicate that we're checking for interrupts */
#endif /* !DeeThread_USE_SINGLE_THREADED */
	atomic_and(&self->t_state, ~Dee_THREAD_STATE_INTERRUPTED);

	/* Someone may be waiting for us to start handling our interrupts.
	 * If that is the case, then we must wake them. */
	_DeeThread_WakeWaiting(self);

	/* Preserve system errors across interrupt checks */
	DeeSystemError_Push();

	/* Check for different types of interrupts */
#ifndef DeeThread_USE_SINGLE_THREADED
again_check_for_interrupts:
	if (state & Dee_THREAD_STATE_SUSPENDING) {

		/* Enter the suspend-state */
		atomic_or(&self->t_state, Dee_THREAD_STATE_SUSPENDED);
		DeeFutex_WakeAll(&self->t_state);

		/* Wait until the suspend-state ends */
		for (;;) {
			state = atomic_read(&self->t_state);
			if (!(state & Dee_THREAD_STATE_SUSPENDING))
				break;
			DeeFutex_Wait32NoInt(&self->t_state, state);
		}

		/* Leave the suspend state */
		atomic_and(&self->t_state, ~Dee_THREAD_STATE_SUSPENDED);
		DeeFutex_WakeAll(&self->t_state);

		/* Check for more interrupts. */
		goto again_check_for_interrupts;
	}
#endif /* !DeeThread_USE_SINGLE_THREADED */

	/* The main thread must handle keyboard interrupts! */
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
#ifndef DeeThread_USE_SINGLE_THREADED
	if (self == &DeeThread_Main.ot_thread)
#endif /* !DeeThread_USE_SINGLE_THREADED */
	{
		for (;;) {
			uint8_t count;
			DeeSignalObject *keyboard_interrupt;
			if ((count = atomic_read(&keyboard_interrupt_counter)) == 0)
				break;
			if (!atomic_cmpxch_weak(&keyboard_interrupt_counter, count, count - 1))
				continue;
			keyboard_interrupt = DeeObject_MALLOC(DeeSignalObject);
			if unlikely(!keyboard_interrupt)
				goto err;
			DeeObject_Init(keyboard_interrupt, &DeeError_KeyboardInterrupt);
			DeeError_Throw((DeeObject *)keyboard_interrupt);
			Dee_Decref(keyboard_interrupt);
			goto err;
		}
	}
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */

	if (state & Dee_THREAD_STATE_SHUTDOWNINTR) {
		/* We're supposed to terminate */
		return DeeError_Throw(&DeeError_Interrupt_instance);
	}

	/* Check for normal interrupts */
	if (atomic_read(&self->t_interrupt.ti_intr) != NULL) {
		DREF DeeObject *intr;
		DREF DeeTupleObject *args;
		struct thread_interrupt *next;
		_DeeThread_AcquireInterrupt(self);
		intr = self->t_interrupt.ti_intr;
		args = self->t_interrupt.ti_args;
		next = self->t_interrupt.ti_next;
		if (next) {
			memcpy(&self->t_interrupt, next, sizeof(struct thread_interrupt));
			/* Set the INTERRUPTED flag because there are more interrupts */
			atomic_or(&self->t_state, Dee_THREAD_STATE_INTERRUPTED);
		} else {
			self->t_interrupt.ti_intr = NULL;
			self->t_interrupt.ti_args = NULL;
			ASSERT(self->t_interrupt.ti_next == NULL);
		}
		_DeeThread_ReleaseInterrupt(self);
		if (intr != NULL) {
			int error;
			Dee_thread_interrupt_xfree(next);

			/* Service the interrupt */
			if (args == NULL) {
				error = DeeError_Throw(intr);
			} else if likely(args != (DREF DeeTupleObject *)ITER_DONE) {
				DREF DeeObject *result;
				result = DeeObject_CallTuple(intr, (DeeObject *)args);
				Dee_Decref(args);
				error = -1;
				if likely(result != NULL) {
					Dee_Decref(result);
					error = 0;
				}
			} else {
				/* Simply decref `intr' */
				error = 0;
			}
			Dee_Decref(intr);

			/* Check if handling of the interrupt caused an exception (if so: propagate it) */
			if (error != 0)
				return error;
		} else {
			ASSERT(next == NULL);
			ASSERT(args == NULL);
		}
	}

	/* Restore the system error context */
	DeeSystemError_Pop();

	/* Check if our interrupt flag has been set yet again. */
	goto again_read_state;
err:
	return -1;
}

/* Check for an interrupt exception and throw it if there is one.
 * This function should be called before any blocking system call and is
 * invoked by the interpreter before execution of any JMP-instruction, or
 * only those that jump backwards in code (aka. is guarantied to be checked
 * periodically during execution of any kind of infinite-loop). */
PUBLIC WUNUSED int (DCALL DeeThread_CheckInterrupt)(void) {
	return DeeThread_CheckInterruptSelf(DeeThread_Self());
}


#ifndef DeeThread_USE_SINGLE_THREADED

/* Forward an appexit interrupt to the main thread. */
PRIVATE NONNULL((1)) void DCALL
forward_appexit_to_main_thread(/*inherit(always)*/ struct thread_interrupt *__restrict interrupt) {
	uint32_t state;
	uintptr_t version;
	for (;;) {
		state = atomic_fetchor(&DeeThread_Main.ot_thread.t_state,
		                       Dee_THREAD_STATE_INTERRUPTING);
		if (!(state & Dee_THREAD_STATE_INTERRUPTING))
			break;
		if (state & Dee_THREAD_STATE_TERMINATING)
			goto already_terminated;
		SCHED_YIELD();
	}

	/* Check if the thread has already terminated (or is unmanaged). */
	if (state & Dee_THREAD_STATE_TERMINATING) {
		atomic_and(&DeeThread_Main.ot_thread.t_state, ~Dee_THREAD_STATE_INTERRUPTING);
		goto already_terminated;
	}

	/* Schedule the interrupt as pending for the target thread. */
	if (DeeThread_Main.ot_thread.t_interrupt.ti_intr == NULL) {
		/* Simple case: first interrupt */
		DeeThread_Main.ot_thread.t_interrupt.ti_intr = interrupt->ti_intr;
		DeeThread_Main.ot_thread.t_interrupt.ti_args = interrupt->ti_args;
	} else {
		/* Complicated case: secondary interrupt */
		struct thread_interrupt *prev;
		prev = &DeeThread_Main.ot_thread.t_interrupt;
		while (prev->ti_next)
			prev = prev->ti_next;
		prev->ti_next = interrupt;
		interrupt->ti_next = NULL;
		interrupt = NULL;
	}

	/* Release the interrupt-lock */
	atomic_and(&DeeThread_Main.ot_thread.t_state, ~Dee_THREAD_STATE_INTERRUPTING);

	/* Free an unused interrupt descriptor. */
	if (interrupt != NULL)
		_Dee_thread_interrupt_free(interrupt);

	/* Set the INTERRUPTED flag for the target thread, and force it to wake up. */
	version = atomic_read(&DeeThread_Main.ot_thread.t_int_vers);
	atomic_or(&DeeThread_Main.ot_thread.t_state, Dee_THREAD_STATE_INTERRUPTED);

	/* Wait for the thread to have handled its pending interrupts.
	 *
	 * NOTE: Because we've already created the interrupt at this
	 *       point, we must perform this wait without doing any
	 *       interrupt checks! */
	for (;;) {
		DeeThread_Wake((DeeObject *)&DeeThread_Main.ot_thread);
		state = atomic_read(&DeeThread_Main.ot_thread.t_state);
		if (!(state & Dee_THREAD_STATE_INTERRUPTED))
			break;
		if (!(state & Dee_THREAD_STATE_WAITING)) {
			atomic_or(&DeeThread_Main.ot_thread.t_state, Dee_THREAD_STATE_WAITING);
			state |= Dee_THREAD_STATE_WAITING;
		}
	
		/* Keep waking the thread in case it just went inside of a blocking system call. */
		DeeFutex_Wait32NoIntTimed(&DeeThread_Main.ot_thread.t_state, state, THREAD_WAKE_DELAY);
		if (version != atomic_read(&DeeThread_Main.ot_thread.t_int_vers))
			break; /* The thread checked for interrupts in the meantime */
	}
	return;
already_terminated:
	Dee_thread_interrupt_free(interrupt);
}


#ifdef DeeThread_USE_CreateThread
PRIVATE DWORD WINAPI DeeThread_Entry_func(void *arg)
#define LOCAL_thread_entry_return return 0
#elif defined(DeeThread_USE_pthread_create)
PRIVATE void *DeeThread_Entry_func(void *arg)
#define LOCAL_thread_entry_return return NULL
#else /* defined(DeeThread_USE_thrd_create) */
PRIVATE int DeeThread_Entry_func(void *arg)
#define LOCAL_thread_entry_return return 0
#endif /* ... */
{
	DREF DeeOSThreadObject *self = (DREF DeeOSThreadObject *)arg;
	DREF DeeObject *thread_main, *thread_args, *result;
	ASSERT_OBJECT_TYPE(&self->ot_thread, &DeeThread_Type);
	DBG_ALIGNMENT_DISABLE();

	/* On non-windows hosts, a thread must save its own thread-id. */
#undef LOCAL_thread_entry_did_set__ot_pid
#if defined(Dee_pid_t) && defined(DeeThread_GetCurrentTid) && !defined(DeeThread_USE_CreateThread)
#define LOCAL_thread_entry_did_set__ot_pid
	self->ot_tid = DeeThread_GetCurrentTid();
#endif /* Dee_pid_t && DeeThread_GetCurrentTid && !DeeThread_USE_CreateThread */

	/* Save the thread-self object in the TLS descriptor,
	 * letting that descriptor inherit its value. */
	thread_tls_set(&self->ot_thread);
	DBG_ALIGNMENT_ENABLE();

	/* Acquire necessary locks to register the thread and read out its main() function */
	thread_list_lock_acquire_noint();
	_DeeThread_AcquireSetup(&self->ot_thread);

	/* Insert the thread into the global list of threads */
	LIST_INSERT_HEAD(&thread_list, &self->ot_thread, t_global);
	thread_main = (DREF DeeObject *)self->ot_thread.t_inout.io_main;  /* Inherit reference */
	thread_args = (DREF DeeObject *)self->ot_thread.t_context.d_args; /* Inherit reference */
	DBG_memset(&self->ot_thread.t_inout.io_main, 0xcc,
	           sizeof(self->ot_thread.t_inout.io_main));
	self->ot_thread.t_context.d_tls = NULL;

	/* Set the thread's name if the OS provides a means to do so */
#ifdef DeeThread_SetName
	if (self->ot_thread.t_threadname) {
		DBG_ALIGNMENT_DISABLE();
		DeeThread_SetName(DeeString_STR(self->ot_thread.t_threadname));
		DBG_ALIGNMENT_ENABLE();
	} else if (DeeFunction_Check(thread_main)) {
		/* If DDI provides, set the name of the function that's to-be executed. */
		DeeFunctionObject *exec_function;
		DeeCodeObject *exec_code;
		char *exec_name;
		exec_function = (DeeFunctionObject *)thread_main;
		exec_code     = exec_function->fo_code;
		exec_name     = DeeCode_NAME(exec_code);
		if (exec_name) {
			DBG_ALIGNMENT_DISABLE();
			DeeThread_SetName(exec_name);
			DBG_ALIGNMENT_ENABLE();
		} else if (exec_code == exec_code->co_module->mo_root) {
			DBG_ALIGNMENT_DISABLE();
			DeeThread_SetName(DeeString_STR(exec_code->co_module->mo_name));
			DBG_ALIGNMENT_ENABLE();
		}
	}
#endif /* DeeThread_SetName */

	/* Confirm startup by setting the started-flag. */
#ifdef LOCAL_thread_entry_did_set__ot_pid
	atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_STARTED | Dee_THREAD_STATE_HASTID);
#else /* LOCAL_thread_entry_did_set__ot_pid */
	atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_STARTED);
#endif /* !LOCAL_thread_entry_did_set__ot_pid */
	_DeeThread_ReleaseSetup(&self->ot_thread);
	thread_list_lock_release();

	/* Tell our creator that we're now up-and-running.
	 * NOTE: We use `DeeFutex_WakeAll()' here instead of `_DeeThread_WakeWaiting()',
	 *       because there should always be someone that is waiting at this point.
	 *       (s.a. the impl of `DeeThread_Start()') */
	DeeFutex_WakeAll(&self->ot_thread.t_state);

	/* Before anything is actually executed, check for interrupts that
	 * may have been scheduled before the thread was even started.
	 * This way, the user can send interrupts before starting a thread
	 * and is allowed to assume that they will be dealt with (in order)
	 * before the thread's actual main method. */
	if (DeeThread_CheckInterruptSelf(&self->ot_thread))
		goto handle_thread_error_threadargs;

	/* Invoke the thread's main() callback. */
	if likely(thread_main) {
		result = DeeObject_CallTuple(thread_main, thread_args);
		Dee_Decref(thread_main);
	} else {
		/* If no thread-main callback has been assigned, invoke the `run()' member function. */
		result = DeeObject_CallAttrTuple((DeeObject *)&self->ot_thread,
		                                 (DeeObject *)&str_run,
		                                 thread_args);
	}
	Dee_Decref(thread_args);
	if unlikely(!result)
		goto handle_thread_error;

	/* Begin the process of terminating the thread (this will prevent further interrupts) */
	_DeeThread_AcquireInterrupt(&self->ot_thread);
	atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_TERMINATING);
	_DeeThread_ReleaseInterrupt(&self->ot_thread);

	/* Handle all remaining interrupts. */
	if (DeeThread_CheckInterruptSelf(&self->ot_thread)) {
		Dee_Decref(result);
		goto handle_thread_error;
	}

	/* Store the thread's return value */
do_cleanup_and_set_result:

	/* Clean-up TLS data (needs to happen in the original thread's context) */
	if (self->ot_thread.t_context.d_tls != NULL) {
		void *tls_data;
		tls_data = self->ot_thread.t_context.d_tls;
		self->ot_thread.t_context.d_tls = NULL;
		fini_tls_data(tls_data);
		goto do_cleanup_and_set_result;
	}

	/* Discard all interrupts that are still pending */
	if (self->ot_thread.t_interrupt.ti_intr) {
		DeeThread_DiscardAllInterrupts(&self->ot_thread);
		goto do_cleanup_and_set_result;
	}

	/* ==== POINT OF NO RETURN ====
	 *
	 * from this point forth, no deemon code may be executed by the thread anymore */

	/* Set-up the thread as having exited */
	_DeeThread_AcquireSetup(&self->ot_thread);
	ASSERT((self->ot_thread.t_except != NULL) == (self->ot_thread.t_exceptsz != 0));
	ASSERT((self->ot_thread.t_except != NULL) == (result == NULL));
	if (result != NULL)
		self->ot_thread.t_inout.io_result = result; /* Inherit reference */
	atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_TERMINATED);
	_DeeThread_ReleaseSetup(&self->ot_thread);

	_DeeThread_AcquireDetaching(&self->ot_thread);
	atomic_and(&self->ot_thread.t_state, ~(Dee_THREAD_STATE_DETACHING |
	                                       Dee_THREAD_STATE_HASOSCTX |
	                                       Dee_THREAD_STATE_INTERRUPTED));

	/* Wake up threads that may be waiting for our thread to terminate */
	_DeeThread_WakeWaiting(&self->ot_thread);

	if (LIST_ISBOUND(&self->ot_thread, t_global)) {
		if (thread_list_lock_tryacquire()) {
			if (LIST_ISBOUND(&self->ot_thread, t_global))
				LIST_UNBIND(&self->ot_thread, t_global);
			thread_list_lock_release();
		}
	}

	/* Drop the reference previously held by the TLS variable */
	DeeThread_DecrefInOtherThread(&self->ot_thread);

	LOCAL_thread_entry_return;
handle_thread_error_threadargs:
	Dee_Decref(thread_args);
handle_thread_error:
	if (self->ot_thread.t_exceptsz) {
		DeeObject *current;
		current = self->ot_thread.t_except->ef_error;
		/* Special case: Thread-exit exception. */
		if (DeeThreadExit_Check(current)) {
			result = DeeThreadExit_Result(current);
			Dee_Incref(result);
			DeeError_Handled(ERROR_HANDLED_INTERRUPT);
			while (DeeError_Catch(&DeeError_Interrupt))
				;
	
			/* If no further exceptions have occurred, set the thread-exit return value. */
			if (!DeeError_Current()) {
				_DeeThread_AcquireInterrupt(&self->ot_thread);
				atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_TERMINATING);
				_DeeThread_ReleaseInterrupt(&self->ot_thread);
				goto do_cleanup_and_set_result;
			}
			Dee_Decref(result);
		}

		/* Special case: App-exit exception. */
		if (DeeAppExit_Check(current)) {
			/* Send an RPC to the main thread and have *it* propagate the exit request. */
			struct except_frame *frame;
			struct thread_interrupt *interrupt;
			STATIC_ASSERT(sizeof(struct thread_interrupt) <=
			              sizeof(struct except_frame));
			frame = self->ot_thread.t_except;
			self->ot_thread.t_except = frame->ef_prev;
			--self->ot_thread.t_exceptsz;

			/* Convert the except frame into an interrupt frame. */
			if (ITER_ISOK(frame->ef_trace))
				Dee_Decref(frame->ef_trace);
			interrupt = (struct thread_interrupt *)frame;
			interrupt->ti_intr = current; /* Inherited from the except frame. */
			interrupt->ti_args = NULL;    /* Its an exception interrupt, so no callback args. */
			forward_appexit_to_main_thread(interrupt);

			/* Handle extra interrupt errors. */
			while (DeeError_Catch(&DeeError_Interrupt))
				;

			/* If no further exceptions have occurred, have the thread return with `none'.
			 * Note however that this shouldn't *really* matter, since the main thread is
			 * supposed to die anyways, however by not throwing more errors into the mix,
			 * we *may* be able to make things easier. */
			if (!DeeError_Current()) {
				_DeeThread_AcquireInterrupt(&self->ot_thread);
				atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_TERMINATING);
				_DeeThread_ReleaseInterrupt(&self->ot_thread);
				result = DeeNone_NewRef();
				goto do_cleanup_and_set_result;
			}
		}
	}

	/* Special case: Catch any remaining errors that are based on `Signal.Interrupt'.
	 *               When a thread terminated due to an interrupt error, its not
	 *               considered to have crashed, but rather to have terminated normally.
	 *               This behavior is special to only `Signal.Interrupt' and not all
	 *               types with the `TP_FINTERRUPT' flag set because other types with
	 *               that flag set are user-defined, implying that they're probably
	 *               being used to signify some rare, yet still important situation
	 *               where normally exceptions wouldn't do, rather than also being used
	 *               to prematurely signal termination of a thread, as would be the case
	 *               when `Signal.Interrupt' would have been used instead. */
	while (DeeError_Catch(&DeeError_Interrupt))
		;

	/* Indicate that the thread is now terminating */
	_DeeThread_AcquireInterrupt(&self->ot_thread);
	atomic_or(&self->ot_thread.t_state, Dee_THREAD_STATE_TERMINATING);
	_DeeThread_ReleaseInterrupt(&self->ot_thread);
	result = NULL;

	/* Special case: if no exception is set anymore (i.e. if `DeeError_Interrupt' was caught),
	 *               then the thread is supposed to return `none'! */
	if (!DeeError_Current())
		result = DeeNone_NewRef();

	goto do_cleanup_and_set_result;
}
#undef LOCAL_thread_entry_did_set__ot_pid
#undef LOCAL_thread_entry_return


PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeThread_Start_impl(/*inherit(on_success)*/ DREF DeeOSThreadObject *__restrict self) {
#ifdef DeeThread_USE_CreateThread
	DWORD dwError;
	HANDLE hThread;
again:
	DBG_ALIGNMENT_DISABLE();
#ifdef Dee_pid_t
	hThread = CreateThread(NULL, 0, &DeeThread_Entry_func, self, 0, (LPDWORD)&self->ot_tid);
#else /* Dee_pid_t */
	hThread = CreateThread(NULL, 0, &DeeThread_Entry_func, self, 0, NULL);
#endif /* !Dee_pid_t */
	if likely(hThread != NULL) {
		DBG_ALIGNMENT_ENABLE();
		self->ot_hThread = hThread;
		return 0;
	}
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
			goto again;
		return -1;
	}
	return DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
	                               "Failed to start thread %k", self);
#endif /* DeeThread_USE_CreateThread */

#ifdef DeeThread_USE_pthread_create
	int error;
#if defined(ENOMEM) || defined(EAGAIN)
again:
#endif /* ENOMEM || EAGAIN */
	DBG_ALIGNMENT_DISABLE();
	error = pthread_create(&self->ot_pthread, NULL, &DeeThread_Entry_func, self);
	DBG_ALIGNMENT_ENABLE();
	if likely(error == 0)
		return 0;
#if defined(ENOMEM) || defined(EAGAIN)
	DeeSystem_IF_E2(error, ENOMEM, EAGAIN, {
		if (Dee_CollectMemory(1))
			goto again;
		return -1;
	});
#endif /* ENOMEM || EAGAIN */
	return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
	                                 "Failed to start thread %k", self);
#endif /* DeeThread_USE_pthread_create */

#ifdef DeeThread_USE_thrd_create
	int error;
#ifdef CONFIG_HAVE_thrd_nomem
again:
#endif /* CONFIG_HAVE_thrd_nomem */
	DBG_ALIGNMENT_DISABLE();
	error = thrd_create(&self->ot_thrd, &DeeThread_Entry_func, self);
	DBG_ALIGNMENT_ENABLE();
	if likely(error == thrd_success)
		return 0;
#ifdef CONFIG_HAVE_thrd_nomem
	if (error == thrd_nomem) {
		if (Dee_CollectMemory(1))
			goto again;
		return -1;
	}
#endif /* CONFIG_HAVE_thrd_nomem */
	return DeeError_Throwf(&DeeError_SystemError,
	                       "Failed to start thread %k", self);
#endif /* DeeThread_USE_thrd_create */

#ifdef DeeThread_USE_SINGLE_THREADED
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unable to start thread %k",
	                       self);
#endif /* DeeThread_USE_SINGLE_THREADED */
}
#endif /* !DeeThread_USE_SINGLE_THREADED */

/* Start execution of the given thread.
 * @return:  0: Successfully started the thread.
 * @return:  1: The thread had already been started.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS') */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeThread_Start(/*Thread*/ DeeObject *__restrict self) {
#ifdef DeeThread_USE_SINGLE_THREADED
	(void)self;
	return 1; /* Both the main thread, as well as unmanaged threads are always already-started */
#else /* DeeThread_USE_SINGLE_THREADED */
	uint32_t state;
	DeeThreadObject *caller = DeeThread_Self();
	DeeOSThreadObject *me = DeeThread_AsOSThread((DeeThreadObject *)self);
	ASSERT_OBJECT_TYPE(&me->ot_thread, &DeeThread_Type);

	/* Check if `me' has already been started. */
again:
	state = atomic_fetchor(&me->ot_thread.t_state, Dee_THREAD_STATE_STARTING);
	if (state & Dee_THREAD_STATE_STARTING) {
		if (state & Dee_THREAD_STATE_STARTED)
			return 1;

		/* Wait until the thread has either fully started,
		 * or the other thread stops its start attempt. */
		if (DeeFutex_Wait32(&me->ot_thread.t_state, state))
			goto err;
		goto again;
	}

	/* If the calling thread is currently terminating, then it's not
	 * allowed to spawn new threads. Instead, throw an `Interrupt()'
	 * exception to urge the caller to keep unwinding their stack.
	 *
	 * We also do the same if we've received a shutdown interrupt
	 * (as the result of the deemon runtime being shut down). */
	if (caller->t_state & (Dee_THREAD_STATE_TERMINATING |
	                       Dee_THREAD_STATE_SHUTDOWNINTR)) {
		atomic_and(&me->ot_thread.t_state, ~Dee_THREAD_STATE_STARTING);
		_DeeThread_WakeWaiting(&me->ot_thread);
		return DeeError_Throw(&DeeError_Interrupt_instance);
	}

	/* Create the reference that is passed to `DeeThread_Entry_func()'
	 * and later stored in the thread's TLS self-pointer. */
	Dee_Incref(&me->ot_thread); /* Inherited by `DeeThread_Start_impl()' */

	/* Do OS-specific stuff needed to start a new thread */
	if unlikely(DeeThread_Start_impl(me))
		goto err_abort;

	/* With the OS-specific thread created, indicate that we have a thread descriptor. */
	atomic_or(&me->ot_thread.t_state, Dee_THREAD_STATE_HASTHREAD);

	/* Wait for the thread to complete its startup-sequence
	 * (and to appear in the global list of threads)
	 *
	 * NOTE: Because we've already created the thread at this
	 *       point, we must perform this wait without doing
	 *       any interrupt checks! */
	for (;;) {
		state = atomic_read(&me->ot_thread.t_state);
		ASSERT(state & Dee_THREAD_STATE_STARTING);
		if (state & Dee_THREAD_STATE_STARTED)
			break;
		if (!(state & Dee_THREAD_STATE_WAITING)) {
			atomic_or(&me->ot_thread.t_state, Dee_THREAD_STATE_WAITING);
			state |= Dee_THREAD_STATE_WAITING;
		}
		DeeFutex_Wait32NoInt(&me->ot_thread.t_state, state);
	}

	/* Indicate success: the thread is now running! */
	return 0;
err_abort:
	Dee_DecrefNokill(&me->ot_thread);
	atomic_and(&me->ot_thread.t_state, ~Dee_THREAD_STATE_STARTING);
	/* Wake up other threads that may be trying to start the thread. */
	_DeeThread_WakeWaiting(&me->ot_thread);
err:
	return -1;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}



#ifdef DeeThread_Wake_USE_QueueUserAPC__AND__CancelSynchronousIo
PRIVATE VOID NTAPI dummy_apc_func(ULONG_PTR Parameter) {
	(void)Parameter;
	Dee_DPRINTF("dummy_apc_func called in %k\n", DeeThread_Self());
}
#endif /* DeeThread_Wake_USE_QueueUserAPC__AND__CancelSynchronousIo */


INTDEF NONNULL((1)) void DCALL
DeeFutex_WakeGlobal(DeeThreadObject *thread);

/* Try to wake the thread. This will:
 * - Interrupt a currently running, blocking system call (unless
 *   that call is specifically being made as uninterruptible)
 * - Force the thread to return from a call to `DeeFutex_Wait*'
 * - Cause the thread to soon call `DeeThread_CheckInterrupt()' */
PUBLIC NONNULL((1)) void DCALL
DeeThread_Wake(/*Thread*/ DeeObject *__restrict self) {
#ifdef DeeThread_USE_SINGLE_THREADED
	/* No-op in all situations */
	(void)self;
	COMPILER_IMPURE();
#else /* DeeThread_USE_SINGLE_THREADED */
	uint32_t state;
	DeeOSThreadObject *me = DeeThread_AsOSThread((DeeThreadObject *)self);
	ASSERT_OBJECT_TYPE(&me->ot_thread, &DeeThread_Type);

	/* Wake up all threads that are currently waiting on a futex.
	 * This is required for (some of) the futex implementations,
	 * as a couple of them don't respond to EINTR-like events.
	 *
	 * We work around this issue by (essentially) keeping track
	 * of all of the addresses that threads are blocking-waiting
	 * for, and explicitly waking up all of those threads. */
	DeeFutex_WakeGlobal(&me->ot_thread);

	/* Ensure that the thread can't be detached at the moment. */
	for (;;) {
		state = atomic_fetchor(&me->ot_thread.t_state, Dee_THREAD_STATE_DETACHING);
		if (!(state & Dee_THREAD_STATE_DETACHING)) {
			/* Must not do anything to unmanaged threads. */
			if unlikely(state & Dee_THREAD_STATE_UNMANAGED) {
				atomic_and(&me->ot_thread.t_state, ~Dee_THREAD_STATE_DETACHING);
				return;
			}
			break;
		}
		if unlikely(state & Dee_THREAD_STATE_UNMANAGED)
			return;
		SCHED_YIELD();
	}

#ifdef DeeThread_Wake_USE_QueueUserAPC__AND__CancelSynchronousIo
	if (state & Dee_THREAD_STATE_HASTHREAD) {
		typedef int (WINAPI *LPCANCELSYNCHRONOUSIO)(HANDLE hThread);
		PRIVATE LPCANCELSYNCHRONOUSIO pCancelSynchronousIo = NULL;
		DBG_ALIGNMENT_DISABLE();
		DeeSystemError_Push();
		(void)QueueUserAPC(&dummy_apc_func, me->ot_hThread, 0);

		/* Also try to interrupt synchronous I/O, meaning calls like `ReadFile()'.
		 * Sadly, we must manually check if that functionality is even available... */
		if (ITER_ISOK(pCancelSynchronousIo)) {
			(void)((*pCancelSynchronousIo)(me->ot_hThread));
		} else if (!pCancelSynchronousIo) {
			LPCANCELSYNCHRONOUSIO ptr;
			static WCHAR const wKernel32[] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
			*(void **)&ptr = GetProcAddress(GetModuleHandleW(wKernel32), "CancelSynchronousIo");
			if (!ptr)
				*(void **)&ptr = (void *)ITER_DONE;
			pCancelSynchronousIo = ptr;
			if (ptr != (void *)ITER_DONE)
				(void)((*ptr)(me->ot_hThread));
		}
		DeeSystemError_Pop();
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* DeeThread_Wake_USE_QueueUserAPC__AND__CancelSynchronousIo */

#ifdef DeeThread_Wake_USE_pthread_kill
	if (state & Dee_THREAD_STATE_HASTHREAD) {
		DBG_ALIGNMENT_DISABLE();
		(void)pthread_kill(me->ot_pthread, DeeThread_Wake_USED_SIGNAL);
		DBG_ALIGNMENT_ENABLE();
	} else
#elif defined(DeeThread_Wake_USE_pthread_sigqueue)
	if (state & Dee_THREAD_STATE_HASTHREAD) {
		union sigval sv;
		DBG_ALIGNMENT_DISABLE();
		bzero(&sv, sizeof(sv));
		(void)pthread_sigqueue(me->ot_pthread, DeeThread_Wake_USED_SIGNAL, sv);
		DBG_ALIGNMENT_ENABLE();
	} else
#endif /* ... */
	{
#ifdef DeeThread_Wake_USE_tgkill
		if (state & Dee_THREAD_STATE_HASTID) {
			DBG_ALIGNMENT_DISABLE();
			(void)tgkill(getpid(), me->ot_tid, DeeThread_Wake_USED_SIGNAL);
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* DeeThread_Wake_USE_tgkill */
#ifdef DeeThread_Wake_USE_kill
		if (state & Dee_THREAD_STATE_HASTID) {
			DBG_ALIGNMENT_DISABLE();
			(void)kill(me->ot_tid, DeeThread_Wake_USED_SIGNAL);
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* DeeThread_Wake_USE_kill */
	}

	/* Release the DETACHING lock. */
	atomic_and(&me->ot_thread.t_state, ~Dee_THREAD_STATE_DETACHING);
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

/* Schedule an interrupt for a given thread.
 * Interrupts are received when a thread calls `DeeThread_CheckInterrupt()'.
 * NOTE: Interrupts are received in order of being sent.
 * NOTE: When `interrupt_args' is non-NULL, rather than throwing the given
 *       `interrupt_main' as an error upon arrival, it is invoked using
 *       `operator ()' with `interrupt_args' (which must be a tuple).
 * @return:  1: The thread has been terminated.
 * @return:  0: Successfully scheduled the interrupt object.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS') */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Interrupt(/*Thread*/ DeeObject *self,
                    DeeObject *interrupt_main,
                    DeeObject *interrupt_args) {
	uint32_t state;
#ifndef DeeThread_USE_SINGLE_THREADED
	uintptr_t version;
#endif /* !DeeThread_USE_SINGLE_THREADED */
	struct thread_interrupt *interrupt;
	DeeThreadObject *caller;
	DeeThreadObject *me;
	interrupt = NULL;
	caller    = DeeThread_Self();
	me        = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
	ASSERT_OBJECT_TYPE_EXACT_OPT(interrupt_args, &DeeTuple_Type);

again:
	for (;;) {
		state = atomic_fetchor(&me->t_state, Dee_THREAD_STATE_INTERRUPTING);
		if (!(state & Dee_THREAD_STATE_INTERRUPTING))
			break;
		if (state & Dee_THREAD_STATE_TERMINATING)
			goto already_terminated;
		SCHED_YIELD();
	}

	/* Check if the thread has already terminated (or is unmanaged). */
	if (state & (Dee_THREAD_STATE_TERMINATING | Dee_THREAD_STATE_UNMANAGED)) {
		atomic_and(&me->t_state, ~Dee_THREAD_STATE_INTERRUPTING);
		if unlikely(state & Dee_THREAD_STATE_UNMANAGED) {
			DeeError_Throwf(&DeeError_ValueError, "Cannot interrupt unmanaged thread");
			goto err;
		}
		goto already_terminated;
	}

	/* In order to send interrupts, you can't have any yourself,
	 * unless you're trying to send an interrupt to yourself. */
#ifndef DeeThread_USE_SINGLE_THREADED
	if ((atomic_read(&caller->t_state) & Dee_THREAD_STATE_INTERRUPTED) &&
	    (caller != me)) {
		atomic_and(&me->t_state, ~Dee_THREAD_STATE_INTERRUPTING);
		if (DeeThread_CheckInterruptSelf(caller))
			goto err_free_interrupt;
		goto again;
	}
#endif /* !DeeThread_USE_SINGLE_THREADED */

	/* Schedule the interrupt as pending for the target thread. */
	if (me->t_interrupt.ti_intr == NULL) {
		/* Simple case: first interrupt */
		me->t_interrupt.ti_intr = interrupt_main;
		me->t_interrupt.ti_args = (DREF DeeTupleObject *)interrupt_args;
	} else {
		/* Complicated case: secondary interrupt */
		struct thread_interrupt *prev;
		if (interrupt == NULL) {
			/* Must allocate a new descriptor. */
			atomic_and(&me->t_state, ~Dee_THREAD_STATE_INTERRUPTING);
			interrupt = Dee_thread_interrupt_alloc();
			if unlikely(!interrupt)
				goto err;
			goto again;
		}
		prev = &me->t_interrupt;
		while (prev->ti_next)
			prev = prev->ti_next;
		prev->ti_next = interrupt;
		interrupt->ti_intr = interrupt_main;
		interrupt->ti_args = (DREF DeeTupleObject *)interrupt_args;
		interrupt->ti_next = NULL;
	}

	/* References as inherited by the interrupt descriptor. */
	Dee_Incref(interrupt_main);
	Dee_XIncref(interrupt_args);

	/* Release the interrupt-lock */
	atomic_and(&me->t_state, ~Dee_THREAD_STATE_INTERRUPTING);

	/* Free an unused interrupt descriptor. */
	Dee_thread_interrupt_xfree(interrupt);

	/* Set the INTERRUPTED flag for the target thread, and force it to wake up. */
#ifndef DeeThread_USE_SINGLE_THREADED
	version = atomic_read(&me->t_int_vers);
#endif /* !DeeThread_USE_SINGLE_THREADED */
	atomic_or(&me->t_state, Dee_THREAD_STATE_INTERRUPTED);

	/* Wait for the thread to have handled its pending interrupts.
	 *
	 * NOTE: Because we've already created the interrupt at this
	 *       point, we must perform this wait without doing any
	 *       interrupt checks! */
#ifndef DeeThread_USE_SINGLE_THREADED
	if (me != caller) {
		for (;;) {
			DeeThread_Wake((DeeObject *)me);
			state = atomic_read(&me->t_state);
			if (!(state & Dee_THREAD_STATE_INTERRUPTED))
				break;
			if (!(state & Dee_THREAD_STATE_WAITING)) {
				atomic_or(&me->t_state, Dee_THREAD_STATE_WAITING);
				state |= Dee_THREAD_STATE_WAITING;
			}
	
			/* Keep waking the thread in case it just went inside of a blocking system call. */
			DeeFutex_Wait32NoIntTimed(&me->t_state, state, THREAD_WAKE_DELAY);
			if (version != atomic_read(&me->t_int_vers))
				break; /* The thread checked for interrupts in the meantime */
		}
	}
#endif /* !DeeThread_USE_SINGLE_THREADED */

	return 0;
already_terminated:
	Dee_thread_interrupt_xfree(interrupt);
	return 1;
#ifndef DeeThread_USE_SINGLE_THREADED
err_free_interrupt:
	Dee_thread_interrupt_xfree(interrupt);
#endif /* !DeeThread_USE_SINGLE_THREADED */
err:
	return -1;
}

/* Detach the given thread (no-op if not possible, or already done).
 * @return:  1: Already detached or not yet started
 * @return:  0: Successfully detached
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeThread_Detach(/*Thread*/ DeeObject *__restrict self) {
#ifndef Dee_THREAD_STATE_HASOSCTX
	uint32_t state;
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(me, &DeeThread_Type);
	state = atomic_read(&me->t_state);
	if (!(state & Dee_THREAD_STATE_UNMANAGED))
		return 1;
#else /* !Dee_THREAD_STATE_HASOSCTX */
	uint32_t state;
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(me, &DeeThread_Type);
again:
	state = atomic_fetchor(&me->t_state, Dee_THREAD_STATE_DETACHING);
	if (state & Dee_THREAD_STATE_DETACHING) {
		if (!(state & Dee_THREAD_STATE_HASOSCTX))
			return 1; /* Already detached, or not started */
		if (state & Dee_THREAD_STATE_UNMANAGED)
			goto err_cannot_detach_unmanaged; /* Not allowed to detach unmanaged threads */
		SCHED_YIELD();
		goto again;
	}
	if unlikely(state & Dee_THREAD_STATE_UNMANAGED)
		goto err_cannot_detach_and_unlock; /* Not allowed to detach unmanaged threads */
#ifdef DeeThread_Detach_system_impl
	if (state & Dee_THREAD_STATE_HASTHREAD)
		DeeThread_Detach_system_impl(me);
#endif /* DeeThread_Detach_system_impl */
	atomic_and(&me->t_state, ~(Dee_THREAD_STATE_DETACHING |
	                           Dee_THREAD_STATE_HASOSCTX));
	return 0;
err_cannot_detach_and_unlock:
	atomic_and(&me->t_state, ~Dee_THREAD_STATE_DETACHING);
err_cannot_detach_unmanaged:
#endif /* Dee_THREAD_STATE_HASOSCTX */
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot detach unmanaged thread %k",
	                       self);
}


/* Same as `DeeThread_Join()', but don't return the thread's result,
 * or propagate its failing exception. Instead, simply wait for the
 * thread to terminate.
 * @return: 1 : The given timeout has expired.
 * @return: 0 : The thread has now terminated.
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeThread_WaitFor(/*Thread*/ DeeObject *__restrict self,
                  uint64_t timeout_nanoseconds) {
#ifndef DeeThread_USE_SINGLE_THREADED
	uint32_t state;
#endif /* !DeeThread_USE_SINGLE_THREADED */
	DeeThreadObject *me = (DeeThreadObject *)self;
	(void)timeout_nanoseconds;
	if unlikely(me == DeeThread_Self()) {
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Dead-lock detected: Thread %k tried to join itself",
		                       me);
	}

#ifdef DeeThread_USE_SINGLE_THREADED
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot wait for unmanaged thread %k",
	                       me);
#else /* DeeThread_USE_SINGLE_THREADED */
	/* Wait for the thread to terminate. */
	state = atomic_read(&me->t_state);
	if (!(state & Dee_THREAD_STATE_TERMINATED)) {
		if (state & Dee_THREAD_STATE_UNMANAGED) {
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Cannot wait for unmanaged thread %k",
			                       me);
		}
		if (timeout_nanoseconds == 0)
			goto timeout; /* Try-join */
		if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
			do {
				if (!(state & Dee_THREAD_STATE_WAITING)) {
					atomic_or(&me->t_state, Dee_THREAD_STATE_WAITING);
					state |= Dee_THREAD_STATE_WAITING;
				}
				if (DeeFutex_Wait32(&me->t_state, state))
					goto err;
				state = atomic_read(&me->t_state);
			} while (!(state & Dee_THREAD_STATE_TERMINATED));
		} else {
			int error;
			uint64_t now_microseconds, then_microseconds;
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
				goto do_infinite_timeout;
do_wait_with_timeout:
			if (!(state & Dee_THREAD_STATE_WAITING)) {
				atomic_or(&me->t_state, Dee_THREAD_STATE_WAITING);
				state |= Dee_THREAD_STATE_WAITING;
			}
			error = DeeFutex_WaitIntTimed(&me->t_state, state, timeout_nanoseconds);
			if unlikely(error != 0)
				return error; /* Timeout or error */
			state = atomic_read(&me->t_state);
			if (!(state & Dee_THREAD_STATE_TERMINATED)) {
				now_microseconds = DeeThread_GetTimeMicroSeconds();
				if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
					goto timeout; /* Timeout */
				timeout_nanoseconds *= 1000;
				goto do_wait_with_timeout;
			}
		}
	}
	return 0;
timeout:
	return 1;
err:
	return -1;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}


#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE WUNUSED NONNULL((1)) struct except_frame *DCALL
except_frame_copy_for_rethrow_or_unlock(struct except_frame *__restrict self,
                                        DeeThreadObject *__restrict thread) {
	struct except_frame *result;
	DeeErrorObject *thread_crash;
	result = Dee_except_frame_tryalloc();
	if unlikely(!result) {
		_DeeThread_ReleaseSetup(thread);
		if (Dee_CollectMemory(sizeof(struct Dee_except_frame)))
			return (struct except_frame *)ITER_DONE;
		goto err;
	}
	thread_crash = DeeObject_TRYMALLOC(DeeErrorObject);
	if unlikely(!thread_crash) {
		_DeeThread_ReleaseSetup(thread);
		if (Dee_CollectMemory(sizeof(DeeErrorObject))) {
			except_frame_free(result);
			return (struct except_frame *)ITER_DONE;
		}
		goto err_result;
	}

	/* Duplicate exception tracebacks and package errors in `Error.ThreadCrash'. */
	DeeObject_Init(thread_crash, &DeeError_ThreadCrash);
	thread_crash->e_message = NULL;
	thread_crash->e_inner   = self->ef_error;
	result->ef_trace = self->ef_trace;
	result->ef_error = (DREF DeeObject *)thread_crash;
	Dee_Incref(self->ef_error);
	if (ITER_ISOK(self->ef_trace))
		Dee_Incref(self->ef_trace);
	return result;
err_result:
	except_frame_free(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
except_frame_free_for_rethrow(struct except_frame *__restrict self) {
	DeeObject_FREE((DeeErrorObject *)self->ef_error);
	Dee_DecrefNokill(&DeeError_ThreadCrash);
	except_frame_free(self);
}

PRIVATE NONNULL((1)) bool DCALL
DeeThread_RethrowExceptionsOrUnlock(DeeThreadObject *__restrict self) {
	DeeThreadObject *caller;
	struct except_frame *frames, *dst, *src;
	ASSERT(self->t_except != NULL);
	ASSERT(self->t_exceptsz != 0);
	src    = self->t_except;
	frames = except_frame_copy_for_rethrow_or_unlock(src, self);
	if unlikely(!ITER_ISOK(frames))
		return frames != (struct except_frame *)ITER_DONE;
	dst = frames;
	while ((src = src->ef_prev) != NULL) {
		struct except_frame *next_copy;
		next_copy = except_frame_copy_for_rethrow_or_unlock(src, self);
		if unlikely(!ITER_ISOK(next_copy)) {
			for (;;) {
				except_frame_free_for_rethrow(frames);
				if (frames == dst)
					break;
				frames = frames->ef_prev;
			}
			return next_copy != (struct except_frame *)ITER_DONE;
		}
		dst->ef_prev = next_copy;
		dst = dst->ef_prev;
	}

	/* Insert the copied exception chain into the calling thread. */
	caller = DeeThread_Self();
	dst->ef_prev     = caller->t_except;
	caller->t_except = frames;
	caller->t_exceptsz += self->t_exceptsz;
	return true;
}
#endif /* !DeeThread_USE_SINGLE_THREADED */

/* Join the given thread.
 * @return: ITER_DONE: The given timeout has expired. (never returned for `(uint64_t)-1')
 * @return: * :   Successfully joined the thread (return value is the thread's return)
 * @return: NULL: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 *                NOTE: If the thread crashed, its errors are propagated into the calling
 *                      thread after being encapsulated as `Error.ThreadError' objects.
 * @param: timeout_nanoseconds: The timeout in microseconds, 0 for try-join,
 *                              or `(uint64_t)-1' for infinite timeout. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeThread_Join(/*Thread*/ DeeObject *__restrict self,
               uint64_t timeout_nanoseconds) {
#ifdef DeeThread_USE_SINGLE_THREADED
	DeeThread_WaitFor(self, timeout_nanoseconds);
	return NULL;
#else /* DeeThread_USE_SINGLE_THREADED */
	int waitfor_status;
	DeeThreadObject *me = (DeeThreadObject *)self;

	/* Wait for the thread to terminate. */
	waitfor_status = DeeThread_WaitFor((DeeObject *)me, timeout_nanoseconds);
	if (waitfor_status != 0) {
		if (waitfor_status > 0)
			return ITER_DONE;
		goto err;
	}

	/* At this point, the thread has exited! */
	ASSERT(atomic_read(&me->t_state) & Dee_THREAD_STATE_TERMINATED);

	/* Check if the thread exited with a return value. */
again_acquire_setup:
	_DeeThread_AcquireSetup(me);
	if likely(me->t_exceptsz == 0) {
		DREF DeeObject *result;
		result = me->t_inout.io_result;
		Dee_Incref(result);
		_DeeThread_ReleaseSetup(me);
		return result;
	}

	/* Try to re-throw exceptions thrown in the context of the target thread. */
	if (!DeeThread_RethrowExceptionsOrUnlock(me))
		goto again_acquire_setup;
	_DeeThread_ReleaseSetup(me);
err:
	return NULL;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}


#ifdef Dee_pid_t
/* Lookup the thread-id of a given thread. Returns `0' when
 * the thread hasn't started, or has already terminated. */
PUBLIC WUNUSED NONNULL((1)) Dee_pid_t DCALL
DeeThread_GetTid(/*Thread*/ DeeObject *__restrict self) {
	Dee_pid_t result;
	uint32_t state;
	DeeOSThreadObject *me = DeeThread_AsOSThread((DeeThreadObject *)self);

	/* Read the PID, then check if it's still valid. */
	result = atomic_read(&me->ot_tid);
	state  = atomic_read(&me->ot_thread.t_state);
	if (!(state & Dee_THREAD_STATE_HASTID))
		result = 0;

	/* Give the caller the TID */
	return result;
}
#endif /* Dee_pid_t */

/* Clear all TLS variables assigned to slots in the calling thread.
 * @return: true:  The TLS descriptor table has been finalized.
 * @return: false: No TLS descriptor table had been assigned. */
INTERN bool DCALL DeeThread_ClearTls(void) {
	DeeThreadObject *caller;
	bool result = false;
	DBG_ALIGNMENT_DISABLE();
	caller = (DeeThreadObject *)thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if likely(caller) {
		void *data = caller->t_context.d_tls;
		if (data) {
			caller->t_context.d_tls = NULL;
			fini_tls_data(data);
			result = true;
		}
	}
	return result;
}


#undef HAVE_thread_clear
#ifdef DeeThread_USE_SINGLE_THREADED
/* Running threads are always reachable, and thus must never be GC-cleared.
 * And since the main thread is always running, there is no situation where
 * a thread needs to be cleared in the single-threaded configuration. */
#else /* DeeThread_USE_SINGLE_THREADED */
#define HAVE_thread_clear
PRIVATE NONNULL((1)) void DCALL
thread_clear(DeeThreadObject *__restrict self) {
	/* NOTE: We're allowed to assume that the thread is not running,
	 *       though we're not allowed to assume that it won't start
	 *       running in a moment (i.e. it may not have started yet) */
	uint32_t state;
	struct thread_interrupt old_intr;
	struct except_frame *old_except;
	DREF DeeObject *old_objects[2];

	/* Capture interrupts. */
	_DeeThread_AcquireInterrupt(self);
	memcpy(&old_intr, &self->t_interrupt, sizeof(struct thread_interrupt));
	self->t_interrupt.ti_next = NULL;
	self->t_interrupt.ti_intr = NULL;
	self->t_interrupt.ti_args = NULL;
	_DeeThread_ReleaseInterrupt(self);

	/* Decref() all pending interrupts. */
	if (old_intr.ti_intr) {
		for (;;) {
			struct thread_interrupt *next;
			ASSERT(old_intr.ti_intr);
			next = old_intr.ti_next;
			if (ITER_ISOK(old_intr.ti_args))
				Dee_Decref(old_intr.ti_args);
			Dee_Decref(old_intr.ti_intr);
			if (!next)
				break;
			memcpy(&old_intr, next, sizeof(struct thread_interrupt));
			Dee_thread_interrupt_free(next);
		}
	}

	/* Capture context, callback, and args */
	old_except = NULL;
	bzero(old_objects, sizeof(old_objects));
	_DeeThread_AcquireSetup(self);
	state = atomic_read(&self->t_state);
	if (state & Dee_THREAD_STATE_STARTED) {
		if (state & Dee_THREAD_STATE_TERMINATED) {
			if (self->t_exceptsz) {
				/* Steal exception context. */
				old_objects[0]   = NULL;
				old_except       = self->t_except;
				self->t_exceptsz = 0;
				self->t_except   = NULL;

				/* When setting `t_except' to NULL, the thread is implicitly changed
				 * such that it returned without an exception. To keep the thread in
				 * a valid state, we need to set a return value (which we use `none'
				 * for). */
			} else {
				old_objects[0] = self->t_inout.io_result; /* Inherit reference */
			}
			self->t_inout.io_result = Dee_None;
			Dee_Incref(Dee_None);
		}
	} else {
		old_objects[0] = self->t_inout.io_main;                    /* Inherit reference */
		old_objects[1] = (DREF DeeObject *)self->t_context.d_args; /* Inherit reference */
		self->t_inout.io_main  = NULL;
		self->t_context.d_args = (DREF struct Dee_tuple_object *)Dee_EmptyTuple;
		Dee_Incref(Dee_EmptyTuple);
	}
	_DeeThread_ReleaseSetup(self);

	/* Kill context objects. */
	Dee_XDecref(old_objects[0]);
	Dee_XDecref(old_objects[1]);
	while (old_except) {
		struct except_frame *prev;
		prev = old_except->ef_prev;
		Dee_Decref(old_except->ef_error);
		if (ITER_ISOK(old_except->ef_trace))
			Dee_Decref(old_except->ef_trace);
		except_frame_free(old_except);
		old_except = prev;
	}
}
#endif /* !DeeThread_USE_SINGLE_THREADED */

PRIVATE NONNULL((1, 2)) void DCALL
thread_visit(DeeThreadObject *__restrict self, dvisit_t proc, void *arg) {
	uint32_t state;
	struct thread_interrupt *iter;

	/* Visit pending interrupts. */
	_DeeThread_AcquireInterrupt(self);
	if (self->t_interrupt.ti_intr) {
		state = atomic_read(&self->t_state);
		if (!(self->t_state & Dee_THREAD_STATE_TERMINATING)) {
			iter = &self->t_interrupt;
			do {
				Dee_Visit(iter->ti_intr);
				if (ITER_ISOK(iter->ti_args))
					Dee_Visit(iter->ti_args);
			} while ((iter = iter->ti_next) != NULL);
		}
	}
	_DeeThread_ReleaseInterrupt(self);

	/* Visit runtime context. */
#ifndef DeeThread_USE_SINGLE_THREADED
	_DeeThread_AcquireSetup(self);
	state = atomic_read(&self->t_state);
	if (state & Dee_THREAD_STATE_STARTED) {
		if (state & Dee_THREAD_STATE_TERMINATED) {
			if (self->t_exceptsz) {
				struct except_frame *except_iter;
				for (except_iter = self->t_except; except_iter;
				     except_iter = except_iter->ef_prev) {
					Dee_Visit(except_iter->ef_error);
					if (ITER_ISOK(except_iter->ef_trace))
						Dee_Visit(except_iter->ef_trace);
				}
			} else {
				Dee_Visit(self->t_inout.io_result);
			}
		}
	} else {
		Dee_XVisit(self->t_inout.io_main);
		Dee_Visit(self->t_context.d_args);
	}
	_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

PRIVATE NONNULL((1)) void DCALL
thread_fini(DeeThreadObject *__restrict self) {
	ASSERT(!self->t_exec);
	ASSERT(!self->t_execsz);
	ASSERT(!self->t_str_curr);
	ASSERT(!self->t_repr_curr);
	ASSERT(!self->t_hash_curr);

#ifdef DeeThread_USE_SINGLE_THREADED
	/* Only unmanaged threads can get here */
	ASSERT(self->t_state & Dee_THREAD_STATE_UNMANAGED);
#ifndef CONFIG_NO_THREADS
	ASSERT(!LIST_ISBOUND(self, t_global));
#endif /* !CONFIG_NO_THREADS */
	ASSERT(self->t_deepassoc.da_used == 0);
	ASSERT(self->t_deepassoc.da_mask == 0);
	ASSERT(self->t_deepassoc.da_list == empty_deep_assoc);
#ifndef CONFIG_NO_THREADS
	ASSERT(self->t_threadname == NULL);
#endif /* !CONFIG_NO_THREADS */
#else /* DeeThread_USE_SINGLE_THREADED */
	/* If the thread is still bound, then we must unlink it! */
	if (LIST_ISBOUND(self, t_global)) {
		thread_list_lock_acquire_noint();
		if (LIST_ISBOUND(self, t_global))
			LIST_UNBIND(self, t_global);
		thread_list_lock_release();
	}

	/* If the thread's OS-handle still hasn't been joined/detached, do so now! */
	if ((self->t_state & (Dee_THREAD_STATE_HASTHREAD | Dee_THREAD_STATE_UNMANAGED)) ==
	    /*            */ (Dee_THREAD_STATE_HASTHREAD)) {
#ifdef DeeThread_Detach_system_impl
		DeeThread_Detach_system_impl(self);
#elif defined(DeeThread_Join_system_impl)
		DeeThread_Join_system_impl(self);
#else /* ... */
		(void)self;
#endif /* !... */
	}

	ASSERT(!self->t_deepassoc.da_used);
	ASSERT((self->t_deepassoc.da_mask != 0) ==
	       (self->t_deepassoc.da_list != empty_deep_assoc));
	if (self->t_deepassoc.da_list != empty_deep_assoc)
		Dee_Free(self->t_deepassoc.da_list);
	Dee_XDecref(self->t_threadname);
	if (self->t_state & Dee_THREAD_STATE_STARTED) {
		if (self->t_state & Dee_THREAD_STATE_TERMINATED) {
			if (self->t_exceptsz) {
				while (self->t_except) {
					struct except_frame *frame;
					frame          = self->t_except;
					self->t_except = frame->ef_prev;
					Dee_Decref(frame->ef_error);
					if (ITER_ISOK(frame->ef_trace))
						Dee_Decref(frame->ef_trace);
					except_frame_free(frame);
				}
			} else {
				Dee_Decref(self->t_inout.io_result);
			}
		}
	} else {
		Dee_XDecref(self->t_inout.io_main);
		Dee_Decref(self->t_context.d_args);
	}

	/* Clean out any remaining interrupts */
	if (self->t_interrupt.ti_intr != NULL &&
	    !(self->t_state & Dee_THREAD_STATE_TERMINATING)) {
		DeeThread_DiscardAllInterrupts(self);
	}
#endif /* !DeeThread_USE_SINGLE_THREADED */
}


#ifdef Dee_pid_t
/* Construct a new wrapper for an external reference to `pid'
 * NOTE: The given `pid' is _NOT_ inherited! */
PUBLIC WUNUSED DREF DeeObject *DCALL DeeThread_FromTid(Dee_pid_t tid) {
	DREF DeeOSThreadObject *result;
	result = DeeGCObject_CALLOC(DeeOSThreadObject);
	if unlikely(!result)
		goto err;
	result->ot_thread.t_state = Dee_THREAD_STATE_STARTED |
	                            Dee_THREAD_STATE_HASTID |
	                            Dee_THREAD_STATE_UNMANAGED;
	result->ot_tid = tid;
	result->ot_thread.t_deepassoc.da_list = empty_deep_assoc;
	DeeObject_Init(&result->ot_thread, &DeeThread_Type);
	return DeeGC_Track((DeeObject *)&result->ot_thread);
err:
	return NULL;
}
#endif /* Dee_pid_t */

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
thread_print_impl(DeeThreadObject *__restrict self,
                  dformatprinter printer, void *arg,
                  bool do_repr) {
	dssize_t temp, result = 0;
#ifndef DeeThread_USE_SINGLE_THREADED
	DREF DeeObject *thread_main;
	DREF DeeTupleObject *thread_args;
	DREF DeeObject *thread_result;
#endif /* !DeeThread_USE_SINGLE_THREADED */
	DREF DeeObject *thread_except;
	uint32_t state;
#ifdef Dee_pid_t
	Dee_pid_t tid;
#endif /* Dee_pid_t */
#ifndef DeeThread_USE_SINGLE_THREADED
	thread_main   = NULL;
	thread_args   = NULL;
	thread_result = NULL;
#endif /* !DeeThread_USE_SINGLE_THREADED */
	thread_except = NULL;

	/* Try to load the thread's PID */
#ifdef Dee_pid_t
	_DeeThread_AcquireDetaching(self);
	tid   = DeeThread_AsOSThread(self)->ot_tid;
	state = atomic_read(&self->t_state);
	_DeeThread_ReleaseDetaching(self);
	if (!(state & Dee_THREAD_STATE_HASTID))
		tid = 0;
#endif /* Dee_pid_t */

	/* Figure out the current state of the thread. */
	_DeeThread_AcquireSetup(self);
	state = atomic_read(&self->t_state);
#ifndef DeeThread_USE_SINGLE_THREADED
	if (!(state & Dee_THREAD_STATE_STARTED)) {
		thread_main = self->t_inout.io_main;
		thread_args = self->t_context.d_args;
		Dee_XIncref(thread_main);
		Dee_Incref(thread_args);
	} else
#endif /* !DeeThread_USE_SINGLE_THREADED */
	{
#ifndef DeeThread_USE_SINGLE_THREADED
		bool iscaller = self == DeeThread_Self();
		if ((state & Dee_THREAD_STATE_TERMINATED) || iscaller)
#else /* !DeeThread_USE_SINGLE_THREADED */
		if (!(state & Dee_THREAD_STATE_UNMANAGED))
#endif /* DeeThread_USE_SINGLE_THREADED */
		{
			if (self->t_exceptsz) {
				thread_except = self->t_except->ef_error;
				Dee_Incref(thread_except);
			}
#ifndef DeeThread_USE_SINGLE_THREADED
			else if (!iscaller) {
				thread_result = self->t_inout.io_result;
				Dee_Incref(thread_result);
			}
#endif /* !DeeThread_USE_SINGLE_THREADED */
		}
	}
	_DeeThread_ReleaseSetup(self);


	result = do_repr ? DeeFormat_PRINT(printer, arg, "Thread(")
	                 : DeeFormat_PRINT(printer, arg, "<Thread ");
	if unlikely(result < 0)
		goto done;

	/* Print TID information (if available) */
#ifdef Dee_pid_t
	_DeeThread_AcquireDetaching(self);
	tid   = DeeThread_AsOSThread(self)->ot_tid;
	state = atomic_read(&self->t_state);
	_DeeThread_ReleaseDetaching(self);
	if (!(state & Dee_THREAD_STATE_HASTID))
		tid = 0;
	if (do_repr) {
		if (state & Dee_THREAD_STATE_UNMANAGED) {
			DO(err, DeeFormat_Printf(printer, arg, "tid: %" PRFd64 ")", (int64_t)tid));
			goto done;
		}
	} else if (tid != 0) {
		DO(err, DeeFormat_Printf(printer, arg, " %" PRFd64, (int64_t)tid));
	}
#endif /* Dee_pid_t */

#ifndef DeeThread_USE_SINGLE_THREADED
	if (self->t_threadname) {
		DO(err, do_repr ? DeeFormat_Printf(printer, arg, "name: %r, ", self->t_threadname)
		                : DeeFormat_Printf(printer, arg, " %k", self->t_threadname));
	} else
#endif /* !DeeThread_USE_SINGLE_THREADED */
	if (!do_repr) {
#ifndef DeeThread_USE_SINGLE_THREADED
		if (thread_main && DeeFunction_Check(thread_main)) {
			/* If DDI provides, set the name of the function that's to-be executed. */
			DeeFunctionObject *exec_function;
			DeeCodeObject *exec_code;
			char *exec_name;
			exec_function = (DeeFunctionObject *)thread_main;
			exec_code     = exec_function->fo_code;
			exec_name     = DeeCode_NAME(exec_code);
			if (exec_name == NULL && exec_code == exec_code->co_module->mo_root)
				exec_name = DeeString_STR(exec_code->co_module->mo_name);
			DO(err, DeeFormat_Printf(printer, arg, " %s", exec_name ? exec_name : "<anonymous>"));
		}
#endif /* !DeeThread_USE_SINGLE_THREADED */
	}
	if (!do_repr) {
		char const *state_desc = NULL;
#ifndef DeeThread_USE_SINGLE_THREADED
		if (state & Dee_THREAD_STATE_TERMINATED) {
			state_desc = thread_except ? "crashed" : "terminated";
		} else
#endif /* !DeeThread_USE_SINGLE_THREADED */
		if (state & Dee_THREAD_STATE_TERMINATING) {
			state_desc = "terminating";
		} else
#ifdef DeeThread_USE_SINGLE_THREADED
		{
			state_desc = "running";
		}
#else /* DeeThread_USE_SINGLE_THREADED */
		if (state & Dee_THREAD_STATE_STARTED) {
			state_desc = "running";
		} else if (state & Dee_THREAD_STATE_STARTING) {
			state_desc = "starting";
		}
#endif /* !DeeThread_USE_SINGLE_THREADED */
		if (state_desc)
			DO(err, DeeFormat_Printf(printer, arg, " <%s>", state_desc));
		DO(err, DeeFormat_PRINT(printer, arg, ">"));
#ifndef DeeThread_USE_SINGLE_THREADED
	} else if ((state & Dee_THREAD_STATE_TERMINATED) && thread_except) {
		DO(err, DeeFormat_Printf(printer, arg, ")<.hascrashed:%r>", thread_except));
	} else if ((state & Dee_THREAD_STATE_TERMINATED) && thread_result) {
		DO(err, DeeFormat_Printf(printer, arg, ")<.hasterminated:%r>", thread_result));
	} else if (!(state & Dee_THREAD_STATE_STARTED)) {
		if (thread_main)
			DO(err, DeeFormat_Printf(printer, arg, "main: %r, ", self->t_threadname));
		DO(err, DeeFormat_Printf(printer, arg, "args: %r)", thread_args));
#endif /* !DeeThread_USE_SINGLE_THREADED */
	} else {
		DO(err, DeeFormat_PRINT(printer, arg, ")<.hasstarted>"));
	}
done:
#ifndef DeeThread_USE_SINGLE_THREADED
	Dee_XDecref(thread_main);
	Dee_XDecref(thread_args);
	Dee_XDecref(thread_result);
#endif /* !DeeThread_USE_SINGLE_THREADED */
	Dee_XDecref(thread_except);
	return result;
err:
	result = temp;
	goto done;
}


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
thread_print(DeeThreadObject *__restrict self,
             dformatprinter printer, void *arg) {
	return thread_print_impl(self, printer, arg, false);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
thread_printrepr(DeeThreadObject *__restrict self,
                 dformatprinter printer, void *arg) {
	return thread_print_impl(self, printer, arg, true);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
thread_init(DeeThreadObject *__restrict self,
            size_t argc, DeeObject *const *argv) {
#ifdef Dee_pid_t
	if (argc == 1 && DeeInt_Check(argv[0])) {
		/* Construct unmanaged thread from TID */
		DeeOSThreadObject *me = DeeThread_AsOSThread(self);
		me->ot_thread.t_str_curr               = NULL;
		me->ot_thread.t_repr_curr              = NULL;
		me->ot_thread.t_hash_curr              = NULL;
		me->ot_thread.t_deepassoc.da_used      = 0;
		me->ot_thread.t_deepassoc.da_mask      = 0;
		me->ot_thread.t_deepassoc.da_list      = empty_deep_assoc;
		me->ot_thread.t_deepassoc.da_recursion = 0;
		me->ot_thread.t_exec                   = NULL;
		me->ot_thread.t_except                 = NULL;
		me->ot_thread.t_execsz                 = 0;
		me->ot_thread.t_exceptsz               = 0;
		me->ot_thread.t_state                  = Dee_THREAD_STATE_STARTED | Dee_THREAD_STATE_HASTID | Dee_THREAD_STATE_UNMANAGED;
		me->ot_thread.t_interrupt.ti_next      = NULL;
		me->ot_thread.t_interrupt.ti_intr      = NULL;
		me->ot_thread.t_interrupt.ti_args      = NULL;
#ifndef CONFIG_NO_THREADS
		me->ot_thread.t_int_vers       = 0;
		me->ot_thread.t_global.le_prev = NULL;
		DBG_memset(&me->ot_thread.t_global.le_next, 0xcc, sizeof(self->t_global.le_next));
		me->ot_thread.t_threadname = NULL;
		DBG_memset(&me->ot_thread.t_inout, 0xcc, sizeof(me->ot_thread.t_inout));
#endif /* !CONFIG_NO_THREADS */
		me->ot_thread.t_context.d_tls = NULL;
		return DeeInt_AsIntX(argv[0], &me->ot_tid);
	}
#endif /* Dee_pid_t */

#ifdef DeeThread_USE_SINGLE_THREADED
	(void)self;
#ifdef Dee_pid_t
	if (argc == 1 && DeeObject_AssertTypeExact(argv[0], &DeeInt_Type))
		return -1;
	return err_invalid_argc(DeeString_STR(&str_Thread), argc, 1, 1);
#else /* Dee_pid_t */
	return err_unimplemented_constructor(&DeeThread_Type, argc, argv);
#endif /* !Dee_pid_t */
#else /* DeeThread_USE_SINGLE_THREADED */
	if unlikely(argc > 3) {
		err_invalid_argc(DeeString_STR(&str_Thread), argc, 0, 3);
		goto err;
	}
	self->t_threadname = NULL;
	if (argc && DeeString_Check(argv[0])) {
		self->t_threadname = (DREF DeeStringObject *)argv[0];
		Dee_Incref(self->t_threadname);
		--argc;
		++argv;
	}
	self->t_inout.io_main = NULL;
	if (argc) {
		/* The thread callback object. */
		self->t_inout.io_main = argv[0];
		Dee_Incref(self->t_inout.io_main);
		--argc;
		++argv;
	}
	if (argc) {
		self->t_context.d_args = (DREF DeeTupleObject *)argv[0];
		/* Allow `none' as an alias for an empty tuple. */
		if (DeeNone_Check(self->t_context.d_args)) {
			self->t_context.d_args = (DREF DeeTupleObject *)Dee_EmptyTuple;
		} else {
			/* Make sure that the callback arguments are a tuple. */
			if (DeeObject_AssertTypeExact(self->t_context.d_args,
			                              &DeeTuple_Type))
				goto err_main;
		}
	} else {
		/* When no arguments have been given, use an empty tuple. */
		self->t_context.d_args = (DREF DeeTupleObject *)Dee_EmptyTuple;
	}
	Dee_Incref(self->t_context.d_args);
	self->t_exec              = NULL;
	self->t_except            = NULL;
	self->t_exceptsz          = 0;
	self->t_execsz            = 0;
	self->t_str_curr          = NULL;
	self->t_repr_curr         = NULL;
	self->t_hash_curr         = NULL;
	self->t_state             = Dee_THREAD_STATE_INITIAL;
	self->t_int_vers          = 0;
	self->t_interrupt.ti_next = NULL;
	self->t_interrupt.ti_intr = NULL;
	self->t_interrupt.ti_args = NULL;
	self->t_global.le_prev    = NULL;
	DBG_memset(&self->t_global.le_next, 0xcc, sizeof(self->t_global.le_next));
	self->t_deepassoc.da_used      = 0;
	self->t_deepassoc.da_mask      = 0;
	self->t_deepassoc.da_list      = empty_deep_assoc;
	self->t_deepassoc.da_recursion = 0;
	return 0;
err_main:
	Dee_XDecref(self->t_inout.io_main);
	Dee_XDecref(self->t_threadname);
err:
	return -1;
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_start(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	if (DeeArg_Unpack(argc, argv, ":start"))
		goto err;
	error = DeeThread_Start(self);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_detach(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	if (DeeArg_Unpack(argc, argv, ":detach"))
		goto err;
	error = DeeThread_Detach(self);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_join(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":join"))
		goto err;
	result = DeeThread_Join(self, (uint64_t)-1);
	ASSERT(result != ITER_DONE);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_tryjoin(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":tryjoin"))
		goto err;
	result = DeeThread_Join(self, 0);
	if unlikely(result == NULL)
		goto err;
	if (result != ITER_DONE)
		return DeeTuple_Newf("bO", 1, result);
	return DeeTuple_Newf("bo", 0, Dee_None);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_timedjoin(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *result;
	uint64_t timeout_in_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedjoin", &timeout_in_nanoseconds))
		goto err;
	result = DeeThread_Join(self, timeout_in_nanoseconds);
	if unlikely(result == NULL)
		goto err;
	if (result != ITER_DONE)
		return DeeTuple_Newf("bO", 1, result);
	return DeeTuple_Newf("bo", 0, Dee_None);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_waitfor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitfor"))
		goto err;
	if (DeeThread_WaitFor(self, (uint64_t)-1) < 0)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_timedwaitfor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_in_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitfor", &timeout_in_nanoseconds))
		goto err;
	error = DeeThread_WaitFor(self, timeout_in_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_interrupt_impl(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *sig  = &DeeError_Interrupt_instance;
	DeeObject *args = NULL;
	int error;
	if (DeeArg_Unpack(argc, argv, "|oo:interrupt", &sig, &args))
		goto err;
	if (args) {
		if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
			goto err;
	}
	error = DeeThread_Interrupt(self, sig, args);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_started(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":started"))
		goto err;
	(void)self;
	return_bool(DeeThread_HasStarted(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_detached(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":detached"))
		goto err;
	(void)self;
	return_bool(DeeThread_WasDetached(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_terminated(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":terminated"))
		goto err;
	(void)self;
#ifdef DeeThread_USE_SINGLE_THREADED
	return_false;
#else /* DeeThread_USE_SINGLE_THREADED */
	return_bool(DeeThread_HasTerminated(self));
#endif /* !DeeThread_USE_SINGLE_THREADED */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_interrupted(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":interrupted"))
		goto err;
	(void)self;
	return_bool(DeeThread_WasInterrupted(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crashed(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":crashed"))
		goto err;
	(void)self;
	return_bool(DeeThread_HasCrashed(self));
err:
	return NULL;
}

PRIVATE NONNULL((1)) int DCALL
err_not_terminated(DeeThreadObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Thread %k has not terminated yet",
	                       self);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crash_error(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	DeeThreadObject *caller = DeeThread_Self();
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":crash_error"))
		goto err;
	if (self != caller) {
#ifdef DeeThread_USE_SINGLE_THREADED
		err_not_terminated(self);
		goto err;
#else /* DeeThread_USE_SINGLE_THREADED */
		_DeeThread_AcquireSetup(self);
		if (!DeeThread_HasTerminated(self)) {
			_DeeThread_ReleaseSetup(self);
			err_not_terminated(self);
			goto err;
		}
#endif /* !DeeThread_USE_SINGLE_THREADED */
	}
	ASSERT((self->t_exceptsz != 0) == (self->t_except != NULL));
	if (self->t_exceptsz == 0) {
		/* No active exceptions. */
#ifndef DeeThread_USE_SINGLE_THREADED
		if (self != caller)
			_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
		err_no_active_exception();
		goto err;
	}
	result = self->t_except->ef_error;
	ASSERT_OBJECT(result);
	Dee_Incref(result);
#ifndef DeeThread_USE_SINGLE_THREADED
	if (self != caller)
		_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crash_traceback(DeeThreadObject *self, size_t argc, DeeObject *const *argv) {
	DeeThreadObject *caller = DeeThread_Self();
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":crash_traceback"))
		goto err;
	if (self != caller) {
#ifdef DeeThread_USE_SINGLE_THREADED
		err_not_terminated(self);
		goto err;
#else /* DeeThread_USE_SINGLE_THREADED */
		_DeeThread_AcquireSetup(self);
		if (!DeeThread_HasTerminated(self)) {
			_DeeThread_ReleaseSetup(self);
			err_not_terminated(self);
			goto err;
		}
#endif /* !DeeThread_USE_SINGLE_THREADED */
	}
	ASSERT((self->t_exceptsz != 0) == (self->t_except != NULL));
	if (self->t_exceptsz == 0) {
		/* No active exceptions. */
#ifndef DeeThread_USE_SINGLE_THREADED
		if (self != caller)
			_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
		err_no_active_exception();
		goto err;
	}
	result = (DeeObject *)self->t_except->ef_trace;
	if (result == ITER_DONE) {
		result = NULL;
		if (self == caller) {
			self->t_except->ef_trace = except_frame_gettb(self->t_except);
			result = (DeeObject *)self->t_except->ef_trace;
		}
	}
	if (result == NULL)
		result = Dee_None;
	ASSERT_OBJECT(result);
	Dee_Incref(result);
#ifndef DeeThread_USE_SINGLE_THREADED
	if (self != caller)
		_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
	return result;
err:
	return NULL;
}



PRIVATE struct type_method tpconst thread_methods[] = {
	TYPE_METHOD("start", &thread_start, /* Not METHOD_FNOREFESCAPE, because incref's "this" for child thread. */
	            "->?Dbool\n"
	            "#tSystemError{Failed to start @this thread for some reason}"
	            "#r{true The ?. is now running}"
	            "#r{false The ?. had already been started}"
	            "Starts @this thread"),
	TYPE_METHOD_F("detach", &thread_detach, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#r{true The ?. has been detached}"
	              "#r{false The ?. was already detached}"
	              "Detaches @this thread"),
	TYPE_METHOD_F("join", &thread_join, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#t{:Interrupt}"
	              "#tThreadCrash{The error(s) that caused @this thread to crash, encapsulated in a :ThreadCrash error}"
	              "#r{The return value of @this thread}"
	              "Joins @this thread and returns the return value of its main function"),
	TYPE_METHOD_F("tryjoin", &thread_tryjoin, METHOD_FNOREFESCAPE,
	              "->?T2?Dbool?O\n"
	              "#tThreadCrash{The error(s) that caused @this thread to crash, encapsulated in a :ThreadCrash error}"
	              "Same as ?#join, but don't check for interrupts and fail immediately"),
	TYPE_METHOD_F("timedjoin", &thread_timedjoin, METHOD_FNOREFESCAPE,
	              "(timeout_in_nanoseconds:?Dint)->?T2?Dbool?O\n"
	              "#tThreadCrash{The error(s) that caused @this thread to crash, encapsulated in a :ThreadCrash error}"
	              "Same as ?#join, but only attempt to join for a given @timeout_in_nanoseconds"),
	TYPE_METHOD_F("waitfor", &thread_waitfor, METHOD_FNOREFESCAPE,
	              "()\n"
	              "#t{:Interrupt}"
	              "Block until @this Thread ?#hasterminated"),
	TYPE_METHOD_F("timedwaitfor", &thread_timedwaitfor, METHOD_FNOREFESCAPE,
	              "(timeout_in_nanoseconds:?Dint)->?Dbool\n"
	              "#t{:Interrupt}"
	              "Same as ?#waitfor, but only attempt to wait at most @timeout_in_nanoseconds"),
	TYPE_METHOD_F("interrupt", &thread_interrupt_impl, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "(signal)->?Dbool\n"
	              "(async_func:?DCallable,async_args:?DTuple)->?Dbool\n"
	              "#r{true: The interrupt was delivered}"
	              "#r{false: The ?. has already terminated and can no longer process interrupts}"
	              "Throws the given @signal or an instance of :Interrupt within @this thread, "
	              "or schedule the given @async_func to be called asynchronously using @async_args\n"
	              "Calling the function with no arguments is identical to:\n"
	              "${"
	              /**/ "import Signal from deemon;\n"
	              /**/ "this.interrupt(Signal.Interrupt());"
	              "}\n"
	              "Calling the function with a single arguments is identical to:\n"
	              "${"
	              /**/ "this.interrupt([](signal) {\n"
	              /**/ "	throw signal;\n"
	              /**/ "}, (signal,));"
	              "}\n"
	              "Note that interrupts delivered by this function are processed at random points during "
	              /**/ "execution of the thread, with the only guaranty that is made being that they will always "
	              /**/ "be handled sooner or later, no matter what the associated thread may be doing, even if "
	              /**/ "what it is doing is executing an infinite loop ${for (;;) { }}\n"
	              "Though it should be noted that a thread that terminated due to an unhandled "
	              /**/ "exception may not get a chance to execute all remaining interrupts before stopping\n"
	              "Also note that remaining interrupts may still be executing once ?#terminated already "
	              /**/ "returns ?t, as indicative of the thread no longer being able to receive new interrupts. "
	              "However to truly ensure that all interrupts have been processed, you must ?#join @this thread\n"
	              "User-code may also check for interrupts explicitly by calling ?#check_interrupt"),

	/* Old, deprecated status-testing functions (replaced by getsets) */
	TYPE_METHOD_F("started", &thread_started, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Deprecated alias for ?#hasstarted"),
	TYPE_METHOD_F("detached", &thread_detached, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Deprecated alias for ?#wasdetached"),
	TYPE_METHOD_F("terminated", &thread_terminated, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Deprecated alias for ?#hasterminated"),
	TYPE_METHOD_F("interrupted", &thread_interrupted, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Deprecated alias for ?#wasinterrupted"),
	TYPE_METHOD_F("crashed", &thread_crashed, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Deprecated alias for ?#hascrashed"),

	/* Old, deprecated function names for backwards compatibility */
	TYPE_METHOD_F("try_join", &thread_tryjoin, METHOD_FNOREFESCAPE,
	              "->?T2?Dbool?O\n"
	              "Old, deprecated name for ?#tryjoin"),
	TYPE_METHOD_F("timed_join", &thread_timedjoin, METHOD_FNOREFESCAPE,
	              "(timeout_in_nanoseconds:?Dint)->?T2?Dbool?O\n"
	              "Old, deprecated name for ?#timedjoin"),
	TYPE_METHOD_F("crash_error", &thread_crash_error, METHOD_FNOREFESCAPE,
	              "->?X2?O?N\n"
	              "Deprecated function that does the same as ${this.crashinfo[0][0]}"),
	TYPE_METHOD_F("crash_traceback", &thread_crash_traceback, METHOD_FNOREFESCAPE,
	              "->?X2?DTraceback?N\n"
	              "Deprecated function that does the same as ${this.crashinfo[0][1]}"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_self(DeeObject *UNUSED(self),
            size_t argc, DeeObject *const *argv) {
	DeeThreadObject *result;
	if (DeeArg_Unpack(argc, argv, ":self"))
		goto err;
	result = DeeThread_Self();
	return_reference_((DeeObject *)result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_selfid(DeeObject *UNUSED(self),
              size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":selfid"))
		goto err;
#ifdef DeeThread_GetCurrentTid
	{
		Dee_pid_t pid;
		DBG_ALIGNMENT_DISABLE();
		pid = DeeThread_GetCurrentTid();
		DBG_ALIGNMENT_ENABLE();
		return DeeInt_NEWU(pid);
	}
#else /* DeeThread_GetCurrentTid */
	DeeError_Throwf(&DeeError_SystemError, "Cannot determine id of current thread");
#endif /* !DeeThread_GetCurrentTid */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_yield(DeeObject *UNUSED(self),
             size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":yield"))
		goto err;
	SCHED_YIELD();
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_check_interrupt(DeeObject *UNUSED(self),
                       size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":check_interrupt"))
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_sleep(DeeObject *UNUSED(self),
             size_t argc, DeeObject *const *argv) {
	uint64_t timeout_in_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":sleep", &timeout_in_nanoseconds))
		goto err;
	if (DeeThread_Sleep(timeout_in_nanoseconds / 1000))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_exit(DeeObject *UNUSED(self),
            size_t argc, DeeObject *const *argv) {
	DeeObject *result = Dee_None;
	DREF struct threadexit_object *error;
	if (DeeArg_Unpack(argc, argv, "|o:exit", &result))
		goto err;
	error = DeeObject_MALLOC(struct threadexit_object);
	if unlikely(!error)
		goto err;
	error->te_result = result;
	Dee_Incref(result);
	DeeObject_Init(error, &DeeError_ThreadExit);
	DeeError_Throw((DeeObject *)error);
	Dee_Decref_unlikely(error);
err:
	return NULL;
}

#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_current_get(DeeObject *__restrict UNUSED(self)) {
	DeeThreadObject *result;
	result = DeeThread_Self();
	return_reference_((DeeObject *)result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_threaded_get(DeeObject *__restrict UNUSED(self)) {
	return_bool(atomic_read(&DeeThread_Main.ot_thread.t_global.le_next) != NULL);
}

#define HAVE_thread_class_getsets
PRIVATE struct type_getset tpconst thread_class_getsets[] = {
	TYPE_GETTER("current", &thread_current_get,
	            "->?.\n"
	            "Returns a thread descriptor for the calling thread"),
	TYPE_GETTER("threaded", &thread_threaded_get,
	            "->?Dbool\n"
	            "True if there are at least 2 running threads"),
	/* TODO: enumerate -> {thread...} 
	 * >> Returns a proxy sequence for enumerating all
	 *    deemon-threads; s.a. `add_running_thread()' */
	TYPE_GETSET_END
};
#else /* !DeeThread_USE_SINGLE_THREADED */
#define thread_class_getsets NULL
#endif /* DeeThread_USE_SINGLE_THREADED */

PRIVATE struct type_member tpconst thread_class_members[] = {
	TYPE_MEMBER_CONST_DOC("main", &DeeThread_Main.ot_thread, "The main (initial) thread"),
#ifdef DeeThread_USE_SINGLE_THREADED
	TYPE_MEMBER_CONST_DOC("current", &DeeThread_Main.ot_thread, "Returns a thread descriptor for the calling thread"),
	TYPE_MEMBER_CONST_DOC("supported", Dee_False, "True if the implementation supports multiple threads"),
	TYPE_MEMBER_CONST_DOC("threaded", Dee_False, "True if there are at least 2 running threads"),
#else /* DeeThread_USE_SINGLE_THREADED */
	TYPE_MEMBER_CONST_DOC("supported", Dee_True, "True if the implementation supports multiple threads"),
#endif /* !DeeThread_USE_SINGLE_THREADED */
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst thread_class_methods[] = {
	TYPE_METHOD("self", &thread_self,
	            "->?.\n"
	            "Deprecated alias for ?#current"),
	TYPE_METHOD("selfid", &thread_selfid,
	            "->?Dint\n"
	            "#tSystemError{The system does not provide a way to query thread ids}"
	            "Deprecated alias for ${Thread.current.id}"),
	TYPE_METHOD("check_interrupt", &thread_check_interrupt,
	            "()\n"
	            "#t{:Interrupt}"
	            "Checks for interrupts in the calling thread"),
	/* TODO: Must make this one deprecated, and add a new one with a different name!
	 *       `yield' is a reserved identifer, and `import Thread from deemon; Thread.yield();'
	 *       causes a compiler warning! */
	TYPE_METHOD(STR_yield, &thread_yield,
	            "()\n"
	            "Willingly preempt execution to another thread or process"),
	TYPE_METHOD("sleep", &thread_sleep,
	            "(timeout_in_nanoseconds:?Dint)\n"
	            "#t{:Interrupt}"
	            "Suspending execution for a total of @timeout_in_nanoseconds"),
	TYPE_METHOD("exit", &thread_exit,
	            "(result=!N)\n"
	            "#tThreadExit{Always thrown to exit the current thread}"
	            "Throw a :ThreadExit error object in order to terminate execution "
	            /**/ "within the current thread. This function does not return normally"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_callback_get(DeeThreadObject *__restrict self) {
#ifndef DeeThread_USE_SINGLE_THREADED
	DREF DeeObject *result = NULL;
	_DeeThread_AcquireSetup(self);
	if likely(!DeeThread_HasStarted(self)) {
		result = self->t_inout.io_main;
		Dee_XIncref(result);
	}
	_DeeThread_ReleaseSetup(self);
	if unlikely(!result)
		goto err_unbound;
	return result;
err_unbound:
#endif /* !DeeThread_USE_SINGLE_THREADED */
	(void)self;
	err_unbound_attribute_string(&DeeThread_Type, "callback");
	return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_cannot_set_thread_subclass_callback(DeeThreadObject *__restrict self,
                                        char const *__restrict attr_name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot set the %s of %k, which is a subclass %k of Thread",
	                       attr_name, self, Dee_TYPE(self));
}

PRIVATE int DCALL
thread_callback_set(DeeThreadObject *__restrict self,
                    DeeObject *value) {
	(void)value;
	if (!DeeThread_CheckExact(self))
		return err_cannot_set_thread_subclass_callback(self, "callback");
#ifndef DeeThread_USE_SINGLE_THREADED
	_DeeThread_AcquireSetup(self);
	if unlikely(DeeThread_HasStarted(self)) {
		_DeeThread_ReleaseSetup(self);
		goto err_already_started;
	} else {
		DREF DeeObject *old_callback;
		old_callback = self->t_inout.io_main;
		self->t_inout.io_main = value;
		Dee_XIncref(value);
		_DeeThread_ReleaseSetup(self);
		Dee_XDecref(old_callback);
	}
	return 0;
err_already_started:
#endif /* !DeeThread_USE_SINGLE_THREADED */
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot set the callback of thread "
	                       "%k that has already been started",
	                       self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
thread_callback_del(DeeThreadObject *__restrict self) {
	return thread_callback_set(self, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
thread_callargs_get(DeeThreadObject *__restrict self) {
#ifndef DeeThread_USE_SINGLE_THREADED
	DREF DeeTupleObject *result = NULL;
	_DeeThread_AcquireSetup(self);
	if likely(!DeeThread_HasStarted(self)) {
		result = self->t_context.d_args;
		Dee_XIncref(result);
	}
	_DeeThread_ReleaseSetup(self);
	if unlikely(!result)
		goto err_unbound;
	return result;
err_unbound:
#endif /* !DeeThread_USE_SINGLE_THREADED */
	(void)self;
	err_unbound_attribute_string(&DeeThread_Type, "callargs");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
thread_callargs_set(DeeThreadObject *__restrict self,
                    DeeTupleObject *__restrict value) {
	if (!DeeThread_CheckExact(self))
		return err_cannot_set_thread_subclass_callback(self, "callargs");

	/* Allow `none' to be used in place to an empty tuple. */
	if (DeeNone_Check(value))
		value = (DeeTupleObject *)Dee_EmptyTuple;

	/* Make sure the new value is actually a tuple. */
	if (DeeObject_AssertTypeExact(value, &DeeTuple_Type))
		goto err;

#ifndef DeeThread_USE_SINGLE_THREADED
	_DeeThread_AcquireSetup(self);
	if unlikely(DeeThread_HasStarted(self)) {
		_DeeThread_ReleaseSetup(self);
		goto err_already_started;
	} else {
		DREF DeeTupleObject *old_callargs;
		old_callargs = self->t_context.d_args;
		self->t_context.d_args = value;
		Dee_XIncref(value);
		_DeeThread_ReleaseSetup(self);
		Dee_XDecref(old_callargs);
	}
	return 0;
err_already_started:
#endif /* !DeeThread_USE_SINGLE_THREADED */
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot set the callargs of thread "
	                "%k that has already been started",
	                self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
thread_callargs_del(DeeThreadObject *__restrict self) {
	return thread_callargs_set(self, (DeeTupleObject *)Dee_EmptyTuple);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_result_get(DeeThreadObject *__restrict self) {
#ifndef DeeThread_USE_SINGLE_THREADED
	DREF DeeObject *result;
	result = DeeThread_Join((DeeObject *)self, 0);
	if (result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
#endif /* !DeeThread_USE_SINGLE_THREADED */
	(void)self;
	err_unbound_attribute_string(&DeeThread_Type, "result");
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_id(DeeThreadObject *__restrict self) {
#ifdef Dee_pid_t
	Dee_pid_t result;
	result = DeeThread_GetTid((DeeObject *)self);
	return DeeInt_NEWU(result);
#else /* Dee_pid_t */
	(void)self;
	err_unbound_attribute_string(&DeeThread_Type, "id");
	return NULL;
#endif /* !Dee_pid_t */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
thread_crashinfo(DeeThreadObject *__restrict self) {
	uint16_t i, count;
	DeeThreadObject *caller = DeeThread_Self();
	struct except_frame *frame_iter;
	DREF DeeTupleObject *result;
again:
	if (self != caller) {
#ifdef DeeThread_USE_SINGLE_THREADED
		err_not_terminated(self);
		goto err;
#else /* DeeThread_USE_SINGLE_THREADED */
		_DeeThread_AcquireSetup(self);
		if (!DeeThread_HasTerminated(self)) {
			_DeeThread_ReleaseSetup(self);
			err_not_terminated(self);
			goto err;
		}
#endif /* !DeeThread_USE_SINGLE_THREADED */
	}
	count = self->t_exceptsz;
	if (count == 0) {
#ifndef DeeThread_USE_SINGLE_THREADED
		if (self != caller)
			_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
		return_reference_((DeeTupleObject *)Dee_EmptyTuple);
	}
	result = DeeTuple_TryNewUninitialized(count);
	if unlikely(!result) {
#ifndef DeeThread_USE_SINGLE_THREADED
		if (self != caller)
			_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
		if (Dee_CollectMemory(DeeTuple_SIZEOF(count)))
			goto again;
		goto err;
	}
	frame_iter = self->t_except;
	for (i = 0; i < count; ++i, frame_iter = frame_iter->ef_prev) {
		DREF DeeTupleObject *pair;
		DeeObject *error;
		DeeObject *trace;
		ASSERT(frame_iter != NULL);
		error = frame_iter->ef_error;
		trace = (DeeObject *)frame_iter->ef_trace;
		if (trace == ITER_DONE) {
			trace = NULL;
			if (self == caller) {
				frame_iter->ef_trace = except_frame_gettb(frame_iter);
				trace = (DeeObject *)frame_iter->ef_trace;
			}
		}
		if (trace == NULL)
			trace = Dee_None;
		pair = (DREF DeeTupleObject *)DeeTuple_TryPack(2, error, trace);
		if unlikely(!pair) {
			bool collect_ok;
#ifndef DeeThread_USE_SINGLE_THREADED
			if (self != caller)
				_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
			collect_ok = Dee_CollectMemory(DeeTuple_SIZEOF(2));
			Dee_Decrefv_likely(DeeTuple_ELEM(result), i);
			DeeTuple_FreeUninitialized(result);
			if (collect_ok)
				goto again;
			goto err;
		}
		DeeTuple_SET(result, i, pair); /* Inherit reference */
	}
	ASSERT(frame_iter == NULL);
#ifndef DeeThread_USE_SINGLE_THREADED
	if (self != caller)
		_DeeThread_ReleaseSetup(self);
#endif /* !DeeThread_USE_SINGLE_THREADED */
	return result;
err:
	return NULL;
}


#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_isrunning(DeeThreadObject *__restrict self) {
	uint32_t state = atomic_read(&self->t_state);
	return_bool((state & (Dee_THREAD_STATE_STARTED | Dee_THREAD_STATE_TERMINATED)) == Dee_THREAD_STATE_STARTED);
}
#endif /* !DeeThread_USE_SINGLE_THREADED */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_hascrashed(DeeThreadObject *__restrict self) {
#ifdef DeeThread_USE_SINGLE_THREADED
	return_bool(atomic_read(&self->t_exceptsz) > 0);
#else /* DeeThread_USE_SINGLE_THREADED */
	uint32_t state = atomic_read(&self->t_state);
	return_bool((state & Dee_THREAD_STATE_TERMINATED) &&
	            (atomic_read(&self->t_exceptsz) > 0));
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

#ifdef Dee_THREAD_STATE_HASOSCTX
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_wasdetached(DeeThreadObject *__restrict self) {
	uint32_t state = atomic_read(&self->t_state);
	return_bool(!(state & Dee_THREAD_STATE_HASOSCTX));
}
#endif /* Dee_THREAD_STATE_HASOSCTX */

#ifndef DeeThread_USE_SINGLE_THREADED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_isstarting(DeeThreadObject *__restrict self) {
	uint32_t state = atomic_read(&self->t_state);
	return_bool(state & (Dee_THREAD_STATE_STARTING | Dee_THREAD_STATE_STARTED));
}
#endif /* !DeeThread_USE_SINGLE_THREADED */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_isterminating(DeeThreadObject *__restrict self) {
#ifdef DeeThread_USE_SINGLE_THREADED
	uint32_t state = atomic_read(&self->t_state);
	return_bool(state & Dee_THREAD_STATE_TERMINATING);
#else /* DeeThread_USE_SINGLE_THREADED */
	uint32_t state = atomic_read(&self->t_state);
	return_bool(state & (Dee_THREAD_STATE_TERMINATING | Dee_THREAD_STATE_TERMINATED));
#endif /* !DeeThread_USE_SINGLE_THREADED */
}

#ifdef DeeThread_USE_CreateThread
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_get_osfhandle_np(DeeThreadObject *__restrict self) {
	HANDLE hThread = DeeThread_GetHThread(self);
	uint32_t state = atomic_read(&self->t_state);
	if likely(state & Dee_THREAD_STATE_HASTHREAD)
		return DeeInt_NewUIntptr((uintptr_t)hThread);
	err_unbound_attribute_string(&DeeThread_Type, Dee_fd_osfhandle_GETSET);
	return NULL;
}
#endif /* DeeThread_USE_CreateThread */


PRIVATE struct type_getset tpconst thread_getsets[] = {
	TYPE_GETSET_F("callback", &thread_callback_get, &thread_callback_del, &thread_callback_set, METHOD_FNOREFESCAPE,
	              "->?DCallable\n"
	              "#tAttributeError{Attempted to overwrite the callback of a sub-class of ?., rather than an exact instance. "
	              /*                 */ "To prevent the need of overwriting this attribute whenever a sub-class wishes to provide a $run "
	              /*                 */ "member function, write-access to this field is denied in sub-classes of ?. and only granted "
	              /*                 */ "to exact instances}"
	              "#tValueError{Attempted to delete or set the attribute when @this thread has already been started.}"
	              "The callback that will be executed when the thread is started\n"
	              "In the event that no callback has been set, or that the callback has been deleted, "
	              /**/ "the getter will attempt to return the instance attribute $run which can be "
	              /**/ "overwritten by sub-classes to provide an automatic and implicit thread-callback"),
	TYPE_GETSET_F("callargs", &thread_callargs_get, &thread_callargs_del, &thread_callargs_set, METHOD_FNOREFESCAPE,
	              "->?DTuple\n"
	              "#tAttributeError{Attempted to overwrite the callback arguments of a sub-class of ?., rather than an exact instance. "
	              /*                 */ "To prevent the need of overwriting this attribute whenever a sub-class wishes to provide a $run "
	              /*                 */ "member function, write-access to this field is denied in sub-classes of ?. and only granted "
	              /*                 */ "to exact instances}"
	              "#tValueError{Attempted to delete or set the attribute when @this thread has already been started}"
	              "The callback arguments that are passed to ?#callback when the thread is started\n"
	              "Deleting this member or setting ?N is the same as setting an empty tuple"),
	TYPE_GETTER_F("result", &thread_result_get, METHOD_FNOREFESCAPE,
	              "#tValueError{@this thread has not terminated yet}"
	              "Return the result value of @this thread once it has terminated\n"
	              "This is similar to what is returned by ?#join, but in the event that "
	              /**/ "the thread terminated because it crashed, ?N is returned rather "
	              /**/ "than all the errors that caused the thread to crash being encapsulated and propagated"),
	TYPE_GETTER_F("crashinfo", &thread_crashinfo, METHOD_FNOREFESCAPE,
	              "->?S?T2?O?DTraceback\n"
	              "#tValueEror{@this thread hasn't terminated yet}"
	              "Returns a sequence of 2-element tuples describing the errors that were "
	              /**/ "active when the thread crashed (s.a. ?#hascrashed), or an empty sequence when "
	              /**/ "the thread didn't crash\n"
	              "The first element of each tuple is the error that was ${throw}n, and the "
	              /**/ "second element is the accompanying traceback, or ?N when not known.\n"
	              "This function replaces the deprecated ?#crash_error and ?#crash_traceback "
	              /**/ "functions that did something similar prior to deemon 200\n"
	              "When iterated, elements of the returned sequence identify errors that "
	              /**/ "caused the crash from most to least recently thrown"),
	TYPE_GETTER_F("traceback", &DeeThread_Trace, METHOD_FNOREFESCAPE,
	              "->?DTraceback\n"
	              "Generate a traceback for the thread's current execution position"),
	TYPE_GETTER_F("id", &thread_id, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The thread hasn't been started yet}"
	              "#tSystemError{The system does not provide a way to query thread ids}"
	              "Returns an operating-system specific id of @this thread"),
#ifndef DeeThread_USE_SINGLE_THREADED
	TYPE_GETTER_F("isrunning", &thread_isrunning, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this thread is current running (i.e. ?#hasstarted, but hasn't ?#hasterminated)"),
	TYPE_GETTER_F("isstarting", &thread_isstarting, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this thread is currently being started, or ?#hasstarted\n"
	              "This is similar to ?#hasstarted, but becomes ?t a little bit earlier"),
#endif /* !DeeThread_USE_SINGLE_THREADED */
	TYPE_GETTER_F("hascrashed", &thread_hascrashed, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this thread has crashed, that "
	              "is having ?#hasterminated while errors were still active\n"
	              "When ?t, attempting to ?#join @this thread will cause all of the "
	              /**/ "errors to be rethrown in the calling thread as a :ThreadCrash error"),
	TYPE_GETTER_F("isterminating", &thread_isterminating, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this thread is currently being terminated, or ?#hasterminated\n"
	              "This is similar to ?#hasterminated, but becomes ?t a little bit earlier"),
#ifdef Dee_THREAD_STATE_HASOSCTX
	TYPE_GETTER_F("wasdetached", &thread_wasdetached, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this thread has been detached, joined, or hasn't been started, yet"),
#endif /* Dee_THREAD_STATE_HASOSCTX */
#ifdef DeeThread_USE_CreateThread
	TYPE_GETTER_F(Dee_fd_osfhandle_GETSET, &thread_get_osfhandle_np, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns the windows HANDLE for this thread"),
#endif /* DeeThread_USE_CreateThread */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst thread_members[] = {
	TYPE_MEMBER_BITFIELD_DOC("wasinterrupted", STRUCT_CONST, DeeThreadObject, t_state, Dee_THREAD_STATE_INTERRUPTED,
	                         "Returns ?t if interrupts are pending for @this thread"),
#ifndef Dee_THREAD_STATE_HASOSCTX
	TYPE_MEMBER_CONST_DOC("wasdetached", Dee_True,
	                      "Returns ?t if @this thread has been detached, joined, or hasn't been started, yet"),
#endif /* Dee_THREAD_STATE_HASOSCTX */
#ifdef DeeThread_USE_SINGLE_THREADED
	TYPE_MEMBER_CONST_DOC("name", Dee_None,
	                      "->?X2?Dstring?N\n"
	                      "The name of the thread, or ?N when none was assigned"),
	TYPE_MEMBER_CONST_DOC("hasstarted", Dee_True,
	                      "Returns ?t if @this thread has been started"),
	TYPE_MEMBER_CONST_DOC("isstarting", Dee_True,
	                      "Returns ?t if @this thread is currently being started, or ?#hasstarted\n"
	                      "This is similar to ?#hasstarted, but becomes ?t a little bit earlier"),
	TYPE_MEMBER_CONST_DOC("isrunning", Dee_True,
	                      "Returns ?t if @this thread is current running (i.e. ?#hasstarted, but hasn't ?#hasterminated)"),
	TYPE_MEMBER_CONST_DOC("hasterminated", Dee_False,
	                      "Returns ?t if @this thread has terminated"),
	TYPE_MEMBER_CONST_DOC("available", Dee_False,
	                      "Alias for ?#hasterminated, here for API-compatibility with ?Ethreading:Lock"),
#else /* DeeThread_USE_SINGLE_THREADED */
	TYPE_MEMBER_FIELD_DOC("name", STRUCT_OBJECT_OPT, offsetof(DeeThreadObject, t_threadname),
	                      "->?X2?Dstring?N\n"
	                      "The name of the thread, or ?N when none was assigned"),
	TYPE_MEMBER_BITFIELD_DOC("hasstarted", STRUCT_CONST, DeeThreadObject, t_state, Dee_THREAD_STATE_STARTED,
	                         "Returns ?t if @this thread has been started"),
	TYPE_MEMBER_BITFIELD_DOC("hasterminated", STRUCT_CONST, DeeThreadObject, t_state, Dee_THREAD_STATE_TERMINATED,
	                         "Returns ?t if @this thread has terminated"),
	TYPE_MEMBER_BITFIELD_DOC("available", STRUCT_CONST, DeeThreadObject, t_state, Dee_THREAD_STATE_TERMINATED,
	                         "Alias for ?#hasterminated, here for API-compatibility with ?Ethreading:Lock"),
#endif /* !DeeThread_USE_SINGLE_THREADED */
	TYPE_MEMBER_END
};


#ifdef HAVE_thread_clear
#define thread_gc_PTR &thread_gc
PRIVATE struct type_gc tpconst thread_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&thread_clear
};
#else /* HAVE_thread_clear */
#define thread_gc_PTR NULL
#endif /* !HAVE_thread_clear */

#ifdef CONFIG_NO_DOC
#define thread_doc NULL
#else /* CONFIG_NO_DOC */
PRIVATE char const thread_doc[] =
"The core object type for enabling parallel computation\n"
"\n"

#ifdef Dee_pid_t
"(tid:?Dint)\n"
"Construct an unmanaged thread descriptor for @tid\n"
#endif /* Dee_pid_t */
#ifndef DeeThread_USE_SINGLE_THREADED
"()\n"
"(name:?Dstring)\n"
"(main:?DCallable,args:?DTuple=!N)\n"
"(name:?Dstring,main:?DCallable,args:?DTuple=!N)\n"
"Construct a new thread that that has yet to be started.\n"
"When no @main callable has been provided, invoke a $run "
/**/ "member which must be implemented by a sub-class:\n"
"${"
/**/ "import Thread, Callable from deemon;\n"
/**/ "class MyWorker: Thread {\n"
/**/ "	private member m_jobs: {Callable...};\n"
/**/ "\\\n"
/**/ "	this(jobs: {Callable...})\n"
/**/ "		: m_jobs = jobs\n"
/**/ "	{}\n"
/**/ "\\\n"
/**/ "	@@Thread entry point\n"
/**/ "	function run() {\n"
/**/ "		for (local j: m_jobs)\n"
/**/ "			j();\n"
/**/ "		return 42;\n"
/**/ "	}\n"
/**/ "}"
"}"
#endif /* !DeeThread_USE_SINGLE_THREADED */
"";
#endif /* !CONFIG_NO_DOC */

PUBLIC DeeTypeObject DeeThread_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Thread),
	/* .tp_doc      = */ thread_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&thread_init,
				TYPE_FIXED_ALLOCATOR_GC(DeeOSThreadObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&thread_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&thread_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&thread_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&thread_visit,
	/* .tp_gc            = */ thread_gc_PTR,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &DeeObject_GenericCmpByAddr,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ thread_methods,
	/* .tp_getsets       = */ thread_getsets,
	/* .tp_members       = */ thread_members,
	/* .tp_class_methods = */ thread_class_methods,
	/* .tp_class_getsets = */ thread_class_getsets,
	/* .tp_class_members = */ thread_class_members,
};


#ifndef DeeThread_USE_SINGLE_THREADED
struct localheap {
	size_t   lh_size;   /* Remaining heap memory */
	size_t   lh_total;  /* Total allocated heap member. */
	uint8_t *lh_buffer; /* [0..lh_size] Remaining heap buffer. */
	uint8_t *lh_base;   /* [0..lh_total] Heap buffer. */
	size_t   lh_req;    /* Total size that was requested.
	                     * NOTE: If this is larger than `lh_total' after
	                     *       the heap was used, then it was too small. */
};

PRIVATE NONNULL((1)) bool DCALL
localheap_init(struct localheap *__restrict self,
               size_t heapsize) {
	self->lh_req    = 0;
	self->lh_size   = heapsize;
	self->lh_total  = heapsize;
	self->lh_buffer = (uint8_t *)Dee_TryMalloc(heapsize);
	self->lh_base   = self->lh_buffer;
	if unlikely(!self->lh_buffer) {
		self->lh_size = 0;
		return false;
	}
	return true;
}

PRIVATE void *DCALL
localheap_malloc(struct localheap *__restrict self,
                 size_t reqsize) {
	void *result = NULL;
	self->lh_req = reqsize;
	if (reqsize < self->lh_size) {
		result = self->lh_buffer;
		self->lh_buffer += reqsize;
		self->lh_size -= reqsize;
	}
	return result;
}


#if 0
/* if (!Dee_IncrefIfNotZero(x)) x = NULL;
 * Because the order of decref() + write when setting locals is
 * undefined and up to the interpreter, having suspended some thread
 * by random, it is possible that it contains local variables who's
 * reference counter is equal to ZERO(0).
 * Technically, the interpreter would also be allowed to write
 * random, illegal pointers to locals during execution of some
 * instruction so long as locals are once again valid when
 * execution continues, but we need to assume that never happens.
 *
 * >> DeeObject *old_object;
 * >> old_object = LOCALimm;
 * >> LOCALimm   = new_object;
 * >> Dee_Decref(old_object);
 * 
 * When the 4th line decrements the object, we may still see the
 * old object who's reference counter has already reached zero.
 */
#define INCREF_IF_NONZERO(x)  (Dee_IncrefIfNotZero(x) || ((x) = NULL, 0))
#define XINCREF_IF_NONZERO(x) (!(x) || INCREF_IF_NONZERO(x))
#else
#define INCREF_IF_NONZERO(x)   Dee_Incref(x)
#define XINCREF_IF_NONZERO(x)  Dee_XIncref(x)
#endif


/* Collect traceback information for the given thread.
 * @return: false: The thread's execution stack was inconsistent. - preempt a bit, then try again.
 * @return: true:  Fully captured the thread's stack. Check `heap' to see if memory was sufficient. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
thread_collect_traceback(DeeThreadObject *__restrict self,
                         struct code_frame *__restrict dst,
                         struct localheap *__restrict heap) {
	struct code_frame *iter = self->t_exec;
	uint16_t count          = self->t_execsz;
	size_t i;
	for (; count && iter && iter != CODE_FRAME_NOT_EXECUTING;
	     iter = iter->cf_prev, --count, ++dst) {
		DeeCodeObject *code = iter->cf_func->fo_code;
		ASSERT(!dst->cf_prev);
		ASSERT(!dst->cf_stack);
		ASSERT(!dst->cf_stacksz);
		ASSERT(!dst->cf_sp);
		dst->cf_func = iter->cf_func;
		Dee_Incref(dst->cf_func);
		dst->cf_argc  = iter->cf_argc;
		dst->cf_argv  = (DREF DeeObject **)localheap_malloc(heap, dst->cf_argc * sizeof(DREF DeeObject *));
		dst->cf_frame = (DREF DeeObject **)localheap_malloc(heap, code->co_localc * sizeof(DREF DeeObject *));
		dst->cf_ip    = iter->cf_ip;
		dst->cf_vargs = iter->cf_vargs;
		XINCREF_IF_NONZERO(iter->cf_vargs);
		dst->cf_this = iter->cf_this;
		if (!(code->co_flags & CODE_FTHISCALL))
			dst->cf_this = NULL;
		Dee_XIncref(dst->cf_this);
		dst->cf_result = iter->cf_result;
		if (ITER_ISOK(dst->cf_result))
			INCREF_IF_NONZERO(dst->cf_result);
		dst->cf_flags = code->co_flags;
		/* Copy arguments and locals */
		if (dst->cf_argv) {
			for (i = 0; i < dst->cf_argc; ++i) {
				((DREF DeeObject **)dst->cf_argv)[i] = iter->cf_argv[i];
				Dee_Incref(dst->cf_argv[i]);
			}
		}
		if (dst->cf_frame) {
			for (i = 0; i < code->co_localc; ++i) {
				dst->cf_frame[i] = iter->cf_frame[i];
				XINCREF_IF_NONZERO(dst->cf_frame[i]);
			}
		}
	}
	/* The thread is consistent when all frames were enumerated. */
	return count == 0 && iter == NULL;
}

/* NOTE: This function is called when the thread is still suspended
 *       in order to perform cleanup on any object that we attempted
 *       to acquire a reference to. */
PRIVATE void DCALL
clear_frames(size_t length, struct code_frame *__restrict vector) {
	size_t i;
	for (; length; --length, ++vector) {
		DeeCodeObject *code = vector->cf_func->fo_code;
		ASSERT(!vector->cf_prev);
		ASSERT(!vector->cf_stack);
		ASSERT(!vector->cf_stacksz);
		ASSERT(!vector->cf_sp);
		if (vector->cf_argv) {
			for (i = 0; i < vector->cf_argc; ++i)
				Dee_DecrefNokill(vector->cf_argv[i]);
		}
		if (vector->cf_frame) {
			for (i = 0; i < code->co_localc; ++i)
				Dee_XDecrefNokill(vector->cf_frame[i]);
		}
		Dee_XDecrefNokill(vector->cf_func);
		Dee_XDecrefNokill(vector->cf_vargs);
		Dee_XDecrefNokill(vector->cf_this);
		if (ITER_ISOK(vector->cf_result))
			Dee_DecrefNokill(vector->cf_result);
	}
}

/* Inplace-override dynamically allocated vector within
 * frames with some that are allocated on the real heap.
 * Return false if allocation failed. */
PRIVATE bool DCALL
copy_dynmem(size_t length, struct code_frame *__restrict vector) {
	for (; length; --length, ++vector) {
		/* Must replace the frame (locals) and argument. */
		DREF DeeObject **new_vector;
		DeeCodeObject *code = vector->cf_func->fo_code;
		ASSERT(!vector->cf_prev);
		ASSERT(!vector->cf_stack);
		ASSERT(!vector->cf_stacksz);
		if (!code->co_localc) {
			vector->cf_frame = NULL;
		} else {
			new_vector = (DREF DeeObject **)Dee_Mallocc(code->co_localc,
			                                            sizeof(DREF DeeObject *));
			if unlikely(!new_vector)
				goto err;
			vector->cf_frame = (DREF DeeObject **)memcpyc(new_vector, vector->cf_frame,
			                                              code->co_localc, sizeof(DREF DeeObject *));
		}
		if (!vector->cf_argc) {
			vector->cf_argv = NULL;
		} else {
			new_vector = (DREF DeeObject **)Dee_Mallocc(vector->cf_argc,
			                                            sizeof(DREF DeeObject *));
			if unlikely(!new_vector)
				goto err;
			vector->cf_argv = (DREF DeeObject **)memcpyc(new_vector, vector->cf_argv,
			                                             vector->cf_argc, sizeof(DREF DeeObject *));
		}
	}
	return true;
err:
	return false;
}
#endif /* !DeeThread_USE_SINGLE_THREADED */



/* Capture a snapshot of the given thread's execution stack, returning
 * a traceback object describing what is actually being run by it.
 * Note that this is just a snapshot that by no means will remain
 * consistent once this function returns.
 * NOTE: If the given thread is the caller's, this is identical `(Traceback from deemon)()' */
PUBLIC WUNUSED NONNULL((1)) DREF /*Traceback*/ DeeObject *DCALL
DeeThread_Trace(/*Thread*/ DeeObject *__restrict self) {
	DeeThreadObject *me = (DeeThreadObject *)self;
#ifdef DeeThread_USE_SINGLE_THREADED
	/* All threads other than the caller are unmanaged, and thus have empty tracebacks. */
	if (me != DeeThread_Self()) {
		return_reference_((DeeObject *)&DeeTraceback_Empty);
	} else
#else /* DeeThread_USE_SINGLE_THREADED */
	if (me != DeeThread_Self()) {
		if (!(atomic_read(&me->t_state) & Dee_THREAD_STATE_STARTED)) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot trace thread %k that hasn't been started",
			                self);
			goto err;
		}

		/* Must suspend and capture this thread. */
		for (;;) {
			int error;
			uint16_t traceback_size;
			uint16_t traceback_used;
			DeeTracebackObject *result;
			struct localheap heap;
			traceback_size = atomic_read(&me->t_execsz);
			result = (DeeTracebackObject *)DeeGCObject_Mallocc(offsetof(DeeTracebackObject, tb_frames),
			                                                   traceback_size, sizeof(struct code_frame));
			if unlikely(!result)
				goto err;

			/* Special (and simple) case: No frames to collect. */
			if (!traceback_size) {
				traceback_used = 0;
				goto done_traceback;
			}

			/* Initializer the local heap buffer used to trace local variables. */
			localheap_init(&heap, traceback_size * (32 * sizeof(void *)));

suspend_me:
			/* zero-initialize all the frames to make our job easier below. */
			bzeroc(result->tb_frames,
			       traceback_size,
			       sizeof(struct code_frame));

			/* This is where it gets dangerous: Suspend the thread and collect information! */
			COMPILER_BARRIER();
			error = DeeThread_Suspend(me);
			if unlikely(error != 0) {
				if (error < 0)
					goto err_free_result;

				/* Thread has already terminated */
				traceback_used = 0;
				goto done_traceback_with_heap;
			}
			COMPILER_BARRIER();

			traceback_used = me->t_execsz;
			if (traceback_used > traceback_size) {
				DeeTracebackObject *new_result;
				/* Our traceback was too small. - Reallocate to fit. */
				COMPILER_BARRIER();
				DeeThread_Resume(me);
				COMPILER_BARRIER();

				traceback_size = traceback_used;
				new_result = (DeeTracebackObject *)DeeGCObject_Reallocc(result, offsetof(DeeTracebackObject, tb_frames),
				                                                        traceback_size, sizeof(struct code_frame));
				if unlikely(!new_result)
					goto err_free_result;
				result = new_result;
				goto suspend_me;
			}

			if (!thread_collect_traceback(me, result->tb_frames, &heap)) {
				/* The thread is in an inconsistent state. - Resume it and preempt a bit. */
				clear_frames(traceback_used, result->tb_frames);
				COMPILER_BARRIER();
				DeeThread_Resume(me);
				COMPILER_BARRIER();
				/* Sleep a bit. */
				if (DeeThread_Sleep(100))
					goto err_free_result;
				goto suspend_me;
			}

			/* Either we've managed to capture a consistent traceback,
			 * or there wasn't enough heap memory to do so. */
			if unlikely(heap.lh_req > heap.lh_total) {
				uint8_t *new_heap;

				/* Clear out all references were copied. */
				clear_frames(traceback_used, result->tb_frames);
				COMPILER_BARRIER();
				DeeThread_Resume(me);
				COMPILER_BARRIER();

				/* Reallocate to fit the required size. */
				new_heap = (uint8_t *)Dee_Realloc(heap.lh_base, heap.lh_req);
				if unlikely(!new_heap)
					goto err_free_result;
				heap.lh_base = heap.lh_buffer = new_heap;
				heap.lh_size = heap.lh_total = heap.lh_req;
				heap.lh_req                  = 0;
				goto suspend_me;
			}
			COMPILER_BARRIER();
			DeeThread_Resume(me);
			COMPILER_BARRIER();

			/* Deallocate traceback frames that weren't used. */
done_traceback_with_heap:
			if (traceback_size != traceback_used) {
				DeeTracebackObject *new_result;
				new_result = (DeeTracebackObject *)DeeGCObject_TryReallocc(result, offsetof(DeeTracebackObject, tb_frames),
				                                                           traceback_used, sizeof(struct code_frame));
				if likely(new_result)
					result = new_result;
			}

			/* With everything collected, we must still convert pointers apart
			 * of our mini-heap to something that is apart of the real heap. */
			if (!copy_dynmem(traceback_used, result->tb_frames))
				goto err_free_result;

			/* With dynamic memory duplicated, free the temporary (local) heap. */
			Dee_Free(heap.lh_base);

			/* Initialize remaining members of the traceback. */
done_traceback:
			Dee_Incref(me); /* Reference stored in `tb_thread' */
			result->tb_thread = me;
			Dee_atomic_lock_init(&result->tb_lock);
			result->tb_numframes = traceback_used;
			DeeObject_Init(result, &DeeTraceback_Type);

			/* Tracebacks are GC objects, so we need to start tracking it here. */
			return DeeGC_Track((DeeObject *)result);
err_free_result:
			Dee_Free(heap.lh_base);
			DeeObject_Free(result);
		}
err:
		return NULL;
	} else
#endif /* !DeeThread_USE_SINGLE_THREADED */
	{
		return (DREF DeeObject *)DeeTraceback_NewWithException(me);
	}
}


/* Returns the traceback of a given exception-frame, or
 * `NULL' if no traceback exists for the exception. */
INTERN WUNUSED NONNULL((1)) struct traceback_object *DCALL
except_frame_gettb(struct except_frame *__restrict self) {
	if (self->ef_trace == (DREF DeeTracebackObject *)ITER_DONE)
		self->ef_trace = DeeTraceback_New(DeeThread_Self());
	return self->ef_trace;
}


DECL_END

#endif /* !GUARD_DEEMON_THREAD_H */
