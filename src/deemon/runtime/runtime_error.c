/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C
#define GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C 1

#include "runtime_error.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

#define Q3  "??" "?"


PUBLIC ATTR_COLD int DCALL
Dee_BadAlloc(size_t req_bytes) {
	DeeNoMemoryErrorObject *nomem_error;
	int result;
	nomem_error = DeeObject_TRYMALLOC(DeeNoMemoryErrorObject);
	if (!nomem_error) {
		/* If we can't even allocate the no-memory
		 * object, throw a static instance. */
		return DeeError_Throw(&DeeError_NoMemory_instance);
	}
	DeeObject_Init(nomem_error, &DeeError_NoMemory);
	nomem_error->e_message    = NULL;
	nomem_error->e_inner      = NULL;
	nomem_error->nm_allocsize = req_bytes;
	/* Throw the no-memory error. */
	result = DeeError_Throw((DeeObject *)nomem_error);
	Dee_Decref(nomem_error);
	return result;
}

INTERN ATTR_COLD int DCALL err_no_active_exception(void) {
	return DeeError_Throwf(&DeeError_RuntimeError, "No active exception");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_subclass_final_type(DeeTypeObject *__restrict tp) {
	ASSERT_OBJECT(tp);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot create sub-class of final type `%k'", tp);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeObject_TypeAssertFailed)(DeeObject *self,
                                   DeeTypeObject *wanted_type) {
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(wanted_type);
	ASSERT(DeeType_Check(wanted_type));
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected instance of `%k', but got a `%k' object: %k",
	                       wanted_type, Dee_TYPE(self), self);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unimplemented_constructor_kw(DeeTypeObject *tp, size_t argc,
                                 DeeObject **argv, DeeObject *kw) {
	int result;
	DREF DeeObject *error_args[1], *error_ob;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char const *name               = tp->tp_name;
	if (!name)
		name = "<anonymous type>";
	if unlikely(unicode_printer_printf(&printer, "Constructor `%s(", name) < 0)
		goto err_printer;
	if unlikely(DeeFormat_PrintArgumentTypesKw(&unicode_printer_print,
	                                           &printer,
	                                           argc,
	                                           argv,
	                                           kw) < 0)
		goto err_printer;
	if unlikely(UNICODE_PRINTER_PRINT(&printer, ")' is not implemented") < 0)
		goto err_printer;
	/* Create the message string. */
	error_args[0] = unicode_printer_pack(&printer);
	if unlikely(!error_args[0])
		goto err;
	/* Pack the constructor argument tuple. */
	error_ob = DeeObject_New(&DeeError_TypeError, 1, error_args);
	Dee_Decref(error_args[0]);
	if unlikely(!error_ob)
		goto err;
	/* Throw the new error object. */
	result = DeeError_Throw(error_ob);
	Dee_Decref(error_ob);
	return result;
err_printer:
	unicode_printer_fini(&printer);
err:
	return -1;
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_divide_by_zero(DeeObject *a, DeeObject *b) {
	ASSERT_OBJECT(a);
	ASSERT_OBJECT(b);
	return DeeError_Throwf(&DeeError_DivideByZero,
	                       "Divide by Zero: `%k / %k'", a, b);
}

INTERN ATTR_COLD int DCALL
err_divide_by_zero_i(dssize_t a) {
	return DeeError_Throwf(&DeeError_DivideByZero,
	                       "Divide by Zero: `%Id / 0'", a);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_shift_negative(DeeObject *a, DeeObject *b, bool is_left_shift) {
	ASSERT_OBJECT(a);
	ASSERT_OBJECT(b);
	return DeeError_Throwf(&DeeError_NegativeShift,
	                       "Negative %s shift: `%k %s %k'",
	                       is_left_shift ? "left" : "right", a, b,
	                       is_left_shift ? "<<" : ">>");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_cannot_weak_reference(DeeObject *__restrict ob) {
	ASSERT_OBJECT(ob);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot create weak reference for instances of type `%k'",
	                       Dee_TYPE(ob));
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_reference_loop(DeeObject *a, DeeObject *b) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Reference loop between instance of %k and %k",
	                       Dee_TYPE(a), Dee_TYPE(b));
}

INTERN ATTR_COLD int DCALL err_cannot_lock_weakref(void) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Cannot lock weak reference");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_bytes_not_writable(DeeObject *__restrict UNUSED(bytes_ob)) {
	return DeeError_Throwf(&DeeError_BufferError,
	                       "The Bytes object is not writable");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unimplemented_operator(DeeTypeObject *__restrict tp, uint16_t operator_name) {
	struct opinfo *info = Dee_OperatorInfo(Dee_TYPE(tp), operator_name);
	ASSERT_OBJECT(tp);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Operator `%k.__%s__' is not implemented",
	                       tp, info ? info->oi_sname : Q3);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unimplemented_operator2(DeeTypeObject *__restrict tp,
                            uint16_t operator_name,
                            uint16_t operator_name2) {
	struct opinfo *info, *info2;
	info  = Dee_OperatorInfo(Dee_TYPE(tp), operator_name);
	info2 = Dee_OperatorInfo(Dee_TYPE(tp), operator_name2);
	ASSERT_OBJECT(tp);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Neither `%k.__%s__', nor `%k.__%s__' are implemented",
	                       tp, info ? info->oi_sname : Q3,
	                       tp, info2 ? info2->oi_sname : Q3);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unimplemented_operator3(DeeTypeObject *__restrict tp,
                            uint16_t operator_name,
                            uint16_t operator_name2,
                            uint16_t operator_name3) {
	struct opinfo *info, *info2, *info3;
	info  = Dee_OperatorInfo(Dee_TYPE(tp), operator_name);
	info2 = Dee_OperatorInfo(Dee_TYPE(tp), operator_name2);
	info3 = Dee_OperatorInfo(Dee_TYPE(tp), operator_name3);
	ASSERT_OBJECT(tp);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Neither `%k.__%s__', nor `%k.__%s__', nor `%k.__%s__' are implemented",
	                       tp, info ? info->oi_sname : Q3,
	                       tp, info2 ? info2->oi_sname : Q3,
	                       tp, info3 ? info3->oi_sname : Q3);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_index_out_of_bounds(DeeObject *__restrict self,
                        size_t index, size_t size) {
	ASSERT_OBJECT(self);
	return DeeError_Throwf(&DeeError_IndexError,
	                       "Index `%Iu' lies outside the valid bounds `0...%Iu' of sequence of type `%k'",
	                       index, size, Dee_TYPE(self));
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_index_out_of_bounds_ob(DeeObject *self, DeeObject *index) {
	return DeeError_Throwf(&DeeError_IndexError,
	                       "Index `%r' lies outside the valid bounds `0...%R' of sequence of type `%k'",
	                       index, DeeObject_SizeObject(self), Dee_TYPE(self));
}

INTERN ATTR_COLD int DCALL
err_va_index_out_of_bounds(size_t index, size_t size) {
	ASSERT(index >= size);
	return DeeError_Throwf(&DeeError_IndexError,
	                       "Index `%Iu' lies outside the valid bounds `0...%Iu' of varargs",
	                       index, size);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unbound_index(DeeObject *__restrict self, size_t index) {
	ASSERT_OBJECT(self);
	return DeeError_Throwf(&DeeError_UnboundItem,
	                       "Index `%Iu' of instance of `%k': %k has not been bound",
	                       index, Dee_TYPE(self), self);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_expected_single_character_string(DeeObject *__restrict str) {
	size_t length;
	ASSERT_OBJECT(str);
	ASSERT(DeeString_Check(str) || DeeBytes_Check(str));
	length = DeeString_Check(str) ? DeeString_WLEN(str) : DeeBytes_SIZE(str);
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Expected a single character but got %Iu characters in %r",
	                       length, str);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_expected_string_for_attribute(DeeObject *__restrict but_instead_got) {
	ASSERT_OBJECT(but_instead_got);
	ASSERT(!DeeString_Check(but_instead_got));
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected string for attribute, but got instance of `%k': %k",
	                       Dee_TYPE(but_instead_got), but_instead_got);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_integer_overflow(DeeObject *__restrict overflowing_object,
                     size_t cutoff_bits, bool positive_overflow) {
	ASSERT_OBJECT(overflowing_object);
	if (!cutoff_bits) {
		return DeeError_Throwf(&DeeError_IntegerOverflow,
		                       "%s integer overflow in %k",
		                       positive_overflow ? "positive" : "negative",
		                       overflowing_object);
	}
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "%s integer overflow after %Iu bits in %k",
	                       positive_overflow ? "positive" : "negative",
	                       cutoff_bits, overflowing_object);
}

INTERN ATTR_COLD int DCALL
err_integer_overflow_i(size_t cutoff_bits, bool positive_overflow) {
	if (!cutoff_bits) {
		return DeeError_Throwf(&DeeError_IntegerOverflow,
		                       "%s integer overflow",
		                       positive_overflow ? "positive" : "negative");
	}
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "%s integer overflow after %Iu bits",
	                       positive_overflow ? "positive" : "negative",
	                       cutoff_bits);
}

INTERN NONNULL((1, 2)) int FCALL
check_empty_keywords(DeeObject *kw, DeeTypeObject *tp_self) {
	if (DeeKwds_Check(kw)) {
		if (DeeKwds_SIZE(kw) != 0)
			goto err_no_keywords;
	} else {
		size_t temp = DeeObject_Size(kw);
		if unlikely(temp == (size_t)-1)
			goto err;
		if (temp != 0)
			goto err_no_keywords;
	}
	return 0;
err_no_keywords:
	err_keywords_not_accepted(tp_self, kw);
err:
	return -1;
}

INTERN NONNULL((1)) int FCALL
check_empty_keywords_obj(DeeObject *__restrict kw) {
	if (DeeKwds_Check(kw)) {
		if (DeeKwds_SIZE(kw) != 0)
			goto err_no_keywords;
	} else {
		size_t temp = DeeObject_Size(kw);
		if unlikely(temp == (size_t)-1)
			goto err;
		if (temp != 0)
			goto err_no_keywords;
	}
	return 0;
err_no_keywords:
	err_keywords_not_accepted(&DeeObject_Type, kw);
err:
	return -1;
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_keywords_not_accepted(DeeTypeObject *tp_self, DeeObject *kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Instance of %k does not accept keyword arguments %r",
	                       tp_self, kw);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_keywords_func_not_accepted(char const *__restrict name,
                               DeeObject *__restrict kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Function %s does not accept keyword arguments %r",
	                       name, kw);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_keywords_ctor_not_accepted(DeeTypeObject *tp_self, DeeObject *kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Constructor for %k does not accept keyword arguments %r",
	                       tp_self, kw);
}

INTERN ATTR_COLD int DCALL
err_keywords_bad_for_argc(size_t argc, size_t kwdc) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid keyword list containing %Iu keywords "
	                       "when only %Iu arguments were given",
	                       kwdc, argc);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_keywords_not_found(char const *__restrict keyword) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Missing argument %s",
	                       keyword);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_keywords_shadows_positional(char const *__restrict keyword) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Keyword argument %s has already been passed as positional",
	                       keyword);
}

INTERN ATTR_COLD int DCALL
err_invalid_segment_size(size_t segsz) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid segment size: %Iu",
	                       segsz);
}

INTERN ATTR_COLD int DCALL
err_invalid_distribution_count(size_t distcnt) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid distribution count: %Iu",
	                       distcnt);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_invalid_argc_missing_kw(char const *__restrict argument_name,
                            char const *function_name,
                            size_t argc_cur, size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Missing argument %s in call to function%s%s expecting %Iu arguments when %Iu w%s given",
		                       argument_name,
		                       function_name ? " " : "",
		                       function_name ? function_name : "",
		                       argc_min,
		                       argc_cur,
		                       argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Missing argument %s in call to function%s%s expecting between %Iu and %Iu arguments when %Iu w%s given",
		                       argument_name,
		                       function_name ? " " : "",
		                       function_name ? function_name : "",
		                       argc_min,
		                       argc_max,
		                       argc_cur,
		                       argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_argc(char const *function_name, size_t argc_cur,
                 size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%s expects %Iu arguments when %Iu w%s given",
		                       function_name ? " " : "", function_name ? function_name : "",
		                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%s expects between %Iu and %Iu arguments when %Iu w%s given",
		                       function_name ? " " : "", function_name ? function_name : "",
		                       argc_min, argc_max, argc_cur, argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_argc_len(char const *function_name, size_t function_size,
                     size_t argc_cur, size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%$s expects %Iu arguments when %Iu w%s given",
		                       function_size ? " " : "", function_size, function_name,
		                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%$s expects between %Iu and %Iu arguments when %Iu w%s given",
		                       function_size ? " " : "", function_size, function_name,
		                       argc_min, argc_max, argc_cur, argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_argc_va(char const *function_name, size_t argc_cur, size_t argc_min) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "function%s%s expects at least %Iu arguments when only %Iu w%s given",
	                       function_name ? " " : "", function_name ? function_name : "",
	                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_argc_unpack(DeeObject *__restrict unpack_object,
                        size_t argc_cur, size_t argc_min, size_t argc_max) {
	ASSERT_OBJECT(unpack_object);
	(void)unpack_object;
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_UnpackError,
		                       "Expected %Iu object%s when %Iu w%s given",
		                       argc_min, argc_min > 1 ? "s" : "", argc_cur,
		                       argc_min == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_UnpackError,
		                       "Expected between %Iu and %Iu objects when %Iu were given",
		                       argc_min, argc_max, argc_cur);
	}
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_unpack_size(DeeObject *__restrict unpack_object,
                        size_t need_size, size_t real_size) {
	ASSERT_OBJECT(unpack_object);
	(void)unpack_object;
	return DeeError_Throwf(&DeeError_UnpackError,
	                       "Expected %Iu object%s when %Iu w%s given",
	                       need_size, need_size > 1 ? "s" : "", real_size,
	                       real_size == 1 ? "as" : "ere");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_unpack_size_minmax(DeeObject *__restrict unpack_object,
                               size_t need_size_min, size_t need_size_max,
                               size_t real_size) {
	ASSERT_OBJECT(unpack_object);
	(void)unpack_object;
	return DeeError_Throwf(&DeeError_UnpackError,
	                       "Expected between %Iu and %Iu objects when %Iu w%s given",
	                       need_size_min, need_size_max, real_size,
	                       real_size == 1 ? "as" : "ere");
}

INTERN ATTR_COLD int DCALL
err_invalid_va_unpack_size(size_t need_size, size_t real_size) {
	return DeeError_Throwf(&DeeError_UnpackError,
	                       "Expected %Iu object%s when %Iu w%s given",
	                       need_size, need_size > 1 ? "s" : "", real_size,
	                       real_size == 1 ? "as" : "ere");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_invalid_unpack_iter_size(DeeObject *unpack_object,
                             DeeObject *unpack_iterator,
                             size_t need_size) {
	ASSERT_OBJECT(unpack_object);
	ASSERT_OBJECT(unpack_iterator);
	(void)unpack_object;
	(void)unpack_iterator;
	return DeeError_Throwf(&DeeError_UnpackError,
	                       "Expected %Iu object%s when at least %Iu w%s given",
	                       need_size, need_size > 1 ? "s" : "", need_size + 1,
	                       need_size == 0 ? "as" : "ere");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_invalid_unpack_iter_size_minmax(DeeObject *unpack_object, DeeObject *unpack_iterator,
                                    size_t need_size_min, size_t need_size_max) {
	ASSERT_OBJECT(unpack_object);
	ASSERT_OBJECT(unpack_iterator);
	(void)unpack_object;
	(void)unpack_iterator;
	return DeeError_Throwf(&DeeError_UnpackError,
	                       "Expected between %Iu and %Iu objects when at least %Iu w%s given",
	                       need_size_min, need_size_max, need_size_max + 1,
	                       need_size_max == 0 ? "as" : "ere");
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unbound_global(DeeModuleObject *__restrict module,
                   uint16_t global_index) {
	char const *name;
	ASSERT_OBJECT(module);
	ASSERT(DeeModule_Check(module));
	ASSERT(global_index < module->mo_globalc);
	name = DeeModule_GlobalName((DeeObject *)module, global_index);
	return DeeError_Throwf(&DeeError_UnboundLocal, /* XXX: UnboundGlobal? */
	                       "Unbound global variable `%s' from `%s'",
	                       name ? name : Q3,
	                       DeeString_STR(module->mo_name));
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_local(struct code_object *code, void *ip, uint16_t local_index) {
	char const *code_name = NULL;
	uint8_t *error;
	struct ddi_state state;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(local_index < code->co_localc);
	error = DeeCode_FindDDI((DeeObject *)code, &state, NULL,
	                        (code_addr_t)((instruction_t *)ip - code->co_code),
	                        DDI_STATE_FNOTHROW);
	if (DDI_ISOK(error)) {
		struct ddi_xregs *iter;
		DDI_STATE_DO(iter, &state) {
			if (local_index < iter->dx_lcnamc) {
				char *local_name;
				if (!code_name)
					code_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_name);
				if ((local_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_lcnamv[local_index])) != NULL) {
					if (!code_name)
						code_name = DeeCode_NAME(code);
					DeeError_Throwf(&DeeError_UnboundLocal,
					                "Unbound local variable `%s' %s%s",
					                local_name,
					                code_name ? "in function " : "",
					                code_name ? code_name : "");
					Dee_ddi_state_fini(&state);
					return -1;
				}
			}
		}
		DDI_STATE_WHILE(iter, &state);
		Dee_ddi_state_fini(&state);
	}
	if (!code_name)
		code_name = DeeCode_NAME(code);
	return DeeError_Throwf(&DeeError_UnboundLocal,
	                       "Unbound local variable %I16u%s%s",
	                       local_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_arg(struct code_object *code, void *ip, uint16_t arg_index) {
	char const *code_name;
	(void)ip;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(arg_index < code->co_argc_max);
	code_name = DeeCode_NAME(code);
	if (code->co_keywords) {
		struct string_object *kwname;
		kwname = code->co_keywords[arg_index];
		if (!DeeString_IsEmpty(kwname)) {
			return DeeError_Throwf(&DeeError_UnboundLocal,
			                       "Unbound argument %k%s%s",
			                       kwname,
			                       code_name ? " in function " : "",
			                       code_name ? code_name : "");
		}
	}
	return DeeError_Throwf(&DeeError_UnboundLocal,
	                       "Unbound argument %I16u%s%s",
	                       arg_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_readonly_local(struct code_object *code,
                   void *ip, uint16_t local_index) {
	char const *code_name = NULL;
	uint8_t *error;
	struct ddi_state state;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(local_index < code->co_localc);
	error = DeeCode_FindDDI((DeeObject *)code, &state, NULL,
	                        (code_addr_t)((instruction_t *)ip - code->co_code),
	                        DDI_STATE_FNOTHROW);
	if (DDI_ISOK(error)) {
		struct ddi_xregs *iter;
		DDI_STATE_DO(iter, &state) {
			if (local_index < iter->dx_lcnamc) {
				char *local_name;
				if (!code_name)
					code_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_name);
				if ((local_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_lcnamv[local_index])) != NULL) {
					if (!code_name)
						code_name = DeeCode_NAME(code);
					DeeError_Throwf(&DeeError_RuntimeError,
					                "Cannot modify read-only local variable `%s' %s%s",
					                local_name,
					                code_name ? "in function " : "",
					                code_name ? code_name : "");
					Dee_ddi_state_fini(&state);
					return -1;
				}
			}
		}
		DDI_STATE_WHILE(iter, &state);
		Dee_ddi_state_fini(&state);
	}
	if (!code_name)
		code_name = DeeCode_NAME(code);
	return DeeError_Throwf(&DeeError_RuntimeError,
	                       "Cannot modify read-only local variable %d%s%s",
	                       local_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_illegal_instruction(struct code_object *code, void *ip) {
	char const *code_name = DeeCode_NAME(code);
	if (!code_name)
		code_name = "<anonymous>";
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Illegal instruction at %s+%.4I32X",
	                       code_name, (uint32_t)((instruction_t *)ip - code->co_code));
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_requires_class(DeeTypeObject *__restrict tp_self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Needed a class when %k is only a regular type",
	                       tp_self);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_class_addr(DeeTypeObject *__restrict tp_self,
                       uint16_t addr) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid class address %I16u for %k",
	                       addr, tp_self);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_invalid_instance_addr(DeeTypeObject *tp_self,
                          DeeObject *UNUSED(self),
                          uint16_t addr) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid class instance address %I16u for %k",
	                       addr, tp_self);
}


INTERN ATTR_COLD NONNULL((1)) int DCALL
err_invalid_refs_size(DeeObject *code, size_t num_refs) {
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Code object expects %I16u references when %Iu were given",
	                       ((DeeCodeObject *)code)->co_refc, num_refs);
}



PRIVATE char const access_names[4][4] = {
	/* [ATTR_ACCESS_GET] = */ "get",
	/* [ATTR_ACCESS_DEL] = */ "del",
	/* [ATTR_ACCESS_SET] = */ "set",
	/* [?]               = */ "",
};

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unknown_attribute(DeeTypeObject *__restrict tp,
                      char const *__restrict name,
                      int access) {
	ASSERT_OBJECT(tp);
	ASSERT(name);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown attribute `%k.%s'",
	                       access_names[access & ATTR_ACCESS_MASK], tp, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unknown_attribute_len(DeeTypeObject *__restrict tp,
                          char const *__restrict name,
                          size_t namelen,
                          int access) {
	ASSERT_OBJECT(tp);
	ASSERT(!namelen || name);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown attribute `%k.%$s'",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       tp,
	                       namelen,
	                       name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unknown_attribute_lookup(DeeTypeObject *__restrict tp,
                             char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Unknown attribute `%k.%s'",
	                       tp, name);
}

INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nodoc_attribute(char const *base,
                    char const *__restrict name) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "No documentation found for `%s.%s'",
	                       base ? base : "?", name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_cant_access_attribute(DeeTypeObject *__restrict tp,
                          char const *__restrict name,
                          int access) {
	ASSERT_OBJECT(tp);
	ASSERT(name);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s attribute `%k.%s'",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       tp, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_cant_access_attribute_len(DeeTypeObject *__restrict tp,
                              char const *__restrict name,
                              size_t namelen,
                              int access) {
	ASSERT_OBJECT(tp);
	ASSERT(!namelen || name);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s attribute `%k.%$s'",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       tp, namelen, name);
}

PRIVATE ATTR_RETNONNULL WUNUSED char const *DCALL
get_desc_name(struct class_desc *__restrict desc) {
	return desc->cd_desc->cd_name
	       ? DeeString_STR(desc->cd_desc->cd_name)
	       : "<unnamed>";
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_cant_access_attribute_c(struct class_desc *__restrict desc,
                            char const *__restrict name,
                            int access) {
	ASSERT(desc);
	ASSERT(name);
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s attribute `%s.%s'",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       get_desc_name(desc), name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_attribute(DeeTypeObject *__restrict tp, char const *__restrict name) {
	ASSERT_OBJECT(tp);
	ASSERT(name);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%k.%s'",
	                       tp, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_attribute_c(struct class_desc *__restrict desc, char const *__restrict name) {
	ASSERT(desc);
	ASSERT(name);
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%s.%s'",
	                       get_desc_name(desc), name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_not_loaded_attr(DeeModuleObject *__restrict self,
                           char const *__restrict name, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s global variable `%s' of module `%k' that hasn't been loaded yet",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       name, self->mo_name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_not_loaded_attr_len(DeeModuleObject *__restrict self,
                               char const *__restrict name,
                               size_t namelen, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s global variable `%$s' of module `%k' that hasn't been loaded yet",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       namelen, name, self->mo_name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_no_such_global(DeeModuleObject *__restrict self,
                          char const *__restrict name, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown global variable: `%k.%s'",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       self->mo_name, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_no_such_global_len(DeeModuleObject *__restrict self,
                              char const *__restrict name,
                              size_t namelen, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown global variable: `%k.%$s'",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       self->mo_name, namelen, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_readonly_global(DeeModuleObject *__restrict self,
                           char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot modify read-only global variable: `%k.%s'",
	                       self->mo_name, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_cannot_read_property(DeeModuleObject *__restrict self,
                                char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot read global property: `%k.%s'",
	                       self->mo_name, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_cannot_delete_property(DeeModuleObject *__restrict self,
                                  char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot write global property: `%k.%s'",
	                       self->mo_name, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_cannot_write_property(DeeModuleObject *__restrict self,
                                 char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot write global property: `%k.%s'",
	                       self->mo_name, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unknown_key(DeeObject *__restrict map, DeeObject *__restrict key) {
	ASSERT_OBJECT(map);
	ASSERT_OBJECT(key);
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Could not find key `%k' in %k `%k'",
	                       key, Dee_TYPE(map), map);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unknown_key_str(DeeObject *__restrict map, char const *__restrict key) {
	ASSERT_OBJECT(map);
	ASSERT(key);
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Could not find key `%s' in %k `%k'",
	                       key, Dee_TYPE(map), map);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unknown_key_str_len(DeeObject *__restrict map, char const *__restrict key, size_t keylen) {
	ASSERT_OBJECT(map);
	ASSERT(key);
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Could not find key `%$s' in %k `%k'",
	                       keylen, key, Dee_TYPE(map), map);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_empty_sequence(DeeObject *__restrict seq) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Empty sequence of type `%k' encountered",
	                       Dee_TYPE(seq));
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_changed_sequence(DeeObject *__restrict seq) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_RuntimeError,
	                       "A sequence `%k' has changed while being iterated: `%k'",
	                       Dee_TYPE(seq), seq);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_immutable_sequence(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_SequenceError,
	                       "Instances of sequence type `%k' are immutable",
	                       Dee_TYPE(self));
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_fixedlength_sequence(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_SequenceError,
	                       "Instances of sequence type `%k' have a fixed length",
	                       Dee_TYPE(self));
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_item_not_found(DeeObject *seq, DeeObject *item) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Could not locate item `%k' in sequence `%k'",
	                       item, seq);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_index_not_found(DeeObject *seq, DeeObject *item) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_IndexError,
	                       "Could not locate item `%k' in sequence `%k'",
	                       item, seq);
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_class_protected_member(DeeTypeObject *__restrict class_type,
                           struct class_attribute *__restrict member) {
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	ASSERT(member);
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot access %s member `%k' of class `%k'",
	                       (member->ca_flag & CLASS_ATTRIBUTE_FPRIVATE) ? "private" : "public",
	                       member->ca_name, class_type);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_no_super_class(DeeTypeObject *__restrict type) {
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Type `%k' has no super-class", type);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_file_not_found(char const *__restrict filename) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "File `%s' could not be found",
	                       filename);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_file_not_found_ob(DeeObject *__restrict filename) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "File `%k' could not be found",
	                       filename);
}

#ifdef CONFIG_HOST_WINDOWS
INTERN ATTR_NOINLINE ATTR_COLD NONNULL((1)) int DCALL
err_system_error_code(char const *__restrict function,
                      unsigned long last_error) {
	DeeError_Throwf(&DeeError_SystemError,
	                "System function `%s' failed: %lx",
	                function, last_error);
	return -1;
}

INTERN ATTR_NOINLINE ATTR_COLD NONNULL((1)) int DCALL
err_system_error(char const *__restrict function) {
	return err_system_error_code(function, GetLastError());
}
#endif /* CONFIG_HOST_WINDOWS */



INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_sp(struct code_frame *__restrict frame, size_t access_sp) {
	return DeeError_Throwf(&DeeError_SegFault,
	                       "Unbound stack variable %Iu lies above active end %Iu",
	                       access_sp, frame->cf_stacksz);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_symid(struct code_frame *__restrict UNUSED(frame),
                      char category, uint16_t id) {
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Attempted to access invalid %cID %I16u",
	                       category, id);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_static(struct code_frame *__restrict frame, uint16_t sid) {
	return err_srt_invalid_symid(frame, 'S', sid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_const(struct code_frame *__restrict frame, uint16_t cid) {
	return err_srt_invalid_symid(frame, 'C', cid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_locale(struct code_frame *__restrict frame, uint16_t lid) {
	return err_srt_invalid_symid(frame, 'L', lid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_ref(struct code_frame *__restrict frame, uint16_t rid) {
	return err_srt_invalid_symid(frame, 'R', rid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_module(struct code_frame *__restrict frame, uint16_t mid) {
	return err_srt_invalid_symid(frame, 'M', mid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_global(struct code_frame *__restrict frame, uint16_t gid) {
	return err_srt_invalid_symid(frame, 'G', gid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_srt_invalid_extern(struct code_frame *__restrict frame, uint16_t mid, uint16_t gid) {
	if (mid >= frame->cf_func->fo_code->co_module->mo_importc)
		return err_srt_invalid_module(frame, mid);
	return err_srt_invalid_global(frame, gid);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C */
