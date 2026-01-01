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
/* deemon.Object.operator str()                                         */
/************************************************************************/

operator {

%[define(DEFINE_instance_call_with_file_writer =
#ifndef DEFINED_instance_call_with_file_writer
#define DEFINED_instance_call_with_file_writer
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
instance_call_with_file_writer(DeeObject *self, DeeObject *func) {
	DREF DeeObject *result, *status;
	DREF DeeObject *writer = DeeFile_OpenWriter();
	if unlikely(!writer)
		goto err;
	status = DeeObject_ThisCall(func, self, 1, &writer);
	if unlikely(!status)
		goto err_writer;
	Dee_Decref(status);
	result = DeeFileWriter_GetString(writer);
	Dee_Decref(writer);
	return result;
err_writer:
	Dee_Decref(writer);
err:
	return NULL;
}
#endif /* !DEFINED_instance_call_with_file_writer */
)]

/*[[export("DeeObject_{|T}Str")]]*/ /* Requires custom handling for recursion */
[[wunused]] DREF DeeObject *
tp_cast.tp_str([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_STR: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, self, OPERATOR_STR);
	if (DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{class using CLASS_OPERATOR_PRINT:
	[[prefix(DEFINE_instance_call_with_file_writer)]]
{
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(THIS_TYPE, CLASS_OPERATOR_PRINT);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_writer(self, func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}}
%{using tp_cast.tp_print: {
	Dee_ssize_t status;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	status = CALL_DEPENDENCY(tp_cast.tp_print, self, &unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}} = OPERATOR_STR;



%[define(DEFINE_instance_call_with_file_printer =
#ifndef DEFINED_instance_call_with_file_printer
#define DEFINED_instance_call_with_file_printer
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
instance_call_with_file_printer(DeeObject *self, DeeObject *func,
                                Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *status;
	DREF DeeObject *printer_file;
	if (printer == (Dee_formatprinter_t)&DeeFile_WriteAll) {
		status = DeeObject_ThisCall(func, self, 1, (DeeObject **)&arg);
		if unlikely(!status)
			goto err;
		Dee_Decref(status);
		/* XXX: Returning `0' here is technically wrong; we'd need to
		 *      return the total sum of bytes written to file, but then
		 *      again: this should also be good enough (and the specs
		 *      require us to pass along the original file in this case)
		 * >> import * from deemon;
		 * >> class MyClass {
		 * >>     operator str(fp: File) {
		 * >>         assert fp === File.stdout;
		 * >>         fp << "Hello!";
		 * >>     }
		 * >> }
		 * >> print MyClass();
		 */
		return 0;
	}

	printer_file = DeeFile_OpenPrinter(printer, arg);
	if unlikely(!printer_file)
		goto err;
	status = DeeObject_ThisCall(func, self, 1, &printer_file);
	if unlikely(!status)
		goto err_printer_file;
	Dee_Decref(status);
	return (Dee_ssize_t)DeeFile_ClosePrinter(printer_file);
err_printer_file:
	Dee_Decref(printer_file);
err:
	return -1;
}
#endif /* !DEFINED_instance_call_with_file_printer */
)]


/*[[export("DeeObject_{|T}Print")]]*/ /* Requires custom handling for recursion */
[[wunused]] Dee_ssize_t
tp_cast.tp_print([[nonnull]] DeeObject *__restrict self,
                 [[nonnull]] Dee_formatprinter_t printer, void *arg)
%{class using OPERATOR_STR:{
	Dee_ssize_t result;
	DREF DeeObject *strval = IF_TYPED_ELSE(tusrtype__str__with__STR(tp_self, self),
	                                       usrtype__str__with__STR(self));
	if unlikely(!strval)
		goto err;
	result = DeeObject_Print(strval, printer, arg);
	Dee_Decref(strval);
	return result;
err:
	return -1;
}}
%{class using CLASS_OPERATOR_PRINT:
	[[prefix(DEFINE_instance_call_with_file_printer)]]
{
	Dee_ssize_t result;
	DREF DeeObject *func = DeeClass_GetOperator(THIS_TYPE, CLASS_OPERATOR_PRINT);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_printer(self, func, printer, arg);
	Dee_Decref(func);
	return result;
err:
	return -1;
}}
%{using tp_cast.tp_str: {
	Dee_ssize_t result;
	DREF DeeObject *str = CALL_DEPENDENCY(tp_cast.tp_str, self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
}} = OPERATOR_STR;


} /* operator */
