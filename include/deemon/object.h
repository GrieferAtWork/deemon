/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECT_H
#define GUARD_DEEMON_OBJECT_H 1

#include "api.h"

#include <hybrid/__atomic.h>
#include <hybrid/__byteorder.h>
#include <hybrid/int128.h>
#include <hybrid/typecore.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */

#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#elif !defined(CONFIG_HAVE_STRING_H) && \
      (defined(__NO_has_include) || __has_include(<string.h>))
#define CONFIG_HAVE_STRING_H 1
#endif

#ifdef CONFIG_NO_strlen
#undef CONFIG_HAVE_strlen
#else
#define CONFIG_HAVE_strlen 1
#endif

#ifdef CONFIG_HAVE_STRING_H
#include <string.h>
#endif /* CONFIG_HAVE_STRING_H */

/* Const modified for static type callback-table declaration */
#ifndef Dee_tpconst
#if (defined(__PIC__) || defined(__PIE__) || \
     defined(__pic__) || defined(__pie__))
#define Dee_tpconst /* nothing */
#elif defined(CONFIG_BUILDING_DEEMON)
#define Dee_tpconst const
#else /* ... */
#define Dee_tpconst const
#endif /* !... */
#endif /* !Dee_tpconst */

#ifdef DEE_SOURCE
#define tpconst Dee_tpconst
#endif /* DEE_SOURCE */

DECL_BEGIN

#ifndef CONFIG_HAVE_strlen
#define CONFIG_HAVE_strlen 1
DECL_BEGIN
#undef strlen
#define strlen dee_strlen
LOCAL WUNUSED NONNULL((1)) size_t dee_strlen(char const *str) {
	size_t result;
	for (result = 0; str[result]; ++result)
		;
	return result;
}
DECL_END
#endif /* !CONFIG_HAVE_strlen */

#ifdef __INTELLISENSE__
typedef __INTPTR_TYPE__  intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
#endif /* __INTELLISENSE__ */

#ifdef DEE_SOURCE
#define Dee_weakref          weakref
#define Dee_object           object_
#define Dee_type_object      type_object
#define Dee_class_desc       class_desc
#define Dee_membercache_slot membercache_slot
#define Dee_membercache      membercache
#define Dee_type_method      type_method
#define Dee_type_getset      type_getset
#define Dee_type_member      type_member
#define Dee_type_constructor type_constructor
#define Dee_type_cast        type_cast
#define Dee_type_gc          type_gc
#define Dee_type_math        type_math
#define Dee_type_cmp         type_cmp
#define Dee_type_nii         type_nii
#define Dee_type_seq         type_seq
#define Dee_type_nsi         type_nsi
#define Dee_type_attr        type_attr
#define Dee_type_with        type_with
#define Dee_type_buffer      type_buffer
#define Dee_weakref_list     weakref_list
#define Dee_opinfo           opinfo
#endif /* DEE_SOURCE */

struct Dee_type_object;
struct Dee_object;
struct Dee_weakref;
struct Dee_class_desc;

typedef struct Dee_type_object DeeTypeObject;
typedef struct Dee_object      DeeObject;
typedef uintptr_t              Dee_refcnt_t;
typedef __SSIZE_TYPE__         Dee_ssize_t;
typedef __LONG64_TYPE__        Dee_off_t;
typedef __ULONG64_TYPE__       Dee_pos_t;
typedef uintptr_t              Dee_hash_t;

#ifdef DEE_SOURCE
typedef Dee_refcnt_t drefcnt_t;
typedef Dee_ssize_t  dssize_t;
typedef Dee_off_t    doff_t;
typedef Dee_pos_t    dpos_t;
typedef Dee_hash_t   dhash_t;
#endif /* DEE_SOURCE */

/* Hashing helpers. */
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashPtr)(void const *__restrict ptr, size_t n_bytes);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashCasePtr)(void const *__restrict ptr, size_t n_bytes);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashStr)(char const *__restrict str);
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashCaseStr)(char const *__restrict str);
#else /* __INTELLISENSE__ */
#define Dee_HashCaseStr(str) Dee_HashCasePtr(str, strlen(str))
#endif /* !__INTELLISENSE__ */

/* Hash a utf-8 encoded string.
 * You can think of these as hashing the ordinal values of the given string,
 * thus allowing this hashing function to return the same value for a string
 * encoded in utf-8, as `Dee_Hash2Byte()' would for a 2-byte, and Dee_Hash4Byte() for
 * a 4-byte string. */
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashUtf8)(char const *__restrict ptr, size_t n_bytes);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashCaseUtf8)(char const *__restrict ptr, size_t n_bytes);

/* Same as the regular hashing function, but with the guaranty that
 * for integer arrays where all items contain values `<= 0xff', the
 * return value is identical to a call to `Dee_HashPtr()' with the array
 * contents down-casted to the fitting data type. */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_Hash1Byte)(uint8_t const *__restrict ptr, size_t n_bytes);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashCase1Byte)(uint8_t const *__restrict ptr, size_t n_bytes);
#else /* __INTELLISENSE__ */
#define Dee_Hash1Byte(ptr, n_bytes)     Dee_HashPtr(ptr, n_bytes)
#define Dee_HashCase1Byte(ptr, n_bytes) Dee_HashCasePtr(ptr, n_bytes)
#endif /* !__INTELLISENSE__ */
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_Hash2Byte)(uint16_t const *__restrict ptr, size_t n_words);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_Hash4Byte)(uint32_t const *__restrict ptr, size_t n_dwords);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashCase2Byte)(uint16_t const *__restrict ptr, size_t n_words);
DFUNDEF WUNUSED ATTR_PURE Dee_hash_t (DCALL Dee_HashCase4Byte)(uint32_t const *__restrict ptr, size_t n_dwords);

/* Generic object hashing: Use the address of the object.
 * HINT: We ignore the lower 6 bits because they're
 *       often just ZERO(0) due to alignment. */
#define DeeObject_HashGeneric(ob) Dee_HashPointer(ob)
#define Dee_HashPointer(ptr)      ((Dee_hash_t)(ptr) >> 6)
#define DeeObject_Id(ob)          ((uintptr_t)(ob))


#define Dee_UINT128_GET8   __hybrid_uint128_vec8
#define Dee_UINT128_GET16  __hybrid_uint128_vec16
#define Dee_UINT128_GET32  __hybrid_uint128_vec32
#define Dee_UINT128_GET64  __hybrid_uint128_vec64
#define Dee_UINT128_GETS8  __hybrid_int128_vec8
#define Dee_UINT128_GETS16 __hybrid_int128_vec16
#define Dee_UINT128_GETS32 __hybrid_int128_vec32
#define Dee_UINT128_GETS64 __hybrid_int128_vec64
typedef __hybrid_uint128_t Dee_uint128_t;
typedef __hybrid_int128_t Dee_int128_t;

#ifdef DEE_SOURCE
typedef Dee_int128_t  dint128_t;
typedef Dee_uint128_t duint128_t;
#define DUINT128_GET8   __hybrid_uint128_vec8
#define DUINT128_GET16  __hybrid_uint128_vec16
#define DUINT128_GET32  __hybrid_uint128_vec32
#define DUINT128_GET64  __hybrid_uint128_vec64
#define DUINT128_GETS8  __hybrid_int128_vec8
#define DUINT128_GETS16 __hybrid_int128_vec16
#define DUINT128_GETS32 __hybrid_int128_vec32
#define DUINT128_GETS64 __hybrid_int128_vec64
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
#define DEE_PRIVATE_REFCHANGE_PRIVATE_DATA \
	struct Dee_reftracker *ob_trace; /* [0..1][owned][lock(WRITE_ONCE)] Tracked reference counter data. */
#define DEE_OBJECT_OFFSETOF_DATA  (__SIZEOF_POINTER__ * 3)
#define DEE_REFTRACKER_UNTRACKED  ((struct Dee_reftracker  *)(uintptr_t)-1)

DFUNDEF void DCALL Dee_DumpReferenceLeaks(void);
#else /* CONFIG_TRACE_REFCHANGES */
#define DEE_PRIVATE_REFCHANGE_PRIVATE_DATA  /* nothing */
#define DEE_OBJECT_OFFSETOF_DATA   (__SIZEOF_POINTER__ * 2)
#endif /* !CONFIG_TRACE_REFCHANGES */

#define DEE_OBJECT_OFFSETOF_REFCNT  0
#define DEE_OBJECT_OFFSETOF_TYPE    __SIZEOF_POINTER__



#ifdef __INTELLISENSE__
#define Dee_REQUIRES_OBJECT(x) ((void)&(x)->ob_refcnt, (x))
#else /* __INTELLISENSE__ */
#define Dee_REQUIRES_OBJECT(x) (x)
#endif /* !__INTELLISENSE__ */


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
/* We add +1 for all statically initialized objects,
 * so we can easily add them to deemon's builtin module. */
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

#ifdef DEE_SOURCE
#define OBJECT_HEAD       Dee_OBJECT_HEAD
#define OBJECT_HEAD_EX    Dee_OBJECT_HEAD_EX
#define OBJECT_HEAD_INIT  Dee_OBJECT_HEAD_INIT
#endif /* DEE_SOURCE */


/* Check if the given object is being shared.
 * WARNING: The returned value cannot be relied upon for
 *          objects implementing the WEAKREF interface. */
#define DeeObject_IsShared(self) ((self)->ob_refcnt != 1)

#ifdef CONFIG_TRACE_REFCHANGES
#define DeeObject_DATA(self)            ((void *)(&(self)->ob_trace + 1))
#define DeeObject_InitNoref(self, type) ((self)->ob_refcnt = 1, (self)->ob_type = (type), (self)->ob_trace = NULL)
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_DATA(self)            ((void *)(&(self)->ob_type + 1))
#define DeeObject_InitNoref(self, type) ((self)->ob_refcnt = 1, (self)->ob_type = (type))
#endif /* !CONFIG_TRACE_REFCHANGES */

#define DeeObject_Init(self, type)    \
	(Dee_Incref((DeeObject *)(type)), \
	 DeeObject_InitNoref(self, type))


#ifndef NDEBUG
#define DeeObject_DoCheck(ob) \
	((ob)->ob_refcnt &&       \
	 (ob)->ob_type &&         \
	 ((DeeObject *)(ob)->ob_type)->ob_refcnt)
#define DeeObject_Check(ob) \
	((ob) && DeeObject_DoCheck(ob))
#define Dee_ASSERT_OBJECT(ob)                      (DeeObject_Check(ob) || (DeeAssert_BadObject(__FILE__, __LINE__, (DeeObject *)(ob)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_OPT(ob)                  (!(ob) || DeeObject_DoCheck(ob) || (DeeAssert_BadObjectOpt(__FILE__, __LINE__, (DeeObject *)(ob)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE(ob, type)           ((DeeObject_Check(ob) && DeeObject_InstanceOf(ob, type)) || (DeeAssert_BadObjectType(__FILE__, __LINE__, (DeeObject *)(ob), (DeeTypeObject *)(type)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_OPT(ob, type)       (!(ob) || (DeeObject_DoCheck(ob) && DeeObject_InstanceOf(ob, type)) || (DeeAssert_BadObjectTypeOpt(__FILE__, __LINE__, (DeeObject *)(ob), (DeeTypeObject *)(type)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)         ((DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type))) || (DeeAssert_BadObjectType(__FILE__, __LINE__, (DeeObject *)(ob), (DeeTypeObject *)(type)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)     (!(ob) || (DeeObject_DoCheck(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type))) || (DeeAssert_BadObjectTypeOpt(__FILE__, __LINE__, (DeeObject *)(ob), (DeeTypeObject *)(type)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_EXACT(ob, type)     ((DeeObject_Check(ob) && DeeObject_InstanceOfExact(ob, type)) || (DeeAssert_BadObjectTypeExact(__FILE__, __LINE__, (DeeObject *)(ob), (DeeTypeObject *)(type)), BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT(ob, type) (!(ob) || (DeeObject_DoCheck(ob) && DeeObject_InstanceOfExact(ob, type)) || (DeeAssert_BadObjectTypeExactOpt(__FILE__, __LINE__, (DeeObject *)(ob), (DeeTypeObject *)(type)), BREAKPOINT(), 0))
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObject(char const *file, int line, DeeObject const *ob);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectOpt(char const *file, int line, DeeObject const *ob);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectType(char const *file, int line, DeeObject const *ob, DeeTypeObject const *wanted_type);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeOpt(char const *file, int line, DeeObject const *ob, DeeTypeObject const *wanted_type);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeExact(char const *file, int line, DeeObject const *ob, DeeTypeObject const *wanted_type);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeExactOpt(char const *file, int line, DeeObject const *ob, DeeTypeObject const *wanted_type);
#else /* !NDEBUG */
#define DeeObject_DoCheck(ob)                      true
#define DeeObject_Check(ob)                        true
#define Dee_ASSERT_OBJECT(ob)                      (void)0
#define Dee_ASSERT_OBJECT_OPT(ob)                  (void)0
#define Dee_ASSERT_OBJECT_TYPE(ob, type)           (void)0
#define Dee_ASSERT_OBJECT_TYPE_OPT(ob, type)       (void)0
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)         (void)0
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)     (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT(ob, type)     (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT(ob, type) (void)0
#endif /* NDEBUG */

#ifdef DEE_SOURCE
#define ASSERT_OBJECT                Dee_ASSERT_OBJECT
#define ASSERT_OBJECT_OPT            Dee_ASSERT_OBJECT_OPT
#define ASSERT_OBJECT_TYPE           Dee_ASSERT_OBJECT_TYPE
#define ASSERT_OBJECT_TYPE_OPT       Dee_ASSERT_OBJECT_TYPE_OPT
#define ASSERT_OBJECT_TYPE_A         Dee_ASSERT_OBJECT_TYPE_A
#define ASSERT_OBJECT_TYPE_A_OPT     Dee_ASSERT_OBJECT_TYPE_A_OPT
#define ASSERT_OBJECT_TYPE_EXACT     Dee_ASSERT_OBJECT_TYPE_EXACT
#define ASSERT_OBJECT_TYPE_EXACT_OPT Dee_ASSERT_OBJECT_TYPE_EXACT_OPT
#endif /* DEE_SOURCE */



/* Prototype for callbacks to-be invoked when a weakref'd object gets destroyed. */
typedef NONNULL((1)) void (DCALL *Dee_weakref_callback_t)(struct Dee_weakref *__restrict self);


/* Object weak reference tracing. */
struct Dee_weakref {
	struct Dee_weakref   **wr_pself; /* [0..1][lock(BIT0(wr_next))][valid_if(wr_pself != NULL)] Indirect self pointer. */
	struct Dee_weakref    *wr_next;  /* [0..1][lock(BIT0(wr_next))][valid_if(wr_pself != NULL)][ORDER(BEFORE(*wr_pself))] Next weak references. */
	DeeObject             *wr_obj;   /* [0..1][lock(BIT0(wr_next))] Pointed-to object. */
	Dee_weakref_callback_t wr_del;   /* [0..1][const]
	                                  * An optional callback that is invoked when the bound object
	                                  * `wr_obj' gets destroyed, causing the weakref to become unbound.
	                                  * NOTE: If set, this callback _MUST_ invoke `DeeWeakref_UnlockCallback()'
	                                  *       in order to unlock the passed `struct Dee_weakref', after it has
	                                  *       acquired shared ownership to a containing object if it intends
	                                  *       to invoke arbitrary user-code, or drop references. */
};
#define Dee_WEAKREF_INIT { NULL, NULL, NULL, NULL }
#ifdef __cplusplus
#define Dee_WEAKREF(T)     struct ::Dee_weakref
#else /* __cplusplus */
#define Dee_WEAKREF(T)     struct Dee_weakref
#endif /* !__cplusplus */


#ifdef DEE_SOURCE
#define WEAKREF_INIT Dee_WEAKREF_INIT
#define WEAKREF      Dee_WEAKREF
#endif /* DEE_SOURCE */

/* The used NULL-pointer value for user-defined weakref callbacks.
 * It is only important that bit#0 be clear during assignment, however
 * a value distinct from NULL (or rather: 0) can be used to make it
 * easier to detect invalid uses of weakref controllers. */
#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
#define _DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	(__UINT32_C(0xcccccccc) & ~__UINT32_C(1))
#elif __SIZEOF_POINTER__ == 8
#define _DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	(__UINT64_C(0xcccccccccccccccc) & ~__UINT64_C(1))
#else /* ... */
#define _DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	((uintptr_t)-1 & ~(uintptr_t)1)
#endif /* !... */
#else /* !NDEBUG */
#define _DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	0
#endif /* NDEBUG */


/* Unlock a weakref from within a `wr_del' callback.
 * An invocation of this macro is _MANDATORY_ for any custom weakref
 * callback, as it is part of the synchronization process used to prevent
 * race conditions when working with weakref callbacks.
 * A simple weakref callback that invokes another user-code function could
 * then look like this:
 * >> typedef struct {
 * >>     Dee_OBJECT_HEAD
 * >>     struct Dee_weakref o_ref; // Uses `my_callback'
 * >>     DREF DeeObject    *o_fun; // 1..1
 * >> } MyObject;
 * >>
 * >> PRIVATE void DCALL my_callback(struct Dee_weakref *__restrict self) {
 * >>     DREF MyObject *me;
 * >>     me = COMPILER_CONTAINER_OF(self, MyObject, o_ref);
 * >>     if (!Dee_IncrefIfNotZero(me)) {
 * >>         // Race condition: the weakref controller died while the
 * >>         //                 weakref'd object was being destroyed.
 * >>         DeeWeakref_UnlockCallback(self);
 * >>     } else {
 * >>         DREF MyObject *result;
 * >>         DeeWeakref_UnlockCallback(self);
 * >>         // At this point, we've unlocked the weakref after safely acquiring
 * >>         // a reference to the controlling object, meaning we're now safe to
 * >>         // execute arbitrary code, with the only caveat being that exceptions
 * >>         // can't be propagated.
 * >>         result = DeeObject_Call(me->o_fun, 0, NULL);
 * >>         Dee_Decref(me);
 * >>         if unlikely(!result) {
 * >>             DeeError_Print("Unhandled exception in weakref callback",
 * >>                            ERROR_PRINT_DOHANDLE);
 * >>         } else {
 * >>             Dee_Decref(result);
 * >>         }
 * >>     }
 * >> }
 */
#ifdef __cplusplus
#define DeeWeakref_UnlockCallback(x) \
	__hybrid_atomic_store((x)->wr_next, (struct ::Dee_weakref *)_DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR, __ATOMIC_RELEASE)
#else /* __cplusplus */
#define DeeWeakref_UnlockCallback(x) \
	__hybrid_atomic_store((x)->wr_next, (struct Dee_weakref *)_DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR, __ATOMIC_RELEASE)
#endif /* !__cplusplus */



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
#define Dee_weakref_support_fini(x)                                    \
	(__hybrid_atomic_load((x)->ob_weakrefs.wl_nodes, __ATOMIC_ACQUIRE) \
	 ? Dee_weakref_support_fini(&(x)->ob_weakrefs)                     \
	 : (void)0)
DFUNDEF NONNULL((1)) void (DCALL Dee_weakref_support_fini)(struct Dee_weakref_list *__restrict self);

#ifdef DEE_SOURCE
#define WEAKREF_SUPPORT      Dee_WEAKREF_SUPPORT
#define WEAKREF_SUPPORT_ADDR Dee_WEAKREF_SUPPORT_ADDR
#define WEAKREF_SUPPORT_INIT Dee_WEAKREF_SUPPORT_INIT
#define weakref_support_init Dee_weakref_support_init
#define weakref_support_fini Dee_weakref_support_fini
#endif /* DEE_SOURCE */


/* Initialize the given weak reference to NULL. */
#define Dee_weakref_null(self) \
	((self)->wr_obj = NULL, (self)->wr_del = NULL)

/* Weak reference functionality.
 * @assume(ob != NULL);
 * @return: true:  Successfully initialized the given weak reference.
 * @return: false: The given object `ob' does not support weak referencing. */
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1, 2)) bool DCALL
Dee_weakref_init(struct Dee_weakref *__restrict self,
                 DeeObject *__restrict ob,
                 Dee_weakref_callback_t callback);
#else /* __INTELLISENSE__ */
#define Dee_weakref_init(self, ob, callback) \
	((self)->wr_del = (callback), (Dee_weakref_init)(self, ob))
DFUNDEF NONNULL((1, 2)) bool
(DCALL Dee_weakref_init)(struct Dee_weakref *__restrict self,
                         DeeObject *__restrict ob);
#endif /* !__INTELLISENSE__ */

/* Finalize a given weak reference. */
DFUNDEF NONNULL((1)) void DCALL Dee_weakref_fini(struct Dee_weakref *__restrict self);

/* Move/Copy a given weak reference into another, optionally
 * overwriting whatever object was referenced before.
 * NOTE: Assignment here does _NOT_ override a set deletion callback! */
DFUNDEF NONNULL((1, 2)) void DCALL Dee_weakref_move(struct Dee_weakref *__restrict dst, struct Dee_weakref *__restrict src);
DFUNDEF NONNULL((1, 2)) void DCALL Dee_weakref_moveassign(struct Dee_weakref *__restrict dst, struct Dee_weakref *__restrict src);
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self, struct Dee_weakref const *__restrict other);
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copyassign)(struct Dee_weakref *__restrict self, struct Dee_weakref const *__restrict other);
#else /* __INTELLISENSE__ */
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self, struct Dee_weakref *__restrict other);
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copyassign)(struct Dee_weakref *__restrict self, struct Dee_weakref *__restrict other);
#ifdef __cplusplus
#define Dee_weakref_copy(self, other) Dee_weakref_copy(self, (struct ::Dee_weakref *)(other))
#define Dee_weakref_copyassign(self, other) Dee_weakref_copyassign(self, (struct ::Dee_weakref *)(other))
#else /* __cplusplus */
#define Dee_weakref_copy(self, other) Dee_weakref_copy(self, (struct Dee_weakref *)(other))
#define Dee_weakref_copyassign(self, other) Dee_weakref_copyassign(self, (struct Dee_weakref *)(other))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Overwrite an already initialize weak reference with the given `ob'.
 * @return: true:    Successfully overwritten the weak reference.
 * @return: false:   The given object `ob' does not support weak referencing
 *                   and the stored weak reference was not modified. */
DFUNDEF NONNULL((1, 2)) bool DCALL
Dee_weakref_set(struct Dee_weakref *__restrict self,
                DeeObject *__restrict ob);

/* Clear the weak reference `self', returning true if it used to point to an object.
 * NOTE: Upon success (return is `true'), the callback will not be
 *       executed for the previously bound object's destruction. */
DFUNDEF NONNULL((1)) bool DCALL
Dee_weakref_clear(struct Dee_weakref *__restrict self);

/* Lock a weak reference, returning a regular reference to the pointed-to object.
 * @return: * :   A new reference to the pointed-to object.
 * @return: NULL: Failed to lock the weak reference. */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL Dee_weakref_lock)(struct Dee_weakref const *__restrict self);
#else /* __INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL Dee_weakref_lock)(struct Dee_weakref *__restrict self);
#ifdef __cplusplus
#define Dee_weakref_lock(self) Dee_weakref_lock((struct ::Dee_weakref *)(self))
#else /* __cplusplus */
#define Dee_weakref_lock(self) Dee_weakref_lock((struct Dee_weakref *)(self))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Return the state of a snapshot of `self' currently being bound. */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_weakref_bound)(struct Dee_weakref const *__restrict self);
#else /* __INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_weakref_bound)(struct Dee_weakref *__restrict self);
#ifdef __cplusplus
#define Dee_weakref_bound(self) Dee_weakref_bound((struct ::Dee_weakref *)(self))
#else /* __cplusplus */
#define Dee_weakref_bound(self) Dee_weakref_bound((struct Dee_weakref *)(self))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Do an atomic compare-exchange operation on the weak reference
 * and return a reference to the previously assigned object, or
 * `NULL' when none was assigned, or `Dee_ITER_DONE' when `new_ob'
 * does not support weak referencing functionality (in which case
 * the actual pointed-to weak object of `self' isn't changed).
 * NOTE: You may pass `NULL' for `new_ob' to clear the the weakref. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_weakref_cmpxch)(struct Dee_weakref *__restrict self,
                           DeeObject *old_ob, DeeObject *new_ob);


/* Type visit helpers.
 * WARNING: These helper macros are allowed to evaluate their arguments multiple times! */
typedef void (DCALL *Dee_visit_t)(DeeObject *__restrict self, void *arg);
#define Dee_Visit(ob)  (*proc)((DeeObject *)Dee_REQUIRES_OBJECT(ob), arg)
#define Dee_XVisit(ob) (!(ob) || (Dee_Visit(ob), 0))
#ifdef DEE_SOURCE
typedef Dee_visit_t  dvisit_t;
#endif /* DEE_SOURCE */

#define Dee_Visitv(object_vector, object_count)           \
	do {                                                  \
		size_t _dvv_i, _count = (object_count);           \
		for (_dvv_i = 0; _dvv_i < _count; ++_dvv_i) {     \
			DeeObject *_dvv_ob = (object_vector)[_dvv_i]; \
			Dee_Visit(_dvv_ob);                           \
		}                                                 \
	} __WHILE0
#define Dee_XVisitv(object_vector, object_count)          \
	do {                                                  \
		size_t _dvv_i, _count = (object_count);           \
		for (_dvv_i = 0; _dvv_i < _count; ++_dvv_i) {     \
			DeeObject *_dvv_ob = (object_vector)[_dvv_i]; \
			Dee_XVisit(_dvv_ob);                          \
		}                                                 \
	} __WHILE0


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
	__hybrid_atomic_cmpxch((self)->ob_refcnt, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)


/* incref() + return `self' */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeObject_NewRef(DeeObject *__restrict self);
#if defined(CONFIG_NO_BADREFCNT_CHECKS) && !defined(CONFIG_TRACE_REFCHANGES)
DFUNDEF NONNULL((1)) void DCALL DeeObject_Destroy(DeeObject *__restrict self);
#else /* CONFIG_NO_BADREFCNT_CHECKS && !CONFIG_TRACE_REFCHANGES */
DFUNDEF NONNULL((1)) void DCALL DeeObject_Destroy_d(DeeObject *__restrict self, char const *file, int line);
#define DeeObject_Destroy(self) DeeObject_Destroy_d(self, __FILE__, __LINE__)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS || CONFIG_TRACE_REFCHANGES */

/* Reference control macros as functions.
 * Use these (#undef'ing the macros, or like `(Dee_Incref)(foo)') in dex
 * modules that should work independently of the deemon configuration. */
DFUNDEF NONNULL((1)) void (DCALL Dee_Incref)(DeeObject *__restrict ob);
DFUNDEF NONNULL((1)) void (DCALL Dee_Incref_n)(DeeObject *__restrict ob, Dee_refcnt_t n);
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_IncrefIfNotZero)(DeeObject *__restrict ob);
DFUNDEF NONNULL((1)) void (DCALL Dee_Decref)(DeeObject *__restrict ob);
DFUNDEF NONNULL((1)) void (DCALL Dee_DecrefDokill)(DeeObject *__restrict ob);
DFUNDEF NONNULL((1)) void (DCALL Dee_DecrefNokill)(DeeObject *__restrict ob);
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_DecrefIfOne)(DeeObject *__restrict ob);
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_DecrefIfNotOne)(DeeObject *__restrict ob);
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_DecrefWasOk)(DeeObject *__restrict ob);

#ifdef __INTELLISENSE__
#define Dee_Incref_untraced(x)          (void)(&(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     (void)(((x)->ob_refcnt += (n)))
#define Dee_Decref_untraced(x)          (void)(&(x)->ob_refcnt)
#define Dee_Decref_likely_untraced(x)   (void)(&(x)->ob_refcnt)
#define Dee_Decref_unlikely_untraced(x) (void)(&(x)->ob_refcnt)
#define Dee_IncrefIfNotZero_untraced(x) (Dee_Incref(x), true)
#define Dee_DecrefDokill_untraced(x)    (void)(&(x)->ob_refcnt)
#define Dee_DecrefNokill_untraced(x)    (void)(&(x)->ob_refcnt)
#define Dee_DecrefIfOne_untraced(x)     (Dee_Decref(x), true)
#define Dee_DecrefIfNotOne_untraced(x)  (Dee_Decref(x), true)
#define Dee_DecrefWasOk_untraced(x)     (Dee_Decref(x), true)
#else /* __INTELLISENSE__ */
#ifndef CONFIG_NO_BADREFCNT_CHECKS
DFUNDEF NONNULL((1)) void DCALL DeeFatal_BadIncref(DeeObject *ob, char const *file, int line);
DFUNDEF NONNULL((1)) void DCALL DeeFatal_BadDecref(DeeObject *ob, char const *file, int line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
#ifdef CONFIG_NO_THREADS
#define _DeeRefcnt_FetchInc(x)    ((x)++)
#define _DeeRefcnt_FetchDec(x)    ((x)--)
#define _DeeRefcnt_IncFetch(x)    (++(x))
#define _DeeRefcnt_DecFetch(x)    (--(x))
#define _DeeRefcnt_AddFetch(x, n) ((x) += (n))
#define _DeeRefcnt_FetchAdd(x, n) (((x) += (n)) - (n))
#ifndef CONFIG_NO_BADREFCNT_CHECKS
#define Dee_Incref_untraced(x)          (void)((x)->ob_refcnt++ || (DeeFatal_BadIncref((DeeObject *)(x), __FILE__, __LINE__), 0))
#define Dee_Incref_n_untraced(x, n)     (void)(_DeeRefcnt_FetchAdd((x)->ob_refcnt, n) || (DeeFatal_BadIncref((DeeObject *)(x), __FILE__, __LINE__), 0))
#define Dee_Decref_untraced(x)          (void)((x)->ob_refcnt-- > 1 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely((x)->ob_refcnt-- > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely((x)->ob_refcnt-- > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    (void)(--(x)->ob_refcnt, DeeObject_Destroy((DeeObject *)(x)))
#define Dee_DecrefNokill_untraced(x)    (void)((x)->ob_refcnt-- > 1 || (DeeFatal_BadDecref((DeeObject *)(x)), 0))
#define Dee_DecrefWasOk_untraced(x)     ((x)->ob_refcnt-- > 1 ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_Incref_untraced(x)          (void)(++(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     (void)((x)->ob_refcnt += (n))
#define Dee_Decref_untraced(x)          (void)(--(x)->ob_refcnt || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(--(x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(--(x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    (void)(DeeObject_Destroy((DeeObject *)(x)))
#define Dee_DecrefNokill_untraced(x)    (void)(--(x)->ob_refcnt)
#define Dee_DecrefWasOk_untraced(x)     (--(x)->ob_refcnt ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_DecrefIfOne_untraced(x)     ((x)->ob_refcnt == 1 ? ((x)->ob_refcnt = 0, DeeObject_Destroy((DeeObject *)(x)), true) : false)
#define Dee_DecrefIfNotOne_untraced(x)  ((x)->ob_refcnt > 1 ? (--(x)->ob_refcnt, true) : false)
#define Dee_IncrefIfNotZero_untraced(x) ((x)->ob_refcnt ? (++(x)->ob_refcnt, true) : false)
#else /* CONFIG_NO_THREADS */
#if defined(_MSC_VER) && defined(CONFIG_HOST_WINDOWS)
/* NOTE: The atomic functions from hybrid would work for this case just as well,  but
 *       they have a rather large compile-time overhead and add a lot of unnecessary
 *       debug information when the resulting code isn't getting optimized.
 *       Therefor, try to bypass them here to speed up compile-time and ease debugging. */
#if __SIZEOF_POINTER__ == 4 && __SIZEOF_LONG__ == 4
#   define _DeeRefcnt_FetchInc(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedIncrement((long volatile *)&(x)) - 1)
#   define _DeeRefcnt_FetchDec(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedDecrement((long volatile *)&(x)) + 1)
#   define _DeeRefcnt_IncFetch(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedIncrement((long volatile *)&(x)))
#   define _DeeRefcnt_DecFetch(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedDecrement((long volatile *)&(x)))
#elif __SIZEOF_POINTER__ == 8
#   define _DeeRefcnt_FetchInc(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedIncrement64((__int64 volatile *)&(x)) - 1)
#   define _DeeRefcnt_FetchDec(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedDecrement64((__int64 volatile *)&(x)) + 1)
#   define _DeeRefcnt_IncFetch(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedIncrement64((__int64 volatile *)&(x)))
#   define _DeeRefcnt_DecFetch(x) ((Dee_refcnt_t)__NAMESPACE_INT_SYM _InterlockedDecrement64((__int64 volatile *)&(x)))
#endif
#endif /* _MSC_VER && CONFIG_HOST_WINDOWS */
#ifndef _DeeRefcnt_FetchInc
#define _DeeRefcnt_FetchInc(x) __hybrid_atomic_fetchinc(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_FetchDec(x) __hybrid_atomic_fetchdec(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_IncFetch(x) __hybrid_atomic_incfetch(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_DecFetch(x) __hybrid_atomic_decfetch(x, __ATOMIC_SEQ_CST)
#endif /* !_DeeRefcnt_FetchInc */
#ifndef _DeeRefcnt_FetchAdd
#define _DeeRefcnt_FetchAdd(x, n) __hybrid_atomic_fetchadd(x, n, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_AddFetch(x, n) __hybrid_atomic_addfetch(x, n, __ATOMIC_SEQ_CST)
#endif /* !_DeeRefcnt_FetchAdd */
#ifndef CONFIG_NO_BADREFCNT_CHECKS
#define Dee_Incref_untraced(x)          (void)(_DeeRefcnt_FetchInc((x)->ob_refcnt) || (DeeFatal_BadIncref((DeeObject *)(x), __FILE__, __LINE__), 0))
#define Dee_Incref_n_untraced(x, n)     (void)(_DeeRefcnt_FetchAdd((x)->ob_refcnt, n) || (DeeFatal_BadIncref((DeeObject *)(x), __FILE__, __LINE__), 0))
#define Dee_Decref_untraced(x)          (void)(_DeeRefcnt_FetchDec((x)->ob_refcnt) > 1 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(_DeeRefcnt_FetchDec((x)->ob_refcnt) > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(_DeeRefcnt_FetchDec((x)->ob_refcnt) > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    (_DeeRefcnt_FetchDec((x)->ob_refcnt), DeeObject_Destroy((DeeObject *)(x)))
#define Dee_DecrefNokill_untraced(x)    (void)(_DeeRefcnt_FetchDec((x)->ob_refcnt) > 1 || (DeeFatal_BadDecref((DeeObject *)(x), __FILE__, __LINE__), 0))
#define Dee_DecrefWasOk_untraced(x)     (_DeeRefcnt_FetchDec((x)->ob_refcnt) > 1 ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#define Dee_DecrefIfOne_untraced(self)  Dee_DecrefIfOne_untraced_d((DeeObject *)(self), __FILE__, __LINE__)
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_Incref_untraced(x)          (void)_DeeRefcnt_FetchInc((x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     (void)_DeeRefcnt_AddFetch((x)->ob_refcnt, n)
#define Dee_Decref_untraced(x)          (void)(_DeeRefcnt_DecFetch((x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(_DeeRefcnt_DecFetch((x)->ob_refcnt)) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(_DeeRefcnt_DecFetch((x)->ob_refcnt)) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    DeeObject_Destroy((DeeObject *)(x))
#define Dee_DecrefNokill_untraced(x)    (void)_DeeRefcnt_DecFetch((x)->ob_refcnt)
#define Dee_DecrefWasOk_untraced(x)     (_DeeRefcnt_DecFetch((x)->ob_refcnt) ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#define Dee_DecrefIfOne_untraced(self)  Dee_DecrefIfOne_untraced((DeeObject *)(self))
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_DecrefIfNotOne_untraced(self)  Dee_DecrefIfNotOne_untraced((DeeObject *)(self))
#define Dee_IncrefIfNotZero_untraced(self) Dee_IncrefIfNotZero_untraced((DeeObject *)(self))

#ifndef __OPTIMIZE_SIZE__
#define DeeObject_NewRef_untraced(self) \
	DeeObject_NewRef_untraced_inline(self)
LOCAL NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewRef_untraced_inline(DeeObject *__restrict self) {
	Dee_Incref_untraced(self);
	return self;
}
#endif /* !__OPTIMIZE_SIZE__ */

LOCAL WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfNotOne_untraced)(DeeObject *__restrict self) {
	Dee_refcnt_t refcnt;
	do {
		refcnt = __hybrid_atomic_load(self->ob_refcnt, __ATOMIC_ACQUIRE);
		if (refcnt <= 1)
			return false;
	} while (!__hybrid_atomic_cmpxch_weak(self->ob_refcnt, refcnt, refcnt - 1,
	                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
	return true;
}

LOCAL WUNUSED NONNULL((1)) bool
(DCALL Dee_IncrefIfNotZero_untraced)(DeeObject *__restrict self) {
	Dee_refcnt_t refcnt;
	do {
		refcnt = __hybrid_atomic_load(self->ob_refcnt, __ATOMIC_ACQUIRE);
		if (!refcnt)
			return false;
	} while (!__hybrid_atomic_cmpxch_weak(self->ob_refcnt, refcnt, refcnt + 1,
	                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
	return true;
}

#ifndef __INTELLISENSE__
#ifndef CONFIG_NO_BADREFCNT_CHECKS
LOCAL WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne_untraced_d)(DeeObject *__restrict self,
                                   char const *file, int line) {
	if (!__hybrid_atomic_cmpxch(self->ob_refcnt, 1, 0,
	                            __ATOMIC_SEQ_CST,
	                            __ATOMIC_SEQ_CST))
		return false;
	DeeObject_Destroy_d(self, file, line);
	return true;
}
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
LOCAL WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne_untraced)(DeeObject *__restrict self) {
	if (!__hybrid_atomic_cmpxch(self->ob_refcnt, 1, 0,
	                            __ATOMIC_SEQ_CST,
	                            __ATOMIC_SEQ_CST))
		return false;
	DeeObject_Destroy(self);
	return true;
}
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
#endif /* !__INTELLISENSE__ */
#endif /* !CONFIG_NO_THREADS */
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_TRACE_REFCHANGES
DFUNDEF NONNULL((1)) void DCALL Dee_Incref_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF NONNULL((1)) void DCALL Dee_Incref_n_traced(DeeObject *__restrict ob, Dee_refcnt_t n, char const *file, int line);
DFUNDEF WUNUSED NONNULL((1)) bool DCALL Dee_IncrefIfNotZero_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF NONNULL((1)) void DCALL Dee_Decref_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF NONNULL((1)) void DCALL Dee_DecrefDokill_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF NONNULL((1)) void DCALL Dee_DecrefNokill_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF WUNUSED NONNULL((1)) bool DCALL Dee_DecrefIfOne_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF WUNUSED NONNULL((1)) bool DCALL Dee_DecrefIfNotOne_traced(DeeObject *__restrict ob, char const *file, int line);
DFUNDEF WUNUSED NONNULL((1)) bool DCALL Dee_DecrefWasOk_traced(DeeObject *__restrict ob, char const *file, int line);
#define Dee_Decref_likely_traced(ob, file, line)   Dee_Decref_traced(ob, file, line)
#define Dee_Decref_unlikely_traced(ob, file, line) Dee_Decref_traced(ob, file, line)
#define Dee_Incref(x)           Dee_Incref_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_Incref_n(x, n)      Dee_Incref_n_traced((DeeObject *)(x), n, __FILE__, __LINE__)
#define Dee_IncrefIfNotZero(x)  Dee_IncrefIfNotZero_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_Decref(x)           Dee_Decref_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_Decref_likely(x)    Dee_Decref_likely_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_Decref_unlikely(x)  Dee_Decref_unlikely_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefDokill(x)     Dee_DecrefDokill_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefNokill(x)     Dee_DecrefNokill_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefIfOne(x)      Dee_DecrefIfOne_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefIfNotOne(x)   Dee_DecrefIfNotOne_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefWasOk(x)      Dee_DecrefWasOk_traced((DeeObject *)(x), __FILE__, __LINE__)
#else /* CONFIG_TRACE_REFCHANGES */
#define Dee_Incref_traced(ob, file, line)          Dee_Incref_untraced(x)
#define Dee_Incref_n_traced(ob, n, file, line)     Dee_Incref_n_untraced(x, n)
#define Dee_IncrefIfNotZero_traced(ob, file, line) Dee_IncrefIfNotZero_untraced(x)
#define Dee_Decref_traced(ob, file, line)          Dee_Decref_untraced(x)
#define Dee_Decref_likely_traced(ob, file, line)   Dee_Decref_likely_untraced(x)
#define Dee_Decref_unlikely_traced(ob, file, line) Dee_Decref_unlikely_untraced(x)
#define Dee_DecrefDokill_traced(ob, file, line)    Dee_DecrefDokill_untraced(x)
#define Dee_DecrefNokill_traced(ob, file, line)    Dee_DecrefNokill_untraced(x)
#define Dee_DecrefIfOne_traced(ob, file, line)     Dee_DecrefIfOne_untraced(x)
#define Dee_DecrefIfNotOne_traced(ob, file, line)  Dee_DecrefIfNotOne_untraced(x)
#define Dee_DecrefWasOk_traced(ob, file, line)     Dee_DecrefWasOk_untraced(x)
#define Dee_Incref(x)                              Dee_Incref_untraced(x)
#define Dee_Incref_n(x, n)                         Dee_Incref_n_untraced(x, n)
#define Dee_IncrefIfNotZero(x)                     Dee_IncrefIfNotZero_untraced(x)
#define Dee_Decref(x)                              Dee_Decref_untraced(x)
#define Dee_Decref_likely(x)                       Dee_Decref_likely_untraced(x)
#define Dee_Decref_unlikely(x)                     Dee_Decref_unlikely_untraced(x)
#define Dee_DecrefDokill(x)                        Dee_DecrefDokill_untraced(x)
#define Dee_DecrefNokill(x)                        Dee_DecrefNokill_untraced(x)
#define Dee_DecrefIfOne(x)                         Dee_DecrefIfOne_untraced(x)
#define Dee_DecrefIfNotOne(x)                      Dee_DecrefIfNotOne_untraced(x)
#define Dee_DecrefWasOk(x)                         Dee_DecrefWasOk_untraced(x)
#endif /* !CONFIG_TRACE_REFCHANGES */

/* Optimization hints when the object actually
 * being destroyed is likely/unlikely. */
#ifndef Dee_Decref_likely
#define Dee_Decref_likely(x)    Dee_Decref(x)
#endif /* !Dee_Decref_likely */
#ifndef Dee_Decref_unlikely
#define Dee_Decref_unlikely(x)  Dee_Decref(x)
#endif /* !Dee_Decref_unlikely */

#define Dee_XIncref(x)          (void)(!(x) || (Dee_Incref(x), 0))
#define Dee_XDecref(x)          (void)(!(x) || (Dee_Decref(x), 0))
#define Dee_XDecref_likely(x)   (void)(!(x) || (Dee_Decref_likely(x), 0))
#define Dee_XDecref_unlikely(x) (void)(!(x) || (Dee_Decref_unlikely(x), 0))
#define Dee_XDecrefNokill(x)    (void)(!(x) || (Dee_DecrefNokill(x), 0))
#define Dee_Clear(x)            (void)(Dee_Decref(x), (x) = NULL)
#define Dee_Clear_likely(x)     (void)(Dee_Decref_likely(x), (x) = NULL)
#define Dee_Clear_unlikely(x)   (void)(Dee_Decref_unlikely(x), (x) = NULL)
#define Dee_XClear(x)           (void)(!(x) || (Dee_Decref(x), (x) = NULL, 0))
#define Dee_XClear_likely(x)    (void)(!(x) || (Dee_Decref_likely(x), (x) = NULL, 0))
#define Dee_XClear_unlikely(x)  (void)(!(x) || (Dee_Decref_unlikely(x), (x) = NULL, 0))

#define Dee_XIncref_untraced(x)          (void)(!(x) || (Dee_Incref_untraced(x), 0))
#define Dee_XDecref_untraced(x)          (void)(!(x) || (Dee_Decref_untraced(x), 0))
#define Dee_XDecref_likely_untraced(x)   (void)(!(x) || (Dee_Decref_likely_untraced(x), 0))
#define Dee_XDecref_unlikely_untraced(x) (void)(!(x) || (Dee_Decref_unlikely_untraced(x), 0))
#define Dee_XDecrefNokill_untraced(x)    (void)(!(x) || (Dee_DecrefNokill_untraced(x), 0))
#define Dee_Clear_untraced(x)            (void)(Dee_Decref_untraced(x), (x) = NULL)
#define Dee_Clear_likely_untraced(x)     (void)(Dee_Decref_likely_untraced(x), (x) = NULL)
#define Dee_Clear_unlikely_untraced(x)   (void)(Dee_Decref_unlikely_untraced(x), (x) = NULL)
#define Dee_XClear_untraced(x)           (void)(!(x) || (Dee_Decref_untraced(x), (x) = NULL, 0))
#define Dee_XClear_likely_untraced(x)    (void)(!(x) || (Dee_Decref_likely_untraced(x), (x) = NULL, 0))
#define Dee_XClear_unlikely_untraced(x)  (void)(!(x) || (Dee_Decref_unlikely_untraced(x), (x) = NULL, 0))

#define Dee_XIncref_traced(x, file, line)          (void)(!(x) || (Dee_Incref_traced(x, file, line), 0))
#define Dee_XDecref_traced(x, file, line)          (void)(!(x) || (Dee_Decref_traced(x, file, line), 0))
#define Dee_XDecref_likely_traced(x, file, line)   (void)(!(x) || (Dee_Decref_likely_traced(x, file, line), 0))
#define Dee_XDecref_unlikely_traced(x, file, line) (void)(!(x) || (Dee_Decref_unlikely_traced(x, file, line), 0))
#define Dee_XDecrefNokill_traced(x, file, line)    (void)(!(x) || (Dee_DecrefNokill_traced(x, file, line), 0))
#define Dee_Clear_traced(x, file, line)            (void)(Dee_Decref_traced(x, file, line), (x) = NULL)
#define Dee_Clear_likely_traced(x, file, line)     (void)(Dee_Decref_likely_traced(x, file, line), (x) = NULL)
#define Dee_Clear_unlikely_traced(x, file, line)   (void)(Dee_Decref_unlikely_traced(x, file, line), (x) = NULL)
#define Dee_XClear_traced(x, file, line)           (void)(!(x) || (Dee_Decref_traced(x, file, line), (x) = NULL, 0))
#define Dee_XClear_likely_traced(x, file, line)    (void)(!(x) || (Dee_Decref_likely_traced(x, file, line), (x) = NULL, 0))
#define Dee_XClear_unlikely_traced(x, file, line)  (void)(!(x) || (Dee_Decref_unlikely_traced(x, file, line), (x) = NULL, 0))

/* NOTE: `(Dee_)return_reference()' only evaluates `ob' _once_! */
#define Dee_return_reference(ob)                                              \
	do {                                                                      \
		DeeObject *const _drr_result_ = (DeeObject *)Dee_REQUIRES_OBJECT(ob); \
		Dee_Incref(_drr_result_);                                             \
		return _drr_result_;                                                  \
	} __WHILE0

/* NOTE: `(Dee_)return_reference_()' may evaluate `ob' multiple times */
#define Dee_return_reference_(ob) \
	return (Dee_Incref(ob), ob)

#ifdef DEE_SOURCE
#define return_reference   Dee_return_reference
#define return_reference_  Dee_return_reference_
#endif /* DEE_SOURCE */




/* Increment the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DREF DeeObject **
(DCALL Dee_Increfv)(DeeObject *const *__restrict object_vector,
                    size_t object_count);

/* Decrement the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DeeObject **
(DCALL Dee_Decrefv)(DREF DeeObject *const *__restrict object_vector,
                    size_t object_count);

/* Copy object pointers from `src' to `dst' and increment
 * the reference counter of every object that got copied.
 * @return: * : Always re-returns the pointer to `dst' */
DFUNDEF ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject **
(DCALL Dee_Movrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *const *__restrict src,
                    size_t object_count);


LOCAL ATTR_RETNONNULL NONNULL((1)) DREF DeeObject **
(DCALL Dee_XIncrefv)(DeeObject * /*nullable*/ const *__restrict object_vector,
                     size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_XIncref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

LOCAL ATTR_RETNONNULL NONNULL((1)) DREF DeeObject **
(DCALL Dee_XDecrefv)(DeeObject * /*nullable*/ const *__restrict object_vector,
                     size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_XDecref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

LOCAL ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject **
(DCALL Dee_XMovrefv)(/*out:ref*/ DeeObject * /*nullable*/ *__restrict dst,
                     /*in*/ DeeObject * /*nullable*/ const *__restrict src,
                     size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = src[object_count];
		Dee_XIncref_untraced(ob);
		dst[object_count] = ob;
	}
	return dst;
}



#if (!defined(CONFIG_INLINE_INCREFV) && \
     !defined(CONFIG_NO_INLINE_INCREFV))
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_INLINE_INCREFV 1
#else /* !__OPTIMIZE_SIZE__ */
#define CONFIG_NO_INLINE_INCREFV 1
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !CONFIG_[NO_]INLINE_INCREFV */

#ifdef __INTELLISENSE__
#define Dee_Increfv_untraced(object_vector, object_count)  ((void)(object_count), (DeeObject **)&Dee_REQUIRES_OBJECT(*(object_vector)))
#define Dee_Decrefv_untraced(object_vector, object_count)  ((void)(object_count), (DeeObject **)&Dee_REQUIRES_OBJECT(*(object_vector)))
#define Dee_Movrefv_untraced(dst, src, object_count)       ((void)(object_count), Dee_REQUIRES_OBJECT(*(src)), (DeeObject **)&Dee_REQUIRES_OBJECT(*(dst)))
#elif defined(CONFIG_INLINE_INCREFV)
#define Dee_Increfv_untraced(object_vector, object_count) \
	Dee_Increfv_untraced((DeeObject **)(object_vector), object_count)
LOCAL ATTR_RETNONNULL NONNULL((1)) DREF DeeObject **
(DCALL Dee_Increfv_untraced)(DeeObject *const *__restrict object_vector,
                             size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Incref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

#define Dee_Decrefv_untraced(object_vector, object_count) \
	Dee_Decrefv_untraced((DREF DeeObject *const *)(object_vector), object_count)
LOCAL ATTR_RETNONNULL NONNULL((1)) DeeObject **
(DCALL Dee_Decrefv_untraced)(DREF DeeObject *const *__restrict object_vector,
                             size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Decref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

#define Dee_Movrefv_untraced(dst, src, object_count) \
	Dee_Movrefv_untraced((DeeObject **)(dst), (DeeObject *const *)(src), object_count)
LOCAL ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject **
(DCALL Dee_Movrefv_untraced)(/*out:ref*/ DeeObject **__restrict dst,
                             /*in*/ DeeObject *const *__restrict src,
                             size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = src[i];
		Dee_Incref_untraced(ob);
		dst[i] = ob;
	}
	return dst;
}
#else /* CONFIG_INLINE_INCREFV */
#define Dee_Increfv_untraced(object_vector, object_count) (Dee_Increfv)((DeeObject *const *)(object_vector), object_count)
#define Dee_Decrefv_untraced(object_vector, object_count) (Dee_Decrefv)((DeeObject *const *)(object_vector), object_count)
#define Dee_Movrefv_untraced(dst, src, object_count)      (Dee_Movrefv)((DeeObject **)(dst), (DeeObject *const *)(src), object_count)
#endif /* !CONFIG_INLINE_INCREFV */


#ifdef __INTELLISENSE__
#define Dee_XIncrefv_untraced(object_vector, object_count)  ((void)(object_count), (DeeObject **)&Dee_REQUIRES_OBJECT(*(object_vector)))
#define Dee_XDecrefv_untraced(object_vector, object_count)  ((void)(object_count), (DeeObject **)&Dee_REQUIRES_OBJECT(*(object_vector)))
#define Dee_XMovrefv_untraced(dst, src, object_count)       ((void)(object_count), Dee_REQUIRES_OBJECT(*(src)), (DeeObject **)&Dee_REQUIRES_OBJECT(*(dst)))
#else /* __INTELLISENSE__ */
#define Dee_XIncrefv_untraced(object_vector, object_count) (Dee_XIncrefv)((DeeObject *const *)(object_vector), object_count)
#define Dee_XDecrefv_untraced(object_vector, object_count) (Dee_XDecrefv)((DeeObject *const *)(object_vector), object_count)
#define Dee_XMovrefv_untraced(dst, src, object_count)      (Dee_XMovrefv)((DeeObject **)(dst), (DeeObject *const *)(src), object_count)
#endif /* !__INTELLISENSE__ */



#ifdef CONFIG_TRACE_REFCHANGES
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DREF DeeObject **(DCALL Dee_Increfv_traced)(DeeObject *const *__restrict object_vector, size_t object_count, char const *file, int line);
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DeeObject **(DCALL Dee_Decrefv_traced)(DREF DeeObject *const *__restrict object_vector, size_t object_count, char const *file, int line);
DFUNDEF ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject **(DCALL Dee_Movrefv_traced)(/*out:ref*/ DeeObject **__restrict dst, /*in*/ DeeObject *const *__restrict src, size_t object_count, char const *file, int line);
#define Dee_Increfv(object_vector, object_count) Dee_Increfv_traced((DeeObject *const *)(object_vector), object_count, __FILE__, __LINE__)
#define Dee_Decrefv(object_vector, object_count) Dee_Decrefv_traced((DeeObject *const *)(object_vector), object_count, __FILE__, __LINE__)
#define Dee_Movrefv(dst, src, object_count)      Dee_Movrefv_traced((DeeObject **)(dst), (DeeObject *const *)(src), object_count, __FILE__, __LINE__)

#define Dee_XIncrefv_traced(object_vector, object_count, file, line) \
	Dee_XIncrefv_traced((DeeObject **)(object_vector), object_count, file, line)
LOCAL ATTR_RETNONNULL NONNULL((1)) DREF DeeObject **
(DCALL Dee_XIncrefv_traced)(DeeObject *const *__restrict object_vector,
                            size_t object_count, char const *file, int line) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Incref_traced(ob, file, line);
	}
	return (DREF DeeObject **)object_vector;
}

#define Dee_XDecrefv_traced(object_vector, object_count, file, line) \
	Dee_XDecrefv_traced((DREF DeeObject *const *)(object_vector), object_count, file, line)
LOCAL ATTR_RETNONNULL NONNULL((1)) DeeObject **
(DCALL Dee_XDecrefv_traced)(DREF DeeObject *const *__restrict object_vector,
                             size_t object_count, char const *file, int line) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Decref_traced(ob, file, line);
	}
	return (DREF DeeObject **)object_vector;
}

#define Dee_XMovrefv_traced(dst, src, object_count, file, line) \
	Dee_XMovrefv_traced((DeeObject **)(dst), (DeeObject *const *)(src), object_count, file, line)
LOCAL ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject **
(DCALL Dee_XMovrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                             /*in*/ DeeObject *const *__restrict src,
                             size_t object_count, char const *file, int line) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = src[i];
		Dee_Incref_traced(ob, file, line);
		dst[i] = ob;
	}
	return dst;
}
#else /* CONFIG_TRACE_REFCHANGES */
#define Dee_Increfv_traced(object_vector, object_count, file, line)  Dee_Increfv_untraced(object_vector, object_count)
#define Dee_Decrefv_traced(object_vector, object_count, file, line)  Dee_Decrefv_untraced(object_vector, object_count)
#define Dee_Movrefv_traced(dst, src, object_count, file, line)       Dee_Movrefv_untraced(dst, src, object_count)
#define Dee_Increfv(object_vector, object_count)                     Dee_Increfv_untraced(object_vector, object_count)
#define Dee_Decrefv(object_vector, object_count)                     Dee_Decrefv_untraced(object_vector, object_count)
#define Dee_Movrefv(dst, src, object_count)                          Dee_Movrefv_untraced(dst, src, object_count)
#define Dee_XIncrefv_traced(object_vector, object_count, file, line) Dee_XIncrefv_untraced(object_vector, object_count)
#define Dee_XDecrefv_traced(object_vector, object_count, file, line) Dee_XDecrefv_untraced(object_vector, object_count)
#define Dee_XMovrefv_traced(dst, src, object_count, file, line)      Dee_XMovrefv_untraced(dst, src, object_count)
#define Dee_XIncrefv(object_vector, object_count)                    Dee_XIncrefv_untraced(object_vector, object_count)
#define Dee_XDecrefv(object_vector, object_count)                    Dee_XDecrefv_untraced(object_vector, object_count)
#define Dee_XMovrefv(dst, src, object_count)                         Dee_XMovrefv_untraced(dst, src, object_count)
#endif /* !CONFIG_TRACE_REFCHANGES */

/* The likely/unlikely annotations are currently ignored for these... */
#define Dee_Decrefv_likely(object_vector, object_count)                       Dee_Decrefv(object_vector, object_count)
#define Dee_Decrefv_unlikely(object_vector, object_count)                     Dee_Decrefv(object_vector, object_count)
#define Dee_Decrefv_untraced_likely(object_vector, object_count)              Dee_Decrefv_untraced(object_vector, object_count)
#define Dee_Decrefv_untraced_unlikely(object_vector, object_count)            Dee_Decrefv_untraced(object_vector, object_count)
#define Dee_Decrefv_traced_likely(object_vector, object_count, file, line)    Dee_Decrefv_traced(object_vector, object_count, file, line)
#define Dee_Decrefv_traced_unlikely(object_vector, object_count, file, line)  Dee_Decrefv_traced(object_vector, object_count, file, line)
#define Dee_XDecrefv_likely(object_vector, object_count)                      Dee_XDecrefv(object_vector, object_count)
#define Dee_XDecrefv_unlikely(object_vector, object_count)                    Dee_XDecrefv(object_vector, object_count)
#define Dee_XDecrefv_untraced_likely(object_vector, object_count)             Dee_XDecrefv_untraced(object_vector, object_count)
#define Dee_XDecrefv_untraced_unlikely(object_vector, object_count)           Dee_XDecrefv_untraced(object_vector, object_count)
#define Dee_XDecrefv_traced_likely(object_vector, object_count, file, line)   Dee_XDecrefv_traced(object_vector, object_count, file, line)
#define Dee_XDecrefv_traced_unlikely(object_vector, object_count, file, line) Dee_XDecrefv_traced(object_vector, object_count, file, line)

/* same as above, but return a pointer to the end of `object_vector' / `dst' */
#define Dee_Incprefv_untraced(object_vector, object_count)            (Dee_Increfv_untraced(object_vector, object_count) + (object_count))
#define Dee_Decprefv_untraced(object_vector, object_count)            (Dee_Decrefv_untraced(object_vector, object_count) + (object_count))
#define Dee_Movprefv_untraced(dst, src, object_count)                 (Dee_Movrefv_untraced(dst, src, object_count) + (object_count))
#define Dee_Incprefv_traced(object_vector, object_count, file, line)  (Dee_Increfv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_Decprefv_traced(object_vector, object_count, file, line)  (Dee_Decrefv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_Movprefv_traced(dst, src, object_count, file, line)       (Dee_Movrefv_traced(dst, src, object_count, file, line) + (object_count))
#define Dee_Incprefv(object_vector, object_count)                     (Dee_Increfv(object_vector, object_count) + (object_count))
#define Dee_Decprefv(object_vector, object_count)                     (Dee_Decrefv(object_vector, object_count) + (object_count))
#define Dee_Movprefv(dst, src, object_count)                          (Dee_Movrefv(dst, src, object_count) + (object_count))
#define Dee_XIncprefv_untraced(object_vector, object_count)           (Dee_XIncrefv_untraced(object_vector, object_count) + (object_count))
#define Dee_XDecprefv_untraced(object_vector, object_count)           (Dee_XDecrefv_untraced(object_vector, object_count) + (object_count))
#define Dee_XMovprefv_untraced(dst, src, object_count)                (Dee_XMovrefv_untraced(dst, src, object_count) + (object_count))
#define Dee_XIncprefv_traced(object_vector, object_count, file, line) (Dee_XIncrefv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_XDecprefv_traced(object_vector, object_count, file, line) (Dee_XDecrefv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_XMovprefv_traced(dst, src, object_count, file, line)      (Dee_XMovrefv_traced(dst, src, object_count, file, line) + (object_count))
#define Dee_XIncprefv(object_vector, object_count)                    (Dee_XIncrefv(object_vector, object_count) + (object_count))
#define Dee_XDecprefv(object_vector, object_count)                    (Dee_XDecrefv(object_vector, object_count) + (object_count))
#define Dee_XMovprefv(dst, src, object_count)                         (Dee_XMovrefv(dst, src, object_count) + (object_count))


/* Callback prototype for enumerating object attributes.
 * @param declarator: [1..1] The type or object that is declaring this attribute.
 * @param attr_name:  [1..1] The name of the attribute.
 * @param attr_doc:   [0..1] An optional documentation string containing additional information.
 * @param perm:       Set of `ATTR_*' describing permissions granted by the attribute.
 * @param attr_type:  The type of object that would be returned by `DeeObject_GetAttr', or `NULL' if unknown.
 * @param arg:        User-defined callback argument.
 * @return: < 0:      Propagate an error, letting `DeeObject_EnumAttr()' fail with the same error.
 * @return: >= 0:     Add this value to the sum of all other positive values, which `DeeObject_EnumAttr()' will then return.
 * @return: -1:       An error occurred and was thrown (This may also be returned by `DeeObject_EnumAttr()' when enumeration fails for some other reason)
 * WARNING: The callback must _NEVER_ be invoked while _ANY_ kind of lock is held! */
typedef WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL *Dee_enum_t)(DeeObject *__restrict declarator,
                    char const *__restrict attr_name, char const *attr_doc,
                    uint16_t perm, DeeTypeObject *attr_type, void *arg);
#define Dee_ATTR_PERMGET   0x0001 /* [NAME("g")] Attribute supports get/has queries (g -- get). */
#define Dee_ATTR_PERMDEL   0x0002 /* [NAME("d")] Attribute supports del queries (d -- del). */
#define Dee_ATTR_PERMSET   0x0004 /* [NAME("s")] Attribute supports set queries (s -- set). */
#define Dee_ATTR_PERMCALL  0x0008 /* [NAME("f")] The attribute is intended to be called (f -- function). */
#define Dee_ATTR_IMEMBER   0x0010 /* [NAME("i")] This attribute is an instance attribute (i -- instance). */
#define Dee_ATTR_CMEMBER   0x0020 /* [NAME("c")] This attribute is a class attribute (c -- class). */
#define Dee_ATTR_PRIVATE   0x0040 /* [NAME("h")] This attribute is considered private (h -- hidden). */
#define Dee_ATTR_PROPERTY  0x0080 /* [NAME("p")] Accessing the attribute may have unpredictable side-effects (p -- property). */
#define Dee_ATTR_WRAPPER   0x0100 /* [NAME("w")] In the current content, the attribute will be accessed as a wrapper. */
#define Dee_ATTR_NAMEOBJ   0x4000 /* HINT: `attr_name' is actually the `s_str' field of a `DeeStringObject'. */
#define Dee_ATTR_DOCOBJ    0x8000 /* HINT: `attr_doc' (when non-NULL) is actually the `s_str' field of a `DeeStringObject'. */

#ifdef DEE_SOURCE
typedef Dee_enum_t denum_t;
#define ATTR_PERMGET   Dee_ATTR_PERMGET  /* [NAME("g")] Attribute supports get/has queries (g -- get). */
#define ATTR_PERMDEL   Dee_ATTR_PERMDEL  /* [NAME("d")] Attribute supports del queries (d -- del). */
#define ATTR_PERMSET   Dee_ATTR_PERMSET  /* [NAME("s")] Attribute supports set queries (s -- set). */
#define ATTR_PERMCALL  Dee_ATTR_PERMCALL /* [NAME("f")] The attribute is intended to be called (f -- function). */
#define ATTR_IMEMBER   Dee_ATTR_IMEMBER  /* [NAME("i")] This attribute is an instance attribute (i -- instance). */
#define ATTR_CMEMBER   Dee_ATTR_CMEMBER  /* [NAME("c")] This attribute is a class attribute (c -- class). */
#define ATTR_PRIVATE   Dee_ATTR_PRIVATE  /* [NAME("h")] This attribute is considered private (h -- hidden). */
#define ATTR_PROPERTY  Dee_ATTR_PROPERTY /* [NAME("p")] Accessing the attribute may have unpredictable side-effects (p -- property). */
#define ATTR_WRAPPER   Dee_ATTR_WRAPPER  /* [NAME("w")] In the current content, the attribute will be accessed as a wrapper. */
#define ATTR_NAMEOBJ   Dee_ATTR_NAMEOBJ  /* HINT: `attr_name' is actually the `s_str' field of a `DeeStringObject'. */
#define ATTR_DOCOBJ    Dee_ATTR_DOCOBJ   /* HINT: `attr_doc' (when non-NULL) is actually the `s_str' field of a `DeeStringObject'. */
#endif /* DEE_SOURCE */



#ifndef DEE_TYPE_ALLOCATOR
/* Specifies a custom object allocator declaration. */
#define DEE_TYPE_ALLOCATOR(tp_malloc, tp_free) (void *)(tp_free), { (uintptr_t)(void *)(tp_malloc) }

/* Specifies an automatic object allocator. */
#define DEE_TYPE_AUTOSIZED_ALLOCATOR(size)                 NULL, { (uintptr_t)(size) }
#define DEE_TYPE_AUTOSIZED_ALLOCATOR_R(min_size, max_size) NULL, { (uintptr_t)(max_size) }
#define DEE_TYPE_AUTO_ALLOCATOR(T)                         NULL, { (uintptr_t)sizeof(T) }

/* Expose shorter variants of macros */
#ifdef DEE_SOURCE
#define TYPE_ALLOCATOR              DEE_TYPE_ALLOCATOR
#define TYPE_AUTOSIZED_ALLOCATOR    DEE_TYPE_AUTOSIZED_ALLOCATOR
#define TYPE_AUTOSIZED_ALLOCATOR_R  DEE_TYPE_AUTOSIZED_ALLOCATOR_R
#define TYPE_AUTO_ALLOCATOR         DEE_TYPE_AUTO_ALLOCATOR
#endif /* DEE_SOURCE */
#endif /* !DEE_TYPE_ALLOCATOR */

#ifdef GUARD_DEEMON_ALLOC_H
/* Specifies an allocator that may provides optimizations
 * for types with a FIXED size (which most objects have). */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DEE_TYPE_SIZED_ALLOCATOR_R     TYPE_AUTOSIZED_ALLOCATOR_R
#define DEE_TYPE_SIZED_ALLOCATOR_GC_R  TYPE_AUTOSIZED_ALLOCATOR_R
#define DEE_TYPE_SIZED_ALLOCATOR       TYPE_AUTOSIZED_ALLOCATOR
#define DEE_TYPE_SIZED_ALLOCATOR_GC    TYPE_AUTOSIZED_ALLOCATOR
#define DEE_TYPE_FIXED_ALLOCATOR       TYPE_AUTO_ALLOCATOR
#define DEE_TYPE_FIXED_ALLOCATOR_GC    TYPE_AUTO_ALLOCATOR
#else /* CONFIG_NO_OBJECT_SLABS */
#define DEE_TYPE_SIZED_ALLOCATOR_R(min_size, max_size)               \
	  DeeSlab_Invoke((void *)&DeeObject_SlabFree, min_size, , NULL), \
	{ DeeSlab_Invoke((uintptr_t)(void *)&DeeObject_SlabMalloc, max_size, , max_size) }
#define DEE_TYPE_SIZED_ALLOCATOR_GC_R(min_size, max_size)              \
	  DeeSlab_Invoke((void *)&DeeGCObject_SlabFree, min_size, , NULL), \
	{ DeeSlab_Invoke((uintptr_t)(void *)&DeeGCObject_SlabMalloc, max_size, , max_size) }
#define DEE_TYPE_SIZED_ALLOCATOR(size)    DEE_TYPE_SIZED_ALLOCATOR_R(size, size)
#define DEE_TYPE_SIZED_ALLOCATOR_GC(size) DEE_TYPE_SIZED_ALLOCATOR_GC_R(size, size)
#define DEE_TYPE_FIXED_ALLOCATOR(T)       DEE_TYPE_SIZED_ALLOCATOR_R(sizeof(T), sizeof(T))
#define DEE_TYPE_FIXED_ALLOCATOR_GC(T)    DEE_TYPE_SIZED_ALLOCATOR_GC_R(sizeof(T), sizeof(T))
#endif /* !CONFIG_NO_OBJECT_SLABS */

/* Same as `DEE_TYPE_FIXED_ALLOCATOR()', but don't link agains dedicated
 * allocator functions when doing so would require the creation of
 * relocations that might cause loading times to become larger. */
#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define DEE_TYPE_FIXED_ALLOCATOR_S(T)      DEE_TYPE_AUTO_ALLOCATOR(T)
#define DEE_TYPE_FIXED_ALLOCATOR_GC_S(T)   DEE_TYPE_AUTO_ALLOCATOR(T)
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define DEE_TYPE_FIXED_ALLOCATOR_S(T)      DEE_TYPE_FIXED_ALLOCATOR(T)
#define DEE_TYPE_FIXED_ALLOCATOR_GC_S(T)   DEE_TYPE_FIXED_ALLOCATOR_GC(T)
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */

#ifdef DEE_SOURCE
#define TYPE_SIZED_ALLOCATOR_R         DEE_TYPE_SIZED_ALLOCATOR_R
#define TYPE_SIZED_ALLOCATOR_GC_R      DEE_TYPE_SIZED_ALLOCATOR_GC_R
#define TYPE_SIZED_ALLOCATOR           DEE_TYPE_SIZED_ALLOCATOR
#define TYPE_SIZED_ALLOCATOR_GC        DEE_TYPE_SIZED_ALLOCATOR_GC
#define TYPE_FIXED_ALLOCATOR           DEE_TYPE_FIXED_ALLOCATOR
#define TYPE_FIXED_ALLOCATOR_GC        DEE_TYPE_FIXED_ALLOCATOR_GC
#define TYPE_FIXED_ALLOCATOR_S         DEE_TYPE_FIXED_ALLOCATOR_S
#define TYPE_FIXED_ALLOCATOR_GC_S      DEE_TYPE_FIXED_ALLOCATOR_GC_S
#endif /* DEE_SOURCE */
#endif /* GUARD_DEEMON_ALLOC_H */




#undef tp_alloc
struct Dee_type_constructor {
	/* Instance constructors/destructors. */
	union {
		struct {
			void *_tp_init0_; /* tp_ctor */
			void *_tp_init1_; /* tp_copy_ctor */
			void *_tp_init2_; /* tp_deep_ctor */
			void *_tp_init3_; /* tp_any_ctor */
			/* Initializer for a custom type allocator. */
			void *_tp_init4_; /* tp_free */
			struct { uintptr_t _tp_init5_; } _tp_init6_;
			void *_tp_init7_; /* tp_any_ctor_kw */
		} _tp_init_;
		struct {
			WUNUSED NONNULL((1))    int (DCALL *tp_ctor)(DeeObject *__restrict self);
			WUNUSED NONNULL((1, 2)) int (DCALL *tp_copy_ctor)(DeeObject *__restrict self, DeeObject *__restrict other);
			WUNUSED NONNULL((1, 2)) int (DCALL *tp_deep_ctor)(DeeObject *__restrict self, DeeObject *__restrict other);
			WUNUSED NONNULL((1))    int (DCALL *tp_any_ctor)(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
			/* WARNING: A situation can arise in which the `tp_free'
			 *          operator of a base-class is used instead of
			 *          the one accompanying `tp_alloc()'.
			 *       >> Because of this, `tp_alloc' and `tp_free' should only
			 *          be used for accessing a cache of pre-allocated objects, that
			 *          were created using regular heap allocations (`DeeObject_Malloc'). */
			NONNULL((1)) void (DCALL *tp_free)(void *__restrict ob);
			union {
				size_t tp_instance_size; /* [valid_if(tp_free == NULL)] */
				void *(DCALL *tp_alloc)(void); /* [valid_if(tp_free != NULL)] */
			}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
			_dee_aunion
#define tp_instance_size _dee_aunion.tp_instance_size
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
			;
			/* WARNING: `tp_any_ctor_kw' may be invoked with `argc == 0 && kw == NULL',
			 *           even when `tp_ctor' has been defined as non-NULL! */
			WUNUSED NONNULL((1))
			int (DCALL *tp_any_ctor_kw)(DeeObject *__restrict self, size_t argc,
			                            DeeObject *const *argv, DeeObject *kw);
		} tp_alloc; /* [valid_if(!TP_FVARIABLE)] */
		struct {
			/* NOTE: Var-constructors are allowed to return instances of types other than `tp'
			 *       However, this is a privilege that is not exposed to user-code.
			 *       Additionally, any type making use of this must
			 *       openly document this in order to prevent confusion.
			 *       It should also be noted that the deemon core does not make use of
			 *       this functionality anywhere, and as of right now, the only type
			 *       that does make use of it is the copy constructor of `lvalue'
			 *       objects found in `ctypes'.
			 *       Rather than returning another instance of the l-value type, it
			 *       returns a regular structured object containing a copy of the data
			 *       that was pointed-to by the l-value.
			 */
			WUNUSED DREF DeeObject *(DCALL *tp_ctor)(void);
			WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *tp_copy_ctor)(DeeObject *__restrict other);
			WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *tp_deep_ctor)(DeeObject *__restrict other);
			WUNUSED              DREF DeeObject *(DCALL *tp_any_ctor)(size_t argc, DeeObject *const *argv);
			        NONNULL((1)) void            (DCALL *tp_free)(void *__restrict ob);
			struct { uintptr_t tp_pad; } tp_pad; /* ... */
			/* WARNING: `tp_any_ctor_kw' may be invoked with `argc == 0 && kw == NULL',
			 *           even when `tp_ctor' has been defined as non-NULL! */
			DREF DeeObject *(DCALL *tp_any_ctor_kw)(size_t argc, DeeObject *const *argv, DeeObject *kw);
		} tp_var; /* [valid_if(TP_FVARIABLE)] */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define tp_alloc _dee_aunion.tp_alloc
#define tp_var   _dee_aunion.tp_var
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	NONNULL((1)) void (DCALL *tp_dtor)(DeeObject *__restrict self); /* [0..1] */
	/* NOTE: `tp_move_assign' is favored in code such as this:
	 * >> local my_list = [];
	 * >> my_list := copy get_other_list(); // Will try to move-assign the copy.
	 * >> my_list := deepcopy get_other_list(); // Will try to move-assign the deep copy. */
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_assign)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_move_assign)(DeeObject *self, DeeObject *other);
	/* Following a previously successful construction using the `tp_deep_ctor' operator,
	 * go through all member objects of the type and replace them with deep copies.
	 * This operator is required to provide a safe way for GC objects to be
	 * constructed for deepcopy, using a 2-step process that allows the
	 * runtime to account for possible recursion:
	 * >>     DeeTypeObject *type;       // The type implementing the visible `tp_deep_ctor'
	 * >>     Deebject      *old_object; // The object being copied.
	 * >>     Deebject      *new_object; // The object resulting from the copy.
	 * >>     // Check if this instance is already being duplicated
	 * >>     new_object = deepcopy_lookup(old_object, type);
	 * >>     if (new_object) return_reference_(new_object);
	 * >>     // Construct a new object using the deep-constructor.
	 * >>     // For non-GC objects, this will do _all_ the work, but for GC objects,
	 * >>     // it will only create shallow references to all associated objects,
	 * >>     // which are then replaced by deep copies once `tp_deepload' is invoked.
	 * >>     // Note however that some GC-able classes (such as user-defined ones)
	 * >>     // do not provide a tp_deepload operator, rather choosing to implement
	 * >>     // their own mechanism to safely implement a deepcopy+load mechanism.
	 * >>     // However since such a complex mechanism is often not required (in the
	 * >>     // case of user-classes it is unify the deepcopy operator to a single
	 * >>     // function invocation), deepload can be used to split the two parts
	 * >>     // and take the load of calling `Dee_DeepCopyAddAssoc' from the
	 * >>     // implementing type to the runtime library.
	 * >>     deepcopy_begin();
	 * >>     new_object = tp_deep_ctor(old_object); // Usually the same callback as `tp_copy_ctor'
	 * >>     if (tp_deepload) {
	 * >>         int error;
	 * >>         if (!new_object) goto err;
	 * >>         if (Dee_DeepCopyAddAssoc(old_object, new_object))
	 * >>             goto err_new;
	 * >>         error = tp_deepload(new_object);
	 * >>         if (error)
	 * >>             goto err_new;
	 * >>         deepcopy_end();
	 * >>     } else if (deepcopy_end()) {
	 * >>         // This is a recursive deepcopy operation, so we must still track the new object
	 * >>         if (Dee_DeepCopyAddAssoc(old_object, new_object))
	 * >>             goto err_new;
	 * >>     }
	 * >>     return new_object;
	 * >> err_new:
	 * >>     Dee_Decref(new_object);
	 * >> err:
	 * >>     deepcopy_end();
	 * >>     return NULL;
	 */
	WUNUSED NONNULL((1)) int (DCALL *tp_deepload)(DeeObject *__restrict self);
};

struct Dee_type_cast {
	/* Instance casting operators. */
	WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *tp_str)(DeeObject *__restrict self);
	WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *tp_repr)(DeeObject *__restrict self);
	WUNUSED NONNULL((1)) int (DCALL *tp_bool)(DeeObject *__restrict self);
};

struct Dee_type_gc {
	/* Clear all possible references with `NULL' or some
	 * statically allocated stub-object (e.g.: `Dee_None') */
	NONNULL((1)) void (DCALL *tp_clear)(DeeObject *__restrict self);
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
	NONNULL((1)) void (DCALL *tp_pclear)(DeeObject *__restrict self, unsigned int gc_priority);
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
	 *    x -> MyClass -> function(MyClass.operators) -> code -> module
	 *    ^      ^  ^                                     |       |  |
	 *    |      |  |                                     |       |  |
	 *    |      |  +-------------------------------------+       |  |
	 *    |      |                                                |  |
	 *    |      +------------------------------------------------+  |
	 *    |                                                          |
	 *    +----------------------------------------------------------+
	 *
	 * This might seem simple at first, but depending on the order with which the GC
	 * chooses to deal with this cycle determines if the destructor can even function
	 * properly:
	 *   - As you can see, it uses a member variable `m_value', meaning that
	 *     if we were to clear the instance `x' first, it would fail with an
	 *     attribute error, so `x' is a bad idea
	 *   - If we were to start by clearing `MyClass's operators, it wouldn't
	 *     work at all because there'd no longer be a destructor to call.
	 *   - The link from `code' to `module' is kind-of special, in that it
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
	 *    (which might cause problems with dynamically imported modules, but we
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
};


/* GC destruction priority levels of builtin types. */
#define Dee_GC_PRIORITY_LATE      0x0000 /* (Preferably) destroyed last. */
#define Dee_GC_PRIORITY_CLASS     0xfd00 /* User-classes. */
#define Dee_GC_PRIORITY_INSTANCE  0xfe00 /* Instances of user-classes. */
#define Dee_GC_PRIORITY_MODULE    0xff00 /* Module objects. */
#define Dee_GC_PRIORITY_EARLY     0xffff /* (Preferably) destroyed before anything else. */


/* Return values for `tp_int32' and `tp_int64' */
#define Dee_INT_SIGNED   0 /* The saved integer value is signed. */
#define Dee_INT_UNSIGNED 1 /* The saved integer value is unsigned. */

#ifdef DEE_SOURCE
#define INT_SIGNED   Dee_INT_SIGNED    /* The saved integer value is signed. */
#define INT_UNSIGNED Dee_INT_UNSIGNED  /* The saved integer value is unsigned. */
#endif /* DEE_SOURCE */

struct Dee_type_math {
	/* Math related operators. */
	/* @return: Dee_INT_SIGNED:   The value stored in `*result' is signed. 
	 * @return: Dee_INT_UNSIGNED: The value stored in `*result' is unsigned.
	 * @return: -1:               An error occurred. */
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_int32)(DeeObject *__restrict self, int32_t *__restrict result);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_int64)(DeeObject *__restrict self, int64_t *__restrict result);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_double)(DeeObject *__restrict self, double *__restrict result);
	WUNUSED NONNULL((1))    DREF DeeObject *(DCALL *tp_int)(DeeObject *__restrict self); /* Cast to `int' (Must return an `DeeInt_Type' object) */
	WUNUSED NONNULL((1))    DREF DeeObject *(DCALL *tp_inv)(DeeObject *__restrict self);
	WUNUSED NONNULL((1))    DREF DeeObject *(DCALL *tp_pos)(DeeObject *__restrict self);
	WUNUSED NONNULL((1))    DREF DeeObject *(DCALL *tp_neg)(DeeObject *__restrict self);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_add)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_sub)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_mul)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_div)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_mod)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_shl)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_shr)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_and)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_or)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_xor)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_pow)(DeeObject *self, DeeObject *some_object);
	/* Inplace operators (Optional; Implemented using functions above when not available) */
	WUNUSED NONNULL((1)) int (DCALL *tp_inc)(DeeObject **__restrict pself);
	WUNUSED NONNULL((1)) int (DCALL *tp_dec)(DeeObject **__restrict pself);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_add)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_sub)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_mul)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_div)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_mod)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_shl)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_shr)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_and)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_or)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_xor)(DeeObject **__restrict pself, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) int (DCALL *tp_inplace_pow)(DeeObject **__restrict pself, DeeObject *some_object);
};

struct Dee_type_cmp {
	/* Compare operators. */
	WUNUSED NONNULL((1))    Dee_hash_t      (DCALL *tp_hash)(DeeObject *__restrict self);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_eq)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_ne)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_lo)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_le)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_gr)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL *tp_ge)(DeeObject *self, DeeObject *some_object);
	/* Optional iterator-extensions for providing optimized (but
	 * less generic) variants for various iterator operations.
	 * NOTE: The compare sub-structure was chosen for this, as native
	 *       iterators usually implement compare operators to allow
	 *       them to be ordered with other operators. */
	struct Dee_type_nii Dee_tpconst *tp_nii;
}; 


struct Dee_type_seq {
	/* Sequence operators. */
	WUNUSED NONNULL((1))          DREF DeeObject *(DCALL *tp_iter_self)(DeeObject *__restrict self);
	WUNUSED NONNULL((1))          DREF DeeObject *(DCALL *tp_size)(DeeObject *__restrict self);
	WUNUSED NONNULL((1, 2))       DREF DeeObject *(DCALL *tp_contains)(DeeObject *self, DeeObject *some_object);
	WUNUSED NONNULL((1, 2))       DREF DeeObject *(DCALL *tp_get)(DeeObject *self, DeeObject *index);
	WUNUSED NONNULL((1, 2))       int             (DCALL *tp_del)(DeeObject *self, DeeObject *index);
	WUNUSED NONNULL((1, 2, 3))    int             (DCALL *tp_set)(DeeObject *self, DeeObject *index, DeeObject *value);
	WUNUSED NONNULL((1, 2, 3))    DREF DeeObject *(DCALL *tp_range_get)(DeeObject *self, DeeObject *begin, DeeObject *end);
	WUNUSED NONNULL((1, 2, 3))    int             (DCALL *tp_range_del)(DeeObject *self, DeeObject *begin, DeeObject *end);
	WUNUSED NONNULL((1, 2, 3, 4)) int             (DCALL *tp_range_set)(DeeObject *self, DeeObject *begin, DeeObject *end, DeeObject *value);
	/* Optional sequence-extensions for providing optimized (but
	 * less generic) variants for various sequence operations. */
	struct Dee_type_nsi Dee_tpconst *tp_nsi;
};

struct Dee_type_attr {
	/* Attribute operators. */
	WUNUSED NONNULL((1, 2))    DREF DeeObject *(DCALL *tp_getattr)(DeeObject *self, /*String*/ DeeObject *name);
	WUNUSED NONNULL((1, 2))    int             (DCALL *tp_delattr)(DeeObject *self, /*String*/ DeeObject *name);
	WUNUSED NONNULL((1, 2, 3)) int             (DCALL *tp_setattr)(DeeObject *self, /*String*/ DeeObject *name, DeeObject *value);
	WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t     (DCALL *tp_enumattr)(DeeTypeObject *tp_self, DeeObject *self, Dee_enum_t proc, void *arg);
};

struct Dee_type_with {
	/* With-statement operators. */
	WUNUSED NONNULL((1)) int (DCALL *tp_enter)(DeeObject *__restrict self);
	WUNUSED NONNULL((1)) int (DCALL *tp_leave)(DeeObject *__restrict self);
};

typedef struct dee_bytesbuffer DeeBuffer;
struct dee_bytesbuffer {
	void           *bb_base;  /* [0..bb_size][const] Base address of the buffer.
	                           * NOTE: Only writable if the buffer was acquired with `Dee_BUFFER_FWRITABLE' set. */
	size_t          bb_size;  /* [const] Size of the buffer (in bytes) */
#ifndef __INTELLISENSE__
	/* [0..1][INTERNAL] used to speed up `DeeObject_PutBuf()' */
	NONNULL((1, 2))
	void    (DCALL *bb_put)(DeeObject *__restrict self, DeeBuffer *__restrict info, unsigned int flags);
#endif /* !__INTELLISENSE__ */
};
#ifdef __INTELLISENSE__
#define DeeBuffer_INIT(base, size) { base, size }
#else /* __INTELLISENSE__ */
#define DeeBuffer_INIT(base, size) { base, size, NULL }
#endif /* !__INTELLISENSE__ */


struct Dee_type_buffer {
	/* Low-level buffer interface. */

	/* When implemented, `tp_getbuf' must fill in at least `bb_base' and `bb_size'
	 * @param: flags: Set of `DEE_BUFFER_F*' */
	WUNUSED NONNULL((1, 2))
	int  (DCALL *tp_getbuf)(DeeObject *__restrict self,
	                        DeeBuffer *__restrict info,
	                        unsigned int flags);
#define Dee_BUFFER_FREADONLY 0x0000 /* Acquire the buffer for reading. */
#define Dee_BUFFER_FWRITABLE 0x0001 /* Acquire the buffer for reading / writing. */
#define Dee_BUFFER_FMASK     0x0001 /* Mask of known buffer flags. */
	/* Release a previously acquired buffer.
	 * @param: flags: Set of `DEE_BUFFER_F*' (same as were passed to `tp_getbuf') */
	NONNULL((1, 2))
	void (DCALL *tp_putbuf)(DeeObject *__restrict self,
	                        DeeBuffer *__restrict info,
	                        unsigned int flags);

#define Dee_BUFFER_TYPE_FNORMAL   0x0000 /* Normal buffer type flags. */
#define Dee_BUFFER_TYPE_FREADONLY 0x0001 /* The buffer can only be used for reading.
	                                      * -> When set, `DeeObject_GetBuf' fails when
	                                      *    the `Dee_BUFFER_FWRITABLE' flag is set. */
	unsigned int tp_buffer_flags; /* Buffer type flags (Set of `DEE_BUFFER_TYPE_F*') */
};




/* WARNING: The `argv' vector should actually be written as `DeeObject *const *__restrict argv',
 *          however since it appears in as many places as it does, doing so would really just
 *          be unnecessary work. However because of this you must remember never to modify an
 *          argument vector!
 *          For example: If your function is called from the interpreted, `argv' often points
 *          into the associated frame's stack, meaning that modifications could bring along
 *          deadly consequences!
 *          Even in user-code itself, where it might seem as though you were able to write to
 *          argument variables, in actuality, the compiler will notice and store the argument
 *          in a local variable at the beginning of the function, meaning you'll actually just
 *          be modifying a local variable.
 * @param: self:  The obj-part of the objmethod.
 * @param: argc:  The number of arguments passed.
 * @param: argv:  [1..1][const][0..argc] Actually this would have
 *                to be written as `DeeObject *const *__restrict'
 * @return: * :   The function return value.
 * @return: NULL: An error occurred. */
typedef WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *Dee_objmethod_t)(DeeObject *self, size_t argc, DeeObject *const *argv);
typedef WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *Dee_kwobjmethod_t)(DeeObject *self, size_t argc, DeeObject *const *argv, /*nullable*/ DeeObject *kw);
typedef WUNUSED NONNULL((1)) DREF DeeObject *(DCALL *Dee_getmethod_t)(DeeObject *__restrict self);
typedef WUNUSED NONNULL((1)) int (DCALL *Dee_delmethod_t)(DeeObject *__restrict self);
typedef WUNUSED NONNULL((1, 2)) int (DCALL *Dee_setmethod_t)(DeeObject *self, DeeObject *value);

#ifdef DEE_SOURCE
typedef Dee_objmethod_t   dobjmethod_t;
typedef Dee_kwobjmethod_t dkwobjmethod_t;
typedef Dee_getmethod_t   dgetmethod_t;
typedef Dee_delmethod_t   ddelmethod_t;
typedef Dee_setmethod_t   dsetmethod_t;
#endif /* DEE_SOURCE */

#define Dee_TYPE_METHOD_FNORMAL 0x0000 /* Normal type method flags. */
#define Dee_TYPE_METHOD_FKWDS   0x0001 /* `m_func' takes a keywords argument.
                                        * When set, `m_func' is actually a `dkwobjmethod_t' */

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

struct Dee_type_getset {
	char const           *gs_name; /* [1..1][SENTINAL(NULL)] Member name. */
	/* Getset callbacks (NULL callbacks will result in `Error.AttributeError' being raised) */
	Dee_getmethod_t       gs_get;  /* [0..1] Getter callback. */
	Dee_delmethod_t       gs_del;  /* [0..1] Delete callback. */
	Dee_setmethod_t       gs_set;  /* [0..1] Setter callback. */
	/*utf-8*/ char const *gs_doc;  /* [0..1] Documentation string. */
};



/* Member type codes. */
#define Dee_STRUCT_NONE        0x0000 /* Ignore offset and always return `none' (Useful for forward/backward no-op compatibility) */
#define Dee_STRUCT_OBJECT      0x8001 /* `[0..1] DREF DeeObject *' (raise `Error.AttributeError' if `NULL') */
#define Dee_STRUCT_WOBJECT     0x0003 /* `[0..1] struct Dee_weakref' (raise `Error.AttributeError' if locking fails) */
#define Dee_STRUCT_OBJECT_OPT  0x8005 /* `[0..1] DREF DeeObject *' (return `none' if NULL) */
#define Dee_STRUCT_WOBJECT_OPT 0x0007 /* `[0..1] struct Dee_weakref' (return `none' if locking fails) */
#define Dee_STRUCT_CSTR        0x8010 /* `[0..1] char *' (utf-8) (Accessible as `DeeStringObject'; raise `Error.AttributeError' if `NULL') */
#define Dee_STRUCT_CSTR_OPT    0x8011 /* `[0..1] char *' (utf-8) (Accessible as `DeeStringObject'; return `none' when `NULL') */
#define Dee_STRUCT_CSTR_EMPTY  0x8012 /* `[0..1] char *' (utf-8) (Accessible as `DeeStringObject'; return an empty string when `NULL') */
#define Dee_STRUCT_STRING      0x8013 /* `char[*]' (utf-8) (Accessible as `DeeStringObject') */
#define Dee_STRUCT_CHAR        0x0014 /* `unsigned char' (latin-1) (Accessible as `DeeStringObject') */
#define Dee_STRUCT_BOOL8       0x0020 /* `uint8_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOL16      0x0021 /* `uint16_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOL32      0x0022 /* `uint32_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOL64      0x0023 /* `uint64_t' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT0    0x0030 /* `uint8_t & 0x01' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT1    0x0031 /* `uint8_t & 0x02' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT2    0x0032 /* `uint8_t & 0x04' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT3    0x0033 /* `uint8_t & 0x08' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT4    0x0034 /* `uint8_t & 0x10' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT5    0x0035 /* `uint8_t & 0x20' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT6    0x0036 /* `uint8_t & 0x40' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT7    0x0037 /* `uint8_t & 0x80' (Accessible as `DeeBoolObject') */
#define Dee_STRUCT_BOOLBIT(mask)                                                   \
	((mask) == 0x80 ? Dee_STRUCT_BOOLBIT7 : (mask) == 0x40 ? Dee_STRUCT_BOOLBIT6 : \
	 (mask) == 0x20 ? Dee_STRUCT_BOOLBIT5 : (mask) == 0x10 ? Dee_STRUCT_BOOLBIT4 : \
	 (mask) == 0x08 ? Dee_STRUCT_BOOLBIT3 : (mask) == 0x04 ? Dee_STRUCT_BOOLBIT2 : \
	 (mask) == 0x02 ? Dee_STRUCT_BOOLBIT1 : Dee_STRUCT_BOOLBIT0)
#define Dee_STRUCT_BOOLBITMASK(type) (1 << ((type) - Dee_STRUCT_BOOLBIT0))
#define Dee_STRUCT_FLOAT       0x0040 /* `float' */
#define Dee_STRUCT_DOUBLE      0x0041 /* `double' */
#define Dee_STRUCT_LDOUBLE     0x0042 /* `long double' */
#define Dee_STRUCT_VOID        Dee_STRUCT_NONE /* `void' */
#define Dee_STRUCT_INT8        0x0800 /* `int8_t' */
#define Dee_STRUCT_INT16       0x0801 /* `int16_t' */
#define Dee_STRUCT_INT32       0x0802 /* `int32_t' */
#define Dee_STRUCT_INT64       0x0803 /* `int64_t' */
#define Dee_STRUCT_INT128      0x0804 /* `Dee_int128_t' */
#define Dee_STRUCT_UNSIGNED    0x0008 /* FLAG: Unsigned integer (Use with `STRUCT_INT*'). */
#define Dee_STRUCT_ATOMIC      0x4000 /* FLAG: Atomic read/write access (Use with `STRUCT_INT*'). */
#define Dee_STRUCT_CONST       0x8000 /* FLAG: Read-only field. */

#define DEE_PRIVATE_STRUCT_INT1 Dee_STRUCT_INT8
#define DEE_PRIVATE_STRUCT_INT2 Dee_STRUCT_INT16
#define DEE_PRIVATE_STRUCT_INT4 Dee_STRUCT_INT32
#define DEE_PRIVATE_STRUCT_INT8 Dee_STRUCT_INT64
#define DEE_PRIVATE_STRUCT_INT(sizeof) DEE_PRIVATE_STRUCT_INT##sizeof
#define Dee_STRUCT_INTEGER(sizeof) DEE_PRIVATE_STRUCT_INT(sizeof)

#define DEE_PRIVATE_STRUCT_BOOL1 Dee_STRUCT_BOOL8
#define DEE_PRIVATE_STRUCT_BOOL2 Dee_STRUCT_BOOL16
#define DEE_PRIVATE_STRUCT_BOOL4 Dee_STRUCT_BOOL32
#define DEE_PRIVATE_STRUCT_BOOL8 Dee_STRUCT_BOOL64
#define DEE_PRIVATE_STRUCT_BOOL(sizeof) DEE_PRIVATE_STRUCT_BOOL##sizeof
#define Dee_STRUCT_BOOL(sizeof) DEE_PRIVATE_STRUCT_BOOL(sizeof)

#ifdef DEE_SOURCE
#define STRUCT_NONE        Dee_STRUCT_NONE        /* Ignore offset and always return `none' (Useful for forward/backward no-op compatibility) */
#define STRUCT_OBJECT      Dee_STRUCT_OBJECT      /* `[0..1] DREF DeeObject *' (raise `Error.AttributeError' if `NULL') */
#define STRUCT_WOBJECT     Dee_STRUCT_WOBJECT     /* `[0..1] struct Dee_weakref' (raise `Error.AttributeError' if locking fails) */
#define STRUCT_OBJECT_OPT  Dee_STRUCT_OBJECT_OPT  /* `[0..1] DREF DeeObject *' (return `none' if NULL) */
#define STRUCT_WOBJECT_OPT Dee_STRUCT_WOBJECT_OPT /* `[0..1] struct Dee_weakref' (return `none' if locking fails) */
#define STRUCT_CSTR        Dee_STRUCT_CSTR        /* `[0..1] char *' (utf-8) (Accessible as `DeeStringObject'; raise `Error.AttributeError' if `NULL') */
#define STRUCT_CSTR_OPT    Dee_STRUCT_CSTR_OPT    /* `[0..1] char *' (utf-8) (Accessible as `DeeStringObject'; return `none' when `NULL') */
#define STRUCT_CSTR_EMPTY  Dee_STRUCT_CSTR_EMPTY  /* `[0..1] char *' (utf-8) (Accessible as `DeeStringObject'; return an empty string when `NULL') */
#define STRUCT_STRING      Dee_STRUCT_STRING      /* `char[*]' (utf-8) (Accessible as `DeeStringObject') */
#define STRUCT_CHAR        Dee_STRUCT_CHAR        /* `unsigned char' (latin-1) (Accessible as `DeeStringObject') */
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
#define STRUCT_DHASH_T     (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(__SIZEOF_POINTER__))
#define STRUCT_SIZE_T      (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(__SIZEOF_SIZE_T__))
#define STRUCT_SSIZE_T     Dee_STRUCT_INTEGER(__SIZEOF_SIZE_T__)
#define STRUCT_INT         Dee_STRUCT_INTEGER(__SIZEOF_INT__)
#define STRUCT_UINT        (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(__SIZEOF_INT__))
#define STRUCT_INT8_T      Dee_STRUCT_INT8
#define STRUCT_INT16_T     Dee_STRUCT_INT16
#define STRUCT_INT32_T     Dee_STRUCT_INT32
#define STRUCT_INT64_T     Dee_STRUCT_INT64
#define STRUCT_UINT8_T     (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT8)
#define STRUCT_UINT16_T    (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT16)
#define STRUCT_UINT32_T    (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT32)
#define STRUCT_UINT64_T    (Dee_STRUCT_UNSIGNED | Dee_STRUCT_INT64)
#endif /* DEE_SOURCE */



struct Dee_type_member {
	char const           *m_name;   /* [1..1][SENTINAL(NULL)] Member name. */
	union {
		DeeObject        *m_const;  /* [valid_if(m_name[-1] == '!')][1..1] Constant. */
		struct {
			/* TODO: Use bit0=0 to indicate that m_const is used, and bit0=1 for offset-based!
			 *       We can always assume an alignment of at least 2 bytes for `m_const', so
			 *       this can always be done safely.
			 * NOTE: Do still include a CONFIG option to use the m_name[-1]-method, though.
			 *       Just in case there is some platform where pointers aren't at least 2-byte
			 *       aligned... */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			uint16_t      m_type;   /* [valid_if(m_name[-1] != '!')] Field type (One of `STRUCT_*'). */
			uint16_t      m_offset; /* [valid_if(m_name[-1] != '!')] Field offset (offsetof() field). */
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#if __SIZEOF_POINTER__ > 4
			uint16_t      m_pad[(__SIZEOF_POINTER__ - 2) / 2]; /* ... */
#endif /* __SIZEOF_POINTER__ > 4 */
			uint16_t      m_offset; /* [valid_if(m_name[-1] != '!')] Field offset (offsetof() field). */
			uint16_t      m_type;   /* [valid_if(m_name[-1] != '!')] Field type (One of `STRUCT_*'). */
#endif /* !... */
		}                 m_field;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define m_const _dee_aunion.m_const
#define m_field _dee_aunion.m_field
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	/*utf-8*/ char const *m_doc;    /* [0..1] Documentation string. */
};

#define Dee_TYPE_MEMBER_ISCONST(x) ((x)->m_name[-1] == '!')
#define Dee_TYPE_MEMBER_ISFIELD(x) ((x)->m_name[-1] != '!')
#define Dee_TYPE_MEMBER_END \
	{ NULL }
#define Dee_TYPE_MEMBER_FIELD_DOC(name, type, offset, doc) \
	{ ("\0" name) + 1, { (DeeObject *)(uintptr_t)((uint32_t)(type) | ((uint32_t)(offset) << 16)) }, DOC(doc) }
#define Dee_TYPE_MEMBER_CONST_DOC(name, value, doc) \
	{ ("!" name) + 1, { (DeeObject *)Dee_REQUIRES_OBJECT(value) }, DOC(doc) }
#define Dee_TYPE_MEMBER_FIELD(name, type, offset) Dee_TYPE_MEMBER_FIELD_DOC(name, type, offset, NULL)
#define Dee_TYPE_MEMBER_CONST(name, value) Dee_TYPE_MEMBER_CONST_DOC(name, value, NULL)

#ifdef UINT64_C
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(mask, err)                                               \
	(((mask) == UINT8_C(0x01) || (mask) == UINT16_C(0x0100) ||                                      \
	  (mask) == UINT32_C(0x010000) || (mask) == UINT32_C(0x01000000) ||                             \
	  (mask) == UINT64_C(0x0100000000) || (mask) == UINT64_C(0x010000000000) ||                     \
	  (mask) == UINT64_C(0x01000000000000) || (mask) == UINT64_C(0x0100000000000000))               \
	 ? Dee_STRUCT_BOOLBIT0                                                                          \
	 : ((mask) == UINT8_C(0x02) || (mask) == UINT16_C(0x0200) ||                                    \
	    (mask) == UINT32_C(0x020000) || (mask) == UINT32_C(0x02000000) ||                           \
	    (mask) == UINT64_C(0x0200000000) || (mask) == UINT64_C(0x020000000000) ||                   \
	    (mask) == UINT64_C(0x02000000000000) || (mask) == UINT64_C(0x0200000000000000))             \
	   ? Dee_STRUCT_BOOLBIT1                                                                        \
	   : ((mask) == UINT8_C(0x04) || (mask) == UINT16_C(0x0400) ||                                  \
	      (mask) == UINT32_C(0x040000) || (mask) == UINT32_C(0x04000000) ||                         \
	      (mask) == UINT64_C(0x0400000000) || (mask) == UINT64_C(0x040000000000) ||                 \
	      (mask) == UINT64_C(0x04000000000000) || (mask) == UINT64_C(0x0400000000000000))           \
	     ? Dee_STRUCT_BOOLBIT2                                                                      \
	     : ((mask) == UINT8_C(0x08) || (mask) == UINT16_C(0x0800) ||                                \
	        (mask) == UINT32_C(0x080000) || (mask) == UINT32_C(0x08000000) ||                       \
	        (mask) == UINT64_C(0x0800000000) || (mask) == UINT64_C(0x080000000000) ||               \
	        (mask) == UINT64_C(0x08000000000000) || (mask) == UINT64_C(0x0800000000000000))         \
	       ? Dee_STRUCT_BOOLBIT3                                                                    \
	       : ((mask) == UINT8_C(0x10) || (mask) == UINT16_C(0x1000) ||                              \
	          (mask) == UINT32_C(0x100000) || (mask) == UINT32_C(0x10000000) ||                     \
	          (mask) == UINT64_C(0x1000000000) || (mask) == UINT64_C(0x100000000000) ||             \
	          (mask) == UINT64_C(0x10000000000000) || (mask) == UINT64_C(0x1000000000000000))       \
	         ? Dee_STRUCT_BOOLBIT4                                                                  \
	         : ((mask) == UINT8_C(0x20) || (mask) == UINT16_C(0x2000) ||                            \
	            (mask) == UINT32_C(0x200000) || (mask) == UINT32_C(0x20000000) ||                   \
	            (mask) == UINT64_C(0x2000000000) || (mask) == UINT64_C(0x200000000000) ||           \
	            (mask) == UINT64_C(0x20000000000000) || (mask) == UINT64_C(0x2000000000000000))     \
	           ? Dee_STRUCT_BOOLBIT5                                                                \
	           : ((mask) == UINT8_C(0x40) || (mask) == UINT16_C(0x4000) ||                          \
	              (mask) == UINT32_C(0x400000) || (mask) == UINT32_C(0x40000000) ||                 \
	              (mask) == UINT64_C(0x4000000000) || (mask) == UINT64_C(0x400000000000) ||         \
	              (mask) == UINT64_C(0x40000000000000) || (mask) == UINT64_C(0x4000000000000000))   \
	             ? Dee_STRUCT_BOOLBIT6                                                              \
	             : err)
#ifdef __INTELLISENSE__
#define _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask, err)                                       \
	_Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(                                                    \
	mask, ((mask) == UINT8_C(0x80) || (mask) == UINT16_C(0x8000) ||                        \
	       (mask) == UINT32_C(0x800000) || (mask) == UINT32_C(0x80000000) ||               \
	       (mask) == UINT64_C(0x8000000000) || (mask) == UINT64_C(0x800000000000) ||       \
	       (mask) == UINT64_C(0x80000000000000) || (mask) == UINT64_C(0x8000000000000000)) \
	      ? Dee_STRUCT_BOOLBIT7                                                            \
	      : err)
#define Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask)               \
	((int(*)[_Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask, -1)])0, \
	 _Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(mask, Dee_STRUCT_BOOLBIT7))
#else /* __INTELLISENSE__ */
#define Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask) \
	_Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC0(mask, Dee_STRUCT_BOOLBIT7)
#endif /* !__INTELLISENSE__ */
#define Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)    \
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
#define Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(mask)                                    \
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
#define Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask) \
	((mask)&UINT8_C(0xff)                       \
	 ? 0                                        \
	 : (mask)&UINT16_C(0xff00)                  \
	   ? 1                                      \
	   : (mask)&UINT32_C(0xff0000)              \
	     ? 2                                    \
	     : 3)
#endif /* !UINT64_C */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, mask) \
	offsetof(struct_type, field) + Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, mask) \
	offsetof(struct_type, field) + (sizeof(((struct_type *)0)->field) - \
	                                (1 + Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)))
#else /* ... */
#error "Unsupported endian"
#endif /* !... */
#define Dee_TYPE_MEMBER_BITFIELD_DOC(name, flags, struct_type, field, flagmask, doc)           \
	Dee_TYPE_MEMBER_FIELD_DOC(name, (flags) | Dee_PRIVATE_STRUCT_BOOLBIT_TRUNC(flagmask),      \
	                          Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, flagmask), \
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


struct Dee_membercache_slot;
struct Dee_membercache {
	struct Dee_membercache     **mc_pself; /* [1..1][0..1][lock(INTERNAL(membercache_lock))] Self-pointer. */
	struct Dee_membercache      *mc_next;  /* [0..1][lock(INTERNAL(membercache_lock))] Next cache. */
	size_t                       mc_mask;  /* [lock(mc_lock)] Allocated table size -1. */
	size_t                       mc_size;  /* [lock(mc_lock)] Amount of used table entries. */
	struct Dee_membercache_slot *mc_table; /* [0..mc_mask+1][owned] Member cache table. */
#ifndef CONFIG_NO_THREADS
	Dee_rwlock_t                 mc_lock;  /* Lock used for accessing this cache. */
#endif /* !CONFIG_NO_THREADS */
};

#ifndef CONFIG_NO_THREADS
#define Dee_MEMBERCACHE_INIT  { NULL, NULL, 0, 0, NULL, RWLOCK_INIT }
#else /* !CONFIG_NO_THREADS */
#define Dee_MEMBERCACHE_INIT  { NULL, NULL, 0, 0, NULL }
#endif /* CONFIG_NO_THREADS */

#ifdef DEE_SOURCE
#define MEMBERCACHE_INIT  Dee_MEMBERCACHE_INIT
#endif /* DEE_SOURCE */



#define Dee_OPERATOR_USERCOUNT    0x003e        /* Number of user-accessible operators. (Used by `class' types) */
#define Dee_OPERATOR_EXTENDED(x) (0x1000 + (x)) /* Extended operator codes. (Type-specific; may be re-used) */
#ifdef DEE_SOURCE
/* Universal operator codes. */
#define OPERATOR_CONSTRUCTOR  0x0000 /* `operator this(args...): Object'                                  - `__constructor__' - `tp_any_ctor'. */
#define OPERATOR_COPY         0x0001 /* `operator copy(): Object'                                         - `__copy__'        - `tp_copy_ctor'. */
#define OPERATOR_DEEPCOPY     0x0002 /* `operator deepcopy(): Object'                                     - `__deepcopy__'    - `tp_deep_ctor'. */
#define OPERATOR_DESTRUCTOR   0x0003 /* `operator ~this(): none'                                          - `__destructor__'  - `tp_dtor'. */
#define OPERATOR_ASSIGN       0x0004 /* `operator := (other: Object): type(this)'                         - `__assign__'      - `tp_assign'. */
#define OPERATOR_MOVEASSIGN   0x0005 /* `operator move := (other: Object): type(this)'                    - `__moveassign__'  - `tp_move_assign'. */
#define OPERATOR_STR          0x0006 /* `operator str(): string'                                          - `__str__'         - `tp_str'. */
#define OPERATOR_REPR         0x0007 /* `operator repr(): string'                                         - `__repr__'        - `tp_repr'. */
#define OPERATOR_BOOL         0x0008 /* `operator bool(): bool'                                           - `__bool__'        - `tp_bool'. */
#define OPERATOR_ITERNEXT     0x0009 /* `operator next(): Object'                                         - `__next__'        - `tp_iter_next'. */
#define OPERATOR_CALL         0x000a /* `operator ()(args...): Object'                                    - `__call__'        - `tp_call'. */
#define OPERATOR_INT          0x000b /* `operator int(): int'                                             - `__int__'         - `tp_int'. */
#define OPERATOR_FLOAT        0x000c /* `operator float(): float'                                         - `__float__'       - `tp_double'. */
#define OPERATOR_INV          0x000d /* `operator ~ (): Object'                                           - `__inv__'         - `tp_inv'. */
#define OPERATOR_POS          0x000e /* `operator + (): Object'                                           - `__pos__'         - `tp_pos'. */
#define OPERATOR_NEG          0x000f /* `operator - (): Object'                                           - `__neg__'         - `tp_neg'. */
#define OPERATOR_ADD          0x0010 /* `operator + (other: Object): Object'                              - `__add__'         - `tp_add'. */
#define OPERATOR_SUB          0x0011 /* `operator - (other: Object): Object'                              - `__sub__'         - `tp_sub'. */
#define OPERATOR_MUL          0x0012 /* `operator * (other: Object): Object'                              - `__mul__'         - `tp_mul'. */
#define OPERATOR_DIV          0x0013 /* `operator / (other: Object): Object'                              - `__div__'         - `tp_div'. */
#define OPERATOR_MOD          0x0014 /* `operator % (other: Object): Object'                              - `__mod__'         - `tp_mod'. */
#define OPERATOR_SHL          0x0015 /* `operator << (other: Object): Object'                             - `__shl__'         - `tp_shl'. */
#define OPERATOR_SHR          0x0016 /* `operator >> (other: Object): Object'                             - `__shr__'         - `tp_shr'. */
#define OPERATOR_AND          0x0017 /* `operator & (other: Object): Object'                              - `__and__'         - `tp_and'. */
#define OPERATOR_OR           0x0018 /* `operator | (other: Object): Object'                              - `__or__'          - `tp_or'. */
#define OPERATOR_XOR          0x0019 /* `operator ^ (other: Object): Object'                              - `__xor__'         - `tp_xor'. */
#define OPERATOR_POW          0x001a /* `operator ** (other: Object): Object'                             - `__pow__'         - `tp_pow'. */
#define OPERATOR_INC          0x001b /* `operator ++ (): type(this)'                                      - `__inc__'         - `tp_inc'. */
#define OPERATOR_DEC          0x001c /* `operator -- (): type(this)'                                      - `__dec__'         - `tp_dec'. */
#define OPERATOR_INPLACE_ADD  0x001d /* `operator += (other: Object): type(this)'                         - `__iadd__'        - `tp_inplace_add'. */
#define OPERATOR_INPLACE_SUB  0x001e /* `operator -= (other: Object): type(this)'                         - `__isub__'        - `tp_inplace_sub'. */
#define OPERATOR_INPLACE_MUL  0x001f /* `operator *= (other: Object): type(this)'                         - `__imul__'        - `tp_inplace_mul'. */
#define OPERATOR_INPLACE_DIV  0x0020 /* `operator /= (other: Object): type(this)'                         - `__idiv__'        - `tp_inplace_div'. */
#define OPERATOR_INPLACE_MOD  0x0021 /* `operator %= (other: Object): type(this)'                         - `__imod__'        - `tp_inplace_mod'. */
#define OPERATOR_INPLACE_SHL  0x0022 /* `operator <<= (other: Object): type(this)'                        - `__ishl__'        - `tp_inplace_shl'. */
#define OPERATOR_INPLACE_SHR  0x0023 /* `operator >>= (other: Object): type(this)'                        - `__ishr__'        - `tp_inplace_shr'. */
#define OPERATOR_INPLACE_AND  0x0024 /* `operator &= (other: Object): type(this)'                         - `__iand__'        - `tp_inplace_and'. */
#define OPERATOR_INPLACE_OR   0x0025 /* `operator |= (other: Object): type(this)'                         - `__ior__'         - `tp_inplace_or'. */
#define OPERATOR_INPLACE_XOR  0x0026 /* `operator ^= (other: Object): type(this)'                         - `__ixor__'        - `tp_inplace_xor'. */
#define OPERATOR_INPLACE_POW  0x0027 /* `operator **= (other: Object): type(this)'                        - `__ipow__'        - `tp_inplace_pow'. */
#define OPERATOR_HASH         0x0028 /* `operator hash(): int'                                            - `__hash__'        - `tp_hash'. */
#define OPERATOR_EQ           0x0029 /* `operator == (other: Object): Object'                             - `__eq__'          - `tp_eq'. */
#define OPERATOR_NE           0x002a /* `operator != (other: Object): Object'                             - `__ne__'          - `tp_ne'. */
#define OPERATOR_LO           0x002b /* `operator < (other: Object): Object'                              - `__lo__'          - `tp_lo'. */
#define OPERATOR_LE           0x002c /* `operator <= (other: Object): Object'                             - `__le__'          - `tp_le'. */
#define OPERATOR_GR           0x002d /* `operator > (other: Object): Object'                              - `__gr__'          - `tp_gr'. */
#define OPERATOR_GE           0x002e /* `operator >= (other: Object): Object'                             - `__ge__'          - `tp_ge'. */
#define OPERATOR_ITERSELF     0x002f /* `operator iter(): Object'                                         - `__iter__'        - `tp_iter_self'. */
#define OPERATOR_SIZE         0x0030 /* `operator # (): Object'                                           - `__size__'        - `tp_size'. */
#define OPERATOR_CONTAINS     0x0031 /* `operator contains(other: Object): Object'                        - `__contains__'    - `tp_contains'. */
#define OPERATOR_GETITEM      0x0032 /* `operator [] (index: Object): Object'                             - `__getitem__'     - `tp_get'. */
#define OPERATOR_DELITEM      0x0033 /* `operator del[] (index: Object): none'                            - `__delitem__'     - `tp_del'. */
#define OPERATOR_SETITEM      0x0034 /* `operator []= (index: Object, value: Object): type(value)'        - `__setitem__'     - `tp_set'. */
#define OPERATOR_GETRANGE     0x0035 /* `operator [:] (begin: Object, end: Object): Object'               - `__getrange__'    - `tp_range_get'. */
#define OPERATOR_DELRANGE     0x0036 /* `operator del[:] (begin: Object, end: Object): none'              - `__delrange__'    - `tp_range_del'. */
#define OPERATOR_SETRANGE     0x0037 /* `operator [:]= (begin: Object, end: Object, value: Object): type(value)' - `__setrange__' - `tp_range_set'. */
#define OPERATOR_GETATTR      0x0038 /* `operator . (string attr): Object'                                - `__getattr__'     - `tp_getattr'. */
#define OPERATOR_DELATTR      0x0039 /* `operator del . (string attr): none'                              - `__delattr__'     - `tp_delattr'. */
#define OPERATOR_SETATTR      0x003a /* `operator . = (string attr, value: Object): type(value)'          - `__setattr__'     - `tp_setattr'. */
#define OPERATOR_ENUMATTR     0x003b /* `operator enumattr(): {attribute...}'                             - `__enumattr__'    - `tp_enumattr'. */
#define OPERATOR_ENTER        0x003c /* `operator enter(): none'                                          - `__enter__'       - `tp_enter'. */
#define OPERATOR_LEAVE        0x003d /* `operator leave(): none'                                          - `__leave__'       - `tp_leave'. */
#define OPERATOR_USERCOUNT    0x003e /* Number of user-accessible operators. (Used by `class' types) */
#define OPERATOR_EXTENDED(x) (0x1000+(x)) /* Extended operator codes. (Type-specific; may be re-used) */
#define OPERATOR_ISINPLACE(x) ((x) >= OPERATOR_INC && (x) <= OPERATOR_INPLACE_POW)

/* Operators not exposed to user-code. */
#define OPERATOR_VISIT        0x8000 /* `tp_visit'. */
#define OPERATOR_CLEAR        0x8001 /* `tp_clear'. */
#define OPERATOR_PCLEAR       0x8002 /* `tp_pclear'. */
#define OPERATOR_GETBUF       0x8003 /* `tp_getbuf'. */

/* Operator association ranges. */
#define OPERATOR_TYPEMIN    OPERATOR_CONSTRUCTOR
#define OPERATOR_TYPEMAX    OPERATOR_CALL
#define OPERATOR_MATHMIN    OPERATOR_INT
#define OPERATOR_MATHMAX    OPERATOR_INPLACE_POW
#define OPERATOR_CMPMIN     OPERATOR_HASH
#define OPERATOR_CMPMAX     OPERATOR_GE
#define OPERATOR_SEQMIN     OPERATOR_ITERSELF
#define OPERATOR_SEQMAX     OPERATOR_SETRANGE
#define OPERATOR_ATTRMIN    OPERATOR_GETATTR
#define OPERATOR_ATTRMAX    OPERATOR_ENUMATTR
#define OPERATOR_WITHMIN    OPERATOR_ENTER
#define OPERATOR_WITHMAX    OPERATOR_LEAVE
#define OPERATOR_PRIVMIN    OPERATOR_VISIT
#define OPERATOR_PRIVMAX    OPERATOR_GETBUF
#endif /* DEE_SOURCE */


#ifdef DEE_SOURCE
/* Operator types (values for `oi_type') */
#define OPTYPE_SPECIAL        0xfff  /* VALUE: A special operator that cannot be invoked directly (e.g.: `__constructor__'). */
#define OPTYPE_UNARY          0x001  /* `...(DeeObject *__restrict self);' */
#define OPTYPE_BINARY         0x002  /* `...(DeeObject *__restrict self, DeeObject *__restrict other);' */
#define OPTYPE_TRINARY        0x003  /* `...(DeeObject *__restrict self, DeeObject *__restrict a, DeeObject *__restrict b);' */
#define OPTYPE_QUAD           0x004  /* `...(DeeObject *__restrict self, DeeObject *__restrict a, DeeObject *__restrict b, DeeObject *__restrict c);' */
#define OPTYPE_INPLACE        0x008  /* FLAG: [override] The first argument is actually `DeeObject **__restrict pself'. */
#define OPTYPE_VARIABLE       0x080  /* FLAG: User-code can invoke the operator with a variable number of arguments passed through the last operand. */
#define OPTYPE_ROBJECT        0x000  /* FLAG: The operator returns `DREF DeeObject *'. */
#define OPTYPE_RINT32         0x010  /* FLAG: The operator returns `int32_t'. */
#define OPTYPE_RINT64         0x020  /* FLAG: The operator returns `int64_t'. */
#define OPTYPE_RUINT32        0x030  /* FLAG: The operator returns `uint32_t'. */
#define OPTYPE_RUINT64        0x040  /* FLAG: The operator returns `uint64_t'. */
#if __SIZEOF_INT__ >= 8
#   define OPTYPE_RINT        OPTYPE_RINT64
#   define OPTYPE_RUINT       OPTYPE_RUINT64
#else /* __SIZEOF_POINTER__ >= 8 */
#   define OPTYPE_RINT        OPTYPE_RINT32
#   define OPTYPE_RUINT       OPTYPE_RUINT32
#endif /* __SIZEOF_POINTER__ < 8 */
#if __SIZEOF_POINTER__ >= 8
#   define OPTYPE_RINTPTR     OPTYPE_RINT64
#   define OPTYPE_RUINTPTR    OPTYPE_RUINT64
#else /* __SIZEOF_POINTER__ >= 8 */
#   define OPTYPE_RINTPTR     OPTYPE_RINT32
#   define OPTYPE_RUINTPTR    OPTYPE_RUINT32
#endif /* __SIZEOF_POINTER__ < 8 */
#define OPTYPE_RVOID          0x0f0  /* FLAG: The operator returns `void'. */
 /* Misc/special operator callback types. */
#define OPTYPE_VISIT         (0x103 | OPTYPE_RVOID)   /* Special type: `void DCALL(DeeObject *__restrict self, Dee_visit_t proc, void *arg);' */
#define OPTYPE_ENUMATTR      (0x204 | OPTYPE_RINT)    /* Special type: `int DCALL(DeeTypeObject *__restrict tp_self, DeeObject *self, Dee_enum_t proc, void *arg);' */
#define OPTYPE_DOUBLE        (0x302 | OPTYPE_RINT)    /* Special type: `int DCALL(DeeObject *__restrict self, double *__restrict result);' */
#define OPTYPE_READWRITE     (0x403 | OPTYPE_RINTPTR) /* Special type: `intptr_t DCALL(DeeObject *__restrict self, void [const] *__restrict buffer, size_t bufsize);' */
#define OPTYPE_PREADWRITE    (0x504 | OPTYPE_RINTPTR) /* Special type: `intptr_t DCALL(DeeObject *__restrict self, void [const] *__restrict buffer, size_t bufsize, uint64_t pos);' */
#define OPTYPE_SEEK          (0x603 | OPTYPE_RINT64)  /* Special type: `int64_t DCALL(DeeObject *__restrict self, int64_t offset, int whence);' */
#define OPTYPE_TRUNC         (0x703 | OPTYPE_RINT)    /* Special type: `int DCALL(DeeObject *__restrict self, uint64_t length);' */
#define OPTYPE_UNGETPUT      (0x802 | OPTYPE_RINT)    /* Special type: `int DCALL(DeeObject *__restrict self, int ch);' */
#define OPTYPE_PCLEAR        (0x902 | OPTYPE_RVOID)   /* Special type: `void DCALL(DeeObject *__restrict self, int priority);' */
#define OPTYPE_GETBUF        (0xa02 | OPTYPE_RINT)    /* Special type: `int DCALL(DeeObject *__restrict self, DeeBuffer *__restrict info, unsigned int flags);' */

/* Operator classes (values for `oi_class') */
#define OPCLASS_TYPE   0 /* `oi_offset' points into `DeeTypeObject'. */
#define OPCLASS_GC     1 /* `oi_offset' points into `struct Dee_type_gc'. */
#define OPCLASS_MATH   2 /* `oi_offset' points into `struct Dee_type_math'. */
#define OPCLASS_CMP    3 /* `oi_offset' points into `struct Dee_type_cmp'. */
#define OPCLASS_SEQ    4 /* `oi_offset' points into `struct Dee_type_seq'. */
#define OPCLASS_ATTR   5 /* `oi_offset' points into `struct Dee_type_attr'. */
#define OPCLASS_WITH   6 /* `oi_offset' points into `struct Dee_type_with'. */
#define OPCLASS_BUFFER 7 /* `oi_offset' points into `struct Dee_type_buffer'. */
#endif /* DEE_SOURCE */

struct Dee_opinfo {
	unsigned int const oi_type : 12;    /* The operator must be used as inplace. */
	unsigned int const oi_class : 3;    /* The class associated with the operator (One of `OPCLASS_*') */
	unsigned int const oi_private : 1;  /* The operator is private. */
	uint16_t const     oi_offset;       /* Offset to where the c-function of this operator can be found. */
	char const         oi_uname[12];    /* `+' */
	char const         oi_sname[12];    /* `add' */
	char const         oi_iname[16];    /* `tp_add' */
};

/* Returns an operator information descriptor for a given operator number.
 * Returns NULL if the given operator is not known.
 * @param: typetype: The type-type implementing the operator, or `NULL' to
 *                   behave identical to `typetype' being `DeeType_Type',
 *                   in which case only operators implemented by all types
 *                   can be queried. */
DFUNDEF WUNUSED struct Dee_opinfo const *DCALL
Dee_OperatorInfo(DeeTypeObject *typetype, uint16_t id);

/* Lookup an operator, given its name, and returning its id.
 * The given `name' is compared against both the `oi_sname' field
 * of all operators, as well as the `oi_uname' field, however only
 * for operators where `oi_uname' isn't ambiguous (aka. `+' is intentionally
 * not resolved by this function, and causes `(uint16_t)-1' to be returned,
 * though `*' is resolved and causes `OPERATOR_MUL' to be returned)
 * @return: (uint16_t)-1: No operator matching `name' exists (no error was thrown). */
DFUNDEF WUNUSED NONNULL((2)) uint16_t DCALL
Dee_OperatorFromName(DeeTypeObject *typetype,
                     char const *__restrict name);
DFUNDEF WUNUSED NONNULL((2)) uint16_t DCALL
Dee_OperatorFromNameLen(DeeTypeObject *typetype,
                        char const *__restrict name,
                        size_t namelen);


/* Invoke an operator on a given object, given its ID and arguments.
 * NOTE: Using these function, any operator can be invoked, including
 *       extension operators as well as some operators marked as
 *      `OPTYPE_SPECIAL' (most notably: `tp_int'), as well as throwing
 *       a `Signal.StopIteration' when `tp_iter_next' is exhausted.
 * Special handling is performed for the read/write operators
 * of `DeeFileType_Type', which are invoked by passing string
 * objects back/forth:
 *    - operator read(): string;
 *    - operator read(int max_bytes): string;
 *    - operator write(string data): int;
 *    - operator write(string data, int max_bytes): int;
 *    - operator write(string data, int begin, int end): int;
 *    - operator pread(int pos): string;
 *    - operator pread(int max_bytes, int pos): string;
 *    - operator pwrite(string data, int pos): int;
 *    - operator pwrite(string data, int max_bytes, int pos): int;
 *    - operator pwrite(string data, int begin, int end, int pos): int;
 * Operators marked as `oi_private' cannot be invoked and
 * attempting to do so will cause an `Error.TypeError' to be thrown.
 * Attempting to invoke an unknown operator will cause an `Error.TypeError' to be thrown.
 * HINT: `DeeObject_PInvokeOperator' can be used the same way `DeeObject_InvokeOperator'
 *        can be, with the addition of allowing inplace operators to be executed.
 *        Attempting to execute an inplace operator using `DeeObject_InvokeOperator()'
 *        will cause an `Error.TypeError' to be thrown. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_InvokeOperator(DeeObject *self, uint16_t name,
                         size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_PInvokeOperator(DeeObject **__restrict pself, uint16_t name,
                          size_t argc, DeeObject *const *argv);
#define DeeObject_InvokeOperatorTuple(self, name, args) \
	DeeObject_InvokeOperator(self, name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_PInvokeOperatorTuple(pself, name, args) \
	DeeObject_PInvokeOperator(pself, name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))


#define Dee_TP_FNORMAL          0x0000 /* Normal type flags. */
#define Dee_TP_FFINAL           0x0001 /* The class cannot be sub-classed again. */
#define Dee_TP_FTRUNCATE        0x0002 /* Truncate values during integer conversion, rather than throwing an `OverflowError'. */
#define Dee_TP_FINTERRUPT       0x0004 /* This type is a so-called interrupt signal.
                                        * Instances of this type have special behavior when thrown as errors,
                                        * or when delivered to threads through use of `thread.interrupt()'
                                        * In such situations, the error can only be caught by exception handlers
                                        * specifically marked as `@[interrupt]' (or rather `EXCEPTION_HANDLER_FINTERPT')
                                        * Additionally (but only when `CONFIG_NO_THREADS' is disabled), such errors are
                                        * re-scheduled in the pending-interrupt system of the calling thread when they
                                        * are thrown in a context where errors are normally discarded (such as destructions
                                        * or secondary exceptions in user-code functions)
                                        * WARNING: This flag must be inherited explicitly by sub-classes! */
#define Dee_TP_FMOVEANY         0x0008 /* The type accepts any other object as second operator to `tp_move_assign' */
/*      Dee_TP_F                0x0010  * ... */
/*      Dee_TP_F                0x0020  * ... */
#define Dee_TP_FINHERITCTOR     0x0040 /* This type inherits its constructor from `tp_base'.
                                        * Additionally, if no assign/move-assign operator is defined, those are inherited as well. */
#define Dee_TP_FABSTRACT        0x0080 /* Member functions and getsets of this type are type-generic and may
                                        * even be invoked when being passed objects that do not fulfill the
                                        * requirement of `DeeObject_InstanceOf(ob, self)' (where `self' is this type).
                                        * For example: Abstract base classes have this flag set, such as `object', `sequence' or `iterator'
                                        * NOTE: This flag is not inherited.
                                        * When this flag is set, the type may also be used to construct super-wrappers
                                        * for any other kind of object, even if that object isn't derived from the type. */
/*      Dee_TP_F                0x0080  * ... */
/*      Dee_TP_F                0x0100  * ... */
/*      Dee_TP_F                0x0200  * ... */
#define Dee_TP_FNAMEOBJECT      0x0400 /* `tp_name' actually points to the `s_str' member of a `string_object' that this type holds a reference to. */
#define Dee_TP_FDOCOBJECT       0x0800 /* `tp_doc' actually points to the `s_str' member of a `string_object' that this type holds a reference to. */
/*      Dee_TP_F                0x1000  * ... */
#define Dee_TP_FVARIABLE        0x2000 /* Variable-length object type. (`tp_new' is used, rather than `tp_init') */
#define Dee_TP_FGC              0x4000 /* Instance of this type can be harvested by the Garbage Collector. */
#define Dee_TP_FHEAP            0x8000 /* This type was allocated on the heap. */
#define Dee_TP_FINTERHITABLE   (Dee_TP_FINTERRUPT | Dee_TP_FVARIABLE | Dee_TP_FGC) \
                                       /* Set of special flags that is inherited by sub-classes. */

#define Dee_TF_NONE             0x00000000 /* No special features. */
#define Dee_TF_HASFILEOPS       0x00000001 /* The type implements file operations (for types typed as DeeFileType_Type). */
#define Dee_TF_NONLOOPING       0x00000002 /* The object's tp_visit function can be skipped when searching for conventional reference loops.
                                            * This flag may be set when it is known that an object will only ever point to other objects
                                            * that either don't point to any objects, or are guarantied to never point back.
                                            * An example for where this flag should be used would be an object that only ever
                                            * holds references to `String' or `int' objects, but not to objects of its own type,
                                            * or any sort of container object capable of holding instances of the same type. */
#define Dee_TF_SINGLETON        0x80000000 /* This type is a singleton. */

#ifdef DEE_SOURCE
#define TP_FNORMAL          Dee_TP_FNORMAL
#define TP_FFINAL           Dee_TP_FFINAL
#define TP_FTRUNCATE        Dee_TP_FTRUNCATE
#define TP_FINTERRUPT       Dee_TP_FINTERRUPT
#define TP_FMOVEANY         Dee_TP_FMOVEANY
#define TP_FINHERITCTOR     Dee_TP_FINHERITCTOR
#define TP_FABSTRACT        Dee_TP_FABSTRACT
#define TP_FNAMEOBJECT      Dee_TP_FNAMEOBJECT
#define TP_FDOCOBJECT       Dee_TP_FDOCOBJECT
#define TP_FVARIABLE        Dee_TP_FVARIABLE
#define TP_FGC              Dee_TP_FGC
#define TP_FHEAP            Dee_TP_FHEAP
#define TP_FINTERHITABLE    Dee_TP_FINTERHITABLE
#define TF_NONE             Dee_TF_NONE
#define TF_HASFILEOPS       Dee_TF_HASFILEOPS
#define TF_NONLOOPING       Dee_TF_NONLOOPING
#define TF_SINGLETON        Dee_TF_SINGLETON
#endif /* DEE_SOURCE */


#define Dee_ITER_ISOK(x) (((uintptr_t)(x) - 1) < (uintptr_t)-2l) /* `x != NULL && x != Dee_ITER_DONE' */
#define Dee_ITER_DONE    ((DeeObject *)-1l) /* Returned when the iterator has been exhausted. */

#ifdef DEE_SOURCE
#define ITER_ISOK           Dee_ITER_ISOK /* `x != NULL && x != ITER_DONE' */
#define ITER_DONE           Dee_ITER_DONE /* Returned when the iterator has been exhausted. */
#endif /* DEE_SOURCE */


struct Dee_type_object {
	Dee_OBJECT_HEAD
	char const                         *tp_name;     /* [0..1] Name of this type. */
	/*utf-8*/ char const               *tp_doc;      /* [0..1] Documentation string of this type and its operators. */
	uint16_t                            tp_flags;    /* Type flags (Set of `TP_F*'). */
	uint16_t                            tp_weakrefs; /* Offset to `offsetof(Dee?Object, ob_weakrefs)', or 0 when not supported.
	                                                  * NOTE: Must be explicitly inherited by derived types.
	                                                  * NOTE: This member must explicitly be initialized during object construction
	                                                  *       using `weakref_support_init' and `weakref_support_fini', during destruction. */
	uint32_t                            tp_features; /* Type sub-class specific features (Set of `TF_*'). */
	DREF DeeTypeObject                 *tp_base;     /* [0..1][const] Base class.
	                                                  * NOTE: When the `TP_FINHERITCTOR' flag is set, then this field must be non-NULL. */
	struct Dee_type_constructor         tp_init;     /* Constructor/destructor operators. */
	struct Dee_type_cast                tp_cast;     /* Type casting operators. */
	WUNUSED NONNULL((1))
	DREF DeeObject             *(DCALL *tp_call)(DeeObject *self, size_t argc, DeeObject *const *argv);
	NONNULL((1, 2)) void        (DCALL *tp_visit)(DeeObject *__restrict self, Dee_visit_t proc, void *arg); /* Visit all reachable, referenced (DREF) objected. */
	/* NOTE: Anything used by `type_inherit_*' can't be made `Dee_tpconst' here! */
	struct Dee_type_gc Dee_tpconst     *tp_gc;       /* [0..1] GC related operators. */
	struct Dee_type_math               *tp_math;     /* [0..1][owned_if(tp_class != NULL)] Math related operators. */
	struct Dee_type_cmp                *tp_cmp;      /* [0..1][owned_if(tp_class != NULL)] Compare operators. */
	struct Dee_type_seq                *tp_seq;      /* [0..1][owned_if(tp_class != NULL)] Sequence operators. */
	WUNUSED NONNULL((1))
	DREF DeeObject             *(DCALL *tp_iter_next)(DeeObject *__restrict self);
	struct Dee_type_attr Dee_tpconst   *tp_attr;     /* [0..1][owned_if(tp_class != NULL)] Attribute access operators. */
	struct Dee_type_with               *tp_with;     /* [0..1][owned_if(tp_class != NULL)] __enter__ / __leave__ operators. */
	struct Dee_type_buffer             *tp_buffer;   /* [0..1] Raw buffer interface. */
	/* NOTE: All of the following as sentinel-terminated vectors. */
	struct Dee_type_method Dee_tpconst *tp_methods;  /* [0..1] Instance methods. */
	struct Dee_type_getset Dee_tpconst *tp_getsets;  /* [0..1] Instance getsets. */
	struct Dee_type_member Dee_tpconst *tp_members;  /* [0..1] Instance member fields. */
	struct Dee_type_method Dee_tpconst *tp_class_methods; /* [0..1] Class methods. */
	struct Dee_type_getset Dee_tpconst *tp_class_getsets; /* [0..1] Class getsets. */
	struct Dee_type_member Dee_tpconst *tp_class_members; /* [0..1] Class members (usually constants). */
	/* [0..1] Same as `tp_call', but using keywords. */
	WUNUSED NONNULL((1))
	DREF DeeObject *(DCALL *tp_call_kw)(DeeObject *self, size_t argc,
	                                    DeeObject *const *argv, DeeObject *kw);
	/* Lazily-filled hash-table of instance members.
	 * >> The member vectors are great for static allocation, but walking
	 *    all of them each time a member is accessed is way too slow.
	 *    So instead, we should cache members and sort them by first-visible on a per-type basis.
	 *    That way, we'll greatly optimize the lookup time for existing members. */
	struct Dee_membercache  tp_cache;
	struct Dee_membercache  tp_class_cache;
	struct Dee_class_desc  *tp_class;    /* [0..1] Class descriptor (Usually points below this type object). */
	Dee_WEAKREF_SUPPORT     /* Weak reference support. */
	/* ... Extended type fields go here (e.g.: `DeeFileTypeObject') */
	/* ... `struct class_desc' of class types goes here */
};
#define DeeType_IsFinal(x)               (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FFINAL)
#define DeeType_IsInterrupt(x)           (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FINTERRUPT)
#define DeeType_IsAbstract(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FABSTRACT)
#define DeeType_IsVariable(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FVARIABLE)
#define DeeType_IsGC(x)                  (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FGC)
#define DeeType_IsClass(x)               (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_class != NULL)
#define DeeType_IsArithmetic(x)          (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_math != NULL)
#define DeeType_IsComparable(x)          (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_cmp != NULL)
#define DeeType_IsSequence(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_seq != NULL)
#define DeeType_IsIntTruncated(x)        (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FTRUNCATE)
#define DeeType_HasMoveAny(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FMOVEANY)
#define DeeType_IsIterator(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_iter_next != NULL)
#define DeeType_IsTypeType(x)            DeeType_IsInherited((DeeTypeObject *)Dee_REQUIRES_OBJECT(x), &DeeType_Type)
#define DeeType_IsCustom(x)              (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FHEAP) /* Custom types are those not pre-defined, but created dynamically. */
#define DeeType_IsSuperConstructible(x)  (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_flags & Dee_TP_FINHERITCTOR)
#define DeeType_IsNoArgConstructible(x)  (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_init.tp_alloc.tp_ctor != NULL)
#define DeeType_IsVarArgConstructible(x) (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_init.tp_alloc.tp_any_ctor != NULL || ((DeeTypeObject *)(x))->tp_init.tp_alloc.tp_any_ctor_kw != NULL)
#define DeeType_IsConstructible(x)       (DeeType_IsSuperConstructible(x) || DeeType_IsNoArgConstructible(x) || DeeType_IsVarArgConstructible(x))
#define DeeType_IsCopyable(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_init.tp_alloc.tp_copy_ctor != NULL || ((DeeTypeObject *)(x))->tp_init.tp_alloc.tp_deep_ctor != NULL)
#define DeeType_Base(x)                  (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_base)
#define DeeType_GCPriority(x)            (((DeeTypeObject *)Dee_REQUIRES_OBJECT(x))->tp_gc ? ((DeeTypeObject *)(x))->tp_gc->tp_gcprio : Dee_GC_PRIORITY_LATE)
#define DeeObject_GCPriority(x)          DeeType_GCPriority(Dee_TYPE(x))
#define DeeObject_IsInterrupt(x)         DeeType_IsInterrupt(Dee_TYPE(x))


/* Check if `name' is being implemented by the given path, or has been inherited by a base-type. */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeType_HasOperator(DeeTypeObject *__restrict self, uint16_t name);

/* Same as `DeeType_HasOperator()', however don't return `true' if the
 * operator has been inherited implicitly through caching mechanisms. */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeType_HasPrivateOperator(DeeTypeObject *__restrict self, uint16_t name);

#ifdef CONFIG_BUILDING_DEEMON
/* Inherit different groups of operators from base-classes, returning `true'
 * if operators were inherited from some base class (even if those same operators
 * had already been inherited previously), and `false' if no base-class provides
 * the any of the specified operators (though note that inheriting constructors
 * requires that all base classes carry the `Dee_TP_FINHERITCTOR' flag; else,
 * a class without this flag cannot inherit constructs from its base, though
 * can still provide its constructs to some derived class that does specify its
 * intend of inheriting constructors) */
INTDEF NONNULL((1)) bool DCALL type_inherit_constructors(DeeTypeObject *__restrict self);  /* tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_assign, tp_move_assign, tp_deepload */
INTDEF NONNULL((1)) bool DCALL type_inherit_str(DeeTypeObject *__restrict self);           /* tp_str */
INTDEF NONNULL((1)) bool DCALL type_inherit_repr(DeeTypeObject *__restrict self);          /* tp_repr */
INTDEF NONNULL((1)) bool DCALL type_inherit_bool(DeeTypeObject *__restrict self);          /* tp_bool */
INTDEF NONNULL((1)) bool DCALL type_inherit_call(DeeTypeObject *__restrict self);          /* tp_call */
INTDEF NONNULL((1)) bool DCALL type_inherit_hash(DeeTypeObject *__restrict self);          /* tp_hash */
INTDEF NONNULL((1)) bool DCALL type_inherit_int(DeeTypeObject *__restrict self);           /* tp_int, tp_int32, tp_int64, tp_double */
INTDEF NONNULL((1)) bool DCALL type_inherit_inv(DeeTypeObject *__restrict self);           /* tp_inv */
INTDEF NONNULL((1)) bool DCALL type_inherit_pos(DeeTypeObject *__restrict self);           /* tp_pos */
INTDEF NONNULL((1)) bool DCALL type_inherit_neg(DeeTypeObject *__restrict self);           /* tp_neg */
INTDEF NONNULL((1)) bool DCALL type_inherit_add(DeeTypeObject *__restrict self);           /* tp_add, tp_sub, tp_inplace_add, tp_inplace_sub, tp_inc, tp_dec */
INTDEF NONNULL((1)) bool DCALL type_inherit_mul(DeeTypeObject *__restrict self);           /* tp_mul, tp_inplace_mul */
INTDEF NONNULL((1)) bool DCALL type_inherit_div(DeeTypeObject *__restrict self);           /* tp_div, tp_inplace_div */
INTDEF NONNULL((1)) bool DCALL type_inherit_mod(DeeTypeObject *__restrict self);           /* tp_mod, tp_inplace_mod */
INTDEF NONNULL((1)) bool DCALL type_inherit_shl(DeeTypeObject *__restrict self);           /* tp_shl, tp_inplace_shl */
INTDEF NONNULL((1)) bool DCALL type_inherit_shr(DeeTypeObject *__restrict self);           /* tp_shr, tp_inplace_shr */
INTDEF NONNULL((1)) bool DCALL type_inherit_and(DeeTypeObject *__restrict self);           /* tp_and, tp_inplace_and */
INTDEF NONNULL((1)) bool DCALL type_inherit_or(DeeTypeObject *__restrict self);            /* tp_or, tp_inplace_or */
INTDEF NONNULL((1)) bool DCALL type_inherit_xor(DeeTypeObject *__restrict self);           /* tp_xor, tp_inplace_xor */
INTDEF NONNULL((1)) bool DCALL type_inherit_pow(DeeTypeObject *__restrict self);           /* tp_pow, tp_inplace_pow */
INTDEF NONNULL((1)) bool DCALL type_inherit_compare(DeeTypeObject *__restrict self);       /* tp_eq, tp_ne, tp_lo, tp_le, tp_gr, tp_ge */
INTDEF NONNULL((1)) bool DCALL type_inherit_iternext(DeeTypeObject *__restrict self);      /* tp_iter_next */
INTDEF NONNULL((1)) bool DCALL type_inherit_iterself(DeeTypeObject *__restrict self);      /* tp_iter_self */
INTDEF NONNULL((1)) bool DCALL type_inherit_size(DeeTypeObject *__restrict self);          /* tp_size */
INTDEF NONNULL((1)) bool DCALL type_inherit_contains(DeeTypeObject *__restrict self);      /* tp_contains */
INTDEF NONNULL((1)) bool DCALL type_inherit_getitem(DeeTypeObject *__restrict self);       /* tp_get */
INTDEF NONNULL((1)) bool DCALL type_inherit_delitem(DeeTypeObject *__restrict self);       /* tp_del */
INTDEF NONNULL((1)) bool DCALL type_inherit_setitem(DeeTypeObject *__restrict self);       /* tp_set */
INTDEF NONNULL((1)) bool DCALL type_inherit_getrange(DeeTypeObject *__restrict self);      /* tp_range_get */
INTDEF NONNULL((1)) bool DCALL type_inherit_delrange(DeeTypeObject *__restrict self);      /* tp_range_del */
INTDEF NONNULL((1)) bool DCALL type_inherit_setrange(DeeTypeObject *__restrict self);      /* tp_range_set */
INTDEF NONNULL((1)) bool DCALL type_inherit_nsi(DeeTypeObject *__restrict self);           /* tp_nsi (only succeeds if `self' has no `tp_seq' field) */
INTDEF NONNULL((1)) bool DCALL type_inherit_with(DeeTypeObject *__restrict self);          /* tp_enter, tp_leave */
INTDEF NONNULL((1)) bool DCALL type_inherit_buffer(DeeTypeObject *__restrict self);        /* tp_getbuf, tp_putbuf, tp_buffer_flags */
#endif /* CONFIG_BUILDING_DEEMON */


/* Generic attribute lookup through `tp_self[->tp_base...]->tp_methods, tp_getsets, tp_members'
 * @return: -1 / ---   / NULL:          Error.
 * @return:  0 / true  / * :            OK.
 * @return:  1 / false / Dee_ITER_DONE: Not found. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericGetAttrString(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericGetAttrStringLen(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericCallAttrString(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringLen(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringKw(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TGenericCallAttrStringLenKw(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericDelAttrString(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericDelAttrStringLen(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL DeeObject_TGenericSetAttrString(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL DeeObject_TGenericSetAttrStringLen(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_GenericEnumAttr(DeeTypeObject *__restrict tp_self, Dee_enum_t proc, void *arg);
#define DeeObject_GenericGetAttrString(self, attr, hash)                                DeeObject_TGenericGetAttrString(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericGetAttrStringLen(self, attr, attrlen, hash)                    DeeObject_TGenericGetAttrStringLen(Dee_TYPE(self), self, attr, attrlen, hash)
#define DeeObject_GenericCallAttrString(self, attr, hash, argc, argv)                   DeeObject_TGenericCallAttrString(Dee_TYPE(self), self, attr, hash, argc, argv)
#define DeeObject_GenericCallAttrStringLen(self, attr, attrlen, hash, argc, argv)       DeeObject_TGenericCallAttrStringLen(Dee_TYPE(self), self, attr, attrlen, hash, argc, argv)
#define DeeObject_GenericCallAttrStringKw(self, attr, hash, argc, argv, kw)             DeeObject_TGenericCallAttrStringKw(Dee_TYPE(self), self, attr, hash, argc, argv, kw)
#define DeeObject_GenericCallAttrStringLenKw(self, attr, attrlen, hash, argc, argv, kw) DeeObject_TGenericCallAttrStringLenKw(Dee_TYPE(self), self, attr, attrlen, hash, argc, argv, kw)
#define DeeObject_GenericDelAttrString(self, attr, hash)                                DeeObject_TGenericDelAttrString(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericDelAttrStringLen(self, attr, attrlen, hash)                    DeeObject_TGenericDelAttrStringLen(Dee_TYPE(self), self, attr, attrlen, hash)
#define DeeObject_GenericSetAttrString(self, attr, hash, value)                         DeeObject_TGenericSetAttrString(Dee_TYPE(self), self, attr, hash, value)
#define DeeObject_GenericSetAttrStringLen(self, attr, attrlen, hash, value)             DeeObject_TGenericSetAttrStringLen(Dee_TYPE(self), self, attr, attrlen, hash, value)

#ifdef CONFIG_BUILDING_DEEMON
struct attribute_info;
struct attribute_lookup_rules;
INTERN WUNUSED NONNULL((1, 2)) bool DCALL DeeObject_TGenericHasAttrString(DeeTypeObject *__restrict tp_self, char const *__restrict attr, Dee_hash_t hash);
INTERN WUNUSED NONNULL((1, 2)) bool DCALL DeeObject_TGenericHasAttrStringLen(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericBoundAttrString(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr, Dee_hash_t hash); /* -2 / -3: not found; -1: error; 0: unbound; 1: bound; */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TGenericBoundAttrStringLen(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash); /* -2 / -3: not found; -1: error; 0: unbound; 1: bound; */
INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL DeeObject_GenericFindAttrString(DeeTypeObject *tp_self, DeeObject *instance, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#define DeeObject_GenericBoundAttrString(self, attr, hash)             DeeObject_TGenericBoundAttrString(Dee_TYPE(self), self, attr, hash)
#define DeeObject_GenericBoundAttrStringLen(self, attr, attrlen, hash) DeeObject_TGenericBoundAttrStringLen(Dee_TYPE(self), self, attr, attrlen, hash)
#define DeeObject_GenericHasAttrString(self, attr, hash)               DeeObject_TGenericHasAttrString(Dee_TYPE(self), attr, hash)
#define DeeObject_GenericHasAttrStringLen(self, attr, attrlen, hash)   DeeObject_TGenericHasAttrStringLen(Dee_TYPE(self), attr, attrlen, hash)
#endif /* CONFIG_BUILDING_DEEMON */


DDATDEF DeeTypeObject DeeObject_Type; /* `object' */
DDATDEF DeeTypeObject DeeType_Type;   /* `type(object)' */
#define DeeType_Check(ob)      DeeObject_InstanceOf(ob, &DeeType_Type)
#define DeeType_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeType_Type)

/* Assert the typing of an object (raising an `Error.TypeError' if the type wasn't expected)
 * HINT: When `required_type' isn't a type-object, these functions throw an error!
 * @return: -1: The object doesn't match the required typing.
 * @return:  0: The object matches the required typing. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AssertType)(DeeObject *self, DeeTypeObject *required_type);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AssertTypeExact)(DeeObject *self, DeeTypeObject *required_type);
/* Throw a TypeError stating that an instance of `required_type' was required, when `self' was given. */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeObject_TypeAssertFailed)(DeeObject *self, DeeTypeObject *required_type);
#define DeeObject_AssertTypeOrNone(self, required_type) \
	(DeeNone_Check(self) ? 0 : DeeObject_AssertType(self, required_type))
#define DeeObject_AssertTypeExactOrNone(self, required_type) \
	(DeeNone_CheckExact(self) ? 0 : DeeObject_AssertTypeExact(self, required_type))
#define DeeObject_AssertType(self, required_type) \
	(unlikely((DeeObject_AssertType)((DeeObject *)Dee_REQUIRES_OBJECT(self), required_type)))
#ifndef __OPTIMIZE_SIZE__
#define DeeObject_AssertTypeExact(self, required_type) \
	(unlikely(Dee_TYPE(self) == required_type ? 0 : DeeObject_TypeAssertFailed((DeeObject *)(self), required_type)))
#else /* !__OPTIMIZE_SIZE__ */
#define DeeObject_AssertTypeExact(self, required_type) \
	(unlikely((DeeObject_AssertTypeExact)((DeeObject *)Dee_REQUIRES_OBJECT(self), required_type)))
#endif /* __OPTIMIZE_SIZE__ */


/* Object typeof(). */
#define Dee_TYPE(self) ((self)->ob_type)

/* Returns the class of `self', automatically
 * dereferencing super-objects and other wrappers.
 * Beyond that, this function returns the same as `Dee_TYPE()' */
DFUNDEF WUNUSED ATTR_RETNONNULL NONNULL((1)) DeeTypeObject *DCALL
DeeObject_Class(DeeObject *__restrict self);

/* Object inheritance checking. */
#define DeeObject_InstanceOf(self, super_type)       DeeType_IsInherited(Dee_TYPE(self), super_type)
#define DeeObject_InstanceOfExact(self, object_type) (Dee_TYPE(self) == (object_type))

/* Return true if `test_type' is equal to, or derived from `inherited_type'
 * NOTE: When `inherited_type' is not a type, this function simply returns `false'
 * >> return inherited_type is Type && inherited_type.baseof(test_type); */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeType_IsInherited(DeeTypeObject *test_type,
                    DeeTypeObject *inherited_type);

/* Return the module used to define a given type `self',
 * or `NULL' if that module could not be determined.
 * NOTE: When `NULL' is returned, _NO_ error is thrown! */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeType_GetModule(DeeTypeObject *__restrict self);

/* Object creation (constructor invocation). */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_NewDefault(DeeTypeObject *__restrict object_type);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeObject_New(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeObject_NewKw(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *DeeObject_NewPack(DeeTypeObject *object_type, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_VNewPack(DeeTypeObject *object_type, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeObject_Newf(DeeTypeObject *object_type, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_VNewf(DeeTypeObject *object_type, char const *__restrict format, va_list args);

/* Object copy/assign operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Copy(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DeepCopy(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_InplaceDeepCopy(/*in|out*/ DREF DeeObject **__restrict pself);
#define DeeObject_XInplaceDeepCopy(pself) (!*(pself) ? 0 : DeeObject_InplaceDeepCopy(pself))
#ifndef CONFIG_NO_THREADS
/* A helper functions to acquire the proper read/write locks on a given
 * Dee_rwlock_t when accessing memory pointed to by the given `*pself'.
 * This is highly useful due to the fact that practically the only
 * place where `DeeObject_InplaceDeepCopy()' might ever be encountered,
 * is in tp_deepload operators, where using it on a raw pointer is
 * not thread-safe, considering the fact that the caller must probably
 * be holding some kind of lock (presumably an `Dee_rwlock_t'), which must
 * be used in the following order to safely replace the field with a
 * deepcopy (the safe order of operations that is then performed by this helper):
 * >> DREF DeeObject *temp, *copy;
 * >> rwlock_read(plock);
 * >> temp = *pself;
 * >> #if IS_XDEEPCOPY
 * >> if (!temp) {
 * >>     rwlock_endread(plock);
 * >>     return 0;
 * >> }
 * >> #endif // IS_XDEEPCOPY
 * >> Dee_Incref(temp);
 * >> rwlock_endread(plock);
 * >> copy = DeeObject_DeepCopy(temp);
 * >> Dee_Decref(temp);
 * >> if unlikely(!copy)
 * >>    return -1;
 * >> rwlock_write(plock);
 * >> temp   = *pself; // Inherit
 * >> *pself = copy;   // Inherit
 * >> rwlock_endwrite(plock);
 * >> #if IS_XDEEPCOPY
 * >> Dee_XDecref(temp);
 * >> #else // IS_XDEEPCOPY
 * >> Dee_Decref(temp);
 * >> #endif // !IS_XDEEPCOPY
 * >> return 0;
 */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_InplaceDeepCopyWithLock(/*in|out*/ DREF DeeObject **__restrict pself, Dee_rwlock_t *__restrict plock);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_XInplaceDeepCopyWithLock(/*in|out*/ DREF DeeObject **__restrict pself, Dee_rwlock_t *__restrict plock);
#else /* !CONFIG_NO_THREADS */
#define DeeObject_InplaceDeepCopyWithLock(pself, plock)  DeeObject_InplaceDeepCopy(pself)
#define DeeObject_XInplaceDeepCopyWithLock(pself, plock) DeeObject_XInplaceDeepCopy(pself)
#endif /* CONFIG_NO_THREADS */

DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_Assign(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_MoveAssign(DeeObject *self, DeeObject *other);

/* Object conversion operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Str(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Repr(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_Bool(DeeObject *__restrict self);

/* Object call operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Call(DeeObject *self, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_CallKw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_ThisCall(DeeObject *self, DeeObject *this_arg, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_ThisCallKw(DeeObject *self, DeeObject *this_arg, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DeeObject_CallPack(DeeObject *self, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_VCallPack(DeeObject *self, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeObject_Callf(DeeObject *self, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DeeObject_ThisCallf(DeeObject *self, DeeObject *this_arg, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_VCallf(DeeObject *self, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_VThisCallf(DeeObject *self, DeeObject *this_arg, char const *__restrict format, va_list args);

/* Same as the regular call functions, however also include special
 * optimizations to re-use `args' as the varargs tuple in calls to
 * pure user-code varargs functions:
 * >> function foo(args...) {
 * >>     import object from deemon;
 * >>     print Object.id(args);
 * >> }
 * // `my_tuple' will be re-used as `args',
 * // without the need to creating a new tuple
 * DeeObject_CallTuple(foo, my_tuple);
 *
 * User-code can test if deemon was compiled with this option enabled
 * through use of code such as follows:
 * >> function b(args...) -> Object.id(args);
 * >> __asm__("" : "+x" (b)); // Ensure that the call to `b()' can't be inlined
 * >> function a(args...) -> Object.id(args) == b(args...);
 * >> print a(10, 20); // Prints `true' if `CONFIG_CALLTUPLE_OPTIMIZATIONS' was enabled; else `false'
 */
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_CallTuple(DeeObject *self, /*Tuple*/ DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_CallTupleKw(DeeObject *self, /*Tuple*/ DeeObject *args, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_ThisCallTuple(DeeObject *self, DeeObject *this_arg, /*Tuple*/ DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_ThisCallTupleKw(DeeObject *self, DeeObject *this_arg, /*Tuple*/ DeeObject *args, DeeObject *kw);
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DeeObject_CallTuple(self, args)                     DeeObject_Call(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallTupleKw(self, args, kw)               DeeObject_CallKw(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_ThisCallTuple(self, this_arg, args)       DeeObject_ThisCall(self, this_arg, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_ThisCallTupleKw(self, this_arg, args, kw) DeeObject_ThisCallKw(self, this_arg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */

/* Generate and return the hash of a given object. */
DFUNDEF WUNUSED /*ATTR_PURE*/ NONNULL((1)) Dee_hash_t (DCALL DeeObject_Hash)(DeeObject *__restrict self);
DFUNDEF WUNUSED /*ATTR_PURE*/ NONNULL((1)) Dee_hash_t (DCALL DeeObject_Hashv)(DeeObject *const *__restrict object_vector, size_t object_count);

/* GC operator invocation. */
DFUNDEF NONNULL((1, 2)) void (DCALL DeeObject_Visit)(DeeObject *__restrict self, Dee_visit_t proc, void *arg);
DFUNDEF NONNULL((1)) void (DCALL DeeObject_Clear)(DeeObject *__restrict self);
DFUNDEF NONNULL((1)) void (DCALL DeeObject_PClear)(DeeObject *__restrict self, unsigned int gc_priority);

/* Integral value lookup operators.
 * @return: INT_SIGNED:   The value stored in `result' must be interpreted as signed.
 * @return: INT_UNSIGNED: The value stored in `result' must be interpreted as unsigned.
 * @return: -1:           An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_GetInt8)(DeeObject *__restrict self, int8_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_GetInt16)(DeeObject *__restrict self, int16_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_GetInt32)(DeeObject *__restrict self, int32_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_GetInt64)(DeeObject *__restrict self, int64_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_GetInt128)(DeeObject *__restrict self, Dee_int128_t *__restrict result);

/* Integral/Floating-point conversion operator invocation. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsInt8)(DeeObject *__restrict self, int8_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsInt16)(DeeObject *__restrict self, int16_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsInt32)(DeeObject *__restrict self, int32_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsInt64)(DeeObject *__restrict self, int64_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsInt128)(DeeObject *__restrict self, Dee_int128_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsUInt8)(DeeObject *__restrict self, uint8_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsUInt16)(DeeObject *__restrict self, uint16_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsUInt32)(DeeObject *__restrict self, uint32_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsUInt64)(DeeObject *__restrict self, uint64_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsUInt128)(DeeObject *__restrict self, Dee_uint128_t *__restrict result);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AsDouble)(DeeObject *__restrict self, double *__restrict result);

/* Cast-to-integer conversion operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_Int)(DeeObject *__restrict self);

#define DEE_PRIVATE_OBJECT_AS_INT_1(self, result)  DeeObject_AsInt8(self, (int8_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_2(self, result)  DeeObject_AsInt16(self, (int16_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_4(self, result)  DeeObject_AsInt32(self, (int32_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_8(self, result)  DeeObject_AsInt64(self, (int64_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT(size)            DEE_PRIVATE_OBJECT_AS_INT_##size
#define DEE_PRIVATE_OBJECT_AS_UINT_1(self, result) DeeObject_AsUInt8(self, (uint8_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_2(self, result) DeeObject_AsUInt16(self, (uint16_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_4(self, result) DeeObject_AsUInt32(self, (uint32_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_8(self, result) DeeObject_AsUInt64(self, (uint64_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT(size)           DEE_PRIVATE_OBJECT_AS_UINT_##size

/* Helper macros for converting objects to integers */
#define DeeObject_AsXInt(size, self, result)       DEE_PRIVATE_OBJECT_AS_INT(size)(self, result)
#define DeeObject_AsXUInt(size, self, result)      DEE_PRIVATE_OBJECT_AS_UINT(size)(self, result)
#ifdef __CHAR_UNSIGNED__
#define DeeObject_AsChar(self, result)    DeeObject_AsXUInt(__SIZEOF_CHAR__, self, DEE_REQUIRES_TYPE(char *, result))
#else /* __CHAR_UNSIGNED__ */
#define DeeObject_AsChar(self, result)    DeeObject_AsXInt(__SIZEOF_CHAR__, self, DEE_REQUIRES_TYPE(char *, result))
#endif /* !__CHAR_UNSIGNED__ */
#define DeeObject_AsSChar(self, result)   DeeObject_AsXInt(__SIZEOF_CHAR__, self, DEE_REQUIRES_TYPE(signed char *, result))
#define DeeObject_AsUChar(self, result)   DeeObject_AsXUInt(__SIZEOF_CHAR__, self, DEE_REQUIRES_TYPE(unsigned char *, result))
#define DeeObject_AsShort(self, result)   DeeObject_AsXInt(__SIZEOF_SHORT__, self, DEE_REQUIRES_TYPE(short *, result))
#define DeeObject_AsUShort(self, result)  DeeObject_AsXUInt(__SIZEOF_SHORT__, self, DEE_REQUIRES_TYPE(unsigned short *, result))
#define DeeObject_AsInt(self, result)     DeeObject_AsXInt(__SIZEOF_INT__, self, DEE_REQUIRES_TYPE(int *, result))
#define DeeObject_AsUInt(self, result)    DeeObject_AsXUInt(__SIZEOF_INT__, self, DEE_REQUIRES_TYPE(unsigned int *, result))
#define DeeObject_AsLong(self, result)    DeeObject_AsXInt(__SIZEOF_LONG__, self, DEE_REQUIRES_TYPE(long *, result))
#define DeeObject_AsULong(self, result)   DeeObject_AsXUInt(__SIZEOF_LONG__, self, DEE_REQUIRES_TYPE(unsigned long *, result))
#ifdef __COMPILER_HAVE_LONGLONG
#define DeeObject_AsLLong(self, result)   DeeObject_AsXInt(__SIZEOF_LONG_LONG__, self, DEE_REQUIRES_TYPE(__LONGLONG *, result))
#define DeeObject_AsULLong(self, result)  DeeObject_AsXUInt(__SIZEOF_LONG_LONG__, self, DEE_REQUIRES_TYPE(__ULONGLONG *, result))
#endif /* __COMPILER_HAVE_LONGLONG */
#define DeeObject_AsSize(self, result)    DeeObject_AsXUInt(__SIZEOF_SIZE_T__, self, DEE_REQUIRES_TYPE(size_t *, result))
#define DeeObject_AsSSize(self, result)   DeeObject_AsXInt(__SIZEOF_SIZE_T__, self, DEE_REQUIRES_TYPE(Dee_ssize_t *, result))
#define DeeObject_AsPtrdiff(self, result) DeeObject_AsXInt(__SIZEOF_PTRDIFF_T__, self, DEE_REQUIRES_TYPE(ptrdiff_t *, result))
#define DeeObject_AsIntptr(self, result)  DeeObject_AsXInt(__SIZEOF_POINTER__, self, DEE_REQUIRES_TYPE(intptr_t *, result))
#define DeeObject_AsUIntptr(self, result) DeeObject_AsXUInt(__SIZEOF_POINTER__, self, DEE_REQUIRES_TYPE(uintptr_t *, result))


/* Math operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_Inv)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_Pos)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_Neg)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Add)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Sub)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Mul)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Div)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Mod)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Shl)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Shr)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_And)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Or)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Xor)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Pow)(DeeObject *self, DeeObject *some_object);

/* Inplace math operator invocation.
 * NOTE: For the duration of the call, `*pself' must not be changed by outside sources.
 *       Because of this, pointers to external, global, or static variables must be passed
 *       indirectly, though local or stack variables can be passed directly (as they are
 *       private to their stack-frame and cannot be changed through outside interference,
 *       aside of debuggers which know to look out for manipulating operands of inplace
 *       instructions)
 * >> Because these functions will inherit a reference to `IN(*pself)' upon success, it is
 *    possible for the implementation to check for otherwise immutable objects to be modified
 *    in-line (when `DeeObject_IsShared(IN(*pself))' is false), thus allowing an invocation
 *    such as `DeeObject_Inc(&my_int)' to potentially be completed without having to allocate
 *    a new integer object (though only in case `my_int' isn't being shared, and incrementing
 *    wouldn't overflow within the available number of digits) */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_Inc)(DREF DeeObject **__restrict pself);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_Dec)(DREF DeeObject **__restrict pself);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceAdd)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceSub)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceMul)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceDiv)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceMod)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceShl)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceShr)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceAnd)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceOr)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceXor)(DREF DeeObject **__restrict pself, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplacePow)(DREF DeeObject **__restrict pself, DeeObject *some_object);

/* Math operations with C (aka. host) integer operand. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AddS8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SubS8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AddInt)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SubInt)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_MulInt)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_DivInt)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_ModInt)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_ShlInt)(DeeObject *__restrict self, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_ShrInt)(DeeObject *__restrict self, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AndInt)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_OrInt)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_XorInt)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceAddS8)(DREF DeeObject **__restrict pself, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceSubS8)(DREF DeeObject **__restrict pself, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceAddInt)(DREF DeeObject **__restrict pself, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceSubInt)(DREF DeeObject **__restrict pself, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceMulInt)(DREF DeeObject **__restrict pself, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceDivInt)(DREF DeeObject **__restrict pself, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceModInt)(DREF DeeObject **__restrict pself, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceShlInt)(DREF DeeObject **__restrict pself, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceShrInt)(DREF DeeObject **__restrict pself, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceAndInt)(DREF DeeObject **__restrict pself, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceOrInt)(DREF DeeObject **__restrict pself, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceXorInt)(DREF DeeObject **__restrict pself, uint32_t val);


/* Comparison operator invocation.
 * NOTE: `DeeObject_CompareEq()' and `DeeObject_CompareNe()' will automatically attempt
 *        to catch the following errors, returning `true' / `false' indicative of
 *        equal/non-equal objects (then computed by comparing pointers) when possible:
 *          - `Error.RuntimeError.NotImplemented' (`DeeError_NotImplemented'; Should indicate compare-not-implemented)
 *          - `Error.TypeError'                   (`DeeError_TypeError';      Should indicate unsupported type combination)
 *          - `Error.ValueError'                  (`DeeError_ValueError';     Should indicate unsupported instance combination)
 *        This error handling is only performed during invocation of the `__eq__' / `__ne__'
 *        operators, but not when the returned result is then interpreted as a boolean. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CompareEqObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CompareNeObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CompareLoObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CompareLeObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CompareGrObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CompareGeObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CompareEq)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CompareNe)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CompareLo)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CompareLe)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CompareGr)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CompareGe)(DeeObject *self, DeeObject *some_object);

/* Compare a pre-keyed `keyed_search_item' with `elem' using the given (optional) `key' function
 * @return:  > 0: The objects are equal.
 * @return: == 0: The objects are non-equal.
 * @return:  < 0: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_CompareKeyEq)(DeeObject *keyed_search_item,
                               DeeObject *elem, /*nullable*/ DeeObject *key);

/* Sequence operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) size_t (DCALL DeeObject_Size)(DeeObject *__restrict self); /* @return: (size_t)-1: Error */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SizeObject)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_Contains)(DeeObject *self, DeeObject *some_object); /* @return: 1: found */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_ContainsObject)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_GetItemDef)(DeeObject *self, DeeObject *key, DeeObject *def);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_GetItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetItemString)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetItemStringLen)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeObject_GetItemStringDef)(DeeObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeObject_GetItemStringLenDef)(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_DelItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelItemString)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelItemStringLen)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_SetItem)(DeeObject *self, DeeObject *index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 3)) int (DCALL DeeObject_SetItemIndex)(DeeObject *self, size_t index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_SetItemString)(DeeObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeObject_SetItemStringLen)(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_GetRange)(DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *(DCALL DeeObject_GetRangeBeginIndex)(DeeObject *self, Dee_ssize_t begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetRangeEndIndex)(DeeObject *self, DeeObject *begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_GetRangeIndex)(DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_DelRange)(DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_SetRange)(DeeObject *self, DeeObject *begin, DeeObject *end, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 3, 4)) int (DCALL DeeObject_SetRangeBeginIndex)(DeeObject *self, Dee_ssize_t begin, DeeObject *end, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_SetRangeEndIndex)(DeeObject *self, DeeObject *begin, Dee_ssize_t end, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL DeeObject_SetRangeIndex)(DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end, DeeObject *value);

/* Check if a given item exists (`deemon.hasitem(self, index)')
 * @return: 0:  Doesn't exist;
 * @return: 1:  Does exists;
 * @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_HasItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasItemString)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasItemStringLen)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);

/* Check if a given item is bound (`self[index] is bound' / `deemon.bounditem(self, index)')
 * @return: 1 : Item is bound.
 * @return: 0 : Item isn't bound. (`UnboundItem' was caught internally)
 * @return: -1: An error occurred.
 * @return: -2: Item doesn't exist (Only when `allow_missing' is `true': `KeyError' or `IndexError' were caught). */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundItem)(DeeObject *self, DeeObject *index, bool allow_missing);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_BoundItemIndex)(DeeObject *__restrict self, size_t index, bool allow_missing);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundItemString)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash, bool allow_missing);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundItemStringLen)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash, bool allow_missing);

/* NOTE: The `argv' vector itself isn't inherited; only its elements are! */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_ConcatInherited)(/*inherit(on_success)*/ DREF DeeObject *self, DeeObject *other);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *(DCALL DeeObject_ExtendInherited)(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc, /*inherit(on_success)*/ DREF DeeObject *const *argv);

/* Process UTF-8-encoded `data' in whatever way you wish. */
typedef WUNUSED NONNULL((2)) Dee_ssize_t
(DCALL *Dee_formatprinter_t)(void *arg, char const *__restrict data, size_t datalen);
#ifdef DEE_SOURCE
typedef Dee_formatprinter_t dformatprinter;
#endif /* DEE_SOURCE */

/* Print the given object to a format printer.
 * This is identical to printing the return value of `DeeObject_Str', but is quite
 * faster for certain types such as `int', `string', as well as certain list types,
 * which then don't have to create temporary string objects.
 * Upon success, the sum of all printer-callbacks is returned. Upon
 * error, the first negative return value of printer is propagated,
 * or -1 is returned if an operator invocation failed.
 * NOTE: Printing is done as UTF-8 encoded strings. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t (DCALL DeeObject_Print)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t (DCALL DeeObject_PrintRepr)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);

/* Print a given object while using the given `format_str' format-string.
 * These functions are called by `string.format' when a `:' is found in
 * a format string, with the associated format-string passed:
 * >> print repr "foo = {:5}".format({ "bar" }); // "foo = bar  "
 * If `self' implements a member function `__format__', that function
 * is called as `self.__format__(format_str)', with the returned object
 * then printed using `DeeObject_Print()'
 * Also note that `object' implements a `__format__' function that calls forward
 * to the `str' operator and allows for alignment of the produced string, as well
 * as the fact that if accessing a sub-classes __format__ attribute causes an
 * AttributeError, or NotImplemented error to be thrown, the object will be formatted
 * using `object.__format__' as well:
 *  - "{:}"        --> arg.operator str()
 *  - "{:42}"      --> arg.operator str().ljust(42);
 *  - "{:<42}"     --> arg.operator str().ljust(42);
 *  - "{:>42}"     --> arg.operator str().rjust(42);
 *  - "{:^42}"     --> arg.operator str().center(42);
 *  - "{:=42}"     --> arg.operator str().zfill(42);
 *  - "{:42:foo}"  --> arg.operator str().ljust(42, "foo");
 *  - "{:<42:foo}" --> arg.operator str().ljust(42, "foo");
 *  - "{:>42:foo}" --> arg.operator str().rjust(42, "foo");
 *  - "{:^42:foo}" --> arg.operator str().center(42, "foo");
 *  - "{:=42:foo}" --> arg.operator str().zfill(42, "foo"); */
DFUNDEF WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t
(DCALL DeeObject_PrintFormatString)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg,
                                    /*utf-8*/ char const *__restrict format_str, size_t format_len);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t
(DCALL DeeObject_PrintFormat)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg,
                              DeeObject *__restrict format_str);

/* Create a/Yield from an iterator.
 * @return: Dee_ITER_DONE: [DeeObject_IterNext] The iterator has been exhausted. */
DFUNDEF WUNUSED WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IterSelf)(DeeObject *__restrict self);
DFUNDEF WUNUSED WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IterNext)(DeeObject *__restrict self);

/* Invoke `proc' for each element of a general-purpose sequence.
 * When `*proc' returns < 0, that value is propagated.
 * Otherwise, return the sum of all calls to it.
 * NOTE: This function does some special optimizations for known sequence types.
 * @return: -1: An error occurred during iteration (or potentially inside of `*proc') */
typedef WUNUSED NONNULL((2)) Dee_ssize_t (DCALL *dforeach_t)(void *arg, DeeObject *__restrict elem);
DFUNDEF WUNUSED WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeObject_Foreach)(DeeObject *__restrict self, dforeach_t proc, void *arg);

/* Unpack the given sequence `self' into `objc' items then stored within the `objv' vector. */
DFUNDEF WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_Unpack)(DeeObject *__restrict self, size_t objc,
                         /*out*/ DREF DeeObject **__restrict objv);

/* Object attribute access. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttr)(DeeObject *self, /*String*/ DeeObject *attr_name);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttrString)(DeeObject *__restrict self, char const *__restrict attr_name);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
#define DeeObject_GetAttrStringLen(self, attr_name, attrlen) DeeObject_GetAttrStringLenHash(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen))
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttr)(DeeObject *self, /*String*/ DeeObject *attr_name);                                                           /* @return: 0: doesn't exist; @return: 1: does exists; @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttrString)(DeeObject *__restrict self, char const *__restrict attr_name);                                         /* @return: 0: doesn't exist; @return: 1: does exists; @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);                    /* @return: 0: doesn't exist; @return: 1: does exists; @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash); /* @return: 0: doesn't exist; @return: 1: does exists; @return: -1: Error. */
#define DeeObject_HasAttrStringLen(self, attr_name, attrlen) DeeObject_HasAttrStringLenHash(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen))
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttr)(DeeObject *self, /*String*/ DeeObject *attr_name);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttrString)(DeeObject *__restrict self, char const *__restrict attr_name);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
#define DeeObject_DelAttrStringLen(self, attr_name, attrlen) DeeObject_DelAttrStringLenHash(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen))
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_SetAttr)(DeeObject *self, /*String*/ DeeObject *attr_name, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_SetAttrString)(DeeObject *self, char const *__restrict attr_name, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_SetAttrStringHash)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeObject_SetAttrStringLenHash)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, DeeObject *value);
#define DeeObject_SetAttrStringLen(self, attr_name, attrlen, value) DeeObject_SetAttrStringLenHash(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen), value)
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t (DCALL DeeObject_EnumAttr)(DeeTypeObject *tp_self, DeeObject *self, Dee_enum_t proc, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttr)(DeeObject *self, /*String*/ DeeObject *attr_name, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrKw)(DeeObject *self, /*String*/ DeeObject *attr_name, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrPack)(DeeObject *self, /*String*/ DeeObject *attr_name, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrPack)(DeeObject *self, /*String*/ DeeObject *attr_name, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DeeObject_CallAttrf)(DeeObject *self, /*String*/ DeeObject *attr_name, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_VCallAttrf)(DeeObject *self, /*String*/ DeeObject *attr_name, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrString)(DeeObject *self, char const *__restrict attr_name, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrStringPack)(DeeObject *self, char const *__restrict attr_name, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringPack)(DeeObject *self, char const *__restrict attr_name, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DeeObject_CallAttrStringf)(DeeObject *self, char const *__restrict attr_name, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringf)(DeeObject *self, char const *__restrict attr_name, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringKw)(DeeObject *self, char const *__restrict attr_name, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringHashKw)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringLenHashKw)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeObject_CallAttrStringLen(self, attr_name, attrlen, argc, argv)       DeeObject_CallAttrStringLenHash(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen), argc, argv)
#define DeeObject_CallAttrStringLenPack(self, attr_name, attrlen, ...)          DeeObject_CallAttrStringLenHashPack(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen), __VA_ARGS__)
#define DeeObject_VCallAttrStringLenPack(self, attr_name, attrlen, argc, args)  DeeObject_VCallAttrStringLenHashPack(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen), argc, args)
#define DeeObject_CallAttrStringLenKw(self, attr_name, attrlen, argc, argv, kw) DeeObject_CallAttrStringLenHashKw(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen), argc, argv, kw)
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringHash)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringLenHash)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrStringHashPack)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrStringLenHashPack)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringLenHashPack)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DeeObject_CallAttrStringHashf)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DeeObject_CallAttrStringLenHashf)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringHashf)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringLenHashf)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, char const *__restrict format, va_list args);
#if defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) || defined(__OPTIMIZE_SIZE__)
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_CallAttrTuple)(DeeObject *self, /*String*/ DeeObject *attr_name, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_CallAttrTupleKw)(DeeObject *self, /*String*/ DeeObject *attr_name, DeeObject *args, DeeObject *kw);
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS || __OPTIMIZE_SIZE__ */
#define DeeObject_CallAttrTuple(self, attr_name, args)       DeeObject_CallAttr(self, attr_name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrTupleKw(self, attr_name, args, kw) DeeObject_CallAttrKw(self, attr_name, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */
#define DeeObject_CallAttrStringTuple(self, attr_name, args)                             DeeObject_CallAttrString(self, attr_name, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringHashTuple(self, attr_name, hash, args)                   DeeObject_CallAttrStringHash(self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringLenTuple(self, attr_name, attrlen, args)                 DeeObject_CallAttrStringLen(self, attr_name, attrlen, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringLenHashTuple(self, attr_name, attrlen, hash, args)       DeeObject_CallAttrStringLenHash(self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringTupleKw(self, attr_name, args, kw)                       DeeObject_CallAttrStringKw(self, attr_name, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_CallAttrStringHashTupleKw(self, attr_name, hash, args, kw)             DeeObject_CallAttrStringHashKw(self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_CallAttrStringLenTupleKw(self, attr_name, attrlen, args, kw)           DeeObject_CallAttrStringLenKw(self, attr_name, attrlen, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_CallAttrStringLenHashTupleKw(self, attr_name, attrlen, hash, args, kw) DeeObject_CallAttrStringLenHashKw(self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)


/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist.
 * @return: -3: A user-defined getattr operator threw an error indicating
 *              that the attribute doesn't exists. - Should be handled the
 *              same way as `-2', however search for the attribute should
 *              not continue. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttr)(DeeObject *self, /*String*/ DeeObject *attr_name);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttrString)(DeeObject *self, char const *__restrict attr_name);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttrStringHash)(DeeObject *self, char const *__restrict attr_name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttrStringLenHash)(DeeObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
#define DeeObject_BoundAttrStringLen(self, attr_name, attrlen) \
	DeeObject_BoundAttrStringLenHash(self, attr_name, attrlen, Dee_HashPtr(attr_name, attrlen))


/* With-operator invocation:
 * >> with (my_object) {
 * >>     ...
 * >> }
 * Translates to:
 * >> DeeObject_Enter(my_object);
 * >> ...
 * >> DeeObject_Leave(my_object); */
DFUNDEF WUNUSED int (DCALL DeeObject_Enter)(DeeObject *__restrict self);
DFUNDEF WUNUSED int (DCALL DeeObject_Leave)(DeeObject *__restrict self);


/* Object buffer interface.
 * @param: flags: Set of `DEE_BUFFER_F*'
 * @throw: Error.RuntimeError.NotImplemented: The object doesn't implement the buffer protocol.
 * @throw: Error.ValueError.BufferError:      The object is an atomic buffer, or cannot be written to. */
DFUNDEF WUNUSED int (DCALL DeeObject_GetBuf)(DeeObject *__restrict self, DeeBuffer *__restrict info, unsigned int flags);
DFUNDEF void (DCALL DeeObject_PutBuf)(DeeObject *__restrict self, DeeBuffer *__restrict info, unsigned int flags);



#if 0
/* A singleton instance of a stand-alone, hidden type that is not derive
 * from any other object, and is used as special return value in operators
 * in order to indicate that the operator cannot be used.
 * Having an operator function return this value is the same as throwing
 * a `NotImplemented' error (which should not be confused with this, as it
 * is actually something completely different).
 * Also note that unlike a completely missing operator, an operator returning
 * this value will immediately indicate failure to the user, rather than having
 * them continue searching for a matching implementation by searching sub-classes. */
DDATDEF DeeObject DeeNotImplemented_Singleton;
#define Dee_NotImplemented          (&DeeNotImplemented_Singleton)
#define DeeNotImplemented_Check(ob) ((ob) == Dee_NotImplemented)
#endif


#ifndef __OPTIMIZE_SIZE__
#define DeeObject_GetAttrString(self, attr_name)                    DeeObject_GetAttrStringHash(self, attr_name, Dee_HashStr(attr_name))
#define DeeObject_BoundAttrString(self, attr_name)                  DeeObject_BoundAttrStringHash(self, attr_name, Dee_HashStr(attr_name))
#define DeeObject_HasAttrString(self, attr_name)                    DeeObject_HasAttrStringHash(self, attr_name, Dee_HashStr(attr_name))
#define DeeObject_DelAttrString(self, attr_name)                    DeeObject_DelAttrStringHash(self, attr_name, Dee_HashStr(attr_name))
#define DeeObject_SetAttrString(self, attr_name, value)             DeeObject_SetAttrStringHash(self, attr_name, Dee_HashStr(attr_name), value)
#define DeeObject_CallAttrString(self, attr_name, argc, argv)       DeeObject_CallAttrStringHash(self, attr_name, Dee_HashStr(attr_name), argc, argv)
#define DeeObject_CallAttrStringKw(self, attr_name, argc, argv, kw) DeeObject_CallAttrStringHashKw(self, attr_name, Dee_HashStr(attr_name), argc, argv, kw)
#define DeeObject_CallAttrStringPack(self, attr_name, ...)          DeeObject_CallAttrStringHashPack(self, attr_name, Dee_HashStr(attr_name), __VA_ARGS__)
#define DeeObject_VCallAttrStringPack(self, attr_name, argc, args)  DeeObject_VCallAttrStringHashPack(self, attr_name, Dee_HashStr(attr_name), argc, args)
#define DeeObject_CallAttrStringf(self, attr_name, ...)             DeeObject_CallAttrStringHashf(self, attr_name, Dee_HashStr(attr_name), __VA_ARGS__)
#define DeeObject_VCallAttrStringf(self, attr_name, format, args)   DeeObject_VCallAttrStringHashf(self, attr_name, Dee_HashStr(attr_name), format, args)
#endif /* !__OPTIMIZE_SIZE__ */

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeObject_InplaceDeepCopy(pself)                               __builtin_expect(DeeObject_InplaceDeepCopy(pself), 0)
#define DeeObject_Assign(self, some_object)                            __builtin_expect(DeeObject_Assign(self, some_object), 0)
#define DeeObject_MoveAssign(self, other)                              __builtin_expect(DeeObject_MoveAssign(self, other), 0)
#define DeeObject_AsInt8(self, result)                                 __builtin_expect(DeeObject_AsInt8(self, result), 0)
#define DeeObject_AsInt16(self, result)                                __builtin_expect(DeeObject_AsInt16(self, result), 0)
#define DeeObject_AsInt32(self, result)                                __builtin_expect(DeeObject_AsInt32(self, result), 0)
#define DeeObject_AsInt64(self, result)                                __builtin_expect(DeeObject_AsInt64(self, result), 0)
#define DeeObject_AsUInt8(self, result)                                __builtin_expect(DeeObject_AsUInt8(self, result), 0)
#define DeeObject_AsUInt16(self, result)                               __builtin_expect(DeeObject_AsUInt16(self, result), 0)
#define DeeObject_AsUInt32(self, result)                               __builtin_expect(DeeObject_AsUInt32(self, result), 0)
#define DeeObject_AsUInt64(self, result)                               __builtin_expect(DeeObject_AsUInt64(self, result), 0)
#define DeeObject_AsDouble(self, result)                               __builtin_expect(DeeObject_AsDouble(self, result), 0)
#define DeeObject_Inc(pself)                                           __builtin_expect(DeeObject_Inc(pself), 0)
#define DeeObject_Dec(pself)                                           __builtin_expect(DeeObject_Dec(pself), 0)
#define DeeObject_InplaceAdd(pself, some_object)                       __builtin_expect(DeeObject_InplaceAdd(pself, some_object), 0)
#define DeeObject_InplaceSub(pself, some_object)                       __builtin_expect(DeeObject_InplaceSub(pself, some_object), 0)
#define DeeObject_InplaceMul(pself, some_object)                       __builtin_expect(DeeObject_InplaceMul(pself, some_object), 0)
#define DeeObject_InplaceDiv(pself, some_object)                       __builtin_expect(DeeObject_InplaceDiv(pself, some_object), 0)
#define DeeObject_InplaceMod(pself, some_object)                       __builtin_expect(DeeObject_InplaceMod(pself, some_object), 0)
#define DeeObject_InplaceShl(pself, some_object)                       __builtin_expect(DeeObject_InplaceShl(pself, some_object), 0)
#define DeeObject_InplaceShr(pself, some_object)                       __builtin_expect(DeeObject_InplaceShr(pself, some_object), 0)
#define DeeObject_InplaceAnd(pself, some_object)                       __builtin_expect(DeeObject_InplaceAnd(pself, some_object), 0)
#define DeeObject_InplaceOr(pself, some_object)                        __builtin_expect(DeeObject_InplaceOr(pself, some_object), 0)
#define DeeObject_InplaceXor(pself, some_object)                       __builtin_expect(DeeObject_InplaceXor(pself, some_object), 0)
#define DeeObject_InplacePow(pself, some_object)                       __builtin_expect(DeeObject_InplacePow(pself, some_object), 0)
#define DeeObject_DelItem(self, index)                                 __builtin_expect(DeeObject_DelItem(self, index), 0)
#define DeeObject_DelItemIndex(self, index)                            __builtin_expect(DeeObject_DelItemIndex(self, index), 0)
#define DeeObject_DelItemString(self, key, hash)                       __builtin_expect(DeeObject_DelItemString(self, key, hash), 0)
#define DeeObject_DelItemStringLen(self, key, keylen, hash)            __builtin_expect(DeeObject_DelItemStringLen(self, key, keylen, hash), 0)
#define DeeObject_SetItem(self, index, value)                          __builtin_expect(DeeObject_SetItem(self, index, value), 0)
#define DeeObject_SetItemIndex(self, index, value)                     __builtin_expect(DeeObject_SetItemIndex(self, index, value), 0)
#define DeeObject_SetItemString(self, key, hash, value)                __builtin_expect(DeeObject_SetItemString(self, key, hash, value), 0)
#define DeeObject_SetItemStringLen(self, key, keylen, hash, value)     __builtin_expect(DeeObject_SetItemStringLen(self, key, keylen, hash, value), 0)
#define DeeObject_DelRange(self, begin, end)                           __builtin_expect(DeeObject_DelRange(self, begin, end), 0)
#define DeeObject_SetRange(self, begin, end, value)                    __builtin_expect(DeeObject_SetRange(self, begin, end, value), 0)
#define DeeObject_SetRangeBeginIndex(self, begin, end, value)          __builtin_expect(DeeObject_SetRangeBeginIndex(self, begin, end, value), 0)
#define DeeObject_SetRangeEndIndex(self, begin, end, value)            __builtin_expect(DeeObject_SetRangeEndIndex(self, begin, end, value), 0)
#define DeeObject_SetRangeIndex(self, begin, end, value)               __builtin_expect(DeeObject_SetRangeIndex(self, begin, end, value), 0)
#define DeeObject_Unpack(self, objc, objv)                             __builtin_expect(DeeObject_Unpack(self, objc, objv), 0)
#define DeeObject_DelAttr(self, attr_name)                             __builtin_expect(DeeObject_DelAttr(self, attr_name), 0)
#define DeeObject_SetAttr(self, attr_name, value)                      __builtin_expect(DeeObject_SetAttr(self, attr_name, value), 0)
#ifndef DeeObject_DelAttrString
#define DeeObject_DelAttrString(self, attr_name)                       __builtin_expect(DeeObject_DelAttrString(self, attr_name), 0)
#endif /* !DeeObject_DelAttrString */
#ifndef DeeObject_DelAttrStringLen
#define DeeObject_DelAttrStringLen(self, attr_name, attrlen)           __builtin_expect(DeeObject_DelAttrStringLen(self, attr_name, attrlen), 0)
#endif /* !DeeObject_DelAttrStringLen */
#ifndef DeeObject_SetAttrString
#define DeeObject_SetAttrString(self, attr_name, value)                __builtin_expect(DeeObject_SetAttrString(self, attr_name, value), 0)
#endif /* !DeeObject_SetAttrString */
#ifndef DeeObject_SetAttrStringLen
#define DeeObject_SetAttrStringLen(self, attr_name, attrlen, value)    __builtin_expect(DeeObject_SetAttrStringLen(self, attr_name, attrlen, value), 0)
#endif /* !DeeObject_SetAttrStringLen */
#ifndef DeeObject_DelAttrStringHash
#define DeeObject_DelAttrStringHash(self, attr_name, hash)             __builtin_expect(DeeObject_DelAttrStringHash(self, attr_name, hash), 0)
#endif /* !DeeObject_DelAttrStringHash */
#ifndef DeeObject_DelAttrStringLenHash
#define DeeObject_DelAttrStringLenHash(self, attr_name, attrlen, hash) __builtin_expect(DeeObject_DelAttrStringLenHash(self, attr_name, attrlen, hash), 0)
#endif /* !DeeObject_DelAttrStringLenHash */
#ifndef DeeObject_SetAttrStringHash
#define DeeObject_SetAttrStringHash(self, attr_name, hash, value)      __builtin_expect(DeeObject_SetAttrStringHash(self, attr_name, hash, value), 0)
#endif /* !DeeObject_SetAttrStringHash */
#ifndef DeeObject_SetAttrStringLenHash
#define DeeObject_SetAttrStringLenHash(self, attr_name, attrlen, hash, value) \
	__builtin_expect(DeeObject_SetAttrStringLenHash(self, attr_name, attrlen, hash, value), 0)
#endif /* !DeeObject_SetAttrStringLenHash */
#define DeeObject_Enter(self)                                          __builtin_expect(DeeObject_Enter(self), 0)
#define DeeObject_Leave(self)                                          __builtin_expect(DeeObject_Leave(self), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


#if 1 /* Integer conversion with automatic operand size. */
INTDEF ATTR_ERROR("Invalid integer size") int _Dee_invalid_integer_size(void);
#ifndef __NO_builtin_choose_expr
#define DeeObject_AsSINT(self, result)                                                                 \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeObject_AsInt8(self, (int8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeObject_AsInt16(self, (int16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeObject_AsInt32(self, (int32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeObject_AsInt64(self, (int64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeObject_AsInt128(self, (Dee_int128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#define DeeObject_AsUINT(self, result)                                                                   \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeObject_AsUInt8(self, (uint8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeObject_AsUInt16(self, (uint16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeObject_AsUInt32(self, (uint32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeObject_AsUInt64(self, (uint64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeObject_AsUInt128(self, (Dee_uint128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#else /* !__NO_builtin_choose_expr */
#define DeeObject_AsSINT(self, result)                                              \
	(sizeof(*(result)) == 1 ?  DeeObject_AsInt8(self, (int8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeObject_AsInt16(self, (int16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeObject_AsInt32(self, (int32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeObject_AsInt64(self, (int64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeObject_AsInt128(self, (Dee_int128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#define DeeObject_AsUINT(self, result)                                                \
	(sizeof(*(result)) == 1 ?  DeeObject_AsUInt8(self, (uint8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeObject_AsUInt16(self, (uint16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeObject_AsUInt32(self, (uint32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeObject_AsUInt64(self, (uint64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeObject_AsUInt128(self, (Dee_uint128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#endif /* __NO_builtin_choose_expr */
#endif

DECL_END

#endif /* !GUARD_DEEMON_OBJECT_H */
