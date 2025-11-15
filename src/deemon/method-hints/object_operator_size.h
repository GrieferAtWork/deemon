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
/* deemon.Object.operator size()                                        */
/************************************************************************/

operator {


[[export("DeeObject_{|T}SizeOb")]]
[[wunused]] DREF DeeObject *
tp_seq->tp_sizeob([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_SIZE: {
	return_DeeClass_CallOperator_NoArgs(THIS_TYPE, self, OPERATOR_SIZE);
}}
%{using tp_seq->tp_size: {
	size_t result = CALL_DEPENDENCY(tp_seq->tp_size, self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}} = OPERATOR_SIZE;


[[export("DeeObject_{|T}Size")]]
[[wunused]] size_t
tp_seq->tp_size([[nonnull]] DeeObject *__restrict self)
%{using tp_seq->tp_sizeob: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_sizeob, self);
	if unlikely(!result)
		goto err;
	return DeeObject_AsSizeDirectInherited(result);
err:
	return (size_t)-1;
}} = OPERATOR_SIZE;


[[export("DeeObject_{|T}SizeFast")]]
[[custom_unsupported_impl_name(default__size_fast__with__)]]
[[custom_badalloc_impl_name(default__size_fast__with__)]]
[[wunused]] size_t
tp_seq->tp_size_fast([[nonnull]] DeeObject *__restrict self)
%{using []: {
	(void)self;
	return (size_t)-1; /* Not fast */
}} = OPERATOR_SIZE;



} /* operator */
