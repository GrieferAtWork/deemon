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
/*!export **/
/*!export DeeTypeMRO_**/
/*!export DeeTypeType_**/
/*!export DeeType_**/
/*!export DeeType_mro_foreach_**/
/*!export Dee_ASSERT_OBJECT_**/
/*!export DeeObject_**/
/*!export Dee_GC_PRIORITY_**/
/*!export Dee_INT_**/
/*!export Dee_METHOD_FCONSTCALL_**/
/*!export Dee_OPERATOR_**/
/*!export -_Dee_PRIVATE_**/
/*!export Dee_REQUIRES_**/
/*!export Dee_STRUCT_**/
/*!export Dee_TF_**/
/*!export Dee_TYPE_GETSET**/
/*!export Dee_TYPE_GETTER**/
/*!export Dee_TYPE_*METHOD**/
/*!export Dee_TYPE_MEMBER_**/
/*!export Dee_TYPE_OPERATOR_**/
/*!export Dee_Visit**/
/*!export Dee_XVisit**/
/*!export Dee_*method_t*/
/*!export Dee_operator_**/
/*!export Dee_type_**/
/*!export type_**/
/*!export FAKE_OPERATOR_**/
/*!export INT_**/
/*!export METHOD_FCONSTCALL_**/
/*!export METHOD_F**/
/*!export OPCC_**/
/*!export OPERATOR_**/
/*!export STRUCT_**/
/*!export TF_**/
/*!export TP_F**/
/*!export TYPE_GETSET**/
/*!export TYPE_GETTER**/
/*!export TYPE_*METHOD**/
/*!export TYPE_MEMBER**/
/*!export TYPE_OPERATOR_**/
/*!export TYPE_**/
/*!export Dee_TYPE_CONSTRUCTOR_INIT_**/
#ifndef GUARD_DEEMON_TYPE_H
#define GUARD_DEEMON_TYPE_H 1 /*!export-*/

#include "api.h"

#include <hybrid/__atomic.h>  /* __ATOMIC_SEQ_CST, __hybrid_atomic_cmpxch */
#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_BIG_ENDIAN__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/host.h>      /* __pic__ */
#include <hybrid/typecore.h>  /* __*_TYPE__, __SIZEOF_*__ */

#include "types.h"        /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, DeeType_Extends, Dee_AsObject, Dee_OBJECT_HEAD, Dee_OBJECT_OFFSETOF_DATA, Dee_REQUIRES_OBJECT, Dee_SIZEOF_HASH_T, Dee_TYPE, Dee_WEAKREF_SUPPORT, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_funptr_t, Dee_hash_t, Dee_ssize_t */
#include "util/weakref.h" /* Dee_weakref */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* UINTn_C, int32_t, int64_t, uint16_t, uint32_t, uint64_t, uintptr_t */

/* To satisfy "fixincludes" (these includes are intentionally missing) */
/*!fixincludes fake_include "alloc.h"  // CONFIG_FIXED_ALLOCATOR_S_IS_AUTO, DeeObject_Free, DeeObject_Malloc, DeeSlab_Invoke */
/*!fixincludes fake_include "gc.h"     // DeeGCObject_Free, DeeGCObject_Malloc */
/*!fixincludes fake_include "object.h" // DeeAssert_BadObjectType, DeeAssert_BadObjectTypeOpt, DeeObject_Check, Dee_Incref */
/*!fixincludes fake_include "string.h" // DeeString_Hash, DeeString_STR */
/*!fixincludes fake_include "tuple.h"  // DeeTuple_ELEM, DeeTuple_SIZE */

#ifndef __INTELLISENSE__
#include "util/hash.h" /* Dee_HashPtr, Dee_HashStr */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_type_method      type_method
#define Dee_type_getset      type_getset
#define Dee_type_member      type_member
#define Dee_type_method_hint type_method_hint /*!export-*/
#define Dee_type_gc          type_gc
#define Dee_type_math        type_math
#define Dee_type_cmp         type_cmp
#define Dee_type_seq         type_seq
#define Dee_type_iterator    type_iterator
#define Dee_type_attr        type_attr
#define Dee_type_with        type_with
#define Dee_type_callable    type_callable
#define Dee_type_buffer      type_buffer
#define Dee_type_operator    type_operator
struct type_method_hint; /* Needed so fixincludes doesn't claim a dependency on "method-hints.h" */

/* Explicitly export unescaped names, since "fixincludes" won't see these otherwise */
/*!export type_method*/
/*!export type_getset*/
/*!export type_member*/
/*!export type_gc*/
/*!export type_math*/
/*!export type_cmp*/
/*!export type_seq*/
/*!export type_iterator*/
/*!export type_attr*/
/*!export type_with*/
/*!export type_callable*/
/*!export type_buffer*/
/*!export type_operator*/
#endif /* DEE_SOURCE */


#ifdef CONFIG_TRACE_REFCHANGES
struct Dee_refchange {
	char const        *rc_file; /* [1..1][SENTINAL(NULL)] The file in which the change happened. */
	int                rc_line; /* Reference change line.
	                             * When negative: decref(); otherwise: incref() */
};
struct Dee_refchanges {
	struct Dee_refchanges *rc_prev;    /* [0..1] Previous Dee_refchange data block. */
	struct Dee_refchange   rc_chng[7]; /* [*] Set of reference counter changes. */
};
struct Dee_reftracker {
	struct Dee_reftracker **rt_pself;  /* [1..1][0..1][lock(INTERNAL(...))] Self-pointer. */
	struct Dee_reftracker  *rt_next;   /* [0..1][lock(INTERNAL(...))] Pointer to the next tracked object. */
	DeeObject              *rt_obj;    /* [1..1][const] The object being tracked. */
	struct Dee_refchanges  *rt_last;   /* [1..1][lock(ATOMIC)] The current refcnt-change set. */
	struct Dee_refchanges   rt_first;  /* The initial refcnt-change set. */
};
DFUNDEF void DCALL Dee_DumpReferenceLeaks(void);
#endif /* CONFIG_TRACE_REFCHANGES */


/* Check if the given object is being shared.
 * WARNING: The returned value cannot be relied upon for
 *          objects implementing the WEAKREF interface. */
#define DeeObject_IsShared(self) ((self)->ob_refcnt != 1)

/* >> DeeTypeObject *DeeObject_InitInherited(DeeObject *self, / *inherit(always)* / DREF DeeTypeObject *type); */
#ifdef CONFIG_TRACE_REFCHANGES
#define DeeObject_InitInherited(self, type) ((self)->ob_refcnt = 1, (self)->ob_type = (type), (self)->ob_trace = NULL)
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_InitInherited(self, type) ((self)->ob_refcnt = 1, (self)->ob_type = (type))
#endif /* !CONFIG_TRACE_REFCHANGES */

/* >> void *DeeObject_DATA(DeeObject *self);
 * Returns a pointer to the object-specific data-area of "self" */
#ifdef __INTELLISENSE__
#ifdef CONFIG_TRACE_REFCHANGES
#define DeeObject_DATA(self) ((void *)(&(self)->ob_trace + 1))
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_DATA(self) ((void *)(&(self)->ob_type + 1))
#endif /* !CONFIG_TRACE_REFCHANGES */
#else /* __INTELLISENSE__ */
#define DeeObject_DATA(self) ((void *)((__BYTE_TYPE__ *)(self) + Dee_OBJECT_OFFSETOF_DATA))
#endif /* !__INTELLISENSE__ */


/* >> DeeTypeObject *DeeObject_Init(DeeObject *self, DeeTypeObject *type);
 * Initialize the standard objects fields of a freshly allocated object.
 * @param: DeeObject      *self: The object to initialize
 * @param: DeeTypeoObject *type: The type to assign to the object */
#define DeeObject_Init(self, type) \
	(Dee_Incref(type),             \
	 DeeObject_InitInherited(self, (DeeTypeObject *)(type)))


/* Serialization address (points into the abstract serialization buffer) */
#ifndef Dee_seraddr_t_DEFINED
#define Dee_seraddr_t_DEFINED           /*!export-*/
typedef __UINTPTR_TYPE__ Dee_seraddr_t; /*!export-*/ /* Should `#include <deemon/serial.h>' for this one... */
#endif /* !Dee_seraddr_t_DEFINED */

struct Dee_serial;

#if defined(__INTELLISENSE__) && defined(__cplusplus)
extern "C++" {namespace __intern {
#define _Dee_PRIVATE_REQUIRE_VAR_tp_ctor(tp_ctor) ::__intern::__Dee_PRIVATE_REQUIRE_VAR_tp_ctor(tp_ctor)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_ctor(Tobj *(DCALL *)(void)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_ctor(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor) ::__intern::__Dee_PRIVATE_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_copy_ctor(Tobj *(DCALL *)(Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_copy_ctor(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor(tp_any_ctor) ::__intern::__Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor(tp_any_ctor)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor(Tobj *(DCALL *)(size_t, DeeObject *const *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw) ::__intern::__Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor_kw(Tobj *(DCALL *)(size_t, DeeObject *const *, DeeObject *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor_kw(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_VAR_tp_serialize(tp_serialize) ::__intern::__Dee_PRIVATE_REQUIRE_VAR_tp_serialize(tp_serialize)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_serialize(Dee_seraddr_t (DCALL *)(Tobj *__restrict, struct Dee_serial *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_VAR_tp_serialize(decltype(nullptr));

#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(tp_ctor) ::__intern::__Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(tp_ctor)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(int (DCALL *)(Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor) ::__intern::__Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(int (DCALL *)(Tobj *__restrict, Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor) ::__intern::__Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(int (DCALL *)(Tobj *__restrict, size_t, DeeObject *const *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw) ::__intern::__Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(int (DCALL *)(Tobj *__restrict, size_t, DeeObject *const *, DeeObject *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(tp_serialize) ::__intern::__Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(tp_serialize)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(int (DCALL *)(Tobj *__restrict, struct Dee_serial *__restrict, Dee_seraddr_t)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(decltype(nullptr));

#define _Dee_PRIVATE_REQUIRE_tp_alloc(tp_alloc) ::__intern::__Dee_PRIVATE_REQUIRE_tp_alloc(tp_alloc)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_tp_alloc(Tobj *(DCALL *)(void)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_tp_alloc(void *(DCALL *)(void));
Dee_funptr_t __Dee_PRIVATE_REQUIRE_tp_alloc(decltype(nullptr));
#define _Dee_PRIVATE_REQUIRE_tp_free(tp_free) ::__intern::__Dee_PRIVATE_REQUIRE_tp_free(tp_free)
template<class Tobj> Dee_funptr_t __Dee_PRIVATE_REQUIRE_tp_free(void (DCALL *)(Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_PRIVATE_REQUIRE_tp_free(void (DCALL *)(void *__restrict));
Dee_funptr_t __Dee_PRIVATE_REQUIRE_tp_free(decltype(nullptr));
}}
#else /* __INTELLISENSE__ && __cplusplus */
#define _Dee_PRIVATE_REQUIRE_VAR_tp_ctor(tp_ctor)                 (Dee_funptr_t)(tp_ctor)
#define _Dee_PRIVATE_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor)       (Dee_funptr_t)(tp_copy_ctor)
#define _Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor(tp_any_ctor)         (Dee_funptr_t)(tp_any_ctor)
#define _Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw)   (Dee_funptr_t)(tp_any_ctor_kw)
#define _Dee_PRIVATE_REQUIRE_VAR_tp_serialize(tp_serialize)       (Dee_funptr_t)(tp_serialize)
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(tp_ctor)               (Dee_funptr_t)(tp_ctor)
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor)     (Dee_funptr_t)(tp_copy_ctor)
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor)       (Dee_funptr_t)(tp_any_ctor)
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw) (Dee_funptr_t)(tp_any_ctor_kw)
#define _Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(tp_serialize)     (Dee_funptr_t)(tp_serialize)
#define _Dee_PRIVATE_REQUIRE_tp_alloc(tp_alloc)                   (Dee_funptr_t)(tp_alloc)
#define _Dee_PRIVATE_REQUIRE_tp_free(tp_free)                     (Dee_funptr_t)(tp_free)
#endif /* !__INTELLISENSE__ || !__cplusplus */

/************************************************************************/
/* Initializers for TP_FVARIABLE types                                  */
/************************************************************************/
#define Dee_TYPE_CONSTRUCTOR_INIT_VAR(tp_ctor,                   \
                                      tp_copy_ctor,              \
                                      tp_any_ctor,               \
                                      tp_any_ctor_kw,            \
                                      tp_serialize,              \
                                      tp_free)                   \
	{{                                                           \
		_Dee_PRIVATE_REQUIRE_VAR_tp_ctor(tp_ctor),               \
		_Dee_PRIVATE_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor),     \
		_Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor(tp_any_ctor),       \
		_Dee_PRIVATE_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw), \
		_Dee_PRIVATE_REQUIRE_VAR_tp_serialize(tp_serialize),     \
		_Dee_PRIVATE_REQUIRE_tp_free(tp_free),                   \
		{ NULL }                                                 \
	}}


/************************************************************************/
/* Initializers for non-TP_FVARIABLE types                              */
/************************************************************************/

/* With custom allocator */
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor,                   \
                                        tp_copy_ctor,              \
                                        tp_any_ctor,               \
                                        tp_any_ctor_kw,            \
                                        tp_serialize,              \
                                        tp_alloc,                  \
                                        tp_free)                   \
	{{                                                             \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(tp_ctor),               \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor),     \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor),       \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw), \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(tp_serialize),     \
		_Dee_PRIVATE_REQUIRE_tp_free(tp_free),                     \
		{ _Dee_PRIVATE_REQUIRE_tp_alloc(tp_alloc) }                \
	}}

/* Specifies an automatic object allocator. */
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED(max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED(tp_instance_size, \
                                                  tp_ctor,          \
                                                  tp_copy_ctor,     \
                                                  tp_any_ctor,      \
                                                  tp_any_ctor_kw,   \
                                                  tp_serialize)     \
	{{                                                              \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_ctor(tp_ctor),                \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor),      \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor),        \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw),  \
		_Dee_PRIVATE_REQUIRE_ALLOC_tp_serialize(tp_serialize),      \
		(Dee_funptr_t)NULL,                                         \
		{ (Dee_funptr_t)(void *)(uintptr_t)(tp_instance_size) }     \
	}}

#undef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#if (!defined(CONFIG_BUILDING_DEEMON) || defined(__PIE__) ||     \
     defined(__PIC__) || defined(__pie__) || defined(__pic__) || \
     defined(CONFIG_NO_OBJECT_SLABS))
#define CONFIG_FIXED_ALLOCATOR_S_IS_AUTO /*!export*/
#endif /* CONFIG_BUILDING_DEEMON && !__pic__ */

/* Specifies an allocator that may provides optimizations
 * for types with a FIXED size (which most objects have). */
#ifdef GUARD_DEEMON_ALLOC_H
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
/*!export Dee_TYPE_CONSTRUCTOR_INIT_SIZED(include("alloc.h"))*/
/*!export Dee_TYPE_CONSTRUCTOR_INIT_FIXED(include("alloc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)            \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                  \
	                                DeeSlab_GetMalloc(tp_instance_size, (void *(DCALL *)(void))(void *)(uintptr_t)(tp_instance_size)), \
	                                DeeSlab_GetFree(tp_instance_size, (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#ifdef CONFIG_NO_OBJECT_SLABS
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R    Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R /*!export(include("alloc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R /*!export(include("alloc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED      Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED   /*!export(include("alloc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC   Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED   /*!export(include("alloc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED      Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO        /*!export(include("alloc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC   Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO        /*!export(include("alloc.h"))*/
#else /* CONFIG_NO_OBJECT_SLABS */
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)              \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                                                \
	                                DeeSlab_Invoke(&DeeObject_SlabMalloc, max_tp_instance_size, , (void *(DCALL *)(void))(void *)(uintptr_t)(max_tp_instance_size)), \
	                                DeeSlab_Invoke(&DeeObject_SlabFree, min_tp_instance_size, , (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)             \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                                                  \
	                                DeeSlab_Invoke(&DeeGCObject_SlabMalloc, max_tp_instance_size, , (void *(DCALL *)(void))(void *)(uintptr_t)(max_tp_instance_size)), \
	                                DeeSlab_Invoke(&DeeGCObject_SlabFree, min_tp_instance_size, , (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(tp_instance_size, tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R(tp_instance_size, tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#endif /* !CONFIG_NO_OBJECT_SLABS */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

/* Same as `Dee_TYPE_CONSTRUCTOR_INIT_FIXED()', but don't link against
 * dedicated allocator functions when doing so would require the creation
 * of relocations that might cause loading times to become larger. */
#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S    Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO /*!export(include("alloc.h"))*/
#ifndef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO /*!export(include("alloc.h"))*/
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S    Dee_TYPE_CONSTRUCTOR_INIT_FIXED    /*!export(include("alloc.h"))*/
#ifndef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC /*!export(include("alloc.h"))*/
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#endif /* GUARD_DEEMON_ALLOC_H */

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#ifdef GUARD_DEEMON_GC_H
/*!export Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(include("gc.h"))*/
/*!export Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(include("gc.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)           \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                    \
	                                DeeGCSlab_GetMalloc(tp_instance_size, (void *(DCALL *)(void))(void *)(uintptr_t)(tp_instance_size)), \
	                                DeeGCSlab_GetFree(tp_instance_size, (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)

/* Same as `Dee_TYPE_CONSTRUCTOR_INIT_FIXED()', but don't link against
 * dedicated allocator functions when doing so would require the creation
 * of relocations that might cause loading times to become larger. */
#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO /*!export(include("alloc.h"))*/
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC /*!export(include("alloc.h"))*/
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#endif /* GUARD_DEEMON_GC_H */
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */



typedef NONNULL_T((1)) void (DCALL *Dee_tp_destroy_t)(DeeObject *__restrict self);

#undef tp_alloc
struct Dee_type_constructor {
	/* Instance constructors/destructors. */
	union {
		struct {
			Dee_funptr_t _tp_init0_; /* tp_ctor */
			Dee_funptr_t _tp_init1_; /* tp_copy_ctor */
			Dee_funptr_t _tp_init3_; /* tp_any_ctor */
			Dee_funptr_t _tp_init4_; /* tp_any_ctor_kw */
			Dee_funptr_t _tp_init5_; /* tp_serialize */
			/* Initializer for a custom type allocator. */
			Dee_funptr_t _tp_init6_; /* tp_free */
			struct { Dee_funptr_t _tp_init7_; } _tp_init7_;
		} _tp_init_;

		struct {
			WUNUSED_T NONNULL_T((1))                  int (DCALL *tp_ctor)(DeeObject *__restrict self);
			WUNUSED_T NONNULL_T((1, 2))               int (DCALL *tp_copy_ctor)(DeeObject *__restrict self, DeeObject *__restrict other);
			WUNUSED_T NONNULL_T((1)) ATTR_INS_T(3, 2) int (DCALL *tp_any_ctor)(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

			/* WARNING: `tp_any_ctor_kw' may be invoked with `argc == 0 && kw == NULL',
			 *           even when `tp_ctor' has been defined as non-NULL! */
			WUNUSED_T NONNULL_T((1)) ATTR_INS_T(3, 2)
			int (DCALL *tp_any_ctor_kw)(DeeObject *__restrict self, size_t argc,
			                            DeeObject *const *argv, DeeObject *kw);

			/* [0..1] Serialize the binary data of `self' into `writer'.
			 * The caller has already pre-allocated the necessary amount
			 * of storage needed as per `tp_instance_size' (or `tp_alloc')
			 * at `addr', leaving it to this operator to stream the data
			 * of `self' into `writer'
			 *
			 * Standard fields like `ob_refcnt' and `ob_type' will have
			 * already been initialized by the time this operator is called.
			 *
			 * @return: 0 : Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1, 2)) int
			(DCALL *tp_serialize)(DeeObject *__restrict self,
			                      struct Dee_serial *__restrict writer,
			                      Dee_seraddr_t addr);

			/* WARNING: A situation can arise in which the `tp_free'
			 *          operator of a base-class is used instead of
			 *          the one accompanying `tp_alloc()'.
			 *       >> Because of this, `tp_alloc' and `tp_free' should only
			 *          be used for accessing a cache of pre-allocated objects, that
			 *          were created using regular heap allocations (`DeeObject_Malloc'). */
			NONNULL_T((1)) void (DCALL *tp_free)(void *__restrict ob);
			union {
				size_t tp_instance_size;       /*       [valid_if(tp_free == NULL)] */
				void *(DCALL *tp_alloc)(void); /* [1..1][valid_if(tp_free != NULL)] */
			}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
			_dee_aunion
#define tp_instance_size _dee_aunion.tp_instance_size /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
			;
		} tp_alloc; /* [valid_if(!TP_FVARIABLE)] */

		struct {
			/* NOTES:
			 * - Var-constructors are allowed to return instances of types other than
			 *   `tp'. However, this is a privilege that is not exposed to user-code.
			 * - Additionally, any type making use of this must openly document this
			 *   in order to prevent confusion.
			 * - It should also be noted that the deemon core does not make use of
			 *   this functionality anywhere, and as of right now, the only type that
			 *   does make use of it is the copy constructor of `LValue' objects
			 *   found in `ctypes'.
			 * - Rather than returning another instance of the l-value type, it
			 *   returns a regular structured object containing a copy of the data
			 *   that was pointed-to by the l-value.
			 */
			WUNUSED_T                  DREF DeeObject *(DCALL *tp_ctor)(void);
			WUNUSED_T NONNULL_T((1))   DREF DeeObject *(DCALL *tp_copy_ctor)(DeeObject *__restrict other);
			WUNUSED_T ATTR_INS_T(2, 1) DREF DeeObject *(DCALL *tp_any_ctor)(size_t argc, DeeObject *const *argv);
			/* WARNING: `tp_any_ctor_kw' may be invoked with `argc == 0 && kw == NULL',
			 *          even when `tp_ctor' or `tp_any_ctor' has been defined as non-NULL! */
			WUNUSED_T ATTR_INS_T(2, 1) DREF DeeObject *(DCALL *tp_any_ctor_kw)(size_t argc, DeeObject *const *argv, DeeObject *kw);

			/* [0..1] Serialize the binary data of `self' into `writer'.
			 * @return: Dee_SERADDR_INVALID : Error
			 * @return: * : Address where data of `self' was written to. */
			WUNUSED_T NONNULL_T((1, 2)) Dee_seraddr_t
			(DCALL *tp_serialize)(DeeObject *__restrict self,
			                      struct Dee_serial *__restrict writer);

			NONNULL_T((1)) void (DCALL *tp_free)(void *__restrict ob);
			struct { Dee_funptr_t tp_pad; } tp_pad; /* ... */
		} tp_var; /* [valid_if(TP_FVARIABLE)] */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define tp_alloc _dee_aunion.tp_alloc /*!export-*/
#define tp_var   _dee_aunion.tp_var   /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;

	/* [0..1] Optional destructor callback. */
	NONNULL_T((1)) void (DCALL *tp_dtor)(DeeObject *__restrict self);

	/* NOTE: `tp_move_assign' is favored in code such as this:
	 * >> local my_list = [];
	 * >> my_list := copy     get_other_list(); // Will try to move-assign the copy.
	 * >> my_list := deepcopy get_other_list(); // Will try to move-assign the deep copy.
	 * @return: == 0: Success
	 * @return: != 0: Error was thrown */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_assign)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_move_assign)(DeeObject *self, DeeObject *other);

	/* Callback for `DeeObject_Destroy()' (usually auto-assigned by
	 * `DeeType_RequireDestroy()', but can be overwritten with a
	 * custom implementation). */
	Dee_tp_destroy_t tp_destroy;

#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
	/* [0..1] Optional destructor callback that is called before "tp_dtor".
	 * Also, unlike "tp_dtor", this callback is allowed to "revive" the
	 * object by returning with its "ob_refcnt > 0". When this happens,
	 * the object destruction is aborted and said "new" reference will
	 * be kept around.
	 *
	 * Note that for GC objects, this operator is only invoked *once*,
	 * even if it ended up reviving the object in question! */
	NONNULL_T((1, 2))
	void (DCALL *tp_finalize)(DeeTypeObject *__restrict tp_self,
	                          DeeObject *__restrict self);
#endif /* CONFIG_EXPERIMENTAL_REWORKED_GC */
};

struct Dee_type_cast {
	/* Instance casting operators. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *tp_str)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *tp_repr)(DeeObject *__restrict self);
	/* @return:  > 0: Object is true
	 * @return: == 0: Object is false
	 * @return: <  0: Error was thrown */
	WUNUSED_T NONNULL_T((1)) int (DCALL *tp_bool)(DeeObject *__restrict self);
	/* Optional fast-pass operator for `DeeObject_Print(tp_str(self), printer, arg)' */
	WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *tp_print)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
	/* Optional fast-pass operator for `DeeObject_Print(tp_repr(self), printer, arg)' */
	WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *tp_printrepr)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
};

struct Dee_type_gc {
	/* Clear all possible references with `NULL' or some
	 * statically allocated stub-object (e.g.: `Dee_None') */
	NONNULL_T((1)) void (DCALL *tp_clear)(DeeObject *__restrict self);

#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
	/* TODO: Once `CONFIG_EXPERIMENTAL_REWORKED_GC' becomes mandatory, consider inlining `tp_clear'
	 *       into `DeeTypeObject' by getting rid of `tp_gc'. `tp_cc' could then become a method hint */
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
	/* Same as `tp_clear', but only clear reachable objects with a `tp_gcprio'
	 * priority that is `>= prio'. (non-gc types have a priority of `Dee_GC_PRIORITY_LATE')
	 * @assume(prio != 0);
	 * When `prio' would be ZERO(0), `tp_clear' is called instead.
	 * Using this, the order in which the GC destroys objects can be specified,
	 * where higher-priority objects are unlinked before lower-priority ones.
	 * The usual implementation acts as a weaker version of `tp_clear' that
	 * should drop references to other objects individually, rather than as
	 * a bulk, as well as do so with regards of GC priority, when comparing
	 * `gc_priority' against the return value of `DeeObject_GCPriority()' */
	NONNULL_T((1)) void (DCALL *tp_pclear)(DeeObject *__restrict self, unsigned int gc_priority);

	/* The GC destruction priority of this object (greater
	 * values are (attempted to be) destroyed before lower ones).
	 * For a list of priority values of builtin types, see the macros below.
	 * However, this priority ordering is a mere hint to the garbage collector,
	 * which is always allowed to detract from this priorative ordering when
	 * it would otherwise be impossible to resolve reference loops.
	 * Also note that for priorative GC to function properly, any GC object
	 * apart of a priorative loop should implement `tp_pclear()' in addition
	 * to `tp_clear'.
	 * This however is _never_ a requirement for properly functioning code.
	 * The most basic usage case for why this is a fairly good idea would be as simple as this:
	 * >> class MyClass {
	 * >>     private m_value = 42;
	 * >>     this() {
	 * >>         print "MyClass.this()", m_value;
	 * >>     }
	 * >>     ~this() {
	 * >>         print "MyClass.~this()", m_value;
	 * >>     }
	 * >> }
	 * >> 
	 * >> global x = MyClass();
	 * The global variable `x' is never unbound, meaning that
	 * when deemon is shutting down, the following GC-cycle
	 * still exists and must be cleaned up before terminating:
	 *
	 *  ***********************************************************************
	 *  *                                                                     *
	 *  *    x ─> MyClass ─> function(MyClass.operators) ─> code ─> module    *
	 *  *    ^      ^  ^                                     │       │  │     *
	 *  *    │      │  │                                     │       │  │     *
	 *  *    │      │  └─────────────────────────────────────┘       │  │     *
	 *  *    │      │                                                │  │     *
	 *  *    │      └────────────────────────────────────────────────┘  │     *
	 *  *    │                                                          │     *
	 *  *    └──────────────────────────────────────────────────────────┘     *
	 *  *                                                                     *
	 *  ***********************************************************************
	 *
	 *  * NOTE: The '*' around the graphic above are required to work around a
	 *          VS bug that causes the IDE's internal parser to enter an infinite
	 *          loop. I don't know exactly what's causing it, but you can test
	 *          it for yourself by creating a file with the following contents,
	 *          then closing+reopening VS (2017) and opening that file:
	 *          >>  #ifndef FOO                                              <<
	 *          >>  #define FOO                                              <<
	 *          >>  //│                                                      <<
	 *          >>  //│                                                      <<
	 *          >>  //│                                                      <<
	 *          >>  //│                                                      <<
	 *          >>  #endif                                                   <<
	 *          I have no idea what exactly is going on here, but the bug only
	 *          seems to appear when at least 4 consecutive comment lines end
	 *          with the '│' unicode character.
	 *
	 * This might seem simple at first, but the order with which the GC
	 * chooses to deal with this cycle determines if the destructor can
	 * even function properly:
	 *   - As you can see, it uses a member variable `m_value', meaning that
	 *     if we were to clear the instance `x' first, it would fail with an
	 *     attribute error, so `x' is a bad idea
	 *   - If we were to start by clearing `MyClass's operators, it wouldn't
	 *     work at all because there'd no longer be a destructor to call.
	 *   - The link from `Code' to `Module' is kind-of special, in that it
	 *     can't actually be broken (plus: preventing the destructor from
	 *     accessing global variables would also break the member lookup,
	 *     as this requires access to the class type `MyClass', which (in this case)
	 *     is assembled to use a global; aka. module variable lookup)
	 *   - And we can't clear the module, because that would include clearing
	 *     not only `x', but also `MyClass', once again breaking the destructor.
	 * So how _do_ we solve this?
	 *   - We clear the module. (Which has the greatest priority `Dee_GC_PRIORITY_MODULE')
	 *   - But you just said that we can't do that because it would unlink
	 *     the `MyClass' type and break member lookup.
	 *   - Yes, it would. But that's where `tp_pclear()' comes into play,
	 *     which we can then use to only clear instances of `Dee_GC_PRIORITY_MODULE'
	 *     (which might cause problems with dynamically imported modules, but we
	 *     won't bother with that, considering how that would cause a priority
	 *     logic loop when `module >before> instance >before> module').
	 *     And after we've done that, we'll move on to clear the next lower
	 *     priority level of all objects in the loop (which is `Dee_GC_PRIORITY_INSTANCE')
	 *   - Now, all we've done thus far is to clear the module's link to `x',
	 *     allowing us to break the cycle surrounding the outer-most ring
	 *     plainly visible by the long arrow from `module' to `x' in the diagram above,
	 *     while the module on the right still continues to know of `MyClass', meaning
	 *     that the destructor can operate normally, just as intended.
	 *   - Now consider what would happen if the instance `x' contained a
	 *     self-reference: We'd be unable to destroy it by just deleting
	 *     the global reference. However eventually we'd move on to delete
	 *     that self-reference, before also deleting `MyClass', gradually
	 *     decreasing the used GC-priority while still doing our best to
	 *     fulfill the desired destruction order.
	 *     Eventually, the destructor will throw an error that will be
	 *     discarded, before the class instance is finally deleted.
	 *     And even if this was not the case, `Dee_Shutdown()' will
	 *     eventually disallow further execution of user-code, meaning that
	 *     anything the user-destructor might do to revive itself will no
	 *     longer be done at some point.
	 */
	unsigned int tp_gcprio;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */

	/* Clear unused heap caches of the object. Called when deemon is
	 * running low on memory (iow: from "Dee_CollectMemory") */
	NONNULL_T((1)) bool (DCALL *tp_cc)(DeeObject *__restrict self);
};


#ifndef CONFIG_EXPERIMENTAL_REWORKED_GC
/* GC destruction priority levels of builtin types. */
#define Dee_GC_PRIORITY_LATE     0x0000 /* (Preferably) destroyed last. */
#define Dee_GC_PRIORITY_CLASS    0xfd00 /* User-classes. */
#define Dee_GC_PRIORITY_INSTANCE 0xfe00 /* Instances of user-classes. */
#define Dee_GC_PRIORITY_MODULE   0xff00 /* Module objects. */
#define Dee_GC_PRIORITY_EARLY    0xffff /* (Preferably) destroyed before anything else. */
#endif /* CONFIG_EXPERIMENTAL_REWORKED_GC */


/* Return values for `tp_int32' and `tp_int64' */
#define Dee_INT_SIGNED   0    /* The saved integer value is signed. */
#define Dee_INT_UNSIGNED 1    /* The saved integer value is unsigned. */
#define Dee_INT_ERROR    (-1) /* An error was thrown */
#define Dee_INT_ISERR(x) ((x) < 0)

#ifdef DEE_SOURCE
#define INT_SIGNED   Dee_INT_SIGNED   /* The saved integer value is signed. */
#define INT_UNSIGNED Dee_INT_UNSIGNED /* The saved integer value is unsigned. */
#define INT_ERROR    Dee_INT_ERROR    /* An error was thrown */
#endif /* DEE_SOURCE */

struct Dee_type_math {
	/* Math related operators. */
	/* @return: Dee_INT_SIGNED:   The value stored in `*result' is signed. 
	 * @return: Dee_INT_UNSIGNED: The value stored in `*result' is unsigned.
	 * @return: Dee_INT_ERROR:    An error occurred. */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_int32)(DeeObject *__restrict self, int32_t *__restrict result);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_int64)(DeeObject *__restrict self, int64_t *__restrict result);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_double)(DeeObject *__restrict self, double *__restrict result);

	/* Cast to `int' (Must return an `DeeInt_Type' object) */
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *tp_int)(DeeObject *__restrict self);

	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *tp_inv)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *tp_pos)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *tp_neg)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_add)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_sub)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_mul)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_div)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_mod)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_shl)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_shr)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_and)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_or)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_xor)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_pow)(DeeObject *self, DeeObject *some_object);

	/* TODO: Change the prototypes of all the following to:
	 * >> DREF DeeObject *(DCALL *tp_inc)(DREF DeeObject *__restrict self);
	 *
	 * Also change the prototypes of:
	 * >> DREF DeeObject *DeeObject_Inc(DREF DeeObject *__restrict self); */

	/* Inplace operators (Optional; Implemented using functions above when not available) */
	WUNUSED_T NONNULL_T((1)) int (DCALL *tp_inc)(DREF DeeObject **__restrict p_self);
	WUNUSED_T NONNULL_T((1)) int (DCALL *tp_dec)(DREF DeeObject **__restrict p_self);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_add)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_sub)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_mul)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_div)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_mod)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_shl)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_shr)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_and)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_or)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_xor)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_inplace_pow)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
};

struct Dee_type_cmp {
	/* Compare operators. */
	WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *tp_hash)(DeeObject *__restrict self);

	/* Same as "tp_compare", but only needs to support equal/not-equal compare:
	 * @return: Dee_COMPARE_ERR: An error occurred.
	 * @return: -1: `lhs != rhs'
	 * @return: 0:  `lhs == rhs'
	 * @return: 1:  `lhs != rhs' */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_compare_eq)(DeeObject *self, DeeObject *some_object);

	/* Rich-compare operator that can be defined instead
	 * of `tp_eq', `tp_ne', `tp_lo', `tp_le', `tp_gr', `tp_ge'
	 * @return: Dee_COMPARE_ERR: An error occurred.
	 * @return: -1: `lhs < rhs'
	 * @return: 0:  `lhs == rhs'
	 * @return: 1:  `lhs > rhs' */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_compare)(DeeObject *self, DeeObject *some_object);

	/* Same as "tp_compare_eq", but shouldn't[1] throw `NotImplemented', `TypeError' or `ValueError'.
	 * Instead of throwing these errors, this implementation should handle these errors by returning
	 * either `-1' or `1' to indicate non-equality.
	 *
	 * [1] With "shouldn't" I mean *REALLY* shouldn't. As in: unless you *really* want it to throw
	 *     one of those errors, you should either use API functions that never throw these errors,
	 *     or add `DeeError_Catch()' calls to your function to catch those errors by returning either
	 *     `-1' or `1' instead.
	 *
	 * !!! THIS OPERATOR CANNOT BE USED TO SUBSTITUTE "tp_compare_eq" !!!
	 * -> Defining this operator but not defining "tp_compare_eq" is !NOT VALID!
	 *    However, "tp_trycompare_eq" can ITSELF be substituted by "tp_compare_eq"
	 *
	 * @return: Dee_COMPARE_ERR: An error occurred.
	 * @return: -1: `lhs != rhs'
	 * @return: 0:  `lhs == rhs'
	 * @return: 1:  `lhs != rhs' */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_trycompare_eq)(DeeObject *self, DeeObject *some_object);

	/* Individual compare operators. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_eq)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_ne)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_lo)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_le)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_gr)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_ge)(DeeObject *self, DeeObject *some_object);
};

struct Dee_type_seq {
	/* Sequence operators. */
	WUNUSED_T NONNULL_T((1))          DREF DeeObject *(DCALL *tp_iter)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1))          DREF DeeObject *(DCALL *tp_sizeob)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1, 2))       DREF DeeObject *(DCALL *tp_contains)(DeeObject *self, DeeObject *some_object);
	WUNUSED_T NONNULL_T((1, 2))       DREF DeeObject *(DCALL *tp_getitem)(DeeObject *self, DeeObject *index);
	WUNUSED_T NONNULL_T((1, 2))       int             (DCALL *tp_delitem)(DeeObject *self, DeeObject *index);
	WUNUSED_T NONNULL_T((1, 2, 3))    int             (DCALL *tp_setitem)(DeeObject *self, DeeObject *index, DeeObject *value);
	WUNUSED_T NONNULL_T((1, 2, 3))    DREF DeeObject *(DCALL *tp_getrange)(DeeObject *self, DeeObject *start, DeeObject *end);
	WUNUSED_T NONNULL_T((1, 2, 3))    int             (DCALL *tp_delrange)(DeeObject *self, DeeObject *start, DeeObject *end);
	WUNUSED_T NONNULL_T((1, 2, 3, 4)) int             (DCALL *tp_setrange)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);

	/* Optional sequence-extensions for providing optimized (but
	 * less generic) variants of the sequence operators above. */

	/* Alternate forms for `tp_iter'...
	 *
	 * Instead of defining `tp_iter', you can just define one of these and have the runtime
	 * use these for enumerating the object. Note however that this is less efficient when
	 * enumeration still requires an iterator, and that for this purpose, the type should
	 * still provide a proper `tp_iter' callback (or if it is derived from DeeSeq_Type, it
	 * can also just provide `tp_size' and `tp_getitem_index'). */
	WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *tp_foreach_pair)(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);

	/* Optional function to check if a specific item index/key is bound. (inherited alongside `tp_getitem')
	 * Check if a given item is bound (`self[index] is bound' / `deemon.bounditem(self, index)')
	 * @return: Dee_BOUND_YES:     Item is bound.
	 * @return: Dee_BOUND_NO:      Item isn't bound. (in `tp_getitem': `UnboundItem')
	 * @return: Dee_BOUND_ERR:     An error occurred.
	 * @return: Dee_BOUND_MISSING: Item doesn't exist (in `tp_getitem': `KeyError'). */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_bounditem)(DeeObject *self, DeeObject *index);

	/* Check if a given item exists (`deemon.hasitem(self, index)') (inherited alongside `tp_getitem')
	 * @return: >  0: Does exists.   (in `tp_getitem': `UnboundItem' or <no error>)
	 * @return: == 0: Doesn't exist. (in `tp_getitem': `KeyError')
	 * @return: <  0: Error. */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_hasitem)(DeeObject *self, DeeObject *index);

	/* Aliases for `tp_sizeob' */
	WUNUSED_T NONNULL_T((1)) size_t (DCALL *tp_size)(DeeObject *__restrict self);

	/* Same as `tp_size', but should execute in O(1) time and never throw exceptions.
	 * NOTE: This operator can NOT be used to substitute `tp_size'!
	 * @return: * : A snapshot of the object's current size.
	 * @return: (size_t)-1: Size cannot be determined fast. */
	WUNUSED_T NONNULL_T((1)) size_t (DCALL *tp_size_fast)(DeeObject *__restrict self);

	/* Aliases for `tp_getitem' */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *tp_getitem_index)(DeeObject *self, size_t index);

	/* Same as `tp_getitem_index', but never throws an exception:
	 * NOTE: This operator can NOT be used to substitute `tp_getitem_index'!
	 * @param: index: Index of item to access. Guarantied to be `<' some preceding
	 *                call to `tp_size_fast', `tp_size', or `tp_sizeob' (from the
	 *                same type; i.e. the size operator will not be from a sub-class).
	 * @return: * :   A reference to the index.
	 * @return: NULL: The sequence is resizable and `index >= CURRENT_SIZE'
	 * @return: NULL: Sequence indices can be unbound, and nothing is bound to `index' right now. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);

	/* Aliases of the non-*_index variants above. Behavior of these matches behavior above. */
	WUNUSED_T NONNULL_T((1))    int (DCALL *tp_delitem_index)(DeeObject *self, size_t index);
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *tp_setitem_index)(DeeObject *self, size_t index, DeeObject *value);
	WUNUSED_T NONNULL_T((1))    int (DCALL *tp_bounditem_index)(DeeObject *self, size_t index);
	WUNUSED_T NONNULL_T((1))    int (DCALL *tp_hasitem_index)(DeeObject *self, size_t index);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *tp_getrange_index)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
	WUNUSED_T NONNULL_T((1))    int             (DCALL *tp_delrange_index)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
	WUNUSED_T NONNULL_T((1, 4)) int             (DCALL *tp_setrange_index)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
	WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *tp_getrange_index_n)(DeeObject *self, Dee_ssize_t start);
	WUNUSED_T NONNULL_T((1))    int             (DCALL *tp_delrange_index_n)(DeeObject *self, Dee_ssize_t start);
	WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *tp_setrange_index_n)(DeeObject *self, Dee_ssize_t start, DeeObject *values);

	/* Operators meant to speed up map operators. */

	/* Same as `tp_getitem', but returns `ITER_DONE' instead of throwing
	 * `KeyError' (or `UnboundItem', which is a given since that one's a
	 * sub-class of `KeyError') */
	WUNUSED_T NONNULL_T((1, 2))    DREF DeeObject *(DCALL *tp_trygetitem)(DeeObject *self, DeeObject *index);
	WUNUSED_T NONNULL_T((1))       DREF DeeObject *(DCALL *tp_trygetitem_index)(DeeObject *self, size_t index);
	WUNUSED_T NONNULL_T((1, 2))    DREF DeeObject *(DCALL *tp_trygetitem_string_hash)(DeeObject *self, char const *key, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    DREF DeeObject *(DCALL *tp_getitem_string_hash)(DeeObject *self, char const *key, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_delitem_string_hash)(DeeObject *self, char const *key, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2, 4)) int             (DCALL *tp_setitem_string_hash)(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_bounditem_string_hash)(DeeObject *self, char const *key, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_hasitem_string_hash)(DeeObject *self, char const *key, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    DREF DeeObject *(DCALL *tp_trygetitem_string_len_hash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    DREF DeeObject *(DCALL *tp_getitem_string_len_hash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_delitem_string_len_hash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2, 5)) int             (DCALL *tp_setitem_string_len_hash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_bounditem_string_len_hash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_hasitem_string_len_hash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

	/* [0..1] Optional helper to help implement `DeeSeq_AsHeapVector()' & friends.
	 * NOTES:
	 * - This operator is NOT used to implement stuff above, and will NOT
	 *   be substituted using other operators. This operator gets inherited
	 *   alongside "tp_iter", though its presence does not qualify for that
	 *   operator being present!
	 * - This operator must NOT produce NULL-elements in "dst". The produced
	 *   vector of items must be equivalent to what would be produced by a
	 *   call to "tp_foreach" (which in turn must be equivalent to "tp_iter").
	 * - This operator must NOT invoke user-code (except as a result of OOM
	 *   handling). - Repeated invocations of this must not have any side-
	 *   effects!
	 *
	 * @return: <= dst_length: Success: the first "return" elements of "dst" were filled with the items of this sequence.
	 * @return: > dst_length:  The given "dst_length" is too short. In this case, "dst" may have been modified,
	 *                         but will not contain any object references. You must resize it until it is able
	 *                         to hold at least "return" elements, and call this operator again.
	 * @return: (size_t)-1:    Error. */
	WUNUSED_T NONNULL_T((1)) size_t (DCALL *tp_asvector)(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);

	/* [0..1] Same as `tp_asvector', but not allowed to throw exceptions or
	 *        ever invoke user-code in any form, or for any reason, or acquire
	 *        arbitrary object locks (other than immediately owned by `self').
	 * This means:
	 * - The caller is allowed to invoke this function while holding some kind
	 *   of lock themselves (so-long as they the lock they're holding isn't
	 *   owned by the given object "self")
	 * - The caller can assume this function to be NOBLOCK, even while holding
	 *   locks themselves (so-long as "self" can't access already-held locks)
	 * Usage:
	 * - This operator is used by `deemon.List' in order to in-place insert
	 *   caller-given sequences into the list's internal vector buffer, without
	 *   the need to transfer objects one-at-a-time, or via a temporary vector.
	 *   Note that in this case, this operator is invoked WHILE the caller is
	 *   HOLDING A LOCK to the list being modified!
	 * NOTE: When defining this operator, you can usually assign the same function
	 *       pointer to `tp_asvector' also. Please do so explicitly. Otherwise,
	 *       the runtime may not be able to notice its availability.
	 * @return: <= dst_length: Success: the first "return" elements of "dst" were filled with the items of this sequence.
	 * @return: > dst_length:  The given "dst_length" is too short. In this case, "dst" may have been modified,
	 *                         but will not contain any object references. You must resize it until it is able
	 *                         to hold at least "return" elements, and call this operator again. */
	WUNUSED_T NONNULL_T((1)) size_t (DCALL *tp_asvector_nothrow)(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);

	/* All of the following are *always* and *unconditionally* implemented
	 * when the associated type has the "tp_features & TF_KW" flag set,
	 * with the exception of `DeeKwds_Type', which has that flag, but does
	 * not implement these operators.
	 *
	 * NOTE: Even when these operators are defined, the operator above will
	 *       NOT auto-substitute themselves with these (even though that
	 *       would be possible), reason being that the amount of runtime
	 *       overhead would be too great for those few types that define
	 *       these operators. */
	WUNUSED_T NONNULL_T((1, 2)) DeeObject *(DCALL *tp_trygetitemnr)(DeeObject *__restrict self, /*string*/ DeeObject *__restrict name);
	WUNUSED_T NONNULL_T((1, 2)) DeeObject *(DCALL *tp_trygetitemnr_string_hash)(DeeObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) DeeObject *(DCALL *tp_trygetitemnr_string_len_hash)(DeeObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
};

#if 0
typedef struct {
	OBJECT_HEAD
} MyObject;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_iter(MyObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_sizeob(MyObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_contains(MyObject *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_getitem(MyObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_delitem(MyObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
myob_setitem(MyObject *self, DeeObject *index, DeeObject *value) {
	(void)self;
	(void)index;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
myob_getrange(MyObject *self, DeeObject *start, DeeObject *end) {
	(void)self;
	(void)start;
	(void)end;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
myob_delrange(MyObject *self, DeeObject *start, DeeObject *end) {
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
myob_setrange(MyObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	(void)self;
	(void)start;
	(void)end;
	(void)values;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
myob_foreach(MyObject *__restrict self, Dee_foreach_t proc, void *arg) {
	(void)self;
	(void)proc;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
myob_foreach_pair(MyObject *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	(void)self;
	(void)proc;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_bounditem(MyObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_hasitem(MyObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
myob_size(MyObject *__restrict self) {
	(void)self;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
myob_size_fast(MyObject *__restrict self) {
	(void)self;
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_getitem_index(MyObject *self, size_t index) {
	(void)self;
	(void)index;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_getitem_index_fast(MyObject *self, size_t index) {
	(void)self;
	(void)index;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
myob_delitem_index(MyObject *self, size_t index) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
myob_setitem_index(MyObject *self, size_t index, DeeObject *value) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
myob_bounditem_index(MyObject *self, size_t index) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
myob_hasitem_index(MyObject *self, size_t index) {
	(void)self;
	(void)index;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_getrange_index(MyObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	(void)self;
	(void)start;
	(void)end;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
myob_delrange_index(MyObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
myob_setrange_index(MyObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_getrange_index_n(MyObject *self, Dee_ssize_t start) {
	(void)self;
	(void)start;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
myob_delrange_index_n(MyObject *self, Dee_ssize_t start) {
	(void)self;
	(void)start;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
myob_setrange_index_n(MyObject *self, Dee_ssize_t start, DeeObject *values) {
	(void)self;
	(void)start;
	(void)values;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_trygetitem(MyObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_trygetitem_index(MyObject *self, size_t index) {
	(void)self;
	(void)index;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_trygetitem_string_hash(MyObject *self, char const *key, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)hash;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_getitem_string_hash(MyObject *self, char const *key, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)hash;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_delitem_string_hash(MyObject *self, char const *key, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
myob_setitem_string_hash(MyObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	(void)self;
	(void)key;
	(void)hash;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_bounditem_string_hash(MyObject *self, char const *key, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_hasitem_string_hash(MyObject *self, char const *key, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_trygetitem_string_len_hash(MyObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
myob_getitem_string_len_hash(MyObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_delitem_string_len_hash(MyObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
myob_setitem_string_len_hash(MyObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_bounditem_string_len_hash(MyObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_hasitem_string_len_hash(MyObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
myob_asvector(MyObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	(void)self;
	(void)dst;
	(void)dst_length;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
myob_asvector_nothrow(MyObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	(void)self;
	(void)dst;
	(void)dst_length;
	return 0;
}

PRIVATE struct type_seq myob_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&myob_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&myob_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&myob_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&myob_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&myob_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&myob_setitem,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&myob_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&myob_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&myob_setrange,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&myob_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&myob_foreach_pair,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&myob_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&myob_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&myob_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&myob_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&myob_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&myob_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&myob_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&myob_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&myob_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&myob_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&myob_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&myob_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&myob_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&myob_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&myob_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&myob_setrange_index_n,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&myob_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&myob_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&myob_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&myob_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&myob_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&myob_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&myob_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&myob_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&myob_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&myob_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&myob_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&myob_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&myob_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&myob_hasitem_string_len_hash,
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&myob_asvector,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&myob_asvector_nothrow,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))NULL,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))NULL,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))NULL,
};
#endif

struct Dee_type_iterator {
	/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2)'
	 * @return: 0 : Success
	 * @return: 1 : Iterator has been exhausted
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_nextpair)(DeeObject *__restrict self, /*out*/ DREF DeeObject *key_and_value[2]);

	/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2).first[key]/last[value]'
	 * In the case of mapping iterators, these can be used to iterate only the
	 * key/value part of the map, without needing to construct a temporary tuple
	 * holding both values (as needs to be done by `tp_iter_next'). */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *tp_nextkey)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *tp_nextvalue)(DeeObject *__restrict self);
};

#if 0
typedef struct {
	OBJECT_HEAD
} MyObject;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_iternext(MyObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myob_nextpair(MyObject *__restrict self, /*out*/ DREF DeeObject *key_and_value[2]) {
	(void)self;
	(void)key_and_value;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_nextkey(MyObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
myob_nextvalue(MyObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE struct type_iterator myob_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&myob_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&myob_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&myob_nextvalue,
};
#endif

struct Dee_attrinfo;
struct Dee_attrspec;
struct Dee_attrdesc;
struct Dee_attriter;
struct Dee_attrhint;

struct Dee_type_attr {
	/* Basic attribute operators. */
	WUNUSED_T NONNULL_T((1, 2))    DREF DeeObject *(DCALL *tp_getattr)(DeeObject *self, /*String*/ DeeObject *attr);
	WUNUSED_T NONNULL_T((1, 2))    int             (DCALL *tp_delattr)(DeeObject *self, /*String*/ DeeObject *attr);
	WUNUSED_T NONNULL_T((1, 2, 3)) int             (DCALL *tp_setattr)(DeeObject *self, /*String*/ DeeObject *attr, DeeObject *value);
	/* Initialize an iterator for enumerating attributes recognized by `tp_getattr'
	 * @param: hint: Hint specifying which attributes to enumerate (may be ignored by constructed iterator,
	 *               meaning you have to do your own additional filtering if you want to be sure that only
	 *               attributes matching your filter get enumerated)
	 * @return: <= bufsize: Success; the given `iterbuf' was initialized and you can start enumeration
	 * @return: > bufsize:  Failure: need a larger buffer size (specifically: one of at least "return" bytes)
	 * @return: (size_t)-1: An error was thrown. */
	WUNUSED_T NONNULL_T((1, 2, 5))
	size_t (DCALL *tp_iterattr)(DeeTypeObject *tp_self, DeeObject *self,
	                            struct Dee_attriter *iterbuf, size_t bufsize,
	                            struct Dee_attrhint const *__restrict hint);

	/* Everything below is just an optional hook for the purpose of optimization.
	 * If implemented, it *MUST* behave identical to the above operators.
	 *
	 * NOTE: Deemon will *NOT* substitute the above functions with those below!!!
	 *       (As a matter of fact: it doesn't use default__* operators for any
	 *       of this stuff since doing so would be slower due to the fact that
	 *       it would add a whole ton of otherwise unnecessary DeeObject_T-checks)
	 *       This means that if you want to implement stuff below, you *MUST* also
	 *       implement the operators above like follows:
	 *   - tp_findattr:                      Must also implement `tp_iterattr'
	 *   - tp_hasattr:                       Must also implement `tp_getattr'
	 *   - tp_boundattr:                     Must also implement `tp_getattr'
	 *   - tp_callattr:                      Must also implement `tp_getattr'
	 *   - tp_callattr_kw:                   Must also implement `tp_getattr'
	 *   - tp_vcallattrf:                    Must also implement `tp_getattr'
	 *   - tp_getattr_string_hash:           Must also implement `tp_getattr'
	 *   - tp_delattr_string_hash:           Must also implement `tp_delattr'
	 *   - tp_setattr_string_hash:           Must also implement `tp_setattr'
	 *   - tp_hasattr_string_hash:           Must also implement `tp_getattr'
	 *   - tp_boundattr_string_hash:         Must also implement `tp_getattr'
	 *   - tp_callattr_string_hash:          Must also implement `tp_getattr'
	 *   - tp_callattr_string_hash_kw:       Must also implement `tp_getattr'
	 *   - tp_vcallattr_string_hashf:        Must also implement `tp_getattr'
	 *   - tp_getattr_string_len_hash:       Must also implement `tp_getattr'
	 *   - tp_delattr_string_len_hash:       Must also implement `tp_delattr'
	 *   - tp_setattr_string_len_hash:       Must also implement `tp_setattr'
	 *   - tp_hasattr_string_len_hash:       Must also implement `tp_getattr'
	 *   - tp_boundattr_string_len_hash:     Must also implement `tp_getattr'
	 *   - tp_callattr_string_len_hash:      Must also implement `tp_getattr'
	 *   - tp_callattr_string_len_hash_kw:   Must also implement `tp_getattr'
	 *   - tp_findattr_info_string_len_hash: Don't implement unless your type uses standard attribute
	 *                                       mechanisms (usually DeeObject_Generic*Attr) at some point,
	 *                                       in which case you can implement this operator to tell deemon
	 *                                       about attributes that *always* resolve to standard ones.
	 */

	/* [0..1] Like `tp_iterattr', but find a specific attribute */
	WUNUSED_T NONNULL_T((1, 2, 3, 4)) int
	(DCALL *tp_findattr)(DeeTypeObject *tp_self, DeeObject *self,
	                     struct Dee_attrspec const *__restrict specs,
	                     struct Dee_attrdesc *__restrict result);

	/* [0..1] Like `tp_getattr', but handles attribute errors:
	 * @return: >  0: Attribute exists.
	 * @return: == 0: Attribute doesn't exist.
	 * @return: <  0: An error occurred. .*/
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_hasattr)(DeeObject *self, /*String*/ DeeObject *attr);

	/* [0..1] Like `tp_getattr', but handles attribute errors:
	 * @return: Dee_BOUND_YES:     Attribute is bound.
	 * @return: Dee_BOUND_NO:      Attribute isn't bound.
	 * @return: Dee_BOUND_ERR:     An error occurred.
	 * @return: Dee_BOUND_MISSING: The attribute doesn't exist. */
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_boundattr)(DeeObject *self, /*String*/ DeeObject *attr);

	/* [0..1] Like `tp_getattr' + `DeeObject_Call()', but no need to create the intermediate object. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_callattr)(DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_callattr_kw)(DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *tp_vcallattrf)(DeeObject *self, /*String*/ DeeObject *attr, char const *format, va_list args);

	/* [0..1] Like the other operators above, but the attribute name is specified as a string. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_getattr_string_hash)(DeeObject *self, char const *attr, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_delattr_string_hash)(DeeObject *self, char const *attr, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2, 4)) int (DCALL *tp_setattr_string_hash)(DeeObject *self, char const *attr, Dee_hash_t hash, DeeObject *value);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_hasattr_string_hash)(DeeObject *self, char const *attr, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_boundattr_string_hash)(DeeObject *self, char const *attr, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_callattr_string_hash)(DeeObject *self, char const *attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_callattr_string_hash_kw)(DeeObject *self, char const *attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
	WUNUSED_T NONNULL_T((1, 2, 4)) DREF DeeObject *(DCALL *tp_vcallattr_string_hashf)(DeeObject *self, char const *attr, Dee_hash_t hash, char const *format, va_list args);

	/* [0..1] Like the other operators above, but the attribute name is specified as a string (that's not necessarily NUL-terminated). */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_getattr_string_len_hash)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_delattr_string_len_hash)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *tp_setattr_string_len_hash)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_hasattr_string_len_hash)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) int (DCALL *tp_boundattr_string_len_hash)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_callattr_string_len_hash)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_callattr_string_len_hash_kw)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);

	/* [0..1] For implementing `DeeObject_TFindAttrInfoStringLenHash()'.
	 * This should ONLY be defined by proxy types (like `Super'), or types that always
	 * make use of standard attribute access mechanisms for certain attributes (usually
	 * `DeeObject_Generic*Attr'), and know what they're doing.
	 * @return: true:   Attribute information was found, and `*retinfo' was filled.
	 *                  Note that in case of `Dee_ATTRINFO_CUSTOM', accessing the
	 *                  attribute can still fail for any number of reasons at runtime.
	 * @return: false:  Attribute information could not be found, and `*retinfo' is undefined.
	 *                  This has the same meaning as `Dee_HAS_ISYES(DeeObject_HasAttr(...))'. */
	WUNUSED_T NONNULL_T((1, 2, 3, 6)) bool (DCALL *tp_findattr_info_string_len_hash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, struct Dee_attrinfo *__restrict retinfo);

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *tp_callattr_tuple)(DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args);
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *tp_callattr_tuple_kw)(DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args, DeeObject *kw);
#elif !defined(CONFIG_BUILDING_DEEMON)
	Dee_funptr_t _tp_pad[2]; /* For binary compatibility */
#endif /* ... */
};

struct Dee_type_with {
	/* With-statement operators. */
	WUNUSED_T NONNULL_T((1)) int (DCALL *tp_enter)(DeeObject *__restrict self);
	WUNUSED_T NONNULL_T((1)) int (DCALL *tp_leave)(DeeObject *__restrict self);
};

struct Dee_type_callable {
	/* Same as `tp_call', but having support for keyword arguments. */
	WUNUSED_T ATTR_INS_T(3, 2) NONNULL_T((1))
	DREF DeeObject *(DCALL *tp_call_kw)(DeeObject *self, size_t argc,
	                                    DeeObject *const *argv, DeeObject *kw);

	/* Same as `tp_call', but "thisarg" gets injected as an additional, leading argument. */
	WUNUSED_T ATTR_INS_T(4, 3) NONNULL_T((1, 2))
	DREF DeeObject *(DCALL *tp_thiscall)(DeeObject *self, DeeObject *thisarg,
	                                     size_t argc, DeeObject *const *argv);

	/* Same as `tp_thiscall', but having support for keyword arguments. */
	WUNUSED_T ATTR_INS_T(4, 3) NONNULL_T((1, 2))
	DREF DeeObject *(DCALL *tp_thiscall_kw)(DeeObject *self, DeeObject *thisarg, size_t argc,
	                                        DeeObject *const *argv, DeeObject *kw);

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_call_tuple)(DeeObject *self, DeeObject *args);
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *tp_call_tuple_kw)(DeeObject *self, DeeObject *args, DeeObject *kw);
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *tp_thiscall_tuple)(DeeObject *self, DeeObject *thisarg, DeeObject *args);
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *tp_thiscall_tuple_kw)(DeeObject *self, DeeObject *thisarg, DeeObject *args, DeeObject *kw);
#elif !defined(CONFIG_BUILDING_DEEMON)
	Dee_funptr_t _tp_pad[4]; /* For binary compatibility */
#endif /* ... */
};

struct Dee_buffer;
struct Dee_type_buffer {
	/* Low-level buffer interface. */

	/* When implemented, `tp_getbuf' must fill in at least `bb_base' and `bb_size'
	 * @param: flags: Set of `DEE_BUFFER_F*' */
	WUNUSED_T NONNULL_T((1, 2))
	int  (DCALL *tp_getbuf)(DeeObject *__restrict self,
	                        struct Dee_buffer *__restrict info,
	                        unsigned int flags);

#define Dee_BUFFER_TYPE_FNORMAL   0x0000 /* Normal buffer type flags. */
#define Dee_BUFFER_TYPE_FREADONLY 0x0001 /* The buffer can only be used for reading.
	                                      * -> When set, `DeeObject_GetBuf' fails when
	                                      *    the `Dee_BUFFER_FWRITABLE' flag is set. */
	unsigned int tp_buffer_flags; /* Buffer type flags (Set of `DEE_BUFFER_TYPE_F*') */
};




/* WARNING: The `argv' vector should actually be written as `DeeObject *const *argv',
 *          however since it appears in as many places as it does, doing so would really
 *          just be unnecessary work. However because of this you must remember never
 *          to modify an argument vector!
 * For example: If your function is called from the interpreter, `argv' often points
 * into the associated frame's stack, meaning that modifications could bring along
 * deadly consequences!
 * Even in user-code itself, where it might seem as though you are able to write to
 * argument variables, in actuality, the compiler will notice and copy the argument
 * in a local variable at the beginning of the function, meaning you'll actually just
 * be modifying a local variable.
 * @param: self:  The obj-part of the objmethod.
 * @param: argc:  The number of arguments passed.
 * @param: argv:  [1..1][const][0..argc] Arguments.
 * @return: * :   The function return value.
 * @return: NULL: An error occurred. */
typedef WUNUSED_T ATTR_INS_T(3, 2) NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_objmethod_t)(DeeObject *self, size_t argc, DeeObject *const *argv);
typedef WUNUSED_T ATTR_INS_T(3, 2) NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_kwobjmethod_t)(DeeObject *self, size_t argc, DeeObject *const *argv, /*nullable*/ DeeObject *kw);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_getmethod_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_delmethod_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_setmethod_t)(DeeObject *self, DeeObject *value);
/* @return: Dee_BOUND_YES: Attribute is bound
 * @return: Dee_BOUND_NO:  Attribute isn't bound (reading it would throw `DeeError_UnboundAttribute')
 * @return: Dee_BOUND_ERR: Some other error occurred. */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_boundmethod_t)(DeeObject *__restrict self);
#if 1 // Dee_BOUND_YES == 1
#if defined(DCALL_RETURN_COMMON) || __SIZEOF_SIZE_T__ == __SIZEOF_INT__
/*!export -_DeeNone_**/
#ifdef DCALL_CALLER_CLEANUP
DFUNDEF size_t (DCALL _DeeNone_rets1)(void); /* Always returns "1" */
#define Dee_boundmethod__ALWAYS_PTR (Dee_boundmethod_t)&_DeeNone_rets1
#else /* DCALL_CALLER_CLEANUP */
DFUNDEF size_t (DCALL _DeeNone_rets1_1)(void *);
#define Dee_boundmethod__ALWAYS_PTR (Dee_boundmethod_t)&_DeeNone_rets1_1
#endif /* !DCALL_CALLER_CLEANUP */
#else /* DCALL_RETURN_COMMON || __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
#ifdef DCALL_CALLER_CLEANUP
DFUNDEF int (DCALL _DeeNone_reti1)(void); /* Always returns "1" */
#define Dee_boundmethod__ALWAYS_PTR (Dee_boundmethod_t)&_DeeNone_reti1
#else /* DCALL_CALLER_CLEANUP */
DFUNDEF int (DCALL _DeeNone_reti1_1)(void *);
#define Dee_boundmethod__ALWAYS_PTR (Dee_boundmethod_t)&_DeeNone_reti1_1
#endif /* !DCALL_CALLER_CLEANUP */
#endif /* !DCALL_RETURN_COMMON && __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
#else /* Dee_BOUND_YES == 1 */
#error "Unsupported value for `Dee_BOUND_YES'"
#endif /* Dee_BOUND_YES != 1 */

#if defined(__INTELLISENSE__) && defined(__cplusplus)
/* Highlight usage errors in IDE */
extern "C++" {
namespace __intern {
Dee_objmethod_t _Dee_RequiresObjMethod(decltype(nullptr));
Dee_kwobjmethod_t _Dee_RequiresKwObjMethod(decltype(nullptr));
Dee_objmethod_t _Dee_RequiresKwObjMethod_(decltype(nullptr));
Dee_getmethod_t _Dee_RequiresGetMethod(decltype(nullptr));
Dee_delmethod_t _Dee_RequiresDelMethod(decltype(nullptr));
Dee_setmethod_t _Dee_RequiresSetMethod(decltype(nullptr));
Dee_boundmethod_t _Dee_RequiresBoundMethod(decltype(nullptr));
template<class _TReturn, class _TSelf> Dee_objmethod_t _Dee_RequiresObjMethod(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(_TSelf *, size_t, DeeObject *const *));
template<class _TReturn, class _TSelf> Dee_kwobjmethod_t _Dee_RequiresKwObjMethod(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(_TSelf *, size_t, DeeObject *const *, /*nullable*/ DeeObject *kw));
template<class _TReturn, class _TSelf> Dee_objmethod_t _Dee_RequiresKwObjMethod_(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(_TSelf *, size_t, DeeObject *const *, /*nullable*/ DeeObject *kw));
template<class _TReturn, class _TSelf> Dee_getmethod_t _Dee_RequiresGetMethod(WUNUSED_T NONNULL_T((1)) DREF _TReturn *(DCALL *_meth)(_TSelf *__restrict));
template<class _TSelf> Dee_delmethod_t _Dee_RequiresDelMethod(WUNUSED_T NONNULL_T((1)) int (DCALL *_meth)(_TSelf *__restrict));
template<class _TSelf, class _TArg> Dee_setmethod_t _Dee_RequiresSetMethod(WUNUSED_T NONNULL_T((1, 2)) int (DCALL *_meth)(_TSelf *, _TArg *));
template<class _TSelf> Dee_boundmethod_t _Dee_RequiresBoundMethod(WUNUSED_T NONNULL_T((1)) int (DCALL *_meth)(_TSelf *__restrict));
} /* namespace __intern */
} /* extern "C++" */
#define Dee_REQUIRES_OBJMETHOD(meth)    ((decltype(::__intern::_Dee_RequiresObjMethod(meth)))(meth))
#define Dee_REQUIRES_KWOBJMETHOD(meth)  ((decltype(::__intern::_Dee_RequiresKwObjMethod(meth)))(meth))
#define Dee_REQUIRES_KWOBJMETHOD_(meth) ((decltype(::__intern::_Dee_RequiresKwObjMethod_(meth)))(meth))
#define Dee_REQUIRES_GETMETHOD(meth)    ((decltype(::__intern::_Dee_RequiresGetMethod(meth)))(meth))
#define Dee_REQUIRES_DELMETHOD(meth)    ((decltype(::__intern::_Dee_RequiresDelMethod(meth)))(meth))
#define Dee_REQUIRES_SETMETHOD(meth)    ((decltype(::__intern::_Dee_RequiresSetMethod(meth)))(meth))
#define Dee_REQUIRES_BOUNDMETHOD(meth)  ((decltype(::__intern::_Dee_RequiresBoundMethod(meth)))(meth))
#else /* __INTELLISENSE__ && __cplusplus */
#define Dee_REQUIRES_OBJMETHOD(meth)    ((Dee_objmethod_t)(meth))
#define Dee_REQUIRES_KWOBJMETHOD(meth)  ((Dee_kwobjmethod_t)(meth))
#define Dee_REQUIRES_KWOBJMETHOD_(meth) ((Dee_objmethod_t)(meth))
#define Dee_REQUIRES_GETMETHOD(meth)    ((Dee_getmethod_t)(meth))
#define Dee_REQUIRES_DELMETHOD(meth)    ((Dee_delmethod_t)(meth))
#define Dee_REQUIRES_SETMETHOD(meth)    ((Dee_setmethod_t)(meth))
#define Dee_REQUIRES_BOUNDMETHOD(meth)  ((Dee_boundmethod_t)(meth))
#endif /* !__INTELLISENSE__ || !__cplusplus */


/* Portable method attribute flags. These can be used in:
 * - struct Dee_type_method::m_flag
 * - struct Dee_type_getset::gs_flag
 *   - here, flags describe all defined callbacks, except
 *     - Dee_METHOD_FPURECALL and Dee_METHOD_FCONSTCALL only describe "gs_get" and "gs_bound"
 * - DeeCMethodObject::cm_flags
 */
#define Dee_METHOD_FMASK        0xffffff00 /* Mask of portable method flags. */
#define Dee_METHOD_FNORMAL      0x00000000 /* Normal flags */
#define Dee_METHOD_FEXACTRETURN 0x04000000 /* RTTI return types are exact (allowed to assume `return == NULL || Dee_TYPE(return) == TYPE_FROM_RTTI')
                                            * WARNING: _hostasm treats `struct type_member' with doc strings as though this flag was *always* set! */
#define Dee_METHOD_FNOTHROW     0x08000000 /* Function never throws an exception and always returns normally (implies `ATTR_RETNONNULL' and also means no OOM allowed) */
#define Dee_METHOD_FNORETURN    0x10000000 /* Function never returns normally (always throws an exception, or calls `exit(3)') */
#define Dee_METHOD_FPURECALL    0x20000000 /* ATTR_PURE: Function does not affect the global state (except for reference counts or memory usage)
                                            * Also means that the function doesn't invoke user-overwritable operators (except OOM hooks).
                                            * When set, repeated calls with identical arguments may be combined. */
#define Dee_METHOD_FCONSTCALL   0x40000000 /* ATTR_CONST: Function does not modify or read mutable args, or affect
                                            * the global state (except for reference counts or memory usage).
                                            * Also means that the function doesn't invoke user-overwritable operators (except OOM hooks).
                                            * Implies `Dee_METHOD_FPURECALL'. Function can be used in constant propagation.
                                            * IMPORTANT: when attached to `OPERATOR_ITER', the meaning isn't that `operator iter()'
                                            *            can be called at compile-time (it never can, since the whole point of an iterator
                                            *            is that it holds a small state usable for enumeration of a sequence).
                                            *            Instead, here the meaning is that `DeeObject_Foreach()' can be called at compile-
                                            *            time (or alternatively creating an iterator, and then enumerating it). */
#define Dee_METHOD_FNOREFESCAPE 0x80000000 /* Optimizer hint flag: when invoked, this method never incref's
                                            * the "this" argument (unless it also appears in argv/kw, or "this"
                                            * has an accessible reference via some other operator chain that
                                            * does not involve "this" directly)
                                            * Note that if an incref does happen, it must *always* be followed
                                            * by another decref before the function returns (i.e. refcnt on
                                            * entry must match refcnt on exit (except as listed above))
                                            *
                                            * !!! IMPORTANT !!! If you're uncertain about this flag, don't set it!
                                            * IMPORTANT: when attached to `OPERATOR_ITER', same special case as `Dee_METHOD_FCONSTCALL' */

/* Extra conditions that may be used to restrict when `Dee_METHOD_FPURECALL'
 * and `Dee_METHOD_FCONSTCALL' should be considered enabled (must be combined
 * with the resp. flag in order to affect anything).
 *
 * NOTES:
 * - IS_CONSTEXPR(ob.operator foo) (where "foo" is a unary operators) means:
 *   >> !DeeType_HasOperator(Dee_TYPE(ob), OPERATOR_FOO) ||
 *   >> (DeeType_GetOperatorFlags(Dee_TYPE(ob), OPERATOR_FOO) & METHOD_FCONSTCALL);
 */
#define Dee_METHOD_FCONSTCALL_IF_MASK                       0x0000ff00 /* Mask of possible CONSTCALL conditions. */
#define Dee_METHOD_FCONSTCALL_IF_TRUE                       0x00000000 /* >> true; */
#define Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST             0x00000100 /* >> (for (local arg: ...) DeeType_IsConstCastable(Dee_TYPE(arg))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_CONSTELEM_ARGS_CONSTCAST   0x00000200 /* >> ((for (local arg: ...) DeeType_IsConstCastable(Dee_TYPE(arg))) && ...) && IS_CONSTEXPR(thisarg.operator iter); */
#define Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTSTR          0x00000200 /* >> (for (local x: thisarg) IS_CONSTEXPR(x.operator str)) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR         0x00000300 /* >> (for (local x: thisarg) IS_CONSTEXPR(x.operator repr)) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH         0x00000400 /* >> (for (local x: thisarg) IS_CONSTEXPR(x.operator hash)) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ             0x00000500 /* >> (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local a, b: zip(thisarg, arg)) IS_CONSTEXPR(a == b, a != b, [a.operator hash(), b.operator hash()]))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCMP               0x00000600 /* >> (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local a, b: zip(thisarg, arg)) IS_CONSTEXPR(a == b, a != b, a < b, a > b, a <= b, a >= b))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS          0x00000700 /* >> (for (local arg: ...) for (local elem: thisarg) IS_CONSTEXPR(elem == arg)) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ             0x00000800 /* TODO (for sets and maps) */
#define Dee_METHOD_FCONSTCALL_IF_SET_CONSTCMP               0x00000900 /* TODO (for sets and maps) */
#define Dee_METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS          0x00000a00 /* TODO (for sets and maps) */
#define Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST_ROBYTES 0x00000b00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_IsWritable(thisarg)) && (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local x: arg) DeeType_IsConstCastable(Dee_TYPE(x)))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES  0x00000c00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_IsWritable(thisarg)) && (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local x: arg) IS_CONSTEXPR(x.operator str))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES     0x00000d00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_IsWritable(thisarg)) && ((for (local arg: ...) DeeBytes_Check(arg) ? !DeeBytes_IsWritable(arg) : DeeType_IsConstCastable(Dee_TYPE(arg))) && ...); */
#define Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES      0x00000e00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_IsWritable(thisarg)) && ((for (local arg: ...) DeeBytes_Check(arg) ? !DeeBytes_IsWritable(arg) : IS_CONSTEXPR(arg.operator str)) && ...); */
#define Dee_METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL          0x00000f00 /* Special casing for `OPERATOR_CALL' of `DeeInstanceMethod_Type', `DeeObjMethod_Type', `DeeKwObjMethod_Type', `DeeClsMethod_Type', `DeeKwClsMethod_Type', `DeeClsProperty_Type', `DeeClsMember_Type', `DeeCMethod_Type' and `DeeKwCMethod_Type' */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR            0x00001000 /* Foreach field in this,args...: IS_CONSTEXPR(f.operator str)    (fields are STRUCT_OBJECT-like tp_members) */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR           0x00001100 /* Foreach field in this,args...: IS_CONSTEXPR(f.operator repr)   (fields are STRUCT_OBJECT-like tp_members) */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ          0x00001200 /* Foreach field in pair(this,arg0): IS_CONSTEXPR(a == b, a != b, [a.operator hash(), b.operator hash()]) */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP            0x00001300 /* Foreach field in pair(this,arg0): IS_CONSTEXPR(a == b, a != b, a < b, a > b, a <= b, a >= b) */

#ifdef DEE_SOURCE
#define METHOD_FMASK                                    Dee_METHOD_FMASK
#define METHOD_FNORMAL                                  Dee_METHOD_FNORMAL
#define METHOD_FEXACTRETURN                             Dee_METHOD_FEXACTRETURN
#define METHOD_FNOTHROW                                 Dee_METHOD_FNOTHROW
#define METHOD_FNORETURN                                Dee_METHOD_FNORETURN
#define METHOD_FPURECALL                                Dee_METHOD_FPURECALL
#define METHOD_FCONSTCALL                               Dee_METHOD_FCONSTCALL
#define METHOD_FNOREFESCAPE                             Dee_METHOD_FNOREFESCAPE
#define METHOD_FCONSTCALL_IF_MASK                       Dee_METHOD_FCONSTCALL_IF_MASK
#define METHOD_FCONSTCALL_IF_TRUE                       Dee_METHOD_FCONSTCALL_IF_TRUE
#define METHOD_FCONSTCALL_IF_ARGS_CONSTCAST             Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST
#define METHOD_FCONSTCALL_IF_CONSTELEM_ARGS_CONSTCAST   Dee_METHOD_FCONSTCALL_IF_CONSTELEM_ARGS_CONSTCAST
#define METHOD_FCONSTCALL_IF_THISELEM_CONSTSTR          Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTSTR
#define METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR         Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR
#define METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH         Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH
#define METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ             Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ
#define METHOD_FCONSTCALL_IF_SEQ_CONSTCMP               Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCMP
#define METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS          Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS
#define METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ             Dee_METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ
#define METHOD_FCONSTCALL_IF_SET_CONSTCMP               Dee_METHOD_FCONSTCALL_IF_SET_CONSTCMP
#define METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS          Dee_METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS
#define METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST         Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST_ROBYTES
#define METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST_ROBYTES Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST_ROBYTES
#define METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR          Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES
#define METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES  Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES
#define METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES     Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES
#define METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES      Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES
#define METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL          Dee_METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL
#define METHOD_FCONSTCALL_IF_THISARG_ROBYTES            Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES
#define METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR            Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR
#define METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR           Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR
#define METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ          Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ
#define METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP            Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP
#endif /* DEE_SOURCE */

/* Check if the condition from `flags & Dee_METHOD_FCONSTCALL_IF_MASK' is
 * fulfilled when applied to the given argument list (which does not
 * include the "this" argument, if there would have been one). */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(4, 3) bool
(DCALL DeeMethodFlags_VerifyConstCallCondition)(uintptr_t flags, DeeObject *thisarg,
                                                size_t argc, DeeObject *const *argv,
                                                DeeObject *kw);
#ifndef __OPTIMIZE_SIZE__
#define DeeMethodFlags_VerifyConstCallCondition(flags, thisarg, argc, argv, kw)      \
	((((flags) & Dee_METHOD_FCONSTCALL_IF_MASK) == Dee_METHOD_FCONSTCALL_IF_TRUE) || \
	 DeeMethodFlags_VerifyConstCallCondition(flags, thisarg, argc, argv, kw))
#endif /* !__OPTIMIZE_SIZE__ */


/* Possible values for `struct Dee_type_method::m_flag' (also accepts `Dee_METHOD_FMASK') */
#define Dee_TYPE_METHOD_FNORMAL 0x00000000 /* Normal type method flags. */
#define Dee_TYPE_METHOD_FKWDS   0x00000001 /* `m_func' takes a keywords argument.
                                            * When set, `m_func' is actually a `Dee_kwobjmethod_t' */
#ifdef DEE_SOURCE
#define TYPE_METHOD_FNORMAL Dee_TYPE_METHOD_FNORMAL
#define TYPE_METHOD_FKWDS   Dee_TYPE_METHOD_FKWDS
#endif /* DEE_SOURCE */

struct Dee_type_method {
	char const           *m_name;   /* [1..1][SENTINAL(NULL)] Method name. */
	Dee_objmethod_t       m_func;   /* [1..1] The method that is getting invoked. */
	/*utf-8*/ char const *m_doc;    /* [0..1] Documentation string. */
	uintptr_t             m_flag;   /* Method flags (Set of `Dee_TYPE_METHOD_F*'). */
};
#define Dee_TYPE_METHOD(name, func, doc)             { name, Dee_REQUIRES_OBJMETHOD(func), DOC(doc), Dee_TYPE_METHOD_FNORMAL }
#define Dee_TYPE_METHOD_NODOC(name, func)            { name, Dee_REQUIRES_OBJMETHOD(func), NULL, Dee_TYPE_METHOD_FNORMAL }
#define Dee_TYPE_KWMETHOD(name, func, doc)           { name, Dee_REQUIRES_KWOBJMETHOD_(func), DOC(doc), Dee_TYPE_METHOD_FKWDS }
#define Dee_TYPE_KWMETHOD_NODOC(name, func)          { name, Dee_REQUIRES_KWOBJMETHOD_(func), NULL, Dee_TYPE_METHOD_FKWDS }
#define Dee_TYPE_METHOD_F(name, func, flags, doc)    { name, Dee_REQUIRES_OBJMETHOD(func), DOC(doc), (flags) }
#define Dee_TYPE_METHOD_F_NODOC(name, func, flags)   { name, Dee_REQUIRES_OBJMETHOD(func), NULL, (flags) }
#define Dee_TYPE_KWMETHOD_F(name, func, flags, doc)  { name, Dee_REQUIRES_KWOBJMETHOD_(func), DOC(doc), Dee_TYPE_METHOD_FKWDS | (flags) }
#define Dee_TYPE_KWMETHOD_F_NODOC(name, func, flags) { name, Dee_REQUIRES_KWOBJMETHOD_(func), NULL, Dee_TYPE_METHOD_FKWDS | (flags) }
#define Dee_TYPE_METHOD_END                          { NULL, NULL, NULL, 0 }
#ifdef DEE_SOURCE
#define TYPE_METHOD           Dee_TYPE_METHOD
#define TYPE_METHOD_NODOC     Dee_TYPE_METHOD_NODOC
#define TYPE_KWMETHOD         Dee_TYPE_KWMETHOD
#define TYPE_KWMETHOD_NODOC   Dee_TYPE_KWMETHOD_NODOC
#define TYPE_METHOD_F         Dee_TYPE_METHOD_F
#define TYPE_METHOD_F_NODOC   Dee_TYPE_METHOD_F_NODOC
#define TYPE_KWMETHOD_F       Dee_TYPE_KWMETHOD_F
#define TYPE_KWMETHOD_F_NODOC Dee_TYPE_KWMETHOD_F_NODOC
#define TYPE_METHOD_END       Dee_TYPE_METHOD_END
#endif /* DEE_SOURCE */

/* Possible values for `struct Dee_type_getset::gs_flag' (also accepts `Dee_METHOD_FMASK') */
#define Dee_TYPE_GETSET_FNORMAL 0x00000000 /* Normal type method flags. */
#ifdef DEE_SOURCE
#define TYPE_GETSET_FNORMAL Dee_TYPE_GETSET_FNORMAL
#endif /* DEE_SOURCE */

struct Dee_type_getset {
	char const           *gs_name;  /* [1..1][SENTINAL(NULL)] Member name. */
	/* Getset callbacks (NULL callbacks will result in `Error.AttributeError' being raised) */
	Dee_getmethod_t       gs_get;   /* [0..1] Getter callback. */
	Dee_delmethod_t       gs_del;   /* [0..1] Delete callback. */
	Dee_setmethod_t       gs_set;   /* [0..1] Setter callback. */
	Dee_boundmethod_t     gs_bound; /* [0..1] Is-bound callback. (optional; if not given, call `gs_get' and see if it throws `UnboundAttribute' or `AttributeError' / `NotImplemented') */
	/*utf-8*/ char const *gs_doc;   /* [0..1] Documentation string. */
	uintptr_t             gs_flags; /* Getset flags (set of `Dee_TYPE_GETSET_F*') */
};
#define Dee_TYPE_GETSET(name, get, del, set, doc)                        { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_NODOC(name, get, del, set)                       { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_BOUND(name, get, del, set, bound, doc)           { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_REQUIRES_BOUNDMETHOD(bound), DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_BOUND_NODOC(name, get, del, set, bound)          { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_REQUIRES_BOUNDMETHOD(bound), NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER(name, get, doc)                                  { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_NODOC(name, get)                                 { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_BOUND(name, get, bound, doc)                     { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_REQUIRES_BOUNDMETHOD(bound), DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_BOUND_NODOC(name, get, bound)                    { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_REQUIRES_BOUNDMETHOD(bound), NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_F(name, get, del, set, flags, doc)               { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, DOC(doc), flags }
#define Dee_TYPE_GETSET_F_NODOC(name, get, del, set, flags)              { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, NULL, flags }
#define Dee_TYPE_GETSET_BOUND_F(name, get, del, set, bound, flags, doc)  { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_REQUIRES_BOUNDMETHOD(bound), DOC(doc), flags }
#define Dee_TYPE_GETSET_BOUND_F_NODOC(name, get, del, set, bound, flags) { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_REQUIRES_BOUNDMETHOD(bound), NULL, flags }
#define Dee_TYPE_GETTER_F(name, get, flags, doc)                         { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, DOC(doc), flags }
#define Dee_TYPE_GETTER_F_NODOC(name, get, flags)                        { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, NULL, flags }
#define Dee_TYPE_GETTER_BOUND_F(name, get, bound, flags, doc)            { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_REQUIRES_BOUNDMETHOD(bound), DOC(doc), flags }
#define Dee_TYPE_GETTER_BOUND_F_NODOC(name, get, bound, flags)           { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_REQUIRES_BOUNDMETHOD(bound), NULL, flags }
/* TODO: Re-write more stuff to use "Dee_TYPE_GETTER_AB" instead of "Dee_TYPE_GETTER" */
#ifdef __pic__ /* Reduce the number of relocations (manually use "Dee_boundmethod__ALWAYS_PTR" if "always-bound" is required) */
#define Dee_TYPE_GETSET_AB(name, get, del, set, doc)                     { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_AB_NODOC(name, get, del, set)                    { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_AB(name, get, doc)                               { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_AB_NODOC(name, get)                              { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_AB_F(name, get, del, set, flags, doc)            { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, DOC(doc), flags }
#define Dee_TYPE_GETSET_AB_F_NODOC(name, get, del, set, flags)           { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), NULL, NULL, flags }
#define Dee_TYPE_GETTER_AB_F(name, get, flags, doc)                      { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, DOC(doc), flags }
#define Dee_TYPE_GETTER_AB_F_NODOC(name, get, flags)                     { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, NULL, NULL, flags }
#else /* __pic__ */
#define Dee_TYPE_GETSET_AB(name, get, del, set, doc)                     { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_boundmethod__ALWAYS_PTR, DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_AB_NODOC(name, get, del, set)                    { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_boundmethod__ALWAYS_PTR, NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_AB(name, get, doc)                               { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_boundmethod__ALWAYS_PTR, DOC(doc), Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETTER_AB_NODOC(name, get)                              { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_boundmethod__ALWAYS_PTR, NULL, Dee_TYPE_GETSET_FNORMAL }
#define Dee_TYPE_GETSET_AB_F(name, get, del, set, flags, doc)            { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_boundmethod__ALWAYS_PTR, DOC(doc), flags }
#define Dee_TYPE_GETSET_AB_F_NODOC(name, get, del, set, flags)           { name, Dee_REQUIRES_GETMETHOD(get), Dee_REQUIRES_DELMETHOD(del), Dee_REQUIRES_SETMETHOD(set), Dee_boundmethod__ALWAYS_PTR, NULL, flags }
#define Dee_TYPE_GETTER_AB_F(name, get, flags, doc)                      { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_boundmethod__ALWAYS_PTR, DOC(doc), flags }
#define Dee_TYPE_GETTER_AB_F_NODOC(name, get, flags)                     { name, Dee_REQUIRES_GETMETHOD(get), NULL, NULL, Dee_boundmethod__ALWAYS_PTR, NULL, flags }
#endif /* !__pic__ */
#define Dee_TYPE_GETSET_END                                              { NULL, NULL, NULL, NULL, NULL, NULL, 0 }
#ifdef DEE_SOURCE
#define TYPE_GETSET               Dee_TYPE_GETSET
#define TYPE_GETSET_NODOC         Dee_TYPE_GETSET_NODOC
#define TYPE_GETSET_BOUND         Dee_TYPE_GETSET_BOUND
#define TYPE_GETSET_BOUND_NODOC   Dee_TYPE_GETSET_BOUND_NODOC
#define TYPE_GETTER               Dee_TYPE_GETTER
#define TYPE_GETTER_NODOC         Dee_TYPE_GETTER_NODOC
#define TYPE_GETTER_BOUND         Dee_TYPE_GETTER_BOUND
#define TYPE_GETTER_BOUND_NODOC   Dee_TYPE_GETTER_BOUND_NODOC
#define TYPE_GETSET_F             Dee_TYPE_GETSET_F
#define TYPE_GETSET_F_NODOC       Dee_TYPE_GETSET_F_NODOC
#define TYPE_GETSET_BOUND_F       Dee_TYPE_GETSET_BOUND_F
#define TYPE_GETSET_BOUND_F_NODOC Dee_TYPE_GETSET_BOUND_F_NODOC
#define TYPE_GETTER_F             Dee_TYPE_GETTER_F
#define TYPE_GETTER_F_NODOC       Dee_TYPE_GETTER_F_NODOC
#define TYPE_GETTER_BOUND_F       Dee_TYPE_GETTER_BOUND_F
#define TYPE_GETTER_BOUND_F_NODOC Dee_TYPE_GETTER_BOUND_F_NODOC
#define TYPE_GETSET_AB            Dee_TYPE_GETSET_AB
#define TYPE_GETSET_AB_NODOC      Dee_TYPE_GETSET_AB_NODOC
#define TYPE_GETTER_AB            Dee_TYPE_GETTER_AB
#define TYPE_GETTER_AB_NODOC      Dee_TYPE_GETTER_AB_NODOC
#define TYPE_GETSET_AB_F          Dee_TYPE_GETSET_AB_F
#define TYPE_GETSET_AB_F_NODOC    Dee_TYPE_GETSET_AB_F_NODOC
#define TYPE_GETTER_AB_F          Dee_TYPE_GETTER_AB_F
#define TYPE_GETTER_AB_F_NODOC    Dee_TYPE_GETTER_AB_F_NODOC
#define TYPE_GETSET_END           Dee_TYPE_GETSET_END
#endif /* DEE_SOURCE */



/* Member type codes. */
#define Dee_STRUCT_NONE        0x0001 /* Ignore offset and always return `none' (Useful for forward/backward no-op compatibility) */
#define Dee_STRUCT_OBJECT      0x8003 /* `[0..1] DREF DeeObject *const' (raise `Error.AttributeError' if `NULL') */
#define Dee_STRUCT_WOBJECT     0x0007 /* `[0..1] struct Dee_weakref' (raise `Error.AttributeError' if locking fails) */
#define Dee_STRUCT_OBJECT_OPT  0x800b /* `[0..1] DREF DeeObject *const' (return `none' if NULL) */
#define Dee_STRUCT_WOBJECT_OPT 0x000f /* `[0..1] struct Dee_weakref' (return `none' if locking fails) */
#define Dee_STRUCT_OBJECT_AB   0x8013 /* `[1..1] DREF DeeObject *const' (never NULL) */
#define Dee_STRUCT_CSTR        0x8021 /* `[0..1] char const *' (utf-8) (Accessible as `DeeStringObject'; raise `Error.AttributeError' if `NULL') */
#define Dee_STRUCT_CSTR_OPT    0x8023 /* `[0..1] char const *' (utf-8) (Accessible as `DeeStringObject'; return `none' when `NULL') */
#define Dee_STRUCT_CSTR_EMPTY  0x8025 /* `[0..1] char const *' (utf-8) (Accessible as `DeeStringObject'; return an empty string when `NULL') */
#define Dee_STRUCT_STRING      0x8027 /* `char const[*]' (utf-8) (Accessible as `DeeStringObject') */
#define Dee_STRUCT_CHAR        0x0029 /* `unsigned char const' (latin-1) (Accessible as `DeeStringObject') */
#define Dee_STRUCT_VARIANT     0x002b /* `struct Dee_variant' (combine with `Dee_STRUCT_CONST' to make read-only) */
#define Dee_STRUCT_BOOL8       0x0041 /* `uint8_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOL16      0x0043 /* `uint16_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOL32      0x0045 /* `uint32_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOL64      0x0047 /* `uint64_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT0    0x0061 /* `uint8_t & 0x01' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT1    0x0063 /* `uint8_t & 0x02' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT2    0x0065 /* `uint8_t & 0x04' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT3    0x0067 /* `uint8_t & 0x08' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT4    0x0069 /* `uint8_t & 0x10' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT5    0x006b /* `uint8_t & 0x20' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT6    0x006d /* `uint8_t & 0x40' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT7    0x006f /* `uint8_t & 0x80' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT(mask)                                                   \
	((mask) == 0x80 ? Dee_STRUCT_BOOLBIT7 : (mask) == 0x40 ? Dee_STRUCT_BOOLBIT6 : \
	 (mask) == 0x20 ? Dee_STRUCT_BOOLBIT5 : (mask) == 0x10 ? Dee_STRUCT_BOOLBIT4 : \
	 (mask) == 0x08 ? Dee_STRUCT_BOOLBIT3 : (mask) == 0x04 ? Dee_STRUCT_BOOLBIT2 : \
	 (mask) == 0x02 ? Dee_STRUCT_BOOLBIT1 : Dee_STRUCT_BOOLBIT0)
#define Dee_STRUCT_BOOLBITMASK(type) (1 << (((type) - Dee_STRUCT_BOOLBIT0) >> 1))
#define Dee_STRUCT_FLOAT       0x0081 /* `float' */
#define Dee_STRUCT_DOUBLE      0x0083 /* `double' */
#define Dee_STRUCT_LDOUBLE     0x0085 /* `long double' */
#define Dee_STRUCT_VOID        Dee_STRUCT_NONE /* `void' */
#define Dee_STRUCT_INT8        0x1001 /* `int8_t' */
#define Dee_STRUCT_INT16       0x1003 /* `int16_t' */
#define Dee_STRUCT_INT32       0x1005 /* `int32_t' */
#define Dee_STRUCT_INT64       0x1007 /* `int64_t' */
#define Dee_STRUCT_INT128      0x1009 /* `Dee_int128_t' */
#define Dee_STRUCT_UNSIGNED    0x0010 /* FLAG: Unsigned integer (Use with `STRUCT_INT*'). */
#define Dee_STRUCT_ATOMIC      0x4000 /* FLAG: Atomic read/write access (Use with `STRUCT_INT*'). */
#define Dee_STRUCT_CONST       0x8000 /* FLAG: Read-only field. */

#define _Dee_PRIVATE_STRUCT_INT1  Dee_STRUCT_INT8
#define _Dee_PRIVATE_STRUCT_INT2  Dee_STRUCT_INT16
#define _Dee_PRIVATE_STRUCT_INT4  Dee_STRUCT_INT32
#define _Dee_PRIVATE_STRUCT_INT8  Dee_STRUCT_INT64
#define _Dee_PRIVATE_STRUCT_INT16 Dee_STRUCT_INT128
#define _Dee_PRIVATE_STRUCT_INT(sizeof) _Dee_PRIVATE_STRUCT_INT##sizeof
#define Dee_STRUCT_INTEGER(sizeof) _Dee_PRIVATE_STRUCT_INT(sizeof)

#define _Dee_PRIVATE_STRUCT_BOOL1 Dee_STRUCT_BOOL8
#define _Dee_PRIVATE_STRUCT_BOOL2 Dee_STRUCT_BOOL16
#define _Dee_PRIVATE_STRUCT_BOOL4 Dee_STRUCT_BOOL32
#define _Dee_PRIVATE_STRUCT_BOOL8 Dee_STRUCT_BOOL64
#define _Dee_PRIVATE_STRUCT_BOOL(sizeof) _Dee_PRIVATE_STRUCT_BOOL##sizeof
#define Dee_STRUCT_BOOL(sizeof) _Dee_PRIVATE_STRUCT_BOOL(sizeof)

#ifdef DEE_SOURCE
#define STRUCT_NONE        Dee_STRUCT_NONE        /* Ignore offset and always return `none' (Useful for forward/backward no-op compatibility) */
#define STRUCT_OBJECT      Dee_STRUCT_OBJECT      /* `[0..1] DREF DeeObject *const' (raise `Error.AttributeError' if `NULL') */
#define STRUCT_WOBJECT     Dee_STRUCT_WOBJECT     /* `[0..1] struct Dee_weakref' (raise `Error.AttributeError' if locking fails) */
#define STRUCT_OBJECT_OPT  Dee_STRUCT_OBJECT_OPT  /* `[0..1] DREF DeeObject *const' (return `none' if NULL) */
#define STRUCT_WOBJECT_OPT Dee_STRUCT_WOBJECT_OPT /* `[0..1] struct Dee_weakref' (return `none' if locking fails) */
#define STRUCT_OBJECT_AB   Dee_STRUCT_OBJECT_AB   /* `[1..1] DREF DeeObject *const' (never NULL) */
#define STRUCT_CSTR        Dee_STRUCT_CSTR        /* `[0..1] char const *' (utf-8) (Accessible as `DeeStringObject'; raise `Error.AttributeError' if `NULL') */
#define STRUCT_CSTR_OPT    Dee_STRUCT_CSTR_OPT    /* `[0..1] char const *' (utf-8) (Accessible as `DeeStringObject'; return `none' when `NULL') */
#define STRUCT_CSTR_EMPTY  Dee_STRUCT_CSTR_EMPTY  /* `[0..1] char const *' (utf-8) (Accessible as `DeeStringObject'; return an empty string when `NULL') */
#define STRUCT_STRING      Dee_STRUCT_STRING      /* `char const[*]' (utf-8) (Accessible as `DeeStringObject') */
#define STRUCT_CHAR        Dee_STRUCT_CHAR        /* `unsigned char const' (latin-1) (Accessible as `DeeStringObject') */
#define STRUCT_VARIANT     Dee_STRUCT_VARIANT     /* `struct Dee_variant' (combine with `Dee_STRUCT_CONST' to make read-only) */
#define STRUCT_BOOL8       Dee_STRUCT_BOOL8       /* `uint8_t' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOL16      Dee_STRUCT_BOOL16      /* `uint16_t' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOL32      Dee_STRUCT_BOOL32      /* `uint32_t' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOL64      Dee_STRUCT_BOOL64      /* `uint64_t' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT0    Dee_STRUCT_BOOLBIT0    /* `uint8_t & 0x01' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT1    Dee_STRUCT_BOOLBIT1    /* `uint8_t & 0x02' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT2    Dee_STRUCT_BOOLBIT2    /* `uint8_t & 0x04' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT3    Dee_STRUCT_BOOLBIT3    /* `uint8_t & 0x08' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT4    Dee_STRUCT_BOOLBIT4    /* `uint8_t & 0x10' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT5    Dee_STRUCT_BOOLBIT5    /* `uint8_t & 0x20' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT6    Dee_STRUCT_BOOLBIT6    /* `uint8_t & 0x40' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOLBIT7    Dee_STRUCT_BOOLBIT7    /* `uint8_t & 0x80' (Accessible as `DeeBoolObject') */
#define STRUCT_BOOL        Dee_STRUCT_BOOL
#define STRUCT_BOOLBIT     Dee_STRUCT_BOOLBIT
#define STRUCT_BOOLBITMASK Dee_STRUCT_BOOLBITMASK
#define STRUCT_FLOAT       Dee_STRUCT_FLOAT       /* `float' */
#define STRUCT_DOUBLE      Dee_STRUCT_DOUBLE      /* `double' */
#define STRUCT_LDOUBLE     Dee_STRUCT_LDOUBLE     /* `long double' */
#define STRUCT_VOID        Dee_STRUCT_VOID        /* `void' */
#define STRUCT_INT8        Dee_STRUCT_INT8        /* `int8_t' */
#define STRUCT_INT16       Dee_STRUCT_INT16       /* `int16_t' */
#define STRUCT_INT32       Dee_STRUCT_INT32       /* `int32_t' */
#define STRUCT_INT64       Dee_STRUCT_INT64       /* `int64_t' */
#define STRUCT_INT128      Dee_STRUCT_INT128      /* `Dee_int128_t' */
#define STRUCT_UNSIGNED    Dee_STRUCT_UNSIGNED    /* FLAG: Unsigned integer (Use with `STRUCT_INT*'). */
#define STRUCT_ATOMIC      Dee_STRUCT_ATOMIC      /* FLAG: Atomic read/write access (Use with `STRUCT_INT*'). */
#define STRUCT_CONST       Dee_STRUCT_CONST       /* FLAG: Read-only field. */
#define STRUCT_INTEGER     Dee_STRUCT_INTEGER

/* Helper macros for certain types. */
#define STRUCT_CBOOL       Dee_STRUCT_BOOL(__SIZEOF_CHAR__)
#define STRUCT_BOOLPTR     Dee_STRUCT_BOOL(__SIZEOF_POINTER__)
#define STRUCT_INTPTR_T    Dee_STRUCT_INTEGER(__SIZEOF_POINTER__)
#define STRUCT_UINTPTR_T   (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(__SIZEOF_POINTER__))
#define STRUCT_HASH_T      (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(Dee_SIZEOF_HASH_T))
#define STRUCT_SIZE_T      (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(__SIZEOF_SIZE_T__))
#define STRUCT_SSIZE_T     Dee_STRUCT_INTEGER(__SIZEOF_SIZE_T__)
#define STRUCT_INT         Dee_STRUCT_INTEGER(__SIZEOF_INT__)
#define STRUCT_UINT        (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(__SIZEOF_INT__))
#define STRUCT_INT8_T      Dee_STRUCT_INT8
#define STRUCT_INT16_T     Dee_STRUCT_INT16
#define STRUCT_INT32_T     Dee_STRUCT_INT32
#define STRUCT_INT64_T     Dee_STRUCT_INT64
#define STRUCT_INT128_T    Dee_STRUCT_INT128
#define STRUCT_UINT8_T     (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT8)
#define STRUCT_UINT16_T    (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT16)
#define STRUCT_UINT32_T    (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT32)
#define STRUCT_UINT64_T    (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT64)
#define STRUCT_UINT128_T   (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT128)
#endif /* DEE_SOURCE */


union Dee_type_member_desc {
	DeeObject *md_const;  /* [valid_if(Dee_TYPE_MEMBER_ISCONST(this))][1..1] Constant. */
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		__UINTPTR_HALF_TYPE__ mdf_type;   /* [valid_if(Dee_TYPE_MEMBER_ISFIELD(this))] Field type (One of `STRUCT_*'). */
		__UINTPTR_HALF_TYPE__ mdf_offset; /* [valid_if(Dee_TYPE_MEMBER_ISFIELD(this))] Field offset (offsetof() field). */
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		__UINTPTR_HALF_TYPE__ mdf_offset; /* [valid_if(Dee_TYPE_MEMBER_ISFIELD(this))] Field offset (offsetof() field). */
		__UINTPTR_HALF_TYPE__ mdf_type;   /* [valid_if(Dee_TYPE_MEMBER_ISFIELD(this))] Field type (One of `STRUCT_*'). */
#endif /* !... */
	}          md_field; /* Field descriptor */
};
#define Dee_TYPE_MEMBER_DESC_INITCONST(value) { Dee_AsObject(value) }
#if __SIZEOF_POINTER__ == 4
#define Dee_TYPE_MEMBER_DESC_INITFIELD(type, offset) \
	{ (DeeObject *)(uintptr_t)((uint32_t)(type) | ((uint32_t)(offset) << 16)) }
#elif __SIZEOF_POINTER__ == 8
#define Dee_TYPE_MEMBER_DESC_INITFIELD(type, offset) \
	{ (DeeObject *)(uintptr_t)((uint64_t)(type) | ((uint64_t)(offset) << 32)) }
#else /* __SIZEOF_POINTER__ == ... */
#define Dee_TYPE_MEMBER_DESC_INITFIELD(type, offset) \
	{ (DeeObject *)(uintptr_t)((uintptr_t)(type) | ((uintptr_t)(offset) << (__SIZEOF_POINTER__ * 8))) }
#endif /* __SIZEOF_POINTER__ != ... */

struct Dee_type_member {
	char const                *m_name; /* [1..1][SENTINAL(NULL)] Member name. */
	union Dee_type_member_desc m_desc; /* Type member descriptor */
	/*utf-8*/ char const      *m_doc;  /* [0..1] Documentation string. */
};

#define Dee_TYPE_MEMBER_ISCONST(x) (((x)->m_desc.md_field.mdf_type & 1) == 0)
#define Dee_TYPE_MEMBER_ISFIELD(x) (((x)->m_desc.md_field.mdf_type & 1) != 0)
#define Dee_TYPE_MEMBER_END \
	{ NULL }
#define Dee_TYPE_MEMBER_FIELD_DOC(name, type, offset, doc) \
	{ name, Dee_TYPE_MEMBER_DESC_INITFIELD(type, offset), DOC(doc) }
#define Dee_TYPE_MEMBER_CONST_DOC(name, value, doc) \
	{ name, Dee_TYPE_MEMBER_DESC_INITCONST(value), DOC(doc) }
#define Dee_TYPE_MEMBER_FIELD(name, type, offset) \
	Dee_TYPE_MEMBER_FIELD_DOC(name, type, offset, NULL)
#define Dee_TYPE_MEMBER_CONST(name, value) \
	Dee_TYPE_MEMBER_CONST_DOC(name, value, NULL)

#ifdef UINT64_C
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(mask, err)                                             \
	(((mask) == UINT8_C(0x01) || (mask) == UINT16_C(0x0100) ||                                    \
	  (mask) == UINT32_C(0x010000) || (mask) == UINT32_C(0x01000000) ||                           \
	  (mask) == UINT64_C(0x0100000000) || (mask) == UINT64_C(0x010000000000) ||                   \
	  (mask) == UINT64_C(0x01000000000000) || (mask) == UINT64_C(0x0100000000000000))             \
	 ? Dee_STRUCT_BOOLBIT0                                                                        \
	 : ((mask) == UINT8_C(0x02) || (mask) == UINT16_C(0x0200) ||                                  \
	    (mask) == UINT32_C(0x020000) || (mask) == UINT32_C(0x02000000) ||                         \
	    (mask) == UINT64_C(0x0200000000) || (mask) == UINT64_C(0x020000000000) ||                 \
	    (mask) == UINT64_C(0x02000000000000) || (mask) == UINT64_C(0x0200000000000000))           \
	   ? Dee_STRUCT_BOOLBIT1                                                                      \
	   : ((mask) == UINT8_C(0x04) || (mask) == UINT16_C(0x0400) ||                                \
	      (mask) == UINT32_C(0x040000) || (mask) == UINT32_C(0x04000000) ||                       \
	      (mask) == UINT64_C(0x0400000000) || (mask) == UINT64_C(0x040000000000) ||               \
	      (mask) == UINT64_C(0x04000000000000) || (mask) == UINT64_C(0x0400000000000000))         \
	     ? Dee_STRUCT_BOOLBIT2                                                                    \
	     : ((mask) == UINT8_C(0x08) || (mask) == UINT16_C(0x0800) ||                              \
	        (mask) == UINT32_C(0x080000) || (mask) == UINT32_C(0x08000000) ||                     \
	        (mask) == UINT64_C(0x0800000000) || (mask) == UINT64_C(0x080000000000) ||             \
	        (mask) == UINT64_C(0x08000000000000) || (mask) == UINT64_C(0x0800000000000000))       \
	       ? Dee_STRUCT_BOOLBIT3                                                                  \
	       : ((mask) == UINT8_C(0x10) || (mask) == UINT16_C(0x1000) ||                            \
	          (mask) == UINT32_C(0x100000) || (mask) == UINT32_C(0x10000000) ||                   \
	          (mask) == UINT64_C(0x1000000000) || (mask) == UINT64_C(0x100000000000) ||           \
	          (mask) == UINT64_C(0x10000000000000) || (mask) == UINT64_C(0x1000000000000000))     \
	         ? Dee_STRUCT_BOOLBIT4                                                                \
	         : ((mask) == UINT8_C(0x20) || (mask) == UINT16_C(0x2000) ||                          \
	            (mask) == UINT32_C(0x200000) || (mask) == UINT32_C(0x20000000) ||                 \
	            (mask) == UINT64_C(0x2000000000) || (mask) == UINT64_C(0x200000000000) ||         \
	            (mask) == UINT64_C(0x20000000000000) || (mask) == UINT64_C(0x2000000000000000))   \
	           ? Dee_STRUCT_BOOLBIT5                                                              \
	           : ((mask) == UINT8_C(0x40) || (mask) == UINT16_C(0x4000) ||                        \
	              (mask) == UINT32_C(0x400000) || (mask) == UINT32_C(0x40000000) ||               \
	              (mask) == UINT64_C(0x4000000000) || (mask) == UINT64_C(0x400000000000) ||       \
	              (mask) == UINT64_C(0x40000000000000) || (mask) == UINT64_C(0x4000000000000000)) \
	             ? Dee_STRUCT_BOOLBIT6                                                            \
	             : err)
#ifdef __INTELLISENSE__
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC_CHK(mask, err)                                   \
	_Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(                                                    \
	mask, ((mask) == UINT8_C(0x80) || (mask) == UINT16_C(0x8000) ||                        \
	       (mask) == UINT32_C(0x800000) || (mask) == UINT32_C(0x80000000) ||               \
	       (mask) == UINT64_C(0x8000000000) || (mask) == UINT64_C(0x800000000000) ||       \
	       (mask) == UINT64_C(0x80000000000000) || (mask) == UINT64_C(0x8000000000000000)) \
	      ? Dee_STRUCT_BOOLBIT7                                                            \
	      : err)
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask)                  \
	((int(*)[_Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC_CHK(mask, -1)])0, \
	 _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(mask, Dee_STRUCT_BOOLBIT7))
#else /* __INTELLISENSE__ */
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask) \
	_Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(mask, Dee_STRUCT_BOOLBIT7)
#endif /* !__INTELLISENSE__ */
#define _Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)   \
	((mask)&UINT8_C(0xff)                          \
	 ? 0                                           \
	 : (mask)&UINT16_C(0xff00)                     \
	   ? 1                                         \
	   : (mask)&UINT32_C(0xff0000)                 \
	     ? 2                                       \
	     : (mask)&UINT32_C(0xff000000)             \
	       ? 3                                     \
	       : (mask)&UINT64_C(0xff00000000)         \
	         ? 4                                   \
	         : (mask)&UINT64_C(0xff0000000000)     \
	           ? 5                                 \
	           : (mask)&UINT64_C(0xff000000000000) \
	             ? 6                               \
	             : 7)
#else /* UINT64_C */
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask)                                   \
	(((mask) == UINT8_C(0x01) || (mask) == UINT16_C(0x0100) ||                    \
	  (mask) == UINT32_C(0x010000) || (mask) == UINT32_C(0x01000000))             \
	 ? Dee_STRUCT_BOOLBIT0                                                        \
	 : ((mask) == UINT8_C(0x02) || (mask) == UINT16_C(0x0200) ||                  \
	    (mask) == UINT32_C(0x020000) || (mask) == UINT32_C(0x02000000))           \
	   ? Dee_STRUCT_BOOLBIT1                                                      \
	   : ((mask) == UINT8_C(0x04) || (mask) == UINT16_C(0x0400) ||                \
	      (mask) == UINT32_C(0x040000) || (mask) == UINT32_C(0x04000000))         \
	     ? Dee_STRUCT_BOOLBIT2                                                    \
	     : ((mask) == UINT8_C(0x08) || (mask) == UINT16_C(0x0800) ||              \
	        (mask) == UINT32_C(0x080000) || (mask) == UINT32_C(0x08000000))       \
	       ? Dee_STRUCT_BOOLBIT3                                                  \
	       : ((mask) == UINT8_C(0x10) || (mask) == UINT16_C(0x1000) ||            \
	          (mask) == UINT32_C(0x100000) || (mask) == UINT32_C(0x10000000))     \
	         ? Dee_STRUCT_BOOLBIT4                                                \
	         : ((mask) == UINT8_C(0x20) || (mask) == UINT16_C(0x2000) ||          \
	            (mask) == UINT32_C(0x200000) || (mask) == UINT32_C(0x20000000))   \
	           ? Dee_STRUCT_BOOLBIT5                                              \
	           : ((mask) == UINT8_C(0x40) || (mask) == UINT16_C(0x4000) ||        \
	              (mask) == UINT32_C(0x400000) || (mask) == UINT32_C(0x40000000)) \
	             ? Dee_STRUCT_BOOLBIT6                                            \
	             : Dee_STRUCT_BOOLBIT7)
#define _Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask) \
	((mask)&UINT8_C(0xff)                        \
	 ? 0                                         \
	 : (mask)&UINT16_C(0xff00)                   \
	   ? 1                                       \
	   : (mask)&UINT32_C(0xff0000)               \
	     ? 2                                     \
	     : 3)
#endif /* !UINT64_C */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, mask) \
	offsetof(struct_type, field) + _Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, mask)    \
	offsetof(struct_type, field) + (sizeof(((struct_type *)0)->field) - \
	                                (1 + _Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)))
#else /* __BYTE_ORDER__ == ... */
#error "Unsupported endian"
#endif /* __BYTE_ORDER__ != ... */
#define Dee_TYPE_MEMBER_BITFIELD_DOC(name, flags, struct_type, field, flagmask, doc)            \
	Dee_TYPE_MEMBER_FIELD_DOC(name, (flags) | _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(flagmask),      \
	                          _Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, flagmask), \
	                          doc)
#define Dee_TYPE_MEMBER_BITFIELD(name, flags, struct_type, field, flagmask) \
	Dee_TYPE_MEMBER_BITFIELD_DOC(name, flags, struct_type, field, flagmask, NULL)

#ifdef DEE_SOURCE
#define TYPE_MEMBER_ISCONST      Dee_TYPE_MEMBER_ISCONST
#define TYPE_MEMBER_ISFIELD      Dee_TYPE_MEMBER_ISFIELD
#define TYPE_MEMBER_END          Dee_TYPE_MEMBER_END
#define TYPE_MEMBER_FIELD_DOC    Dee_TYPE_MEMBER_FIELD_DOC
#define TYPE_MEMBER_BITFIELD_DOC Dee_TYPE_MEMBER_BITFIELD_DOC
#define TYPE_MEMBER_CONST_DOC    Dee_TYPE_MEMBER_CONST_DOC
#define TYPE_MEMBER_FIELD        Dee_TYPE_MEMBER_FIELD
#define TYPE_MEMBER_BITFIELD     Dee_TYPE_MEMBER_BITFIELD
#define TYPE_MEMBER_CONST        Dee_TYPE_MEMBER_CONST
#endif /* DEE_SOURCE */


struct Dee_membercache_table;
struct Dee_membercache {
	struct {
		struct Dee_membercache *le_next, **le_prev;
	}                                  mc_link;   /* [0..1][lock(INTERNAL(membercache_lock))]
	                                               * Entry in global list of caches (or unbound if not yet
	                                               * linked, in which case `mc_table == NULL') */
	DREF struct Dee_membercache_table *mc_table;  /* [0..1][lock(mc_tabuse > 0)] Member cache table. */
#ifndef CONFIG_NO_THREADS
	size_t                             mc_tabuse; /* [lock(ATOMIC)] When non-zero, some thread is reading `mc_table'. */
#endif /* !CONFIG_NO_THREADS */
};



#ifndef Dee_operator_t_DEFINED
#define Dee_operator_t_DEFINED
typedef uint16_t Dee_operator_t;
#endif /* !Dee_operator_t_DEFINED */

#define Dee_OPERATOR_USERCOUNT    0x003e        /* Number of user-accessible operators. (Used by `class' types) */
#define Dee_OPERATOR_EXTENDED(x) (0x1000 + (x)) /* Extended operator codes. (Type-specific; may be re-used) */
#ifdef DEE_SOURCE
/* Universal operator codes. */
#define OPERATOR_CONSTRUCTOR  0x0000 /* `operator this(args...)'                                    - `__constructor__' - `tp_any_ctor'. */
#define OPERATOR_COPY         0x0001 /* `operator copy(other: type(this))'                          - `__copy__'        - `tp_copy_ctor'. */
#define OPERATOR_SERIALIZE    0x0002 /* `operator serialize(TODO)'                                  - `__serialize__'   - `tp_serialize'. */
#define OPERATOR_DESTRUCTOR   0x0003 /* `operator ~this()'                                          - `__destructor__'  - `tp_dtor'. */
#define OPERATOR_ASSIGN       0x0004 /* `operator := (other: Object)'                               - `__assign__'      - `tp_assign'. */
#define OPERATOR_MOVEASSIGN   0x0005 /* `operator move := (other: type(this))'                      - `__moveassign__'  - `tp_move_assign'. */
#define OPERATOR_STR          0x0006 /* `operator str(): string'                                    - `__str__'         - `tp_str'. */
#define OPERATOR_REPR         0x0007 /* `operator repr(): string'                                   - `__repr__'        - `tp_repr'. */
#define OPERATOR_BOOL         0x0008 /* `operator bool(): bool'                                     - `__bool__'        - `tp_bool'. */
#define OPERATOR_ITERNEXT     0x0009 /* `operator next(): Object'                                   - `__next__'        - `tp_iter_next'. */
#define OPERATOR_CALL         0x000a /* `operator ()(args...): Object'                              - `__call__'        - `tp_call'. */
#define OPERATOR_INT          0x000b /* `operator int(): int'                                       - `__int__'         - `tp_int'. */
#define OPERATOR_FLOAT        0x000c /* `operator float(): float'                                   - `__float__'       - `tp_double'. */
#define OPERATOR_INV          0x000d /* `operator ~ (): Object'                                     - `__inv__'         - `tp_inv'. */
#define OPERATOR_POS          0x000e /* `operator + (): Object'                                     - `__pos__'         - `tp_pos'. */
#define OPERATOR_NEG          0x000f /* `operator - (): Object'                                     - `__neg__'         - `tp_neg'. */
#define OPERATOR_ADD          0x0010 /* `operator + (other: Object): Object'                        - `__add__'         - `tp_add'. */
#define OPERATOR_SUB          0x0011 /* `operator - (other: Object): Object'                        - `__sub__'         - `tp_sub'. */
#define OPERATOR_MUL          0x0012 /* `operator * (other: Object): Object'                        - `__mul__'         - `tp_mul'. */
#define OPERATOR_DIV          0x0013 /* `operator / (other: Object): Object'                        - `__div__'         - `tp_div'. */
#define OPERATOR_MOD          0x0014 /* `operator % (other: Object): Object'                        - `__mod__'         - `tp_mod'. */
#define OPERATOR_SHL          0x0015 /* `operator << (other: Object): Object'                       - `__shl__'         - `tp_shl'. */
#define OPERATOR_SHR          0x0016 /* `operator >> (other: Object): Object'                       - `__shr__'         - `tp_shr'. */
#define OPERATOR_AND          0x0017 /* `operator & (other: Object): Object'                        - `__and__'         - `tp_and'. */
#define OPERATOR_OR           0x0018 /* `operator | (other: Object): Object'                        - `__or__'          - `tp_or'. */
#define OPERATOR_XOR          0x0019 /* `operator ^ (other: Object): Object'                        - `__xor__'         - `tp_xor'. */
#define OPERATOR_POW          0x001a /* `operator ** (other: Object): Object'                       - `__pow__'         - `tp_pow'. */
#define OPERATOR_INC          0x001b /* `operator ++ (): type(this)'                                - `__inc__'         - `tp_inc'. */
#define OPERATOR_DEC          0x001c /* `operator -- (): type(this)'                                - `__dec__'         - `tp_dec'. */
#define OPERATOR_INPLACE_ADD  0x001d /* `operator += (other: Object): type(this)'                   - `__iadd__'        - `tp_inplace_add'. */
#define OPERATOR_INPLACE_SUB  0x001e /* `operator -= (other: Object): type(this)'                   - `__isub__'        - `tp_inplace_sub'. */
#define OPERATOR_INPLACE_MUL  0x001f /* `operator *= (other: Object): type(this)'                   - `__imul__'        - `tp_inplace_mul'. */
#define OPERATOR_INPLACE_DIV  0x0020 /* `operator /= (other: Object): type(this)'                   - `__idiv__'        - `tp_inplace_div'. */
#define OPERATOR_INPLACE_MOD  0x0021 /* `operator %= (other: Object): type(this)'                   - `__imod__'        - `tp_inplace_mod'. */
#define OPERATOR_INPLACE_SHL  0x0022 /* `operator <<= (other: Object): type(this)'                  - `__ishl__'        - `tp_inplace_shl'. */
#define OPERATOR_INPLACE_SHR  0x0023 /* `operator >>= (other: Object): type(this)'                  - `__ishr__'        - `tp_inplace_shr'. */
#define OPERATOR_INPLACE_AND  0x0024 /* `operator &= (other: Object): type(this)'                   - `__iand__'        - `tp_inplace_and'. */
#define OPERATOR_INPLACE_OR   0x0025 /* `operator |= (other: Object): type(this)'                   - `__ior__'         - `tp_inplace_or'. */
#define OPERATOR_INPLACE_XOR  0x0026 /* `operator ^= (other: Object): type(this)'                   - `__ixor__'        - `tp_inplace_xor'. */
#define OPERATOR_INPLACE_POW  0x0027 /* `operator **= (other: Object): type(this)'                  - `__ipow__'        - `tp_inplace_pow'. */
#define OPERATOR_HASH         0x0028 /* `operator hash(): int'                                      - `__hash__'        - `tp_hash'. */
#define OPERATOR_EQ           0x0029 /* `operator == (other: Object): Object'                       - `__eq__'          - `tp_eq'. */
#define OPERATOR_NE           0x002a /* `operator != (other: Object): Object'                       - `__ne__'          - `tp_ne'. */
#define OPERATOR_LO           0x002b /* `operator < (other: Object): Object'                        - `__lo__'          - `tp_lo'. */
#define OPERATOR_LE           0x002c /* `operator <= (other: Object): Object'                       - `__le__'          - `tp_le'. */
#define OPERATOR_GR           0x002d /* `operator > (other: Object): Object'                        - `__gr__'          - `tp_gr'. */
#define OPERATOR_GE           0x002e /* `operator >= (other: Object): Object'                       - `__ge__'          - `tp_ge'. */
#define OPERATOR_ITER         0x002f /* `operator iter(): Object'                                   - `__iter__'        - `tp_iter'. */
#define OPERATOR_SIZE         0x0030 /* `operator # (): Object'                                     - `__size__'        - `tp_sizeob'. */
#define OPERATOR_CONTAINS     0x0031 /* `operator contains(other: Object): Object'                  - `__contains__'    - `tp_contains'. */
#define OPERATOR_GETITEM      0x0032 /* `operator [] (index: Object): Object'                       - `__getitem__'     - `tp_getitem'. */
#define OPERATOR_DELITEM      0x0033 /* `operator del[] (index: Object)'                            - `__delitem__'     - `tp_delitem'. */
#define OPERATOR_SETITEM      0x0034 /* `operator []= (index: Object, value: Object)'               - `__setitem__'     - `tp_setitem'. */
#define OPERATOR_GETRANGE     0x0035 /* `operator [:] (begin: Object, end: Object): Object'         - `__getrange__'    - `tp_getrange'. */
#define OPERATOR_DELRANGE     0x0036 /* `operator del[:] (begin: Object, end: Object)'              - `__delrange__'    - `tp_delrange'. */
#define OPERATOR_SETRANGE     0x0037 /* `operator [:]= (begin: Object, end: Object, value: Object)' - `__setrange__'    - `tp_setrange'. */
#define OPERATOR_GETATTR      0x0038 /* `operator . (string attr): Object'                          - `__getattr__'     - `tp_getattr'. */
#define OPERATOR_DELATTR      0x0039 /* `operator del. (string attr)'                               - `__delattr__'     - `tp_delattr'. */
#define OPERATOR_SETATTR      0x003a /* `operator .= (string attr, value: Object)'                  - `__setattr__'     - `tp_setattr'. */
#define OPERATOR_ENUMATTR     0x003b /* `operator enumattr(): {attribute...}'                       - `__enumattr__'    - `tp_iterattr'. */
#define OPERATOR_ENTER        0x003c /* `operator enter()'                                          - `__enter__'       - `tp_enter'. */
#define OPERATOR_LEAVE        0x003d /* `operator leave()'                                          - `__leave__'       - `tp_leave'. */
#define OPERATOR_USERCOUNT    0x003e /* Number of user-accessible operators. (Used by `class' types) */
#define OPERATOR_EXTENDED(x)  (0x1000 + (x)) /* Extended operator codes. (Type-specific; may be re-used) */
#define OPERATOR_ISINPLACE(x) ((x) >= OPERATOR_INC && (x) <= OPERATOR_INPLACE_POW)

/* Operators not exposed to user-code. */
#define OPERATOR_GETBUF       0x8000 /* `tp_getbuf'. */
#ifndef CONFIG_EXPERIMENTAL_REWORKED_GC
#define OPERATOR_VISIT        0x8001 /* `tp_visit'. */
#define OPERATOR_CLEAR        0x8002 /* `tp_clear'. */
#define OPERATOR_PCLEAR       0x8003 /* `tp_pclear'. */
#endif /* CONFIG_EXPERIMENTAL_REWORKED_GC */

/* Fake operators (for use with `DeeFormat_PrintOperatorRepr()'). */
#define FAKE_OPERATOR_IS          0xff00 /* `a is b' */
#define FAKE_OPERATOR_SAME_OBJECT 0xff01 /* `a === b' */
#define FAKE_OPERATOR_DIFF_OBJECT 0xff02 /* `a !== b' */
#define FAKE_OPERATOR_NOT         0xff03 /* `!a' */

/* Operator association ranges. */
#define OPERATOR_PRIVMIN    OPERATOR_GETBUF
#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
#define OPERATOR_PRIVMAX    OPERATOR_GETBUF
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
#define OPERATOR_PRIVMAX    OPERATOR_PCLEAR
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */

/* Aliases that should be used when operators need to appear sorted by ID.
 *
 * By using these, you can ensure a proper operator order by simply doing
 * a lexicographical line sort. */
#define OPERATOR_0000_CONSTRUCTOR  OPERATOR_CONSTRUCTOR
#define OPERATOR_0001_COPY         OPERATOR_COPY
#define OPERATOR_0002_SERIALIZE    OPERATOR_SERIALIZE
#define OPERATOR_0003_DESTRUCTOR   OPERATOR_DESTRUCTOR
#define OPERATOR_0004_ASSIGN       OPERATOR_ASSIGN
#define OPERATOR_0005_MOVEASSIGN   OPERATOR_MOVEASSIGN
#define OPERATOR_0006_STR          OPERATOR_STR
#define OPERATOR_0007_REPR         OPERATOR_REPR
#define OPERATOR_0008_BOOL         OPERATOR_BOOL
#define OPERATOR_0009_ITERNEXT     OPERATOR_ITERNEXT
#define OPERATOR_000A_CALL         OPERATOR_CALL
#define OPERATOR_000B_INT          OPERATOR_INT
#define OPERATOR_000C_FLOAT        OPERATOR_FLOAT
#define OPERATOR_000D_INV          OPERATOR_INV
#define OPERATOR_000E_POS          OPERATOR_POS
#define OPERATOR_000F_NEG          OPERATOR_NEG
#define OPERATOR_0010_ADD          OPERATOR_ADD
#define OPERATOR_0011_SUB          OPERATOR_SUB
#define OPERATOR_0012_MUL          OPERATOR_MUL
#define OPERATOR_0013_DIV          OPERATOR_DIV
#define OPERATOR_0014_MOD          OPERATOR_MOD
#define OPERATOR_0015_SHL          OPERATOR_SHL
#define OPERATOR_0016_SHR          OPERATOR_SHR
#define OPERATOR_0017_AND          OPERATOR_AND
#define OPERATOR_0018_OR           OPERATOR_OR
#define OPERATOR_0019_XOR          OPERATOR_XOR
#define OPERATOR_001A_POW          OPERATOR_POW
#define OPERATOR_001B_INC          OPERATOR_INC
#define OPERATOR_001C_DEC          OPERATOR_DEC
#define OPERATOR_001D_INPLACE_ADD  OPERATOR_INPLACE_ADD
#define OPERATOR_001E_INPLACE_SUB  OPERATOR_INPLACE_SUB
#define OPERATOR_001F_INPLACE_MUL  OPERATOR_INPLACE_MUL
#define OPERATOR_0020_INPLACE_DIV  OPERATOR_INPLACE_DIV
#define OPERATOR_0021_INPLACE_MOD  OPERATOR_INPLACE_MOD
#define OPERATOR_0022_INPLACE_SHL  OPERATOR_INPLACE_SHL
#define OPERATOR_0023_INPLACE_SHR  OPERATOR_INPLACE_SHR
#define OPERATOR_0024_INPLACE_AND  OPERATOR_INPLACE_AND
#define OPERATOR_0025_INPLACE_OR   OPERATOR_INPLACE_OR
#define OPERATOR_0026_INPLACE_XOR  OPERATOR_INPLACE_XOR
#define OPERATOR_0027_INPLACE_POW  OPERATOR_INPLACE_POW
#define OPERATOR_0028_HASH         OPERATOR_HASH
#define OPERATOR_0029_EQ           OPERATOR_EQ
#define OPERATOR_002A_NE           OPERATOR_NE
#define OPERATOR_002B_LO           OPERATOR_LO
#define OPERATOR_002C_LE           OPERATOR_LE
#define OPERATOR_002D_GR           OPERATOR_GR
#define OPERATOR_002E_GE           OPERATOR_GE
#define OPERATOR_002F_ITER         OPERATOR_ITER
#define OPERATOR_0030_SIZE         OPERATOR_SIZE
#define OPERATOR_0031_CONTAINS     OPERATOR_CONTAINS
#define OPERATOR_0032_GETITEM      OPERATOR_GETITEM
#define OPERATOR_0033_DELITEM      OPERATOR_DELITEM
#define OPERATOR_0034_SETITEM      OPERATOR_SETITEM
#define OPERATOR_0035_GETRANGE     OPERATOR_GETRANGE
#define OPERATOR_0036_DELRANGE     OPERATOR_DELRANGE
#define OPERATOR_0037_SETRANGE     OPERATOR_SETRANGE
#define OPERATOR_0038_GETATTR      OPERATOR_GETATTR
#define OPERATOR_0039_DELATTR      OPERATOR_DELATTR
#define OPERATOR_003A_SETATTR      OPERATOR_SETATTR
#define OPERATOR_003B_ENUMATTR     OPERATOR_ENUMATTR
#define OPERATOR_003C_ENTER        OPERATOR_ENTER
#define OPERATOR_003D_LEAVE        OPERATOR_LEAVE
#define OPERATOR_8000_GETBUF       OPERATOR_GETBUF
#ifndef CONFIG_EXPERIMENTAL_REWORKED_GC
#define OPERATOR_8001_VISIT        OPERATOR_VISIT
#define OPERATOR_8002_CLEAR        OPERATOR_CLEAR
#define OPERATOR_8003_PCLEAR       OPERATOR_PCLEAR
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
#endif /* DEE_SOURCE */



#ifdef DEE_SOURCE
/* Operator calling conventions (values for `oi_cc')
 * These are completely optional and only serve to aid certain sub-systems in making
 * special optimizations (such as `_hostasm' having an easier time inlining operator
 * calls) */
#define OPCC_SPECIAL            0x0000 /* A special operator that cannot be invoked directly (e.g.: `OPERATOR_CONSTRUCTOR'). */
#define OPCC_FINPLACE           0x8000 /* Flag: this operator must be invoked as inplace. */
#define OPCC_UNARY_OBJECT       0x0010 /* DREF DeeObject *(DCALL *)(DeeObject *__restrict self); */
#define OPCC_UNARY_VOID         0x0011 /* void (DCALL *)(DeeObject *__restrict self); */
#define OPCC_UNARY_INT          0x0012 /* int (DCALL *)(DeeObject *__restrict self); */
#define OPCC_UNARY_INPLACE      0x8013 /* int (DCALL *)(DREF DeeObject **__restrict p_self); */
#define OPCC_UNARY_UINTPTR_NX   0x0014 /* uintptr_t (DCALL *)(DeeObject *__restrict self); */
#define OPCC_BINARY_OBJECT      0x0020 /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *other); */
#define OPCC_BINARY_VOID        0x0021 /* void (DCALL *)(DeeObject *self, DeeObject *other); */
#define OPCC_BINARY_INT         0x0022 /* int (DCALL *)(DeeObject *self, DeeObject *other); */
#define OPCC_BINARY_INPLACE     0x8023 /* int (DCALL *)(DREF DeeObject **__restrict p_self, DeeObject *other); */
#define OPCC_TRINARY_OBJECT     0x0030 /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *a, DeeObject *b); */
#define OPCC_TRINARY_VOID       0x0031 /* void (DCALL *)(DeeObject *self, DeeObject *a, DeeObject *b); */
#define OPCC_TRINARY_INT        0x0032 /* int (DCALL *)(DeeObject *self, DeeObject *a, DeeObject *b); */
#define OPCC_TRINARY_INPLACE    0x8033 /* int (DCALL *)(DREF DeeObject **__restrict p_self, DeeObject *a, DeeObject *b); */
#define OPCC_QUATERNARY_OBJECT  0x0040 /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *a, DeeObject *b, DeeObject *c); */
#define OPCC_QUATERNARY_VOID    0x0041 /* void (DCALL *)(DeeObject *self, DeeObject *a, DeeObject *b, DeeObject *c); */
#define OPCC_QUATERNARY_INT     0x0042 /* int (DCALL *)(DeeObject *self, DeeObject *a, DeeObject *b, DeeObject *c); */
#define OPCC_QUATERNARY_INPLACE 0x8043 /* int (DCALL *)(DREF DeeObject **__restrict p_self, DeeObject *a, DeeObject *b, DeeObject *c); */
#define OPCC_ARGC(x) (((x) & 0xf0) >> 4)

/* Operator classes (values for `oi_class') */
#define OPCLASS_TYPE    0x0      /* `oi_offset' points into `DeeTypeObject'. */
#define OPCLASS(offset) (offset) /* `oi_offset' points into a [0..1] struct at this offset */
#define OPCLASS_CUSTOM  0xffff   /* Custom operator (never appears in `Dee_opinfo') */
#endif /* DEE_SOURCE */

struct Dee_opinfo;

/* Abstract/generic operator invocation wrapper.
 * NOTE: This callback should:
 * - Load the operator C function from `fun = *(*(tp_self + :oi_class) + :oi_offset) != NULL'
 * - Check if `fun' is the special `opi_classhook' function (in which case, invoke `DeeClass_GetOperator(tp_self, :oi_id)')
 * - Otherwise, unpack "argc" and "argv", and invoke `fun' with the loaded arguments.
 * @param: p_self: Self-pointer argument (non-NULL in case of an inplace invocation), or `NULL' (in case of a normal operator)
 * @return: * :   The operator result (in case of inplace operator, usually the same as was written back to `*p_self')
 * @return: NULL: An error was thrown. */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *
(DCALL *Dee_operator_invoke_cb_t)(DeeTypeObject *tp_self, DeeObject *self,
                                  /*0..1*/ DREF DeeObject **p_self, size_t argc,
                                  DeeObject *const *argv, Dee_operator_t opname);

/* Generic operator inheritance function.
 *
 * This function may be overwritten in order to control how/when a
 * specific operator "info->oi_id" (as declared by "type_type") can
 * be inherited by "self" from one of its direct bases, and is used
 * to implement `DeeType_InheritOperator()'.
 *
 * When not explicit operator inheritance callback is provided,
 * operators are inherited automatically by recursively checking
 * all direct bases of "self" that are able to provide operators
 * from "type_type", then recursively trying to have those types
 * inherit "info->oi_id" until one of them ends up implementing
 * it (or was already implementing it from the get-go), then
 * inheriting that base's callback into "self".
 *
 * This callback only gets invoked when the operator slot described
 * by "info" leads to a NULL-function-pointer in "self".
 *
 * Using this override, it's possible to make it so only certain
 * groups of operators can be inherited (e.g. "*" and "*=" can only
 * ever be inherited as a pair, rather than individually, meaning
 * that when a type defines one of them, the other can't be inherited
 * from a base class). */
typedef NONNULL_T((1, 2, 3)) void
(DCALL *Dee_operator_inherit_cb_t)(DeeTypeObject *self, DeeTypeObject *type_type,
                                   struct Dee_opinfo const *info);
#if defined(__INTELLISENSE__) && defined(__cplusplus)
/* Highlight usage errors in IDE */
extern "C++" {
namespace __intern {
Dee_operator_invoke_cb_t _Dee_RequiresOperatorInvokeCb(decltype(nullptr));
template<class _TReturn, class _TTpSelf, class _TSelf> Dee_operator_invoke_cb_t
_Dee_RequiresOperatorInvokeCb(_TReturn *(DCALL *_meth)(_TTpSelf *tp_self, _TSelf *self, /*0..1*/ _TSelf **p_self,
                                                       size_t argc, DeeObject *const *argv, Dee_operator_t opname));
Dee_operator_inherit_cb_t _Dee_RequiresOperatorInheritCb(decltype(nullptr));
template<class _TSelf, class _TTypeType> Dee_operator_inherit_cb_t
_Dee_RequiresOperatorInheritCb(void (DCALL *_meth)(_TSelf *self, _TTypeType *type_type,
                                                   struct Dee_opinfo const *info));
} /* namespace __intern */
} /* extern "C++" */
#define Dee_REQUIRES_OPERATOR_INVOKE_CB(meth)  ((decltype(::__intern::_Dee_RequiresOperatorInvokeCb(meth)))(meth))
#define Dee_REQUIRES_OPERATOR_INHERIT_CB(meth) ((decltype(::__intern::_Dee_RequiresOperatorInheritCb(meth)))(meth))
#else /* __INTELLISENSE__ && __cplusplus */
#define Dee_REQUIRES_OPERATOR_INVOKE_CB(meth)  ((Dee_operator_invoke_cb_t)(meth))
#define Dee_REQUIRES_OPERATOR_INHERIT_CB(meth) ((Dee_operator_inherit_cb_t)(meth))
#endif /* !__INTELLISENSE__ || !__cplusplus */

struct Dee_operator_invoke {
	Dee_operator_invoke_cb_t  opi_invoke;    /* [1..1] Called by `DeeObject_InvokeOperator()' when this operator is
	                                          * invoked. (Only called when `*(*(tp + oi_class) + oi_offset) != NULL'). */
	Dee_funptr_t              opi_classhook; /* [0..1] Function pointer to store in `*(*(tp + oi_class) + oi_offset)' when `DeeClass_New()'
	                                          * hooks this operator. The implementation of `opi_invoke' should check for this function and
	                                          * manually invoke its t* variant, which should then use `DeeClass_GetOperator()' in order to
	                                          * load the user-defined function object associated with this operator.
	                                          * When set to `NULL', the operator cannot be overwritten by user-code. */
	Dee_operator_inherit_cb_t opi_inherit;   /* [0..1] Override for inheriting this operator. */
};
#define Dee_OPERATOR_INVOKE_INIT(opi_invoke_, opi_classhook_, opi_inherit)          \
	{ Dee_REQUIRES_OPERATOR_INVOKE_CB(opi_invoke_), (Dee_funptr_t)(opi_classhook_), \
	  Dee_REQUIRES_OPERATOR_INHERIT_CB(opi_inherit) }

struct Dee_opinfo {
	/* TODO: This here needs to be re-designed */
	Dee_operator_t                          oi_id;        /* Operator ID */
	uint16_t                                oi_class;     /* Offset into the type for where to find a `0..1' struct that contains more  */
	uint16_t                                oi_offset;    /* Offset from `oi_class' to where the c-function of this operator can be found. */
	uint16_t                                oi_cc;        /* The operator calling convention (s.a. `OPCC_*'). */
	char                                    oi_uname[12]; /* `+' */
	char                                    oi_sname[12]; /* `add' */
	char                                    oi_iname[16]; /* `tp_add' */
	struct Dee_operator_invoke Dee_tpconst *oi_invoke;    /* [1..1] Generic info on how to invoke/hook this operator. */
};

#if defined(__INTELLISENSE__) && defined(__cplusplus)
extern "C++" {
namespace __intern {
struct Dee_opinfo const &_Dee_OPINFO_INIT(Dee_operator_t id, uint16_t class_, uint16_t offset, uint16_t cc,
                                          char const uname[12], char const sname[12], char const iname[16],
                                          struct Dee_operator_invoke Dee_tpconst *invoke);
} /* namespace __intern */
} /* extern "C++" */
#define Dee_OPINFO_INIT ::__intern::_Dee_OPINFO_INIT
#else /* __INTELLISENSE__ && __cplusplus */
#define Dee_OPINFO_INIT(id, class, offset, cc, uname, sname, iname, invoke) \
	{ id, class, offset, cc, uname, sname, iname, invoke }
#endif /* !__INTELLISENSE__ || !__cplusplus */
#define _Dee_PRIVATE_OPINFO_INIT_AS_CUSTOM(id, flags, invoke)                                                           \
	{ id, OPCLASS_CUSTOM, 0, OPCC_SPECIAL, { _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST((char)(unsigned char), flags) }, "", "", \
	  (struct Dee_operator_invoke Dee_tpconst *)(void Dee_tpconst *)(void const *)Dee_REQUIRES_OPERATOR_INVOKE_CB(invoke) }
#ifdef DEE_SOURCE
#define OPINFO_INIT Dee_OPINFO_INIT
#endif /* DEE_SOURCE */

#if __SIZEOF_POINTER__ == 4 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT32_C(0xff)), T(((v) & UINT32_C(0xff00)) >> 8), T(((v) & UINT32_C(0xff0000)) >> 16), T(((v) & UINT32_C(0xff000000)) >> 24)
#elif __SIZEOF_POINTER__ == 8 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT64_C(0xff)), T(((v) & UINT64_C(0xff00)) >> 8), T(((v) & UINT64_C(0xff0000)) >> 16), T(((v) & UINT64_C(0xff000000)) >> 24), T(((v) & UINT64_C(0xff00000000)) >> 32), T(((v) & UINT64_C(0xff0000000000)) >> 40), T(((v) & UINT64_C(0xff000000000000)) >> 48), T(((v) & UINT64_C(0xff00000000000000)) >> 56)
#elif __SIZEOF_POINTER__ == 4 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T(((v) & UINT32_C(0xff000000)) >> 24), T(((v) & UINT32_C(0xff0000)) >> 16), T(((v) & UINT32_C(0xff00)) >> 8), T((v) & UINT32_C(0xff))
#elif __SIZEOF_POINTER__ == 8 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T(((v) & UINT64_C(0xff00000000000000)) >> 56), T(((v) & UINT64_C(0xff000000000000)) >> 48), T(((v) & UINT64_C(0xff0000000000)) >> 40), T(((v) & UINT64_C(0xff00000000)) >> 32), T(((v) & UINT64_C(0xff000000)) >> 24), T(((v) & UINT64_C(0xff0000)) >> 16), T(((v) & UINT64_C(0xff00)) >> 8), T((v) & UINT64_C(0xff))
#elif __SIZEOF_POINTER__ == 2 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT16_C(0xff)), T(((v) & UINT16_C(0xff00)) >> 8)
#elif __SIZEOF_POINTER__ == 2 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T(((v) & UINT16_C(0xff00)) >> 8), T((v) & UINT16_C(0xff))
#elif __SIZEOF_POINTER__ == 1
#define _Dee_PRIVATE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT8_C(0xff))
#elif !defined(__DEEMON__)
#error "Unsupported __SIZEOF_POINTER__"
#endif /* !... */

struct Dee_type_operator {
	union {
		struct Dee_opinfo            to_decl;     /* [valid_if(Dee_type_operator_isdecl(.))] Operator declaration
		                                           * NOTE: Only allowed in `tp->tp_operators' if `DeeType_IsTypeType(tp)'! */
		struct {
			Dee_operator_t          _s_pad_id;    /* ... */
			uint16_t                _s_class;     /* Always `OPCLASS_CUSTOM' */
			uint16_t                _s_offset;    /* ... */
			uint16_t                _s_cc;        /* Always `OPCC_SPECIAL' */
			uintptr_t                s_flags;     /* Operator method flags (set of `Dee_METHOD_F*') */
			char                    _s_Xname[40 - sizeof(uintptr_t)]; /* ... */
			Dee_operator_invoke_cb_t s_invoke;    /* [0..1] Operator implementation (or `NULL' if only flags are declared) */
		}                            to_custom;   /* [valid_if(Dee_type_operator_iscustom(.))] Custom operator, or operator flags */
		Dee_operator_t               to_id;       /* Operator ID */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define to_decl  _dee_aunion.to_decl /*!export-*/
#define to_spec  _dee_aunion.to_spec /*!export-*/
#define to_id    _dee_aunion.to_id   /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define Dee_type_operator_iscustom(x) ((x)->to_custom._s_class == OPCLASS_CUSTOM)
#define Dee_type_operator_isdecl(x)   ((x)->to_custom._s_class != OPCLASS_CUSTOM)

#if defined(__INTELLISENSE__) && defined(__cplusplus)
#define Dee_TYPE_OPERATOR_DECL (struct Dee_type_operator const &)Dee_OPINFO_INIT
#else /* __INTELLISENSE__ && __cplusplus */
#define Dee_TYPE_OPERATOR_DECL(id, class, offset, cc, uname, sname, iname, invoke) \
	{ { Dee_OPINFO_INIT(id, class, offset, cc, uname, sname, iname, invoke) } }
#endif /* !__INTELLISENSE__ || !__cplusplus */
#define Dee_TYPE_OPERATOR_CUSTOM(id, invoke, flags) { { _Dee_PRIVATE_OPINFO_INIT_AS_CUSTOM(id, flags, invoke) } }
#define Dee_TYPE_OPERATOR_FLAGS(id, flags)          { { _Dee_PRIVATE_OPINFO_INIT_AS_CUSTOM(id, flags, NULL) } }
#ifdef DEE_SOURCE
#define type_operator_iscustom Dee_type_operator_iscustom
#define type_operator_isdecl   Dee_type_operator_isdecl
#define TYPE_OPERATOR_DECL     Dee_TYPE_OPERATOR_DECL
#define TYPE_OPERATOR_CUSTOM   Dee_TYPE_OPERATOR_CUSTOM
#define TYPE_OPERATOR_FLAGS    Dee_TYPE_OPERATOR_FLAGS
#endif /* DEE_SOURCE */





/* Type flags for `DeeTypeObject::tp_flags' */
#define Dee_TP_FNORMAL          0x0000 /* Normal type flags. */
#define Dee_TP_FFINAL           0x0001 /* The class cannot be sub-classed again. */
#define Dee_TP_FTRUNCATE        0x0002 /* Truncate values during integer conversion, rather than throwing an `OverflowError'. */
#define Dee_TP_FINTERRUPT       0x0004 /* This type is a so-called interrupt signal.
                                        * Instances of this type have special behavior when thrown as errors,
                                        * or when delivered to threads through use of `Thread.interrupt()'.
                                        * In such situations, the error can only be caught by exception handlers
                                        * specifically marked as `@[interrupt]' (or rather `Dee_EXCEPTION_HANDLER_FINTERPT')
                                        * Additionally (but only when `CONFIG_NO_THREADS' is disabled), such errors are
                                        * re-scheduled in the pending-interrupt system of the calling thread when they
                                        * are thrown in a context where errors are normally discarded (such as destructions
                                        * or secondary exceptions in user-code functions)
                                        * WARNING: This flag must be inherited explicitly by sub-classes! */
#define Dee_TP_FMOVEANY         0x0008 /* The type accepts any other object as second operator to `tp_move_assign' */
#define Dee_TP_FDEEPIMMUTABLE   0x0010 /* Instances of this type are deeply immutable, meaning "deepcopy INSTANCE" is allowed to just re-return "INSTANCE" */
/*      Dee_TP_F                0x0020  * ... */
#define Dee_TP_FINHERITCTOR     0x0040 /* This type inherits its constructor from `tp_base'.
                                        * Additionally, if no assign/move-assign operator is defined, those are inherited as well. */
#define Dee_TP_FABSTRACT        0x0080 /* Member functions and getsets of this type are type-generic and may
                                        * even be invoked when being passed objects that do not fulfill the
                                        * requirement of `DeeObject_InstanceOf(ob, self)' (where `self' is this type).
                                        * For example: Abstract base classes have this flag set, such as `Object', `Sequence' or `Iterator'
                                        * NOTE: This flag is not inherited.
                                        * When this flag is set, the type may also be used to construct super-wrappers
                                        * for any other kind of object, even if that object isn't derived from the type.
                                        * NOTE: When combined with `TF_SINGLETON' and `TP_FFINAL', class member functions
                                        *       may be called with invalid/broken "self" arguments, as it is assumed that
                                        *       the class only functions as a simple namespace (e.g. `deemon.gc').
                                        *       s.a.: `DeeType_IsNamespace()' */
/*      Dee_TP_F                0x0080  * ... */
/*      Dee_TP_F                0x0100  * ... */
/*      Dee_TP_F                0x0200  * ... */
#define Dee_TP_FNAMEOBJECT      0x0400 /* `tp_name' actually points to the `s_str' member of a `DeeStringObject' that this type holds a reference to. */
#define Dee_TP_FDOCOBJECT       0x0800 /* `tp_doc' actually points to the `s_str' member of a `DeeStringObject' that this type holds a reference to. */
#ifndef CONFIG_EXPERIMENTAL_REWORKED_GC
#define Dee_TP_FMAYREVIVE       0x1000 /* This type's `tp_dtor' may revive the object (return with the object's `ob_refcnt > 0')
                                        * Note that when reviving an object, `tp_dtor' must (in addition to an external reference)
                                        * gift the caller one further reference to the object that got revived (meaning that upon
                                        * return, `ob_refcnt >= 2', where at least 1 reference is caused by some global reference,
                                        * and 1 will be inherited by `DeeObject_Destroy()'). This is needed to prevent a potential
                                        * race condition where a GC-object isn't being tracked during destruction. */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
#define Dee_TP_FGC              0x2000 /* Instance of this type can be harvested by the Garbage Collector. */
#define Dee_TP_FHEAP            0x4000 /* This type was allocated on the heap. */
#define Dee_TP_FVARIABLE        0x8000 /* Variable-length object type. (`tp_var' is used, rather than `tp_alloc') */
#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
#define Dee_TP_FINTERHITABLE    (Dee_TP_FINTERRUPT | Dee_TP_FGC | Dee_TP_FVARIABLE) \
                                       /* Set of special flags that are inherited by sub-classes. */
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
#define Dee_TP_FINTERHITABLE    (Dee_TP_FINTERRUPT | Dee_TP_FMAYREVIVE | Dee_TP_FGC | Dee_TP_FVARIABLE) \
                                       /* Set of special flags that are inherited by sub-classes. */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */

#define Dee_TF_NONE             0x00000000 /* No special features. */
#define Dee_TF_NONLOOPING       0x00000001 /* The object's tp_visit function can be skipped when searching for conventional reference loops.
                                            * This flag may be set when it is known that an object will only ever point to other objects
                                            * that either don't point to any objects, or are guarantied to never point back.
                                            * An example for where this flag should be used would be an object that only ever
                                            * holds references to `String' or `int' objects, but not to objects of its own type,
                                            * or any sort of container object capable of holding instances of the same type. */
#define Dee_TF_KW               0x00000002 /* Instances of this type can be used as keyword argument objects (s.a. `DeeType_IsKw()')
                                            * WARNING: If you set this flag, you must also implement support in `DeeKw_Get*' */
#ifdef CONFIG_EXPERIMENTAL_TPVISIT_ALSO_AFFECTS_CLEAR
#define Dee_TF_TPVISIT          0x00000004 /* Adds an extra `DeeTypeObject *tp_self' argument o "tp_visit" and "tp_cleaar":
                                            * >> void (DCALL *tp_visit)(DeeTypeObject *tp_self, DeeObject *self, Dee_visit_t proc, void *arg);
                                            * >> void (DCALL *tp_clear)(DeeTypeObject *tp_self, DeeObject *self); */
#else /* CONFIG_EXPERIMENTAL_TPVISIT_ALSO_AFFECTS_CLEAR */
#define Dee_TF_TPVISIT          0x00000004 /* "tp_visit" is actually typed as "void (DCALL *tp_visit)(DeeTypeObject *tp_self, DeeObject *self, Dee_visit_t proc, void *arg)" */
#endif /* !CONFIG_EXPERIMENTAL_TPVISIT_ALSO_AFFECTS_CLEAR */
#define Dee_TF_SEQCLASS_SHFT    26         /* [INTERNAL] Shift for `Dee_TF_SEQCLASS_MASK' */
#define Dee_TF_SEQCLASS_MASK    0x1c000000 /* [INTERNAL] Mask for cached `Dee_SEQCLASS_*' */
#define Dee_TF_NOTCONSTCASTABLE 0x20000000 /* [INTERNAL] Cached result for `DeeType_IsConstCastable': false */
#define Dee_TF_ISCONSTCASTABLE  0x40000000 /* [INTERNAL] Cached result for `DeeType_IsConstCastable': true */
#define Dee_TF_SINGLETON        0x80000000 /* This type is a singleton. */

#ifdef DEE_SOURCE
#define TP_FNORMAL          Dee_TP_FNORMAL
#define TP_FFINAL           Dee_TP_FFINAL
#define TP_FTRUNCATE        Dee_TP_FTRUNCATE
#define TP_FINTERRUPT       Dee_TP_FINTERRUPT
#define TP_FMOVEANY         Dee_TP_FMOVEANY
#define TP_FDEEPIMMUTABLE   Dee_TP_FDEEPIMMUTABLE
#define TP_FINHERITCTOR     Dee_TP_FINHERITCTOR
#define TP_FABSTRACT        Dee_TP_FABSTRACT
#define TP_FNAMEOBJECT      Dee_TP_FNAMEOBJECT
#define TP_FDOCOBJECT       Dee_TP_FDOCOBJECT
#ifndef CONFIG_EXPERIMENTAL_REWORKED_GC
#define TP_FMAYREVIVE       Dee_TP_FMAYREVIVE
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
#define TP_FVARIABLE        Dee_TP_FVARIABLE
#define TP_FGC              Dee_TP_FGC
#define TP_FHEAP            Dee_TP_FHEAP
#define TP_FINTERHITABLE    Dee_TP_FINTERHITABLE
#define TF_NONE             Dee_TF_NONE
#define TF_NONLOOPING       Dee_TF_NONLOOPING
#define TF_KW               Dee_TF_KW
#define TF_TPVISIT          Dee_TF_TPVISIT
#define TF_SINGLETON        Dee_TF_SINGLETON
#endif /* DEE_SOURCE */

#ifndef Dee_visit_t_DEFINED
#define Dee_visit_t_DEFINED /*!export-*/
typedef NONNULL_T((1)) void (DCALL *Dee_visit_t)(DeeObject *__restrict self, void *arg);
#endif /* !Dee_visit_t_DEFINED */

struct Dee_class_desc;
struct Dee_type_method_hint;
struct Dee_type_mh_cache;
struct Dee_type_object {
	Dee_OBJECT_HEAD
#ifdef __INTELLISENSE__
		;
#endif /* __INTELLISENSE__ */
	char const                         *tp_name;     /* [0..1] Name of this type. */
	/*utf-8*/ char const               *tp_doc;      /* [0..1] Documentation string of this type and its operators. */
	uint16_t                            tp_flags;    /* Type flags (Set of `TP_F*'). */
	uint16_t                            tp_weakrefs; /* Offset to `offsetof(Dee?Object, ob_weakrefs)', or 0 when not supported.
	                                                  * NOTE: Must be explicitly inherited by derived types.
	                                                  * NOTE: This member must explicitly be initialized during object construction
	                                                  *       using `Dee_weakref_support_init' and `Dee_weakref_support_fini', during destruction. */
	uint32_t                            tp_features; /* Type sub-class specific features (Set of `TF_*'). */
	DREF DeeTypeObject                 *tp_base;     /* [0..1][const] Base class.
	                                                  * NOTE: When the `TP_FINHERITCTOR' flag is set, then this field must be non-NULL. */
	struct Dee_type_constructor         tp_init;     /* Constructor/destructor operators. */
	struct Dee_type_cast                tp_cast;     /* Type casting operators. */
	/* WARNINGS:
	 * - When "Dee_TF_TPVISIT" is set, "tp_visit" is actually typed as:
	 *   >> void (DCALL *tp_visit)(DeeTypeObject *tp_self, DeeObject *self, Dee_visit_t proc, void *arg);
	 * - "tp_visit" may be invoked while "self->ob_refcnt == 0"! 
	 * - "tp_visit" must never throw errors or invoke user-code in any form
	 * - "tp_visit" can skip visiting references that can never lead to GC objects:
	 *   - DeeString_Type
	 *   - DeeInt_Type
	 *   - DeeBool_Type
	 *   - ... anything that can't be used as a container for potential GC objects,
	 *     ... or could be a GC object itself
	 */
	NONNULL_T((1, 2)) void      (DCALL *tp_visit)(DeeObject *__restrict self, Dee_visit_t proc, void *arg); /* Visit all reachable, referenced (DREF) objected. */
	/* NOTE: Anything used by `DeeType_Inherit*' can't be made `Dee_tpconst' here! */
	struct Dee_type_gc Dee_tpconst     *tp_gc;       /* [0..1] GC related operators. */
	struct Dee_type_math               *tp_math;     /* [0..1][owned_if(tp_class != NULL)] Math related operators. */
	struct Dee_type_cmp                *tp_cmp;      /* [0..1][owned_if(tp_class != NULL)] Compare operators. */
	struct Dee_type_seq                *tp_seq;      /* [0..1][owned_if(tp_class != NULL)] Sequence operators. */
	WUNUSED_T NONNULL_T((1))
	DREF DeeObject             *(DCALL *tp_iter_next)(DeeObject *__restrict self);
	/* TODO: Get rid of "struct Dee_type_iterator *tp_iterator" -- all functionality should be implemented using method hints! */
	struct Dee_type_iterator           *tp_iterator; /* [0..1][owned_if(tp_class != NULL)] Extra iterator operators (all of these are optional; only `tp_iter_next' is required) */
	struct Dee_type_attr               *tp_attr;     /* [0..1][owned_if(tp_class != NULL)] Attribute access operators. */
	struct Dee_type_with               *tp_with;     /* [0..1][owned_if(tp_class != NULL)] __enter__ / __leave__ operators. */
	struct Dee_type_buffer             *tp_buffer;   /* [0..1] Raw buffer interface. */
	/* NOTE: All of the following are sentinel-terminated vectors. */
	struct Dee_type_method Dee_tpconst *tp_methods;  /* [0..1] Instance methods. */
	struct Dee_type_getset Dee_tpconst *tp_getsets;  /* [0..1] Instance getsets. */
	struct Dee_type_member Dee_tpconst *tp_members;  /* [0..1] Instance member fields. */
	struct Dee_type_method Dee_tpconst *tp_class_methods; /* [0..1] Class methods. */
	struct Dee_type_getset Dee_tpconst *tp_class_getsets; /* [0..1] Class getsets. */
	struct Dee_type_member Dee_tpconst *tp_class_members; /* [0..1] Class members (usually constants). */
	struct Dee_type_method_hint Dee_tpconst *tp_method_hints; /* [0..1] Instance method hints (referenced by `tp_methods'; see `<deemon/method-hints.h>') */
	WUNUSED_T ATTR_INS_T(3, 2) NONNULL_T((1))
	DREF DeeObject             *(DCALL *tp_call)(DeeObject *self, size_t argc, DeeObject *const *argv);
	struct Dee_type_callable           *tp_callable; /* [0..1][owned_if(tp_class != NULL)] Sequence operators. */

	/* [1..1][0..n][owned] NULL-terminated MRO override for this type.
	 * - When NULL, MRO for this type is facilitated through `tp_base'
	 * - When non-NULL, MRO for this type is [<the type itself>, tp_mro[0], tp_mro[1], ...]
	 *   until the first NULL-element in `tp_mro' is reached. Note that for this purpose,
	 *   it is assumed that `tp_mro[0] != NULL', and that <the type itself> does not appear
	 *   within `tp_mro' a second time.
	 * In order to enumerate the MRO of a type, irregardless of that type having a custom
	 * `tp_mro' or not, you should use `DeeTypeMRO' and its helper API (its API is also
	 * used in order to facilitate MRO for stuff like `DeeObject_GetAttr()', as well as
	 * all other operators)
	 *
	 * NOTES:
	 *  - MRO bases that don't appear in `tp_base' must not define any
	 *    extra instance fields (i.e. have the `TP_FABSTRACT' flag set).
	 *    As such, these types essentially only act as interface definitions,
	 *    with the ability to define fixed functions and operators,
	 *    thought no actual instance members, and not as actual types
	 *    when it comes to instantiation.
	 *  - Constructors/Destructors of MRO bases are NOT invoked by default.
	 *    They are only invoked if sub-classed by a user-defined class type.
	 */
	DREF DeeTypeObject *Dee_tpconst *tp_mro;

	/* [0..tp_operators_size][SORT(to_id)][owned_if(tp_class != NULL)]
	 * Extra per-type operators, and operator info for type-types.
	 * IMPORTANT: This list must be `tp_operators_size' items long, and be sorted by `to_id'
	 * - DeeType_IsTypeType types can define their type-specific operators (including offsets) here
	 * - !DeeType_IsTypeType types can define per-type extra operators here (without offsets,
	 *   but instead the direct function pointers).
	 *
	 * This way, TypeType types (DeeType_Type, DeeFileType_Type, and DeeSType_Type) can define
	 * their new operators such that anything can invoke them, while normal types can define
	 * extra flags for the operators they implement.
	 *
	 * IMPORTANT: Sub-type-types are *NOT* allowed to re-define operator IDs that were already defined
	 *            by bases (e.g. when creating a new type-type, you're not allowed to re-defined operators
	 *            types already defined by DeeType_Type) */
	struct Dee_type_operator const *tp_operators;
	size_t tp_operators_size; /* Size of "tp_operators" in items. */

	/* [0..1][owned][lock(WRITE_ONCE)] Method hint cache. */
	struct Dee_type_mh_cache *tp_mhcache;

	/* Lazily-filled hash-table of instance members.
	 * >> The member vectors are great for static allocation, but walking
	 *    all of them each time a member is accessed is way too slow.
	 *    So instead, we cache members and sort them by first-visible on a per-type basis.
	 *    That way, we can greatly optimize the lookup time for already-known members. */
	struct Dee_membercache  tp_cache;
	struct Dee_membercache  tp_class_cache;
	struct Dee_class_desc  *tp_class;    /* [0..1][owned] Class descriptor (Usually points below this type object). */
	Dee_WEAKREF_SUPPORT                  /* Weak reference support. */
	struct Dee_weakref      tp_module;   /* [0..1] Weak reference to module that is declaring this type. */
	/* ... Extended type fields go here (e.g.: `DeeFileTypeObject') */
};
#define DeeType_IsFinal(x)               (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FFINAL)
#define DeeType_IsFinalOrVariable(x)     (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & (Dee_TP_FFINAL | Dee_TP_FVARIABLE))
#define DeeType_IsInterrupt(x)           (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FINTERRUPT)
#define DeeType_IsAbstract(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FABSTRACT)
#define DeeType_IsVariable(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FVARIABLE)
#define DeeType_IsGC(x)                  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FGC)
#define DeeType_IsClass(x)               (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_class != NULL)
#define DeeType_IsArithmetic(x)          (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_math != NULL)
#define DeeType_IsComparable(x)          (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_cmp != NULL)
#define DeeType_IsSequence(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_seq != NULL)
#define DeeType_IsIntTruncated(x)        (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FTRUNCATE)
#define DeeType_HasMoveAny(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FMOVEANY)
#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
#define DeeType_HasRevivingDestructor(x) (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_finalize != NULL)
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
#define DeeType_HasRevivingDestructor(x) (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FMAYREVIVE)
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
#define DeeType_IsDeepImmutable(x)       (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FDEEPIMMUTABLE)
#define DeeType_IsIterator(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_iter_next != NULL)
#define DeeType_IsTypeType(x)            DeeType_Extends(Dee_REQUIRES_OBJECT(DeeTypeObject const, x), &DeeType_Type)
#define DeeType_IsHeapType(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FHEAP) /* Custom types are those not pre-defined, but created dynamically. */
#define DeeType_IsSuperConstructible(x)  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FINHERITCTOR)
#define DeeType_IsNoArgConstructible(x)  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_alloc.tp_ctor != NULL)
#define DeeType_IsVarArgConstructible(x) (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_alloc.tp_any_ctor != NULL || ((DeeTypeObject const *)(x))->tp_init.tp_alloc.tp_any_ctor_kw != NULL)
#define DeeType_IsConstructible(x)       (DeeType_IsSuperConstructible(x) || DeeType_IsNoArgConstructible(x) || DeeType_IsVarArgConstructible(x))
#define DeeType_IsCopyable(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_alloc.tp_copy_ctor != NULL) /* TODO: What about inherited copyability? (Dee_TP_FINHERITCTOR) */
#define DeeType_IsNamespace(x)           ((Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & (Dee_TP_FFINAL | Dee_TP_FABSTRACT)) == (Dee_TP_FFINAL | Dee_TP_FABSTRACT) && (((DeeTypeObject const *)(x))->tp_features & Dee_TF_SINGLETON))
#define DeeType_Base(x)                  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_base)
#ifndef CONFIG_EXPERIMENTAL_REWORKED_GC
#define DeeType_GCPriority(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_gc ? ((DeeTypeObject const *)(x))->tp_gc->tp_gcprio : Dee_GC_PRIORITY_LATE)
#define DeeObject_GCPriority(x)          DeeType_GCPriority(Dee_TYPE(x))
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
#define DeeObject_IsInterrupt(x)         DeeType_IsInterrupt(Dee_TYPE(x))

#ifdef CONFIG_BUILDING_DEEMON
/* Returns the "tp_serialize" operator for "tp". If possible, inherit from base class. */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t (DCALL DeeType_GetTpSerialize)(DeeTypeObject *__restrict self);
#endif /* CONFIG_BUILDING_DEEMON */

/* Return a pointer to the optimized implementation of
 * object destruction called by `DeeObject_Destroy()' */
DFUNDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_destroy_t DCALL
DeeType_RequireDestroy(DeeTypeObject *__restrict self);


/* Returns the "instance-size" of a given object "self",
 * whilst trying to resolve known standard allocators.
 * The caller must ensure that `!DeeType_IsVariable(Dee_TYPE(self))'
 * @return: * : The instance size of "self"
 * @return: 0 : Instance size is unknown (non-standard allocator used) */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) size_t
(DCALL DeeType_GetInstanceSize)(DeeTypeObject const *__restrict self);

/* Check if "self" may be considered as deep-immutable.
 * This is a (somewhat smarter) helper-wrapper around "DeeType_IsDeepImmutable()"
 * that does a couple of extra checks for certain types of shallow-immutable
 * objects, like "Tuple" of "_SeqOne". */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) bool
(DCALL DeeObject_IsDeepImmutable)(DeeObject const *__restrict self);


/* Helpers for allocating/freeing fixed-length (non-variable) type instances. */
#define DeeType_AllocInstance(tp_self)                                                      \
	(((tp_self)->tp_init.tp_alloc.tp_free)                                                  \
	 ? (DREF DeeObject *)(*(tp_self)->tp_init.tp_alloc.tp_alloc)()                          \
	 : DeeType_IsGC(tp_self)                                                                \
	   ? (DREF DeeObject *)DeeGCObject_Malloc((tp_self)->tp_init.tp_alloc.tp_instance_size) \
	   : (DREF DeeObject *)DeeObject_Malloc((tp_self)->tp_init.tp_alloc.tp_instance_size))
#define DeeType_FreeInstance(tp_self, obj)         \
	(((tp_self)->tp_init.tp_alloc.tp_free)         \
	 ? (*(tp_self)->tp_init.tp_alloc.tp_free)(obj) \
	 : DeeType_IsGC(tp_self)                       \
	   ? DeeGCObject_Free(obj)                     \
	   : DeeObject_Free(obj))

typedef struct {
	DeeTypeObject  const *tp_mro_orig; /* [1..1][const] MRO origin */
	DeeTypeObject *const *tp_mro_iter; /* [?..1][valid_if(:tp_iter != tp_mro_orig)] MRO array pointer */
} DeeTypeMRO;

/* Initialize an MRO enumerator. */
#define DeeTypeMRO_Init(self, tp_iter) \
	((DeeTypeObject *)((self)->tp_mro_orig = (tp_iter)))

/* Advance an MRO enumerator, returning the next type in MRO order.
 * @param: tp_iter: The previously enumerated type
 * @return: * :     The next type for the purpose of MRO resolution.
 * @return: NULL:   End of MRO chain has been reached. */
DFUNDEF WUNUSED NONNULL((1, 2)) DeeTypeObject *DFCALL
DeeTypeMRO_Next(DeeTypeMRO *__restrict self,
                DeeTypeObject const *tp_iter);

/* Like `DeeTypeMRO_Next()', but only enumerate direct
 * bases of the type passed to `DeeTypeMRO_Init()' */
DFUNDEF WUNUSED NONNULL((1, 2)) DeeTypeObject *DFCALL
DeeTypeMRO_NextDirectBase(DeeTypeMRO *__restrict self,
                          DeeTypeObject const *tp_iter);

#define DeeType_mro_foreach_start(tp_iter)  \
	do {                                    \
		DeeTypeMRO _tp_mro;                 \
		DeeTypeMRO_Init(&_tp_mro, tp_iter); \
		do
#define DeeType_mro_foreach_end(tp_iter)                                  \
		while (((tp_iter) = DeeTypeMRO_Next(&_tp_mro, tp_iter)) != NULL); \
	}	__WHILE0




/* Lookup information about operator `id', as defined by `typetype'
 * Returns NULL if the given operator is not known.
 * NOTE: The given `typetype' must be a type-type, meaning it must
 *       be the result of `Dee_TYPE(Dee_TYPE(ob))', in order to return
 *       information about generic operators that can be used on `ob' */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) struct Dee_opinfo const *DCALL
DeeTypeType_GetOperatorById(DeeTypeObject const *__restrict typetype, Dee_operator_t id);

/* Same as `DeeTypeType_GetOperatorById()', but also fill in `*p_declaring_type_type'
 * as the type-type that is declaring the operator "id". This can differ from "typetype"
 * in (e.g.) `DeeTypeType_GetOperatorByIdEx(&DeeFileType_Type, OPERATOR_BOOL)', where
 * `&DeeFileType_Type' is still able to implement "OPERATOR_BOOL", but the declaration
 * originates from `DeeType_Type', so in that case, `*p_declaring_type_type' is set to
 * `DeeType_Type', whereas for `FILE_OPERATOR_READ', it would be `DeeFileType_Type'
 * @param: p_declaring_type_type: [0..1] When non-null, store the declaring type here.
 * @return: NULL: No such operator (`*p_declaring_type_type' is undefined) */
DFUNDEF WUNUSED ATTR_OUT_OPT(3) NONNULL((1)) struct Dee_opinfo const *DCALL
DeeTypeType_GetOperatorByIdEx(DeeTypeObject const *__restrict typetype, Dee_operator_t id,
                              DeeTypeObject **p_declaring_type_type);

/* Same as `DeeTypeType_GetOperatorById()', but lookup operators by `oi_sname'
 * or `oi_uname' (though `oi_uname' only when that name isn't ambiguous).
 * @param: argc: The number of extra arguments taken by the operator (excluding
 *               the "this"-argument), or `(size_t)-1' if unknown. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1, 2)) struct Dee_opinfo const *DCALL
DeeTypeType_GetOperatorByName(DeeTypeObject const *__restrict typetype,
                              char const *__restrict name, size_t argc);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(2, 3) NONNULL((1)) struct Dee_opinfo const *DCALL
DeeTypeType_GetOperatorByNameLen(DeeTypeObject const *__restrict typetype,
                                 char const *__restrict name, size_t namelen,
                                 size_t argc);

/* Check if "self" is defining a custom descriptor for "id", and if so, return it. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) struct Dee_type_operator const *DCALL
DeeType_GetCustomOperatorById(DeeTypeObject const *__restrict self, Dee_operator_t id);

/* Lookup per-type method flags that may be defined for "opname".
 * IMPORTANT: When querying the flags for `OPERATOR_ITER', the `Dee_METHOD_FCONSTCALL',
 *            `Dee_METHOD_FPURECALL', and `Dee_METHOD_FNOREFESCAPE' flags doesn't mean that
 *            you can call `operator iter()' at compile-time. Instead, it means that
 *            *enumerating* the object can be done at compile-time (so-long as the associated
 *            iterator is never exposed). Alternatively, think of this case as allowing a
 *            call to `DeeObject_Foreach()' at compile-time.
 * @return: * : Set of `Dee_METHOD_F*' describing special optimizations possible for "opname".
 * @return: Dee_METHOD_FNORMAL: No special flags are defined for "opname" (or "opname" doesn't have special flags) */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
DeeType_GetOperatorFlags(DeeTypeObject const *__restrict self, Dee_operator_t opname);

/* Helper for checking that every cast-like operators is
 * either: not implemented, or marked as Dee_METHOD_FCONSTCALL:
 * >> (!DeeType_HasOperator(self, OPERATOR_BOOL) || (DeeType_GetOperatorFlags(self, OPERATOR_BOOL) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_INT) || (DeeType_GetOperatorFlags(self, OPERATOR_INT) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_FLOAT) || (DeeType_GetOperatorFlags(self, OPERATOR_FLOAT) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_ITER) || (DeeType_GetOperatorFlags(self, OPERATOR_ITER) & Dee_METHOD_FCONSTCALL));
 * This is the condition that must be fulfilled by all arguments other than "this" when
 * a function uses "Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST" to make its CONSTCALL flag
 * conditional. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_IsConstCastable(DeeTypeObject const *__restrict self);

/* Check if `name' is being implemented by the given type, or has been inherited by a base. */
#define DeeType_HasOperator(self, name) \
	DeeType_InheritOperator(Dee_REQUIRES_OBJECT(DeeTypeObject, self), name)

/* Check if the callback slot for `name' in `self' is populated.
 * If it isn't, then search the MRO of `self' for the first type
 * that *does* implement said operator, and cache that base's
 * callback in `self'
 * @return: true:  Either `self' already implemented the operator, it it was successfully inherited.
 * @return: false: `self' doesn't implement the operator, and neither does one of its bases (or the
 *                 operator could not be inherited by one of its bases). In this case, trying to
 *                 invoke the operator will result in a NotImplemented error. */
DFUNDEF NONNULL((1)) bool DCALL
DeeType_InheritOperator(DeeTypeObject *__restrict self, Dee_operator_t name);

/* Same as `DeeType_HasOperator()', however don't return `true' if the
 * operator has been inherited implicitly from a base-type of `self'. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasPrivateOperator(DeeTypeObject *__restrict self, Dee_operator_t name);

/* Return the type from `self' inherited its operator `name'.
 * If `name' wasn't inherited, or isn't defined, simply re-return `self'.
 * Returns `NULL' when the operator isn't being implemented. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetOperatorOrigin(DeeTypeObject const *__restrict self, Dee_operator_t name);

#ifdef CONFIG_BUILDING_DEEMON
/* Inherit different groups of operators from base-classes, returning `true' if
 * operators were inherited from some base class (even if those same operators
 * had already been inherited previously), and `false' if no base-class provides
 * any of the specified operators (though note that inheriting constructors
 * requires that all base classes carry the `Dee_TP_FINHERITCTOR' flag; else,
 * a class without this flag cannot inherit constructors from its base, though
 * can still provide its constructors to some derived class that does specify
 * its intend of inheriting constructors)
 *
 * s.a. `DeeType_InheritOperator()' */
INTDEF NONNULL((1)) bool DCALL DeeType_InheritConstructors(DeeTypeObject *__restrict self); /* tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_assign, tp_move_assign */
INTDEF NONNULL((1)) bool DCALL DeeType_InheritBuffer(DeeTypeObject *__restrict self);       /* tp_getbuf, tp_buffer_flags */
#else /* CONFIG_BUILDING_DEEMON */
#define DeeType_InheritConstructors(self) DeeType_InheritOperator(self, OPERATOR_CONSTRUCTOR)
#define DeeType_InheritBuffer(self)       DeeType_InheritOperator(self, OPERATOR_GETBUF)
#endif /* !CONFIG_BUILDING_DEEMON */

/* Invoke an operator on a given object, given its ID and arguments.
 * NOTE: Using these function, any operator can be invoked, including
 *       extension operators as well as some operators marked as
 *       `OPCC_SPECIAL' (most notably: `tp_int'), as well as throwing
 *       a `Signal.StopIteration' when `tp_iter_next' is exhausted.
 * Operators marked as `oi_private' cannot be invoked and
 * attempting to do so will cause an `Error.TypeError' to be thrown.
 * Attempting to invoke an unknown operator will cause an `Error.TypeError' to be thrown.
 * HINT: `DeeObject_PInvokeOperator' can be used the same way `DeeObject_InvokeOperator'
 *        can be, with the addition of allowing inplace operators to be executed.
 *        Attempting to execute an inplace operator using `DeeObject_InvokeOperator()'
 *        will cause an `Error.TypeError' to be thrown. */
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_InvokeOperator(DeeObject *self, Dee_operator_t name,
                         size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_TInvokeOperator(DeeTypeObject *tp_self, DeeObject *self,
                          Dee_operator_t name, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_PInvokeOperator(DREF DeeObject **__restrict p_self, Dee_operator_t name,
                          size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_PTInvokeOperator(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self,
                           Dee_operator_t name, size_t argc, DeeObject *const *argv);
#define DeeObject_InvokeOperatorTuple(self, name, args)              DeeObject_InvokeOperator(self, name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_TInvokeOperatorTuple(tp_self, self, name, args)    DeeObject_TInvokeOperator(tp_self, self, name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_PInvokeOperatorTuple(p_self, name, args)           DeeObject_PInvokeOperator(p_self, name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_PTInvokeOperatorTuple(tp_self, p_self, name, args) DeeObject_PTInvokeOperator(tp_self, p_self, name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))

DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DeeObject_TInvokeOperatorf(DeeTypeObject *tp_self, DeeObject *self, Dee_operator_t name, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL DeeObject_VTInvokeOperatorf(DeeTypeObject *tp_self, DeeObject *self, Dee_operator_t name, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DeeObject_PTInvokeOperatorf(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, Dee_operator_t name, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL DeeObject_VPTInvokeOperatorf(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self, Dee_operator_t name, char const *__restrict format, va_list args);
#define DeeObject_InvokeOperatorf(self, name, format, ...)      DeeObject_TInvokeOperatorf(Dee_TYPE(self), self, name, format, __VA_ARGS__)
#define DeeObject_VInvokeOperatorf(self, name, format, args)    DeeObject_VTInvokeOperatorf(Dee_TYPE(self), self, name, format, args)
#define DeeObject_PInvokeOperatorf(p_self, name, format, ...)   DeeObject_PTInvokeOperatorf(Dee_TYPE(*(p_self)), p_self, name, format, __VA_ARGS__)
#define DeeObject_PVInvokeOperatorf(p_self, name, format, args) DeeObject_VPTInvokeOperatorf(Dee_TYPE(*(p_self)), p_self, name, format, args)


/* Generic attribute lookup through `tp_self[->tp_base...]->tp_methods, tp_getsets, tp_members'
 * @return: -1 / ---   / NULL:          Error.
 * @return:  0 / true  / * :            OK.
 * @return:  1 / false / Dee_ITER_DONE: Not found. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericGetAttrStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericGetAttrStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) ATTR_INS(6, 5) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) ATTR_INS(7, 6) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) ATTR_INS(6, 5) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringHashKw(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) ATTR_INS(7, 6) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringLenHashKw(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *DCALL DeeObject_VTGenericCallAttrStringHashf(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericDelAttrStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericDelAttrStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL DeeObject_TGenericSetAttrStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL DeeObject_TGenericSetAttrStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeObject_TGenericHasAttrStringHash(DeeTypeObject *tp_self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeObject_TGenericHasAttrStringLenHash(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericBoundAttrStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash); /* Dee_BOUND_MISSING: not found; Dee_BOUND_ERR: error; Dee_BOUND_NO: unbound; Dee_BOUND_YES: bound; */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericBoundAttrStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash); /* Dee_BOUND_MISSING: not found; Dee_BOUND_ERR: error; Dee_BOUND_NO: unbound; Dee_BOUND_YES: bound; */

#ifdef __INTELLISENSE__
#define _Dee_PRIVATE_HashStr(s)    ((Dee_hash_t)(s))
#define _Dee_PRIVATE_HashPtr(p, n) ((Dee_hash_t)(p) + (n))
#else /* __INTELLISENSE__ */
#define _Dee_PRIVATE_HashStr(s)    Dee_HashStr(s)
#define _Dee_PRIVATE_HashPtr(p, n) Dee_HashPtr(p, n)
#endif /* !__INTELLISENSE__ */

#define DeeObject_TGenericGetAttr(tp_self, self, attr)                                      DeeObject_TGenericGetAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericGetAttrHash(tp_self, self, attr, hash)                            DeeObject_TGenericGetAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeObject_TGenericGetAttrString(tp_self, self, attr)                                DeeObject_TGenericGetAttrStringHash(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr))
#define DeeObject_TGenericGetAttrStringLen(tp_self, self, attr, attrlen)                    DeeObject_TGenericGetAttrStringLenHash(tp_self, self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen))
#define DeeObject_TGenericCallAttr(tp_self, self, attr, argc, argv)                         DeeObject_TGenericCallAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeObject_TGenericCallAttrHash(tp_self, self, attr, hash, argc, argv)               DeeObject_TGenericCallAttrStringHash(tp_self, self, DeeString_STR(attr), hash, argc, argv)
#define DeeObject_TGenericCallAttrString(tp_self, self, attr, argc, argv)                   DeeObject_TGenericCallAttrStringHash(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr), argc, argv)
#define DeeObject_TGenericCallAttrStringLen(tp_self, self, attr, attrlen, argc, argv)       DeeObject_TGenericCallAttrStringLenHash(tp_self, self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen), argc, argv)
#define DeeObject_TGenericCallAttrKw(tp_self, self, attr, argc, argv, kw)                   DeeObject_TGenericCallAttrStringHashKw(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeObject_TGenericCallAttrHashKw(tp_self, self, attr, hash, argc, argv, kw)         DeeObject_TGenericCallAttrStringHashKw(tp_self, self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeObject_TGenericCallAttrStringKw(tp_self, self, attr, argc, argv, kw)             DeeObject_TGenericCallAttrStringHashKw(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr), argc, argv, kw)
#define DeeObject_TGenericCallAttrStringLenKw(tp_self, self, attr, attrlen, argc, argv, kw) DeeObject_TGenericCallAttrStringLenHashKw(tp_self, self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeObject_VTGenericCallAttrf(tp_self, self, attr, format, args)                     DeeObject_VTGenericCallAttrStringHashf(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeObject_VTGenericCallAttrHashf(tp_self, self, attr, hash, format, args)           DeeObject_VTGenericCallAttrStringHashf(tp_self, self, DeeString_STR(attr), hash, format, args)
#define DeeObject_VTGenericCallAttrStringf(tp_self, self, attr, format, args)               DeeObject_VTGenericCallAttrStringHashf(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr), format, args)
#define DeeObject_TGenericDelAttr(tp_self, self, attr)                                      DeeObject_TGenericDelAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericDelAttrHash(tp_self, self, attr, hash)                            DeeObject_TGenericDelAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeObject_TGenericDelAttrString(tp_self, self, attr)                                DeeObject_TGenericDelAttrStringHash(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr))
#define DeeObject_TGenericDelAttrStringLen(tp_self, self, attr, attrlen)                    DeeObject_TGenericDelAttrStringLenHash(tp_self, self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen))
#define DeeObject_TGenericSetAttr(tp_self, self, attr, value)                               DeeObject_TGenericSetAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeObject_TGenericSetAttrHash(tp_self, self, attr, hash, value)                     DeeObject_TGenericSetAttrStringHash(tp_self, self, DeeString_STR(attr), hash, value)
#define DeeObject_TGenericSetAttrString(tp_self, self, attr, value)                         DeeObject_TGenericSetAttrStringHash(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr), value)
#define DeeObject_TGenericSetAttrStringLen(tp_self, self, attr, attrlen, value)             DeeObject_TGenericSetAttrStringLenHash(tp_self, self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen), value)
#define DeeObject_TGenericHasAttr(tp_self, attr)                                            DeeObject_TGenericHasAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericHasAttrHash(tp_self, attr, hash)                                  DeeObject_TGenericHasAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeObject_TGenericHasAttrString(tp_self, attr)                                      DeeObject_TGenericHasAttrStringHash(tp_self, attr, _Dee_PRIVATE_HashStr(attr))
#define DeeObject_TGenericHasAttrStringLen(tp_self, attr, attrlen)                          DeeObject_TGenericHasAttrStringLenHash(tp_self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen))
#define DeeObject_TGenericBoundAttr(tp_self, self, attr)                                    DeeObject_TGenericBoundAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericBoundAttrHash(tp_self, self, attr, hash)                          DeeObject_TGenericBoundAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeObject_TGenericBoundAttrString(tp_self, self, attr)                              DeeObject_TGenericBoundAttrStringHash(tp_self, self, attr, _Dee_PRIVATE_HashStr(attr))
#define DeeObject_TGenericBoundAttrStringLen(tp_self, self, attr, attrlen)                  DeeObject_TGenericBoundAttrStringLenHash(tp_self, self, attr, attrlen, _Dee_PRIVATE_HashPtr(attr, attrlen))

#define DeeObject_GenericGetAttr(self, attr)                                                  DeeObject_TGenericGetAttr(Dee_TYPE(self), self, attr)
#define DeeObject_GenericGetAttrHash(self, attr, hash)                                        DeeObject_TGenericGetAttrHash(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericGetAttrString(self, attr)                                            DeeObject_TGenericGetAttrString(Dee_TYPE(self), self, attr)
#define DeeObject_GenericGetAttrStringLen(self, attr, attrlen)                                DeeObject_TGenericGetAttrStringLen(Dee_TYPE(self), self, attr, attrlen)
#define DeeObject_GenericGetAttrStringHash(self, attr, hash)                                  DeeObject_TGenericGetAttrStringHash(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericGetAttrStringLenHash(self, attr, attrlen, hash)                      DeeObject_TGenericGetAttrStringLenHash(Dee_TYPE(self), self, attr, attrlen, hash)
#define DeeObject_GenericCallAttr(self, attr, argc, argv)                                     DeeObject_TGenericCallAttr(Dee_TYPE(self), self, attr, argc, argv)
#define DeeObject_GenericCallAttrHash(self, attr, hash, argc, argv)                           DeeObject_TGenericCallAttrHash(Dee_TYPE(self), self, attr, hash, argc, argv)
#define DeeObject_GenericCallAttrString(self, attr, argc, argv)                               DeeObject_TGenericCallAttrString(Dee_TYPE(self), self, attr, argc, argv)
#define DeeObject_GenericCallAttrStringLen(self, attr, attrlen, argc, argv)                   DeeObject_TGenericCallAttrStringLen(Dee_TYPE(self), self, attr, attrlen, argc, argv)
#define DeeObject_GenericCallAttrStringHash(self, attr, hash, argc, argv)                     DeeObject_TGenericCallAttrStringHash(Dee_TYPE(self), self, attr, hash, argc, argv)
#define DeeObject_GenericCallAttrStringLenHash(self, attr, attrlen, hash, argc, argv)         DeeObject_TGenericCallAttrStringLenHash(Dee_TYPE(self), self, attr, attrlen, hash, argc, argv)
#define DeeObject_GenericCallAttrKw(self, attr, argc, argv, kw)                               DeeObject_TGenericCallAttrKw(Dee_TYPE(self), self, attr, argc, argv, kw)
#define DeeObject_GenericCallAttrHashKw(self, attr, hash, argc, argv, kw)                     DeeObject_TGenericCallAttrHashKw(Dee_TYPE(self), self, attr, hash, argc, argv, kw)
#define DeeObject_GenericCallAttrStringKw(self, attr, argc, argv, kw)                         DeeObject_TGenericCallAttrStringKw(Dee_TYPE(self), self, attr, argc, argv, kw)
#define DeeObject_GenericCallAttrStringLenKw(self, attr, attrlen, argc, argv, kw)             DeeObject_TGenericCallAttrStringLenKw(Dee_TYPE(self), self, attr, attrlen, argc, argv, kw)
#define DeeObject_GenericCallAttrStringHashKw(self, attr, hash, argc, argv, kw)               DeeObject_TGenericCallAttrStringHashKw(Dee_TYPE(self), self, attr, hash, argc, argv, kw)
#define DeeObject_GenericCallAttrStringLenHashKw(self, attr, attrlen, hash, argc, argv, kw)   DeeObject_TGenericCallAttrStringLenHashKw(Dee_TYPE(self), self, attr, attrlen, hash, argc, argv, kw)
#define DeeObject_VGenericCallAttrf(self, attr, format, args)                                 DeeObject_VTGenericCallAttrf(Dee_TYPE(self), self, attr, format, args)
#define DeeObject_VGenericCallAttrHashf(self, attr, hash, format, args)                       DeeObject_VTGenericCallAttrHashf(Dee_TYPE(self), self, attr, hash, format, args)
#define DeeObject_VGenericCallAttrStringf(self, attr, format, args)                           DeeObject_VTGenericCallAttrStringf(Dee_TYPE(self), self, attr, format, args)
#define DeeObject_VGenericCallAttrStringHashf(self, attr, hash, format, args)                 DeeObject_VTGenericCallAttrStringHashf(Dee_TYPE(self), self, attr, hash, format, args)
#define DeeObject_GenericDelAttr(self, attr)                                                  DeeObject_TGenericDelAttr(Dee_TYPE(self), self, attr)
#define DeeObject_GenericDelAttrHash(self, attr, hash)                                        DeeObject_TGenericDelAttrHash(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericDelAttrString(self, attr)                                            DeeObject_TGenericDelAttrString(Dee_TYPE(self), self, attr)
#define DeeObject_GenericDelAttrStringLen(self, attr, attrlen)                                DeeObject_TGenericDelAttrStringLen(Dee_TYPE(self), self, attr, attrlen)
#define DeeObject_GenericDelAttrStringHash(self, attr, hash)                                  DeeObject_TGenericDelAttrStringHash(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericDelAttrStringLenHash(self, attr, attrlen, hash)                      DeeObject_TGenericDelAttrStringLenHash(Dee_TYPE(self), self, attr, attrlen, hash)
#define DeeObject_GenericSetAttr(self, attr, value)                                           DeeObject_TGenericSetAttr(Dee_TYPE(self), self, attr, value)
#define DeeObject_GenericSetAttrHash(self, attr, hash, value)                                 DeeObject_TGenericSetAttrHash(Dee_TYPE(self), self, attr, hash, value)
#define DeeObject_GenericSetAttrString(self, attr, value)                                     DeeObject_TGenericSetAttrString(Dee_TYPE(self), self, attr, value)
#define DeeObject_GenericSetAttrStringLen(self, attr, attrlen, value)                         DeeObject_TGenericSetAttrStringLen(Dee_TYPE(self), self, attr, attrlen, value)
#define DeeObject_GenericSetAttrStringHash(self, attr, hash, value)                           DeeObject_TGenericSetAttrStringHash(Dee_TYPE(self), self, attr, hash, value)
#define DeeObject_GenericSetAttrStringLenHash(self, attr, attrlen, hash, value)               DeeObject_TGenericSetAttrStringLenHash(Dee_TYPE(self), self, attr, attrlen, hash, value)
#define DeeObject_GenericHasAttr(self, attr)                                                  DeeObject_TGenericHasAttr(Dee_TYPE(self), attr)
#define DeeObject_GenericHasAttrHash(self, attr, hash)                                        DeeObject_TGenericHasAttrHash(Dee_TYPE(self), attr, hash)
#define DeeObject_GenericHasAttrString(self, attr)                                            DeeObject_TGenericHasAttrString(Dee_TYPE(self), attr)
#define DeeObject_GenericHasAttrStringLen(self, attr, attrlen)                                DeeObject_TGenericHasAttrStringLen(Dee_TYPE(self), attr, attrlen)
#define DeeObject_GenericHasAttrStringHash(self, attr, hash)                                  DeeObject_TGenericHasAttrStringHash(Dee_TYPE(self), attr, hash)
#define DeeObject_GenericHasAttrStringLenHash(self, attr, attrlen, hash)                      DeeObject_TGenericHasAttrStringLenHash(Dee_TYPE(self), attr, attrlen, hash)
#define DeeObject_GenericBoundAttr(self, attr)                                                DeeObject_TGenericBoundAttr(Dee_TYPE(self), self, attr)
#define DeeObject_GenericBoundAttrHash(self, attr, hash)                                      DeeObject_TGenericBoundAttrHash(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericBoundAttrString(self, attr)                                          DeeObject_TGenericBoundAttrString(Dee_TYPE(self), self, attr)
#define DeeObject_GenericBoundAttrStringLen(self, attr, attrlen)                              DeeObject_TGenericBoundAttrStringLen(Dee_TYPE(self), self, attr, attrlen)
#define DeeObject_GenericBoundAttrStringHash(self, attr, hash)                                DeeObject_TGenericBoundAttrStringHash(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericBoundAttrStringLenHash(self, attr, attrlen, hash)                    DeeObject_TGenericBoundAttrStringLenHash(Dee_TYPE(self), self, attr, attrlen, hash)



/* Generic operators that implement equals using `===' and hash using `Object.id()'
 * Use this instead of re-inventing the wheel in order to allow for special optimization
 * to be possible when your type appears in compare operations. */
DDATDEF struct Dee_type_cmp DeeObject_GenericCmpByAddr;

DDATDEF DeeTypeObject DeeType_Type;   /* `type(Object)' */
#define DeeType_Check(ob)      DeeObject_InstanceOf(ob, &DeeType_Type)
#define DeeType_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeType_Type)

struct Dee_module_object;
/* Return the module used to define a given type `self',
 * or `NULL' if that module could not be determined.
 * NOTE: When `NULL' is returned, _NO_ error is thrown! */
DFUNDEF WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeType_GetModule(DeeTypeObject *__restrict self);

/* Returns the `tp_name' of `self', or the string
 * "<anonymous type>" when `self' doesn't have a
 * type name set. */
DFUNDEF ATTR_RETNONNULL ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeType_GetName(DeeTypeObject const *__restrict self);




/* Type visit helpers.
 * WARNING: These helper macros are allowed to evaluate their arguments multiple times! */
#define Dee_Visit(ob)  (*proc)(Dee_AsObject(ob), arg)
#define Dee_XVisit(ob) (void)(!(ob) || (Dee_Visit(ob), 0))

#define Dee_Visitv(object_vector, object_count)                         \
	do {                                                                \
		size_t _dvv_i;                                                  \
		for (_dvv_i = 0; _dvv_i < (size_t)(object_count); ++_dvv_i) {   \
			DeeObject *_dvv_ob = Dee_AsObject((object_vector)[_dvv_i]); \
			Dee_Visit(_dvv_ob);                                         \
		}                                                               \
	}	__WHILE0
#define Dee_XVisitv(object_vector, object_count)                        \
	do {                                                                \
		size_t _dvv_i;                                                  \
		for (_dvv_i = 0; _dvv_i < (size_t)(object_count); ++_dvv_i) {   \
			DeeObject *_dvv_ob = Dee_AsObject((object_vector)[_dvv_i]); \
			Dee_XVisit(_dvv_ob);                                        \
		}                                                               \
	}	__WHILE0


/* Used to undo object construction in generic sub-classes
 * after base classes had already been constructed when a
 * later constructor fails.
 * This function will invoke destructors of each type already constructed. */
DFUNDEF WUNUSED NONNULL((2)) bool DCALL
DeeObject_UndoConstruction(DeeTypeObject *undo_start,
                           DeeObject *self);

/* Same as `DeeObject_UndoConstruction()', however optimize for the
 * case of `undo_start' known to either be `NULL' or `DeeObject_Type' */
#define DeeObject_UndoConstructionNoBase(self) \
	__hybrid_atomic_cmpxch(&(self)->ob_refcnt, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)


#ifdef GUARD_DEEMON_OBJECT_H
#ifdef NDEBUG
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)                        (void)0 /*!export(include("object.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)                    (void)0 /*!export(include("object.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, file, line)         (void)0 /*!export(include("object.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, file, line)     (void)0 /*!export(include("object.h"))*/
#else /* NDEBUG */
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)                        Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, __FILE__, __LINE__) /*!export(include("object.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)                    Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, __FILE__, __LINE__) /*!export(include("object.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, file, line)         (void)(likely((DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type)))) || (DeeAssert_BadObjectType((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0)) /*!export(include("object.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, file, line)     (void)(likely(!(ob) || (DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type)))) || (DeeAssert_BadObjectTypeOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0)) /*!export(include("object.h"))*/
#endif /* !NDEBUG */
#ifdef DEE_SOURCE
#define ASSERT_OBJECT_TYPE_A        Dee_ASSERT_OBJECT_TYPE_A        /*!export(include("object.h"))*/
#define ASSERT_OBJECT_TYPE_A_OPT    Dee_ASSERT_OBJECT_TYPE_A_OPT    /*!export(include("object.h"))*/
#define ASSERT_OBJECT_TYPE_A_AT     Dee_ASSERT_OBJECT_TYPE_A_AT     /*!export(include("object.h"))*/
#define ASSERT_OBJECT_TYPE_A_OPT_AT Dee_ASSERT_OBJECT_TYPE_A_OPT_AT /*!export(include("object.h"))*/
#endif /* DEE_SOURCE */
#endif /* GUARD_DEEMON_OBJECT_H */

#ifdef GUARD_DEEMON_TUPLE_H
/*!export Dee_tuple_builder_visit(include("tuple.h"))*/
/*!export Dee_nullable_tuple_builder_visit(include("tuple.h"))*/
#define Dee_tuple_builder_visit(self)                              \
	do {                                                           \
		if ((self)->tb_tuple)                                      \
			Dee_Visitv((self)->tb_tuple->t_elem, (self)->tb_size); \
	}	__WHILE0
#define Dee_nullable_tuple_builder_visit(self)                      \
	do {                                                            \
		if ((self)->tb_tuple)                                       \
			Dee_XVisitv((self)->tb_tuple->t_elem, (self)->tb_size); \
	}	__WHILE0
#endif /* GUARD_DEEMON_TUPLE_H */

DECL_END

#endif /* !GUARD_DEEMON_TYPE_H */
