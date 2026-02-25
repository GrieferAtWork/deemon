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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ISCOPE_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ISCOPE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack0, DeeArg_UnpackStructKw */
#include <deemon/bool.h>               /* return_bool */
#include <deemon/compiler/ast.h>       /* ast, loc_here */
#include <deemon/compiler/compiler.h>  /* COMPILER_BEGIN, COMPILER_END, DeeCompiler* */
#include <deemon/compiler/interface.h> /* DR_*, DeeCompiler*, err_compiler_item_deleted, err_invalid_file_compiler */
#include <deemon/compiler/symbol.h>    /* DeeBaseScope_Type, DeeRootScope_Type, DeeScopeObject, DeeScope_IsClassScope, DeeScope_Type, SYMBOL_TYPE_NONE, ast_loc, del_local_symbol, get_local_symbol_in_scope, new_local_symbol_in_scope, new_unnamed_symbol_in_scope, scope_lookup_str, scope_object, symbol */
#include <deemon/error-rt.h>           /* DeeRT_ErrItemNotFound */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Printf */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/map.h>                /* DeeMapping_Type */
#include <deemon/none.h>               /* DeeNone_Check, Dee_None */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_Incref, Dee_TYPE, Dee_formatprinter_t, Dee_ssize_t, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeSeq_Unpack */
#include <deemon/string.h>             /* DeeString*, Dee_EmptyString, WSTR_LENGTH */
#include <deemon/type.h>               /* DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_visit_t, TF_NONE, TP_FNORMAL, TYPE_*, type_* */

#include "../../runtime/kwlist.h"
#include "../../runtime/strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */

DECL_BEGIN

typedef DeeCompilerScopeObject Scope;

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCompiler_GetScope(struct scope_object *__restrict scope) {
	DeeTypeObject *result_type;
	if (Dee_TYPE(scope) == &DeeScope_Type) {
		result_type = &DeeCompilerScope_Type;
	} else if (Dee_TYPE(scope) == &DeeBaseScope_Type) {
		result_type = &DeeCompilerBaseScope_Type;
	} else {
		/* TODO: Support for DeeClassScope_Type! */
		ASSERT(Dee_TYPE(scope) == &DeeRootScope_Type);
		result_type = &DeeCompilerRootScope_Type;
	}
	return DeeCompiler_GetObjItem(result_type, Dee_AsObject(scope));
}



INTERN WUNUSED NONNULL((2)) int DCALL
set_astloc_from_obj(DeeObject *obj,
                    struct ast *__restrict result) {
	if unlikely(get_astloc_from_obj(obj, &result->a_ddi))
		goto err;
	if likely(result->a_ddi.l_file)
		TPPFile_Incref(result->a_ddi.l_file);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((2)) int DCALL
get_astloc_from_obj(DeeObject *obj,
                    struct ast_loc *__restrict result) {
	DREF DeeObject *args[3];
	if (!obj) {
		loc_here(result);
		goto done;
	}
	if (DeeNone_Check(obj)) {
		result->l_file = NULL;
		goto done;
	}
	if (DeeSeq_Unpack(obj, 3, args))
		goto err;
	if (DeeNone_Check(args[0])) {
		result->l_file = NULL;
	} else {
		if (DeeObject_AsInt(args[2], &result->l_col))
			goto err_args_2;
		Dee_Decref(args[2]);
		if (DeeObject_AsInt(args[1], &result->l_line))
			goto err_args_1;
		Dee_Decref(args[1]);
		if (DeeObject_AssertTypeExact(args[0], &DeeCompilerFile_Type))
			goto err_args_0;
		if (((DeeCompilerItemObject *)args[0])->ci_compiler != DeeCompiler_Current) {
			err_invalid_file_compiler((DeeCompilerItemObject *)args[0]);
			goto err_args_0;
		}
		if unlikely(!((DeeCompilerItemObject *)args[0])->ci_value) {
			err_compiler_item_deleted((DeeCompilerItemObject *)args[0]);
			goto err_args_0;
		}
		result->l_file = (struct TPPFile *)((DeeCompilerItemObject *)args[0])->ci_value;
		TPPFile_Incref(result->l_file);
	}
	Dee_Decref(args[0]);
done:
	return 0;
err_args_2:
	Dee_Decref(args[2]);
err_args_1:
	Dee_Decref(args[1]);
err_args_0:
	Dee_Decref(args[0]);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
get_scope_lookupmode(DeeObject *__restrict value,
                     unsigned int *__restrict p_result) {
	if (value == Dee_EmptyString) {
		*p_result = 0;
		return 0;
	}
	if (DeeString_Check(value)) {
		/* TODO */
	}
	return DeeObject_AsUInt(value, p_result);
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_getbase(DeeCompilerScopeObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	result = DeeCompiler_GetScope((DeeScopeObject *)self->ci_value->s_base);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_getprev(DeeCompilerScopeObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if (!self->ci_value->s_prev) {
		result = Dee_None;
		Dee_Incref(result);
	} else {
		result = DeeCompiler_GetScope(self->ci_value->s_prev);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_get_isclassscope(DeeCompilerScopeObject *__restrict self) {
	return_bool(DeeScope_IsClassScope(self->ci_value));
}


PRIVATE struct type_getset tpconst scope_getsets[] = {
	TYPE_GETTER("base", &scope_getbase,
	            "->" DR_CBaseScope "\n"
	            "Returns the nearest base-scope that @this scope is apart of"),
	TYPE_GETTER(STR_prev, &scope_getprev,
	            "->?X2" DR_CScope "?N\n"
	            "Returns a the parent of @this scope, or ?N if @this scope is the root-scope"),
	TYPE_GETTER("isclassscope", &scope_get_isclassscope,
	            "->?Dbool\n"
	            "Check if @this scope is a class-scope\n"
	            "Class scopes are somewhat special, in that they prolong the full linkage of "
	            /**/ "symbol lookup going beyond their range, up to the point where the scope is "
	            /**/ "removed from the scope-stack\n"
	            "This in turn allows for so-called #Cfwd (forward; s.a.: ?Akind?#Symbol) symbols "
	            /**/ "to be used like any other general-purpose symbol, but only be fully linked once "
	            /**/ "it is known if the surrounding class defines a symbol of the same name, thus "
	            /**/ "allowing member functions that havn't actually been declared, to already be used "
	            /**/ "ahead of time"),
	TYPE_GETSET_END
};

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* Re-used in "./iast.c" */
print_scope_repr(DeeScopeObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<scope at %p>", self);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
scope_print(DeeCompilerScopeObject *__restrict self,
            Dee_formatprinter_t printer, void *arg) {
	return print_scope_repr(self->ci_value, printer, arg);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
scope_bool(DeeCompilerScopeObject *__restrict self) {
	int result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	result = self->ci_value->s_mapc != 0;
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_iter(DeeCompilerScopeObject *__restrict self) {
	(void)self; /* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_size(DeeCompilerScopeObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	result = DeeInt_NewSize(self->ci_value->s_mapc);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
scope_contains(DeeCompilerScopeObject *self,
               DeeObject *elem) {
	bool result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if (DeeObject_InstanceOfExact(elem, &DeeCompilerSymbol_Type)) {
		DeeCompilerSymbolObject *e = (DeeCompilerSymbolObject *)elem;
		result = (e->ci_compiler == self->ci_compiler &&
		          e->ci_value != NULL &&
		          e->ci_value->s_scope == self->ci_value);
	} else if (DeeString_Check(elem)) {
		char const *utf8 = DeeString_AsUtf8(elem);
		if unlikely(!utf8)
			goto err_end;
		result = scope_lookup_str(self->ci_value, utf8, WSTR_LENGTH(utf8)) != NULL;
	} else {
		result = false;
	}
	COMPILER_END();
	return_bool(result);
err_end:
	COMPILER_END();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
scope_getitem(DeeCompilerScopeObject *self, DeeObject *elem) {
	DREF DeeObject *result;
	struct symbol *sym;
	char const *utf8;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(elem);
	if unlikely(!utf8)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	sym = scope_lookup_str(self->ci_value, utf8, WSTR_LENGTH(utf8));
	if unlikely(!sym) {
		DeeRT_ErrItemNotFound(self, elem);
		result = NULL;
	} else {
		result = DeeCompiler_GetSymbol(sym);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
scope_delitem(DeeCompilerScopeObject *__restrict self,
              DeeObject *__restrict elem) {
	int result;
	struct symbol *sym;
	char const *utf8;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(elem);
	if unlikely(!utf8)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	sym = scope_lookup_str(self->ci_value, utf8, WSTR_LENGTH(utf8));
	if unlikely(!sym) {
		DeeRT_ErrItemNotFound(self, elem);
		result = -1;
	} else {
		/* Delete the symbol. */
		del_local_symbol(sym);
		result = 0;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE struct type_seq scope_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_iter,
	/* .tp_sizeob   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_size,
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&scope_contains,
	/* .tp_getitem  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&scope_getitem,
	/* .tp_delitem  = */ (int (DCALL *)(DeeObject *, DeeObject *))&scope_delitem,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};

PRIVATE struct type_member tpconst scope_class_members[] = {
	TYPE_MEMBER_CONST("Symbol", &DeeCompilerSymbol_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_newanon(DeeCompilerScopeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	DeeArg_Unpack0(done_compiler_end, argc, argv, "newanon");
	sym = new_unnamed_symbol_in_scope(self->ci_value);
	if unlikely(!sym)
		goto done_compiler_end;
	sym->s_type = SYMBOL_TYPE_NONE;
	result      = DeeCompiler_GetSymbol(sym);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_newlocal(DeeCompilerScopeObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	struct TPPKeyword *kwd;
	char const *name_utf8;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("newlocal", params: """
	DeeStringObject *name:?Dstring;
	bool requirenew = true;
	DeeObject *loc:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint = NULL;
""", docStringPrefix: "file", err: "done");]]]*/
#define file_newlocal_params "name:?Dstring,requirenew=!t,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint"
	struct {
		DeeStringObject *name;
		bool requirenew;
		DeeObject *loc;
	} args;
	args.requirenew = true;
	args.loc = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_requirenew_loc, "o|bo:newlocal", &args))
		goto done;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto done;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	name_utf8 = DeeString_AsUtf8(Dee_AsObject(args.name));
	if unlikely(!name_utf8)
		goto done_compiler_end;
	kwd = TPPLexer_LookupKeyword(name_utf8, WSTR_LENGTH(name_utf8), 1);
	if unlikely(!kwd)
		goto done_compiler_end;
	sym = get_local_symbol_in_scope(self->ci_value, kwd);
	if unlikely(sym) {
		if (args.requirenew) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Local symbol %r has already been defined");
			goto done_compiler_end;
		}
	} else {
		struct ast_loc symloc;
		if unlikely(get_astloc_from_obj(args.loc, &symloc))
			goto done_compiler_end;
		sym = new_local_symbol_in_scope(self->ci_value, kwd, &symloc);
		if unlikely(!sym)
			goto done_compiler_end;
		sym->s_type = SYMBOL_TYPE_NONE;
	}
	result = DeeCompiler_GetSymbol(sym);
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE struct type_method tpconst scope_methods[] = {
	TYPE_METHOD("newanon", &scope_newanon,
	            "->" DR_CSymbol "\n"
	            "Construct a new anonymous symbol, and add it as part of @this scope\n"
	            "The symbol isn't given a name (when queried it will have an empty name), and "
	            /**/ "will otherwise behave just like a symbol that has been deleted (s.a. ?#{op:delitem})\n"
	            "The symbol can however be used to hold values just like any other symbol, "
	            /**/ "meaning that this is the type of symbol that should be used to hold hidden "
	            /**/ "values, as used by $with-statements\n"
	            "New symbols are created with $\"none\"-typing (s.a. ?Akind?#Symbol)"),
	TYPE_KWMETHOD("newlocal", &scope_newlocal,
	              "(" file_newlocal_params ")->" DR_CSymbol "\n"
	              "#ploc{The declaration position of the symbol, omitted to use the current "
	              /* */ "token position, or ?N when not available}"
	              "#tValueError{@requirenew is ?t, and another symbol @name already exists}"
	              "Lookup, or create a new local symbol named @name\n"
	              "If another symbol with that same name already exists, either return that "
	              /**/ "symbol when @requirenew is ?f, or throw a :ValueError otherwise\n"
	              "New symbols are created with $\"none\"-typing (s.a. ?Akind?#Symbol)"),
	TYPE_METHOD_END
};


INTERN DeeTypeObject DeeCompilerScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Scope",
	/* .tp_doc      = */ DOC("Access the symbols declared within a scope during ast parsing\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating all symbols within @this scope\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of symbols found within @this scope\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this scope is non-empty\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns a unique, human-readable representation of @this scope\n"
	                         "\n"

	                         "contains(name:?Dstring)->?Dbool\n"
	                         "contains(sym:" DR_CSymbol ")->?Dbool\n"
	                         "Returns ?t if @this scope contains a given symbol @sym, "
	                         /**/ "or some symbol with a name matching the given @name\n"
	                         "\n"

	                         "[](name:?Dstring)->" DR_CSymbol "\n"
	                         "#tValueError{No symbol matching @name is contained within @this scope}"
	                         "Returns the symbol associated with @name\n"
	                         "\n"

	                         "del[](name:?Dstring)->\n"
	                         "#tValueError{No symbol matching @name is contained within @this scope}"
	                         "Delete the symbol associated with @name, adding it to the set of deleted symbols"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerItemObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerObjItem_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&scope_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&scope_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&scope_print
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&DeeCompilerObjItem_Visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &scope_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ scope_methods,
	/* .tp_getsets       = */ scope_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ scope_class_members
};

INTERN DeeTypeObject DeeCompilerBaseScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BaseScope",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerScope_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerItemObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeCompilerRootScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RootScope",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerBaseScope_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerItemObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_ISCOPE_C */
