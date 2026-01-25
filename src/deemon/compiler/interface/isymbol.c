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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ISYMBOL_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ISYMBOL_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack0, DeeArg_Unpack1 */
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Print, DeeFormat_Printf */
#include <deemon/none.h>               /* DeeNone_NewRef */
#include <deemon/object.h>
#include <deemon/string.h>             /* DeeString*, STRING_ERROR_FIGNORE */
#include <deemon/system-features.h>    /* access, read, write */

#include "../../runtime/strings.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

typedef DeeCompilerSymbolObject Symbol;

INTDEF DeeStringObject *tpconst symbol_type_names[];
INTERN_CONST DeeStringObject *tpconst symbol_type_names[] = {
	/* [SYMBOL_TYPE_NONE]   = */ &str_none,
	/* [SYMBOL_TYPE_GLOBAL] = */ &str_global,
	/* [SYMBOL_TYPE_EXTERN] = */ &str_extern,
	/* [SYMBOL_TYPE_MODULE] = */ &str_module,
	/* [SYMBOL_TYPE_MYMOD]  = */ &str_mymod,
	/* [SYMBOL_TYPE_GETSET] = */ &str_getset,
	/* [SYMBOL_TYPE_IFIELD] = */ &str_ifield,
	/* [SYMBOL_TYPE_CFIELD] = */ &str_cfield,
	/* [SYMBOL_TYPE_ALIAS]  = */ &str_alias,
	/* [SYMBOL_TYPE_ARG]    = */ &str_arg,
	/* [SYMBOL_TYPE_LOCAL]  = */ &str_local,
	/* [SYMBOL_TYPE_STACK]  = */ &str_stack,
	/* [SYMBOL_TYPE_STATIC] = */ &str_static,
	/* [SYMBOL_TYPE_EXCEPT] = */ &str_except,
	/* [SYMBOL_TYPE_MYFUNC] = */ &str_myfunc,
	/* [SYMBOL_TYPE_THIS]   = */ &str_this,
	/* [SYMBOL_TYPE_AMBIG]  = */ &str_ambig,
	/* [SYMBOL_TYPE_FWD]    = */ &str_fwd,
	/* [SYMBOL_TYPE_CONST]  = */ &str_const
};

PRIVATE WUNUSED NONNULL((1)) uint16_t DCALL
get_symbol_kind_from_name(char const *__restrict name) {
	/* TODO */

	DeeError_Throwf(&DeeError_ValueError,
	                "Unknown symbol kind %q",
	                name);
	return (uint16_t)-1;
}


/* SYMBOL_TYPE_ARG: Since symbols are also referenced by the base-scope's argument vector,
 *                  it is assumed that these symbols will not appear anywhere else, or be
 *                  changed to some else (with the exception of the assembler being allowed
 *                  to change them) */
#define SYMBOL_TYPE_IS_IMMUTABLE(x) ((x) == SYMBOL_TYPE_ARG)

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
symbol_getkind(DeeCompilerSymbolObject *__restrict self) {
	DREF DeeStringObject *result = NULL;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		ASSERT(sym->s_type < COMPILER_LENOF(symbol_type_names));
		result = symbol_type_names[sym->s_type];
		Dee_Incref(result);
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}

PRIVATE NONNULL((1)) int DCALL
err_symbol_readonly(struct symbol *__restrict sym) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot modify argument symbol %$q",
	                       sym->s_name->k_size,
	                       sym->s_name->k_name);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
symbol_delkind(DeeCompilerSymbolObject *__restrict self) {
	int result = -1;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		if unlikely(SYMBOL_TYPE_IS_IMMUTABLE(sym->s_type)) {
			result = err_symbol_readonly(sym);
		} else {
			/* Set the symbol type to `none' */
			symbol_fini(sym);
			sym->s_type = SYMBOL_TYPE_NONE;
			result      = 0;
		}
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
symbol_setkind(DeeCompilerSymbolObject *__restrict self,
               DeeObject *__restrict value) {
	int result = -1;
	struct symbol *sym;
	uint16_t new_kind;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto done;
	new_kind = get_symbol_kind_from_name(DeeString_STR(value));
	if unlikely(new_kind == (uint16_t)-1)
		goto done;
	switch (new_kind) {

	case SYMBOL_TYPE_EXTERN:
	case SYMBOL_TYPE_MODULE:
	case SYMBOL_TYPE_CATTR:
	case SYMBOL_TYPE_ALIAS:
	case SYMBOL_TYPE_ARG:
		break;

	default: break;
	}

	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		if unlikely(SYMBOL_TYPE_IS_IMMUTABLE(sym->s_type)) {
			result = err_symbol_readonly(sym);
		} else {
			/* Set the symbol type to `none' */
			symbol_fini(sym);
			sym->s_type = new_kind;
			switch (new_kind) {

			case SYMBOL_TYPE_GLOBAL:
				sym->s_global.g_doc = NULL;
				break;
			case SYMBOL_TYPE_GETSET:
				sym->s_getset.gs_get = NULL;
				sym->s_getset.gs_del = NULL;
				sym->s_getset.gs_set = NULL;
				break;
			case SYMBOL_TYPE_AMBIG:
				sym->s_ambig.a_decl2.l_file = NULL;
				sym->s_ambig.a_declc        = 0;
				sym->s_ambig.a_declv        = NULL;
				break;
			case SYMBOL_TYPE_CONST:
				sym->s_const = DeeNone_NewRef();
				break;

			default: break;
			}
			result = 0;
		}
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
symbol_name(DeeCompilerSymbolObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		result = DeeString_NewUtf8(sym->s_name->k_name,
		                           sym->s_name->k_size,
		                           STRING_ERROR_FIGNORE);
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
symbol_print(DeeCompilerSymbolObject *__restrict self,
             Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result = -1;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		result = DeeFormat_Print(printer, arg,
		                         sym->s_name->k_name,
		                         sym->s_name->k_size);
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
symbol_printrepr(DeeCompilerSymbolObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result = -1;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		result = DeeFormat_Printf(printer, arg, "<symbol %$q>",
		                          sym->s_name->k_size,
		                          sym->s_name->k_name);
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}


PRIVATE struct type_getset tpconst symbol_getsets[] = {
	TYPE_GETSET("kind", &symbol_getkind, &symbol_delkind, &symbol_setkind,
	            "->?Dstring\n"
	            "#tValueError{Attempted to set an invalid symbol type}"
	            "#tTypeError{Attempted to set the symbol type to one of "
	            /*            */ "${[\"extern\", \"module\", \"cfield\", \"ifield\", \"alias\", \"arg\"]} "
	            /*            */ "(use the `set*' member functions instead)}"
	            "#tTypeError{Attempted to modify the typing of an $\"arg\" symbol}"
	            "Get, del (set to $\"none\"), or set the typing of @this symbol\n"
	            "The symbol's typing affects the assembly generated when the symbol "
	            /**/ "is used in " DR_CAst " branches\n"
	            "The following symbol types (kinds) exist:\n"
	            "#T{Name|Get|Del|Set|Write|Ref|Life|Description~"
	            /**/ "$\"none\"|none|-|-|-|no|-|Placeholder type, and used as typing of newly created symbols&"
	            /**/ "$\"global\"|global|global|global|-|no|Same as the module|A global symbol, that is exported from the current module (though only if used)&"
	            /**/ "$\"extern\"|extern|extern|extern|-|no|Same as the module|An external symbol, imported from another module&"
	            /**/ "$\"module\"|module|-|-|local|no|-|A reference to the module object of an import&"
	            /**/ "$\"mymod\"|module|-|-|local|no|-|A special sub-class of $\"module\", referring to one's own module (s.a. ?Amodule" DR_Compiler ")&"
	            /**/ "$\"getset\"|(getter)|(delete)|(setter)|-|no (callbacks: yes)|-|"
	            /**/ /**/ "A special symbol class that allows for up to 3 other symbols to be defined, "
	            /**/ /**/ "which are then read from, with the retrieved object then being called&"
	            /**/ "$\"ifield\"|member|member|member|-|no (access-descriptors: yes)|Per-instance|A symbol referring to an instance-member&"
	            /**/ "$\"cfield\"|member|member|member|-|no (class-symbol: yes)|Same as the class-symbol|A symbol referring to an class-member&"
	            /**/ "$\"alias\"|ind|ind|ind|-|-|-|An alias referring to a different symbol&"
	            /**/ "$\"arg\"|arg|-|-|local|yes|Duration of invocation|A symbol referring to an argument passed to a function during invocation&"
	            /**/ "$\"local\"|local|local|local|-|yes|Duration of invocation|A symbol that exists local to each invocation of a function&"
	            /**/ "$\"static\"|static|static|static|-|yes|Duration of function|A symbol that exists statically as part of its function (not shared between different function instantiations)&"
	            /**/ "$\"except\"|except|-|-|local|yes|Until handled|A symbol referring to the last-thrown exception&"
	            /**/ "$\"myfunc\"|myfunc|-|-|local|yes|-|A symbol referring to the current function, within that function. "
	            /**/ /*                              */ "Used to prevent an unneeded reference loop when a function calls itself.&"
	            /**/ "$\"this\"|arg|-|-|-|yes|-|Access to the hidden this-argument&"
	            /**/ "$\"ambig\"|-|-|-|-|no|-|Illegal access caused by ambiguity, including verbose information about ambiguous declaration locations&"
	            /**/ "$\"fwd\"|-|-|-|-|-|-|Access to another symbol that has yet to be declared. (Usually replaced by an $\"alias\" symbol during final linking)&"
	            /**/ "$\"const\"|const|-|-|-|no|forever|Access to a constant expressions that may be duplicated an undefined amount of times. "
	            /**/ /*                             */ "Should only be used for constant expressions that are also immutable, "
	            /**/ /*                             */ "such as :{Tuple}s, :{string}s or :{int}egers, etc.&"
	            /**/ "<ref>|ref|-|-|-|yes|Same as function instance|A symbol that is being accessed as a reference (see #IRef below)"
	            "}\n"
	            "#L-{"
	            /**/ "#IGet: The mechanism used to read from the symbol, and check its binding|"
	            /**/ "#IDel: The mechanism used to delete the symbol|"
	            /**/ "#ISet: The mechanism used to write to the symbol|"
	            /**/ "#IWrite: A conversion that is applied to the symbol during assembly, if the symbol has been written at any point|"
	            /**/ "#IRef: Indicates if the symbol must be accessed by-ref within inner base-scopes. "
	            /**/ /**/ "Symbols accessed by-ref can only be read from, but not be deleted, or written to. "
	            /**/ /**/ "At runtime, this is done by storing a reference to all symbols used by an inner "
	            /**/ /**/ "function, but declared by an outer function, meaning that the act of initializing "
	            /**/ /**/ "such a function will invoke the Get operation of the symbol (with the exception of "
	            /**/ /**/ "$\"getset\", which will reference the individual callbacks instead)|"
	            /**/ "#ILife: How long objects stored in this symbol exist for"
	            "}\n"
	            "#T{Mechanism|Description~"
	            /**/ "$none|Simply evaluates to ?N whenever accessed&"
	            /**/ "$global|Writable access to a symbol stored within ones own module object, and accessible from other modules&"
	            /**/ "$extern|Similar to $global, however the symbol appears as a $global object of another module&"
	            /**/ "$module|Read-only access to ones own, or an imported module object&"
	            /**/ "$foo|Read from another symbol $foo, then invoke the result to emulate symbol operations&"
	            /**/ "$member|A class- or instance-symbol, similar to an attribute, but stripped of being a virtual access&"
	            /**/ "$ind|Seamless forwarding of an access to another symbol&"
	            /**/ "$arg|Read-only access to a argument passed to a function&"
	            /**/ "$local|Writable access to objects stored as part of a function getting executed&"
	            /**/ "$static|Writable access to objects stored alongside code being executed&"
	            /**/ "$except|Read-only access to the last thrown exception&"
	            /**/ "$myfunc|Read-only access to the function currently being executed&"
	            /**/ "$const|Access to a constant expression"
	            "}"),
	TYPE_GETTER("name", &symbol_name,
	            "->?Dstring\n"
	            "Returns the name of @this symbol"),
	TYPE_GETSET_END
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
symbol_getalias(DeeCompilerSymbolObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	DeeArg_Unpack0(done, argc, argv, "getalias");
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		if (sym->s_type != SYMBOL_TYPE_ALIAS) {
			/* We're not an alias, so just re-return the given symbol. */
			result = Dee_AsObject(self);
			Dee_Incref(result);
		} else {
			do {
				sym = sym->s_alias;
			} while (sym->s_type == SYMBOL_TYPE_ALIAS);
			result = DeeCompiler_GetSymbol(sym);
		}
	}
/*done_compiler_end:*/
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
symbol_setalias(DeeCompilerSymbolObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	DeeCompilerSymbolObject *other;
	struct symbol *other_sym;
	DeeArg_Unpack1(done, argc, argv, "setalias", &other);
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	if (DeeObject_AssertTypeExact(other, &DeeCompilerSymbol_Type))
		goto done_compiler_end;
	if unlikely(other->ci_compiler != DeeCompiler_Current) {
		err_invalid_symbol_compiler(other);
		goto done_compiler_end;
	}
	other_sym = DeeCompilerItem_VALUE(other, struct symbol);
	if unlikely(!other_sym)
		goto done_compiler_end;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if unlikely(!sym)
		goto done_compiler_end;
	if unlikely(SYMBOL_TYPE_IS_IMMUTABLE(sym->s_type)) {
		err_symbol_readonly(sym);
	} else {
		/* Check that `self' isn't reachable from `other_sym' */
		struct symbol *iter = other_sym;
		for (;; iter = iter->s_alias) {
			if unlikely(iter == sym) {
				DeeError_Throwf(&DeeError_ReferenceError,
				                "Symbol alias loop with %$q",
				                sym->s_name->k_size,
				                sym->s_name->k_name);
				goto done_compiler_end;
			}
			if (iter->s_type != SYMBOL_TYPE_ALIAS)
				break;
		}
		symbol_fini(sym);
		sym->s_type  = SYMBOL_TYPE_ALIAS;
		sym->s_alias = other_sym;
		symbol_incref(other_sym);
		/* Track new symbol usage within the referenced symbol(s). */
		SYMBOL_ADD_NREAD(other_sym, sym->s_nread);
		SYMBOL_ADD_NWRITE(other_sym, sym->s_nwrite);
		SYMBOL_ADD_NBOUND(other_sym, sym->s_nbound);
		result = Dee_AsObject(self);
		Dee_Incref(result);
	}
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
symbol_setnone(DeeCompilerSymbolObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "setnone");
	if (symbol_delkind(self))
		goto err;
	return_reference_(Dee_AsObject(self));
err:
	return NULL;
}


PRIVATE struct type_method tpconst symbol_methods[] = {
	TYPE_METHOD("getalias", &symbol_getalias,
	            "->?.\n"
	            "Either re-returns @this symbol, or unwinds it to return "
	            /**/ "the effective symbol that is being aliased by it"),
	TYPE_METHOD("setalias", &symbol_setalias,
	            "(other:?.)->?.\n"
	            "#tReferenceError{Either @other is the same symbol as @this, or "
	            /*                 */ "it is another alias eventually leading to @this}"
	            "#tTypeError{Attempted to modify the typing of an $\"arg\" symbol}"
	            "Change @this symbol to be an alias for @other, and re-return @this"),
	TYPE_METHOD("setnone", &symbol_setnone,
	            "->?.\n"
	            "#tTypeError{Attempted to modify the typing of an $\"arg\" symbol}"
	            "Change @this symbol to a none-symbol"),
	TYPE_METHOD_END
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
symbol_bool(DeeCompilerSymbolObject *__restrict self) {
	int result = -1;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym)
		result = sym->s_type != SYMBOL_TYPE_NONE;
	COMPILER_END();
done:
	return result;
}

INTERN DeeTypeObject DeeCompilerSymbol_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Symbol",
	/* .tp_doc      = */ DOC("Inspect and modify attributes, typing, and linkage of a symbol\n"
	                         "\n"

	                         "str->\n"
	                         "Returns the name of the symbol (same as #name)\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns a human-readable representation of the symbol\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if #kind isn't $\"none\""),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerItem_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerItemObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&symbol_name,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&symbol_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&symbol_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&symbol_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ symbol_methods,
	/* .tp_getsets       = */ symbol_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_ISYMBOL_C */
