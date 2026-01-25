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
#ifndef GUARD_DEEMON_COMPILER_LEXER_IMPORT_C
#define GUARD_DEEMON_COMPILER_LEXER_IMPORT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_*alloc*, Dee_Free */
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/module.h>          /* DeeModule*, Dee_MODSYM_F*, Dee_MODULE_FDIDLOAD, Dee_MODULE_HASHIT, Dee_MODULE_HASHNX, Dee_MODULE_HASHST, Dee_MODULE_SYMBOL_EQUALS, Dee_MODULE_SYMBOL_GETNAMELEN, Dee_MODULE_SYMBOL_GETNAMESTR, Dee_compiler_options, Dee_module_symbol, Dee_module_symbol_getindex */
#include <deemon/none.h>            /* Dee_None */
#include <deemon/object.h>
#include <deemon/string.h>          /* DeeString*, DeeUni_Flags, Dee_UNICODE_*, Dee_unicode_printer*, Dee_uniflag_t, STRING_ERROR_FSTRICT, WSTR_LENGTH */
#include <deemon/stringutils.h>     /* Dee_unicode_readutf8_n */
#include <deemon/system-features.h> /* CONFIG_HAVE_memrend, DeeSystem_DEFINE_memrend, memcpy, memrend, strlen */
#include <deemon/thread.h>          /* DeeThread_Self, Dee_import_frame */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t, uint32_t */

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#include <deemon/system.h> /* DeeSystem_BaseName */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_BEGIN

#ifndef CONFIG_HAVE_memrend
#define CONFIG_HAVE_memrend
#define memrend dee_memrend
DeeSystem_DEFINE_memrend(dee_memrend)
#endif /* !CONFIG_HAVE_memrend */

INTERN struct Dee_compiler_options *inner_compiler_options = NULL;

#define TOK_ISDOT(x) ((x) == '.' || (x) == TOK_DOTDOT || (x) == TOK_DOTS)
LOCAL unsigned int DCALL dot_count(tok_t tk) {
	if (tk == TOK_DOTS)
		return 3;
	if (tk == TOK_DOTDOT)
		return 2;
	return 1;
}

PRIVATE WUNUSED DREF DeeModuleObject *DCALL
import_module_by_name(DeeStringObject *__restrict module_name,
                      struct ast_loc *loc) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DREF DeeModuleObject *result;
	char const *filename;
	size_t name_size;
	if (module_name->s_str[0] == '.' && module_name->s_len == 1) {
		/* Special case: Import your own module. */
		return MODULE_CURRENT;
	}

	filename = TPPFile_RealFilename(token.t_file, &name_size);
	result = DeeModule_OpenEx(module_name->s_str, module_name->s_len,
	                          filename, filename ? strlen(filename) : 0,
	                          DeeModule_IMPORT_F_NORMAL |
	                          DeeModule_IMPORT_F_ENOENT |
	                          DeeModule_IMPORT_F_ERECUR,
	                          inner_compiler_options);
	if unlikely(!DeeModule_IMPORT_ISOK(result)) {
		if unlikely(result == DeeModule_IMPORT_ERROR) {
			goto err;
		} else if (result == DeeModule_IMPORT_ENOENT) {
			if (WARNAT(loc, W_MODULE_NOT_FOUND, module_name))
				goto err;
		} else if (result == DeeModule_IMPORT_ERECUR) {
			struct Dee_import_frame *current = DeeThread_Self()->t_import_curr;
			char const *current_name = current ? current->if_absfile : NULL;
			if (WARNAT(loc, W_RECURSIVE_MODULE_DEPENDENCY, module_name, current_name))
				goto err;
		}
		result = &DeeModule_Empty;
		Dee_Incref(result);
	}
	return result;
err:
	return NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	DREF DeeModuleObject *result;
	if (module_name->s_str[0] == '.') {
		char const *filename;
		size_t name_size;
		if (module_name->s_len == 1) {
			/* Special case: Import your own module. */
			return_reference_(MODULE_CURRENT);
		}
		filename = TPPFile_RealFilename(token.t_file, &name_size);
		if likely(filename) {
			char const *name_base; /* Relative module import. */
			name_base = DeeSystem_BaseName(filename, name_size);
			name_size = (size_t)(name_base - filename);

			/* Interpret the module name relative to the path of the current source file. */
			result = DeeModule_OpenRelative(Dee_AsObject(module_name),
			                                filename, name_size,
			                                inner_compiler_options,
			                                false);
			goto module_opened;
		}
	}
	result = DeeModule_OpenGlobal(Dee_AsObject(module_name),
	                              inner_compiler_options,
	                              false);
module_opened:
	if unlikely(!ITER_ISOK(result)) {
		if unlikely(!result)
			goto err;
		if (WARNAT(loc, W_MODULE_NOT_FOUND, module_name))
			goto err;
		result = &DeeModule_Empty;
		Dee_Incref(result);
	} else {
		/* Check for recursive dependency. */
		if (!(result->mo_flags & Dee_MODULE_FDIDLOAD) &&
		    result != current_rootscope->rs_module) {
			PERRAT(loc, W_RECURSIVE_MODULE_DEPENDENCY,
			       result->mo_name, current_rootscope->rs_module->mo_name);
			Dee_Decref(result);
			goto err;
		}
	}
	return result;
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

INTERN WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL
import_module_symbol(DeeModuleObject *__restrict mod,
                     struct TPPKeyword *__restrict name) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash = Dee_HashUtf8(name->k_name, name->k_size);
	perturb = i = Dee_MODULE_HASHST(mod, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(mod, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (!Dee_MODULE_SYMBOL_EQUALS(item, name->k_name, name->k_size))
			continue; /* Differing strings. */
		return item;  /* Found it! */
	}
	return NULL;
}

/* @return:  1: OK (the parsed object most certainly was a module)
 * @return:  0: OK
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_parse_module_name(struct Dee_unicode_printer *__restrict printer,
                      bool for_alias) {
	int result = 0;
	for (;;) {
		if (TOK_ISDOT(tok)) {
			if (Dee_unicode_printer_printascii(printer, "...", dot_count(tok)) < 0)
				goto err;
			result = 1;
			if unlikely(yield() < 0)
				goto err;
			if (Dee_UNICODE_PRINTER_LENGTH(printer) == 1 &&
			    (!TPP_ISKEYWORD(tok) && tok != TOK_STRING &&
			     (tok != TOK_CHAR || HAS(EXT_CHARACTER_LITERALS)) &&
			     !TOK_ISDOT(tok)))
				break; /* Special case: `.' is a valid name for the current module. */
		} else if (TPP_ISKEYWORD(tok)) {
			/* Warn about reserved identifiers.
			 * -> Reserved identifiers should be written as strings. */
			if (is_reserved_symbol_name(token.t_kwd) &&
			    WARN(for_alias ? W_RESERVED_IDENTIFIER_IN_MODULE_NAME
			                   : W_RESERVED_IDENTIFIER_IN_MODULE_NAME_NOALIAS,
			         token.t_kwd))
				goto err;
			if (Dee_unicode_printer_print(printer,
			                              token.t_kwd->k_name,
			                              token.t_kwd->k_size) < 0)
				goto err;
			if unlikely(yield() < 0)
				goto err;
			if (!TOK_ISDOT(tok))
				break;
		} else if (tok == TOK_STRING ||
		           (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
			if (ast_decode_unicode_string(printer) < 0)
				goto err;
			if unlikely(yield() < 0)
				goto err;
			if (!TOK_ISDOT(tok) && tok != TOK_STRING &&
			    (tok != TOK_CHAR || HAS(EXT_CHARACTER_LITERALS)))
				break;
		} else {
			if (WARN(W_EXPECTED_DOTS_KEYWORD_OR_STRING_IN_IMPORT_LIST))
				goto err;
			break;
		}
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_parse_symbol_name(struct Dee_unicode_printer *__restrict printer,
                      bool for_alias) {
	int result = 0;
	if (TPP_ISKEYWORD(tok)) {
		/* Warn about reserved identifiers.
		 * -> Reserved identifiers should be written as string imports. */
		if (is_reserved_symbol_name(token.t_kwd) &&
		    WARN(for_alias ? W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME
		                   : W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME_NOALIAS,
		         token.t_kwd))
			goto err;
		if (Dee_unicode_printer_print(printer,
		                              token.t_kwd->k_name,
		                              token.t_kwd->k_size) < 0)
			goto err;
		if unlikely(yield() < 0)
			goto err;
	} else if (tok == TOK_STRING ||
	           (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
		do {
			if (ast_decode_unicode_string(printer) < 0)
				goto err;
			if unlikely(yield() < 0)
				goto err;
		} while (tok == TOK_STRING ||
		         (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS)));
	} else {
		if (WARN(W_EXPECTED_KEYWORD_OR_STRING_IN_IMPORT_LIST))
			goto err;
	}
	return result;
err:
	return -1;
}


/* Parse a module name and return the associated module object.
 * @param: for_alias: Should be `true' if the name is used in `foo = <name>',
 *                    or if no alias can be used where the name appears,
 *                    else `false'
 * @return: * :             The named module
 * @return: NULL:           Error was thrown
 * @return: MODULE_CURRENT: The module currently being compiled */
INTERN WUNUSED DREF DeeModuleObject *DCALL
parse_module_byname(bool for_alias) {
	DREF DeeModuleObject *result;
	DREF DeeStringObject *module_name;
	struct Dee_unicode_printer name = Dee_UNICODE_PRINTER_INIT;
	struct ast_loc loc;
	loc_here(&loc);
	if unlikely(ast_parse_module_name(&name, for_alias) < 0)
		goto err_printer;
	module_name = (DREF DeeStringObject *)Dee_unicode_printer_pack(&name);
	if unlikely(!module_name)
		goto err;
	result = import_module_by_name(module_name, &loc);
	Dee_Decref(module_name);
	return result;
err_printer:
	Dee_unicode_printer_fini(&name);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) struct symbol *DFCALL
ast_parse_import_single_sym(struct TPPKeyword *__restrict import_name) {
	DREF DeeModuleObject *mod;
	struct symbol *extern_symbol;
	struct Dee_module_symbol *modsym;

	/* Parse the name of the module from which to import a symbol. */
	mod = parse_module_byname(true);
	if unlikely(!mod)
		goto err;

	/* We've got the module. - Now just create an anonymous symbol. */
	extern_symbol = new_unnamed_symbol();
	if unlikely(!extern_symbol)
		goto err_module;

	/* Lookup the symbol which we're importing. */
	if (mod == MODULE_CURRENT) {
		if (WARN(W_IMPORT_GLOBAL_FROM_OWN_MODULE))
			goto err;
		extern_symbol->s_type = SYMBOL_TYPE_MYMOD;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		Dee_Decref(mod);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	} else {
		modsym = import_module_symbol(mod, import_name);
		if unlikely(!modsym) {
			if (WARN(W_MODULE_IMPORT_NOT_FOUND,
			         import_name->k_name,
			         DeeModule_GetShortName(mod)))
				goto err_module;
			extern_symbol->s_type   = SYMBOL_TYPE_MODULE;
			extern_symbol->s_module = mod; /* Inherit reference. */
		} else {
			/* Setup this symbol as an external reference. */
			extern_symbol->s_type            = SYMBOL_TYPE_EXTERN;
			extern_symbol->s_extern.e_module = mod; /* Inherit reference. */
			extern_symbol->s_extern.e_symbol = modsym;
		}
	}
	return extern_symbol;
err_module:
	decref_parse_module_byname(mod);
	/*goto err;*/
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_import_single(struct TPPKeyword *__restrict import_name) {
	struct symbol *extern_symbol;
	extern_symbol = ast_parse_import_single_sym(import_name);
	if unlikely(!extern_symbol)
		goto err;
	/* Return the whole thing as a symbol-ast. */
	return ast_sym(extern_symbol);
err:
	return NULL;
}



struct import_item {
	struct ast_loc        ii_import_loc;  /* Parser location of `ii_import_name' */
	struct TPPKeyword    *ii_symbol_name; /* [1..1] The name by which the item should be imported. */
	DREF DeeStringObject *ii_import_name; /* [0..1] The name of the object being imported.
	                                       * When NULL, `ii_symbol_name' is used instead. */
};

/* Return `bar' for a module name `.foo.bar', etc. */
PRIVATE struct TPPKeyword *DCALL
get_module_symbol_name(DeeStringObject *__restrict module_name, bool is_module) {
	char const *utf8_repr, *symbol_start;
	size_t symbol_length;
	utf8_repr = DeeString_AsUtf8(Dee_AsObject(module_name));
	if unlikely(!utf8_repr)
		goto err;
	symbol_start  = (char const *)memrend(utf8_repr, '.', WSTR_LENGTH(utf8_repr)) + 1;
	symbol_length = (size_t)((utf8_repr + WSTR_LENGTH(utf8_repr)) - symbol_start);

	/* Make sure that the symbol name is valid. */
	{
		char const *iter, *end;
		end = (iter = symbol_start) + symbol_length;
		if unlikely(!symbol_length)
			goto bad_symbol_name;
		for (; iter < end; ++iter) {
			uint32_t ch;
			Dee_uniflag_t flags;
			ch    = Dee_unicode_readutf8_n(&iter, end);
			flags = DeeUni_Flags(ch);
			if (iter == symbol_start ? !(flags & Dee_UNICODE_ISSYMSTRT)
			                         : !(flags & Dee_UNICODE_ISSYMCONT)) {
bad_symbol_name:
				if (is_module) {
					if (WARN(W_INVALID_NAME_FOR_MODULE_SYMBOL,
					         module_name, symbol_length, symbol_start))
						goto err;
				} else {
					if (WARN(W_INVALID_NAME_FOR_IMPORT_SYMBOL,
					         module_name, symbol_length, symbol_start))
						goto err;
				}
				break;
			}
		}
	}

	/* Lookup/create a keyword for the module's symbol name. */
	return TPPLexer_LookupKeyword(symbol_start, symbol_length, 1);
err:
	return NULL;
}


/* @return:  2: Nothing was parsed
 * @return:  1: OK (when `allow_module_name' is true, a module import was parsed)
 * @return:  0: OK
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
parse_import_symbol(struct import_item *__restrict result,
                    bool allow_module_name) {
	struct Dee_unicode_printer printer;
	int return_value = 0;
	loc_here(&result->ii_import_loc);
	if (TPP_ISKEYWORD(tok)) {
		/* - `foo'
		 * - `foo = bar'
		 * - `foo = .foo.bar'
		 * - `foo = "bar"'
		 * - `foo as bar'
		 * - `foo.bar'
		 * - `foo.bar as foobar' */
		result->ii_symbol_name = token.t_kwd;
		if unlikely(yield() < 0)
			goto err;
		if (tok == '=') {
			/* - `foo = bar'
			 * - `foo = .foo.bar'
			 * - `foo = "bar"' */
			if (is_reserved_symbol_name(result->ii_symbol_name) &&
			    WARNAT(&result->ii_import_loc, W_RESERVED_IDENTIFIER_IN_ALIAS_NAME, result->ii_symbol_name))
				goto err;
			if unlikely(yield() < 0)
				goto err;
			Dee_unicode_printer_init(&printer);
			loc_here(&result->ii_import_loc);
			return_value = allow_module_name
			               ? ast_parse_module_name(&printer, true)
			               : ast_parse_symbol_name(&printer, true);
			if unlikely(return_value < 0)
				goto err_printer;
			result->ii_import_name = (DREF DeeStringObject *)Dee_unicode_printer_pack(&printer);
			if unlikely(!result->ii_import_name)
				goto err;
		} else if (tok == KWD_as) {
			if (is_reserved_symbol_name(result->ii_symbol_name) &&
			    WARNAT(&result->ii_import_loc, W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME, result->ii_symbol_name))
				goto err;
			if unlikely(yield() < 0)
				goto err;
			/* - `foo as bar' */
			if (TPP_ISKEYWORD(tok)) {
				result->ii_import_name = (DREF DeeStringObject *)DeeString_NewUtf8(result->ii_symbol_name->k_name,
				                                                                   result->ii_symbol_name->k_size,
				                                                                   STRING_ERROR_FSTRICT);
				if unlikely(!result->ii_import_name)
					goto err;
				result->ii_symbol_name = token.t_kwd;
				if (is_reserved_symbol_name(token.t_kwd) &&
				    WARN(W_RESERVED_IDENTIFIER_IN_ALIAS_NAME, token.t_kwd))
					goto err;
				if unlikely(yield() < 0)
					goto err;
			} else {
				if (WARN(W_EXPECTED_KEYWORD_AFTER_AS))
					goto err;
				result->ii_import_name = NULL;
			}
		} else if (TOK_ISDOT(tok) && allow_module_name) {
			/* - `foo.bar'
			 * - `foo.bar as foobar' */
			if (is_reserved_symbol_name(result->ii_symbol_name) &&
			    WARNAT(&result->ii_import_loc, W_RESERVED_IDENTIFIER_IN_MODULE_NAME, result->ii_symbol_name))
				goto err;
			Dee_unicode_printer_init(&printer);
			if unlikely(Dee_unicode_printer_print(&printer,
			                                      result->ii_symbol_name->k_name,
			                                      result->ii_symbol_name->k_size) < 0)
				goto err_printer;
			goto complete_module_name;
		} else {
			if (is_reserved_symbol_name(result->ii_symbol_name) &&
			    WARNAT(&result->ii_import_loc,
			           allow_module_name ? W_RESERVED_IDENTIFIER_IN_SYMBOL_OR_MODULE_NAME
			                             : W_RESERVED_IDENTIFIER_IN_SYMBOL_NAME,
			           result->ii_symbol_name))
				goto err;
			result->ii_import_name = NULL;
		}
	} else if (TOK_ISDOT(tok) && allow_module_name) {
		/* - `.foo.bar'
		 * - `.foo.bar as foobar' */
		Dee_unicode_printer_init(&printer);
complete_module_name:
		return_value = 1;
		if unlikely(Dee_unicode_printer_printascii(&printer, "...", dot_count(tok)) < 0)
			goto err_printer;
		if unlikely(yield() < 0)
			goto err_printer;
		/* Make sure to properly parse `import . as me' */
		if ((TPP_ISKEYWORD(tok) && tok != KWD_as) ||
		    TOK_ISDOT(tok) || tok == TOK_STRING ||
		    (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
			if unlikely(ast_parse_module_name(&printer, true) < 0)
				goto err_printer;
		}
		result->ii_import_name = (DREF DeeStringObject *)Dee_unicode_printer_pack(&printer);
		if unlikely(!result->ii_import_name)
			goto err;
		if (tok == KWD_as) {
			/* - `.foo.bar as foobar' */
			if unlikely(yield() < 0)
				goto err_name;
			if unlikely(!TPP_ISKEYWORD(tok)) {
				if (WARN(W_EXPECTED_KEYWORD_AFTER_AS))
					goto err_name;
				goto autogenerate_symbol_name;
			}
			result->ii_symbol_name = token.t_kwd;
			/* Warn about reserved identifiers */
			if (is_reserved_symbol_name(token.t_kwd) &&
			    WARN(W_RESERVED_IDENTIFIER_IN_ALIAS_NAME, token.t_kwd))
				goto err_name;
			if unlikely(yield() < 0)
				goto err_name;
		} else {
			/* - `.foo.bar' */
autogenerate_symbol_name:
			/* Autogenerate the module import symbol name. */
			result->ii_symbol_name = get_module_symbol_name(result->ii_import_name,
			                                                return_value != 0);
			if unlikely(!result->ii_symbol_name)
				goto err_name;
			/* Warn about the auto-generated name being a reserved identifiers */
			if (is_reserved_symbol_name(result->ii_symbol_name) &&
			    WARNAT(&result->ii_import_loc,
			           allow_module_name ? W_RESERVED_IDENTIFIER_IN_AUTOGENERATED_SYMBOL_OR_MODULE_NAME
			                             : W_RESERVED_IDENTIFIER_IN_AUTOGENERATED_SYMBOL_NAME,
			           result->ii_symbol_name))
				goto err_name;
		}
	} else if (tok == TOK_STRING ||
	           (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
		/* - `"foo"'
		 * - `"foo" as foobar'
		 * - `"foo.bar"'
		 * - `"foo.bar" as foobar' */
		Dee_unicode_printer_init(&printer);
		return_value = allow_module_name
		               ? ast_parse_module_name(&printer, true)
		               : ast_parse_symbol_name(&printer, true);
		if unlikely(return_value < 0)
			goto err_printer;
		result->ii_import_name = (DREF DeeStringObject *)Dee_unicode_printer_pack(&printer);
		if unlikely(!result->ii_import_name)
			goto err;
		if (tok != KWD_as)
			goto autogenerate_symbol_name;

		/* An import alias was given. */
		if unlikely(yield() < 0)
			goto err_name;
		if (!TPP_ISKEYWORD(tok)) {
			if (WARN(W_EXPECTED_KEYWORD_AFTER_AS))
				goto err_name;
			goto autogenerate_symbol_name;
		}
		result->ii_symbol_name = token.t_kwd;

		/* Warn about reserved identifiers in alias names. */
		if (is_reserved_symbol_name(token.t_kwd) &&
		    WARN(W_RESERVED_IDENTIFIER_IN_ALIAS_NAME, token.t_kwd))
			goto err_name;
		if unlikely(yield() < 0)
			goto err_name;
	} else {
		if (WARN(W_EXPECTED_KEYWORD_OR_STRING_IN_IMPORT_LIST))
			goto err;
		return_value = 2;
	}
	return return_value;
err_printer:
	Dee_unicode_printer_fini(&printer);
	goto err;
err_name:
	Dee_Decref(result->ii_import_name);
err:
	return -1;
}

PRIVATE NONNULL((1)) int DCALL
ast_import_all_from_module(DeeModuleObject *__restrict mod,
                           struct ast_loc *loc) {
	struct Dee_module_symbol *iter, *end;
	ASSERT_OBJECT_TYPE(mod, &DeeModule_Type);
	if (mod == MODULE_CURRENT) {
		if (WARNAT(loc, W_IMPORT_GLOBAL_FROM_OWN_MODULE))
			goto err;
		goto done;
	}
	end = (iter = mod->mo_bucketv) + (mod->mo_bucketm + 1);
	for (; iter < end; ++iter) {
		struct symbol *sym;
		struct TPPKeyword *name;
		if (!Dee_MODULE_SYMBOL_GETNAMESTR(iter))
			continue; /* Empty slot. */
		if (iter->ss_flags & Dee_MODSYM_FHIDDEN)
			continue; /* Hidden symbol. */
		name = TPPLexer_LookupKeyword(Dee_MODULE_SYMBOL_GETNAMESTR(iter),
		                              Dee_MODULE_SYMBOL_GETNAMELEN(iter),
		                              1);
		if unlikely(!name)
			goto err;
		sym = get_local_symbol(name);
		if unlikely(sym) {
			/* Re-importing a different symbol that would collide with another
			 * weak symbol will create an ambiguity when that symbol is used.
			 * Instead of producing an error here and now, only do so if the
			 * symbol ever turns out to be used.
			 * That way, modules exporting symbols with the same name can still
			 * both be used in the same namespace, with all symbols from both
			 * imported using `import *'
			 * Additionally, `SYMBOL_TYPE_AMBIG' symbols are weak, meaning that
			 * in a situation where 2 modules `a' and `b' both define `foo', you
			 * can write:
			 * >> import * from a;
			 * >> import * from b;
			 * >> import foo from b;  // Explicitly link `foo' to be import from the desired module.
			 */
			if (!SYMBOL_IS_WEAK(sym))
				continue; /* Not a weakly declared symbol. */

			/* Special handling for re-imports. */
			if (sym->s_type == SYMBOL_TYPE_EXTERN) {
				if (sym->s_extern.e_module == mod) {
					if (sym->s_extern.e_symbol == iter)
						continue; /* Same declaration. */
				} else {
					/* TODO: Special handling when aliasing `Dee_MODSYM_FEXTERN'-symbols.
					 *       Importing an external symbol that has been aliased should
					 *       not cause ambiguity if it is the original symbol with which
					 *       the new one would collide (this goes if at least either the
					 *       old, or the new symbol has the `Dee_MODSYM_FEXTERN' flag)
					 */

					/* Special case: When both the old and new symbol refers to an external `final global'
					 * variable (with neither being `varying'), then ensure that the modules have been
					 * initialized, and check if the values bound for the symbols differ.
					 *
					 * If they are identical, we're allowed to assume that they simply alias each other,
					 * in which case it doesn't really matter which one we use for binding. However in the
					 * interest of minimizing dependencies, if either module is the builtin `deemon' module,
					 * bind against the symbol as found in `deemon', otherwise check if either module uses
					 * the other, in which case: bind against the symbol of the module being used (aka.
					 * further down in the dependency tree) */
					if ((sym->s_extern.e_symbol->ss_flags & (Dee_MODSYM_FREADONLY | Dee_MODSYM_FCONSTEXPR |
					                                         Dee_MODSYM_FPROPERTY | Dee_MODSYM_FEXTERN)) ==
					    /*                               */ (Dee_MODSYM_FREADONLY | Dee_MODSYM_FCONSTEXPR) &&
					    (iter->ss_flags & (Dee_MODSYM_FREADONLY | Dee_MODSYM_FCONSTEXPR |
					                       Dee_MODSYM_FPROPERTY | Dee_MODSYM_FEXTERN)) ==
					    /*             */ (Dee_MODSYM_FREADONLY | Dee_MODSYM_FCONSTEXPR)) {
						/* Both symbols are non-varying (allowing value inlining).
						 * -> Make sure both modules have been loaded, and compare the values that have been bound.
						 * NOTE: For this purpose, we must perform an exact comparison (i.e. `a === b') */
						int error;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
						error = DeeModule_Initialize(mod);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
						error = DeeModule_RunInit(mod);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
						if unlikely(error < 0)
							goto err;
						if (error == 0) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
							error = DeeModule_Initialize(sym->s_extern.e_module);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
							error = DeeModule_RunInit(sym->s_extern.e_module);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
							if unlikely(error < 0)
								goto err;
							if (error == 0) {
								/* Both modules are now initialized. */
								DeeObject *old_val, *new_val;
								DeeModule_LockRead(mod);
								old_val = mod->mo_globalv[Dee_module_symbol_getindex(iter)];
								DeeModule_LockEndRead(mod);
								DeeModule_LockRead(sym->s_extern.e_module);
								new_val = sym->s_extern.e_module->mo_globalv[Dee_module_symbol_getindex(sym->s_extern.e_symbol)];
								DeeModule_LockEndRead(sym->s_extern.e_module);
								if (old_val == new_val) {
									/* One of the modules contains a copy-alias (i.e. a secondary memory location)
									 * for the other symbol. - Try to figure out which one is aliasing which, and
									 * potentially re-assign the import such that the new module has less dependencies. */
									if (sym->s_extern.e_module != &DeeModule_Deemon) {
										if (mod == &DeeModule_Deemon) {
do_reassign_new_alias:
											Dee_Incref(mod);
											Dee_Decref(sym->s_extern.e_module);
											sym->s_extern.e_module = mod; /* Inherit reference */
											sym->s_extern.e_symbol = iter;
										} else {
											uint16_t i;

											/* Neither module is the builtin `deemon' module.
											 *
											 * Check if one of the modules is importing the other, or
											 * bind the new module if it doesn't have any imports. */
											if (!mod->mo_importc)
												goto do_reassign_new_alias;
											for (i = 0; i < sym->s_extern.e_module->mo_importc; ++i) {
												if (mod == sym->s_extern.e_module->mo_importv[i])
													goto do_reassign_new_alias;
											}
										}
									}
									continue;
								}
							}
						}
					}
				}
			}
			if (sym->s_type != SYMBOL_TYPE_AMBIG) {
				/* Turn the symbol into one that is ambiguous. */
				symbol_fini(sym);
				sym->s_type = SYMBOL_TYPE_AMBIG;
				if (loc) {
					memcpy(&sym->s_ambig.a_decl2, loc,
					       sizeof(struct ast_loc));
				} else {
					loc_here(&sym->s_ambig.a_decl2);
				}
				if (sym->s_ambig.a_decl2.l_file)
					TPPFile_Incref(sym->s_ambig.a_decl2.l_file);
				sym->s_ambig.a_declc = 0;
				sym->s_ambig.a_declv = NULL;
			} else {
				/* Add another ambiguous symbol declaration location. */
				symbol_addambig(sym, loc);
			}
		} else {
			sym = new_local_symbol(name, loc);
			if unlikely(!sym)
				goto err;

			/* Define this symbol as an import from this module. */
			sym->s_type = SYMBOL_TYPE_EXTERN;
			sym->s_flag |= SYMBOL_FWEAK; /* Symbols imported by `*' are defined weakly. */
			sym->s_extern.e_module = mod;
			sym->s_extern.e_symbol = iter;
			Dee_Incref(mod);
		}
	}
done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_import_single_from_module(DeeModuleObject *__restrict mod,
                              struct import_item *__restrict item) {
	struct Dee_module_symbol *sym;
	struct symbol *import_symbol;
	if (mod == MODULE_CURRENT) {
		if (WARNAT(&item->ii_import_loc, W_IMPORT_GLOBAL_FROM_OWN_MODULE))
			goto err;
		goto done;
	}
	if (item->ii_import_name) {
		sym = DeeModule_GetSymbol(mod, Dee_AsObject(item->ii_import_name));
		if (!sym) {
			if (WARNAT(&item->ii_import_loc, W_MODULE_IMPORT_NOT_FOUND,
			           DeeString_STR(item->ii_import_name),
			           DeeModule_GetShortName(mod)))
				goto err;
			goto done;
		}
	} else {
		sym = import_module_symbol(mod, item->ii_symbol_name);
		if (!sym) {
			if (WARNAT(&item->ii_import_loc, W_MODULE_IMPORT_NOT_FOUND,
			           item->ii_symbol_name->k_name,
			           DeeModule_GetShortName(mod)))
				goto err;
			goto done;
		}
	}
	import_symbol = get_local_symbol(item->ii_symbol_name);
	if unlikely(import_symbol) {
		if (SYMBOL_IS_WEAK(import_symbol)) {
			SYMBOL_CLEAR_WEAK(import_symbol);
			goto init_import_symbol;
		}

		/* Another symbol with the same name had already been imported. */
		if (import_symbol->s_type == SYMBOL_TYPE_EXTERN &&
		    import_symbol->s_extern.e_module == mod &&
		    import_symbol->s_extern.e_symbol == sym) {
			/* It was the same symbol that had been imported before.
			 * -> Ignore the secondary import! */
		} else {
			if (WARNAT(&item->ii_import_loc,
			           W_IMPORT_ALIAS_IS_ALREADY_DEFINED,
			           item->ii_symbol_name))
				goto err;
		}
	} else {
		import_symbol = new_local_symbol(item->ii_symbol_name,
		                                 &item->ii_import_loc);
		if unlikely(!import_symbol)
			goto err;
init_import_symbol:
		import_symbol->s_type            = SYMBOL_TYPE_EXTERN;
		import_symbol->s_extern.e_module = mod;
		import_symbol->s_extern.e_symbol = sym;
		Dee_Incref(mod);
	}
done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_import_module(struct import_item *__restrict item) {
	DREF DeeModuleObject *mod;
	struct symbol *import_symbol;
	if (item->ii_import_name) {
		mod = import_module_by_name(item->ii_import_name,
		                            &item->ii_import_loc);
	} else {
		DREF DeeStringObject *module_name;
		module_name = (DREF DeeStringObject *)DeeString_NewUtf8(item->ii_symbol_name->k_name,
		                                                        item->ii_symbol_name->k_size,
		                                                        STRING_ERROR_FSTRICT);
		if unlikely(!module_name)
			goto err;
		mod = import_module_by_name(module_name,
		                            &item->ii_import_loc);
		Dee_Decref(module_name);
	}
	if unlikely(!mod)
		goto err;
	import_symbol = get_local_symbol(item->ii_symbol_name);
	if unlikely(import_symbol) {
		if (SYMBOL_IS_WEAK(import_symbol)) {
			SYMBOL_CLEAR_WEAK(import_symbol);
			goto init_import_symbol;
		}

		/* Another symbol with the same name had already been imported. */
		if ((import_symbol->s_type == SYMBOL_TYPE_MODULE &&
		     import_symbol->s_extern.e_module == mod) ||
		    (mod == MODULE_CURRENT &&
		     import_symbol->s_type == SYMBOL_TYPE_MYMOD)) {
			/* The same module has already been imported under this name! */
		} else {
			if (WARNAT(&item->ii_import_loc,
			           W_IMPORT_ALIAS_IS_ALREADY_DEFINED,
			           item->ii_symbol_name))
				goto err_module;
		}
		decref_parse_module_byname(mod);
	} else {
		import_symbol = new_local_symbol(item->ii_symbol_name,
		                                 &item->ii_import_loc);
		if unlikely(!import_symbol)
			goto err_module;
init_import_symbol:
		if (mod == MODULE_CURRENT) {
			import_symbol->s_type = SYMBOL_TYPE_MYMOD;
			decref_parse_module_byname(mod);
		} else {
			import_symbol->s_type   = SYMBOL_TYPE_MODULE;
			import_symbol->s_module = mod; /* Inherit reference */
		}
	}
	return 0;
err_module:
	decref_parse_module_byname(mod);
err:
	return -1;
}


INTERN int DFCALL ast_parse_post_import(void) {
	/* - import deemon;
	 * - import deemon, util;
	 * - import my_deemon = deemon;
	 * - import my_deemon = "deemon";
	 * - import my_deemon = deemon, my_util = util;
	 * - import deemon as my_deemon;
	 * - import "deemon" as my_deemon;
	 * - import deemon as my_deemon, util as my_util;
	 * - import * from deemon;
	 * - import Object from deemon;
	 * - import Object, List from deemon;
	 * - import MyObject = Object from deemon;
	 * - import MyObject = "Object" from deemon;
	 * - import MyObject = Object, MyList = List from deemon;
	 * - import Object as MyObject from deemon;
	 * - import "Object" as MyObject from deemon;
	 * - import Object as MyObject, List as MyList from deemon; */
	int error;
	struct import_item item;
	bool allow_modules = true;
	struct ast_loc star_loc;
	struct import_item *item_v;
	size_t item_a, item_c;
	DREF DeeModuleObject *mod;
	star_loc.l_file = NULL; /* When non-NULL, import all */
	if (tok == '*') {
		loc_here(&star_loc);
		if unlikely(yield() < 0)
			goto err;
		if (tok == KWD_from) {
			if unlikely(yield() < 0)
				goto err;
			mod = parse_module_byname(true);
			if unlikely(!mod)
				goto err;
			error = ast_import_all_from_module(mod, &star_loc);
			decref_parse_module_byname(mod);
			goto done;
		} else if (tok == ',') {
			item_c = 0;
			item_a = 0;
			item_v = NULL;
			allow_modules = false;
			goto import_parse_list;
		}
		if (WARN(W_EXPECTED_COMMA_OR_FROM_AFTER_START_IN_IMPORT_LIST))
			goto err;
		goto done;
	}
	error = parse_import_symbol(&item, true);
	if unlikely(error < 0)
		goto err;
	if unlikely(error == 2)
		goto done;
	if (error) {
		/* Module import list */
		for (;;) {
import_item_as_module:
			error = ast_import_module(&item);
			Dee_XDecref(item.ii_import_name);
			if unlikely(error)
				goto err;
parse_module_import_list:
			if (tok != ',')
				break;
			if unlikely(yield() < 0)
				goto err;
			error = parse_import_symbol(&item, true);
			if unlikely(error < 0)
				goto err;
			if unlikely(error == 2)
				break;
		}

		/* Warn if the module import list is followed by a `from' */
		if (tok == KWD_from &&
		    WARN(W_UNEXPECTED_FROM_AFTER_MODULE_IMPORT_LIST))
			goto err;
	} else if (tok == KWD_from) {
		/*  - `import foo from bar' */
		if unlikely(yield() < 0)
			goto err_item;
		mod = parse_module_byname(true);
		if unlikely(!mod)
			goto err_item;
		error = ast_import_single_from_module(mod, &item);
		Dee_XDecref(item.ii_import_name);
		decref_parse_module_byname(mod);
		if unlikely(error)
			goto err;
	} else if (tok == ',') {
		item_a = 4;
		item_v = (struct import_item *)Dee_TryMallocc(4, sizeof(struct import_item));
		if unlikely(!item_v) {
			item_a = 2;
			item_v = (struct import_item *)Dee_Mallocc(2, sizeof(struct import_item));
			if unlikely(!item_v)
				goto err_item;
		}
		item_v[0] = item;
		item_c    = 1;
import_parse_list:
		do {
			ASSERT(tok == ',');
			if unlikely(yield() < 0)
				goto err_item_v;
			if (tok == '*') {
				if (star_loc.l_file) {
					if (WARN(W_UNEXPECTED_STAR_DUPLICATION_IN_IMPORT_LIST))
						goto err_item_v;
				} else {
					loc_here(&star_loc);
				}
				if unlikely(yield() < 0)
					goto err_item_v;
				/* Don't allow modules after `*' confirmed that symbols are being imported.
				 * -> There is no such thing as import-all-modules. */
				allow_modules = false;
			} else {
				/* Parse the next import item. */
				ASSERT(item_c <= item_a);
				if (item_c >= item_a) {
					/* Allocate more space. */
					struct import_item *new_item_v;
					size_t new_item_a = item_a * 2;
					if unlikely(!new_item_a)
						new_item_a = 2;
					new_item_v = (struct import_item *)Dee_TryReallocc(item_v, new_item_a,
					                                                   sizeof(struct import_item));
					if unlikely(!new_item_v) {
						new_item_a = item_c + 1;
						new_item_v = (struct import_item *)Dee_Reallocc(item_v, new_item_a,
						                                                sizeof(struct import_item));
						if unlikely(!new_item_v)
							goto err_item_v;
					}
					item_v = new_item_v;
					item_a = new_item_a;
				}
				error = parse_import_symbol(&item_v[item_c], allow_modules);
				if unlikely(error < 0)
					goto err_item_v;
				if unlikely(error == 2)
					break;
				++item_c; /* Import parsing confirmed. */
				if (error) {
					/* We're dealing with a module import list!
					 * -> Import all items already parsed as modules. */
					size_t i;
					ASSERT(allow_modules != false);
					for (i = 0; i < item_c; ++i) {
						if unlikely(ast_import_module(&item_v[i]))
							goto err_item_v;
						Dee_XClear(item_v[i].ii_import_name);
					}
					Dee_Free(item_v);
					/* Then continue by parsing the remainder as a module import list. */
					goto parse_module_import_list;
				}
			}
		} while (tok == ',');

		/* A multi-item, comma-separated import list has now been parsed. */
		if (tok == KWD_from) {
			size_t i;

			/* import foo, bar, foobar from foobarfoo;  (symbol import) */
			if unlikely(yield() < 0)
				goto err_item_v;
			mod = parse_module_byname(true);
			if unlikely(!mod)
				goto err_item_v;

			/* If `*' was apart of the symbol import list,
			 * start by importing all symbols from the module. */
			if (star_loc.l_file) {
				if unlikely(ast_import_all_from_module(mod, &star_loc))
					goto err_item_v_module;
			}

			/* Now import all the explicitly defined symbols. */
			for (i = 0; i < item_c; ++i) {
				if unlikely(ast_import_single_from_module(mod, &item_v[i]))
					goto err_item_v_module;
				Dee_XClear(item_v[i].ii_import_name);
			}
			decref_parse_module_byname(mod);
		} else {
			size_t i;
			if unlikely(!allow_modules) {
				/* Warn if there is a `from' missing following a symbol import list. */
				if (WARN(W_EXPECTED_FROM_AFTER_SYMBOL_IMPORT_LIST))
					goto err_item_v;
			}

			/* import foo, bar, foobar;  (module import) */
			for (i = 0; i < item_c; ++i) {
				if unlikely(ast_import_module(&item_v[i]))
					goto err_item_v;
				Dee_XClear(item_v[i].ii_import_name);
			}
		}
		Dee_Free(item_v);
		goto done;
err_item_v_module:
		decref_parse_module_byname(mod);
err_item_v:
		while (item_c--)
			Dee_XDecref(item_v[item_c].ii_import_name);
		Dee_Free(item_v);
		goto err;
err_item:
		Dee_XDecref(item.ii_import_name);
		goto err;
	} else {
		/* This is simply a single-module, stand-along import statement. */
		goto import_item_as_module;
	}
done:
	return 0;
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_import_expression_after_import(struct ast_loc *__restrict import_loc);

/* Same as `ast_parse_try_hybrid' but for import statements / expressions. */
INTERN WUNUSED DREF struct ast *DFCALL
ast_parse_import_hybrid(unsigned int *p_was_expression) {
	DREF struct ast *result;
	struct ast_loc import_loc;
	ASSERT(tok == KWD_import);
	loc_here(&import_loc);
	if unlikely(yield() < 0)
		goto err;
	if (tok == '(' || tok == KWD_pack) {
		/* `import', as seen in expressions. */
		result = ast_parse_import_expression_after_import(&import_loc);
		if unlikely(!result)
			goto err;
		result = ast_parse_postexpr(result);
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_YES;
	} else {
		result = ast_constexpr(Dee_None);
		result = ast_setddi(result, &import_loc);
		if unlikely(!result)
			goto err;
		if unlikely(ast_parse_post_import())
			goto err_r;
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_NO;
	}
	return result;
err_r:
	ast_decref(result);
err:
	return NULL;
}


INTERN WUNUSED DREF struct ast *DFCALL ast_parse_import(void) {
	DREF DeeModuleObject *mod;
	DREF struct ast *result;
	struct ast_loc import_loc;

	/* There are many valid ways of writing import statements:
	 *  - import("deemon");
	 *  - import pack "deemon";
	 *  - import deemon;
	 *  - import deemon, util;
	 *  - import my_deemon = deemon;
	 *  - import my_deemon = "deemon";
	 *  - import my_deemon = deemon, my_util = util;
	 *  - import deemon as my_deemon;
	 *  - import "deemon" as my_deemon;
	 *  - import deemon as my_deemon, util as my_util;
	 *  - import * from deemon;
	 *  - import Object from deemon;
	 *  - import Object, List from deemon;
	 *  - import MyObject = Object from deemon;
	 *  - import MyObject = "Object" from deemon;
	 *  - import MyObject = Object, MyList = List from deemon;
	 *  - import Object as MyObject from deemon;
	 *  - import "Object" as MyObject from deemon;
	 *  - import Object as MyObject, List as MyList from deemon;
	 *  - from deemon import *;
	 *  - from deemon import Object;
	 *  - from deemon import Object, List;
	 *  - from deemon import MyObject = Object;
	 *  - from deemon import MyObject = "Object";
	 *  - from deemon import MyObject = Object, MyList = List;
	 *  - from deemon import Object as MyObject;
	 *  - from deemon import "Object" as MyObject;
	 *  - from deemon import Object as MyObject, List as MyList;
	 * ----
	 *  // NOTE: `keyword' may not be followed by `string', and
	 *  //       `string' may not be followed by `keyword'
	 *  module_name ::= (
	 *      (string | '.' | keyword)...
	 *  );
	 *  symbol_name ::= (
	 *       keyword |
	 *       string...
	 *  );
	 *  symbol_import ::= (
	 *      (['global' | 'local'] symbol_name) |
	 *      (['global' | 'local'] keyword '=' symbol_name) |
	 *      (symbol_name 'as' ['global' | 'local'] keyword)
	 *  );
	 *  rule_import ::= (
	 *       ('import' '(' string ')') |
	 *
	 *       (['global' | 'local'] 'import' ',' ~~ (
	 *          (['global' | 'local'] module_name) |
	 *          (['global' | 'local'] keyword '=' module_name) |
	 *          (module_name 'as' ['global' | 'local'] keyword)
	 *       )...) |
	 *
	 *       (['global' | 'local'] 'import' ',' ~~ (
	 *          '*' | symbol_import
	 *       )... 'from' module_name) |
	 *
	 *       ('from' module_name 'import' ',' ~~ (
	 *          '*' | symbol_import
	 *       )...)
	 *  );
	 * ----
	 * NOTES:
	 *   - Importing the same symbol under the same
	 *     name twice is allowed and behaves as a no-op.
	 *   - A `global' prefix can be used to forward an external
	 *     symbol as an export of the current module, which then
	 *     refers to the same variable as was originally defined
	 *     by the module from which symbols are being imported.
	 *     The default is `local', and `global' can only be used
	 *     which the root scope.
	 */
	ASSERT(tok == KWD_import || tok == KWD_from);
	loc_here(&import_loc);
	if (tok == KWD_from) {
		/* - from deemon import *;
		 * - from deemon import Object;
		 * - from deemon import Object, List;
		 * - from deemon import MyObject = Object;
		 * - from deemon import MyObject = "Object";
		 * - from deemon import MyObject = Object, MyList = List;
		 * - from deemon import Object as MyObject;
		 * - from deemon import "Object" as MyObject;
		 * - from deemon import Object as MyObject, List as MyList; */
		bool has_star = false;
		result = ast_constexpr(Dee_None);
		result = ast_setddi(result, &import_loc);
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
		mod = parse_module_byname(true);
		if unlikely(!mod)
			goto err_r;

		/* All right! we've got the module. */
		if (skip(KWD_import, W_EXPECTED_IMPORT_AFTER_FROM))
			goto err_r_module;
		for (;;) {
			/* Parse an entire import list. */
			if (tok == '*') {
				if (has_star &&
				    WARN(W_UNEXPECTED_STAR_DUPLICATION_IN_IMPORT_LIST))
					goto err_r_module;
				if unlikely(ast_import_all_from_module(mod, NULL))
					goto err_r_module;
				if unlikely(yield() < 0)
					goto err_r_module;
				has_star = true;
			} else {
				int error;
				struct import_item item;
				error = parse_import_symbol(&item, false);
				if unlikely(error < 0)
					goto err_r_module;
				if unlikely(error == 2)
					break; /* failed */
				error = ast_import_single_from_module(mod, &item);
				Dee_XDecref(item.ii_import_name);
				if unlikely(error)
					goto err_r_module;
			}
			if (tok != ',')
				break;
			if unlikely(yield() < 0)
				goto err_r_module;
		}
		decref_parse_module_byname(mod);
	} else {
		/* - import("deemon");
		 * - import pack "deemon";
		 * - import deemon;
		 * - import deemon, util;
		 * - import my_deemon = deemon;
		 * - import my_deemon = "deemon";
		 * - import my_deemon = deemon, my_util = util;
		 * - import deemon as my_deemon;
		 * - import "deemon" as my_deemon;
		 * - import deemon as my_deemon, util as my_util;
		 * - import * from deemon;
		 * - import Object from deemon;
		 * - import Object, List from deemon;
		 * - import MyObject = Object from deemon;
		 * - import MyObject = "Object" from deemon;
		 * - import MyObject = Object, MyList = List from deemon;
		 * - import Object as MyObject from deemon;
		 * - import "Object" as MyObject from deemon;
		 * - import Object as MyObject, List as MyList from deemon; */
		ASSERT(tok == KWD_import);
		if unlikely(yield() < 0)
			goto err;
		if (tok == '(' || tok == KWD_pack) {
			/* `import', as seen in expressions. */
			result = ast_parse_import_expression_after_import(&import_loc);
			if unlikely(!result)
				goto err;
			result = ast_parse_postexpr(result);
			goto done;
		}
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		if (tok == '.') {
			/* FIXME: Ambiguity:
			 * >> import .foo.bar.baz;  // same as: local bar = import(".foo.bar");
			 * vs:
			 * >> import.foo.bar.baz;   // same as: import("foo").bar.baz;
			 *
			 * The only real solution that I can see is so compile the expression
			 * in both ways, and see if one of the ways works without causing any
			 * compiler errors due to module-not-found.
			 * In most cases, exactly one way should work, and if both ways end up
			 * working, issue warning so the user re-writes the code so one of:
			 * >> (import.foo.bar.baz);      // for `import("foo").bar.baz;'
			 * >> import baz = .foo.bar.baz; // for `local bar = import(".foo.bar");'
			 *
			 * Alternatively, actually make use of the presence of that singular " "
			 * between "import" and "." to differentiate between the two meanings...
			 */
		}
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

		result = ast_constexpr(Dee_None);
		result = ast_setddi(result, &import_loc);
		if unlikely(!result)
			goto err;
		if unlikely(ast_parse_post_import())
			goto err_r;
	}
done:
	return result;
err_r_module:
	decref_parse_module_byname(mod);
err_r:
	ast_decref(result);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_IMPORT_C */
