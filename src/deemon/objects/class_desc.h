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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_DESC_H
#define GUARD_DEEMON_OBJECTS_CLASS_DESC_H 1

#include <deemon/api.h>

#include <deemon/class.h>  /* DeeClassDescriptorObject, Dee_class_attribute, Dee_class_operator, Dee_instance_desc */
#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject, Dee_AsObject */

#include "generic-proxy.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

typedef struct {
	/* A mapping-like {(string | int, int)...} object used for mapping
	 * operator names to their respective class instance table slots. */
	PROXY_OBJECT_HEAD_EX(DeeClassDescriptorObject, co_desc) /* [1..1][const] The referenced class descriptor. */
} ClassOperatorTable;

#define ClassOperatorTable_New(self) \
	((DREF ClassOperatorTable *)ProxyObject_New(&ClassOperatorTable_Type, Dee_AsObject(self)))


typedef struct {
	/* A mapping-like {(string | int, int)...} object used for mapping
	 * operator names to their respective class instance table slots. */
	PROXY_OBJECT_HEAD_EX(DeeClassDescriptorObject, co_desc) /* [1..1][const] The referenced class descriptor. */
	DWEAK struct Dee_class_operator               *co_iter; /* [1..1][lock(ATOMIC)] Current iterator position. */
	struct Dee_class_operator                     *co_end;  /* [1..1][const] Iterator end position. */
} ClassOperatorTableIterator;

INTDEF DeeTypeObject ClassOperatorTableIterator_Type;
INTDEF DeeTypeObject ClassOperatorTable_Type;





typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeClassDescriptorObject, ca_desc) /* [1..1][const] Class descriptor. */
	struct Dee_class_attribute const              *ca_attr; /* [1..1][const] The attribute that was queried. */
} ClassAttribute;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeClassDescriptorObject, ca_desc) /* [1..1][const] Class descriptor. */
	DWEAK struct Dee_class_attribute const        *ca_iter; /* [1..1] Current iterator position. */
	struct Dee_class_attribute const              *ca_end;  /* [1..1][const] Iterator end. */
} ClassAttributeTableIterator;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeClassDescriptorObject, ca_desc)  /* [1..1][const] Class descriptor. */
	struct Dee_class_attribute const              *ca_start; /* [1..1][const] Hash-vector starting pointer. */
	size_t                                         ca_mask;  /* [const] Mask-vector size mask. */
} ClassAttributeTable;

INTDEF DeeTypeObject ClassAttribute_Type;
INTDEF DeeTypeObject ClassAttributeTable_Type;
INTDEF DeeTypeObject ClassAttributeTableIterator_Type;




typedef struct {
	PROXY_OBJECT_HEAD(ot_owner)        /* [1..1][const] The associated owner object.
	                                    * NOTE: This may be a super-object, in which case the referenced
	                                    *       object table refers to the described super-type. */
	struct Dee_instance_desc *ot_desc; /* [1..1][valid_if(ot_size != 0)][const] The referenced instance descriptor. */
	uint16_t                  ot_size; /* [const] The length of the object table contained within `ot_desc' */
} ObjectTable;

INTDEF DeeTypeObject ObjectTable_Type;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_ctable(DeeTypeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_DESC_H */
