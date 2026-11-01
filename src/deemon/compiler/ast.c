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

#include "../runtime/strings.h"
#include "../runtime/builtin.h"

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


/* Initialize "self" from "value", and emit a warning `TPP_W_ASM_STORE_TO_RVALUE`
 * if `Dee_AST_RVALUE_KIND_ASSIGN` needs to be used and `lexer != NULL`. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeAstRValue_FromAst(DeeAstRValue *__restrict self,
                     /*inherit(always)*/ DeeAst *__restrict value,
                     DeeLexer *lexer, enum Dee_ast_rvalue_mode mode) {
	/* Inherit metadata. */
	self->arv_loc    = value->a_loc;
	self->arv_labels = value->a_labels;
	value->a_labels  = NULL; /* Steal */

	/* Convert AST into unpack-write-target */
	switch (value->a_kind) {
	case Dee_AST_CONST:
		/* Check for special case: `none` */
		if (DeeNone_Check(value->a_data.ad_const.ace_value)) {
			self->arv_kind = Dee_AST_RVALUE_KIND_NONE;
			DeeAst_Destroy(value);
			return 0;
		}
		break;

	case Dee_AST_SYMBOL_RD:
		self->arv_kind = Dee_AST_RVALUE_KIND_SYMBOL;
		self->arv_data.arvd_symbol = value->a_data.ad_symbol_rd.asrd_symbol;
		if (mode & Dee_AST_RVALUE_MODE_RD)
			Dee_link_symbol_rd_inc(self->arv_data.arvd_symbol);
		if (mode & Dee_AST_RVALUE_MODE_WR)
			Dee_link_symbol_wr_inc(self->arv_data.arvd_symbol);
		Dee_link_symbol_incref(self->arv_data.arvd_symbol);
		DeeAst_Destroy(value);
		return 0;

	case Dee_AST_SYMREF:
		self->arv_kind = Dee_AST_RVALUE_KIND_SYMREF;
		Dee_ast_symref_move(&self->arv_data.arvd_symref,
		                    &value->a_data.ad_symref);
		_DeeAst_FiniCommon(value);
		DeeAst_FreeUninitializedSymref(value);
		return 0;

#if 0 /* Not necessary, because `Dee_AST_IOPERATOR` shouldn't be used for OPERATOR_GET(ATTR|ITEM|RANGE) */
	case Dee_AST_IOPERATOR:
		...;
		break;
#endif
	case Dee_AST_OPERATOR:
		if (!Dee_ast_operator_id_hasid(&value->a_data.ad_operator.ao_id))
			break;
		switch (Dee_ast_operator_id_getid(&value->a_data.ad_operator.ao_id)) {

		case OPERATOR_GETATTR:
			if (value->a_data.ad_operator.ao_argc != 2)
				break;
			if (value->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
			                                        Dee_AST_OPERATOR_FLAG_VARARG))
				break; /* Not allowed */
			self->arv_kind = Dee_AST_RVALUE_KIND_SETATTR;
			self->arv_data.arvd_setattr.arvsa_obj  = value->a_data.ad_operator.ao_argv[0]; /* Steal */
			self->arv_data.arvd_setattr.arvsa_attr = value->a_data.ad_operator.ao_argv[1]; /* Steal */
			ASSERT(self->arv_data.arvd_setattr.arvsa_obj);
			ASSERT(self->arv_data.arvd_setattr.arvsa_attr);
			_DeeAst_FiniCommon(value);
			DeeAst_FreeUninitializedOperator(value);
			return 0;

		case OPERATOR_GETITEM:
			if (value->a_data.ad_operator.ao_argc != 2)
				break;
			if (value->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
			                                        Dee_AST_OPERATOR_FLAG_VARARG))
				break; /* Not allowed */
			self->arv_kind = Dee_AST_RVALUE_KIND_SETITEM;
			self->arv_data.arvd_setitem.arvsi_obj  = value->a_data.ad_operator.ao_argv[0]; /* Steal */
			self->arv_data.arvd_setitem.arvsi_item = value->a_data.ad_operator.ao_argv[1]; /* Steal */
			ASSERT(self->arv_data.arvd_setitem.arvsi_obj);
			ASSERT(self->arv_data.arvd_setitem.arvsi_item);
			_DeeAst_FiniCommon(value);
			DeeAst_FreeUninitializedOperator(value);
			return 0;

		case OPERATOR_GETRANGE:
			if (value->a_data.ad_operator.ao_argc != 3)
				break;
			if (value->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
			                                        Dee_AST_OPERATOR_FLAG_VARARG))
				break; /* Not allowed */
			self->arv_kind = Dee_AST_RVALUE_KIND_SETRANGE;
			self->arv_data.arvd_setrange.arvsr_obj   = value->a_data.ad_operator.ao_argv[0]; /* Steal */
			self->arv_data.arvd_setrange.arvsr_start = value->a_data.ad_operator.ao_argv[1]; /* Steal */
			self->arv_data.arvd_setrange.arvsr_end   = value->a_data.ad_operator.ao_argv[2]; /* Steal */
			ASSERT(self->arv_data.arvd_setrange.arvsr_obj);
			ASSERT(self->arv_data.arvd_setrange.arvsr_start);
			ASSERT(self->arv_data.arvd_setrange.arvsr_end);
			_DeeAst_FiniCommon(value);
			DeeAst_FreeUninitializedOperator(value);
			return 0;

		default: break;
		}
		break;

	default: break;
	}

	/* Fallback: use `Dee_AST_RVALUE_KIND_ASSIGN` */
	if (lexer) {
		if (DeeLexer_WarnfAst(lexer, value, TPP_W_ASM_STORE_TO_RVALUE))
			goto err_from; /* Emit warning (using `from` as location context) */
	}
	self->arv_kind = Dee_AST_RVALUE_KIND_ASSIGN;
	self->arv_data.arvd_assign.arva_obj = value;
	return 0;
err_from:
	DeeAst_Destroy(value);
	return -1;
}

INTERN NONNULL((1)) void DCALL
DeeAstRValue_Fini(DeeAstRValue *__restrict self,
                  enum Dee_ast_rvalue_mode mode) {
	struct Dee_ast_labels *labels = self->arv_labels;
	if (labels)
		Dee_ast_labels_destroy(labels);
	switch (self->arv_kind) {
	case Dee_AST_RVALUE_KIND_NONE:
		break;
	case Dee_AST_RVALUE_KIND_SYMBOL:
		if (mode & Dee_AST_RVALUE_MODE_WR)
			Dee_link_symbol_wr_dec(self->arv_data.arvd_symbol);
		if (mode & Dee_AST_RVALUE_MODE_RD)
			Dee_link_symbol_rd_dec(self->arv_data.arvd_symbol);
		Dee_link_symbol_decref(self->arv_data.arvd_symbol);
		break;
	case Dee_AST_RVALUE_KIND_SYMREF:
		Dee_ast_symref_fini(&self->arv_data.arvd_symref);
		break;
	case Dee_AST_RVALUE_KIND_SETRANGE:
		DeeAst_Destroy(self->arv_data.arvd_setrange.arvsr_end);
		ATTR_FALLTHROUGH
	case Dee_AST_RVALUE_KIND_SETATTR:
	case Dee_AST_RVALUE_KIND_SETITEM:
		DeeAst_Destroy(self->arv_data.arvd_setattr.arvsa_attr);
		ATTR_FALLTHROUGH
	case Dee_AST_RVALUE_KIND_ASSIGN:
	case Dee_AST_RVALUE_KIND_SETTER:
		DeeAst_Destroy(self->arv_data.arvd_assign.arva_obj);
		break;
	default: __builtin_unreachable();
	}
}


/* Check if `self` can be used in `DeeAstRValue_FromAst()`
 * without triggering a `TPP_W_ASM_STORE_TO_RVALUE` warning. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeAst_IsRValue(DeeAst const *__restrict self) {
	switch (self->a_kind) {
	case Dee_AST_CONST: /* Check for special case: `none` */
		return DeeNone_Check(self->a_data.ad_const.ace_value);

	case Dee_AST_SYMBOL_RD:
	case Dee_AST_SYMREF:
		return true;

	case Dee_AST_IOPERATOR:
	case Dee_AST_OPERATOR:
		if (!Dee_ast_operator_id_hasid(&self->a_data.ad_operator.ao_id))
			break;
		if (self->a_data.ad_operator.ao_flag & (Dee_AST_OPERATOR_FLAG_POSTOP |
		                                        Dee_AST_OPERATOR_FLAG_VARARG))
			break; /* Not allowed */
		switch (Dee_ast_operator_id_getid(&self->a_data.ad_operator.ao_id)) {
		case OPERATOR_GETATTR:
		case OPERATOR_GETITEM:
			return self->a_data.ad_operator.ao_argc == 2;
		case OPERATOR_GETRANGE:
			return self->a_data.ad_operator.ao_argc == 3;
		default: break;
		}
		break;

	default: break;
	}
	return false;
}



INTERN void DCALL
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
#define STATIC_ASSERT_BINARY_COMPATIBLE(f1, f2)                                    \
		STATIC_ASSERT(offsetof(DeeAst, a_data.f1) == offsetof(DeeAst, a_data.f2)); \
		STATIC_ASSERT(sizeof(((DeeAst *)0)->a_data.f1) == sizeof(((DeeAst *)0)->a_data.f2))

	case Dee_AST_CONST:
	case Dee_AST_IMPORT_STAR: {
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_const, ad_import_star);
		Dee_Decref(self->a_data.ad_const.ace_value);
		DeeAst_Dbg_FreeUninitializedConst(self, DBG_file, DBG_line);
	}	break;

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
		Dee_ast_typedef_fini(&self->a_data.ad_function.af_return_typedef);
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
			DeeAstRValue *this_target;
			this_target = &self->a_data.ad_unpack_wr.aupwr_targetv[i];
			DeeAstRValue_Fini(this_target, Dee_AST_RVALUE_MODE_WR);
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

	case Dee_AST_IOPERATOR: {
#ifndef NDEBUG
		size_t i;
		for (i = 1; i < self->a_data.ad_ioperator.aio_argc; ++i) {
			ASSERTF(self->a_data.ad_ioperator.aio_xargv[i - 1] != NULL,
			        "Operand %" PRFuSIZ " should be non-`NULL`", i);
		}
		for (; i < Dee_AST_OPERATOR_ARGC_MAX; ++i) {
			ASSERTF(self->a_data.ad_ioperator.aio_xargv[i - 1] == NULL,
			        "Operand %" PRFuSIZ " should be `NULL`", i);
		}
#endif /* !NDEBUG */
		Dee_ast_operator_id_fini(&self->a_data.ad_ioperator.aio_id);
		DeeAst_Destroyv(self->a_data.ad_ioperator.aio_xargv,
		                self->a_data.ad_ioperator.aio_argc);
		DeeAstRValue_Fini(&self->a_data.ad_ioperator.aio_self, Dee_AST_RVALUE_MODE_RDWR);
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

	case Dee_AST_SYMBOL_WR:
	case Dee_AST_SYMBOL_XCH:
	case Dee_AST_SYMBOL_INIT: {
		DREF struct Dee_link_symbol *symbol;
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_wr.aswr_value, ad_symbol_xch.asxc_value);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_wr.aswr_value, ad_symbol_init.asin_value);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_wr, ad_symbol_xch);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_wr, ad_symbol_init);
		DeeAst_Destroy(self->a_data.ad_symbol_wr.aswr_value);
		symbol = self->a_data.ad_symbol_rd.asrd_symbol;
		switch (self->a_kind) {
		case Dee_AST_SYMBOL_WR:
			Dee_link_symbol_wr_dec(symbol);
			break;
		case Dee_AST_SYMBOL_XCH:
		case Dee_AST_SYMBOL_INIT:
			Dee_link_symbol_rdwr_dec(symbol);
			break;
		default: __builtin_unreachable();
		}
		Dee_link_symbol_decref(symbol);
		DeeAst_Dbg_FreeUninitializedSymbolWr(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_SYMBOL_RD:
	case Dee_AST_SYMBOL_BND:
	case Dee_AST_SYMBOL_DEL: {
		DREF struct Dee_link_symbol *symbol;
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd.asrd_symbol, ad_symbol_bnd.asbn_symbol);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd.asrd_symbol, ad_symbol_del.asub_symbol);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd, ad_symbol_bnd);
		STATIC_ASSERT_BINARY_COMPATIBLE(ad_symbol_rd, ad_symbol_del);
		symbol = self->a_data.ad_symbol_rd.asrd_symbol;
		switch (self->a_kind) {
		case Dee_AST_SYMBOL_RD:
		case Dee_AST_SYMBOL_BND:
			Dee_link_symbol_rd_dec(symbol);
			break;
		case Dee_AST_SYMBOL_DEL:
			Dee_link_symbol_wr_dec(symbol);
			break;
		default: __builtin_unreachable();
		}
		Dee_link_symbol_decref(symbol);
		DeeAst_Dbg_FreeUninitializedSymbolRd(self, DBG_file, DBG_line);
	}	break;

	case Dee_AST_SYMREF:
		Dee_ast_symref_fini(&self->a_data.ad_symref);
		DeeAst_Dbg_FreeUninitializedSymref(self, DBG_file, DBG_line);
		break;

	case Dee_AST_SYMREF_ALIAS:
		Dee_ast_symref_fini(&self->a_data.ad_symref);
		DeeAst_Dbg_FreeUninitializedSymrefAlias(self, DBG_file, DBG_line);
		break;

	case Dee_AST_ASM_GOTO:
	case Dee_AST_GOTO:
	case Dee_AST_BREAK:
	case Dee_AST_CONTINUE:
		_DeeAst_Dbg_FreeUninitializedGotoLike(self, DBG_file, DBG_line);
		break;
	default: __builtin_unreachable();
#undef STATIC_ASSERT_BINARY_COMPATIBLE
	}
}

/* AST API */
INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewConst_(/*inherit(always)*/ DREF DeeObject *__restrict value DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedConst(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_CONST);
	result->a_data.ad_const.ace_value = value;
	return result;
err:
	Dee_Decref(value);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewConstString_(/*utf-8*/ char const *__restrict value,
                       size_t value_len DeeAst__DBG_PARAMS) {
	DREF DeeStringObject *string_value;
	string_value = (DREF DeeStringObject *)DeeDbgString_NewUtf8(value, value_len,
	                                                            STRING_ERROR_FIGNORE,
	                                                            DBG_file, DBG_line);
	if unlikely(!string_value)
		goto err;
	return DeeAst_NewConst_(Dee_AsObject(string_value) DBG__args);
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

/* Check if `self_arg.operator <opid>` can + should be compiled
 * as `Dee_AST_IOPERATOR` (as opposed to `Dee_AST_OPERATOR`) */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeAst_IsInplaceOperator(DeeAst const *__restrict self_arg,
                         struct Dee_ast_operator_id const *__restrict opid) {
	struct Dee_opinfo const *opinfo;
	if (!DeeAst_IsRValue(self_arg))
		return false;

	/* Compile as inplace if:
	 * - The AST allows for it
	 * - And the operator is either unknown, or is know to require it */
	if (!Dee_ast_operator_id_hasid(opid))
		return true;
	opinfo = DeeTypeType_GetOperatorById(&DeeType_Type, Dee_ast_operator_id_getid(opid));
	return !opinfo || (opinfo->oi_cc & OPCC_FINPLACE) != 0;
}

INTERN WUNUSED NONNULL((1, 3)) DeeAst *DCALL
DeeAst_NewOperatorEx1_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid,
                       Dee_ast_operator_flag flag,
                       /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS) {
	DeeAst *result;
	if (DeeAst_IsInplaceOperator(arg0, opid)) {
		result = DeeAst_Dbg_MallocUninitializedIOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg0;
		if (DeeAstRValue_FromAst(&result->a_data.ad_ioperator.aio_self, arg0,
		                         NULL, Dee_AST_RVALUE_MODE_RDWR)) {
			DeeAst_Dbg_FreeUninitializedIOperator(result, DBG_file, DBG_line);
			goto err_op;
		}
		_DeeAst_InitCommon(result, Dee_AST_IOPERATOR);
		result->a_data.ad_ioperator.aio_xargv[1 - 1] = NULL;
		result->a_data.ad_ioperator.aio_xargv[2 - 1] = NULL;
		result->a_data.ad_ioperator.aio_xargv[3 - 1] = NULL;
	} else {
		result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg0;
		_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
		result->a_data.ad_operator.ao_argv[0] = arg0;
		result->a_data.ad_operator.ao_argv[1] = NULL;
		result->a_data.ad_operator.ao_argv[2] = NULL;
		result->a_data.ad_operator.ao_argv[3] = NULL;
	}
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_id) == offsetof(DeeAst, a_data.ad_operator.ao_id));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_flag) == offsetof(DeeAst, a_data.ad_operator.ao_flag));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_argc) == offsetof(DeeAst, a_data.ad_operator.ao_argc));
	Dee_ast_operator_id_move(&result->a_data.ad_operator.ao_id, opid);
	result->a_data.ad_operator.ao_flag = flag;
	result->a_data.ad_operator.ao_argc = 1;
	return result;
err_op_arg0:
	DeeAst_Destroy(arg0);
err_op:
	Dee_ast_operator_id_fini(opid);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3, 4)) DeeAst *DCALL
DeeAst_NewOperator2Ex_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid,
                       Dee_ast_operator_flag flag,
                       /*inherit(always)*/ DeeAst *__restrict arg0,
                       /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS) {
	DeeAst *result;
	if (DeeAst_IsInplaceOperator(arg0, opid)) {
		result = DeeAst_Dbg_MallocUninitializedIOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg1_arg0;
		if (DeeAstRValue_FromAst(&result->a_data.ad_ioperator.aio_self, arg0,
		                         NULL, Dee_AST_RVALUE_MODE_RDWR)) {
			DeeAst_Dbg_FreeUninitializedIOperator(result, DBG_file, DBG_line);
			goto err_op_arg1;
		}
		_DeeAst_InitCommon(result, Dee_AST_IOPERATOR);
		result->a_data.ad_ioperator.aio_xargv[1 - 1] = arg1;
		result->a_data.ad_ioperator.aio_xargv[2 - 1] = NULL;
		result->a_data.ad_ioperator.aio_xargv[3 - 1] = NULL;
	} else {
		result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg1_arg0;
		_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
		result->a_data.ad_operator.ao_argv[0] = arg0;
		result->a_data.ad_operator.ao_argv[1] = arg1;
		result->a_data.ad_operator.ao_argv[2] = NULL;
		result->a_data.ad_operator.ao_argv[3] = NULL;
	}
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_id) == offsetof(DeeAst, a_data.ad_operator.ao_id));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_flag) == offsetof(DeeAst, a_data.ad_operator.ao_flag));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_argc) == offsetof(DeeAst, a_data.ad_operator.ao_argc));
	Dee_ast_operator_id_move(&result->a_data.ad_operator.ao_id, opid);
	result->a_data.ad_operator.ao_flag = flag;
	result->a_data.ad_operator.ao_argc = 2;
	return result;
err_op_arg1_arg0:
	DeeAst_Destroy(arg0);
err_op_arg1:
	DeeAst_Destroy(arg1);
/*err_op:*/
	Dee_ast_operator_id_fini(opid);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3, 4, 5)) DeeAst *DCALL
DeeAst_NewOperatorEx3_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid,
                       Dee_ast_operator_flag flag,
                       /*inherit(always)*/ DeeAst *__restrict arg0,
                       /*inherit(always)*/ DeeAst *__restrict arg1,
                       /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS) {
	DeeAst *result;
	if (DeeAst_IsInplaceOperator(arg0, opid)) {
		result = DeeAst_Dbg_MallocUninitializedIOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg2_arg1_arg0;
		if (DeeAstRValue_FromAst(&result->a_data.ad_ioperator.aio_self, arg0,
		                         NULL, Dee_AST_RVALUE_MODE_RDWR)) {
			DeeAst_Dbg_FreeUninitializedIOperator(result, DBG_file, DBG_line);
			goto err_op_arg2_arg1;
		}
		_DeeAst_InitCommon(result, Dee_AST_IOPERATOR);
		result->a_data.ad_ioperator.aio_xargv[1 - 1] = arg1;
		result->a_data.ad_ioperator.aio_xargv[2 - 1] = arg2;
		result->a_data.ad_ioperator.aio_xargv[3 - 1] = NULL;
	} else {
		result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg2_arg1_arg0;
		_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
		result->a_data.ad_operator.ao_argv[0] = arg0;
		result->a_data.ad_operator.ao_argv[1] = arg1;
		result->a_data.ad_operator.ao_argv[2] = arg2;
		result->a_data.ad_operator.ao_argv[3] = NULL;
	}
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_id) == offsetof(DeeAst, a_data.ad_operator.ao_id));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_flag) == offsetof(DeeAst, a_data.ad_operator.ao_flag));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_argc) == offsetof(DeeAst, a_data.ad_operator.ao_argc));
	Dee_ast_operator_id_move(&result->a_data.ad_operator.ao_id, opid);
	result->a_data.ad_operator.ao_flag = flag;
	result->a_data.ad_operator.ao_argc = 3;
	return result;
err_op_arg2_arg1_arg0:
	DeeAst_Destroy(arg0);
err_op_arg2_arg1:
	DeeAst_Destroy(arg1);
/*err_op_arg2:*/
	DeeAst_Destroy(arg2);
/*err_op:*/
	Dee_ast_operator_id_fini(opid);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((3, 4, 5, 6)) DeeAst *DCALL
DeeAst_NewOperatorEx4_(/*inherit(always)*/ struct Dee_ast_operator_id const *__restrict opid,
                       Dee_ast_operator_flag flag,
                       /*inherit(always)*/ DeeAst *__restrict arg0,
                       /*inherit(always)*/ DeeAst *__restrict arg1,
                       /*inherit(always)*/ DeeAst *__restrict arg2,
                       /*inherit(always)*/ DeeAst *__restrict arg3 DeeAst__DBG_PARAMS) {
	DeeAst *result;
	if (DeeAst_IsInplaceOperator(arg0, opid)) {
		result = DeeAst_Dbg_MallocUninitializedIOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg3_arg2_arg1_arg0;
		if (DeeAstRValue_FromAst(&result->a_data.ad_ioperator.aio_self, arg0,
		                         NULL, Dee_AST_RVALUE_MODE_RDWR)) {
			DeeAst_Dbg_FreeUninitializedIOperator(result, DBG_file, DBG_line);
			goto err_op_arg3_arg2_arg1;
		}
		_DeeAst_InitCommon(result, Dee_AST_IOPERATOR);
		result->a_data.ad_ioperator.aio_xargv[1 - 1] = arg1;
		result->a_data.ad_ioperator.aio_xargv[2 - 1] = arg2;
		result->a_data.ad_ioperator.aio_xargv[3 - 1] = arg3;
	} else {
		result = DeeAst_Dbg_MallocUninitializedOperator(DBG_file, DBG_line);
		if unlikely(!result)
			goto err_op_arg3_arg2_arg1_arg0;
		_DeeAst_InitCommon(result, Dee_AST_OPERATOR);
		result->a_data.ad_operator.ao_argv[0] = arg0;
		result->a_data.ad_operator.ao_argv[1] = arg1;
		result->a_data.ad_operator.ao_argv[2] = arg2;
		result->a_data.ad_operator.ao_argv[3] = arg3;
	}
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_id) == offsetof(DeeAst, a_data.ad_operator.ao_id));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_flag) == offsetof(DeeAst, a_data.ad_operator.ao_flag));
	STATIC_ASSERT(offsetof(DeeAst, a_data.ad_ioperator.aio_argc) == offsetof(DeeAst, a_data.ad_operator.ao_argc));
	Dee_ast_operator_id_move(&result->a_data.ad_operator.ao_id, opid);
	result->a_data.ad_operator.ao_flag = flag;
	result->a_data.ad_operator.ao_argc = 4;
	return result;
err_op_arg3_arg2_arg1_arg0:
	DeeAst_Destroy(arg0);
err_op_arg3_arg2_arg1:
	DeeAst_Destroy(arg1);
/*err_op_arg3_arg2:*/
	DeeAst_Destroy(arg2);
/*err_op_arg3:*/
	DeeAst_Destroy(arg3);
/*err_op:*/
	Dee_ast_operator_id_fini(opid);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((2)) DeeAst *DCALL
DeeAst_NewOperator1_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0 DeeAst__DBG_PARAMS) {
	struct Dee_ast_operator_id opid;
	Dee_ast_operator_id_initid(&opid, op);
	return DeeAst_NewOperatorEx1_(&opid, Dee_AST_OPERATOR_FLAG_NORMAL, arg0 DBG__args);
}

INTERN WUNUSED NONNULL((2, 3)) DeeAst *DCALL
DeeAst_NewOperator2_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0,
                     /*inherit(always)*/ DeeAst *__restrict arg1 DeeAst__DBG_PARAMS) {
	struct Dee_ast_operator_id opid;
	Dee_ast_operator_id_initid(&opid, op);
	return DeeAst_NewOperatorEx2_(&opid, Dee_AST_OPERATOR_FLAG_NORMAL, arg0, arg1 DBG__args);
}

INTERN WUNUSED NONNULL((2, 3, 4)) DeeAst *DCALL
DeeAst_NewOperator3_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0,
                     /*inherit(always)*/ DeeAst *__restrict arg1,
                     /*inherit(always)*/ DeeAst *__restrict arg2 DeeAst__DBG_PARAMS) {
	struct Dee_ast_operator_id opid;
	Dee_ast_operator_id_initid(&opid, op);
	return DeeAst_NewOperatorEx3_(&opid, Dee_AST_OPERATOR_FLAG_NORMAL, arg0, arg1, arg2 DBG__args);
}

INTERN WUNUSED NONNULL((2, 3, 4, 5)) DeeAst *DCALL
DeeAst_NewOperator4_(Dee_operator_t op,
                     /*inherit(always)*/ DeeAst *__restrict arg0,
                     /*inherit(always)*/ DeeAst *__restrict arg1,
                     /*inherit(always)*/ DeeAst *__restrict arg2,
                     /*inherit(always)*/ DeeAst *__restrict arg3 DeeAst__DBG_PARAMS) {
	struct Dee_ast_operator_id opid;
	Dee_ast_operator_id_initid(&opid, op);
	return DeeAst_NewOperatorEx4_(&opid, Dee_AST_OPERATOR_FLAG_NORMAL, arg0, arg1, arg2, arg3 DBG__args);
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
	DeeAst *result = DeeAst_Dbg_MallocUninitializedSymref(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF);
	Dee_ast_symref_init_byname(&result->a_data.ad_symref, name, kind, flag);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewSymRefNth_(tpp_keyword const *__restrict name,
                     Dee_ast_symref_nth_id nth_id DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedSymref(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF);
	Dee_ast_symref_init_nth(&result->a_data.ad_symref, name, nth_id);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewSymRefExtern_(/*inherit(always)*/ DREF struct Dee_module_object *__restrict mod,
                        struct Dee_module_symbol *__restrict sym DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedSymref(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF);
	Dee_ast_symref_init_extern(&result->a_data.ad_symref, mod, sym);
	return result;
err:
	Dee_Decref(mod);
	return NULL;
}

INTERN WUNUSED DeeAst *DCALL
DeeAst_NewSymRefSimple_(enum Dee_ast_symref_kind kind DeeAst__DBG_PARAMS) {
	DeeAst *result = DeeAst_Dbg_MallocUninitializedSymref(DBG_file, DBG_line);
	if unlikely(!result)
		goto err;
	_DeeAst_InitCommon(result, Dee_AST_SYMREF);
	Dee_ast_symref_init_simple(&result->a_data.ad_symref, kind);
	return result;
err:
	return NULL;
}

/* Label functions */
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_AddGotoLabel_(/*inherit(always)*/ DeeAst *__restrict self, tpp_keyword const *__restrict name DeeAst__DBG_PARAMS);
INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL DeeAst_AddCaseLabel_(/*inherit(always)*/ DeeAst *__restrict self, /*inherit(always)*/ DeeAst *__restrict expr DeeAst__DBG_PARAMS);
INTERN WUNUSED NONNULL((1)) DeeAst *DCALL DeeAst_AddDefaultLabel_(/*inherit(always)*/ DeeAst *__restrict self DeeAst__DBG_PARAMS);



/* Misc. helpers... */
INTERN WUNUSED NONNULL((1)) DeeAst *DCALL
DeeAst_NewCall_(/*inherit(always)*/ DeeAst *__restrict func, size_t argc,
                /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS) {
	DeeAst *call_args = DeeAst_NewMulti_(Dee_AST_MULTI_KIND_TUPLE, argc, argv DBG__args);
	if unlikely(!call_args)
		goto err_func;
	return DeeAst_NewOperator2_(OPERATOR_CALL, func, call_args DBG__args);
err_func:
	DeeAst_Destroy(func);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewCallSymRefExtern_(/*inherit(always)*/ DREF struct Dee_module_object *__restrict mod,
                            struct Dee_module_symbol *__restrict sym,
                            size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS) {
	DeeAst *func = DeeAst_NewSymRefExtern_(mod, sym DBG__args);
	if unlikely(!func)
		goto err_argv;
	return DeeAst_NewCall_(func, argc, argv DBG__args);
err_argv:
	DeeAst_Destroyv(argv, argc);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewImportModule_(/*inherit(always)*/ DeeAst *__restrict module_name DeeAst__DBG_PARAMS) {
	DeeAst *import_argv[2];
	import_argv[0] = DeeAst_NewSymRefThisModule();
	if unlikely(!import_argv[0])
		goto err_module_name;
	import_argv[1] = module_name;
	return DeeAst_NewCallSymRefExtern_(DeeModule_NewDeemon(),
	                                   &deemon_modsym___import__,
	                                   2, import_argv DBG__args);
err_module_name:
	DeeAst_Destroy(module_name);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewSymRefExternByName_(/*utf-8*/char const *module_name,
                              /*utf-8*/char const *symbol_name,
                              DeeLexer *__restrict lexer DeeAst__DBG_PARAMS) {
	DeeAst *module_name_ast;
	DeeAst *module_import_ast;
	DeeAst *symbol_name_ast;
	DREF DeeModuleObject *mod = DeeLexer_OpenModule(lexer, module_name);
	if likely(ITER_ISOK(mod)) {
		struct Dee_module_symbol *sym = DeeLexer_GetModuleSymbol(lexer, mod, symbol_name);
		if (ITER_ISOK(sym))
			return DeeAst_NewSymRefExtern_(mod, sym DBG__args);

		/* Emit as `{Dee_AST_CONST:mod}.operator . ({Dee_AST_CONST:symbol_name})` */
		if unlikely(!sym)
			goto err_mod;
		module_import_ast = DeeAst_NewConst_(Dee_AsObject(mod) DBG__args);
	} else {
		if unlikely(!mod)
			goto err;
		module_name_ast = DeeAst_NewConstString_(module_name, strlen(module_name) DBG__args);
		if unlikely(!module_name_ast)
			goto err;
		/* Module not found -> emit a call to `deemon.import` */
		module_import_ast = DeeAst_NewImportModule_(module_name_ast DBG__args);
	}
	if unlikely(!module_import_ast)
		goto err;
	symbol_name_ast = DeeAst_NewConstString_(symbol_name, strlen(symbol_name) DBG__args);
	if unlikely(!symbol_name_ast)
		goto err_module_import_ast;
	return DeeAst_NewOperator2_(OPERATOR_GETATTR, module_import_ast, symbol_name_ast DBG__args);
err_module_import_ast:
	DeeAst_Destroy(module_import_ast);
	goto err;
err_mod:
	Dee_Decref(mod);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL /* import.deemon.InstanceMethod({method}, {thisarg}) */
DeeAst_NewInstanceMethod_(/*inherit(always)*/ DeeAst *__restrict method,
                          /*inherit(always)*/ DeeAst *__restrict thisarg DeeAst__DBG_PARAMS) {
	DeeAst *instance_method_args[2];
	instance_method_args[0] = method;
	instance_method_args[1] = thisarg;
	return DeeAst_NewCallSymRefExtern_(DeeModule_NewDeemon(), &deemon_modsym_InstanceMethod,
	                                   2, instance_method_args DBG__args);
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL
DeeAst_NewOperatorFunction_(/*inherit(always)*/ struct Dee_ast_operator_id *__restrict opid,
                            DeeLexer *__restrict lexer DeeAst__DBG_PARAMS) {
	DeeAst *operators_operator;
	DeeAst *constexpr_id;
	DREF DeeObject *const_id;
	Dee_operator_t id;
	struct Dee_opinfo const *opinfo;
	if (!Dee_ast_operator_id_hasid(opid)) {
		/* TODO: Support for named operators */
	}

	id = Dee_ast_operator_id_getid(opid);
	Dee_ast_operator_id_fini(opid);
	opinfo = DeeTypeType_GetOperatorById(&DeeType_Type, id);
	if (opinfo) /* `import.operators.{opinfo->oi_sname}` */
		return DeeAst_NewSymRefExternByName_(STR_operators, opinfo->oi_sname, lexer DBG__args);

	/* `import.deemon.InstanceMethod(import.operators.operator, {id})` */
	operators_operator = DeeAst_NewSymRefExternByName_(STR_operators, STR_operator, lexer DBG__args);
	if unlikely(!operators_operator)
		goto err;
	const_id = DeeInt_NEWU(id);
	if unlikely(!const_id)
		goto err_operators_operator;
	constexpr_id = DeeAst_NewConst_(const_id DBG__args);
	if unlikely(!constexpr_id)
		goto err_operators_operator;
	return DeeAst_NewInstanceMethod_(operators_operator, constexpr_id DBG__args);
err_operators_operator:
	DeeAst_Destroy(operators_operator);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL /* {obj}.operator . ({attr})({argv...}) */
DeeAst_NewCallAttr_(/*inherit(always)*/ DeeAst *__restrict obj,
                    /*inherit(always)*/ DeeAst *__restrict attr,
                    size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS) {
	DeeAst *getattr_ast;
	DeeAst *call_args = DeeAst_NewMulti_(Dee_AST_MULTI_KIND_TUPLE, argc, argv DBG__args);
	if unlikely(!call_args)
		goto err_obj_attr;
	getattr_ast = DeeAst_NewOperator2_(OPERATOR_GETATTR, obj, attr DBG__args);
	if unlikely(!getattr_ast)
		goto err_call_args;
	return DeeAst_NewOperator2_(OPERATOR_CALL, getattr_ast, call_args DBG__args);
err_call_args:
	DeeAst_Destroy(call_args);
	goto err;
err_obj_attr:
	DeeAst_Destroy(attr);
/*err_obj:*/
	DeeAst_Destroy(obj);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeAst *DCALL /* {obj}.operator . ({attr})({argv...}) */
DeeAst_NewCallAttrConst_(/*inherit(always)*/ DeeAst *__restrict obj,
                         /*inherit(always)*/ /*string*/ DREF DeeObject *__restrict attr,
                         size_t argc, /*inherit(always)*/ DeeAst *const *argv DeeAst__DBG_PARAMS) {
	DeeAst *attr_ast = DeeAst_NewConst_(attr DBG__args);
	if unlikely(!attr_ast)
		goto err_obj_argv;
	return DeeAst_NewCallAttr_(obj, attr_ast, argc, argv DBG__args);
err_obj_argv:
	DeeAst_Destroyv(argv, argc);
/*err_obj:*/
	DeeAst_Destroy(obj);
/*err:*/
	return NULL;
}


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
