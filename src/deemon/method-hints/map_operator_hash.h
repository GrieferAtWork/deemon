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
/* deemon.Mapping.operator hash()                                       */
/************************************************************************/
__map_hash__()->?Dint {
	Dee_hash_t result = CALL_DEPENDENCY(map_operator_hash, self);
	return DeeInt_NewHash(result);
}

%[define(DEFINE_map_handle_hash_error =
#ifndef DEFINED_map_handle_hash_error
#define DEFINED_map_handle_hash_error
PRIVATE NONNULL((1)) Dee_hash_t DCALL
map_handle_hash_error(DeeObject *self) {
	DeeError_Print("Unhandled error in `Mapping.operator hash'",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINED_map_handle_hash_error */
)]

%[define(DEFINE_default_map_hash_with_foreach_pair_cb =
#ifndef DEFINED_default_map_hash_with_foreach_pair_cb
#define DEFINED_default_map_hash_with_foreach_pair_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_hash_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	*(Dee_hash_t *)arg ^= Dee_HashCombine(DeeObject_Hash(key),
	                                      DeeObject_Hash(value));
	return 0;
}
#endif /* !DEFINED_default_map_hash_with_foreach_pair_cb */
)]

[[operator(Mapping: tp_cmp->tp_hash)]]
[[wunused]] Dee_hash_t
__map_hash__.map_operator_hash([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias("default__seq_operator_hash__unsupported")}
%{$none = 0}
%{$empty = DEE_HASHOF_EMPTY_SEQUENCE}
%{$with__map_operator_foreach_pair =
[[prefix(DEFINE_default_map_hash_with_foreach_pair_cb)]]
[[prefix(DEFINE_map_handle_hash_error)]] {
	Dee_hash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	if unlikely(CALL_DEPENDENCY(map_operator_foreach_pair, self,
	                            &default_map_hash_with_foreach_pair_cb,
	                            &result))
		goto err;
	return result;
err:
	return map_handle_hash_error(self);
}}
[[prefix(DEFINE_map_handle_hash_error)]] {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return map_handle_hash_error(self);
}

map_operator_hash = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &$empty;
	if (map_operator_foreach_pair)
		return &$with__map_operator_foreach_pair;
};
