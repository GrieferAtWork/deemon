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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_TRY_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_TRY_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>

DECL_BEGIN

INTERN int (DCALL ast_optimize_try)(struct ast_optimize_stack *__restrict stack,
                                    struct ast *__restrict self, bool result_used) {
 struct catch_expr *iter,*end;
 ASSERT(self->a_type == AST_TRY);
 end = (iter = self->a_try.t_catchv)+self->a_try.t_catchc;
#ifdef OPTIMIZE_FASSUME
 if (optimizer_flags & OPTIMIZE_FASSUME) {
  /* Since at any point within the guarded expression, execution may jump
   * to one of the handlers, no additional assumptions can be gathered from
   * the guarded expression (though theoretically, we could gather everything
   * leading up to the first non-nothrow branch) */
  struct ast_assumes guard_assumes;
  struct ast_optimize_stack child_stack;
  if (ast_assumes_initcond(&guard_assumes,stack->os_assume))
      goto err;
  child_stack.os_assume = &guard_assumes;
  child_stack.os_ast    = self->a_try.t_guard;
  child_stack.os_prev   = stack;
  child_stack.os_used   = result_used;
  if (ast_optimize(&child_stack,child_stack.os_ast,result_used)) {
err_guard_assumes:
   ast_assumes_fini(&guard_assumes);
   goto err;
  }
  /* Since execution may jump out of the guard branch anywhere, it behaves as
   * though any part of the guard branch may not have gotten executed at all,
   * making it equivalent to an if-branch with one size being the guarded
   * expression, and the other side being an empty branch, technically
   * representing every part of the guarded expression leading up to the first
   * non-nothrow branch, however we don't track that kind of thing. */
  if (ast_assumes_mergecond(&guard_assumes,NULL))
      goto err_guard_assumes;
  /* Merge assumptions after the guard-branch with the main branch, resulting
   * in the common set of assumptions valid upon entry to any kind of handler,
   * as well as the caller (in case there are no finally branches). */
  if (ast_assumes_merge(stack->os_assume,&guard_assumes))
      goto err_guard_assumes;
  ast_assumes_fini(&guard_assumes);
  for (; iter != end; ++iter) {
   if (iter->ce_flags & EXCEPTION_HANDLER_FFINALLY) {
    /* Finally branches are always executed, meaning they
     * are run in-line with the guarded expression. */
    if (iter->ce_mask &&
        ast_optimize(stack,iter->ce_mask,false))
        goto err;
    if (ast_optimize(stack,iter->ce_code,result_used)) goto err;
   } else {
    /* Catch-branches are all optional, as well as staggered, meaning
     * that they look like this:
     * >> <guard>
     * >> if (...) {
     * >>     <catch_01>
     * >> }
     * >> if (...) {
     * >>     <catch_02>
     * >> }
     * >> if (...) {
     * >>     <catch_03>
     * >> }
     * NOTE: It must look like this, because `catch_01' can jump to
     *      `catch_02' when re-throwing an exception! */
    if (ast_assumes_initcond(&guard_assumes,stack->os_assume))
        goto err;
    child_stack.os_assume = &guard_assumes;
    child_stack.os_prev   = stack;
    if (iter->ce_mask) {
     child_stack.os_used = true;
     child_stack.os_ast  = iter->ce_mask;
     if (ast_optimize(&child_stack,child_stack.os_ast,true))
         goto err_guard_assumes;
    }
    child_stack.os_used = result_used;
    child_stack.os_ast  = iter->ce_code;
    if (ast_optimize(&child_stack,child_stack.os_ast,result_used))
        goto err_guard_assumes;
    /* Merge catch-assumptions with the an empty branch, indicating
     * that the catch expression is not necessarily executed. */
    if (ast_assumes_mergecond(&guard_assumes,NULL))
        goto err_guard_assumes;
    /* Merge catch-assumptions with those of the main branch. */
    if (ast_assumes_merge(stack->os_assume,&guard_assumes))
        goto err_guard_assumes;
    ast_assumes_fini(&guard_assumes);
   }
  }
 } else
#endif /* OPTIMIZE_FASSUME */
 {
  if (ast_optimize(stack,self->a_try.t_guard,result_used))
      goto err;
  for (; iter != end; ++iter) {
   if (iter->ce_mask &&
       ast_optimize(stack,iter->ce_mask,
                  !(iter->ce_flags & EXCEPTION_HANDLER_FFINALLY)))
       goto err;
   if (ast_optimize(stack,iter->ce_code,result_used)) goto err;
  }
 }
 iter = self->a_try.t_catchv;
 for (; iter != end; ++iter) {
  if (iter->ce_flags & EXCEPTION_HANDLER_FFINALLY)
      continue;
  /* `catch (object)' --> `catch (...)' */
  if (iter->ce_mask &&
      iter->ce_mask->a_type == AST_CONSTEXPR &&
      iter->ce_mask->a_constexpr == (DeeObject *)&DeeObject_Type) {
   OPTIMIZE_VERBOSE("Optimize object-mask to catch-all\n");
   ++optimizer_count;
   ast_decref(iter->ce_mask);
   iter->ce_mask = NULL;
  }
  /* Set the `CATCH_EXPR_FSECOND' flag for all noexcept catch-handlers. */
  if (!(iter->ce_mode & CATCH_EXPR_FSECOND) &&
        ast_is_nothrow(iter->ce_code,result_used) &&
     (!iter->ce_mask || ast_is_nothrow(iter->ce_mask,true))) {
   OPTIMIZE_VERBOSE("Optimize nothrow catch-handler\n");
   iter->ce_mode |= CATCH_EXPR_FSECOND;
   ++optimizer_count;
  }
 }
 if (ast_is_nothrow(self->a_try.t_guard,result_used)) {
  /* TODO: `try { foo; } finally { bar; }' -> `({ __stack local _r = foo; bar; _r; })'
   * TODO: `try { foo; } catch (...) { bar; }' -> `(foo)' */
 }
 return 0;
err:
 return -1;
/*
did_optimize:
 ++optimizer_count;
 return 0;
*/
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_TRY_C */
