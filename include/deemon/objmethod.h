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
#ifndef GUARD_DEEMON_OBJMETHOD_H
#define GUARD_DEEMON_OBJMETHOD_H 1

#include "api.h"
#include "object.h"
#include <stddef.h>
#include <stdarg.h>

DECL_BEGIN

typedef struct objmethod_object DeeObjMethodObject;
typedef struct kwobjmethod_object DeeKwObjMethodObject;
typedef struct clsmethod_object DeeClsMethodObject;
typedef struct kwclsmethod_object DeeKwClsMethodObject;
typedef struct clsproperty_object DeeClsPropertyObject;
typedef struct clsmember_object DeeClsMemberObject;
typedef struct cmethod_object DeeCMethodObject;
typedef struct kwcmethod_object DeeKwCMethodObject;

typedef DREF DeeObject *(DCALL *dcmethod_t)(size_t argc, DeeObject **__restrict argv);
typedef DREF DeeObject *(DCALL *dkwcmethod_t)(size_t argc, DeeObject **__restrict argv, DeeObject *kw);


struct objmethod_object {
    OBJECT_HEAD /* Object-bound member function. */
    dobjmethod_t    om_func;  /* [1..1][const] C-level object function. */
    DREF DeeObject *om_self;  /* [1..1][const] The `self' argument passed to `om_func'. */
};
struct kwobjmethod_object {
    OBJECT_HEAD /* Object-bound member function. */
    dkwobjmethod_t  om_func;  /* [1..1][const] C-level object function. */
    DREF DeeObject *om_self;  /* [1..1][const] The `self' argument passed to `om_func'. */
};
DDATDEF DeeTypeObject DeeObjMethod_Type;
DDATDEF DeeTypeObject DeeKwObjMethod_Type;
#define DeeObjMethod_SELF(x)       ((DeeObjMethodObject *)REQUIRES_OBJECT(x))->om_self
#define DeeObjMethod_FUNC(x)       ((DeeObjMethodObject *)REQUIRES_OBJECT(x))->om_func
#define DeeObjMethod_Check(x)        DeeObject_InstanceOfExact(x,&DeeObjMethod_Type) /* `objmethod' is `final'. */
#define DeeObjMethod_CheckExact(x)   DeeObject_InstanceOfExact(x,&DeeObjMethod_Type)
#define DeeKwObjMethod_Check(x)      DeeObject_InstanceOfExact(x,&DeeKwObjMethod_Type) /* `kwobjmethod' is `final'. */
#define DeeKwObjMethod_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeKwObjMethod_Type)
#define DEFINE_OBJMETHOD(name,func,self) \
  DeeObjMethodObject name = { OBJECT_HEAD_INIT(&DeeObjMethod_Type), func, self }
#define DEFINE_KWOBJMETHOD(name,func,self) \
  DeeKwObjMethodObject name = { OBJECT_HEAD_INIT(&DeeKwObjMethod_Type), func, self }

/* Construct a new `objmethod' object. */
DFUNDEF DREF DeeObject *DCALL DeeObjMethod_New(dobjmethod_t func, DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeKwObjMethod_New(dkwobjmethod_t func, DeeObject *__restrict self);

/* Returns the name of the function bound by the given
 * objmethod, or `NULL' if the name could not be determined. */
DFUNDEF char const *DCALL DeeObjMethod_Name(DeeObject *__restrict self);

struct clsmethod_object {
    OBJECT_HEAD /* Unbound member function (`classmethod') (may be invoked as a thiscall object). */
    dobjmethod_t        cm_func; /* [1..1] C-level object function. */
    DREF DeeTypeObject *cm_type; /* [1..1] The type that this-arguments must match. */
};
struct kwclsmethod_object {
    OBJECT_HEAD /* Unbound member function (`classmethod') (may be invoked as a thiscall object). */
    dkwobjmethod_t      cm_func; /* [1..1] C-level object function. */
    DREF DeeTypeObject *cm_type; /* [1..1] The type that this-arguments must match. */
};

DDATDEF DeeTypeObject DeeClsMethod_Type;
DDATDEF DeeTypeObject DeeKwClsMethod_Type;
#define DeeClsMethod_FUNC(x)       ((DeeClsMethodObject *)REQUIRES_OBJECT(x))->cm_func
#define DeeClsMethod_TYPE(x)       ((DeeClsMethodObject *)REQUIRES_OBJECT(x))->cm_type
#define DeeClsMethod_Check(x)        DeeObject_InstanceOfExact(x,&DeeClsMethod_Type)   /* `_classmethod' is `final'. */
#define DeeClsMethod_CheckExact(x)   DeeObject_InstanceOfExact(x,&DeeClsMethod_Type)
#define DeeKwClsMethod_Check(x)      DeeObject_InstanceOfExact(x,&DeeKwClsMethod_Type) /* `_kwclassmethod' is `final'. */
#define DeeKwClsMethod_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeKwClsMethod_Type)
#define DEFINE_CLSMETHOD(name,func,type) \
  DeeClsMethodObject name = { OBJECT_HEAD_INIT(&DeeClsMethod_Type), func, type }
#define DEFINE_KWCLSMETHOD(name,func,type) \
  DeeKwClsMethodObject name = { OBJECT_HEAD_INIT(&DeeKwClsMethod_Type), func, type }


/* Construct a new `classmethod' object. */
DFUNDEF DREF /*ClsMethod*/DeeObject *DCALL DeeClsMethod_New(DeeTypeObject *__restrict type, dobjmethod_t func);
DFUNDEF DREF /*KwClsMethod*/DeeObject *DCALL DeeKwClsMethod_New(DeeTypeObject *__restrict type, dkwobjmethod_t func);



struct clsproperty_object {
    OBJECT_HEAD
    dgetmethod_t        cp_get;  /* [0..1] Getter callback. */
    DREF DeeTypeObject *cp_type; /* [1..1] The type that this-arguments must match. */
    ddelmethod_t        cp_del;  /* [0..1] Delete callback. */
    dsetmethod_t        cp_set;  /* [0..1] Setter callback. */
};
DDATDEF DeeTypeObject DeeClsProperty_Type;

/* Create a new unbound class property object. */
DFUNDEF DREF /*ClsProperty*/DeeObject *DCALL
DeeClsProperty_New(DeeTypeObject *__restrict type,
                   struct type_getset const *__restrict desc);



struct clsmember_object {
    /* Proxy descriptor for instance members to-be accessed like class properties. */
    OBJECT_HEAD
    struct type_member  cm_memb; /* The member descriptor. */
    DREF DeeTypeObject *cm_type; /* [1..1] The type that this-arguments must match. */
};
DDATDEF DeeTypeObject DeeClsMember_Type;

/* Create a new unbound class property object. */
DFUNDEF DREF /*ClsMember*/DeeObject *DCALL
DeeClsMember_New(DeeTypeObject *__restrict type,
                 struct type_member const *__restrict desc);


/* Wrapper for a static C-method invoked through tp_call.
 * This type's main purpose is to be used by statically allocated
 * objects and is the type for some helper functions found in the
 * deemon module which the compiler generates call to in some rare
 * corner cases. */
struct cmethod_object {
    OBJECT_HEAD
    dcmethod_t   cm_func; /* [1..1][const] */
};
struct kwcmethod_object {
    OBJECT_HEAD
    dkwcmethod_t cm_func; /* [1..1][const] */
};
DDATDEF DeeTypeObject DeeCMethod_Type;
DDATDEF DeeTypeObject DeeKwCMethod_Type;
#define DeeCMethod_Check(x)        DeeObject_InstanceOfExact(x,&DeeCMethod_Type) /* `_cmethod' is `final'. */
#define DeeCMethod_CheckExact(x)   DeeObject_InstanceOfExact(x,&DeeCMethod_Type)
#define DeeKwCMethod_Check(x)      DeeObject_InstanceOfExact(x,&DeeKwCMethod_Type) /* `_kwcmethod' is `final'. */
#define DeeKwCMethod_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeKwCMethod_Type)
#define DEFINE_CMETHOD(name,func) \
  DeeCMethodObject name = { OBJECT_HEAD_INIT(&DeeCMethod_Type), func }
#define DEFINE_KWCMETHOD(name,func) \
  DeeKwCMethodObject name = { OBJECT_HEAD_INIT(&DeeKwCMethod_Type), func }


#ifdef CONFIG_BUILDING_DEEMON
/* Invoke a given c-function callback.
 * Since this is the main way through which external functions are invoked,
 * we use this point to add some debug checks for proper use of exceptions.
 * That means we assert that the exception depth is manipulated as follows:
 * >> ASSERT(old_except_depth == new_except_depth + (return == NULL ? 1 : 0));
 * This way we can quickly determine improper use of exceptions in most cases,
 * even before the interpreter would crash because it tried to handle exceptions
 * when there were none.
 * A very common culprit for improper exception propagation is the return value
 * of printf-style format functions. Usually, code can just check for a non-zero
 * return value to determine if an integer-returning function has failed.
 * However, printf-style format functions return a negative value when that
 * happens, but return the positive sum of all printer calls on success (which
 * unless nothing was printed, is also non-zero).
 * This can easily be fixed by replacing:
 * >> if (Dee_FormatPrintf(...)) goto err;
 * with this:
 * >> if (Dee_FormatPrintf(...) < 0) goto err;
 */
#ifdef NDEBUG
#define DeeCMethod_CallFunc(fun,argc,argv)              (*fun)(argc,argv)
#define DeeKwCMethod_CallFunc(fun,argc,argv,kw)         (*fun)(argc,argv,kw)
#define DeeObjMethod_CallFunc(fun,self,argc,argv)       (*fun)(self,argc,argv)
#define DeeKwObjMethod_CallFunc(fun,self,argc,argv,kw)  (*fun)(self,argc,argv,kw)
#else
#define DeeCMethod_CallFunc(fun,argc,argv)             DeeCMethod_CallFunc_d(fun,argc,argv)
#define DeeObjMethod_CallFunc(fun,self,argc,argv)      DeeObjMethod_CallFunc_d(fun,self,argc,argv)
#define DeeKwCMethod_CallFunc(fun,argc,argv,kw)        DeeKwCMethod_CallFunc_d(fun,argc,argv,kw)
#define DeeKwObjMethod_CallFunc(fun,self,argc,argv,kw) DeeKwObjMethod_CallFunc_d(fun,self,argc,argv,kw)
INTDEF DREF DeeObject *DCALL DeeCMethod_CallFunc_d(dcmethod_t fun, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL DeeObjMethod_CallFunc_d(dobjmethod_t fun, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL DeeKwCMethod_CallFunc_d(dkwcmethod_t fun, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL DeeKwObjMethod_CallFunc_d(dkwobjmethod_t fun, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#endif
#endif


#ifdef GUARD_DEEMON_SEQ_H
#ifdef CONFIG_BUILDING_DEEMON
#define DeeSeq_KeyIsID(key) ((DeeObject *)REQUIRES_OBJECT(key) == (DeeObject *)&_DeeObject_IdObjMethod)
INTDEF DeeClsMethodObject _DeeObject_IdObjMethod;
#else /* CONFIG_BUILDING_DEEMON */
#define DeeSeq_KeyIsID(key) ((key) && DeeClsMethod_Check(key) && DeeClsMethod_FUNC(key) == &_DeeObject_IdFunc)
DFUNDEF DREF DeeObject *DCALL
_DeeObject_IdFunc(DeeObject *__restrict self, size_t argc,
                  DeeObject **__restrict argv);
#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* GUARD_DEEMON_SEQ_H */

DECL_END

#endif /* !GUARD_DEEMON_OBJMETHOD_H */
