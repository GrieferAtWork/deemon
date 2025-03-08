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
/* deemon.Sequence.xchitem()                                            */
/************************************************************************/
[[kw, alias(Sequence.xchitem)]]
__seq_xchitem__(index:?Dint,item)->?O {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
	                    UNPuSIZ "o:__seq_xchitem__",
	                    &index, &item))
		goto err;
	return CALL_DEPENDENCY(seq_xchitem_index, self, index, item);
err:
	return NULL;
}


[[wunused]] DREF DeeObject *
__seq_xchitem__.seq_xchitem_index([[nonnull]] DeeObject *self, size_t index,
                                  [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = "default__seq_xchitem_index__unsupported"}
%{$with__seq_operator_getitem_index__and__seq_operator_setitem_index = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_getitem_index, self, index);
	if likely(result) {
		if (CALL_DEPENDENCY(seq_operator_setitem_index, self, index, item))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ "o", index, item);
}

seq_xchitem_index = {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index = REQUIRE(seq_operator_setitem_index);
	if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
		return &$empty;
	if (seq_operator_setitem_index) {
		if (REQUIRE_ANY(seq_operator_getitem_index) != &default__seq_operator_getitem_index__unsupported)
			return &$with__seq_operator_getitem_index__and__seq_operator_setitem_index;
	}
};
