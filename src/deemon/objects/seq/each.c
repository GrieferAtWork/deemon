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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_EACH_C
#define GUARD_DEEMON_OBJECTS_SEQ_EACH_C 1

#include "each.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../seq_functions.h"
#include "default-api.h"

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
	self->se_seq = Dee_EmptySeq;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

#define se_copy  generic_proxy_copy_alias
#define se_deep  generic_proxy_deepcopy
#define se_init  generic_proxy_init
#define se_fini  generic_proxy_fini
#define se_visit generic_proxy_visit

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

#define SEQEACH_FOREACH_ERROR (-1)
#define SEQEACH_FOREACH_YES   0
#define SEQEACH_FOREACH_NO    (-2)

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqeach_map_to_bound(int bound_result) {
	if (bound_result > 0)
		return SEQEACH_FOREACH_YES; /* Item is bound */
	if (bound_result == -2)
		return SEQEACH_FOREACH_NO; /* Item doesn't exist */
	if unlikely(bound_result == -1)
		goto err;
	return 1; /* Item exists, but isn't bound */
err:
	return SEQEACH_FOREACH_ERROR;
}

LOCAL ATTR_CONST int DCALL
seqeach_map_from_bound(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQEACH_FOREACH_ERROR:
		return -1;
	case SEQEACH_FOREACH_YES:
		return 1;
	case SEQEACH_FOREACH_NO:
		return -2;
	default:
		return 0;
	}
}

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqeach_map_to_has(int bound_result) {
	if (bound_result > 0)
		return SEQEACH_FOREACH_YES; /* Item exists */
	if (bound_result == 0)
		return SEQEACH_FOREACH_NO; /* Item doesn't exists */
	return SEQEACH_FOREACH_ERROR;
}

LOCAL ATTR_CONST int DCALL
seqeach_map_from_has(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQEACH_FOREACH_ERROR:
		return -1;
	case SEQEACH_FOREACH_YES:
		return 1;
	case SEQEACH_FOREACH_NO:
		return 0;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_bounditem_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_to_bound(DeeObject_BoundItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_bounditem(SeqEachBase *self, DeeObject *index) {
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_to_has(DeeObject_HasItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasitem(SeqEachBase *self, DeeObject *index) {
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_bounditem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_to_bound(DeeObject_BoundItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_bounditem_index(SeqEachBase *self, size_t index) {
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_bounditem_index_foreach_cb,
	                                                     (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqeach_map_to_has(DeeObject_HasItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_hasitem_index(SeqEachBase *self, size_t index) {
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasitem_index_foreach_cb,
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
	return seqeach_map_to_bound(DeeObject_BoundItemStringHash(item, data->ssbishfd_key, data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_bounditem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_bounditem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_string_hash_foreach_cb(void *arg, DeeObject *item) {
	struct se_bounditem_string_hash_foreach_data *data;
	data = (struct se_bounditem_string_hash_foreach_data *)arg;
	return seqeach_map_to_has(DeeObject_HasItemStringHash(item, data->ssbishfd_key, data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasitem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasitem_string_hash_foreach_cb, &data));
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
	return seqeach_map_to_bound(DeeObject_BoundItemStringLenHash(item, data->ssbislhfd_key,
	                                                             data->ssbislhfd_keylen,
	                                                             data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_bounditem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_bounditem_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasitem_string_len_hash_foreach_cb(void *arg, DeeObject *item) {
	struct se_bounditem_string_len_hash_foreach_data *data;
	data = (struct se_bounditem_string_len_hash_foreach_data *)arg;
	return seqeach_map_to_has(DeeObject_HasItemStringLenHash(item, data->ssbislhfd_key,
	                                                         data->ssbislhfd_keylen,
	                                                         data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasitem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasitem_string_len_hash_foreach_cb, &data));
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
	return seqeach_map_to_bound(DeeObject_BoundAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_boundattr(SeqEachBase *self, DeeObject *attr) {
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasattr_foreach_cb(void *arg, DeeObject *attr) {
	return seqeach_map_to_has(DeeObject_HasAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasattr(SeqEachBase *self, DeeObject *attr) {
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasattr_foreach_cb, attr));
}

struct se_boundattr_string_hash_foreach_data {
	char const *ssbashfd_attr;
	Dee_hash_t  ssbashfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_boundattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_hash_foreach_data *data;
	data = (struct se_boundattr_string_hash_foreach_data *)arg;
	return seqeach_map_to_bound(DeeObject_BoundAttrStringHash(attr, data->ssbashfd_attr, data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_boundattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_hash_foreach_data *data;
	data = (struct se_boundattr_string_hash_foreach_data *)arg;
	return seqeach_map_to_has(DeeObject_HasAttrStringHash(attr, data->ssbashfd_attr, data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasattr_string_hash_foreach_cb, &data));
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
	return seqeach_map_to_bound(DeeObject_BoundAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                             data->ssbaslhfd_attrlen,
	                                                             data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_boundattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &se_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
se_hasattr_string_len_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct se_boundattr_string_len_hash_foreach_data *data;
	data = (struct se_boundattr_string_len_hash_foreach_data *)arg;
	return seqeach_map_to_has(DeeObject_HasAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                         data->ssbaslhfd_attrlen,
	                                                         data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_hasattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &se_hasattr_string_len_hash_foreach_cb, &data));
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
	return DeeSeq_InvokeAll(self->se_seq);
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
	size = DeeSeq_OperatorSize(core_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		elem = DeeSeq_OperatorGetItemIndex(core_seq, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			goto err;
		}
		if ((*op)(&elem, other))
			goto err_elem;
		if (DeeObject_SetItemIndex(core_seq, i, elem))
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
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
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
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_pow
};

PRIVATE struct type_cmp se_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_ge
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
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&se_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&se_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&se_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&se_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&se_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&se_setrange_index,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&se_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&se_setrange_index_n,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&se_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&se_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
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
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&se_leave
};


PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
se_enumattr_impl(DeeObject *seq, denum_t proc, void *arg) {
	(void)seq;
	(void)proc;
	(void)arg;
	/* TODO: Enumerate attributes available via the common
	 *       base class of all elements of `seq'. */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
se_enumattr(DeeTypeObject *UNUSED(tp_self),
            SeqEachBase *self, denum_t proc, void *arg) {
	return se_enumattr_impl(self->se_seq, proc, arg);
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
	/* .tp_enumattr                      = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&se_enumattr,
	/* .tp_findattr                      = */ NULL,
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


INTERN DeeTypeObject SeqEach_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEach",
	/* .tp_doc      = */ DOC("(seq:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&se_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&se_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&se_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&se_init,
				TYPE_FIXED_ALLOCATOR(SeqEachBase)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&se_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&se_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&se_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&se_printrepr,
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&se_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&se_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &se_math,
	/* .tp_cmp           = */ &se_cmp,
	/* .tp_seq           = */ &se_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_iter_next,
	/* .tp_iterator      = */ NULL,
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
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&se_call_kw,
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
	return DeeSeq_InvokeAny(self->se_seq);
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
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
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
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_trycompare_eq(SeqEachBase *self, DeeObject *other) {
	/* `seq.some == other'  <=>  `other in seq' */
	int contains = DeeSeq_OperatorContainsAsBool(self->se_seq, other);
	if unlikely(contains < 0)
		goto err;
	return contains ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

#define SEQSOME_FOREACH_ERROR (-1)
#define SEQSOME_FOREACH_YES   (-2)
#define SEQSOME_FOREACH_NO    0

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqsome_map_to_bound(int bound_result) {
	if (bound_result > 0)
		return SEQSOME_FOREACH_YES; /* Item is bound */
	if (bound_result == -2)
		return SEQSOME_FOREACH_NO; /* Item doesn't exist */
	if unlikely(bound_result == -1)
		goto err;
	return 1; /* Item exists, but isn't bound */
err:
	return SEQSOME_FOREACH_ERROR;
}

LOCAL ATTR_CONST int DCALL
seqsome_map_from_bound(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQSOME_FOREACH_ERROR:
		return -1;
	case SEQSOME_FOREACH_YES:
		return 1;
	case SEQSOME_FOREACH_NO:
		return -2;
	default:
		return 0;
	}
}

LOCAL ATTR_CONST Dee_ssize_t DCALL
seqsome_map_to_has(int bound_result) {
	if (bound_result > 0)
		return SEQSOME_FOREACH_YES; /* Item exists */
	if (bound_result == 0)
		return SEQSOME_FOREACH_NO; /* Item doesn't exists */
	return SEQSOME_FOREACH_ERROR;
}

LOCAL ATTR_CONST int DCALL
seqsome_map_from_has(Dee_ssize_t foreach_result) {
	switch (foreach_result) {
	case SEQSOME_FOREACH_ERROR:
		return -1;
	case SEQSOME_FOREACH_YES:
		return 1;
	case SEQSOME_FOREACH_NO:
		return 0;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_bounditem_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_to_bound(DeeObject_BoundItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_bounditem(SeqEachBase *self, DeeObject *index) {
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_to_has(DeeObject_HasItem(item, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasitem(SeqEachBase *self, DeeObject *index) {
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_bounditem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_to_bound(DeeObject_BoundItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ss_bounditem_index(SeqEachBase *self, size_t index) {
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_bounditem_index_foreach_cb,
	                                                     (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_index_foreach_cb(void *arg, DeeObject *item) {
	return seqsome_map_to_has(DeeObject_HasItemIndex(item, (size_t)(uintptr_t)arg));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ss_hasitem_index(SeqEachBase *self, size_t index) {
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasitem_index_foreach_cb,
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
	return seqsome_map_to_bound(DeeObject_BoundItemStringHash(item, data->ssbishfd_key, data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_bounditem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_bounditem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_string_hash_foreach_cb(void *arg, DeeObject *item) {
	struct ss_bounditem_string_hash_foreach_data *data;
	data = (struct ss_bounditem_string_hash_foreach_data *)arg;
	return seqsome_map_to_has(DeeObject_HasItemStringHash(item, data->ssbishfd_key, data->ssbishfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasitem_string_hash(SeqEachBase *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasitem_string_hash_foreach_cb, &data));
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
	return seqsome_map_to_bound(DeeObject_BoundItemStringLenHash(item, data->ssbislhfd_key,
	                                                             data->ssbislhfd_keylen,
	                                                             data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_bounditem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_bounditem_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasitem_string_len_hash_foreach_cb(void *arg, DeeObject *item) {
	struct ss_bounditem_string_len_hash_foreach_data *data;
	data = (struct ss_bounditem_string_len_hash_foreach_data *)arg;
	return seqsome_map_to_has(DeeObject_HasItemStringLenHash(item, data->ssbislhfd_key,
	                                                         data->ssbislhfd_keylen,
	                                                         data->ssbislhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasitem_string_len_hash(SeqEachBase *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasitem_string_len_hash_foreach_cb, &data));
}


PRIVATE struct type_cmp ss_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_trycompare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_trycompare_eq,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_ge
};

PRIVATE struct type_seq ss_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_size,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ss_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ss_getrange,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ss_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ss_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&ss_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&ss_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&ss_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&ss_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
ss_enumattr_impl(DeeObject *seq, denum_t proc, void *arg) {
	(void)seq;
	(void)proc;
	(void)arg;
	/* TODO: Enumerate attributes available
	 *       by at least one element of `seq'. */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
ss_enumattr(DeeTypeObject *UNUSED(tp_self),
            SeqEachBase *self, denum_t proc, void *arg) {
	return ss_enumattr_impl(self->se_seq, proc, arg);
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
	return seqsome_map_to_bound(DeeObject_BoundAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_boundattr(SeqEachBase *self, DeeObject *attr) {
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasattr_foreach_cb(void *arg, DeeObject *attr) {
	return seqsome_map_to_has(DeeObject_HasAttr(attr, (DeeObject *)arg));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasattr(SeqEachBase *self, DeeObject *attr) {
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasattr_foreach_cb, attr));
}

struct ss_boundattr_string_hash_foreach_data {
	char const *ssbashfd_attr;
	Dee_hash_t  ssbashfd_hash;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_boundattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_hash_foreach_data *data;
	data = (struct ss_boundattr_string_hash_foreach_data *)arg;
	return seqsome_map_to_bound(DeeObject_BoundAttrStringHash(attr, data->ssbashfd_attr, data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_boundattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasattr_string_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_hash_foreach_data *data;
	data = (struct ss_boundattr_string_hash_foreach_data *)arg;
	return seqsome_map_to_has(DeeObject_HasAttrStringHash(attr, data->ssbashfd_attr, data->ssbashfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasattr_string_hash(SeqEachBase *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasattr_string_hash_foreach_cb, &data));
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
	return seqsome_map_to_bound(DeeObject_BoundAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                             data->ssbaslhfd_attrlen,
	                                                             data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_boundattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_from_bound(DeeSeq_OperatorForeach(self->se_seq, &ss_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_hasattr_string_len_hash_foreach_cb(void *arg, DeeObject *attr) {
	struct ss_boundattr_string_len_hash_foreach_data *data;
	data = (struct ss_boundattr_string_len_hash_foreach_data *)arg;
	return seqsome_map_to_has(DeeObject_HasAttrStringLenHash(attr, data->ssbaslhfd_attr,
	                                                         data->ssbaslhfd_attrlen,
	                                                         data->ssbaslhfd_hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_hasattr_string_len_hash(SeqEachBase *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_from_has(DeeSeq_OperatorForeach(self->se_seq, &ss_hasattr_string_len_hash_foreach_cb, &data));
}



PRIVATE struct type_attr ss_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&ss_getattr,
	/* .tp_delattr                       = */ NULL,
	/* .tp_setattr                       = */ NULL,
	/* .tp_enumattr                      = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&ss_enumattr,
	/* .tp_findattr                      = */ NULL,
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

INTERN DeeTypeObject SeqSome_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSome",
	/* .tp_doc      = */ DOC("(seq:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ss_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&ss_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ss_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&ss_init,
				TYPE_FIXED_ALLOCATOR(SeqEachBase)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ss_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&ss_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ss_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ss_printrepr,
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&ss_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ss_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &ss_math,
	/* .tp_cmp           = */ &ss_cmp,
	/* .tp_seq           = */ &ss_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_iter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &ss_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ss_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&ss_call_kw,
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
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Some(DeeObject *__restrict self) {
#if 1 /* TODO */
	DREF SeqEachBase *result;
	result = DeeObject_MALLOC(SeqEachBase);
	if unlikely(!result)
		goto done;
	result->se_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqSome_Type);
done:
	return (DREF DeeObject *)result;
#else
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
#endif
}





/* SeqEach<WRAPPER> -- Operator */
PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_ctor(SeqEachOperator *__restrict self) {
	self->se_seq    = Dee_EmptySeq;
	self->so_opname = OPERATOR_BOOL;
	self->so_opargc = 0;
	Dee_Incref(Dee_EmptySeq);
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
	DeeObject *name;
	DeeObject *args = Dee_EmptyTuple;
	if (DeeArg_Unpack(argc, argv, "oo|o:_SeqEachOperator",
	                  &self->se_seq, &name, &args))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if unlikely(DeeTuple_SIZE(args) > COMPILER_LENOF(self->so_opargv)) {
		DeeError_Throwf(&DeeError_UnpackError,
		                "Too many operator arguments (%" PRFuSIZ " > %" PRFuSIZ ")",
		                (size_t)DeeTuple_SIZE(args),
		                (size_t)COMPILER_LENOF(self->so_opargv));
		goto err;
	}
	if (DeeString_Check(name)) {
		struct opinfo const *info;
		info = DeeTypeType_GetOperatorByName(&DeeType_Type, DeeString_STR(name), (size_t)-1);
		if unlikely(info == NULL) {
			/* TODO: In this case, remember the used "name" string,
			 * and query the operator on a per-element basis */
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown operator %q",
			                DeeString_STR(name));
			goto err;
		}
		self->so_opname = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(name, &self->so_opname))
			goto err;
	}
	self->so_opargc = (uint16_t)DeeTuple_SIZE(args);
	Dee_Movrefv(self->so_opargv, DeeTuple_ELEM(args), self->so_opargc);
	Dee_Incref(self->se_seq);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_HAVE_SEQEACH_OPERATOR_REPR
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
#endif /* CONFIG_HAVE_SEQEACH_OPERATOR_REPR */

PRIVATE NONNULL((1)) void DCALL
seo_fini(SeqEachOperator *__restrict self) {
	Dee_Decref(self->se_seq);
	Dee_Decrefv(self->so_opargv, self->so_opargc);
}

PRIVATE NONNULL((1, 2)) void DCALL
seo_visit(SeqEachOperator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->se_seq);
	Dee_Visitv(self->so_opargv, self->so_opargc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sew_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return (DREF DeeObject *)seqeach_makeop1(self, OPERATOR_CALL, tuple);
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
#ifndef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
DEFINE_SEW_BINARY(sew_eq, OPERATOR_EQ)
DEFINE_SEW_BINARY(sew_ne, OPERATOR_NE)
DEFINE_SEW_BINARY(sew_lo, OPERATOR_LO)
DEFINE_SEW_BINARY(sew_le, OPERATOR_LE)
DEFINE_SEW_BINARY(sew_gr, OPERATOR_GR)
DEFINE_SEW_BINARY(sew_ge, OPERATOR_GE)
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
#define sew_contains DeeSeq_DefaultContainsWithForeachDefault
#define sew_size     generic_proxy_size
#define sew_sizeob   generic_proxy_sizeob
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
DEFINE_SEW_BINARY(sew_contains, OPERATOR_CONTAINS)
DEFINE_SEW_BINARY(sew_sizeob, OPERATOR_GETITEM)
DEFINE_SEW_BINARY(sew_getitem, OPERATOR_GETITEM)
DEFINE_SEW_TRINARY(sew_getrange, OPERATOR_GETRANGE)
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */


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
	return (int)DeeSeq_OperatorForeach(self->se_seq, &seo_inplace_foreach_cb, &data);
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
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
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
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_pow
};

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
#define sew_cmp DeeSeq_DefaultCmpWithForeachDefault
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
PRIVATE struct type_cmp sew_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_ge
};
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */


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
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&sew_leave
};


PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
sew_enumattr(DeeTypeObject *UNUSED(tp_self),
             SeqEachBase *self, denum_t proc, void *arg) {
	return se_enumattr_impl((DeeObject *)self, proc, arg);
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

struct seo_enumerate_data {
	SeqEachOperator *seoed_me;   /* [1..1] The related seq-each operator */
	Dee_enumerate_t  seoed_proc; /* [1..1] User-defined callback */
	void            *seoed_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seo_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct seo_enumerate_data *data;
	data = (struct seo_enumerate_data *)arg;
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
seo_enumerate(SeqEachOperator *__restrict self, Dee_enumerate_t proc, void *arg) {
	struct seo_enumerate_data data;
	data.seoed_me   = self;
	data.seoed_proc = proc;
	data.seoed_arg  = arg;
	return DeeObject_Enumerate(self->se_seq, &seo_enumerate_cb, &data);
}

struct seo_enumerate_index_data {
	SeqEachOperator      *seoeid_me;   /* [1..1] The related seq-each operator */
	Dee_enumerate_index_t seoeid_proc; /* [1..1] User-defined callback */
	void                 *seoeid_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seo_enumerate_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct seo_enumerate_index_data *data;
	data = (struct seo_enumerate_index_data *)arg;
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
seo_enumerate_index(SeqEachOperator *__restrict self, Dee_enumerate_index_t proc,
                    void *arg, size_t start, size_t end) {
	struct seo_enumerate_index_data data;
	data.seoeid_me   = self;
	data.seoeid_proc = proc;
	data.seoeid_arg  = arg;
	return DeeObject_EnumerateIndex(self->se_seq, &seo_enumerate_index_cb,
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
		return -2;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_bool(SeqEachOperator *__restrict self) {
	Dee_ssize_t result = seo_foreach(self, &se_foreach_bool_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		return 0;
	if (result == 0)
		return 1;
	return -1;
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
seo_bounditem(SeqEachOperator *self, DeeObject *index) {
	return seqeach_map_from_bound(seo_foreach(self, &se_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasitem(SeqEachOperator *self, DeeObject *index) {
	return seqeach_map_from_has(seo_foreach(self, &se_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_bounditem_index(SeqEachOperator *self, size_t index) {
	return seqeach_map_from_bound(seo_foreach(self, &se_bounditem_index_foreach_cb,
	                                          (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_hasitem_index(SeqEachOperator *self, size_t index) {
	return seqeach_map_from_has(seo_foreach(self, &se_hasitem_index_foreach_cb,
	                                        (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_bounditem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_from_bound(seo_foreach(self, &se_bounditem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_from_has(seo_foreach(self, &se_hasitem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_bounditem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_from_bound(seo_foreach(self, &se_bounditem_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_from_has(seo_foreach(self, &se_hasitem_string_len_hash_foreach_cb, &data));
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
	return seqeach_map_from_bound(seo_foreach(self, &se_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasattr(SeqEachOperator *self, DeeObject *attr) {
	return seqeach_map_from_has(seo_foreach(self, &se_hasattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_boundattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_from_bound(seo_foreach(self, &se_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_from_has(seo_foreach(self, &se_hasattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_boundattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_from_bound(seo_foreach(self, &se_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_hasattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_from_has(seo_foreach(self, &se_hasattr_string_len_hash_foreach_cb, &data));
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
	/* .tp_enumattr                      = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&sew_enumattr,
	/* .tp_findattr                      = */ NULL,
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
seo_iter(SeqEachOperator *__restrict self) {
	DREF SeqEachIterator *result;
	result = DeeObject_MALLOC(SeqEachIterator);
	if unlikely(!result)
		goto done;
	result->ei_each = (DREF SeqEachBase *)self;
	result->ei_iter = DeeObject_Iter(((DREF SeqEachBase *)self)->se_seq);
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
#define sew_size_fast generic_proxy_size_fast

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
#define sew_iterkeys generic_proxy_iterkeys

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_getitem(SeqEachOperator *self, DeeObject *index) {
	DREF DeeObject *result = DeeObject_GetItem(self->se_seq, index);
	if likely(result)
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
seo_getrange(SeqEachOperator *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *base = DeeObject_GetRange(self->se_seq, start, end);
	if unlikely(!base)
		goto err;
	return seo_wraprange(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seo_getitem_index(SeqEachOperator *self, size_t index) {
	DREF DeeObject *result = DeeObject_GetItemIndex(self->se_seq, index);
	if likely(result)
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seo_getrange_index(SeqEachOperator *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *base = DeeObject_GetRangeIndex(self->se_seq, start, end);
	if unlikely(!base)
		goto err;
	return seo_wraprange(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seo_getrange_index_n(SeqEachOperator *self, Dee_ssize_t start) {
	DREF DeeObject *base = DeeObject_GetRangeIndexN(self->se_seq, start);
	if unlikely(!base)
		goto err;
	return seo_wraprange(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_trygetitem(SeqEachOperator *self, DeeObject *index) {
	DREF DeeObject *result = DeeObject_TryGetItem(self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seo_trygetitem_index(SeqEachOperator *self, size_t index) {
	DREF DeeObject *result = DeeObject_TryGetItemIndex(self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_trygetitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = DeeObject_TryGetItemStringHash(self->se_seq, key, hash);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_getitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = DeeObject_GetItemStringHash(self->se_seq, key, hash);
	if likely(result)
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_trygetitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = DeeObject_TryGetItemStringLenHash(self->se_seq, key, keylen, hash);
	if likely(ITER_ISOK(result))
		result = seo_transform_inherit(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_getitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = DeeObject_GetItemStringLenHash(self->se_seq, key, keylen, hash);
	if likely(result)
		result = seo_transform_inherit(self, result);
	return result;
}

#endif /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */


PRIVATE struct type_seq seo_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seo_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_contains,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seo_getitem,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_getitem,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&seo_setitem,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&seo_getrange,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&sew_getrange,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&seo_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&seo_setrange,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&seo_foreach,
	/* .tp_foreach_pair               = */ NULL, /* &DeeObject_DefaultForeachPairWithForeachs */
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&seo_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&seo_enumerate_index,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_iterkeys                   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iterkeys,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_iterkeys                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_hasitem,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&sew_size,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_size                       = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&sew_size_fast,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&seo_getitem_index,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_getitem_index              = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&seo_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&seo_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&seo_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&seo_hasitem_index,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&seo_getrange_index,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_getrange_index             = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&seo_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&seo_setrange_index,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&seo_getrange_index_n,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_getrange_index_n           = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&seo_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&seo_setrange_index_n,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seo_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&seo_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_getitem_string_hash,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&seo_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&seo_hasitem_string_hash,
#ifdef CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_getitem_string_len_hash,
#else /* CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_IS_SEQLIKE */
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&seo_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&seo_hasitem_string_len_hash,
};

#define seo_members se_members  /* TODO: Access to operator name & arguments */
#define sso_members seo_members

PRIVATE struct type_member tpconst seo_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqEachOperatorIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqEachOperator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEachOperator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,op:?X2?Dstring?Dint,args=!T0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&seo_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&seo_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&seo_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&seo_init,
				TYPE_SIZED_ALLOCATOR_R(offsetof(SeqEachOperator, so_opargv), sizeof(SeqEachOperator))
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seo_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&seo_bool,
		/* .tp_print     = */ NULL,
#ifdef CONFIG_HAVE_SEQEACH_OPERATOR_REPR
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&seo_printrepr,
#else /* CONFIG_HAVE_SEQEACH_OPERATOR_REPR */
		/* .tp_printrepr = */ &default_seq_printrepr,
#endif /* !CONFIG_HAVE_SEQEACH_OPERATOR_REPR */
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sew_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&seo_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &seo_math,
	/* .tp_cmp           = */ &sew_cmp,
	/* .tp_seq           = */ &seo_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &seo_attr,
	/* .tp_with          = */ &sew_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seo_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ seo_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&sew_call_kw,
};


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_foreach_bool_cb(void *arg, DeeObject *elem) {
	int result;
	(void)arg;
	result = DeeObject_Bool(elem);
	if (result > 0)
		result = -2;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sso_bool(SeqEachOperator *__restrict self) {
	Dee_ssize_t result = seo_foreach(self, &ss_foreach_bool_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		return 1;
	return (int)result;
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
	return (DREF DeeObject *)seqsome_makeop1(self, OPERATOR_CALL, tuple);
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
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
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
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ss_trycompare_eq_cb(void *arg, DeeObject *elem) {
	int result = DeeObject_TryCompareEq(elem, (DeeObject *)arg);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	if (result != 0)
		result = -2;
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_trycompare_eq(SeqEachOperator *self, DeeObject *other) {
	Dee_ssize_t result = seo_foreach(self, &ss_trycompare_eq_cb, other);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		return 1;
	if unlikely(result == -1)
		goto err;
	return 0;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp sso_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_trycompare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_trycompare_eq,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_ge
};

STATIC_ASSERT(offsetof(SeqEachBase, se_seq) == offsetof(ProxyObject, po_obj));
#define ssw_size_fast generic_proxy_size_fast

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_bounditem(SeqEachOperator *self, DeeObject *index) {
	return seqsome_map_from_bound(seo_foreach(self, &ss_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasitem(SeqEachOperator *self, DeeObject *index) {
	return seqsome_map_from_has(seo_foreach(self, &ss_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sso_bounditem_index(SeqEachOperator *self, size_t index) {
	return seqsome_map_from_bound(seo_foreach(self, &ss_bounditem_index_foreach_cb,
	                                                     (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sso_hasitem_index(SeqEachOperator *self, size_t index) {
	return seqsome_map_from_has(seo_foreach(self, &ss_hasitem_index_foreach_cb,
	                                                   (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_bounditem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_from_bound(seo_foreach(self, &ss_bounditem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasitem_string_hash(SeqEachOperator *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_from_has(seo_foreach(self, &ss_hasitem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_bounditem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_from_bound(seo_foreach(self, &ss_bounditem_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasitem_string_len_hash(SeqEachOperator *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_from_has(seo_foreach(self, &ss_hasitem_string_len_hash_foreach_cb, &data));
}

PRIVATE struct type_seq sso_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ssw_getrange,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&seo_foreach, /* Needed for recursion! */
	/* .tp_foreach_pair               = */ &DeeObject_DefaultForeachPairWithForeach,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sso_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ssw_size_fast,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&sso_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&sso_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sso_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sso_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
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
	return seqsome_map_from_bound(seo_foreach(self, &ss_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasattr(SeqEachOperator *self, DeeObject *attr) {
	return seqsome_map_from_has(seo_foreach(self, &ss_hasattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_boundattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_from_bound(seo_foreach(self, &ss_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasattr_string_hash(SeqEachOperator *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_from_has(seo_foreach(self, &ss_hasattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_boundattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_from_bound(seo_foreach(self, &ss_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sso_hasattr_string_len_hash(SeqEachOperator *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_from_has(seo_foreach(self, &ss_hasattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
ssw_enumattr(DeeTypeObject *UNUSED(tp_self),
             SeqEachBase *self, denum_t proc, void *arg) {
	return ss_enumattr_impl((DeeObject *)self, proc, arg);
}

PRIVATE struct type_attr sso_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&ssw_getattr,
	/* .tp_delattr                       = */ NULL,
	/* .tp_setattr                       = */ NULL,
	/* .tp_enumattr                      = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&ssw_enumattr,
	/* .tp_findattr                      = */ NULL,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&seo_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&seo_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&seo_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&seo_init,
				TYPE_SIZED_ALLOCATOR_R(offsetof(SeqEachOperator, so_opargv), sizeof(SeqEachOperator))
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seo_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&seo_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&sso_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sso_printrepr,
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&ssw_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&seo_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &ssw_math,
	/* .tp_cmp           = */ &sso_cmp,
	/* .tp_seq           = */ &sso_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_iter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &sso_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sso_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&ssw_call_kw,
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
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachOperatorIterator", &self->ei_each))
		goto err;
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
#define sewi_copy  generic_proxy2_copy_recursive1_alias2 /* Copy "ei_iter", alias "ei_each" */
#define sewi_deep  generic_proxy2_deepcopy
#define sewi_fini  generic_proxy2_fini
#define sewi_visit generic_proxy2_visit

STATIC_ASSERT(offsetof(SeqEachIterator, ei_iter) == offsetof(ProxyObject, po_obj));
#define sewi_bool generic_proxy_bool

STATIC_ASSERT(offsetof(SeqEachIterator, ei_iter) == offsetof(ProxyObject, po_obj));
#define sewi_compare_eq    generic_proxy_compare_eq_recursive
#define sewi_compare       generic_proxy_compare_recursive
#define sewi_trycompare_eq generic_proxy_trycompare_eq_recursive

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
			/* .nii_getseq   = */ (dfunptr_t)&sewi_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&sewi_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&sewi_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&sewi_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)&sewi_nii_revert,
			/* .nii_advance  = */ (dfunptr_t)&sewi_nii_advance,
			/* .nii_prev     = */ (dfunptr_t)&sewi_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&sewi_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&sewi_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&sewi_nii_peek
		}
	}
};

PRIVATE struct type_cmp sewi_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sewi_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&sewi_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&sewi_trycompare_eq,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &sewi_nii
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&seoi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sewi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sewi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&seoi_init,
				TYPE_FIXED_ALLOCATOR(SeqEachIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sewi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sewi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sewi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sewi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seoi_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seoi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
	return (DREF DeeObject *)result;
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
	return (DREF DeeObject *)result;
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
	return (DREF DeeObject *)result;
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
	return (DREF DeeObject *)result;
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
