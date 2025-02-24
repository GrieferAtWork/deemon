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
/* deemon.Sequence.append()                                             */
/************************************************************************/
[[alias(Sequence.append -> "seq_append")]]
[[alias(Sequence.pushback -> "seq_pushback")]]
__seq_append__(item) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:__seq_append__", &item))
		goto err;
	if unlikely(CALL_DEPENDENCY(seq_append, self, item))
		goto err;
	return_none;
err:
	return NULL;
}


[[wunused]]
int __seq_append__.seq_append([[nonnull]] DeeObject *self,
                              [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__seq_append__unsupported"}
%{$with__seq_extend = {
	int result;
	DREF DeeTupleObject *item_tuple;
	item_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!item_tuple)
		goto err;
	DeeTuple_SET(item_tuple, 0, item);
	result = CALL_DEPENDENCY(seq_extend, self, (DeeObject *)item_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)item_tuple);
	return result;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_append = {
	DeeMH_seq_extend_t seq_extend = REQUIRE(seq_extend);
	if (seq_extend == &default__seq_extend__empty)
		return &$empty;
	if (seq_extend)
		return &$with__seq_extend;
};
