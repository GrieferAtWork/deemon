/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HOSTASM_LIBHOSTASM_C
#define GUARD_DEX_HOSTASM_LIBHOSTASM_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#include <deemon/arg.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/objmethod.h>
#include <deemon/tuple.h>

DECL_BEGIN

#ifdef CONFIG_HAVE_LIBHOSTASM
#if 0 /* Only here to test that unwinding works correctly (don't use c++ exceptions with deemon). */
#define HAVE_test_throw
PRIVATE DREF DeeObject *DCALL
test_throw(size_t argc, DeeObject *const *argv) {
	(void)argv;
	throw argc;
}
#endif

PRIVATE DREF DeeObject *DCALL
test_compile_and_run(size_t argc, DeeObject *const *argv) {
	struct hostfunc hfunc;
	DREF DeeObject *result;
	DeeFunctionObject *func;
	DeeTupleObject *args = (DeeTupleObject *)Dee_EmptyTuple;
	if (DeeArg_Unpack(argc, argv, "o|o:test_compile_and_run", &func, &args))
		goto err;
	if (DeeObject_AssertTypeExact(func, &DeeFunction_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;

	/* TODO: On i386, the argc/argv/kw/... stack locations should be usable
	 *       for hstack allocation once the respective arguments are no longer
	 *       used (just like how argument registers can get re-used on x86_64
	 *       once no longer needed) */

	/* Assemble the function. */
	if unlikely(hostfunc_assemble(func, func->fo_code, &hfunc,
	                              HOST_CC_CALL,
	                              FUNCTION_ASSEMBLER_F_NORMAL))
		goto err;

	/* Call the function. */
#ifdef HAVE_test_throw
	try
#endif /* HAVE_test_throw */
	{
		result = (*hfunc.hf_raw.rhf_entry.hfe_call)(DeeTuple_SIZE(args),
		                                            DeeTuple_ELEM(args));
	}
#ifdef HAVE_test_throw
	catch (...) {
		HA_printf("caught!\n");
		DeeError_Throwf(&DeeError_ValueError, "Exception caught");
		result = NULL;
	}
#endif /* HAVE_test_throw */

	hostfunc_fini(&hfunc);
	return result;
err:
	return NULL;
}

DEFINE_CMETHOD(test_compile_and_run_o, &test_compile_and_run);
#ifdef HAVE_test_throw
DEFINE_CMETHOD(test_throw_o, &test_throw);
#endif /* HAVE_test_throw */
#endif /* CONFIG_HAVE_LIBHOSTASM */

PRIVATE struct dex_symbol symbols[] = {
#ifdef CONFIG_HAVE_LIBHOSTASM
	{ "test_compile_and_run", (DeeObject *)&test_compile_and_run_o, MODSYM_FREADONLY,
	  DOC("(func:?DFunction,args=!T0)->") },
#ifdef HAVE_test_throw
	{ "test_throw", (DeeObject *)&test_throw_o, MODSYM_FREADONLY,
	  DOC("()") },
#endif /* HAVE_test_throw */
#endif /* CONFIG_HAVE_LIBHOSTASM */
	 /* TODO: Proper API that allows you to re-compile deemon.Function and deemon.Code objects,
	  *       and be given their compiled equivalents (`test_compile_and_run()' will *not* stay
	  *       and is only here to test re-compiling and running code) */
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols,
	/* .d_init    = */ NULL,
	/* .d_fini    = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_HOSTASM_LIBHOSTASM_C */
