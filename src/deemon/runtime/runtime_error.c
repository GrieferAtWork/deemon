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
#ifndef GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C
#define GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_TRYMALLOC */
#include <deemon/bytes.h>           /* DeeBytes_Check, DeeBytes_SIZE */
#include <deemon/code.h>            /* DeeCode_*, Dee_DDI_ISOK, Dee_DDI_STATE_*, Dee_code_frame, Dee_code_object, Dee_ddi_*, code_addr_t, instruction_t */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/error_types.h>     /* DeeError_NoMemory_instance, DeeNoMemoryErrorObject */
#include <deemon/format.h>          /* DeeFormat_PrintArgumentTypesKw, PRF* */
#include <deemon/kwds.h>            /* DeeKwds*, Dee_kwds_object */
#include <deemon/module.h>          /* DeeModule*, Dee_module_object */
#include <deemon/object.h>          /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_TYPE */
#include <deemon/string.h>          /* DeeString*, Dee_UNICODE_PRINTER_INIT, Dee_UNICODE_PRINTER_PRINT, Dee_unicode_printer* */
#include <deemon/system-features.h> /* access */
#include <deemon/type.h>            /* DeeObject_Init, DeeTypeType_GetOperatorById, DeeType_GetName, DeeType_Type, Dee_operator_t, Dee_opinfo */

#include <hybrid/typecore.h> /* __SIZE_WIDTH__ */

#include "runtime_error.h"

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#define Q3 "??" "?"
#define OPNAME(opname) "operator " opname


/* Throw a bad-allocation error for `req_bytes' bytes.
 * @return: -1: Always returns -1. */
PUBLIC ATTR_COLD int (DCALL Dee_BadAlloc)(size_t req_bytes) {
	DeeNoMemoryErrorObject *nomem_error;
	nomem_error = DeeObject_TRYMALLOC(DeeNoMemoryErrorObject);
	if (!nomem_error) {
		/* If we can't even allocate the no-memory
		 * object, throw a static instance. */
		return DeeError_Throw(&DeeError_NoMemory_instance);
	}
	DeeObject_Init(nomem_error, &DeeError_NoMemory);
	nomem_error->e_msg    = NULL;
	nomem_error->e_cause      = NULL;
	nomem_error->nm_allocsize = req_bytes;
	/* Throw the no-memory error. */
	return DeeError_ThrowInherited((DeeObject *)nomem_error);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeObject_TypeAssertFailed)(DeeObject *self,
                                   DeeTypeObject *required_type) {
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(required_type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected instance of `%r', but got a `%r' object: %k",
	                       required_type, Dee_TYPE(self), self);
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeObject_TypeAssertFailed2)(DeeObject *self,
                                    DeeTypeObject *required_type1,
                                    DeeTypeObject *required_type2) {
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(required_type1);
	ASSERT_OBJECT(required_type2);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected instance of `%r' or `%r', but got a `%r' object: %k",
	                       required_type1, required_type2,
	                       Dee_TYPE(self), self);
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3, 4)) int
(DCALL DeeObject_TypeAssertFailed3)(DeeObject *self,
                                    DeeTypeObject *required_type1,
                                    DeeTypeObject *required_type2,
                                    DeeTypeObject *required_type3) {
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(required_type1);
	ASSERT_OBJECT(required_type2);
	ASSERT_OBJECT(required_type3);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected instance of `%r', `%r' or `%r', but got a `%r' object: %k",
	                       required_type1, required_type2, required_type3,
	                       Dee_TYPE(self), self);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_unimplemented_constructor_kw)(DeeTypeObject *tp, size_t argc,
                                         DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *error_args[1], *error_ob;
	struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
	char const *name = DeeType_GetName(tp);
	if unlikely(Dee_unicode_printer_printf(&printer, "Constructor `%s(", name) < 0)
		goto err_printer;
	if unlikely(DeeFormat_PrintArgumentTypesKw(&Dee_unicode_printer_print,
	                                           &printer,
	                                           argc,
	                                           argv,
	                                           kw) < 0)
		goto err_printer;
	if unlikely(Dee_UNICODE_PRINTER_PRINT(&printer, ")' is not implemented") < 0)
		goto err_printer;
	/* Create the message string. */
	error_args[0] = Dee_unicode_printer_pack(&printer);
	if unlikely(!error_args[0])
		goto err;
	/* Pack the constructor argument tuple. */
	error_ob = DeeObject_New(&DeeError_TypeError, 1, error_args);
	Dee_Decref(error_args[0]);
	if unlikely(!error_ob)
		goto err;
	/* Throw the new error object. */
	return DeeError_ThrowInherited(error_ob);
err_printer:
	Dee_unicode_printer_fini(&printer);
err:
	return -1;
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_cannot_weak_reference)(DeeObject *__restrict ob) {
	ASSERT_OBJECT(ob);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot create weak reference for instances of type `%k'",
	                       Dee_TYPE(ob));
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_reference_loop)(DeeObject *a, DeeObject *b) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Reference loop between instance of %k and %k",
	                       Dee_TYPE(a), Dee_TYPE(b));
}

INTERN ATTR_COLD int (DCALL err_cannot_lock_weakref)(void) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Cannot lock weak reference");
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_bytes_not_writable)(DeeObject *__restrict UNUSED(bytes_ob)) {
	return DeeError_Throwf(&DeeError_BufferError,
	                       "The Bytes object is not writable");
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_unimplemented_operator)(DeeTypeObject const *__restrict tp,
                                   Dee_operator_t operator_name) {
	struct Dee_opinfo const *info;
	info = DeeTypeType_GetOperatorById(Dee_TYPE(tp), operator_name);
	ASSERT_OBJECT(tp);
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Operator `%r." OPNAME("%s") "' is not implemented",
	                       tp, info ? info->oi_sname : Q3);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_expected_single_character_string)(DeeObject *__restrict str) {
	size_t length;
	ASSERT_OBJECT(str);
	ASSERT(DeeString_Check(str) || DeeBytes_Check(str));
	length = DeeString_Check(str) ? DeeString_WLEN(str) : DeeBytes_SIZE(str);
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Expected a single character but got %" PRFuSIZ " characters in %r",
	                       length, str);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_expected_string_for_attribute)(DeeObject *__restrict but_instead_got) {
	ASSERT_OBJECT(but_instead_got);
	ASSERT(!DeeString_Check(but_instead_got));
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected string for attribute, but got instance of `%k': %k",
	                       Dee_TYPE(but_instead_got), but_instead_got);
}

#if __SIZE_WIDTH__ == 8
#define SIZE_WIDTH_STR "8"
#elif __SIZE_WIDTH__ == 16
#define SIZE_WIDTH_STR "16"
#elif __SIZE_WIDTH__ == 32
#define SIZE_WIDTH_STR "32"
#elif __SIZE_WIDTH__ == 64
#define SIZE_WIDTH_STR "64"
#else /* __SIZE_WIDTH__ == ... */
#define SIZE_WIDTH_STR PP_STR(__SIZE_WIDTH__)
#endif /* __SIZE_WIDTH__ != ... */

INTERN NONNULL((1)) int
(DFCALL check_empty_keywords_obj)(DeeObject *__restrict kw) {
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

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_keywords_not_accepted)(DeeTypeObject *tp_self, DeeObject *kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Instance of %k does not accept keyword arguments %r",
	                       tp_self, kw);
}

INTERN ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL err_keywords_func_not_accepted_string)(DeeTypeObject *tp_self,
                                              char const *__restrict name,
                                              DeeObject *__restrict kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Function `%r.%s' does not accept keyword arguments %r",
	                       tp_self, name, kw);
}

INTERN ATTR_COLD NONNULL((1, 2, 4)) int
(DCALL err_keywords_func_not_accepted_string_len)(DeeTypeObject *tp_self,
                                                  char const *__restrict name,
                                                  size_t namelen,
                                                  DeeObject *__restrict kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Function `%r.%$s' does not accept keyword arguments %r",
	                       tp_self, namelen, name, kw);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_keywords_ctor_not_accepted)(DeeTypeObject *tp_self, DeeObject *kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Constructor for `%r' does not accept keyword arguments %r",
	                       tp_self, kw);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_classmember_requires_1_argument_string)(DeeTypeObject *tp_self, char const *__restrict name) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Class member `%r.%s' must be called with exactly 1 argument",
	                       tp_self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_classmember_requires_1_argument_string_len)(DeeTypeObject *tp_self,
                                                       char const *__restrict name, size_t namelen) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Class member `%r.%$s' must be called with exactly 1 argument",
	                       tp_self, namelen, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_classproperty_requires_1_argument_string)(DeeTypeObject *tp_self, char const *__restrict name) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Class property `%r.%s' must be called with exactly 1 argument",
	                       tp_self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_classproperty_requires_1_argument_string_len)(DeeTypeObject *tp_self,
                                                         char const *__restrict name, size_t namelen) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Class property `%r.%$s' must be called with exactly 1 argument",
	                       tp_self, namelen, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_classmethod_requires_at_least_1_argument_string)(DeeTypeObject *tp_self, char const *__restrict name) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Class method `%r.%s' must be called with at least 1 argument",
	                       tp_self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_classmethod_requires_at_least_1_argument_string_len)(DeeTypeObject *tp_self,
                                                                char const *__restrict name, size_t namelen) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Class method `%r.%$s' must be called with at least 1 argument",
	                       tp_self, namelen, name);
}


INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_keywords_bad_for_argc)(struct Dee_kwds_object *kwds,
                                  size_t argc, DeeObject *const *argv) {
	ASSERT_OBJECT_TYPE_EXACT(kwds, &DeeKwds_Type);
	(void)argv;
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid keyword list containing %" PRFuSIZ " keywords "
	                       "when only %" PRFuSIZ " arguments were given",
	                       DeeKwds_SIZE(kwds), argc);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_keywords_shadows_positional)(DeeObject *__restrict keyword) {
	ASSERT_OBJECT_TYPE_EXACT(keyword, &DeeString_Type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Keyword argument %r has already been passed as positional",
	                       keyword);
}

INTERN ATTR_COLD int
(DCALL err_invalid_segment_size)(size_t segsz) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid segment size: %" PRFuSIZ,
	                       segsz);
}

INTERN ATTR_COLD int
(DCALL err_invalid_distribution_count)(size_t distcnt) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid distribution count: %" PRFuSIZ,
	                       distcnt);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_argc_missing_kw)(char const *__restrict argument_name,
                                    char const *function_name,
                                    size_t argc_cur, size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Missing argument %s in call to function%s%s expecting "
		                       "%" PRFuSIZ " arguments when %" PRFuSIZ " w%s given",
		                       argument_name,
		                       function_name ? " " : "",
		                       function_name ? function_name : "",
		                       argc_min,
		                       argc_cur,
		                       argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Missing argument %s in call to function%s%s expecting between "
		                       "%" PRFuSIZ " and %" PRFuSIZ " arguments when %" PRFuSIZ " w%s given",
		                       argument_name,
		                       function_name ? " " : "",
		                       function_name ? function_name : "",
		                       argc_min,
		                       argc_max,
		                       argc_cur,
		                       argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD int
(DCALL err_invalid_argc)(char const *function_name, size_t argc_cur,
                         size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%s expects %" PRFuSIZ " arguments when %" PRFuSIZ " w%s given",
		                       function_name ? " " : "", function_name ? function_name : "",
		                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%s expects between %" PRFuSIZ " and %" PRFuSIZ " "
		                       "arguments when %" PRFuSIZ " w%s given",
		                       function_name ? " " : "", function_name ? function_name : "",
		                       argc_min, argc_max, argc_cur, argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD int
(DCALL err_invalid_argc_len)(char const *function_name, size_t function_size,
                             size_t argc_cur, size_t argc_min, size_t argc_max) {
	if (argc_min == argc_max) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%$s expects %" PRFuSIZ " arguments when %" PRFuSIZ " w%s given",
		                       function_size ? " " : "", function_size, function_name,
		                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "function%s%$s expects between %" PRFuSIZ " and %" PRFuSIZ " "
		                       "arguments when %" PRFuSIZ " w%s given",
		                       function_size ? " " : "", function_size, function_name,
		                       argc_min, argc_max, argc_cur, argc_cur == 1 ? "as" : "ere");
	}
}

INTERN ATTR_COLD int
(DCALL err_invalid_argc_va)(char const *function_name, size_t argc_cur, size_t argc_min) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "function%s%s expects at least %" PRFuSIZ " "
	                       "arguments when only %" PRFuSIZ " w%s given",
	                       function_name ? " " : "", function_name ? function_name : "",
	                       argc_min, argc_cur, argc_cur == 1 ? "as" : "ere");
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_unbound_global)(DeeModuleObject *__restrict mod,
                           uint16_t global_index) {
	char const *name;
	ASSERT_OBJECT(mod);
	ASSERT(DeeModule_Check(mod));
	ASSERT(global_index < mod->mo_globalc);
	name = DeeModule_GlobalName(mod, global_index);
	return DeeError_Throwf(&DeeError_UnboundLocal, /* XXX: UnboundGlobal? */
	                       "Unbound global variable `%s' from `%s'",
	                       name ? name : Q3,
	                       DeeModule_GetShortName(mod));
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_unbound_local)(struct Dee_code_object *code, void *ip, uint16_t local_index) {
	char const *code_name = NULL;
	uint8_t *error;
	struct Dee_ddi_state state;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(local_index < code->co_localc);
	error = DeeCode_FindDDI(Dee_AsObject(code), &state, NULL,
	                        (code_addr_t)((instruction_t *)ip - code->co_code),
	                        Dee_DDI_STATE_FNOTHROW);
	if (Dee_DDI_ISOK(error)) {
		struct Dee_ddi_xregs *iter;
		Dee_DDI_STATE_DO(iter, &state) {
			if (local_index < iter->dx_lcnamc) {
				char const *local_name;
				if (!code_name)
					code_name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_base.dr_name);
				if ((local_name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_lcnamv[local_index])) != NULL) {
					if (!code_name)
						code_name = DeeCode_NAME(code);
					DeeError_Throwf(&DeeError_UnboundLocal,
					                "Unbound local variable `%s'%s%s",
					                local_name,
					                code_name ? " in function " : "",
					                code_name ? code_name : "");
					Dee_ddi_state_fini(&state);
					return -1;
				}
			}
		}
		Dee_DDI_STATE_WHILE(iter, &state);
		Dee_ddi_state_fini(&state);
	}
	if (!code_name)
		code_name = DeeCode_NAME(code);
	return DeeError_Throwf(&DeeError_UnboundLocal,
	                       "Unbound local variable %" PRFu16 "%s%s",
	                       local_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_unbound_static)(struct Dee_code_object *code, void *ip, uint16_t static_index) {
	char const *code_name;
	char const *symbol_name;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(static_index < code->co_refstaticc);
	(void)ip;
	code_name   = DeeCode_NAME(code);
	symbol_name = DeeCode_GetRSymbolName(Dee_AsObject(code), static_index);
	if (symbol_name) {
		return DeeError_Throwf(&DeeError_UnboundLocal,
		                       "Unbound static variable `%s'%s%s",
		                       symbol_name,
		                       code_name ? " in function " : "",
		                       code_name ? code_name : "");
	}
	return DeeError_Throwf(&DeeError_UnboundLocal,
	                       "Unbound static variable %" PRFu16 "%s%s",
	                       static_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_unbound_arg)(struct Dee_code_object *code, void *ip, uint16_t arg_index) {
	char const *code_name;
	(void)ip;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(arg_index < code->co_argc_max);
	code_name = DeeCode_NAME(code);
	if (code->co_keywords) {
		DeeStringObject *kwname;
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
	                       "Unbound argument %" PRFu16 "%s%s",
	                       arg_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_readonly_local)(struct Dee_code_object *code,
                           void *ip, uint16_t local_index) {
	char const *code_name = NULL;
	uint8_t *error;
	struct Dee_ddi_state state;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(local_index < code->co_localc);
	error = DeeCode_FindDDI(Dee_AsObject(code), &state, NULL,
	                        (code_addr_t)((instruction_t *)ip - code->co_code),
	                        Dee_DDI_STATE_FNOTHROW);
	if (Dee_DDI_ISOK(error)) {
		struct Dee_ddi_xregs *iter;
		Dee_DDI_STATE_DO(iter, &state) {
			if (local_index < iter->dx_lcnamc) {
				char const *local_name;
				if (!code_name)
					code_name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_base.dr_name);
				if ((local_name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_lcnamv[local_index])) != NULL) {
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
		Dee_DDI_STATE_WHILE(iter, &state);
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

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_illegal_instruction)(struct Dee_code_object *code, void *ip) {
	uint32_t offset;
	char const *code_name = DeeCode_NAME(code);
	if (!code_name)
		code_name = "<anonymous>";
	offset = (uint32_t)((instruction_t *)ip - code->co_code);
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Illegal instruction at %s+%.4" PRFX32,
	                       code_name, offset);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_requires_class)(DeeTypeObject *__restrict tp_self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Needed a class when %k is only a regular type",
	                       tp_self);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_class_addr)(DeeTypeObject *__restrict tp_self,
                               uint16_t addr) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid class address %" PRFu16 " for %k",
	                       addr, tp_self);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_invalid_instance_addr)(DeeTypeObject *tp_self,
                                  DeeObject *UNUSED(self),
                                  uint16_t addr) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid class instance address %" PRFu16 " for %k",
	                       addr, tp_self);
}


INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_refs_size)(struct Dee_code_object *__restrict code, size_t num_refs) {
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Code object expects %" PRFu16 " references when %" PRFuSIZ " were given",
	                       code->co_refc, num_refs);
}



PRIVATE char const access_names[4][4] = {
	/* [ATTR_ACCESS_GET] = */ "get",
	/* [ATTR_ACCESS_DEL] = */ "del",
	/* [ATTR_ACCESS_SET] = */ "set",
	/* [?]               = */ "",
};

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_not_loaded_attr_string)(DeeModuleObject *__restrict self,
                                          char const *__restrict name, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s global variable `%s' of module `%k' that hasn't been loaded yet",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       name, self);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_not_loaded_attr_string_len)(DeeModuleObject *__restrict self,
                                              char const *__restrict name,
                                              size_t namelen, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s global variable `%$s' of module `%k' that hasn't been loaded yet",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       namelen, name, self);
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_no_such_global)(struct Dee_module_object *self,
                                  DeeObject *name, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown global variable: %r.%k",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_no_such_global_string)(DeeModuleObject *__restrict self,
                                         char const *__restrict name, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown global variable: %r.%s",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_no_such_global_string_len)(DeeModuleObject *__restrict self,
                                             char const *__restrict name,
                                             size_t namelen, int access) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown global variable: %r.%$s",
	                       access_names[access & ATTR_ACCESS_MASK],
	                       self, namelen, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_readonly_global_string)(DeeModuleObject *__restrict self,
                                          char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot modify read-only global variable: %r.%s",
	                       self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_cannot_read_property_string)(DeeModuleObject *__restrict self,
                                               char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot read global property: %r.%s",
	                       self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_cannot_delete_property_string)(DeeModuleObject *__restrict self,
                                                 char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot write global property: %r.%s",
	                       self, name);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_module_cannot_write_property_string)(DeeModuleObject *__restrict self,
                                                char const *__restrict name) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot write global property: %r.%s",
	                       self, name);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_changed_sequence)(DeeObject *__restrict seq) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_RuntimeError,
	                       "A sequence `%k' has changed while being iterated: `%k'",
	                       Dee_TYPE(seq), seq);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_no_super_class)(DeeTypeObject *__restrict type) {
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Type `%k' has no super-class", type);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_file_not_found_string)(char const *__restrict filename) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "File `%s' could not be found",
	                       filename);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_file_not_found)(DeeObject *__restrict filename) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "File `%k' could not be found",
	                       filename);
}



INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_sp)(struct Dee_code_frame *__restrict frame, size_t access_sp) {
	return DeeError_Throwf(&DeeError_SegFault,
	                       "Unbound stack variable %" PRFuSIZ " lies above active end %" PRFuSIZ,
	                       access_sp, frame->cf_stacksz);
}

PRIVATE ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_symid)(struct Dee_code_frame *__restrict UNUSED(frame),
                              char category, uint16_t id) {
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Attempted to access invalid %cID %" PRFu16,
	                       category, id);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_static)(struct Dee_code_frame *__restrict frame, uint16_t sid) {
	return err_srt_invalid_symid(frame, 'S', sid);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_const)(struct Dee_code_frame *__restrict frame, uint16_t cid) {
	return err_srt_invalid_symid(frame, 'C', cid);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_locale)(struct Dee_code_frame *__restrict frame, uint16_t lid) {
	return err_srt_invalid_symid(frame, 'L', lid);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_ref)(struct Dee_code_frame *__restrict frame, uint16_t rid) {
	return err_srt_invalid_symid(frame, 'R', rid);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_module)(struct Dee_code_frame *__restrict frame, uint16_t mid) {
	return err_srt_invalid_symid(frame, 'M', mid);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_global)(struct Dee_code_frame *__restrict frame, uint16_t gid) {
	return err_srt_invalid_symid(frame, 'G', gid);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_srt_invalid_extern)(struct Dee_code_frame *__restrict frame, uint16_t mid, uint16_t gid) {
	if (mid >= frame->cf_func->fo_code->co_module->mo_importc)
		return err_srt_invalid_module(frame, mid);
	return err_srt_invalid_global(frame, gid);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_C */
