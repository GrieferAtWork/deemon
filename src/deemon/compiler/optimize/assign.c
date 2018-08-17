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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_ASSIGN_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_ASSIGN_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN

INTDEF void DCALL ast_incwrite(DeeAstObject *__restrict self);
INTDEF void DCALL ast_decwrite(DeeAstObject *__restrict self);

INTERN int DCALL
ast_assign(DeeAstObject *__restrict self,
           DeeAstObject *__restrict other) {
 uint8_t buffer[(sizeof(DeeAstObject)-
                 COMPILER_OFFSETOF(DeeAstObject,ast_type))];
 /* Use a temporary buffer for the variable portion of the AST.
  * Until we actually assign `other' to `self', we initialize it as a shallow copy of `other'. */
#if defined(__INTELLISENSE__) || 1
 DeeAstObject *temp = COMPILER_CONTAINER_OF(buffer,DeeAstObject,ast_type);
#else
#define temp   COMPILER_CONTAINER_OF(buffer,DeeAstObject,ast_type)
#endif
 temp->ast_type = other->ast_type;
 temp->ast_flag = other->ast_flag;
 switch (other->ast_type) {
 case AST_LOOP:
  if (other->ast_flag&AST_FLOOP_FOREACH &&
      other->ast_loop.ast_elem) {
   /* For an explanation, see AST_ACTION:AST_FACTION_STORE */
   ast_incwrite(other->ast_loop.ast_elem);
  }
  temp->ast_loop.ast_cond = other->ast_loop.ast_cond;
  temp->ast_loop.ast_iter = other->ast_loop.ast_iter;
  temp->ast_loop.ast_loop = other->ast_loop.ast_loop;
  Dee_XIncref(other->ast_loop.ast_cond);
  Dee_XIncref(other->ast_loop.ast_iter);
  Dee_XIncref(other->ast_loop.ast_loop);
  break;

 case AST_OPERATOR:
  temp->ast_operator.ast_exflag = other->ast_operator.ast_exflag;
  goto do_subast_x4;

 {
  struct class_member *iter,*end,*dst;
 case AST_CLASS:
  end = (iter = other->ast_class.ast_memberv)+
                other->ast_class.ast_memberc;
  temp->ast_class.ast_memberc = other->ast_class.ast_memberc;
  dst = (struct class_member *)Dee_Malloc(temp->ast_class.ast_memberc*
                                          sizeof(struct class_member));
  if unlikely(!dst) goto err;
  temp->ast_class.ast_memberv = dst;
  /* Copy the member descriptor table. */
  for (; iter != end; ++iter,++dst) {
   *dst = *iter;
   Dee_Incref(dst->cm_ast);
  }
 }
do_subast_x4:
  temp->ast_operator.ast_opd = other->ast_operator.ast_opd;
  Dee_XIncref(temp->ast_operator.ast_opd);
  ATTR_FALLTHROUGH
 case AST_CONDITIONAL:
do_xcopy_3:
  temp->ast_operator.ast_opc = other->ast_operator.ast_opc;
  Dee_XIncref(temp->ast_operator.ast_opc);
  ATTR_FALLTHROUGH
 case AST_FUNCTION:
  temp->ast_operator.ast_opb = other->ast_operator.ast_opb;
  Dee_XIncref(temp->ast_operator.ast_opb);
  ATTR_FALLTHROUGH
 case AST_CONSTEXPR:
 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
 case AST_BOOL:
 case AST_EXPAND:
  temp->ast_constexpr = other->ast_constexpr;
  Dee_XIncref(temp->ast_constexpr);
  break;

 case AST_ACTION:
  /* NOTE: Must fix the symbol effect counters, but ignore any reference underflow
   *       that will be fixed as soon as the given branch is destroyed.
   *       This _MUST_ always happen unless the caller uses this function
   *       improperly in order to duplicate a branch without the intent of
   *       eventually destroying the source branch. */
  if ((other->ast_flag&AST_FACTION_KINDMASK) ==
      (AST_FACTION_STORE&AST_FACTION_KINDMASK)) {
   ast_incwrite(other->ast_action.ast_act0);
  }
  goto do_xcopy_3;

 {
  DREF DeeAstObject **iter,**end,**dst;
 case AST_MULTIPLE:
  temp->ast_multiple.ast_exprc = other->ast_multiple.ast_exprc;
  end = (iter = other->ast_multiple.ast_exprv)+
                other->ast_multiple.ast_exprc;
  dst = (DREF DeeAstObject **)Dee_Malloc(temp->ast_multiple.ast_exprc*
                                         sizeof(DREF DeeAstObject *));
  if unlikely(!dst) goto err;
  temp->ast_multiple.ast_exprv = dst;
  for (; iter != end; ++iter,++dst) {
   *dst = *iter;
   Dee_Incref(*dst);
  }
 } break;

 {
  struct catch_expr *iter,*end,*dst;
 case AST_TRY:
  temp->ast_try.ast_guard = other->ast_try.ast_guard;
  temp->ast_try.ast_catchc = other->ast_try.ast_catchc;
  Dee_Incref(temp->ast_try.ast_guard);
  end = (iter = other->ast_try.ast_catchv)+
                other->ast_try.ast_catchc;
  dst = (struct catch_expr *)Dee_Malloc(temp->ast_try.ast_catchc*
                                        sizeof(struct catch_expr));
  if unlikely(!dst) goto err;
  temp->ast_try.ast_catchv = dst;
  for (; iter != end; ++iter,++dst) {
   *dst = *iter;
   Dee_XIncref(dst->ce_mask);
   Dee_Incref(dst->ce_code);
  }
 } break;

 case AST_SYM:
  ASSERT(other->ast_sym);
  temp->ast_sym = other->ast_sym;
  if (temp->ast_flag) {
   ASSERT(SYMBOL_NWRITE(other->ast_sym));
   SYMBOL_INC_NWRITE(temp->ast_sym);
  } else {
   ASSERT(SYMBOL_NREAD(other->ast_sym));
   SYMBOL_INC_NREAD(temp->ast_sym);
  }
  break;

 case AST_BNDSYM:
  ASSERT(other->ast_sym);
  temp->ast_sym = other->ast_sym;
  ASSERT(SYMBOL_NBOUND(other->ast_sym));
  SYMBOL_INC_NBOUND(temp->ast_sym);
  break;

 case AST_GOTO:
  ASSERT(other->ast_goto.ast_label);
  ASSERT(other->ast_goto.ast_label->tl_goto);
  temp->ast_goto.ast_label = other->ast_goto.ast_label;
  ++other->ast_goto.ast_label->tl_goto;
  temp->ast_goto.ast_base = other->ast_goto.ast_base;
  Dee_Incref((DeeObject *)temp->ast_goto.ast_base);
  break;
 case AST_LABEL:
  ASSERT(other->ast_label.ast_label);
  temp->ast_label.ast_label = other->ast_label.ast_label;
  temp->ast_label.ast_base  = other->ast_label.ast_base;
  Dee_Incref((DeeObject *)temp->ast_label.ast_base);
  break;

// case AST_SWITCH: /* Use the default case... */
//  break;

 default:
  /* XXX: Couldn't we must always do a move-construction like this? */
  memcpy(buffer,&other->ast_type,sizeof(buffer));
  memset(&other->ast_type,0,sizeof(buffer));
  break;
 }
 if (self != other) {
  /* Copy over DDI information. */
  if (other->ast_ddi.l_file)
      TPPFile_Incref(other->ast_ddi.l_file);
  if (self->ast_ddi.l_file)
      TPPFile_Decref(self->ast_ddi.l_file);
  memcpy(&self->ast_ddi,&other->ast_ddi,sizeof(struct ast_loc));
 }
 /* Finalize the contents of the AST that's being assigned to. */
 ast_fini_contents(self);
 /* Copy our temporary buffer into the actual, new AST. */
 memcpy(&self->ast_type,buffer,sizeof(buffer));
 return 0;
err:
 return -1;
#undef temp
}
INTERN int DCALL
ast_graft_onto(DeeAstObject *__restrict self,
               DeeAstObject *__restrict other) {
 DREF DeeAstObject **elemv;
 if (self->ast_scope == other->ast_scope)
     return ast_assign(self,other);
 elemv = (DeeAstObject **)Dee_Malloc(1*sizeof(DREF DeeAstObject *));
 if unlikely(!elemv) return -1;
 elemv[0] = other;
 Dee_Incref(other);
 if (self != other) {
  /* Copy over DDI information. */
  if (other->ast_ddi.l_file)
      TPPFile_Incref(other->ast_ddi.l_file);
  if (self->ast_ddi.l_file)
      TPPFile_Decref(self->ast_ddi.l_file);
  memcpy(&self->ast_ddi,&other->ast_ddi,sizeof(struct ast_loc));
 }
 ast_fini_contents(self);
 /* Override the (currently) invalid ast `self'. */
 self->ast_type               = AST_MULTIPLE;
 self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
 self->ast_multiple.ast_exprc = 1;
 self->ast_multiple.ast_exprv = elemv;
 return 0;
}

INTERN DeeAstObject *DCALL
ast_setscope_and_ddi(DeeAstObject *ast,
                     DeeAstObject *__restrict src) {
 if unlikely(!ast) return NULL;
 Dee_Incref(src->ast_scope);
 Dee_Decref(ast->ast_scope);
 /* Override the effective scope of the AST. */
 ast->ast_scope = src->ast_scope;
 /* Override the DDI information. */
 if (src->ast_ddi.l_file) TPPFile_Incref(src->ast_ddi.l_file);
 if (ast->ast_ddi.l_file) TPPFile_Decref(ast->ast_ddi.l_file);
 ast->ast_ddi = src->ast_ddi;
 return ast;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_ASSIGN_C */
