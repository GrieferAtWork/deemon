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
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

/**/
#include "runtime_error.h"

#define do_fix_negative_range_index(index, size) \
	((size) - ((size_t)(-(index)) % (size)))

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
/* tp_init.tp_assign */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_ASSIGN, 1, &value);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	return (*tp_self->tp_init.tp_assign)(self, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__assign(DeeObject *self, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__assign(Dee_TYPE(self), self, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_ASSIGN, 1, &value);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_init.tp_move_assign */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__move_assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_MOVEASSIGN, 1, &value);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__move_assign__with__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) { /* TODO: Directly alias */
	return (*(tp_self->tp_init.tp_assign == &usrtype__assign ? &tusrtype__assign : &tdefault__assign))(tp_self, self, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__move_assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	return (*tp_self->tp_init.tp_move_assign)(self, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__move_assign(DeeObject *self, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__move_assign(Dee_TYPE(self), self, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_MOVEASSIGN, 1, &value);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__move_assign__with__assign(DeeObject *self, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__move_assign__with__assign(Dee_TYPE(self), self, value);
#else /* __OPTIMIZE_SIZE__ */
	/* TODO: Directly alias */
	return (*Dee_TYPE(self)->tp_init.tp_assign)(self, value);
#endif /* __OPTIMIZE_SIZE__ */
}

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
	status = (*(tp_self->tp_cast.tp_print == &usrtype__print ? &tusrtype__print : tp_self->tp_cast.tp_print == &usrtype__print__by_print ? &tusrtype__print__by_print : &tdefault__print))(tp_self, self, &unicode_printer_print, &printer);
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
	DREF DeeObject *str = (*(tp_self->tp_cast.tp_str == &usrtype__str ? &tusrtype__str : tp_self->tp_cast.tp_str == &usrtype__str__by_print ? &tusrtype__str__by_print : &tdefault__str))(tp_self, self);
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
	status = (*(tp_self->tp_cast.tp_printrepr == &usrtype__printrepr ? &tusrtype__printrepr : tp_self->tp_cast.tp_printrepr == &usrtype__printrepr__by_print ? &tusrtype__printrepr__by_print : &tdefault__printrepr))(tp_self, self, &unicode_printer_print, &printer);
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
	DREF DeeObject *str = (*(tp_self->tp_cast.tp_repr == &usrtype__repr ? &tusrtype__repr : tp_self->tp_cast.tp_repr == &usrtype__repr__by_printrepr ? &tusrtype__repr__by_printrepr : &tdefault__repr))(tp_self, self);
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
	return (*(tp_self->tp_call_kw == &usrtype__call_kw ? &tusrtype__call_kw : &tdefault__call_kw))(tp_self, self, argc, argv, NULL);
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
	return (*(tp_self->tp_call == &usrtype__call ? &tusrtype__call : &tdefault__call))(tp_self, self, argc, argv);
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
	DREF DeeObject *item = (*(tp_self->tp_iter_next == &usrtype__iter_next ? &tusrtype__iter_next : &tdefault__iter_next))(tp_self, self);
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
default__nextpair__with__iter_next(DeeObject *__restrict self, DREF DeeObject *key_and_value[2]) {
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
	DREF DeeObject *item = (*(tp_self->tp_iter_next == &usrtype__iter_next ? &tusrtype__iter_next : tp_self->tp_iter_next == &default__iter_next__with__nextpair ? &tdefault__iter_next__with__nextpair : &tdefault__iter_next))(tp_self, self);
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
	int status = (*(tp_self->tp_iterator->tp_nextpair == &default__nextpair__with__iter_next ? &tdefault__nextpair__with__iter_next : &tdefault__nextpair))(tp_self, self, key_and_value);
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
	DREF DeeObject *item = (*(tp_self->tp_iter_next == &usrtype__iter_next ? &tusrtype__iter_next : tp_self->tp_iter_next == &default__iter_next__with__nextpair ? &tdefault__iter_next__with__nextpair : &tdefault__iter_next))(tp_self, self);
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
	int status = (*(tp_self->tp_iterator->tp_nextpair == &default__nextpair__with__iter_next ? &tdefault__nextpair__with__iter_next : &tdefault__nextpair))(tp_self, self, key_and_value);
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
	
	while (result < step) {
		DREF DeeObject *elem = (*(tp_self->tp_iterator->tp_nextkey == &default__nextkey__with__iter_next ? &tdefault__nextkey__with__iter_next : tp_self->tp_iterator->tp_nextkey == &default__nextkey__with__nextpair ? &tdefault__nextkey__with__nextpair : &tdefault__nextkey))(tp_self, self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__nextvalue(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	
	while (result < step) {
		DREF DeeObject *elem = (*(tp_self->tp_iterator->tp_nextvalue == &default__nextvalue__with__iter_next ? &tdefault__nextvalue__with__iter_next : tp_self->tp_iterator->tp_nextvalue == &default__nextvalue__with__nextpair ? &tdefault__nextvalue__with__nextpair : &tdefault__nextvalue))(tp_self, self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__nextpair(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	
	while (result < step) {
		DREF DeeObject *key_and_value[2];
		int error = (*(tp_self->tp_iterator->tp_nextpair == &default__nextpair__with__iter_next ? &tdefault__nextpair__with__iter_next : &tdefault__nextpair))(tp_self, self, key_and_value);
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
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__advance__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	size_t result = 0;
	
	while (result < step) {
		DREF DeeObject *elem = (*(tp_self->tp_iter_next == &usrtype__iter_next ? &tusrtype__iter_next : tp_self->tp_iter_next == &default__iter_next__with__nextpair ? &tdefault__iter_next__with__nextpair : &tdefault__iter_next))(tp_self, self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
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
	
	while (result < step) {
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
	}
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
	
	while (result < step) {
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
	}
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
	
	while (result < step) {
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
	}
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
	
	while (result < step) {
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
	}
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
	int status = (*(tp_self->tp_math->tp_int64 == &default__int64__with__int32 ? &tdefault__int64__with__int32 : tp_self->tp_math->tp_int64 == &default__int64__with__double ? &tdefault__int64__with__double : &tdefault__int64))(tp_self, self, &value);
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
	int status = (*(tp_self->tp_math->tp_int32 == &default__int32__with__int64 ? &tdefault__int32__with__int64 : tp_self->tp_math->tp_int32 == &default__int32__with__double ? &tdefault__int32__with__double : &tdefault__int32))(tp_self, self, &value);
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
	int status = (*(tp_self->tp_math->tp_double == &usrtype__double ? &tusrtype__double : tp_self->tp_math->tp_double == &default__double__with__int64 ? &tdefault__double__with__int64 : tp_self->tp_math->tp_double == &default__double__with__int32 ? &tdefault__double__with__int32 : &tdefault__double))(tp_self, self, &value);
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
	int status = (*(tp_self->tp_math->tp_int64 == &default__int64__with__int ? &tdefault__int64__with__int : tp_self->tp_math->tp_int64 == &default__int64__with__double ? &tdefault__int64__with__double : &tdefault__int64))(tp_self, self, &value);
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
	DREF DeeObject *temp = (*(tp_self->tp_math->tp_int == &usrtype__int ? &tusrtype__int : tp_self->tp_math->tp_int == &default__int__with__int64 ? &tdefault__int__with__int64 : tp_self->tp_math->tp_int == &default__int__with__double ? &tdefault__int__with__double : &tdefault__int))(tp_self, self);
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
	int status = (*(tp_self->tp_math->tp_double == &usrtype__double ? &tusrtype__double : tp_self->tp_math->tp_double == &default__double__with__int ? &tdefault__double__with__int : tp_self->tp_math->tp_double == &default__double__with__int64 ? &tdefault__double__with__int64 : &tdefault__double))(tp_self, self, &value);
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
	int status = (*(tp_self->tp_math->tp_int32 == &default__int32__with__int ? &tdefault__int32__with__int : tp_self->tp_math->tp_int32 == &default__int32__with__double ? &tdefault__int32__with__double : &tdefault__int32))(tp_self, self, &value);
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
	DREF DeeObject *temp = (*(tp_self->tp_math->tp_int == &usrtype__int ? &tusrtype__int : tp_self->tp_math->tp_int == &default__int__with__int32 ? &tdefault__int__with__int32 : tp_self->tp_math->tp_int == &default__int__with__double ? &tdefault__int__with__double : &tdefault__int))(tp_self, self);
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
	int status = (*(tp_self->tp_math->tp_double == &usrtype__double ? &tusrtype__double : tp_self->tp_math->tp_double == &default__double__with__int ? &tdefault__double__with__int : tp_self->tp_math->tp_double == &default__double__with__int32 ? &tdefault__double__with__int32 : &tdefault__double))(tp_self, self, &value);
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
	DREF DeeObject *temp = (*(tp_self->tp_math->tp_int == &usrtype__int ? &tusrtype__int : tp_self->tp_math->tp_int == &default__int__with__int64 ? &tdefault__int__with__int64 : tp_self->tp_math->tp_int == &default__int__with__int32 ? &tdefault__int__with__int32 : &tdefault__int))(tp_self, self);
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
	int status = (*(tp_self->tp_math->tp_int64 == &default__int64__with__int32 ? &tdefault__int64__with__int32 : tp_self->tp_math->tp_int64 == &default__int64__with__int ? &tdefault__int64__with__int : &tdefault__int64))(tp_self, self, &intval);
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
	int status = (*(tp_self->tp_math->tp_int32 == &default__int32__with__int64 ? &tdefault__int32__with__int64 : tp_self->tp_math->tp_int32 == &default__int32__with__int ? &tdefault__int32__with__int : &tdefault__int32))(tp_self, self, &intval);
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

/* tp_cmp->tp_hash */
INTERN WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL
tusrtype__hash(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *func, *result;
	dhash_t result_value;
	int temp;
	func = DeeClass_TryGetOperator(tp_self, OPERATOR_HASH);
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
}

INTERN WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL
tdefault__hash(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_cmp->tp_hash)(self);
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
usrtype__hash(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__hash(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func, *result;
	dhash_t result_value;
	int temp;
	func = DeeClass_TryGetOperator(Dee_TYPE(self), OPERATOR_HASH);
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
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_compare_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare_eq__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) { // TODO: Special handling in linker: directly alias
	return (*(tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare))(tp_self, lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare_eq__with__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*(tp_self->tp_cmp->tp_eq == &usrtype__eq ? &tusrtype__eq : &tdefault__eq))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare_eq__with__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *cmp_ob = (*(tp_self->tp_cmp->tp_ne == &usrtype__ne ? &tusrtype__ne : &tdefault__ne))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare_eq__with__lo__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob = (*(tp_self->tp_cmp->tp_lo == &usrtype__lo ? &tusrtype__lo : tp_self->tp_cmp->tp_lo == &default__lo__with__compare ? &tdefault__lo__with__compare : &tdefault__lo))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = (*(tp_self->tp_cmp->tp_gr == &usrtype__gr ? &tusrtype__gr : tp_self->tp_cmp->tp_gr == &default__gr__with__compare ? &tdefault__gr__with__compare : &tdefault__gr))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare_eq__with__le__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob = (*(tp_self->tp_cmp->tp_le == &usrtype__le ? &tusrtype__le : tp_self->tp_cmp->tp_le == &default__le__with__compare ? &tdefault__le__with__compare : &tdefault__le))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = (*(tp_self->tp_cmp->tp_gr == &usrtype__gr ? &tusrtype__gr : tp_self->tp_cmp->tp_gr == &default__gr__with__compare ? &tdefault__gr__with__compare : &tdefault__gr))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return -1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_compare_eq)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare_eq__with__compare(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare_eq__with__compare(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	// TODO: Special handling in linker: directly alias
	return (*Dee_TYPE(lhs)->tp_cmp->tp_compare)(lhs, rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare_eq__with__eq(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare_eq__with__eq(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_eq)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare_eq__with__ne(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare_eq__with__ne(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ne)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare_eq__with__lo__and__gr(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare_eq__with__lo__and__gr(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_lo)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_gr)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare_eq__with__le__and__ge(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare_eq__with__le__and__ge(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_le)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_gr)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return -1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_compare */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__eq__and__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_eq == &usrtype__eq ? &tusrtype__eq : tp_self->tp_cmp->tp_eq == &default__eq__with__compare_eq ? &tdefault__eq__with__compare_eq : &tdefault__eq))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_lo == &usrtype__lo ? &tusrtype__lo : &tdefault__lo))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__eq__and__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_eq == &usrtype__eq ? &tusrtype__eq : tp_self->tp_cmp->tp_eq == &default__eq__with__compare_eq ? &tdefault__eq__with__compare_eq : &tdefault__eq))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_le == &usrtype__le ? &tusrtype__le : &tdefault__le))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__eq__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_eq == &usrtype__eq ? &tusrtype__eq : tp_self->tp_cmp->tp_eq == &default__eq__with__compare_eq ? &tdefault__eq__with__compare_eq : &tdefault__eq))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_gr == &usrtype__gr ? &tusrtype__gr : &tdefault__gr))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__eq__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_eq == &usrtype__eq ? &tusrtype__eq : tp_self->tp_cmp->tp_eq == &default__eq__with__compare_eq ? &tdefault__eq__with__compare_eq : &tdefault__eq))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_ge == &usrtype__ge ? &tusrtype__ge : &tdefault__ge))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__ne__and__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_ne == &usrtype__ne ? &tusrtype__ne : tp_self->tp_cmp->tp_ne == &default__ne__with__compare_eq ? &tdefault__ne__with__compare_eq : &tdefault__ne))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_lo == &usrtype__lo ? &tusrtype__lo : &tdefault__lo))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__ne__and__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_ne == &usrtype__ne ? &tusrtype__ne : tp_self->tp_cmp->tp_ne == &default__ne__with__compare_eq ? &tdefault__ne__with__compare_eq : &tdefault__ne))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_le == &usrtype__le ? &tusrtype__le : &tdefault__le))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__ne__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_ne == &usrtype__ne ? &tusrtype__ne : tp_self->tp_cmp->tp_ne == &default__ne__with__compare_eq ? &tdefault__ne__with__compare_eq : &tdefault__ne))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_gr == &usrtype__gr ? &tusrtype__gr : &tdefault__gr))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare__with__ne__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*(tp_self->tp_cmp->tp_ne == &usrtype__ne ? &tusrtype__ne : tp_self->tp_cmp->tp_ne == &default__ne__with__compare_eq ? &tdefault__ne__with__compare_eq : &tdefault__ne))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*(tp_self->tp_cmp->tp_ge == &usrtype__ge ? &tusrtype__ge : &tdefault__ge))(tp_self, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_compare)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__eq__and__lo(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__eq__and__lo(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_eq)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_lo)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__eq__and__le(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__eq__and__le(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_eq)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_le)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__eq__and__gr(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__eq__and__gr(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_eq)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_gr)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__eq__and__ge(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__eq__and__ge(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_eq)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ge)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__ne__and__lo(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__ne__and__lo(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ne)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_lo)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__ne__and__le(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__ne__and__le(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ne)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_le)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__ne__and__gr(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__ne__and__gr(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ne)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_gr)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__compare__with__ne__and__ge(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__compare__with__ne__and__ge(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ne)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = (*Dee_TYPE(lhs)->tp_cmp->tp_ge)(lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_trycompare_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__trycompare_eq__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__compare ? &tdefault__compare_eq__with__compare : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__eq ? &tdefault__compare_eq__with__eq : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__ne ? &tdefault__compare_eq__with__ne : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__lo__and__gr ? &tdefault__compare_eq__with__lo__and__gr : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__le__and__ge ? &tdefault__compare_eq__with__le__and__ge : &tdefault__compare_eq))(tp_self, lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__trycompare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_trycompare_eq)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__trycompare_eq__with__compare_eq(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trycompare_eq__with__compare_eq(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare_eq)(lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_eq */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_EQ, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__eq__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__compare ? &tdefault__compare_eq__with__compare : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__ne ? &tdefault__compare_eq__with__ne : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__lo__and__gr ? &tdefault__compare_eq__with__lo__and__gr : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__le__and__ge ? &tdefault__compare_eq__with__le__and__ge : &tdefault__compare_eq))(tp_self, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_eq)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__eq(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__eq(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_EQ, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__eq__with__compare_eq(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__eq__with__compare_eq(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare_eq)(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_ne */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_NE, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__ne__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__compare ? &tdefault__compare_eq__with__compare : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__eq ? &tdefault__compare_eq__with__eq : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__lo__and__gr ? &tdefault__compare_eq__with__lo__and__gr : tp_self->tp_cmp->tp_compare_eq == &default__compare_eq__with__le__and__ge ? &tdefault__compare_eq__with__le__and__ge : &tdefault__compare_eq))(tp_self, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_ne)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__ne(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__ne(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_NE, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__ne__with__compare_eq(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__ne__with__compare_eq(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare_eq)(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_lo */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_LO, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__lo__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare))(tp_self, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_lo)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__lo(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__lo(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_LO, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__lo__with__compare(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__lo__with__compare(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare)(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_le */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_LE, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__le__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare))(tp_self, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_le)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__le(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__le(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_LE, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__le__with__compare(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__le__with__compare(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare)(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_gr */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_GR, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__gr__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare))(tp_self, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_gr)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__gr(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__gr(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_GR, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__gr__with__compare(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__gr__with__compare(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare)(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_cmp->tp_ge */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return_DeeClass_CallOperator(tp_self, lhs, OPERATOR_GE, 1, &rhs);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__ge__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	int result = (*(tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : tp_self->tp_cmp->tp_compare == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : tp_self->tp_cmp->tp_compare == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare))(tp_self, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->tp_cmp->tp_ge)(lhs, rhs);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__ge(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__ge(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(lhs), lhs, OPERATOR_GE, 1, &rhs);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__ge__with__compare(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__ge__with__compare(Dee_TYPE(lhs), lhs, rhs);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(lhs)->tp_cmp->tp_compare)(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
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
	DREF DeeObject *iter = (*(tp_self->tp_seq->tp_iter == &usrtype__iter ? &tusrtype__iter : tp_self->tp_seq->tp_iter == &default__iter__with__foreach_pair ? &tdefault__iter__with__foreach_pair : &tdefault__iter))(tp_self, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

#ifndef DECLARED_default_foreach_with_foreach_pair_cb
#define DECLARED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DECLARED_default_foreach_with_foreach_pair_cb */
#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
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
	return (*(tp_self->tp_seq->tp_foreach_pair == &default__foreach_pair__with__iter ? &tdefault__foreach_pair__with__iter : &tdefault__foreach_pair))(tp_self, self, &default_foreach_with_foreach_pair_cb, &data);
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

#ifndef DECLARED_default_foreach_with_foreach_pair_cb
#define DECLARED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DECLARED_default_foreach_with_foreach_pair_cb */
#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
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
#ifndef DECLARED_default_foreach_pair_with_foreach_cb
#define DECLARED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DECLARED_default_foreach_pair_with_foreach_cb */
#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
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
	return (*(tp_self->tp_seq->tp_foreach == &default__foreach__with__iter ? &tdefault__foreach__with__iter : &tdefault__foreach))(tp_self, self, &default_foreach_pair_with_foreach_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach_pair__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = (*(tp_self->tp_seq->tp_iter == &usrtype__iter ? &tusrtype__iter : tp_self->tp_seq->tp_iter == &default__iter__with__foreach ? &tdefault__iter__with__foreach : &tdefault__iter))(tp_self, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	return (*tp_self->tp_seq->tp_foreach_pair)(self, cb, arg);
}

#ifndef DECLARED_default_foreach_pair_with_foreach_cb
#define DECLARED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DECLARED_default_foreach_pair_with_foreach_cb */
#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
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

/* tp_seq->tp_sizeob */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__sizeob(DeeTypeObject *tp_self, DeeObject *self) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_SIZE, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__sizeob__with__size(DeeTypeObject *tp_self, DeeObject *self) {
	size_t result = (*tp_self->tp_seq->tp_size)(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__sizeob(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_seq->tp_sizeob)(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__sizeob(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__sizeob(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_SIZE, 0, NULL);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__sizeob__with__size(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__sizeob__with__size(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	size_t result = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_size */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__size__with__sizeob(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_sizeob == &usrtype__sizeob ? &tusrtype__sizeob : &tdefault__sizeob))(tp_self, self);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__size(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_seq->tp_size)(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__size__with__sizeob(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__size__with__sizeob(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_sizeob)(self);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_size_fast */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__size_fast__with__(DeeTypeObject *tp_self, DeeObject *self) {
	return (size_t)-1; /* Not fast */
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
tdefault__size_fast(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_seq->tp_size_fast)(self);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
default__size_fast__with__(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__size_fast__with__(Dee_TYPE(self), self);
#else /* __OPTIMIZE_SIZE__ */
	return (size_t)-1; /* Not fast */
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_contains */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__contains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_CONTAINS, 1, &item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__contains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	return (*tp_self->tp_seq->tp_contains)(self, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__contains(DeeObject *self, DeeObject *item) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__contains(Dee_TYPE(self), self, item);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_CONTAINS, 1, &item);
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getitem */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_GETITEM, 1, &index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem))(tp_self, self, index);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem__with__getitem_index__and__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_getitem_string_len_hash == &default__getitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__getitem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__trygetitem_index ? &tdefault__getitem_index__with__trygetitem_index : &tdefault__getitem_index))(tp_self, self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem__with__getitem_index__and__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_getitem_string_hash == &default__getitem_string_hash__with__trygetitem_string_hash ? &tdefault__getitem_string_hash__with__trygetitem_string_hash : &tdefault__getitem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__trygetitem_index ? &tdefault__getitem_index__with__trygetitem_index : &tdefault__getitem_index))(tp_self, self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__trygetitem_index ? &tdefault__getitem_index__with__trygetitem_index : &tdefault__getitem_index))(tp_self, self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_getitem_string_len_hash == &default__getitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__getitem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_getitem_string_hash == &default__getitem_string_hash__with__trygetitem_string_hash ? &tdefault__getitem_string_hash__with__trygetitem_string_hash : &tdefault__getitem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return (*tp_self->tp_seq->tp_getitem)(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__getitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__getitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_GETITEM, 1, &index);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem__with__trygetitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem__with__trygetitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, index);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem__with__getitem_index__and__getitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem__with__getitem_index__and__getitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_getitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index_value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem__with__getitem_index__and__getitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem__with__getitem_index__and__getitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_getitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index_value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem__with__getitem_index(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem__with__getitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index_value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem__with__getitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem__with__getitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem__with__getitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem__with__getitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_trygetitem */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem__with__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_getitem == &usrtype__getitem ? &tusrtype__getitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index ? &tdefault__getitem__with__getitem_index : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_len_hash ? &tdefault__getitem__with__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_hash ? &tdefault__getitem__with__getitem_string_hash : &tdefault__getitem))(tp_self, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : &tdefault__trygetitem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : &tdefault__trygetitem_index))(tp_self, self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : &tdefault__trygetitem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : &tdefault__trygetitem_index))(tp_self, self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : &tdefault__trygetitem_index))(tp_self, self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : &tdefault__trygetitem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : &tdefault__trygetitem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return (*tp_self->tp_seq->tp_trygetitem)(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem__with__getitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem__with__getitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index_value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index_value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem__with__trygetitem_index(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem__with__trygetitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index_value);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem__with__trygetitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem__with__trygetitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem__with__trygetitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem__with__trygetitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getitem_index_fast */
/* tp_seq->tp_getitem_index */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result;
	size_t size = (*(tp_self->tp_seq->tp_size == &default__size__with__sizeob ? &tdefault__size__with__sizeob : &tdefault__size))(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index);
	if unlikely(!result)
		err_unbound_index(self, index);
	return result;
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getitem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__trygetitem ? &tdefault__trygetitem_index__with__trygetitem : &tdefault__trygetitem_index))(tp_self, self, index);
	if unlikely(result == ITER_DONE) {
		err_unbound_index(self, index);
		result = NULL;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getitem_index__with__getitem(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*(tp_self->tp_seq->tp_getitem == &usrtype__getitem ? &tusrtype__getitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__trygetitem ? &tdefault__getitem__with__trygetitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_len_hash ? &tdefault__getitem__with__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_hash ? &tdefault__getitem__with__getitem_string_hash : &tdefault__getitem))(tp_self, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	return (*tp_self->tp_seq->tp_getitem_index)(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__getitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_index__with__size__and__getitem_index_fast(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, index);
	if unlikely(!result)
		err_unbound_index(self, index);
	return result;
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__getitem_index__with__trygetitem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_index__with__trygetitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index);
	if unlikely(result == ITER_DONE) {
		err_unbound_index(self, index);
		result = NULL;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__getitem_index__with__getitem(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_index__with__getitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_trygetitem_index */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__trygetitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result;
	size_t size = (*(tp_self->tp_seq->tp_size == &default__size__with__sizeob ? &tdefault__size__with__sizeob : &tdefault__size))(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return ITER_DONE;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index);
	if (result == NULL)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__trygetitem_index__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__getitem ? &tdefault__getitem_index__with__getitem : &tdefault__getitem_index))(tp_self, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__trygetitem_index__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem))(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	return (*tp_self->tp_seq->tp_trygetitem_index)(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__trygetitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_index__with__size__and__getitem_index_fast(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return ITER_DONE;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, index);
	if (result == NULL)
		result = ITER_DONE;
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__trygetitem_index__with__getitem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_index__with__getitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__trygetitem_index__with__trygetitem(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_index__with__trygetitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__trygetitem ? &tdefault__trygetitem_string_hash__with__trygetitem : &tdefault__trygetitem_string_hash))(tp_self, self, key, hash);
	if unlikely(result == ITER_DONE) {
		err_unbound_key_str(self, key);
		result = NULL;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem_string_hash__with__getitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_getitem == &usrtype__getitem ? &tusrtype__getitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__trygetitem ? &tdefault__getitem__with__trygetitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index ? &tdefault__getitem__with__getitem_index : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_len_hash ? &tdefault__getitem__with__getitem_string_len_hash : &tdefault__getitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_getitem_string_hash)(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_string_hash__with__trygetitem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_hash)(self, key, hash);
	if unlikely(result == ITER_DONE) {
		err_unbound_key_str(self, key);
		result = NULL;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem_string_hash__with__getitem(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_string_hash__with__getitem(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_trygetitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem_string_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_getitem_string_hash == &default__getitem_string_hash__with__getitem ? &tdefault__getitem_string_hash__with__getitem : &tdefault__getitem_string_hash))(tp_self, self, key, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem_string_hash__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : &tdefault__trygetitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_trygetitem_string_hash)(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem_string_hash__with__getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_string_hash__with__getitem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem_string_hash)(self, key, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem_string_hash__with__trygetitem(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_string_hash__with__trygetitem(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__trygetitem ? &tdefault__trygetitem_string_len_hash__with__trygetitem : &tdefault__trygetitem_string_len_hash))(tp_self, self, key, keylen, hash);
	if unlikely(result == ITER_DONE) {
		err_unbound_key_str(self, key);
		result = NULL;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem_string_len_hash__with__getitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_getitem == &usrtype__getitem ? &tusrtype__getitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__trygetitem ? &tdefault__getitem__with__trygetitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index ? &tdefault__getitem__with__getitem_index : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_hash ? &tdefault__getitem__with__getitem_string_hash : &tdefault__getitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_len_hash)(self, key, keylen, hash);
	if unlikely(result == ITER_DONE) {
		err_unbound_key_str(self, key);
		result = NULL;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__getitem_string_len_hash__with__getitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getitem_string_len_hash__with__getitem(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_trygetitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = (*(tp_self->tp_seq->tp_getitem_string_len_hash == &default__getitem_string_len_hash__with__getitem ? &tdefault__getitem_string_len_hash__with__getitem : &tdefault__getitem_string_len_hash))(tp_self, self, key, keylen, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem_string_len_hash__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_trygetitem_string_len_hash)(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem_string_len_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result = (*Dee_TYPE(self)->tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
default__trygetitem_string_len_hash__with__trygetitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__trygetitem_string_len_hash__with__trygetitem(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_bounditem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return tdefault__bounditem_index__with__size__and__getitem_index_fast(tp_self, self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_getitem == &usrtype__getitem ? &tusrtype__getitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__trygetitem ? &tdefault__getitem__with__trygetitem : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_index ? &tdefault__getitem__with__getitem_index : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_len_hash ? &tdefault__getitem__with__getitem_string_len_hash : tp_self->tp_seq->tp_getitem == &default__getitem__with__getitem_string_hash ? &tdefault__getitem__with__getitem_string_hash : &tdefault__getitem))(tp_self, self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__getitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__getitem_string_len_hash : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__bounditem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__size__and__getitem_index_fast ? &tdefault__bounditem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__getitem_index ? &tdefault__bounditem_index__with__getitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index ? &tdefault__bounditem_index__with__trygetitem_index : &tdefault__bounditem_index))(tp_self, self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__getitem_string_hash ? &tdefault__bounditem_string_hash__with__getitem_string_hash : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__trygetitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash : &tdefault__bounditem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__size__and__getitem_index_fast ? &tdefault__bounditem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__getitem_index ? &tdefault__bounditem_index__with__getitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index ? &tdefault__bounditem_index__with__trygetitem_index : &tdefault__bounditem_index))(tp_self, self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__trygetitem__and__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem))(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*(tp_self->tp_seq->tp_hasitem == &default__hasitem__with__size__and__getitem_index_fast ? &tdefault__hasitem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index ? &tdefault__hasitem__with__hasitem_index : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_string_len_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_string_hash : &tdefault__hasitem))(tp_self, self, index);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__size__and__getitem_index_fast ? &tdefault__bounditem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__getitem_index ? &tdefault__bounditem_index__with__getitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index ? &tdefault__bounditem_index__with__trygetitem_index : &tdefault__bounditem_index))(tp_self, self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__getitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__getitem_string_len_hash : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__bounditem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__getitem_string_hash ? &tdefault__bounditem_string_hash__with__getitem_string_hash : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__trygetitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash : &tdefault__bounditem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem))(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return (*tp_self->tp_seq->tp_bounditem)(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__size__and__getitem_index_fast(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__size__and__getitem_index_fast(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return default__bounditem_index__with__size__and__getitem_index_fast(self, index_value);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__getitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__getitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__bounditem_index__and__bounditem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_bounditem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, index_value);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__bounditem_index__and__bounditem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_bounditem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, index_value);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__trygetitem__and__hasitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__trygetitem__and__hasitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem)(self, index);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__bounditem_index(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__bounditem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, index_value);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__bounditem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__bounditem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__bounditem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__bounditem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem__with__trygetitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem__with__trygetitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_bounditem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bounditem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *result;
	size_t size = (*(tp_self->tp_seq->tp_size == &default__size__with__sizeob ? &tdefault__size__with__sizeob : &tdefault__size))(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return Dee_BOUND_MISSING;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index);
	if unlikely(!result)
		return Dee_BOUND_NO;
	Dee_Decref(result);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bounditem_index__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__trygetitem_index ? &tdefault__getitem_index__with__trygetitem_index : tp_self->tp_seq->tp_getitem_index == &default__getitem_index__with__getitem ? &tdefault__getitem_index__with__getitem : &tdefault__getitem_index))(tp_self, self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bounditem_index__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*(tp_self->tp_seq->tp_bounditem == &default__bounditem__with__size__and__getitem_index_fast ? &tdefault__bounditem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__getitem ? &tdefault__bounditem__with__getitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem__and__hasitem ? &tdefault__bounditem__with__trygetitem__and__hasitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem ? &tdefault__bounditem__with__trygetitem : &tdefault__bounditem))(tp_self, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__trygetitem ? &tdefault__trygetitem_index__with__trygetitem : &tdefault__trygetitem_index))(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*(tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__size__and__getitem_index_fast ? &tdefault__hasitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__hasitem ? &tdefault__hasitem_index__with__hasitem : &tdefault__hasitem_index))(tp_self, self, index);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bounditem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__trygetitem ? &tdefault__trygetitem_index__with__trygetitem : &tdefault__trygetitem_index))(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	return (*tp_self->tp_seq->tp_bounditem_index)(self, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__bounditem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_index__with__size__and__getitem_index_fast(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return Dee_BOUND_MISSING;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem_index_fast)(self, index);
	if unlikely(!result)
		return Dee_BOUND_NO;
	Dee_Decref(result);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__bounditem_index__with__getitem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_index__with__getitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__bounditem_index__with__bounditem(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_index__with__bounditem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__bounditem_index__with__trygetitem_index__and__hasitem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem_index)(self, index);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__bounditem_index__with__trygetitem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_index__with__trygetitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_bounditem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_getitem_string_hash == &default__getitem_string_hash__with__trygetitem_string_hash ? &tdefault__getitem_string_hash__with__trygetitem_string_hash : tp_self->tp_seq->tp_getitem_string_hash == &default__getitem_string_hash__with__getitem ? &tdefault__getitem_string_hash__with__getitem : &tdefault__getitem_string_hash))(tp_self, self, key, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_hash__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_bounditem == &default__bounditem__with__size__and__getitem_index_fast ? &tdefault__bounditem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__getitem ? &tdefault__bounditem__with__getitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem__and__hasitem ? &tdefault__bounditem__with__trygetitem__and__hasitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index ? &tdefault__bounditem__with__bounditem_index : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem ? &tdefault__bounditem__with__trygetitem : &tdefault__bounditem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__trygetitem ? &tdefault__trygetitem_string_hash__with__trygetitem : &tdefault__trygetitem_string_hash))(tp_self, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*(tp_self->tp_seq->tp_hasitem_string_hash == &default__hasitem_string_hash__with__hasitem ? &tdefault__hasitem_string_hash__with__hasitem : &tdefault__hasitem_string_hash))(tp_self, self, key, hash);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__trygetitem ? &tdefault__trygetitem_string_hash__with__trygetitem : &tdefault__trygetitem_string_hash))(tp_self, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_bounditem_string_hash)(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_hash__with__getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_hash__with__getitem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_getitem_string_hash)(self, key, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_hash__with__bounditem(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_hash__with__bounditem(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_hash)(self, key, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem_string_hash)(self, key, hash);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_hash__with__trygetitem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_hash)(self, key, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_bounditem_string_len_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_len_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_getitem_string_len_hash == &default__getitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_getitem_string_len_hash == &default__getitem_string_len_hash__with__getitem ? &tdefault__getitem_string_len_hash__with__getitem : &tdefault__getitem_string_len_hash))(tp_self, self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_len_hash__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_bounditem == &default__bounditem__with__size__and__getitem_index_fast ? &tdefault__bounditem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__getitem ? &tdefault__bounditem__with__getitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem__and__hasitem ? &tdefault__bounditem__with__trygetitem__and__hasitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index ? &tdefault__bounditem__with__bounditem_index : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem ? &tdefault__bounditem__with__trygetitem : &tdefault__bounditem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__trygetitem ? &tdefault__trygetitem_string_len_hash__with__trygetitem : &tdefault__trygetitem_string_len_hash))(tp_self, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*(tp_self->tp_seq->tp_hasitem_string_len_hash == &default__hasitem_string_len_hash__with__hasitem ? &tdefault__hasitem_string_len_hash__with__hasitem : &tdefault__hasitem_string_len_hash))(tp_self, self, key, keylen, hash);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__trygetitem ? &tdefault__trygetitem_string_len_hash__with__trygetitem : &tdefault__trygetitem_string_len_hash))(tp_self, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_bounditem_string_len_hash)(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_len_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_len_hash__with__getitem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_len_hash__with__bounditem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_len_hash__with__bounditem(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_len_hash)(self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem_string_len_hash)(self, key, keylen, hash);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__bounditem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_len_hash)(self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_hasitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return tdefault__hasitem_index__with__size__and__getitem_index_fast(tp_self, self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_trygetitem == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem))(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	int result = (*(tp_self->tp_seq->tp_bounditem == &default__bounditem__with__size__and__getitem_index_fast ? &tdefault__bounditem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__getitem ? &tdefault__bounditem__with__getitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index__and__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem__and__hasitem ? &tdefault__bounditem__with__trygetitem__and__hasitem : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_index ? &tdefault__bounditem__with__bounditem_index : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_string_len_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_string_hash : tp_self->tp_seq->tp_bounditem == &default__bounditem__with__trygetitem ? &tdefault__bounditem__with__trygetitem : &tdefault__bounditem))(tp_self, self, index);
	return Dee_BOUND_ASHAS(result);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_hasitem_string_len_hash == &default__hasitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_hasitem_string_len_hash == &default__hasitem_string_len_hash__with__bounditem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash : &tdefault__hasitem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__size__and__getitem_index_fast ? &tdefault__hasitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__trygetitem_index ? &tdefault__hasitem_index__with__trygetitem_index : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__bounditem_index ? &tdefault__hasitem_index__with__bounditem_index : &tdefault__hasitem_index))(tp_self, self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*(tp_self->tp_seq->tp_hasitem_string_hash == &default__hasitem_string_hash__with__trygetitem_string_hash ? &tdefault__hasitem_string_hash__with__trygetitem_string_hash : tp_self->tp_seq->tp_hasitem_string_hash == &default__hasitem_string_hash__with__bounditem_string_hash ? &tdefault__hasitem_string_hash__with__bounditem_string_hash : &tdefault__hasitem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__size__and__getitem_index_fast ? &tdefault__hasitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__trygetitem_index ? &tdefault__hasitem_index__with__trygetitem_index : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__bounditem_index ? &tdefault__hasitem_index__with__bounditem_index : &tdefault__hasitem_index))(tp_self, self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*(tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__size__and__getitem_index_fast ? &tdefault__hasitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__trygetitem_index ? &tdefault__hasitem_index__with__trygetitem_index : tp_self->tp_seq->tp_hasitem_index == &default__hasitem_index__with__bounditem_index ? &tdefault__hasitem_index__with__bounditem_index : &tdefault__hasitem_index))(tp_self, self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_hasitem_string_len_hash == &default__hasitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash : tp_self->tp_seq->tp_hasitem_string_len_hash == &default__hasitem_string_len_hash__with__bounditem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash : &tdefault__hasitem_string_len_hash))(tp_self, self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem__with__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*(tp_self->tp_seq->tp_hasitem_string_hash == &default__hasitem_string_hash__with__trygetitem_string_hash ? &tdefault__hasitem_string_hash__with__trygetitem_string_hash : tp_self->tp_seq->tp_hasitem_string_hash == &default__hasitem_string_hash__with__bounditem_string_hash ? &tdefault__hasitem_string_hash__with__bounditem_string_hash : &tdefault__hasitem_string_hash))(tp_self, self, DeeString_STR(index), DeeString_Hash(index));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return (*tp_self->tp_seq->tp_hasitem)(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__size__and__getitem_index_fast(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__size__and__getitem_index_fast(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return default__hasitem_index__with__size__and__getitem_index_fast(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__trygetitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__trygetitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem)(self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__bounditem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__bounditem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, index);
	return Dee_BOUND_ASHAS(result);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__hasitem_index__and__hasitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_hasitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_hasitem_index)(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__hasitem_index__and__hasitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_hasitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_hasitem_index)(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__hasitem_index(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__hasitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_hasitem_index)(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__hasitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__hasitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_hasitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem__with__hasitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem__with__hasitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_hasitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_hasitem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__hasitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	size_t size = (*(tp_self->tp_seq->tp_size == &default__size__with__sizeob ? &tdefault__size__with__sizeob : &tdefault__size))(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__hasitem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : tp_self->tp_seq->tp_trygetitem_index == &default__trygetitem_index__with__trygetitem ? &tdefault__trygetitem_index__with__trygetitem : &tdefault__trygetitem_index))(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__hasitem_index__with__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	int result = (*(tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__size__and__getitem_index_fast ? &tdefault__bounditem_index__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__getitem_index ? &tdefault__bounditem_index__with__getitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__bounditem ? &tdefault__bounditem_index__with__bounditem : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index : tp_self->tp_seq->tp_bounditem_index == &default__bounditem_index__with__trygetitem_index ? &tdefault__bounditem_index__with__trygetitem_index : &tdefault__bounditem_index))(tp_self, self, index);
	return Dee_BOUND_ASHAS(result);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__hasitem_index__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*(tp_self->tp_seq->tp_hasitem == &default__hasitem__with__size__and__getitem_index_fast ? &tdefault__hasitem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__trygetitem ? &tdefault__hasitem__with__trygetitem : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__bounditem ? &tdefault__hasitem__with__bounditem : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_string_len_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_string_hash : &tdefault__hasitem))(tp_self, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	return (*tp_self->tp_seq->tp_hasitem_index)(self, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__hasitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_index__with__size__and__getitem_index_fast(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t size = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__hasitem_index__with__trygetitem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_index__with__trygetitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_index)(self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__hasitem_index__with__bounditem_index(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_index__with__bounditem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, index);
	return Dee_BOUND_ASHAS(result);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__hasitem_index__with__hasitem(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_index__with__hasitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem)(self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_hasitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : tp_self->tp_seq->tp_trygetitem_string_hash == &default__trygetitem_string_hash__with__trygetitem ? &tdefault__trygetitem_string_hash__with__trygetitem : &tdefault__trygetitem_string_hash))(tp_self, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_hash__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	int result = (*(tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__getitem_string_hash ? &tdefault__bounditem_string_hash__with__getitem_string_hash : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__bounditem ? &tdefault__bounditem_string_hash__with__bounditem : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash : tp_self->tp_seq->tp_bounditem_string_hash == &default__bounditem_string_hash__with__trygetitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash : &tdefault__bounditem_string_hash))(tp_self, self, key, hash);
	return Dee_BOUND_ASHAS(result);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_hash__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_hasitem == &default__hasitem__with__size__and__getitem_index_fast ? &tdefault__hasitem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__trygetitem ? &tdefault__hasitem__with__trygetitem : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__bounditem ? &tdefault__hasitem__with__bounditem : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index ? &tdefault__hasitem__with__hasitem_index : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_string_len_hash : &tdefault__hasitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_hasitem_string_hash)(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_string_hash__with__trygetitem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_hash)(self, key, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem_string_hash__with__bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_string_hash__with__bounditem_string_hash(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(self)->tp_seq->tp_bounditem_string_hash)(self, key, hash);
	return Dee_BOUND_ASHAS(result);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem_string_hash__with__hasitem(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_string_hash__with__hasitem(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_hasitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *value = (*(tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : tp_self->tp_seq->tp_trygetitem_string_len_hash == &default__trygetitem_string_len_hash__with__trygetitem ? &tdefault__trygetitem_string_len_hash__with__trygetitem : &tdefault__trygetitem_string_len_hash))(tp_self, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result = (*(tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__getitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__getitem_string_len_hash : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__bounditem ? &tdefault__bounditem_string_len_hash__with__bounditem : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash : tp_self->tp_seq->tp_bounditem_string_len_hash == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__bounditem_string_len_hash))(tp_self, self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_len_hash__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_hasitem == &default__hasitem__with__size__and__getitem_index_fast ? &tdefault__hasitem__with__size__and__getitem_index_fast : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__trygetitem ? &tdefault__hasitem__with__trygetitem : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__bounditem ? &tdefault__hasitem__with__bounditem : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index__and__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_index ? &tdefault__hasitem__with__hasitem_index : tp_self->tp_seq->tp_hasitem == &default__hasitem__with__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_string_hash : &tdefault__hasitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_hasitem_string_len_hash)(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *value = (*Dee_TYPE(self)->tp_seq->tp_trygetitem_string_len_hash)(self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem_string_len_hash__with__bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result = (*Dee_TYPE(self)->tp_seq->tp_bounditem_string_len_hash)(self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__hasitem_string_len_hash__with__hasitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__hasitem_string_len_hash__with__hasitem(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_hasitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delitem */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__delitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_DELITEM, 1, &index);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem__with__delitem_index__and__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*tp_self->tp_seq->tp_delitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*tp_self->tp_seq->tp_delitem_index)(self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem__with__delitem_index__and__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*tp_self->tp_seq->tp_delitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*tp_self->tp_seq->tp_delitem_index)(self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem__with__delitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*tp_self->tp_seq->tp_delitem_index)(self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem__with__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*tp_self->tp_seq->tp_delitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem__with__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*tp_self->tp_seq->tp_delitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return (*tp_self->tp_seq->tp_delitem)(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__delitem(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__delitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_DELITEM, 1, &index);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem__with__delitem_index__and__delitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem__with__delitem_index__and__delitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_delitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem__with__delitem_index__and__delitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem__with__delitem_index__and__delitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_delitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem__with__delitem_index(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem__with__delitem_index(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index_value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem__with__delitem_string_len_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem__with__delitem_string_len_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index));
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem__with__delitem_string_hash(DeeObject *self, DeeObject *index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem__with__delitem_string_hash(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index));
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delitem_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__delitem_index__with__delitem(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*(tp_self->tp_seq->tp_delitem == &usrtype__delitem ? &tusrtype__delitem : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index__and__delitem_string_len_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index__and__delitem_string_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_string_len_hash ? &tdefault__delitem__with__delitem_string_len_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_string_hash ? &tdefault__delitem__with__delitem_string_hash : &tdefault__delitem))(tp_self, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__delitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	return (*tp_self->tp_seq->tp_delitem_index)(self, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__delitem_index__with__delitem(DeeObject *self, size_t index) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem_index__with__delitem(Dee_TYPE(self), self, index);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem_string_hash__with__delitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_delitem == &usrtype__delitem ? &tusrtype__delitem : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index__and__delitem_string_len_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index__and__delitem_string_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index ? &tdefault__delitem__with__delitem_index : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_string_len_hash ? &tdefault__delitem__with__delitem_string_len_hash : &tdefault__delitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_delitem_string_hash)(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem_string_hash__with__delitem(DeeObject *self, char const *key, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem_string_hash__with__delitem(Dee_TYPE(self), self, key, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem_string_len_hash__with__delitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_delitem == &usrtype__delitem ? &tusrtype__delitem : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index__and__delitem_string_len_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index__and__delitem_string_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_hash : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_index ? &tdefault__delitem__with__delitem_index : tp_self->tp_seq->tp_delitem == &default__delitem__with__delitem_string_hash ? &tdefault__delitem__with__delitem_string_hash : &tdefault__delitem))(tp_self, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return (*tp_self->tp_seq->tp_delitem_string_len_hash)(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__delitem_string_len_hash__with__delitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delitem_string_len_hash__with__delitem(Dee_TYPE(self), self, key, keylen, hash);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setitem */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tusrtype__setitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_SETITEM, 2, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__setitem__with__setitem_index__and__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*tp_self->tp_seq->tp_setitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index), value);
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*tp_self->tp_seq->tp_setitem_index)(self, index_value, value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__setitem__with__setitem_index__and__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*tp_self->tp_seq->tp_setitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index), value);
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*tp_self->tp_seq->tp_setitem_index)(self, index_value, value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__setitem__with__setitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*tp_self->tp_seq->tp_setitem_index)(self, index_value, value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__setitem__with__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*tp_self->tp_seq->tp_setitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index), value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__setitem__with__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*tp_self->tp_seq->tp_setitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index), value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__setitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	return (*tp_self->tp_seq->tp_setitem)(self, index, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
usrtype__setitem(DeeObject *self, DeeObject *index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__setitem(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_SETITEM, 2, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__setitem__with__setitem_index__and__setitem_string_len_hash(DeeObject *self, DeeObject *index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem__with__setitem_index__and__setitem_string_len_hash(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_setitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index), value);
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_index)(self, index_value, value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__setitem__with__setitem_index__and__setitem_string_hash(DeeObject *self, DeeObject *index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem__with__setitem_index__and__setitem_string_hash(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeString_Check(index)) {
		return (*Dee_TYPE(self)->tp_seq->tp_setitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index), value);
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_index)(self, index_value, value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__setitem__with__setitem_index(DeeObject *self, DeeObject *index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem__with__setitem_index(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_index)(self, index_value, value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__setitem__with__setitem_string_len_hash(DeeObject *self, DeeObject *index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem__with__setitem_string_len_hash(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_string_len_hash)(self, DeeString_STR(index), DeeString_SIZE(index), DeeString_Hash(index), value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__setitem__with__setitem_string_hash(DeeObject *self, DeeObject *index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem__with__setitem_string_hash(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_string_hash)(self, DeeString_STR(index), DeeString_Hash(index), value);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setitem_index */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__setitem_index__with__setitem(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value) {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*(tp_self->tp_seq->tp_setitem == &usrtype__setitem ? &tusrtype__setitem : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index__and__setitem_string_len_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index__and__setitem_string_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_string_len_hash ? &tdefault__setitem__with__setitem_string_len_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_string_hash ? &tdefault__setitem__with__setitem_string_hash : &tdefault__setitem))(tp_self, self, indexob, value);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__setitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value) {
	return (*tp_self->tp_seq->tp_setitem_index)(self, index, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__setitem_index__with__setitem(DeeObject *self, size_t index, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem_index__with__setitem(Dee_TYPE(self), self, index, value);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, indexob, value);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setitem_string_hash */
INTERN WUNUSED NONNULL((1, 2, 3, 5)) int DCALL
tdefault__setitem_string_hash__with__setitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_setitem == &usrtype__setitem ? &tusrtype__setitem : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index__and__setitem_string_len_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index__and__setitem_string_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index ? &tdefault__setitem__with__setitem_index : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_string_len_hash ? &tdefault__setitem__with__setitem_string_len_hash : &tdefault__setitem))(tp_self, self, keyob, value);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5)) int DCALL
tdefault__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	return (*tp_self->tp_seq->tp_setitem_string_hash)(self, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
default__setitem_string_hash__with__setitem(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem_string_hash__with__setitem(Dee_TYPE(self), self, key, hash, value);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, keyob, value);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setitem_string_len_hash */
INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__setitem_string_len_hash__with__setitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*(tp_self->tp_seq->tp_setitem == &usrtype__setitem ? &tusrtype__setitem : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index__and__setitem_string_len_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index__and__setitem_string_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_hash : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_index ? &tdefault__setitem__with__setitem_index : tp_self->tp_seq->tp_setitem == &default__setitem__with__setitem_string_hash ? &tdefault__setitem__with__setitem_string_hash : &tdefault__setitem))(tp_self, self, keyob, value);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 6)) int DCALL
tdefault__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return (*tp_self->tp_seq->tp_setitem_string_len_hash)(self, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
default__setitem_string_len_hash__with__setitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setitem_string_len_hash__with__setitem(Dee_TYPE(self), self, key, keylen, hash, value);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, keyob, value);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tusrtype__getrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return_DeeClass_CallOperator(tp_self, self, OPERATOR_GETRANGE, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__getrange__with__getrange_index__and__getrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*tp_self->tp_seq->tp_getrange_index_n)(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*tp_self->tp_seq->tp_getrange_index)(self, start_index, end_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
tdefault__getrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	return (*tp_self->tp_seq->tp_getrange)(self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
usrtype__getrange(DeeObject *self, DeeObject *start, DeeObject *end) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__getrange(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return_DeeClass_CallOperator(Dee_TYPE(self), self, OPERATOR_GETRANGE, 2, args);
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
default__getrange__with__getrange_index__and__getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getrange__with__getrange_index__and__getrange_index_n(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*Dee_TYPE(self)->tp_seq->tp_getrange_index_n)(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_getrange_index)(self, start_index, end_index);
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getrange_index */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getrange_index__with__getrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*(tp_self->tp_seq->tp_getrange == &usrtype__getrange ? &tusrtype__getrange : tp_self->tp_seq->tp_getrange == &default__getrange__with__getrange_index__and__getrange_index_n ? &tdefault__getrange__with__getrange_index__and__getrange_index_n : &tdefault__getrange))(tp_self, self, startob, endob);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getrange_index(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return (*tp_self->tp_seq->tp_getrange_index)(self, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__getrange_index__with__getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getrange_index__with__getrange(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*Dee_TYPE(self)->tp_seq->tp_getrange)(self, startob, endob);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_getrange_index_n */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getrange_index_n__with__getrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start) {
	DREF DeeObject *result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*(tp_self->tp_seq->tp_getrange == &usrtype__getrange ? &tusrtype__getrange : tp_self->tp_seq->tp_getrange == &default__getrange__with__getrange_index__and__getrange_index_n ? &tdefault__getrange__with__getrange_index__and__getrange_index_n : &tdefault__getrange))(tp_self, self, startob, Dee_None);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__getrange_index_n(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start) {
	return (*tp_self->tp_seq->tp_getrange_index_n)(self, start);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__getrange_index_n__with__getrange(DeeObject *self, Dee_ssize_t start) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__getrange_index_n__with__getrange(Dee_TYPE(self), self, start);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_getrange)(self, startob, Dee_None);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return NULL;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tusrtype__delrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_GETRANGE, 2, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__delrange__with__delrange_index__and__delrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*tp_self->tp_seq->tp_delrange_index_n)(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*tp_self->tp_seq->tp_delrange_index)(self, start_index, end_index);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
tdefault__delrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	return (*tp_self->tp_seq->tp_delrange)(self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
usrtype__delrange(DeeObject *self, DeeObject *start, DeeObject *end) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__delrange(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_GETRANGE, 2, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
default__delrange__with__delrange_index__and__delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delrange__with__delrange_index__and__delrange_index_n(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*Dee_TYPE(self)->tp_seq->tp_delrange_index_n)(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_delrange_index)(self, start_index, end_index);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delrange_index */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__delrange_index__with__delrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*(tp_self->tp_seq->tp_delrange == &usrtype__delrange ? &tusrtype__delrange : tp_self->tp_seq->tp_delrange == &default__delrange__with__delrange_index__and__delrange_index_n ? &tdefault__delrange__with__delrange_index__and__delrange_index_n : &tdefault__delrange))(tp_self, self, startob, endob);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__delrange_index(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return (*tp_self->tp_seq->tp_delrange_index)(self, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__delrange_index__with__delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delrange_index__with__delrange(Dee_TYPE(self), self, start, end);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*Dee_TYPE(self)->tp_seq->tp_delrange)(self, startob, endob);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_delrange_index_n */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__delrange_index_n__with__delrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start) {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*(tp_self->tp_seq->tp_delrange == &usrtype__delrange ? &tusrtype__delrange : tp_self->tp_seq->tp_delrange == &default__delrange__with__delrange_index__and__delrange_index_n ? &tdefault__delrange__with__delrange_index__and__delrange_index_n : &tdefault__delrange))(tp_self, self, startob, Dee_None);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__delrange_index_n(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start) {
	return (*tp_self->tp_seq->tp_delrange_index_n)(self, start);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default__delrange_index_n__with__delrange(DeeObject *self, Dee_ssize_t start) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__delrange_index_n__with__delrange(Dee_TYPE(self), self, start);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_delrange)(self, startob, Dee_None);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setrange */
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
tusrtype__setrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	store_DeeClass_CallOperator(err, result, tp_self, self, OPERATOR_SETRANGE, 3, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
tdefault__setrange__with__setrange_index__and__setrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*tp_self->tp_seq->tp_setrange_index_n)(self, start_index, values);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*tp_self->tp_seq->tp_setrange_index)(self, start_index, end_index, values);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
tdefault__setrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	return (*tp_self->tp_seq->tp_setrange)(self, start, end, values);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
usrtype__setrange(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
#ifdef __OPTIMIZE_SIZE__
	return tusrtype__setrange(Dee_TYPE(self), self, start, end, values);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	store_DeeClass_CallOperator(err, result, Dee_TYPE(self), self, OPERATOR_SETRANGE, 3, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
default__setrange__with__setrange_index__and__setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setrange__with__setrange_index__and__setrange_index_n(Dee_TYPE(self), self, start, end, values);
#else /* __OPTIMIZE_SIZE__ */
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return (*Dee_TYPE(self)->tp_seq->tp_setrange_index_n)(self, start_index, values);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, start_index, end_index, values);
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setrange_index */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__setrange_index__with__setrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*(tp_self->tp_seq->tp_setrange == &usrtype__setrange ? &tusrtype__setrange : tp_self->tp_seq->tp_setrange == &default__setrange__with__setrange_index__and__setrange_index_n ? &tdefault__setrange__with__setrange_index__and__setrange_index_n : &tdefault__setrange))(tp_self, self, startob, endob, values);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
tdefault__setrange_index(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	return (*tp_self->tp_seq->tp_setrange_index)(self, start, end, values);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
default__setrange_index__with__setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setrange_index__with__setrange(Dee_TYPE(self), self, start, end, values);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange)(self, startob, endob, values);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
#endif /* __OPTIMIZE_SIZE__ */
}

/* tp_seq->tp_setrange_index_n */
INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__setrange_index_n__with__setrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*(tp_self->tp_seq->tp_setrange == &usrtype__setrange ? &tusrtype__setrange : tp_self->tp_seq->tp_setrange == &default__setrange__with__setrange_index__and__setrange_index_n ? &tdefault__setrange__with__setrange_index__and__setrange_index_n : &tdefault__setrange))(tp_self, self, startob, Dee_None, values);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
tdefault__setrange_index_n(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	return (*tp_self->tp_seq->tp_setrange_index_n)(self, start, values);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
default__setrange_index_n__with__setrange(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
#ifdef __OPTIMIZE_SIZE__
	return tdefault__setrange_index_n__with__setrange(Dee_TYPE(self), self, start, values);
#else /* __OPTIMIZE_SIZE__ */
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange)(self, startob, Dee_None, values);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return -1;
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_add == &usrtype__add ? &tusrtype__add : &tdefault__add))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_sub == &usrtype__sub ? &tusrtype__sub : &tdefault__sub))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_mul == &usrtype__mul ? &tusrtype__mul : &tdefault__mul))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_div == &usrtype__div ? &tusrtype__div : &tdefault__div))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_mod == &usrtype__mod ? &tusrtype__mod : &tdefault__mod))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_shl == &usrtype__shl ? &tusrtype__shl : &tdefault__shl))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_shr == &usrtype__shr ? &tusrtype__shr : &tdefault__shr))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_and == &usrtype__and ? &tusrtype__and : &tdefault__and))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_or == &usrtype__or ? &tusrtype__or : &tdefault__or))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_xor == &usrtype__xor ? &tusrtype__xor : &tdefault__xor))(tp_self, *p_lhs, rhs);
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
	DREF DeeObject *result = (*(tp_self->tp_math->tp_pow == &usrtype__pow ? &tusrtype__pow : &tdefault__pow))(tp_self, *p_lhs, rhs);
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
	return (*(tp_self->tp_math->tp_inplace_add == &usrtype__inplace_add ? &tusrtype__inplace_add : tp_self->tp_math->tp_inplace_add == &default__inplace_add__with__add ? &tdefault__inplace_add__with__add : &tdefault__inplace_add))(tp_self, p_self, DeeInt_One);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__inc__with__add(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result = (*(tp_self->tp_math->tp_add == &usrtype__add ? &tusrtype__add : &tdefault__add))(tp_self, *p_self, DeeInt_One);
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
	return (*(tp_self->tp_math->tp_inplace_sub == &usrtype__inplace_sub ? &tusrtype__inplace_sub : tp_self->tp_math->tp_inplace_sub == &default__inplace_sub__with__sub ? &tdefault__inplace_sub__with__sub : &tdefault__inplace_sub))(tp_self, p_self, DeeInt_One);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__dec__with__sub(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result = (*(tp_self->tp_math->tp_sub == &usrtype__sub ? &tusrtype__sub : &tdefault__sub))(tp_self, *p_self, DeeInt_One);
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
