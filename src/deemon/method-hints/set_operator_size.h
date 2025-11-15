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

/* NOTE: "Set.operator size()" does *NOT* count potentially unbound items! */

/************************************************************************/
/* deemon.Set.operator size()                                           */
/************************************************************************/
__set_size__()->?Dint {
	return CALL_DEPENDENCY(set_operator_sizeob, self);
}

[[operator(Set: tp_seq->tp_sizeob)]]
[[wunused]] DREF DeeObject *
__set_size__.set_operator_sizeob([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator size"))}
%{$none = return_none}
%{$empty = "default__seq_operator_sizeob__empty"}
%{using set_operator_size: {
	size_t setsize = CALL_DEPENDENCY(set_operator_size, self);
	if unlikely(setsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(setsize);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}


[[operator(Set: tp_seq->tp_size)]]
[[wunused]] size_t
__set_size__.set_operator_size([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator size"))}
%{$empty = "default__seq_operator_size__empty"}
%{$with__set_operator_foreach = [[prefix(DEFINE_default_seq_size_with_foreach_cb)]] {
	return (size_t)CALL_DEPENDENCY(set_operator_foreach, self, &default_seq_size_with_foreach_cb, NULL);
}}
%{$with__set_operator_iter = {
	size_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(set_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterAdvance(iter, (size_t)-1);
	Dee_Decref_likely(iter);
	return result;
err:
	return (size_t)-1;
}}
%{using set_operator_sizeob: {
	DREF DeeObject *sizeob;
	sizeob = CALL_DEPENDENCY(set_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsSizeDirectInherited(sizeob);
err:
	return (size_t)-1;
}} = $with__set_operator_sizeob;


set_operator_sizeob = {
	DeeMH_set_operator_size_t set_operator_size;
	if (HAS_TRAIT(__map_getitem_always_bound__)) {
		/* Method hint "map_operator_sizeob" includes */
		DeeMH_map_operator_sizeob_t map_operator_sizeob = REQUIRE_NODEFAULT(map_operator_sizeob);
		if (map_operator_sizeob) {
			if (map_operator_sizeob == &default__map_operator_sizeob__empty)
				return &$empty;
			return map_operator_sizeob;
		}
	}
	set_operator_size = REQUIRE(set_operator_size);
	if (set_operator_size == &default__set_operator_size__empty)
		return &$empty;
	if (set_operator_size)
		return &$with__set_operator_size;
};

set_operator_size = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	/* Because "map_operator_size" counts unbound items,
	 * it can only be used if there can never be any. */
	DeeMH_map_operator_size_t map_operator_size = REQUIRE_NODEFAULT(map_operator_size);
	if (map_operator_size) {
		if (map_operator_size == &default__map_operator_size__empty ||
		    map_operator_size == &default__map_operator_size__with__map_operator_foreach_pair ||
		    map_operator_size == &default__map_operator_size__with__map_operator_iter ||
		    HAS_TRAIT(__map_getitem_always_bound__))
			return map_operator_size;
	}
	if (REQUIRE_NODEFAULT(set_operator_sizeob))
		return &$with__set_operator_sizeob;
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach == &default__set_operator_foreach__with__set_operator_iter)
		return &$with__set_operator_iter;
	if (set_operator_foreach)
		return &$with__set_operator_foreach;
};
