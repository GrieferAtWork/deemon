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
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
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
	&DeeRoDict_Type,
	&DeeRoSet_Type,
	&DeeAttribute_Type,
};

/* Returns `true' if operator `name' of `self' can be invoked without unintended
 * side-effects, which includes the possibility of other threads accessing any
 * an instance of the type at the same time, which must *NOT* affect the result
 * of the operator being invoked (iow: `List.operator +' is not constexpr). */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_IsOperatorConstexpr(DeeTypeObject const *__restrict self,
                            uint16_t name) {
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



/* Check if C-method attached to objects are constant expressions. */

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_IsGetMethodConstexpr(DeeTypeObject const *__restrict self,
                             Dee_getmethod_t getter) {
	/* TODO: Search for getters that are constant expressions.
	 *       Note that a getter that returns a writable field
	 *       is NOT a constant expression! Only read-only, or
	 *       write-once fields (that have already been assigned)
	 *       can be considered constant expressions! */
	(void)self;
	(void)getter;
	return false;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_IsBoundMethodConstexpr(DeeTypeObject const *__restrict self,
                               Dee_boundmethod_t bound) {
	/* TODO: see `DeeType_IsGetMethodConstexpr()' */
	(void)self;
	(void)bound;
	return false;
}

INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL /* Also usable for Dee_kwobjmethod_t */
DeeType_IsObjMethodConstexpr(DeeTypeObject const *__restrict self,
                             Dee_objmethod_t method) {
	/* TODO: see `DeeType_IsGetMethodConstexpr()' */
	(void)self;
	(void)method;
	return false;
}

INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL /* Also usable for Dee_kwcmethod_t */
DeeCMethod_IsConstExpr(Dee_cmethod_t method,
                       DeeTypeObject const *type,
                       struct Dee_module_object const *mod) {
	/* TODO: see `DeeType_IsGetMethodConstexpr()' */
	(void)method;
	(void)type;
	(void)mod;
	return false;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_TRAITS_C */
