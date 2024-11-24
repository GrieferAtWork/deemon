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
#ifdef __INTELLISENSE__
#include "default-api-methods.c"
#endif /* __INTELLISENSE__ */

DECL_BEGIN

/************************************************************************/
/* Sequence.last                                                        */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetLastWithSeqGetLast(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeSeq_InvokeGetLast(self);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, size - 1);
	if unlikely(!result)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, size - 1);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetLastWithSizeAndGetItem(DeeObject *__restrict self) {
	int temp;
	DREF DeeObject *result, *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp) {
		Dee_Decref(sizeob);
		return ITER_DONE;
	}
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, sizeob);
	Dee_Decref(sizeob);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err_sizeob:
	Dee_Decref(sizeob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_last_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	Dee_Decref_unlikely(*(DREF DeeObject **)arg);
	*(DREF DeeObject **)arg = item;
	return 1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetLastWithForeach(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	Dee_Incref(Dee_None);
	result = Dee_None;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_last_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	result = (*seq->tp_getitem_index_fast)(self, size);
	if unlikely(!result)
		err_unbound_index(self, size);
	return result;
err_isempty:
	err_empty_sequence(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_getitem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItem(DeeObject *__restrict self) {
	int temp;
	DREF DeeObject *result, *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithForeach(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	Dee_Incref(Dee_None);
	result = Dee_None;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_last_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithError(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "last");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_bounditem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeAndBoundItem(DeeObject *__restrict self) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithError(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "last is bound");
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSizeAndDelItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_delitem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSizeAndDelItem(DeeObject *__restrict self) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSeqGetLastAndSetRemove(DeeObject *__restrict self) {
	int result;
	DREF DeeObject *key = DeeSeq_InvokeGetLast(self);
	if unlikely(!key)
		goto err;
	result = DeeSet_InvokeRemove(self, key);
	Dee_Decref(key);
	if (result > 0)
		result = 0;
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSeqGetLastAndMaplikeDelItem(DeeObject *__restrict self) {
	int result;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = DeeSeq_InvokeGetLast(self);
	if unlikely(!item)
		goto err;
	if unlikely(DeeObject_Unpack(item, 2, key_and_value))
		goto err_item;
	Dee_Decref(item);
	Dee_Decref(key_and_value[1]);
	result = DeeObject_DelItem(self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	return result;
err_item:
	Dee_Decref(item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithError(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "del last");
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSizeAndSetItemIndex(DeeObject *self, DeeObject *value) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_setitem_index)(self, size, value);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSizeAndSetItem(DeeObject *self, DeeObject *value) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, sizeob, value);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithError(DeeObject *self, DeeObject *value) {
	return err_seq_unsupportedf(self, "last = %r", value);
}

DECL_END
