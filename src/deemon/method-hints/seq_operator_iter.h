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
	return DeeType_InvokeMethodHint0(self, seq_operator_iter);
err:
	return NULL;
}

[[operator([Sequence,Set,Mapping].OPERATOR_ITER: tp_seq->tp_iter)]]
[[wunused]] DREF DeeObject *
__seq_iter__.seq_operator_iter([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator iter"))}
%{$empty = { return_empty_iterator; }}
{
	return LOCAL_CALLATTR(self, 0, NULL);
}

[[operator([Sequence,Set,Mapping].OPERATOR_ITER: tp_seq->tp_foreach)]]
[[wunused]] Dee_ssize_t
__seq_iter__.seq_operator_foreach([[nonnull]] DeeObject *__restrict self,
                                  [[nonnull]] Dee_foreach_t cb,
                                  void *arg)
%{$empty = 0} %{$with__seq_operator_iter = {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeType_InvokeMethodHint0(self, seq_operator_iter);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}} = $with__seq_operator_iter;


%[define(DEFINE_default_seq_operator_foreach_pair__with__seq_operator_foreach_cb =
#ifndef DEFINED_default_seq_operator_foreach_pair__with__seq_operator_foreach_cb
#define DEFINED_default_seq_operator_foreach_pair__with__seq_operator_foreach_cb
struct default_seq_operator_foreach_pair__with__seq_operator_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_operator_foreach_pair__with__seq_operator_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_seq_operator_foreach_pair__with__seq_operator_foreach_cb */
)]


[[operator([Sequence,Set,Mapping].OPERATOR_ITER: tp_seq->tp_foreach_pair)]]
[[wunused]] Dee_ssize_t
__seq_iter__.seq_operator_foreach_pair([[nonnull]] DeeObject *__restrict self,
                                       [[nonnull]] Dee_foreach_pair_t cb,
                                       void *arg)
%{$empty = 0}
%{$with__seq_operator_iter = {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeType_InvokeMethodHint0(self, seq_operator_iter);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_operator_foreach_pair__with__seq_operator_foreach_cb)]] {
	struct default_seq_operator_foreach_pair__with__seq_operator_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return DeeType_InvokeMethodHint(self, seq_operator_foreach, &default_seq_operator_foreach_pair__with__seq_operator_foreach_cb, &data);
}} = $with__seq_operator_foreach;

//seq_operator_iter = {
//	// TODO: $with__seq_operator_getitem_index__and__seq_operator_size
//};

seq_operator_foreach = {
	DeeMH_seq_operator_iter_t seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter)
		return &$with__seq_operator_iter;
};

seq_operator_foreach_pair = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &$with__seq_operator_iter;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};
