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
/* deemon.Sequence.unpackub()                                             */
/************************************************************************/
[[alias(Sequence.unpackub)]]
[[docstring("(length:?Dint)->?Ert:NullableTuple\n"
            "(min:?Dint,max:?Dint)->?Ert:NullableTuple")]]
__seq_unpackub__(min:?Dint,max?:?Dint)->?Ert:NullableTuple {{
	DREF DeeTupleObject *result;
	size_t min_count, max_count;
	if (argc == 1) {
		min_count = DeeObject_AsSizeDirect(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = min_count;
	} else if (argc == 2) {
		min_count = DeeObject_AsSizeDirect(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = DeeObject_AsSizeDirect(argv[1]);
		if unlikely(max_count == (size_t)-1)
			goto err;
		if unlikely(min_count > max_count) {
			DeeError_Throwf(&DeeError_ValueError,
			                "In __seq_unpackub__: min(%" PRFuSIZ ") "
			                "is greater than max(%" PRFuSIZ ")",
			                min_count, max_count);
			goto err;
		}
	} else {
		err_invalid_argc("__seq_unpackub__", argc, 1, 2);
		goto err;
	}
	result = DeeTuple_NewUninitialized(max_count);
	if unlikely(!result)
		goto err;
	min_count = CALL_DEPENDENCY(seq_unpack_ub, self, min_count, max_count, result->t_elem);
	if unlikely(min_count == (size_t)-1)
		goto err_r;
	ASSERT(min_count <= max_count);
	if (min_count < max_count)
		result = DeeTuple_TruncateUninitialized(result, min_count);
	Dee_DecrefNokill(&DeeTuple_Type);
	Dee_Incref(&DeeNullableTuple_Type);
	ASSERT(result->ob_type == &DeeTuple_Type);
	result->ob_type = &DeeNullableTuple_Type;
	return (DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}}


/* @return: * : The actual # of objects written to `result' (always in range [min_count, max_count])
 * @return: (size_t)-1: Error */
[[wunused]]
size_t __seq_unpackub__.seq_unpack_ub([[nonnull]] DeeObject *__restrict self,
                                      size_t min_count, size_t max_count,
                                      [[nonnull]] /*out*/ DREF DeeObject *result[])
%{unsupported({
	return err_seq_unsupportedf(self, "__seq_unpackub__(%" PRFuSIZ ", %" PRFuSIZ ")", min_count, max_count);
})}
%{$none = "default__seq_unpack_ex__none"}
%{$empty = {
	if unlikely(min_count > 0)
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, 0);
	return 0;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i)
		result[i] = (*getitem_index_fast)(self, i); /* Inherit reference */
	return real_count;
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index);
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
		if unlikely(!ITER_ISOK(item)) {
			if unlikely(!item)
				goto err_result_i;
			item = NULL; /* Unbound */
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err_result_i:
	Dee_XDecrefv(result, i);
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index);
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!item) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* It's just unbound (which is allowed) */
			} else {
				DREF DeeObject *error;
				if ((error = DeeError_CatchError(&DeeError_IndexError)) != NULL) {
					/* Early sequence end (sequence may have been truncated) */
					if (i >= min_count)
						return i;
					DeeRT_ErrUnpackErrorExWithCause(self, min_count, max_count, i, error);
					goto err_result_i;
				} else {
					goto err_result_i;
				}
			}
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err_result_i:
	Dee_XDecrefv(result, i);
err:
	return (size_t)-1;
}} {
	size_t result_count;
	DREF DeeObject *resultob;
	resultob = min_count == max_count ? LOCAL_CALLATTRF(self, PCKuSIZ, min_count)
	                                  : LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (!DeeObject_InstanceOfExact(resultob, &DeeTuple_Type) &&
	    !DeeObject_InstanceOfExact(resultob, &DeeNullableTuple_Type)) {
		DeeObject_TypeAssertFailed2(resultob, &DeeTuple_Type, &DeeNullableTuple_Type);
		goto err_r;
	}
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		DeeRT_ErrUnpackErrorEx(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	/* XXX: DeeNullableTuple_DecrefSymbolic(resultob); */
	Dee_XMovrefv(result, DeeTuple_ELEM(resultob), result_count);
	Dee_Decref_likely(resultob);
	return result_count;
err_r:
	Dee_Decref_likely(resultob);
err:
	return (size_t)-1;
}


seq_unpack_ub = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (HAS_TRAIT(__seq_getitem_always_bound__))
		return REQUIRE(seq_unpack_ex); /* Can just re-use the regular `seq_unpack_ex' */
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach)
		return REQUIRE(seq_unpack_ex);
};
