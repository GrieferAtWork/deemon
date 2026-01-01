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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_H 1

#include <deemon/api.h>
#include <deemon/object.h>
/**/

#include <stddef.h> /* size_t */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(drwgii_seq) /* [1..1][const] The sequence to reverse */
	/* [1..1][const] The `tp_getitem_index', `tp_getitem_index_fast' or `tp_trygetitem_index' operator of `drwgii_seq' */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *drwgii_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          drwgii_max;   /* [const] Index of the 0'th element of the reversed of `drwgii_seq' */
	size_t          drwgii_size;  /* [const][<= (drwgii_max + 1)] Length of the reversed sequence. */
} DefaultReversed_WithGetItemIndex;

INTDEF DeeTypeObject DefaultReversed_WithGetItemIndex_Type;     /* DefaultReversed_WithGetItemIndex */
INTDEF DeeTypeObject DefaultReversed_WithGetItemIndexFast_Type; /* DefaultReversed_WithGetItemIndex */
INTDEF DeeTypeObject DefaultReversed_WithTryGetItemIndex_Type;  /* DefaultReversed_WithGetItemIndex */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_H */
