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
#ifndef GUARD_DEX_POSIX_P_EXIT_C_INL
#define GUARD_DEX_POSIX_P_EXIT_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_UnpackStructKw */
#include <deemon/dex.h>             /* DEXSYM_READONLY, DEX_MEMBER_F */
#include <deemon/exec.h>            /* Dee_AtExit, Dee_Exit */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>
#include <deemon/objmethod.h>       /*  */
#include <deemon/system-features.h> /* EXIT_FAILURE */
#include <deemon/tuple.h>           /* DeeTuple_Type, Dee_EmptyTuple */

#include <stdbool.h> /* false, true */
#include <stddef.h>  /* NULL, size_t */

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

/*[[[deemon import("rt.gen.dexutils").gw("atexit", """
	DeeObject *callback:?DCallable,
	DeeObject *args:?DTuple=Dee_EmptyTuple
""", libname: "posix"); ]]]*/
#define POSIX_ATEXIT_DEF          DEX_MEMBER_F("atexit", &posix_atexit, DEXSYM_READONLY, "(callback:?DCallable,args=!T0)"),
#define POSIX_ATEXIT_DEF_DOC(doc) DEX_MEMBER_F("atexit", &posix_atexit, DEXSYM_READONLY, "(callback:?DCallable,args=!T0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_atexit_f_impl(DeeObject *callback, DeeObject *args);
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
PRIVATE DEFINE_KWCMETHOD(posix_atexit, &posix_atexit_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_atexit_f_impl(DeeObject *callback, DeeObject *args)
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
#define POSIX_EXIT_DEF          DEX_MEMBER_F("exit", &posix_exit, DEXSYM_READONLY, "(exitcode:?Dint)"),
#define POSIX_EXIT_DEF_DOC(doc) DEX_MEMBER_F("exit", &posix_exit, DEXSYM_READONLY, "(exitcode:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_exit_f_impl(int exitcode);
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
PRIVATE DEFINE_KWCMETHOD(posix_exit, &posix_exit_f, METHOD_FNORMAL);
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
#define POSIX__EXIT_DEF          DEX_MEMBER_F("_Exit", &posix__Exit, DEXSYM_READONLY, "(exitcode:?Dint)"),
#define POSIX__EXIT_DEF_DOC(doc) DEX_MEMBER_F("_Exit", &posix__Exit, DEXSYM_READONLY, "(exitcode:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__Exit_f_impl(int exitcode);
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
PRIVATE DEFINE_KWCMETHOD(posix__Exit, &posix__Exit_f, METHOD_FNORMAL);
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
#define POSIX_ABORT_DEF          DEX_MEMBER_F("abort", &posix_abort, DEXSYM_READONLY, "()"),
#define POSIX_ABORT_DEF_DOC(doc) DEX_MEMBER_F("abort", &posix_abort, DEXSYM_READONLY, "()\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_abort_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_abort, &posix_abort_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_abort_f_impl(void)
/*[[[end]]]*/
{
#ifdef EXIT_FAILURE
	Dee_Exit(EXIT_FAILURE, false);
#else /* EXIT_FAILURE */
	Dee_Exit(1, false);
#endif /* !EXIT_FAILURE */
	return NULL;
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_EXIT_C_INL */
