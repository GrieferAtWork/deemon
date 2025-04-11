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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_H
#define GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_H 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/attribute.h>
#include <deemon/object.h>
/**/

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(mfa_ob) /* [1..1][const] The object whose attributes are being enumerated */
} MapFromAttr;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject     *mfai_obj;  /* [1..1][const] The object whose attributes are being enumerated. */
	size_t              mfai_itsz; /* [const] Size of "ei_iter" (in bytes) */
	struct Dee_attriter mfai_iter; /* Attribute enumerator. */
} MapFromAttrIterator;

INTDEF DeeTypeObject MapFromAttr_Type;             /* type(Mapping.fromattr(ob)); */
INTDEF DeeTypeObject MapFromAttrKeysIterator_Type; /* type(Mapping.fromattr(ob).__map_iterkeys__()); */

LOCAL WUNUSED NONNULL((1)) DREF MapFromAttr *DCALL
MapFromAttr_New(DeeObject *__restrict ob) {
	DREF MapFromAttr *result = DeeObject_MALLOC(MapFromAttr);
	if likely(result) {
		result->mfa_ob = ob;
		Dee_Incref(ob);
		DeeObject_Init(result, &MapFromAttr_Type);
	}
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_H */
