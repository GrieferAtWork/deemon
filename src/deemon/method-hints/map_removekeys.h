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
/* deemon.Mapping.removekeys()                                          */
/************************************************************************/
[[alias(Mapping.removekeys)]]
__map_removekeys__(keys:?X3?DSet?DSequence?S?O) {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_removekeys__", &keys))
		goto err;
	if unlikely(CALL_DEPENDENCY(map_removekeys, self, keys))
		goto err;
	return_none;
err:
	return NULL;
}


%[define(DEFINE_map_removekeys_foreach_cb_PTR =
#ifndef map_removekeys_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define map_removekeys_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireMethodHint(Dee_TYPE(self), map_remove))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define map_removekeys_foreach_cb_PTR &map_removekeys_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
map_removekeys_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(map_remove, (DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !map_removekeys_foreach_cb_PTR */
)]


/* Delete all keys that appear in `keys'.
 * Same as `for (local key: keys) del self[key];'
 * @return: 0 : Success
 * @return: -1: Error */
[[wunused]] int
__map_removekeys__.map_removekeys([[nonnull]] DeeObject *self,
                                  [[nonnull]] DeeObject *keys)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__map_removekeys__unsupported"}
%{$with__map_operator_inplace_sub = {
	int result;
	Dee_Incref(self);
	result = CALL_DEPENDENCY(map_operator_inplace_sub, (DeeObject **)&self, keys);
	Dee_Decref(self);
	return result;
}}
%{$with__map_remove = [[prefix(DEFINE_map_removekeys_foreach_cb_PTR)]] {
	Dee_ssize_t status = DeeObject_InvokeMethodHint(seq_operator_foreach, keys, map_removekeys_foreach_cb_PTR, self);
	return likely(status >= 0) ? 0 : -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}


map_removekeys = {
	if (REQUIRE_NODEFAULT(map_operator_inplace_sub))
		return &$with__map_operator_inplace_sub;
	if (REQUIRE(map_remove))
		return &$with__map_remove;
};
