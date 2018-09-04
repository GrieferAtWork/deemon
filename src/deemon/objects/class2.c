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
#ifndef GUARD_DEEMON_OBJECTS_CLASS2_C
#define GUARD_DEEMON_OBJECTS_CLASS2_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/none.h>
#include <deemon/gc.h>
#include <deemon/string.h>
#include <deemon/error.h>

#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
DECL_BEGIN

PRIVATE DeeObject *DCALL
class_desc_get_known_operator(struct class_desc *__restrict self,
                              uint16_t name) {
 DREF DeeObject *result;
 if (name < CLASS_OPERATOR_USERCOUNT) {
  struct class_optable *table;
  table = self->cd_ops[name / CLASS_HEADER_OPC2];
  if likely(table) {
   result = table->co_operators[name % CLASS_HEADER_OPC2];
   if likely(result)
      return_reference_(result);

  }

 }


}




/* Initialize the super-class instances
 * of `self' as shallow copies of `other'. */
INTERN int DCALL
initialize_super_copy(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      DeeObject *__restrict other) {
 (void)tp_self;
 (void)self;
 (void)other;
 DERROR_NOTIMPLEMENTED();
 return -1;
}

/* Initialize the super-class instances
 * of `self' as deep copies of `other'. */
INTERN int DCALL
initialize_super_deepcopy(DeeTypeObject *__restrict tp_self,
                          DeeObject *__restrict self,
                          DeeObject *__restrict other) {
 (void)tp_self;
 (void)self;
 (void)other;
 DERROR_NOTIMPLEMENTED();
 return -1;
}

INTERN int DCALL
finalize_super(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
 /* TODO */

}


/* Constructor hooks when the user-class defines a `CLASS_OPERATOR_SUPERARGS' operator. */
INTERN int DCALL instance_builtin_super_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTERN int DCALL instance_builtin_super_ctor(DeeObject *__restrict self);
INTERN int DCALL instance_builtin_super_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_builtin_super_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_builtin_super_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTERN int DCALL instance_builtin_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTERN int DCALL instance_super_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTERN int DCALL instance_super_ctor(DeeObject *__restrict self);
INTERN int DCALL instance_super_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_super_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_super_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTERN int DCALL instance_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* Constructor hooks when the user-class doesn't define a `CLASS_OPERATOR_SUPERARGS' operator. */
INTERN int DCALL instance_builtin_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTERN int DCALL instance_builtin_ctor(DeeObject *__restrict self);
INTERN int DCALL instance_builtin_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_builtin_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_builtin_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTERN int DCALL instance_builtin_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTERN int DCALL instance_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTERN int DCALL instance_ctor(DeeObject *__restrict self);
INTERN int DCALL instance_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTERN int DCALL instance_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTERN int DCALL instance_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* Builtin (pre-defined) hooks that are used when the user-class doesn't override these operators. */
INTERN int DCALL instance_builtin_tcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTERN int DCALL instance_builtin_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTERN int DCALL instance_builtin_tdeepload(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTERN int DCALL instance_builtin_deepload(DeeObject *__restrict self);
INTERN void DCALL instance_builtin_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */
INTERN int DCALL instance_builtin_tassign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTERN int DCALL instance_builtin_assign(DeeObject *__restrict self, DeeObject *__restrict other);
INTERN int DCALL instance_builtin_tmoveassign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTERN int DCALL instance_builtin_moveassign(DeeObject *__restrict self, DeeObject *__restrict other);

/* Hooks when the user-class overrides the associated operator. */
INTERN int DCALL
instance_tcopy(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict other) {
 struct class_desc *desc = DeeClass_DESC(tp_self);
 DeeObject *func;
 func = class_desc_get_known_operator(desc,OPERATOR_COPY);
 if unlikely(!func) goto err;
 if unlikely(initialize_super_copy(tp_self,self,other)) goto err;



err:
 return -1;
}

INTERN int DCALL
instance_tdeepcopy(DeeTypeObject *__restrict tp_self,
                   DeeObject *__restrict self,
                   DeeObject *__restrict other) {


}


INTERN void DCALL
instance_destructor(DeeObject *__restrict self) {
 DREF DeeObject *callback;
 callback = DeeClass_TryGetPrivateOperator(Dee_TYPE(self),
                                           OPERATOR_DESTRUCTOR);
 if likely(callback) {
  DREF DeeObject *dtor_result;
  dtor_result = DeeObject_ThisCall(callback,self,0,NULL);
  Dee_Decref(callback);
  if likely(dtor_result) {
   Dee_Decref(dtor_result);
  } else {
   DeeError_Print("Unhandled error in destructor\n",
                  ERROR_PRINT_DOHANDLE);
  }
 }
 /* Forward to the builtin destructor callback. */
 instance_builtin_destructor(self);
}

INTERN int DCALL
instance_copy(DeeObject *__restrict self,
              DeeObject *__restrict other) {
 return instance_tcopy(Dee_TYPE(self),self,other);
}
INTERN int DCALL
instance_deepcopy(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
 return instance_tdeepcopy(Dee_TYPE(self),self,other);
}


#define DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_txxx,instance_xxx,op) \
INTERN int DCALL \
instance_txxx(DeeTypeObject *__restrict tp_self, \
              DeeObject *__restrict self, \
              DeeObject *__restrict other) { \
 DeeObject *func,*result; \
 func = DeeClass_GetOperator(tp_self,op); \
 if unlikely(!func) goto err; \
 result = DeeObject_ThisCall(func,self,1,(DeeObject **)&other); \
 if unlikely(!result) goto err; \
 Dee_Decref(result); \
 return 0; \
err: \
 return -1; \
} \
INTERN int DCALL \
instance_xxx(DeeObject *__restrict self, \
             DeeObject *__restrict other) { \
 return instance_txxx(Dee_TYPE(self),self,other); \
}
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tassign,instance_assign,OPERATOR_ASSIGN)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tmoveassign,instance_moveassign,OPERATOR_MOVEASSIGN)
#undef DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT


#define DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_txxx,instance_xxx,op) \
INTERN DREF DeeObject *DCALL \
instance_txxx(DeeTypeObject *__restrict tp_self, \
              DeeObject *__restrict self) { \
 DeeObject *func; \
 func = DeeClass_GetOperator(tp_self,op); \
 if unlikely(!func) return NULL; \
 return DeeObject_ThisCall(func,self,0,NULL); \
} \
INTERN DREF DeeObject *DCALL \
instance_xxx(DeeObject *__restrict self) { \
 return instance_txxx(Dee_TYPE(self),self); \
}
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tinv,instance_inv,OPERATOR_INV)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tpos,instance_pos,OPERATOR_POS)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tneg,instance_neg,OPERATOR_NEG)
#undef DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION


INTERN DREF DeeObject *DCALL
instance_tstr(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self) {
 DeeObject *func,*result;
 func = DeeClass_GetOperator(tp_self,OPERATOR_STR);
 if unlikely(!func) goto err;
 result = DeeObject_ThisCall(func,self,0,NULL);
 if (likely(result) &&
     DeeObject_AssertTypeExact(result,&DeeString_Type))
     goto err_r;
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
instance_str(DeeObject *__restrict self) {
 return instance_tstr(Dee_TYPE(self),self);
}
INTERN DREF DeeObject *DCALL
instance_trepr(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
 DeeObject *func,*result;
 func = DeeClass_GetOperator(tp_self,OPERATOR_REPR);
 if unlikely(!func) goto err;
 result = DeeObject_ThisCall(func,self,0,NULL);
 if (likely(result) &&
     DeeObject_AssertTypeExact(result,&DeeString_Type))
     goto err_r;
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
instance_repr(DeeObject *__restrict self) {
 return instance_trepr(Dee_TYPE(self),self);
}


INTERN int DCALL
instance_tbool(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
 DeeObject *func,*result; int retval;
 func = DeeClass_GetOperator(tp_self,OPERATOR_BOOL);
 if unlikely(!func) return -1;
 result = DeeObject_ThisCall(func,self,0,NULL);
 /* XXX: Invocation loop? */
 retval = DeeObject_Bool(result);
 Dee_Decref(result);
 return retval;
}
INTERN int DCALL
instance_bool(DeeObject *__restrict self) {
 return instance_tbool(Dee_TYPE(self),self);
}




PUBLIC DREF DeeTypeObject *DCALL
DeeClass_New(DeeTypeObject *__restrict base,
             DeeObject *__restrict descriptor) {
 DeeClassDescriptorObject *desc;
 DREF DeeTypeObject *result;
 DeeTypeObject *result_type_type;
 struct class_desc *result_class;
 size_t result_class_offset;
 ASSERT_OBJECT_TYPE_EXACT(descriptor,&DeeClassDescriptor_Type);
 desc = (DeeClassDescriptorObject *)descriptor;
 result_type_type = Dee_TYPE(base);
 if (result_type_type == &DeeNone_Type) {
  result_type_type = &DeeType_Type; /* No base class. */
 } else {
  /* Make sure that the given base-object is actually a type. */
  if unlikely(!DeeType_IsInherited(&DeeType_Type,(DeeTypeObject *)result_type_type)) {
   DeeObject_TypeAssertFailed((DeeObject *)base,&DeeType_Type);
   goto err;
  }
  ASSERTF(!(result_type_type->tp_flags & TP_FVARIABLE),
          "type-type objects must not have the variable-size flag, but %s has it set!",
          result_type_type->tp_name);
  if (base->tp_flags & (TP_FFINAL|TP_FVARIABLE)) {
   DeeError_Throwf(&DeeError_TypeError,
                   "Cannot use final, or variable type `%s' as class base",
                   base->tp_name);
   goto err;
  }
 }
 result_class_offset  = result_type_type->tp_init.tp_alloc.tp_instance_size;
 result_class_offset +=  (sizeof(void *) - 1);
 result_class_offset &= ~(sizeof(void *) - 1);
 /* Allocate the resulting class object. */
 result = (DREF DeeTypeObject *)DeeGCObject_Calloc(result_class_offset +
                                                   COMPILER_OFFSETOF(struct class_desc,cd_members) +
                                                  (desc->cd_cmemb_size * sizeof(DREF DeeObject *)));
 if unlikely(!result) goto err;
 /* Figure out where the class descriptor starts. */
 result_class = (struct class_desc *)((uintptr_t)result + result_class_offset);
 result->tp_class = result_class;
 result->tp_flags = TP_FHEAP | TP_FGC | desc->cd_flags;
 if (DeeNone_Check(base)) {
  /*result->tp_base = NULL;*/
 } else {
  result->tp_base = base;
  Dee_Incref(base);
 }
 result_class->cd_desc = desc;
 Dee_Incref(desc);
 rwlock_cinit(&result_class->cd_lock);
 if likely(desc->cd_name) {
  result->tp_name   = DeeString_STR(desc->cd_name);
  result->tp_flags |= TP_FNAMEOBJECT;
  Dee_Incref(desc->cd_name);
 }
 if likely(desc->cd_doc) {
  result->tp_doc    = DeeString_STR(desc->cd_doc);
  result->tp_flags |= TP_FDOCOBJECT;
  Dee_Incref(desc->cd_doc);
 }
 ASSERT(desc->cd_clsop_mask != 0);
 ASSERT(desc->cd_cattr_mask != 0);
 ASSERT(desc->cd_iattr_mask != 0);
 {
  uint16_t i = 0;
  do {
   struct class_operator *op;
   op = &desc->cd_clsop_list[i];
   if (op->co_name == (uint16_t)-1) continue;
   /* TODO: Bind the C-wrapper-function for this operator. */

  } while (++i < desc->cd_clsop_mask);
 }


 if (result_type_type != &DeeType_Type) {
  /* Initialize custom fields of the underlying type. */
  int error = 0;
  if (result_type_type->tp_init.tp_alloc.tp_ctor) {
   error = (*result_type_type->tp_init.tp_alloc.tp_ctor)((DeeObject *)result);
  } else if (result_type_type->tp_init.tp_alloc.tp_any_ctor) {
   error = (*result_type_type->tp_init.tp_alloc.tp_any_ctor)((DeeObject *)result,0,NULL);
  } else if (result_type_type->tp_init.tp_alloc.tp_any_ctor_kw) {
   error = (*result_type_type->tp_init.tp_alloc.tp_any_ctor_kw)((DeeObject *)result,0,NULL,NULL);
  }
  if unlikely(error)
     goto err_r_base;
 }

 /* Initialize the resulting object, and start tracking it. */
 DeeObject_Init(result,result_type_type);
 return (DeeTypeObject *)DeeGC_Track((DeeObject *)result);
err_r_base:
 Dee_XDecref_unlikely(result->tp_base);
 Dee_Decref_unlikely(desc);
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}



DECL_END
#endif /* !CONFIG_USE_NEW_CLASS_SYSTEM */

#endif /* !GUARD_DEEMON_OBJECTS_CLASS2_C */
