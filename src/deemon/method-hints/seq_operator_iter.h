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
/* deemon.Sequence.operator iter()                                      */
/************************************************************************/
__seq_iter__()->?DIterator {
	if (DeeArg_Unpack(argc, argv, ":__seq_iter__"))
		goto err;
	return DeeSeq_OperatorIter(self);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_iter__.seq_operator_iter([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator iter"))}
%{$empty = { return_empty_iterator; }}
{
	return LOCAL_CALLATTR(self, 0, NULL);
}

[[wunused]] Dee_ssize_t
__seq_iter__.seq_operator_foreach([[nonnull]] DeeObject *__restrict self,
                                  [[nonnull]] Dee_foreach_t proc,
                                  void *arg)
%{$empty = 0} %{$with__seq_operator_iter = {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeSeq_OperatorIter(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, proc, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}} = $with__seq_operator_iter;

[[wunused]] Dee_ssize_t
__seq_iter__.seq_operator_foreach_pair([[nonnull]] DeeObject *__restrict self,
                                       [[nonnull]] Dee_foreach_pair_t proc,
                                       void *arg)
%{$empty = 0} %{$with__seq_operator_iter = {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeSeq_OperatorIter(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, proc, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}} = $with__seq_operator_iter;

seq_operator_iter = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireIter(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_iter;
#endif /* !LOCAL_FOR_OPTIMIZE */
};

seq_operator_foreach = {
	DeeMH_seq_operator_iter_t seq_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeach(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter)
		return &$with__seq_operator_iter;
};

seq_operator_foreach_pair = {
	DeeMH_seq_operator_iter_t seq_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeachPair(self))
		return THIS_TYPE->tp_seq->tp_foreach_pair;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter)
		return &$with__seq_operator_iter;
};

