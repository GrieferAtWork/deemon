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
/* deemon.Sequence.erase()                                              */
/************************************************************************/
[[kw, alias(Sequence.erase -> "seq_erase"), declNameAlias("explicit_seq_erase")]]
__seq_erase__(index:?Dint,count=!1) {
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_count,
	                    UNPuSIZ "|" UNPuSIZ ":__seq_erase__",
	                    &index, &count))
		goto err;
	if unlikely(DeeType_InvokeMethodHint(self, seq_erase, index, count))
		goto err;
	return_none;
err:
	return NULL;
}

[[wunused]]
int __seq_erase__.seq_erase([[nonnull]] DeeObject *__restrict self,
                            size_t index, size_t count)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_operator_delrange_index = {
	size_t end_index;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	return DeeType_InvokeMethodHint(self, seq_operator_delrange_index, (Dee_ssize_t)index, (Dee_ssize_t)end_index);
err_overflow:
	return err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
}}
%{$with__seq_pop = {
	size_t end_index;
	DeeMH_seq_pop_t seq_pop;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	seq_pop = DeeType_RequireSeqPop(Dee_TYPE(self));
	while (end_index > index) {
		--end_index;
		if unlikely((*seq_pop)(self, (Dee_ssize_t)end_index))
			goto err;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return 0;
err_overflow:
	err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_erase = {
	DeeMH_seq_pop_t seq_pop;
	if (REQUIRE_NODEFAULT(seq_operator_delrange_index))
		return &$with__seq_operator_delrange_index;
	seq_pop = REQUIRE(seq_pop);
	if (seq_pop == &default__seq_pop__empty)
		return &$empty;
	if (seq_pop)
		return &$with__seq_pop;
};
