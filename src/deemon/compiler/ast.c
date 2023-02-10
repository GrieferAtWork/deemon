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
#ifndef GUARD_DEEMON_COMPILER_AST_C
#define GUARD_DEEMON_COMPILER_AST_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/cache.h>

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#ifdef CONFIG_AST_IS_STRUCT
DEFINE_STRUCT_CACHE(ast, struct ast, 512)
#else /* CONFIG_AST_IS_STRUCT */
DEFINE_OBJECT_CACHE(ast, struct ast, 512)
#endif /* !CONFIG_AST_IS_STRUCT */
DECLARE_STRUCT_CACHE(sym, struct symbol)

#ifndef NDEBUG
#define sym_alloc() sym_dbgalloc(__FILE__, __LINE__)
#endif /* !NDEBUG */

/* Re-use the symbol cache for labels. (As rare as they are, this is the best way to allocate them) */
#define lbl_alloc() ((struct text_label *)sym_alloc())
#define lbl_free(p) sym_free((struct symbol *)(p))


#ifdef CONFIG_TRACE_REFCHANGES
#define INIT_REF(x) (Dee_Incref(x), Dee_DecrefNokill(x))
#else /* CONFIG_TRACE_REFCHANGES */
#define INIT_REF(x) (void)0
#endif /* !CONFIG_TRACE_REFCHANGES */

#ifndef NDEBUG
#ifdef CONFIG_NO_AST_DEBUG
#define ast_new() ast_dbgnew(__FILE__, __LINE__)
#else /* CONFIG_NO_AST_DEBUG */
#define ast_new() ast_dbgnew(file, line)
#endif /* !CONFIG_NO_AST_DEBUG */
PRIVATE WUNUSED DREF struct ast *DCALL
ast_dbgnew(char const *file, int line) {
	DREF struct ast *result = ast_dbgalloc(file, line);
#ifndef CONFIG_NO_THREADS
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
#endif /* !CONFIG_NO_THREADS */
	if likely(result) {
#ifdef CONFIG_AST_IS_STRUCT
		result->a_refcnt = 1;
#else /* CONFIG_AST_IS_STRUCT */
		DeeObject_Init(result, &DeeAst_Type);
#endif /* !CONFIG_AST_IS_STRUCT */
		result->a_scope      = current_scope;
		result->a_ddi.l_file = NULL;
		Dee_Incref(result->a_scope);
	}
	return result;
}
#else /* !NDEBUG */
INTERN WUNUSED DREF struct ast *DCALL ast_new(void) {
	DREF struct ast *result = ast_alloc();
#ifndef CONFIG_NO_THREADS
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
#endif /* !CONFIG_NO_THREADS */
	if likely(result) {
		DeeObject_Init(result, &DeeAst_Type);
		result->a_scope      = current_scope;
		result->a_ddi.l_file = NULL;
		Dee_Incref(result->a_scope);
	}
	return result;
}
#endif /* NDEBUG */

INTERN NONNULL((1)) void FCALL
loc_here(struct ast_loc *__restrict info) {
	info->l_file = TPPLexer_Global.l_token.t_file;
	/* Query line/column information for the current token's start position. */
	TPPFile_LCAt(info->l_file, &info->l_lc,
	             TPPLexer_Global.l_token.t_begin);
}

INTERN NONNULL((2)) struct ast *FCALL
ast_setddi(struct ast *self,
           struct ast_loc *__restrict info) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL' for `ast'. */
	ASSERT_AST(self);
	if unlikely(self->a_ddi.l_file)
		TPPFile_Decref(self->a_ddi.l_file);
	if (tpp_is_reachable_file(info->l_file)) {
		self->a_ddi = *info;
	} else {
		loc_here(&self->a_ddi);
	}
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
done:
	return self;
}

INTERN struct ast *FCALL
ast_sethere(struct ast *self) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL' for `ast'. */
	ASSERT_AST(self);
	if unlikely(self->a_ddi.l_file)
		TPPFile_Decref(self->a_ddi.l_file);
	loc_here(&self->a_ddi);
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
done:
	return self;
}

INTERN NONNULL((2)) struct ast *FCALL
ast_putddi(struct ast *self,
           struct ast_loc *__restrict info) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL' for `ast'. */
	ASSERT_AST(self);
	if unlikely(self->a_ddi.l_file)
		goto done;
	if (tpp_is_reachable_file(info->l_file)) {
		self->a_ddi = *info;
	} else {
		loc_here(&self->a_ddi);
	}
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
done:
	return self;
}

INTERN struct ast *FCALL
ast_puthere(struct ast *self) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL' for `ast'. */
	ASSERT_AST(self);
	if unlikely(self->a_ddi.l_file)
		goto done;
	loc_here(&self->a_ddi);
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
done:
	return self;
}



INTERN NONNULL((1)) void DCALL
ast_incwrite(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_SYM:
		ASSERT(self->a_sym);
		if (self->a_flag++ == 0) {
			SYMBOL_DEC_NREAD(self->a_sym);
			SYMBOL_INC_NWRITE(self->a_sym);
		}
		break;

	case AST_MULTIPLE: {
		size_t i;
		/* Multiple write targets (incref each individually) */
		for (i = 0; i < self->a_multiple.m_astc; ++i)
			ast_incwrite(self->a_multiple.m_astv[i]);
	}	break;

	default: break;
	}
}

INTERN NONNULL((1)) void DCALL
ast_incwriteonly(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_SYM:
		ASSERT(self->a_sym);
		if (self->a_flag++ == 0)
			SYMBOL_INC_NWRITE(self->a_sym);
		break;

	case AST_MULTIPLE: {
		size_t i;
		/* Multiple write targets (incref each individually) */
		for (i = 0; i < self->a_multiple.m_astc; ++i)
			ast_incwriteonly(self->a_multiple.m_astv[i]);
	}	break;

	default: break;
	}
}

INTERN NONNULL((1)) void DCALL
ast_decwrite(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_SYM:
		ASSERT(self->a_sym);
		ASSERT(self->a_flag);
		if (!--self->a_flag) {
			SYMBOL_DEC_NWRITE(self->a_sym);
			SYMBOL_INC_NREAD(self->a_sym);
		}
		break;

	case AST_MULTIPLE: {
		size_t i;
		/* Multiple write targets (decref each individually) */
		for (i = 0; i < self->a_multiple.m_astc; ++i)
			ast_decwrite(self->a_multiple.m_astv[i]);
	}	break;

	default: break;
	}
}

INTERN NONNULL((1)) void DCALL
ast_decwriteonly(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_SYM:
		ASSERT(self->a_sym);
		ASSERT(self->a_flag);
		if (!--self->a_flag)
			SYMBOL_DEC_NWRITE(self->a_sym);
		break;

	case AST_MULTIPLE: {
		size_t i;
		/* Multiple write targets (decref each individually) */
		for (i = 0; i < self->a_multiple.m_astc; ++i)
			ast_decwriteonly(self->a_multiple.m_astv[i]);
	}	break;

	default: break;
	}
}

#ifdef CONFIG_NO_AST_DEBUG
#define DEFINE_AST_GENERATOR(attr, name, args) \
	INTERN WUNUSED attr DREF struct ast *(DCALL name)args
#else /* CONFIG_NO_AST_DEBUG */
#define DEFINE_AST_GENERATOR(attr, name, args) \
	INTERN WUNUSED attr DREF struct ast *(DCALL name##_d)PRIVATE_AST_GENERATOR_UNPACK_ARGS args
#endif /* !CONFIG_NO_AST_DEBUG */


DEFINE_AST_GENERATOR(NONNULL((1)), ast_constexpr,
                     (DeeObject *__restrict constant_expression)) {
	DREF struct ast *result = ast_new();
	ASSERT_OBJECT(constant_expression);
	if likely(result) {
		result->a_type      = AST_CONSTEXPR;
		result->a_constexpr = constant_expression;
		Dee_Incref(constant_expression);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1)), ast_sym,
                     (struct symbol * __restrict sym)) {
	DREF struct ast *result = ast_new();
	if likely(result) {
		result->a_type = AST_SYM;
		result->a_flag = 0; /* Start out as a read-reference. */
		result->a_sym  = sym;
		SYMBOL_INC_NREAD(sym);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1)), ast_unbind,
                     (struct symbol * __restrict sym)) {
	DREF struct ast *result = ast_new();
	if likely(result) {
		result->a_type   = AST_UNBIND;
		result->a_unbind = sym;
		SYMBOL_INC_NWRITE(sym);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1)), ast_bound,
                     (struct symbol * __restrict sym)) {
	DREF struct ast *result = ast_new();
	if likely(result) {
		result->a_type  = AST_BOUND;
		result->a_bound = sym;
		SYMBOL_INC_NBOUND(sym);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(, ast_operator_func,
                     (uint16_t operator_name, struct ast *binding)) {
	DREF struct ast *result;
	ASSERT_AST_OPT(binding);
	result = ast_new();
	if likely(result) {
		result->a_type                     = AST_OPERATOR_FUNC;
		result->a_flag                     = operator_name;
		result->a_operator_func.of_binding = binding;
		ast_xincref(binding);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(, ast_multiple,
                     (uint16_t flags, size_t exprc,
                      /*inherit*/ DREF struct ast **exprv)) {
	DREF struct ast *result;
	/* Prevent AST ambiguity by not allowing
	 * keep-last, single-element multiple ASTs.
	 * -> ... Because they're literally no-ops that would otherwise
	 *        confuse the optimizer into not detecting constant
	 *        expressions, as well as special behavior surrounding
	 *       `AST_EXPAND' expressions not being triggered. */
	if unlikely(exprc == 1 && flags == AST_FMULTIPLE_KEEPLAST &&
	            current_scope == exprv[0]->a_scope) {
		result = exprv[0]; /* Inherit reference. */
got_result:
		Dee_Free(exprv); /* We're supposed to ~inherit~ this on success. (So we simply discard it). */
		return result;
	}
	/* Prevent some more ambiguity when ZERO(0) expressions were passed. */
	if unlikely(exprc == 0) {
		if (flags == AST_FMULTIPLE_KEEPLAST) {
			result = ast_constexpr(Dee_None);
got_result_maybe:
			if unlikely(!result)
				goto err;
			goto got_result;
		}
		if (flags == AST_FMULTIPLE_TUPLE ||
		    flags == AST_FMULTIPLE_GENERIC) {
			result = ast_constexpr(Dee_EmptyTuple);
			goto got_result_maybe;
		}
	}
	result = ast_new();
	if likely(result) {
		result->a_type            = AST_MULTIPLE;
		result->a_flag            = flags;
		result->a_multiple.m_astc = exprc;
		result->a_multiple.m_astv = exprv;
		INIT_REF(result);
	}
	return result;
err:
	return NULL;
}

DEFINE_AST_GENERATOR(, ast_return,
                     (struct ast *return_expr)) {
	DREF struct ast *result;
	ASSERT_AST_OPT(return_expr);
	result = ast_new();
	if likely(result) {
		result->a_type   = AST_RETURN;
		result->a_return = return_expr;
		ast_xincref(return_expr);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1)), ast_yield,
                     (struct ast *__restrict yield_expr)) {
	DREF struct ast *result;
	ASSERT_AST(yield_expr);
	result = ast_new();
	if likely(result) {
		result->a_type   = AST_YIELD;
		result->a_return = yield_expr;
		ast_incref(yield_expr);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(, ast_throw,
                     (struct ast *throw_expr)) {
	DREF struct ast *result;
	ASSERT_AST_OPT(throw_expr);
	result = ast_new();
	if likely(result) {
		result->a_type   = AST_THROW;
		result->a_return = throw_expr;
		ast_xincref(throw_expr);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1)), ast_try,
                     (struct ast *__restrict guarded_expression,
                      size_t catchc, /*inherit*/ struct catch_expr *catchv)) {
	DREF struct ast *result;
	ASSERT_AST(guarded_expression);
	if unlikely(!catchc) {
		/* Prevent ambiguity when no handlers are defined. */
		Dee_Free(catchv); /* May still be a non-NULL buffer. */
		ast_incref(guarded_expression);
		return guarded_expression;
	}
	result = ast_new();
	if likely(result) {
		result->a_type         = AST_TRY;
		result->a_try.t_guard  = guarded_expression;
		result->a_try.t_catchc = catchc;
		result->a_try.t_catchv = catchv;
		ast_incref(guarded_expression);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1, 2)), ast_tryfinally,
                     (struct ast *__restrict guarded_expression,
                      struct ast *__restrict finally_expression)) {
	struct catch_expr *catchv;
	DREF struct ast *result;
	/* Allocate the catch-expression vector inherited by `a_try' upon success. */
	catchv = (struct catch_expr *)Dee_Mallocc(1, sizeof(struct catch_expr));
	if unlikely(!catchv)
		goto err;
	catchv[0].ce_code  = finally_expression;         /* Reference is incremented later. */
	catchv[0].ce_flags = EXCEPTION_HANDLER_FFINALLY; /* Regular, old finally-handler. */
	catchv[0].ce_mask  = NULL;                       /* Regular finally-handlers don't have masks. */
#ifdef CONFIG_NO_AST_DEBUG
	result = ast_try(guarded_expression, 1, catchv);
#else /* CONFIG_NO_AST_DEBUG */
	result = ast_try_d(guarded_expression, 1, catchv, file, line);
#endif /* !CONFIG_NO_AST_DEBUG */
	if unlikely(!result) {
		Dee_Free(catchv);
	} else {
		ast_incref(finally_expression);
	}
	return result;
err:
	return NULL;
}

DEFINE_AST_GENERATOR(, ast_loop,
                     (uint16_t flags, struct ast *elem_or_cond,
                      struct ast *iter_or_next, struct ast *loop)) {
	DREF struct ast *result;
	ASSERT_AST_OPT(elem_or_cond);
	ASSERT_AST_OPT(iter_or_next);
	ASSERT_AST_OPT(loop);
	result = ast_new();
	if likely(result) {
		/* Apply the unlikely-branch tag to loop expressions.
		 * When set, the loop block is usually placed in cold text. */
#if AST_FCOND_UNLIKELY == AST_FLOOP_UNLIKELY
		flags |= (current_tags.at_expect & AST_FCOND_UNLIKELY);
#else /* AST_FCOND_UNLIKELY == AST_FLOOP_UNLIKELY */
		if (current_tags.at_expect & AST_FCOND_UNLIKELY)
			flags |= AST_FLOOP_UNLIKELY;
#endif /* AST_FCOND_UNLIKELY != AST_FLOOP_UNLIKELY */
		result->a_type        = AST_LOOP;
		result->a_flag        = flags;
		result->a_loop.l_cond = elem_or_cond;
		result->a_loop.l_next = iter_or_next;
		result->a_loop.l_loop = loop;
		ast_xincref(elem_or_cond);
		ast_xincref(iter_or_next);
		ast_xincref(loop);
		if (elem_or_cond && (flags & AST_FLOOP_FOREACH))
			ast_incwrite(elem_or_cond);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(, ast_loopctl, (uint16_t flags)) {
	DREF struct ast *result;
	result = ast_new();
	if likely(result) {
		result->a_type = AST_LOOPCTL;
		result->a_flag = flags;
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((2)), ast_conditional,
                     (uint16_t flags, struct ast *cond,
                      struct ast *tt_expr, struct ast *ff_expr)) {
	DREF struct ast *result;
	ASSERT_AST(cond);
	ASSERT_AST_OPT(tt_expr);
	ASSERT_AST_OPT(ff_expr);
	ASSERTF(tt_expr || ff_expr, "At least one must be present");
	ASSERTF(tt_expr != ff_expr, "These can't be the same");
	result = ast_new();
	if likely(result) {
		result->a_type               = AST_CONDITIONAL;
		result->a_flag               = flags;
		result->a_conditional.c_cond = cond;
		result->a_conditional.c_tt   = tt_expr;
		result->a_conditional.c_ff   = ff_expr;
		ast_incref(cond);
		ast_xincref(tt_expr);
		ast_xincref(ff_expr);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((2)), ast_bool,
                     (uint16_t flags, struct ast *__restrict expr)) {
	DREF struct ast *result;
	ASSERT_AST(expr);
	result = ast_new();
	if likely(result) {
		result->a_type = AST_BOOL;
		result->a_flag = flags;
		result->a_bool = expr;
		ast_incref(expr);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1)), ast_expand,
                     (struct ast *__restrict expr)) {
	DREF struct ast *result;
	ASSERT_AST(expr);
#if 0 /* This causes problems with code such as `([foo]...)' being \
       * interpreted as though it was written as `(foo)', rather  \
       * than as written as `(foo,)' */
	/* To prevent ambiguity, always expand single-element,
	 * sequence-multi-expressions without going through an AST_EXPAND. */
	if (expr->a_type == AST_MULTIPLE &&
	    expr->a_flag != AST_FMULTIPLE_KEEPLAST &&
	    expr->a_multiple.m_astc == 1) {
		result = expr->a_multiple.m_astv[0];
		ast_incref(result);
		return result;
	}
#endif
	result = ast_new();
	if likely(result) {
		result->a_type   = AST_EXPAND;
		result->a_expand = expr;
		ast_incref(expr);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((1, 2)), ast_function,
                     (struct ast *__restrict function_code,
                      DeeBaseScopeObject *__restrict scope)) {
	DREF struct ast *result;
	ASSERT_AST(function_code);
	ASSERT_OBJECT_TYPE((DeeObject *)scope, &DeeBaseScope_Type);
	result = ast_new();
	if likely(result) {
		result->a_type             = AST_FUNCTION;
		result->a_function.f_code  = function_code;
		result->a_function.f_scope = scope;
		ast_incref(function_code);
		Dee_Incref((DeeObject *)scope);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((3)), ast_operator1,
                     (uint16_t operator_name, uint16_t flags,
                      struct ast *__restrict opa)) {
	DREF struct ast *result;
	ASSERT_AST(opa);
	result = ast_new();
	if likely(result) {
		if (OPERATOR_ISINPLACE(operator_name))
			ast_incwriteonly(opa);
		result->a_type              = AST_OPERATOR;
		result->a_flag              = operator_name;
		result->a_operator.o_op0    = opa;
		result->a_operator.o_op1    = NULL;
		result->a_operator.o_op2    = NULL;
		result->a_operator.o_op3    = NULL;
		result->a_operator.o_exflag = flags;
		ast_incref(opa);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((3, 4)), ast_operator2,
                     (uint16_t operator_name, uint16_t flags,
                      struct ast *__restrict opa,
                      struct ast *__restrict opb)) {
	DREF struct ast *result;
	ASSERT_AST(opa);
	ASSERT_AST(opb);
	result = ast_new();
	if likely(result) {
		if (OPERATOR_ISINPLACE(operator_name))
			ast_incwriteonly(opa);
		result->a_type              = AST_OPERATOR;
		result->a_flag              = operator_name;
		result->a_operator.o_op0    = opa;
		result->a_operator.o_op1    = opb;
		result->a_operator.o_op2    = NULL;
		result->a_operator.o_op3    = NULL;
		result->a_operator.o_exflag = flags;
		ast_incref(opa);
		ast_incref(opb);
		INIT_REF(result);
	}
	return result;
}
DEFINE_AST_GENERATOR(NONNULL((3, 4, 5)), ast_operator3,
                     (uint16_t operator_name, uint16_t flags,
                      struct ast *__restrict opa,
                      struct ast *__restrict opb,
                      struct ast *__restrict opc)) {
	DREF struct ast *result;
	ASSERT_AST(opa);
	ASSERT_AST(opb);
	ASSERT_AST(opc);
	result = ast_new();
	if likely(result) {
		if (OPERATOR_ISINPLACE(operator_name))
			ast_incwriteonly(opa);
		result->a_type              = AST_OPERATOR;
		result->a_flag              = operator_name;
		result->a_operator.o_op0    = opa;
		result->a_operator.o_op1    = opb;
		result->a_operator.o_op2    = opc;
		result->a_operator.o_op3    = NULL;
		result->a_operator.o_exflag = flags;
		ast_incref(opa);
		ast_incref(opb);
		ast_incref(opc);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((3, 4, 5, 6)), ast_operator4,
                     (uint16_t operator_name, uint16_t flags,
                      struct ast *__restrict opa,
                      struct ast *__restrict opb,
                      struct ast *__restrict opc,
                      struct ast *__restrict opd)) {
	DREF struct ast *result;
	ASSERT_AST(opa);
	ASSERT_AST(opb);
	ASSERT_AST(opc);
	ASSERT_AST(opd);
	result = ast_new();
	if likely(result) {
		if (OPERATOR_ISINPLACE(operator_name))
			ast_incwriteonly(opa);
		result->a_type              = AST_OPERATOR;
		result->a_flag              = operator_name;
		result->a_operator.o_op0    = opa;
		result->a_operator.o_op1    = opb;
		result->a_operator.o_op2    = opc;
		result->a_operator.o_op3    = opd;
		result->a_operator.o_exflag = flags;
		ast_incref(opa);
		ast_incref(opb);
		ast_incref(opc);
		ast_incref(opd);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(, ast_action0,
                     (uint16_t action_flags)) {
	DREF struct ast *result;
	ASSERT(AST_FACTION_ARGC_GT(action_flags) == 0);
	result = ast_new();
	if likely(result) {
		result->a_type          = AST_ACTION;
		result->a_flag          = action_flags;
		result->a_action.a_act0 = NULL;
		result->a_action.a_act1 = NULL;
		result->a_action.a_act2 = NULL;
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((2)), ast_action1,
                     (uint16_t action_flags,
                      struct ast *__restrict act0)) {
	DREF struct ast *result;
	ASSERT(AST_FACTION_ARGC_GT(action_flags) == 1);
	ASSERT_AST(act0);
	result = ast_new();
	if likely(result) {
		result->a_type          = AST_ACTION;
		result->a_flag          = action_flags;
		result->a_action.a_act0 = act0;
		result->a_action.a_act1 = NULL;
		result->a_action.a_act2 = NULL;
		ast_incref(act0);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((2, 3)), ast_action2,
                     (uint16_t action_flags,
                      struct ast *__restrict act0,
                      struct ast *__restrict act1)) {
	DREF struct ast *result;
	ASSERT(AST_FACTION_ARGC_GT(action_flags) == 2);
	ASSERT_AST(act0);
	ASSERT_AST(act1);
	result = ast_new();
	if likely(result) {
		result->a_type          = AST_ACTION;
		result->a_flag          = action_flags;
		result->a_action.a_act0 = act0;
		result->a_action.a_act1 = act1;
		result->a_action.a_act2 = NULL;
		ast_incref(act0);
		ast_incref(act1);
		if ((action_flags & AST_FACTION_KINDMASK) ==
		    (AST_FACTION_STORE & AST_FACTION_KINDMASK))
			ast_incwrite(act0);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((2, 3, 4)), ast_action3,
                     (uint16_t action_flags,
                      struct ast *__restrict act0,
                      struct ast *__restrict act1,
                      struct ast *__restrict act2)) {
	DREF struct ast *result;
	ASSERT(AST_FACTION_ARGC_GT(action_flags) == 3);
	ASSERT_AST(act0);
	ASSERT_AST(act1);
	ASSERT_AST(act2);
	result = ast_new();
	if likely(result) {
		result->a_type          = AST_ACTION;
		result->a_flag          = action_flags;
		result->a_action.a_act0 = act0;
		result->a_action.a_act1 = act1;
		result->a_action.a_act2 = act2;
		ast_incref(act0);
		ast_incref(act1);
		ast_incref(act2);
		INIT_REF(result);
	}
	return result;
}


DEFINE_AST_GENERATOR(NONNULL((2)), ast_class,
                     (struct ast *base, struct ast *__restrict descriptor,
                      struct symbol *class_symbol, struct symbol *super_symbol,
                      size_t memberc, struct class_member *memberv)) {
	DREF struct ast *result;
	ASSERT_AST_OPT(base);
	ASSERT_AST(descriptor);
	ASSERT(!memberc || memberv);
	result = ast_new();
	if likely(result) {
		result->a_type             = AST_CLASS;
		result->a_flag             = AST_FCLASS_NORMAL;
		result->a_class.c_base     = base;
		result->a_class.c_desc     = descriptor;
		result->a_class.c_memberc  = memberc;
		result->a_class.c_memberv  = memberv;
		result->a_class.c_classsym = class_symbol;
		result->a_class.c_supersym = super_symbol;
		if (class_symbol)
			SYMBOL_INC_NWRITE(class_symbol);
		if (super_symbol) {
			/* If the base expression is a symbol that is identical to the super-symbol,
			 * then set the NOWRITESUPER flag to prevent an unnecessary write! */
			if (base && base->a_type == AST_SYM &&
			    base->a_sym == super_symbol) {
				result->a_flag |= AST_FCLASS_NOWRITESUPER;
			} else {
				SYMBOL_INC_NWRITE(super_symbol);
			}
		}
		ast_xincref(base);
		ast_incref(descriptor);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((2, 3)), ast_label,
                     (uint16_t flags, struct text_label *__restrict lbl,
                      DeeBaseScopeObject *__restrict base_scope)) {
	DREF struct ast *result;
	result = ast_new();
	if likely(result) {
		result->a_type          = AST_LABEL;
		result->a_flag          = flags;
		result->a_label.l_label = lbl;
		result->a_label.l_base  = base_scope;
		Dee_Incref((DeeObject *)base_scope);
		INIT_REF(result);
	}
	return result;
}
DEFINE_AST_GENERATOR(NONNULL((1, 2)), ast_goto,
                     (struct text_label * __restrict lbl,
                      DeeBaseScopeObject *__restrict base_scope)) {
	DREF struct ast *result;
	result = ast_new();
	if likely(result) {
		result->a_type         = AST_GOTO;
		result->a_flag         = AST_FNORMAL;
		result->a_goto.g_label = lbl;
		result->a_goto.g_base  = base_scope;
		Dee_Incref((DeeObject *)base_scope);
		/* Trace the number of uses of this label. */
		++lbl->tl_goto;
		INIT_REF(result);
	}
	return result;
}
DEFINE_AST_GENERATOR(NONNULL((2, 3)), ast_switch,
                     (uint16_t flags,
                      struct ast *__restrict expr,
                      struct ast *__restrict block,
                      struct text_label *cases,
                      struct text_label *default_case)) {
	DREF struct ast *result;
	result = ast_new();
	if likely(result) {
		result->a_type             = AST_SWITCH;
		result->a_flag             = flags;
		result->a_switch.s_expr    = expr;
		result->a_switch.s_block   = block;
		result->a_switch.s_cases   = cases;
		result->a_switch.s_default = default_case;
		ast_incref(expr);
		ast_incref(block);
		/* Increment the counter of the default-case. */
		if (default_case)
			++default_case->tl_goto;
		/* Increment the goto-counter of each case. */
		for (; cases; cases = cases->tl_next)
			++cases->tl_goto;
		INIT_REF(result);
	}
	return result;
}


#ifndef CONFIG_LANGUAGE_NO_ASM
DEFINE_AST_GENERATOR(NONNULL((2)), ast_assembly,
                     (uint16_t flags, struct TPPString *__restrict text,
                      size_t num_o, size_t num_i, size_t num_l,
                      /*inherit*/ struct asm_operand *opv))
#else /* !CONFIG_LANGUAGE_NO_ASM */
DEFINE_AST_GENERATOR(, ast_assembly,
                     (uint16_t flags,
                      size_t num_o, size_t num_i, size_t num_l,
                      /*inherit*/ struct asm_operand *opv))
#endif /* CONFIG_LANGUAGE_NO_ASM */
{
	DREF struct ast *result;
	ASSERT(!(num_o + num_i + num_l) || opv);
	result = ast_new();
	if likely(result) {
		size_t i;
		/* Track the writes to output operands. */
		for (i = 0; i < num_o; ++i) {
			if (ASM_OPERAND_IS_INOUT(&opv[i])) {
				ast_incwriteonly(opv[i].ao_expr);
			} else {
				ast_incwrite(opv[i].ao_expr);
			}
		}
		result->a_type = AST_ASSEMBLY;
		result->a_flag = flags;
#ifndef CONFIG_LANGUAGE_NO_ASM
		result->a_assembly.as_text.at_text = text;
		TPPString_Incref(text);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
		result->a_assembly.as_num_o = num_o;
		result->a_assembly.as_num_i = num_i;
		result->a_assembly.as_num_l = num_l;
		result->a_assembly.as_opc   = num_o + num_i + num_l;
		result->a_assembly.as_opv   = opv;
		INIT_REF(result);
	}
	return result;
}
#undef DEFINE_AST_GENERATOR

INTERN void DCALL
cleanup_switch_cases(struct text_label *switch_cases,
                     struct text_label *switch_default) {
	if (switch_default) {
		ASSERT(!switch_default->tl_next);
		ASSERT(!switch_default->tl_expr);
		lbl_free(switch_default);
	}
	while (switch_cases) {
		struct text_label *next;
		next = switch_cases->tl_next;
		ASSERT_AST(switch_cases->tl_expr);
		ast_decref(switch_cases->tl_expr);
		lbl_free(switch_cases);
		switch_cases = next;
	}
}

INTERN NONNULL((2)) void DCALL
visit_switch_cases(struct text_label *switch_cases,
                   dvisit_t proc, void *arg) {
	while (switch_cases) {
		ASSERT_AST(switch_cases->tl_expr);
		ast_visit(switch_cases->tl_expr);
		switch_cases = switch_cases->tl_next;
	}
}

INTERN NONNULL((1)) void DCALL
ast_fini_contents(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_LOOP:
		if (self->a_flag & AST_FLOOP_FOREACH &&
		    self->a_loop.l_elem) {
			ast_decwrite(self->a_loop.l_elem);
		}
		goto do_xdecref_3;

	case AST_CLASS: {
		size_t i;
		/* Cleanup the member descriptor table. */
		for (i = 0; i < self->a_class.c_memberc; ++i)
			ast_decref(self->a_class.c_memberv[i].cm_ast);
		Dee_Free(self->a_class.c_memberv);
		ast_xdecref(self->a_class.c_base);
		ast_decref(self->a_class.c_desc);
		if (self->a_class.c_classsym)
			SYMBOL_DEC_NWRITE(self->a_class.c_classsym);
		if (self->a_class.c_supersym &&
		    !(self->a_flag & AST_FCLASS_NOWRITESUPER))
			SYMBOL_DEC_NWRITE(self->a_class.c_supersym);
	}	break;

	case AST_OPERATOR:
		if (OPERATOR_ISINPLACE(self->a_flag))
			ast_decwriteonly(self->a_operator.o_op0);
		ast_xdecref(self->a_operator.o_op3);
		ATTR_FALLTHROUGH
	case AST_CONDITIONAL:
do_xdecref_3:
		ast_xdecref(self->a_operator.o_op2);
		ATTR_FALLTHROUGH
	case AST_FUNCTION:
		ast_xdecref(self->a_operator.o_op1);
		ATTR_FALLTHROUGH
	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
	case AST_BOOL:
	case AST_EXPAND:
	case AST_OPERATOR_FUNC:
		ast_xdecref(self->a_return);
		break;

	case AST_CONSTEXPR:
		Dee_Decref(self->a_constexpr);
		break;

	case AST_ACTION:
		if ((self->a_flag & AST_FACTION_KINDMASK) ==
		    (AST_FACTION_STORE & AST_FACTION_KINDMASK)) {
			ast_decwrite(self->a_action.a_act0);
		}
		goto do_xdecref_3;

	case AST_MULTIPLE: {
		DREF struct ast **iter, **end;
		end = (iter = self->a_multiple.m_astv) +
		      self->a_multiple.m_astc;
		for (; iter < end; ++iter)
			ast_decref(*iter);
		Dee_Free(self->a_multiple.m_astv);
	}	break;

	case AST_TRY: {
		struct catch_expr *iter, *end;
		ast_decref(self->a_try.t_guard);
		end = (iter = self->a_try.t_catchv) +
		      self->a_try.t_catchc;
		for (; iter < end; ++iter) {
			ast_xdecref(iter->ce_mask);
			ast_decref(iter->ce_code);
		}
		Dee_Free(self->a_try.t_catchv);
	}	break;

	case AST_SYM:
		ASSERTF(!self->a_flag, "At least some write-wrappers havn't been unwound.");
		SYMBOL_DEC_NREAD(self->a_sym);
		break;

	case AST_UNBIND:
		SYMBOL_DEC_NWRITE(self->a_unbind);
		break;

	case AST_BOUND:
		SYMBOL_DEC_NBOUND(self->a_bound);
		break;

	case AST_GOTO:
		ASSERT(self->a_goto.g_label);
		ASSERT(self->a_goto.g_label->tl_goto);
		--self->a_goto.g_label->tl_goto;
		ATTR_FALLTHROUGH
	case AST_LABEL:
		Dee_Decref((DeeObject *)self->a_label.l_base);
		break;

	case AST_SWITCH:
		cleanup_switch_cases(self->a_switch.s_cases,
		                     self->a_switch.s_default);
		ast_decref(self->a_switch.s_expr);
		ast_decref(self->a_switch.s_block);
		break;

	case AST_ASSEMBLY: {
		struct asm_operand *iter, *end;
		size_t i;
		end = (iter = self->a_assembly.as_opv) +
		      (self->a_assembly.as_num_o +
		       self->a_assembly.as_num_i);
		/* Track the writes to output operands. */
		for (i = 0; i < self->a_assembly.as_num_o; ++i) {
			struct asm_operand *op = &self->a_assembly.as_opv[i];
			if (ASM_OPERAND_IS_INOUT(op)) {
				ast_decwriteonly(op->ao_expr);
			} else {
				ast_decwrite(op->ao_expr);
			}
		}
		for (; iter < end; ++iter) {
			ASSERT(iter->ao_type);
			ASSERT(iter->ao_expr);
			TPPString_Decref(iter->ao_type);
			ast_decref(iter->ao_expr);
		}
		end += self->a_assembly.as_num_l;
		for (; iter < end; ++iter) {
			ASSERT(!iter->ao_type);
			ASSERT(iter->ao_label);
			ASSERT(iter->ao_label->tl_goto);
			--iter->ao_label->tl_goto;
		}
		Dee_Free(self->a_assembly.as_opv);
#ifndef CONFIG_LANGUAGE_NO_ASM
		TPPString_Decref(self->a_assembly.as_text.at_text);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	}	break;

	//case AST_LOOPCTL:
	default: break;
	}
}

INTERN NONNULL((1, 2)) void DCALL
ast_visit_impl(struct ast *__restrict self,
               dvisit_t proc, void *arg) {
	switch (self->a_type) {

	case AST_CLASS: {
		size_t i;
		/* Visit the member descriptor table. */
		for (i = 0; i < self->a_class.c_memberc; ++i)
			ast_visit(self->a_class.c_memberv[i].cm_ast);
		if (self->a_class.c_base)
			ast_visit(self->a_class.c_base);
		ast_visit(self->a_class.c_desc);
	}	break;

	case AST_OPERATOR:
		if (self->a_operator.o_op3)
			ast_visit(self->a_operator.o_op3);
		ATTR_FALLTHROUGH
	case AST_CONDITIONAL:
	case AST_LOOP:
do_xvisit_3:
		if (self->a_operator.o_op2)
			ast_visit(self->a_operator.o_op2);
		ATTR_FALLTHROUGH
	case AST_FUNCTION:
		if (self->a_operator.o_op1)
			ast_visit(self->a_operator.o_op1);
		ATTR_FALLTHROUGH
	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
	case AST_BOOL:
	case AST_EXPAND:
	case AST_OPERATOR_FUNC:
		if (self->a_return)
			ast_visit(self->a_return);
		break;

	case AST_CONSTEXPR:
		Dee_Visit(self->a_constexpr);
		break;

	case AST_ACTION:
		if ((self->a_flag & AST_FACTION_KINDMASK) ==
		    (AST_FACTION_STORE & AST_FACTION_KINDMASK)) {
			ast_decwrite(self->a_action.a_act0);
		}
		goto do_xvisit_3;

	case AST_MULTIPLE: {
		DREF struct ast **iter, **end;
		end = (iter = self->a_multiple.m_astv) +
		      self->a_multiple.m_astc;
		for (; iter < end; ++iter)
			ast_visit(*iter);
	}	break;

	case AST_TRY: {
		struct catch_expr *iter, *end;
		ast_visit(self->a_try.t_guard);
		end = (iter = self->a_try.t_catchv) +
		      self->a_try.t_catchc;
		for (; iter < end; ++iter) {
			if (iter->ce_mask)
				ast_visit(iter->ce_mask);
			ast_visit(iter->ce_code);
		}
	}	break;

	case AST_GOTO:
	case AST_LABEL:
		Dee_Visit((DeeObject *)self->a_label.l_base);
		break;

	case AST_SWITCH:
		visit_switch_cases(self->a_switch.s_cases, proc, arg);
		ast_visit(self->a_switch.s_expr);
		ast_visit(self->a_switch.s_block);
		break;

	case AST_ASSEMBLY: {
		struct asm_operand *iter, *end;
		end = (iter = self->a_assembly.as_opv) +
		      (self->a_assembly.as_num_o +
		       self->a_assembly.as_num_i);
		for (; iter < end; ++iter)
			ast_visit(iter->ao_expr);
	}	break;

	default: break;
	}
#if 0 /* ??? */
	if (self->a_ddi.l_file)
		TPPFile_Visit(self->a_ddi.l_file, proc, arg);
#endif
	Dee_Visit(self->a_scope);
}


#ifdef CONFIG_AST_IS_STRUCT
INTERN NONNULL((1)) void DCALL ast_destroy(struct ast *__restrict self)
#else /* CONFIG_AST_IS_STRUCT */
PRIVATE NONNULL((1)) void DCALL ast_fini(struct ast *__restrict self)
#endif /* !CONFIG_AST_IS_STRUCT */
{
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
	DeeCompiler_DelItem(self);
	if (self->a_ddi.l_file)
		TPPFile_Decref(self->a_ddi.l_file);
	ast_fini_contents(self);
	/* NOTE: Must destroy the scope _AFTER_ the AST contents, in case the
	 *       ast itself references some symbol that is owned by this scope. */
	Dee_Decref(self->a_scope);
#ifdef CONFIG_AST_IS_STRUCT
	ast_free(self);
#endif /* CONFIG_AST_IS_STRUCT */
}




#ifndef CONFIG_AST_IS_STRUCT
INTERN DeeTypeObject DeeAst_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Ast",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL, /*&DeeObject_Type*/
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_ALLOCATOR(&ast_tp_alloc, &ast_tp_free)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ast_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ast_visit_impl,
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
#endif /* !CONFIG_AST_IS_STRUCT */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_C */
