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
/* deemon.Object.operator move:=()                                      */
/************************************************************************/

operator {

/* TODO: This operator can only be inherited into types with the "TP_FINHERITCTOR" flag set! */
[[export("DeeObject_{|T}MoveAssign")]]
[[wunused]] int
tp_init.tp_move_assign([[nonnull]] DeeObject *self,
                       [[nonnull]] DeeObject *value)
%{class using OPERATOR_MOVEASSIGN: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, self, OPERATOR_MOVEASSIGN, value);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}}
%{using tp_init.tp_assign: { /* TODO: Directly alias */
	return CALL_DEPENDENCY(tp_init.tp_assign, self, value);
}} = OPERATOR_MOVEASSIGN;

} /* operator */
