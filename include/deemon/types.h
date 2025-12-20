/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_TYPES_H
#define GUARD_DEEMON_TYPES_H 1

#include "api.h"
/**/

#include <hybrid/int128.h>
#include <hybrid/typecore.h>
/**/

#include <stddef.h>  /* size_t, NULL */

DECL_BEGIN

#ifndef DREF
#define DREF  /* Annotation for pointer: transfer/storage of a reference.
               * NOTE: When returned by a function, a return value
               *       of NULL indicates a newly raised exception. */
#endif /* !DREF */

#ifdef DEE_SOURCE
#define Dee_object       object_
#define Dee_type_object  type_object
#define Dee_weakref      weakref
#define Dee_weakref_list weakref_list
#define OBJECT_HEAD      Dee_OBJECT_HEAD
#define OBJECT_HEAD_EX   Dee_OBJECT_HEAD_EX
#define OBJECT_HEAD_INIT Dee_OBJECT_HEAD_INIT
#endif /* DEE_SOURCE */

struct Dee_object;
struct Dee_type_object;

typedef struct Dee_object      DeeObject;     /* Common base for all deemon objects */
typedef struct Dee_type_object DeeTypeObject; /* Common base for all deemon type objects */
typedef __SSIZE_TYPE__         Dee_ssize_t;   /* Signed size type (s.a. `size_t') */
typedef __ULONG64_TYPE__       Dee_pos_t;     /* File position */
typedef __LONG64_TYPE__        Dee_off_t;     /* Delta between 2 file positions */
typedef __UINTPTR_TYPE__       Dee_refcnt_t;  /* Object reference count type */
typedef __UINTPTR_TYPE__       Dee_hash_t;    /* Object hash data type */
typedef __hybrid_uint128_t     Dee_uint128_t; /* unsigned, 128-bit integer */
typedef __hybrid_int128_t      Dee_int128_t;  /* signed, 128-bit integer */

#define Dee_SIZEOF_POS_T    8                  /* == sizeof(Dee_pos_t) */
#define Dee_SIZEOF_OFF_T    Dee_SIZEOF_POS_T   /* == sizeof(Dee_off_t) */
#define Dee_SIZEOF_REFCNT_T __SIZEOF_POINTER__ /* == sizeof(Dee_refcnt_t) */
#define Dee_SIZEOF_HASH_T   __SIZEOF_POINTER__ /* == sizeof(Dee_hash_t) */

/* Hash selection macros used by generated code */
#if Dee_SIZEOF_HASH_T <= 4
#define _Dee_HashSelect(hash32, hash64)  hash32
#define _Dee_HashSelectC(hash32, hash64) __UINT32_C(hash32)
#else /* Dee_SIZEOF_HASH_T <= 4 */
#define _Dee_HashSelect(hash32, hash64)  hash64
#define _Dee_HashSelectC(hash32, hash64) __UINT64_C(hash64)
#endif /* Dee_SIZEOF_HASH_T > 4 */




/* Generic print receiver.
 * @param: arg:     [?..?] Caller-provided cookie
 * @param: data:    [0..datalen] Chunk base-pointer (usually in utf-8)
 * @param: datalen: The number of *bytes* to consume, starting at "data"
 * @return: >= 0:   Success; return value gets added to total propagated by caller.
 * @return: < 0:    Error; abort printing and immediately propagate this value to caller. */
typedef WUNUSED_T ATTR_INS_T(2, 3) Dee_ssize_t
(DPRINTER_CC *Dee_formatprinter_t)(void *arg, char const *__restrict data, size_t datalen);

#ifdef __cplusplus
typedef void (*Dee_funptr_t)(void);
#else /* __cplusplus */
typedef void (*Dee_funptr_t)();
#endif /* !__cplusplus */

/* Foreach callbacks. */
typedef WUNUSED_T NONNULL_T((2)) Dee_ssize_t (DCALL *Dee_foreach_t)(void *arg, DeeObject *elem);
typedef WUNUSED_T NONNULL_T((2, 3)) Dee_ssize_t (DCALL *Dee_foreach_pair_t)(void *arg, DeeObject *key, DeeObject *value);




/* Iterator/tristate pointer helpers. */
#define Dee_ITER_ISOK(x) (((uintptr_t)(x) - 1) < (uintptr_t)-2l) /* `x != NULL && x != Dee_ITER_DONE' */
#define Dee_ITER_DONE    ((DeeObject *)-1l) /* Returned when the iterator has been exhausted. */
#ifdef DEE_SOURCE
#define ITER_ISOK Dee_ITER_ISOK /* `x != NULL && x != ITER_DONE' */
#define ITER_DONE Dee_ITER_DONE /* Returned when the iterator has been exhausted. */
#endif /* DEE_SOURCE */




/************************************************************************/
/* DeeObject                                                            */
/************************************************************************/
#ifdef CONFIG_TRACE_REFCHANGES
struct Dee_reftracker;
#define DEE_REFTRACKER_UNTRACKED ((struct Dee_reftracker  *)(uintptr_t)-1l)
#define DEE_PRIVATE_REFCHANGE_PRIVATE_DATA \
	struct Dee_reftracker *ob_trace; /* [0..1][owned][lock(WRITE_ONCE)] Tracked reference counter data. */
#define DEE_OBJECT_OFFSETOF_DATA (__SIZEOF_POINTER__ * 3)
#else /* CONFIG_TRACE_REFCHANGES */
#define DEE_PRIVATE_REFCHANGE_PRIVATE_DATA  /* nothing */
#define DEE_OBJECT_OFFSETOF_DATA (__SIZEOF_POINTER__ * 2)
#endif /* !CONFIG_TRACE_REFCHANGES */

/* Statically defined offsets within deemon objects. */
#define DEE_OBJECT_OFFSETOF_REFCNT  0
#define DEE_OBJECT_OFFSETOF_TYPE    __SIZEOF_POINTER__


/* IDE hint for macros that require arguments types to implement `OBJECT_HEAD' */
#ifdef __INTELLISENSE__
#ifdef __cplusplus
extern "C++" {namespace __intern {
template<class T, class S> T *__Dee_REQUIRES_OBJECT(S const *x, decltype(((S *)0)->ob_refcnt)=0);
template<class T> T *__Dee_REQUIRES_OBJECT(decltype(nullptr));
}} /* extern "C++" */
#define Dee_REQUIRES_OBJECT(T, x) ::__intern::__Dee_REQUIRES_OBJECT< T >(x)
#else /* __cplusplus */
#define Dee_REQUIRES_OBJECT(T, x) ((T *)&(x)->ob_refcnt)
#endif /* !__cplusplus */
#else /* __INTELLISENSE__ */
#define Dee_REQUIRES_OBJECT(T, x) ((T *)(x))
#endif /* !__INTELLISENSE__ */
#define Dee_REQUIRES_ANYOBJECT(x) Dee_REQUIRES_OBJECT(DeeObject, x)


#ifdef __INTELLISENSE__
#define Dee_OBJECT_HEAD       \
	Dee_refcnt_t   ob_refcnt; \
	DeeTypeObject *ob_type;   \
	DEE_PRIVATE_REFCHANGE_PRIVATE_DATA
#define Dee_OBJECT_HEAD_EX(Ttype) \
	Dee_refcnt_t ob_refcnt;       \
	Ttype       *ob_type;         \
	DEE_PRIVATE_REFCHANGE_PRIVATE_DATA
struct Dee_object {
	Dee_refcnt_t   ob_refcnt;
	DeeTypeObject *ob_type;
	DEE_PRIVATE_REFCHANGE_PRIVATE_DATA
};
#else /* __INTELLISENSE__ */
#define Dee_OBJECT_HEAD            \
	Dee_refcnt_t        ob_refcnt; \
	DREF DeeTypeObject *ob_type;   \
	DEE_PRIVATE_REFCHANGE_PRIVATE_DATA
#define Dee_OBJECT_HEAD_EX(Ttype) \
	Dee_refcnt_t ob_refcnt;       \
	DREF Ttype  *ob_type;         \
	DEE_PRIVATE_REFCHANGE_PRIVATE_DATA
struct Dee_object {
	Dee_OBJECT_HEAD
};
#endif /* !__INTELLISENSE__ */



#ifndef Dee_STATIC_REFCOUNT_INIT
#ifdef CONFIG_BUILDING_DEEMON
/* We add +1 for all statically initialized objects, so
 * we can easily add them to deemon's builtin module. */
#define Dee_STATIC_REFCOUNT_INIT 3
#else /* CONFIG_BUILDING_DEEMON */
#define Dee_STATIC_REFCOUNT_INIT 2
#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* !Dee_STATIC_REFCOUNT_INIT */

#ifdef CONFIG_TRACE_REFCHANGES
#define Dee_OBJECT_HEAD_INIT(type) Dee_STATIC_REFCOUNT_INIT, type, DEE_REFTRACKER_UNTRACKED
#else /* CONFIG_TRACE_REFCHANGES */
#define Dee_OBJECT_HEAD_INIT(type) Dee_STATIC_REFCOUNT_INIT, type
#endif /* !CONFIG_TRACE_REFCHANGES */



/* Object typeof(). */
#define Dee_TYPE(self) (self)->ob_type


/* Object inheritance checking:
 * - DeeObject_Implements:      Check if part of MRO
 * - DeeObject_InstanceOf:      Check if part of base-chain (implies MRO; enough for non-TP_FABSTRACT types)
 * - DeeObject_InstanceOfExact: Check if type matches exactly (fastest check; enough for TP_FFINAL/TP_FVARIABLE types) */
#define DeeObject_Implements(self, super_type)       DeeType_Implements(Dee_TYPE(self), super_type)
#define DeeObject_InstanceOf(self, super_type)       DeeType_Extends(Dee_TYPE(self), super_type)
#define DeeObject_InstanceOfExact(self, object_type) (Dee_TYPE(self) == (object_type))


/* Return true if `test_type' is equal to, or extends `extended_type'
 * NOTE: When `extended_type' is not a type, this function simply returns `false'
 * >> return test_type.extends(extended_type);
 *
 * HINT: Always returns either `0' or `1'!
 * @return: 0 : "test_type" does not inherit from `extended_type', or `extended_type' isn't a type
 * @return: 1 : "test_type" does inherit from `extended_type' */
DFUNDEF WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_Extends(DeeTypeObject const *test_type,
                DeeTypeObject const *extended_type);

/* Same as `DeeType_Extends()', but also check `tp_mro' for matches.
 * This function should be used when `implemented_type' is an abstract type.
 * >> return test_type.implements(implemented_type);
 *
 * HINT: Always returns either `0' or `1'!
 * @return: 0 : "test_type" does not implement "implemented_type"
 * @return: 1 : "test_type" does implement "implemented_type" */
DFUNDEF WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_Implements(DeeTypeObject const *test_type,
                   DeeTypeObject const *implemented_type);





struct Dee_weakref;
struct Dee_weakref_list {
	struct Dee_weakref *wl_nodes; /* [0..1][lock(BIT0(wl_nodes))] chain of weak references. */
};

/* Structure field: When present in an object, it supports weak referencing. */
#define Dee_WEAKREF_SUPPORT         struct Dee_weakref_list ob_weakrefs;
#define Dee_WEAKREF_SUPPORT_ADDR(T) offsetof(T, ob_weakrefs)
#define Dee_WEAKREF_SUPPORT_INIT    { NULL }

/* Initialize weakref support (must be called manually by
 * constructors of types implementing weakref support!) */
#define Dee_weakref_support_init(x) ((x)->ob_weakrefs.wl_nodes = NULL)

/* Finalize weakref support (must be called manually by
 * destructors of types implementing weakref support!) */
DFUNDEF NONNULL((1)) void (DCALL Dee_weakref_support_fini)(struct Dee_weakref_list *__restrict self);
#if defined(__OPTIMIZE_SIZE__) || defined(__INTELLISENSE__)
#define Dee_weakref_support_fini(x) (Dee_weakref_support_fini)(&(x)->ob_weakrefs)
#else /* __OPTIMIZE_SIZE__ || __INTELLISENSE__ */
DECL_END
#include <hybrid/__atomic.h>
DECL_BEGIN
#define Dee_weakref_support_fini(x)                                     \
	(__hybrid_atomic_load(&(x)->ob_weakrefs.wl_nodes, __ATOMIC_ACQUIRE) \
	 ? (Dee_weakref_support_fini)(&(x)->ob_weakrefs)                    \
	 : (void)0)
#endif /* !__OPTIMIZE_SIZE__ && !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#define WEAKREF_SUPPORT      Dee_WEAKREF_SUPPORT
#define WEAKREF_SUPPORT_ADDR Dee_WEAKREF_SUPPORT_ADDR
#define WEAKREF_SUPPORT_INIT Dee_WEAKREF_SUPPORT_INIT
#define weakref_support_init Dee_weakref_support_init
#define weakref_support_fini Dee_weakref_support_fini
#endif /* DEE_SOURCE */


DECL_END

#endif /* !GUARD_DEEMON_TYPES_H */
