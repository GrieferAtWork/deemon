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
#ifndef GUARD_DEEMON_ATTRIBUTE_H
#define GUARD_DEEMON_ATTRIBUTE_H 1

#include "api.h"
/**/

#include "types.h"
#include "mro.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_attribute_object         attribute_object
#define Dee_enumattr_object          enumattr_object
#define Dee_enumattr_iterator_object enumattr_iterator_object
#endif /* DEE_SOURCE */

typedef struct Dee_attribute_object DeeAttributeObject;
typedef struct Dee_enumattr_object DeeEnumAttrObject;
typedef struct Dee_enumattr_iterator_object DeeEnumAttrIteratorObject;

struct Dee_attribute_object {
	Dee_OBJECT_HEAD
	struct Dee_attrdesc a_desc; /* [OVERRIDE(.ad_info.ai_decl, [DREF])][const] Attribute descriptor. */
};

struct Dee_enumattr_object {
	Dee_OBJECT_HEAD
	DREF DeeObject     *ea_obj;  /* [1..1][const] The object whose attributes are being enumerated. */
	struct Dee_attrhint ea_hint; /* [OVERRIDE(.ah_decl, [DREF])][const] Filter for attributes matching this hint */
};

struct Dee_enumattr_iterator_object {
	Dee_OBJECT_HEAD
	DREF DeeEnumAttrObject *ei_seq;  /* [1..1][const] The underlying enumattr() controller. */
	size_t                  ei_itsz; /* [const] Size of "ei_iter" (in bytes) */
	struct Dee_attriter     ei_iter; /* Attribute enumerator. */
};

DDATDEF DeeTypeObject DeeAttribute_Type;        /* `Attribute from deemon' */
DDATDEF DeeTypeObject DeeEnumAttr_Type;         /* `enumattr from deemon' */
DDATDEF DeeTypeObject DeeEnumAttrIterator_Type; /* `(enumattr from deemon).Iterator' */
#define DeeEnumAttr_Check(x)      DeeObject_InstanceOfExact(x, &DeeEnumAttr_Type) /* `enumattr' is final */
#define DeeEnumAttr_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeEnumAttr_Type)

DECL_END

#endif /* !GUARD_DEEMON_ATTRIBUTE_H */
