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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ISYMBOL_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ISYMBOL_C 1
#define _KOS_SOURCE 1

#include <deemon/compiler/compiler.h>

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/util/cache.h>

#include "../../runtime/strings.h"

DECL_BEGIN

DECLARE_OBJECT_CACHE(compiler_item,DeeCompilerItemObject)
typedef DeeCompilerSymbolObject Symbol;

INTERN DeeObject *symbol_type_names[] = {
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

PRIVATE uint16_t DCALL
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

PRIVATE DREF DeeObject *DCALL
symbol_getkind(DeeCompilerSymbolObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		ASSERT(sym->s_type < COMPILER_LENOF(symbol_type_names));
		result = symbol_type_names[sym->s_type];
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE int DCALL
err_symbol_readonly(struct symbol *__restrict sym) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot modify argument symbol %$q",
	                       sym->s_name->k_size,
	                       sym->s_name->k_name);
}

PRIVATE int DCALL
symbol_delkind(DeeCompilerSymbolObject *__restrict self) {
	int result = -1;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
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
	COMPILER_END();
	return result;
}

PRIVATE int DCALL
symbol_setkind(DeeCompilerSymbolObject *__restrict self,
               DeeObject *__restrict value) {
	int result = -1;
	struct symbol *sym;
	uint16_t new_kind;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto done2;
	new_kind = get_symbol_kind_from_name(DeeString_STR(value));
	if unlikely(new_kind == (uint16_t)-1)
		goto done2;
	switch (new_kind) {

	case SYMBOL_TYPE_EXTERN:
	case SYMBOL_TYPE_MODULE:
	case SYMBOL_TYPE_CATTR:
	case SYMBOL_TYPE_ALIAS:
	case SYMBOL_TYPE_ARG:
		break;

	default: break;
	}

	COMPILER_BEGIN(self->ci_compiler);
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
				sym->s_const = Dee_None;
				Dee_Incref(Dee_None);
				break;

			default: break;
			}
			result = 0;
		}
	}
	COMPILER_END();
done2:
	return result;
}

PRIVATE DREF DeeObject *DCALL
symbol_name(DeeCompilerSymbolObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		result = DeeString_NewUtf8(sym->s_name->k_name,
		                           sym->s_name->k_size,
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
}


PRIVATE struct type_getset symbol_getsets[] = {
	{ "kind",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & symbol_getkind,
	  (int(DCALL *)(DeeObject *__restrict)) & symbol_delkind,
	  (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict)) & symbol_setkind,
	  DOC("->?Dstring\n"
	      "@throw ValueError Attempted to set an invalid symbol type\n"
	      "@throw TypeError Attempted to set the symbol type to one of "
	      "${[\"extern\",\"module\",\"cfield\",\"ifield\",\"alias\",\"arg\"]} "
	      "(use the `set*' member functions instead)\n"
	      "@throw TypeError Attempted to modify the typing of an $\"arg\" symbol\n"
	      "get, del (set to $\"none\"), or set the typing of @this symbol\n"
	      "The symbol's typing affects the assembly generated when the symbol "
	      "is used in :Compiler.ast branches\n"
	      "The following symbol types (kinds) exist:\n"
	      "%{table Name|Get|Del|Set|Write|Ref|Life|Description\n"
	      "$\"none\"|none|-|-|-|no|-|Placeholder type, and used as typing of newly created symbols\n"
	      "$\"global\"|global|global|global|-|no|Same as the module|A global symbol, that is exported from the current module (though only if used)\n"
	      "$\"extern\"|extern|extern|extern|-|no|Same as the module|An external symbol, imported from another module\n"
	      "$\"module\"|module|-|-|local|no|-|A reference to the module object of an import\n"
	      "$\"mymod\"|module|-|-|local|no|-|A special sub-class of $\"module\", referring to one's own module (s.a. :Compiler.module)\n"
	      "$\"getset\"|(getter)|(delete)|(setter)|-|no (callbacks: yes)|-|"
	      "A special symbol class that allows for up to 3 other symbols to be defined, "
	      "which are then read from, with the retrieved object then being called\n"
	      "$\"ifield\"|member|member|member|-|no (access-descriptors: yes)|Per-instance|A symbol referring to an instance-member\n"
	      "$\"cfield\"|member|member|member|-|no (class-symbol: yes)|Same as the class-symbol|A symbol referring to an class-member\n"
	      "$\"alias\"|ind|ind|ind|-|-|-|An alias referring to a different symbol\n"
	      "$\"arg\"|arg|-|-|local|yes|Duration of invocation|A symbol referring to an argument passed to a function during invocation\n"
	      "$\"local\"|local|local|local|-|yes|Duration of invocation|A symbol that exists local to each invocation of a function\n"
	      "$\"static\"|static|static|static|-|yes|Duration of code|A symbol that exists statically as part of its code (shared between different function instantiations)\n"
	      "$\"except\"|except|-|-|local|yes|Until handled|A symbol referring to the last-thrown exception\n"
	      "$\"myfunc\"|myfunc|-|-|local|yes|-|A symbol referring to the current function, within that function. "
	      "Used to prevent an unneeded reference loop when a function calls itself.\n"
	      "$\"this\"|arg|-|-|-|yes|-|Access to the hidden this-argument\n"
	      "$\"ambig\"|-|-|-|-|no|-|Illegal access caused by ambiguity, including verbose information about ambiguous declaration locations\n"
	      "$\"fwd\"|-|-|-|-|-|-|Access to another symbol that has yet to be declared. (Usually replaced by an $\"alias\" symbol during final linking)\n"
	      "$\"const\"|const|-|-|-|no|forever|Access to a constant expressions that may be duplicated an undefined amount of times. "
	      "Should only be used for constant expressions that are also immutable, "
	      "such as :{tuple}s, :{string}s or :{int}egers, etc.\n"
	      "<ref>|ref|-|-|-|yes|Same as function instance|A symbol that is being accessed as a reference (see `Referenced' below)}\n"
	      "Get: The mechanism used to read from the symbol, and check its binding\n"
	      "Del: The mechanism used to delete the symbol\n"
	      "Set: The mechanism used to write to the symbol\n"
	      "Write: A conversion that is applied to the symbol during assembly, if the symbol has been written at any point\n"
	      "Ref: Indicates if the symbol must be accessed by-ref within inner base-scopes. "
	      "Symbols accessed by-ref can only be read from, but not be deleted, or written to. "
	      "At runtime, this is done by storing a reference to all symbols used by an inner "
	      "function, but declared by an outer function, meaning that the act of initializing "
	      "such a function will invoke the Get operation of the symbol (with the exception of "
	      "$\"getset\", which will reference the individual callbacks instead)\n"
	      "Life: How long objects stored in this symbol exist for\n"
	      "%{table Mechanism|Description\n"
	      "$none|Simply evaluates to :none whenever accessed\n"
	      "$global|Writable access to a symbol stored within ones own module object, and accessible from other modules\n"
	      "$extern|Similar to `global', however the symbol appears as a `global' object of another module\n"
	      "$module|Read-only access to ones own, or an imported module object\n"
	      "$(*)|Read from another symbol `*', then invoke the result to emulate symbol operations\n"
	      "$member|A class- or instance-symbol, similar to an attribute, but stripped of being a virtual access\n"
	      "$ind|Seamless forwarding of an access to another symbol\n"
	      "$arg|Read-only access to a argument passed to a function\n"
	      "$local|Writable access to objects stored as part of a function getting executed\n"
	      "$static|Writable access to objects stored alongside code being executed\n"
	      "$except|Read-only access to the last thrown exception\n"
	      "$myfunc|Read-only access to the function currently being executed\n"
	      "$const|Access to a constant expression}") },
	{ "name",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & symbol_name, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the name of @this symbol") },
	{ NULL }
};



PRIVATE DREF DeeObject *DCALL
symbol_getalias(DeeCompilerSymbolObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
	if (DeeArg_Unpack(argc, argv, ":getalias"))
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		if (sym->s_type != SYMBOL_TYPE_ALIAS) {
			/* We're not an alias, so just re-return the given symbol. */
			result = (DREF DeeObject *)self;
			Dee_Incref(result);
		} else {
			do {
				sym = sym->s_alias;
			} while (sym->s_type == SYMBOL_TYPE_ALIAS);
			result = DeeCompiler_GetSymbol(sym);
		}
	}
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
symbol_setalias(DeeCompilerSymbolObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	DeeCompilerSymbolObject *other;
	struct symbol *other_sym;
	COMPILER_BEGIN(self->ci_compiler);
	if (DeeArg_Unpack(argc, argv, "o:setalias", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeCompilerSymbol_Type))
		goto done;
	if unlikely(other->ci_compiler != DeeCompiler_Current) {
		err_invalid_symbol_compiler(other);
		goto done;
	}
	other_sym = DeeCompilerItem_VALUE(other, struct symbol);
	if unlikely(!other_sym)
		goto done;
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if unlikely(!sym)
		goto done;
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
				goto done;
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
		result = (DREF DeeObject *)self;
		Dee_Incref(result);
	}
done:
	COMPILER_END();
	return result;
}

PRIVATE DREF DeeObject *DCALL
symbol_setnone(DeeCompilerSymbolObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":setnone") ||
	    symbol_delkind(self))
		return NULL;
	return_reference_((DeeObject *)self);
}


PRIVATE struct type_method symbol_methods[] = {
	{ "getalias", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & symbol_getalias,
	  DOC("->?.\n"
	      "Either re-returns @this symbol, or unwinds it to return "
	      "the effective symbol that is being aliased by it") },
	{ "setalias", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & symbol_setalias,
	  DOC("(other:?.)->?.\n"
	      "@throw ReferenceError Either @other is the same symbol as @this, or "
	      "it is another alias eventually leading to @this\n"
	      "@throw TypeError Attempted to modify the typing of an $\"arg\" symbol\n"
	      "Change @this symbol to be an alias for @other, and re-return @this") },
	{ "setnone", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & symbol_setnone,
	  DOC("->?.\n"
	      "@throw TypeError Attempted to modify the typing of an $\"arg\" symbol\n"
	      "Change @this symbol to a none-symbol") },
	{ NULL }
};



PRIVATE DREF DeeObject *DCALL
symbol_repr(DeeCompilerSymbolObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym) {
		result = DeeString_Newf("<symbol %$q>",
		                        sym->s_name->k_size,
		                        sym->s_name->k_name);
	}
	COMPILER_END();
	return result;
}

PRIVATE int DCALL
symbol_bool(DeeCompilerSymbolObject *__restrict self) {
	int result = -1;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
	sym = DeeCompilerItem_VALUE(self, struct symbol);
	if likely(sym)
		result = sym->s_type != SYMBOL_TYPE_NONE;
	COMPILER_END();
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
	                         "Returns :true if #kind isn't $\"none\"\n"
	                         "\n"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerItem_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_ALLOCATOR(&compiler_item_tp_alloc, &compiler_item_tp_free)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&symbol_name,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&symbol_repr,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&symbol_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
