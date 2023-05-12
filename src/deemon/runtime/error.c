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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_C
#define GUARD_DEEMON_RUNTIME_ERROR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* fprintf(stderr, ...) */
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/sched/yield.h>

#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */

#include "strings.h"

DECL_BEGIN

/* Try to catch (and thereby handle) an instance of `type',
 * returning true if doing so was possible.
 * Upon success, the actual error object thrown is discarded during this process. */
PUBLIC WUNUSED NONNULL((1)) bool DCALL
DeeError_Catch(DeeTypeObject *__restrict type) {
	DeeObject *current;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	current = DeeError_Current();
	if (current && DeeObject_InstanceOf(current, type))
		return DeeError_Handled(ERROR_HANDLED_INTERRUPT);
	return false;
}

STATIC_ASSERT(ERROR_PRINT_DOHANDLE   == ERROR_HANDLED_RESTORE);
STATIC_ASSERT(ERROR_PRINT_HANDLEINTR == ERROR_HANDLED_INTERRUPT);

/* Handle an error and print it, alongside a human-readable message to `stderr'
 * @param: handle_errors: Describes how (if at all) errors should be handled.
 * @param: reason: When non-NULL, a message explaining the reason for the exception is being handled.
 * @return: true:  The current (or previous) error was printed.
 * @return: false: No error was thrown. */
PUBLIC bool DCALL
DeeError_Print(char const *reason, unsigned int handle_errors) {
	DeeThreadObject *thread_self = DeeThread_Self();
	DeeObject *error_ob;
	if unlikely(!thread_self->t_except)
		return false;
	error_ob = thread_self->t_except->ef_error;
	if (handle_errors != ERROR_PRINT_DOHANDLE ||
	    !DeeType_IsInterrupt(Dee_TYPE(error_ob))) {
		DeeError_Display(reason, error_ob,
		                 (DeeObject *)except_frame_gettb(thread_self->t_except));
	}

	/* If we're not supposed to handle any errors, then don't */
	if (handle_errors == ERROR_PRINT_DONTHANDLE)
		return true;

	/* Handle the error according to interrupt-mode. */
	return DeeError_Handled(handle_errors);
}



PRIVATE WUNUSED ATTR_INS(2, 3) dssize_t DPRINTER_CC
stderr_printer(void *__restrict self,
               char const *__restrict data, size_t datalen) {
	size_t result;
	DREF DeeObject *deemon_stderr;
	(void)self;
#if defined(CONFIG_HOST_WINDOWS) && !defined(Dee_DPRINT_IS_NOOP)
	Dee_DPRINTER(self, data, datalen);
#endif /* CONFIG_HOST_WINDOWS && !Dee_DPRINT_IS_NOOP */
	deemon_stderr = DeeFile_GetStd(DEE_STDERR);
	if likely(deemon_stderr) {
		result = DeeFile_WriteAll(deemon_stderr, data, datalen);
		Dee_Decref_unlikely(deemon_stderr);
	} else {
		result = (size_t)-1;
	}
	return (dssize_t)result;
}


/* Display (print to stderr) an error, as well as an optional traceback. */
PUBLIC NONNULL((2)) void DCALL
DeeError_Display(char const *reason,
                 DeeObject *error,
                 DeeObject *traceback) {
	dssize_t status;
	ASSERT_OBJECT(error);
	ASSERT_OBJECT_TYPE_OPT(traceback, &DeeTraceback_Type);
	if (reason == NULL)
		reason = "Unhandled exception\n";
	status = DeeFormat_Printf(&stderr_printer, NULL,
	                          ">> %k: %k\n",
	                          Dee_TYPE(error), error);
	if unlikely(status < 0)
		goto handle_print_error;
	if (traceback) {
		status = DeeObject_Print(traceback, &stderr_printer, NULL);
		if unlikely(status < 0)
			goto handle_print_error;
	}
	return;
handle_print_error:
	DeeError_Handled(ERROR_HANDLED_RESTORE);
	if (stderr_printer(NULL, "Failed to print error", 21) < 0)
		DeeError_Handled(ERROR_HANDLED_RESTORE);
}

/* Throw a given object `error' as an error.
 * @return: -1: Always returns `-1' */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeError_Throw)(DeeObject *__restrict error) {
	struct except_frame *frame;
	DeeThreadObject *ts = DeeThread_Self();
	ASSERT_OBJECT(error);
	if (error == (DeeObject *)&DeeError_NoMemory_instance) {
		/* Special handling for throwing a bad-allocation error.
		 * >> Required to prevent infinite recursion when allocating
		 *    the exception frame for an out-of-memory error. */
		frame = except_frame_tryalloc();
	} else {
		frame = except_frame_alloc();
	}
	if unlikely(!frame)
		goto done;
	frame->ef_prev  = ts->t_except;
	frame->ef_error = error;
	frame->ef_trace = (DREF DeeTracebackObject *)ITER_DONE;
	ts->t_except    = frame;
	Dee_Incref(error);
	++ts->t_exceptsz;
	Dee_DPRINTF("[RT] Throw exception: %r (%" PRFu16 ")\n", error, ts->t_exceptsz);
done:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeError_VThrowf)(DeeTypeObject *__restrict tp,
                         char const *__restrict format,
                         va_list args) {
	int result;
	DREF DeeObject *argv[1], *error_ob;

	/* Create the message string. */
	argv[0] = DeeString_VNewf(format, args);
	if unlikely(!argv[0])
		goto err;

	/* Pack the constructor argument tuple. */
	error_ob = DeeType_Check(tp)
	           ? DeeObject_New(tp, 1, argv)
	           : DeeObject_Call((DeeObject *)tp, 1, argv);
	Dee_Decref(argv[0]);
	if unlikely(!error_ob)
		goto err;

	/* Throw the new error object. */
	result = DeeError_Throw(error_ob);
	Dee_Decref(error_ob);
	return result;
err:
	return -1;
}

/* Throw a new error of type `tp', using a printf-formatted
 * message passed through `format' and varargs.
 * @return: -1: Always returns `-1'*/
PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DeeError_Throwf)(DeeTypeObject *__restrict tp,
                  char const *__restrict format, ...) {
	va_list args;
	int result;
	va_start(args, format);
	result = DeeError_VThrowf(tp, format, args);
	va_end(args);
	return result;
}

INTERN void DCALL
restore_interrupt_error(DeeThreadObject *__restrict ts,
                        /*inherit*/ struct except_frame *__restrict frame) {
	DREF DeeObject *frame_error = frame->ef_error;

	/* Special handling for interrupt exceptions.
	 * >> Rather than handling this now, we must instead re-schedule
	 *    the interrupt to be executed next with max priority. */
	STATIC_ASSERT(sizeof(struct thread_interrupt) <=
	              sizeof(struct except_frame));
	STATIC_ASSERT(offsetof(struct thread_interrupt, ti_intr) ==
	              offsetof(struct except_frame, ef_error));

	/* Drop a reference to the traceback. - Those don't get scheduled. */
	if (ITER_ISOK(frame->ef_trace))
		Dee_Decref(frame->ef_trace);
	_DeeThread_AcquireInterrupt(ts);
	if (ts->t_interrupt.ti_intr) {
		struct thread_interrupt *pend;
		pend = (struct thread_interrupt *)frame;

		/* If we can safe memory doing it, relocate the
		 * frame to best fit the pending interrupt. */
		__STATIC_IF(sizeof(struct thread_interrupt) < sizeof(struct except_frame)) {
			pend = (struct thread_interrupt *)Dee_TryRealloc(frame, sizeof(struct thread_interrupt));
			if unlikely(!pend)
				pend = (struct thread_interrupt *)frame; /* Not. A. Problem. */
		}
		memcpy(pend, &ts->t_interrupt, sizeof(struct thread_interrupt));
		ts->t_interrupt.ti_next = pend;
		frame                   = NULL; /* Indicate that the frame is being re-used. */
	}

	/* Set the new interrupt to-be delivered next
	 * as the interrupt error we've just handled. */
	ts->t_interrupt.ti_intr = frame_error; /* Inherit */

	/* Indicate that the signal is to-be thrown as an error, not executed as a function. */
	ts->t_interrupt.ti_args = NULL;

	/* Unset the interrupting-flag and set the interrupted-flag. */
	atomic_or(&ts->t_state, Dee_THREAD_STATE_INTERRUPTED);
	_DeeThread_ReleaseInterrupt(ts);

	/* If the frame wasn't used, then still free it! */
	except_frame_xfree(frame);
}

/* Handle the current error, discarding it in the process.
 * @param: mode:   One of `ERROR_HANDLED_*'
 * @return: true:  The current error was handled.
 * @return: false: No error could be handled. */
PUBLIC bool (DCALL DeeError_Handled)(unsigned int mode) {
	struct except_frame *frame;
	DeeThreadObject *ts = DeeThread_Self();
	ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
	if ((frame = ts->t_except) == NULL)
		return false;
	ts->t_except = frame->ef_prev;
	--ts->t_exceptsz;
	if (mode != ERROR_HANDLED_INTERRUPT &&
	    DeeType_IsInterrupt(Dee_TYPE(frame->ef_error))) {
		if (mode != ERROR_HANDLED_RESTORE) {
			/* Don't handle the error at all (keep it as current error). */
			ts->t_except = frame;
			++ts->t_exceptsz;
			return false;
		}
		/* Restore the frame as a pending interrupt. */
		restore_interrupt_error(ts, frame);
		return true;
	}

	Dee_Decref(frame->ef_error);
	if (ITER_ISOK(frame->ef_trace))
		Dee_Decref(frame->ef_trace);
	except_frame_free(frame);
	return true;
}

/* Return the currently effective error, or NULL if none is. */
PUBLIC WUNUSED DeeObject *DCALL DeeError_Current(void) {
	DeeThreadObject *ts = DeeThread_Self();
	ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
	return ts->t_except ? ts->t_except->ef_error : NULL;
}

/* Check if the current exception is an instance of `tp' */
PUBLIC WUNUSED NONNULL((1)) bool DCALL DeeError_CurrentIs(DeeTypeObject *__restrict tp) {
	DeeThreadObject *ts = DeeThread_Self();
	ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
	if unlikely(!ts->t_except)
		return false;
	return DeeObject_InstanceOf(ts->t_except->ef_error, tp);
}



/* Install the keyboard interrupt handler. */
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
INTDEF DeeThreadObject DeeThread_Main;
INTDEF uint8_t keyboard_interrupt_counter;

#define INC_KEYBOARD_INTERRUPT_COUNTER()                                                  \
	do {                                                                                  \
		uint8_t counter;                                                                  \
		do {                                                                              \
			if ((counter = atomic_read(&keyboard_interrupt_counter)) == 0xff)             \
				break;                                                                    \
		} while (!atomic_cmpxch_weak(&keyboard_interrupt_counter, counter, counter + 1)); \
		atomic_or(&DeeThread_Main.t_state, Dee_THREAD_STATE_INTERRUPTED);                 \
	}	__WHILE0


#ifdef CONFIG_HOST_WINDOWS
PRIVATE BOOL WINAPI
nt_ConsoleControlHandler(DWORD CtrlType) {
	if (CtrlType == CTRL_C_EVENT) {
		INC_KEYBOARD_INTERRUPT_COUNTER();
		DeeThread_Wake((DeeObject *)&DeeThread_Main);
		return TRUE;
	}
	return FALSE;
}

INTERN void DCALL
DeeError_InstallKeyboardInterrupt(void) {
	SetConsoleCtrlHandler(&nt_ConsoleControlHandler, TRUE);
}

INTERN void DCALL
DeeError_UninstallKeyboardInterrupt(void) {
	SetConsoleCtrlHandler(&nt_ConsoleControlHandler, FALSE);
}

#elif defined(CONFIG_HOST_UNIX)

PRIVATE void sigint_handler(int UNUSED(signo)) {
	INC_KEYBOARD_INTERRUPT_COUNTER();
	DeeThread_Wake((DeeObject *)&DeeThread_Main);
}

PRIVATE void (*saved_sigint_handler)(int signo) = NULL;

INTERN void DCALL
DeeError_InstallKeyboardInterrupt(void) {
	saved_sigint_handler = signal(SIGINT, &sigint_handler);
}

INTERN void DCALL
DeeError_UninstallKeyboardInterrupt(void) {
	signal(SIGINT, saved_sigint_handler);
}

#else /* ... */

INTERN void DCALL
DeeError_InstallKeyboardInterrupt(void) {
}
INTERN void DCALL
DeeError_UninstallKeyboardInterrupt(void) {
}

#endif /* !... */
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ERROR_C */
