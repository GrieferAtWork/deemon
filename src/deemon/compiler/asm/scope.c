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
#ifndef GUARD_DEEMON_COMPILER_ASM_SCOPE_C
#define GUARD_DEEMON_COMPILER_ASM_SCOPE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/compiler/assembler.h>
#include <deemon/module.h>

DECL_BEGIN

/* Go through all the symbols of `scope' and generate wrapper
 * code for storing symbols of read-only classes, that are still
 * being written to in local variable.
 * This includes symbols such as module references, or exception-variables. */
LOCAL int DCALL
transpose_modified_symbols(DeeScopeObject *__restrict scope) {
 size_t i;
 for (i = 0; i < scope->s_mapa; ++i) {
  struct symbol *sym = scope->s_map[i];
  for (; sym; sym = sym->sym_next) {
   if (!SYMBOL_NWRITE(sym))
        continue; /* Symbol is never actually being written to! */
check_symbol:
   switch (SYMBOL_TYPE(sym)) {

   case SYM_CLASS_ALIAS:
    ASSERT(SYMBOL_TYPE(SYMBOL_ALIAS(sym)) != SYM_CLASS_ALIAS);
    sym = SYMBOL_ALIAS(sym);
    goto check_symbol;


   {
    int32_t lid;
   case SYM_CLASS_MODULE:
   case SYM_CLASS_EXCEPT:
    /* Don't transpose these types of symbols.
     * Doing so would not reflect the user-expectation. */
   //case SYM_CLASS_REF:
   //case SYM_CLASS_THIS_MODULE:
   //case SYM_CLASS_THIS_FUNCTION:
   //case SYM_CLASS_THIS:
    lid = asm_newlocal();
    if unlikely(lid < 0) goto err;
#if 1
    if (asm_plocal((uint16_t)lid)) goto err;
    if (SYMBOL_TYPE(sym) == SYM_CLASS_MODULE) {
     int32_t mid = asm_msymid(sym);
     if unlikely(mid < 0) goto err;
     if (asm_gpush_module_p((uint16_t)mid)) goto err;
     Dee_Decref(SYMBOL_MODULE_MODULE(sym));
    } else {
     ASSERT(SYMBOL_TYPE(sym) == SYM_CLASS_EXCEPT);
     if (asm_gpush_except_p()) goto err;
    }
#else
    if (asm_gpush_symbol(sym,warn_ast)) goto err;
    if (asm_gpop_local((uint16_t)lid)) goto err;
#endif
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
    sym->s_type  = SYMBOL_TYPE_LOCAL;
    sym->s_flag |= SYMBOL_FALLOC;
    sym->s_symid = (uint16_t)lid;
#else
    SYMBOL_TYPE(sym) = SYM_CLASS_VAR;
    sym->sym_flag = SYM_FVAR_LOCAL | SYM_FVAR_ALLOC;
    sym->sym_var.sym_doc = NULL;
    sym->sym_var.sym_index = (uint16_t)lid;
#endif
   } break;

   default: break;
   }
  }
 }
 return 0;
err:
 return -1;
}


INTERN int DCALL
asm_enter_scope(DeeScopeObject *__restrict scope) {
 uint16_t num_stack_vars = 0;
 DeeScopeObject *old_scope;
 DeeScopeObject *new_scope;
 old_scope = current_assembler.a_scope;
 if (scope == old_scope)
     goto done;
 new_scope = scope;
 /* Make sure that the old scope can be reached from the new one. */
 if (!old_scope) goto set_new_scope;
 do new_scope = new_scope->s_prev;
 while (new_scope && new_scope != old_scope);
 if (new_scope) {
set_new_scope:
  current_assembler.a_scope = scope;
  while (scope && scope != old_scope) {
   if (!(current_assembler.a_flag&ASM_FSTACKDISP)) {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
    struct symbol **bucket_iter,**bucket_end,*iter;
    bucket_end = (bucket_iter = scope->s_map) + scope->s_mapa;
    for (; bucket_iter < bucket_end; ++bucket_iter)
    for (iter = *bucket_iter; iter; iter = iter->s_next)
    if (iter->s_type == SYMBOL_TYPE_STACK)
#else
    struct symbol *iter = scope->s_stk;
    for (; iter; iter = iter->sym_stack.sym_nstck)
#endif
    {
     /* Allocate memory for stack-variables when stack displacement is disabled. */
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
     if (iter->s_nwrite)
#else
     if (iter->sym_write)
#endif
     {
      /* Allocate this stack variable. */
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
      iter->s_flag |= SYMBOL_FALLOC;
#else
      iter->sym_flag |= SYM_FSTK_ALLOC;
#endif
      SYMBOL_STACK_OFFSET(iter)  = current_assembler.a_stackcur;
      SYMBOL_STACK_OFFSET(iter) += num_stack_vars;
      if (asm_putddi_sbind(SYMBOL_STACK_OFFSET(iter),
                           iter->sym_name))
          goto err;
      ++num_stack_vars;
     } else {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
      iter->s_flag &= ~SYMBOL_FALLOC;
#else
      iter->sym_flag &= ~SYM_FSTK_ALLOC;
#endif
     }
    }
   }
   if (transpose_modified_symbols(scope))
       goto err;
   scope = scope->s_prev;
  }
  if (num_stack_vars) {
   /* Adjust the stack accordingly. */
   if (asm_gadjstack((int16_t)num_stack_vars))
       goto err;
  }
 }
done:
 return 0;
err:
 return -1;
}
INTERN int DCALL
asm_leave_scope(DeeScopeObject *old_scope, uint16_t num_preserve
#ifndef NDEBUG
                , uint16_t old_stacksz
#endif
                ) {
 uint16_t num_stack_vars;
 DeeScopeObject *scope;
 scope = current_assembler.a_scope;
 if (old_scope == scope)
     goto done;
 num_stack_vars = 0;
 /* Search for local variables that have went out-of-scope. */
 if (current_assembler.a_flag & ASM_FREUSELOC) {
  do {
   size_t i;
   for (i = 0; i < scope->s_mapa; ++i) {
    struct symbol *iter = scope->s_map[i];
    for (; iter; iter = iter->sym_next) {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
     if (iter->s_type != SYMBOL_TYPE_LOCAL) continue;
     if (!(iter->s_flag & SYMBOL_FALLOC)) continue;
     ASSERT(!SYMBOL_MUST_REFERENCE_TYPEMAY(iter));
     asm_dellocal(iter->s_symid);
     iter->s_flag &= ~SYMBOL_FALLOC;
#else
     if (iter->sym_class != SYM_CLASS_VAR) continue;
     if (iter->sym_flag != (SYM_FVAR_ALLOC|SYM_FVAR_LOCAL)) continue;
     asm_dellocal(iter->sym_var.sym_index);
     iter->sym_flag &= ~SYM_FVAR_ALLOC;
#endif
    }
   }
   scope = scope->s_prev;
  } while (scope && scope != old_scope);
  scope = current_assembler.a_scope;
 }
 /* Pop all new stack variables initialized between this and the previous scope. */
 do {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
  struct symbol **bucket_iter,**bucket_end,*iter;
  bucket_end = (bucket_iter = scope->s_map) + scope->s_mapa;
  for (; bucket_iter < bucket_end; ++bucket_iter)
  for (iter = *bucket_iter; iter; iter = iter->s_next)
  if (iter->s_type == SYMBOL_TYPE_STACK)
#else
  struct symbol *iter = scope->s_stk;
  for (; iter; iter = iter->sym_stack.sym_nstck)
#endif
  {
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
   if (iter->s_flag & SYMBOL_FALLOC)
#else
   ASSERT(iter->sym_class == SYM_CLASS_STACK);
   if (iter->sym_flag & SYM_FSTK_ALLOC)
#endif
   {
    if (asm_putddi_sunbind(SYMBOL_STACK_OFFSET(iter)))
        goto err;
    ++num_stack_vars;
    iter->s_flag &= ~SYMBOL_FALLOC;
   }
  }
  scope = scope->s_prev;
 } while (scope && scope != old_scope);
#ifndef NDEBUG
 ASSERTF(current_assembler.a_stackcur >= num_stack_vars + num_preserve,
         "Stack is too small (Needed at least %I16u, but only have %I16u)\n"
         "old_stacksz    = %I16u\n"
         "num_stack_vars = %I16u\n"
         "num_preserve   = %I16u\n",
        (uint16_t)(num_stack_vars + num_preserve),
        (uint16_t)current_assembler.a_stackcur,
        (uint16_t)old_stacksz,
        (uint16_t)num_stack_vars,
        (uint16_t)num_preserve);
 ASSERTF(current_assembler.a_stackcur == old_stacksz + num_stack_vars + num_preserve,
         "Invalid stack depth when leaving scope (expected %I16u, but got %I16u)\n"
         "old_stacksz    = %I16u\n"
         "num_stack_vars = %I16u\n"
         "num_preserve   = %I16u\n",
        (uint16_t)(old_stacksz + num_stack_vars + num_preserve),
        (uint16_t)current_assembler.a_stackcur,
        (uint16_t)old_stacksz,
        (uint16_t)num_stack_vars,
        (uint16_t)num_preserve);
#else
 ASSERT(current_assembler.a_stackcur >= num_stack_vars + num_preserve);
#endif
 if (num_stack_vars) {
  /* Cleanup stack variables that were allocated in this scope. */
  if (!num_preserve) {
   if (asm_gadjstack(-(int16_t)num_stack_vars))
       goto err;
  } else if (num_preserve == 1) {
   /* Special case: Must preserve the result value! */
   /* Generate this code:
    * >> ...               // ..., a, b, c, d, result
    * >> pop      #4       // ..., result, b, c, d
    * >> adjstack #SP - 3  // ..., result
    */
   if (asm_gpop_n(num_stack_vars-1)) goto err;
   if (asm_gadjstack(-(int16_t)(num_stack_vars-1)))
       goto err;
  } else if (num_stack_vars >= num_preserve) {
   /* S0, S1, S2, R0, R1
    * >> pop #SP - 4  // S0, R1, S2, R0
    * >> pop #SP - 4  // R0, R1, S2
    */
   uint16_t diff = (num_stack_vars + 1) - 2;
   uint16_t i;
   for (i = 0; i < num_preserve; ++i) {
    if (asm_gpop_n(diff)) goto err;
   }
   /* R0, R1, S2
    * >> pop
    */
   diff = num_stack_vars - num_preserve;
   if (asm_gadjstack(-(int16_t)diff)) goto err;
  } else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
   /* S0, S1, R0, R1, R2
    * >> lrot #5  // S1, R0, R1, R2, S0
    * >> lrot #5  // R0, R1, R2, S1, S0
    */
   uint16_t i;
   uint16_t total = num_preserve + num_stack_vars;
   for (i = 0; i < num_stack_vars; ++i) {
    if (asm_glrot(total)) goto err;
   }
   /* R0, R1, R2, S1, S0
    * >> adjstack #SP - 2 // R0, R1, R2
    */
   if (asm_gadjstack(-(int16_t)num_stack_vars)) goto err;
  } else {
   /* S0, S1, R0, R1, R2
    * >> lrot #5  // S1, R0, R1, R2, S0
    * >> pop      // S1, R0, R1, R2
    * >> lrot #4  // R0, R1, R2, S1
    * >> pop      // S1, R0, R1
    */
   uint16_t i;
   uint16_t total = num_preserve + num_stack_vars;
   for (i = 0; i < num_stack_vars; ++i) {
    if (asm_glrot(total)) goto err;
    if (asm_gpop()) goto err;
    --total;
   }
  }
 }
 current_assembler.a_scope = old_scope;
done:
 return 0;
err:
 return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_SCOPE_C */
