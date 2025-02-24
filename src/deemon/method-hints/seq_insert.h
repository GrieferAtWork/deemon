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
/* deemon.Sequence.insert()                                             */
/************************************************************************/
[[kw, alias(Sequence.insert -> "seq_insert")]]
__seq_insert__(index:?Dint,item) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
	                    UNPuSIZ "o:__seq_insert__",
	                    &index, &item))
		goto err;
	if unlikely(CALL_DEPENDENCY(seq_insert, self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}


[[wunused]]
int __seq_insert__.seq_insert([[nonnull]] DeeObject *self, size_t index,
                              [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__seq_insert__unsupported"}
%{$with__seq_insertall = {
	int result;
	DREF DeeTupleObject *item_tuple;
	item_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!item_tuple)
		goto err;
	DeeTuple_SET(item_tuple, 0, item);
	result = CALL_DEPENDENCY(seq_insertall, self, index, (DeeObject *)item_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)item_tuple);
	return result;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_insert = {
	DeeMH_seq_insertall_t seq_insertall = REQUIRE(seq_insertall);
	if (seq_insertall == &default__seq_insertall__empty)
		return &$empty;
	if (seq_insertall)
		return &$with__seq_insertall;
};
