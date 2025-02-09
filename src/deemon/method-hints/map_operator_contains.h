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
/* deemon.Mapping.operator contains()                                   */
/************************************************************************/
__map_contains__(key)->?Dbool {
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_contains__", &key))
		goto err;
	return CALL_DEPENDENCY(map_operator_contains, self, key);
err:
	return NULL;
}

[[operator(Mapping.OPERATOR_CONTAINS: tp_seq->tp_contains)]]
[[wunused]] DREF DeeObject *
__map_contains__.map_operator_contains([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto("operator contains"))}
%{$empty = return_false}
%{$with__map_operator_trygetitem = {
	DREF DeeObject *value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (value == ITER_DONE)
		return_false;
	if (value) {
		Dee_Decref(value);
		return_true;
	}
	return NULL;
}}
%{$with__map_operator_bounditem = {
	int result = CALL_DEPENDENCY(map_operator_bounditem, self, key);
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	return_bool_(Dee_BOUND_ISBOUND(result));
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &key);
}

map_operator_contains = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
	DeeMH_map_operator_bounditem_t map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem) {
		if (map_operator_bounditem == &default__map_operator_bounditem__empty)
			return &$empty;
		if (map_operator_bounditem != &default__map_operator_bounditem__with__map_operator_getitem)
			return &$with__map_operator_bounditem;
	}
	map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};
