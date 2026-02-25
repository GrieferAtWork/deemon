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
/* deemon.Object.operator bool()                                        */
/************************************************************************/

operator {

[[export("DeeObject_{|T}Bool")]]
[[export_precheck(
#ifndef __OPTIMIZE_SIZE__
#ifndef CONFIG_EXPERIMENTAL_PER_THREAD_BOOL
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
#endif /* !CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */
	if (tp_self == &DeeBool_Type)
		return DeeBool_IsTrue(self) ? 1 : 0;
#endif /* !__OPTIMIZE_SIZE__ */
)]]
[[wunused]] int
tp_cast.tp_bool([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_BOOL: {
	int retval;
	DREF DeeObject *result;
	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, self, OPERATOR_BOOL);
	if (DeeObject_AssertTypeExact(result, &DeeBool_Type))
		goto err_r;
	retval = DeeBool_IsTrue(result) ? 1 : 0;
	Dee_DecrefNokill(result);
	return retval;
err_r:
	Dee_Decref(result);
err:
	return -1;
}} = OPERATOR_BOOL;

} /* operator */
