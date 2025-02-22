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
/* deemon.Sequence.frozen                                               */
/************************************************************************/

[[getset, alias(Sequence.frozen)]]
__seq_frozen__->?O;


[[wunused, getset_member("get")]] DREF DeeObject *
__seq_frozen__.seq_frozen([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$empty = "DeeObject_NewRef"}
%{$with__seq_operator_foreach = "DeeTuple_FromSequence"}
%{$with__set_frozen = {
	/* return Set.frozen(this) as Sequence */
	DREF DeeObject *result;
	DREF DeeObject *set_frozen = CALL_DEPENDENCY(set_frozen, self);
	if unlikely(!set_frozen)
		goto err;
	result = DeeSuper_New(&DeeSeq_Type, set_frozen);
	Dee_Decref_unlikely(set_frozen);
	return result;
err:
	return NULL;
}}
%{$with__map_frozen = {
	/* return Mapping.frozen(this) as Sequence */
	DREF DeeObject *result;
	DREF DeeObject *map_frozen = CALL_DEPENDENCY(map_frozen, self);
	if unlikely(!map_frozen)
		goto err;
	result = DeeSuper_New(&DeeSeq_Type, map_frozen);
	Dee_Decref_unlikely(map_frozen);
	return result;
err:
	return NULL;
}} {
	return LOCAL_GETATTR(self);
}


seq_frozen = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (REQUIRE_NODEFAULT(set_frozen))
		return &$with__set_frozen;
	if (REQUIRE_NODEFAULT(map_frozen))
		return &$with__map_frozen;
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};
