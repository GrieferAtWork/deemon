/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_GENFASTER_C
#define GUARD_DEEMON_COMPILER_ASM_GENFASTER_C 1

#include <deemon/api.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/tuple.h>

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) bool DCALL
has_sequence_cast_constructor(DeeObject *__restrict type) {
	if (type == (DeeObject *)&DeeTuple_Type)
		goto yes;
	if (type == (DeeObject *)&DeeList_Type)
		goto yes;
	if (type == (DeeObject *)&DeeHashSet_Type)
		goto yes;
	if (type == (DeeObject *)&DeeDict_Type)
		goto yes;
	if (type == (DeeObject *)&DeeRoDict_Type)
		goto yes;
	if (type == (DeeObject *)&DeeRoSet_Type)
		goto yes;
	return false;
yes:
	return true;
}


INTERN WUNUSED NONNULL((1)) struct ast *DCALL
ast_strip_seqcast(struct ast *__restrict self) {
	for (;;) {
		if (self->a_type == AST_MULTIPLE &&
		    self->a_flag != AST_FMULTIPLE_KEEPLAST &&
		    self->a_multiple.m_astc == 1 &&
		    self->a_multiple.m_astv[0]->a_type == AST_EXPAND) {
			self = self->a_multiple.m_astv[0]->a_expand;
			continue;
		}
		if (self->a_type == AST_OPERATOR &&
		    self->a_flag == OPERATOR_CALL &&
		    self->a_operator.o_op1 &&
		    !(self->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS)) &&
		    self->a_operator.o_op0->a_type == AST_CONSTEXPR &&
		    has_sequence_cast_constructor(self->a_operator.o_op0->a_constexpr)) {
			struct ast *seq_args;
			seq_args = self->a_operator.o_op1;
			if (seq_args->a_type == AST_MULTIPLE &&
			    seq_args->a_flag != AST_FMULTIPLE_KEEPLAST &&
			    seq_args->a_multiple.m_astc == 1 &&
			    seq_args->a_multiple.m_astv[0]->a_type != AST_EXPAND) {
				self = seq_args->a_multiple.m_astv[0];
				continue;
			}
#if 0
			if (seq_args->a_type == AST_CONSTEXPR &&
			    DeeTuple_Check(seq_args->ast_constexpr) &&
			    DeeTuple_SIZE(seq_args->ast_constexpr) == 1) {
				/* XXX: Return the inner expression? */
			}
#endif
		}
		break;
	}
	return self;
}

INTERN WUNUSED NONNULL((1)) int
(DCALL ast_genasm_asp)(struct ast *__restrict self,
                       unsigned int gflags) {
	/* Strip away unnecessary sequence casts. */
	self = ast_strip_seqcast(self);
	/* Generate the expression. */
	/* TODO: If `ast' is AST_MULTIPLE, we should generate it as `AST_FMULTIPLE_GENERIC':
	 * >> for (local x: [a, b, c]) ...;
	 * This doesn't need to be a list and could be compiled as ...
	 * >> for (local x: { a, b, c }) ...;
	 * ... to speed up the program.
	 */
	return ast_genasm(self, gflags);
}

/* Same as `DeeRoSet_FromSequence()', but has special handling for when `self' is a Mapping */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromSequenceOrMappingForContains(DeeObject *__restrict self) {
	DREF DeeObject *keys, *result;
	if (!DeeMapping_Check(self))
		return DeeRoSet_FromSequence(self);

	/* `x in Mapping' checks if `x' is a key.
	 *
	 * But if we do `x in HashSet.Frozen(Mapping)', the we'd be
	 * checking if `x' is a tuple `(key, item)'
	 *
	 * As such, when checking if a key is apart of a constant
	 * mapping, we need to construct a set of that mapping's
	 * keys! */
	keys = DeeObject_InvokeMethodHint(map_keys, self);
	if unlikely(!keys)
		goto err;
	result = DeeRoSet_FromSequence(keys);
	Dee_Decref_likely(keys);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int
(DCALL ast_genasm_set)(struct ast *__restrict self,
                       unsigned int gflags) {
	/* Strip away unnecessary sequence-style expression casts. */
	self = ast_strip_seqcast(self);
	if (self->a_type == AST_CONSTEXPR) {
		/* The inner sequence is a constant expression.
		 * -> Compile it as a _RoSet object. */
		DREF DeeObject *inner_set;
		inner_set = DeeRoSet_FromSequenceOrMappingForContains(self->a_constexpr);
		if unlikely(!inner_set) {
restore_error:
			DeeError_Handled(ERROR_HANDLED_RESTORE);
			goto push_generic;
		}
		if (asm_allowconst(inner_set)) {
			/* Push the inner-set expression. */
			return asm_gpush_constexpr_inherited(inner_set);
		}

		/* The inner set-expression isn't allowed as a constant.
		 * Instead, try to generate it as a regular hash-set. */
		Dee_Decref_likely(inner_set);
		inner_set = DeeHashSet_FromSequence(self->a_constexpr);
		if unlikely(!inner_set)
			goto restore_error;
		return asm_gpush_constexpr_inherited(inner_set);
	}
	/* Generate the expression. */
push_generic:
	/* TODO: If `ast' is AST_MULTIPLE, we should generate it as `AST_FMULTIPLE_HASHSET' */
	return ast_genasm(self, gflags);
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENFASTER_C */
