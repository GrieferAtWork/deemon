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
/* deemon.Sequence.unpack()                                             */
/************************************************************************/
[[alias(Sequence.unpack)]]
[[docstring("(length:?Dint)->?DTuple\n"
            "(min:?Dint,max:?Dint)->?DTuple")]]
__seq_unpack__(size_t min, size_t max?)->?DTuple {{
	DREF DeeTupleObject *result;
	size_t min_count, max_count;
	if (argc == 1) {
		min_count = DeeObject_AsSizeDirect(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
handle_single_count:
		result = DeeTuple_NewUninitialized(min_count);
		if unlikely(!result)
			goto err;
		if unlikely(CALL_DEPENDENCY(seq_unpack, self, min_count, result->t_elem))
			goto err_r;
	} else if (argc == 2) {
		min_count = DeeObject_AsSizeDirect(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = DeeObject_AsSizeDirect(argv[1]);
		if unlikely(max_count == (size_t)-1)
			goto err;
		if unlikely(min_count >= max_count) {
			if (min_count == max_count)
				goto handle_single_count;
			DeeError_Throwf(&DeeError_ValueError,
			                "In __seq_unpack__: min(%" PRFuSIZ ") "
			                "is greater than max(%" PRFuSIZ ")",
			                min_count, max_count);
			goto err;
		}
		result = DeeTuple_NewUninitialized(max_count);
		if unlikely(!result)
			goto err;
		min_count = CALL_DEPENDENCY(seq_unpack_ex, self, min_count, max_count, result->t_elem);
		if unlikely(min_count == (size_t)-1)
			goto err_r;
		ASSERT(min_count <= max_count);
		if (min_count < max_count)
			result = DeeTuple_TruncateUninitialized(result, min_count);
	} else {
		err_invalid_argc("__seq_unpack__", argc, 1, 2);
		goto err;
	}
	return (DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}}


%[define(DEFINE_default_unpack_with_foreach_cb =
#ifndef DEFINED_default_unpack_with_foreach_cb
#define DEFINED_default_unpack_with_foreach_cb
struct default_unpack_with_foreach_data {
	size_t           duqfd_count;  /* Remaining destination length */
	DREF DeeObject **duqfd_result; /* [?..?][0..duqfd_count] Pointer to next destination */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_unpack_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_unpack_with_foreach_data *data;
	data = (struct default_unpack_with_foreach_data *)arg;
	if likely(data->duqfd_count) {
		Dee_Incref(elem);
		*data->duqfd_result = elem;
		--data->duqfd_count;
		++data->duqfd_result;
	}
	return 1;
}
#endif /* !DEFINED_default_unpack_with_foreach_cb */
)]


/* Unpack the given sequence `self' into `dst_length' items then stored within the `dst' vector.
 * This operator follows `DeeObject_Foreach()' semantics, in that unbound items are skipped.
 * @return: 0 : Success (`dst' now contains exactly `dst_length' references to [1..1] objects)
 * @return: -1: An error was thrown (`dst' may have been modified, but contains no references) */
[[wunused]]
int __seq_unpack__.seq_unpack([[nonnull]] DeeObject *__restrict self, size_t count,
                              [[nonnull]] /*out*/ DREF DeeObject *result[])
%{unsupported({
	return err_seq_unsupportedf(self, "__seq_unpack__(%" PRFuSIZ ")", count);
})}
%{$none = {
	/* "none" can be unpacked into any number of none-s. */
	Dee_Setrefv(result, Dee_None, count);
	return 0;
}}
%{$empty = {
	if unlikely(count != 0)
		return DeeRT_ErrUnpackError(self, count, 0);
	return 0;
}}
%{using seq_unpack_ex: {
	size_t real_count = CALL_DEPENDENCY(seq_unpack_ex, self, count, count, result);
	if likely(real_count != (size_t)-1)
		real_count = 0;
	return (int)(Dee_ssize_t)real_count;
}}
%{$with__tp_asvector = [[inherit_as($with__seq_operator_foreach)]] {
	size_t real_count = (*THIS_TYPE->tp_seq->tp_asvector)(self, count, result);
	if likely(real_count == count)
		return 0;
	DeeRT_ErrUnpackError(self, count, real_count);
	if (real_count < count)
		Dee_Decrefv(result, real_count);
	return -1;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast = /* requires: __seq_getitem_always_bound__ */
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count != count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return DeeRT_ErrUnpackError(self, count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*getitem_index_fast)(self, i);
		if unlikely(!item) {
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			Dee_Decrefv(result, i);
			return DeeRT_ErrUnpackError(self, count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = { /* requires: __seq_getitem_always_bound__ */
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index);
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count != count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return DeeRT_ErrUnpackError(self, count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
		if unlikely(!ITER_ISOK(item)) {
			Dee_Decrefv(result, i);
			if unlikely(!item)
				goto err;
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			return DeeRT_ErrUnpackError(self, count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = { /* requires: __seq_getitem_always_bound__ */
	PRELOAD_DEPENDENCY(seq_operator_getitem_index);
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count != count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return DeeRT_ErrUnpackError(self, count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!item) {
			DREF DeeObject *error;
			Dee_Decrefv(result, i);
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if ((error = DeeError_CatchError(&DeeError_IndexError)) != NULL)
				return DeeRT_ErrUnpackErrorWithCause(self, count, i, error);
			goto err;
		}
		result[i] = item; /* Inherit reference */
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_unpack_with_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_unpack_with_foreach_data data;
	data.duqfd_count  = count;
	data.duqfd_result = result;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_unpack_with_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	ASSERT(((size_t)foreach_status == (size_t)(data.duqfd_result - result)) ||
	       ((size_t)foreach_status > count));
	if likely((size_t)foreach_status == count)
		return 0;
	Dee_Decrefv(result, (size_t)(data.duqfd_result - result));
	DeeRT_ErrUnpackError(self, count, (size_t)foreach_status);
err:
	return -1;
}}
%{$with__seq_operator_iter = {
	size_t i, remainder;
	DREF DeeObject *elem;
	DREF DeeObject *iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	for (i = 0; i < count; ++i) {
		elem = DeeObject_IterNext(iter);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem)
				DeeRT_ErrUnpackError(self, count, i);
			goto err_iter_result_i;
		}
		result[i] = elem; /* Inherit reference. */
	}

	/* Check to make sure that the iterator actually ends here. */
	remainder = DeeObject_InvokeMethodHint(iter_advance, iter, (size_t)-2);
	if unlikely(remainder != 0) {
		if unlikely(remainder == (size_t)-1)
			goto err_iter_result_i;
		if (OVERFLOW_UADD(remainder, count, &remainder))
			remainder = (size_t)-1;
		DeeRT_ErrUnpackError(self, count, remainder);
		goto err_iter_result_i;
	}
	Dee_Decref(iter);
	return 0;
err_iter_result_i:
	Dee_Decrefv(result, i);
/*err_iter:*/
	Dee_Decref(iter);
err:
	return -1;
}} {
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, PCKuSIZ, count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	if (DeeTuple_SIZE(resultob) != count) {
		DeeRT_ErrUnpackError(resultob, count, DeeTuple_SIZE(resultob));
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return 0;
err_r:
	Dee_Decref_likely(resultob);
err:
	return -1;
}







/* @return: * : The actual # of objects written to `result' (always in range [min_count, max_count])
 * @return: (size_t)-1: Error */
[[wunused]]
size_t __seq_unpack__.seq_unpack_ex([[nonnull]] DeeObject *__restrict self,
                                    size_t min_count, size_t max_count,
                                    [[nonnull]] /*out*/ DREF DeeObject *result[])
%{unsupported({
	return err_seq_unsupportedf(self, "__seq_unpack__(%" PRFuSIZ ", %" PRFuSIZ ")", min_count, max_count);
})}
%{$none = {
	/* "none" always turns everything into more "none", so unpack to the max # of objects. */
	Dee_Setrefv(result, Dee_None, max_count);
	return max_count;
}}
%{$empty = {
	if unlikely(min_count > 0)
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, 0);
	return 0;
}}
%{$with__tp_asvector = [[inherit_as($with__seq_operator_foreach)]] {
	size_t real_count = (*THIS_TYPE->tp_seq->tp_asvector)(self, max_count, result);
	if unlikely(real_count < min_count || real_count > max_count) {
		if (real_count < min_count)
			Dee_Decrefv(result, real_count);
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, real_count);
	}
	return real_count;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast = /* requires: __seq_getitem_always_bound__ */
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	size_t i, real_count = CALL_DEPENDENCY(seq_operator_size, self);
	ASSERTF(max_count < (size_t)-1, "If this was really true, how did you alloc that buffer?");
	if unlikely(real_count < min_count || real_count > max_count) {
		if unlikely(real_count == (size_t)-1)
			goto err;
		return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, real_count);
	}
	for (i = 0; i < real_count; ++i) {
		DREF DeeObject *item = (*getitem_index_fast)(self, i);
		if unlikely(!item) {
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if (i >= min_count)
				return i;
			Dee_Decrefv(result, i);
			return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = { /* requires: __seq_getitem_always_bound__ */
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
			Dee_Decrefv(result, i);
			if unlikely(!item)
				goto err;
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if (i >= min_count)
				return i;
			return (size_t)DeeRT_ErrUnpackErrorEx(self, min_count, max_count, i);
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = { /* requires: __seq_getitem_always_bound__ */
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
			DREF DeeObject *error;
			Dee_Decrefv(result, i);
			/* This can only mean that the size changed, because we're
			 * allowed to assume "__seq_getitem_always_bound__" */
			if ((error = DeeError_CatchError(&DeeError_IndexError)) != NULL) {
				if (i >= min_count)
					return i;
				return (size_t)DeeRT_ErrUnpackErrorExWithCause(self, min_count, max_count, i, error);
			}
			goto err;
		}
		result[i] = item; /* Inherit reference */
	}
	return real_count;
err:
	return (size_t)-1;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_unpack_with_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_unpack_with_foreach_data data;
	data.duqfd_count  = max_count;
	data.duqfd_result = result;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_unpack_with_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	ASSERT(((size_t)foreach_status == (size_t)(data.duqfd_result - result)) ||
	       ((size_t)foreach_status > max_count));
	if likely((size_t)foreach_status >= min_count && (size_t)foreach_status <= max_count)
		return (size_t)foreach_status;
	Dee_Decrefv(result, (size_t)(data.duqfd_result - result));
	DeeRT_ErrUnpackErrorEx(self, min_count, max_count, (size_t)foreach_status);
err:
	return (size_t)-1;
}}
%{$with__seq_operator_iter = {
	size_t i, remainder;
	DREF DeeObject *elem;
	DREF DeeObject *iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	for (i = 0; i < max_count; ++i) {
		elem = DeeObject_IterNext(iter);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err_iter_result_i;
			if likely(i >= min_count) {
				Dee_Decref(iter);
				return i;
			}
			DeeRT_ErrUnpackErrorEx(self, min_count, max_count, i);
			goto err_iter_result_i;
		}
		result[i] = elem; /* Inherit reference. */
	}

	/* Check to make sure that the iterator actually ends here. */
	remainder = DeeObject_InvokeMethodHint(iter_advance, iter, (size_t)-2);
	if unlikely(remainder != 0) {
		if unlikely(remainder == (size_t)-1)
			goto err_iter_result_i;
		if (OVERFLOW_UADD(remainder, max_count, &remainder))
			remainder = (size_t)-1;
		DeeRT_ErrUnpackErrorEx(self, min_count, max_count, remainder);
		goto err_iter_result_i;
	}
	Dee_Decref(iter);
	return max_count;
err_iter_result_i:
	Dee_Decrefv(result, i);
/*err_iter:*/
	Dee_Decref(iter);
err:
	return (size_t)-1;
}} {
	size_t result_count;
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, min_count, max_count);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeTuple_Type))
		goto err_r;
	result_count = DeeTuple_SIZE(resultob);
	if (result_count < min_count || result_count > max_count) {
		DeeRT_ErrUnpackErrorEx(resultob, min_count, max_count, result_count);
		goto err_r;
	}
	memcpyc(result, DeeTuple_ELEM(resultob), result_count, sizeof(DREF DeeObject *));
	DeeTuple_DecrefSymbolic(resultob);
	return result_count;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)-1;
}


seq_unpack = {
	DeeMH_seq_unpack_ex_t seq_unpack_ex = REQUIRE(seq_unpack_ex);
	if (seq_unpack_ex == &default__seq_unpack_ex__empty)
		return &$empty;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__tp_asvector)
		return &$with__tp_asvector;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_iter)
		return &$with__seq_operator_iter;
	if (seq_unpack_ex)
		return &$with__seq_unpack_ex;
};


seq_unpack_ex = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_asvector)
		return &$with__tp_asvector;

	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast &&
	    HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index &&
	    HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index &&
	    HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &$with__seq_operator_iter;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};
