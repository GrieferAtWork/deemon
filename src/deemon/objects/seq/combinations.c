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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_C
#define GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
/**/

#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "combinations.h"
/**/

#include <stddef.h> /* size_t, offsetof */

DECL_BEGIN

/************************************************************************/
/* SEQUENCE                                                             */
/************************************************************************/

STATIC_ASSERT(offsetof(SeqCombinations, sc_seq) == offsetof(ProxyObject, po_obj));
#define sc_fini   generic_proxy__fini
#define sc_visit  generic_proxy__visit
#define src_fini  generic_proxy__fini
#define src_visit generic_proxy__visit
#define sp_fini   generic_proxy__fini
#define sp_visit  generic_proxy__visit

#define src_ctor sc_ctor
#define sp_ctor  sc_ctor
PRIVATE WUNUSED NONNULL((1)) int DCALL
sc_ctor(SeqCombinations *__restrict self) {
	self->sc_seq              = DeeSeq_NewEmpty();
	self->sc_trygetitem_index = &default__seq_operator_trygetitem_index__empty;
	self->sc_seqsize          = 0;
	self->sc_rparam           = 1;
	return 0;
}

#define src_copy sc_copy
#define sp_copy  sc_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sc_copy(SeqCombinations *__restrict self,
        SeqCombinations *__restrict other) {
	Dee_Incref(other->sc_seq);
	self->sc_seq              = other->sc_seq;
	self->sc_trygetitem_index = other->sc_trygetitem_index;
	self->sc_seqsize          = other->sc_seqsize;
	self->sc_rparam           = other->sc_rparam;
	return 0;
}

#define src_deep sc_deep
#define sp_deep  sc_deep
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sc_deep(SeqCombinations *__restrict self,
        SeqCombinations *__restrict other) {
	self->sc_seq = DeeObject_DeepCopy(other->sc_seq);
	if unlikely(!self->sc_seq)
		goto err;
	self->sc_trygetitem_index = DeeObject_RequireMethodHint(self->sc_seq, seq_operator_trygetitem_index);
	self->sc_seqsize          = other->sc_seqsize;
	self->sc_rparam           = other->sc_rparam;
	return 0;
err:
	return -1;
}

#define src_init sc_init
#define sp_init  sc_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
sc_init(SeqCombinations *__restrict self,
        size_t argc, DeeObject *const *argv) {
	if (argc < 1 || argc > 2)
		return err_invalid_argc(DeeType_GetName(Dee_TYPE(self)), argc, 1, 2);
	self->sc_rparam = (size_t)-1;
	if (argc >= 2) {
		if unlikely(DeeObject_AsSize(argv[1], &self->sc_rparam))
			goto err;
		if unlikely(!self->sc_rparam)
			goto err_zero_rparam;
	}
	self->sc_seq = argv[0];
	Dee_Incref(self->sc_seq);
	self->sc_trygetitem_index = DeeObject_RequireMethodHint(self->sc_seq, seq_operator_trygetitem_index);
	self->sc_seqsize = (size_t)-1;
	return 0;
err_zero_rparam:
	DeeError_Throwf(&DeeError_ValueError,
	                "'r' parameter of '%k' must not be 0",
	                Dee_TYPE(self));
err:
	return -1;
}


#define src_serialize sc_serialize
#define sp_serialize  sc_serialize
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sc_serialize(SeqCombinations *__restrict self,
             DeeSerial *__restrict writer,
             Dee_seraddr_t addr) {
	int result = generic_proxy__serialize_and_copy((ProxyObject *)self, writer, addr);
	if likely(result == 0) {
		result = DeeSerial_PutPointer(writer,
		                              addr + offsetof(SeqCombinations, sc_trygetitem_index),
		                              (void const *)self->sc_trygetitem_index);
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sc_getseqsize(SeqCombinations *__restrict self) {
	size_t result = atomic_read(&self->sc_seqsize);
	if (result == (size_t)-1) {
		result = DeeObject_InvokeMethodHint(seq_operator_size, self->sc_seq);
		if likely(result != (size_t)-1)
			atomic_write(&self->sc_seqsize, result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sc_getrparam(SeqCombinations *__restrict self) {
	size_t result = atomic_read(&self->sc_rparam);
	if (result == (size_t)-1) {
		result = sc_getseqsize(self);
		if unlikely(result == 0)
			result = 1;
		if likely(result != (size_t)-1)
			atomic_write(&self->sc_rparam, result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sc_iter(SeqCombinations *__restrict self) {
	DREF SeqCombinationsIterator *result;
	size_t i, seqsize, rparam;
	seqsize = sc_getseqsize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err; /* Must be loaded for there to be an iterator! */
	rparam = sc_getrparam(self);
	if unlikely(rparam == (size_t)-1)
		goto err;
	if unlikely(rparam > seqsize)
		return DeeIterator_NewEmpty();
	result = (DREF SeqCombinationsIterator *)DeeObject_Mallocc(offsetof(SeqCombinationsIterator, sci_idx),
	                                                           rparam, sizeof(size_t));
	if unlikely(!result)
		goto err;
	/* Fill in iterator as the normal matrix (truncated to "rparam"). */
	for (i = 0; i < rparam; ++i)
		result->sci_idx[i] = i;
	ASSERT(rparam != 0);
	--result->sci_idx[rparam - 1]; /* Hack to get the first element to load correctly */
	Dee_Incref(self);
	result->sci_com = self;
	Dee_weakref_initempty(&result->sci_view);
	DeeObject_Init(result, &SeqCombinationsIterator_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
src_iter(SeqCombinations *__restrict self) {
	DREF SeqCombinationsIterator *result;
	size_t rparam;
	if unlikely(sc_getseqsize(self) == (size_t)-1)
		goto err; /* Must be loaded for there to be an iterator! */
	rparam = sc_getrparam(self);
	if unlikely(rparam == (size_t)-1)
		goto err;
	result = (DREF SeqCombinationsIterator *)DeeObject_Callocc(offsetof(SeqCombinationsIterator, sci_idx),
	                                                           rparam, sizeof(size_t));
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->sci_com = self;
	result->sci_idx[rparam - 1] = (size_t)-1; /* Hack to get the first element to load correctly */
	Dee_weakref_initempty(&result->sci_view);
	DeeObject_Init(result, &SeqRepeatCombinationsIterator_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_iter(SeqCombinations *__restrict self) {
	DREF SeqCombinationsIterator *result;
	size_t i, seqsize, rparam;
	seqsize = sc_getseqsize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err; /* Must be loaded for there to be an iterator! */
	rparam = sc_getrparam(self);
	if unlikely(rparam == (size_t)-1)
		goto err;
	if unlikely(rparam > seqsize)
		return DeeIterator_NewEmpty();
	result = (DREF SeqCombinationsIterator *)DeeObject_Mallocc(offsetof(SeqCombinationsIterator, sci_idx),
	                                                           rparam, sizeof(size_t));
	if unlikely(!result)
		goto err;
	/* Fill in iterator as the normal matrix (truncated to "rparam"). */
	for (i = 0; i < rparam; ++i)
		result->sci_idx[i] = i;
	ASSERT(rparam != 0);
	--result->sci_idx[rparam - 1]; /* Hack to get the first element to load correctly */
	Dee_Incref(self);
	result->sci_com = self;
	Dee_weakref_initempty(&result->sci_view);
	DeeObject_Init(result, &SeqPermutationsIterator_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE struct type_seq sc_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sc_iter,
	/* TODO: Formula for getitem/size */
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq src_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&src_iter,
	/* TODO: Formula for getitem/size */
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq sp_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sp_iter,
	/* TODO: Formula for getitem/size */
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sc_getseqsize_ob(SeqCombinations *__restrict self) {
	size_t result = sc_getseqsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sc_getrparam_ob(SeqCombinations *__restrict self) {
	size_t result = sc_getrparam(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


#define src_getsets sc_getsets
#define sp_getsets  sc_getsets
PRIVATE struct type_getset tpconst sc_getsets[] = {
	TYPE_GETTER_AB_F("__seqsize__", &sc_getseqsize_ob, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB_F("__rparam__", &sc_getrparam_ob, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

#define src_members sc_members
#define sp_members  sc_members
PRIVATE struct type_member tpconst sc_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SeqCombinations, sc_seq), "->?DSequence"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sc_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqCombinationsIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &SeqCombinationsView_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst src_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqRepeatCombinationsIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &SeqCombinationsView_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sp_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqPermutationsIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &SeqCombinationsView_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqCombinations_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqCombinations",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,r?:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqCombinations,
			/* tp_ctor:        */ &sc_ctor,
			/* tp_copy_ctor:   */ &sc_copy,
			/* tp_deep_ctor:   */ &sc_deep,
			/* tp_any_ctor:    */ &sc_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sc_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sc_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sc_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &sc_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ sc_getsets,
	/* .tp_members       = */ sc_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sc_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject SeqRepeatCombinations_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatCombinations",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,r?:?Dint)\n"
	                         "\n"
	                         "iter->?#Iterator"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqCombinations,
			/* tp_ctor:        */ &src_ctor,
			/* tp_copy_ctor:   */ &src_copy,
			/* tp_deep_ctor:   */ &src_deep,
			/* tp_any_ctor:    */ &src_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &src_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&src_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&src_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &src_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ src_getsets,
	/* .tp_members       = */ src_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ src_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject SeqPermutations_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqPermutations",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,r?:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqCombinations,
			/* tp_ctor:        */ &sp_ctor,
			/* tp_copy_ctor:   */ &sp_copy,
			/* tp_deep_ctor:   */ &sp_deep,
			/* tp_any_ctor:    */ &sp_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sp_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sp_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &sp_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ sp_getsets,
	/* .tp_members       = */ sp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sp_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};















/************************************************************************/
/* ITERATOR                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsIterator *DCALL
generic_default_iter_of(DeeTypeObject *__restrict type) {
	DREF SeqCombinationsIterator *result;
	DREF SeqCombinations *com;
	com = (DREF SeqCombinations *)DeeObject_NewDefault(type);
	if unlikely(!com)
		goto err;
	result = (DREF SeqCombinationsIterator *)DeeObject_Iter((DeeObject *)com);
	Dee_Decref_unlikely(com);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF SeqCombinationsIterator *DCALL sci_ctor(void) {
	return generic_default_iter_of(&SeqCombinations_Type);
}

PRIVATE WUNUSED DREF SeqCombinationsIterator *DCALL srci_ctor(void) {
	return generic_default_iter_of(&SeqRepeatCombinations_Type);
}

PRIVATE WUNUSED DREF SeqCombinationsIterator *DCALL spi_ctor(void) {
	return generic_default_iter_of(&SeqPermutations_Type);
}

#define srci_copy sci_copy
#define spi_copy  sci_copy
PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsIterator *DCALL
sci_copy(SeqCombinationsIterator *__restrict self) {
	DREF SeqCombinationsIterator *result;
	size_t i, rparam = atomic_read(&self->sci_com->sc_rparam);
	result = (DREF SeqCombinationsIterator *)DeeObject_Mallocc(offsetof(SeqCombinationsIterator, sci_idx),
	                                                           rparam, sizeof(size_t));
	if unlikely(!result)
		goto err;
	Dee_Incref(self->sci_com);
	result->sci_com = self->sci_com;
	for (i = 0; i < rparam; ++i)
		result->sci_idx[i] = atomic_read(&self->sci_idx[i]);
	Dee_weakref_initempty(&result->sci_view);
	DeeObject_Init(result, Dee_TYPE(self));
	return result;
err:
	return NULL;
}

#define srci_deep sci_deep
#define spi_deep  sci_deep
PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsIterator *DCALL
sci_deep(SeqCombinationsIterator *__restrict self) {
	DREF SeqCombinationsIterator *result = sci_copy(self);
	if (DeeObject_InplaceDeepCopy((DREF DeeObject **)&result->sci_com))
		goto err_r;
	return result;
err_r:
	Dee_DecrefDokill(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((3, 4)) DREF SeqCombinationsIterator *DCALL
sci_init_generic(size_t argc, DeeObject *const *argv,
                 DeeTypeObject *expected_seq_type,
                 DeeTypeObject *returned_iter_type) {
	SeqCombinations *com;
	if unlikely(argc != 1)
		goto err_bad_argc;
	com = (DREF SeqCombinations *)argv[0];
	if (DeeObject_AssertTypeExact(com, expected_seq_type))
		goto err;
	return (DREF SeqCombinationsIterator *)DeeObject_Iter((DeeObject *)com);
err_bad_argc:
	err_invalid_argc(DeeType_GetName(returned_iter_type), argc, 1, 1);
err:
	return NULL;
}

PRIVATE WUNUSED DREF SeqCombinationsIterator *DCALL
sci_init(size_t argc, DeeObject *const *argv) {
	return sci_init_generic(argc, argv, &SeqCombinations_Type, &SeqCombinationsIterator_Type);
}

PRIVATE WUNUSED DREF SeqCombinationsIterator *DCALL
srci_init(size_t argc, DeeObject *const *argv) {
	return sci_init_generic(argc, argv, &SeqRepeatCombinations_Type, &SeqRepeatCombinationsIterator_Type);
}

PRIVATE WUNUSED DREF SeqCombinationsIterator *DCALL
spi_init(size_t argc, DeeObject *const *argv) {
	return sci_init_generic(argc, argv, &SeqPermutations_Type, &SeqPermutationsIterator_Type);
}

#define srci_fini sci_fini
#define spi_fini  sci_fini
PRIVATE NONNULL((1)) void DCALL
sci_fini(SeqCombinationsIterator *__restrict self) {
	Dee_Decref(self->sci_com);
	Dee_weakref_fini(&self->sci_view);
}

#define srci_serialize sci_serialize
#define spi_serialize  sci_serialize
PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
sci_serialize(SeqCombinationsIterator *__restrict self,
              DeeSerial *__restrict writer) {
	SeqCombinationsIterator *out;
	size_t i, idx_count = self->sci_com->sc_rparam;
	size_t sizeof_self = offsetof(SeqCombinationsIterator, sci_idx) + (idx_count * sizeof(size_t));
	Dee_seraddr_t out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
	if (generic_proxy__serialize((ProxyObject *)self, writer, out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, SeqCombinationsIterator);
	Dee_weakref_initempty(&out->sci_view);
	for (i = 0; i < idx_count; ++i) {
		out->sci_idx[i] = atomic_read(&self->sci_idx[i]);
	}
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
}

STATIC_ASSERT(offsetof(SeqCombinationsIterator, sci_com) == offsetof(ProxyObject, po_obj));
#define sci_visit  generic_proxy__visit
#define srci_visit generic_proxy__visit
#define spi_visit  generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
sci_unshare_view(SeqCombinationsIterator *__restrict self) {
	DREF SeqCombinationsView *view;
	view = (DREF SeqCombinationsView *)Dee_weakref_lock(&self->sci_view);
	if (!view)
		return 0;
	if (SeqCombinationsView_GetIdx(view) == self->sci_idx) {
		/* Do the unshare */
		size_t i, rparam = self->sci_com->sc_rparam, *idx_copy;
		idx_copy = (size_t *)Dee_Mallocc(rparam, sizeof(size_t));
		if unlikely(!idx_copy)
			goto err_view;
		for (i = 0; i < rparam; ++i)
			idx_copy[i] = atomic_read(&self->sci_idx[i]);
		/* Store the unshared copy within the view (this also asserts
		 * that no other thread already did this in the mean-time, and
		 * if some other thread did, then we simply free our copy) */
		if unlikely(!SeqCombinationsView_SetIdx(view, idx_copy))
			Dee_Free(idx_copy);
	}
	Dee_Decref_unlikely(view);
	/* Clear the weakref since its indices won't reference us anymore. */
	(void)Dee_weakref_clear(&self->sci_view);
	return 0;
err_view:
	Dee_Decref_unlikely(view);
	return -1;
}

#define srci_getview sci_getview
#define spi_getview  sci_getview
PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsView *DCALL
sci_getview(SeqCombinationsIterator *__restrict self) {
	DREF SeqCombinationsView *result;
	result = (DREF SeqCombinationsView *)Dee_weakref_lock(&self->sci_view);
	if (!result) {
		DREF SeqCombinationsView *old_view;
		/* Lazily allocate view */
		result = DeeObject_MALLOC(SeqCombinationsView);
		if unlikely(!result)
			goto err;
		Dee_Incref(self);
		result->scv_iter = self;
		result->scv_com  = self->sci_com;
		result->scv_idx  = self->sci_idx;
		weakref_support_init(result);
		DeeObject_Init(result, &SeqCombinationsView_Type);
		COMPILER_WRITE_BARRIER();

		/* Cache the view within the weakref, and handle the
		 * case where another view was created in the mean-time. */
		old_view = (DREF SeqCombinationsView *)Dee_weakref_cmpxch(&self->sci_view, NULL,
		                                                          (DeeObject *)result);
		ASSERT(old_view != (DREF SeqCombinationsView *)ITER_DONE);
		if unlikely(old_view) {
			Dee_DecrefDokill(result);
			result = old_view;
		}
	}
	return result;
err:
	return NULL;
}

#define srci_assign sci_assign
#define spi_assign  sci_assign
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sci_assign(SeqCombinationsIterator *self,
           SeqCombinationsIterator *other) {
	size_t i;
	if unlikely(self == other)
		return 0;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	if unlikely(self->sci_com->sc_seq != other->sci_com->sc_seq ||
	            self->sci_com->sc_rparam != other->sci_com->sc_rparam)
		goto wrong_com;
	if unlikely(sci_unshare_view(self))
		goto err;
	for (i = 0; i < self->sci_com->sc_rparam; ++i)
		self->sci_idx[i] = atomic_read(&other->sci_idx[i]);
	return 0;
wrong_com:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot assign incompatible combination iterators");
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sci_bool(SeqCombinationsIterator *__restrict self) {
	size_t i, rparam = self->sci_com->sc_rparam;
	size_t seqsize = self->sci_com->sc_seqsize;
	size_t value;
	ASSERT(rparam != (size_t)-1);
	ASSERT(seqsize != (size_t)-1);
	ASSERT(rparam > 0);
	i = rparam;
	do {
		--i;
		value = atomic_read(&self->sci_idx[i]);
		if (value < i + seqsize - rparam)
			return 1;
	} while (i);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
srci_bool(SeqCombinationsIterator *__restrict self) {
	size_t i, rparam = self->sci_com->sc_rparam;
	size_t offset, seqsize = self->sci_com->sc_seqsize;
	ASSERT(rparam != (size_t)-1);
	ASSERT(seqsize != (size_t)-1);
	ASSERT(rparam > 0);
	i = rparam;
	do {
		--i;
		offset = atomic_read(&self->sci_idx[i]) + 1;
		if (offset < seqsize)
			return 1;
	} while (i);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
spi_bool(SeqCombinationsIterator *__restrict self) {
	size_t i, rparam = self->sci_com->sc_rparam;
	size_t seqsize = self->sci_com->sc_seqsize;
	ASSERT(rparam != (size_t)-1);
	ASSERT(seqsize != (size_t)-1);
	ASSERT(rparam > 0);
	/* For "ABCD".permutations(3), the final state is "DCB", so check for that! */
	i = 0;
	do {
		if ((self->sci_idx[i] + 1) < (seqsize - i))
			return 1;
	} while (++i < rparam);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
sci_inc(SeqCombinationsIterator *__restrict self) {
	size_t i, rparam = self->sci_com->sc_rparam;
	size_t seqsize = self->sci_com->sc_seqsize;
	size_t value;
	ASSERT(rparam != (size_t)-1);
	ASSERT(seqsize != (size_t)-1);
	ASSERT(rparam > 0);
	i = rparam;
	do {
		--i;
		value = atomic_read(&self->sci_idx[i]);
		if (value < i + seqsize - rparam)
			goto do_inc_at_i;
	} while (i);
	return false;
do_inc_at_i:
	do {
		++value;
		atomic_write(&self->sci_idx[i], value);
	} while (++i < rparam);
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
srci_inc(SeqCombinationsIterator *__restrict self) {
	size_t i, rparam = self->sci_com->sc_rparam;
	size_t offset, seqsize = self->sci_com->sc_seqsize;
	ASSERT(rparam != (size_t)-1);
	ASSERT(seqsize != (size_t)-1);
	ASSERT(rparam > 0);
	i = rparam;
	do {
		--i;
		offset = atomic_read(&self->sci_idx[i]) + 1;
		if (offset < seqsize)
			goto do_inc_at_i;
	} while (i);
	return false;
do_inc_at_i:
	do {
		atomic_write(&self->sci_idx[i], offset);
	} while (++i < rparam);
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
spi_inc(SeqCombinationsIterator *__restrict self) {
	size_t i, rparam = self->sci_com->sc_rparam;
	size_t seqsize = self->sci_com->sc_seqsize;
	ASSERT(rparam != (size_t)-1);
	ASSERT(seqsize != (size_t)-1);
	ASSERT(rparam > 0);
	if (self->sci_idx[0] >= seqsize && (self->sci_idx[0] != (size_t)-1 && rparam != 0))
		return false;
	i = rparam;
	for (;;) {
		size_t j, index;
		--i;
increment_i:
		ASSERT(i < rparam);
		if (++self->sci_idx[i] >= seqsize) {
			if (i == 0) {
				self->sci_idx[i] = seqsize - 1;
				return false;
			}
			self->sci_idx[i] = 0;
			continue;
		}
		index = self->sci_idx[i];
		for (j = 0; j < rparam; ++j) {
			if (j == i)
				continue;
			if (self->sci_idx[j] == index)
				goto increment_i;
		}
		break;
	}
	return true;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsView *DCALL
sci_next(SeqCombinationsIterator *__restrict self) {
	if unlikely(sci_unshare_view(self))
		goto err;
	if (!sci_inc(self))
		return (DREF SeqCombinationsView *)ITER_DONE;
	return sci_getview(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsView *DCALL
srci_next(SeqCombinationsIterator *__restrict self) {
	if unlikely(sci_unshare_view(self))
		goto err;
	if (!srci_inc(self))
		return (DREF SeqCombinationsView *)ITER_DONE;
	return srci_getview(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqCombinationsView *DCALL
spi_next(SeqCombinationsIterator *__restrict self) {
	if unlikely(sci_unshare_view(self))
		goto err;
	if (!spi_inc(self))
		return (DREF SeqCombinationsView *)ITER_DONE;
	return spi_getview(self);
err:
	return NULL;
}

#define srci_compare sci_compare
#define spi_compare  sci_compare
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sci_compare(SeqCombinationsIterator *lhs, SeqCombinationsIterator *rhs) {
	size_t i, rparam;
	if (DeeObject_AssertTypeExact(rhs, Dee_TYPE(lhs)))
		goto err;
	Dee_return_compare_if_ne(lhs->sci_com, lhs->sci_com);
	rparam = lhs->sci_com->sc_rparam;
	ASSERT(rparam != (size_t)-1);
	/* All combinations iterators increment starting at "sci_idx[0]",
	 * so we can compare all of them the same way by starting at the
	 * greatest index "sci_idx[rparam - 1]" */
	i = rparam;
	do {
		Dee_ssize_t lhs_index, rhs_index;
		--i;
		/* Force use of signed indices here, so we're correctly handling
		 * the case where we make use of integer roll-over to represent
		 * the -1'th matrix state (as set-up for the initial state) */
		lhs_index = (Dee_ssize_t)atomic_read(&lhs->sci_idx[i]);
		rhs_index = (Dee_ssize_t)atomic_read(&rhs->sci_idx[i]);
		Dee_return_compare_if_ne(lhs_index, rhs_index);
	} while (i);
	return 0;
err:
	return Dee_COMPARE_ERR;
}


#define srci_cmp sci_cmp
#define spi_cmp  sci_cmp
PRIVATE struct type_cmp sci_cmp = {
	/* .tp_hash       = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sci_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

#define srci_getsets sci_getsets
#define spi_getsets  sci_getsets
PRIVATE struct type_getset tpconst sci_getsets[] = {
	TYPE_GETTER_AB("__view__", &sci_getview, "->?Ert:SeqCombinationsView"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst sci_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqCombinationsIterator, sci_com), "->?Ert:SeqCombinations"),
	TYPE_MEMBER_FIELD_DOC("__wview__", STRUCT_WOBJECT, offsetof(SeqCombinationsIterator, sci_view), "->?Ert:SeqCombinationsView"),
	TYPE_MEMBER_END
};
#ifdef CONFIG_NO_DOC
#define srci_members sci_members
#define spi_members  sci_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member tpconst srci_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqCombinationsIterator, sci_com), "->?Ert:SeqRepeatCombinations"),
	TYPE_MEMBER_FIELD_DOC("__wview__", STRUCT_WOBJECT, offsetof(SeqCombinationsIterator, sci_view), "->?Ert:SeqCombinationsView"),
	TYPE_MEMBER_END
};
PRIVATE struct type_member tpconst spi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqCombinationsIterator, sci_com), "->?Ert:SeqPermutations"),
	TYPE_MEMBER_FIELD_DOC("__wview__", STRUCT_WOBJECT, offsetof(SeqCombinationsIterator, sci_view), "->?Ert:SeqCombinationsView"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */

INTERN DeeTypeObject SeqCombinationsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqCombinationsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqCombinations)\n"
	                         "\n"
	                         "iter->?Ert:SeqCombinationsView"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &sci_ctor,
			/* tp_copy_ctor:   */ &sci_copy,
			/* tp_deep_ctor:   */ &sci_deep,
			/* tp_any_ctor:    */ &sci_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sci_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sci_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&sci_assign,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sci_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sci_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &sci_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sci_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ sci_getsets,
	/* .tp_members       = */ sci_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject SeqRepeatCombinationsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatCombinationsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqRepeatCombinations)\n"
	                         "\n"
	                         "iter->?Ert:SeqCombinationsView"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &srci_ctor,
			/* tp_copy_ctor:   */ &srci_copy,
			/* tp_deep_ctor:   */ &srci_deep,
			/* tp_any_ctor:    */ &srci_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &srci_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srci_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&srci_assign,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&srci_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&srci_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &srci_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&srci_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ srci_getsets,
	/* .tp_members       = */ srci_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject SeqPermutationsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqPermutationsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqPermutations)\n"
	                         "\n"
	                         "iter->?Ert:SeqCombinationsView"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &spi_ctor,
			/* tp_copy_ctor:   */ &spi_copy,
			/* tp_deep_ctor:   */ &spi_deep,
			/* tp_any_ctor:    */ &spi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &spi_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&spi_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&spi_assign,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&spi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&spi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &spi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&spi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ spi_getsets,
	/* .tp_members       = */ spi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};















/************************************************************************/
/* VIEW                                                                 */
/************************************************************************/

STATIC_ASSERT(offsetof(SeqCombinationsView, scv_iter) == offsetof(ProxyObject, po_obj));
#define scv_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
scv_ctor(SeqCombinationsView *__restrict self) {
	self->scv_iter = (DREF SeqCombinationsIterator *)DeeObject_NewDefault(&SeqCombinationsIterator_Type);
	if unlikely(!self->scv_iter)
		goto err;
	self->scv_com = self->scv_iter->sci_com;
	self->scv_idx = self->scv_iter->sci_idx;
	Dee_weakref_support_init(self);
	(void)Dee_weakref_set(&self->scv_iter->sci_view, Dee_AsObject(self));
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
scv_fini(SeqCombinationsView *__restrict self) {
	if unlikely(self->scv_idx != self->scv_iter->sci_idx)
		Dee_Free(self->scv_idx);
	Dee_Decref_unlikely(self->scv_iter);
	Dee_weakref_support_fini(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
scv_copy(SeqCombinationsView *__restrict self,
         SeqCombinationsView *__restrict other) {
	size_t i, rparam = other->scv_com->sc_rparam, *idx_copy, *idx_other;
	idx_copy = (size_t *)Dee_Mallocc(rparam, sizeof(size_t));
	if unlikely(!idx_copy)
		goto err;
	idx_other = SeqCombinationsView_GetIdx(other);
	for (i = 0; i < rparam; ++i)
		idx_copy[i] = atomic_read(&idx_other[i]);
	self->scv_idx = idx_copy;
	Dee_Incref(other->scv_iter);
	self->scv_iter = other->scv_iter;
	self->scv_com  = self->scv_iter->sci_com;
	Dee_weakref_support_init(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
scv_deep(SeqCombinationsView *__restrict self,
         SeqCombinationsView *__restrict other) {
	if unlikely(scv_copy(self, other))
		goto err;
	if (DeeObject_InplaceDeepCopy((DeeObject **)&self->scv_iter))
		goto err_self;
	self->scv_com = self->scv_iter->sci_com;
	return 0;
err_self:
	scv_fini(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scv_init(SeqCombinationsView *__restrict self,
         size_t argc, DeeObject *const *argv) {
	size_t i, rparam, *idx_copy, *idx_other;
	DeeArg_Unpack1(err, argc, argv, "_SeqCombinationsView", &self->scv_iter);
	if unlikely(!DeeObject_InstanceOfExact(self->scv_iter, &SeqCombinationsIterator_Type) &&
	            !DeeObject_InstanceOfExact(self->scv_iter, &SeqRepeatCombinationsIterator_Type) &&
	            !DeeObject_InstanceOfExact(self->scv_iter, &SeqPermutationsIterator_Type))
		goto err_bad_iter_type;
	self->scv_com = self->scv_iter->sci_com;
	rparam = self->scv_com->sc_rparam;
	idx_copy = (size_t *)Dee_Mallocc(rparam, sizeof(size_t));
	if unlikely(!idx_copy)
		goto err;
	idx_other = self->scv_iter->sci_idx;
	for (i = 0; i < rparam; ++i)
		idx_copy[i] = atomic_read(&idx_other[i]);
	self->scv_idx = idx_copy;
	Dee_Incref(self->scv_iter);
	Dee_weakref_support_init(self);
	return 0;
err_bad_iter_type:
	DeeObject_TypeAssertFailed3(Dee_AsObject(self->scv_iter),
	                            &SeqCombinationsIterator_Type,
	                            &SeqRepeatCombinationsIterator_Type,
	                            &SeqPermutationsIterator_Type);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
scv_serialize(SeqCombinationsView *__restrict self,
              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	size_t i, *in__scv_idx, *ou__scv_idx;
	Dee_seraddr_t out__scv_idx;
	if (generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	if (DeeSerial_PutPointer(writer, addr + offsetof(SeqCombinationsView, scv_com), self->scv_com))
		goto err;
	in__scv_idx = atomic_read(&self->scv_idx);
	if (in__scv_idx == self->scv_iter->sci_idx)
		return DeeSerial_PutPointer(writer, addr + offsetof(SeqCombinationsView, scv_idx), in__scv_idx);
	out__scv_idx = DeeSerial_Malloc(writer, self->scv_com->sc_rparam * sizeof(size_t), NULL);
	if (!Dee_SERADDR_ISOK(out__scv_idx))
		goto err;
	if (DeeSerial_PutAddr(writer, addr + offsetof(SeqCombinationsView, scv_idx), out__scv_idx))
		goto err;
	ou__scv_idx = DeeSerial_Addr2Mem(writer, out__scv_idx, size_t);
	for (i = 0; i < self->scv_com->sc_rparam; ++i)
		ou__scv_idx[i] = atomic_read(&in__scv_idx[i]);
	return 0;
err:
	return -1;
}


PRIVATE struct type_member tpconst scv_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqCombinationsView, scv_iter),
	                      "->?X3?Ert:SeqCombinationsIterator"
	                      /**/ "?Ert:SeqRepeatCombinationsIterator"
	                      /**/ "?Ert:SeqPermutationsIterator"),
	TYPE_MEMBER_FIELD_DOC("__com__", STRUCT_OBJECT, offsetof(SeqCombinationsView, scv_com),
	                      "->?X3?Ert:SeqCombinations"
	                      /**/ "?Ert:SeqRepeatCombinations"
	                      /**/ "?Ert:SeqPermutations"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
scv_size(SeqCombinationsView *__restrict self) {
	ASSERT(self->scv_com->sc_rparam != (size_t)-1);
	return self->scv_com->sc_rparam;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scv_getitem_index(SeqCombinationsView *__restrict self, size_t index) {
	size_t *idx = SeqCombinationsView_GetIdx(self);
	ASSERT(self->scv_com->sc_rparam != (size_t)-1);
	if unlikely(index >= self->scv_com->sc_rparam)
		goto err_oob;
	index = idx[index];
	return DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->scv_com->sc_seq, index);
err_oob:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->scv_com->sc_rparam);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scv_trygetitem_index(SeqCombinationsView *__restrict self, size_t index) {
	size_t *idx = SeqCombinationsView_GetIdx(self);
	ASSERT(self->scv_com->sc_rparam != (size_t)-1);
	if unlikely(index >= self->scv_com->sc_rparam)
		return ITER_DONE;
	index = idx[index];
	return (*self->scv_com->sc_trygetitem_index)(self->scv_com->sc_seq, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scv_bounditem_index(SeqCombinationsView *__restrict self, size_t index) {
	size_t *idx = SeqCombinationsView_GetIdx(self);
	ASSERT(self->scv_com->sc_rparam != (size_t)-1);
	if unlikely(index >= self->scv_com->sc_rparam)
		return Dee_BOUND_MISSING;
	index = idx[index];
	return DeeObject_InvokeMethodHint(seq_operator_bounditem_index, self->scv_com->sc_seq, index);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
scv_foreach(SeqCombinationsView *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i, rparam = self->scv_com->sc_rparam, *idx = SeqCombinationsView_GetIdx(self);
	ASSERT(rparam != (size_t)-1);
	for (i = 0; i < rparam; ++i) {
		DREF DeeObject *elem;
		size_t index = idx[i];
		elem = (*self->scv_com->sc_trygetitem_index)(self->scv_com->sc_seq, index);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
		} else {
			temp = (*cb)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
scv_mh_seq_enumerate_index(SeqCombinationsView *__restrict self,
                           Dee_seq_enumerate_index_t proc,
                           void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	size_t i, *idx = SeqCombinationsView_GetIdx(self);
	ASSERT(self->scv_com->sc_rparam != (size_t)-1);
	if (end > self->scv_com->sc_rparam)
		end = self->scv_com->sc_rparam;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		size_t index = idx[i];
		elem = (*self->scv_com->sc_trygetitem_index)(self->scv_com->sc_seq, index);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
		} else {
			temp = (*proc)(arg, i, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
scv_mh_seq_enumerate_index_reverse(SeqCombinationsView *__restrict self,
                                   Dee_seq_enumerate_index_t proc,
                                   void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	size_t *idx = SeqCombinationsView_GetIdx(self);
	ASSERT(self->scv_com->sc_rparam != (size_t)-1);
	if (end > self->scv_com->sc_rparam)
		end = self->scv_com->sc_rparam;
	while (end > start) {
		DREF DeeObject *elem;
		size_t index = idx[--end];
		elem = (*self->scv_com->sc_trygetitem_index)(self->scv_com->sc_seq, index);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
		} else {
			temp = (*proc)(arg, end, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE struct type_method tpconst scv_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst scv_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &scv_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &scv_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq scv_seq = {
	/* .tp_iter               = */ DEFIMPL(&default__iter__with__foreach),
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&scv_foreach,
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&scv_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&scv_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&scv_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&scv_bounditem_index,
	/* .tp_hasitem_index      = */ &default__seq_operator_hasitem_index__with__seq_operator_size,
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index   = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&scv_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

INTERN DeeTypeObject SeqCombinationsView_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqCombinationsView",
	/* .tp_doc      = */ DOC("()\n"
	                         "(iter:?X3?Ert:SeqCombinationsIterator"
	                         /*    */ "?Ert:SeqRepeatCombinationsIterator"
	                         /*    */ "?Ert:SeqPermutationsIterator)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(SeqCombinationsView),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqCombinationsView,
			/* tp_ctor:        */ &scv_ctor,
			/* tp_copy_ctor:   */ &scv_copy,
			/* tp_deep_ctor:   */ &scv_deep,
			/* tp_any_ctor:    */ &scv_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &scv_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&scv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&scv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &scv_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ scv_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ scv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ scv_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


PRIVATE DEFINE_TUPLE(empty_combinations, 1, { Dee_EmptyTuple });

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
SeqCombinations_New(/*inherit(always)*/ DREF DeeObject *__restrict seq,
                    size_t rparam, DeeTypeObject *__restrict type) {
	DREF SeqCombinations *result;
	if unlikely(!rparam) {
		Dee_Decref(seq);
		Dee_Incref(&empty_combinations);
		return (DeeObject *)&empty_combinations;
	}
	result = DeeObject_MALLOC(SeqCombinations);
	if unlikely(!result)
		goto err_seq;
	result->sc_trygetitem_index = DeeObject_RequireMethodHint(seq, seq_operator_trygetitem_index);
	if unlikely(result->sc_trygetitem_index == &default__seq_operator_trygetitem_index__unsupported)
		goto err_seq_r_combinations_not_supported;
	result->sc_seqsize = (size_t)-1; /* Will be calculated lazily */
	result->sc_rparam  = rparam;
	result->sc_seq     = seq; /* Inherit reference */
	DeeObject_Init(result, type);
	return Dee_AsObject(result);
err_seq_r_combinations_not_supported:
	DeeObject_FREE(result);
	DeeError_Throwf(&DeeError_SequenceError,
	                "Type %r does not support sequence combinations",
	                Dee_TYPE(seq));
err_seq:
	Dee_Decref(seq);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Combinations(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t r) {
	return SeqCombinations_New(self, r, &SeqCombinations_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatCombinations(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t r) {
	return SeqCombinations_New(self, r, &SeqRepeatCombinations_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Permutations2(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t r) {
	return SeqCombinations_New(self, r, &SeqPermutations_Type);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_C */
