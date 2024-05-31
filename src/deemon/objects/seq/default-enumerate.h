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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

/* Sequence enumerations can be used to enumerate the keys & items of a sequence.
 * These proxy types can be used in the following ways:
 *
 * Enumerate all items (bound keys only):
 * - >> local seq = ...;
 *   >> for (local key, value: seq.enumerate())
 *   >>     print f"{repr seq}[{repr key}] == {repr value}";
 *
 * Enumerate all items (bound keys only) within a specific range:
 * - >> local seq = ...;
 *   >> for (local key, value: seq.enumerate(start, end)) {
 *   >>     assert key >= start;
 *   >>     assert key < end;
 *   >>     print f"{repr seq}[{repr key}] == {repr value}";
 *   >> }
 *
 * Enumerate all items (including unbound keys):
 * - >> local seq = ...;
 *   >> seq.enumerate((key, value?) -> {
 *   >>     assert key >= start;
 *   >>     assert key < end;
 *   >>     print f"{repr seq}[{repr key}] == {value is bound ? repr value : "<unbound>"}";
 *   >> }, start, end);
 *
 */


typedef struct {
	/* Enumerate all keys within [:dewsgi_size] */
	OBJECT_HEAD
	DREF DeeObject  *defr_seq;    /* [1..1][const] The sequence being iterated. */
	struct type_seq *defr_tp_seq; /* [1..1][const] Sequence operators of `defr_seq' (used operators depend on "ob_type") */
} DefaultEnumeration_FullRange;

typedef struct {
	/* Enumerate all keys within [dewir_start:dewir_end] */
	OBJECT_HEAD
	DREF DeeObject  *dewir_seq;    /* [1..1][const] The sequence being iterated. */
	struct type_seq *dewir_tp_seq; /* [1..1][const] Sequence operators of `dewir_seq' (always has `tp_size' and one of `tp_getitem_index_fast', `tp_trygetitem_index', `tp_getitem_index') */
	size_t           dewir_start;  /* [const] Enumeration range start. */
	size_t           dewir_end;    /* [const] Enumeration range end. */
} DefaultEnumeration_WithIntRange;

typedef struct {
	/* Enumerate all keys within [dewr_start:dewr_end] */
	OBJECT_HEAD
	DREF DeeObject  *dewr_seq;    /* [1..1][const] The sequence being iterated. */
	struct type_seq *dewr_tp_seq; /* [1..1][const] Sequence operators of `dewr_seq' (always has `tp_size' and one of `tp_getitem_index_fast', `tp_trygetitem_index', `tp_getitem_index') */
	DREF DeeObject  *dewr_start; /* [1..1][const] Enumeration range start. */
	DREF DeeObject  *dewr_end;   /* [1..1][const] Enumeration range end. */
} DefaultEnumeration_WithRange;

INTDEF DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndexFast_Type;          /* DefaultEnumeration_FullRange:    de_wsagiif_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeAndTryGetItemIndex_Type;           /* DefaultEnumeration_FullRange:    de_wsatgii_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndex_Type;              /* DefaultEnumeration_FullRange:    de_wsagii_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeObAndGetItem_Type;                 /* DefaultEnumeration_FullRange:    de_wsoagi_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndexFastAndFilter_Type; /* DefaultEnumeration_WithIntRange: de_wsagiifaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeAndTryGetItemIndexAndFilter_Type;  /* DefaultEnumeration_WithIntRange: de_wsatgiiaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndexAndFilter_Type;     /* DefaultEnumeration_WithIntRange: de_wsagiiaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithSizeObAndGetItemAndFilter_Type;        /* DefaultEnumeration_WithRange:    de_wsoagiaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithGetItemIndex_Type;                     /* DefaultEnumeration_FullRange:    de_wgii_ */
INTDEF DeeTypeObject DefaultEnumeration_WithGetItemIndexAndFilter_Type;            /* DefaultEnumeration_WithIntRange: de_wgiiaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithGetItemAndFilter_Type;                 /* DefaultEnumeration_WithRange:    de_wgiaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithIterKeysAndTryGetItem_Type;            /* DefaultEnumeration_FullRange:    de_wikatgi_ */
INTDEF DeeTypeObject DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type;   /* DefaultEnumeration_WithRange:    de_wikatgiaf_ */
INTDEF DeeTypeObject DefaultEnumeration_WithIterAndCounter_Type;                   /* DefaultEnumeration_FullRange:    de_wiac_ */
INTDEF DeeTypeObject DefaultEnumeration_WithIterAndCounterAndFilter_Type;          /* DefaultEnumeration_WithIntRange: de_wiacaf_ */
/*TDEF DeeTypeObject DefaultEnumeration_WithIterAndUnpack_Type;                     * DefaultEnumeration_FullRange:    <Not needed; can use identity for this one> (for Mappings) */
INTDEF DeeTypeObject DefaultEnumeration_WithIterAndUnpackAndFilter_Type;           /* DefaultEnumeration_WithIntRange: de_wiauaf_ (for Mappings) */
INTDEF DeeTypeObject DefaultEnumeration_WithEnumerate_Type;                        /* DefaultEnumeration_FullRange:    de_we_ */
INTDEF DeeTypeObject DefaultEnumeration_WithEnumerateIndex_Type;                   /* DefaultEnumeration_WithIntRange: de_wei_ */
INTDEF DeeTypeObject DefaultEnumeration_WithEnumerateAndFilter_Type;               /* DefaultEnumeration_WithRange:    de_weaf_ */

/* Default functions for constructing sequence enumerations. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast(DeeObject *self); /* DefaultEnumeration_WithSizeAndGetItemIndexFast_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex(DeeObject *self);  /* DefaultEnumeration_WithSizeAndTryGetItemIndex_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex(DeeObject *self);     /* DefaultEnumeration_WithSizeAndGetItemIndex_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem(DeeObject *self);        /* DefaultEnumeration_WithSizeObAndGetItem_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithGetItemIndex(DeeObject *self);            /* DefaultEnumeration_WithGetItemIndex_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem(DeeObject *self);   /* DefaultEnumeration_WithIterKeysAndTryGetItem_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIterAndCounter(DeeObject *self);          /* DefaultEnumeration_WithIterAndCounter_Type */
/*INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithIterAndUnpack(DeeObject *self);          * DefaultEnumeration_WithIterAndUnpack_Type (Not needed; can use identity for this one!) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithEnumerate(DeeObject *self);               /* DefaultEnumeration_WithEnumerate_Type */
#define DeeMap_DefaultMakeEnumerationWithIterAndUnpack DeeObject_NewRef

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexFastAndFilter(DeeObject *self, size_t start, size_t end); /* DefaultEnumeration_WithSizeAndGetItemIndexFastAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndTryGetItemIndexAndFilter(DeeObject *self, size_t start, size_t end);  /* DefaultEnumeration_WithSizeAndTryGetItemIndexAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexAndFilter(DeeObject *self, size_t start, size_t end);     /* DefaultEnumeration_WithSizeAndGetItemIndexAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeObAndGetItemAndFilter(DeeObject *self, size_t start, size_t end);        /* DefaultEnumeration_WithSizeObAndGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithGetItemIndexAndFilter(DeeObject *self, size_t start, size_t end);            /* DefaultEnumeration_WithGetItemIndexAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndTryGetItemAndFilter(DeeObject *self, size_t start, size_t end);   /* DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter(DeeObject *self, size_t start, size_t end);          /* DefaultEnumeration_WithIterAndCounterAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithIntRangeWithIterAndUnpackAndFilter(DeeObject *self, size_t start, size_t end);           /* DefaultEnumeration_WithIterAndUnpackAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithIntRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);                   /* DefaultEnumeration_WithEnumerateIndex_Type */

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);      /* DefaultEnumeration_WithSizeObAndGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);               /* DefaultEnumeration_WithGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndTryGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end); /* DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithIterAndCounterAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);        /* DefaultEnumeration_WithIterAndCounterAndFilter_Type  (same as `DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter') */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);         /* DefaultEnumeration_WithIterAndUnpackAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithRangeWithEnumerateAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);             /* DefaultEnumeration_WithEnumerateAndFilter_Type */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H */
