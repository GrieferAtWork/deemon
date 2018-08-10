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
#ifndef GUARD_DEEMON_OBJECTS_OBJMETHOD_C
#define GUARD_DEEMON_OBJECTS_OBJMETHOD_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/thread.h>
#include <deemon/none.h>
#include <deemon/arg.h>
#include <deemon/string.h>

#ifndef CONFIG_NO_STDIO
#ifndef NDEBUG
#include <stdlib.h>
#endif
#endif

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

PUBLIC DREF DeeObject *DCALL
DeeObjMethod_New(dobjmethod_t func, DeeObject *__restrict self) {
 DREF DeeObjMethodObject *result;
 ASSERT_OBJECT(self);
 result = DeeObject_MALLOC(DeeObjMethodObject);
 if (result) {
  DeeObject_Init(result,&DeeObjMethod_Type);
  result->om_func = func;
  result->om_self = self;
  Dee_Incref(self);
 }
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeKwObjMethod_New(dkwobjmethod_t func, DeeObject *__restrict self) {
 DREF DeeKwObjMethodObject *result;
 ASSERT_OBJECT(self);
 result = DeeObject_MALLOC(DeeKwObjMethodObject);
 if (result) {
  DeeObject_Init(result,&DeeKwObjMethod_Type);
  result->om_func = func;
  result->om_self = self;
  Dee_Incref(self);
 }
 return (DREF DeeObject *)result;
}


PRIVATE void DCALL
objmethod_fini(DeeObjMethodObject *__restrict self) {
 Dee_Decref(self->om_self);
}

PRIVATE void DCALL
objmethod_visit(DeeObjMethodObject *__restrict self,
                dvisit_t proc, void *arg) {
 Dee_Visit(((DeeObjMethodObject *)self)->om_self);
}

PRIVATE DREF DeeObject *DCALL
objmethod_call(DeeObjMethodObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 return DeeObjMethod_CallFunc(self->om_func,self->om_self,argc,argv);
}
PRIVATE dhash_t DCALL
objmethod_hash(DeeObjMethodObject *__restrict self) {
 return DeeObject_Hash(self->om_self) ^ (dhash_t)self->om_func;
}
PRIVATE DREF DeeObject *DCALL
objmethod_eq(DeeObjMethodObject *__restrict self,
             DeeObjMethodObject *__restrict other) {
 int temp;
 if (DeeObject_AssertType((DeeObject *)other,&DeeObjMethod_Type))
     return NULL;
 if (self->om_func != other->om_func) return_false;
 temp = DeeObject_CompareEq(self->om_self,other->om_self);
 if unlikely(temp < 0) return NULL;
 return_bool_(temp);
}
PRIVATE DREF DeeObject *DCALL
objmethod_ne(DeeObjMethodObject *__restrict self,
             DeeObjMethodObject *__restrict other) {
 int temp;
 if (DeeObject_AssertType((DeeObject *)other,&DeeObjMethod_Type))
     return NULL;
 if (self->om_func != other->om_func) return_true;
 temp = DeeObject_CompareNe(self->om_self,other->om_self);
 if unlikely(temp < 0) return NULL;
 return_bool_(temp);
}

PRIVATE struct type_cmp objmethod_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&objmethod_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&objmethod_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&objmethod_ne
};

PRIVATE char const *DCALL
typeobject_find_objmethod(DeeTypeObject *__restrict self,
                          dobjmethod_t meth) {
 do {
  struct type_method *iter;
  iter = self->tp_class_methods;
  if (iter) for (; iter->m_name; ++iter) {
   if (iter->m_func == meth)
       return iter->m_name;
  }
 } while ((self = DeeType_Base(self)) != NULL);
 return NULL;
}
PRIVATE char const *DCALL
object_find_objmethod(DeeObject *__restrict self,
                      dobjmethod_t meth) {
 char const *result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 do {
  struct type_method *iter;
  if (tp_self == &DeeType_Type) {
   result = typeobject_find_objmethod((DeeTypeObject *)self,meth);
   if (result) return result;
  }
  iter = tp_self->tp_methods;
  if (iter) for (; iter->m_name; ++iter) {
   if (iter->m_func == meth)
       return iter->m_name;
  }
 } while ((tp_self = DeeType_Base(tp_self)) != NULL);
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
objmethod_get_func(DeeObjMethodObject *__restrict self) {
 return DeeClsMethod_New(Dee_TYPE(self->om_self),self->om_func);
}
PRIVATE DREF DeeObject *DCALL
objmethod_get_name(DeeObjMethodObject *__restrict self) {
 char const *name;
 name = object_find_objmethod(self->om_self,self->om_func);
 if unlikely(!name) name = "?";
 return DeeString_NewAuto(name);
}


PRIVATE struct type_getset objmethod_getsets[] = {
    { "__func__", (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_func, NULL, NULL, DOC("->callable") },
    { "__name__", (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_name, NULL, NULL, DOC("->string") },
    { NULL }
};
PRIVATE struct type_member objmethod_members[] = {
    TYPE_MEMBER_FIELD("__self__",STRUCT_OBJECT,
                      offsetof(DeeObjMethodObject,om_self)),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
objmethod_repr(DeeObjMethodObject *__restrict self) {
 char const *name;
 name = object_find_objmethod(self->om_self,self->om_func);
 if unlikely(!name) name = "?";
 return DeeString_Newf("%r.%s",self->om_self,name);
}


PUBLIC DeeTypeObject DeeObjMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_objmethod",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                    /* .tp_instance_size = */sizeof(DeeObjMethodObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&objmethod_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&objmethod_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&objmethod_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&objmethod_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&objmethod_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */objmethod_getsets,
    /* .tp_members       = */objmethod_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


STATIC_ASSERT(COMPILER_OFFSETOF(DeeObjMethodObject,om_self) ==
              COMPILER_OFFSETOF(DeeKwObjMethodObject,om_self));
STATIC_ASSERT(COMPILER_OFFSETOF(DeeObjMethodObject,om_func) ==
              COMPILER_OFFSETOF(DeeKwObjMethodObject,om_func));
#define kwobjmethod_fini  objmethod_fini
#define kwobjmethod_visit objmethod_visit
PRIVATE DREF DeeObject *DCALL
kwobjmethod_call(DeeKwObjMethodObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 return DeeKwObjMethod_CallFunc(self->om_func,self->om_self,argc,argv,NULL);
}
PRIVATE DREF DeeObject *DCALL
kwobjmethod_call_kw(DeeKwObjMethodObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 return DeeKwObjMethod_CallFunc(self->om_func,self->om_self,argc,argv,kw);
}
#define kwobjmethod_cmp objmethod_cmp
PRIVATE DREF DeeObject *DCALL
kwobjmethod_get_func(DeeKwObjMethodObject *__restrict self) {
 return DeeKwClsMethod_New(Dee_TYPE(self->om_self),self->om_func);
}
#define kwobjmethod_get_name objmethod_get_name

PRIVATE struct type_getset kwobjmethod_getsets[] = {
    { "__func__", (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_func, NULL, NULL, DOC("->callable") },
    { "__name__", (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_name, NULL, NULL, DOC("->string") },
    { NULL }
};
#define kwobjmethod_members objmethod_members
#define kwobjmethod_repr    objmethod_repr

PUBLIC DeeTypeObject DeeKwObjMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_kwobjmethod",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                    /* .tp_instance_size = */sizeof(DeeKwObjMethodObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&kwobjmethod_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwobjmethod_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&kwobjmethod_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&kwobjmethod_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&kwobjmethod_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */kwobjmethod_getsets,
    /* .tp_members       = */kwobjmethod_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL,
    /* .tp_call_kw       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict,DeeObject*))&kwobjmethod_call_kw,
};


PUBLIC DREF DeeObject *DCALL
DeeClsMethod_New(DeeTypeObject *__restrict type,
                 dobjmethod_t func) {
 DeeClsMethodObject *result;
 ASSERT_OBJECT_TYPE(type,&DeeType_Type);
 result = DeeObject_MALLOC(DeeClsMethodObject);
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&DeeClsMethod_Type);
 result->cm_type = type;
 result->cm_func = func;
 Dee_Incref(type);
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeKwClsMethod_New(DeeTypeObject *__restrict type,
                   dkwobjmethod_t func) {
 DeeKwClsMethodObject *result;
 ASSERT_OBJECT_TYPE(type,&DeeType_Type);
 result = DeeObject_MALLOC(DeeKwClsMethodObject);
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&DeeKwClsMethod_Type);
 result->cm_type = type;
 result->cm_func = func;
 Dee_Incref(type);
 return (DREF DeeObject *)result;
}



PRIVATE DREF DeeObject *DCALL
clsmethod_call(DeeClsMethodObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 if (!argc) {
  DeeError_Throwf(&DeeError_TypeError,
                  "classmethod objects must be called with at least 1 argument");
  return NULL;
 }
 /* Allow non-instance objects for generic types. */
 if (!(self->cm_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(argv[0],self->cm_type))
       return NULL;
 /* Use the first argument as the this-argument. */
 return (*self->cm_func)(argv[0],argc-1,argv+1);
}

PRIVATE void DCALL
clsmethod_fini(DeeClsMethodObject *__restrict self) {
 Dee_Decref(self->cm_type);
}
PRIVATE void DCALL
clsmethod_visit(DeeClsMethodObject *__restrict self,
                dvisit_t proc, void *arg) {
 Dee_Visit(self->cm_type);
}
PRIVATE dhash_t DCALL
clsmethod_hash(DeeClsMethodObject *__restrict self) {
 return Dee_HashPointer(self->cm_func);
}
PRIVATE DREF DeeObject *DCALL
clsmethod_eq(DeeClsMethodObject *__restrict self,
             DeeClsMethodObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,Dee_TYPE(self)))
     return NULL;
 return_bool_(self->cm_func == other->cm_func);
}
PRIVATE DREF DeeObject *DCALL
clsmethod_ne(DeeClsMethodObject *__restrict self,
             DeeClsMethodObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,Dee_TYPE(self)))
     return NULL;
 return_bool_(self->cm_func != other->cm_func);
}
PRIVATE DREF DeeObject *DCALL
clsmethod_lo(DeeClsMethodObject *__restrict self,
             DeeClsMethodObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,Dee_TYPE(self)))
     return NULL;
 return_bool_(self->cm_func < other->cm_func);
}
PRIVATE DREF DeeObject *DCALL
clsmethod_le(DeeClsMethodObject *__restrict self,
             DeeClsMethodObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,Dee_TYPE(self)))
     return NULL;
 return_bool_(self->cm_func <= other->cm_func);
}
PRIVATE DREF DeeObject *DCALL
clsmethod_gr(DeeClsMethodObject *__restrict self,
             DeeClsMethodObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,Dee_TYPE(self)))
     return NULL;
 return_bool_(self->cm_func > other->cm_func);
}
PRIVATE DREF DeeObject *DCALL
clsmethod_ge(DeeClsMethodObject *__restrict self,
             DeeClsMethodObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,Dee_TYPE(self)))
     return NULL;
 return_bool_(self->cm_func >= other->cm_func);
}

PRIVATE struct type_cmp clsmethod_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&clsmethod_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmethod_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmethod_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmethod_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmethod_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmethod_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmethod_ge
};

PRIVATE struct type_member clsmethod_members[] = {
    TYPE_MEMBER_FIELD("__type__",STRUCT_OBJECT,offsetof(DeeClsMethodObject,cm_type)),
    TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeClsMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classmethod",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                    /* .tp_instance_size = */sizeof(DeeClsMethodObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&clsmethod_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsmethod_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&clsmethod_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&clsmethod_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */clsmethod_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsMethodObject,cm_func) ==
              COMPILER_OFFSETOF(DeeKwClsMethodObject,cm_func));
STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsMethodObject,cm_type) ==
              COMPILER_OFFSETOF(DeeKwClsMethodObject,cm_type));

PRIVATE DREF DeeObject *DCALL
kwclsmethod_call(DeeKwClsMethodObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 if (!argc) {
  DeeError_Throwf(&DeeError_TypeError,
                  "classmethod objects must be called with at least 1 argument");
  return NULL;
 }
 /* Allow non-instance objects for generic types. */
 if (!(self->cm_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(argv[0],self->cm_type))
       return NULL;
 /* Use the first argument as the this-argument. */
 return (*self->cm_func)(argv[0],argc-1,argv+1,NULL);
}

PRIVATE DREF DeeObject *DCALL
kwclsmethod_call_kw(DeeKwClsMethodObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv,
                    DeeObject *kw) {
 if (!argc) {
  DeeError_Throwf(&DeeError_TypeError,
                  "classmethod objects must be called with at least 1 argument");
  return NULL;
 }
 /* Allow non-instance objects for generic types. */
 if (!(self->cm_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(argv[0],self->cm_type))
       return NULL;
 /* Use the first argument as the this-argument. */
 return (*self->cm_func)(argv[0],argc-1,argv+1,kw);
}

#define kwclsmethod_fini    clsmethod_fini
#define kwclsmethod_visit   clsmethod_visit
#define kwclsmethod_hash    clsmethod_hash
#define kwclsmethod_cmp     clsmethod_cmp
#define kwclsmethod_members clsmethod_members

PUBLIC DeeTypeObject DeeKwClsMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_kwclassmethod",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                    /* .tp_instance_size = */sizeof(DeeKwClsMethodObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&kwclsmethod_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&kwclsmethod_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&kwclsmethod_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&kwclsmethod_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */kwclsmethod_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL,
    /* .tp_call_kw       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict,DeeObject*))&kwclsmethod_call_kw
};


PUBLIC DREF /*ClsProperty*/DeeObject *DCALL
DeeClsProperty_New(DeeTypeObject *__restrict type,
                   struct type_getset const *__restrict desc) {
 DeeClsPropertyObject *result;
 ASSERT_OBJECT_TYPE(type,&DeeType_Type);
 result = DeeObject_MALLOC(DeeClsPropertyObject);
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&DeeClsProperty_Type);
 result->cp_type = type;
 Dee_Incref(type);
 result->cp_get = desc->gs_get;
 result->cp_del = desc->gs_del;
 result->cp_set = desc->gs_set;
 return (DREF DeeObject *)result;
}

PRIVATE dhash_t DCALL
clsproperty_hash(DeeClsPropertyObject *__restrict self) {
 return (Dee_HashPointer(self->cp_get) ^
         Dee_HashPointer(self->cp_del) ^
         Dee_HashPointer(self->cp_set));
}
PRIVATE DREF DeeObject *DCALL
clsproperty_eq(DeeClsPropertyObject *__restrict self,
               DeeClsPropertyObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeClsProperty_Type))
     return NULL;
 return_bool(self->cp_get == other->cp_get &&
             self->cp_del == other->cp_del &&
             self->cp_set == other->cp_set);
}
PRIVATE DREF DeeObject *DCALL
clsproperty_ne(DeeClsPropertyObject *__restrict self,
               DeeClsPropertyObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeClsProperty_Type))
     return NULL;
 return_bool(self->cp_get != other->cp_get ||
             self->cp_del != other->cp_del ||
             self->cp_set != other->cp_set);
}

PRIVATE struct type_cmp clsproperty_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&clsproperty_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsproperty_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsproperty_ne
};

PRIVATE struct keyword getter_kwlist[] = { K(thisarg), KEND };
PRIVATE struct keyword setter_kwlist[] = { K(thisarg), K(value), KEND };

PRIVATE DREF DeeObject *DCALL
clsproperty_get(DeeClsPropertyObject *__restrict self,
                size_t argc, DREF DeeObject **__restrict argv,
                DeeObject *kw) {
 DeeObject *thisarg;
 if (!self->cp_get) {
  err_cant_access_attribute(&DeeClsProperty_Type,"get",ATTR_ACCESS_GET);
  return NULL;
 }
 if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg) ||
       /* Allow non-instance objects for generic types. */
    (!(self->cp_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(thisarg,self->cp_type)))
       return NULL;
 return (*self->cp_get)(thisarg);
}
PRIVATE DREF DeeObject *DCALL
clsproperty_del(DeeClsPropertyObject *__restrict self,
                size_t argc, DREF DeeObject **__restrict argv,
                DeeObject *kw) {
 DeeObject *thisarg;
 if (!self->cp_del) {
  err_cant_access_attribute(&DeeClsProperty_Type,"del",ATTR_ACCESS_GET);
  return NULL;
 }
 if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:del",&thisarg) ||
       /* Allow non-instance objects for generic types. */
    (!(self->cp_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(thisarg,self->cp_type)))
       return NULL;
 if unlikely((*self->cp_del)(thisarg))
    return NULL;
 return_none;
}
PRIVATE DREF DeeObject *DCALL
clsproperty_set(DeeClsPropertyObject *__restrict self,
                size_t argc, DREF DeeObject **__restrict argv,
                DeeObject *kw) {
 DeeObject *thisarg,*value;
 if (!self->cp_set) {
  err_cant_access_attribute(&DeeClsProperty_Type,"set",ATTR_ACCESS_GET);
  return NULL;
 }
 if (DeeArg_UnpackKw(argc,argv,kw,setter_kwlist,"oo:set",&thisarg,&value) ||
       /* Allow non-instance objects for generic types. */
    (!(self->cp_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(thisarg,self->cp_type)))
       return NULL;
 if unlikely((*self->cp_set)(thisarg,value))
    return NULL;
 return_none;
}

PRIVATE struct type_method clsproperty_methods[] = {
    { "get", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsproperty_get,
       DOC("(thisarg)"), TYPE_METHOD_FKWDS },
    { "del", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsproperty_del,
       DOC("(thisarg)->none"), TYPE_METHOD_FKWDS },
    { "set", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsproperty_set,
       DOC("(thisarg,value)->none"), TYPE_METHOD_FKWDS },
    { NULL }
};

PRIVATE struct type_member clsproperty_members[] = {
    TYPE_MEMBER_FIELD("__type__",STRUCT_OBJECT,offsetof(DeeClsPropertyObject,cp_type)),
    TYPE_MEMBER_END
};

/* Make sure that we're allowed to re-use operators from classmethod. */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsPropertyObject,cp_type) ==
              COMPILER_OFFSETOF(DeeClsMethodObject,cm_type));
PUBLIC DeeTypeObject DeeClsProperty_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classproperty",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeClsPropertyObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&clsmethod_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsproperty_get,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&clsmethod_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&clsproperty_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */clsproperty_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */clsproperty_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



/* Create a new unbound class property object. */
PUBLIC DREF /*ClsMember*/DeeObject *DCALL
DeeClsMember_New(DeeTypeObject *__restrict type,
                 struct type_member const *__restrict desc) {
 DREF DeeClsMemberObject *result;
 ASSERT_OBJECT_TYPE(type,&DeeType_Type);
 result = DeeObject_MALLOC(DeeClsMemberObject);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&DeeClsMember_Type);
 result->cm_memb = *desc;
 result->cm_type = type;
 Dee_Incref(type);
done:
 return (DREF DeeObject *)result;
}

PRIVATE void DCALL
clsmember_fini(DeeClsMemberObject *__restrict self) {
 Dee_Decref(self->cm_type);
}
PRIVATE DREF DeeObject *DCALL
clsmember_str(DeeClsMemberObject *__restrict self) {
 return DeeString_Newf("%s.%s",self->cm_type->tp_name,self->cm_memb.m_name);
}
PRIVATE void DCALL
clsmember_visit(DeeClsMemberObject *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->cm_type);
}

PRIVATE DREF DeeObject *DCALL
clsmember_get(DeeClsMemberObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
 DeeObject *thisarg;
 if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg) ||
       /* Allow non-instance objects for generic types. */
    (!(self->cm_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(thisarg,self->cm_type)))
       return NULL;
 return type_member_get(&self->cm_memb,thisarg);
}
PRIVATE DREF DeeObject *DCALL
clsmember_del(DeeClsMemberObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
 DeeObject *thisarg;
 if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:del",&thisarg) ||
       /* Allow non-instance objects for generic types. */
    (!(self->cm_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(thisarg,self->cm_type)) ||
       type_member_del(&self->cm_memb,thisarg))
       return NULL;
 return_none;
}
PRIVATE DREF DeeObject *DCALL
clsmember_set(DeeClsMemberObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
 DeeObject *thisarg,*value;
 if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"oo:set",&thisarg,&value) ||
       /* Allow non-instance objects for generic types. */
    (!(self->cm_type->tp_flags&TP_FABSTRACT) &&
       DeeObject_AssertType(thisarg,self->cm_type)) ||
       type_member_set(&self->cm_memb,thisarg,value))
       return NULL;
 return_none;
}

PRIVATE struct type_method clsmember_methods[] = {
    { "get", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsmember_get,
       DOC("(thisarg)"), TYPE_METHOD_FKWDS },
    { "del", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsmember_del,
       DOC("(thisarg)->none"), TYPE_METHOD_FKWDS },
    { "set", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsmember_set,
       DOC("(thisarg,value)->none"), TYPE_METHOD_FKWDS },
    { NULL }
};

PRIVATE dhash_t DCALL
clsmember_hash(DeeClsMemberObject *__restrict self) {
 return (Dee_HashPointer(self->cm_type) ^
         Dee_HashPointer(self->cm_memb.m_name) ^
         Dee_HashPointer(self->cm_memb.m_const));
}
#define CLSMEMBER_CMP(a,b) \
  memcmp(&(a)->cm_memb,&(b)->cm_memb, \
           sizeof(DeeClsMemberObject)- \
           offsetof(DeeClsMemberObject,cm_memb))

#define DEFINE_CLSMEMBER_CMP(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(DeeClsMemberObject *__restrict self, \
     DeeClsMemberObject *__restrict other) { \
 if (DeeObject_AssertType((DeeObject *)other,&DeeClsMember_Type)) \
     return NULL; \
 return_bool(CLSMEMBER_CMP(self,other) op 0); \
}
DEFINE_CLSMEMBER_CMP(clsmember_eq,==)
DEFINE_CLSMEMBER_CMP(clsmember_ne,!=)
DEFINE_CLSMEMBER_CMP(clsmember_lo,<)
DEFINE_CLSMEMBER_CMP(clsmember_le,<=)
DEFINE_CLSMEMBER_CMP(clsmember_gr,>)
DEFINE_CLSMEMBER_CMP(clsmember_ge,>=)
#undef DEFINE_CLSMEMBER_CMP
#undef CLSMEMBER_CMP

PRIVATE struct type_cmp clsmember_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&clsmember_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmember_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmember_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmember_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmember_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmember_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&clsmember_ge
};

PRIVATE struct type_member clsmember_members[] = {
    TYPE_MEMBER_FIELD("__type__",STRUCT_OBJECT,offsetof(DeeClsMemberObject,cm_type)),
    TYPE_MEMBER_FIELD("__name__",STRUCT_CONST|STRUCT_CSTR,offsetof(DeeClsMemberObject,cm_memb.m_name)),
    TYPE_MEMBER_FIELD("__doc__",STRUCT_CONST|STRUCT_CSTR_EMPTY,offsetof(DeeClsMemberObject,cm_memb.m_doc)),
    TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeClsMember_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classmember",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeClsMemberObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&clsmember_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmember_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmember_str,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&clsmember_get,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&clsmember_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&clsmember_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */clsmember_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */clsmember_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



/* Make sure that we can re-use some functions from `classmethod' */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeCMethodObject,cm_func) ==
              COMPILER_OFFSETOF(DeeClsMethodObject,cm_func));


PRIVATE DREF DeeObject *DCALL
cmethod_call(DeeCMethodObject *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 return DeeCMethod_CallFunc(self->cm_func,argc,argv);
}

PUBLIC DeeTypeObject DeeCMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_cmethod",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                    /* .tp_instance_size = */sizeof(DeeCMethodObject)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&cmethod_call,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&clsmethod_cmp,
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


/* Make sure that we can re-use some functions from `classmethod' */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeKwCMethodObject,cm_func) ==
              COMPILER_OFFSETOF(DeeCMethodObject,cm_func));


PRIVATE DREF DeeObject *DCALL
kwcmethod_call(DeeKwCMethodObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 return DeeKwCMethod_CallFunc(self->cm_func,argc,argv,NULL);
}

PRIVATE DREF DeeObject *DCALL
kwcmethod_call_kw(DeeKwCMethodObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv,
                  DeeObject *kw) {
 return DeeKwCMethod_CallFunc(self->cm_func,argc,argv,kw);
}

PUBLIC DeeTypeObject DeeKwCMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_kwcmethod",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
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
                    /* .tp_instance_size = */sizeof(DeeKwCMethodObject)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&kwcmethod_call,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&clsmethod_cmp,
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
    /* .tp_class_members = */NULL,
    /* .tp_call_kw       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict,DeeObject*))&kwcmethod_call_kw
};


#ifndef NDEBUG
PRIVATE ATTR_NORETURN void DCALL
fatal_invalid_except(DeeObject *__restrict return_value,
                     uint16_t excepted, void *callback_addr) {
 DEE_DPRINTF("Exception depth was improperly modified:\n"
             "After a return value %p from C-function %p, the exception depth should have been %u, but was actually %u\n"
             "For details, see the C documentation of `DeeCMethod_CallFunc'",
             return_value,callback_addr,excepted,DeeThread_Self()->t_exceptsz);
 BREAKPOINT();
#ifndef CONFIG_NO_STDIO
#if !defined(CONFIG_NO__Exit) && \
    (defined(CONFIG_HAVE__Exit) || \
     defined(_Exit) || defined(__USE_ISOC99))
 _Exit(1);
#elif !defined(CONFIG_NO__exit) && \
      (defined(_MSC_VER) || defined(CONFIG_HAVE__exit) || defined(_exit))
 _exit(1);
#else
 ASSERT(0);
#endif
#endif
 for (;;){}
}

INTERN DREF DeeObject *DCALL
DeeCMethod_CallFunc_d(dcmethod_t fun, size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result;
 DeeThreadObject *caller = DeeThread_Self();
 uint16_t old_depth = caller->t_exceptsz;
 result = (*fun)(argc,argv);
 if unlikely(result ? old_depth   != caller->t_exceptsz
                    : old_depth+1 != caller->t_exceptsz)
    fatal_invalid_except(result,old_depth+!result,(void *)fun);
 return result;
}
INTERN DREF DeeObject *DCALL
DeeObjMethod_CallFunc_d(dobjmethod_t fun, DeeObject *__restrict self,
                        size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result;
 DeeThreadObject *caller = DeeThread_Self();
 uint16_t old_depth = caller->t_exceptsz;
 result = (*fun)(self,argc,argv);
 if unlikely(result ? old_depth   != caller->t_exceptsz
                    : old_depth+1 != caller->t_exceptsz)
    fatal_invalid_except(result,old_depth+!result,(void *)fun);
 return result;
}

INTERN DREF DeeObject *DCALL
DeeKwCMethod_CallFunc_d(dkwcmethod_t fun, size_t argc,
                        DeeObject **__restrict argv, DeeObject *kw) {
 DREF DeeObject *result;
 DeeThreadObject *caller = DeeThread_Self();
 uint16_t old_depth = caller->t_exceptsz;
 result = (*fun)(argc,argv,kw);
 if unlikely(result ? old_depth   != caller->t_exceptsz
                    : old_depth+1 != caller->t_exceptsz)
    fatal_invalid_except(result,old_depth+!result,(void *)fun);
 return result;
}
INTERN DREF DeeObject *DCALL
DeeKwObjMethod_CallFunc_d(dkwobjmethod_t fun, DeeObject *__restrict self,
                          size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DREF DeeObject *result;
 DeeThreadObject *caller = DeeThread_Self();
 uint16_t old_depth = caller->t_exceptsz;
 result = (*fun)(self,argc,argv,kw);
 if unlikely(result ? old_depth   != caller->t_exceptsz
                    : old_depth+1 != caller->t_exceptsz)
    fatal_invalid_except(result,old_depth+!result,(void *)fun);
 return result;
}
#endif


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJMETHOD_C */
