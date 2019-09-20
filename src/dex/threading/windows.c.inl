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
#ifndef GUARD_DEX_THREADING_WINDOWS_C_INL
#define GUARD_DEX_THREADING_WINDOWS_C_INL 1
#define CONFIG_BUILDING_LIBTHREADING 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/thread.h>

#include <hybrid/atomic.h>

#include <Windows.h>

#include "libthreading.h"

DECL_BEGIN

/* @return:  1: The timeout has expired. (Never returned when `uTimeoutInMicroseconds' is (uint64_t)-1)
 * @return:  0: A ticket was acquired.
 * @return: -1: An error occurred. */
PRIVATE int DCALL
nt_WaitForSemaphore(HANDLE hSemaphore, uint64_t uTimeoutInMicroseconds) {
	DWORD wait_state;
	if (uTimeoutInMicroseconds == (uint64_t)-1) {
again_infinite:
		if (DeeThread_CheckInterrupt())
			goto err;
		DBG_ALIGNMENT_DISABLE();
		wait_state = WaitForMultipleObjectsEx(1, &hSemaphore, FALSE, INFINITE, TRUE);
		DBG_ALIGNMENT_ENABLE();
		switch (wait_state) {

		case WAIT_IO_COMPLETION:
		case WAIT_TIMEOUT:
			goto again_infinite;

		case WAIT_FAILED:
			goto err_nt;

		default:
			break;
		}
	} else if (uTimeoutInMicroseconds) {
		uint64_t timeout_end, now;
		timeout_end = DeeThread_GetTimeMicroSeconds() + uTimeoutInMicroseconds;
again_timed:
		if (DeeThread_CheckInterrupt())
			goto err;
		DBG_ALIGNMENT_DISABLE();
		wait_state = WaitForMultipleObjectsEx(1, &hSemaphore, FALSE,
		                                      (DWORD)(uTimeoutInMicroseconds / 1000),
		                                      TRUE);
		DBG_ALIGNMENT_ENABLE();
		switch (wait_state) {

		case WAIT_IO_COMPLETION:
			goto again_timed;

		case WAIT_TIMEOUT:
			now = DeeThread_GetTimeMicroSeconds();
			if (now >= timeout_end)
				return 1; /* Timeout */
			/* Continue waiting. */
			uTimeoutInMicroseconds = timeout_end - now;
			goto again_timed;

		case WAIT_FAILED:
			goto err_nt;

		default:
			break;
		}
	} else {
		DBG_ALIGNMENT_DISABLE();
		wait_state = WaitForMultipleObjectsEx(1, &hSemaphore, FALSE, 0, TRUE);
		DBG_ALIGNMENT_ENABLE();
		switch (wait_state) {

		case WAIT_IO_COMPLETION:
		case WAIT_TIMEOUT:
			return 1; /* wait failed. */

		case WAIT_FAILED:
			goto err_nt;

		default:
			break;
		}
	}
	return 0;
err_nt:
	DeeError_Throwf(&DeeError_SystemError,
	                "Failed to wait for semaphore");
err:
	return -1;
}


typedef struct {
	OBJECT_HEAD
	HANDLE sem_handle; /* The handle of the semaphore. */
} Semaphore;

PRIVATE int DCALL
sema_init(Semaphore *__restrict self,
          size_t argc, DeeObject **argv) {
	LONG init_value = 0;
	if (DeeArg_Unpack(argc, argv, "|I32u:semaphore", &init_value))
		goto err;
	DBG_ALIGNMENT_DISABLE();
	self->sem_handle = CreateSemaphoreW(NULL, init_value, 0x7fffffff, NULL);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!self->sem_handle)
		goto err_nt;
	return 0;
err_nt:
	DeeError_Throwf(&DeeError_SystemError,
	                "Failed to construct semaphore");
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
sema_fini(Semaphore *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	CloseHandle(self->sem_handle);
	DBG_ALIGNMENT_ENABLE();
}


PRIVATE ATTR_COLD int DCALL err_post_failed(void) {
	return DeeError_Throwf(&DeeError_SystemError,
	                       "Failed to post to semaphore");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
sema_post(Semaphore *__restrict self, size_t argc,
          DeeObject **argv) {
	LONG count = 1;
	if (DeeArg_Unpack(argc, argv, "|I32u:post", &count))
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!ReleaseSemaphore(self->sem_handle, count, NULL))
		goto err_nt;
	DBG_ALIGNMENT_ENABLE();
	return_none;
err_nt:
	DBG_ALIGNMENT_ENABLE();
	err_post_failed();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
sema_wait(Semaphore *__restrict self, size_t argc,
          DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":wait") ||
	    nt_WaitForSemaphore(self->sem_handle, (uint64_t)-1))
		return NULL;
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
sema_trywait(Semaphore *__restrict self, size_t argc,
             DeeObject **argv) {
	int error;
	if (DeeArg_Unpack(argc, argv, ":trywait"))
		goto err;
	error = nt_WaitForSemaphore(self->sem_handle, 0);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
sema_timedwait(Semaphore *__restrict self, size_t argc,
               DeeObject **argv) {
	int error;
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64u:timedwait", &timeout))
		goto err;
	error = nt_WaitForSemaphore(self->sem_handle, timeout);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
sema_fileno(Semaphore *__restrict self, size_t argc,
            DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":fileno"))
		return NULL;
	return DeeInt_NewUIntptr((uintptr_t)self->sem_handle);
}


PRIVATE struct type_method sema_methods[] = {
	{ "post",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sema_post,
	  DOC("(count=!1)\n"
	      "Post @count tickets to the semaphore") },
	{ "wait",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sema_wait,
	  DOC("()\n"
	      "Wait for the semaphore to become ready and acquire a ticket") },
	{ "trywait",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sema_trywait,
	  DOC("->?Dbool\n"
	      "@interrupt\n"
	      "@return true: A ticket was acquired\n"
	      "@return false: No ticket was available\n"
	      "Check if unused tickets are available and acquire one") },
	{ "timedwait",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sema_timedwait,
	  DOC("(timeout_microseconds:?Dint)->?Dbool\n"
	      "@interrupt\n"
	      "@return true: A ticket was acquired\n"
	      "@return false: The given @timeout_microseconds has expired without a ticket becoming available\n"
	      "Wait for up to @timeout_microseconds for a ticket to become ready and try to acquire it") },
	{ "fileno",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sema_fileno,
	  DOC("->?Dint\n"
	      "Non-portable windows extension to retrive the file descriptor number (HANDLE) of the semaphore") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
sema_enter(Semaphore *__restrict self) {
	return nt_WaitForSemaphore(self->sem_handle, (uint64_t)-1);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sema_leave(Semaphore *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!ReleaseSemaphore(self->sem_handle, 1, NULL))
		goto err;
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	DBG_ALIGNMENT_ENABLE();
	return err_post_failed();
}

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
	                         "Same as #wait\n"
	                         "\n"
	                         "leave->\n"
	                         "Same as #post"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&sema_init,
				TYPE_FIXED_ALLOCATOR(Semaphore)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sema_fini,
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
	/* .tp_methods       = */sema_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




typedef struct {
	OBJECT_HEAD
	HANDLE   m_sema;      /* Semaphore used to deal with the initial wait. */
	DWORD    m_owner;     /* [atomic] GetCurrentThreadId() of the owning thread, or (DWORD)-1 when not owned. */
	uint32_t m_recursion; /* [lock(GetCurrentThreadId() == m_owner)]
	                       * [valid_if(m_owner != (DWORD)-1)]
	                       *  The amount of times that the mutex was acquired
	                       *  by its current owner (excluding the initial). */
} Mutex;


PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_timedenter(Mutex *__restrict self, uint64_t timeout) {
	DWORD owner, caller;
	DBG_ALIGNMENT_DISABLE();
	caller = GetCurrentThreadId();
	DBG_ALIGNMENT_ENABLE();
again:
	/* Try to acquire the semaphore and figure out who owns it. */
	owner = ATOMIC_CMPXCH_VAL(self->m_owner, (DWORD)-1, caller);
	if (owner == caller) {
		/* We're already holding the semaphore. */
		++self->m_recursion;
	} else if (owner == (DWORD)-1) {
		/* Acquired the semaphore for the first time. */
		self->m_recursion = 0;
	} else {
		int error;
		/* Wait for a ticket to become available. */
		error = nt_WaitForSemaphore(self->m_sema, timeout);
		if (error)
			return error; /* Error, or timeout. */
		goto again;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_leave(Mutex *__restrict self) {
	DWORD caller;
	DBG_ALIGNMENT_DISABLE();
	caller = GetCurrentThreadId();
	DBG_ALIGNMENT_ENABLE();
	/* Check if the caller is actually the owner. */
	if unlikely(caller != self->m_owner) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Mutex is not owned by the calling thread");
		goto err;
	}
	if (!self->m_recursion) {
		/* Last lock (must clear the owner-field and post the semaphore) */
		self->m_owner = (DWORD)-1;
		DBG_ALIGNMENT_DISABLE();
		if unlikely(!ReleaseSemaphore(self->m_sema, 1, NULL)) {
			DBG_ALIGNMENT_ENABLE();
			err_post_failed();
			goto err;
		}
		DBG_ALIGNMENT_ENABLE();
	} else {
		/* Decrement the recursion counter, but keep the lock. */
		--self->m_recursion;
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_ctor(Mutex *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	self->m_sema = CreateSemaphoreW(NULL, 0, 0x7fffffff, NULL);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!self->m_sema)
		goto err_nt;
	self->m_owner = (DWORD)-1;
	return 0;
err_nt:
	DeeError_Throwf(&DeeError_SystemError,
	                "Failed to construct semaphore");
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
mutex_fini(Mutex *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	CloseHandle(self->m_sema);
	DBG_ALIGNMENT_ENABLE();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mutex_enter(Mutex *__restrict self) {
	return mutex_timedenter(self, (uint64_t)-1);
}

PRIVATE struct type_with mutex_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&mutex_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&mutex_leave
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_acquire(Mutex *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":acquire") ||
	    mutex_timedenter(self, (uint64_t)-1))
		return NULL;
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_tryacquire(Mutex *self, size_t argc, DeeObject **argv) {
	int error;
	if (DeeArg_Unpack(argc, argv, ":tryacquire"))
		goto err;
	error = mutex_timedenter(self, 0);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_timedacquire(Mutex *self, size_t argc, DeeObject **argv) {
	int error;
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64u:tryacquire", &timeout))
		goto err;
	error = mutex_timedenter(self, timeout);
	if unlikely(error < 0)
		goto err;
	return_bool_(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mutex_release(Mutex *self, size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":release") ||
	    mutex_leave(self))
		return NULL;
	return_none;
}

PRIVATE struct type_method mutex_methods[] = {
	{ "acquire", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&mutex_acquire,
	  DOC("()\n"
	      "Wait for the mutex to becomes available and recursive acquires an exclusive lock") },
	{ "tryacquire", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&mutex_tryacquire,
	  DOC("->?Dbool\n"
	      "Try to recursive acquire an exclusive lock but fail and "
	      "return :false if this is not possible without blocking") },
	{ "timedacquire", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&mutex_timedacquire,
	  DOC("(timeout_microseconds:?Dint)->?Dbool\n"
	      "Try to recursive acquire an exclusive lock but fail and "
	      "return :false if the given @timeout_microseconds has passed") },
	{ "release", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&mutex_release,
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
	                         "Same as #acquire\n"
	                         "\n"
	                         "leave->\n"
	                         "Same as #release"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&mutex_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(Mutex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mutex_fini,
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

#endif /* !GUARD_DEX_THREADING_WINDOWS_C_INL */
