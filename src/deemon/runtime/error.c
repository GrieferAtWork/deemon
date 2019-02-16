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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_C
#define GUARD_DEEMON_RUNTIME_ERROR_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/error.h>
#include <deemon/tuple.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/util/cache.h>

#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#elif defined(CONFIG_HOST_UNIX)
#include <signal.h>
#endif
#endif

#include "strings.h"

DECL_BEGIN

PUBLIC bool DCALL
DeeError_Catch(DeeTypeObject *__restrict type) {
 DeeObject *current;
 ASSERT_OBJECT(type);
 ASSERT(DeeType_Check(type));
 current = DeeError_Current();
 if (current && DeeObject_InstanceOf(current,type))
     return DeeError_Handled(ERROR_HANDLED_INTERRUPT);
 return false;
}

STATIC_ASSERT(ERROR_PRINT_DOHANDLE   == ERROR_HANDLED_RESTORE);
STATIC_ASSERT(ERROR_PRINT_HANDLEINTR == ERROR_HANDLED_INTERRUPT);

PUBLIC bool DCALL
DeeError_Print(char const *reason, unsigned int handle_errors) {
 DeeThreadObject *thread_self = DeeThread_Self();
 DeeObject *error_ob;
 if unlikely(!thread_self->t_except) return false;
 error_ob = thread_self->t_except->ef_error;
#ifndef CONFIG_NO_THREADS
 if (handle_errors != ERROR_PRINT_DOHANDLE ||
    !DeeType_IsInterrupt(Dee_TYPE(error_ob)))
#endif
 {
  DeeError_Display(reason,error_ob,
                  (DeeObject *)except_frame_gettb(thread_self->t_except));
 }
 /* If we're not supposed to handle any errors, then don't */
 if (handle_errors == ERROR_PRINT_DONTHANDLE)
     return true;
 /* Handle the error according to interrupt-mode. */
 return DeeError_Handled(handle_errors);
}

PUBLIC void DCALL
DeeError_Display(char const *reason,
                 DeeObject *__restrict error,
                 DeeObject *traceback) {
 DeeStringObject *error_str;
 ASSERT_OBJECT(error);
 ASSERT_OBJECT_TYPE_OPT(traceback,&DeeTraceback_Type);
 error_str = (DeeStringObject *)DeeObject_Str(error);
 if unlikely(!error_str) goto handle_error;
 if (!reason) reason = "Unhandled exception\n";
 /* TODO: Use a `dformatprinter' here, and stop relying on stdio!
  *       Also: When printing string objects, print them as UTF-8! */
 DBG_ALIGNMENT_DISABLE();
 fwrite(reason,sizeof(char),strlen(reason),stderr);
 fprintf(stderr,">> %s: %s\n",Dee_TYPE(error)->tp_name,DeeString_STR(error_str));
 DBG_ALIGNMENT_ENABLE();
 DEE_DPRINT(reason);
 DEE_DPRINT(">> ");
 DEE_DPRINT(Dee_TYPE(error)->tp_name);
 DEE_DPRINT(": ");
 DEE_DPRINT(DeeString_STR(error_str));
 DEE_DPRINT("\n");
 Dee_Decref(error_str);
 if (traceback) {
  error_str = (DeeStringObject *)DeeObject_Repr(traceback);
  if unlikely(!error_str) {
   DEE_DPRINT("Failed to print traceback\n");
   DBG_ALIGNMENT_DISABLE();
   fprintf(stderr,"Failed to print traceback\n");
   DBG_ALIGNMENT_ENABLE();
   goto handle_error;
  }
  DBG_ALIGNMENT_DISABLE();
  fprintf(stderr,"%s\n",DeeString_STR(error_str));
  DBG_ALIGNMENT_ENABLE();
  DEE_DPRINT(DeeString_STR(error_str));
  DEE_DPRINT("\n");
  Dee_Decref(error_str);
 }
 return;
handle_error:
 DeeError_Handled(ERROR_HANDLED_RESTORE);
 return;
}

PUBLIC int DCALL
DeeError_Throw(DeeObject *__restrict ob) {
 struct except_frame *frame;
 DeeThreadObject *ts = DeeThread_Self();
 ASSERT_OBJECT(ob);
 if (ob == (DeeObject *)&DeeError_NoMemory_instance) {
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
 frame->ef_error = ob;
 frame->ef_trace = (DREF DeeTracebackObject *)ITER_DONE;
 ts->t_except    = frame;
 Dee_Incref(ob);
 ++ts->t_exceptsz;
 DEE_DPRINTF("[RT] Throw exception: %r (%I16u)\n",ob,ts->t_exceptsz);
done:
 return -1;
}

PUBLIC int DCALL
DeeError_VThrowf(DeeTypeObject *__restrict tp,
                 char const *__restrict format,
                 va_list args) {
 int result;
 DREF DeeObject *argv[1],*error_ob;
 /* Create the message string. */
 argv[0] = DeeString_VNewf(format,args);
 if unlikely(!argv[0]) goto err;
 /* Pack the constructor argument tuple. */
 error_ob = DeeObject_New(tp,1,argv);
 Dee_Decref(argv[0]);
 if unlikely(!error_ob) goto err;
 /* Throw the new error object. */
 result = DeeError_Throw(error_ob);
 Dee_Decref(error_ob);
 return result;
err:
 return -1;
}
PUBLIC int
DeeError_Throwf(DeeTypeObject *__restrict tp,
                char const *__restrict format, ...) {
 va_list args; int result;
 va_start(args,format);
 result = DeeError_VThrowf(tp,format,args);
 va_end(args);
 return result;
}
PUBLIC int DCALL
DeeError_VSysThrowf(DeeTypeObject *__restrict tp, Dee_syserrno_t error,
                    char const *__restrict format, va_list args) {
 (void)error; /* TODO */
 return DeeError_VThrowf(tp,format,args);
}
PUBLIC int
DeeError_SysThrowf(DeeTypeObject *__restrict tp, Dee_syserrno_t error,
                   char const *__restrict format, ...) {
 va_list args; int result;
 va_start(args,format);
 result = DeeError_VSysThrowf(tp,error,format,args);
 va_end(args);
 return result;
}


#ifndef CONFIG_NO_THREADS
INTERN void DCALL
restore_interrupt_error(DeeThreadObject *__restrict ts,
             /*inherit*/struct except_frame *__restrict frame) {
 DREF DeeObject *frame_error = frame->ef_error;
 uint16_t state;
 /* Special handling for interrupt exceptions.
  * >> Rather than handling this now, we must instead re-schedule
  *    the interrupt to be executed next with max priority. */
 STATIC_ASSERT(sizeof(struct thread_interrupt) <=
               sizeof(struct except_frame));
 STATIC_ASSERT(COMPILER_OFFSETOF(struct thread_interrupt,ti_intr) ==
               COMPILER_OFFSETOF(struct except_frame,ef_error));
 /* Drop a reference to the traceback. - Those don't get scheduled. */
 if (ITER_ISOK(frame->ef_trace))
     Dee_Decref(frame->ef_trace);
 while (ATOMIC_FETCHOR(ts->t_state,THREAD_STATE_INTERRUPTING) &
                                   THREAD_STATE_INTERRUPTING)
        SCHED_YIELD();
 if (ts->t_interrupt.ti_intr) {
  struct thread_interrupt *pend;
  pend = (struct thread_interrupt *)frame;
  /* If we can safe memory doing it, relocate the
   * frame to best fit the pending interrupt. */
  __STATIC_IF(sizeof(struct thread_interrupt) < sizeof(struct except_frame)) {
   pend = (struct thread_interrupt *)Dee_TryRealloc(frame,sizeof(struct thread_interrupt));
   if unlikely(!pend) pend = (struct thread_interrupt *)frame; /* Not. A. Problem. */
  }
  memcpy(pend,&ts->t_interrupt,sizeof(struct thread_interrupt));
  ts->t_interrupt.ti_next = pend;
  frame = NULL; /* Indicate that the frame is being re-used. */
 }
 /* Set the new interrupt to-be delivered next
  * as the interrupt error we've just handled. */
 ts->t_interrupt.ti_intr = frame_error; /* Inherit */
 /* Indicate that the signal is to-be thrown as an error, not executed as a function. */
 ts->t_interrupt.ti_args = NULL;
 /* Unset the interrupting-flag and set the interrupted-flag. */
 do state = ATOMIC_READ(ts->t_state);
 while (!ATOMIC_CMPXCH_WEAK(ts->t_state,state,
                           (state&~(THREAD_STATE_INTERRUPTING))|
                                    THREAD_STATE_INTERRUPTED));
 /* If the frame wasn't used, then still free it! */
 except_frame_xfree(frame);
}
#endif

#ifdef CONFIG_NO_THREADS
PUBLIC bool (DCALL DeeError_HandledNoSMP)(void)
#else
PUBLIC bool (DCALL DeeError_Handled)(unsigned int mode)
#endif
{
 struct except_frame *frame;
 DeeThreadObject *ts = DeeThread_Self();
 ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
 if ((frame = ts->t_except) == NULL) return false;
 ts->t_except = frame->ef_prev;
 --ts->t_exceptsz;
#ifndef CONFIG_NO_THREADS
 if (mode != ERROR_HANDLED_INTERRUPT &&
     DeeType_IsInterrupt(Dee_TYPE(frame->ef_error))) {
  if (mode != ERROR_HANDLED_RESTORE) {
   /* Don't handle the error at all (keep it as current error). */
   ts->t_except = frame;
   ++ts->t_exceptsz;
   return false;
  }
  /* Restore the frame as a pending interrupt. */
  restore_interrupt_error(ts,frame);
  return true;
 }
#endif /* !CONFIG_NO_THREADS */
 Dee_Decref(frame->ef_error);
 if (ITER_ISOK(frame->ef_trace))
     Dee_Decref(frame->ef_trace);
 except_frame_free(frame);
 return true;
}

PUBLIC DeeObject *DCALL DeeError_Current(void) {
 DeeThreadObject *ts = DeeThread_Self();
 ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
 return ts->t_except ? ts->t_except->ef_error : NULL;
}

/* Check if the current exception is an instance of `tp' */
PUBLIC bool DCALL DeeError_CurrentIs(DeeTypeObject *__restrict tp) {
 DeeThreadObject *ts = DeeThread_Self();
 ASSERT((ts->t_except != NULL) == (ts->t_exceptsz != 0));
 if unlikely(!ts->t_except)
    return false;
 return DeeObject_InstanceOf(ts->t_except->ef_error,tp);
}



#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
#ifdef CONFIG_NO_THREADS
INTERN void DCALL DeeError_InstallKeyboardInterrupt(void) {
 /* XXX: Without interrupts, how can we do this? */
}
#else /* CONFIG_NO_THREADS */
INTDEF uint8_t keyboard_interrupt_counter;
#define INC_KEYBOARD_INTERRUPT_COUNTER() \
do{ uint8_t counter; \
    do if ((counter = ATOMIC_READ(keyboard_interrupt_counter)) == 0xff) break; \
    while (!ATOMIC_CMPXCH_WEAK(keyboard_interrupt_counter,counter,counter+1)); \
    ATOMIC_FETCHOR(DeeThread_Main.t_state,THREAD_STATE_INTERRUPTED); \
}__WHILE0

#ifndef CONFIG_NO_THREADS
INTDEF DeeThreadObject DeeThread_Main;
#else
DATDEF DeeThreadObject DeeThread_Main;
#endif

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
 SetConsoleCtrlHandler(&nt_ConsoleControlHandler,TRUE);
}
#elif defined(CONFIG_HOST_UNIX)

PRIVATE void sigint_handler(int signo) {
 INC_KEYBOARD_INTERRUPT_COUNTER();
#if 0 /* Not async-safe. */
 DeeThread_Wake((DeeObject *)&DeeThread_Main);
#endif
}

INTERN void DCALL
DeeError_InstallKeyboardInterrupt(void) {
 signal(SIGINT,&sigint_handler);
}
#else
INTERN void DCALL
DeeError_InstallKeyboardInterrupt(void) {
}
#endif
#endif /* !CONFIG_NO_THREADS */
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ERROR_C */
