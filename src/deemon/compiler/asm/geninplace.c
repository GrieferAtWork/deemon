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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENINPLACE_C
#define GUARD_DEEMON_COMPILER_ASM_GENINPLACE_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
/**/

#include <stdint.h> /* uint16_t */

DECL_BEGIN

#define DO(expr) if unlikely(expr) goto err

/* Quick translation table for most basic operator instruction codes. */
INTDEF instruction_t const operator_instr_table[];

INTERN WUNUSED NONNULL((1, 3)) int DCALL
ast_gen_symbol_inplace(struct symbol *__restrict sym,
                       struct ast *operand,
                       struct ast *ddi_ast,
                       Dee_operator_t inplace_operator_name,
                       bool is_post_operator,
                       unsigned int gflags) {
	if (asm_can_prefix_symbol(sym)) {
		if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(sym, ddi_ast));
			DO(asm_gcopy());
		}
		if (operand) {
			DO(ast_genasm(operand, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gprefix_symbol(sym, ddi_ast));
			DO(asm_put(operator_instr_table[inplace_operator_name]));
			asm_decsp();
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gprefix_symbol(sym, ddi_ast));
			DO(asm_put(operator_instr_table[inplace_operator_name]));
		}
		/* Inplace operations return the symbol */
		if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
			DO(asm_gpush_symbol(sym, ddi_ast));
		}
		return 0;
	}

	if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
		DO(asm_gpush_symbol(sym, ddi_ast));
		DO(asm_gcopy()); /* sym, COPY(sym) */
	}
	DO(asm_gpush_symbol(sym, ddi_ast));
	if (operand) {
		DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* sym, operand */
		DO(asm_putddi(ddi_ast));
		DO(asm_pstack(current_assembler.a_stackcur - 2));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_ddicsp(); /* sym += operand */
	} else {
		DO(asm_pstack(current_assembler.a_stackcur - 1));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_dicsp(); /* ++sym */
	}
	if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
		DO(asm_gdup());
	}
	return asm_gpop_symbol(sym, ddi_ast);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
ast_gen_setattr_inplace(struct ast *__restrict base,
                        struct ast *__restrict name,
                        struct ast *operand,
                        struct ast *ddi_ast,
                        uint16_t inplace_operator_name,
                        bool is_post_operator,
                        unsigned int gflags) {
	/* <base>.<name> += operand; */
	ASSERT(OPERATOR_ISINPLACE(inplace_operator_name));
	if (name->a_type == AST_CONSTEXPR &&
	    DeeString_Check(name->a_constexpr)) {
		/* Special case: Name is a constant string. */
		int32_t cid = asm_newconst(name->a_constexpr);
		if unlikely(cid < 0)
			goto err;
		if (base->a_type == AST_SYM) {
			struct symbol *sym = SYMBOL_UNWIND_ALIAS(base->a_sym);
			if (sym->s_type == SYMBOL_TYPE_THIS &&
			    !SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) {
				DO(asm_ggetattr_this_const((uint16_t)cid)); /* this.name */
				if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
					DO(asm_gdup()); /* this.name, this.name */
					DO(asm_gcopy()); /* this.name, COPY(this.name) */
					DO(asm_gswap()); /* COPY(this.name), this.name */
				}
				if (operand) {
					DO(ast_genasm(operand, ASM_G_FPUSHRES)); /* this.name, operand */
					DO(asm_putddi(ddi_ast));
					DO(asm_pstack(current_assembler.a_stackcur - 2));
					DO(asm_put(operator_instr_table[inplace_operator_name]));
					asm_ddicsp(); /* this.name += operand */
				} else {
					DO(asm_pstack(current_assembler.a_stackcur - 1));
					DO(asm_put(operator_instr_table[inplace_operator_name]));
					asm_dicsp(); /* ++this.name */
				}
				if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
					DO(asm_gdup()); /* this.name += operand, this.name += operand */
				}
				DO(asm_gsetattr_this_const((uint16_t)cid)); /* . */
				goto done;
			}
		}
		DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
		DO(asm_putddi(ddi_ast));
		DO(asm_gdup()); /* base, base */
		DO(asm_ggetattr_const((uint16_t)cid)); /* base, base.name */
		if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
			DO(asm_gdup()); /* base, base.name, base.name */
			DO(asm_gcopy()); /* base, base.name, COPY(base.name) */
			DO(asm_grrot(3)); /* COPY(base.name), base, base.name */
		}
		if (operand) {
			DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, base.name, operand */
			DO(asm_putddi(ddi_ast));
			DO(asm_pstack(current_assembler.a_stackcur - 2));
			DO(asm_put(operator_instr_table[inplace_operator_name]));
			asm_ddicsp(); /* base, base.name += operand */
		} else {
			DO(asm_pstack(current_assembler.a_stackcur - 1));
			DO(asm_put(operator_instr_table[inplace_operator_name]));
			asm_dicsp(); /* base, ++base.name */
		}
		if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
			DO(asm_gdup()); /* base, base.name += operand, base.name += operand */
			DO(asm_grrot(3)); /* base.name += operand, base, base.name += operand */
		}
		DO(asm_gsetattr_const((uint16_t)cid)); /* . */
		goto done;
	}
	DO(ast_genasm(base, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup());
	DO(ast_genasm_one(name, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup()); /* base, base, name, name */
	DO(asm_grrot(3)); /* base, name, base, name */
	DO(asm_ggetattr()); /* base, name, base.name */
	if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
		DO(asm_gdup()); /* base, name, base.name, base.name */
		DO(asm_gcopy()); /* base, name, base.name, COPY(base.name) */
		DO(asm_grrot(4)); /* COPY(base.name), base, name, base.name */
	}
	if (operand) {
		DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, name, base.name, operand */
		DO(asm_putddi(ddi_ast));
		DO(asm_pstack(current_assembler.a_stackcur - 2));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_ddicsp(); /* base, name, base.name += operand */
	} else {
		DO(asm_pstack(current_assembler.a_stackcur - 1));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_dicsp(); /* base, name, ++base.name */
	}
	if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
		DO(asm_gdup()); /* base, name, base.name += operand, base.name += operand */
		DO(asm_grrot(4)); /* base.name += operand, base, name, base.name += operand */
	}
	DO(asm_gsetattr()); /* . */
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
ast_gen_setitem_inplace(struct ast *__restrict base,
                        struct ast *__restrict index,
                        struct ast *operand,
                        struct ast *ddi_ast,
                        Dee_operator_t inplace_operator_name,
                        bool is_post_operator,
                        unsigned int gflags) {
	/* <base>[<index>] += operand; */
	ASSERT(OPERATOR_ISINPLACE(inplace_operator_name));
	if (index->a_type == AST_CONSTEXPR) {
		/* Special case: The index is constant. */
		if (DeeInt_Check(index->a_constexpr)) {
			int32_t int_index;
			if (DeeInt_TryAsInt32(index->a_constexpr, &int_index) &&
			    int_index >= INT16_MIN && int_index <= INT16_MAX) {
				DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
				DO(asm_putddi(ddi_ast));
				DO(asm_gdup()); /* base, base */
				DO(asm_ggetitem_index((int16_t)int_index)); /* base, base[index] */
				if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
					DO(asm_gdup()); /* base, base[index], base[index] */
					DO(asm_gcopy()); /* base, base[index], COPY(base[index]) */
					DO(asm_grrot(3)); /* COPY(base[index]), base, base[index] */
				}
				if (operand) {
					DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, base[index], operand */
					DO(asm_putddi(ddi_ast));
					DO(asm_pstack(current_assembler.a_stackcur - 2));
					DO(asm_put(operator_instr_table[inplace_operator_name]));
					asm_ddicsp(); /* base, base[index] += operand */
				} else {
					DO(asm_pstack(current_assembler.a_stackcur - 1));
					DO(asm_put(operator_instr_table[inplace_operator_name]));
					asm_dicsp(); /* base, ++base[index] */
				}
				if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
					DO(asm_gdup()); /* base, base[index] += operand, base[index] += operand */
					DO(asm_grrot(3)); /* base[index] += operand, base, base[index] += operand */
				}
				DO(asm_gsetitem_index((int16_t)int_index)); /* . */
				goto done;
			}
		}
		if (asm_allowconst(index->a_constexpr)) {
			int32_t cid = asm_newconst(index->a_constexpr);
			if unlikely(cid < 0)
				goto err;
			DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* base, base */
			DO(asm_ggetitem_const((uint16_t)cid)); /* base, base[index] */
			if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
				DO(asm_gdup()); /* base, base[index], base[index] */
				DO(asm_gcopy()); /* base, base[index], COPY(base[index]) */
				DO(asm_grrot(3)); /* COPY(base[index]), base, base[index] */
			}
			if (operand) {
				DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, base[index], operand */
				DO(asm_putddi(ddi_ast));
				DO(asm_pstack(current_assembler.a_stackcur - 2));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_ddicsp(); /* base, base[index] += operand */
			} else {
				DO(asm_pstack(current_assembler.a_stackcur - 1));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_dicsp(); /* base, ++base[index] */
			}
			if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
				DO(asm_gdup()); /* base, base[index] += operand, base[index] += operand */
				DO(asm_grrot(3)); /* base[index] += operand, base, base[index] += operand */
			}
			DO(asm_gsetitem_const((uint16_t)cid)); /* . */
			goto done;
		}
	}
	DO(ast_genasm(base, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup());
	DO(ast_genasm_one(index, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup()); /* base, base, index, index */
	DO(asm_grrot(3)); /* base, index, base, index */
	DO(asm_ggetitem()); /* base, index, base[index] */
	if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
		DO(asm_gdup()); /* base, index, base[index], base[index] */
		DO(asm_gcopy()); /* base, index, base[index], COPY(base[index]) */
		DO(asm_grrot(4)); /* COPY(base[index]), base, index, base[index] */
	}
	if (operand) {
		DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, index, base[index], operand */
		DO(asm_putddi(ddi_ast));
		DO(asm_pstack(current_assembler.a_stackcur - 2));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_ddicsp(); /* base, index, base[index] += operand */
	} else {
		DO(asm_pstack(current_assembler.a_stackcur - 1));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_dicsp(); /* base, index, ++base[index] */
	}
	if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
		DO(asm_gdup()); /* base, index, base[index] += operand, base[index] += operand */
		DO(asm_grrot(4)); /* base[index] += operand, base, index, base[index] += operand */
	}
	DO(asm_gsetitem()); /* . */
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5)) int DCALL
ast_gen_setrange_inplace(struct ast *__restrict base,
                         struct ast *__restrict start,
                         struct ast *__restrict end,
                         struct ast *operand,
                         struct ast *ddi_ast,
                         Dee_operator_t inplace_operator_name,
                         bool is_post_operator,
                         unsigned int gflags) {
	/* <base>[<start>:<end>] += operand; */
	ASSERT(OPERATOR_ISINPLACE(inplace_operator_name));
	/* Special case: The start or end index is constant. */
	if (start->a_type == AST_CONSTEXPR) {
		if (DeeNone_Check(start->a_constexpr)) {
			if (end->a_type == AST_CONSTEXPR) {
				int32_t end_index;
				if (DeeInt_Check(end->a_constexpr) &&
				    DeeInt_TryAsInt32(end->a_constexpr, &end_index) &&
				    end_index >= INT16_MIN && end_index <= INT16_MAX) {
					/* <base>[none:43] += operand; (Uses `asm_ggetrange_ni') */
					DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
					DO(asm_putddi(ddi_ast));
					DO(asm_gdup()); /* base, base */
					DO(asm_ggetrange_ni((int16_t)end_index)); /* base, base[none:43] */
					if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
						DO(asm_gdup()); /* base, base[none:43], base[none:43] */
						DO(asm_gcopy()); /* base, base[none:43], COPY(base[none:43]) */
						DO(asm_grrot(3)); /* COPY(base[none:43]), base, base[none:43] */
					}
					if (operand) {
						DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, base[none:43], operand */
						DO(asm_putddi(ddi_ast));
						DO(asm_pstack(current_assembler.a_stackcur - 2));
						DO(asm_put(operator_instr_table[inplace_operator_name]));
						asm_ddicsp(); /* base, base[none:43] += operand */
					} else {
						DO(asm_pstack(current_assembler.a_stackcur - 1));
						DO(asm_put(operator_instr_table[inplace_operator_name]));
						asm_dicsp(); /* base, ++base[none:43] */
					}
					if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
						DO(asm_gdup()); /* base, base[none:43] += operand, base[none:43] += operand */
						DO(asm_grrot(3)); /* base[none:43] += operand, base, base[none:43] += operand */
					}
					DO(asm_gsetrange_ni((int16_t)end_index)); /* . */
					goto done;
				}
			}

			/* <base>[none:<end>] += operand; (Uses `asm_ggetrange_ip') */
			DO(ast_genasm(base, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup());
			DO(ast_genasm_one(end, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* base, base, end, end */
			DO(asm_grrot(3)); /* base, end, base, end */
			DO(asm_ggetrange_np()); /* base, end, base[none:end] */
			if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
				DO(asm_gdup()); /* base, end, base[none:end], base[none:end] */
				DO(asm_gcopy()); /* base, end, base[none:end], COPY(base[none:end]) */
				DO(asm_grrot(4)); /* COPY(base[none:end]), base, end, base[none:end] */
			}
			if (operand) {
				DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, end, base[none:end], operand */
				DO(asm_putddi(ddi_ast));
				DO(asm_pstack(current_assembler.a_stackcur - 2));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_ddicsp(); /* base, end, base[none:end] += operand */
			} else {
				DO(asm_pstack(current_assembler.a_stackcur - 1));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_dicsp(); /* base, end, ++base[none:end] */
			}
			if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
				DO(asm_gdup()); /* base, end, base[none:end] += operand, base[none:end] += operand */
				DO(asm_grrot(4)); /* base[none:end] += operand, base, end, base[none:end] += operand */
			}
			DO(asm_gsetrange_np()); /* . */
			goto done;
		} else if (DeeInt_Check(start->a_constexpr)) {
			int32_t start_index;
			if (DeeInt_TryAsInt32(start->a_constexpr, &start_index) &&
			    start_index >= INT16_MIN && start_index <= INT16_MAX) {
				if (end->a_type == AST_CONSTEXPR) {
					int32_t end_index;
					if (DeeNone_Check(end->a_constexpr)) {
						/* <base>[42:none] += operand; (Uses `asm_ggetrange_in') */
						DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
						DO(asm_putddi(ddi_ast));
						DO(asm_gdup()); /* base, base */
						DO(asm_ggetrange_in((int16_t)start_index)); /* base, base[42:none] */
						if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
							DO(asm_gdup()); /* base, base[42:none], base[42:none] */
							DO(asm_gcopy()); /* base, base[42:none], COPY(base[42:none]) */
							DO(asm_grrot(3)); /* COPY(base[42:none]), base, base[42:none] */
						}
						if (operand) {
							DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, base[42:none], operand */
							DO(asm_putddi(ddi_ast));
							DO(asm_pstack(current_assembler.a_stackcur - 2));
							DO(asm_put(operator_instr_table[inplace_operator_name]));
							asm_ddicsp(); /* base, base[42:none] += operand */
						} else {
							DO(asm_pstack(current_assembler.a_stackcur - 1));
							DO(asm_put(operator_instr_table[inplace_operator_name]));
							asm_dicsp(); /* base, ++base[42:none] */
						}
						if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
							DO(asm_gdup()); /* base, base[42:none] += operand, base[42:none] += operand */
							DO(asm_grrot(3)); /* base[42:none] += operand, base, base[42:none] += operand */
						}
						DO(asm_gsetrange_in((int16_t)start_index)); /* . */
						goto done;
					} else if (DeeInt_Check(end->a_constexpr) &&
					           DeeInt_TryAsInt32(end->a_constexpr, &end_index) &&
					           end_index >= INT16_MIN && end_index <= INT16_MAX) {
						/* <base>[42:43] += operand; (Uses `asm_ggetrange_ii') */
						DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
						DO(asm_putddi(ddi_ast));
						DO(asm_gdup()); /* base, base */
						DO(asm_ggetrange_ii((int16_t)start_index, (int16_t)end_index)); /* base, base[42:43] */
						if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
							DO(asm_gdup()); /* base, base[42:43], base[42:43] */
							DO(asm_gcopy()); /* base, base[42:43], COPY(base[42:43]) */
							DO(asm_grrot(3)); /* COPY(base[42:43]), base, base[42:43] */
						}
						if (operand) {
							DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, base[42:43], operand */
							DO(asm_putddi(ddi_ast));
							DO(asm_pstack(current_assembler.a_stackcur - 2));
							DO(asm_put(operator_instr_table[inplace_operator_name]));
							asm_ddicsp(); /* base, base[42:43] += operand */
						} else {
							DO(asm_pstack(current_assembler.a_stackcur - 1));
							DO(asm_put(operator_instr_table[inplace_operator_name]));
							asm_dicsp(); /* base, ++base[42:43] */
						}
						if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
							DO(asm_gdup()); /* base, base[42:43] += operand, base[42:43] += operand */
							DO(asm_grrot(3)); /* base[42:43] += operand, base, base[42:43] += operand */
						}
						DO(asm_gsetrange_ii((int16_t)start_index, (int16_t)end_index)); /* . */
						goto done;
					}
				}
				/* <base>[42:<end>] += operand; (Uses `asm_ggetrange_ip') */
				DO(ast_genasm(base, ASM_G_FPUSHRES));
				DO(asm_putddi(ddi_ast));
				DO(asm_gdup());
				DO(ast_genasm_one(end, ASM_G_FPUSHRES));
				DO(asm_putddi(ddi_ast));
				DO(asm_gdup()); /* base, base, end, end */
				DO(asm_grrot(3)); /* base, end, base, end */
				DO(asm_ggetrange_ip((int16_t)start_index)); /* base, end, base[42:end] */
				if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
					DO(asm_gdup()); /* base, end, base[42:end], base[42:end] */
					DO(asm_gcopy()); /* base, end, base[42:end], COPY(base[42:end]) */
					DO(asm_grrot(4)); /* COPY(base[42:end]), base, end, base[42:end] */
				}
				if (operand) {
					DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, end, base[42:end], operand */
					DO(asm_putddi(ddi_ast));
					DO(asm_pstack(current_assembler.a_stackcur - 2));
					DO(asm_put(operator_instr_table[inplace_operator_name]));
					asm_ddicsp(); /* base, end, base[42:end] += operand */
				} else {
					DO(asm_pstack(current_assembler.a_stackcur - 1));
					DO(asm_put(operator_instr_table[inplace_operator_name]));
					asm_dicsp(); /* base, end, ++base[42:end] */
				}
				if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
					DO(asm_gdup()); /* base, end, base[42:end] += operand, base[42:end] += operand */
					DO(asm_grrot(4)); /* base[42:end] += operand, base, end, base[42:end] += operand */
				}
				DO(asm_gsetrange_ip((int16_t)start_index)); /* . */
				goto done;
			}
		}
	}
	if (end->a_type == AST_CONSTEXPR) {
		int32_t end_index;
		if (DeeNone_Check(end->a_constexpr)) {
			/* <base>[start:none] += operand; (Uses `asm_ggetrange_pn') */
			DO(ast_genasm(base, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup());
			DO(ast_genasm_one(start, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* base, base, start, start */
			DO(asm_grrot(3)); /* base, start, base, start */
			DO(asm_ggetrange_pn()); /* base, start, base[start:none] */
			if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
				DO(asm_gdup()); /* base, start, base[start:none], base[start:none] */
				DO(asm_gcopy()); /* base, start, base[start:none], COPY(base[start:none]) */
				DO(asm_grrot(4)); /* COPY(base[start:none]), base, start, base[start:none] */
			}
			if (operand) {
				DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, start, base[start:none], operand */
				DO(asm_putddi(ddi_ast));
				DO(asm_pstack(current_assembler.a_stackcur - 2));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_ddicsp(); /* base, start, base[start:none] += operand */
			} else {
				DO(asm_pstack(current_assembler.a_stackcur - 1));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_dicsp(); /* base, start, ++base[start:none] */
			}
			if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
				DO(asm_gdup()); /* base, start, base[start:none] += operand, base[start:none] += operand */
				DO(asm_grrot(4)); /* base[start:none] += operand, base, start, base[start:none] += operand */
			}
			DO(asm_gsetrange_pn()); /* . */
			goto done;
		} else if (DeeInt_Check(end->a_constexpr) &&
		           DeeInt_TryAsInt32(end->a_constexpr, &end_index) &&
		           end_index >= INT16_MIN && end_index <= INT16_MAX) {
			/* <base>[start:42] += operand; (Uses `asm_ggetrange_pi') */
			DO(ast_genasm(base, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup());
			DO(ast_genasm_one(start, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* base, base, start, start */
			DO(asm_grrot(3)); /* base, start, base, start */
			DO(asm_ggetrange_pi((int16_t)end_index)); /* base, start, base[start:42] */
			if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
				DO(asm_gdup()); /* base, start, base[start:42], base[start:42] */
				DO(asm_gcopy()); /* base, start, base[start:42], COPY(base[start:42]) */
				DO(asm_grrot(4)); /* COPY(base[start:42]), base, start, base[start:42] */
			}
			if (operand) {
				DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, start, base[start:42], operand */
				DO(asm_putddi(ddi_ast));
				DO(asm_pstack(current_assembler.a_stackcur - 2));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_ddicsp(); /* base, start, base[start:42] += operand */
			} else {
				DO(asm_pstack(current_assembler.a_stackcur - 1));
				DO(asm_put(operator_instr_table[inplace_operator_name]));
				asm_dicsp(); /* base, start, ++base[start:42] */
			}
			if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
				DO(asm_gdup()); /* base, start, base[start:42] += operand, base[start:42] += operand */
				DO(asm_grrot(4)); /* base[start:42] += operand, base, start, base[start:42] += operand */
			}
			DO(asm_gsetrange_pi((int16_t)end_index)); /* . */
			goto done;
		}
	}
	DO(ast_genasm(base, ASM_G_FPUSHRES)); /* base */
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup()); /* base, base */
	DO(ast_genasm_one(start, ASM_G_FPUSHRES)); /* base, base, start */
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup()); /* base, base, start, start */
	DO(asm_grrot(3)); /* base, start, base, start */
	DO(ast_genasm_one(end, ASM_G_FPUSHRES)); /* base, start, base, start, end */
	DO(asm_putddi(ddi_ast));
	DO(asm_gdup()); /* base, start, base, start, end, end */
	DO(asm_grrot(4)); /* base, start, end, base, start, end */
	DO(asm_ggetrange()); /* base, start, end, base[start:end] */
	if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
		DO(asm_gdup()); /* base, start, end, base[start:end], base[start:end] */
		DO(asm_gcopy()); /* base, start, end, base[start:end], COPY(base[start:end]) */
		DO(asm_grrot(5)); /* COPY(base[start:end]), base, start, end, base[start:end] */
	}
	if (operand) {
		DO(ast_genasm_one(operand, ASM_G_FPUSHRES)); /* base, start, end, base[start:end], operand */
		DO(asm_putddi(ddi_ast));
		DO(asm_pstack(current_assembler.a_stackcur - 2));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_ddicsp(); /* base, start, end, base[start:end] += operand */
	} else {
		DO(asm_pstack(current_assembler.a_stackcur - 1));
		DO(asm_put(operator_instr_table[inplace_operator_name]));
		asm_dicsp(); /* base, start, end, ++base[start:end] */
	}
	if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
		DO(asm_gdup()); /* base, start, end, base[start:end] += operand, base[start:end] += operand */
		DO(asm_grrot(5)); /* base[start:end] += operand, base, start, end, base[start:end] += operand */
	}
	DO(asm_gsetrange()); /* . */
done:
	return 0;
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENINPLACE_C */
