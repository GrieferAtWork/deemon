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
/* deemon.Object.operator repr()                                        */
/************************************************************************/

operator {

[[wunused]] DREF DeeObject *
tp_cast.tp_repr([[nonnull]] DeeObject *__restrict self)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_REPR, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{class by_printrepr: [[prefix(DEFINE_instance_call_with_file_writer)]] {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(THIS_TYPE, CLASS_OPERATOR_PRINTREPR);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_writer(self, func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}}
%{using tp_cast.tp_printrepr: {
	Dee_ssize_t status;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	status = CALL_DEPENDENCY(tp_cast.tp_printrepr, self, &unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}};




[[wunused]] Dee_ssize_t
tp_cast.tp_printrepr([[nonnull]] DeeObject *__restrict self,
                     [[nonnull]] Dee_formatprinter_t printer, void *arg)
%{class {
	Dee_ssize_t result;
	DREF DeeObject *strval = IF_TYPED_ELSE(tusrtype__repr(tp_self, self), usrtype__repr(self));
	if unlikely(!strval)
		goto err;
	result = DeeObject_Print(strval, printer, arg);
	Dee_Decref(strval);
	return result;
err:
	return -1;
}}
%{class by_print: [[prefix(DEFINE_instance_call_with_file_printer)]] {
	Dee_ssize_t result;
	DREF DeeObject *func = DeeClass_GetOperator(THIS_TYPE, CLASS_OPERATOR_PRINTREPR);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_printer(self, func, printer, arg);
	Dee_Decref(func);
	return result;
err:
	return -1;
}}
%{using tp_cast.tp_repr: {
	Dee_ssize_t result;
	DREF DeeObject *str = CALL_DEPENDENCY(tp_cast.tp_repr, self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
}};


} /* operator */
