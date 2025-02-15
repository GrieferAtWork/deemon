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
/* deemon.Object.operator hash()                                        */
/************************************************************************/

operator {

/*[[export("DeeObject_{|T}Hash")]]*/ /* Requires custom handling for recursion */
[[custom_unsupported_impl_name(default__hash__unsupported)]]
[[wunused]] Dee_hash_t
tp_cmp->tp_hash([[nonnull]] DeeObject *__restrict self)
%{class {
	DREF DeeObject *func, *result;
	dhash_t result_value;
	int temp;
	func = DeeClass_TryGetOperator(THIS_TYPE, OPERATOR_HASH);
	if unlikely(!func)
		goto fallback;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if unlikely(!result)
		goto fallback_handled;
	temp = DeeObject_AsUIntptr(result, &result_value);
	Dee_Decref(result);
	if unlikely(temp)
		goto fallback_handled;
	return result_value;
fallback_handled:
	DeeError_Print("Unhandled error in `operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
fallback:
	return DeeObject_HashGeneric(self);
}}
/*%{using []: { // Not done here since that would break inheritance
	return DeeObject_HashGeneric(self);
}}*/
= OPERATOR_HASH;

} /* operator */
