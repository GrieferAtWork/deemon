/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_THREADING_NOTHREAD_C_INL
#define GUARD_DEX_THREADING_NOTHREAD_C_INL 1
#define CONFIG_BUILDING_LIBTHREADING 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/thread.h>

#include "libthreading.h"

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>
#endif /* !CONFIG_NO_THREADS */

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	size_t sem_count; /* Number of available tickets. */
} Semaphore;

PRIVATE WUNUSED NONNULL((1)) int DCALL
sema_init(Semaphore *__restrict self,
          size_t argc, DeeObject *const *argv) {
	self->sem_count = 0;
	return DeeArg_Unpack(argc, argv, "|" UNPuSIZ ":semaphore", &self->sem_count);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sema_enter(Semaphore *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (!self->sem_count) {
		return DeeError_Throwf(&DeeError_RuntimeError,
		                       "Deadlock: no tickets, and not producers");
	}
	--self->sem_count;
	return 0;
#else /* CONFIG_NO_THREADS */
	size_t count;
	for (;;) {
		count = ATOMIC_READ(self->sem_count);
		if (count) {
			if (ATOMIC_CMPXCH_WEAK(self->sem_count, count, count - 1))
				break;
			continue;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return 0;
err:
	return -1;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sema_leave(Semaphore *__restrict self, size_t count) {
#ifdef CONFIG_NO_THREADS
	if unlikely((self->sem_count + count) < self->sem_count) {
		return DeeError_Throwf(&DeeError_IntegerOverflow,
		                       "Integer overflow when posting to semaphore");
	}
	self->sem_count += count;
	return 0;
#else /* CONFIG_NO_THREADS */
	size_t old_count;
	for (;;) {
		old_count = ATOMIC_READ(self->sem_count);
		if unlikely((old_count + count) < old_count) {
			return DeeError_Throwf(&DeeError_IntegerOverflow,
			                       "Integer overflow when posting to semaphore");
		}
		if (ATOMIC_CMPXCH_WEAK(self->sem_count, old_count, old_count + count))
			break;
	}
	return 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sema_post(Semaphore *self, size_t argc, DeeObject *const *argv) {
	size_t count = 1;
	if (DeeArg_Unpack(argc, argv, "|" UNPuSIZ ":post", &count))
		goto err;
	if (sema_leave(self, count))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sema_wait(Semaphore *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":wait"))
		goto err;
	if (sema_enter(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sema_trywait(Semaphore *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":trywait"))
		goto err;
#ifdef CONFIG_NO_THREADS
	if (!self->sem_count)
		return_false;
	--self->sem_count;
	return_true;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		size_t count;
		count = ATOMIC_READ(self->sem_count);
		if (!count)
			return_false;
		if (ATOMIC_CMPXCH_WEAK(self->sem_count, count, count - 1))
			break;
	}
	return_true;
#endif /* !CONFIG_NO_THREADS */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sema_timedwait(Semaphore *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwait", &timeout))
		goto err;
#ifdef CONFIG_NO_THREADS
	if (!self->sem_count)
		return_false;
	--self->sem_count;
	return_true;
#else /* CONFIG_NO_THREADS */
	for (;;) {
		size_t count;
		count = ATOMIC_READ(self->sem_count);
		if (!count) {
			/* XXX: Timeout? */
			return_false;
		}
		if (ATOMIC_CMPXCH_WEAK(self->sem_count, count, count - 1))
			break;
	}
	return_true;
#endif /* !CONFIG_NO_THREADS */
err:
	return NULL;
}


PRIVATE struct type_method tpconst sema_methods[] = {
	{ "post", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sema_post,
	  DOC("(count=!1)\n"
	      "Post @count tickets to the semaphore") },
	{ "wait", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sema_wait,
	  DOC("()\nWait for the semaphore to become ready and acquire a ticket") },
	{ "trywait", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sema_trywait,
	  DOC("->?Dbool\n"
	      "@interrupt\n"
	      "@return true: A ticket was acquired\n"
	      "@return false: No ticket was available\n"
	      "Check if unused tickets are available and acquire one") },
	{ "timedwait", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sema_timedwait,
	  DOC("(timeout_microseconds:?Dint)->?Dbool\n"
	      "@interrupt\n"
	      "@return true: A ticket was acquired\n"
	      "@return false: The given @timeout_microseconds has expired without a ticket becoming available\n"
	      "Wait for up to @timeout_microseconds for a ticket to become ready and try to acquire it") },
	{ NULL }
};

PRIVATE struct type_with sema_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&sema_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&sema_leave
};


INTERN DeeTypeObject DeeSemaphore_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Semaphore",
	/* .tp_doc      = */ DOC("(num_tickets=!0)\n"
	                         "Construct a new semaphore with @num_tickets initial tickets\n"
	                         "\n"
	                         "enter->\n"
	                         "@interrupt\n"
	                         "Same as ?#wait\n"
	                         "\n"
	                         "leave->\n"
	                         "Same as ?#post"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&sema_init,
				TYPE_FIXED_ALLOCATOR(Semaphore)
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
	/* .tp_with          = */ &sema_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sema_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *m_owner;     /* The owning thread (NULL if the mutex is available). */
	size_t           m_recursion; /* The amount of times that the mutex was acquired
	                               * by its current owner (excluding the initial). */
#endif /* !CONFIG_NO_THREADS */
} Mutex;


PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_enter(Mutex *__restrict self) {
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *caller = DeeThread_Self();
	if (self->m_owner == caller) {
		++self->m_recursion;
	} else {
		for (;;) {
			if (ATOMIC_CMPXCH_WEAK(self->m_owner, NULL, caller))
				break;
			if (DeeThread_CheckInterrupt())
				return -1;
			SCHED_YIELD();
		}
		self->m_recursion = 1;
	}
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_leave(Mutex *__restrict self) {
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *caller = DeeThread_Self();
	if (self->m_owner != caller) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "The calling thread isn't holding a lock to this mutex");
		return -1;
	}
	ASSERT(self->m_recursion != 0);
	if (!--self->m_recursion)
		ATOMIC_WRITE(self->m_owner, NULL);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_ctor(Mutex *__restrict self) {
#ifndef CONFIG_NO_THREADS
	self->m_owner     = NULL;
	self->m_recursion = 0;
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE struct type_with mutex_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&mutex_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&mutex_leave
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_acquire(Mutex *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":acquire"))
		goto err;
	if (mutex_enter(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_tryacquire(Mutex *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":tryacquire"))
		goto err;
#ifndef CONFIG_NO_THREADS
	{
		DeeThreadObject *caller = DeeThread_Self();
		if (self->m_owner == caller) {
			++self->m_recursion;
		} else if (ATOMIC_CMPXCH_WEAK(self->m_owner, NULL, caller)) {
			self->m_recursion = 1;
		} else {
			return_false;
		}
	}
#endif /* !CONFIG_NO_THREADS */
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_timedacquire(Mutex *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":tryacquire", &timeout))
		goto err;
#ifndef CONFIG_NO_THREADS
	{
		DeeThreadObject *caller = DeeThread_Self();
		if (self->m_owner == caller) {
			++self->m_recursion;
		} else {
			if (ATOMIC_CMPXCH_WEAK(self->m_owner, NULL, caller)) {
				self->m_recursion = 1;
			} else {
				/* XXX: Timeout? */
				return_false;
			}
		}
	}
#endif /* !CONFIG_NO_THREADS */
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_release(Mutex *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":release"))
		goto err;
	if (mutex_leave(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst mutex_methods[] = {
	{ "acquire", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&mutex_acquire,
	  DOC("()\n"
	      "Wait for the mutex to becomes available and recursive acquires an exclusive lock") },
	{ "tryacquire", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&mutex_tryacquire,
	  DOC("->?Dbool\n"
	      "Try to recursive acquire an exclusive lock but fail and "
	      "return ?f if this is not possible without blocking") },
	{ "timedacquire", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&mutex_timedacquire,
	  DOC("(timeout_microseconds:?Dint)->?Dbool\n"
	      "Try to recursive acquire an exclusive lock but fail and "
	      "return ?f if the given @timeout_microseconds has passed") },
	{ "release", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&mutex_release,
	  DOC("()\n"
	      "@throw RuntimeError The calling thread has not acquired the mutex\n"
	      "Recursively release a lock to @this mutex") },
	{ NULL }
};


INTERN DeeTypeObject DeeMutex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Mutex",
	/* .tp_doc      = */ DOC("Construct a new mutex (mutual exclusion) object\n"
	                         "\n"
	                         "enter->\n"
	                         "Same as ?#acquire\n"
	                         "\n"
	                         "leave->\n"
	                         "Same as ?#release"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mutex_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(Mutex)
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
	/* .tp_with          = */ &mutex_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ mutex_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


/*INTERN DeeTypeObject DeeRWLock_Type;*/ /* TODO */

DECL_END

#endif /* !GUARD_DEX_THREADING_NOTHREAD_C_INL */
