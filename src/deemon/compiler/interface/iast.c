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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_IAST_C
#define GUARD_DEEMON_COMPILER_INTERFACE_IAST_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <deemon/util/atomic.h>

#include "../../runtime/builtin.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

typedef DeeCompilerAstObject Ast;

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_invalid_ast_basescope)(DeeCompilerAstObject *__restrict obj,
                                  struct base_scope_object *__restrict base_scope) {
	(void)obj;
	(void)base_scope;
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "base-scope of ast differs from the effective base-scope");
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_ast_compiler)(DeeCompilerAstObject *__restrict obj) {
	(void)obj;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Ast is associated with a different compiler");
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_scope_compiler)(DeeCompilerScopeObject *__restrict obj) {
	(void)obj;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Scope is associated with a different compiler");
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_symbol_compiler)(DeeCompilerSymbolObject *__restrict obj) {
	(void)obj;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Symbol is associated with a different compiler");
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_symbol_not_reachable)(struct scope_object *__restrict scope,
                                 struct symbol *__restrict sym) {
	(void)scope;
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Symbol %$q is not reachable from the specified scope",
	                       sym->s_name->k_size, sym->s_name->k_name);
}

INTERN WUNUSED NONNULL((1, 2)) bool
(DCALL scope_reaches_symbol)(DeeScopeObject *__restrict scope,
                             struct symbol *__restrict sym) {
	DeeScopeObject *dst = sym->s_scope;
	for (; scope; scope = scope->s_prev) {
		if (scope == dst)
			return true;
	}
	return false;
}

INTERN ATTR_COLD int (DCALL err_different_base_scope)(void) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Cannot assign a new scope that isn't apart "
	                       "of the same base-scope as the old one");
}

INTERN ATTR_COLD int (DCALL err_different_root_scope)(void) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Cannot assign a new scope that isn't apart "
	                       "of the same root-scope as the old one");
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getscope(Ast *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	result = DeeCompiler_GetScope(self->ci_value->a_scope);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setscope(Ast *__restrict self,
             DeeCompilerScopeObject *__restrict value) {
	struct ast *branch = self->ci_value;
	if (DeeObject_AssertType(value, &DeeCompilerScope_Type))
		goto err;
	if (value->ci_compiler != self->ci_compiler)
		return err_invalid_scope_compiler(value);
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if unlikely(value->ci_value->s_base != branch->a_scope->s_base) {
		err_different_base_scope();
		goto err_compiler;
	}
	/* Make sure that referenced symbols is
	 * still reachable from the new scope. */
	switch (branch->a_type) {

	case AST_SYM:
	case AST_UNBIND:
	case AST_BOUND:
		if (scope_reaches_symbol(value->ci_value, branch->a_sym))
			break;
err_unreachable_symbols:
		DeeError_Throwf(&DeeError_ReferenceError,
		                "Cannot assign new scope to branch containing "
		                "symbols that would no longer be reachable");
		goto err_compiler;

	case AST_CLASS:
		if (branch->a_class.c_classsym &&
		    !scope_reaches_symbol(value->ci_value, branch->a_class.c_classsym))
			goto err_unreachable_symbols;
		if (branch->a_class.c_supersym &&
		    !scope_reaches_symbol(value->ci_value, branch->a_class.c_supersym))
			goto err_unreachable_symbols;
		break;

	default: break;
	}
	/* Assign the new scope. */
	Dee_Incref(value->ci_value);
	Dee_Decref(branch->a_scope);
	branch->a_scope = value->ci_value;
	COMPILER_END();
	return 0;
err_compiler:
	COMPILER_END();
err:
	return -1;
}


PRIVATE DeeStringObject *tpconst ast_names[] = {
	/* [AST_CONSTEXPR]     = */ &str_constexpr,
	/* [AST_SYM]           = */ &str_sym,
	/* [AST_UNBIND]        = */ &str_unbind,
	/* [AST_BOUND]         = */ &str_bound,
	/* [AST_MULTIPLE]      = */ &str_multiple,
	/* [AST_RETURN]        = */ &str_return,
	/* [AST_YIELD]         = */ &str_yield,
	/* [AST_THROW]         = */ &str_throw,
	/* [AST_TRY]           = */ &str_try,
	/* [AST_LOOP]          = */ &str_loop,
	/* [AST_LOOPCTL]       = */ &str_loopctl,
	/* [AST_CONDITIONAL]   = */ &str_conditional,
	/* [AST_BOOL]          = */ &str_bool,
	/* [AST_EXPAND]        = */ &str_expand,
	/* [AST_FUNCTION]      = */ &str_function,
	/* [AST_OPERATOR_FUNC] = */ &str_operatorfunc,
	/* [AST_OPERATOR]      = */ &str_operator,
	/* [AST_ACTION]        = */ &str_action,
	/* [AST_CLASS]         = */ &str_class,
	/* [AST_LABEL]         = */ &str_label,
	/* [AST_GOTO]          = */ &str_goto,
	/* [AST_SWITCH]        = */ &str_switch,
	/* [AST_ASSEMBLY]      = */ &str_assembly,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_gettypeid(Ast *__restrict self) {
	uint16_t result;
#ifdef CONFIG_NO_THREADS
	result = self->ci_value->a_type;
#else /* CONFIG_NO_THREADS */
	do {
		result = atomic_read(&self->ci_value->a_type);
	} while unlikely(result >= COMPILER_LENOF(ast_names));
#endif /* !CONFIG_NO_THREADS */
	return DeeInt_NewU16(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getkind(Ast *__restrict self) {
	uint16_t result;
#ifdef CONFIG_NO_THREADS
	result = self->ci_value->a_type;
#else /* CONFIG_NO_THREADS */
	do {
		result = atomic_read(&self->ci_value->a_type);
	} while unlikely(result >= COMPILER_LENOF(ast_names));
#endif /* !CONFIG_NO_THREADS */
	return_reference(ast_names[result]);
}

PRIVATE ATTR_COLD int DCALL
err_invalid_ast_type(Ast *__restrict self,
                     uint16_t expected_type) {
	ASSERT(expected_type < COMPILER_LENOF(ast_names));
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected a %s ast, but got %K instead",
	                       ast_names[expected_type],
	                       ast_getkind(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconstexpr(Ast *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if (self->ci_value->a_type != AST_CONSTEXPR) {
		err_invalid_ast_type(self, AST_CONSTEXPR);
		result = NULL;
	} else {
		result = self->ci_value->a_constexpr;
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconstexpr(Ast *__restrict self,
                 DeeObject *__restrict value) {
	int result = 0;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if (self->ci_value->a_type != AST_CONSTEXPR) {
		result = err_invalid_ast_type(self, AST_CONSTEXPR);
	} else {
		DREF DeeObject *old_value;
		Dee_Incref(value);
		old_value                   = self->ci_value->a_constexpr;
		self->ci_value->a_constexpr = value;
		Dee_Decref(old_value);
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getsym(Ast *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if (self->ci_value->a_type != AST_SYM &&
	    self->ci_value->a_type != AST_UNBIND &&
	    self->ci_value->a_type != AST_BOUND) {
		err_invalid_ast_type(self, AST_SYM);
		result = NULL;
	} else {
		result = DeeCompiler_GetSymbol(self->ci_value->a_sym);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setsym(Ast *__restrict self,
           DeeCompilerSymbolObject *__restrict value) {
	int result = 0;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_symbol_compiler(value);
	} else {
		struct symbol *sym;
		sym = DeeCompilerItem_VALUE(value, struct symbol);
		if unlikely(!sym) {
			result = -1;
		} else {
			struct ast *me = self->ci_value;
			switch (me->a_type) {
			case AST_SYM:
				if (me->a_flag) {
				case AST_UNBIND:
					SYMBOL_DEC_NWRITE(me->a_sym);
					SYMBOL_INC_NWRITE(sym);
				} else {
					SYMBOL_DEC_NREAD(me->a_sym);
					SYMBOL_INC_NREAD(sym);
				}
				break;
			case AST_BOUND:
				SYMBOL_DEC_NBOUND(me->a_sym);
				SYMBOL_INC_NBOUND(sym);
				break;
			default:
				err_invalid_ast_type(self, AST_SYM);
				result = -1;
				goto done;
			}
			me->a_sym = sym;
		}
	}
done:
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
ast_getmultiple(Ast *__restrict self) {
	DREF DeeTupleObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	me = self->ci_value;
	if unlikely(me->a_type != AST_MULTIPLE) {
		err_invalid_ast_type(self, AST_MULTIPLE);
		result = NULL;
	} else {
		size_t i;
		DREF DeeObject *temp;
		/* No proxy-sequence for this one. This function just isn't used that much! */
		result = DeeTuple_NewUninitialized(me->a_multiple.m_astc);
		for (i = 0; i < me->a_multiple.m_astc; ++i) {
			temp = DeeCompiler_GetAst(me->a_multiple.m_astv[i]);
			if unlikely(!temp) {
				Dee_Decrefv(DeeTuple_ELEM(result), i);
				DeeTuple_FreeUninitialized(result);
				result = NULL;
				goto done;
			}
			DeeTuple_SET(result, i, temp); /* Inherit reference. */
		}
	}
done:
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delmultiple(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	size_t i;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto err;
	me = self->ci_value;
	if unlikely(me->a_type != AST_MULTIPLE) {
		result = err_invalid_ast_type(self, AST_MULTIPLE);
	} else {
		/* Assign the new branch vector. */
		for (i = 0; i < me->a_multiple.m_astc; ++i)
			Dee_Decref(me->a_multiple.m_astv[i]);
		Dee_Free(me->a_multiple.m_astv);
		me->a_multiple.m_astc = 0;
		me->a_multiple.m_astv = NULL;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setmultiple(Ast *__restrict self, DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	size_t i, new_astc, old_astc;
	DREF DeeCompilerAstObject **new_astv;
	DREF struct ast **old_astv;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_MULTIPLE) {
		result = err_invalid_ast_type(self, AST_MULTIPLE);
	} else {
		new_astv = (DREF DeeCompilerAstObject **)DeeSeq_AsHeapVector(value, &new_astc);
		if unlikely(!new_astv) {
err:
			result = -1;
		} else {
#ifdef CONFIG_AST_IS_STRUCT
#error "This loop doesn't work when asts are structs"
#endif
			for (i = 0; i < new_astc; ++i) {
				struct ast *branch_ast;
				if (DeeObject_AssertTypeExact(new_astv[i], &DeeCompilerAst_Type)) {
err_branch_v:
					for (i = 0; i < new_astc; ++i)
						Dee_Decref(new_astv[i]);
					Dee_Free(new_astv);
					goto err;
				}
				if unlikely(new_astv[i]->ci_compiler != DeeCompiler_Current) {
					err_invalid_ast_compiler(new_astv[i]);
					goto err_branch_v;
				}
				if unlikely(new_astv[i]->ci_value->a_scope->s_base != me->a_scope->s_base) {
					err_invalid_ast_basescope(new_astv[i], me->a_scope->s_base);
					goto err_branch_v;
				}
				branch_ast = new_astv[i]->ci_value;
				ast_incref(branch_ast);
				Dee_Decref(new_astv[i]);
				new_astv[i] = (DREF DeeCompilerAstObject *)branch_ast;
			}
			/* Assign the new branch vector. */
			old_astc              = me->a_multiple.m_astc;
			old_astv              = me->a_multiple.m_astv;
			me->a_multiple.m_astc = new_astc;
			me->a_multiple.m_astv = (DREF struct ast **)new_astv;
			for (i = 0; i < old_astc; ++i)
				Dee_Decref(old_astv[i]);
			Dee_Free(old_astv);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getmultiple_typing(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_MULTIPLE) {
		err_invalid_ast_type(self, AST_MULTIPLE);
		result = NULL;
	} else {
		switch (me->a_flag) {

		case AST_FMULTIPLE_TUPLE:
			result = (DeeObject *)&DeeTuple_Type;
			break;
		case AST_FMULTIPLE_LIST:
			result = (DeeObject *)&DeeList_Type;
			break;
		case AST_FMULTIPLE_HASHSET:
			result = (DeeObject *)&DeeHashSet_Type;
			break;
		case AST_FMULTIPLE_DICT:
			result = (DeeObject *)&DeeDict_Type;
			break;
		case AST_FMULTIPLE_GENERIC:
			result = (DeeObject *)&DeeSeq_Type;
			break;
		case AST_FMULTIPLE_GENERIC_KEYS:
			result = (DeeObject *)&DeeMapping_Type;
			break;

		default:
			result = Dee_None;
			break;
		}
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setmultiple_typing(Ast *__restrict self,
                       DeeTypeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_MULTIPLE) {
		result = err_invalid_ast_type(self, AST_MULTIPLE);
	} else {
		uint16_t new_flags;
		new_flags = get_ast_multiple_typing(value);
		if (new_flags == (uint16_t)-1) {
			result = -1;
		} else {
			me->a_flag = new_flags;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getreturnast(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_RETURN) {
		err_invalid_ast_type(self, AST_RETURN);
		result = NULL;
	} else if (!me->a_return) {
		err_unbound_attribute(Dee_TYPE(self), "returnast");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_return);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delreturnast(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_RETURN) {
		result = err_invalid_ast_type(self, AST_RETURN);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_return) {
		result = err_unbound_attribute(Dee_TYPE(self), "returnast");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_return);
		me->a_return = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setreturnast(Ast *__restrict self,
                 DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delreturnast(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_RETURN) {
		result = err_invalid_ast_type(self, AST_RETURN);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast      = me->a_return;
		me->a_return = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getyieldast(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_YIELD) {
		err_invalid_ast_type(self, AST_YIELD);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_yield);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setyieldast(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_YIELD) {
		result = err_invalid_ast_type(self, AST_YIELD);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast     = me->a_yield;
		me->a_yield = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getthrowast(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_THROW) {
		err_invalid_ast_type(self, AST_THROW);
		result = NULL;
	} else if (!me->a_throw) {
		err_unbound_attribute(Dee_TYPE(self), "throwast");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_throw);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delthrowast(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_THROW) {
		result = err_invalid_ast_type(self, AST_THROW);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_throw) {
		result = err_unbound_attribute(Dee_TYPE(self), "throwast");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_throw);
		me->a_throw = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setthrowast(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delthrowast(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_THROW) {
		result = err_invalid_ast_type(self, AST_THROW);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast     = me->a_throw;
		me->a_throw = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_gettryguard(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_TRY) {
		err_invalid_ast_type(self, AST_TRY);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_try.t_guard);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_settryguard(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_TRY) {
		result = err_invalid_ast_type(self, AST_TRY);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_try.t_guard;
		me->a_try.t_guard = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
ast_gettryhandlers(Ast *__restrict self) {
	DREF DeeTupleObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_TRY) {
		err_invalid_ast_type(self, AST_TRY);
		result = NULL;
	} else {
		size_t i;
		result = DeeTuple_NewUninitialized(me->a_try.t_catchc);
		if unlikely(!result)
			goto done;
		for (i = 0; i < me->a_try.t_catchc; ++i) {
			DREF DeeTupleObject *triple;
			DREF DeeObject *temp;
			triple = DeeTuple_NewUninitialized(3);
			if unlikely(!triple)
				goto err_r_i;
			switch (me->a_try.t_catchv[i].ce_flags &
			        (EXCEPTION_HANDLER_FFINALLY | EXCEPTION_HANDLER_FINTERPT)) {
			case EXCEPTION_HANDLER_FFINALLY | EXCEPTION_HANDLER_FINTERPT:
				temp = DeeString_New("finally, interrupt");
				break;
			case EXCEPTION_HANDLER_FFINALLY:
				temp = DeeString_New("finally");
				break;
			case EXCEPTION_HANDLER_FINTERPT:
				temp = DeeString_New("interrupt");
				break;
			default:
				temp = Dee_EmptyString;
				Dee_Incref(temp);
				break;
			}
			if unlikely(!temp) {
err_r_i_triple_0:
				DeeTuple_FreeUninitialized(triple);
				goto err_r_i;
			}
			DeeTuple_SET(triple, 0, temp); /* Inherit reference. */
			if (me->a_try.t_catchv[i].ce_mask) {
				temp = DeeCompiler_GetAst(me->a_try.t_catchv[i].ce_mask);
				if unlikely(!temp) {
err_r_i_triple_1:
					Dee_Decref(DeeTuple_GET(triple, 0));
					goto err_r_i_triple_0;
				}
			} else {
				temp = Dee_None;
				Dee_Incref(temp);
			}
			DeeTuple_SET(triple, 1, temp); /* Inherit reference. */
			temp = DeeCompiler_GetAst(me->a_try.t_catchv[i].ce_code);
			if unlikely(!temp) {
/*err_r_i_triple_2:*/
				Dee_Decref(DeeTuple_GET(triple, 1));
				goto err_r_i_triple_1;
			}
			DeeTuple_SET(triple, 2, temp);   /* Inherit reference. */
			DeeTuple_SET(result, i, triple); /* Inherit reference. */
		}
		goto done;
err_r_i:
		Dee_Decrefv(DeeTuple_ELEM(result), i);
		DeeTuple_FreeUninitialized(result);
		result = NULL;
	}
done:
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_settryhandlers(Ast *__restrict self,
                   DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_TRY) {
		result = err_invalid_ast_type(self, AST_TRY);
	} else {
		struct catch_expr *new_handv;
		size_t new_handc;
		struct catch_expr *old_handv;
		size_t old_handc, i;
		new_handv = unpack_catch_expressions(value, &new_handc, me->a_scope->s_base);
		if unlikely(!new_handv) {
			result = -1;
		} else {
			old_handv = me->a_try.t_catchv;
			old_handc = me->a_try.t_catchc;
			me->a_try.t_catchv = new_handv;
			me->a_try.t_catchc = new_handc;
			for (i = 0; i < old_handc; ++i) {
				ast_decref(old_handv[i].ce_code);
				ast_xdecref(old_handv[i].ce_mask);
			}
			Dee_Free(old_handv);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopisforeach(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FLOOP_FOREACH);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopisforeach(Ast *__restrict self,
                     DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (newval) {
		if (!me->a_loop.l_iter) {
			result = err_unbound_attribute(Dee_TYPE(self), "loopiter");
		} else {
			me->a_flag |= AST_FLOOP_FOREACH;
		}
	} else {
		me->a_flag &= ~AST_FLOOP_FOREACH;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopispostcond(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FLOOP_POSTCOND);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopispostcond(Ast *__restrict self,
                      DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (newval) {
		me->a_flag |= AST_FLOOP_POSTCOND;
	} else {
		me->a_flag &= ~AST_FLOOP_POSTCOND;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopisunlikely(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FLOOP_POSTCOND);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopisunlikely(Ast *__restrict self,
                      DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (newval) {
		me->a_flag |= AST_FLOOP_UNLIKELY;
	} else {
		me->a_flag &= ~AST_FLOOP_UNLIKELY;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopflags(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else {
		bool is_first                  = true;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (me->a_flag & AST_FLOOP_FOREACH) {
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "foreach") < 0)
				goto err_printer;
			is_first = false;
		}
		if (me->a_flag & AST_FLOOP_POSTCOND) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "postcond") < 0)
				goto err_printer;
			is_first = false;
		}
		if (me->a_flag & AST_FLOOP_UNLIKELY) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "unlikely") < 0)
				goto err_printer;
		}
		result = unicode_printer_pack(&printer);
		goto done;
err_printer:
		unicode_printer_fini(&printer);
		result = NULL;
	}
done:
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopflags(Ast *__restrict self,
                 DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	uint16_t new_flags;
	if (DeeString_Check(value)) {
		if unlikely(parse_loop_flags(DeeString_STR(value), &new_flags))
			goto err;
	} else {
		if unlikely(DeeObject_AsUInt16(value, &new_flags))
			goto err;
	}
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else {
		if ((new_flags & AST_FLOOP_FOREACH) && !me->a_loop.l_iter) {
			result = err_unbound_attribute(Dee_TYPE(self), "loopiter");
		} else {
			me->a_flag = new_flags;
		}
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE NONNULL((1)) int DCALL
err_is_a_foreach_loop(Ast *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Ast is a foreach-loop");
}

PRIVATE NONNULL((1)) int DCALL
err_not_a_foreach_loop(Ast *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Ast isn't a foreach-loop");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopcond(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		err_is_a_foreach_loop(self);
		result = NULL;
	} else if (!me->a_loop.l_cond) {
		err_unbound_attribute(Dee_TYPE(self), "loopcond");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_cond);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delloopcond(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		result = err_is_a_foreach_loop(self);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_loop.l_cond) {
		result = err_unbound_attribute(Dee_TYPE(self), "loopcond");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_loop.l_cond);
		me->a_loop.l_cond = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopcond(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delloopcond(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		result = err_is_a_foreach_loop(self);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_cond;
		me->a_loop.l_cond = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopnext(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		err_is_a_foreach_loop(self);
		result = NULL;
	} else if (!me->a_loop.l_next) {
		err_unbound_attribute(Dee_TYPE(self), "loopnext");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_next);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delloopnext(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		result = err_is_a_foreach_loop(self);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_loop.l_next) {
		result = err_unbound_attribute(Dee_TYPE(self), "loopnext");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_loop.l_next);
		me->a_loop.l_next = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopnext(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delloopnext(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		result = err_is_a_foreach_loop(self);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_next;
		me->a_loop.l_next = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopelem(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (!(me->a_flag & AST_FLOOP_FOREACH)) {
		err_not_a_foreach_loop(self);
		result = NULL;
	} else if (!me->a_loop.l_elem) {
		err_unbound_attribute(Dee_TYPE(self), "loopelem");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_elem);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delloopelem(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (!(me->a_flag & AST_FLOOP_FOREACH)) {
		result = err_not_a_foreach_loop(self);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_loop.l_elem) {
		result = err_unbound_attribute(Dee_TYPE(self), "loopelem");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_loop.l_elem);
		me->a_loop.l_elem = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopelem(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delloopelem(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (!(me->a_flag & AST_FLOOP_FOREACH)) {
		result = err_not_a_foreach_loop(self);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_elem;
		me->a_loop.l_elem = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopiter(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (!(me->a_flag & AST_FLOOP_FOREACH)) {
		err_not_a_foreach_loop(self);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_iter);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopiter(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (!(me->a_flag & AST_FLOOP_FOREACH)) {
		result = err_not_a_foreach_loop(self);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_iter;
		me->a_loop.l_iter = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getlooploop(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (!me->a_loop.l_loop) {
		err_unbound_attribute(Dee_TYPE(self), "looploop");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_loop);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_dellooploop(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_loop.l_loop) {
		result = err_unbound_attribute(Dee_TYPE(self), "looploop");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_loop.l_loop);
		me->a_loop.l_loop = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setlooploop(Ast *__restrict self,
                DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_dellooploop(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_loop;
		me->a_loop.l_loop = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopelemcond(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (!me->a_loop.l_elem) {
		err_unbound_attribute(Dee_TYPE(self), "loopelemcond");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_elem);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delloopelemcond(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_loop.l_elem) {
		result = err_unbound_attribute(Dee_TYPE(self), "loopelemcond");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_loop.l_elem);
		me->a_loop.l_elem = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopelemcond(Ast *__restrict self,
                    DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delloopelemcond(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_elem;
		me->a_loop.l_elem = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopiternext(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		err_invalid_ast_type(self, AST_LOOP);
		result = NULL;
	} else if (!me->a_loop.l_iter) {
		err_unbound_attribute(Dee_TYPE(self), "loopiternext");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_loop.l_iter);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delloopiternext(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (me->a_flag & AST_FLOOP_FOREACH) {
		result = err_cant_access_attribute(Dee_TYPE(self), "loopiternext", ATTR_ACCESS_DEL);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_loop.l_iter) {
		result = err_unbound_attribute(Dee_TYPE(self), "loopiternext");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_loop.l_iter);
		me->a_loop.l_iter = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopiternext(Ast *__restrict self,
                    DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delloopiternext(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast           = me->a_loop.l_iter;
		me->a_loop.l_iter = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getloopctlisbreak(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOPCTL) {
		err_invalid_ast_type(self, AST_LOOPCTL);
		result = NULL;
	} else {
		result = DeeBool_For(!(me->a_flag & AST_FLOOPCTL_CON));
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setloopctlisbreak(Ast *__restrict self,
                      DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_LOOP) {
		result = err_invalid_ast_type(self, AST_LOOP);
	} else if (newval) {
		me->a_flag &= ~AST_FLOOPCTL_CON;
	} else {
		me->a_flag |= AST_FLOOPCTL_CON;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionalcond(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_conditional.c_cond);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionalcond(Ast *__restrict self,
                       DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast                  = me->a_conditional.c_cond;
		me->a_conditional.c_cond = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionaltt(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else if (!me->a_conditional.c_tt) {
		err_unbound_attribute(Dee_TYPE(self), "conditionaltt");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_conditional.c_tt);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delconditionaltt(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_conditional.c_tt) {
		result = err_unbound_attribute(Dee_TYPE(self), "conditionaltt");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_conditional.c_tt);
		me->a_conditional.c_tt = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionaltt(Ast *__restrict self,
                     DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delconditionaltt(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast                = me->a_conditional.c_tt;
		me->a_conditional.c_tt = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionalff(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else if (!me->a_conditional.c_ff) {
		err_unbound_attribute(Dee_TYPE(self), "conditionalff");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_conditional.c_ff);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_delconditionalff(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_conditional.c_ff) {
		result = err_unbound_attribute(Dee_TYPE(self), "conditionalff");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_conditional.c_ff);
		me->a_conditional.c_ff = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionalff(Ast *__restrict self,
                     DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_delconditionalff(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast                = me->a_conditional.c_ff;
		me->a_conditional.c_ff = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionalflags(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else {
		bool is_first                  = true;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (me->a_flag & AST_FCOND_BOOL) {
			if unlikely(unicode_printer_print(&printer, STR_bool, 4) < 0)
				goto err_printer;
			is_first = false;
		}
		if (me->a_flag & AST_FCOND_LIKELY) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "likely") < 0)
				goto err_printer;
			is_first = false;
		}
		if (me->a_flag & AST_FCOND_UNLIKELY) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "unlikely") < 0)
				goto err_printer;
		}
		result = unicode_printer_pack(&printer);
		goto done;
err_printer:
		unicode_printer_fini(&printer);
		result = NULL;
	}
done:
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionalflags(Ast *__restrict self,
                        DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	uint16_t new_flags;
	if (DeeString_Check(value)) {
		if unlikely(parse_conditional_flags(DeeString_STR(value), &new_flags))
			goto err;
	} else {
		if unlikely(DeeObject_AsUInt16(value, &new_flags))
			goto err;
	}
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else {
		me->a_flag = new_flags;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionalisbool(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FCOND_BOOL);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionalisbool(Ast *__restrict self,
                         DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else if (newval) {
		me->a_flag |= AST_FCOND_BOOL;
	} else {
		me->a_flag &= ~AST_FCOND_BOOL;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionalislikely(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FCOND_LIKELY);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionalislikely(Ast *__restrict self,
                           DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else if (newval) {
		me->a_flag |= AST_FCOND_LIKELY;
	} else {
		me->a_flag &= ~AST_FCOND_LIKELY;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getconditionalisunlikely(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		err_invalid_ast_type(self, AST_CONDITIONAL);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FCOND_UNLIKELY);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setconditionalisunlikely(Ast *__restrict self,
                             DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_CONDITIONAL) {
		result = err_invalid_ast_type(self, AST_CONDITIONAL);
	} else if (newval) {
		me->a_flag |= AST_FCOND_UNLIKELY;
	} else {
		me->a_flag &= ~AST_FCOND_UNLIKELY;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getboolast(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_BOOL) {
		err_invalid_ast_type(self, AST_BOOL);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_bool);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setboolast(Ast *__restrict self,
               DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_BOOL) {
		result = err_invalid_ast_type(self, AST_BOOL);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast    = me->a_bool;
		me->a_bool = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getboolisnegated(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_BOOL) {
		err_invalid_ast_type(self, AST_BOOL);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_flag & AST_FBOOL_NEGATE);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setboolisnegated(Ast *__restrict self,
                     DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_BOOL) {
		result = err_invalid_ast_type(self, AST_BOOL);
	} else if (newval) {
		me->a_flag |= AST_FBOOL_NEGATE;
	} else {
		me->a_flag &= ~AST_FBOOL_NEGATE;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getexpandast(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_EXPAND) {
		err_invalid_ast_type(self, AST_EXPAND);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_expand);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setexpandast(Ast *__restrict self,
                 DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_EXPAND) {
		result = err_invalid_ast_type(self, AST_EXPAND);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast      = me->a_expand;
		me->a_expand = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getfunctioncode(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_FUNCTION) {
		err_invalid_ast_type(self, AST_FUNCTION);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_function.f_code);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setfunctioncode(Ast *__restrict self,
                    DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_EXPAND) {
		result = err_invalid_ast_type(self, AST_EXPAND);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else {
		DeeBaseScopeObject *code_scope;
		code_scope = value->ci_value->a_scope->s_base;
		if unlikely(check_function_code_scope(code_scope, me->a_scope->s_base)) {
			result = -1;
		} else {
			DREF DeeBaseScopeObject *old_scope;
			DREF struct ast *old_code;
			ast_incref(value->ci_value);
			Dee_Incref((DeeObject *)code_scope);
			old_code               = me->a_function.f_code;
			old_scope              = me->a_function.f_scope;
			me->a_function.f_code  = value->ci_value;
			me->a_function.f_scope = code_scope;
			ast_decref(old_code);
			Dee_Decref((DeeObject *)old_scope);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
get_operator_name(uint16_t opid) {
	struct opinfo const *info;
	switch (opid) {
	case AST_OPERATOR_POS_OR_ADD:
		return DeeString_Chr((uint8_t)'+');
	case AST_OPERATOR_NEG_OR_SUB:
		return DeeString_Chr((uint8_t)'-');
	case AST_OPERATOR_GETITEM_OR_SETITEM:
		return DeeString_NewSized("[]", 2);
	case AST_OPERATOR_GETRANGE_OR_SETRANGE:
		return DeeString_NewSized("[:]", 3);
	case AST_OPERATOR_GETATTR_OR_SETATTR:
		return DeeString_Chr((uint8_t)'.');
	default: break;
	}
	info = Dee_OperatorInfo(NULL, opid);
	if unlikely(!info)
		return DeeInt_NewU16(opid);
	return DeeString_New(info->oi_sname);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorfuncname(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		err_invalid_ast_type(self, AST_OPERATOR_FUNC);
		result = NULL;
	} else {
		result = get_operator_name(me->a_flag);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorfuncname(Ast *__restrict self,
                        DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		result = err_invalid_ast_type(self, AST_OPERATOR_FUNC);
	} else {
		uint16_t new_id;
		result = get_operator_id(value, &new_id);
		if likely(!result)
			me->a_flag = new_id;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorfuncbinding(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		err_invalid_ast_type(self, AST_OPERATOR_FUNC);
		result = NULL;
	} else if (!me->a_operator_func.of_binding) {
		err_unbound_attribute(Dee_TYPE(self), "operatorfuncbinding");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_operator_func.of_binding);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_deloperatorfuncbinding(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		result = err_invalid_ast_type(self, AST_OPERATOR_FUNC);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_operator_func.of_binding) {
		result = err_unbound_attribute(Dee_TYPE(self), "operatorfuncbinding");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_operator_func.of_binding);
		me->a_operator_func.of_binding = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorfuncbinding(Ast *__restrict self,
                           DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_deloperatorfuncbinding(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		result = err_invalid_ast_type(self, AST_OPERATOR_FUNC);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast                        = me->a_operator_func.of_binding;
		me->a_operator_func.of_binding = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorname(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		result = get_operator_name(me->a_flag);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorname(Ast *__restrict self,
                    DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else {
		uint16_t new_id;
		result = get_operator_id(value, &new_id);
		if likely(!result)
			me->a_flag = new_id;
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatora(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_operator.o_op0);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatora(Ast *__restrict self,
                 DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast              = me->a_operator.o_op0;
		me->a_operator.o_op0 = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorb(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else if (!me->a_operator.o_op1) {
		err_unbound_attribute(Dee_TYPE(self), "operatorb");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_operator.o_op1);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_deloperatorb(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		result = err_invalid_ast_type(self, AST_OPERATOR_FUNC);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_operator.o_op1) {
		result = err_unbound_attribute(Dee_TYPE(self), "operatorb");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_operator.o_op1);
		me->a_operator.o_op1 = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorb(Ast *__restrict self,
                 DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_deloperatorb(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast              = me->a_operator.o_op1;
		me->a_operator.o_op1 = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorc(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else if (!me->a_operator.o_op2) {
		err_unbound_attribute(Dee_TYPE(self), "operatorc");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_operator.o_op2);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_deloperatorc(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		result = err_invalid_ast_type(self, AST_OPERATOR_FUNC);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_operator.o_op2) {
		result = err_unbound_attribute(Dee_TYPE(self), "operatorc");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_operator.o_op2);
		me->a_operator.o_op2 = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorc(Ast *__restrict self,
                 DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_deloperatorc(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast              = me->a_operator.o_op2;
		me->a_operator.o_op2 = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatord(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else if (!me->a_operator.o_op3) {
		err_unbound_attribute(Dee_TYPE(self), "operatord");
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_operator.o_op3);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_deloperatord(Ast *__restrict self) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR_FUNC) {
		result = err_invalid_ast_type(self, AST_OPERATOR_FUNC);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	} else if (!me->a_operator.o_op3) {
		result = err_unbound_attribute(Dee_TYPE(self), "operatord");
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		ast_decref(me->a_operator.o_op3);
		me->a_operator.o_op3 = NULL;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatord(Ast *__restrict self,
                 DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (DeeNone_Check(value))
		return ast_deloperatord(self);
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast              = me->a_operator.o_op3;
		me->a_operator.o_op3 = value->ci_value;
		ast_xdecref(old_ast);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorflags(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		bool is_first                  = true;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (me->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "post") < 0)
				goto err_printer;
			is_first = false;
		}
		if (me->a_operator.o_exflag & AST_OPERATOR_FVARARGS) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "varargs") < 0)
				goto err_printer;
			is_first = false;
		}
		if (me->a_operator.o_exflag & AST_OPERATOR_FMAYBEPFX) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "maybeprefix") < 0)
				goto err_printer;
		}
		if (me->a_operator.o_exflag & AST_OPERATOR_FDONTOPT) {
			if (!is_first &&
			    unlikely(unicode_printer_put8(&printer, ',')))
				goto err_printer;
			if unlikely(UNICODE_PRINTER_PRINT(&printer, "dontoptimize") < 0)
				goto err_printer;
		}
		result = unicode_printer_pack(&printer);
		goto done;
err_printer:
		unicode_printer_fini(&printer);
		result = NULL;
	}
done:
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorflags(Ast *__restrict self,
                     DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	uint16_t new_flags;
	if (DeeString_Check(value)) {
		if unlikely(parse_operator_flags(DeeString_STR(value), &new_flags))
			goto err;
	} else {
		if unlikely(DeeObject_AsUInt16(value, &new_flags))
			goto err;
	}
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else {
		me->a_operator.o_exflag = new_flags;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorispost(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_operator.o_exflag & AST_OPERATOR_FPOSTOP);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorispost(Ast *__restrict self,
                      DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (newval) {
		me->a_operator.o_exflag |= AST_OPERATOR_FPOSTOP;
	} else {
		me->a_operator.o_exflag &= ~AST_OPERATOR_FPOSTOP;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorisvarargs(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_operator.o_exflag & AST_OPERATOR_FVARARGS);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorisvarargs(Ast *__restrict self,
                         DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (newval) {
		me->a_operator.o_exflag |= AST_OPERATOR_FVARARGS;
	} else {
		me->a_operator.o_exflag &= ~AST_OPERATOR_FVARARGS;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorismaybeprefix(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_operator.o_exflag & AST_OPERATOR_FMAYBEPFX);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorismaybeprefix(Ast *__restrict self,
                             DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (newval) {
		me->a_operator.o_exflag |= AST_OPERATOR_FMAYBEPFX;
	} else {
		me->a_operator.o_exflag &= ~AST_OPERATOR_FMAYBEPFX;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getoperatorisdontoptimize(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		err_invalid_ast_type(self, AST_OPERATOR);
		result = NULL;
	} else {
		result = DeeBool_For(me->a_operator.o_exflag & AST_OPERATOR_FDONTOPT);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setoperatorisdontoptimize(Ast *__restrict self,
                              DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto err;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_OPERATOR) {
		result = err_invalid_ast_type(self, AST_OPERATOR);
	} else if (newval) {
		me->a_operator.o_exflag |= AST_OPERATOR_FDONTOPT;
	} else {
		me->a_operator.o_exflag &= ~AST_OPERATOR_FDONTOPT;
	}
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getactionname(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		err_invalid_ast_type(self, AST_ACTION);
		result = NULL;
	} else {
		char const *name = get_action_name(me->a_flag);
		result           = name ? DeeString_New(name) : DeeInt_NewU16(me->a_flag);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setactionname(Ast *__restrict self,
                  DeeObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		result = err_invalid_ast_type(self, AST_ACTION);
	} else if (DeeObject_AssertType(value, &DeeString_Type)) {
		result = -1;
	} else {
		int32_t new_id;
		new_id = get_action_by_name(DeeString_STR(value));
		if unlikely(new_id < 0) {
			result = -1;
		} else if (AST_FACTION_ARGC_GT(new_id) ==
		           AST_FACTION_ARGC_GT(me->a_flag)) {
			me->a_flag = (uint16_t)new_id;
		} else if ((uint16_t)new_id == AST_FACTION_ASSERT &&
		         AST_FACTION_ARGC_GT(me->a_flag) == 2) {
			me->a_flag = AST_FACTION_ASSERT_M;
		} else {
			result = DeeError_Throwf(&DeeError_ValueError,
			                         "Attempted to set a new action %q taking %u "
			                         "arguments, when the old action only uses %u",
			                         DeeString_STR(value),
			                         (unsigned int)AST_FACTION_ARGC_GT(new_id),
			                         (unsigned int)AST_FACTION_ARGC_GT(me->a_flag));
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getactiona(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		err_invalid_ast_type(self, AST_ACTION);
		result = NULL;
	} else if (AST_FACTION_ARGC_GT(me->a_flag) < 1) {
		err_unknown_attribute(Dee_TYPE(self), "actiona", ATTR_ACCESS_GET);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_action.a_act0);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setactiona(Ast *__restrict self,
               DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		result = err_invalid_ast_type(self, AST_ACTION);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else if (AST_FACTION_ARGC_GT(me->a_flag) < 1) {
		result = err_unknown_attribute(Dee_TYPE(self), "actiona", ATTR_ACCESS_SET);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast             = me->a_action.a_act0;
		me->a_action.a_act0 = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getactionb(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		err_invalid_ast_type(self, AST_ACTION);
		result = NULL;
	} else if (AST_FACTION_ARGC_GT(me->a_flag) < 2) {
		err_unknown_attribute(Dee_TYPE(self), "actionb", ATTR_ACCESS_GET);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_action.a_act1);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setactionb(Ast *__restrict self,
               DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		result = err_invalid_ast_type(self, AST_ACTION);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else if (AST_FACTION_ARGC_GT(me->a_flag) < 2) {
		result = err_unknown_attribute(Dee_TYPE(self), "actionb", ATTR_ACCESS_SET);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast             = me->a_action.a_act1;
		me->a_action.a_act1 = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ast_getactionc(Ast *__restrict self) {
	DREF DeeObject *result;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return NULL;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		err_invalid_ast_type(self, AST_ACTION);
		result = NULL;
	} else if (AST_FACTION_ARGC_GT(me->a_flag) < 3) {
		err_unknown_attribute(Dee_TYPE(self), "actionc", ATTR_ACCESS_GET);
		result = NULL;
	} else {
		result = DeeCompiler_GetAst(me->a_action.a_act2);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ast_setactionc(Ast *__restrict self,
               DeeCompilerAstObject *__restrict value) {
	int result = 0;
	struct ast *me;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	me = self->ci_value;
	if unlikely(me->a_type != AST_ACTION) {
		result = err_invalid_ast_type(self, AST_ACTION);
	} else if (DeeObject_AssertTypeExact(value, &DeeCompilerAst_Type)) {
		result = -1;
	} else if (value->ci_compiler != DeeCompiler_Current) {
		result = err_invalid_ast_compiler(value);
	} else if (value->ci_value->a_scope->s_base != me->a_scope->s_base) {
		result = err_invalid_ast_basescope(value, me->a_scope->s_base);
	} else if (AST_FACTION_ARGC_GT(me->a_flag) < 3) {
		result = err_unknown_attribute(Dee_TYPE(self), "actionc", ATTR_ACCESS_SET);
	} else {
		DREF struct ast *old_ast;
		ast_incref(value->ci_value);
		old_ast             = me->a_action.a_act2;
		me->a_action.a_act2 = value->ci_value;
		ast_decref(old_ast);
	}
	COMPILER_END();
	return result;
}


#define DO(x)                         \
	do {                              \
		if unlikely((temp = (x)) < 0) \
			goto err;                 \
		result += temp;               \
	}	__WHILE0
#define print(p, s)   DO(DeeFormat_Print(printer, arg, p, s))
#define printf(...)   DO(DeeFormat_Printf(printer, arg, __VA_ARGS__))
#define PRINT(str)    DO(DeeFormat_PRINT(printer, arg, str))
#define PRINTAST(obj) DO(print_ast_repr(obj, printer, arg))

#define PRINT_TRUE()  print(STR_true, 4)
#define PRINT_FALSE() print(STR_false, 5)
#define PRINT_NONE()  print(STR_none, 4)


PRIVATE WUNUSED NONNULL((2, 3, 6, 7)) dssize_t DCALL
print_enter_scope(DeeScopeObject *caller_scope,
                  DeeScopeObject *__restrict child_scope,
                  dformatprinter printer, void *arg,
                  bool is_expression,
                  size_t *__restrict p_indent,
                  bool *__restrict p_is_scope) {
	dssize_t temp, result = 0;
	if (child_scope == caller_scope)
		return 0;
	if (is_expression) {
		PRINT("({\n");
		++*p_indent;
		*p_is_scope = true;
	} else if (caller_scope) {
		PRINT("{\n");
		++*p_indent;
		*p_is_scope = true;
	}
	DO(DeeFormat_Repeat(printer, arg, '\t', *p_indent));
	for (; child_scope && child_scope != caller_scope; child_scope = child_scope->s_prev) {
		size_t i;
		struct symbol *sym;
		if (!child_scope->s_mapc)
			continue;
		for (i = 0; i < child_scope->s_mapa; ++i) {
			sym = child_scope->s_map[i];
			for (; sym; sym = sym->s_next) {
				if (sym->s_type != SYMBOL_TYPE_GLOBAL &&
				    sym->s_type != SYMBOL_TYPE_EXTERN &&
				    sym->s_type != SYMBOL_TYPE_MODULE &&
				    sym->s_type != SYMBOL_TYPE_MYMOD &&
				    sym->s_type != SYMBOL_TYPE_LOCAL &&
				    sym->s_type != SYMBOL_TYPE_STACK &&
				    sym->s_type != SYMBOL_TYPE_STATIC)
					continue;
				switch (sym->s_type) {
				case SYMBOL_TYPE_EXTERN:
					if (MODULE_SYMBOL_EQUALS(sym->s_extern.e_symbol,
					                         sym->s_name->k_name,
					                         sym->s_name->k_size)) {
						printf("import %s from %k",
						       sym->s_extern.e_symbol->ss_name,
						       sym->s_extern.e_module->mo_name);
					} else {
						printf("import %$s = %s from %k",
						       sym->s_name->k_size,
						       sym->s_name->k_name,
						       sym->s_extern.e_symbol->ss_name,
						       sym->s_extern.e_module->mo_name);
					}
					break;
				case SYMBOL_TYPE_MODULE:
					if (sym->s_name->k_size == DeeString_SIZE(sym->s_module->mo_name) &&
					    bcmpc(sym->s_name->k_name, DeeString_STR(sym->s_module->mo_name),
					          sym->s_name->k_size, sizeof(char)) == 0) {
						printf("import %k", sym->s_module->mo_name);
					} else {
						printf("import %$s = %k",
						       sym->s_name->k_size,
						       sym->s_name->k_name,
						       sym->s_module->mo_name);
					}
					break;
				case SYMBOL_TYPE_MYMOD:
					printf("import %$s = .",
					       sym->s_name->k_size,
					       sym->s_name->k_name);
					break;
				case SYMBOL_TYPE_GLOBAL:
					PRINT("global ");
					goto print_symbol_name;
				case SYMBOL_TYPE_LOCAL:
					PRINT("local ");
					goto print_symbol_name;
				case SYMBOL_TYPE_STACK:
					PRINT("__stack local ");
					goto print_symbol_name;
				case SYMBOL_TYPE_STATIC:
					PRINT("static local ");
print_symbol_name:
					print(sym->s_name->k_name, sym->s_name->k_size);
					break;
				default: break;
				}
				PRINT(";\n");
				DO(DeeFormat_Repeat(printer, arg, '\t', *p_indent));
			}
		}
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
print_leave_scope(dformatprinter printer, void *arg,
                  bool is_expression, bool need_semicolon,
                  size_t indent, bool is_scope) {
	dssize_t temp, result = 0;
	if (is_scope) {
		if (need_semicolon) {
			PRINT(";\n");
		} else {
			PRINT("\n");
		}
		DO(DeeFormat_Repeat(printer, arg, '\t', indent - 1));
		PRINT("}");
		if (is_expression)
			PRINT(")");
	} else if (!is_expression && need_semicolon) {
		PRINT(";");
	}
	return result;
err:
	return temp;
}


#define ENTER_SCOPE(is_scope, caller_scope, child_scope, is_expression) \
	do {                                                                \
		bool is_scope = false;                                          \
		DO(print_enter_scope(caller_scope, child_scope,                 \
		                     printer, arg, is_expression,               \
		                     &indent, &is_scope))
#define LEAVE_SCOPE(is_scope, is_expression, need_semicolon)                                  \
		DO(print_leave_scope(printer, arg, is_expression, need_semicolon, indent, is_scope)); \
	}	__WHILE0

INTDEF WUNUSED NONNULL((1)) bool DCALL
DeeString_IsSymbol(DeeStringObject *__restrict self,
                   size_t start_index,
                   size_t end_index);
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_ast_code(struct ast *__restrict self,
               dformatprinter printer, void *arg, bool is_expression,
               DeeScopeObject *caller_scope, size_t indent);

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
print_symbol(struct symbol *__restrict sym,
             DeeScopeObject *__restrict ref_scope,
             dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
	if (sym->s_name == &TPPKeyword_Empty) {
		if (sym->s_type == SYMBOL_TYPE_EXTERN) {
			if (sym->s_extern.e_module == &deemon_module &&
			    sym->s_extern.e_symbol->ss_index == id_import) {
				print(STR_import, 6);
			} else {
				PRINT("(");
				DO((*printer)(arg, sym->s_extern.e_symbol->ss_name,
				              strlen(sym->s_extern.e_symbol->ss_name)));
				PRINT(" from ");
				DO(DeeObject_Print((DeeObject *)sym->s_extern.e_module->mo_name, printer, arg));
				PRINT(")");
			}
			goto done;
		}
		if (sym->s_type == SYMBOL_TYPE_MODULE) {
			printf("({ import _sym = %k; _sym; })",
			       sym->s_module->mo_name);
			goto done;
		}
		if (sym->s_type == SYMBOL_TYPE_MYMOD) {
			PRINT("({ import _sym = .; _sym; })");
			goto done;
		}
	}
	(void)ref_scope; /* TODO: __nth symbols? */
	if (sym->s_name == &TPPKeyword_Empty) {
		PRINT("__TPP_IDENTIFIER(\"\")"); /* ??? */
	} else {
		print(sym->s_name->k_name, sym->s_name->k_size);
	}
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_code_tags(DeeBaseScopeObject *__restrict function_scope,
                dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
	if (function_scope->bs_flags & CODE_FCOPYABLE)
		PRINT("@copyable ");
	if (function_scope->bs_flags & CODE_FTHISCALL)
		PRINT("@thiscall ");
	if (function_scope->bs_flags & CODE_FASSEMBLY)
		PRINT("@assembly ");
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
is_instance_method(DeeBaseScopeObject *__restrict self) {
	if (!(self->bs_flags & CODE_FTHISCALL))
		return false;
	/* TODO: Use the this-symbol to check for instance methods. */
	return true;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_function_atargs(struct ast *__restrict self,
                      dformatprinter printer, void *arg,
                      size_t indent, bool print_tags) {
	dssize_t temp, result = 0;
	DeeBaseScopeObject *function_scope;
	size_t i;
	function_scope = self->a_function.f_scope;
	if (print_tags)
		DO(print_code_tags(function_scope, printer, arg));
	PRINT("(");
	for (i = 0; i < function_scope->bs_argc; ++i) {
		struct symbol *argsym = function_scope->bs_argv[i];
		if (i != 0)
			PRINT(", ");
		if (argsym == function_scope->bs_varkwds)
			PRINT("**");
		print(argsym->s_name->k_name, argsym->s_name->k_size);
		if (argsym == function_scope->bs_varargs) {
			PRINT("...");
		} else if (argsym == function_scope->bs_varkwds) {
			/* ... */
		} else if (i >= function_scope->bs_argc_min && i < function_scope->bs_argc_max) {
			DeeObject *defl = function_scope->bs_default[i - function_scope->bs_argc_min];
			if (defl) {
				printf(" = %r", defl);
			} else {
				PRINT("?");
			}
		}
	}
	PRINT(") ");
	DO(print_ast_code(self->a_function.f_code, printer, arg, false, self->a_scope, indent));
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_asm_operator(struct asm_operand *__restrict operand,
                   dformatprinter printer, void *arg,
                   DeeScopeObject *caller_scope,
                   size_t indent) {
	dssize_t temp, result = 0;
#ifndef CONFIG_LANGUAGE_NO_ASM
	if (operand->ao_name)
		printf("[%$s] ", operand->ao_name->k_size, operand->ao_name->k_name);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	ASSERT(operand->ao_type);
	printf("%$q (",
	       operand->ao_type->s_size,
	       operand->ao_type->s_text);
	DO(print_ast_code(operand->ao_expr, printer, arg, true, caller_scope, indent));
	PRINT(")");
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_asm_label_operator(struct asm_operand *__restrict operand,
                         dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
#ifndef CONFIG_LANGUAGE_NO_ASM
	if (operand->ao_name)
		printf("[%$s] ", operand->ao_name->k_size, operand->ao_name->k_name);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	ASSERT(!operand->ao_type);
	print(operand->ao_label->tl_name->k_name,
	      operand->ao_label->tl_name->k_size);
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) struct class_member *DCALL
find_class_member(struct ast *__restrict self, uint16_t index) {
	size_t i;
	for (i = 0; i < self->a_class.c_memberc; ++i) {
		if (self->a_class.c_memberv[i].cm_index != index)
			continue;
		return &self->a_class.c_memberv[i];
	}
	return NULL;
}

PRIVATE char const property_names[3][4] = { "get", "del", "set" };


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_ast_code(struct ast *__restrict self,
               dformatprinter printer, void *arg, bool is_expression,
               DeeScopeObject *caller_scope, size_t indent) {
	dssize_t temp, result = 0;
	bool need_semicolon = true;
	ENTER_SCOPE(is_scope, caller_scope, self->a_scope, is_expression);
	__IF0 {
force_scope:
		if (is_expression)
			PRINT("(");
		PRINT("{\n");
		++indent;
		DO(DeeFormat_Repeat(printer, arg, '\t', indent));
		is_scope = true;
	}
	switch (self->a_type) {

	case AST_CONSTEXPR:
		DO(DeeObject_PrintRepr(self->a_constexpr, printer, arg));
		break;

	case AST_SYM:
		DO(print_symbol(self->a_sym, self->a_scope, printer, arg));
		break;

	case AST_UNBIND:
		PRINT("del(");
		DO(print_symbol(self->a_sym, self->a_scope, printer, arg));
		PRINT(")");
		break;

	case AST_BOUND:
		PRINT("(");
		DO(print_symbol(self->a_sym, self->a_scope, printer, arg));
		PRINT(" is bound)");
		break;

	case AST_MULTIPLE: {
		size_t i;
		switch (self->a_flag) {

		case AST_FMULTIPLE_TUPLE:
			if (!self->a_multiple.m_astc) {
				PRINT("pack()");
			} else {
				PRINT("(");
				for (i = 0; i < self->a_multiple.m_astc; ++i) {
					DO(print_ast_code(self->a_multiple.m_astv[i], printer, arg, true, self->a_scope, indent + 1));
					if (i < self->a_multiple.m_astc - 1)
						PRINT(", ");
				}
				if (self->a_multiple.m_astc == 1) {
					PRINT(",)");
				} else {
					PRINT(")");
				}
			}
			break;

		case AST_FMULTIPLE_LIST:
			PRINT("[");
			for (i = 0; i < self->a_multiple.m_astc; ++i) {
				DO(print_ast_code(self->a_multiple.m_astv[i], printer, arg, true, self->a_scope, indent + 1));
				if (i < self->a_multiple.m_astc - 1)
					PRINT(", ");
			}
			PRINT("]");
			break;

		case AST_FMULTIPLE_HASHSET:
		case AST_FMULTIPLE_GENERIC:
			PRINT("{ ");
			for (i = 0; i < self->a_multiple.m_astc; ++i) {
				DO(print_ast_code(self->a_multiple.m_astv[i], printer, arg, true, self->a_scope, indent + 1));
				if (i < self->a_multiple.m_astc - 1)
					PRINT(", ");
			}
			PRINT(" }");
			break;

		case AST_FMULTIPLE_DICT:
		case AST_FMULTIPLE_GENERIC_KEYS:
			PRINT("{ ");
			for (i = 0; i < self->a_multiple.m_astc / 2; ++i) {
				DO(print_ast_code(self->a_multiple.m_astv[i * 2 + 0], printer, arg, true, self->a_scope, indent + 1));
				PRINT(": ");
				DO(print_ast_code(self->a_multiple.m_astv[i * 2 + 1], printer, arg, true, self->a_scope, indent + 1));
				if (i < (self->a_multiple.m_astc / 2) - 1)
					PRINT(", ");
			}
			PRINT(" }");
			break;

		default:
			if (!is_scope && is_expression)
				goto force_scope;
			if (!self->a_multiple.m_astc) {
				PRINT("none;");
			} else {
				for (i = 0; i < self->a_multiple.m_astc; ++i) {
					DO(print_ast_code(self->a_multiple.m_astv[i], printer, arg, false, self->a_scope, indent));
					if (i < self->a_multiple.m_astc - 1) {
						PRINT("\n");
						DO(DeeFormat_Repeat(printer, arg, '\t', indent));
					}
				}
			}
			need_semicolon = false;
			break;
		}
	}	break;

	case AST_RETURN:
		if (!is_scope && is_expression)
			goto force_scope;
		print(STR_return, 6);
		if (self->a_return) {
			PRINT(" ");
			DO(print_ast_code(self->a_return, printer, arg, true, self->a_scope, indent));
		}
		break;

	case AST_YIELD:
		if (!is_scope && is_expression)
			goto force_scope;
		PRINT("yield ");
		DO(print_ast_code(self->a_yield, printer, arg, true, self->a_scope, indent));
		break;

	case AST_THROW:
		if (!is_scope && is_expression)
			goto force_scope;
		print(STR_throw, 5);
		if (self->a_throw) {
			PRINT(" ");
			DO(print_ast_code(self->a_throw, printer, arg, true, self->a_scope, indent));
		} else {
		}
		break;

	case AST_TRY: {
		size_t i;
		if (!self->a_try.t_catchc) {
			DO(print_ast_code(self->a_try.t_guard, printer, arg,
			                  is_expression && !is_scope,
			                  self->a_scope, indent));
			break;
		}
		PRINT("try ");
		DO(print_ast_code(self->a_try.t_guard, printer, arg,
		                  is_expression && !is_scope,
		                  self->a_scope, indent));
		need_semicolon = false;
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			struct catch_expr *handler;
			handler = &self->a_try.t_catchv[i];
			if (handler->ce_flags & EXCEPTION_HANDLER_FFINALLY) {
				PRINT(" finally ");
				if (handler->ce_mask) {
					PRINT("{\n");
					DO(DeeFormat_Repeat(printer, arg, '\t', indent + 1));
					DO(print_ast_code(handler->ce_mask, printer, arg, false,
					                  self->a_scope, indent + 1));
					PRINT(";\n");
					DO(DeeFormat_Repeat(printer, arg, '\t', indent + 1));
					DO(print_ast_code(handler->ce_code, printer, arg,
					                  is_expression && !is_scope,
					                  self->a_scope, indent + 1));
					PRINT(";\n");
					DO(DeeFormat_Repeat(printer, arg, '\t', indent));
					PRINT("}");
				} else {
					DO(print_ast_code(handler->ce_code, printer, arg,
					                  is_expression && !is_scope,
					                  self->a_scope, indent));
				}
			} else {
				struct symbol *except_symbol = NULL;
				size_t j;
				DeeScopeObject *handler_scope = handler->ce_code->a_scope;
				if (handler_scope->s_mapc) {
					for (j = 0; j < handler_scope->s_mapa; ++j) {
						except_symbol = handler_scope->s_map[j];
						for (; except_symbol; except_symbol = except_symbol->s_next) {
							if (except_symbol->s_type == SYMBOL_TYPE_EXCEPT)
								goto got_except_symbol;
						}
					}
					except_symbol = NULL;
				}
got_except_symbol:
				if (handler->ce_flags & EXCEPTION_HANDLER_FINTERPT)
					PRINT("@[interrupt] ");
				PRINT(" catch (");
				if (handler->ce_mask) {
					DO(print_ast_code(handler->ce_mask, printer, arg, true, self->a_scope, indent));
					if (except_symbol) {
						PRINT(" ");
						print(except_symbol->s_name->k_name,
						      except_symbol->s_name->k_size);
					}
				} else {
					print(except_symbol->s_name->k_name,
					      except_symbol->s_name->k_size);
					PRINT("...");
				}
				PRINT(") ");
				DO(print_ast_code(handler->ce_code, printer, arg,
				                  is_expression && !is_scope,
				                  self->a_scope, indent));
			}
		}
	}	break;

	case AST_LOOP:
		if (!is_scope && is_expression)
			goto force_scope;
		if (self->a_flag & AST_FLOOP_UNLIKELY)
			PRINT("@unlikely ");
		if (self->a_flag & AST_FLOOP_FOREACH) {
			PRINT("foreach (");
			need_semicolon = false;
			if (self->a_loop.l_elem) {
				ENTER_SCOPE(inner_is_scope, self->a_scope, self->a_loop.l_elem->a_scope, false);
				DO(print_ast_code(self->a_loop.l_elem, printer, arg, true, self->a_loop.l_elem->a_scope, indent));
				PRINT(": ");
				DO(print_ast_code(self->a_loop.l_iter, printer, arg, true, self->a_loop.l_elem->a_scope, indent));
				PRINT(") ");
				if (self->a_loop.l_loop) {
					DO(print_ast_code(self->a_loop.l_loop, printer, arg, false, self->a_loop.l_elem->a_scope, indent));
				} else {
					PRINT("none;");
				}
				LEAVE_SCOPE(inner_is_scope, false, false);
			} else {
				PRINT("none: ");
				DO(print_ast_code(self->a_loop.l_iter, printer, arg, true, self->a_scope, indent));
				PRINT(") ");
				if (self->a_loop.l_loop) {
					DO(print_ast_code(self->a_loop.l_loop, printer, arg, false, self->a_scope, indent));
				} else {
					PRINT("none;");
				}
			}
		} else if (self->a_flag & AST_FLOOP_POSTCOND) {
			PRINT("do ");
			if (self->a_loop.l_next && self->a_loop.l_loop) {
				PRINT("{\n");
				DO(DeeFormat_Repeat(printer, arg, '\t', indent + 1));
				DO(print_ast_code(self->a_loop.l_loop, printer, arg, false,
				                  self->a_scope, indent + 1));
				PRINT(";\n");
				DO(DeeFormat_Repeat(printer, arg, '\t', indent + 1));
				DO(print_ast_code(self->a_loop.l_next, printer, arg, false,
				                  self->a_scope, indent + 1));
				PRINT(";\n");
				DO(DeeFormat_Repeat(printer, arg, '\t', indent));
				PRINT("}");
			} else if (self->a_loop.l_next) {
				DO(print_ast_code(self->a_loop.l_next, printer, arg, false, self->a_scope, indent));
			} else if (self->a_loop.l_loop) {
				DO(print_ast_code(self->a_loop.l_loop, printer, arg, false, self->a_scope, indent));
			} else {
				PRINT("{ }");
			}
			PRINT(" while (");
			if (self->a_loop.l_cond) {
				DO(print_ast_code(self->a_loop.l_cond, printer, arg, true, self->a_scope, indent));
			} else {
				PRINT_TRUE();
			}
			PRINT(")");
		} else if (!self->a_loop.l_next) {
			need_semicolon = false;
			PRINT("while (");
			if (self->a_loop.l_cond) {
				DO(print_ast_code(self->a_loop.l_cond, printer, arg, true, self->a_scope, indent));
			} else {
				PRINT_TRUE();
			}
			PRINT(") ");
			if (self->a_loop.l_loop) {
				DO(print_ast_code(self->a_loop.l_loop, printer, arg, false, self->a_scope, indent));
			} else {
				PRINT("{ }");
			}
		} else {
			need_semicolon = false;
			PRINT("for (; ");
			if (self->a_loop.l_cond) {
				DO(print_ast_code(self->a_loop.l_cond, printer, arg, true, self->a_scope, indent));
			}
			PRINT("; ");
			DO(print_ast_code(self->a_loop.l_next, printer, arg, true, self->a_scope, indent));
			PRINT(") ");
			if (self->a_loop.l_loop) {
				DO(print_ast_code(self->a_loop.l_loop, printer, arg, false, self->a_scope, indent));
			} else {
				PRINT("{ }");
			}
		}
		break;

	case AST_LOOPCTL:
		if (self->a_flag & AST_FLOOPCTL_CON) {
			PRINT("continue");
		} else {
			PRINT("break");
		}
		break;

	case AST_CONDITIONAL:
		if (is_expression ||
		    self->a_conditional.c_tt == self->a_conditional.c_cond ||
		    self->a_conditional.c_ff == self->a_conditional.c_cond) {
			/* Special handling for logical and / or. */
			if (self->a_flag & AST_FCOND_BOOL) {
				if (self->a_conditional.c_tt == self->a_conditional.c_cond &&
				    self->a_conditional.c_ff != self->a_conditional.c_cond) {
					/* c_cond || c_ff */
					PRINT("(");
					DO(print_ast_code(self->a_conditional.c_cond, printer, arg, true, self->a_scope, indent));
					PRINT(" || ");
					DO(print_ast_code(self->a_conditional.c_ff, printer, arg, true, self->a_scope, indent));
					PRINT(")");
					break;
				}
				if (self->a_conditional.c_ff == self->a_conditional.c_cond &&
				    self->a_conditional.c_tt != self->a_conditional.c_cond) {
					/* c_cond && c_tt */
					PRINT("(");
					DO(print_ast_code(self->a_conditional.c_cond, printer, arg, true, self->a_scope, indent));
					PRINT(" && ");
					DO(print_ast_code(self->a_conditional.c_tt, printer, arg, true, self->a_scope, indent));
					PRINT(")");
					break;
				}
			}
			PRINT("(");
			if (self->a_flag & AST_FCOND_BOOL)
				PRINT("!!(");
			DO(print_ast_code(self->a_conditional.c_cond, printer, arg, true, self->a_scope, indent));
			if (self->a_flag & AST_FCOND_BOOL)
				PRINT(")");
			PRINT(" ? ");
			if (self->a_conditional.c_tt == self->a_conditional.c_cond) {
			} else if (!self->a_conditional.c_tt) {
				PRINT_NONE();
			} else {
				if (self->a_flag & AST_FCOND_BOOL)
					PRINT("!!(");
				DO(print_ast_code(self->a_conditional.c_tt, printer, arg, true, self->a_scope, indent));
				if (self->a_flag & AST_FCOND_BOOL)
					PRINT(")");
			}
			PRINT(" : ");
			if (self->a_conditional.c_ff == self->a_conditional.c_cond) {
			} else if (!self->a_conditional.c_ff) {
				PRINT_NONE();
			} else {
				if (self->a_flag & AST_FCOND_BOOL)
					PRINT("!!(");
				DO(print_ast_code(self->a_conditional.c_ff, printer, arg, true, self->a_scope, indent));
				if (self->a_flag & AST_FCOND_BOOL)
					PRINT(")");
			}
			PRINT(")");
		} else {
			need_semicolon = false;
			PRINT("if (");
			if (self->a_flag & AST_FCOND_BOOL)
				PRINT("!!(");
			DO(print_ast_code(self->a_conditional.c_cond, printer, arg, true, self->a_scope, indent));
			if (self->a_flag & AST_FCOND_BOOL)
				PRINT(")");
			PRINT(") ");
			if (self->a_conditional.c_tt) {
				DO(print_ast_code(self->a_conditional.c_tt, printer, arg, false, self->a_scope, indent));
			} else {
				PRINT("{ }");
			}
			if (self->a_conditional.c_ff) {
				PRINT(" else ");
				DO(print_ast_code(self->a_conditional.c_ff, printer, arg, false, self->a_scope, indent));
			}
		}
		break;

	case AST_BOOL:
		print("!!", self->a_flag & AST_FBOOL_NEGATE ? 1 : 2);
		PRINT("(");
		DO(print_ast_code(self->a_bool, printer, arg, true, self->a_scope, indent));
		PRINT(")");
		break;

	case AST_EXPAND:
		DO(print_ast_code(self->a_expand, printer, arg, true, self->a_scope, indent));
		PRINT("...");
		break;

	case AST_FUNCTION:
		PRINT("[]");
		DO(print_function_atargs(self, printer, arg, indent, true));
		break;

	case AST_OPERATOR_FUNC: {
		struct opinfo const *info;
		info = Dee_OperatorInfo(NULL, self->a_flag);
		if (!info)
			PRINT("(");
		if (self->a_operator_func.of_binding) {
			DO(print_ast_code(self->a_operator_func.of_binding, printer, arg, true, self->a_scope, indent));
			PRINT(".");
		}
		PRINT("operator ");
		if (info) {
			printf("%s", info->oi_sname);
		} else {
			printf("%" PRFu16, self->a_flag);
			PRINT(")");
		}
	}	break;

	case AST_OPERATOR: {
		struct opinfo const *info;
		char const *name;
		if (self->a_operator.o_exflag & AST_OPERATOR_FVARARGS)
			goto operator_fallback;
		if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
			switch (self->a_flag) {
			case OPERATOR_INC:
				DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
				PRINT("++");
				goto done;
			case OPERATOR_DEC:
				DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
				PRINT("--");
				goto done;
			default: break;
			}
			goto operator_fallback;
		}
		switch (self->a_flag) {

		case OPERATOR_COPY:
			PRINT("copy(");
do_unary_operator:
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		case OPERATOR_DEEPCOPY:
			PRINT("deepcopy(");
			goto do_unary_operator;

		case OPERATOR_STR:
			PRINT("str(");
			goto do_unary_operator;

		case OPERATOR_REPR:
			PRINT("repr(");
			goto do_unary_operator;

		case OPERATOR_ASSIGN:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			PRINT("(");
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT(" := ");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		case OPERATOR_CALL:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			if (self->a_operator.o_op1->a_type == AST_MULTIPLE &&
			    self->a_operator.o_op1->a_flag == AST_FMULTIPLE_TUPLE &&
			    self->a_operator.o_op1->a_scope == self->a_scope) {
				struct ast *args = self->a_operator.o_op1;
				size_t i;
				PRINT("(");
				for (i = 0; i < args->a_multiple.m_astc; ++i) {
					if (i != 0)
						PRINT(", ");
					DO(print_ast_code(args->a_multiple.m_astv[i],
					                  printer, arg, true,
					                  self->a_scope, indent));
				}
				PRINT(")");
			} else if (self->a_operator.o_op1->a_type == AST_CONSTEXPR &&
			           DeeTuple_Check(self->a_operator.o_op1->a_constexpr) &&
			           self->a_operator.o_op1->a_scope == self->a_scope) {
				DeeObject *args = self->a_operator.o_op1->a_constexpr;
				size_t i;
				PRINT("(");
				for (i = 0; i < DeeTuple_SIZE(args); ++i) {
					if (i != 0)
						PRINT(", ");
					DO(DeeObject_PrintRepr(DeeTuple_GET(args, i), printer, arg));
				}
				PRINT(")");
			} else {
				PRINT("((");
				DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
				PRINT(")...)");
			}
			break;

		case OPERATOR_INV:
			PRINT("~(");
			goto do_unary_operator;

		case OPERATOR_POS:
			PRINT("+(");
			goto do_unary_operator;

		case OPERATOR_INC:
			PRINT("++(");
			goto do_unary_operator;

		case OPERATOR_DEC:
			PRINT("--(");
			goto do_unary_operator;

		case OPERATOR_NEG:
			PRINT("-(");
			goto do_unary_operator;

		case OPERATOR_SIZE:
			PRINT("#(");
			goto do_unary_operator;

		case OPERATOR_ADD:
			name = "+";
do_binary:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			PRINT("(");
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			printf(" %s ", name);
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		case OPERATOR_SUB:
			name = "-";
			goto do_binary;

		case OPERATOR_MUL:
			name = "*";
			goto do_binary;

		case OPERATOR_DIV:
			name = "/";
			goto do_binary;

		case OPERATOR_MOD:
			name = "%";
			goto do_binary;

		case OPERATOR_SHL:
			name = "<<";
			goto do_binary;

		case OPERATOR_SHR:
			name = ">>";
			goto do_binary;

		case OPERATOR_AND:
			name = "&";
			goto do_binary;

		case OPERATOR_OR:
			name = "|";
			goto do_binary;

		case OPERATOR_XOR:
			name = "^";
			goto do_binary;

		case OPERATOR_POW:
			name = "**";
			goto do_binary;

		case OPERATOR_INPLACE_ADD:
			name = "+=";
			goto do_binary;

		case OPERATOR_INPLACE_SUB:
			name = "-=";
			goto do_binary;

		case OPERATOR_INPLACE_MUL:
			name = "*=";
			goto do_binary;

		case OPERATOR_INPLACE_DIV:
			name = "/=";
			goto do_binary;

		case OPERATOR_INPLACE_MOD:
			name = "%=";
			goto do_binary;

		case OPERATOR_INPLACE_SHL:
			name = "<<=";
			goto do_binary;

		case OPERATOR_INPLACE_SHR:
			name = ">>=";
			goto do_binary;

		case OPERATOR_INPLACE_AND:
			name = "&=";
			goto do_binary;

		case OPERATOR_INPLACE_OR:
			name = "|=";
			goto do_binary;

		case OPERATOR_INPLACE_XOR:
			name = "^=";
			goto do_binary;

		case OPERATOR_INPLACE_POW:
			name = "**=";
			goto do_binary;

		case OPERATOR_EQ:
			name = "==";
			goto do_binary;

		case OPERATOR_NE:
			name = "!=";
			goto do_binary;

		case OPERATOR_LO:
			name = "<";
			goto do_binary;

		case OPERATOR_LE:
			name = "<=";
			goto do_binary;

		case OPERATOR_GR:
			name = ">";
			goto do_binary;

		case OPERATOR_GE:
			name = ">=";
			goto do_binary;

		case OPERATOR_GETITEM:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT("[");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT("]");
			break;

		case OPERATOR_DELITEM:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			PRINT("del(");
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT("[");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT("])");
			break;

		case OPERATOR_SETITEM:
			if (!self->a_operator.o_op2)
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT("[");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT("] = ");
			DO(print_ast_code(self->a_operator.o_op2, printer, arg, true, self->a_scope, indent));
			break;

		case OPERATOR_GETRANGE:
			if (!self->a_operator.o_op2)
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT("[");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT(":");
			DO(print_ast_code(self->a_operator.o_op2, printer, arg, true, self->a_scope, indent));
			PRINT("]");
			break;

		case OPERATOR_DELRANGE:
			if (!self->a_operator.o_op2)
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT("del([");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT(":");
			DO(print_ast_code(self->a_operator.o_op2, printer, arg, true, self->a_scope, indent));
			PRINT("])");
			break;

		case OPERATOR_SETRANGE:
			if (!self->a_operator.o_op3)
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT("[");
			DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
			PRINT(":");
			DO(print_ast_code(self->a_operator.o_op2, printer, arg, true, self->a_scope, indent));
			PRINT("] = ");
			DO(print_ast_code(self->a_operator.o_op3, printer, arg, true, self->a_scope, indent));
			break;

		case OPERATOR_GETATTR:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR ||
			    !DeeString_Check(self->a_operator.o_op1->a_constexpr) ||
			    !DeeString_IsSymbol((DeeStringObject *)self->a_operator.o_op1->a_constexpr, 0, (size_t)-1))
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT(".");
			DO(DeeObject_Print(self->a_operator.o_op1->a_constexpr, printer, arg));
			break;

		case OPERATOR_DELATTR:
			if (!self->a_operator.o_op1)
				goto operator_fallback;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR ||
			    !DeeString_Check(self->a_operator.o_op1->a_constexpr) ||
			    !DeeString_IsSymbol((DeeStringObject *)self->a_operator.o_op1->a_constexpr, 0, (size_t)-1))
				goto operator_fallback;
			PRINT("del(");
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT(".");
			DO(DeeObject_Print(self->a_operator.o_op1->a_constexpr, printer, arg));
			PRINT(")");
			break;

		case OPERATOR_SETATTR:
			if (!self->a_operator.o_op2)
				goto operator_fallback;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR ||
			    !DeeString_Check(self->a_operator.o_op1->a_constexpr) ||
			    !DeeString_IsSymbol((DeeStringObject *)self->a_operator.o_op1->a_constexpr, 0, (size_t)-1))
				goto operator_fallback;
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT(".");
			DO(DeeObject_Print(self->a_operator.o_op1->a_constexpr, printer, arg));
			PRINT(" = ");
			DO(print_ast_code(self->a_operator.o_op2, printer, arg, true, self->a_scope, indent));
			break;

		default:
operator_fallback:
			info = Dee_OperatorInfo(NULL, self->a_flag);
			/* TODO: if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP); */
			if (!info)
				PRINT("(");
			DO(print_ast_code(self->a_operator.o_op0, printer, arg, true, self->a_scope, indent));
			PRINT(".operator ");
			if (info) {
				printf("%s", info->oi_sname);
			} else {
				printf("%" PRFu16 " ", self->a_flag);
			}
			PRINT("(");
			if (self->a_operator.o_op1) {
				DO(print_ast_code(self->a_operator.o_op1, printer, arg, true, self->a_scope, indent));
				if (self->a_operator.o_op2) {
					PRINT(", ");
					DO(print_ast_code(self->a_operator.o_op2, printer, arg, true, self->a_scope, indent));
					if (self->a_operator.o_op3) {
						PRINT(", ");
						DO(print_ast_code(self->a_operator.o_op3, printer, arg, true, self->a_scope, indent));
					}
				}
			}
			PRINT(")");
			if (!info)
				PRINT(")");
			break;
		}
	}	break;

	case AST_ACTION:
		switch (self->a_flag & AST_FACTION_KINDMASK) {
#define ACTION(x) case ((x)&AST_FACTION_KINDMASK)

		ACTION(AST_FACTION_CELL0):
			PRINT("<>");
			break;

		ACTION(AST_FACTION_CELL1):
			PRINT("<");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(">");
			break;

		ACTION(AST_FACTION_TYPEOF):
			PRINT("type(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_CLASSOF):
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(".class");
			break;

		ACTION(AST_FACTION_SUPEROF):
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(".super");
			break;

		ACTION(AST_FACTION_PRINT):
			if (!is_scope && is_expression) goto force_scope;
			PRINT("print ");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT("...,");
			break;

		ACTION(AST_FACTION_PRINTLN):
			if (!is_scope && is_expression) goto force_scope;
			PRINT("print ");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT("...");
			break;

		ACTION(AST_FACTION_FPRINT):
			if (!is_scope && is_expression) goto force_scope;
			PRINT("print ");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(": ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT("...,");
			break;

		ACTION(AST_FACTION_FPRINTLN):
			if (!is_scope && is_expression) goto force_scope;
			PRINT("print ");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(": ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT("...");
			break;

		ACTION(AST_FACTION_RANGE):
			PRINT("[");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(":");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(", ");
			DO(print_ast_code(self->a_action.a_act2, printer, arg, true, self->a_scope, indent));
			PRINT("]");
			break;

		ACTION(AST_FACTION_IS):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" is ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_IN):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" in ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_AS):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" as ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_MIN):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" < ...)");
			break;

		ACTION(AST_FACTION_MAX):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" > ...)");
			break;

		ACTION(AST_FACTION_SUM):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" + ...)");
			break;

		ACTION(AST_FACTION_ANY):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" || ...)");
			break;

		ACTION(AST_FACTION_ALL):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" && ...)");
			break;

		ACTION(AST_FACTION_STORE):
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" = ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			break;

		ACTION(AST_FACTION_ASSERT):
			PRINT("assert(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_ASSERT_M):
			PRINT("assert(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(", ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_BOUNDATTR):
			if (self->a_action.a_act1->a_type != AST_CONSTEXPR ||
			          !DeeString_Check(self->a_action.a_act1->a_constexpr) ||
			          !DeeString_IsSymbol((DeeStringObject *)self->a_action.a_act1->a_constexpr, 0, (size_t)-1)) {
				PRINT("bound(");
				DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
				PRINT(".operator . (");
				DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
				PRINT("))");
			} else {
				PRINT("(");
				DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
				PRINT(".");
				DO(DeeObject_Print(self->a_action.a_act1->a_constexpr, printer, arg));
				PRINT(" is bound)");
			}
			break;

		ACTION(AST_FACTION_BOUNDITEM):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT("[");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT("] is bound)");
			break;

		ACTION(AST_FACTION_SAMEOBJ):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" === ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_DIFFOBJ):
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT(" !== ");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;

		ACTION(AST_FACTION_CALL_KW):
			DO(print_ast_code(self->a_action.a_act0, printer, arg, true, self->a_scope, indent));
			PRINT("(");
			DO(print_ast_code(self->a_action.a_act1, printer, arg, true, self->a_scope, indent));
			PRINT("..., **");
			DO(print_ast_code(self->a_action.a_act2, printer, arg, true, self->a_scope, indent));
			PRINT(")");
			break;
#undef ACTION

		default:
			printf("/* unknown action %#I16x */", self->a_flag);
			break;
		}
		break;

	case AST_CLASS: {
		size_t i;
		DeeClassDescriptorObject *desc;
		if (!is_scope && is_expression)
			goto force_scope;
		need_semicolon = false;
		if (self->a_class.c_desc->a_type != AST_CONSTEXPR ||
		    !DeeClassDescriptor_Check(self->a_class.c_desc->a_constexpr)) {
			PRINT("class <");
			DO(print_ast_code(self->a_class.c_desc, printer, arg, true, self->a_scope, indent));
			PRINT(">");
			if (self->a_class.c_base) {
				PRINT(": ");
				DO(print_ast_code(self->a_class.c_base, printer, arg, true, self->a_scope, indent));
			}
			PRINT(" {\n");
			++indent;
			for (i = 0; i < self->a_class.c_memberc; ++i) {
				DO(DeeFormat_Repeat(printer, arg, '\t', indent - 1));
				printf("<member(%" PRFu16 ")> = ", self->a_class.c_memberv[i].cm_index);
				DO(print_ast_code(self->a_class.c_memberv[i].cm_ast, printer, arg, true, self->a_scope, indent));
				PRINT("\n");
			}
			DO(DeeFormat_Repeat(printer, arg, '\t', indent - 1));
			PRINT("}");
			break;
		}
		desc = (DeeClassDescriptorObject *)self->a_class.c_desc->a_constexpr;
		if (desc->cd_doc) {
			PRINT("@");
			DO(DeeObject_PrintRepr((DeeObject *)desc->cd_doc, printer, arg));
			PRINT("\n");
		}
		PRINT("class ");
		if (desc->cd_name) {
			PRINT(" ");
			DO(DeeObject_Print((DeeObject *)desc->cd_name, printer, arg));
		}
		if (self->a_class.c_base) {
			PRINT(": ");
			DO(print_ast_code(self->a_class.c_base, printer, arg, true, self->a_scope, indent));
		}
		PRINT(" {\n");
		++indent;
		/* Print the contents of the instance member table. */
		for (i = 0; i <= desc->cd_iattr_mask; ++i) {
			struct class_attribute *attr;
			attr = &desc->cd_iattr_list[i];
			if (!attr->ca_name)
				continue;
			if (attr->ca_doc) {
				DO(DeeFormat_Repeat(printer, arg, '\t', indent));
				PRINT("@");
				DO(DeeObject_PrintRepr((DeeObject *)attr->ca_doc, printer, arg));
				PRINT("\n");
			}
			DO(DeeFormat_Repeat(printer, arg, '\t', indent));
			if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
				PRINT("private ");
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
				if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
					PRINT("@readonly ");
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
					PRINT("@method ");
				printf("member %k;\n", attr->ca_name);
			} else if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				struct class_member *functions[3];
				size_t j;
				/* Instance-property (with its callbacks saved as part of the class) */
				functions[1] = functions[2] = NULL;
				functions[0]                = find_class_member(self, attr->ca_addr + CLASS_GETSET_GET);
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
					functions[1] = find_class_member(self, attr->ca_addr + CLASS_GETSET_DEL);
					functions[2] = find_class_member(self, attr->ca_addr + CLASS_GETSET_SET);
				}
				printf("property %k = {\n", attr->ca_name);
				++indent;
				for (j = 0; j < 3; ++j) {
					if (!functions[j])
						continue;
					DO(DeeFormat_Repeat(printer, arg, '\t', indent));
					print(property_names[j], 3);
					if (functions[j]->cm_ast->a_type == AST_FUNCTION &&
					    is_instance_method(functions[j]->cm_ast->a_function.f_scope)) {
						DO(print_function_atargs(functions[j]->cm_ast, printer, arg, indent, false));
					} else {
						PRINT(" = ");
						DO(print_ast_code(functions[j]->cm_ast, printer, arg, true, self->a_scope, indent));
						PRINT(";");
					}
					PRINT("\n");
				}
				--indent;
				DO(DeeFormat_Repeat(printer, arg, '\t', indent));
				PRINT("}\n");
			} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				struct class_member *method;
				/* Instance-method (that is saved within the class) */
				method = find_class_member(self, attr->ca_addr);
				if unlikely(!method)
					goto instance_member_in_class;
				if unlikely(method->cm_ast->a_type != AST_FUNCTION)
					goto instance_member_in_class;
				if (!is_instance_method(method->cm_ast->a_function.f_scope))
					goto instance_member_in_class;
				printf("function %k", attr->ca_name);
				DO(print_function_atargs(method->cm_ast, printer, arg, indent, false));
				PRINT("\n");
			} else {
				struct class_member *member;
				/* An instance-member that is saved within the class??? */
instance_member_in_class:
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
					PRINT("@method ");
				if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
					PRINT("@readonly ");
				printf("<instance-memory-in-class-table %k", attr->ca_name);
				member = find_class_member(self, attr->ca_addr);
				if (member) {
					PRINT(" = ");
					DO(print_ast_code(member->cm_ast, printer, arg, true, self->a_scope, indent));
				}
				PRINT(">\n");
			}
		}
		/* Print the contents of the class member table. */
		for (i = 0; i <= desc->cd_cattr_mask; ++i) {
			struct class_attribute *attr;
			attr = &desc->cd_cattr_list[i];
			if (!attr->ca_name)
				continue;
			if (attr->ca_doc) {
				DO(DeeFormat_Repeat(printer, arg, '\t', indent));
				PRINT("@");
				DO(DeeObject_PrintRepr((DeeObject *)attr->ca_doc, printer, arg));
				PRINT("\n");
			}
			DO(DeeFormat_Repeat(printer, arg, '\t', indent));
			if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
				PRINT("private ");
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				struct class_member *functions[3];
				size_t j;
				/* Instance-property (with its callbacks saved as part of the class) */
				functions[1] = functions[2] = NULL;
				functions[0]                = find_class_member(self, attr->ca_addr + CLASS_GETSET_GET);
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
					functions[1] = find_class_member(self, attr->ca_addr + CLASS_GETSET_DEL);
					functions[2] = find_class_member(self, attr->ca_addr + CLASS_GETSET_SET);
				}
				printf("class property %k = {\n", attr->ca_name);
				++indent;
				for (j = 0; j < 3; ++j) {
					if (!functions[j])
						continue;
					DO(DeeFormat_Repeat(printer, arg, '\t', indent));
					print(property_names[j], 3);
					if (functions[j]->cm_ast->a_type == AST_FUNCTION) {
						DO(print_function_atargs(functions[j]->cm_ast, printer, arg, indent, true));
					} else {
						PRINT(" = ");
						DO(print_ast_code(functions[j]->cm_ast, printer, arg, true, self->a_scope, indent));
						PRINT(";");
					}
					PRINT("\n");
				}
				--indent;
				DO(DeeFormat_Repeat(printer, arg, '\t', indent));
				PRINT("}\n");
			} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				struct class_member *method;
				/* Instance-method (that is saved within the class) */
				method = find_class_member(self, attr->ca_addr);
				if unlikely(!method)
					goto class_member_in_class;
				if unlikely(method->cm_ast->a_type != AST_FUNCTION)
					goto class_member_in_class;
				printf("class function %k", attr->ca_name);
				DO(print_function_atargs(method->cm_ast, printer, arg, indent, true));
				PRINT("\n");
			} else {
				struct class_member *member;
				/* An instance-member that is saved within the class??? */
class_member_in_class:
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
					PRINT("@method ");
				if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
					PRINT("@readonly ");
				printf("class member %k", attr->ca_name);
				member = find_class_member(self, attr->ca_addr);
				if (member) {
					PRINT(" = ");
					DO(print_ast_code(member->cm_ast, printer, arg, true, self->a_scope, indent));
				}
				PRINT(";\n");
			}
		}

		/* Print class operators. */
		for (i = 0; i <= desc->cd_clsop_mask; ++i) {
			struct class_member *member;
			struct opinfo const *info;
			struct class_operator *op = &desc->cd_clsop_list[i];
			if (op->co_name == (uint16_t)-1)
				continue;
			member = find_class_member(self, op->co_addr);
			DO(DeeFormat_Repeat(printer, arg, '\t', indent));
			if (op->co_name == OPERATOR_CONSTRUCTOR &&
			    member && member->cm_ast->a_type == AST_FUNCTION) {
				print(STR_this, 4);
			} else if (op->co_name == OPERATOR_DESTRUCTOR &&
			           member && member->cm_ast->a_type == AST_FUNCTION) {
				PRINT("~this");
			} else {
				PRINT("operator ");
				info = Dee_OperatorInfo(NULL, op->co_name);
				if (info) {
					printf("%s", info->oi_sname);
				} else {
					printf("%" PRFu16, op->co_name);
				}
			}
			if (member) {
				if (member->cm_ast->a_type == AST_FUNCTION &&
				    is_instance_method(member->cm_ast->a_function.f_scope)) {
					DO(print_function_atargs(member->cm_ast, printer, arg, indent, false));
				} else {
					PRINT(" = ");
					DO(print_ast_code(member->cm_ast, printer, arg, true, self->a_scope, indent));
					PRINT(";");
				}
			} else {
				PRINT(";");
			}
			PRINT("\n");
		}
		DO(DeeFormat_Repeat(printer, arg, '\t', indent - 1));
		PRINT("}");
	}	break;

	case AST_LABEL:
		if (self->a_flag & AST_FLABEL_CASE) {
			if (self->a_label.l_label->tl_expr) {
				PRINT("case ");
				DO(print_ast_code(self->a_label.l_label->tl_expr, printer, arg, true, self->a_scope, indent));
			} else {
				PRINT("default");
			}
		} else {
			print(self->a_label.l_label->tl_name->k_name,
			      self->a_label.l_label->tl_name->k_size);
		}
		PRINT(":");
		break;

	case AST_GOTO:
		printf("goto %$s",
		       self->a_goto.g_label->tl_name->k_size,
		       self->a_goto.g_label->tl_name->k_name);
		break;

	case AST_SWITCH:
		if (!is_scope && is_expression)
			goto force_scope;
		PRINT("switch (");
		DO(print_ast_code(self->a_switch.s_expr, printer, arg, true, self->a_scope, indent));
		PRINT(") ");
		DO(print_ast_code(self->a_switch.s_block, printer, arg, false, self->a_scope, indent));
		break;

	case AST_ASSEMBLY:
		if (!is_scope && is_expression)
			goto force_scope;
		PRINT("__asm__");
		if (self->a_flag & AST_FASSEMBLY_VOLATILE)
			PRINT(" __volatile__");
		if (self->a_assembly.as_num_l != 0)
			PRINT(" goto");
		PRINT("(");
#ifdef CONFIG_LANGUAGE_NO_ASM
		PRINT("\"\"");
#else /* CONFIG_LANGUAGE_NO_ASM */
		printf("%$q",
		       self->a_assembly.as_text.at_text->s_size,
		       self->a_assembly.as_text.at_text->s_text);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
		if (self->a_assembly.as_opc ||
		    (self->a_flag & (AST_FASSEMBLY_FORMAT | AST_FASSEMBLY_MEMORY |
		                     AST_FASSEMBLY_CLOBSP | AST_FASSEMBLY_REACH |
		                     AST_FASSEMBLY_NORETURN))) {
			PRINT(" : ");
			if (self->a_assembly.as_num_o) {
				size_t i;
				for (i = 0; i < self->a_assembly.as_num_o; ++i) {
					if (i != 0)
						PRINT(", ");
					DO(print_asm_operator(&self->a_assembly.as_opv[i], printer, arg, self->a_scope, indent));
				}
			}
			if (self->a_assembly.as_num_i ||
			    self->a_assembly.as_num_l ||
			    (self->a_flag & (AST_FASSEMBLY_MEMORY | AST_FASSEMBLY_CLOBSP |
			                     AST_FASSEMBLY_REACH | AST_FASSEMBLY_NORETURN))) {
				if (self->a_assembly.as_num_o) {
					PRINT(" : ");
				} else {
					PRINT(": ");
				}
				if (self->a_assembly.as_num_i) {
					size_t i;
					for (i = 0; i < self->a_assembly.as_num_i; ++i) {
						if (i != 0)
							PRINT(", ");
						DO(print_asm_operator(&self->a_assembly.as_opv[self->a_assembly.as_num_o + i],
						                      printer, arg, self->a_scope, indent));
					}
				}
				if (self->a_assembly.as_num_l ||
				    (self->a_flag & (AST_FASSEMBLY_MEMORY | AST_FASSEMBLY_CLOBSP |
				                     AST_FASSEMBLY_REACH | AST_FASSEMBLY_NORETURN))) {
					bool first_flag = true;
					if (self->a_assembly.as_num_i) {
						PRINT(" : ");
					} else {
						PRINT(": ");
					}
					if (self->a_flag & AST_FASSEMBLY_MEMORY) {
						PRINT("\"memory\"");
						first_flag = false;
					}
					if (self->a_flag & AST_FASSEMBLY_CLOBSP) {
						if (!first_flag)
							PRINT(", ");
						PRINT("\"sp\"");
						first_flag = false;
					}
					if (self->a_flag & AST_FASSEMBLY_REACH) {
						if (!first_flag)
							PRINT(", ");
						PRINT("\"reach\"");
						first_flag = false;
					}
					if (self->a_flag & AST_FASSEMBLY_NORETURN) {
						if (!first_flag)
							PRINT(", ");
						PRINT("\"noreturn\"");
						first_flag = false;
					}
					if (self->a_assembly.as_num_l) {
						if (self->a_flag & (AST_FASSEMBLY_MEMORY | AST_FASSEMBLY_CLOBSP |
						                    AST_FASSEMBLY_REACH | AST_FASSEMBLY_NORETURN)) {
							PRINT(" : ");
						} else {
							PRINT(": ");
						}
						if (self->a_assembly.as_num_l) {
							size_t i;
							for (i = 0; i < self->a_assembly.as_num_l; ++i) {
								if (i != 0)
									PRINT(", ");
								DO(print_asm_label_operator(&self->a_assembly.as_opv[self->a_assembly.as_num_i +
								                                                     self->a_assembly.as_num_o +
								                                                     i],
								                            printer, arg));
							}
						}
					}
				}
			}
		}
		PRINT(")");
		break;


	default:
		printf("/* unknown ast: %" PRFu16 " */", self->a_type);
		break;
	}
done:
	LEAVE_SCOPE(is_scope, is_expression, need_semicolon);
	return result;
err:
	return temp;
}

PRIVATE dssize_t DCALL
print_operator_name(uint16_t opid,
                    dformatprinter printer, void *arg) {
	struct opinfo const *info;
	switch (opid) {

	case AST_OPERATOR_POS_OR_ADD:
		return DeeFormat_PRINT(printer, arg, "\"+\"");

	case AST_OPERATOR_NEG_OR_SUB:
		return DeeFormat_PRINT(printer, arg, "\"-\"");

	case AST_OPERATOR_GETITEM_OR_SETITEM:
		return DeeFormat_PRINT(printer, arg, "\"[]\"");

	case AST_OPERATOR_GETRANGE_OR_SETRANGE:
		return DeeFormat_PRINT(printer, arg, "\"[:]\"");

	case AST_OPERATOR_GETATTR_OR_SETATTR:
		return DeeFormat_PRINT(printer, arg, "\".\"");

	default: break;
	}
	info = Dee_OperatorInfo(NULL, opid);
	if unlikely(!info)
		return DeeFormat_Printf(printer, arg, "%" PRFu16, opid);
	return DeeFormat_Printf(printer, arg, "%q", info->oi_sname);
}


PRIVATE char const action_names[][10] = {
	/* [AST_FACTION_CELL0    & AST_FACTION_KINDMASK] = */ "cell",
	/* [AST_FACTION_CELL1    & AST_FACTION_KINDMASK] = */ "cell",
	/* [AST_FACTION_TYPEOF   & AST_FACTION_KINDMASK] = */ "typeof",
	/* [AST_FACTION_CLASSOF  & AST_FACTION_KINDMASK] = */ "classof",
	/* [AST_FACTION_SUPEROF  & AST_FACTION_KINDMASK] = */ "superof",
	/* [AST_FACTION_PRINT    & AST_FACTION_KINDMASK] = */ "print",
	/* [AST_FACTION_PRINTLN  & AST_FACTION_KINDMASK] = */ "println",
	/* [AST_FACTION_FPRINT   & AST_FACTION_KINDMASK] = */ "fprint",
	/* [AST_FACTION_FPRINTLN & AST_FACTION_KINDMASK] = */ "fprintln",
	/* [AST_FACTION_RANGE    & AST_FACTION_KINDMASK] = */ "range",
	/* [AST_FACTION_IS       & AST_FACTION_KINDMASK] = */ "is",
	/* [AST_FACTION_IN       & AST_FACTION_KINDMASK] = */ "in",
	/* [AST_FACTION_AS       & AST_FACTION_KINDMASK] = */ "as",
	/* [AST_FACTION_MIN      & AST_FACTION_KINDMASK] = */ "min",
	/* [AST_FACTION_MAX      & AST_FACTION_KINDMASK] = */ "max",
	/* [AST_FACTION_SUM      & AST_FACTION_KINDMASK] = */ "sum",
	/* [AST_FACTION_ANY      & AST_FACTION_KINDMASK] = */ "any",
	/* [AST_FACTION_ALL      & AST_FACTION_KINDMASK] = */ "all",
	/* [AST_FACTION_STORE    & AST_FACTION_KINDMASK] = */ "store",
	/* [AST_FACTION_ASSERT   & AST_FACTION_KINDMASK] = */ "assert",
	/* [AST_FACTION_ASSERT_M & AST_FACTION_KINDMASK] = */ "assert",
};

INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_scope_repr(DeeScopeObject *__restrict self,
                 dformatprinter printer, void *arg);


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_ast_repr(struct ast *__restrict self,
               dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
	switch (self->a_type) {

	case AST_CONSTEXPR:
		printf("makeconstexpr(value: %r", self->a_constexpr);
		break;

	case AST_SYM:
		printf("makesym(sym: <symbol %$q>",
		       self->a_sym->s_name->k_size,
		       self->a_sym->s_name->k_name);
		break;

	case AST_UNBIND:
		printf("makeunbind(sym: <symbol %$q>",
		       self->a_sym->s_name->k_size,
		       self->a_sym->s_name->k_name);
		break;

	case AST_BOUND:
		printf("makebound(sym: <symbol %$q>",
		       self->a_sym->s_name->k_size,
		       self->a_sym->s_name->k_name);
		break;

	case AST_MULTIPLE: {
		char *typing;
		size_t i;
		typing = NULL;
		if (self->a_flag == AST_FMULTIPLE_TUPLE) {
			typing = STR_Tuple;
		} else if (self->a_flag == AST_FMULTIPLE_LIST) {
			typing = STR_List;
		} else if (self->a_flag == AST_FMULTIPLE_HASHSET) {
			typing = STR_Set;
		} else if (self->a_flag == AST_FMULTIPLE_DICT) {
			typing = STR_Dict;
		} else if (self->a_flag == AST_FMULTIPLE_GENERIC) {
			typing = STR_Sequence;
		} else if (self->a_flag == AST_FMULTIPLE_GENERIC_KEYS) {
			typing = STR_Mapping;
		}
		printf("makemultiple(branches: { ");
		for (i = 0; i < self->a_multiple.m_astc; ++i) {
			if (i != 0)
				PRINT(", ");
			PRINTAST(self->a_multiple.m_astv[i]);
		}
		PRINT(" }, typing: ");
		if (typing) {
			printf("\"%s\"", typing);
		} else {
			PRINT_NONE();
		}
	}	break;

	case AST_RETURN:
		PRINT("makereturn(expr: ");
print_single_expr:
		if (self->a_return) {
			PRINTAST(self->a_return);
		} else {
			PRINT_NONE();
		}
		break;

	case AST_YIELD:
		PRINT("makeyield(expr: ");
		ASSERT(self->a_yield);
		goto print_single_expr;

	case AST_THROW:
		PRINT("makethrow(expr: ");
		goto print_single_expr;

	case AST_TRY: {
		size_t i;
		PRINT("maketry(guard: ");
		PRINTAST(self->a_try.t_guard);
		PRINT(", handlers: {");
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			bool first_flag = true;
			struct catch_expr *handler;
			handler = &self->a_try.t_catchv[i];
			if (i != 0)
				PRINT(", ");
			PRINT("(\"");
			if (handler->ce_flags & EXCEPTION_HANDLER_FFINALLY) {
				PRINT("finally");
				first_flag = false;
			}
			if (handler->ce_flags & EXCEPTION_HANDLER_FINTERPT) {
				if (!first_flag)
					PRINT(",");
				PRINT("interrupt");
			}
			PRINT("\", ");
			if (handler->ce_mask) {
				PRINTAST(handler->ce_mask);
				PRINT(", ");
			} else {
				PRINT("none, ");
			}
			PRINTAST(handler->ce_code);
			PRINT(")");
		}
		PRINT("}");
	}	break;

	case AST_LOOP: {
		bool first_flag;
		PRINT("makeloop(flags: \"");
		first_flag = true;
		if (self->a_flag & AST_FLOOP_FOREACH) {
			PRINT("foreach");
			first_flag = false;
		}
		if (self->a_flag & AST_FLOOP_POSTCOND) {
			if (!first_flag)
				PRINT(",");
			PRINT("postcond");
			first_flag = false;
		}
		if (self->a_flag & AST_FLOOP_UNLIKELY) {
			if (!first_flag)
				PRINT(",");
			PRINT("unlikely");
		}
		PRINT("\", ");
		if (self->a_flag & AST_FLOOP_FOREACH) {
			PRINT("\", elem: ");
			if (self->a_loop.l_elem) {
				PRINTAST(self->a_loop.l_elem);
			} else {
				PRINT_NONE();
			}
			PRINT(", iter: ");
			PRINTAST(self->a_loop.l_iter);
		} else {
			PRINT("\", cond: ");
			if (self->a_loop.l_cond) {
				PRINTAST(self->a_loop.l_cond);
			} else {
				PRINT_NONE();
			}
			PRINT(", next: ");
			if (self->a_loop.l_next) {
				PRINTAST(self->a_loop.l_next);
			} else {
				PRINT_NONE();
			}
		}
		PRINT(", loop: ");
		if (self->a_loop.l_loop) {
			PRINTAST(self->a_loop.l_loop);
		} else {
			PRINT_NONE();
		}
	}	break;

	case AST_LOOPCTL:
		PRINT("makeloopctl(isbreak: ");
		if (self->a_flag & AST_FLOOPCTL_CON) {
			PRINT_FALSE();
		} else {
			PRINT_TRUE();
		}
		break;

	case AST_CONDITIONAL: {
		bool first_flag;
		PRINT("makeconditional(cond: ");
		PRINTAST(self->a_conditional.c_cond);
		PRINT(", tt: ");
		if (!self->a_conditional.c_tt) {
			PRINT_NONE();
		} else if (self->a_conditional.c_tt == self->a_conditional.c_cond) {
			PRINT("<cond>");
		} else {
			PRINTAST(self->a_conditional.c_tt);
		}
		PRINT(", ff: ");
		if (!self->a_conditional.c_ff) {
			PRINT_NONE();
		} else if (self->a_conditional.c_ff == self->a_conditional.c_cond) {
			PRINT("<cond>");
		} else {
			PRINTAST(self->a_conditional.c_ff);
		}
		first_flag = true;
		PRINT(", flags: \"");
		if (self->a_flag & AST_FCOND_BOOL) {
			print(STR_bool, 4);
			first_flag = false;
		}
		if (self->a_flag & AST_FCOND_LIKELY) {
			if (first_flag)
				PRINT(",");
			PRINT("likely");
			first_flag = false;
		}
		if (self->a_flag & AST_FCOND_UNLIKELY) {
			if (first_flag)
				PRINT(",");
			PRINT("unlikely");
			first_flag = false;
		}
		PRINT("\"");
	}	break;

	case AST_BOOL:
		PRINT("makebool(expr: ");
		PRINTAST(self->a_bool);
		PRINT(", negate: ");
		if (self->a_flag & AST_FBOOL_NEGATE) {
			PRINT_TRUE();
		} else {
			PRINT_FALSE();
		}
		break;

	case AST_EXPAND:
		PRINT("makeexpand(expr: ");
		goto print_single_expr;

	case AST_FUNCTION:
		PRINT("makefunction(code: ");
		goto print_single_expr;

	case AST_OPERATOR_FUNC:
		PRINT("makeoperatorfunc(name: ");
		DO(print_operator_name(self->a_flag, printer, arg));
		PRINT(", binding: ");
		if (self->a_operator_func.of_binding) {
			PRINTAST(self->a_operator_func.of_binding);
		} else {
			PRINT_NONE();
		}
		break;

	case AST_OPERATOR: {
		bool first_flag;
		PRINT("makeoperator(name: ");
		DO(print_operator_name(self->a_flag, printer, arg));
		PRINT(", a: ");
		if (self->a_operator.o_op0) {
			PRINTAST(self->a_operator.o_op0);
		} else {
			PRINT_NONE();
		}
		PRINT(", b: ");
		if (self->a_operator.o_op1) {
			PRINTAST(self->a_operator.o_op1);
		} else {
			PRINT_NONE();
		}
		PRINT(", c: ");
		if (self->a_operator.o_op2) {
			PRINTAST(self->a_operator.o_op2);
		} else {
			PRINT_NONE();
		}
		PRINT(", d: ");
		if (self->a_operator.o_op3) {
			PRINTAST(self->a_operator.o_op3);
		} else {
			PRINT_NONE();
		}
		first_flag = true;
		PRINT(", flags: \"");
		if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
			PRINT("post");
			first_flag = false;
		}
		if (self->a_operator.o_exflag & AST_OPERATOR_FVARARGS) {
			if (!first_flag)
				PRINT(",");
			PRINT("varargs");
			first_flag = false;
		}
		if (self->a_operator.o_exflag & AST_OPERATOR_FMAYBEPFX) {
			if (!first_flag)
				PRINT(",");
			PRINT("maybeprefix");
			first_flag = false;
		}
		if (self->a_operator.o_exflag & AST_OPERATOR_FDONTOPT) {
			if (!first_flag)
				PRINT(",");
			PRINT("dontoptimize");
			first_flag = false;
		}
		PRINT("\"");
	}	break;

	case AST_ACTION:
		PRINT("makeaction(name: ");
		if ((self->a_flag & AST_FACTION_KINDMASK) < COMPILER_LENOF(action_names)) {
			printf("%q", action_names[self->a_flag & AST_FACTION_KINDMASK]);
		} else {
			printf("\"unknown:%" PRFu16 "\"", self->a_flag & AST_FACTION_KINDMASK);
		}
		PRINT(", a: ");
		if (AST_FACTION_ARGC_GT(self->a_flag) >= 1) {
			PRINTAST(self->a_action.a_act0);
		} else {
			PRINT_NONE();
		}
		PRINT(", b: ");
		if (AST_FACTION_ARGC_GT(self->a_flag) >= 2) {
			PRINTAST(self->a_action.a_act1);
		} else {
			PRINT_NONE();
		}
		PRINT(", c: ");
		if (AST_FACTION_ARGC_GT(self->a_flag) >= 3) {
			PRINTAST(self->a_action.a_act2);
		} else {
			PRINT_NONE();
		}
		PRINT(", mustrun: ");
		if (self->a_flag & AST_FACTION_MAYBERUN) {
			PRINT_FALSE();
		} else {
			PRINT_TRUE();
		}
		break;

	/* case AST_CLASS:    // TODO */
	/* case AST_LABEL:    // TODO */
	/* case AST_GOTO:     // TODO */
	/* case AST_SWITCH:   // TODO */
	/* case AST_ASSEMBLY: // TODO */

	default:
		printf("<ast(type: %" PRFu16 ")>", self->a_type);
		break;
	}
	PRINT(", scope: ");
	DO(print_scope_repr(self->a_scope, printer, arg));
	if (self->a_ddi.l_file) {
		PRINT(", loc: (");
		printf("<file %$q>, %d, %d",
		       self->a_ddi.l_file->f_namesize,
		       self->a_ddi.l_file->f_name,
		       self->a_ddi.l_line,
		       self->a_ddi.l_col);
		PRINT(")");
	}
	PRINT(")");
	return result;
err:
	return temp;
}


#undef printf
#undef PRINTAST
#undef PRINT
#undef print
#undef DO

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ast_print(DeeCompilerAstObject *__restrict self,
          dformatprinter printer, void *arg) {
	dssize_t result;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	result = print_ast_code(self->ci_value, printer, arg, false, NULL, 0);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ast_printrepr(DeeCompilerAstObject *__restrict self,
              dformatprinter printer, void *arg) {
	dssize_t result;
	if (COMPILER_BEGIN(self->ci_compiler))
		return -1;
	result = print_ast_repr(self->ci_value, printer, arg);
	COMPILER_END();
	return result;
}



PRIVATE struct type_getset tpconst ast_getsets[] = {
	TYPE_GETSET("scope", &ast_getscope, NULL, &ast_setscope,
	            "->?AScope?Ert:Compiler\n"
	            "@throw ValueError Attempted to set a scope associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a scope not apart of the same base-scope (s.a. :Compiler.scope.base)\n"
	            "@throw ReferenceError Attempted to set the scope of a branch containing symbols that would no longer be reachable\n"
	            "Get or set the scope with which this branch is associated"),
	TYPE_GETTER("kind", &ast_getkind,
	            "->?Dstring\n"
	            "Get the name of the ast kind (same as the `make*' methods of :Compiler)"),
	TYPE_GETTER("typeid", &ast_gettypeid,
	            "->?Dint\n"
	            "Get the internal type-id of ast"),
	TYPE_GETSET(STR_constexpr, &ast_getconstexpr, NULL, &ast_setconstexpr,
	            "->\n"
	            "@throw TypeError ?#kind isn't $\"constexpr\"\n"
	            "Get or set the constant expression value of a $\"constexpr\" (s.a. ?#kind) ast"),
	TYPE_GETSET(STR_sym, &ast_getsym, NULL, &ast_setsym,
	            "->?ASymbol?Ert:Compiler\n"
	            "@throw TypeError ?#kind isn't $\"sym\", $\"unbind\" or $\"bound\"\n"
	            "@throw ValueError Attempted to set a :Compiler.symbol associated with a different compiler\n"
	            "Get or set the symbol associated with a symbol-related AST"),
	TYPE_GETSET(STR_multiple,
	            &ast_getmultiple,
	            &ast_delmultiple,
	            &ast_setmultiple,
	            "->?S?.\n"
	            "@throw TypeError ?#kind isn't $\"multiple\"\n"
	            "@throw ValueError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the sequence of sub-branches associated with @this multi-branch ast"),
	TYPE_GETSET("multiple_typing", &ast_getmultiple_typing, NULL, &ast_setmultiple_typing,
	            "->?DType\n"
	            "@throw TypeError ?#kind isn't $\"multiple\"\n"
	            "@throw TypeError Attempted to set a typing that is neither ?N, nor one of the type listed in :Compiler.makemultiple\n"
	            "Get or set the typing of a @ multi-branch ast"),
	TYPE_GETSET("returnast",
	            &ast_getreturnast,
	            &ast_delreturnast,
	            &ast_setreturnast,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"return\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No return expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the ast describing the expression returned by @this branch\n"
	            "Additionally, you may assign ?N to delete the throw expression and have the branch return ?N"),
	TYPE_GETSET("yieldast", &ast_getyieldast, NULL, &ast_setyieldast,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"yield\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the ast describing the expression yielded by @this branch"),
	TYPE_GETSET("throwast",
	            &ast_getthrowast,
	            &ast_delthrowast,
	            &ast_setthrowast,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"throw\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No throw expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the ast describing the expression thrown by @this branch\n"
	            "Additionally, you may assign ?N to delete the throw expression and turn the branch into a re-throw"),
	TYPE_GETSET("tryguard", &ast_gettryguard, NULL, &ast_settryguard,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"try\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No throw expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the ast guarded by the ?#tryhandlers of @this try-branch"),
	TYPE_GETSET("tryhandlers", &ast_gettryhandlers, NULL, &ast_settryhandlers,
	            "->?S?T3?Dstring?.?.\n"
	            "@throw TypeError ?#kind isn't $\"try\"\n"
	            "@throw ValueError The compiler of one of the given branches or @scope doesn't match @this\n"
	            "@throw ValueError One of the flags-strings contains an unknown flag\n"
	            "@throw ReferenceError One of the given branch is not part of the basescope of the effective @scope\n"
	            "Get or set the ast guarded by the ?#tryhandlers of @this try-branch (s.a. :Compiler.maketry)"),
	TYPE_GETSET("loopflags", &ast_getloopflags, NULL, &ast_setloopflags,
	            "->?Dstring\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw ValueError Attempted to set an invalid flags string\n"
	            "@throw UnboundAttribute Attempted to enable $\"foreach\" mode with ?#loopnext being unbound\n"
	            "Get or set the flags controlling how a loop is evaluated\n"
	            "When enabling/disabling $\"foreach\" mode, ?#loopcond becomes ?#loopelem and "
	            /**/ "?#loopnext becomes ?#loopiter, though regardless of foreach-mode, ?#loopcond and "
	            /**/ "?#loopelem can be addressed as ?#loopelemcond, and ?#loopnext and ?#loopiter as ?#loopiternext"),
	TYPE_GETSET("loopisforeach", &ast_getloopisforeach, NULL, &ast_setloopisforeach,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw UnboundAttribute Attempted to enable $\"foreach\" mode with ?#loopnext being unbound\n"
	            "Get or set if @this ast is a foreach-loop, controlling the $\"foreach\" flag of ?#loopflags"),
	TYPE_GETSET("loopispostcond", &ast_getloopispostcond, NULL, &ast_setloopispostcond,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "Get or set if ?#loopcond is evaluated after ?#looploop or before, controlling the $\"postcond\" flag of ?#loopflags"),
	TYPE_GETSET("loopisunlikely",
	            &ast_getloopisunlikely,
	            NULL,
	            &ast_setloopisunlikely,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "Get or set if ?#looploop is unlikely to be executed, controlling the $\"unlikely\" flag of ?#loopflags"),
	TYPE_GETSET("loopcond",
	            &ast_getloopcond,
	            &ast_delloopcond,
	            &ast_setloopcond,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError ?#loopflags contains $\"foreach\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No condition expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the continue-condition of @this loop (${for (; loopcond; loopnext) looploop})\n"
	            "Additionally, you may assign ?N to delete the condition, causing it to always be true"),
	TYPE_GETSET("loopnext",
	            &ast_getloopnext,
	            &ast_delloopnext,
	            &ast_setloopnext,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError ?#loopflags contains $\"foreach\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No advance expression expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the optional advance expression of @this loop (${for (; loopcond; loopnext) looploop})\n"
	            "Additionally, you may assign ?N to delete the expression"),
	TYPE_GETSET("looploop",
	            &ast_getlooploop,
	            &ast_dellooploop,
	            &ast_setlooploop,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No loop block expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the block executed by @this loop (${for (; loopcond; loopnext) looploop})\n"
	            "Additionally, you may assign ?N to delete the block"),
	TYPE_GETSET("loopelem",
	            &ast_getloopelem,
	            &ast_delloopelem,
	            &ast_setloopelem,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError ?#loopflags doesn't contain $\"foreach\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No loop element has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the foreach element of @this loop (${foreach (loopelem: loopiter) looploop})\n"
	            "Additionally, you may assign ?N to delete the element, causing its value to be discarded immediately"),
	TYPE_GETSET("loopiter", &ast_getloopiter, NULL, &ast_setloopiter,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError ?#loopflags doesn't contain $\"foreach\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the foreach iterator expression of @this loop (${foreach (loopelem: loopiter) looploop})"),
	TYPE_GETSET("loopelemcond",
	            &ast_getloopelemcond,
	            &ast_delloopelemcond,
	            &ast_setloopelemcond,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No condition or element expression has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Alias for accessing either the condition of a regular loop (#loopcond), or the element of foreach-loop (#loopelem)"),
	TYPE_GETSET("loopiternext",
	            &ast_getloopiternext,
	            &ast_delloopiternext,
	            &ast_setloopiternext,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"loop\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No loop advance expression has been bound\n"
	            "@throw AttributeError Attempted to unbind or assign ?N to ?#loopiter\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Alias for accessing either the advance expression of a regular loop (#loopnext), or the iterator of foreach-loop (#loopiter)"),
	TYPE_GETSET("loopctlisbreak", &ast_getloopctlisbreak, NULL, &ast_setloopctlisbreak,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"loopctl\"\n"
	            "Get or set if @this loop control branch behaves as a $break, or as a $continue"),
	TYPE_GETSET("conditionalcond", &ast_getconditionalcond, NULL, &ast_setconditionalcond,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the condition used to determine the the path taken by a conditional branch"),
	TYPE_GETSET("conditionaltt",
	            &ast_getconditionaltt,
	            &ast_delconditionaltt,
	            &ast_setconditionaltt,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No true-branch has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the branch taken when ?#conditionalcond evaluates to ?t\n"
	            "Additionally, you may assign ?N to unbind the branch, or ?#conditionalcond to re-use "
	            /**/ "the value resulting from the conditiona branch as result of the true-branch"),
	TYPE_GETSET("conditionalff",
	            &ast_getconditionalff,
	            &ast_delconditionalff,
	            &ast_setconditionalff,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute No false-branch has been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the branch taken when ?#conditionalcond evaluates to ?f\n"
	            "Additionally, you may assign ?N to unbind the branch, or ?#conditionalcond to re-use "
	            /**/ "the value resulting from the conditiona branch as result of the false-branch"),
	TYPE_GETSET("conditionalflags", &ast_getconditionalflags, NULL, &ast_setconditionalflags,
	            "->?Dstring\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "@throw ValueError Attempted to set an invalid set of flags\n"
	            "Get or set the flags used for evaluating @this conditional branch (s.a. :Compiler.makeconditional)"),
	TYPE_GETSET("conditionalisbool", &ast_getconditionalisbool, NULL, &ast_setconditionalisbool,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "Control the $\"bool\"-flag of ?#conditionalflags (s.a. :Compiler.makeconditional)"),
	TYPE_GETSET("conditionalislikely", &ast_getconditionalislikely, NULL, &ast_setconditionalislikely,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "Control the $\"likely\"-flag of ?#conditionalflags (s.a. :Compiler.makeconditional)"),
	TYPE_GETSET("conditionalisunlikely", &ast_getconditionalisunlikely, NULL, &ast_setconditionalisunlikely,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"conditional\"\n"
	            "Control the $\"unlikely\"-flag of ?#conditionalflags (s.a. :Compiler.makeconditional)"),
	TYPE_GETSET("boolast", &ast_getboolast, NULL, &ast_setboolast,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"bool\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the ast describing the expression turned into a boolean by @this branch"),
	TYPE_GETSET("boolisnegated", &ast_getboolisnegated, NULL, &ast_setboolisnegated,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"bool\"\n"
	            "Get or set if the boolean value of ?#boolast should be negated"),
	TYPE_GETSET("expandast", &ast_getexpandast, NULL, &ast_setexpandast,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"expand\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the ast being expanded by @this one"),
	TYPE_GETSET("functioncode", &ast_getfunctioncode, NULL, &ast_setfunctioncode,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"function\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError ${this.scope} is not reachable from ${VALUE.scope}\n"
	            "@throw ReferenceError ${this.scope.base} is identical to ${VALUE.scope.base}\n"
	            "Get or set the code bound to the function of @this ast"),
	TYPE_GETSET("operatorfuncname", &ast_getoperatorfuncname, NULL, &ast_setoperatorfuncname,
	            "->?X2?Dstring?Dint\n"
	            "@throw TypeError ?#kind isn't $\"operatorfunc\"\n"
	            "@throw ValueError Attempted to set a name not recognized as a valid operator\n"
	            "Get or set the name of the operator that is loaded as a function by this branch"),
	TYPE_GETSET("operatorfuncbinding",
	            &ast_getoperatorfuncbinding,
	            &ast_deloperatorfuncbinding,
	            &ast_setoperatorfuncbinding,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operatorfunc\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute The operator function hasn't been bound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the binding of the operator function loaded by this branch\n"
	            "Additionally, you may assign ?N to unbind the binding, causing the operator "
	            /**/ "to be loaded as an unbound function"),
	TYPE_GETSET("operatorname", &ast_getoperatorname, NULL, &ast_setoperatorname,
	            "->?X2?Dstring?Dint\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw ValueError Attempted to set a name not recognized as a valid operator\n"
	            "Get or set the name of the operator executed by @this ast"),
	TYPE_GETSET("operatora", &ast_getoperatora, NULL, &ast_setoperatora,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the first operand used for invoking ?#operatorname"),
	TYPE_GETSET("operatorb", &ast_getoperatorb, &ast_deloperatorb, &ast_setoperatorb,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute The second operand has been unbound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the second operand used for invoking ?#operatorname\n"
	            "Additionally, you may assign ?N to unbind the operand"),
	TYPE_GETSET("operatorc", &ast_getoperatorc, &ast_deloperatorc, &ast_setoperatorc,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute The third operand has been unbound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the second third used for invoking ?#operatorname\n"
	            "Additionally, you may assign ?N to unbind the operand"),
	TYPE_GETSET("operatord", &ast_getoperatord, &ast_deloperatord, &ast_setoperatord,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw UnboundAttribute The fourth operand has been unbound\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get, del or set the second fourth used for invoking ?#operatorname\n"
	            "Additionally, you may assign ?N to unbind the operand"),
	TYPE_GETSET("operatorflags", &ast_getoperatorflags, NULL, &ast_setoperatorflags,
	            "->?Dstring\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw ValueError Attempted to set invalid flags\n"
	            "Get or set the flags used to describe special behavior for executing an operator (s.a. :Compiler.makeoperator)"),
	TYPE_GETSET("operatorispost", &ast_getoperatorispost, NULL, &ast_setoperatorispost,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "Get or set the $\"post\"-flag of ?#operatorflags (s.a. :Compiler.makeoperator)"),
	TYPE_GETSET("operatorisvarargs", &ast_getoperatorisvarargs, NULL, &ast_setoperatorisvarargs,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "Get or set the $\"varargs\"-flag of ?#operatorflags (s.a. :Compiler.makeoperator)"),
	TYPE_GETSET("operatorismaybeprefix", &ast_getoperatorismaybeprefix, NULL, &ast_setoperatorismaybeprefix,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "Get or set the $\"maybeprefix\"-flag of ?#operatorflags (s.a. :Compiler.makeoperator)"),
	TYPE_GETSET("operatorisdontoptimize", &ast_getoperatorisdontoptimize, NULL, &ast_setoperatorisdontoptimize,
	            "->?Dbool\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "Get or set the $\"dontoptimize\"-flag of ?#operatorflags (s.a. :Compiler.makeoperator)"),
	TYPE_GETSET("actionname", &ast_getactionname, NULL, &ast_setactionname,
	            "->?Dstring\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw ValueError Attempted to set an invalid action\n"
	            "@throw ValueError Attempted to set an action taking a different number of operands than "
	            "the old. To work around this restriction, use ?#setaction instead\n"
	            "Get or set the name of the action performed by @this ast"),
	TYPE_GETSET("actiona", &ast_getactiona, NULL, &ast_setactiona,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw AttributeError The currently set action takes $0 arguments\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the first operand used by the action performed by @this ast"),
	TYPE_GETSET("actionb", &ast_getactionb, NULL, &ast_setactionb,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw AttributeError The currently set action takes $0, or $1 argument\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the second operand used by the action performed by @this ast"),
	TYPE_GETSET("actionc", &ast_getactionc, NULL, &ast_setactionc,
	            "->?.\n"
	            "@throw TypeError ?#kind isn't $\"operator\"\n"
	            "@throw TypeError Attempted to set an ?AAst?Ert:Compiler associated with a different compiler\n"
	            "@throw AttributeError The currently set action takes $0, $1 or $2 arguments\n"
	            "@throw ReferenceError Attempted to set a sub-branch apart of a different base-scope than @this\n"
	            "Get or set the third operand used by the action performed by @this ast"),

	/* TODO: Access to all the different ast fields. */
	TYPE_GETSET_END
};



INTERN DeeTypeObject DeeCompilerAst_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Ast",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerObjItem_Type,
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
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ast_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ast_printrepr
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
	/* .tp_getsets       = */ ast_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_IAST_C */
