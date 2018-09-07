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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENCLASS_C
#define GUARD_DEEMON_COMPILER_ASM_GENCLASS_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/class.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/assembler.h>

DECL_BEGIN

INTERN int
(DCALL asm_genclass)(struct ast *__restrict class_ast,
                     unsigned int gflags) {
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
 size_t i;
 ASSERT(class_ast->a_type == AST_CLASS);
 if (class_ast->a_class.c_base) {
  if (class_ast->a_class.c_base->a_type == AST_SYM) {
   struct symbol *base = class_ast->a_class.c_base->a_sym;
   SYMBOL_INPLACE_UNWIND_ALIAS(base);
   /* TODO: Optimizations for global/extern base types. */
   //if (base->s_type == SYMBOL_TYPE_GLOBAL) {
   //} else if (base->s_type == SYMBOL_TYPE_EXTERN) {
   //}
  }
  if (ast_genasm(class_ast->a_class.c_base,ASM_G_FPUSHRES))
      goto err;
 } else {
  if (asm_putddi(class_ast)) goto err;
  if (asm_gpush_none())
      goto err;
 }
 if (class_ast->a_class.c_supersym &&
   (!class_ast->a_class.c_base ||
     class_ast->a_class.c_base->a_type != AST_SYM ||
     class_ast->a_class.c_base->a_sym != class_ast->a_class.c_supersym)) {
  /* Save the super-class in its symbol. */
  if (asm_can_prefix_symbol(class_ast->a_class.c_supersym)) {
   /* mov <c_supersym>, top */
   if (asm_gprefix_symbol(class_ast->a_class.c_supersym,class_ast))
       goto err;
   if (asm_gdup_p()) goto err;
  } else {
   if (asm_gdup()) goto err;
   if (asm_gpop_symbol(class_ast->a_class.c_supersym,class_ast))
       goto err;
  }
 }
 if (class_ast->a_class.c_desc->a_type == AST_CONSTEXPR &&
     DeeClassDescriptor_Check(class_ast->a_class.c_desc->a_constexpr)) {
  int32_t cid = asm_newconst(class_ast->a_class.c_desc->a_constexpr);
  if unlikely(cid < 0) goto err;
  if (asm_gclass_c((uint16_t)cid))
      goto err;
 } else {
  if (ast_genasm(class_ast->a_class.c_desc,ASM_G_FPUSHRES))
      goto err;
  if (asm_putddi(class_ast)) goto err;
  if (asm_gclass())
      goto err;
 }
 /* At this point, the new class type has already been created.
  * Now to store it in its own symbol, before moving on to
  * initialize all of the members saved within the class
  * member table. */
 if (class_ast->a_class.c_classsym) {
  if (asm_can_prefix_symbol(class_ast->a_class.c_classsym)) {
   /* mov <c_supersym>, top */
   if (asm_gprefix_symbol(class_ast->a_class.c_classsym,class_ast))
       goto err;
   if (asm_gdup_p()) goto err;
  } else {
   if (asm_gdup()) goto err;
   if (asm_gpop_symbol(class_ast->a_class.c_classsym,class_ast))
       goto err;
  }
 }
 /* Now move on to initialize all of the members. */
 for (i = 0; i < class_ast->a_class.c_memberc; ++i) {
  struct class_member *member;
  member = &class_ast->a_class.c_memberv[i];
  if (ast_genasm(member->cm_ast,ASM_G_FPUSHRES))
      goto err;
  if (asm_putddi(class_ast)) goto err;
  if (asm_gdefmember(member->cm_index)) goto err;
 }
 /* And that's already it! - The new class is complete. */
 if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
     goto err;
 return 0;
err:
 return -1;
#else
 uint8_t genflags,opcount;
 uint16_t class_addr;
 struct class_member *iter,*end;
 struct symbol *super_sym;
 /* Figure out the flags accompanying the opcode. */
 genflags = class_ast->a_flag & 0xf,opcount = 0;
 if (class_ast->a_class.c_base) genflags |= CLASSGEN_FHASBASE,++opcount;
 if (class_ast->a_class.c_name) genflags |= CLASSGEN_FHASNAME,++opcount;
 if (class_ast->a_class.c_imem) genflags |= CLASSGEN_FHASIMEM,++opcount;
 if (class_ast->a_class.c_cmem) genflags |= CLASSGEN_FHASCMEM,++opcount;
 super_sym = class_ast->a_class.c_supersym;
 if (super_sym) {
  SYMBOL_INPLACE_UNWIND_ALIAS(super_sym);
  if (!super_sym->s_nread)
       super_sym = NULL;
 }

 /* Now to actually create the class. */
 if ((!class_ast->a_class.c_name ||
      (class_ast->a_class.c_name->a_type == AST_CONSTEXPR &&
       DeeString_CheckExact(class_ast->a_class.c_name->a_constexpr))) &&
     (!class_ast->a_class.c_imem ||
      (class_ast->a_class.c_imem->a_type == AST_CONSTEXPR &&
       DeeMemberTable_CheckExact(class_ast->a_class.c_imem->a_constexpr))) &&
     (!class_ast->a_class.c_cmem ||
      (class_ast->a_class.c_cmem->a_type == AST_CONSTEXPR &&
       DeeMemberTable_CheckExact(class_ast->a_class.c_cmem->a_constexpr)))) {
  /* Special optimizations using operator-optimized instructions. */
  int32_t name_cid,imem_cid,cmem_cid; instruction_t *text;
  struct ast *base = class_ast->a_class.c_base;
  /* Allocate the constant for operands.
   * NOTE: Even when we can't optimize the base expression, the assembly generators
   *       used when the operands are pushed individually will automatically 
   *       re-use indices that we've already allocated here.
   */
  if unlikely((name_cid = (class_ast->a_class.c_name ?
                           asm_newconst(class_ast->a_class.c_name->a_constexpr) :
                           0)) < 0 ||
              (imem_cid = (class_ast->a_class.c_imem ?
                           asm_newconst(class_ast->a_class.c_imem->a_constexpr) :
                           0)) < 0 ||
              (cmem_cid = (class_ast->a_class.c_cmem ?
                           asm_newconst(class_ast->a_class.c_cmem->a_constexpr) :
                           0)) < 0)
      goto err;
  /* The optimized class instructions can only be used
   * when all constant operands have an index that can
   * fit into the 8-bit range. */
  if ((uint16_t)name_cid > UINT8_MAX) goto class_fallback;
  if ((uint16_t)imem_cid > UINT8_MAX) goto class_fallback;
  if ((uint16_t)cmem_cid > UINT8_MAX) goto class_fallback;
  if (!base) {
   /* Without a base type defined, encode the operation using `ASM_CLASS_C' */
   if unlikely((text = asm_alloc(6)) == NULL) goto err;
   *           (text + 0) = ASM_CLASS_C;
   *(uint8_t *)(text + 1) = genflags;
   *(uint8_t *)(text + 2) = 0;
   *(uint8_t *)(text + 3) = (uint8_t)name_cid;
   *(uint8_t *)(text + 4) = (uint8_t)imem_cid;
   *(uint8_t *)(text + 5) = (uint8_t)cmem_cid;
   goto got_class_incsp;
  }
  if (base->a_type == AST_CONSTEXPR &&
      asm_allowconst(base->a_constexpr)) {
   int32_t base_cid;
   base_cid = asm_newconst(base->a_constexpr);
   if unlikely(base_cid < 0) goto err;
   if (base_cid > UINT8_MAX) goto class_fallback;
   if (super_sym) {
    /* Make sure to save the base expression in the super-symbol. */
    if (asm_gpush_const8((uint8_t)base_cid)) goto err;
    if (asm_gpop_symbol(super_sym,class_ast)) goto err;
   }
   /* Encode a constant expression base-type. */
   if unlikely((text = asm_alloc(6)) == NULL) goto err;
   *           (text + 0) = ASM_CLASS_C;
   *(uint8_t *)(text + 1) = genflags;
   *(uint8_t *)(text + 2) = (uint8_t)base_cid;
   *(uint8_t *)(text + 3) = (uint8_t)name_cid;
   *(uint8_t *)(text + 4) = (uint8_t)imem_cid;
   *(uint8_t *)(text + 5) = (uint8_t)cmem_cid;
   goto got_class_incsp;
  }
  if (base->a_type == AST_SYM) {
   struct symbol *base_sym = base->a_sym;
   SYMBOL_INPLACE_UNWIND_ALIAS(base_sym);
   if (base_sym->s_type == SYMBOL_TYPE_GLOBAL ||
      (base_sym->s_type == SYMBOL_TYPE_LOCAL &&
      !SYMBOL_MUST_REFERENCE_TYPEMAY(base_sym))) {
    int32_t varid; bool is_global;
    is_global = base_sym->s_type == SYMBOL_TYPE_GLOBAL;
    varid = is_global ? asm_gsymid_for_read(base_sym,base)
                      : asm_lsymid_for_read(base_sym,base);
    if unlikely(varid < 0) goto err;
    if (varid > UINT8_MAX) goto class_fallback;
    if (super_sym) {
     /* Make sure to save the base expression in the super-symbol. */
     if (is_global ? asm_gpush_global8((uint8_t)varid)
                   : asm_gpush_local8((uint8_t)varid)) goto err;
     if (asm_gpop_symbol(super_sym,class_ast)) goto err;
    }
    /* Encode a constant expression base-type. */
    if unlikely((text = asm_alloc(6)) == NULL) goto err;
    *           (text + 0) = is_global ? ASM_CLASS_CBG : ASM_CLASS_CBL;
    *(uint8_t *)(text + 1) = genflags;
    *(uint8_t *)(text + 2) = (uint8_t)varid;
    *(uint8_t *)(text + 3) = (uint8_t)name_cid;
    *(uint8_t *)(text + 4) = (uint8_t)imem_cid;
    *(uint8_t *)(text + 5) = (uint8_t)cmem_cid;
    goto got_class_incsp;
   }
  }
 }
class_fallback:
 if (class_ast->a_class.c_base) {
  if (ast_genasm(class_ast->a_class.c_base,ASM_G_FPUSHRES))
      goto err;
  if (super_sym) {
   /* Write the class base to the super-symbol. */
   SYMBOL_INPLACE_UNWIND_ALIAS(super_sym);
   if (super_sym->s_type == SYMBOL_TYPE_STACK &&
     !(super_sym->s_flag & SYMBOL_FALLOC) &&
      !SYMBOL_MUST_REFERENCE_TYPEMAY(super_sym)) {
    super_sym->s_symid = current_assembler.a_stackcur-1;
    super_sym->s_flag |= SYMBOL_FALLOC;
    if (asm_gdup()) goto err;
   } else {
    if (asm_gdup()) goto err;
    if (asm_gpop_symbol(super_sym,class_ast)) goto err;
   }
  }
 }
 if (class_ast->a_class.c_name && ast_genasm(class_ast->a_class.c_name,ASM_G_FPUSHRES)) goto err;
 if (class_ast->a_class.c_imem && ast_genasm(class_ast->a_class.c_imem,ASM_G_FPUSHRES)) goto err;
 if (class_ast->a_class.c_cmem && ast_genasm(class_ast->a_class.c_cmem,ASM_G_FPUSHRES)) goto err;

 /* Create a regular, old class. */
 if (asm_putddi(class_ast)) goto err;
 if (asm_putimm8(ASM_CLASS,genflags)) goto err;
 asm_subsp(opcount);
got_class_incsp:
 asm_incsp();

 if (class_ast->a_class.c_classsym) {
  /* Write the class itself to the class-symbol. */
  if (asm_gdup()) goto err;
  if (asm_gpop_symbol(class_ast->a_class.c_classsym,class_ast)) goto err;
 }
 class_addr = current_assembler.a_stackcur-1;
 /* Go through all class member descriptors and generate their code. */
 end = (iter = class_ast->a_class.c_memberv)+
               class_ast->a_class.c_memberc;
 for (; iter != end; ++iter) {
  PRIVATE instruction_t member_modes[2] = {
      /* [CLASS_MEMBER_MEMBER]   = */ASM_DEFMEMBER,
      /* [CLASS_MEMBER_OPERATOR] = */ASM_DEFOP
  };
  uint16_t member_id = iter->cm_index;
  instruction_t def_instr;
  ASSERT(iter->cm_type < COMPILER_LENOF(member_modes));
  def_instr = member_modes[iter->cm_type];
  /* Compile the expression (In the event of it using the class
   * type, it will be able to access it as a stack variable). */
  if (ast_genasm(iter->cm_ast,ASM_G_FPUSHRES))
      goto err;
  if (asm_putddi(class_ast)) goto err;
  if (class_addr == current_assembler.a_stackcur-2) {
   /* Simple (and most likely) case:
    * >> ..., class, member-value */
   if (member_id > UINT8_MAX) {
    if (asm_put(ASM_EXTENDED1)) goto err;
    if (asm_putimm16(def_instr,member_id)) goto err;
   } else {
    if (asm_putimm8(def_instr,(uint8_t)member_id)) goto err;
   }
   asm_ddicsp(); /* All member-define instructions behave as `-2,+1' */
  } else {
   uint16_t displacement;
   ASSERT(current_assembler.a_flag&ASM_FSTACKDISP);
   ASSERT(class_addr < current_assembler.a_stackcur-2);
   /* Complicated case (can happen in displacement-mode):
    * >> ..., class, ..., member-value
    * Here, we must generate code to do this:
    * >> dup  #sizeof(1+...)   // ..., class, ..., member-value, class
    * >> swap                  // ..., class, ..., class, member-value
    * >> defop / defmember     // ..., class, ..., class
    * >> pop                   // ..., class, ...
    */
   displacement = (current_assembler.a_stackcur-2)-class_addr;
   if (asm_gdup_n(displacement)) goto err;
   if (asm_gswap()) goto err;
   if (member_id > UINT8_MAX) {
    if (asm_put(ASM_EXTENDED1)) goto err;
    if (asm_putimm16(def_instr,member_id)) goto err;
   } else {
    if (asm_putimm8(def_instr,(uint8_t)member_id)) goto err;
   }
   asm_ddicsp(); /* All member-define instructions behave as `-2,+1' */
   if (asm_gpop()) goto err;
  }
 }
 if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
     goto err;
 return 0;
err:
 return -1;
#endif
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENCLASS_C */
