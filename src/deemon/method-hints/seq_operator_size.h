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
/* deemon.Sequence.operator size()                                      */
/************************************************************************/
__seq_size__()->?Dint {
	if (DeeArg_Unpack(argc, argv, ":__seq_size__"))
		goto err;
	return DeeSeq_OperatorSizeOb(self);
err:
	return NULL;
}

[[wunused]]
DREF DeeObject *__seq_size__.seq_operator_sizeob([[nonnull]] DeeObject *self)
%{unsupported(auto("operator size"))}
%{$empty = { return_reference_(DeeInt_Zero); }}
%{$with__seq_operator_size = {
	size_t seqsize = DeeSeq_OperatorSize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(seqsize);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}


%[define(DEFINE_default_seq_size_with_foreach_cb =
INTDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_cb(void *arg, DeeObject *elem);
)]

[[wunused]]
size_t __seq_size__.seq_operator_size([[nonnull]] DeeObject *self)
%{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_size_with_foreach_cb)]] {
	return (size_t)DeeSeq_OperatorForeach(self, &default_seq_size_with_foreach_cb, NULL);
}} %{$with__seq_operator_sizeob = {
	size_t result;
	DREF DeeObject *sizeob;
	sizeob = DeeSeq_OperatorSizeOb(self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}} = $with__seq_operator_sizeob;


seq_operator_sizeob = {
	DeeMH_seq_operator_size_t seq_operator_size;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS != Dee_SEQCLASS_NONE && DeeType_RequireSizeOb(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_sizeob;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_size = REQUIRE(seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size)
		return &$with__seq_operator_size;
};

seq_operator_size = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS != Dee_SEQCLASS_NONE && DeeType_RequireSize(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_size;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return $with__seq_operator_foreach;
};
