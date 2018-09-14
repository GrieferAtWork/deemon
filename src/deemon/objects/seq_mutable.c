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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MUTABLE_C
#define GUARD_DEEMON_OBJECTS_SEQ_MUTABLE_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/int.h>
#include <deemon/bool.h>
#include <deemon/none.h>
#include <deemon/thread.h>
#include <deemon/class.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/error.h>
#include <deemon/callable.h>
#include <deemon/super.h>

#include "seq_functions.h"

#include "seq/svec.h"
#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#ifndef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX  __SSIZE_MAX__
#endif

DECL_BEGIN


PRIVATE int DCALL
has_generic_attribute(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      DeeObject *__restrict attr) {
 struct membercache *cache;
 if (tp_self->tp_attr) {
  if (tp_self->tp_attr->tp_getattr) {
   DREF DeeObject *obj;
   if (tp_self->tp_attr->tp_getattr == &instance_getattr)
    obj = instance_tgetattr(tp_self,self,attr);
   else {
    obj = (*tp_self->tp_attr->tp_getattr)(self,attr);
   }
   if (obj) { Dee_Decref(obj); return 1; }
   if (DeeError_Catch(&DeeError_NotImplemented) ||
       DeeError_Catch(&DeeError_AttributeError))
       return 0;
   return -1;
  }
 } else {
  char *name = DeeString_STR(attr);
  dhash_t hash = DeeString_Hash(attr);
  if (DeeType_IsClass(tp_self))
      return DeeClass_QueryInstanceAttributeStringWithHash(tp_self,name,hash) != NULL ? 1 : 0;
  cache = &tp_self->tp_cache;
#if 0 /* Don't use the cache, which may contain members from lower-level types! */
  if (membercache_hasattr(cache,name,hash))
      goto yes;
#endif
  if (tp_self->tp_methods &&
      type_method_hasattr(cache,tp_self->tp_methods,name,hash))
      goto yes;
  if (tp_self->tp_getsets &&
      type_getset_hasattr(cache,tp_self->tp_getsets,name,hash))
      goto yes;
  if (tp_self->tp_members &&
      type_member_hasattr(cache,tp_self->tp_members,name,hash))
      goto yes;
 }
 return 0;
yes:
 return 1;
}

/* @return:  0: Call was OK.
 * @return:  1: No such attribute.
 * @return: -1: Error. */
PRIVATE DREF DeeObject *DCALL
vcall_generic_attribute(DeeTypeObject *__restrict tp_self,
                        DeeObject *__restrict self,
                        char const *__restrict name, dhash_t hash,
                        char const *__restrict format,
                        va_list args) {
 struct membercache *cache;
 DREF DeeObject *result;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 if (DeeType_IsClass(tp_self)) {
  struct class_attribute *member;
  struct class_desc *desc = DeeClass_DESC(tp_self);
  if ((member = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
   DREF DeeObject *args_tuple;
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   result = DeeInstance_CallAttribute(tp_self,
                                      DeeInstance_DESC(desc,
                                                       self),
                                      self,
                                      member,
                                      DeeTuple_SIZE(args_tuple),
                                      DeeTuple_ELEM(args_tuple));
   Dee_Decref(args_tuple);
   return result;
  }
  result = ITER_DONE;
 } else {
  cache = &tp_self->tp_cache;
#if 0 /* Don't use the cache, which may contain members from lower-level types! */
  if ((result = membercache_vcallattrf(cache,self,name,hash,format,args)) != ITER_DONE)
       goto done;
#else
  result = ITER_DONE;
#endif
  if (tp_self->tp_methods &&
     (result = type_method_vcallattrf(cache,tp_self->tp_methods,self,name,hash,format,args)) != ITER_DONE)
      goto done;
  if (tp_self->tp_getsets &&
     (result = type_getset_getattr(cache,tp_self->tp_getsets,self,name,hash)) != ITER_DONE)
      goto done_call;
  if (tp_self->tp_members &&
     (result = type_member_getattr(cache,tp_self->tp_members,self,name,hash)) != ITER_DONE)
      goto done_call;
 }
done:
 return result;
done_call:
 if likely(result) {
  DREF DeeObject *real_result;
  real_result = DeeObject_VCallf(result,format,args);
  Dee_Decref(result);
  result = real_result;
 }
 return result;
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
get_generic_attribute(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      DeeObject *__restrict name) {
 struct membercache *cache;
 DREF DeeObject *result;
 dhash_t hash;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 hash = DeeString_Hash(name);
 if (DeeType_IsClass(tp_self)) {
  struct class_attribute *member;
  struct class_desc *desc = DeeClass_DESC(tp_self);
  if ((member = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,
                                                                  DeeString_STR(name),
                                                                  hash)) != NULL) {
   return DeeInstance_GetAttribute(tp_self,
                                   DeeInstance_DESC(desc,
                                                    self),
                                   self,
                                   member);
  }
  result = ITER_DONE;
 } else {
  cache = &tp_self->tp_cache;
#if 0 /* Don't use the cache, which may contain members from lower-level types! */
  if ((result = membercache_getattr(cache,self,
                                    DeeString_STR(name),
                                    hash)) != ITER_DONE)
       goto done;
#else
  result = ITER_DONE;
#endif
  if (tp_self->tp_methods &&
     (result = type_method_getattr(cache,tp_self->tp_methods,self,DeeString_STR(name),hash)) != ITER_DONE)
      goto done;
  if (tp_self->tp_getsets &&
     (result = type_getset_getattr(cache,tp_self->tp_getsets,self,DeeString_STR(name),hash)) != ITER_DONE)
      goto done;
  if (tp_self->tp_members &&
     (result = type_member_getattr(cache,tp_self->tp_members,self,DeeString_STR(name),hash)) != ITER_DONE)
      goto done;
 }
done:
 return result;
}

PRIVATE DREF DeeObject *
call_generic_attribute(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self,
                       char const *__restrict name, dhash_t hash,
                       char const *__restrict format,
                       ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = vcall_generic_attribute(tp_self,self,name,hash,format,args);
 va_end(args);
 return result;
}

PRIVATE DREF DeeObject *
call_generic_attribute_in_range(DeeTypeObject *__restrict tp_limit,
                                DeeObject *__restrict self,
                                DeeObject *__restrict name,
                                char const *__restrict format,
                                ...) {
 DREF DeeObject *result;
 DeeTypeObject *iter = Dee_TYPE(self);
 va_list args;
 for (;;) {
  if (iter->tp_attr) {
   if (iter->tp_attr->tp_getattr) {
    DREF DeeObject *func;
    if (iter->tp_attr->tp_getattr == &instance_getattr)
     func = instance_tgetattr(iter,self,name);
    else {
     func = (*iter->tp_attr->tp_getattr)(self,name);
    }
    if unlikely(!func)
       goto err;
    va_start(args,format);
    result = DeeObject_VCallf(func,format,args);
    va_end(args);
    Dee_Decref(func);
    return result;
   }
   break;
  } else {
   /* Try to invoke a generic attribute. */
   va_start(args,format);
   result = vcall_generic_attribute(iter,
                                    self,
                                    DeeString_STR(name),
                                    DeeString_Hash(name),
                                    format,
                                    args);
   va_end(args);
   if (result != ITER_DONE)
       return result;
  }
  if (iter == tp_limit) break;
  iter = DeeType_Base(iter);
 }
 return ITER_DONE;
err:
 return NULL;
}


PRIVATE DREF DeeObject *
call_generic_attribute_anywhere(DeeObject *__restrict self,
                                DeeObject *__restrict name,
                                char const *__restrict format,
                                ...) {
 DREF DeeObject *result;
 DeeTypeObject *iter = Dee_TYPE(self);
 va_list args;
 for (;;) {
  if (iter->tp_attr) {
   if (iter->tp_attr->tp_getattr) {
    DREF DeeObject *func;
    if (iter->tp_attr->tp_getattr == &instance_getattr)
     func = instance_tgetattr(iter,self,name);
    else {
     func = (*iter->tp_attr->tp_getattr)(self,name);
    }
    if unlikely(!func)
       goto err;
    va_start(args,format);
    result = DeeObject_VCallf(func,format,args);
    va_end(args);
    Dee_Decref(func);
    return result;
   }
   break;
  } else {
   /* Try to invoke a generic attribute. */
   va_start(args,format);
   result = vcall_generic_attribute(iter,
                                    self,
                                    DeeString_STR(name),
                                    DeeString_Hash(name),
                                    format,
                                    args);
   va_end(args);
   if (result != ITER_DONE)
       return result;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
 }
 return ITER_DONE;
err:
 return NULL;
}



INTERN int DCALL
DeeSeq_DelItem(DeeObject *__restrict self, size_t index) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 bool found_attributes = false;
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_delitem)
        return (*nsi->nsi_seqlike.nsi_delitem)(self,index);
    if (nsi->nsi_seqlike.nsi_setrange) {
     size_t my_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(my_length == (size_t)-1) goto err;
     if unlikely(index >= my_length)
        return err_index_out_of_bounds(self,index,my_length);
     return (*nsi->nsi_seqlike.nsi_setrange)(self,
                                             index,
                                             index + 1,
                                             Dee_None);
    }
    if (nsi->nsi_seqlike.nsi_erase) {
     size_t temp;
     temp = (*nsi->nsi_seqlike.nsi_erase)(self,index,1);
     if unlikely(temp == (size_t)-1) goto err;
     return 0;
    }
    if (nsi->nsi_seqlike.nsi_pop) {
     DREF DeeObject *temp;
     if ((dssize_t)index < 0)
          return err_integer_overflow_i(sizeof(size_t)*8-1,false);
     temp = (*nsi->nsi_seqlike.nsi_pop)(self,index);
     if unlikely(!temp) goto err;
     Dee_Decref(temp);
     return 0;
    }
   }
   if (has_noninherited_delitem(tp_self,seq)) {
    /* Try to invoke the native delitem operator. */
    DREF DeeObject *index_ob;
    index_ob = DeeInt_NewSize(index);
    if unlikely(!index_ob) goto err;
    result = (*seq->tp_del)(self,index_ob);
    Dee_Decref(index_ob);
    return result;
   }
  }
  if (!found_attributes) {
   if (tp_self->tp_attr) {
    if (tp_self->tp_attr->tp_getattr) {
     DREF DeeObject *erase_function,*erase_result;
     if (tp_self->tp_attr->tp_getattr == &instance_getattr)
      erase_function = instance_tgetattr(tp_self,self,&str_erase);
     else {
      erase_function = (*tp_self->tp_attr->tp_getattr)(self,&str_erase);
     }
     if unlikely(!erase_function)
        goto err_bad_attribute;
     /* Found an erase() function. -> Now call it! */
     erase_result = DeeObject_Callf(erase_function,"Iuu",index,1u);
     Dee_Decref(erase_function);
     if unlikely(!erase_result) goto err_bad_attribute;
     Dee_Decref(erase_result);
     return 0;
    }
did_find_attributes:
    found_attributes = true;
   } else {
    /* Try to invoke a generic attribute. */
    DREF DeeObject *callback_result;
    callback_result = call_generic_attribute(tp_self,
                                             self,
                                             DeeString_STR(&str_erase),
                                             DeeString_Hash(&str_erase),
                                             "Iuu",
                                             index,
                                             1);
    if (callback_result != ITER_DONE) {
     if unlikely(!callback_result)
        goto err_bad_attribute;
     /* Invocation was successful */
     Dee_Decref(callback_result);
     return 0;
    }
   }
  }
  if (seq &&
     (has_noninherited_delrange(tp_self,seq) ||
      has_noninherited_setrange(tp_self,seq))) {
   /* Try to implement delitem using delrange or setrange. */
   DREF DeeObject *start_index,*end_index;
   size_t mylen;
   if (seq->tp_nsi &&
       is_noninherited_nsi(tp_self,seq,seq->tp_nsi)) {
    mylen = (*seq->tp_nsi->nsi_common.nsi_getsize)(self);
   } else {
    mylen = DeeObject_Size(self);
   }
   if unlikely(mylen == (size_t)-1) goto err;
   start_index = DeeInt_NewSize(index);
   if unlikely(!start_index) goto err;
   end_index = DeeInt_NewSize(index+1);
   if unlikely(!end_index) { Dee_Decref(start_index); goto err; }
   if (has_noninherited_delrange(tp_self,seq))
    result = (*seq->tp_range_del)(self,start_index,end_index);
   else {
    result = (*seq->tp_range_set)(self,start_index,end_index,Dee_None);
   }
   Dee_Decref(end_index);
   Dee_Decref(start_index);
   return result;
  }
  /*if (seq) break;*/
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 return err_fixedlength_sequence(self);
err_bad_attribute:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto did_find_attributes;
err:
 return -1;
}

INTERN int DCALL
DeeSeq_SetItem(DeeObject *__restrict self, size_t index,
               DeeObject *__restrict value) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_setitem)
        return (*nsi->nsi_seqlike.nsi_setitem)(self,index,value);
    if (nsi->nsi_seqlike.nsi_xch) {
     DREF DeeObject *old_value;
     old_value = (*nsi->nsi_seqlike.nsi_xch)(self,index,value);
     if unlikely(!old_value) goto err;
     Dee_Decref(old_value);
     return 0;
    }
    if (nsi->nsi_seqlike.nsi_setrange) {
     size_t my_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
     DREF DeeObject *value_seq;
     if unlikely(my_length == (size_t)-1) goto err;
     if unlikely(index >= my_length)
        return err_index_out_of_bounds(self,index,my_length);
     value_seq = DeeTuple_Pack(1,value);
     if unlikely(!value_seq) goto err;
     Dee_Incref(value);
     result = (*nsi->nsi_seqlike.nsi_setrange)(self,
                                               index,
                                               index + 1,
                                               value_seq);
     Dee_Decref(value_seq);
     return result;
    }
   }
   if (has_noninherited_setitem(tp_self,seq)) {
    /* Try to invoke the native delitem operator. */
    DREF DeeObject *index_ob;
    index_ob = DeeInt_NewSize(index);
    if unlikely(!index_ob) goto err;
    result = (*seq->tp_set)(self,index_ob,value);
    Dee_Decref(index_ob);
    return result;
   }
   if (has_noninherited_setrange(tp_self,seq)) {
    /* Try to implement setitem using setrange. */
    DREF DeeObject *start_index,*end_index;
    DREF DeeObject *value_seq; size_t mylen;
    mylen = nsi ? (*nsi->nsi_common.nsi_getsize)(self) : DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    start_index = DeeInt_NewSize(index);
    if unlikely(!start_index) goto err;
    end_index = DeeInt_NewSize(index+1);
    if unlikely(!end_index) { err_start_index: Dee_Decref(start_index); goto err; }
    value_seq = DeeTuple_Pack(1,value);
    if unlikely(!value_seq) { Dee_Decref(end_index); goto err_start_index; }
    result = (*seq->tp_range_set)(self,start_index,end_index, value_seq);
    Dee_Decref(value_seq);
    Dee_Decref(end_index);
    Dee_Decref(start_index);
    return result;
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 return err_immutable_sequence(self);
err:
 return -1;
}

INTERN DREF DeeObject *DCALL
DeeSeq_XchItem(DeeObject *__restrict self, size_t index,
               DeeObject *__restrict value) {
 DREF DeeObject *result; int error;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_xch)
        return (*nsi->nsi_seqlike.nsi_xch)(self,index,value);
    if (nsi->nsi_seqlike.nsi_getitem) {
     if (nsi->nsi_seqlike.nsi_setitem) {
      result = (*nsi->nsi_seqlike.nsi_getitem)(self,index);
      if unlikely(!result) goto err;
      error = (*nsi->nsi_seqlike.nsi_setitem)(self,index,value);
      if unlikely(error) goto err_r;
      return result;
     }
     if (nsi->nsi_seqlike.nsi_setrange) {
      size_t my_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
      DREF DeeObject *value_seq;
      if unlikely(my_length == (size_t)-1) goto err;
      if unlikely(index >= my_length) {
       err_index_out_of_bounds(self,index,my_length);
       goto err;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem)(self,index);
      if unlikely(!result) goto err;
      value_seq = DeeTuple_Pack(1,value);
      if unlikely(!value_seq) goto err_r;
      error = (*nsi->nsi_seqlike.nsi_setrange)(self,
                                               index,
                                               index + 1,
                                               value_seq);
      Dee_Decref(value_seq);
      if unlikely(error) goto err_r;
      return result;
     }
    }
    if (nsi->nsi_seqlike.nsi_getrange) {
     if (nsi->nsi_seqlike.nsi_setitem) {
      DREF DeeObject *real_result;
      result = (*nsi->nsi_seqlike.nsi_getrange)(self,index,index+1);
      if unlikely(!result) goto err;
      error = (*nsi->nsi_seqlike.nsi_setitem)(self,index,value);
      if unlikely(error) goto err_r;
return_result_first:
      real_result = DeeSeq_Front(result);
      Dee_Decref(result);
      if unlikely(!real_result) {
       /* Translate the empty-sequence error into an index-out-of-bounds */
       if (DeeError_Catch(&DeeError_ValueError)) {
        size_t mylen = DeeObject_Size(self);
        if unlikely(mylen == (size_t)-1) goto err;
        err_index_out_of_bounds(self,index,mylen);
       }
      }
      return real_result;
     }
     if (nsi->nsi_seqlike.nsi_setrange) {
      size_t my_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
      DREF DeeObject *value_seq;
      if unlikely(my_length == (size_t)-1) goto err;
      if unlikely(index >= my_length) {
       err_index_out_of_bounds(self,index,my_length);
       goto err;
      }
      result = (*nsi->nsi_seqlike.nsi_getrange)(self,index,index+1);
      if unlikely(!result) goto err;
      value_seq = DeeTuple_Pack(1,value);
      if unlikely(!value_seq) goto err_r;
      error = (*nsi->nsi_seqlike.nsi_setrange)(self,
                                               index,
                                               index + 1,
                                               value_seq);
      Dee_Decref(value_seq);
      if unlikely(error) goto err_r;
      goto return_result_first;
     }
    }
   }
   if (has_noninherited_setitem(tp_self,seq)) {
    DREF DeeObject *index_ob;
    index_ob = DeeInt_NewSize(index);
    if unlikely(!index_ob) goto err;
    result = DeeObject_GetItem(self,index_ob);
    if unlikely(!result) { Dee_Decref(index_ob); goto err; }
    error = (*seq->tp_set)(self,index_ob,value);
    Dee_Decref(index_ob);
    if unlikely(error) goto err_r;
    return result;
   }
   if (has_noninherited_setrange(tp_self,seq)) {
    DREF DeeObject *index_ob,*index_plus1_ob;
    index_ob = DeeInt_NewSize(index);
    if unlikely(!index_ob) goto err;
    result = DeeObject_GetItem(self,index_ob);
    if unlikely(!result) { Dee_Decref(index_ob); goto err; }
    index_plus1_ob = DeeInt_NewSize(index + 1);
    if unlikely(!index_plus1_ob) { Dee_Decref(index_ob); goto err_r; }
    error = (*seq->tp_range_set)(self,index_ob,index_plus1_ob,value);
    Dee_Decref(index_plus1_ob);
    Dee_Decref(index_ob);
    if unlikely(error) goto err_r;
    return result;
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 err_immutable_sequence(self);
err:
 return NULL;
err_r:
 Dee_Decref(result);
 goto err;
}

INTERN int DCALL
DeeSeq_DelRange(DeeObject *__restrict self,
                size_t start, size_t end) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_setrange)
        return (*nsi->nsi_seqlike.nsi_setrange)(self,start,end,Dee_None);
    if (nsi->nsi_seqlike.nsi_erase) {
     size_t temp,mylen;
     mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (start > mylen) start = mylen;
     if (end > start) end = start;
     temp = (*nsi->nsi_seqlike.nsi_erase)(self,start,end - start);
     if unlikely(temp == (size_t)-1) goto err;
     return 0;
    }
    if (nsi->nsi_seqlike.nsi_delitem) {
     size_t mylen;
     mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (end > mylen) end = mylen;
     while (end > start) {
      --end;
      if ((*nsi->nsi_seqlike.nsi_delitem)(self,end))
           goto err;
     }
     return 0;
    }
    if (nsi->nsi_seqlike.nsi_pop) {
     size_t mylen;
     mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (end > mylen) end = mylen;
     while (end > start) {
      DREF DeeObject *temp;
      --end;
      temp = (*nsi->nsi_seqlike.nsi_pop)(self,end);
      if unlikely(!temp) goto err;
      Dee_Decref(temp);
     }
     return 0;
    }
   }
   if (has_noninherited_delrange(tp_self,seq) ||
       has_noninherited_setrange(tp_self,seq)) {
    /* Try to implement delitem using delrange or setrange. */
    DREF DeeObject *start_index,*end_index;
    start_index = DeeInt_NewSize(start);
    if unlikely(!start_index) goto err;
    end_index = DeeInt_NewSize(end);
    if unlikely(!end_index) { Dee_Decref(start_index); goto err; }
    if (has_noninherited_delrange(tp_self,seq))
     result = (*seq->tp_range_del)(self,start_index,end_index);
    else {
     result = (*seq->tp_range_set)(self,start_index,end_index,Dee_None);
    }
    Dee_Decref(end_index);
    Dee_Decref(start_index);
    return result;
   }
   {
    size_t mylen;
    DREF DeeObject *erase_result;
    mylen = nsi ? (*nsi->nsi_common.nsi_getsize)(self) : DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    /* Search for a type implementing an `erase()' function. */
    if (start > mylen) start = mylen;
    if (end < start) end = start;
    end -= start;
    erase_result = call_generic_attribute_in_range(tp_self,
                                                   self,
                                                   &str_erase,
                                                   "IuIu",
                                                   start,
                                                   end);
    if (erase_result != ITER_DONE) {
     if unlikely(!erase_result)
        goto err_bad_attribute;
     Dee_Decref(erase_result);
     return 0;
    }
    /* Last chance: Use `operator del[]' to erase each item individually. */
    if (has_noninherited_delitem(tp_self,seq)) {
     while (end--) {
      DREF DeeObject *index_ob;
      index_ob = DeeInt_NewSize(start + end);
      if unlikely(!index_ob) goto err;
      result = (*seq->tp_del)(self,index_ob);
      Dee_Decref(index_ob);
      if unlikely(result) goto err;
     }
     return 0;
    }
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 return err_immutable_sequence(self);
err_bad_attribute:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
err:
 return -1;
}

INTERN int DCALL
DeeSeq_DelRangeN(DeeObject *__restrict self, size_t start) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (start == 0) {
   DREF DeeObject *callback_result;
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_clear),
                                            DeeString_Hash(&str_clear),
                                            "");
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_bad_attribute;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_setrange_n)
        return (*nsi->nsi_seqlike.nsi_setrange_n)(self,start,Dee_None);
    if (nsi->nsi_seqlike.nsi_setrange) {
     size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     return (*nsi->nsi_seqlike.nsi_setrange)(self,start,mylen,Dee_None);
    }
    if (nsi->nsi_seqlike.nsi_erase) {
     size_t temp,mylen;
     mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (start > mylen) start = mylen;
     temp = (*nsi->nsi_seqlike.nsi_erase)(self,start,mylen - start);
     if unlikely(temp == (size_t)-1) goto err;
     return 0;
    }
    if (nsi->nsi_seqlike.nsi_delitem) {
     size_t mylen;
     mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     while (mylen > start) {
      --mylen;
      if ((*nsi->nsi_seqlike.nsi_delitem)(self,mylen))
           goto err;
     }
     return 0;
    }
    if (nsi->nsi_seqlike.nsi_pop) {
     size_t mylen;
     mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     while (mylen > start) {
      DREF DeeObject *temp;
      --mylen;
      temp = (*nsi->nsi_seqlike.nsi_pop)(self,mylen);
      if unlikely(!temp) goto err;
      Dee_Decref(temp);
     }
     return 0;
    }
   }
   if (has_noninherited_delrange(tp_self,seq) ||
       has_noninherited_setrange(tp_self,seq)) {
    /* Try to implement delitem using delrange or setrange. */
    DREF DeeObject *start_index;
    start_index = DeeInt_NewSize(start);
    if unlikely(!start_index) goto err;
    if (seq->tp_range_del)
     result = (*seq->tp_range_del)(self,start_index,Dee_None);
    else {
     result = (*seq->tp_range_set)(self,start_index,Dee_None,Dee_None);
    }
    Dee_Decref(start_index);
    return result;
   }
   {
    size_t mylen;
    DREF DeeObject *erase_result;
    mylen = nsi ? (*nsi->nsi_common.nsi_getsize)(self) : DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    /* Search for a type implementing an `erase()' function. */
    if (start > mylen) start = mylen;
    mylen -= start;
    erase_result = call_generic_attribute_in_range(tp_self,
                                                   self,
                                                   &str_erase,
                                                   "IuIu",
                                                   start,
                                                   mylen);
    if (erase_result != ITER_DONE) {
     if unlikely(!erase_result)
        goto err_bad_attribute;
     Dee_Decref(erase_result);
     return 0;
    }
    /* Last chance: Use `operator del[]' to erase each item individually. */
    if (has_noninherited_delitem(tp_self,seq)) {
     while (mylen--) {
      DREF DeeObject *index_ob;
      index_ob = DeeInt_NewSize(start + mylen);
      if unlikely(!index_ob) goto err;
      result = (*seq->tp_del)(self,index_ob);
      Dee_Decref(index_ob);
      if unlikely(result) goto err;
     }
     return 0;
    }
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 return err_immutable_sequence(self);
err_bad_attribute:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
err:
 return -1;
}


#define nsi_hasinsert(x) \
 ((x)->nsi_seqlike.nsi_insert || \
  (x)->nsi_seqlike.nsi_insertall || \
  (x)->nsi_seqlike.nsi_insertvec)

INTDEF DREF DeeObject *DCALL IteratorFuture_For(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL IteratorPending_For(DeeObject *__restrict self);


PRIVATE int DCALL
nsi_insert_iterator(struct type_nsi *__restrict nsi,
                    DeeObject *__restrict self, size_t index,
                    DeeObject *__restrict iterator) {
 if (nsi->nsi_seqlike.nsi_insert) {
  DeeObject *elem; int error;
  while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
   error = (*nsi->nsi_seqlike.nsi_insert)(self,index,elem);
   Dee_Decref(elem);
   if unlikely(error) goto err;
   if (index != (size_t)-1) ++index;
   if (DeeThread_CheckInterrupt())
       goto err;
  }
  if unlikely(!elem) goto err;
 } else if (nsi->nsi_seqlike.nsi_insertvec) {
  DeeObject *elem; int error;
  while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
   error = (*nsi->nsi_seqlike.nsi_insertvec)(self,index,1,&elem);
   Dee_Decref(elem);
   if unlikely(error) goto err;
   if (DeeThread_CheckInterrupt())
       goto err;
  }
  if unlikely(!elem) goto err;
 } else {
  DREF DeeObject *pending; int error;
  ASSERT(nsi->nsi_seqlike.nsi_insertall);
  pending = IteratorPending_For(iterator);
  if unlikely(!pending) goto err;
  error = (*nsi->nsi_seqlike.nsi_insertall)(self,index,pending);
  Dee_Decref(pending);
  return error;
 }
 return 0;
err:
 return -1;
}
PRIVATE int DCALL
nsi_insert_sequence_as_single(struct type_nsi *__restrict nsi,
                              DeeObject *__restrict self, size_t index,
                              DeeObject *__restrict values) {
 DREF DeeObject *iterator,*elem; int temp;
 size_t i,fast_size;
 ASSERT(nsi->nsi_seqlike.nsi_insert);
 fast_size = DeeFastSeq_GetSize(values);
 if (fast_size != DEE_FASTSEQ_NOTFAST) {
  for (i = 0; i < fast_size; ++i) {
   DREF DeeObject *elem;
   elem = DeeFastSeq_GetItem(values,i);
   if unlikely(!elem) goto err;
   temp = (*nsi->nsi_seqlike.nsi_insert)(self,index,elem);
   Dee_Decref(elem);
   if unlikely(temp) goto err;
   if (index != (size_t)-1) ++index;
  }
  return 0;
 }
 iterator = DeeObject_IterSelf(values);
 if unlikely(!iterator) goto err;
 while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
  temp = (*nsi->nsi_seqlike.nsi_insert)(self,index,elem);
  Dee_Decref(elem);
  if unlikely(temp) goto err_iter;
  if (index != (size_t)-1) ++index;
   if (DeeThread_CheckInterrupt())
       goto err_iter;
 }
 if unlikely(elem) goto err_iter;
 Dee_Decref(iterator);
 return 0;
err_iter:
 Dee_Decref(iterator);
err:
 return -1;
}


INTERN int DCALL
DeeSeq_SetRange(DeeObject *__restrict self,
                size_t start, size_t end,
                DeeObject *__restrict values) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 DREF DeeObject *values_iterator;
 DREF DeeObject *callback_result;
 int values_is_empty;
 if (start >= end) {
  /* Use insertall() */
  size_t mylen = DeeObject_Size(self);
  if unlikely(mylen == (size_t)-1) goto err;
  if (start > mylen) start = mylen;
  callback_result = call_generic_attribute_anywhere(self,
                                                    &str_insertall,
                                                    "Iuo",
                                                    start,
                                                    values);
  if (!ITER_ISOK(callback_result)) {
   if (!callback_result) {
    if (DeeError_Catch(&DeeError_NotImplemented) ||
        DeeError_Catch(&DeeError_AttributeError)) {
     size_t values_len;
     values_len = DeeObject_Size(values);
     if unlikely(values_len == (size_t)-1) goto err;
     if (values_len == 0) return 0;
     goto not_resizable;
    }
    goto err;
   }
   goto is_immutable;
  }
  Dee_Decref(callback_result);
  return 0;
 }
 values_is_empty = -1;
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_setrange)
        return (*nsi->nsi_seqlike.nsi_setrange)(self,start,end,values);
    if (nsi->nsi_seqlike.nsi_setitem) {
     DREF DeeObject *elem;
     size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (start > mylen) start = mylen;
     if (end < start) end = start;
     values_iterator = DeeObject_IterSelf(values);
     if unlikely(!values_iterator) goto err;
     while (start < end) {
      elem = DeeObject_IterNext(values_iterator);
      if (!ITER_ISOK(elem)) {
       Dee_Decref(values_iterator);
       if unlikely(!elem) goto err;
       /* Erase the remaining of elements. */
       if (nsi->nsi_seqlike.nsi_erase) {
        size_t temp;
        temp = (*nsi->nsi_seqlike.nsi_erase)(self,start,end - start);
        if unlikely(temp == (size_t)-1) goto err;
       } else {
        goto erase_remainder;
       }
       return 0;
      }
      result = (*nsi->nsi_seqlike.nsi_setitem)(self,start,elem);
      Dee_Decref(elem);
      if unlikely(result) goto err_valiter;
      ++start;
      if (DeeThread_CheckInterrupt())
          goto err_valiter;
     }
     /* Insert the remainder */
     result = nsi_insert_iterator(nsi,self,start,values_iterator);
     Dee_Decref(values_iterator);
     return result;
    }
   }
   if (has_noninherited_setrange(tp_self,seq)) {
    DREF DeeObject *start_index,*end_index;
    start_index = DeeInt_NewSize(start);
    if unlikely(!start_index) goto err;
    end_index   = DeeInt_NewSize(end);
    if unlikely(!end_index) { Dee_Decref(start_index); goto err; }
    result = (*seq->tp_range_set)(self,start_index,end_index,values);
    Dee_Decref(end_index);
    Dee_Decref(start_index);
    return result;
   }
   if (has_noninherited_setitem(tp_self,seq)) {
    DREF DeeObject *future; size_t mylen;
    mylen = DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    if (start > mylen) start = mylen;
    if (end < start) end = start;
    values_iterator = DeeObject_IterSelf(values);
    if unlikely(!values_iterator) goto err;
    /* Override existing / Delete trailing */
    while (start < end) {
     DREF DeeObject *index_ob,*elem;
     elem = DeeObject_IterNext(values_iterator);
     if (!ITER_ISOK(elem)) {
      if unlikely(!elem) goto err_valiter;
      /* Erase all the remaining indices. */
      Dee_Decref(values_iterator);
erase_remainder:
      callback_result = call_generic_attribute_in_range(tp_self,
                                                        self,
                                                        &str_erase,
                                                       "IuIu",
                                                        start,
                                                        end - start);
      if (!ITER_ISOK(callback_result)) {
       if unlikely(!callback_result)
          goto err_attr;
       goto not_resizable;
      }
      Dee_Decref(callback_result);
      return 0;
     }
     index_ob = DeeInt_NewSize(start);
     if unlikely(!index_ob) { Dee_Decref(elem); goto err_valiter; }
     result = (*seq->tp_set)(self,index_ob,elem);
     Dee_Decref(index_ob);
     Dee_Decref(elem);
     if unlikely(result) goto err;
     ++start;
     if (DeeThread_CheckInterrupt())
         goto err;
    }
    ASSERT(start == end);
    /* Insert the remainder. */
    future = IteratorPending_For(values_iterator);
    if unlikely(!future) goto err_valiter;
    callback_result = call_generic_attribute_in_range(tp_self,
                                                      self,
                                                      &str_insertall,
                                                     "Iuo",
                                                      start,
                                                      future);
    Dee_Decref(future);
    if (!ITER_ISOK(callback_result)) {
     DREF DeeObject *trailing;
     if unlikely(!callback_result) goto err_valiter;
     trailing = DeeObject_IterNext(values_iterator);
     Dee_Decref(values_iterator);
     if (trailing == ITER_DONE)
         return 0; /* Empty input iterator -> the caller didn't actually want to re-size the  */
     if (!trailing) goto err;
     Dee_Decref(trailing);
     goto not_resizable;
    }
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (values_is_empty < 0) {
   size_t temp = DeeObject_Size(values);
   if unlikely(temp == (size_t)-1) goto err;
   values_is_empty = temp == 0;
  }
  if (values_is_empty) {
   /* Empty values! */
   if (seq) {
    struct type_nsi *nsi = seq->tp_nsi;
    if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
        is_noninherited_nsi(tp_self,seq,nsi)) {
     if (nsi->nsi_seqlike.nsi_erase) {
      if unlikely((*nsi->nsi_seqlike.nsi_erase)(self,start,end-start) == (size_t)-1)
         goto err;
      return 0;
     }
     ASSERT(!nsi->nsi_seqlike.nsi_setrange);
     if (nsi->nsi_seqlike.nsi_delitem) {
      do {
       --end;
       if unlikely((*nsi->nsi_seqlike.nsi_delitem)(self,end))
          goto err;
      } while (end > start);
      return 0;
     }
    }
    if (has_noninherited_delrange(tp_self,seq)) {
     DREF DeeObject *start_ob,*end_ob;
     start_ob = DeeInt_NewSize(start);
     if unlikely(!start_ob) goto err;
     end_ob = DeeInt_NewSize(end);
     if unlikely(!end_ob) { Dee_Decref(start_ob); goto err; }
     result = (*seq->tp_range_del)(self,start_ob,end_ob);
     Dee_Decref(end_ob);
     Dee_Decref(start_ob);
     return result;
    }
#if 0
    if (has_noninherited_setrange(tp_self,seq)) {
     DREF DeeObject *start_ob,*end_ob;
     start_ob = DeeInt_NewSize(start);
     if unlikely(!start_ob) goto err;
     end_ob = DeeInt_NewSize(end);
     if unlikely(!end_ob) { Dee_Decref(start_ob); goto err; }
     result = (*seq->tp_range_set)(self,start_ob,end_ob,Dee_None);
     Dee_Decref(end_ob);
     Dee_Decref(start_ob);
     return result;
    }
#endif
    if (has_noninherited_delitem(tp_self,seq)) {
     do {
      DREF DeeObject *index_ob;
      --end;
      index_ob = DeeInt_NewSize(end);
      if unlikely(!index_ob) goto err;
      result = (*seq->tp_del)(self,index_ob);
      Dee_Decref(index_ob);
      if unlikely(result) break;
     } while (end > start);
     return result;
    }
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 return err_immutable_sequence(self);
not_resizable:
 return err_fixedlength_sequence(self);
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
 goto err;
err_valiter:
 Dee_Decref(values_iterator);
err:
 return -1;
}

INTERN int DCALL
DeeSeq_SetRangeN(DeeObject *__restrict self, size_t start,
                 DeeObject *__restrict values) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 DREF DeeObject *values_iterator;
 DREF DeeObject *callback_result;
 size_t mylen = DeeObject_Size(self);
 if unlikely(mylen == (size_t)-1) goto err;
 if (start >= mylen) {
  /* Use insertall() */
  callback_result = call_generic_attribute_anywhere(self,
                                                    &str_insertall,
                                                    "Iuo",
                                                    mylen,
                                                    values);
  if (!ITER_ISOK(callback_result)) {
   if (!callback_result) {
    if (DeeError_Catch(&DeeError_NotImplemented) ||
        DeeError_Catch(&DeeError_AttributeError)) {
     size_t values_len;
     values_len = DeeObject_Size(values);
     if unlikely(values_len == (size_t)-1) goto err;
     if (values_len == 0) return 0;
     goto not_resizable;
    }
    goto err;
   }
   goto is_immutable;
  }
  Dee_Decref(callback_result);
  return 0;
 }
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_setrange_n)
        return (*nsi->nsi_seqlike.nsi_setrange_n)(self,start,values);
    if (nsi->nsi_seqlike.nsi_setrange)
        return (*nsi->nsi_seqlike.nsi_setrange)(self,start,mylen,values);
    if (nsi->nsi_seqlike.nsi_setitem) {
     DREF DeeObject *elem;
     values_iterator = DeeObject_IterSelf(values);
     if unlikely(!values_iterator) goto err;
     while (start < mylen) {
      elem = DeeObject_IterNext(values_iterator);
      if (!ITER_ISOK(elem)) {
       Dee_Decref(values_iterator);
       if unlikely(!elem) goto err;
       /* Erase the remaining of elements. */
       if (nsi->nsi_seqlike.nsi_erase) {
        size_t temp;
        temp = (*nsi->nsi_seqlike.nsi_erase)(self,start,mylen - start);
        if unlikely(temp == (size_t)-1) goto err;
       } else {
        goto erase_remainder;
       }
       return 0;
      }
      result = (*nsi->nsi_seqlike.nsi_setitem)(self,start,elem);
      Dee_Decref(elem);
      if unlikely(result) goto err_valiter;
      ++start;
      if (DeeThread_CheckInterrupt())
          goto err_valiter;
     }
     /* Insert the remainder */
     result = nsi_insert_iterator(nsi,self,start,values_iterator);
     Dee_Decref(values_iterator);
     return result;
    }
   }
   if (has_noninherited_setrange(tp_self,seq)) {
    DREF DeeObject *start_index;
    start_index = DeeInt_NewSize(start);
    if unlikely(!start_index) goto err;
    result = (*seq->tp_range_set)(self,start_index,Dee_None,values);
    Dee_Decref(start_index);
    return result;
   }
   if (has_noninherited_setitem(tp_self,seq)) {
    DREF DeeObject *future;
    values_iterator = DeeObject_IterSelf(values);
    if unlikely(!values_iterator) goto err;
    /* Override existing / Delete trailing */
    while (start < mylen) {
     DREF DeeObject *index_ob,*elem;
     elem = DeeObject_IterNext(values_iterator);
     if (!ITER_ISOK(elem)) {
      if unlikely(!elem) goto err_valiter;
      /* Erase all the remaining indices. */
      Dee_Decref(values_iterator);
erase_remainder:
      callback_result = call_generic_attribute_in_range(tp_self,
                                                        self,
                                                        &str_erase,
                                                       "IuIu",
                                                        start,
                                                        mylen - start);
      if (!ITER_ISOK(callback_result)) {
       if unlikely(!callback_result)
          goto err_attr;
       goto not_resizable;
      }
      Dee_Decref(callback_result);
      return 0;
     }
     index_ob = DeeInt_NewSize(start);
     if unlikely(!index_ob) { Dee_Decref(elem); goto err_valiter; }
     result = (*seq->tp_set)(self,index_ob,elem);
     Dee_Decref(index_ob);
     Dee_Decref(elem);
     if unlikely(result) goto err;
     ++start;
     if (DeeThread_CheckInterrupt())
         goto err;
    }
    ASSERT(start == mylen);
    /* Insert the remainder. */
    future = IteratorPending_For(values_iterator);
    if unlikely(!future) goto err_valiter;
    callback_result = call_generic_attribute_in_range(tp_self,
                                                      self,
                                                      &str_insertall,
                                                     "Iuo",
                                                      start,
                                                      future);
    Dee_Decref(future);
    if (!ITER_ISOK(callback_result)) {
     DREF DeeObject *trailing;
     if unlikely(!callback_result) goto err_valiter;
     trailing = DeeObject_IterNext(values_iterator);
     Dee_Decref(values_iterator);
     if (trailing == ITER_DONE)
         return 0; /* Empty input iterator -> the caller didn't actually want to re-size the  */
     if (!trailing) goto err;
     Dee_Decref(trailing);
     goto not_resizable;
    }
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 return err_immutable_sequence(self);
not_resizable:
 return err_fixedlength_sequence(self);
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
 goto err;
err_valiter:
 Dee_Decref(values_iterator);
err:
 return -1;
}



INTERN int DCALL
DeeSeq_Insert(DeeObject *__restrict self, size_t index,
              DeeObject *__restrict value) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_insert)
        return (*nsi->nsi_seqlike.nsi_insert)(self,index,value);
    if (nsi->nsi_seqlike.nsi_insertvec)
        return (*nsi->nsi_seqlike.nsi_insertvec)(self,index,1,(DeeObject **)&value);
    if (nsi->nsi_seqlike.nsi_insertall) {
     DREF DeeObject *value_seq;
     value_seq = DeeTuple_Pack(1,value);
     if unlikely(!value_seq) goto err;
     result = (*nsi->nsi_seqlike.nsi_insertall)(self,index,(DeeObject *)value_seq);
     Dee_Decref(value_seq);
     return result;
    }
    if (nsi->nsi_seqlike.nsi_setrange) {
     DREF DeeObject *value_seq;
     value_seq = DeeTuple_Pack(1,value);
     if unlikely(!value_seq) goto err;
     if ((dssize_t)index < 0) index = SSIZE_MAX;
     result = (*nsi->nsi_seqlike.nsi_setrange)(self,index,index,(DeeObject *)value_seq);
     Dee_Decref(value_seq);
     return result;
    }
   }
  }
  if ((dssize_t)index < 0) {
   /* Use append / extend. */
   DREF DeeObject *callback_result;
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_append),
                                            DeeString_Hash(&str_append),
                                            "o",
                                            value);
   if (callback_result == ITER_DONE) {
    callback_result = call_generic_attribute(tp_self,
                                             self,
                                             DeeString_STR(&str_extend),
                                             DeeString_Hash(&str_extend),
                                             "(o)",
                                             value);
   }
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_attr;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  {
   DREF DeeObject *callback_result;
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_insertall),
                                            DeeString_Hash(&str_insertall),
                                            "Iu(o)",
                                            index,
                                            value);
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_attr;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (seq && has_noninherited_setrange(tp_self,seq)) {
   DREF DeeObject *index_ob,*value_seq;
   if ((dssize_t)index < 0) index = SSIZE_MAX;
   index_ob = DeeInt_NewSize(index);
   if unlikely(!index_ob) goto err;
   value_seq = DeeTuple_Pack(1,value);
   if unlikely(!value_seq) { Dee_Decref(index_ob); goto err; }
   result = (*seq->tp_range_set)(self,index_ob,index_ob,(DeeObject *)value_seq);
   Dee_Decref(value_seq);
   Dee_Decref(index_ob);
   return result;
  }
  if (tp_self->tp_init.tp_assign && (dssize_t)index < 0) {
   DREF DeeObject *items,*concat;
   items = DeeTuple_Pack(1,value);
   if unlikely(!items) goto err;
   concat = DeeSeq_Concat(self,items);
   Dee_Decref(items);
   if unlikely(!concat) goto err;
   result = (*tp_self->tp_init.tp_assign)(self,concat);
   Dee_Decref(concat);
   return result;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
not_resizable:
 return err_fixedlength_sequence(self);
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto not_resizable;
 goto err;
err:
 return -1;
}

INTERN int DCALL
DeeSeq_InsertAll(DeeObject *__restrict self, size_t index,
                 DeeObject *__restrict values) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_insertall)
        return (*nsi->nsi_seqlike.nsi_insertall)(self,index,values);
    if (nsi->nsi_seqlike.nsi_setrange) {
     if ((dssize_t)index < 0) index = SSIZE_MAX;
     return (*nsi->nsi_seqlike.nsi_setrange)(self,index,index,values);
    }
    if (nsi->nsi_seqlike.nsi_insertvec) {
     DREF DeeObject **vector; size_t length;
     if (DeeTuple_Check(values))
         return (*nsi->nsi_seqlike.nsi_insertvec)(self,index,DeeTuple_SIZE(values),DeeTuple_ELEM(values));
     if (nsi->nsi_seqlike.nsi_insert)
         goto do_insert_as_single;
     vector = DeeSeq_AsHeapVector(values,&length);
     if unlikely(!vector) goto err;
     result = (*nsi->nsi_seqlike.nsi_insertvec)(self,index,length,vector);
     while (length--) Dee_Decref(vector[length]);
     Dee_Free(vector);
     return result;
    }
    if (nsi->nsi_seqlike.nsi_insert) {
do_insert_as_single:
     return nsi_insert_sequence_as_single(nsi,self,index,values);
    }
   }
  }
  if ((dssize_t)index < 0) {
   /* Try to call `extend()' */
   DREF DeeObject *callback_result;
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_extend),
                                            DeeString_Hash(&str_extend),
                                            "o",
                                            values);
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_attr;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (seq && has_noninherited_setrange(tp_self,seq)) {
   DREF DeeObject *index_ob;
   if ((dssize_t)index < 0) index = SSIZE_MAX;
   index_ob = DeeInt_NewSize(index);
   if unlikely(!index_ob) goto err;
   result = (*seq->tp_range_set)(self,index_ob,index_ob,values);
   Dee_Decref(index_ob);
   return result;
  }
  if ((dssize_t)index < 0) {
   DREF DeeObject *append_function;
   append_function = get_generic_attribute(tp_self,
                                           self,
                                          &str_append);
   if (append_function != ITER_DONE) {
    size_t i,fast_size; DREF DeeObject *iterator,*elem;
    if unlikely(!append_function) goto err_attr;
    /* Use the append function to append everything */
    fast_size = DeeFastSeq_GetSize(values);
    if (fast_size != DEE_FASTSEQ_NOTFAST) {
     for (i = 0; i < fast_size; ++i) {
      DREF DeeObject *callback_result;
      elem = DeeFastSeq_GetItem(values,i);
      if unlikely(!elem) goto err_append_function;
      callback_result = DeeObject_Call(append_function,1,&elem);
      Dee_Decref(elem);
      if unlikely(!callback_result)
         goto err_append_function;
      Dee_Decref(callback_result);
     }
     Dee_Decref(append_function);
     return 0;
    }
    iterator = DeeObject_IterSelf(values);
    if unlikely(!iterator) goto err_append_function;
    while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
     DREF DeeObject *callback_result;
     callback_result = DeeObject_Call(append_function,1,&elem);
     Dee_Decref(elem);
     if unlikely(!callback_result)
        goto err_append_function_iterator;
     Dee_Decref(callback_result);
     if (DeeThread_CheckInterrupt())
         goto err_append_function_iterator;
    }
    if unlikely(!elem) goto err_append_function_iterator;
    Dee_Decref(iterator);
    Dee_Decref(append_function);
    return 0;
err_append_function_iterator:
    Dee_Decref(iterator);
err_append_function:
    Dee_Decref(append_function);
    goto err;
   }
  }
  {
   DREF DeeObject *insert_function;
   insert_function = get_generic_attribute(tp_self,
                                           self,
                                          &str_insert);
   if (insert_function != ITER_DONE) {
    size_t i,fast_size; DREF DeeObject *iterator,*elem;
    if unlikely(!insert_function) goto err_attr;
    if (index > SSIZE_MAX) index = SSIZE_MAX;
    /* Use the insert function to insert everything */
    fast_size = DeeFastSeq_GetSize(values);
    if (fast_size != DEE_FASTSEQ_NOTFAST) {
     for (i = 0; i < fast_size; ++i) {
      DREF DeeObject *callback_result;
      elem = DeeFastSeq_GetItem(values,i);
      if unlikely(!elem) goto err_insert_function;
      callback_result = DeeObject_Callf(insert_function,"Iuo",index,elem);
      Dee_Decref(elem);
      if unlikely(!callback_result)
         goto err_insert_function;
      Dee_Decref(callback_result);
      if (index < SSIZE_MAX) ++index;
     }
     Dee_Decref(insert_function);
     return 0;
    }
    iterator = DeeObject_IterSelf(values);
    if unlikely(!iterator) goto err_insert_function;
    while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
     DREF DeeObject *callback_result;
     callback_result = DeeObject_Callf(insert_function,"Iuo",index,elem);
     Dee_Decref(elem);
     if unlikely(!callback_result)
        goto err_insert_function_iterator;
     Dee_Decref(callback_result);
     if (index < SSIZE_MAX) ++index;
    }
    if unlikely(!elem) goto err_insert_function_iterator;
    Dee_Decref(iterator);
    Dee_Decref(insert_function);
    return 0;
err_insert_function_iterator:
    Dee_Decref(iterator);
err_insert_function:
    Dee_Decref(insert_function);
    goto err;
   }
  }
  if (tp_self->tp_init.tp_assign && (dssize_t)index < 0) {
   DREF DeeObject *concat;
   concat = DeeSeq_Concat(self,values);
   if unlikely(!concat) goto err;
   result = (*tp_self->tp_init.tp_assign)(self,concat);
   Dee_Decref(concat);
   return result;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
not_resizable:
 return err_fixedlength_sequence(self);
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto not_resizable;
 goto err;
err:
 return -1;
}


INTERN int DCALL
DeeSeq_Append(DeeObject *__restrict self,
              DeeObject *__restrict value) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_insert)
        return (*nsi->nsi_seqlike.nsi_insert)(self,(size_t)-1,value);
    if (nsi->nsi_seqlike.nsi_insertvec)
        return (*nsi->nsi_seqlike.nsi_insertvec)(self,(size_t)-1,1,(DeeObject **)&value);
    if (nsi->nsi_seqlike.nsi_insertall) {
     DREF DeeObject *value_seq;
     value_seq = DeeTuple_Pack(1,value);
     if unlikely(!value_seq) goto err;
     result = (*nsi->nsi_seqlike.nsi_insertall)(self,(size_t)-1,(DeeObject *)value_seq);
     Dee_Decref(value_seq);
     return result;
    }
    if (nsi->nsi_seqlike.nsi_setrange) {
     DREF DeeObject *value_seq;
     value_seq = DeeTuple_Pack(1,value);
     if unlikely(!value_seq) goto err;
     result = (*nsi->nsi_seqlike.nsi_setrange)(self,SSIZE_MAX,SSIZE_MAX,(DeeObject *)value_seq);
     Dee_Decref(value_seq);
     return result;
    }
   }
  }
  {
   /* Use extend. */
   DREF DeeObject *callback_result;
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_extend),
                                            DeeString_Hash(&str_extend),
                                            "(o)",
                                            value);
   if (callback_result == ITER_DONE) {
    callback_result = call_generic_attribute(tp_self,
                                             self,
                                             DeeString_STR(&str_insert),
                                             DeeString_Hash(&str_insert),
                                             "Iuo",
                                            (size_t)SSIZE_MAX,
                                             value);
    if (callback_result == ITER_DONE) {
     callback_result = call_generic_attribute(tp_self,
                                              self,
                                              DeeString_STR(&str_insertall),
                                              DeeString_Hash(&str_insertall),
                                              "Iu(o)",
                                             (size_t)SSIZE_MAX,
                                              value);
    }
   }
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_attr;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (seq && has_noninherited_setrange(tp_self,seq)) {
   DREF DeeObject *index_ob,*value_seq;
   index_ob = DeeInt_NewSize(SSIZE_MAX);
   if unlikely(!index_ob) goto err;
   value_seq = DeeTuple_Pack(1,value);
   if unlikely(!value_seq) { Dee_Decref(index_ob); goto err; }
   result = (*seq->tp_range_set)(self,index_ob,index_ob,(DeeObject *)value_seq);
   Dee_Decref(value_seq);
   Dee_Decref(index_ob);
   return result;
  }
  if (tp_self->tp_init.tp_assign) {
   DREF DeeObject *items,*concat;
   items = DeeTuple_Pack(1,value);
   if unlikely(!items) goto err;
   concat = DeeSeq_Concat(self,items);
   Dee_Decref(items);
   if unlikely(!concat) goto err;
   result = (*tp_self->tp_init.tp_assign)(self,concat);
   Dee_Decref(concat);
   return result;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
not_resizable:
 return err_fixedlength_sequence(self);
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto not_resizable;
 goto err;
err:
 return -1;
}

INTERN int DCALL
DeeSeq_Extend(DeeObject *__restrict self,
              DeeObject *__restrict values) {
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_insertall)
        return (*nsi->nsi_seqlike.nsi_insertall)(self,(size_t)-1,values);
    if (nsi->nsi_seqlike.nsi_setrange)
        return (*nsi->nsi_seqlike.nsi_setrange)(self,SSIZE_MAX,SSIZE_MAX,values);
    if (nsi->nsi_seqlike.nsi_insertvec) {
     DREF DeeObject **vector; size_t length;
     if (DeeTuple_Check(values))
         return (*nsi->nsi_seqlike.nsi_insertvec)(self,(size_t)-1,DeeTuple_SIZE(values),DeeTuple_ELEM(values));
     if (nsi->nsi_seqlike.nsi_insert)
         goto do_insert_as_single;
     vector = DeeSeq_AsHeapVector(values,&length);
     if unlikely(!vector) goto err;
     result = (*nsi->nsi_seqlike.nsi_insertvec)(self,(size_t)-1,length,vector);
     while (length--) Dee_Decref(vector[length]);
     Dee_Free(vector);
     return result;
    }
    if (nsi->nsi_seqlike.nsi_insert) {
do_insert_as_single:
     return nsi_insert_sequence_as_single(nsi,self,(size_t)-1,values);
    }
   }
  }
  {
   /* Try to call `insertall()' */
   DREF DeeObject *callback_result;
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_insertall),
                                            DeeString_Hash(&str_insertall),
                                            "Ido",
                                           (dssize_t)SSIZE_MAX,
                                            values);
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_attr;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (seq && has_noninherited_setrange(tp_self,seq)) {
   DREF DeeObject *index_ob;
   index_ob = DeeInt_NewSize(SSIZE_MAX);
   if unlikely(!index_ob) goto err;
   result = (*seq->tp_range_set)(self,index_ob,index_ob,values);
   Dee_Decref(index_ob);
   return result;
  }
  {
   DREF DeeObject *append_function;
   append_function = get_generic_attribute(tp_self,
                                           self,
                                          &str_append);
   if (append_function != ITER_DONE) {
    size_t i,fast_size; DREF DeeObject *iterator,*elem;
    if unlikely(!append_function) goto err_attr;
    /* Use the append function to append everything */
    fast_size = DeeFastSeq_GetSize(values);
    if (fast_size != DEE_FASTSEQ_NOTFAST) {
     for (i = 0; i < fast_size; ++i) {
      DREF DeeObject *callback_result;
      elem = DeeFastSeq_GetItem(values,i);
      if unlikely(!elem) goto err_append_function;
      callback_result = DeeObject_Call(append_function,1,&elem);
      Dee_Decref(elem);
      if unlikely(!callback_result)
         goto err_append_function;
      Dee_Decref(callback_result);
     }
     Dee_Decref(append_function);
     return 0;
    }
    iterator = DeeObject_IterSelf(values);
    if unlikely(!iterator) goto err_append_function;
    while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
     DREF DeeObject *callback_result;
     callback_result = DeeObject_Call(append_function,1,&elem);
     Dee_Decref(elem);
     if unlikely(!callback_result)
        goto err_append_function_iterator;
     Dee_Decref(callback_result);
     if (DeeThread_CheckInterrupt())
         goto err_append_function_iterator;
    }
    if unlikely(!elem) goto err_append_function_iterator;
    Dee_Decref(iterator);
    Dee_Decref(append_function);
    return 0;
err_append_function_iterator:
    Dee_Decref(iterator);
err_append_function:
    Dee_Decref(append_function);
    goto err;
   }
  }
  {
   DREF DeeObject *insert_function;
   insert_function = get_generic_attribute(tp_self,
                                           self,
                                          &str_insert);
   if (insert_function != ITER_DONE) {
    size_t i,fast_size; DREF DeeObject *iterator,*elem;
    if unlikely(!insert_function) goto err_attr;
    /* Use the insert function to insert everything */
    fast_size = DeeFastSeq_GetSize(values);
    if (fast_size != DEE_FASTSEQ_NOTFAST) {
     for (i = 0; i < fast_size; ++i) {
      DREF DeeObject *callback_result;
      elem = DeeFastSeq_GetItem(values,i);
      if unlikely(!elem) goto err_insert_function;
      callback_result = DeeObject_Callf(insert_function,"Iuo",(size_t)SSIZE_MAX,elem);
      Dee_Decref(elem);
      if unlikely(!callback_result)
         goto err_insert_function;
      Dee_Decref(callback_result);
     }
     Dee_Decref(insert_function);
     return 0;
    }
    iterator = DeeObject_IterSelf(values);
    if unlikely(!iterator) goto err_insert_function;
    while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
     DREF DeeObject *callback_result;
     callback_result = DeeObject_Callf(insert_function,"Iuo",(size_t)SSIZE_MAX,elem);
     Dee_Decref(elem);
     if unlikely(!callback_result)
        goto err_insert_function_iterator;
     Dee_Decref(callback_result);
     if (DeeThread_CheckInterrupt())
         goto err_insert_function_iterator;
    }
    if unlikely(!elem) goto err_insert_function_iterator;
    Dee_Decref(iterator);
    Dee_Decref(insert_function);
    return 0;
err_insert_function_iterator:
    Dee_Decref(iterator);
err_insert_function:
    Dee_Decref(insert_function);
    goto err;
   }
  }
  if (tp_self->tp_init.tp_assign) {
   DREF DeeObject *concat;
   concat = DeeSeq_Concat(self,values);
   if unlikely(!concat) goto err;
   result = (*tp_self->tp_init.tp_assign)(self,concat);
   Dee_Decref(concat);
   return result;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
not_resizable:
 return err_fixedlength_sequence(self);
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto not_resizable;
 goto err;
err:
 return -1;
}


INTERN int DCALL
DeeSeq_InplaceExtend(DREF DeeObject **__restrict pself,
                     DeeObject *__restrict values) {
 DeeObject *self = *pself; int result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 DREF DeeObject *new_self;
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_insertall)
        return (*nsi->nsi_seqlike.nsi_insertall)(self,(size_t)-1,values);
    if (nsi->nsi_seqlike.nsi_setrange)
        return (*nsi->nsi_seqlike.nsi_setrange)(self,SSIZE_MAX,SSIZE_MAX,values);
    if (nsi->nsi_seqlike.nsi_insertvec) {
     DREF DeeObject **vector; size_t length;
     if (DeeTuple_Check(values))
         return (*nsi->nsi_seqlike.nsi_insertvec)(self,(size_t)-1,DeeTuple_SIZE(values),DeeTuple_ELEM(values));
     if (nsi->nsi_seqlike.nsi_insert)
         goto do_insert_as_single;
     vector = DeeSeq_AsHeapVector(values,&length);
     if unlikely(!vector) goto err;
     result = (*nsi->nsi_seqlike.nsi_insertvec)(self,(size_t)-1,length,vector);
     while (length--) Dee_Decref(vector[length]);
     Dee_Free(vector);
     return result;
    }
    if (nsi->nsi_seqlike.nsi_insert) {
do_insert_as_single:
     return nsi_insert_sequence_as_single(nsi,self,(size_t)-1,values);
    }
   }
  }
  {
   DREF DeeObject *callback_result;
   /* Try to call `extend()' */
   callback_result = call_generic_attribute(tp_self,
                                            self,
                                            DeeString_STR(&str_extend),
                                            DeeString_Hash(&str_extend),
                                            "o",
                                            values);
   if (callback_result == ITER_DONE) {
    /* Try to call `insertall()' */
    callback_result = call_generic_attribute(tp_self,
                                             self,
                                             DeeString_STR(&str_insertall),
                                             DeeString_Hash(&str_insertall),
                                             "Ido",
                                            (dssize_t)SSIZE_MAX,
                                             values);
   }
   if (callback_result != ITER_DONE) {
    if unlikely(!callback_result) goto err_attr;
    Dee_Decref(callback_result);
    return 0;
   }
  }
  if (seq && has_noninherited_setrange(tp_self,seq)) {
   DREF DeeObject *index_ob;
   index_ob = DeeInt_NewSize(SSIZE_MAX);
   if unlikely(!index_ob) goto err;
   result = (*seq->tp_range_set)(self,index_ob,index_ob,values);
   Dee_Decref(index_ob);
   return result;
  }
  {
   DREF DeeObject *append_function;
   append_function = get_generic_attribute(tp_self,
                                           self,
                                          &str_append);
   if (append_function != ITER_DONE) {
    size_t i,fast_size; DREF DeeObject *iterator,*elem;
    if unlikely(!append_function) goto err_attr;
    /* Use the append function to append everything */
    fast_size = DeeFastSeq_GetSize(values);
    if (fast_size != DEE_FASTSEQ_NOTFAST) {
     for (i = 0; i < fast_size; ++i) {
      DREF DeeObject *callback_result;
      elem = DeeFastSeq_GetItem(values,i);
      if unlikely(!elem) goto err_append_function;
      callback_result = DeeObject_Call(append_function,1,&elem);
      Dee_Decref(elem);
      if unlikely(!callback_result)
         goto err_append_function;
      Dee_Decref(callback_result);
     }
     Dee_Decref(append_function);
     return 0;
    }
    iterator = DeeObject_IterSelf(values);
    if unlikely(!iterator) goto err_append_function;
    while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
     DREF DeeObject *callback_result;
     callback_result = DeeObject_Call(append_function,1,&elem);
     Dee_Decref(elem);
     if unlikely(!callback_result)
        goto err_append_function_iterator;
     Dee_Decref(callback_result);
     if (DeeThread_CheckInterrupt())
         goto err_append_function_iterator;
    }
    if unlikely(!elem) goto err_append_function_iterator;
    Dee_Decref(iterator);
    Dee_Decref(append_function);
    return 0;
err_append_function_iterator:
    Dee_Decref(iterator);
err_append_function:
    Dee_Decref(append_function);
    goto err;
   }
  }
  {
   DREF DeeObject *insert_function;
   insert_function = get_generic_attribute(tp_self,
                                           self,
                                          &str_insert);
   if (insert_function != ITER_DONE) {
    size_t i,fast_size; DREF DeeObject *iterator,*elem;
    if unlikely(!insert_function) goto err_attr;
    /* Use the insert function to insert everything */
    fast_size = DeeFastSeq_GetSize(values);
    if (fast_size != DEE_FASTSEQ_NOTFAST) {
     for (i = 0; i < fast_size; ++i) {
      DREF DeeObject *callback_result;
      elem = DeeFastSeq_GetItem(values,i);
      if unlikely(!elem) goto err_insert_function;
      callback_result = DeeObject_Callf(insert_function,"Iuo",(size_t)SSIZE_MAX,elem);
      Dee_Decref(elem);
      if unlikely(!callback_result)
         goto err_insert_function;
      Dee_Decref(callback_result);
     }
     Dee_Decref(insert_function);
     return 0;
    }
    iterator = DeeObject_IterSelf(values);
    if unlikely(!iterator) goto err_insert_function;
    while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
     DREF DeeObject *callback_result;
     callback_result = DeeObject_Callf(insert_function,"Iuo",(size_t)SSIZE_MAX,elem);
     Dee_Decref(elem);
     if unlikely(!callback_result)
        goto err_insert_function_iterator;
     Dee_Decref(callback_result);
     if (DeeThread_CheckInterrupt())
         goto err_insert_function_iterator;
    }
    if unlikely(!elem) goto err_insert_function_iterator;
    Dee_Decref(iterator);
    Dee_Decref(insert_function);
    return 0;
err_insert_function_iterator:
    Dee_Decref(iterator);
err_insert_function:
    Dee_Decref(insert_function);
    goto err;
   }
  }
  if (tp_self->tp_init.tp_assign) {
   new_self = DeeSeq_Concat(self,values);
   if unlikely(!new_self) goto err;
   result = (*tp_self->tp_init.tp_assign)(self,new_self);
   Dee_Decref(new_self);
   return result;
  }
  if (tp_self->tp_math && tp_self->tp_math->tp_add) {
   new_self = (*tp_self->tp_math->tp_add)(self,values);
   if unlikely(!new_self) goto err;
   Dee_Decref(self);
   *pself = new_self;
   return 0;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
not_resizable:
 new_self = DeeSeq_Concat(self,values);
set_new_self:
 if unlikely(!new_self) goto err;
 Dee_Decref(self);
 *pself = new_self;
 return 0;
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError)) {
  if (tp_self == &DeeSeq_Type)
      goto not_resizable;
  new_self = DeeObject_Add(self,values);
  goto set_new_self;
 }
err:
 return -1;
}

INTERN int DCALL
DeeSeq_InplaceRepeat(DREF DeeObject **__restrict pself,
                     DeeObject *__restrict count) {
 DeeObject *self = *pself; int result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 DREF DeeObject *new_self;
 size_t integer_count;
 while (tp_self != &DeeSeq_Type) {
  if (tp_self->tp_init.tp_assign) {
   if (DeeObject_AsSize(count,&integer_count))
       goto err;
   if (integer_count == 1) return 0;
   new_self = DeeSeq_Repeat(self,integer_count);
   if unlikely(!new_self) goto err;
   result = (*tp_self->tp_init.tp_assign)(self,new_self);
   Dee_Decref(new_self);
   return result;
  }
  if (tp_self->tp_seq &&
      has_noninherited_setrange(tp_self,tp_self->tp_seq)) {
   if (DeeObject_AsSize(count,&integer_count))
       goto err;
   if (integer_count == 1) return 0;
   new_self = DeeSeq_Repeat(self,integer_count);
   if unlikely(!new_self) goto err;
   result = (*tp_self->tp_seq->tp_range_set)(self,Dee_None,Dee_None,new_self);
   Dee_Decref(new_self);
   return result;
  }
  if (tp_self->tp_math && tp_self->tp_math->tp_mul) {
   new_self = (*tp_self->tp_math->tp_mul)(self,count);
   if unlikely(!new_self) goto err;
   Dee_Decref(self);
   *pself = new_self;
   return 0;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 if (DeeObject_AsSize(count,&integer_count))
     goto err;
 new_self = DeeSeq_Repeat(self,integer_count);
 if unlikely(!new_self) goto err;
 Dee_Decref(self);
 *pself = new_self;
 return 0;
err:
 return -1;
}



INTERN size_t DCALL
DeeSeq_Erase(DeeObject *__restrict self,
             size_t index, size_t count) {
 int error; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_erase)
        return (*nsi->nsi_seqlike.nsi_erase)(self,index,count);
    if (nsi->nsi_seqlike.nsi_setrange) {
     size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (index >= mylen) index = mylen,count = 0;
     else if (index + count > mylen) count = mylen - index;
     if ((*nsi->nsi_seqlike.nsi_setrange)(self,index,index + count,Dee_None))
           goto err;
     return count;
    }
    if (nsi->nsi_seqlike.nsi_delitem) {
     size_t i,mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if unlikely(mylen == (size_t)-1) goto err;
     if (index >= mylen) index = mylen,count = 0;
     else if (index + count > mylen) count = mylen - index;
     if (count) {
      i = index + count;
      do {
       --i;
       if ((*nsi->nsi_seqlike.nsi_delitem)(self,i))
            goto err;
      } while (i > index);
     }
     return count;
    }
   }
   if (has_noninherited_delrange(tp_self,seq) ||
       has_noninherited_setrange(tp_self,seq)) {
    DREF DeeObject *start_index,*end_index;
    size_t mylen = DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    if (index >= mylen) index = mylen,count = 0;
    else if (index + count > mylen) count = mylen - index;
    start_index = DeeInt_NewSize(index);
    if unlikely(!start_index) goto err;
    end_index = DeeInt_NewSize(index + count);
    if unlikely(!end_index) { Dee_Decref(start_index); goto err; }
    error = has_noninherited_delrange(tp_self,seq)
           ? (*seq->tp_range_del)(self,start_index,end_index)
           : (*seq->tp_range_set)(self,start_index,end_index,Dee_None)
           ;
    Dee_Decref(end_index);
    Dee_Decref(start_index);
    if unlikely(error) goto err;
    return count;
   }
   if (has_noninherited_delitem(tp_self,seq)) {
    size_t mylen = DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    if (index >= mylen) index = mylen,count = 0;
    else if (index + count > mylen) count = mylen - index;
    if (count) {
     size_t i = index + count;
     do {
      DREF DeeObject *index_ob;
      --i;
      index_ob = DeeInt_NewSize(i);
      if unlikely(!index_ob) goto err;
      error = (*seq->tp_del)(self,index_ob);
      Dee_Decref(index_ob);
      if unlikely(error) goto err;
     } while (i > index);
    }
    return count;
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 err_fixedlength_sequence(self);
err:
 return (size_t)-1;
}

INTERN DREF DeeObject *DCALL
DeeSeq_PopItem(DeeObject *__restrict self,
               dssize_t index) {
 DREF DeeObject *result; int error;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    /* Check for NSI-optimized variants */
    if (nsi->nsi_seqlike.nsi_pop)
        return (*nsi->nsi_seqlike.nsi_pop)(self,index);
    if (nsi->nsi_seqlike.nsi_delitem) {
     if (nsi->nsi_seqlike.nsi_getitem) {
      if (index < 0) {
       size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
       if unlikely(mylen == (size_t)-1) goto err;
       index += mylen;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem)(self,(size_t)index);
      if unlikely(!result) goto err;
      if unlikely((*nsi->nsi_seqlike.nsi_delitem)(self,(size_t)index)) goto err_r;
      return result;
     }
     if (nsi->nsi_seqlike.nsi_getitem_fast) {
      size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
      if unlikely(mylen == (size_t)-1) goto err;
      if (index < 0) index += mylen;
      else if unlikely((size_t)index >= mylen) {
       err_index_out_of_bounds(self,(size_t)index,mylen);
       goto err;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self,(size_t)index);
      if unlikely(!result) { err_index_unbound(self,(size_t)index); goto err; }
      if unlikely((*nsi->nsi_seqlike.nsi_delitem)(self,(size_t)index)) goto err_r;
      return result;
     }
    }
    if (nsi->nsi_seqlike.nsi_erase) {
     if (nsi->nsi_seqlike.nsi_getitem) {
      if (index < 0) {
       size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
       if unlikely(mylen == (size_t)-1) goto err;
       index += mylen;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem)(self,(size_t)index);
      if unlikely(!result) goto err;
      if unlikely((*nsi->nsi_seqlike.nsi_erase)(self,(size_t)index,1) == (size_t)-1) goto err_r;
      return result;
     }
     if (nsi->nsi_seqlike.nsi_getitem_fast) {
      size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
      if unlikely(mylen == (size_t)-1) goto err;
      if (index < 0) index += mylen;
      else if unlikely((size_t)index >= mylen) {
       err_index_out_of_bounds(self,(size_t)index,mylen);
       goto err;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self,(size_t)index);
      if unlikely(!result) { err_index_unbound(self,(size_t)index); goto err; }
      if unlikely((*nsi->nsi_seqlike.nsi_erase)(self,(size_t)index,1) == (size_t)-1) goto err_r;
      return result;
     }
    }
    if (nsi->nsi_seqlike.nsi_setrange) {
     if (nsi->nsi_seqlike.nsi_getitem) {
      if (index < 0) {
       size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
       if unlikely(mylen == (size_t)-1) goto err;
       index += mylen;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem)(self,(size_t)index);
      if unlikely(!result) goto err;
      if unlikely((*nsi->nsi_seqlike.nsi_setrange)(self,(size_t)index,(size_t)index+1,Dee_None)) goto err_r;
      return result;
     }
     if (nsi->nsi_seqlike.nsi_getitem_fast) {
      size_t mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
      if unlikely(mylen == (size_t)-1) goto err;
      if (index < 0) index += mylen;
      else if unlikely((size_t)index >= mylen) {
       err_index_out_of_bounds(self,(size_t)index,mylen);
       goto err;
      }
      result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self,(size_t)index);
      if unlikely(!result) { err_index_unbound(self,(size_t)index); goto err; }
      if unlikely((*nsi->nsi_seqlike.nsi_setrange)(self,(size_t)index,(size_t)index+1,Dee_None)) goto err_r;
      return result;
     }
    }
   }
   if (has_noninherited_delitem(tp_self,seq)) {
    DREF DeeObject *index_ob;
    if (index < 0) {
     size_t mylen = DeeObject_Size(self);
     if unlikely(mylen == (size_t)-1) goto err;
     index += mylen;
    }
    index_ob = DeeInt_NewSize((size_t)index);
    if unlikely(!index_ob) goto err;
    result = DeeObject_GetItem(self,index_ob);
    if unlikely(!result) { Dee_Decref(index_ob); goto err; }
    error = (*seq->tp_del)(self,index_ob);
    Dee_Decref(index_ob);
    if unlikely(error) goto err_r;
    return result;
   }
   if (has_noninherited_delrange(tp_self,seq) ||
       has_noninherited_setrange(tp_self,seq)) {
    DREF DeeObject *index_ob,*index_plus1_ob;
    if (index < 0) {
     size_t mylen = DeeObject_Size(self);
     if unlikely(mylen == (size_t)-1) goto err;
     index += mylen;
    }
    index_ob = DeeInt_NewSize((size_t)index);
    if unlikely(!index_ob) goto err;
    result = DeeObject_GetItem(self,index_ob);
    if unlikely(!result) { Dee_Decref(index_ob); goto err; }
    index_plus1_ob = DeeInt_NewSize((size_t)index + 1);
    if unlikely(!index_plus1_ob) { Dee_Decref(index_ob); goto err_r; }
    error = has_noninherited_delrange(tp_self,seq)
          ? (*seq->tp_range_del)(self,index_ob,index_plus1_ob)
          : (*seq->tp_range_set)(self,index_ob,index_plus1_ob,Dee_None)
          ;
    Dee_Decref(index_plus1_ob);
    Dee_Decref(index_ob);
    if unlikely(error) goto err_r;
    return result;
   }
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 err_fixedlength_sequence(self);
err:
 return NULL;
err_r:
 Dee_Decref(result);
 goto err;
}

INTERN int DCALL
DeeSeq_Remove(DeeObject *__restrict self,
              size_t start, size_t end,
              DeeObject *__restrict elem,
              DeeObject *pred_eq) {
 DREF DeeObject *item,*callback_result,*erase_func;
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_remove)
        return (*nsi->nsi_seqlike.nsi_remove)(self,start,end,elem,pred_eq);
    if (nsi->nsi_seqlike.nsi_getitem &&
       (nsi->nsi_seqlike.nsi_delitem ||
        nsi->nsi_seqlike.nsi_erase ||
        nsi->nsi_seqlike.nsi_setrange)) {
     size_t i,mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if (end > mylen) end = mylen;
     for (i = start; i < end; ++i) {
      item = (*nsi->nsi_seqlike.nsi_getitem)(self,i);
      if unlikely(!item) goto err;
      if (pred_eq) {
       callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
       if unlikely(!callback_result) goto err_item;
       result = DeeObject_Bool(callback_result);
       Dee_Decref(callback_result);
      } else {
       result = DeeObject_CompareEq(item,elem);
      }
      Dee_Decref(item);
      if (result != 0) {
       if unlikely(result < 0) goto err;
       if (nsi->nsi_seqlike.nsi_delitem) {
        if ((*nsi->nsi_seqlike.nsi_delitem)(self,i))
             goto err;
       } else if (nsi->nsi_seqlike.nsi_erase) {
        if ((*nsi->nsi_seqlike.nsi_erase)(self,i,1) == (size_t)-1)
             goto err;
       } else {
        ASSERT(nsi->nsi_seqlike.nsi_setrange);
        if ((*nsi->nsi_seqlike.nsi_setrange)(self,i,i+1,Dee_None))
             goto err;
       }
       return 1;
      }
     }
     return 0;
    }
   }
   if (has_noninherited_delitem(tp_self,seq)) {
    size_t i,mylen = DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    if (end > mylen) end = mylen;
    for (i = start; i < end; ++i) {
     DREF DeeObject *index_ob;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob_del: Dee_Decref(index_ob); goto err; }
     if (pred_eq) {
      callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
      if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_del; }
      result = DeeObject_Bool(callback_result);
      Dee_Decref(callback_result);
     } else {
      result = DeeObject_CompareEq(item,elem);
     }
     Dee_Decref(item);
     if (result != 0) {
      if unlikely(result < 0) goto err_index_ob_del;
      result = (*seq->tp_del)(self,index_ob);
      if unlikely(result < 0) goto err_index_ob_del;
      Dee_Decref(index_ob);
      return 1;
     }
     Dee_Decref(index_ob);
    }
    return 0;
   }
  }
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_erase);
  if (erase_func != ITER_DONE) {
   size_t i,mylen;
   if unlikely(!erase_func) goto err_attr;
   mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err_erase_func;
   if (end > mylen) end = mylen;
   for (i = start; i < end; ++i) {
    DREF DeeObject *index_ob;
    index_ob = DeeInt_NewSize(i);
    if unlikely(!index_ob) goto err_erase_func;
    item = DeeObject_GetItem(self,index_ob);
    if unlikely(!item) { err_index_ob: Dee_Decref(index_ob); goto err_erase_func; }
    if (pred_eq) {
     callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
     if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob; }
     result = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
    } else {
     result = DeeObject_CompareEq(item,elem);
    }
    Dee_Decref(item);
    if (result != 0) {
     if unlikely(result < 0) goto err_index_ob;
     callback_result = DeeObject_CallPack(erase_func,2,index_ob,&DeeInt_One);
     if unlikely(!callback_result) goto err_index_ob;
     Dee_Decref(callback_result);
     Dee_Decref(index_ob);
     Dee_Decref(erase_func);
     return 1;
    }
    Dee_Decref(index_ob);
   }
   Dee_Decref(erase_func);
   return 0;
  }
  if (seq && has_noninherited_delrange(tp_self,seq)) {
   size_t i,mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err;
   if (end > mylen) end = mylen;
   for (i = start; i < end; ++i) {
    DREF DeeObject *index_ob;
    index_ob = DeeInt_NewSize(i);
    if unlikely(!index_ob) goto err;
    item = DeeObject_GetItem(self,index_ob);
    if unlikely(!item) { err_index_ob_delrange: Dee_Decref(index_ob); goto err; }
    if (pred_eq) {
     callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
     if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_delrange; }
     result = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
    } else {
     result = DeeObject_CompareEq(item,elem);
    }
    Dee_Decref(item);
    if (result != 0) {
     DREF DeeObject *index_plus1_ob;
     if unlikely(result < 0) goto err_index_ob_delrange;
     index_plus1_ob = DeeInt_NewSize(i + 1);
     if unlikely(!index_plus1_ob) goto err_index_ob_delrange;
     result = (*seq->tp_range_del)(self,index_ob,index_plus1_ob);
     Dee_Decref(index_plus1_ob);
     if unlikely(result < 0) goto err_index_ob_delrange;
     Dee_Decref(index_ob);
     return 1;
    }
    Dee_Decref(index_ob);
   }
   return 0;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 err_immutable_sequence(self);
err:
 return -1;
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
 goto err;
err_erase_func:
 Dee_Decref(erase_func);
 goto err;
err_item:
 Dee_Decref(item);
 goto err;
}
INTERN int DCALL
DeeSeq_RRemove(DeeObject *__restrict self,
               size_t start, size_t end,
               DeeObject *__restrict elem,
               DeeObject *pred_eq) {
 DREF DeeObject *item,*callback_result,*erase_func;
 int result; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_rremove)
        return (*nsi->nsi_seqlike.nsi_rremove)(self,start,end,elem,pred_eq);
    if (nsi->nsi_seqlike.nsi_getitem &&
       (nsi->nsi_seqlike.nsi_delitem ||
        nsi->nsi_seqlike.nsi_erase ||
        nsi->nsi_seqlike.nsi_setrange)) {
     size_t i,mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if (end > mylen) end = mylen;
     if (end > start) {
      i = end;
      do {
       --i;
       item = (*nsi->nsi_seqlike.nsi_getitem)(self,i);
       if unlikely(!item) goto err;
       if (pred_eq) {
        callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
        if unlikely(!callback_result) goto err_item;
        result = DeeObject_Bool(callback_result);
        Dee_Decref(callback_result);
       } else {
        result = DeeObject_CompareEq(item,elem);
       }
       Dee_Decref(item);
       if (result != 0) {
        if unlikely(result < 0) goto err;
        if (nsi->nsi_seqlike.nsi_delitem) {
         if ((*nsi->nsi_seqlike.nsi_delitem)(self,i))
              goto err;
        } else if (nsi->nsi_seqlike.nsi_erase) {
         if ((*nsi->nsi_seqlike.nsi_erase)(self,i,1) == (size_t)-1)
              goto err;
        } else {
         ASSERT(nsi->nsi_seqlike.nsi_setrange);
         if ((*nsi->nsi_seqlike.nsi_setrange)(self,i,i+1,Dee_None))
              goto err;
        }
        return 1;
       }
      } while (i > start);
     }
     return 0;
    }
   }
   if (has_noninherited_delitem(tp_self,seq)) {
    size_t i,mylen = DeeObject_Size(self);
    if unlikely(mylen == (size_t)-1) goto err;
    if (end > mylen) end = mylen;
    if (end > start) {
     i = end;
     do {
      DREF DeeObject *index_ob;
      --i;
      index_ob = DeeInt_NewSize(i);
      if unlikely(!index_ob) goto err;
      item = DeeObject_GetItem(self,index_ob);
      if unlikely(!item) { err_index_ob_del: Dee_Decref(index_ob); goto err; }
      if (pred_eq) {
       callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
       if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_del; }
       result = DeeObject_Bool(callback_result);
       Dee_Decref(callback_result);
      } else {
       result = DeeObject_CompareEq(item,elem);
      }
      Dee_Decref(item);
      if (result != 0) {
       if unlikely(result < 0) goto err_index_ob_del;
       result = (*seq->tp_del)(self,index_ob);
       if unlikely(result < 0) goto err_index_ob_del;
       Dee_Decref(index_ob);
       return 1;
      }
      Dee_Decref(index_ob);
     } while (i > start);
    }
    return 0;
   }
  }
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_erase);
  if (erase_func != ITER_DONE) {
   size_t i,mylen;
   if unlikely(!erase_func) goto err_attr;
   mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err_erase_func;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err_erase_func;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob: Dee_Decref(index_ob); goto err_erase_func; }
     if (pred_eq) {
      callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
      if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob; }
      result = DeeObject_Bool(callback_result);
      Dee_Decref(callback_result);
     } else {
      result = DeeObject_CompareEq(item,elem);
     }
     Dee_Decref(item);
     if (result != 0) {
      if unlikely(result < 0) goto err_index_ob;
      callback_result = DeeObject_CallPack(erase_func,2,index_ob,&DeeInt_One);
      if unlikely(!callback_result) goto err_index_ob;
      Dee_Decref(callback_result);
      Dee_Decref(index_ob);
      Dee_Decref(erase_func);
      return 1;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   Dee_Decref(erase_func);
   return 0;
  }
  if (seq && has_noninherited_delrange(tp_self,seq)) {
   size_t i,mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob_delrange: Dee_Decref(index_ob); goto err; }
     if (pred_eq) {
      callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
      if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_delrange; }
      result = DeeObject_Bool(callback_result);
      Dee_Decref(callback_result);
     } else {
      result = DeeObject_CompareEq(item,elem);
     }
     Dee_Decref(item);
     if (result != 0) {
      DREF DeeObject *index_plus1_ob;
      if unlikely(result < 0) goto err_index_ob_delrange;
      index_plus1_ob = DeeInt_NewSize(i + 1);
      if unlikely(!index_plus1_ob) goto err_index_ob_delrange;
      result = (*seq->tp_range_del)(self,index_ob,index_plus1_ob);
      Dee_Decref(index_plus1_ob);
      if unlikely(result < 0) goto err_index_ob_delrange;
      Dee_Decref(index_ob);
      return 1;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   return 0;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 err_immutable_sequence(self);
err:
 return -1;
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
 goto err;
err_erase_func:
 Dee_Decref(erase_func);
 goto err;
err_item:
 Dee_Decref(item);
 goto err;
}

typedef struct {
    OBJECT_HEAD
    DREF DeeObject *ria_elem; /* [1..1][const] The element to compare against. */
    DREF DeeObject *ria_pred; /* [0..1][const] The optional comparison predicate. */
} RemoveIfAllWrapper;

PRIVATE void DCALL
ria_fini(RemoveIfAllWrapper *__restrict self) {
 Dee_Decref(self->ria_elem);
 Dee_XDecref(self->ria_pred);
}
PRIVATE void DCALL
ria_visit(RemoveIfAllWrapper *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->ria_elem);
 Dee_XVisit(self->ria_pred);
}
PRIVATE DREF DeeObject *DCALL
ria_call(RemoveIfAllWrapper *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if unlikely(argc != 1) {
  err_invalid_argc("_removeif_all_wrapper",argc,1,1);
  return NULL;
 }
 if (self->ria_pred)
     return DeeObject_CallPack(self->ria_pred,2,argv[0],self->ria_elem);
 return DeeObject_CompareEqObject(argv[0],self->ria_elem);
}

PRIVATE DeeTypeObject RemoveIfAllWrapper_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_removeif_all_wrapper",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCallable_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(RemoveIfAllWrapper)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ria_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ria_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ria_visit,
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

PRIVATE DREF DeeObject *DCALL
make_removeif_all_wrapper(DeeObject *__restrict elem, DeeObject *pred_eq) {
 /* >> return [](x) -> pred_eq is none ? x == elem : pred_eq(x,elem);
  * So simple, yet sooo complex to implement in C... */
 DREF RemoveIfAllWrapper *result;
 result = DeeObject_MALLOC(RemoveIfAllWrapper);
 if unlikely(!result) goto done;
 result->ria_elem = elem;
 result->ria_pred = pred_eq;
 Dee_Incref(elem);
 Dee_XIncref(pred_eq);
 DeeObject_Init(result,&RemoveIfAllWrapper_Type);
done:
 return (DREF DeeObject *)result;
}



INTERN size_t DCALL
DeeSeq_RemoveAll(DeeObject *__restrict self,
                 size_t start, size_t end,
                 DeeObject *__restrict elem,
                 DeeObject *pred_eq) {
 DREF DeeObject *item,*callback_result,*erase_func;
 int error; size_t count = 0; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_removeall)
        return (*nsi->nsi_seqlike.nsi_removeall)(self,start,end,elem,pred_eq);
    if (nsi->nsi_seqlike.nsi_removeif) {
     DREF DeeObject *wrapper;
     wrapper = make_removeif_all_wrapper(elem,pred_eq);
     if unlikely(!wrapper) goto err;
     count = (*nsi->nsi_seqlike.nsi_removeif)(self,wrapper,start,end);
     Dee_Decref(wrapper);
     return count;
    }
    if (nsi->nsi_seqlike.nsi_remove) {
     while (end > start) {
      error = (*nsi->nsi_seqlike.nsi_remove)(self,start,end,elem,pred_eq);
      if (error <= 0) {
       if unlikely(error < 0) goto err;
       break;
      }
      if unlikely(count == (size_t)-2)
         goto err_overflow;
      ++count;
      --end;
     }
     return count;
    }
    if (nsi->nsi_seqlike.nsi_rremove) {
     while (end > start) {
      error = (*nsi->nsi_seqlike.nsi_rremove)(self,start,end,elem,pred_eq);
      if (error <= 0) {
       if unlikely(error < 0) goto err;
       break;
      }
      if unlikely(count == (size_t)-2)
         goto err_overflow;
      ++count;
      --end;
     }
     return count;
    }
    if (nsi->nsi_seqlike.nsi_getitem &&
       (nsi->nsi_seqlike.nsi_delitem ||
        nsi->nsi_seqlike.nsi_erase ||
        nsi->nsi_seqlike.nsi_setrange)) {
     size_t i,mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if (end > mylen) end = mylen;
     if (end > start) {
      i = end;
      do {
       --i;
       item = (*nsi->nsi_seqlike.nsi_getitem)(self,i);
       if unlikely(!item) goto err;
       if (pred_eq) {
        callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
        if unlikely(!callback_result) goto err_item;
        error = DeeObject_Bool(callback_result);
        Dee_Decref(callback_result);
       } else {
        error = DeeObject_CompareEq(item,elem);
       }
       Dee_Decref(item);
       if (error != 0) {
        if unlikely(error < 0) goto err;
        if (nsi->nsi_seqlike.nsi_delitem) {
         if ((*nsi->nsi_seqlike.nsi_delitem)(self,i))
              goto err;
        } else if (nsi->nsi_seqlike.nsi_erase) {
         if ((*nsi->nsi_seqlike.nsi_erase)(self,i,1) == (size_t)-1)
              goto err;
        } else {
         ASSERT(nsi->nsi_seqlike.nsi_setrange);
         if ((*nsi->nsi_seqlike.nsi_setrange)(self,i,i+1,Dee_None))
              goto err;
        }
        if unlikely(count == (size_t)-2)
           goto err_overflow;
        ++count;
       }
      } while (i > start);
     }
     return count;
    }
   }
  }
  /* >> `self.removeif([](x) -> pred_eq is none ? x == elem : pred_eq(x,elem),start,end)' */
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_removeif);
  if (erase_func != ITER_DONE) {
   if (end > start) {
    DREF DeeObject *removeif_argv[3];
    removeif_argv[0] = make_removeif_all_wrapper(elem,pred_eq);
    if unlikely(!removeif_argv[0]) goto err;
    removeif_argv[1] = DeeInt_NewSize(start);
    if unlikely(!removeif_argv[1]) { err_removeif_argv_0: Dee_Decref(removeif_argv[0]); goto err; }
    removeif_argv[2] = DeeInt_NewSize(end);
    if unlikely(!removeif_argv[2]) { /*err_removeif_argv_1:*/ Dee_Decref(removeif_argv[1]); goto err_removeif_argv_0; }
    callback_result = DeeObject_Call(erase_func,3,removeif_argv);
    Dee_Decref(removeif_argv[2]);
    Dee_Decref(removeif_argv[1]);
    Dee_Decref(removeif_argv[0]);
    if unlikely(!callback_result) goto err;
    error = DeeObject_AsSize(callback_result,&count);
    Dee_Decref(callback_result);
    if unlikely(error) goto err;
   }
   return count;
  }
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_rremove);
  if (erase_func == ITER_DONE) {
   erase_func = get_generic_attribute(tp_self,
                                      self,
                                     &str_remove);
  }
  if (erase_func != ITER_DONE) {
   if unlikely(!erase_func) goto err_attr;
   if (end > start) {
    DREF DeeObject *remove_argv[4]; size_t remove_argc;
    remove_argv[0] = elem;
    remove_argv[3] = pred_eq;
    remove_argc    = pred_eq ? 4 : 3;
    remove_argv[1] = DeeInt_NewSize(start);
    if unlikely(!remove_argv[1]) goto err_erase_func;
    while (end > start) {
     remove_argv[2] = DeeInt_NewSize(end);
     if unlikely(!remove_argv[2]) { err_remove_argv: Dee_Decref(remove_argv[1]); goto err_erase_func; }
     callback_result = DeeObject_Call(erase_func,remove_argc,remove_argv);
     Dee_Decref(remove_argv[2]);
     if unlikely(!callback_result) goto err_remove_argv;
     error = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
     if (error <= 0) {
      if unlikely(error < 0) goto err_remove_argv;
      break;
     }
     if unlikely(count == (size_t)-2) {
      Dee_Decref(remove_argv[1]);
      Dee_Decref(erase_func);
      goto err_overflow;
     }
     ++count;
     --end;
    }
    Dee_Decref(remove_argv[1]);
   }
   Dee_Decref(erase_func);
   return count;
  }
  if (seq && has_noninherited_delitem(tp_self,seq)) {
   size_t i,mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob_del: Dee_Decref(index_ob); goto err; }
     if (pred_eq) {
      callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
      if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_del; }
      error = DeeObject_Bool(callback_result);
      Dee_Decref(callback_result);
     } else {
      error = DeeObject_CompareEq(item,elem);
     }
     Dee_Decref(item);
     if (error != 0) {
      if unlikely(error < 0) goto err_index_ob_del;
      error = (*seq->tp_del)(self,index_ob);
      if unlikely(error < 0) goto err_index_ob_del;
      if unlikely(count == (size_t)-2) {
       Dee_Decref(index_ob);
       goto err_overflow;
      }
      ++count;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   return count;
  }
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_erase);
  if (erase_func != ITER_DONE) {
   size_t i,mylen;
   if unlikely(!erase_func) goto err_attr;
   mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err_erase_func;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err_erase_func;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob: Dee_Decref(index_ob); goto err_erase_func; }
     if (pred_eq) {
      callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
      if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob; }
      error = DeeObject_Bool(callback_result);
      Dee_Decref(callback_result);
     } else {
      error = DeeObject_CompareEq(item,elem);
     }
     Dee_Decref(item);
     if (error != 0) {
      if unlikely(error < 0) goto err_index_ob;
      callback_result = DeeObject_CallPack(erase_func,2,index_ob,&DeeInt_One);
      if unlikely(!callback_result) goto err_index_ob;
      Dee_Decref(callback_result);
      if unlikely(count == (size_t)-2) {
       Dee_Decref(index_ob);
       Dee_Decref(erase_func);
       goto err_overflow;
      }
      ++count;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   Dee_Decref(erase_func);
   return count;
  }
  if (seq && has_noninherited_delrange(tp_self,seq)) {
   size_t i,mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob_delrange: Dee_Decref(index_ob); goto err; }
     if (pred_eq) {
      callback_result = DeeObject_CallPack(pred_eq,2,item,elem);
      if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_delrange; }
      error = DeeObject_Bool(callback_result);
      Dee_Decref(callback_result);
     } else {
      error = DeeObject_CompareEq(item,elem);
     }
     Dee_Decref(item);
     if (error != 0) {
      DREF DeeObject *index_plus1_ob;
      if unlikely(error < 0) goto err_index_ob_delrange;
      index_plus1_ob = DeeInt_NewSize(i + 1);
      if unlikely(!index_plus1_ob) goto err_index_ob_delrange;
      error = (*seq->tp_range_del)(self,index_ob,index_plus1_ob);
      Dee_Decref(index_plus1_ob);
      if unlikely(error < 0) goto err_index_ob_delrange;
      if unlikely(count == (size_t)-2) {
       Dee_Decref(index_ob);
       goto err_overflow;
      }
      ++count;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   return count;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 err_immutable_sequence(self);
err:
 return (size_t)-1;
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
 goto err;
err_erase_func:
 Dee_Decref(erase_func);
 goto err;
err_item:
 Dee_Decref(item);
 goto err;
err_overflow:
 err_integer_overflow_i(sizeof(size_t)*8,true);
 goto err;
}


typedef struct {
    OBJECT_HEAD
    DREF DeeObject *rai_should; /* [1..1][const] The function determining if the attribute should be removed. */
} RemoveAllIfWrapper;

PRIVATE void DCALL
rai_fini(RemoveAllIfWrapper *__restrict self) {
 Dee_Decref(self->rai_should);
}
PRIVATE void DCALL
rai_visit(RemoveAllIfWrapper *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->rai_should);
}
PRIVATE DREF DeeObject *DCALL
rai_call(RemoveAllIfWrapper *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if unlikely(argc != 2) {
  err_invalid_argc("_removeall_if_wrapper",argc,2,2);
  return NULL;
 }
 return DeeObject_Call(self->rai_should,1,argv);
}

PRIVATE DeeTypeObject RemoveAllIfWrapper_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_removeall_if_wrapper",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCallable_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(RemoveAllIfWrapper)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&rai_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&rai_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&rai_visit,
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

PRIVATE DREF DeeObject *DCALL
make_removeall_if_wrapper(DeeObject *__restrict should) {
 /* >> return [](x,y) -> should(x);
  * So simple, yet sooo complex to implement in C... */
 DREF RemoveAllIfWrapper *result;
 result = DeeObject_MALLOC(RemoveAllIfWrapper);
 if unlikely(!result) goto done;
 result->rai_should = should;
 Dee_Incref(should);
 DeeObject_Init(result,&RemoveAllIfWrapper_Type);
done:
 return (DREF DeeObject *)result;
}


INTERN size_t DCALL
DeeSeq_RemoveIf(DeeObject *__restrict self,
                DeeObject *__restrict should,
                size_t start, size_t end) {
 DREF DeeObject *item,*callback_result,*erase_func,*wrapper;
 int error; size_t count = 0; DeeTypeObject *tp_self = Dee_TYPE(self);
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq;
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_removeall);
  if (erase_func != ITER_DONE) {
   /* Forward the call to the `removeall()' function. */
   DREF DeeObject *remove_argv[4];
   if unlikely(!erase_func) goto err_attr;
   wrapper = make_removeall_if_wrapper(should);
   if unlikely(!wrapper) goto err_erase_func;
   remove_argv[0] = Dee_None;
   remove_argv[1] = DeeInt_NewSize(start);
   if unlikely(!remove_argv[1]) goto err_erase_func_wrapper;
   remove_argv[2] = DeeInt_NewSize(end);
   if unlikely(!remove_argv[2]) { Dee_Decref(remove_argv[1]); goto err_erase_func_wrapper; }
   remove_argv[3] = wrapper;
   callback_result = DeeObject_Call(erase_func,4,remove_argv);
   Dee_Decref(remove_argv[2]);
   Dee_Decref(remove_argv[1]);
   Dee_Decref(wrapper);
   if unlikely(!callback_result) goto err;
   error = DeeObject_AsSize(callback_result,&count);
   Dee_Decref(callback_result);
   if unlikely(error) goto err;
   return count;
  }

  seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
       is_noninherited_nsi(tp_self,seq,nsi)) {
    if (nsi->nsi_seqlike.nsi_removeif)
        return (*nsi->nsi_seqlike.nsi_removeif)(self,should,start,end);
    if (nsi->nsi_seqlike.nsi_removeall) {
     wrapper = make_removeall_if_wrapper(should);
     if unlikely(!wrapper) goto err;
     count = (*nsi->nsi_seqlike.nsi_removeall)(self,start,end,Dee_None,wrapper);
     Dee_Decref(wrapper);
     return count;
    }
    if (nsi->nsi_seqlike.nsi_remove) {
     wrapper = make_removeall_if_wrapper(should);
     if unlikely(!wrapper) goto err;
     while (end > start) {
      error = (*nsi->nsi_seqlike.nsi_remove)(self,start,end,Dee_None,wrapper);
      if (error <= 0) {
       if unlikely(error < 0) goto err_wrapper;
       break;
      }
      if unlikely(count == (size_t)-2)
         goto err_overflow_wrapper;
      ++count;
      --end;
     }
     Dee_Decref(wrapper);
     return count;
    }
    if (nsi->nsi_seqlike.nsi_rremove) {
     wrapper = make_removeall_if_wrapper(should);
     if unlikely(!wrapper) goto err;
     while (end > start) {
      error = (*nsi->nsi_seqlike.nsi_rremove)(self,start,end,Dee_None,wrapper);
      if (error <= 0) {
       if unlikely(error < 0) goto err_wrapper;
       break;
      }
      if unlikely(count == (size_t)-2)
         goto err_overflow_wrapper;
      ++count;
      --end;
     }
     Dee_Decref(wrapper);
     return count;
    }
    if (nsi->nsi_seqlike.nsi_getitem &&
       (nsi->nsi_seqlike.nsi_delitem ||
        nsi->nsi_seqlike.nsi_erase ||
        nsi->nsi_seqlike.nsi_setrange)) {
     size_t i,mylen = (*nsi->nsi_seqlike.nsi_getsize)(self);
     if (end > mylen) end = mylen;
     if (end > start) {
      i = end;
      do {
       --i;
       item = (*nsi->nsi_seqlike.nsi_getitem)(self,i);
       if unlikely(!item) goto err;
       callback_result = DeeObject_Call(should,1,&item);
       if unlikely(!callback_result) goto err_item;
       error = DeeObject_Bool(callback_result);
       Dee_Decref(callback_result);
       Dee_Decref(item);
       if (error != 0) {
        if unlikely(error < 0) goto err;
        if (nsi->nsi_seqlike.nsi_delitem) {
         if ((*nsi->nsi_seqlike.nsi_delitem)(self,i))
              goto err;
        } else if (nsi->nsi_seqlike.nsi_erase) {
         if ((*nsi->nsi_seqlike.nsi_erase)(self,i,1) == (size_t)-1)
              goto err;
        } else {
         ASSERT(nsi->nsi_seqlike.nsi_setrange);
         if ((*nsi->nsi_seqlike.nsi_setrange)(self,i,i+1,Dee_None))
              goto err;
        }
        if unlikely(count == (size_t)-2)
           goto err_overflow;
        ++count;
       }
      } while (i > start);
     }
     return count;
    }
   }
  }
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_rremove);
  if (erase_func == ITER_DONE) {
   erase_func = get_generic_attribute(tp_self,
                                      self,
                                     &str_remove);
  }
  if (erase_func != ITER_DONE) {
   if unlikely(!erase_func) goto err_attr;
   if (end > start) {
    DREF DeeObject *remove_argv[4];
    wrapper = make_removeall_if_wrapper(should);
    if unlikely(!wrapper) goto err;
    remove_argv[0] = Dee_None;
    remove_argv[3] = wrapper;
    remove_argv[1] = DeeInt_NewSize(start);
    if unlikely(!remove_argv[1]) goto err_erase_func_wrapper;
    while (end > start) {
     remove_argv[2] = DeeInt_NewSize(end);
     if unlikely(!remove_argv[2]) { err_remove_argv_wraper: Dee_Decref(remove_argv[1]); goto err_erase_func_wrapper; }
     callback_result = DeeObject_Call(erase_func,4,remove_argv);
     Dee_Decref(remove_argv[2]);
     if unlikely(!callback_result) goto err_remove_argv_wraper;
     error = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
     if (error <= 0) {
      if unlikely(error < 0) goto err_remove_argv_wraper;
      break;
     }
     if unlikely(count == (size_t)-2) {
      Dee_Decref(remove_argv[1]);
      Dee_Decref(erase_func);
      goto err_overflow_wrapper;
     }
     ++count;
     --end;
    }
    Dee_Decref(remove_argv[1]);
    Dee_Decref(wrapper);
   }
   Dee_Decref(erase_func);
   return count;
  }
  if (seq && has_noninherited_delitem(tp_self,seq)) {
   size_t i,mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob_del: Dee_Decref(index_ob); goto err; }
     callback_result = DeeObject_Call(should,1,&item);
     if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_del; }
     error = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
     Dee_Decref(item);
     if (error != 0) {
      if unlikely(error < 0) goto err_index_ob_del;
      error = (*seq->tp_del)(self,index_ob);
      if unlikely(error < 0) goto err_index_ob_del;
      if unlikely(count == (size_t)-2) {
       Dee_Decref(index_ob);
       goto err_overflow;
      }
      ++count;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   return count;
  }
  erase_func = get_generic_attribute(tp_self,
                                     self,
                                    &str_erase);
  if (erase_func != ITER_DONE) {
   size_t i,mylen;
   if unlikely(!erase_func) goto err_attr;
   mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err_erase_func;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err_erase_func;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob: Dee_Decref(index_ob); goto err_erase_func; }
     callback_result = DeeObject_Call(should,1,&item);
     if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob; }
     error = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
     Dee_Decref(item);
     if (error != 0) {
      if unlikely(error < 0) goto err_index_ob;
      callback_result = DeeObject_CallPack(erase_func,2,index_ob,&DeeInt_One);
      if unlikely(!callback_result) goto err_index_ob;
      Dee_Decref(callback_result);
      if unlikely(count == (size_t)-2) {
       Dee_Decref(index_ob);
       Dee_Decref(erase_func);
       goto err_overflow;
      }
      ++count;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   Dee_Decref(erase_func);
   return count;
  }
  if (seq && has_noninherited_delrange(tp_self,seq)) {
   size_t i,mylen = DeeObject_Size(self);
   if unlikely(mylen == (size_t)-1) goto err;
   if (end > mylen) end = mylen;
   if (end > start) {
    i = end;
    do {
     DREF DeeObject *index_ob;
     --i;
     index_ob = DeeInt_NewSize(i);
     if unlikely(!index_ob) goto err;
     item = DeeObject_GetItem(self,index_ob);
     if unlikely(!item) { err_index_ob_delrange: Dee_Decref(index_ob); goto err; }
     callback_result = DeeObject_Call(should,1,&item);
     if unlikely(!callback_result) { Dee_Decref(item); goto err_index_ob_delrange; }
     error = DeeObject_Bool(callback_result);
     Dee_Decref(callback_result);
     Dee_Decref(item);
     if (error != 0) {
      DREF DeeObject *index_plus1_ob;
      if unlikely(error < 0) goto err_index_ob_delrange;
      index_plus1_ob = DeeInt_NewSize(i + 1);
      if unlikely(!index_plus1_ob) goto err_index_ob_delrange;
      error = (*seq->tp_range_del)(self,index_ob,index_plus1_ob);
      Dee_Decref(index_plus1_ob);
      if unlikely(error < 0) {
       Dee_Decref(index_ob);
       goto err_index_ob_delrange;
      }
      if unlikely(count == (size_t)-2) {
       Dee_Decref(index_ob);
       goto err_overflow;
      }
      ++count;
     }
     Dee_Decref(index_ob);
    } while (i > start);
   }
   return count;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
is_immutable:
 err_immutable_sequence(self);
err:
 return (size_t)-1;
err_attr:
 if (DeeError_Catch(&DeeError_NotImplemented) ||
     DeeError_Catch(&DeeError_AttributeError))
     goto is_immutable;
 goto err;
err_wrapper:
 Dee_Decref(wrapper);
 goto err;
err_erase_func_wrapper:
 Dee_Decref(wrapper);
err_erase_func:
 Dee_Decref(erase_func);
 goto err;
err_item:
 Dee_Decref(item);
 goto err;
err_overflow_wrapper:
 Dee_Decref(wrapper);
err_overflow:
 err_integer_overflow_i(sizeof(size_t)*8,true);
 goto err;
}

INTERN size_t DCALL
DeeSeq_Fill(DeeObject *__restrict self,
            size_t start, size_t end,
            DeeObject *__restrict value) {
 DREF DeeObject *temp; size_t mylen,result;
 int error; DeeTypeObject *tp_self = Dee_TYPE(self);
 if (start >= end) return 0;
 mylen = DeeObject_Size(self);
 if unlikely(mylen == (size_t)-1) goto err;
 if (start >= mylen) return 0;
 if (end > mylen) end = mylen;
 ASSERT(start < end);
 result = end - start;
 if unlikely(result == (size_t)-1) goto err_overflow;
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq) {
   struct type_nsi *nsi = seq->tp_nsi;
   if (seq->tp_range_set) {
    /* >> this[start:end] = sequence.repeat(value,end - start); */
    DREF DeeObject *repeated_value;
    repeated_value = DeeSeq_RepeatItem(value,result);
    if unlikely(!repeated_value) goto err;
    if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
        nsi->nsi_seqlike.nsi_setrange && end <= SSIZE_MAX) {
     error = (*nsi->nsi_seqlike.nsi_setrange)(self,(dssize_t)start,(dssize_t)end,repeated_value);
    } else {
     DREF DeeObject *start_ob;
     DREF DeeObject *end_ob;
     start_ob = DeeInt_NewSize(start);
     if unlikely(!start_ob) { err_repeated_value: Dee_Decref(repeated_value); goto err; }
     end_ob = DeeInt_NewSize(end);
     if unlikely(!end_ob) { Dee_Decref(start_ob); goto err_repeated_value; }
     error = (*nsi->nsi_seqlike.nsi_setrange)(self,(dssize_t)start,(dssize_t)end,repeated_value);
     Dee_Decref(end_ob);
     Dee_Decref(start_ob);
    }
    Dee_Decref(repeated_value);
    if unlikely(error) goto err;
    return result;
   }
   if (seq->tp_set) {
    /* >> for (local i = start; i < end; ++i)
     * >>     this[i] = value; */
    size_t i;
    if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
        nsi->nsi_seqlike.nsi_setitem) {
     for (i = start; i < end; ++i) {
      error = (*nsi->nsi_seqlike.nsi_setitem)(self,i,value);
      if unlikely(error) goto err;
     }
    } else {
     for (i = start; i < end; ++i) {
      temp = DeeInt_NewSize(i);
      if unlikely(!temp) goto err;
      error = (*seq->tp_set)(self,temp,value);
      Dee_Decref(temp);
      if unlikely(error) goto err;
     }
    }
    return result;
   }
   break;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 err_immutable_sequence(self);
err:
 return (size_t)-1;
err_overflow:
 err_integer_overflow_i(sizeof(size_t)*8,true);
 return (size_t)-1;
}

INTERN int DCALL
DeeSeq_Reverse(DeeObject *__restrict self) {
 DREF DeeObject *reversed; int result;
 /* TODO: This breaks when `reversed' is an index-based proxy! */
 reversed = DeeSeq_Reversed(self);
 if unlikely(!reversed) goto err;
 result = DeeObject_Assign(self,reversed);
 Dee_Decref(reversed);
 return result;
err:
 return -1;
}
INTERN int DCALL
DeeSeq_Sort(DeeObject *__restrict self, DeeObject *pred_lo) {
 DREF DeeObject *sorted; int result;
 /* TODO: This breaks when `sorted' is an index-based proxy! */
 sorted = DeeSeq_Sorted(self,pred_lo);
 if unlikely(!sorted) goto err;
 result = DeeObject_Assign(self,sorted);
 Dee_Decref(sorted);
 return result;
err:
 return -1;
}



PRIVATE DeeObject *mutable_sequence_attributes[] = {
    &str_remove,
    &str_rremove,
    &str_removeall,
    &str_removeif,
    &str_xch,
    &str_clear,
#define resizable_sequence_attributes \
       (mutable_sequence_attributes + 6)
#define resizable_sequence_attributes_count \
       (COMPILER_LENOF(mutable_sequence_attributes) - 6)

    &str_pop,
    &str_append,
    &str_extend,
    &str_insert,
    &str_insertall,
    &str_erase,
    &str_resize,
    &str_pushfront,
    &str_pushback,
    &str_popfront,
    &str_popback
};


INTERN int DCALL
DeeSeq_IsMutable(DeeObject *__restrict self) {
 DeeTypeObject *tp_self;
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq; size_t i;
  if ((seq = tp_self->tp_seq) != NULL) {
   if (seq->tp_nsi)
       return (seq->tp_nsi->nsi_flags & TYPE_SEQX_FMUTABLE) ? 1 : 0;
   if (seq->tp_del || seq->tp_set ||
       seq->tp_range_del || seq->tp_range_set)
       return 1;
  }
  for (i = 0; i < COMPILER_LENOF(mutable_sequence_attributes); ++i) {
   int temp = has_generic_attribute(tp_self,self,mutable_sequence_attributes[i]);
   if (temp != 0) return temp;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 return 0;
}
INTERN int DCALL
DeeSeq_IsResizable(DeeObject *__restrict self) {
 DeeTypeObject *tp_self;
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (tp_self != &DeeSeq_Type) {
  struct type_seq *seq; size_t i;
  if ((seq = tp_self->tp_seq) != NULL && seq->tp_nsi)
      return (seq->tp_nsi->nsi_flags & TYPE_SEQX_FRESIZABLE) ? 1 : 0;
  for (i = 0; i < resizable_sequence_attributes_count; ++i) {
   int temp = has_generic_attribute(tp_self,self,resizable_sequence_attributes[i]);
   if (temp != 0) return temp;
  }
  if ((tp_self = DeeType_Base(tp_self)) == NULL) break;
 }
 return 0;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MUTABLE_C */
