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
/* deemon.Iterator.operator bool()                                      */
/************************************************************************/
__iter_bool__()->?Dbool {
	int result = CALL_DEPENDENCY(iter_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

[[wunused]]
[[operator([Iterator]: tp_cast.tp_bool)]]
int __iter_bool__.iter_operator_bool([[nonnull]] DeeObject *__restrict self)
%{unsupported(err_iter_unsupportedf(self, "operator bool"))}
%{$empty = 0}
%{$with__iter_peek = {
	DREF DeeObject *next = CALL_DEPENDENCY(iter_peek, self);
	if unlikely(!next)
		goto err;
	if (next == ITER_DONE)
		return 0;
	Dee_Decref_unlikely(next);
	return 1;
err:
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!result)
		goto err;
	if (DeeObject_AssertTypeExact(result, &DeeBool_Type))
		goto err_r;
	Dee_DecrefNokill(result);
	return DeeBool_IsTrue(result);
err_r:
	Dee_Decref(result);
err:
	return -1;
}

iter_operator_bool = {
	DeeMH_iter_peek_t iter_peek = REQUIRE(iter_peek);
	if (iter_peek == &default__iter_peek__empty)
		return &$empty;
	if (iter_peek)
		return &$with__iter_peek;
};
