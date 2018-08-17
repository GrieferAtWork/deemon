/* Copyright (c) 2018 Griefer@Work                                            *
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

#include <deemon/api.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/list.h>
#include <deemon/dict.h>
#include <deemon/hashset.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN

PRIVATE int DCALL
push_tuple_items(DeeObject *__restrict self) {
 DeeObject **iter,**end;
 end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
 for (; iter != end; ++iter) if (asm_gpush_constexpr(*iter)) return -1;
 return 0;
}

INTDEF struct module_symbol *DCALL
get_module_symbol(DeeModuleObject *__restrict module,
                  DeeStringObject *__restrict name);

INTERN int DCALL
asm_gcall_expr(DeeAstObject *__restrict func,
               DeeAstObject *__restrict args,
               DeeAstObject *__restrict ddi_ast,
               unsigned int gflags) {
 /* Special instruction encoding for call operations. */
 struct symbol *funsym; int32_t symid;
 uint8_t i,argc; DeeAstObject **argv;
 ASSERT_OBJECT_TYPE(func,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(args,&DeeAst_Type);
 if (args->ast_type == AST_MULTIPLE &&
     args->ast_multiple.ast_exprc != 0 &&
     args->ast_multiple.ast_exprc <= UINT8_MAX) {
  /* Check if this call can be encoded as an argument-forward-call. */
  size_t i; uint16_t start_offset; struct symbol *arg_sym;
  DeeAstObject *arg = args->ast_multiple.ast_exprv[0];
  if (arg->ast_type == AST_EXPAND) {
   /* If the argument is an expand expression, check for the varargs argument. */
   arg = arg->ast_expandexpr;
   if (arg->ast_type != AST_SYM)
       goto not_argument_forward;
   arg_sym = arg->ast_sym;
   SYMBOL_INPLACE_UNWIND_ALIAS(arg_sym);
   if (SYMBOL_TYPE(arg_sym) != SYMBOL_TYPE_ARG)
       goto not_argument_forward;
   if (SYMBOL_MUST_REFERENCE_TYPEMAY(arg_sym))
       goto not_argument_forward;
   start_offset = arg_sym->s_symid;
   /* Make sure that this is the varargs symbol. */
   if (!DeeBaseScope_IsArgVarArgs(current_basescope,start_offset))
        goto not_argument_forward;
  } else {
   if (arg->ast_type != AST_SYM)
       goto not_argument_forward;
   arg_sym = arg->ast_sym;
   SYMBOL_INPLACE_UNWIND_ALIAS(arg_sym);
   if (SYMBOL_TYPE(arg_sym) != SYMBOL_TYPE_ARG)
       goto not_argument_forward;
   if (SYMBOL_MUST_REFERENCE_TYPEMAY(arg_sym))
       goto not_argument_forward;
   start_offset = arg_sym->s_symid;
   /* Make sure that this isn't the varargs symbol. */
   if (DeeBaseScope_IsArgVarArgs(current_basescope,start_offset))
       goto not_argument_forward;
  }
  /* Check if the final argument range can fit the
   * 8-bit operand limit for argument forwarding. */
  if ((size_t)start_offset+args->ast_multiple.ast_exprc > (size_t)UINT8_MAX+1)
      goto not_argument_forward;
  /* Check if the remainder of arguments follows our own argument list. */
  for (i = 1; i < args->ast_multiple.ast_exprc; ++i) {
   arg = args->ast_multiple.ast_exprv[i];
   if (DeeBaseScope_IsArgVarArgs(current_basescope,(uint16_t)i)) {
    /* The varargs-symbol must appear as an expand expression to be forwarded. */
    if (arg->ast_type != AST_EXPAND) goto not_argument_forward;
    arg = arg->ast_expandexpr;
   }
   if (arg->ast_type != AST_SYM)
       goto not_argument_forward;
   arg_sym = arg->ast_sym;
   SYMBOL_INPLACE_UNWIND_ALIAS(arg_sym);
   if (SYMBOL_TYPE(arg_sym) != SYMBOL_TYPE_ARG)
       goto not_argument_forward;
   if (SYMBOL_MUST_REFERENCE_TYPEMAY(arg_sym))
       goto not_argument_forward;
   if ((size_t)arg_sym->s_symid != (size_t)start_offset+i)
       goto not_argument_forward;
   /* Optional arguments cannot be forwarded, because accessing
    * them can have the side-effect of them not being bound, so
    * we can't just forward them in a call-forward. */
   if (DeeBaseScope_IsArgOptional(current_basescope,(uint16_t)i))
       goto not_argument_forward;
  }
#if 1 /* Optimization: When only a couple of arguments are being forwarded,
       *               consider if there even is an advantage over doing a
       *               regular call that can be optimized much better than
       *               this very specific argument-forward call. */
  if ((size_t)args->ast_multiple.ast_exprc <=
      (size_t)(func->ast_type == AST_OPERATOR && func->ast_flag == OPERATOR_GETATTR ? 3 : 1) &&
      !DeeBaseScope_IsArgVarArgs(current_basescope,arg_sym->s_symid))
       goto not_argument_forward;
#endif

  /* This one can be encoded as an argument-forward-call! */
  if (ast_genasm(func,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_gcall_argsfwd(start_offset,
                        start_offset+
                        args->ast_multiple.ast_exprc-1))
      goto err;
  goto pop_unused;
 }
not_argument_forward:

 /* Optimized call for stack-packed argument list within an 8-bit range. */
 if ((args->ast_type != AST_MULTIPLE ||
      args->ast_flag == AST_FMULTIPLE_KEEPLAST ||
      args->ast_multiple.ast_exprc > UINT8_MAX ||
      ast_multiple_hasexpand(args)) &&
     (args->ast_type != AST_CONSTEXPR ||
      args->ast_constexpr != Dee_EmptyTuple)) {
  if (args->ast_type == AST_CONSTEXPR &&
      DeeTuple_Check(args->ast_constexpr) &&
      DeeTuple_SIZE(args->ast_constexpr) <= 2) {
   argc = (uint8_t)DeeTuple_SIZE(args->ast_constexpr);
   /* Special handling for small argument tuples to prevent this:
    * >> push global @foo
    * >> pack tuple, #0
    * >> call top, pop
    * That assembly is kind-of less efficient than this:
    * >> call global @foo, #0
    * So that's why we can't just blindly go ahead and
    * always make use of constant expression tuples... */
   if (func->ast_type == AST_SYM) {
    funsym = func->ast_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(funsym);
    if ((SYMBOL_TYPE(funsym) == SYMBOL_TYPE_EXTERN &&
       !(SYMBOL_EXTERN_SYMBOL(funsym)->ss_flags & MODSYM_FPROPERTY)) ||
        (funsym->s_type == SYMBOL_TYPE_GLOBAL ||
        (funsym->s_type == SYMBOL_TYPE_LOCAL &&
        !SYMBOL_MUST_REFERENCE_TYPEMAY(funsym)))) {
     if unlikely(push_tuple_items(args->ast_constexpr)) goto err;
     /* Direct call to symbol. */
     if (asm_putddi(ddi_ast)) goto err;
     if (SYMBOL_TYPE(funsym) == SYMBOL_TYPE_EXTERN) {
      if unlikely((symid = asm_esymid(funsym)) < 0) goto err;
      ASSERT(SYMBOL_EXTERN_SYMBOL(funsym));
      if (asm_gcall_extern((uint16_t)symid,SYMBOL_EXTERN_SYMBOL(funsym)->ss_index,argc)) goto err;
     } else if (funsym->s_type == SYMBOL_TYPE_GLOBAL) {
      if unlikely((symid = asm_gsymid_for_read(funsym,func)) < 0) goto err;
      if (asm_gcall_global((uint16_t)symid,argc)) goto err;
     } else {
      if unlikely((symid = asm_lsymid_for_read(funsym,func)) < 0) goto err;
      if (asm_gcall_local((uint16_t)symid,argc)) goto err;
     }
     goto pop_unused;
    }
   }
   if (func->ast_type == AST_OPERATOR &&
       func->ast_flag == OPERATOR_GETATTR &&
     !(func->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS))) {
    DeeAstObject *function_self = func->ast_operator.ast_opa;
    DeeAstObject *function_attr = func->ast_operator.ast_opb;
    if unlikely(!function_attr) goto generic_call;
    /* Call to an attribute with stack-based argument list. */
    if (function_attr->ast_type == AST_CONSTEXPR &&
        DeeString_Check(function_attr->ast_constexpr)) {
     int32_t attrid;
     /* (very) likely case: call to an attribute with a constant name. */
     if (function_self->ast_type == AST_SYM) {
      struct symbol *sym = function_self->ast_sym;
check_function_symbol_class:
      if (!SYMBOL_MUST_REFERENCE(sym)) {
       switch (SYMBOL_TYPE(sym)) {
       case SYMBOL_TYPE_ALIAS:
        ASSERT(SYMBOL_TYPE(SYMBOL_ALIAS(sym)) != SYMBOL_TYPE_ALIAS);
        sym = SYMBOL_ALIAS(sym);
        goto check_function_symbol_class;

       case SYMBOL_TYPE_THIS:
        /* call to the `this' argument. (aka. in-class member call) */
        if unlikely(push_tuple_items(args->ast_constexpr)) goto err;
        attrid = asm_newconst(function_attr->ast_constexpr);
        if unlikely(attrid < 0) goto err;
        if (asm_putddi(ddi_ast)) goto err;
        if (asm_gcallattr_this_const((uint16_t)attrid,argc)) goto err;
        goto pop_unused;
       {
        struct module_symbol *modsym; int32_t module_id;
       case SYMBOL_TYPE_MODULE: /* module.attr() --> call extern ... */
        modsym = get_module_symbol(SYMBOL_MODULE_MODULE(sym),
                                  (DeeStringObject *)function_attr->ast_constexpr);
        if (!modsym) break;
        module_id = asm_msymid(sym);
        if unlikely(module_id < 0) goto err;
        /* Do a call to an external symbol. `ASM_CALL_EXTERN' */
        if unlikely(push_tuple_items(args->ast_constexpr)) goto err;
        if (asm_putddi(ddi_ast)) goto err;
        if (asm_gcall_extern((uint16_t)module_id,modsym->ss_index,argc)) goto err;
        goto pop_unused;
       } break;
       default: break;
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
      attrid = asm_newconst(function_attr->ast_constexpr);
      if unlikely(attrid < 0) goto err;
      if (ast_genasm(function_self,ASM_G_FPUSHRES)) goto err;
      if unlikely(push_tuple_items(args->ast_constexpr)) goto err;
      if (asm_putddi(ddi_ast)) goto err;
      if (asm_gcallattr_const((uint16_t)attrid,argc)) goto err;
      goto pop_unused;
     }
    } else if (argc <= 1) {
     /* Pretty unlikely: The attribute name is not known.
      * Due to the runtime optimization impact that optimizing this still has,
      * there is also an opcode for this (callattr() is much faster than getattr+call). */
     if (ast_genasm(function_self,ASM_G_FPUSHRES)) goto err;
     if (ast_genasm(function_attr,ASM_G_FPUSHRES)) goto err;
     if unlikely(push_tuple_items(args->ast_constexpr)) goto err;
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_gcallattr(argc)) goto err;
     goto pop_unused;
    }
   }
  }
  if (func->ast_type == AST_OPERATOR &&
      func->ast_flag == OPERATOR_GETATTR &&
    !(func->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS))) {
   DeeAstObject *function_self = func->ast_operator.ast_opa;
   DeeAstObject *function_attr = func->ast_operator.ast_opb;
   if unlikely(!function_attr) goto generic_call;
   /* callattr() with an argument list only known at runtime.
    * This can actually happen _very_ easily when expand expressions are being used. */
   if (function_attr->ast_type == AST_CONSTEXPR &&
       DeeString_Check(function_attr->ast_constexpr)) {
    int32_t attrid = asm_newconst(function_attr->ast_constexpr);
    if unlikely(attrid < 0) goto err;
    if (function_self->ast_type == AST_SYM) {
     SYMBOL_INPLACE_UNWIND_ALIAS(function_self->ast_sym);
     if (SYMBOL_TYPE(function_self->ast_sym) == SYMBOL_TYPE_THIS &&
        !SYMBOL_MUST_REFERENCE_TYPEMAY(function_self->ast_sym)) {
      if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
      if (asm_putddi(ddi_ast)) goto err;
      if (ast_predict_type(args) != &DeeTuple_Type && asm_gcast_tuple()) goto err;
      if (asm_gcallattr_this_const_tuple((uint16_t)attrid)) goto err;
      goto pop_unused;
     }
    }
    if (ast_genasm(function_self,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (ast_predict_type(args) != &DeeTuple_Type && asm_gcast_tuple()) goto err;
    if (asm_gcallattr_const_tuple((uint16_t)attrid)) goto err;
    goto pop_unused;
   }
   if (ast_genasm(function_self,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(function_attr,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (ast_predict_type(args) != &DeeTuple_Type && asm_gcast_tuple()) goto err;
   if (asm_gcallattr_tuple()) goto err;
   goto pop_unused;
  }
  goto generic_call;
 }
 if (args->ast_type == AST_CONSTEXPR) {
  /* Special case: call with empty argument list. */
  ASSERT(args->ast_constexpr == Dee_EmptyTuple);
  argc = 0;
  argv = NULL;
 } else {
  argc = (uint8_t)args->ast_multiple.ast_exprc;
  argv = args->ast_multiple.ast_exprv;
 }

 if (argc == 1) {
  DeeAstObject *arg0 = argv[0];
  if (arg0->ast_type == AST_MULTIPLE &&
     !ast_multiple_hasexpand(arg0)) {
   if (arg0->ast_flag == AST_FMULTIPLE_GENERIC &&
       arg0->ast_multiple.ast_exprc <= UINT8_MAX) {
    size_t         seq_argc = arg0->ast_multiple.ast_exprc;
    DeeAstObject **seq_argv = arg0->ast_multiple.ast_exprv;
    /* Special case: Brace initializer-call can be encoded as ASM_CALL_SEQ. */
#if 1
    if (func->ast_type == AST_OPERATOR &&
        func->ast_flag == OPERATOR_GETATTR &&
      !(func->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) &&
        func->ast_operator.ast_opb &&
        func->ast_operator.ast_opb->ast_type == AST_CONSTEXPR &&
        DeeString_Check(func->ast_operator.ast_opb->ast_constexpr)) {
     int32_t cid;
     /* callattr with sequence argument. */
     if (ast_genasm(func->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
     cid = asm_newconst(func->ast_operator.ast_opb->ast_constexpr);
     if unlikely(cid < 0) goto err;
     for (i = 0; i < seq_argc; ++i) if (ast_genasm(seq_argv[i],ASM_G_FPUSHRES)) goto err;
     if (asm_gcallattr_const_seq((uint16_t)cid,(uint8_t)seq_argc)) goto err;
     goto pop_unused;
    }
#endif
    if (ast_genasm(func,ASM_G_FPUSHRES)) goto err;
    for (i = 0; i < seq_argc; ++i) if (ast_genasm(seq_argv[i],ASM_G_FPUSHRES)) goto err;
    if (asm_gcall_seq((uint8_t)seq_argc)) goto err;
    goto pop_unused;
   }
   if (arg0->ast_flag == AST_FMULTIPLE_GENERIC_KEYS &&
       arg0->ast_multiple.ast_exprc <= (size_t)UINT8_MAX*2) {
    size_t         seq_argc = arg0->ast_multiple.ast_exprc & ~1;
    DeeAstObject **seq_argv = arg0->ast_multiple.ast_exprv;
#if 1
    if (func->ast_type == AST_OPERATOR &&
        func->ast_flag == OPERATOR_GETATTR &&
      !(func->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) &&
        func->ast_operator.ast_opb &&
        func->ast_operator.ast_opb->ast_type == AST_CONSTEXPR &&
        DeeString_Check(func->ast_operator.ast_opb->ast_constexpr)) {
     int32_t cid;
     /* callattr with sequence argument. */
     if (ast_genasm(func->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
     cid = asm_newconst(func->ast_operator.ast_opb->ast_constexpr);
     if unlikely(cid < 0) goto err;
     for (i = 0; i < seq_argc; ++i) if (ast_genasm(seq_argv[i],ASM_G_FPUSHRES)) goto err;
     if (asm_gcallattr_const_map((uint16_t)cid,(uint8_t)(seq_argc / 2))) goto err;
     goto pop_unused;
    }
#endif
    /* Special case: Brace initializer-call with keys can be encoded as ASM_CALL_MAP. */
    if (ast_genasm(func,ASM_G_FPUSHRES)) goto err;
    for (i = 0; i < seq_argc; ++i) if (ast_genasm(seq_argv[i],ASM_G_FPUSHRES)) goto err;
    if (asm_gcall_map((uint8_t)(seq_argc / 2))) goto err;
    goto pop_unused;
   }
  }
  if (func->ast_type == AST_CONSTEXPR) {
   DeeObject *cxpr = func->ast_constexpr;
   /* Some casting-style expression have their own opcode. */
#if 0 /* The real constructor has a special case for strings... */
   if (cxpr == (DeeObject *)&DeeInt_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcast_int()) goto err;
    goto pop_unused;
   }
#endif
   if (cxpr == (DeeObject *)&DeeBool_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gbool(false)) goto err;
    goto pop_unused;
   }
   if (cxpr == (DeeObject *)&DeeString_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gstr()) goto err;
    goto pop_unused;
   }
   if (cxpr == (DeeObject *)&DeeTuple_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcast_tuple()) goto err;
    goto pop_unused;
   }
   if (cxpr == (DeeObject *)&DeeList_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcast_list()) goto err;
    goto pop_unused;
   }
   if (cxpr == (DeeObject *)&DeeDict_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcast_dict()) goto err;
    goto pop_unused;
   }
   if (cxpr == (DeeObject *)&DeeHashSet_Type) {
    if (ast_genasm(arg0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcast_hashset()) goto err;
    goto pop_unused;
   }
  }
 }

 if (func->ast_type == AST_SYM) {
  funsym = func->ast_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(funsym);
  if ((SYMBOL_TYPE(funsym) == SYMBOL_TYPE_EXTERN &&
     !(SYMBOL_EXTERN_SYMBOL(funsym)->ss_flags & MODSYM_FPROPERTY)) ||
      (funsym->s_type == SYMBOL_TYPE_GLOBAL ||
      (funsym->s_type == SYMBOL_TYPE_LOCAL &&
      !SYMBOL_MUST_REFERENCE_TYPEMAY(funsym)))) {
   for (i = 0; i < argc; ++i) if (ast_genasm(argv[i],ASM_G_FPUSHRES)) goto err;
   /* Direct call to symbol. */
   if (asm_putddi(ddi_ast)) goto err;
   if (SYMBOL_TYPE(funsym) == SYMBOL_TYPE_EXTERN) {
    if unlikely((symid = asm_esymid(funsym)) < 0) goto err;
    ASSERT(SYMBOL_EXTERN_SYMBOL(funsym));
    if (asm_gcall_extern((uint16_t)symid,SYMBOL_EXTERN_SYMBOL(funsym)->ss_index,argc)) goto err;
   } else if (funsym->s_type == SYMBOL_TYPE_GLOBAL) {
    if unlikely((symid = asm_gsymid_for_read(funsym,func)) < 0) goto err;
    if (asm_gcall_global((uint16_t)symid,argc)) goto err;
   } else {
    if unlikely((symid = asm_lsymid_for_read(funsym,func)) < 0) goto err;
    if (asm_gcall_local((uint16_t)symid,argc)) goto err;
   }
   goto pop_unused;
  }
 }
 if (func->ast_type == AST_OPERATOR &&
     func->ast_flag == OPERATOR_GETATTR &&
   !(func->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS))) {
  DeeAstObject *function_self = func->ast_operator.ast_opa;
  DeeAstObject *function_attr = func->ast_operator.ast_opb;
  if unlikely(!function_attr) goto generic_call;
  /* Call to an attribute with stack-based argument list. */
  if (function_attr->ast_type == AST_CONSTEXPR &&
      DeeString_Check(function_attr->ast_constexpr)) {
   int32_t attrid;
   /* (very) likely case: call to an attribute with a constant name. */
   if (function_self->ast_type == AST_SYM) {
    struct symbol *sym = function_self->ast_sym;
check_getattr_base_symbol_class:
    if (!SYMBOL_MUST_REFERENCE(sym)) {
     switch (SYMBOL_TYPE(sym)) {

     case SYMBOL_TYPE_ALIAS:
      ASSERT(SYMBOL_TYPE(SYMBOL_ALIAS(sym)) != SYMBOL_TYPE_ALIAS);
      sym = SYMBOL_ALIAS(sym);
      goto check_getattr_base_symbol_class;

     case SYMBOL_TYPE_THIS:
      /* call to the `this' argument. (aka. in-class member call) */
      for (i = 0; i < argc; ++i) if (ast_genasm(argv[i],ASM_G_FPUSHRES)) goto err;
      attrid = asm_newconst(function_attr->ast_constexpr);
      if unlikely(attrid < 0) goto err;
      if (asm_putddi(ddi_ast)) goto err;
      if (asm_gcallattr_this_const((uint16_t)attrid,argc)) goto err;
      goto pop_unused;
     {
      struct module_symbol *modsym; int32_t module_id;
     case SYMBOL_TYPE_MODULE: /* module.attr() --> call extern ... */
      modsym = get_module_symbol(SYMBOL_MODULE_MODULE(sym),
                                (DeeStringObject *)function_attr->ast_constexpr);
      if (!modsym) break;
      module_id = asm_msymid(sym);
      if unlikely(module_id < 0) goto err;
      /* Do a call to an external symbol. `ASM_CALL_EXTERN' */
      for (i = 0; i < argc; ++i) if (ast_genasm(argv[i],ASM_G_FPUSHRES)) goto err;
      if (asm_putddi(ddi_ast)) goto err;
      if (asm_gcall_extern((uint16_t)module_id,modsym->ss_index,argc)) goto err;
      goto pop_unused;
     } break;
     default: break;
     }
    }
   }
   /* call to some other object. */
   attrid = asm_newconst(function_attr->ast_constexpr);
   if unlikely(attrid < 0) goto err;
   if (ast_genasm(function_self,ASM_G_FPUSHRES)) goto err;
   for (i = 0; i < argc; ++i) if (ast_genasm(argv[i],ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gcallattr_const((uint16_t)attrid,argc)) goto err;
   goto pop_unused;
  }
  /* Pretty unlikely: The attribute name is not known.
   * Due to the runtime optimization impact that optimizing this still has,
   * there is also an opcode for this (callattr() is much faster than getattr+call). */
  if (ast_genasm(function_self,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(function_attr,ASM_G_FPUSHRES)) goto err;
  for (i = 0; i < argc; ++i) if (ast_genasm(argv[i],ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_gcallattr(argc)) goto err;
  goto pop_unused;
 }
 /* Call with stack-based argument list. */
 if (ast_genasm(func,ASM_G_FPUSHRES)) goto err;
 for (i = 0; i < argc; ++i) if (ast_genasm(argv[i],ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gcall(argc)) goto err;
pop_unused:
 if (!(gflags & ASM_G_FPUSHRES) && asm_gpop()) goto err;
 return 0;
generic_call:
 if (ast_genasm(func,ASM_G_FPUSHRES)) goto err;
 if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
 if (asm_gcall_tuple()) goto err;
 goto pop_unused;
err:
 return -1;
}


INTERN int DCALL
asm_gcall_kw_expr(DeeAstObject *__restrict func,
                  DeeAstObject *__restrict args,
                  DeeAstObject *__restrict kwds,
                  DeeAstObject *__restrict ddi_ast,
                  unsigned int gflags) {
 /* Optimizations for (highly likely) invocations, using the dedicated instruction. */
 if (func->ast_type == AST_OPERATOR &&
     func->ast_flag == OPERATOR_GETATTR &&
   !(func->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS))) {
  DeeAstObject *base,*name;
  base = func->ast_operator.ast_opa;
  name = func->ast_operator.ast_opb;
  if (kwds->ast_type == AST_CONSTEXPR &&
      name->ast_type == AST_CONSTEXPR &&
      DeeString_Check(name->ast_constexpr)) {
   int32_t kwd_cid,att_cid;
   kwd_cid = asm_newconst(kwds->ast_constexpr);
   if unlikely(kwd_cid < 0) goto err;
   att_cid = asm_newconst(name->ast_constexpr);
   if unlikely(att_cid < 0) goto err;
   if (args->ast_type == AST_MULTIPLE &&
       args->ast_flag != AST_FMULTIPLE_KEEPLAST &&
       args->ast_multiple.ast_exprc <= UINT8_MAX &&
      !ast_multiple_hasexpand(args)) {
    size_t i;
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
    for (i = 0; i < args->ast_multiple.ast_exprc; ++i) {
     if (ast_genasm(args->ast_multiple.ast_exprv[i],ASM_G_FPUSHRES))
         goto err;
    }
    if (ast_predict_type(args) != &DeeTuple_Type &&
        asm_gcast_tuple()) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcallattr_const_kw((uint16_t)att_cid,
                               (uint8_t)args->ast_multiple.ast_exprc,
                               (uint16_t)kwd_cid)) goto err;
    goto pop_unused;
   }
   if (args->ast_type == AST_CONSTEXPR &&
       args->ast_constexpr == Dee_EmptyTuple) {
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
    if (ast_predict_type(args) != &DeeTuple_Type &&
        asm_gcast_tuple()) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcallattr_const_kw((uint16_t)att_cid,0,(uint16_t)kwd_cid)) goto err;
    goto pop_unused;
   }
   if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
   if (ast_predict_type(args) != &DeeTuple_Type &&
       asm_gcast_tuple()) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gcallattr_const_tuple_kw((uint16_t)att_cid,(uint16_t)kwd_cid)) goto err;
   goto pop_unused;
  } else {
   if (args->ast_type == AST_MULTIPLE &&
       args->ast_flag != AST_FMULTIPLE_KEEPLAST &&
       args->ast_multiple.ast_exprc <= UINT8_MAX &&
      !ast_multiple_hasexpand(args)) {
    size_t i;
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(name,ASM_G_FPUSHRES)) goto err;
    for (i = 0; i < args->ast_multiple.ast_exprc; ++i) {
     if (ast_genasm(args->ast_multiple.ast_exprv[i],ASM_G_FPUSHRES))
         goto err;
    }
    if (ast_predict_type(args) != &DeeTuple_Type &&
        asm_gcast_tuple()) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (ast_genasm(kwds,ASM_G_FPUSHRES)) goto err;
    if (asm_gcallattr_kwds((uint8_t)args->ast_multiple.ast_exprc)) goto err;
    goto pop_unused;
   }
   if (args->ast_type == AST_CONSTEXPR &&
       args->ast_constexpr == Dee_EmptyTuple) {
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(name,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
    if (ast_predict_type(args) != &DeeTuple_Type &&
        asm_gcast_tuple()) goto err;
    if (ast_genasm(kwds,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gcallattr_kwds(0)) goto err;
    goto pop_unused;
   }
   if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(name,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
   if (ast_predict_type(args) != &DeeTuple_Type &&
       asm_gcast_tuple()) goto err;
   if (ast_genasm(kwds,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gcallattr_tuple_kwds()) goto err;
   goto pop_unused;
  }
 }
 /* The object being called isn't an attribute. */
 if (ast_genasm(func,ASM_G_FPUSHRES)) goto err;
 if (kwds->ast_type == AST_CONSTEXPR) {
  int32_t kwd_cid;
  kwd_cid = asm_newconst(kwds->ast_constexpr);
  if unlikely(kwd_cid < 0) goto err;
  if (args->ast_type == AST_MULTIPLE &&
      args->ast_flag != AST_FMULTIPLE_KEEPLAST &&
      args->ast_multiple.ast_exprc <= UINT8_MAX &&
     !ast_multiple_hasexpand(args)) {
   size_t i;
   for (i = 0; i < args->ast_multiple.ast_exprc; ++i) {
    if (ast_genasm(args->ast_multiple.ast_exprv[i],ASM_G_FPUSHRES))
        goto err;
   }
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gcall_kw((uint8_t)args->ast_multiple.ast_exprc,
                    (uint16_t)kwd_cid))
       goto err;
  } else if (args->ast_type == AST_CONSTEXPR &&
             args->ast_constexpr == Dee_EmptyTuple) {
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gcall_kw(0,(uint16_t)kwd_cid))
       goto err;
  } else {
   if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
   if (ast_predict_type(args) != &DeeTuple_Type &&
       asm_gcast_tuple()) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gcall_tuple_kw((uint16_t)kwd_cid)) goto err;
  }
 } else {
  /* Fallback: use the stack to pass all the arguments. */
  if (ast_genasm(args,ASM_G_FPUSHRES)) goto err;
  if (ast_predict_type(args) != &DeeTuple_Type &&
      asm_gcast_tuple()) goto err;
  if (ast_genasm(kwds,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_gcall_tuple_kwds()) goto err;
 }
pop_unused:
 if (!(gflags & ASM_G_FPUSHRES)) {
  if (asm_gpop()) goto err;
 }
 return 0;
err:
 return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENCALL_C */
