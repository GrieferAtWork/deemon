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
%{$none = return_none}
%{$empty = "DeeObject_NewRef"}
%{$with__seq_operator_foreach = {
	struct Dee_tuple_builder builder;
	size_t hint = DeeObject_SizeFast(self);
	if (hint != (size_t)-1) {
		Dee_tuple_builder_init_with_hint(&builder, hint);
	} else {
		Dee_tuple_builder_init(&builder);
	}
	if unlikely(CALL_DEPENDENCY(seq_operator_foreach, self, &Dee_tuple_builder_append, &builder))
		goto err_builder;
	return Dee_tuple_builder_pack(&builder);
err_builder:
	Dee_tuple_builder_fini(&builder);
	return NULL;
}}
%{$with__seq_enumerate_index = {
	struct Dee_nullable_tuple_builder builder;
	size_t hint = DeeObject_SizeFast(self);
	if (hint != (size_t)-1) {
		Dee_nullable_tuple_builder_init_with_hint(&builder, hint);
	} else {
		Dee_nullable_tuple_builder_init(&builder);
	}
	if unlikely(CALL_DEPENDENCY(seq_enumerate_index, self,
	                            &Dee_nullable_tuple_builder_setitem_index,
	                            &builder, 0, (size_t)-1))
		goto err_builder;
	return Dee_nullable_tuple_builder_pack(&builder);
err_builder:
	Dee_tuple_builder_fini(&builder);
	return NULL;
}}
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
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (REQUIRE_NODEFAULT(set_frozen))
		return &$with__set_frozen;
	if (REQUIRE_NODEFAULT(map_frozen))
		return &$with__map_frozen;
	seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_iter ||
	    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_foreach__and__counter)
		return &$with__seq_operator_foreach;
	if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_enumerate) {
		DeeMH_seq_enumerate_t seq_enumerate = REQUIRE(seq_enumerate);
		if (seq_enumerate == &default__seq_enumerate__empty)
			return &$empty;
		if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter)
			return &$with__seq_operator_foreach;
	}
	if (seq_enumerate_index) {
		if (DeeType_HasTraitHint(THIS_TYPE, __seq_getitem_always_bound__))
			return &$with__seq_operator_foreach;
		return &$with__seq_enumerate_index;
	}
};
