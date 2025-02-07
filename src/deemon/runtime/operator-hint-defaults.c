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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_DEFAULTS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_DEFAULTS_C 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/filetypes.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

/**/
#include "runtime_error.h"

DECL_BEGIN


#ifdef __OPTIMIZE_SIZE__
#define return_DeeClass_CallOperator(tp_self, self, operator, argc, argv) \
	return DeeClass_CallOperator(tp_self, self, operator, argc, argv)
#define return_DeeClass_CallOperator2(err, tp_self, self, operator, argc, argv) \
	return DeeClass_CallOperator(tp_self, self, operator, argc, argv)
#define store_DeeClass_CallOperator(err, result, tp_self, self, operator, argc, argv)                \
	do {                                                                                             \
		if unlikely(((result) = DeeClass_CallOperator(tp_self, self, operator, argc, argv)) == NULL) \
			goto err;                                                                                \
	}	__WHILE0
#else /* __OPTIMIZE_SIZE__ */
#define return_DeeClass_CallOperator(tp_self, self, operator, argc, argv) \
	do {                                                                  \
		DREF DeeObject *_func, *_result;                                  \
		_func = DeeClass_GetOperator(tp_self, operator);                  \
		if unlikely(!_func)                                               \
			goto err;                                                     \
		_result = DeeObject_ThisCall(_func, self, argc, argv);            \
		Dee_Decref_unlikely(_func);                                       \
		return _result;                                                   \
	err:                                                                  \
		return NULL;                                                      \
	}	__WHILE0
#define return_DeeClass_CallOperator2(err, tp_self, self, operator, argc, argv) \
	do {                                                                        \
		DREF DeeObject *_func, *_result;                                        \
		_func = DeeClass_GetOperator(tp_self, operator);                        \
		if unlikely(!_func)                                                     \
			goto err;                                                           \
		_result = DeeObject_ThisCall(_func, self, argc, argv);                  \
		Dee_Decref_unlikely(_func);                                             \
		return _result;                                                         \
	}	__WHILE0
#define store_DeeClass_CallOperator(err, result, tp_self, self, operator, argc, argv) \
	do {                                                                              \
		DREF DeeObject *_func;                                                        \
		_func = DeeClass_GetOperator(tp_self, operator);                              \
		if unlikely(!_func)                                                           \
			goto err;                                                                 \
		(result) = DeeObject_ThisCall(_func, self, argc, argv);                       \
		Dee_Decref_unlikely(_func);                                                   \
		if unlikely(!(result))                                                        \
			goto err;                                                                 \
	}	__WHILE0
#endif /* !__OPTIMIZE_SIZE__ */


/* clang-format off */
/*[[[deemon (printNativeOperatorHintImpls from "..method-hints.method-hints")();]]]*/
/* tp_cast.tp_str */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__str(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_STR, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

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
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__str__by_print(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, CLASS_OPERATOR_PRINT);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_writer(self, func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__str__with__print(DeeTypeObject *tp_self, DeeObject *self) {
	Dee_ssize_t status;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	status = (*tp_self->tp_cast.tp_print)(self, &unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__str(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_cast.tp_str)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__str(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__str(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_STR, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

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
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__str__by_print(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__str__by_print(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(Dee_TYPE(self), CLASS_OPERATOR_PRINT);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_writer(self, func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__str__with__print(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__str__with__print(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t status;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	status = (*Dee_TYPE(self)->tp_cast.tp_print)(self, &unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cast.tp_print */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tusrtype__print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *strval = tusrtype__str(tp_self, self);
	if unlikely(!strval)
		goto err;
	result = DeeObject_Print(strval, printer, arg);
	Dee_Decref(strval);
	return result;
err:
	return -1;
}

#ifndef DEFINED_instance_call_with_file_printer
#define DEFINED_instance_call_with_file_printer
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
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
	return (dssize_t)DeeFile_ClosePrinter(printer_file);
err_printer_file:
	Dee_Decref(printer_file);
err:
	return -1;
}
#endif /* !DEFINED_instance_call_with_file_printer */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tusrtype__print__by_print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *func = DeeClass_GetOperator(tp_self, CLASS_OPERATOR_PRINT);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_printer(self, func, printer, arg);
	Dee_Decref(func);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__print__with__str(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *str = (*tp_self->tp_cast.tp_str)(self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	return (*tp_self->tp_cast.tp_print)(self, printer, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
usrtype__print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__print(Dee_TYPE(self), self, printer, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *strval = usrtype__str(self);
	if unlikely(!strval)
		goto err;
	result = DeeObject_Print(strval, printer, arg);
	Dee_Decref(strval);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

#ifndef DEFINED_instance_call_with_file_printer
#define DEFINED_instance_call_with_file_printer
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
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
	return (dssize_t)DeeFile_ClosePrinter(printer_file);
err_printer_file:
	Dee_Decref(printer_file);
err:
	return -1;
}
#endif /* !DEFINED_instance_call_with_file_printer */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
usrtype__print__by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__print__by_print(Dee_TYPE(self), self, printer, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *func = DeeClass_GetOperator(Dee_TYPE(self), CLASS_OPERATOR_PRINT);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_printer(self, func, printer, arg);
	Dee_Decref(func);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__print__with__str(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__print__with__str(Dee_TYPE(self), self, printer, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *str = (*Dee_TYPE(self)->tp_cast.tp_str)(self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cast.tp_repr */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__repr(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_REPR, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

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
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__repr__by_printrepr(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, CLASS_OPERATOR_PRINTREPR);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_writer(self, func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__repr__with__printrepr(DeeTypeObject *tp_self, DeeObject *self) {
	Dee_ssize_t status;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	status = (*tp_self->tp_cast.tp_printrepr)(self, &unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__repr(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_cast.tp_repr)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__repr(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__repr(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_REPR, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

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
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__repr__by_printrepr(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__repr__by_printrepr(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(Dee_TYPE(self), CLASS_OPERATOR_PRINTREPR);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_writer(self, func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__repr__with__printrepr(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__repr__with__printrepr(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t status;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	status = (*Dee_TYPE(self)->tp_cast.tp_printrepr)(self, &unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cast.tp_printrepr */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tusrtype__printrepr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *strval = tusrtype__repr(tp_self, self);
	if unlikely(!strval)
		goto err;
	result = DeeObject_Print(strval, printer, arg);
	Dee_Decref(strval);
	return result;
err:
	return -1;
}

#ifndef DEFINED_instance_call_with_file_printer
#define DEFINED_instance_call_with_file_printer
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
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
	return (dssize_t)DeeFile_ClosePrinter(printer_file);
err_printer_file:
	Dee_Decref(printer_file);
err:
	return -1;
}
#endif /* !DEFINED_instance_call_with_file_printer */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tusrtype__printrepr__by_print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *func = DeeClass_GetOperator(tp_self, CLASS_OPERATOR_PRINTREPR);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_printer(self, func, printer, arg);
	Dee_Decref(func);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__printrepr__with__repr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *str = (*tp_self->tp_cast.tp_repr)(self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__printrepr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg) {
	return (*tp_self->tp_cast.tp_printrepr)(self, printer, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
usrtype__printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__printrepr(Dee_TYPE(self), self, printer, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *strval = usrtype__repr(self);
	if unlikely(!strval)
		goto err;
	result = DeeObject_Print(strval, printer, arg);
	Dee_Decref(strval);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

#ifndef DEFINED_instance_call_with_file_printer
#define DEFINED_instance_call_with_file_printer
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
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
	return (dssize_t)DeeFile_ClosePrinter(printer_file);
err_printer_file:
	Dee_Decref(printer_file);
err:
	return -1;
}
#endif /* !DEFINED_instance_call_with_file_printer */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
usrtype__printrepr__by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__printrepr__by_print(Dee_TYPE(self), self, printer, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *func = DeeClass_GetOperator(Dee_TYPE(self), CLASS_OPERATOR_PRINTREPR);
	if unlikely(!func)
		goto err;
	result = instance_call_with_file_printer(self, func, printer, arg);
	Dee_Decref(func);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__printrepr__with__repr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__printrepr__with__repr(Dee_TYPE(self), self, printer, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *str = (*Dee_TYPE(self)->tp_cast.tp_repr)(self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cast.tp_bool */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__bool(DeeTypeObject *tp_self, DeeObject *self) {
	int retval;
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_BOOL, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeBool_Type))
		goto err_r;
	retval = DeeBool_IsTrue(result);
	Dee_DecrefNokill(result);
	return retval;
err_r:
	Dee_Decref(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bool(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_cast.tp_bool)(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__bool(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__bool(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int retval;
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_BOOL, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeBool_Type))
		goto err_r;
	retval = DeeBool_IsTrue(result);
	Dee_DecrefNokill(result);
	return retval;
err_r:
	Dee_Decref(result);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_call */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_CALL, argc, argv);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__call__with__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv) {
	return (*tp_self->tp_call_kw)(self, argc, argv, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv) {
	return (*tp_self->tp_call)(self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__call(DeeObject *self, size_t argc, DeeObject *const *argv) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__call(Dee_TYPE(self), self, argc, argv);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_CALL, argc, argv);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__call__with__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__call__with__call_kw(Dee_TYPE(self), self, argc, argv);
#else /* __OPTIMIZE_SIZE__ */
	return (*Dee_TYPE(self)->tp_call_kw)(self, argc, argv, NULL);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_call_kw */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_CALL);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	Dee_Decref_unlikely(func);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__call_kw__with__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_length;
			kw_length = DeeObject_Size(kw);
			if unlikely(kw_length == (size_t)-1)
				goto err;
			if (kw_length != 0)
				goto err_no_keywords;
		}
	}
	return (*tp_self->tp_call)(self, argc, argv);
err_no_keywords:
	err_keywords_not_accepted(tp_self, kw);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return (*tp_self->tp_call_kw)(self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__call_kw(Dee_TYPE(self), self, argc, argv, kw);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(Dee_TYPE(self), OPERATOR_CALL);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	Dee_Decref_unlikely(func);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__call_kw__with__call(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__call_kw__with__call(Dee_TYPE(self), self, argc, argv, kw);
#else /* __OPTIMIZE_SIZE__ */
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_length;
			kw_length = DeeObject_Size(kw);
			if unlikely(kw_length == (size_t)-1)
				goto err;
			if (kw_length != 0)
				goto err_no_keywords;
		}
	}
	return (*Dee_TYPE(self)->tp_call)(self, argc, argv);
err_no_keywords:
	err_keywords_not_accepted(Dee_TYPE(self), kw);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_iter_next */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__iter_next(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(tp_self, OPERATOR_ITERNEXT, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func;
	func = DeeClass_GetOperator(tp_self, OPERATOR_ITERNEXT);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result) {
		if (!DeeError_Catch(&DeeError_StopIteration))
			goto err;
		result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__iter_next__with__nextpair(DeeTypeObject *tp_self, DeeObject *self) {
	int error;
	DREF DeeObject *key_and_value[2];
	error = (*tp_self->tp_iterator->tp_nextpair)(self, key_and_value);
	if (error == 0) {
		DREF DeeTupleObject *result;
		result = DeeTuple_NewUninitializedPair();
		if unlikely(!result)
			goto err_key_and_value;
		result->t_elem[0] = key_and_value[0]; /* Inherit reference */
		result->t_elem[1] = key_and_value[1]; /* Inherit reference */
		return (DREF DeeObject *)result;
	}
	if likely(error > 0)
		return ITER_DONE;
err:
	return NULL;
err_key_and_value:
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__iter_next(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_iter_next)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__iter_next(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__iter_next(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(Dee_TYPE(self), OPERATOR_ITERNEXT, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func;
	func = DeeClass_GetOperator(Dee_TYPE(self), OPERATOR_ITERNEXT);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result) {
		if (!DeeError_Catch(&DeeError_StopIteration))
			goto err;
		result = ITER_DONE;
	}
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__iter_next__with__nextpair(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__iter_next__with__nextpair(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int error;
	DREF DeeObject *key_and_value[2];
	error = (*Dee_TYPE(self)->tp_iterator->tp_nextpair)(self, key_and_value);
	if (error == 0) {
		DREF DeeTupleObject *result;
		result = DeeTuple_NewUninitializedPair();
		if unlikely(!result)
			goto err_key_and_value;
		result->t_elem[0] = key_and_value[0]; /* Inherit reference */
		result->t_elem[1] = key_and_value[1]; /* Inherit reference */
		return (DREF DeeObject *)result;
	}
	if likely(error > 0)
		return ITER_DONE;
err:
	return NULL;
err_key_and_value:
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	goto err;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_iterator->tp_nextpair */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__nextpair__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject *key_and_value[2]) {
	int result;
	DREF DeeObject *item = (*tp_self->tp_iter_next)(self);
	if (item == ITER_DONE)
		return 1;
	if unlikely(item == NULL)
		goto err;
	result = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__nextpair(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject *key_and_value[2]) {
	return (*tp_self->tp_iterator->tp_nextpair)(self, key_and_value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__nextpair__with__iter_next(DeeObject *__restrict self, DREF DeeObject *__restrict key_and_value[2]) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__nextpair__with__iter_next(Dee_TYPE(self), self, key_and_value);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *item = (*Dee_TYPE(self)->tp_iter_next)(self);
	if (item == ITER_DONE)
		return 1;
	if unlikely(item == NULL)
		goto err;
	result = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_iterator->tp_nextkey */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__nextkey__with__iter_next(DeeTypeObject *tp_self, DeeObject *self) {
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = (*tp_self->tp_iter_next)(self);
	if unlikely(!ITER_ISOK(item))
		return item;
	unpack_status = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	if unlikely(unpack_status)
		goto err;
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__nextkey__with__nextpair(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *key_and_value[2];
	int status = (*tp_self->tp_iterator->tp_nextpair)(self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__nextkey(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_iterator->tp_nextkey)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__nextkey__with__iter_next(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__nextkey__with__iter_next(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = (*Dee_TYPE(self)->tp_iter_next)(self);
	if unlikely(!ITER_ISOK(item))
		return item;
	unpack_status = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	if unlikely(unpack_status)
		goto err;
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__nextkey__with__nextpair(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__nextkey__with__nextpair(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *key_and_value[2];
	int status = (*Dee_TYPE(self)->tp_iterator->tp_nextpair)(self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_iterator->tp_nextvalue */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__nextvalue__with__iter_next(DeeTypeObject *tp_self, DeeObject *self) {
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = (*tp_self->tp_iter_next)(self);
	if unlikely(!ITER_ISOK(item))
		return item;
	unpack_status = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	if unlikely(unpack_status)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__nextvalue__with__nextpair(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *key_and_value[2];
	int status = (*tp_self->tp_iterator->tp_nextpair)(self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__nextvalue(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_iterator->tp_nextvalue)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__nextvalue__with__iter_next(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__nextvalue__with__iter_next(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = (*Dee_TYPE(self)->tp_iter_next)(self);
	if unlikely(!ITER_ISOK(item))
		return item;
	unpack_status = DeeObject_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	if unlikely(unpack_status)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__nextvalue__with__nextpair(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__nextvalue__with__nextpair(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *key_and_value[2];
	int status = (*Dee_TYPE(self)->tp_iterator->tp_nextpair)(self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_iterator->tp_advance */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__nextkey(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	;
	do {
		DREF DeeObject *elem = (*tp_self->tp_iterator->tp_nextkey)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__nextvalue(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	;
	do {
		DREF DeeObject *elem = (*tp_self->tp_iterator->tp_nextvalue)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__nextpair(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	;
	do {
		DREF DeeObject *key_and_value[2];
		int error = (*tp_self->tp_iterator->tp_nextpair)(self, key_and_value);
		if unlikely(error != 0) {
			if unlikely(error < 0)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	;
	do {
		DREF DeeObject *elem = (*tp_self->tp_iter_next)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	return (*tp_self->tp_iterator->tp_advance)(self, step);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__advance__with__nextkey(DeeObject *__restrict self, size_t step) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__advance__with__nextkey(Dee_TYPE(self), self, step);
#else /* __OPTIMIZE_SIZE__ */
	size_t result = 0;
	;
	do {
		DREF DeeObject *elem = (*Dee_TYPE(self)->tp_iterator->tp_nextkey)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__advance__with__nextvalue(DeeObject *__restrict self, size_t step) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__advance__with__nextvalue(Dee_TYPE(self), self, step);
#else /* __OPTIMIZE_SIZE__ */
	size_t result = 0;
	;
	do {
		DREF DeeObject *elem = (*Dee_TYPE(self)->tp_iterator->tp_nextvalue)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__advance__with__nextpair(DeeObject *__restrict self, size_t step) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__advance__with__nextpair(Dee_TYPE(self), self, step);
#else /* __OPTIMIZE_SIZE__ */
	size_t result = 0;
	;
	do {
		DREF DeeObject *key_and_value[2];
		int error = (*Dee_TYPE(self)->tp_iterator->tp_nextpair)(self, key_and_value);
		if unlikely(error != 0) {
			if unlikely(error < 0)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__advance__with__iter_next(DeeObject *__restrict self, size_t step) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__advance__with__iter_next(Dee_TYPE(self), self, step);
#else /* __OPTIMIZE_SIZE__ */
	size_t result = 0;
	;
	do {
		DREF DeeObject *elem = (*Dee_TYPE(self)->tp_iter_next)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	} while (result < step);
	return result;
err:
	return (size_t)-1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_int */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__int(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_INT, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeInt_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__int__with__int64(DeeTypeObject *tp_self, DeeObject *self) {
	int64_t value;
	int status = (*tp_self->tp_math->tp_int64)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt64((uint64_t)value);
	return DeeInt_NewInt64(value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__int__with__int32(DeeTypeObject *tp_self, DeeObject *self) {
	int32_t value;
	int status = (*tp_self->tp_math->tp_int32)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt32((uint32_t)value);
	return DeeInt_NewInt32(value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__int__with__double(DeeTypeObject *tp_self, DeeObject *self) {
	double value;
	int status = (*tp_self->tp_math->tp_double)(self, &value);
	if unlikely(status < 0)
		goto err;
	return DeeInt_NewDouble(value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__int(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_math->tp_int)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__int(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__int(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_INT, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeInt_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__int__with__int64(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int__with__int64(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int64_t value;
	int status = (*Dee_TYPE(self)->tp_math->tp_int64)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt64((uint64_t)value);
	return DeeInt_NewInt64(value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__int__with__int32(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int__with__int32(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	int32_t value;
	int status = (*Dee_TYPE(self)->tp_math->tp_int32)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt32((uint32_t)value);
	return DeeInt_NewInt32(value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__int__with__double(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int__with__double(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	double value;
	int status = (*Dee_TYPE(self)->tp_math->tp_double)(self, &value);
	if unlikely(status < 0)
		goto err;
	return DeeInt_NewDouble(value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_int32 */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int32__with__int64(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result) {
	int64_t value;
	int status = (*tp_self->tp_math->tp_int64)(self, &value);
	if (status == Dee_INT_UNSIGNED) {
		if ((uint64_t)value > UINT32_MAX && !(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*p_result = (int32_t)(uint32_t)(uint64_t)value;
	} else {
		if ((value < INT32_MIN || value > INT32_MAX) && status == Dee_INT_SIGNED &&
		    !(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*p_result = (int32_t)value;
	}
	return status;
err_overflow:
	return err_integer_overflow(self, 32, status == Dee_INT_SIGNED);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int32__with__int(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result) {
	int status;
	DREF DeeObject *temp = (*tp_self->tp_math->tp_int)(self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get32Bit(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int32__with__double(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result) {
	double value;
	int status = (*tp_self->tp_math->tp_double)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT32_MIN && !(tp_self->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT32_MAX) {
		if (value <= UINT32_MAX) {
			*p_result = (int32_t)(uint32_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*p_result = (int32_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 32, value >= 0);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int32(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result) {
	return (*tp_self->tp_math->tp_int32)(self, p_result);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__int32__with__int64(DeeObject *__restrict self, int32_t *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int32__with__int64(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int64_t value;
	int status = (*Dee_TYPE(self)->tp_math->tp_int64)(self, &value);
	if (status == Dee_INT_UNSIGNED) {
		if ((uint64_t)value > UINT32_MAX && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*p_result = (int32_t)(uint32_t)(uint64_t)value;
	} else {
		if ((value < INT32_MIN || value > INT32_MAX) && status == Dee_INT_SIGNED &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*p_result = (int32_t)value;
	}
	return status;
err_overflow:
	return err_integer_overflow(self, 32, status == Dee_INT_SIGNED);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__int32__with__int(DeeObject *__restrict self, int32_t *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int32__with__int(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int status;
	DREF DeeObject *temp = (*Dee_TYPE(self)->tp_math->tp_int)(self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get32Bit(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__int32__with__double(DeeObject *__restrict self, int32_t *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int32__with__double(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	double value;
	int status = (*Dee_TYPE(self)->tp_math->tp_double)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT32_MIN && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT32_MAX) {
		if (value <= UINT32_MAX) {
			*p_result = (int32_t)(uint32_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*p_result = (int32_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 32, value >= 0);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_int64 */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int64__with__int32(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result) {
	int32_t value;
	int status = (*tp_self->tp_math->tp_int32)(self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (int64_t)(uint64_t)(uint32_t)value;
	} else {
		*p_result = (int64_t)value;
	}
	return status;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int64__with__int(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result) {
	int status;
	DREF DeeObject *temp = (*tp_self->tp_math->tp_int)(self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get64Bit(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int64__with__double(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result) {
	double value;
	int status = (*tp_self->tp_math->tp_double)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT64_MIN && !(tp_self->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT64_MAX) {
		if (value <= UINT64_MAX) {
			*p_result = (int64_t)(uint64_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*p_result = (int64_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 64, value >= 0);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__int64(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result) {
	return (*tp_self->tp_math->tp_int64)(self, p_result);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__int64__with__int32(DeeObject *__restrict self, int64_t *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int64__with__int32(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int32_t value;
	int status = (*Dee_TYPE(self)->tp_math->tp_int32)(self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (int64_t)(uint64_t)(uint32_t)value;
	} else {
		*p_result = (int64_t)value;
	}
	return status;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__int64__with__int(DeeObject *__restrict self, int64_t *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int64__with__int(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int status;
	DREF DeeObject *temp = (*Dee_TYPE(self)->tp_math->tp_int)(self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get64Bit(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__int64__with__double(DeeObject *__restrict self, int64_t *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__int64__with__double(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	double value;
	int status = (*Dee_TYPE(self)->tp_math->tp_double)(self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT64_MIN && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT64_MAX) {
		if (value <= UINT64_MAX) {
			*p_result = (int64_t)(uint64_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*p_result = (int64_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 64, value >= 0);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_double */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__double(DeeTypeObject *tp_self, DeeObject *self, double *p_result) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_FLOAT, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeFloat_Type))
		goto err_r;
	*p_result = DeeFloat_VALUE(result);
	Dee_Decref_likely(result);
	return 0;
err_r:
	Dee_Decref(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__double__with__int(DeeTypeObject *tp_self, DeeObject *self, double *p_result) {
	int status;
	DREF DeeObject *temp = (*tp_self->tp_math->tp_int)(self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_AsDouble(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__double__with__int64(DeeTypeObject *tp_self, DeeObject *self, double *p_result) {
	int64_t intval;
	int status = (*tp_self->tp_math->tp_int64)(self, &intval);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (double)(uint64_t)intval;
		status = 0;
	} else {
		*p_result = (double)intval;
	}
	return status;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__double__with__int32(DeeTypeObject *tp_self, DeeObject *self, double *p_result) {
	int32_t intval;
	int status = (*tp_self->tp_math->tp_int32)(self, &intval);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (double)(uint32_t)intval;
		status = 0;
	} else {
		*p_result = (double)intval;
	}
	return status;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__double(DeeTypeObject *tp_self, DeeObject *self, double *p_result) {
	return (*tp_self->tp_math->tp_double)(self, p_result);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__double(DeeObject *__restrict self, double *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__double(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_FLOAT, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeFloat_Type))
		goto err_r;
	*p_result = DeeFloat_VALUE(result);
	Dee_Decref_likely(result);
	return 0;
err_r:
	Dee_Decref(result);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__double__with__int(DeeObject *__restrict self, double *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__double__with__int(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int status;
	DREF DeeObject *temp = (*Dee_TYPE(self)->tp_math->tp_int)(self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_AsDouble(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__double__with__int64(DeeObject *__restrict self, double *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__double__with__int64(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int64_t intval;
	int status = (*Dee_TYPE(self)->tp_math->tp_int64)(self, &intval);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (double)(uint64_t)intval;
		status = 0;
	} else {
		*p_result = (double)intval;
	}
	return status;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__double__with__int32(DeeObject *__restrict self, double *__restrict p_result) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__double__with__int32(Dee_TYPE(self), self, p_result);
#else /* __OPTIMIZE_SIZE__ */
	int32_t intval;
	int status = (*Dee_TYPE(self)->tp_math->tp_int32)(self, &intval);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (double)(uint32_t)intval;
		status = 0;
	} else {
		*p_result = (double)intval;
	}
	return status;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_iter */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__iter(DeeTypeObject *tp_self, DeeObject *self) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_ITER, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__iter__with__foreach(DeeTypeObject *tp_self, DeeObject *self) {
	/* TODO: Custom iterator type that uses "tp_foreach" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__iter__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self) {
	/* TODO: Custom iterator type that uses "tp_foreach_pair" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__iter(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_seq->tp_iter)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__iter(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__iter(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_ITER, 0, NULL);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__iter__with__foreach(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__iter__with__foreach(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	/* TODO: Custom iterator type that uses "tp_foreach" */
	(void)Dee_TYPE(self);
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__iter__with__foreach_pair(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__iter__with__foreach_pair(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	/* TODO: Custom iterator type that uses "tp_foreach_pair" */
	(void)Dee_TYPE(self);
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_foreach */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = (*tp_self->tp_seq->tp_iter)(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_foreach_pair_data *data;
	Dee_ssize_t result;
	DREF DeeTupleObject *pair;
	data = (struct default_foreach_with_foreach_pair_data *)arg;
	pair = DeeTuple_NewUninitializedPair();
	if unlikely(!pair)
		goto err;
	pair->t_elem[0] = key;   /* Symbolic reference */
	pair->t_elem[1] = value; /* Symbolic reference */
	result = (*data->dfwfp_cb)(data->dfwfp_arg, (DeeObject *)pair);
	DeeTuple_DecrefSymbolic((DREF DeeObject *)pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_foreach_pair_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return (*tp_self->tp_seq->tp_foreach_pair)(self, &default_foreach_with_foreach_pair_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	return (*tp_self->tp_seq->tp_foreach)(self, cb, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__foreach__with__iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__foreach__with__iter(Dee_TYPE(self), self, cb, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *iter = (*Dee_TYPE(self)->tp_seq->tp_iter)(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_foreach_pair_data *data;
	Dee_ssize_t result;
	DREF DeeTupleObject *pair;
	data = (struct default_foreach_with_foreach_pair_data *)arg;
	pair = DeeTuple_NewUninitializedPair();
	if unlikely(!pair)
		goto err;
	pair->t_elem[0] = key;   /* Symbolic reference */
	pair->t_elem[1] = value; /* Symbolic reference */
	result = (*data->dfwfp_cb)(data->dfwfp_arg, (DeeObject *)pair);
	DeeTuple_DecrefSymbolic((DREF DeeObject *)pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_foreach_pair_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__foreach__with__foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__foreach__with__foreach_pair(Dee_TYPE(self), self, cb, arg);
#else /* __OPTIMIZE_SIZE__ */
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return (*Dee_TYPE(self)->tp_seq->tp_foreach_pair)(self, &default_foreach_with_foreach_pair_cb, &data);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_foreach_pair */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach_pair__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = (*tp_self->tp_seq->tp_iter)(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_foreach_pair_with_foreach_data *data;
	Dee_ssize_t result;
	data = (struct default_foreach_pair_with_foreach_data *)arg;
	if likely(DeeTuple_Check(elem) && DeeTuple_SIZE(elem) == 2) {
		result = (*data->dfpwf_cb)(data->dfpwf_arg,
		                             DeeTuple_GET(elem, 0),
		                             DeeTuple_GET(elem, 1));
	} else {
		DREF DeeObject *pair[2];
		if unlikely(DeeObject_Unpack(elem, 2, pair))
			goto err;
		result = (*data->dfpwf_cb)(data->dfpwf_arg, pair[0], pair[1]);
		Dee_Decref_unlikely(pair[1]);
		Dee_Decref_unlikely(pair[0]);
	}
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_pair_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach_pair__with__foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return (*tp_self->tp_seq->tp_foreach)(self, &default_foreach_pair_with_foreach_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	return (*tp_self->tp_seq->tp_foreach_pair)(self, cb, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__foreach_pair__with__iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__foreach_pair__with__iter(Dee_TYPE(self), self, cb, arg);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t result;
	DREF DeeObject *iter = (*Dee_TYPE(self)->tp_seq->tp_iter)(self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_foreach_pair_with_foreach_data *data;
	Dee_ssize_t result;
	data = (struct default_foreach_pair_with_foreach_data *)arg;
	if likely(DeeTuple_Check(elem) && DeeTuple_SIZE(elem) == 2) {
		result = (*data->dfpwf_cb)(data->dfpwf_arg,
		                             DeeTuple_GET(elem, 0),
		                             DeeTuple_GET(elem, 1));
	} else {
		DREF DeeObject *pair[2];
		if unlikely(DeeObject_Unpack(elem, 2, pair))
			goto err;
		result = (*data->dfpwf_cb)(data->dfpwf_arg, pair[0], pair[1]);
		Dee_Decref_unlikely(pair[1]);
		Dee_Decref_unlikely(pair[0]);
	}
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_pair_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__foreach_pair__with__foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__foreach_pair__with__foreach(Dee_TYPE(self), self, cb, arg);
#else /* __OPTIMIZE_SIZE__ */
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &default_foreach_pair_with_foreach_cb, &data);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inv */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__inv(DeeTypeObject *tp_self, DeeObject *self) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_INV, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__inv(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_math->tp_inv)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__inv(DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inv(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_INV, 0, NULL);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_pos */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__pos(DeeTypeObject *tp_self, DeeObject *self) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_POS, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__pos(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_math->tp_pos)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__pos(DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__pos(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_POS, 0, NULL);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_neg */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__neg(DeeTypeObject *tp_self, DeeObject *self) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_NEG, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__neg(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_math->tp_neg)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__neg(DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__neg(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_NEG, 0, NULL);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_add */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_ADD, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_add)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__add(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__add(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_ADD, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_add */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_ADD, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_add__with__add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_add)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_add)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_add(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_add(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_ADD, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_add__with__add(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_add__with__add(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_add)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_sub */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_SUB, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_sub)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__sub(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__sub(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_SUB, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_sub */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_SUB, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_sub__with__sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_sub)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_sub)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_sub(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_sub(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_SUB, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_sub__with__sub(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_sub__with__sub(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_sub)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_mul */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_MUL, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_mul)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__mul(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__mul(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_MUL, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_mul */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_MUL, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_mul__with__mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_mul)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_mul)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_mul(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_mul(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_MUL, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_mul__with__mul(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_mul__with__mul(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_mul)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_div */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_DIV, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_div)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__div(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__div(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_DIV, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_div */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_DIV, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_div__with__div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_div)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_div)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_div(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_div(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_DIV, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_div__with__div(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_div__with__div(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_div)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_mod */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_MOD, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_mod)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__mod(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__mod(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_MOD, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_mod */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_MOD, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_mod__with__mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_mod)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_mod)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_mod(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_mod(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_MOD, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_mod__with__mod(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_mod__with__mod(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_mod)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_shl */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_SHL, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_shl)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__shl(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__shl(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_SHL, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_shl */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_SHL, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_shl__with__shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_shl)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_shl)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_shl(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_shl(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_SHL, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_shl__with__shl(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_shl__with__shl(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_shl)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_shr */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_SHR, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_shr)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__shr(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__shr(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_SHR, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_shr */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_SHR, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_shr__with__shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_shr)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_shr)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_shr(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_shr(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_SHR, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_shr__with__shr(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_shr__with__shr(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_shr)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_and */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_AND, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_and)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__and(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__and(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_AND, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_and */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_AND, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_and__with__and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_and)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_and)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_and(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_and(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_AND, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_and__with__and(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_and__with__and(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_and)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_or */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_OR, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_or)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__or(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__or(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_OR, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_or */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_OR, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_or__with__or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_or)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_or)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_or(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_or(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_OR, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_or__with__or(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_or__with__or(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_or)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_xor */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_XOR, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_xor)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__xor(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__xor(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_XOR, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_xor */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_XOR, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_xor__with__xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_xor)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_xor)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_xor(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_xor(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_XOR, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_xor__with__xor(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_xor__with__xor(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_xor)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_pow */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_POW, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_pow)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__pow(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__pow(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_POW, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_pow */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_lhs, OPERATOR_INPLACE_POW, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_pow__with__pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_pow)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->tp_math->tp_inplace_pow)(p_lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__inplace_pow(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inplace_pow(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_lhs), *p_lhs, OPERATOR_INPLACE_POW, 1, &rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__inplace_pow__with__pow(DeeObject **__restrict p_lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inplace_pow__with__pow(Dee_TYPE(*p_lhs), p_lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_lhs)->tp_math->tp_pow)(*p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inc */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__inc(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_self, OPERATOR_INC, 0, NULL);
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__inc__with__inplace_add(DeeTypeObject *tp_self, DeeObject **p_self) {
	return (*tp_self->tp_math->tp_inplace_add)(p_self, DeeInt_One);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__inc__with__add(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_add)(*p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__inc(DeeTypeObject *tp_self, DeeObject **p_self) {
	return (*tp_self->tp_math->tp_inc)(p_self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__inc(DeeObject **__restrict p_self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__inc(Dee_TYPE(*p_self), p_self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_self), *p_self, OPERATOR_INC, 0, NULL);
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__inc__with__inplace_add(DeeObject **__restrict p_self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inc__with__inplace_add(Dee_TYPE(*p_self), p_self);
#else /* __OPTIMIZE_SIZE__ */
	return (*Dee_TYPE(*p_self)->tp_math->tp_inplace_add)(p_self, DeeInt_One);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__inc__with__add(DeeObject **__restrict p_self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__inc__with__add(Dee_TYPE(*p_self), p_self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_self)->tp_math->tp_add)(*p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_math->tp_dec */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__dec(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, *p_self, OPERATOR_DEC, 0, NULL);
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__dec__with__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_self) {
	return (*tp_self->tp_math->tp_inplace_sub)(p_self, DeeInt_One);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__dec__with__sub(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result = (*tp_self->tp_math->tp_sub)(*p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__dec(DeeTypeObject *tp_self, DeeObject **p_self) {
	return (*tp_self->tp_math->tp_dec)(p_self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__dec(DeeObject **__restrict p_self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__dec(Dee_TYPE(*p_self), p_self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(*p_self), *p_self, OPERATOR_DEC, 0, NULL);
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__dec__with__inplace_sub(DeeObject **__restrict p_self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__dec__with__inplace_sub(Dee_TYPE(*p_self), p_self);
#else /* __OPTIMIZE_SIZE__ */
	return (*Dee_TYPE(*p_self)->tp_math->tp_inplace_sub)(p_self, DeeInt_One);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__dec__with__sub(DeeObject **__restrict p_self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__dec__with__sub(Dee_TYPE(*p_self), p_self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(*p_self)->tp_math->tp_sub)(*p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_with->tp_enter */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__enter(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_ENTER, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__enter(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_with->tp_enter)(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__enter(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__enter(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_ENTER, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_with->tp_leave */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__leave(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_LEAVE, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__leave(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_with->tp_leave)(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__leave(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__leave(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_LEAVE, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}
/*[[[end]]]*/
/* clang-format on */


DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_DEFAULTS_C */
