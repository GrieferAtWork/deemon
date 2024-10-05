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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
/**/

#include "default-enumerate.h"
#include "default-iterators.h"
/**/

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

#define de_wsagiif_copy   fullrange_copy
#define de_wsatgii_copy   fullrange_copy
#define de_wsagii_copy    fullrange_copy
#define de_wsoagi_copy    fullrange_copy
#define de_wsagiifaf_copy withintrange_copy
#define de_wsatgiiaf_copy withintrange_copy
#define de_wsagiiaf_copy  withintrange_copy
#define de_wsoagiaf_copy  withrange_copy
#define de_wgii_copy      fullrange_copy
#define de_wgiiaf_copy    withintrange_copy
#define de_wgiaf_copy     withrange_copy
#define de_wikagi_copy    fullrange_copy
#define de_wikagiaf_copy  withrange_copy
#define de_wikatgi_copy   fullrange_copy
#define de_wikatgiaf_copy withrange_copy
#define de_wiac_copy      fullrange_copy
#define de_wiacaf_copy    withintrange_copy
#define de_wiauaf_copy    withintrange_copy
#define de_we_copy        fullrange_copy
#define de_wei_copy       withintrange_copy
#define de_weaf_copy      withrange_copy

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fullrange_copy(DefaultEnumeration_FullRange *__restrict self,
               DefaultEnumeration_FullRange *__restrict other) {
	Dee_Incref(other->defr_seq);
	self->defr_seq    = other->defr_seq;
	self->defr_tp_seq = other->defr_tp_seq;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
withintrange_copy(DefaultEnumeration_WithIntRange *__restrict self,
                  DefaultEnumeration_WithIntRange *__restrict other) {
	self->dewir_seq = other->dewir_seq;
	Dee_Incref(self->dewir_seq);
	self->dewir_tp_seq = other->dewir_tp_seq;
	self->dewir_start  = other->dewir_start;
	self->dewir_end    = other->dewir_end;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
withrange_copy(DefaultEnumeration_WithRange *__restrict self,
               DefaultEnumeration_WithRange *__restrict other) {
	Dee_Incref(other->dewr_seq);
	self->dewr_seq = other->dewr_seq;
	self->dewr_tp_seq = other->dewr_tp_seq;
	Dee_Incref(other->dewr_start);
	self->dewr_start = other->dewr_start;
	Dee_Incref(other->dewr_end);
	self->dewr_end = other->dewr_end;
	return 0;
}


#define de_wsagiif_deepcopy   fullrange_deepcopy
#define de_wsatgii_deepcopy   fullrange_deepcopy
#define de_wsagii_deepcopy    fullrange_deepcopy
#define de_wsoagi_deepcopy    fullrange_deepcopy
#define de_wsagiifaf_deepcopy withintrange_deepcopy_with_size_fix
#define de_wsatgiiaf_deepcopy withintrange_deepcopy
#define de_wsagiiaf_deepcopy  withintrange_deepcopy
#define de_wsoagiaf_deepcopy  withrange_deepcopy
#define de_wgii_deepcopy      fullrange_deepcopy
#define de_wgiiaf_deepcopy    withintrange_deepcopy
#define de_wgiaf_deepcopy     withrange_deepcopy
#define de_wikagi_deepcopy    fullrange_deepcopy
#define de_wikagiaf_deepcopy  withrange_deepcopy
#define de_wikatgi_deepcopy   fullrange_deepcopy
#define de_wikatgiaf_deepcopy withrange_deepcopy
#define de_wiac_deepcopy      fullrange_deepcopy
#define de_wiacaf_deepcopy    withintrange_deepcopy
#define de_wiauaf_deepcopy    withintrange_deepcopy
#define de_we_deepcopy        fullrange_deepcopy
#define de_wei_deepcopy       withintrange_deepcopy
#define de_weaf_deepcopy      withrange_deepcopy

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fullrange_deepcopy(DefaultEnumeration_FullRange *__restrict self,
                   DefaultEnumeration_FullRange *__restrict other) {
	self->defr_seq = DeeObject_DeepCopy(other->defr_seq);
	if unlikely(!self->defr_seq)
		goto err;
	self->defr_tp_seq = other->defr_tp_seq;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
withintrange_deepcopy(DefaultEnumeration_WithIntRange *__restrict self,
                      DefaultEnumeration_WithIntRange *__restrict other) {
	self->dewir_seq = DeeObject_DeepCopy(other->dewir_seq);
	if unlikely(!self->dewir_seq)
		goto err;
	self->dewir_tp_seq = other->dewir_tp_seq;
	self->dewir_start  = other->dewir_start;
	self->dewir_end    = other->dewir_end;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
withintrange_deepcopy_with_size_fix(DefaultEnumeration_WithIntRange *__restrict self,
                                    DefaultEnumeration_WithIntRange *__restrict other) {
	size_t realsize;
	if unlikely(withintrange_deepcopy(self, other))
		goto err;
	realsize = DeeObject_Size(self->dewir_seq);
	if unlikely(realsize == (size_t)-1)
		goto err_seq;
	if (self->dewir_end > realsize)
		self->dewir_end = realsize;
	if (self->dewir_start > self->dewir_end)
		self->dewir_start = self->dewir_end;
	return 0;
err_seq:
	Dee_Decref(self->dewir_seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
withrange_deepcopy(DefaultEnumeration_WithRange *__restrict self,
                   DefaultEnumeration_WithRange *__restrict other) {
	self->dewr_seq = DeeObject_DeepCopy(other->dewr_seq);
	if unlikely(!self->dewr_seq)
		goto err;
	self->dewr_start = DeeObject_DeepCopy(other->dewr_start);
	if unlikely(!self->dewr_start)
		goto err_seq;
	self->dewr_end = DeeObject_DeepCopy(other->dewr_end);
	if unlikely(!self->dewr_end)
		goto err_seq_start;
	self->dewr_tp_seq = other->dewr_tp_seq;
	return 0;
err_seq_start:
	Dee_Decref(self->dewr_start);
err_seq:
	Dee_Decref(self->dewr_seq);
err:
	return -1;
}


#define withintrange_fini fullrange_fini

#define de_wsagiif_fini   fullrange_fini
#define de_wsatgii_fini   fullrange_fini
#define de_wsagii_fini    fullrange_fini
#define de_wsoagi_fini    fullrange_fini
#define de_wsagiifaf_fini withintrange_fini
#define de_wsatgiiaf_fini withintrange_fini
#define de_wsagiiaf_fini  withintrange_fini
#define de_wsoagiaf_fini  withrange_fini
#define de_wgii_fini      fullrange_fini
#define de_wgiiaf_fini    withintrange_fini
#define de_wgiaf_fini     withrange_fini
#define de_wikagi_fini    fullrange_fini
#define de_wikagiaf_fini  withrange_fini
#define de_wikatgi_fini   fullrange_fini
#define de_wikatgiaf_fini withrange_fini
#define de_wiac_fini      fullrange_fini
#define de_wiacaf_fini    withintrange_fini
#define de_wiauaf_fini    withintrange_fini
#define de_we_fini        fullrange_fini
#define de_wei_fini       withintrange_fini
#define de_weaf_fini      withrange_fini

PRIVATE NONNULL((1)) void DCALL
fullrange_fini(DefaultEnumeration_FullRange *__restrict self) {
	Dee_Decref(self->defr_seq);
}

PRIVATE NONNULL((1)) void DCALL
withrange_fini(DefaultEnumeration_WithRange *__restrict self) {
	Dee_Decref(self->dewr_seq);
	Dee_Decref(self->dewr_start);
	Dee_Decref(self->dewr_end);
}

#define withintrange_visit fullrange_visit

#define de_wsagiif_visit   fullrange_visit
#define de_wsatgii_visit   fullrange_visit
#define de_wsagii_visit    fullrange_visit
#define de_wsoagi_visit    fullrange_visit
#define de_wsagiifaf_visit withintrange_visit
#define de_wsatgiiaf_visit withintrange_visit
#define de_wsagiiaf_visit  withintrange_visit
#define de_wsoagiaf_visit  withrange_visit
#define de_wgii_visit      fullrange_visit
#define de_wgiiaf_visit    withintrange_visit
#define de_wgiaf_visit     withrange_visit
#define de_wikagi_visit    fullrange_visit
#define de_wikagiaf_visit  withrange_visit
#define de_wikatgi_visit   fullrange_visit
#define de_wikatgiaf_visit withrange_visit
#define de_wiac_visit      fullrange_visit
#define de_wiacaf_visit    withintrange_visit
#define de_wiauaf_visit    withintrange_visit
#define de_we_visit        fullrange_visit
#define de_wei_visit       withintrange_visit
#define de_weaf_visit      withrange_visit

PRIVATE NONNULL((1, 2)) void DCALL
fullrange_visit(DefaultEnumeration_FullRange *__restrict self,
                dvisit_t proc, void *arg) {
	Dee_Visit(self->defr_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
withrange_visit(DefaultEnumeration_WithRange *__restrict self,
                dvisit_t proc, void *arg) {
	Dee_Visit(self->dewr_seq);
	Dee_Visit(self->dewr_start);
	Dee_Visit(self->dewr_end);
}



#define de_wsagiif_contains   fullrange_contains
#define de_wsatgii_contains   fullrange_contains
#define de_wsagii_contains    fullrange_contains
#define de_wsoagi_contains    fullrange_contains
#define de_wsagiifaf_contains withintrange_contains
#define de_wsatgiiaf_contains withintrange_contains
#define de_wsagiiaf_contains  withintrange_contains
#define de_wsoagiaf_contains  withrange_contains
#define de_wgii_contains      fullrange_contains
#define de_wgiiaf_contains    withintrange_contains
#define de_wgiaf_contains     withrange_contains
#define de_wikagi_contains    fullrange_contains
#define de_wikagiaf_contains  withrange_contains
#define de_wikatgi_contains   fullrange_contains
#define de_wikatgiaf_contains withrange_contains
#define de_wiac_contains      fullrange_contains
#define de_wiacaf_contains    withintrange_contains
#define de_wiauaf_contains    withintrange_contains
#define de_we_contains        fullrange_contains
#define de_wei_contains       withintrange_contains
#define de_weaf_contains      withrange_contains

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
fullrange_contains(DefaultEnumeration_FullRange *self, DeeObject *item) {
	int temp;
	DREF DeeObject *real_value;
	DREF DeeObject *key_and_value[2];
	if unlikely(DeeObject_Unpack(item, 2, key_and_value))
		goto err_tryhandle;
	real_value = DeeObject_TryGetItem(self->defr_seq, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if (!ITER_ISOK(real_value)) {
		Dee_Decref(key_and_value[1]);
		if unlikely(!real_value)
			goto err_tryhandle;
		goto nope;
	}
	temp = DeeObject_TryCompareEq(key_and_value[1], real_value);
	Dee_Decref(real_value);
	Dee_Decref(key_and_value[1]);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return_bool(temp == 0);
err_tryhandle:
	if (DeeError_Catch(&DeeError_ValueError) ||
	    DeeError_Catch(&DeeError_TypeError) ||
	    DeeError_Catch(&DeeError_NotImplemented))
		goto nope;
err:
	return NULL;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
withintrange_contains(DefaultEnumeration_WithIntRange *self, DeeObject *item) {
	int temp;
	DREF DeeObject *real_value;
	DREF DeeObject *key_and_value[2];
	if unlikely(DeeObject_Unpack(item, 2, key_and_value))
		goto err_tryhandle;
	/* if (!(self->dewr_start <= key_and_value[0]) ||
	 *     !(self->dewr_end > key_and_value[0]))
	 *     goto nope; */
	temp = DeeInt_Size_Compare(self->dewir_start, key_and_value[0]);
	if (temp == Dee_COMPARE_ERR)
		goto err_key_and_value;
	if (temp > 0)
		goto nope_key_and_value;
	temp = DeeInt_Size_Compare(self->dewir_end, key_and_value[0]);
	if (temp == Dee_COMPARE_ERR)
		goto err_key_and_value;
	if (temp <= 0)
		goto nope_key_and_value;
	real_value = DeeObject_TryGetItem(self->dewir_seq, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if (!ITER_ISOK(real_value)) {
		Dee_Decref(key_and_value[1]);
		if unlikely(!real_value)
			goto err_tryhandle;
		goto nope;
	}
	temp = DeeObject_TryCompareEq(key_and_value[1], real_value);
	Dee_Decref(real_value);
	Dee_Decref(key_and_value[1]);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return_bool(temp == 0);
err_tryhandle:
	if (DeeError_Catch(&DeeError_ValueError) ||
	    DeeError_Catch(&DeeError_TypeError) ||
	    DeeError_Catch(&DeeError_NotImplemented))
		goto nope;
err:
	return NULL;
err_key_and_value:
	Dee_Decrefv(key_and_value, 2);
	goto err;
nope_key_and_value:
	Dee_Decrefv(key_and_value, 2);
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
withrange_contains(DefaultEnumeration_WithRange *self, DeeObject *item) {
	int temp;
	DREF DeeObject *real_value;
	DREF DeeObject *key_and_value[2];
	if unlikely(DeeObject_Unpack(item, 2, key_and_value))
		goto err_tryhandle;
	/* if (!(self->dewr_start <= key_and_value[0]) ||
	 *     !(self->dewr_end > key_and_value[0]))
	 *     goto nope; */
	temp = DeeObject_CmpLeAsBool(self->dewr_start, key_and_value[0]);
	if (temp <= 0)
		goto err_key_and_value_or_nope;
	temp = DeeObject_CmpGrAsBool(self->dewr_end, key_and_value[0]);
	if (temp <= 0)
		goto err_key_and_value_or_nope;
	real_value = DeeObject_TryGetItem(self->dewr_seq, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if (!ITER_ISOK(real_value)) {
		Dee_Decref(key_and_value[1]);
		if unlikely(!real_value)
			goto err_tryhandle;
		goto nope;
	}
	temp = DeeObject_TryCompareEq(key_and_value[1], real_value);
	Dee_Decref(real_value);
	Dee_Decref(key_and_value[1]);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return_bool(temp == 0);
err_tryhandle:
	if (DeeError_Catch(&DeeError_ValueError) ||
	    DeeError_Catch(&DeeError_TypeError) ||
	    DeeError_Catch(&DeeError_NotImplemented))
		goto nope;
err:
	return NULL;
err_key_and_value_or_nope:
	Dee_Decrefv(key_and_value, 2);
	if unlikely(temp < 0)
		goto err;
nope:
	return_false;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsagiif_init(DefaultEnumeration_FullRange *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithSizeAndGetItemIndexFast", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index_fast) &&
	    (!DeeType_InheritGetItem(seqtyp) || !seqtyp->tp_seq->tp_getitem_index_fast))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsatgii_init(DefaultEnumeration_FullRange *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithSizeAndTryGetItemIndex", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsagii_init(DefaultEnumeration_FullRange *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithSizeAndGetItemIndex", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsoagi_init(DefaultEnumeration_FullRange *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithSizeObAndGetItem", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_sizeob && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsagiifaf_init(DefaultEnumeration_WithIntRange *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	size_t realsize;
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqEnumWithSizeAndGetItemIndexFastAndFilter",
	                  &self->dewir_seq, &self->dewir_start, &self->dewir_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewir_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index_fast) &&
	    (!DeeType_InheritGetItem(seqtyp) || !seqtyp->tp_seq->tp_getitem_index_fast))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	realsize = (*seqtyp->tp_seq->tp_size)(self->dewir_seq);
	if unlikely(realsize == (size_t)-1)
		goto err;
	if (self->dewir_end > realsize)
		self->dewir_end = realsize;
	if (self->dewir_start > self->dewir_end)
		self->dewir_start = self->dewir_end;
	self->dewir_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewir_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsatgiiaf_init(DefaultEnumeration_WithIntRange *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqEnumWithSizeAndTryGetItemIndexAndFilter",
	                  &self->dewir_seq, &self->dewir_start, &self->dewir_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewir_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->dewir_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewir_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsagiiaf_init(DefaultEnumeration_WithIntRange *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqEnumWithSizeAndGetItemIndexAndFilter",
	                  &self->dewir_seq, &self->dewir_start, &self->dewir_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewir_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->dewir_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewir_seq);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wsoagiaf_init(DefaultEnumeration_WithRange *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_SeqEnumWithSizeObAndGetItemAndFilter",
	                  &self->dewr_seq, &self->dewr_start, &self->dewr_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if (!seqtyp->tp_seq->tp_size && !DeeType_InheritSize(seqtyp))
		goto err_no_size;
	self->dewr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewr_seq);
	Dee_Incref(self->dewr_start);
	Dee_Incref(self->dewr_end);
	return 0;
err_no_size:
	return err_unimplemented_operator(seqtyp, OPERATOR_SIZE);
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wgii_init(DefaultEnumeration_FullRange *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithGetItemIndex", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wgiiaf_init(DefaultEnumeration_WithIntRange *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqEnumWithGetItemIndexAndFilter",
	                  &self->dewir_seq, &self->dewir_start, &self->dewir_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewir_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->dewir_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewir_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wgiaf_init(DefaultEnumeration_WithRange *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_SeqEnumWithGetItemAndFilter",
	                  &self->dewr_seq, &self->dewr_start, &self->dewr_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->dewr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewr_seq);
	Dee_Incref(self->dewr_start);
	Dee_Incref(self->dewr_end);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wikagi_init(DefaultEnumeration_FullRange *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithIterKeysAndGetItem", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iterkeys) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_iterkeys))
		goto err_no_enum;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wikagiaf_init(DefaultEnumeration_WithRange *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_SeqEnumWithIterKeysAndGetItemAndFilter",
	                  &self->dewr_seq, &self->dewr_start, &self->dewr_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iterkeys) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_iterkeys))
		goto err_no_enum;
	self->dewr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewr_seq);
	Dee_Incref(self->dewr_start);
	Dee_Incref(self->dewr_end);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wikatgi_init(DefaultEnumeration_FullRange *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithIterKeysAndTryGetItem", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iterkeys) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_iterkeys))
		goto err_no_enum;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wikatgiaf_init(DefaultEnumeration_WithRange *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_SeqEnumWithIterKeysAndTryGetItemAndFilter",
	                  &self->dewr_seq, &self->dewr_start, &self->dewr_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iterkeys) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_iterkeys))
		goto err_no_enum;
	self->dewr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewr_seq);
	Dee_Incref(self->dewr_start);
	Dee_Incref(self->dewr_end);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wiac_init(DefaultEnumeration_FullRange *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithIterAndCounter", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iter) &&
	    !DeeType_InheritIter(seqtyp))
		goto err_no_iter;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_iter:
	return err_unimplemented_operator(seqtyp, OPERATOR_ITER);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wiacaf_init(DefaultEnumeration_WithIntRange *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqEnumWithIterAndCounterAndFilter",
	                  &self->dewir_seq, &self->dewir_start, &self->dewir_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewir_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iter) &&
	    !DeeType_InheritIter(seqtyp))
		goto err_no_iter;
	self->dewir_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewir_seq);
	return 0;
err_no_iter:
	return err_unimplemented_operator(seqtyp, OPERATOR_ITER);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wiauaf_init(DefaultEnumeration_WithRange *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_SeqEnumWithIterAndUnpackAndFilter",
	                  &self->dewr_seq, &self->dewr_start, &self->dewr_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_iter) &&
	    !DeeType_InheritIter(seqtyp))
		goto err_no_iter;
	self->dewr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewr_seq);
	Dee_Incref(self->dewr_start);
	Dee_Incref(self->dewr_end);
	return 0;
err_no_iter:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_we_init(DefaultEnumeration_FullRange *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o:_SeqEnumWithEnumerate", &self->defr_seq))
		goto err;
	seqtyp = Dee_TYPE(self->defr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_enumerate) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_enumerate))
		goto err_no_enum;
	self->defr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->defr_seq);
	return 0;
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_wei_init(DefaultEnumeration_WithIntRange *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqEnumWithEnumerateIndex",
	                  &self->dewir_seq, &self->dewir_start, &self->dewir_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewir_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_enumerate_index) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_enumerate_index))
		goto err_no_enum;
	self->dewir_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewir_seq);
	return 0;
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_weaf_init(DefaultEnumeration_WithRange *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_SeqEnumWithEnumerateAndFilter",
	                  &self->dewr_seq, &self->dewr_start, &self->dewr_end))
		goto err;
	seqtyp = Dee_TYPE(self->dewr_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_enumerate) &&
	    (!DeeType_InheritIter(seqtyp) || !seqtyp->tp_seq->tp_enumerate))
		goto err_no_enum;
	self->dewr_tp_seq = seqtyp->tp_seq;
	Dee_Incref(self->dewr_seq);
	Dee_Incref(self->dewr_start);
	Dee_Incref(self->dewr_end);
	return 0;
err_no_enum:
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate keys of non-sequence type `%r'",
	                       seqtyp);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wsagiif_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_end = (*self->defr_tp_seq->tp_size)(self->defr_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	result->disgi_index = 0;
	result->disgi_tp_getitem_index = self->defr_tp_seq->tp_getitem_index_fast;
	Dee_Incref(self->defr_seq);
	result->disgi_seq = self->defr_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wsatgii_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_end = (*self->defr_tp_seq->tp_size)(self->defr_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	result->disgi_index = 0;
	result->disgi_tp_getitem_index = self->defr_tp_seq->tp_trygetitem_index;
	Dee_Incref(self->defr_seq);
	result->disgi_seq = self->defr_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wsagii_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_end = (*self->defr_tp_seq->tp_size)(self->defr_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	result->disgi_index = 0;
	result->disgi_tp_getitem_index = self->defr_tp_seq->tp_getitem_index;
	Dee_Incref(self->defr_seq);
	result->disgi_seq = self->defr_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
de_wsoagi_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	result->disg_end = (*self->defr_tp_seq->tp_sizeob)(self->defr_seq);
	if unlikely(!result->disg_end)
		goto err_r;
	result->disg_index = DeeObject_NewDefault(Dee_TYPE(result->disg_end));
	if unlikely(!result->disg_index)
		goto err_r_end;
	result->disg_tp_getitem = self->defr_tp_seq->tp_getitem;
	Dee_Incref(self->defr_seq);
	result->disg_seq = self->defr_seq;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItemPair_Type);
	return result;
err_r_end:
	Dee_Decref(result->disg_end);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wsagiifaf_iter(DefaultEnumeration_WithIntRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_end = (*self->dewir_tp_seq->tp_size)(self->dewir_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	result->disgi_index = self->dewir_start;
	if (result->disgi_end > self->dewir_end)
		result->disgi_end = self->dewir_end;
	result->disgi_tp_getitem_index = self->dewir_tp_seq->tp_getitem_index_fast;
	Dee_Incref(self->dewir_seq);
	result->disgi_seq = self->dewir_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wsatgiiaf_iter(DefaultEnumeration_WithIntRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_end = (*self->dewir_tp_seq->tp_size)(self->dewir_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	result->disgi_index = self->dewir_start;
	if (result->disgi_end > self->dewir_end)
		result->disgi_end = self->dewir_end;
	result->disgi_tp_getitem_index = self->dewir_tp_seq->tp_trygetitem_index;
	Dee_Incref(self->dewir_seq);
	result->disgi_seq = self->dewir_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wsagiiaf_iter(DefaultEnumeration_WithIntRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_end = (*self->dewir_tp_seq->tp_size)(self->dewir_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	result->disgi_index = self->dewir_start;
	if (result->disgi_end > self->dewir_end)
		result->disgi_end = self->dewir_end;
	result->disgi_tp_getitem_index = self->dewir_tp_seq->tp_getitem_index;
	Dee_Incref(self->dewir_seq);
	result->disgi_seq = self->dewir_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
de_wsoagiaf_iter(DefaultEnumeration_WithRange *__restrict self) {
	int temp;
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	result->disg_end = (*self->dewr_tp_seq->tp_sizeob)(self->dewr_seq);
	if unlikely(!result->disg_end)
		goto err_r;
	temp = DeeObject_CmpLoAsBool(self->dewr_end, result->disg_end);
	if (temp != 0) {
		if unlikely(temp < 0)
			goto err_r_end;
		Dee_Incref(self->dewr_end);
		Dee_Decref(result->disg_end);
		result->disg_end = self->dewr_end;
	}
	Dee_Incref(self->dewr_start);
	result->disg_index = self->dewr_start;
	result->disg_tp_getitem = self->dewr_tp_seq->tp_getitem;
	Dee_Incref(self->dewr_seq);
	result->disg_seq = self->dewr_seq;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItemPair_Type);
	return result;
err_r_end:
	Dee_Decref(result->disg_end);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithGetItemIndex *DCALL
de_wgii_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->digi_index = 0;
	result->digi_tp_getitem_index = self->defr_tp_seq->tp_getitem_index;
	Dee_Incref(self->defr_seq);
	result->digi_seq = self->defr_seq;
	DeeObject_Init(result, &DefaultIterator_WithGetItemIndexPair_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_wgiiaf_iter(DefaultEnumeration_WithIntRange *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_index = self->dewir_start;
	result->disgi_end   = self->dewir_end;
	result->disgi_tp_getitem_index = self->dewir_tp_seq->tp_getitem_index;
	Dee_Incref(self->dewir_seq);
	result->disgi_seq = self->dewir_seq;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexPair_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
de_wgiaf_iter(DefaultEnumeration_WithRange *__restrict self) {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dewr_seq);
	result->disg_seq = self->dewr_seq;
	result->disg_tp_getitem = self->dewr_tp_seq->tp_getitem;
	Dee_Incref(self->dewr_start);
	result->disg_index = self->dewr_start;
	Dee_Incref(self->dewr_end);
	result->disg_end = self->dewr_end;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItemPair_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithIterKeysAndGetItem *DCALL
de_wikagi_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = (*self->defr_tp_seq->tp_iterkeys)(self->defr_seq);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	result->diikgi_tp_next = Dee_TYPE(result->diikgi_iter)->tp_iter_next;
	if (!result->diikgi_tp_next) {
		if (!DeeType_InheritIterNext(Dee_TYPE(result->diikgi_iter)))
			goto err_r_no_next;
		result->diikgi_tp_next = Dee_TYPE(result->diikgi_iter)->tp_iter_next;
	}
	result->diikgi_tp_getitem = self->defr_tp_seq->tp_getitem;
	Dee_Incref(self->defr_seq);
	result->diikgi_seq = self->defr_seq;
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndGetItemMap_Type);
	return result;
err_r_no_next:
	err_unimplemented_operator(Dee_TYPE(result->diikgi_iter), OPERATOR_ITERNEXT);
	Dee_Decref(result->diikgi_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_wikagiaf_iter(DefaultEnumeration_WithRange *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithIterKeysAndGetItem *DCALL
de_wikatgi_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = (*self->defr_tp_seq->tp_iterkeys)(self->defr_seq);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	result->diikgi_tp_next = Dee_TYPE(result->diikgi_iter)->tp_iter_next;
	if (!result->diikgi_tp_next) {
		if (!DeeType_InheritIterNext(Dee_TYPE(result->diikgi_iter)))
			goto err_r_no_next;
		result->diikgi_tp_next = Dee_TYPE(result->diikgi_iter)->tp_iter_next;
	}
	result->diikgi_tp_getitem = self->defr_tp_seq->tp_trygetitem;
	Dee_Incref(self->defr_seq);
	result->diikgi_seq = self->defr_seq;
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type);
	return result;
err_r_no_next:
	err_unimplemented_operator(Dee_TYPE(result->diikgi_iter), OPERATOR_ITERNEXT);
	Dee_Decref(result->diikgi_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_wikatgiaf_iter(DefaultEnumeration_WithRange *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithNextAndCounter *DCALL
de_wiac_iter(DefaultEnumeration_FullRange *__restrict self) {
	DREF DefaultIterator_WithNextAndCounter *result;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndCounter);
	if unlikely(!result)
		goto err;
	result->dinc_iter = (*self->defr_tp_seq->tp_iter)(self->defr_seq);
	if unlikely(!result->dinc_iter)
		goto err_r;
	result->dinc_tp_next = Dee_TYPE(result->dinc_iter)->tp_iter_next;
	if unlikely(!result->dinc_tp_next) {
		if (!DeeType_InheritIterNext(Dee_TYPE(result->dinc_iter)))
			goto err_r_iter_no_next;
		result->dinc_tp_next = Dee_TYPE(result->dinc_iter)->tp_iter_next;
	}
	result->dinc_counter = 0;
	DeeObject_Init(result, &DefaultIterator_WithNextAndCounterPair_Type);
	return result;
err_r_iter_no_next:
	err_unimplemented_operator(Dee_TYPE(result->dinc_iter), OPERATOR_ITERNEXT);
	Dee_Decref(result->dinc_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithNextAndCounterAndLimit *DCALL
de_wiacaf_iter(DefaultEnumeration_WithIntRange *__restrict self) {
	DREF DefaultIterator_WithNextAndCounterAndLimit *result;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndCounterAndLimit);
	if unlikely(!result)
		goto err;
	result->dincl_iter = (*self->dewir_tp_seq->tp_iter)(self->dewir_seq);
	if unlikely(!result->dincl_iter)
		goto err_r;
	result->dincl_tp_next = Dee_TYPE(result->dincl_iter)->tp_iter_next;
	if unlikely(!result->dincl_tp_next) {
		if (!DeeType_InheritIterNext(Dee_TYPE(result->dincl_iter)))
			goto err_r_iter_no_next;
		result->dincl_tp_next = Dee_TYPE(result->dincl_iter)->tp_iter_next;
	}
	if (DeeObject_IterAdvance(result->dincl_iter, self->dewir_start) == (size_t)-1)
		goto err_r_iter;
	result->dincl_counter = self->dewir_start;
	result->dincl_limit   = self->dewir_end;
	DeeObject_Init(result, &DefaultIterator_WithNextAndCounterAndLimitPair_Type);
	return result;
err_r_iter_no_next:
	err_unimplemented_operator(Dee_TYPE(result->dincl_iter), OPERATOR_ITERNEXT);
err_r_iter:
	Dee_Decref(result->dincl_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithNextAndUnpackFilter *DCALL
de_wiauaf_iter(DefaultEnumeration_WithRange *__restrict self) {
	DREF DefaultIterator_WithNextAndUnpackFilter *result;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndUnpackFilter);
	if unlikely(!result)
		goto err;
	result->dinuf_iter = (*self->dewr_tp_seq->tp_iter)(self->dewr_seq);
	if unlikely(!result->dinuf_iter)
		goto err_r;
	result->dinuf_tp_next = Dee_TYPE(result->dinuf_iter)->tp_iter_next;
	if unlikely(!result->dinuf_tp_next) {
		if (!DeeType_InheritIterNext(Dee_TYPE(result->dinuf_iter)))
			goto err_r_iter_no_next;
		result->dinuf_tp_next = Dee_TYPE(result->dinuf_iter)->tp_iter_next;
	}
	Dee_Incref(self->dewr_start);
	result->dinuf_start = self->dewr_start;
	Dee_Incref(self->dewr_end);
	result->dinuf_end = self->dewr_end;
	DeeObject_Init(result, &DefaultIterator_WithNextAndUnpackFilter_Type);
	return result;
err_r_iter_no_next:
	err_unimplemented_operator(Dee_TYPE(result->dinuf_iter), OPERATOR_ITERNEXT);
/*err_r_iter:*/
	Dee_Decref(result->dinuf_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_we_iter(DefaultEnumeration_FullRange *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_wei_iter(DefaultEnumeration_WithIntRange *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_weaf_iter(DefaultEnumeration_WithRange *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}









struct foreach_with_enumerate_index_data {
	Dee_foreach_pair_t fewei_cb;  /* [1..1] Inner callback */
	void              *fewei_arg; /* [?..?] Argument for `fewei_cb' */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
foreach_with_enumerate_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct foreach_with_enumerate_index_data *data;
	DREF DeeObject *indexob;
	data = (struct foreach_with_enumerate_index_data *)arg;
	if (!value)
		return 0;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*data->fewei_cb)(data->fewei_arg, indexob, value);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}

struct foreach_with_enumerate_and_filter_data {
	Dee_foreach_pair_t feweaf_cb;    /* [1..1] Inner callback */
	void              *feweaf_arg;   /* [?..?] Argument for `feweaf_cb' */
	DeeObject         *feweaf_start; /* [1..1] Start index */
	DeeObject         *feweaf_end;   /* [1..1] End index */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
foreach_with_enumerate_and_filter_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	int temp;
	struct foreach_with_enumerate_and_filter_data *data;
	data = (struct foreach_with_enumerate_and_filter_data *)arg;
	if (!value)
		return 0;
	/* feweaf_start <= index && feweaf_end > index */
	temp = DeeObject_CmpLeAsBool(data->feweaf_start, index);
	if unlikely(temp <= 0)
		goto err_or_filtered;
	temp = DeeObject_CmpGrAsBool(data->feweaf_end, index);
	if unlikely(temp <= 0)
		goto err_or_filtered;
	return (*data->feweaf_cb)(data->feweaf_arg, index, value);
err_or_filtered:
	if (temp == 0)
		return 0;
/*err:*/
	return -1;
}





PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsagiif_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                        Dee_foreach_pair_t cb, void *arg) {
	size_t i, size;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_size)(self->defr_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		value = (*tp_seq->tp_getitem_index_fast)(self->defr_seq, i);
		if (!value)
			goto next_index;
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsatgii_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                        Dee_foreach_pair_t cb, void *arg) {
	size_t i, size;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_size)(self->defr_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		value = (*tp_seq->tp_trygetitem_index)(self->defr_seq, i);
		if (!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				goto next_index;
			goto err;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsagii_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                       Dee_foreach_pair_t cb, void *arg) {
	size_t i, size;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_size)(self->defr_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		value = (*tp_seq->tp_getitem_index)(self->defr_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsoagi_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                       Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *i, *size, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_sizeob)(self->defr_seq);
	if unlikely(!size)
		goto err;
	i = DeeObject_NewDefault(Dee_TYPE(size));
	if unlikely(!i)
		goto err_size;
	for (;;) {
		int is_not_done;
		is_not_done = DeeObject_CmpLoAsBool(i, size);
		if (is_not_done <= 0) {
			if unlikely(is_not_done < 0)
				goto err_size_i;
			break;
		}
		value = (*tp_seq->tp_getitem)(self->defr_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err_size_i;
		}
		temp = (*cb)(arg, i, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_size_i_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err_size_i;
		if (DeeObject_Inc(&i))
			goto err_size_i;
	}
	Dee_Decref(i);
	Dee_Decref(size);
	return result;
err_size_i_temp:
	Dee_Decref(i);
	Dee_Decref(size);
	return temp;
err_size_i:
	Dee_Decref(i);
err_size:
	Dee_Decref(size);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsagiifaf_foreach_pair(DefaultEnumeration_WithIntRange *__restrict self,
                          Dee_foreach_pair_t cb, void *arg) {
	size_t i, size;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewir_tp_seq;

	/* If object supports tp_enumerate_index, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewir_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate_index) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewir_seq)) && tp_seq->tp_enumerate_index)) {
		struct foreach_with_enumerate_index_data data;
		data.fewei_cb  = cb;
		data.fewei_arg = arg;
		return (*tp_seq->tp_enumerate_index)(self->dewir_seq, &foreach_with_enumerate_index_cb, &data,
		                                     self->dewir_start, self->dewir_end);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_size)(self->dewir_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > self->dewir_end)
		size = self->dewir_end;
	for (i = self->dewir_start; i < size; ++i) {
		value = (*tp_seq->tp_getitem_index_fast)(self->dewir_seq, i);
		if (!value)
			goto next_index;
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsatgiiaf_foreach_pair(DefaultEnumeration_WithIntRange *__restrict self,
                          Dee_foreach_pair_t cb, void *arg) {
	size_t i, size;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewir_tp_seq;

	/* If object supports tp_enumerate_index, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewir_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate_index) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewir_seq)) && tp_seq->tp_enumerate_index)) {
		struct foreach_with_enumerate_index_data data;
		data.fewei_cb  = cb;
		data.fewei_arg = arg;
		return (*tp_seq->tp_enumerate_index)(self->dewir_seq, &foreach_with_enumerate_index_cb, &data,
		                                     self->dewir_start, self->dewir_end);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_size)(self->dewir_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > self->dewir_end)
		size = self->dewir_end;
	for (i = self->dewir_start; i < size; ++i) {
		value = (*tp_seq->tp_trygetitem_index)(self->dewir_seq, i);
		if (!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				goto next_index;
			goto err;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsagiiaf_foreach_pair(DefaultEnumeration_WithIntRange *__restrict self,
                         Dee_foreach_pair_t cb, void *arg) {
	size_t i, size;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewir_tp_seq;

	/* If object supports tp_enumerate_index, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewir_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate_index) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewir_seq)) && tp_seq->tp_enumerate_index)) {
		struct foreach_with_enumerate_index_data data;
		data.fewei_cb  = cb;
		data.fewei_arg = arg;
		return (*tp_seq->tp_enumerate_index)(self->dewir_seq, &foreach_with_enumerate_index_cb, &data,
		                                     self->dewir_start, self->dewir_end);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_size)(self->dewir_seq);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > self->dewir_end)
		size = self->dewir_end;
	for (i = self->dewir_start; i < size; ++i) {
		value = (*tp_seq->tp_getitem_index)(self->dewir_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wsoagiaf_foreach_pair(DefaultEnumeration_WithRange *__restrict self,
                         Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *i, *size, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewr_seq)) && tp_seq->tp_enumerate)) {
		struct foreach_with_enumerate_and_filter_data data;
		data.feweaf_cb    = cb;
		data.feweaf_arg   = arg;
		data.feweaf_start = self->dewr_start;
		data.feweaf_end   = self->dewr_end;
		return (*tp_seq->tp_enumerate)(self->dewr_seq, &foreach_with_enumerate_and_filter_cb, &data);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	size = (*tp_seq->tp_sizeob)(self->dewr_seq);
	if unlikely(!size)
		goto err;
	{
		int must_clamp_size;
		must_clamp_size = DeeObject_CmpLoAsBool(self->dewr_end, size);
		if (must_clamp_size != 0) {
			if unlikely(must_clamp_size < 0)
				goto err_size;
			Dee_Incref(self->dewr_end);
			Dee_Decref(size);
			size = self->dewr_end;
		}
	}
	i = self->dewr_start;
	Dee_Incref(i);
	for (;;) {
		int is_not_done;
		is_not_done = DeeObject_CmpLoAsBool(i, size);
		if (is_not_done <= 0) {
			if unlikely(is_not_done < 0)
				goto err_size_i;
			break;
		}
		value = (*tp_seq->tp_getitem)(self->dewr_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err_size_i;
		}
		temp = (*cb)(arg, i, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_size_i_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err_size_i;
		if (DeeObject_Inc(&i))
			goto err_size_i;
	}
	Dee_Decref(i);
	Dee_Decref(size);
	return result;
err_size_i_temp:
	Dee_Decref(i);
	Dee_Decref(size);
	return temp;
err_size_i:
	Dee_Decref(i);
err_size:
	Dee_Decref(size);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wgii_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                     Dee_foreach_pair_t cb, void *arg) {
	size_t i;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	for (i = 0;; ++i) {
		value = (*tp_seq->tp_getitem_index)(self->defr_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wgiiaf_foreach_pair(DefaultEnumeration_WithIntRange *__restrict self,
                       Dee_foreach_pair_t cb, void *arg) {
	size_t i;
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewir_tp_seq;

	/* If object supports tp_enumerate_index, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewir_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate_index) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewir_seq)) && tp_seq->tp_enumerate_index)) {
		struct foreach_with_enumerate_index_data data;
		data.fewei_cb  = cb;
		data.fewei_arg = arg;
		return (*tp_seq->tp_enumerate_index)(self->dewir_seq, &foreach_with_enumerate_index_cb, &data,
		                                     self->dewir_start, self->dewir_end);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	for (i = self->dewir_start; i < self->dewir_end; ++i) {
		value = (*tp_seq->tp_getitem_index)(self->dewir_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wgiaf_foreach_pair(DefaultEnumeration_WithRange *__restrict self,
                      Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *i, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewr_seq)) && tp_seq->tp_enumerate)) {
		struct foreach_with_enumerate_and_filter_data data;
		data.feweaf_cb    = cb;
		data.feweaf_arg   = arg;
		data.feweaf_start = self->dewr_start;
		data.feweaf_end   = self->dewr_end;
		return (*tp_seq->tp_enumerate)(self->dewr_seq, &foreach_with_enumerate_and_filter_cb, &data);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	result = 0;
	i = self->dewr_start;
	Dee_Incref(i);
	for (;;) {
		int is_not_done;
		is_not_done = DeeObject_CmpLoAsBool(i, self->dewr_end);
		if (is_not_done <= 0) {
			if unlikely(is_not_done < 0)
				goto err_i;
			break;
		}
		value = (*tp_seq->tp_getitem)(self->dewr_seq, i);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err_i;
		}
		temp = (*cb)(arg, i, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_i_temp;
		result += temp;
next_index:
		if (DeeThread_CheckInterrupt())
			goto err_i;
		if (DeeObject_Inc(&i))
			goto err_i;
	}
	Dee_Decref(i);
	return result;
err_i_temp:
	Dee_Decref(i);
	return temp;
err_i:
	Dee_Decref(i);
/*err:*/
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wikagi_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                       Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *key, *iterkeys, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	iterkeys = (*tp_seq->tp_iterkeys)(self->defr_seq);
	if unlikely(!iterkeys)
		goto err;
	result = 0;
	for (;;) {
		key = DeeObject_IterNext(iterkeys);
		if (!ITER_ISOK(key)) {
			if (key == ITER_DONE)
				break;
			goto err_iterkeys;
		}
		value = (*tp_seq->tp_getitem)(self->defr_seq, key);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_Decref(key);
				break;
			}
			goto err_iterkeys_key;
		}
		temp = (*cb)(arg, key, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_iterkeys_key_temp;
		result += temp;
next_index:
		Dee_Decref(key);
		if (DeeThread_CheckInterrupt())
			goto err_iterkeys;
	}
	Dee_Decref(iterkeys);
	return result;
err_iterkeys_key_temp:
	Dee_Decref(key);
	Dee_Decref(iterkeys);
	return temp;
err_iterkeys_key:
	Dee_Decref(key);
err_iterkeys:
	Dee_Decref(iterkeys);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wikagiaf_foreach_pair(DefaultEnumeration_WithRange *__restrict self,
                         Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *key, *iterkeys, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewr_seq)) && tp_seq->tp_enumerate)) {
		struct foreach_with_enumerate_and_filter_data data;
		data.feweaf_cb    = cb;
		data.feweaf_arg   = arg;
		data.feweaf_start = self->dewr_start;
		data.feweaf_end   = self->dewr_end;
		return (*tp_seq->tp_enumerate)(self->dewr_seq, &foreach_with_enumerate_and_filter_cb, &data);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	iterkeys = (*tp_seq->tp_iterkeys)(self->dewr_seq);
	if unlikely(!iterkeys)
		goto err;
	result = 0;
	for (;;) {
		int key_is_ok;
		key = DeeObject_IterNext(iterkeys);
		if (!ITER_ISOK(key)) {
			if (key == ITER_DONE)
				break;
			goto err_iterkeys;
		}
		key_is_ok = DeeObject_CmpLeAsBool(self->dewr_start, key);
		if unlikely(key_is_ok <= 0) {
err_iterkeys_key_or_filtered:
			if (key_is_ok == 0)
				goto next_index;
			goto err_iterkeys_key;
		}
		key_is_ok = DeeObject_CmpGrAsBool(self->dewr_end, key);
		if unlikely(key_is_ok <= 0)
			goto err_iterkeys_key_or_filtered;
		value = (*tp_seq->tp_getitem)(self->dewr_seq, key);
		if unlikely(!value) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				goto next_index;
			if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_Decref(key);
				break;
			}
			goto err_iterkeys_key;
		}
		temp = (*cb)(arg, key, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_iterkeys_key_temp;
		result += temp;
next_index:
		Dee_Decref(key);
		if (DeeThread_CheckInterrupt())
			goto err_iterkeys;
	}
	Dee_Decref(iterkeys);
	return result;
err_iterkeys_key_temp:
	Dee_Decref(key);
	Dee_Decref(iterkeys);
	return temp;
err_iterkeys_key:
	Dee_Decref(key);
err_iterkeys:
	Dee_Decref(iterkeys);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wikatgi_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                        Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *key, *iterkeys, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	iterkeys = (*tp_seq->tp_iterkeys)(self->defr_seq);
	if unlikely(!iterkeys)
		goto err;
	result = 0;
	for (;;) {
		key = DeeObject_IterNext(iterkeys);
		if (!ITER_ISOK(key)) {
			if (key == ITER_DONE)
				break;
			goto err_iterkeys;
		}
		value = (*tp_seq->tp_trygetitem)(self->defr_seq, key);
		if unlikely(!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				goto next_index;
			goto err_iterkeys_key;
		}
		temp = (*cb)(arg, key, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_iterkeys_key_temp;
		result += temp;
next_index:
		Dee_Decref(key);
		if (DeeThread_CheckInterrupt())
			goto err_iterkeys;
	}
	Dee_Decref(iterkeys);
	return result;
err_iterkeys_key_temp:
	Dee_Decref(key);
	Dee_Decref(iterkeys);
	return temp;
err_iterkeys_key:
	Dee_Decref(key);
err_iterkeys:
	Dee_Decref(iterkeys);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wikatgiaf_foreach_pair(DefaultEnumeration_WithRange *__restrict self,
                          Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *key, *iterkeys, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewr_seq)) && tp_seq->tp_enumerate)) {
		struct foreach_with_enumerate_and_filter_data data;
		data.feweaf_cb    = cb;
		data.feweaf_arg   = arg;
		data.feweaf_start = self->dewr_start;
		data.feweaf_end   = self->dewr_end;
		return (*tp_seq->tp_enumerate)(self->dewr_seq, &foreach_with_enumerate_and_filter_cb, &data);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	iterkeys = (*tp_seq->tp_iterkeys)(self->dewr_seq);
	if unlikely(!iterkeys)
		goto err;
	result = 0;
	for (;;) {
		int key_is_ok;
		key = DeeObject_IterNext(iterkeys);
		if (!ITER_ISOK(key)) {
			if (key == ITER_DONE)
				break;
			goto err_iterkeys;
		}
		key_is_ok = DeeObject_CmpLeAsBool(self->dewr_start, key);
		if unlikely(key_is_ok <= 0) {
err_iterkeys_key_or_filtered:
			if (key_is_ok == 0)
				goto next_index;
			goto err_iterkeys_key;
		}
		key_is_ok = DeeObject_CmpGrAsBool(self->dewr_end, key);
		if unlikely(key_is_ok <= 0)
			goto err_iterkeys_key_or_filtered;
		value = (*tp_seq->tp_getitem)(self->dewr_seq, key);
		if unlikely(!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				goto next_index;
			goto err_iterkeys_key;
		}
		temp = (*cb)(arg, key, value);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_iterkeys_key_temp;
		result += temp;
next_index:
		Dee_Decref(key);
		if (DeeThread_CheckInterrupt())
			goto err_iterkeys;
	}
	Dee_Decref(iterkeys);
	return result;
err_iterkeys_key_temp:
	Dee_Decref(key);
	Dee_Decref(iterkeys);
	return temp;
err_iterkeys_key:
	Dee_Decref(key);
err_iterkeys:
	Dee_Decref(iterkeys);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wiac_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                     Dee_foreach_pair_t cb, void *arg) {
	size_t i;
	DREF DeeObject *key, *iter, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->defr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->defr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->defr_seq)) && tp_seq->tp_enumerate))
		return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);

	/* Else, manually enumerate here using the designated method (index < size). */
	iter = (*tp_seq->tp_iter)(self->defr_seq);
	if unlikely(!iter)
		goto err;
	result = 0;
	for (i = 0;; ++i) {
		value = DeeObject_IterNext(iter);
		if (!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				break;
			goto err_iter;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_iter_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_iter_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iter);
	return result;
err_iter_temp:
	Dee_Decref(iter);
	return temp;
err_iter_value:
	Dee_Decref(value);
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wiacaf_foreach_pair(DefaultEnumeration_WithIntRange *__restrict self,
                       Dee_foreach_pair_t cb, void *arg) {
	size_t i;
	DREF DeeObject *key, *iter, *value;
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewir_tp_seq;

	/* If object supports tp_enumerate_index, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewir_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate_index) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewir_seq)) && tp_seq->tp_enumerate_index)) {
		struct foreach_with_enumerate_index_data data;
		data.fewei_cb  = cb;
		data.fewei_arg = arg;
		return (*tp_seq->tp_enumerate_index)(self->dewir_seq, &foreach_with_enumerate_index_cb, &data,
		                                     self->dewir_start, self->dewir_end);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	iter = (*tp_seq->tp_iter)(self->dewir_seq);
	if unlikely(!iter)
		goto err;
	result = 0;
	i = DeeObject_IterAdvance(iter, self->dewir_start);
	if unlikely(i != self->dewir_start) {
		if unlikely(i == (size_t)-1)
			goto err_iter;
		goto done_iter;
	}
	for (; i < self->dewir_end; ++i) {
		value = DeeObject_IterNext(iter);
		if (!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				break;
			goto err_iter;
		}
		key = DeeInt_NewSize(i);
		if unlikely(!key)
			goto err_iter_value;
		temp = (*cb)(arg, key, value);
		Dee_Decref(key);
		Dee_Decref(value);
		if unlikely(temp < 0)
			goto err_iter_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
done_iter:
	Dee_Decref(iter);
	return result;
err_iter_temp:
	Dee_Decref(iter);
	return temp;
err_iter_value:
	Dee_Decref(value);
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wiauaf_foreach_pair(DefaultEnumeration_WithRange *__restrict self,
                       Dee_foreach_pair_t cb, void *arg) {
	DREF DeeObject *iter, *item, *key_and_value[2];
	Dee_ssize_t temp, result;
	struct type_seq *tp_seq = self->dewr_tp_seq;

	/* If object supports tp_enumerate, use that. */
	ASSERT(tp_seq == Dee_TYPE(self->dewr_seq)->tp_seq);
	if likely(likely(tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(Dee_TYPE(self->dewr_seq)) && tp_seq->tp_enumerate)) {
		struct foreach_with_enumerate_and_filter_data data;
		data.feweaf_cb    = cb;
		data.feweaf_arg   = arg;
		data.feweaf_start = self->dewr_start;
		data.feweaf_end   = self->dewr_end;
		return (*tp_seq->tp_enumerate)(self->dewr_seq, &foreach_with_enumerate_and_filter_cb, &data);
	}

	/* Else, manually enumerate here using the designated method (index < size). */
	iter = (*tp_seq->tp_iter)(self->dewr_seq);
	if unlikely(!iter)
		goto err;
	result = 0;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		int key_is_ok;
		if (DeeObject_Unpack(item, 2, key_and_value))
			goto err_iter_item;
		Dee_Decref(item);
		key_is_ok = DeeObject_CmpLeAsBool(self->dewr_start, key_and_value[0]);
		if unlikely(key_is_ok <= 0) {
err_iterkeys_key_or_filtered:
			if (key_is_ok == 0)
				goto next_index;
			goto err_iter_key_and_value;
		}
		key_is_ok = DeeObject_CmpGrAsBool(self->dewr_end, key_and_value[0]);
		if unlikely(key_is_ok <= 0)
			goto err_iterkeys_key_or_filtered;
		temp = (*cb)(arg, key_and_value[0], key_and_value[1]);
		if unlikely(temp < 0)
			goto err_iter_key_and_value_temp;
		result += temp;
next_index:
		Dee_Decrefv(key_and_value, 2);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if unlikely(!item)
		goto err_iter;
	Dee_Decref(iter);
	return result;
err_iter_key_and_value_temp:
	Dee_Decrefv(key_and_value, 2);
	Dee_Decref(iter);
	return temp;
err_iter_key_and_value:
	Dee_Decrefv(key_and_value, 2);
	goto err_iter;
err_iter_item:
	Dee_Decref(item);
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_we_foreach_pair(DefaultEnumeration_FullRange *__restrict self,
                   Dee_foreach_pair_t cb, void *arg) {
	return DeeMap_DefaultForeachPairWithEnumerate(self->defr_seq, cb, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_wei_foreach_pair(DefaultEnumeration_WithIntRange *__restrict self,
                   Dee_foreach_pair_t cb, void *arg) {
	struct foreach_with_enumerate_index_data data;
	struct type_seq *tp_seq = self->dewir_tp_seq;
	data.fewei_cb  = cb;
	data.fewei_arg = arg;
	return (*tp_seq->tp_enumerate_index)(self->dewir_seq, &foreach_with_enumerate_index_cb, &data,
	                                     self->dewir_start, self->dewir_end);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
de_weaf_foreach_pair(DefaultEnumeration_WithRange *__restrict self,
                     Dee_foreach_pair_t cb, void *arg) {
	struct foreach_with_enumerate_and_filter_data data;
	struct type_seq *tp_seq = self->dewr_tp_seq;
	data.feweaf_cb    = cb;
	data.feweaf_arg   = arg;
	data.feweaf_start = self->dewr_start;
	data.feweaf_end   = self->dewr_end;
	return (*tp_seq->tp_enumerate)(self->dewr_seq, &foreach_with_enumerate_and_filter_cb, &data);
}



PRIVATE struct type_seq de_wsagiif_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsagiif_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsagiif_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsagiif_foreach_pair,
};

PRIVATE struct type_seq de_wsatgii_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsatgii_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsatgii_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsatgii_foreach_pair,
};

PRIVATE struct type_seq de_wsagii_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsagii_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsagii_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsagii_foreach_pair,
};

PRIVATE struct type_seq de_wsoagi_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsoagi_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsoagi_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsoagi_foreach_pair,
};

PRIVATE struct type_seq de_wsagiifaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsagiifaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsagiifaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsagiifaf_foreach_pair,
};

PRIVATE struct type_seq de_wsatgiiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsatgiiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsatgiiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsatgiiaf_foreach_pair,
};

PRIVATE struct type_seq de_wsagiiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsagiiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsagiiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsagiiaf_foreach_pair,
};

PRIVATE struct type_seq de_wsoagiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wsoagiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wsoagiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wsoagiaf_foreach_pair,
};

PRIVATE struct type_seq de_wgii_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wgii_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wgii_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wgii_foreach_pair,
};

PRIVATE struct type_seq de_wgiiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wgiiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wgiiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wgiiaf_foreach_pair,
};

PRIVATE struct type_seq de_wgiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wgiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wgiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wgiaf_foreach_pair,
};

PRIVATE struct type_seq de_wikagi_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wikagi_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wikagi_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wikagi_foreach_pair,
};

PRIVATE struct type_seq de_wikagiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wikagiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wikagiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wikagiaf_foreach_pair,
};

PRIVATE struct type_seq de_wikatgi_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wikatgi_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wikatgi_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wikatgi_foreach_pair,
};

PRIVATE struct type_seq de_wikatgiaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wikatgiaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wikatgiaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wikatgiaf_foreach_pair,
};

PRIVATE struct type_seq de_wiac_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wiac_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wiac_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wiac_foreach_pair,
};

PRIVATE struct type_seq de_wiacaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wiacaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wiacaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wiacaf_foreach_pair,
};

PRIVATE struct type_seq de_wiauaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wiauaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wiauaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wiauaf_foreach_pair,
};

PRIVATE struct type_seq de_we_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_we_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_we_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_we_foreach_pair,
};

PRIVATE struct type_seq de_wei_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_wei_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_wei_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_wei_foreach_pair,
};

PRIVATE struct type_seq de_weaf_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_weaf_iter,
	/* .tp_sizeob       = */ NULL,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&de_weaf_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&de_weaf_foreach_pair,
};

#define de_wsagiif_members   fullrange_members
#define de_wsatgii_members   fullrange_members
#define de_wsagii_members    fullrange_members
#define de_wsoagi_members    fullrange_members
#define de_wsagiifaf_members withintrange_members
#define de_wsatgiiaf_members withintrange_members
#define de_wsagiiaf_members  withintrange_members
#define de_wsoagiaf_members  withrange_members
#define de_wgii_members      fullrange_members
#define de_wgiiaf_members    withintrange_members
#define de_wgiaf_members     withrange_members
#define de_wikagi_members    fullrange_members
#define de_wikagiaf_members  withrange_members
#define de_wikatgi_members   fullrange_members
#define de_wikatgiaf_members withrange_members
#define de_wiac_members      fullrange_members
#define de_wiacaf_members    withintrange_members
#define de_wiauaf_members    withintrange_members
#define de_we_members        fullrange_members
#define de_wei_members       withintrange_members
#define de_weaf_members      withrange_members

PRIVATE struct type_member tpconst withintrange_members[] = {
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(DefaultEnumeration_WithIntRange, dewir_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(DefaultEnumeration_WithIntRange, dewir_end)),
#define fullrange_members (withintrange_members + 2)
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT, offsetof(DefaultEnumeration_WithIntRange, dewir_seq)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst withrange_members[] = {
	TYPE_MEMBER_FIELD("__start__", STRUCT_OBJECT, offsetof(DefaultEnumeration_WithRange, dewr_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_OBJECT, offsetof(DefaultEnumeration_WithRange, dewr_end)),
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT, offsetof(DefaultEnumeration_WithRange, dewr_seq)),
	TYPE_MEMBER_END
};

#define de_wsagiifaf_class_members de_wsagiif_class_members
PRIVATE struct type_member tpconst de_wsagiif_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type),
	TYPE_MEMBER_END
};

#define de_wsatgiiaf_class_members de_wsatgii_class_members
PRIVATE struct type_member tpconst de_wsatgii_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type),
	TYPE_MEMBER_END
};

#define de_wsagiiaf_class_members de_wsagii_class_members
#define de_wgiiaf_class_members   de_wsagii_class_members
PRIVATE struct type_member tpconst de_wsagii_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndexPair_Type),
	TYPE_MEMBER_END
};

#define de_wsoagiaf_class_members de_wsoagi_class_members
#define de_wgiaf_class_members    de_wsoagi_class_members
PRIVATE struct type_member tpconst de_wsoagi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeObAndGetItemPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wgii_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithGetItemIndexPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wikagi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithIterKeysAndGetItemMap_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wikagiaf_class_members[] = {
	/* TODO: TYPE_MEMBER_CONST(STR_Iterator, &TODO), */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wikatgi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wikatgiaf_class_members[] = {
	/* TODO: TYPE_MEMBER_CONST(STR_Iterator, &TODO), */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wiac_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndCounterPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wiacaf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndCounterAndLimitPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wiauaf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndUnpackFilter_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_we_class_members[] = {
	/* TODO: TYPE_MEMBER_CONST(STR_Iterator, &TODO), */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_wei_class_members[] = {
	/* TODO: TYPE_MEMBER_CONST(STR_Iterator, &TODO), */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst de_weaf_class_members[] = {
	/* TODO: TYPE_MEMBER_CONST(STR_Iterator, &TODO), */
	TYPE_MEMBER_END
};



INTERN DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeAndGetItemIndexFast",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItemIndexFast)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsagiif_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsagiif_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsagiif_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsagiif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsagiif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsagiif_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsagiif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsagiif_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeAndTryGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeAndTryGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsatgii_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsatgii_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsatgii_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsatgii_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsatgii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsatgii_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsatgii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsatgii_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeAndGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsagii_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsagii_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsagii_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsagii_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsagii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsagii_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsagii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsagii_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeObAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeObAndGetItem",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsoagi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsoagi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsoagi_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsoagi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsoagi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsoagi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsoagi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsoagi_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndexFastAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeAndGetItemIndexFastAndFilter",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItemIndexFast,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsagiifaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsagiifaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsagiifaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsagiifaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsagiifaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsagiifaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsagiifaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsagiifaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeAndTryGetItemIndexAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeAndTryGetItemIndexAndFilter",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItem,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsatgiiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsatgiiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsatgiiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsatgiiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsatgiiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsatgiiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsatgiiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsatgiiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeAndGetItemIndexAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeAndGetItemIndexAndFilter",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItem,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsagiiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsagiiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsagiiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsagiiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsagiiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsagiiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsagiiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsagiiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithSizeObAndGetItemAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithSizeObAndGetItemAndFilter",
	/* .tp_doc      = */ DOC("(objWithSizeAndGetItem,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wsoagiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wsoagiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wsoagiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wsoagiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wsoagiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wsoagiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wsoagiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wsoagiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wgii_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wgii_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wgii_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wgii_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wgii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wgii_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wgii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wgii_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithGetItemIndexAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithGetItemIndexAndFilter",
	/* .tp_doc      = */ DOC("(objWithGetItem,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wgiiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wgiiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wgiiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wgiiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wgiiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wgiiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wgiiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wgiiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithGetItemAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithGetItemAndFilter",
	/* .tp_doc      = */ DOC("(objWithGetItem,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wgiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wgiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wgiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wgiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wgiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wgiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wgiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wgiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterKeysAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterKeysAndGetItem",
	/* .tp_doc      = */ DOC("(objWithIterKeysAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wikagi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wikagi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wikagi_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wikagi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wikagi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wikagi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wikagi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wikagi_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterKeysAndGetItemAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterKeysAndGetItemAndFilter",
	/* .tp_doc      = */ DOC("(objWithIterKeysAndGetItem,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wikagiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wikagiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wikagiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wikagiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wikagiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wikagiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wikagiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wikagiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterKeysAndTryGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterKeysAndTryGetItem",
	/* .tp_doc      = */ DOC("(objWithIterKeysAndGetItem)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wikatgi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wikatgi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wikatgi_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wikatgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wikatgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wikatgi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wikatgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wikatgi_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterKeysAndTryGetItemAndFilter",
	/* .tp_doc      = */ DOC("(objWithIterKeysAndGetItem,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wikatgiaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wikatgiaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wikatgiaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wikatgiaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wikatgiaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wikatgiaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wikatgiaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wikatgiaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterAndCounter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterAndCounter",
	/* .tp_doc      = */ DOC("(objWithIter)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wiac_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wiac_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wiac_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_FullRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wiac_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wiac_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wiac_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wiac_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wiac_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterAndCounterAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterAndCounterAndFilter",
	/* .tp_doc      = */ DOC("(objWithIter,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wiacaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wiacaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wiacaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wiacaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wiacaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wiacaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wiacaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wiacaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithIterAndUnpackAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithIterAndUnpackAndFilter",
	/* .tp_doc      = */ DOC("(objWithIter,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wiauaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wiauaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wiauaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wiauaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wiauaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wiauaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wiauaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wiauaf_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithEnumerate_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithEnumerate",
	/* .tp_doc      = */ DOC("(objWithEnumerate)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_we_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_we_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_we_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_we_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_we_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_we_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_we_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_we_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithEnumerateIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithEnumerateIndex",
	/* .tp_doc      = */ DOC("(objWithEnumerate,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_wei_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_wei_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_wei_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithIntRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_wei_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_wei_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_wei_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_wei_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_wei_class_members,
};

INTERN DeeTypeObject DefaultEnumeration_WithEnumerateAndFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEnumWithEnumerateAndFilter",
	/* .tp_doc      = */ DOC("(objWithEnumerate,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&de_weaf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&de_weaf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&de_weaf_init,
				TYPE_FIXED_ALLOCATOR(DefaultEnumeration_WithRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_weaf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&de_weaf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_weaf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_weaf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_weaf_class_members,
};





/* Default functions for constructing sequence enumerations. */

/* DeeSeq_DefaultMakeEnumerationWith... */
#define IMPL_return_new_DefaultEnumeration_FullRange(seq, type) \
	DREF DefaultEnumeration_FullRange *result;                  \
	result = DeeObject_MALLOC(DefaultEnumeration_FullRange);    \
	if unlikely(!result)                                        \
		goto err;                                               \
	result->defr_tp_seq = Dee_TYPE(seq)->tp_seq;                \
	Dee_Incref(seq);                                            \
	result->defr_seq = seq;                                     \
	DeeObject_Init(result, type);                               \
	return (DREF DeeObject *)result;                            \
err:                                                            \
	return NULL

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithSizeAndGetItemIndexFast_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithSizeAndTryGetItemIndex_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithSizeAndGetItemIndex_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithSizeObAndGetItem_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithGetItemIndex(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithGetItemIndex_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithIterKeysAndGetItem_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithIterKeysAndTryGetItem_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIterAndCounter(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithIterAndCounter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithEnumerate(DeeObject *self) {
	IMPL_return_new_DefaultEnumeration_FullRange(self, &DefaultEnumeration_WithEnumerate_Type);
}


/* DeeSeq_DefaultMakeEnumerationWithIntRangeWith... */
#define IMPL_return_new_DefaultEnumeration_WithIntRange(seq, start, end, type) \
	DREF DefaultEnumeration_WithIntRange *result;                              \
	result = DeeObject_MALLOC(DefaultEnumeration_WithIntRange);                \
	if unlikely(!result)                                                       \
		goto err;                                                              \
	result->dewir_tp_seq = Dee_TYPE(seq)->tp_seq;                              \
	Dee_Incref(seq);                                                           \
	result->dewir_seq   = seq;                                                 \
	result->dewir_start = start;                                               \
	result->dewir_end   = end;                                                 \
	DeeObject_Init(result, type);                                              \
	return (DREF DeeObject *)result;                                           \
err:                                                                           \
	return NULL

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexFastAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithSizeAndGetItemIndexFastAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndTryGetItemIndexAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithSizeAndTryGetItemIndexAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithSizeAndGetItemIndexAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeObAndGetItemAndFilter(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *startob, *endob, *result;
	startob = DeeInt_NewSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter(self, startob, endob);
	Dee_Decref_unlikely(endob);
	Dee_Decref_unlikely(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithGetItemIndexAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithGetItemIndexAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndGetItemAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithIterKeysAndGetItemAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndTryGetItemAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithIterAndCounterAndFilter_Type);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultMakeEnumerationWithIntRangeWithIterAndUnpackAndFilter(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *startob, *endob, *result;
	startob = DeeInt_NewSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter(self, startob, endob);
	Dee_Decref_unlikely(endob);
	Dee_Decref_unlikely(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultMakeEnumerationWithIntRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	IMPL_return_new_DefaultEnumeration_WithIntRange(self, start, end, &DefaultEnumeration_WithEnumerateIndex_Type);
}


/* DeeSeq_DefaultMakeEnumerationWithRangeWith... */
#define IMPL_return_new_DefaultEnumeration_WithRange(seq, start, end, type) \
	DREF DefaultEnumeration_WithRange *result;                              \
	result = DeeObject_MALLOC(DefaultEnumeration_WithRange);                \
	if unlikely(!result)                                                    \
		goto err;                                                           \
	result->dewr_tp_seq = Dee_TYPE(seq)->tp_seq;                            \
	Dee_Incref(seq);                                                        \
	result->dewr_seq = seq;                                                 \
	Dee_Incref(start);                                                      \
	result->dewr_start = start;                                             \
	Dee_Incref(end);                                                        \
	result->dewr_end = end;                                                 \
	DeeObject_Init(result, type);                                           \
	return (DREF DeeObject *)result;                                        \
err:                                                                        \
	return NULL

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	IMPL_return_new_DefaultEnumeration_WithRange(self, start, end, &DefaultEnumeration_WithSizeObAndGetItemAndFilter_Type);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	IMPL_return_new_DefaultEnumeration_WithRange(self, start, end, &DefaultEnumeration_WithGetItemAndFilter_Type);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	IMPL_return_new_DefaultEnumeration_WithRange(self, start, end, &DefaultEnumeration_WithIterKeysAndGetItemAndFilter_Type);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndTryGetItemAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	IMPL_return_new_DefaultEnumeration_WithRange(self, start, end, &DefaultEnumeration_WithIterKeysAndTryGetItemAndFilter_Type);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithIterAndCounterAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	/* DefaultEnumeration_WithIterAndCounterAndFilter_Type (same as
	 * `DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter') */
	size_t start_index, end_index;
	if (DeeObject_AsSize(start, &start_index))
		goto err;
	if (DeeObject_AsSize(end, &end_index))
		goto err;
	return DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter(self, start_index, end_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	IMPL_return_new_DefaultEnumeration_WithRange(self, start, end, &DefaultEnumeration_WithIterAndUnpackAndFilter_Type);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeMap_DefaultMakeEnumerationWithRangeWithEnumerateAndFilter(DeeObject *self, DeeObject *start, DeeObject *end) {
	IMPL_return_new_DefaultEnumeration_WithRange(self, start, end, &DefaultEnumeration_WithEnumerateAndFilter_Type);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_C */
