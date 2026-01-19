/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJMETHOD_H
#define GUARD_DEEMON_OBJMETHOD_H 1

#include "api.h"

#include "types.h"
#ifndef __INTELLISENSE__
#include "object.h"
#endif /* !__INTELLISENSE__ */
/**/

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_objmethod_origin   objmethod_origin
#define Dee_objmethod_object   objmethod_object
#define Dee_clsmethod_object   clsmethod_object
#define Dee_clsproperty_origin clsproperty_origin
#define Dee_clsproperty_object clsproperty_object
#define Dee_clsmember_object   clsmember_object
#define Dee_cmethod_origin     cmethod_origin
#define Dee_cmethod_object     cmethod_object
#define Dee_module_object      module_object
#define DEFINE_OBJMETHOD       Dee_DEFINE_OBJMETHOD
#define DEFINE_KWOBJMETHOD     Dee_DEFINE_KWOBJMETHOD
#define DEFINE_CLSMETHOD       Dee_DEFINE_CLSMETHOD
#define DEFINE_KWCLSMETHOD     Dee_DEFINE_KWCLSMETHOD
#define DEFINE_CLSPROPERTY     Dee_DEFINE_CLSPROPERTY
#define DEFINE_CMETHOD         Dee_DEFINE_CMETHOD
#define DEFINE_CMETHOD0        Dee_DEFINE_CMETHOD0
#define DEFINE_CMETHOD1        Dee_DEFINE_CMETHOD1
#define DEFINE_KWCMETHOD       Dee_DEFINE_KWCMETHOD
#endif /* DEE_SOURCE */


typedef struct Dee_objmethod_object DeeObjMethodObject;
typedef struct Dee_clsmethod_object DeeClsMethodObject;
typedef struct Dee_clsproperty_object DeeClsPropertyObject;
typedef struct Dee_clsmember_object DeeClsMemberObject;
typedef struct Dee_cmethod_object DeeCMethodObject;

typedef WUNUSED_T ATTR_INS_T(2, 1) DREF DeeObject *(DCALL *Dee_cmethod_t)(size_t argc, DeeObject *const *argv);
typedef WUNUSED_T ATTR_INS_T(2, 1) DREF DeeObject *(DCALL *Dee_kwcmethod_t)(size_t argc, DeeObject *const *argv, DeeObject *kw);
typedef WUNUSED_T                  DREF DeeObject *(DCALL *Dee_cmethod0_t)(void);
typedef WUNUSED_T NONNULL_T((1))   DREF DeeObject *(DCALL *Dee_cmethod1_t)(DeeObject *__restrict arg0);

#if defined(__INTELLISENSE__) && defined(__cplusplus)
/* Highlight usage errors in IDE */
extern "C++" {
namespace __intern {
Dee_cmethod_t _Dee_RequiresCMethod(decltype(nullptr));
Dee_kwcmethod_t _Dee_RequiresKwCMethod(decltype(nullptr));
Dee_cmethod0_t _Dee_RequiresCMethod0(decltype(nullptr));
Dee_cmethod1_t _Dee_RequiresCMethod1(decltype(nullptr));
template<class _TReturn, class _TObject> Dee_cmethod_t _Dee_RequiresCMethod(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(size_t, _TObject *const *));
template<class _TReturn, class _TObject> Dee_kwcmethod_t _Dee_RequiresKwCMethod(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(size_t, _TObject *const *, /*nullable*/ DeeObject *kw));
template<class _TReturn> Dee_cmethod0_t _Dee_RequiresCMethod0(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(void));
template<class _TReturn, class _TObject> Dee_cmethod1_t _Dee_RequiresCMethod1(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(_TObject *));
} /* namespace __intern */
} /* extern "C++" */
#define Dee_REQUIRES_CMETHOD(meth)    ((decltype(::__intern::_Dee_RequiresCMethod(meth)))(meth))
#define Dee_REQUIRES_KWCMETHOD(meth)  ((decltype(::__intern::_Dee_RequiresKwCMethod(meth)))(meth))
#define Dee_REQUIRES_CMETHOD0(meth)   ((decltype(::__intern::_Dee_RequiresCMethod0(meth)))(meth))
#define Dee_REQUIRES_CMETHOD1(meth)   ((decltype(::__intern::_Dee_RequiresCMethod1(meth)))(meth))
#else /* __INTELLISENSE__ && __cplusplus */
#define Dee_REQUIRES_CMETHOD(meth)    ((Dee_cmethod_t)(meth))
#define Dee_REQUIRES_KWCMETHOD(meth)  ((Dee_kwcmethod_t)(meth))
#define Dee_REQUIRES_CMETHOD0(meth)   ((Dee_cmethod0_t)(meth))
#define Dee_REQUIRES_CMETHOD1(meth)   ((Dee_cmethod1_t)(meth))
#endif /* !__INTELLISENSE__ || !__cplusplus */


struct Dee_objmethod_origin {
	DeeTypeObject                *omo_type;  /* [1..1] Declaring type. */
	struct Dee_type_method const *omo_chain; /* [1..1] Method declaration chain (either `omo_type->tp_methods' or `omo_type->tp_class_methods'). */
	struct Dee_type_method const *omo_decl;  /* [1..1] Method declaration in question. */
};

struct Dee_objmethod_object {
	Dee_OBJECT_HEAD /* Object-bound member function. */
	DREF DeeObject   *om_this;  /* [1..1][const] The `self' argument passed to `om_func'. */
	union {
		Dee_objmethod_t   omf_meth;   /* [1..1][valid_if(ob_type == &DeeObjMethod_Type)][const] Method pointer. */
		Dee_kwobjmethod_t omf_kwmeth; /* [1..1][valid_if(ob_type == &DeeKwObjMethod_Type)][const] Method pointer. */
	} om_func;
};

DDATDEF DeeTypeObject DeeObjMethod_Type;
DDATDEF DeeTypeObject DeeKwObjMethod_Type;
#define DeeObjMethod_SELF(x)         Dee_REQUIRES_OBJECT(DeeObjMethodObject, x)->om_this
#define DeeObjMethod_FUNC(x)         Dee_REQUIRES_OBJECT(DeeObjMethodObject, x)->om_func.omf_meth
#define DeeObjMethod_Check(x)        DeeObject_InstanceOfExact(x, &DeeObjMethod_Type) /* `_ObjMethod' is final. */
#define DeeObjMethod_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeObjMethod_Type)
#define DeeKwObjMethod_Check(x)      DeeObject_InstanceOfExact(x, &DeeKwObjMethod_Type) /* `_KwObjMethod' is final. */
#define DeeKwObjMethod_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeKwObjMethod_Type)
#define Dee_DEFINE_OBJMETHOD(name, func, self) \
	DeeObjMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeObjMethod_Type), self, Dee_REQUIRES_OBJMETHOD(func) }
#define Dee_DEFINE_KWOBJMETHOD(name, func, self) \
	DeeObjMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeKwObjMethod_Type), self, Dee_REQUIRES_KWOBJMETHOD(func) }

/* Construct a new `_ObjMethod' object. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObjMethod_New(Dee_objmethod_t func, DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwObjMethod_New(Dee_kwobjmethod_t func, DeeObject *__restrict self);

/* Lookup the origin of the function bound by
 * the given `_ObjMethod', or `NULL' if unknown. */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeObjMethod_GetOrigin(DeeObject const *__restrict self,
                       struct Dee_objmethod_origin *__restrict result);
#define DeeKwObjMethod_GetOrigin(self, result) \
	DeeObjMethod_GetOrigin(self, result)

struct Dee_clsmethod_object {
	/* Unbound member function (`ClassMethod') (may be invoked as a thiscall object). */
	Dee_OBJECT_HEAD
	DREF DeeTypeObject *clm_type; /* [1..1] The type that this-arguments must match. */
	union {
		Dee_objmethod_t   clmf_meth;   /* [1..1][valid_if(ob_type == &DeeClsMethod_Type)][const] Method pointer. */
		Dee_kwobjmethod_t clmf_kwmeth; /* [1..1][valid_if(ob_type == &DeeKwClsMethod_Type)][const] Method pointer. */
	} clm_func;
};

DDATDEF DeeTypeObject DeeClsMethod_Type;
DDATDEF DeeTypeObject DeeKwClsMethod_Type;
#define DeeClsMethod_FUNC(x)         Dee_REQUIRES_OBJECT(DeeClsMethodObject, x)->clm_func.clmf_meth
#define DeeClsMethod_TYPE(x)         Dee_REQUIRES_OBJECT(DeeClsMethodObject, x)->clm_type
#define DeeClsMethod_Check(x)        DeeObject_InstanceOfExact(x, &DeeClsMethod_Type)   /* `_ClassMethod' is final. */
#define DeeClsMethod_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeClsMethod_Type)
#define DeeKwClsMethod_Check(x)      DeeObject_InstanceOfExact(x, &DeeKwClsMethod_Type) /* `_KwClassMethod' is final. */
#define DeeKwClsMethod_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeKwClsMethod_Type)
#define Dee_DEFINE_CLSMETHOD(name, func, type) \
	DeeClsMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeClsMethod_Type), type, { Dee_REQUIRES_OBJMETHOD(func) } }
#define Dee_DEFINE_KWCLSMETHOD(name, func, type) \
	DeeClsMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeKwClsMethod_Type), type, { (Dee_objmethod_t)Dee_REQUIRES_KWOBJMETHOD(func) } }


/* Construct a new `_ClassMethod' object. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*ClsMethod*/ DeeObject *DCALL
DeeClsMethod_New(DeeTypeObject *__restrict type, Dee_objmethod_t func);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*KwClsMethod*/ DeeObject *DCALL
DeeKwClsMethod_New(DeeTypeObject *__restrict type, Dee_kwobjmethod_t func);

/* Lookup the origin of the function bound by
 * the given `_ClsMethod', or `NULL' if unknown. */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeClsMethod_GetOrigin(DeeObject const *__restrict self,
                       struct Dee_objmethod_origin *__restrict result);
#define DeeKwClsMethod_GetOrigin(self, result) \
	DeeClsMethod_GetOrigin(self, result)



struct Dee_clsproperty_origin {
	DeeTypeObject                *cpo_type;  /* [1..1] Declaring type. */
	struct Dee_type_getset const *cpo_chain; /* [1..1] Method declaration chain (either `cpo_type->tp_methods' or `cpo_type->tp_class_methods'). */
	struct Dee_type_getset const *cpo_decl;  /* [1..1] Method declaration in question. */
};

struct Dee_clsproperty_object {
	Dee_OBJECT_HEAD
	DREF DeeTypeObject *cp_type;  /* [1..1] The type that this-arguments must match. */
	Dee_getmethod_t     cp_get;   /* [0..1] Getter callback. */
	Dee_delmethod_t     cp_del;   /* [0..1] Delete callback. */
	Dee_setmethod_t     cp_set;   /* [0..1] Setter callback. */
	Dee_boundmethod_t   cp_bound; /* [0..1] Is-bound callback. */
};

DDATDEF DeeTypeObject DeeClsProperty_Type;
#define DeeClsProperty_TYPE(x)       Dee_REQUIRES_OBJECT(DeeClsPropertyObject, x)->cp_type
#define DeeClsProperty_GET(x)        Dee_REQUIRES_OBJECT(DeeClsPropertyObject, x)->cp_get
#define DeeClsProperty_DEL(x)        Dee_REQUIRES_OBJECT(DeeClsPropertyObject, x)->cp_del
#define DeeClsProperty_SET(x)        Dee_REQUIRES_OBJECT(DeeClsPropertyObject, x)->cp_set
#define DeeClsProperty_BOUND(x)      Dee_REQUIRES_OBJECT(DeeClsPropertyObject, x)->cp_bound
#define DeeClsProperty_Check(x)      DeeObject_InstanceOfExact(x, &DeeClsProperty_Type) /* `_ClassProperty' is final. */
#define DeeClsProperty_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeClsProperty_Type)
#define Dee_DEFINE_CLSPROPERTY(name, type, get, del, set, bound)              \
	DeeClsPropertyObject name = { Dee_OBJECT_HEAD_INIT(&DeeClsProperty_Type), \
	                              type,                                       \
	                              Dee_REQUIRES_GETMETHOD(get),                \
	                              Dee_REQUIRES_DELMETHOD(del),                \
	                              Dee_REQUIRES_SETMETHOD(set),                \
	                              Dee_REQUIRES_BOUNDMETHOD(bound) }


/* Create a new unbound class property object. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*ClsProperty*/ DeeObject *
(DCALL DeeClsProperty_New)(DeeTypeObject *__restrict type,
                           struct Dee_type_getset const *getset);
DFUNDEF WUNUSED NONNULL((1)) DREF /*ClsProperty*/ DeeObject *DCALL
DeeClsProperty_NewEx(DeeTypeObject *__restrict type,
                     Dee_getmethod_t get, Dee_delmethod_t del,
                     Dee_setmethod_t set, Dee_boundmethod_t bound);
#ifndef __OPTIMIZE_SIZE__
#define DeeClsProperty_New(type, getset) \
	DeeClsProperty_NewEx(type, (getset)->gs_get, (getset)->gs_del, (getset)->gs_set, (getset)->gs_bound)
#endif /* !__OPTIMIZE_SIZE__ */

/* Lookup the origin of the function bound by
 * the given `_ClassProperty', or `NULL' if unknown. */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeClsProperty_GetOrigin(DeeObject const *__restrict self,
                         struct Dee_clsproperty_origin *__restrict result);



struct Dee_clsmember_object {
	/* Proxy descriptor for instance members to-be accessed like class properties. */
	Dee_OBJECT_HEAD
	DREF DeeTypeObject    *cmb_type; /* [1..1] The type that this-arguments must match. */
	struct Dee_type_member cmb_memb; /* The member descriptor. */
};
DDATDEF DeeTypeObject DeeClsMember_Type;

/* Create a new unbound class member object. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*ClsMember*/ DeeObject *DCALL
DeeClsMember_New(DeeTypeObject *__restrict type,
                 struct Dee_type_member const *desc);


/* Wrapper for a static C-method invoked through tp_call.
 * This type's main purpose is to be used by statically allocated
 * objects and is the type for some helper functions found in the
 * deemon module which the compiler generates call to in some rare
 * corner cases. */
struct Dee_module_object;
struct Dee_cmethod_origin {
	DREF struct Dee_module_object *cmo_module; /* [0..1] Module containing the C-method. */
	struct module_symbol          *cmo_modsym; /* [0..1] Module symbol that exposes `func'. */
	DREF DeeTypeObject            *cmo_type;   /* [0..1] Type containing the C-method. */
	struct type_member const      *cmo_member; /* [0..1] Member of `cmo_type' that exposes the C-method. */
	char const                    *cmo_name;   /* [1..1] Function name. */
	char const                    *cmo_doc;    /* [0..1] Function doc string. */
};

/* Search for the origin of "func", filling in `self'
 * @return: true:  Success (`self' was initialized)
 * @return: false: Failure (origin of `func' could not be determined) */
DFUNDEF WUNUSED NONNULL((1, 2)) bool DCALL
Dee_cmethod_origin_init(struct Dee_cmethod_origin *__restrict self,
                        Dee_cmethod_t func);
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1)) void DCALL
Dee_cmethod_origin_fini(struct Dee_cmethod_origin *__restrict self);
#else /* __INTELLISENSE__ */
#define Dee_cmethod_origin_fini(self) \
	(Dee_XDecref((self)->cmo_module), \
	 Dee_XDecref((self)->cmo_type))
#endif /* !__INTELLISENSE__ */

struct Dee_cmethod_object {
	Dee_OBJECT_HEAD
	uintptr_t cm_flags; /* [const] Method flags (set of `Dee_METHOD_F*') */
	union {
		Dee_cmethod_t   cmf_meth;   /* [1..1][valid_if(ob_type == &DeeCMethod_Type)][const] Method pointer. */
		Dee_kwcmethod_t cmf_kwmeth; /* [1..1][valid_if(ob_type == &DeeKwCMethod_Type)][const] Method pointer. */
		Dee_cmethod0_t  cmf_meth0;  /* [1..1][valid_if(ob_type == &DeeCMethod0_Type)][const] Method pointer. */
		Dee_cmethod1_t  cmf_meth1;  /* [1..1][valid_if(ob_type == &DeeCMethod1_Type)][const] Method pointer. */
	} cm_func;
};

DDATDEF DeeTypeObject DeeCMethod_Type;
DDATDEF DeeTypeObject DeeKwCMethod_Type;
DDATDEF DeeTypeObject DeeCMethod0_Type;
DDATDEF DeeTypeObject DeeCMethod1_Type;
#define DeeCMethod_FUNC(x)         Dee_REQUIRES_OBJECT(DeeCMethodObject, x)->cm_func.cmf_meth
#define DeeCMethod_Check(x)        DeeObject_InstanceOfExact(x, &DeeCMethod_Type) /* `_CMethod' is final. */
#define DeeCMethod_CheckExact(x)   DeeObject_InstanceOfExact(x, &DeeCMethod_Type)
#define DeeKwCMethod_Check(x)      DeeObject_InstanceOfExact(x, &DeeKwCMethod_Type) /* `_KwCMethod' is final. */
#define DeeKwCMethod_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeKwCMethod_Type)
#define DeeCMethod0_Check(x)       DeeObject_InstanceOfExact(x, &DeeCMethod0_Type) /* `_CMethod0' is final. */
#define DeeCMethod0_CheckExact(x)  DeeObject_InstanceOfExact(x, &DeeCMethod0_Type)
#define DeeCMethod1_Check(x)       DeeObject_InstanceOfExact(x, &DeeCMethod1_Type) /* `_CMethod1' is final. */
#define DeeCMethod1_CheckExact(x)  DeeObject_InstanceOfExact(x, &DeeCMethod1_Type)

#define Dee_DEFINE_CMETHOD(name, func, flags) \
	DeeCMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeCMethod_Type), flags, { Dee_REQUIRES_CMETHOD(func) } }
#define Dee_DEFINE_KWCMETHOD(name, func, flags) \
	DeeCMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeKwCMethod_Type), flags, { (Dee_cmethod_t)Dee_REQUIRES_KWCMETHOD(func) } }
#define Dee_DEFINE_CMETHOD0(name, func, flags) \
	DeeCMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeCMethod0_Type), flags, { (Dee_cmethod_t)Dee_REQUIRES_CMETHOD0(func) } }
#define Dee_DEFINE_CMETHOD1(name, func, flags) \
	DeeCMethodObject name = { Dee_OBJECT_HEAD_INIT(&DeeCMethod1_Type), flags, { (Dee_cmethod_t)Dee_REQUIRES_CMETHOD1(func) } }

/* Helpers for dynamically creating C method wrapper objects.
 * You really shouldn't use these (unless you *really* need to
 * wrap functions dynamically with no way of creating CMETHOD
 * objects statically). These are really only here to allow
 * for portable bindings of the deemon API in languages other
 * than C.
 *
 * If at all possible, use `Dee_DEFINE_CMETHOD' instead! */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCMethod_New(Dee_cmethod_t func, uintptr_t flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeKwCMethod_New(Dee_kwcmethod_t func, uintptr_t flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCMethod0_New(Dee_cmethod0_t func, uintptr_t flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCMethod1_New(Dee_cmethod1_t func, uintptr_t flags);

/* Try to figure out information about the origin of `self' */
#define DeeCMethod_GetOrigin(self, result)   Dee_cmethod_origin_init(result, DeeCMethod_FUNC(self))
#define DeeKwCMethod_GetOrigin(self, result) Dee_cmethod_origin_init(result, DeeCMethod_FUNC(self))
#define DeeCMethod0_GetOrigin(self, result)  Dee_cmethod_origin_init(result, DeeCMethod_FUNC(self))
#define DeeCMethod1_GetOrigin(self, result)  Dee_cmethod_origin_init(result, DeeCMethod_FUNC(self))


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
#if defined(CONFIG_BUILDING_DEEMON) && !defined(NDEBUG)
#define DeeCMethod_CallFunc(funptr, argc, argv)               DeeCMethod_CallFunc_d(funptr, argc, argv)
#define DeeCMethod0_CallFunc(funptr)                          DeeCMethod0_CallFunc_d(funptr)
#define DeeCMethod1_CallFunc(funptr, arg0)                    DeeCMethod1_CallFunc_d(funptr, arg0)
#define DeeKwCMethod_CallFunc(funptr, argc, argv, kw)         DeeKwCMethod_CallFunc_d(funptr, argc, argv, kw)
#define DeeObjMethod_CallFunc(funptr, self, argc, argv)       DeeObjMethod_CallFunc_d(funptr, self, argc, argv)
#define DeeKwObjMethod_CallFunc(funptr, self, argc, argv, kw) DeeKwObjMethod_CallFunc_d(funptr, self, argc, argv, kw)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCMethod_CallFunc_d(Dee_cmethod_t funptr, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCMethod0_CallFunc_d(Dee_cmethod0_t funptr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeCMethod1_CallFunc_d(Dee_cmethod1_t funptr, DeeObject *arg0);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeKwCMethod_CallFunc_d(Dee_kwcmethod_t funptr, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObjMethod_CallFunc_d(Dee_objmethod_t funptr, DeeObject *thisarg, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeKwObjMethod_CallFunc_d(Dee_kwobjmethod_t funptr, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
#endif /* CONFIG_BUILDING_DEEMON && !NDEBUG */

/* Helpers for calling native deemon function with format arguments. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeCMethod_CallFuncf(Dee_cmethod_t funptr, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeKwCMethod_CallFuncf(Dee_kwcmethod_t funptr, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DeeObjMethod_CallFuncf(Dee_objmethod_t funptr, DeeObject *thisarg, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DeeKwObjMethod_CallFuncf(Dee_kwobjmethod_t funptr, DeeObject *thisarg, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeCMethod_VCallFuncf(Dee_cmethod_t funptr, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeKwCMethod_VCallFuncf(Dee_kwcmethod_t funptr, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObjMethod_VCallFuncf(Dee_objmethod_t funptr, DeeObject *thisarg, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeKwObjMethod_VCallFuncf(Dee_kwobjmethod_t funptr, DeeObject *thisarg, char const *__restrict format, va_list args);

#ifndef DeeCMethod_CallFunc
#define DeeCMethod_CallFunc(funptr, argc, argv)               (*funptr)(argc, argv)
#define DeeCMethod0_CallFunc(funptr)                          (*funptr)()
#define DeeCMethod1_CallFunc(funptr, arg0)                    (*funptr)(arg0)
#define DeeKwCMethod_CallFunc(funptr, argc, argv, kw)         (*funptr)(argc, argv, kw)
#define DeeObjMethod_CallFunc(funptr, self, argc, argv)       (*funptr)(self, argc, argv)
#define DeeKwObjMethod_CallFunc(funptr, self, argc, argv, kw) (*funptr)(self, argc, argv, kw)
#endif /* !DeeCMethod_CallFunc */


DECL_END

#endif /* !GUARD_DEEMON_OBJMETHOD_H */
