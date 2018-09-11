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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_MULTIPLE_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_MULTIPLE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/none.h>
#include <deemon/util/string.h>
#include <deemon/tuple.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/dict.h>
#include <deemon/rodict.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN

INTERN int (DCALL ast_optimize_multiple)(struct ast_optimize_stack *__restrict stack,
                                         struct ast *__restrict self, bool result_used) {
 struct ast **iter,**end;
 bool is_unreachable,only_constexpr;
 ASSERT(self->a_type == AST_MULTIPLE);
 if (!result_used &&
      self->a_flag != AST_FMULTIPLE_KEEPLAST) {
  self->a_flag = AST_FMULTIPLE_KEEPLAST;
  OPTIMIZE_VERBOSE("Replace unused sequence expression with keep-last multi-branch");
  ++optimizer_count;
 }

 /* Optimize all asts within.
  * In the event of a keep-last AST, delete a branches
  * without side-effects except for the last. */
 end = (iter = self->a_multiple.m_astv)+
               self->a_multiple.m_astc;
 is_unreachable = false;
 only_constexpr = true;
multiple_continue_at_iter:
 for (; iter != end; ++iter) {
  bool using_value = result_used; int temp;
  if (self->a_flag == AST_FMULTIPLE_KEEPLAST && iter != end-1)
      using_value = false;
  if (ast_optimize(stack,*iter,using_value)) goto err;
  /* TODO: Optimize something like this:
   *       `[10,20,[30,40]...]' --> [10,20,30,40]'
   * NOTE: This shouldn't even be exclusive to constant expressions!
   *       Any time an ast_expand(ast_multiple()) is encounted
   *       here, we can insert all the elements from the
   *       underlying AST here!
   */
  if ((*iter)->a_type != AST_CONSTEXPR) only_constexpr = false;
  temp = ast_doesnt_return(*iter,AST_DOESNT_RETURN_FNORMAL);
  if (temp < 0) is_unreachable = temp == -2;
  else if (is_unreachable ||
           /* Delete branches that are unreachable or have no side-effects. */
          (self->a_flag == AST_FMULTIPLE_KEEPLAST &&
          (iter != end-1 || !result_used) && !ast_has_sideeffects(*iter))) {
   /* Get rid of this one. */
   OPTIMIZE_VERBOSEAT(*iter,is_unreachable ? "Delete unreachable AST\n"
                                           : "Delete AST without side-effects\n");
   ast_decref(*iter);
   --end;
   --self->a_multiple.m_astc;
   MEMMOVE_PTR(iter,iter+1,(size_t)(end-iter));
   ++optimizer_count;
   goto multiple_continue_at_iter;
  } else if (temp > 0) {
   /* If the AST doesn't return, everything that follows is no-return. */
   is_unreachable = true;
  }
 }
 if (only_constexpr) {
  if (self->a_multiple.m_astc == 1 &&
     !self->a_ddi.l_file) {
   memcpy(&self->a_ddi,
          &self->a_multiple.m_astv[0]->a_ddi,
          sizeof(struct ast_loc));
   if (self->a_ddi.l_file)
       TPPFile_Incref(self->a_ddi.l_file);
  }
  if (self->a_multiple.m_astc == 0) {
   DREF DeeObject *cexpr;
   if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
    cexpr = Dee_None;
    Dee_Incref(cexpr);
   } else if (self->a_flag == AST_FMULTIPLE_TUPLE ||
              self->a_flag == AST_FMULTIPLE_GENERIC) {
    cexpr = Dee_EmptyTuple;
    Dee_Incref(cexpr);
   } else if (self->a_flag == AST_FMULTIPLE_LIST) {
    cexpr = DeeList_New();
    if unlikely(!cexpr) goto err;
   } else if (self->a_flag == AST_FMULTIPLE_SET) {
    cexpr = DeeHashSet_New();
    if unlikely(!cexpr) goto err;
   } else if (self->a_flag == AST_FMULTIPLE_DICT) {
    cexpr = DeeDict_New();
    if unlikely(!cexpr) goto err;
   } else if (self->a_flag == AST_FMULTIPLE_GENERIC_KEYS) {
    cexpr = DeeRoDict_New();
    if unlikely(!cexpr) goto err;
   } else {
    goto after_multiple_constexpr;
   }
   Dee_Free(self->a_multiple.m_astv);
   self->a_constexpr = cexpr; /* Inherit reference */
  } else if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
   if unlikely(ast_graft_onto(self,self->a_multiple.m_astv[
                                   self->a_multiple.m_astc-1]))
      goto err;
  } else if (self->a_flag == AST_FMULTIPLE_TUPLE ||
             self->a_flag == AST_FMULTIPLE_GENERIC) {
   DREF DeeObject *new_tuple; size_t i;
   new_tuple = DeeTuple_NewUninitialized(self->a_multiple.m_astc);
   if unlikely(!new_tuple) goto err;
   for (i = 0; i < self->a_multiple.m_astc; ++i) {
    struct ast *branch = self->a_multiple.m_astv[i];
    DeeObject *value = branch->a_constexpr;
    DeeTuple_SET(new_tuple,i,value);
    Dee_Incref(value);
    ast_decref(branch);
   }
   Dee_Free(self->a_multiple.m_astv);
   self->a_constexpr = new_tuple; /* Inherit reference. */
  } else if (self->a_flag == AST_FMULTIPLE_LIST) {
   DREF DeeObject *new_list; size_t i;
   new_list = DeeList_NewUninitialized(self->a_multiple.m_astc);
   if unlikely(!new_list) goto err;
   for (i = 0; i < self->a_multiple.m_astc; ++i) {
    struct ast *branch = self->a_multiple.m_astv[i];
    DeeObject *value = branch->a_constexpr;
    DeeList_SET(new_list,i,value);
    Dee_Incref(value);
    ast_decref(branch);
   }
   DeeGC_Track((DeeObject *)new_list);
   Dee_Free(self->a_multiple.m_astv);
   self->a_constexpr = new_list; /* Inherit reference. */
  } else if (self->a_flag == AST_FMULTIPLE_SET) {
   DREF DeeObject *new_set; size_t i;
   new_set = DeeHashSet_New();
   if unlikely(!new_set) goto err;
   for (i = 0; i < self->a_multiple.m_astc; ++i) {
    if (DeeHashSet_Insert(new_set,self->a_multiple.m_astv[i]->a_constexpr) < 0) {
     Dee_Decref(new_set);
     goto err;
    }
   }
   for (i = 0; i < self->a_multiple.m_astc; ++i)
        ast_decref(self->a_multiple.m_astv[i]);
   Dee_Free(self->a_multiple.m_astv);
   self->a_constexpr = new_set;
  } else if (self->a_flag == AST_FMULTIPLE_DICT) {
   DREF DeeObject *new_dict; size_t i;
   new_dict = DeeDict_New();
   if unlikely(!new_dict) goto err;
   for (i = 0; i < self->a_multiple.m_astc/2; ++i) {
    DeeObject *key  = self->a_multiple.m_astv[(i*2)+0]->a_constexpr;
    DeeObject *item = self->a_multiple.m_astv[(i*2)+1]->a_constexpr;
    if (DeeObject_SetItem(new_dict,key,item)) {
     Dee_Decref(new_dict);
     goto err;
    }
   }
   for (i = 0; i < self->a_multiple.m_astc; ++i)
        ast_decref(self->a_multiple.m_astv[i]);
   Dee_Free(self->a_multiple.m_astv);
   self->a_constexpr = new_dict; /* Inherit reference. */
  } else if (self->a_flag == AST_FMULTIPLE_GENERIC_KEYS) {
   DREF DeeObject *new_dict; size_t i,length;
   length   = self->a_multiple.m_astc/2;
   new_dict = DeeRoDict_NewWithHint(length);
   if unlikely(!new_dict) goto err;
   for (i = 0; i < length; ++i) {
    DeeObject *key  = self->a_multiple.m_astv[(i*2)+0]->a_constexpr;
    DeeObject *item = self->a_multiple.m_astv[(i*2)+1]->a_constexpr;
    if (DeeRoDict_Insert(&new_dict,key,item)) {
     Dee_Decref(new_dict);
     goto err;
    }
   }
   for (i = 0; i < self->a_multiple.m_astc; ++i)
        ast_decref(self->a_multiple.m_astv[i]);
   Dee_Free(self->a_multiple.m_astv);
   self->a_constexpr = new_dict; /* Inherit reference. */
  } else {
   goto after_multiple_constexpr;
  }
  self->a_flag = AST_FNORMAL;
  self->a_type = AST_CONSTEXPR;
  OPTIMIZE_VERBOSE("Reduce constant-expression multi-ast to %r\n",
                   self->a_constexpr);
  goto did_optimize;
 }
after_multiple_constexpr:
 if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
  switch (self->a_multiple.m_astc) {
  case 1:
   if (self->a_scope == self->a_multiple.m_astv[0]->a_scope) {
    /* We can simply inherit this single branch, simplifying the entire AST. */
    if (ast_assign(self,self->a_multiple.m_astv[0])) goto err;
    OPTIMIZE_VERBOSE("Collapse single-branch multi-ast\n");
    goto did_optimize;
   }
   break;
  case 0:
   /* Convert this branch into `none' */
   Dee_Free(self->a_multiple.m_astv);
   self->a_type = AST_CONSTEXPR;
   self->a_flag = AST_FNORMAL;
   self->a_constexpr = Dee_None;
   Dee_Incref(Dee_None);
   OPTIMIZE_VERBOSE("Replace empty multi-ast with `none'\n");
   goto did_optimize;
  default: break;
  }
 }
 return 0;
err:
 return -1;
did_optimize:
 ++optimizer_count;
 return 0;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_MULTIPLE_C */
