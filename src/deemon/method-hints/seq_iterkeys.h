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
/* deemon.Sequence.__iterkeys__()                                       */
/************************************************************************/
__seq_iterkeys__()->?DIterator {
	if (DeeArg_Unpack(argc, argv, ":__seq_iterkeys__"))
		goto err;
	return DeeType_InvokeMethodHint0(self, seq_iterkeys);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_iterkeys__.seq_iterkeys([[nonnull]] DeeObject *self)
%{unsupported(auto)}
%{$empty = "default__seq_operator_iter__empty"}
%{$with__seq_size = {
	size_t size;
	DREF IntRangeIterator *result;
	size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(IntRangeIterator);
	if unlikely(!result)
		goto err;
	result->iri_index = 0;
	result->iri_end   = (Dee_ssize_t)size; /* TODO: Need another range iterator type that uses unsigned indices */
	result->iri_step  = 1;
	DeeObject_Init(result, &SeqIntRangeIterator_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

seq_iterkeys = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_size;
};
