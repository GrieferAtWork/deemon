/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_ERROR_H
#define GUARD_DEEMON_ERROR_H 1

#include "api.h"
#include "object.h"
#include <stdint.h>
#include <stdbool.h>

DECL_BEGIN

typedef struct error_object DeeErrorObject;
typedef int32_t syserrno_t;

/* Builtin error classes. */
DDATDEF DeeTypeObject DeeError_Signal;
/* Interrupt signal (cannot be caught using a normal catch-guard)
 * NOTE: In order for an exception handler to be able to process this
 *       signal, it must have the `EXCEPTION_HANDLER_FINTERPT' flag set
 *      (which can be added using the `@:interrupt' tag):
 * >> import Signal from deemon;
 * >> try {
 * >>    try {
 * >>        throw Signal.Interrupt();
 * >>    } catch (...) {
 * >>        print "Won't get here...";
 * >>    }
 * >> } @:interrupt catch (...) {
 * >>    print "But will get here!";
 * >> }
 * This choice was made because of the fact that error names are usually
 * pretty tedious, making the solution of simply using a catch(...)-all
 * guard the best choice in a lot of cases.
 * This however greatly interferes with the thread-interrupt mechanism
 * that will throw a Signal.Interrupt in an attempt to safely unwind
 * user-code running in the thread, to the point where it can terminate
 * normally, exiting with just an Interrupt signal that can then be discarded.
 * However taking catch-all handlers into account, this design breaks
 * because such an interrupt signal would likely be caught by the first
 * piece of user-code that the user wanted to protect against exceptions,
 * not counting on the possibility of that piece of code eventually
 * running in the context of a multi-threaded application that now
 * never realizes that it was meant to terminate.
 * Therefor, from now on only specially marked catch-guards are
 * allowed to process instances of Signal.Interrupt, technically
 * breaking the meaning of a catch-__ALL__ guard, but counting on
 * the fact that no catch-all guard is actually ever written to
 * truly catch everything, but rather only exceptions that would
 * seem obvious.
 * NOTE: To keep things as generic as possible, this behavior isn't actually
 *       specific to the `Signal.Interrupt' builtin type, but rather to any
 *       type that has the `TP_FINTERRUPT' flag set.
 * User-code can define their own interrupt-classes using the `@:interrupt'
 * tag (yes: the same tag that's also used to mark interrupt-catch blocks):
 * >> @:interrupt
 * >> class MyInterrupt { }
 * >> 
 * >> try {
 * >>     throw MyInterrupt();
 * >> } @:interrupt catch (e...) {
 * >>     print e; // `MyInterrupt'
 * >> }
 * >> 
 * Note however that types derived from `Signal.Interrupt' still have a
 * minor special behavior in that a thread callback exiting by throwing
 * an instance of it or a derived class will not cause the thread to be
 * considered having crashed, but rather having exited normally, and
 * returning `none'.
 * WARNING: Be careful how you use interrupts, because the runtime will
 *          attempt to broadcast a `Signal.Interrupt' when trying to
 *          terminate any thread that is still running during shutdown. */
DDATDEF DeeTypeObject     DeeError_Interrupt;
DDATDEF DeeTypeObject         DeeError_KeyboardInterrupt;
DDATDEF DeeTypeObject         DeeError_ThreadExit;
DDATDEF DeeTypeObject     DeeError_StopIteration;
DDATDEF DeeTypeObject DeeError_Error;
DDATDEF DeeTypeObject     DeeError_AttributeError;
DDATDEF DeeTypeObject         DeeError_UnboundAttribute;
DDATDEF DeeTypeObject     DeeError_CompilerError;
DDATDEF DeeTypeObject         DeeError_SyntaxError;
DDATDEF DeeTypeObject         DeeError_SymbolError;
DDATDEF DeeTypeObject     DeeError_ThreadCrash;
DDATDEF DeeTypeObject     DeeError_NoMemory;
DDATDEF DeeTypeObject     DeeError_RuntimeError;
DDATDEF DeeTypeObject         DeeError_NotImplemented;
DDATDEF DeeTypeObject         DeeError_AssertionError;
DDATDEF DeeTypeObject         DeeError_UnboundLocal;
DDATDEF DeeTypeObject         DeeError_StackOverflow;
DDATDEF DeeTypeObject         DeeError_SegFault;
DDATDEF DeeTypeObject         DeeError_IllegalInstruction;
DDATDEF DeeTypeObject     DeeError_TypeError;
DDATDEF DeeTypeObject     DeeError_ValueError;
DDATDEF DeeTypeObject         DeeError_Arithmetic;
DDATDEF DeeTypeObject             DeeError_IntegerOverflow;
DDATDEF DeeTypeObject             DeeError_DivideByZero;
DDATDEF DeeTypeObject             DeeError_NegativeShift;
DDATDEF DeeTypeObject         DeeError_KeyError;
DDATDEF DeeTypeObject         DeeError_IndexError;
DDATDEF DeeTypeObject             DeeError_UnboundItem;
DDATDEF DeeTypeObject         DeeError_SequenceError;
DDATDEF DeeTypeObject         DeeError_UnicodeError;
DDATDEF DeeTypeObject             DeeError_UnicodeDecodeError;
DDATDEF DeeTypeObject             DeeError_UnicodeEncodeError;
DDATDEF DeeTypeObject         DeeError_ReferenceError;
DDATDEF DeeTypeObject         DeeError_UnpackError;
DDATDEF DeeTypeObject         DeeError_BufferError;
DDATDEF DeeTypeObject     DeeError_SystemError;
DDATDEF DeeTypeObject         DeeError_UnsupportedAPI;
DDATDEF DeeTypeObject         DeeError_FSError;
DDATDEF DeeTypeObject             DeeError_AccessError;
DDATDEF DeeTypeObject                 DeeError_ReadOnly;
DDATDEF DeeTypeObject             DeeError_FileNotFound;
DDATDEF DeeTypeObject             DeeError_FileExists;
DDATDEF DeeTypeObject             DeeError_HandleClosed;

/* A very special error type that doesn't actually derive
 * from `Error', or even `object' for that matter.
 * It does however have the `TP_FINTERRUPT' flag set,
 * meaning that it can only be caught by interrupt-enabled
 * exception handlers.
 * The main purpose of this error is to allow user-code to
 * throw it (the type is accessible as `(Error from deemon).AppExit'),
 * while also providing for proper stack unwinding and correct
 * destruction of all existing objects.
 * The implementation's main() function should then terminate
 * by returning the contained `ae_exitcode' value.
 * Note that this type is `final', meaning that
 * user-classes cannot be further derived from it.
 * Additionally, this type of error is used by the builtin implementation
 * of `exit()' when deemon was built with `CONFIG_NO_STDLIB'. */
DDATDEF DeeTypeObject DeeError_AppExit;
struct appexit_object { OBJECT_HEAD int ae_exitcode; };
#define DeeAppExit_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeError_AppExit)
#define DeeAppExit_Exitcode(ob) ((struct appexit_object *)REQUIRES_OBJECT(ob))->ae_exitcode

struct threadexit_object { OBJECT_HEAD DREF DeeObject *te_result; };
#define DeeThreadExit_Check(ob)    DeeObject_InstanceOfExact(ob,&DeeError_ThreadExit)
#define DeeThreadExit_Result(ob) ((struct threadexit_object *)REQUIRES_OBJECT(ob))->te_result

struct string_object;

/* Object header for types derived from `DeeError_Error'
 * Note that types derived from `DeeError_Signal' don't have any special header.
 * Special object heads for other error types can be found in `error_types.h' */
#define ERROR_OBJECT_HEAD \
    OBJECT_HEAD \
    DREF struct string_object *e_message; /* [0..1][const] Error message string. \
                                           * NOTE: May be substituted with, or extended by \
                                           *       the `__str__' operators of sub-classes). */ \
    DREF DeeObject            *e_inner;   /* [0..1][const] Inner error object. */
struct error_object { ERROR_OBJECT_HEAD };



#ifndef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
DDATDEF DeeObject DeeError_NoMemory_instance;
DDATDEF DeeObject DeeError_StopIteration_instance;
DDATDEF DeeObject DeeError_Interrupt_instance;
#endif /* !GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */

/* Try to catch (and thereby handle) an instance of `type',
 * returning true if doing so was possible.
 * Upon success, the actual error object thrown is discarded during this process. */
DFUNDEF bool DCALL DeeError_Catch(DeeTypeObject *__restrict type);

/* Throw a given object `ob' as an error.
 * @return: -1: Always returns `-1' */
DFUNDEF int DCALL DeeError_Throw(DeeObject *__restrict ob);

/* Throw a new error of type `tp', using a printf-formatted
 * message passed through `format' and varargs.
 * @return: -1: Always returns `-1'*/
DFUNDEF int DeeError_Throwf(DeeTypeObject *__restrict tp, char const *__restrict format, ...);
DFUNDEF int DCALL DeeError_VThrowf(DeeTypeObject *__restrict tp, char const *__restrict format, va_list args);
DFUNDEF int DeeError_SysThrowf(DeeTypeObject *__restrict tp, syserrno_t error, char const *__restrict format, ...);
DFUNDEF int DCALL DeeError_VSysThrowf(DeeTypeObject *__restrict tp, syserrno_t error, char const *__restrict format, va_list args);

/* Return the currently effective error, or NULL if none is. */
DFUNDEF DeeObject *DCALL DeeError_Current(void);

/* Handle an error and print it, alongside a human-readable message to `stderr'
 * @param: handle_errors: Describes how (if at all) errors should be handled.
 * @param: reason: When non-NULL, a message explaining the reason for the exception is being handled. */
DFUNDEF bool DCALL DeeError_Print(char const *reason, unsigned int handle_errors);
#define ERROR_PRINT_DONTHANDLE 0
#define ERROR_PRINT_DOHANDLE   1
#ifdef CONFIG_NO_THREADS
#define ERROR_PRINT_HANDLEINTR ERROR_PRINT_DOHANDLE
#else
#define ERROR_PRINT_HANDLEINTR 2
#endif

/* Display (print to stderr) an error, as well as an optional traceback. */
DFUNDEF void DCALL DeeError_Display(char const *reason, DeeObject *__restrict error, DeeObject *traceback);

/* Handle the current error, discarding it in the process.
 * @param: mode:   One of `ERROR_HANDLED_*'
 * @return: true:  The current error was handled.
 * @return: false: No error could be handled. */
#ifdef CONFIG_NO_THREADS
DFUNDEF bool (DCALL DeeError_HandledNoSMP)(void);
#define DeeError_Handled(mode) DeeError_HandledNoSMP()
#else
DFUNDEF bool (DCALL DeeError_Handled)(unsigned int mode);
#endif
#define ERROR_HANDLED_NORMAL    0x0000 /* Only handle non-interrupt exceptions (return `false' if the current error is an interrupt). */
#define ERROR_HANDLED_RESTORE   0x0001 /* Handle non-interrupt exceptions normally, and re-schedule interrupt exceptions
                                        * to-be delivered once again the next time `DeeThread_CheckInterrupt()' is called. */
#define ERROR_HANDLED_INTERRUPT 0x0002 /* Handle any type of error, including interrupts. */


#define DERROR_NOTIMPLEMENTED() \
  DeeError_Throwf(&DeeError_NotImplemented,"Not implemented: %s",__FUNCTION__)


#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
/* Install the keyboard interrupt handler. */
INTDEF void DCALL DeeError_InstallKeyboardInterrupt(void);
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */
#endif


DECL_END

#endif /* !GUARD_DEEMON_ERROR_H */
