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
/*!export -CONFIG_HAVE_**/
/*!export DeeThread*/
/*!export Dee_THREAD_STATE_**/
/*!export Dee_except_frame*/
/*!export Dee_pid_t_IS_**/
/*!export Dee_thread_interrupt**/
/*!export _DeeThread_**/
#ifndef GUARD_DEEMON_THREAD_H
#define GUARD_DEEMON_THREAD_H 1 /*!export-*/

#include "api.h"

#include <hybrid/__atomic.h>      /* __ATOMIC_ACQUIRE, __ATOMIC_RELEASE, __hybrid_atomic_* */
#include <hybrid/host.h>          /* __arm__, __i386__, __linux__, __unix__, __x86_64__ */
#include <hybrid/sched/__yield.h> /* __hybrid_yield */
#include <hybrid/typecore.h>      /* __ULONG32_TYPE__ */

#include "types.h"      /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, Dee_OBJECT_HEAD, Dee_REQUIRES_OBJECT, Dee_refcnt_t, ITER_DONE */
#include "util/futex.h" /* DeeFutex_WakeAll */

#include <stdbool.h> /* bool */
#include <stdint.h>  /* uint16_t, uint32_t, uint64_t, uintptr_t */

#ifndef __INTELLISENSE__
#include "alloc.h"  /* DeeSlab_* */
#include "object.h" /* DeeObject_NewPack, _DeeRefcnt_* */
#endif /* !__INTELLISENSE__ */

/*!fixincludes fake_include "system-features.h" // pid_t */

#undef Dee_pid_t
#ifdef CONFIG_HOST_WINDOWS
#define Dee_pid_t_IS_DWORD
#define Dee_pid_t __ULONG32_TYPE__
#else /* CONFIG_HOST_WINDOWS */
#ifdef CONFIG_NO_SYS_TYPES_H
#undef CONFIG_HAVE_SYS_TYPES_H
#elif !defined(CONFIG_HAVE_SYS_TYPES_H) && \
      (__has_include(<sys/types.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_TYPES_H
#endif

#ifdef CONFIG_HAVE_SYS_TYPES_H
#include <sys/types.h> /* pid_t */
#ifdef CONFIG_NO_pid_t
#undef CONFIG_HAVE_pid_t
#elif !defined(CONFIG_HAVE_pid_t) && \
      (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))
#define CONFIG_HAVE_pid_t
#endif
#ifdef CONFIG_HAVE_pid_t
#define Dee_pid_t_IS_pid_t
#define Dee_pid_t pid_t
#endif /* CONFIG_HAVE_pid_t */
#endif /* CONFIG_HAVE_SYS_TYPES_H */
#endif /* !CONFIG_HOST_WINDOWS */

DECL_BEGIN

struct Dee_code_frame;
struct Dee_tuple_object;
struct Dee_traceback_object;
struct Dee_string_object;

struct Dee_except_frame {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	struct Dee_except_frame          *ef_prev;  /* [0..1][lock(PRIVATE(DeeThread_Self()))][owned] Previous frame. */
	DREF DeeObject                   *ef_error; /* [1..1][const] The actual error object that got thrown. */
	DREF struct Dee_traceback_object *ef_trace; /* [0..1][const] A copy of the execution stack at the time of the error being thrown.
	                                             * NOTE: When `ITER_DONE' the traceback has yet to be allocated.
	                                             * NOTE: Set to `NULL' when there is no traceback. */
};

#ifdef __INTELLISENSE__
#define Dee_except_frame_tryalloc() ((struct Dee_except_frame *)sizeof(struct Dee_except_frame))
#define Dee_except_frame_alloc()    ((struct Dee_except_frame *)sizeof(struct Dee_except_frame))
#define Dee_except_frame_free(ptr)  (void)(Dee_REQUIRES_TYPE(struct Dee_except_frame *, ptr))
#define Dee_except_frame_xfree(ptr) (void)(Dee_REQUIRES_TYPE(struct Dee_except_frame *, ptr))
#else /* __INTELLISENSE__ */
#define Dee_except_frame_tryalloc() DeeSlab_TRYMALLOC(struct Dee_except_frame)
#define Dee_except_frame_alloc()    DeeSlab_MALLOC(struct Dee_except_frame)
#define Dee_except_frame_free(ptr)  DeeSlab_FREE(Dee_REQUIRES_TYPE(struct Dee_except_frame *, ptr))
#define Dee_except_frame_xfree(ptr) DeeSlab_XFREE(Dee_REQUIRES_TYPE(struct Dee_except_frame *, ptr))
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_BUILDING_DEEMON
/* Returns the traceback of a given exception-frame, or
 * `NULL' if no traceback exists for the exception. */
INTDEF WUNUSED NONNULL((1)) struct Dee_traceback_object *DCALL
except_frame_gettb(struct Dee_except_frame *__restrict self);
#endif /* CONFIG_BUILDING_DEEMON */

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
struct Dee_import_frame {
	struct Dee_import_frame *if_prev;    /* [0..1][lock(PRIVATE(DeeThread_Self()))] Previous frame. */
	/*utf-8*/ char const    *if_absfile; /* [1..1][const] Absolute, normalized filename of module being compiled */
};
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

struct Dee_repr_frame {
	struct Dee_repr_frame *rf_prev; /* [0..1][lock(PRIVATE(DeeThread_Self()))] Previous frame. */
	DeeObject             *rf_obj;  /* [1..1][const] The object for which the `__str__' or `__repr__' operator is being invoked. */
#if !defined(__i386__) && !defined(__x86_64__) && !defined(__arm__)
	DeeTypeObject         *rf_type; /* [1..1][const] The type of object that is being converted to a string.
	                                 * NOTE: On architectures where it isn't a fault to access memory past
	                                 *       the allocated end of local variables, we don't need to allocate
	                                 *       a field for the object's type when we're not required to track
	                                 *       it. Additionally, when we do actually track it, it doesn't matter
	                                 *       when this field contains undefined contents as we only read it
	                                 *       for a comparison check, but don't actually dereference the pointer. */
#endif /* !__i386__ && !__x86_64__ && !__arm__ */
};

struct Dee_trepr_frame {
	struct Dee_trepr_frame *rf_prev; /* [0..1][lock(PRIVATE(DeeThread_Self()))] Previous frame. */
	DeeObject              *rf_obj;  /* [1..1][const] The object for which the `__str__' or `__repr__' operator is being invoked. */
	DeeTypeObject          *rf_type; /* [1..1][const] The type of object that is being converted to a string. */
};

#ifdef CONFIG_BUILDING_DEEMON
struct Dee_thread_object;

/* Lookup a GC association of `old_object', who's
 * new object is an exact instance of `new_type' */
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
deepcopy_lookup(struct Dee_thread_object *thread_self, DeeObject *old_object,
                DeeTypeObject *new_type);

/* Begin/end a deepcopy operation after a lookup fails. */
#define deepcopy_begin(thread_self) (++(thread_self)->t_deepassoc.da_recursion)
#define deepcopy_end(thread_self)   (--(thread_self)->t_deepassoc.da_recursion || (deepcopy_clear(thread_self), 0))
INTDEF NONNULL((1)) void DCALL deepcopy_clear(struct Dee_thread_object *__restrict thread_self);
#endif /* CONFIG_BUILDING_DEEMON */


struct Dee_thread_interrupt;
struct Dee_thread_interrupt {
	struct Dee_thread_interrupt  *ti_next; /* [0..1][owned] Next pending interrupt descriptor. */
	DREF DeeObject               *ti_intr; /* [1..1][const] The interrupt object/callback. */
	DREF struct Dee_tuple_object *ti_args; /* [0..1][const] Interrupt callback arguments or `NULL' if
	                                        *               `ti_intr' should be thrown as an error.
	                                        * When set to `ITER_DONE', don't free the interrupt
	                                        * descriptor, and simply decref() `ti_intr'. */
};

/* NOTE: It is important that thread interrupts and exceptions can be reused for one-another! */
#ifdef __INTELLISENSE__
#define Dee_thread_interrupt_alloc()     ((struct Dee_thread_interrupt *)sizeof(struct Dee_thread_interrupt))
#define _Dee_thread_interrupt_free(self) (void)(self)
#else /* __INTELLISENSE__ */
#define Dee_thread_interrupt_alloc()     DeeSlab_MALLOC(struct Dee_thread_interrupt)
#define _Dee_thread_interrupt_free(self) DeeSlab_FREE(self)
#endif /* !__INTELLISENSE__ */
#define Dee_thread_interrupt_free(self)  (likely((self)->ti_args != (DREF struct Dee_tuple_object *)ITER_DONE) ? _Dee_thread_interrupt_free(self) : (void)0)
#define Dee_thread_interrupt_xfree(self) (void)((self) && (Dee_thread_interrupt_free(self), 0))

/*
 * Thread state phases:
 *
 * Startup:
 * #1: Dee_THREAD_STATE_INITIAL:      The deemon thread object was created and can be configured
 * #2: Dee_THREAD_STATE_SETUP:        By holding this bit-lock, you can configure the thread
 * #3: Dee_THREAD_STATE_STARTING:     You're inside of `DeeThread_Start()'
 * #4: Dee_THREAD_STATE_STARTED:      The thread was created and has acknowledged that it now exists
 * #5: Dee_THREAD_STATE_STARTING:     The flag by the parent thread `Dee_THREAD_STATE_STARTED' becomes set
 *
 * Running (interrupt):
 * #1: Dee_THREAD_STATE_INTERRUPTING: By holding this bit-lock, you can schedule interrupts for the thread
 * #2: Dee_THREAD_STATE_INTERRUPTED:  When this flag is set, the thread will check for interrupts the next
 *                                    time it calls `DeeThread_CheckInterrupt()'. To force the thread to do
 *                                    so in the near future, use `DeeThread_Wake()'.
 * #3: Dee_THREAD_STATE_INTERRUPTED:  The thread clears this flag once interrupts were handled
 *
 * Running (shutdown):
 * #1: Dee_THREAD_STATE_SHUTDOWNINTR: If the main thread exits, this flag is set for all other threads,
 *                                    alongside `Dee_THREAD_STATE_INTERRUPTED'. Together, these cause the
 *                                    next call to `DeeThread_CheckInterrupt()' to throw `Signal.Interrupt()'
 *
 * Running (suspend):
 * #1: Dee_THREAD_STATE_SUSPENDING:   The thread is supposed to suspend itself. This is done during the next
 *                                    call to `DeeThread_CheckInterrupt()', where the thread will set the
 *                                    `Dee_THREAD_STATE_SUSPENDED' flag, and then block until said flag is
 *                                    unset by another thread.
 * #2: Dee_THREAD_STATE_SUSPENDED:    The thread is suspended. To resume execution, clear this flag and do
 *                                    a futex broadcast on the thread's `t_state'-word.
 *
 * Termination:
 * #1: Dee_THREAD_STATE_TERMINATING:  Set once the thread is about to terminate. Once this flag is set, it
 *                                    becomes impossible for the thread to spawn more threads, receive extra
 *                                    interrupts, or access TLS variables. This state is used for cleanup of
 *                                    still-remaining interrupts, as well as TLS variables.
 * #2: Dee_THREAD_STATE_TERMINATED:   The thread has fully terminated (`DeeThread_Join()' waits for this).
 *
 * Other flags:
 * - Dee_THREAD_STATE_DETACHING: Lock-flag to ensure that `Dee_THREAD_STATE_HASTHREAD' / `Dee_THREAD_STATE_HASTID' don't change
 * - Dee_THREAD_STATE_HASTHREAD: The thread's OS-specific descriptor is valid (s.a. `DeeOSThreadObject')
 * - Dee_THREAD_STATE_HASTID:    The thread's OS-specific ID is valid (s.a. `DeeOSThreadObject')
 * - Dee_THREAD_STATE_UNMANAGED: The thread isn't managed by deemon (don't destroy the OS-specific thread descriptor)
 * - Dee_THREAD_STATE_WAITING:   Indicator that another thread is waiting for `t_state' to change
 */

#define Dee_THREAD_STATE_INITIAL      0x0000 /* The initial (not-started) thread state */
#define Dee_THREAD_STATE_STARTING     0x0001 /* [lock(SET(ATOMIC), CLEAR(SETTER)), if(Dee_THREAD_STATE_STARTED, [const][SET])] The thread is currently starting. */
#define Dee_THREAD_STATE_STARTED      0x0002 /* [lock(WRITE_ONCE && Dee_THREAD_STATE_SETUP && DeeThread_Self())] The thread has started */
#define Dee_THREAD_STATE_INTERRUPTING 0x0004 /* [lock(ATOMIC)] Lock-flag for `t_interrupt' */
#define Dee_THREAD_STATE_INTERRUPTED  0x0008 /* [lock(SET(ATOMIC), CLEAR(DeeThread_Self()))] There may be unhandled interrupts. */
#define Dee_THREAD_STATE_DETACHING    0x0010 /* [lock(ATOMIC)] Lock-flag for `Dee_THREAD_STATE_HASTHREAD' / `Dee_THREAD_STATE_HASTID'. */
#define Dee_THREAD_STATE_SUSPENDING   0x0040 /* [lock(SET(thread_list_lock), CLEAR(thread_list_lock && SETTER))]
                                              * The thread should suspend itself (NOTE: While this flag is set,
                                              * `thread_list_lock' must not be released).
                                              * NOTE: When cleared, do a futex broadcast on `t_state' */
#define Dee_THREAD_STATE_SUSPENDED    0x0080 /* [lock(DeeThread_Self())] The thread is currently suspended.
                                              * NOTE: When set, do a futex broadcast on `t_state' */
#define Dee_THREAD_STATE_TERMINATING  0x0100 /* [lock(WRITE_ONCE && Dee_THREAD_STATE_INTERRUPTING && DeeThread_Self())] The thread is about to terminate (don't schedule new interrupts) */
#define Dee_THREAD_STATE_TERMINATED   0x0200 /* [lock(WRITE_ONCE && Dee_THREAD_STATE_SETUP && DeeThread_Self())] The thread has exited (when set, do a futex broardcast on `t_state'). */
#define Dee_THREAD_STATE_SHUTDOWNINTR 0x0400 /* [lock(WRITE_ONCE)] When set alongside `Dee_THREAD_STATE_INTERRUPTED', throw `DeeError_Interrupt_instance' */
#define Dee_THREAD_STATE_SETUP        0x0800 /* [lock(ATOMIC), if(Dee_THREAD_STATE_STARTING, [const][CLEAR])] Lock-flag for configuring a thread prior to its lauch. */
#define Dee_THREAD_STATE_HASTHREAD    0x1000 /* [lock(SET(Dee_THREAD_STATE_STARTING), CLEAR(Dee_THREAD_STATE_DETACHING))]
                                              * We have the thread's OS-specific thread handle (always clear when `Dee_THREAD_STATE_DETACHED') */
#ifdef Dee_pid_t
#define Dee_THREAD_STATE_HASTID       0x2000 /* [lock(SET(Dee_THREAD_STATE_STARTING), CLEAR(Dee_THREAD_STATE_DETACHING))]
                                              * We know the thread's TID (always clear when `Dee_THREAD_STATE_DETACHED') */
#endif /* Dee_pid_t */
#define Dee_THREAD_STATE_UNMANAGED    0x4000 /* [const] Set if the thread is unmanaged (i.e. wasn't created by `DeeThread_Start()') */
#define Dee_THREAD_STATE_WAITING      0x8000 /* [lock(ATOMIC)] Set is there may be threads waiting for the futex at `t_state' */

#ifdef CONFIG_NO_THREADS
#undef Dee_THREAD_STATE_SUSPENDING
#undef Dee_THREAD_STATE_SUSPENDED
#undef Dee_THREAD_STATE_TERMINATED
#undef Dee_THREAD_STATE_STARTING
#undef Dee_THREAD_STATE_HASTHREAD
/*#undef Dee_THREAD_STATE_STARTED*/ /* Always set */
#endif /* !CONFIG_NO_THREADS */

struct Dee_thread_object;
typedef struct Dee_thread_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD /* GC object. */
	Dee_refcnt_t                   t_inthookon;  /* [lock(ATOMIC)] Are interrupt hooks enabled? */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	struct Dee_import_frame       *t_import_curr;/* [lock(PRIVATE(DeeThread_Self()))][0..1]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Currently in-progress import module operations. */
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	struct Dee_repr_frame         *t_str_curr;   /* [lock(PRIVATE(DeeThread_Self()))][0..1]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Chain of GC objects currently invoking the `__str__' operator. */
	struct Dee_repr_frame         *t_repr_curr;  /* [lock(PRIVATE(DeeThread_Self()))][0..1]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Chain of GC objects currently invoking the `__repr__' operator. */
	struct Dee_repr_frame         *t_hash_curr;  /* [lock(PRIVATE(DeeThread_Self()))][0..1]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Chain of GC objects currently invoking the `__hash__' operator. */
	struct Dee_code_frame         *t_exec;       /* [lock(PRIVATE(DeeThread_Self()))][0..1][(!= NULL) == (t_execsz != 0)]
	                                              * [if(!Dee_THREAD_STATE_STARTED || Dee_THREAD_STATE_TERMINATED, [0..0])]
	                                              * [const_if(Dee_THREAD_STATE_TERMINATED)]
	                                              * Linked list of code frames currently executing. */
	struct Dee_except_frame       *t_except;     /* [lock(PRIVATE(DeeThread_Self()))][0..1][(!= NULL) == (t_exceptsz != 0)][owned]
	                                              * [if(Dee_THREAD_STATE_TERMINATED, [lock(Dee_THREAD_STATE_SETUP)])]
	                                              * Linked list of all exceptions currently active in this thread.
	                                              * NOTE: Once the thread has been terminated, read/write access
	                                              *       to this field is synchronized using `THREAD_STATE_STARTING'. */
	uint16_t                       t_execsz;     /* [lock(PRIVATE(DeeThread_Self()))][(!= 0) == (t_exec != NULL)]
	                                              * [if(!Dee_THREAD_STATE_STARTED || Dee_THREAD_STATE_TERMINATED, [== 0])]
	                                              * [const_if(Dee_THREAD_STATE_TERMINATED)]
	                                              * The total number of code frames being executed. */
	uint16_t                       t_exceptsz;   /* [lock(PRIVATE(DeeThread_Self()))][(!= 0) == (t_except != NULL)]
	                                              * [if(Dee_THREAD_STATE_TERMINATED, [lock(Dee_THREAD_STATE_SETUP)])]
	                                              * The total number of currently thrown exceptions. */
	uint32_t                       t_state;      /* The current thread's execution state (Set of `THREAD_STATE_*'). */
	struct Dee_thread_interrupt    t_interrupt;  /* [OVERRIDE(ti_intr, [0..1])][OVERRIDE(ti_args, [== NULL || ti_intr != NULL])]
	                                              * [lock(Dee_THREAD_STATE_INTERRUPTING)]
	                                              * [valid_if(!Dee_THREAD_STATE_TERMINATING)]
	                                              * Chain of pending interrupts and synchronous callbacks
	                                              * to-be executed in the context of this thread. */
#ifndef CONFIG_NO_THREADS
	uintptr_t                      t_int_vers;   /* [lock(READ(ATOMIC), WRITE(PRIVATE(DeeThread_Self())))]
	                                              * Incremented each time the thread calls `DeeThread_CheckInterrupt()' while
	                                              * its `Dee_THREAD_STATE_INTERRUPTED' flag is set. Used in order to sync the
	                                              * thread receiving interrupt requests by `DeeThread_Interrupt()'. */
	struct { /* Structure that is API-compatible with `LIST_ENTRY()' */
		struct Dee_thread_object  *le_next;
		struct Dee_thread_object **le_prev;
	}                              t_global;     /* [0..1] Link in list of running threads.
	                                              * [lock(INSERT(thread_list_lock && Dee_THREAD_STATE_STARTING))]
	                                              * [lock(REMOVE(thread_list_lock && Dee_THREAD_STATE_TERMINATED))] */
	DREF struct Dee_string_object *t_threadname; /* [0..1][const] The name of this thread. */
	union {
		DREF DeeObject            *io_main;   /* [0..1][lock(Dee_THREAD_STATE_SETUP)][valid_if(!Dee_THREAD_STATE_STARTED)]
		                                       * The callable object that is executed by this thread.
		                                       * NOTE: If NULL at time of thread start, `Thread.current.run' is used instead */
		DREF DeeObject            *io_result; /* [1..1][lock(DeeThread_Self())]
		                                       * [if(Dee_THREAD_STATE_TERMINATED, [lock(Dee_THREAD_STATE_SETUP)])]
		                                       * [valid_if(Dee_THREAD_STATE_TERMINATED && t_exceptsz == 0)]
		                                       * Thread return value (unset if the thread exits with an error) */
	} t_inout; /* Input/output data */
#endif /* !CONFIG_NO_THREADS */
	union {
#ifndef CONFIG_NO_THREADS
		DREF struct Dee_tuple_object *d_args; /* [1..1][lock(Dee_THREAD_STATE_SETUP)][valid_if(!Dee_THREAD_STATE_STARTED)]
		                                       * Arguments passed to the thread's main-function (`t_inout.io_main' or `Thread.current.run') */
#endif /* !CONFIG_NO_THREADS */
		void                         *d_tls;  /* [0..?][lock(PRIVATE(DeeThread_Self()))]
		                                       * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
		                                       * Thread TLS data controller. (Set to NULL during thread creation / clear) */
	} t_context; /* Contextual data */
#ifndef CONFIG_NO_THREADS
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
	void *t_heap; /* [0..1][lock(WRITE_ONCE && PRIVATE(DeeThread_Self()))] Thread-local heap (for faster Dee_Malloc()) */
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
#endif /* !CONFIG_NO_THREADS */

	/* OS-specific thread data goes here. */
} DeeThreadObject;

#ifdef CONFIG_NO_THREADS
#define _DeeThread__AcquireStateLock(self, flag) (void)0
#define _DeeThread__ReleaseStateLock(self, flag) (void)0
#define _DeeThread_WakeWaiting(self)             (void)0
#define DeeThread_HasStarted(self)               1
#define DeeThread_HasTerminated(self)            0
#define DeeThread_WasInterrupted(self)           0
#define DeeThread_WasDetached(self)              0
#define DeeThread_HasCrashed(self)               ((self) == DeeThread_Self() && (self)->t_exceptsz > 0)
#else /* CONFIG_NO_THREADS */
#define _DeeThread__AcquireStateLock(self, flag)                                           \
	do {                                                                                   \
		while (__hybrid_atomic_fetchor(&(self)->t_state, flag, __ATOMIC_ACQUIRE) & (flag)) \
			__hybrid_yield();                                                              \
	}	__WHILE0
#define _DeeThread__ReleaseStateLock(self, flag) __hybrid_atomic_and(&(self)->t_state, ~(flag), __ATOMIC_RELEASE)
#define _DeeThread_WakeWaiting(self)                                                        \
	((__hybrid_atomic_load(&(self)->t_state, __ATOMIC_ACQUIRE) & Dee_THREAD_STATE_WAITING)  \
	 ? (__hybrid_atomic_and(&(self)->t_state, ~Dee_THREAD_STATE_WAITING, __ATOMIC_RELEASE), \
	    DeeFutex_WakeAll(&(self)->t_state))                                                 \
	 : (void)0)
#define DeeThread_HasStarted(self)     (__hybrid_atomic_load(&(self)->t_state, __ATOMIC_ACQUIRE) & Dee_THREAD_STATE_STARTED)
#define DeeThread_HasTerminated(self)  (__hybrid_atomic_load(&(self)->t_state, __ATOMIC_ACQUIRE) & Dee_THREAD_STATE_TERMINATED)
#define DeeThread_WasInterrupted(self) (__hybrid_atomic_load(&(self)->t_state, __ATOMIC_ACQUIRE) & Dee_THREAD_STATE_INTERRUPTED)
#define DeeThread_WasDetached(self)    (!(__hybrid_atomic_load(&(self)->t_state, __ATOMIC_ACQUIRE) & Dee_THREAD_STATE_HASTHREAD))
#define DeeThread_HasCrashed(self)     ((DeeThread_HasTerminated(self) || (self) == DeeThread_Self()) && (self)->t_exceptsz > 0)
#endif /* !CONFIG_NO_THREADS */

#define _DeeThread_AcquireInterrupt(self) _DeeThread__AcquireStateLock(self, Dee_THREAD_STATE_INTERRUPTING)
#define _DeeThread_ReleaseInterrupt(self) _DeeThread__ReleaseStateLock(self, Dee_THREAD_STATE_INTERRUPTING)
#define _DeeThread_AcquireDetaching(self) _DeeThread__AcquireStateLock(self, Dee_THREAD_STATE_DETACHING)
#define _DeeThread_ReleaseDetaching(self) _DeeThread__ReleaseStateLock(self, Dee_THREAD_STATE_DETACHING)
#define _DeeThread_AcquireSetup(self)     _DeeThread__AcquireStateLock(self, Dee_THREAD_STATE_SETUP)
#define _DeeThread_ReleaseSetup(self)     _DeeThread__ReleaseStateLock(self, Dee_THREAD_STATE_SETUP)




DDATDEF DeeTypeObject DeeThread_Type;
#define DeeThread_Check(ob)      DeeObject_InstanceOf(ob, &DeeThread_Type)
#define DeeThread_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeThread_Type)

/* Create a new thread that will invoke `main()' (without any arguments) once started.
 * @return: * :   The new thread object.
 * @return: NULL: An error occurred. (Always returned for `CONFIG_NO_THREADS') */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeThread_New(DeeObject *main);
#else /* __INTELLISENSE__ */
#define DeeThread_New(main) DeeObject_NewPack(&DeeThread_Type, 1, main)
#endif /* !__INTELLISENSE__ */

/* Construct a new wrapper for an external reference to `pid'
 * NOTE: The given `pid' is _NOT_ inherited! */
#ifdef Dee_pid_t
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeThread_FromTid(Dee_pid_t pid);
#endif /* Dee_pid_t */


/* Start execution of the given thread.
 * @return:  0: Successfully started the thread.
 * @return:  1: The thread had already been started.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS') */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_Start(/*Thread*/ DeeObject *__restrict self);

/* Schedule an interrupt for a given thread.
 * Interrupts are received when a thread calls `DeeThread_CheckInterrupt()'.
 * NOTE: Interrupts are received in order of being sent.
 * NOTE: When `interrupt_args' is non-NULL, rather than throwing the given
 *       `interrupt_main' as an error upon arrival, it is invoked using
 *       `operator ()' with `interrupt_args' (which must be a tuple).
 * @return:  1: The thread has been terminated.
 * @return:  0: Successfully scheduled the interrupt object.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS') */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Interrupt(/*Thread*/ DeeObject *self,
                    DeeObject *interrupt_main,
                    DeeObject *interrupt_args);

/* Try to wake the thread. This will:
 * - Interrupt a currently running, blocking system call (unless
 *   that call is specifically being made as uninterruptible)
 * - Force the thread to return from a call to `DeeFutex_Wait*'
 * - Cause the thread to soon call `DeeThread_CheckInterrupt()' */
DFUNDEF NONNULL((1)) void DCALL
DeeThread_Wake(/*Thread*/ DeeObject *__restrict self);

/* Detach the given thread (no-op if not possible, or already done).
 * @return:  1: Already detached or not yet started
 * @return:  0: Successfully detached
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_Detach(/*Thread*/ DeeObject *__restrict self);

/* Join the given thread.
 * @return: ITER_DONE: The given timeout has expired. (never returned for `(uint64_t)-1')
 * @return: * :   Successfully joined the thread (return value is the thread's return)
 * @return: NULL: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 *                NOTE: If the thread crashed, its errors are propagated into the calling
 *                      thread after being encapsulated as `Error.ThreadError' objects.
 * @param: timeout_nanoseconds: The timeout in microseconds, 0 for try-join,
 *                              or `(uint64_t)-1' for infinite timeout. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeThread_Join(/*Thread*/ DeeObject *__restrict self,
               uint64_t timeout_nanoseconds);

/* Same as `DeeThread_Join()', but don't return the thread's result,
 * or propagate its failing exception. Instead, simply wait for the
 * thread to terminate.
 * @return: 1 : The given timeout has expired.
 * @return: 0 : The thread has now terminated.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_WaitFor(/*Thread*/ DeeObject *__restrict self,
                  uint64_t timeout_nanoseconds);

#ifdef Dee_pid_t
/* Lookup the thread-id of a given thread. Returns `0' when
 * the thread hasn't started, or has already terminated. */
DFUNDEF WUNUSED NONNULL((1)) Dee_pid_t DCALL
DeeThread_GetTid(/*Thread*/ DeeObject *__restrict self);
#endif /* Dee_pid_t */

/* Capture a snapshot of the given thread's execution stack, returning
 * a traceback object describing what is actually being run by it.
 * Note that this is just a snapshot that by no means will remain
 * consistent once this function returns.
 * NOTE: If the given thread is the caller's, this is identical `(Traceback from deemon)()' */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Traceback*/ DeeObject *DCALL
DeeThread_Trace(/*Thread*/ DeeObject *__restrict self);

/* Check for an interrupt exception and throw it if there is one.
 * This function should be called before any blocking system call and is
 * invoked by the interpreter before execution of any JMP-instruction, or
 * only those that jump backwards in code (aka. is guarantied to be checked
 * periodically during execution of any kind of infinite-loop). */
DFUNDEF WUNUSED int (DCALL DeeThread_CheckInterrupt)(void);

#ifdef CONFIG_BUILDING_DEEMON
/* Same as `DeeThread_CheckInterrupt()', but faster
 * if the caller already knows their own thread object. */
INTDEF WUNUSED NONNULL((1)) int
(DCALL DeeThread_CheckInterruptSelf)(DeeThreadObject *__restrict thread_self);
#ifndef __OPTIMIZE_SIZE__
/* Since `DeeThread_Self()' is marked as ATTR_CONST, in many cases where the calling function
 * already knows about its current thread from a previous call to `DeeThread_Self()', the compiler
 * will be able to optimize the secondary lookup away, making the code slightly faster. */
#define DeeThread_CheckInterrupt() DeeThread_CheckInterruptSelf(DeeThread_Self())
#endif /* !__OPTIMIZE_SIZE__ */
#else /* CONFIG_BUILDING_DEEMON */
#define DeeThread_CheckInterruptSelf(self) DeeThread_CheckInterrupt()
#endif /* !CONFIG_BUILDING_DEEMON */

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
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeThread_Suspend(DeeThreadObject *__restrict self);
DFUNDEF NONNULL((1)) void DCALL DeeThread_Resume(DeeThreadObject *__restrict self);

/* Safely suspend/resume all threads but the calling.
 * The same restrictions that apply to `DeeThread_Suspend()'
 * and `DeeThread_Resume()' also apply to this function pair.
 * @return: * :   Start of thread list
 * @return: NULL: An error was thrown */
DFUNDEF WUNUSED DeeThreadObject *DCALL DeeThread_SuspendAll(void);
DFUNDEF void DCALL DeeThread_ResumeAll(void);
#define DeeThread_FOREACH(x) for (; (x); (x) = (x)->t_global.le_next)


/* Sleep for the specified number of microseconds (1/1000000 seconds). */
DFUNDEF WUNUSED int (DCALL DeeThread_Sleep)(uint64_t microseconds);
DFUNDEF void (DCALL DeeThread_SleepNoInt)(uint64_t microseconds);

/* Get the current time (offset from some undefined point) in microseconds. */
DFUNDEF WUNUSED uint64_t (DCALL DeeThread_GetTimeMicroSeconds)(void);

/* Return the thread controller object for the calling thread.
 * If the calling thread wasn't created by `DeeThread_Start()',
 * the caller must call `DeeThread_Accede()' at least once in
 * order to affiliate their thread with deemon. */
DFUNDEF WUNUSED ATTR_CONST ATTR_RETNONNULL DeeThreadObject *DCALL DeeThread_Self(void);

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
DFUNDEF WUNUSED DeeThreadObject *DCALL DeeThread_Accede(void);

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
DFUNDEF void DCALL DeeThread_Secede(DREF DeeObject *thread_result);


#ifdef CONFIG_BUILDING_DEEMON
/* Initialize/Finalize the threading sub-system.
 * NOTE: `DeeThread_SubSystemInit()' must be called by the
 *       thread main before any other part of deemon's API,
 *       whilst `DeeThread_SubSystemFini()' can optionally be called
 *       (if only to prevent resource leaks cleaned up by the OS in
 *       any case) once no other API function is going to run. */
INTDEF void DCALL DeeThread_SubSystemInit(void);
INTDEF void DCALL DeeThread_SubSystemFini(void);

/* Join all threads that are still running
 * after sending an interrupt signal to each.
 * Returns true if at least one thread was joined. */
INTDEF bool DCALL DeeThread_InterruptAndJoinAll(void);

/* Clear all TLS variables assigned to slots in the calling thread.
 * @return: true:  The TLS descriptor table has been finalized.
 * @return: false: No TLS descriptor table had been assigned. */
INTDEF bool DCALL DeeThread_ClearTls(void);
#endif /* CONFIG_BUILDING_DEEMON */


#if defined(CONFIG_BUILDING_LIBTHREADING) || defined(CONFIG_BUILDING_DEEMON)
/* TLS implementation library hooks.
 * NOTE: These are exported publicly because there'd be no point in hiding them.
 *       However the only module that's meant to use these is `libthreading'.
 *       If you attempt to override them yourself, you'll just break that module.
 *       Also: Don't expose these to user-code! */
struct Dee_tls_callback_hooks {
	/* [1..1][lock(WRITE_ONCE)] Called during thread finalization / clear.
	 * @param: data: The `t_context.d_tls' value of the thread in question (may not be calling thread)
	 *               If the thread's `t_context.d_tls' value was NULL, this function is not called. */
	void (DCALL *tc_fini)(void *__restrict data);
};

/* TLS implementation callbacks. */
DDATDEF struct Dee_tls_callback_hooks _DeeThread_TlsCallbacks;
#endif /* CONFIG_BUILDING_LIBTHREADING || CONFIG_BUILDING_DEEMON */


/* Callback hook invoked by `DeeThread_Wake()'. These hooks are presented
 * to allow special interrupt forwarding calls to get the thread to stop
 * doing what it's currently doing, in case it is in some deeply nested
 * 3rd party C code that only provides explicit means of interrupting
 *
 * To prevent unnecessary invocation of hooks, these additional hooks are
 * only executed if a thread has a non-zero `t_inthookon'. This still means
 * that you can just permanently enable hooks for some thread, but most
 * code should simply inc/dec `t_inthookon' around sections where some
 * thread should receive interrupt hooks.
 *
 * Example: sqlite3's `sqlite3_interrupt()' function is called if the
 *          thread being interrupted is currently doing a database op. */
struct Dee_thread_interrupt_hook {
	/* Internal reference counter for this hook. */
	Dee_refcnt_t tih_refcnt;

	/* [1..1][const] Called when `tih_refcnt' hits zero.
	 * This may happen asynchronously **AFTER** `DeeThread_RemoveInterruptHook()'
	 * was called, possibly after `tih_onwake' was called a couple more times,
	 * and/or in the context of another thread. */
	NONNULL_T((1)) void
	(DCALL *tih_destroy)(struct Dee_thread_interrupt_hook *__restrict self);

	/* [1..1][const] The callback that gets invoked */
	NONNULL_T((1, 2)) void
	(DCALL *tih_onwake)(struct Dee_thread_interrupt_hook *__restrict self,
	                    DeeThreadObject *__restrict thread);
};

#define Dee_thread_interrupt_hook_destroy(self) (*(self)->tih_destroy)(self)
#define Dee_thread_interrupt_hook_incref(self)  _DeeRefcnt_Inc(&(self)->tih_refcnt)
#define Dee_thread_interrupt_hook_decref(self)         \
	(void)(_DeeRefcnt_DecFetch(&(self)->tih_refcnt) || \
	       (Dee_thread_interrupt_hook_destroy(self), 0))

/* Helper macros to enable/disable interrupt hooks for a given thread. */
#define DeeThread_EnableInterruptHooks(self) \
	_DeeRefcnt_Inc(&Dee_REQUIRES_OBJECT(DeeThreadObject, self)->t_inthookon)
#define DeeThread_DisableInterruptHooks(self) \
	_DeeRefcnt_Dec(&Dee_REQUIRES_OBJECT(DeeThreadObject, self)->t_inthookon)

/* Register an additional thread interrupt hook.
 * @return: 1 : No-op (given `hook' was already registered)
 * @return: 0 : Success (hook was registered)
 * @return: -1: Failure (an error was thrown) */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_AddInterruptHook(struct Dee_thread_interrupt_hook *__restrict hook);

/* Unregister a previously register string finalization hook.
 * @return: true:  Given `hook' has been unregistered.
 * @return: false: Given `hook' was never registered. */
DFUNDEF NONNULL((1)) bool DCALL
DeeThread_RemoveInterruptHook(struct Dee_thread_interrupt_hook *__restrict hook);



/* The max stack-depth during execution before a stack-overflow is raised. */
DDATDEF uint16_t DeeExec_StackLimit; /* TODO: Change this to uint32_t (we want to allow the user to set stack limits > 2**16, plus 32-bit arithmetic is probably faster than 16-bit!) */

#ifndef Dee_EXEC_DEFAULT_STACK_LIMIT
#define Dee_EXEC_DEFAULT_STACK_LIMIT 1024
#endif /* !Dee_EXEC_DEFAULT_STACK_LIMIT */

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#ifndef DeeThread_CheckInterrupt
#define DeeThread_CheckInterrupt() __builtin_expect(DeeThread_CheckInterrupt(), 0)
#endif /* !DeeThread_CheckInterrupt */
#ifdef CONFIG_BUILDING_DEEMON
#ifndef DeeThread_CheckInterruptSelf
#define DeeThread_CheckInterruptSelf(thread_self) \
	__builtin_expect(DeeThread_CheckInterruptSelf(thread_self), 0)
#endif /* !DeeThread_CheckInterruptSelf */
#endif /* CONFIG_BUILDING_DEEMON */
#ifndef DeeThread_Sleep
#define DeeThread_Sleep(microseconds) __builtin_expect(DeeThread_Sleep(microseconds), 0)
#endif /* !DeeThread_Sleep */
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_THREAD_H */
