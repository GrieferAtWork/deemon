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
/* deemon.Mapping.update()                                              */
/************************************************************************/
[[alias(Mapping.update -> "map_update"), declNameAlias("explicit_map_update")]]
__map_update__(items:?X3?DMapping?M?O?O?S?T2?O?O) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__map_update__", &items))
		goto err;
	if unlikely(CALL_DEPENDENCY(map_update, self, items))
		goto err;
	return_none;
err:
	return NULL;
}


%[define(DEFINE_map_update_foreach_cb_PTR =
#ifndef map_update_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define map_update_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *, DeeObject *))(Dee_funptr_t)&DeeObject_SetItem)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define map_update_foreach_cb_PTR &map_update_foreach_cb
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
map_update_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	return (Dee_ssize_t)DeeObject_SetItem((DeeObject *)arg, key, value);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !map_update_foreach_cb_PTR */
)]


/* Copy all key-value pairs from `items' and assign them to `self'.
 * Same as `for (local key, value: items) self[key] = value;'
 * @return: 0 : Success
 * @return: -1: Error */
[[wunused]] int
__map_update__.map_update([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *items)
%{unsupported(auto)}
%{$empty = {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__map_update__unsupported(self, items);
err:
	return -1;
}}
%{$with__map_operator_inplace_add = {
	int result;
	Dee_Incref(self);
	result = CALL_DEPENDENCY(map_operator_inplace_add, &self, items);
	Dee_Decref(self);
	return result;
}}
%{$with__map_operator_setitem = [[prefix(DEFINE_map_update_foreach_cb_PTR)]] {
	return (int)DeeObject_InvokeMethodHint(seq_operator_foreach_pair, items, map_update_foreach_cb_PTR, self);
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}

map_update = {
	DeeMH_map_operator_setitem_t map_operator_setitem;
	if (REQUIRE_NODEFAULT(map_operator_inplace_add))
		return &$with__map_operator_inplace_add;
	map_operator_setitem = REQUIRE(map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &$empty;
	if (map_operator_setitem)
		return &$with__map_operator_setitem;
};
