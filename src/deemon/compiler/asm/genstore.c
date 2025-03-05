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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENSTORE_C
#define GUARD_DEEMON_COMPILER_ASM_GENSTORE_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/traits.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#define PUSH_RESULT (gflags & ASM_G_FPUSHRES)

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_gpush2_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict ddi_ast,
                   unsigned int gflags) {
	if (PUSH_RESULT) {
		if (ast_can_exchange(b, a)) {
			if (ast_genasm(b, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(a, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup_n(2 - 2))
				goto err;
		} else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
			if (ast_genasm(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err; /* a, b */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup())
				goto err; /* a, b, b */
			if (asm_grrot(3))
				goto err; /* b, a, b */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_none())
				goto err;
			if (ast_genasm(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err; /* none, a, b */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup())
				goto err; /* none, a, b, b */
			if (asm_gpop_n(4 - 2))
				goto err; /* b,    a, b */
		}
	} else {
		if (ast_genasm(a, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(b, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
asm_gpush3_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict c,
                   struct ast *__restrict ddi_ast,
                   unsigned int gflags) {
	if (PUSH_RESULT) {
		if (ast_can_exchange(c, b) && ast_can_exchange(c, a)) {
			if (ast_genasm(c, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup_n(3 - 2))
				goto err;
		} else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
			if (ast_genasm(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(c, ASM_G_FPUSHRES))
				goto err; /* a, b, c */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup())
				goto err; /* a, b, c, c */
			if (asm_grrot(4))
				goto err; /* c, a, b, c */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_none())
				goto err;
			if (ast_genasm(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(c, ASM_G_FPUSHRES))
				goto err; /* none, a, b, c */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup())
				goto err; /* none, a, b, c, c */
			if (asm_gpop_n(5 - 2))
				goto err; /* c,    a, b, c */
		}
	} else {
		if (ast_genasm(a, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(b, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(c, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
asm_gpush4_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict c,
                   struct ast *__restrict d,
                   struct ast *__restrict ddi_ast,
                   unsigned int gflags) {
	if (PUSH_RESULT) {
		if (ast_can_exchange(d, c) &&
		    ast_can_exchange(d, b) &&
		    ast_can_exchange(d, a)) {
			if (ast_genasm(d, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(c, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup_n(4 - 2))
				goto err;
		} else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
			if (ast_genasm(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(c, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(d, ASM_G_FPUSHRES))
				goto err; /* a, b, c, d */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup())
				goto err; /* a, b, c, d, d */
			if (asm_grrot(5))
				goto err; /* d, a, b, c, d */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_none())
				goto err;
			if (ast_genasm(a, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(b, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(c, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(d, ASM_G_FPUSHRES))
				goto err; /* none, a, b, c, d */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gdup())
				goto err; /* none, a, b, c, d, d */
			if (asm_gpop_n(6 - 2))
				goto err; /* d,    a, b, c, d */
		}
	} else {
		if (ast_genasm(a, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(b, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(c, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(d, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_gen_getattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *ddi_ast,
                unsigned int gflags) {
	/* Special optimizations when the attribute name is known at compile-time. */
	if (name->a_type == AST_CONSTEXPR &&
	    DeeString_Check(name->a_constexpr)) {
		DeeStringObject *attrname;
		int32_t attrid;
		attrname = (DeeStringObject *)name->a_constexpr;
		if (base->a_type == AST_SYM) {
			struct symbol *sym = base->a_sym;
			DeeClassScopeObject *class_scope;
check_getattr_sym:
			for (class_scope = base->a_scope->s_class; class_scope;
			     class_scope = DeeClassScope_Prev(class_scope)) {
				/* Try to statically access known class members! */
				if (sym == class_scope->cs_class ||
				    sym == class_scope->cs_this) {
					struct symbol *classsym;
					classsym = scope_lookup_str(&class_scope->cs_scope,
					                            DeeString_STR(attrname),
					                            DeeString_SIZE(attrname));
					/* Generate a regular attribute access. */
					if (classsym &&
					    classsym->s_type == SYMBOL_TYPE_CATTR &&
					    classsym->s_attr.a_class == class_scope->cs_class) {
						if (sym == classsym->s_attr.a_this ||                                /* Regular access to a dynamic attribute. */
						    (sym == classsym->s_attr.a_class && !classsym->s_attr.a_this)) { /* Regular access to a static-attribute. */
							if (asm_putddi(ddi_ast))
								goto err;
							if (asm_gpush_symbol(classsym, ddi_ast))
								goto err;
							goto pop_unused;
						}
					}
					break;
				}
			}
			switch (sym->s_type) {

			case SYMBOL_TYPE_ALIAS:
				sym = sym->s_alias;
				goto check_getattr_sym;

			case SYMBOL_TYPE_THIS:
				attrid = asm_newconst((DeeObject *)attrname);
				if unlikely(attrid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_ggetattr_this_const((uint16_t)attrid))
					goto err;
				goto pop_unused;

			case SYMBOL_TYPE_MYMOD: {
				struct symbol *globsym;
				struct TPPKeyword *kwd;
				int32_t symid;
				/* mymod.attr --> push bnd global ... */
				kwd = TPPLexer_LookupKeyword(DeeString_STR(attrname),
				                             DeeString_SIZE(attrname),
				                             0);
				if (!kwd)
					break; /* Never used as keyword (TODO: Add a warning for this) */
				globsym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd);
				if (!globsym)
					break; /* No such symbol was ever defined (TODO: Add a warning for this) */
				if (globsym->s_type != SYMBOL_TYPE_GLOBAL)
					break; /* Not a global symbol (TODO: Add a warning for this) */
				if (!PUSH_RESULT)
					goto done;
				symid = asm_gsymid_for_read(globsym, ddi_ast);
				if unlikely(symid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				return asm_gpush_global((uint16_t)symid);
			}	break;

			case SYMBOL_TYPE_MODULE: {
				struct module_symbol *modsym;
				int32_t module_id;
				/* module.attr --> push extern ... */
				modsym = DeeModule_GetSymbol(SYMBOL_MODULE_MODULE(sym),
				                             (DeeObject *)attrname);
				if (!modsym)
					break;
				if (!PUSH_RESULT && !(modsym->ss_flags & MODSYM_FPROPERTY))
					goto done;
				if (modsym->ss_flags & MODSYM_FEXTERN) {
					uint16_t impid = modsym->ss_extern.ss_impid;
					ASSERT(impid < SYMBOL_MODULE_MODULE(sym)->mo_importc);
					module_id = asm_newmodule(SYMBOL_MODULE_MODULE(sym)->mo_importv[impid]);
				} else {
					module_id = asm_msymid(sym);
				}
				if unlikely(module_id < 0)
					goto err;
				/* Push an external symbol accessed through its module. */
				if (asm_putddi(ddi_ast))
					goto err;
				return modsym->ss_flags & MODSYM_FPROPERTY
				       ? asm_gcall_extern((uint16_t)module_id, modsym->ss_index + MODULE_PROPERTY_GET, 0)
				       : asm_gpush_extern((uint16_t)module_id, modsym->ss_index);
			}	break;

			default: break;
			}
		}
		if (base->a_type == AST_ACTION &&
		    base->a_flag == AST_FACTION_AS &&
		    base->a_action.a_act0->a_type == AST_SYM &&
		    base->a_action.a_act0->a_sym->s_type == SYMBOL_TYPE_THIS &&
		    !SYMBOL_MUST_REFERENCE_THIS(base->a_action.a_act0->a_sym)) {
			/* `(this as ...).foobar'
			 * -> Check if we can make use of `ASM_SUPERGETATTR_THIS_RC' instructions. */
			struct ast *type_expr = base->a_action.a_act1;
			int32_t type_rid;
			if (type_expr->a_type == AST_SYM &&
			    ASM_SYMBOL_MAY_REFERENCE(type_expr->a_sym)) {
				/* We are allowed to reference the base-symbol! */
				type_rid = asm_rsymid(type_expr->a_sym);
do_perform_supergetattr:
				if unlikely(type_rid < 0)
					goto err;
				attrid = asm_newconst((DeeObject *)attrname);
				if unlikely(attrid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gsupergetattr_this_rc((uint16_t)type_rid, (uint16_t)attrid))
					goto err;
				goto pop_unused;
			}
			if (type_expr->a_type == AST_CONSTEXPR &&
			    current_basescope != (DeeBaseScopeObject *)current_rootscope &&
			    !(current_assembler.a_flag & ASM_FREDUCEREFS)) {
				/* Check if the type-expression is a constant that had been exported
				 * from the builtin `deemon' module, in which case we are able to cast
				 * an explicit reference to it. */
				struct symbol *deemon_symbol;
				deemon_symbol = asm_bind_deemon_export(type_expr->a_constexpr);
				if unlikely(!deemon_symbol)
					goto err;
				if (deemon_symbol != ASM_BIND_DEEMON_EXPORT_NOTFOUND) {
					type_rid = asm_rsymid(deemon_symbol);
					goto do_perform_supergetattr;
				}
			}
		}
		if (ast_genasm(base, ASM_G_FPUSHRES))
			goto err;
		attrid = asm_newconst((DeeObject *)attrname);
		if unlikely(attrid < 0)
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_ggetattr_const((uint16_t)attrid))
			goto err;
		goto pop_unused;
	}
	if (ast_genasm(base, ASM_G_FPUSHRES))
		goto err;
	if (ast_genasm_one(name, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	if (asm_ggetattr())
		goto err;
pop_unused:
	if (!(gflags & ASM_G_FPUSHRES))
		return asm_gpop();
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_gen_boundattr(struct ast *__restrict base,
                  struct ast *__restrict name,
                  struct ast *ddi_ast,
                  unsigned int gflags) {
	/* Special optimizations when the attribute name is known at compile-time. */
	if (name->a_type == AST_CONSTEXPR &&
	    DeeString_Check(name->a_constexpr)) {
		DeeStringObject *attrname;
		attrname = (DeeStringObject *)name->a_constexpr;
		if (base->a_type == AST_SYM) {
			struct symbol *sym = base->a_sym;
			DeeClassScopeObject *class_scope;
check_boundattr_sym:
			for (class_scope = base->a_scope->s_class; class_scope;
			     class_scope = DeeClassScope_Prev(class_scope)) {
				/* Try to statically access known class members! */
				if (sym == class_scope->cs_class ||
				    sym == class_scope->cs_this) {
					struct symbol *classsym;
					classsym = scope_lookup_str(&class_scope->cs_scope,
					                            DeeString_STR(attrname),
					                            DeeString_SIZE(attrname));
					/* Generate a regular attribute access. */
					if (classsym &&
					    classsym->s_type == SYMBOL_TYPE_CATTR &&
					    classsym->s_attr.a_class == class_scope->cs_class) {
						if (sym == classsym->s_attr.a_this ||                                /* Regular access to a dynamic attribute. */
						    (sym == classsym->s_attr.a_class && !classsym->s_attr.a_this)) { /* Regular access to a static-attribute. */
							if (asm_putddi(ddi_ast))
								goto err;
							if (asm_gpush_bnd_symbol(classsym, ddi_ast))
								goto err;
							goto pop_unused;
						}
					}
					break;
				}
			}
			switch (sym->s_type) {

			case SYMBOL_TYPE_ALIAS:
				sym = sym->s_alias;
				goto check_boundattr_sym;

			case SYMBOL_TYPE_MYMOD: {
				struct symbol *globsym;
				struct TPPKeyword *kwd;
				int32_t symid;
				/* mymod.attr --> push bnd global ... */
				kwd = TPPLexer_LookupKeyword(DeeString_STR(attrname),
				                             DeeString_SIZE(attrname),
				                             0);
				if (!kwd)
					break; /* Never used as keyword (TODO: Add a warning for this) */
				globsym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd);
				if (!globsym)
					break; /* No such symbol was ever defined (TODO: Add a warning for this) */
				if (globsym->s_type != SYMBOL_TYPE_GLOBAL)
					break; /* Not a global symbol (TODO: Add a warning for this) */
				if (!PUSH_RESULT)
					goto done;
				if (!SYMBOL_NWRITE(globsym)) {
					/* Never written -> must be unbound */
					if (asm_putddi(ddi_ast))
						goto err;
					return asm_gpush_false();
				}
				symid = asm_gsymid_for_read(globsym, ddi_ast);
				if unlikely(symid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				return asm_gpush_bnd_global((uint16_t)symid);
			}	break;

			case SYMBOL_TYPE_MODULE: {
				struct module_symbol *modsym;
				int32_t module_id;
				/* module.attr --> push bnd extern ... */
				modsym = DeeModule_GetSymbol(SYMBOL_MODULE_MODULE(sym),
				                             (DeeObject *)attrname);
				if (!modsym)
					break;
				if (modsym->ss_flags & MODSYM_FPROPERTY)
					break; /* Handle property-like module symbols via attributes. */
				if (!PUSH_RESULT)
					goto done;
				if (modsym->ss_flags & MODSYM_FEXTERN) {
					uint16_t impid = modsym->ss_extern.ss_impid;
					ASSERT(impid < SYMBOL_MODULE_MODULE(sym)->mo_importc);
					module_id = asm_newmodule(SYMBOL_MODULE_MODULE(sym)->mo_importv[impid]);
				} else {
					module_id = asm_msymid(sym);
				}
				if unlikely(module_id < 0)
					goto err;
				/* Push an external symbol accessed through its module. */
				if (asm_putddi(ddi_ast))
					goto err;
				return asm_gpush_bnd_extern((uint16_t)module_id, modsym->ss_index);
			}	break;

			default: break;
			}
		}
	}
	if (ast_genasm(base, ASM_G_FPUSHRES))
		goto err;
	if (ast_genasm_one(name, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	if (asm_gboundattr())
		goto err;
pop_unused:
	if (!(gflags & ASM_G_FPUSHRES))
		return asm_gpop();
done:
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
ast_gen_delattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *ddi_ast) {
	/* Special optimizations when the attribute name is known at compile-time. */
	if (name->a_type == AST_CONSTEXPR &&
	    DeeString_Check(name->a_constexpr)) {
		int32_t attrid;
		if (base->a_type == AST_SYM) {
			struct symbol *sym = base->a_sym;
			DeeClassScopeObject *class_scope;
check_delattr_sym:
			for (class_scope = base->a_scope->s_class; class_scope;
			     class_scope = DeeClassScope_Prev(class_scope)) {
				/* Try to statically access known class members! */
				if (sym == class_scope->cs_class ||
				    sym == class_scope->cs_this) {
					struct symbol *classsym;
					classsym = scope_lookup_str(&class_scope->cs_scope,
					                            DeeString_STR(name->a_constexpr),
					                            DeeString_SIZE(name->a_constexpr));
					/* Generate a regular attribute access. */
					if (classsym &&
					    classsym->s_type == SYMBOL_TYPE_CATTR &&
					    classsym->s_attr.a_class == class_scope->cs_class) {
						if (sym == classsym->s_attr.a_this ||                                /* Regular access to a dynamic attribute. */
						    (sym == classsym->s_attr.a_class && !classsym->s_attr.a_this)) { /* Regular access to a static-attribute. */
							if (asm_putddi(ddi_ast))
								goto err;
							return asm_gdel_symbol(classsym, ddi_ast);
						}
					}
					break;
				}
			}
			switch (sym->s_type) {

			case SYMBOL_TYPE_ALIAS:
				sym = sym->s_alias;
				goto check_delattr_sym;

			case SYMBOL_TYPE_THIS:
				if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
					break;
				attrid = asm_newconst(name->a_constexpr);
				if unlikely(attrid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				return asm_gdelattr_this_const((uint16_t)attrid);

			case SYMBOL_TYPE_MYMOD: {
				struct symbol *globsym;
				struct TPPKeyword *kwd;
				int32_t symid;
				/* mymod.attr --> push bnd global ... */
				kwd = TPPLexer_LookupKeyword(DeeString_STR(name->a_constexpr),
				                             DeeString_SIZE(name->a_constexpr),
				                             0);
				if (!kwd)
					break; /* Never used as keyword (TODO: Add a warning for this) */
				globsym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd);
				if (!globsym)
					break; /* No such symbol was ever defined (TODO: Add a warning for this) */
				if (globsym->s_type != SYMBOL_TYPE_GLOBAL)
					break; /* Not a global symbol (TODO: Add a warning for this) */
				symid = asm_gsymid(globsym);
				if unlikely(symid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				return asm_gdel_global((uint16_t)symid);
			}	break;

			default: break;
			}
		}
		attrid = asm_newconst(name->a_constexpr);
		if unlikely(attrid < 0)
			goto err;
		if (ast_genasm(base, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		return asm_gdelattr_const((uint16_t)attrid);
	}
	if (ast_genasm(base, ASM_G_FPUSHRES))
		goto err;
	if (ast_genasm_one(name, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	return asm_gdelattr();
err:
	return -1;
}


INTDEF WUNUSED NONNULL((1, 2)) int DCALL
asm_check_thiscall(struct symbol *__restrict sym,
                   struct ast *__restrict warn_ast);

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_set_cattr_symbol(struct symbol *__restrict sym,
                     struct ast *__restrict value,
                     struct ast *__restrict ddi_ast,
                     unsigned int gflags) {
	struct symbol *class_sym, *this_sym;
	struct class_attribute *attr;
	int32_t symid;
	ASSERT(sym->s_type == SYMBOL_TYPE_CATTR);
	class_sym = sym->s_attr.a_class;
	this_sym  = sym->s_attr.a_this;
	attr      = sym->s_attr.a_attr;
	SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
	if (!this_sym) {
set_class_attribute:
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
				goto fallback;
			/* Must invoke the setter callback. */
			if (PUSH_RESULT && ast_genasm(value, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET))
					goto err; /* [result], func */
			} else {
				if (asm_gpush_symbol(class_sym, ddi_ast))
					goto err; /* [result], class_sym */
				if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_SET))
					goto err; /* [result], func */
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				if (asm_gpush_symbol(class_sym, ddi_ast))
					goto err; /* [result], setter, class */
				if (PUSH_RESULT) {
					if (asm_gdup_n(1))
						goto err; /* result, setter, class, value */
				} else {
					if (ast_genasm_one(value, ASM_G_FPUSHRES))
						goto err; /* setter, class, value */
					if (asm_putddi(ddi_ast))
						goto err;
				}
				if (asm_gcall(2))
					goto err; /* [result], discard */
			} else {
				if (PUSH_RESULT) {
					if (asm_gdup_n(0))
						goto err; /* result, setter, value */
				} else {
					if (ast_genasm_one(value, ASM_G_FPUSHRES))
						goto err; /* setter, value */
					if (asm_putddi(ddi_ast))
						goto err;
				}
				if (asm_gcall(1))
					goto err; /* [result], discard */
			}
			return asm_gpop();
		}
		if (PUSH_RESULT) {
			if (ast_genasm(value, ASM_G_FPUSHRES))
				goto err; /* result */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* result, class */
			if (asm_gdup_n(0))
				goto err; /* result, class, value */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* class */
			if (ast_genasm_one(value, ASM_G_FPUSHRES))
				goto err; /* class, value */
			if (asm_putddi(ddi_ast))
				goto err;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
			/* XXX: Assert that not already bound? */
		}
		if (asm_gdefcmember(attr->ca_addr))
			goto err;      /* [result], class */
		return asm_gpop(); /* [result] */
	}

	/* Check if the attribute must be accessed as virtual. */
	if unlikely(asm_check_thiscall(sym, ddi_ast))
		goto err;
	SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
	if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
do_virtual_access:
		symid = asm_newconst((DeeObject *)attr->ca_name);
		if unlikely(symid < 0)
			goto err;
		if (this_sym->s_type == SYMBOL_TYPE_THIS &&
		    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
			if (ast_genasm(value, ASM_G_FPUSHRES))
				goto err; /* value */
			if (asm_putddi(ddi_ast))
				goto err;
			if (PUSH_RESULT && asm_gdup())
				goto err;                                    /* [result], value */
			return asm_gsetattr_this_const((uint16_t)symid); /* [result] */
		}
		if (PUSH_RESULT) {
			if (ast_genasm(value, ASM_G_FPUSHRES))
				goto err; /* result */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(this_sym, ddi_ast))
				goto err; /* result, this_sym */
			if (asm_gdup_n(0))
				goto err; /* result, this_sym, value */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(this_sym, ddi_ast))
				goto err; /* this_sym */
			if (ast_genasm_one(value, ASM_G_FPUSHRES))
				goto err; /* this_sym, value */
			if (asm_putddi(ddi_ast))
				goto err;
		}
		return asm_gsetattr_const((uint16_t)symid); /* [result] */
	}

	/* Regular, old member variable. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto fallback;
		/* Call the setter function of the attribute. */
		if (PUSH_RESULT && ast_genasm(value, ASM_G_FPUSHRES))
			goto err; /* value */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD)) ==
				    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD) &&
				    this_sym->s_type == SYMBOL_TYPE_THIS &&
				    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
					/* Invoke the setter callback. */
					if (!PUSH_RESULT && ast_genasm(value, ASM_G_FPUSHRES))
						goto err; /* value */
					if (asm_putddi(ddi_ast))
						goto err;
					if (PUSH_RESULT && asm_gdup())
						goto err; /* [result], value */
					if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET, 0))
						goto err;
					goto pop_unused_result;
				}
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET))
					goto err; /* [result], setter */
			} else {
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gpush_symbol(class_sym, ddi_ast))
					goto err; /* [result], class_sym */
				if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_SET))
					goto err; /* [result], setter */
			}
		} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
		           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(this_sym, ddi_ast))
				goto err; /* [result], this_sym */
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* [result], this_sym, class_sym */
			if (asm_ggetmember(attr->ca_addr + CLASS_GETSET_SET))
				goto err; /* [result], setter */
		} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
			symid = asm_rsymid(class_sym);
			if unlikely(symid < 0)
				goto err;
			if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET))
				goto err; /* [result], setter */
		} else {
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* [result], class_sym */
			if (asm_ggetmember_this(attr->ca_addr + CLASS_GETSET_SET))
				goto err; /* [result], setter */
		}
		/* [result], setter */
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
			if (PUSH_RESULT) {
				if (asm_gdup_n(0))
					goto err; /* result, setter, value */
			} else {
				if (ast_genasm_one(value, ASM_G_FPUSHRES))
					goto err; /* setter, value */
				if (asm_putddi(ddi_ast))
					goto err;
			}
			if (asm_gcall(1))
				goto err; /* [result], discard */
		} else {
			/* Invoke as a this-call. */
			if (asm_gpush_symbol(this_sym, ddi_ast))
				goto err; /* [result], setter, this */
			if (PUSH_RESULT) {
				if (asm_gdup_n(1))
					goto err; /* result, setter, this, value */
			} else {
				if (ast_genasm_one(value, ASM_G_FPUSHRES))
					goto err; /* setter, this, value */
				if (asm_putddi(ddi_ast))
					goto err;
			}
			if (asm_gcall(2))
				goto err; /* [result], discard */
		}
pop_unused_result:
		return asm_gpop();
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		goto set_class_attribute;
	if (this_sym->s_type != SYMBOL_TYPE_THIS ||
	    SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto do_virtual_access; /* There is no `setmemberi pop, pop, $<imm8>, pop' instruction, so use fallback */
		if (PUSH_RESULT) {
			if (ast_genasm(value, ASM_G_FPUSHRES))
				goto err; /* result */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(this_sym, ddi_ast))
				goto err; /* result, this */
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* result, this, class */
			if (asm_gdup_n(1))
				goto err; /* result, this, class, value */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(this_sym, ddi_ast))
				goto err; /* this */
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* this, class */
			if (ast_genasm_one(value, ASM_G_FPUSHRES))
				goto err; /* this, class, value */
			if (asm_putddi(ddi_ast))
				goto err;
		}
		if (asm_gsetmember(attr->ca_addr))
			goto err; /* [result] */
	} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
		symid = asm_rsymid(class_sym);
		if unlikely(symid < 0)
			goto err;
		if (ast_genasm(value, ASM_G_FPUSHRES))
			goto err; /* value */
		if (asm_putddi(ddi_ast))
			goto err;
		if (PUSH_RESULT && asm_gdup())
			goto err; /* [result], value */
		if ((attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		    ? asm_gsetmemberi_this_r((uint16_t)symid, attr->ca_addr)
		    : asm_gsetmember_this_r((uint16_t)symid, attr->ca_addr))
			goto err; /* [result] */
	} else {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto do_virtual_access; /* There is no `setmemberi this, pop, $<imm8>, pop' instruction, so use fallback */
		if (PUSH_RESULT) {
			if (ast_genasm(value, ASM_G_FPUSHRES))
				goto err; /* result */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* result, class */
			if (asm_gdup_n(0))
				goto err; /* result, class, value */
		} else {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gpush_symbol(class_sym, ddi_ast))
				goto err; /* class */
			if (ast_genasm_one(value, ASM_G_FPUSHRES))
				goto err; /* class, value */
			if (asm_putddi(ddi_ast))
				goto err;
		}
		if (asm_gsetmember_this(attr->ca_addr))
			goto err; /* [result] */
	}
	return 0;
fallback:
	if (ast_genasm(value, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	if ((gflags & ASM_G_FPUSHRES) && asm_gdup())
		goto err;
	return asm_gpop_symbol(sym, ddi_ast);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
ast_gen_setattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *__restrict value,
                struct ast *ddi_ast,
                unsigned int gflags) {
	if (name->a_type == AST_CONSTEXPR &&
	    DeeString_Check(name->a_constexpr)) {
		int32_t cid;
		if (base->a_type == AST_SYM) {
			struct symbol *sym = base->a_sym;
			DeeClassScopeObject *class_scope;
check_base_symbol_class:
			for (class_scope = base->a_scope->s_class; class_scope;
			     class_scope = DeeClassScope_Prev(class_scope)) {
				/* Try to statically access known class members! */
				if (sym == class_scope->cs_class ||
				    sym == class_scope->cs_this) {
					struct symbol *funsym;
					funsym = scope_lookup_str(&class_scope->cs_scope,
					                          DeeString_STR(name->a_constexpr),
					                          DeeString_SIZE(name->a_constexpr));
					/* Generate a regular attribute access. */
					if (funsym &&
					    funsym->s_type == SYMBOL_TYPE_CATTR &&
					    funsym->s_attr.a_class == class_scope->cs_class) {
						if (sym == funsym->s_attr.a_this ||                            /* Regular access to a dynamic attribute. */
						    (sym == funsym->s_attr.a_class && !funsym->s_attr.a_this)) /* Regular access to a static-attribute. */
							return asm_set_cattr_symbol(funsym, value, ddi_ast, gflags);
					}
					break;
				}
			}
			switch (sym->s_type) {

			case SYMBOL_TYPE_ALIAS:
				sym = sym->s_alias;
				goto check_base_symbol_class;

			case SYMBOL_TYPE_THIS:
				if (SYMBOL_MUST_REFERENCE_THIS(sym))
					break;
				cid = asm_newconst(name->a_constexpr);
				if unlikely(cid < 0)
					goto err;
				if (ast_genasm(value, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (PUSH_RESULT && asm_gdup())
					goto err;
				if (asm_gsetattr_this_const((uint16_t)cid))
					goto err;
				goto done;

			case SYMBOL_TYPE_MYMOD: {
				struct symbol *globsym;
				struct TPPKeyword *kwd;
				int32_t symid;
				/* mymod.attr --> pop global ... */
				kwd = TPPLexer_LookupKeyword(DeeString_STR(name->a_constexpr),
				                             DeeString_SIZE(name->a_constexpr),
				                             0);
				if (!kwd)
					break; /* Never used as keyword (TODO: Add a warning for this) */
				globsym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd);
				if (!globsym)
					break; /* No such symbol was ever defined (TODO: Add a warning for this) */
				if (globsym->s_type != SYMBOL_TYPE_GLOBAL)
					break; /* Not a global symbol (TODO: Add a warning for this) */
				symid = asm_gsymid(globsym);
				if unlikely(symid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (PUSH_RESULT && asm_gdup())
					goto err;
				return asm_gpop_global((uint16_t)symid);
			}	break;

			case SYMBOL_TYPE_MODULE: {
				struct module_symbol *modsym;
				int32_t module_id;
				/* module.attr --> pop extern ... */
				modsym = DeeModule_GetSymbol(SYMBOL_MODULE_MODULE(sym),
				                             name->a_constexpr);
				if (!modsym)
					break;
				if (modsym->ss_flags & MODSYM_FREADONLY)
					break;
				if (modsym->ss_flags & MODSYM_FEXTERN) {
					uint16_t impid = modsym->ss_extern.ss_impid;
					ASSERT(impid < SYMBOL_MODULE_MODULE(sym)->mo_importc);
					module_id = asm_newmodule(SYMBOL_MODULE_MODULE(sym)->mo_importv[impid]);
				} else {
					module_id = asm_msymid(sym);
				}
				if unlikely(module_id < 0)
					goto err;
				/* Push an external symbol accessed through its module. */
				if (ast_genasm(value, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (PUSH_RESULT && asm_gdup())
					goto err;
				if (modsym->ss_flags & MODSYM_FPROPERTY) {
					/* Invoke the setter callback. */
					if (asm_gcall_extern((uint16_t)module_id, modsym->ss_index + MODULE_PROPERTY_SET, 1))
						goto err;
					return asm_gpop();
				}
				return asm_gpop_extern((uint16_t)module_id, modsym->ss_index);
			}	break;

			default:
				break;
			}
		}
		if unlikely(asm_gpush2_duplast(base, value, ddi_ast, gflags))
			goto err;
		cid = asm_newconst(name->a_constexpr);
		if unlikely(cid < 0)
			goto err;
		if (asm_gsetattr_const((uint16_t)cid))
			goto err;
		goto done;
	}
	if unlikely(asm_gpush3_duplast(base, name, value, ddi_ast, gflags))
		goto err;
	if (asm_gsetattr())
		goto err;
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
ast_gen_setitem(struct ast *__restrict sequence,
                struct ast *__restrict index,
                struct ast *__restrict value,
                struct ast *ddi_ast,
                unsigned int gflags) {
	if (index->a_type == AST_CONSTEXPR) {
		int32_t int_index;
		/* Special optimizations for constant indices. */
		if (DeeInt_Check(index->a_constexpr) &&
		    DeeInt_TryAsInt32(index->a_constexpr, &int_index) &&
		    int_index >= INT16_MIN && int_index <= INT16_MAX) {
			if unlikely(asm_gpush2_duplast(sequence, value, ddi_ast, gflags))
				goto err;
			if (asm_gsetitem_index((int16_t)int_index))
				goto err;
			goto done;
		}
		if (asm_allowconst(index->a_constexpr)) {
			int_index = asm_newconst(index->a_constexpr);
			if unlikely(int_index < 0)
				goto err;
			if unlikely(asm_gpush2_duplast(sequence, value, ddi_ast, gflags))
				goto err;
			if (asm_gsetitem_const((uint16_t)int_index))
				goto err;
			goto done;
		}
	}
	if unlikely(asm_gpush3_duplast(sequence, index, value, ddi_ast, gflags))
		goto err;
	if (asm_gsetitem())
		goto err;
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
ast_gen_setrange(struct ast *__restrict sequence,
                 struct ast *__restrict begin,
                 struct ast *__restrict end,
                 struct ast *__restrict value,
                 struct ast *ddi_ast,
                 unsigned int gflags) {
	int32_t index;
	/* Special optimizations for certain ranges. */
	if (begin->a_type == AST_CONSTEXPR) {
		DeeObject *begin_index = begin->a_constexpr;
		if (DeeNone_Check(begin_index)) {
			/* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
			if (end->a_type == AST_CONSTEXPR &&
			    DeeInt_Check(end->a_constexpr) &&
			    DeeInt_TryAsInt32(end->a_constexpr, &index) &&
			    index >= INT16_MIN && index <= INT16_MAX) {
				/* `setrange pop, none, $<Simm16>, pop' */
				if unlikely(asm_gpush2_duplast(sequence, value, ddi_ast, gflags))
					goto err;
				if (asm_gsetrange_ni((int16_t)index))
					goto err;
				goto done;
			}
			/* `setrange pop, none, pop, pop' */
			if unlikely(asm_gpush3_duplast(sequence, end, value, ddi_ast, gflags))
				goto err;
			if (asm_gsetrange_np())
				goto err;
			goto done;
		}
		if (DeeInt_Check(begin_index) &&
		    DeeInt_TryAsInt32(begin_index, &index) &&
		    index >= INT16_MIN && index <= INT16_MAX) {
			if (end->a_type == AST_CONSTEXPR) {
				int32_t index2;
				DeeObject *end_index = end->a_constexpr;
				if (DeeNone_Check(end_index)) {
					/* `setrange pop, $<Simm16>, none, pop' */
					if unlikely(asm_gpush2_duplast(sequence, value, ddi_ast, gflags))
						goto err;
					if (asm_gsetrange_in((int16_t)index))
						goto err;
					goto done;
				}
				if (DeeInt_Check(end_index) &&
				    DeeInt_TryAsInt32(end_index, &index2) &&
				    index2 >= INT16_MIN && index2 <= INT16_MAX) {
					/* `setrange pop, $<Simm16>, $<Simm16>, pop' */
					if unlikely(asm_gpush2_duplast(sequence, value, ddi_ast, gflags))
						goto err;
					if (asm_gsetrange_ii((int16_t)index, (int16_t)index2))
						goto err;
					goto done;
				}
			}
			if unlikely(asm_gpush3_duplast(sequence, end, value, ddi_ast, gflags))
				goto err;
			if (asm_gsetrange_ip((int16_t)index))
				goto err;
			goto done;
		}
	} else if (end->a_type == AST_CONSTEXPR) {
		/* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
		DeeObject *end_index = end->a_constexpr;
		if (DeeNone_Check(end_index)) {
			/* `setrange pop, pop, none, pop' */
			if unlikely(asm_gpush3_duplast(sequence, begin, value, ddi_ast, gflags))
				goto err;
			if (asm_gsetrange_pn())
				goto err;
			goto done;
		}
		if (DeeInt_Check(end_index) &&
		    DeeInt_TryAsInt32(end_index, &index) &&
		    index >= INT16_MIN && index <= INT16_MAX) {
			/* `setrange pop, pop, $<Simm16>, pop' */
			if unlikely(asm_gpush3_duplast(sequence, begin, value, ddi_ast, gflags))
				goto err;
			if (asm_gsetrange_pi((int16_t)index))
				goto err;
			goto done;
		}
	}
	if unlikely(asm_gpush4_duplast(sequence, begin, end, value, ddi_ast, gflags))
		goto err;
	if (asm_gsetrange())
		goto err;
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
asm_gunpack_expr(struct ast *__restrict src,
                 uint16_t num_targets,
                 struct ast *__restrict ddi_ast) {
	/* Unwind inner sequence-expand expressions.
	 * This optimizes:
	 * >> `(x, y, z) = [(10, 20, 30)...];'
	 * Into:
	 * >> `(x, y, z) = (10, 20, 30);'
	 */
	for (;;) {
		struct ast *inner;
		if (src->a_type != AST_MULTIPLE)
			break;
		if (src->a_flag == AST_FMULTIPLE_KEEPLAST)
			break;
		if (src->a_multiple.m_astc != 1)
			break;
		inner = src->a_multiple.m_astv[0];
		if (inner->a_type != AST_EXPAND)
			break;
		src = inner->a_expand;
	}
	if (src->a_type == AST_SYM) {
		struct symbol *sym = src->a_sym;
		SYMBOL_INPLACE_UNWIND_ALIAS(sym);
		if (DeeBaseScope_IsVarargs(current_basescope, sym)) {
			/* Unpack the varargs argument. */
			if (asm_putddi(ddi_ast))
				goto err;
			return asm_gvarargs_unpack(num_targets);
		}
		if (asm_can_prefix_symbol_for_read(sym)) {
			/* The unpack instructions can make use of an object prefix. */
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gprefix_symbol_for_read(sym, src))
				goto err;
			return asm_gunpack_p(num_targets);
		}
	}

	/* Fallback: generate regular assembly for `src' and use `ASM_UNPACK' */
	if (ast_genasm(src, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	return asm_gunpack(num_targets);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL asm_gmov_sym_sym)(struct symbol *__restrict dst_sym,
                         struct symbol *__restrict src_sym,
                         struct ast *dst_ast,
                         struct ast *src_ast) {
	int32_t symid;
	ASSERT(asm_can_prefix_symbol(dst_sym));
check_src_sym_class:
	if (SYMBOL_MUST_REFERENCE(src_sym)) {
		/* mov PREFIX, ref <imm8/16> */
		symid = asm_rsymid(src_sym);
		if unlikely(symid < 0)
			goto err;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_ref_p((uint16_t)symid);
	}
	switch (src_sym->s_type) {

	case SYMBOL_TYPE_ALIAS:
		src_sym = src_sym->s_alias;
		goto check_src_sym_class;

	case SYMBOL_TYPE_STACK:
		/* mov PREFIX, #... */
		if (!(src_sym->s_flag & SYMBOL_FALLOC))
			break;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		if (SYMBOL_STACK_OFFSET(src_sym) == current_assembler.a_stackcur - 1) {
			return asm_gdup_p();
		} else {
			uint16_t offset;
			offset = (uint16_t)((current_assembler.a_stackcur -
			                     SYMBOL_STACK_OFFSET(src_sym)) -
			                    1);
			return asm_gdup_n_p(offset);
		}
		break;

	case SYMBOL_TYPE_EXCEPT:
		/* mov PREFIX, except */
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_except_p();

	case SYMBOL_TYPE_MODULE:
		/* mov PREFIX, module <imm8/16> */
		symid = asm_msymid(src_sym);
		if unlikely(symid < 0)
			goto err;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_module_p((uint16_t)symid);

	case SYMBOL_TYPE_THIS:
		/* mov PREFIX, this */
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_this_p();

	case SYMBOL_TYPE_MYMOD:
		/* mov PREFIX, this_module */
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_this_module_p();

	case SYMBOL_TYPE_MYFUNC:
		if (current_basescope->bs_flags & CODE_FTHISCALL)
			break; /* The function has to be bound! */
		/* mov PREFIX, this_function */
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_this_function_p();

	case SYMBOL_TYPE_ARG:
		/* mov PREFIX, arg <imm8/16> */
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		if (DeeBaseScope_IsVarargs(current_basescope, src_sym))
			return asm_gpush_varargs_p();
		if (DeeBaseScope_IsVarkwds(current_basescope, src_sym))
			return asm_gpush_varkwds_p();
		return asm_gpush_arg_p(src_sym->s_symid);

	case SYMBOL_TYPE_GLOBAL:
		/* mov PREFIX, global <imm8/16> */
		if (!(src_sym->s_flag & SYMBOL_FALLOC))
			break;
		symid = asm_gsymid_for_read(src_sym, dst_ast);
		if unlikely(symid < 0)
			goto err;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_global_p((uint16_t)symid);

	case SYMBOL_TYPE_LOCAL:
		/* mov PREFIX, local <imm8/16> */
		if (!(src_sym->s_flag & SYMBOL_FALLOC))
			break;
		symid = asm_lsymid_for_read(src_sym, dst_ast);
		if unlikely(symid < 0)
			goto err;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_local_p((uint16_t)symid);

	case SYMBOL_TYPE_STATIC:
		/* mov PREFIX, static <imm8/16> */
		if (!(src_sym->s_flag & SYMBOL_FALLOC))
			break;
		symid = asm_ssymid_for_read(src_sym, dst_ast);
		if unlikely(symid < 0)
			goto err;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_static_p((uint16_t)symid);

	case SYMBOL_TYPE_EXTERN:
		/* mov PREFIX, extern <imm8/16>:<imm8/16> */
		if (SYMBOL_EXTERN_SYMBOL(src_sym)->ss_flags & MODSYM_FPROPERTY)
			break; /* Cannot be used for external properties. */
		symid = asm_esymid(src_sym);
		if unlikely(symid < 0)
			goto err;
		if (asm_putddi(dst_ast))
			goto err;
		if (asm_gprefix_symbol(dst_sym, dst_ast))
			goto err;
		return asm_gpush_extern_p((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(src_sym)->ss_index);

	default: break;
	}
	if (asm_putddi(dst_ast))
		goto err;
	if (asm_gpush_symbol(src_sym, dst_ast))
		goto err;
	if (src_ast != dst_ast && asm_putddi(src_ast))
		goto err;
	return asm_gpop_symbol(dst_sym, src_ast);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 3)) int
(DCALL asm_gmov_sym_ast)(struct symbol *__restrict dst_sym,
                         struct ast *src,
                         struct ast *dst_ast) {
	ASSERT(asm_can_prefix_symbol(dst_sym));
	switch (src->a_type) {

		/* The ASM_FUNCTION* instructions can be used with a prefix to
		 * construct + store the function using a single instruction. */
	case AST_FUNCTION:
		return asm_gmov_function(dst_sym, src, dst_ast);

	case AST_SYM:
		return asm_gmov_sym_sym(dst_sym, src->a_sym, dst_ast, src);

	case AST_CONSTEXPR: {
		DeeObject *constval;
		constval = src->a_constexpr;
		if (DeeNone_Check(constval)) {
			/* mov PREFIX, none */
			if (asm_putddi(dst_ast))
				goto err;
			if (asm_gprefix_symbol(dst_sym, dst_ast))
				goto err;
			return asm_gpush_none_p();
		} else if (constval == Dee_True) {
			/* mov PREFIX, true */
			if (asm_putddi(dst_ast))
				goto err;
			if (asm_gprefix_symbol(dst_sym, dst_ast))
				goto err;
			return asm_gpush_true_p();
		} else if (constval == Dee_False) {
			/* mov PREFIX, false */
			if (asm_putddi(dst_ast))
				goto err;
			if (asm_gprefix_symbol(dst_sym, dst_ast))
				goto err;
			return asm_gpush_false_p();
		} else if (asm_allowconst(constval)) {
			int32_t cid;
			/* mov PREFIX, const <imm8/16> */
			cid = asm_newconst(constval);
			if unlikely(cid < 0)
				goto err;
			if (asm_putddi(dst_ast))
				goto err;
			if (asm_gprefix_symbol(dst_sym, dst_ast))
				goto err;
			return asm_gpush_const_p((uint16_t)cid);
		}
	}	break;

	default:
		break;
	}
	if (ast_genasm(src, ASM_G_FPUSHRES))
		goto err;
	if (asm_putddi(dst_ast))
		goto err;
	return asm_gpop_symbol(dst_sym, dst_ast);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int
(DCALL asm_gmov_ast_sym)(struct ast *dst,
                         struct symbol *__restrict src_sym,
                         struct ast *src_ast) {
	switch (dst->a_type) {

	case AST_SYM:
		if (asm_can_prefix_symbol(dst->a_sym))
			return asm_gmov_sym_sym(dst->a_sym, src_sym, dst, src_ast);
		break;

	default: break;
	}
	if (asm_putddi(src_ast))
		goto err;
	if (asm_gpop_expr_enter(dst))
		goto err;
	if (asm_gpush_symbol(src_sym, src_ast))
		goto err;
	if (asm_gpop_expr_leave(dst, ASM_G_FNORMAL))
		goto err;
	return asm_gpop_expr(dst);
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL asm_gmov_ast_constexpr)(struct ast *__restrict dst,
                               DeeObject *__restrict src,
                               struct ast *__restrict src_ast);


INTERN WUNUSED NONNULL((1, 2, 3)) int
(DCALL asm_gstore)(struct ast *__restrict dst,
                   struct ast *__restrict src,
                   struct ast *ddi_ast,
                   unsigned int gflags) {
again:
	switch (dst->a_type) {

	case AST_SYM: {
		struct symbol *dst_sym;
		dst_sym = dst->a_sym;

check_dst_sym_class:
		switch (dst_sym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			dst_sym = dst_sym->s_alias;
			goto check_dst_sym_class;

		case SYMBOL_TYPE_CATTR:
			return asm_set_cattr_symbol(dst_sym, src, ddi_ast, gflags);

		case SYMBOL_TYPE_STATIC: {
			int32_t sid;
			bool src_contains_return;

			/* Check if this is the initial declaration assignment, which gets treated special */
			if (dst->a_scope != dst_sym->s_scope)
				break;
			if (dst->a_ddi.l_file != dst_sym->s_decl.l_file)
				break;
			if (dst->a_ddi.l_line != dst_sym->s_decl.l_line)
				break;
			if (dst->a_ddi.l_col != dst_sym->s_decl.l_col)
				break;

			/* In the new static variable model, the first assignment should
			 * still only be executed *once*, however this definitely needs
			 * a special instruction that store a value in a static *only* if
			 * the static is currently unbound (use this to assign constants
			 * to static variables, or expressions that can be executed many
			 * times without side-effects)
			 *
			 * >> static local x = 42;
			 * ASM:
			 * >> push const @42                       # Value to store
			 * >> push cmpxch static @x, unbound, pop  # Pushes true/false indicative of cmpxch success
			 * >> pop                                  # Get rid of the true/false
			 *
			 * XXX: Why not have the caller assign the initial static value?
			 *      >> push const @42
			 *      >> push function const 1, #1  // Code has 0 refs, but allow caller to pre-assign statics
			 *
			 * In order to execute initializers with side-effects:
			 * >> static local x = foo();
			 *
			 * ASM:
			 * >>     push  cmpxch static @x, unbound, initializing # Pushes "true" if initialization started, "false" otherwise
			 * >>     jf    pop, 1f
			 * >> .Linit_except_start:
			 * >>     call  @foo, #1
			 * >> .Linit_except_end:
			 * >>     pop   static @x
			 * >> 1:
			 * >>
			 * >> .pushsection .cold    # Only needed if initializer can throw an exception
			 * >> .except .Linit_except_start, .Linit_except_end, .Linit_except_entry, @finally
			 * >> .Linit_except_entry:  # Clear "ITER_DONE" value on initialization exception
			 * >>     del   static @x
			 * >>     end   finally, except
			 * >> .popsection
			 *
			 * NOTES:
			 * - >> again:
			 *   >> if (!ITER_ISOK(fo_refv[x])) {
			 *   >>     if (!atomic_cmpxch(&fo_refv[x], NULL, ITER_DONE)) {
			 *   >>         DeeFutex_WaitPtr(&fo_refv[x], ITER_DONE);
			 *   >>         goto again;
			 *   >>     }
			 *   >>     DREF DeeObject *init = ...;
			 *   >>     if (!init) { // For exception handler...
			 *   >>         fo_refv[x] = NULL;              // ASM_DEL_STATIC
			 *   >>         DeeFutex_WakeAll(&fo_refv[x]);
			 *   >>         HANDLE_EXCEPT();
			 *   >>     } else { // When initializer doesn't throw an exception...
			 *   >>         fo_refv[x] = init;              // ASM_POP_STATIC
			 *   >>         DeeFutex_WakeAll(&fo_refv[x]);
			 *   >>     }
			 *   >> }
			 * - ASM_PUSH_STATIC and Function.__static__ implicitly handle ITER_DONE like NULL
			 * - ASM_POP_STATIC and ASM_DEL_STATIC call `DeeFutex_WakeAll()'
			 */
			sid = asm_ssymid(dst_sym);
			if unlikely(sid < 0)
				goto err;

			src_contains_return = ast_contains_return(src);
			if (src_contains_return || ast_has_sideeffects(src)) {
				struct asm_sym *Lalready_init;
				/* Complicated case: must generate the full initializer. */
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_pstatic((uint16_t)sid))
					goto err;
				if (asm_gcmpxch_ub_lock_p())
					goto err;
				Lalready_init = asm_newsym();
				if unlikely(!Lalready_init)
					goto err;
				if (asm_gjmp(ASM_JF, Lalready_init))
					goto err;
				asm_decsp();

				/* Generate the initializer expression. */
				if (!src_contains_return && ast_is_nothrow(src, true)) {
					if (ast_genasm_one(src, ASM_G_FPUSHRES))
						goto err;
				} else {
					struct asm_sym *Linit_try_except;
					struct asm_sym *Lafter_except;
					struct asm_sym *old_finally;
					uint16_t i, old_finflag, guard_finflags;
					code_addr_t guard_begin[SECTION_TEXTCOUNT];
					code_addr_t guard_end[SECTION_TEXTCOUNT];
					struct asm_sec *old_sect;
					int temp;

					Linit_try_except = asm_newsym();
					if unlikely(!Linit_try_except)
						goto err;

					/* Setup code for the try-finally we're going to generate */
					old_finflag = current_assembler.a_finflag;
					old_finally = current_assembler.a_finsym;
					current_assembler.a_finflag = ASM_FINFLAG_NORMAL;
					current_assembler.a_finsym  = Linit_try_except;
					for (i = 0; i < SECTION_TEXTCOUNT; ++i)
						guard_begin[i] = asm_secip(i);

					/* Do the actual push of the initializer. */
					temp = ast_genasm_one(src, ASM_G_FPUSHRES);

					/* Restore state. */
					guard_finflags = current_assembler.a_finflag;
					current_assembler.a_finsym  = old_finally;
					current_assembler.a_finflag = old_finflag;
					if (temp)
						goto err;
					for (i = 0; i < SECTION_TEXTCOUNT; ++i)
						guard_end[i] = asm_secip(i);
					for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
						ASSERT(guard_begin[i] <= guard_end[i]);
						if (guard_begin[i] != guard_end[i]) {
							struct asm_exc *hand;
							struct asm_sym *except_begin;
							struct asm_sym *except_end;
							except_begin = asm_newsym();
							if unlikely(!except_begin)
								goto err;
							except_end = asm_newsym();
							if unlikely(!except_end)
								goto err;
							except_begin->as_sect = i;
							except_end->as_sect   = i;
							except_begin->as_stck = ASM_SYM_STCK_INVALID;
							except_end->as_stck   = ASM_SYM_STCK_INVALID;
							except_begin->as_hand = current_assembler.a_handlerc;
							except_end->as_hand   = current_assembler.a_handlerc;
							except_begin->as_addr = guard_begin[i];
							except_end->as_addr   = guard_end[i];
							hand = asm_newexc();
							if unlikely(!hand)
								goto err;
							hand->ex_mask  = NULL;
							hand->ex_start = except_begin;
							hand->ex_end   = except_end;
							hand->ex_addr  = Linit_try_except;
							hand->ex_flags = EXCEPTION_HANDLER_FFINALLY;
							++except_begin->as_used;
							++except_end->as_used;
							++Linit_try_except->as_used;
							current_basescope->bs_flags |= CODE_FFINALLY;
						}
					}

					/* Set-up context for the except case. */
					old_sect = current_assembler.a_sect;
					current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
					Lafter_except = NULL;
					if (old_sect == current_assembler.a_curr) {
						Lafter_except = asm_newsym();
						if unlikely(!Lafter_except)
							goto err;
						if unlikely(asm_gjmp(ASM_JMP, Lafter_except))
							goto err;
					}
					asm_decsp();
					if (guard_finflags & ASM_FINFLAG_USED) {
						asm_incsp();
						asm_incsp();
					}
					asm_defsym(Linit_try_except);
					if (asm_putddi(ddi_ast))
						goto err;
					if unlikely(asm_gdel_static((uint16_t)sid))
						goto err;
					if (guard_finflags & ASM_FINFLAG_USED) {
						if unlikely(asm_gendfinally())
							goto err;
						if (asm_gjmp_pop_pop())
							goto err;
					} else {
						if unlikely(asm_gendfinally_except())
							goto err;
					}

					/* Restore context for the non-except case. */
					current_assembler.a_curr = old_sect;
					asm_incsp();
					if (Lafter_except)
						asm_defsym(Lafter_except);
				}

				/* Store initializer result into the static variable. */
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gpop_static((uint16_t)sid))
					goto err;

				/* This is where we jump when the static was already initialized. */
				asm_defsym(Lalready_init);
				goto push_static_as_result;
			}

			/* Simple case: can always construct the initializer and
			 *              assign it if it hasn't been assigned already. */
			if (!(current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) &&
			    !ast_is_nothrow(src, true)) {
				/* Source expression has no side-effects, but can throw.
				 *
				 * This probably means that the initialize needs to construct
				 * a new object (even though that constructor has no observable
				 * side-effects). But for the sake of performance, we don't
				 * want to create an object every time, only to throw it away
				 * most of the time.
				 *
				 * As such, generate code like this:
				 * >>     push   bound static @x
				 * >>     jt     pop, 1f
				 * >>     push   @initializer
				 * >>     push   cmpxch static @x, unbound, pop
				 * >>     pop
				 * >> 1: */
				struct asm_sym *Lalready_init;
				Lalready_init = asm_newsym();
				if unlikely(!Lalready_init)
					goto err;
				if (asm_putddi(dst))
					goto err;
				if (asm_gpush_bnd_static((uint16_t)sid))
					goto err;
				if (asm_gjmp(ASM_JT, Lalready_init))
					goto err;
				asm_decsp();
				if (ast_genasm_one(src, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_pstatic((uint16_t)sid))
					goto err;
				if (asm_gcmpxch_ub_pop_p())
					goto err;
				if (asm_gpop())
					goto err;
				asm_defsym(Lalready_init);
				goto push_static_as_result;
			}
			if (src->a_type == AST_CONSTEXPR) {
				/* Special case: can use "push cmpxch PREFIX, unbound, const <imm8/16>" */
				int32_t cid = asm_newconst(src->a_constexpr);
				if unlikely(cid < 0)
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_pstatic((uint16_t)sid))
					goto err;
				if (asm_gcmpxch_ub_c_p((uint16_t)cid))
					goto err;
			} else {
				if (ast_genasm_one(src, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_pstatic((uint16_t)sid))
					goto err;
				if (asm_gcmpxch_ub_pop_p())
					goto err;
			}
			if (asm_gpop())
				goto err;
push_static_as_result:
			if (PUSH_RESULT) {
				if (asm_putddi(dst))
					goto err;
				if (asm_gpush_static((uint16_t)sid))
					goto err;
			}
			return 0;
		}	break;

		case SYMBOL_TYPE_GLOBAL:
		case SYMBOL_TYPE_EXTERN:
		case SYMBOL_TYPE_LOCAL:
		case SYMBOL_TYPE_STACK:
			if (!PUSH_RESULT && asm_can_prefix_symbol(dst_sym))
				return asm_gmov_sym_ast(dst_sym, src, dst);
			if (ast_genasm(src, ASM_G_FPUSHRES))
				goto err;
			if (PUSH_RESULT) {
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gdup())
					goto err;
			}
			if (asm_putddi(dst))
				goto err;
			return asm_gpop_symbol(dst_sym, dst);

		default: break;
		}

		/* Special instructions that allow a symbol prefix to specify the target. */
		if (!PUSH_RESULT && asm_can_prefix_symbol(dst_sym))
			return asm_gmov_sym_ast(dst_sym, src, dst);

		if (src->a_type == AST_SYM && !PUSH_RESULT &&
		    asm_can_prefix_symbol_for_read(src->a_sym)) {
			int32_t symid;
check_dst_sym_class_hybrid:
			if (!SYMBOL_MUST_REFERENCE(dst_sym)) {
				switch (dst_sym->s_type) {

				case SYMBOL_TYPE_ALIAS:
					dst_sym = dst_sym->s_alias;
					goto check_dst_sym_class_hybrid;

				case SYMBOL_TYPE_GLOBAL:
					/* mov global <imm8/16>, PREFIX */
					symid = asm_gsymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_putddi(dst))
						goto err;
					if (asm_gprefix_symbol_for_read(src->a_sym, src))
						goto err;
					return asm_gpop_global_p((uint16_t)symid);

				case SYMBOL_TYPE_LOCAL:
					/* mov local <imm8/16>, PREFIX */
					symid = asm_lsymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_putddi(dst))
						goto err;
					if (asm_gprefix_symbol_for_read(src->a_sym, src))
						goto err;
					return asm_gpop_local_p((uint16_t)symid);

				case SYMBOL_TYPE_STATIC:
					/* mov static <imm8/16>, PREFIX */
					symid = asm_ssymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_putddi(dst))
						goto err;
					if (asm_gprefix_symbol_for_read(src->a_sym, src))
						goto err;
					return asm_gpop_static_p((uint16_t)symid);

				case SYMBOL_TYPE_EXTERN:
					/* mov extern <imm8/16>, PREFIX */
					if (SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_flags &
					    (MODSYM_FREADONLY | MODSYM_FPROPERTY))
						break;
					symid = asm_esymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_putddi(dst))
						goto err;
					if (asm_gprefix_symbol_for_read(src->a_sym, src))
						goto err;
					return asm_gpop_extern_p((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_index);

				case SYMBOL_TYPE_STACK:
					if (!(dst_sym->s_flag & SYMBOL_FALLOC))
						break;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gprefix_symbol_for_read(src->a_sym, src))
						goto err;
					if (SYMBOL_STACK_OFFSET(dst_sym) == current_assembler.a_stackcur - 1) {
						/* mov top, PREFIX */
						return asm_gpop_p();
					} else {
						/* mov #<imm8/16>, PREFIX */
						uint16_t offset;
						offset = (uint16_t)((current_assembler.a_stackcur -
						                     SYMBOL_STACK_OFFSET(dst_sym)) -
						                    1);
						return asm_gpop_n_p(offset);
					}
					break;

				default: break;
				}
			}
		}
	}	break;


	case AST_CONSTEXPR:
		/* Check for special case: store into a constant
		 * expression `none' is the same as `pop' */
		if (!DeeNone_Check(dst->a_constexpr))
			break;
		/* Store-to-none is a no-op (so just assembly the source-expression). */
		return ast_genasm(src, gflags);

	case AST_OPERATOR:
		switch (dst->a_flag) {

		case OPERATOR_GETATTR:
			if unlikely(!dst->a_operator.o_op1)
				break;
			return ast_gen_setattr(dst->a_operator.o_op0,
			                       dst->a_operator.o_op1,
			                       src, ddi_ast, gflags);

		case OPERATOR_GETITEM:
			if unlikely(!dst->a_operator.o_op1)
				break;
			return ast_gen_setitem(dst->a_operator.o_op0,
			                       dst->a_operator.o_op1,
			                       src, ddi_ast, gflags);

		case OPERATOR_GETRANGE:
			if unlikely(!dst->a_operator.o_op2)
				break;
			return ast_gen_setrange(dst->a_operator.o_op0,
			                        dst->a_operator.o_op1,
			                        dst->a_operator.o_op2,
			                        src, ddi_ast, gflags);

		default: break;
		}
		break;

	case AST_MULTIPLE:
		/* Special handling for unpack expressions (i.e. `(a, b, c) = foo()'). */
		if (dst->a_flag == AST_FMULTIPLE_KEEPLAST) {
			size_t i = 0;
			if (dst->a_multiple.m_astc == 0)
				return ast_genasm(src, gflags);
			/* Compile all branches except for the last one normally. */
			if (dst->a_multiple.m_astc > 1) {
				ASM_PUSH_SCOPE(dst->a_scope, err);
				for (; i < dst->a_multiple.m_astc - 1; ++i) {
					if (ast_genasm(dst->a_multiple.m_astv[i], ASM_G_FNORMAL))
						goto err;
				}
				ASM_POP_SCOPE(0, err);
			}
			/* The last branch is the one we're going to write to. */
			dst = dst->a_multiple.m_astv[i];
			goto again;
		}
		/* Optimization for special case: (a, b, c) = (d, e, f); */
		if (src->a_type == AST_MULTIPLE &&
		    src->a_flag != AST_FMULTIPLE_KEEPLAST &&
		    src->a_multiple.m_astc == dst->a_multiple.m_astc &&
		    !ast_chk_multiple_hasexpand(src) && !PUSH_RESULT) {
			size_t i;
			for (i = 0; i < src->a_multiple.m_astc; ++i) {
				if (asm_gstore(dst->a_multiple.m_astv[i],
				               src->a_multiple.m_astv[i],
				               ddi_ast, gflags))
					goto err;
			}
			goto done;
		}
		if (src->a_type == AST_CONSTEXPR) {
			/* TODO: Optimizations for `none'. */
			/* TODO: Optimizations for sequence constants. */
		}
		break;

	default: break;
	}
	/* TODO: Special handling when the stored value uses the target:
	 *  >> local myList = [10, 20, myList]; // Should create a self-referencing list.
	 * ASM:
	 *  >>    pack List 0
	 *  >>    pop  local @myList
	 *  >>    push local @myList  // May be optimized to a dup
	 *  >>    push $10
	 *  >>    push $20
	 *  >>    push local @myList
	 *  >>    pack List 3
	 *  >>    move assign top, pop // Move-assign the second list.
	 * Essentially, this would look like this:
	 *  >> local myList = [];
	 *  >> myList := [10, 20, myList]; */
	if (asm_gpop_expr_enter(dst))
		goto err;
	if (ast_genasm(src, ASM_G_FPUSHRES))
		goto err;
	if (asm_gpop_expr_leave(dst, gflags))
		goto err;
done:
	return 0;
err:
	return -1;
}

#undef PUSH_RESULT

INTERN WUNUSED int DCALL
asm_gpop_expr_multiple(size_t astc, struct ast **astv) {
	size_t i, j;

	/* Optimization: Trailing asts with no side-effects can be handled in reverse order */
	while (astc && !ast_has_sideeffects(astv[astc - 1])) {
		--astc; /* This way we don't have to rotate stack-elements. */
		if (asm_gpop_expr(astv[astc]))
			goto err;
	}
	if (!astc)
		goto done;
	i = 0;

	/* Find the first AST that does actually have side-effects. */
	while (i < astc && !ast_has_sideeffects(astv[i]))
		++i;
	for (j = i; j < astc; ++j) {
		if (asm_glrot((uint16_t)(astc - j)))
			goto err;
		if (asm_gpop_expr(astv[j]))
			goto err;
	}

	/* Handle leading asts without side-effects in reverse order. */
	while (i--) {
		if (asm_gpop_expr(astv[i]))
			goto err;
	}
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
asm_gpop_expr(struct ast *__restrict self) {
	switch (self->a_type) {

	case AST_SYM:
		if (asm_putddi(self))
			goto err;
		return asm_gpop_symbol(self->a_sym, self);

	case AST_MULTIPLE:
		if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
			size_t i;
			/* Special case: Store to KEEP-last multiple AST:
			 *               Evaluate all branches but the last without using them.
			 *               Then simply write the value to the last expression. */
			if (self->a_multiple.m_astc == 0)
				return asm_gpop();
			i = 0;
			if (self->a_multiple.m_astc > 1) {
				ASM_PUSH_SCOPE(self->a_scope, err);
				for (; i < self->a_multiple.m_astc - 1; ++i) {
					if (ast_genasm(self->a_multiple.m_astv[i], ASM_G_FNORMAL))
						goto err;
				}
				ASM_POP_SCOPE(0, err);
			}
			ASSERT(i == self->a_multiple.m_astc - 1);
			return asm_gpop_expr(self->a_multiple.m_astv[i]);
		}
		if (asm_putddi(self))
			goto err;
		if (asm_gunpack((uint16_t)self->a_multiple.m_astc))
			goto err;
		if (asm_gpop_expr_multiple(self->a_multiple.m_astc,
		                           self->a_multiple.m_astv))
			goto err;
		break;

	case AST_OPERATOR:
		switch (self->a_flag) {
		case OPERATOR_GETATTR: {
			struct ast *attr;
			if unlikely((attr = self->a_operator.o_op1) == NULL)
				break;
			if (attr->a_type == AST_CONSTEXPR &&
			    DeeString_Check(attr->a_constexpr)) {
				int32_t cid      = asm_newconst(attr->a_constexpr);
				struct ast *base = self->a_operator.o_op0;
				if unlikely(cid < 0)
					goto err;
				if (base->a_type == AST_SYM) {
					struct symbol *sym = SYMBOL_UNWIND_ALIAS(base->a_sym);
					if (sym->s_type == SYMBOL_TYPE_THIS &&
					    !SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) {
						if (asm_putddi(self))
							goto err;
						if (asm_gsetattr_this_const((uint16_t)cid))
							goto err;
						goto done;
					}
				}
				if (ast_genasm_one(base, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(self))
					goto err;
				if (asm_gswap())
					goto err;
				if (asm_gsetattr_const((uint16_t)cid))
					goto err;
				goto done;
			}
			if (ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(self))
				goto err;
			if (asm_glrot(3))
				goto err;
			if (asm_gsetattr())
				goto err;
			goto done;
		}

		case OPERATOR_GETITEM: {
			struct ast *index;
			if unlikely((index = self->a_operator.o_op1) == NULL)
				break;
			if (ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES))
				goto err;
			if (index->a_type == AST_CONSTEXPR) {
				int32_t int_index;
				/* Special optimizations for constant indices. */
				if (DeeInt_Check(index->a_constexpr) &&
				    DeeInt_TryAsInt32(index->a_constexpr, &int_index) &&
				    int_index >= INT16_MIN && int_index <= INT16_MAX) {
					if (asm_putddi(self))
						goto err;
					if (asm_gswap())
						goto err;
					if (asm_gsetitem_index((int16_t)int_index))
						goto err;
					goto done;
				}
				if (asm_allowconst(index->a_constexpr)) {
					int_index = asm_newconst(index->a_constexpr);
					if unlikely(int_index < 0)
						goto err;
					if (asm_putddi(self))
						goto err;
					if (asm_gswap())
						goto err;
					if (asm_gsetitem_const((uint16_t)int_index))
						goto err;
					goto done;
				}
			}
			if (ast_genasm_one(index, ASM_G_FPUSHRES))
				goto err; /* STACK: item, base, index */
			if (asm_putddi(self))
				goto err;
			if (asm_glrot(3))
				goto err; /* STACK: base, index, item */
			if (asm_gsetitem())
				goto err; /* STACK: - */
			goto done;
		}

		case OPERATOR_GETRANGE: {
			struct ast *begin, *end;
			int32_t index;
			if unlikely(!self->a_operator.o_op2)
				break;
			if (ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES))
				goto err;
			begin = self->a_operator.o_op1;
			end   = self->a_operator.o_op2;
			/* Special optimizations for certain ranges. */
			if (begin->a_type == AST_CONSTEXPR) {
				DeeObject *begin_index = begin->a_constexpr;
				if (DeeNone_Check(begin_index)) {
					/* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
					if (end->a_type == AST_CONSTEXPR &&
					    DeeInt_Check(end->a_constexpr) &&
					    DeeInt_TryAsInt32(end->a_constexpr, &index) &&
					    index >= INT16_MIN && index <= INT16_MAX) {
						/* `setrange pop, none, $<Simm16>, pop' */
						if (asm_putddi(self))
							goto err;
						if (asm_gswap())
							goto err; /* STACK: base, item */
						if (asm_gsetrange_ni((int16_t)index))
							goto err;
						goto done;
					}
					/* `setrange pop, none, pop, pop' */
					if (ast_genasm_one(end, ASM_G_FPUSHRES))
						goto err; /* STACK: item, base, end */
					if (asm_putddi(self))
						goto err;
					if (asm_glrot(3))
						goto err; /* STACK: base, end, item */
					if (asm_gsetrange_np())
						goto err; /* STACK: - */
					goto done;
				}
				if (DeeInt_Check(begin_index) &&
				    DeeInt_TryAsInt32(begin_index, &index) &&
				    index >= INT16_MIN && index <= INT16_MAX) {
					if (end->a_type == AST_CONSTEXPR) {
						int32_t index2;
						DeeObject *end_index = end->a_constexpr;
						if (DeeNone_Check(end_index)) {
							/* `setrange pop, $<Simm16>, none, pop' */
							if (asm_putddi(self))
								goto err;
							if (asm_gswap())
								goto err; /* STACK: base, item */
							if (asm_gsetrange_in((int16_t)index))
								goto err; /* STACK: - */
							goto done;
						}
						if (DeeInt_Check(end_index) &&
						    DeeInt_TryAsInt32(end_index, &index2) &&
						    index2 >= INT16_MIN && index2 <= INT16_MAX) {
							/* `setrange pop, $<Simm16>, $<Simm16>, pop' */
							if (asm_putddi(self))
								goto err;
							if (asm_gswap())
								goto err; /* STACK: base, item */
							if (asm_gsetrange_ii((int16_t)index, (int16_t)index2))
								goto err; /* STACK: - */
							goto done;
						}
					}
					if (ast_genasm_one(end, ASM_G_FPUSHRES))
						goto err; /* STACK: item, base, end */
					if (asm_putddi(self))
						goto err;
					if (asm_glrot(3))
						goto err; /* STACK: base, end, item */
					if (asm_gsetrange_ip((int16_t)index))
						goto err; /* STACK: - */
					goto done;
				}
			} else if (end->a_type == AST_CONSTEXPR) {
				/* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
				DeeObject *end_index = end->a_constexpr;
				if (DeeNone_Check(end_index)) {
					/* `setrange pop, pop, none, pop' */
					if (ast_genasm_one(begin, ASM_G_FPUSHRES))
						goto err; /* STACK: item, base, begin */
					if (asm_putddi(self))
						goto err;
					if (asm_glrot(3))
						goto err; /* STACK: base, begin, item */
					if (asm_gsetrange_pn())
						goto err; /* STACK: - */
					goto done;
				}
				if (DeeInt_Check(end_index) &&
				    DeeInt_TryAsInt32(end_index, &index) &&
				    index >= INT16_MIN && index <= INT16_MAX) {
					/* `setrange pop, pop, $<Simm16>, pop' */
					if (ast_genasm_one(begin, ASM_G_FPUSHRES))
						goto err; /* STACK: item, base, begin */
					if (asm_putddi(self))
						goto err;
					if (asm_glrot(3))
						goto err; /* STACK: base, begin, item */
					if (asm_gsetrange_pi((int16_t)index))
						goto err; /* STACK: - */
					goto done;
				}
			}
			if (ast_genasm_one(begin, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(end, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(self))
				goto err;
			/* STACK: item, base, begin, end */
			if (asm_glrot(4))
				goto err; /* STACK: base, begin, end, item */
			if (asm_gsetrange())
				goto err; /* STACK: - */
			goto done;
		}

		default: break;
		}
		goto default_case;

	case AST_CONSTEXPR:
		/* Check for special case: store into a constant
		 * expression `none' is the same as `pop' */
		if (!DeeNone_Check(self->a_constexpr))
			goto default_case;
		if (asm_gpop())
			goto err;
		break;

	default:
default_case:
		/* Emit a warning about an r-value store. */
		if (WARNAST(self, W_ASM_STORE_TO_RVALUE))
			goto err;
		/* Fallback: Generate the ast and store it directly. */
		if (ast_genasm_one(self, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(self))
			goto err;
		if (asm_gswap())
			goto err;
		if (asm_gassign())
			goto err;
		break;
	}
done:
	return 0;
err:
	return -1;
}

DECL_END

#ifndef __INTELLISENSE__
#undef ENTER
#undef LEAVE
#define ENTER 1
#include "genstore-setup.c.inl"
#define LEAVE 1
#include "genstore-setup.c.inl"
#endif

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENSTORE_C */
