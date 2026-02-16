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
/* deemon.Mapping.itervalues                                            */
/************************************************************************/
[[getset, alias(Mapping.itervalues)]]
__map_itervalues__->?DIterator;



[[wunused, getset_member("get")]] DREF DeeObject *
__map_itervalues__.map_itervalues([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = "default__seq_operator_iter__empty"}
%{$with__map_values = {
	DREF DeeObject *result, *values = CALL_DEPENDENCY(map_values, self);
	if unlikely(!values)
		goto err;
	result = DeeObject_Iter(values);
	Dee_Decref_probably_none(values);
	return result;
err:
	return NULL;
}}
%{$with__map_operator_iter = {
	DeeTypeObject *itertyp;
	DREF DefaultIterator_PairSubItem *result;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	result->dipsi_iter = CALL_DEPENDENCY(map_operator_iter, self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp            = Dee_TYPE(result->dipsi_iter);
	result->dipsi_next = DeeType_RequireNativeOperator(itertyp, nextvalue);
	DeeObject_Init(result, &DefaultIterator_WithNextValue);
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}} {
	return LOCAL_GETATTR(self);
}

map_itervalues = {
	DeeMH_map_operator_iter_t map_operator_iter;
	if (REQUIRE_NODEFAULT(map_values))
		return &$with__map_values;
	map_operator_iter = REQUIRE(map_operator_iter);
	if (map_operator_iter == &default__map_operator_iter__empty)
		return &$empty;
	if (map_operator_iter)
		return &$with__map_operator_iter;
};
