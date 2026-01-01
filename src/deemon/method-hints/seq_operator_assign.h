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
/* deemon.Sequence.operator := ()                                       */
/************************************************************************/
__seq_assign__(items:?X2?DSequence?S?O) {
	if unlikely(CALL_DEPENDENCY(seq_operator_assign, self, items))
		goto err;
	return_none;
err:
	return NULL;
}

[[wunused]]
[[operator([Sequence, Set, Mapping]: tp_init.tp_assign)]]
int __seq_assign__.seq_operator_assign([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *items)
%{unsupported(auto("operator :="))}
%{$none = 0}
%{$empty = {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_assign__unsupported(self, items);
err:
	return -1;
}}
%{$with__seq_operator_setrange = {
	return CALL_DEPENDENCY(seq_operator_setrange, self, Dee_None, Dee_None, items);
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}

seq_operator_assign = {
	DeeMH_seq_operator_setrange_t seq_operator_setrange;
	seq_operator_setrange = REQUIRE(seq_operator_setrange);
	if (seq_operator_setrange == &default__seq_operator_setrange__empty)
		return &$empty;
	if (seq_operator_setrange)
		return &$with__seq_operator_setrange;
};
