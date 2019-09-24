/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_THREAD_C
#define GUARD_DEEMON_RUNTIME_THREAD_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* pthread_setname_np() */
#define _DEFAULT_SOURCE 1 /* `pthread_suspend()' */
//#define _POSIX_C_SOURCE 199309L /* sigaction() */
#define _POSIX_C_SOURCE 199506L /* pthread_kill() */
#define _XOPEN_SOURCE   500     /* pthread_kill() */


/* I have yet to find a pthread_suspend() implementation that
 * forces the system to acknowledge thread suspension synchronously,
 * this being a requirement for deemon that can still be fulfilled
 * by signals.
 * You may use the following code to determine if a given implementation does
 * actually implement synchronous thread suspension (which deemon requires).
 * WARNING: This test _MUST_ be performed on a machine with multiple cores
 * >> 
 * >> #include <pthread.h>
 * >> #include <stdio.h>
 * >> #include <stdlib.h>
 * >> 
 * >> #ifdef __cplusplus
 * >> extern "C" {
 * >> #endif
 * >> 
 * >> static volatile int test_atomic = 0;
 * >> 
 * >> static void *thread_main(void *arg) {
 * >>     int old;
 * >>     (void)arg;
 * >>     // Allow the thread to be canceled at any time.
 * >>     pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old);
 * >>     for (;;) {
 * >>         __atomic_fetch_add(&test_atomic,1,__ATOMIC_SEQ_CST);
 * >>     }
 * >> }
 * >> 
 * >> int main(int argc, char *argv[]) {
 * >>     unsigned int i,count,pass;
 * >>     
 * >>     for (count = 0; count < 1000; ++count) {
 * >>         pthread_t thread;
 * >>         pthread_create(&thread, NULL, &thread_main);
 * >>         for (pass = 0; pass < 50; ++pass) {
 * >>             int value, new_value;
 * >>             // Yield to the thread we're using for testing.
 * >>             for (i = 0; i < 10; ++i)
 * >>                 pthread_yield();
 * >>             pthread_suspend(&thread);
 * >>             // If `pthread_suspend()' is synchronous, then `test_atomic' will no longer change.
 * >>             // This is a requirement that deemon needs in order to take advantage
 * >>             // of pthread_suspend() without having to implement its own (synchronous)
 * >>             // suspension system using signals and `pthread_kill()'.
 * >>             value = __atomic_fetch_or(&test_atomic, 0, __ATOMIC_SEQ_CST);
 * >>             pthread_yield(); // Wait till the end of our quantum.
 * >>             new_value = __atomic_fetch_or(&test_atomic, 0, __ATOMIC_SEQ_CST);
 * >>             if (value != new_value) {
 * >>                 fprintf(stderr, "pthread_suspend() isn't synchronous\n");
 * >>                 exit(1);
 * >>             }
 * >>             pthread_resume(&thread);
 * >>         }
 * >>         pthread_cancel(thread);
 * >>     }
 * >>     fprintf(stderr, "Either your machine has only a single core, or pthread_suspend() is synchronous\n");
 * >>     return 0;
 * >> }
 * >> 
 * >> #ifdef __cplusplus
 * >> }
 * >> #endif
 */
#define CONFIG_NO_PTHREAD_SUSPEND 1
/*#define CONFIG_HAVE_PTHREAD_SUSPEND 1*/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/cache.h>
#include <deemon/util/string.h>

#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>
#include <hybrid/overflow.h>

#ifndef CONFIG_NO_THREADS
#include <string.h>

#ifndef CONFIG_NO_STDIO
#include <stdio.h>
#endif /* !CONFIG_NO_STDIO */

#ifndef CONFIG_NO_STDLIB
#include <stdlib.h>
#endif /* !CONFIG_NO_STDLIB */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_UNIX
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#ifndef CONFIG_HOST_WINDOWS
#if defined(__NO_has_include) || __has_include(<sys/syscall.h>)
#include <sys/syscall.h>
#endif /* __NO_has_include || __has_include(<sys/syscall.h>) */
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_HOST_UNIX */

#ifdef CONFIG_THREADS_PTHREAD
#include <pthread.h>
#include <unistd.h>
#if !defined(CONFIG_NO_PTHREAD_SUSPEND) && \
    (defined(__MISC_VISIBLE) || defined(CONFIG_HAVE_PTHREAD_SUSPEND))
#undef CONFIG_HAVE_PTHREAD_SUSPEND
#define CONFIG_HAVE_PTHREAD_SUSPEND 1
#else
#define CONFIG_NEED_SUSPEND_SIGNALS 1
#endif
#endif /* CONFIG_THREADS_PTHREAD */

#ifdef CONFIG_NEED_SUSPEND_SIGNALS
#ifndef CONFIG_NO_SYS_SIGNALS_H
#include <sys/signal.h>
#endif /* !CONFIG_NO_SYS_SIGNALS_H */
#ifndef CONFIG_NO_SIGNALS_H
#include <signal.h>
#endif /* !CONFIG_NO_SIGNALS_H */
#endif /* CONFIG_NEED_SUSPEND_SIGNALS */
#endif /* !CONFIG_NO_THREADS */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

DECL_BEGIN

#ifdef CONFIG_THREADS_WINDOWS
PRIVATE HANDLE DCALL os_getcurrenthread(void) {
	HANDLE hResult, hThread, hProcess;
	DBG_ALIGNMENT_DISABLE();
	hThread  = GetCurrentThread();
	hProcess = GetCurrentProcess();
	if (!DuplicateHandle(hProcess, hThread, hProcess, &hResult,
	                     0, TRUE, DUPLICATE_SAME_ACCESS))
		hResult = hThread;
	DBG_ALIGNMENT_ENABLE();
	return hResult;
}
//#define os_getcurrenthread() GetCurrentThread()
#else /* CONFIG_THREADS_WINDOWS */
#define os_getcurrenthread() pthread_self()
#endif /* !CONFIG_THREADS_WINDOWS */

#ifndef CONFIG_NO_THREADID
#ifdef CONFIG_HOST_WINDOWS
#define os_gettid() (dthreadid_t)GetCurrentThreadId()
#elif defined(SYS_gettid)
#define os_gettid() (dthreadid_t)syscall(SYS_gettid)
#elif defined(__NR_gettid)
#define os_gettid() (dthreadid_t)syscall(__NR_gettid)
#else
#warning "Threadid is not available. Try building with `-DCONFIG_NO_THREADID' to reduce overhead"
#define CONFIG_NO_THREADID_INTERNAL 1
#endif
#else /* !CONFIG_NO_THREADID */
#define CONFIG_NO_THREADID_INTERNAL 1
#endif /* CONFIG_NO_THREADID */

#ifndef os_gettid
#define os_gettid() 0
#endif /* !os_gettid */

#if defined(CONFIG_THREADS_PTHREAD) && defined(__pthread_gettid_np_defined)
#define os_gettid_with_thread(thread) pthread_gettid_np(thread)
#else /* CONFIG_THREADS_PTHREAD && __pthread_gettid_np_defined */
#define os_gettid_with_thread(thread) os_gettid()
#endif /* !CONFIG_THREADS_PTHREAD || !__pthread_gettid_np_defined */



#ifndef CONFIG_NO_THREADID
/* Library hooks for implementing thread-local storage. */
#define fini_tls_data(data)             (*_DeeThread_TlsCallbacks.tc_fini)(data)
#define visit_tls_data(data, proc, arg) (*_DeeThread_TlsCallbacks.tc_visit)(data, proc, arg)
PRIVATE void DCALL stub_tc_fini(void *__restrict UNUSED(data)) {}

PRIVATE void DCALL stub_tc_visit(void *__restrict UNUSED(data), dvisit_t UNUSED(proc), void *UNUSED(arg)) {}

PUBLIC struct tls_callback_hooks _DeeThread_TlsCallbacks = {
	/* .tc_fini  = */ &stub_tc_fini,
	/* .tc_visit = */ &stub_tc_visit,
};
#endif /* !CONFIG_NO_THREADID */


STATIC_ASSERT(SIZEOF_DTHREADID_T == sizeof(dthreadid_t));
PRIVATE DEFINE_STRING(main_thread_name,"MainThread");
PUBLIC uint16_t DeeExec_StackLimit = DEE_CONFIG_DEFAULT_STACK_LIMIT;


PRIVATE struct deep_assoc_entry empty_deep_assoc[] = {
	{ NULL, NULL }
};


#if defined(CONFIG_HOST_WINDOWS) && defined(_MSC_VER)
#ifdef _PREFAST_
#pragma warning(push)
#pragma warning(disable: 6320)
#pragma warning(disable: 6322)
#endif /* _PREFAST_ */
#pragma pack(push,8)
PRIVATE NONNULL((1)) void DCALL
sys_setthreadname(char const *__restrict name) {
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
#elif !defined(CONFIG_NO_PTHREAD_SETNAME_NP) && \
      (defined(__USE_GNU) || defined(CONFIG_HAVE_PTHREAD_SETNAME_NP))
#define sys_setthreadname(name) \
	pthread_setname_np(pthread_self(), name)
#else
#define CONFIG_NO_SETTHREADNAME 1
#endif


PRIVATE NONNULL((1)) bool DCALL
deepassoc_rehash(DeeThreadObject *__restrict self) {
	struct deep_assoc_entry *new_vector, *iter, *end;
	size_t new_mask = self->t_deepassoc.da_mask;
	new_mask        = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 64 - 1; /* Start out bigger than 2. */
	ASSERT(self->t_deepassoc.da_used < new_mask);
	new_vector = (struct deep_assoc_entry *)Dee_TryCalloc((new_mask + 1) * sizeof(struct deep_assoc_entry));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->t_deepassoc.da_list == empty_deep_assoc) == (self->t_deepassoc.da_used == 0));
	ASSERT((self->t_deepassoc.da_list == empty_deep_assoc) == (self->t_deepassoc.da_mask == 0));
	if (self->t_deepassoc.da_list != empty_deep_assoc) {
		/* Re-insert all existing items into the new table vector. */
		end = (iter = self->t_deepassoc.da_list) + (self->t_deepassoc.da_mask + 1);
		for (; iter != end; ++iter) {
			struct deep_assoc_entry *item;
			dhash_t i, perturb;
			/* Skip NULL entires. */
			if (!iter->de_old)
				continue;
			perturb = i = (Dee_HashPointer(iter->de_old) ^
			               Dee_HashPointer(Dee_TYPE(iter->de_new))) &
			              new_mask;
			for (;; i = DEEPASSOC_HASHNX(i, perturb), DEEPASSOC_HASHPT(perturb)) {
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
	for (;; i = DEEPASSOC_HASHNX(i, perturb), DEEPASSOC_HASHPT(perturb)) {
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

INTERN WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
deepcopy_lookup(DeeThreadObject *thread_self, DeeObject *old_object,
                DeeTypeObject *new_type) {
	uintptr_t i, perturb;
	uintptr_t hash = (Dee_HashPointer(old_object) ^
	                  Dee_HashPointer(new_type));
	perturb = i = DEEPASSOC_HASHST(&thread_self->t_deepassoc, hash);
	for (;; i = DEEPASSOC_HASHNX(i, perturb), DEEPASSOC_HASHPT(perturb)) {
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
	begin                            = thread_self->t_deepassoc.da_list;
	mask                             = thread_self->t_deepassoc.da_mask;
	thread_self->t_deepassoc.da_list = empty_deep_assoc;
	thread_self->t_deepassoc.da_mask = 0;
	thread_self->t_deepassoc.da_used = 0;
	end                              = (iter = begin) + (mask + 1);
	/* Go through and clear out all the generated mappings. */
	for (; iter != end; ++iter) {
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



/* Controller for the initial/main thread object. */
#ifndef CONFIG_NO_THREADS
INTERN
#else /* !CONFIG_NO_THREADS */
PUBLIC
#endif /* CONFIG_NO_THREADS */
DeeThreadObject DeeThread_Main = {
	OBJECT_HEAD_INIT(&DeeThread_Type),
	/* .t_exec       = */ NULL,
	/* .t_except     = */ NULL,
	/* .t_exceptsz   = */ 0,
	/* .t_execsz     = */ 0,
#if __SIZEOF_POINTER__ > 4
	/* .t_padding    = */ { 0 },
#endif /* __SIZEOF_POINTER__ > 4 */
	/* .t_str_curr   = */ NULL,
	/* .t_repr_curr  = */ NULL,
	/* .t_deepassoc  = */ {
		/* .da_used = */ 0,
		/* .da_mask = */ 0,
		/* .da_list = */empty_deep_assoc
	}
#ifndef CONFIG_NO_THREADS
	,
	/* .t_globlpself = */ NULL,
	/* .t_globalnext = */ NULL,
	/* .t_threadname = */ (DeeStringObject *)&main_thread_name,
	/* .t_state      = */THREAD_STATE_STARTED,
	/* .t_padding2   = */ 0,
#ifndef CONFIG_NO_THREADID
	/* .t_threadid   = */ 0,
#endif /* !CONFIG_NO_THREADID */
	/* .t_thread     = */ 0,
	/* .t_suspended  = */ 0,
	/* .t_interrupt  = */ {
		/* .ti_next = */ NULL,
		/* .ti_intr = */ NULL,
		/* .ti_args = */ NULL
	},
	/* .t_threadmain = */ NULL,
	/* .t_threadargs = */ (DeeTupleObject *)Dee_EmptyTuple,
	/* .t_threadres  = */ NULL,
	/* .t_tlsdata    = */ NULL
#endif /* !CONFIG_NO_THREADS */
};


#ifdef CONFIG_HOST_WINDOWS
PRIVATE uint64_t performance_freq = 0;
PRIVATE uint64_t performance_freq_div_1000000 = 0;
PRIVATE uint64_t performance_1000000_div_freq = 0;
PRIVATE WCHAR const kernel32[] = {'K','E','R','N','E','L','3','2',0};

PRIVATE ULONGLONG (WINAPI *lp_GetTickCount64)(void) = NULL;
#endif /* CONFIG_HOST_WINDOWS */

PUBLIC WUNUSED uint64_t (DCALL DeeThread_GetTimeMicroSeconds)(void) {
#ifdef CONFIG_HOST_WINDOWS
	uint64_t result;
	if (!performance_freq) {
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
#if 0
#define FILETIME_PER_SECONDS 10000000   /* 100 nanoseconds / 0.1 microseconds. */
#define MICROSECONDS_PER_SECOND 1000000 /* 0.001 milliseconds / 0.000001 seconds. */
	if (performance_freq < FILETIME_PER_SECONDS) {
		/* The performance counter is slower than `GetSystemTimePreciseAsFileTime()',
		 * so we can just use the later to get more precision. */
		DBG_ALIGNMENT_DISABLE();
		GetSystemTimePreciseAsFileTime((LPFILETIME)&result);
		DBG_ALIGNMENT_ENABLE();
		result /= (FILETIME_PER_SECONDS / MICROSECONDS_PER_SECOND);
		return result;
	}
#endif
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!QueryPerformanceCounter((LARGE_INTEGER *)&result)) {
		uint64_t result;
do_tickcount:
		if (!lp_GetTickCount64) {
			*(void **)&lp_GetTickCount64 = GetProcAddress(GetModuleHandleW(kernel32), "GetTickCount64");
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
#else /* CONFIG_HOST_WINDOWS */
	struct timespec now;
	DBG_ALIGNMENT_DISABLE();
	if unlikely(clock_gettime(CLOCK_MONOTONIC, &now)) {
		now.tv_sec  = time(NULL);
		now.tv_nsec = 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return ((uint64_t)now.tv_sec * 1000000) + (now.tv_nsec / 1000);
#endif /* !CONFIG_HOST_WINDOWS */
}

#ifndef CONFIG_NO_THREADS
#if defined(CONFIG_THREADS_PTHREAD) || \
    defined(CONFIG_NEED_SUSPEND_SIGNALS)
#ifndef PTHREAD_INTERRUPT_SIGNAL
#define PTHREAD_INTERRUPT_SIGNAL SIGUSR1
#endif
#define sys_threadstartup  sys_threadstartup
PRIVATE void suspend_signal_handler(int signo);

PRIVATE NONNULL((1)) void DCALL sys_threadstartup(DeeThreadObject *__restrict self) {
#ifdef CONFIG_NEED_SUSPEND_SIGNALS
#ifndef SA_NODEFER
#error "The suspension signal handler needs to be able to recurse"
#endif
	struct sigaction action;
	action.sa_handler = &suspend_signal_handler;
	action.sa_flags   = SA_NODEFER;
	/* Install the custom signal handler. */
	DBG_ALIGNMENT_DISABLE();
	if (sigaction(PTHREAD_INTERRUPT_SIGNAL, &action, NULL))
		BREAKPOINT();
	DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_NEED_SUSPEND_SIGNALS */
	/* Even when we don't need it for suspension
	 * signals, we still need it for interrupts. */
	DBG_ALIGNMENT_DISABLE();
	signal(PTHREAD_INTERRUPT_SIGNAL, &suspend_signal_handler);
	DBG_ALIGNMENT_ENABLE();
#endif /* !CONFIG_NEED_SUSPEND_SIGNALS */
	   /*printf("thread_startup\n");*/
}

#ifdef CONFIG_NEED_SUSPEND_SIGNALS
#define sys_threadshutdown sys_threadshutdown
PRIVATE NONNULL((1)) void DCALL sys_threadshutdown(DeeThreadObject *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	signal(PTHREAD_INTERRUPT_SIGNAL, SIG_IGN);
	DBG_ALIGNMENT_ENABLE();
	/*printf("thread_shutdown\n");*/
}
#endif /* CONFIG_NEED_SUSPEND_SIGNALS */
#endif /* CONFIG_THREADS_PTHREAD || CONFIG_NEED_SUSPEND_SIGNALS */

/* Define thread-startup as a no-op when it wasn't defined before. */
#ifndef sys_threadstartup
#define CONFIG_NO_SYS_THREADSTARTUP 1
#define sys_threadstartup(self)  (void)0
#endif /* !sys_threadstartup */
#ifndef sys_threadshutdown
#define CONFIG_NO_SYS_THREADSHUTDOWN 1
#define sys_threadshutdown(self)  (void)0
#endif /* !sys_threadshutdown */


PRIVATE NONNULL((1)) void DCALL
do_suspend_thread(DeeThreadObject *__restrict self) {
	if (ATOMIC_FETCHINC(self->t_suspended) == 0) {
		if (self->t_state & THREAD_STATE_STARTED) {
#ifdef CONFIG_THREADS_WINDOWS
			CONTEXT ignored_context;
			DBG_ALIGNMENT_DISABLE();
			SuspendThread(self->t_thread);
			DBG_ALIGNMENT_ENABLE();
			/* _FORCE_ the scheduler to wait for the thread to acknowledge the
			 * suspension request when its being hosted by a different CPU that
			 * still hasn't gotten around to handling the associated IPC interrupt. */
#ifdef CONTEXT_INTEGER
			/* Try not to request an empty context, just so
			 * the kernel can't just ignore the request. */
			ignored_context.ContextFlags = CONTEXT_INTEGER;
#else /* CONTEXT_INTEGER */
			ignored_context.ContextFlags = 0;
#endif /* !CONTEXT_INTEGER */
			DBG_ALIGNMENT_DISABLE();
			GetThreadContext(self->t_thread, &ignored_context);
			DBG_ALIGNMENT_ENABLE();
#elif defined(CONFIG_HAVE_PTHREAD_SUSPEND)
			DBG_ALIGNMENT_DISABLE();
			pthread_suspend(self->t_thread);
			DBG_ALIGNMENT_ENABLE();
#else
#ifndef CONFIG_NEED_SUSPEND_SIGNALS
#error "Invalid configuration"
#endif /* !CONFIG_NEED_SUSPEND_SIGNALS */
			unsigned int timeout;
			int error;
			ATOMIC_FETCHOR(self->t_state, THREAD_STATE_SUSPENDREQ);
restart:
			timeout = 10;
			/* Send the signal to the thread. */
			DBG_ALIGNMENT_DISABLE();
			error = pthread_kill(self->t_thread, PTHREAD_INTERRUPT_SIGNAL);
			DBG_ALIGNMENT_ENABLE();
			if (error)
				BREAKPOINT();
			/* Since signal delivery happens asynchronously, we need to
			 * wait for the thread to acknowledge the suspend request. */
			while (ATOMIC_READ(self->t_state) & THREAD_STATE_SUSPENDREQ) {
				ASSERT(ATOMIC_READ(self->t_suspended));
				SCHED_YIELD();
				/* There seems to be a race condition connected to signal delivery
				 * that results in the signal (maybe) being delivered, but (somehow)
				 * not ending up clearing the suspend-req flag???
				 * Anyways... While really hacky, this seems to fix it. */
				if (!timeout--)
					goto restart;
			}
#endif
		}
	}
}

PRIVATE NONNULL((1)) void DCALL
do_resume_thread(DeeThreadObject *__restrict self) {
	if (ATOMIC_DECFETCH(self->t_suspended) == 0) {
		if (self->t_state & THREAD_STATE_STARTED) {
#ifdef CONFIG_THREADS_WINDOWS
			DBG_ALIGNMENT_DISABLE();
			ResumeThread(self->t_thread);
			DBG_ALIGNMENT_ENABLE();
#elif defined(CONFIG_HAVE_PTHREAD_SUSPEND)
			DBG_ALIGNMENT_DISABLE();
			pthread_continue(self->t_thread);
			DBG_ALIGNMENT_ENABLE();
#else
#ifndef CONFIG_NEED_SUSPEND_SIGNALS
#error "Invalid configuration"
#endif /* !CONFIG_NEED_SUSPEND_SIGNALS */
			DBG_ALIGNMENT_DISABLE();
			pthread_kill(self->t_thread, PTHREAD_INTERRUPT_SIGNAL);
			DBG_ALIGNMENT_ENABLE();
#endif
		}
	}
}

/* This lock needs to be recursive so that `destroy_thread_self()'
 * can old onto it while cleaning out its members.
 * Additionally, this lock is held whenever
 * any thread has suspended some other thread. */
PRIVATE DEFINE_RECURSIVE_RWLOCK(globthread_lock);


/* Similar to `DeeThread_SuspendAll()', keep a lock
 * to `globthread_lock' while suspending other others.
 * That way, only a single thread can ever suspend any
 * others and we prevent the race condition arising from
 * 2 threads attempting to suspend each other. */
PUBLIC NONNULL((1)) void DCALL
DeeThread_Suspend(DeeThreadObject *__restrict self) {
	recursive_rwlock_write(&globthread_lock);
	do_suspend_thread(self);
}

PUBLIC NONNULL((1)) void DCALL
DeeThread_Resume(DeeThreadObject *__restrict self) {
	do_resume_thread(self);
	recursive_rwlock_endwrite(&globthread_lock);
}

PUBLIC WUNUSED ATTR_RETNONNULL DeeThreadObject *DCALL DeeThread_SuspendAll(void) {
	/* Acquire the calling thread's context (this
	 * ensures that the caller is tracked and identifiable) */
	DeeThreadObject *iter, *ts = DeeThread_Self();
	/* NOTE: Acquire (and keep) a write-lock to ensure that only
	 *       a single thread is ever able to suspend all others. */
	recursive_rwlock_write(&globthread_lock);
	iter = &DeeThread_Main;
	do {
		/* Suspend all threads but the caller. */
		if (iter != ts)
			do_suspend_thread(iter);
	} while ((iter = iter->t_globalnext) != NULL);
	/* Expose the global thread list to allow the caller to enumerate it. */
	return &DeeThread_Main;
}

PUBLIC void DCALL DeeThread_ResumeAll(void) {
	DeeThreadObject *iter, *ts;
	ASSERT(recursive_rwlock_writing(&globthread_lock));
	ts   = DeeThread_Self();
	iter = &DeeThread_Main;
	do {
		/* Resume all threads but the caller. */
		if (iter != ts)
			do_resume_thread(iter);
	} while ((iter = iter->t_globalnext) != NULL);
	/* Release the write-lock acquired in `DeeThread_SuspendAll()' */
	recursive_rwlock_endwrite(&globthread_lock);
}

PRIVATE NONNULL((1)) void DCALL
add_running_thread(DeeThreadObject *__restrict thread) {
	thread->t_globlpself = &DeeThread_Main.t_globalnext;
	recursive_rwlock_write(&globthread_lock);
	if ((thread->t_globalnext = DeeThread_Main.t_globalnext) != NULL)
		thread->t_globalnext->t_globlpself = &thread->t_globalnext;
	DeeThread_Main.t_globalnext = thread;
	recursive_rwlock_endwrite(&globthread_lock);
}

PRIVATE NONNULL((1)) void DCALL
del_running_thread(DeeThreadObject *__restrict thread) {
	recursive_rwlock_write(&globthread_lock);
	if (thread->t_globlpself) {
		if ((*thread->t_globlpself = thread->t_globalnext) != NULL)
			thread->t_globalnext->t_globlpself = thread->t_globlpself;
	}
	recursive_rwlock_endwrite(&globthread_lock);
}

/* Join all threads that are still running
 * after sending an interrupt signal to each.
 * Returns true if at least one thread was joined. */
PUBLIC WUNUSED bool (DCALL DeeThread_JoinAll)(void) {
	DeeThreadObject *iter;
	bool interrupt_phase = true;
	bool result          = false;
again:
	recursive_rwlock_write(&globthread_lock);
again_locked:
	iter = DeeThread_Main.t_globalnext;
	while (iter) {
		dref_t refcnt;
		if (interrupt_phase) {
			if (iter->t_state & THREAD_STATE_SHUTDOWNINTR)
				goto next_thread;
		}
		do { /* IncrefIfNotZero() */
			refcnt = ATOMIC_READ(iter->ob_refcnt);
			if (!refcnt)
				goto next_thread;
		} while (!ATOMIC_CMPXCH(iter->ob_refcnt, refcnt, refcnt + 1));
		goto handle_iter;
next_thread:
		iter = iter->t_globalnext;
	}
	/* Once all threads have been interrupted, move
	 * on to the second phase of joining them all. */
	if (interrupt_phase) {
		interrupt_phase = false;
		goto again_locked;
	}
	if (iter) {
handle_iter:
		if (!interrupt_phase) {
			/* Pop this thread from the global chain when running the 2nd phase. */
			if ((*iter->t_globlpself = iter->t_globalnext) != NULL)
				iter->t_globalnext->t_globlpself = iter->t_globlpself;
			iter->t_globlpself = NULL;
			iter->t_globalnext = NULL;
		}
	}
	recursive_rwlock_endwrite(&globthread_lock);
	/* When no more threads are left, then we are done! */
	if (!iter)
		return result;
	/* There are still some threads left -> Return `true' eventually. */
	result = true;
	if (interrupt_phase) {
		int error;
		/* First phase: Send interrupt signals to all threads. */
		error = DeeThread_Interrupt((DeeObject *)iter,
		                            &DeeError_Interrupt_instance,
		                            NULL);
		ATOMIC_FETCHOR(iter->t_state, THREAD_STATE_SHUTDOWNINTR);
		/* NOTE: Also handle interrupt signals on error, because we're
		 *       the ones trying to interrupt all the other threads. */
		if (error < 0)
			DeeError_Print("Failed to signal interrupt\n",
			               ERROR_PRINT_HANDLEINTR);
	} else {
		/* Second phase: Join all running threads. */
		uint16_t state;
		while (ATOMIC_FETCHOR(iter->t_state, THREAD_STATE_DETACHING) &
		       THREAD_STATE_DETACHING) {
			DeeThread_SleepNoInterrupt(1000);
		}
#ifdef CONFIG_THREADS_WINDOWS
		DBG_ALIGNMENT_DISABLE();
		WaitForSingleObject(iter->t_thread, INFINITE);
		DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_THREADS_WINDOWS */
#ifdef EINTR
		/* Ignore attempts to interrupt us. */
		DBG_ALIGNMENT_DISABLE();
		while (pthread_join(iter->t_thread, NULL) == EINTR)
			;
		DBG_ALIGNMENT_ENABLE();
#else /* EINTR */
		DBG_ALIGNMENT_DISABLE();
		pthread_join(iter->t_thread, NULL);
		DBG_ALIGNMENT_ENABLE();
#endif /* !EINTR */
#endif /* !CONFIG_THREADS_WINDOWS */
		do {
			state = ATOMIC_READ(iter->t_state);
		} while (!ATOMIC_CMPXCH_WEAK(iter->t_state, state,
		                             (state & ~(THREAD_STATE_DETACHING)) |
		                             THREAD_STATE_DETACHED |
		                             THREAD_STATE_DIDJOIN));
	}
	Dee_Decref(iter);
	/* Continue joining all the other threads. */
	goto again;
}



PRIVATE WUNUSED DREF DeeThreadObject *DCALL
allocate_thread_self(void) {
	DREF DeeThreadObject *result;
again:
	result = DeeObject_TRYCALLOC(DeeThreadObject);
	if unlikely(!result) {
		/* We can attempt to clear out caches to get more memory,
		 * as those will not go so far as to invoke user-code callbacks,
		 * or require the thread.self object in any other way. */
		if (DeeMem_ClearCaches(sizeof(DeeThreadObject)))
			goto again;
		/* Well shit! Try one more time?... */
		result = DeeObject_TRYCALLOC(DeeThreadObject);
		if (!result)
			return NULL;
	}
	DeeObject_Init(result, &DeeThread_Type);
	result->t_state             = THREAD_STATE_STARTED;
	result->t_deepassoc.da_list = empty_deep_assoc;
	/* Lookup descriptor numbers for the calling thread. */
	result->t_thread = os_getcurrenthread();
#ifndef CONFIG_NO_THREADID
	DBG_ALIGNMENT_DISABLE();
	result->t_threadid = os_gettid_with_thread(result->t_thread);
	DBG_ALIGNMENT_ENABLE();
#endif /* !CONFIG_NO_THREADID */
#ifdef CONFIG_THREADS_JOIN_SEMPAHORE
	/* Construct the join-semaphore.
	 * XXX: Error handling? - But then again: the handling would mean to just give up and `abort()' */
#ifdef CONFIG_HOST_WINDOWS
	DBG_ALIGNMENT_DISABLE();
	result->t_join = CreateSemaphoreW(NULL, 0, 0x7fffffff, NULL);
	DBG_ALIGNMENT_ENABLE();
#elif !defined(CONFIG_NO_SEMAPHORE_H)
	DBG_ALIGNMENT_DISABLE();
	sem_init(&result->t_join, 0, 0);
	DBG_ALIGNMENT_ENABLE();
#endif
#endif /* CONFIG_THREADS_JOIN_SEMPAHORE */
	return result;
}

PRIVATE NONNULL((1)) bool DCALL thread_doclear(DeeThreadObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL thread_fini(DeeThreadObject *__restrict self);

PRIVATE void DCALL
destroy_thread_self(DREF DeeThreadObject *__restrict self) {
	dref_t refcnt;
	uint16_t old_state;
	bool did_shutdown = false;
#ifdef CONFIG_THREADS_PTHREAD
	/* Signify thread termination. */
#ifdef CONFIG_HOST_WINDOWS
	DBG_ALIGNMENT_DISABLE();
	ReleaseSemaphore(self->t_join, 1, NULL);
	DBG_ALIGNMENT_ENABLE();
#elif !defined(CONFIG_NO_SEMAPHORE_H)
	DBG_ALIGNMENT_DISABLE();
	sem_post(&self->t_join);
	DBG_ALIGNMENT_ENABLE();
#else
	/* The fallback implements this is an atomic flag. */
	ATOMIC_WRITE(self->t_join, 1);
#endif
#endif /* CONFIG_THREADS_PTHREAD */
	/* Drop a reference from `self' while also
	 * switching the thread's state to being terminated. */
	recursive_rwlock_write(&globthread_lock);
	if (self->t_globlpself) {
		if ((*self->t_globlpself = self->t_globalnext) != NULL)
			self->t_globalnext->t_globlpself = self->t_globlpself;
		self->t_globlpself = NULL;
	}
	for (;;) {
		/* Set the terminated-flag to prevent fields from being re-initialized.
		 * Without this, thread finalization might re-invoke thread members at
		 * a time when doing so is no longer valid. */
		do {
			old_state = ATOMIC_READ(self->t_state);
		} while (!ATOMIC_CMPXCH_WEAK(self->t_state, old_state,
		                             (old_state & (THREAD_STATE_DETACHED | THREAD_STATE_DETACHING |
		                                           THREAD_STATE_STARTING | THREAD_STATE_INTERRUPTING)) |
		                             THREAD_STATE_TERMINATED));
		if (!did_shutdown) {
			/* During the first pass, handle any remaining interrupts that another
			 * thread may have send to this one, yet hadn't been handled thus far.
			 * NOTE: Interrupts are not checked if the thread has no dangling exceptions. */
			if (self->t_exceptsz == 0 &&
			    DeeThread_CheckInterruptSelf(self)) {
				/* Just as in the main thread function, ignore any errors derived from
				 * `Signal.Interrupt', as those don't count as true errors, but rather
				 * as mere hints for the thread to gracefully stop execution. */
				while (DeeError_Catch(&DeeError_Interrupt))
					;
			}
#ifdef CONFIG_NO_SYS_THREADSHUTDOWN
			did_shutdown = true;
#endif /* CONFIG_NO_SYS_THREADSHUTDOWN */
		}
		refcnt = ATOMIC_READ(self->ob_refcnt);
		if (refcnt == 1) {
			/* Clear the thread while its reference counter it still non-zero,
			 * in case doing so invokes usercode, thus making further use of `thread.self()'. */
			while (thread_doclear(self))
				;
#ifndef CONFIG_NO_SYS_THREADSHUTDOWN
		}
		if (!did_shutdown) {
			/* Try to perform a system shutdown only after the thread was cleared. */
			sys_threadshutdown(self);
			did_shutdown = true;
		}
		if (refcnt == 1) {
#endif /* !CONFIG_NO_SYS_THREADSHUTDOWN */
			if (!ATOMIC_CMPXCH(self->ob_refcnt, 1, 0))
				continue;
			DeeGC_Untrack((DeeObject *)self);
			/* Assert that the thread is now in a state in which any remaining
			 * termination callbacks no longer rely on a valid thread being
			 * accessible through `DeeThread_Self()' */
			ASSERT(!self->t_exec);
			ASSERT(!self->t_execsz);
			ASSERT(!self->t_str_curr);
			ASSERT(!self->t_repr_curr);
			ASSERT(!self->t_except);
			ASSERT(!self->t_exceptsz);
			ASSERT(!self->t_deepassoc.da_used);
			ASSERT((self->t_deepassoc.da_mask != 0) ==
			       (self->t_deepassoc.da_list != empty_deep_assoc));
			ASSERT(!self->t_threadname || DeeString_Check(self->t_threadname));
			ASSERT(!self->t_interrupt.ti_next);
			ASSERT(!self->t_interrupt.ti_intr);
			ASSERT(!self->t_threadmain);
			ASSERT(!self->t_tlsdata);
			ASSERT(self->t_threadargs == (DREF DeeTupleObject *)Dee_EmptyTuple);
			ASSERT(!self->t_threadres);
			thread_fini(self);
			DeeObject_FreeTracker((DeeObject *)self);
			DeeGCObject_FREE(self);
			break;
		}
		if (ATOMIC_CMPXCH(self->ob_refcnt, refcnt, refcnt - 1))
			break;
#if 0 /* No need to go back on this... */
		ATOMIC_WRITE(self->t_state,old_state);
#endif
	}
	/* WARNING: Once we release this lock, the main thread is allowed to assume that we no longer exist.
	 *          With that in mind, _ALL_ cleanup _MUST_ be done before this point! */
	recursive_rwlock_endwrite(&globthread_lock);
	DEE_CHECKMEMORY();
}

#ifndef __NO_ATTR_THREAD
#define THREAD_SELF_TLS_USE_ATTR_THREAD  1
#elif defined(CONFIG_THREADS_WINDOWS)
#define THREAD_SELF_TLS_USE_TLS_ALLOC    1
#else
#define THREAD_SELF_TLS_USE_PTHREAD_KEY  1
#endif

PRIVATE void DCALL DeeThread_InitMain(void) {
	/* Collect information on the main thread. */
	DeeThread_Main.t_thread = os_getcurrenthread();
#ifndef CONFIG_NO_THREADID
	DBG_ALIGNMENT_DISABLE();
	DeeThread_Main.t_threadid = os_gettid_with_thread(DeeThread_Main.t_thread);
	DBG_ALIGNMENT_ENABLE();
#endif /* !CONFIG_NO_THREADID */
#ifdef CONFIG_THREADS_JOIN_SEMPAHORE
#ifdef CONFIG_HOST_WINDOWS
	DBG_ALIGNMENT_DISABLE();
	DeeThread_Main.t_join = CreateSemaphoreW(NULL, 0, 0x7fffffff, NULL);
	DBG_ALIGNMENT_ENABLE();
#elif !defined(CONFIG_NO_SEMAPHORE_H)
	DBG_ALIGNMENT_DISABLE();
	sem_init(&DeeThread_Main.t_join, 0, 0);
	DBG_ALIGNMENT_ENABLE();
#endif
#endif /* CONFIG_THREADS_JOIN_SEMPAHORE */
}

#ifdef CONFIG_HOST_WINDOWS
PRIVATE void DCALL DeeThread_FiniMain(void) {
	/* This one we've duplicated before. */
	DBG_ALIGNMENT_DISABLE();
	CloseHandle(DeeThread_Main.t_thread);
	DBG_ALIGNMENT_ENABLE();
}
#else /* CONFIG_HOST_WINDOWS */
#define DeeThread_FiniMain() (void)0
#endif /* !CONFIG_HOST_WINDOWS */

/* Platform-dependent part: thread.self TLS management. */
#ifdef THREAD_SELF_TLS_USE_TLS_ALLOC
#ifdef NDEBUG
PRIVATE DWORD thread_self_tls;
#else /* NDEBUG */
PRIVATE DWORD thread_self_tls = TLS_OUT_OF_INDEXES;
#endif /* !NDEBUG */

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
FORCELOCAL void *(thread_tls_get)(void) {
	void *result;
	__asm {
		MOV EAX, thread_self_tls
		MOV EAX, DWORD PTR FS:[0xe10 + EAX * 4]
		MOV result, EAX
	}
	return result;
}
#define thread_tls_set(value) thread_tls_set(value)
FORCELOCAL void(thread_tls_set)(void *value) {
	__asm {
		MOV ECX, value
		MOV EAX, thread_self_tls
		MOV DWORD PTR FS:[0xe10 + EAX * 4], ECX
	}
}
#elif defined(__COMPILER_HAVE_GCC_ASM)
#define thread_tls_get()  thread_tls_get()
FORCELOCAL void *(thread_tls_get)(void) {
	register void *result;
	__asm__("movl %%fs:0xe10(,%1,4), %0\n"
	        : "=r"(result)
	        : "r"(thread_self_tls));
	return result;
}
#define thread_tls_set(value) thread_tls_set(value)
FORCELOCAL void(thread_tls_set)(void *value) {
	__asm__("movl %1, %%fs:0xe10(,%0,4)\n"
	        :
	        : "r"(thread_self_tls), "r"(value));
}
#endif
#endif /* __i386__ */

#ifndef thread_tls_get
#define thread_tls_get()      TlsGetValue(thread_self_tls)
#define thread_tls_set(value) TlsSetValue(thread_self_tls, (LPVOID)(value))
#endif /* !thread_tls_get */

PUBLIC void DCALL DeeThread_Init(void) {
	ASSERT(thread_self_tls == TLS_OUT_OF_INDEXES);
	DBG_ALIGNMENT_DISABLE();
	thread_self_tls = TlsAlloc();
	if unlikely(thread_self_tls == TLS_OUT_OF_INDEXES) {
#ifndef CONFIG_NO_STDIO
		fprintf(stderr, "Failed to initialize deemon thread subsystem: "
		                "Couldn't allocate Thread.current Tls: %u\n",
		        (unsigned int)GetLastError());
#endif /* !CONFIG_NO_STDIO */
		abort();
	}
	/* Set the thread-self TLS value of the main thread. */
	thread_tls_set(&DeeThread_Main);
	DBG_ALIGNMENT_ENABLE();
	DeeThread_InitMain();
}

PUBLIC void DCALL DeeThread_Fini(void) {
#ifndef NDEBUG
	ASSERT(thread_self_tls != TLS_OUT_OF_INDEXES);
#endif /* !NDEBUG */
	DeeThread_FiniMain();
	DBG_ALIGNMENT_DISABLE();
	TlsFree(thread_self_tls);
	DBG_ALIGNMENT_ENABLE();
#ifndef NDEBUG
	thread_self_tls = TLS_OUT_OF_INDEXES;
#endif /* !NDEBUG */
	ASSERT(!DeeThread_Main.t_deepassoc.da_used);
	if (DeeThread_Main.t_deepassoc.da_list != empty_deep_assoc) {
		Dee_Free(DeeThread_Main.t_deepassoc.da_list);
		DeeThread_Main.t_deepassoc.da_list = empty_deep_assoc;
		DeeThread_Main.t_deepassoc.da_mask = 0;
	}
}

PUBLIC void DCALL DeeThread_Shutdown(void) {
	DREF DeeThreadObject *self;
	DBG_ALIGNMENT_DISABLE();
	self = (DeeThreadObject *)thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if (!self)
		return;
	ASSERT(self != &DeeThread_Main);
	destroy_thread_self(self);
	DBG_ALIGNMENT_DISABLE();
	thread_tls_set(NULL);
	DBG_ALIGNMENT_ENABLE();
}
#else /* THREAD_SELF_TLS_USE_TLS_ALLOC */

#ifdef THREAD_SELF_TLS_USE_PTHREAD_KEY
PRIVATE pthread_key_t thread_self_tls;
#define thread_tls_set(v) pthread_setspecific(thread_self_tls, (void *)(v))
#define thread_tls_get()  pthread_getspecific(thread_self_tls)
#else /* THREAD_SELF_TLS_USE_PTHREAD_KEY */
PRIVATE ATTR_THREAD DREF DeeThreadObject *thread_self_tls = NULL;
#define thread_tls_set(v) (thread_self_tls = (v))
#define thread_tls_get()  thread_self_tls
#endif /* !THREAD_SELF_TLS_USE_PTHREAD_KEY */


PUBLIC void DCALL DeeThread_Init(void) {
#ifdef THREAD_SELF_TLS_USE_PTHREAD_KEY
	int error;
	DBG_ALIGNMENT_DISABLE();
	error = pthread_key_create(&thread_self_tls, NULL);
	if unlikely(error) {
#ifndef CONFIG_NO_STDIO
		fprintf(stderr, "Failed to initialize deemon thread subsystem: "
		                "Couldn't allocate Thread.current Tls: %d - %s\n",
		        error, strerror(error));
#endif /* !CONFIG_NO_STDIO */
		abort();
	}
#endif /* THREAD_SELF_TLS_USE_PTHREAD_KEY */
	/* Set the thread-self TLS value of the main thread. */
	DBG_ALIGNMENT_DISABLE();
	thread_tls_set(&DeeThread_Main);
	DBG_ALIGNMENT_ENABLE();
	/* Initialize the join-semaphore for the main thread. */
	DeeThread_InitMain();
}

PUBLIC void DCALL DeeThread_Fini(void) {
	DeeThread_FiniMain();
	if (DeeThread_Main.t_deepassoc.da_list != empty_deep_assoc)
		Dee_Free(DeeThread_Main.t_deepassoc.da_list);
	DeeThread_Main.t_deepassoc.da_used = 0;
	DeeThread_Main.t_deepassoc.da_mask = 0;
	DeeThread_Main.t_deepassoc.da_list = empty_deep_assoc;
#ifdef THREAD_SELF_TLS_USE_PTHREAD_KEY
	DBG_ALIGNMENT_DISABLE();
	pthread_key_delete(thread_self_tls);
	DBG_ALIGNMENT_ENABLE();
#endif /* THREAD_SELF_TLS_USE_PTHREAD_KEY */
}
#endif /* !THREAD_SELF_TLS_USE_TLS_ALLOC */

PUBLIC void DCALL DeeThread_Shutdown(void) {
	DREF DeeThreadObject *self;
	DBG_ALIGNMENT_DISABLE();
	self = (DeeThreadObject *)thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if (!self)
		return;
	ASSERT(self != &DeeThread_Main);
	destroy_thread_self(self);
	DBG_ALIGNMENT_DISABLE();
	thread_tls_set(NULL);
	DBG_ALIGNMENT_ENABLE();
}

PUBLIC WUNUSED ATTR_CONST ATTR_RETNONNULL DeeThreadObject *DCALL DeeThread_Self(void) {
	DeeThreadObject *result;
#ifdef THREAD_SELF_TLS_USE_TLS_ALLOC
	ASSERT(thread_self_tls != TLS_OUT_OF_INDEXES);
#endif /* THREAD_SELF_TLS_USE_TLS_ALLOC */
	DBG_ALIGNMENT_DISABLE();
	result = (DeeThreadObject *)thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if likely(result)
		return result;
	/* Lazily create the thread-self descriptor. */
	result = allocate_thread_self();
	if unlikely(!result) {
#ifndef CONFIG_NO_STDIO
#ifdef CONFIG_HOST_WINDOWS
		fprintf(stderr, "Failed to lazily allocate the thread-self descriptor: %lu\n",
		        (unsigned long)GetLastError());
#else /* CONFIG_HOST_WINDOWS */
		fprintf(stderr, "Failed to lazily allocate the thread-self descriptor: %d\n",
		        (int)errno);
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_NO_STDIO */
		abort();
	}
	/* Save the generated thread object in the TLS slot. */
	DBG_ALIGNMENT_DISABLE();
	thread_tls_set(result);
	DBG_ALIGNMENT_ENABLE();
	add_running_thread(result);
	sys_threadstartup(result);
	return result;
}

#if defined(CONFIG_THREADS_PTHREAD) || \
    defined(CONFIG_NEED_SUSPEND_SIGNALS)
PRIVATE void suspend_signal_handler(int UNUSED(signo)) {
#ifdef CONFIG_NEED_SUSPEND_SIGNALS
	DeeThreadObject *caller;
	/* Read out the calling thread. */
	DBG_ALIGNMENT_DISABLE();
	caller = (DeeThreadObject *)thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	ASSERT(caller);
	/* Clear the suspension-requested flag for the calling thread,
	 * indicating that the thread is now being suspended. */
	ATOMIC_FETCHAND(caller->t_state, ~THREAD_STATE_SUSPENDREQ);
	/* Suspension is really as simple as this (NOTE: `pause()' is async-safe) */
	while (caller->t_suspended) {
		/* Must preserve errno for the caller. */
		int old_error = errno;
		DBG_ALIGNMENT_DISABLE();
		pause();
		DBG_ALIGNMENT_ENABLE();
		errno = old_error;
	}
#endif /* CONFIG_NEED_SUSPEND_SIGNALS */
}
#endif /* CONFIG_THREADS_PTHREAD || CONFIG_NEED_SUSPEND_SIGNALS */

#ifndef CONFIG_NO_THREADID
PUBLIC WUNUSED ATTR_CONST dthreadid_t (DCALL DeeThread_SelfId)(void) {
	dthreadid_t result;
	DBG_ALIGNMENT_DISABLE();
	result = os_gettid();
	DBG_ALIGNMENT_ENABLE();
	return result;
}
#endif /* !CONFIG_NO_THREADID */

#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
INTERN uint8_t keyboard_interrupt_counter = 0;
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */


INTERN WUNUSED NONNULL((1)) int
(DCALL DeeThread_CheckInterruptSelf)(DeeThreadObject *__restrict self) {
	DREF DeeObject *interrupt_main;
	DREF DeeTupleObject *interrupt_args;
	DREF DeeObject *callback_result;
	struct thread_interrupt *next;
	/* Check: Is there an interrupt present for our thread, or are interrupt disabled? */
	if ((self->t_state & (THREAD_STATE_INTERRUPTING | THREAD_STATE_INTERRUPTED)) == 0)
		return 0;
	if ((self->t_state & THREAD_STATE_NOINTERRUPT))
		return 0;
	for (;;) {
		COMPILER_READ_BARRIER();
		if (self->t_state & THREAD_STATE_INTERRUPTED)
			break;
		/* The interrupting-flag may get unset if construction is aborted
		 * for some reason by the other end (e.g. allocation failure) */
		if (!(self->t_state & THREAD_STATE_INTERRUPTING))
			return 0;
		/* Assume that the interrupt object is currently being constructed,
		 * meaning the best thing we can do is to switch threads and let it
		 * continue being created. */
		SCHED_YIELD();
	}
next_interrupt:
	/* Atomically set the interrupting-flag to start processing the signal. */
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_INTERRUPTING) &
	       THREAD_STATE_INTERRUPTING)
		SCHED_YIELD();
	/* Pop one interrupt descriptor. */
	interrupt_main = self->t_interrupt.ti_intr; /* Inherit */
	interrupt_args = self->t_interrupt.ti_args; /* Inherit */
	ASSERT_OBJECT_TYPE_EXACT_OPT(interrupt_args, &DeeTuple_Type);
	if ((next = self->t_interrupt.ti_next) != NULL) {
		ASSERT(interrupt_main);
		/* Handle the case of more than one remaining action. */
		ASSERT(next->ti_intr);
		memcpy(&self->t_interrupt, next, sizeof(struct thread_interrupt));
		Dee_Free(next);
		/* NOTE: Don't clear the interrupted-flag since
		 *       there are still unhandled interrupts left. */
		ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_INTERRUPTING);
		if (!interrupt_args)
			goto throw_main;
		/* When an asynchronous callback is to-be executed, also handle
		 * the next interrupt if this one didn't cause any errors. */
		callback_result = DeeObject_Call(interrupt_main,
		                                 DeeTuple_SIZE(interrupt_args),
		                                 DeeTuple_ELEM(interrupt_args));
		Dee_Decref(interrupt_args);
		Dee_Decref(interrupt_main);
		if unlikely(!callback_result)
			goto err;
		Dee_Decref(callback_result);
		goto next_interrupt;
	}
	self->t_interrupt.ti_intr = NULL;
	self->t_interrupt.ti_args = NULL;
	ATOMIC_FETCHAND(self->t_state, ~(THREAD_STATE_INTERRUPTING |
	                                 THREAD_STATE_INTERRUPTED));
	/* Last interrupt action. */
	if (interrupt_args) {
		ASSERT(interrupt_main);
		/* Call the given interrupt-object, thus executing it. */
		callback_result = DeeObject_Call(interrupt_main,
		                                 DeeTuple_SIZE(interrupt_args),
		                                 DeeTuple_ELEM(interrupt_args));
		Dee_Decref(interrupt_args);
		Dee_Decref(interrupt_main);
		if (!callback_result)
			goto err;
		Dee_Decref(callback_result);
		return 0;
	}
	if (interrupt_main) {
throw_main:
		/* Throw the object that caused the interrupt in the context of this thread. */
		DeeError_Throw(interrupt_main);
		Dee_Decref(interrupt_main);
err:
		return -1;
	}
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
	/* Without any other interrupts, the main thread
	 * can still handle keyboard interrupts! */
	if (self == &DeeThread_Main) {
		uint8_t count;
		DeeSignalObject *keyboard_interrupt;
		/* try to consume one keyboard interrupt. */
		do {
			if ((count = ATOMIC_READ(keyboard_interrupt_counter)) == 0)
				return 0;
		} while (!ATOMIC_CMPXCH_WEAK(keyboard_interrupt_counter, count, count - 1));
		keyboard_interrupt = DeeObject_MALLOC(DeeSignalObject);
		if unlikely(!keyboard_interrupt)
			goto err;
		DeeObject_Init(keyboard_interrupt, &DeeError_KeyboardInterrupt);
		DeeError_Throw((DeeObject *)keyboard_interrupt);
		Dee_Decref(keyboard_interrupt);
		goto err;
	}
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */
	return 0;
}

PUBLIC WUNUSED int (DCALL DeeThread_CheckInterrupt)(void) {
	return DeeThread_CheckInterruptSelf(DeeThread_Self());
}


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_GetThread(/*Thread*/ DeeObject *__restrict self,
                    dthread_t *__restrict pthread) {
	ASSERT(pthread);
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
	if unlikely(!(((DeeThreadObject *)self)->t_state & THREAD_STATE_STARTED)) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Thread %k has no descriptor because it wasn't started",
		                self);
		return -1;
	}
	*pthread = ((DeeThreadObject *)self)->t_thread;
	return 0;
}


#ifdef CONFIG_THREADS_WINDOWS
PRIVATE DWORD WINAPI thread_entry(DREF DeeThreadObject *__restrict self)
#else /* CONFIG_THREADS_WINDOWS */
PRIVATE void *thread_entry(DREF DeeThreadObject *__restrict self)
#endif /* !CONFIG_THREADS_WINDOWS */
{
	DREF DeeObject *threadmain, *threadargs, *result;
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
	ASSERT(!self->t_threadres);
	ASSERT_OBJECT_TYPE_EXACT(self->t_threadargs, &DeeTuple_Type);

	DBG_ALIGNMENT_DISABLE();
#ifndef CONFIG_NO_THREADID
#ifdef CONFIG_THREADS_PTHREAD
	/* On non-windows hosts, a thread must save its own thread-id. */
	self->t_threadid = os_gettid();
#endif /* CONFIG_THREADS_PTHREAD */
#endif /* !CONFIG_NO_THREADID */

	/* Save the thread-self object in the TLS descriptor,
	 * letting that descriptor inherit its value. */
	thread_tls_set(self);
	DBG_ALIGNMENT_ENABLE();
	COMPILER_WRITE_BARRIER();

	sys_threadstartup(self);
	COMPILER_BARRIER();

	threadmain = (DREF DeeObject *)self->t_threadmain;
	threadargs = (DREF DeeObject *)self->t_threadargs;
	Dee_XIncref(threadmain);
	Dee_Incref(threadargs);

#ifndef CONFIG_NO_SETTHREADNAME
	if (self->t_threadname) {
		sys_setthreadname(DeeString_STR(self->t_threadname));
	} else if (DeeFunction_Check(threadmain)) {
		/* If DDI allows, set the name of the function that's to-be executed. */
		DeeCodeObject *exec_code = ((DeeFunctionObject *)threadmain)->fo_code;
		char *name               = DeeCode_NAME(exec_code);
		if (name) {
			sys_setthreadname(name);
		} else if (exec_code == exec_code->co_module->mo_root) {
			sys_setthreadname(DeeString_STR(exec_code->co_module->mo_name));
		}
	}
#endif /* !CONFIG_NO_SETTHREADNAME */

	/* Confirm startup by deleting the starting-flag and setting the started-flag. */
	{
		uint16_t state;
		do {
			state = ATOMIC_READ(self->t_state);
		} while (!ATOMIC_CMPXCH_WEAK(self->t_state, state,
		                             (state & ~(THREAD_STATE_STARTING)) |
		                             THREAD_STATE_STARTED));
	}

#ifdef CONFIG_NEED_SUSPEND_SIGNALS
	/* If the thread was started as suspended, wait for the suspension to clean out.
	 * HINT: The suspended-requested flag was already cleared above. */
	while (ATOMIC_READ(self->t_suspended)) {
		ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_SUSPENDREQ);
		pause();
	}
#endif /* CONFIG_NEED_SUSPEND_SIGNALS */

	/* Before anything is actually executed, check for interrupts that
	 * may have been scheduled before the thread was even started.
	 * This way, the user can send interrupts before starting a thread
	 * and is allowed to assume that they will dealt with (in order)
	 * before the thread's actual main method. */
	if (DeeThread_CheckInterruptSelf(self))
		goto early_err;

	/* If no thread-main callback has been assigned,
	 * search for a `run()' member function. */
	if (!threadmain) {
		DREF DeeObject *old_main;
		threadmain = DeeObject_GetAttr((DeeObject *)self, &str_run);
		/* Without any run() member, an error will have been set and we'll already be done. */
		if unlikely(!threadmain) {
early_err:
			result = NULL;
			goto done_nomain;
		}

		/* Write the main function back into the thread object. */
		while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING) &
		       THREAD_STATE_STARTING)
			SCHED_YIELD();
		old_main = self->t_threadmain;
		if likely(!old_main) {
			/* Likely case: Sill no main function. - Store the one we've just figured out. */
			Dee_Incref(threadmain);
			self->t_threadmain = threadmain;
			ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
		} else {
			/* Call the new thread-main function that has been defined. */
			Dee_Incref(old_main);
			ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
			Dee_Decref(threadmain);
			threadmain = old_main;
		}
	}

	/* Invoke the thread's main() callback. */
	DEE_CHECKMEMORY();
	result = DeeObject_Call(threadmain,
	                        DeeTuple_SIZE(threadargs),
	                        DeeTuple_ELEM(threadargs));
	DEE_CHECKMEMORY();

	Dee_Decref(threadmain);
done_nomain:
	Dee_Decref(threadargs);

	if likely(result) {
set_result:
		if (self->t_state & THREAD_STATE_DETACHED) {
			/* Simplified case: No need to safe the result if the
			 * thread was detached and re-reading it is illegal. */
			Dee_Decref(result);
		} else {
			/* Save the thread return value on success. */
			while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING) &
			       THREAD_STATE_STARTING)
				SCHED_YIELD();
			ASSERT(!self->t_threadres);
			self->t_threadres = result; /* Inherit reference. */
			ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
		}
	} else {
		DeeObject *current;
		current = DeeError_Current();
		/* Special case: Check for a thread-exit exception. */
		if (current && DeeThreadExit_Check(current)) {
			result = DeeThreadExit_Result(current);
			Dee_Incref(result);
			DeeError_Handled(ERROR_HANDLED_INTERRUPT);
			while (DeeError_Catch(&DeeError_Interrupt))
				;
			/* If no further exceptions have occurred, set the thread-exit return value. */
			if (!DeeError_Current())
				goto set_result;
			Dee_Decref(result);
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
	}

	/* As the last set, destroy the thread descriptor.
	 * (Although destroy is the wrong term. - Rather: clear()+decref()) */
	DEE_CHECKMEMORY();
	destroy_thread_self(self);
	/* Don't write a new TLS value. - There'd be no point
	 * and we are no longer guarantied to continue running! */
	/*thread_tls_set(NULL);*/
	return 0;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeThread_Start(/*Thread*/ DeeObject *__restrict self) {
	uint16_t state;
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
	/* Set the starting-flag.
	 * During a successfully control flow, this flag is later
	 * replaced with the started-flag by the thread itself. */
	do {
		state = ATOMIC_READ(me->t_state);
		/* Check if the thread has already been started by testing
		 * if either the started, or terminated flag has been set. */
		if (state & (THREAD_STATE_STARTED | THREAD_STATE_TERMINATED))
			return 1;
	} while (!ATOMIC_CMPXCH_WEAK(me->t_state, state, state | THREAD_STATE_STARTING));
	/* Create the reference that is passed to `thread_entry()'
	 * and later stored in the thread's TLS self-pointer. */
	Dee_Incref(me);
	add_running_thread(me);
#ifdef CONFIG_THREADS_WINDOWS
	DBG_ALIGNMENT_DISABLE();
	me->t_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&thread_entry,
	                            self, 0,
#ifndef CONFIG_NO_THREADID
	                            (LPDWORD)&me->t_threadid
#else /* !CONFIG_NO_THREADID */
	                            NULL
#endif /* CONFIG_NO_THREADID */
	                            );
	DBG_ALIGNMENT_ENABLE();
	if unlikely(me->t_thread == NULL) {
		DWORD error;
		ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_STARTING);
		/* Drop the reference that never came to be... */
		del_running_thread(me);
		Dee_DecrefNokill(self);
		DBG_ALIGNMENT_DISABLE();
		error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		return DeeError_SysThrowf(&DeeError_SystemError, error,
		                          "Failed to start thread %k", self);
	}
#elif defined(CONFIG_THREADS_PTHREAD)
	{
		int error;
		/* Construct the semaphore that's going to be used to join the thread. */
		DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_THREADS_JOIN_SEMPAHORE
#ifdef CONFIG_HOST_WINDOWS
		me->t_join = CreateSemaphoreW(NULL, 0, 0x7fffffff, NULL);
		if unlikely(me->t_join == NULL) {
			error = GetLastError();
		} else
#elif !defined(CONFIG_NO_SEMAPHORE_H)
		if unlikely(sem_init(&me->t_join, 0, 0)) {
			error = errno;
		} else
#endif
#endif /* CONFIG_THREADS_JOIN_SEMPAHORE */
		{
			pthread_attr_t attr;
			error = pthread_attr_init(&attr);
			if likely(error == 0) {
				error = pthread_create(&me->t_thread, &attr,
				                       (void *(*)(void *))&thread_entry, self);
				pthread_attr_destroy(&attr);
			}
		}
		DBG_ALIGNMENT_ENABLE();
		if unlikely(error != 0) {
			ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_STARTING);
			/* Drop the reference that never came to be... */
			del_running_thread(me);
			Dee_DecrefNokill(self);
			return DeeError_SysThrowf(&DeeError_SystemError, error,
			                          "Failed to start thread %k", self);
		}
	}
#endif
	/* Wait for the thread to acknowledge having started,
	 * just so we can guaranty that the thread is not longer
	 * starting, but is either running, or has terminated. */
	while (ATOMIC_READ(me->t_state) & THREAD_STATE_STARTING)
		SCHED_YIELD();
	/* Indicate success. */
	return 0;
}

#ifndef CONFIG_THREADS_PTHREAD
#ifdef CONFIG_HOST_WINDOWS
PRIVATE VOID NTAPI
dummy_apc_func(ULONG_PTR UNUSED(Parameter)) {
}
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_THREADS_PTHREAD */


#ifndef CONFIG_THREADS_PTHREAD
#ifdef CONFIG_HOST_WINDOWS
typedef int (WINAPI *LPCANCELSYNCHRONOUSIO)(HANDLE hThread);
PRIVATE LPCANCELSYNCHRONOUSIO pCancelSynchronousIo = NULL;
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_THREADS_PTHREAD */

/* Try to wake the thread. */
PUBLIC NONNULL((1)) void DCALL
DeeThread_Wake(/*Thread*/ DeeObject *__restrict self) {
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);

#ifndef CONFIG_THREADS_PTHREAD
#ifdef CONFIG_HOST_WINDOWS
	DBG_ALIGNMENT_DISABLE();
	QueueUserAPC(&dummy_apc_func, me->t_thread, 0);
	/* Also try to interrupt synchronous I/O, meaning calls like `ReadFile()'.
	 * Sadly, we must manually check if that functionality is even available... */
	if (ITER_ISOK(pCancelSynchronousIo)) {
		(*pCancelSynchronousIo)(me->t_thread);
	} else if (!pCancelSynchronousIo) {
		LPCANCELSYNCHRONOUSIO ptr;
		*(void **)&ptr = GetProcAddress(GetModuleHandleW(kernel32), "CancelSynchronousIo");
		if (!ptr)
			*(void **)&ptr = (void *)ITER_DONE;
		pCancelSynchronousIo = ptr;
		if (ptr != (void *)ITER_DONE)
			(*ptr)(me->t_thread);
	}
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_THREADS_PTHREAD */

#ifdef CONFIG_THREADS_PTHREAD
	DBG_ALIGNMENT_DISABLE();
	pthread_kill(me->t_thread, PTHREAD_INTERRUPT_SIGNAL);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_THREADS_PTHREAD */
}

/* Schedule an interrupt for a given thread.
 * Interrupts are delivered when through threads
 * periodically calling `DeeThread_CheckInterrupt()'.
 * NOTE: Interrupts are delivered in order of being received.
 * NOTE: When `interrupt_args' is non-NULL, rather than throwing the given
 *      `interrupt_main' as an error upon arrival, it is invoked
 *       using `operator ()' with `interrupt_args' (which must be a tuple).
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 * @return:  0: Successfully scheduled the interrupt object.
 * @return:  1: The thread has been terminated. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Interrupt(/*Thread*/ DeeObject *self,
                    DeeObject *interrupt_main,
                    DeeObject *interrupt_args) {
	uint16_t state;
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
	ASSERT_OBJECT_TYPE_EXACT_OPT(interrupt_args, &DeeTuple_Type);
	/* Start interrupting the thread. */
restart:
	do {
reread_state:
		state = ATOMIC_READ(me->t_state);
		/* Check if the thread has already terminated. */
		if (state & THREAD_STATE_TERMINATED)
			return 1;
		if (state & THREAD_STATE_EXTERNAL) {
			/* If the thread wasn't started, that's an error. */
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot deliver interrupt to external thread %k",
			                self);
			goto err;
		}
#if 0
		if unlikely(!(state & THREAD_STATE_STARTED)) {
			/* If the thread wasn't started, that's an error. */
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot deliver interrupt to thread %k that was never started",
			                self);
			goto err;
		}
#endif
		/* If someone else is already interrupting, yield to them. */
		if (state & THREAD_STATE_INTERRUPTING) {
			SCHED_YIELD();
			goto reread_state;
		}
	} while (!ATOMIC_CMPXCH_WEAK(me->t_state, state, state | THREAD_STATE_INTERRUPTING));
	/* Create the reference that's going to be stored in the thread. */
	Dee_Incref(interrupt_main);
	Dee_XIncref(interrupt_args);
	if (!me->t_interrupt.ti_intr) {
		/* Simple case: The first and only scheduled interrupt. */
		me->t_interrupt.ti_intr = interrupt_main;
		me->t_interrupt.ti_args = (DREF DeeTupleObject *)interrupt_args;
	} else {
		/* Must allocate a new pending-interrupt entry. */
		struct thread_interrupt *new_entry, *last_entry;
		new_entry = (struct thread_interrupt *)Dee_TryMalloc(sizeof(struct thread_interrupt));
		if unlikely(!new_entry) {
			/* Failed to allocate the interrupt extension.
			 * Temporarily stop interrupting and try to collect memory. */
			OATOMIC_FETCHAND(me->t_state, ~(THREAD_STATE_INTERRUPTING),
			                 __ATOMIC_RELEASE);
			COMPILER_WRITE_BARRIER();
			/* Drop the reference we've created above. */
			Dee_DecrefNokill(interrupt_main);
			Dee_XDecrefNokill(interrupt_args);
			if (Dee_CollectMemory(sizeof(struct thread_interrupt)))
				goto restart;
			goto err;
		}
		/* Fill in the pending interrupt and prepend it to the chain of pending interrupts.
		 * The thread will then unwind that list in reverse order, meaning our interrupt
		 * will get delivered last. */
		last_entry = &me->t_interrupt;
		while (last_entry->ti_next)
			last_entry = last_entry->ti_next;
		new_entry->ti_next  = NULL;
		new_entry->ti_intr  = interrupt_main;
		new_entry->ti_args  = (DREF DeeTupleObject *)interrupt_args;
		last_entry->ti_next = new_entry;
	}
	/* Now we need to atomically delete the interrupting-flag,
	 * while making sure that the interrupted-flag gets set. */
	do {
		state = ATOMIC_READ(me->t_state);
	} while (!ATOMIC_CMPXCH_WEAK(me->t_state, state,
	                             (state & ~(THREAD_STATE_INTERRUPTING)) |
	                             (THREAD_STATE_INTERRUPTED)));
	/* Try to interrupt what the thread is currently doing, so it
	 * will check for pending interrupts and handle them immediately. */
	DeeThread_Wake(self);
	/* And we're done! */
	return 0;
err:
	return -1;
}

PRIVATE bool DCALL
try_alloc_wrappers(struct except_frame **__restrict presult,
                   uint16_t *__restrict pnum_wrappers) {
	bool result                = true;
	struct except_frame *chain = *presult;
	if (chain && !chain->ef_error) {
		chain->ef_error = (DREF DeeObject *)DeeObject_TRYMALLOC(DeeErrorObject);
		if unlikely(!chain->ef_error)
			goto fail;
	}
	while (*pnum_wrappers) {
		struct except_frame *new_frame;
		new_frame = except_frame_tryalloc();
		if unlikely(!new_frame)
			goto fail;
		new_frame->ef_prev = chain;
		chain              = new_frame;
		chain->ef_error    = (DREF DeeObject *)DeeObject_TRYMALLOC(DeeErrorObject);
		if unlikely(!chain->ef_error)
			goto fail;
		--*pnum_wrappers;
	}
done:
	*presult = chain;
	return result;
fail:
	result = false;
	goto done;
}

/* Detach the given thread.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 * @return:  0: The thread was successfully detached.
 * @return:  1: The thread had already been detached. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeThread_Detach(/*Thread*/ DeeObject *__restrict self) {
	uint16_t state;
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
	while (ATOMIC_FETCHOR(me->t_state, THREAD_STATE_DETACHING) &
	       THREAD_STATE_DETACHING)
		SCHED_YIELD();
	if (!(me->t_state & (THREAD_STATE_STARTED | THREAD_STATE_TERMINATED))) {
		/* Thread was never started. */
		ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Cannot detach thread %k that hasn't been started",
		                       self);
	}
	if (me->t_state & THREAD_STATE_EXTERNAL) {
		ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Cannot detach external thread %k",
		                       self);
	}
	if (me->t_state & THREAD_STATE_DETACHED) {
		/* Thread was already detached. */
		ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
		return 1;
	}

	/* Set the detached-flag and unset the detaching-flag. */
	do {
		state = ATOMIC_READ(me->t_state);
	} while (!ATOMIC_CMPXCH_WEAK(me->t_state, state,
	                             (state & ~THREAD_STATE_DETACHING) | THREAD_STATE_DETACHED));
	return 0;
}


/* Join the given thread.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 *              NOTE: If the thread crashed, its errors are propagated into the calling
 *                    thread after being encapsulated as `Error.ThreadError' objects.
 * @return:  0: Successfully joined the thread and wrote its return value in *pthread_result.
 * @return:  1: The given timeout has expired.
 * @param: timeout_microseconds: The timeout in microseconds, 0 for try-join,
 *                               or (uint64_t)-1 for infinite timeout. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Join(/*Thread*/ DeeObject *__restrict self,
               DREF DeeObject **__restrict pthread_result,
               uint64_t timeout_microseconds) {
	DeeThreadObject *me = (DeeThreadObject *)self;
	ASSERT(pthread_result);
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
again:
	if (!(me->t_state & THREAD_STATE_DIDJOIN)) {
		uint64_t timeout_end = (uint64_t)-1;
		uint16_t state, new_flags;
		/* Always set the did-join flag in the end. */
		new_flags = THREAD_STATE_DIDJOIN;
		if (timeout_microseconds != (uint64_t)-1) {
			timeout_end = DeeThread_GetTimeMicroSeconds();
			timeout_end += timeout_microseconds;
		}
		if (timeout_microseconds != 0 &&
		    DeeThread_CheckInterrupt())
			goto err;
		/* Wait for the thread to terminate. */
		while (ATOMIC_FETCHOR(me->t_state, THREAD_STATE_DETACHING) &
		       THREAD_STATE_DETACHING) {
			uint64_t now;
			/* Idly wait for another thread also in the process of joining this thread. */
			if (timeout_microseconds == 0)
				return 1; /* Timeout */
			if (DeeThread_Sleep(1000))
				goto err; /* Error (interrupt delivery) */
			now = DeeThread_GetTimeMicroSeconds();
			if (now >= timeout_end)
				return 1; /* Timeout */
			/* Update the remaining timeout. */
			timeout_microseconds = timeout_end - now;
		}
		/* Check for case: The thread was joined in the mean time. */
		if (me->t_state & THREAD_STATE_DIDJOIN)
			goto done_join;
		/* Check for case: The thread has terminated, but was never ~actually~ detached. */
		if ((me->t_state & (THREAD_STATE_TERMINATED | THREAD_STATE_DETACHED)) ==
		    THREAD_STATE_TERMINATED)
			goto done_join;
		if (me->t_state & THREAD_STATE_DETACHED) {
			/* Special case: The thread was detached, but not joined. */
			ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot join thread %k after being detached",
			                self);
			goto err;
		}
		{
#ifdef CONFIG_THREADS_WINDOWS
			/* Wait for the thread to complete. */
			DWORD wait_state;
			DBG_ALIGNMENT_DISABLE();
			wait_state = WaitForMultipleObjectsEx(1, (HANDLE *)&me->t_thread, FALSE,
			                                      timeout_microseconds == (uint64_t)-1
			                                      ? INFINITE
			                                      : (DWORD)(timeout_microseconds / 1000),
			                                      TRUE);
			DBG_ALIGNMENT_ENABLE();
			switch (wait_state) {
			case WAIT_IO_COMPLETION:
				/* Interrupt. */
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				goto again;
			case WAIT_TIMEOUT:
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				return 1; /* Timeout */
			case WAIT_FAILED:
				/* Error */
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				DBG_ALIGNMENT_DISABLE();
				wait_state = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				DeeError_SysThrowf(&DeeError_SystemError, wait_state,
				                   "Failed to join thread %k", self);
				goto err;
#if 0
			case WAIT_ABANDONED_0:
			case WAIT_OBJECT_0:
				break;
#endif
			default: break;
			}
#elif defined(CONFIG_THREADS_PTHREAD)
#ifndef CONFIG_THREADS_JOIN_SEMPAHORE
#error "Invalid configuration"
#endif
			/* XXX: Deal with external threads? */
#ifdef CONFIG_HOST_WINDOWS
			DWORD wait_state;
			DBG_ALIGNMENT_DISABLE();
			wait_state = WaitForMultipleObjectsEx(1, &me->t_join, TRUE,
			                                      timeout_microseconds == (uint64_t)-1 ? INFINITE : (DWORD)(timeout_microseconds / 1000), TRUE);
			DBG_ALIGNMENT_ENABLE();
			switch (wait_state) {

			case WAIT_IO_COMPLETION:
				/* Interrupt. */
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				goto again;

			case WAIT_TIMEOUT:
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				return 1; /* Timeout */

			case WAIT_FAILED:
				/* Error */
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				DBG_ALIGNMENT_DISABLE();
				wait_state = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				DeeError_SysThrowf(&DeeError_SystemError, wait_state,
				                   "Failed to join thread %k", self);
				goto err;

#if 0
			case WAIT_ABANDONED_0:
			case WAIT_OBJECT_0:
				break;
#endif
			default: break;
			}
#elif !defined(CONFIG_NO_SEMAPHORE_H)
			int error;
			if (timeout_microseconds == (uint64_t)-1) {
				DBG_ALIGNMENT_DISABLE();
				error = sem_wait(&me->t_join);
				DBG_ALIGNMENT_ENABLE();
			} else if (timeout_microseconds == 0) {
				DBG_ALIGNMENT_DISABLE();
				error = sem_trywait(&me->t_join);
				DBG_ALIGNMENT_ENABLE();
			} else {
				struct timespec join_time;
				DBG_ALIGNMENT_DISABLE();
				error = clock_gettime(CLOCK_REALTIME, &join_time);
				DBG_ALIGNMENT_ENABLE();
				if (!error) {
					join_time.tv_sec += (time_t)(timeout_microseconds / 1000000);
					join_time.tv_nsec += (long)(timeout_microseconds % 1000000) * 1000;
					join_time.tv_sec += join_time.tv_nsec / 1000000000;
					join_time.tv_nsec %= 1000000000;
					DBG_ALIGNMENT_DISABLE();
					error = sem_timedwait(&me->t_join, &join_time);
					DBG_ALIGNMENT_ENABLE();
				}
			}
			if unlikely(error != 0) {
				ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
				error = errno;
				/* Don't throw an error on busy/timeout. */
#ifdef ETIMEDOUT /* Returned by `sem_timedwait' */
				if (error == ETIMEDOUT)
					return 1;
#endif /* ETIMEDOUT */
#ifdef EAGAIN /* Returned by `sem_trywait' */
				if (error == EAGAIN)
					return 1;
#endif /* EAGAIN */
#ifdef EINTR
				if (error == EINTR)
					goto again;
#endif /* EINTR */
				DeeError_SysThrowf(&DeeError_SystemError, error,
				                   "Failed to join thread %k", self);
				goto err;
			}
#else
			int error;
			uint64_t time_end;
			if (timeout_microseconds == (uint64_t)-1) {
				while (!ATOMIC_READ(me->t_join)) {
					if (DeeThread_Sleep(1000)) {
						ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
						goto err; /* Interrupt received. */
					}
				}
			} else if (timeout_microseconds == 0) {
				if (!ATOMIC_READ(me->t_join)) {
					ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
					return 1; /* time-join failed. */
				}
			} else {
				uint64_t end_time;
				end_time = DeeThread_GetTimeMicroSeconds();
				end_time += timeout_microseconds;
				for (;;) {
					if (ATOMIC_READ(me->t_join))
						break;
					if (DeeThread_Sleep(1000)) {
						ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
						goto err;
					}
					if (DeeThread_GetTimeMicroSeconds() >= end_time) {
						ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_DETACHING);
						return 1; /* Timeout. */
					}
				}
			}
#endif
#else
#error "Unsupported thread configuration"
#endif
		}

done_join:
		/* Delete the detaching-flag and set the detached-flag. */
		do {
			state = ATOMIC_READ(me->t_state);
		} while (!ATOMIC_CMPXCH_WEAK(me->t_state, state,
		                             (state & ~THREAD_STATE_DETACHING) | new_flags));
	}

	/* Capture the thread's return value and
	 * potential errors that lead to its failure. */
	{
		DREF DeeObject *result;
		uint16_t current_alloc            = 0, req_alloc;
		struct except_frame *error_frames = NULL;
relock_state:
		while (ATOMIC_FETCHOR(me->t_state, THREAD_STATE_STARTING) &
		       THREAD_STATE_STARTING)
			SCHED_YIELD();
		/* In case errors have occurred during execution
		 * of the thread, package and rethrow those errors. */
		req_alloc = me->t_exceptsz;
		if (req_alloc != current_alloc) {
			if (req_alloc > current_alloc) {
				uint16_t total_alloc = req_alloc;
				req_alloc -= current_alloc;
				if unlikely(!try_alloc_wrappers(&error_frames, &req_alloc)) {
					current_alloc = total_alloc - req_alloc;
					ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_STARTING);
					/* Try to collect memory before allocating more wrappers. */
					do {
						if (!Dee_CollectMemory(sizeof(struct except_frame))) {
							while (error_frames) {
								struct except_frame *next = error_frames->ef_prev;
								DeeObject_FFree(error_frames->ef_error, sizeof(DeeErrorObject));
								except_frame_free(error_frames);
								error_frames = next;
							}
							goto err;
						}
					} while (!try_alloc_wrappers(&error_frames, &req_alloc));
					goto relock_state;
				}
				req_alloc = me->t_exceptsz;
			} else {
				uint16_t num_free = current_alloc - req_alloc;
				while (num_free--) {
					struct except_frame *next = error_frames->ef_prev;
					DeeObject_FFree(error_frames->ef_error, sizeof(DeeErrorObject));
					except_frame_free(error_frames);
					error_frames = next;
				}
			}
		}
		if (req_alloc) {
			/* Exceptions have occurred in the thread.
			 * Now we must mirror those in the buffer we've just painfully created. */
			struct except_frame *frame_dst = error_frames;
			struct except_frame *frame_src = me->t_except;
			current_alloc                  = req_alloc; /* Save how many error occurred. */
			while (req_alloc--) {
				/* Duplicate the exception frames themself and package errors in `Error.ThreadCrash'. */
				ASSERT(frame_src);
				DeeObject_Init(frame_dst->ef_error, &DeeError_ThreadCrash);
				((DeeErrorObject *)frame_dst->ef_error)->e_message = NULL;
				((DeeErrorObject *)frame_dst->ef_error)->e_inner   = frame_src->ef_error;
				frame_dst->ef_trace                                = frame_src->ef_trace;
				Dee_Incref(frame_src->ef_error);
				if (ITER_ISOK(frame_src->ef_trace))
					Dee_Incref(frame_src->ef_trace);
				if (!req_alloc) {
					frame_dst->ef_prev = NULL;
				} else {
					frame_dst = frame_dst->ef_prev;
				}
				frame_src = frame_src->ef_prev;
			}
			ASSERT(!frame_src);
			ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_STARTING);
			/* Append the list of new exception to those of the calling thread. */
			me                 = DeeThread_Self();
			frame_dst->ef_prev = me->t_except; /* Inherit */
			me->t_except       = error_frames; /* Inherit */
			me->t_exceptsz += current_alloc;
			goto err;
		}
		result = me->t_threadres;
		if (!result)
			result = Dee_None;
		Dee_Incref(result);
		ATOMIC_FETCHAND(me->t_state, ~THREAD_STATE_STARTING);
		/* Save the thread-result in the caller-provided field. */
		*pthread_result = result; /* Inherit reference. */
	}
	return 0;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_GetTid(/*Thread*/ DeeObject *__restrict self,
                 dthreadid_t *__restrict pthreadid) {
	ASSERT(pthreadid);
	ASSERT_OBJECT_TYPE(self, &DeeThread_Type);
#if defined(CONFIG_NO_THREADID_INTERNAL) || defined(CONFIG_NO_THREADID)
	return DeeError_Throwf(&DeeError_SystemError,
	                       "Cannot determine id of thread %k",
	                       self);
#else
	if unlikely(!(((DeeThreadObject *)self)->t_state & THREAD_STATE_STARTED)) {
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Thread %k has no id because it wasn't started",
		                       self);
	}
	*pthreadid = ((DeeThreadObject *)self)->t_threadid;
	return 0;
#endif
}

/* Clear all TLS variables assigned to slots in the calling thread.
 * @return: true:  The TLS descriptor table has been finalized.
 * @return: false: No TLS descriptor table had been assigned. */
PUBLIC bool (DCALL DeeThread_ClearTls)(void) {
	DeeThreadObject *caller;
	bool result = false;
	DBG_ALIGNMENT_DISABLE();
	caller = (DeeThreadObject *)thread_tls_get();
	DBG_ALIGNMENT_ENABLE();
	if likely(caller) {
		void *data = caller->t_tlsdata;
		if likely(data) {
			caller->t_tlsdata = NULL;
			fini_tls_data(data);
			result = true;
		}
	}
	return result;
}


PRIVATE NONNULL((1)) bool DCALL
thread_doclear(DeeThreadObject *__restrict self) {
	bool result = false;
	void *tls_data;
	struct thread_interrupt old_intr;
	DREF DeeObject *old_objects[3];
	struct except_frame *old_except;
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_INTERRUPTING) &
	       THREAD_STATE_INTERRUPTING)
		SCHED_YIELD();
	memcpy(&old_intr, &self->t_interrupt, sizeof(struct thread_interrupt));
	self->t_interrupt.ti_next = NULL;
	self->t_interrupt.ti_intr = NULL;
	self->t_interrupt.ti_args = NULL;
	ATOMIC_FETCHAND(self->t_state, ~(THREAD_STATE_INTERRUPTING |
	                                 THREAD_STATE_INTERRUPTED));
	/* Decref() all pending interrupts. */
	if (old_intr.ti_intr)
		for (;;) {
			struct thread_interrupt *next;
			ASSERT(old_intr.ti_intr);
			next = old_intr.ti_next;
			Dee_XDecref(old_intr.ti_args);
			Dee_Decref(old_intr.ti_intr);
			result = true;
			if (!next)
				break;
			memcpy(&old_intr, next, sizeof(struct thread_interrupt));
			Dee_Free(next);
		}

	Dee_Incref(Dee_EmptyTuple);
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING) &
	       THREAD_STATE_STARTING)
		SCHED_YIELD();
	old_objects[0]     = (DREF DeeObject *)self->t_threadmain;
	old_objects[1]     = (DREF DeeObject *)self->t_threadargs;
	old_objects[2]     = (DREF DeeObject *)self->t_threadres;
	tls_data           = self->t_tlsdata;
	self->t_tlsdata    = NULL;
	old_except         = self->t_except;
	self->t_except     = NULL;
	self->t_exceptsz   = 0;
	self->t_threadmain = NULL;
	self->t_threadargs = (DREF DeeTupleObject *)Dee_EmptyTuple;
	self->t_threadres  = NULL;
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	if (tls_data)
		fini_tls_data(tls_data);
	while (old_except) {
		struct except_frame *next = old_except->ef_prev;
		Dee_Decref(old_except->ef_error);
		if (ITER_ISOK(old_except->ef_trace))
			Dee_Decref(old_except->ef_trace);
		except_frame_free(old_except);
		old_except = next;
		result     = true;
	}
	if (old_objects[2]) {
		Dee_Decref(old_objects[2]);
		result = true;
	}
	if (old_objects[1]) {
		Dee_Decref(old_objects[1]);
		result |= old_objects[1] != Dee_EmptyTuple;
	}
	if (old_objects[0]) {
		Dee_Decref(old_objects[0]);
		result = true;
	}
	return result;
}

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
/* Any platform that passes boolean return values through a
 * general-purpose register is applicable to this optimization. */
#define thread_clear thread_doclear
#else
PRIVATE NONNULL((1)) void DCALL
thread_clear(DeeThreadObject *__restrict self) {
	thread_doclear(self);
}
#endif

PRIVATE NONNULL((1, 2)) void DCALL
thread_visit(DeeThreadObject *__restrict self, dvisit_t proc, void *arg) {
	struct thread_interrupt *iter;
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_INTERRUPTING) &
	       THREAD_STATE_INTERRUPTING)
		SCHED_YIELD();
	/* Visit pending interrupts. */
	if (self->t_interrupt.ti_intr) {
		iter = &self->t_interrupt;
		do {
			Dee_Visit(iter->ti_intr);
			Dee_XVisit(iter->ti_args);
		} while ((iter = iter->ti_next) != NULL);
	}
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_INTERRUPTING);
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING) &
	       THREAD_STATE_STARTING)
		SCHED_YIELD();
	Dee_XVisit(self->t_threadmain);
	Dee_Visit(self->t_threadargs);
	Dee_XVisit(self->t_threadres);
	if (self->t_state & THREAD_STATE_TERMINATED) {
		struct except_frame *iter;
		/* Once terminated, the thread object inherits
		 * ownership of exceptions and TLS data.
		 * Without this addition, we'd get an unresolvable reference loop
		 * when a thread crashed by throwing an object from which the thread
		 * is reachable:
		 * >> local x = thread([]{
		 * >>     throw thread.self();
		 * >> });
		 * >> x.start();
		 * >> x.join();
		 * After code like that, the thread itself remains reachable
		 * from the thread controller, causing `join()' to fail with
		 * a ThreadCrash error.
		 * However during GC cleanup of the thread, we need to make
		 * the connection that the thread then holds a reference to
		 * itself.
		 * However we can't do this before the thread has officially
		 * terminated, as before then, this kind of data is private
		 * to the thread itself, and furthermore describes gc-root
		 * data that must not be cleaned (because it wouldn't just
		 * be illegal for the GC to cleanup exceptions that haven't
		 * even been handled, yet, but it wouldn't make sense either)
		 */
		for (iter = self->t_except; iter;
		     iter = iter->ef_prev) {
			Dee_Visit(iter->ef_error);
			if (ITER_ISOK(iter->ef_trace))
				Dee_Visit(iter->ef_trace);
		}
		/* Same deal with TLS data as with exceptions: Only
		 * visit them if the thread has already terminated. */
		if (self->t_tlsdata)
			visit_tls_data(self->t_tlsdata, proc, arg);
	}
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
}

PRIVATE NONNULL((1)) void DCALL
thread_fini(DeeThreadObject *__restrict self) {
	struct except_frame *frame;
	ASSERT(!self->t_exec);
	ASSERT(!self->t_execsz);
	ASSERT(!self->t_str_curr);
	ASSERT(!self->t_repr_curr);
	DBG_ALIGNMENT_DISABLE();
	ASSERT(!(self->t_state & THREAD_STATE_STARTED) ||
	       (self->t_state & THREAD_STATE_TERMINATED) ||
	       (self->t_state & THREAD_STATE_EXTERNAL) ||
	       self == (DeeThreadObject *)thread_tls_get());
	DBG_ALIGNMENT_ENABLE();
	ASSERT(!self->t_globlpself);

	if (self->t_state & (THREAD_STATE_STARTED | THREAD_STATE_TERMINATED)) {
#ifdef CONFIG_THREADS_WINDOWS
		/* Close the system thread-handle.
		 * NOTE: On windows, detaching a thread doesn't actually mean
		 *       anything other than to cause an error when attempts
		 *       are made to join the thread never-the-less. */
		CloseHandle(self->t_thread);
#endif /* CONFIG_THREADS_WINDOWS */
#ifdef CONFIG_THREADS_PTHREAD
		/* detach the thread descriptor. */
		if (!(self->t_state & THREAD_STATE_EXTERNAL)) {
			DBG_ALIGNMENT_DISABLE();
			pthread_detach(self->t_thread);
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* CONFIG_THREADS_PTHREAD */
#ifdef CONFIG_THREADS_JOIN_SEMPAHORE
		if (!(self->t_state & THREAD_STATE_EXTERNAL)) {
			/* Destroy the join-semaphore. */
#ifdef CONFIG_HOST_WINDOWS
			DBG_ALIGNMENT_DISABLE();
			CloseHandle(self->t_join);
			DBG_ALIGNMENT_ENABLE();
#elif !defined(CONFIG_NO_SEMAPHORE_H)
			DBG_ALIGNMENT_DISABLE();
			sem_destroy(&self->t_join);
			DBG_ALIGNMENT_ENABLE();
#endif
		}
#endif /* CONFIG_THREADS_JOIN_SEMPAHORE */
	}
	/* Finalize TLS objects. */
	frame            = self->t_except;
	self->t_except   = NULL;
	self->t_exceptsz = 0;
	while (frame) {
		struct except_frame *next;
		next = frame->ef_prev;
		if (!(self->t_state & THREAD_STATE_DIDJOIN)) {
			DeeTracebackObject *trace = frame->ef_trace;
			if (trace == (DeeTracebackObject *)ITER_DONE)
				trace = NULL;
			/* Display thread errors if they hadn't been propagated during a join(). */
			DeeError_Display("Unhandled thread error\n",
			                 (DeeObject *)frame->ef_error,
			                 (DeeObject *)trace);
		}
		Dee_Decref(frame->ef_error);
		if (ITER_ISOK(frame->ef_trace))
			Dee_Decref(frame->ef_trace);
		except_frame_free(frame);
		frame = next;
	}
	ASSERT(!self->t_except);
	ASSERT(!self->t_exceptsz);
	ASSERT(!self->t_deepassoc.da_used);
	ASSERT((self->t_deepassoc.da_mask != 0) ==
	       (self->t_deepassoc.da_list != empty_deep_assoc));
	if (self->t_deepassoc.da_list != empty_deep_assoc)
		Dee_Free(self->t_deepassoc.da_list);
	Dee_XDecref(self->t_threadname);
	Dee_XDecref(self->t_threadmain);
	Dee_Decref(self->t_threadargs);
	Dee_XDecref(self->t_threadres);
	if (self->t_tlsdata)
		fini_tls_data(self->t_tlsdata);
	while (self->t_interrupt.ti_intr) {
		struct thread_interrupt *next;
		Dee_Decref(self->t_interrupt.ti_intr);
		Dee_XDecref(self->t_interrupt.ti_args);
		next = self->t_interrupt.ti_next;
		if (!next)
			break;
		memcpy(&self->t_interrupt, next, sizeof(struct thread_interrupt));
		Dee_Free(next);
	}
}


#ifndef CONFIG_NO_THREADS
#ifndef CONFIG_NO_THREADID
PUBLIC WUNUSED DREF DeeObject *(DCALL DeeThread_NewExternal)(dthread_t thread, dthreadid_t id)
#else /* !CONFIG_NO_THREADID */
PUBLIC WUNUSED DREF DeeObject *(DCALL DeeThread_NewExternal)(dthread_t thread)
#endif /* CONFIG_NO_THREADID */
{
	DREF DeeThreadObject *result;
	result = DeeGCObject_CALLOC(DeeThreadObject);
	if unlikely(!result)
		goto done;
	result->t_state = (THREAD_STATE_STARTED | THREAD_STATE_EXTERNAL);
#ifndef CONFIG_NO_THREADID
	result->t_threadid = id;
#endif /* !CONFIG_NO_THREADID */
	result->t_thread            = thread;
	result->t_threadargs        = (DeeTupleObject *)Dee_EmptyTuple;
	result->t_deepassoc.da_list = empty_deep_assoc;
	Dee_Incref(Dee_EmptyTuple);
	DeeObject_Init(result, &DeeThread_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
}
#endif /* !CONFIG_NO_THREADS */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_str(DeeThreadObject *__restrict self) {
	if (self->t_threadname)
		return_reference_((DeeObject *)self->t_threadname);
	return_reference_(&str_Thread);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_repr(DeeThreadObject *__restrict self) {
	DREF DeeObject *threadmain;
	DREF DeeObject *threadargs;
	DREF DeeObject *result;
	uint16_t state;
	struct unicode_printer printer;
	while ((state = ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING),
	        state & THREAD_STATE_STARTING))
		SCHED_YIELD();
	threadmain = (DREF DeeObject *)self->t_threadmain;
	threadargs = (DREF DeeObject *)self->t_threadargs;
	Dee_XIncref(threadmain);
	Dee_Incref(threadargs);
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	unicode_printer_init(&printer);
	if unlikely(UNICODE_PRINTER_PRINT(&printer, "thread(") < 0)
		goto err;
	if (self->t_threadname &&
	    unlikely(unicode_printer_printf(&printer, "%r, ", self->t_threadname) < 0))
		goto err;
	if (threadmain &&
	    unlikely(unicode_printer_printf(&printer, "%r, ", threadmain) < 0))
		goto err;
	if unlikely(unicode_printer_printobjectrepr(&printer, threadargs) < 0)
		goto err;
	if unlikely(UNICODE_PRINTER_PRINT(&printer, ")") < 0)
		goto err;
	result = unicode_printer_pack(&printer);
done:
	Dee_XDecref(threadargs);
	Dee_XDecref(threadmain);
	return result;
err:
	unicode_printer_fini(&printer);
	result = NULL;
	goto done;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
thread_ctor(DeeThreadObject *__restrict self,
            size_t argc, DeeObject **argv) {
	if unlikely(argc > 3) {
		err_invalid_argc(DeeString_STR(&str_Thread), argc, 0, 3);
		goto err;
	}
	self->t_threadname = NULL;
	if (argc && DeeString_Check(argv[0])) {
		self->t_threadname = (DREF DeeStringObject *)argv[0];
		Dee_Incref(self->t_threadname);
		--argc, ++argv;
	}
	self->t_threadmain = NULL;
	if (argc) {
		/* The thread callback object. */
		self->t_threadmain = argv[0];
		Dee_Incref(self->t_threadmain);
		--argc, ++argv;
	}
	if (argc) {
		self->t_threadargs = (DREF struct tuple_object *)argv[0];
		/* Allow `none' as an alias for an empty tuple. */
		if (DeeNone_Check(self->t_threadargs))
			self->t_threadargs = (DREF struct tuple_object *)Dee_EmptyTuple;
		else {
			/* Make sure that the callback arguments are a tuple. */
			if (DeeObject_AssertTypeExact((DeeObject *)self->t_threadargs,
			                              &DeeTuple_Type))
				goto err_main;
		}
	} else {
		/* When no arguments have been given, use an empty tuple. */
		self->t_threadargs = (DREF struct tuple_object *)Dee_EmptyTuple;
	}
	Dee_Incref(self->t_threadargs);
	self->t_exec              = NULL;
	self->t_except            = NULL;
	self->t_exceptsz          = 0;
	self->t_execsz            = 0;
	self->t_str_curr          = NULL;
	self->t_repr_curr         = NULL;
	self->t_state             = THREAD_STATE_INITIAL;
	self->t_interrupt.ti_next = NULL;
	self->t_interrupt.ti_intr = NULL;
	self->t_interrupt.ti_args = NULL;
	self->t_threadres         = NULL;
	self->t_globlpself        = NULL;
	self->t_globalnext        = NULL;
	self->t_suspended         = 0;
	self->t_deepassoc.da_used = 0;
	self->t_deepassoc.da_mask = 0;
	self->t_deepassoc.da_list = empty_deep_assoc;
	self->t_tlsdata           = NULL;
	return 0;
err_main:
	Dee_XDecref(self->t_threadmain);
	Dee_XDecref(self->t_threadname);
err:
	return -1;
}

#else /* !CONFIG_NO_THREADS */

PUBLIC void DCALL DeeThread_Fini(void) {
	ASSERT(!DeeThread_Main.t_deepassoc.da_used);
	if (DeeThread_Main.t_deepassoc.da_list != empty_deep_assoc) {
		Dee_Free(DeeThread_Main.t_deepassoc.da_list);
		DeeThread_Main.t_deepassoc.da_list = empty_deep_assoc;
		DeeThread_Main.t_deepassoc.da_mask = 0;
	}
}

PRIVATE ATTR_COLD int DCALL err_no_thread_api(void) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Threading support has been disabled");
}

PUBLIC int DCALL
DeeThread_Start(/*Thread*/ DeeObject *__restrict UNUSED(self)) {
	return err_no_thread_api();
}

PUBLIC int DCALL
DeeThread_Interrupt(/*Thread*/ DeeObject *__restrict UNUSED(self),
                    DeeObject *__restrict UNUSED(interrupt_ob)) {
	return err_no_thread_api();
}

PUBLIC int DCALL
DeeThread_Detach(/*Thread*/ DeeObject *__restrict UNUSED(self)) {
	return err_no_thread_api();
}

PUBLIC int DCALL
DeeThread_Join(/*Thread*/ DeeObject *__restrict UNUSED(self),
               DeeObject **__restrict UNUSED(pthread_result),
               uint64_t UNUSED(timeout_microseconds)) {
	return err_no_thread_api();
}
#endif /* CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_start(DeeObject *self, size_t argc, DeeObject **argv) {
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
thread_detach(DeeObject *self, size_t argc, DeeObject **argv) {
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
thread_join(DeeObject *self, size_t argc, DeeObject **argv) {
	int error;
	DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":join"))
		goto err;
	error = DeeThread_Join(self, &result, (uint64_t)-1);
	if unlikely(error < 0)
		goto err;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_tryjoin(DeeObject *self, size_t argc, DeeObject **argv) {
	int error;
	DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":tryjoin"))
		goto err;
	error = DeeThread_Join(self, &result, 0);
	if unlikely(error < 0)
		goto err;
	if (error == 0)
		return Dee_Packf("(bO)", 1, result);
	return Dee_Packf("(bo)", 0, Dee_None);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_timedjoin(DeeObject *self, size_t argc, DeeObject **argv) {
	int error;
	DeeObject *result;
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64d:timedjoin", &timeout))
		goto err;
	error = DeeThread_Join(self, &result, timeout);
	if unlikely(error < 0)
		goto err;
	if (error == 0)
		return Dee_Packf("(bO)", 1, result);
	return Dee_Packf("(bo)", 0, Dee_None);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_interrupt(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *sig  = &DeeError_Interrupt_instance;
	DeeObject *args = NULL;
	int error;
	if (DeeArg_Unpack(argc, argv, "|oo:interrupt", &sig, &args) ||
	    (args && DeeObject_AssertTypeExact(args, &DeeTuple_Type)))
		goto err;
	error = DeeThread_Interrupt(self, sig, args);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_started(DeeObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":started"))
		return NULL;
	return_bool(DeeThread_HasStarted(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_detached(DeeObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":detached"))
		return NULL;
	return_bool(DeeThread_WasDetached(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_terminated(DeeObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":terminated"))
		return NULL;
	return_bool(DeeThread_HasTerminated(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_interrupted(DeeObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":interrupted"))
		return NULL;
	return_bool(DeeThread_WasInterrupted(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crashed(DeeObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":crashed"))
		return NULL;
	return_bool(DeeThread_HasCrashed(self));
}

#ifndef CONFIG_NO_THREADS
PRIVATE NONNULL((1)) void DCALL
err_not_terminated(DeeThreadObject *__restrict self) {
	DeeError_Throwf(&DeeError_ValueError,
	                "Thread %k has not terminated yet",
	                self);
}
#endif /* !CONFIG_NO_THREADS */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crash_error(DeeThreadObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":crash_error"))
		goto err;
	{
#ifdef CONFIG_NO_THREADS
		(void)self;
		err_no_thread_api();
#else /* CONFIG_NO_THREADS */
		uint16_t state;
		DREF DeeObject *result;
		do {
			state = ATOMIC_READ(self->t_state);
			if (!(state & THREAD_STATE_TERMINATED)) {
				err_not_terminated(self);
				goto err;
			}
		} while (ATOMIC_CMPXCH_WEAK(self->t_state, state, state | THREAD_STATE_STARTING));
		ASSERT((self->t_exceptsz != 0) == (self->t_except != NULL));
		if (self->t_exceptsz == 0) {
			/* No active exceptions. */
			ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
			err_no_active_exception();
			goto err;
		}
		result = self->t_except->ef_error;
		ASSERT_OBJECT(result);
		Dee_Incref(result);
		ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
		return result;
#endif /* !CONFIG_NO_THREADS */
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crash_traceback(DeeThreadObject *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":crash_traceback"))
		goto err;
	{
#ifdef CONFIG_NO_THREADS
		(void)self;
		err_no_thread_api();
#else /* CONFIG_NO_THREADS */
		uint16_t state;
		DREF DeeObject *result;
		do {
			state = ATOMIC_READ(self->t_state);
			if (!(state & THREAD_STATE_TERMINATED)) {
				err_not_terminated(self);
				goto err;
			}
		} while (ATOMIC_CMPXCH_WEAK(self->t_state, state, state | THREAD_STATE_STARTING));
		ASSERT((self->t_exceptsz != 0) == (self->t_except != NULL));
		if (self->t_exceptsz == 0) {
			/* No active exceptions. */
			ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
			err_no_active_exception();
			goto err;
		}
		result = (DeeObject *)self->t_except->ef_trace;
		if (result == ITER_DONE && self == DeeThread_Self())
			result = (DeeObject *)except_frame_gettb(self->t_except);
		if (!result)
			result = Dee_None;
		ASSERT_OBJECT(result);
		Dee_Incref(result);
		ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
		return result;
#endif /* !CONFIG_NO_THREADS */
	}
err:
	return NULL;
}



PRIVATE struct type_method thread_methods[] = {
	{ "start", &thread_start,
	  DOC("->?Dbool\n"
	      "@throw SystemError Failed to start @this thread for some reason\n"
	      "@return true: The :thread is now running\n"
	      "@return false: The :thread had already been started\n"
	      "Starts @this thread") },
	{ "detach", &thread_detach,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this thread was never started\n"
	      "@throw SystemError Failed to detach @this thread for some reason\n"
	      "@return true: The :thread has been detached\n"
	      "@return false: The :thread was already detached\n"
	      "Detaches @this thread") },
	{ "join", &thread_join,
	  DOC("->\n"
	      "@interrupt\n"
	      "@throw ValueError @this thread was never started\n"
	      "@throw SystemError Failed to join @this thread for some reason\n"
	      "@throw ThreadCrash The error(s) that caused @this thread to crash, encapsulated in a :ThreadCrash error\n"
	      "@return The return value of @this thead\n"
	      "Joins @this thread and returns the return value of its main function\n"
	      "In the event") },
	{ "tryjoin", &thread_tryjoin,
	  DOC("->?T2?Dbool?O\n"
	      "@throw ValueError @this thread was never started\n"
	      "@throw SystemError Failed to join @this thread for some reason\n"
	      "@throw ThreadCrash The error(s) that caused @this thread to crash, encapsulated in a :ThreadCrash error\n"
	      "Same as #join, but don't check for interrupts and fail immediately") },
	{ "timedjoin", &thread_timedjoin,
	  DOC("(timeout_in_microseconds:?Dint)->?T2?Dbool?O\n"
	      "@throw ValueError @this thread was never started\n"
	      "@throw SystemError Failed to join @this thread for some reason\n"
	      "@throw ThreadCrash The error(s) that caused @this thread to crash, encapsulated in a :ThreadCrash error\n"
	      "Same as #join, but only attempt to join for a given @timeout_in_microseconds") },
	{ "interrupt", &thread_interrupt,
	  DOC("->?Dbool\n"
	      "(signal)->?Dbool\n"
	      "(async_func:?DCallable,async_args:?DTuple)->?Dbool\n"
	      "@return true: The interrupt was delivered\n"
	      "@return false: The :thread has already terminated and can no longer process interrupts\n"
	      "Throws the given @signal or an instance of :Interrupt within @this thread, "
	      "or schedule the given @async_func to be called asynchronously using @async_args\n"
	      "Calling the function with no arguments is identical to:\n"
	      ">import Signal from deemon;\n"
	      ">this.interrupt(Signal.Interrupt());\n"
	      "Calling the function with a single arguments is identical to:\n"
	      ">this.interrupt(function(signal){\n"
	      ">\tthrow signal;\n"
	      ">},pack(signal));\n"
	      "Note that interrupts delivered by this function are processed at random points during "
	      "execution of the thread, with the only guaranty that is made being that they will always "
	      "be handled sooner or later, no matter what the associated thread may be doing, even if "
	      "what it is doing is executing an infinite loop ${for (;;) { }}\n"
	      "Though it should be noted that a thread that terminated due to an unhandled "
	      "exception may not get a chance to execute all remaining interrupts before stopping\n"
	      "Also note that remaining interrupts may still be executing once #terminated already "
	      "returns :true, as indicative of the thread no longer being able to receive new interrupts. "
	      "However to truely ensure that all interrupts have been processed, you must #join @this thread\n"
	      "User-code may also check for interrupts explicitly by calling `:thread.check_interrupt'") },
	{ "started", &thread_started,
	  DOC("->?Dbool\n"
	      "Deprecated alias for #hasstarted") },
	{ "detached", &thread_detached,
	  DOC("->?Dbool\n"
	      "Deprecated alias for #wasdetached") },
	{ "terminated", &thread_terminated,
	  DOC("->?Dbool\n"
	      "Deprecated alias for #hasterminated") },
	{ "interrupted", &thread_interrupted,
	  DOC("->?Dbool\n"
	      "Deprecated alias for #wasinterrupted") },
	{ "crashed", &thread_crashed,
	  DOC("->?Dbool\n"
	      "Deprecated alias for #hascrashed") },

	/* Old, deprecated function names for backwards compatibility */
	{ "try_join", &thread_tryjoin,
	  DOC("->?T2?Dbool?O\n"
	      "Old, deprecated name for #tryjoin") },
	{ "timed_join", &thread_timedjoin,
	  DOC("(timeout_in_microseconds:?Dint)->?T2?Dbool?O\n"
	      "Old, deprecated name for #timedjoin") },
	{ "crash_error", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&thread_crash_error,
	  DOC("->?DTraceback\n"
	      "->?N\n"
	      "Deprecated function that does the same as ${this.crashinfo.first()[0]}") },
	{ "crash_traceback", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&thread_crash_traceback,
	  DOC("->?DTraceback\n"
	      "->?N\n"
	      "Deprecated function that does the same as ${this.crashinfo.first()[1]}") },
	{ NULL }
};

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_self(DeeObject *__restrict UNUSED(self),
            size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":self"))
		return NULL;
	return_reference(DeeThread_Self());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_current_get(DeeObject *__restrict UNUSED(self)) {
	return_reference(DeeThread_Self());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_selfid(DeeObject *__restrict UNUSED(self),
              size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":selfid"))
		return NULL;
#ifdef CONFIG_NO_THREADID_INTERNAL
	dthreadid_t result;
	if (DeeThread_GetTid(DeeThread_Self(), &result))
		return NULL;
	return DeeInt_Newu(SIZEOF_DTHREADID_T, result);
#else /* CONFIG_NO_THREADID_INTERNAL */
#ifdef NO_DBG_ALIGNMENT
	return DeeInt_Newu(SIZEOF_DTHREADID_T, os_gettid());
#else /* NO_DBG_ALIGNMENT */
	{
		dthreadid_t result;
		DBG_ALIGNMENT_DISABLE();
		result = os_gettid();
		DBG_ALIGNMENT_ENABLE();
		return DeeInt_Newu(SIZEOF_DTHREADID_T, result);
	}
#endif /* !NO_DBG_ALIGNMENT */
#endif /* !CONFIG_NO_THREADID_INTERNAL */
}

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_yield(DeeObject *__restrict UNUSED(self),
             size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":yield"))
		return NULL;
	SCHED_YIELD();
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_check_interrupt(DeeObject *__restrict UNUSED(self),
                       size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":check_interrupt") ||
	    DeeThread_CheckInterrupt())
		return NULL;
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_sleep(DeeObject *__restrict UNUSED(self),
             size_t argc, DeeObject **argv) {
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64u:sleep", &timeout) ||
	    DeeThread_Sleep(timeout))
		return NULL;
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
thread_exit(DeeObject *__restrict UNUSED(self),
            size_t argc, DeeObject **argv) {
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

PRIVATE struct type_getset thread_class_getsets[] = {
	{ "current",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_current_get, NULL, NULL,
	  DOC("->?.\n"
	      "Returns a thread descriptor for the calling thread") },
	/* TODO: enumerate -> {thread...} 
	 * >> Returns a proxy sequence for enumerating all
	 *    deemon-threads; s.a. `add_running_thread()' */
	{ NULL }
};

PRIVATE struct type_member thread_class_members[] = {
	TYPE_MEMBER_CONST_DOC("main", &DeeThread_Main, "The main (initial) thread"),
	TYPE_MEMBER_END
};

PRIVATE struct type_method thread_class_methods[] = {
	{ "self", &thread_self,
	  DOC("->?.\n"
	      "Deprecated alias for #current") },
	{ "selfid", &thread_selfid,
	  DOC("->?Dint\n"
	      "@throw SystemError The system does not provide a way to query thread ids\n"
	      "Deprecated alias for ${thread.current.id}") },
	{ "check_interrupt", &thread_check_interrupt,
	  DOC("()\n"
	      "@interrupt\n"
	      "Checks for interrupts in the calling thread") },
	{ DeeString_STR(&str_yield),
	  &thread_yield,
	  /* TODO: Must make this one deprecated, and add a new one with a different name!
	   *       `yield' is a reserved identifer, and `import thread from deemon; thread.yield();'
	   *       causes a compiler warning! */
	  DOC("()\n"
	      "Willingly preempt execution to another thread or process") },
	{ "sleep", &thread_sleep,
	  DOC("(timeout_in_microseconds:?Dint)\n"
	      "@interrupt\n"
	      "Suspending execution for a total of @timeout_in_microseconds") },
	{ "exit", &thread_exit,
	  DOC("(result=!N)\n"
	      "@throw ThreadExit Always thrown to exit the current thread\n"
	      "Throw a :ThreadExit error object in order to terminate execution "
	      "within the current thread. This function does not return normally") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_callback_get(DeeThreadObject *__restrict self) {
#ifdef CONFIG_NO_THREADS
	(void)self;
	err_no_thread_api();
	return NULL;
#else /* CONFIG_NO_THREADS */
	DREF DeeObject *result;
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING) &
	       THREAD_STATE_STARTING)
		SCHED_YIELD();
	result = self->t_threadmain;
	Dee_XIncref(result);
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	/* If the callback isn't set, lookup the `run' member function. */
	if (!result)
		result = DeeObject_GetAttr((DeeObject *)self, &str_run);
	return result;
#endif /* !CONFIG_NO_THREADS */
}

#ifndef CONFIG_NO_THREADS
PRIVATE ATTR_COLD int DCALL
err_cannot_set_thread_subclass_callback(DeeThreadObject *__restrict self,
                                        char const *__restrict attr_name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot set the %s of %k being a subclass %k of thread",
	                       attr_name, self, Dee_TYPE(self));
}
#endif /* !CONFIG_NO_THREADS */

PRIVATE int DCALL
thread_callback_set(DeeThreadObject *__restrict self,
                    DeeObject *value) {
#ifdef CONFIG_NO_THREADS
	(void)self;
	(void)value;
	return err_no_thread_api();
#else /* CONFIG_NO_THREADS */
	DREF DeeObject *old_callback;
	uint16_t state;
	if (!DeeThread_CheckExact(self))
		return err_cannot_set_thread_subclass_callback(self, "callback");
restart:
	do {
		state = ATOMIC_READ(self->t_state);
		if (state & (THREAD_STATE_STARTED | THREAD_STATE_TERMINATED)) {
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Cannot set the callback of thread %k that has already been started",
			                       self);
		}
		if (state & THREAD_STATE_STARTING) {
			SCHED_YIELD();
			goto restart;
		}
	} while (!ATOMIC_CMPXCH_WEAK(self->t_state, state, state | THREAD_STATE_STARTING));
	old_callback       = self->t_threadmain;
	self->t_threadmain = value;
	Dee_XIncref(value);
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	Dee_XDecref(old_callback);
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
thread_callback_del(DeeThreadObject *__restrict self) {
	return thread_callback_set(self, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_callargs_get(DeeThreadObject *__restrict self) {
#ifdef CONFIG_NO_THREADS
	(void)self;
	return err_no_thread_api();
#else /* CONFIG_NO_THREADS */
	DREF DeeObject *result;
	while (ATOMIC_FETCHOR(self->t_state, THREAD_STATE_STARTING) &
	       THREAD_STATE_STARTING)
		SCHED_YIELD();
	result = (DREF DeeObject *)self->t_threadargs;
	Dee_Incref(result);
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	return result;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
thread_callargs_set(DeeThreadObject *__restrict self,
                    DeeObject *__restrict value) {
#ifdef CONFIG_NO_THREADS
	(void)self;
	(void)value;
	return err_no_thread_api();
#else /* CONFIG_NO_THREADS */
	DREF DeeObject *old_callargs;
	uint16_t state;
	/* Allow `none' to be used in place to an empty tuple. */
	if (DeeNone_Check(value))
		value = Dee_EmptyTuple;
	/* Make sure the new value is actually a tuple. */
	if (DeeObject_AssertTypeExact(value, &DeeTuple_Type))
		return -1;
	/* Don't allow this to be overwritten for sub-classes of `thread' */
	if (!DeeThread_CheckExact(self))
		return err_cannot_set_thread_subclass_callback(self, "callargs");
restart:
	do {
		state = ATOMIC_READ(self->t_state);
		if (state & (THREAD_STATE_STARTED | THREAD_STATE_TERMINATED)) {
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Cannot set the callback arguments of thread %k that has already been started",
			                       self);
		}
		if (state & THREAD_STATE_STARTING) {
			SCHED_YIELD();
			goto restart;
		}
	} while (!ATOMIC_CMPXCH_WEAK(self->t_state, state, state | THREAD_STATE_STARTING));
	/* Override the threadargs member. */
	old_callargs       = (DREF DeeObject *)self->t_threadargs;
	self->t_threadargs = (DREF DeeTupleObject *)value;
	Dee_Incref(value);
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	Dee_Decref(old_callargs);
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
thread_callargs_del(DeeThreadObject *__restrict self) {
	return thread_callargs_set(self, Dee_EmptyTuple);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_result_get(DeeThreadObject *__restrict self) {
#ifdef CONFIG_NO_THREADS
	err_no_thread_api();
	return NULL;
#else /* CONFIG_NO_THREADS */
	uint16_t state;
	DREF DeeObject *result;
restart:
	do {
		state = ATOMIC_READ(self->t_state);
		if (!(state & THREAD_STATE_TERMINATED)) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot read the return value of a thread %k that hasn't terminated",
			                self);
			return NULL;
		}
		if (state & THREAD_STATE_STARTING) {
			SCHED_YIELD();
			goto restart;
		}
	} while (!ATOMIC_CMPXCH_WEAK(self->t_state, state, state | THREAD_STATE_STARTING));
	result = self->t_threadres;
	if (!result)
		result = Dee_None;
	Dee_XIncref(result);
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	return result;
#endif /* !CONFIG_NO_THREADS */
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_id(DeeObject *__restrict self) {
	dthreadid_t result;
	if (DeeThread_GetTid(self, &result))
		return NULL;
	return DeeInt_Newu(SIZEOF_DTHREADID_T, result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_isrunning(DeeObject *__restrict self) {
	return_bool(DeeThread_IsRunning(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_hasstarted(DeeObject *__restrict self) {
	return_bool(DeeThread_HasStarted(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_wasdetached(DeeObject *__restrict self) {
	return_bool(DeeThread_WasDetached(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_hasterminated(DeeObject *__restrict self) {
	return_bool(DeeThread_HasTerminated(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_wasinterrupted(DeeObject *__restrict self) {
	return_bool(DeeThread_WasInterrupted(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_hascrashed(DeeObject *__restrict self) {
	return_bool(DeeThread_HasCrashed(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
thread_crashinfo(DeeThreadObject *__restrict self) {
#ifdef CONFIG_NO_THREADS
	(void)self;
	err_no_thread_api();
#else /* CONFIG_NO_THREADS */
	uint16_t state, i, count;
	DREF DeeTupleObject *result;
restart:
	do {
		state = ATOMIC_READ(self->t_state);
		if (!(state & THREAD_STATE_TERMINATED)) {
			err_not_terminated(self);
			goto err;
		}
	} while (ATOMIC_CMPXCH_WEAK(self->t_state, state, state | THREAD_STATE_STARTING));
	ASSERT((self->t_exceptsz != 0) == (self->t_except != NULL));
	if (self->t_exceptsz == 0) {
		/* No active exceptions. */
		result = (DREF DeeTupleObject *)Dee_EmptyTuple;
		Dee_Incref(result);
	} else {
		struct except_frame *frame_iter;
		count          = self->t_exceptsz;
		size_t reqsize = (offsetof(DeeTupleObject, t_elem) +
		                  count * sizeof(DREF DeeObject *));
		result         = (DREF DeeTupleObject *)DeeObject_TryMalloc(reqsize);
		if unlikely(!result) {
			ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
			if (Dee_CollectMemory(reqsize))
				goto restart;
			goto err;
		}
		result->t_size = (size_t)reqsize;
		frame_iter     = self->t_except;
		for (i = 0; i < count; ++i) {
			DREF DeeTupleObject *item;
			ASSERT(frame_iter);
			/* Create the element tuple. */
			item = (DREF DeeTupleObject *)DeeObject_TryMalloc(offsetof(DeeTupleObject, t_elem) +
			                                                  2 * sizeof(DREF DeeObject *));
			if unlikely(!item)
				goto err_start_over;
			DeeObject_Init(item, &DeeTuple_Type);
			/* Fill in the item with information from the frame. */
			item->t_size    = 2;
			item->t_elem[0] = (DREF DeeObject *)frame_iter->ef_error;
			item->t_elem[1] = (DREF DeeObject *)frame_iter->ef_trace;
			if (!item->t_elem[1] && self == DeeThread_Self())
				item->t_elem[1] = (DeeObject *)except_frame_gettb(frame_iter);
			if (!ITER_ISOK(item->t_elem[1]))
				item->t_elem[1] = Dee_None;
			Dee_Incref(item->t_elem[0]);
			Dee_Incref(item->t_elem[1]);
			result->t_elem[i] = (DREF DeeObject *)item; /* Inherit */
			frame_iter        = frame_iter->ef_prev;
		}
		DeeObject_Init(result, &DeeTuple_Type);
	}
	ATOMIC_FETCHAND(self->t_state, ~THREAD_STATE_STARTING);
	return (DREF DeeObject *)result;
err_start_over:
	{
		bool can_start_over;
		can_start_over = Dee_CollectMemory(offsetof(DeeTupleObject, t_elem) +
		                                   2 * sizeof(DREF DeeObject *));
		while (i--)
			Dee_DecrefDokill(result->t_elem[i]);
		DeeObject_Free(result);
		if (can_start_over)
			goto restart;
	}
err:
#endif /* !CONFIG_NO_THREADS */
	return NULL;
}



PRIVATE struct type_getset thread_getsets[] = {
	{ "callback",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_callback_get,
	  (int (DCALL *)(DeeObject *__restrict))&thread_callback_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&thread_callback_set,
	  DOC("->?DCallable\n"
	      "@throw AttributeError Attempted to overwrite the callback of a sub-class of :thread, rather than an exact instance. "
	      "To prevent the need of overwriting this attribute whenever a sub-class wishes to provide a $run "
	      "member function, write-access to this field is denied in sub-classes of :thread and only granted "
	      "to exact instances\n"
	      "@throw ValueError Attempted to delete or set the attribute when @this thread has already been started.\n"
	      "The callback that will be executed when the thread is started\n"
	      "In the event that no callback has been set, or that the callback has been deleted, "
	      "the getter will attempt to return the instance attribute $run which can be "
	      "overwritten by sub-classes to provide an automatic and implicit thread-callback") },
	{ "callargs",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_callargs_get,
	  (int (DCALL *)(DeeObject *__restrict))&thread_callargs_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&thread_callargs_set,
	  DOC("->?DTuple\n"
	      "@throw AttributeError Attempted to overwrite the callback arguments of a sub-class of :thread, rather than an exact instance. "
	      "To prevent the need of overwriting this attribute whenever a sub-class wishes to provide a $run "
	      "member function, write-access to this field is denied in sub-classes of :thread and only granted "
	      "to exact instances\n"
	      "@throw ValueError Attempted to delete or set the attribute when @this thread has already been started\n"
	      "The callback arguments that are passed to #callback when the thread is started\n"
	      "Deleting this member or setting :none is the same as setting an empty tuple") },
	{ "result",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_result_get, NULL, NULL,
	  DOC("@throw ValueError @this thread has not terminated yet\n"
	      "Return the result value of @this thread once it has terminated\n"
	      "This is similar to what is returned by #join, but in the event that "
	      "the thread terminated because it crashed, :none is returned rather "
	      "than all the errors that caused the thread to crash being encapsulated and propagated") },
	{ "crashinfo",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_crashinfo, NULL, NULL,
	  DOC("->?S?T2?O?DTraceback\n"
	      "@throw ValueEror @this thread hasn't terminated yet\n"
	      "Returns a sequence of 2-element tuples describing the errors that were "
	      "active when the thread crashed (s.a. #hascrashed), or an empty sequence when "
	      "the thread didn't crash\n"
	      "The first element of each tuple is the error that was ${throw}n, and the "
	      "second element is the accompanying traceback, or :none when not known.\n"
	      "This function replaces the deprecated #crash_error and #crash_traceback "
	      "functions that did something similar prior to deemon 200\n"
	      "When iterated, elements of the returned sequence identify errors that "
	      "caused the crash from most to least recently thrown") },
	{ "traceback",
	  &DeeThread_Trace, NULL, NULL,
	  DOC("->?DTraceback\n"
	      "Generate a traceback for the thread's current execution position") },
	{ "id",
	  &thread_id, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError The thread hasn't been started yet\n"
	      "@throw SystemError The system does not provide a way to query thread ids\n"
	      "Returns an operating-system specific id of @this thread") },
	{ "isrunning",
	  &thread_isrunning, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this thread is current running (i.e. #wasstarted, but hasn't #hasterminated)") },
	{ "hasstarted",
	  &thread_hasstarted, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this thread has been started") },
	{ "wasdetached",
	  &thread_wasdetached, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this thread has been detached") },
	{ "hasterminated",
	  &thread_hasterminated, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this thread has terminated") },
	{ "wasinterrupted",
	  &thread_wasinterrupted, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if interrupts are pending for @this thread") },
	{ "hascrashed",
	  &thread_hascrashed, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this thread has crashed, that "
	      "is having #hasterminated while errors were still active\n"
	      "When :true, attempting to #join @this thread will cause all of the "
	      "errors to be rethrown in the calling thread as a :ThreadCrash error") },
	{ NULL }
};

PRIVATE struct type_member thread_members[] = {
#ifndef CONFIG_NO_THREADS
	TYPE_MEMBER_FIELD_DOC("name",
	                      STRUCT_OBJECT_OPT,
	                      offsetof(DeeThreadObject, t_threadname),
	                      "->?Dstring\n"
	                      "The name of the thread, or :none when none was assigned"),
#else /* !CONFIG_NO_THREADS */
	TYPE_MEMBER_CONST_DOC("name", &main_thread_name,
	                      "The name of the thread, or :none when none was assigned"),
#endif /* CONFIG_NO_THREADS */
	TYPE_MEMBER_END
};


PUBLIC WUNUSED int (DCALL DeeThread_Sleep)(uint64_t microseconds) {
#ifdef CONFIG_HOST_WINDOWS
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
#elif defined(CONFIG_HOST_UNIX)
	struct timespec sleep_time, rem;
	sleep_time.tv_sec  = (time_t)(microseconds / 1000000);
	sleep_time.tv_nsec = (long)(microseconds % 1000000) * 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	/* XXX: Without nanosleep(), use select() */
	if (nanosleep(&sleep_time, &rem)) {
		DBG_ALIGNMENT_ENABLE();
		if (DeeThread_CheckInterrupt())
			goto err;
		if (errno == EINTR) {
			memcpy(&sleep_time, &rem, sizeof(struct timespec));
			goto again;
		}
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#else
	if (DeeThread_CheckInterrupt())
		goto err;
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "The host does not implement a way of sleeping");
#endif
err:
	return -1;
}

PUBLIC void DCALL
DeeThread_SleepNoInterrupt(uint64_t microseconds) {
#ifdef CONFIG_HOST_WINDOWS
	/* XXX: More precision? */
	DBG_ALIGNMENT_DISABLE();
	SleepEx((DWORD)(microseconds / 1000), TRUE);
	DBG_ALIGNMENT_ENABLE();
#elif defined(CONFIG_HOST_UNIX)
	struct timespec sleep_time;
	sleep_time.tv_sec  = (time_t)(microseconds / 1000000);
	sleep_time.tv_nsec = (long)(microseconds % 1000000) * 1000;
	DBG_ALIGNMENT_DISABLE();
	/* XXX: Without nanosleep(), use select() */
	nanosleep(&sleep_time, NULL);
	DBG_ALIGNMENT_ENABLE();
#else
	(void)microseconds;
	/* Unsupported */
#endif
}

#ifndef CONFIG_NO_THREADS
PRIVATE struct type_gc thread_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&thread_clear
};
#endif /* !CONFIG_NO_THREADS */


PUBLIC DeeTypeObject DeeThread_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Thread),
	/* .tp_doc      = */ DOC("The core object type for enabling parallel computation\n"
							"\n"
							"()\n"
							"(name:?Dstring)\n"
							"(main:?DCallable,args:?DTuple=!N)\n"
							"(name:?Dstring,main:?DCallable,args:?DTuple=!N)\n"
							"Construct a new thread that that has yet to be started.\n"
							"When no @main callable has been provided, invoke a $run "
							"member which must be implemented by a sub-class:\n"
							">import thread from deemon;\n"
							">class MyWorker: thread {\n"
							"> private m_jobs;\n"
							">\n"
							"> this(jobs)\n"
							">  : m_jobs = jobs\n"
							"> {}\n"
							">\n"
							"> @\"Thread entry point\"\n"
							"> run() {\n"
							">  for (local j: m_jobs)\n"
							">   j();\n"
							">  return 42;\n"
							"> }\n"
							">}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
#ifndef CONFIG_NO_THREADS
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &thread_ctor,
				TYPE_FIXED_ALLOCATOR_GC(DeeThreadObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&thread_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&thread_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&thread_visit,
	/* .tp_gc            = */ &thread_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
#else /* !CONFIG_NO_THREADS */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeThreadObject)
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
#endif /* CONFIG_NO_THREADS */
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */thread_methods,
	/* .tp_getsets       = */thread_getsets,
	/* .tp_members       = */thread_members,
	/* .tp_class_methods = */thread_class_methods,
	/* .tp_class_getsets = */thread_class_getsets,
	/* .tp_class_members = */thread_class_members
};


#ifndef CONFIG_NO_THREADS
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
		dst->cf_argv  = (DREF DeeObject **)localheap_malloc(heap, dst->cf_argc *
		                                                         sizeof(DREF DeeObject *));
		dst->cf_frame = (DREF DeeObject **)localheap_malloc(heap, code->co_localc *
		                                                          sizeof(DREF DeeObject *));
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
				dst->cf_argv[i] = iter->cf_argv[i];
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
			new_vector = (DREF DeeObject **)Dee_Malloc(code->co_localc *
			                                           sizeof(DREF DeeObject *));
			if unlikely(!new_vector)
				goto err;
			MEMCPY_PTR(new_vector, vector->cf_frame, code->co_localc);
			vector->cf_frame = new_vector;
		}
		if (!vector->cf_argc) {
			vector->cf_argv = NULL;
		} else {
			new_vector = (DREF DeeObject **)Dee_Malloc(vector->cf_argc *
			                                           sizeof(DREF DeeObject *));
			if unlikely(!new_vector)
				goto err;
			MEMCPY_PTR(new_vector, vector->cf_argv, vector->cf_argc);
			vector->cf_argv = new_vector;
		}
	}
	return true;
err:
	return false;
}
#endif /* !CONFIG_NO_THREADS */



PUBLIC WUNUSED NONNULL((1)) DREF /*Traceback*/ DeeObject *DCALL
DeeThread_Trace(/*Thread*/ DeeObject *__restrict self) {
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *me = (DeeThreadObject *)self;
	if (me != DeeThread_Self()) {
		if (me->t_state & THREAD_STATE_EXTERNAL) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot trace external thread %k",
			                self);
			goto err;
		}
		/* Must suspend and capture this thread. */
		for (;;) {
			uint16_t traceback_size = ATOMIC_READ(me->t_execsz);
			uint16_t traceback_used;
			DeeTracebackObject *result;
			struct localheap heap;
			/* Always return an empty traceback for terminated threads. */
			if (me->t_state & THREAD_STATE_TERMINATED)
				traceback_size = 0;
			result = (DeeTracebackObject *)DeeGCObject_Malloc(offsetof(DeeTracebackObject, tb_frames) +
			                                                  traceback_size * sizeof(struct code_frame));
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
			memset(result->tb_frames, 0, traceback_size * sizeof(struct code_frame));
			/* This is where it gets dangerous: Suspend the thread and collect information! */
			COMPILER_BARRIER();
			DeeThread_Suspend(me);
			COMPILER_BARRIER();
			traceback_used = me->t_execsz;
			if (traceback_used > traceback_size) {
				DeeTracebackObject *new_result;
				/* Our traceback was too small. - Reallocate to fit. */
				COMPILER_BARRIER();
				DeeThread_Resume(me);
				COMPILER_BARRIER();
				traceback_size = traceback_used;
				new_result = (DeeTracebackObject *)DeeGCObject_Realloc(result, offsetof(DeeTracebackObject, tb_frames) +
				                                                               traceback_size * sizeof(struct code_frame));
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
			if (traceback_size != traceback_used) {
				DeeTracebackObject *new_result;
				new_result = (DeeTracebackObject *)DeeGCObject_TryRealloc(result,
				                                                          offsetof(DeeTracebackObject, tb_frames) +
				                                                          traceback_used * sizeof(struct code_frame));
				if likely(new_result)
					result = new_result;
			}

			/* With everything collected, we must still convert pointers apart
			 * of our mini-heap to something that is apart of the real heap. */
			if (!copy_dynmem(traceback_used, result->tb_frames))
				goto err_free_result;

			/* With dynamic memory duplicated, free the temporary (local) heap. */
			Dee_Free(heap.lh_base);

done_traceback:
			/* Initialize remaining members of the traceback. */
			Dee_Incref(me); /* Reference stored in `tb_thread' */
			result->tb_thread = me;
			rwlock_init(&result->tb_lock);
			result->tb_numframes = traceback_used;
			DeeObject_Init(result, &DeeTraceback_Type);
			/* Tracebacks are GC objects, so we need to start tracking it here. */
			DeeGC_Track((DeeObject *)result);
			DEE_CHECKMEMORY();
			return (DREF DeeObject *)result;
err_free_result:
			Dee_Free(heap.lh_base);
			DeeObject_Free(result);
		}
err:
		return NULL;
	}
#endif /* !CONFIG_NO_THREADS */
	return (DREF DeeObject *)DeeTraceback_New((DeeThreadObject *)self);
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
