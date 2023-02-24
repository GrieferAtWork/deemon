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

#if defined(CONFIG_HOST_WINDOWS) /*&& !defined(__CYGWIN__)*/
#define CONFIG_THREADS_WINDOWS
#else /* CONFIG_HOST_WINDOWS */
#define CONFIG_THREADS_PTHREAD
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef CONFIG_THREADS_PTHREAD
#define CONFIG_THREADS_JOIN_SEMPAHORE
#endif /* CONFIG_THREADS_PTHREAD */


#ifndef CONFIG_NO_THREADS
#ifdef CONFIG_THREADS_JOIN_SEMPAHORE
#ifndef CONFIG_HOST_WINDOWS
#include "system-features.h"
#if (defined(CONFIG_HAVE_SEMAPHORE_H) && \
     defined(CONFIG_HAVE_sem_init) &&    \
     defined(CONFIG_HAVE_sem_post) &&    \
     defined(CONFIG_HAVE_sem_wait) &&    \
     defined(CONFIG_HAVE_sem_trywait) && \
     defined(CONFIG_HAVE_sem_timedwait))
#define CONFIG_THREADS_JOIN_SEMPAHORE_IS_SEM_T
#endif /* Semaphore... */
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* CONFIG_THREADS_JOIN_SEMPAHORE */
#ifdef CONFIG_THREADS_PTHREAD
#include <pthread.h>
#include <sys/types.h>
#endif /* CONFIG_THREADS_PTHREAD */
#endif /* !CONFIG_NO_THREADS */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_thread_object         thread_object
#define Dee_code_frame            code_frame
#define Dee_tuple_object          tuple_object
#define Dee_traceback_object      traceback_object
#define Dee_except_frame          except_frame
#define except_frame_tryalloc     Dee_except_frame_tryalloc
#define except_frame_alloc        Dee_except_frame_alloc
#define except_frame_free         Dee_except_frame_free
#define except_frame_xfree        Dee_except_frame_xfree
#define Dee_repr_frame            repr_frame
#define Dee_trepr_frame           trepr_frame
#define Dee_deep_assoc_entry      deep_assoc_entry
#define Dee_deep_assoc            deep_assoc
#define DEEPASSOC_HASHST          Dee_DEEPASSOC_HASHST
#define DEEPASSOC_HASHNX          Dee_DEEPASSOC_HASHNX
#define DEEPASSOC_HASHIT          Dee_DEEPASSOC_HASHIT
#define Dee_thread_interrupt      thread_interrupt
#define Dee_string_object         string_object
#define THREAD_STATE_INITIAL      Dee_THREAD_STATE_INITIAL
#define THREAD_STATE_STARTED      Dee_THREAD_STATE_STARTED
#define THREAD_STATE_STARTING     Dee_THREAD_STATE_STARTING
#define THREAD_STATE_INTERRUPTING Dee_THREAD_STATE_INTERRUPTING
#define THREAD_STATE_INTERRUPTED  Dee_THREAD_STATE_INTERRUPTED
#define THREAD_STATE_NOINTERRUPT  Dee_THREAD_STATE_NOINTERRUPT
#define THREAD_STATE_DETACHING    Dee_THREAD_STATE_DETACHING
#define THREAD_STATE_DETACHED     Dee_THREAD_STATE_DETACHED
#define THREAD_STATE_DIDJOIN      Dee_THREAD_STATE_DIDJOIN
#define THREAD_STATE_EXTERNAL     Dee_THREAD_STATE_EXTERNAL
#define THREAD_STATE_SUSPENDREQ   Dee_THREAD_STATE_SUSPENDREQ
#define THREAD_STATE_SHUTDOWNINTR Dee_THREAD_STATE_SHUTDOWNINTR
#define THREAD_STATE_TERMINATED   Dee_THREAD_STATE_TERMINATED
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

#ifndef CONFIG_NO_THREADS
#ifdef CONFIG_THREADS_WINDOWS
typedef void *Dee_thread_t; /* HANDLE */
#elif defined(CONFIG_THREADS_PTHREAD)
typedef pthread_t Dee_thread_t;
#endif
#ifndef CONFIG_NO_THREADID
#ifdef CONFIG_THREADS_WINDOWS
#define SIZEOF_DTHREADID_T  4
typedef uint32_t Dee_threadid_t; /* DWORD */
#else /* CONFIG_THREADS_WINDOWS */
#ifdef __SIZEOF_PID_T__
#define SIZEOF_DTHREADID_T  __SIZEOF_PID_T__
#else /* __SIZEOF_PID_T__ */
#define SIZEOF_DTHREADID_T  4
#endif /* !__SIZEOF_PID_T__ */
#define SIGNED_DTHREADID_T
typedef pid_t Dee_threadid_t;
#endif /* !CONFIG_THREADS_WINDOWS */
#else /* !CONFIG_NO_THREADID */
#define SIZEOF_DTHREADID_T  __SIZEOF_INT__
#define SIGNED_DTHREADID_T
typedef int Dee_threadid_t;
#endif /* CONFIG_NO_THREADID */
#endif /* !CONFIG_NO_THREADS */

#ifdef DEE_SOURCE
typedef Dee_thread_t   dthread_t;
typedef Dee_threadid_t dthreadid_t;
#endif /* DEE_SOURCE */


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
	 * To deal with this, the `deepcopy' operator must be able
	 * to recursively track all objects that are already in the
	 * process of being copied, even when those object have only
	 * been partially constructed.
	 * Therefor, it is necessary to keep track of the association
	 * of any GC-object (those are the ones that can be recursive
	 * and therefor able to re-appear in chains) to the respective
	 * existing object, such that an attempt to deep-copy an object
	 * that is already being duplicated will return the partially
	 * constructed copy.
	 * For this, we need something that is similar to what a Dict
	 * does, however it doesn't need to be thread-safe (because
	 * we keep track of this thread-locally and only clear the
	 * set of objects having already been copied once execution
	 * is no longer inside any deepcopy operator), and also mustn't
	 * keep track of objects based on __hash__ and __eq__, but based
	 * on their pointers alone.
	 * It must however still keep a reference to both the old and
	 * the new object, since there is a possibility that some object
	 * is only temporarily being constructed during deepcopy, but
	 * then not actually referenced by the end product.
	 * >> // MAP:  DeeObject *old --> DeeObject *new;
	 * >> function deepcopy(ob) {
	 * >>     local result;
	 * >>     if (ob in THREAD_LOCAL_DEEPCOPY_MAP)
	 * >>         return THREAD_LOCAL_DEEPCOPY_MAP[ob];
	 * >>     if (type(ob).isgc()) {
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


#ifndef CONFIG_NO_THREADS
struct Dee_thread_interrupt;
struct Dee_thread_interrupt {
	struct Dee_thread_interrupt  *ti_next; /* [0..1][owned][lock(:THREAD_STATE_INTERRUPTING)]
	                                        * Next pending interrupt descriptor. */
	DREF DeeObject               *ti_intr; /* [1..1][const] The interrupt object/callback. */
	DREF struct Dee_tuple_object *ti_args; /* [0..1][const] Interrupt callback arguments or `NULL' if
	                                        *              `ti_intr' should be thrown as an error. */
};

#define Dee_THREAD_STATE_INITIAL      0x0000 /* The initial (not-started) thread state */
#define Dee_THREAD_STATE_STARTED      0x0001 /* The thread was started. */
#define Dee_THREAD_STATE_STARTING     0x0100 /* The thread is currently starting. */
#define Dee_THREAD_STATE_INTERRUPTING 0x0002 /* Construction of an interrupting-object has begun.
                                              * This flag is used as an early indicator of an interrupt
                                              * about to occur and is used to synchronize that operation. */
#define Dee_THREAD_STATE_INTERRUPTED  0x0004 /* An interrupt object has been set.
                                              * NOTE: This flag can only be unset by `DeeThread_Self()' */
#define Dee_THREAD_STATE_NOINTERRUPT  0x0800 /* [PRIVATE] The thread isn't serving interrupts right now. */
#define Dee_THREAD_STATE_DETACHING    0x0008 /* The thread is currently being detached. */
#define Dee_THREAD_STATE_DETACHED     0x0010 /* The thread has been joined/detached. */
#define Dee_THREAD_STATE_DIDJOIN      0x0020 /* The thread has joined. */
#define Dee_THREAD_STATE_EXTERNAL     0x1000 /* The thread exists externally and is not under the control of deemon. */
#define Dee_THREAD_STATE_SUSPENDREQ   0x2000 /* Implementation-specific flag: A suspend has been requested but hasn't been acknowledged yet. */
#define Dee_THREAD_STATE_SHUTDOWNINTR 0x4000 /* Set internally to mark threads that have been interrupted for the purposes to shutdown. */
#define Dee_THREAD_STATE_TERMINATED   0x8000 /* The thread has run its course (this unsets all other flags).
                                              * NOTE: When this flag has been set, any unhandled errors
                                              *       still remaining in `t_except' are either re-thrown
                                              *       when join() is called, or are discarded when the
                                              *       thread descriptor is destroyed or cleared. */

#endif /* !CONFIG_NO_THREADS */

struct Dee_thread_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD /* GC object. */
	struct Dee_code_frame    *t_exec;       /* [lock(PRIVATE(DeeThread_Self()))][0..1][(!= NULL) == (t_execsz != 0)]
	                                         * Linked list of code frames currently executing. */
	struct Dee_except_frame  *t_except;     /* [lock(PRIVATE(DeeThread_Self()))][0..1][(!= NULL) == (t_exceptsz != 0)][owned]
	                                         * Linked list of all exceptions currently active in this thread.
	                                         * NOTE: Once the thread has been terminated, read/write access
	                                         *       to this field is synchronized using `THREAD_STATE_STARTING'. */
	uint16_t                  t_exceptsz;   /* [lock(PRIVATE(DeeThread_Self()))][(!= 0) == (t_except != NULL)]
	                                         * The total number of currently thrown exceptions. */
	uint16_t                  t_execsz;     /* [lock(PRIVATE(DeeThread_Self()))][(!= 0) == (t_exec != NULL)]
	                                         * The total number of code frames being executed. */
#if __SIZEOF_POINTER__ > 4
	uint16_t                  t_padding[(sizeof(void *)-2)/2]; /* ... */
#endif /* __SIZEOF_POINTER__ > 4 */
	struct Dee_repr_frame    *t_str_curr;   /* [lock(PRIVATE(DeeThread_Self()))][0..1] Chain of objects currently invoking the `__str__' operator. */
	struct Dee_repr_frame    *t_repr_curr;  /* [lock(PRIVATE(DeeThread_Self()))][0..1] Chain of objects currently invoking the `__repr__' operator. */
	struct Dee_deep_assoc     t_deepassoc;  /* [lock(PRIVATE(DeeThread_Self()))] Deepcopy association map. */
#ifndef CONFIG_NO_THREADS
	DeeThreadObject         **t_globlpself; /* [1..1][0..1][lock(INTERN(globthread_lock))] Self pointer in the globally linked list of running thread. */
	DeeThreadObject          *t_globalnext; /* [0..1][lock(INTERN(globthread_lock))] Next running thread.
	                                         * NOTE: This list is only used internally and no special information
	                                         *       should be determined from the values or relations of these points.
	                                         *       The only reason this exists, is so that we can define the behavior when
	                                         *       there are still running threads during shutdown (aka. when
	                                         *      `DeeThread_JoinAll()' is called) */
	DREF struct Dee_string_object
	                         *t_threadname; /* [0..1][const] The name of this thread. */
	uint16_t                  t_state;      /* The current thread execution state (Set of `THREAD_STATE_*'). */
	uint16_t                  t_padding2;   /*... */
#ifndef CONFIG_NO_THREADID
	Dee_threadid_t            t_threadid;   /* [valid_if(THREAD_STATE_STARTED|THREAD_STATE_TERMINATED)]
	                                         * System-specific thread ID.
	                                         * WARNING: As far as the system is concerned, this ID is no longer
	                                         *          valid when `THREAD_STATE_TERMINATED' has been set. */
#endif /* !CONFIG_NO_THREADID */
	Dee_thread_t              t_thread;     /* [valid_if(THREAD_STATE_STARTED|THREAD_STATE_TERMINATED)]
	                                         * System-specific thread descriptor. */
	int                       t_suspended;  /* Thread arbitrary suspension counter (Thread must not be/stop executing when non-zero). */
	struct Dee_thread_interrupt
	                          t_interrupt;  /* [OVERRIDE(ti_intr, [0..1])][OVERRIDE(ti_args, [== NULL || ti_intr != NULL])]
	                                         * Chain of pending interrupts and asynchronous callbacks to-be executed in the context of this thread. */
	DREF DeeObject           *t_threadmain; /* [0..1][lock(THREAD_STATE_STARTING)] The user-code callable object that is executed by this thread.
	                                         * NOTE: When NULL during thread startup, this field is filled with a
	                                         *       member function `thread.self().run', which is then invoked instead.
	                                         * HINT: Once execution completes, this field is once again reset to `NULL'. */
	DREF struct Dee_tuple_object
	                         *t_threadargs; /* [1..1][lock(THREAD_STATE_STARTING)]
	                                         * An argument tuple passed to the `tp_call' operator during execution of `t_threadmain'. */
	DREF DeeObject           *t_threadres;  /* [0..1][lock(THREAD_STATE_STARTING)][valid_if(THREAD_STATE_TERMINATED)]
	                                         * The return value of `t_threadmain' once it has finished execution.
	                                         * This value is returned by the `join()' function upon success. */
	void                     *t_tlsdata;    /* [0..?][lock(PRIVATE(DeeThread_Self()))] Thread TLS data controller. (Set to NULL during thread creation / clear) */
#ifdef CONFIG_THREADS_JOIN_SEMPAHORE
	/* Semaphore signaled when the thread becomes joinable.
	 * You might argue that `pthread_join()' could be used for this,
	 * but besides the fact that there is no portable way to perform
	 * a try/timed-join (other than maybe using `alarm()'), pthread_join
	 * has the undesired side-effect of also detaching the thread so where
	 * any further use of its descriptor causes undefined behavior.
	 * This however would lead to various race conditions that can
	 * easily be prevented by just always using a semaphore to
	 * communicate thread termination. */
#ifdef CONFIG_HOST_WINDOWS
	void                     *t_join; /* HANDLE */
#elif defined(CONFIG_THREADS_JOIN_SEMPAHORE_IS_SEM_T)
	sem_t                     t_join;
#else /* ... */
	unsigned int              t_join;
#endif /* !... */
#endif /* CONFIG_THREADS_JOIN_SEMPAHORE */
#endif /* !CONFIG_NO_THREADS */
};

/* Helper macros for querying the state of a given thread.
 * WARNING: The information returned by these is highly volatile and
 *          only a snapshot of what used to be at a certain point. */
#ifndef CONFIG_NO_THREADS
#define DeeThread_IsRunning(x)      ((((DeeThreadObject *)Dee_REQUIRES_OBJECT(x))->t_state & (Dee_THREAD_STATE_STARTED | Dee_THREAD_STATE_TERMINATED)) == Dee_THREAD_STATE_STARTED)
#define DeeThread_HasStarted(x)     (((DeeThreadObject *)Dee_REQUIRES_OBJECT(x))->t_state & Dee_THREAD_STATE_STARTED)
#define DeeThread_WasDetached(x)    (((DeeThreadObject *)Dee_REQUIRES_OBJECT(x))->t_state & Dee_THREAD_STATE_DETACHED)
#define DeeThread_HasTerminated(x)  (((DeeThreadObject *)Dee_REQUIRES_OBJECT(x))->t_state & Dee_THREAD_STATE_TERMINATED)
#define DeeThread_WasInterrupted(x) (((DeeThreadObject *)Dee_REQUIRES_OBJECT(x))->t_state & Dee_THREAD_STATE_INTERRUPTED)
#define DeeThread_HasCrashed(x)     (((DeeThreadObject *)Dee_REQUIRES_OBJECT(x))->t_state & Dee_THREAD_STATE_TERMINATED && ((DeeThreadObject *)(x))->t_except != NULL)
#else /* !CONFIG_NO_THREADS */
#define DeeThread_IsRunning(x)      true
#define DeeThread_HasStarted(x)     true
#define DeeThread_WasDetached(x)    false
#define DeeThread_HasTerminated(x)  false
#define DeeThread_WasInterrupted(x) false
#define DeeThread_HasCrashed(x)     false
#endif /* CONFIG_NO_THREADS */



DDATDEF DeeTypeObject DeeThread_Type;
#define DeeThread_Check(ob)      DeeObject_InstanceOf(ob, &DeeThread_Type)
#define DeeThread_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeThread_Type)

#ifndef CONFIG_NO_THREADS
/* Construct a new wrapper for an external reference to `thread'
 * NOTE: The given `thread' is _NOT_ inherited! */
#ifndef CONFIG_NO_THREADID
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeThread_NewExternal(Dee_thread_t thread, Dee_threadid_t id);
#else /* !CONFIG_NO_THREADID */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeThread_NewExternal(Dee_thread_t thread);
#endif /* CONFIG_NO_THREADID */
#endif /* !CONFIG_NO_THREADS */


/* Start execution of the given thread.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 * @return:  0: Successfully started the thread.
 * @return:  1: The thread had already been started. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_Start(/*Thread*/ DeeObject *__restrict self);

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
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Interrupt(/*Thread*/ DeeObject *self,
                    DeeObject *interrupt_main,
                    DeeObject *interrupt_args);

/* Try to wake the thread. */
DFUNDEF NONNULL((1)) void DCALL
DeeThread_Wake(/*Thread*/ DeeObject *__restrict self);

/* Detach the given thread.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 * @return:  0: The thread was successfully detached.
 * @return:  1: The thread had already been detached. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeThread_Detach(/*Thread*/ DeeObject *__restrict self);

/* Join the given thread.
 * @return: -1: An error occurred. (Always returned for `CONFIG_NO_THREADS')
 *              NOTE: If the thread crashed, its errors are propagated into the calling
 *                    thread after being encapsulated as `Error.ThreadError' objects.
 * @return:  0: Successfully joined the thread and wrote its return value in *pthread_result.
 * @return:  1: The given timeout has expired.
 * @param: timeout_microseconds: The timeout in microseconds, 0 for try-join,
 *                               or (uint64_t)-1 for infinite timeout. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_Join(/*Thread*/ DeeObject *__restrict self,
               DREF DeeObject **__restrict pthread_result,
               uint64_t timeout_microseconds);

/* Capture a snapshot of the given thread's execution stack, returning
 * a traceback object describing what is actually being run by it.
 * Note that this is just a snapshot that by no means will remain
 * consistent once this function returns.
 * NOTE: If the given thread is the caller's this is identical `(traceback from deemon)()' */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Traceback*/ DeeObject *DCALL
DeeThread_Trace(/*Thread*/ DeeObject *__restrict self);

#ifndef CONFIG_NO_THREADS
/* Lookup the descriptor/id of a given thread object.
 * NOTE: When such an object isn't associated, a ValueError is
 *       thrown an -1 is returned, otherwise 0 is returned. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_GetThread(/*Thread*/ DeeObject *__restrict self,
                    Dee_thread_t *__restrict pthread);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeThread_GetTid(/*Thread*/ DeeObject *__restrict self,
                 Dee_threadid_t *__restrict pthreadid);

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
 * NOTE: If the thread is not actually running, this behaves as a no-op.
 * WARNING: On unix-based systems, this function may use `SIGUSR1', meaning
 *          that linked libraries should not make use of that signal.
 * WARNING: To prevent deadlocks, you must _NEVER_ acquire _ANY_ sort
 *          of blocking locks while suspending another thread, for the
 *          chance that the thread that got suspended was (is) holding
 *          the lock, but was robbed of the chance to release it.
 *          Additionally, only async-safe functions must be called
 *         (`DeeThread_Suspend()' and `DeeThread_Resume()' are async-safe)
 * WARNING: Do _NOT_ expose these functions to user-code. They are not
 *          safe in such a context and cannot be made safe either.
 * WARNING: Do not attempt to suspend more than a single thread at once using this
 *          method. If you need to suspend more, use `DeeThread_SuspendAll()' instead!
 * NOTE: This function (`DeeThread_Suspend') synchronously waits for the thread to
 *       actually become suspended, meaning that once it returns, the caller is allowed
 *       to assume that the given thread is no longer capable of executing instructions. */
DFUNDEF NONNULL((1)) void DCALL DeeThread_Suspend(DeeThreadObject *__restrict self);
DFUNDEF NONNULL((1)) void DCALL DeeThread_Resume(DeeThreadObject *__restrict self);

/* Safely suspend/resume all threads but the calling.
 * The same restrictions that apply to `DeeThread_Suspend()'
 * and `DeeThread_Resume()' also apply to this function pair.
 * Additionally, overhead caused by these functions is quite considerable
 * and the main reason as to why they're even here, is to allow for an
 * easy way for debuggers to inspect the traceback, or other thread-private
 * parts of the execution context of other threads (e.g.: This is used
 * when attempting to set the CODE_FASSEMBLY flag after the fact when
 * ensuring that the code object isn't already running)
 * WARNING: Do _NOT_ expose these functions to user-code. They are not
 *          safe in such a context and cannot be made safe either.
 * NOTE: Only threads created and started using deemon, or those that
 *       had called `DeeThread_Self()' beforehand will be affected.
 * WARNING: These functions are non-recursive. Any call to `DeeThread_SuspendAll()'
 *          _MUST_ be followed by a call to `DeeThread_ResumeAll()' before
 *         `DeeThread_SuspendAll()' may be called again. This is because in addition
 *          to suspending execution of all threads but the callers, in order to prevent
 *          a potential deadlock that could arise when 2 threads simultaneously attempt
 *          to suspend all thread other than themself, or to prevent the possibility
 *          of some other thread that wasn't tracked before suddenly appearing,
 *          calls to `DeeThread_Self()' from threads that weren't tracked previously
 *          will block until `DeeThread_ResumeAll()' is called.
 *          Note however that in order to identify (and exclude) the calling thread,
 *          `DeeThread_SuspendAll()' internally calls `DeeThread_Self()', meaning
 *          that the calling thread will always be tracked (and therefor needs to
 *          call `DeeThread_Shutdown()' before terminating) after using this function.
 * NOTE: `DeeThread_SuspendAll()' returns a pointer to a thread object that can
 *        be used to enumerate all the threads that have been suspended using
 *        the macro `DeeThread_FOREACH()'
 *        Note however that this list is in no particular order
 *        and also contains the calling thread among all the others. */
DFUNDEF WUNUSED ATTR_RETNONNULL DeeThreadObject *DCALL DeeThread_SuspendAll(void);
DFUNDEF void DCALL DeeThread_ResumeAll(void);
#define DeeThread_FOREACH(x) for (; (x); (x) = (x)->t_globalnext)


/* Sleep for the specified number of microseconds (1/1000000 seconds). */
DFUNDEF WUNUSED int (DCALL DeeThread_Sleep)(uint64_t microseconds);
DFUNDEF void (DCALL DeeThread_SleepNoInterrupt)(uint64_t microseconds);

/* Get the current time (offset from some undefined point) in microseconds. */
DFUNDEF WUNUSED uint64_t (DCALL DeeThread_GetTimeMicroSeconds)(void);

/* Return the thread controller object for the calling thread.
 * This object is usually allocated when the thread is created
 * through use of `DeeThread_New()', though if the calling thread
 * was created using some other API, this function can be used
 * to lazily allocate the associated thread object at a later
 * point in time.
 * Note however, that if allocation fails at that point, the
 * application will terminate in order to ensure compliance
 * with a non-NULL return value, as well as the fact that without
 * a current-thread object, deemon would have no way of actually
 * throwing an `Error.NoMemory()'.
 * So with that in mind, if you choose to use deemon in your project,
 * you should probably always use its threading API for creating any
 * thread that might eventually invoke any function from deemon's API.
 * WARNING: Much of deemon's core functionality calls this function
 *          internally, including throwing any kind of exception,
 *          or attempting to execute user-code. */
DFUNDEF WUNUSED ATTR_CONST ATTR_RETNONNULL DeeThreadObject *(DCALL DeeThread_Self)(void);
#ifndef CONFIG_NO_THREADID
DFUNDEF WUNUSED ATTR_CONST Dee_threadid_t (DCALL DeeThread_SelfId)(void);
#endif /* !CONFIG_NO_THREADID */

/* Should be called at the end of any kind of custom-created thread
 * that may have invoked `DeeThread_Self()' during any point in its
 * lifetime.
 * This function will drop the thread's reference to its own
 * descriptor (if allocated at any point), as well as set its
 * state to indicate termination.
 * WARNING: This function must _NOT_ be called by the same
 *          thread that initially called `DeeThread_Init()'!
 * HINT: This function may be called multiple times where
 *       all but the first call behave as nops. */
DFUNDEF void (DCALL DeeThread_Shutdown)(void);

/* Initialize/Finalize the threading sub-system.
 * NOTE: `DeeThread_Init()' must be called by the thread
 *        main before any other part of deemon's API,
 *        whilst `DeeThread_Fini()' can optionally be called
 *       (if only to prevent resource leaks cleaned up by the OS in any case)
 *        once no other API function is going to run. */
DFUNDEF void (DCALL DeeThread_Init)(void);
DFUNDEF void (DCALL DeeThread_Fini)(void);

/* Join all threads that are still running
 * after sending an interrupt signal to each.
 * Returns true if at least one thread was joined. */
DFUNDEF bool (DCALL DeeThread_JoinAll)(void);

/* Clear all TLS variables assigned to slots in the calling thread.
 * @return: true:  The TLS descriptor table has been finalized.
 * @return: false: No TLS descriptor table had been assigned. */
DFUNDEF bool (DCALL DeeThread_ClearTls)(void);


#if defined(CONFIG_BUILDING_LIBTHREADING) || \
    defined(CONFIG_BUILDING_DEEMON)
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
	/* [1..1][lock(WRITE_ONCE)] Called when visiting a thread after it has been
	 * terminated, but before it's controller has been destroyed.
	 * @param: data: The `t_tlsdata' value of the thread in question (may not be calling thread)
	 *               If the thread's `t_tlsdata' value was NULL, this function is not called. */
	void (DCALL *tc_visit)(void *__restrict data, Dee_visit_t proc, void *arg);
};

/* TLS implementation callbacks. */
DDATDEF struct Dee_tls_callback_hooks _DeeThread_TlsCallbacks;
#endif /* LIBTHREADING or DEEMON */


#else /* !CONFIG_NO_THREADS */

/* Stub macros for functions not available without thread-support. */
#define DeeThread_CheckInterrupt()       0
#define DeeThread_Suspend(self)    (void)0
#define DeeThread_Resume(self)     (void)0
#define DeeThread_SuspendAll()     (void)0
#define DeeThread_ResumeAll()      (void)0
#ifdef CONFIG_BUILDING_DEEMON
#define DeeThread_CheckInterruptSelf(thread_self)  0
#endif /* CONFIG_BUILDING_DEEMON */

DFUNDEF void (DCALL DeeThread_SleepNoInterrupt)(unsigned int microseconds);
#define DeeThread_Sleep(microseconds) (DeeThread_SleepNoInterrupt(microseconds), 0)

DDATDEF DeeThreadObject       DeeThread_Main;
#define DeeThread_Self()    (&DeeThread_Main)
#define DeeThread_Init()     (void)0
DFUNDEF void (DCALL DeeThread_Fini)(void);
#define DeeThread_JoinAll()  (void)0
#endif /* CONFIG_NO_THREADS */

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
