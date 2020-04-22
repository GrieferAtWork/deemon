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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENASM_YIELD_C
#define GUARD_DEEMON_COMPILER_ASM_GENASM_YIELD_C 1

#include <deemon/api.h>
/**/

#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 3, 4)) int
(DCALL ast_genasm_yieldall_fastseq)(DeeObject *__restrict seqexpr, size_t seqlen,
                                    struct ast *__restrict seqexpr_ddi,
                                    struct ast *__restrict ddi) {
	size_t i;
	/* Now to push individual elements. */
	for (i = 0; i < seqlen; ++i) {
		DREF DeeObject *elem;
		elem = DeeFastSeq_GetItem(seqexpr, i);
		if (asm_allowconst(elem)) {
			/* Use a constant prefix. */
			int32_t cid;
			cid = asm_newconst(elem);
			Dee_Decref(elem);
			if unlikely(cid < 0)
				goto err;
			if (asm_putddi(ddi))
				goto err;
			if (asm_pconst((uint16_t)cid))
				goto err;
			if (asm_gyield_p())
				goto err;
			continue;
		}
		/* Must explicitly push+yield the constant. */
		if (asm_putddi(seqexpr_ddi)) {
err_elem:
			Dee_Decref(elem);
			goto err;
		}
		if unlikely(asm_gpush_constexpr(elem))
			goto err_elem;
		Dee_Decref(elem);
		if (asm_putddi(ddi))
			goto err;
		if (asm_gyield())
			goto err;
	}
	return 0;
err:
	return -1;
}

/* Yield the given `yieldexpr'. (keeps the stack-pointer unmodified) */
PRIVATE WUNUSED NONNULL((1, 2)) int
(DCALL ast_genasm_yield)(struct ast *__restrict yieldexpr,
                         struct ast *__restrict ddi) {
	if (yieldexpr->a_type == AST_EXPAND) {
		struct ast *expandexpr;
		/* By default, the compiler will encode a regular, old `yield foo;' as `yield (foo,)...;',
		 * so without special handling here, that would always be assembled as:
		 * >> push   @foo
		 * >> push   pack Tuple, #1
		 * >> iterself top
		 * >> yield  foreach, pop */
		expandexpr = yieldexpr->a_expand;
		if (expandexpr->a_type == AST_MULTIPLE) {
			if (AST_FMULTIPLE_ISSEQUENCE(expandexpr->a_flag)) {
				size_t i;
				for (i = 0; i < expandexpr->a_multiple.m_astc; ++i) {
					/* Yield the contained sequence expression. */
					if unlikely(ast_genasm_yield(expandexpr->a_multiple.m_astv[i], ddi))
						goto err;
				}
				goto done;
			}
		}
		if (expandexpr->a_type == AST_CONSTEXPR) {
			/* Yielding a constant expression via expand can happen when the ast-optimizer
			 * has optimized the contained expression into a sequence (such as a tuple).
			 * Doing this is actually a viable method of generating smaller code for large
			 * sequences. However, doing this always produces larger code for small sequences:
			 * >> 3a nn       push   @(10,)
			 * >> b9          iterself top
			 * >> 02          yield  foreach, pop
			 * Which could instead be encoded as:
			 * >> fc nn 01    yield  @10
			 *
			 * Even in cases where the sequence being expanded is larger, this method should
			 * still be used (for sequences of up to 8 elements), as this way of yielding
			 * sequence elements doesn't require the creation of an iterator at runtime, and
			 * as such can be executed faster.
			 * However, also respect `ASM_FOPTIMIZE_SIZE', and only perform this optimization
			 * for sequences of at most 1 element, as required assembly for anything larger
			 * would result is more bytecode. */
			size_t fastlen;
			DeeObject *seqexpr;
			seqexpr = expandexpr->a_constexpr;
			fastlen = DeeFastSeq_GetSize(seqexpr);
			if (fastlen != DEE_FASTSEQ_NOTFAST) {
				if (fastlen > 8)
					goto after_constexpr_expand;
				if ((current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) && fastlen > 1)
					goto after_constexpr_expand;
				if unlikely(ast_genasm_yieldall_fastseq(seqexpr, fastlen, expandexpr, ddi))
					goto err;
			} else {
				seqexpr = DeeTuple_FromSequence(seqexpr);
				if unlikely(!seqexpr)
					goto err;
				fastlen = DeeTuple_SIZE(seqexpr);
				if (fastlen > 8 ||
				    (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) && fastlen > 1) {
					Dee_Decref(seqexpr);
					goto after_constexpr_expand;
				}
				if unlikely(ast_genasm_yieldall_fastseq(seqexpr, fastlen, expandexpr, ddi)) {
					Dee_Decref(seqexpr);
					goto err;
				}
				Dee_Decref(seqexpr);
			}
			goto done;
		}
after_constexpr_expand:
		/* Special case: Must do a YIELDALL when the
		 *               expression is an expand-expression. */
		if (ast_genasm(expandexpr, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi))
			goto err;
		if (asm_giterself())
			goto err;
		if (asm_gyieldall())
			goto err;
		goto done;
	}
	/* Check if this yield can be encoded via instruction prefix. */
	if (yieldexpr->a_type == AST_SYM) {
		struct symbol *sym = yieldexpr->a_sym;
		if (asm_can_prefix_symbol_for_read(sym)) {
			if (asm_putddi(ddi))
				goto err;
			if (asm_gprefix_symbol_for_read(sym, yieldexpr))
				goto err;
			if (asm_gyield_p())
				goto err;
			goto done;
		}
	}
	/* Fallback: Yield the raw expression AST */
	if (ast_genasm(yieldexpr, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi))
		goto err;
	if (asm_gyield())
		goto err;
done:
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENASM_YIELD_C */
