/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

/************************************************************************/
/* deemon.Object.operator iter()                                        */
/************************************************************************/

operator {

[[export("DeeObject_{|T}Iter")]]
[[wunused]] DREF DeeObject *
tp_seq->tp_iter([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_ITER: {
	return_DeeClass_CallOperator_NoArgs(THIS_TYPE, self, OPERATOR_ITER);
}} = OPERATOR_ITER;




%[define(DEFINE_default_foreach_with_foreach_pair_cb =
#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_foreach_pair_data *data;
	Dee_ssize_t result;
	DREF DeeObject *pair;
	data = (struct default_foreach_with_foreach_pair_data *)arg;
	pair = DeeSeq_OfPairSymbolic(key, value);
	if unlikely(!pair)
		goto err;
	result = (*data->dfwfp_cb)(data->dfwfp_arg, pair);
	DeeSeqPair_DecrefSymbolic(pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_foreach_pair_cb */
)]


[[export("DeeObject_{|T}Foreach")]]
[[wunused]] Dee_ssize_t
tp_seq->tp_foreach([[nonnull]] DeeObject *__restrict self,
                   [[nonnull]] Dee_foreach_t cb, void *arg)
%{using tp_seq->tp_iter: {
	Dee_ssize_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(tp_seq->tp_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}}
%{using tp_seq->tp_foreach_pair: [[prefix(DEFINE_default_foreach_with_foreach_pair_cb)]] {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return CALL_DEPENDENCY(tp_seq->tp_foreach_pair, self, &default_foreach_with_foreach_pair_cb, &data);
}} = OPERATOR_ITER;





%[define(DEFINE_default_foreach_pair_with_foreach_cb =
#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_foreach_pair_with_foreach_data *data;
	Dee_ssize_t result;
	data = (struct default_foreach_pair_with_foreach_data *)arg;
	if likely(DeeTuple_Check(elem) && DeeTuple_SIZE(elem) == 2) {
		result = (*data->dfpwf_cb)(data->dfpwf_arg,
		                           DeeTuple_GET(elem, 0),
		                           DeeTuple_GET(elem, 1));
	} else {
		DREF DeeObject *pair[2];
		if unlikely(DeeSeq_Unpack(elem, 2, pair))
			goto err;
		result = (*data->dfpwf_cb)(data->dfpwf_arg, pair[0], pair[1]);
		Dee_Decref_unlikely(pair[1]);
		Dee_Decref_unlikely(pair[0]);
	}
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_pair_with_foreach_cb */
)]


[[export("DeeObject_{|T}ForeachPair")]]
[[wunused]] Dee_ssize_t
tp_seq->tp_foreach_pair([[nonnull]] DeeObject *__restrict self,
                        [[nonnull]] Dee_foreach_pair_t cb, void *arg)
%{using tp_seq->tp_foreach: [[prefix(DEFINE_default_foreach_pair_with_foreach_cb)]] {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return CALL_DEPENDENCY(tp_seq->tp_foreach, self, &default_foreach_pair_with_foreach_cb, &data);
}}
%{using tp_seq->tp_iter: {
	Dee_ssize_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(tp_seq->tp_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}} = OPERATOR_ITER;


} /* operator */
