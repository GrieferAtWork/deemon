/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_EXIT_C_INL
#define GUARD_DEX_POSIX_P_EXIT_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-exit-constants.def");

local allDecls = [];

function egii(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc);
}

egii("EXIT_SUCCESS");
egii("EXIT_FAILURE");


File.stdout = orig_stdout;
print "#define POSIX_EXIT_DEFS \\";
for (local x: allDecls)
	print("\tPOSIX_", x, "_DEF \\");
print "/" "**" "/";

]]]*/
#include "p-exit-constants.def"
#define POSIX_EXIT_DEFS \
	POSIX_EXIT_SUCCESS_DEF \
	POSIX_EXIT_FAILURE_DEF \
/**/
/*[[[end]]]*/




/************************************************************************/
/* atexit()                                                             */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("atexit", "callback:?DCallable,args:?DTuple=Dee_EmptyTuple", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_atexit_f_impl(DeeObject *callback, DeeObject *args);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_atexit_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ATEXIT_DEF { "atexit", (DeeObject *)&posix_atexit, MODSYM_FREADONLY, DOC("(callback:?DCallable,args=!T0)") },
#define POSIX_ATEXIT_DEF_DOC(doc) { "atexit", (DeeObject *)&posix_atexit, MODSYM_FREADONLY, DOC("(callback:?DCallable,args=!T0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_atexit, &posix_atexit_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__callback_args
#define DEFINED_kwlist__callback_args
PRIVATE DEFINE_KWLIST(kwlist__callback_args, { KEX("callback", 0x3b9dd39e, 0x1e7dd8df6e98f4c6), KEX("args", 0xc6fc997f, 0x4af7cd17f976719e), KEND });
#endif /* !DEFINED_kwlist__callback_args */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_atexit_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *callback;
		DeeObject *args;
	} args;
	args.args = Dee_EmptyTuple;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__callback_args, "o|o:atexit", &args))
		goto err;
	return posix_atexit_f_impl(args.callback, args.args);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1, 2))DREF DeeObject *DCALL posix_atexit_f_impl(DeeObject *callback, DeeObject *args)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (Dee_AtExit(callback, args))
		goto err;
	return_none;
err:
	return NULL;
}





/************************************************************************/
/* exit()                                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("exit", "exitcode:d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_exit_f_impl(int exitcode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_exit_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_EXIT_DEF { "exit", (DeeObject *)&posix_exit, MODSYM_FREADONLY, DOC("(exitcode:?Dint)") },
#define POSIX_EXIT_DEF_DOC(doc) { "exit", (DeeObject *)&posix_exit, MODSYM_FREADONLY, DOC("(exitcode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_exit, &posix_exit_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__exitcode
#define DEFINED_kwlist__exitcode
PRIVATE DEFINE_KWLIST(kwlist__exitcode, { KEX("exitcode", 0x79c9d863, 0x3fcba2655ec7fa85), KEND });
#endif /* !DEFINED_kwlist__exitcode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_exit_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		int exitcode;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__exitcode, "d:exit", &args))
		goto err;
	return posix_exit_f_impl(args.exitcode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_exit_f_impl(int exitcode)
/*[[[end]]]*/
{
	Dee_Exit(exitcode, true);
	return NULL;
}





/************************************************************************/
/* _Exit()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_Exit", "exitcode:d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__Exit_f_impl(int exitcode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix__Exit_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX__EXIT_DEF { "_Exit", (DeeObject *)&posix__Exit, MODSYM_FREADONLY, DOC("(exitcode:?Dint)") },
#define POSIX__EXIT_DEF_DOC(doc) { "_Exit", (DeeObject *)&posix__Exit, MODSYM_FREADONLY, DOC("(exitcode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix__Exit, &posix__Exit_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__exitcode
#define DEFINED_kwlist__exitcode
PRIVATE DEFINE_KWLIST(kwlist__exitcode, { KEX("exitcode", 0x79c9d863, 0x3fcba2655ec7fa85), KEND });
#endif /* !DEFINED_kwlist__exitcode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__Exit_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		int exitcode;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__exitcode, "d:_Exit", &args))
		goto err;
	return posix__Exit_f_impl(args.exitcode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__Exit_f_impl(int exitcode)
/*[[[end]]]*/
{
	Dee_Exit(exitcode, false);
	return NULL;
}





/************************************************************************/
/* abort()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("abort", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_abort_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_abort_f(size_t argc, DeeObject *const *argv);
#define POSIX_ABORT_DEF { "abort", (DeeObject *)&posix_abort, MODSYM_FREADONLY, DOC("()") },
#define POSIX_ABORT_DEF_DOC(doc) { "abort", (DeeObject *)&posix_abort, MODSYM_FREADONLY, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_abort, &posix_abort_f, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_abort_f(size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "abort");
	return posix_abort_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_abort_f_impl(void)
/*[[[end]]]*/
{
	Dee_Exit(EXIT_FAILURE, false);
	return NULL;
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_EXIT_C_INL */
