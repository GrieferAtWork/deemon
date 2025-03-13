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
/* deemon.Sequence.operator []=()                                       */
/************************************************************************/
__seq_setitem__(index:?Dint,value) {
	if (CALL_DEPENDENCY(seq_operator_setitem, self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

[[operator(Sequence: tp_seq->tp_setitem)]]
[[wunused]] int
__seq_setitem__.seq_operator_setitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index,
                                     [[nonnull]] DeeObject *value)
%{unsupported(auto("operator []="))}
%{$none = 0}
%{$empty = err_index_out_of_bounds_ob(self, index)}
%{using seq_operator_setitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_setitem_index, self, index_value, value);
err:
	return -1;
}} {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	result = LOCAL_CALLATTR(self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}




[[operator(Sequence: tp_seq->tp_setitem_index)]]
[[wunused]] int
__seq_setitem__.seq_operator_setitem_index([[nonnull]] DeeObject *self,
                                           size_t index,
                                           [[nonnull]] DeeObject *value)
%{unsupported(auto("operator []="))}
%{$none = 0}
%{$empty = err_index_out_of_bounds(self, index, 0)}
%{$with__seq_operator_setrange_index = {
	int result;
	DREF DeeTupleObject *values = DeeTuple_NewUninitialized(1);
	if unlikely(!values)
		goto err;
	values->t_elem[0] = value;
	result = CALL_DEPENDENCY(seq_operator_setrange_index, self, (Dee_ssize_t)index, (Dee_ssize_t)index + 1, (DeeObject *)values);
	DeeTuple_DecrefSymbolic((DeeObject *)values);
	return result;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ "o", index, value);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_operator_setitem = {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index = REQUIRE(seq_operator_setitem_index);
	if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
		return &$empty;
	if (seq_operator_setitem_index)
		return &$with__seq_operator_setitem_index;
};

seq_operator_setitem_index = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (REQUIRE_NODEFAULT(seq_operator_setrange_index))
		return &$with__seq_operator_setrange_index;
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
};

