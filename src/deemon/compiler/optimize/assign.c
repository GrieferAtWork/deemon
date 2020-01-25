/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_ASSIGN_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_ASSIGN_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/object.h>

DECL_BEGIN

INTDEF void DCALL ast_incwrite(struct ast *__restrict self);
INTDEF void DCALL ast_decwrite(struct ast *__restrict self);
INTDEF void DCALL ast_incwriteonly(struct ast *__restrict self);

INTERN int (DCALL ast_assign)(struct ast *__restrict self,
                              struct ast *__restrict other) {
	uint8_t buffer[(sizeof(struct ast) -
	                COMPILER_OFFSETOF(struct ast, a_type))];
	/* Use a temporary buffer for the variable portion of the AST.
	 * Until we actually assign `other' to `self', we initialize it as a shallow copy of `other'. */
#if defined(__INTELLISENSE__) || 1
	struct ast *temp = COMPILER_CONTAINER_OF(buffer, struct ast, a_type);
#else
#define temp COMPILER_CONTAINER_OF(buffer, struct ast, a_type)
#endif
	temp->a_type = other->a_type;
	temp->a_flag = other->a_flag;
	switch (other->a_type) {

	case AST_LOOP:
		if (other->a_flag & AST_FLOOP_FOREACH &&
		    other->a_loop.l_elem) {
			/* For an explanation, see AST_ACTION:AST_FACTION_STORE */
			ast_incwrite(other->a_loop.l_elem);
		}
		temp->a_loop.l_cond = other->a_loop.l_cond;
		temp->a_loop.l_iter = other->a_loop.l_iter;
		temp->a_loop.l_loop = other->a_loop.l_loop;
		ast_xincref(other->a_loop.l_cond);
		ast_xincref(other->a_loop.l_iter);
		ast_xincref(other->a_loop.l_loop);
		break;

	case AST_CLASS: {
		struct class_member *dst;
		size_t i;
		temp->a_class.c_memberc = other->a_class.c_memberc;
		dst = (struct class_member *)Dee_Malloc(temp->a_class.c_memberc *
		                                        sizeof(struct class_member));
		if unlikely(!dst)
			goto err;
		temp->a_class.c_base = other->a_class.c_base;
		temp->a_class.c_desc = other->a_class.c_desc;
		ast_xincref(temp->a_class.c_base);
		ast_incref(temp->a_class.c_desc);
		temp->a_class.c_classsym = other->a_class.c_classsym;
		temp->a_class.c_supersym = other->a_class.c_supersym;
		if (temp->a_class.c_classsym)
			SYMBOL_INC_NWRITE(temp->a_class.c_classsym);
		if (temp->a_class.c_supersym &&
		    !(temp->a_flag & AST_FCLASS_NOWRITESUPER))
			SYMBOL_INC_NWRITE(temp->a_class.c_supersym);
		temp->a_class.c_memberv = dst;
		/* Copy the member descriptor table. */
		for (i = 0; i < temp->a_class.c_memberc; ++i) {
			memcpy(&dst[i], &other->a_class.c_memberv[i],
			       sizeof(struct class_member));
			ast_incref(dst[i].cm_ast);
		}
	}	break;

	case AST_OPERATOR:
		if (OPERATOR_ISINPLACE(temp->a_flag))
			ast_incwriteonly(other->a_operator.o_op0);
		temp->a_operator.o_exflag = other->a_operator.o_exflag;
		temp->a_operator.o_op3    = other->a_operator.o_op3;
		ast_xincref(temp->a_operator.o_op3);
		ATTR_FALLTHROUGH
	case AST_CONDITIONAL:
do_xcopy_3:
		temp->a_operator.o_op2 = other->a_operator.o_op2;
		ast_xincref(temp->a_operator.o_op2);
		ATTR_FALLTHROUGH
	case AST_FUNCTION:
		temp->a_operator.o_op1 = other->a_operator.o_op1;
		ast_xincref(temp->a_operator.o_op1);
		ATTR_FALLTHROUGH
	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
	case AST_BOOL:
	case AST_EXPAND:
		temp->a_return = other->a_return;
		ast_xincref(temp->a_return);
		break;

	case AST_CONSTEXPR:
		temp->a_constexpr = other->a_constexpr;
		Dee_Incref(temp->a_constexpr);
		break;

	case AST_ACTION:
		/* NOTE: Must fix the symbol effect counters, but ignore any reference underflow
		 *       that will be fixed as soon as the given branch is destroyed.
		 *       This _MUST_ always happen unless the caller uses this function
		 *       improperly in order to duplicate a branch without the intent of
		 *       eventually destroying the source branch. */
		if ((other->a_flag & AST_FACTION_KINDMASK) ==
		    (AST_FACTION_STORE & AST_FACTION_KINDMASK))
			ast_incwrite(other->a_action.a_act0);
		goto do_xcopy_3;

	case AST_MULTIPLE: {
		DREF struct ast **iter, **end, **dst;
		temp->a_multiple.m_astc = other->a_multiple.m_astc;
		end = (iter = other->a_multiple.m_astv) + other->a_multiple.m_astc;
		dst = (DREF struct ast **)Dee_Malloc(temp->a_multiple.m_astc *
		                                     sizeof(DREF struct ast *));
		if unlikely(!dst)
			goto err;
		temp->a_multiple.m_astv = dst;
		for (; iter != end; ++iter, ++dst) {
			*dst = *iter;
			ast_incref(*dst);
		}
	}	break;

	case AST_TRY: {
		struct catch_expr *iter, *end, *dst;
		temp->a_try.t_guard  = other->a_try.t_guard;
		temp->a_try.t_catchc = other->a_try.t_catchc;
		ast_incref(temp->a_try.t_guard);
		end = (iter = other->a_try.t_catchv) + other->a_try.t_catchc;
		dst = (struct catch_expr *)Dee_Malloc(temp->a_try.t_catchc *
		                                      sizeof(struct catch_expr));
		if unlikely(!dst)
			goto err;
		temp->a_try.t_catchv = dst;
		for (; iter != end; ++iter, ++dst) {
			*dst = *iter;
			ast_xincref(dst->ce_mask);
			ast_incref(dst->ce_code);
		}
	}	break;

	case AST_SYM:
		ASSERT(other->a_sym);
		if (temp->a_flag) {
		case AST_UNBIND:
			ASSERT(SYMBOL_NWRITE(other->a_sym));
			SYMBOL_INC_NWRITE(other->a_sym);
		} else {
			ASSERT(SYMBOL_NREAD(other->a_sym));
			SYMBOL_INC_NREAD(other->a_sym);
		}
		temp->a_sym = other->a_sym;
		break;

	case AST_BOUND:
		ASSERT(other->a_sym);
		ASSERT(SYMBOL_NBOUND(other->a_sym));
		SYMBOL_INC_NBOUND(other->a_sym);
		temp->a_sym = other->a_sym;
		break;

	case AST_GOTO:
		ASSERT(other->a_goto.g_label);
		ASSERT(other->a_goto.g_label->tl_goto);
		temp->a_goto.g_label = other->a_goto.g_label;
		++other->a_goto.g_label->tl_goto;
		temp->a_goto.g_base = other->a_goto.g_base;
		Dee_Incref((DeeObject *)temp->a_goto.g_base);
		break;

	case AST_LABEL:
		ASSERT(other->a_label.l_label);
		temp->a_label.l_label = other->a_label.l_label;
		temp->a_label.l_base  = other->a_label.l_base;
		Dee_Incref((DeeObject *)temp->a_label.l_base);
		break;

#if 0
	case AST_ASSEMBLY:
		break;

	case AST_SWITCH: /* Use the default case... */
		break;
#endif

	default:
		/* XXX: Couldn't we must always do a move-construction like this? */
		memcpy(buffer, &other->a_type, sizeof(buffer));
		memset(&other->a_type, 0, sizeof(buffer));
		other->a_type = AST_RETURN;
		break;
	}
	if (self != other) {
		/* Copy over DDI information. */
		if (other->a_ddi.l_file)
			TPPFile_Incref(other->a_ddi.l_file);
		if (self->a_ddi.l_file)
			TPPFile_Decref(self->a_ddi.l_file);
		memcpy(&self->a_ddi, &other->a_ddi, sizeof(struct ast_loc));
	}
	/* Finalize the contents of the AST that's being assigned to. */
	ast_fini_contents(self);
	/* Copy our temporary buffer into the actual, new AST. */
	memcpy(&self->a_type, buffer, sizeof(buffer));
	return 0;
err:
	return -1;
#undef temp
}

INTERN int (DCALL ast_graft_onto)(struct ast *__restrict self,
                                  struct ast *__restrict other) {
	DREF struct ast **elemv;
	if (self->a_scope == other->a_scope)
		return ast_assign(self, other);
	elemv = (struct ast **)Dee_Malloc(1 * sizeof(DREF struct ast *));
	if unlikely(!elemv)
		return -1;
	elemv[0] = other;
	ast_incref(other);
	if (self != other) {
		/* Copy over DDI information. */
		if (other->a_ddi.l_file)
			TPPFile_Incref(other->a_ddi.l_file);
		if (self->a_ddi.l_file)
			TPPFile_Decref(self->a_ddi.l_file);
		memcpy(&self->a_ddi, &other->a_ddi, sizeof(struct ast_loc));
	}
	ast_fini_contents(self);
	/* Override the (currently) invalid ast `self'. */
	self->a_type            = AST_MULTIPLE;
	self->a_flag            = AST_FMULTIPLE_KEEPLAST;
	self->a_multiple.m_astc = 1;
	self->a_multiple.m_astv = elemv;
	return 0;
}

INTERN struct ast *DCALL
ast_setscope_and_ddi(struct ast *self,
                     struct ast *__restrict src) {
	if unlikely(!self)
		return NULL;
	Dee_Incref(src->a_scope);
	Dee_Decref(self->a_scope);
	/* Override the effective scope of the AST. */
	self->a_scope = src->a_scope;
	/* Override the DDI information. */
	if (src->a_ddi.l_file)
		TPPFile_Incref(src->a_ddi.l_file);
	if (self->a_ddi.l_file)
		TPPFile_Decref(self->a_ddi.l_file);
	self->a_ddi = src->a_ddi;
	return self;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_ASSIGN_C */
