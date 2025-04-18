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
/* deemon.Sequence.clear()                                          */
/************************************************************************/
[[alias(Sequence.clear)]]
[[alias(Set.clear)]]
[[alias(Mapping.clear)]]
__seq_clear__() {
	if unlikely(CALL_DEPENDENCY(seq_clear, self))
		goto err;
	return_none;
err:
	return NULL;
}


[[wunused]] int
__seq_clear__.seq_clear([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_operator_delrange = {
	return CALL_DEPENDENCY(seq_operator_delrange, self, DeeInt_Zero, Dee_None);
}}
%{$with__seq_operator_delrange_index_n = {
	return CALL_DEPENDENCY(seq_operator_delrange_index_n, self, 0);
}}
%{$with__seq_erase = {
	return CALL_DEPENDENCY(seq_erase, self, 0, (size_t)-1);
}}
//TODO:%{$with__set_removeall = {
//TODO:	Set.remove(this, Set.frozen(this));
//TODO:}}
//TODO:%{$with__map_keys__and__map_removekeys = {
//TODO:	Mapping.removekeys(this, Set.frozen(Mapping.keys(this)));
//TODO:}}
{
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}

seq_clear = {
	DeeMH_seq_erase_t seq_erase = REQUIRE(seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &$empty;
	if (seq_erase == &default__seq_erase__with__seq_operator_delrange_index ||
	    seq_erase == &default__seq_erase__with__seq_pop) {
		DeeMH_seq_operator_delrange_index_n_t seq_operator_delrange_index_n = REQUIRE(seq_operator_delrange_index_n);
		if (seq_operator_delrange_index_n == &default__seq_operator_delrange_index_n__empty)
			return &$empty;
		if (seq_operator_delrange_index_n == &default__seq_operator_delrange_index_n__with__seq_operator_delrange)
			return &$with__seq_operator_delrange;
		if (seq_operator_delrange_index_n)
			return &$with__seq_operator_delrange_index_n;
	}
	if (seq_erase)
		return &$with__seq_erase;
};
