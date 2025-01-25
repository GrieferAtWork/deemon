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
/* deemon.Sequence.operator hash()                                      */
/************************************************************************/
__seq_hash__()->?Dint {
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__seq_hash__"))
		goto err;
	result = DeeSeq_OperatorHash(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

%[define(DEFINE_default_seq_hash_with_foreach_cb =
#ifndef DEFINED_default_seq_hash_with_foreach_cb
#define DEFINED_default_seq_hash_with_foreach_cb
struct default_seq_hash_with_foreach_data {
	Dee_hash_t sqhwf_result;   /* Hash result (or DEE_HASHOF_EMPTY_SEQUENCE when sqhwf_nonempty=false) */
	bool       sqhwf_nonempty; /* True after the first element */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_seq_hash_with_foreach_cb */
)]

%[define(DEFINE_DeeSeq_HandleHashError =
#ifndef DEFINED_DeeSeq_HandleHashError
#define DEFINED_DeeSeq_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSeq_HandleHashError */
)]

[[wunused]]
Dee_hash_t __seq_hash__.seq_operator_hash([[nonnull]] DeeObject *self)
%{unsupported(auto("operator hash"))}
%{$empty = DEE_HASHOF_EMPTY_SEQUENCE}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_default_seq_hash_with_foreach_cb)]]
[[prefix(DEFINE_DeeSeq_HandleHashError)]] {
	struct default_seq_hash_with_foreach_data data;
	data.sqhwf_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwf_nonempty = false;
	if unlikely(DeeSeq_OperatorForeach(self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return DeeSeq_HandleHashError(self);
}}
[[prefix(DEFINE_DeeSeq_HandleHashError)]] {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

seq_operator_hash = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireHash(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return $with__seq_operator_foreach;
};
