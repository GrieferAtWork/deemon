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
#ifndef GUARD_DEEMON_COMPILER_AST_C
#define GUARD_DEEMON_COMPILER_AST_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/code.h>              /* Dee_EXCEPTION_HANDLER_FFINALLY */
#include <deemon/compiler/ast.h>      /* ASM_OPERAND_IS_INOUT, ASSERT_AST, ASSERT_AST_OPT, AST_*, CONFIG_NO_AST_DEBUG, PRIVATE_AST_GENERATOR_UNPACK_ARGS, asm_operand, ast, ast_*, catch_expr, class_member */
#include <deemon/compiler/compiler.h> /* DeeCompiler_DelItem, DeeCompiler_LockReading */
#include <deemon/compiler/lexer.h>    /* DeeLexer, DeeLexer_GetLoc, ast_loc, ast_loc_init_empty, ast_loc_isempty, loc_here, tpp_* */
#include <deemon/compiler/parser.h>   /* current_tags */
#include <deemon/compiler/symbol.h>   /* DeeBaseScopeObject, DeeBaseScope_Type, SYMBOL_DEC_N*, SYMBOL_INC_N*, current_scope, lbl_free, symbol, text_label */
#include <deemon/map.h>               /* Dee_EmptyMap */
#include <deemon/none.h>              /* Dee_None */
#include <deemon/format.h>              /* Dee_None */
#include <deemon/object.h>            /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, DREF, DeeObject, DeeTypeObject, Dee_Decref, Dee_DecrefNokill, Dee_Incref, OBJECT_HEAD_INIT */
#include <deemon/seq.h>               /* Dee_EmptySeq */
#include <deemon/set.h>               /* Dee_EmptySet */
#include <deemon/tuple.h>             /* Dee_EmptyTuple */
#include <deemon/type.h>              /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, Dee_operator_t, Dee_visit_t, OPERATOR_ISINPLACE, TF_NONE, TP_FNORMAL */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER

#ifdef DeeAst_HAVE_DBG
#define DBG_file  file
#define DBG_line  line
#define DBG__args , file, line
#else /* DeeAst_HAVE_DBG */
#define DBG_file  __FILE__
#define DBG_line  __LINE__
#define DBG__args /* nothing */
#endif /* !DeeAst_HAVE_DBG */

PRIVATE NONNULL((1)) void DCALL
DeeAst_Destroyv(DeeAst *const *vector, size_t count) {
	while (count--)
		DeeAst_Destroy(vector[count]);
}

/* Destroy any type of (fully initialized) AST */
INTERN NONNULL((1)) void DCALL
DeeAst_Destroy_(DeeAst *__restrict self DeeAst__DBG_PARAMS) {
	/* Destroy common data... */
	_DeeAst_FiniCommon(self);

	/* Destroy ast-specific data... */
	switch (self->a_kind) {

	case Dee_AST_CONST:
		Dee_Decref(self->a_data.ad_const.ace_value);
		DeeAst_Dbg_FreeUninitializedConst(self, DBG_file, DBG_line);
		break;

	case Dee_AST_MULTI:
		DeeAst_Destroyv(self->a_data.ad_multi.am_elem,
		                self->a_data.ad_multi.am_count);
		DeeAst_Dbg_FreeUninitializedMulti(self, DBG_file, DBG_line);
		break;

	case Dee_AST_LOOP:
		DeeAst_XDestroy(self->a_data.ad_loop.al_data.ald_normal.aln_cond);
		DeeAst_XDestroy(self->a_data.ad_loop.al_data.ald_normal.aln_next);
		DeeAst_Destroy(self->a_data.ad_loop.al_body);
		Dee_asm_symbol_fini(&self->a_data.ad_loop.al_break);
		Dee_asm_symbol_fini(&self->a_data.ad_loop.al_continue);
		DeeAst_Dbg_FreeUninitializedLoop(self, DBG_file, DBG_line);
		break;

	case Dee_AST_IF:
		DeeAst_Destroy(self->a_data.ad_if.ai_cond);
		if (self->a_data.ad_if.ai_tt != self->a_data.ad_if.ai_cond)
			DeeAst_XDestroy(self->a_data.ad_if.ai_tt);
		if (self->a_data.ad_if.ai_ff != self->a_data.ad_if.ai_cond)
			DeeAst_XDestroy(self->a_data.ad_if.ai_ff);
		DeeAst_Dbg_FreeUninitializedIf(self, DBG_file, DBG_line);
		break;

	case Dee_AST_SWITCH_STMT: {
		/* Start by unbinding all cases */
		struct Dee_ast_label *iter, *tvar;
		LIST_FOREACH_SAFE (iter, &self->a_data.ad_switch_stmt.asws_cases,
		                   al_data.ald_case.alc_entry, tvar) {
			LIST_UNBIND(iter, al_data.ald_case.alc_entry);
		}
		DeeAst_Destroy(self->a_data.ad_switch_stmt.asws_expr);
		DeeAst_Destroy(self->a_data.ad_switch_stmt.asws_body);
		Dee_asm_symbol_fini(&self->a_data.ad_switch_stmt.asws_break);
		DeeAst_Dbg_FreeUninitializedSwitchStmt(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_SWITCH_EXPR: {
		size_t i;
		DeeAst_Destroy(self->a_data.ad_switch_expr.aswe_expr);
		for (i = 0; i < self->a_data.ad_switch_expr.aswe_casec; ++i) {
			struct Dee_ast_switch_expr_case *this_case;
			this_case = self->a_data.ad_switch_expr.aswe_casev[i];
			Dee_ast_switch_expr_case_destroy(this_case);
		}
		DeeAst_Dbg_FreeUninitializedSwitchExpr(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_CATCH: {
		size_t i;
		DeeAst_Destroy(self->a_data.ad_catch.ac_body);
		for (i = 0; i < self->a_data.ad_catch.ac_handlerc; ++i) {
			struct Dee_ast_catch_handler *handler;
			handler = &self->a_data.ad_catch.ac_handlerv[i];
			Dee_ast_catch_handler_fini(handler);
		}
		DeeAst_Dbg_FreeUninitializedCatch(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_FINALLY:
		DeeAst_Destroy(self->a_data.ad_finally.af_body);
		DeeAst_Destroy(self->a_data.ad_finally.af_finally);
		DeeAst_Dbg_FreeUninitializedFinally(self, DBG_file, DBG_line);
		break;

	case Dee_AST_YIELD:
		DeeAst_Destroy(self->a_data.ad_yield.ay_expr);
		DeeAst_Dbg_FreeUninitializedYield(self, DBG_file, DBG_line);
		break;

	case Dee_AST_RETURN:
		DeeAst_XDestroy(self->a_data.ad_return.ar_expr);
		DeeAst_Dbg_FreeUninitializedReturn(self, DBG_file, DBG_line);
		break;

	case Dee_AST_FUNCTION: {
		size_t i;
		struct Dee_ast_function_param_base *special_param;
		DeeAst_Destroy(self->a_data.ad_function.af_body);
		if ((special_param = self->a_data.ad_function.af_varargs) != NULL)
			Dee_ast_function_param_base_destroy(special_param);
		if ((special_param = self->a_data.ad_function.af_varkwds) != NULL)
			Dee_ast_function_param_base_destroy(special_param);
		Dee_link_symbol_xdecref(self->a_data.ad_function.af_symbol_this);
		Dee_link_symbol_xdecref(self->a_data.ad_function.af_symbol_tp_this);
		for (i = 0; i < self->a_data.ad_function.af_argc_max; ++i) {
			struct Dee_ast_function_param *this_param;
			this_param = &self->a_data.ad_function.af_params[i];
			Dee_ast_function_param_fini(this_param);
		}
		DeeAst_Dbg_FreeUninitializedFunction(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_CLASS: {
		size_t i;
		DeeAst_XDestroy(self->a_data.ad_class.ac_base);
		DeeAst_Destroy(self->a_data.ad_class.ac_desc);
		if (self->a_data.ad_class.ac_tp_this) {
			Dee_link_symbol_wr_dec(self->a_data.ad_class.ac_tp_this);
			Dee_link_symbol_decref(self->a_data.ad_class.ac_tp_this);
		}
		for (i = 0; i < self->a_data.ad_class.ac_memberc; ++i) {
			struct Dee_ast_class_member *this_member;
			this_member = &self->a_data.ad_class.ac_memberv[i];
			Dee_ast_class_member_fini(this_member);
		}
		DeeAst_Dbg_FreeUninitializedClass(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_UNPACK_WR: {
		size_t i;
		DeeAst_Destroy(self->a_data.ad_unpack_wr.aupwr_expr);
		for (i = 0; i < self->a_data.ad_unpack_wr.aupwr_targetc; ++i) {
			struct Dee_ast_unpack_wr_target *this_target;
			this_target = &self->a_data.ad_unpack_wr.aupwr_targetv[i];
			Dee_ast_unpack_wr_target_fini(this_target);
		}
		DeeAst_Dbg_FreeUninitializedUnpackWr(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_IASM: {
		size_t i;
		ASSERT(self->a_data.ad_iasm.aia_opc ==
		       (self->a_data.ad_iasm.aia_opc_i +
		        self->a_data.ad_iasm.aia_opc_o +
		        self->a_data.ad_iasm.aia_opc_l));
		if (self->a_data.ad_iasm.aia_text_chunk)
			tpp_string_decref(self->a_data.ad_iasm.aia_text_chunk);
		for (i = 0; i < self->a_data.ad_iasm.aia_opc; ++i) {
			struct Dee_ast_iasm_operand *this_operand;
			this_operand = &self->a_data.ad_iasm.aia_opv[i];
			Dee_ast_iasm_operand_fini(this_operand);
		}
		DeeAst_Dbg_FreeUninitializedIAsm(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_OPERATOR: {
#ifndef NDEBUG
		size_t i;
		for (i = 0; i < self->a_data.ad_operator.ao_argc; ++i) {
			ASSERTF(self->a_data.ad_operator.ao_argv[i] != NULL,
			        "Operand %" PRFuSIZ " should be non-`NULL`", i);
		}
		for (; i < Dee_AST_OPERATOR_ARGC_MAX; ++i) {
			ASSERTF(self->a_data.ad_operator.ao_argv[i] == NULL,
			        "Operand %" PRFuSIZ " should be `NULL`", i);
		}
#endif /* !NDEBUG */
		Dee_ast_operator_id_fini(&self->a_data.ad_operator.ao_id);
		DeeAst_Destroyv(self->a_data.ad_operator.ao_argv,
		                self->a_data.ad_operator.ao_argc);
		DeeAst_Dbg_FreeUninitializedOperator(self, DBG_file, DBG_line);
	}	break;

	Dee_AST_CASE_ACT1
		DeeAst_Destroy(self->a_data.ad_act1.aa_act0);
		DeeAst_Dbg_FreeUninitializedAct1(self, DBG_file, DBG_line);
		break;

	Dee_AST_CASE_ACT2
		DeeAst_Destroy(self->a_data.ad_act2.aa_act1);
		DeeAst_Destroy(self->a_data.ad_act2.aa_act0);
		DeeAst_Dbg_FreeUninitializedAct2(self, DBG_file, DBG_line);
		break;

	Dee_AST_CASE_ACT3
		DeeAst_Destroy(self->a_data.ad_act3.aa_act2);
		DeeAst_Destroy(self->a_data.ad_act3.aa_act1);
		DeeAst_Destroy(self->a_data.ad_act3.aa_act0);
		DeeAst_Dbg_FreeUninitializedAct3(self, DBG_file, DBG_line);
		break;

#define STATIC_ASSERT_BINARY_COMPATIBLE(f1, f2)                                    \
		STATIC_ASSERT(offsetof(DeeAst, a_data.f1) == offsetof(DeeAst, a_data.f2)); \
		STATIC_ASSERT(sizeof(((DeeAst *)0)->a_data.f1) == sizeof(((DeeAst *)0)->a_data.f2))
	case Dee_AST_SYMBOL_WR:
	case Dee_AST_SYMBOL_XCH:
	case Dee_AST_SYMBOL_INIT: {
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_wr.aswr_value, ad_symbol_xch.asxc_value);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_wr.aswr_value, ad_symbol_init.asin_value);
		DeeAst_Destroy(self->a_data.ad_symbol_wr.aswr_value);
	}	ATTR_FALLTHROUGH
	case Dee_AST_SYMBOL_RD:
	case Dee_AST_SYMBOL_DEL: {
		DREF struct Dee_link_symbol *symbol;
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd.asrd_symbol, ad_symbol_del.asub_symbol);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd.asrd_symbol, ad_symbol_wr.aswr_symbol);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd.asrd_symbol, ad_symbol_xch.asxc_symbol);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd.asrd_symbol, ad_symbol_init.asin_symbol);
		symbol = self->a_data.ad_symbol_rd.asrd_symbol;
		switch (self->a_kind) {
		case Dee_AST_SYMBOL_RD:
			Dee_link_symbol_rd_dec(symbol);
			break;
		case Dee_AST_SYMBOL_WR:
		case Dee_AST_SYMBOL_INIT:
			Dee_link_symbol_wr_dec(symbol);
			break;
		case Dee_AST_SYMBOL_DEL:
			Dee_link_symbol_del_dec(symbol);
			break;
		case Dee_AST_SYMBOL_XCH:
			Dee_link_symbol_rdwr_dec(symbol);
			break;
		default: __builtin_unreachable();
		}
		Dee_link_symbol_decref(symbol);
		_DeeAst_Dbg_FreeUninitializedSymbolLike(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_SYMREF_WR:
	case Dee_AST_SYMREF_XCH: {
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symref_wr.asrwr_value, ad_symref_xch.asrxc_value);
		DeeAst_Destroy(self->a_data.ad_symref_wr.asrwr_value);
	}	ATTR_FALLTHROUGH
	case Dee_AST_SYMREF_RD:
	case Dee_AST_SYMREF_DEL: {
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symref_rd.asrrd_sym, ad_symref_del.asrub_sym);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symref_rd.asrrd_sym, ad_symref_wr.asrwr_sym);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symref_rd.asrrd_sym, ad_symref_xch.asrxc_sym);
		Dee_ast_symref_fini(&self->a_data.ad_symref_rd.asrrd_sym);
		_DeeAst_Dbg_FreeUninitializedSymbolLike(self, DBG_file, DBG_line);
	}	break;
#undef STATIC_ASSERT_BINARY_COMPATIBLE

	case Dee_AST_ASM_GOTO:
	case Dee_AST_GOTO:
	case Dee_AST_BREAK:
	case Dee_AST_CONTINUE:
		_DeeAst_Dbg_FreeUninitializedGotoLike(self, DBG_file, DBG_line);
		break;
	default: __builtin_unreachable();
	}
}

/* AST API */
INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewConst_(DeeObject *__restrict value DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedConst(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_CONST);
	result->a_data.ad_const.ace_value = value;
	Dee_Incref(value);
	return result;
err:
	return NULL;
}

INTERN WUNUSED DeeAst *DCALL
DeeAst_NewMulti_(enum Dee_ast_multi_kind kind, size_t count,
                 /*inherit(always)*/ DeeAst *const *asts DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedMulti(count, DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_MULTI);
	result->a_data.ad_multi.am_kind  = kind;
	result->a_data.ad_multi.am_count = count;
	memcpyc(result->a_data.ad_multi.am_elem, asts, count, sizeof(DeeAst *));
	return result;
err:
	DeeAst_Destroyv(asts, count);
	return NULL;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_PackMulti_(DeeAst *__restrict self, enum Dee_ast_multi_kind kind,
                  size_t count DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedMulti(self, count, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_MULTI);
	result->a_data.ad_multi.am_kind  = kind;
	result->a_data.ad_multi.am_count = count;
	return result;
}

INTERN WUNUSED NONNULL((4)) DeeAst *DCALL
DeeAst_NewLoop_(Dee_ast_loop_flag flag,
                /*inherit(always)*/ DeeAst *opt_cond_or_elem,
                /*inherit(always)*/ DeeAst *opt_next_or_iter,
                /*inherit(always)*/ DeeAst *body DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedLoop(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_LOOP);
	ASSERT(body);
	result->a_data.ad_loop.al_flag                     = flag;
	result->a_data.ad_loop.al_data.ald_normal.aln_cond = opt_cond_or_elem;
	result->a_data.ad_loop.al_data.ald_normal.aln_next = opt_next_or_iter;
	result->a_data.ad_loop.al_body                     = body;
	Dee_asm_symbol_init(&result->a_data.ad_loop.al_break);
	Dee_asm_symbol_init(&result->a_data.ad_loop.al_continue);
	return result;
err:
	DeeAst_XDestroy(opt_cond_or_elem);
	DeeAst_XDestroy(opt_next_or_iter);
	DeeAst_Destroy(body);
	return NULL;
}

INTERN WUNUSED NONNULL((2)) DeeAst *DCALL
DeeAst_NewIf_(Dee_ast_if_flag flag,
              /*inherit(always)*/ DeeAst *cond,
              /*inherit(always)*/ DeeAst *opt_tt,
              /*inherit(always)*/ DeeAst *opt_ff DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedIf(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_IF);
	result->a_data.ad_if.ai_flag = flag;
	result->a_data.ad_if.ai_cond = cond;
	result->a_data.ad_if.ai_tt   = opt_tt;
	result->a_data.ad_if.ai_ff   = opt_ff;
	return result;
err:
	if (opt_tt != cond)
		DeeAst_XDestroy(opt_tt);
	if (opt_ff != cond)
		DeeAst_XDestroy(opt_ff);
	DeeAst_Destroy(cond);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewSwitchStmt_(/*inherit(always)*/ DeeAst *__restrict expr,
                      /*inherit(always)*/ DeeAst *__restrict body DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedSwitchStmt(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SWITCH_STMT);
	result->a_data.ad_switch_stmt.asws_expr = expr;
	result->a_data.ad_switch_stmt.asws_body = body;
	LIST_INIT(&result->a_data.ad_switch_stmt.asws_cases); /* Populated by linker */
	result->a_data.ad_switch_stmt.asws_default = NULL;    /* Populated by linker */
	Dee_asm_symbol_init(&result->a_data.ad_switch_stmt.asws_break);
	return result;
err:
	DeeAst_Destroy(body);
	DeeAst_Destroy(expr);
	return NULL;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_PackSwitchExpr_(/*inherit(always)*/ DeeAst *__restrict self,
                       /*inherit(always)*/ DeeAst *__restrict expr,
                       size_t defcase, size_t casec DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedSwitchExpr(self, casec, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_SWITCH_EXPR);
	result->a_data.ad_switch_expr.aswe_expr    = expr;
	result->a_data.ad_switch_expr.aswe_defcase = defcase;
	result->a_data.ad_switch_expr.aswe_casec   = casec;
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_PackCatch_(/*inherit(always)*/ DeeAst *__restrict self,
                  /*inherit(always)*/ DeeAst *__restrict body,
                  size_t handlerc DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedCatch(self, handlerc, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_CATCH);
	result->a_data.ad_catch.ac_body     = body;
	result->a_data.ad_catch.ac_handlerc = handlerc;
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewFinally_(/*inherit(always)*/ DeeAst *__restrict body,
                   /*inherit(always)*/ DeeAst *__restrict finally_ DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedFinally(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_FINALLY);
	result->a_data.ad_finally.af_body    = body;
	result->a_data.ad_finally.af_finally = finally_;
	return result;
err:
	DeeAst_Destroy(body);
	DeeAst_Destroy(finally_);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewYield_(/*inherit(always)*/ DeeAst *__restrict expr DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedYield(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_YIELD);
	result->a_data.ad_yield.ay_expr = expr;
	return result;
err:
	DeeAst_Destroy(expr);
	return NULL;
}

INTERN WUNUSED DeeAst *DCALL
DeeAst_NewReturn_(/*inherit(always)*/ DeeAst *opt_expr DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedReturn(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_RETURN);
	result->a_data.ad_return.ar_expr = opt_expr;
	return result;
err:
	DeeAst_XDestroy(opt_expr);
	return NULL;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1, 4)) DeeAst *DCALL
DeeAst_PackFunction_(/*inherit(always)*/ DeeAst *__restrict self,
                     Dee_ast_function_flag flag, tpp_keyword const *opt_name,
                     /*inherit(always)*/ DeeAst *__restrict body,
                     /*inherit(always)*/ struct Dee_ast_function_param_base *opt_varargs,
                     /*inherit(always)*/ struct Dee_ast_function_param_base *opt_varkwds,
                     size_t argc_min, size_t argc_max DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedFunction(self, argc_max, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_FUNCTION);
	result->a_data.ad_function.af_flag           = flag;
	result->a_data.ad_function.af_name           = opt_name;
	result->a_data.ad_function.af_body           = body;
	result->a_data.ad_function.af_varargs        = opt_varargs;
	result->a_data.ad_function.af_varkwds        = opt_varkwds;
	result->a_data.ad_function.af_argc_min       = argc_min;
	result->a_data.ad_function.af_argc_max       = argc_max;
	result->a_data.ad_function.af_symbol_this    = NULL; /* Set by linker */
	result->a_data.ad_function.af_symbol_tp_this = NULL; /* Set by linker */
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1, 3)) DeeAst *DCALL
DeeAst_PackClass_(/*inherit(always)*/ DeeAst *__restrict self,
                  /*inherit(always)*/ DeeAst *opt_base,
                  /*inherit(always)*/ DeeAst *__restrict desc,
                  size_t memberc DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedClass(self, memberc, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_CLASS);
	result->a_data.ad_class.ac_base    = opt_base;
	result->a_data.ad_class.ac_desc    = desc;
	result->a_data.ad_class.ac_tp_this = NULL; /* Set by linker */
	result->a_data.ad_class.ac_memberc = memberc;
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_PackUnpackWr_(/*inherit(always)*/ DeeAst *__restrict self,
                     /*inherit(always)*/ DeeAst *__restrict expr,
                     size_t targetc DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedUnpackWr(self, targetc, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_UNPACK_WR);
	result->a_data.ad_unpack_wr.aupwr_expr    = expr;
	result->a_data.ad_unpack_wr.aupwr_targetc = targetc;
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_PackIAsm_(/*inherit(always)*/ DeeAst *__restrict self, Dee_ast_iasm_flag flag,
                 tpp_char const *text, tpp_size text_size, tpp_string *opt_text_chunk,
                 tpp_lcinfo text_chunk_lc, size_t opc_o, size_t opc_i, size_t opc_l DeeAst__DBG_PARAMS) {
	size_t opc = opc_o + opc_i + opc_l;
	DeeAst *result = DeeAst_Dbg_TryReallocUninitializedIAsm(self, opc, DBG_file, DBG_line);
	if unlikely(!result)
		result = self;
	_DeeAst_InitCommon(result, Dee_AST_IASM);
	if (opt_text_chunk)
		tpp_string_incref(opt_text_chunk);
	result->a_data.ad_iasm.aia_flag       = flag;
	result->a_data.ad_iasm.aia_text       = text;
	result->a_data.ad_iasm.aia_text_size  = text_size;
	result->a_data.ad_iasm.aia_text_chunk = opt_text_chunk;
	result->a_data.ad_iasm.aia_text_lc    = text_chunk_lc;
	result->a_data.ad_iasm.aia_opc_o      = opc_o;
	result->a_data.ad_iasm.aia_opc_i      = opc_i;
	result->a_data.ad_iasm.aia_opc_l      = opc_l;
	result->a_data.ad_iasm.aia_opc        = opc;
	return result;
}

INTERN WUNUSED NONNULL((2)) DeeAst *DCALL
DeeAst_NewOperator1_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
	Dee_ast_operator_id_initid(&result->a_data.ad_operator.ao_id, op);
	result->a_data.ad_operator.ao_flag    = Dee_AST_OPERATOR_FLAG_NORMAL;
	result->a_data.ad_operator.ao_argc    = 1;
	result->a_data.ad_operator.ao_argv[0] = arg0;
	result->a_data.ad_operator.ao_argv[1] = NULL;
	result->a_data.ad_operator.ao_argv[2] = NULL;
	result->a_data.ad_operator.ao_argv[3] = NULL;
	return result;
err:
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((2, 3)) DeeAst *DCALL
DeeAst_NewOperator2_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0,
                     /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
	Dee_ast_operator_id_initid(&result->a_data.ad_operator.ao_id, op);
	result->a_data.ad_operator.ao_flag    = Dee_AST_OPERATOR_FLAG_NORMAL;
	result->a_data.ad_operator.ao_argc    = 2;
	result->a_data.ad_operator.ao_argv[0] = arg0;
	result->a_data.ad_operator.ao_argv[1] = arg1;
	result->a_data.ad_operator.ao_argv[2] = NULL;
	result->a_data.ad_operator.ao_argv[3] = NULL;
	return result;
err:
	DeeAst_Destroy(arg1);
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((2, 3, 4)) DeeAst *DCALL
DeeAst_NewOperator3_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0,
                     /*inherit(always)*/ DeeAst *__restrict arg1,
                     /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
	Dee_ast_operator_id_initid(&result->a_data.ad_operator.ao_id, op);
	result->a_data.ad_operator.ao_flag    = Dee_AST_OPERATOR_FLAG_NORMAL;
	result->a_data.ad_operator.ao_argc    = 3;
	result->a_data.ad_operator.ao_argv[0] = arg0;
	result->a_data.ad_operator.ao_argv[1] = arg1;
	result->a_data.ad_operator.ao_argv[2] = arg2;
	result->a_data.ad_operator.ao_argv[3] = NULL;
	return result;
err:
	DeeAst_Destroy(arg2);
	DeeAst_Destroy(arg1);
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((2, 3, 4, 5)) DeeAst *DCALL
DeeAst_NewOperator4_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0,
                     /*inherit(always)*/ DeeAst *__restrict arg1,
                     /*inherit(always)*/ DeeAst *__restrict arg2,
                     /*inherit(always)*/ DeeAst *__restrict arg3 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
	Dee_ast_operator_id_initid(&result->a_data.ad_operator.ao_id, op);
	result->a_data.ad_operator.ao_flag    = Dee_AST_OPERATOR_FLAG_NORMAL;
	result->a_data.ad_operator.ao_argc    = 4;
	result->a_data.ad_operator.ao_argv[0] = arg0;
	result->a_data.ad_operator.ao_argv[1] = arg1;
	result->a_data.ad_operator.ao_argv[2] = arg2;
	result->a_data.ad_operator.ao_argv[3] = arg3;
	return result;
err:
	DeeAst_Destroy(arg3);
	DeeAst_Destroy(arg2);
	DeeAst_Destroy(arg1);
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((2)) DeeAst *DCALL
DeeAst_NewAct1_(enum Dee_ast_kind act,
                /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedAct1(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	ASSERT(Dee_AST_ISACT1(act));
	_DeeAst_InitCommon(result, act);
	result->a_data.ad_act1.aa_act0 = arg0;
	return result;
err:
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((2, 3)) DeeAst *DCALL
DeeAst_NewAct2_(enum Dee_ast_kind act,
                /*inherit(always)*/ DeeAst *__restrict arg0,
                /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedAct2(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	ASSERT(Dee_AST_ISACT2(act));
	_DeeAst_InitCommon(result, act);
	result->a_data.ad_act2.aa_act0 = arg0;
	result->a_data.ad_act2.aa_act1 = arg1;
	return result;
err:
	DeeAst_Destroy(arg1);
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((2, 3, 4)) DeeAst *DCALL
DeeAst_NewAct3_(enum Dee_ast_kind act,
                /*inherit(always)*/ DeeAst *__restrict arg0,
                /*inherit(always)*/ DeeAst *__restrict arg1,
                /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedAct3(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	ASSERT(Dee_AST_ISACT3(act));
	_DeeAst_InitCommon(result, act);
	result->a_data.ad_act3.aa_act0 = arg0;
	result->a_data.ad_act3.aa_act1 = arg1;
	result->a_data.ad_act3.aa_act2 = arg2;
	return result;
err:
	DeeAst_Destroy(arg2);
	DeeAst_Destroy(arg1);
	DeeAst_Destroy(arg0);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewGoto_(tpp_keyword const *label_name DeeAst__DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedGotoLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_GOTO);
	result->a_data.ad_goto.ach_symname = label_name;
	return result;
err:
	return NULL;
}

INTERN WUNUSED DeeAst *DCALL
DeeAst_NewBreak_(tpp_keyword const *opt_label_name DeeAst__DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedGotoLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_BREAK);
	result->a_data.ad_break.ab_symname = opt_label_name;
	return result;
err:
	return NULL;
}

INTERN WUNUSED DeeAst *DCALL
DeeAst_NewContinue_(tpp_keyword const *opt_label_name DeeAst__DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedGotoLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_CONTINUE);
	result->a_data.ad_continue.ac_symname = opt_label_name;
	return result;
err:
	return NULL;
}

/* Symbol functions */
INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewSymRefByName_(tpp_keyword const *__restrict name,
                        enum Dee_ast_symref_byname_kind kind,
                        Dee_ast_symref_byname_flag flag DeeAst__DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedSymbolLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF_RD);
	Dee_ast_symref_init_byname(&result->a_data.ad_symref_rd.asrrd_sym, name, kind, flag);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewSymRefNth_(tpp_keyword const *__restrict name,
                     Dee_ast_symref_nth_id nth_id DeeAst__DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedSymbolLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF_RD);
	Dee_ast_symref_init_nth(&result->a_data.ad_symref_rd.asrrd_sym, name, nth_id);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewSymRefExtern_(tpp_keyword const *__restrict name,
                        tpp_char const *modname_str, tpp_size modname_len,
                        tpp_string *opt_modname_chunk DeeAst__DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedSymbolLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF_RD);
	Dee_ast_symref_init_extern(&result->a_data.ad_symref_rd.asrrd_sym, name,
	                           modname_str, modname_len, opt_modname_chunk);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewSymRefThis_(DeeAst_DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedSymbolLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF_RD);
	Dee_ast_symref_init_this(&result->a_data.ad_symref_rd.asrrd_sym);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewSymRefTpThis_(DeeAst_DBG_PARAMS) {
	DeeAst *result = _DeeAst_Dbg_MallocUninitializedSymbolLike(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF_RD);
	Dee_ast_symref_init_tp_this(&result->a_data.ad_symref_rd.asrrd_sym);
	return result;
err:
	return NULL;
}

PRIVATE ATTR_COLD ATTR_NOINLINE WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_Transform_Del_dummy_(/*inherit(always)*/ DeeAst *__restrict self,
                            DeeLexer *__restrict lexer,
                            struct Dee_ast_loc *__restrict loc DeeAst__DBG_PARAMS) {
	DeeAst *const_none;
	if (DeeLexer_WarnfLoc(lexer, loc, TPP_W_ASM_STORE_TO_RVALUE))
		goto err; /* Emit warning */
	const_none = DeeAst_NewConst_(Dee_None DBG__args);
	if unlikely(!const_none)
		goto err;
	DeeAst_SetLoc(const_none, loc);
	return DeeAst_TrySetLoc(DeeAst_NewOperator2_(OPERATOR_ASSIGN, self, const_none DBG__args), loc);
err:
	DeeAst_Destroy(self);
	return NULL;
}

/* Transform the given AST `self` for an expression `del(<self>)`:
 * - Convert `Dee_AST_OPERATOR + OPERATOR_GETATTR` into `Dee_AST_OPERATOR + OPERATOR_DELATTR`
 * - Convert `Dee_AST_OPERATOR + OPERATOR_GETITEM` into `Dee_AST_OPERATOR + OPERATOR_DELITEM`
 * - Convert `Dee_AST_OPERATOR + OPERATOR_GETRANGE` into `Dee_AST_OPERATOR + OPERATOR_DELRANGE`
 * - Convert `Dee_AST_SYMBOL_RD` into `Dee_AST_SYMBOL_DEL`
 * - Convert `Dee_AST_SYMREF_RD` into `Dee_AST_SYMREF_DEL`
 * - Convert `Dee_AST_MULTI + Dee_AST_MULTI_KIND_ISSEQ()` into `Dee_AST_MULTI + Dee_AST_MULTI_KIND_KEEPLAST`,
 *   where each element is the appropriate `Dee_AST_SYMBOL_DEL` / `Dee_AST_SYMREF_DEL` / ...
 * All other types of ASTs are warned about, then packed with `<self> := none` */
INTERN WUNUSED NONNULL((1, 3, 4)) DeeAst *DCALL
DeeAst_Transform_Del_(/*inherit(always)*/ DeeAst *__restrict self,
                      Dee_ast_symref_del_flag flag,
                      DeeLexer *__restrict lexer,
                      struct Dee_ast_loc *__restrict loc DeeAst__DBG_PARAMS) {
	DeeAst_UpdateLoc(self, loc);
	switch (self->a_kind) {

	case Dee_AST_MULTI: {
		size_t i;
		if (Dee_AST_MULTI_KIND_ISLAST(self->a_data.ad_multi.am_kind)) {
			/* Propagate into multi-last -> last-element */
			DeeAst *last_multi;
			if (self->a_data.ad_multi.am_count == 0)
				return self;
			last_multi = self->a_data.ad_multi.am_elem[self->a_data.ad_multi.am_count - 1];
			last_multi = DeeAst_Transform_Del_(self, flag, lexer, loc DBG__args);
			if unlikely(!last_multi) {
				--self->a_data.ad_multi.am_count;
				DeeAst_Destroy(self);
				return NULL;
			}
			self->a_data.ad_multi.am_elem[self->a_data.ad_multi.am_count - 1] = last_multi;
			return self;
		}
		if (!Dee_AST_MULTI_KIND_ISSEQ(self->a_data.ad_multi.am_kind))
			break;

		/* Transform ever element of `self` into a delete-expression. */
		for (i = 0; i < self->a_data.ad_multi.am_count; ++i) {
			DeeAst *this_elem;
			this_elem = self->a_data.ad_multi.am_elem[i];
			this_elem = DeeAst_Transform_Del_(this_elem, flag, lexer, loc DBG__args);
			if unlikely(!this_elem) {
				DeeAst_Destroyv(self->a_data.ad_multi.am_elem, i);
				++i;
				DeeAst_Destroyv(self->a_data.ad_multi.am_elem + i,
				                self->a_data.ad_multi.am_count - i);
				_DeeAst_FiniCommon(self);
				DeeAst_Dbg_FreeUninitializedMulti(self, DBG_file, DBG_line);
				return NULL;
			}
			self->a_data.ad_multi.am_elem[i] = this_elem;
		}

		/* Convert from sequence-style multi to eval-to-last multi.
		 * Since all Del-style expression return `none`, by us evaluating
		 * to the return value of the our last AST, we will implicitly also
		 * evaluate to `none`! */
		self->a_data.ad_multi.am_kind = Dee_AST_MULTI_KIND_KEEPLAST;
		return self;
	}	break;

	case Dee_AST_OPERATOR:
		if (!Dee_ast_operator_id_hasid(&self->a_data.ad_operator.ao_id))
			break;
		switch (Dee_ast_operator_id_getid(&self->a_data.ad_operator.ao_id)) {
		case OPERATOR_GETATTR:
			Dee_ast_operator_id_setid(&self->a_data.ad_operator.ao_id, OPERATOR_DELATTR);
			return self;
		case OPERATOR_GETITEM:
			Dee_ast_operator_id_setid(&self->a_data.ad_operator.ao_id, OPERATOR_DELITEM);
			return self;
		case OPERATOR_GETRANGE:
			Dee_ast_operator_id_setid(&self->a_data.ad_operator.ao_id, OPERATOR_DELRANGE);
			return self;
		default: break;
		}
		return self;

	case Dee_AST_SYMBOL_RD: {
		STATIC_ASSERT(offsetof(DeeAst, a_data.ad_symbol_rd.asrd_symbol) ==
		              offsetof(DeeAst, a_data.ad_symbol_del.asub_symbol));
		Dee_link_symbol_rd_dec(self->a_data.ad_symbol_del.asub_symbol);
		self->a_kind = Dee_AST_SYMBOL_DEL;
		Dee_link_symbol_del_inc(self->a_data.ad_symbol_del.asub_symbol);
		return self;
	}	break;

	case Dee_AST_SYMREF_RD: {
		STATIC_ASSERT(offsetof(DeeAst, a_data.ad_symref_rd.asrrd_sym) ==
		              offsetof(DeeAst, a_data.ad_symref_del.asrub_sym));
		self->a_kind = Dee_AST_SYMREF_DEL;
		self->a_data.ad_symref_del.asrub_flag = flag;
		return self;
	}	break;

	default: break;
	}
	return DeeAst_Transform_Del_dummy_(self, lexer, loc DBG__args);
}

PRIVATE ATTR_COLD ATTR_NOINLINE WUNUSED NONNULL((1, 2, 3, 4)) DeeAst *DCALL
DeeAst_Transform_Store_dummy_(/*inherit(always)*/ DeeAst *__restrict dst,
                              /*inherit(always)*/ DeeAst *__restrict value,
                              DeeLexer *__restrict lexer,
                              struct Dee_ast_loc *__restrict loc DeeAst__DBG_PARAMS) {
	/* Emit warning (using `dst` as location context) */
	if (DeeLexer_WarnfLoc(lexer, loc, TPP_W_ASM_STORE_TO_RVALUE))
		goto err;
	return DeeAst_TrySetLoc(DeeAst_NewOperator2_(OPERATOR_ASSIGN, dst, value DBG__args), loc);
err:
	DeeAst_Destroy(value);
	DeeAst_Destroy(dst);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_ast_unpack_wr_target_init_from_ast(struct Dee_ast_unpack_wr_target *__restrict self,
                                       /*inherit(always)*/ DeeAst *__restrict from,
                                       DeeLexer *__restrict lexer) {
	/* Inherit metadata. */
	self->aupwrt_loc    = from->a_loc;
	self->aupwrt_labels = from->a_labels;
	from->a_labels      = NULL; /* Steal */

	/* Convert AST into unpack-write-target */
	switch (from->a_kind) {
	case Dee_AST_CONST:
		/* Check for special case: `none` */
		if (DeeNone_Check(from->a_data.ad_const.ace_value)) {
			self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_NONE;
			DeeAst_Destroy(from);
			return 0;
		}
		break;

	case Dee_AST_SYMBOL_RD:
		self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_SYMBOL;
		self->aupwrt_data.aupwrtd_symbol = from->a_data.ad_symbol_rd.asrd_symbol;
		Dee_link_symbol_wr_inc(self->aupwrt_data.aupwrtd_symbol);
		Dee_link_symbol_incref(self->aupwrt_data.aupwrtd_symbol);
		DeeAst_Destroy(from);
		return 0;

	case Dee_AST_SYMREF_RD:
		self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_SYMREF;
		Dee_ast_symref_move(&self->aupwrt_data.aupwrtd_symref,
		                    &from->a_data.ad_symref_rd.asrrd_sym);
		_DeeAst_FiniCommon(from);
		_DeeAst_FreeUninitializedSymbolLike(from);
		return 0;

	case Dee_AST_OPERATOR:
		if (!Dee_ast_operator_id_hasid(&from->a_data.ad_operator.ao_id))
			break;
		switch (Dee_ast_operator_id_getid(&from->a_data.ad_operator.ao_id)) {

		case OPERATOR_GETATTR:
			if (from->a_data.ad_operator.ao_argc != 2)
				break;
			if (from->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
			                                        Dee_AST_OPERATOR_FLAG_VARARG))
				break; /* Not allowed */
			self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_SETATTR;
			self->aupwrt_data.aupwrtd_setattr.aupwrtsa_obj  = from->a_data.ad_operator.ao_argv[0]; /* Steal */
			self->aupwrt_data.aupwrtd_setattr.aupwrtsa_attr = from->a_data.ad_operator.ao_argv[1]; /* Steal */
			ASSERT(self->aupwrt_data.aupwrtd_setattr.aupwrtsa_obj);
			ASSERT(self->aupwrt_data.aupwrtd_setattr.aupwrtsa_attr);
			_DeeAst_FiniCommon(from);
			DeeAst_FreeUninitializedOperator(from);
			return 0;

		case OPERATOR_GETITEM:
			if (from->a_data.ad_operator.ao_argc != 2)
				break;
			if (from->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
			                                        Dee_AST_OPERATOR_FLAG_VARARG))
				break; /* Not allowed */
			self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_SETITEM;
			self->aupwrt_data.aupwrtd_setitem.aupwrtsi_obj  = from->a_data.ad_operator.ao_argv[0]; /* Steal */
			self->aupwrt_data.aupwrtd_setitem.aupwrtsi_item = from->a_data.ad_operator.ao_argv[1]; /* Steal */
			ASSERT(self->aupwrt_data.aupwrtd_setitem.aupwrtsi_obj);
			ASSERT(self->aupwrt_data.aupwrtd_setitem.aupwrtsi_item);
			_DeeAst_FiniCommon(from);
			DeeAst_FreeUninitializedOperator(from);
			return 0;

		case OPERATOR_GETRANGE:
			if (from->a_data.ad_operator.ao_argc != 3)
				break;
			if (from->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
			                                        Dee_AST_OPERATOR_FLAG_VARARG))
				break; /* Not allowed */
			self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_SETRANGE;
			self->aupwrt_data.aupwrtd_setrange.aupwrtsr_obj   = from->a_data.ad_operator.ao_argv[0]; /* Steal */
			self->aupwrt_data.aupwrtd_setrange.aupwrtsr_start = from->a_data.ad_operator.ao_argv[1]; /* Steal */
			self->aupwrt_data.aupwrtd_setrange.aupwrtsr_end   = from->a_data.ad_operator.ao_argv[2]; /* Steal */
			ASSERT(self->aupwrt_data.aupwrtd_setrange.aupwrtsr_obj);
			ASSERT(self->aupwrt_data.aupwrtd_setrange.aupwrtsr_start);
			ASSERT(self->aupwrt_data.aupwrtd_setrange.aupwrtsr_end);
			_DeeAst_FiniCommon(from);
			DeeAst_FreeUninitializedOperator(from);
			return 0;

		default: break;
		}
		break;

	default: break;
	}

	/* Fallback: use `Dee_AST_UNPACK_WR_TARGET_KIND_ASSIGN` */
	if (DeeLexer_WarnfAst(lexer, from, TPP_W_ASM_STORE_TO_RVALUE))
		goto err_from; /* Emit warning (using `from` as location context) */
	self->aupwrt_kind = Dee_AST_UNPACK_WR_TARGET_KIND_ASSIGN;
	self->aupwrt_data.aupwrtd_assign.aupwrtas_obj = from;
	return 0;
err_from:
	DeeAst_Destroy(from);
	return -1;
}

PRIVATE WUNUSED NONNULL((2, 3, 4, 5, 6)) DeeAst *DCALL
DeeAst_Transform_Store_multi_impl(size_t count,
                                  /*inherit(always)*/ DeeAst *const *elem,
                                  /*inherit(always)*/ DeeAst *__restrict value,
                                  DeeAst *__restrict orig_dst,
                                  DeeLexer *__restrict lexer,
                                  struct Dee_ast_loc *__restrict loc DeeAst__DBG_PARAMS) {
	size_t i;
	DeeAst *result = DeeAst_Dbg_MallocUninitializedUnpackWr(count, DBG_file, DBG_line);
	if unlikely(!result)
		goto err_elem;
	for (i = 0; i < count; ++i) {
		DeeAst *this_elem = elem[i];
		struct Dee_ast_unpack_wr_target *this_target;
		this_target = &result->a_data.ad_unpack_wr.aupwr_targetv[i];
		if (Dee_ast_unpack_wr_target_init_from_ast(this_target, this_elem, lexer))
			goto err_r_i;
	}
	_DeeAst_MoveCommon(result, Dee_AST_SYMREF_RD, orig_dst);
	result->a_data.ad_unpack_wr.aupwr_expr    = value;
	result->a_data.ad_unpack_wr.aupwr_targetc = count;
	DeeAst_Destroy(orig_dst);
	DeeAst_UpdateLoc(result, loc);
	return result;
err_r_i:
	DeeAst_Destroyv(elem + (i + 1), count - (i + 1));
	while (i) {
		struct Dee_ast_unpack_wr_target *this_target;
		--i;
		this_target = &result->a_data.ad_unpack_wr.aupwr_targetv[i];
		Dee_ast_unpack_wr_target_fini(this_target);
	}
	DeeAst_FreeUninitializedUnpackWr(result);
err:
	DeeAst_Destroy(orig_dst);
	DeeAst_Destroy(value);
	return NULL;
err_elem:
	DeeAst_Destroyv(elem, count);
	goto err;
}

/* Transform the given AST `dst` for an expression `<dst> = <value>`:
 * - Convert `Dee_AST_OPERATOR + OPERATOR_GETATTR` into `Dee_AST_OPERATOR + OPERATOR_SETATTR`
 * - Convert `Dee_AST_OPERATOR + OPERATOR_GETITEM` into `Dee_AST_OPERATOR + OPERATOR_SETITEM`
 * - Convert `Dee_AST_OPERATOR + OPERATOR_GETRANGE` into `Dee_AST_OPERATOR + OPERATOR_SETRANGE`
 * - Convert `Dee_AST_SYMBOL_RD` into `Dee_AST_SYMBOL_WR`
 * - Convert `Dee_AST_SYMREF_RD` into `Dee_AST_SYMREF_WR`
 * - Convert `Dee_AST_MULTI + Dee_AST_MULTI_KIND_ISSEQ()` into `Dee_AST_UNPACK_WR`
 * All other types of ASTs are warned about, then packed with `<dst> := <value>` */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) DeeAst *DCALL
DeeAst_Transform_Store_(/*inherit(always)*/ DeeAst *__restrict dst,
                        /*inherit(always)*/ DeeAst *__restrict value,
                        DeeLexer *__restrict lexer,
                        struct Dee_ast_loc *__restrict loc DeeAst__DBG_PARAMS) {
	DeeAst_UpdateLoc(dst, loc);
	switch (dst->a_kind) {

	case Dee_AST_MULTI: {
		DeeAst *result;
		if (Dee_AST_MULTI_KIND_ISLAST(dst->a_data.ad_multi.am_kind)) {
			/* Propagate into multi-last -> last-element */
			DeeAst *last_multi;
			if (dst->a_data.ad_multi.am_count == 0)
				break;
			last_multi = dst->a_data.ad_multi.am_elem[dst->a_data.ad_multi.am_count - 1];
			last_multi = DeeAst_Transform_Store_(dst, value, lexer, loc DBG__args);
			if unlikely(!last_multi) {
				--dst->a_data.ad_multi.am_count;
				DeeAst_Destroy(dst);
				return NULL;
			}
			dst->a_data.ad_multi.am_elem[dst->a_data.ad_multi.am_count - 1] = last_multi;
			return dst;
		}
		if (!Dee_AST_MULTI_KIND_ISSEQ(dst->a_data.ad_multi.am_kind))
			break;

		/* Replace with `Dee_AST_UNPACK_WR` */
		result = DeeAst_Transform_Store_multi_impl(dst->a_data.ad_multi.am_count,
		                                           dst->a_data.ad_multi.am_elem,
		                                           value, dst, lexer, loc DBG__args);
		/* `dst->a_data.ad_multi.am_elem` is always inherited,
		 * so only (and always) need to free common + ast itself. */
		_DeeAst_FiniCommon(dst);
		DeeAst_Dbg_FreeUninitializedMulti(dst, DBG_file, DBG_line);
		return result;
	}	break;

	case Dee_AST_OPERATOR:
		if (!Dee_ast_operator_id_hasid(&dst->a_data.ad_operator.ao_id))
			break;
		if (dst->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
		                                       Dee_AST_OPERATOR_FLAG_VARARG))
			break; /* Not allowed */
		switch (Dee_ast_operator_id_getid(&dst->a_data.ad_operator.ao_id)) {
		case OPERATOR_GETATTR:
			Dee_ast_operator_id_setid(&dst->a_data.ad_operator.ao_id, OPERATOR_SETATTR);
append_argument:
			if (dst->a_data.ad_operator.ao_argc < Dee_AST_OPERATOR_ARGC_MAX) {
				dst->a_data.ad_operator.ao_argv[dst->a_data.ad_operator.ao_argc] = value;
				++dst->a_data.ad_operator.ao_argc;
			} else {
				DeeAst_Destroy(value);
			}
			return dst;
		case OPERATOR_GETITEM:
			Dee_ast_operator_id_setid(&dst->a_data.ad_operator.ao_id, OPERATOR_SETITEM);
			goto append_argument;
		case OPERATOR_GETRANGE:
			Dee_ast_operator_id_setid(&dst->a_data.ad_operator.ao_id, OPERATOR_SETRANGE);
			goto append_argument;
		default: break;
		}
		break;

	case Dee_AST_SYMBOL_RD: {
		STATIC_ASSERT(offsetof(DeeAst, a_data.ad_symbol_rd.asrd_symbol) ==
		              offsetof(DeeAst, a_data.ad_symbol_wr.aswr_symbol));
		Dee_link_symbol_rd_dec(dst->a_data.ad_symbol_del.asub_symbol);
		dst->a_kind = Dee_AST_SYMBOL_WR;
		Dee_link_symbol_wr_inc(dst->a_data.ad_symbol_del.asub_symbol);
		dst->a_data.ad_symbol_wr.aswr_value = value;
		return dst;
	}	break;

	case Dee_AST_SYMREF_RD: {
		STATIC_ASSERT(offsetof(DeeAst, a_data.ad_symref_rd.asrrd_sym) ==
		              offsetof(DeeAst, a_data.ad_symref_wr.asrwr_sym));
		dst->a_kind = Dee_AST_SYMREF_WR;
		dst->a_data.ad_symref_wr.asrwr_value = value;
		return dst;
	}	break;

	default: break;
	}
	return DeeAst_Transform_Store_dummy_(dst, value, lexer, loc DBG__args);
}


/* Label functions */
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_AddGotoLabel_(/*inherit(always)*/ DeeAst *__restrict self, tpp_keyword const *__restrict name DeeAst__DBG_PARAMS);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_AddCaseLabel_(/*inherit(always)*/ DeeAst *__restrict self, /*inherit(always)*/ DeeAst *__restrict expr DeeAst__DBG_PARAMS);
INTERN WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_AddDefaultLabel_(/*inherit(always)*/ DeeAst *__restrict self DeeAst__DBG_PARAMS);

#undef DBG__args
#undef DBG_line
#undef DBG_file
#else /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

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
	ASSERT(DeeCompiler_LockReading());
	if likely(result) {
#ifdef CONFIG_AST_IS_STRUCT
		result->a_refcnt = 1;
#else /* CONFIG_AST_IS_STRUCT */
		DeeObject_InitStatic(result, &DeeAst_Type);
#endif /* !CONFIG_AST_IS_STRUCT */
		result->a_scope = current_scope;
		ast_loc_init_empty(&result->a_ddi);
		Dee_Incref(result->a_scope);
	}
	return result;
}
#else /* !NDEBUG */
INTERN WUNUSED DREF struct ast *DCALL ast_new(void) {
	DREF struct ast *result = ast_alloc();
#ifndef CONFIG_NO_THREADS
	ASSERT(DeeCompiler_LockReading());
#endif /* !CONFIG_NO_THREADS */
	if likely(result) {
		DeeObject_InitStatic(result, &DeeAst_Type);
		result->a_scope      = current_scope;
		result->a_ddi.l_file = NULL;
		Dee_Incref(result->a_scope);
	}
	return result;
}
#endif /* NDEBUG */

INTERN NONNULL((2)) struct ast *DFCALL
ast_setddi(struct ast *self, struct ast_loc *__restrict info) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL` for `ast`. */
	ASSERT_AST(self);
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	self->a_ddi = *info;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
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
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
done:
	return self;
}

INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_sethere(DeeLexer *lexer, /*inherit(always)*/DREF struct ast *self) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL` for `ast`. */
	ASSERT_AST(self);
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	if unlikely(DeeLexer_GetLoc(lexer, &self->a_ddi)) {
		ast_decref(self);
		self = NULL;
	}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	(void)lexer;
	if unlikely(self->a_ddi.l_file)
		TPPFile_Decref(self->a_ddi.l_file);
	loc_here(&self->a_ddi);
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
done:
	return self;
}

INTERN NONNULL((2)) struct ast *DFCALL
ast_putddi(struct ast *self, struct ast_loc *__restrict info) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL` for `ast`. */
	ASSERT_AST(self);
	if unlikely(!ast_loc_isempty(&self->a_ddi))
		goto done;
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	self->a_ddi = *info;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	if (tpp_is_reachable_file(info->l_file)) {
		self->a_ddi = *info;
	} else {
		loc_here(&self->a_ddi);
	}
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
done:
	return self;
}

INTERN WUNUSED NONNULL((1)) struct ast *DFCALL
ast_puthere(DeeLexer *lexer, struct ast *self) {
	if unlikely(!self)
		goto done; /* Special case: Ignore `NULL` for `ast`. */
	ASSERT_AST(self);
	if unlikely(!ast_loc_isempty(&self->a_ddi))
		goto done;
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	if unlikely(DeeLexer_GetLoc(lexer, &self->a_ddi)) {
		ast_decref(self);
		self = NULL;
	}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	(void)lexer;
	loc_here(&self->a_ddi);
	ASSERT(self->a_ddi.l_file);
	/* Keep a reference to the associated file (so we can later read its filename). */
	TPPFile_Incref(self->a_ddi.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
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
                     (Dee_operator_t operator_name, struct ast *binding)) {
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
                      /*inherit(on_success)*/ DREF struct ast **__restrict exprv)) {
	DREF struct ast *result;
	/* Prevent AST ambiguity by not allowing
	 * keep-last, single-element multiple ASTs.
	 * -> ... Because they're literally no-ops that would otherwise
	 *        confuse the optimizer into not detecting constant
	 *        expressions, as well as special behavior surrounding
	 *        `AST_EXPAND` expressions not being triggered. */
	if unlikely(exprc == 1 && flags == AST_FMULTIPLE_KEEPLAST &&
	            current_scope == exprv[0]->a_scope) {
		result = exprv[0]; /* Inherit reference. */
got_result:
		Dee_Free(exprv); /* We're supposed to ~inherit~ this on success. (So we simply discard it). */
		return result;
	}

	/* Prevent some more ambiguity when ZERO(0) expressions were passed. */
	if unlikely(exprc == 0) {
		switch (flags) {
		case AST_FMULTIPLE_KEEPLAST:
			result = ast_constexpr(Dee_None);
got_result_maybe:
			if unlikely(!result)
				goto err;
			goto got_result;
		case AST_FMULTIPLE_TUPLE:
			result = ast_constexpr(Dee_EmptyTuple);
			goto got_result_maybe;
		case AST_FMULTIPLE_GENERIC:
			result = ast_constexpr(Dee_EmptySeq);
			goto got_result_maybe;
		case AST_FMULTIPLE_GENERIC_SET:
			result = ast_constexpr(Dee_EmptySet);
			goto got_result_maybe;
		case AST_FMULTIPLE_GENERIC_MAP:
			result = ast_constexpr(Dee_EmptyMap);
			goto got_result_maybe;
		default: break;
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
	/* Allocate the catch-expression vector inherited by `a_try` upon success. */
	catchv = (struct catch_expr *)Dee_Mallocc(1, sizeof(struct catch_expr));
	if unlikely(!catchv)
		goto err;
	catchv[0].ce_code  = finally_expression;         /* Reference is incremented later. */
	catchv[0].ce_flags = Dee_EXCEPTION_HANDLER_FFINALLY; /* Regular, old finally-handler. */
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
#if 0 /* This causes problems with code such as `([foo]...)` being \
       * interpreted as though it was written as `(foo)`, rather  \
       * than as written as `(foo,)` */
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
	ASSERT_OBJECT_TYPE(&scope->bs_scope, &DeeBaseScope_Type);
	result = ast_new();
	if likely(result) {
		result->a_type             = AST_FUNCTION;
		result->a_function.f_code  = function_code;
		result->a_function.f_scope = scope;
		ast_incref(function_code);
		Dee_Incref(&scope->bs_scope);
		INIT_REF(result);
	}
	return result;
}

DEFINE_AST_GENERATOR(NONNULL((3)), ast_operator1,
                     (Dee_operator_t operator_name, uint16_t flags,
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
                     (Dee_operator_t operator_name, uint16_t flags,
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
                     (Dee_operator_t operator_name, uint16_t flags,
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
                     (Dee_operator_t operator_name, uint16_t flags,
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
		Dee_Incref(&base_scope->bs_scope);
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
		Dee_Incref(&base_scope->bs_scope);
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
                     (uint16_t flags, tpp_string *__restrict text,
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
		tpp_string_incref(text);
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

#if defined(CONFIG_AST_IS_STRUCT) || 1
#define ast_visit(x) ast_visit_impl(x, proc, arg)
PRIVATE NONNULL((1, 2)) void DCALL
ast_visit_impl(struct ast *__restrict self,
               Dee_visit_t proc, void *arg);
#else /* ... */
#define ast_visit(x) Dee_Visit(x)
#endif /* !... */


INTERN NONNULL((2)) void DCALL
visit_switch_cases(struct text_label *switch_cases,
                   Dee_visit_t proc, void *arg) {
	while (switch_cases) {
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
		ast_decrefv(self->a_multiple.m_astv, self->a_multiple.m_astc);
		Dee_Free(self->a_multiple.m_astv);
	}	break;

	case AST_TRY: {
		struct catch_expr *iter, *end;
		ast_decref(self->a_try.t_guard);
		iter = self->a_try.t_catchv;
		end  = iter + self->a_try.t_catchc;
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
		Dee_Decref(&self->a_label.l_base->bs_scope);
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
		/* Track the writes to output operands. */
		for (i = 0; i < self->a_assembly.as_num_o; ++i) {
			struct asm_operand *op = &self->a_assembly.as_opv[i];
			if (ASM_OPERAND_IS_INOUT(op)) {
				ast_decwriteonly(op->ao_expr);
			} else {
				ast_decwrite(op->ao_expr);
			}
		}
		iter = self->a_assembly.as_opv;
		end  = iter + (self->a_assembly.as_num_o + self->a_assembly.as_num_i);
		for (; iter < end; ++iter) {
			ASSERT(iter->ao_type);
			ASSERT(iter->ao_expr);
			tpp_string_decref(iter->ao_type);
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
		tpp_string_decref(self->a_assembly.as_text.at_text);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	}	break;

	//case AST_LOOPCTL:
	default: break;
	}
}


PRIVATE NONNULL((1, 2)) void DCALL
ast_visit_impl(struct ast *__restrict self,
               Dee_visit_t proc, void *arg) {
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
		Dee_Visit(&self->a_label.l_base->bs_scope);
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
	ASSERT(DeeCompiler_LockReading());
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
	DeeCompiler_DelItem(self);
	if (self->a_ddi.l_file)
		TPPFile_Decref(self->a_ddi.l_file);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ struct ast,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ast_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ast_visit_impl,
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
#endif /* !CONFIG_AST_IS_STRUCT */
#endif /* !CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_C */
