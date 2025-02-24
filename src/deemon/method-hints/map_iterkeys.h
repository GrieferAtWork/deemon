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
/* deemon.Mapping.iterkeys                                              */
/************************************************************************/
[[getset, alias(Mapping.iterkeys)]]
__map_iterkeys__->?DIterator;



[[wunused, getset_member("get")]] DREF DeeObject *
__map_iterkeys__.map_iterkeys([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$empty = "default__seq_operator_iter__empty"}
%{$none = return_none}
%{$with__map_keys = {
	DREF DeeObject *result, *keys = CALL_DEPENDENCY(map_keys, self);
	if unlikely(!keys)
		goto err;
	result = DeeObject_Iter(keys);
	Dee_Decref_unlikely(keys); /* *_unlikely because it's probably referenced by the iterator */
	return result;
err:
	return NULL;
}}
%{$with__map_enumerate = {
	/* TODO: Custom iterator type that uses "tp_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}}
%{$with__map_operator_iter = {
	/* NOTE: This only works when the mapping can't have unbound keys! */
	DeeTypeObject *itertyp;
	DREF DefaultIterator_PairSubItem *result;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	result->dipsi_iter = CALL_DEPENDENCY(map_operator_iter, self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp            = Dee_TYPE(result->dipsi_iter);
	result->dipsi_next = DeeType_RequireNativeOperator(itertyp, nextkey);
	DeeObject_Init(result, &DefaultIterator_WithNextKey);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}} {
	return LOCAL_GETATTR(self);
}

map_iterkeys = {
	DeeMH_map_enumerate_t map_enumerate;
	if (REQUIRE_NODEFAULT(map_keys))
		return &$with__map_keys;
	map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
	if (DeeType_HasTraitHint(THIS_TYPE, __map_getitem_always_bound__) ||
	    map_enumerate == REQUIRE(map_operator_foreach_pair)) {
		DeeMH_map_operator_iter_t map_operator_iter = REQUIRE(map_operator_iter);
		if (map_operator_iter == &default__map_operator_iter__empty)
			return &$empty;
		if (map_operator_iter)
			return &$with__map_operator_iter;
	}
	if (map_enumerate)
		return &$with__map_enumerate;
};
