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
#ifndef GUARD_DEX_CTYPES_CTYPES_C
#define GUARD_DEX_CTYPES_CTYPES_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
#include <deemon/api.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/float.h>

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) CType *DCALL
CType_FromDeemonType(DeeTypeObject *__restrict self) {
	/* Map some builtin types to their structured counterparts. */
	if (DeeNone_Check(self) || self == &DeeNone_Type)
		return &CVoid_Type;
	if (self == &DeeBool_Type)
		return &CBool_Type;
	if (self == &DeeInt_Type)
		return &CInt_Type;
	if (self == &DeeFloat_Type)
		return &CDouble_Type;
	return NULL;
}

/* Return the structured type equivalent of `self', or
 * re-return `self' if it already is a structured type.
 * The following types found in the builtin `deemon' module are mapped:
 *   - `none from deemon'       --> `void from ctypes'
 *   - `type(none from deemon)' --> `void from ctypes'
 *   - `bool from deemon'       --> `bool from ctypes'
 *   - `int from deemon'        --> `int from ctypes'
 *   - `float from deemon'      --> `double from ctypes'
 * If `self' is not one of these mappings and also not
 * a c-type, a TypeError is thrown and NULL is returned.
 * WARNING: This function does not return a reference! */
INTERN WUNUSED NONNULL((1)) CType *DCALL
CType_Of(DeeObject *__restrict self) {
	CType *result;
	if (Object_IsCType(self))
		return Object_AsCType(self);

	/* Map some builtin types to their structured counterparts. */
	result = CType_FromDeemonType((DeeTypeObject *)self);
	if (result)
		return result;

	/* Throw a type-assertion failure error. */
	DeeObject_TypeAssertFailed(self, &CType_Type);
	return NULL;
}


/* Same as `DeeSType_Get()', but also able to handle the
 * case where "self" is an *instance*, rather a some type. */
INTERN WUNUSED NONNULL((1)) CType *DCALL
CType_TypeOf(DeeObject *__restrict self) {
	DeeSTypeObject *result;
	if (Object_IsCType(self))
		return Object_AsCType(self);
	if (Object_IsCObject(self))
		return Dee_TYPE(Object_AsCObject(self));

	/* Map some builtin types to their structured counterparts. */
	result = CType_FromDeemonType((DeeTypeObject *)self);
	if (result)
		return result;
	result = CType_FromDeemonType(Dee_TYPE(self));
	if (result)
		return result;

	/* Throw a type-assertion failure error. */
	DeeObject_TypeAssertFailed(self, &CType_Type);
	return NULL;
}

DECL_END
#endif /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */

#endif /* !GUARD_DEX_CTYPES_CTYPES_C */
