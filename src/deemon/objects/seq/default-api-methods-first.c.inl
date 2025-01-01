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
#ifdef __INTELLISENSE__
#include "default-api-methods.c"
#endif /* __INTELLISENSE__ */

DECL_BEGIN

/************************************************************************/
/* Sequence.first                                                       */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetFirstWithSeqGetFirst(DeeObject *__restrict self) {
	DREF DeeObject *result = DeeSeq_InvokeGetFirst(self);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetFirstWithTryGetItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetFirstWithTryGetItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, 0);
	if unlikely(!result)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndex(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, 0);
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
DeeSeq_DefaultTryGetFirstWithSizeAndGetItem(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, DeeInt_Zero);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getfirst_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	*(DREF DeeObject **)arg = item;
	return -2;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultTryGetFirstWithForeach(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithForeach(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithError(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "first");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, DeeInt_Zero);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_boundfirst_with_foreach_cb(void *arg, DeeObject *item) {
	(void)arg;
	(void)item;
	return -2;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithForeach(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &seq_default_boundfirst_with_foreach_cb, NULL);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if likely(foreach_status == -2)
		return Dee_BOUND_YES;
	if unlikely(foreach_status == -1)
		return Dee_BOUND_ERR;
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithError(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "first is bound");
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithSeqGetFirstAndSetRemove(DeeObject *__restrict self) {
	int result;
	DREF DeeObject *key = DeeSeq_InvokeGetFirst(self);
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
DeeSeq_DefaultDelFirstWithSeqGetFirstAndMaplikeDelItem(DeeObject *__restrict self) {
	int result;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = DeeSeq_InvokeGetFirst(self);
	if unlikely(!item)
		goto err;
	if unlikely(DeeObject_Unpack(item, 2, key_and_value))
		goto err_item;
	Dee_Decref(item);
	Dee_Decref(key_and_value[1]);
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	return result;
err_item:
	Dee_Decref(item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "del first");
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value) {
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_index)(self, 0, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value) {
	return (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, DeeInt_Zero, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value) {
	return err_seq_unsupportedf(self, "first = %r", value);
}

DECL_END
