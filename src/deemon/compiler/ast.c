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
#ifndef GUARD_DEEMON_COMPILER_AST_C
#define GUARD_DEEMON_COMPILER_AST_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/none.h>
#include <deemon/tuple.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/tpp.h>
#include <deemon/util/cache.h>
#include <deemon/compiler/symbol.h>

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

DEFINE_OBJECT_CACHE(ast,DeeAstObject,512)
DECLARE_STRUCT_CACHE(sym,struct symbol)

#ifndef NDEBUG
#define sym_alloc()  sym_dbgalloc(__FILE__,__LINE__)
#endif

/* Re-use the symbol cache for labels. (As rare as they are, this is the best way to allocate them) */
#define lbl_alloc()  ((struct text_label *)sym_alloc())
#define lbl_free(p)      sym_free((struct symbol *)(p))


#ifdef CONFIG_TRACE_REFCHANGES
#define INIT_REF(x) (Dee_Incref(x),Dee_DecrefNokill(x))
#else
#define INIT_REF(x) (void)0
#endif

#ifndef NDEBUG
#ifdef CONFIG_NO_AST_DEBUG
#define ast_new()    ast_dbgnew(__FILE__,__LINE__)
#else
#define ast_new()    ast_dbgnew(file,line)
#endif
PRIVATE DREF DeeAstObject *DCALL
ast_dbgnew(char const *file, int line) {
 DREF DeeAstObject *result = ast_dbgalloc(file,line);
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
#endif
 if likely(result) {
  DeeObject_Init(result,&DeeAst_Type);
  result->ast_scope = current_scope;
  result->ast_ddi.l_file = NULL;
  Dee_Incref(result->ast_scope);
 }
 return result;
}
#else
PRIVATE DREF DeeAstObject *DCALL ast_new(void) {
 DREF DeeAstObject *result = ast_alloc();
#ifndef CONFIG_NO_THREADS
 ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
#endif
 if likely(result) {
  DeeObject_Init(result,&DeeAst_Type);
  result->ast_scope = current_scope;
  result->ast_ddi.l_file = NULL;
  Dee_Incref(result->ast_scope);
 }
 return result;
}
#endif

INTERN void FCALL
loc_here(struct ast_loc *__restrict info) {
 ASSERT(info);
 info->l_file = TPPLexer_Global.l_token.t_file;
 /* Query line/column information for the current token's start position. */
 TPPFile_LCAt(info->l_file,&info->l_lc,
              TPPLexer_Global.l_token.t_begin);
}
INTERN DeeAstObject *FCALL
ast_setddi(DeeAstObject *ast,
           struct ast_loc *__restrict info) {
 if unlikely(!ast) goto done; /* Special case: Ignore `NULL' for `ast'. */
 ASSERT_OBJECT_TYPE(ast,&DeeAst_Type);
 ASSERT(info);
 if unlikely(ast->ast_ddi.l_file)
    TPPFile_Decref(ast->ast_ddi.l_file);
 if (info->l_file == TPPLexer_Global.l_token.t_file) {
  ast->ast_ddi = *info;
 } else {
  loc_here(&ast->ast_ddi);
 }
 ASSERT(ast->ast_ddi.l_file);
 /* Keep a reference to the associated file (so we can later read its filename). */
 TPPFile_Incref(ast->ast_ddi.l_file);
done:
 return ast;
}
INTERN DeeAstObject *FCALL
ast_sethere(DeeAstObject *ast) {
 if unlikely(!ast) goto done; /* Special case: Ignore `NULL' for `ast'. */
 ASSERT_OBJECT_TYPE(ast,&DeeAst_Type);
 if unlikely(ast->ast_ddi.l_file)
    TPPFile_Decref(ast->ast_ddi.l_file);
 loc_here(&ast->ast_ddi);
 ASSERT(ast->ast_ddi.l_file);
 /* Keep a reference to the associated file (so we can later read its filename). */
 TPPFile_Incref(ast->ast_ddi.l_file);
done:
 return ast;
}
INTERN DeeAstObject *FCALL
ast_putddi(DeeAstObject *ast,
           struct ast_loc *__restrict info) {
 if unlikely(!ast) goto done; /* Special case: Ignore `NULL' for `ast'. */
 ASSERT_OBJECT_TYPE(ast,&DeeAst_Type);
 ASSERT(info);
 if unlikely(ast->ast_ddi.l_file) goto done;
 if (info->l_file == TPPLexer_Global.l_token.t_file) {
  ast->ast_ddi = *info;
 } else {
  loc_here(&ast->ast_ddi);
 }
 ASSERT(ast->ast_ddi.l_file);
 /* Keep a reference to the associated file (so we can later read its filename). */
 TPPFile_Incref(ast->ast_ddi.l_file);
done:
 return ast;
}
INTERN DeeAstObject *FCALL
ast_puthere(DeeAstObject *ast) {
 if unlikely(!ast) goto done; /* Special case: Ignore `NULL' for `ast'. */
 ASSERT_OBJECT_TYPE(ast,&DeeAst_Type);
 if unlikely(ast->ast_ddi.l_file) goto done;
 loc_here(&ast->ast_ddi);
 ASSERT(ast->ast_ddi.l_file);
 /* Keep a reference to the associated file (so we can later read its filename). */
 TPPFile_Incref(ast->ast_ddi.l_file);
done:
 return ast;
}

INTERN void DCALL
ast_incwrite(DeeAstObject *__restrict self) {
 switch (self->ast_type) {
 case AST_SYM:
  ASSERT(self->ast_sym);
  if (self->ast_flag++ == 0) {
   SYMBOL_DEC_NREAD(self->ast_sym);
   SYMBOL_INC_NWRITE(self->ast_sym);
  }
  break;
 {
  DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  /* Multiple write targets (incref each individually) */
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter) ast_incwrite(*iter);
 } break;
 default: break;
 }
}

INTERN void DCALL
ast_incwriteonly(DeeAstObject *__restrict self) {
 switch (self->ast_type) {
 case AST_SYM:
  ASSERT(self->ast_sym);
  if (self->ast_flag++ == 0)
      SYMBOL_INC_NWRITE(self->ast_sym);
  break;
 {
  DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  /* Multiple write targets (incref each individually) */
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter) ast_incwriteonly(*iter);
 } break;
 default: break;
 }
}

INTERN void DCALL
ast_decwrite(DeeAstObject *__restrict self) {
 switch (self->ast_type) {
 case AST_SYM:
  ASSERT(self->ast_sym);
  ASSERT(self->ast_flag);
  if (!--self->ast_flag) {
   SYMBOL_DEC_NWRITE(self->ast_sym);
   SYMBOL_INC_NREAD(self->ast_sym);
  }
  break;
 {
  DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  /* Multiple write targets (decref each individually) */
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter) ast_decwrite(*iter);
 } break;
 default: break;
 }
}

INTERN void DCALL
ast_decwriteonly(DeeAstObject *__restrict self) {
 switch (self->ast_type) {
 case AST_SYM:
  ASSERT(self->ast_sym);
  ASSERT(self->ast_flag);
  if (!--self->ast_flag)
       SYMBOL_DEC_NWRITE(self->ast_sym);
  break;
 {
  DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  /* Multiple write targets (decref each individually) */
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter) ast_decwriteonly(*iter);
 } break;
 default: break;
 }
}

#ifdef CONFIG_NO_AST_DEBUG
#define DEFINE_AST_GENERATOR(name,args) \
   INTERN DREF DeeAstObject *(DCALL name) args
#else
#define DEFINE_AST_GENERATOR(name,args) \
   INTERN DREF DeeAstObject *(DCALL name##_d) PRIVATE_AST_GENERATOR_UNPACK_ARGS args
#endif


DEFINE_AST_GENERATOR(ast_constexpr,
                    (DeeObject *__restrict constant_expression)) {
 DREF DeeAstObject *result = ast_new();
 ASSERT_OBJECT(constant_expression);
 ASSERT(!DeeObject_InstanceOf(constant_expression,&DeeAst_Type));
 if likely(result) {
  result->ast_type      = AST_CONSTEXPR;
  result->ast_constexpr = constant_expression;
  Dee_Incref(constant_expression);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_sym,
                    (struct symbol *__restrict sym)) {
 DREF DeeAstObject *result = ast_new();
 ASSERT(sym);
 if likely(result) {
  result->ast_type = AST_SYM;
  result->ast_flag = 0; /* Start out as a read-reference. */
  result->ast_sym  = sym;
  SYMBOL_INC_NREAD(sym);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_unbind,
                    (struct symbol *__restrict sym)) {
 DREF DeeAstObject *result = ast_new();
 ASSERT(sym);
 if likely(result) {
  result->ast_type   = AST_UNBIND;
  result->ast_unbind = sym;
  SYMBOL_INC_NWRITE(sym);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_bndsym,(struct symbol *__restrict sym)) {
 DREF DeeAstObject *result = ast_new();
 ASSERT(sym);
 if likely(result) {
  result->ast_type   = AST_BNDSYM;
  result->ast_bndsym = sym;
  SYMBOL_INC_NBOUND(sym);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_operator_func,
                    (uint16_t operator_name, DeeAstObject *binding)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE_OPT(binding,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type = AST_OPERATOR_FUNC;
  result->ast_flag = operator_name;
  result->ast_operator_func.ast_binding = binding;
  Dee_XIncref(binding);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_multiple,(uint16_t flags, size_t exprc,
                        /*inherit*/DREF DeeAstObject **__restrict exprv)) {
 DREF DeeAstObject *result;
 /* Prevent AST ambiguity by not allowing
  * keep-last, single-element multiple ASTs.
  * -> ... Because they're literally no-ops that would otherwise
  *        confuse the optimizer into not detecting constant
  *        expressions, as well as special behavior surrounding
  *       `AST_EXPAND' expressions not being triggered. */
 if unlikely(exprc == 1 && flags == AST_FMULTIPLE_KEEPLAST &&
             current_scope == exprv[0]->ast_scope) {
  result = exprv[0]; /* Inherit reference. */
got_result:
  Dee_Free(exprv);   /* We're supposed to ~inherit~ this on success. (So we simply discard it). */
  return result;
 }
 /* Prevent some more ambiguity when ZERO(0) expressions were passed. */
 if unlikely(exprc == 0) {
  if (flags == AST_FMULTIPLE_KEEPLAST) {
   result = ast_constexpr(Dee_None);
got_result_maybe:
   if unlikely(!result) return NULL;
   goto got_result;
  }
  if (flags == AST_FMULTIPLE_TUPLE ||
      flags == AST_FMULTIPLE_GENERIC) {
   result = ast_constexpr(Dee_EmptyTuple);
   goto got_result_maybe;
  }
 }
 if likely((result = ast_new()) != NULL) {
  result->ast_type               = AST_MULTIPLE;
  result->ast_flag               = flags;
  result->ast_multiple.ast_exprc = exprc;
  result->ast_multiple.ast_exprv = exprv;
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_return,
                    (DeeAstObject *return_expr)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE_OPT(return_expr,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type       = AST_RETURN;
  result->ast_returnexpr = return_expr;
  Dee_XIncref(return_expr);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_yield,
                    (DeeAstObject *__restrict yield_expr)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(yield_expr,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type       = AST_YIELD;
  result->ast_returnexpr = yield_expr;
  Dee_Incref(yield_expr);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_throw,
                    (DeeAstObject *throw_expr)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE_OPT(throw_expr,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type       = AST_THROW;
  result->ast_returnexpr = throw_expr;
  Dee_XIncref(throw_expr);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_try,(DeeAstObject *__restrict guarded_expression, size_t catchc,
                   /*inherit*/struct catch_expr *__restrict catchv)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(guarded_expression,&DeeAst_Type);
 if unlikely(!catchc) {
  /* Prevent ambiguity when no handlers are defined. */
  Dee_Free(catchv); /* May still be a non-NULL buffer. */
  Dee_Incref(guarded_expression);
  return guarded_expression;
 }
 if likely((result = ast_new()) != NULL) {
  result->ast_type           = AST_TRY;
  result->ast_try.ast_guard  = guarded_expression;
  result->ast_try.ast_catchc = catchc;
  result->ast_try.ast_catchv = catchv;
  Dee_Incref(guarded_expression);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_tryfinally,
                    (DeeAstObject *__restrict guarded_expression,
                     DeeAstObject *__restrict finally_expression)) {
 struct catch_expr *catchv; DREF DeeAstObject *result;
 /* Allocate the catch-expression vector inherited by `ast_try' upon success. */
 catchv = (struct catch_expr *)Dee_Malloc(1*sizeof(struct catch_expr));
 if unlikely(!catchv) return NULL;
 catchv[0].ce_code  = finally_expression; /* Reference is incremented later. */
 catchv[0].ce_flags = EXCEPTION_HANDLER_FFINALLY; /* Regular, old finally-handler. */
 catchv[0].ce_mask  = NULL; /* Regular finally-handlers don't have masks. */
#ifdef CONFIG_NO_AST_DEBUG
 result = ast_try(guarded_expression,1,catchv);
#else
 result = ast_try_d(file,line,guarded_expression,1,catchv);
#endif
 if unlikely(!result) Dee_Free(catchv);
 else Dee_Incref(finally_expression);
 return result;
}

DEFINE_AST_GENERATOR(ast_loop,
                    (uint16_t flags,
                     DeeAstObject *elem_or_cond,
                     DeeAstObject *iter_or_next,
                     DeeAstObject *loop)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE_OPT(iter_or_next,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(loop,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  /* Apply the unlikely-branch tag to loop expressions.
   * When set, the loop block is usually placed in cold text. */
#if AST_FCOND_UNLIKELY == AST_FLOOP_UNLIKELY
  flags |= (current_tags.at_expect&AST_FCOND_UNLIKELY);
#else
  if (current_tags.at_expect&AST_FCOND_UNLIKELY)
      flags |= AST_FLOOP_UNLIKELY;
#endif
  result->ast_type           = AST_LOOP;
  result->ast_flag           = flags;
  result->ast_loop.ast_cond  = elem_or_cond;
  result->ast_loop.ast_next  = iter_or_next;
  result->ast_loop.ast_loop  = loop;
  Dee_XIncref(elem_or_cond);
  Dee_XIncref(iter_or_next);
  Dee_XIncref(loop);
  if (elem_or_cond && (flags&AST_FLOOP_FOREACH))
      ast_incwrite(elem_or_cond);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_loopctl,(uint16_t flags)) {
 DREF DeeAstObject *result;
 if likely((result = ast_new()) != NULL) {
  result->ast_type = AST_LOOPCTL;
  result->ast_flag = flags;
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_conditional,
                    (uint16_t flags,
                     DeeAstObject *__restrict cond,
                     DeeAstObject *tt_expr,
                     DeeAstObject *ff_expr)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(cond,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(tt_expr,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(ff_expr,&DeeAst_Type);
 ASSERTF(tt_expr || ff_expr,"At least one must be present");
 ASSERTF(tt_expr != ff_expr,"These can't be the same");
 if likely((result = ast_new()) != NULL) {
  result->ast_type                 = AST_CONDITIONAL;
  result->ast_flag                 = flags;
  result->ast_conditional.ast_cond = cond;
  result->ast_conditional.ast_tt   = tt_expr;
  result->ast_conditional.ast_ff   = ff_expr;
  Dee_Incref(cond);
  Dee_XIncref(tt_expr);
  Dee_XIncref(ff_expr);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_bool,
                    (uint16_t flags, DeeAstObject *__restrict expr)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(expr,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type     = AST_BOOL;
  result->ast_flag     = flags;
  result->ast_boolexpr = expr;
  Dee_Incref(expr);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_expand,
                    (DeeAstObject *__restrict expr)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(expr,&DeeAst_Type);
 /* To prevent ambiguity, always expand single-element,
  * sequence-multi-expressions without going through an AST_EXPAND. */
 if (expr->ast_type == AST_MULTIPLE &&
     expr->ast_flag != AST_FMULTIPLE_KEEPLAST &&
     expr->ast_multiple.ast_exprc == 1) {
  result = expr->ast_multiple.ast_exprv[0];
  Dee_Incref(result);
  return result;
 }
 if likely((result = ast_new()) != NULL) {
  result->ast_type       = AST_EXPAND;
  result->ast_expandexpr = expr;
  Dee_Incref(expr);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_function,
                    (DeeAstObject *__restrict function_code,
                     DeeBaseScopeObject *__restrict scope)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(function_code,&DeeAst_Type);
 ASSERT_OBJECT_TYPE((DeeObject *)scope,&DeeBaseScope_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type = AST_FUNCTION;
  result->ast_function.ast_code  = function_code;
  result->ast_function.ast_scope = scope;
  Dee_Incref(function_code);
  Dee_Incref((DeeObject *)scope);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_operator1,
                    (uint16_t operator_name, uint16_t flags,
                     DeeAstObject *__restrict opa)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(opa,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  if (OPERATOR_ISINPLACE(operator_name))
      ast_incwriteonly(opa);
  result->ast_type                = AST_OPERATOR;
  result->ast_flag                = operator_name;
  result->ast_operator.ast_opa    = opa;
  result->ast_operator.ast_opb    = NULL;
  result->ast_operator.ast_opc    = NULL;
  result->ast_operator.ast_opd    = NULL;
  result->ast_operator.ast_exflag = flags;
  Dee_Incref(opa);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_operator2,
                    (uint16_t operator_name, uint16_t flags,
                     DeeAstObject *__restrict opa,
                     DeeAstObject *__restrict opb)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(opa,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(opb,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  if (OPERATOR_ISINPLACE(operator_name))
      ast_incwriteonly(opa);
  result->ast_type                = AST_OPERATOR;
  result->ast_flag                = operator_name;
  result->ast_operator.ast_opa    = opa;
  result->ast_operator.ast_opb    = opb;
  result->ast_operator.ast_opc    = NULL;
  result->ast_operator.ast_opd    = NULL;
  result->ast_operator.ast_exflag = flags;
  Dee_Incref(opa);
  Dee_Incref(opb);
  INIT_REF(result);
 }
 return result;
}
DEFINE_AST_GENERATOR(ast_operator3,
                    (uint16_t operator_name, uint16_t flags,
                     DeeAstObject *__restrict opa,
                     DeeAstObject *__restrict opb,
                     DeeAstObject *__restrict opc)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(opa,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(opb,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(opc,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  if (OPERATOR_ISINPLACE(operator_name))
      ast_incwriteonly(opa);
  result->ast_type                = AST_OPERATOR;
  result->ast_flag                = operator_name;
  result->ast_operator.ast_opa    = opa;
  result->ast_operator.ast_opb    = opb;
  result->ast_operator.ast_opc    = opc;
  result->ast_operator.ast_opd    = NULL;
  result->ast_operator.ast_exflag = flags;
  Dee_Incref(opa);
  Dee_Incref(opb);
  Dee_Incref(opc);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_operator4,
                    (uint16_t operator_name, uint16_t flags,
                     DeeAstObject *__restrict opa,
                     DeeAstObject *__restrict opb,
                     DeeAstObject *__restrict opc,
                     DeeAstObject *__restrict opd)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE(opa,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(opb,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(opc,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(opd,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  if (OPERATOR_ISINPLACE(operator_name))
      ast_incwriteonly(opa);
  result->ast_type                = AST_OPERATOR;
  result->ast_flag                = operator_name;
  result->ast_operator.ast_opa    = opa;
  result->ast_operator.ast_opb    = opb;
  result->ast_operator.ast_opc    = opc;
  result->ast_operator.ast_opd    = opd;
  result->ast_operator.ast_exflag = flags;
  Dee_Incref(opa);
  Dee_Incref(opb);
  Dee_Incref(opc);
  Dee_Incref(opd);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_action0,
                    (uint16_t action_flags)) {
 DREF DeeAstObject *result;
 ASSERT(AST_FACTION_ARGC_GT(action_flags) == 0);
 if likely((result = ast_new()) != NULL) {
  result->ast_type            = AST_ACTION;
  result->ast_flag            = action_flags;
  result->ast_action.ast_act0 = NULL;
  result->ast_action.ast_act1 = NULL;
  result->ast_action.ast_act2 = NULL;
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_action1,
                    (uint16_t action_flags,
                     DeeAstObject *__restrict act0)) {
 DREF DeeAstObject *result;
 ASSERT(AST_FACTION_ARGC_GT(action_flags) == 1);
 ASSERT_OBJECT_TYPE(act0,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type            = AST_ACTION;
  result->ast_flag            = action_flags;
  result->ast_action.ast_act0 = act0;
  result->ast_action.ast_act1 = NULL;
  result->ast_action.ast_act2 = NULL;
  Dee_Incref(act0);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_action2,
                    (uint16_t action_flags,
                     DeeAstObject *__restrict act0,
                     DeeAstObject *__restrict act1)) {
 DREF DeeAstObject *result;
 ASSERT(AST_FACTION_ARGC_GT(action_flags) == 2);
 ASSERT_OBJECT_TYPE(act0,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(act1,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type            = AST_ACTION;
  result->ast_flag            = action_flags;
  result->ast_action.ast_act0 = act0;
  result->ast_action.ast_act1 = act1;
  result->ast_action.ast_act2 = NULL;
  Dee_Incref(act0);
  Dee_Incref(act1);
  if ((action_flags&AST_FACTION_KINDMASK) ==
      (AST_FACTION_STORE&AST_FACTION_KINDMASK))
       ast_incwrite(act0);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_action3,
                    (uint16_t action_flags,
                     DeeAstObject *__restrict act0,
                     DeeAstObject *__restrict act1,
                     DeeAstObject *__restrict act2)) {
 DREF DeeAstObject *result;
 ASSERT(AST_FACTION_ARGC_GT(action_flags) == 3);
 ASSERT_OBJECT_TYPE(act0,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(act1,&DeeAst_Type);
 ASSERT_OBJECT_TYPE(act2,&DeeAst_Type);
 if likely((result = ast_new()) != NULL) {
  result->ast_type            = AST_ACTION;
  result->ast_flag            = action_flags;
  result->ast_action.ast_act0 = act0;
  result->ast_action.ast_act1 = act1;
  result->ast_action.ast_act2 = act2;
  Dee_Incref(act0);
  Dee_Incref(act1);
  Dee_Incref(act2);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_class,
                    (uint16_t class_flags, DeeAstObject *base, DeeAstObject *name,
                     DeeAstObject *doc, DeeAstObject *imem, DeeAstObject *cmem,
                     struct symbol *class_symbol, struct symbol *super_symbol,
                     size_t memberc, struct class_member *__restrict memberv,
                     size_t anonc, struct member_entry *anonv)) {
 DREF DeeAstObject *result;
 ASSERT_OBJECT_TYPE_OPT(base,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(name,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(doc,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(imem,&DeeAst_Type);
 ASSERT_OBJECT_TYPE_OPT(cmem,&DeeAst_Type);
 ASSERT(!memberc || memberv);
 ASSERT(!anonc || anonv);
 if likely((result = ast_new()) != NULL) {
  result->ast_type               = AST_CLASS;
  result->ast_flag               = class_flags;
  result->ast_class.ast_base     = base;
  result->ast_class.ast_name     = name;
  result->ast_class.ast_doc      = doc;
  result->ast_class.ast_imem     = imem;
  result->ast_class.ast_cmem     = cmem;
  result->ast_class.ast_memberc  = memberc;
  result->ast_class.ast_memberv  = memberv;
  result->ast_class.ast_classsym = class_symbol;
  result->ast_class.ast_supersym = super_symbol;
  result->ast_class.ast_anonc    = anonc;
  result->ast_class.ast_anonv    = anonv;
  if (class_symbol) SYMBOL_INC_NWRITE(class_symbol);
  if (super_symbol) SYMBOL_INC_NWRITE(super_symbol);
  Dee_XIncref(base);
  Dee_XIncref(name);
  Dee_XIncref(doc);
  Dee_XIncref(imem);
  Dee_XIncref(cmem);
  INIT_REF(result);
 }
 return result;
}

DEFINE_AST_GENERATOR(ast_label,
                    (uint16_t flags, struct text_label *__restrict lbl,
                     DeeBaseScopeObject *__restrict base_scope)) {
 DREF DeeAstObject *result;
 ASSERT(lbl);
 if likely((result = ast_new()) != NULL) {
  result->ast_type            = AST_LABEL;
  result->ast_flag            = flags;
  result->ast_label.ast_label = lbl;
  result->ast_label.ast_base  = base_scope;
  Dee_Incref((DeeObject *)base_scope);
  INIT_REF(result);
 }
 return result;
}
DEFINE_AST_GENERATOR(ast_goto,
                    (struct text_label *__restrict lbl,
                     DeeBaseScopeObject *__restrict base_scope)) {
 DREF DeeAstObject *result;
 ASSERT(lbl);
 if likely((result = ast_new()) != NULL) {
  result->ast_type           = AST_GOTO;
  result->ast_flag           = AST_FNORMAL;
  result->ast_goto.ast_label = lbl;
  result->ast_goto.ast_base  = base_scope;
  Dee_Incref((DeeObject *)base_scope);
  /* Trace the number of uses of this label. */
  ++lbl->tl_goto;
  INIT_REF(result);
 }
 return result;
}
DEFINE_AST_GENERATOR(ast_switch,
                    (uint16_t flags,
                     DeeAstObject *__restrict expr,
                     DeeAstObject *__restrict block,
                     struct text_label *cases,
                     struct text_label *default_case)) {
 DREF DeeAstObject *result;
 if likely((result = ast_new()) != NULL) {
  result->ast_type               = AST_SWITCH;
  result->ast_flag               = flags;
  result->ast_switch.ast_expr    = expr;
  result->ast_switch.ast_block   = block;
  result->ast_switch.ast_cases   = cases;
  result->ast_switch.ast_default = default_case;
  Dee_Incref(expr);
  Dee_Incref(block);
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
DEFINE_AST_GENERATOR(ast_assembly,
                    (uint16_t flags, struct TPPString *__restrict text,
                     size_t num_o, size_t num_i, size_t num_l,
                   /*inherit*/struct asm_operand *__restrict opv))
#else /* !CONFIG_LANGUAGE_NO_ASM */
DEFINE_AST_GENERATOR(ast_assembly,
                    (uint16_t flags,
                     size_t num_o, size_t num_i, size_t num_l,
                   /*inherit*/struct asm_operand *__restrict opv))
#endif /* CONFIG_LANGUAGE_NO_ASM */
{
 DREF DeeAstObject *result;
#ifndef CONFIG_LANGUAGE_NO_ASM
 ASSERT(text);
#endif
 ASSERT(!(num_o+num_i+num_l) || opv);
 if likely((result = ast_new()) != NULL) {
  size_t i;
  /* Track the writes to output operands. */
  for (i = 0; i < num_o; ++i) {
   if (ASM_OPERAND_IS_INOUT(&opv[i])) {
    ast_incwriteonly(opv[i].ao_expr);
   } else {
    ast_incwrite(opv[i].ao_expr);
   }
  }
  result->ast_type = AST_ASSEMBLY;
  result->ast_flag = flags;
#ifndef CONFIG_LANGUAGE_NO_ASM
  result->ast_assembly.ast_text.at_text = text;
  TPPString_Incref(text);
#endif
  result->ast_assembly.ast_num_o = num_o;
  result->ast_assembly.ast_num_i = num_i;
  result->ast_assembly.ast_num_l = num_l;
  result->ast_assembly.ast_opc   = num_o+num_i+num_l;
  result->ast_assembly.ast_opv   = opv;
  INIT_REF(result);
 }
 return result;
}
#undef DEFINE_AST_GENERATOR

INTERN bool DCALL
ast_multiple_hasexpand(DeeAstObject *__restrict self) {
 DeeAstObject **iter,**end;
 ASSERT_OBJECT_TYPE(self,&DeeAst_Type);
 ASSERT(self->ast_type == AST_MULTIPLE);
 end = (iter = self->ast_multiple.ast_exprv)+
               self->ast_multiple.ast_exprc;
 for (; iter != end; ++iter) {
  if ((*iter)->ast_type == AST_EXPAND)
        return true;
 }
 return false;
}


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
  ASSERT_OBJECT_TYPE(switch_cases->tl_expr,&DeeAst_Type);
  Dee_Decref(switch_cases->tl_expr);
  lbl_free(switch_cases);
  switch_cases = next;
 }
}
INTERN void DCALL
visit_switch_cases(struct text_label *switch_cases,
                   dvisit_t proc, void *arg) {
 while (switch_cases) {
  ASSERT_OBJECT_TYPE(switch_cases->tl_expr,&DeeAst_Type);
  Dee_Visit(switch_cases->tl_expr);
  switch_cases = switch_cases->tl_next;
 }
}

INTERN void DCALL
ast_fini_contents(DeeAstObject *__restrict self) {
 switch (self->ast_type) {

 case AST_LOOP:
  if (self->ast_flag&AST_FLOOP_FOREACH &&
      self->ast_loop.ast_elem) {
   ast_decwrite(self->ast_loop.ast_elem);
  }
  goto do_xdecref_3;
 {
  struct class_member *iter,*end;
 case AST_CLASS:
  end = (iter = self->ast_class.ast_memberv)+
                self->ast_class.ast_memberc;
  /* Cleanup the member descriptor table. */
  for (; iter != end; ++iter)
      Dee_Decref(iter->cm_ast);
  Dee_Free(self->ast_class.ast_memberv);
  Dee_Free(self->ast_class.ast_anonv);
  Dee_XDecref(self->ast_class.ast_cmem);
  if (self->ast_class.ast_classsym)
      SYMBOL_DEC_NWRITE(self->ast_class.ast_classsym);
  if (self->ast_class.ast_supersym)
      SYMBOL_DEC_NWRITE(self->ast_class.ast_supersym);
 }
  ATTR_FALLTHROUGH
 case AST_OPERATOR:
  if (OPERATOR_ISINPLACE(self->ast_flag))
      ast_decwriteonly(self->ast_operator.ast_opa);
  Dee_XDecref(self->ast_operator.ast_opd);
  ATTR_FALLTHROUGH
 case AST_CONDITIONAL:
do_xdecref_3:
  Dee_XDecref(self->ast_operator.ast_opc);
  ATTR_FALLTHROUGH
 case AST_FUNCTION:
  Dee_XDecref(self->ast_operator.ast_opb);
  ATTR_FALLTHROUGH
 case AST_CONSTEXPR:
 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
 case AST_BOOL:
 case AST_EXPAND:
 case AST_OPERATOR_FUNC:
  Dee_XDecref(self->ast_constexpr);
  break;

 case AST_ACTION:
  if ((self->ast_flag&AST_FACTION_KINDMASK) ==
      (AST_FACTION_STORE&AST_FACTION_KINDMASK)) {
   ast_decwrite(self->ast_action.ast_act0);
  }
  goto do_xdecref_3;

 {
  DREF DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter)
         Dee_Decref(*iter);
  Dee_Free(self->ast_multiple.ast_exprv);
 } break;

 {
  struct catch_expr *iter,*end;
 case AST_TRY:
  Dee_Decref(self->ast_try.ast_guard);
  end = (iter = self->ast_try.ast_catchv)+
                self->ast_try.ast_catchc;
  for (; iter != end; ++iter) {
   Dee_XDecref(iter->ce_mask);
   Dee_Decref(iter->ce_code);
  }
  Dee_Free(self->ast_try.ast_catchv);
 } break;

 case AST_SYM:
  ASSERTF(!self->ast_flag,"At least some write-wrappers havn't been unwound.");
  SYMBOL_DEC_NREAD(self->ast_sym);
  break;
 case AST_UNBIND:
  SYMBOL_DEC_NWRITE(self->ast_unbind);
  break;
 case AST_BNDSYM:
  SYMBOL_DEC_NBOUND(self->ast_bndsym);
  break;

 case AST_GOTO:
  ASSERT(self->ast_goto.ast_label);
  ASSERT(self->ast_goto.ast_label->tl_goto);
  --self->ast_goto.ast_label->tl_goto;
  ATTR_FALLTHROUGH
 case AST_LABEL:
  Dee_Decref((DeeObject *)self->ast_label.ast_base);
  break;

 case AST_SWITCH:
  cleanup_switch_cases(self->ast_switch.ast_cases,
                       self->ast_switch.ast_default);
  Dee_Decref(self->ast_switch.ast_expr);
  Dee_Decref(self->ast_switch.ast_block);
  break;

 {
  struct asm_operand *iter,*end;
  size_t i;
 case AST_ASSEMBLY:
  end = (iter = self->ast_assembly.ast_opv)+
               (self->ast_assembly.ast_num_o+
                self->ast_assembly.ast_num_i);
  /* Track the writes to output operands. */
  for (i = 0; i < self->ast_assembly.ast_num_o; ++i) {
   struct asm_operand *op = &self->ast_assembly.ast_opv[i];
   if (ASM_OPERAND_IS_INOUT(op)) {
    ast_decwriteonly(op->ao_expr);
   } else {
    ast_decwrite(op->ao_expr);
   }
  }
  for (; iter != end; ++iter) {
   ASSERT(iter->ao_type);
   ASSERT(iter->ao_expr);
   TPPString_Decref(iter->ao_type);
   Dee_Decref(iter->ao_expr);
  }
  end += self->ast_assembly.ast_num_l;
  for (; iter != end; ++iter) {
   ASSERT(!iter->ao_type);
   ASSERT(iter->ao_label);
   ASSERT(iter->ao_label->tl_goto);
   --iter->ao_label->tl_goto;
  }
  Dee_Free(self->ast_assembly.ast_opv);
#ifndef CONFIG_LANGUAGE_NO_ASM
  TPPString_Decref(self->ast_assembly.ast_text.at_text);
#endif
 } break;

 //case AST_LOOPCTL:
 default: break;
 }
}

PRIVATE void DCALL
ast_fini(DeeAstObject *__restrict self) {
 if (self->ast_ddi.l_file)
     TPPFile_Decref(self->ast_ddi.l_file);
 ast_fini_contents(self);
 /* NOTE: Must destroy the scope _AFTER_ the AST contents, in case the
  *       ast itself references some symbol that is owned by this scope. */
 Dee_Decref(self->ast_scope);
}


PUBLIC DeeTypeObject DeeAst_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"ast",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                TYPE_ALLOCATOR(ast_tp_alloc,ast_tp_free)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ast_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_C */
