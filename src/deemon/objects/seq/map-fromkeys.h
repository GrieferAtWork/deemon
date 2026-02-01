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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMKEYS_H
#define GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMKEYS_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>  /* DeeObject_MALLOC */
#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject, Dee_Incref */
#include <deemon/type.h>   /* DeeObject_Init */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD2(mfk_keys,  /* [1..1][const] Mapping keys. */
	                   mfk_value) /* [1..1][const] Value for all keys, or callable to produce value */
} MapFromKeys;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,   mfki_iter, /* [1..1][const] Base map-from-keys object. */
	                      MapFromKeys, mfki_base) /* [1..1][const] Set-Iterator for mfki_base->mfk_keys */
} MapFromKeysIterator;

INTDEF DeeTypeObject MapFromKeysAndValue_Type;            /* type(Mapping.fromkeys(keys, value: v)); */
INTDEF DeeTypeObject MapFromKeysAndCallback_Type;         /* type(Mapping.fromkeys(keys, valuefor: cb)); */
INTDEF DeeTypeObject MapFromKeysAndValueIterator_Type;    /* type(Mapping.fromkeys(keys, value: v).operator iter()); */
INTDEF DeeTypeObject MapFromKeysAndCallbackIterator_Type; /* type(Mapping.fromkeys(keys, valuefor: cb).operator iter()); */

LOCAL WUNUSED NONNULL((1, 2)) DREF MapFromKeys *DCALL
MapFromKeysAndValue_New(DeeObject *keys, DeeObject *value) {
	DREF MapFromKeys *result = DeeObject_MALLOC(MapFromKeys);
	if likely(result) {
		result->mfk_keys  = keys;
		result->mfk_value = value;
		Dee_Incref(keys);
		Dee_Incref(value);
		DeeObject_Init(result, &MapFromKeysAndValue_Type);
	}
	return result;
}

LOCAL WUNUSED NONNULL((1, 2)) DREF MapFromKeys *DCALL
MapFromKeysAndCallback_New(DeeObject *keys, DeeObject *valuefor) {
	DREF MapFromKeys *result = DeeObject_MALLOC(MapFromKeys);
	if likely(result) {
		result->mfk_keys  = keys;
		result->mfk_value = valuefor;
		Dee_Incref(keys);
		Dee_Incref(valuefor);
		DeeObject_Init(result, &MapFromKeysAndCallback_Type);
	}
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMKEYS_H */
