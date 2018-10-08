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
#ifndef GUARD_DEEMON_OBJECTS_WEAKREF_C
#define GUARD_DEEMON_OBJECTS_WEAKREF_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/weakref.h>
#include <deemon/string.h>
#include <deemon/none.h>
#include <deemon/error.h>
#include <deemon/bool.h>
#include <deemon/arg.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeWeakRefObject     WeakRef;
typedef DeeWeakRefAbleObject WeakRefAble;


PRIVATE void DCALL
ob_weakref_fini(WeakRef *__restrict self) {
 weakref_fini(&self->wr_ref);
}
PRIVATE int DCALL
ob_weakref_init(WeakRef *__restrict self) {
 weakref_null(&self->wr_ref);
 return 0;
}
PRIVATE int DCALL
ob_weakref_copy(WeakRef *__restrict self,
                WeakRef *__restrict other) {
 DREF DeeObject *refobj;
 refobj = weakref_lock(&other->wr_ref);
 if (!refobj) weakref_null(&self->wr_ref);
 else {
#ifdef NDEBUG
  weakref_init(&self->wr_ref,refobj);
#else
  bool ok = weakref_init(&self->wr_ref,refobj);
  ASSERT(ok && "Then how did `other' manage to create one?");
#endif
  Dee_Decref(refobj);
 }
 return 0;
}
PRIVATE int DCALL
ob_weakref_deep(WeakRef *__restrict self,
                WeakRef *__restrict other) {
 DREF DeeObject *refobj,*refcopy;
 refobj = weakref_lock(&other->wr_ref);
 if (!refobj)
  weakref_null(&self->wr_ref);
 else {
  refcopy = DeeObject_DeepCopy(refobj);
  Dee_Decref(refobj);
  if unlikely(!refcopy) return -1;
#ifdef NDEBUG
  weakref_init(&self->wr_ref,refcopy);
#else
  {
   bool ok = weakref_init(&self->wr_ref,refcopy);
   ASSERT(ok && "Then how did `other' manage to create one?");
  }
#endif
  Dee_Decref(refcopy);
 }
 return 0;
}
PRIVATE int DCALL
ob_weakref_ctor(WeakRef *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *obj;
 if (DeeArg_Unpack(argc,argv,"o:weakref",&obj))
     goto err;
 if (!weakref_init(&self->wr_ref,obj))
      goto err_nosupport;
 return 0;
err_nosupport:
 err_cannot_weak_reference(obj);
err:
 return -1;
}

PRIVATE int DCALL
ob_weakref_assign(WeakRef *__restrict self,
                  DeeObject *__restrict other) {
 DREF DeeObject *refobj;
 if (DeeWeakRef_Check(other)) {
  refobj = weakref_lock(&((WeakRef *)other)->wr_ref);
  if (!refobj) weakref_null(&self->wr_ref);
  else {
#ifdef NDEBUG
   weakref_init(&self->wr_ref,refobj);
#else
   bool ok = weakref_init(&self->wr_ref,refobj);
   ASSERT(ok && "Then how did `other' manage to create one?");
#endif
   Dee_Decref(refobj);
  }
 } else {
  /* Assign the given other to our weak reference. */
  if (!weakref_set(&self->wr_ref,other)) {
   err_cannot_weak_reference(other);
   return -1;
  }
 }
 return 0;
}

PRIVATE DEFINE_STRING(empty_weakref,"empty weakref");
PRIVATE DEFINE_STRING(empty_weakref_repr,"weakref()");

PRIVATE DREF DeeObject *DCALL
ob_weakref_str(WeakRef *__restrict self) {
 DREF DeeObject *refobj;
 refobj = weakref_lock(&self->wr_ref);
 if (!refobj) return_reference_((DeeObject *)&empty_weakref);
 return DeeString_Newf("weakref -> %K",refobj);
}

PRIVATE DREF DeeObject *DCALL
ob_weakref_repr(WeakRef *__restrict self) {
 DREF DeeObject *refobj;
 refobj = weakref_lock(&self->wr_ref);
 if (!refobj) return_reference_((DeeObject *)&empty_weakref_repr);
 return DeeString_Newf("weakref(%R)",refobj);
}

PRIVATE int DCALL
ob_weakref_bool(WeakRef *__restrict self) {
 DREF DeeObject *refobj;
 refobj = weakref_lock(&self->wr_ref);
 if (!refobj) return 0;
 Dee_Decref(refobj);
 return 1;
}

#ifdef CONFIG_NO_THREADS
#define LAZY_GETOBJ(x)             ((x)->wr_ref.wr_obj)
#else
#define LAZY_GETOBJ(x)  ATOMIC_READ((x)->wr_ref.wr_obj)
#endif


PRIVATE dhash_t DCALL
ob_weakref_hash(WeakRef *__restrict self) {
 return Dee_HashPointer(LAZY_GETOBJ(self));
}

#define DEFINE_WEAKREF_CMP(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(WeakRef *__restrict self, \
     WeakRef *__restrict other) { \
 if (DeeNone_Check(other)) \
     return_bool((void *)LAZY_GETOBJ(self) op (void *)NULL); \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&DeeWeakRef_Type)) \
     return NULL; \
 return_bool(LAZY_GETOBJ(self) op LAZY_GETOBJ(other)); \
}
DEFINE_WEAKREF_CMP(ob_weakref_eq,==)
DEFINE_WEAKREF_CMP(ob_weakref_ne,!=)
DEFINE_WEAKREF_CMP(ob_weakref_lo,<)
DEFINE_WEAKREF_CMP(ob_weakref_le,<=)
DEFINE_WEAKREF_CMP(ob_weakref_gr,>)
DEFINE_WEAKREF_CMP(ob_weakref_ge,>=)
#undef DEFINE_WEAKREF_CMP

PRIVATE struct type_cmp ob_weakref_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&ob_weakref_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_ge
};


PRIVATE DREF DeeObject *DCALL
ob_weakref_get(WeakRef *__restrict self) {
 DREF DeeObject *result;
 result = weakref_lock(&self->wr_ref);
 if (!result)
      err_cannot_lock_weakref();
 return result;
}

PRIVATE int DCALL
ob_weakref_del(WeakRef *__restrict self) {
 weakref_clear(&self->wr_ref);
 /* Don't throw an error if the reference wasn't bound to prevent
  * a race condition between someone trying to delete the weakref
  * and someone else destroying the pointed-to object. */
 return 0;
}

PRIVATE int DCALL
ob_weakref_set(WeakRef *__restrict self,
               DeeObject *__restrict value) {
 if (!weakref_set(&self->wr_ref,value)) {
  err_cannot_weak_reference(value);
  return -1;
 }
 return 0;
}

PRIVATE DREF DeeObject *DCALL
ob_weakref_lock(WeakRef *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; DeeObject *alt = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:lock",&alt))
     return NULL;
 result = weakref_lock(&self->wr_ref);
 if (!result) {
  if ((result = alt) == NULL)
       err_cannot_lock_weakref();
  else Dee_Incref(result);
 }
 return result;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DREF DeeObject *DCALL
ob_weakref_try_lock(WeakRef *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result;
 if (DeeArg_Unpack(argc,argv,":try_lock"))
     return NULL;
 result = weakref_lock(&self->wr_ref);
 if (!result) {
  result = Dee_None;
  Dee_Incref(Dee_None);
 }
 return result;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE DREF DeeObject *DCALL
ob_weakref_alive(WeakRef *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":alive"))
     return NULL;
 return_bool(ob_weakref_bool(self));
}

PRIVATE struct type_method ob_weakref_methods[] = {
    { "lock", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ob_weakref_lock,
      DOC("->object\n"
          "(object def)->object\n"
          "@throw ReferenceError The weak reference is no longer bound and no @def was given\n"
          "Lock the weak reference and return the pointed-to object") },
    { "alive", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ob_weakref_alive,
      DOC("->bool\n"
          "Alias for #op:bool") },
#ifndef CONFIG_NO_DEEMON_100_COMPAT
    { "try_lock", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&ob_weakref_try_lock,
      DOC("->none\n"
          "->object\n"
          "Deprecated alias for #lock with passing :none (${this.lock(none)})") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
    { NULL }
};

PRIVATE struct type_getset ob_weakref_getsets[] = {
    { "value",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ob_weakref_get,
     (int(DCALL *)(DeeObject *__restrict))&ob_weakref_del,
     (int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_set,
      DOC("->object\n"
          "@throw ReferenceError Attempted to get the value after the reference has been unbound\n"
          "@throw ValueError Attempted to set an object that does not support weak referencing\n"
          "Access to the referenced object") },
    { NULL }
};


PUBLIC DeeTypeObject DeeWeakRef_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_weakref),
    /* .tp_doc      = */DOC("A weak reference to another object implementing weakref functionality\n"
                            "\n"
                            "()\n"
                            "Construct an unbound weak reference\n"
                            "\n"
                            "(object obj)\n"
                            "@throw TypeError The given object @obj does not implement weak referencing support\n"
                            "Construct a weak reference bound to @obj, that will be notified once said "
                            "object is supposed to be destroyed With that in mind, weak references don't "
                            "actually hold references at all, but rather allow the user to test if an "
                            "object is still allocated at runtime.\n"
                            "\n"
                            "operator bool\n"
                            "Returns true if the weak reference is currently bound. Note however that this "
                            "information is volatile and may not longer be up-to-date by the time the operator returns\n"
                            "\n"
                            "==(none other)\n"
                            "!=(none other)\n"
                            "<(none other)\n"
                            "<=(none other)\n"
                            ">(none other)\n"
                            ">=(none other)\n"
                            "Test for the pointed-to object being bound\n"
                            "==(weakref other)\n"
                            "!=(weakref other)\n"
                            "<(weakref other)\n"
                            "<=(weakref other)\n"
                            ">(weakref other)\n"
                            ">=(weakref other)\n"
                            "Compare the pointed-to objects to @this weak reference to that of @other\n"
                            "\n"
                            ":=(weakref other)\n"
                            ":=(object obj)\n"
                            "@throw TypeError The given @obj does not implement weak referencing support\n"
                            "Assign the value of @other, or @obj to @this weakref object"),
    /* .tp_flags    = */TP_FNORMAL|TP_FGC|TP_FNAMEOBJECT|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&ob_weakref_init,
                /* .tp_copy_ctor = */&ob_weakref_copy,
                /* .tp_deep_ctor = */&ob_weakref_deep,
                /* .tp_any_ctor  = */&ob_weakref_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(WeakRef)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ob_weakref_fini,
        /* .tp_assign      = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ob_weakref_assign,
        /* .tp_move_assign = */NULL,
        /* .tp_deepload    = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ob_weakref_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ob_weakref_repr,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&ob_weakref_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&ob_weakref_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */ob_weakref_methods,
    /* .tp_getsets       = */ob_weakref_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE int DCALL
weakrefable_init(WeakRefAble *__restrict self) {
 weakref_support_init(self);
 return 0;
}
PRIVATE int DCALL
weakrefable_copy(WeakRefAble *__restrict self,
                 WeakRefAble *__restrict UNUSED(other)) {
 weakref_support_init(self);
 return 0;
}

PUBLIC DeeTypeObject DeeWeakRefAble_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_weakrefable),
    /* .tp_doc      = */DOC("An base class that user-defined clases can "
                            "be derived from to become weakly referencable"),
    /* .tp_flags    = */TP_FNORMAL|TP_FINHERITCTOR|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */WEAKREF_SUPPORT_ADDR(WeakRefAble),
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&weakrefable_init,
                /* .tp_copy_ctor = */&weakrefable_copy,
                /* .tp_deep_ctor = */&weakrefable_copy,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(WeakRefAble)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL,
        /* .tp_deepload    = */NULL
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

#endif /* !GUARD_DEEMON_OBJECTS_WEAKREF_C */
