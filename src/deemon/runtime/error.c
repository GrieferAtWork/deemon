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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_C
#define GUARD_DEEMON_RUNTIME_ERROR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* fprintf(stderr, ...) */
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/util/atomic.h>
/**/

#include <stddef.h> /* offsetof, size_t */
#include <stdarg.h> /* va_list */
#include <stdint.h> /* uint8_t */
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */

DECL_BEGIN

/* Try to catch (and thereby handle) an instance of `type',
 * returning true if doing so was possible.
 * Upon success, the actual error object thrown is discarded during this process. */
PUBLIC WUNUSED NONNULL((1)) bool DCALL
DeeError_Catch(DeeTypeObject *__restrict type) {
	DeeObject *current;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	current = DeeError_Current();
	if (current && DeeObject_Implements(current, type))
		return DeeError_Handled(ERROR_HANDLED_INTERRUPT);
	return false;
}

/* Same as `DeeError_Catch()', but returns the actual, caught
 * error on success, or "NULL" if no error was thrown, or the
 * currently thrown error doesn't implement `type'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeError_CatchError(DeeTypeObject *__restrict type) {
	DeeObject *current;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	current = DeeError_Current();
	if (current && DeeObject_Implements(current, type)) {
		Dee_Incref(current);
		if likely(DeeError_Handled(ERROR_HANDLED_INTERRUPT))
			return current;
		Dee_Decref_unlikely(current);
	}
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



PRIVATE WUNUSED ATTR_INS(2, 3) Dee_ssize_t DPRINTER_CC
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
	return (Dee_ssize_t)result;
}


/* Display (print to stderr) an error, as well as an optional traceback. */
PUBLIC NONNULL((2)) void DCALL
DeeError_Display(/*utf-8*/ char const *reason,
                 DeeObject *error, DeeObject *traceback) {
	Dee_ssize_t status;
	status = DeeError_DisplayImpl(reason, error, traceback,
	                              &stderr_printer, NULL);
	if unlikely(status < 0) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		if (stderr_printer(NULL, "Failed to print error", 21) < 0)
			DeeError_Handled(ERROR_HANDLED_RESTORE);
	}
}



struct prefix_printer {
	Dee_formatprinter_t pp_printer; /* [1..1] Underlying printer */
	void               *pp_arg;     /* [?..?] Cookie for `pp_printer' */
	unsigned int        pp_state;   /* Printer state (one of `PREFIX_PRINTER_STATE__*') */
#define PREFIX_PRINTER_STATE__INITIAL  0 /* Initial state */
#define PREFIX_PRINTER_STATE__PASSTHRU 1 /* Input passthru state */
#define PREFIX_PRINTER_STATE__ATSOL    2 /* At start-of-line */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
prefix_printer_print(void *arg, char const *__restrict data, size_t datalen) {
	struct prefix_printer *me = (struct prefix_printer *)arg;
	if unlikely(!datalen)
		return 0;
	switch (me->pp_state) {
	case PREFIX_PRINTER_STATE__INITIAL: {
		Dee_ssize_t result, temp;
		me->pp_state = PREFIX_PRINTER_STATE__PASSTHRU;
		result = (*me->pp_printer)(me->pp_arg, ": ", 2);
		if unlikely(result < 0)
			return result;
		temp = prefix_printer_print(me, data, datalen);
		result = likely(temp >= 0) ? result + temp : temp;
		return result;
	}	break;

	case PREFIX_PRINTER_STATE__PASSTHRU:
	case PREFIX_PRINTER_STATE__ATSOL: {
		Dee_ssize_t result = (*me->pp_printer)(me->pp_arg, data, datalen);
		if likely(result >= 0) {
			char const *data_end = data + datalen;
			uint32_t last_ch = unicode_readutf8_rev_n(&data, data_end);
			if (DeeUni_IsLF(last_ch)) {
				me->pp_state = PREFIX_PRINTER_STATE__ATSOL;
			} else {
				me->pp_state = PREFIX_PRINTER_STATE__PASSTHRU;
			}
		}
		return result;
	}	break;

	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

/* Underlying function used to implement "DeeError_Display()" and `Error.display()'
 * This function handles all the formatting / wrapping of errors and-the-like...
 * CAUTION: This function is itself allowed to throw errors! */
PUBLIC WUNUSED NONNULL((2, 4)) Dee_ssize_t DCALL
DeeError_DisplayImpl(char const *reason, DeeObject *error, DeeObject *traceback,
                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	ASSERT_OBJECT(error);
	ASSERT_OBJECT_TYPE_OPT(traceback, &DeeTraceback_Type);
	if (reason == NULL)
		reason = "Unhandled exception";
	for (;;) {
		bool is_error;
		DeeObject *message_obj;
		struct prefix_printer prefixed;
		temp = DeeFormat_Printf(printer, arg, "%s: %k", reason, Dee_TYPE(error));
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;

		/* Special handling for "error":
		 * - If the error has a custom "message", print that instead
		 * - If the error has a custom "cause", print that afterwards */
		is_error = DeeObject_InstanceOf(error, &DeeError_Error);
		message_obj = error;
		if (is_error && ((DeeErrorObject *)error)->e_msg)
			message_obj = ((DeeErrorObject *)error)->e_msg;
		prefixed.pp_printer = printer;
		prefixed.pp_arg     = arg;
		prefixed.pp_state   = PREFIX_PRINTER_STATE__INITIAL;
		temp = DeeObject_Print(message_obj, &prefix_printer_print, &prefixed);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;

		/* If the error message didn't end with a line-feed, print one now */
		if (prefixed.pp_state != PREFIX_PRINTER_STATE__ATSOL) {
			temp = (*printer)(arg, "\n", 1);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (!is_error)
			break;

		/* Special handling for "error": also print causal errors */
		error = ((DeeErrorObject *)error)->e_cause;
		if (!error)
			break;
		reason = "Caused by";
	}
	if (traceback) {
		temp = DeeObject_Print(traceback, &stderr_printer, NULL);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}


/* Throw a given object `error' as an error.
 * @return: -1: Always returns `-1' */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeError_Throw)(DeeObject *__restrict error) {
	Dee_Incref(error);
	return DeeError_ThrowInherited(error);
}

PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeError_ThrowInherited)(/*inherit(always)*/ DREF DeeObject *__restrict error) {
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
	if unlikely(!frame) {
		/* TODO: OOM needs special handling where we:
		 * - Always have at least 1 fallback "except_frame" per thread for use by OOM
		 * - Every "except_frame" has a way of encoding that N additional OOM errors
		 *   happened after that frame (but before the next frame)
		 * - This way, OOM can always be thrown, even if no further frame can be
		 *   allocated anymore. */
		goto err;
	}
	frame->ef_prev  = ts->t_except;
	frame->ef_error = error;
	frame->ef_trace = (DREF DeeTracebackObject *)ITER_DONE;
	ts->t_except    = frame;
	++ts->t_exceptsz;
#ifndef Dee_DPRINT_IS_NOOP
	if (ts->t_exceptsz == 1) {
		/* Only print first-level exceptions to prevent recursion if the error's "repr"
		 * is implemented like:
		 * >> class MyError {
		 * >>     operator repr() {
		 * >>         throw this; // or "throw MyError();"
		 * >>     }
		 * >> } */
		Dee_DPRINTF("[RT] Throw exception: %r (%" PRFu16 ")\n", error, ts->t_exceptsz);
	}
#endif /* !Dee_DPRINT_IS_NOOP */
	return -1;
err:
	Dee_Decref(error);
	return -1;
}


PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeError_VThrowf)(DeeTypeObject *__restrict tp,
                         char const *__restrict format,
                         va_list args) {
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
	return DeeError_ThrowInherited(error_ob);
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

INTERN NONNULL((1, 2)) void DCALL
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
PUBLIC WUNUSED NONNULL((1)) bool DCALL
DeeError_CurrentIs(DeeTypeObject *__restrict tp) {
	DeeThreadObject *ts = DeeThread_Self();
	ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
	if unlikely(!ts->t_except)
		return false;
	return DeeObject_Implements(ts->t_except->ef_error, tp);
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

#elif defined(CONFIG_HOST_UNIX) /* TODO: Use feature test macros instead */

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
