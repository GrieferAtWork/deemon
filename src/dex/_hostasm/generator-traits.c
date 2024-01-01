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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_TRAITS_C
#define GUARD_DEX_HOSTASM_GENERATOR_TRAITS_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/bool.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

DECL_BEGIN

PRIVATE DeeTypeObject *tpconst always_constexpr_types[] = {
	/* Immutable builtin types implement all operators with constexpr semantics. */
	&DeeNone_Type,
	&DeeString_Type,
	&DeeFloat_Type,
	&DeeBool_Type,
	&DeeInt_Type,
	&DeeTuple_Type,
};

/* Returns `true' if operator `name' of `self' can be invoked without unintended
 * side-effects, which includes the possibility of other threads accessing any
 * an instance of the type at the same time, which must *NOT* affect the result
 * of the operator being invoked (iow: `List.operator +' is not constexpr). */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_IsOperatorConstexpr(DeeTypeObject const *__restrict self, uint16_t name) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(always_constexpr_types); ++i) {
		if (always_constexpr_types[i] == self)
			goto yes;
	}

	(void)name;

	return false;
yes:
	return true;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_TRAITS_C */
