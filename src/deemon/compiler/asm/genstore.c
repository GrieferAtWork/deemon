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

#define DO(expr) if unlikely(expr) goto err

#define PUSH_RESULT (gflags & ASM_G_FPUSHRES)

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
asm_gpush2_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict ddi_ast,
                   unsigned int gflags) {
	if (PUSH_RESULT) {
		if (ast_can_exchange(b, a)) {
			DO(ast_genasm(b, ASM_G_FPUSHRES));
			DO(ast_genasm_one(a, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup_n(2 - 2));
		} else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
			DO(ast_genasm(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES)); /* a, b */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* a, b, b */
			DO(asm_grrot(3)); /* b, a, b */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_none());
			DO(ast_genasm(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES)); /* none, a, b */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* none, a, b, b */
			DO(asm_gpop_n(4 - 2)); /* b,    a, b */
		}
	} else {
		DO(ast_genasm(a, ASM_G_FPUSHRES));
		DO(ast_genasm_one(b, ASM_G_FPUSHRES));
		DO(asm_putddi(ddi_ast));
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
			DO(ast_genasm(c, ASM_G_FPUSHRES));
			DO(ast_genasm_one(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup_n(3 - 2));
		} else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
			DO(ast_genasm(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES));
			DO(ast_genasm_one(c, ASM_G_FPUSHRES)); /* a, b, c */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* a, b, c, c */
			DO(asm_grrot(4)); /* c, a, b, c */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_none());
			DO(ast_genasm(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES));
			DO(ast_genasm_one(c, ASM_G_FPUSHRES)); /* none, a, b, c */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* none, a, b, c, c */
			DO(asm_gpop_n(5 - 2)); /* c,    a, b, c */
		}
	} else {
		DO(ast_genasm(a, ASM_G_FPUSHRES));
		DO(ast_genasm_one(b, ASM_G_FPUSHRES));
		DO(ast_genasm_one(c, ASM_G_FPUSHRES));
		DO(asm_putddi(ddi_ast));
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
			DO(ast_genasm(d, ASM_G_FPUSHRES));
			DO(ast_genasm_one(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES));
			DO(ast_genasm_one(c, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup_n(4 - 2));
		} else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
			DO(ast_genasm(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES));
			DO(ast_genasm_one(c, ASM_G_FPUSHRES));
			DO(ast_genasm_one(d, ASM_G_FPUSHRES)); /* a, b, c, d */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* a, b, c, d, d */
			DO(asm_grrot(5)); /* d, a, b, c, d */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_none());
			DO(ast_genasm(a, ASM_G_FPUSHRES));
			DO(ast_genasm_one(b, ASM_G_FPUSHRES));
			DO(ast_genasm_one(c, ASM_G_FPUSHRES));
			DO(ast_genasm_one(d, ASM_G_FPUSHRES)); /* none, a, b, c, d */
			DO(asm_putddi(ddi_ast));
			DO(asm_gdup()); /* none, a, b, c, d, d */
			DO(asm_gpop_n(6 - 2)); /* d,    a, b, c, d */
		}
	} else {
		DO(ast_genasm(a, ASM_G_FPUSHRES));
		DO(ast_genasm_one(b, ASM_G_FPUSHRES));
		DO(ast_genasm_one(c, ASM_G_FPUSHRES));
		DO(ast_genasm_one(d, ASM_G_FPUSHRES));
		DO(asm_putddi(ddi_ast));
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
							DO(asm_putddi(ddi_ast));
							DO(asm_gpush_symbol(classsym, ddi_ast));
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
				DO(asm_putddi(ddi_ast));
				DO(asm_ggetattr_this_const((uint16_t)attrid));
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
				DO(asm_putddi(ddi_ast));
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
				DO(asm_putddi(ddi_ast));
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
				DO(asm_putddi(ddi_ast));
				DO(asm_gsupergetattr_this_rc((uint16_t)type_rid, (uint16_t)attrid));
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
		DO(ast_genasm(base, ASM_G_FPUSHRES));
		attrid = asm_newconst((DeeObject *)attrname);
		if unlikely(attrid < 0)
			goto err;
		DO(asm_putddi(ddi_ast));
		DO(asm_ggetattr_const((uint16_t)attrid));
		goto pop_unused;
	}
	DO(ast_genasm(base, ASM_G_FPUSHRES));
	DO(ast_genasm_one(name, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	DO(asm_ggetattr());
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
							DO(asm_putddi(ddi_ast));
							DO(asm_gpush_bnd_symbol(classsym, ddi_ast));
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
					DO(asm_putddi(ddi_ast));
					return asm_gpush_false();
				}
				symid = asm_gsymid_for_read(globsym, ddi_ast);
				if unlikely(symid < 0)
					goto err;
				DO(asm_putddi(ddi_ast));
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
				DO(asm_putddi(ddi_ast));
				return asm_gpush_bnd_extern((uint16_t)module_id, modsym->ss_index);
			}	break;

			default: break;
			}
		}
	}
	DO(ast_genasm(base, ASM_G_FPUSHRES));
	DO(ast_genasm_one(name, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	DO(asm_gboundattr());
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
							DO(asm_putddi(ddi_ast));
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
				DO(asm_putddi(ddi_ast));
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
				DO(asm_putddi(ddi_ast));
				return asm_gdel_global((uint16_t)symid);
			}	break;

			default: break;
			}
		}
		attrid = asm_newconst(name->a_constexpr);
		if unlikely(attrid < 0)
			goto err;
		DO(ast_genasm(base, ASM_G_FPUSHRES));
		DO(asm_putddi(ddi_ast));
		return asm_gdelattr_const((uint16_t)attrid);
	}
	DO(ast_genasm(base, ASM_G_FPUSHRES));
	DO(ast_genasm_one(name, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
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
			if (PUSH_RESULT)
				DO(ast_genasm(value, ASM_G_FPUSHRES));
			DO(asm_putddi(ddi_ast));
			if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				DO(asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET)); /* [result], func */
			} else {
				DO(asm_gpush_symbol(class_sym, ddi_ast));              /* [result], class_sym */
				DO(asm_ggetcmember(attr->ca_addr + CLASS_GETSET_SET)); /* [result], func */
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				DO(asm_gpush_symbol(class_sym, ddi_ast)); /* [result], setter, class */
				if (PUSH_RESULT) {
					DO(asm_gdup_n(1)); /* result, setter, class, value */
				} else {
					DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* setter, class, value */
					DO(asm_putddi(ddi_ast));
				}
				DO(asm_gcall(2)); /* [result], discard */
			} else {
				if (PUSH_RESULT) {
					DO(asm_gdup_n(0)); /* result, setter, value */
				} else {
					DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* setter, value */
					DO(asm_putddi(ddi_ast));
				}
				DO(asm_gcall(1)); /* [result], discard */
			}
			return asm_gpop();
		}
		if (PUSH_RESULT) {
			DO(ast_genasm(value, ASM_G_FPUSHRES)); /* result */
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(class_sym, ddi_ast)); /* result, class */
			DO(asm_gdup_n(0));                        /* result, class, value */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(class_sym, ddi_ast));  /* class */
			DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* class, value */
			DO(asm_putddi(ddi_ast));
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
			/* XXX: Assert that not already bound? */
		}
		DO(asm_gdefcmember(attr->ca_addr)); /* [result], class */
		return asm_gpop();                  /* [result] */
	}

	/* Check if the attribute must be accessed as virtual. */
	DO(asm_check_thiscall(sym, ddi_ast));
	SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
	if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
do_virtual_access:
		symid = asm_newconst((DeeObject *)attr->ca_name);
		if unlikely(symid < 0)
			goto err;
		if (this_sym->s_type == SYMBOL_TYPE_THIS &&
		    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
			DO(ast_genasm(value, ASM_G_FPUSHRES)); /* value */
			DO(asm_putddi(ddi_ast));
			if (PUSH_RESULT)
				DO(asm_gdup());                              /* [result], value */
			return asm_gsetattr_this_const((uint16_t)symid); /* [result] */
		}
		if (PUSH_RESULT) {
			DO(ast_genasm(value, ASM_G_FPUSHRES)); /* result */
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(this_sym, ddi_ast)); /* result, this_sym */
			DO(asm_gdup_n(0)); /* result, this_sym, value */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(this_sym, ddi_ast)); /* this_sym */
			DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* this_sym, value */
			DO(asm_putddi(ddi_ast));
		}
		return asm_gsetattr_const((uint16_t)symid); /* [result] */
	}

	/* Regular, old member variable. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto fallback;
		/* Call the setter function of the attribute. */
		if (PUSH_RESULT)
			DO(ast_genasm(value, ASM_G_FPUSHRES)); /* value */
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
					if (!PUSH_RESULT)
						DO(ast_genasm(value, ASM_G_FPUSHRES)); /* value */
					DO(asm_putddi(ddi_ast));
					if (PUSH_RESULT)
						DO(asm_gdup()); /* [result], value */
					DO(asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET, 0));
					goto pop_unused_result;
				}
				DO(asm_putddi(ddi_ast));
				DO(asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET)); /* [result], setter */
			} else {
				DO(asm_putddi(ddi_ast));
				DO(asm_gpush_symbol(class_sym, ddi_ast)); /* [result], class_sym */
				DO(asm_ggetcmember(attr->ca_addr + CLASS_GETSET_SET)); /* [result], setter */
			}
		} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
		           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(this_sym, ddi_ast));              /* [result], this_sym */
			DO(asm_gpush_symbol(class_sym, ddi_ast));             /* [result], this_sym, class_sym */
			DO(asm_ggetmember(attr->ca_addr + CLASS_GETSET_SET)); /* [result], setter */
		} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
			symid = asm_rsymid(class_sym);
			if unlikely(symid < 0)
				goto err;
			DO(asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET)); /* [result], setter */
		} else {
			DO(asm_gpush_symbol(class_sym, ddi_ast)); /* [result], class_sym */
			DO(asm_ggetmember_this(attr->ca_addr + CLASS_GETSET_SET)); /* [result], setter */
		}
		/* [result], setter */
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
			if (PUSH_RESULT) {
				DO(asm_gdup_n(0)); /* result, setter, value */
			} else {
				DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* setter, value */
				DO(asm_putddi(ddi_ast));
			}
			DO(asm_gcall(1)); /* [result], discard */
		} else {
			/* Invoke as a this-call. */
			DO(asm_gpush_symbol(this_sym, ddi_ast)); /* [result], setter, this */
			if (PUSH_RESULT) {
				DO(asm_gdup_n(1)); /* result, setter, this, value */
			} else {
				DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* setter, this, value */
				DO(asm_putddi(ddi_ast));
			}
			DO(asm_gcall(2)); /* [result], discard */
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
			DO(ast_genasm(value, ASM_G_FPUSHRES)); /* result */
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(this_sym, ddi_ast));  /* result, this */
			DO(asm_gpush_symbol(class_sym, ddi_ast)); /* result, this, class */
			DO(asm_gdup_n(1));                        /* result, this, class, value */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(this_sym, ddi_ast));   /* this */
			DO(asm_gpush_symbol(class_sym, ddi_ast));  /* this, class */
			DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* this, class, value */
			DO(asm_putddi(ddi_ast));
		}
		DO(asm_gsetmember(attr->ca_addr)); /* [result] */
	} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
		symid = asm_rsymid(class_sym);
		if unlikely (symid < 0)
			goto err;
		DO(ast_genasm(value, ASM_G_FPUSHRES)); /* value */
		DO(asm_putddi(ddi_ast));
		if (PUSH_RESULT)
			DO(asm_gdup()); /* [result], value */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
			DO(asm_gsetmemberi_this_r((uint16_t)symid, attr->ca_addr)); /* [result] */
		} else {
			DO(asm_gsetmember_this_r((uint16_t)symid, attr->ca_addr)); /* [result] */
		}
	} else {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto do_virtual_access; /* There is no `setmemberi this, pop, $<imm8>, pop' instruction, so use fallback */
		if (PUSH_RESULT) {
			DO(ast_genasm(value, ASM_G_FPUSHRES)); /* result */
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(class_sym, ddi_ast)); /* result, class */
			DO(asm_gdup_n(0)); /* result, class, value */
		} else {
			DO(asm_putddi(ddi_ast));
			DO(asm_gpush_symbol(class_sym, ddi_ast)); /* class */
			DO(ast_genasm_one(value, ASM_G_FPUSHRES)); /* class, value */
			DO(asm_putddi(ddi_ast));
		}
		DO(asm_gsetmember_this(attr->ca_addr)); /* [result] */
	}
	return 0;
fallback:
	DO(ast_genasm(value, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
	if (gflags & ASM_G_FPUSHRES)
		DO(asm_gdup());
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
				DO(ast_genasm(value, ASM_G_FPUSHRES));
				DO(asm_putddi(ddi_ast));
				if (PUSH_RESULT)
					DO(asm_gdup());
				DO(asm_gsetattr_this_const((uint16_t)cid));
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
				DO(asm_putddi(ddi_ast));
				if (PUSH_RESULT)
					DO(asm_gdup());
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
				DO(ast_genasm(value, ASM_G_FPUSHRES));
				DO(asm_putddi(ddi_ast));
				if (PUSH_RESULT)
					DO(asm_gdup());
				if (modsym->ss_flags & MODSYM_FPROPERTY) {
					/* Invoke the setter callback. */
					DO(asm_gcall_extern((uint16_t)module_id, modsym->ss_index + MODULE_PROPERTY_SET, 1));
					return asm_gpop();
				}
				return asm_gpop_extern((uint16_t)module_id, modsym->ss_index);
			}	break;

			default:
				break;
			}
		}
		DO(asm_gpush2_duplast(base, value, ddi_ast, gflags));
		cid = asm_newconst(name->a_constexpr);
		if unlikely(cid < 0)
			goto err;
		DO(asm_gsetattr_const((uint16_t)cid));
		goto done;
	}
	DO(asm_gpush3_duplast(base, name, value, ddi_ast, gflags));
	DO(asm_gsetattr());
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
		/* Special optimizations for constant indices. */
		int16_t int_index;
		if (DeeInt_Check(index->a_constexpr) &&
		    DeeInt_TryAsInt16(index->a_constexpr, &int_index)) {
			DO(asm_gpush2_duplast(sequence, value, ddi_ast, gflags));
			DO(asm_gsetitem_index(int_index));
			goto done;
		}
		if (asm_allowconst(index->a_constexpr)) {
			int32_t cid = asm_newconst(index->a_constexpr);
			if unlikely(cid < 0)
				goto err;
			DO(asm_gpush2_duplast(sequence, value, ddi_ast, gflags));
			DO(asm_gsetitem_const((uint16_t)cid));
			goto done;
		}
	}
	DO(asm_gpush3_duplast(sequence, index, value, ddi_ast, gflags));
	DO(asm_gsetitem());
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
	int16_t int_index;
	/* Special optimizations for certain ranges. */
	if (begin->a_type == AST_CONSTEXPR) {
		DeeObject *begin_index = begin->a_constexpr;
		if (DeeNone_Check(begin_index)) {
			/* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
			if (end->a_type == AST_CONSTEXPR &&
			    DeeInt_Check(end->a_constexpr) &&
			    DeeInt_TryAsInt16(end->a_constexpr, &int_index)) {
				/* `setrange pop, none, $<Simm16>, pop' */
				DO(asm_gpush2_duplast(sequence, value, ddi_ast, gflags));
				DO(asm_gsetrange_ni(int_index));
				goto done;
			}
			/* `setrange pop, none, pop, pop' */
			DO(asm_gpush3_duplast(sequence, end, value, ddi_ast, gflags));
			DO(asm_gsetrange_np());
			goto done;
		}
		if (DeeInt_Check(begin_index) &&
		    DeeInt_TryAsInt16(begin_index, &int_index)) {
			if (end->a_type == AST_CONSTEXPR) {
				int16_t int_index2;
				DeeObject *end_index = end->a_constexpr;
				if (DeeNone_Check(end_index)) {
					/* `setrange pop, $<Simm16>, none, pop' */
					DO(asm_gpush2_duplast(sequence, value, ddi_ast, gflags));
					DO(asm_gsetrange_in(int_index));
					goto done;
				}
				if (DeeInt_Check(end_index) &&
				    DeeInt_TryAsInt16(end_index, &int_index2)) {
					/* `setrange pop, $<Simm16>, $<Simm16>, pop' */
					DO(asm_gpush2_duplast(sequence, value, ddi_ast, gflags));
					DO(asm_gsetrange_ii(int_index, int_index2));
					goto done;
				}
			}
			DO(asm_gpush3_duplast(sequence, end, value, ddi_ast, gflags));
			DO(asm_gsetrange_ip(int_index));
			goto done;
		}
	} else if (end->a_type == AST_CONSTEXPR) {
		/* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
		DeeObject *end_index = end->a_constexpr;
		if (DeeNone_Check(end_index)) {
			/* `setrange pop, pop, none, pop' */
			DO(asm_gpush3_duplast(sequence, begin, value, ddi_ast, gflags));
			DO(asm_gsetrange_pn());
			goto done;
		}
		if (DeeInt_Check(end_index) && DeeInt_TryAsInt16(end_index, &int_index)) {
			/* `setrange pop, pop, $<Simm16>, pop' */
			DO(asm_gpush3_duplast(sequence, begin, value, ddi_ast, gflags));
			DO(asm_gsetrange_pi(int_index));
			goto done;
		}
	}
	DO(asm_gpush4_duplast(sequence, begin, end, value, ddi_ast, gflags));
	DO(asm_gsetrange());
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
			DO(asm_putddi(ddi_ast));
			return asm_gvarargs_unpack(num_targets);
		}
		if (asm_can_prefix_symbol_for_read(sym)) {
			/* The unpack instructions can make use of an object prefix. */
			DO(asm_putddi(ddi_ast));
			DO(asm_gprefix_symbol_for_read(sym, src));
			return asm_gunpack_p(num_targets);
		}
	}

	/* Fallback: generate regular assembly for `src' and use `ASM_UNPACK' */
	DO(ast_genasm(src, ASM_G_FPUSHRES));
	DO(asm_putddi(ddi_ast));
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
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
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
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
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
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_except_p();

	case SYMBOL_TYPE_MODULE:
		/* mov PREFIX, module <imm8/16> */
		symid = asm_msymid(src_sym);
		if unlikely(symid < 0)
			goto err;
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_module_p((uint16_t)symid);

	case SYMBOL_TYPE_THIS:
		/* mov PREFIX, this */
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_this_p();

	case SYMBOL_TYPE_MYMOD:
		/* mov PREFIX, this_module */
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_this_module_p();

	case SYMBOL_TYPE_MYFUNC:
		if (current_basescope->bs_flags & CODE_FTHISCALL)
			break; /* The function has to be bound! */
		/* mov PREFIX, this_function */
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_this_function_p();

	case SYMBOL_TYPE_ARG:
		/* mov PREFIX, arg <imm8/16> */
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
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
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_global_p((uint16_t)symid);

	case SYMBOL_TYPE_LOCAL:
		/* mov PREFIX, local <imm8/16> */
		if (!(src_sym->s_flag & SYMBOL_FALLOC))
			break;
		symid = asm_lsymid_for_read(src_sym, dst_ast);
		if unlikely(symid < 0)
			goto err;
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_local_p((uint16_t)symid);

	case SYMBOL_TYPE_STATIC:
		/* mov PREFIX, static <imm8/16> */
		if (!(src_sym->s_flag & SYMBOL_FALLOC))
			break;
		symid = asm_ssymid_for_read(src_sym, dst_ast);
		if unlikely(symid < 0)
			goto err;
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_static_p((uint16_t)symid);

	case SYMBOL_TYPE_EXTERN:
		/* mov PREFIX, extern <imm8/16>:<imm8/16> */
		if (SYMBOL_EXTERN_SYMBOL(src_sym)->ss_flags & MODSYM_FPROPERTY)
			break; /* Cannot be used for external properties. */
		symid = asm_esymid(src_sym);
		if unlikely(symid < 0)
			goto err;
		DO(asm_putddi(dst_ast));
		DO(asm_gprefix_symbol(dst_sym, dst_ast));
		return asm_gpush_extern_p((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(src_sym)->ss_index);

	default: break;
	}
	DO(asm_putddi(dst_ast));
	DO(asm_gpush_symbol(src_sym, dst_ast));
	if (src_ast != dst_ast)
		DO(asm_putddi(src_ast));
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
			DO(asm_putddi(dst_ast));
			DO(asm_gprefix_symbol(dst_sym, dst_ast));
			return asm_gpush_none_p();
		} else if (constval == Dee_True) {
			/* mov PREFIX, true */
			DO(asm_putddi(dst_ast));
			DO(asm_gprefix_symbol(dst_sym, dst_ast));
			return asm_gpush_true_p();
		} else if (constval == Dee_False) {
			/* mov PREFIX, false */
			DO(asm_putddi(dst_ast));
			DO(asm_gprefix_symbol(dst_sym, dst_ast));
			return asm_gpush_false_p();
		} else if (asm_allowconst(constval)) {
			int32_t cid;
			/* mov PREFIX, const <imm8/16> */
			cid = asm_newconst(constval);
			if unlikely(cid < 0)
				goto err;
			DO(asm_putddi(dst_ast));
			DO(asm_gprefix_symbol(dst_sym, dst_ast));
			return asm_gpush_const_p((uint16_t)cid);
		}
	}	break;

	default:
		break;
	}
	DO(ast_genasm(src, ASM_G_FPUSHRES));
	DO(asm_putddi(dst_ast));
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
	DO(asm_putddi(src_ast));
	DO(asm_gpop_expr_enter(dst));
	DO(asm_gpush_symbol(src_sym, src_ast));
	DO(asm_gpop_expr_leave(dst, ASM_G_FNORMAL));
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
				DO(asm_putddi(ddi_ast));
				DO(asm_pstatic((uint16_t)sid));
				DO(asm_gcmpxch_ub_lock_p());
				Lalready_init = asm_newsym();
				if unlikely(!Lalready_init)
					goto err;
				DO(asm_gjmp(ASM_JF, Lalready_init));
				asm_decsp();

				/* Generate the initializer expression. */
				if (!src_contains_return && ast_is_nothrow(src, true)) {
					DO(ast_genasm_one(src, ASM_G_FPUSHRES));
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
						DO(asm_gjmp(ASM_JMP, Lafter_except));
					}
					asm_decsp();
					if (guard_finflags & ASM_FINFLAG_USED) {
						asm_incsp();
						asm_incsp();
					}
					asm_defsym(Linit_try_except);
					DO(asm_putddi(ddi_ast));
					DO(asm_gdel_static((uint16_t)sid));
					if (guard_finflags & ASM_FINFLAG_USED) {
						DO(asm_gendfinally());
						DO(asm_gjmp_pop_pop());
					} else {
						DO(asm_gendfinally_except());
					}

					/* Restore context for the non-except case. */
					current_assembler.a_curr = old_sect;
					asm_incsp();
					if (Lafter_except)
						asm_defsym(Lafter_except);
				}

				/* Store initializer result into the static variable. */
				DO(asm_putddi(ddi_ast));
				DO(asm_gpop_static((uint16_t)sid));

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
				DO(asm_putddi(dst));
				DO(asm_gpush_bnd_static((uint16_t)sid));
				DO(asm_gjmp(ASM_JT, Lalready_init));
				asm_decsp();
				DO(ast_genasm_one(src, ASM_G_FPUSHRES));
				DO(asm_putddi(ddi_ast));
				DO(asm_pstatic((uint16_t)sid));
				DO(asm_gcmpxch_ub_pop_p());
				DO(asm_gpop());
				asm_defsym(Lalready_init);
				goto push_static_as_result;
			}
			if (src->a_type == AST_CONSTEXPR) {
				/* Special case: can use "push cmpxch PREFIX, unbound, const <imm8/16>" */
				int32_t cid = asm_newconst(src->a_constexpr);
				if unlikely(cid < 0)
					goto err;
				DO(asm_putddi(ddi_ast));
				DO(asm_pstatic((uint16_t)sid));
				DO(asm_gcmpxch_ub_c_p((uint16_t)cid));
			} else {
				DO(ast_genasm_one(src, ASM_G_FPUSHRES));
				DO(asm_putddi(ddi_ast));
				DO(asm_pstatic((uint16_t)sid));
				DO(asm_gcmpxch_ub_pop_p());
			}
			DO(asm_gpop());
push_static_as_result:
			if (PUSH_RESULT) {
				DO(asm_putddi(dst));
				DO(asm_gpush_static((uint16_t)sid));
			}
			return 0;
		}	break;

		case SYMBOL_TYPE_GLOBAL:
		case SYMBOL_TYPE_EXTERN:
		case SYMBOL_TYPE_LOCAL:
		case SYMBOL_TYPE_STACK:
			if (!PUSH_RESULT && asm_can_prefix_symbol(dst_sym))
				return asm_gmov_sym_ast(dst_sym, src, dst);
			DO(ast_genasm(src, ASM_G_FPUSHRES));
			if (PUSH_RESULT) {
				DO(asm_putddi(ddi_ast));
				DO(asm_gdup());
			}
			DO(asm_putddi(dst));
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
					DO(asm_putddi(dst));
					DO(asm_gprefix_symbol_for_read(src->a_sym, src));
					return asm_gpop_global_p((uint16_t)symid);

				case SYMBOL_TYPE_LOCAL:
					/* mov local <imm8/16>, PREFIX */
					symid = asm_lsymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					DO(asm_putddi(dst));
					DO(asm_gprefix_symbol_for_read(src->a_sym, src));
					return asm_gpop_local_p((uint16_t)symid);

				case SYMBOL_TYPE_STATIC:
					/* mov static <imm8/16>, PREFIX */
					symid = asm_ssymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					DO(asm_putddi(dst));
					DO(asm_gprefix_symbol_for_read(src->a_sym, src));
					return asm_gpop_static_p((uint16_t)symid);

				case SYMBOL_TYPE_EXTERN:
					/* mov extern <imm8/16>, PREFIX */
					if (SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_flags &
					    (MODSYM_FREADONLY | MODSYM_FPROPERTY))
						break;
					symid = asm_esymid(dst_sym);
					if unlikely(symid < 0)
						goto err;
					DO(asm_putddi(dst));
					DO(asm_gprefix_symbol_for_read(src->a_sym, src));
					return asm_gpop_extern_p((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_index);

				case SYMBOL_TYPE_STACK:
					if (!(dst_sym->s_flag & SYMBOL_FALLOC))
						break;
					DO(asm_putddi(ddi_ast));
					DO(asm_gprefix_symbol_for_read(src->a_sym, src));
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
					DO(ast_genasm(dst->a_multiple.m_astv[i], ASM_G_FNORMAL));
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
				DO(asm_gstore(dst->a_multiple.m_astv[i],
				              src->a_multiple.m_astv[i],
				              ddi_ast, gflags));
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
	DO(asm_gpop_expr_enter(dst));
	DO(ast_genasm(src, ASM_G_FPUSHRES));
	DO(asm_gpop_expr_leave(dst, gflags));
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
		DO(asm_gpop_expr(astv[astc]));
	}
	if (!astc)
		goto done;
	i = 0;

	/* Find the first AST that does actually have side-effects. */
	while (i < astc && !ast_has_sideeffects(astv[i]))
		++i;
	for (j = i; j < astc; ++j) {
		DO(asm_glrot((uint16_t)(astc - j)));
		DO(asm_gpop_expr(astv[j]));
	}

	/* Handle leading asts without side-effects in reverse order. */
	while (i--) {
		DO(asm_gpop_expr(astv[i]));
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
		DO(asm_putddi(self));
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
				for (; i < self->a_multiple.m_astc - 1; ++i)
					DO(ast_genasm(self->a_multiple.m_astv[i], ASM_G_FNORMAL));
				ASM_POP_SCOPE(0, err);
			}
			ASSERT(i == self->a_multiple.m_astc - 1);
			return asm_gpop_expr(self->a_multiple.m_astv[i]);
		}
		DO(asm_putddi(self));
		DO(asm_gunpack((uint16_t)self->a_multiple.m_astc));
		DO(asm_gpop_expr_multiple(self->a_multiple.m_astc,
		                          self->a_multiple.m_astv));
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
						DO(asm_putddi(self));
						DO(asm_gsetattr_this_const((uint16_t)cid));
						goto done;
					}
				}
				DO(ast_genasm_one(base, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_gswap());
				DO(asm_gsetattr_const((uint16_t)cid));
				goto done;
			}
			DO(ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_glrot(3));
			DO(asm_gsetattr());
			goto done;
		}

		case OPERATOR_GETITEM: {
			struct ast *index = self->a_operator.o_op1;
			if unlikely(!index)
				break;
			DO(ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES));
			if (index->a_type == AST_CONSTEXPR) {
				int16_t int_index;
				/* Special optimizations for constant indices. */
				if (DeeInt_Check(index->a_constexpr) &&
				    DeeInt_TryAsInt16(index->a_constexpr, &int_index)) {
					DO(asm_putddi(self));
					DO(asm_gswap());
					DO(asm_gsetitem_index(int_index));
					goto done;
				}
				if (asm_allowconst(index->a_constexpr)) {
					int32_t cid = asm_newconst(index->a_constexpr);
					if unlikely(cid < 0)
						goto err;
					DO(asm_putddi(self));
					DO(asm_gswap());
					DO(asm_gsetitem_const((uint16_t)cid));
					goto done;
				}
			}
			DO(ast_genasm_one(index, ASM_G_FPUSHRES)); /* STACK: item, base, index */
			DO(asm_putddi(self));
			DO(asm_glrot(3)); /* STACK: base, index, item */
			DO(asm_gsetitem()); /* STACK: - */
			goto done;
		}

		case OPERATOR_GETRANGE: {
			struct ast *start, *end;
			int16_t int_index;
			if unlikely (!self->a_operator.o_op2)
				break;
			DO(ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES));
			start = self->a_operator.o_op1;
			end   = self->a_operator.o_op2;

			/* Special optimizations for certain ranges. */
			if (start->a_type == AST_CONSTEXPR) {
				DeeObject *begin_index = start->a_constexpr;
				if (DeeNone_Check(begin_index)) {
					/* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
					if (end->a_type == AST_CONSTEXPR &&
					    DeeInt_Check(end->a_constexpr) &&
					    DeeInt_TryAsInt16(end->a_constexpr, &int_index)) {
						/* `setrange pop, none, $<Simm16>, pop' */
						DO(asm_putddi(self));
						DO(asm_gswap()); /* STACK: base, item */
						DO(asm_gsetrange_ni(int_index));
						goto done;
					}

					/* `setrange pop, none, pop, pop' */
					DO(ast_genasm_one(end, ASM_G_FPUSHRES)); /* STACK: item, base, end */
					DO(asm_putddi(self));
					DO(asm_glrot(3));       /* STACK: base, end, item */
					DO(asm_gsetrange_np()); /* STACK: - */
					goto done;
				}
				if (DeeInt_Check(begin_index) &&
				    DeeInt_TryAsInt16(begin_index, &int_index)) {
					if (end->a_type == AST_CONSTEXPR) {
						int16_t int_index2;
						DeeObject *end_index = end->a_constexpr;
						if (DeeNone_Check(end_index)) {
							/* `setrange pop, $<Simm16>, none, pop' */
							DO(asm_putddi(self));
							DO(asm_gswap());                 /* STACK: base, item */
							DO(asm_gsetrange_in(int_index)); /* STACK: - */
							goto done;
						}
						if (DeeInt_Check(end_index) &&
						    DeeInt_TryAsInt16(end_index, &int_index2)) {
							/* `setrange pop, $<Simm16>, $<Simm16>, pop' */
							DO(asm_putddi(self));
							DO(asm_gswap());                                      /* STACK: base, item */
							DO(asm_gsetrange_ii(int_index, (int16_t)int_index2)); /* STACK: - */
							goto done;
						}
					}
					DO(ast_genasm_one(end, ASM_G_FPUSHRES)); /* STACK: item, base, end */
					DO(asm_putddi(self));
					DO(asm_glrot(3));                /* STACK: base, end, item */
					DO(asm_gsetrange_ip(int_index)); /* STACK: - */
					goto done;
				}
			} else if (end->a_type == AST_CONSTEXPR) {
				/* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
				DeeObject *end_index = end->a_constexpr;
				if (DeeNone_Check(end_index)) {
					/* `setrange pop, pop, none, pop' */
					DO(ast_genasm_one(start, ASM_G_FPUSHRES)); /* STACK: item, base, begin */
					DO(asm_putddi(self));
					DO(asm_glrot(3));       /* STACK: base, begin, item */
					DO(asm_gsetrange_pn()); /* STACK: - */
					goto done;
				}
				if (DeeInt_Check(end_index) &&
				    DeeInt_TryAsInt16(end_index, &int_index)) {
					/* `setrange pop, pop, $<Simm16>, pop' */
					DO(ast_genasm_one(start, ASM_G_FPUSHRES)); /* STACK: item, base, begin */
					DO(asm_putddi(self));
					DO(asm_glrot(3));                /* STACK: base, begin, item */
					DO(asm_gsetrange_pi(int_index)); /* STACK: - */
					goto done;
				}
			}
			DO(ast_genasm_one(start, ASM_G_FPUSHRES));
			DO(ast_genasm_one(end, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			/* STACK: item, base, begin, end */
			DO(asm_glrot(4));    /* STACK: base, begin, end, item */
			DO(asm_gsetrange()); /* STACK: - */
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
		DO(asm_gpop());
		break;

	default:
default_case:
		/* Emit a warning about an r-value store. */
		DO(WARNAST(self, W_ASM_STORE_TO_RVALUE));

		/* Fallback: Generate the ast and store it directly. */
		DO(ast_genasm_one(self, ASM_G_FPUSHRES));
		DO(asm_putddi(self));
		DO(asm_gswap());
		DO(asm_gassign());
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
