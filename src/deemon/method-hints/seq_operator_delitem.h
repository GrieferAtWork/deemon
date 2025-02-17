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
	if (CALL_DEPENDENCY(seq_operator_delitem, self, index))
		goto err;
	return_none;
err:
	return NULL;
}



[[operator(Sequence.OPERATOR_DELITEM: tp_seq->tp_delitem)]]
[[wunused]] int
__seq_delitem__.seq_operator_delitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index)
%{unsupported(auto("operator del[]"))}
%{$empty = err_index_out_of_bounds_ob(self, index)}
%{using seq_operator_delitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_delitem_index, self, index_value);
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




[[operator(Sequence.OPERATOR_DELITEM: tp_seq->tp_delitem_index)]]
[[wunused]] int
__seq_delitem__.seq_operator_delitem_index([[nonnull]] DeeObject *__restrict self,
                                           size_t index)
%{unsupported(auto("operator del[]"))}
%{$empty = err_index_out_of_bounds(self, index, 0)}
%{$with__seq_operator_delrange_index = {
	return CALL_DEPENDENCY(seq_operator_delrange_index, self, (Dee_ssize_t)index, (Dee_ssize_t)index + 1);
}}
%{$with__seq_operator_size__and__seq_erase = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	return CALL_DEPENDENCY(seq_erase, self, index, 1);
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
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index = REQUIRE(seq_operator_delitem_index);
	if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
		return &$empty;
	if (seq_operator_delitem_index)
		return &$with__seq_operator_delitem_index;
	/* TODO: CALL_DEPENDENCY(set_remove, self, CALL_DEPENDENCY(seq_operator_getitem, self, index)) */
};

seq_operator_delitem_index = {
	DeeMH_seq_erase_t seq_erase;
	if (REQUIRE_NODEFAULT(seq_operator_delrange_index) ||
	    REQUIRE_NODEFAULT(seq_operator_setrange_index))
		return &$with__seq_operator_delrange_index;
	seq_erase = REQUIRE_NODEFAULT(seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &$empty;
	if (seq_erase && REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
		return &$with__seq_operator_size__and__seq_erase;
	/* TODO: CALL_DEPENDENCY(set_remove, self, CALL_DEPENDENCY(seq_operator_getitem_index, self, index)) */
};

