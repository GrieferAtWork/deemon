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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_TRAITS_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_TRAITS_C 1

#include <deemon/api.h>

#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/callable.h>
#include <deemon/cell.h>
#include <deemon/code.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

PRIVATE DeeTypeObject *generic_sequence_types[] = {
	&DeeList_Type,
	&DeeTuple_Type,
	&DeeString_Type,
	&DeeHashSet_Type,
	&DeeDict_Type,
	&DeeRoSet_Type,
	&DeeRoDict_Type,
};

PRIVATE bool DCALL
is_generic_sequence_type(DeeTypeObject *self) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(generic_sequence_types); ++i)
		if (generic_sequence_types[i] == self)
			return true;
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
is_defined_by_deemon_core(DeeTypeObject *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	return DeeModule_ContainsPointer(&DeeModule_Deemon, self);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	bool result;
	DREF DeeModuleObject *type_module;
	type_module = DeeType_GetModule(self);
	if unlikely(!type_module) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		return false;
	}
	result = type_module == &DeeModule_Deemon;
	Dee_Decref(type_module);
	return result;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
filter_builtin_deemon_types(/*inherit(always)*/ DREF DeeObject *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (!DeeType_Check(self))
		goto err_decref_self;
	if (DeeType_IsCustom(self))
		goto err_decref_self;
	if (!DeeModule_ContainsPointer(&DeeModule_Deemon, self))
		goto err_decref_self;
	/* Because it's builtin+non-custom, it mustn't be heap-allocated
	 * (but be allocated statically), so decref-ing it mustn't kill it */
	Dee_DecrefNokill(self);
	return (DeeTypeObject *)self;
err_decref_self:
	Dee_Decref(self);
	return NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	DREF DeeModuleObject *type_module;
	if (!DeeType_Check(self))
		goto err_decref_self;
	if (DeeType_IsCustom(self))
		goto err_decref_self;
	type_module = DeeType_GetModule((DeeTypeObject *)self);
	if unlikely(!type_module) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		goto err_decref_self;
	}
	Dee_Decref(type_module);

	/* Only propagate types from the builtin deemon module. */
	if (type_module != &DeeModule_Deemon)
		goto err_decref_self;

	/* Because it's builtin+non-custom, it mustn't be heap-allocated
	 * (but be allocated statically), so decref-ing it mustn't kill it */
	Dee_DecrefNokill(self);
	return (DeeTypeObject *)self;
err_decref_self:
	Dee_Decref(self);
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
eval_decl_ast_type(struct decl_ast *__restrict self) {
again:
	switch (self->da_type) {

	case DAST_CONST:
		if (DeeType_Check(self->da_const))
			return (DeeTypeObject *)self->da_const;
		break;

	case DAST_SYMBOL: {
		struct symbol *typesym = self->da_symbol;
		SYMBOL_INPLACE_UNWIND_ALIAS(typesym);
		switch (typesym->s_type) {

		case SYMBOL_TYPE_EXTERN:
			if ((typesym->s_extern.e_symbol->ss_flags & (MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FPROPERTY)) ==
			    /*                                   */ (MODSYM_FREADONLY | MODSYM_FCONSTEXPR)) {
				DREF DeeObject *typval;
				typval = DeeModule_GetAttrSymbol(typesym->s_extern.e_module,
				                                 typesym->s_extern.e_symbol);
				if (!typval) {
					DeeError_Handled(ERROR_HANDLED_RESTORE);
					return NULL;
				}
				return filter_builtin_deemon_types(typval);
			}
			break;

		case SYMBOL_TYPE_CONST:
			if (DeeType_Check(typesym->s_const))
				return (DeeTypeObject *)typesym->s_const;
			break;

		default:
			break;
		}
	}	break;

	case DAST_TUPLE:
		return &DeeTuple_Type;

	case DAST_SEQ:
		return &DeeSeq_Type;

	case DAST_FUNC:
		return &DeeCallable_Type;

	case DAST_MAP:
		return &DeeMapping_Type;

	case DAST_WITH:
		self = &self->da_with.w_cell[0];
		goto again;

	case DAST_ATTR: {
		DeeTypeObject *base;
		DREF DeeObject *attr;
		base = eval_decl_ast_type(self->da_attr.a_base);
		if (!base)
			break;
		attr = DeeObject_GetAttr(Dee_AsObject(base),
		                         Dee_AsObject(self->da_attr.a_name));
		if (!attr) {
			DeeError_Handled(ERROR_HANDLED_RESTORE);
			return NULL;
		}
		return filter_builtin_deemon_types(attr);
	}	break;

	default:
		break;
	}
	return NULL;
}


struct seqops {
	/* Opcodes are encoded in big-endian.
	 * When the mask 0xff00 is ZERO, the opcode is a single byte long. */
	DeeTypeObject *so_typ;    /* The deemon type for this sequence. */
	uint16_t       so_pck[2]; /* Pack - [0]: 8-bit; [1]: 16-bit; */
	uint16_t       so_cas;    /* Cast */
};

INTDEF struct seqops seqops_info[4];


/* Predict the typing of a given AST, or return NULL when unpredictable.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, this function always returns `NULL'.
 * @param: flags: Set of `AST_PREDICT_TYPE_F_*' */
INTERN WUNUSED NONNULL((1)) DeeTypeObject *DFCALL
ast_predict_type_ex(struct ast *__restrict self, unsigned int flags) {
	ASSERT_AST(self);
	/* When AST type prediction is disabled, always indicate unpredictable ASTs. */
	if (optimizer_flags & OPTIMIZE_FNOPREDICT)
		goto nope;
	switch (self->a_type) {

	case AST_CONSTEXPR:
		return Dee_TYPE(self->a_constexpr);

	case AST_MULTIPLE:
		if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
			if (!self->a_multiple.m_astc)
				return &DeeNone_Type;
			return ast_predict_type_ex(self->a_multiple.m_astv[self->a_multiple.m_astc - 1],
			                           flags);
		}
		if (flags & AST_PREDICT_TYPE_F_NOANNO) {
			/* Special case: the normal code generator is allowed to optimize
			 *               based on type annotation, such that:
			 * >> local a: Tuple;
			 * >> local b: List;
			 * >> local c = (a...); // Cast-to-tuple can be optimized away
			 * >> local d = (b...); // Cast-to-tuple must be retained
			 *
			 * So if the caller is now asking us about the type of `c' when
			 * not considering type annotations, we mustn't respect the type
			 * annotation of `a' */
			if (self->a_multiple.m_astc >= 1) {
				struct ast *e0 = self->a_multiple.m_astv[0];
				if (e0->a_type == AST_EXPAND) {
					DeeTypeObject *expected_type = seqops_info[self->a_flag & 3].so_typ;
					if ((expected_type == &DeeTuple_Type /* Immutable sequence type */ ||
					     !ast_predict_object_shared(e0->a_expand)) &&
					    (ast_predict_type(e0->a_expand) == expected_type)) {
						/* In this case, the code generator will have no produced a cast operator.
						 * As such, it is our job to return the type of the sequence *without*
						 * taking type annotations into account. */
						return ast_predict_type_noanno(e0->a_expand);
					}
				}
			}
		}
		if (self->a_flag == AST_FMULTIPLE_TUPLE)
			return &DeeTuple_Type;
		if (self->a_flag == AST_FMULTIPLE_LIST)
			return &DeeList_Type;
		if (self->a_flag == AST_FMULTIPLE_HASHSET)
			return &DeeHashSet_Type;
		if (self->a_flag == AST_FMULTIPLE_DICT)
			return &DeeDict_Type;
		if (self->a_flag == AST_FMULTIPLE_GENERIC_MAP)
			return &DeeMapping_Type;
		return &DeeSeq_Type; /* That's all we can guaranty. */

	case AST_LOOP:
	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
	case AST_LOOPCTL:
	case AST_SWITCH:
	case AST_ASSEMBLY:
		return &DeeNone_Type;

#if 0
	case AST_TRY:
		/* TODO: Predictable, but only when the guard and all
		 *       catch-handles share the same return type. */
		return ast_predict_type_ex(self->a_try.t_guard, flags);
#endif

	case AST_CONDITIONAL: {
		DeeTypeObject *tt_type;
		DeeTypeObject *ff_type;
		if (self->a_flag & AST_FCOND_BOOL)
			return &DeeBool_Type;
		tt_type = self->a_conditional.c_tt ? ast_predict_type_ex(self->a_conditional.c_tt, flags) : &DeeNone_Type;
		ff_type = self->a_conditional.c_ff ? ast_predict_type_ex(self->a_conditional.c_ff, flags) : &DeeNone_Type;
		if (tt_type == ff_type)
			return tt_type;
	}	break;

	case AST_SYM: {
		struct symbol *sym;
		/* Certain symbol classes always refer to specific object types. */
		sym = self->a_sym;
		SYMBOL_INPLACE_UNWIND_ALIAS(sym);
		switch (sym->s_type) {

		case SYMBOL_TYPE_MODULE:
		case SYMBOL_TYPE_MYMOD:
			return &DeeModule_Type;

		case SYMBOL_TYPE_MYFUNC:
			return &DeeFunction_Type;

		case SYMBOL_TYPE_ARG: {
			DeeBaseScopeObject *bscope;
			/* The type of the varargs-argument is always a tuple!
			 * This deduction is required to optimize:
			 * >> local x = (...);
			 * ASM:
			 * >>     push varargs
			 * >>     cast top, tuple
			 * >>     pop  local @x
			 * Into:
			 * >>     push varargs
			 * >>     pop  local @x
			 */
			bscope = self->a_scope->s_base;
			if (DeeBaseScope_IsVarargs(bscope, sym))
				return &DeeTuple_Type;
#if 0 /* Not necessarily... */
			if (DeeBaseScope_IsVarkwds(bscope, sym))
				return &DeeMapping_Type; /* {string: Object} */
#endif
		}	break;

		default: break;
		}
		if (!(flags & AST_PREDICT_TYPE_F_NOANNO))
			return eval_decl_ast_type(&sym->s_decltype);
	}	break;

	case AST_BOOL:
		return &DeeBool_Type;

	case AST_FUNCTION:
		return &DeeFunction_Type;

	case AST_OPERATOR:
		/* If the self-operator gets re-returned, it's type is the result type. */
		if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP)
			return ast_predict_type_ex(self->a_operator.o_op0, flags);
		if (self->a_operator.o_exflag & AST_OPERATOR_FVARARGS)
			break; /* XXX: Special handling? */
		/* TODO: When !AST_PREDICT_TYPE_F_NOANNO, predict the type of op0 and look
		 *       if its `tp_doc' makes any mention of operator return types. */
		switch (self->a_flag) {

		case OPERATOR_STR:
		case OPERATOR_REPR:
			return &DeeString_Type;

		case OPERATOR_COPY:
		case OPERATOR_DEEPCOPY:
			return ast_predict_type_ex(self->a_operator.o_op0, flags);

		case OPERATOR_DELITEM:
		case OPERATOR_DELATTR:
		case OPERATOR_DELRANGE:
			return &DeeNone_Type;

		case OPERATOR_SIZE: {
			DeeTypeObject *predict;
			predict = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (is_generic_sequence_type(predict))
				return &DeeInt_Type;
			if (predict == &DeeNone_Type)
				return &DeeNone_Type;
		}	break;

		case OPERATOR_ADD: {
			DeeTypeObject *predict;
			predict = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (predict == &DeeNone_Type || /* Always re-returns itself */
			    predict == &DeeInt_Type ||  /* int_add */
#ifdef CONFIG_HAVE_FPU
			    predict == &DeeFloat_Type || /* float_add */
#endif /* CONFIG_HAVE_FPU */
			    predict == &DeeString_Type || /* string_cat */
			    predict == &DeeBytes_Type ||  /* bytes_add */
			    predict == &DeeList_Type ||   /* list_add */
			    predict == &DeeTuple_Type     /* tuple_concat */
			) {
				return predict;
			}
		}	break;

		case OPERATOR_POS:
		case OPERATOR_NEG:
		case OPERATOR_SUB:
		case OPERATOR_MUL:
		case OPERATOR_DIV:
		case OPERATOR_POW: {
			DeeTypeObject *predict;
			predict = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (predict == &DeeNone_Type ||
#ifdef CONFIG_HAVE_FPU
			    predict == &DeeFloat_Type ||
#endif /* CONFIG_HAVE_FPU */
			    predict == &DeeInt_Type) {
				return predict;
			}
		}	break;

		case OPERATOR_INV:
		case OPERATOR_MOD:
		case OPERATOR_SHL:
		case OPERATOR_SHR:
		case OPERATOR_AND:
		case OPERATOR_OR:
		case OPERATOR_XOR: {
			DeeTypeObject *predict;
			predict = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (predict == &DeeNone_Type ||
			    predict == &DeeInt_Type) {
				return predict;
			}
		}	break;

		case OPERATOR_ASSIGN:
		case OPERATOR_MOVEASSIGN:
			return ast_predict_type_ex(self->a_operator.o_op1, flags);

		/* TODO: OPERATOR_GETATTR (by searching for and interpreting doc strings) */
		/* TODO: OPERATOR_CALL    (by searching for and interpreting doc strings) */

		case OPERATOR_EQ:
		case OPERATOR_NE:
		case OPERATOR_LO:
		case OPERATOR_LE:
		case OPERATOR_GR:
		case OPERATOR_GE: {
			DeeTypeObject *predict;
			predict = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (!predict)
				break;
			if (predict == &DeeNone_Type) {
				if (self->a_flag == OPERATOR_EQ ||
				    self->a_flag == OPERATOR_NE)
					return &DeeBool_Type;
				return &DeeNone_Type;
			}

			/* Assume that all types (other than none) that are defined by
			 * the deemon core return `bool' for their compare operators. */
			if (is_defined_by_deemon_core(predict)) {
				INTDEF DeeTypeObject SeqEachOperator_Type;
				INTDEF DeeTypeObject SeqEach_Type;
				if (predict == &SeqEachOperator_Type ||
				    predict == &SeqEach_Type)
					return NULL; /* Undefined! */

				return &DeeBool_Type;
			}

			/* TODO: When !AST_PREDICT_TYPE_F_NOANNO, look into `predict->tp_doc' */
		}	break;

		case OPERATOR_CONTAINS: {
			DeeTypeObject *sequence_type;
			sequence_type = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (is_generic_sequence_type(sequence_type))
				return &DeeBool_Type;
		}	break;

		case OPERATOR_SETITEM:
		case OPERATOR_SETATTR:
			return ast_predict_type_ex(self->a_operator.o_op2, flags);

		case OPERATOR_SETRANGE:
			return ast_predict_type_ex(self->a_operator.o_op3, flags);

		case OPERATOR_GETITEM:
		case OPERATOR_GETRANGE: {
			DeeTypeObject *sequence_type;
			sequence_type = ast_predict_type_ex(self->a_operator.o_op0, flags);
			if (sequence_type == &DeeString_Type)
				return &DeeString_Type;
		}	break;

		default: break;
		}
		break;

	case AST_ACTION:
		switch (self->a_flag & AST_FACTION_KINDMASK) {
#define ACTION(x) case x &AST_FACTION_KINDMASK:

		ACTION(AST_FACTION_STORE)
			return ast_predict_type_ex(self->a_action.a_act1, flags);

		ACTION(AST_FACTION_IN) {
			DeeTypeObject *sequence_type;
			sequence_type = ast_predict_type_ex(self->a_action.a_act1, flags);
			if (is_generic_sequence_type(sequence_type))
				return &DeeBool_Type;
		}	break;

		ACTION(AST_FACTION_AS)
		ACTION(AST_FACTION_SUPEROF)
			return &DeeSuper_Type;

		ACTION(AST_FACTION_RANGE)
			return &DeeSeq_Type;

		ACTION(AST_FACTION_PRINT)
		ACTION(AST_FACTION_PRINTLN)
			return &DeeNone_Type;

		ACTION(AST_FACTION_FPRINT)
		ACTION(AST_FACTION_FPRINTLN)
			return ast_predict_type_ex(self->a_action.a_act0, flags);

		ACTION(AST_FACTION_CELL0)
		ACTION(AST_FACTION_CELL1)
			return &DeeCell_Type;

		ACTION(AST_FACTION_TYPEOF)
		ACTION(AST_FACTION_CLASSOF)
			return &DeeType_Type;

		ACTION(AST_FACTION_IS)
		ACTION(AST_FACTION_ANY)
		ACTION(AST_FACTION_ALL)
		ACTION(AST_FACTION_BOUNDATTR)
		ACTION(AST_FACTION_BOUNDITEM)
		ACTION(AST_FACTION_SAMEOBJ)
		ACTION(AST_FACTION_DIFFOBJ)
			return &DeeBool_Type;

		ACTION(AST_FACTION_ASSERT)
		ACTION(AST_FACTION_ASSERT_M)
			return ast_predict_type_ex(self->a_action.a_act0, flags);

		default: break;
#undef ACTION
		}

	default:
		break;
	}
nope:
	return NULL;
}


/* Predict the reference count of a given AST at runtime (if predictable)
 * If not predictable, return `0' (which is never a valid reference count) */
INTERN WUNUSED NONNULL((1)) Dee_refcnt_t DFCALL
ast_predict_object_refcnt(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_MULTIPLE:
		if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
			if (!self->a_multiple.m_astc)
				goto nope;
			return ast_predict_object_refcnt(self->a_multiple.m_astv[self->a_multiple.m_astc - 1]);
		}
		if (self->a_multiple.m_astc == 0 && (self->a_flag == AST_FMULTIPLE_TUPLE ||
		                                     self->a_flag == AST_FMULTIPLE_GENERIC ||
		                                     self->a_flag == AST_FMULTIPLE_GENERIC_MAP))
			goto nope; /* These will generate to access global singletons (with unknown reference counts) */
		if (self->a_flag != AST_FMULTIPLE_TUPLE)
			return 1; /* Anything but tuples must be created on the spot. */
		if (self->a_multiple.m_astc == 1) {
			/* Tuples with at least 2 elements must be created on the spot. */
		} else {
			/* Special case for 1-element tuples.
			 * Here, `(foo...)' can get optimized when `foo' is already known to have tuple
			 * typing (when not considering type annotations), so if that optimization is
			 * done, then the resulting expression won't represent a new tuple, and we need
			 * to return the reference count of `foo'. */
			struct ast *e0 = self->a_multiple.m_astv[0];
			if (e0->a_type == AST_EXPAND) {
				if (ast_predict_type(e0->a_expand) == &DeeTuple_Type)
					return ast_predict_object_refcnt(e0->a_expand);
			}
		}
		return 1;

	case AST_TRY: {
		size_t i;
		Dee_refcnt_t guard;
		guard = ast_predict_object_refcnt(self->a_try.t_guard);
		if (guard == 0)
			goto nope;
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			if (ast_predict_object_refcnt(self->a_try.t_catchv[i].ce_code) != guard)
				goto nope;
		}
		return guard;
	}	break;

	case AST_CONDITIONAL: {
		Dee_refcnt_t result;
		if (self->a_flag & AST_FCOND_BOOL)
			goto nope; /* Evaluates to a boolean singleton (which has unknown refcnt) */
		if (!self->a_conditional.c_tt || !self->a_conditional.c_ff)
			goto nope; /* Possibly evaluates to `none' (which has unknown refcnt) */
		result = ast_predict_object_refcnt(self->a_conditional.c_tt);
		if (result == 0)
			goto nope;
		if (ast_predict_object_refcnt(self->a_conditional.c_ff) != result)
			goto nope;
		return result;
	}	break;

	case AST_ACTION: {
		switch (self->a_flag & AST_FACTION_KINDMASK) {
#define ACTION(x) case x &AST_FACTION_KINDMASK:

		ACTION(AST_FACTION_CELL0)
		ACTION(AST_FACTION_CELL1)
		ACTION(AST_FACTION_SUPEROF)
		ACTION(AST_FACTION_AS)
			return 1;

		default: break;
#undef ACTION
		}
	}	break;

	case AST_CLASS:
		return 1;

	default:
		break;
	}
nope:
	return 0;
}



INTERN WUNUSED NONNULL((1)) bool DCALL
ast_has_sideeffects(struct ast *__restrict self) {
	if (optimizer_flags & OPTIMIZE_FNOPREDICT)
		return true;
	switch (self->a_type) {

	case AST_CONSTEXPR:
		return false;

#ifndef CONFIG_SYMBOL_BND_HASEFFECT_IS_SYMBOL_GET_HASEFFECT
	case AST_BOUND:
		return symbol_bnd_haseffect(self->a_sym, self->a_scope);

#else /* !CONFIG_SYMBOL_BND_HASEFFECT_IS_SYMBOL_GET_HASEFFECT */
	case AST_BOUND:
#endif /* CONFIG_SYMBOL_BND_HASEFFECT_IS_SYMBOL_GET_HASEFFECT */
	case AST_SYM:
		return symbol_get_haseffect(self->a_sym, self->a_scope);

	case AST_MULTIPLE: {
		struct ast **iter, **end;
		/* No side-effects when no branch has any.
		 * NOTE: This is true for all types of multiple-asts. */
		end = (iter = self->a_multiple.m_astv) +
		      self->a_multiple.m_astc;
		for (; iter < end; ++iter) {
			if (ast_has_sideeffects(*iter))
				return true;
		}
		return false;
	}	break;

	case AST_CONDITIONAL:
		/* AST_CONDITIONAL (with none of the branches having side-effects) */
		return (ast_has_sideeffects(self->a_conditional.c_cond) ||
		        (self->a_conditional.c_tt &&
		         self->a_conditional.c_tt != self->a_conditional.c_cond &&
		         ast_has_sideeffects(self->a_conditional.c_tt)) ||
		        (self->a_conditional.c_ff &&
		         self->a_conditional.c_ff != self->a_conditional.c_cond &&
		         ast_has_sideeffects(self->a_conditional.c_ff)));

		/* TODO: `function' without references that could cause side-effects (aka. property refs) */

	case AST_ACTION:
		switch (self->a_flag) {

		case AST_FACTION_CELL0:
			return false;

		case AST_FACTION_CELL1:
		case AST_FACTION_TYPEOF:
		case AST_FACTION_CLASSOF:
		case AST_FACTION_SUPEROF:
			return ast_has_sideeffects(self->a_action.a_act0);

		case AST_FACTION_IS:
		case AST_FACTION_AS:
		case AST_FACTION_BOUNDATTR:
		case AST_FACTION_BOUNDITEM:
		case AST_FACTION_SAMEOBJ:
		case AST_FACTION_DIFFOBJ:
			return (ast_has_sideeffects(self->a_action.a_act0) ||
			        ast_has_sideeffects(self->a_action.a_act1));

		case AST_FACTION_RANGE:
			return (ast_has_sideeffects(self->a_action.a_act0) ||
			        ast_has_sideeffects(self->a_action.a_act1) ||
			        ast_has_sideeffects(self->a_action.a_act2));

		default: break;
		}
		break;

	default: break;
	}
	/* Fallback: anything we don't recognize explicitly has side-effects. */
	return true;
}


INTERN WUNUSED NONNULL((1)) int DCALL
ast_doesnt_return(struct ast *__restrict self,
                  unsigned int flags) {
	int temp;
	switch (self->a_type) {

		/* Some normal branches that always return normally. */
	case AST_CONSTEXPR:
	case AST_SYM:
	case AST_UNBIND:
	case AST_BOUND:
	case AST_FUNCTION: /* Function definitions always return normally. */
		goto does_return;

	case AST_OPERATOR_FUNC:
		if (!self->a_operator_func.of_binding)
			goto does_return;
		ATTR_FALLTHROUGH
	case AST_YIELD:
	case AST_BOOL:
	case AST_EXPAND:
		/* Simple single-branch wrappers that can return normally. */
		return ast_doesnt_return(self->a_yield, flags);

	case AST_CLASS: {
		size_t i;
		bool has_noreturn;
		has_noreturn = false;
		for (i = 0; i < 2; ++i) {
			if (!(&self->a_class.c_base)[i])
				continue;
			temp = ast_doesnt_return(self->a_operator_ops[i], flags);
			if (temp != 0) { /* doesn't return, or is unpredictable. */
				if (temp < 0)
					return temp;
				has_noreturn = true;
			}
		}
		/* Check class members. */
		for (i = 0; i < self->a_class.c_memberc; ++i) {
			temp = ast_doesnt_return(self->a_class.c_memberv[i].cm_ast, flags);
			if (temp != 0) { /* doesn't return, or is unpredictable. */
				if (temp < 0)
					return temp;
				has_noreturn = true;
			}
		}
		if (has_noreturn)
			goto doesnt_return;
		goto does_return;
	}	break;

	case AST_OPERATOR: {
		size_t i;
		bool has_noreturn;
		/* Assume simple, ordered operator execution (a, [b, [c, [d]]]),
		 * which is guarantied for any operator invocation. */
		has_noreturn = false;
		for (i = 0; i < 4; ++i) {
			if (!self->a_operator_ops[i])
				break;
			temp = ast_doesnt_return(self->a_operator_ops[i], flags);
			if (temp != 0) { /* doesn't return, or is unpredictable. */
				if (temp < 0)
					return temp;
				has_noreturn = true;
			}
		}
		if (has_noreturn)
			goto doesnt_return;
		goto does_return;
	}	break;

	case AST_ACTION:
		/* Actions behave similar to operators, but mustn't necessarily follow
		 * regular operator invocation behavior. That is why we keep of whitelist
		 * of known action behavior here to determine if they can actually return. */
		switch (self->a_flag & AST_FACTION_KINDMASK) {
#define ACTION(x) case x &AST_FACTION_KINDMASK:

		ACTION(AST_FACTION_STORE)
			/* Always allowed to return when destination is a static variable initializer. */
			if (self->a_action.a_act0->a_type == AST_SYM &&
			    self->a_action.a_act0->a_sym->s_type == SYMBOL_TYPE_STATIC &&
			    self->a_action.a_act0->a_sym->s_scope == self->a_action.a_act0->a_scope &&
			    self->a_action.a_act0->a_sym->s_decl.l_file == self->a_action.a_act0->a_ddi.l_file &&
			    self->a_action.a_act0->a_sym->s_decl.l_line == self->a_action.a_act0->a_ddi.l_line &&
			    self->a_action.a_act0->a_sym->s_decl.l_col == self->a_action.a_act0->a_ddi.l_col)
				return false;
			ATTR_FALLTHROUGH
		ACTION(AST_FACTION_CELL0)
		ACTION(AST_FACTION_CELL1)
		ACTION(AST_FACTION_TYPEOF)
		ACTION(AST_FACTION_CLASSOF)
		ACTION(AST_FACTION_SUPEROF)
		ACTION(AST_FACTION_PRINT)
		ACTION(AST_FACTION_PRINTLN)
		ACTION(AST_FACTION_FPRINT)
		ACTION(AST_FACTION_FPRINTLN)
		ACTION(AST_FACTION_RANGE)
		ACTION(AST_FACTION_IS)
		ACTION(AST_FACTION_IN)
		ACTION(AST_FACTION_AS)
		ACTION(AST_FACTION_MIN)
		ACTION(AST_FACTION_MAX)
		ACTION(AST_FACTION_SUM)
		ACTION(AST_FACTION_ANY)
		ACTION(AST_FACTION_ALL)
		ACTION(AST_FACTION_ASSERT)
		//ACTION(AST_FACTION_ASSERT_M) /* Assertion-with-message doesn't follow normal rules. */
		ACTION(AST_FACTION_BOUNDATTR)
		ACTION(AST_FACTION_BOUNDITEM)
		ACTION(AST_FACTION_SAMEOBJ)
		ACTION(AST_FACTION_DIFFOBJ) {
			size_t i;
			bool has_noreturn;
			/* Actions which follow regular operator execution rules. */
			has_noreturn = false;
			for (i = 0; i < (size_t)AST_FACTION_ARGC_GT(self->a_flag); ++i) {
				temp = ast_doesnt_return(self->a_operator_ops[i], flags);
				if (temp != 0) { /* doesn't return, or is unpredictable. */
					if (temp < 0)
						return temp;
					has_noreturn = true;
				}
			}
			if (has_noreturn)
				goto doesnt_return;
			goto does_return;
		}	break;

		default: break;
#undef ACTION
		}
		break;


	case AST_MULTIPLE: {
		size_t i;
		bool has_noreturn;
		/* We never return if we contain another branch that doesn't
		 * return, with no unpredictable branches anywhere. */
		has_noreturn = false;
		for (i = 0; i < self->a_multiple.m_astc; ++i) {
			temp = ast_doesnt_return(self->a_multiple.m_astv[i], flags);
			if (temp != 0) { /* doesn't return, or is unpredictable. */
				if (temp < 0)
					return temp;
				has_noreturn = true;
			}
		}
		if (has_noreturn)
			goto doesnt_return;
		goto does_return;
	}	break;

	case AST_THROW:
		/* In a catch-all statement, a throw expression always returns. */
		if (self->a_throw) {
			temp = ast_doesnt_return(self->a_throw, flags);
			if (temp != 0)
				return temp; /* Unpredictable, or doesn't return */
		}
		if (flags & AST_DOESNT_RETURN_FINCATCHALL)
			goto does_return;
		/* If there are catch-expression, but not a catch-all one,
		 * then we can't predict if this throw will return. */
		if (flags & AST_DOESNT_RETURN_FINCATCH)
			goto unpredictable;
		goto doesnt_return;

	case AST_LOOPCTL:
		/* When looking a a loop as a whole,
		 * loop-control statements do actually return. */
		if (flags & AST_DOESNT_RETURN_FINLOOP)
			goto does_return;
		goto doesnt_return;

	case AST_RETURN:
		/* Check if the return-expression is predictable. */
		if (self->a_return) {
			if (flags & AST_DOESNT_RETURN_FINCATCH &&
			    !ast_is_nothrow(self->a_return, true))
				goto does_return; /* The return-expression may throw an exception... */
			temp = ast_doesnt_return(self->a_return, flags);
			if (temp != 0) {
				if (temp < 0)
					return temp;
				return temp; /* Unpredictable, or doesn't return */
			}
		}
		goto doesnt_return;

	case AST_TRY: {
		bool has_returning_handler;
		size_t i;
		struct catch_expr *vec;
		if unlikely(!self->a_try.t_catchc)
			return ast_doesnt_return(self->a_try.t_guard, flags);
		has_returning_handler = false;
		vec                   = self->a_try.t_catchv;
		flags &= ~(AST_DOESNT_RETURN_FINCATCH |
		           AST_DOESNT_RETURN_FINCATCHALL);
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			bool is_returning_handler = true;
			if (vec[i].ce_mask) {
				temp = ast_doesnt_return(vec[i].ce_mask, flags);
				if (temp < 0)
					return temp;
				if (temp)
					is_returning_handler = false;
			}
			/* Check if this handler ever returns. */
			temp = ast_doesnt_return(vec[i].ce_code, flags);
			if (temp < 0)
				return temp;
			if (temp)
				is_returning_handler = false;
			if (is_returning_handler)
				has_returning_handler = true;
			if (!(vec[i].ce_flags & EXCEPTION_HANDLER_FFINALLY)) {
				flags |= AST_DOESNT_RETURN_FINCATCH;
				if (!vec[i].ce_mask)
					flags |= AST_DOESNT_RETURN_FINCATCHALL;
			}
		}
		/* test the guarded expression. */
		temp = ast_doesnt_return(self->a_try.t_guard, flags);
		if (temp < 0)
			return temp;
		if (temp) {
			/* The guarded expression doesn't return.
			 * If there are no handlers that ever return, the try-block doesn't either */
			if (!has_returning_handler)
				goto doesnt_return;
		}
		goto does_return;
	}	break;

	case AST_SWITCH: {
		bool has_noreturn;
		struct text_label *iter;
		has_noreturn = false;
		temp         = ast_doesnt_return(self->a_switch.s_expr, flags);
		if (temp < 0)
			return temp;
		if (temp)
			has_noreturn = true;
		flags |= AST_DOESNT_RETURN_FINLOOP; /* Switch overrides `break' */
		temp = ast_doesnt_return(self->a_switch.s_block, flags);
		if (temp < 0)
			return temp;
		if (temp)
			has_noreturn = true;
		/* Enumerate switch cases. */
		iter = self->a_switch.s_cases;
		for (; iter; iter = iter->tl_next) {
			temp = ast_doesnt_return(iter->tl_expr, flags);
			if (temp < 0)
				return temp;
			if (temp)
				has_noreturn = true;
		}
		if (has_noreturn)
			goto doesnt_return;
		goto does_return;
	}	break;

	case AST_GOTO:
		/* The simple never-return-branches */
		goto doesnt_return;

	case AST_ASSEMBLY: {
		size_t i;
		bool has_noreturn;
		/* Inspect user-assembly operands. */
		has_noreturn = false;
		for (i = 0; i < self->a_assembly.as_num_i +
		                self->a_assembly.as_num_o;
		     ++i) {
			temp = ast_doesnt_return(self->a_assembly.as_opv[i].ao_expr, flags);
			if (temp < 0)
				return temp;
			if (temp)
				has_noreturn = true;
		}
		if (has_noreturn)
			goto doesnt_return;
		if (self->a_flag & AST_FASSEMBLY_REACH) {
			if (self->a_flag & AST_FASSEMBLY_NORETURN)
				goto unpredictable_noreturn; /* doesn't return, but is reachable. */
			goto unpredictable;              /* Reachable user-assembly is unpredictable. */
		}
		if (self->a_flag & AST_FASSEMBLY_NORETURN)
			goto doesnt_return; /* If the user-assembly states that it doesn't return, then this ast doesn't either! */
		goto does_return;
	}	break;

	case AST_CONDITIONAL:
		/* Simple case: If the condition doesn't return, neither
		 *              do we if both conditions are predictable. */
		temp = ast_doesnt_return(self->a_conditional.c_cond, flags);
		if (temp) {
			if (temp < 0)
				return temp;
			if (self->a_conditional.c_tt) {
				temp = ast_doesnt_return(self->a_conditional.c_tt, flags);
				if (temp < 0)
					return temp;
			}
			if (self->a_conditional.c_ff) {
				temp = ast_doesnt_return(self->a_conditional.c_ff, flags);
				if (temp < 0)
					return temp;
			}
			goto doesnt_return;
		}
		/* Extended case: With both conditional branches existing,
		 *                if neither returns, we don't either. */
		if (self->a_conditional.c_tt) {
			int temp2;
			temp = ast_doesnt_return(self->a_conditional.c_tt, flags);
			if (temp < 0)
				return temp;
			if (!self->a_conditional.c_ff)
				goto does_return; /* Without a false-branch, the AST can potentially return. */
			temp2 = ast_doesnt_return(self->a_conditional.c_ff, flags);
			if (temp2 < 0)
				return temp2; /* Check if the false-branch is unpredictable. */
			/* If both the true- and false-branches don't return, but the
			 * condition does, then the entire AST won't return, either! */
			if (temp && temp2)
				goto doesnt_return;
			goto does_return;
		}
		if (self->a_conditional.c_ff) {
			temp = ast_doesnt_return(self->a_conditional.c_ff, flags);
			if (temp < 0)
				return temp;
			/* Without a true-branch, the AST can potentially return. */
			goto does_return;
		}
		break;

	default: break;
	}

	/* Default case: Any branch we don't recognize is unpredictable. */
unpredictable:
	return -1;
does_return:
	return 0;
doesnt_return:
	return 1;
unpredictable_noreturn:
	return -2;
}



INTERN WUNUSED NONNULL((1)) bool DCALL
ast_is_nothrow(struct ast *__restrict self, bool result_used) {
	switch (self->a_type) {

	case AST_CONSTEXPR:
		goto is_nothrow;

	case AST_SYM: {
		struct symbol *sym;
		/* Access to some symbols is nothrow. */
		sym = self->a_sym;
		SYMBOL_INPLACE_UNWIND_ALIAS(sym);
		/* Ref vars are static and never cause exceptions. */
		if (SYMBOL_MUST_REFERENCE(sym))
			goto is_nothrow;
		switch (sym->s_type) {

		case SYMBOL_TYPE_STATIC:
			/* Since static variables can never be unbound,
			 * accessing them can never cause any exceptions. */
			goto is_nothrow;

		case SYMBOL_TYPE_STACK:
			/* Stack-based symbols can never be unbound, so
			 * accessing them is always a nothrow operation. */
			goto is_nothrow;

		case SYMBOL_TYPE_ARG:
			/* Accessing non-variadic & non-optional argument symbols is nothrow.
			 * Only varargs themself can potentially cause exceptions
			 * when accessed at runtime.
			 * Additionally, the symbol must never be written to, because
			 * if it is, it gets turned into a local variable, which _can_
			 * cause exceptions when accessed. */
			if (SYMBOL_NWRITE(sym) == 0 &&
			    (sym->s_symid < current_basescope->bs_argc_min ||
			     (sym->s_symid < current_basescope->bs_argc_max &&
			      current_basescope->bs_default[sym->s_symid - current_basescope->bs_argc_min])))
				goto is_nothrow;
			break;

		case SYMBOL_TYPE_MODULE:
			/* Imported modules are static and never cause exceptions. */
			goto is_nothrow;

		case SYMBOL_TYPE_EXCEPT:
		case SYMBOL_TYPE_MYMOD:
		case SYMBOL_TYPE_MYFUNC:
		case SYMBOL_TYPE_THIS:
			/* These never cause exceptions, either. */
			goto is_nothrow;

		default: break;
		}
	}	break;

	case AST_BOUND:
		/* Checking if a symbol is bound is nothrow. */
		goto is_nothrow;

	case AST_MULTIPLE: {
		size_t i;
		/* If the result is being used, and the AST is something other than
		 * a keep-last-expression AST, then the sequence pack operation may
		 * cause an exception. */
		if (result_used &&
		    self->a_flag != AST_FMULTIPLE_KEEPLAST)
			goto is_not_nothrow;
		/* If this is a keep-last expression, or the result isn't being used,
		 * make sure that none of the contained expressions can cause exceptions. */
		for (i = 0; i < self->a_multiple.m_astc; ++i) {
			if (!ast_is_nothrow(self->a_multiple.m_astv[i],
			                    result_used && i == self->a_multiple.m_astc - 1))
				goto is_not_nothrow;
		}
		goto is_nothrow;
	}	break;

	case AST_RETURN:
		if (!self->a_return)
			return true;
		ATTR_FALLTHROUGH
	case AST_YIELD:
		return ast_is_nothrow(self->a_return, true);

#if 0
	case AST_TRY: {
		size_t i;
		/* A try-block is nothrow if: the guarded block is nothrow,
		 * or if a catch-all guard exists, who's handler is nothrow,
		 * and all . */
		if (ast_is_nothrow(self->a_try.t_guard, result_used))
			goto is_nothrow;
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			self->a_try.t_catchv[i].ce_code;
		}
	}	break;
#endif

	default: break;
	}
is_not_nothrow:
	return false;
is_nothrow:
	return true;
}


INTERN WUNUSED NONNULL((1)) int
(DCALL ast_get_boolean)(struct ast *__restrict self) {
	/* NOTE: Assume that other operations on constant
	 *       expressions have already been propagated. */
	if (self->a_type == AST_CONSTEXPR &&
	    !(optimizer_flags & OPTIMIZE_FNOPREDICT)) {
		int result = DeeObject_Bool(self->a_constexpr);
		if unlikely(result < 0)
			DeeError_Handled(ERROR_HANDLED_RESTORE);
		return result;
	}
	return -1;
}

INTERN WUNUSED NONNULL((1)) int
(DCALL ast_get_boolean_noeffect)(struct ast *__restrict self) {
	int result;
	result = ast_get_boolean(self);
	if (result >= 0 && ast_has_sideeffects(self))
		result = -1;
	return result;
}


INTERN WUNUSED NONNULL((1, 2)) bool
(DCALL ast_uses_symbol)(struct ast *__restrict self,
                        struct symbol *__restrict sym) {
	if (optimizer_flags & OPTIMIZE_FNOPREDICT)
		goto yup;
	switch (self->a_type) {

	case AST_SYM:
		if (self->a_flag)
			return symbol_uses_symbol_on_set(self->a_sym, sym);
		return symbol_uses_symbol_on_get(self->a_sym, sym);

	case AST_UNBIND:
		return symbol_uses_symbol_on_del(self->a_sym, sym);

	case AST_BOUND:
		return symbol_uses_symbol_on_bnd(self->a_sym, sym);

	case AST_MULTIPLE: {
		size_t i;
		for (i = 0; i < self->a_multiple.m_astc; ++i) {
			if (ast_uses_symbol(self->a_multiple.m_astv[i], sym))
				goto yup;
		}
	}	break;

	case AST_TRY: {
		struct catch_expr *iter, *end;
		end = (iter = self->a_try.t_catchv) +
		      self->a_try.t_catchc;
		for (; iter < end; ++iter) {
			if (iter->ce_mask &&
			    ast_uses_symbol(iter->ce_mask, sym))
				goto yup;
			if (ast_uses_symbol(iter->ce_code, sym))
				goto yup;
		}
	}	ATTR_FALLTHROUGH
	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
	case AST_BOOL:
	case AST_EXPAND:
	case AST_FUNCTION:
		if (self->a_return &&
		    ast_uses_symbol(self->a_return, sym))
			goto yup;
		break;

	case AST_CLASS: {
		size_t i;
		if (self->a_class.c_base &&
		    ast_uses_symbol(self->a_class.c_base, sym))
			goto yup;
		if (ast_uses_symbol(self->a_class.c_desc, sym))
			goto yup;
		if (symbol_uses_symbol_on_set(self->a_class.c_classsym, sym))
			goto yup;
		if (!(self->a_flag & AST_FCLASS_NOWRITESUPER)) {
			if (symbol_uses_symbol_on_set(self->a_class.c_supersym, sym))
				goto yup;
		}
		for (i = 0; i < self->a_class.c_memberc; ++i) {
			if (ast_uses_symbol(self->a_class.c_memberv[i].cm_ast, sym))
				goto yup;
		}
	}	break;

	case AST_OPERATOR:
		if (self->a_operator.o_op3 &&
		    ast_uses_symbol(self->a_operator.o_op3, sym))
			goto yup;
		ATTR_FALLTHROUGH
	case AST_LOOP:
	case AST_CONDITIONAL:
	case AST_ACTION:
		if (self->a_loop.l_loop &&
		    ast_uses_symbol(self->a_loop.l_loop, sym))
			goto yup;
		ATTR_FALLTHROUGH
	case AST_SWITCH:
		if (self->a_loop.l_iter &&
		    ast_uses_symbol(self->a_loop.l_iter, sym))
			goto yup;
		if (self->a_loop.l_elem &&
		    ast_uses_symbol(self->a_loop.l_elem, sym))
			goto yup;
		break;

	case AST_ASSEMBLY: {
		struct asm_operand *iter, *end;
		/* Assembly branches with the `AST_FASSEMBLY_MEMORY'
		 * flag set are assumed to use _any_ symbol. */
		if (self->a_flag & AST_FASSEMBLY_MEMORY)
			goto yup;
		end = (iter = self->a_assembly.as_opv) +
		      (self->a_assembly.as_num_i +
		       self->a_assembly.as_num_o);
		for (; iter < end; ++iter) {
			if (ast_uses_symbol(iter->ao_expr, sym))
				goto yup;
		}
	}	break;

	default: break;
	}
/*nope:*/
	return false;
yup:
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
ast_can_exchange(struct ast *__restrict first,
                 struct ast *__restrict second) {
	if (optimizer_flags & OPTIMIZE_FNOPREDICT)
		goto nope;

	/* Check if simple cases: When one of the branches
	 * has absolutely no side-effects, then it never
	 * matters in what order they appear. */
	if (first->a_type == AST_CONSTEXPR ||
	    second->a_type == AST_CONSTEXPR)
		goto yup;

	/* If one of the asts is a symbol and the other branch
	 * doesn't affect that symbol, then they can be exchanged
	 * as well.
	 * TODO: If both asts only read from the symbol, they could
	 *       still be exchanged! */
	switch (first->a_type) {
		/* TODO: Handling for AST_MULTIPLE */

	case AST_SYM:
	case AST_BOUND:
	case AST_UNBIND:
		if (!ast_uses_symbol(second, first->a_sym))
			goto yup;
		break;

	default: break;
	}

	switch (second->a_type) {

		/* TODO: Handling for AST_MULTIPLE */

	case AST_SYM:
	case AST_BOUND:
	case AST_UNBIND:
		if (!ast_uses_symbol(first, second->a_sym))
			goto yup;
		break;

	default: break;
	}

nope:
	return false;
yup:
	return true;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
ast_equal_impl(struct ast const *a,
               struct ast const *b) {
	if (a->a_type != b->a_type)
		goto ne;
	if (a->a_scope != b->a_scope) {
		/* TODO: If the scopes aren't identical, the branches may still
		 *       have the same meaning, dependent on the context:
		 *    >> if (foo()) {
		 *    >>     print x;
		 *    >> } else {
		 *    >>     // Different scope, but same meaning...
		 *    >>     print x;
		 *    >> }
		 */
		goto ne;
	}

	switch (a->a_type) {

	case AST_CONSTEXPR: {
		int temp;
		if (a->a_constexpr == b->a_constexpr)
			goto eq;
		if (Dee_TYPE(a->a_constexpr) != Dee_TYPE(b->a_constexpr))
			goto ne;
		temp = DeeObject_TryCompareEq(a->a_constexpr, b->a_constexpr);
		if (Dee_COMPARE_ISERR(temp))
			DeeError_Handled(ERROR_HANDLED_RESTORE);
		return Dee_COMPARE_ISEQ_NO_ERR(temp);
	}	break;

	case AST_SYM:
	case AST_UNBIND:
	case AST_BOUND:
		/* TODO: If the symbols aren't identical, the branches may still
		 *       have the same meaning, dependent on the context:
		 *    >> if (foo()) {
		 *    >>     local x = 20;
		 *    >>     print x;
		 *    >> } else {
		 *    >>     local x = 20; // Different symbol, but same meaning...
		 *    >>     print x;
		 *    >> }
		 */
		return a->a_sym == b->a_sym;

	case AST_MULTIPLE: {
		size_t i;
		if (a->a_flag != b->a_flag)
			goto ne;
		if (a->a_multiple.m_astc != b->a_multiple.m_astc)
			goto ne;
		for (i = 0; i < a->a_multiple.m_astc; ++i) {
			if (!ast_equal_impl(a->a_multiple.m_astv[i],
			                    b->a_multiple.m_astv[i]))
				goto ne;
		}
		goto eq;
	}	break;

	case AST_RETURN:
	case AST_THROW:
		if (!a->a_return)
			return b->a_return == NULL;
		if (!b->a_return)
			goto ne;
		ATTR_FALLTHROUGH;
	case AST_YIELD:
		return ast_equal_impl(a->a_return,
		                      b->a_return);

	case AST_TRY: {
		size_t i;
		if (a->a_try.t_catchc != b->a_try.t_catchc)
			goto ne;
		if (!ast_equal_impl(a->a_try.t_guard,
		                    b->a_try.t_guard))
			goto ne;
		for (i = 0; i < a->a_try.t_catchc; ++i) {
			struct catch_expr *ahand = &a->a_try.t_catchv[i];
			struct catch_expr *bhand = &b->a_try.t_catchv[i];
			if (ahand->ce_flags != bhand->ce_flags)
				goto ne;
			if (ahand->ce_mask) {
				if (!bhand->ce_mask)
					goto ne;
				if (!ast_equal_impl(ahand->ce_mask,
				                    bhand->ce_mask))
					goto ne;
			} else {
				if (bhand->ce_mask)
					goto ne;
			}
		}
	}	break;

	case AST_LOOPCTL:
		if (a->a_flag != b->a_flag)
			goto ne;
		break;

	default: goto ne;
	}
eq:
	return true;
ne:
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
ast_equal(struct ast const *a,
          struct ast const *b) {
	if (optimizer_flags & OPTIMIZE_FNOCOMPARE)
		return false;
	if (a == b)
		return true;
	return ast_equal_impl(a, b);
}



/* Check if a given ast `self' is, or contains a `goto' branch,
 * or a `break' / `continue' branch when `consider_loopctl' is true.
 * NOTE: `goto' branches found in inner functions are not considered here! */
INTERN WUNUSED NONNULL((1)) bool DCALL
ast_contains_goto(struct ast *__restrict self,
                  uint16_t consider_loopctl) {
	switch (self->a_type) {

	case AST_CONSTEXPR:
	case AST_SYM:
	case AST_UNBIND:
	case AST_BOUND:
	case AST_FUNCTION:
no:
		return false;

	case AST_MULTIPLE: {
		size_t i;
		for (i = 0; i < self->a_multiple.m_astc; ++i) {
			if (ast_contains_goto(self->a_multiple.m_astv[i], consider_loopctl))
				goto yes;
		}
		goto no;
	}

	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
	case AST_BOOL:
	case AST_EXPAND:
	case AST_OPERATOR_FUNC:
		if (!self->a_return)
			goto no;
		return ast_contains_goto(self->a_return, consider_loopctl);

	case AST_TRY: {
		size_t i;
		if (ast_contains_goto(self->a_try.t_guard, consider_loopctl))
			goto yes;
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			if (self->a_try.t_catchv[i].ce_mask &&
			    ast_contains_goto(self->a_try.t_catchv[i].ce_mask, consider_loopctl))
				goto yes;
			if (ast_contains_goto(self->a_try.t_catchv[i].ce_code, consider_loopctl))
				goto yes;
		}
		goto no;
	}	break;

	case AST_LOOP:
		if (self->a_loop.l_cond &&
		    ast_contains_goto(self->a_loop.l_cond, AST_CONTAINS_GOTO_CONSIDER_NONE))
			goto yes;
		if (self->a_loop.l_next &&
		    ast_contains_goto(self->a_loop.l_next, AST_CONTAINS_GOTO_CONSIDER_NONE))
			goto yes;
		if (self->a_loop.l_loop &&
		    ast_contains_goto(self->a_loop.l_loop, AST_CONTAINS_GOTO_CONSIDER_NONE))
			goto yes;
		goto no;

	case AST_LOOPCTL:
		if (self->a_flag == AST_FLOOPCTL_BRK
		    ? (consider_loopctl & AST_CONTAINS_GOTO_CONSIDER_BREAK)
		    : (consider_loopctl & AST_CONTAINS_GOTO_CONSIDER_CONTINUE))
			goto yes;
		goto no;

	case AST_CONDITIONAL:
		if (ast_contains_goto(self->a_conditional.c_cond, consider_loopctl))
			goto yes;
		if (self->a_conditional.c_tt &&
		    self->a_conditional.c_tt != self->a_conditional.c_cond &&
		    ast_contains_goto(self->a_conditional.c_tt, consider_loopctl))
			goto yes;
		if (self->a_conditional.c_ff &&
		    self->a_conditional.c_ff != self->a_conditional.c_cond &&
		    ast_contains_goto(self->a_conditional.c_ff, consider_loopctl))
			goto yes;
		goto no;

	case AST_OPERATOR:
		if (self->a_operator.o_op0) {
			if (ast_contains_goto(self->a_operator.o_op0, consider_loopctl))
				goto yes;
			if (self->a_operator.o_op1) {
				if (ast_contains_goto(self->a_operator.o_op1, consider_loopctl))
					goto yes;
				if (self->a_operator.o_op2) {
					if (ast_contains_goto(self->a_operator.o_op2, consider_loopctl))
						goto yes;
					if (self->a_operator.o_op3) {
						if (ast_contains_goto(self->a_operator.o_op3, consider_loopctl))
							goto yes;
					}
				}
			}
		}
		goto no;

	case AST_ACTION:
		switch (AST_FACTION_ARGC_GT(self->a_flag)) {

		case 3:
			if (ast_contains_goto(self->a_action.a_act0, consider_loopctl))
				goto yes;
			ATTR_FALLTHROUGH
		case 2:
			if (ast_contains_goto(self->a_action.a_act0, consider_loopctl))
				goto yes;
			ATTR_FALLTHROUGH
		case 1:
			if (ast_contains_goto(self->a_action.a_act0, consider_loopctl))
				goto yes;
			ATTR_FALLTHROUGH
		default: break;
		}
		goto no;

	case AST_CLASS: {
		size_t i;
		if (self->a_class.c_base &&
		    ast_contains_goto(self->a_class.c_base, consider_loopctl))
			goto yes;
		if (ast_contains_goto(self->a_class.c_desc, consider_loopctl))
			goto yes;
		for (i = 0; i < self->a_class.c_memberc; ++i) {
			if (ast_contains_goto(self->a_class.c_memberv[i].cm_ast, consider_loopctl))
				goto yes;
		}
	}	goto no;

	case AST_SWITCH: {
		struct text_label *iter;
		/* Don't consider `break', which appears as part of the switch-branch! */
		consider_loopctl &= ~AST_CONTAINS_GOTO_CONSIDER_BREAK;
		if (ast_contains_goto(self->a_switch.s_expr, consider_loopctl))
			goto yes;
		if (ast_contains_goto(self->a_switch.s_block, consider_loopctl))
			goto yes;
		/* Don't forget to check the case expressions, too. */
		iter = self->a_switch.s_cases;
		for (; iter; iter = iter->tl_next) {
			if (ast_contains_goto(iter->tl_expr, consider_loopctl))
				goto yes;
		}
	}	goto no;

	case AST_ASSEMBLY: {
		size_t i;
#if 0 /* Nope! - User-assembly should mark variables it uses manually! \
       * >> local x = "foobar";                                        \
       * >> __asm__("" : "+x" (x)); // This prevents `x' from being optimized away! */
		if (self->a_flag & AST_FASSEMBLY_NORETURN)
			goto yes;
#endif
		/* Check user-assembly operands. */
		for (i = 0; i < self->a_assembly.as_num_i + self->a_assembly.as_num_i; ++i) {
			if (ast_contains_goto(self->a_assembly.as_opv[i].ao_expr, consider_loopctl))
				goto yes;
		}
	}	goto no;

	default: break;
	}
	/* fallback: Any branch we don't recognize may point to a GOTO branch.
	 *        -> While we should be able to recognize everything, we still
	 *           got to be as future-proof as possible! */
yes:
	return true;
}


/* Check if a given ast `self' contains a return-statement. */
INTERN WUNUSED NONNULL((1)) bool DCALL
ast_contains_return(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_CONSTEXPR:
	case AST_SYM:
	case AST_UNBIND:
	case AST_BOUND:
	case AST_FUNCTION:
	case AST_LOOPCTL:
no:
		return false;

	case AST_MULTIPLE: {
		size_t i;
		for (i = 0; i < self->a_multiple.m_astc; ++i) {
			if (ast_contains_return(self->a_multiple.m_astv[i]))
				goto yes;
		}
		goto no;
	}

	case AST_RETURN:
		return true;

	case AST_YIELD:
	case AST_THROW:
	case AST_BOOL:
	case AST_EXPAND:
	case AST_OPERATOR_FUNC:
		if (!self->a_return)
			goto no;
		return ast_contains_return(self->a_return);

	case AST_TRY: {
		size_t i;
		if (ast_contains_return(self->a_try.t_guard))
			goto yes;
		for (i = 0; i < self->a_try.t_catchc; ++i) {
			if (self->a_try.t_catchv[i].ce_mask &&
			    ast_contains_return(self->a_try.t_catchv[i].ce_mask))
				goto yes;
			if (ast_contains_return(self->a_try.t_catchv[i].ce_code))
				goto yes;
		}
		goto no;
	}	break;

	case AST_LOOP:
		if (self->a_loop.l_cond &&
		    ast_contains_return(self->a_loop.l_cond))
			goto yes;
		if (self->a_loop.l_next &&
		    ast_contains_return(self->a_loop.l_next))
			goto yes;
		if (self->a_loop.l_loop &&
		    ast_contains_return(self->a_loop.l_loop))
			goto yes;
		goto no;

	case AST_CONDITIONAL:
		if (ast_contains_return(self->a_conditional.c_cond))
			goto yes;
		if (self->a_conditional.c_tt &&
		    self->a_conditional.c_tt != self->a_conditional.c_cond &&
		    ast_contains_return(self->a_conditional.c_tt))
			goto yes;
		if (self->a_conditional.c_ff &&
		    self->a_conditional.c_ff != self->a_conditional.c_cond &&
		    ast_contains_return(self->a_conditional.c_ff))
			goto yes;
		goto no;

	case AST_OPERATOR:
		if (self->a_operator.o_op0) {
			if (ast_contains_return(self->a_operator.o_op0))
				goto yes;
			if (self->a_operator.o_op1) {
				if (ast_contains_return(self->a_operator.o_op1))
					goto yes;
				if (self->a_operator.o_op2) {
					if (ast_contains_return(self->a_operator.o_op2))
						goto yes;
					if (self->a_operator.o_op3) {
						if (ast_contains_return(self->a_operator.o_op3))
							goto yes;
					}
				}
			}
		}
		goto no;

	case AST_ACTION:
		switch (AST_FACTION_ARGC_GT(self->a_flag)) {

		case 3:
			if (ast_contains_return(self->a_action.a_act0))
				goto yes;
			ATTR_FALLTHROUGH
		case 2:
			if (ast_contains_return(self->a_action.a_act0))
				goto yes;
			ATTR_FALLTHROUGH
		case 1:
			if (ast_contains_return(self->a_action.a_act0))
				goto yes;
			ATTR_FALLTHROUGH
		default: break;
		}
		goto no;

	case AST_CLASS: {
		size_t i;
		if (self->a_class.c_base &&
		    ast_contains_return(self->a_class.c_base))
			goto yes;
		if (ast_contains_return(self->a_class.c_desc))
			goto yes;
		for (i = 0; i < self->a_class.c_memberc; ++i) {
			if (ast_contains_return(self->a_class.c_memberv[i].cm_ast))
				goto yes;
		}
	}	goto no;

	case AST_SWITCH: {
		struct text_label *iter;
		if (ast_contains_return(self->a_switch.s_expr))
			goto yes;
		if (ast_contains_return(self->a_switch.s_block))
			goto yes;
		/* Don't forget to check the case expressions, too. */
		iter = self->a_switch.s_cases;
		for (; iter; iter = iter->tl_next) {
			if (ast_contains_return(iter->tl_expr))
				goto yes;
		}
	}	goto no;

	case AST_ASSEMBLY: {
		size_t i;
		if (self->a_flag & AST_FASSEMBLY_NORETURN)
			goto yes;

		/* Check user-assembly operands. */
		for (i = 0; i < self->a_assembly.as_num_i + self->a_assembly.as_num_i; ++i) {
			if (ast_contains_return(self->a_assembly.as_opv[i].ao_expr))
				goto yes;
		}
	}	goto no;

	default: break;
	}
	/* fallback: Any branch we don't recognize may point to a GOTO branch.
	 *        -> While we should be able to recognize everything, we still
	 *           got to be as future-proof as possible! */
yes:
	return true;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_TRAITS_C */
