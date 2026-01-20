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
#ifndef GUARD_DEX_HOSTASM_API_C
#define GUARD_DEX_HOSTASM_API_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#include <deemon/alloc.h>

#include <stddef.h> /* NULL, offsetof */


/* Public API, as used by the deemon core in "code.c" */
#ifdef CONFIG_HAVE_LIBHOSTASM
DECL_BEGIN

STATIC_ASSERT_MSG(offsetof(struct hostfunc, hf_raw.rhf_entry.hfe_addr) == 0,
                  "The deemon core (in `code.c') expects the pointer-to-the entry "
                  "point to be located at the start of the hostfunc struct.");

/* Create a host assembly function for `code' */
EXPDEF WUNUSED NONNULL((2)) struct hostfunc *DCALL
hostasm_hostfunc_new(/*0..1*/ DeeFunctionObject *function,
                     /*1..1*/ DeeCodeObject *code,
                     host_cc_t cc);
EXPDEF NONNULL((1)) void DCALL
hostasm_hostfunc_destroy(struct hostfunc *func);

/* Create a host assembly function for `code' */
PUBLIC WUNUSED NONNULL((2)) struct hostfunc *DCALL
hostasm_hostfunc_new(/*0..1*/ DeeFunctionObject *function,
                     /*1..1*/ DeeCodeObject *code,
                     host_cc_t cc) {
	struct hostfunc *result;
	result = (struct hostfunc *)Dee_Malloc(sizeof(struct hostfunc));
	if unlikely(!result)
		goto err;
	if unlikely(hostfunc_assemble(function, code, result, cc,
	                              FUNCTION_ASSEMBLER_F_NORMAL))
		goto err_r;
	return result;
err_r:
	Dee_Free(result);
err:
	return NULL;
}

PUBLIC NONNULL((1)) void DCALL
hostasm_hostfunc_destroy(struct hostfunc *func) {
	hostfunc_fini(func);
	Dee_Free(func);
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_API_C */
