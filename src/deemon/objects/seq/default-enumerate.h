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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#include <deemon/alloc.h>
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
/**/

#include "../generic-proxy.h"
/**/

#include <stddef.h> /* size_t */

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


#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
typedef struct {
	/* Enumerate all keys within */
	PROXY_OBJECT_HEAD(de_seq)     /* [1..1][const] The sequence being iterated. */
} DefaultEnumeration;

typedef struct {
	/* Enumerate all keys within within range [dewf_start,dewf_end) */
	PROXY_OBJECT_HEAD3(dewf_seq,   /* [1..1][const] The sequence being iterated. */
	                   dewf_start, /* [1..1][const] Enumeration range start. */
	                   dewf_end)   /* [1..1][const] Enumeration range end. */
} DefaultEnumerationWithFilter;

typedef struct {
	/* Enumerate all keys within within range [dewf_start,dewf_end) */
	PROXY_OBJECT_HEAD(dewif_seq)   /* [1..1][const] The sequence being iterated. */
	size_t            dewif_start; /* [const] Enumeration range start. */
	size_t            dewif_end;   /* [const] Enumeration range end. */
} DefaultEnumerationWithIntFilter;

LOCAL WUNUSED NONNULL((1, 2)) DREF DefaultEnumeration *DCALL
DefaultEnumeration_New(DeeTypeObject *DefaultEnumeration__with__xxx, DeeObject *self) {
	DREF DefaultEnumeration *result = DeeObject_MALLOC(DefaultEnumeration);
	if likely(result) {
		Dee_Incref(self);
		result->de_seq = self;
		DeeObject_Init(result, DefaultEnumeration__with__xxx);
	}
	return result;
}

LOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DefaultEnumerationWithFilter *DCALL
DefaultEnumerationWithFilter_New(DeeTypeObject *DefaultEnumerationWithFilter__with__xxx,
                                 DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DefaultEnumerationWithFilter *result = DeeObject_MALLOC(DefaultEnumerationWithFilter);
	if likely(result) {
		Dee_Incref(self);
		result->dewf_seq = self;
		Dee_Incref(start);
		result->dewf_start = start;
		Dee_Incref(end);
		result->dewf_end = end;
		DeeObject_Init(result, DefaultEnumerationWithFilter__with__xxx);
	}
	return result;
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DefaultEnumerationWithIntFilter *DCALL
DefaultEnumerationWithIntFilter_New(DeeTypeObject *DefaultEnumerationWithIntFilter__with__xxx,
                                    DeeObject *self, size_t start, size_t end) {
	DREF DefaultEnumerationWithIntFilter *result = DeeObject_MALLOC(DefaultEnumerationWithIntFilter);
	if likely(result) {
		Dee_Incref(self);
		result->dewif_seq = self;
		result->dewif_start = start;
		result->dewif_end = end;
		DeeObject_Init(result, DefaultEnumerationWithIntFilter__with__xxx);
	}
	return result;
}

/* For Sequence */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast;                         /* de_sos_gif__*:      seq_operator_size, tp_getitem_index_fast */
INTDEF DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast;            /* dewif_sos_gif__*:   seq_operator_size, tp_getitem_index_fast */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index;              /* de_sos_sotgi__*:    seq_operator_size, seq_operator_trygetitem_index */
INTDEF DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index; /* dewif_sos_sotgi__*: seq_operator_size, seq_operator_trygetitem_index */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index;                 /* de_sos_sogi__*:     seq_operator_size, seq_operator_getitem_index */
INTDEF DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index;    /* dewif_sos_soii__*:  seq_operator_size, seq_operator_getitem_index */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem;                     /* de_soso_sog__*:     seq_operator_sizeob, seq_operator_getitem */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_getitem_index;                                         /* de_sogi__*:         seq_operator_getitem_index */
INTDEF DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index;                            /* dewif_soii__*:      seq_operator_getitem_index */
INTDEF DeeTypeObject DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem;           /* dewf_soso_sog__*:   seq_operator_sizeob, seq_operator_getitem */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_getitem;                                               /* de_sog__*:          seq_operator_getitem */
INTDEF DeeTypeObject DefaultEnumerationWithFilter__with__seq_operator_getitem;                                     /* dewf_sog__*:        seq_operator_getitem */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_operator_iter__and__counter;                                    /* de_soi__*:          seq_operator_iter */
INTDEF DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter;                       /* dewif_soi__*:       seq_operator_iter */
INTDEF DeeTypeObject DefaultEnumeration__with__seq_enumerate;                                                      /* de_se__*:           seq_enumerate */
INTDEF DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_enumerate_index;                                   /* dewif_sei__*:       seq_enumerate_index */

/* For Mapping */
/*TDEF DeeTypeObject DefaultEnumeration__with__map_operator_iter__and__unpack;                        * de_moi__*:        map_operator_iter  <Not needed; can use identity for this one> (for Mappings) */
INTDEF DeeTypeObject DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack;             /* dewf_moi__*:      map_operator_iter */
INTDEF DeeTypeObject DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem;              /* de_mik_mog__*:    map_iterkeys, map_operator_getitem */
INTDEF DeeTypeObject DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem;    /* dewf_mik_mog__*:  map_iterkeys, map_operator_getitem */
INTDEF DeeTypeObject DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem;           /* de_mik_motg__*:   map_iterkeys, map_operator_trygetitem */
INTDEF DeeTypeObject DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem; /* dewf_mik_motg__*: map_iterkeys, map_operator_trygetitem */
INTDEF DeeTypeObject DefaultEnumeration__with__map_enumerate;                                        /* de_me__*:         map_enumerate */
INTDEF DeeTypeObject DefaultEnumerationWithFilter__with__map_enumerate_range;                        /* dewf_mer__*:      map_enumerate_range */
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

typedef struct {
	/* Enumerate all keys within [:dewsgi_size] */
	PROXY_OBJECT_HEAD(defr_seq)   /* [1..1][const] The sequence being iterated. */
	struct type_seq *defr_tp_seq; /* [1..1][const] Sequence operators of `defr_seq' (used operators depend on "ob_type") */
} DefaultEnumeration_FullRange;

typedef struct {
	/* Enumerate all keys within [dewir_start:dewir_end] */
	PROXY_OBJECT_HEAD(dewir_seq)   /* [1..1][const] The sequence being iterated. */
	struct type_seq *dewir_tp_seq; /* [1..1][const] Sequence operators of `dewir_seq' (always has `tp_size' and one of `tp_getitem_index_fast', `tp_trygetitem_index', `tp_getitem_index') */
	size_t           dewir_start;  /* [const] Enumeration range start. */
	size_t           dewir_end;    /* [const] Enumeration range end. */
} DefaultEnumeration_WithIntRange;

typedef struct {
	/* Enumerate all keys within [dewr_start:dewr_end] */
	OBJECT_HEAD
	DREF DeeObject  *dewr_seq;    /* [1..1][const] The sequence being iterated. */
	struct type_seq *dewr_tp_seq; /* [1..1][const] Sequence operators of `dewr_seq' (always has `tp_size' and one of `tp_getitem_index_fast', `tp_trygetitem_index', `tp_getitem_index') */
	DREF DeeObject  *dewr_start;  /* [1..1][const] Enumeration range start. */
	DREF DeeObject  *dewr_end;    /* [1..1][const] Enumeration range end. */
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
INTDEF DeeTypeObject DefaultEnumeration_WithIterKeysAndGetItem_Type;               /* DefaultEnumeration_FullRange:    de_wikagi_ */
INTDEF DeeTypeObject DefaultEnumeration_WithIterKeysAndGetItemAndFilter_Type;      /* DefaultEnumeration_WithRange:    de_wikagiaf_ */
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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem(DeeObject *self);      /* DefaultEnumeration_WithIterKeysAndGetItem_Type */
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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndGetItemAndFilter(DeeObject *self, size_t start, size_t end);      /* DefaultEnumeration_WithIterKeysAndGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndTryGetItemAndFilter(DeeObject *self, size_t start, size_t end);   /* DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter(DeeObject *self, size_t start, size_t end);          /* DefaultEnumeration_WithIterAndCounterAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithIntRangeWithIterAndUnpackAndFilter(DeeObject *self, size_t start, size_t end);           /* DefaultEnumeration_WithIterAndUnpackAndFilter_Type */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithIntRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);                   /* DefaultEnumeration_WithEnumerateIndex_Type */

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);      /* DefaultEnumeration_WithSizeObAndGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);               /* DefaultEnumeration_WithGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);    /* DefaultEnumeration_WithIterKeysAndGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndTryGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end); /* DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultMakeEnumerationWithRangeWithIterAndCounterAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);        /* DefaultEnumeration_WithIterAndCounterAndFilter_Type  (same as `DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter') */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);         /* DefaultEnumeration_WithIterAndUnpackAndFilter_Type */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultMakeEnumerationWithRangeWithEnumerateAndFilter(DeeObject *self, DeeObject *start, DeeObject *end);             /* DefaultEnumeration_WithEnumerateAndFilter_Type */
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H */
