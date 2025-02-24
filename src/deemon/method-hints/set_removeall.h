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
/* deemon.Set.removeall()                                               */
/************************************************************************/
[[alias(Set.removeall -> "set_removeall")]]
__set_removeall__(keys:?X3?DSet?DSequence?S?O) {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__set_removeall__", &keys))
		goto err;
	if unlikely(CALL_DEPENDENCY(set_removeall, self, keys))
		goto err;
	return_none;
err:
	return NULL;
}


%[define(DEFINE_set_removeall_foreach_cb_PTR =
#ifndef set_removeall_foreach_cb_PTR
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define set_removeall_foreach_cb_PTR ((Dee_ssize_t (DCALL *)(void *, DeeObject *))(Dee_funptr_t)DeeType_RequireMethodHint(Dee_TYPE(self), set_remove))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define set_removeall_foreach_cb_PTR &set_removeall_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_removeall_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(set_remove, (DeeObject *)arg, elem);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !set_removeall_foreach_cb_PTR */
)]


/* @return: 0 : Success
 * @return: -1: Error  */
[[wunused]] int
__set_removeall__.set_removeall([[nonnull]] DeeObject *self,
                                [[nonnull]] DeeObject *keys)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__set_removeall__unsupported"}
%{$with__set_operator_inplace_sub = {
	int result;
	Dee_Incref(self);
	result = CALL_DEPENDENCY(set_operator_inplace_sub, (DeeObject **)&self, keys);
	Dee_Decref(self);
	return result;
}}
%{$with__set_remove = [[prefix(DEFINE_set_removeall_foreach_cb_PTR)]] {
	Dee_ssize_t status = DeeObject_InvokeMethodHint(seq_operator_foreach, keys, set_removeall_foreach_cb_PTR, self);
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


set_removeall = {
	if (REQUIRE_NODEFAULT(set_operator_inplace_sub))
		return &$with__set_operator_inplace_sub;
	if (REQUIRE(set_remove))
		return &$with__set_remove;
};
