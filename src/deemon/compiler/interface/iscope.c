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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ISCOPE_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ISCOPE_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>

#include "../../runtime/runtime_error.h"

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
	return DeeCompiler_GetObjItem(result_type, (DeeObject *)scope);
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
	if (DeeObject_Unpack(obj, 3, args))
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

INTERN int DCALL
get_scope_lookupmode(DeeObject *__restrict value,
                     unsigned int *__restrict presult) {
	if (value == Dee_EmptyString) {
		*presult = 0;
		return 0;
	}
	if (DeeString_Check(value)) {
		/* TODO */
	}
	return DeeObject_AsUInt(value, presult);
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_getbase(DeeCompilerScopeObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->ci_compiler);
	result = DeeCompiler_GetScope((DeeScopeObject *)self->ci_value->s_base);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_getprev(DeeCompilerScopeObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->ci_compiler);
	if (!self->ci_value->s_prev) {
		result = Dee_None;
		Dee_Incref(result);
	} else {
		result = DeeCompiler_GetScope(self->ci_value->s_prev);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_get_isclassscope(DeeCompilerScopeObject *__restrict self) {
	return_bool(DeeScope_IsClassScope(self->ci_value));
}


PRIVATE struct type_getset tpconst scope_getsets[] = {
	{ "base", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_getbase, NULL, NULL,
	  DOC("->?ABaseScope?Ert:Compiler\n"
	      "Returns the nearest base-scope that @this scope is apart of") },
	{ "prev", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_getprev, NULL, NULL,
	  DOC("->?X2?AScope?Ert:Compiler?N\n"
	      "Returns a the parent of @this scope, or ?N if @this scope is the root-scope") },
	{ "isclassscope",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_get_isclassscope, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Check if @this scope is a class-scope\n"
	      "Class scopes are somewhat special, in that they prolong the full linkage of "
	      "symbol lookup going beyond their range, up to the point where the scope is "
	      "removed from the scope-stack\n"
	      "This in turn allows for so-called `fwd' (forward; s.a.: #symbol.kind) symbols "
	      "to be used like any other general-purpose symbol, but only be fully linked once "
	      "it is known if the surrounding class defines a symbol of the same name, thus "
	      "allowing member functions that havn't actually been declared, to already be used "
	      "ahead of time") },
	{ NULL }
};

INTERN WUNUSED NONNULL((1, 2)) int DCALL
print_scope_repr(DeeScopeObject *__restrict self,
                 struct unicode_printer *__restrict printer) {
	dssize_t error;
	error = unicode_printer_printf(printer, "<scope at %p>", self);
	return (unlikely(error < 0))
	       ? -1
	       : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_str(DeeCompilerScopeObject *__restrict self) {
	return DeeString_Newf("<scope at %p>", self->ci_value);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
scope_bool(DeeCompilerScopeObject *__restrict self) {
	int result;
	COMPILER_BEGIN(self->ci_compiler);
	result = self->ci_value->s_mapc != 0;
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_iter(DeeCompilerScopeObject *__restrict self) {
	(void)self; /* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_size(DeeCompilerScopeObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->ci_compiler);
	result = DeeInt_NewSize(self->ci_value->s_mapc);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
scope_contains(DeeCompilerScopeObject *self,
               DeeObject *elem) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->ci_compiler);
	if (DeeObject_InstanceOfExact(elem, &DeeCompilerSymbol_Type)) {
		result = DeeBool_For((((DeeCompilerSymbolObject *)elem)->ci_compiler == self->ci_compiler &&
		                      ((DeeCompilerSymbolObject *)elem)->ci_value != NULL &&
		                      ((DeeCompilerSymbolObject *)elem)->ci_value->s_scope == self->ci_value));
	} else if (DeeString_Check(elem)) {
		char *utf8 = DeeString_AsUtf8(elem);
		if unlikely(!utf8) {
			result = NULL;
			goto done;
		}
		result = DeeBool_For(scope_lookup_str(self->ci_value, utf8, WSTR_LENGTH(utf8)) != NULL);
	} else {
		result = Dee_False;
	}
	Dee_Incref(result);
done:
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
scope_getitem(DeeCompilerScopeObject *self, DeeObject *elem) {
	DREF DeeObject *result;
	struct symbol *sym;
	char *utf8;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(elem);
	if unlikely(!utf8)
		goto err;
	COMPILER_BEGIN(self->ci_compiler);
	sym = scope_lookup_str(self->ci_value, utf8, WSTR_LENGTH(utf8));
	if unlikely(!sym) {
		err_item_not_found((DeeObject *)self, elem);
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
	char *utf8;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(elem);
	if unlikely(!utf8)
		goto err;
	COMPILER_BEGIN(self->ci_compiler);
	sym = scope_lookup_str(self->ci_value, utf8, WSTR_LENGTH(utf8));
	if unlikely(!sym) {
		err_item_not_found((DeeObject *)self, elem);
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
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&scope_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&scope_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&scope_delitem,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_member tpconst scope_class_members[] = {
	TYPE_MEMBER_CONST("Symbol", &DeeCompilerSymbol_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_newanon(DeeCompilerScopeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	COMPILER_BEGIN(self->ci_compiler);
	if (DeeArg_Unpack(argc, argv, ":newanon"))
		goto done;
	sym = new_unnamed_symbol_in_scope(self->ci_value);
	if unlikely(!sym)
		goto done;
	sym->s_type = SYMBOL_TYPE_NONE;
	result      = DeeCompiler_GetSymbol(sym);
done:
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scope_newlocal(DeeCompilerScopeObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = NULL;
	struct symbol *sym;
	struct TPPKeyword *kwd;
	DeeObject *name, *loc = NULL;
	bool requirenew = true;
	char *name_utf8;
	PRIVATE struct keyword kwlist[] = { K(name), K(requirenew), K(loc), KEND };
	COMPILER_BEGIN(self->ci_compiler);
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|bo:newlocal", &name, &requirenew, &loc))
		goto done;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto done;
	name_utf8 = DeeString_AsUtf8(name);
	if unlikely(!name_utf8)
		goto done;
	kwd = TPPLexer_LookupKeyword(name_utf8, WSTR_LENGTH(name_utf8), 1);
	if unlikely(!kwd)
		goto done;
	sym = get_local_symbol_in_scope(self->ci_value, kwd);
	if unlikely(sym) {
		if (requirenew) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Local symbol %r has already been defined");
			goto done;
		}
	} else {
		struct ast_loc symloc;
		if unlikely(get_astloc_from_obj(loc, &symloc))
			goto done;
		sym = new_local_symbol_in_scope(self->ci_value, kwd, &symloc);
		if unlikely(!sym)
			goto done;
		sym->s_type = SYMBOL_TYPE_NONE;
	}
	result = DeeCompiler_GetSymbol(sym);
done:
	COMPILER_END();
	return result;
}

PRIVATE struct type_method tpconst scope_methods[] = {
	{ "newanon", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&scope_newanon,
	  DOC("->?ASymbol?Ert:Compiler\n"
	      "Construct a new anonymous symbol, and add it as part of @this scope\n"
	      "The symbol isn't given a name (when queried it will have an empty name), and "
	      /**/ "will otherwise behave just like a symbol that has been deleted (s.a. ?#{op:delitem})\n"
	      "The symbol can however be used to hold values just like any other symbol, "
	      /**/ "meaning that this is the type of symbol that should be used to hold hidden "
	      /**/ "values, as used by $with-statements\n"
	      "New symbols are created with $\"none\"-typing (s.a. ?Akind?#symbol)") },
	{ "newlocal", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&scope_newlocal,
	  DOC("(name:?Dstring,requirenew=!t,loc?:?T3?AFile?ALexer?Ert:Compiler?Dint?Dint)->?ASymbol?Ert:Compiler\n"
	      "@param loc The declaration position of the symbol, omitted to use the current "
	      /*      */ "token position, or ?N when not available\n"
	      "@throw ValueError @requirenew is ?t, and another symbol @name already exists\n"
	      "Lookup, or create a new local symbol named @name\n"
	      "If another symbol with that same name already exists, either return that "
	      /**/ "symbol when @requirenew is ?f, or throw a :ValueError otherwise\n"
	      "New symbols are created with $\"none\"-typing (s.a. ?Akind?#symbol)"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
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
	                         "contains(sym:?ASymbol?Ert:Compiler)->?Dbool\n"
	                         "Returns ?t if @this scope contains a given symbol @sym, "
	                         /**/ "or some symbol with a name matching the given @name\n"
	                         "\n"
	                         "[](string name)->symbol\n"
	                         "@throw ValueError No symbol matching @name is contained within @this scope\n"
	                         "Returns the symbol associated with @name\n"
	                         "\n"
	                         "del[](string name)->\n"
	                         "@throw ValueError No symbol matching @name is contained within @this scope\n"
	                         "Delete the symbol associated with @name, adding it to the set of deleted symbols"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerItemObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerObjItem_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scope_str,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&scope_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&DeeCompilerObjItem_Visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &scope_seq,
	/* .tp_iter_next     = */ NULL,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerItemObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerItemObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_ISCOPE_C */
