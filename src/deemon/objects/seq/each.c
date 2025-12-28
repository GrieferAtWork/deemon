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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_EACH_C
#define GUARD_DEEMON_OBJECTS_SEQ_EACH_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/method-hints.h>
#include <deemon/mro.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
/**/

#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "../seq_functions.h"
#include "each.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seqeach_makeop0(DeeObject *__restrict seq, Dee_operator_t opname) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(0);
	if unlikely(!result)
		goto done;
	result->se_seq    = seq;
	result->so_opname = opname;
	result->so_opargc = 0;
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqEachOperator_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
seqeach_makeop1(DeeObject *seq, Dee_operator_t opname,
                /*inherit(always)*/ DREF DeeObject *arg_0) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(1);
	if unlikely(!result)
		goto err;
	result->se_seq       = seq;
	result->so_opname    = opname;
	result->so_opargc    = 1;
	result->so_opargv[0] = arg_0; /* Inherit reference. */
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqEachOperator_Type);
	return result;
err:
	Dee_Decref(arg_0);
	return NULL;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
seqeach_makeop2(DeeObject *seq, Dee_operator_t opname,
                /*inherit(always)*/ DREF DeeObject *arg_0,
                /*inherit(always)*/ DREF DeeObject *arg_1) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(2);
	if unlikely(!result)
		goto err;
	result->se_seq       = seq;
	result->so_opname    = opname;
	result->so_opargc    = 2;
	result->so_opargv[0] = arg_0; /* Inherit reference. */
	result->so_opargv[1] = arg_1; /* Inherit reference. */
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqEachOperator_Type);
	return result;
err:
	Dee_Decref(arg_1);
	Dee_Decref(arg_0);
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
se_ctor(SeqEachBase *__restrict self) {
	self->se_seq = DeeSeq_NewEmpty();
	return 0;
}

#define se_copy      generic_proxy__copy_alias
#define se_deep      generic_proxy__deepcopy
#define se_init      generic_proxy__init
#define se_serialize generic_proxy__serialize
#define se_fini      generic_proxy__fini
#define se_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_assign_cb(void *arg, DeeObject *elem) {
	return DeeObject_Assign(elem, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_assign(SeqEachBase *self, DeeObject *value) {
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_assign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_moveassign_cb(void *arg, DeeObject *elem) {
	return DeeObject_MoveAssign(elem, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_moveassign(SeqEachBase *self, DeeObject *value) {
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_moveassign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delitem_cb(void *arg, DeeObject *elem) {
	return DeeObject_DelItem(elem, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delitem(SeqEachBase *self, DeeObject *index) {
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delitem_cb, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delitem_index_cb(void *arg, DeeObject *elem) {
	return DeeObject_DelItemIndex(elem, (size_t)(uintptr_t)arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_delitem_index(SeqEachBase *self, size_t index) {
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delitem_index_cb, (void *)(uintptr_t)index);
}

struct se_foreach_setitem_data {
	DeeObject *sfesi_index; /* [1..1] Index to set */
	DeeObject *sfesi_value; /* [1..1] Value to set index to */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setitem_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setitem_data *data;
	data = (struct se_foreach_setitem_data *)arg;
	return DeeObject_SetItem(elem, data->sfesi_index, data->sfesi_value);
}

struct se_foreach_setitem_index_data {
	size_t     sfesii_index; /* Index to set */
	DeeObject *sfesii_value; /* [1..1] Value to set index to */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setitem_index_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setitem_index_data *data;
	data = (struct se_foreach_setitem_index_data *)arg;
	return DeeObject_SetItemIndex(elem, data->sfesii_index, data->sfesii_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_setitem(SeqEachBase *self, DeeObject *index, DeeObject *value) {
	struct se_foreach_setitem_data data;
	data.sfesi_index = index;
	data.sfesi_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setitem_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
se_setitem_index(SeqEachBase *self, size_t index, DeeObject *value) {
	struct se_foreach_setitem_index_data data;
	data.sfesii_index = index;
	data.sfesii_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setitem_index_cb, &data);
}

struct se_foreach_delrange_data {
	DeeObject *sfedr_start; /* [1..1] Range start */
	DeeObject *sfedr_end;   /* [1..1] Range end */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delrange_cb(void *arg, DeeObject *elem) {
	struct se_foreach_delrange_data *data;
	data = (struct se_foreach_delrange_data *)arg;
	return DeeObject_DelRange(elem, data->sfedr_start, data->sfedr_end);
}

struct se_foreach_delrange_index_data {
	Dee_ssize_t sfedri_start; /* Range start */
	Dee_ssize_t sfedri_end;   /* Range end */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delrange_index_cb(void *arg, DeeObject *elem) {
	struct se_foreach_delrange_index_data *data;
	data = (struct se_foreach_delrange_index_data *)arg;
	return DeeObject_DelRangeIndex(elem, data->sfedri_start, data->sfedri_end);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_delrange(SeqEachBase *self, DeeObject *start, DeeObject *end) {
	struct se_foreach_delrange_data data;
	data.sfedr_start = start;
	data.sfedr_end   = end;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_delrange_index(SeqEachBase *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct se_foreach_delrange_index_data data;
	data.sfedri_start = start;
	data.sfedri_end   = end;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delrange_index_n_cb(void *arg, DeeObject *elem) {
	return DeeObject_DelRangeIndexN(elem, (Dee_ssize_t)(size_t)(uintptr_t)arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_delrange_index_n(SeqEachBase *self, Dee_ssize_t start) {
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delrange_index_n_cb,
	                              (void *)(uintptr_t)(size_t)start);
}

struct se_foreach_setrange_data {
	DeeObject *sfesr_start; /* [1..1] Range start */
	DeeObject *sfesr_end;   /* [1..1] Range end */
	DeeObject *sfesr_value; /* [1..1] Range value */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setrange_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setrange_data *data;
	data = (struct se_foreach_setrange_data *)arg;
	return DeeObject_SetRange(elem, data->sfesr_start, data->sfesr_end, data->sfesr_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
se_setrange(SeqEachBase *self, DeeObject *start,
            DeeObject *end, DeeObject *value) {
	struct se_foreach_setrange_data data;
	data.sfesr_start = start;
	data.sfesr_end   = end;
	data.sfesr_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setrange_cb, &data);
}

struct se_foreach_setrange_index_data {
	Dee_ssize_t sfesri_start; /* Range start */
	Dee_ssize_t sfesri_end;   /* Range end */
	DeeObject  *sfesri_value; /* [1..1] Range value */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setrange_index_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setrange_index_data *data;
	data = (struct se_foreach_setrange_index_data *)arg;
	return DeeObject_SetRangeIndex(elem, data->sfesri_start, data->sfesri_end, data->sfesri_value);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
se_setrange_index(SeqEachBase *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value) {
	struct se_foreach_setrange_index_data data;
	data.sfesri_start = start;
	data.sfesri_end   = end;
	data.sfesri_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setrange_index_cb, &data);
}

struct se_foreach_setrange_index_n_data {
	Dee_ssize_t sfesrin_start; /* Range start */
	DeeObject  *sfesrin_value; /* [1..1] Range value */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setrange_index_n_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setrange_index_n_data *data;
	data = (struct se_foreach_setrange_index_n_data *)arg;
	return DeeObject_SetRangeIndexN(elem, data->sfesrin_start, data->sfesrin_value);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
se_setrange_index_n(SeqEachBase *self, Dee_ssize_t start, DeeObject *value) {
	struct se_foreach_setrange_index_n_data data;
	data.sfesrin_start = start;
	data.sfesrin_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setrange_index_n_cb, &data);
}

struct se_foreach_delitem_string_hash_data {
	char const *sfedish_key;  /* [1..1] Key name */
	Dee_hash_t  sfedish_hash; /* Key hash */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delitem_string_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_delitem_string_hash_data *data;
	data = (struct se_foreach_delitem_string_hash_data *)arg;
	return DeeObject_DelItemStringHash(elem, data->sfedish_key, data->sfedish_hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delitem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct se_foreach_delitem_string_hash_data data;
	data.sfedish_key  = key;
	data.sfedish_hash = hash;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delitem_string_hash_cb, &data);
}

struct se_foreach_delitem_string_len_hash_data {
	char const *sfedislh_key;    /* [1..sfedislh_keylen] Key name */
	size_t      sfedislh_keylen; /* Key length */
	Dee_hash_t  sfedislh_hash;   /* Key hash */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delitem_string_len_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_delitem_string_len_hash_data *data;
	data = (struct se_foreach_delitem_string_len_hash_data *)arg;
	return DeeObject_DelItemStringLenHash(elem, data->sfedislh_key,
	                                      data->sfedislh_keylen,
	                                      data->sfedislh_hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delitem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_foreach_delitem_string_len_hash_data data;
	data.sfedislh_key    = key;
	data.sfedislh_keylen = keylen;
	data.sfedislh_hash   = hash;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delitem_string_len_hash_cb, &data);
}

struct se_foreach_setitem_string_hash_data {
	char const *sfesish_key;   /* [1..1] Key name */
	Dee_hash_t  sfesish_hash;  /* Key hash */
	DeeObject  *sfesish_value; /* [1..1] Value to set. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setitem_string_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setitem_string_hash_data *data;
	data = (struct se_foreach_setitem_string_hash_data *)arg;
	return DeeObject_SetItemStringHash(elem, data->sfesish_key,
	                                   data->sfesish_hash,
	                                   data->sfesish_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
se_setitem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_hash_data data;
	data.sfesish_key   = key;
	data.sfesish_hash  = hash;
	data.sfesish_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setitem_string_hash_cb, &data);
}

struct se_foreach_setitem_string_len_hash_data {
	char const *sfesislh_key;    /* [1..sfesislh_keylen] Key name */
	size_t      sfesislh_keylen; /* Key length */
	Dee_hash_t  sfesislh_hash;   /* Key hash */
	DeeObject  *sfesislh_value;  /* [1..1] Value to set. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setitem_string_len_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setitem_string_len_hash_data *data;
	data = (struct se_foreach_setitem_string_len_hash_data *)arg;
	return DeeObject_SetItemStringLenHash(elem, data->sfesislh_key,
	                                      data->sfesislh_keylen,
	                                      data->sfesislh_hash,
	                                      data->sfesislh_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
se_setitem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen,
                           Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_len_hash_data data;
	data.sfesislh_key    = key;
	data.sfesislh_keylen = keylen;
	data.sfesislh_hash   = hash;
	data.sfesislh_value  = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setitem_string_len_hash_cb, &data);
}

#define SEQEACH_FOREACH_ERR     (-1)
#define SEQEACH_FOREACH_NO      (-2)
#define SEQEACH_FOREACH_YES     0
#define SEQEACH_FOREACH_MISSING 1 /* Found something that isn't present -> must be distinct from "found nothing" */

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqeach_map_bound2fe(int bounditem_result) {
	switch (bounditem_result) {
	case Dee_BOUND_YES:
		return SEQEACH_FOREACH_YES; /* Item is bound */
	case Dee_BOUND_NO:
		return SEQEACH_FOREACH_NO; /* Item exists, but isn't bound */
	case Dee_BOUND_MISSING:
		return SEQEACH_FOREACH_MISSING; /* Item doesn't exist */
	case Dee_BOUND_ERR:
		return SEQEACH_FOREACH_ERR;
	default: __builtin_unreachable();
	}
}

LOCAL ATTR_CONST int DCALL
seqeach_map_fe2bound(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQEACH_FOREACH_ERR:
		return Dee_BOUND_ERR;
	case SEQEACH_FOREACH_YES:
		return Dee_BOUND_YES;
	case SEQEACH_FOREACH_NO:
		return Dee_BOUND_NO;
	default: /* "default:" because "SEQSOME_FOREACH_MISSING" is positive */
		return Dee_BOUND_MISSING;
	}
}

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqeach_map_has2fe(int hasitem_result) {
	if (Dee_HAS_ISYES(hasitem_result))
		return SEQEACH_FOREACH_YES; /* Item exists */
	if (Dee_HAS_ISNO(hasitem_result))
		return SEQEACH_FOREACH_NO; /* Item doesn't exists */
	return SEQEACH_FOREACH_ERR;
}

LOCAL ATTR_CONST int DCALL
seqeach_map_fe2has(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQEACH_FOREACH_YES:
		return Dee_HAS_YES;
	case SEQEACH_FOREACH_NO:
		return Dee_HAS_NO;
	case SEQEACH_FOREACH_ERR:
		return Dee_HAS_ERR;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_bounditem_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_bound2fe(DeeObject_BoundItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_bounditem(SeqEachBase *self, DeeObject *index) {
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_has2fe(DeeObject_HasItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasitem(SeqEachBase *self, DeeObject *index) {
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &se_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_bounditem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_bound2fe(DeeObject_BoundItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_bounditem_index(SeqEachBase *self, size_t index) {
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_bounditem_index_foreach_cb,
	                                                       (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_has2fe(DeeObject_HasItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_hasitem_index(SeqEachBase *self, size_t index) {
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &se_hasitem_index_foreach_cb,
	                                                     (void *)(uintptr_t)index));
}

struct se_bounditem_string_hash_foreach_data {
	char const *ssbishfd_key;
	Dee_hash_t  ssbishfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_bounditem_string_hash_foreach_cb(void *arg, DeeObject *item) {
	struct se_bounditem_string_hash_foreach_data *data;
	data = (struct se_bounditem_string_hash_foreach_data *)arg;
	return seqeach_map_bound2fe(DeeObject_BoundItemStringHash(item, data->ssbishfd_key,
	                                                          data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_bounditem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_bounditem_string_hash_foreach_cb,
	                                                       &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_string_hash_foreach_cb(void *arg, DeeObject *item) {
	struct se_bounditem_string_hash_foreach_data *data;
	data = (struct se_bounditem_string_hash_foreach_data *)arg;
	return seqeach_map_has2fe(DeeObject_HasItemStringHash(item, data->ssbishfd_key,
	                                                      data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasitem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &se_hasitem_string_hash_foreach_cb,
	                                                     &data));
}

struct se_bounditem_string_len_hash_foreach_data {
	char const *ssbislhfd_key;
	size_t      ssbislhfd_keylen;
	Dee_hash_t  ssbislhfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_bounditem_string_len_hash_foreach_cb(void *arg, DeeObject *item) {
	struct se_bounditem_string_len_hash_foreach_data *data;
	data = (struct se_bounditem_string_len_hash_foreach_data *)arg;
	return seqeach_map_bound2fe(DeeObject_BoundItemStringLenHash(item, data->ssbislhfd_key,
	                                                             data->ssbislhfd_keylen,
	                                                             data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_bounditem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_bounditem_string_len_hash_foreach_cb,
	                                                       &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_string_len_hash_foreach_cb(void *arg, DeeObject *item) {
	struct se_bounditem_string_len_hash_foreach_data *data;
	data = (struct se_bounditem_string_len_hash_foreach_data *)arg;
	return seqeach_map_has2fe(DeeObject_HasItemStringLenHash(item, data->ssbislhfd_key,
	                                                         data->ssbislhfd_keylen,
	                                                         data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasitem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &se_hasitem_string_len_hash_foreach_cb,
	                                                     &data));
}






PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delattr_cb(void *arg, DeeObject *elem) {
	return DeeObject_DelAttr(elem, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delattr(SeqEachBase *self, DeeObject *attr) {
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delattr_cb, attr);
}

struct se_foreach_delattr_string_hash_data {
	char const *sfedsh_attr; /* [1..1] Attribute name */
	Dee_hash_t  sfedsh_hash; /* Attribute name hash */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delattr_string_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_delattr_string_hash_data *data;
	data = (struct se_foreach_delattr_string_hash_data *)arg;
	return DeeObject_DelAttrStringHash(elem, data->sfedsh_attr, data->sfedsh_hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct se_foreach_delattr_string_hash_data data;
	data.sfedsh_attr = attr;
	data.sfedsh_hash = hash;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delattr_string_hash_cb, &data);
}

struct se_foreach_delattr_string_len_hash_data {
	char const *sfedslh_attr;    /* [1..1] Attribute name */
	size_t      sfedslh_attrlen; /* Attribute name length */
	Dee_hash_t  sfedslh_hash;    /* Attribute name hash */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_delattr_string_len_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_delattr_string_len_hash_data *data;
	data = (struct se_foreach_delattr_string_len_hash_data *)arg;
	return DeeObject_DelAttrStringLenHash(elem, data->sfedslh_attr,
	                                      data->sfedslh_attrlen,
	                                      data->sfedslh_hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delattr_string_len_hash(SeqEachBase *self, char const *attr,
                           size_t attrlen, Dee_hash_t hash) {
	struct se_foreach_delattr_string_len_hash_data data;
	data.sfedslh_attr    = attr;
	data.sfedslh_attrlen = attrlen;
	data.sfedslh_hash    = hash;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_delattr_string_len_hash_cb, &data);
}

struct se_foreach_setattr_data {
	DeeObject *sfes_attr;  /* [1..1] Attribute name */
	DeeObject *sfes_value; /* [1..1] Value to assign  */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setattr_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setattr_data *data;
	data = (struct se_foreach_setattr_data *)arg;
	return DeeObject_SetAttr(elem, data->sfes_attr, data->sfes_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_setattr(SeqEachBase *self, DeeObject *attr, DeeObject *value) {
	struct se_foreach_setattr_data data;
	data.sfes_attr  = attr;
	data.sfes_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setattr_cb, &data);
}

struct se_foreach_setattr_string_hash_data {
	char const *sfessh_attr;  /* [1..1] Attribute name */
	Dee_hash_t  sfessh_hash;  /* Attribute name hash */
	DeeObject  *sfessh_value; /* [1..1] Value to assign */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setattr_string_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setattr_string_hash_data *data;
	data = (struct se_foreach_setattr_string_hash_data *)arg;
	return DeeObject_SetAttrStringHash(elem, data->sfessh_attr,
	                                   data->sfessh_hash,
	                                   data->sfessh_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
se_setattr_string_hash(SeqEachBase *self, char const *attr,
                       Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setattr_string_hash_data data;
	data.sfessh_attr  = attr;
	data.sfessh_hash  = hash;
	data.sfessh_value = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setattr_string_hash_cb, &data);
}

struct se_foreach_setattr_string_len_hash_data {
	char const *sfesslh_attr;    /* [1..1] Attribute name */
	size_t      sfesslh_attrlen; /* Attribute name length */
	Dee_hash_t  sfesslh_hash;    /* Attribute name hash */
	DeeObject  *sfesslh_value;   /* [1..1] Value to assign */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_foreach_setattr_string_len_hash_cb(void *arg, DeeObject *elem) {
	struct se_foreach_setattr_string_len_hash_data *data;
	data = (struct se_foreach_setattr_string_len_hash_data *)arg;
	return DeeObject_SetAttrStringLenHash(elem, data->sfesslh_attr,
	                                      data->sfesslh_attrlen,
	                                      data->sfesslh_hash,
	                                      data->sfesslh_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
se_setattr_string_len_hash(SeqEachBase *self, char const *attr,
                           size_t attrlen, Dee_hash_t hash,
                           DeeObject *value) {
	struct se_foreach_setattr_string_len_hash_data data;
	data.sfesslh_attr    = attr;
	data.sfesslh_attrlen = attrlen;
	data.sfesslh_hash    = hash;
	data.sfesslh_value   = value;
	return (int)DeeObject_Foreach(self->se_seq, &se_foreach_setattr_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_boundattr_foreach_cb(void *arg, DeeObject *attr) {
	return seqeach_map_bound2fe(DeeObject_BoundAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_boundattr(SeqEachBase *self, DeeObject *attr) {
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &se_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasattr_foreach_cb(void *arg, DeeObject *attr) {
	return seqeach_map_has2fe(DeeObject_HasAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasattr(SeqEachBase *self, DeeObject *attr) {
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_hasattr_foreach_cb, attr));
}

struct se_boundattr_string_hash_foreach_data {
	char const *ssbashfd_attr;
	Dee_hash_t  ssbashfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_boundattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_hash_foreach_data *data;
	data = (struct se_boundattr_string_hash_foreach_data *)arg;
	return seqeach_map_bound2fe(DeeObject_BoundAttrStringHash(attr, data->ssbashfd_attr,
	                                                          data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_boundattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &se_boundattr_string_hash_foreach_cb,
	                                                         &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_hash_foreach_data *data;
	data = (struct se_boundattr_string_hash_foreach_data *)arg;
	return seqeach_map_has2fe(DeeObject_HasAttrStringHash(attr, data->ssbashfd_attr, data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_hasattr_string_hash_foreach_cb,
	                                                       &data));
}

struct se_boundattr_string_len_hash_foreach_data {
	char const *ssbaslhfd_attr;
	size_t      ssbaslhfd_attrlen;
	Dee_hash_t  ssbaslhfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_boundattr_string_len_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_len_hash_foreach_data *data;
	data = (struct se_boundattr_string_len_hash_foreach_data *)arg;
	return seqeach_map_bound2fe(DeeObject_BoundAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                             data->ssbaslhfd_attrlen,
	                                                             data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_boundattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &se_boundattr_string_len_hash_foreach_cb,
	                                                         &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasattr_string_len_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_len_hash_foreach_data *data;
	data = (struct se_boundattr_string_len_hash_foreach_data *)arg;
	return seqeach_map_has2fe(DeeObject_HasAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                         data->ssbaslhfd_attrlen,
	                                                         data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &se_hasattr_string_len_hash_foreach_cb,
	                                                       &data));
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_print(SeqEachBase *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<each of %r>", self->se_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
se_printrepr(SeqEachBase *__restrict self, Dee_formatprinter_t printer, void *arg) {
	if (DeeSeq_Check(self->se_seq))
		return DeeFormat_Printf(printer, arg, "%r.each", self->se_seq);
	return DeeFormat_Printf(printer, arg, "(%r as Sequence).each", self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_bool(SeqEachBase *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_all, self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
se_call(SeqEachBase *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return seqeach_makeop1(self->se_seq, OPERATOR_CALL, tuple);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
se_call_kw(SeqEachBase *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	if (kw) {
		Dee_Incref(kw);
		return seqeach_makeop2(self->se_seq, OPERATOR_CALL, tuple, kw);
	}
	return seqeach_makeop1(self->se_seq, OPERATOR_CALL, tuple);
err:
	return NULL;
}

#define DEFINE_SEQ_EACH_UNARY(name, op)                      \
	PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL \
	name(SeqEachBase *__restrict self) {                     \
		return seqeach_makeop0(self->se_seq, op);            \
	}
#define DEFINE_SEQ_EACH_BINARY(name, op)                        \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachOperator *DCALL \
	name(SeqEachBase *self, DeeObject *other) {                 \
		Dee_Incref(other);                                      \
		return seqeach_makeop1(self->se_seq, op, other);        \
	}
#define DEFINE_SEQ_EACH_TRINARY(name, op)                          \
	PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF SeqEachOperator *DCALL \
	name(SeqEachBase *self, DeeObject *a, DeeObject *b) {          \
		Dee_Incref(a);                                             \
		Dee_Incref(b);                                             \
		return seqeach_makeop2(self->se_seq, op, a, b);            \
	}
DEFINE_SEQ_EACH_UNARY(se_iter_next, OPERATOR_ITERNEXT)
DEFINE_SEQ_EACH_UNARY(se_inv, OPERATOR_INV)
DEFINE_SEQ_EACH_UNARY(se_pos, OPERATOR_POS)
DEFINE_SEQ_EACH_UNARY(se_neg, OPERATOR_NEG)
DEFINE_SEQ_EACH_BINARY(se_add, OPERATOR_ADD)
DEFINE_SEQ_EACH_BINARY(se_sub, OPERATOR_SUB)
DEFINE_SEQ_EACH_BINARY(se_mul, OPERATOR_MUL)
DEFINE_SEQ_EACH_BINARY(se_div, OPERATOR_DIV)
DEFINE_SEQ_EACH_BINARY(se_mod, OPERATOR_MOD)
DEFINE_SEQ_EACH_BINARY(se_shl, OPERATOR_SHL)
DEFINE_SEQ_EACH_BINARY(se_shr, OPERATOR_SHR)
DEFINE_SEQ_EACH_BINARY(se_and, OPERATOR_AND)
DEFINE_SEQ_EACH_BINARY(se_or, OPERATOR_OR)
DEFINE_SEQ_EACH_BINARY(se_xor, OPERATOR_XOR)
DEFINE_SEQ_EACH_BINARY(se_pow, OPERATOR_POW)
DEFINE_SEQ_EACH_BINARY(se_eq, OPERATOR_EQ)
DEFINE_SEQ_EACH_BINARY(se_ne, OPERATOR_NE)
DEFINE_SEQ_EACH_BINARY(se_lo, OPERATOR_LO)
DEFINE_SEQ_EACH_BINARY(se_le, OPERATOR_LE)
DEFINE_SEQ_EACH_BINARY(se_gr, OPERATOR_GR)
DEFINE_SEQ_EACH_BINARY(se_ge, OPERATOR_GE)
DEFINE_SEQ_EACH_UNARY(se_iter, OPERATOR_ITER)
DEFINE_SEQ_EACH_UNARY(se_size, OPERATOR_SIZE)
DEFINE_SEQ_EACH_BINARY(se_contains, OPERATOR_CONTAINS)
DEFINE_SEQ_EACH_BINARY(se_getitem, OPERATOR_GETITEM)
DEFINE_SEQ_EACH_TRINARY(se_getrange, OPERATOR_GETRANGE)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_inplace(DeeObject *core_seq,
           int (DCALL *op)(DREF DeeObject **__restrict p_self, DeeObject *other),
           DeeObject *other) {
	size_t i, size;
	DREF DeeObject *elem;
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	size = DeeObject_InvokeMethodHint(seq_operator_size, core_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	seq_operator_getitem_index = DeeObject_RequireMethodHint(core_seq, seq_operator_getitem_index);
	seq_operator_setitem_index = DeeObject_RequireMethodHint(core_seq, seq_operator_setitem_index);
	for (i = 0; i < size; ++i) {
		elem = (*seq_operator_getitem_index)(core_seq, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			goto err;
		}
		if ((*op)(&elem, other))
			goto err_elem;
		if ((*seq_operator_setitem_index)(core_seq, i, elem))
			goto err_elem;
		Dee_Decref(elem);
	}
	return 0;
err_elem:
	Dee_Decref(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
inplace_inc_wrapper(DREF DeeObject **__restrict p_self,
                    DeeObject *other) {
	(void)other;
	return DeeObject_Inc(p_self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
inplace_dec_wrapper(DREF DeeObject **__restrict p_self,
                    DeeObject *other) {
	(void)other;
	return DeeObject_Dec(p_self);
}

#define DEFINE_SEQ_EACH_BINARY_INPLACE(name, func)            \
	PRIVATE NONNULL((1, 2)) int DCALL                         \
	name(SeqEachBase **__restrict p_self, DeeObject *other) { \
		return se_inplace((*p_self)->se_seq, &func, other);   \
	}
PRIVATE NONNULL((1)) int DCALL se_inc(SeqEachBase **__restrict p_self) {
	return se_inplace((*p_self)->se_seq, &inplace_inc_wrapper, NULL);
}
PRIVATE NONNULL((1)) int DCALL se_dec(SeqEachBase **__restrict p_self) {
	return se_inplace((*p_self)->se_seq, &inplace_dec_wrapper, NULL);
}

DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_add, DeeObject_InplaceAdd)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_sub, DeeObject_InplaceSub)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_mul, DeeObject_InplaceMul)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_div, DeeObject_InplaceDiv)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_mod, DeeObject_InplaceMod)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_shl, DeeObject_InplaceShl)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_shr, DeeObject_InplaceShr)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_and, DeeObject_InplaceAnd)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_or, DeeObject_InplaceOr)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_xor, DeeObject_InplaceXor)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_pow, DeeObject_InplacePow)



PRIVATE struct type_math se_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_pow,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&se_inc,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&se_dec,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_add,
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_sub,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_mul,
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_div,
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_mod,
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_shl,
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_shr,
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_xor,
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_pow,
};

PRIVATE struct type_cmp se_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__eq),
	/* .tp_compare       = */ DEFIMPL(&default__compare__with__eq__and__lo),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_ge,
};

PRIVATE struct type_seq se_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_size,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&se_setitem,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&se_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&se_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&se_setrange,
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_hasitem,
	/* .tp_size                       = */ DEFIMPL(&default__size__with__sizeob),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&se_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&se_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&se_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&se_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__getrange_index__with__getrange),
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&se_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&se_setrange_index,
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__getrange_index_n__with__getrange),
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&se_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&se_setrange_index_n,
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&se_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&se_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&se_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&se_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&se_hasitem_string_len_hash,
};

PRIVATE char const s_unhandled_leave_message[] = "Unhandled exception in `operator leave'";


PRIVATE WUNUSED NONNULL((1)) int DCALL
se_enter(SeqEachBase *__restrict self) {
	DREF DeeObject **elem;
	size_t i, count;
	elem = DeeSeq_AsHeapVector(self->se_seq, &count);
	if unlikely(!elem)
		goto err;
	for (i = 0; i < count; ++i) {
		if (DeeObject_Enter(elem[i]))
			goto err_elem_i;
	}
	Dee_Decrefv(elem, count);
	Dee_Free(elem);
	return 0;
err_elem_i:
	while (i--) {
		if unlikely(DeeObject_Leave(elem[i]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
	}
/*err_elem:*/
	Dee_Decrefv(elem, count);
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_leave(SeqEachBase *__restrict self) {
	DREF DeeObject **elem;
	size_t count;
	elem = DeeSeq_AsHeapVector(self->se_seq, &count);
	if unlikely(!elem)
		goto err;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			goto err_elem_count;
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
	return 0;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
err_elem_count:
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE struct type_with se_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&se_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&se_leave,
};


PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
se_iterattr_impl(DeeObject *seq, struct Dee_attriter *iterbuf,
                 size_t bufsize, struct Dee_attrhint const *__restrict hint) {
	(void)seq;
	(void)hint;
	/* TODO: Enumerate attributes available to all elements of `seq'. */
	return Dee_attriter_initempty(iterbuf, bufsize);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_findattr_impl(DeeObject *seq,
                 struct Dee_attrspec const *__restrict specs,
                 struct Dee_attrdesc *__restrict result) {
	(void)seq;
	(void)specs;
	(void)result;
	/* TODO: Find attributes available to all elements of `seq'. */
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
se_iterattr(DeeTypeObject *UNUSED(tp_self),
            SeqEachBase *self, struct Dee_attriter *iterbuf,
            size_t bufsize, struct Dee_attrhint const *__restrict hint) {
	return se_iterattr_impl(self->se_seq, iterbuf, bufsize, hint);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_findattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
            struct Dee_attrspec const *__restrict specs,
            struct Dee_attrdesc *__restrict result) {
	return se_findattr_impl(self->se_seq, specs, result);
}

#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define se_getattr seqeach_getattr
PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL
seqeach_getattr(SeqEachBase *self,
                DeeStringObject *attr) {
	DREF SeqEachGetAttr *result;
	result = DeeObject_MALLOC(SeqEachGetAttr);
	if unlikely(!result)
		goto done;
	result->se_seq  = self->se_seq;
	result->sg_attr = attr;
	Dee_Incref(self->se_seq);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqEachGetAttr_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
se_callattr(SeqEachBase *self, /*String*/ DeeObject *attr,
            size_t argc, DeeObject *const *argv) {
	return DeeSeqEach_CallAttr(self->se_seq, attr, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
se_callattr_kw(SeqEachBase *self, /*String*/ DeeObject *attr,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeSeqEach_CallAttrKw(self->se_seq, attr, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
se_callattr_string_hash(SeqEachBase *self, char const *attr,
                        Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	return DeeSeqEach_CallAttrStringHash(self->se_seq, attr, hash, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
se_callattr_string_hash_kw(SeqEachBase *self, char const *attr, Dee_hash_t hash,
                           size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeSeqEach_CallAttrStringHashKw(self->se_seq, attr, hash, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
se_callattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen,
                            Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	return DeeSeqEach_CallAttrStringLenHash(self->se_seq, attr, attrlen, hash, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
se_callattr_string_len_hash_kw(SeqEachBase *self, char const *attr, size_t attrlen,
                               Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeSeqEach_CallAttrStringLenHashKw(self->se_seq, attr, attrlen, hash, argc, argv, kw);
}
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
DEFINE_SEQ_EACH_BINARY(se_getattr, OPERATOR_GETATTR)
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */


PRIVATE struct type_attr se_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&se_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&se_delattr,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&se_setattr,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&se_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&se_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_boundattr,
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&se_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&se_callattr_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_delattr_string_hash,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&se_setattr_string_hash,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_hasattr_string_hash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_boundattr_string_hash,
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&se_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&se_callattr_string_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&se_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&se_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&se_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&se_boundattr_string_len_hash,
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&se_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&se_callattr_string_len_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_findattr_info_string_len_hash = */ NULL
};

PRIVATE struct type_member tpconst se_members[] = {
#define ss_members se_members
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SeqEachBase, se_seq), "->?DSequence"),
	TYPE_MEMBER_END
};

PRIVATE struct type_callable se_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&se_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

INTERN DeeTypeObject SeqEach_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEach",
	/* .tp_doc      = */ DOC("(seq:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqEachBase,
			/* tp_ctor:        */ &se_ctor,
			/* tp_copy_ctor:   */ &se_copy,
			/* tp_deep_ctor:   */ &se_deep,
			/* tp_any_ctor:    */ &se_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &se_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&se_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_moveassign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&se_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&se_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&se_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&se_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &se_math,
	/* .tp_cmp           = */ &se_cmp,
	/* .tp_seq           = */ &se_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ &se_attr,
	/* .tp_with          = */ &se_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ se_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&se_call,
	/* .tp_callable      = */ &se_callable,
};


/* SeqSome */
PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seqsome_makeop0(DeeObject *__restrict seq, Dee_operator_t opname) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(0);
	if unlikely(!result)
		goto done;
	result->se_seq    = seq;
	result->so_opname = opname;
	result->so_opargc = 0;
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqSomeOperator_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
seqsome_makeop1(DeeObject *seq, Dee_operator_t opname,
                /*inherit(always)*/ DREF DeeObject *arg_0) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(1);
	if unlikely(!result)
		goto err;
	result->se_seq       = seq;
	result->so_opname    = opname;
	result->so_opargc    = 1;
	result->so_opargv[0] = arg_0; /* Inherit reference. */
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqSomeOperator_Type);
	return result;
err:
	Dee_Decref(arg_0);
	return NULL;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
seqsome_makeop2(DeeObject *seq, Dee_operator_t opname,
                /*inherit(always)*/ DREF DeeObject *arg_0,
                /*inherit(always)*/ DREF DeeObject *arg_1) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(2);
	if unlikely(!result)
		goto err;
	result->se_seq       = seq;
	result->so_opname    = opname;
	result->so_opargc    = 2;
	result->so_opargv[0] = arg_0; /* Inherit reference. */
	result->so_opargv[1] = arg_1; /* Inherit reference. */
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqSomeOperator_Type);
	return result;
err:
	Dee_Decref(arg_1);
	Dee_Decref(arg_0);
	return NULL;
}

#define ss_ctor  se_ctor
#define ss_copy  se_copy
#define ss_deep  se_deep
#define ss_init  se_init
#define ss_fini  se_fini
#define ss_visit se_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
ss_bool(SeqEachBase *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_any, self->se_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ss_print(SeqEachBase *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<some of %r>", self->se_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ss_printrepr(SeqEachBase *__restrict self, Dee_formatprinter_t printer, void *arg) {
	if (DeeSeq_Check(self->se_seq))
		return DeeFormat_Printf(printer, arg, "%r.some", self->se_seq);
	return DeeFormat_Printf(printer, arg, "(%r as Sequence).some", self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
ss_call(SeqEachBase *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return seqsome_makeop1(self->se_seq, OPERATOR_CALL, tuple);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
ss_call_kw(SeqEachBase *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	if (kw) {
		Dee_Incref(kw);
		return seqsome_makeop2(self->se_seq, OPERATOR_CALL, tuple, kw);
	}
	return seqsome_makeop1(self->se_seq, OPERATOR_CALL, tuple);
err:
	return NULL;
}

#define DEFINE_SEQ_SOME_UNARY(name, op)                      \
	PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL \
	name(SeqEachBase *__restrict self) {                     \
		return seqsome_makeop0(self->se_seq, op);            \
	}
#define DEFINE_SEQ_SOME_BINARY(name, op)                        \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachOperator *DCALL \
	name(SeqEachBase *self, DeeObject *other) {                 \
		Dee_Incref(other);                                      \
		return seqsome_makeop1(self->se_seq, op, other);        \
	}
#define DEFINE_SEQ_SOME_TRINARY(name, op)                          \
	PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF SeqEachOperator *DCALL \
	name(SeqEachBase *self, DeeObject *a, DeeObject *b) {          \
		Dee_Incref(a);                                             \
		Dee_Incref(b);                                             \
		return seqsome_makeop2(self->se_seq, op, a, b);            \
	}
DEFINE_SEQ_SOME_UNARY(ss_iter_next, OPERATOR_ITERNEXT)
DEFINE_SEQ_SOME_UNARY(ss_inv, OPERATOR_INV)
DEFINE_SEQ_SOME_UNARY(ss_pos, OPERATOR_POS)
DEFINE_SEQ_SOME_UNARY(ss_neg, OPERATOR_NEG)
DEFINE_SEQ_SOME_BINARY(ss_add, OPERATOR_ADD)
DEFINE_SEQ_SOME_BINARY(ss_sub, OPERATOR_SUB)
DEFINE_SEQ_SOME_BINARY(ss_mul, OPERATOR_MUL)
DEFINE_SEQ_SOME_BINARY(ss_div, OPERATOR_DIV)
DEFINE_SEQ_SOME_BINARY(ss_mod, OPERATOR_MOD)
DEFINE_SEQ_SOME_BINARY(ss_shl, OPERATOR_SHL)
DEFINE_SEQ_SOME_BINARY(ss_shr, OPERATOR_SHR)
DEFINE_SEQ_SOME_BINARY(ss_and, OPERATOR_AND)
DEFINE_SEQ_SOME_BINARY(ss_or, OPERATOR_OR)
DEFINE_SEQ_SOME_BINARY(ss_xor, OPERATOR_XOR)
DEFINE_SEQ_SOME_BINARY(ss_pow, OPERATOR_POW)
DEFINE_SEQ_SOME_BINARY(ss_eq, OPERATOR_EQ)
DEFINE_SEQ_SOME_BINARY(ss_ne, OPERATOR_NE)
DEFINE_SEQ_SOME_BINARY(ss_lo, OPERATOR_LO)
DEFINE_SEQ_SOME_BINARY(ss_le, OPERATOR_LE)
DEFINE_SEQ_SOME_BINARY(ss_gr, OPERATOR_GR)
DEFINE_SEQ_SOME_BINARY(ss_ge, OPERATOR_GE)
DEFINE_SEQ_SOME_UNARY(ss_iter, OPERATOR_ITER)
DEFINE_SEQ_SOME_UNARY(ss_size, OPERATOR_SIZE)
DEFINE_SEQ_SOME_BINARY(ss_contains, OPERATOR_CONTAINS)
DEFINE_SEQ_SOME_BINARY(ss_getitem, OPERATOR_GETITEM)
DEFINE_SEQ_SOME_TRINARY(ss_getrange, OPERATOR_GETRANGE)


PRIVATE struct type_math ss_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_pow,
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL(&default__dec__with__sub),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__inplace_sub__with__sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL(&default__inplace_div__with__div),
	/* .tp_inplace_mod = */ DEFIMPL(&default__inplace_mod__with__mod),
	/* .tp_inplace_shl = */ DEFIMPL(&default__inplace_shl__with__shl),
	/* .tp_inplace_shr = */ DEFIMPL(&default__inplace_shr__with__shr),
	/* .tp_inplace_and = */ DEFIMPL(&default__inplace_and__with__and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__inplace_or__with__or),
	/* .tp_inplace_xor = */ DEFIMPL(&default__inplace_xor__with__xor),
	/* .tp_inplace_pow = */ DEFIMPL(&default__inplace_pow__with__pow),
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_trycompare_eq(SeqEachBase *self, DeeObject *other) {
	/* `seq.some == other'  <=>  `other in seq' */
	int contains = DeeObject_InvokeMethodHint(seq_contains, self->se_seq, other);
	if unlikely(contains < 0)
		goto err;
	return contains ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

#define SEQSOME_FOREACH_ERR     (-1)
#define SEQSOME_FOREACH_YES     (-2)
#define SEQSOME_FOREACH_NO      0
#define SEQSOME_FOREACH_MISSING 1 /* Found something that isn't present -> must be distinct from "found nothing" */

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqsome_map_bound2fe(int bounditem_result) {
	switch (bounditem_result) {
	case Dee_BOUND_YES:
		return SEQSOME_FOREACH_YES; /* Item is bound */
	case Dee_BOUND_NO:
		return SEQSOME_FOREACH_NO; /* Item doesn't exist */
	case Dee_BOUND_MISSING:
		return SEQSOME_FOREACH_MISSING; /* Item exists, but isn't bound */
	case Dee_BOUND_ERR:
		return SEQSOME_FOREACH_ERR;
	default: __builtin_unreachable();
	}
}

LOCAL ATTR_CONST int DCALL
seqsome_map_fe2bound(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQSOME_FOREACH_ERR:
		return Dee_BOUND_ERR;
	case SEQSOME_FOREACH_YES:
		return Dee_BOUND_YES;
	case SEQSOME_FOREACH_NO:
		return Dee_BOUND_NO;
	default: /* "default:" because "SEQSOME_FOREACH_MISSING" is positive */
		return Dee_BOUND_MISSING;
	}
}

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqsome_map_has2fe(int hasitem_result) {
	if (Dee_HAS_ISYES(hasitem_result))
		return SEQSOME_FOREACH_YES; /* Item exists */
	if (Dee_HAS_ISNO(hasitem_result))
		return SEQSOME_FOREACH_NO; /* Item doesn't exists */
	return SEQSOME_FOREACH_ERR;
}

LOCAL ATTR_CONST int DCALL
seqsome_map_fe2has(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQSOME_FOREACH_ERR:
		return Dee_HAS_ERR;
	case SEQSOME_FOREACH_YES:
		return Dee_HAS_YES;
	case SEQSOME_FOREACH_NO:
		return Dee_HAS_NO;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_bounditem_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_bound2fe(DeeObject_BoundItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_bounditem(SeqEachBase *self, DeeObject *index) {
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &ss_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_has2fe(DeeObject_HasItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasitem(SeqEachBase *self, DeeObject *index) {
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_bounditem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_bound2fe(DeeObject_BoundItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ss_bounditem_index(SeqEachBase *self, size_t index) {
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &ss_bounditem_index_foreach_cb,
	                                                         (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_has2fe(DeeObject_HasItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ss_hasitem_index(SeqEachBase *self, size_t index) {
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_hasitem_index_foreach_cb,
	                                                       (void *)(uintptr_t)index));
}

struct ss_bounditem_string_hash_foreach_data {
	char const *ssbishfd_key;
	Dee_hash_t  ssbishfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_bounditem_string_hash_foreach_cb(void *arg, DeeObject *item) {
	struct ss_bounditem_string_hash_foreach_data *data;
	data = (struct ss_bounditem_string_hash_foreach_data *)arg;
	return seqsome_map_bound2fe(DeeObject_BoundItemStringHash(item, data->ssbishfd_key,
	                                                          data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_bounditem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &ss_bounditem_string_hash_foreach_cb,
	                                                         &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_string_hash_foreach_cb(void *arg, DeeObject *item) {
	struct ss_bounditem_string_hash_foreach_data *data;
	data = (struct ss_bounditem_string_hash_foreach_data *)arg;
	return seqsome_map_has2fe(DeeObject_HasItemStringHash(item, data->ssbishfd_key,
	                                                      data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasitem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_hasitem_string_hash_foreach_cb,
	                                                       &data));
}

struct ss_bounditem_string_len_hash_foreach_data {
	char const *ssbislhfd_key;
	size_t      ssbislhfd_keylen;
	Dee_hash_t  ssbislhfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_bounditem_string_len_hash_foreach_cb(void *arg, DeeObject *item) {
	struct ss_bounditem_string_len_hash_foreach_data *data;
	data = (struct ss_bounditem_string_len_hash_foreach_data *)arg;
	return seqsome_map_bound2fe(DeeObject_BoundItemStringLenHash(item, data->ssbislhfd_key,
	                                                             data->ssbislhfd_keylen,
	                                                             data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_bounditem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                         &ss_bounditem_string_len_hash_foreach_cb,
	                                                         &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_string_len_hash_foreach_cb(void *arg, DeeObject *item) {
	struct ss_bounditem_string_len_hash_foreach_data *data;
	data = (struct ss_bounditem_string_len_hash_foreach_data *)arg;
	return seqsome_map_has2fe(DeeObject_HasItemStringLenHash(item, data->ssbislhfd_key,
	                                                         data->ssbislhfd_keylen,
	                                                         data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasitem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_hasitem_string_len_hash_foreach_cb,
	                                                       &data));
}


PRIVATE struct type_cmp ss_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_trycompare_eq,
	/* .tp_compare       = */ DEFIMPL(&default__compare__with__eq__and__lo),
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_trycompare_eq,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_ge,
};

PRIVATE struct type_seq ss_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_size,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_getitem,
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ss_getrange,
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_hasitem,
	/* .tp_size                       = */ DEFIMPL(&default__size__with__sizeob),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ss_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ss_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__getrange_index__with__getrange),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__getrange_index_n__with__getrange),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&ss_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&ss_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&ss_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&ss_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
ss_iterattr_impl(DeeObject *seq, struct Dee_attriter *iterbuf,
                 size_t bufsize, struct Dee_attrhint const *__restrict hint) {
	(void)seq;
	(void)hint;
	/* TODO: Enumerate attributes available to at least one element of `seq'. */
	return Dee_attriter_initempty(iterbuf, bufsize);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ss_findattr_impl(DeeObject *seq,
                 struct Dee_attrspec const *__restrict specs,
                 struct Dee_attrdesc *__restrict result) {
	(void)seq;
	(void)specs;
	(void)result;
	/* TODO: Find attributes available to at least one element of `seq'. */
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
ss_iterattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
            struct Dee_attriter *iterbuf, size_t bufsize,
            struct Dee_attrhint const *__restrict hint) {
	return ss_iterattr_impl(self->se_seq, iterbuf, bufsize, hint);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
ss_findattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
            struct Dee_attrspec const *__restrict specs,
            struct Dee_attrdesc *__restrict result) {
	return ss_findattr_impl(self->se_seq, specs, result);
}

#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
#define ss_getattr seqsome_getattr
PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL
seqsome_getattr(SeqEachBase *self, DeeStringObject *attr) {
	DREF SeqEachGetAttr *result;
	result = DeeObject_MALLOC(SeqEachGetAttr);
	if unlikely(!result)
		goto done;
	result->se_seq  = self->se_seq;
	result->sg_attr = attr;
	Dee_Incref(self->se_seq);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqSomeGetAttr_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ss_callattr(SeqEachBase *self, /*String*/ DeeObject *attr,
            size_t argc, DeeObject *const *argv) {
	return DeeSeqSome_CallAttr(self->se_seq, attr, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ss_callattr_kw(SeqEachBase *self, /*String*/ DeeObject *attr,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeSeqSome_CallAttrKw(self->se_seq, attr, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ss_callattr_string_hash(SeqEachBase *self, char const *attr,
                        Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	return DeeSeqSome_CallAttrStringHash(self->se_seq, attr, hash, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ss_callattr_string_hash_kw(SeqEachBase *self, char const *attr, Dee_hash_t hash,
                           size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeSeqSome_CallAttrStringHashKw(self->se_seq, attr, hash, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ss_callattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen,
                            Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	return DeeSeqSome_CallAttrStringLenHash(self->se_seq, attr, attrlen, hash, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ss_callattr_string_len_hash_kw(SeqEachBase *self, char const *attr, size_t attrlen,
                               Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeSeqSome_CallAttrStringLenHashKw(self->se_seq, attr, attrlen, hash, argc, argv, kw);
}
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
DEFINE_SEQ_SOME_BINARY(ss_getattr, OPERATOR_GETATTR)
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_boundattr_foreach_cb(void *arg, DeeObject *attr) {
	return seqsome_map_bound2fe(DeeObject_BoundAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_boundattr(SeqEachBase *self, DeeObject *attr) {
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasattr_foreach_cb(void *arg, DeeObject *attr) {
	return seqsome_map_has2fe(DeeObject_HasAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasattr(SeqEachBase *self, DeeObject *attr) {
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &ss_hasattr_foreach_cb, attr));
}

struct ss_boundattr_string_hash_foreach_data {
	char const *ssbashfd_attr;
	Dee_hash_t  ssbashfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_boundattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_hash_foreach_data *data;
	data = (struct ss_boundattr_string_hash_foreach_data *)arg;
	return seqsome_map_bound2fe(DeeObject_BoundAttrStringHash(attr, data->ssbashfd_attr,
	                                                          data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_boundattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_boundattr_string_hash_foreach_cb,
	                                                       &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_hash_foreach_data *data;
	data = (struct ss_boundattr_string_hash_foreach_data *)arg;
	return seqsome_map_has2fe(DeeObject_HasAttrStringHash(attr, data->ssbashfd_attr,
	                                                      data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &ss_hasattr_string_hash_foreach_cb,
	                                                     &data));
}

struct ss_boundattr_string_len_hash_foreach_data {
	char const *ssbaslhfd_attr;
	size_t      ssbaslhfd_attrlen;
	Dee_hash_t  ssbaslhfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_boundattr_string_len_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_len_hash_foreach_data *data;
	data = (struct ss_boundattr_string_len_hash_foreach_data *)arg;
	return seqsome_map_bound2fe(DeeObject_BoundAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                             data->ssbaslhfd_attrlen,
	                                                             data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_boundattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_fe2bound(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                       &ss_boundattr_string_len_hash_foreach_cb,
	                                                       &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasattr_string_len_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_len_hash_foreach_data *data;
	data = (struct ss_boundattr_string_len_hash_foreach_data *)arg;
	return seqsome_map_has2fe(DeeObject_HasAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                         data->ssbaslhfd_attrlen,
	                                                         data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_fe2has(DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                                     &ss_hasattr_string_len_hash_foreach_cb,
	                                                     &data));
}



PRIVATE struct type_attr ss_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&ss_getattr,
	/* .tp_delattr                       = */ NULL,
	/* .tp_setattr                       = */ NULL,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&ss_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&ss_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_boundattr,
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&ss_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&ss_callattr_kw,
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ NULL,
	/* .tp_setattr_string_hash           = */ NULL,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&ss_hasattr_string_hash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&ss_boundattr_string_hash,
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&ss_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&ss_callattr_string_hash_kw,
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ NULL,
	/* .tp_setattr_string_len_hash       = */ NULL,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&ss_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&ss_boundattr_string_len_hash,
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&ss_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&ss_callattr_string_len_hash_kw,
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_findattr_info_string_len_hash = */ NULL
};

PRIVATE struct type_callable ss_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&ss_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeSeqSome_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSome",
	/* .tp_doc      = */ DOC("(seq:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqEachBase,
			/* tp_ctor:        */ &ss_ctor,
			/* tp_copy_ctor:   */ &ss_copy,
			/* tp_deep_ctor:   */ &ss_deep,
			/* tp_any_ctor:    */ &ss_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ss_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&ss_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ss_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ss_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ss_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &ss_math,
	/* .tp_cmp           = */ &ss_cmp,
	/* .tp_seq           = */ &ss_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ &ss_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ss_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&ss_call,
	/* .tp_callable      = */ &ss_callable,
};

#undef DEFINE_SEQ_EACH_TRINARY
#undef DEFINE_SEQ_EACH_BINARY
#undef DEFINE_SEQ_EACH_UNARY
#undef DEFINE_SEQ_EACH_BINARY_INPLACE



/* Construct an each-wrapper for `self' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Each(DeeObject *__restrict self) {
	DREF SeqEachBase *result;
	result = DeeObject_MALLOC(SeqEachBase);
	if unlikely(!result)
		goto done;
	result->se_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqEach_Type);
done:
	return Dee_AsObject(result);
}


STATIC_ASSERT(sizeof(DeeSeqSomeObject) == sizeof(SeqEachBase));
STATIC_ASSERT(offsetof(DeeSeqSomeObject, se_seq) == offsetof(SeqEachBase, se_seq));

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Some(DeeObject *__restrict self) {
	DREF SeqEachBase *result;
	result = DeeObject_MALLOC(SeqEachBase);
	if unlikely(!result)
		goto done;
	result->se_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeSeqSome_Type);
done:
	return Dee_AsObject(result);
}





/* SeqEach<WRAPPER> -- Operator */
PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_ctor(SeqEachOperator *__restrict self) {
	self->se_seq    = DeeSeq_NewEmpty();
	self->so_opname = OPERATOR_BOOL;
	self->so_opargc = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_copy(SeqEachOperator *__restrict self,
         SeqEachOperator *__restrict other) {
	self->se_seq    = other->se_seq;
	self->so_opname = other->so_opname;
	self->so_opargc = other->so_opargc;
	Dee_Movrefv(self->so_opargv, other->so_opargv, self->so_opargc);
	Dee_Incref(self->se_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_deep(SeqEachOperator *__restrict self,
         SeqEachOperator *__restrict other) {
	size_t i;
	self->se_seq = DeeObject_DeepCopy(other->se_seq);
	if unlikely(!self->se_seq)
		goto err;
	for (i = 0; i < other->so_opargc; ++i) {
		self->so_opargv[i] = DeeObject_DeepCopy(other->so_opargv[i]);
		if unlikely(!self->so_opargv[i])
			goto err_i;
	}
	self->so_opname = other->so_opname;
	self->so_opargc = other->so_opargc;
	return 0;
err_i:
	Dee_Decrefv(self->so_opargv, i);
	Dee_Decref(self->se_seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_init(SeqEachOperator *__restrict self,
         size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_SeqEachOperator", params: """
	DeeObject *seq:?DSequence;
	DeeObject *op:?X2?Dstring?Dint;
	DeeTupleObject *args = (DeeTupleObject *)Dee_EmptyTuple;
""", docStringPrefix: "seo");]]]*/
#define seo__SeqEachOperator_params "seq:?DSequence,op:?X2?Dstring?Dint,args=!T0"
	struct {
		DeeObject *seq;
		DeeObject *op;
		DeeTupleObject *args;
	} args;
	args.args = (DeeTupleObject *)Dee_EmptyTuple;
	if (DeeArg_UnpackStruct(argc, argv, "oo|o:_SeqEachOperator", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.args, &DeeTuple_Type))
		goto err;
	self->se_seq = args.seq;
	if unlikely(DeeTuple_SIZE(args.args) > COMPILER_LENOF(self->so_opargv)) {
		DeeError_Throwf(&DeeError_UnpackError,
		                "Too many operator arguments (%" PRFuSIZ " > %" PRFuSIZ ")",
		                (size_t)DeeTuple_SIZE(args.args),
		                (size_t)COMPILER_LENOF(self->so_opargv));
		goto err;
	}
	if (DeeString_Check(args.op)) {
		struct opinfo const *info;
		char const *name = DeeString_STR(args.op);
		info = DeeTypeType_GetOperatorByName(&DeeType_Type, name, (size_t)-1);
		if unlikely(info == NULL) {
			/* TODO: In this case, remember the used "args.op" string,
			 * and query the operator on a per-element basis */
			DeeError_Throwf(&DeeError_ValueError, "Unknown operator %q", name);
			goto err;
		}
		self->so_opname = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(args.op, &self->so_opname))
			goto err;
	}
	self->so_opargc = (uint16_t)DeeTuple_SIZE(args.args);
	Dee_Movrefv(self->so_opargv, DeeTuple_ELEM(args.args), self->so_opargc);
	Dee_Incref(self->se_seq);
	return 0;
err:
	return -1;
}

#ifndef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_printrepr(SeqEachOperator *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	char const *each_suffix = ".each";
	DeeTypeObject *seq_type = Dee_TYPE(self->se_seq);
	if (DeeType_IsSeqEachWrapper(seq_type))
		each_suffix = NULL;
	return DeeFormat_PrintOperatorRepr(printer, arg,
	                                   self->se_seq, self->so_opname,
	                                   self->so_opargc, self->so_opargv,
	                                   NULL, 0,
	                                   each_suffix, each_suffix ? 5 : 0);
}
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR */

PRIVATE NONNULL((1)) void DCALL
seo_fini(SeqEachOperator *__restrict self) {
	Dee_Decref(self->se_seq);
	Dee_Decrefv(self->so_opargv, self->so_opargc);
}

PRIVATE NONNULL((1, 2)) void DCALL
seo_visit(SeqEachOperator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->se_seq);
	Dee_Visitv(self->so_opargv, self->so_opargc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sew_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return Dee_AsObject(seqeach_makeop1(self, OPERATOR_CALL, tuple));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
sew_call_kw(DeeObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	if (kw) {
		Dee_Incref(kw);
		return seqeach_makeop2(self, OPERATOR_CALL, tuple, kw);
	} else {
		return seqeach_makeop1(self, OPERATOR_CALL, tuple);
	}
err:
	return NULL;
}


#define DEFINE_SEW_UNARY(name, op)                           \
	PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL \
	name(DeeObject *__restrict self) {                       \
		return seqeach_makeop0(self, op);                    \
	}
#define DEFINE_SEW_BINARY(name, op)                             \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachOperator *DCALL \
	name(DeeObject *self, DeeObject *other) {                   \
		Dee_Incref(other);                                      \
		return seqeach_makeop1(self, op, other);                \
	}
#define DEFINE_SEW_TRINARY(name, op)                               \
	PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF SeqEachOperator *DCALL \
	name(DeeObject *self, DeeObject *a, DeeObject *b) {            \
		Dee_Incref(a);                                             \
		Dee_Incref(b);                                             \
		return seqeach_makeop2(self, op, a, b);                    \
	}
DEFINE_SEW_UNARY(sew_iter_next, OPERATOR_ITERNEXT)
DEFINE_SEW_UNARY(sew_inv, OPERATOR_INV)
DEFINE_SEW_UNARY(sew_pos, OPERATOR_POS)
DEFINE_SEW_UNARY(sew_neg, OPERATOR_NEG)
DEFINE_SEW_BINARY(sew_add, OPERATOR_ADD)
DEFINE_SEW_BINARY(sew_sub, OPERATOR_SUB)
DEFINE_SEW_BINARY(sew_mul, OPERATOR_MUL)
DEFINE_SEW_BINARY(sew_div, OPERATOR_DIV)
DEFINE_SEW_BINARY(sew_mod, OPERATOR_MOD)
DEFINE_SEW_BINARY(sew_shl, OPERATOR_SHL)
DEFINE_SEW_BINARY(sew_shr, OPERATOR_SHR)
DEFINE_SEW_BINARY(sew_and, OPERATOR_AND)
DEFINE_SEW_BINARY(sew_or, OPERATOR_OR)
DEFINE_SEW_BINARY(sew_xor, OPERATOR_XOR)
DEFINE_SEW_BINARY(sew_pow, OPERATOR_POW)
#ifndef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_COMPARE
DEFINE_SEW_BINARY(sew_eq, OPERATOR_EQ)
DEFINE_SEW_BINARY(sew_ne, OPERATOR_NE)
DEFINE_SEW_BINARY(sew_lo, OPERATOR_LO)
DEFINE_SEW_BINARY(sew_le, OPERATOR_LE)
DEFINE_SEW_BINARY(sew_gr, OPERATOR_GR)
DEFINE_SEW_BINARY(sew_ge, OPERATOR_GE)
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_COMPARE */

#ifndef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM
DEFINE_SEW_UNARY(sew_sizeob, OPERATOR_SIZE)
DEFINE_SEW_BINARY(sew_getitem, OPERATOR_GETITEM)
DEFINE_SEW_TRINARY(sew_getrange, OPERATOR_GETRANGE)
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM */

#ifndef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS
DEFINE_SEW_BINARY(sew_contains, OPERATOR_CONTAINS)
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS */

#ifndef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER
DEFINE_SEW_UNARY(sew_iter, OPERATOR_ITER)
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER */


struct seo_inplace_foreach_data {
	SeqEachOperator *sifd_self;
	int (DCALL      *sifd_op)(DREF DeeObject **__restrict p_self, DeeObject *other);
	DeeObject       *sifd_other;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seo_inplace_foreach_cb(void *arg, DeeObject *elem) {
	int result;
	DREF DeeObject *elem_value;
	struct seo_inplace_foreach_data *data;
	SeqEachOperator *self;
	data = (struct seo_inplace_foreach_data *)arg;
	self = data->sifd_self;
	switch (self->so_opname) {
	case OPERATOR_GETATTR:
		elem_value = DeeObject_GetAttr(elem, self->so_opargv[0]);
		break;
	case OPERATOR_GETITEM:
		elem_value = DeeObject_GetItem(elem, self->so_opargv[0]);
		break;
	case OPERATOR_GETRANGE:
		elem_value = DeeObject_GetRange(elem, self->so_opargv[0], self->so_opargv[1]);
		break;
	default: __builtin_unreachable();
	}
	if unlikely(!elem_value)
		goto err;
	if unlikely((*data->sifd_op)(&elem_value, data->sifd_other))
		goto err_elem_value;
	switch (self->so_opname) {
	case OPERATOR_GETATTR:
		result = DeeObject_SetAttr(elem, self->so_opargv[0], elem_value);
		break;
	case OPERATOR_GETITEM:
		result = DeeObject_SetItem(elem, self->so_opargv[0], elem_value);
		break;
	case OPERATOR_GETRANGE:
		result = DeeObject_SetRange(elem, self->so_opargv[0], self->so_opargv[1], elem_value);
		break;
	default: __builtin_unreachable();
	}
	Dee_Decref(elem_value);
	return result;
err_elem_value:
	Dee_Decref(elem_value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_inplace(SeqEachOperator *self,
            int (DCALL *op)(DREF DeeObject **__restrict p_self, DeeObject *other),
            DeeObject *other, Dee_operator_t opname) {
	struct seo_inplace_foreach_data data;
	switch (self->so_opname) {

		/* Only a select few operators can be used for inplace. */
	case OPERATOR_GETATTR:
		if unlikely(self->so_opargc < 1)
			goto err_noimp;
		if (DeeObject_AssertTypeExact(self->so_opargv[0], &DeeString_Type))
			goto err_noimp;
		break;

	case OPERATOR_GETITEM:
		if unlikely(self->so_opargc < 1)
			goto err_noimp;
		break;

	case OPERATOR_GETRANGE:
		if unlikely(self->so_opargc < 2)
			goto err_noimp;
		break;

	default: goto err_noimp;
	}
	data.sifd_self  = self;
	data.sifd_op    = op;
	data.sifd_other = other;
	return (int)DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                       &seo_inplace_foreach_cb, &data);
err_noimp:
	return err_unimplemented_operator(&SeqEachOperator_Type, opname);
}

#define DEFINE_SEO_BINARY_INPLACE(name, func, opname)             \
	PRIVATE NONNULL((1, 2)) int DCALL                             \
	name(SeqEachOperator **__restrict p_self, DeeObject *other) { \
		return seo_inplace(*p_self, &func, other, opname);        \
	}
PRIVATE NONNULL((1)) int DCALL seo_inc(SeqEachOperator **__restrict p_self) {
	return seo_inplace(*p_self, &inplace_inc_wrapper, NULL, OPERATOR_INC);
}
PRIVATE NONNULL((1)) int DCALL seo_dec(SeqEachOperator **__restrict p_self) {
	return seo_inplace(*p_self, &inplace_dec_wrapper, NULL, OPERATOR_DEC);
}

DEFINE_SEO_BINARY_INPLACE(seo_inplace_add, DeeObject_InplaceAdd, OPERATOR_INPLACE_ADD)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_sub, DeeObject_InplaceSub, OPERATOR_INPLACE_SUB)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_mul, DeeObject_InplaceMul, OPERATOR_INPLACE_MUL)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_div, DeeObject_InplaceDiv, OPERATOR_INPLACE_DIV)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_mod, DeeObject_InplaceMod, OPERATOR_INPLACE_MOD)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_shl, DeeObject_InplaceShl, OPERATOR_INPLACE_SHL)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_shr, DeeObject_InplaceShr, OPERATOR_INPLACE_SHR)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_and, DeeObject_InplaceAnd, OPERATOR_INPLACE_AND)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_or, DeeObject_InplaceOr, OPERATOR_INPLACE_OR)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_xor, DeeObject_InplaceXor, OPERATOR_INPLACE_XOR)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_pow, DeeObject_InplacePow, OPERATOR_INPLACE_POW)
#undef DEFINE_SEO_BINARY_INPLACE



PRIVATE struct type_math seo_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_pow,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&seo_inc,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&seo_dec,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_add,
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_sub,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_mul,
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_div,
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_mod,
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_shl,
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_shr,
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_xor,
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_pow,
};

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_COMPARE
PRIVATE struct type_cmp sew_cmp = {
	/* .tp_hash          = */ &default__seq_operator_hash__with__seq_operator_foreach,
	/* .tp_compare_eq    = */ &default__seq_operator_compare_eq__with__seq_operator_foreach,
	/* .tp_compare       = */ &default__seq_operator_compare__with__seq_operator_foreach,
	/* .tp_trycompare_eq = */ &default__seq_operator_trycompare_eq__with__seq_operator_compare_eq,
	/* .tp_eq            = */ &default__seq_operator_eq__with__seq_operator_compare_eq,
	/* .tp_ne            = */ &default__seq_operator_ne__with__seq_operator_compare_eq,
	/* .tp_lo            = */ &default__seq_operator_lo__with__seq_operator_compare,
	/* .tp_le            = */ &default__seq_operator_le__with__seq_operator_compare,
	/* .tp_gr            = */ &default__seq_operator_gr__with__seq_operator_compare,
	/* .tp_ge            = */ &default__seq_operator_ge__with__seq_operator_compare,
};
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_COMPARE */
#define sew_cmp noseq__sew_cmp
PRIVATE struct type_cmp noseq__sew_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_ge,
};
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_COMPARE */


PRIVATE WUNUSED NONNULL((1)) int DCALL
sew_enter(DeeObject *__restrict self) {
	DREF DeeObject **elem;
	size_t i, count;
	elem = DeeSeq_AsHeapVector(self, &count);
	if unlikely(!elem)
		goto err;
	for (i = 0; i < count; ++i) {
		if (DeeObject_Enter(elem[i]))
			goto err_elem_i;
	}
	Dee_Decrefv(elem, count);
	Dee_Free(elem);
	return 0;
err_elem_i:
	while (i--) {
		if unlikely(DeeObject_Leave(elem[i]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
	}
/*err_elem:*/
	Dee_Decrefv(elem, count);
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sew_leave(DeeObject *__restrict self) {
	DREF DeeObject **elem;
	size_t count;
	elem = DeeSeq_AsHeapVector(self, &count);
	if unlikely(!elem)
		goto err;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			goto err_elem_count;
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
	return 0;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
err_elem_count:
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE struct type_with sew_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&sew_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&sew_leave,
};


PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
sew_iterattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
             struct Dee_attriter *iterbuf, size_t bufsize,
             struct Dee_attrhint const *__restrict hint) {
	return se_iterattr_impl(Dee_AsObject(self), iterbuf, bufsize, hint);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
sew_findattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
             struct Dee_attrspec const *__restrict specs,
             struct Dee_attrdesc *__restrict result) {
	return se_findattr_impl(Dee_AsObject(self), specs, result);
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_transform(SeqEachOperator *self, DeeObject *elem) {
	return DeeObject_InvokeOperator(elem,
	                                self->so_opname,
	                                self->so_opargc,
	                                self->so_opargv);
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_transform_inherit(SeqEachOperator *self,
                      /*inherit(always)*/ DREF DeeObject *elem) {
	DREF DeeObject *result;
	result = seo_transform(self, elem);
	Dee_Decref(elem);
	return result;
}

struct seo_foreach_data {
	SeqEachOperator *seofd_me;   /* [1..1] The related seq-each operator */
	Dee_foreach_t    seofd_proc; /* [1..1] User-defined callback */
	void            *seofd_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct seo_foreach_data *data;
	data = (struct seo_foreach_data *)arg;
	elem = seo_transform(data->seofd_me, elem);
	if unlikely(!elem)
		goto err;
	result = (*data->seofd_proc)(data->seofd_arg, elem);
	Dee_Decref(elem);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_foreach(SeqEachOperator *__restrict self, Dee_foreach_t proc, void *arg) {
	struct seo_foreach_data data;
	data.seofd_me   = self;
	data.seofd_proc = proc;
	data.seofd_arg  = arg;
	return DeeObject_Foreach(self->se_seq, &seo_foreach_cb, &data);
}

struct seo_mh_seq_enumerate_data {
	SeqEachOperator *seoed_me;   /* [1..1] The related seq-each operator */
	Dee_seq_enumerate_t  seoed_proc; /* [1..1] User-defined callback */
	void            *seoed_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_mh_seq_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct seo_mh_seq_enumerate_data *data;
	data = (struct seo_mh_seq_enumerate_data *)arg;
	if unlikely(!value)
		return (*data->seoed_proc)(data->seoed_arg, index, value);
	value = seo_transform(data->seoed_me, value);
	if unlikely(!value)
		goto err;
	result = (*data->seoed_proc)(data->seoed_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_mh_seq_enumerate(SeqEachOperator *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	struct seo_mh_seq_enumerate_data data;
	data.seoed_me   = self;
	data.seoed_proc = proc;
	data.seoed_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->se_seq,
	                                  &seo_mh_seq_enumerate_cb,
	                                  &data);
}

struct seo_mh_seq_enumerate_index_data {
	SeqEachOperator          *seoeid_me;   /* [1..1] The related seq-each operator */
	Dee_seq_enumerate_index_t seoeid_proc; /* [1..1] User-defined callback */
	void                     *seoeid_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seo_mh_seq_enumerate_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct seo_mh_seq_enumerate_index_data *data;
	data = (struct seo_mh_seq_enumerate_index_data *)arg;
	if unlikely(!value)
		return (*data->seoeid_proc)(data->seoeid_arg, index, value);
	value = seo_transform(data->seoeid_me, value);
	if unlikely(!value)
		goto err;
	result = (*data->seoeid_proc)(data->seoeid_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_mh_seq_enumerate_index(SeqEachOperator *__restrict self, Dee_seq_enumerate_index_t proc,
                           void *arg, size_t start, size_t end) {
	struct seo_mh_seq_enumerate_index_data data;
	data.seoeid_me   = self;
	data.seoeid_proc = proc;
	data.seoeid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->se_seq,
	                                  &seo_mh_seq_enumerate_index_cb,
	                                  &data, start, end);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_foreach_bool_cb(void *arg, DeeObject *elem) {
	int result;
	(void)arg;
	result = DeeObject_Bool(elem);
	if unlikely(result < 0)
		goto err;
	if (result == 0)
		return SEQEACH_FOREACH_NO;
	return SEQEACH_FOREACH_YES;
err:
	return SEQEACH_FOREACH_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_bool(SeqEachOperator *__restrict self) {
	Dee_ssize_t result = seo_foreach(self, &se_foreach_bool_cb, NULL);
	ASSERT(result == SEQEACH_FOREACH_YES ||
	       result == SEQEACH_FOREACH_ERR ||
	       result == SEQEACH_FOREACH_NO);
	switch (result) {
	case SEQEACH_FOREACH_NO:
		return 0;
	case SEQEACH_FOREACH_YES:
		return 1;
	case SEQEACH_FOREACH_ERR:
		return -1;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_assign(SeqEachOperator *self, DeeObject *value) {
	return (int)seo_foreach(self, &se_foreach_assign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_moveassign(SeqEachOperator *self, DeeObject *value) {
	return (int)seo_foreach(self, &se_foreach_moveassign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_delitem(SeqEachOperator *self, DeeObject *index) {
	return (int)seo_foreach(self, &se_foreach_delitem_cb, index);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
seo_setitem(SeqEachOperator *self, DeeObject *index, DeeObject *value) {
	struct se_foreach_setitem_data data;
	data.sfesi_index = index;
	data.sfesi_value = value;
	return (int)seo_foreach(self, &se_foreach_setitem_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
seo_delrange(SeqEachOperator *self, DeeObject *start, DeeObject *end) {
	struct se_foreach_delrange_data data;
	data.sfedr_start = start;
	data.sfedr_end   = end;
	return (int)seo_foreach(self, &se_foreach_delrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
seo_setrange(SeqEachOperator *self, DeeObject *start,
             DeeObject *end, DeeObject *value) {
	struct se_foreach_setrange_data data;
	data.sfesr_start = start;
	data.sfesr_end   = end;
	data.sfesr_value = value;
	return (int)seo_foreach(self, &se_foreach_setrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_delitem_index(SeqEachOperator *self, size_t index) {
	return (int)seo_foreach(self, &se_foreach_delitem_index_cb, (void *)(uintptr_t)index);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
seo_setitem_index(SeqEachOperator *self, size_t index, DeeObject *value) {
	struct se_foreach_setitem_index_data data;
	data.sfesii_index = index;
	data.sfesii_value = value;
	return (int)seo_foreach(self, &se_foreach_setitem_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_delrange_index(SeqEachOperator *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct se_foreach_delrange_index_data data;
	data.sfedri_start = start;
	data.sfedri_end   = end;
	return (int)seo_foreach(self, &se_foreach_delrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_delrange_index_n(SeqEachOperator *self, Dee_ssize_t start) {
	return (int)seo_foreach(self, &se_foreach_delrange_index_n_cb,
	                        (void *)(uintptr_t)(size_t)start);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
seo_setrange_index(SeqEachOperator *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value) {
	struct se_foreach_setrange_index_data data;
	data.sfesri_start = start;
	data.sfesri_end   = end;
	data.sfesri_value = value;
	return (int)seo_foreach(self, &se_foreach_setrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
seo_setrange_index_n(SeqEachOperator *self, Dee_ssize_t start, DeeObject *value) {
	struct se_foreach_setrange_index_n_data data;
	data.sfesrin_start = start;
	data.sfesrin_value = value;
	return (int)seo_foreach(self, &se_foreach_setrange_index_n_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_delitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct se_foreach_delitem_string_hash_data data;
	data.sfedish_key  = key;
	data.sfedish_hash = hash;
	return (int)seo_foreach(self, &se_foreach_delitem_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_delitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_foreach_delitem_string_len_hash_data data;
	data.sfedislh_key    = key;
	data.sfedislh_keylen = keylen;
	data.sfedislh_hash   = hash;
	return (int)seo_foreach(self, &se_foreach_delitem_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
seo_setitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_hash_data data;
	data.sfesish_key   = key;
	data.sfesish_hash  = hash;
	data.sfesish_value = value;
	return (int)seo_foreach(self, &se_foreach_setitem_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
seo_setitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen,
                            Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_len_hash_data data;
	data.sfesislh_key    = key;
	data.sfesislh_keylen = keylen;
	data.sfesislh_hash   = hash;
	data.sfesislh_value  = value;
	return (int)seo_foreach(self, &se_foreach_setitem_string_len_hash_cb, &data);
}





PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_delattr(SeqEachOperator *self, DeeObject *attr) {
	return (int)seo_foreach(self, &se_foreach_delattr_cb, attr);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_delattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct se_foreach_delattr_string_hash_data data;
	data.sfedsh_attr = attr;
	data.sfedsh_hash = hash;
	return (int)seo_foreach(self, &se_foreach_delattr_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_delattr_string_len_hash(SeqEachOperator *self, char const *attr,
                            size_t attrlen, Dee_hash_t hash) {
	struct se_foreach_delattr_string_len_hash_data data;
	data.sfedslh_attr    = attr;
	data.sfedslh_attrlen = attrlen;
	data.sfedslh_hash    = hash;
	return (int)seo_foreach(self, &se_foreach_delattr_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
seo_setattr(SeqEachOperator *self, DeeObject *attr, DeeObject *value) {
	struct se_foreach_setattr_data data;
	data.sfes_attr  = attr;
	data.sfes_value = value;
	return (int)seo_foreach(self, &se_foreach_setattr_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
seo_setattr_string_hash(SeqEachOperator *self, char const *attr,
                        Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setattr_string_hash_data data;
	data.sfessh_attr  = attr;
	data.sfessh_hash  = hash;
	data.sfessh_value = value;
	return (int)seo_foreach(self, &se_foreach_setattr_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
seo_setattr_string_len_hash(SeqEachOperator *self, char const *attr,
                            size_t attrlen, Dee_hash_t hash,
                            DeeObject *value) {
	struct se_foreach_setattr_string_len_hash_data data;
	data.sfesslh_attr    = attr;
	data.sfesslh_attrlen = attrlen;
	data.sfesslh_hash    = hash;
	data.sfesslh_value   = value;
	return (int)seo_foreach(self, &se_foreach_setattr_string_len_hash_cb, &data);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_boundattr(SeqEachOperator *self, DeeObject *attr) {
	return seqeach_map_fe2bound(seo_foreach(self, &se_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasattr(SeqEachOperator *self, DeeObject *attr) {
	return seqeach_map_fe2has(seo_foreach(self, &se_hasattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_boundattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_fe2bound(seo_foreach(self, &se_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_fe2has(seo_foreach(self, &se_hasattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_boundattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_fe2bound(seo_foreach(self, &se_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_fe2has(seo_foreach(self, &se_hasattr_string_len_hash_foreach_cb, &data));
}


#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define sew_getattr seqeachw_getattr
INTERN WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL
seqeachw_getattr(DeeObject *__restrict self,
                 DeeStringObject *__restrict attr) {
	DREF SeqEachGetAttr *result;
	result = DeeObject_MALLOC(SeqEachGetAttr);
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = attr;
	Dee_Incref(self);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqEachGetAttr_Type);
done:
	return result;
}

#define sew_callattr                    DeeSeqEach_CallAttr
#define sew_callattr_kw                 DeeSeqEach_CallAttrKw
#define sew_callattr_string_hash        DeeSeqEach_CallAttrStringHash
#define sew_callattr_string_hash_kw     DeeSeqEach_CallAttrStringHashKw
#define sew_callattr_string_len_hash    DeeSeqEach_CallAttrStringLenHash
#define sew_callattr_string_len_hash_kw DeeSeqEach_CallAttrStringLenHashKw
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
DEFINE_SEW_BINARY(sew_getattr, OPERATOR_GETATTR)
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */

PRIVATE struct type_attr seo_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&sew_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&seo_delattr,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&seo_setattr,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&sew_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&sew_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_boundattr,
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&sew_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&sew_callattr_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_delattr_string_hash,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&seo_setattr_string_hash,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_hasattr_string_hash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_boundattr_string_hash,
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&sew_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&sew_callattr_string_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&seo_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_boundattr_string_len_hash,
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&sew_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&sew_callattr_string_len_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_findattr_info_string_len_hash = */ NULL,
};

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachIterator *DCALL
seo_mh_seq_operator_iter(SeqEachOperator *__restrict self) {
	DREF SeqEachIterator *result;
	result = DeeObject_MALLOC(SeqEachIterator);
	if unlikely(!result)
		goto done;
	result->ei_each = (DREF SeqEachBase *)self;
	result->ei_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->se_seq);
	if unlikely(!result->ei_iter)
		goto err_r;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqEachOperatorIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

STATIC_ASSERT(offsetof(SeqEachBase, se_seq) == offsetof(ProxyObject, po_obj));
#define sew_size_fast generic_proxy__size_fast

STATIC_ASSERT(offsetof(SeqEachBase, se_seq) == offsetof(ProxyObject, po_obj));
#define seo_mh_seq_operator_size             generic_proxy__seq_operator_size
#define seo_mh_seq_operator_sizeob           generic_proxy__seq_operator_sizeob
#define seo_mh_seq_operator_bounditem        generic_proxy__seq_operator_bounditem
#define seo_mh_seq_operator_bounditem_index  generic_proxy__seq_operator_bounditem_index
#define seo_mh_seq_operator_hasitem          generic_proxy__seq_operator_hasitem
#define seo_mh_seq_operator_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define seo_mh_seq_operator_delitem          generic_proxy__seq_operator_delitem
#define seo_mh_seq_operator_delitem_index    generic_proxy__seq_operator_delitem_index
#define seo_mh_seq_operator_delrange         generic_proxy__seq_operator_delrange
#define seo_mh_seq_operator_delrange_index   generic_proxy__seq_operator_delrange_index
#define seo_mh_seq_operator_delrange_index_n generic_proxy__seq_operator_delrange_index_n


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_mh_seq_operator_getitem(SeqEachOperator *self, DeeObject *index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_getitem,
	                                                    self->se_seq, index);
	if likely(result)
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seo_mh_seq_operator_getitem_index(SeqEachOperator *self, size_t index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_getitem_index,
	                                                    self->se_seq, index);
	if likely(result)
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_mh_seq_operator_trygetitem(SeqEachOperator *self, DeeObject *index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_trygetitem,
	                                                    self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seo_mh_seq_operator_trygetitem_index(SeqEachOperator *self, size_t index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index,
	                                                    self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachOperator *DCALL
seo_wraprange(SeqEachOperator *self, /*inherit(always)*/ DREF DeeObject *base) {
	DREF SeqEachOperator *result;
	switch (self->so_opargc) {
	case 0: result = SeqEachOperator_MALLOC(0); break;
	case 1: result = SeqEachOperator_MALLOC(1); break;
	case 2: result = SeqEachOperator_MALLOC(2); break;
	default: __builtin_unreachable();
	}
	if unlikely(!result)
		goto err_base;
	result->se_seq    = base; /* Inherit reference. */
	result->so_opname = self->so_opname;
	result->so_opargc = self->so_opargc;
	Dee_Movrefv(result->so_opargv, self->so_opargv, self->so_opargc);
	DeeObject_Init(result, &SeqEachOperator_Type);
	return result;
err_base:
	Dee_Decref(base);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF SeqEachOperator *DCALL
seo_mh_seq_operator_getrange(SeqEachOperator *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *base = DeeObject_InvokeMethodHint(seq_operator_getrange,
	                                                  self->se_seq, start, end);
	if unlikely(!base)
		goto err;
	return seo_wraprange(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seo_mh_seq_operator_getrange_index(SeqEachOperator *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *base = DeeObject_InvokeMethodHint(seq_operator_getrange_index,
	                                                  self->se_seq, start, end);
	if unlikely(!base)
		goto err;
	return seo_wraprange(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seo_mh_seq_operator_getrange_index_n(SeqEachOperator *self, Dee_ssize_t start) {
	DREF DeeObject *base = DeeObject_InvokeMethodHint(seq_operator_getrange_index_n,
	                                                  self->se_seq, start);
	if unlikely(!base)
		goto err;
	return seo_wraprange(self, base);
err:
	return NULL;
}

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM
#define seo_operator_size_PTR                       &seo_mh_seq_operator_size
#define seo_operator_sizeob_PTR                     &seo_mh_seq_operator_sizeob
#define seo_operator_getitem_PTR                    &seo_mh_seq_operator_getitem
#define seo_operator_getitem_index_PTR              &seo_mh_seq_operator_getitem_index
#define seo_operator_trygetitem_PTR                 &seo_mh_seq_operator_trygetitem
#define seo_operator_trygetitem_index_PTR           &seo_mh_seq_operator_trygetitem_index
#define seo_operator_bounditem_PTR                  &seo_mh_seq_operator_bounditem
#define seo_operator_bounditem_index_PTR            &seo_mh_seq_operator_bounditem_index
#define seo_operator_hasitem_PTR                    &seo_mh_seq_operator_hasitem
#define seo_operator_hasitem_index_PTR              &seo_mh_seq_operator_hasitem_index
#define seo_operator_trygetitem_string_hash_PTR     DEFIMPL(&default__trygetitem_string_hash__with__trygetitem)
#define seo_operator_trygetitem_string_len_hash_PTR DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem)
#define seo_operator_getitem_string_hash_PTR        DEFIMPL(&default__getitem_string_hash__with__getitem)
#define seo_operator_getitem_string_len_hash_PTR    DEFIMPL(&default__getitem_string_len_hash__with__getitem)
#define seo_operator_bounditem_string_hash_PTR      DEFIMPL(&default__bounditem_string_hash__with__bounditem)
#define seo_operator_bounditem_string_len_hash_PTR  DEFIMPL(&default__bounditem_string_len_hash__with__bounditem)
#define seo_operator_hasitem_string_hash_PTR        DEFIMPL(&default__hasitem_string_hash__with__hasitem)
#define seo_operator_hasitem_string_len_hash_PTR    DEFIMPL(&default__hasitem_string_len_hash__with__hasitem)
#define seo_operator_getrange_PTR                   &seo_mh_seq_operator_getrange
#define seo_operator_getrange_index_PTR             &seo_mh_seq_operator_getrange_index
#define seo_operator_getrange_index_n_PTR           &seo_mh_seq_operator_getrange_index_n
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM */
#define seo_operator_bounditem_PTR &seo_operator_bounditem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_operator_bounditem(SeqEachOperator *self, DeeObject *index) {
	return seqeach_map_fe2bound(seo_foreach(self, &se_bounditem_foreach_cb, index));
}

#define seo_operator_hasitem_PTR &seo_operator_hasitem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_operator_hasitem(SeqEachOperator *self, DeeObject *index) {
	return seqeach_map_fe2has(seo_foreach(self, &se_hasitem_foreach_cb, index));
}

#define seo_operator_bounditem_index_PTR &seo_operator_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_operator_bounditem_index(SeqEachOperator *self, size_t index) {
	return seqeach_map_fe2bound(seo_foreach(self, &se_bounditem_index_foreach_cb,
	                                        (void *)(uintptr_t)index));
}

#define seo_operator_hasitem_index_PTR &seo_operator_hasitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_operator_hasitem_index(SeqEachOperator *self, size_t index) {
	return seqeach_map_fe2has(seo_foreach(self, &se_hasitem_index_foreach_cb,
	                                      (void *)(uintptr_t)index));
}

#define seo_operator_bounditem_string_hash_PTR &seo_operator_bounditem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_operator_bounditem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_fe2bound(seo_foreach(self, &se_bounditem_string_hash_foreach_cb, &data));
}

#define seo_operator_hasitem_string_hash_PTR &seo_operator_hasitem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_operator_hasitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_fe2has(seo_foreach(self, &se_hasitem_string_hash_foreach_cb, &data));
}

#define seo_operator_bounditem_string_len_hash_PTR &seo_operator_bounditem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_operator_bounditem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_fe2bound(seo_foreach(self, &se_bounditem_string_len_hash_foreach_cb, &data));
}

#define seo_operator_hasitem_string_len_hash_PTR &seo_operator_hasitem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_operator_hasitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_fe2has(seo_foreach(self, &se_hasitem_string_len_hash_foreach_cb, &data));
}

#define seo_operator_sizeob_PTR                     &sew_sizeob
#define seo_operator_size_PTR                       &default__size__with__sizeob
#define seo_operator_getitem_PTR                    &sew_getitem
#define seo_operator_getitem_index_PTR              &default__getitem_index__with__getitem
#define seo_operator_getitem_string_hash_PTR        &default__getitem_string_hash__with__getitem
#define seo_operator_getitem_string_len_hash_PTR    &default__getitem_string_len_hash__with__getitem
#define seo_operator_trygetitem_PTR                 &sew_getitem
#define seo_operator_trygetitem_index_PTR           &default__getitem_index__with__getitem
#define seo_operator_trygetitem_string_hash_PTR     &default__getitem_string_hash__with__getitem
#define seo_operator_trygetitem_string_len_hash_PTR &default__getitem_string_len_hash__with__getitem
#define seo_operator_getrange_PTR                   &sew_getrange
#define seo_operator_getrange_index_PTR             &default__getrange_index__with__getrange
#define seo_operator_getrange_index_n_PTR           &default__getrange_index_n__with__getrange
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM */

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS
#define seo_operator_contains_PTR &default__seq_operator_contains__with__seq_contains
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS */
#define seo_operator_contains_PTR &sew_contains
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS */

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER
#define seo_operator_iter_PTR    &seo_mh_seq_operator_iter
#define seo_operator_foreach_PTR &seo_foreach
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER */
#define seo_operator_iter_PTR    &sew_iter
#define seo_operator_foreach_PTR &default__foreach__with__iter
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER */


PRIVATE struct type_seq seo_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))seo_operator_iter_PTR,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))seo_operator_sizeob_PTR,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))seo_operator_contains_PTR,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))seo_operator_getitem_PTR,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&seo_setitem,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))seo_operator_getrange_PTR,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&seo_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&seo_setrange,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))seo_operator_foreach_PTR,
	/* .tp_foreach_pair               = */ &default__foreach_pair__with__foreach,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))seo_operator_bounditem_PTR,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))seo_operator_hasitem_PTR,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))seo_operator_size_PTR,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&sew_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))seo_operator_getitem_index_PTR,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&seo_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&seo_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))seo_operator_bounditem_index_PTR,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))seo_operator_hasitem_index_PTR,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))seo_operator_getrange_index_PTR,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&seo_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&seo_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))seo_operator_getrange_index_n_PTR,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&seo_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&seo_setrange_index_n,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))seo_operator_trygetitem_PTR,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))seo_operator_trygetitem_index_PTR,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))seo_operator_trygetitem_string_hash_PTR,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))seo_operator_getitem_string_hash_PTR,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&seo_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))seo_operator_bounditem_string_hash_PTR,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))seo_operator_hasitem_string_hash_PTR,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))seo_operator_trygetitem_string_len_hash_PTR,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))seo_operator_getitem_string_len_hash_PTR,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&seo_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))seo_operator_bounditem_string_len_hash_PTR,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))seo_operator_hasitem_string_len_hash_PTR,
};

PRIVATE struct type_method tpconst sew_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_iter__),
	TYPE_METHOD_HINTREF(__seq_getitem__),
	TYPE_METHOD_HINTREF(__seq_delitem__),
	TYPE_METHOD_HINTREF(__seq_getrange__),
	TYPE_METHOD_HINTREF(__seq_delrange__),
	TYPE_METHOD_HINTREF(__seq_size__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst seo_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate, &seo_mh_seq_enumerate),
	TYPE_METHOD_HINT(seq_enumerate_index, &seo_mh_seq_enumerate_index),
	TYPE_METHOD_HINT(seq_operator_iter, &seo_mh_seq_operator_iter),
	TYPE_METHOD_HINT(seq_operator_foreach, &seo_foreach),
	TYPE_METHOD_HINT(seq_operator_getitem, &seo_mh_seq_operator_getitem),
	TYPE_METHOD_HINT(seq_operator_getitem_index, &seo_mh_seq_operator_getitem_index),
	TYPE_METHOD_HINT(seq_operator_trygetitem, &seo_mh_seq_operator_trygetitem),
	TYPE_METHOD_HINT(seq_operator_trygetitem_index, &seo_mh_seq_operator_trygetitem_index),
	TYPE_METHOD_HINT(seq_operator_bounditem, &seo_mh_seq_operator_bounditem),
	TYPE_METHOD_HINT(seq_operator_bounditem_index, &seo_mh_seq_operator_bounditem_index),
	TYPE_METHOD_HINT(seq_operator_hasitem, &seo_mh_seq_operator_hasitem),
	TYPE_METHOD_HINT(seq_operator_hasitem_index, &seo_mh_seq_operator_hasitem_index),
	TYPE_METHOD_HINT(seq_operator_delitem, &seo_mh_seq_operator_delitem),
	TYPE_METHOD_HINT(seq_operator_delitem_index, &seo_mh_seq_operator_delitem_index),
	TYPE_METHOD_HINT(seq_operator_getrange, &seo_mh_seq_operator_getrange),
	TYPE_METHOD_HINT(seq_operator_getrange_index, &seo_mh_seq_operator_getrange_index),
	TYPE_METHOD_HINT(seq_operator_getrange_index_n, &seo_mh_seq_operator_getrange_index_n),
	TYPE_METHOD_HINT(seq_operator_delrange, &seo_mh_seq_operator_delrange),
	TYPE_METHOD_HINT(seq_operator_delrange_index, &seo_mh_seq_operator_delrange_index),
	TYPE_METHOD_HINT(seq_operator_delrange_index_n, &seo_mh_seq_operator_delrange_index_n),
	TYPE_METHOD_HINT(seq_operator_size, &seo_mh_seq_operator_size),
	TYPE_METHOD_HINT(seq_operator_sizeob, &seo_mh_seq_operator_sizeob),
	TYPE_METHOD_HINT_END
};


#define seo_members se_members  /* TODO: Access to operator name & arguments */
#define sso_members seo_members

PRIVATE struct type_member tpconst seo_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqEachOperatorIterator_Type),
	TYPE_MEMBER_END
};

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR
#define seo_operator_printrepr_PTR &default_seq_printrepr
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR */
#define seo_operator_printrepr_PTR &seo_printrepr
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR */


PRIVATE struct type_callable sew_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&sew_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

INTERN DeeTypeObject SeqEachOperator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEachOperator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(" seo__SeqEachOperator_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(
			/* min_tp_instance_size: */ offsetof(SeqEachOperator, so_opargv),
			/* max_tp_instance_size: */ sizeof(SeqEachOperator),
			/* tp_ctor:              */ &seo_ctor,
			/* tp_copy_ctor:         */ &seo_copy,
			/* tp_deep_ctor:         */ &seo_deep,
			/* tp_any_ctor:          */ &seo_init,
			/* tp_any_ctor_kw:       */ NULL,
			/* tp_serialize:         */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seo_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_moveassign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&seo_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))seo_operator_printrepr_PTR,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&seo_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &seo_math,
	/* .tp_cmp           = */ &sew_cmp,
	/* .tp_seq           = */ &seo_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ &seo_attr,
	/* .tp_with          = */ &sew_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sew_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seo_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ seo_class_members,
	/* .tp_method_hints  = */ seo_method_hints,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sew_call,
	/* .tp_callable      = */ &sew_callable,
};


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_foreach_bool_cb(void *arg, DeeObject *elem) {
	int result;
	(void)arg;
	result = DeeObject_Bool(elem);
	if unlikely(result < 0)
		goto err;
	if (result == 0)
		return SEQSOME_FOREACH_NO;
	return SEQSOME_FOREACH_YES;
err:
	return SEQSOME_FOREACH_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sso_bool(SeqEachOperator *__restrict self) {
	Dee_ssize_t result = seo_foreach(self, &ss_foreach_bool_cb, NULL);
	ASSERT(result == SEQSOME_FOREACH_YES ||
	       result == SEQSOME_FOREACH_ERR ||
	       result == SEQSOME_FOREACH_NO);
	switch (result) {
	case SEQSOME_FOREACH_NO:
		return 0;
	case SEQSOME_FOREACH_YES:
		return 1;
	case SEQSOME_FOREACH_ERR:
		return -1;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sso_printrepr(SeqEachOperator *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	char const *some_suffix = ".some";
	DeeTypeObject *seq_type = Dee_TYPE(self->se_seq);
	if (DeeType_IsSeqSomeWrapper(seq_type))
		some_suffix = NULL;
	return DeeFormat_PrintOperatorRepr(printer, arg,
	                                   self->se_seq, self->so_opname,
	                                   self->so_opargc, self->so_opargv,
	                                   NULL, 0,
	                                   some_suffix, some_suffix ? 5 : 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ssw_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return Dee_AsObject(seqsome_makeop1(self, OPERATOR_CALL, tuple));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
ssw_call_kw(DeeObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	if (kw) {
		Dee_Incref(kw);
		return seqsome_makeop2(self, OPERATOR_CALL, tuple, kw);
	} else {
		return seqsome_makeop1(self, OPERATOR_CALL, tuple);
	}
err:
	return NULL;
}


#define DEFINE_SSW_UNARY(name, op)                           \
	PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL \
	name(DeeObject *__restrict self) {                       \
		return seqsome_makeop0(self, op);                    \
	}
#define DEFINE_SSW_BINARY(name, op)                             \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF SeqEachOperator *DCALL \
	name(DeeObject *self, DeeObject *other) {                   \
		Dee_Incref(other);                                      \
		return seqsome_makeop1(self, op, other);                \
	}
#define DEFINE_SSW_TRINARY(name, op)                               \
	PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF SeqEachOperator *DCALL \
	name(DeeObject *self, DeeObject *a, DeeObject *b) {            \
		Dee_Incref(a);                                             \
		Dee_Incref(b);                                             \
		return seqsome_makeop2(self, op, a, b);                    \
	}
DEFINE_SSW_UNARY(ssw_iter, OPERATOR_ITER)
DEFINE_SSW_UNARY(ssw_iter_next, OPERATOR_ITERNEXT)
DEFINE_SSW_UNARY(ssw_inv, OPERATOR_INV)
DEFINE_SSW_UNARY(ssw_pos, OPERATOR_POS)
DEFINE_SSW_UNARY(ssw_neg, OPERATOR_NEG)
DEFINE_SSW_BINARY(ssw_add, OPERATOR_ADD)
DEFINE_SSW_BINARY(ssw_sub, OPERATOR_SUB)
DEFINE_SSW_BINARY(ssw_mul, OPERATOR_MUL)
DEFINE_SSW_BINARY(ssw_div, OPERATOR_DIV)
DEFINE_SSW_BINARY(ssw_mod, OPERATOR_MOD)
DEFINE_SSW_BINARY(ssw_shl, OPERATOR_SHL)
DEFINE_SSW_BINARY(ssw_shr, OPERATOR_SHR)
DEFINE_SSW_BINARY(ssw_and, OPERATOR_AND)
DEFINE_SSW_BINARY(ssw_or, OPERATOR_OR)
DEFINE_SSW_BINARY(ssw_xor, OPERATOR_XOR)
DEFINE_SSW_BINARY(ssw_pow, OPERATOR_POW)
DEFINE_SSW_BINARY(ssw_eq, OPERATOR_EQ)
DEFINE_SSW_BINARY(ssw_ne, OPERATOR_NE)
DEFINE_SSW_BINARY(ssw_lo, OPERATOR_LO)
DEFINE_SSW_BINARY(ssw_le, OPERATOR_LE)
DEFINE_SSW_BINARY(ssw_gr, OPERATOR_GR)
DEFINE_SSW_BINARY(ssw_ge, OPERATOR_GE)
DEFINE_SSW_UNARY(ssw_sizeob, OPERATOR_SIZE)
DEFINE_SSW_BINARY(ssw_contains, OPERATOR_CONTAINS)
DEFINE_SSW_BINARY(ssw_getitem, OPERATOR_GETITEM)
DEFINE_SSW_TRINARY(ssw_getrange, OPERATOR_GETRANGE)

PRIVATE struct type_math ssw_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_pow,
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL(&default__dec__with__sub),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__inplace_sub__with__sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL(&default__inplace_div__with__div),
	/* .tp_inplace_mod = */ DEFIMPL(&default__inplace_mod__with__mod),
	/* .tp_inplace_shl = */ DEFIMPL(&default__inplace_shl__with__shl),
	/* .tp_inplace_shr = */ DEFIMPL(&default__inplace_shr__with__shr),
	/* .tp_inplace_and = */ DEFIMPL(&default__inplace_and__with__and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__inplace_or__with__or),
	/* .tp_inplace_xor = */ DEFIMPL(&default__inplace_xor__with__xor),
	/* .tp_inplace_pow = */ DEFIMPL(&default__inplace_pow__with__pow),
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_trycompare_eq_cb(void *arg, DeeObject *elem) {
	int result = DeeObject_TryCompareEq(elem, (DeeObject *)arg);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	if (result == 0)
		return SEQSOME_FOREACH_YES;
	return SEQSOME_FOREACH_NO;
err:
	return SEQSOME_FOREACH_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_trycompare_eq(SeqEachOperator *self, DeeObject *other) {
	Dee_ssize_t result = seo_foreach(self, &ss_trycompare_eq_cb, other);
	ASSERT(result == SEQSOME_FOREACH_YES ||
	       result == SEQSOME_FOREACH_ERR ||
	       result == SEQSOME_FOREACH_NO);
	switch (result) {
	case SEQSOME_FOREACH_YES:
		return Dee_COMPARE_EQ; /* Some item *does* compare equal */
	case SEQSOME_FOREACH_NO:
		return Dee_COMPARE_NE; /* No items compare equal */
	case SEQSOME_FOREACH_ERR:
		return Dee_COMPARE_ERR;
	default: __builtin_unreachable();
	}
}

PRIVATE struct type_cmp sso_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_trycompare_eq,
	/* .tp_compare       = */ DEFIMPL(&default__compare__with__eq__and__lo),
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_trycompare_eq,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_ge,
};

STATIC_ASSERT(offsetof(SeqEachBase, se_seq) == offsetof(ProxyObject, po_obj));
#define ssw_size_fast generic_proxy__size_fast

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_bounditem(SeqEachOperator *self, DeeObject *index) {
	return seqsome_map_fe2bound(seo_foreach(self, &ss_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasitem(SeqEachOperator *self, DeeObject *index) {
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sso_bounditem_index(SeqEachOperator *self, size_t index) {
	return seqsome_map_fe2bound(seo_foreach(self, &ss_bounditem_index_foreach_cb,
	                                        (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sso_hasitem_index(SeqEachOperator *self, size_t index) {
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasitem_index_foreach_cb,
	                                      (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_bounditem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_fe2bound(seo_foreach(self, &ss_bounditem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasitem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_bounditem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_fe2bound(seo_foreach(self, &ss_bounditem_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasitem_string_len_hash_foreach_cb, &data));
}

PRIVATE struct type_seq sso_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_getitem,
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ssw_getrange,
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&seo_foreach, /* Needed for recursion! */
	/* .tp_foreach_pair               = */ &default__foreach_pair__with__foreach,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_hasitem,
	/* .tp_size                       = */ DEFIMPL(&default__size__with__sizeob),
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ssw_size_fast,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&sso_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&sso_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__getrange_index__with__getrange),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__getrange_index_n__with__getrange),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sso_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sso_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&sso_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&sso_hasitem_string_len_hash,
};

#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
#define ssw_getattr seqsomew_getattr
INTERN WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL
seqsomew_getattr(DeeObject *__restrict self,
                 DeeStringObject *__restrict attr) {
	DREF SeqEachGetAttr *result;
	result = DeeObject_MALLOC(SeqEachGetAttr);
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = attr;
	Dee_Incref(self);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqSomeGetAttr_Type);
done:
	return result;
}

#define ssw_callattr                    DeeSeqSome_CallAttr
#define ssw_callattr_kw                 DeeSeqSome_CallAttrKw
#define ssw_callattr_string_hash        DeeSeqSome_CallAttrStringHash
#define ssw_callattr_string_hash_kw     DeeSeqSome_CallAttrStringHashKw
#define ssw_callattr_string_len_hash    DeeSeqSome_CallAttrStringLenHash
#define ssw_callattr_string_len_hash_kw DeeSeqSome_CallAttrStringLenHashKw
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
DEFINE_SSW_BINARY(ssw_getattr, OPERATOR_GETATTR)
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_boundattr(SeqEachOperator *self, DeeObject *attr) {
	return seqsome_map_fe2bound(seo_foreach(self, &ss_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasattr(SeqEachOperator *self, DeeObject *attr) {
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_boundattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_fe2bound(seo_foreach(self, &ss_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_boundattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_fe2bound(seo_foreach(self, &ss_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_fe2has(seo_foreach(self, &ss_hasattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
ssw_iterattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
             struct Dee_attriter *iterbuf, size_t bufsize,
             struct Dee_attrhint const *__restrict hint) {
	return ss_iterattr_impl(Dee_AsObject(self), iterbuf, bufsize, hint);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
ssw_findattr(DeeTypeObject *UNUSED(tp_self), SeqEachBase *self,
             struct Dee_attrspec const *__restrict specs,
             struct Dee_attrdesc *__restrict result) {
	return ss_findattr_impl(Dee_AsObject(self), specs, result);
}

PRIVATE struct type_attr sso_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&ssw_getattr,
	/* .tp_delattr                       = */ NULL,
	/* .tp_setattr                       = */ NULL,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&ssw_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&ssw_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_boundattr,
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&ssw_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&ssw_callattr_kw,
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ NULL,
	/* .tp_setattr_string_hash           = */ NULL,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sso_hasattr_string_hash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sso_boundattr_string_hash,
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&ssw_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&ssw_callattr_string_hash_kw,
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ NULL,
	/* .tp_setattr_string_len_hash       = */ NULL,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&sso_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&sso_boundattr_string_len_hash,
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&ssw_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&ssw_callattr_string_len_hash_kw,
#else /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
#endif /* !CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_findattr_info_string_len_hash = */ NULL,
};

PRIVATE struct type_callable ssw_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&ssw_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

INTERN DeeTypeObject SeqSomeOperator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSomeOperator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,op:?X2?Dstring?Dint,args=!T0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(
			/* min_tp_instance_size: */ offsetof(SeqEachOperator, so_opargv),
			/* max_tp_instance_size: */ sizeof(SeqEachOperator),
			/* tp_ctor:              */ &seo_ctor,
			/* tp_copy_ctor:         */ &seo_copy,
			/* tp_deep_ctor:         */ &seo_deep,
			/* tp_any_ctor:          */ &seo_init,
			/* tp_any_ctor_kw:       */ NULL,
			/* tp_serialize:         */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seo_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_moveassign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&sso_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sso_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&seo_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &ssw_math,
	/* .tp_cmp           = */ &sso_cmp,
	/* .tp_seq           = */ &sso_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ &sso_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sso_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&ssw_call,
	/* .tp_callable      = */ &ssw_callable,
};



/* Iterator support */
PRIVATE WUNUSED NONNULL((1)) int DCALL
seoi_ctor(SeqEachIterator *__restrict self) {
	self->ei_each = (DREF SeqEachBase *)DeeObject_NewDefault(&SeqEachOperator_Type);
	if unlikely(!self->ei_each)
		goto err;
	self->ei_iter = DeeObject_Iter(self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err_each;
	return 0;
err_each:
	Dee_Decref(self->ei_each);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seoi_init(SeqEachIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_SeqEachOperatorIterator", &self->ei_each);
	if (DeeObject_AssertTypeExact(self->ei_each, &SeqEachOperator_Type))
		goto err;
	self->ei_iter = DeeObject_Iter(self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err;
	Dee_Incref(self->ei_each);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(SeqEachIterator, ei_iter) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(SeqEachIterator, ei_each) == offsetof(ProxyObject2, po_obj2));
#define sewi_copy  generic_proxy2__copy_recursive1_alias2 /* Copy "ei_iter", alias "ei_each" */
#define sewi_deep  generic_proxy2__deepcopy
#define sewi_fini  generic_proxy2__fini
#define sewi_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(SeqEachIterator, ei_iter) == offsetof(ProxyObject, po_obj));
#define sewi_bool generic_proxy__bool

STATIC_ASSERT(offsetof(SeqEachIterator, ei_iter) == offsetof(ProxyObject, po_obj));
#define sewi_compare_eq    generic_proxy__compare_eq_recursive
#define sewi_compare       generic_proxy__compare_recursive
#define sewi_trycompare_eq generic_proxy__trycompare_eq_recursive

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachBase *DCALL
sewi_nii_getseq(SeqEachIterator *__restrict self) {
	return_reference_(self->ei_each);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sewi_nii_getindex(SeqEachIterator *__restrict self) {
	return DeeIterator_GetIndex(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_setindex(SeqEachIterator *__restrict self, size_t index) {
	return DeeIterator_SetIndex(self->ei_iter, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_rewind(SeqEachIterator *__restrict self) {
	return DeeIterator_Rewind(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_revert(SeqEachIterator *__restrict self, size_t skip) {
	return DeeIterator_Revert(self->ei_iter, skip);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_advance(SeqEachIterator *__restrict self, size_t skip) {
	return DeeIterator_Advance(self->ei_iter, skip);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_prev(SeqEachIterator *__restrict self) {
	return DeeIterator_Prev(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_next(SeqEachIterator *__restrict self) {
	return DeeIterator_Next(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_hasprev(SeqEachIterator *__restrict self) {
	return DeeIterator_HasPrev(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sewi_nii_peek(SeqEachIterator *__restrict self) {
	return DeeIterator_Peek(self->ei_iter);
}

PRIVATE struct type_nii tpconst sewi_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&sewi_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&sewi_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&sewi_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&sewi_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)&sewi_nii_revert,
			/* .nii_advance  = */ (Dee_funptr_t)&sewi_nii_advance,
			/* .nii_prev     = */ (Dee_funptr_t)&sewi_nii_prev,
			/* .nii_next     = */ (Dee_funptr_t)&sewi_nii_next,
			/* .nii_hasprev  = */ (Dee_funptr_t)&sewi_nii_hasprev,
			/* .nii_peek     = */ (Dee_funptr_t)&sewi_nii_peek
		}
	}
};

PRIVATE struct type_cmp sewi_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sewi_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&sewi_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&sewi_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &sewi_nii,
};

PRIVATE struct type_member tpconst seoi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachOperator"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seoi_next(SeqEachIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->ei_iter);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit((SeqEachOperator *)self->ei_each, result);
	return result;
}

INTERN DeeTypeObject SeqEachOperatorIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEachOperatorIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqEachOperator)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqEachIterator,
			/* tp_ctor:        */ &seoi_ctor,
			/* tp_copy_ctor:   */ &sewi_copy,
			/* tp_deep_ctor:   */ &sewi_deep,
			/* tp_any_ctor:    */ &seoi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sewi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sewi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sewi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &sewi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seoi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seoi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};





#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqEach_CallAttr(DeeObject *self, DeeObject *attr,
                    size_t argc, DeeObject *const *argv) {
	DREF SeqEachCallAttr *result;
	result = (DREF SeqEachCallAttr *)DeeObject_Mallocc(offsetof(SeqEachCallAttr, sg_argv),
	                                                   argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = (DREF DeeStringObject *)attr;
	result->sg_argc = argc;
	Dee_Movrefv(result->sg_argv, argv, argc);
	Dee_Incref(self);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqEachCallAttr_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqEach_CallAttrKw(DeeObject *self, DeeObject *attr, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	DREF SeqEachCallAttrKw *result;
	if (!kw)
		return DeeSeqEach_CallAttr(self, attr, argc, argv);
	result = (DREF SeqEachCallAttrKw *)DeeObject_Mallocc(offsetof(SeqEachCallAttrKw, sg_argv),
	                                                     argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = (DREF DeeStringObject *)attr;
	result->sg_kw   = kw;
	result->sg_argc = argc;
	Dee_Movrefv(result->sg_argv, argv, argc);
	Dee_Incref(self);
	Dee_Incref(attr);
	Dee_Incref(kw);
	DeeObject_Init(result, &SeqEachCallAttrKw_Type);
done:
	return Dee_AsObject(result);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringHash(DeeObject *self, char const *__restrict attr,
                              Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewWithHash(attr, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttr(self, (DeeObject *)attr_ob, argc, argv);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringLenHash(DeeObject *self, char const *__restrict attr, size_t attrlen,
                                 Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttr(self, (DeeObject *)attr_ob, argc, argv);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringHashKw(DeeObject *self, char const *__restrict attr, Dee_hash_t hash,
                                size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewWithHash(attr, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttrKw(self, (DeeObject *)attr_ob, argc, argv, kw);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringLenHashKw(DeeObject *self, char const *__restrict attr, size_t attrlen,
                                   Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttrKw(self, (DeeObject *)attr_ob, argc, argv, kw);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */

#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqSome_CallAttr(DeeObject *self, DeeObject *attr,
                    size_t argc, DeeObject *const *argv) {
	DREF SeqEachCallAttr *result;
	result = (DREF SeqEachCallAttr *)DeeObject_Mallocc(offsetof(SeqEachCallAttr, sg_argv),
	                                                   argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = (DREF DeeStringObject *)attr;
	result->sg_argc = argc;
	Dee_Movrefv(result->sg_argv, argv, argc);
	Dee_Incref(self);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqSomeCallAttr_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqSome_CallAttrKw(DeeObject *self, DeeObject *attr, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	DREF SeqEachCallAttrKw *result;
	if (!kw)
		return DeeSeqSome_CallAttr(self, attr, argc, argv);
	result = (DREF SeqEachCallAttrKw *)DeeObject_Mallocc(offsetof(SeqEachCallAttrKw, sg_argv),
	                                                     argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = (DREF DeeStringObject *)attr;
	result->sg_kw   = kw;
	result->sg_argc = argc;
	Dee_Movrefv(result->sg_argv, argv, argc);
	Dee_Incref(self);
	Dee_Incref(attr);
	Dee_Incref(kw);
	DeeObject_Init(result, &SeqSomeCallAttrKw_Type);
done:
	return Dee_AsObject(result);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqSome_CallAttrStringHash(DeeObject *self, char const *__restrict attr,
                              Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewWithHash(attr, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqSome_CallAttr(self, (DeeObject *)attr_ob, argc, argv);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqSome_CallAttrStringLenHash(DeeObject *self, char const *__restrict attr, size_t attrlen,
                                 Dee_hash_t hash, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqSome_CallAttr(self, (DeeObject *)attr_ob, argc, argv);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqSome_CallAttrStringHashKw(DeeObject *self, char const *__restrict attr, Dee_hash_t hash,
                                size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewWithHash(attr, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqSome_CallAttrKw(self, (DeeObject *)attr_ob, argc, argv, kw);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeqSome_CallAttrStringLenHashKw(DeeObject *self, char const *__restrict attr, size_t attrlen,
                                   Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqSome_CallAttrKw(self, (DeeObject *)attr_ob, argc, argv, kw);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}
#endif /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */

DECL_END

#ifndef __INTELLISENSE__
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define DEFINE_SeqEachGetAttr
#include "each-fastpass.c.inl"
#define DEFINE_SeqEachCallAttr
#include "each-fastpass.c.inl"
#define DEFINE_SeqEachCallAttrKw
#include "each-fastpass.c.inl"
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_EACH_C */
