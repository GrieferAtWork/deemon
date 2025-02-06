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
/* deemon.Set.operator hash()                                           */
/************************************************************************/
__set_hash__()->?Dint {
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__set_hash__"))
		goto err;
	result = DeeType_InvokeMethodHint0(self, set_operator_hash);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

%[define(DEFINE_default_set_hash_with_foreach_cb =
#ifndef DEFINED_default_set_hash_with_foreach_cb
#define DEFINED_default_set_hash_with_foreach_cb
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_hash_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_set_hash_with_foreach_cb */
)]

%[define(DEFINE_DeeSet_HandleHashError =
#ifndef DEFINED_DeeSet_HandleHashError
#define DEFINED_DeeSet_HandleHashError
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
#endif /* !DEFINED_DeeSet_HandleHashError */
)]

[[operator([Set, Mapping].OPERATOR_HASH: tp_cmp->tp_hash)]]
[[wunused]] Dee_hash_t
__set_hash__.set_operator_hash([[nonnull]] DeeObject *self)
%{unsupported_alias("default__seq_operator_hash__unsupported")}
%{$empty = DEE_HASHOF_EMPTY_SEQUENCE}
%{$with__set_operator_foreach =
[[prefix(DEFINE_default_set_hash_with_foreach_cb)]]
[[prefix(DEFINE_DeeSet_HandleHashError)]] {
	Dee_hash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	if unlikely(DeeType_InvokeMethodHint(self, set_operator_foreach, &default_set_hash_with_foreach_cb, &result))
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}}
[[prefix(DEFINE_DeeSet_HandleHashError)]] {
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
	return DeeSet_HandleHashError(self);
}

set_operator_hash = {
	DeeMH_set_operator_foreach_t set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return $with__set_operator_foreach;
};
