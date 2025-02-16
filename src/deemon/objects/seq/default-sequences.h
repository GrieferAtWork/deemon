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

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dssgi_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `dssgi_seq'.
	 * This is either a `tp_getitem_index' or `tp_getitem_index_fast' operator. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dssgi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          dssgi_start; /* [const] Starting index for enumeration. */
	size_t          dssgi_end;   /* [const] Enumeration stop index. */
} DefaultSequence_WithSizeAndGetItemIndex;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dssg_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `dssg_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *dssg_tp_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject *dssg_start; /* [1..1][const] Starting index for enumeration. */
	DREF DeeObject *dssg_end;   /* [1..1][const] Enumeration stop index. */
} DefaultSequence_WithSizeAndGetItem;

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dstsg_seq;    /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `dstsg_seq'. */
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *dstsg_tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
	DREF DeeObject *dstsg_start;  /* [1..1][const] Starting index for enumeration. */
	DREF DeeObject *dstsg_end;    /* [1..1][const] Enumeration stop index. */
	DeeTypeObject  *dstsg_tp_seq; /* [1..1][const] The type to pass to `dstsg_tp_tgetitem'. */
} DefaultSequence_WithTSizeAndGetItem;
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dsi_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to construct an iterator for `dsi_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dsi_tp_iter)(DeeObject *self);
} DefaultSequence_WithIter;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dsial_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to construct an iterator for `dsial_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *dsial_tp_iter)(DeeObject *self);
	size_t          dsial_start; /* [const] # of items to skip in constructed iterators. */
	size_t          dsial_limit; /* [const] Max # of items to enumerate starting with `dsial_start' */
} DefaultSequence_WithIterAndLimit;

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dsti_seq;    /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to construct an iterator for `dsti_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *dsti_tp_titer)(DeeTypeObject *tp_self, DeeObject *self);
	size_t          dsti_start;  /* [const] # of items to skip in constructed iterators. */
	size_t          dsti_limit;  /* [const] Max # of items to enumerate starting with `dsti_start' (but enumeration may stop earlier than that) */
	DeeTypeObject  *dsti_tp_seq; /* [1..1][const] The type to pass to `dsti_tp_titer'. */
} DefaultSequence_WithTIterAndLimit;
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

INTDEF DeeTypeObject DefaultSequence_WithSizeAndGetItemIndex_Type;     /* DefaultSequence_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultSequence_WithSizeAndGetItemIndexFast_Type; /* DefaultSequence_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultSequence_WithSizeAndTryGetItemIndex_Type;  /* DefaultSequence_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultSequence_WithSizeAndGetItem_Type;          /* DefaultSequence_WithSizeAndGetItem */
INTDEF DeeTypeObject DefaultSequence_WithIter_Type;                    /* DefaultSequence_WithIter */
INTDEF DeeTypeObject DefaultSequence_WithIterAndLimit_Type;            /* DefaultSequence_WithIterAndLimit */
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
INTDEF DeeTypeObject DefaultSequence_WithTSizeAndGetItem_Type;         /* DefaultSequence_WithTSizeAndGetItem */
INTDEF DeeTypeObject DefaultSequence_WithTIterAndLimit_Type;           /* DefaultSequence_WithTIterAndLimit */
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCE_H */
