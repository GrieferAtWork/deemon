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
/* deemon.Sequence.pushfront()                                          */
/************************************************************************/
[[alias(Sequence.pushfront)]]
__seq_pushfront__(item) {
	if unlikely(CALL_DEPENDENCY(seq_pushfront, self, item))
		goto err;
	return_none;
err:
	return NULL;
}


[[wunused]]
int __seq_pushfront__.seq_pushfront([[nonnull]] DeeObject *self,
                                    [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__seq_pushfront__unsupported"}
%{$with__seq_insert = {
	return CALL_DEPENDENCY(seq_insert, self, 0, item);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_pushfront = {
	DeeMH_seq_insert_t seq_insert = REQUIRE(seq_insert);
	if (seq_insert == &default__seq_insert__empty)
		return &$empty;
	if (seq_insert)
		return &$with__seq_insert;
};
