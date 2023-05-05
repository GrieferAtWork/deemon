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
#ifndef GUARD_DEEMON_THREAD_H
#define GUARD_DEEMON_THREAD_H 1

#include "api.h"

#include <stdbool.h>
#include <stddef.h>

#include "object.h"

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

#ifdef DEE_SOURCE
#define Dee_thread_object     thread_object
#define Dee_code_frame        code_frame
#define Dee_tuple_object      tuple_object
#define Dee_traceback_object  traceback_object
#define Dee_except_frame      except_frame
#define except_frame_tryalloc Dee_except_frame_tryalloc
#define except_frame_alloc    Dee_except_frame_alloc
#define except_frame_free     Dee_except_frame_free
#define except_frame_xfree    Dee_except_frame_xfree
#define Dee_repr_frame        repr_frame
#define Dee_trepr_frame       trepr_frame
#define Dee_deep_assoc_entry  deep_assoc_entry
#define Dee_deep_assoc        deep_assoc
#define DEEPASSOC_HASHST      Dee_DEEPASSOC_HASHST
#define DEEPASSOC_HASHNX      Dee_DEEPASSOC_HASHNX
#define DEEPASSOC_HASHIT      Dee_DEEPASSOC_HASHIT
#define Dee_thread_interrupt  thread_interrupt
#define Dee_string_object     string_object
#endif /* DEE_SOURCE */

typedef struct Dee_thread_object DeeThreadObject;

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
#define Dee_except_frame_tryalloc() DeeSlab_TRYMALLOC(struct Dee_except_frame)
#define Dee_except_frame_alloc()    DeeSlab_MALLOC(struct Dee_except_frame)
#define Dee_except_frame_free(ptr)  DeeSlab_FREE(Dee_REQUIRES_TYPE(struct Dee_except_frame *, ptr))
#define Dee_except_frame_xfree(ptr) DeeSlab_XFREE(Dee_REQUIRES_TYPE(struct Dee_except_frame *, ptr))

#ifdef CONFIG_BUILDING_DEEMON
/* Returns the traceback of a given exception-frame, or
 * `NULL' if no traceback exists for the exception. */
INTDEF WUNUSED NONNULL((1)) struct Dee_traceback_object *DCALL
except_frame_gettb(struct Dee_except_frame *__restrict self);
#endif /* CONFIG_BUILDING_DEEMON */

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

struct Dee_deep_assoc_entry {
	DREF DeeObject *de_old; /* [0..1] The old object that is being copied.
	                         * NOTE: NULL is used to indicate a sentinel entry.
	                         * NOTE: For the purposes of hashing, `Dee_HashPointer(de_old) ^ Dee_HashPointer(Dee_TYPE(de_new))' is used. */
	DREF DeeObject *de_new; /* [?..1][valid_if(da_old)] The new object that results the copy operation. */
};

struct Dee_deep_assoc {
	/* During deepcopy operations, it should be noted that there
	 * exists the chance that some recursive object is being copied:
	 * >> local my_list = [10, 20];
	 * >> my_list.append(my_list);
	 * >> local dup = deepcopy my_list; // Here.
	 *
	 * To deal with this, the `deepcopy' operator must be able to
	 * recursively track all objects that are already in the process
	 * of being copied, even when those object have only been
	 * partially constructed.
	 *
	 * Therefor, it is necessary to keep track of the association of
	 * any GC-object (those are the ones that can be recursive and
	 * therefor able to re-appear in chains) to the respective
	 * existing object, such that an attempt to deep-copy an object
	 * that is already being duplicated will return the partially
	 * constructed copy.
	 *
	 * For this, we need something that is similar to what a Dict
	 * does, however it doesn't need to be thread-safe (because we
	 * keep track of this thread-locally and only clear the set of
	 * objects having already been copied once execution is no
	 * longer inside any deepcopy operator), and also mustn't keep
	 * track of objects based on __hash__ and __eq__, but based on
	 * their pointers alone.
	 *
	 * It must however still keep a reference to both the old and
	 * the new object, since there is a possibility that some object
	 * is only temporarily being constructed during deepcopy, but
	 * then not actually referenced by the end product.
	 * >> // MAP:  DeeObject *old --> DeeObject *new;
	 * >> function deepcopy(ob) {
	 * >>     local result;
	 * >>     if (ob in THREAD_LOCAL_DEEPCOPY_MAP)
	 * >>         return THREAD_LOCAL_DEEPCOPY_MAP[ob];
	 * >>     if (type(ob).__isgc__) {
	 * >>         result = type(ob).begin_deepcopy();
	 * >>         THREAD_LOCAL_DEEPCOPY_MAP[ob] = result;
	 * >>         type(ob).perform_deepcopy(result, ob);
	 * >>     } else {
	 * >>         result = deepcopy:
	 * >>     }
	 * >>     return result;
	 * >> }
	 */
	size_t                       da_used;      /* Amount of old-new pairs actually in use. */
	size_t                       da_mask;      /* Allocated dictionary size. */
	struct Dee_deep_assoc_entry *da_list;      /* [1..da_used|ALLOC(da_mask+1)][owned_if(!= INTERNAL(empty_deep_assoc))] Deepcopy old-new value pairs. */
	size_t                       da_recursion; /* Amount of recursive deepcopy operations currently active.
	                                            * When this counter is decremented down to ZERO(0), the association map is cleared. */
};
#define Dee_DEEPASSOC_HASHST(self, hash)  ((hash) & (self)->da_mask)
#define Dee_DEEPASSOC_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_DEEPASSOC_HASHIT(self, i)     ((self)->da_list + ((i) & (self)->da_mask))


/* Implementation detail required to implement recursive deepcopy.
 * To see how this function must be used, look at the documentation for `tp_deepload'
 * WARNING: THIS FUNCTION MUST NOT BE CALLED BY THE IMPLEMENTING
 *          TYPE WHEN `tp_deepload' IS BEING IMPLEMENTED! */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_DeepCopyAddAssoc(DeeObject *new_object,
                     DeeObject *old_object);


#ifdef CONFIG_BUILDING_DEEMON
/* Lookup a GC association of `old_object', who's
 * new object is an exact instance of `new_type' */
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
deepcopy_lookup(DeeThreadObject *thread_self, DeeObject *old_object,
                DeeTypeObject *new_type);

/* Begin/end a deepcopy operation after a lookup fails. */
#define deepcopy_begin(thread_self) (++(thread_self)->t_deepassoc.da_recursion)
#define deepcopy_end(thread_self)   (--(thread_self)->t_deepassoc.da_recursion || (deepcopy_clear(thread_self), 0))
INTDEF NONNULL((1)) void DCALL deepcopy_clear(DeeThreadObject *__restrict thread_self);
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

#define Dee_thread_interrupt_alloc()     ((struct Dee_thread_interrupt *)Dee_Malloc(sizeof(struct Dee_thread_interrupt)))
#define Dee_thread_interrupt_free(self)  (likely((self)->ti_args != (DREF struct Dee_tuple_object *)ITER_DONE) ? Dee_Free(self) : (void)0)
#define Dee_thread_interrupt_xfree(self) (void)((self) && (Dee_thread_interrupt_free(self), 0))


/*
 * Thread state phases:
 *
 * Startup:
 * #1: Dee_THREAD_STATE_INITIAL:      The deemon thread object was created and can be configured
 * #2: Dee_THREAD_STATE_SETUP:        By holding this bit-lock, you can configure the thread
 * #3: Dee_THREAD_STATE_STARTING:     You're inside of `DeeThread_Start()'
 * #4: Dee_THREAD_STATE_STARTED:      The thread was created and has acknowledged that it not exists
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

struct Dee_thread_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD /* GC object. */
	struct Dee_repr_frame         *t_str_curr;   /* [lock(PRIVATE(DeeThread_Self()))][0..1]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Chain of objects currently invoking the `__str__' operator. */
	struct Dee_repr_frame         *t_repr_curr;  /* [lock(PRIVATE(DeeThread_Self()))][0..1]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Chain of objects currently invoking the `__repr__' operator. */
	struct Dee_deep_assoc          t_deepassoc;  /* [lock(PRIVATE(DeeThread_Self()))]
	                                              * [valid_if(Dee_THREAD_STATE_STARTED && !Dee_THREAD_STATE_TERMINATED)]
	                                              * Deepcopy association map. */
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
		                                       * NOTE: If NULL when at time of thread start, `Thread.current.run' is used instead */
		DREF DeeObject            *io_result; /* [1..1][lock(DeeThread_Self())]
		                                       * [if(Dee_THREAD_STATE_TERMINATED, [lock(Dee_THREAD_STATE_SETUP)])]
		                                       * [valid_if(Dee_THREAD_STATE_TERMINATED && t_exceptsz == 0)]
		                                       * Return return value (unset if the thread exits with an error) */
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
	/* OS-specific thread data goes here. */
};

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

/* Construct a new wrapper for an external reference to `thread'
 * NOTE: The given `thread' is _NOT_ inherited! */
#ifdef Dee_pid_t
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeThread_FromTid(Dee_pid_t pid);
#endif /* Dee_pid_t */


/* Start execution of the given thread.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 * @return:  0: Successfully started the thread.
 * @return:  1: The thread had already been started. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_Start(/*Thread*/ DeeObject *__restrict self);

/* Schedule an interrupt for a given thread.
 * Interrupts are received when a thread calls `DeeThread_CheckInterrupt()'.
 * NOTE: Interrupts are received in order of being sent.
 * NOTE: When `interrupt_args' is non-NULL, rather than throwing the given
 *      `interrupt_main' as an error upon arrival, it is invoked
 *       using `operator ()' with `interrupt_args' (which must be a tuple).
 * @return:  1: The thread has been terminated.
 * @return:  0: Successfully scheduled the interrupt object.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS') */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Interrupt(/*Thread*/ DeeObject *self,
                    DeeObject *interrupt_main,
                    DeeObject *interrupt_args);

/* Try to wake the thread. This will:
 * - Interrupt a currently running, blocking system call (unless
 *   that call is specifically being made as non-blocking)
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
 * @return: * :   Successfully joined the thread and wrote its return value in *pthread_result.
 * @return: NULL: An error occurred.
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
 * NOTE: If the given thread is the caller's this is identical `(traceback from deemon)()' */
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
#endif /* CONFIG_BUILDING_DEEMON */

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
DFUNDEF WUNUSED DeeThreadObject *DCALL DCALL DeeThread_Accede(void);

/* Secede deemon's control over the calling thread by simulating said
 * thread's termination in the eyes of user-code. Following, this the
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
#ifdef DEE_SOURCE
#define Dee_tls_callback_hooks tls_callback_hooks
#endif /* DEE_SOURCE */

/* TLS implementation library hooks.
 * NOTE: These are exported publicly because there'd be no point in hiding them.
 *       However the only module that's meant to use these is `libthreading'.
 *       If you attempt to override them yourself, you'll just break that module.
 *       Also: Don't expose these to user-code! */
struct Dee_tls_callback_hooks {
	/* [1..1][lock(WRITE_ONCE)] Called during thread finalization / clear.
	 * @param: data: The `t_tlsdata' value of the thread in question (may not be calling thread)
	 *               If the thread's `t_tlsdata' value was NULL, this function is not called. */
	void (DCALL *tc_fini)(void *__restrict data);
};

/* TLS implementation callbacks. */
DDATDEF struct Dee_tls_callback_hooks _DeeThread_TlsCallbacks;
#endif /* CONFIG_BUILDING_LIBTHREADING || CONFIG_BUILDING_DEEMON */

/* The max stack-depth during execution before a stack-overflow is raised. */
DDATDEF uint16_t DeeExec_StackLimit;

#ifndef DEE_CONFIG_DEFAULT_STACK_LIMIT
#define DEE_CONFIG_DEFAULT_STACK_LIMIT 1024
#endif /* !DEE_CONFIG_DEFAULT_STACK_LIMIT */

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define Dee_DeepCopyAddAssoc(new_object, old_object) \
	__builtin_expect(Dee_DeepCopyAddAssoc(new_object, old_object), 0)
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
