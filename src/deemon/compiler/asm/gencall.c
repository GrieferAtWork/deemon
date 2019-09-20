/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_GENCALL_C
#define GUARD_DEEMON_COMPILER_ASM_GENCALL_C 1
#define _KOS_SOURCE 1

#include <deemon/HashSet.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/traits.h>
#include <deemon/dict.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include "../../runtime/builtin.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
push_tuple_items(DeeObject *__restrict self,
                 struct ast *__restrict ddi_ast) {
	size_t i, size;
	size = DeeTuple_SIZE(self);
	if (size) {
		if (asm_putddi(ddi_ast))
			goto err;
		for (i = 0; i < size; ++i) {
			if (asm_gpush_constexpr(DeeTuple_GET(self, i)))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

INTDEF struct module_symbol *DCALL
get_module_symbol(DeeModuleObject *__restrict module,
                  DeeStringObject *__restrict name);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL asm_check_thiscall(struct symbol *__restrict sym,
                                    struct ast *__restrict warn_ast);


PRIVATE WUNUSED DREF DeeCodeObject *DCALL
ast_assemble_function_refargs(struct ast *__restrict function_ast,
                              uint16_t *__restrict prefc,
                              struct asm_symbol_ref **__restrict prefv,
                              uint16_t *__restrict pargc,
                              /*out:inherit*/ struct symbol ***__restrict pargv) {
	DREF DeeCodeObject *result;
	DeeScopeObject *prev_scope;
	ASSERT(function_ast->a_type == AST_FUNCTION);
	/* This is where it gets interesting, because this will
	 * create a temporary sub-assembler, allowing for recursion! */
	ASSERT_OBJECT_TYPE((DeeObject *)current_scope, &DeeScope_Type);
	ASSERT_OBJECT_TYPE((DeeObject *)current_basescope, &DeeBaseScope_Type);
	ASSERT_OBJECT_TYPE((DeeObject *)function_ast->a_function.f_scope, &DeeBaseScope_Type);
	ASSERT(function_ast->a_function.f_scope->bs_root == current_rootscope);
	ASSERT(current_basescope == current_scope->s_base);
	prev_scope = current_scope;
	/* Temporarily override the active scope + base-scope. */
	current_scope     = (DREF DeeScopeObject *)function_ast->a_function.f_scope;
	current_basescope = function_ast->a_function.f_scope;

	/* HINT: `code_compile_argrefs' will safe and restore our own assembler context. */
	result = code_compile_argrefs(function_ast->a_function.f_code,
	                              /* Don't propagate `ASM_FBIGCODE' */
	                              (current_assembler.a_flag & ~(ASM_FBIGCODE)) |
	                              (DeeCompiler_Current->cp_options ? (DeeCompiler_Current->cp_options->co_assembler & ASM_FBIGCODE) : 0)
#if 1 /* Further reduce the amount of needed references by passing this flag!  \
       * TODO: This flag should not be effective for recursive sub-functions,  \
       *       but rather only for symbols where references would have to pass \
       *       through us. - I.e. a sub-function should not be hindered from   \
       *       references a symbol that wouldn't even need to pass through our \
       *       scope (because it would be declared by one of our children) */
	                              | ASM_FREDUCEREFS
#endif
	                              ,
	                              prefc, prefv, pargc, pargv);
	/* Now that the code has been generated, it's time to
	 * register it as a constant variable of our own code. */
	current_basescope = prev_scope->s_base;
	current_scope     = prev_scope;
	if unlikely(!result)
		goto err;
	/* Save the restore-code-object for the event of the linker having to be reset. */
	function_ast->a_function.f_scope->bs_restore = result;
	return result;
err:
	return NULL;
}

PRIVATE int DCALL
asm_gcall_func(struct ast *__restrict func,
               struct ast *__restrict args,
               struct ast *__restrict ddi_ast,
               unsigned int gflags) {
	/* If possible, references should be passed by-argument, rather than by-reference!
	 * That way, we can possible prevent having to re-create new function objects
	 * whenever the function is invoked at runtime.
	 * The major culprit here are generator expression, which are practically
	 * forced to use referenced variables:
	 * >> local count = 42;
	 * >> local items = [(for (local x: [:count]) (x * 2) / 3)...];
	 * Compiled as:
	 * >>     mov   local @count, $42
	 * >>     push  local @count
	 * >>     push  function const @.Lgen0, #1  // <--- Unnecessary temporary created here!
	 * >>     call  top, #0
	 * >>     cast  top, list
	 * >>     pop   local @items
	 * >> .const .Lgen0 = code {
	 * >>     push  ref @count
	 * >>     push  range default, pop
	 * >>     iterself top
	 * >> 1:  foreach top, 2f
	 * >>     mul   top, $2
	 * >>     div   top, $3
	 * >>     yield pop
	 * >>     jmp   1b
	 * >> 2:  ret
	 * >> }
	 * Can be compiled as as:
	 * >>     mov   local @count, $42
	 * >>     push  local @count
	 * >>     call  top, #1
	 * >>     cast  top, list
	 * >>     pop   local @items
	 * >> .const .Lgen0 = function {
	 * >>     push  arg @count
	 * >>     push  range default, pop
	 * >>     iterself top
	 * >> 1:  foreach top, 2f
	 * >>     mul   top, $2
	 * >>     div   top, $3
	 * >>     yield pop
	 * >>     jmp   1b
	 * >> 2:  ret
	 * >> } */
	uint16_t refc, i;
	uint16_t refargc;
	struct asm_symbol_ref *refv;
	struct symbol **refargv;
	DREF DeeCodeObject *code;
	int32_t cid;
	code = ast_assemble_function_refargs(func, &refc, &refv, &refargc, &refargv);
	if unlikely(!code)
		goto err;
	if (asm_putddi(func))
		goto err_refv;
	if (!refc) {
		/* Special case: The function doesn't reference any code.
		 * -> In this case, we can construct the function object
		 *    itself as a constant. */
		DREF DeeFunctionObject *function;
		Dee_Free(refv);
		function = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)code);
		Dee_Decref(code);
		if unlikely(!function)
			goto err_refargv;
		cid = asm_newconst((DeeObject *)function);
		Dee_Decref(function);
		if unlikely(cid < 0)
			goto err_refargv;
		if (asm_gpush_const((uint16_t)cid))
			goto err_refargv;
	} else {
		cid = asm_newconst((DeeObject *)code);
		Dee_Decref(code);
		if unlikely(cid < 0)
			goto err_refv;
		/* Push referenced symbols. */
		for (i = 0; i < refc; ++i) {
			refv[i].sr_sym->s_flag &= ~(SYMBOL_FALLOC | SYMBOL_FALLOCREF);
			refv[i].sr_sym->s_flag |= refv[i].sr_orig_flag & (SYMBOL_FALLOC | SYMBOL_FALLOCREF);
			refv[i].sr_sym->s_refid = refv[i].sr_orig_refid;
			if (asm_gpush_symbol(refv[i].sr_sym, func))
				goto err_refv;
		}
		Dee_Free(refv);
		/* Generate assembly to create (and push) the new function object. */
		ASSERT(refc != 0);
		if (asm_gfunction_ii((uint16_t)cid, (uint16_t)refc))
			goto err_refargv;
	}
	/* At this point, the function that is being called is located ontop of the stack
	 * Now it's just a matter of figuring out what exactly it is that we're dealing
	 * with here, when it comes to arguments. */
	if (args->a_type == AST_MULTIPLE &&
	    AST_FMULTIPLE_ISSEQUENCE(args->a_flag)) {
		ASSERTF(!ast_chk_multiple_hasexpand(args), "The caller should have asserted this!");
		if (args->a_multiple.m_astc + refargc > UINT8_MAX)
			goto generic_call;
		for (i = 0; i < (uint8_t)args->a_multiple.m_astc; ++i) {
			if (ast_genasm_one(args->a_multiple.m_astv[i], ASM_G_FPUSHRES))
				goto err_refargv;
		}
		if (refargc) {
			/* Push all of the reference argument symbols. */
			if (asm_putddi(func))
				goto err_refargv;
			for (i = 0; i < refargc; ++i) {
				if (asm_gpush_symbol(refargv[i], func))
					goto err_refargv;
			}
		}
		Dee_Free(refargv);
		/* Do the call. */
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gcall((uint8_t)(args->a_multiple.m_astc + refargc)))
			goto err;
		goto pop_unused;
	}
	if (args->a_type == AST_CONSTEXPR &&
	    DeeTuple_Check(args->a_constexpr)) {
		if (DeeTuple_SIZE(args->a_constexpr) + refargc > UINT8_MAX)
			goto generic_call;
		if (push_tuple_items(args->a_constexpr, args))
			goto err_refargv;
		if (refargc) {
			/* Push all of the reference argument symbols. */
			if (asm_putddi(func))
				goto err_refargv;
			for (i = 0; i < refargc; ++i) {
				if (asm_gpush_symbol(refargv[i], func))
					goto err_refargv;
			}
		}
		Dee_Free(refargv);
		/* Do the call. */
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gcall((uint8_t)(DeeTuple_SIZE(args->a_constexpr) + refargc)))
			goto err;
		goto pop_unused;
	}

	/* Fallback: Push the arguments as a tuple, then concat that with refargs. */
generic_call:
	if (ast_genasm_one(args, ASM_G_FPUSHRES))
		goto err_refargv;
	if (ast_predict_type(args) != &DeeTuple_Type &&
	    (asm_putddi(args) || asm_gcast_tuple()))
		goto err_refargv;
	if (refargv) {
		if (refargc) {
			/* Push all of the reference argument symbols. */
			if (asm_putddi(func))
				goto err_refv;
			for (i = 0; i < refargc; ++i) {
				if (asm_gpush_symbol(refargv[i], func))
					goto err_refv;
			}
			if (asm_gextend(refargc))
				goto err_refv;
		}
		Dee_Free(refargv);
	}
	if (asm_putddi(ddi_ast))
		goto err;
	if (asm_gcall_tuple())
		goto err;
pop_unused:
	if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
		goto err;
	return 0;
err_refv:
	Dee_Free(refv);
err_refargv:
	Dee_Free(refargv);
err:
	return -1;
}

INTERN int DCALL
asm_gcall_expr(struct ast *__restrict func,
               struct ast *__restrict args,
               struct ast *__restrict ddi_ast,
               unsigned int gflags) {
	/* Special instruction encoding for call operations. */
	struct symbol *funsym;
	int32_t symid;
	uint8_t i, argc;
	struct ast **argv;
	ASSERT_AST(func);
	ASSERT_AST(args);
	if (func->a_type == AST_FUNCTION &&
	    /* NOTE: Don't perform this optimization when the argument list is unpredictable. */
	    (args->a_type != AST_MULTIPLE ||
	     (AST_FMULTIPLE_ISSEQUENCE(args->a_flag) && !ast_chk_multiple_hasexpand(args))))
		return asm_gcall_func(func, args, ddi_ast, gflags);

	/* Optimized call for stack-packed argument list within an 8-bit range. */
	if ((args->a_type != AST_MULTIPLE ||
	     !AST_FMULTIPLE_ISSEQUENCE(args->a_flag) ||
	     args->a_multiple.m_astc > UINT8_MAX ||
	     ast_chk_multiple_hasexpand(args)) &&
	    (args->a_type != AST_CONSTEXPR ||
	     args->a_constexpr != Dee_EmptyTuple)) {
		if (args->a_type == AST_CONSTEXPR &&
		    DeeTuple_Check(args->a_constexpr) &&
		    DeeTuple_SIZE(args->a_constexpr) <= 2) {
			argc = (uint8_t)DeeTuple_SIZE(args->a_constexpr);
			/* Special handling for small argument tuples to prevent this:
			 * >> push global @foo
			 * >> pack Tuple, #0
			 * >> call top, pop
			 * That assembly is kind-of less efficient than this:
			 * >> call global @foo, #0
			 * So that's why we can't just blindly go ahead and
			 * always make use of constant expression tuples... */
			if (func->a_type == AST_SYM) {
				funsym = func->a_sym;
check_small_constargs_symbol:
				switch (funsym->s_type) {
				case SYMBOL_TYPE_ALIAS:
					funsym = SYMBOL_ALIAS(funsym);
					goto check_small_constargs_symbol;

				case SYMBOL_TYPE_EXTERN: {
					struct module_symbol *modsym;
					modsym = funsym->s_extern.e_symbol;
					if (modsym->ss_flags & MODSYM_FPROPERTY)
						break;
					/* Direct call to symbol. */
					if (modsym->ss_flags & MODSYM_FEXTERN) {
						ASSERT(modsym->ss_extern.ss_impid < funsym->s_extern.e_module->mo_importc);
						symid = asm_newmodule(funsym->s_extern.e_module->mo_importv[modsym->ss_extern.ss_impid]);
					} else {
						symid = asm_esymid(funsym);
					}
					if unlikely(symid < 0)
						goto err;
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall_extern((uint16_t)symid, modsym->ss_index, argc))
						goto err;
					goto pop_unused;
				}	break;

				case SYMBOL_TYPE_GLOBAL:
					/* Direct call to symbol. */
					if unlikely((symid = asm_gsymid_for_read(funsym, func)) < 0)
						goto err;
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall_global((uint16_t)symid, argc))
						goto err;
					goto pop_unused;

				case SYMBOL_TYPE_LOCAL:
					if (SYMBOL_MUST_REFERENCE_TYPEMAY(funsym))
						break;
					if unlikely((symid = asm_lsymid_for_read(funsym, func)) < 0)
						goto err;
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall_local((uint16_t)symid, argc))
						goto err;
					goto pop_unused;

				case SYMBOL_TYPE_CATTR: {
					struct symbol *class_sym, *this_sym;
					struct class_attribute *attr;
invoke_cattr_funsym_small:
					class_sym = funsym->s_attr.a_class;
					this_sym  = funsym->s_attr.a_this;
					attr      = funsym->s_attr.a_attr;
					SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
					if (!this_sym) {
						if (asm_putddi(func))
							goto err;
						if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
							symid = asm_rsymid(class_sym);
							if (asm_ggetcmember_r(symid, attr->ca_addr))
								goto err;
						} else {
							if (asm_gpush_symbol(class_sym, func))
								goto err;
							if (asm_ggetcmember(attr->ca_addr))
								goto err;
						}
						if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
							/* Must invoke the getter callback. */
							if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
								if (asm_gpush_symbol(class_sym, func))
									goto err; /* getter, class */
								if (asm_gcall(1))
									goto err; /* func */
							} else {
								if (asm_gcall(0))
									goto err;
							}
						} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
							ASSERT(argc != (uint8_t)-1);
							if (asm_gpush_symbol(class_sym, func))
								goto err; /* func, class_sym */
							if unlikely(push_tuple_items(args->a_constexpr, args))
								goto err;
							if (asm_putddi(ddi_ast))
								goto err; /* func, class_sym, args... */
							if (asm_gcall(argc + 1))
								goto err; /* result */
							goto pop_unused;
						}
						if unlikely(push_tuple_items(args->a_constexpr, args))
							goto err;
						if (asm_putddi(ddi_ast))
							goto err; /* func, args... */
						if (asm_gcall(argc))
							goto err; /* result */
						goto pop_unused;
					}
					/* The attribute must be accessed as virtual. */
					if unlikely(asm_check_thiscall(funsym, func))
						goto err;
					SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
					if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
						symid = asm_newconst((DeeObject *)attr->ca_name);
						if unlikely(symid < 0)
							goto err;
						if (this_sym->s_type == SYMBOL_TYPE_THIS &&
						    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
							if unlikely(push_tuple_items(args->a_constexpr, args))
								goto err;
							if (asm_putddi(ddi_ast))
								goto err; /* args... */
							if (asm_gcallattr_this_const((uint16_t)symid, argc))
								goto err;
							goto pop_unused;
						}
						if (asm_putddi(func))
							goto err;
						if (asm_gpush_symbol(this_sym, func))
							goto err; /* this */
						if unlikely(push_tuple_items(args->a_constexpr, args))
							goto err;
						if (asm_putddi(ddi_ast))
							goto err; /* this, args... */
						if (asm_gcallattr_const((uint16_t)symid, argc))
							goto err; /* result */
						goto pop_unused;
					}
					/* Regular, old member variable. */
					if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
						if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
							symid = asm_rsymid(class_sym);
							if unlikely(symid < 0)
								goto err;
							if ((attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) &&
							    this_sym->s_type == SYMBOL_TYPE_THIS &&
							    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
								if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
									/* Invoke the getter callback. */
									if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET, 0))
										goto err;
									goto got_small_method;
								}
								if unlikely(push_tuple_items(args->a_constexpr, args))
									goto err;
								if (asm_putddi(ddi_ast))
									goto err;
								if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr, argc))
									goto err;
								goto pop_unused;
							}
							if (asm_putddi(func))
								goto err;
							if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr))
								goto err;
						} else {
							if (asm_gpush_symbol(class_sym, func))
								goto err;
							if (asm_putddi(func))
								goto err;
							if (asm_ggetcmember(attr->ca_addr))
								goto err;
						}
					} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
					           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
						if (asm_putddi(func))
							goto err;
						if (asm_gpush_symbol(this_sym, func))
							goto err;
						if (asm_gpush_symbol(class_sym, func))
							goto err;
						if (asm_ggetmember(attr->ca_addr))
							goto err;
					} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if unlikely(symid < 0)
							goto err;
						if (asm_putddi(func))
							goto err;
						if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr))
							goto err;
					} else {
						if (asm_putddi(func))
							goto err;
						if (asm_gpush_symbol(class_sym, func))
							goto err;
						if (asm_ggetmember_this(attr->ca_addr))
							goto err;
					}
					if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
						/* Call the getter of the attribute. */
						if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
							/* Invoke as a this-call. */
							if (asm_gpush_symbol(this_sym, func))
								goto err;
							if (asm_gcall(1))
								goto err;
						} else {
							if (asm_gcall(0))
								goto err; /* Directly invoke. */
						}
					} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						/* Access to an instance member function (must produce a bound method). */
						if (asm_gpush_symbol(this_sym, func))
							goto err; /* func, this */
						ASSERT(argc != (uint8_t)-1);
						if unlikely(push_tuple_items(args->a_constexpr, args))
							goto err;
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gcall(argc + 1))
							goto err;
						goto pop_unused;
					}
got_small_method:
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err; /* func, args... */
					if (asm_gcall(argc))
						goto err; /* result */
					goto pop_unused;
				}	break;

				case SYMBOL_TYPE_MYFUNC:
					if (funsym->s_scope->s_base != current_basescope)
						break;
					if (!(current_basescope->bs_flags & CODE_FTHISCALL))
						break;
					ASSERT(!SYMBOL_MUST_REFERENCE_NOTTHIS(funsym));
					/* Call to the current or surrounding function, when
					 * that function is defined to be a this-call function.
					 * In this case, pushing the raw function would result
					 * in an InstanceMethod object having to be created at
					 * runtime, which is something that we can prevent by
					 * referencing the associated this-argument and simply
					 * calling the function directly. */
					if (asm_putddi(func))
						goto err;
					if (asm_gpush_this_function())
						goto err;
					if (asm_gpush_this())
						goto err;
					ASSERT(argc != (uint8_t)-1);
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall(argc + 1))
						goto err;
					goto pop_unused;

				default: break;
				}
			}
			if (func->a_type == AST_CONSTEXPR && argc <= 1 &&
			    (DeeObjMethod_Check(func->a_constexpr) ||
			     DeeKwObjMethod_Check(func->a_constexpr))) {
				char const *name = DeeObjMethod_GetName(func->a_constexpr);
				if (name) {
					/* call to some other object. */
					int32_t attrid;
					DREF DeeObject *name_ob;
					name_ob = DeeString_New(name);
					if unlikely(!name_ob)
						goto err;
					attrid = asm_newconst(name_ob);
					Dee_Decref(name_ob);
					if unlikely(attrid < 0)
						goto err;
					if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
						goto err;
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_const((uint16_t)attrid, argc))
						goto err;
					goto pop_unused;
				}
			}
			if (func->a_type == AST_OPERATOR &&
			    func->a_flag == OPERATOR_GETATTR &&
			    !(func->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS))) {
				struct ast *function_self = func->a_operator.o_op0;
				struct ast *function_attr = func->a_operator.o_op1;
				if unlikely(!function_attr)
					goto generic_call;
				/* Call to an attribute with stack-based argument list. */
				if (function_attr->a_type == AST_CONSTEXPR &&
				    DeeString_Check(function_attr->a_constexpr)) {
					int32_t attrid;
					/* (very) likely case: call to an attribute with a constant name. */
					if (function_self->a_type == AST_SYM) {
						DeeClassScopeObject *class_scope;
						struct symbol *sym = function_self->a_sym;
check_getattr_base_symbol_class_small:
						for (class_scope = function_self->a_scope->s_class; class_scope;
						     class_scope = DeeClassScope_Prev(class_scope)) {
							/* Try to statically access known class members! */
							if (sym == class_scope->cs_class ||
							    sym == class_scope->cs_this) {
								funsym = scope_lookup_str(&class_scope->cs_scope,
								                          DeeString_STR(function_attr->a_constexpr),
								                          DeeString_SIZE(function_attr->a_constexpr));
								/* Generate a regular attribute access. */
								if (funsym &&
								    funsym->s_type == SYMBOL_TYPE_CATTR &&
								    funsym->s_attr.a_class == class_scope->cs_class) {
									if (sym == funsym->s_attr.a_this)
										goto invoke_cattr_funsym_small; /* Regular access to a dynamic attribute. */
									if (sym == funsym->s_attr.a_class && !funsym->s_attr.a_this)
										goto invoke_cattr_funsym_small; /* Regular access to a static-attribute. */
								}
								break;
							}
						}
						switch (SYMBOL_TYPE(sym)) {
						case SYMBOL_TYPE_ALIAS:
							sym = SYMBOL_ALIAS(sym);
							goto check_getattr_base_symbol_class_small;

						case SYMBOL_TYPE_THIS:
							if (SYMBOL_MUST_REFERENCE_THIS(sym))
								break;
							/* call to the `this' argument. (aka. in-class member call) */
							if unlikely(push_tuple_items(args->a_constexpr, args))
								goto err;
							attrid = asm_newconst(function_attr->a_constexpr);
							if unlikely(attrid < 0)
								goto err;
							if (asm_putddi(ddi_ast))
								goto err;
							if (asm_gcallattr_this_const((uint16_t)attrid, argc))
								goto err;
							goto pop_unused;

						case SYMBOL_TYPE_MODULE: {
							struct module_symbol *modsym;
							int32_t module_id;
							/* module.attr() --> call extern ... */
							modsym = get_module_symbol(sym->s_module,
							                           (DeeStringObject *)function_attr->a_constexpr);
							if (!modsym)
								break;
							if (modsym->ss_flags & MODSYM_FPROPERTY)
								break;
							if (modsym->ss_flags & MODSYM_FEXTERN) {
								ASSERT(modsym->ss_extern.ss_impid < sym->s_module->mo_importc);
								module_id = asm_newmodule(sym->s_module->mo_importv[modsym->ss_extern.ss_impid]);
							} else {
								module_id = asm_msymid(sym);
							}
							if unlikely(module_id < 0)
								goto err;
							/* Do a call to an external symbol. `ASM_CALL_EXTERN' */
							if unlikely(push_tuple_items(args->a_constexpr, args))
								goto err;
							if (asm_putddi(ddi_ast))
								goto err;
							if (asm_gcall_extern((uint16_t)module_id, modsym->ss_index, argc))
								goto err;
							goto pop_unused;
						}	break;

						default: break;
						}
					}
					if (function_self->a_type == AST_ACTION &&
					    function_self->a_flag == AST_FACTION_AS &&
					    function_self->a_action.a_act0->a_type == AST_SYM &&
					    function_self->a_action.a_act0->a_sym->s_type == SYMBOL_TYPE_THIS &&
					    !SYMBOL_MUST_REFERENCE_THIS(function_self->a_action.a_act0->a_sym) &&
					    /* Because we can't inline the argument tuple, only do this variant
					     * if we either don't have to allocate too many additional stack-slots,
					     * of when doing so wouldn't increase the size of the resulting text. */
					    /* When optimizing for size, only use this variant for up to 2 arguments:
					     * >> push     callattr this, ref <imm8/16>, @"foo", #... // 5 bytes
					     * VS:
					     * >> push     super this, ref <imm8/16>                  // 4 bytes
					     * >> push     const @args                                // 2 bytes
					     * >> callattr top, @"foo", pop...                        // 3 bytes
					     * Because of this, the 0-arguments case is 5 bytes vs 9 bytes
					     * With that in mind, we start out by saving 4 bytes.
					     *   4 bytes / sizeof(push const <imm8>) --> 2
					     * As you can see, so long as there aren't more than 2 arguments, we
					     * save text space by encoding a supercall. (with no loss or gain at
					     * exactly 2 arguments) */
					    argc <= ((current_assembler.a_stackmax - current_assembler.a_stackcur) +
					             ((current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) ? 2 : 16))) {
						/* `(this as ...).foobar(10,20,30)'
						 * -> Check if we can make use of `ASM_SUPERGETATTR_THIS_RC' instructions. */
						struct ast *type_expr = function_self->a_action.a_act1;
						int32_t type_rid;
						if (type_expr->a_type == AST_SYM &&
						    ASM_SYMBOL_MAY_REFERENCE(type_expr->a_sym)) {
							/* We are allowed to reference the base-symbol! */
							type_rid = asm_rsymid(type_expr->a_sym);
do_perform_supercallattr_small:
							if unlikely(type_rid < 0)
								goto err;
							attrid = asm_newconst((DeeObject *)function_attr->a_constexpr);
							if unlikely(attrid < 0)
								goto err;
							if unlikely(push_tuple_items(args->a_constexpr, args))
								goto err;
							if (asm_putddi(ddi_ast))
								goto err;
							if (asm_gsupercallattr_this_rc((uint16_t)type_rid, (uint16_t)attrid, argc))
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
								goto do_perform_supercallattr_small;
							}
						}
					}
					/* Only do this for 0/1 arguments:
					 * 2 arguments would already need 4 instructions:
					 * >> push     <this>
					 * >> push     $10
					 * >> push     $20
					 * >> callattr top, @"foo", #2
					 * But if we encode this using a constant tuple, it only needs 3:
					 * >> push     <this>
					 * >> push     @(10, 20)
					 * >> callattr top, @"foo", pop
					 * Ergo: Only encode individual arguments for less than 2 args.
					 */
					if (argc <= 1) {
						/* call to some other object. */
						attrid = asm_newconst(function_attr->a_constexpr);
						if unlikely(attrid < 0)
							goto err;
						if (ast_genasm(function_self, ASM_G_FPUSHRES))
							goto err;
						if unlikely(push_tuple_items(args->a_constexpr, args))
							goto err;
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gcallattr_const((uint16_t)attrid, argc))
							goto err;
						goto pop_unused;
					}
				} else if (argc <= 1) {
					/* Pretty unlikely: The attribute name is not known.
					 * Due to the runtime optimization impact that optimizing this still has,
					 * there is also an opcode for this (callattr() is much faster than getattr+call). */
					if (ast_genasm(function_self, ASM_G_FPUSHRES))
						goto err;
					if (ast_genasm_one(function_attr, ASM_G_FPUSHRES))
						goto err;
					if unlikely(push_tuple_items(args->a_constexpr, args))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr(argc))
						goto err;
					goto pop_unused;
				}
			}
		}
		if (func->a_type == AST_SYM) {
			funsym = SYMBOL_UNWIND_ALIAS(func->a_sym);
			if (funsym->s_type == SYMBOL_TYPE_CATTR) {
				struct symbol *class_sym, *this_sym;
				struct class_attribute *attr;
invoke_cattr_funsym_tuple:
				class_sym = funsym->s_attr.a_class;
				this_sym  = funsym->s_attr.a_this;
				attr      = funsym->s_attr.a_attr;
				SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
				if (!this_sym) {
					if (asm_putddi(func))
						goto err;
					if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if (asm_ggetcmember_r(symid, attr->ca_addr))
							goto err;
					} else {
						if (asm_gpush_symbol(class_sym, func))
							goto err;
						if (asm_ggetcmember(attr->ca_addr))
							goto err;
					}
					if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
						/* Must invoke the getter callback. */
						if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
							if (asm_gpush_symbol(class_sym, func))
								goto err; /* getter, class */
							if (asm_gcall(1))
								goto err; /* func */
						} else {
							if (asm_gcall(0))
								goto err;
						}
					} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						if (asm_gpush_symbol(class_sym, func))
							goto err; /* func, class_sym */
						if (ast_genasm_one(args, ASM_G_FPUSHRES))
							goto err; /* func, class_sym, args */
						if (ast_predict_type(args) != &DeeTuple_Type &&
						    (asm_putddi(args) || asm_gcast_tuple()))
							goto err;
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gthiscall_tuple())
							goto err; /* result */
						goto pop_unused;
					}
					if (ast_genasm_one(args, ASM_G_FPUSHRES))
						goto err; /* func, args */
					if (ast_predict_type(args) != &DeeTuple_Type &&
					    (asm_putddi(args) || asm_gcast_tuple()))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall_tuple())
						goto err; /* result */
					goto pop_unused;
				}
				/* The attribute must be accessed as virtual. */
				if unlikely(asm_check_thiscall(funsym, func))
					goto err;
				SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
				if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
					symid = asm_newconst((DeeObject *)attr->ca_name);
					if unlikely(symid < 0)
						goto err;
					if (this_sym->s_type == SYMBOL_TYPE_THIS &&
					    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
						if (ast_genasm(args, ASM_G_FPUSHRES))
							goto err; /* args */
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gcallattr_this_const_tuple((uint16_t)symid))
							goto err;
						goto pop_unused;
					}
					if (asm_putddi(func))
						goto err;
					if (asm_gpush_symbol(this_sym, func))
						goto err; /* this */
					if (ast_genasm_one(args, ASM_G_FPUSHRES))
						goto err; /* this, args */
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_const_tuple((uint16_t)symid))
						goto err; /* result */
					goto pop_unused;
				}
				/* Regular, old member variable. */
				if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
					if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if unlikely(symid < 0)
							goto err;
						if (asm_putddi(func))
							goto err;
						if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr))
							goto err;
					} else {
						if (asm_gpush_symbol(class_sym, func))
							goto err;
						if (asm_putddi(func))
							goto err;
						if (asm_ggetcmember(attr->ca_addr))
							goto err;
					}
				} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
				           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
					if (asm_putddi(func))
						goto err;
					if (asm_gpush_symbol(this_sym, func))
						goto err;
					if (asm_gpush_symbol(class_sym, func))
						goto err;
					if (asm_ggetmember(attr->ca_addr))
						goto err;
				} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_putddi(func))
						goto err;
					if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr))
						goto err;
				} else {
					if (asm_putddi(func))
						goto err;
					if (asm_gpush_symbol(class_sym, func))
						goto err;
					if (asm_ggetmember_this(attr->ca_addr))
						goto err;
				}
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					/* Call the getter of the attribute. */
					if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						/* Invoke as a this-call. */
						if (asm_gpush_symbol(this_sym, func))
							goto err;
						if (asm_gcall(1))
							goto err;
					} else {
						if (asm_gcall(0))
							goto err; /* Directly invoke. */
					}
				} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					/* Access to an instance member function (must produce a bound method). */
					if (asm_gpush_symbol(this_sym, func))
						goto err; /* func, this */
					if (ast_genasm_one(args, ASM_G_FPUSHRES))
						goto err; /* func, this, args */
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gthiscall_tuple())
						goto err; /* result */
					goto pop_unused;
				}
				if (ast_genasm_one(args, ASM_G_FPUSHRES))
					goto err; /* func, args */
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcall_tuple())
					goto err; /* result */
				goto pop_unused;
			}
		}
		if (func->a_type == AST_CONSTEXPR &&
		    (DeeObjMethod_Check(func->a_constexpr) ||
		     DeeKwObjMethod_Check(func->a_constexpr))) {
			char const *name = DeeObjMethod_GetName(func->a_constexpr);
			if (name) {
				int32_t attrid;
				DREF DeeObject *name_ob;
				name_ob = DeeString_New(name);
				if unlikely(!name_ob)
					goto err;
				attrid = asm_newconst(name_ob);
				Dee_Decref(name_ob);
				if unlikely(attrid < 0)
					goto err;
				if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
					goto err;
				if (ast_genasm_one(args, ASM_G_FPUSHRES))
					goto err;
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_const_tuple((uint16_t)attrid))
					goto err;
				goto pop_unused;
			}
		}
		if (func->a_type == AST_OPERATOR &&
		    func->a_flag == OPERATOR_GETATTR &&
		    !(func->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS))) {
			struct ast *function_self = func->a_operator.o_op0;
			struct ast *function_attr = func->a_operator.o_op1;
			if unlikely(!function_attr)
				goto generic_call;
			/* callattr() with an argument list only known at runtime.
			 * This can actually happen _very_ easily when expand expressions are being used. */
			if (function_attr->a_type == AST_CONSTEXPR &&
			    DeeString_Check(function_attr->a_constexpr)) {
				int32_t attrid = asm_newconst(function_attr->a_constexpr);
				if unlikely(attrid < 0)
					goto err;
				if (function_self->a_type == AST_SYM) {
					DeeClassScopeObject *class_scope;
					struct symbol *sym = function_self->a_sym;
check_getattr_base_symbol_class_tuple:
					for (class_scope = function_self->a_scope->s_class; class_scope;
					     class_scope = DeeClassScope_Prev(class_scope)) {
						/* Try to statically access known class members! */
						if (sym == class_scope->cs_class ||
						    sym == class_scope->cs_this) {
							funsym = scope_lookup_str(&class_scope->cs_scope,
							                          DeeString_STR(function_attr->a_constexpr),
							                          DeeString_SIZE(function_attr->a_constexpr));
							/* Generate a regular attribute access. */
							if (funsym &&
							    funsym->s_type == SYMBOL_TYPE_CATTR &&
							    funsym->s_attr.a_class == class_scope->cs_class) {
								if (sym == funsym->s_attr.a_this)
									goto invoke_cattr_funsym_tuple; /* Regular access to a dynamic attribute. */
								if (sym == funsym->s_attr.a_class && !funsym->s_attr.a_this)
									goto invoke_cattr_funsym_tuple; /* Regular access to a static-attribute. */
							}
							break;
						}
					}
					switch (sym->s_type) {

					case SYMBOL_TYPE_ALIAS:
						sym = SYMBOL_ALIAS(sym);
						goto check_getattr_base_symbol_class_tuple;

					case SYMBOL_TYPE_THIS:
						if (SYMBOL_MUST_REFERENCE_THIS(sym))
							break;
						if (ast_genasm(args, ASM_G_FPUSHRES))
							goto err;
						if (ast_predict_type(args) != &DeeTuple_Type &&
						    (asm_putddi(args) || asm_gcast_tuple()))
							goto err;
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gcallattr_this_const_tuple((uint16_t)attrid))
							goto err;
						goto pop_unused;

					default: break;
					}
				}
				if (ast_genasm(function_self, ASM_G_FPUSHRES))
					goto err;
				if (ast_genasm_one(args, ASM_G_FPUSHRES))
					goto err;
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_const_tuple((uint16_t)attrid))
					goto err;
				goto pop_unused;
			}
			if (ast_genasm(function_self, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(function_attr, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(args, ASM_G_FPUSHRES))
				goto err;
			if (ast_predict_type(args) != &DeeTuple_Type &&
			    (asm_putddi(args) || asm_gcast_tuple()))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcallattr_tuple())
				goto err;
			goto pop_unused;
		}
		goto generic_call;
	}
	if (args->a_type == AST_CONSTEXPR) {
		/* Special case: call with empty argument list. */
		ASSERT(args->a_constexpr == Dee_EmptyTuple);
		argc = 0;
		argv = NULL;
	} else {
		argc = (uint8_t)args->a_multiple.m_astc;
		argv = args->a_multiple.m_astv;
	}

	if (argc == 1) {
		struct ast *arg0 = argv[0];
		if (arg0->a_type == AST_MULTIPLE &&
		    !ast_chk_multiple_hasexpand(arg0)) {
			if (arg0->a_flag == AST_FMULTIPLE_GENERIC &&
			    arg0->a_multiple.m_astc <= UINT8_MAX) {
				size_t seq_argc       = arg0->a_multiple.m_astc;
				struct ast **seq_argv = arg0->a_multiple.m_astv;
				/* Special case: Brace initializer-call can be encoded as ASM_CALL_SEQ. */
				if (func->a_type == AST_CONSTEXPR &&
				    (DeeObjMethod_Check(func->a_constexpr) ||
				     DeeKwObjMethod_Check(func->a_constexpr))) {
					char const *name = DeeObjMethod_GetName(func->a_constexpr);
					if (name) {
						int32_t attrid;
						DREF DeeObject *name_ob;
						name_ob = DeeString_New(name);
						if unlikely(!name_ob)
							goto err;
						attrid = asm_newconst(name_ob);
						Dee_Decref(name_ob);
						if unlikely(attrid < 0)
							goto err;
						/* callattr with sequence argument. */
						if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
							goto err;
						for (i = 0; i < seq_argc; ++i)
							if (ast_genasm_one(seq_argv[i], ASM_G_FPUSHRES))
								goto err;
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gcallattr_const_seq((uint16_t)attrid, (uint8_t)seq_argc))
							goto err;
						goto pop_unused;
					}
				}
				if (func->a_type == AST_OPERATOR &&
				    func->a_flag == OPERATOR_GETATTR &&
				    !(func->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS)) &&
				    func->a_operator.o_op1 &&
				    func->a_operator.o_op1->a_type == AST_CONSTEXPR &&
				    DeeString_Check(func->a_operator.o_op1->a_constexpr)) {
					int32_t cid;
					/* callattr with sequence argument. */
					if (ast_genasm(func->a_operator.o_op0, ASM_G_FPUSHRES))
						goto err;
					cid = asm_newconst(func->a_operator.o_op1->a_constexpr);
					if unlikely(cid < 0)
						goto err;
					for (i = 0; i < seq_argc; ++i)
						if (ast_genasm_one(seq_argv[i], ASM_G_FPUSHRES))
							goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_const_seq((uint16_t)cid, (uint8_t)seq_argc))
						goto err;
					goto pop_unused;
				}
				if (ast_genasm(func, ASM_G_FPUSHRES))
					goto err;
				for (i = 0; i < seq_argc; ++i)
					if (ast_genasm_one(seq_argv[i], ASM_G_FPUSHRES))
						goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcall_seq((uint8_t)seq_argc))
					goto err;
				goto pop_unused;
			}
			if (arg0->a_flag == AST_FMULTIPLE_GENERIC_KEYS &&
			    arg0->a_multiple.m_astc <= (size_t)UINT8_MAX * 2) {
				size_t seq_argc       = arg0->a_multiple.m_astc & ~1;
				struct ast **seq_argv = arg0->a_multiple.m_astv;
#if 1
				if (func->a_type == AST_OPERATOR &&
				    func->a_flag == OPERATOR_GETATTR &&
				    !(func->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS)) &&
				    func->a_operator.o_op1 &&
				    func->a_operator.o_op1->a_type == AST_CONSTEXPR &&
				    DeeString_Check(func->a_operator.o_op1->a_constexpr)) {
					int32_t cid;
					/* callattr with sequence argument. */
					if (ast_genasm(func->a_operator.o_op0, ASM_G_FPUSHRES))
						goto err;
					cid = asm_newconst(func->a_operator.o_op1->a_constexpr);
					if unlikely(cid < 0)
						goto err;
					for (i = 0; i < seq_argc; ++i)
						if (ast_genasm_one(seq_argv[i], ASM_G_FPUSHRES))
							goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_const_map((uint16_t)cid, (uint8_t)(seq_argc / 2)))
						goto err;
					goto pop_unused;
				}
#endif
				/* Special case: Brace initializer-call with keys can be encoded as ASM_CALL_MAP. */
				if (ast_genasm(func, ASM_G_FPUSHRES))
					goto err;
				for (i = 0; i < seq_argc; ++i)
					if (ast_genasm_one(seq_argv[i], ASM_G_FPUSHRES))
						goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcall_map((uint8_t)(seq_argc / 2)))
					goto err;
				goto pop_unused;
			}
		}
		if (func->a_type == AST_CONSTEXPR) {
			DeeObject *cxpr = func->a_constexpr;
			/* Some casting-style expression have their own opcode. */
#if 0 /* The real constructor has a special case for strings... */
			if (cxpr == (DeeObject *)&DeeInt_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcast_int())
					goto err;
				goto pop_unused;
			}
#endif
			if (cxpr == (DeeObject *)&DeeBool_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gbool(false))
					goto err;
				goto pop_unused;
			}
			if (cxpr == (DeeObject *)&DeeString_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gstr())
					goto err;
				goto pop_unused;
			}
			if (cxpr == (DeeObject *)&DeeTuple_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcast_tuple())
					goto err;
				goto pop_unused;
			}
#if 0 /* The real constructor has a special case for integer pre-sizing... */
			if (cxpr == (DeeObject *)&DeeList_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcast_list())
					goto err;
				goto pop_unused;
			}
#endif
			if (cxpr == (DeeObject *)&DeeDict_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcast_dict())
					goto err;
				goto pop_unused;
			}
			if (cxpr == (DeeObject *)&DeeHashSet_Type) {
				if (ast_genasm(arg0, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcast_hashset())
					goto err;
				goto pop_unused;
			}
		}
	}

	if (func->a_type == AST_SYM) {
		funsym = func->a_sym;
check_funsym_class:
		switch (funsym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			funsym = SYMBOL_ALIAS(funsym);
			goto check_funsym_class;

		case SYMBOL_TYPE_EXTERN:
			if (funsym->s_extern.e_symbol->ss_flags & MODSYM_FPROPERTY)
				break;
			if (funsym->s_extern.e_symbol->ss_flags & MODSYM_FEXTERN) {
				symid = funsym->s_extern.e_symbol->ss_extern.ss_impid;
				ASSERT(symid < funsym->s_extern.e_module->mo_importc);
				symid = asm_newmodule(funsym->s_extern.e_module->mo_importv[symid]);
			} else {
				symid = asm_esymid(funsym);
			}
			if unlikely(symid < 0)
				goto err;
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall_extern((uint16_t)symid, funsym->s_extern.e_symbol->ss_index, argc))
				goto err;
			goto pop_unused;

		case SYMBOL_TYPE_LOCAL:
			if (SYMBOL_MUST_REFERENCE_NOTTHIS(funsym))
				break;
			symid = asm_lsymid_for_read(funsym, func);
			if unlikely(symid < 0)
				goto err;
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall_local((uint16_t)symid, argc))
				goto err;
			goto pop_unused;

		case SYMBOL_TYPE_GLOBAL:
			symid = asm_gsymid_for_read(funsym, func);
			if unlikely(symid < 0)
				goto err;
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall_global((uint16_t)symid, argc))
				goto err;
			goto pop_unused;

		case SYMBOL_TYPE_CATTR: {
			struct symbol *class_sym, *this_sym;
			struct class_attribute *attr;
invoke_cattr_funsym_argv:
			class_sym = funsym->s_attr.a_class;
			this_sym  = funsym->s_attr.a_this;
			attr      = funsym->s_attr.a_attr;
			SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
			if (!this_sym) {
				if (asm_putddi(func))
					goto err;
				if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if (asm_ggetcmember_r(symid, attr->ca_addr))
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, func))
						goto err;
					if (asm_ggetcmember(attr->ca_addr))
						goto err;
				}
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					/* Must invoke the getter callback. */
					if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						if (asm_gpush_symbol(class_sym, func))
							goto err; /* getter, class */
						if (asm_gcall(1))
							goto err; /* func */
					} else {
						if (asm_gcall(0))
							goto err;
					}
				} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					if (asm_gpush_symbol(class_sym, func))
						goto err; /* func, class_sym */
					if (argc != (uint8_t)-1) {
						for (i = 0; i < argc; ++i)
							if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
								goto err;
						if (asm_putddi(ddi_ast))
							goto err; /* func, class_sym, args... */
						if (asm_gcall(argc + 1))
							goto err; /* result */
						goto pop_unused;
					}
					symid = asm_newmodule(DeeModule_GetDeemon());
					if unlikely(symid < 0)
						goto err; /* Call as an InstanceMethod */
					if (asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2))
						goto err;
					/* Fallthrough to invoke the InstanceMethod normally. */
				}
				for (i = 0; i < argc; ++i)
					if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
						goto err;
				if (asm_putddi(ddi_ast))
					goto err; /* func, args... */
				if (asm_gcall(argc))
					goto err; /* result */
				goto pop_unused;
			}
			/* The attribute must be accessed as virtual. */
			if unlikely(asm_check_thiscall(funsym, func))
				goto err;
			SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
			if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
				symid = asm_newconst((DeeObject *)attr->ca_name);
				if unlikely(symid < 0)
					goto err;
				if (this_sym->s_type == SYMBOL_TYPE_THIS &&
				    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
					for (i = 0; i < argc; ++i)
						if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
							goto err;
					if (asm_putddi(ddi_ast))
						goto err; /* args... */
					if (asm_gcallattr_this_const((uint16_t)symid, argc))
						goto err;
					goto pop_unused;
				}
				if (asm_putddi(func))
					goto err;
				if (asm_gpush_symbol(this_sym, func))
					goto err; /* this */
				for (i = 0; i < argc; ++i)
					if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
						goto err;
				if (asm_putddi(ddi_ast))
					goto err; /* this, args... */
				if (asm_gcallattr_const((uint16_t)symid, argc))
					goto err; /* result */
				goto pop_unused;
			}
			/* Regular, old member variable. */
			if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
				if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if ((attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) &&
					    this_sym->s_type == SYMBOL_TYPE_THIS &&
					    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
						if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
							/* Invoke the getter callback. */
							if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET, 0))
								goto err;
							goto got_method;
						}
						for (i = 0; i < argc; ++i)
							if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
								goto err;
						if (asm_putddi(ddi_ast))
							goto err;
						if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr, argc))
							goto err;
						goto pop_unused;
					}
					if (asm_putddi(func))
						goto err;
					if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr))
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, func))
						goto err;
					if (asm_putddi(func))
						goto err;
					if (asm_ggetcmember(attr->ca_addr))
						goto err;
				}
			} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
			           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
				if (asm_putddi(func))
					goto err;
				if (asm_gpush_symbol(this_sym, func))
					goto err;
				if (asm_gpush_symbol(class_sym, func))
					goto err;
				if (asm_ggetmember(attr->ca_addr))
					goto err;
			} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if (asm_putddi(func))
					goto err;
				if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr))
					goto err;
			} else {
				if (asm_putddi(func))
					goto err;
				if (asm_gpush_symbol(class_sym, func))
					goto err;
				if (asm_ggetmember_this(attr->ca_addr))
					goto err;
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				/* Call the getter of the attribute. */
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					/* Invoke as a this-call. */
					if (asm_gpush_symbol(this_sym, func))
						goto err;
					if (asm_gcall(1))
						goto err;
				} else {
					if (asm_gcall(0))
						goto err; /* Directly invoke. */
				}
			} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				/* Access to an instance member function (must produce a bound method). */
				if (asm_gpush_symbol(this_sym, func))
					goto err; /* func, this */
				if unlikely(argc != (uint8_t)-1) {
					for (i = 0; i < argc; ++i)
						if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
							goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall(argc + 1))
						goto err;
					goto pop_unused;
				}
				symid = asm_newmodule(DeeModule_GetDeemon());
				if unlikely(symid < 0)
					goto err; /* Call as an InstanceMethod */
				if (asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2))
					goto err;
				/* Fallthrough to invoke the InstanceMethod normally. */
			}
got_method:
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err; /* func, args... */
			if (asm_gcall(argc))
				goto err; /* result */
			goto pop_unused;
		}	break;

		case SYMBOL_TYPE_MYFUNC:
			if (funsym->s_scope->s_base != current_basescope)
				break;
			if (!(current_basescope->bs_flags & CODE_FTHISCALL))
				break;
			if unlikely(argc >= (uint8_t)-1)
				break;
			ASSERT(!SYMBOL_MUST_REFERENCE_NOTTHIS(funsym));
			/* Call to the current or surrounding function, when
			 * that function is defined to be a this-call function.
			 * In this case, pushing the raw function would result
			 * in an InstanceMethod object having to be created at
			 * runtime, which is something that we can prevent by
			 * referencing the associated this-argument and simply
			 * calling the function directly. */
			if (asm_putddi(func))
				goto err;
			if (asm_gpush_this_function())
				goto err;
			if (asm_gpush_this())
				goto err;
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall(argc + 1))
				goto err;
			goto pop_unused;

		default:
			break;
		}
	}
	if (func->a_type == AST_CONSTEXPR &&
	    (DeeObjMethod_Check(func->a_constexpr) ||
	     DeeKwObjMethod_Check(func->a_constexpr))) {
		char const *name = DeeObjMethod_GetName(func->a_constexpr);
		if (name) {
			int32_t attrid;
			DREF DeeObject *name_ob;
			name_ob = DeeString_New(name);
			if unlikely(!name_ob)
				goto err;
			attrid = asm_newconst(name_ob);
			Dee_Decref(name_ob);
			if unlikely(attrid < 0)
				goto err;
			/* call to some other object. */
			if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
				goto err;
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcallattr_const((uint16_t)attrid, argc))
				goto err;
			goto pop_unused;
		}
	}
	if (func->a_type == AST_OPERATOR &&
	    func->a_flag == OPERATOR_GETATTR &&
	    !(func->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS))) {
		struct ast *function_self = func->a_operator.o_op0;
		struct ast *function_attr = func->a_operator.o_op1;
		if unlikely(!function_attr)
			goto generic_call;
		/* Call to an attribute with stack-based argument list. */
		if (function_attr->a_type == AST_CONSTEXPR &&
		    DeeString_Check(function_attr->a_constexpr)) {
			int32_t attrid;
			/* (very) likely case: call to an attribute with a constant name. */
			if (function_self->a_type == AST_SYM) {
				struct symbol *sym = function_self->a_sym;
				DeeClassScopeObject *class_scope;
check_getattr_base_symbol_class_argv:
				for (class_scope = function_self->a_scope->s_class; class_scope;
				     class_scope = DeeClassScope_Prev(class_scope)) {
					/* Try to statically access known class members! */
					if (sym == class_scope->cs_class ||
					    sym == class_scope->cs_this) {
						funsym = scope_lookup_str(&class_scope->cs_scope,
						                          DeeString_STR(function_attr->a_constexpr),
						                          DeeString_SIZE(function_attr->a_constexpr));
						/* Generate a regular attribute access. */
						if (funsym &&
						    funsym->s_type == SYMBOL_TYPE_CATTR &&
						    funsym->s_attr.a_class == class_scope->cs_class) {
							if (sym == funsym->s_attr.a_this)
								goto invoke_cattr_funsym_argv; /* Regular access to a dynamic attribute. */
							if (sym == funsym->s_attr.a_class && !funsym->s_attr.a_this)
								goto invoke_cattr_funsym_argv; /* Regular access to a static-attribute. */
						}
						break;
					}
				}
				switch (SYMBOL_TYPE(sym)) {

				case SYMBOL_TYPE_ALIAS:
					sym = SYMBOL_ALIAS(sym);
					goto check_getattr_base_symbol_class_argv;

				case SYMBOL_TYPE_THIS:
					if (SYMBOL_MUST_REFERENCE_THIS(sym))
						break;
					/* call to the `this' argument. (aka. in-class member call) */
					for (i = 0; i < argc; ++i)
						if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
							goto err;
					attrid = asm_newconst(function_attr->a_constexpr);
					if unlikely(attrid < 0)
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_this_const((uint16_t)attrid, argc))
						goto err;
					goto pop_unused;

				case SYMBOL_TYPE_MODULE: {
					struct module_symbol *modsym;
					int32_t module_id;
					/* module.attr() --> call extern ... */
					modsym = get_module_symbol(sym->s_module,
					                           (DeeStringObject *)function_attr->a_constexpr);
					if (!modsym)
						break;
					if (modsym->ss_flags & MODSYM_FPROPERTY)
						break;
					if (modsym->ss_flags & MODSYM_FEXTERN) {
						ASSERT(modsym->ss_extern.ss_impid < sym->s_module->mo_importc);
						module_id = asm_newmodule(sym->s_module->mo_importv[modsym->ss_extern.ss_impid]);
					} else {
						module_id = asm_msymid(sym);
					}
					if unlikely(module_id < 0)
						goto err;
					/* Do a call to an external symbol. `ASM_CALL_EXTERN' */
					for (i = 0; i < argc; ++i)
						if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
							goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcall_extern((uint16_t)module_id, modsym->ss_index, argc))
						goto err;
					goto pop_unused;
				}	break;

				default:
					break;
				}
			}
			if (function_self->a_type == AST_ACTION &&
			    function_self->a_flag == AST_FACTION_AS &&
			    function_self->a_action.a_act0->a_type == AST_SYM &&
			    function_self->a_action.a_act0->a_sym->s_type == SYMBOL_TYPE_THIS &&
			    !SYMBOL_MUST_REFERENCE_THIS(function_self->a_action.a_act0->a_sym)) {
				/* `(this as ...).foobar(a,b,c)'
				 * -> Check if we can make use of `ASM_SUPERGETATTR_THIS_RC' instructions. */
				struct ast *type_expr = function_self->a_action.a_act1;
				int32_t type_rid;
				if (type_expr->a_type == AST_SYM &&
				    ASM_SYMBOL_MAY_REFERENCE(type_expr->a_sym)) {
					/* We are allowed to reference the base-symbol! */
					type_rid = asm_rsymid(type_expr->a_sym);
do_perform_supercallattr_argv:
					if unlikely(type_rid < 0)
						goto err;
					attrid = asm_newconst((DeeObject *)function_attr->a_constexpr);
					if unlikely(attrid < 0)
						goto err;
					for (i = 0; i < argc; ++i)
						if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
							goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gsupercallattr_this_rc((uint16_t)type_rid, (uint16_t)attrid, argc))
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
						goto do_perform_supercallattr_argv;
					}
				}
			}
			/* call to some other object. */
			attrid = asm_newconst(function_attr->a_constexpr);
			if unlikely(attrid < 0)
				goto err;
			if (ast_genasm(function_self, ASM_G_FPUSHRES))
				goto err;
			for (i = 0; i < argc; ++i)
				if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
					goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcallattr_const((uint16_t)attrid, argc))
				goto err;
			goto pop_unused;
		}
		/* Pretty unlikely: The attribute name is not known.
		 * Due to the runtime optimization impact that optimizing this still has,
		 * there is also an opcode for this (callattr() is much faster than getattr+call). */
		if (ast_genasm(function_self, ASM_G_FPUSHRES))
			goto err;
		if (ast_genasm_one(function_attr, ASM_G_FPUSHRES))
			goto err;
		for (i = 0; i < argc; ++i)
			if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
				goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gcallattr(argc))
			goto err;
		goto pop_unused;
	}
	/* Call with stack-based argument list. */
	if (ast_genasm(func, ASM_G_FPUSHRES))
		goto err;
	for (i = 0; i < argc; ++i)
		if (ast_genasm_one(argv[i], ASM_G_FPUSHRES))
			goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	if (asm_gcall(argc))
		goto err;
pop_unused:
	if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
		goto err;
	return 0;
generic_call:
	if (ast_genasm(func, ASM_G_FPUSHRES))
		goto err;
	if (ast_genasm_one(args, ASM_G_FPUSHRES))
		goto err;
	if (ast_predict_type(args) != &DeeTuple_Type &&
	    (asm_putddi(args) || asm_gcast_tuple()))
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	if (asm_gcall_tuple())
		goto err;
	goto pop_unused;
err:
	return -1;
}


INTERN int DCALL
asm_gcall_kw_expr(struct ast *__restrict func,
                  struct ast *__restrict args,
                  struct ast *__restrict kwds,
                  struct ast *__restrict ddi_ast,
                  unsigned int gflags) {
	/* Optimizations for (highly likely) invocations, using the dedicated instruction. */
	if (func->a_type == AST_CONSTEXPR &&
	    (DeeObjMethod_Check(func->a_constexpr) ||
	     DeeKwObjMethod_Check(func->a_constexpr))) {
		char const *name = DeeObjMethod_GetName(func->a_constexpr);
		if (name) {
			int32_t attrid;
			DREF DeeObject *name_ob;
			name_ob = DeeString_New(name);
			if unlikely(!name_ob)
				goto err;
			attrid = asm_newconst(name_ob);
			Dee_Decref(name_ob);
			if unlikely(attrid < 0)
				goto err;
			if (kwds->a_type == AST_CONSTEXPR) {
				int32_t kwd_cid;
				kwd_cid = asm_newconst(kwds->a_constexpr);
				if unlikely(kwd_cid < 0)
					goto err;
				if (args->a_type == AST_MULTIPLE &&
				    AST_FMULTIPLE_ISSEQUENCE(args->a_flag) &&
				    args->a_multiple.m_astc <= UINT8_MAX &&
				    !ast_chk_multiple_hasexpand(args)) {
					size_t i;
					if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
						goto err;
					for (i = 0; i < args->a_multiple.m_astc; ++i) {
						if (ast_genasm_one(args->a_multiple.m_astv[i], ASM_G_FPUSHRES))
							goto err;
					}
					if (ast_predict_type(args) != &DeeTuple_Type &&
					    (asm_putddi(args) || asm_gcast_tuple()))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_const_kw((uint16_t)attrid,
					                           (uint8_t)args->a_multiple.m_astc,
					                           (uint16_t)kwd_cid))
						goto err;
					goto pop_unused;
				}
				if (args->a_type == AST_CONSTEXPR &&
				    args->a_constexpr == Dee_EmptyTuple) {
					if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
						goto err;
					if (ast_genasm_one(args, ASM_G_FPUSHRES))
						goto err;
					if (ast_predict_type(args) != &DeeTuple_Type &&
					    (asm_putddi(args) || asm_gcast_tuple()))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_const_kw((uint16_t)attrid, 0, (uint16_t)kwd_cid))
						goto err;
					goto pop_unused;
				}
				if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
					goto err;
				if (ast_genasm_one(args, ASM_G_FPUSHRES))
					goto err;
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_const_tuple_kw((uint16_t)attrid, (uint16_t)kwd_cid))
					goto err;
				goto pop_unused;
			} else {
				if (args->a_type == AST_MULTIPLE &&
				    AST_FMULTIPLE_ISSEQUENCE(args->a_flag) &&
				    args->a_multiple.m_astc <= UINT8_MAX &&
				    !ast_chk_multiple_hasexpand(args)) {
					size_t i;
					if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
						goto err;
					if (asm_gpush_const((uint16_t)attrid))
						goto err;
					for (i = 0; i < args->a_multiple.m_astc; ++i) {
						if (ast_genasm_one(args->a_multiple.m_astv[i], ASM_G_FPUSHRES))
							goto err;
					}
					if (ast_predict_type(args) != &DeeTuple_Type &&
					    (asm_putddi(args) || asm_gcast_tuple()))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
						goto err;
					if (asm_gcallattr_kwds((uint8_t)args->a_multiple.m_astc))
						goto err;
					goto pop_unused;
				}
				if (args->a_type == AST_CONSTEXPR &&
				    args->a_constexpr == Dee_EmptyTuple) {
					if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
						goto err;
					if (asm_gpush_const((uint16_t)attrid))
						goto err;
					if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
						goto err;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_gcallattr_kwds(0))
						goto err;
					goto pop_unused;
				}
				if (asm_gpush_constexpr(DeeObjMethod_SELF(func->a_constexpr)))
					goto err;
				if (asm_gpush_const((uint16_t)attrid))
					goto err;
				if (ast_genasm_one(args, ASM_G_FPUSHRES))
					goto err;
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_tuple_kwds())
					goto err;
				goto pop_unused;
			}
		}
	}
	if (func->a_type == AST_OPERATOR &&
	    func->a_flag == OPERATOR_GETATTR &&
	    !(func->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS))) {
		struct ast *base, *name;
		base = func->a_operator.o_op0;
		name = func->a_operator.o_op1;
		if (kwds->a_type == AST_CONSTEXPR &&
		    name->a_type == AST_CONSTEXPR &&
		    DeeString_Check(name->a_constexpr)) {
			int32_t kwd_cid, att_cid;
			kwd_cid = asm_newconst(kwds->a_constexpr);
			if unlikely(kwd_cid < 0)
				goto err;
			att_cid = asm_newconst(name->a_constexpr);
			if unlikely(att_cid < 0)
				goto err;
			if (args->a_type == AST_MULTIPLE &&
			    AST_FMULTIPLE_ISSEQUENCE(args->a_flag) &&
			    args->a_multiple.m_astc <= UINT8_MAX &&
			    !ast_chk_multiple_hasexpand(args)) {
				size_t i;
				if (ast_genasm(base, ASM_G_FPUSHRES))
					goto err;
				for (i = 0; i < args->a_multiple.m_astc; ++i) {
					if (ast_genasm_one(args->a_multiple.m_astv[i], ASM_G_FPUSHRES))
						goto err;
				}
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_const_kw((uint16_t)att_cid,
				                           (uint8_t)args->a_multiple.m_astc,
				                           (uint16_t)kwd_cid))
					goto err;
				goto pop_unused;
			}
			if (args->a_type == AST_CONSTEXPR &&
			    args->a_constexpr == Dee_EmptyTuple) {
				if (ast_genasm(base, ASM_G_FPUSHRES))
					goto err;
				if (ast_genasm_one(args, ASM_G_FPUSHRES))
					goto err;
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_const_kw((uint16_t)att_cid, 0, (uint16_t)kwd_cid))
					goto err;
				goto pop_unused;
			}
			if (ast_genasm(base, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(args, ASM_G_FPUSHRES))
				goto err;
			if (ast_predict_type(args) != &DeeTuple_Type &&
			    (asm_putddi(args) || asm_gcast_tuple()))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcallattr_const_tuple_kw((uint16_t)att_cid, (uint16_t)kwd_cid))
				goto err;
			goto pop_unused;
		} else {
			if (args->a_type == AST_MULTIPLE &&
			    AST_FMULTIPLE_ISSEQUENCE(args->a_flag) &&
			    args->a_multiple.m_astc <= UINT8_MAX &&
			    !ast_chk_multiple_hasexpand(args)) {
				size_t i;
				if (ast_genasm(base, ASM_G_FPUSHRES))
					goto err;
				if (ast_genasm_one(name, ASM_G_FPUSHRES))
					goto err;
				for (i = 0; i < args->a_multiple.m_astc; ++i) {
					if (ast_genasm_one(args->a_multiple.m_astv[i], ASM_G_FPUSHRES))
						goto err;
				}
				if (ast_predict_type(args) != &DeeTuple_Type &&
				    (asm_putddi(args) || asm_gcast_tuple()))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
					goto err;
				if (asm_gcallattr_kwds((uint8_t)args->a_multiple.m_astc))
					goto err;
				goto pop_unused;
			}
			if (args->a_type == AST_CONSTEXPR &&
			    args->a_constexpr == Dee_EmptyTuple) {
				if (ast_genasm(base, ASM_G_FPUSHRES))
					goto err;
				if (ast_genasm_one(name, ASM_G_FPUSHRES))
					goto err;
				if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
					goto err;
				if (asm_putddi(ddi_ast))
					goto err;
				if (asm_gcallattr_kwds(0))
					goto err;
				goto pop_unused;
			}
			if (ast_genasm(base, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(name, ASM_G_FPUSHRES))
				goto err;
			if (ast_genasm_one(args, ASM_G_FPUSHRES))
				goto err;
			if (ast_predict_type(args) != &DeeTuple_Type &&
			    (asm_putddi(args) || asm_gcast_tuple()))
				goto err;
			if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcallattr_tuple_kwds())
				goto err;
			goto pop_unused;
		}
	}
	/* The object being called isn't an attribute. */
	if (ast_genasm(func, ASM_G_FPUSHRES))
		goto err;
	if (kwds->a_type == AST_CONSTEXPR) {
		int32_t kwd_cid;
		kwd_cid = asm_newconst(kwds->a_constexpr);
		if unlikely(kwd_cid < 0)
			goto err;
		if (args->a_type == AST_MULTIPLE &&
		    AST_FMULTIPLE_ISSEQUENCE(args->a_flag) &&
		    args->a_multiple.m_astc <= UINT8_MAX &&
		    !ast_chk_multiple_hasexpand(args)) {
			size_t i;
			for (i = 0; i < args->a_multiple.m_astc; ++i) {
				if (ast_genasm_one(args->a_multiple.m_astv[i], ASM_G_FPUSHRES))
					goto err;
			}
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall_kw((uint8_t)args->a_multiple.m_astc,
			                 (uint16_t)kwd_cid))
				goto err;
		} else if (args->a_type == AST_CONSTEXPR &&
		           args->a_constexpr == Dee_EmptyTuple) {
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall_kw(0, (uint16_t)kwd_cid))
				goto err;
		} else {
			if (ast_genasm_one(args, ASM_G_FPUSHRES))
				goto err;
			if (ast_predict_type(args) != &DeeTuple_Type &&
			    (asm_putddi(args) || asm_gcast_tuple()))
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			if (asm_gcall_tuple_kw((uint16_t)kwd_cid))
				goto err;
		}
	} else {
		/* Fallback: use the stack to pass all the arguments. */
		if (ast_genasm_one(args, ASM_G_FPUSHRES))
			goto err;
		if (ast_predict_type(args) != &DeeTuple_Type &&
		    (asm_putddi(args) || asm_gcast_tuple()))
			goto err;
		if (ast_genasm_one(kwds, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_gcall_tuple_kwds())
			goto err;
	}
pop_unused:
	if (!(gflags & ASM_G_FPUSHRES)) {
		if (asm_gpop())
			goto err;
	}
	return 0;
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENCALL_C */
