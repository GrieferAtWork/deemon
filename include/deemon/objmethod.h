/* Copyright (c) 2019 Griefer@Work                                            *
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

#include <stdarg.h>
#include <stddef.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_objmethod_object   objmethod_object
#define Dee_kwobjmethod_object kwobjmethod_object
#define Dee_clsmethod_object   clsmethod_object
#define Dee_kwclsmethod_object kwclsmethod_object
#define Dee_clsproperty_object clsproperty_object
#define Dee_clsmember_object   clsmember_object
#define Dee_cmethod_object     cmethod_object
#define Dee_kwcmethod_object   kwcmethod_object
#define DEFINE_OBJMETHOD       Dee_DEFINE_OBJMETHOD
#define DEFINE_KWOBJMETHOD     Dee_DEFINE_KWOBJMETHOD
#define DEFINE_CLSMETHOD       Dee_DEFINE_CLSMETHOD
#define DEFINE_KWCLSMETHOD     Dee_DEFINE_KWCLSMETHOD
#define DEFINE_CLSPROPERTY     Dee_DEFINE_CLSPROPERTY
#define DEFINE_CMETHOD         Dee_DEFINE_CMETHOD
#define DEFINE_KWCMETHOD       Dee_DEFINE_KWCMETHOD
#endif /* DEE_SOURCE */


typedef struct Dee_objmethod_object DeeObjMethodObject;
typedef struct Dee_kwobjmethod_object DeeKwObjMethodObject;
typedef struct Dee_clsmethod_object DeeClsMethodObject;
typedef struct Dee_kwclsmethod_object DeeKwClsMethodObject;
typedef struct Dee_clsproperty_object DeeClsPropertyObject;
typedef struct Dee_clsmember_object DeeClsMemberObject;
typedef struct Dee_cmethod_object DeeCMethodObject;
typedef struct Dee_kwcmethod_object DeeKwCMethodObject;

typedef DREF DeeObject *(DCALL *Dee_cmethod_t)(size_t argc, DeeObject **argv);
typedef DREF DeeObject *(DCALL *Dee_kwcmethod_t)(size_t argc, DeeObject **argv, DeeObject *kw);

#ifdef DEE_SOURCE
typedef Dee_cmethod_t   dcmethod_t;
typedef Dee_kwcmethod_t dkwcmethod_t;
#endif /* DEE_SOURCE */


struct Dee_objmethod_object {
	Dee_OBJECT_HEAD /* Object-bound member function. */
	Dee_objmethod_t   om_func;  /* [1..1][const] C-level object function. */
	DREF DeeObject   *om_this;  /* [1..1][const] The `self' argument passed to `om_func'. */
};
struct Dee_kwobjmethod_object {
	Dee_OBJECT_HEAD /* Object-bound member function. */
	Dee_kwobjmethod_t om_func;  /* [1..1][const] C-level object function. */
	DREF DeeObject   *om_this;  /* [1..1][const] The `self' argument passed to `om_func'. */
};
DDATDEF DeeTypeObject DeeObjMethod_Type;
DDATDEF DeeTypeObject DeeKwObjMethod_Type;
#define DeeObjMethod_SELF(x)       ((DeeObjMethodObject *)Dee_REQUIRES_OBJECT(x))->om_this
#define DeeObjMethod_FUNC(x)       ((DeeObjMethodObject *)Dee_REQUIRES_OBJECT(x))->om_func
#define DeeObjMethod_Check(x)        DeeObject_InstanceOfExact(x, &DeeObjMethod_Type) /* `objmethod' is final. */
#define DeeObjMethod_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeObjMethod_Type)
#define DeeKwObjMethod_Check(x)      DeeObject_InstanceOfExact(x, &DeeKwObjMethod_Type) /* `kwobjmethod' is final. */
#define DeeKwObjMethod_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeKwObjMethod_Type)
#define Dee_DEFINE_OBJMETHOD(name, func, self) \
	DeeObjMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeObjMethod_Type), func, self }
#define Dee_DEFINE_KWOBJMETHOD(name, func, self) \
	DeeKwObjMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeKwObjMethod_Type), func, self }

/* Construct a new `objmethod' object. */
DFUNDEF DREF DeeObject *DCALL DeeObjMethod_New(Dee_objmethod_t func, DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeKwObjMethod_New(Dee_kwobjmethod_t func, DeeObject *__restrict self);

/* Returns the name of the function bound by the given
 * objmethod, or `NULL' if the name could not be determined. */
DFUNDEF char const *DCALL DeeObjMethod_GetName(DeeObject *__restrict self);
DFUNDEF char const *DCALL DeeObjMethod_GetDoc(DeeObject *__restrict self);
/* Returns the type that is implementing the bound method. */
DFUNDEF ATTR_RETNONNULL DeeTypeObject *DCALL DeeObjMethod_GetType(DeeObject *__restrict self);

struct Dee_clsmethod_object {
	Dee_OBJECT_HEAD /* Unbound member function (`classmethod') (may be invoked as a thiscall object). */
	Dee_objmethod_t     cm_func; /* [1..1] C-level object function. */
	DREF DeeTypeObject *cm_type; /* [1..1] The type that this-arguments must match. */
};

struct Dee_kwclsmethod_object {
	Dee_OBJECT_HEAD /* Unbound member function (`classmethod') (may be invoked as a thiscall object). */
	Dee_kwobjmethod_t   cm_func; /* [1..1] C-level object function. */
	DREF DeeTypeObject *cm_type; /* [1..1] The type that this-arguments must match. */
};

DDATDEF DeeTypeObject DeeClsMethod_Type;
DDATDEF DeeTypeObject DeeKwClsMethod_Type;
#define DeeClsMethod_FUNC(x)       ((DeeClsMethodObject *)Dee_REQUIRES_OBJECT(x))->cm_func
#define DeeClsMethod_TYPE(x)       ((DeeClsMethodObject *)Dee_REQUIRES_OBJECT(x))->cm_type
#define DeeClsMethod_Check(x)        DeeObject_InstanceOfExact(x, &DeeClsMethod_Type)   /* `_classmethod' is final. */
#define DeeClsMethod_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeClsMethod_Type)
#define DeeKwClsMethod_Check(x)      DeeObject_InstanceOfExact(x, &DeeKwClsMethod_Type) /* `_kwclassmethod' is final. */
#define DeeKwClsMethod_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeKwClsMethod_Type)
#define Dee_DEFINE_CLSMETHOD(name, func, type) \
	DeeClsMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeClsMethod_Type), func, type }
#define Dee_DEFINE_KWCLSMETHOD(name, func, type) \
	DeeKwClsMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeKwClsMethod_Type), func, type }


/* Construct a new `classmethod' object. */
DFUNDEF DREF /*ClsMethod*/ DeeObject *DCALL DeeClsMethod_New(DeeTypeObject *__restrict type, Dee_objmethod_t func);
DFUNDEF DREF /*KwClsMethod*/ DeeObject *DCALL DeeKwClsMethod_New(DeeTypeObject *__restrict type, Dee_kwobjmethod_t func);

/* Returns the name of the function bound by the given
 * clsmethod, or `NULL' if the name could not be determined. */
DFUNDEF char const *DCALL DeeClsMethod_GetName(DeeObject *__restrict self);
DFUNDEF char const *DCALL DeeClsMethod_GetDoc(DeeObject *__restrict self);



struct Dee_clsproperty_object {
	Dee_OBJECT_HEAD
	Dee_getmethod_t     cp_get;  /* [0..1] Getter callback. */
	DREF DeeTypeObject *cp_type; /* [1..1] The type that this-arguments must match. */
	Dee_delmethod_t     cp_del;  /* [0..1] Delete callback. */
	Dee_setmethod_t     cp_set;  /* [0..1] Setter callback. */
};

DDATDEF DeeTypeObject DeeClsProperty_Type;
#define DeeClsProperty_TYPE(x)         ((DeeClsPropertyObject *)Dee_REQUIRES_OBJECT(x))->cp_type
#define DeeClsProperty_GET(x)          ((DeeClsPropertyObject *)Dee_REQUIRES_OBJECT(x))->cp_get
#define DeeClsProperty_DEL(x)          ((DeeClsPropertyObject *)Dee_REQUIRES_OBJECT(x))->cp_del
#define DeeClsProperty_SET(x)          ((DeeClsPropertyObject *)Dee_REQUIRES_OBJECT(x))->cp_set
#define DeeClsProperty_Check(x)        DeeObject_InstanceOfExact(x, &DeeClsProperty_Type) /* `_classproperty' is final. */
#define DeeClsProperty_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeClsProperty_Type)
#define Dee_DEFINE_CLSPROPERTY(name, type, get, del, set) \
	DeeClsPropertyObject name = { Dee_OBJECT_HEAD_INIT(&DeeClsProperty_Type), get, type, del, set }


/* Create a new unbound class property object. */
DFUNDEF DREF /*ClsProperty*/ DeeObject *DCALL
DeeClsProperty_New(DeeTypeObject *__restrict type,
                   Dee_getmethod_t get,
                   Dee_delmethod_t del,
                   Dee_setmethod_t set);

/* Returns the name of the function bound by the given
 * clsproperty, or `NULL' if the name could not be determined. */
DFUNDEF char const *DCALL DeeClsProperty_GetName(DeeObject *__restrict self);
DFUNDEF char const *DCALL DeeClsProperty_GetDoc(DeeObject *__restrict self);



struct Dee_clsmember_object {
	/* Proxy descriptor for instance members to-be accessed like class properties. */
	Dee_OBJECT_HEAD
	struct Dee_type_member cm_memb; /* The member descriptor. */
	DREF DeeTypeObject    *cm_type; /* [1..1] The type that this-arguments must match. */
};
DDATDEF DeeTypeObject DeeClsMember_Type;

/* Create a new unbound class property object. */
DFUNDEF DREF /*ClsMember*/ DeeObject *DCALL
DeeClsMember_New(DeeTypeObject *__restrict type,
                 struct Dee_type_member const *__restrict desc);


/* Wrapper for a static C-method invoked through tp_call.
 * This type's main purpose is to be used by statically allocated
 * objects and is the type for some helper functions found in the
 * deemon module which the compiler generates call to in some rare
 * corner cases. */
struct Dee_cmethod_object {
	Dee_OBJECT_HEAD
	Dee_cmethod_t   cm_func; /* [1..1][const] */
};
struct Dee_kwcmethod_object {
	Dee_OBJECT_HEAD
	Dee_kwcmethod_t cm_func; /* [1..1][const] */
};
DDATDEF DeeTypeObject DeeCMethod_Type;
DDATDEF DeeTypeObject DeeKwCMethod_Type;
#define DeeCMethod_FUNC(x)         ((DeeCMethodObject *)Dee_REQUIRES_OBJECT(x))->cm_func
#define DeeCMethod_Check(x)        DeeObject_InstanceOfExact(x, &DeeCMethod_Type) /* `_cmethod' is final. */
#define DeeCMethod_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeCMethod_Type)
#define DeeKwCMethod_Check(x)      DeeObject_InstanceOfExact(x, &DeeKwCMethod_Type) /* `_kwcmethod' is final. */
#define DeeKwCMethod_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeKwCMethod_Type)
#define Dee_DEFINE_CMETHOD(name, func) \
	DeeCMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeCMethod_Type), func }
#define Dee_DEFINE_KWCMETHOD(name, func) \
	DeeKwCMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeKwCMethod_Type), func }


#ifdef CONFIG_BUILDING_DEEMON
/* Invoke a given c-function callback.
 * Since this is the main way through which external functions are invoked,
 * we use this point to add some debug checks for proper use of exceptions.
 * That means we assert that the exception depth is manipulated as follows:
 * >> Dee_ASSERT(old_except_depth == new_except_depth + (return == NULL ? 1 : 0));
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
 * >> if (DeeFormat_Printf(...)) goto err;
 * with this:
 * >> if (DeeFormat_Printf(...) < 0) goto err;
 */
#ifdef NDEBUG
#define DeeCMethod_CallFunc(fun, argc, argv)               (*fun)(argc, argv)
#define DeeKwCMethod_CallFunc(fun, argc, argv, kw)         (*fun)(argc, argv, kw)
#define DeeObjMethod_CallFunc(fun, self, argc, argv)       (*fun)(self, argc, argv)
#define DeeKwObjMethod_CallFunc(fun, self, argc, argv, kw) (*fun)(self, argc, argv, kw)
#else /* NDEBUG */
#define DeeCMethod_CallFunc(fun, argc, argv)               DeeCMethod_CallFunc_d(fun, argc, argv)
#define DeeObjMethod_CallFunc(fun, self, argc, argv)       DeeObjMethod_CallFunc_d(fun, self, argc, argv)
#define DeeKwCMethod_CallFunc(fun, argc, argv, kw)         DeeKwCMethod_CallFunc_d(fun, argc, argv, kw)
#define DeeKwObjMethod_CallFunc(fun, self, argc, argv, kw) DeeKwObjMethod_CallFunc_d(fun, self, argc, argv, kw)
INTDEF DREF DeeObject *DCALL DeeCMethod_CallFunc_d(Dee_cmethod_t fun, size_t argc, DeeObject **argv);
INTDEF DREF DeeObject *DCALL DeeObjMethod_CallFunc_d(Dee_objmethod_t fun, DeeObject *__restrict self, size_t argc, DeeObject **argv);
INTDEF DREF DeeObject *DCALL DeeKwCMethod_CallFunc_d(Dee_kwcmethod_t fun, size_t argc, DeeObject **argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL DeeKwObjMethod_CallFunc_d(Dee_kwobjmethod_t fun, DeeObject *__restrict self, size_t argc, DeeObject **argv, DeeObject *kw);
#endif /* !NDEBUG */
#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_OBJMETHOD_H */
