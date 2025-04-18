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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCE_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCE_H 1

#include <deemon/api.h>
#include <deemon/object.h>
/**/

#include "../generic-proxy.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(dssgi_seq)  /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `dssgi_seq'.
	 * This is either a `tp_getitem_index' or `tp_getitem_index_fast' operator. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dssgi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          dssgi_start; /* [const] Starting index for enumeration. */
	size_t          dssgi_end;   /* [const] Enumeration stop index. */
} DefaultSequence_WithSizeAndGetItemIndex;

typedef struct {
	PROXY_OBJECT_HEAD3(dssg_seq,   /* [1..1][const] The sequence being iterated. */
	                   dssg_start, /* [1..1][const] Starting index for enumeration. */
	                   dssg_end)   /* [1..1][const] Enumeration stop index. */
	/* [1..1][const] Callback to load the `index'th element of `dssg_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *dssg_tp_getitem)(DeeObject *self, DeeObject *index);
} DefaultSequence_WithSizeObAndGetItem;

typedef struct {
	PROXY_OBJECT_HEAD(dsi_seq) /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to construct an iterator for `dsi_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dsi_tp_iter)(DeeObject *self);
} DefaultSequence_WithIter;

typedef struct {
	PROXY_OBJECT_HEAD(dsial_seq) /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to construct an iterator for `dsial_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dsial_tp_iter)(DeeObject *self);
	size_t          dsial_start; /* [const] # of items to skip in constructed iterators. */
	size_t          dsial_limit; /* [const] Max # of items to enumerate starting with `dsial_start' */
} DefaultSequence_WithIterAndLimit;

INTDEF DeeTypeObject DefaultSequence_WithSizeAndGetItemIndex_Type;     /* DefaultSequence_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultSequence_WithSizeAndGetItemIndexFast_Type; /* DefaultSequence_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultSequence_WithSizeAndTryGetItemIndex_Type;  /* DefaultSequence_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultSequence_WithSizeObAndGetItem_Type;        /* DefaultSequence_WithSizeObAndGetItem */
INTDEF DeeTypeObject DefaultSequence_WithIter_Type;                    /* DefaultSequence_WithIter */
INTDEF DeeTypeObject DefaultSequence_WithIterAndLimit_Type;            /* DefaultSequence_WithIterAndLimit */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCE_H */
