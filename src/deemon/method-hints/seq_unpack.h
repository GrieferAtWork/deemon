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
/* deemon.Sequence.unpack()                                             */
/************************************************************************/
[[alias(Sequence.unpack -> "unpack"), declNameAlias("explicit_seq_unpack")]]
__seq_unpack__(min:?Dint,max?:?Dint)->?DTuple {
	DREF DeeTupleObject *result;
	size_t min_count, max_count;
	if (argc == 1) {
		min_count = DeeObject_AsDirectSize(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
handle_single_count:
		result = DeeTuple_NewUninitialized(min_count);
		if unlikely(!result)
			goto err;
		if unlikely(CALL_DEPENDENCY(seq_unpack, self, min_count, result->t_elem))
			goto err_r;
	} else if (argc == 2) {
		min_count = DeeObject_AsDirectSize(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = DeeObject_AsDirectSize(argv[1]);
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
}

[[wunused]]
int __seq_unpack__.seq_unpack([[nonnull]] DeeObject *__restrict self, size_t count,
                              [[nonnull]] /*out*/ DREF DeeObject *__restrict result[])
%{unsupported(auto)}
%{$empty = {
	if unlikely(count != 0)
		return err_invalid_unpack_size(self, count, 0);
	return 0;
}}
%{$with__seq_unpack_ex = {
	size_t result = CALL_DEPENDENCY(seq_unpack_ex, self, count, count, result);
	if likely(result != (size_t)-1)
		result = 0;
	return (int)(Dee_ssize_t)result;
}}
%{$with__tp_asvector = {
	size_t real_count = (*THIS_TYPE->tp_seq->tp_asvector)(self, count, dst);
	if likely(real_count == count)
		return 0;
	err_invalid_unpack_size(self, count, real_count);
	if (real_count < count)
		Dee_Decrefv(dst, real_count);
	return -1;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	/* TODO: Only when __seq_getitem_always_bound__ is defined */
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	/* TODO: Only when __seq_getitem_always_bound__ is defined */
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	/* TODO: Only when __seq_getitem_always_bound__ is defined */
}}
%{$with__seq_operator_foreach = {
	/* TODO */
}} {
	// TODO
}


[[wunused]]
size_t __seq_unpack__.seq_unpack_ex([[nonnull]] DeeObject *__restrict self,
                                    size_t min_count, size_t max_count,
                                    [[nonnull]] /*out*/ DREF DeeObject *__restrict result[])
%{unsupported(auto)}
%{$empty = {
	if unlikely(min_count > 0)
		return (size_t)err_invalid_unpack_size_minmax(self, min_count, max_count, 0);
	return 0;
}}
%{$with__tp_asvector = {
	/* TODO */
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	/* TODO: Only when __seq_getitem_always_bound__ is defined */
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	/* TODO: Only when __seq_getitem_always_bound__ is defined */
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	/* TODO: Only when __seq_getitem_always_bound__ is defined */
}}
%{$with__seq_operator_foreach = {
	/* TODO */
}} {
	// TODO
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
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};
