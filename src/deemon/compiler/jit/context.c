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
#ifndef GUARD_DEEMON_COMPILER_JIT_CONTEXT_C
#define GUARD_DEEMON_COMPILER_JIT_CONTEXT_C 1

#include <deemon/api.h>
#include <deemon/compiler/jit.h>
#ifndef CONFIG_NO_JIT
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/class.h>
#include <deemon/compiler/lexer.h>
#include <deemon/util/objectlist.h>
#include <hybrid/unaligned.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

INTERN void FCALL
JITLValue_Fini(JITLValue *__restrict self) {
 switch (self->lv_kind) {
 case JIT_LVALUE_RANGE:
  Dee_Decref(self->lv_range.lr_end);
  ATTR_FALLTHROUGH
 case JIT_LVALUE_ATTR:
 case JIT_LVALUE_ITEM:
  Dee_Decref(self->lv_item.li_index);
  ATTR_FALLTHROUGH
 case JIT_LVALUE_EXTERN:
 case JIT_LVALUE_USERGLOB:
 case JIT_LVALUE_ATTR_STR:
 case JIT_LVALUE_RVALUE:
  Dee_Decref(self->lv_uglob_name);
  break;
 default: break;
 }
}


PRIVATE bool FCALL
update_symbol_objent(JITSymbol *__restrict self) {
 struct jit_object_table *tab;
 struct jit_object_entry *ent;
 ASSERT(self->js_kind == JIT_SYMBOL_OBJENT);
 ent = self->js_objent.jo_ent;
 tab = self->js_objent.jo_tab;
 if (ent < tab->ot_list)
     goto do_reload;
 if (ent > tab->ot_list + tab->ot_mask)
     goto do_reload;
 if (ent->oe_namestr != self->js_objent.jo_namestr)
     goto do_reload;
 if (ent->oe_namelen != self->js_objent.jo_namelen)
     goto do_reload;
 return true;
do_reload:
 {
  dhash_t i,perturb,hash;
  hash = hash_ptr(self->js_objent.jo_namestr,
                  self->js_objent.jo_namelen);
  i = perturb = hash & tab->ot_mask;
  for (;; JITObjectTable_NEXT(i,perturb)) {
   ent = &tab->ot_list[i & tab->ot_mask];
   if unlikely(!ent->oe_nameobj) goto err_unloaded;
   if unlikely(ent->oe_nameobj == ITER_DONE) continue;
   if (ent->oe_namehsh != hash) continue;
   if (ent->oe_namelen != self->js_objent.jo_namelen) continue;
   if (ent->oe_namestr == self->js_objent.jo_namestr) break; /* Exact same string */
   if (memcmp(ent->oe_namestr,
              self->js_objent.jo_namestr,
              self->js_objent.jo_namelen *
              sizeof(char)) != 0)
       continue;
   /* Found it! */
   break;
  }
  /* Update cached values. */
  self->js_objent.jo_ent     = ent;
  self->js_objent.jo_namestr = ent->oe_namestr;
  self->js_objent.jo_namelen = ent->oe_namelen;
 }
 return true;
err_unloaded:
 DeeError_Throwf(&DeeError_SymbolError,
                 "Unknown variable `%$s'",
                 self->js_objent.jo_namelen,
                 self->js_objent.jo_namestr);
 return false;
}



INTDEF ATTR_COLD int DCALL err_cannot_test_binding(void);
INTDEF DREF DeeObject *DCALL module_getattr_symbol(DeeModuleObject *__restrict self, struct module_symbol *__restrict symbol);
INTDEF int DCALL module_boundattr_symbol(DeeModuleObject *__restrict self, struct module_symbol *__restrict symbol);
INTDEF int DCALL module_delattr_symbol(DeeModuleObject *__restrict self, struct module_symbol *__restrict symbol);
INTDEF int DCALL module_setattr_symbol(DeeModuleObject *__restrict self, struct module_symbol *__restrict symbol, DeeObject *__restrict value);



INTERN int FCALL
JITLValue_IsBound(JITLValue *__restrict self,
                  JITContext *__restrict context) {
 int result;
 ASSERT(self->lv_kind != JIT_LVALUE_NONE);
 switch (self->lv_kind) {

 case JIT_LVALUE_POINTER:
  return *self->lv_ptr != NULL;

 case JIT_LVALUE_OBJENT:
  if unlikely(!update_symbol_objent((JITSymbol *)self))
     goto err;
  result = self->lv_objent.lo_ent->oe_value != NULL;
  break;

 case JIT_LVALUE_EXTERN:
  result = module_boundattr_symbol(self->lv_extern.lx_mod,
                                   self->lv_extern.lx_sym);
  if (result < -1) result = 0; /* Attribute doesn't exist */
  break;

 case JIT_LVALUE_USERGLOB:
  ASSERT(context->jc_module || context->jc_uglobals);
  if (context->jc_module) {
   result = DeeModule_BoundAttrString((DeeModuleObject *)context->jc_module,
                                       DeeString_STR(self->lv_uglob_name),
                                       DeeString_Hash((DeeObject *)self->lv_uglob_name));
  } else {
   result = DeeObject_BoundItem(context->jc_uglobals,
                               (DeeObject *)self->lv_uglob_name,
                                true);
  }
  if (result < -1) result = 0; /* Attribute doesn't exist */
  break;

 case JIT_LVALUE_ATTR:
  result = DeeObject_BoundAttr(self->lv_attr.la_base,
                              (DeeObject *)self->lv_attr.la_name);
  if (result < -1) result = 0; /* Attribute doesn't exist */
  break;

 {
  DREF DeeObject *attr_name_ob;
 case JIT_LVALUE_ATTR_STR:
  attr_name_ob = DeeString_NewUtf8(self->lv_attr_str.la_name,
                                   self->lv_attr_str.la_nsiz,
                                   STRING_ERROR_FSTRICT);
  if unlikely(!attr_name_ob) goto err;
  result = DeeObject_BoundAttr(self->lv_attr.la_base,
                               attr_name_ob);
  self->lv_attr.la_name = (DREF DeeStringObject *)attr_name_ob; /* Inherit reference. */
  self->lv_kind = JIT_LVALUE_ATTR;
  if (result < -1) result = 0; /* Attribute doesn't exist */
 } break;

 case JIT_LVALUE_ITEM:
  result = DeeObject_BoundItem(self->lv_item.li_base,
                               self->lv_item.li_index,
                               true);
  if (result < -1) result = 0; /* Item doesn't exist. */
  break;

 case JIT_LVALUE_RANGE:
  return err_cannot_test_binding();

 case JIT_LVALUE_RVALUE:
  result = 1;
  break;

 default: __builtin_unreachable();
 }
 return result;
err:
 return -1;
}




INTERN DREF DeeObject *FCALL
JITLValue_GetValue(JITLValue *__restrict self,
                   JITContext *__restrict context) {
 DREF DeeObject *result;
 ASSERT(self->lv_kind != JIT_LVALUE_NONE);
 switch (self->lv_kind) {

 case JIT_LVALUE_POINTER:
  result = *self->lv_ptr;
  if unlikely(!result) {
err_unbound:
   DeeError_Throwf(&DeeError_UnboundLocal,
                   "Unbound local variable");
   goto err;
  }
  Dee_Incref(result);
  break;

 case JIT_LVALUE_OBJENT:
  if unlikely(!update_symbol_objent((JITSymbol *)self))
     goto err;
  result = self->lv_objent.lo_ent->oe_value;
  if unlikely(!result)
     goto err_unbound;
  Dee_Incref(result);
  break;

 case JIT_LVALUE_EXTERN:
  result = module_getattr_symbol(self->lv_extern.lx_mod,
                                 self->lv_extern.lx_sym);
  break;

 case JIT_LVALUE_USERGLOB:
  ASSERT(context->jc_module || context->jc_uglobals);
  if (context->jc_module) {
   result = DeeModule_GetAttrString((DeeModuleObject *)context->jc_module,
                                     DeeString_STR(self->lv_uglob_name),
                                     DeeString_Hash((DeeObject *)self->lv_uglob_name));
  } else {
   result = DeeObject_GetItem(context->jc_uglobals,
                             (DeeObject *)self->lv_uglob_name);
  }
  break;

 case JIT_LVALUE_ATTR:
  result = DeeObject_GetAttr(self->lv_attr.la_base,
                            (DeeObject *)self->lv_attr.la_name);
  break;

 {
  DREF DeeObject *attr_name_ob;
 case JIT_LVALUE_ATTR_STR:
  attr_name_ob = DeeString_NewUtf8(self->lv_attr_str.la_name,
                                   self->lv_attr_str.la_nsiz,
                                   STRING_ERROR_FSTRICT);
  if unlikely(!attr_name_ob) goto err;
  result = DeeObject_GetAttr(self->lv_attr.la_base,
                             attr_name_ob);
  self->lv_attr.la_name = (DREF DeeStringObject *)attr_name_ob; /* Inherit reference. */
  self->lv_kind = JIT_LVALUE_ATTR;
 } break;

 case JIT_LVALUE_ITEM:
  result = DeeObject_GetItem(self->lv_item.li_base,
                             self->lv_item.li_index);
  break;
 case JIT_LVALUE_RANGE:
  result = DeeObject_GetRange(self->lv_range.lr_base,
                              self->lv_range.lr_start,
                              self->lv_range.lr_end);
  break;

 case JIT_LVALUE_RVALUE:
  result = self->lv_rvalue;
  Dee_Incref(result);
  break;

 default: __builtin_unreachable();
 }
 return result;
err:
 return NULL;
}




INTERN int FCALL
JITLValue_DelValue(JITLValue *__restrict self,
                   JITContext *__restrict context) {
 int result;
 ASSERT(self->lv_kind != JIT_LVALUE_NONE);
 switch (self->lv_kind) {

 {
  DREF DeeObject *old_value;
 case JIT_LVALUE_POINTER:
  old_value = *self->lv_ptr;
  *self->lv_ptr = NULL;
  Dee_XDecref(old_value);
  result = 0;
 } break;

 {
  DREF DeeObject *old_value;
 case JIT_LVALUE_OBJENT:
  if unlikely(!update_symbol_objent((JITSymbol *)self))
     goto err;
  old_value = self->lv_objent.lo_ent->oe_value;
  self->lv_objent.lo_ent->oe_value = NULL;
  Dee_XDecref(old_value);
  result = 0;
 } break;

 case JIT_LVALUE_EXTERN:
  result = module_delattr_symbol(self->lv_extern.lx_mod,
                                 self->lv_extern.lx_sym);
  break;

 case JIT_LVALUE_USERGLOB:
  ASSERT(context->jc_module || context->jc_uglobals);
  if (context->jc_module) {
   result = DeeModule_DelAttrString((DeeModuleObject *)context->jc_module,
                                     DeeString_STR(self->lv_uglob_name),
                                     DeeString_Hash((DeeObject *)self->lv_uglob_name));
  } else {
   result = DeeObject_DelItem(context->jc_uglobals,
                             (DeeObject *)self->lv_uglob_name);
  }
  break;

 case JIT_LVALUE_ATTR:
  result = DeeObject_DelAttr(self->lv_attr.la_base,
                            (DeeObject *)self->lv_attr.la_name);
  break;
 {
  DREF DeeObject *attr_name_ob;
 case JIT_LVALUE_ATTR_STR:
  attr_name_ob = DeeString_NewUtf8(self->lv_attr_str.la_name,
                                   self->lv_attr_str.la_nsiz,
                                   STRING_ERROR_FSTRICT);
  if unlikely(!attr_name_ob) goto err;
  result = DeeObject_DelAttr(self->lv_attr.la_base,
                             attr_name_ob);
  self->lv_attr.la_name = (DREF DeeStringObject *)attr_name_ob; /* Inherit reference. */
  self->lv_kind = JIT_LVALUE_ATTR;
 } break;

 case JIT_LVALUE_ITEM:
  result = DeeObject_DelItem(self->lv_item.li_base,
                             self->lv_item.li_index);
  break;
 case JIT_LVALUE_RANGE:
  result = DeeObject_DelRange(self->lv_range.lr_base,
                              self->lv_range.lr_start,
                              self->lv_range.lr_end);
  break;

 case JIT_LVALUE_RVALUE:
  result = DeeError_Throwf(&DeeError_SyntaxError,
                           "Cannot delete R-value");
  break;

 default: __builtin_unreachable();
 }
 return result;
err:
 return -1;
}




INTERN int FCALL
JITLValue_SetValue(JITLValue *__restrict self,
                   JITContext *__restrict context,
                   DeeObject *__restrict value) {
 int result;
 ASSERT(self->lv_kind != JIT_LVALUE_NONE);
 switch (self->lv_kind) {

 {
  DREF DeeObject *old_value;
 case JIT_LVALUE_POINTER:
  old_value = *self->lv_ptr;
  *self->lv_ptr = value;
  Dee_Incref(value);
  Dee_XDecref(old_value);
  result = 0;
 } break;

 {
  DREF DeeObject *old_value;
 case JIT_LVALUE_OBJENT:
  if unlikely(!update_symbol_objent((JITSymbol *)self))
     goto err;
  Dee_Incref(value);
  old_value = self->lv_objent.lo_ent->oe_value;
  self->lv_objent.lo_ent->oe_value = value;
  Dee_XDecref(old_value);
  result = 0;
 } break;

 case JIT_LVALUE_EXTERN:
  result = module_setattr_symbol(self->lv_extern.lx_mod,
                                 self->lv_extern.lx_sym,
                                 value);
  break;

 case JIT_LVALUE_USERGLOB:
  ASSERT(context->jc_module || context->jc_uglobals);
  if (context->jc_module) {
   result = DeeModule_SetAttrString((DeeModuleObject *)context->jc_module,
                                     DeeString_STR(self->lv_uglob_name),
                                     DeeString_Hash((DeeObject *)self->lv_uglob_name),
                                     value);
  } else {
   result = DeeObject_SetItem(context->jc_uglobals,
                             (DeeObject *)self->lv_uglob_name,
                              value);
  }
  break;

 case JIT_LVALUE_ATTR:
  result = DeeObject_SetAttr(self->lv_attr.la_base,
                            (DeeObject *)self->lv_attr.la_name,
                             value);
  break;

 {
  DREF DeeObject *attr_name_ob;
 case JIT_LVALUE_ATTR_STR:
  attr_name_ob = DeeString_NewUtf8(self->lv_attr_str.la_name,
                                   self->lv_attr_str.la_nsiz,
                                   STRING_ERROR_FSTRICT);
  if unlikely(!attr_name_ob) goto err;
  result = DeeObject_SetAttr(self->lv_attr.la_base,
                             attr_name_ob,
                             value);
  self->lv_attr.la_name = (DREF DeeStringObject *)attr_name_ob; /* Inherit reference. */
  self->lv_kind = JIT_LVALUE_ATTR;
 } break;

 case JIT_LVALUE_ITEM:
  result = DeeObject_SetItem(self->lv_item.li_base,
                             self->lv_item.li_index,
                             value);
  break;

 case JIT_LVALUE_RANGE:
  result = DeeObject_SetRange(self->lv_range.lr_base,
                              self->lv_range.lr_start,
                              self->lv_range.lr_end,
                              value);
  break;

 case JIT_LVALUE_RVALUE:
  result = DeeError_Throwf(&DeeError_SyntaxError,
                           "Cannot store to R-value");
  break;

 default: __builtin_unreachable();
 }
 return result;
err:
 return -1;
}


/* Finalize the given L-value list. */
INTERN void DCALL
JITLValueList_Fini(JITLValueList *__restrict self) {
 size_t i;
 for (i = 0; i < self->ll_size; ++i)
    JITLValue_Fini(&self->ll_list[i]);
 Dee_Free(self->ll_list);
}

/* Append the given @value onto @self, returning -1 on error and 0 on success. */
INTERN int DCALL
JITLValueList_Append(JITLValueList *__restrict self,
                     /*inherit(on_success)*/JITLValue *__restrict value) {
 ASSERT(self->ll_size <= self->ll_alloc);
 if (self->ll_size >= self->ll_alloc) {
  JITLValue *new_list;
  size_t new_alloc = self->ll_alloc * 2;
  if (!new_alloc) new_alloc = 1;
  ASSERT(new_alloc > self->ll_size);
  new_list = (JITLValue *)Dee_TryRealloc(self->ll_list,
                                         new_alloc *
                                         sizeof(JITLValue));
  if unlikely(!new_list) {
   new_alloc = self->ll_size + 1;
   new_list = (JITLValue *)Dee_Realloc(self->ll_list,
                                       new_alloc *
                                       sizeof(JITLValue));
   if unlikely(!new_list) goto err;
  }
  self->ll_list  = new_list;
  self->ll_alloc = new_alloc;
 }
 /* Inherit all of the given data. */
 memcpy(&self->ll_list[self->ll_size],value,sizeof(JITLValue));
 ++self->ll_size;
 return 0;
err:
 return -1;
}

/* Append an R-value expression. */
INTERN int DCALL
JITLValueList_AppendRValue(JITLValueList *__restrict self,
                           DeeObject *__restrict value) {
 ASSERT(self->ll_size <= self->ll_alloc);
 if (self->ll_size >= self->ll_alloc) {
  JITLValue *new_list;
  size_t new_alloc = self->ll_alloc * 2;
  if (!new_alloc) new_alloc = 1;
  ASSERT(new_alloc > self->ll_size);
  new_list = (JITLValue *)Dee_TryRealloc(self->ll_list,
                                         new_alloc *
                                         sizeof(JITLValue));
  if unlikely(!new_list) {
   new_alloc = self->ll_size + 1;
   new_list = (JITLValue *)Dee_Realloc(self->ll_list,
                                       new_alloc *
                                       sizeof(JITLValue));
   if unlikely(!new_list) goto err;
  }
  self->ll_list  = new_list;
  self->ll_alloc = new_alloc;
 }
 /* Initialize an R-value descriptor. */
 self->ll_list[self->ll_size].lv_kind   = JIT_LVALUE_RVALUE;
 self->ll_list[self->ll_size].lv_rvalue = value;
 Dee_Incref(value);
 ++self->ll_size;
 return 0;
err:
 return -1;
}


/* Pack `self' and append all of the referenced objects to the given object list. */
INTERN int DCALL
JITLValueList_CopyObjects(JITLValueList *__restrict self,
                          struct objectlist *__restrict dst,
                          JITContext *__restrict context) {
 size_t i; DREF DeeObject **buf;
 if (!self->ll_size) goto done;
 buf = objectlist_alloc(dst,self->ll_size);
 if unlikely(!buf) goto err;
 for (i = 0; i < self->ll_size; ++i) {
  DREF DeeObject *ob;
  ob = JITLValue_GetValue(&self->ll_list[i],context);
  if unlikely(!ob)
     goto err_i;
  buf[i] = ob; /* Inherit reference. */
 }
done:
 return 0;
err_i:
 dst->ol_size -= (self->ll_size - i);
err:
 return -1;
}




/* Similar to `JITLexer_GetLValue()', but also finalize
 * the stored L-value, and set it to describe nothing.
 * NOTE: The stored L-value is _always_ reset! */
INTERN DREF DeeObject *DCALL
JITLexer_PackLValue(JITLexer *__restrict self) {
 DREF DeeObject *result;
 result = JITLValue_GetValue(&self->jl_lvalue,
                              self->jl_context);
 JITLValue_Fini(&self->jl_lvalue);
 self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
 return result;
}


/* Lookup a given symbol within a specific JIT context
 * @param: mode: Set of `LOOKUP_SYM_*'
 * @return: 0:  The specified symbol was found, and `result' was filled
 * @return: -1: An error occurred. */
INTERN int FCALL
JITContext_Lookup(JITContext *__restrict self,
                  struct jit_symbol *__restrict result,
                  /*utf-8*/char const *__restrict name,
                  size_t namelen, unsigned int mode) {
 (void)self;
 (void)result;
 (void)name;
 (void)namelen;
 (void)mode;
 /* TODO */
 return DeeError_Throwf(&DeeError_SymbolError,
                        "Unknown variable `%$s'",
                        namelen,name);
}
INTERN int FCALL
JITContext_LookupNth(JITContext *__restrict self,
                     struct jit_symbol *__restrict result,
                     /*utf-8*/char const *__restrict name,
                     size_t namelen, size_t nth) {
 (void)self;
 (void)result;
 (void)name;
 (void)namelen;
 (void)nth;
 /* TODO */
 return DeeError_Throwf(&DeeError_SymbolError,
                        "Unknown variable `%$s'",
                        namelen,name);
}



DECL_END

#endif /* !CONFIG_NO_JIT */

#endif /* !GUARD_DEEMON_COMPILER_JIT_CONTEXT_C */
