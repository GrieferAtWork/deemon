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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H 1

#include <deemon/api.h>

#include <deemon/object.h>
#include <deemon/alloc.h>

#include "../generic-proxy.h"

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

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_H */
