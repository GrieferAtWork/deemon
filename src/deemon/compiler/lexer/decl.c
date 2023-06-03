/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_DECL_C
#define GUARD_DEEMON_COMPILER_LEXER_DECL_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy(), ... */
#include <deemon/tuple.h>

#include "../../runtime/builtin.h"

DECL_BEGIN

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION

INTERN WUNUSED NONNULL((1, 2)) int DCALL
decl_ast_copy(struct decl_ast *__restrict self,
              struct decl_ast const *__restrict other) {
	memcpy(self, other, sizeof(struct decl_ast));
	switch (self->da_type) {

	case DAST_SYMBOL:
		symbol_incref(self->da_symbol);
		break;

	case DAST_CONST:
	case DAST_STRING:
		Dee_Incref(self->da_const);
		break;

	case DAST_ALT:
	case DAST_TUPLE: {
		size_t i;
		struct decl_ast *new_vec;
		new_vec = (struct decl_ast *)Dee_Mallocc(self->da_alt.a_altc,
		                                         sizeof(struct decl_ast));
		if unlikely(!new_vec)
			goto err;
		for (i = 0; i < self->da_alt.a_altc; ++i) {
			if unlikely(decl_ast_copy(&new_vec[i],
			                          &self->da_alt.a_altv[i])) {
				while (i--)
					decl_ast_fini(&new_vec[i]);
				Dee_Free(new_vec);
				goto err;
			}
		}
		self->da_alt.a_altv = new_vec;
	}	break;

	case DAST_ATTR: {
		struct decl_ast *inner;
		Dee_Incref(self->da_attr.a_name);
		goto copy_inner;
	case DAST_FUNC:
		Dee_weakref_copy(&self->da_func.f_scope,
		                 &other->da_func.f_scope);
		if (!self->da_func.f_ret)
			break;
		ATTR_FALLTHROUGH
	case DAST_SEQ:
copy_inner:
		inner = (struct decl_ast *)Dee_Malloc(sizeof(struct decl_ast));
		if unlikely(!inner)
			goto err;
		if unlikely(decl_ast_copy(inner, self->da_seq)) {
			Dee_Free(inner);
			goto err;
		}
		self->da_seq = inner;
	}	break;

	case DAST_MAP:
	case DAST_WITH: {
		struct decl_ast *inner;
		inner = (struct decl_ast *)Dee_Mallocc(2, sizeof(struct decl_ast));
		if unlikely(!inner)
			goto err;
		if unlikely(decl_ast_copy(&inner[0], &self->da_with.w_cell[0])) {
err_with_inner:
			Dee_Free(inner);
			goto err;
		}
		if unlikely(decl_ast_copy(&inner[1], &self->da_with.w_cell[1])) {
			decl_ast_fini(&inner[0]);
			goto err_with_inner;
		}
		self->da_with.w_cell = inner;
	}	break;

	default: break;
	}
	return 0;
err:
	return -1;
}


/* Finalize the given declaration ast. */
INTERN NONNULL((1)) void DCALL
decl_ast_fini(struct decl_ast *__restrict self) {
	switch (self->da_type) {

	case DAST_SYMBOL:
		symbol_decref(self->da_symbol);
		break;

	case DAST_CONST:
	case DAST_STRING:
		Dee_Decref(self->da_const);
		break;

	case DAST_ALT:
	case DAST_TUPLE: {
		size_t i;
		for (i = 0; i < self->da_alt.a_altc; ++i)
			decl_ast_fini(&self->da_alt.a_altv[i]);
		Dee_Free(self->da_alt.a_altv);
	}	break;

	case DAST_MAP:
	case DAST_WITH:
		decl_ast_fini(&self->da_with.w_cell[1]);
		goto free_inner;

	case DAST_ATTR:
		Dee_Decref(self->da_attr.a_name);
		goto free_inner;

	case DAST_FUNC:
		Dee_weakref_fini(&self->da_func.f_scope);
		if (!self->da_func.f_ret)
			break;
		ATTR_FALLTHROUGH
	case DAST_SEQ:
free_inner:
		decl_ast_fini(self->da_seq);
		Dee_Free(self->da_seq);
		break;

	default: break;
	}
}




/* If `self' refers to a constant object, return that object. */
INTERN WUNUSED NONNULL((1)) DeeObject *DCALL
decl_ast_getobj(struct decl_ast const *__restrict self) {
	if (self->da_type == DAST_CONST)
		return self->da_const;
	if (self->da_type == DAST_SYMBOL) {
		struct symbol *sym = self->da_symbol;
		SYMBOL_INPLACE_UNWIND_ALIAS(sym);
		if (sym->s_type == SYMBOL_TYPE_CONST)
			return sym->s_const;
	}
	return NULL;
}

/* Check if `self' refers to `none' or `type none' */
INTERN WUNUSED NONNULL((1)) bool DCALL
decl_ast_isnone(struct decl_ast const *__restrict self) {
	if (self->da_type == DAST_CONST)
		return DeeNone_Check(self->da_const) || self->da_const == (DeeObject *)&DeeNone_Type;
	if (self->da_type == DAST_SYMBOL) {
		struct symbol *sym = self->da_symbol;
check_symbol:
		switch (sym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			sym = sym->s_alias;
			goto check_symbol;

		case SYMBOL_TYPE_EXTERN:
			return (sym->s_extern.e_module == &DeeModule_Deemon &&
			        sym->s_extern.e_symbol->ss_index == id_none);

		case SYMBOL_TYPE_CONST:
			return DeeNone_Check(sym->s_const) || sym->s_const == (DeeObject *)&DeeNone_Type;

		default: break;
		}
	}
	return false;
}


INTERN WUNUSED NONNULL((1)) bool DCALL
decl_ast_isobject(struct decl_ast const *__restrict self) {
	if (self->da_type == DAST_CONST)
		return self->da_const == (DeeObject *)&DeeObject_Type;
	if (self->da_type == DAST_SYMBOL) {
		struct symbol *sym = self->da_symbol;
check_symbol:
		switch (sym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			sym = sym->s_alias;
			goto check_symbol;

		case SYMBOL_TYPE_EXTERN:
			return (sym->s_extern.e_module == &DeeModule_Deemon &&
			        sym->s_extern.e_symbol->ss_index == id_Object);

		case SYMBOL_TYPE_CONST:
			return sym->s_const == (DeeObject *)&DeeObject_Type;

		default: break;
		}
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
decl_ast_istype(struct decl_ast const *__restrict self,
                DeeTypeObject *__restrict tp) {
	if (tp == &DeeNone_Type)
		return decl_ast_isnone(self);
	if (self->da_type == DAST_CONST)
		return self->da_const == (DeeObject *)tp;
	if (self->da_type == DAST_SYMBOL) {
		struct symbol *sym = self->da_symbol;
check_symbol:
		switch (sym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			sym = sym->s_alias;
			goto check_symbol;

		case SYMBOL_TYPE_EXTERN:
			/* Check if the symbol refers to the same type, when
			 * that type is exported from the deemon module. */
			if (sym->s_extern.e_module != &DeeModule_Deemon)
				return false;
			if (sym->s_extern.e_symbol->ss_index < DeeModule_Deemon.mo_globalc &&
			    DeeModule_Deemon.mo_globalv[sym->s_extern.e_symbol->ss_index] == (DeeObject *)tp)
				return true;
			break;

		case SYMBOL_TYPE_CONST:
			return sym->s_const == (DeeObject *)tp;

		default: break;
		}
	}
	return false;
}


INTERN WUNUSED NONNULL((1)) bool DCALL
decl_ast_isempty(struct decl_ast const *__restrict self) {
	switch (self->da_type) {

	case DAST_SYMBOL:
	case DAST_CONST:
	case DAST_ALT:
	case DAST_TUPLE:
	case DAST_SEQ:
	case DAST_ATTR:
	case DAST_MAP:
	case DAST_WITH:
		goto nope;

	case DAST_STRING:
		return DeeString_IsEmpty(self->da_string);

	case DAST_FUNC: {
		DeeBaseScopeObject *scope;
		if (self->da_func.f_ret &&
		    !decl_ast_isempty(self->da_func.f_ret))
			goto nope; /* We've got a return type. */
		scope = decl_ast_func_getscope(self);
		if unlikely(!scope)
			goto nope;
		if ((scope->bs_argc != 0) ||                   /* We've got argument names */
		    !(scope->bs_cflags & BASESCOPE_FRETURN)) { /* We know that only `none' is ever returned */
			Dee_Decref_unlikely((DeeObject *)scope);
			goto nope;
		}
		Dee_Decref_unlikely((DeeObject *)scope);
		/* The argument count can be determined without the doc,
		 * so we can simply consider this decl ast as empty. */
	}	break;

	default: break;
	}
	return true;
nope:
	return false;
}

/* Print the given `name' as encoded documentation name:
 * >> if (!name.issymbol()) {
 * >>     for (local x: r'\?!{}|,()<>[]=')
 * >>          name = name.replace(x, r'\' + x);
 * >>     name = name.replace(r"->", r"-\>");
 * >>     name = name.replace("\n", "\\\n"); // For any type of line-feed
 * >>     name = "{" + name + "}";
 * >> } */
INTERN int DCALL
decl_ast_escapename(/*utf-8*/ char const *__restrict name, size_t name_len,
                    struct unicode_printer *__restrict printer) {
	char const *iter, *end, *flush_start = name;
	bool must_escape = !name_len || !DeeUni_IsSymStrt(name[0]);
	end              = (iter = name) + name_len;
	for (; iter < end; ++iter) {
		char ch = *iter;
		if (!must_escape && !DeeUni_IsSymCont(ch)) {
			if unlikely(unicode_printer_putascii(printer, '{'))
				goto err;
			must_escape = true;
		}
		if (ch == '\\' || ch == '?' || ch == '!' ||
		    ch == '{' || ch == '}' || ch == '|' ||
		    ch == ',' || ch == '(' || ch == ')' ||
		    ch == '<' || ch == '>' || ch == '[' ||
		    ch == ']' || ch == '=' || ch == '\n' ||
		    ch == '\r') {
			/* Must escape this character. */
			if (!must_escape &&
			    unlikely(unicode_printer_putascii(printer, '{')))
				goto err;
			if unlikely(unicode_printer_print(printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err;
			if unlikely(unicode_printer_putascii(printer, '\\'))
				goto err;
			must_escape = true;
			flush_start = iter;
			if (ch == '\r' && iter + 1 < end && iter[1] == '\n')
				++iter; /* CRLF */
			continue;
		}
		/* Escape `->' as `-\>' */
		if (ch == '-' && iter + 1 < end && iter[1] == '>') {
			++iter;
			if (!must_escape &&
			    unlikely(unicode_printer_putascii(printer, '{')))
				goto err;
			if unlikely(unicode_printer_print(printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err;
			if unlikely(unicode_printer_putascii(printer, '\\'))
				goto err;
			must_escape = true;
			flush_start = iter;
			continue;
		}
	}
	if unlikely(unicode_printer_print(printer, flush_start, (size_t)(end - flush_start)) < 0)
		goto err;
	/* Print the trailing right-brace required when the name was escaped. */
	if (must_escape &&
	    unlikely(unicode_printer_putascii(printer, '}')))
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
decl_ast_print_const_type(DeeObject const *__restrict ob,
                          struct unicode_printer *__restrict printer) {
	uint16_t i;
	DeeModuleObject *deemon;
	if (DeeNone_Check(ob) || ob == (DeeObject *)&DeeNone_Type) {
		if (UNICODE_PRINTER_PRINT(printer, "?N") < 0)
			goto err;
		return 0;
	}
	if (!DeeType_Check(ob))
		goto print_object;
	deemon = DeeModule_GetDeemon();
	/* Search the builtin `deemon' module for this export. */
	for (i = 0; i < deemon->mo_globalc; ++i) {
		struct module_symbol *sym;
		if (deemon->mo_globalv[i] != ob)
			continue;
			/* Special encodings for specific objects. */
#if 0
		if (i == id_Sequence) {
			if (UNICODE_PRINTER_PRINT(printer, "?S?O") < 0)
				goto err;
		} else if (i == id_Mapping) {
			if (UNICODE_PRINTER_PRINT(printer, "?M?O?O") < 0) /* {Object: Object} */
				goto err;
		} else
#endif
		if (i == id_none) {
			if (UNICODE_PRINTER_PRINT(printer, "?N") < 0)
				goto err;
		} else if (i == id_Object) {
			if (UNICODE_PRINTER_PRINT(printer, "?O") < 0)
				goto err;
		} else {
			/* Found it! */
			sym = DeeModule_GetSymbolID(deemon, i);
			ASSERT(sym != NULL);
			if (UNICODE_PRINTER_PRINT(printer, "?D") < 0)
				goto err;
			/* NOTE: No need to use `decl_ast_escapename()' here. - We can assume that
			 *       the builtin deemon module doesn't export anything that would require
			 *       its name to be escaped (deemon only exposes symbols with pure names) */
			if (sym->ss_flags & MODSYM_FNAMEOBJ) {
				if (unicode_printer_printstring(printer, (DeeObject *)COMPILER_CONTAINER_OF(sym->ss_name, DeeStringObject, s_str)) < 0)
					goto err;
			} else {
				if (unicode_printer_print(printer, sym->ss_name, strlen(sym->ss_name)) < 0)
					goto err;
			}
		}
		return 0;
	}
print_object:
	if (UNICODE_PRINTER_PRINT(printer, "?O") < 0)
		goto err;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
decl_ast_print_type(struct decl_ast const *__restrict self,
                    struct unicode_printer *__restrict printer) {
	switch (self->da_type) {

	case DAST_SYMBOL: {
		struct symbol *sym;
		sym = self->da_symbol;
switch_symbol_type:
		switch (sym->s_type) {

		case SYMBOL_TYPE_GLOBAL:
			/* Global symbol reference. */
			if (UNICODE_PRINTER_PRINT(printer, "?G") < 0)
				goto err;
			if (decl_ast_escapename(sym->s_name->k_name,
			                        sym->s_name->k_size,
			                        printer) < 0)
				goto err;
			break;

		case SYMBOL_TYPE_EXTERN: {
			struct module_symbol *msym;
			msym = sym->s_extern.e_symbol;
			if (sym->s_extern.e_module != &DeeModule_Deemon) {
				char const *module_name;
				/* External symbol reference. */
				if (UNICODE_PRINTER_PRINT(printer, "?E") < 0)
					goto err;
				module_name = DeeString_AsUtf8((DeeObject *)sym->s_extern.e_module->mo_name);
				if unlikely(!module_name)
					goto err;
				if (decl_ast_escapename(module_name,
				                        WSTR_LENGTH(module_name),
				                        printer) < 0)
					goto err;
				if unlikely(unicode_printer_putc(printer, ':'))
					goto err;
				if (decl_ast_escapename(MODULE_SYMBOL_GETNAMESTR(msym),
				                        MODULE_SYMBOL_GETNAMELEN(msym),
				                        printer) < 0)
					goto err;
			} else {
				/* Reference into deemon. */
				if (msym->ss_index == id_Object) {
					if (UNICODE_PRINTER_PRINT(printer, "?O") < 0)
						goto err;
				} else if (msym->ss_index == id_none) {
					if (UNICODE_PRINTER_PRINT(printer, "?N") < 0)
						goto err;
#if 0
				} else if (msym->ss_index == id_Sequence) {
					if (UNICODE_PRINTER_PRINT(printer, "?S?O") < 0)
						goto err;
				} else if (msym->ss_index == id_Mapping) {
					if (UNICODE_PRINTER_PRINT(printer, "?M?O?O") < 0)
						goto err;
#endif
				} else {
					if (UNICODE_PRINTER_PRINT(printer, "?D") < 0)
						goto err;
					if (decl_ast_escapename(MODULE_SYMBOL_GETNAMESTR(msym),
					                        MODULE_SYMBOL_GETNAMELEN(msym),
					                        printer) < 0)
						goto err;
				}
			}
		}	break;

#if 0 /* TODO: This can sometimes be implemented through `?#' */
		case SYMBOL_TYPE_CATTR:
#endif

		case SYMBOL_TYPE_GETSET:
			if (!sym->s_getset.gs_get)
				goto print_undefined_symbol_name;
			sym = sym->s_getset.gs_get;
			goto switch_symbol_type;

		case SYMBOL_TYPE_ALIAS:
			sym = sym->s_alias;
			goto switch_symbol_type;

		case SYMBOL_TYPE_CONST:
			return decl_ast_print_const_type(sym->s_const, printer);

		case SYMBOL_TYPE_FWD: {
			struct TPPKeyword *symbol_name;
			DeeScopeObject *iter;
			/* Search for the symbol that was actually intended.
			 * TODO: We really should only do this _after_ a class
			 *       definition has already been finished. */
			symbol_name = sym->s_name;
			iter        = sym->s_scope;
			while ((iter = iter->s_prev) != NULL) {
				sym = get_local_symbol_in_scope(iter, symbol_name);
				if (sym)
					goto switch_symbol_type;
			}
			sym = self->da_symbol;
			goto print_undefined_symbol_name;
		}	break;

		default:
			/* Fallback: emit the name of the symbol as private/undefined */
print_undefined_symbol_name:
			if (UNICODE_PRINTER_PRINT(printer, "?U") < 0)
				goto err;
			if (decl_ast_escapename(sym->s_name->k_name,
			                        sym->s_name->k_size,
			                        printer) < 0)
				goto err;
			return 0;
		}
	}	break;

	case DAST_CONST:
		return decl_ast_print_const_type(self->da_const, printer);

	case DAST_ALT: {
		size_t i;
		if (unicode_printer_printf(printer, "?X%" PRFuSIZ, self->da_alt.a_altc) < 0)
			goto err;
		for (i = 0; i < self->da_alt.a_altc; ++i) {
			if unlikely(decl_ast_print_type(&self->da_alt.a_altv[i], printer))
				goto err;
		}
	}	break;

	case DAST_TUPLE: {
		size_t i;
		if (unicode_printer_printf(printer, "?T%" PRFuSIZ, self->da_tuple.t_itemc) < 0)
			goto err;
		for (i = 0; i < self->da_tuple.t_itemc; ++i) {
			if unlikely(decl_ast_print_type(&self->da_tuple.t_itemv[i], printer))
				goto err;
		}
	}	break;

	case DAST_SEQ:
		if (UNICODE_PRINTER_PRINT(printer, "?S") < 0)
			goto err;
		if unlikely(decl_ast_print_type(self->da_seq, printer))
			goto err;
		break;

	case DAST_ATTR: {
		char *attrname;
		attrname = DeeString_AsUtf8((DeeObject *)self->da_attr.a_name);
		if unlikely(!attrname)
			goto err;
		if unlikely(UNICODE_PRINTER_PRINT(printer, "?A") < 0)
			goto err;
		if unlikely(decl_ast_escapename(attrname, WSTR_LENGTH(attrname), printer) < 0)
			goto err;
		if unlikely(decl_ast_print_type(self->da_attr.a_base, printer))
			goto err;
	}	break;

	case DAST_WITH:
		if unlikely(UNICODE_PRINTER_PRINT(printer, "?C") < 0)
			goto err;
		if unlikely(decl_ast_print_type(&self->da_with.w_cell[0], printer))
			goto err;
		if unlikely(decl_ast_print_type(&self->da_with.w_cell[1], printer))
			goto err;
		break;

	case DAST_MAP:
		if unlikely(UNICODE_PRINTER_PRINT(printer, "?M") < 0)
			goto err;
		if unlikely(decl_ast_print_type(&self->da_map.m_key_value[0], printer))
			goto err;
		if unlikely(decl_ast_print_type(&self->da_map.m_key_value[1], printer))
			goto err;
		break;

	case DAST_STRING:
		if unlikely(unicode_printer_printstring(printer, (DeeObject *)self->da_string) < 0)
			goto err;
		break;

	default:
/*print_object:*/
		/* Fallback: emit a reference to `object' */
		if (UNICODE_PRINTER_PRINT(printer, "?O") < 0)
			goto err;
		break;
	}
	return 0;
err:
	return -1;
}

#if 0
INTERN WUNUSED NONNULL((1, 2)) int DCALL
decl_ast_print_const_expr(DeeObject *__restrict self,
                          struct unicode_printer *__restrict printer) {
	if (unicode_printer_putascii(printer, '!'))
		goto err;
	if (DeeNone_Check(self))
		goto eval_none;
	if (DeeInt_Check(self)) {
		if (DeeInt_Print(self, DEEINT_PRINT_DEC, 0,
		                 &unicode_printer_print,
		                 printer) < 0)
			goto err;
		return 0;
	}
	if (DeeFloat_Check(self)) {
		if (DeeFloat_Print(DeeFloat_VALUE(self),
		                   &unicode_printer_print,
		                   printer, 0, 0, DEEFLOAT_PRINT_FNORMAL) < 0)
			goto err;
		return 0;
	}
	if (DeeString_Check(self)) {
		char const *utf8_repr;
		utf8_repr = DeeString_AsUtf8(self);
		if unlikely(!utf8_repr)
			goto err;
		if unlikely(unicode_printer_putascii(printer, 'P'))
			goto err;
		return decl_ast_escapename(utf8_repr, WSTR_LENGTH(utf8_repr), printer);
	}
	if (DeeTuple_Check(self)) {
		/* TODO */
	}
	if (DeeList_Check(self)) {
		/* TODO */
	}
	if (DeeDict_Check(self)) {
		/* TODO */
	}
	if (DeeHashSet_Check(self)) {
		/* TODO */
	}
	/* Fallback: just print `none' */
eval_none:
	return unicode_printer_putascii(printer, 'n');
err:
	return -1;
}
#endif

INTERN WUNUSED NONNULL((1, 2)) int DCALL
decl_ast_print(struct decl_ast const *__restrict self,
               struct unicode_printer *__restrict printer) {
	size_t i;
	struct symbol **argv;
	DREF DeeBaseScopeObject *scope;
	if (self->da_type != DAST_FUNC) {
		if (self->da_type == DAST_STRING) {
			/* Simply print the declaration string prefix. */
			return decl_ast_print_type(self, printer);
		}
		if (UNICODE_PRINTER_PRINT(printer, "->") < 0)
			goto err_noscope;
		return decl_ast_print_type(self, printer);
	}
	scope = decl_ast_func_getscope(self);
	if (scope->bs_argc) {
		/* Special case: function */
		if (unicode_printer_putascii(printer, '('))
			goto err;
		argv = scope->bs_argv;
		for (i = 0; i < scope->bs_argc; ++i) {
			struct symbol *arg;
			if (i != 0 && unicode_printer_putascii(printer, ','))
				goto err;
			arg = argv[i];
			if (arg == scope->bs_varargs ||
			    arg == scope->bs_varkwds) {
				if (unicode_printer_print(printer,
				                          arg->s_name->k_name,
				                          arg->s_name->k_size) < 0)
					goto err;
			} else {
				/* Since we always generate keyword information, the names of
				 * positional arguments can always be extracted from code keywords. */
			}
			if (i >= scope->bs_argc_min && i < scope->bs_argc_max &&
			    !scope->bs_default[i - scope->bs_argc_min]) {
				/* Optional argument */
				if (unicode_printer_putascii(printer, '?'))
					goto err;
			} else if (arg == scope->bs_varargs) {
				/* Varargs argument */
				if (unicode_printer_putascii(printer, '!'))
					goto err;
			} else if (arg == scope->bs_varkwds) {
				/* Varkwds argument */
				if (UNICODE_PRINTER_PRINT(printer, "!!") < 0)
					goto err;
			}
			if (!decl_ast_isempty(&arg->s_decltype) &&
			    (i < scope->bs_argc_min || i >= scope->bs_argc_max ||
			     !scope->bs_default[i - scope->bs_argc_min] ||
			     !decl_ast_istype(&arg->s_decltype, /* Don't encode a type if it can be deduced from the default argument */
			                      Dee_TYPE(scope->bs_default[i - scope->bs_argc_min])))) {
				/* Encode the argument type. */
				if (unicode_printer_putascii(printer, ':'))
					goto err;
				if (decl_ast_print_type(&arg->s_decltype, printer))
					goto err;
			}
			if (i >= scope->bs_argc_min && i < scope->bs_argc_max &&
			    scope->bs_default[i - scope->bs_argc_min]) {
				/* Argument has a default parameter. */
				if (unicode_printer_putascii(printer, '='))
					goto err;
#if 0
				if (decl_ast_print_const_expr(scope->bs_default[i - scope->bs_argc_min], printer))
					goto err;
#endif
			}
		}
		if (unicode_printer_putascii(printer, ')'))
			goto err;
	}
	if (self->da_func.f_ret) {
		struct decl_ast *ret = self->da_func.f_ret;
		/* Check if the hinted type is `none'
		 * If it is, we can encode declaration information
		 * through its shorted variant written as `()' */
		if (!scope->bs_argc && decl_ast_isnone(ret))
			goto encode_empty_paren;
		/* Explicit return type information. */
		if (UNICODE_PRINTER_PRINT(printer, "->") < 0)
			goto err;
		/* The return type can be omitted when it is `object from deemon' */
		if (!decl_ast_isobject(ret)) {
			if (decl_ast_print_type(ret, printer))
				goto err;
		}
	} else if (scope->bs_cflags & BASESCOPE_FRETURN) {
		/* Function can return anything --- encode as `(args...)->' */
		if (UNICODE_PRINTER_PRINT(printer, "->") < 0)
			goto err;
	} else if (!scope->bs_argc) {
		/* Function only returns `none' --- encode as `()' */
encode_empty_paren:
		if (UNICODE_PRINTER_PRINT(printer, "()") < 0)
			goto err;
	}
	Dee_Decref_unlikely((DeeObject *)scope);
	return 0;
err:
	Dee_Decref_unlikely((DeeObject *)scope);
err_noscope:
	return -1;
}


/* Print the given `text' as encoded documentation text.
 *  - Escape any line-feed immediately following after another
 *  - Escape any instance of "->" with "-\>"
 *  - Escape any line starting with "(" as "\(" */
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL
decl_ast_escapetext8(uint8_t const *__restrict text, size_t text_len,
                     struct unicode_printer *__restrict printer,
                     struct unicode_printer *__restrict source_printer);
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL
decl_ast_escapetext16(uint16_t const *__restrict text, size_t text_len,
                      struct unicode_printer *__restrict printer,
                      struct unicode_printer *__restrict source_printer);
INTDEF WUNUSED NONNULL((1, 3, 4)) int DCALL
decl_ast_escapetext32(uint32_t const *__restrict text, size_t text_len,
                      struct unicode_printer *__restrict printer,
                      struct unicode_printer *__restrict source_printer);
#ifndef __INTELLISENSE__
#define N 8
#include "decl-escape-text-impl.c.inl"
#define N 16
#include "decl-escape-text-impl.c.inl"
#define N 32
#include "decl-escape-text-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
/* Escape documentation text from "Encoded Documentation Text"
 * into "Fully Encoded Documentation Text" */
INTERN WUNUSED NONNULL((1)) int DCALL
doctext_escape(struct unicode_printer *__restrict doctext) {
	if (!UNICODE_PRINTER_ISEMPTY(doctext)) {
		struct unicode_printer result = UNICODE_PRINTER_INIT;
		switch (doctext->up_flags & UNICODE_PRINTER_FWIDTH) {

		CASE_WIDTH_1BYTE:
			if unlikely(decl_ast_escapetext8((uint8_t *)doctext->up_buffer,
			                                 doctext->up_length,
			                                 &result, doctext))
				goto err_r;
			break;

		CASE_WIDTH_2BYTE:
			if unlikely(decl_ast_escapetext16((uint16_t *)doctext->up_buffer,
			                                  doctext->up_length,
			                                  &result, doctext))
				goto err_r;
			break;

		CASE_WIDTH_4BYTE:
			if unlikely(decl_ast_escapetext32((uint32_t *)doctext->up_buffer,
			                                  doctext->up_length,
			                                  &result, doctext))
				goto err_r;
			break;
		}
		__IF0 {
err_r:
			unicode_printer_fini(&result);
			goto err;
		}
		/* Have the printer re-inherit itself. */
		unicode_printer_fini(doctext);
		memcpy(doctext, &result, sizeof(struct unicode_printer));
	}
	return 0;
err:
	return -1;
}
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */


/* Pack together the current documentation string. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_tags_doc(struct decl_ast const *__restrict decl) {
#if 0
	(void)decl;
	return_empty_string;
#else
	bool decl_is_empty;
	if unlikely(!UNICODE_PRINTER_ISEMPTY(&current_tags.at_decl)) {
		/* Return the user-defined declaration prefix. */
		DREF DeeObject *result;
		while (UNICODE_PRINTER_LENGTH(&current_tags.at_decl)) {
			uint32_t ch;
			ch = UNICODE_PRINTER_GETCHAR(&current_tags.at_decl,
			                             UNICODE_PRINTER_LENGTH(&current_tags.at_decl) - 1);
			if (!DeeUni_IsSpace(ch))
				break;
			--UNICODE_PRINTER_LENGTH(&current_tags.at_decl);
		}
		if (!UNICODE_PRINTER_ISEMPTY(&current_tags.at_doc)) {
			if unlikely(unicode_printer_putascii(&current_tags.at_decl, '\n'))
				goto err2;
			switch (current_tags.at_doc.up_flags & UNICODE_PRINTER_FWIDTH) {

			CASE_WIDTH_1BYTE:
				if unlikely(decl_ast_escapetext8((uint8_t *)current_tags.at_doc.up_buffer,
				                                 current_tags.at_doc.up_length,
				                                 &current_tags.at_decl, &current_tags.at_doc))
					goto err2;
				break;

			CASE_WIDTH_2BYTE:
				if unlikely(decl_ast_escapetext16((uint16_t *)current_tags.at_doc.up_buffer,
				                                  current_tags.at_doc.up_length,
				                                  &current_tags.at_decl, &current_tags.at_doc))
					goto err2;
				break;

			CASE_WIDTH_4BYTE:
				if unlikely(decl_ast_escapetext32((uint32_t *)current_tags.at_doc.up_buffer,
				                                  current_tags.at_doc.up_length,
				                                  &current_tags.at_decl, &current_tags.at_doc))
					goto err2;
				break;
			}
		}
		result = unicode_printer_pack(&current_tags.at_decl);
		unicode_printer_init(&current_tags.at_decl);
		return result;
	}

	/* Check if we have any ~real~ information to add to do strings. */
	decl_is_empty = decl_ast_isempty(decl);
	if (decl_is_empty &&
	    UNICODE_PRINTER_ISEMPTY(&current_tags.at_doc))
		return_empty_string;
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		/* Print the prefix containing declaration information. */
		if (!decl_is_empty && unlikely(decl_ast_print(decl, &printer)))
			goto err;
		if (!UNICODE_PRINTER_ISEMPTY(&current_tags.at_doc)) {
			/* Add a line-feed between the declaration prefix, and the user-defined doc text */
			if (!decl_is_empty && unicode_printer_putascii(&printer, '\n'))
				goto err;
			switch (current_tags.at_doc.up_flags & UNICODE_PRINTER_FWIDTH) {

			CASE_WIDTH_1BYTE:
				if unlikely(decl_ast_escapetext8((uint8_t *)current_tags.at_doc.up_buffer,
				                                 current_tags.at_doc.up_length,
				                                 &printer, &current_tags.at_doc))
					goto err;
				break;

			CASE_WIDTH_2BYTE:
				if unlikely(decl_ast_escapetext16((uint16_t *)current_tags.at_doc.up_buffer,
				                                  current_tags.at_doc.up_length,
				                                  &printer, &current_tags.at_doc))
					goto err;
				break;

			CASE_WIDTH_4BYTE:
				if unlikely(decl_ast_escapetext32((uint32_t *)current_tags.at_doc.up_buffer,
				                                  current_tags.at_doc.up_length,
				                                  &printer, &current_tags.at_doc))
					goto err;
				break;
			}
		}
		return unicode_printer_pack(&printer);
err:
		unicode_printer_fini(&printer);
	}
err2:
	return NULL;
#endif
}


/* Parse a declaration expression. */
INTERN WUNUSED NONNULL((1)) int DCALL
decl_ast_parse_unary_head(struct decl_ast *__restrict self) {
	uint32_t old_flags;
	switch (tok) {

	case KWD___asm:
	case KWD___asm__: {
		bool has_paren;
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (paren_begin(&has_paren, W_EXPECTED_LPAREN_AFTER_ASM))
			goto err_flags;

		/* Custom, user-defined encoding:
		 * >> function foo(a: __asm__("?Dobject")) {
		 * >>     ...
		 * >> } */
		if likely(tok == TOK_STRING ||
		          (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
			DREF DeeStringObject *text;
			text = (DREF DeeStringObject *)ast_parse_string();
			if unlikely(!text)
				goto err_flags;
			self->da_type   = DAST_STRING;
			self->da_string = text; /* Inherit reference */
		} else {
			if (WARN(W_EXPECTED_STRING_AFTER_ASM))
				goto err_flags;
			self->da_type = DAST_NONE;
		}
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (paren_end(has_paren, W_EXPECTED_RPAREN_AFTER_ASM))
			goto err_r;
	}	break;

	case KWD_none:
		/* None-type. */
		if unlikely(yield() < 0)
			goto err;
		self->da_type  = DAST_CONST;
		self->da_flag  = DAST_FNORMAL;
		self->da_const = (DREF DeeObject *)&DeeNone_Type;
		Dee_Incref(&DeeNone_Type);
		break;

#if 1
	case KWD_type: {
		DREF struct ast *type_expr;
		uint16_t old_opt_flags;
		/* Type-of-expression declaration. */
		if unlikely(yield() < 0)
			goto err;
		if (tok == '(') {
			type_expr = ast_parse_unaryhead(LOOKUP_SYM_NORMAL |
			                                PARSE_UNARY_DISALLOW_CASTS);
		} else {
			type_expr = ast_parse_unary(LOOKUP_SYM_NORMAL);
		}
		if unlikely(!type_expr)
			goto err;
		if (ast_optimize_all(type_expr, true)) {
err_type_expr:
			ast_decref(type_expr);
			goto err;
		}
		old_opt_flags = optimizer_flags;
		optimizer_flags &= ~OPTIMIZE_FNOPREDICT;
		self->da_const = (DREF DeeObject *)ast_predict_type(type_expr);
		optimizer_flags |= old_opt_flags & OPTIMIZE_FNOPREDICT;
		if unlikely(!self->da_const) {
			if (WARN(W_EXPECTED_CONSTANT_AFTER_TYPE_IN_DECL_EXPRESSION))
				goto err_type_expr;
			self->da_type = DAST_NONE;
			self->da_flag = DAST_FNORMAL;
		} else {
			self->da_type = DAST_CONST;
			self->da_flag = DAST_FNORMAL;
			Dee_Incref(self->da_const);
		}
		ast_decref(type_expr);
	}	break;
#endif

	case KWD_pack:
		/* support for "pack"
		 * >> local x: pack int, int, string;
		 * >> local y: (int, int, string); // Same as this */
	case '(': {
		int error;
		size_t elema, elemc;
		struct decl_ast *elemv;
		bool has_pack, has_paren;
		has_paren = tok == '(';
		has_pack  = tok != '(';

		/* Tuple type declaration. */
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		if (!has_paren && tok == '(') {
			has_paren = true;
			if unlikely(yield() < 0)
				goto err_flags;
		}

		error = decl_ast_parse(self);
		if unlikely(error)
			goto err_flags;

		/* This also functions as regular parenthesis, just like in
		 * normal expressions, where `()' is the empty tuple, `(foo)'
		 * is regular parenthesis, `(foo,)' is a 1-element tuple, and
		 * `(foo, bar)' and `(foo, bar,)' are 2-element tuples. */
		if (tok == ')' && !has_pack) {
			/* Simple parenthesis. */
			if unlikely(yield() < 0)
				goto err_r_flags;
			break;
		}
		elema = 2, elemc = 1;
		elemv = (struct decl_ast *)Dee_Mallocc(2, sizeof(struct decl_ast));
		if unlikely(!elemv)
			goto err_r_flags;
		memcpy(&elemv[0], self, sizeof(struct decl_ast));
		if (tok == ',') {
			for (;;) {
				if unlikely(yield() < 0)
					goto err_elemv;
				if (tok == ')')
					break; /* Single-element tuple / trailing comma */
				error = decl_ast_parse(&elemv[elemc]);
				if unlikely(error)
					goto err_elemv;
				++elemc;
				if (tok != ',')
					break;
				ASSERT(elemc <= elema);
				if (elemc >= elema) {
					struct decl_ast *new_elemv;
					elema *= 2;
					new_elemv = (struct decl_ast *)Dee_TryReallocc(elemv, elema,
					                                               sizeof(struct decl_ast));
					if unlikely(!new_elemv) {
						elema     = elemc + 1;
						new_elemv = (struct decl_ast *)Dee_Reallocc(elemv, elema,
						                                            sizeof(struct decl_ast));
						if unlikely(!new_elemv)
							goto err_elemv;
					}
					elemv = new_elemv;
				}
			}
		}
		if (elema != elemc) {
			struct decl_ast *new_elemv;
			new_elemv = (struct decl_ast *)Dee_TryReallocc(elemv, elemc,
			                                               sizeof(struct decl_ast));
			if likely(new_elemv)
				elemv = new_elemv;
		}
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (has_paren) {
			if (skip(')', W_EXPECTED_RPAREN_AFTER_TUPLE)) {
				old_flags = 0;
				goto err_elemv;
			}
		}

		/* Fill in the resulting AST representation as a TUPLE-ast */
		self->da_type          = DAST_TUPLE;
		self->da_flag          = DAST_FNORMAL;
		self->da_tuple.t_itemc = elemc;
		self->da_tuple.t_itemv = elemv; /* Inherit */
		break;
err_elemv:
		while (elemc--)
			decl_ast_fini(&elemv[elemc]);
		Dee_Free(elemv);
		goto err_flags;
	}	break;

	case '{': {
		int error;
		struct decl_ast *decl_seq;
		/* Sequence type declaration. */
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err_flags;
		decl_seq = (struct decl_ast *)Dee_Malloc(sizeof(struct decl_ast));
		if unlikely(!decl_seq)
			goto err_flags;
		error = decl_ast_parse(decl_seq);
		if unlikely(error) {
err_seq:
			Dee_Free(decl_seq);
			goto err_flags;
		}
		if (tok == ':') {
			/* Special case: `{x: y}' is an alias for `{(x, y)...}', as it best represents a mapping */
			struct decl_ast *key_value;
			key_value = (struct decl_ast *)Dee_Reallocc(decl_seq, 2, sizeof(struct decl_ast));
			if unlikely(!key_value) {
err_seq_0:
				decl_ast_fini(decl_seq);
				goto err_seq;
			}
			if unlikely(yield() < 0) {
err_elemv_0:
				decl_ast_fini(&key_value[0]);
				Dee_Free(key_value);
				goto err_flags;
			}
			error = decl_ast_parse(&key_value[1]);
			if unlikely(error)
				goto err_elemv_0;
			self->da_type = DAST_MAP;
			self->da_flag = DAST_FNORMAL;
			self->da_map.m_key_value = key_value; /* Inherit */
		} else {
			self->da_type = DAST_SEQ;
			self->da_flag = DAST_FNORMAL;
			self->da_seq  = decl_seq;
			if (skip(TOK_DOTS, W_EXPECTED_DOTS_OR_COLON_AFTER_BRACE_IN_TYPE_ANNOTATION))
				goto err_seq_0;
		}
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip('}', W_EXPECTED_RBRACE_AFTER_SEQUENCE))
			goto err_seq_0;
	}	break;


	case KWD___nth: {
		DREF struct ast *nth_expr;
		bool has_paren;

		/* N'th symbol compatibility. */
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (paren_begin(&has_paren, W_EXPECTED_LPAREN_AFTER_NTH))
			goto err_flags;
		nth_expr = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!nth_expr)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		/* Optimize the ast-expression to propagate constant, thus
		 * allowing the use of `__nth(2+3)' instead of forcing the
		 * user to write `__nth(5)' or `__nth(__TPP_EVAL(2+3))' */
		if (ast_optimize_all(nth_expr, true)) {
err_nth:
			ast_decref(nth_expr);
			goto err;
		}
		if (nth_expr->a_type != AST_CONSTEXPR &&
		    WARN(W_EXPECTED_CONSTANT_AFTER_NTH))
			goto err_nth;
		if (paren_end(has_paren, W_EXPECTED_RPAREN_AFTER_NTH))
			goto err_nth;
		if (TPP_ISKEYWORD(tok)) {
			unsigned int nth_symbol = 0;
			struct symbol *sym;
			if (nth_expr->a_type == AST_CONSTEXPR &&
			    DeeObject_AsUInt(nth_expr->a_constexpr, &nth_symbol)) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				if (WARN(W_EXPECTED_CONSTANT_AFTER_NTH))
					goto err_nth;
			}
			ast_decref(nth_expr);
			sym = lookup_nth(nth_symbol, token.t_kwd);
			if likely(sym) {
				self->da_type   = DAST_SYMBOL;
				self->da_flag   = DAST_FNORMAL;
				self->da_symbol = sym;
				symbol_incref(sym);
			} else {
				if (WARN(W_UNKNOWN_NTH_SYMBOL, nth_symbol))
					goto err;
				self->da_type = DAST_NONE;
				self->da_flag = DAST_FNORMAL;
			}
			if unlikely(yield() < 0)
				goto err_r;
		} else {
			ast_decref(nth_expr);
			if (WARN(W_EXPECTED_KEYWORD_AFTER_NTH))
				goto err;
			self->da_type = DAST_NONE;
			self->da_flag = DAST_FNORMAL;
		}
	}	break;

	default:
		if (TPP_ISKEYWORD(tok)) {
			/* Lookup a symbol-like expression. */
			DREF struct symbol *sym; /* Perform a regular symbol lookup. */
			struct TPPKeyword *name = token.t_kwd;
			if unlikely(yield() < 0)
				goto err;
			if (tok == KWD_from) {
				/* `Error from deemon' - Short form of `import Error from deemon' */
				if unlikely(yield() < 0)
					goto err;
				sym = ast_parse_import_single_sym(name);
			} else {
				sym = lookup_symbol(LOOKUP_SYM_NORMAL, name, NULL);
			}
			if unlikely(!sym)
				goto err;
			self->da_type   = DAST_SYMBOL;
			self->da_flag   = DAST_FNORMAL;
			self->da_symbol = sym;
			symbol_incref(sym);
			break;
		}
		if (WARN(W_UNEXPECTED_TOKEN_IN_DECL_EXPRESSION))
			goto err;
		self->da_type = DAST_NONE;
		self->da_flag = DAST_FNORMAL;
		break;
	}
	return 0;
err_r_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err_r:
	decl_ast_fini(self);
	goto err;
err_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	/*goto err;*/
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
decl_ast_parse_unary(struct decl_ast *__restrict self) {
	int result;
	result = decl_ast_parse_unary_head(self);
	switch (tok) {

	case '.':
		if unlikely(yield() < 0)
			goto err_r;
		if unlikely(!TPP_ISKEYWORD(tok)) {
			if (WARN(W_EXPECTED_KEYWORD_AFTER_DOT))
				goto err_r;
			break;
		}
		if (self->da_type == DAST_CONST) {
			DREF DeeObject *attrib;
			attrib = DeeObject_GetAttrString(self->da_const, token.t_kwd->k_name);
			if unlikely(!attrib) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				if (WARN(W_DECL_EXPRESSION_UNKNOWN_ATTRIBUTE,
				         token.t_kwd->k_name, self->da_const))
					goto err_r;
			} else {
				Dee_Decref(self->da_const);
				self->da_const = attrib; /* Inherit reference. */
			}
		} else if (self->da_type == DAST_SYMBOL &&
		           self->da_symbol->s_type == SYMBOL_TYPE_MODULE) {
			struct module_symbol *modsym;
			modsym = DeeModule_GetSymbolString(self->da_symbol->s_module, token.t_kwd->k_name,
			                                   Dee_HashPtr(token.t_kwd->k_name, token.t_kwd->k_size));
			if likely(modsym) {
				struct symbol *new_symbol;
				new_symbol = new_unnamed_symbol_in_scope(self->da_symbol->s_scope);
				if unlikely(!new_symbol)
					goto err_r;
				new_symbol->s_type            = SYMBOL_TYPE_EXTERN;
				new_symbol->s_extern.e_module = self->da_symbol->s_module;
				new_symbol->s_extern.e_symbol = modsym;
				Dee_Incref(self->da_symbol->s_module);
				symbol_incref(new_symbol);
				symbol_decref(self->da_symbol);
				self->da_symbol = new_symbol;
			} else {
				if (WARN(W_MODULE_IMPORT_NOT_FOUND, token.t_kwd->k_name,
				         DeeString_STR(self->da_symbol->s_module->mo_name)))
					goto err_r;
			}
		} else {
			struct decl_ast *inner_ast;
			DREF struct string_object *attr_name;
			attr_name = (DREF struct string_object *)DeeString_NewUtf8(token.t_kwd->k_name,
			                                                           token.t_kwd->k_size,
			                                                           STRING_ERROR_FIGNORE);
			if unlikely(!attr_name)
				goto err_r;
			inner_ast = (struct decl_ast *)Dee_Malloc(sizeof(struct decl_ast));
			if unlikely(!inner_ast) {
				Dee_Decref(attr_name);
				goto err_r;
			}
			decl_ast_move(inner_ast, self);
			self->da_type        = DAST_ATTR;
			self->da_flag        = DAST_FNORMAL;
			self->da_attr.a_base = inner_ast; /* Inherit reference. */
			self->da_attr.a_name = attr_name; /* Inherit reference. */
		}
		if unlikely(yield() < 0)
			goto err_r;
		break;

	default: break;
	}
	return result;
err_r:
	decl_ast_fini(self);
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
decl_ast_parse_alt(struct decl_ast *__restrict self) {
	int result;
	size_t elema, elemc, i;
	struct decl_ast *elemv;
	result = decl_ast_parse_unary(self);
	if unlikely(result)
		goto err;
	if (tok == '|') {
		elema = 2, elemc = 1;
		elemv = (struct decl_ast *)Dee_Mallocc(2, sizeof(struct decl_ast));
		if unlikely(!elemv)
			goto err_r;
		decl_ast_move(&elemv[0], self);
		for (;;) {
			if unlikely(yield() < 0)
				goto err_elemv;
			result = decl_ast_parse_unary(&elemv[elemc]);
			if unlikely(result)
				goto err_elemv;
			/* Check if the secondary variant is identical to a previous one.
			 * If it is, don't add it again! */
			for (i = 0; i < elemc; ++i) {
				if (!decl_ast_equal(&elemv[elemc], &elemv[i]))
					continue;
				/* This variant has already been declared! */
				decl_ast_fini(&elemv[elemc]);
				goto after_inc_elemc;
			}
			++elemc;
after_inc_elemc:
			if (tok != '|')
				break;
			ASSERT(elemc <= elema);
			if (elemc >= elema) {
				struct decl_ast *new_elemv;
				elema *= 2;
				new_elemv = (struct decl_ast *)Dee_TryReallocc(elemv, elema,
				                                               sizeof(struct decl_ast));
				if unlikely(!new_elemv) {
					elema     = elemc + 1;
					new_elemv = (struct decl_ast *)Dee_Reallocc(elemv, elema,
					                                            sizeof(struct decl_ast));
					if unlikely(!new_elemv)
						goto err_elemv;
				}
				elemv = new_elemv;
			}
		}
		if (elema != elemc) {
			struct decl_ast *new_elemv;
			new_elemv = (struct decl_ast *)Dee_TryReallocc(elemv, elemc,
			                                               sizeof(struct decl_ast));
			if likely(new_elemv)
				elemv = new_elemv;
		}
		/* Fill in the resulting AST representation as an ALT-ast */
		self->da_type       = DAST_ALT;
		self->da_flag       = DAST_FNORMAL;
		self->da_alt.a_altc = elemc;
		self->da_alt.a_altv = elemv; /* Inherit */
	}
	return result;
err_elemv:
	while (elemc--)
		decl_ast_fini(&elemv[elemc]);
	Dee_Free(elemv);
	goto err;
err_r:
	decl_ast_fini(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
decl_ast_parse_for_symbol(struct symbol *__restrict self) {
	if likely(self->s_decltype.da_type == DAST_NONE) {
		if unlikely(decl_ast_parse(&self->s_decltype))
			goto err;
	} else {
		struct decl_ast decl;
		if unlikely(decl_ast_parse(&decl))
			goto err;
		if unlikely(!decl_ast_equal(&self->s_decltype, &decl) &&
		            WARN(W_SYMBOL_TYPE_DECLARATION_CHANGED, self)) {
			decl_ast_fini(&decl);
			goto err;
		}
		decl_ast_fini(&self->s_decltype);
		self->s_decltype.da_type = DAST_NONE;
		decl_ast_move(&self->s_decltype, &decl);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
decl_ast_parse(struct decl_ast *__restrict self) {
	int result;
	result = decl_ast_parse_alt(self);
	if unlikely(result)
		goto err;
	while (tok == KWD_with) {
		struct decl_ast *inner;
		if unlikely(yield() < 0)
			goto err;
		inner = (struct decl_ast *)Dee_Mallocc(2, sizeof(struct decl_ast));
		if unlikely(!inner)
			goto err_r;
		result = decl_ast_parse_alt(&inner[1]);
		if unlikely(result) {
			Dee_Free(inner);
			goto err_r;
		}
		decl_ast_move(&inner[0], self);
		self->da_type        = DAST_WITH;
		self->da_flag        = DAST_FNORMAL;
		self->da_with.w_cell = inner; /* Inherit */
	}
	return 0;
err_r:
	decl_ast_fini(self);
err:
	return -1;
}

INTERN WUNUSED int DCALL
decl_ast_skip(void) {
	int result;
	struct decl_ast temp;
	result = decl_ast_parse(&temp);
	if (result == 0)
		decl_ast_fini(&temp);
	return result;
}



INTERN WUNUSED NONNULL((1, 2)) bool DCALL
decl_ast_equal(struct decl_ast const *__restrict a,
               struct decl_ast const *__restrict b) {
	if (a->da_type != b->da_type)
		goto nope; /* XXX: Some symbols can be identical to constants... */
	if (a->da_flag != b->da_flag)
		goto nope;
	switch (a->da_type) {

	case DAST_SYMBOL:
		if (SYMBOL_UNWIND_ALIAS(a->da_symbol) !=
		    SYMBOL_UNWIND_ALIAS(b->da_symbol))
			goto nope;
		break;

	case DAST_CONST:
		if (a->da_const != b->da_const)
			goto nope;
		break;

	case DAST_ALT: {
		size_t i, j;
		if (a->da_alt.a_altc != b->da_alt.a_altc)
			goto nope;
		/* Alternative representations don't have a fixed order,
		 * so compare all combinations when searching for maches. */
		for (i = 0; i < a->da_alt.a_altc; ++i) {
			for (j = 0; j < b->da_alt.a_altc; ++j) {
				if (decl_ast_equal(&a->da_alt.a_altv[i],
				                   &b->da_alt.a_altv[j]))
					goto found_alt;
			}
			goto nope;
		found_alt:;
		}
	}	break;

	case DAST_TUPLE: {
		size_t i;
		if (a->da_tuple.t_itemc != b->da_tuple.t_itemc)
			goto nope;
		for (i = 0; i < a->da_tuple.t_itemc; ++i) {
			if (!decl_ast_equal(&a->da_tuple.t_itemv[i],
			                    &b->da_tuple.t_itemv[i]))
				goto nope;
		}
	}	break;

	case DAST_SEQ:
		return decl_ast_equal(a->da_seq, b->da_seq);

	default: break;
	}
	return true;
nope:
	return false;
}

#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_DECL_C */
