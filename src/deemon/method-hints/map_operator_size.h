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
/* deemon.Mapping.operator size()                                       */
/************************************************************************/
__map_size__()->?Dint {
	return CALL_DEPENDENCY(map_operator_sizeob, self);
}

[[operator(Mapping: tp_seq->tp_sizeob)]] /* TODO: Allow operator init for Set, but not method hint init */
[[wunused]] DREF DeeObject *
__map_size__.map_operator_sizeob([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator size"))}
%{$none = return_none}
%{$empty = "default__seq_operator_sizeob__empty"}
%{using map_operator_size: {
	size_t mapsize = CALL_DEPENDENCY(map_operator_size, self);
	if unlikely(mapsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(mapsize);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}


[[operator(Mapping: tp_seq->tp_size)]] /* TODO: Allow operator init for Set, but not method hint init */
[[wunused]] size_t
__map_size__.map_operator_size([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator size"))}
%{$none = 0}
%{$empty = "default__seq_operator_size__empty"}
%{$with__map_operator_foreach_pair = [[prefix(DEFINE_default_seq_size_with_foreach_pair_cb)]] {
	return (size_t)CALL_DEPENDENCY(map_operator_foreach_pair, self, &default_seq_size_with_foreach_pair_cb, NULL);
}}
%{$with__map_operator_iter = {
	size_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(map_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterAdvance(iter, (size_t)-1);
	Dee_Decref_likely(iter);
	return result;
err:
	return (size_t)-1;
}}
%{using map_operator_sizeob: {
	DREF DeeObject *sizeob;
	sizeob = CALL_DEPENDENCY(map_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsSizeDirectInherited(sizeob);
err:
	return (size_t)-1;
}} = $with__map_operator_sizeob;


map_operator_sizeob = {
	DeeMH_map_operator_size_t map_operator_size = REQUIRE(map_operator_size);
	if (map_operator_size == &default__map_operator_size__empty)
		return &$empty;
	if (map_operator_size)
		return &$with__map_operator_size;
};

map_operator_size = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if (REQUIRE_NODEFAULT(map_operator_sizeob))
		return &$with__map_operator_sizeob;
	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &$empty;
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__with__map_operator_iter)
		return &$with__map_operator_iter;
	if (map_operator_foreach_pair)
		return &$with__map_operator_foreach_pair;
};
