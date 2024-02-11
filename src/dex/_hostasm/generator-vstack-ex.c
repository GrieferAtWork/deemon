/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_EX_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_EX_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/property.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>
#include <deemon/weakref.h>

DECL_BEGIN

#ifndef CONFIG_HAVE_strchrnul
#define CONFIG_HAVE_strchrnul
#undef strchrnul
#define strchrnul dee_strchrnul
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
dee_strchrnul(char const *haystack, int needle) {
	for (; *haystack; ++haystack) {
		if ((unsigned char)*haystack == (unsigned char)needle)
			break;
	}
	return (char *)haystack;
}
#endif /* !CONFIG_HAVE_strchrnul */

/************************************************************************/
/* HIGH-LEVEL VSTACK CONTROLS                                           */
/************************************************************************/

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err


/* Pop "kw" (as used for `DeeObject_CallKw') and assert that it is NULL or empty:
 * >> if (__builtin_constant_p(kw) ? kw != NULL : 1) {
 * >>     size_t kw_length;
 * >>     if (DeeKwds_Check(kw)) {
 * >>         kw_length = DeeKwds_SIZE(kw);
 * >>     } else {
 * >>         kw_length = DeeObject_Size(kw);
 * >>         if unlikely(kw_length == (size_t)-1)
 * >>             goto err;
 * >>     }
 * >>     if (kw_length != 0) {
 * >>         ...
 * >>     }
 * >> }
 */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vpop_empty_kwds(struct Dee_function_generator *__restrict self) {
	DREF struct Dee_memstate *enter_state;
	DREF struct Dee_memstate *leave_state;
	struct Dee_memval *kw_mval;

	/* Check for simple case: compile-time constant NULL */
	ASSERT(self->fg_state->ms_stackc >= 1);
	DO(Dee_function_generator_vdirect1(self));
	kw_mval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(kw_mval)) {
		/* Special case: keyword arguments are described by a compile-time constant. */
		DeeObject *kw = Dee_memval_const_getobj(kw_mval);
		if (kw != NULL)
			DO(libhostasm_rt_assert_empty_kw(kw));
		return Dee_function_generator_vpop(self);
	}
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Generate inline code. */
		struct Dee_memloc loc_DeeKwds_Type, loc_kwds_ob_type, loc_size;
		struct Dee_host_symbol *Lnot_kwds;
		struct Dee_host_symbol *Lgot_size;
		Lnot_kwds = Dee_function_generator_newsym_named(self, ".Lnot_kwds");
		if unlikely(!Lnot_kwds)
			goto err;
		Lgot_size = Dee_function_generator_newsym_named(self, ".Lgot_size");
		if unlikely(!Lgot_size)
			goto err;
		DO(Dee_function_generator_vdup(self));             /* kw, kw */
		DO(Dee_function_generator_voptypeof(self, false)); /* kw, type(kw) */
		DO(Dee_function_generator_vdirect1(self));         /* kw, type(kw) */
		loc_kwds_ob_type = *Dee_function_generator_vtopdloc(self);
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));
		DO(Dee_function_generator_vpop(self)); /* kw */
		Dee_memloc_init_const(&loc_DeeKwds_Type, &DeeKwds_Type);
		DO(Dee_function_generator_gjcc(self, &loc_kwds_ob_type, &loc_DeeKwds_Type, false,
		                                NULL, Lnot_kwds, NULL));
		enter_state = self->fg_state; /* kw */
		Dee_memstate_incref(enter_state);
		EDO(err_enter_state, Dee_function_generator_vdup(self));                                   /* kw, kw */
		EDO(err_enter_state, Dee_function_generator_vind(self, offsetof(DeeKwdsObject, kw_size))); /* kw, kw->kw_size */
		EDO(err_enter_state, Dee_function_generator_vreg(self, NULL));                             /* kw, reg:kw->kw_size */
		if (self->fg_sect == &self->fg_block->bb_hcold) {
			/* >>     jmp .Lgot_size
			 * >> .Lnot_kwds:
			 * >>     ...
			 * >> .Lgot_size: */
			EDO(err_enter_state, Dee_function_generator_gjmp(self, Lgot_size));
			leave_state = self->fg_state; /* Inherit reference */
			self->fg_state = enter_state; /* Inherit reference */
			Dee_host_symbol_setsect(Lnot_kwds, self->fg_sect);       /* kw */
			EDO(err_leave_state, Dee_function_generator_vdup(self)); /* kw, kw */
			EDO(err_leave_state, Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SIZE, 1)); /* kw, kw */
			EDO(err_leave_state, Dee_function_generator_vcallapi(self, &DeeObject_Size, VCALL_CC_M1INT, 1)); /* kw, size */
			EDO(err_leave_state, Dee_function_generator_vmorph(self, leave_state));
			EDO(err_leave_state, Dee_function_generator_gjmp(self, Lgot_size));
			Dee_host_symbol_setsect(Lgot_size, self->fg_sect);
		} else {
			struct Dee_host_section *old_text;
			/* >> .section .cold
			 * >> .Lnot_kwds:
			 * >>     ...
			 * >>     jmp .Lgot_size
			 * >> .section .text
			 * >> .Lgot_size: */
			HA_printf(".section .cold\n");
			old_text = self->fg_sect;
			self->fg_sect = &self->fg_block->bb_hcold;
			leave_state = self->fg_state; /* Inherit reference */
			self->fg_state = enter_state; /* Inherit reference */
			Dee_host_symbol_setsect(Lnot_kwds, self->fg_sect);       /* kw */
			EDO(err_leave_state, Dee_function_generator_vdup(self)); /* kw, kw */
			EDO(err_leave_state, Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_SIZE, 1)); /* kw, kw */
			EDO(err_leave_state, Dee_function_generator_vcallapi(self, &DeeObject_Size, VCALL_CC_M1INT, 1)); /* kw, size */
			EDO(err_leave_state, Dee_function_generator_vmorph(self, leave_state));
			EDO(err_leave_state, Dee_function_generator_gjmp(self, Lgot_size));
			HA_printf(".section .text\n");
			self->fg_sect = old_text;
			Dee_host_symbol_setsect(Lgot_size, self->fg_sect);
		}
		Dee_memstate_decref(self->fg_state);
		self->fg_state = leave_state; /* Inherit reference */
		DO(Dee_function_generator_vdirect1(self));         /* kw, size */
		loc_size = *Dee_function_generator_vtopdloc(self); /* kw, size */
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));
		DO(Dee_function_generator_vpop(self)); /* kw */

		if (self->fg_sect == &self->fg_block->bb_hcold) {
			struct Dee_host_symbol *Lsize_is_zero;
			/* >>     jz <loc_size>, .Lsize_is_zero
			 * >>     ...
			 * >> .Lsize_is_zero: */
			Lsize_is_zero = Dee_function_generator_newsym_named(self, ".Lsize_is_zero");
			if unlikely(!Lsize_is_zero)
				goto err;
			DO(Dee_function_generator_gjz(self, &loc_size, Lsize_is_zero));
			enter_state = self->fg_state;
			Dee_memstate_incref(enter_state);
			EDO(err_enter_state, Dee_function_generator_vcallapi(self, &libhostasm_rt_err_nonempty_kw, VCALL_CC_EXCEPT, 1));
			Dee_host_symbol_setsect(Lsize_is_zero, self->fg_sect);
		} else {
			struct Dee_host_symbol *Lsize_is_not_zero;
			struct Dee_host_section *old_text;
			/* >>     jnz <loc_size>, .Lsize_is_not_zero
			 * >> .section .cold
			 * >> .Lsize_is_not_zero:
			 * >>     ...
			 * >> .section .text
			 */
			Lsize_is_not_zero = Dee_function_generator_newsym_named(self, ".Lsize_is_not_zero");
			if unlikely(!Lsize_is_not_zero)
				goto err;
			DO(Dee_function_generator_gjnz(self, &loc_size, Lsize_is_not_zero));
			enter_state = self->fg_state;
			Dee_memstate_incref(enter_state);
			HA_printf(".section .cold\n");
			old_text = self->fg_sect;
			self->fg_sect = &self->fg_block->bb_hcold;
			Dee_host_symbol_setsect(Lsize_is_not_zero, self->fg_sect);
			EDO(err_enter_state, Dee_function_generator_vcallapi(self, &libhostasm_rt_err_nonempty_kw, VCALL_CC_EXCEPT, 1));
			HA_printf(".section .text\n");
			self->fg_sect = old_text;
		}
		Dee_memstate_decref(self->fg_state);
		self->fg_state = enter_state;             /* kw */
		return Dee_function_generator_vpop(self); /* - */
	}

	/* Use an API function to do the assert for us. */
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_assert_empty_kw, VCALL_CC_INT, 1);
err_leave_state:
	Dee_memstate_decref(leave_state);
	goto err;
err_enter_state:
	Dee_memstate_decref(enter_state);
err:
	return -1;
}


struct docinfo {
	char const      *di_doc; /* [0..1] Doc string. */
	DeeModuleObject *di_mod; /* [0..1] Associated module. */
	DeeTypeObject   *di_typ; /* [0..1] Associated type. */
};

#define isnulorlf(ch) ((ch) == '\0' || (ch) == '\n')

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) char const *DCALL
seek_after_unescaped_char(char const *iter, char findme) {
	while (!isnulorlf(*iter)) {
		char ch = *iter++;
		if (ch == findme) {
			break;
		} else if (ch == '\\') {
			if (!isnulorlf(*iter))
				++iter;
		} else if (ch == '{') {
			iter = seek_after_unescaped_char(iter, '}');
		}
	}
	return iter;
}

struct typexpr_parser {
	char const                    *txp_iter; /* [1..1] Pointer into type information. */
	struct docinfo const          *txp_info; /* [1..1][const] Doc information. */
	struct Dee_function_generator *txp_gen;  /* [1..1][const] Function generator. */
};

struct type_expression_name {
	char const           *ten_start; /* [1..1] Start of name */
	char const           *ten_end;   /* [1..1] End of name */
	DREF DeeStringObject *ten_str;   /* [0..1] Name string (in case an extended name was used) */
};

#define type_expression_name_fini(self) Dee_XDecref((self)->ten_str)

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_expression_name_unescape(struct type_expression_name *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char const *iter, *end, *flush_start;

	/* Parse the string and unescape special symbols. */
	iter = self->ten_start;
	end  = self->ten_end;
	flush_start = iter;
	while (iter < end) {
		char ch = *iter++;
		if (ch == '\\') { /* Remove every first '\'-character */
			if unlikely(unicode_printer_print(&printer, flush_start,
			                                  (size_t)((iter - 1) - flush_start)) < 0)
				goto err_printer;
			flush_start = iter;
			if (iter < end)
				++iter; /* Don't remove the character following '\', even if it's another '\' */
		}
	}
	if (flush_start < end) {
		if unlikely(unicode_printer_print(&printer, flush_start,
		                                  (size_t)(end - flush_start)) < 0)
			goto err_printer;
	}

	/* Pack the unicode string */
	self->ten_str = (DREF DeeStringObject *)unicode_printer_pack(&printer);
	if unlikely(!self->ten_str)
		goto err;
	self->ten_start = DeeString_AsUtf8((DeeObject *)self->ten_str);
	if unlikely(!self->ten_start)
		goto err_ten_str;
	self->ten_end = self->ten_start + WSTR_LENGTH(self->ten_start);
	return 0;
err_ten_str:
	Dee_Decref(self->ten_str);
	goto err;
err_printer:
	unicode_printer_fini(&printer);
err:
	return -1;
}

/* Parse a type-expression `<NAME>' element
 * @return: 0 : Success (*result was initialized)
 * @return: -1: An error was thrown (*result is in an undefined state) */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
typexpr_parser_parsename(struct typexpr_parser *__restrict self,
                         /*out*/ struct type_expression_name *__restrict result) {
	char const *doc = self->txp_iter;
	result->ten_str = NULL;
	if (*doc != '{') {
		result->ten_start = doc;
		while (DeeUni_IsSymCont(*doc))
			++doc;
		result->ten_end = doc;
		self->txp_iter  = doc;
		return 0;
	}
	++doc;
	result->ten_start = doc;
	doc = strchr(doc, '}');
	if unlikely(!doc)
		goto err_bad_doc_string;
	result->ten_end = doc;
	self->txp_iter  = doc + 1;

	/* Check if the string must be unescaped (i.e. contains any '\' characters) */
	if (memchr(result->ten_start, '\\',
	           (size_t)(result->ten_end - result->ten_start)) != NULL)
		return type_expression_name_unescape(result);
	return 0;
err_bad_doc_string:
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Malformed type annotation: Missing '}' after '{' in %q",
	                       self->txp_iter);
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
appears_in_import_tree(DeeModuleObject const *import_tree_of_this,
                       DeeModuleObject const *contains_this) {
	uint16_t mid;
	for (mid = 0; mid < import_tree_of_this->mo_importc; ++mid) {
		if (import_tree_of_this->mo_importv[mid] == contains_this)
			return true;
	}
	for (mid = 0; mid < import_tree_of_this->mo_importc; ++mid) {
		if (appears_in_import_tree(import_tree_of_this->mo_importv[mid],
		                           contains_this))
			return true;
	}
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeModule_CheckStaticAndDecref(struct typexpr_parser *__restrict self,
                               /*inherit(on_success)*/ DREF DeeModuleObject *mod) {
	DeeModuleObject *commod;
	if unlikely(!DeeObject_IsShared(mod))
		return false;
	if (mod == (DREF DeeModuleObject *)&DeeModule_Deemon)
		goto ok;
	if (mod == (DREF DeeModuleObject *)self->txp_info->di_mod)
		goto ok;
	commod = self->txp_gen->fg_assembler->fa_code->co_module;
	if (commod == mod)
		goto ok;
	if (appears_in_import_tree(commod, mod))
		goto ok;
	return false;
ok:
	Dee_DecrefNokill(mod);
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeType_CheckStaticAndDecref(struct typexpr_parser *__restrict self,
                             /*inherit(on_success)*/ DREF DeeTypeObject *type) {
	if unlikely(!DeeObject_IsShared(type))
		return false;
	if (!DeeType_IsCustom(type)) {
		DREF DeeModuleObject *type_module;
		if (type == self->txp_info->di_typ) {
ok:
			Dee_DecrefNokill(type);
			return true;
		}
		type_module = (DREF DeeModuleObject *)DeeType_GetModule(type);
		if likely(type_module) {
			if (DeeModule_CheckStaticAndDecref(self, type_module))
				goto ok;
		}
	}
	return false;
}

/* @return: * :   The encoded object
 * @return: NULL: The encoded object could not be determined (generic / multiple-choice)
 * @return: ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
typexpr_parser_parse_object_after_qmark(struct typexpr_parser *__restrict self) {
	DeeObject *result;
	struct type_expression_name name;
	switch (*self->txp_iter++) {

	case '.':
		return (DeeObject *)self->txp_info->di_typ;

	case 'N':
		return (DeeObject *)&DeeNone_Type;
	case 'O':
		return (DeeObject *)&DeeObject_Type;

	case 'T':
		/* TODO: Also remember information about tuple size!
		 * >> local x: string = ...;
		 * >> local a, b, c = x.partition(",")...; // No need to assert that returned tuple has size=3 here!
		 *
		 * Idea #1: Dee_memobj is allowed to predict the first data-word of an object.
		 * Idea #2: Just use equivalence classes to equate the returned tuple's t_size
		 *          with the constant from here!
		 */
		return (DeeObject *)&DeeTuple_Type;

	case '#':
	case 'D':
	case 'G':
	case 'E':
	case 'A': {
		char how = self->txp_iter[-1];
		if (typexpr_parser_parsename(self, &name))
			goto err;
		switch (how) {
		case '#':
			/* Use current context as base */
			result = (DeeObject *)self->txp_info->di_typ;
			if (result == NULL) {
		case 'G':
				result = (DeeObject *)self->txp_info->di_mod;
			if (result == NULL)
				goto unknown;
			}
			break;
		case 'D':
			result = (DeeObject *)&DeeModule_Deemon;
			break;

		case 'E': {
			DREF DeeObject *mod_export;
			if (name.ten_str) {
				result = (DeeObject *)DeeModule_OpenGlobal((DeeObject *)name.ten_str, NULL, false);
			} else {
				result = (DeeObject *)DeeModule_OpenGlobalString(name.ten_start,
				                                                 (size_t)(name.ten_end - name.ten_start),
				                                                 NULL, false);
			}
			type_expression_name_fini(&name);
			if (result == ITER_DONE)
				goto unknown;
			if (result == NULL)
				goto err;
			if (*self->txp_iter != ':') {
				Dee_Decref(result);
				goto unknown;
			}
			++self->txp_iter;
			if (typexpr_parser_parsename(self, &name)) {
				Dee_Decref(result);
				goto err;
			}
			if (name.ten_str) {
				mod_export = DeeObject_GetAttr(result, (DeeObject *)name.ten_str);
			} else {
				mod_export = DeeObject_GetAttrStringLen(result, name.ten_start,
				                                        (size_t)(name.ten_end - name.ten_start));
			}
			type_expression_name_fini(&name);
			Dee_Decref(result);
			result = mod_export;
			goto fini_name_and_check_result_after_getattr;
		}	break;

		case 'A':
			if (*self->txp_iter != '?')
				goto unknown;
			++self->txp_iter;
			result = typexpr_parser_parse_object_after_qmark(self);
			if (!ITER_ISOK(result)) {
				type_expression_name_fini(&name);
				return result;
			}
			break;
		default: __builtin_unreachable();
		}
		ASSERT(result);
		if (name.ten_str) {
			result = DeeObject_GetAttr(result, (DeeObject *)name.ten_str);
		} else {
			result = DeeObject_GetAttrStringLen(result, name.ten_start,
			                                    (size_t)(name.ten_end - name.ten_start));
		}
fini_name_and_check_result_after_getattr:
		type_expression_name_fini(&name);
		if unlikely(!result) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			goto unknown;
		}
		/* Validate that "result" can be used. */
		if (DeeType_Check(result)) {
			if (!DeeType_CheckStaticAndDecref(self, (DeeTypeObject *)result))
				goto unknown_decref_result;
		} else if (DeeModule_Check(result)) {
			if (!DeeModule_CheckStaticAndDecref(self, (DeeModuleObject *)result))
				goto unknown_decref_result;
		} else {
			Dee_Decref(result);
			goto unknown;
		}
	}	break;

	default:
		goto unknown;
	}
	return result;
unknown_decref_result:
	Dee_Decref(result);
unknown:
	return NULL; /* Unknown... */
err:
	return ITER_DONE;
}

/* @return: * :   The return type of the overload
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
typexpr_parser_extract_overload_return_type(struct typexpr_parser *__restrict self) {
	DeeTypeObject *result;
	if (self->txp_iter[0] == '(')
		self->txp_iter = seek_after_unescaped_char(self->txp_iter, ')');
	if (self->txp_iter[0] != '-' || self->txp_iter[1] != '>')
		return &DeeNone_Type; /* No return pointer -> function returns "none" */
	self->txp_iter += 2;
	if (self->txp_iter[0] != '?')
		return &DeeObject_Type; /* Nothing after return pointer -> function returns "Object" */
	self->txp_iter += 1;
	result = (DeeTypeObject *)typexpr_parser_parse_object_after_qmark(self);
	if (ITER_ISOK(result) && !DeeType_Check(result))
		result = NULL; /* Not actually a type */
	return result;
}

/* @return: * :   The return type of the overload
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeTypeObject *DCALL
overload_extract_return_type(char const *iter, struct docinfo const *__restrict info,
                             struct Dee_function_generator *__restrict generator) {
	struct typexpr_parser parser;
	parser.txp_iter = iter;
	parser.txp_info = info;
	parser.txp_gen  = generator;
	return typexpr_parser_extract_overload_return_type(&parser);
}

#define OVERLOAD_MATCHES_ERR   (-1) /* Error */
#define OVERLOAD_MATCHES_NO    0    /* No match */
#define OVERLOAD_MATCHES_MAYBE 1    /* Potential match (insufficient type information available) */
#define OVERLOAD_MATCHES_YES   2    /* Full match */

/* Check how/if the overload for "iter" matches the parameter list. */
PRIVATE WUNUSED NONNULL((1, 3, 4, 5)) int DCALL
overload_matches_arguments(struct Dee_function_generator *__restrict self,
                           Dee_vstackaddr_t argc, char const *iter,
                           struct docinfo const *__restrict info,
                           struct Dee_function_generator *__restrict generator) {
	struct Dee_memstate const *state = self->fg_state;
#define LOCAL_locfor_arg(argi) (&state->ms_stackv[state->ms_stackc - argc + (argi)])
#define LOCAL_typeof_arg(argi) Dee_memval_typeof(LOCAL_locfor_arg(argi))
	int result = OVERLOAD_MATCHES_YES;
	size_t argi = 0;
	if (iter[0] == '(') {
		struct typexpr_parser parser;
		parser.txp_iter = iter + 1;
		parser.txp_info = info;
		parser.txp_gen  = generator;
		while (!isnulorlf(*parser.txp_iter) && *parser.txp_iter != ')') {
			/* Seek until after the argument name */
			for (;;) {
				char ch = *parser.txp_iter++;
				if (isnulorlf(ch) || strchr("!?,=:)", ch)) {
					--parser.txp_iter;
					break;
				}
				if (ch == '{') {
					parser.txp_iter = seek_after_unescaped_char(parser.txp_iter, '}');
					continue;
				}
				if (ch == '\\' && *parser.txp_iter)
					++parser.txp_iter;
			}
			if (strchr("?!=", *parser.txp_iter)) {
				/* Optional or varargs from here on. */
				return result;
			} else if (*parser.txp_iter == ':') {
				/* Check if there is a type encoded here. */
				DeeTypeObject *want_argtype = &DeeObject_Type;
				DeeTypeObject *have_argtype;
				++parser.txp_iter;
				if (*parser.txp_iter == '?') {
					++parser.txp_iter;
					want_argtype = (DeeTypeObject *)typexpr_parser_parse_object_after_qmark(&parser);
					if unlikely(want_argtype == (DeeTypeObject *)ITER_DONE)
						goto err;
					if (want_argtype) {
						if (!DeeType_Check(want_argtype))
							want_argtype = NULL;
					} else {
						for (;;) {
							char ch = *parser.txp_iter++;
							if (isnulorlf(ch) || strchr(",=)", ch)) {
								--parser.txp_iter;
								break;
							}
							if (ch == '{') {
								parser.txp_iter = seek_after_unescaped_char(parser.txp_iter, '}');
								continue;
							}
							if (ch == '\\' && *parser.txp_iter)
								++parser.txp_iter;
						}
					}
				}
				if (*parser.txp_iter == '=')
					return result; /* Optional from here on. */
				if (argi >= argc)
					return OVERLOAD_MATCHES_NO; /* Too few arguments for call */
				if (want_argtype != &DeeObject_Type) {
					if (want_argtype == NULL) {
						/* Failed to determined wanted argument type (or expression too complex) */
						result = OVERLOAD_MATCHES_MAYBE;
					} else {
						have_argtype = LOCAL_typeof_arg(argi);
						if (have_argtype == NULL) {
							/* Unknown, but may be a potential overload... */
							result = OVERLOAD_MATCHES_MAYBE;
						} else if (!DeeType_Implements(have_argtype, want_argtype)) {
							/* Wrong argument type -> incorrect overload */
							return OVERLOAD_MATCHES_NO;
						}
					}
				}
			}

			/* Seek to the next argument in case the current one wasn't parsed fully */
			for (;;) {
				char ch = *parser.txp_iter++;
				if (isnulorlf(ch) || ch == ')') {
					--parser.txp_iter;
					break;
				}
				if (ch == '{') {
					parser.txp_iter = seek_after_unescaped_char(parser.txp_iter, '}');
					continue;
				}
				if (ch == ',')
					break;
				if (ch == '\\' && *parser.txp_iter)
					++parser.txp_iter;
			}
			++argi;
		}
	}
	if (argi != argc)
		return OVERLOAD_MATCHES_NO;
	return result;
err:
	return OVERLOAD_MATCHES_ERR;
#undef LOCAL_typeof_arg
#undef LOCAL_locfor_arg
}

/* @return: * :   The correct return type
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 3)) DeeTypeObject *DCALL
impl_extra_return_type_from_doc(struct Dee_function_generator *__restrict self,
                                Dee_vstackaddr_t argc,
                                struct docinfo const *__restrict info) {
	bool is_first;
	char const *iter, *next;
	DeeTypeObject *maybe_matched_overload_type = NULL;
	ASSERT(info->di_doc != NULL);
	iter = info->di_doc;
	is_first = true;
	while (*iter) {
		int how;
		next = strchrnul(iter, '\n');
		if (*next)
			++next;
		if (!(iter[0] == '(' || (iter[0] == '-' && iter[1] == '>')))
			break; /* End of prototype declaration list. */
		if (is_first && !(next[0] == '(' || (next[0] == '-' && next[1] == '>')))
			return overload_extract_return_type(iter, info, self); /* Only a singular overload exists. */
		how = overload_matches_arguments(self, argc, iter, info, self);
		if (how != OVERLOAD_MATCHES_NO) {
			if unlikely(how == OVERLOAD_MATCHES_ERR)
				goto err;
			if (how == OVERLOAD_MATCHES_YES)
				return overload_extract_return_type(iter, info, self);
			ASSERT(how == OVERLOAD_MATCHES_MAYBE);
			if (maybe_matched_overload_type != (DeeTypeObject *)ITER_DONE) {
				DeeTypeObject *overload_type = overload_extract_return_type(iter, info, self);
				if unlikely(overload_type == (DeeTypeObject *)ITER_DONE)
					goto err;
				if (overload_type == NULL)
					overload_type = (DeeTypeObject *)ITER_DONE;
				if (maybe_matched_overload_type == NULL) {
					maybe_matched_overload_type = overload_type; /* Initial option */
				} else if (maybe_matched_overload_type != overload_type) {
					maybe_matched_overload_type = (DeeTypeObject *)ITER_DONE; /* Multiple options... */
				}
			}
		}
		iter = next;
		is_first = false;
	}

	/* If we got exactly 1 potential overload, then that one has to be it. */
	if (maybe_matched_overload_type == (DeeTypeObject *)ITER_DONE)
		maybe_matched_overload_type = NULL;
	return maybe_matched_overload_type;
err:
	return (DeeTypeObject *)ITER_DONE;
}


/* [args...] -> [args...]
 * Try to extract the type of object returned by a C function as per `info'
 * NOTE: *only* do this for C functions (since type annotations from user-code
 *       may not be correct and thus cannot be trusted unconditionally)
 * NOTE: This function assumes that `info->di_doc != NULL'
 * @return: * :   The correct return type
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 3)) DeeTypeObject *DCALL
extra_return_type_from_doc(struct Dee_function_generator *__restrict self,
                           Dee_vstackaddr_t argc, struct docinfo *info) {
	DeeTypeObject *result;
	ASSERT(info->di_doc != NULL);
	if (info->di_typ != NULL && info->di_mod == NULL) {
		info->di_mod = (DeeModuleObject *)DeeType_GetModule(info->di_typ);
		result = impl_extra_return_type_from_doc(self, argc, info);
		Dee_XDecref(info->di_mod);
	} else {
		result = impl_extra_return_type_from_doc(self, argc, info);
	}
	if (result && result != (DeeTypeObject *)ITER_DONE &&
	    DeeType_IsAbstract(result) && result != &DeeNone_Type) {
		/* Ignore abstract return type information (like sequence proxies).
		 * We want the *exact* return type (not some base class). As such,
		 * simply disregard abstract types.
		 * XXX: Technically, we'd need to disregard anything that's not a
		 *      final type here, since it's technically OK to document a
		 *      non-abstract, non-final base class as return type a sub-
		 *      class of it. (does that happen anywhere?) */
		result = NULL;
	}
	return result;
}


/* Try to find the documentation string for `operator_name'
 * @return: * :   The doc string for `operator_name'
 * @return: NULL: No doc string available for `operator_name'
 * @return: (char const *)ITER_DONE: Error */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) char const *DCALL
get_operator_doc(struct Dee_function_generator const *__restrict self,
                 DeeTypeObject const *this_type, uint16_t operator_name) {
	if (this_type->tp_doc == NULL)
		goto unknown;
	/* TODO */
	(void)self;
	(void)this_type;
	(void)operator_name;
unknown:
	return NULL;
}



/* UNCHECKED(result) -> result */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
check_result_with_doc(struct Dee_function_generator *__restrict self,
                      Dee_vstackaddr_t argc, struct docinfo *doc) {
	ASSERT(doc);
	if (doc->di_doc != NULL && !(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
		DeeTypeObject *result_type;
		DO(Dee_function_generator_state_unshare(self));
		self->fg_state->ms_stackc += (argc - 1); /* Cheat a little here... */
		result_type = extra_return_type_from_doc(self, argc, doc);
		self->fg_state->ms_stackc -= (argc - 1); /* Cheat a little here... */
		if (result_type != NULL) {
			if unlikely(result_type == (DeeTypeObject *)ITER_DONE)
				goto err;
			DO(Dee_function_generator_vcheckobj(self));
			return Dee_function_generator_vsettyp_noalias(self, result_type);
		}
	}
	return Dee_function_generator_vcheckobj(self);
err:
	return -1;
}


/* [args...], kw  ->  [args...], kw   (return != 0)
 *                ->  [args...]       (return == 0)
 * Try to inline keyword arguments represented by a constant `DeeKwds_Type'
 * object by re-ordering positional "[args...]" such that provided keyword
 * arguments fall into their proper positions when passed without keyword:
 * >> local x = SOME_STRING_VALUE;
 * >> local y = x.lower(end: 4);
 * >> local z = x.contains(start: 0, end: 10, needle: "foo");
 *
 * By re-ordering arguments, it becomes possible to omit keywords:
 * >> local y = x.lower(0, 4);
 * >> local z = x.contains("foo", 0, 10);
 * Note that this doesn't change the evaluation order, since code to evaluate
 * arguments has already been generated at this point!
 *
 * @param: func:      When non-NULL, the function that is being invoked (may be used in place of "doc")
 * @param: func_type: The effective type of "func" (may differ from Dee_TYPE() when super objects are involved)
 * @return: 0 : Keyword arguments were successfully inlined
 * @return: 1 : Keyword arguments could not be inlined
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vinline_kwds(struct Dee_function_generator *__restrict self,
             Dee_vstackaddr_t *p_argc, struct docinfo *doc,
             DeeObject *func, DeeTypeObject *func_type) {
	(void)self;
	(void)p_argc;
	(void)doc;
	(void)func;
	(void)func_type;
	/* TODO */
	return 1;
}

/* Same as `vinline_kwds()', but when keyword arguments could be inlined,
 * then replace them with `NULL' on the v-stack.
 * @param: func: When non-NULL, the function that is being invoked (may be used in place of "doc")
 * @return: 0 : Success (keyword arguments may or may not have been inlined)
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vinline_kwds_and_replace_with_null(struct Dee_function_generator *__restrict self,
                                   Dee_vstackaddr_t *p_argc, struct docinfo *doc,
                                   DeeObject *func, DeeTypeObject *func_type) {
	int result = vinline_kwds(self, p_argc, doc, func, func_type);
	if (result == 0) {
		result = Dee_function_generator_vpush_NULL(self);
	} else if (result > 0) {
		result = 0; /* Inlining was not possible. */
	}
	return result;
}

/* this, [args...] -> result */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_objmethod(struct Dee_function_generator *__restrict self,
                Dee_objmethod_t func, Dee_vstackaddr_t argc,
                struct docinfo *doc, char const *funcname,
                uintptr_t func_flags) {
	DeeTypeObject *type = doc->di_typ;
	if (type) {
		if (Dee_function_generator_vallconst(self, argc + 1)) {
			/* Inline the object method call. */
			size_t i;
			DeeObject *thisobj, **argv;
			DREF DeeObject *retval;
			struct Dee_memval *thisval;
			argv = (DeeObject **)Dee_Mallocac(argc, sizeof(DeeObject *));
			if unlikely(!argv)
				goto err;
			thisval = Dee_function_generator_vtop(self) - argc;
			thisobj = Dee_memval_const_getobj(thisval);
			for (i = 0; i < argc; ++i)
				argv[i] = Dee_memval_const_getobj(&thisval[i + 1]);
			if (!DeeType_IsObjMethodConstexpr(type, func, thisobj, argc, argv, NULL)) {
				Dee_Freea(argv);
			} else {
				retval = (*func)(thisobj, argc, argv);
				Dee_Freea(argv);
				if likely(retval != NULL) {
					retval = Dee_function_generator_inlineref(self, retval);
					if unlikely(!retval)
						goto err;
					DO(Dee_function_generator_vpopmany(self, argc + 1));
					return Dee_function_generator_vpush_const(self, retval);
				} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
					goto err;
				} else {
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				}
			}
		}
		if (funcname == NULL) {
			struct type_method const *iter = type->tp_methods;
			if likely(iter) {
				while (iter->m_name && iter->m_func != func)
					++iter;
				if (iter->m_name) {
					funcname   = iter->m_name;
					func_flags = iter->m_flag;
				}
			}
		}

		/* Check if we have a known optimization for the function being called. */
		if (funcname != NULL && argc <= Dee_CCALL_ARGC_MAX) {
			struct Dee_ccall_optimization const *cco;
			cco = Dee_ccall_find_attr_optimization(type, funcname, argc);
			if (cco != NULL) {
				int status;
				status = (*cco->tcco_func)(self, argc);
				if (status <= 0)
					return status; /* Success, or error */
			}
		}
	}
	DO(Dee_function_generator_vnotoneref(self, argc));            /* this, [args...] */
	if (!(func_flags & Dee_TYPE_METHOD_FNOREFESCAPE))             /* this, [args...] */
		DO(Dee_function_generator_vnotoneref_at(self, argc + 1)); /* this, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));         /* this, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 2));             /* [args...], argv, this */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));          /* [args...], argv, this, argc */
	DO(Dee_function_generator_vlrot(self, 3));                    /* [args...], this, argc, argv */
	DO(Dee_function_generator_vcallapi_ex(self, func, VCALL_CC_RAWINTPTR, 3, argc + 3)); /* UNCHECKED(result) */
	return check_result_with_doc(self, argc, doc);                /* result */
err:
	return -1;
}

/* this, [args...], kw -> result */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_kwobjmethod(struct Dee_function_generator *__restrict self,
                  Dee_kwobjmethod_t func, Dee_vstackaddr_t argc,
                  struct docinfo *doc, char const *funcname,
                  uintptr_t func_flags) {
	DeeTypeObject *type;
	DO(vinline_kwds_and_replace_with_null(self, &argc, doc, NULL, NULL)); /* this, [args...], kw */
	type = doc->di_typ;
	if (type) {
		if (Dee_function_generator_vallconst(self, argc + 2)) {
			/* Inline the object method call. */
			size_t i;
			DeeObject *thisobj, *kw, **argv;
			DREF DeeObject *retval;
			struct Dee_memval *thisval;
			argv = (DeeObject **)Dee_Mallocac(argc, sizeof(DeeObject *));
			if unlikely(!argv)
				goto err;
			thisval = Dee_function_generator_vtop(self) - (argc + 1);
			thisobj = Dee_memval_const_getobj(thisval);
			for (i = 0; i < argc; ++i)
				argv[i] = Dee_memval_const_getobj(&thisval[i + 1]);
			kw = Dee_memval_const_getobj(&thisval[argc + 1]);
			if (!DeeType_IsObjMethodConstexpr(type, (Dee_objmethod_t)(void const *)func,
			                                  thisobj, argc, argv, kw)) {
				Dee_Freea(argv);
			} else {
				retval = (*func)(thisobj, argc, argv, kw);
				Dee_Freea(argv);
				if likely(retval != NULL) {
					retval = Dee_function_generator_inlineref(self, retval);
					if unlikely(!retval)
						goto err;
					DO(Dee_function_generator_vpopmany(self, argc + 2));
					return Dee_function_generator_vpush_const(self, retval);
				} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
					goto err;
				} else {
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				}
			}
		}
		if (funcname == NULL) {
			struct type_method const *iter = type->tp_methods;
			if likely(iter) {
				while (iter->m_name && iter->m_func != (Dee_objmethod_t)(void const *)func)
					++iter;
				if (iter->m_name) {
					funcname   = iter->m_name;
					func_flags = iter->m_flag;
				}
			}
		}

		/* Check if we have a known optimization for the function being called. */
		if (funcname != NULL) {
			struct Dee_memval *kwval = Dee_function_generator_vtop(self);
			if (Dee_memval_isnull(kwval) && argc <= Dee_CCALL_ARGC_MAX) {
				struct Dee_ccall_optimization const *cco;
				cco = Dee_ccall_find_attr_optimization(type, funcname, argc);
				if (cco != NULL) {
					int status;
					DO(Dee_function_generator_vpop(self)); /* this, [args...] */
					status = (*cco->tcco_func)(self, argc);
					if (status <= 0)
						return status; /* Success, or error */
					DO(Dee_function_generator_vpush_NULL(self)); /* this, [args...], kw=NULL */
				}
			}
		}
	}
	DO(Dee_function_generator_vnotoneref(self, argc + 1));        /* this, [args...], kw */
	if (!(func_flags & Dee_TYPE_METHOD_FNOREFESCAPE))             /* this, [args...], kw */
		DO(Dee_function_generator_vnotoneref_at(self, argc + 2)); /* this, [args...], kw */
	DO(Dee_function_generator_vrrot(self, argc + 1));             /* this, kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));         /* this, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 3));             /* kw, [args...], argv, this */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));          /* kw, [args...], argv, this, argc */
	DO(Dee_function_generator_vlrot(self, 3));                    /* kw, [args...], this, argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 4));             /* [args...], this, argc, argv, kw */
	DO(Dee_function_generator_vcallapi_ex(self, func, VCALL_CC_RAWINTPTR, 4, argc + 4)); /* UNCHECKED(result) */
	return check_result_with_doc(self, argc, doc);                /* result */
err:
	return -1;
}

/* this -> result */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vcall_getmethod(struct Dee_function_generator *__restrict self,
                Dee_getmethod_t func, struct docinfo *doc,
                char const *getset_name, uintptr_t getset_flags) {
	DeeTypeObject *type;

	/* Optimizations for some well-known getter-like functions. */
	if (func == &DeeObject_NewRef) {
		/* This right here optimizes stuff like `Object.this' */
		return 0;
	} else if (func == &DeeObject_SizeObject) {
		return Dee_function_generator_vopsize(self);
	} else if (func == &DeeObject_IterSelf) {
		return Dee_function_generator_vop(self, OPERATOR_ITERSELF, 1, VOP_F_PUSHRES);
	} else if (func == &DeeTuple_FromSequence) {
		return Dee_function_generator_vopcast(self, &DeeTuple_Type);
	} else if (func == &DeeList_FromSequence) {
		return Dee_function_generator_vopcast(self, &DeeList_Type);
	} else if (func == &DeeDict_FromSequence) {
		return Dee_function_generator_vopcast(self, &DeeDict_Type);
	} else if (func == &DeeHashSet_FromSequence) {
		return Dee_function_generator_vopcast(self, &DeeHashSet_Type);
	} else if (func == &DeeRoDict_FromSequence) {
		return Dee_function_generator_vopcast(self, &DeeRoDict_Type);
	} else if (func == &DeeRoSet_FromSequence) {
		return Dee_function_generator_vopcast(self, &DeeRoSet_Type);
	}

	/* Do certain optimizations when the type is known. */
	type = doc->di_typ;
	if (type) {
		if (Dee_function_generator_vallconst(self, 1) && DeeType_IsGetMethodConstexpr(type, func)) {
			/* Inline the getter call. */
			DeeObject *thisval = Dee_memval_const_getobj(Dee_function_generator_vtop(self));
			DREF DeeObject *retval = (*func)(thisval);
			if likely(retval != NULL) {
				retval = Dee_function_generator_inlineref(self, retval);
				if unlikely(!retval)
					goto err;
				DO(Dee_function_generator_vpop(self));
				return Dee_function_generator_vpush_const(self, retval);
			} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
				goto err;
			} else {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
		}
		if (getset_name == NULL) {
			struct type_getset const *iter = type->tp_getsets;
			if likely(iter) {
				while (iter->gs_name && iter->gs_get != func)
					++iter;
				if (iter->gs_name) {
					getset_name  = iter->gs_name;
					getset_flags = iter->gs_flags;
				}
			}
		}

		/* Check if we have a known optimization for the getter being called. */
		if (getset_name != NULL) {
			struct Dee_ccall_optimization const *cco;
			cco = Dee_ccall_find_attr_optimization(type, getset_name, Dee_CCALL_ARGC_GETTER);
			if (cco != NULL && cco->tcco_argc == Dee_CCALL_ARGC_GETTER) {
				int status = (*cco->tcco_func)(self, Dee_CCALL_ARGC_GETTER);
				if (status <= 0)
					return status; /* Success, or error */
			}
		}
	}
	if (!(getset_flags & TYPE_GETSET_FNOREFESCAPE))
		DO(Dee_function_generator_vnotoneref_at(self, 1));                  /* this */
	DO(Dee_function_generator_vcallapi(self, func, VCALL_CC_RAWINTPTR, 1)); /* result */
	return check_result_with_doc(self, 0, doc);
err:
	return -1;
}

/* this -> result */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_boundmethod(struct Dee_function_generator *__restrict self,
                  Dee_boundmethod_t func, DeeTypeObject *type,
                  char const *getset_name, uintptr_t getset_flags) {
	if (type) {
		if (Dee_function_generator_vallconst(self, 1) && DeeType_IsBoundMethodConstexpr(type, func)) {
			/* Inline the is-bound call. */
			DeeObject *thisval = Dee_memval_const_getobj(Dee_function_generator_vtop(self));
			int retval = (*func)(thisval);
			if likely(retval != -1) {
				DO(Dee_function_generator_vpop(self));
				return Dee_function_generator_vpush_const(self, DeeBool_For(retval > 0));
			} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
				goto err;
			} else {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
		}
		if (getset_name == NULL) {
			struct type_getset const *iter = type->tp_getsets;
			if likely(iter) {
				while (iter->gs_name && iter->gs_bound != func)
					++iter;
				if (iter->gs_name) {
					getset_name  = iter->gs_name;
					getset_flags = iter->gs_flags;
				}
			}
		}

		/* Check if we have a known optimization for the bound-function being called. */
		if (getset_name != NULL) {
			struct Dee_ccall_optimization const *cco;
			cco = Dee_ccall_find_attr_optimization(type, getset_name, Dee_CCALL_ARGC_BOUND);
			if (cco != NULL && cco->tcco_argc == Dee_CCALL_ARGC_BOUND) {
				int status = (*cco->tcco_func)(self, Dee_CCALL_ARGC_BOUND);
				if (status <= 0)
					return status; /* Success, or error */
			}
		}
	}
	if (!(getset_flags & TYPE_GETSET_FNOREFESCAPE))
		DO(Dee_function_generator_vnotoneref_at(self, 1));              /* this */
	DO(Dee_function_generator_vcallapi(self, func, VCALL_CC_M1INT, 1)); /* result */
	DO(Dee_function_generator_vdirect1(self));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* this -> N/A */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_delmethod(struct Dee_function_generator *__restrict self,
                Dee_delmethod_t func, DeeTypeObject *type,
                char const *getset_name, uintptr_t getset_flags) {
	if (type != NULL) {
		if (getset_name == NULL) {
			struct type_getset const *iter = type->tp_getsets;
			if likely(iter) {
				while (iter->gs_name && iter->gs_del != func)
					++iter;
				if (iter->gs_name) {
					getset_name  = iter->gs_name;
					getset_flags = iter->gs_flags;
				}
			}
		}

		/* Check if we have a known optimization for the bound-function being called. */
		if (getset_name != NULL) {
			struct Dee_ccall_optimization const *cco;
			cco = Dee_ccall_find_attr_optimization(type, getset_name, Dee_CCALL_ARGC_DELETE);
			if (cco != NULL && cco->tcco_argc == Dee_CCALL_ARGC_DELETE) {
				int status = (*cco->tcco_func)(self, Dee_CCALL_ARGC_DELETE);
				if (status <= 0)
					return status; /* Success, or error */
			}
		}
	}
	if (!(getset_flags & TYPE_GETSET_FNOREFESCAPE))
		DO(Dee_function_generator_vnotoneref_at(self, 1)); /* this */
	return Dee_function_generator_vcallapi(self, func, VCALL_CC_INT, 1); /* N/A */
err:
	return -1;
}

/* this, value -> N/A */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_setmethod(struct Dee_function_generator *__restrict self,
                Dee_setmethod_t func, DeeTypeObject *type,
                char const *getset_name, uintptr_t getset_flags) {
	if (type != NULL) {
		if (getset_name == NULL) {
			struct type_getset const *iter = type->tp_getsets;
			if likely(iter) {
				while (iter->gs_name && iter->gs_set != func)
					++iter;
				if (iter->gs_name) {
					getset_name  = iter->gs_name;
					getset_flags = iter->gs_flags;
				}
			}
		}

		/* Check if we have a known optimization for the bound-function being called. */
		if (getset_name != NULL) {
			struct Dee_ccall_optimization const *cco;
			cco = Dee_ccall_find_attr_optimization(type, getset_name, Dee_CCALL_ARGC_SETTER);
			if (cco != NULL && cco->tcco_argc == Dee_CCALL_ARGC_SETTER) {
				int status = (*cco->tcco_func)(self, Dee_CCALL_ARGC_SETTER);
				if (status <= 0)
					return status; /* Success, or error */
			}
		}
	}
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* this, value */
	if (!(getset_flags & TYPE_GETSET_FNOREFESCAPE))
		DO(Dee_function_generator_vnotoneref_at(self, 2)); /* this, value */
	return Dee_function_generator_vcallapi(self, func, VCALL_CC_INT, 2); /* N/A */
err:
	return -1;
}


/* [args...] -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vophash_n(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t argc) {
	if (argc == 0) {
		STATIC_ASSERT(DEE_HASHOF_EMPTY_SEQUENCE == 0);
		return Dee_function_generator_vpush_const(self, DeeInt_Zero);
	}
	if (argc == 1)
		return Dee_function_generator_vop(self, OPERATOR_HASH, 1, VOP_F_PUSHRES);
	DO(Dee_function_generator_vlinear(self, argc, true)); /* [objs...], argv */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* [objs...], argv, argc */
	DO(Dee_function_generator_vswap(self));               /* [objs...], argc, argv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_Hashv, VCALL_CC_MORPH_UINTPTR, 2)); /* [objs...], result */
	DO(Dee_function_generator_vrrot(self, argc + 1));     /* result, [objs...] */
	return Dee_function_generator_vpopmany(self, argc);   /* result */
err:
	return -1;
}

/* [args...] -> result */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_cmethod(struct Dee_function_generator *__restrict self,
              Dee_cmethod_t func, Dee_vstackaddr_t argc,
              struct docinfo *doc) {
	if (Dee_function_generator_vallconst(self, argc)) {
		/* Inline the c-method call. */
		size_t i;
		DeeObject **argv;
		DREF DeeObject *retval;
		struct Dee_memval *argvalv;
		argv = (DeeObject **)Dee_Mallocac(argc, sizeof(DeeObject *));
		if unlikely(!argv)
			goto err;
		argvalv = Dee_function_generator_vtop(self) - (argc - 1);
		for (i = 0; i < argc; ++i)
			argv[i] = Dee_memval_const_getobj(&argvalv[i]);
		if (!DeeCMethod_IsConstExpr(func, argc, argv, NULL)) {
			Dee_Freea(argv);
		} else {
			retval = (*func)(argc, argv);
			Dee_Freea(argv);
			if likely(retval != NULL) {
				retval = Dee_function_generator_inlineref(self, retval);
				if unlikely(!retval)
					goto err;
				DO(Dee_function_generator_vpopmany(self, argc));
				return Dee_function_generator_vpush_const(self, retval);
			} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
				goto err;
			} else {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
		}
	}

	/* Optimizations for special C methods from the builtin deemon module. */
	if (func == DeeBuiltin_HasAttr.cm_func) {
		if (argc == 2)
			return Dee_function_generator_vophasattr(self);
	} else if (func == DeeBuiltin_HasItem.cm_func) {
		/*if (argc == 2) // TODO
			return Dee_function_generator_vophasitem(self);*/
	} else if (func == DeeBuiltin_BoundAttr.cm_func) {
		if (argc == 2)
			return Dee_function_generator_vopboundattr(self);
		if (argc == 3) {
			/* TODO: Inline call when 3rd "allow_missing" argument is given. */
		}
	} else if (func == DeeBuiltin_BoundItem.cm_func) {
		if (argc == 2)
			return Dee_function_generator_vopbounditem(self);
		if (argc == 3) {
			/* TODO: Inline call when 3rd "allow_missing" argument is given. */
		}
	} else if (func == DeeBuiltin_Hash.cm_func) {
		return Dee_function_generator_vophash_n(self, argc);
	}

	DO(Dee_function_generator_vnotoneref(self, argc));    /* [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true)); /* [args...], argv */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* [args...], argv, argc */
	DO(Dee_function_generator_vswap(self));               /* [args...], argc, argv */
	DO(Dee_function_generator_vcallapi_ex(self, func, VCALL_CC_RAWINTPTR, 2, argc + 2)); /* UNCHECKED(result) */
	return check_result_with_doc(self, argc, doc);        /* result */
err:
	return -1;
}

/* [args...], kw -> result */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_kwcmethod(struct Dee_function_generator *__restrict self,
                Dee_kwcmethod_t func, Dee_vstackaddr_t argc,
                struct docinfo *doc) {
	if (Dee_function_generator_vallconst(self, argc + 1)) {
		/* Inline the c-method call. */
		size_t i;
		DeeObject *kw, **argv;
		DREF DeeObject *retval;
		struct Dee_memval *argvalv;
		argv = (DeeObject **)Dee_Mallocac(argc, sizeof(DeeObject *));
		if unlikely(!argv)
			goto err;
		argvalv = Dee_function_generator_vtop(self) - argc;
		for (i = 0; i < argc; ++i)
			argv[i] = Dee_memval_const_getobj(&argvalv[i]);
		kw = Dee_memval_const_getobj(&argvalv[argc]);
		if (!DeeCMethod_IsConstExpr((Dee_cmethod_t)(void const *)func, argc, argv, kw)) {
			Dee_Freea(argv);
		} else {
			retval = (*func)(argc, argv, kw);
			Dee_Freea(argv);
			if likely(retval != NULL) {
				retval = Dee_function_generator_inlineref(self, retval);
				if unlikely(!retval)
					goto err;
				DO(Dee_function_generator_vpopmany(self, argc + 1));
				return Dee_function_generator_vpush_const(self, retval);
			} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
				goto err;
			} else {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
		}
	}

	DO(vinline_kwds_and_replace_with_null(self, &argc, doc, NULL, NULL)); /* [args...], kw */

	/* Optimizations for special C methods from the builtin deemon module. */
	if (func == DeeBuiltin_Compare.cm_func) {
		/*if (argc == 2 && Dee_memval_isnull(Dee_function_generator_vtop(self))) // XXX: Inline?
			return Dee_function_generator_vopcompare(self);*/
	} else if (func == DeeBuiltin_Import.cm_func) {
		/*if (argc == 1 && Dee_memval_isnull(Dee_function_generator_vtop(self))) // XXX: Inline?
			return Dee_function_generator_vopimport(self);*/
	}

	DO(Dee_function_generator_vnotoneref(self, argc + 1)); /* [args...], kw */
	DO(Dee_function_generator_vrrot(self, argc + 1));      /* kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));  /* kw, [args...], argv */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));   /* kw, [args...], argv, argc */
	DO(Dee_function_generator_vswap(self));                /* kw, [args...], argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 3));      /* [args...], argc, argv, kw */
	DO(Dee_function_generator_vcallapi_ex(self, func, VCALL_CC_RAWINTPTR, 3, argc + 3)); /* UNCHECKED(result) */
	return check_result_with_doc(self, argc, doc);         /* result */
err:
	return -1;
}


/* instance  ->  instance, UNCHECKED(status_int)   (return == 0)
 * instance  ->  instance                          (return == 1)
 * @return: 1 : Success (no "status" was pushed because none was needed)
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vcall_Type_tp_ctor_unchecked(struct Dee_function_generator *__restrict self, DeeTypeObject *type,
                             int (DCALL *tp_ctor)(DeeObject *__restrict self)) {
	ASSERT(tp_ctor);
	ASSERT(type->tp_init.tp_alloc.tp_ctor == tp_ctor);
	(void)type;

	/* Inline select constructor implementations (e.g. that of DeeList_Type)
	 * NOTE: We *always* compare against "tp_ctor", so we also hit
	 *       sub-classes that inherited these special constructors. */
	if (tp_ctor == DeeObject_Type.tp_init.tp_alloc.tp_ctor) {
		return 1; /* This one is known as "none_i1", and a bunch of types use it to implement no-op constructors. */
	} else if (tp_ctor == DeeCell_Type.tp_init.tp_alloc.tp_ctor) {
		DO(Dee_function_generator_vpush_NULL(self));                               /* instance, NULL */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeCellObject, c_item))); /* instance */
#ifndef CONFIG_NO_THREADS
		DO(Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self));                 /* instance, ATOMIC_RWLOCK_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeCellObject, c_lock))); /* instance */
#endif /* !CONFIG_NO_THREADS */
		return 1;
	} else if (tp_ctor == DeeDict_Type.tp_init.tp_alloc.tp_ctor) {
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                          /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, d_mask))); /* instance */
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                          /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, d_size))); /* instance */
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                          /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, d_used))); /* instance */
		DO(Dee_function_generator_vpush_addr(self, DeeDict_EmptyItems));           /* instance, DeeDict_EmptyItems */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, d_elem))); /* instance */
#ifndef CONFIG_NO_THREADS
		DO(Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self));                 /* instance, ATOMIC_RWLOCK_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, d_lock))); /* instance */
#endif /* !CONFIG_NO_THREADS */
		DO(Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self));                    /* instance, WEAKREF_SUPPORT_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, ob_weakrefs))); /* instance */
		return 1;
	} else if (tp_ctor == DeeHashSet_Type.tp_init.tp_alloc.tp_ctor) {
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                              /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, hs_mask))); /* instance */
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                              /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, hs_size))); /* instance */
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                              /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, hs_used))); /* instance */
		DO(Dee_function_generator_vpush_addr(self, DeeHashSet_EmptyItems));            /* instance, DeeHashSet_EmptyItems */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, hs_elem))); /* instance */
#ifndef CONFIG_NO_THREADS
		DO(Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self));                     /* instance, ATOMIC_RWLOCK_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, hs_lock))); /* instance */
#endif /* !CONFIG_NO_THREADS */
		DO(Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self));                       /* instance, WEAKREF_SUPPORT_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, ob_weakrefs))); /* instance */
		return 1;
	} else if (tp_ctor == DeeList_Type.tp_init.tp_alloc.tp_ctor) {
		DO(Dee_function_generator_vpush_NULL(self));                                        /* instance, NULL */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeListObject, l_list.ol_elemv))); /* instance */
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                                   /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeListObject, l_list.ol_elemc))); /* instance */
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
		DO(Dee_function_generator_vpush_immSIZ(self, 0));                                   /* instance, 0 */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeListObject, l_list.ol_elema))); /* instance */
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */
#ifndef CONFIG_NO_THREADS
		DO(Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self));                          /* instance, ATOMIC_RWLOCK_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeListObject, l_lock)));          /* instance */
#endif /* !CONFIG_NO_THREADS */
		DO(Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self));                        /* instance, WEAKREF_SUPPORT_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeListObject, ob_weakrefs)));     /* instance */
		return 1;
	} else if (tp_ctor == DeeWeakRefAble_Type.tp_init.tp_alloc.tp_ctor) {
		DO(Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self));                           /* instance, WEAKREF_SUPPORT_INIT */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeWeakRefAbleObject, ob_weakrefs))); /* instance */
		return 1;
	}

	/* Fallback: emit a direct call to the constructor's C-implementation. */
	if (!DeeType_IsOperatorNoRefEscape(type, OPERATOR_CONSTRUCTOR))
		DO(Dee_function_generator_vnotoneref_at(self, 1)); /* instance */
	return Dee_function_generator_vcallapi_ex(self, tp_ctor, VCALL_CC_RAWINTPTR, 1, 0);
err:
	return -1;
}

/* instance, other  ->  instance, UNCHECKED(status_int) (return == 0)
 * instance, other  ->  instance                        (return == 1)
 * @return: 1 : Success (no "status" was pushed because none was needed)
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vcall_Type_tp_copy_ctor_unchecked(struct Dee_function_generator *__restrict self, DeeTypeObject *type,
                                  int (DCALL *tp_copy_ctor)(DeeObject *__restrict self, DeeObject *__restrict other)) {
	ASSERT(tp_copy_ctor);
	ASSERT(type->tp_init.tp_alloc.tp_copy_ctor == tp_copy_ctor);
	(void)type; /* XXX: Inline select constructor implementations? */
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* instance, other */
	if (!DeeType_IsOperatorNoRefEscape(type, OPERATOR_CONSTRUCTOR))
		DO(Dee_function_generator_vnotoneref_at(self, 2));                                /* instance, other */
	DO(Dee_function_generator_vcallapi_ex(self, tp_copy_ctor, VCALL_CC_RAWINTPTR, 2, 0)); /* instance, other, UNCHECKED(status_int) */
	return Dee_function_generator_vpop_at(self, 2);                                       /* instance, UNCHECKED(status_int) */
err:
	return -1;
}

/* instance, [args...]  ->  instance, UNCHECKED(status_int)   (return == 0)
 * instance, [args...]  ->  instance                          (return == 1)
 * @return: 1 : Success (no "status" was pushed because none was needed)
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vcall_Type_tp_any_ctor_unchecked(struct Dee_function_generator *__restrict self, DeeTypeObject *type, Dee_vstackaddr_t argc,
                                 int (DCALL *tp_any_ctor)(DeeObject *__restrict self, size_t argc, DeeObject *const *argv)) {
	ASSERT(tp_any_ctor);
	ASSERT(type->tp_init.tp_alloc.tp_any_ctor == tp_any_ctor);
	(void)type; /* XXX: Inline select constructor implementations? */
	DO(Dee_function_generator_vnotoneref(self, argc));/* instance, [args...] */
	if (!DeeType_IsOperatorNoRefEscape(type, OPERATOR_CONSTRUCTOR))
		DO(Dee_function_generator_vnotoneref_at(self, argc + 1)); /* instance, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true)); /* instance, [args...], argv */
	DO(Dee_function_generator_vdup_n(self, argc + 2));    /* instance, [args...], argv, instance */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* instance, [args...], argv, instance, argc */
	DO(Dee_function_generator_vlrot(self, 3));            /* instance, [args...], instance, argc, argv */
	DO(Dee_function_generator_vcallapi(self, tp_any_ctor, VCALL_CC_RAWINTPTR, 3)); /* instance, [args...], UNCHECKED(status_int) */
	DO(Dee_function_generator_vrrot(self, argc + 1));     /* instance, UNCHECKED(status_int), [args...] */
	return Dee_function_generator_vpopmany(self, argc);   /* instance, UNCHECKED(status_int) */
err:
	return -1;
}

/* instance, [args...], kw  ->  instance, UNCHECKED(status_int)   (return == 0)
 * instance, [args...], kw  ->  instance                          (return == 1)
 * @return: 1 : Success (no "status" was pushed because none was needed)
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vcall_Type_tp_any_ctor_kw_unchecked(struct Dee_function_generator *__restrict self, DeeTypeObject *type, Dee_vstackaddr_t argc,
                                    int (DCALL *tp_any_ctor_kw)(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw)) {
	ASSERT(tp_any_ctor_kw);
	ASSERT(type->tp_init.tp_alloc.tp_any_ctor_kw == tp_any_ctor_kw);
	(void)type; /* XXX: Inline select constructor implementations? */
	DO(Dee_function_generator_vnotoneref(self, argc + 1));/* instance, [args...], kw */
	if (!DeeType_IsOperatorNoRefEscape(type, OPERATOR_CONSTRUCTOR))
		DO(Dee_function_generator_vnotoneref_at(self, argc + 2)); /* instance, [args...], kw */
	DO(Dee_function_generator_vrrot(self, argc + 1));     /* instance, kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true)); /* instance, kw, [args...], argv */
	DO(Dee_function_generator_vdup_n(self, argc + 3));    /* instance, kw, [args...], argv, instance */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* instance, kw, [args...], argv, instance, argc */
	DO(Dee_function_generator_vlrot(self, 3));            /* instance, kw, [args...], instance, argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 4));     /* instance, [args...], instance, argc, argv, kw */
	DO(Dee_function_generator_vcallapi(self, tp_any_ctor_kw, VCALL_CC_RAWINTPTR, 4)); /* instance, [args...], UNCHECKED(status_int) */
	DO(Dee_function_generator_vrrot(self, argc + 1));     /* instance, UNCHECKED(status_int), [args...] */
	return Dee_function_generator_vpopmany(self, argc);   /* instance, UNCHECKED(status_int) */
err:
	return -1;
}



/* N/A  ->  instance
 * Emit code to do `DeeType_AllocInstance(type)'.
 * The generated code already includes a NULL-check
 * The pushed "instance" has the "MEMOBJ_F_NOREF" CLEAR and "MEMOBJ_F_ONEREF" SET. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_DeeType_AllocInstance(struct Dee_function_generator *__restrict self,
                            DeeTypeObject *type) {
	if (type->tp_init.tp_alloc.tp_free) {
		DO(Dee_function_generator_vcallapi(self, type->tp_init.tp_alloc.tp_alloc, VCALL_CC_OBJECT, 0));
	} else {
		size_t instance_size;
		instance_size = type->tp_init.tp_alloc.tp_instance_size;
		DO(Dee_function_generator_vpush_immSIZ(self, instance_size));
		DO(Dee_function_generator_vcallapi(self,
		                                   (type->tp_flags & TP_FGC) ? (void const *)&DeeGCObject_Malloc
		                                                             : (void const *)&DeeObject_Malloc,
		                                   VCALL_CC_OBJECT, 1));
	}
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_voneref_noalias(self));
	return 0;
err:
	return -1;
}

/* instance  ->  N/A
 * Emit code to do `DeeType_FreeInstance(type, instance)'. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_DeeType_FreeInstance(struct Dee_function_generator *__restrict self,
                           DeeTypeObject *type) {
	void const *api_function;
	Dee_function_generator_vtop_direct_clearref(self);
	api_function = (void const *)type->tp_init.tp_alloc.tp_free;
	if (!api_function) {
		api_function = (type->tp_flags & TP_FGC) ? (void const *)&DeeGCObject_Free
		                                         : (void const *)&DeeObject_Free;
	}
	return Dee_function_generator_vcallapi(self, api_function, VCALL_CC_VOID_NX, 1);
}

struct Dee_function_exceptinject_freeinstance {
	struct Dee_function_exceptinject fei_fi_base; /* Underlying injector */
	DeeTypeObject                   *fei_fi_type; /* [1..1] The type of whom an instance should be free'd */
};

#define Dee_function_generator_xinject_push_freeinstance(self, ij, type)       \
	((ij)->fei_fi_base.fei_inject = &Dee_function_exceptinject_freeinstance_f, \
	 (ij)->fei_fi_type            = (type),                                    \
	 Dee_function_generator_xinject_push(self, &(ij)->fei_fi_base))
#define Dee_function_generator_xinject_pop_freeinstance(self, ij) \
	Dee_function_generator_xinject_pop(self, &(ij)->fei_fi_base)

INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* `fei_inject' value for `struct Dee_function_exceptinject_freeinstance' */
Dee_function_exceptinject_freeinstance_f(struct Dee_function_generator *__restrict self,
                                         struct Dee_function_exceptinject *__restrict inject) {
	struct Dee_function_exceptinject_freeinstance *me;
	me = (struct Dee_function_exceptinject_freeinstance *)inject;
	return vcall_DeeType_FreeInstance(self, me->fei_fi_type);
}

struct Dee_function_exceptinject_fini_and_freeinstance {
	struct Dee_function_exceptinject fei_fafi_base; /* Underlying injector */
	DeeTypeObject                   *fei_fafi_type; /* [1..1] The type of whom an instance should be free'd */
};

#define Dee_function_generator_xinject_push_fini_and_freeinstance(self, ij, type)         \
	((ij)->fei_fafi_base.fei_inject = &Dee_function_exceptinject_fini_and_freeinstance_f, \
	 (ij)->fei_fafi_type            = (type),                                             \
	 Dee_function_generator_xinject_push(self, &(ij)->fei_fafi_base))
#define Dee_function_generator_xinject_pop_fini_and_freeinstance(self, ij) \
	Dee_function_generator_xinject_pop(self, &(ij)->fei_fafi_base)

INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* `fei_inject' value for `struct Dee_function_exceptinject_fini_and_freeinstance' */
Dee_function_exceptinject_fini_and_freeinstance_f(struct Dee_function_generator *__restrict self,
                                                  struct Dee_function_exceptinject *__restrict inject) {
	struct Dee_memloc typeloc;
	struct Dee_function_exceptinject_fini_and_freeinstance *me;
	me = (struct Dee_function_exceptinject_fini_and_freeinstance *)inject;
#ifdef CONFIG_TRACE_REFCHANGES
	DO(Dee_function_generator_vcallapi(self, &DeeObject_FreeTracker, VCALL_CC_RAWINTPTR, 1)); /* instance */
#endif /* CONFIG_TRACE_REFCHANGES */
	DO(vcall_DeeType_FreeInstance(self, me->fei_fafi_type)); /* N/A */
	Dee_memloc_init_const(&typeloc, me->fei_fafi_type);
	return Dee_function_generator_gdecref_loc(self, &typeloc, 1);
err:
	return -1;
}

/* func, arg, kw -> result          (return == 0)
 * func, arg, kw -> func, arg, kw   (return > 0)
 * @return: 0 : Optimization successfully applied
 * @return: 1 : Type cannot be encoded as a cast.
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_attempt_vopcallkw_as_vopcast(struct Dee_function_generator *__restrict self,
                                DeeTypeObject *type) {
	int temp;
	DO(vpop_empty_kwds(self));                   /* &DeeXXX_Type, obj */
	DO(Dee_function_generator_vpop_at(self, 2)); /* obj */
	temp = Dee_function_generator_vopcast_nofallback(self, type);
	if (temp > 0) {
		DO(Dee_function_generator_vpush_const(self, type)); /* obj, type */
		DO(Dee_function_generator_vpush_NULL(self));        /* obj, type, kw */
	}
	return temp;
err:
	return -1;
}

/* func, [args...], kw -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `Dee_TYPE(func_obj)'
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
vopcallkw_constfunc(struct Dee_function_generator *__restrict self,
                    DeeObject *func_obj, DeeTypeObject *func_type,
                    Dee_vstackaddr_t true_argc) {
	struct docinfo doc;
	ASSERT_OBJECT_TYPE_A(func_obj, func_type);
	bzero(&doc, sizeof(doc));
	if (func_type == &DeeObjMethod_Type) {
		DeeObjMethodObject *func = (DeeObjMethodObject *)func_obj;
		DO(vpop_empty_kwds(self));                                   /* func, [args...] */
		DO(Dee_function_generator_vpop_at(self, true_argc + 1));     /* [args...] */
		DO(Dee_function_generator_vpush_const(self, func->om_this)); /* [args...], this */
		DO(Dee_function_generator_vrrot(self, true_argc + 1));       /* this, [args...] */
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
			doc.di_doc = DeeObjMethod_GetDoc((DeeObject *)func);
			if (doc.di_doc != NULL)
				doc.di_typ = DeeObjMethod_GetType((DeeObject *)func);
		}
		return vcall_objmethod(self, func->om_func, true_argc, &doc, NULL, 0);
	} else if (func_type == &DeeKwObjMethod_Type) {
		DeeKwObjMethodObject *func = (DeeKwObjMethodObject *)func_obj;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
			doc.di_doc = DeeKwObjMethod_GetDoc((DeeObject *)func);
			if (doc.di_doc != NULL)
				doc.di_typ = DeeKwObjMethod_GetType((DeeObject *)func);
			DO(vinline_kwds_and_replace_with_null(self, &true_argc, &doc, NULL, NULL)); /* func, [args...], kw */
		}                                                            /* func, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, true_argc + 2));     /* [args...], kw */
		DO(Dee_function_generator_vpush_const(self, func->om_this)); /* [args...], kw, this */
		DO(Dee_function_generator_vrrot(self, true_argc + 2));       /* this, [args...], kw */
		return vcall_kwobjmethod(self, func->om_func, true_argc, &doc, NULL, 0);
	} else if (func_type == &DeeClsMethod_Type) {
		DeeClsMethodObject *func = (DeeClsMethodObject *)func_obj;
		if (true_argc >= 1) {
			Dee_vstackaddr_t argc = true_argc - 1; /* Account for "this" argument */
			DO(vpop_empty_kwds(self));                                                  /* func, this, [args...] */
			DO(Dee_function_generator_vpop_at(self, argc + 2));                         /* this, [args...] */
			DO(Dee_function_generator_vlrot(self, argc + 1));                           /* [args...], this */
			DO(Dee_function_generator_vassert_type_or_abstract_c(self, func->ob_type)); /* [args...], this */
			DO(Dee_function_generator_vrrot(self, argc + 1));                           /* this, [args...] */
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
				doc.di_doc = DeeClsMethod_GetDoc((DeeObject *)func);
				doc.di_typ = func->cm_type;
			}
			return vcall_objmethod(self, func->cm_func, argc, &doc, NULL, 0);
		}
	} else if (func_type == &DeeKwClsMethod_Type) {
		DeeKwClsMethodObject *func = (DeeKwClsMethodObject *)func_obj;
		if (true_argc >= 1) {
			Dee_vstackaddr_t argc = true_argc - 1; /* Account for "this" argument */
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
				doc.di_doc = DeeKwClsMethod_GetDoc((DeeObject *)func);
				doc.di_typ = func->cm_type;
				DO(vinline_kwds_and_replace_with_null(self, &argc, &doc, NULL, NULL));  /* func, this, [args...], kw */
			}                                                                           /* func, this, [args...], kw */
			DO(Dee_function_generator_vpop_at(self, argc + 3));                         /* this, [args...], kw */
			DO(Dee_function_generator_vlrot(self, argc + 2));                           /* [args...], kw, this */
			DO(Dee_function_generator_vassert_type_or_abstract_c(self, func->ob_type)); /* [args...], kw, this */
			DO(Dee_function_generator_vrrot(self, argc + 2));                           /* this, [args...], kw */
			return vcall_kwobjmethod(self, func->cm_func, argc, &doc, NULL, 0);         /* result */
		}
	} else if (func_type == &DeeClsProperty_Type) {
		DeeClsPropertyObject *func = (DeeClsPropertyObject *)func_obj;
		if (func->cp_get && true_argc == 1) {
			DO(vpop_empty_kwds(self));                                                  /* func, this */
			DO(Dee_function_generator_vassert_type_or_abstract_c(self, func->cp_type)); /* func, this */
			DO(Dee_function_generator_vpop_at(self, 2));                                /* this */
			doc.di_typ = func->cp_type;
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE))
				doc.di_doc = DeeClsProperty_GetDoc((DeeObject *)func);
			return vcall_getmethod(self, func->cp_get, &doc, NULL, 0);
		}
	} else if (func_type == &DeeClsMember_Type) {
		DeeClsMemberObject *func = (DeeClsMemberObject *)func_obj;
		if (true_argc == 1) {
			DO(vpop_empty_kwds(self));                                                  /* func, this */
			DO(Dee_function_generator_vassert_type_or_abstract_c(self, func->cm_type)); /* func, this */
			DO(Dee_function_generator_vpop_at(self, 2));                                /* this */
			/* XXX: Look at current instruction to see if the result needs to be a reference. */
			return Dee_function_generator_vpush_type_member(self, func->cm_type, &func->cm_memb, true);
		}
	} else if (func_type == &DeeCMethod_Type) {
		int result;
		struct cmethod_docinfo di;
		DeeCMethodObject *func = (DeeCMethodObject *)func_obj;
		DO(vpop_empty_kwds(self));                               /* func, [args...] */
		DO(Dee_function_generator_vlrot(self, true_argc + 1));   /* [args...], func */
		DO(Dee_function_generator_vpop_at(self, true_argc + 1)); /* [args...] */
		DeeCMethod_DocInfo(func->cm_func, &di);
		doc.di_doc = di.dmdi_doc;
		doc.di_mod = di.dmdi_mod;
		doc.di_typ = di.dmdi_typ;
		result = vcall_cmethod(self, func->cm_func, true_argc, &doc);
		Dee_cmethod_docinfo_fini(&di);
		return result;
	} else if (func_type == &DeeKwCMethod_Type) {
		int result;
		DeeKwCMethodObject *func = (DeeKwCMethodObject *)func_obj; /* func, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, true_argc + 2));   /* [args...], kw */
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE) {
			result = vcall_kwcmethod(self, func->cm_func, true_argc, &doc);
		} else {
			struct cmethod_docinfo di;
			DeeKwCMethod_DocInfo(func->cm_func, &di);
			doc.di_doc = di.dmdi_doc;
			doc.di_mod = di.dmdi_mod;
			doc.di_typ = di.dmdi_typ;
			result = vinline_kwds_and_replace_with_null(self, &true_argc, &doc, NULL, NULL); /* [args...], kw */
			if likely(result == 0)
				result = vcall_kwcmethod(self, func->cm_func, true_argc, &doc);
			Dee_cmethod_docinfo_fini(&di);
		}
		return result;
	} else if (func_type == &DeeType_Type) {
		DeeTypeObject *type = (DeeTypeObject *)func_obj;

		/* Try to inline keyword arguments from the type's constructor. */
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE) &&
		    (type->tp_init.tp_var.tp_any_ctor_kw != NULL)) {
			doc.di_doc = get_operator_doc(self, type, OPERATOR_CONSTRUCTOR);
			if (ITER_ISOK(doc.di_doc)) {
				doc.di_typ = type;
				DO(vinline_kwds_and_replace_with_null(self, &true_argc, &doc, NULL, NULL)); /* func, [args...], kw */
			} else {
				if unlikely(doc.di_doc == (char const *)ITER_DONE)
					goto err;
			}
		}

		/* Check for constructor calls to specific types. */
		if (true_argc == 1) {
			if (type == &DeeString_Type) {
				/* The string constructor acts just like `OPERATOR_STR' */
				DO(vpop_empty_kwds(self));                   /* &DeeString_Type, obj */
				DO(Dee_function_generator_vpop_at(self, 2)); /* obj */
				return Dee_function_generator_vopstr(self);  /* result */
			} else if (type == &DeeBool_Type) {
				/* The bool constructor acts just like `OPERATOR_BOOL' */
				DO(vpop_empty_kwds(self));                   /* &DeeBool_Type, obj */
				DO(Dee_function_generator_vpop_at(self, 2)); /* obj */
				return Dee_function_generator_vopbool(self, VOPBOOL_F_NORMAL); /* result */
			} else if (type == &DeeInt_Type
#ifdef CONFIG_HAVE_FPU
			           || type == &DeeFloat_Type
#endif /* CONFIG_HAVE_FPU */
			           ) {
				/* The int/float constructor acts just like `OPERATOR_INT/FLOAT',
				 * so-long as the passed argument isn't a string */
				DeeTypeObject *arg_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - 1);
				if (arg_type != NULL && arg_type != &DeeString_Type) {
					DO(vpop_empty_kwds(self));                   /* &DeeXXX_Type, obj */
					DO(Dee_function_generator_vpop_at(self, 2)); /* obj */
#ifdef CONFIG_HAVE_FPU
					if (type == &DeeFloat_Type)
						return Dee_function_generator_vop(self, OPERATOR_FLOAT, 1, VOP_F_PUSHRES); /* result */
#endif /* CONFIG_HAVE_FPU */
					return Dee_function_generator_vopint(self); /* result */
				}
			} else if (type == &DeeList_Type) { /* Check for sequence casts. */
				/* The constructor of "List(a)" acts just like "[a...]", except when "a" is an integer */
				DeeTypeObject *arg_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - 1);
				if (arg_type != NULL && arg_type != &DeeInt_Type) {
					int temp = do_attempt_vopcallkw_as_vopcast(self, &DeeList_Type);
					if (temp <= 0)
						return temp;
				}
			} else if (type == &DeeTuple_Type ||
			           type == &DeeDict_Type || type == &DeeRoDict_Type ||
			           type == &DeeHashSet_Type || type == &DeeRoSet_Type) {
				int temp = do_attempt_vopcallkw_as_vopcast(self, type);
				if (temp <= 0)
					return temp;
			}
		} /* if (true_argc == 1) */


		/* Check if the constructor call can be inlined. */
		if (DeeType_InheritOperator(type, OPERATOR_CONSTRUCTOR)) {
			unsigned int ctor_type;
			struct Dee_memval *kwval;
			if (DeeType_IsOperatorConstexpr(type, OPERATOR_CONSTRUCTOR) &&
			    Dee_function_generator_vallconst(self, true_argc + 1)) {
				/* Inline the result of the constructor call. */
				size_t i;
				DeeObject *kw, **argv;
				DREF DeeObject *retval;
				struct Dee_memval *argvalv;
				argv = (DeeObject **)Dee_Mallocac(true_argc, sizeof(DeeObject *));
				if unlikely(!argv)
					goto err;
				argvalv = Dee_function_generator_vtop(self) - true_argc;
				for (i = 0; i < true_argc; ++i)
					argv[i] = Dee_memval_const_getobj(&argvalv[i]);
				kw = Dee_memval_const_getobj(&argvalv[true_argc]);
				retval = DeeObject_NewKw(type, true_argc, argv, kw);
				Dee_Freea(argv);
				if likely(retval != NULL) {
					retval = Dee_function_generator_inlineref(self, retval);
					if unlikely(!retval)
						goto err;
					DO(Dee_function_generator_vpopmany(self, true_argc + 2));
					return Dee_function_generator_vpush_const(self, retval);
				} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
					goto err;
				} else {
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				}
			}

			/* Possible types of constructor calls. */
#define CTOR_TYPE_UNKNOWN    0 /* ??? */
#define CTOR_TYPE_NOARGS     1 /* MyType() */
#define CTOR_TYPE_COPY       2 /* MyType(instanceOfMyType) */
#define CTOR_TYPE_VARARGS    3 /* MyType(args...) */
#define CTOR_TYPE_VARARGS_KW 4 /* MyType(args..., **kwds) */

			/* Figure out which type of constructor call this is at compile-time.
			 *
			 * Note that NULL-ness of "kwval" is a compile-time constant, since
			 * user-code specifying keyword arguments can only specify them as
			 * non-NULL, so the only reason it might be null is that the original
			 * call simply didn't specify keyword arguments! */
			ctor_type = CTOR_TYPE_UNKNOWN;
			kwval = Dee_function_generator_vtop(self);
			if (type->tp_init.tp_var.tp_any_ctor_kw && !Dee_memval_isnull(kwval)) {
				ctor_type = CTOR_TYPE_VARARGS_KW;
			} else if (type->tp_init.tp_var.tp_ctor && true_argc == 0) {
				ctor_type = CTOR_TYPE_NOARGS;
			} else if (type->tp_init.tp_var.tp_any_ctor) {
				ctor_type = CTOR_TYPE_VARARGS;
			} else if (type->tp_init.tp_var.tp_any_ctor_kw) {
				ctor_type = CTOR_TYPE_VARARGS_KW;
			} else if (type->tp_init.tp_var.tp_copy_ctor && true_argc == 1) {
				ctor_type = CTOR_TYPE_COPY;
			}

			/* If the constructor type could be determined at compile-time, then hard-code it. */
			if (ctor_type != CTOR_TYPE_UNKNOWN) {
				if (type->tp_flags & TP_FVARIABLE) {
					/* In case of TP_FVARIABLE types, we can simply inline the call to a var-constructor */
					DO(Dee_function_generator_vnotoneref(self, true_argc + 1)); /* type, [args...], kw */
					switch (ctor_type) {
					case CTOR_TYPE_NOARGS:
						ASSERT(true_argc == 0);                /* type, kw */
						DO(vpop_empty_kwds(self));             /* type */
						DO(Dee_function_generator_vpop(self)); /* N/A */
						DO(Dee_function_generator_vcallapi(self, type->tp_init.tp_var.tp_ctor, VCALL_CC_OBJECT, 0)); /* result */
						return Dee_function_generator_vsettyp_noalias(self, type); /* result */
					case CTOR_TYPE_COPY:
						/* TODO: Optimize when `type->tp_init.tp_var.tp_copy_ctor == &DeeObject_NewRef' */
						ASSERT(true_argc == 1);                      /* type, copyme, kw */
						DO(vpop_empty_kwds(self));                   /* type, copyme */
						DO(Dee_function_generator_vpop_at(self, 2)); /* copyme */
						DO(Dee_function_generator_vcallapi(self, type->tp_init.tp_var.tp_copy_ctor, VCALL_CC_OBJECT, 1)); /* result */
						return Dee_function_generator_vsettyp_noalias(self, type); /* result */
					case CTOR_TYPE_VARARGS:                                        /* type, [args...], kw */
						DO(vpop_empty_kwds(self));                                 /* type, [args...] */
						DO(Dee_function_generator_vpop_at(self, true_argc + 1));   /* [args...] */
						DO(Dee_function_generator_vlinear(self, true_argc, true)); /* [args...], argv */
						DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* [args...], argv, argc */
						DO(Dee_function_generator_vswap(self));                    /* [args...], argc, argv */
						DO(Dee_function_generator_vcallapi_ex(self, type->tp_init.tp_var.tp_any_ctor, VCALL_CC_OBJECT, 2, true_argc + 2)); /* result */
						return Dee_function_generator_vsettyp_noalias(self, type); /* result */
					case CTOR_TYPE_VARARGS_KW:                                     /* type, [args...], kw */
						DO(Dee_function_generator_vpop_at(self, true_argc + 2));   /* [args...], kw */
						DO(Dee_function_generator_vrrot(self, true_argc + 1));     /* kw, [args...] */
						DO(Dee_function_generator_vlinear(self, true_argc, true)); /* kw, [args...], argv */
						DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* kw, [args...], argv, argc */
						DO(Dee_function_generator_vswap(self));                    /* kw, [args...], argc, argv */
						DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* [args...], argc, argv, kw */
						DO(Dee_function_generator_vcallapi_ex(self, type->tp_init.tp_var.tp_any_ctor_kw, VCALL_CC_OBJECT, 3, true_argc + 3)); /* result */
						return Dee_function_generator_vsettyp_noalias(self, type); /* result */
					default: __builtin_unreachable();
					}
				} else if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
					int temp;
					struct Dee_function_exceptinject_fini_and_freeinstance ij;
					/* TODO: If we knew that the newly constructed object never ends up in a place
					 *       where it needs to be constrained with another memory location such that
					 *       it loses its typing, and isn't returned or ever passed to a function or
					 *       operator that might end up incrementing its reference counter (i.e. if
					 *       we knew that the object only ever exists within the confines of the
					 *       function being generated), then it would be possible to allocate it as
					 *       in-line with the hstack (i.e. the Dee_memval used to access it would
					 *       have type "MEMADR_TYPE_HSTACK").
					 * A good example for this would be a iterators for sequences with a compile-time
					 * known type being iterated:
					 * >> local l = List(getItems());
					 * >> for (local x: l)
					 * >>     print repr x;
					 * The hidden iterator of "l" is known to have typing DeeListIterator_Type, and
					 * could be allocated as inlined on the host stack:
					 * >> local l = List(getItems());
					 * >> {
					 * >>     // __stack local _it = l.operator iter();
					 * >>     %{
					 * >>         ListIterator _it_storage, *_it = &_it_storage;
					 * >>         _it_storage.li_list = l;
					 * >>         _it_storage.li_index = 0;
					 * >>     }%
					 * >>     foreach (local x: _it) {
					 * >>         print repr x;
					 * >>     }
					 * >> }
					 */


					/* Hard-code the object allocation and initialization:
					 * >> DeeObject *result = DeeType_AllocInstance(type);
					 * >> if (!result)
					 * >>     HANDLE_EXCEPT();
					 * >> DeeObject_Init(result, type);
					 * >> int temp = DO_CALL_CTOR(result, args..., **kw);
					 * >> if unlikely(temp) {
					 * >>     DeeObject_FreeTracker(result);
					 * >>     DeeType_FreeInstance(type, result);
					 * >>     Dee_DecrefNokill(type);
					 * >>     HANDLE_EXCEPT();
					 * >> }
					 * >> if (type->tp_flags & TP_FGC)
					 * >>     result = DeeGC_Track(result); */

					/**/                                                     /* type, [args...], kw */
					DO(Dee_function_generator_vpop_at(self, true_argc + 2)); /* [args...], kw */
					DO(vcall_DeeType_AllocInstance(self, type));             /* [args...], kw, instance */
					ASSERT(Dee_function_generator_vtop_direct_isref(self));
					DO(Dee_function_generator_vpush_const(self, type));    /* [args...], kw, instance, type */
					DO(Dee_function_generator_vcall_DeeObject_Init(self)); /* [args...], kw, instance */
					DO(Dee_function_generator_vrrot(self, true_argc + 2)); /* instance, [args...], kw */

					/* Push custom exception cleanup handler to do the proper FreeInstance */
					Dee_function_generator_xinject_push_fini_and_freeinstance(self, &ij, type);
					ij.fei_fafi_base.fei_stack -= (true_argc + 1); /* Point at the "instance" object. */
					switch (ctor_type) {
					case CTOR_TYPE_NOARGS:
						ASSERT(true_argc == 0);
						DO(vpop_empty_kwds(self));/* instance */
						temp = vcall_Type_tp_ctor_unchecked(self, type, type->tp_init.tp_alloc.tp_ctor);
						break;
					case CTOR_TYPE_COPY:
						ASSERT(true_argc == 1);
						DO(vpop_empty_kwds(self));/* instance, copyme */
						temp = vcall_Type_tp_copy_ctor_unchecked(self, type, type->tp_init.tp_alloc.tp_copy_ctor);
						break;
					case CTOR_TYPE_VARARGS:
						DO(vpop_empty_kwds(self));/* instance, [args...] */
						temp = vcall_Type_tp_any_ctor_unchecked(self, type, true_argc, type->tp_init.tp_alloc.tp_any_ctor);
						break;
					case CTOR_TYPE_VARARGS_KW:
						temp = vcall_Type_tp_any_ctor_kw_unchecked(self, type, true_argc, type->tp_init.tp_alloc.tp_any_ctor_kw);
						break;
					default: __builtin_unreachable();
					}
					if (temp != 0) {
						if unlikely(temp < 0)
							goto err; /* instance */
						/* Simple case: the constructor could be inlined in such a way as to make it
						 * noexcept, or such that it doesn't return a "normal" 0/-1 status integer. */
					} else {
						/* Generate exception handling code while our custom exception injection is still active! */
						DO(Dee_function_generator_vdirect1(self));                                           /* instance, UNCHECKED(status_int) */
						DO(Dee_function_generator_gjnz_except(self, Dee_function_generator_vtopdloc(self))); /* instance, UNCHECKED(status_int) */
						DO(Dee_function_generator_vpop(self));                                               /* instance */
					}

					/* Now that the object has been fully initialized, get rid of the exception injection. */
					Dee_function_generator_xinject_pop_fini_and_freeinstance(self, &ij);

					/* Finalize the state of the produced object to indicate
					 * that it's been checked and contains a proper reference. */
					if (type->tp_flags & TP_FGC) {
						/* Emit code to start tracking the object. */
						ASSERT(Dee_function_generator_vtop_direct_isref(self));
						Dee_function_generator_vtop_direct_clearref(self);
						DO(Dee_function_generator_vcallapi(self, &DeeGC_Track, VCALL_CC_RAWINTPTR_NX, 1));
						ASSERT(!Dee_function_generator_vtop_direct_isref(self));
						Dee_function_generator_vtop_direct_setref(self);
					}

					DO(Dee_function_generator_vsettyp_noalias(self, type)); /* Instance */
					ASSERT(Dee_function_generator_vtop_direct_isref(self));
					return Dee_function_generator_voneref_noalias(self);
				}
			} /* if (ctor_type != CTOR_TYPE_UNKNOWN) */
#undef CTOR_TYPE_UNKNOWN
#undef CTOR_TYPE_NOARGS
#undef CTOR_TYPE_COPY
#undef CTOR_TYPE_VARARGS
#undef CTOR_TYPE_VARARGS_KW
		} /* if (DeeType_InheritOperator(type, OPERATOR_CONSTRUCTOR)) */
	}     /* if (func_type == &DeeType_Type) */

	/* No dedicated optimization available */
	return 1;
err:
	return -1;
}

/* tp_func, func, [args...], kw -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
do_impl_vopTcallkw(struct Dee_function_generator *__restrict self,
                Dee_vstackaddr_t true_argc, bool prefer_thiscall);

/* func, [args...], kw -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `func_type'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopcallkw_consttype(struct Dee_function_generator *__restrict self,
                    DeeTypeObject *func_type, Dee_vstackaddr_t true_argc,
                    bool prefer_thiscall) {
	if (func_type == &DeeObjMethod_Type) {
		DO(vpop_empty_kwds(self));                                                    /* orig_func, [args...] */
		DO(Dee_function_generator_vnotoneref(self, true_argc));                       /* orig_func, [args...] */
		DO(Dee_function_generator_vlinear(self, true_argc, true));                    /* orig_func, [args...], argv */
		DO(Dee_function_generator_vlrot(self, true_argc + 2));                        /* [args...], argv, orig_func */
		DO(Dee_function_generator_vdup(self));                                        /* [args...], argv, orig_func, func */
		DO(Dee_function_generator_vswap(self));                                       /* [args...], argv, func, orig_func */
		DO(Dee_function_generator_vrrot(self, true_argc + 3));                        /* orig_func, [args...], argv, func */
		DO(Dee_function_generator_vdup(self));                                        /* orig_func, [args...], argv, func, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeObjMethodObject, om_this))); /* orig_func, [args...], argv, func, func->om_this */
		DO(Dee_function_generator_vpush_immSIZ(self, true_argc));                     /* orig_func, [args...], argv, func, func->om_this, argc */
		DO(Dee_function_generator_vlrot(self, 4));                                    /* orig_func, [args...], func, func->om_this, argc, argv */
		DO(Dee_function_generator_vlrot(self, 4));                                    /* orig_func, [args...], func->om_this, argc, argv, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeObjMethodObject, om_func))); /* orig_func, [args...], func->om_this, argc, argv, func->om_func */
		return Dee_function_generator_vcalldynapi_ex(self, VCALL_CC_OBJECT, 3, true_argc + 4); /* result */
	} else if (func_type == &DeeKwObjMethod_Type) {
		DO(Dee_function_generator_vnotoneref(self, true_argc + 1));                     /* orig_func, [args...], kw */
		DO(Dee_function_generator_vrrot(self, true_argc + 1));                          /* orig_func, kw, [args...] */
		DO(Dee_function_generator_vlinear(self, true_argc, true));                      /* orig_func, kw, [args...], argv */
		DO(Dee_function_generator_vlrot(self, true_argc + 3));                          /* kw, [args...], argv, orig_func */
		DO(Dee_function_generator_vdup(self));                                          /* kw, [args...], argv, orig_func, func */
		DO(Dee_function_generator_vswap(self));                                         /* kw, [args...], argv, func, orig_func */
		DO(Dee_function_generator_vrrot(self, true_argc + 4));                          /* orig_func, kw, [args...], argv, func */
		DO(Dee_function_generator_vdup(self));                                          /* orig_func, kw, [args...], argv, func, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeKwObjMethodObject, om_this))); /* orig_func, kw, [args...], argv, func, func->om_this */
		DO(Dee_function_generator_vpush_immSIZ(self, true_argc));                       /* orig_func, kw, [args...], argv, func, func->om_this, argc */
		DO(Dee_function_generator_vlrot(self, 4));                                      /* orig_func, kw, [args...], func, func->om_this, argc, argv */
		DO(Dee_function_generator_vlrot(self, true_argc + 5));                          /* orig_func, [args...], func, func->om_this, argc, argv, kw */
		DO(Dee_function_generator_vlrot(self, 4));                                      /* orig_func, [args...], func->om_this, argc, argv, kw, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeKwObjMethodObject, om_func))); /* orig_func, [args...], func->om_this, argc, argv, kw, func->om_func */
		return Dee_function_generator_vcalldynapi_ex(self, VCALL_CC_OBJECT, 4, true_argc + 5); /* result */
	} else if (func_type == &DeeClsMethod_Type) {
		if (true_argc >= 1) {
			--true_argc;
			DO(vpop_empty_kwds(self));                                 /* orig_func, this, [args...] */
			DO(Dee_function_generator_vnotoneref(self, true_argc + 1));/* orig_func, this, [args...] */
			DO(Dee_function_generator_vlinear(self, true_argc, true)); /* orig_func, this, [args...], argv */
			DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* this, [args...], argv, orig_func */
			DO(Dee_function_generator_vdup(self));                     /* this, [args...], argv, orig_func, func */
			DO(Dee_function_generator_vdup(self));                     /* this, [args...], argv, orig_func, func, func */
			DO(Dee_function_generator_vlrot(self, true_argc + 5));     /* [args...], argv, orig_func, func, func, this */
			DO(Dee_function_generator_vlrot(self, 4));                 /* [args...], argv, func, func, this, orig_func */
			DO(Dee_function_generator_vrrot(self, true_argc + 5));     /* orig_func, [args...], argv, func, func, this */
			DO(Dee_function_generator_vdup(self));                     /* orig_func, [args...], argv, func, func, this, this */
			DO(Dee_function_generator_vlrot(self, 3));                 /* orig_func, [args...], argv, func, this, this, func */
			DO(Dee_function_generator_vind(self, offsetof(DeeClsMethodObject, cm_type))); /* orig_func, [args...], argv, func, this, this, func->cm_type */
			DO(Dee_function_generator_vassert_type_or_abstract(self)); /* orig_func, [args...], argv, func, this */
			DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* orig_func, [args...], argv, func, this, argc */
			DO(Dee_function_generator_vlrot(self, 4));                 /* orig_func, [args...], func, this, argc, argv */
			DO(Dee_function_generator_vlrot(self, 4));                 /* orig_func, [args...], this, argc, argv, func */
			DO(Dee_function_generator_vind(self, offsetof(DeeClsMethodObject, cm_func))); /* orig_func, [args...], this, argc, argv, func->cm_func */
			return Dee_function_generator_vcalldynapi_ex(self, VCALL_CC_OBJECT, 3, true_argc + 4); /* result */
		}
	} else if (func_type == &DeeKwClsMethod_Type) {
		if (true_argc >= 1) {
			--true_argc;
			DO(Dee_function_generator_vnotoneref(self, true_argc + 1)); /* func, [args...], kw */
			DO(Dee_function_generator_vrrot(self, true_argc + 1));     /* orig_func, this, kw, [args...] */
			DO(Dee_function_generator_vlinear(self, true_argc, true)); /* orig_func, this, [args...], argv */
			DO(Dee_function_generator_vlrot(self, true_argc + 4));     /* this, kw, [args...], argv, orig_func */
			DO(Dee_function_generator_vdup(self));                     /* this, kw, [args...], argv, orig_func, func */
			DO(Dee_function_generator_vdup(self));                     /* this, kw, [args...], argv, orig_func, func, func */
			DO(Dee_function_generator_vlrot(self, true_argc + 6));     /* kw, [args...], argv, orig_func, func, func, this */
			DO(Dee_function_generator_vlrot(self, 4));                 /* kw, [args...], argv, func, func, this, orig_func */
			DO(Dee_function_generator_vrrot(self, true_argc + 6));     /* orig_func, kw, [args...], argv, func, func, this */
			DO(Dee_function_generator_vdup(self));                     /* orig_func, kw, [args...], argv, func, func, this, this */
			DO(Dee_function_generator_vlrot(self, 3));                 /* orig_func, kw, [args...], argv, func, this, this, func */
			DO(Dee_function_generator_vind(self, offsetof(DeeKwClsMethodObject, cm_type))); /* orig_func, kw, [args...], argv, func, this, this, func->cm_type */
			DO(Dee_function_generator_vassert_type_or_abstract(self)); /* orig_func, kw, [args...], argv, func, this */
			DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* orig_func, kw, [args...], argv, func, this, argc */
			DO(Dee_function_generator_vlrot(self, 4));                 /* orig_func, kw, [args...], func, this, argc, argv */
			DO(Dee_function_generator_vlrot(self, true_argc + 5));     /* orig_func, [args...], func, this, argc, argv, kw */
			DO(Dee_function_generator_vlrot(self, 5));                 /* orig_func, [args...], this, argc, argv, kw, func */
			DO(Dee_function_generator_vind(self, offsetof(DeeKwClsMethodObject, cm_func))); /* orig_func, [args...], this, argc, argv, kw, func->cm_func */
			return Dee_function_generator_vcalldynapi_ex(self, VCALL_CC_OBJECT, 4, true_argc + 5); /* result */
		}
	} else if (func_type == &DeeClsMember_Type) {
		if (true_argc == 1) {
			DO(vpop_empty_kwds(self));                         /* func, this */
			DO(Dee_function_generator_vnotoneref_at(self, 1)); /* func, this */
			DO(Dee_function_generator_vdup(self));             /* func, this, this */
			DO(Dee_function_generator_vdup_n(self, 3));        /* func, this, this, func */
			DO(Dee_function_generator_vind(self, offsetof(DeeClsMemberObject, cm_type))); /* func, this, this, func->cm_type */
			DO(Dee_function_generator_vassert_type_or_abstract(self)); /* func, this */
			DO(Dee_function_generator_vswap(self));            /* this, func */
			DO(Dee_function_generator_vdelta(self, offsetof(DeeClsMemberObject, cm_memb))); /* this, &func->cm_memb */
			DO(Dee_function_generator_vswap(self));            /* &func->cm_memb, this */
			return Dee_function_generator_vcallapi(self, &Dee_type_member_get, VCALL_CC_OBJECT, 2); /* result */
		}
	} else if (func_type == &DeeCMethod_Type) {
		DO(vpop_empty_kwds(self));                                 /* func, [args...] */
		DO(Dee_function_generator_vnotoneref(self, true_argc));    /* func, [args...] */
		DO(Dee_function_generator_vlinear(self, true_argc, true)); /* func, [args...], argv */
		DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* func, [args...], argv, argc */
		DO(Dee_function_generator_vswap(self));                    /* func, [args...], argc, argv */
		DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* [args...], argc, argv, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeCMethodObject, cm_func))); /* [args...], argc, argv, func->cm_func */
		return Dee_function_generator_vcalldynapi_ex(self, VCALL_CC_OBJECT, 2, true_argc + 2); /* result */
	} else if (func_type == &DeeKwCMethod_Type) {
		DO(Dee_function_generator_vnotoneref(self, true_argc + 1));/* func, [args...], kw */
		DO(Dee_function_generator_vrrot(self, true_argc + 1));     /* func, kw, [args...] */
		DO(Dee_function_generator_vlinear(self, true_argc, true)); /* func, kw, [args...], argv */
		DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* func, kw, [args...], argv, argc */
		DO(Dee_function_generator_vswap(self));                    /* func, kw, [args...], argc, argv */
		DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* func, [args...], argc, argv, kw */
		DO(Dee_function_generator_vlrot(self, true_argc + 4));     /* [args...], argc, argv, kw, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeKwCMethodObject, cm_func))); /* [args...], argc, argv, kw, func->cm_func */
		return Dee_function_generator_vcalldynapi_ex(self, VCALL_CC_OBJECT, 3, true_argc + 3); /* result */
	} else if (func_type == &DeeNone_Type) {
		/* Special case: call where this-argument is "none" -> pop all arguments and re-return "none" */
		DO(Dee_function_generator_vpopmany(self, true_argc + 2)); /* N/A */
		return Dee_function_generator_vpush_none(self);           /* Dee_None */
	} else if (func_type == &DeeSuper_Type) {
		DO(Dee_function_generator_vlrot(self, true_argc + 2));                   /* [args...], kw, func */
		DO(Dee_function_generator_vdup(self));                                   /* [args...], kw, func, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_type))); /* [args...], kw, func, func->s_type */
		DO(Dee_function_generator_vdep(self));                                   /* [args...], kw, func, func->s_type */
		DO(Dee_function_generator_vrrot(self, true_argc + 3));                   /* func->s_type, [args...], kw, func */
		DO(Dee_function_generator_vdup(self));                                   /* func->s_type, [args...], kw, func, func */
		DO(Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_self))); /* func->s_type, [args...], kw, func, func->s_self */
		DO(Dee_function_generator_vdep(self));                                   /* func->s_type, [args...], kw, func, func->s_self */
		DO(Dee_function_generator_vrrot(self, true_argc + 3));                   /* func->s_type, func->s_self, [args...], kw, func */
		DO(Dee_function_generator_vpop(self));                                   /* func->s_type, func->s_self, [args...], kw */
		return do_impl_vopTcallkw(self, true_argc, prefer_thiscall);
	} else if (DeeType_InheritOperator(func_type, OPERATOR_CALL)) {
		ASSERT(func_type->tp_call || func_type->tp_call_kw);
		if (func_type == &DeeFunction_Type) {
			/* TODO: When `func_type' is a `DeeFunctionObject', see if it has already been optimized,
			 *       or if we're supposed to produce a deeply optimized code object (in which case we
			 *       have to optimize the referenced function recursively). Then, generate a direct
			 *       call to function's _hostasm representation. */
		}

		if (func_type->tp_call_kw == NULL) {
do_inline_tp_call:
			ASSERT(func_type->tp_call);
			DO(vpop_empty_kwds(self));                                 /* func, [args...] */
			DO(Dee_function_generator_vnotoneref(self, true_argc));    /* func, [args...] */
			DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_CALL, true_argc + 1)); /* func, [args...] */
			DO(Dee_function_generator_vlinear(self, true_argc, true)); /* func, [args...], argv */
			DO(Dee_function_generator_vlrot(self, true_argc + 2));     /* [args...], argv, func */
			DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* [args...], argv, func, argc */
			DO(Dee_function_generator_vlrot(self, 3));                 /* [args...], func, argc, argv */
			return Dee_function_generator_vcallapi_ex(self, func_type->tp_call, VCALL_CC_OBJECT, 3, true_argc + 3); /* result */
		} else if (func_type->tp_call == NULL) {
do_inline_tp_call_kw:
			ASSERT(func_type->tp_call_kw);
			DO(Dee_function_generator_vnotoneref(self, true_argc + 1));/* func, [args...], kw */
			DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_CALL, true_argc + 2)); /* func, [args...], kw */
			DO(Dee_function_generator_vrrot(self, true_argc + 1));     /* func, kw, [args...] */
			DO(Dee_function_generator_vlinear(self, true_argc, true)); /* func, kw, [args...], argv */
			DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* kw, [args...], argv, func */
			DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* kw, [args...], argv, func, argc */
			DO(Dee_function_generator_vlrot(self, 3));                 /* kw, [args...], func, argc, argv */
			DO(Dee_function_generator_vlrot(self, true_argc + 4));     /* [args...], func, argc, argv, kw */
			return Dee_function_generator_vcallapi_ex(self, func_type->tp_call_kw, VCALL_CC_OBJECT, 3, true_argc + 3); /* result */
		} else {
			/* Both calls are possible. Check if the caller-given "kw" is empty. */
			struct Dee_memval *kwval = Dee_function_generator_vtop(self);
			if (Dee_memval_isnull(kwval))
				goto do_inline_tp_call;
			goto do_inline_tp_call_kw;
		}
	}
	return 1; /* No dedicated optimization available */
err:
	return -1;
}

/* tp_func, func, [args...], kw -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
do_impl_vopTcallkw(struct Dee_function_generator *__restrict self,
                   Dee_vstackaddr_t true_argc, bool prefer_thiscall) {
	struct Dee_memval *funcval;
	struct Dee_memval *tpfuncval;
	ASSERT(self->fg_state->ms_stackc >= (true_argc + 3));
	tpfuncval = Dee_function_generator_vtop(self) - (true_argc + 2);
	if (!Dee_memval_isdirect(tpfuncval)) {
		DO(Dee_function_generator_vdirect_at(self, true_argc + 3));
		tpfuncval = Dee_function_generator_vtop(self) - (true_argc + 2);
	}
	ASSERT(Dee_memval_isdirect(tpfuncval));
	funcval = tpfuncval + 1;

	if (Dee_memval_direct_isconst(tpfuncval)) {
		DeeTypeObject *func_type = (DeeTypeObject *)Dee_memval_const_getobj(tpfuncval);
		int temp;                                                /* tp_func, func, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, true_argc + 3)); /* func, [args...], kw */
		funcval = Dee_function_generator_vtop(self) - (true_argc + 1);
		if (Dee_memval_isconst(funcval)) {
			DeeObject *func_obj = Dee_memval_const_getobj(funcval);
			temp = vopcallkw_constfunc(self, func_obj, func_type, true_argc);
			if (temp <= 0)
				return temp; /* Optimized call encoded, or error */
			/* Try to inline keyword arguments. */
			DO(vinline_kwds_and_replace_with_null(self, &true_argc, NULL, func_obj, func_type));
		}
		temp = vopcallkw_consttype(self, func_type, true_argc, prefer_thiscall);
		if (temp <= 0)
			return temp; /* Optimized call encoded, or error */
		DO(Dee_function_generator_vpush_const(self, func_type)); /* func, [args...], kw, tp_func */
		DO(Dee_function_generator_vrrot(self, true_argc + 3));   /* tp_func, func, [args...], kw */
	}

	/* Fallback: generate code to do a dynamic call at runtime. */
	DO(Dee_function_generator_vnotoneref(self, true_argc + 1)); /* tp_func, func, [args...], kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_CALL, true_argc + 2)); /* tp_func, func, [args...], kw */
	if (prefer_thiscall) {
		Dee_vstackaddr_t argc = true_argc - 1;
		DO(Dee_function_generator_vrrot(self, argc + 1)); /* tp_func, func, this, kw, [args...] */
		/* TODO: If generating the linear version of `[args...]' combined with `this' prefixed
		 *       is not any more complex than it is without, then include it in the argument
		 *       list and encode as `DeeObject_ThisCall()' instead. */
		DO(Dee_function_generator_vlinear(self, argc, true)); /* tp_func, func, this, kw, [args...], argv */
		DO(Dee_function_generator_vlrot(self, argc + 5));     /* func, this, kw, [args...], argv, tp_func */
		DO(Dee_function_generator_vlrot(self, argc + 5));     /* this, kw, [args...], argv, tp_func, func */
		DO(Dee_function_generator_vlrot(self, argc + 5));     /* kw, [args...], argv, tp_func, func, this */
		DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* kw, [args...], argv, tp_func, func, this, argc */
		DO(Dee_function_generator_vlrot(self, 5));            /* kw, [args...], tp_func, func, this, argc, argv */
		DO(Dee_function_generator_vlrot(self, argc + 6));     /* [args...], tp_func, func, this, argc, argv, kw */
		if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
			DO(Dee_function_generator_vpop(self));            /* [args...], tp_func, func, this, argc, argv */
			return Dee_function_generator_vcallapi_ex(self, &DeeObject_TThisCall, VCALL_CC_OBJECT, 5, argc + 5); /* result */
		}
		return Dee_function_generator_vcallapi_ex(self, &DeeObject_TThisCallKw, VCALL_CC_OBJECT, 6, argc + 6); /* result */
	}                                                      /* tp_func, func, [args...], kw */
	DO(Dee_function_generator_vrrot(self, true_argc + 1)); /* tp_func, func, kw, [args...] */
	/* TODO: If generating the linear version of `true_argc' is much more complicated
	 *       than doing the same for `true_argc - 1', then encode as `DeeObject_ThisCall()'
	 *       instead. */
	DO(Dee_function_generator_vlinear(self, true_argc, true)); /* tp_func, func, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, true_argc + 2));     /* tp_func, func, [args...], argv, kw */
	DO(Dee_function_generator_vlrot(self, true_argc + 4));     /* func, [args...], argv, kw, tp_func */
	DO(Dee_function_generator_vlrot(self, true_argc + 4));     /* [args...], argv, kw, tp_func, func */
	DO(Dee_function_generator_vrrot(self, 4));                 /* [args...], func, argv, kw, tp_func */
	DO(Dee_function_generator_vrrot(self, 4));                 /* [args...], tp_func, func, argv, kw */
	DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* [args...], tp_func, func, argv, kw, true_argc */
	DO(Dee_function_generator_vrrot(self, 3));                 /* [args...], tp_func, func, true_argc, argv, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self));                 /* [args...], tp_func, func, true_argc, argv */
		return Dee_function_generator_vcallapi_ex(self, &DeeObject_TCall, VCALL_CC_OBJECT, 4, true_argc + 4); /* result */
	}
	return Dee_function_generator_vcallapi_ex(self, &DeeObject_TCallKw, VCALL_CC_OBJECT, 5, true_argc + 5); /* result */
err:
	return -1;
}

/* func, [args...], kw -> result */
INTERN WUNUSED NONNULL((1)) int DCALL
impl_vopcallkw(struct Dee_function_generator *__restrict self,
               Dee_vstackaddr_t true_argc, bool prefer_thiscall) {
	DeeTypeObject *func_type;
	struct Dee_memval *funcval;
	if unlikely(self->fg_state->ms_stackc < (true_argc + 2))
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	funcval = Dee_function_generator_vtop(self) - (true_argc + 1);

	/* Optimizations for when the function is a constant. (e.g. `DeeObjMethodObject') */
	if (Dee_memval_isconst(funcval)) {
		DeeObject *func_obj = Dee_memval_const_getobj(funcval);
		int temp = vopcallkw_constfunc(self, func_obj, Dee_TYPE(func_obj), true_argc);
		if (temp <= 0)
			return temp; /* Optimized call encoded, or error */
		/* Try to inline keyword arguments. */
		DO(vinline_kwds_and_replace_with_null(self, &true_argc, NULL, func_obj, Dee_TYPE(func_obj)));
	}

	/* Optimizations when `type(func)' is known by skipping operator
	 * resolution and directly invoking the tp_call[_kw]-operator. */
	func_type = Dee_memval_typeof(funcval);
	if (func_type != NULL) {
		int temp = vopcallkw_consttype(self, func_type, true_argc, prefer_thiscall);
		if (temp <= 0)
			return temp; /* Optimized call encoded, or error */
	}

	/* Fallback: generate code to do a dynamic call at runtime. */
	DO(Dee_function_generator_vnotoneref(self, true_argc + 1)); /* func, [args...], kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_CALL, true_argc + 2)); /* func, [args...], kw */
	if (prefer_thiscall) {
		Dee_vstackaddr_t argc = true_argc - 1;
		DO(Dee_function_generator_vrrot(self, argc + 1)); /* func, this, kw, [args...] */
		/* TODO: If generating the linear version of `[args...]' combined with `this' prefixed
		 *       is not any more complex than it is without, then include it in the argument
		 *       list and encode as `DeeObject_ThisCall()' instead. */
		DO(Dee_function_generator_vlinear(self, argc, true)); /* func, this, kw, [args...], argv */
		DO(Dee_function_generator_vlrot(self, argc + 4));     /* this, kw, [args...], argv, func */
		DO(Dee_function_generator_vlrot(self, argc + 4));     /* kw, [args...], argv, func, this */
		DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* kw, [args...], argv, func, this, argc */
		DO(Dee_function_generator_vlrot(self, 4));            /* kw, [args...], func, this, argc, argv */
		DO(Dee_function_generator_vlrot(self, argc + 5));     /* [args...], func, this, argc, argv, kw */
		if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
			DO(Dee_function_generator_vpop(self));            /* [args...], func, this, argc, argv */
			return Dee_function_generator_vcallapi_ex(self, &DeeObject_ThisCall, VCALL_CC_OBJECT, 4, argc + 4); /* result */
		}
		return Dee_function_generator_vcallapi_ex(self, &DeeObject_ThisCallKw, VCALL_CC_OBJECT, 5, argc + 5); /* result */
	}                                                      /* func, [args...], kw */
	DO(Dee_function_generator_vrrot(self, true_argc + 1)); /* func, kw, [args...] */
	/* TODO: If generating the linear version of `true_argc' is much more complicated
	 *       than doing the same for `true_argc - 1', then encode as `DeeObject_ThisCall()'
	 *       instead. */
	DO(Dee_function_generator_vlinear(self, true_argc, true)); /* func, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, true_argc + 2));     /* func, [args...], argv, kw */
	DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* [args...], argv, kw, func */
	DO(Dee_function_generator_vrrot(self, 3));                 /* [args...], func, argv, kw */
	DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* [args...], func, argv, kw, true_argc */
	DO(Dee_function_generator_vrrot(self, 3));                 /* [args...], func, true_argc, argv, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self));                 /* [args...], func, true_argc, argv */
		return Dee_function_generator_vcallapi_ex(self, &DeeObject_Call, VCALL_CC_OBJECT, 3, true_argc + 3); /* result */
	}
	return Dee_function_generator_vcallapi_ex(self, &DeeObject_CallKw, VCALL_CC_OBJECT, 4, true_argc + 4); /* result */
err:
	return -1;
}

/* this -> Dee[Kw]ObjMethod_New(method, this)
 * @param: method: When `wrapper_type == DeeKwObjMethod_Type', this must be `Dee_kwobjmethod_t'
 * @param: wrapper_type: Either `DeeObjMethod_Type' or `DeeKwObjMethod_Type'
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vnew_ObjMethod(struct Dee_function_generator *__restrict self,
               Dee_objmethod_t method,
               DeeTypeObject *__restrict wrapper_type) {
	ASSERT(wrapper_type == &DeeObjMethod_Type ||
	       wrapper_type == &DeeKwObjMethod_Type);
	DO(Dee_function_generator_vdirect1(self)); /* this */
	if (Dee_memval_direct_isconst(Dee_function_generator_vtop(self))) {
		/* Generate compile-time constant */
		DREF DeeObject *meth;
		DeeObject *thisarg = Dee_memval_const_getobj(Dee_function_generator_vtop(self));
		meth = wrapper_type == &DeeObjMethod_Type ? DeeObjMethod_New(method, thisarg)
		                                          : DeeKwObjMethod_New((Dee_kwobjmethod_t)(void const *)method, thisarg);
		if unlikely(!meth)
			goto err;
		meth = Dee_function_generator_inlineref(self, meth);
		if unlikely(!meth)
			goto err;
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, meth);
	}
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Inline the call to `DeeObjMethod_New()' / `DeeKwObjMethod_New()' */
		STATIC_ASSERT(sizeof(DeeObjMethodObject) == sizeof(DeeKwObjMethodObject));
		DO(Dee_function_generator_vcall_DeeObject_MALLOC(self, sizeof(DeeObjMethodObject), false)); /* this, ref:result */
		DO(Dee_function_generator_vswap(self));                                                     /* ref:result, this */
		DO(Dee_function_generator_vref2(self, 2));                                                  /* ref:result, ref:this */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeObjMethodObject, om_this)));            /* ref:result */
		DO(Dee_function_generator_vpush_addr(self, (void const *)method));                          /* ref:result, method */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeObjMethodObject, om_func)));            /* ref:result */
		DO(Dee_function_generator_vcall_DeeObject_Init_c(self, wrapper_type));                      /* ref:result */
	} else {
		DO(Dee_function_generator_vpush_addr(self, (void const *)method)); /* this, method */
		DO(Dee_function_generator_vswap(self));                            /* method, this */
		DO(Dee_function_generator_vcallapi(self,
		                                   wrapper_type == &DeeObjMethod_Type ? (void const *)&DeeObjMethod_New
		                                                                      : (void const *)&DeeKwObjMethod_New,
		                                   VCALL_CC_OBJECT, 2));
		DO(Dee_function_generator_voneref_noalias(self)); /* result */
	}
	return Dee_function_generator_vsettyp_noalias(self, wrapper_type);
err:
	return -1;
}

/* func, this -> DeeInstanceMethod_New(func, this)
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vnew_InstanceMethod(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vdirect(self, 2)); /* func, this */
	if (Dee_function_generator_vallconst(self, 2)) {
		/* Generate compile-time constant */
		DREF DeeObject *meth;
		DeeObject *funcarg = Dee_memval_const_getobj(&Dee_function_generator_vtop(self)[-1]);
		DeeObject *thisarg = Dee_memval_const_getobj(&Dee_function_generator_vtop(self)[-0]);
		meth = DeeInstanceMethod_New(funcarg, thisarg);
		if unlikely(!meth)
			goto err;
		meth = Dee_function_generator_inlineref(self, meth);
		if unlikely(!meth)
			goto err;
		DO(Dee_function_generator_vpop(self)); /* func */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, meth);
	}
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Inline the behavior of `DeeInstanceMethod_New()' */
		DO(Dee_function_generator_vcall_DeeObject_MALLOC(self, sizeof(DeeInstanceMethodObject), false)); /* func, this, ref:result */
		DO(Dee_function_generator_vlrot(self, 3));                                                       /* this, ref:result, func */
		DO(Dee_function_generator_vref2(self, 3));                                                       /* this, ref:result, ref:func */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeInstanceMethodObject, im_func)));            /* this, ref:result */
		DO(Dee_function_generator_vswap(self));                                                          /* ref:result, this */
		DO(Dee_function_generator_vref2(self, 2));                                                       /* ref:result, ref:this */
		DO(Dee_function_generator_vpopind(self, offsetof(DeeInstanceMethodObject, im_this)));            /* ref:result */
		DO(Dee_function_generator_vcall_DeeObject_Init_c(self, &DeeInstanceMethod_Type));                /* ref:result */
	} else {
		DO(Dee_function_generator_vnotoneref(self, 2));                                        /* func, this */
		DO(Dee_function_generator_vcallapi(self, &DeeInstanceMethod_New, VCALL_CC_OBJECT, 2)); /* result */
		DO(Dee_function_generator_voneref_noalias(self));                                      /* result */
	}
	return Dee_function_generator_vsettyp_noalias(self, &DeeInstanceMethod_Type);
err:
	return -1;
}

/* a, b -> value */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpop_twice_and_push_constant_value(struct Dee_function_generator *__restrict self,
                                   /*inherit(always)*/ DREF DeeObject *value) {
	value = Dee_function_generator_inlineref(self, value);
	if unlikely(!value)
		goto err;                          /* a, b */
	DO(Dee_function_generator_vpop(self)); /* a */
	DO(Dee_function_generator_vpop(self)); /* N/A */
	return Dee_function_generator_vpush_const(self, value);
err:
	return -1;
}

/* this, attr -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopgetattr_constattr(struct Dee_function_generator *__restrict self,
                     struct attrinfo const *__restrict attr) {
	switch (attr->ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *item = attr->ai_value.v_custom;
		if (item->tp_getattr) {
			DO(Dee_function_generator_vnotoneref_at(self, 1)); /* this, attr */
			DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, 2)); /* this, attr */
			return Dee_function_generator_vcallapi(self, item->tp_getattr, VCALL_CC_OBJECT, 2);
		}
	}	break;

	case Dee_ATTRINFO_ATTR:
		DO(Dee_function_generator_vpop(self)); /* this */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                  attr->ai_value.v_attr, true);

	case Dee_ATTRINFO_METHOD: {
		/* Return a `DeeObjMethod_Type' / `DeeKwObjMethod_Type' wrapper */
		struct type_method const *item = attr->ai_value.v_method;
		Dee_objmethod_t method = item->m_func;
		DeeTypeObject *wrapper_type = &DeeObjMethod_Type;
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS)
			wrapper_type = &DeeKwObjMethod_Type;
		DO(Dee_function_generator_vpop(self)); /* this */
		return vnew_ObjMethod(self, method, wrapper_type);
	}	break;

	case Dee_ATTRINFO_GETSET: {
		struct docinfo di;
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_get == NULL)
			break;
		di.di_doc = item->gs_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl; /* this, attr */
		DO(Dee_function_generator_vpop(self));      /* this */
		return vcall_getmethod(self, item->gs_get, &di, item->gs_name, item->gs_flags);
	}	break;

	case Dee_ATTRINFO_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;        /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_type_member(self, (DeeTypeObject *)attr->ai_decl, item, true);
	}	break;

	case Dee_ATTRINFO_INSTANCE_ATTR: {
		DREF DeeObject *value;
		struct class_attribute const *item;
		item = attr->ai_value.v_instance_attr; /* this, attr */
		if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FGETSET) {
			DeeTypeObject *type = (DeeTypeObject *)attr->ai_decl;
			struct class_desc *desc = DeeClass_DESC(type);
			/* Wrapper for producing `DeeProperty_Type' */
			DO(Dee_function_generator_vpop(self));     /* this */
			DO(Dee_function_generator_vdirect1(self)); /* this */
			if ((item->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM) &&
			    !(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOROINLINE)) {
				/* See if we can produce a compile-time constant */
				DREF DeePropertyObject *prop;
				DREF DeeObject *get, *del = NULL, *set = NULL;
				Dee_class_desc_lock_read(desc);
				get = desc->cd_members[item->ca_addr + Dee_CLASS_GETSET_GET];
				if (get) {
					Dee_Incref(get);
					if (!(item->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY)) {
						del = desc->cd_members[item->ca_addr + Dee_CLASS_GETSET_DEL];
						Dee_XIncref(del);
						set = desc->cd_members[item->ca_addr + Dee_CLASS_GETSET_SET];
						Dee_XIncref(set);
					}
					Dee_class_desc_lock_endread(desc);
					prop = DeeObject_MALLOC(DeePropertyObject);
					if unlikely(!prop)
						goto err;
					DeeObject_Init(prop, &DeeProperty_Type);
					prop->p_get = get; /* Inherit reference */
					prop->p_del = del; /* Inherit reference */
					prop->p_set = set; /* Inherit reference */
					prop = (DREF DeePropertyObject *)Dee_function_generator_inlineref(self, (DeeObject *)prop);
					if unlikely(!prop)
						goto err;
					DO(Dee_function_generator_vpop(self)); /* N/A */
					return Dee_function_generator_vpush_const(self, prop);
				}
				Dee_class_desc_lock_endread(desc);
			}

			/* Construct the object at runtime. */
			if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM)
				DO(Dee_function_generator_vpop(self)); /* N/A */
			DO(Dee_function_generator_vcall_DeeObject_MALLOC(self, sizeof(DeePropertyObject), false)); /* [this], ref:result */
			if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM) {
				DO(Dee_function_generator_grwlock_read_const(self, &desc->cd_lock));                 /* ref:result */
				DO(Dee_function_generator_vpush_addr(self, &desc->cd_members[item->ca_addr + Dee_CLASS_GETSET_GET])); /* ref:result, &GETTER */
				DO(Dee_function_generator_vind(self, 0));                                            /* ref:result, GETTER */
				DO(Dee_function_generator_vdirect1(self));                                           /* ref:result, GETTER */
				DO(Dee_function_generator_gxincref_loc(self, Dee_function_generator_vtopdloc(self), 1)); /* ref:result, ref:GETTER */
				if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY) {
					DO(Dee_function_generator_vreg(self, NULL));                                     /* ref:result, ref:GETTER */
					DO(Dee_function_generator_grwlock_endread_const(self, &desc->cd_lock));          /* ref:result, ref:GETTER */
					DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_get)));    /* ref:result */
				} else {
					DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_get)));    /* ref:result */
					DO(Dee_function_generator_vpush_addr(self, &desc->cd_members[item->ca_addr + Dee_CLASS_GETSET_DEL])); /* ref:result, &DELETE */
					DO(Dee_function_generator_vind(self, 0));                                        /* ref:result, DELETE */
					DO(Dee_function_generator_vdirect1(self));                                       /* ref:result, DELETE */
					DO(Dee_function_generator_gxincref_loc(self, Dee_function_generator_vtopdloc(self), 1)); /* ref:result, ref:DELETE */
					DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_del)));    /* ref:result */
					DO(Dee_function_generator_vpush_addr(self, &desc->cd_members[item->ca_addr + Dee_CLASS_GETSET_SET])); /* ref:result, &SETTER */
					DO(Dee_function_generator_vind(self, 0));                                        /* ref:result, SETTER */
					DO(Dee_function_generator_vdirect1(self));                                       /* ref:result, SETTER */
					DO(Dee_function_generator_gxincref_loc(self, Dee_function_generator_vtopdloc(self), 1)); /* ref:result, ref:SETTER */
					DO(Dee_function_generator_vreg(self, NULL));                                     /* ref:result, ref:SETTER */
					DO(Dee_function_generator_grwlock_endread_const(self, &desc->cd_lock));          /* ref:result, ref:SETTER */
					DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_set)));    /* ref:result */
				}
			} else {
				DO(Dee_function_generator_vdup_n(self, 2));               /* this, ref:result, this */
				DO(Dee_function_generator_vdelta(self, desc->cd_offset)); /* this, ref:result, DeeInstance_DESC(desc, this) */
				DO(Dee_function_generator_vdup(self));                    /* this, ref:result, DeeInstance_DESC(desc, this), DeeInstance_DESC(desc, this) */
				DO(Dee_function_generator_vdelta(self, offsetof(struct Dee_instance_desc, id_lock))); /* this, ref:result, DeeInstance_DESC(desc, this), &DeeInstance_DESC(desc, this)->id_lock */
				DO(Dee_function_generator_vdirect1(self));                /* this, ref:result, DeeInstance_DESC(desc, this), &DeeInstance_DESC(desc, this)->id_lock */
				DO(Dee_function_generator_grwlock_read(self, Dee_function_generator_vtopdloc(self))); /* this, ref:result, DeeInstance_DESC(desc, this), &DeeInstance_DESC(desc, this)->id_lock */
				DO(Dee_function_generator_vpop(self));                    /* this, ref:result, DeeInstance_DESC(desc, this) */
				DO(Dee_function_generator_vdup(self));                    /* this, ref:result, DeeInstance_DESC(desc, this), DeeInstance_DESC(desc, this) */
				DO(Dee_function_generator_vind(self, offsetof(struct Dee_instance_desc, id_vtab) +
				                                     (item->ca_addr + Dee_CLASS_GETSET_GET) *
				                                     sizeof(DREF DeeObject *))); /* this, ref:result, DeeInstance_DESC(desc, this), GETTER */
				if (!(item->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY)) {
					DO(Dee_function_generator_vdirect1(self));            /* this, ref:result, DeeInstance_DESC(desc, this), ref:GETTER */
					DO(Dee_function_generator_gxincref_loc(self, Dee_function_generator_vtopdloc(self), 1)); /* this, ref:result, DeeInstance_DESC(desc, this), ref:GETTER */
					DO(Dee_function_generator_vdup_n(self, 3));           /* this, ref:result, DeeInstance_DESC(desc, this), ref:GETTER, result */
					DO(Dee_function_generator_vswap(self));               /* this, ref:result, DeeInstance_DESC(desc, this), result, ref:GETTER */
					DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_get))); /* this, ref:result, DeeInstance_DESC(desc, this), result */
					DO(Dee_function_generator_vdup_n(self, 2));           /* this, ref:result, DeeInstance_DESC(desc, this), result, DeeInstance_DESC(desc, this) */
					DO(Dee_function_generator_vind(self, offsetof(struct Dee_instance_desc, id_vtab) +
					                                     (item->ca_addr + Dee_CLASS_GETSET_DEL) *
					                                     sizeof(DREF DeeObject *))); /* this, ref:result, DeeInstance_DESC(desc, this), result, DELETE */
					DO(Dee_function_generator_vdirect1(self));            /* this, ref:result, DeeInstance_DESC(desc, this), result, ref:DELETE */
					DO(Dee_function_generator_gxincref_loc(self, Dee_function_generator_vtopdloc(self), 1)); /* this, ref:result, DeeInstance_DESC(desc, this), result, ref:DELETE */
					DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_del))); /* this, ref:result, DeeInstance_DESC(desc, this), result */
					DO(Dee_function_generator_vpop(self));                /* this, ref:result, DeeInstance_DESC(desc, this) */
					DO(Dee_function_generator_vdup(self));                /* this, ref:result, DeeInstance_DESC(desc, this), DeeInstance_DESC(desc, this) */
					DO(Dee_function_generator_vind(self, offsetof(struct Dee_instance_desc, id_vtab) +
					                                     (item->ca_addr + Dee_CLASS_GETSET_SET) *
					                                     sizeof(DREF DeeObject *))); /* this, ref:result, DeeInstance_DESC(desc, this), SETTER */
				}                                                         /* this, ref:result, DeeInstance_DESC(desc, this), GETTER_OR_SETTER */
				DO(Dee_function_generator_vdirect1(self));                /* this, ref:result, DeeInstance_DESC(desc, this), GETTER_OR_SETTER */
				DO(Dee_function_generator_gxincref_loc(self, Dee_function_generator_vtopdloc(self), 1)); /* this, ref:result, DeeInstance_DESC(desc, this), ref:GETTER_OR_SETTER */
				DO(Dee_function_generator_vreg(self, NULL));              /* this, ref:result, DeeInstance_DESC(desc, this), ref:GETTER_OR_SETTER */
				DO(Dee_function_generator_vswap(self));                   /* this, ref:result, ref:GETTER_OR_SETTER, DeeInstance_DESC(desc, this) */
				DO(Dee_function_generator_vdelta(self, offsetof(struct Dee_instance_desc, id_lock))); /* this, ref:result, ref:GETTER_OR_SETTER, &DeeInstance_DESC(desc, this)->id_lock */
				DO(Dee_function_generator_vdirect1(self));                /* this, ref:result, ref:GETTER_OR_SETTER, &DeeInstance_DESC(desc, this)->id_lock */
				DO(Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtopdloc(self))); /* this, ref:result, ref:GETTER_OR_SETTER, &DeeInstance_DESC(desc, this)->id_lock */
				DO(Dee_function_generator_vpop(self));                    /* this, ref:result, ref:GETTER_OR_SETTER */
				DO(Dee_function_generator_vpopind(self, (item->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY)
				                                        ? offsetof(DeePropertyObject, p_get)
				                                        : offsetof(DeePropertyObject, p_set))); /* this, ref:result */
				DO(Dee_function_generator_vpop_at(self, 2));                                    /* ref:result */
			}                                                                                   /* ref:result */
			if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY) {
				DO(Dee_function_generator_vpush_NULL(self));                                  /* ref:result, NULL */
				DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_del))); /* ref:result */
				DO(Dee_function_generator_vpush_NULL(self));                                  /* ref:result, NULL */
				DO(Dee_function_generator_vpopind(self, offsetof(DeePropertyObject, p_set))); /* ref:result */
			}
			DO(Dee_function_generator_vcall_DeeObject_Init_c(self, &DeeProperty_Type)); /* ref:result */
			return 0;
		} else if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD) {
			/* Wrapper for producing `DeeInstanceMethod_Type' */
			DO(Dee_function_generator_vpop(self)); /* this */
			if (item->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM) {
				DO(Dee_function_generator_vpush_const(self, attr->ai_decl)); /* this, type */
				DO(Dee_function_generator_vpush_cmember(self, item->ca_addr, /* this, func */
				                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF));
				DO(Dee_function_generator_vswap(self)); /* func, this */
				return vnew_InstanceMethod(self);       /* result */
			} else {
				DO(Dee_function_generator_vpush_const(self, attr->ai_decl)); /* this, type */
			}
			return vnew_InstanceMethod(self);
		} else {
			/* Wrapper for producing `DeeInstanceMember_Type' */
			value = DeeInstanceMember_New((DeeTypeObject *)attr->ai_decl, item);
			if unlikely(!value)
				goto err;
			return vpop_twice_and_push_constant_value(self, value);
		}
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD: {
		/* Wrapper for producing `DeeClsMethod_Type' / `DeeKwClsMethod_Type' */
		struct type_method const *item = attr->ai_value.v_instance_method;
		DREF DeeObject *value;
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS) {
			value = DeeKwClsMethod_New((DeeTypeObject *)attr->ai_decl,
			                           (Dee_kwobjmethod_t)(void const *)item->m_func);
		} else {
			value = DeeClsMethod_New((DeeTypeObject *)attr->ai_decl, item->m_func);
		}
		if unlikely(!value)
			goto err;
		return vpop_twice_and_push_constant_value(self, value);
	}	break;

	case Dee_ATTRINFO_INSTANCE_GETSET: {
		/* Wrapper for producing `DeeClsProperty_Type' */
		struct type_getset const *item = attr->ai_value.v_instance_getset;
		DREF DeeObject *value;
		value = DeeClsProperty_New((DeeTypeObject *)attr->ai_decl,
		                           item->gs_get,
		                           item->gs_del,
		                           item->gs_set);
		if unlikely(!value)
			goto err;
		return vpop_twice_and_push_constant_value(self, value);
	}	break;

	case Dee_ATTRINFO_INSTANCE_MEMBER: {
		/* Wrapper for producing `DeeClsMember_Type' */
		struct type_member const *item = attr->ai_value.v_instance_member;
		DREF DeeObject *value;
		value = DeeClsMember_New((DeeTypeObject *)attr->ai_decl, item);
		if unlikely(!value)
			goto err;
		return vpop_twice_and_push_constant_value(self, value);
	}	break;

	case Dee_ATTRINFO_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_module_symbol(self, (DeeModuleObject *)attr->ai_decl, sym, true);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}

/* this, attr -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopboundattr_constattr(struct Dee_function_generator *__restrict self,
                       struct attrinfo const *__restrict attr) {
	switch (attr->ai_type) {

	case Dee_ATTRINFO_CUSTOM:
		/* XXX: Inline a call to tp_getattr, then check for UnboundAttribute & friends on error? */
		break;

	case Dee_ATTRINFO_ATTR:
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vbound_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                   attr->ai_value.v_attr);

	case Dee_ATTRINFO_GETSET: {
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_bound == NULL)
			break;
		return vcall_boundmethod(self, item->gs_bound,
		                         (DeeTypeObject *)attr->ai_decl,
		                         item->gs_name, item->gs_flags);
	}	break;

	case Dee_ATTRINFO_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;        /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vbound_type_member(self, item);
	}	break;

	case Dee_ATTRINFO_METHOD:
	case Dee_ATTRINFO_INSTANCE_ATTR:
	case Dee_ATTRINFO_INSTANCE_METHOD:
	case Dee_ATTRINFO_INSTANCE_GETSET:
	case Dee_ATTRINFO_INSTANCE_MEMBER:
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, Dee_True);

	case Dee_ATTRINFO_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vpop(self));            /* N/A */
		return Dee_function_generator_vbound_module_symbol(self, (DeeModuleObject *)attr->ai_decl, sym);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}

/* this, attr -> N/A
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopdelattr_constattr(struct Dee_function_generator *__restrict self,
                     struct attrinfo const *__restrict attr) {
	switch (attr->ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *item = attr->ai_value.v_custom;
		if (item->tp_delattr) {
			DO(Dee_function_generator_vnotoneref_at(self, 1)); /* this, attr */
			DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_DELATTR, 2)); /* this, attr */
			return Dee_function_generator_vcallapi(self, item->tp_delattr, VCALL_CC_INT, 2);
		}
	}	break;

	case Dee_ATTRINFO_ATTR:
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vdel_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                 attr->ai_value.v_attr);

	case Dee_ATTRINFO_GETSET: {
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_del == NULL)
			break;
		DO(Dee_function_generator_vpop(self)); /* this */
		return vcall_delmethod(self, item->gs_del,
		                       (DeeTypeObject *)attr->ai_decl,
		                       item->gs_name, item->gs_flags);
	}	break;

	case Dee_ATTRINFO_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;        /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vdel_type_member(self, item);
	}	break;

	case Dee_ATTRINFO_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vdel_module_symbol(self, (DeeModuleObject *)attr->ai_decl, sym);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}

/* this, attr, value -> N/A
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopsetattr_constattr(struct Dee_function_generator *__restrict self,
                     struct attrinfo const *__restrict attr) {
	switch (attr->ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *item = attr->ai_value.v_custom;
		if (item->tp_setattr) {
			DO(Dee_function_generator_vnotoneref(self, 2)); /* this, attr, value */
			DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SETATTR, 3)); /* this, attr, value */
			return Dee_function_generator_vcallapi(self, item->tp_setattr, VCALL_CC_INT, 3);
		}
	}	break;

	case Dee_ATTRINFO_ATTR:                          /* this, attr, value */
		DO(Dee_function_generator_vpop_at(self, 2)); /* this, value */
		return Dee_function_generator_vpop_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                 attr->ai_value.v_attr);

	case Dee_ATTRINFO_GETSET: {
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_set == NULL)
			break;                                   /* this, attr, value */
		DO(Dee_function_generator_vpop_at(self, 2)); /* this, value */
		return vcall_setmethod(self, item->gs_set,
		                       (DeeTypeObject *)attr->ai_decl,
		                       item->gs_name, item->gs_flags);
	}	break;

	case Dee_ATTRINFO_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;              /* this, attr, value */
		DO(Dee_function_generator_vpop_at(self, 2)); /* this, value */
		return Dee_function_generator_vpop_type_member(self, item);
	}	break;

	case Dee_ATTRINFO_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym; /* this, attr, value */
		DO(Dee_function_generator_vrrot(self, 3));                     /* value, this, attr */
		DO(Dee_function_generator_vpop(self));                         /* value, this */
		DO(Dee_function_generator_vpop(self));                         /* value */
		return Dee_function_generator_vpop_module_symbol(self, (DeeModuleObject *)attr->ai_decl, sym);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}


/* this, attr, [args...], kw -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
vopcallattrkw_constattr(struct Dee_function_generator *__restrict self,
                        Dee_vstackaddr_t argc,
                        struct attrinfo const *__restrict attr) {
	switch (attr->ai_type) {

	case Dee_ATTRINFO_ATTR:
		/* NOTE: In this case, we're allowed to assume that "this" is an instance of "attr->ai_decl" */
		DO(Dee_function_generator_vpop_at(self, argc + 2)); /* this, [args...], kw */
		return Dee_function_generator_vcall_instance_attrkw(self, (DeeTypeObject *)attr->ai_decl,
		                                                    attr->ai_value.v_attr, argc);

	case Dee_ATTRINFO_METHOD: {
		struct docinfo di;
		struct type_method const *item;
		item      = attr->ai_value.v_method;
		di.di_doc = item->m_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl;         /* this, attr, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, argc + 2)); /* this, [args...], kw */
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS) {
			return vcall_kwobjmethod(self, (Dee_kwobjmethod_t)(void const *)item->m_func,
			                         argc, &di, item->m_name, item->m_flag);
		}
		DO(vpop_empty_kwds(self)); /* this, [args...] */
		return vcall_objmethod(self, item->m_func, argc, &di, item->m_name, item->m_flag);
	}	break;

	case Dee_ATTRINFO_INSTANCE_ATTR: {
		uint16_t callback_addr;
		struct Dee_class_attribute const *item;
		if (argc < 1)
			break;
		--argc; /* type, attr, this, [args...], kw */
		item = attr->ai_value.v_instance_attr;
		/* Behavior here mirrors `DeeClass_CallInstanceAttributeKw()' */
		if (!(item->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
			if (argc != 0)
				break;
			/* XXX: "kw" here doesn't need to be empty. It is also allowed to be "thisarg: foo" */
			DO(vpop_empty_kwds(self));                 /* type, attr, this */
			DO(Dee_function_generator_vrrot(self, 3)); /* this, type, attr */
			DO(Dee_function_generator_vpop(self));     /* this, type */
			DO(Dee_function_generator_vpop(self));     /* this */
			DO(Dee_function_generator_vassert_type_or_abstract_c(self, (DeeTypeObject *)attr->ai_decl)); /* this */
			/* XXX: Look at current instruction to see if the result needs to be a reference. */
			return Dee_function_generator_vpush_instance_attr(self, (DeeTypeObject *)attr->ai_decl, item, true);
		}                                                   /* type, attr, this, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, argc + 4)); /* attr, this, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, argc + 3)); /* this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 2));   /* [args...], kw, this */
		DO(Dee_function_generator_vassert_type_or_abstract_c(self, (DeeTypeObject *)attr->ai_decl)); /* [args...], kw, this */
		DO(Dee_function_generator_vpush_const(self, attr->ai_decl)); /* [args...], kw, this, decl_type */
		callback_addr = item->ca_addr;
#if CLASS_GETSET_GET != 0
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			callback_addr += CLASS_GETSET_GET;
#endif /* CLASS_GETSET_GET != 0 */
		DO(Dee_function_generator_vpush_imember(self, callback_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], kw, func */
		DO(Dee_function_generator_vlrot(self, argc + 2));    /* func, [args...], kw */
		return Dee_function_generator_vopcallkw(self, argc); /* result */
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD: {
		struct docinfo di;
		struct type_method const *item;
		if (argc < 1)
			break;
		item      = attr->ai_value.v_instance_method;
		di.di_doc = item->m_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl;
		--argc;                                             /* type, attr, this, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, argc + 4)); /* attr, this, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, argc + 3)); /* this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 2));   /* [args...], kw, this */
		DO(Dee_function_generator_vassert_type_or_abstract_c(self, (DeeTypeObject *)attr->ai_decl)); /* [args...], kw, this */
		DO(Dee_function_generator_vrrot(self, argc + 2));   /* this, [args...], kw */
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS) {
			return vcall_kwobjmethod(self, (Dee_kwobjmethod_t)(void const *)item->m_func,
			                         argc, &di, item->m_name, item->m_flag);
		}
		DO(vpop_empty_kwds(self)); /* this, [args...] */
		return vcall_objmethod(self, item->m_func, argc, &di, item->m_name, item->m_flag); /* result */
	}	break;

	case Dee_ATTRINFO_INSTANCE_GETSET: {
		struct docinfo di;
		struct type_getset const *item;
		if (argc != 1)
			break;
		item = attr->ai_value.v_instance_getset;
		if (item->gs_get == NULL)
			break;
		di.di_doc = item->gs_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl;       /* type, attr, this, kw */
		DO(vpop_empty_kwds(self));                        /* type, attr, this */
		DO(Dee_function_generator_vrrot(self, 3));        /* this, type, attr */
		DO(Dee_function_generator_vpop(self));            /* this, type */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vassert_type_or_abstract_c(self, (DeeTypeObject *)attr->ai_decl)); /* this */
		return vcall_getmethod(self, item->gs_get, &di, item->gs_name, item->gs_flags);  /* result */
	}	break;

	case Dee_ATTRINFO_INSTANCE_MEMBER: {
		struct type_member const *item;
		if (argc != 1)
			break;
		item = attr->ai_value.v_instance_member;
		DO(vpop_empty_kwds(self));                        /* type, attr, this */
		DO(Dee_function_generator_vrrot(self, 3));        /* this, type, attr */
		DO(Dee_function_generator_vpop(self));            /* this, type */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vassert_type_or_abstract_c(self, (DeeTypeObject *)attr->ai_decl)); /* this */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_type_member(self, (DeeTypeObject *)attr->ai_decl, item, true);
	}	break;

	default: {
		/* Fallback: try to load the attribute as-is, and then call it */
		int result;
		DO(Dee_function_generator_vlrot(self, argc + 3)); /* attr, [args...], kw, this */
		DO(Dee_function_generator_vlrot(self, argc + 3)); /* [args...], kw, this, attr */
		result = vopgetattr_constattr(self, attr);
		if (result <= 0) {
			if unlikely(result < 0)
				goto err;                                        /* [args...], kw, func */
			DO(Dee_function_generator_vrrot(self, argc + 2));    /* func, [args...], kw */
			return Dee_function_generator_vopcallkw(self, argc); /* result */
		}
	}	break;

	}
	return 1;
err:
	return -1;
}


/* tp_this, this, attr, [args...], kw -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
do_impl_vopTcallattrkw(struct Dee_function_generator *__restrict self,
                       Dee_vstackaddr_t argc) {
	struct attrinfo attr;
	struct Dee_memval *thisval;
	struct Dee_memval *typeval;
	struct Dee_memval *attrval;
	ASSERT(self->fg_state->ms_stackc >= (argc + 4));

	/* Load memory locations. */
	attrval = Dee_function_generator_vtop(self) - (argc + 1);
	thisval = attrval - 1;
	typeval = thisval - 1;
	ASSERT(Dee_memval_isdirect(attrval));

	/* Special handling for when  */
	if (Dee_memval_isconst(typeval)) {
		DeeTypeObject *type = (DeeTypeObject *)Dee_memval_const_getobj(typeval);
		ASSERT_OBJECT_TYPE(type, &DeeType_Type);            /* tp_this, this, attr, [args...], kw */
		DO(Dee_function_generator_vpop_at(self, argc + 4)); /* this, attr, [args...], kw */
		attrval = Dee_function_generator_vtop(self) - (argc + 1);
		thisval = attrval - 1;
		ASSERT(Dee_memval_isdirect(attrval));
		if (Dee_memval_direct_isconst(attrval)) {
			DeeObject *this_value_or_null = NULL;
			DeeStringObject *attr_obj = (DeeStringObject *)Dee_memval_const_getobj(attrval);
			ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
			if (Dee_memval_isconst(thisval))
				this_value_or_null = Dee_memval_const_getobj(thisval);
			if (DeeObject_TFindAttrInfo(type, this_value_or_null, (DeeObject *)attr_obj, &attr)) {
				int temp = vopcallattrkw_constattr(self, argc, &attr);
				if (temp <= 0)
					return temp; /* Optimization applied, or error */
			}
		}

		/* Inline the call to query the attribute */
		if (type->tp_attr) {
			int temp;
			attr.ai_type = Dee_ATTRINFO_CUSTOM;
			attr.ai_decl = (DeeObject *)type;
			attr.ai_value.v_custom = type->tp_attr;
			temp = vopcallattrkw_constattr(self, argc, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}

		DO(Dee_function_generator_vpush_const(self, type)); /* this, attr, [args...], kw, tp_this */
		DO(Dee_function_generator_vrrot(self, argc + 4));   /* tp_this, this, attr, [args...], kw */
	}

	/* Fallback: perform a generic TCallAttr operation at runtime. */
	DO(Dee_function_generator_vnotoneref(self, argc + 3)); /* tp_this, this, attr, [args...], kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, argc + 3)); /* tp_this, this, attr, [args...], kw */
	DO(Dee_function_generator_vrrot(self, argc + 1));      /* tp_this, this, attr, kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));  /* tp_this, this, attr, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 5));      /* this, attr, kw, [args...], argv, tp_this */
	DO(Dee_function_generator_vlrot(self, argc + 5));      /* attr, kw, [args...], argv, tp_this, this */
	DO(Dee_function_generator_vlrot(self, argc + 5));      /* kw, [args...], argv, tp_this, this, attr */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));   /* kw, [args...], argv, tp_this, this, attr, argc */
	DO(Dee_function_generator_vlrot(self, 5));             /* kw, [args...], tp_this, this, attr, argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 6));      /* [args...], tp_this, this, attr, argc, argv, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self));             /* [args...], tp_this, this, attr, argc, argv */
		return Dee_function_generator_vcallapi_ex(self, &DeeObject_TCallAttr, VCALL_CC_OBJECT, 5, argc + 5); /* result */
	}
	return Dee_function_generator_vcallapi_ex(self, &DeeObject_TCallAttrKw, VCALL_CC_OBJECT, 6, argc + 6); /* result */
err:
	return -1;
}

/* this, attr, [args...], kw -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vopcallattrkw(struct Dee_function_generator *__restrict self,
                   Dee_vstackaddr_t argc) {
	struct attrinfo attr;
	DeeTypeObject *this_type;
	struct Dee_memval *thisval;
	struct Dee_memval *attrval;
	if unlikely(self->fg_state->ms_stackc < (argc + 3))
		return err_illegal_stack_effect();

	/* Generate code to assert that "attr" is a string. */
	DO(Dee_function_generator_vlrot(self, argc + 2));                       /* this, [args...], kw, attr */
	DO(Dee_function_generator_vdirect1(self));                              /* this, [args...], kw, attr */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeString_Type)); /* this, [args...], kw, attr */
	DO(Dee_function_generator_vrrot(self, argc + 2));                       /* this, attr, [args...], kw */

	/* Load memory locations. */
	DO(Dee_function_generator_state_unshare(self));
	attrval = Dee_function_generator_vtop(self) - (argc + 1);
	thisval = attrval - 1;
	ASSERT(Dee_memval_isdirect(attrval));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memval_typeof(thisval);
	if (this_type != NULL) {
		if (thisval->mv_vmorph == MEMVAL_VMORPH_SUPER) { /* Special case for super-morphs */
			struct Dee_memobjs *objs = Dee_memval_getobjn(thisval);
			ASSERT(this_type == &DeeSuper_Type);
			ASSERT(objs->mos_objc == 2);
			DO(Dee_function_generator_vpush_memobj(self, &objs->mos_objv[1])); /* this, attr, [args...], kw, tp_self */
			DO(Dee_function_generator_vrrot(self, argc + 3));                  /* this, tp_self, attr, [args...], kw */
			DO(Dee_function_generator_vpush_memobj(self, &objs->mos_objv[0])); /* this, tp_self, attr, [args...], kw, self */
			DO(Dee_function_generator_vrrot(self, argc + 3));                  /* this, tp_self, self, attr, [args...], kw */
			DO(Dee_function_generator_vpop_at(self, argc + 5));                /* tp_self, self, attr, [args...], kw */
			return do_impl_vopTcallattrkw(self, argc);
		}
		if (Dee_memval_direct_isconst(attrval)) {
			DeeObject *this_value_or_null = NULL;
			DeeStringObject *attr_obj = (DeeStringObject *)Dee_memval_const_getobj(attrval);
			ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
			if (Dee_memval_isconst(thisval))
				this_value_or_null = Dee_memval_const_getobj(thisval);
			if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, (DeeObject *)attr_obj, &attr)) {
				int temp = vopcallattrkw_constattr(self, argc, &attr);
				if (temp <= 0)
					return temp; /* Optimization applied, or error */
			}
		}

		/* Inline the call to query the attribute */
		if (this_type->tp_attr) {
			int temp;
			attr.ai_type = Dee_ATTRINFO_CUSTOM;
			attr.ai_decl = (DeeObject *)this_type;
			attr.ai_value.v_custom = this_type->tp_attr;
			temp = vopcallattrkw_constattr(self, argc, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Fallback: perform a generic CallAttr operation at runtime. */
	DO(Dee_function_generator_vnotoneref(self, argc + 3)); /* this, attr, [args...], kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, argc + 3)); /* this, attr, [args...], kw */
	DO(Dee_function_generator_vrrot(self, argc + 1));      /* this, attr, kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));  /* this, attr, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 4));      /* attr, kw, [args...], argv, this */
	DO(Dee_function_generator_vlrot(self, argc + 4));      /* kw, [args...], argv, this, attr */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));   /* kw, [args...], argv, this, attr, argc */
	DO(Dee_function_generator_vlrot(self, 4));             /* kw, [args...], this, attr, argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 5));      /* [args...], this, attr, argc, argv, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self));             /* [args...], this, attr, argc, argv */
		return Dee_function_generator_vcallapi_ex(self, &DeeObject_CallAttr, VCALL_CC_OBJECT, 4, argc + 4); /* result */
	}
	return Dee_function_generator_vcallapi_ex(self, &DeeObject_CallAttrKw, VCALL_CC_OBJECT, 5, argc + 5); /* result */
err:
	return -1;
}

/* func, [args...], kw -> result -- Invoke `DeeObject_CallKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallkw(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t argc) {
	return impl_vopcallkw(self, argc, false);
}

/* func, args, kw -> result -- Invoke `DeeObject_CallTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcalltuplekw(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimizations for MEMADR_TYPE_CONST functions with certain types. (e.g. `DeeObjMethodObject') */
	DO(Dee_function_generator_vswap(self));                                /* func, kw, args */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeTuple_Type)); /* func, kw, args */
	DO(Dee_function_generator_vswap(self));                                /* func, args, kw */
	DO(Dee_function_generator_vnotoneref(self, 2));                        /* func, args, kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_CALL, 3)); /* func, args, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self)); /* func, args */
		return Dee_function_generator_vcallapi(self, &DeeObject_CallTuple, VCALL_CC_OBJECT, 2);
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_CallTupleKw, VCALL_CC_OBJECT, 3);
err:
	return -1;
}

/* func, this, [args...], kw -> result -- Invoke `DeeObject_ThisCallKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscallkw(struct Dee_function_generator *__restrict self,
                                               Dee_vstackaddr_t argc) {
	return impl_vopcallkw(self, argc + 1, true);
}

/* func, this, args, kw -> result -- Invoke `DeeObject_ThisCallTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscalltuplekw(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimizations for MEMADR_TYPE_CONST functions with certain types. (e.g. `DeeClsMethodObject') */
	/* TODO: Optimizations when `type(func)' is known by skipping operator resolution and directly invoking the call-operator */
	DO(Dee_function_generator_vswap(self));                                        /* func, this, kw, args */
	DO(Dee_function_generator_vassert_type_exact_if_safe_c(self, &DeeTuple_Type)); /* func, this, kw, args */
	DO(Dee_function_generator_vswap(self));                                        /* func, this, args, kw */
	DO(Dee_function_generator_vnotoneref(self, 3));                                /* func, this, args, kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_CALL, 4));  /* func, this, args, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self)); /* func, this, args */
		return Dee_function_generator_vcallapi(self, &DeeObject_ThisCallTuple, VCALL_CC_OBJECT, 3);
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_ThisCallTupleKw, VCALL_CC_OBJECT, 4);
err:
	return -1;
}

/* this, attr, [args...], kw -> result -- Invoke `DeeObject_CallAttrKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrkw(struct Dee_function_generator *__restrict self,
                                     Dee_vstackaddr_t argc) {
	return impl_vopcallattrkw(self, argc);
}

/* this, attr, args, kw -> result -- Invoke `DeeObject_CallAttrTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrtuplekw(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimization for when `attr' and the type of `this' is known:
	 * >> return "a,b,c".split(x); // Inline the actual call to `string_split()',
	 * >>                          // bypassing the complete attribute lookup */
	DO(Dee_function_generator_vlrot(self, 3));                                       /* this, args, kw, attr */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeString_Type));          /* this, args, kw, attr */
	DO(Dee_function_generator_vlrot(self, 3));                                       /* this, kw, attr, args */
	DO(Dee_function_generator_vassert_type_exact_if_safe_c(self, &DeeTuple_Type));   /* this, kw, attr, args */
	DO(Dee_function_generator_vlrot(self, 3));                                       /* this, attr, args, kw */
	DO(Dee_function_generator_vnotoneref(self, 3));                                  /* this, attr, args, kw */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, 4)); /* this, attr, args, kw */
	if (Dee_memval_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self)); /* this, attr, args */
		return Dee_function_generator_vcallapi(self, &DeeObject_CallAttrTuple, VCALL_CC_OBJECT, 3);
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_CallAttrTupleKw, VCALL_CC_OBJECT, 4);
err:
	return -1;
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
visspare_location(struct Dee_memstate const *__restrict self,
                  struct Dee_memval const *__restrict loc,
                  Dee_vstackaddr_t ms_stackc_override) {
	Dee_lid_t i;
	size_t n_aliases = 0;
	size_t n_references = 0;
	ASSERT(Dee_memval_isdirect(loc));
	for (i = 0; i < ms_stackc_override; ++i) {
		struct Dee_memval const *alias = &self->ms_stackv[i];
		if (!Dee_memval_sameval(loc, alias))
			continue;
		++n_aliases;
		if (Dee_memval_direct_isref(alias))
			++n_references;
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memval const *alias = &self->ms_localv[i];
		if (!Dee_memval_sameval(loc, alias))
			continue;
		++n_aliases;
		if (Dee_memval_direct_isref(alias))
			++n_references;
	}
	if (Dee_memval_direct_isref(loc))
		++n_references;
	if (Dee_memval_isconst(loc) && n_references)
		return true;
	return n_aliases > 0 ? n_references >= 2
	                     : n_references >= 1;
}

/* Check the top "n" v-stack elements and count how many
 * are currently holding a "spare" object reference:
 * >> GIVEN MEMLOC L;
 * >> LET N_ALIASES    = NUMBER OF ALIASES (NOT IN VTOP(n)) FOR L;
 * >> LET N_REFERENCES = NUMBER OF ALIASES (NOT IN VTOP(n)) WITHOUT "MEMOBJ_F_NOREF" FOR L;
 * >> IF L DOES NOT HAVE "MEMOBJ_F_NOREF" THEN
 * >>     N_REFERENCES = N_REFERENCES + 1;
 * >> FI
 * >> IF L IS "MEMADR_TYPE_CONST" AND N_REFERENCES >= 1 THEN
 * >>     N_REFERENCES = INT_MAX;
 * >> FI
 * >> LET L_HAS_SPARE_REFERENCES = N_ALIASES > 0 ? N_REFERENCES >= 2
 * >>                                            : N_REFERENCES >= 1;
 * In practice, this means:
 *  - the # of locations that are holding references and aren't aliased
 *  - plus: the # of locations that have aliases outside the top "n" v-stack
 *          elements, with this set of aliases having at least 2 references.
 *  - A special case is made for constants, where there simply needs to be
 *    at least 1 reference somewhere in "L" or the previously mentioned set
 *    of aliases. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) Dee_vstackaddr_t DCALL
vspare_location_count(struct Dee_memstate const *__restrict self,
                      Dee_vstackaddr_t n) {
	Dee_vstackaddr_t vbase_offset, i, result = 0;
	struct Dee_memval const *vbase;
	vbase_offset = self->ms_stackc - n;
	vbase = self->ms_stackv + vbase_offset;
	for (i = 0; i < n; ++i) {
		if (visspare_location(self, &vbase[i], vbase_offset))
			++result;
	}
	return result;
}


/* func, [attr], [items...] -> result */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vopcallseqmap_impl(struct Dee_function_generator *__restrict self,
                   Dee_vstackaddr_t itemc, bool asmap, bool hasattr,
                   bool allow_vpackseq) {
	struct Dee_function_exceptinject_callvoidapi ij;
	struct Dee_memval *funcval;
	DeeTypeObject *func_type;
	DeeTypeObject *shared_type;
	void const *api_create;
	void const *api_decref;
	Dee_vstackaddr_t n_spare_refs;
	bool gift_references;
	ASSERTF(!asmap || (itemc % 2) == 0,
	        "Need an even number of elements for a mapping type");

	if (itemc == 0) {
		/* Special case: use an empty sequence */
		DeeObject *seq_or_map = asmap ? Dee_EmptyMapping : Dee_EmptyTuple;
		DO(Dee_function_generator_vpush_const(self, seq_or_map));
		return hasattr ? Dee_function_generator_vopcallattr(self, 1)
		               : Dee_function_generator_vopcall(self, 1);
	}

	/* Check if "func" is a known sequence type. If so, then try to
	 * encode the call as an optimized sequence pack expression. */
	funcval = Dee_function_generator_vtop(self) - (hasattr ? (itemc + 1) : itemc);
	if (Dee_memval_isconst(funcval)) {
		DeeObject *func = Dee_memval_const_getobj(funcval);
		if (!hasattr && allow_vpackseq && DeeType_Check(func) &&
		    (!!asmap == !!DeeType_InheritsFrom((DeeTypeObject *)func, &DeeMapping_Type)) &&
		    ((DeeTypeObject *)func == &DeeList_Type || (DeeTypeObject *)func == &DeeTuple_Type ||
		     (DeeTypeObject *)func == &DeeDict_Type || (DeeTypeObject *)func == &DeeRoDict_Type ||
		     (DeeTypeObject *)func == &DeeHashSet_Type || (DeeTypeObject *)func == &DeeRoSet_Type)) {
			/**/                                                 /* func, [items...] */
			DO(Dee_function_generator_vpop_at(self, itemc + 1)); /* [items...] */
			return Dee_function_generator_vpackseq(self, (DeeTypeObject *)func, itemc);
		}

		/* XXX: More optimizations? */
	}

	func_type = Dee_memval_typeof(funcval);
	if (func_type) {
		/* XXX: this optimization is possible whenever the function doesn't let passed arguments escape!
		 *   -> optimize this by adding another flag `Dee_TYPE_METHOD_FNOARGREFESCAPE' that can then be
		 *      set on a per-function basis.
		 * NOPE: not that easy. Technically, `string.format' can let its argument escape, because it
		 *       calls `operator iter' on its argument. What we need to know here is: "can references
		 *       escape, assuming that OPERATOR_SEQ_ENUMERATE doesn't let references escape?" */
#ifndef CONFIG_TRACE_REFCHANGES
		if (func_type == &DeeString_Type && hasattr && !asmap &&
		    Dee_memval_isconst(&funcval[1]) &&
		    DeeString_Check(Dee_memval_const_getobj(&funcval[1])) &&
		    DeeString_EQUALS_ASCII(Dee_memval_const_getobj(&funcval[1]), "format")) {
			/* Special optimization for template strings:
			 * >> function foo(a, b, c) {
			 * >>     return f"Function foo({a}, {b}, {c}) called";
			 * >> }
			 *
			 * ASM (deemon):
			 * >> push  const @"Function foo({}, {}, {}) called"
			 * >> push  arg @a
			 * >> push  arg @b
			 * >> push  arg @c
			 * >> callattr top, @"format", [#2]
			 * >> ret   pop
			 *
			 * Since we know that "String.format" never incref's its args-sequence
			 * (without later decref'ing it), we can cheat a bit here and push a
			 * couple more things in order to create an in-line Tuple:
			 * >> 1                 (ob_refcnt)
			 * >> &DeeTuple_Type    (ob_type)
			 * >> itemc             (t_size)
			 * >> [items...]        (t_elem)
			 *
			 * ASM (i386):
			 * >>     movl  8(%esp), %eax // argv
			 * >>     pushl 8(%eax)       // c
			 * >>     pushl 4(%eax)       // b
			 * >>     pushl 0(%eax)       // a
			 * >>     pushl $3            // # of arguments
			 * >>     pushl $DeeTuple_Type
			 * >>     pushl $1            // Fake tuple: ob_refcnt
			 * >>     pushl %esp          // argv[0]   (DeeTupleObject *)
			 * >>     pushl %esp          // argv      (DeeTupleObject **)
			 * >>     pushl $1            // argc
			 * >>     pushl $ADDROF("Function foo({}, {}, {}) called")
			 * >>     call  string_format
			 * >>     addl  $28, %esp
			 * >>     ret   $8 */
			/* TODO: Inline constant arguments into the format string */
			DO(Dee_function_generator_vnotoneref(self, itemc));               /* func, attr, [items...] */ /* XXX: Based on how items are used by the format string... ("{}" -> OPERATOR_STR, "{!r}" -> OPERATOR_REPR) */
			DO(Dee_function_generator_vpush_immSIZ(self, itemc));             /* func, attr, [items...], t_size */
			DO(Dee_function_generator_vrrot(self, itemc + 1));                /* func, attr, t_size, [items...] */
			DO(Dee_function_generator_vpush_const(self, &DeeTuple_Type));     /* func, attr, t_size, [items...], ob_type */
			DO(Dee_function_generator_vrrot(self, itemc + 2));                /* func, attr, ob_type, t_size, [items...] */
			DO(Dee_function_generator_vpush_immSIZ(self, 1));                 /* func, attr, ob_type, t_size, [items...], 1 */
			DO(Dee_function_generator_vrrot(self, itemc + 3));                /* func, attr, 1, ob_type, t_size, [items...] */
			DO(Dee_function_generator_vlinear(self, itemc + 3, false));       /* func, attr, 1, ob_type, t_size, [items...], fake_tuple */
			DO(Dee_function_generator_vsettyp_noalias(self, &DeeTuple_Type)); /* func, attr, 1, ob_type, t_size, [items...], fake_tuple */
			DO(Dee_function_generator_vlrot(self, itemc + 6));                /* attr, 1, ob_type, t_size, [items...], fake_tuple, func */
			DO(Dee_function_generator_vlrot(self, itemc + 6));                /* 1, ob_type, t_size, [items...], fake_tuple, func, attr */
			DO(Dee_function_generator_vlrot(self, 3));                        /* 1, ob_type, t_size, [items...], func, attr, fake_tuple */
			DO(Dee_function_generator_vopcallattr(self, 1));                  /* 1, ob_type, t_size, [items...], result */
			DO(Dee_function_generator_vrrot(self, itemc + 4));                /* result, 1, ob_type, t_size, [items...] */
			return Dee_function_generator_vpopmany(self, itemc + 3);          /* result */
		}
#endif /* !CONFIG_TRACE_REFCHANGES */
	}

	/* Check for special case: all items are constants
	 * -> Pack into either a tuple, or an RoDict, then call the function like that. */
	if (Dee_function_generator_vallconst(self, itemc)) {
		size_t i;
		struct Dee_memval *elemv;
		DREF DeeObject *cseq;
		elemv = Dee_function_generator_vtop(self) - (itemc - 1);
		if (asmap) {
			cseq = (DREF DeeObject *)DeeRoDict_NewWithHint((size_t)(itemc / 2));
			if unlikely(!cseq)
				goto err;
			for (i = 0; i < (size_t)(itemc / 2); ++i) {
				DeeObject *key   = Dee_memval_const_getobj(&elemv[(i * 2) + 0]);
				DeeObject *value = Dee_memval_const_getobj(&elemv[(i * 2) + 1]);
				if unlikely(DeeRoDict_Insert((DeeRoDictObject **)&cseq, key, value)) {
/*err_cseq:*/
					Dee_Decref_likely(cseq);
					goto err;
				}
			}
		} else {
			cseq = (DREF DeeObject *)DeeTuple_NewUninitialized(itemc);
			if unlikely(!cseq)
				goto err;
			for (i = 0; i < itemc; ++i) {
				DeeObject *elem = Dee_memval_const_getobj(&elemv[i]);
				Dee_Incref(elem);
				DeeTuple_SET(cseq, i, elem); /* Inherit reference */
			}
		}
		cseq = Dee_function_generator_inlineref(self, cseq);
		if unlikely(!cseq)
			goto err;
		DO(Dee_function_generator_vpopmany(self, itemc));   /* func, [attr] */
		DO(Dee_function_generator_vpush_const(self, cseq)); /* func, [attr], cseq */
		return hasattr ? Dee_function_generator_vopcallattr(self, 1)
		               : Dee_function_generator_vopcall(self, 1);
	}

	/* Fallback: Generate code similar to what the deemon interpreter already does.
	 *           The only exception here is that we're also able to choose if we
	 *           want the vector to inherit references or not. */
	shared_type = &DeeSharedVector_Type;
	api_create = (void const *)&DeeSharedVector_NewShared;
	api_decref = (void const *)&DeeSharedVector_DecrefNoGiftItems;
	if (asmap) {
		shared_type = &DeeSharedMap_Type;
		api_create = (void const *)&DeeSharedMap_NewShared;
		api_decref = (void const *)&DeeSharedMap_DecrefNoGiftItems;
	}

	/* Figure out how many spare references there are within the items that are
	 * supposed to end up as part of the Shared{Vector|Map}. If at least half of
	 * those items can act as spare references, then force all items to  */
	n_spare_refs = vspare_location_count(self->fg_state, itemc);
	gift_references = (n_spare_refs * 2) >= itemc;
	if (gift_references) {
		Dee_vstackaddr_t i, total = hasattr ? itemc + 2 : itemc + 1;
		for (i = 0; i < itemc; ++i) {
			DO(Dee_function_generator_vlrot(self, itemc)); /* func, [attr], [items...] */
			DO(Dee_function_generator_vref2(self, total)); /* func, [attr], [items...] */
		}                                                  /* func, [attr], [items...] */
		api_decref = asmap ? (void const *)&DeeSharedMap_Decref
		                   : (void const *)&DeeSharedVector_Decref;
#ifndef NDEBUG
		for (i = 0; i < itemc; ++i) {
			struct Dee_memval *itemval;
			itemval = self->fg_state->ms_stackv + self->fg_state->ms_stackc - itemc + i;
			ASSERT(Dee_memval_isdirect(itemval));
			ASSERT(Dee_memval_direct_isref(itemval));
		}
#endif /* !NDEBUG */
	}                                                                          /* func, [attr], [items...] */
	DO(Dee_function_generator_vnotoneref(self, itemc));                        /* func, [attr], [items...] */
	DO(Dee_function_generator_vlinear(self, itemc, true));                     /* func, [attr], [items...], itemv */
	DO(Dee_function_generator_vpush_immSIZ(self, asmap ? itemc / 2 : itemc));  /* func, [attr], [items...], itemv, itemc */
	DO(Dee_function_generator_vswap(self));                                    /* func, [attr], [items...], itemc, itemv */
	DO(Dee_function_generator_vcallapi(self, api_create, VCALL_CC_OBJECT, 2)); /* func, [attr], [items...], custom:seq */
	DO(Dee_function_generator_vdirect1(self));                                 /* func, [attr], [items...], custom:seq */
	Dee_function_generator_vtop_direct_clearref(self);                         /* func, [attr], [items...], custom:seq */ /* Not a "normal" reference */
	DO(Dee_function_generator_vsettyp_noalias(self, shared_type));             /* func, [attr], [items...], custom:seq */
	DO(Dee_function_generator_vlrot(self, hasattr ? itemc + 3 : itemc + 2));   /* [attr], [items...], custom:seq, func */
	if (hasattr)                                                               /* attr, [items...], custom:seq, func */
		DO(Dee_function_generator_vlrot(self, itemc + 3));                     /* [items...], custom:seq, func, attr */
	Dee_function_generator_xinject_push_callvoidapi(self, &ij, api_decref, 1); /* [items...], custom:seq, func, [attr] */
	ij.fei_cva_base.fei_stack -= hasattr ? 2 : 1; /* Fix stack address to point at "custom:seq" */

	/* Mark all of the items as (no longer) holding any references. */
	if (gift_references) {
		Dee_vstackaddr_t i;
		struct Dee_memval *itemv;
		itemv = self->fg_state->ms_stackv + self->fg_state->ms_stackc - (itemc + 2);
		if (hasattr)
			--itemv;
		for (i = 0; i < itemc; ++i) {
			struct Dee_memval *mval = &itemv[i];
			ASSERT(Dee_memval_isdirect(mval));
			ASSERT(Dee_memval_direct_isref(mval));
			Dee_memval_direct_clearref(mval); /* Reference got stolen by the shared vector/map. */
		}
	}
	if (hasattr) {                                                              /* [items...], custom:seq, func, attr */
		DO(Dee_function_generator_vdup_n(self, 3));                             /* [items...], custom:seq, func, attr, custom:seq */
		DO(Dee_function_generator_vopcallattr(self, 1));                        /* [items...], custom:seq, result */
	} else {                                                                    /* [items...], custom:seq, func */
		DO(Dee_function_generator_vdup_n(self, 2));                             /* [items...], custom:seq, func, custom:seq */
		DO(Dee_function_generator_vopcall(self, 1));                            /* [items...], custom:seq, result */
	}                                                                           /* [items...], custom:seq, result */
	Dee_function_generator_xinject_pop_callvoidapi(self, &ij);                  /* [items...], custom:seq, result */
	DO(Dee_function_generator_vrrot(self, itemc + 2));                          /* result, [items...], custom:seq */
	DO(Dee_function_generator_vcallapi(self, api_decref, VCALL_CC_VOID_NX, 1)); /* result, [items...] */
	return Dee_function_generator_vpopmany(self, itemc);                        /* result */
err:
	return -1;
}


/* func, [items...] -> result -- Invoke `DeeObject_Call(func, DeeSharedVector_NewShared(...))' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallseq(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t itemc) {
	return vopcallseqmap_impl(self, itemc, false, false, true);
}

/* func, [[key, value]...] -> result -- Invoke `DeeObject_Call(func, DeeSharedMap_NewShared(...))' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallmap(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t pairc) {
	return vopcallseqmap_impl(self, pairc * 2, true, false, true);
}

/* func, attr, [items...] -> result -- Invoke `DeeObject_CallAttr(func, attr, DeeSharedVector_NewShared(...))' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrseq(struct Dee_function_generator *__restrict self,
                                      Dee_vstackaddr_t itemc) {
	return vopcallseqmap_impl(self, itemc, false, true, true);
}

/* func, attr, [[key, value]...] -> result -- Invoke `DeeObject_CallAttr(func, attr, DeeSharedMap_NewShared(...))' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrmap(struct Dee_function_generator *__restrict self,
                                      Dee_vstackaddr_t pairc) {
	return vopcallseqmap_impl(self, pairc * 2, true, true, true);
}




/* func, [args...] -> [args...], result -- Invoke `DeeObject_Call()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcall(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpush_NULL(self);
	if likely(result == 0) /* func, [args...], kw=NULL */
		result = Dee_function_generator_vopcallkw(self, argc);
	return result;
}

/* func, args -> result -- Invoke `DeeObject_CallTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcalltuple(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_NULL(self);
	if likely(result == 0) /* func, args, kw=NULL */
		result = Dee_function_generator_vopcalltuplekw(self);
	return result;
}

/* func, this, [args...] -> [args...], result -- Invoke `DeeObject_ThisCall()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscall(struct Dee_function_generator *__restrict self,
                                   Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpush_NULL(self);
	if likely(result == 0) /* func, this, [args...], kw=NULL */
		result = Dee_function_generator_vopthiscallkw(self, argc);
	return result;
}

/* func, this, args -> result -- Invoke `DeeObject_ThisCallTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscalltuple(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_NULL(self);
	if likely(result == 0) /* func, this, args, kw=NULL */
		result = Dee_function_generator_vopthiscalltuplekw(self);
	return result;
}

/* this, attr, [args...] -> [args...], result -- Invoke `DeeObject_CallAttr()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattr(struct Dee_function_generator *__restrict self,
                                   Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpush_NULL(self);
	if likely(result == 0) /* this, attr, [args...], kw=NULL */
		result = Dee_function_generator_vopcallattrkw(self, argc);
	return result;
}

/* this, attr, args -> result -- Invoke `DeeObject_CallAttrTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrtuple(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_NULL(self);
	if likely(result == 0) /* this, attr, args, kw=NULL */
		result = Dee_function_generator_vopcallattrtuplekw(self);
	return result;
}


/* this, attr -> result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopgetattr(struct Dee_function_generator *__restrict self) {
	struct attrinfo attr;
	DeeTypeObject *this_type;
	struct Dee_memval *thisval;
	struct Dee_memval *attrval;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_vdirect1(self));                              /* this, attr */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeString_Type)); /* this, attr */
	DO(Dee_function_generator_state_unshare(self));

	/* Load value locations. */
	attrval = Dee_function_generator_vtop(self);
	thisval = attrval - 1;
	ASSERT(Dee_memval_isdirect(attrval));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memval_typeof(thisval);
	if (this_type != NULL) {
		if (thisval->mv_vmorph == MEMVAL_VMORPH_SUPER) { /* Special case for super-morphs */
			/* TODO */
		}
		if (Dee_memval_direct_isconst(attrval)) {
			DeeObject *this_value_or_null = NULL;
			DeeStringObject *attr_obj = (DeeStringObject *)Dee_memval_const_getobj(attrval);
			ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
			if (Dee_memval_isconst(thisval))
				this_value_or_null = Dee_memval_const_getobj(thisval);
			if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, (DeeObject *)attr_obj, &attr)) {
				int temp = vopgetattr_constattr(self, &attr);
				if (temp <= 0)
					return temp; /* Optimization applied, or error */
			}
		}

		/* Inline the call to query the attribute */
		if (this_type->tp_attr) {
			int temp;
			attr.ai_type = Dee_ATTRINFO_CUSTOM;
			attr.ai_decl = (DeeObject *)this_type;
			attr.ai_value.v_custom = this_type->tp_attr;
			temp = vopgetattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Fallback: emit a runtime attribute lookup. */
	DO(Dee_function_generator_vnotoneref_at(self, 1));                               /* this, attr */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, 2)); /* this, attr */
	return Dee_function_generator_vcallapi(self, &DeeObject_GetAttr, VCALL_CC_OBJECT, 2);
err:
	return -1;
}

/* this, attr -> hasattr */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vophasattr(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimizations */

	/* Fallback: emit a runtime attribute lookup. */
	DO(Dee_function_generator_vnotoneref_at(self, 1));                               /* this, attr */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, 2)); /* this, attr */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_HasAttr, VCALL_CC_NEGINT, 2));
	DO(Dee_function_generator_vdirect1(self));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
	return 0;
err:
	return -1;
}

/* this, attr -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopboundattr(struct Dee_function_generator *__restrict self) {
	struct attrinfo attr;
	DeeTypeObject *this_type;
	struct Dee_memval *thisval;
	struct Dee_memval *attrval;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_vdirect1(self));                              /* this, attr */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeString_Type)); /* this, attr */
	DO(Dee_function_generator_state_unshare(self));

	/* Load value locations. */
	attrval = Dee_function_generator_vtop(self);
	thisval = attrval - 1;
	ASSERT(Dee_memval_isdirect(attrval));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memval_typeof(thisval);
	if (this_type != NULL) {
		if (thisval->mv_vmorph == MEMVAL_VMORPH_SUPER) { /* Special case for super-morphs */
			/* TODO */
		}
		if (Dee_memval_direct_isconst(attrval)) {
			DeeObject *this_value_or_null = NULL;
			DeeStringObject *attr_obj = (DeeStringObject *)Dee_memval_const_getobj(attrval);
			ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
			if (Dee_memval_isconst(thisval))
				this_value_or_null = Dee_memval_const_getobj(thisval);
			if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, (DeeObject *)attr_obj, &attr)) {
				int temp = vopboundattr_constattr(self, &attr);
				if (temp <= 0)
					return temp; /* Optimization applied, or error */
			}
		}

		/* Inline the call to query the attribute */
		if (this_type->tp_attr) {
			int temp;
			attr.ai_type = Dee_ATTRINFO_CUSTOM;
			attr.ai_decl = (DeeObject *)this_type;
			attr.ai_value.v_custom = this_type->tp_attr;
			temp = vopboundattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Fallback: emit a runtime attribute lookup. */
	DO(Dee_function_generator_vnotoneref_at(self, 1));                               /* this, attr */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETATTR, 2)); /* this, attr */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_BoundAttr, VCALL_CC_M1INT, 2));
	DO(Dee_function_generator_vdirect1(self));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* this, attr -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopdelattr(struct Dee_function_generator *__restrict self) {
	struct attrinfo attr;
	DeeTypeObject *this_type;
	struct Dee_memval *thisval;
	struct Dee_memval *attrval;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_vdirect1(self));                              /* this, attr */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeString_Type)); /* this, attr */
	DO(Dee_function_generator_state_unshare(self));

	/* Load value locations. */
	attrval = Dee_function_generator_vtop(self);
	thisval = attrval - 1;
	ASSERT(Dee_memval_isdirect(attrval));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memval_typeof(thisval);
	if (this_type != NULL) {
		if (thisval->mv_vmorph == MEMVAL_VMORPH_SUPER) { /* Special case for super-morphs */
			/* TODO */
		}
		if (Dee_memval_direct_isconst(attrval)) {
			DeeObject *this_value_or_null = NULL;
			DeeStringObject *attr_obj = (DeeStringObject *)Dee_memval_const_getobj(attrval);
			ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
			if (Dee_memval_isconst(thisval))
				this_value_or_null = Dee_memval_const_getobj(thisval);
			if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, (DeeObject *)attr_obj, &attr)) {
				int temp = vopdelattr_constattr(self, &attr);
				if (temp <= 0)
					return temp; /* Optimization applied, or error */
			}
		}

		/* Inline the call to query the attribute */
		if (this_type->tp_attr) {
			int temp;
			attr.ai_type = Dee_ATTRINFO_CUSTOM;
			attr.ai_decl = (DeeObject *)this_type;
			attr.ai_value.v_custom = this_type->tp_attr;
			temp = vopdelattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Fallback: emit a runtime attribute lookup. */
	DO(Dee_function_generator_vnotoneref_at(self, 1));                               /* this, attr */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_DELATTR, 2)); /* this, attr */
	return Dee_function_generator_vcallapi(self, &DeeObject_DelAttr, VCALL_CC_INT, 2);
err:
	return -1;
}

/* this, attr, value -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsetattr(struct Dee_function_generator *__restrict self) {
	struct attrinfo attr;
	DeeTypeObject *this_type;
	struct Dee_memval *thisval;
	struct Dee_memval *attrval;
	if unlikely(self->fg_state->ms_stackc < 3)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_vswap(self));                                 /* this, value, attr */
	DO(Dee_function_generator_vdirect1(self));                              /* this, value, attr */
	DO(Dee_function_generator_vassert_type_exact_c(self, &DeeString_Type)); /* this, value, attr */
	DO(Dee_function_generator_vswap(self));                                 /* this, attr, value */
	DO(Dee_function_generator_state_unshare(self));

	/* Load memory locations. */
	DO(Dee_function_generator_state_unshare(self));
	attrval = Dee_function_generator_vtop(self) - 1;
	thisval = attrval - 1;
	ASSERT(Dee_memval_isdirect(attrval));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memval_typeof(thisval);
	if (this_type != NULL) {
		if (thisval->mv_vmorph == MEMVAL_VMORPH_SUPER) { /* Special case for super-morphs */
			/* TODO */
		}
		if (Dee_memval_direct_isconst(attrval)) {
			DeeObject *this_value_or_null = NULL;
			DeeStringObject *attr_obj = (DeeStringObject *)Dee_memval_const_getobj(attrval);
			ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
			if (Dee_memval_isconst(thisval))
				this_value_or_null = Dee_memval_const_getobj(thisval);
			if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, (DeeObject *)attr_obj, &attr)) {
				int temp = vopsetattr_constattr(self, &attr);
				if (temp <= 0)
					return temp; /* Optimization applied, or error */
			}
		}

		/* Inline the call to query the attribute */
		if (this_type->tp_attr) {
			int temp;
			attr.ai_type = Dee_ATTRINFO_CUSTOM;
			attr.ai_decl = (DeeObject *)this_type;
			attr.ai_value.v_custom = this_type->tp_attr;
			temp = vopsetattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Fallback: emit a runtime attribute lookup. */
	DO(Dee_function_generator_vnotoneref(self, 2));                                  /* this, attr, value */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SETATTR, 3)); /* this, attr, value */
	return Dee_function_generator_vcallapi(self, &DeeObject_SetAttr, VCALL_CC_INT, 3);
err:
	return -1;
}



/* seq, key_or_index, def -> result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopgetitemdef(struct Dee_function_generator *__restrict self) {
	/* IMPORTANT: `def' is allowed to be `ITER_DONE', and `DeeObject_GetItemDef()' is allowed to return `ITER_DONE' */
	DeeTypeObject *seq_type;
	if unlikely(self->fg_state->ms_stackc < 3)
		return err_illegal_stack_effect();
	seq_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - 2);
	if (seq_type) {
		/* Try to inline a constant expression */
		if (Dee_function_generator_vallconst(self, 3) &&
		    DeeType_IsOperatorConstexpr(seq_type, OPERATOR_GETITEM)) {
			DREF DeeObject *result;
			DeeObject *seq = Dee_memval_const_getobj(&Dee_function_generator_vtop(self)[-2]);
			DeeObject *key = Dee_memval_const_getobj(&Dee_function_generator_vtop(self)[-1]);
			DeeObject *def = Dee_memval_const_getobj(&Dee_function_generator_vtop(self)[-0]);
			result = DeeObject_GetItemDef(seq, key, def);
			if likely(result != NULL) {
				if (result != ITER_DONE) {
					result = Dee_function_generator_inlineref(self, result);
					if unlikely(!result)
						goto err;
				}
				DO(Dee_function_generator_vpop(self)); /* seq, key_or_index */
				DO(Dee_function_generator_vpop(self)); /* seq */
				DO(Dee_function_generator_vpop(self)); /* N/A */
				return Dee_function_generator_vpush_const(self, result); /* result */
			} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
				goto err;
			} else {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
		}

		/* Optimizations when typeof(seq) is known */
		if (DeeType_InheritOperator(seq_type, OPERATOR_GETITEM) &&
		    seq_type->tp_seq && seq_type->tp_seq->tp_get) {
			struct type_nsi const *nsi = seq_type->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_getdefault != NULL) {
				DO(Dee_function_generator_vnotoneref(self, 2));                                  /* seq, key_or_index, def */
				DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETITEM, 3)); /* seq, key_or_index, def */
				return Dee_function_generator_vcallapi(self, nsi->nsi_maplike.nsi_getdefault, VCALL_CC_OBJECT, 3); /* result */
			}

			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
#if 0 /* TODO: Inline this */
				result = DeeType_INVOKE_GETITEM(seq_type, self, key);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_KeyError) ||
					    DeeError_Catch(&DeeError_NotImplemented)) {
						if (def != ITER_DONE)
							Dee_Incref(def);
						return def;
					}
				}
				return result;
#endif
			}
		}
	}
	DO(Dee_function_generator_vnotoneref(self, 2));                                  /* seq, key_or_index, def */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETITEM, 3)); /* seq, key_or_index, def */
	return Dee_function_generator_vcallapi(self, &DeeObject_GetItemDef, VCALL_CC_OBJECT, 3);
err:
	return -1;
}

/* seq, key_or_index -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopbounditem(struct Dee_function_generator *__restrict self) {
	DeeTypeObject *seq_type;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	seq_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - 1);
	if (seq_type) {
		if (Dee_function_generator_vallconst(self, 2)) {
			/* TODO: Inline constant expression */
		}
		/* TODO: Optimizations when typeof(seq) is known */
	}
	DO(Dee_function_generator_vnotoneref_at(self, 1));                                  /* seq, index */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_GETITEM, 2));    /* seq, index */
	DO(Dee_function_generator_vpush_imm8(self, 1));                                     /* seq, index, true */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_BoundItem, VCALL_CC_M1INT, 3)); /* result */
	DO(Dee_function_generator_vdirect1(self));                                          /* result */
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}


/* this, [args...]
 * Try to figure out the return type of `operator_name' by looking at doc info.
 * NOTE: This function makes no special case for operators that always return
 *       the same type. Doing this is the responsibility of the caller!
 * @param: argc:  Number of extra arguments (excluding the "this" argument)
 * @return: * :   The guarantied return type
 * @return: NULL: Return type could not be deduced 
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 2)) DeeTypeObject *DCALL
vget_operator_return_type(struct Dee_function_generator *__restrict self,
                          DeeTypeObject *this_type, uint16_t operator_name,
                          Dee_vstackaddr_t argc) {
	char const *operator_doc;
	ASSERT(this_type);

	/* Fast-pass for some common types (so we don't have to keep decoding doc strings) */
	if (this_type == &DeeNone_Type)
		return &DeeNone_Type;
	if (this_type == &DeeInt_Type || this_type == &DeeString_Type ||
	    this_type == &DeeBytes_Type || this_type == &DeeList_Type ||
	    this_type == &DeeTuple_Type || this_type == &DeeDict_Type ||
	    this_type == &DeeHashSet_Type || this_type == &DeeBool_Type) {
		if (DeeType_InheritOperator(this_type, operator_name)) {
			switch (operator_name) {

			case OPERATOR_EQ:
			case OPERATOR_NE:
			case OPERATOR_LO:
			case OPERATOR_LE:
			case OPERATOR_GR:
			case OPERATOR_GE:
			case OPERATOR_CONTAINS:
				return &DeeBool_Type;

			case OPERATOR_SIZE:
				return &DeeInt_Type;

			case OPERATOR_INV:
			case OPERATOR_POS:
			case OPERATOR_NEG:
			case OPERATOR_ADD:
			case OPERATOR_SUB:
			case OPERATOR_MUL:
			case OPERATOR_DIV:
			case OPERATOR_MOD:
			case OPERATOR_SHL:
			case OPERATOR_SHR:
			case OPERATOR_AND:
			case OPERATOR_OR:
			case OPERATOR_XOR:
			case OPERATOR_POW:
			case OPERATOR_INC:
			case OPERATOR_DEC:
			case OPERATOR_INPLACE_ADD:
			case OPERATOR_INPLACE_SUB:
			case OPERATOR_INPLACE_MUL:
			case OPERATOR_INPLACE_DIV:
			case OPERATOR_INPLACE_MOD:
			case OPERATOR_INPLACE_SHL:
			case OPERATOR_INPLACE_SHR:
			case OPERATOR_INPLACE_AND:
			case OPERATOR_INPLACE_OR:
			case OPERATOR_INPLACE_XOR:
			case OPERATOR_INPLACE_POW:
				return this_type;

			case OPERATOR_GETRANGE:
				if (this_type == &DeeString_Type ||
				    this_type == &DeeBytes_Type)
					return this_type;
				break;

			case OPERATOR_GETITEM:
				if (this_type == &DeeString_Type)
					return &DeeString_Type;
				if (this_type == &DeeBytes_Type)
					return &DeeInt_Type;
				break;

			default: break;
			}
		}
	}

	operator_doc = get_operator_doc(self, this_type, operator_name);
	if (!ITER_ISOK(operator_doc))
		return (DeeTypeObject *)operator_doc;

	/* TODO: Decode doc information to search for "operator_name"
	 *       and try to extract the correct return type. */
	(void)self;
	(void)this_type;
	(void)operator_name;
	(void)argc;

/*unknown:*/
	return NULL;
}



/* value -> bool */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopbool(struct Dee_function_generator *__restrict self,
                               unsigned int flags) {
	DeeTypeObject *vtop_type;
	struct Dee_memval *vtop;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	vtop = Dee_function_generator_vtop(self);
	if (Dee_memval_isconst(vtop)) {
		DeeObject *constval;
		if (Dee_memval_const_typeof(vtop) == &DeeBool_Type)
			return 0; /* Already a constant boolean */
		constval = Dee_memval_const_getobj(vtop);
		if (DeeType_IsOperatorConstexpr(Dee_TYPE(constval), OPERATOR_BOOL)) {
			int temp = DeeObject_Bool(constval);
			if unlikely(temp < 0) {
				if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
					goto err;
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			} else {
				Dee_memval_const_setobj_ex(vtop, DeeBool_For(temp), &DeeBool_Type);
				return 0;
			}
		}
	}

	/* Optimizations when types are known */
	vtop_type = Dee_memval_typeof(vtop);
	if (vtop_type == NULL) {
		/* Unknown type... */
	} else if (vtop_type == &DeeBool_Type) {
		/* Special case: object is already a boolean */
		if ((flags & VOPBOOL_F_FORCE_MORPH) && !MEMVAL_VMORPH_ISBOOL(vtop->mv_vmorph)) {
			/* Special case: if we know that the object is an instance of "bool",
			 *               then we can encode the morph-to-bool as:
			 *                   obj != Dee_False
			 *               <=> (obj - Dee_False) != 0 */
			DO(Dee_function_generator_vdirect1(self)); /* bool */
			ASSERT(vtop == Dee_function_generator_vtop(self));
			ASSERT(Dee_memval_isdirect(vtop));
			if (Dee_memval_direct_isref(vtop)) {
				/* Make sure to drop the reference from the bool. Note that this is nokill
				 * since we know that booleans are singletons that can never be destroyed. */
				DO(Dee_function_generator_gdecref_nokill_loc(self, Dee_memval_direct_getloc(vtop), 1));
				Dee_memval_direct_clearref(vtop);
			}
			ASSERT(vtop == Dee_function_generator_vtop(self));
			ASSERT(Dee_memval_isdirect(vtop));
			DO(Dee_function_generator_vdelta(self, -(ptrdiff_t)(uintptr_t)Dee_False));
			vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ; /* (bool - Dee_False) != 0 */
		}
		return 0;
	} else if (DeeType_InheritOperator(vtop_type, OPERATOR_BOOL)) {
		/* Check if there is a dedicated optimization for this operator and type. */
		struct Dee_ccall_optimization const *cco;
		DeeTypeObject *orig_type = DeeType_GetOperatorOrigin(vtop_type, OPERATOR_BOOL);
		cco = Dee_ccall_find_operator_optimization(orig_type, OPERATOR_BOOL, 0);
		if (cco != NULL) {
#ifndef NDEBUG
			Dee_vstackaddr_t old_stackc = self->fg_state->ms_stackc;
#endif /* !NDEBUG */
			int temp = (*cco->tcco_func)(self, 0);
			if (temp <= 0) {
#ifndef NDEBUG
				ASSERT(temp != 0 || (self->fg_state->ms_stackc == old_stackc + 1));
#endif /* !NDEBUG */
				return temp; /* Error */
			}
		}

		/* Invoke a constant operator */
		ASSERT(vtop_type->tp_cast.tp_bool != NULL);
		if (flags & VOPBOOL_F_NOFALLBACK)
			return 1;
		DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_BOOL, 1));
		DO(Dee_function_generator_vcallapi(self, vtop_type->tp_cast.tp_bool, VCALL_CC_NEGINT, 1)); /* result */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_TESTNZ(vtop->mv_vmorph);
		return 0;
	}

	/* Fallback: Make a call to "DeeObject_Bool" */
	if (flags & VOPBOOL_F_NOFALLBACK)
		return 1;
	DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_BOOL, 1));
	DO(Dee_function_generator_vcallapi(self, &DeeObject_Bool, VCALL_CC_NEGINT, 1));
	vtop = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(vtop));
	vtop->mv_vmorph = MEMVAL_VMORPH_TESTNZ(vtop->mv_vmorph);
	return 0;
err:
	return -1;
}

/* value -> !bool */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopnot(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *vtop;
	/* First: cast to some sort of boolean type. */
	if unlikely(Dee_function_generator_vopbool(self, VOPBOOL_F_NORMAL))
		goto err;
again:
	vtop = Dee_function_generator_vtop(self);
	switch (vtop->mv_vmorph) {
	default:
		if (Dee_memval_isconst(vtop)) {
			DeeObject *constval = Dee_memval_const_getobj(vtop);
			if (DeeType_IsOperatorConstexpr(Dee_TYPE(constval), OPERATOR_BOOL)) {
				int temp = DeeObject_Bool(constval);
				if likely(temp >= 0) {
					Dee_memval_const_setobj_ex(vtop, DeeBool_For(!temp), &DeeBool_Type);
					return 0;
				}
				if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
					goto err;
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
		}
		if unlikely(Dee_function_generator_vopbool(self, VOPBOOL_F_FORCE_MORPH))
			goto err;
		goto again;

	case MEMVAL_VMORPH_BOOL_Z:
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
		break;

	case MEMVAL_VMORPH_BOOL_Z_01:
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ_01;
		break;

	case MEMVAL_VMORPH_BOOL_NZ:
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_Z;
		break;

	case MEMVAL_VMORPH_BOOL_NZ_01:
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_Z_01;
		break;

	case MEMVAL_VMORPH_BOOL_LZ:
		/*      !(value < 0)
		 * <=>  value >= 0
		 * <=>  (value + 1) > 0 */
		if unlikely(Dee_function_generator_vdelta(self, 1))
			goto err;
		vtop = Dee_function_generator_vtop(self);
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
		break;

	case MEMVAL_VMORPH_BOOL_GZ:
		/*      !(value > 0)
		 * <=>  value <= 0
		 * <=>  (value - 1) < 0 */
		if unlikely(Dee_function_generator_vdelta(self, -1))
			goto err;
		vtop = Dee_function_generator_vtop(self);
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_LZ;
		break;
	}
	return 0;
err:
	return -1;
}

/* value -> size */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsize(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *vtop;
	DeeTypeObject *return_type = NULL;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	vtop = Dee_function_generator_vtop(self);
	if (Dee_memval_isdirect(vtop)) {
		DeeTypeObject *vtop_type;

		/* Optimization when operands are constant. */
		if (Dee_memval_direct_isconst(vtop)) {
			DeeObject *constval = Dee_memval_const_getobj(vtop);
			if (DeeType_IsOperatorConstexpr(Dee_TYPE(constval), OPERATOR_SIZE)) {
				DREF DeeObject *sizeval = DeeObject_SizeObject(constval);
				if unlikely(!sizeval) {
					if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
						goto err;
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				} else {
					sizeval = Dee_function_generator_inlineref(self, sizeval);
					if unlikely(!sizeval)
						goto err;
					Dee_memval_const_setobj(vtop, sizeval);
					return 0;
				}
			}
		}

		/* Optimizations when types are known */
		vtop_type = Dee_memval_typeof(vtop);
		if (vtop_type != NULL) {
			/* Check if there is a dedicated optimization for this operator and type. */
			struct Dee_ccall_optimization const *cco;
			DeeTypeObject *orig_type = DeeType_GetOperatorOrigin(vtop_type, OPERATOR_SIZE);
			cco = Dee_ccall_find_operator_optimization(orig_type, OPERATOR_SIZE, 0);
			if (cco != NULL) {
#ifndef NDEBUG
				Dee_vstackaddr_t old_stackc = self->fg_state->ms_stackc;
#endif /* !NDEBUG */
				int temp = (*cco->tcco_func)(self, 0);
				if (temp <= 0) {
#ifndef NDEBUG
					ASSERT(temp != 0 || (self->fg_state->ms_stackc == old_stackc + 1));
#endif /* !NDEBUG */
					return temp; /* Error */
				}
			}

			/* Try to determine the operator's runtime return type. */
			return_type = vget_operator_return_type(self, vtop_type, OPERATOR_SIZE, 0);
			if unlikely(return_type == (DeeTypeObject *)ITER_DONE)
				goto err;

			/* See if we can prematurely load the type's size operator to inline it. */
			if (DeeType_InheritOperator(vtop_type, OPERATOR_SIZE)) {
				ASSERT(vtop_type->tp_seq != NULL);
				ASSERT(vtop_type->tp_seq->tp_size != NULL);

				/* TODO: Check if there is an NSI size operator. */

				/* Invoke the inlined operator */
				DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_SIZE, 1));
				DO(Dee_function_generator_vcallapi(self, vtop_type->tp_seq->tp_size, VCALL_CC_OBJECT, 1)); /* result */
				goto do_set_return_type;
			}
		}
	}

	/* Fallback: emit a dynamic call. */
	DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_SIZE, 1));
	DO(Dee_function_generator_vcallapi(self, &DeeObject_SizeObject, VCALL_CC_OBJECT, 1));
do_set_return_type:
	if (return_type)
		return Dee_function_generator_vsettyp_noalias(self, return_type);
	return 0;
err:
	return -1;
}

/* value -> int */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopint(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *vtop;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	vtop = Dee_function_generator_vtop(self);
	if (Dee_memval_isdirect(vtop)) {
		DeeTypeObject *vtop_type;
		/* Optimizations when types are known */
		if (Dee_memval_direct_isconst(vtop)) {
			DeeObject *constval;
			if (Dee_memval_const_typeof(vtop) == &DeeString_Type)
				return 0; /* Already a constant integer */
			constval = Dee_memval_const_getobj(vtop);
			if (DeeType_IsOperatorConstexpr(Dee_TYPE(constval), OPERATOR_INT)) {
				DREF DeeObject *intval = DeeObject_Int(constval);
				if unlikely(!intval) {
					if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
						goto err;
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				} else {
					intval = Dee_function_generator_inlineref(self, intval);
					if unlikely(!intval)
						goto err;
					Dee_memval_const_setobj_ex(vtop, intval, &DeeInt_Type);
					return 0;
				}
			}
		}
		vtop_type = Dee_memval_typeof(vtop);
		if (vtop_type == NULL) {
			/* Unknown type... */
		} else if (vtop_type == &DeeInt_Type) {
			return 0;
		} else {
			/* Check if there is a dedicated optimization for this operator and type. */
			struct Dee_ccall_optimization const *cco;
			DeeTypeObject *orig_type = DeeType_GetOperatorOrigin(vtop_type, OPERATOR_INT);
			cco = Dee_ccall_find_operator_optimization(orig_type, OPERATOR_INT, 0);
			if (cco != NULL) {
#ifndef NDEBUG
				Dee_vstackaddr_t old_stackc = self->fg_state->ms_stackc;
#endif /* !NDEBUG */
				int temp = (*cco->tcco_func)(self, 0);
				if (temp <= 0) {
#ifndef NDEBUG
					ASSERT(temp != 0 || (self->fg_state->ms_stackc == old_stackc + 1));
#endif /* !NDEBUG */
					return temp; /* Error */
				}
			}

			/* See if we can prematurely load the type's int operator to inline it. */
			if (DeeType_InheritOperator(vtop_type, OPERATOR_INT)) {
				ASSERT(vtop_type->tp_math != NULL);
				if (vtop_type->tp_math->tp_int != NULL) {
					DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_INT, 1));
					return Dee_function_generator_vcallapi(self, vtop_type->tp_math->tp_int,
					                                       VCALL_CC_OBJECT, 1); /* result */
				}
				/* Invoking tp_int32 / tp_int64 inline would be to complicated (because the
				 * signed-ness of the returned value would only be known at runtime, but the
				 * integer morph needs to know it at compile-time) */
			}
		}
	}

	/* Fallback: emit a dynamic call. */
	DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_INT, 1));
	DO(Dee_function_generator_vcallapi(self, &DeeObject_Int, VCALL_CC_OBJECT, 1));
	return Dee_function_generator_vsettyp_noalias(self, &DeeInt_Type);
err:
	return -1;
}


/* value -> string */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopstr(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *vtop;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	vtop = Dee_function_generator_vtop(self);
	if (Dee_memval_isdirect(vtop)) {
		DeeTypeObject *vtop_type;
		/* Optimizations when types are known */
		if (Dee_memval_direct_isconst(vtop)) {
			DeeObject *constval;
			if (Dee_memval_const_typeof(vtop) == &DeeString_Type)
				return 0; /* Already a constant string */
			constval = Dee_memval_const_getobj(vtop);
			if (DeeType_IsOperatorConstexpr(Dee_TYPE(constval), OPERATOR_STR)) {
				DREF DeeObject *strval = DeeObject_Str(constval);
				if unlikely(!strval) {
					if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
						goto err;
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				} else {
					strval = Dee_function_generator_inlineref(self, strval);
					if unlikely(!strval)
						goto err;
					Dee_memval_const_setobj_ex(vtop, strval, &DeeString_Type);
					return 0;
				}
			}
		}
		vtop_type = Dee_memval_typeof(vtop);
		if (vtop_type == NULL) {
			/* Unknown type... */
		} else if (vtop_type == &DeeString_Type) {
			return 0; /* When the type is already a string, the call becomes a no-op. */
		} else if (DeeType_InheritOperator(vtop_type, OPERATOR_STR)) {
			/* Check if there is a dedicated optimization for this operator and type. */
			struct Dee_ccall_optimization const *cco;
			DeeTypeObject *orig_type = DeeType_GetOperatorOrigin(vtop_type, OPERATOR_STR);
			cco = Dee_ccall_find_operator_optimization(orig_type, OPERATOR_STR, 0);
			if (cco != NULL) {
#ifndef NDEBUG
				Dee_vstackaddr_t old_stackc = self->fg_state->ms_stackc;
#endif /* !NDEBUG */
				int temp = (*cco->tcco_func)(self, 0);
				if (temp <= 0) {
#ifndef NDEBUG
					ASSERT(temp != 0 || (self->fg_state->ms_stackc == old_stackc + 1));
#endif /* !NDEBUG */
					return temp; /* Error */
				}
			}

			/* See if we can prematurely load the type's str operator to inline it. */
			ASSERT(vtop_type->tp_cast.tp_str);
			ASSERT(vtop_type->tp_cast.tp_print);
			DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_STR, 1));
			DO(Dee_function_generator_vcallapi(self, vtop_type->tp_cast.tp_str, VCALL_CC_OBJECT, 1)); /* result */
			goto set_return_type;
		}
	}

	/* Fallback: emit a dynamic call. */
	DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_STR, 1));
	DO(Dee_function_generator_vcallapi(self, &DeeObject_Str, VCALL_CC_OBJECT, 1));
set_return_type:
	return Dee_function_generator_vsettyp_noalias(self, &DeeString_Type);
err:
	return -1;
}

/* Helpers to implement "is" */

/* this, type -> bool
 *
 * Generates code:
 * >> bool result;
 * >> if (DeeNone_Check(type)) {
 * >>     result = DeeNone_Check(this);
 * >> } else {
 * >>     DeeTypeObject *tp_this = Dee_TYPE(this);
 * >>     if (tp_this == &DeeSuper_Type)
 * >>         tp_this = DeeSuper_TYPE(this);
 * >>     result = do_implements ? DeeType_Implements(tp_this, type)
 * >>                            : DeeType_InheritsFrom(tp_this, type);
 * >> }
 * >> PUSH(result);
 */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vinstanceof(struct Dee_function_generator *__restrict self,
                 bool do_implements) {
	struct Dee_memval *thisval, *typeval;
	DeeTypeObject *type_type;
	DeeTypeObject *this_type;
	DeeTypeObject *logical_this_type;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	typeval = Dee_function_generator_vtop(self);
	thisval = typeval - 1;
	this_type = Dee_memval_typeof(thisval);
	type_type = Dee_memval_typeof(typeval);
	logical_this_type = this_type;
	if (logical_this_type == &DeeSuper_Type) {
		logical_this_type = NULL;
		if (Dee_memval_isconst(thisval))
			logical_this_type = DeeSuper_TYPE(Dee_memval_const_getobj(thisval));
	}
	if (type_type != NULL) {
		if (type_type == &DeeNone_Type) {
			/* Special case: `this is none' */
			DO(Dee_function_generator_vpop(self)); /* this */
			return Dee_function_generator_veqconstaddr(self, Dee_None);
		} else if (!DeeType_InheritsFrom(type_type, &DeeType_Type)) {
			/* When the "type" argument isn't "none", and also isn't a type,
			 * then the `DeeType_InheritsFrom()' would always return "false". */
			DO(Dee_function_generator_vpop(self)); /* this */
			DO(Dee_function_generator_vpop(self)); /* N/A */
			return Dee_function_generator_vpush_const(self, Dee_False);
		} else if (Dee_memval_isconst(typeval)) {
			DeeTypeObject *type = (DeeTypeObject *)Dee_memval_const_getobj(typeval);
			ASSERT_OBJECT_TYPE(type, type_type);
			if (logical_this_type != NULL) {
				/* Special case: the result is a constant expression. */
				bool result = do_implements ? DeeType_Implements(logical_this_type, type)
				                            : DeeType_InheritsFrom(logical_this_type, type);
				DO(Dee_function_generator_vpop(self)); /* this */
				DO(Dee_function_generator_vpop(self)); /* N/A */
				return Dee_function_generator_vpush_const(self, DeeBool_For(result));
			}

			/* Check for special case: if "type" is final, then we can check `Dee_TYPE(this) === type' */
			if (DeeType_IsFinal(type)) {
				DO(Dee_function_generator_vpop(self));                               /* this */
				DO(Dee_function_generator_vdup(self));                               /* this, this */
				DO(Dee_function_generator_vind(self, offsetof(DeeObject, ob_type))); /* this, Dee_TYPE(this) */
				DO(Dee_function_generator_vpop_at(self, 2));                         /* Dee_TYPE(this) */
				return Dee_function_generator_veqconstaddr(self, (DeeObject *)type); /* result */
			}

			/* Check for special case: if "type" isn't abstract, then we won't have to look at MRO */
			if (!DeeType_IsAbstract(type))
				do_implements = false;
		}
	}

	/* TODO */
	(void)self;
	(void)do_implements;
	return DeeError_NOTIMPLEMENTED();
err:
	return -1;
}


/* this, type -> bool */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopinstanceof(struct Dee_function_generator *__restrict self) {
	return impl_vinstanceof(self, false);
}

/* this, type -> bool */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopimplements(struct Dee_function_generator *__restrict self) {
	return impl_vinstanceof(self, true);
}


#define _LOCAL_DeeObject_ASINT_1      DeeObject_AsInt8
#define _LOCAL_DeeObject_ASINT_2      DeeObject_AsInt16
#define _LOCAL_DeeObject_ASINT_4      DeeObject_AsInt32
#define _LOCAL_DeeObject_ASINT_8      DeeObject_AsInt64
#define _LOCAL_DeeObject_ASINT_16     DeeObject_AsInt128
#define _LOCAL_DeeObject_ASUINT_1     DeeObject_AsUInt8
#define _LOCAL_DeeObject_ASUINT_2     DeeObject_AsUInt16
#define _LOCAL_DeeObject_ASUINT_4     DeeObject_AsUInt32
#define _LOCAL_DeeObject_ASUINT_8     DeeObject_AsUInt64
#define _LOCAL_DeeObject_ASUINT_16    DeeObject_AsUInt128
#define _LOCAL_DeeObject_ASINT(size)  _LOCAL_DeeObject_ASINT_##size
#define _LOCAL_DeeObject_ASUINT(size) _LOCAL_DeeObject_ASUINT_##size
#define LOCAL_DeeObject_AsTINT(size)  _LOCAL_DeeObject_ASINT(size)
#define LOCAL_DeeObject_AsTUINT(size) _LOCAL_DeeObject_ASUINT(size)

/* Helpers to evaluate an object into a C integer */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vmorph_int(struct Dee_function_generator *__restrict self, uint8_t wanted_morph) {
	void const *api_function;
	struct Dee_memval *vtop;
	Dee_cfa_t result_cfa;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = Dee_function_generator_vtop(self);
	if (vtop->mv_vmorph == wanted_morph) {
		vtop->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		return 0;
	}
	DO(Dee_function_generator_state_unshare(self));
	DO(Dee_function_generator_vdirect1(self));
	vtop = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(vtop));
	api_function = wanted_morph == MEMVAL_VMORPH_UINT
	               ? (void const *)&LOCAL_DeeObject_AsTUINT(HOST_SIZEOF_POINTER)
	               : (void const *)&LOCAL_DeeObject_AsTINT(HOST_SIZEOF_POINTER);
	if (Dee_memval_direct_isconst(vtop)) {
		/* Special handling to inline a constant. */
		int temp;
		uintptr_t value;
		temp = (*(int (DCALL *)(DeeObject *, uintptr_t *))api_function)(Dee_memval_const_getobj(vtop), &value);
		if likely(temp == 0) {
			Dee_memval_const_setaddr(vtop, (void *)value);
			return 0;
		} else if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			goto err;
		}
	}
	result_cfa = Dee_memstate_hstack_find(self->fg_state, self->fg_state_hstack_res, HOST_SIZEOF_POINTER);
	if (result_cfa == (Dee_cfa_t)-1) {
		result_cfa = Dee_memstate_hstack_alloca(self->fg_state, HOST_SIZEOF_POINTER);
		DO(Dee_function_generator_ghstack_adjust(self, HOST_SIZEOF_POINTER));
	}
	DO(Dee_function_generator_vpush_hstackind(self, result_cfa, 0)); /* obj, result */
	DO(Dee_function_generator_vswap(self));                          /* result, obj */
	DO(Dee_function_generator_vpush_hstack(self, result_cfa));       /* result, obj, p_value */
	return Dee_function_generator_vcallapi(self, api_function, VCALL_CC_INT, 2); /* result */
err:
	return -1;
}

/* value -> DeeObject_AsIntptr(value) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vmorph_int(struct Dee_function_generator *__restrict self) {
	return vmorph_int(self, MEMVAL_VMORPH_INT);
}

/* value -> DeeObject_AsUIntptr(value) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vmorph_uint(struct Dee_function_generator *__restrict self) {
	return vmorph_int(self, MEMVAL_VMORPH_UINT);
}

/* Helpers for wrapping values as simple object morphs */

/* value -> ...(value) */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_vmakemorph(struct Dee_function_generator *__restrict self,
                                   uint8_t morph) {
	int result = Dee_function_generator_vdirect1(self);
	if likely(result == 0)
		Dee_function_generator_vtop(self)->mv_vmorph = morph;
	return result;
}


/* [elems...] -> seq
 * NOTE: [elems...] are all DIRECT values. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpack_map_or_set_at_runtime(struct Dee_function_generator *__restrict self,
                            DeeTypeObject *__restrict seq_type, Dee_vstackaddr_t elemc) {
	/* Initialize a known built-in mapping/set type from a set of keys[/values] stored on the v-stack.
	 * NOTE: items where the key is a constant (and has a constexpr OPERATOR_HASH) can
	 *       have their hash values be calculated at compile-time and filled in as-is.
	 *
	 * Example:
	 * >> local x = Dict({ "a": x, foo: bar, "c": y });
	 *
	 * Compile as:
	 * >> DeeDictObject *d = DeeType_AllocInstance(&DeeDict_Type);
	 * >> if (!d) HANDLE_EXCEPT();
	 * >> d->d_elem = Dee_Callocc(REQ_MASK, sizeof(struct Dee_dict_item));
	 * >> if (!d->d_elem) { DeeType_FreeInstance(d); HANDLE_EXCEPT(); }
	 * >> d->d_mask = REQ_MASK;
	 * >> d->d_size = NUMBER_OF_DISTINCT_CONSTANT_KEYS;
	 * >> d->d_used = NUMBER_OF_DISTINCT_CONSTANT_KEYS;
	 * >> d->d_elem[HASHOF_CONSTANT_key1 & REQ_MASK].di_key   = CONSTANT_key1;       // inherit reference
	 * >> d->d_elem[HASHOF_CONSTANT_key1 & REQ_MASK].di_value = CONSTANT_key1_value; // inherit reference
	 * >> d->d_elem[HASHOF_CONSTANT_key1 & REQ_MASK].di_hash  = HASHOF_CONSTANT_key1;
	 * >> d->d_elem[HASHOF_CONSTANT_key2 & REQ_MASK].di_key   = CONSTANT_key2;       // inherit reference
	 * >> d->d_elem[HASHOF_CONSTANT_key2 & REQ_MASK].di_value = CONSTANT_key2_value; // inherit reference
	 * >> d->d_elem[HASHOF_CONSTANT_key2 & REQ_MASK].di_hash  = HASHOF_CONSTANT_key2;
	 * >> [...] // Repeat for all constant keys
	 * >>
	 * >> DeeDictObject *chain_d = d;
	 * >> chain_d = libhostasm_rt_DeeDict_InsertFast(chain_d, NON_CONSTANT_key1, NON_CONSTANT_key1_value);
	 * >> if (!chain_d) HANDLE_EXCEPT();
	 * >> chain_d = libhostasm_rt_DeeDict_InsertFast(chain_d, NON_CONSTANT_key2, NON_CONSTANT_key2_value);
	 * >> if (!chain_d) HANDLE_EXCEPT();
	 * >> [...] // Repeat for all non-constant keys
	 * >>
	 * >> d->d_lock = DEE_ATOMIC_RWLOCK_INIT;
	 * >> d->ob_weakrefs = Dee_WEAKREF_SUPPORT_INIT;
	 * >> DeeObject_Init(d, &DeeDict_Type);
	 * >> RESULT = DeeGC_Track(d);
	 */
	struct hashmap_template_item {
		DeeObject       *ti_key;   /* [0..1] Compile-time constant key (or NULL if this is a dummy item) */
		Dee_hash_t       ti_hash;  /* [valid_if(ti_key)] The hash for "ti_key" (in the form of a snapshot) */
		Dee_vstackaddr_t ti_value; /* [valid_if(:asmap)] V-stack address of the value linked to "ti_key" */
	};
	struct seq_object_info {
		size_t      soi_sizeof_DeeSeqObject;                 /* sizeof(DeeDictObject)  (set to 0 if TP_FVARIABLE) */
		size_t      soi_offsetof_d_mask;                     /* offsetof(DeeDictObject, d_mask) */
		size_t      soi_offsetof_d_size;                     /* offsetof(DeeDictObject, d_size) */
		size_t      soi_offsetof_d_used;                     /* offsetof(DeeDictObject, d_used) (set to 0 for read-only types) */
		size_t      soi_offsetof_d_elem;                     /* offsetof(DeeDictObject, d_elem) */
		size_t      soi_sizeof_dict_item;                    /* sizeof(struct Dee_dict_item) */
		size_t      soi_offsetof_dict_item__di_key;          /* offsetof(struct Dee_dict_item, di_key) */
		size_t      soi_offsetof_dict_item__di_hash;         /* offsetof(struct Dee_dict_item, di_hash) */
		size_t      soi_offsetof_dict_item__di_value;        /* offsetof(struct Dee_dict_item, di_value)  (set to 0 for set types) */
		void const *soi_libhostasm_rt_DeeSeqType_InsertFast; /* &libhostasm_rt_DeeDict_InsertFast */
	};
	struct seq_object_info const *soi;
	struct hashmap_template_item *result_d_elem_template;
	struct Dee_memval *elemv;
	bool asmap = (seq_type == &DeeDict_Type || seq_type == &DeeRoDict_Type);
	bool is_ro = (seq_type->tp_flags & TP_FVARIABLE) != 0;
	bool use_calloc_for_d_elem;
	size_t result_d_size, result_d_maxsize;
	size_t result_d_mask, result_d_minmask;
	Dee_vstackaddr_t i;

	ASSERT(!!is_ro == !!(seq_type == &DeeRoDict_Type || seq_type == &DeeRoSet_Type));
	if (seq_type == &DeeDict_Type) {
		PRIVATE struct seq_object_info tpconst dict_soi = {
			/* .soi_sizeof_DeeSeqObject                 = */ sizeof(DeeDictObject),
			/* .soi_offsetof_d_mask                     = */ offsetof(DeeDictObject, d_mask),
			/* .soi_offsetof_d_size                     = */ offsetof(DeeDictObject, d_size),
			/* .soi_offsetof_d_used                     = */ offsetof(DeeDictObject, d_used),
			/* .soi_offsetof_d_elem                     = */ offsetof(DeeDictObject, d_elem),
			/* .soi_sizeof_dict_item                    = */ sizeof(struct Dee_dict_item),
			/* .soi_offsetof_dict_item__di_key          = */ offsetof(struct Dee_dict_item, di_key),
			/* .soi_offsetof_dict_item__di_hash         = */ offsetof(struct Dee_dict_item, di_hash),
			/* .soi_offsetof_dict_item__di_value        = */ offsetof(struct Dee_dict_item, di_value),
			/* .soi_libhostasm_rt_DeeSeqType_InsertFast = */ (void const *)&libhostasm_rt_DeeDict_InsertFast
		};
		soi = &dict_soi;
	} else if (seq_type == &DeeRoDict_Type) {
		PRIVATE struct seq_object_info tpconst rodict_soi = {
			/* .soi_sizeof_DeeSeqObject                 = */ 0,
			/* .soi_offsetof_d_mask                     = */ offsetof(DeeRoDictObject, rd_mask),
			/* .soi_offsetof_d_size                     = */ offsetof(DeeRoDictObject, rd_size),
			/* .soi_offsetof_d_used                     = */ 0,
			/* .soi_offsetof_d_elem                     = */ offsetof(DeeRoDictObject, rd_elem),
			/* .soi_sizeof_dict_item                    = */ sizeof(struct Dee_rodict_item),
			/* .soi_offsetof_dict_item__di_key          = */ offsetof(struct Dee_rodict_item, rdi_key),
			/* .soi_offsetof_dict_item__di_hash         = */ offsetof(struct Dee_rodict_item, rdi_hash),
			/* .soi_offsetof_dict_item__di_value        = */ offsetof(struct Dee_rodict_item, rdi_value),
			/* .soi_libhostasm_rt_DeeSeqType_InsertFast = */ (void const *)&libhostasm_rt_DeeRoDict_InsertFast
		};
		soi = &rodict_soi;
	} else if (seq_type == &DeeHashSet_Type) {
		PRIVATE struct seq_object_info tpconst hashset_soi = {
			/* .soi_sizeof_DeeSeqObject                 = */ sizeof(DeeHashSetObject),
			/* .soi_offsetof_d_mask                     = */ offsetof(DeeHashSetObject, hs_mask),
			/* .soi_offsetof_d_size                     = */ offsetof(DeeHashSetObject, hs_size),
			/* .soi_offsetof_d_used                     = */ offsetof(DeeHashSetObject, hs_used),
			/* .soi_offsetof_d_elem                     = */ offsetof(DeeHashSetObject, hs_elem),
			/* .soi_sizeof_dict_item                    = */ sizeof(struct Dee_hashset_item),
			/* .soi_offsetof_dict_item__di_key          = */ offsetof(struct Dee_hashset_item, hsi_key),
			/* .soi_offsetof_dict_item__di_hash         = */ offsetof(struct Dee_hashset_item, hsi_hash),
			/* .soi_offsetof_dict_item__di_value        = */ 0,
			/* .soi_libhostasm_rt_DeeSeqType_InsertFast = */ (void const *)&libhostasm_rt_DeeHashSet_InsertFast
		};
		soi = &hashset_soi;
	} else {
		PRIVATE struct seq_object_info tpconst roset_soi = {
			/* .soi_sizeof_DeeSeqObject                 = */ 0,
			/* .soi_offsetof_d_mask                     = */ offsetof(DeeRoSetObject, rs_mask),
			/* .soi_offsetof_d_size                     = */ offsetof(DeeRoSetObject, rs_size),
			/* .soi_offsetof_d_used                     = */ 0,
			/* .soi_offsetof_d_elem                     = */ offsetof(DeeRoSetObject, rs_elem),
			/* .soi_sizeof_dict_item                    = */ sizeof(struct Dee_roset_item),
			/* .soi_offsetof_dict_item__di_key          = */ offsetof(struct Dee_roset_item, rsi_key),
			/* .soi_offsetof_dict_item__di_hash         = */ offsetof(struct Dee_roset_item, rsi_hash),
			/* .soi_offsetof_dict_item__di_value        = */ 0,
			/* .soi_libhostasm_rt_DeeSeqType_InsertFast = */ (void const *)&libhostasm_rt_DeeRoSet_InsertFast
		};
		ASSERT(seq_type == &DeeRoSet_Type);
		soi = &roset_soi;
	}

again:
	use_calloc_for_d_elem = true;
	result_d_maxsize = asmap ? (size_t)(elemc / 2) : (size_t)elemc;
	result_d_minmask = 1;
	while ((result_d_maxsize & result_d_minmask) != result_d_maxsize)
		result_d_minmask = (result_d_minmask << 1) | 1;
	result_d_mask = result_d_minmask;
	if (!is_ro) {
		if (result_d_mask < 16 - 1)
			result_d_mask = 16 - 1;
		/* Prefer using a mask of one greater level to improve performance. */
		result_d_mask = (result_d_mask << 1) | 1;
	}

	/* Figure out the template for how the initial result map should look like. */
	result_d_elem_template = (struct hashmap_template_item *)Dee_Callocac(result_d_mask + 1,
	                                                                      sizeof(struct hashmap_template_item));
	if unlikely(!result_d_elem_template)
		goto err_no_result_d_elem_template;
	result_d_size = 0;
	elemv = self->fg_state->ms_stackv + self->fg_state->ms_stackc - elemc;
	for (i = 0; i < elemc;) {
		struct Dee_memval *keyval = &elemv[i];
		if (Dee_memval_direct_isconst(keyval)) {
			struct hashmap_template_item *it;
			Dee_vstackaddr_t key_voffset;
			Dee_hash_t template_i, perturb, hash;
			DeeObject *key = Dee_memval_const_getobj(keyval);
			if (!DeeType_IsOperatorConstexpr(Dee_TYPE(key), OPERATOR_HASH))
				goto next_key;
			if (!DeeType_IsOperatorConstexpr(Dee_TYPE(key), OPERATOR_EQ))
				goto next_key;
			hash = DeeObject_Hash(key);
			key_voffset = (Dee_vstackaddr_t)((self->fg_state->ms_stackv +
			                                  self->fg_state->ms_stackc) -
			                                 keyval);
			perturb = template_i = hash & result_d_mask;
			for (;; seq_type == &DeeDict_Type
			        ? DeeDict_HashNx(template_i, perturb)
			        : seq_type == &DeeHashSet_Type
			          ? DeeHashSet_HashNx(template_i, perturb)
			          : seq_type == &DeeRoDict_Type
			            ? DeeRoDict_HashNx(template_i, perturb)
			            : DeeRoSet_HashNx(template_i, perturb)) {
				it = &result_d_elem_template[template_i & result_d_mask];
				if (!it->ti_key)
					break;

				if (it->ti_hash == hash) {
					/* Check if this is a duplicate key. */
					int temp = DeeObject_CompareEq(it->ti_key, key);
					if unlikely(temp < 0) {
						if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
							goto err;
						DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
						goto next_key;
					} else if (temp) {
						/* Duplicate key (just get rid of this second instance by removing it from the v-stack) */
						if (asmap) {
							ASSERT(key_voffset >= 2);
							DO(Dee_function_generator_vpop_at(self, key_voffset - 1));
							--key_voffset;
							--elemc;
						}
						DO(Dee_function_generator_vpop_at(self, key_voffset));
						Dee_Freea(result_d_elem_template);
						--elemc;
						goto again;
					}
				}
			}

			/* Remember where this key goes in the template. */
			it->ti_key   = key;
			it->ti_hash  = hash;
			it->ti_value = (Dee_vstackaddr_t)(keyval - self->fg_state->ms_stackv);
			++result_d_size;
			ASSERT(result_d_size < result_d_mask);

			/* Pop the key from the v-stack (but keep the value in case this is a map). */
			DO(Dee_function_generator_vpop_at(self, key_voffset));
			--elemc;
		} else {
next_key:
			++i;
		}
		if (asmap)
			++i;
	}

	/* HINT: At this point, the vstack looks like this:
	 * >> CONSTANT_key1_value, CONSTANT_key2_value, [...], NON_CONSTANT_key1, NON_CONSTANT_key1_value, [...] */

	/* If less than 1/3 of elements in the template are NULL-entries, then don't use CALLOC
	 * when allocating the d_elem vector, but MALLOC, and generate code to NULL out the keys
	 * of unused template elements.
	 * XXX: Is that really a good cut-off? calloc isn't generally that much more expensive
	 *      than malloc, but I could be wrong... */
	{
		size_t num_items = (result_d_mask + 1);
		size_t num_null_items;
		ASSERT(num_items > result_d_size); /* > because there always needs to be 1 extra NULL item */
		num_null_items = num_items - result_d_size;
		if ((num_null_items * 3) <= result_d_size)
			use_calloc_for_d_elem = false;
	}

	/* Generate code to allocate an instance of the dict/set. */
	if (is_ro) {
		size_t sz = soi->soi_offsetof_d_elem + ((result_d_mask + 1) * soi->soi_sizeof_dict_item);
		ASSERT(seq_type == &DeeRoDict_Type || seq_type == &DeeRoSet_Type);
		ASSERT(!(seq_type->tp_flags & TP_FGC));
		DO(Dee_function_generator_vcall_DeeObject_Malloc(self, sz, use_calloc_for_d_elem)); /* [elems...], d */
		DO(Dee_function_generator_vdup(self));                                              /* [elems...], d, d */
		DO(Dee_function_generator_vdelta(self, soi->soi_offsetof_d_elem));                  /* [elems...], d, d->d_elem */
	} else {
		struct Dee_function_exceptinject_freeinstance ij;
		size_t sz = (result_d_mask + 1) * soi->soi_sizeof_dict_item;
		ASSERT(seq_type == &DeeDict_Type || seq_type == &DeeHashSet_Type);

		/* Allocate the hash-vector for the dict/hashset */
		DO(vcall_DeeType_AllocInstance(self, seq_type));   /* [elems...], d */
		Dee_function_generator_xinject_push_freeinstance(self, &ij, seq_type);
		DO(Dee_function_generator_vpush_immSIZ(self, sz)); /* [elems...], d, sz */
		DO(Dee_function_generator_vcallapi(self,           /* [elems...], d, UNCHECKED(d_elem) */
		                                   use_calloc_for_d_elem ? &Dee_Calloc
		                                                         : &Dee_Malloc,
		                                   VCALL_CC_OBJECT, 1));
		DO(Dee_function_generator_vdirect1(self));                          /* [elems...], d, NOT_ASSIGNED(d_elem) */
		Dee_function_generator_xinject_pop_freeinstance(self, &ij);
		ASSERT(Dee_function_generator_vtop_direct_isref(self));
		Dee_function_generator_vtop_direct_clearref(self);                  /* [elems...], d, NOT_ASSIGNED(d_elem) */
		DO(Dee_function_generator_vswap(self));                             /* [elems...], NOT_ASSIGNED(d_elem), d */
		DO(Dee_function_generator_vdup_n(self, 2));                         /* [elems...], NOT_ASSIGNED(d_elem), d, NOT_ASSIGNED(d_elem) */
		DO(Dee_function_generator_vpopind(self, soi->soi_offsetof_d_elem)); /* [elems...], d_elem, d */
		DO(Dee_function_generator_vswap(self));                             /* [elems...], d, d_elem */
	}                                                                       /* [elems...], d, d_elem */

	/* Initialize fields of the dict:
	 * - d_mask = result_d_mask
	 * - d_size = result_d_size
	 * - d_used = result_d_size
	 * These fields must be initialized before we fill in non-constant keys,
	 * as `soi_libhostasm_rt_DeeSeqType_InsertFast()' relies on these fields
	 * already being initialized. */
	DO(Dee_function_generator_vswap(self));                             /* [elems...], d_elem, d */
	DO(Dee_function_generator_vpush_immSIZ(self, result_d_mask));       /* [elems...], d_elem, d, result_d_mask */
	DO(Dee_function_generator_vpopind(self, soi->soi_offsetof_d_mask)); /* [elems...], d_elem, d */
	DO(Dee_function_generator_vpush_immSIZ(self, result_d_size));       /* [elems...], d_elem, d, result_d_size */
	DO(Dee_function_generator_vpopind(self, soi->soi_offsetof_d_size)); /* [elems...], d_elem, d */
	if (soi->soi_offsetof_d_used) {
		DO(Dee_function_generator_vpush_immSIZ(self, result_d_size));       /* [elems...], d_elem, d, result_d_size */
		DO(Dee_function_generator_vpopind(self, soi->soi_offsetof_d_used)); /* [elems...], d_elem, d */
	}
	DO(Dee_function_generator_vswap(self));                             /* [elems...], d, d_elem */

	/* Generate code to populate "d_elem" from the template. */
	{
		size_t template_i;
		for (template_i = 0; template_i <= result_d_mask; ++template_i) {
			struct hashmap_template_item *it = &result_d_elem_template[template_i];
			size_t offsetof_it;
			if (it->ti_key == NULL && use_calloc_for_d_elem)
				continue; /* Already correctly initialized. */
			offsetof_it = soi->soi_sizeof_dict_item * (size_t)(it - result_d_elem_template);
			DO(Dee_function_generator_vpush_addr(self, it->ti_key));                                      /* [elems...], d, d_elem, key */
			DO(Dee_function_generator_vref2(self, 1));                                                    /* [elems...], d, d_elem, ref:key */
			DO(Dee_function_generator_vpopind(self, offsetof_it + soi->soi_offsetof_dict_item__di_key));  /* [elems...], d, d_elem */
			if (it->ti_key == NULL)
				continue; /* Filling in the key is enough. */
			if (asmap) {
				size_t template_j;
				Dee_vstackaddr_t it_value_offset;
				/* Rotate the value for the key into VTOP */
				it_value_offset = self->fg_state->ms_stackc - it->ti_value;
				DO(Dee_function_generator_vlrot(self, it_value_offset));                                       /* [elems...], d, d_elem, value */
				DO(Dee_function_generator_vref2(self, 1));                                                     /* [elems...], d, d_elem, ref:value */
				DO(Dee_function_generator_vpopind(self, offsetof_it + soi->soi_offsetof_dict_item__di_value)); /* [elems...], d, d_elem */
				--elemc;
				/* Adjust VSTACK addresses of template items that still need to be written to the dict. */
				for (template_j = template_i + 1; template_j <= result_d_mask; ++template_j) {
					struct hashmap_template_item *it_j = &result_d_elem_template[template_j];
					if (it_j->ti_key && it_j->ti_value >= it->ti_value)
						--it_j->ti_value;
				}
			}
			DO(Dee_function_generator_vpush_immSIZ(self, it->ti_hash));                                   /* [elems...], d, d_elem, hash */
			DO(Dee_function_generator_vpopind(self, offsetof_it + soi->soi_offsetof_dict_item__di_hash)); /* [elems...], d, d_elem */
		}
	}

	/* HINT: At this point, the vstack looks like this (with "elemc" non-constant key[/value-pair]s):
	 * >> NON_CONSTANT_key1, NON_CONSTANT_key1_value, [...], d, d_elem  */

	/* Generate code to insert all of the non-constant key[/value-pair]s */
	ASSERT(!asmap || (elemc % 2) == 0);    /* [elems...], d, d_elem */
	DO(Dee_function_generator_vpop(self)); /* [elems...], d */
	ASSERT(Dee_function_generator_vtop_direct_isref(self));
	Dee_function_generator_vtop_direct_clearref(self); /* The "reference" is always inherited by `soi_libhostasm_rt_DeeSeqType_InsertFast' */
	while (elemc) {
		DO(Dee_function_generator_vlrot(self, elemc + 1));     /* [elems...], d, key */
		if (asmap) {
			DO(Dee_function_generator_vlrot(self, elemc + 1)); /* [elems...], d, key, value */
			DO(Dee_function_generator_vref2(self, 2));         /* [elems...], d, key, ref:value */
			DO(Dee_function_generator_vswap(self));            /* [elems...], d, ref:value, key */
			DO(Dee_function_generator_vref2(self, 2));         /* [elems...], d, ref:value, ref:key */
			DO(Dee_function_generator_vswap(self));            /* [elems...], d, ref:key, ref:value */
			ASSERT(Dee_memval_direct_isref(&Dee_function_generator_vtop(self)[-1]));
			ASSERT(Dee_memval_direct_isref(&Dee_function_generator_vtop(self)[-0]));
			Dee_memval_direct_clearref(&Dee_function_generator_vtop(self)[-1]); /* Always stolen by `soi_libhostasm_rt_DeeSeqType_InsertFast' */
			Dee_memval_direct_clearref(&Dee_function_generator_vtop(self)[-0]); /* Always stolen by `soi_libhostasm_rt_DeeSeqType_InsertFast' */
			DO(Dee_function_generator_vcallapi(self, soi->soi_libhostasm_rt_DeeSeqType_InsertFast,
			                                   VCALL_CC_RAWINTPTR, 3)); /* [elems...], UNCHECKED(d) */
			--elemc;
		} else {
			DO(Dee_function_generator_vref2(self, 1));         /* [elems...], d, ref:key */
			ASSERT(Dee_function_generator_vtop_direct_isref(self));
			Dee_function_generator_vtop_direct_clearref(self); /* Always stolen by `soi_libhostasm_rt_DeeSeqType_InsertFast' */
			DO(Dee_function_generator_vcallapi(self, soi->soi_libhostasm_rt_DeeSeqType_InsertFast,
			                                   VCALL_CC_RAWINTPTR, 2));                     /* [elems...], UNCHECKED(d) */
		}                                                                                   /* [elems...], UNCHECKED(d) */
		ASSERT(Dee_function_generator_vtop_isdirect(self));    /* [elems...], UNCHECKED(d) */
		DO(Dee_function_generator_gjz_except(self, Dee_function_generator_vtopdloc(self))); /* [elems...], d */
		--elemc;
	}

	/* At this point, the v-stack only contains "d", which currently contains all relevant
	 * items. However, there are still some extra fields (possibly type-specific) fields
	 * that need to be initialized, including a call to `DeeObject_Init()' */
	if (seq_type == &DeeDict_Type) {
#ifndef CONFIG_NO_THREADS
		DO(Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self));
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, d_lock)));
#endif /* !CONFIG_NO_THREADS */
		DO(Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self));
		DO(Dee_function_generator_vpopind(self, offsetof(DeeDictObject, ob_weakrefs)));
	} else if (seq_type == &DeeHashSet_Type) {
#ifndef CONFIG_NO_THREADS
		DO(Dee_function_generator_vpush_ATOMIC_RWLOCK_INIT(self));
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, hs_lock)));
#endif /* !CONFIG_NO_THREADS */
		DO(Dee_function_generator_vpush_WEAKREF_SUPPORT_INIT(self));
		DO(Dee_function_generator_vpopind(self, offsetof(DeeHashSetObject, ob_weakrefs)));
	} else {
		/* The read-only dict/set types don't feature additional fields. */
		ASSERT(seq_type == &DeeRoDict_Type || seq_type == &DeeRoSet_Type);
	}

	/* Do the common object init (which also assigns the proper type) */
	DO(Dee_function_generator_vcall_DeeObject_Init_c(self, seq_type));

	/* If the sequence type is a GC object, emit code to start tracking it. */
	if (seq_type->tp_flags & TP_FGC) {
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));
		DO(Dee_function_generator_vcallapi(self, &DeeGC_Track, VCALL_CC_RAWINTPTR_NX, 1));
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	}

	/* Now that the object is fully ready, mark its location as containing a reference. */
	Dee_Freea(result_d_elem_template);
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));                          /* d */
	Dee_function_generator_vtop_direct_setref(self);                                  /* ref:d */
	EDO(err_no_result_d_elem_template, Dee_function_generator_voneref_noalias(self)); /* oneref:ref:d */
	return Dee_function_generator_vsettyp_noalias(self, seq_type);                    /* seq_type:oneref:ref:d */
err:
	Dee_Freea(result_d_elem_template);
err_no_result_d_elem_template:
	return -1;
}

/* [elems...] -> seq */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpackseq(struct Dee_function_generator *__restrict self,
                                DeeTypeObject *__restrict seq_type, Dee_vstackaddr_t elemc) {
	bool asmap = DeeType_InheritsFrom(seq_type, &DeeMapping_Type);
	ASSERTF(!asmap || (elemc % 2) == 0,
	        "Need an even number of elements for a mapping type");
	DO(Dee_function_generator_vdirect(self, elemc));
	DO(Dee_function_generator_vnotoneref(self, elemc));

	/* Check for special case: construct an empty sequence */
	if (elemc == 0) {
		if (seq_type == &DeeTuple_Type)
			return Dee_function_generator_vpush_const(self, Dee_EmptyTuple);
		/* Other sequence types we simply default-construct (which in turn
		 * *should* make it possible for the constructor-call to get inlined) */
		DO(Dee_function_generator_vpush_const(self, seq_type));
		return Dee_function_generator_vopcall(self, 0);
	}

	/* Check for special case: when all elements are constants, we can create the
	 * sequence as its frozen equivalent, and then (if necessary) cast it to its
	 * unfrozen, caller-given type.
	 *
	 * The only type we don't do this for is List when not optimizing for size,
	 * since the list would need to be copied, and putting it together by hand
	 * is faster, as we're able to let the list inherit references to individual
	 * items (if possible). */
	if (((seq_type != &DeeList_Type) ||
	     (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) &&
	    Dee_function_generator_vallconst(self, elemc)) {
		size_t i;
		struct Dee_memval *elemv;
		DREF DeeObject *cseq;
		elemv  = Dee_function_generator_vtop(self) - (elemc - 1);
		if (asmap) {
			cseq = (DREF DeeObject *)DeeRoDict_NewWithHint((size_t)(elemc / 2));
			if unlikely(!cseq)
				goto err;
			for (i = 0; i < (size_t)(elemc / 2); ++i) {
				DeeObject *key   = Dee_memval_const_getobj(&elemv[(i * 2) + 0]);
				DeeObject *value = Dee_memval_const_getobj(&elemv[(i * 2) + 1]);
				if unlikely(DeeRoDict_Insert((DeeRoDictObject **)&cseq, key, value)) {
err_cseq:
					Dee_Decref_likely(cseq);
					goto err;
				}
			}
		} else if (DeeType_InheritsFrom(seq_type, &DeeSet_Type)) {
			cseq = (DREF DeeObject *)DeeRoSet_NewWithHint(elemc);
			if unlikely(!cseq)
				goto err;
			for (i = 0; i < elemc; ++i) {
				DeeObject *key = Dee_memval_const_getobj(&elemv[i]);
				if unlikely(DeeRoSet_Insert((DeeRoSetObject **)&cseq, key))
					goto err_cseq;
			}
		} else {
			cseq = (DREF DeeObject *)DeeTuple_NewUninitialized(elemc);
			if unlikely(!cseq)
				goto err;
			for (i = 0; i < elemc; ++i) {
				DeeObject *elem = Dee_memval_const_getobj(&elemv[i]);
				Dee_Incref(elem);
				DeeTuple_SET(cseq, i, elem); /* Inherit reference */
			}
		}

		cseq = Dee_function_generator_inlineref(self, cseq);
		if unlikely(!cseq)
			goto err;
		DO(Dee_function_generator_vpopmany(self, elemc));
		DO(Dee_function_generator_vpush_const(self, cseq));
		if (Dee_TYPE(cseq) == seq_type)
			return 0;
		return Dee_function_generator_vopcast(self, seq_type);
	}

	/* Special handling when constructing a runtime List/Tuple.
	 * These, we allocate as uninitialized and then fill in. */
	if (seq_type == &DeeList_Type || seq_type == &DeeTuple_Type) {
		bool is_list = seq_type == &DeeList_Type;
		DO(Dee_function_generator_vpush_immSIZ(self, elemc));    /* [elems...], elemc */
		DO(Dee_function_generator_vcallapi(self, is_list ? (void const *)&DeeList_NewUninitialized
		                                                 : (void const *)&DeeTuple_NewUninitialized,
		                                   VCALL_CC_OBJECT, 1)); /* [elems...], ref:seq */
		DO(Dee_function_generator_vdirect1(self));               /* [elems...], ref:seq */
		DO(Dee_function_generator_vdup(self));                   /* [elems...], ref:seq, seq */
		DO(is_list ? Dee_function_generator_vind(self, offsetof(DeeListObject, l_list.ol_elemv))
		           : Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* [elems...], ref:seq, elemv */
		DO(Dee_function_generator_vswap(self));                  /* [elems...], elemv, ref:seq */
		DO(Dee_function_generator_vrrot(self, elemc + 2));       /* ref:seq, [elems...], elemv */
		while (elemc) {
			--elemc;
			DO(Dee_function_generator_vswap(self));                                     /* ref:seq, [elems...], elemv, elem */
			DO(Dee_function_generator_vref2(self, elemc + 3));                          /* ref:seq, [elems...], elemv, ref:elem */
			DO(Dee_function_generator_vpopind(self, elemc * sizeof(DREF DeeObject *))); /* ref:seq, [elems...], elemv */
		}
		DO(Dee_function_generator_vpop(self)); /* ref:seq */
		if (is_list) {
			ASSERT(Dee_function_generator_vtop_direct_isref(self));
			Dee_function_generator_vtop_direct_clearref(self); /* Inherited by `DeeGC_Track()' */
			DO(Dee_function_generator_vcallapi(self, &DeeGC_Track, VCALL_CC_RAWINTPTR_NX, 1));
			ASSERT(!Dee_function_generator_vtop_direct_isref(self));
			Dee_function_generator_vtop_direct_setref(self); /* Returned by `DeeGC_Track()' */
		}
		DO(Dee_function_generator_vsettyp_noalias(self, seq_type));
		return Dee_function_generator_voneref_noalias(self);
	} else if ((seq_type == &DeeDict_Type || seq_type == &DeeRoDict_Type ||
	            seq_type == &DeeHashSet_Type || seq_type == &DeeRoSet_Type) &&
	           !(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		return vpack_map_or_set_at_runtime(self, seq_type, elemc);
	} else if (seq_type == &DeeDict_Type || seq_type == &DeeHashSet_Type) {
		Dee_vstackaddr_t i;
		/* Force all elements to become references. */
		for (i = 0; i < elemc; ++i) {
			DO(Dee_function_generator_vlrot(self, elemc));
			DO(Dee_function_generator_vref2(self, elemc));
		}
		DO(Dee_function_generator_vlinear(self, elemc, true)); /* [elems...], elemv */
		DO(Dee_function_generator_vpush_immSIZ(self,           /* [elems...], elemv, elemc */
		                                       seq_type == &DeeDict_Type ? (size_t)(elemc / 2)
		                                                                 : (size_t)(elemc)));
		DO(Dee_function_generator_vswap(self));  /* [elems...], elemc, elemv */
		DO(Dee_function_generator_vcallapi(self, /* [elems...], result */
		                                   seq_type == &DeeDict_Type ? (void const *)&DeeDict_NewKeyItemsInherited
		                                                             : (void const *)&DeeHashSet_NewItemsInherited,
		                                   VCALL_CC_OBJECT, 2));
		DO(Dee_function_generator_vdirect1(self));          /* [elems...], result */
		DO(Dee_function_generator_vrrot(self, elemc + 1));  /* result, [elems...] */
		/* In the success-case, all references were inherited by the Dict/HashSet */
		for (i = 0; i < elemc; ++i) {
			ASSERT(Dee_function_generator_vtop_direct_isref(self));
			Dee_function_generator_vtop_direct_clearref(self);
			DO(Dee_function_generator_vpop(self));
		}
		DO(Dee_function_generator_vsettyp_noalias(self, seq_type));
		return Dee_function_generator_voneref_noalias(self);
	}

	/* Fallback: Generate a constructor call "seq_type({ elems... })" */
	DO(Dee_function_generator_vpush_const(self, seq_type)); /* [elems...], seq_type */
	DO(Dee_function_generator_vrrot(self, elemc + 1));      /* seq_type, [elems...] */
	return vopcallseqmap_impl(self, elemc, asmap, false, false);
err:
	return -1;
}



struct host_operator_specs {
	void const *hos_apifunc; /* [0..1] API function (or NULL if fallback handling must be used) */
	uint8_t     hos_argc;    /* Argument count (1-4) */
	uint8_t     hos_cc;      /* Operator calling convention (one of `VCALL_CC_*') */
	bool        hos_inplace; /* Is this an inplace operator? */
};

PRIVATE struct host_operator_specs const operator_apis[] = {
	/* [OPERATOR_CONSTRUCTOR]  = */ { (void const *)NULL },
	/* [OPERATOR_COPY]         = */ { (void const *)&DeeObject_Copy, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_DEEPCOPY]     = */ { (void const *)&DeeObject_DeepCopy, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_DESTRUCTOR]   = */ { (void const *)NULL },
	/* [OPERATOR_ASSIGN]       = */ { (void const *)&DeeObject_Assign, 2, VCALL_CC_INT, false },
	/* [OPERATOR_MOVEASSIGN]   = */ { (void const *)&DeeObject_MoveAssign, 2, VCALL_CC_INT, false },
	/* [OPERATOR_STR]          = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_REPR]         = */ { (void const *)&DeeObject_Repr, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_BOOL]         = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_ITERNEXT]     = */ { (void const *)NULL }, /* Special handling (because `DeeObject_IterNext' can return ITER_DONE) */
	/* [OPERATOR_CALL]         = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_INT]          = */ { (void const *)&DeeObject_Int, 1, VCALL_CC_OBJECT, false },
#ifdef CONFIG_HAVE_FPU
	/* [OPERATOR_FLOAT]        = */ { (void const *)&libhostasm_rt_DeeObject_Float, 1, VCALL_CC_OBJECT, false },
#else /* CONFIG_HAVE_FPU */
	/* [OPERATOR_FLOAT]        = */ { (void const *)NULL },
#endif /* !CONFIG_HAVE_FPU */
	/* [OPERATOR_INV]          = */ { (void const *)&DeeObject_Inv, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_POS]          = */ { (void const *)&DeeObject_Pos, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_NEG]          = */ { (void const *)&DeeObject_Neg, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_ADD]          = */ { (void const *)&DeeObject_Add, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_SUB]          = */ { (void const *)&DeeObject_Sub, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_MUL]          = */ { (void const *)&DeeObject_Mul, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_DIV]          = */ { (void const *)&DeeObject_Div, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_MOD]          = */ { (void const *)&DeeObject_Mod, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_SHL]          = */ { (void const *)&DeeObject_Shl, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_SHR]          = */ { (void const *)&DeeObject_Shr, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_AND]          = */ { (void const *)&DeeObject_And, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_OR]           = */ { (void const *)&DeeObject_Or, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_XOR]          = */ { (void const *)&DeeObject_Xor, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_POW]          = */ { (void const *)&DeeObject_Pow, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_INC]          = */ { (void const *)&DeeObject_Inc, 1, VCALL_CC_INT, true },
	/* [OPERATOR_DEC]          = */ { (void const *)&DeeObject_Dec, 1, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_ADD]  = */ { (void const *)&DeeObject_InplaceAdd, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_SUB]  = */ { (void const *)&DeeObject_InplaceSub, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_MUL]  = */ { (void const *)&DeeObject_InplaceMul, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_DIV]  = */ { (void const *)&DeeObject_InplaceDiv, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_MOD]  = */ { (void const *)&DeeObject_InplaceMod, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_SHL]  = */ { (void const *)&DeeObject_InplaceShl, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_SHR]  = */ { (void const *)&DeeObject_InplaceShr, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_AND]  = */ { (void const *)&DeeObject_InplaceAnd, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_OR]   = */ { (void const *)&DeeObject_InplaceOr, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_XOR]  = */ { (void const *)&DeeObject_InplaceXor, 2, VCALL_CC_INT, true },
	/* [OPERATOR_INPLACE_POW]  = */ { (void const *)&DeeObject_InplacePow, 2, VCALL_CC_INT, true },
	/* [OPERATOR_HASH]         = */ { (void const *)&DeeObject_Hash, 1, VCALL_CC_MORPH_UINTPTR, false },
	/* [OPERATOR_EQ]           = */ { (void const *)&DeeObject_CompareEqObject, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_NE]           = */ { (void const *)&DeeObject_CompareNeObject, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_LO]           = */ { (void const *)&DeeObject_CompareLoObject, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_LE]           = */ { (void const *)&DeeObject_CompareLeObject, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_GR]           = */ { (void const *)&DeeObject_CompareGrObject, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_GE]           = */ { (void const *)&DeeObject_CompareGeObject, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_ITERSELF]     = */ { (void const *)&DeeObject_IterSelf, 1, VCALL_CC_OBJECT, false },
	/* [OPERATOR_SIZE]         = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_CONTAINS]     = */ { (void const *)&DeeObject_Contains, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_GETITEM]      = */ { (void const *)&DeeObject_GetItem, 2, VCALL_CC_OBJECT, false },
	/* [OPERATOR_DELITEM]      = */ { (void const *)&DeeObject_DelItem, 2, VCALL_CC_INT, false },
	/* [OPERATOR_SETITEM]      = */ { (void const *)&DeeObject_SetItem, 3, VCALL_CC_INT, false },
	/* [OPERATOR_GETRANGE]     = */ { (void const *)&DeeObject_GetRange, 3, VCALL_CC_OBJECT, false },
	/* [OPERATOR_DELRANGE]     = */ { (void const *)&DeeObject_DelRange, 3, VCALL_CC_INT, false },
	/* [OPERATOR_SETRANGE]     = */ { (void const *)&DeeObject_SetRange, 4, VCALL_CC_INT, false },
	/* [OPERATOR_GETATTR]      = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_DELATTR]      = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_SETATTR]      = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_ENUMATTR]     = */ { (void const *)NULL }, /* Special handling */
	/* [OPERATOR_ENTER]        = */ { (void const *)&DeeObject_Enter, 1, VCALL_CC_INT, false },
	/* [OPERATOR_LEAVE]        = */ { (void const *)&DeeObject_Leave, 1, VCALL_CC_INT, false },
};

/* this, [args...]  ->  this, [args...]
 * Try to lookup the inlined API function belonging to `operator_name' with `p_extra_argc'
 * @assume(Dee_memval_typeof(Dee_function_generator_vtop(self) - *p_extra_argc) == type);
 * @return: 0 : Dedicated operator API exists
 * @return: 1 : No dedicated operator API exists
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 4, 5)) int DCALL
vtype_get_operator_api_function(struct Dee_function_generator *__restrict self,
                                DeeTypeObject const *__restrict type,
                                uint16_t operator_name, Dee_vstackaddr_t *__restrict p_extra_argc,
                                struct host_operator_specs *__restrict result, bool inplace) {
	void const *api_function;
	byte_t const *field_base;
	struct opinfo const *info;
	unsigned int optype;
	(void)self;

	/* Special case for operators whose behavior depends on the given # of arguments. */
	switch (operator_name) {

	case OPERATOR_CONSTRUCTOR:
	case OPERATOR_DESTRUCTOR:
	case OPERATOR_DEEPCOPY:
	case OPERATOR_COPY:
		goto nope;

	case OPERATOR_CALL:
		api_function = NULL;
		if (*p_extra_argc == 1) {
			api_function = type->tp_call;
		} else if (*p_extra_argc == 2) {
			api_function = type->tp_call_kw;
		}
		if (api_function && !inplace) {
			result->hos_apifunc = api_function;
			result->hos_argc    = (uint8_t)*p_extra_argc + 1;
			result->hos_cc      = VCALL_CC_OBJECT;
			result->hos_inplace = false;
			return 0;
		}
		break;

	default: break;
	}

	/* Lookup dynamic info on the operator. */
	info = Dee_OperatorInfo(Dee_TYPE(type), operator_name);
	if unlikely(!info)
		goto nope;

	/* Load the operator's C function pointer. */
	switch (info->oi_class) {
	case OPCLASS_TYPE: field_base = (byte_t const *)type; break;
	case OPCLASS_GC: field_base = (byte_t const *)type->tp_gc; break;
	case OPCLASS_MATH: field_base = (byte_t const *)type->tp_math; break;
	case OPCLASS_CMP: field_base = (byte_t const *)type->tp_cmp; break;
	case OPCLASS_SEQ: field_base = (byte_t const *)type->tp_seq; break;
	case OPCLASS_ATTR: field_base = (byte_t const *)type->tp_attr; break;
	case OPCLASS_WITH: field_base = (byte_t const *)type->tp_with; break;
	case OPCLASS_BUFFER: field_base = (byte_t const *)type->tp_buffer; break;
	default: goto nope;
	}
	if (!field_base)
		goto nope;
	field_base += info->oi_offset;
	api_function = *(void const *const *)field_base;
	if (!api_function)
		goto nope;
	result->hos_apifunc = api_function;

	/* Translate C RTTI info into a VCALL calling convention (an potentially rotate arguments) */
	optype = info->oi_type;
	result->hos_inplace = !!(optype & OPTYPE_INPLACE);
	if unlikely(!!inplace != result->hos_inplace)
		goto nope;
	optype &= ~OPTYPE_INPLACE;
	switch (optype) {

	case OPTYPE_ROBJECT | OPTYPE_UNARY:
	case OPTYPE_ROBJECT | OPTYPE_BINARY:
	case OPTYPE_ROBJECT | OPTYPE_TRINARY:
	case OPTYPE_ROBJECT | OPTYPE_QUAD: {
		STATIC_ASSERT(OPTYPE_UNARY == 1);
		STATIC_ASSERT(OPTYPE_BINARY == 2);
		STATIC_ASSERT(OPTYPE_TRINARY == 3);
		STATIC_ASSERT(OPTYPE_QUAD == 4);
		result->hos_argc = optype & ~OPTYPE_ROBJECT;
		result->hos_cc   = VCALL_CC_OBJECT;
	}	break;

	case OPTYPE_RUINTPTR | OPTYPE_UNARY:
	case OPTYPE_RUINTPTR | OPTYPE_BINARY:
	case OPTYPE_RUINTPTR | OPTYPE_TRINARY:
	case OPTYPE_RUINTPTR | OPTYPE_QUAD: {
		result->hos_argc = optype & ~OPTYPE_ROBJECT;
		result->hos_cc   = VCALL_CC_MORPH_UINTPTR;
	}	break;

	case OPTYPE_RINT | OPTYPE_UNARY:
	case OPTYPE_RINT | OPTYPE_BINARY:
	case OPTYPE_RINT | OPTYPE_TRINARY:
	case OPTYPE_RINT | OPTYPE_QUAD: {
		result->hos_argc = optype & ~OPTYPE_RINT;
		result->hos_cc   = VCALL_CC_INT;
	}	break;

	default: goto nope;
	}
	if (result->hos_argc != (*p_extra_argc + 1))
		goto nope;
	return 0;
nope:
	return 1;
}

/* [args...]  ->  result (flags == VOP_F_PUSHRES)
 * [args...]  ->  N/A    (flags == VOP_F_NORMAL) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vop(struct Dee_function_generator *__restrict self,
                           uint16_t operator_name, Dee_vstackaddr_t argc,
                           unsigned int flags) {
	DeeTypeObject *return_type = NULL;
	DO(Dee_function_generator_state_unshare(self));

	/* Special handling for certain operators. */
	if unlikely(self->fg_state->ms_stackc < argc)
		return err_illegal_stack_effect();
	switch (operator_name) {

	case OPERATOR_STR:
		if (argc == 1) {
			DO(Dee_function_generator_vopstr(self));
			goto done_with_result;
		}
		return_type = &DeeString_Type;
		break;

	case OPERATOR_REPR:
		return_type = &DeeString_Type;
		break;

	case OPERATOR_BOOL:
		if (argc == 1) {
			DO(Dee_function_generator_vopbool(self, false));
			goto done_with_result;
		}
		return_type = &DeeBool_Type;
		break;

	case OPERATOR_CALL:
		if (argc == 2) {
			DO(Dee_function_generator_vassert_type_exact_c(self, &DeeTuple_Type));
			DO(Dee_function_generator_vopcalltuple(self));
			goto done_with_result;
		}
		if (argc == 3) {
			DO(Dee_function_generator_vswap(self));
			DO(Dee_function_generator_vassert_type_exact_c(self, &DeeTuple_Type));
			DO(Dee_function_generator_vswap(self));
			DO(Dee_function_generator_vopcalltuplekw(self));
			goto done_with_result;
		}
		break;

	case OPERATOR_INT:
		if (argc == 1) {
			DO(Dee_function_generator_vopint(self));
			goto done_with_result;
		}
		return_type = &DeeInt_Type;
		break;

	case OPERATOR_SIZE:
		if (argc == 1) {
			DO(Dee_function_generator_vopsize(self));
			goto done_with_result;
		}
		break;

	case OPERATOR_GETATTR:
		if (argc == 2) {
			DO(Dee_function_generator_vopgetattr(self));
			goto done_with_result;
		}
		break;

	case OPERATOR_DELATTR:
		if (argc == 2) {
			DO(Dee_function_generator_vopdelattr(self));
			goto done_without_result;
		}
		break;

	case OPERATOR_SETATTR:
		if (argc == 3) {
			DO(Dee_function_generator_vopsetattr(self));
			goto done_without_result;
		}
		break;

	default: break;
	}

	DO(Dee_function_generator_vdirect(self, argc));
	if likely(argc >= 1) {
		/* If the type of the "this"-operand is known, try to
		 * directly dispatch to the operator implementation. */
		struct host_operator_specs specs;
		struct Dee_memval *thisval;
		DeeTypeObject *this_type;
		thisval = Dee_function_generator_vtop(self) - (argc - 1);
		if (Dee_memval_direct_isconst(thisval)) {
			DeeObject *thisobj = Dee_memval_const_getobj(thisval);
			/* Try to produce a compile-time call if the operator is
			 * constexpr, and all arguments are constants as well. */
			if (DeeType_IsOperatorConstexpr(Dee_TYPE(thisobj), operator_name)) {
				size_t i;
				DeeObject **constant_argv;
				DREF DeeObject *op_result;
				constant_argv = (DeeObject **)Dee_Mallocac(argc - 1, sizeof(DeeObject *));
				if unlikely(!constant_argv)
					goto err;
				for (i = 1; i < argc; ++i) {
					if (!Dee_memval_direct_isconst(&thisval[i]))
						goto not_all_args_are_constant;
					constant_argv[i - 1] = Dee_memval_const_getobj(&thisval[i]);
				}
				op_result = DeeObject_InvokeOperator(thisobj, operator_name, argc - 1, constant_argv);
				if unlikely(!op_result) {
					if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
						Dee_Freea(constant_argv);
						goto err;
					}
					DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
					goto not_all_args_are_constant;
				}
				Dee_Freea(constant_argv);
				if (flags & VOP_F_PUSHRES) {
					op_result = Dee_function_generator_inlineref(self, op_result);
					if unlikely(!op_result)
						goto err;
				} else {
					Dee_Decref_likely(op_result);
				}
				DO(Dee_function_generator_vpopmany(self, argc));
				if (flags & VOP_F_PUSHRES)
					return Dee_function_generator_vpush_const(self, op_result);
				return 0;
not_all_args_are_constant:
				Dee_Freea(constant_argv);
			}
		}

		this_type = Dee_memval_typeof(thisval);
		if (this_type != NULL) {
			/* Try to determine the operator's return type from doc info. */
			if ((flags & VOP_F_PUSHRES) && return_type == NULL) {
				return_type = vget_operator_return_type(self, this_type, operator_name, argc - 1);
				if unlikely(return_type == (DeeTypeObject *)ITER_DONE)
					goto err;
			}

			/* Inherit the operator from an underlying base-class. */
			if (DeeType_InheritOperator(this_type, operator_name)) {
				int temp;
				/* Check if there is a dedicated optimization for this operator and type. */
				struct Dee_ccall_optimization const *cco;
				DeeTypeObject *orig_type = DeeType_GetOperatorOrigin(this_type, operator_name);
				cco = Dee_ccall_find_operator_optimization(orig_type, operator_name, argc - 1);
				if (cco != NULL) {
					Dee_vstackaddr_t old_stackc = self->fg_state->ms_stackc;
					temp = (*cco->tcco_func)(self, argc - 1);
					if (temp <= 0) {
						if likely(temp == 0) {
							Dee_vstackaddr_t new_stackc = self->fg_state->ms_stackc;
							ASSERT((new_stackc == old_stackc - argc) ||
							       (new_stackc == old_stackc - argc + 1));
							if (new_stackc == old_stackc - argc)
								goto done_without_result;
							goto done_with_result;
						}
						return temp; /* Error */
					}
				}

				/* TODO: Check if there is an NSI operator. */

				/* Try to produce an inlined operator call. */
				--argc;
				temp = vtype_get_operator_api_function(self, this_type, operator_name,
				                                       &argc, &specs, false);
				++argc;
				if (temp <= 0) {
					if unlikely(temp < 0)
						goto err;
					ASSERT(specs.hos_argc == argc);
					DO(Dee_function_generator_vnotoneref_if_operator_at(self, operator_name, argc));
					DO(Dee_function_generator_vnotoneref(self, argc - 1));
					DO(Dee_function_generator_vcallapi(self, specs.hos_apifunc, specs.hos_cc, argc));
					if (specs.hos_cc != VCALL_CC_INT) /* `VCALL_CC_INT' is the only one used that doesn't have a return value */
						goto done_with_result;
					goto done_without_result;
				}
			}
		}
	}

	if (flags & VOP_F_ALLOWNATIVE) {
		/* TODO: check arguments to see if native object calls can be used:
		 * - DeeObject_AddInt8
		 * - DeeObject_SubInt8
		 * - DeeObject_MulInt8
		 * - DeeObject_DivInt8
		 * - DeeObject_ModInt8
		 * - DeeObject_ShlUInt8
		 * - DeeObject_ShrUInt8
		 * - DeeObject_AddUInt32
		 * - DeeObject_SubUInt32
		 * - DeeObject_AndUInt32
		 * - DeeObject_OrUInt32
		 * - DeeObject_XorUInt32
		 * - DeeObject_GetRangeBeginIndex
		 * - DeeObject_GetRangeEndIndex
		 * - DeeObject_GetRangeIndex
		 * - DeeObject_SetRangeBeginIndex
		 * - DeeObject_SetRangeEndIndex
		 * - DeeObject_SetRangeIndex
		 */
	}

	/* Check if there is a dedicated API function. */
	if (operator_name < COMPILER_LENOF(operator_apis)) {
		struct host_operator_specs const *specs = &operator_apis[operator_name];
		if (specs->hos_apifunc != NULL && specs->hos_argc == argc && !specs->hos_inplace) {
			DO(Dee_function_generator_vnotoneref_if_operator_at(self, operator_name, argc));
			DO(Dee_function_generator_vnotoneref(self, argc - 1));
			DO(Dee_function_generator_vcallapi(self, specs->hos_apifunc,
			                                   specs->hos_cc, argc));
			if (specs->hos_cc != VCALL_CC_INT) /* `VCALL_CC_INT' is the only one used that doesn't have a return value */
				goto done_with_result;
done_without_result:
			if (flags & VOP_F_PUSHRES) {
				/* Always make sure to return some value on-stack. */
				return Dee_function_generator_vpush_none(self);
			}
			return 0;
		}
	}

	/* Fallback: encode a call to `DeeObject_InvokeOperator()' */
	if unlikely(argc < 1)
		return err_illegal_stack_effect();
	--argc; /* The "this"-argument is passed individually */
	DO(Dee_function_generator_vnotoneref(self, argc));           /* this, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));        /* this, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 2));            /* [args...], argv, this */
	DO(Dee_function_generator_vnotoneref_if_operator(self, operator_name, 1)); /* [args...], argv, this */
	DO(Dee_function_generator_vpush_imm16(self, operator_name)); /* [args...], argv, this, opname */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));         /* [args...], argv, this, opname, argc */
	DO(Dee_function_generator_vlrot(self, 4));                   /* [args...], this, opname, argc, argv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_InvokeOperator, VCALL_CC_RAWINTPTR, 4)); /* [args...], UNCHECKED(result) */
	DO(Dee_function_generator_vrrot(self, argc + 1));            /* UNCHECKED(result), [args...] */
	DO(Dee_function_generator_vpopmany(self, argc));             /* UNCHECKED(result) */
	DO(Dee_function_generator_vcheckobj(self));                  /* result */
done_with_result:                                                /* result */
	if (!(flags & VOP_F_PUSHRES))
		return Dee_function_generator_vpop(self);
	if (return_type)
		return Dee_function_generator_vsettyp_noalias(self, return_type); /* result */
	return 0;
err:
	return -1;
}


/* [ref]:this, [args...]  ->  ref:this, result
 * [ref]:this, [args...]  ->  ref:this
 * NOTE: This function doesn't assign the new assumed object type to this/result! */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vinplaceop_invoke_specs(struct Dee_function_generator *__restrict self,
                        struct host_operator_specs const *__restrict specs,
                        uint16_t operator_name) {
	Dee_vstackaddr_t argc = specs->hos_argc - 1;
	DO(Dee_function_generator_vnotoneref(self, argc));  /* [ref]:this, [args...] */
	DO(Dee_function_generator_vlrot(self, argc + 1));   /* [args...], [ref]:this */
	DO(Dee_function_generator_vnotoneref_if_operator(self, operator_name, 1)); /* [args...], [ref]:this */
	/* IMPORTANT: don't use vref2() here! The caller of the `vinplaceop()' pushed an alias
	 *            into VTOP, so vref2() would do an extra incref by thinking that the location
	 *            being alocated should also need one. -- Only do vref_noalais() to force a
	 *            reference in case the caller passed a constant as original this-value. */
	DO(Dee_function_generator_vref_noalias(self));      /* [args...], ref:this */
	DO(Dee_function_generator_vlinear(self, 1, false)); /* [args...], ref:this, p_this */
	DO(Dee_function_generator_vswap(self));             /* [args...], p_this, ref:this */
	DO(Dee_function_generator_vrrot(self, argc + 2));   /* ref:this, [args...], p_this */
	DO(Dee_function_generator_vrrot(self, argc + 1));   /* ref:this, p_this, [args...] */
	return Dee_function_generator_vcallapi(self, specs->hos_apifunc, specs->hos_cc, argc + 1); /* ref:this, [result] */
err:
	return -1;
}

/* [ref]:this, [args...]  ->  [ref]:this, result (flags == VOP_F_PUSHRES)
 * [ref]:this, [args...]  ->  [ref]:this         (flags == VOP_F_NORMAL) */
INTERN WUNUSED NONNULL((1)) int DCALL
vinplaceop_with_vop(struct Dee_function_generator *__restrict self,
                    uint16_t operator_name, Dee_vstackaddr_t argc,
                    unsigned int flags) {
	int result = Dee_function_generator_vop(self, operator_name, argc + 1, flags | VOP_F_PUSHRES);
	if (likely(result == 0) && (flags & VOP_F_PUSHRES))
		result = Dee_function_generator_vdup(self);
	return result;
}

/* [ref]:this, [args...]  ->  [ref]:this, result (flags == VOP_F_PUSHRES)
 * [ref]:this, [args...]  ->  [ref]:this         (flags == VOP_F_NORMAL)
 * NOTE: If "this" isn't a constant, it is the caller's responsibility to
 *       ensure that "this" doesn't have any aliases. Otherwise, aliases
 *       might inadvertently also receive the updated object. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vinplaceop(struct Dee_function_generator *__restrict self,
                                  uint16_t operator_name, Dee_vstackaddr_t argc,
                                  unsigned int flags) {
	struct Dee_memval *thisval;
	DeeTypeObject *this_type;
	DeeTypeObject *return_type = NULL;
	DO(Dee_function_generator_state_unshare(self));

	/* Special handling for certain operators. */
	/*switch (operator_name) {
	default: break;
	}*/

	DO(Dee_function_generator_vdirect(self, argc + 1));
	thisval = Dee_function_generator_vtop(self) - argc;
	if (Dee_memval_direct_isconst(thisval)) {
		DeeObject *thisobj = Dee_memval_const_getobj(thisval);
		/* Try to produce a compile-time call if the operator is
		 * constexpr, and all arguments are constants as well. */
		if (DeeType_IsOperatorConstexpr(Dee_TYPE(thisobj), operator_name)) {
			size_t i;
			DeeObject **constant_argv;
			DREF DeeObject *op_result, *new_thisobj;
			constant_argv = (DeeObject **)Dee_Mallocac(argc, sizeof(DeeObject *));
			if unlikely(!constant_argv)
				goto err;
			for (i = 0; i < argc; ++i) {
				if (!Dee_memval_direct_isconst(&thisval[i + 1]))
					goto not_all_args_are_constant;
				constant_argv[i] = Dee_memval_const_getobj(&thisval[i + 1]);
			}
			new_thisobj = thisobj;
			Dee_Incref(new_thisobj);
			op_result = DeeObject_PInvokeOperator(&new_thisobj, operator_name, argc, constant_argv);
			if unlikely(!op_result) {
				Dee_Decref(new_thisobj);
				if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
					Dee_Freea(constant_argv);
					goto err;
				}
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				goto not_all_args_are_constant;
			}
			Dee_Freea(constant_argv);
			ASSERT(new_thisobj);
			if (new_thisobj == thisobj) {
				Dee_DecrefNokill(new_thisobj);
			} else {
				new_thisobj = Dee_function_generator_inlineref(self, new_thisobj);
				if unlikely(!new_thisobj) {
					Dee_Decref(op_result);
					goto err;
				}
			}
			ASSERT(Dee_memval_direct_isconst(thisval));
			ASSERT(Dee_memval_const_getobj(thisval) == thisobj);
			Dee_memval_const_setobj(thisval, new_thisobj);
			if (flags & VOP_F_PUSHRES) {
				op_result = Dee_function_generator_inlineref(self, op_result);
				if unlikely(!op_result)
					goto err;
			} else {
				Dee_Decref_likely(op_result);
			}
			DO(Dee_function_generator_vpopmany(self, argc));
			if (flags & VOP_F_PUSHRES)
				return Dee_function_generator_vpush_const(self, op_result);
			return 0;
not_all_args_are_constant:
			Dee_Freea(constant_argv);
		}
	}

	this_type = Dee_memval_typeof(thisval);
	if (this_type != NULL) {
		uint16_t non_inplace_operator_name;

		/* Try to determine the operator's return type from doc info. */
		return_type = vget_operator_return_type(self, this_type, operator_name, argc);
		if unlikely(return_type == (DeeTypeObject *)ITER_DONE)
			goto err;

		/* Try to produce an inlined operator call. */
		if (DeeType_InheritOperator(this_type, operator_name)) {
			struct host_operator_specs specs;
			int temp;

			/* Check if there is a dedicated optimization for this operator and type. */
			struct Dee_ccall_optimization const *cco;
			DeeTypeObject *orig_type = DeeType_GetOperatorOrigin(this_type, operator_name);
			cco = Dee_ccall_find_operator_optimization(orig_type, operator_name, argc);
			if (cco != NULL) {
				Dee_vstackaddr_t old_stackc = self->fg_state->ms_stackc;
				temp = (*cco->tcco_func)(self, argc);
				if (temp <= 0) {
					if likely(temp == 0) {
						Dee_vstackaddr_t new_stackc = self->fg_state->ms_stackc;
						ASSERT((new_stackc == old_stackc - argc) ||
						       (new_stackc == old_stackc - argc + 1));
						if (new_stackc == old_stackc - argc)
							goto done_without_result;
						goto done_with_result;
					}
					return temp; /* Error */
				}
			}

			temp = vtype_get_operator_api_function(self, this_type, operator_name,
			                                       &argc, &specs, true);
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err;
				ASSERT(specs.hos_argc == argc + 1);
				ASSERT(specs.hos_inplace);                                /* [ref]:this, [args...] */
				DO(vinplaceop_invoke_specs(self, &specs, operator_name)); /* ref:this, [result] */
				if (specs.hos_cc != VCALL_CC_INT) /* `VCALL_CC_INT' is the only one used that doesn't have a return value */
					goto done_with_result;
				goto done_without_result;
			}
		}

		/* If we know the type won't implement (e.g. OPERATOR_INPLACE_ADD),
		 * then we can instead generate code to do OPERATOR_ADD, and re-assign
		 * the result to the indirection of the p_this argument. */
		non_inplace_operator_name = 0;
		switch (operator_name) {

		case OPERATOR_INC:
		case OPERATOR_DEC:
			if (argc == 0) {
				uint16_t operator_inplace_add = OPERATOR_INPLACE_ADD;
				uint16_t operator_inplace_sub = OPERATOR_INPLACE_SUB;
				uint16_t operator_add = OPERATOR_ADD;
				uint16_t operator_sub = OPERATOR_SUB;
				if (operator_name == OPERATOR_DEC) {
					operator_inplace_add = OPERATOR_INPLACE_SUB;
					operator_inplace_sub = OPERATOR_INPLACE_ADD;
					operator_add = OPERATOR_SUB;
					operator_sub = OPERATOR_ADD;
				}
				if (DeeType_HasOperator(this_type, operator_inplace_add)) {
					DO(Dee_function_generator_vpush_const(self, DeeInt_One));                       /* [ref]:this, DeeInt_One */
					return Dee_function_generator_vinplaceop(self, operator_inplace_add, 1, flags); /* [ref]:this, [result] */
				} else if (DeeType_HasOperator(this_type, operator_add)) {
					DO(Dee_function_generator_vpush_const(self, DeeInt_One)); /* [ref]:this, DeeInt_One */
					return vinplaceop_with_vop(self, operator_add, 1, flags); /* result, [result] */
				} else if (DeeType_HasOperator(this_type, operator_inplace_sub)) {
					DO(Dee_function_generator_vpush_const(self, DeeInt_MinusOne));                  /* [ref]:this, DeeInt_MinusOne */
					return Dee_function_generator_vinplaceop(self, operator_inplace_sub, 1, flags); /* [ref]:this, [result] */
				} else if (DeeType_HasOperator(this_type, operator_sub)) {
					DO(Dee_function_generator_vpush_const(self, DeeInt_MinusOne)); /* [ref]:this, DeeInt_MinusOne */
					return vinplaceop_with_vop(self, operator_sub, 1, flags);      /* result, [result] */
				}
			}
			break;

		case OPERATOR_INPLACE_ADD: non_inplace_operator_name = OPERATOR_ADD; break;
		case OPERATOR_INPLACE_SUB: non_inplace_operator_name = OPERATOR_SUB; break;
		case OPERATOR_INPLACE_MUL: non_inplace_operator_name = OPERATOR_MUL; break;
		case OPERATOR_INPLACE_DIV: non_inplace_operator_name = OPERATOR_DIV; break;
		case OPERATOR_INPLACE_MOD: non_inplace_operator_name = OPERATOR_MOD; break;
		case OPERATOR_INPLACE_SHL: non_inplace_operator_name = OPERATOR_SHL; break;
		case OPERATOR_INPLACE_SHR: non_inplace_operator_name = OPERATOR_SHR; break;
		case OPERATOR_INPLACE_AND: non_inplace_operator_name = OPERATOR_AND; break;
		case OPERATOR_INPLACE_OR: non_inplace_operator_name = OPERATOR_OR; break;
		case OPERATOR_INPLACE_XOR: non_inplace_operator_name = OPERATOR_XOR; break;
		case OPERATOR_INPLACE_POW: non_inplace_operator_name = OPERATOR_POW; break;

		default: break;
		}

		/* Try to encode as the non-inplace equivalent, if we can determine that the type supports it. */
		if (non_inplace_operator_name != 0 && DeeType_HasOperator(this_type, non_inplace_operator_name))
			return vinplaceop_with_vop(self, non_inplace_operator_name, argc, flags);
	}

	if (flags & VOP_F_ALLOWNATIVE) {
		/* TODO: check arguments to see if native object calls can be used:
		 * - DeeObject_InplaceAddInt8
		 * - DeeObject_InplaceSubInt8
		 * - DeeObject_InplaceMulInt8
		 * - DeeObject_InplaceDivInt8
		 * - DeeObject_InplaceModInt8
		 * - DeeObject_InplaceShlUInt8
		 * - DeeObject_InplaceShrUInt8
		 * - DeeObject_InplaceAddUInt32
		 * - DeeObject_InplaceSubUInt32
		 * - DeeObject_InplaceAndUInt32
		 * - DeeObject_InplaceOrUInt32
		 * - DeeObject_InplaceXorUInt32
		 */
	}

	/* Check if there is a dedicated API function. */
	if (operator_name < COMPILER_LENOF(operator_apis)) {
		struct host_operator_specs const *specs = &operator_apis[operator_name];
		if (specs->hos_apifunc != NULL && specs->hos_argc == argc + 1 && specs->hos_inplace) {
			DO(vinplaceop_invoke_specs(self, specs, operator_name)); /* ref:this, [result] */
			if (specs->hos_cc != VCALL_CC_INT) /* `VCALL_CC_INT' is the only one used that doesn't have a return value */
				goto done_with_result;
done_without_result:
			DO(Dee_function_generator_vsettyp(self, return_type)); /* [ref]:this */
			if (flags & VOP_F_PUSHRES) {
				/* Always make sure to return some value on-stack. */
				return Dee_function_generator_vpush_none(self);
			}
			return 0;
		}
	}

	/* Fallback: encode a call to `DeeObject_PInvokeOperator()' */
	DO(Dee_function_generator_vnotoneref(self, argc));           /* [ref]:this, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));        /* [ref]:this, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 2));            /* [args...], argv, [ref]:this */
	DO(Dee_function_generator_vref_noalias(self));               /* [args...], argv, ref:this */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, operator_name, 1)); /* [args...], argv, ref:this */
	DO(Dee_function_generator_vlinear(self, 1, false));          /* [args...], argv, ref:this, p_this */
	DO(Dee_function_generator_vswap(self));                      /* [args...], argv, p_this, ref:this */
	DO(Dee_function_generator_vrrot(self, argc + 3));            /* ref:this, [args...], argv, p_this */
	DO(Dee_function_generator_vpush_imm16(self, operator_name)); /* ref:this, [args...], argv, p_this, opname */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));         /* ref:this, [args...], argv, p_this, opname, argc */
	DO(Dee_function_generator_vlrot(self, 4));                   /* ref:this, [args...], p_this, opname, argc, argv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_PInvokeOperator, VCALL_CC_RAWINTPTR, 4)); /* ref:this, [args...], UNCHECKED(result) */
	DO(Dee_function_generator_vrrot(self, argc + 1));            /* ref:this, UNCHECKED(result), [args...] */
	DO(Dee_function_generator_vpopmany(self, argc));             /* ref:this, UNCHECKED(result) */
	DO(Dee_function_generator_vcheckobj(self));                  /* ref:this, result */
done_with_result:                                                /* ref:this, result */
	if (!(flags & VOP_F_PUSHRES)) {                              /* ref:this, result */
		DO(Dee_function_generator_vpop(self));                   /* ref:this */
		return Dee_function_generator_vsettyp(self, return_type); /* ref:this */
	}
	/* NOTE: When return types are known, the that return type always
	 *       matches the type of object that got assigned to `*p_this' */
	DO(Dee_function_generator_vsettyp_noalias(self, return_type)); /* ref:this, result */
	DO(Dee_function_generator_vswap(self));                        /* result, ref:this */
	DO(Dee_function_generator_vsettyp(self, return_type));         /* result, ref:this */
	return Dee_function_generator_vswap(self);                     /* ref:this, result */
err:
	return -1;
}



/* this, args  ->  result (flags == VOP_F_PUSHRES)
 * this, args  ->  N/A    (flags == VOP_F_NORMAL)
 * Same as `Dee_function_generator_vop()', but arguments are given as via what
 * should be a tuple-object (the type is asserted by this function) in vtop.
 * NOTE: A tuple-type check is only generated if DEE_FUNCTION_ASSEMBLER_F_SAFE is set. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_voptuple(struct Dee_function_generator *__restrict self,
                                uint16_t operator_name, unsigned int flags) {
	struct Dee_memval *argsval;
	DO(Dee_function_generator_vdirect(self, 2));
	DO(Dee_function_generator_vassert_type_exact_if_safe_c(self, &DeeTuple_Type));
	DO(Dee_function_generator_state_unshare(self));
	argsval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(argsval)) {
		/* Inline arguments so we can do regular operator invocation. */
		size_t i;
		DeeTupleObject *args = (DeeTupleObject *)Dee_memval_const_getobj(argsval);
		ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type); /* Could only fail in SAFE code, but there it's already checked-for above */
		DO(Dee_function_generator_vpop(self));          /* this */
		for (i = 0; i < DeeTuple_SIZE(args); ++i) {
			DO(Dee_function_generator_vpush_const(self, DeeTuple_GET(args, i))); /* this, [args...] */
		}
		return Dee_function_generator_vop(self, operator_name, (Dee_vstackaddr_t)DeeTuple_SIZE(args), flags);
	}                                                                          /* this, args */
	DO(Dee_function_generator_vswap(self));                                    /* args, this */
	DO(Dee_function_generator_vnotoneref_if_operator(self, operator_name, 1)); /* args, this */
	DO(Dee_function_generator_vpush_imm16(self, operator_name));               /* args, this, operator_name */
	DO(Dee_function_generator_vdup_n(self, 3));                                /* args, this, operator_name, args */
	DO(Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size)));   /* args, this, operator_name, argc */
	DO(Dee_function_generator_vdup_n(self, 4));                                /* args, this, operator_name, argc, args */
	DO(Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* args, this, operator_name, argc, argv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_InvokeOperator, VCALL_CC_RAWINTPTR, 4)); /* args, UNCHECKED(result) */
	DO(Dee_function_generator_vpop_at(self, 2));                               /* UNCHECKED(result) */
	DO(Dee_function_generator_vcheckobj(self));                                /* result */
	if (!(flags & VOP_F_PUSHRES))
		return Dee_function_generator_vpop(self); /* [this] */
	return 0;
err:
	return -1;
}


/* [ref]:this, args  ->  [ref]:this, result (flags == VOP_F_PUSHRES)
 * [ref]:this, args  ->  [ref]:this         (flags == VOP_F_NORMAL)
 * NOTE: If "this" isn't a constant, it is the caller's responsibility to
 *       ensure that "this" doesn't have any aliases. Otherwise, aliases
 *       might inadvertently also receive the updated object. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vinplaceoptuple(struct Dee_function_generator *__restrict self,
                                       uint16_t operator_name, unsigned int flags) {
	struct Dee_memval *argsval;
	DO(Dee_function_generator_vdirect(self, 2));
	DO(Dee_function_generator_vassert_type_exact_if_safe_c(self, &DeeTuple_Type));
	DO(Dee_function_generator_state_unshare(self));
	argsval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(argsval)) {
		/* Inline arguments so we can do regular operator invocation. */
		size_t i;
		DeeTupleObject *args = (DeeTupleObject *)Dee_memval_const_getobj(argsval);
		ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type); /* Could only fail in SAFE code, but there it's already checked-for above */
		DO(Dee_function_generator_vpop(self));          /* [ref]:this */
		for (i = 0; i < DeeTuple_SIZE(args); ++i)
			DO(Dee_function_generator_vpush_const(self, DeeTuple_GET(args, i))); /* [ref]:this, [args...] */
		return Dee_function_generator_vinplaceop(self, operator_name, (Dee_vstackaddr_t)DeeTuple_SIZE(args), flags);
	}                                                                          /* [ref]:this, args */
	DO(Dee_function_generator_vswap(self));                                    /* args, [ref]:this */
	DO(Dee_function_generator_vref_noalias(self));                             /* args, ref:this */
	DO(Dee_function_generator_vnotoneref_if_operator(self, operator_name, 1)); /* args, ref:this */
	DO(Dee_function_generator_vlinear(self, 1, false));                        /* args, ref:this, p_this */
	DO(Dee_function_generator_vpush_imm16(self, operator_name));               /* args, ref:this, p_this, operator_name */
	DO(Dee_function_generator_vdup_n(self, 4));                                /* args, ref:this, p_this, operator_name, args */
	DO(Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size)));   /* args, ref:this, p_this, operator_name, argc */
	DO(Dee_function_generator_vdup_n(self, 5));                                /* args, ref:this, p_this, operator_name, argc, args */
	DO(Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem))); /* args, ref:this, p_this, operator_name, argc, argv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_PInvokeOperator, VCALL_CC_RAWINTPTR, 4)); /* args, ref:this, UNCHECKED(result) */
	DO(Dee_function_generator_vpop_at(self, 3));                               /* ref:this, UNCHECKED(result) */
	DO(Dee_function_generator_vcheckobj(self));                                /* ref:this, result */
	if (!(flags & VOP_F_PUSHRES))
		return Dee_function_generator_vpop(self); /* ref:this */
	return 0;                                     /* ref:this, result */
err:
	return -1;
}


/* seq, size -> seq */
INTERN WUNUSED NONNULL((1)) int DCALL
vassert_unpack_size(struct Dee_function_generator *__restrict self,
                    size_t expected_size) {
	DREF struct Dee_memstate *saved_state;
	struct Dee_host_section *text, *cold;
	struct Dee_memval *sizeval;
	DO(Dee_function_generator_vdelta(self, -(ptrdiff_t)expected_size)); /* seq, size-expected_size */
	sizeval = Dee_function_generator_vtop(self);
	if (Dee_memval_iszero(sizeval)) /* Special case: size is constant and matches expected value. */
		return Dee_function_generator_vpop(self);
	text = self->fg_sect;
	cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if (cold == text || Dee_memval_isconst(sizeval)) {
		struct Dee_host_symbol *Lsize_is_correct;
		Lsize_is_correct = Dee_function_generator_newsym_named(self, ".Lsize_is_correct");
		if unlikely(!Lsize_is_correct)
			goto err;
		DO(Dee_function_generator_gjz(self, Dee_function_generator_vtopdloc(self),
		                              Lsize_is_correct)); /* seq, size-expected_size */
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		EDO(err_saved_state, Dee_function_generator_vdelta(self, (ptrdiff_t)expected_size)); /* seq, size */
		EDO(err_saved_state, Dee_function_generator_vpush_immSIZ(self, expected_size));      /* seq, size, expected_size */
		EDO(err_saved_state, Dee_function_generator_vswap(self));                            /* seq, expected_size, size */
		EDO(err_saved_state, Dee_function_generator_vcallapi(self, &libhostasm_err_invalid_unpack_size, VCALL_CC_EXCEPT, 3));
		Dee_memstate_decref(self->fg_state);
		self->fg_state = saved_state;
		Dee_host_symbol_setsect(Lsize_is_correct, self->fg_sect);
	} else {
		struct Dee_host_symbol *Lerr_invalid_unpack_size;
		Lerr_invalid_unpack_size = Dee_function_generator_newsym_named(self, ".Lerr_invalid_unpack_size");
		if unlikely(!Lerr_invalid_unpack_size)
			goto err;
		DO(Dee_function_generator_gjnz(self, Dee_function_generator_vtopdloc(self),
		                               Lerr_invalid_unpack_size)); /* seq, size-expected_size */
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		self->fg_sect = cold;
		Dee_host_symbol_setsect(Lerr_invalid_unpack_size, self->fg_sect);
		EDO(err_saved_state, Dee_function_generator_vdelta(self, (ptrdiff_t)expected_size)); /* seq, size */
		EDO(err_saved_state, Dee_function_generator_vpush_immSIZ(self, expected_size));      /* seq, size, expected_size */
		EDO(err_saved_state, Dee_function_generator_vswap(self));                            /* seq, expected_size, size */
		EDO(err_saved_state, Dee_function_generator_vcallapi(self, &libhostasm_err_invalid_unpack_size, VCALL_CC_EXCEPT, 3));
		Dee_memstate_decref(self->fg_state);
		self->fg_state = saved_state;
		HA_printf(".section .text\n");
		self->fg_sect = text;
	}                                         /* seq, size-expected_size */
	return Dee_function_generator_vpop(self); /* seq */
err_saved_state:
	Dee_memstate_decref(saved_state);
err:
	return -1;
}

/* self, index -> CHECKED(elem) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
vcall_DeeFastSeq_GetItem(struct Dee_function_generator *__restrict self,
                         struct Dee_type_nsi const *__restrict nsi) {
	DREF struct Dee_memstate *saved_state;
	ASSERT(nsi->nsi_class == TYPE_SEQX_CLASS_SEQ);
	if (nsi->nsi_seqlike.nsi_getitem_fast) {
		struct Dee_host_section *text, *cold;
		DO(Dee_function_generator_vdup_n(self, 2)); /* self, index, self */
		DO(Dee_function_generator_vdup_n(self, 2)); /* self, index, self, index */
		DO(Dee_function_generator_vcallapi(self, nsi->nsi_seqlike.nsi_getitem_fast,
		                                   VCALL_CC_RAWINTPTR, 2)); /* self, index, UNCHECKED(elem) */
		ASSERT(Dee_function_generator_vtop_isdirect(self));
		text = self->fg_sect;
		cold = &self->fg_block->bb_hcold;
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
			cold = text;
		if (cold == text) {
			struct Dee_host_symbol *Lis_bound;
			Lis_bound = Dee_function_generator_newsym_named(self, ".Lis_bound");
			if unlikely(!Lis_bound)
				goto err;
			DO(Dee_function_generator_gjnz(self, Dee_function_generator_vtopdloc(self), Lis_bound)); /* self, index, UNCHECKED(elem) */
			saved_state = self->fg_state;
			Dee_memstate_incref(saved_state);
			EDO(err_saved_state, Dee_function_generator_vrrot(self, 3)); /* UNCHECKED(elem), self, index */
			EDO(err_saved_state, Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_index, VCALL_CC_EXCEPT, 2));
			Dee_memstate_decref(self->fg_state);
			self->fg_state = saved_state;
			Dee_host_symbol_setsect(Lis_bound, self->fg_sect);
		} else {
			struct Dee_host_symbol *Lerr_unbound_index;
			Lerr_unbound_index = Dee_function_generator_newsym_named(self, ".Lerr_unbound_index");
			if unlikely(!Lerr_unbound_index)
				goto err;
			DO(Dee_function_generator_gjz(self, Dee_function_generator_vtopdloc(self),
			                              Lerr_unbound_index)); /* self, index, UNCHECKED(elem) */
			saved_state = self->fg_state;
			Dee_memstate_incref(saved_state);
			HA_printf(".section .cold\n");
			self->fg_sect = cold;
			Dee_host_symbol_setsect(Lerr_unbound_index, self->fg_sect);
			EDO(err_saved_state, Dee_function_generator_vrrot(self, 3)); /* UNCHECKED(elem), self, index */
			EDO(err_saved_state, Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_index, VCALL_CC_EXCEPT, 2));
			Dee_memstate_decref(self->fg_state);
			self->fg_state = saved_state;
			HA_printf(".section .text\n");
			self->fg_sect = text;
		}                                          /* self, index, elem */
		DO(Dee_function_generator_vrrot(self, 3)); /* elem, self, index */
		DO(Dee_function_generator_vpop(self));     /* elem, self */
		DO(Dee_function_generator_vpop(self));     /* elem */
		ASSERT(Dee_function_generator_vtop_isdirect(self));
		Dee_function_generator_vtop_direct_setref(self);
		return 0;
	}
	ASSERT(nsi->nsi_seqlike.nsi_getitem);
	return Dee_function_generator_vcallapi(self, nsi->nsi_seqlike.nsi_getitem,
	                                       VCALL_CC_OBJECT, 2);
err_saved_state:
	Dee_memstate_decref(saved_state);
err:
	return -1;
}


/* seq -> [elems...] */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopunpack(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t n) {
	struct Dee_memval *seqval;
	DeeTypeObject *seqtype;
	Dee_vstackaddr_t i;
	Dee_cfa_t cfa_offset;
	size_t alloc_size;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	seqval = Dee_function_generator_vtop(self);

	/* Optimization when "vtop" is always "none" */
	seqtype = Dee_memval_typeof(seqval);
	if (seqtype != NULL) {
		if (seqtype == &DeeNone_Type) {
			DO(Dee_function_generator_vpop(self));
			for (i = 0; i < n; ++i)
				DO(Dee_function_generator_vpush_none(self));
			return 0;
		}
		if (Dee_memval_isconst(seqval) &&
		    DeeType_IsOperatorConstexpr(seqtype, OPERATOR_SEQ_ENUMERATE)) {
			/* Optimizations to inline-expand a constant expression */
			DREF DeeObject **elemv;
			DeeObject *seqobj = Dee_memval_const_getobj(seqval);
			elemv = (DREF DeeObject **)Dee_Mallocac(n, sizeof(DREF DeeObject *));
			if unlikely(!elemv)
				goto err;
			if likely(DeeObject_Unpack(seqobj, n, elemv) == 0) {
				int temp = Dee_function_generator_vpop(self);
				if likely(temp == 0) {
					for (i = 0; i < n; ++i) {
						DeeObject *elem = Dee_function_generator_inlineref(self, elemv[i]);
						temp = unlikely(!elem) ? -1 : Dee_function_generator_vpush_const(self, elem);
						if unlikely(temp) {
							++i;
							Dee_Decrefv(elemv + i, n - i);
							break;
						}
					}
				}
				Dee_Freea(elemv);
				return temp;
			}
			Dee_Freea(elemv);
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR))
				return -1;
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
		}

		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
			if (seqtype == &DeeTuple_Type) {
				/* Verify that the tuple has the correct size */
				DO(Dee_function_generator_vdup(self));                                   /* seq, seq */
				DO(Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size))); /* seq, t_size */
				DO(vassert_unpack_size(self, n));                                        /* seq */
#ifdef HOSTASM_STACK_GROWS_DOWN
				for (i = n; i--;)
#else /* HOSTASM_STACK_GROWS_DOWN */
				for (i = 0; i < n; ++i)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				{
#ifdef HOSTASM_STACK_GROWS_DOWN
					bool is_last = i == 0;
					Dee_vstackaddr_t n_pushed = n - (i + 1);
#else /* HOSTASM_STACK_GROWS_DOWN */
					bool is_last = i == n - 1;
					Dee_vstackaddr_t n_pushed = i;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
					/* TODO: If element "i" isn't used at all (is just popped), then push none for it. */
					if (!is_last) {                                            /* seq, [items...] */
						DO(Dee_function_generator_vdup_n(self, n_pushed + 1)); /* seq, [items...], seq */
					} else {
						DO(Dee_function_generator_vlrot(self, n_pushed + 1)); /* [items...], seq */
					}                                                         /* [seq], [items...], seq */
					DO(Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_elem) +
					                                     i * sizeof(DREF DeeObject *))); /* [items...], elem */
					/* TODO: Don't incref if:
					 * - Reference to tuple "seq" has longer lifetime than reference to current element
					 * - Current element doesn't need to be a reference (is used by "===" or "is none", etc.)
					 * - The value in question can be a constant. */
					DO(Dee_function_generator_vref_noconst_noalias(self)); /* [items...], ref:elem */
				}
				if (n == 0)
					return Dee_function_generator_vpop(self);
				return Dee_function_generator_vmirror(self, n);
			}

			if (seqtype == &DeeList_Type) {
				/* TODO: Acquire a lock to he list (if the list isn't ONEREF) */
				/* TODO: Verify that the list has the correct size */
				/* TODO: Push all of the list's elements (starting with the greatest index when `HOSTASM_STACK_GROWS_DOWN') */
				/* TODO: Incref every element where the consumer needs it to be a reference */
				/* TODO: Release the lock from the list (if the list isn't ONEREF) */
			}

			/* If "seqtype" implements "nsi_getsize_fast" (DeeFastSeq_GetSize),
			 * inline the relevant code from `DeeObject_Unpack()' */
			if (seqtype->tp_seq &&
			    seqtype->tp_seq->tp_nsi &&
			    seqtype->tp_seq->tp_nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    seqtype->tp_seq->tp_nsi->nsi_seqlike.nsi_getsize_fast) {
				struct Dee_type_nsi const *nsi = seqtype->tp_seq->tp_nsi;
				ASSERT(nsi->nsi_seqlike.nsi_getitem ||
				       nsi->nsi_seqlike.nsi_getitem_fast);

				/* NOTE: Here, we're *NOT* free to load in arbitrary order!
				 *       If more than one element is unbound / causes an exception,
				 *       then the actual error needs to be for the *first* (lowest)
				 *       index where access causes a fault! */
				DO(Dee_function_generator_vdup(self)); /* seq, seq */
				DO(Dee_function_generator_vcallapi(self, nsi->nsi_seqlike.nsi_getsize_fast,
				                                   VCALL_CC_RAWINTPTR, 1)); /* seq, size */
				DO(vassert_unpack_size(self, n));                           /* seq */
				for (i = 0; i < n; ++i) {
					/* TODO: For locations that don't end up being used, simply push none:
					 * >> local a, none, b = foo.partition(",")...;
					 *
					 * ASM:
					 * >> push   @foo
					 * >> push   @","
					 * >> callattr top, @"partition", #1
					 * >> unpack pop, #3    // Don't actually need to load second value
					 * >> pop    local @b
					 * >> pop               // Second value is discarded without being used
					 * >> pop    local @a
					 */
					if (i == n - 1) {
						DO(Dee_function_generator_vlrot(self, n));      /* [items...], seq */
					} else {
						DO(Dee_function_generator_vdup_n(self, i + 1)); /* seq, [items...], seq */
					}
					DO(Dee_function_generator_vpush_immSIZ(self, i));   /* [seq], [items...], seq, i */
					DO(vcall_DeeFastSeq_GetItem(self, nsi));            /* [seq], [items...], elem */
					DO(Dee_function_generator_vdirect1(self));          /* [seq], [items...], elem */ /* TODO: Remove me once single-NULLABLE check is done */
				}                                                       /* [seq], [items...] */
				if (n == 0)
					return Dee_function_generator_vpop(self);
				return 0;
			}
		}
	}

	alloc_size = n * sizeof(DREF DeeObject *);
	cfa_offset = Dee_memstate_hstack_find(self->fg_state, self->fg_state_hstack_res, alloc_size);
	if (cfa_offset == (Dee_cfa_t)-1) {
		DO(Dee_function_generator_ghstack_adjust(self, alloc_size));
		cfa_offset = self->fg_state->ms_host_cfa_offset;
#ifndef HOSTASM_STACK_GROWS_DOWN
		cfa_offset -= alloc_size;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	}
	for (i = 0; i < n; ++i) {
		Dee_cfa_t n_cfa_offset;
#ifdef HOSTASM_STACK_GROWS_DOWN
		n_cfa_offset = cfa_offset - i * sizeof(DREF DeeObject *);
#else /* HOSTASM_STACK_GROWS_DOWN */
		n_cfa_offset = cfa_offset + i * sizeof(DREF DeeObject *);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		DO(Dee_function_generator_vpush_hstackind(self, n_cfa_offset, 0));
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	}                                                                              /* seq, [elems...] */
	DO(Dee_function_generator_vlrot(self, n + 1));                                 /* [elems...], seq */
	DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 1)); /* [elems...], seq */
	DO(Dee_function_generator_vpush_immSIZ(self, n));                              /* [elems...], seq, objc */
	DO(Dee_function_generator_vpush_hstack(self, cfa_offset));                     /* [elems...], seq, objc, objv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_Unpack, VCALL_CC_INT, 3)); /* [elems...] */
	for (i = 0; i < n; ++i) {
		ASSERT(!Dee_memval_direct_isref(Dee_function_generator_vtop(self) - i));
		Dee_memval_direct_setref(Dee_function_generator_vtop(self) - i);
	} /* [ref:elems...] */
	return 0;
err:
	return -1;
}


/* lhs, rhs -> result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopconcat(struct Dee_function_generator *__restrict self) {
	void const *concat_inherited_api_function;
	DeeTypeObject *lhs_type, *rhs_type;
	DO(Dee_function_generator_vdirect(self, 2)); /* lhs, rhs */
	lhs_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - 1);
	rhs_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - 0);
	/* Optimizations for known object types (see impl of `DeeObject_ConcatInherited()'). */
	if (lhs_type == &DeeTuple_Type) {
		concat_inherited_api_function = (void const *)&DeeTuple_ConcatInherited;
	} else if (lhs_type == &DeeList_Type) {
		concat_inherited_api_function = (void const *)&DeeList_ConcatInherited;
	} else if (lhs_type != NULL && (rhs_type != NULL && rhs_type != &DeeTuple_Type)) {
		/* Fallback: perform an arithmetic add operation if we know
		 * `DeeObject_ConcatInherited()' won't give us any advantages. */
		return Dee_function_generator_vop(self, OPERATOR_ADD, 2, VOP_F_PUSHRES);
	} else {
		concat_inherited_api_function = (void const *)&DeeObject_ConcatInherited;
	}
	DO(Dee_function_generator_vswap(self));            /* rhs, lhs */
	DO(Dee_function_generator_vref2(self, 2));         /* rhs, ref:lhs */
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* rhs, ref:lhs */
	DO(Dee_function_generator_vswap(self));            /* ref:lhs, rhs */
	if (concat_inherited_api_function == (void const *)&DeeObject_ConcatInherited)
		DO(Dee_function_generator_vnotoneref_at(self, 1)); /* ref:lhs, rhs */
	DO(Dee_function_generator_vdup_n(self, 2));        /* ref:lhs, rhs, lhs */
	DO(Dee_function_generator_vswap(self));            /* ref:lhs, lhs, rhs */
	DO(Dee_function_generator_vcallapi(self, concat_inherited_api_function, VCALL_CC_RAWINTPTR, 2)); /* [ref]:lhs, UNCHECKED(result) */
	ASSERT(Dee_function_generator_vtop_isdirect(self));  /* [ref]:lhs, UNCHECKED(result) */
	DO(Dee_function_generator_gjz_except(self, Dee_function_generator_vtopdloc(self)));  /* noref:lhs, UNCHECKED(result) */
	if (concat_inherited_api_function != (void const *)&DeeObject_ConcatInherited)
		DO(Dee_function_generator_voneref_noalias(self));
	DO(Dee_function_generator_vswap(self)); /* result, noref:lhs */
	ASSERT(Dee_function_generator_vtop_direct_isref(self));
	Dee_function_generator_vtop_direct_clearref(self); /* result, lhs */
	return Dee_function_generator_vpop(self);          /* result */
err:
	return -1;
}

/* seq, [elems...] -> seq */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopextend(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	void const *extend_inherited_api_function;
	DeeTypeObject *seq_type;
	uint16_t old_seqflags;
	for (i = 0; i < n; ++i) {
		/* TODO: Don't create references here if `DeeSharedVector_NewShared()' gets used below! */
		DO(Dee_function_generator_vlrot(self, n));
		DO(Dee_function_generator_vref2(self, n + 1));
	}
	seq_type = Dee_memval_typeof(Dee_function_generator_vtop(self) - n);
	if (seq_type != &DeeTuple_Type)
		DO(Dee_function_generator_vnotoneref(self, n)); /* seq, [ref:elems...] */
	DO(Dee_function_generator_vlinear(self, n, true));  /* seq, [ref:elems...], elemv */
	DO(Dee_function_generator_vlrot(self, n + 2));      /* [ref:elems...], elemv, seq */

	/* Optimizations for known object types (see impl of `DeeObject_ExtendInherited()'). */
	extend_inherited_api_function = (void const *)&DeeObject_ExtendInherited;
	if (seq_type == NULL) {
		/* Use default API function. */
	} else if (seq_type == &DeeTuple_Type) {
		extend_inherited_api_function = (void const *)&DeeTuple_ExtendInherited;
	} else if (seq_type == &DeeList_Type) {
		extend_inherited_api_function = (void const *)&DeeList_ExtendInherited;
	} else if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Inline the fallback from `DeeObject_ExtendInherited()' */
		struct Dee_function_exceptinject_callvoidapi ij;
		/* TODO: Use `DeeSharedVector_Decref()' if more than half of "elemv" are references. */
		void const *api_decref = (void const *)&DeeSharedVector_DecrefNoGiftItems;
		DO(Dee_function_generator_vpush_immSIZ(self, n));                                          /* [elems...], elemv, seq, elemc */
		DO(Dee_function_generator_vlrot(self, 3));                                                 /* [elems...], seq, elemc, elemv */
		DO(Dee_function_generator_vcallapi(self, &DeeSharedVector_NewShared, VCALL_CC_OBJECT, 2)); /* [elems...], seq, svec */
		DO(Dee_function_generator_vdirect1(self));                                                 /* [elems...], seq, svec */
		DO(Dee_function_generator_vsettyp_noalias(self, &DeeSharedVector_Type));                   /* [elems...], seq, svec */
		Dee_function_generator_vtop_direct_clearref(self);                                         /* [elems...], seq, svec */
		DO(Dee_function_generator_vswap(self));                                                    /* [elems...], svec, seq */
		Dee_function_generator_xinject_push_callvoidapi(self, &ij, api_decref, 1);                 /* [elems...], svec, seq */
		ij.fei_cva_base.fei_stack -= 1;                                                            /* [elems...], svec, seq */
		if (api_decref == (void const *)&DeeSharedVector_Decref) {
			struct Dee_memval *elemv = self->fg_state->ms_stackv + self->fg_state->ms_stackc - (2 + n);
			for (i = 0; i < n; ++i) {
				ASSERT(Dee_memval_isdirect(&elemv[i]));
				ASSERT(Dee_memval_direct_isref(&elemv[i]));
				Dee_memval_direct_clearref(&elemv[i]); /* Stolen by the shared vector */
			}
		}
		DO(Dee_function_generator_vdup_n(self, 2));                                 /* [elems...], svec, seq, svec */
		DO(Dee_function_generator_vop(self, OPERATOR_ADD, 2, VOP_F_PUSHRES));       /* [elems...], svec, result */
		Dee_function_generator_xinject_pop_callvoidapi(self, &ij);                  /* [elems...], svec, result */
		DO(Dee_function_generator_vswap(self));                                     /* [elems...], result, svec */
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));                    /* [elems...], result, svec */
		DO(Dee_function_generator_vcallapi(self, api_decref, VCALL_CC_VOID_NX, 1)); /* [elems...], result */
		goto rotate_result_and_pop_elems;
	}
	DO(Dee_function_generator_vdirect1(self));         /* [elems...], elemv, seq */
	DO(Dee_function_generator_vref2(self, n + 2));     /* [elems...], elemv, ref:seq */
	if (extend_inherited_api_function == (void const *)&DeeObject_ExtendInherited)
		DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_ADD, 1)); /* [elems...], elemv, ref:seq */
	DO(Dee_function_generator_vpush_immSIZ(self, n));  /* [elems...], elemv, ref:seq, elemc */
	DO(Dee_function_generator_vlrot(self, 3));         /* [elems...], ref:seq, elemc, elemv */
	old_seqflags = Dee_memval_direct_getobj(&Dee_function_generator_vtop(self)[-2])->mo_flags;
	DO(Dee_function_generator_vcallapi_ex(self, extend_inherited_api_function, VCALL_CC_RAWINTPTR, 3, 0)); /* [[valid_if(!result)] elems...], [valid_if(!result)] ref:seq, elemc, elemv, UNCHECKED(result) */
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	DO(Dee_function_generator_gjz_except(self, Dee_function_generator_vtopdloc(self))); /* [[valid_if(false)] elems...], [valid_if(false)] ref:seq, elemc, elemv, result */
	Dee_function_generator_vtop_direct_setref(self);   /* [[valid_if(false)] elems...], [valid_if(false)] ref:seq, elemc, elemv, ref:result */
	if (extend_inherited_api_function != (void const *)&DeeObject_ExtendInherited)
		Dee_memval_direct_getobj(Dee_function_generator_vtop(self))->mo_flags |= n ? MEMOBJ_F_ONEREF : (old_seqflags & MEMOBJ_F_ONEREF);
	DO(Dee_function_generator_vrrot(self, 4));         /* [[valid_if(false)] elems...], ref:result, [valid_if(false)] ref:seq, elemc, elemv */
	DO(Dee_function_generator_vpop(self));             /* [[valid_if(false)] elems...], ref:result, [valid_if(false)] ref:seq, elemc */
	DO(Dee_function_generator_vpop(self));             /* [[valid_if(false)] elems...], ref:result, [valid_if(false)] ref:seq */
	Dee_function_generator_vtop_direct_clearref(self); /* [[valid_if(false)] elems...], ref:result, [valid_if(false)] seq */
	DO(Dee_function_generator_vpop(self));             /* [[valid_if(false)] elems...], ref:result */
rotate_result_and_pop_elems:                           /* [[valid_if(false)] elems...], ref:result */
	DO(Dee_function_generator_vrrot(self, n + 1));     /* ref:result, [[valid_if(false)] elems...] */
	for (i = 0; i < n; ++i) {
		/* In the success-case, references were inherited! */
		ASSERT(Dee_function_generator_vtop_direct_isref(self));
		Dee_function_generator_vtop_direct_clearref(self);
		DO(Dee_function_generator_vpop(self));
	}
	return 0; /* ref:result */
err:
	return -1;
}

/* ob -> type(ob) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_voptypeof(struct Dee_function_generator *__restrict self, bool ref) {
	struct Dee_memval *objval;
	DeeTypeObject *known_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));

	/* Check if the object type is known to be a constant. */
	objval = Dee_function_generator_vtop(self);
	known_type = Dee_memval_typeof(objval);
	if (known_type != NULL) {
		DO(Dee_function_generator_vpop(self));
		return Dee_function_generator_vpush_const(self, known_type);
	}

	/* If the object whose type we're trying to read is a
	 * reference, then we also need a reference to the type! */
	DO(Dee_function_generator_vdirect1(self));                           /* obj */
	DO(Dee_function_generator_vdup(self));                               /* obj, obj */
	DO(Dee_function_generator_vind(self, offsetof(DeeObject, ob_type))); /* obj, obj->ob_type */
	if (ref)
		DO(Dee_function_generator_vdep(self));      /* obj, obj->ob_type */
	return Dee_function_generator_vpop_at(self, 2); /* obj->ob_type */
err:
	return -1;
}

/* ob -> ob.class */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopclassof(struct Dee_function_generator *__restrict self, bool ref) {
	struct Dee_memval *objval;
	DeeTypeObject *known_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));

	/* Check if the object type is known to be a constant. */
	objval = Dee_function_generator_vtop(self);
	known_type = Dee_memval_typeof(objval);
	if (known_type != NULL) {
		/* Special case: for super, we must return the embedded type */
		if (known_type == &DeeSuper_Type)
			return Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_type));
		DO(Dee_function_generator_vpop(self));
		return Dee_function_generator_vpush_const(self, known_type);
	}

	/* TODO: Inline call to `DeeObject_Class()' */
	DO(Dee_function_generator_vcallapi_ex(self, &DeeObject_Class, VCALL_CC_RAWINTPTR, 1, 0)); /* obj, obj.class */
	if (ref)
		DO(Dee_function_generator_vdep(self));      /* obj, obj.class */
	return Dee_function_generator_vpop_at(self, 2); /* obj.class */
err:
	return -1;
}


/* ob -> ob.super */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsuperof(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *objval;
	DeeTypeObject *known_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));

	/* Check if the object type is known to be a constant. */
	objval = Dee_function_generator_vtop(self);
	known_type = Dee_memval_typeof(objval);
	if (known_type != NULL) {
		if (known_type == &DeeSuper_Type) {
			if (objval->mv_vmorph == MEMVAL_VMORPH_SUPER) {
				struct Dee_memobjs *objs = Dee_memval_getobjn(objval);
				/* TODO: Optimization */
				(void)objs;
			}
		} else {
			DeeTypeObject *base = DeeType_Base(known_type);
			if (base) {                                             /* ob */
				DO(Dee_function_generator_vpush_const(self, base)); /* ob, base */
				return Dee_function_generator_vopsuper(self);       /* ob.super */
			}
		}
	}

	/* Fallback: do the super operation at runtime. */
	DO(Dee_function_generator_vnotoneref(self, 2));
	DO(Dee_function_generator_vcallapi(self, &DeeSuper_Of, VCALL_CC_OBJECT, 1));
	DO(Dee_function_generator_voneref_noalias(self));
	return Dee_function_generator_vsettyp_noalias(self, &DeeSuper_Type);
err:
	return -1;
}

/* ob, type -> ob as type  (doesn't do type checking) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vopsuper_impl_unchecked(struct Dee_function_generator *__restrict self) {
	struct Dee_memobjs *objs;
	struct Dee_memval *p_ob;
	objs = Dee_memobjs_new(2);
	if unlikely(!objs)
		goto err;
	p_ob = Dee_function_generator_vtop(self) - 1;
	Dee_memobj_initmove(&objs->mos_objv[0], &p_ob[0].mv_obj.mvo_0); /* Inherit */
	Dee_memobj_initmove(&objs->mos_objv[1], &p_ob[1].mv_obj.mvo_0); /* Inherit */
	--self->fg_state->ms_stackc;
	Dee_memval_init_objn_inherited(p_ob, objs, MEMVAL_VMORPH_SUPER, MEMVAL_F_NORMAL);
	return 0;
err:
	return -1;
}

/* ob_self, ob_type, type -> ob_self as type  (assuming that "ob_self.class === ob_type") */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vopsuper_impl(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *p_type;
	struct Dee_function_generator_branch branch;

	/* Must generate code to ensure that "type.isabstract || ob_type.derivedfrom(type)" */
	p_type = Dee_function_generator_vtop(self);
	if (Dee_memval_isconst(p_type)) {
		DeeTypeObject *type;
		struct Dee_memval *ob_type;
		type = (DeeTypeObject *)Dee_memval_const_getobj(p_type);
		if (DeeType_IsAbstract(type))
			goto do_pop_ob_type;
		ob_type = Dee_function_generator_vtop(self) - 1;
		if (Dee_memval_isconst(ob_type)) {
			DeeTypeObject *ob_type_c;
			ob_type_c = (DeeTypeObject *)Dee_memval_const_getobj(ob_type);
			if (!DeeType_InheritsFrom(ob_type_c, type)) {
				DO(Dee_function_generator_vpop_at(self, 2));          /* ob_self, type */
				DO(Dee_function_generator_vassert_type_failed(self)); /* N/A */
				return Dee_function_generator_vpush_undefined(self);  /* result */
			}
			goto do_pop_ob_type;
		}                                                                  /* ob_self, ob_type, type */
		DO(Dee_function_generator_vswap(self));                            /* ob_self, type, ob_type */
		DO(Dee_function_generator_vdup_n(self, 2));                        /* ob_self, type, ob_type, type */
		DO(Dee_function_generator_vtype_inheritsfrom(self));               /* ob_self, type, inherited */
		DO(Dee_function_generator_vjz_enter_unlikely(self, &branch));      /* ob_self, type */
		EDO(err_branch, Dee_function_generator_vassert_type_failed(self)); /* N/A */
		DO(Dee_function_generator_vjz_leave_noreturn(self, &branch));      /* ob_self, type */
	} else {
		/* TODO: Must generate runtime code for:
		 * >> if (!DeeType_IsAbstract(type) && !DeeType_InheritsFrom(ob_type, type)) {
		 * >>     DeeObject_TypeAssertFailed(ob_self, type);
		 * >>     HANDLE_EXCEPT();
		 * >> } */
do_pop_ob_type:                                      /* ob_self, ob_type, type */
		DO(Dee_function_generator_vpop_at(self, 2)); /* ob_self, type */
	}                                                /* ob_self, type */
	return vopsuper_impl_unchecked(self);            /* result */
err_branch:
	Dee_function_generator_branch_fini(&branch);
err:
	return -1;
}

/* ob, type -> ob as type */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsuper(struct Dee_function_generator *__restrict self) {
	struct Dee_function_generator_branch branch;
	struct Dee_memval *p_ob;
	DeeTypeObject *ob_type;
	DO(Dee_function_generator_vassert_type_c(self, &DeeType_Type));

	/* Unpack "ob" into its "self" and "type" components */
	p_ob    = Dee_function_generator_vtop(self) - 1;
	ob_type = Dee_memval_typeof(p_ob);
	if (ob_type == &DeeSuper_Type) {
		DO(Dee_function_generator_vswap(self));                                  /* type, ob */
		DO(Dee_function_generator_vdup(self));                                   /* type, ob, ob */
		DO(Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_self))); /* type, ob, ob->s_self */
		DO(Dee_function_generator_vdep(self));                                   /* type, ob, ob->s_self@depends_on(ob) */
		DO(Dee_function_generator_vdup_n(self, 2));                              /* type, ob, ob->s_self, ob */
		DO(Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_type))); /* type, ob, ob->s_self, ob->s_type */
		DO(Dee_function_generator_vlrot(self, 4));                               /* ob, ob->s_self, ob->s_type, type */
		DO(vopsuper_impl(self));                                                 /* ob, result */
		return Dee_function_generator_vpop_at(self, 2);                          /* result */
	} else if (ob_type != NULL) {
		DO(Dee_function_generator_vpush_const(self, ob_type)); /* ob_self, type, ob_type */
		DO(Dee_function_generator_vswap(self));                /* ob_self, ob_type, type */
		return vopsuper_impl(self);                            /* result */
	} else {
		/* The type of "self" is not known -> must emit a runtime check for it being a Super-object.
		 * However, if the intended target type is abstract, then we don't have to generate code to
		 * load the runtime base super-type also (we can just unwrap a super object and re-wrap it
		 * with the intended type) */
		struct Dee_memval *p_type = Dee_function_generator_vtop(self);
		if (Dee_memval_isconst(p_type)) {
			DeeTypeObject *type = (DeeTypeObject *)Dee_memval_const_getobj(p_type);
			if (DeeType_IsAbstract(type)) {
				DO(Dee_function_generator_vpop(self));                                                /* ob */
				DO(Dee_function_generator_vdup(self));                                                /* ob, ob */
				DO(Dee_function_generator_vdup(self));                                                /* ob, ob, ob */
				DO(Dee_function_generator_vind(self, offsetof(DeeObject, ob_type)));                  /* ob, ob, ob->ob_type */
				DO(Dee_function_generator_vdelta(self, -(ptrdiff_t)(uintptr_t)&DeeSuper_Type));       /* ob, ob, ob->ob_type-&DeeSuper_Type */
				DO(Dee_function_generator_vjz_enter(self, &branch));                                  /* ob, ob */
				EDO(err_branch, Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_self))); /* ob, ob->s_self */
				DO(Dee_function_generator_vjz_leave(self, &branch));                                  /* ob, ob_self */
				DO(Dee_function_generator_vdep(self));                                                /* ob, ob_self@depends_on(ob) */
				DO(Dee_function_generator_vpush_const(self, type));                                   /* ob, ob_self@depends_on(ob), type */
				DO(vopsuper_impl_unchecked(self));                                                    /* ob, result */
				return Dee_function_generator_vpop_at(self, 2);                                       /* result */
			}
		}
	}

	/* The type of "self" is not known -> must emit a runtime check for it being a Super-object */
	DO(Dee_function_generator_vdup_n(self, 2));                                           /* ob, type, ob */
	DO(Dee_function_generator_vdup(self));                                                /* ob, type, ob, ob */
	DO(Dee_function_generator_vind(self, offsetof(DeeObject, ob_type)));                  /* ob, type, ob, ob->ob_type */
	DO(Dee_function_generator_vdup(self));                                                /* ob, type, ob, ob->ob_type, ob->ob_type */
	DO(Dee_function_generator_vdelta(self, -(ptrdiff_t)(uintptr_t)&DeeSuper_Type));       /* ob, type, ob, ob->ob_type, ob->ob_type-&DeeSuper_Type */
	DO(Dee_function_generator_vjz_enter(self, &branch));                                  /* ob, type, ob, ob->ob_type */
	EDO(err_branch, Dee_function_generator_vpop(self));                                   /* ob, type, ob */
	EDO(err_branch, Dee_function_generator_vdup(self));                                   /* ob, type, ob, ob */
	EDO(err_branch, Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_self))); /* ob, type, ob, ob->s_self */ /* FIXME: If the super object may be destroyed before "result", must incref "ob->s_self"! */
	EDO(err_branch, Dee_function_generator_vswap(self));                                  /* ob, type, ob->s_self, ob */
	EDO(err_branch, Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_type))); /* ob, type, ob->s_self, ob->s_type */
	DO(Dee_function_generator_vjz_leave(self, &branch));                                  /* ob, type, ob_self, ob_type */
	DO(Dee_function_generator_vlrot(self, 4));                                            /* type, ob_self, ob_type, ob */
	DO(Dee_function_generator_vlrot(self, 3));                                            /* type, ob_type, ob, ob_self */
	DO(Dee_function_generator_vdep(self));                                                /* type, ob_type, ob, ob_self@depends_on(ob) */
	DO(Dee_function_generator_vlrot(self, 3));                                            /* type, ob, ob_self@depends_on(ob), ob_type */
	DO(Dee_function_generator_vlrot(self, 4));                                            /* ob, ob_self@depends_on(ob), ob_type, type */
	DO(vopsuper_impl(self));                                                              /* ob, result */
	return Dee_function_generator_vpop_at(self, 2);                                       /* result */
err_branch:
	Dee_function_generator_branch_fini(&branch);
err:
	return -1;
}

/* @return: 0 : No
 * @return: 1 : Yes
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
is_constexpr_empty_sequence(struct Dee_function_generator *__restrict gen,
                            DeeObject *__restrict self) {
	STATIC_ASSERT((int)true > 0);
	STATIC_ASSERT((int)false == 0);
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (tp_self == &DeeNone_Type)
		return true;
	if (tp_self == &DeeTuple_Type)
		return DeeTuple_IsEmpty(self);
	if (tp_self == &DeeString_Type)
		return DeeString_IsEmpty(self);
	if (tp_self == &DeeSeq_Type)
		return true;
	if (tp_self == &DeeSet_Type)
		return true;
	if (tp_self == &DeeMapping_Type)
		return true;
	if (tp_self == &DeeRoDict_Type)
		return ((DeeRoDictObject *)self)->rd_size == 0;
	if (tp_self == &DeeRoSet_Type)
		return ((DeeRoSetObject *)self)->rs_size == 0;
	if (DeeType_IsOperatorConstexpr(tp_self, OPERATOR_SEQ_ENUMERATE)) {
		/* Construct an iterator to see if the sequence is empty. */
		DREF DeeObject *elem = NULL;
		DREF DeeObject *iter = DeeObject_IterSelf(self);
		if likely(iter) {
			elem = DeeObject_IterNext(iter);
			Dee_Decref(iter);
		}
		if (elem == ITER_DONE) {
			return true;
		} else if (elem) {
			Dee_Decref(elem);
			return false;
		} else if (gen->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
		} else {
			return -1;
		}
	}
	return false;
}

/* Like Dee_function_generator_vopcast(), but return 1
 * if the fallback (1-arg ctor-call) would be used. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vopcast_nofallback(struct Dee_function_generator *__restrict self,
                                          DeeTypeObject *newtype) {
	void const *cast_api_function;
	struct Dee_memval *objval;
	DeeTypeObject *oldtype;

	/* Special handling for certain types. */
	if (newtype == &DeeInt_Type)
		return Dee_function_generator_vopint(self);
	if (newtype == &DeeBool_Type)
		return Dee_function_generator_vopbool(self, VOPBOOL_F_NORMAL);
	if (newtype == &DeeString_Type)
		return Dee_function_generator_vopstr(self);
	if (newtype == &DeeFloat_Type)
		return Dee_function_generator_vop(self, OPERATOR_FLOAT, 1, VOP_F_PUSHRES);

	DO(Dee_function_generator_vdirect1(self));
	objval = Dee_function_generator_vtop(self);
	oldtype = Dee_memval_typeof(objval);
	if (oldtype == newtype) {
		if (Dee_function_generator_isoneref_noalias(self, objval))
			return 0; /* location is already distinct and of the proper type -> no-op */
		if (DeeType_IsOperatorConstexpr(newtype, OPERATOR_COPY))
			return 0; /* Copy doesn't have side-effects, and type already matches -> no-op */
		/* Types already match, but object isn't distinct -> must create a copy. */
		return Dee_function_generator_vop(self, OPERATOR_COPY, 1, VOP_F_PUSHRES);
	}

	/* Optimizations when `obj' is a constant Tuple/HashSet.Frozen/Dict.Frozen.
	 * NOTE: Other object types aren't constexpr, so can't be inlined here! */
	if (Dee_memval_direct_isconst(objval)) {
		DeeObject *obj = Dee_memval_const_getobj(objval);
		if (DeeType_InheritsFrom(newtype, &DeeSeq_Type)) {
			int temp = is_constexpr_empty_sequence(self, obj);
			if (temp) {
				if unlikely(temp < 0)
					goto err;
				/* Special case: casting an empty sequence to another can be optimized to
				 *               produce a default-constructed instance of the target type. */
				DO(Dee_function_generator_vpop(self)); /* N/A */
				return Dee_function_generator_vpackseq(self, newtype, 0);
			}
		}

		if (DeeType_IsOperatorConstexpr(Dee_TYPE(obj), OPERATOR_COPY) &&
		    DeeType_IsOperatorConstexpr(newtype, OPERATOR_COPY)) {
			/* Cast the object at compile-time. */
			DREF DeeObject *cast_result;
			cast_result = DeeObject_New(newtype, 1, &obj);
			if likely(cast_result != NULL) {
				cast_result = Dee_function_generator_inlineref(self, cast_result);
				if unlikely(!cast_result)
					goto err;
				DO(Dee_function_generator_vpop(self));
				return Dee_function_generator_vpush_const(self, cast_result);
			} else if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR) {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			} else {
				goto err;
			}
		}
	}

	/* Special optimizations for certain types of casts. */
	if (oldtype == &DeeRoDict_Type && newtype == &DeeDict_Type) {
		DO(Dee_function_generator_vcallapi(self, &DeeDict_FromRoDict, VCALL_CC_OBJECT, 1)); /* result */
		goto set_vtop_to_type;
	} else if (oldtype == &DeeRoSet_Type && newtype == &DeeHashSet_Type) {
		DO(Dee_function_generator_vcallapi(self, &DeeHashSet_FromRoSet, VCALL_CC_OBJECT, 1)); /* result */
		goto set_vtop_to_type;
	} else if (oldtype == &DeeTuple_Type && newtype == &DeeList_Type) {
		DO(Dee_function_generator_vcallapi(self, &DeeList_FromTuple, VCALL_CC_OBJECT, 1)); /* result */
		goto set_vtop_to_type;
	}

	/* Encode a runtime cast */
	if (newtype == &DeeList_Type) {
		cast_api_function = &DeeList_FromSequence;
	} else if (newtype == &DeeTuple_Type) {
		cast_api_function = &DeeTuple_FromSequence;
	} else if (newtype == &DeeDict_Type) {
		cast_api_function = &DeeDict_FromSequence;
	} else if (newtype == &DeeHashSet_Type) {
		cast_api_function = &DeeHashSet_FromSequence;
	} else if (newtype == &DeeRoDict_Type) {
		cast_api_function = &DeeRoDict_FromSequence;
	} else if (newtype == &DeeRoSet_Type) {
		cast_api_function = &DeeRoSet_FromSequence;
	} else {
		cast_api_function = NULL;
	}
	if (cast_api_function) {
		if (cast_api_function == &DeeTuple_FromSequence && oldtype == &DeeList_Type) {
			/* Special optimization: casting a List to a Tuple can be done
			 *                       most optimally with List.frozen */
			struct type_getset const *list_frozen = DeeList_Type.tp_getsets;
			if likely(list_frozen) {
				while (list_frozen->gs_name && strcmp(list_frozen->gs_name, "frozen") != 0)
					++list_frozen;
				if likely(list_frozen->gs_name && list_frozen->gs_get) {
					DO(Dee_function_generator_vcallapi(self, list_frozen->gs_get, VCALL_CC_OBJECT, 1)); /* result */
					return Dee_function_generator_vsettyp_noalias(self, &DeeTuple_Type);                /* result */
				}
			}
		}
		DO(Dee_function_generator_vnotoneref_if_operator_at(self, OPERATOR_SEQ_ENUMERATE, 1));
		DO(Dee_function_generator_vcallapi(self, cast_api_function, VCALL_CC_OBJECT, 1));
		if (cast_api_function != (void const *)&DeeTuple_FromSequence && /* These casters may re-return the given argument */
		    cast_api_function != (void const *)&DeeRoDict_FromSequence &&
		    cast_api_function != (void const *)&DeeRoSet_FromSequence)
			Dee_function_generator_voneref_noalias(self);
set_vtop_to_type:
		return Dee_function_generator_vsettyp_noalias(self, newtype);
	}

	/* No dedicated optimization available -> tell the caller that the fallback needs to be used. */
	return 1;
err:
	return -1;
}

/* obj -> newtype(obj) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vopcast(struct Dee_function_generator *__restrict self,
                               DeeTypeObject *newtype) {
	int result = Dee_function_generator_vopcast_nofallback(self, newtype);
	if (result > 0) {
		/* Fallback: Do a call-style cast. */
		DO(Dee_function_generator_vpush_const(self, newtype)); /* obj, type */
		DO(Dee_function_generator_vswap(self));                /* type, obj */
		result = Dee_function_generator_vopcall(self, 1);      /* result */
	}
	return result;
err:
	return -1;
}



/* Helpers to perform certain operations. */

/* instance, type -> instance */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcall_DeeObject_Init(struct Dee_function_generator *__restrict self) {
#ifdef CONFIG_TRACE_REFCHANGES
	DO(Dee_function_generator_vpush_NULL(self));                             /* instance, type, NULL */
	DO(Dee_function_generator_vpopind(self, offsetof(DeeObject, ob_trace))); /* instance, type */
#endif /* CONFIG_TRACE_REFCHANGES */
	DO(Dee_function_generator_vref2(self, 2));                               /* instance, ref:type */
	DO(Dee_function_generator_vpopind(self, offsetof(DeeObject, ob_type)));  /* instance */
	DO(Dee_function_generator_vpush_immSIZ(self, 1));                        /* instance, 1 */
	return Dee_function_generator_vpopind(self, offsetof(DeeObject, ob_refcnt)); /* instance */
err:
	return -1;
}



/* Helpers for accessing C-level "struct type_member". NOTE: These don't do type assertions! */

/* this -> value */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_vpush_type_member(struct Dee_function_generator *__restrict self, DeeTypeObject *type,
                                         struct Dee_type_member const *__restrict desc, bool ref) {
	DO(_Dee_function_generator_vpush_type_member(self, desc, ref));
	if (desc->m_doc != NULL && Dee_memval_typeof(Dee_function_generator_vtop(self)) == NULL) {
		struct docinfo doc;
		DeeTypeObject *result_type;
		doc.di_doc = desc->m_doc;
		doc.di_typ = type;
		doc.di_mod = NULL;
		result_type = extra_return_type_from_doc(self, 0, &doc);
		if (result_type != NULL) {
			if unlikely(result_type == (DeeTypeObject *)ITER_DONE)
				goto err;
			return Dee_function_generator_vsettyp_noalias(self, result_type);
		}
	}
	return 0;
err:
	return -1;
}

/* this -> value  (doesn't look at the doc string to determine typing) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_vpush_type_member(struct Dee_function_generator *__restrict self,
                                          struct Dee_type_member const *__restrict desc, bool ref) {
	/* Behavior here mirrors `Dee_type_member_get()' */
	if (TYPE_MEMBER_ISCONST(desc)) {
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, desc->m_const);
	}

	/* Inline the set operation when possible. */
	switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)

	CASE(STRUCT_NONE):
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_none(self);

	CASE(STRUCT_WOBJECT):
	CASE(STRUCT_WOBJECT_OPT): {
		/* XXX: It's possible to inline these! */
		(void)ref;
	}	break;

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_OBJECT_OPT): {
		/* XXX: It's possible to inline these! */
		(void)ref;
	}	break;

	CASE(Dee_STRUCT_BOOL(HOST_SIZEOF_POINTER)): {
		DO(Dee_function_generator_vind(self, desc->m_field.m_offset)); /* FIELD */
		DO(Dee_function_generator_vreg(self, NULL));                   /* reg:FIELD */
		DO(Dee_function_generator_vdirect1(self));                     /* reg:FIELD */
		ASSERT(Dee_function_generator_vtop_isdirect(self));
		Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
		return 0;
	}	break;

	CASE(Dee_STRUCT_INTEGER(HOST_SIZEOF_POINTER)):
	CASE(Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(HOST_SIZEOF_POINTER)): {
		DO(Dee_function_generator_vind(self, desc->m_field.m_offset)); /* FIELD */
		DO(Dee_function_generator_vreg(self, NULL));                   /* reg:FIELD */
		DO(Dee_function_generator_vdirect1(self));                     /* reg:FIELD */
		ASSERT(Dee_function_generator_vtop_isdirect(self));
		Dee_function_generator_vtop(self)->mv_vmorph = (desc->m_field.m_type & Dee_STRUCT_UNSIGNED)
		                                               ? MEMVAL_VMORPH_UINT
		                                               : MEMVAL_VMORPH_INT;
		return 0;
	}	break;

#undef CASE
	default: break;
	}

	/* Fallback: emit a call to `Dee_type_member_get()' */
/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc)); /* this, desc */
	return Dee_function_generator_vcallapi(self, &Dee_type_member_get, VCALL_CC_OBJECT, 2);
err:
	return -1;
}

/* this -> bound */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vbound_type_member(struct Dee_function_generator *__restrict self,
                                          struct Dee_type_member const *__restrict desc) {
	/* Behavior here mirrors `Dee_type_member_bound()' */
	if (TYPE_MEMBER_ISCONST(desc)) {
push_true:
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, Dee_True);
	}
	switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)
	CASE(STRUCT_NONE):
	CASE(STRUCT_OBJECT_OPT): /* Always bound (because it is `none' when NULL) */
	CASE(STRUCT_CSTR_OPT):
	CASE(STRUCT_CSTR_EMPTY):
	CASE(STRUCT_STRING):
	CASE(STRUCT_CHAR):
	CASE(STRUCT_BOOL8):
	CASE(STRUCT_BOOL16):
	CASE(STRUCT_BOOL32):
	CASE(STRUCT_BOOL64):
	CASE(STRUCT_BOOLBIT0):
	CASE(STRUCT_BOOLBIT1):
	CASE(STRUCT_BOOLBIT2):
	CASE(STRUCT_BOOLBIT3):
	CASE(STRUCT_BOOLBIT4):
	CASE(STRUCT_BOOLBIT5):
	CASE(STRUCT_BOOLBIT6):
	CASE(STRUCT_BOOLBIT7):
	CASE(STRUCT_FLOAT):
	CASE(STRUCT_DOUBLE):
	CASE(STRUCT_LDOUBLE):
	CASE(STRUCT_UNSIGNED|STRUCT_INT8):
	CASE(STRUCT_INT8):
	CASE(STRUCT_UNSIGNED|STRUCT_INT16):
	CASE(STRUCT_INT16):
	CASE(STRUCT_UNSIGNED|STRUCT_INT32):
	CASE(STRUCT_INT32):
	CASE(STRUCT_UNSIGNED|STRUCT_INT64):
	CASE(STRUCT_INT64):
	CASE(STRUCT_UNSIGNED|STRUCT_INT128):
	CASE(STRUCT_INT128):
	CASE(STRUCT_WOBJECT_OPT):
		goto push_true;

	CASE(STRUCT_WOBJECT): {
		/* Check if the reference is bound by generating a call to `Dee_weakref_bound()' */
		DO(Dee_function_generator_vdelta(self, desc->m_field.m_offset)); /* &FIELD */
		return Dee_function_generator_vcallapi(self, &Dee_weakref_bound, VCALL_CC_BOOL_NX, 1);
	}	break;

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_CSTR): {
		struct Dee_memval *vtop;
		DO(Dee_function_generator_vind(self, desc->m_field.m_offset)); /* FIELD */
		DO(Dee_function_generator_vreg(self, NULL));                   /* reg:FIELD */
		DO(Dee_function_generator_vdirect1(self));                     /* reg:FIELD */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_TESTNZ(vtop->mv_vmorph);
		return 0;
	}	break;

#undef CASE
	default: break;
	}

	/* Fallback: emit a call to `Dee_type_member_bound()' */
/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc)); /* this, desc */
	DO(Dee_function_generator_vswap(self));            /* desc, this */
	return Dee_function_generator_vcallapi(self, &Dee_type_member_bound, VCALL_CC_BOOL_NX, 2);
err:
	return -1;
}

/* this -> N/A */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vdel_type_member(struct Dee_function_generator *__restrict self,
                                        struct Dee_type_member const *__restrict desc) {
	/* Behavior here mirrors `Dee_type_member_del()' */
	int result = Dee_function_generator_vpush_none(self);
	if likely(result == 0)
		result = Dee_function_generator_vpop_type_member(self, desc);
	return result;
}

/* this, value -> N/A */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpop_type_member(struct Dee_function_generator *__restrict self,
                                        struct Dee_type_member const *__restrict desc) {
	/* Behavior here mirrors `Dee_type_member_set()' */
	if unlikely(TYPE_MEMBER_ISCONST(desc))
		goto fallback;
	if unlikely(desc->m_field.m_type & STRUCT_CONST)
		goto fallback;

	/* XXX: Inline the set operation where possible. */

	/* Fallback: emit a call to `Dee_type_member_set()' */
fallback:
	DO(Dee_function_generator_vpush_addr(self, desc)); /* this, value, desc */
	DO(Dee_function_generator_vswap(self));            /* this, desc, value */
//	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* this, desc, value */ /* There is actually no case where Dee_type_member_set() incref's the given value */
	return Dee_function_generator_vcallapi(self, &Dee_type_member_set, VCALL_CC_INT, 3);
err:
	return -1;
}


/* Helpers for accessing Module-level "struct Dee_module_symbol". */

/* N/A -> value */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vpush_module_symbol(struct Dee_function_generator *__restrict self,
                                           DeeModuleObject *mod, struct Dee_module_symbol const *sym, bool ref) {
	uint16_t symaddr;
	if (sym->ss_flags & Dee_MODSYM_FEXTERN) {
		mod = mod->mo_importv[sym->ss_extern.ss_impid];
		return Dee_function_generator_vpush_mod_global(self, mod, sym->ss_extern.ss_symid, ref);
	}
	symaddr = sym->ss_index;
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		symaddr += Dee_MODULE_PROPERTY_GET;
		ref = true;
	}
	DO(Dee_function_generator_vpush_mod_global(self, mod, symaddr, ref)); /* value */
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY)
		return Dee_function_generator_vopcall(self, 0);
	return 0;
err:
	return -1;
}

/* N/A -> bound */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vbound_module_symbol(struct Dee_function_generator *__restrict self,
                                            DeeModuleObject *mod, struct Dee_module_symbol const *sym) {
	if (sym->ss_flags & Dee_MODSYM_FEXTERN) {
		mod = mod->mo_importv[sym->ss_extern.ss_impid];
		return Dee_function_generator_vbound_mod_global(self, mod, sym->ss_extern.ss_symid);
	}
	if (!(sym->ss_flags & Dee_MODSYM_FPROPERTY))
		return Dee_function_generator_vbound_mod_global(self, mod, sym->ss_index);
	DO(Dee_function_generator_vpush_const(self, mod)); /* mod */
	DO(Dee_function_generator_vpush_addr(self, sym));  /* mod, sym */
	DO(Dee_function_generator_vcallapi(self, &DeeModule_BoundAttrSymbol, VCALL_CC_M1INT, 2));
	DO(Dee_function_generator_vdirect1(self));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* N/A -> N/A */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vdel_module_symbol(struct Dee_function_generator *__restrict self,
                                          DeeModuleObject *mod, struct Dee_module_symbol const *sym) {
	if (!(sym->ss_flags & Dee_MODSYM_FREADONLY)) {
		if (sym->ss_flags & Dee_MODSYM_FEXTERN) {
			mod = mod->mo_importv[sym->ss_extern.ss_impid];
			return Dee_function_generator_vdel_mod_global(self, mod, sym->ss_extern.ss_symid);
		}
		if (self->fg_assembler->fa_flags & Dee_MODSYM_FPROPERTY) {
			DO(Dee_function_generator_vpush_mod_global(self, mod, sym->ss_index + Dee_MODULE_PROPERTY_DEL, true)); /* delete */
			return Dee_function_generator_vopcall(self, 0);
		}
		return Dee_function_generator_vdel_mod_global(self, mod, sym->ss_index);
	}
	DO(Dee_function_generator_vpush_const(self, mod)); /* mod */
	DO(Dee_function_generator_vpush_addr(self, sym));  /* mod, sym */
	return Dee_function_generator_vcallapi(self, &DeeModule_DelAttrSymbol, VCALL_CC_INT, 2);
err:
	return -1;
}

/* value -> - */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vpop_module_symbol(struct Dee_function_generator *__restrict self,
                                          DeeModuleObject *mod, struct Dee_module_symbol const *sym) {
	if (!(sym->ss_flags & Dee_MODSYM_FREADONLY)) {
		if (sym->ss_flags & Dee_MODSYM_FEXTERN) {
			mod = mod->mo_importv[sym->ss_extern.ss_impid];
			return Dee_function_generator_vpop_mod_global(self, mod, sym->ss_extern.ss_symid);
		}
		if (self->fg_assembler->fa_flags & Dee_MODSYM_FPROPERTY) {
			DO(Dee_function_generator_vpush_mod_global(self, mod, sym->ss_index + Dee_MODULE_PROPERTY_SET, true)); /* value, setter */
			DO(Dee_function_generator_vswap(self));                                                                /* setter, value */
			return Dee_function_generator_vopcall(self, 1);
		}
		return Dee_function_generator_vpop_mod_global(self, mod, sym->ss_index);
	}
	DO(Dee_function_generator_vpush_const(self, mod)); /* value, mod */
	DO(Dee_function_generator_vpush_addr(self, sym));  /* value, mod, sym */
	DO(Dee_function_generator_vlrot(self, 3));         /* mod, sym, value */
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* mod, sym, value */
	return Dee_function_generator_vcallapi(self, &DeeModule_SetAttrSymbol, VCALL_CC_INT, 3);
err:
	return -1;
}


/* Helpers for accessing Class-level "struct Dee_class_attribute". NOTE: These don't do type assertions! */

/* this -> value */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vpush_instance_attr(struct Dee_function_generator *__restrict self,
                                           DeeTypeObject *type, struct Dee_class_attribute const *attr, bool ref) {
	unsigned int icmember_flags = ref ? DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF
	                                  : DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL;
	uint16_t field_addr;
	/* Behavior here mirrors `DeeInstance_GetAttribute()' */
	field_addr = attr->ca_addr;
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		field_addr += CLASS_GETSET_GET;
#endif /* CLASS_GETSET_GET != 0 */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		/* Member lies in class memory. */                      /* this */
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD))
			DO(Dee_function_generator_vpop(self));              /* N/A */
		DO(Dee_function_generator_vpush_const(self, type));     /* [this], type */
		DO(Dee_function_generator_vpush_cmember(self, field_addr, icmember_flags)); /* [this], member */
	} else {
		/* Member lies in instance memory. */                   /* this */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			DO(Dee_function_generator_vdup(self));              /* this, this */
		DO(Dee_function_generator_vpush_const(self, type));     /* [this], this, type */
		DO(Dee_function_generator_vpush_imember(self, field_addr, icmember_flags)); /* [this], member */
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {              /* [this], getter */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {          /* this, getter */
			return Dee_function_generator_vopthiscall(self, 0); /* result */
		} else {                                                /* getter */
			return Dee_function_generator_vopcall(self, 0);     /* result */
		}
		__builtin_unreachable();
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) { /* this, func */
		DO(Dee_function_generator_vswap(self));    /* func, this */
		return vnew_InstanceMethod(self);          /* result */
	}
	return 0;
err:
	return -1;
}

/* this -> bound */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vbound_instance_attr(struct Dee_function_generator *__restrict self,
                                            DeeTypeObject *type, struct Dee_class_attribute const *attr) {
	struct class_desc *desc;
	/* Behavior here mirrors `DeeInstance_BoundAttribute()' */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		/* When it isn't a get-set, then we can just check if the class/instance member is bound. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			/* Member lies in class memory. */                  /* this */
			DO(Dee_function_generator_vpop(self));              /* N/A */
			DO(Dee_function_generator_vpush_const(self, type)); /* type */
			return Dee_function_generator_vbound_cmember(self, attr->ca_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
		} else {
			/* Member lies in instance memory. */               /* this */
			DO(Dee_function_generator_vpush_const(self, type)); /* this, type */
			return Dee_function_generator_vbound_imember(self, attr->ca_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
		}
	}
	desc = DeeClass_DESC(type);
	DO(Dee_function_generator_vpush_addr(self, desc));        /* this, desc */
	DO(Dee_function_generator_vswap(self));                   /* desc, this */
	DO(Dee_function_generator_vdup(self));                    /* desc, this, this */
	DO(Dee_function_generator_vdelta(self, desc->cd_offset)); /* desc, this, DeeInstance_DESC(desc, this) */
	DO(Dee_function_generator_vswap(self));                   /* desc, DeeInstance_DESC(desc, this), this */
	DO(Dee_function_generator_vpush_addr(self, attr));        /* desc, DeeInstance_DESC(desc, this), this, attr */
	DO(Dee_function_generator_vnotoneref(self, 2));           /* desc, DeeInstance_DESC(desc, this), this, attr */
	DO(Dee_function_generator_vcallapi(self, &DeeInstance_BoundAttribute, VCALL_CC_M1INT, 4)); /* result */
	DO(Dee_function_generator_vdirect1(self));                /* result */
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* this -> N/A */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vdel_instance_attr(struct Dee_function_generator *__restrict self,
                                          DeeTypeObject *type, struct Dee_class_attribute const *attr) {
	/* Behavior here mirrors `DeeInstance_DelAttribute()' */
	struct class_desc *desc = DeeClass_DESC(type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					DO(Dee_function_generator_vpush_const(self, type)); /* this, type */
					DO(Dee_function_generator_vpush_cmember(self, attr->ca_addr + CLASS_GETSET_DEL,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* this, delete */
					DO(Dee_function_generator_vswap(self));          /* delete, this */
					DO(Dee_function_generator_vopthiscall(self, 0)); /* result */
				} else {
					DO(Dee_function_generator_vpop(self));              /* N/A */
					DO(Dee_function_generator_vpush_const(self, type)); /* type */
					DO(Dee_function_generator_vpush_cmember(self, attr->ca_addr + CLASS_GETSET_DEL,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* delete */
					DO(Dee_function_generator_vopcall(self, 0)); /* result */
				}
				return Dee_function_generator_vpop(self);
			} else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
				/* XXX: Dee_function_generator_vdel_cmember() */
			}
		} else {
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					DO(Dee_function_generator_vdup(self));              /* this, this */
					DO(Dee_function_generator_vpush_const(self, type)); /* this, this, type */
					DO(Dee_function_generator_vpush_imember(self, attr->ca_addr + CLASS_GETSET_DEL,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* this, delete */
					DO(Dee_function_generator_vswap(self));          /* delete, this */
					DO(Dee_function_generator_vopthiscall(self, 0)); /* result */
				} else {
					DO(Dee_function_generator_vpush_const(self, type)); /* this, type */
					DO(Dee_function_generator_vpush_imember(self, attr->ca_addr + CLASS_GETSET_DEL,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* delete */
					DO(Dee_function_generator_vopcall(self, 0)); /* result */
				}
				return Dee_function_generator_vpop(self);
			} else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
				DO(Dee_function_generator_vpush_const(self, type)); /* this, type */
				return Dee_function_generator_vdel_imember(self, attr->ca_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
			}
		}
	}
/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc));        /* this, desc */
	DO(Dee_function_generator_vswap(self));                   /* desc, this */
	DO(Dee_function_generator_vdup(self));                    /* desc, this, this */
	DO(Dee_function_generator_vdelta(self, desc->cd_offset)); /* desc, this, DeeInstance_DESC(desc, this) */
	DO(Dee_function_generator_vswap(self));                   /* desc, DeeInstance_DESC(desc, this), this */
	DO(Dee_function_generator_vpush_addr(self, attr));        /* desc, DeeInstance_DESC(desc, this), this, attr */
	return Dee_function_generator_vcallapi(self, &DeeInstance_DelAttribute, VCALL_CC_INT, 4);
err:
	return -1;
}

/* this, value -> N/A */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vpop_instance_attr(struct Dee_function_generator *__restrict self,
                                          DeeTypeObject *type, struct Dee_class_attribute const *attr) {
	/* Behavior here mirrors `DeeInstance_SetAttribute()' */
	struct class_desc *desc = DeeClass_DESC(type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					DO(Dee_function_generator_vpush_const(self, type)); /* this, value, type */
					DO(Dee_function_generator_vpush_cmember(self, attr->ca_addr + CLASS_GETSET_SET,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* this, value, setter */
					DO(Dee_function_generator_vrrot(self, 3));       /* setter, this, value */
					DO(Dee_function_generator_vopthiscall(self, 1)); /* result */
				} else {
					DO(Dee_function_generator_vpop_at(self, 2));        /* value */
					DO(Dee_function_generator_vpush_const(self, type)); /* value, type */
					DO(Dee_function_generator_vpush_cmember(self, attr->ca_addr + CLASS_GETSET_SET,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* value, setter */
					DO(Dee_function_generator_vswap(self));      /* setter, value */
					DO(Dee_function_generator_vopcall(self, 1)); /* result */
				}
				return Dee_function_generator_vpop(self);
			} else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
				/* XXX: Dee_function_generator_vpop_cmember() */
			}
		} else {
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					DO(Dee_function_generator_vswap(self));          /* value, this */
					DO(Dee_function_generator_vdup(self));           /* value, this, this */
					DO(Dee_function_generator_vpush_const(self, type)); /* value, this, this, type */
					DO(Dee_function_generator_vpush_imember(self, attr->ca_addr + CLASS_GETSET_SET,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* value, this, setter */
					DO(Dee_function_generator_vswap(self));          /* value, setter, this */
					DO(Dee_function_generator_vlrot(self, 3));       /* setter, this, value */
					DO(Dee_function_generator_vopthiscall(self, 1)); /* result */
				} else {
					DO(Dee_function_generator_vswap(self));          /* value, this */
					DO(Dee_function_generator_vpush_const(self, type)); /* value, this, type */
					DO(Dee_function_generator_vpush_imember(self, attr->ca_addr + CLASS_GETSET_SET,
					                                        DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* value, setter */
					DO(Dee_function_generator_vswap(self));          /* setter, value */
					DO(Dee_function_generator_vopcall(self, 1));     /* result */
				}
				return Dee_function_generator_vpop(self);
			} else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
				DO(Dee_function_generator_vpush_const(self, type));  /* this, value, type */
				DO(Dee_function_generator_vswap(self));              /* this, type, value */
				return Dee_function_generator_vpop_imember(self, attr->ca_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
			}
		}
	}
/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc));        /* this, value, desc */
	DO(Dee_function_generator_vlrot(self, 3));                /* value, desc, this */
	DO(Dee_function_generator_vdup(self));                    /* value, desc, this, this */
	DO(Dee_function_generator_vdelta(self, desc->cd_offset)); /* value, desc, this, DeeInstance_DESC(desc, this) */
	DO(Dee_function_generator_vswap(self));                   /* value, desc, DeeInstance_DESC(desc, this), this */
	DO(Dee_function_generator_vpush_addr(self, attr));        /* value, desc, DeeInstance_DESC(desc, this), this, attr */
	DO(Dee_function_generator_vlrot(self, 5));                /* desc, DeeInstance_DESC(desc, this), this, attr, value */
	DO(Dee_function_generator_vnotoneref_at(self, 1));        /* desc, DeeInstance_DESC(desc, this), this, attr, value */
	return Dee_function_generator_vcallapi(self, &DeeInstance_SetAttribute, VCALL_CC_INT, 4);
err:
	return -1;
}

/* this, [args...], kw -> result */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vcall_instance_attrkw(struct Dee_function_generator *__restrict self, DeeTypeObject *type,
                                             struct Dee_class_attribute const *attr, Dee_vstackaddr_t argc) {
	uint16_t field_addr;
	/* Behavior here mirrors `DeeInstance_CallAttributeKw()' */
	DO(Dee_function_generator_vlrot(self, argc + 2)); /* [args...], kw, this */
	field_addr = attr->ca_addr;
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		field_addr += CLASS_GETSET_GET;
#endif /* CLASS_GETSET_GET != 0 */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		/* Member lies in class memory. */                  /* [args...], kw, this */
		DO(Dee_function_generator_vpush_const(self, type)); /* [args...], kw, this, type */
		DO(Dee_function_generator_vpush_cmember(self, field_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], kw, this, member */
	} else {
		/* Member lies in instance memory. */               /* [args...], kw, this */
		DO(Dee_function_generator_vdup(self));              /* [args...], kw, this, this */
		DO(Dee_function_generator_vpush_const(self, type)); /* [args...], kw, this, this, type */
		DO(Dee_function_generator_vpush_imember(self, field_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], kw, this, member */
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) { /* [args...], kw, this, getter */
		DO(Dee_function_generator_vswap(self));    /* [args...], kw, getter, this */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			DO(Dee_function_generator_vopthiscall(self, 0)); /* [args...], kw, func */
		} else {
			DO(Dee_function_generator_vpop(self));       /* [args...], kw, getter */
			DO(Dee_function_generator_vopcall(self, 0)); /* [args...], kw, func */
		}
		DO(Dee_function_generator_vrrot(self, argc + 2)); /* func, [args...], kw */
		return Dee_function_generator_vopcallkw(self, argc);
	} else {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {        /* [args...], kw, this, func */
			DO(Dee_function_generator_vrrot(self, argc + 3)); /* func, [args...], kw, this */
			DO(Dee_function_generator_vrrot(self, argc + 2)); /* func, this, [args...], kw */
			return Dee_function_generator_vopthiscallkw(self, argc);
		} else {                                              /* [args...], kw, this, func */
			DO(Dee_function_generator_vrrot(self, argc + 3)); /* func, [args...], kw, this */
			DO(Dee_function_generator_vpop(self));            /* func, [args...], kw */
			return Dee_function_generator_vopcallkw(self, argc);
		}
		__builtin_unreachable();
	}
	__builtin_unreachable();
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_EX_C */
