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

/************************************************************************/
/* deemon.Sequence.operator del[]()                                     */
/************************************************************************/
__seq_delitem__(index:?Dint) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__seq_delitem__", &index))
		goto err;
	if (DeeSeq_OperatorDelItem(self, index))
		goto err;
	return_none;
err:
	return NULL;
}

[[wunused]] int
__seq_delitem__.seq_operator_delitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index)
%{unsupported(auto("operator del[]"))}
%{$empty = err_index_out_of_bounds_ob(self, index)}
%{$with__seq_operator_delitem_index = {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorDelItemIndex(self, index_value);
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}




[[wunused]] int
__seq_delitem__.seq_operator_delitem_index([[nonnull]] DeeObject *__restrict self,
                                           size_t index)
%{unsupported(auto("operator del[]"))}
%{$empty = err_index_out_of_bounds(self, index, 0)}
%{$with__seq_operator_size__and__seq_erase = {
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	return DeeSeq_InvokeErase(self, index, 1);
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ, index);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_operator_delitem = {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireDelItem(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_delitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_delitem_index = REQUIRE(seq_operator_delitem_index);
	if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
		return &$empty;
	if (seq_operator_delitem_index)
		return &$with__seq_operator_delitem_index;
};

seq_operator_delitem_index = {
	DeeMH_seq_erase_t seq_erase;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireDelItemIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_delitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_erase = REQUIRE_NODEFAULT(seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &$empty;
	if (seq_erase && REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
		return &$with__seq_operator_size__and__seq_erase;
};

