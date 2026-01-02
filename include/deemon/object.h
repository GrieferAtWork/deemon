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
#ifndef GUARD_DEEMON_OBJECT_H
#define GUARD_DEEMON_OBJECT_H 1

#include "api.h"
/**/

#include "types.h"
#include "util/lock.h"
/**/

#include <hybrid/__atomic.h>
#include <hybrid/byteorder.h>
#include <hybrid/int128.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
/**/

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t, NULL */
#include <stdint.h>  /* intX_t, uintX_t */

#ifndef __INTELLISENSE__
#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#elif (!defined(CONFIG_HAVE_STRING_H) && \
       (defined(__NO_has_include) || __has_include(<string.h>)))
#define CONFIG_HAVE_STRING_H
#endif

#ifdef CONFIG_NO_strlen
#undef CONFIG_HAVE_strlen
#else
#define CONFIG_HAVE_strlen
#endif

#ifdef CONFIG_HAVE_STRING_H
#include <string.h> /* strlen */
#endif /* CONFIG_HAVE_STRING_H */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

#ifndef __INTELLISENSE__
#ifndef CONFIG_HAVE_strlen
#define CONFIG_HAVE_strlen
#undef strlen
#define strlen dee_strlen
LOCAL WUNUSED NONNULL((1)) size_t dee_strlen(char const *str) {
	size_t result;
	for (result = 0; str[result]; ++result)
		;
	return result;
}
#endif /* !CONFIG_HAVE_strlen */
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#define Dee_class_desc       class_desc
#define Dee_type_method      type_method
#define Dee_type_getset      type_getset
#define Dee_type_member      type_member
#define Dee_type_method_hint type_method_hint
#define Dee_type_constructor type_constructor
#define Dee_type_cast        type_cast
#define Dee_type_gc          type_gc
#define Dee_type_math        type_math
#define Dee_type_cmp         type_cmp
#define Dee_type_nii         type_nii
#define Dee_type_seq         type_seq
#define Dee_type_iterator    type_iterator
#define Dee_type_attr        type_attr
#define Dee_type_with        type_with
#define Dee_type_callable    type_callable
#define Dee_type_buffer      type_buffer
#define Dee_type_operator    type_operator
#define Dee_opinfo           opinfo
#define Dee_module_object    module_object
#endif /* DEE_SOURCE */

/* Hashing helpers. */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashPtr)(void const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCasePtr)(void const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_IN(1) Dee_hash_t (DCALL Dee_HashStr)(char const *__restrict str);
#ifdef __INTELLISENSE__
DFUNDEF ATTR_PURE WUNUSED ATTR_IN(1) Dee_hash_t (DCALL Dee_HashCaseStr)(char const *__restrict str);
#else /* __INTELLISENSE__ */
#define Dee_HashCaseStr(str) Dee_HashCasePtr(str, strlen(str))
#endif /* !__INTELLISENSE__ */

/* Combine 2 hash values into 1, while losing as little entropy
 * from either as possible. Note that this function tries to
 * include the order of arguments in the result, meaning that:
 * >> Dee_HashCombine(a, b) != Dee_HashCombine(b, a) */
DFUNDEF ATTR_CONST WUNUSED Dee_hash_t
(DFCALL Dee_HashCombine)(Dee_hash_t a, Dee_hash_t b);

/* This is the special hash we assign to empty sequences.
 *
 * It doesn't *have* to be zero; it could be anything. But
 * thinking about it, only zero *really* makes sense... */
#define DEE_HASHOF_EMPTY_SEQUENCE 0
#define DEE_HASHOF_UNBOUND_ITEM   0
#define DEE_HASHOF_RECURSIVE_ITEM 0



/* Hash a utf-8 encoded string.
 * You can think of these as hashing the ordinal values of the given string,
 * thus allowing this hashing function to return the same value for a string
 * encoded in utf-8, as `Dee_Hash2Byte()' would for a 2-byte, and Dee_Hash4Byte()
 * for a 4-byte string. */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashUtf8)(char const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCaseUtf8)(char const *__restrict ptr, size_t n_bytes);

/* Same as the regular hashing function, but with the guaranty that
 * for integer arrays where all items contain values `<= 0xff', the
 * return value is identical to a call to `Dee_HashPtr()' with the
 * array contents down-casted to the fitting data type. */
#ifdef __INTELLISENSE__
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_Hash1Byte)(uint8_t const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCase1Byte)(uint8_t const *__restrict ptr, size_t n_bytes);
#else /* __INTELLISENSE__ */
#define Dee_Hash1Byte(ptr, n_bytes)     Dee_HashPtr(ptr, n_bytes)
#define Dee_HashCase1Byte(ptr, n_bytes) Dee_HashCasePtr(ptr, n_bytes)
#endif /* !__INTELLISENSE__ */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_Hash2Byte)(uint16_t const *__restrict ptr, size_t n_words);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_Hash4Byte)(uint32_t const *__restrict ptr, size_t n_dwords);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCase2Byte)(uint16_t const *__restrict ptr, size_t n_words);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCase4Byte)(uint32_t const *__restrict ptr, size_t n_dwords);

/* Generic object hashing: Use the address of the object.
 * HINT: We ignore the lower 6 bits because they're
 *       often just ZERO(0) due to alignment. */
#define DeeObject_HashGeneric(ob) Dee_HashPointer(ob)
#define Dee_HashPointer(ptr)      ((Dee_hash_t)(ptr) >> 6)
#define DeeObject_Id(ob)          ((uintptr_t)(ob))


/* Return value for compare "tp_compare_eq", "tp_compare" and "tp_trycompare_eq". */
#define Dee_COMPARE_ERR (-2)
#define Dee_COMPARE_EQ  0
#define Dee_COMPARE_NE  1 /* or "-1" */
#define Dee_COMPARE_LO  (-1)
#define Dee_COMPARE_GR  1

#define Dee_COMPARE_ISERR(x) ((x) == Dee_COMPARE_ERR)
#define Dee_COMPARE_ISEQ(x)  ((x) == 0)
#define Dee_COMPARE_ISNE(x)  ((x) != 0)
#define Dee_COMPARE_ISLO(x)  ((x) < 0)
#define Dee_COMPARE_ISGR(x)  ((x) > 0)

/* #define Dee_COMPARE_FROMBOOL(is_equal)            ((is_equal) ? Dee_COMPARE_EQ : Dee_COMPARE_NE)
 * #define Dee_COMPARE_FROM_NOT_EQUALS(is_not_equal) ((is_not_equal) ? Dee_COMPARE_NE : Dee_COMPARE_EQ) */
#if Dee_COMPARE_EQ == 0 && Dee_COMPARE_ERR < 0
#define Dee_COMPARE_FROMBOOL(is_equal)            (!(is_equal))
#define Dee_COMPARE_FROM_NOT_EQUALS(is_not_equal) (is_not_equal) /* @assume(is_not_equal >= 0) */
#else /* Dee_COMPARE_EQ == 0 && Dee_COMPARE_ERR < 0 */
#define Dee_COMPARE_FROMBOOL(is_equal)            ((is_equal) ? Dee_COMPARE_EQ : Dee_COMPARE_NE)
#define Dee_COMPARE_FROM_NOT_EQUALS(is_not_equal) ((is_equal) ? Dee_COMPARE_NE : Dee_COMPARE_EQ)
#endif /* Dee_COMPARE_EQ != 0 || Dee_COMPARE_ERR >= 0 */

/* Helper macros for implementing compare operators. */
#if Dee_COMPARE_LO == -1 && Dee_COMPARE_EQ == 0 && Dee_COMPARE_GR == 1
#define Dee_Compare(a, b)           (((a) > (b)) - ((a) < (b)))
#define Dee_CompareFromDiff(diff)   (((diff) > 0) - ((diff) < 0))
#else /* Dee_COMPARE_LO == -1 && Dee_COMPARE_EQ == 0 && Dee_COMPARE_GR == 1 */
#define Dee_Compare(a, b)           ((a) == (b) ? Dee_COMPARE_EQ : Dee_CompareNe(a, b))
#define Dee_CompareFromDiff(diff)   ((diff) == 0 ? Dee_COMPARE_EQ : (diff) < 0 ? Dee_COMPARE_LO : Dee_COMPARE_GR)
#endif /* Dee_COMPARE_LO != -1 || Dee_COMPARE_EQ != 0 || Dee_COMPARE_GR != 1 */
#if Dee_COMPARE_LO == -1 && Dee_COMPARE_GR == 1
#define Dee_CompareNe(a, b)         ((((a) > (b)) << 1) - 1)
#else /* Dee_COMPARE_LO == -1 && Dee_COMPARE_GR == 1 */
#define Dee_CompareNe(a, b)         ((a) < (b) ? Dee_COMPARE_LO : Dee_COMPARE_GR)
#endif /* Dee_COMPARE_LO != -1 || Dee_COMPARE_GR != 1 */
#if Dee_COMPARE_EQ == 0 && Dee_COMPARE_NE != 0
#define Dee_CompareEqFromDiff(diff) (+!!(diff)) /* Leading "+" to force promotion to "int" */
#else /* Dee_COMPARE_EQ == 0 && Dee_COMPARE_NE != 0 */
#define Dee_CompareEqFromDiff(diff) ((diff) ? Dee_COMPARE_NE : Dee_COMPARE_EQ)
#endif /* Dee_COMPARE_EQ != 0 || Dee_COMPARE_NE == 0 */

#define Dee_return_compare_if_ne(a, b)  \
	do {                                \
		if ((a) != (b))                 \
			return Dee_CompareNe(a, b); \
	}	__WHILE0
#define Dee_return_compare(a, b)  \
	do {                          \
		return Dee_Compare(a, b); \
	}	__WHILE0
#define Dee_return_compare_if_neT(T, a, b)          \
	do {                                            \
		T _rcin_a = (a);                            \
		T _rcin_b = (b);                            \
		if (_rcin_a != _rcin_b)                     \
			return Dee_CompareNe(_rcin_a, _rcin_b); \
	}	__WHILE0
#define Dee_return_compareT(T, a, b)      \
	do {                                  \
		T _rc_a = (a);                    \
		T _rc_b = (b);                    \
		return Dee_Compare(_rc_a, _rc_b); \
	}	__WHILE0
#define Dee_return_DeeObject_Compare_if_ne(a, b)                                    \
	do {                                                                            \
		int _rtceqin_temp = DeeObject_Compare(Dee_AsObject(a),  \
		                                      Dee_AsObject(b)); \
		if (_rtceqin_temp != Dee_COMPARE_EQ)                                        \
			return _rtceqin_temp;                                                   \
	}	__WHILE0
#define Dee_return_DeeObject_CompareEq_if_ne(a, b)                                    \
	do {                                                                              \
		int _rtceqin_temp = DeeObject_CompareEq(Dee_AsObject(a),  \
		                                        Dee_AsObject(b)); \
		if (_rtceqin_temp != Dee_COMPARE_EQ)                                          \
			return _rtceqin_temp;                                                     \
	}	__WHILE0
#define Dee_return_DeeObject_TryCompareEq_if_ne(a, b)                                    \
	do {                                                                                 \
		int _rtceqin_temp = DeeObject_TryCompareEq(Dee_AsObject(a),  \
		                                           Dee_AsObject(b)); \
		if (_rtceqin_temp != Dee_COMPARE_EQ)                                             \
			return _rtceqin_temp;                                                        \
	}	__WHILE0




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
#define DeeObject_DATA(self)                ((void *)(&(self)->ob_trace + 1))
#define DeeObject_InitInherited(self, type) ((self)->ob_refcnt = 1, (self)->ob_type = (type), (self)->ob_trace = NULL)
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_DATA(self)                ((void *)(&(self)->ob_type + 1))
#define DeeObject_InitInherited(self, type) ((self)->ob_refcnt = 1, (self)->ob_type = (type))
#endif /* !CONFIG_TRACE_REFCHANGES */

/* >> DeeTypeObject *DeeObject_Init(DeeObject *self, DeeTypeObject *type);
 * Initialize the standard objects fields of a freshly allocated object.
 * @param: DeeObject      *self: The object to initialize
 * @param: DeeTypeoObject *type: The type to assign to the object */
#define DeeObject_Init(self, type) \
	(Dee_Incref(type),             \
	 DeeObject_InitInherited(self, (DeeTypeObject *)(type)))


#ifndef NDEBUG
#define DeeObject_Check(ob) \
	((ob)->ob_refcnt &&     \
	 (ob)->ob_type &&       \
	 ((DeeObject *)(ob)->ob_type)->ob_refcnt)
#define Dee_ASSERT_OBJECT(ob)                                     Dee_ASSERT_OBJECT_AT(ob, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_OPT(ob)                                 Dee_ASSERT_OBJECT_OPT_AT(ob, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE(ob, type)                          Dee_ASSERT_OBJECT_TYPE_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_OPT(ob, type)                      Dee_ASSERT_OBJECT_TYPE_OPT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)                        Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)                    Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_EXACT(ob, type)                    Dee_ASSERT_OBJECT_TYPE_EXACT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT(ob, type)                Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_AT(ob, file, line)                      (void)(likely(DeeObject_Check(ob)) || (DeeAssert_BadObject((DeeObject *)(ob), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_OPT_AT(ob, file, line)                  (void)(likely(!(ob) || DeeObject_Check(ob)) || (DeeAssert_BadObjectOpt((DeeObject *)(ob), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_AT(ob, type, file, line)           (void)(likely((DeeObject_Check(ob) && DeeObject_InstanceOf(ob, type))) || (DeeAssert_BadObjectType((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_OPT_AT(ob, type, file, line)       (void)(likely(!(ob) || (DeeObject_Check(ob) && DeeObject_InstanceOf(ob, type))) || (DeeAssert_BadObjectTypeOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, file, line)         (void)(likely((DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type)))) || (DeeAssert_BadObjectType((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, file, line)     (void)(likely(!(ob) || (DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type)))) || (DeeAssert_BadObjectTypeOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_EXACT_AT(ob, type, file, line)     (void)(likely((DeeObject_Check(ob) && DeeObject_InstanceOfExact(ob, type))) || (DeeAssert_BadObjectTypeExact((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT(ob, type, file, line) (void)(likely(!(ob) || (DeeObject_Check(ob) && DeeObject_InstanceOfExact(ob, type))) || (DeeAssert_BadObjectTypeExactOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObject(DeeObject const *ob, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectOpt(DeeObject const *ob, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectType(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeOpt(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeExact(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeExactOpt(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
#else /* !NDEBUG */
#define DeeObject_Check(ob)                                       true
#define Dee_ASSERT_OBJECT(ob)                                     (void)0
#define Dee_ASSERT_OBJECT_OPT(ob)                                 (void)0
#define Dee_ASSERT_OBJECT_TYPE(ob, type)                          (void)0
#define Dee_ASSERT_OBJECT_TYPE_OPT(ob, type)                      (void)0
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)                        (void)0
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)                    (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT(ob, type)                    (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT(ob, type)                (void)0
#define Dee_ASSERT_OBJECT_AT(ob, file, line)                      (void)0
#define Dee_ASSERT_OBJECT_OPT_AT(ob, file, line)                  (void)0
#define Dee_ASSERT_OBJECT_TYPE_AT(ob, type, file, line)           (void)0
#define Dee_ASSERT_OBJECT_TYPE_OPT_AT(ob, type, file, line)       (void)0
#define Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, file, line)         (void)0
#define Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, file, line)     (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_AT(ob, type, file, line)     (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT(ob, type, file, line) (void)0
#endif /* NDEBUG */

#ifdef DEE_SOURCE
#define ASSERT_OBJECT                   Dee_ASSERT_OBJECT
#define ASSERT_OBJECT_OPT               Dee_ASSERT_OBJECT_OPT
#define ASSERT_OBJECT_TYPE              Dee_ASSERT_OBJECT_TYPE
#define ASSERT_OBJECT_TYPE_OPT          Dee_ASSERT_OBJECT_TYPE_OPT
#define ASSERT_OBJECT_TYPE_A            Dee_ASSERT_OBJECT_TYPE_A
#define ASSERT_OBJECT_TYPE_A_OPT        Dee_ASSERT_OBJECT_TYPE_A_OPT
#define ASSERT_OBJECT_TYPE_EXACT        Dee_ASSERT_OBJECT_TYPE_EXACT
#define ASSERT_OBJECT_TYPE_EXACT_OPT    Dee_ASSERT_OBJECT_TYPE_EXACT_OPT
#define ASSERT_OBJECT_AT                Dee_ASSERT_OBJECT_AT
#define ASSERT_OBJECT_OPT_AT            Dee_ASSERT_OBJECT_OPT_AT
#define ASSERT_OBJECT_TYPE_AT           Dee_ASSERT_OBJECT_TYPE_AT
#define ASSERT_OBJECT_TYPE_OPT_AT       Dee_ASSERT_OBJECT_TYPE_OPT_AT
#define ASSERT_OBJECT_TYPE_A_AT         Dee_ASSERT_OBJECT_TYPE_A_AT
#define ASSERT_OBJECT_TYPE_A_OPT_AT     Dee_ASSERT_OBJECT_TYPE_A_OPT_AT
#define ASSERT_OBJECT_TYPE_EXACT_AT     Dee_ASSERT_OBJECT_TYPE_EXACT_AT
#define ASSERT_OBJECT_TYPE_EXACT_OPT_AT Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT
#endif /* DEE_SOURCE */



struct Dee_weakref;
/* Prototype for callbacks to-be invoked when a weakref'd object gets destroyed. */
typedef NONNULL_T((1)) void (DCALL *Dee_weakref_callback_t)(struct Dee_weakref *__restrict self);


/* Object weak reference tracing. */
struct Dee_weakref {
	struct Dee_weakref   **wr_pself; /* [0..1][lock(BIT0(wr_next))][valid_if(wr_pself != NULL)] Indirect self pointer. */
	struct Dee_weakref    *wr_next;  /* [0..1][lock(BIT0(wr_next))][valid_if(wr_pself != NULL)] Next weak references. */
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
#define Dee_WEAKREF(T) struct ::Dee_weakref
#else /* __cplusplus */
#define Dee_WEAKREF(T) struct Dee_weakref
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
 * >>         DREF DeeObject *result;
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
	__hybrid_atomic_store(&(x)->wr_next, (struct ::Dee_weakref *)_DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR, __ATOMIC_RELEASE)
#else /* __cplusplus */
#define DeeWeakref_UnlockCallback(x) \
	__hybrid_atomic_store(&(x)->wr_next, (struct Dee_weakref *)_DEE_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR, __ATOMIC_RELEASE)
#endif /* !__cplusplus */


/* Initialize the given weak reference to NULL. */
#define Dee_weakref_initempty(self)                         \
	(void)((self)->wr_pself = NULL, (self)->wr_next = NULL, \
	       (self)->wr_obj = NULL, (self)->wr_del = NULL)

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
DFUNDEF NONNULL((1, 2)) void DCALL Dee_weakref_moveassign(struct Dee_weakref *dst, struct Dee_weakref *src);
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self, struct Dee_weakref const *__restrict other);
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copyassign)(struct Dee_weakref *self, struct Dee_weakref const *other);
#else /* __INTELLISENSE__ */
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self, struct Dee_weakref *__restrict other);
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copyassign)(struct Dee_weakref *self, struct Dee_weakref *other);
#ifdef __cplusplus
#define Dee_weakref_copy(self, other) Dee_weakref_copy(self, (struct ::Dee_weakref *)(other))
#define Dee_weakref_copyassign(self, other) Dee_weakref_copyassign(self, (struct ::Dee_weakref *)(other))
#else /* __cplusplus */
#define Dee_weakref_copy(self, other) Dee_weakref_copy(self, (struct Dee_weakref *)(other))
#define Dee_weakref_copyassign(self, other) Dee_weakref_copyassign(self, (struct Dee_weakref *)(other))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Overwrite an already initialize weak reference with the given `ob'.
 * @return: true:  Successfully overwritten the weak reference.
 * @return: false: The given object `ob' does not support weak referencing
 *                 and the stored weak reference was not modified. */
DFUNDEF NONNULL((1, 2)) bool DCALL
Dee_weakref_set(struct Dee_weakref *__restrict self,
                DeeObject *__restrict ob);

#if !defined(NDEBUG) && !defined(NDEBUG_ASSERT)
#define Dee_weakref_set_forced(self, ob) Dee_ASSERT(Dee_weakref_set(self, ob))
#else /* !defined(NDEBUG) && !defined(NDEBUG_ASSERT) */
#define Dee_weakref_set_forced(self, ob) Dee_weakref_set(self, ob)
#endif /* NDEBUG || NDEBUG_ASSERT */

/* Clear the weak reference `self', returning true if it used to point to an object.
 * NOTE: Upon success (return is `true'), the callback will not be
 *       executed for the previously bound object's destruction. */
DFUNDEF NONNULL((1)) bool DCALL
Dee_weakref_clear(struct Dee_weakref *__restrict self);

#define Dee_weakref_getaddr(self) ((DeeObject *)((uintptr_t)__hybrid_atomic_load(&(self)->wr_obj, __ATOMIC_ACQUIRE) & ~1))

/* Lock a weak reference, returning a regular reference to the pointed-to object.
 * @return: * :   A new reference to the pointed-to object.
 * @return: NULL: Failed to lock the weak reference (No error is thrown in this case). */
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
 * NOTE: You may pass `NULL' for `new_ob' to clear the weakref. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_weakref_cmpxch)(struct Dee_weakref *__restrict self,
                           DeeObject *old_ob, DeeObject *new_ob);



/* Type visit helpers.
 * WARNING: These helper macros are allowed to evaluate their arguments multiple times! */
typedef NONNULL_T((1)) void (DCALL *Dee_visit_t)(DeeObject *__restrict self, void *arg);
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


/* Destroy a given deemon object (called when its refcnt reaches `0') */
#if defined(CONFIG_NO_BADREFCNT_CHECKS) && !defined(CONFIG_TRACE_REFCHANGES)
DFUNDEF NONNULL((1)) void DCALL DeeObject_Destroy(DeeObject *__restrict self);
#else /* CONFIG_NO_BADREFCNT_CHECKS && !CONFIG_TRACE_REFCHANGES */
DFUNDEF NONNULL((1)) void DCALL DeeObject_Destroy_d(DeeObject *__restrict self, char const *file, int line);
#define DeeObject_Destroy(self) DeeObject_Destroy_d(self, __FILE__, __LINE__)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS || CONFIG_TRACE_REFCHANGES */

typedef NONNULL_T((1)) void (DCALL *Dee_tp_destroy_t)(DeeObject *__restrict self);

/* Return a pointer to the optimized implementation of
 * object destruction called by `DeeObject_Destroy()' */
DFUNDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_destroy_t DCALL
DeeType_RequireDestroy(DeeTypeObject *__restrict self);



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
#define _DeeRefcnt_Inc(x)               (void)(++*(x))
#define _DeeRefcnt_Dec(x)               (void)(--*(x))
#define _DeeRefcnt_FetchInc(x)          ((*(x))++)
#define _DeeRefcnt_FetchDec(x)          ((*(x))--)
#define _DeeRefcnt_IncFetch(x)          (++(*(x)))
#define _DeeRefcnt_DecFetch(x)          (--(*(x)))
#define _DeeRefcnt_AddFetch(x, n)       ((*(x)) += (n))
#define _DeeRefcnt_FetchAdd(x, n)       ((Dee_refcnt_t)(((*(x)) += (n)) - (n)))
#define _DeeRefcnt_SubFetch(x, n)       ((*(x)) -= (n))
#define _DeeRefcnt_FetchSub(x, n)       ((Dee_refcnt_t)(((*(x)) -= (n)) + (n)))
#define Dee_Incref_untraced(x)          (void)(&(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     (void)(((x)->ob_refcnt += (n)))
#define Dee_Decref_untraced(x)          (void)(&(x)->ob_refcnt)
#define Dee_Decref_n_untraced(x, n)     (void)(((x)->ob_refcnt -= (n)))
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
#define _DeeFatal_BadIncref(ob) DeeFatal_BadIncref((DeeObject *)(ob), __FILE__, __LINE__)
#define _DeeFatal_BadDecref(ob) DeeFatal_BadDecref((DeeObject *)(ob), __FILE__, __LINE__)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
#ifdef CONFIG_NO_THREADS
#define _DeeRefcnt_Inc(x)         (void)(++*(x))
#define _DeeRefcnt_Dec(x)         (void)(--*(x))
#define _DeeRefcnt_FetchInc(x)    ((*(x))++)
#define _DeeRefcnt_FetchDec(x)    ((*(x))--)
#define _DeeRefcnt_IncFetch(x)    (++(*(x)))
#define _DeeRefcnt_DecFetch(x)    (--(*(x)))
#define _DeeRefcnt_AddFetch(x, n) ((*(x)) += (n))
#define _DeeRefcnt_FetchAdd(x, n) ((Dee_refcnt_t)(((*(x)) += (n)) - (n)))
#define _DeeRefcnt_SubFetch(x, n) ((*(x)) -= (n))
#define _DeeRefcnt_FetchSub(x, n) ((Dee_refcnt_t)(((*(x)) -= (n)) + (n)))
#ifndef CONFIG_NO_BADREFCNT_CHECKS
#ifdef __NO_builtin_expect
#define Dee_Incref_untraced(x)          (void)((x)->ob_refcnt++ || (_DeeFatal_BadIncref(x), 0))
#define Dee_Incref_n_untraced(x, n)     (void)(_DeeRefcnt_FetchAdd(&(x)->ob_refcnt, n) || (_DeeFatal_BadIncref(x), 0))
#define Dee_DecrefNokill_untraced(x)    (void)((x)->ob_refcnt-- > 1 || (_DeeFatal_BadDecref(x), 0))
#define Dee_Decref_likely_untraced(x)   (void)((x)->ob_refcnt-- > 1 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)((x)->ob_refcnt-- > 1 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#else /* __NO_builtin_expect */
#define Dee_Incref_untraced(x)          (void)(likely((x)->ob_refcnt++) || (_DeeFatal_BadIncref(x), 0))
#define Dee_Incref_n_untraced(x, n)     (void)(likely(_DeeRefcnt_FetchAdd(&(x)->ob_refcnt, n)) || (_DeeFatal_BadIncref(x), 0))
#define Dee_DecrefNokill_untraced(x)    (void)(likely((x)->ob_refcnt-- > 1) || (_DeeFatal_BadDecref(x), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely((x)->ob_refcnt-- > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely((x)->ob_refcnt-- > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#endif /* !__NO_builtin_expect */
#define Dee_Decref_untraced(x)          (void)((x)->ob_refcnt-- > 1 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_n_untraced(x, n)     (void)(_DeeRefcnt_FetchSub(&(x)->ob_refcnt, n) > (n) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    (void)(--(x)->ob_refcnt, DeeObject_Destroy((DeeObject *)(x)))
#define Dee_DecrefWasOk_untraced(x)     ((x)->ob_refcnt-- > 1 ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_Incref_untraced(x)          (void)(++(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     (void)((x)->ob_refcnt += (n))
#define Dee_Decref_untraced(x)          (void)(--(x)->ob_refcnt || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_n_untraced(x, n)     (void)(((x)->ob_refcnt -= (n)) != 0 || (DeeObject_Destroy((DeeObject *)(x)), 0))
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
#ifdef _MSC_VER
/* NOTE: The atomic functions from hybrid would work for this case just as well,  but
 *       they have a rather large compile-time overhead and add a lot of unnecessary
 *       debug information when the resulting code isn't getting optimized.
 *       Therefor, try to bypass them here to speed up compile-time and ease debugging. */
#if Dee_SIZEOF_REFCNT_T == __SIZEOF_LONG__
#   define _DeeRefcnt_Inc(x)      (void)_InterlockedIncrement((long volatile *)(x))
#   define _DeeRefcnt_Dec(x)      (void)_InterlockedDecrement((long volatile *)(x))
#   define _DeeRefcnt_FetchInc(x) ((Dee_refcnt_t)_InterlockedIncrement((long volatile *)(x)) - 1)
#   define _DeeRefcnt_FetchDec(x) ((Dee_refcnt_t)_InterlockedDecrement((long volatile *)(x)) + 1)
#   define _DeeRefcnt_IncFetch(x) ((Dee_refcnt_t)_InterlockedIncrement((long volatile *)(x)))
#   define _DeeRefcnt_DecFetch(x) ((Dee_refcnt_t)_InterlockedDecrement((long volatile *)(x)))
#elif Dee_SIZEOF_REFCNT_T == 8
#   define _DeeRefcnt_Inc(x)      (void)_InterlockedIncrement64((__int64 volatile *)(x))
#   define _DeeRefcnt_Dec(x)      (void)_InterlockedDecrement64((__int64 volatile *)(x))
#   define _DeeRefcnt_FetchInc(x) ((Dee_refcnt_t)_InterlockedIncrement64((__int64 volatile *)(x)) - 1)
#   define _DeeRefcnt_FetchDec(x) ((Dee_refcnt_t)_InterlockedDecrement64((__int64 volatile *)(x)) + 1)
#   define _DeeRefcnt_IncFetch(x) ((Dee_refcnt_t)_InterlockedIncrement64((__int64 volatile *)(x)))
#   define _DeeRefcnt_DecFetch(x) ((Dee_refcnt_t)_InterlockedDecrement64((__int64 volatile *)(x)))
#endif /* Dee_SIZEOF_REFCNT_T == ... */
#endif /* _MSC_VER */
#ifndef _DeeRefcnt_Inc
#define _DeeRefcnt_Inc(x)      __hybrid_atomic_inc(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_Dec(x)      __hybrid_atomic_dec(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_FetchInc(x) __hybrid_atomic_fetchinc(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_FetchDec(x) __hybrid_atomic_fetchdec(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_IncFetch(x) __hybrid_atomic_incfetch(x, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_DecFetch(x) __hybrid_atomic_decfetch(x, __ATOMIC_SEQ_CST)
#endif /* !_DeeRefcnt_Inc */
#ifndef _DeeRefcnt_Add
#define _DeeRefcnt_Add(x, n)      __hybrid_atomic_add(x, n, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_FetchAdd(x, n) __hybrid_atomic_fetchadd(x, n, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_AddFetch(x, n) __hybrid_atomic_addfetch(x, n, __ATOMIC_SEQ_CST)
#endif /* !_DeeRefcnt_Add */
#ifndef _DeeRefcnt_Sub
#define _DeeRefcnt_Sub(x, n)      __hybrid_atomic_sub(x, n, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_FetchSub(x, n) __hybrid_atomic_fetchsub(x, n, __ATOMIC_SEQ_CST)
#define _DeeRefcnt_SubFetch(x, n) __hybrid_atomic_subfetch(x, n, __ATOMIC_SEQ_CST)
#endif /* !_DeeRefcnt_Sub */
#ifndef CONFIG_NO_BADREFCNT_CHECKS
#ifdef __NO_builtin_expect
#define Dee_Incref_untraced(x)          (void)(_DeeRefcnt_FetchInc(&(x)->ob_refcnt) || (_DeeFatal_BadIncref(x), 0))
#define Dee_Incref_n_untraced(x, n)     (void)(_DeeRefcnt_FetchAdd(&(x)->ob_refcnt, n) || (_DeeFatal_BadIncref(x), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefNokill_untraced(x)    (void)(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1 || (_DeeFatal_BadDecref(x), 0))
#else /* __NO_builtin_expect */
#define Dee_Incref_untraced(x)          (void)(likely(_DeeRefcnt_FetchInc(&(x)->ob_refcnt)) || (_DeeFatal_BadIncref(x), 0))
#define Dee_Incref_n_untraced(x, n)     (void)(likely(_DeeRefcnt_FetchAdd(&(x)->ob_refcnt, n)) || (_DeeFatal_BadIncref(x), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefNokill_untraced(x)    (void)(likely(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1) || (_DeeFatal_BadDecref(x), 0))
#endif /* !__NO_builtin_expect */
#define Dee_Decref_untraced(x)          (void)(_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_n_untraced(x, n)     (void)(_DeeRefcnt_FetchSub(&(x)->ob_refcnt, n) > (n) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    (_DeeRefcnt_FetchDec(&(x)->ob_refcnt), DeeObject_Destroy((DeeObject *)(x)))
#define Dee_DecrefWasOk_untraced(x)     (_DeeRefcnt_FetchDec(&(x)->ob_refcnt) > 1 ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#define Dee_DecrefIfOne_untraced(self)  Dee_DecrefIfOne_untraced_d((DeeObject *)(self), __FILE__, __LINE__)
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_Incref_untraced(x)          _DeeRefcnt_Inc(&(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     _DeeRefcnt_Add(&(x)->ob_refcnt, n)
#define Dee_Decref_untraced(x)          (void)(_DeeRefcnt_DecFetch(&(x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_n_untraced(x, n)     (void)(_DeeRefcnt_SubFetch(&(x)->ob_refcnt, n) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(_DeeRefcnt_DecFetch(&(x)->ob_refcnt)) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(_DeeRefcnt_DecFetch(&(x)->ob_refcnt)) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    DeeObject_Destroy((DeeObject *)(x))
#define Dee_DecrefNokill_untraced(x)    (void)_DeeRefcnt_DecFetch(&(x)->ob_refcnt)
#define Dee_DecrefWasOk_untraced(x)     (_DeeRefcnt_DecFetch(&(x)->ob_refcnt) ? false : (DeeObject_Destroy((DeeObject *)(x)), true))
#define Dee_DecrefIfOne_untraced(self)  Dee_DecrefIfOne_untraced((DeeObject *)(self))
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_DecrefIfNotOne_untraced(self)  Dee_DecrefIfNotOne_untraced((DeeObject *)(self))
#define Dee_IncrefIfNotZero_untraced(self) Dee_IncrefIfNotZero_untraced((DeeObject *)(self))

LOCAL ATTR_ARTIFICIAL WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfNotOne_untraced)(DeeObject *__restrict self) {
	Dee_refcnt_t refcnt;
	do {
		refcnt = __hybrid_atomic_load(&self->ob_refcnt, __ATOMIC_ACQUIRE);
		if (refcnt <= 1)
			return false;
	} while (!__hybrid_atomic_cmpxch_weak(&self->ob_refcnt, refcnt, refcnt - 1,
	                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
	return true;
}

LOCAL ATTR_ARTIFICIAL WUNUSED NONNULL((1)) bool
(DCALL Dee_IncrefIfNotZero_untraced)(DeeObject *__restrict self) {
	Dee_refcnt_t refcnt;
	do {
		refcnt = __hybrid_atomic_load(&self->ob_refcnt, __ATOMIC_ACQUIRE);
		if (!refcnt)
			return false;
	} while (!__hybrid_atomic_cmpxch_weak(&self->ob_refcnt, refcnt, refcnt + 1,
	                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
	return true;
}

#ifndef __INTELLISENSE__
#ifndef CONFIG_NO_BADREFCNT_CHECKS
LOCAL ATTR_ARTIFICIAL WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne_untraced_d)(DeeObject *__restrict self,
                                   char const *file, int line) {
	if (!__hybrid_atomic_cmpxch(&self->ob_refcnt, 1, 0,
	                            __ATOMIC_SEQ_CST,
	                            __ATOMIC_SEQ_CST))
		return false;
	DeeObject_Destroy_d(self, file, line);
	return true;
}
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
LOCAL ATTR_ARTIFICIAL WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne_untraced)(DeeObject *__restrict self) {
	if (!__hybrid_atomic_cmpxch(&self->ob_refcnt, 1, 0,
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
DFUNDEF NONNULL((1)) void DCALL Dee_Decref_n_traced(DeeObject *__restrict ob, Dee_refcnt_t n, char const *file, int line);
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
#define Dee_Decref_n(x, n)      Dee_Decref_n_traced((DeeObject *)(x), n, __FILE__, __LINE__)
#define Dee_Decref_likely(x)    Dee_Decref_likely_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_Decref_unlikely(x)  Dee_Decref_unlikely_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefDokill(x)     Dee_DecrefDokill_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefNokill(x)     Dee_DecrefNokill_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefIfOne(x)      Dee_DecrefIfOne_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefIfNotOne(x)   Dee_DecrefIfNotOne_traced((DeeObject *)(x), __FILE__, __LINE__)
#define Dee_DecrefWasOk(x)      Dee_DecrefWasOk_traced((DeeObject *)(x), __FILE__, __LINE__)
#else /* CONFIG_TRACE_REFCHANGES */
#define Dee_Incref_traced(x, file, line)          Dee_Incref_untraced(x)
#define Dee_Incref_n_traced(x, n, file, line)     Dee_Incref_n_untraced(x, n)
#define Dee_IncrefIfNotZero_traced(x, file, line) Dee_IncrefIfNotZero_untraced(x)
#define Dee_Decref_traced(x, file, line)          Dee_Decref_untraced(x)
#define Dee_Decref_n_traced(x, n, file, line)     Dee_Decref_n_untraced(x, n)
#define Dee_Decref_likely_traced(x, file, line)   Dee_Decref_likely_untraced(x)
#define Dee_Decref_unlikely_traced(x, file, line) Dee_Decref_unlikely_untraced(x)
#define Dee_DecrefDokill_traced(x, file, line)    Dee_DecrefDokill_untraced(x)
#define Dee_DecrefNokill_traced(x, file, line)    Dee_DecrefNokill_untraced(x)
#define Dee_DecrefIfOne_traced(x, file, line)     Dee_DecrefIfOne_untraced(x)
#define Dee_DecrefIfNotOne_traced(x, file, line)  Dee_DecrefIfNotOne_untraced(x)
#define Dee_DecrefWasOk_traced(x, file, line)     Dee_DecrefWasOk_untraced(x)
#define Dee_Incref(x)                             Dee_Incref_untraced(x)
#define Dee_Incref_n(x, n)                        Dee_Incref_n_untraced(x, n)
#define Dee_IncrefIfNotZero(x)                    Dee_IncrefIfNotZero_untraced(x)
#define Dee_Decref(x)                             Dee_Decref_untraced(x)
#define Dee_Decref_n(x, n)                        Dee_Decref_n_untraced(x, n)
#define Dee_Decref_likely(x)                      Dee_Decref_likely_untraced(x)
#define Dee_Decref_unlikely(x)                    Dee_Decref_unlikely_untraced(x)
#define Dee_DecrefDokill(x)                       Dee_DecrefDokill_untraced(x)
#define Dee_DecrefNokill(x)                       Dee_DecrefNokill_untraced(x)
#define Dee_DecrefIfOne(x)                        Dee_DecrefIfOne_untraced(x)
#define Dee_DecrefIfNotOne(x)                     Dee_DecrefIfNotOne_untraced(x)
#define Dee_DecrefWasOk(x)                        Dee_DecrefWasOk_untraced(x)
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
#define Dee_return_reference(ob)                                   \
	do {                                                           \
		__register DeeObject *const _rr_result = Dee_AsObject(ob); \
		Dee_Incref(_rr_result);                                    \
		return _rr_result;                                         \
	}	__WHILE0

/* NOTE: `(Dee_)return_reference_()' may evaluate `ob' multiple times */
#define Dee_return_reference_(ob) \
	return (Dee_Incref(ob), ob)

#ifdef DEE_SOURCE
#define return_reference   Dee_return_reference
#define return_reference_  Dee_return_reference_
#endif /* DEE_SOURCE */




/* Increment the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
DFUNDEF ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv)(DeeObject *const *__restrict object_vector,
                    size_t object_count);

/* Decrement the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
DFUNDEF ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv)(DREF DeeObject *const *__restrict object_vector,
                    size_t object_count);

/* Copy object pointers from `src' to `dst' and increment
 * the reference counter of every object that got copied.
 * @return: * : Always re-returns the pointer to `dst' */
DFUNDEF ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *const *__restrict src,
                    size_t object_count);

/* Fill object pointers in `dst' with `obj' and increment
 * the reference counter of `obj' accordingly.
 * @return: * : Always re-returns the pointer to `dst' */
DFUNDEF ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *obj, size_t object_count);


LOCAL ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_XIncrefv)(DeeObject * /*nullable*/ const *__restrict object_vector,
                     size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob = object_vector[i];
		Dee_XIncref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

LOCAL ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_XDecrefv)(DeeObject * /*nullable*/ const *__restrict object_vector,
                     size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_XDecref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

LOCAL ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_XMovrefv)(/*out:ref*/ DeeObject * /*nullable*/ *__restrict dst,
                     /*in*/ DeeObject * /*nullable*/ const *__restrict src,
                     size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob = src[i];
		Dee_XIncref_untraced(ob);
		dst[i] = ob;
	}
	return dst;
}



#if (!defined(CONFIG_INLINE_INCREFV) && \
     !defined(CONFIG_NO_INLINE_INCREFV))
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_INLINE_INCREFV
#else /* !__OPTIMIZE_SIZE__ */
#define CONFIG_NO_INLINE_INCREFV
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !CONFIG_[NO_]INLINE_INCREFV */


/* Try to define `DEE_PRIVATE_MEMSETP' with platform-specific optimizations (if possible) */
#undef DEE_PRIVATE_MEMSETP
#ifdef CONFIG_INLINE_INCREFV
#ifdef CONFIG_HAVE_memsetp
#define DEE_PRIVATE_MEMSETP memsetp
#elif __SIZEOF_POINTER__ == 4
#ifdef CONFIG_NO_memsetl
#undef CONFIG_HAVE_memsetl
#elif !defined(CONFIG_HAVE_memsetl) && \
      (defined(memsetl) || defined(__memsetl_defined) || (defined(CONFIG_HAVE_STRING_H) && \
       defined(__USE_KOS)))
#define CONFIG_HAVE_memsetl
#endif
#ifndef CONFIG_HAVE_memsetl
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#define CONFIG_HAVE_memsetl
DECL_BEGIN
#undef memsetl
#ifdef __x86_64__
extern void __stosd(unsigned long *, unsigned long, unsigned __int64);
#define memsetl(dst, c, n)                   \
	(__stosd((unsigned long *)(void *)(dst), \
	         (unsigned long)(c),             \
	         (unsigned __int64)(n)),         \
	 (uint32_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __stosd(unsigned long *, unsigned long, unsigned int);
#define memsetl(dst, c, n)                   \
	(__stosd((unsigned long *)(void *)(dst), \
	         (unsigned long)(c),             \
	         (unsigned int)(n)),             \
	 (uint32_t *)(void *)(dst))
#endif /* !__x86_64__ */
#pragma intrinsic(__stosd)
DECL_END
#endif /* _MSC_VER && (__i386__ || __x86_64__) */
#endif /* !CONFIG_HAVE_memsetl */
#ifdef CONFIG_HAVE_memsetl
#define DEE_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
	memsetl(dst, (uint32_t)(pointer), num_pointers)
#endif /* CONFIG_HAVE_memsetl */
#elif __SIZEOF_POINTER__ == 8
#ifdef CONFIG_NO_memsetq
#undef CONFIG_HAVE_memsetq
#elif !defined(CONFIG_HAVE_memsetq) && \
      (defined(memsetq) || defined(__memsetq_defined) || (defined(CONFIG_HAVE_STRING_H) && \
       defined(__USE_KOS)))
#define CONFIG_HAVE_memsetq
#endif
#ifndef CONFIG_HAVE_memsetq
#if defined(_MSC_VER) && defined(__x86_64__)
#define CONFIG_HAVE_memsetq
DECL_BEGIN
extern void __stosq(unsigned long long *, unsigned long long, unsigned __int64);
#undef memsetq
#define memsetq(dst, c, n)                        \
	(__stosq((unsigned long long *)(void *)(dst), \
	         (unsigned long long)(c),             \
	         (unsigned __int64)(n)),              \
	 (uint64_t *)(void *)(dst))
#pragma intrinsic(__stosq)
DECL_END
#endif /* _MSC_VER && __x86_64__ */
#endif /* !CONFIG_HAVE_memsetq */
#ifdef CONFIG_HAVE_memsetq
#define DEE_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
	memsetq(dst, (uint64_t)(pointer), num_pointers)
#endif /* CONFIG_HAVE_memsetq */
#elif __SIZEOF_POINTER__ == 2
#ifdef CONFIG_NO_memsetw
#undef CONFIG_HAVE_memsetw
#elif !defined(CONFIG_HAVE_memsetw) && \
      (defined(memsetw) || defined(__memsetw_defined) || (defined(CONFIG_HAVE_STRING_H) && \
       defined(__USE_KOS)))
#define CONFIG_HAVE_memsetw
#endif
#ifndef CONFIG_HAVE_memsetw
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#define CONFIG_HAVE_memsetw
DECL_BEGIN
#undef memsetw
#ifdef __x86_64__
extern void __stosw(unsigned short *, unsigned short, unsigned __int64);
#define memsetw(dst, c, n)                    \
	(__stosw((unsigned short *)(void *)(dst), \
	         (unsigned short)(c),             \
	         (unsigned __int64)(n)),          \
	 (uint16_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __stosw(unsigned short *, unsigned short, unsigned int);
#define memsetw(dst, c, n)                    \
	(__stosw((unsigned short *)(void *)(dst), \
	         (unsigned short)(c),             \
	         (unsigned int)(n)),              \
	 (uint16_t *)(void *)(dst))
#endif /* !__x86_64__ */
#pragma intrinsic(__stosw)
DECL_END
#endif /* _MSC_VER && (__i386__ || __x86_64__) */
#endif /* !CONFIG_HAVE_memsetw */
#ifdef CONFIG_HAVE_memsetw
#define DEE_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
	memsetw(dst, (uint16_t)(pointer), num_pointers)
#endif /* CONFIG_HAVE_memsetw */
#elif __SIZEOF_POINTER__ == 1
#ifdef CONFIG_NO_memset
#undef CONFIG_HAVE_memset
#else
#define CONFIG_HAVE_memset
#endif
#ifndef CONFIG_HAVE_memset
#define CONFIG_HAVE_memset
DECL_BEGIN
#undef memset
#define memset dee_memset
LOCAL WUNUSED ATTR_OUTS(1, 3) void *
dee_memset(void *__restrict dst, int byte, size_t num_bytes) {
	uint8_t *dst_p = (uint8_t *)dst;
	while (num_bytes--)
		*dst_p++ = (uint8_t)(unsigned int)byte;
	return dst;
}
DECL_END
#endif /* !CONFIG_HAVE_memset */
#define DEE_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
	memset(dst, (int)(unsigned int)(__UINT8_TYPE__)(pointer), num_pointers)
#endif /* ... */
#endif /* CONFIG_INLINE_INCREFV */


#ifdef __INTELLISENSE__
#define Dee_Increfv_untraced(object_vector, object_count) ((void)(object_count), (DeeObject **)Dee_AsObject(*(object_vector)))
#define Dee_Decrefv_untraced(object_vector, object_count) ((void)(object_count), (DeeObject **)Dee_AsObject(*(object_vector)))
#define Dee_Movrefv_untraced(dst, src, object_count)      ((void)(object_count), Dee_AsObject(*(src)), (DeeObject **)Dee_AsObject(*(dst)))
#define Dee_Setrefv_untraced(dst, obj, object_count)      ((void)(object_count), Dee_AsObject(obj), (DeeObject **)Dee_AsObject(*(dst)))
#elif defined(CONFIG_INLINE_INCREFV)
#define Dee_Increfv_untraced(object_vector, object_count) \
	Dee_Increfv_untraced((DeeObject *const *)(object_vector), object_count)
LOCAL ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
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
LOCAL ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
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
LOCAL ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
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

#define Dee_Setrefv_untraced(dst, obj, object_count) \
	Dee_Setrefv_untraced((DeeObject **)(dst), (DeeObject *)(obj), object_count)
LOCAL ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv_untraced)(/*out:ref*/ DeeObject **__restrict dst,
                             /*in*/ DeeObject *obj, size_t object_count) {
#ifdef DEE_PRIVATE_MEMSETP
	Dee_Incref_n_untraced(obj, object_count);
	return (DREF DeeObject **)DEE_PRIVATE_MEMSETP(dst, obj, object_count);
#else /* DEE_PRIVATE_MEMSETP */
	size_t i;
	Dee_Incref_n_untraced(obj, object_count);
	for (i = 0; i < object_count; ++i)
		dst[i] = obj;
	return dst;
#endif /* !DEE_PRIVATE_MEMSETP */
}
#else /* CONFIG_INLINE_INCREFV */
#define Dee_Increfv_untraced(object_vector, object_count) (Dee_Increfv)((DeeObject *const *)(object_vector), object_count)
#define Dee_Decrefv_untraced(object_vector, object_count) (Dee_Decrefv)((DeeObject *const *)(object_vector), object_count)
#define Dee_Movrefv_untraced(dst, src, object_count)      (Dee_Movrefv)((DeeObject **)(dst), (DeeObject *const *)(src), object_count)
#define Dee_Setrefv_untraced(dst, obj, object_count)      (Dee_Setrefv)((DeeObject **)(dst), (DeeObject *)(obj), object_count)
#endif /* !CONFIG_INLINE_INCREFV */


#ifdef __INTELLISENSE__
#define Dee_XIncrefv_untraced(object_vector, object_count) ((void)(object_count), (DeeObject **)Dee_AsObject(*(object_vector)))
#define Dee_XDecrefv_untraced(object_vector, object_count) ((void)(object_count), (DeeObject **)Dee_AsObject(*(object_vector)))
#define Dee_XMovrefv_untraced(dst, src, object_count)      ((void)(object_count), Dee_AsObject(*(src)), (DeeObject **)Dee_AsObject(*(dst)))
#else /* __INTELLISENSE__ */
#define Dee_XIncrefv_untraced(object_vector, object_count) (Dee_XIncrefv)((DeeObject *const *)(object_vector), object_count)
#define Dee_XDecrefv_untraced(object_vector, object_count) (Dee_XDecrefv)((DeeObject *const *)(object_vector), object_count)
#define Dee_XMovrefv_untraced(dst, src, object_count)      (Dee_XMovrefv)((DeeObject **)(dst), (DeeObject *const *)(src), object_count)
#endif /* !__INTELLISENSE__ */



#ifdef CONFIG_TRACE_REFCHANGES
DFUNDEF ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **(DCALL Dee_Increfv_traced)(DeeObject *const *__restrict object_vector, size_t object_count, char const *file, int line);
DFUNDEF ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **(DCALL Dee_Decrefv_traced)(DREF DeeObject *const *__restrict object_vector, size_t object_count, char const *file, int line);
DFUNDEF ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **(DCALL Dee_Movrefv_traced)(/*out:ref*/ DeeObject **__restrict dst, /*in*/ DeeObject *const *__restrict src, size_t object_count, char const *file, int line);
DFUNDEF ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **(DCALL Dee_Setrefv_traced)(/*out:ref*/ DeeObject **__restrict dst, /*in*/ DeeObject *obj, size_t object_count, char const *file, int line);
#define Dee_Increfv(object_vector, object_count) Dee_Increfv_traced((DeeObject *const *)(object_vector), object_count, __FILE__, __LINE__)
#define Dee_Decrefv(object_vector, object_count) Dee_Decrefv_traced((DeeObject *const *)(object_vector), object_count, __FILE__, __LINE__)
#define Dee_Movrefv(dst, src, object_count)      Dee_Movrefv_traced((DeeObject **)(dst), (DeeObject *const *)(src), object_count, __FILE__, __LINE__)
#define Dee_Setrefv(dst, obj, object_count)      Dee_Setrefv_traced((DeeObject **)(dst), (DeeObject *)(obj), object_count, __FILE__, __LINE__)

#define Dee_XIncrefv_traced(object_vector, object_count, file, line) \
	Dee_XIncrefv_traced((DeeObject *const *)(object_vector), object_count, file, line)
LOCAL ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
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
LOCAL ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
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
LOCAL ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
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
#define Dee_Setrefv_traced(dst, obj, object_count, file, line)       Dee_Setrefv_untraced(dst, obj, object_count)
#define Dee_Increfv(object_vector, object_count)                     Dee_Increfv_untraced(object_vector, object_count)
#define Dee_Decrefv(object_vector, object_count)                     Dee_Decrefv_untraced(object_vector, object_count)
#define Dee_Movrefv(dst, src, object_count)                          Dee_Movrefv_untraced(dst, src, object_count)
#define Dee_Setrefv(dst, obj, object_count)                          Dee_Setrefv_untraced(dst, obj, object_count)
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
#define Dee_Setprefv_untraced(dst, obj, object_count)                 (Dee_Setrefv_untraced(dst, obj, object_count) + (object_count))
#define Dee_Incprefv_traced(object_vector, object_count, file, line)  (Dee_Increfv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_Decprefv_traced(object_vector, object_count, file, line)  (Dee_Decrefv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_Movprefv_traced(dst, src, object_count, file, line)       (Dee_Movrefv_traced(dst, src, object_count, file, line) + (object_count))
#define Dee_Setprefv_traced(dst, obj, object_count, file, line)       (Dee_Setrefv_traced(dst, obj, object_count, file, line) + (object_count))
#define Dee_Incprefv(object_vector, object_count)                     (Dee_Increfv(object_vector, object_count) + (object_count))
#define Dee_Decprefv(object_vector, object_count)                     (Dee_Decrefv(object_vector, object_count) + (object_count))
#define Dee_Movprefv(dst, src, object_count)                          (Dee_Movrefv(dst, src, object_count) + (object_count))
#define Dee_Setprefv(dst, obj, object_count)                          (Dee_Setrefv(dst, obj, object_count) + (object_count))
#define Dee_XIncprefv_untraced(object_vector, object_count)           (Dee_XIncrefv_untraced(object_vector, object_count) + (object_count))
#define Dee_XDecprefv_untraced(object_vector, object_count)           (Dee_XDecrefv_untraced(object_vector, object_count) + (object_count))
#define Dee_XMovprefv_untraced(dst, src, object_count)                (Dee_XMovrefv_untraced(dst, src, object_count) + (object_count))
#define Dee_XIncprefv_traced(object_vector, object_count, file, line) (Dee_XIncrefv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_XDecprefv_traced(object_vector, object_count, file, line) (Dee_XDecrefv_traced(object_vector, object_count, file, line) + (object_count))
#define Dee_XMovprefv_traced(dst, src, object_count, file, line)      (Dee_XMovrefv_traced(dst, src, object_count, file, line) + (object_count))
#define Dee_XIncprefv(object_vector, object_count)                    (Dee_XIncrefv(object_vector, object_count) + (object_count))
#define Dee_XDecprefv(object_vector, object_count)                    (Dee_XDecrefv(object_vector, object_count) + (object_count))
#define Dee_XMovprefv(dst, src, object_count)                         (Dee_XMovrefv(dst, src, object_count) + (object_count))


/* incref() + return `self' (may be used in type operators,
 * and receives special optimizations in some situations) */
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef)(DeeObject *__restrict self);
#ifdef CONFIG_TRACE_REFCHANGES
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef_traced)(DeeObject *__restrict self, char const *file, int line);
#define DeeObject_NewRef(self) DeeObject_NewRef_traced(self, __FILE__, __LINE__)
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_NewRef_traced(self, file, line) DeeObject_NewRef(self)
#endif /* !CONFIG_TRACE_REFCHANGES */


/* Inline version of `DeeObject_NewRef()' */
#ifndef __OPTIMIZE_SIZE__
LOCAL ATTR_ARTIFICIAL ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef_inline)(DeeObject *__restrict self) {
	Dee_Incref(self);
	return self;
}
#ifdef CONFIG_TRACE_REFCHANGES
#define DeeObject_NewRef_inline(self) \
	DeeObject_NewRef_inline_traced(self, __FILE__, __LINE__)
LOCAL ATTR_ARTIFICIAL ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef_inline_traced)(DeeObject *__restrict self,
                                       char const *file, int line) {
	Dee_Incref_traced(self, file, line);
	return self;
}
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_NewRef_inline_traced(self, file, line) \
	DeeObject_NewRef_inline(self)
#endif /* !CONFIG_TRACE_REFCHANGES */
#undef DeeObject_NewRef
#undef DeeObject_NewRef_traced
#define DeeObject_NewRef(self) DeeObject_NewRef_inline(self)
#define DeeObject_NewRef_traced(self, file, line) \
	DeeObject_NewRef_inline_traced(self, file, line)
#endif /* !__OPTIMIZE_SIZE__ */


/* Serialization address (points into the abstract serialization buffer) */
#ifndef Dee_seraddr_t_DEFINED
#define Dee_seraddr_t_DEFINED
typedef __UINTPTR_TYPE__ Dee_seraddr_t;
#endif /* !Dee_seraddr_t_DEFINED */

struct Dee_serial;

#if 0
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
myobject_serialize(MyObject *__restrict self, DeeSerial *__restrict writer, Dee_seraddr_t addr) {
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
myobject_serialize(MyObject *__restrict self, DeeSerial *__restrict writer) {
}
#endif





#if defined(__INTELLISENSE__) && defined(__cplusplus)
extern "C++" {namespace __intern {
#define _Dee_REQUIRE_VAR_tp_ctor(tp_ctor) ::__intern::__Dee_REQUIRE_VAR_tp_ctor(tp_ctor)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_VAR_tp_ctor(Tobj *(DCALL *)(void)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_VAR_tp_ctor(decltype(nullptr));
#define _Dee_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor) ::__intern::__Dee_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_VAR_tp_copy_ctor(Tobj *(DCALL *)(Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_VAR_tp_copy_ctor(decltype(nullptr));
#define _Dee_REQUIRE_VAR_tp_any_ctor(tp_any_ctor) ::__intern::__Dee_REQUIRE_VAR_tp_any_ctor(tp_any_ctor)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_VAR_tp_any_ctor(Tobj *(DCALL *)(size_t, DeeObject *const *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_VAR_tp_any_ctor(decltype(nullptr));
#define _Dee_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw) ::__intern::__Dee_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_VAR_tp_any_ctor_kw(Tobj *(DCALL *)(size_t, DeeObject *const *, DeeObject *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_VAR_tp_any_ctor_kw(decltype(nullptr));
#define _Dee_REQUIRE_VAR_tp_serialize(tp_serialize) ::__intern::__Dee_REQUIRE_VAR_tp_serialize(tp_serialize)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_VAR_tp_serialize(Dee_seraddr_t (DCALL *)(Tobj *__restrict, struct Dee_serial *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_VAR_tp_serialize(decltype(nullptr));

#define _Dee_REQUIRE_ALLOC_tp_ctor(tp_ctor) ::__intern::__Dee_REQUIRE_ALLOC_tp_ctor(tp_ctor)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_ctor(int (DCALL *)(Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_ctor(decltype(nullptr));
#define _Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor) ::__intern::__Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_copy_ctor(int (DCALL *)(Tobj *__restrict, Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_copy_ctor(decltype(nullptr));
#define _Dee_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor) ::__intern::__Dee_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_any_ctor(int (DCALL *)(Tobj *__restrict, size_t, DeeObject *const *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_any_ctor(decltype(nullptr));
#define _Dee_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw) ::__intern::__Dee_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_any_ctor_kw(int (DCALL *)(Tobj *__restrict, size_t, DeeObject *const *, DeeObject *)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_any_ctor_kw(decltype(nullptr));
#define _Dee_REQUIRE_ALLOC_tp_serialize(tp_serialize) ::__intern::__Dee_REQUIRE_ALLOC_tp_serialize(tp_serialize)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_serialize(int (DCALL *)(Tobj *__restrict, struct Dee_serial *__restrict, Dee_seraddr_t)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_ALLOC_tp_serialize(decltype(nullptr));

#define _Dee_REQUIRE_tp_alloc(tp_alloc) ::__intern::__Dee_REQUIRE_tp_alloc(tp_alloc)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_tp_alloc(Tobj *(DCALL *)(void)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_tp_alloc(void *(DCALL *)(void));
Dee_funptr_t __Dee_REQUIRE_tp_alloc(decltype(nullptr));
#define _Dee_REQUIRE_tp_free(tp_free) ::__intern::__Dee_REQUIRE_tp_free(tp_free)
template<class Tobj> Dee_funptr_t __Dee_REQUIRE_tp_free(void (DCALL *)(Tobj *__restrict)/*, decltype(((Tobj *)0)->ob_refcnt)=0*/);
Dee_funptr_t __Dee_REQUIRE_tp_free(void (DCALL *)(void *__restrict));
Dee_funptr_t __Dee_REQUIRE_tp_free(decltype(nullptr));
}}
#else /* __INTELLISENSE__ && __cplusplus */
#define _Dee_REQUIRE_VAR_tp_ctor(tp_ctor)                 (Dee_funptr_t)(tp_ctor)
#define _Dee_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor)       (Dee_funptr_t)(tp_copy_ctor)
#define _Dee_REQUIRE_VAR_tp_any_ctor(tp_any_ctor)         (Dee_funptr_t)(tp_any_ctor)
#define _Dee_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw)   (Dee_funptr_t)(tp_any_ctor_kw)
#define _Dee_REQUIRE_VAR_tp_serialize(tp_serialize)       (Dee_funptr_t)(tp_serialize)
#define _Dee_REQUIRE_ALLOC_tp_ctor(tp_ctor)               (Dee_funptr_t)(tp_ctor)
#define _Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor)     (Dee_funptr_t)(tp_copy_ctor)
#define _Dee_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor)       (Dee_funptr_t)(tp_any_ctor)
#define _Dee_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw) (Dee_funptr_t)(tp_any_ctor_kw)
#define _Dee_REQUIRE_ALLOC_tp_serialize(tp_serialize)     (Dee_funptr_t)(tp_serialize)
#define _Dee_REQUIRE_tp_alloc(tp_alloc)                   (Dee_funptr_t)(tp_alloc)
#define _Dee_REQUIRE_tp_free(tp_free)                     (Dee_funptr_t)(tp_free)
#endif /* !__INTELLISENSE__ || !__cplusplus */

/************************************************************************/
/* Initializers for TP_FVARIABLE types                                  */
/************************************************************************/
#define Dee_TYPE_CONSTRUCTOR_INIT_VAR(tp_ctor,           \
                                      tp_copy_ctor,      \
                                      tp_deep_ctor,      \
                                      tp_any_ctor,       \
                                      tp_any_ctor_kw,    \
                                      tp_serialize,      \
                                      tp_free)           \
	{{                                                   \
		_Dee_REQUIRE_VAR_tp_ctor(tp_ctor),               \
		_Dee_REQUIRE_VAR_tp_copy_ctor(tp_copy_ctor),     \
		_Dee_REQUIRE_VAR_tp_copy_ctor(tp_deep_ctor),     \
		_Dee_REQUIRE_VAR_tp_any_ctor(tp_any_ctor),       \
		_Dee_REQUIRE_VAR_tp_any_ctor_kw(tp_any_ctor_kw), \
		_Dee_REQUIRE_VAR_tp_serialize(tp_serialize),     \
		_Dee_REQUIRE_tp_free(tp_free),                   \
		{ NULL }                                         \
	}}


/************************************************************************/
/* Initializers for non-TP_FVARIABLE types                              */
/************************************************************************/

/* With custom allocator */
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor,           \
                                        tp_copy_ctor,      \
                                        tp_deep_ctor,      \
                                        tp_any_ctor,       \
                                        tp_any_ctor_kw,    \
                                        tp_serialize,      \
                                        tp_alloc,          \
                                        tp_free)           \
	{{                                                     \
		_Dee_REQUIRE_ALLOC_tp_ctor(tp_ctor),               \
		_Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor),     \
		_Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_deep_ctor),     \
		_Dee_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor),       \
		_Dee_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw), \
		_Dee_REQUIRE_ALLOC_tp_serialize(tp_serialize),     \
		_Dee_REQUIRE_tp_free(tp_free),                     \
		{ _Dee_REQUIRE_tp_alloc(tp_alloc) }                \
	}}

/* Specifies an automatic object allocator. */
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO(T, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED(max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED(tp_instance_size, \
                                                  tp_ctor,          \
                                                  tp_copy_ctor,     \
                                                  tp_deep_ctor,     \
                                                  tp_any_ctor,      \
                                                  tp_any_ctor_kw,   \
                                                  tp_serialize)     \
	{{                                                              \
		_Dee_REQUIRE_ALLOC_tp_ctor(tp_ctor),                        \
		_Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_copy_ctor),              \
		_Dee_REQUIRE_ALLOC_tp_copy_ctor(tp_deep_ctor),              \
		_Dee_REQUIRE_ALLOC_tp_any_ctor(tp_any_ctor),                \
		_Dee_REQUIRE_ALLOC_tp_any_ctor_kw(tp_any_ctor_kw),          \
		_Dee_REQUIRE_ALLOC_tp_serialize(tp_serialize),              \
		(Dee_funptr_t)NULL,                                         \
		{ (Dee_funptr_t)(void *)(uintptr_t)(tp_instance_size) }     \
	}}

/* Specifies an allocator that may provides optimizations
 * for types with a FIXED size (which most objects have). */
#ifdef GUARD_DEEMON_ALLOC_H
#ifdef CONFIG_NO_OBJECT_SLABS
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R    Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED      Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC   Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED      Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC   Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO
#else /* CONFIG_NO_OBJECT_SLABS */
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                                   \
	                                DeeSlab_Invoke(&DeeObject_SlabMalloc, max_tp_instance_size, , (void *(DCALL *)(void))(void *)(uintptr_t)(max_tp_instance_size)),  \
	                                DeeSlab_Invoke(&DeeObject_SlabFree, min_tp_instance_size, , (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                                      \
	                                DeeSlab_Invoke(&DeeGCObject_SlabMalloc, max_tp_instance_size, , (void *(DCALL *)(void))(void *)(uintptr_t)(max_tp_instance_size)),   \
	                                DeeSlab_Invoke(&DeeGCObject_SlabFree, min_tp_instance_size, , (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED(tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(tp_instance_size, tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R(tp_instance_size, tp_instance_size, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED(T, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(T, tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(sizeof(T), tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#endif /* !CONFIG_NO_OBJECT_SLABS */

/* Same as `Dee_TYPE_CONSTRUCTOR_INIT_FIXED()', but don't link against
 * dedicated allocator functions when doing so would require the creation
 * of relocations that might cause loading times to become larger. */
#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S    Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S    Dee_TYPE_CONSTRUCTOR_INIT_FIXED
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */

#endif /* GUARD_DEEMON_ALLOC_H */


#undef tp_alloc
struct Dee_type_constructor {
	/* Instance constructors/destructors. */
	union {
		struct {
			Dee_funptr_t _tp_init0_; /* tp_ctor */
			Dee_funptr_t _tp_init1_; /* tp_copy_ctor */
			Dee_funptr_t _tp_init2_; /* tp_deep_ctor */
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
			WUNUSED_T NONNULL_T((1, 2))               int (DCALL *tp_deep_ctor)(DeeObject *__restrict self, DeeObject *__restrict other);
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
				size_t tp_instance_size;       /* [valid_if(tp_free == NULL)] */
				void *(DCALL *tp_alloc)(void); /* [valid_if(tp_free != NULL)] */
			}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
			_dee_aunion
#define tp_instance_size _dee_aunion.tp_instance_size
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
			WUNUSED_T NONNULL_T((1))   DREF DeeObject *(DCALL *tp_deep_ctor)(DeeObject *__restrict other);
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
#define tp_alloc _dee_aunion.tp_alloc
#define tp_var   _dee_aunion.tp_var
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
	 * >>         if unlikely(!new_object)
	 * >>             goto err;
	 * >>         if unlikely(!Dee_DeepCopyAddAssoc(old_object, new_object))
	 * >>             goto err_new;
	 * >>         error = tp_deepload(new_object);
	 * >>         if (error)
	 * >>             goto err_new;
	 * >>         deepcopy_end();
	 * >>     } else if (deepcopy_end()) {
	 * >>         // This is a recursive deepcopy operation, so we must still track the new object
	 * >>         if unlikely(!Dee_DeepCopyAddAssoc(old_object, new_object))
	 * >>             goto err_new;
	 * >>     }
	 * >>     return new_object;
	 * >> err_new:
	 * >>     Dee_Decref(new_object);
	 * >> err:
	 * >>     deepcopy_end();
	 * >>     return NULL;
	 * @return: == 0: Success
	 * @return: != 0: Error was thrown */
	WUNUSED_T NONNULL_T((1)) int (DCALL *tp_deepload)(DeeObject *__restrict self);

	/* Callback for `DeeObject_Destroy()' (usually auto-assigned by
	 * `DeeType_RequireDestroy()', but can be overwritten with a
	 * custom implementation). */
	Dee_tp_destroy_t tp_destroy;
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

	/* Clear unused heap caches of the object. Called when deemon is
	 * running low on memory (iow: from "Dee_CollectMemory") */
	NONNULL_T((1)) bool (DCALL *tp_cc)(DeeObject *__restrict self);
};


/* GC destruction priority levels of builtin types. */
#define Dee_GC_PRIORITY_LATE     0x0000 /* (Preferably) destroyed last. */
#define Dee_GC_PRIORITY_CLASS    0xfd00 /* User-classes. */
#define Dee_GC_PRIORITY_INSTANCE 0xfe00 /* Instances of user-classes. */
#define Dee_GC_PRIORITY_MODULE   0xff00 /* Module objects. */
#define Dee_GC_PRIORITY_EARLY    0xffff /* (Preferably) destroyed before anything else. */


/* Return values for `tp_int32' and `tp_int64' */
#define Dee_INT_SIGNED   0    /* The saved integer value is signed. */
#define Dee_INT_UNSIGNED 1    /* The saved integer value is unsigned. */
#define Dee_INT_ERROR    (-1) /* An error was thrown */

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

struct Dee_type_nii;
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

	/* Optional iterator-extensions for providing optimized (but
	 * less generic) variants for various iterator operations.
	 * NOTE: The compare sub-structure was chosen for this, as native
	 *       iterators usually implement compare operators to allow
	 *       them to be ordered with other operators. */
	struct Dee_type_nii Dee_tpconst *tp_nii; /* TODO: Deprecated */
};

/* Suggested return values for `DeeObject_HasItem()' and `DeeObject_HasAttr()'
 * In actuality:
 * @return: < 0:  Error
 * @return: == 0: No (item/attr does not exist)
 * @return: > 0:  Yes (item/attr does exist) */
#define Dee_HAS_ERR (-1)
#define Dee_HAS_NO  0
#define Dee_HAS_YES 1

/* Helper methods for testing return values of `DeeObject_HasItem()' and `DeeObject_HasAttr()' */
#define Dee_HAS_ISERR(x) unlikely((x) < 0)
#define Dee_HAS_ISNO(x)  ((x) == 0)
#define Dee_HAS_ISYES(x) ((x) > 0)

/* >> #define Dee_HAS_FROMBOOL(has_item) ((has_item) ? Dee_HAS_YES : Dee_HAS_NO) */
#if Dee_HAS_YES > 0 && Dee_HAS_NO == 0
#define Dee_HAS_FROMBOOL(has_item) (has_item)
#else /* Dee_HAS_YES > 0 && Dee_HAS_NO == 0 */
#define Dee_HAS_FROMBOOL(has_item) ((has_item) ? Dee_HAS_YES : Dee_HAS_NO)
#endif /* Dee_HAS_YES <= 0 || Dee_HAS_NO != 0 */

/* Possible values returned by C-API isbound checking functions.
 * These values have been intentionally chosen so-as to be binary-
 * compatible with `DeeObject_HasItem()' and `DeeObject_HasAttr()' */
#define Dee_BOUND_ERR      Dee_HAS_ERR
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define Dee_BOUND_MISSING  Dee_HAS_NO
#define Dee_BOUND_YES      Dee_HAS_YES
#define Dee_BOUND_NO       2
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
#define Dee_BOUND_MISSING  (-2)
#define Dee_BOUND_YES      1
#define Dee_BOUND_NO       0
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

/* #define Dee_BOUND_ISBOUND(x) ((x) == Dee_BOUND_YES) */
#define Dee_BOUND_ISBOUND(x) ((x) == Dee_BOUND_YES)

/* #define Dee_BOUND_ISUNBOUND(x) ((x) == Dee_BOUND_NO) */
#define Dee_BOUND_ISUNBOUND(x) ((x) == Dee_BOUND_NO)

/* #define Dee_BOUND_ISMISSING(x) ((x) == Dee_BOUND_MISSING) */
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define Dee_BOUND_ISMISSING(x) Dee_HAS_ISNO(x)
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
#define Dee_BOUND_ISMISSING(x) ((x) == Dee_BOUND_MISSING)
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

/* #define Dee_BOUND_ISERR(x) ((x) == Dee_BOUND_ERR) */
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define Dee_BOUND_ISERR(x) Dee_HAS_ISERR(x)
#elif Dee_BOUND_ERR < 0 && Dee_BOUND_NO >= 0 && Dee_BOUND_YES >= 0 && Dee_BOUND_MISSING >= 0
#define Dee_BOUND_ISERR(x) ((x) < 0)
#else /* Dee_BOUND_ERR < 0 && Dee_BOUND_NO >= 0 && Dee_BOUND_YES >= 0 && Dee_BOUND_MISSING >= 0 */
#define Dee_BOUND_ISERR(x) ((x) == Dee_BOUND_ERR)
#endif /* Dee_BOUND_ERR >= 0 || Dee_BOUND_NO < 0 || Dee_BOUND_YES < 0 || Dee_BOUND_MISSING < 0 */

/* #define Dee_BOUND_ISPRESENT(x) ((x) == Dee_BOUND_YES || (x) == Dee_BOUND_NO) */
#if Dee_BOUND_YES > 0 && Dee_BOUND_NO > 0 && Dee_BOUND_MISSING <= 0 && Dee_BOUND_ERR <= 0
#define Dee_BOUND_ISPRESENT(x) ((x) > 0)
#elif Dee_BOUND_YES >= 0 && Dee_BOUND_NO >= 0 && Dee_BOUND_MISSING < 0 && Dee_BOUND_ERR < 0
#define Dee_BOUND_ISPRESENT(x) ((x) >= 0)
#elif Dee_BOUND_YES <= 0 && Dee_BOUND_NO <= 0 && Dee_BOUND_MISSING > 0 && Dee_BOUND_ERR > 0
#define Dee_BOUND_ISPRESENT(x) ((x) <= 0)
#elif Dee_BOUND_YES < 0 && Dee_BOUND_NO < 0 && Dee_BOUND_MISSING >= 0 && Dee_BOUND_ERR >= 0
#define Dee_BOUND_ISPRESENT(x) ((x) < 0)
#else /* ... */
#define Dee_BOUND_ISPRESENT(x) ((x) == Dee_BOUND_YES || (x) == Dee_BOUND_NO)
#endif /* !... */

/* #define Dee_BOUND_ISMISSING_OR_UNBOUND(x) ((x) == Dee_BOUND_MISSING || (x) == Dee_BOUND_NO) */
#if Dee_BOUND_MISSING > 0 && Dee_BOUND_NO > 0 && Dee_BOUND_YES <= 0 && Dee_BOUND_ERR <= 0
#define Dee_BOUND_ISMISSING_OR_UNBOUND(x) ((x) > 0)
#elif Dee_BOUND_MISSING >= 0 && Dee_BOUND_NO >= 0 && Dee_BOUND_YES < 0 && Dee_BOUND_ERR < 0
#define Dee_BOUND_ISMISSING_OR_UNBOUND(x) ((x) >= 0)
#elif Dee_BOUND_MISSING <= 0 && Dee_BOUND_NO <= 0 && Dee_BOUND_YES > 0 && Dee_BOUND_ERR > 0
#define Dee_BOUND_ISMISSING_OR_UNBOUND(x) ((x) <= 0)
#elif Dee_BOUND_MISSING < 0 && Dee_BOUND_NO < 0 && Dee_BOUND_YES >= 0 && Dee_BOUND_ERR >= 0
#define Dee_BOUND_ISMISSING_OR_UNBOUND(x) ((x) < 0)
#elif !(Dee_BOUND_MISSING & 1) && !(Dee_BOUND_NO & 1) && (Dee_BOUND_YES & 1) && (Dee_BOUND_ERR & 1)
#define Dee_BOUND_ISMISSING_OR_UNBOUND(x) (!((x) & 1))
#else /* ... */
#define Dee_BOUND_ISMISSING_OR_UNBOUND(x) ((x) == Dee_BOUND_MISSING || (x) == Dee_BOUND_NO)
#endif /* !... */

/* #define Dee_BOUND_FROMBOOL(is_bound) ((is_bound) ? Dee_BOUND_YES : Dee_BOUND_NO) */
#if Dee_BOUND_NO == 0 && Dee_BOUND_YES == 1
#define Dee_BOUND_FROMBOOL(is_bound) ((int)!!(is_bound))
#elif Dee_BOUND_NO == 2 && Dee_BOUND_YES == 1
#define Dee_BOUND_FROMBOOL(is_bound) (2 - (int)!!(is_bound))
#else /* Dee_BOUND_NO == ... && Dee_BOUND_YES == ... */
#define Dee_BOUND_FROMBOOL(is_bound) ((is_bound) ? Dee_BOUND_YES : Dee_BOUND_NO)
#endif /* Dee_BOUND_NO != ... || Dee_BOUND_YES != ... */

/* #define Dee_BOUND_FROMPRESENT_BOUND(is_present) ((is_present) ? Dee_BOUND_YES : Dee_BOUND_MISSING)
 * Considers is_present == true as bound */
#if Dee_BOUND_MISSING == 0 && Dee_BOUND_YES == 1
#define Dee_BOUND_FROMPRESENT_BOUND(is_present) ((int)!!(is_present))
#else /* Dee_BOUND_MISSING == 0 && Dee_BOUND_YES == 1 */
#define Dee_BOUND_FROMPRESENT_BOUND(is_present) ((is_present) ? Dee_BOUND_YES : Dee_BOUND_MISSING)
#endif /* Dee_BOUND_MISSING != 0 || Dee_BOUND_YES != 1 */

/* #define Dee_BOUND_FROMPRESENT_UNBOUND(is_present) ((is_present) ? Dee_BOUND_NO : Dee_BOUND_MISSING)
 * Considers is_present == true as unbound */
#if Dee_BOUND_MISSING == 0 && Dee_BOUND_NO == 1
#define Dee_BOUND_FROMPRESENT_UNBOUND(is_present) ((int)!!(is_present))
#elif Dee_BOUND_MISSING == 0 && Dee_BOUND_NO == 2
#define Dee_BOUND_FROMPRESENT_UNBOUND(is_present) ((int)!!(is_present) << 1)
#else /* Dee_BOUND_MISSING == 0 && Dee_BOUND_YES == 1 */
#define Dee_BOUND_FROMPRESENT_UNBOUND(is_present) ((is_present) ? Dee_BOUND_YES : Dee_BOUND_MISSING)
#endif /* Dee_BOUND_MISSING != 0 || Dee_BOUND_YES != 1 */

/* Convert a Dee_BOUND_* value into the equivalent return value of hasattr/hasitem */
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define Dee_BOUND_ASHAS(bound_status)          (bound_status)
#define Dee_BOUND_ASHAS_NOEXCEPT(bound_status) (bound_status)
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
#define Dee_BOUND_ASHAS(bound_status)          (Dee_BOUND_ISERR(bound_status) ? -1 : !Dee_BOUND_ISMISSING(bound_status))
#define Dee_BOUND_ASHAS_NOEXCEPT(bound_status) (!Dee_BOUND_ISMISSING(bound_status))
#endif /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

/* Considers has_value == true as bound */
#define Dee_BOUND_FROMHAS_BOUND(has_value) \
	(Dee_HAS_ISERR(has_value) ? Dee_BOUND_ERR : Dee_BOUND_FROMPRESENT_BOUND(has_value))

/* Considers has_value == true as unbound */
#define Dee_BOUND_FROMHAS_UNBOUND(has_value) \
	(Dee_HAS_ISERR(has_value) ? Dee_BOUND_ERR : Dee_BOUND_FROMPRESENT_UNBOUND(has_value))

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

	/* Advance an iterator by "step" items.
	 * @return: step:       Success.
	 * @return: < step:     Success, but advancing was stopped prematurely because ITER_DONE
	 *                      was encountered. Return value is the # of successfully skipped
	 *                      entries before "ITER_DONE" was encountered.
	 * @return: (size_t)-1: Error. */
	WUNUSED_T NONNULL_T((1)) size_t (DCALL *tp_advance)(DeeObject *__restrict self, size_t step);
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
myob_advance(MyObject *__restrict self, size_t step) {
	(void)self;
	(void)step;
	return (size_t)DeeError_NOTIMPLEMENTED();
}

PRIVATE struct type_iterator myob_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&myob_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&myob_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&myob_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&myob_advance,
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

typedef struct {
	void  *bb_base; /* [0..bb_size][const] Base address of the buffer.
	                 * NOTE: Only writable if the buffer was acquired with `Dee_BUFFER_FWRITABLE' set. */
	size_t bb_size; /* [const] Size of the buffer (in bytes) */
} DeeBuffer;
#define DeeBuffer_INIT(base, size) { base, size }
#define DeeBuffer_Fini(self) (void)0


struct Dee_type_buffer {
	/* Low-level buffer interface. */

	/* When implemented, `tp_getbuf' must fill in at least `bb_base' and `bb_size'
	 * @param: flags: Set of `DEE_BUFFER_F*' */
	WUNUSED_T NONNULL_T((1, 2))
	int  (DCALL *tp_getbuf)(DeeObject *__restrict self,
	                        DeeBuffer *__restrict info,
	                        unsigned int flags);
#define Dee_BUFFER_FREADONLY 0x0000 /* Acquire the buffer for reading. */
#define Dee_BUFFER_FWRITABLE 0x0001 /* Acquire the buffer for reading / writing. */
#define Dee_BUFFER_FMASK     0x0001 /* Mask of known buffer flags. */

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
#if Dee_BOUND_YES == 1
#if defined(DCALL_RETURN_COMMON) || __SIZEOF_SIZE_T__ == __SIZEOF_INT__
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
#define Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP         0x00000500 /* >> (for (local x: thisarg) IS_CONSTEXPR(x.operator deepcopy)) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ             0x00000600 /* >> (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local a, b: zip(thisarg, arg)) IS_CONSTEXPR(a == b, a != b, [a.operator hash(), b.operator hash()]))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCMP               0x00000700 /* >> (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local a, b: zip(thisarg, arg)) IS_CONSTEXPR(a == b, a != b, a < b, a > b, a <= b, a >= b))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS          0x00000800 /* >> (for (local arg: ...) for (local elem: thisarg) IS_CONSTEXPR(elem == arg)) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ             0x00000900 /* TODO (for sets and maps) */
#define Dee_METHOD_FCONSTCALL_IF_SET_CONSTCMP               0x00000a00 /* TODO (for sets and maps) */
#define Dee_METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS          0x00000b00 /* TODO (for sets and maps) */
#define Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST_ROBYTES 0x00000c00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_WRITABLE(thisarg)) && (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local x: arg) DeeType_IsConstCastable(Dee_TYPE(x)))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES  0x00000d00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_WRITABLE(thisarg)) && (for (local arg: ...) IS_CONSTEXPR(arg.operator iter) && (for (local x: arg) IS_CONSTEXPR(x.operator str))) && ...; */
#define Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES     0x00000e00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_WRITABLE(thisarg)) && ((for (local arg: ...) DeeBytes_Check(arg) ? !DeeBytes_WRITABLE(arg) : DeeType_IsConstCastable(Dee_TYPE(arg))) && ...); */
#define Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES      0x00000f00 /* >> (!DeeBytes_Check(thisarg) || !DeeBytes_WRITABLE(thisarg)) && ((for (local arg: ...) DeeBytes_Check(arg) ? !DeeBytes_WRITABLE(arg) : IS_CONSTEXPR(arg.operator str)) && ...); */
#define Dee_METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL          0x00001000 /* Special casing for `OPERATOR_CALL' of `DeeInstanceMethod_Type', `DeeObjMethod_Type', `DeeKwObjMethod_Type', `DeeClsMethod_Type', `DeeKwClsMethod_Type', `DeeClsProperty_Type', `DeeClsMember_Type', `DeeCMethod_Type' and `DeeKwCMethod_Type' */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR            0x00001100 /* Foreach field in this,args...: IS_CONSTEXPR(f.operator str)    (fields are STRUCT_OBJECT-like tp_members) */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR           0x00001200 /* Foreach field in this,args...: IS_CONSTEXPR(f.operator repr)   (fields are STRUCT_OBJECT-like tp_members) */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ          0x00001300 /* Foreach field in pair(this,arg0): IS_CONSTEXPR(a == b, a != b, [a.operator hash(), b.operator hash()]) */
#define Dee_METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP            0x00001400 /* Foreach field in pair(this,arg0): IS_CONSTEXPR(a == b, a != b, a < b, a > b, a <= b, a >= b) */

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
#define METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP         Dee_METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP
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

#define DEE_PRIVATE_STRUCT_INT1  Dee_STRUCT_INT8
#define DEE_PRIVATE_STRUCT_INT2  Dee_STRUCT_INT16
#define DEE_PRIVATE_STRUCT_INT4  Dee_STRUCT_INT32
#define DEE_PRIVATE_STRUCT_INT8  Dee_STRUCT_INT64
#define DEE_PRIVATE_STRUCT_INT16 Dee_STRUCT_INT128
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
#define STRUCT_OBJECT      Dee_STRUCT_OBJECT      /* `[0..1] DREF DeeObject *const' (raise `Error.AttributeError' if `NULL') */
#define STRUCT_WOBJECT     Dee_STRUCT_WOBJECT     /* `[0..1] struct Dee_weakref' (raise `Error.AttributeError' if locking fails) */
#define STRUCT_OBJECT_OPT  Dee_STRUCT_OBJECT_OPT  /* `[0..1] DREF DeeObject *const' (return `none' if NULL) */
#define STRUCT_WOBJECT_OPT Dee_STRUCT_WOBJECT_OPT /* `[0..1] struct Dee_weakref' (return `none' if locking fails) */
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
#define Dee_PRIVATE_STRUCT_BOOLBIT_OFFSET(struct_type, field, mask)     \
	offsetof(struct_type, field) + (sizeof(((struct_type *)0)->field) - \
	                                (1 + Dee_PRIVATE_STRUCT_BOOLBIT_ADDEND(mask)))
#else /* __BYTE_ORDER__ == ... */
#error "Unsupported endian"
#endif /* __BYTE_ORDER__ != ... */
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
#define OPERATOR_DEEPCOPY     0x0002 /* `operator deepcopy(other: type(this))'                      - `__deepcopy__'    - `tp_deep_ctor'. */
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
#define OPERATOR_DELATTR      0x0039 /* `operator del . (string attr)'                              - `__delattr__'     - `tp_delattr'. */
#define OPERATOR_SETATTR      0x003a /* `operator .= (string attr, value: Object)'                  - `__setattr__'     - `tp_setattr'. */
#define OPERATOR_ENUMATTR     0x003b /* `operator enumattr(): {attribute...}'                       - `__enumattr__'    - `tp_iterattr'. */
#define OPERATOR_ENTER        0x003c /* `operator enter()'                                          - `__enter__'       - `tp_enter'. */
#define OPERATOR_LEAVE        0x003d /* `operator leave()'                                          - `__leave__'       - `tp_leave'. */
#define OPERATOR_USERCOUNT    0x003e /* Number of user-accessible operators. (Used by `class' types) */
#define OPERATOR_EXTENDED(x)  (0x1000 + (x)) /* Extended operator codes. (Type-specific; may be re-used) */
#define OPERATOR_ISINPLACE(x) ((x) >= OPERATOR_INC && (x) <= OPERATOR_INPLACE_POW)

/* Operators not exposed to user-code. */
#define OPERATOR_VISIT        0x8000 /* `tp_visit'. */
#define OPERATOR_CLEAR        0x8001 /* `tp_clear'. */
#define OPERATOR_PCLEAR       0x8002 /* `tp_pclear'. */
#define OPERATOR_GETBUF       0x8003 /* `tp_getbuf'. */

/* Fake operators (for use with `DeeFormat_PrintOperatorRepr()'). */
#define FAKE_OPERATOR_IS          0xff00 /* `a is b' */
#define FAKE_OPERATOR_SAME_OBJECT 0xff01 /* `a === b' */
#define FAKE_OPERATOR_DIFF_OBJECT 0xff02 /* `a !== b' */

/* Operator association ranges. */
#define OPERATOR_TYPEMIN    OPERATOR_CONSTRUCTOR
#define OPERATOR_TYPEMAX    OPERATOR_CALL
#define OPERATOR_MATHMIN    OPERATOR_INT
#define OPERATOR_MATHMAX    OPERATOR_INPLACE_POW
#define OPERATOR_CMPMIN     OPERATOR_HASH
#define OPERATOR_CMPMAX     OPERATOR_GE
#define OPERATOR_SEQMIN     OPERATOR_ITER
#define OPERATOR_SEQMAX     OPERATOR_SETRANGE
#define OPERATOR_ATTRMIN    OPERATOR_GETATTR
#define OPERATOR_ATTRMAX    OPERATOR_ENUMATTR
#define OPERATOR_WITHMIN    OPERATOR_ENTER
#define OPERATOR_WITHMAX    OPERATOR_LEAVE
#define OPERATOR_PRIVMIN    OPERATOR_VISIT
#define OPERATOR_PRIVMAX    OPERATOR_GETBUF

/* Aliases that should be used when operators need to appear sorted by ID.
 *
 * By using these, you can ensure a proper operator order by simply doing
 * a lexicographical line sort. */
#define OPERATOR_0000_CONSTRUCTOR  OPERATOR_CONSTRUCTOR
#define OPERATOR_0001_COPY         OPERATOR_COPY
#define OPERATOR_0002_DEEPCOPY     OPERATOR_DEEPCOPY
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
#define OPERATOR_8000_VISIT        OPERATOR_VISIT
#define OPERATOR_8001_CLEAR        OPERATOR_CLEAR
#define OPERATOR_8002_PCLEAR       OPERATOR_PCLEAR
#define OPERATOR_8003_GETBUF       OPERATOR_GETBUF
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
#define _Dee_OPINFO_INIT_AS_CUSTOM(id, flags, invoke)                                                           \
	{ id, OPCLASS_CUSTOM, 0, OPCC_SPECIAL, { _DEE_UINTPTR_AS_CHAR_LIST((char)(unsigned char), flags) }, "", "", \
	  (struct Dee_operator_invoke Dee_tpconst *)(void Dee_tpconst *)(void const *)Dee_REQUIRES_OPERATOR_INVOKE_CB(invoke) }
#ifdef DEE_SOURCE
#define OPINFO_INIT Dee_OPINFO_INIT
#endif /* DEE_SOURCE */

#if __SIZEOF_POINTER__ == 4 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT32_C(0xff)), T(((v) & UINT32_C(0xff00)) >> 8), T(((v) & UINT32_C(0xff0000)) >> 16), T(((v) & UINT32_C(0xff000000)) >> 24)
#elif __SIZEOF_POINTER__ == 8 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT64_C(0xff)), T(((v) & UINT64_C(0xff00)) >> 8), T(((v) & UINT64_C(0xff0000)) >> 16), T(((v) & UINT64_C(0xff000000)) >> 24), T(((v) & UINT64_C(0xff00000000)) >> 32), T(((v) & UINT64_C(0xff0000000000)) >> 40), T(((v) & UINT64_C(0xff000000000000)) >> 48), T(((v) & UINT64_C(0xff00000000000000)) >> 56)
#elif __SIZEOF_POINTER__ == 4 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T(((v) & UINT32_C(0xff000000)) >> 24), T(((v) & UINT32_C(0xff0000)) >> 16), T(((v) & UINT32_C(0xff00)) >> 8), T((v) & UINT32_C(0xff))
#elif __SIZEOF_POINTER__ == 8 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T(((v) & UINT64_C(0xff00000000000000)) >> 56), T(((v) & UINT64_C(0xff000000000000)) >> 48), T(((v) & UINT64_C(0xff0000000000)) >> 40), T(((v) & UINT64_C(0xff00000000)) >> 32), T(((v) & UINT64_C(0xff000000)) >> 24), T(((v) & UINT64_C(0xff0000)) >> 16), T(((v) & UINT64_C(0xff00)) >> 8), T((v) & UINT64_C(0xff))
#elif __SIZEOF_POINTER__ == 2 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT16_C(0xff)), T(((v) & UINT16_C(0xff00)) >> 8)
#elif __SIZEOF_POINTER__ == 2 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T(((v) & UINT16_C(0xff00)) >> 8), T((v) & UINT16_C(0xff))
#elif __SIZEOF_POINTER__ == 1
#define _DEE_UINTPTR_AS_CHAR_LIST(T, v) T((v) & UINT8_C(0xff))
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
#define to_decl  _dee_aunion.to_decl
#define to_spec  _dee_aunion.to_spec
#define to_id    _dee_aunion.to_id
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
#define Dee_TYPE_OPERATOR_CUSTOM(id, invoke, flags) { { _Dee_OPINFO_INIT_AS_CUSTOM(id, flags, invoke) } }
#define Dee_TYPE_OPERATOR_FLAGS(id, flags)          { { _Dee_OPINFO_INIT_AS_CUSTOM(id, flags, NULL) } }
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
#define Dee_TP_FNAMEOBJECT      0x0400 /* `tp_name' actually points to the `s_str' member of a `string_object' that this type holds a reference to. */
#define Dee_TP_FDOCOBJECT       0x0800 /* `tp_doc' actually points to the `s_str' member of a `string_object' that this type holds a reference to. */
#define Dee_TP_FMAYREVIVE       0x1000 /* This type's `tp_dtor' may revive the object (return with the object's `ob_refcnt > 0')
                                        * Note that when reviving an object, `tp_dtor' must (in addition to an external reference)
                                        * gift the caller one further reference to the object that got revived (meaning that upon
                                        * return, `ob_refcnt >= 2', where at least 1 reference is caused by some global reference,
                                        * and 1 will be inherited by `DeeObject_Destroy()'). This is needed to prevent a potential
                                        * race condition where a GC-object isn't being tracked during destruction. */
#define Dee_TP_FGC              0x2000 /* Instance of this type can be harvested by the Garbage Collector. */
#define Dee_TP_FHEAP            0x4000 /* This type was allocated on the heap. */
#define Dee_TP_FVARIABLE        0x8000 /* Variable-length object type. (`tp_var' is used, rather than `tp_alloc') */
#define Dee_TP_FINTERHITABLE    (Dee_TP_FINTERRUPT | Dee_TP_FMAYREVIVE | Dee_TP_FGC | Dee_TP_FVARIABLE) \
                                       /* Set of special flags that are inherited by sub-classes. */

#define Dee_TF_NONE             0x00000000 /* No special features. */
#define Dee_TF_NONLOOPING       0x00000001 /* The object's tp_visit function can be skipped when searching for conventional reference loops.
                                            * This flag may be set when it is known that an object will only ever point to other objects
                                            * that either don't point to any objects, or are guarantied to never point back.
                                            * An example for where this flag should be used would be an object that only ever
                                            * holds references to `String' or `int' objects, but not to objects of its own type,
                                            * or any sort of container object capable of holding instances of the same type. */
#define Dee_TF_KW               0x00000002 /* Instances of this type can be used as keyword argument objects (s.a. `DeeType_IsKw()')
                                            * WARNING: If you set this flag, you must also implement support in `DeeKw_Get*' */
#define Dee_TF_TPVISIT          0x00000004 /* "tp_visit" is actually typed as "void (DCALL *tp_visit)(DeeTypeObject *tp_self, DeeObject *self, Dee_visit_t proc, void *arg)" */
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
#define TP_FINHERITCTOR     Dee_TP_FINHERITCTOR
#define TP_FABSTRACT        Dee_TP_FABSTRACT
#define TP_FNAMEOBJECT      Dee_TP_FNAMEOBJECT
#define TP_FDOCOBJECT       Dee_TP_FDOCOBJECT
#define TP_FMAYREVIVE       Dee_TP_FMAYREVIVE
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
	                                                  *       using `weakref_support_init' and `weakref_support_fini', during destruction. */
	uint32_t                            tp_features; /* Type sub-class specific features (Set of `TF_*'). */
	DREF DeeTypeObject                 *tp_base;     /* [0..1][const] Base class.
	                                                  * NOTE: When the `TP_FINHERITCTOR' flag is set, then this field must be non-NULL. */
	struct Dee_type_constructor         tp_init;     /* Constructor/destructor operators. */
	struct Dee_type_cast                tp_cast;     /* Type casting operators. */
	/* WARNING: When "Dee_TF_TPVISIT" is set, "tp_visit" is actually typed as:
	 * >> void (DCALL *tp_visit)(DeeTypeObject *tp_self, DeeObject *self, Dee_visit_t proc, void *arg); */
	NONNULL_T((1, 2)) void      (DCALL *tp_visit)(DeeObject *__restrict self, Dee_visit_t proc, void *arg); /* Visit all reachable, referenced (DREF) objected. */
	/* NOTE: Anything used by `DeeType_Inherit*' can't be made `Dee_tpconst' here! */
	struct Dee_type_gc Dee_tpconst     *tp_gc;       /* [0..1] GC related operators. */
	struct Dee_type_math               *tp_math;     /* [0..1][owned_if(tp_class != NULL)] Math related operators. */
	struct Dee_type_cmp                *tp_cmp;      /* [0..1][owned_if(tp_class != NULL)] Compare operators. */
	struct Dee_type_seq                *tp_seq;      /* [0..1][owned_if(tp_class != NULL)] Sequence operators. */
	WUNUSED_T NONNULL_T((1))
	DREF DeeObject             *(DCALL *tp_iter_next)(DeeObject *__restrict self);
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
#define DeeType_IsIterator(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_iter_next != NULL)
#define DeeType_IsTypeType(x)            DeeType_Extends(Dee_REQUIRES_OBJECT(DeeTypeObject const, x), &DeeType_Type)
#define DeeType_IsCustom(x)              (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FHEAP) /* Custom types are those not pre-defined, but created dynamically. */
#define DeeType_IsSuperConstructible(x)  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & Dee_TP_FINHERITCTOR)
#define DeeType_IsNoArgConstructible(x)  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_alloc.tp_ctor != NULL)
#define DeeType_IsVarArgConstructible(x) (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_alloc.tp_any_ctor != NULL || ((DeeTypeObject const *)(x))->tp_init.tp_alloc.tp_any_ctor_kw != NULL)
#define DeeType_IsConstructible(x)       (DeeType_IsSuperConstructible(x) || DeeType_IsNoArgConstructible(x) || DeeType_IsVarArgConstructible(x))
#define DeeType_IsCopyable(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_init.tp_alloc.tp_copy_ctor != NULL || ((DeeTypeObject const *)(x))->tp_init.tp_alloc.tp_deep_ctor != NULL)
#define DeeType_IsNamespace(x)           ((Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_flags & (Dee_TP_FFINAL | Dee_TP_FABSTRACT)) == (Dee_TP_FFINAL | Dee_TP_FABSTRACT) && (((DeeTypeObject const *)(x))->tp_features & Dee_TF_SINGLETON))
#define DeeType_Base(x)                  (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_base)
#define DeeType_GCPriority(x)            (Dee_REQUIRES_OBJECT(DeeTypeObject const, x)->tp_gc ? ((DeeTypeObject const *)(x))->tp_gc->tp_gcprio : Dee_GC_PRIORITY_LATE)
#define DeeObject_GCPriority(x)          DeeType_GCPriority(Dee_TYPE(x))
#define DeeObject_IsInterrupt(x)         DeeType_IsInterrupt(Dee_TYPE(x))

#ifdef CONFIG_BUILDING_DEEMON
/* Returns the "tp_serialize" operator for "tp". If possible, inherit from base class. */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t (DCALL DeeType_GetTpSerialize)(DeeTypeObject *__restrict self);
#endif /* CONFIG_BUILDING_DEEMON */

/* Returns the "instance-size" of a given object "self",
 * whilst trying to resolve known standard allocators.
 * The caller must ensure that `!DeeType_IsVariable(Dee_TYPE(self))'
 * @return: * : The instance size of "self"
 * @return: 0 : Instance size is unknown (non-standard allocator used) */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) size_t
(DCALL DeeType_GetInstanceSize)(DeeTypeObject const *__restrict self);


/* Helpers for allocating/freeing fixed-length (non-variable) type instances. */
#define DeeType_AllocInstance(tp_self)                                                      \
	(((tp_self)->tp_init.tp_alloc.tp_free)                                                  \
	 ? (DREF DeeObject *)(*(tp_self)->tp_init.tp_alloc.tp_alloc)()                          \
	 : ((tp_self)->tp_flags & TP_FGC)                                                       \
	   ? (DREF DeeObject *)DeeGCObject_Malloc((tp_self)->tp_init.tp_alloc.tp_instance_size) \
	   : (DREF DeeObject *)DeeObject_Malloc((tp_self)->tp_init.tp_alloc.tp_instance_size))
#define DeeType_FreeInstance(tp_self, obj)         \
	(((tp_self)->tp_init.tp_alloc.tp_free)         \
	 ? (*(tp_self)->tp_init.tp_alloc.tp_free)(obj) \
	 : ((tp_self)->tp_flags & TP_FGC)              \
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
INTDEF NONNULL((1)) bool DCALL DeeType_InheritConstructors(DeeTypeObject *__restrict self); /* tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor, tp_any_ctor_kw, tp_assign, tp_move_assign, tp_deepload */
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

#define DeeObject_TGenericGetAttr(tp_self, self, attr)                                      DeeObject_TGenericGetAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericGetAttrHash(tp_self, self, attr, hash)                            DeeObject_TGenericGetAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeObject_TGenericGetAttrString(tp_self, self, attr)                                DeeObject_TGenericGetAttrStringHash(tp_self, self, attr, Dee_HashStr(attr))
#define DeeObject_TGenericGetAttrStringLen(tp_self, self, attr, attrlen)                    DeeObject_TGenericGetAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeObject_TGenericCallAttr(tp_self, self, attr, argc, argv)                         DeeObject_TGenericCallAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeObject_TGenericCallAttrHash(tp_self, self, attr, hash, argc, argv)               DeeObject_TGenericCallAttrStringHash(tp_self, self, DeeString_STR(attr), hash, argc, argv)
#define DeeObject_TGenericCallAttrString(tp_self, self, attr, argc, argv)                   DeeObject_TGenericCallAttrStringHash(tp_self, self, attr, Dee_HashStr(attr), argc, argv)
#define DeeObject_TGenericCallAttrStringLen(tp_self, self, attr, attrlen, argc, argv)       DeeObject_TGenericCallAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeObject_TGenericCallAttrKw(tp_self, self, attr, argc, argv, kw)                   DeeObject_TGenericCallAttrStringHashKw(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeObject_TGenericCallAttrHashKw(tp_self, self, attr, hash, argc, argv, kw)         DeeObject_TGenericCallAttrStringHashKw(tp_self, self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeObject_TGenericCallAttrStringKw(tp_self, self, attr, argc, argv, kw)             DeeObject_TGenericCallAttrStringHashKw(tp_self, self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeObject_TGenericCallAttrStringLenKw(tp_self, self, attr, attrlen, argc, argv, kw) DeeObject_TGenericCallAttrStringLenHashKw(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeObject_VTGenericCallAttrf(tp_self, self, attr, format, args)                     DeeObject_VTGenericCallAttrStringHashf(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeObject_VTGenericCallAttrHashf(tp_self, self, attr, hash, format, args)           DeeObject_VTGenericCallAttrStringHashf(tp_self, self, DeeString_STR(attr), hash, format, args)
#define DeeObject_VTGenericCallAttrStringf(tp_self, self, attr, format, args)               DeeObject_VTGenericCallAttrStringHashf(tp_self, self, attr, Dee_HashStr(attr), format, args)
#define DeeObject_TGenericDelAttr(tp_self, self, attr)                                      DeeObject_TGenericDelAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericDelAttrHash(tp_self, self, attr, hash)                            DeeObject_TGenericDelAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeObject_TGenericDelAttrString(tp_self, self, attr)                                DeeObject_TGenericDelAttrStringHash(tp_self, self, attr, Dee_HashStr(attr))
#define DeeObject_TGenericDelAttrStringLen(tp_self, self, attr, attrlen)                    DeeObject_TGenericDelAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeObject_TGenericSetAttr(tp_self, self, attr, value)                               DeeObject_TGenericSetAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeObject_TGenericSetAttrHash(tp_self, self, attr, hash, value)                     DeeObject_TGenericSetAttrStringHash(tp_self, self, DeeString_STR(attr), hash, value)
#define DeeObject_TGenericSetAttrString(tp_self, self, attr, value)                         DeeObject_TGenericSetAttrStringHash(tp_self, self, attr, Dee_HashStr(attr), value)
#define DeeObject_TGenericSetAttrStringLen(tp_self, self, attr, attrlen, value)             DeeObject_TGenericSetAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeObject_TGenericHasAttr(tp_self, attr)                                            DeeObject_TGenericHasAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericHasAttrHash(tp_self, attr, hash)                                  DeeObject_TGenericHasAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeObject_TGenericHasAttrString(tp_self, attr)                                      DeeObject_TGenericHasAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeObject_TGenericHasAttrStringLen(tp_self, attr, attrlen)                          DeeObject_TGenericHasAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeObject_TGenericBoundAttr(tp_self, self, attr)                                    DeeObject_TGenericBoundAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeObject_TGenericBoundAttrHash(tp_self, self, attr, hash)                          DeeObject_TGenericBoundAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeObject_TGenericBoundAttrString(tp_self, self, attr)                              DeeObject_TGenericBoundAttrStringHash(tp_self, self, attr, Dee_HashStr(attr))
#define DeeObject_TGenericBoundAttrStringLen(tp_self, self, attr, attrlen)                  DeeObject_TGenericBoundAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))

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


DDATDEF DeeTypeObject DeeObject_Type; /* `Object' */
DDATDEF DeeTypeObject DeeType_Type;   /* `type(Object)' */
#define DeeType_Check(ob)      DeeObject_InstanceOf(ob, &DeeType_Type)
#define DeeType_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeType_Type)

/* Assert the typing of an object (raising an `Error.TypeError' if the type wasn't expected)
 * HINT: When `required_type' isn't a type-object, these functions throw an error!
 * @return: -1: The object doesn't match the required typing.
 * @return:  0: The object matches the required typing. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AssertType)(DeeObject *self, DeeTypeObject *required_type);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AssertTypeOrAbstract)(DeeObject *self, DeeTypeObject *required_type);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AssertImplements)(DeeObject *self, DeeTypeObject *required_type);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_AssertTypeExact)(DeeObject *self, DeeTypeObject *required_type);
/* Throw a TypeError stating that an instance of `required_type' was required, when `self' was given. */
DFUNDEF ATTR_COLD NONNULL((1, 2)) int (DCALL DeeObject_TypeAssertFailed)(DeeObject *self, DeeTypeObject *required_type);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3)) int (DCALL DeeObject_TypeAssertFailed2)(DeeObject *self, DeeTypeObject *required_type1, DeeTypeObject *required_type2);
DFUNDEF ATTR_COLD NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_TypeAssertFailed3)(DeeObject *self, DeeTypeObject *required_type1, DeeTypeObject *required_type2, DeeTypeObject *required_type3);
#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define DeeObject_TypeAssertFailed(self, required_type) \
	Dee_ASSUMED_VALUE(DeeObject_TypeAssertFailed(self, required_type), -1)
#define DeeObject_TypeAssertFailed2(self, required_type1, required_type2) \
	Dee_ASSUMED_VALUE(DeeObject_TypeAssertFailed2(self, required_type1, required_type2), -1)
#define DeeObject_TypeAssertFailed3(self, required_type1, required_type2, required_type3) \
	Dee_ASSUMED_VALUE(DeeObject_TypeAssertFailed3(self, required_type1, required_type2, required_type3), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */
#define DeeObject_AssertTypeOrNone(self, required_type)      (DeeNone_Check(self) ? 0 : DeeObject_AssertType(self, required_type))
#define DeeObject_AssertTypeExactOrNone(self, required_type) (DeeNone_CheckExact(self) ? 0 : DeeObject_AssertTypeExact(self, required_type))
#define DeeObject_AssertType(self, required_type)            (unlikely((DeeObject_AssertType)(Dee_AsObject(self), required_type)))
#define DeeObject_AssertTypeOrAbstract(self, required_type)  (unlikely((DeeObject_AssertTypeOrAbstract)(Dee_AsObject(self), required_type)))
#define DeeObject_AssertImplements(self, required_type)      (unlikely((DeeObject_AssertImplements)(Dee_AsObject(self), required_type)))
#ifdef __OPTIMIZE_SIZE__
#define DeeObject_AssertTypeExact(self, required_type) (unlikely((DeeObject_AssertTypeExact)(Dee_AsObject(self), required_type)))
#else /* __OPTIMIZE_SIZE__ */
#undef DeeObject_AssertTypeOrAbstract
#define DeeObject_AssertTypeOrAbstract(self, required_type) (DeeType_IsAbstract(required_type) ? 0 : DeeObject_AssertType(self, required_type))
#define DeeObject_AssertTypeExact(self, required_type)      (unlikely(Dee_TYPE(self) == (required_type) ? 0 : DeeObject_TypeAssertFailed((DeeObject *)(self), required_type)))
#endif /* !__OPTIMIZE_SIZE__ */


/* Returns the class of `self', automatically
 * dereferencing super-objects and other wrappers.
 * Beyond that, this function returns the same as `Dee_TYPE()' */
DFUNDEF WUNUSED ATTR_RETNONNULL NONNULL((1)) DeeTypeObject *DCALL
DeeObject_Class(DeeObject *__restrict self);

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

/* Object creation (constructor invocation). */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_NewDefault(DeeTypeObject *__restrict object_type);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1, 3)) DREF DeeObject *DCALL DeeObject_New(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1, 3)) DREF DeeObject *DCALL DeeObject_NewKw(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_NewTuple(DeeTypeObject *object_type, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_NewTupleKw(DeeTypeObject *object_type, DeeObject *args, DeeObject *kw);
DFUNDEF ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *DeeObject_NewPack(DeeTypeObject *object_type, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_VNewPack(DeeTypeObject *object_type, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeObject_Newf(DeeTypeObject *object_type, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_VNewf(DeeTypeObject *object_type, char const *__restrict format, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define DeeObject_NewTuple(object_type, args)       DeeObject_New(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_NewTupleKw(object_type, args, kw) DeeObject_NewKw(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !__OPTIMIZE_SIZE__ */

/* Object copy/assign operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Copy(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DeepCopy(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_CopyInherited(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DeepCopyInherited(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_InplaceDeepCopy(/*in|out*/ DREF DeeObject **__restrict p_self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_InplaceDeepCopyv(/*in|out*/ DREF DeeObject **__restrict object_vector, size_t object_count);
#define DeeObject_XInplaceDeepCopy(p_self) (!*(p_self) ? 0 : DeeObject_InplaceDeepCopy(p_self))
#ifndef CONFIG_NO_THREADS
/* A helper functions to acquire the proper read/write locks on a given
 * Dee_atomic_rwlock_t when accessing memory pointed to by the given `*p_self'.
 *
 * This is highly useful due to the fact that practically the only place where
 * `DeeObject_InplaceDeepCopy()' might ever be encountered, is in tp_deepload
 * operators, where using it on a raw pointer is not thread-safe, considering
 * the fact that the caller must probably be holding some kind of lock
 * (presumably an `Dee_atomic_rwlock_t'), which must be used in the following
 * order to safely replace the field with a deepcopy (the safe order of
 * operations that is then performed by this helper):
 * >> DREF DeeObject *temp, *copy;
 * >> Dee_atomic_rwlock_read(p_lock);
 * >> temp = *p_self;
 * >> #if IS_XDEEPCOPY
 * >> if (!temp) {
 * >>     Dee_atomic_rwlock_endread(p_lock);
 * >>     return 0;
 * >> }
 * >> #endif // IS_XDEEPCOPY
 * >> Dee_Incref(temp);
 * >> Dee_atomic_rwlock_endread(p_lock);
 * >> copy = DeeObject_DeepCopy(temp);
 * >> Dee_Decref(temp);
 * >> if unlikely(!copy)
 * >>    return -1;
 * >> Dee_atomic_rwlock_write(p_lock);
 * >> temp   = *p_self; // Inherit
 * >> *p_self = copy;   // Inherit
 * >> Dee_atomic_rwlock_endwrite(p_lock);
 * >> #if IS_XDEEPCOPY
 * >> Dee_XDecref(temp);
 * >> #else // IS_XDEEPCOPY
 * >> Dee_Decref(temp);
 * >> #endif // !IS_XDEEPCOPY
 * >> return 0;
 */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_InplaceDeepCopyWithLock(/*in|out*/ DREF DeeObject **__restrict p_self, Dee_atomic_lock_t *__restrict p_lock);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_XInplaceDeepCopyWithLock(/*in|out*/ DREF DeeObject **__restrict p_self, Dee_atomic_lock_t *__restrict p_lock);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_InplaceDeepCopyWithRWLock(/*in|out*/ DREF DeeObject **__restrict p_self, Dee_atomic_rwlock_t *__restrict p_lock);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_XInplaceDeepCopyWithRWLock(/*in|out*/ DREF DeeObject **__restrict p_self, Dee_atomic_rwlock_t *__restrict p_lock);
#else /* !CONFIG_NO_THREADS */
#define DeeObject_InplaceDeepCopyWithLock(p_self, p_lock)    DeeObject_InplaceDeepCopy(p_self)
#define DeeObject_XInplaceDeepCopyWithLock(p_self, p_lock)   DeeObject_XInplaceDeepCopy(p_self)
#define DeeObject_InplaceDeepCopyWithRWLock(p_self, p_lock)  DeeObject_InplaceDeepCopy(p_self)
#define DeeObject_XInplaceDeepCopyWithRWLock(p_self, p_lock) DeeObject_XInplaceDeepCopy(p_self)
#endif /* CONFIG_NO_THREADS */

/* @return: == 0: Success
 * @return: != 0: Error was thrown */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_Assign(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_MoveAssign(DeeObject *self, DeeObject *other);

/* Object conversion operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Str(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_Repr(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_StrInherited(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_ReprInherited(/*inherit(always)*/ DREF DeeObject *__restrict self);

/* @return:  > 0: Object is true
 * @return: == 0: Object is false
 * @return: <  0: Error was thrown */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_Bool(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_BoolInherited(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeObject_BoolInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_BoolOb(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_BoolObInherited(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_BoolObInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);

/* Object call operator invocation. */
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL DeeObject_Call(DeeObject *self, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL DeeObject_CallKw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_ThisCall(DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_ThisCallKw(DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DeeObject_CallPack(DeeObject *self, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_VCallPack(DeeObject *self, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeObject_Callf(DeeObject *self, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DeeObject_ThisCallf(DeeObject *self, DeeObject *thisarg, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_VCallf(DeeObject *self, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_VThisCallf(DeeObject *self, DeeObject *thisarg, char const *__restrict format, va_list args);

/* Same as the regular call functions, however also include special
 * optimizations to re-use `args' as the varargs tuple in calls to
 * pure user-code varargs functions:
 * >> function foo(args...) {
 * >>     import Object from deemon;
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
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallTuple)(DeeObject *self, /*Tuple*/ DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallTupleKw)(DeeObject *self, /*Tuple*/ DeeObject *args, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_ThisCallTuple)(DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_ThisCallTupleKw)(DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args, DeeObject *kw);
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#define DeeObject_CallTuple(self, args)                    DeeObject_Call(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallTupleKw(self, args, kw)              DeeObject_CallKw(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_ThisCallTuple(self, thisarg, args)       DeeObject_ThisCall(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_ThisCallTupleKw(self, thisarg, args, kw) DeeObject_ThisCallKw(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */

/* Same as the other functions above, but *always* inherits references to "self" (iow: the function being called) */
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL DeeObject_CallInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL DeeObject_CallKwInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_ThisCallInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_ThisCallKwInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DeeObject_CallInheritedf(/*inherit(always)*/ DREF DeeObject *self, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DeeObject_ThisCallInheritedf(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_VCallInheritedf(/*inherit(always)*/ DREF DeeObject *self, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_VThisCallInheritedf(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallTupleInherited)(/*inherit(always)*/ DREF DeeObject *self, /*Tuple*/ DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallTupleKwInherited)(/*inherit(always)*/ DREF DeeObject *self, /*Tuple*/ DeeObject *args, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_ThisCallTupleInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_ThisCallTupleKwInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args, DeeObject *kw);
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#define DeeObject_CallTupleInherited(/*inherit(always)*/ self, args)                    DeeObject_CallInherited(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallTupleKwInherited(/*inherit(always)*/ self, args, kw)              DeeObject_CallKwInherited(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_ThisCallTupleInherited(/*inherit(always)*/ self, thisarg, args)       DeeObject_ThisCallInherited(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_ThisCallTupleKwInherited(/*inherit(always)*/ self, thisarg, args, kw) DeeObject_ThisCallKwInherited(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */


/* Generate and return the hash of a given object. */
DFUNDEF WUNUSED /*ATTR_PURE*/ NONNULL((1)) Dee_hash_t (DCALL DeeObject_Hash)(DeeObject *__restrict self);
DFUNDEF WUNUSED /*ATTR_PURE*/ NONNULL((1)) Dee_hash_t (DCALL DeeObject_HashInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) Dee_hash_t (DCALL DeeObject_Hashv)(DeeObject *const *__restrict object_vector, size_t object_count);
DFUNDEF WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) Dee_hash_t (DCALL DeeObject_XHashv)(DeeObject *const *__restrict object_vector, size_t object_count);
#define DeeObject_Hash(self)          DeeObject_Hash(Dee_AsObject(self))
#define DeeObject_HashInherited(self) DeeObject_HashInherited(Dee_AsObject(self))

/* GC operator invocation. */
DFUNDEF NONNULL((1, 2)) void (DCALL DeeObject_Visit)(DeeObject *__restrict self, Dee_visit_t proc, void *arg);
DFUNDEF NONNULL((1)) void (DCALL DeeObject_Clear)(DeeObject *__restrict self);
DFUNDEF NONNULL((1)) void (DCALL DeeObject_PClear)(DeeObject *__restrict self, unsigned int gc_priority);

/* Integral value lookup operators.
 * @return: Dee_INT_SIGNED:   The value stored in `result' must be interpreted as signed.
 * @return: Dee_INT_UNSIGNED: The value stored in `result' must be interpreted as unsigned.
 * @return: Dee_INT_ERROR:    An error occurred. */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_Get8Bit)(DeeObject *__restrict self, int8_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_Get16Bit)(DeeObject *__restrict self, int16_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_Get32Bit)(DeeObject *__restrict self, int32_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_Get64Bit)(DeeObject *__restrict self, int64_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_Get128Bit)(DeeObject *__restrict self, Dee_int128_t *__restrict result);

/* Integral/Floating-point conversion operator invocation. */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsInt8)(DeeObject *__restrict self, int8_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsInt16)(DeeObject *__restrict self, int16_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsInt32)(DeeObject *__restrict self, int32_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsInt64)(DeeObject *__restrict self, int64_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsInt128)(DeeObject *__restrict self, Dee_int128_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt8)(DeeObject *__restrict self, uint8_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt16)(DeeObject *__restrict self, uint16_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt32)(DeeObject *__restrict self, uint32_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt64)(DeeObject *__restrict self, uint64_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt128)(DeeObject *__restrict self, Dee_uint128_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsDouble)(DeeObject *__restrict self, double *__restrict result);

/* Same as the functions above, but these also accept `-1' as an alias for `UINTn_MAX' */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt8M1)(DeeObject *__restrict self, uint8_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt16M1)(DeeObject *__restrict self, uint16_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt32M1)(DeeObject *__restrict self, uint32_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt64M1)(DeeObject *__restrict self, uint64_t *__restrict result);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeObject_AsUInt128M1)(DeeObject *__restrict self, Dee_uint128_t *__restrict result);

/* All of these return (T)-1 on error. When the object's actual value is `(T)-1', throw `IntegerOverflow' */
DFUNDEF WUNUSED NONNULL((1)) uint8_t (DCALL DeeObject_AsUInt8Direct)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint16_t (DCALL DeeObject_AsUInt16Direct)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint32_t (DCALL DeeObject_AsUInt32Direct)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint64_t (DCALL DeeObject_AsUInt64Direct)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint8_t (DCALL DeeObject_AsUInt8DirectInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint16_t (DCALL DeeObject_AsUInt16DirectInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint32_t (DCALL DeeObject_AsUInt32DirectInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) uint64_t (DCALL DeeObject_AsUInt64DirectInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);

/* Cast-to-integer conversion operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_Int)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IntInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IntInheritedOnSuccess)(/*inherit(on_success)*/ DREF DeeObject *__restrict self);

#define DEE_PRIVATE_OBJECT_AS_INT_1(self, result)     DeeObject_AsInt8(self, (int8_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_2(self, result)     DeeObject_AsInt16(self, (int16_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_4(self, result)     DeeObject_AsInt32(self, (int32_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_8(self, result)     DeeObject_AsInt64(self, (int64_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT_16(self, result)    DeeObject_AsInt128(self, (Dee_int128_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_INT(size)               DEE_PRIVATE_OBJECT_AS_INT_##size
#define DEE_PRIVATE_OBJECT_AS_UINT_1(self, result)    DeeObject_AsUInt8(self, (uint8_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_2(self, result)    DeeObject_AsUInt16(self, (uint16_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_4(self, result)    DeeObject_AsUInt32(self, (uint32_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_8(self, result)    DeeObject_AsUInt64(self, (uint64_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT_16(self, result)   DeeObject_AsUInt128(self, (Dee_uint128_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINT(size)              DEE_PRIVATE_OBJECT_AS_UINT_##size
#define DEE_PRIVATE_OBJECT_AS_UINTM1_1(self, result)  DeeObject_AsUInt8M1(self, (uint8_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINTM1_2(self, result)  DeeObject_AsUInt16M1(self, (uint16_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINTM1_4(self, result)  DeeObject_AsUInt32M1(self, (uint32_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINTM1_8(self, result)  DeeObject_AsUInt64M1(self, (uint64_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINTM1_16(self, result) DeeObject_AsUInt128M1(self, (Dee_uint128_t *)(result))
#define DEE_PRIVATE_OBJECT_AS_UINTM1(size)            DEE_PRIVATE_OBJECT_AS_UINTM1_##size

#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_1(self) DeeObject_AsUInt8Direct(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_2(self) DeeObject_AsUInt16Direct(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_4(self) DeeObject_AsUInt32Direct(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_8(self) DeeObject_AsUInt64Direct(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT(size)   DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_##size

#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_1(self) DeeObject_AsUInt8DirectInherited(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_2(self) DeeObject_AsUInt16DirectInherited(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_4(self) DeeObject_AsUInt32DirectInherited(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_8(self) DeeObject_AsUInt64DirectInherited(self)
#define DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED(size)   DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_##size

#define DeeObject_MapAsXInt(size)                 DEE_PRIVATE_OBJECT_AS_INT(size)
#define DeeObject_MapAsXUInt(size)                DEE_PRIVATE_OBJECT_AS_UINT(size)
#define DeeObject_MapAsXUIntM1(size)              DEE_PRIVATE_OBJECT_AS_UINTM1(size)
#define DeeObject_MapAsXUIntDirect(size)          DEE_PRIVATE_OBJECT_AS_DIRECT_UINT(size)
#define DeeObject_MapAsXUIntDirectInherited(size) DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED(size)

/* Helper macros for converting objects to integers */
#define DeeObject_AsXInt(size, self, result)         DEE_PRIVATE_OBJECT_AS_INT(size)(self, result)
#define DeeObject_AsXUInt(size, self, result)        DEE_PRIVATE_OBJECT_AS_UINT(size)(self, result)
#define DeeObject_AsXUIntM1(size, self, result)      DEE_PRIVATE_OBJECT_AS_UINTM1(size)(self, result)
#define DeeObject_AsXUIntDirect(size, self)          DEE_PRIVATE_OBJECT_AS_DIRECT_UINT(size)(self)
#define DeeObject_AsXUIntDirectInherited(size, self) DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED(size)(self)
#ifdef __CHAR_UNSIGNED__
#define DeeObject_AsChar(self, result)    DeeObject_AsXUInt(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(char *, result))
#else /* __CHAR_UNSIGNED__ */
#define DeeObject_AsChar(self, result)    DeeObject_AsXInt(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(char *, result))
#endif /* !__CHAR_UNSIGNED__ */
#define DeeObject_AsSChar(self, result)          DeeObject_AsXInt(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(signed char *, result))
#define DeeObject_AsUChar(self, result)          DeeObject_AsXUInt(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(unsigned char *, result))
#define DeeObject_AsUCharM1(self, result)        DeeObject_AsXUIntM1(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(unsigned char *, result))
#define DeeObject_AsUCharDirect(self)            DeeObject_AsXUIntDirect(__SIZEOF_CHAR__, self)
#define DeeObject_AsUCharDirectInherited(self)   DeeObject_AsXUIntDirectInherited(__SIZEOF_CHAR__, self)
#define DeeObject_AsShort(self, result)          DeeObject_AsXInt(__SIZEOF_SHORT__, self, Dee_REQUIRES_TYPE(short *, result))
#define DeeObject_AsUShort(self, result)         DeeObject_AsXUInt(__SIZEOF_SHORT__, self, Dee_REQUIRES_TYPE(unsigned short *, result))
#define DeeObject_AsUShortM1(self, result)       DeeObject_AsXUIntM1(__SIZEOF_SHORT__, self, Dee_REQUIRES_TYPE(unsigned short *, result))
#define DeeObject_AsUShortDirect(self)           DeeObject_AsXUIntDirect(__SIZEOF_SHORT__, self)
#define DeeObject_AsUShortDirectInherited(self)  DeeObject_AsXUIntDirectInherited(__SIZEOF_SHORT__, self)
#define DeeObject_AsInt(self, result)            DeeObject_AsXInt(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(int *, result))
#define DeeObject_AsUInt(self, result)           DeeObject_AsXUInt(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(unsigned int *, result))
#define DeeObject_AsUIntM1(self, result)         DeeObject_AsXUIntM1(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(unsigned int *, result))
#define DeeObject_AsUIntDirect(self)             DeeObject_AsXUIntDirect(__SIZEOF_INT__, self)
#define DeeObject_AsUIntDirectInherited(self)    DeeObject_AsXUIntDirectInherited(__SIZEOF_INT__, self)
#define DeeObject_AsLong(self, result)           DeeObject_AsXInt(__SIZEOF_LONG__, self, Dee_REQUIRES_TYPE(long *, result))
#define DeeObject_AsULong(self, result)          DeeObject_AsXUInt(__SIZEOF_LONG__, self, Dee_REQUIRES_TYPE(unsigned long *, result))
#define DeeObject_AsULongM1(self, result)        DeeObject_AsXUIntM1(__SIZEOF_LONG__, self, Dee_REQUIRES_TYPE(unsigned long *, result))
#define DeeObject_AsULongDirect(self)            DeeObject_AsXUIntDirect(__SIZEOF_LONG__, self)
#define DeeObject_AsULongDirectInherited(self)   DeeObject_AsXUIntDirectInherited(__SIZEOF_LONG__, self)
#ifdef __COMPILER_HAVE_LONGLONG
#define DeeObject_AsLLong(self, result)          DeeObject_AsXInt(__SIZEOF_LONG_LONG__, self, Dee_REQUIRES_TYPE(__LONGLONG *, result))
#define DeeObject_AsULLong(self, result)         DeeObject_AsXUInt(__SIZEOF_LONG_LONG__, self, Dee_REQUIRES_TYPE(__ULONGLONG *, result))
#define DeeObject_AsULLongM1(self, result)       DeeObject_AsXUIntM1(__SIZEOF_LONG_LONG__, self, Dee_REQUIRES_TYPE(__ULONGLONG *, result))
#define DeeObject_AsULLongDirect(self)           DeeObject_AsXUIntDirect(__SIZEOF_LONG_LONG__, self)
#define DeeObject_AsULLongDirectInherited(self)  DeeObject_AsXUIntDirectInherited(__SIZEOF_LONG_LONG__, self)
#endif /* __COMPILER_HAVE_LONGLONG */
#define DeeObject_AsSize(self, result)           DeeObject_AsXUInt(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(size_t *, result))
#define DeeObject_AsSizeM1(self, result)         DeeObject_AsXUIntM1(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(size_t *, result))
#define DeeObject_AsSizeDirect(self)             DeeObject_AsXUIntDirect(__SIZEOF_SIZE_T__, self)
#define DeeObject_AsSizeDirectInherited(self)    DeeObject_AsXUIntDirectInherited(__SIZEOF_SIZE_T__, self)
#define DeeObject_AsSSize(self, result)          DeeObject_AsXInt(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(Dee_ssize_t *, result))
#define DeeObject_AsPtrdiff(self, result)        DeeObject_AsXInt(__SIZEOF_PTRDIFF_T__, self, Dee_REQUIRES_TYPE(ptrdiff_t *, result))
#define DeeObject_AsIntptr(self, result)         DeeObject_AsXInt(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(intptr_t *, result))
#define DeeObject_AsUIntptr(self, result)        DeeObject_AsXUInt(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(uintptr_t *, result))
#define DeeObject_AsUIntptrM1(self, result)      DeeObject_AsXUIntM1(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(uintptr_t *, result))
#define DeeObject_AsUIntptrDirect(self)          DeeObject_AsXUIntDirect(__SIZEOF_POINTER__, self)
#define DeeObject_AsUIntptrDirectInherited(self) DeeObject_AsXUIntDirectInherited(__SIZEOF_POINTER__, self)
#define DeeObject_AsSByte(self, result)          DEE_PRIVATE_OBJECT_AS_INT_1(self, Dee_REQUIRES_TYPE(__SBYTE_TYPE__ *, result))
#define DeeObject_AsByte(self, result)           DEE_PRIVATE_OBJECT_AS_UINT_1(self, Dee_REQUIRES_TYPE(__BYTE_TYPE__ *, result))
#define DeeObject_AsByteM1(self, result)         DEE_PRIVATE_OBJECT_AS_UINTM1_1(self, Dee_REQUIRES_TYPE(__BYTE_TYPE__ *, result))
#define DeeObject_AsByteDirect(self)             DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_1(self)
#define DeeObject_AsByteDirectInherited(self)    DEE_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_1(self)

/* Helper wrapper around `DeeObject_Bool()' that writes
 * the value to `*result' rather than use the return value. */
LOCAL WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AsBool)(DeeObject *__restrict self, bool *__restrict result) {
	int value = DeeObject_Bool(self);
	*result = value != 0;
	return unlikely(value < 0) ? value : 0;
}


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
 * NOTE: For the duration of the call, `*p_self' must not be changed by outside sources.
 *       Because of this, pointers to external, global, or static variables must be passed
 *       indirectly, though local or stack variables can be passed directly (as they are
 *       private to their stack-frame and cannot be changed through outside interference,
 *       aside of debuggers which know to look out for manipulating operands of inplace
 *       instructions)
 * >> Because these functions will inherit a reference to `IN(*p_self)' upon success, it is
 *    possible for the implementation to check for otherwise immutable objects to be modified
 *    in-line (when `DeeObject_IsShared(IN(*p_self))' is false), thus allowing an invocation
 *    such as `DeeObject_Inc(&my_int)' to potentially be completed without having to allocate
 *    a new integer object (though only in case `my_int' isn't being shared, and incrementing
 *    wouldn't overflow within the available number of digits) */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_Inc)(DREF DeeObject **__restrict p_self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_Dec)(DREF DeeObject **__restrict p_self);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceAdd)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceSub)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceMul)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceDiv)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceMod)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceShl)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceShr)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceAnd)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceOr)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplaceXor)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_InplacePow)(DREF DeeObject **__restrict p_self, DeeObject *some_object);

/* Math operations with C (aka. host) integer operand. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AddInt8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SubInt8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AddUInt32)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AddUInt64)(DeeObject *__restrict self, uint64_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SubUInt32)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SubUInt64)(DeeObject *__restrict self, uint64_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_MulInt8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_DivInt8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_ModInt8)(DeeObject *__restrict self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_ShlUInt8)(DeeObject *__restrict self, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_ShrUInt8)(DeeObject *__restrict self, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_AndUInt32)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_OrUInt32)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_XorUInt32)(DeeObject *__restrict self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceAddInt8)(DREF DeeObject **__restrict p_self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceSubInt8)(DREF DeeObject **__restrict p_self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceAddUInt32)(DREF DeeObject **__restrict p_self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceSubUInt32)(DREF DeeObject **__restrict p_self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceMulInt8)(DREF DeeObject **__restrict p_self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceDivInt8)(DREF DeeObject **__restrict p_self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceModInt8)(DREF DeeObject **__restrict p_self, int8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceShlUInt8)(DREF DeeObject **__restrict p_self, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceShrUInt8)(DREF DeeObject **__restrict p_self, uint8_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceAndUInt32)(DREF DeeObject **__restrict p_self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceOrUInt32)(DREF DeeObject **__restrict p_self, uint32_t val);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_InplaceXorUInt32)(DREF DeeObject **__restrict p_self, uint32_t val);
#if __SIZEOF_SIZE_T__ > 4
#define DeeObject_AddSize(self, val) DeeObject_AddUInt64(self, (uint64_t)(val))
#define DeeObject_SubSize(self, val) DeeObject_SubUInt64(self, (uint64_t)(val))
#else /* __SIZEOF_SIZE_T__ > 4 */
#define DeeObject_AddSize(self, val) DeeObject_AddUInt32(self, (uint32_t)(val))
#define DeeObject_SubSize(self, val) DeeObject_SubUInt32(self, (uint32_t)(val))
#endif /* __SIZEOF_SIZE_T__ <= 4 */


/* Comparison operator invocation. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CmpEq)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CmpNe)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CmpLo)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CmpLe)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CmpGr)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CmpGe)(DeeObject *self, DeeObject *some_object);

/* Same as above, but automatically cast the returned object using `DeeObject_Bool()' */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CmpEqAsBool)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CmpNeAsBool)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CmpLoAsBool)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CmpLeAsBool)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CmpGrAsBool)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_CmpGeAsBool)(DeeObject *self, DeeObject *some_object);


/* @return: == -1: `lhs < rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs > rhs'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_Compare)(DeeObject *lhs, DeeObject *rhs);

/* @return: == -1: `lhs != rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs != rhs'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_CompareEq)(DeeObject *lhs, DeeObject *rhs);

/* Same as `DeeObject_CompareEq()', but automatically handles errors
 * that usually indicate that "lhs" and "rhs" cannot be compared by returning
 * either `-1' or `1' instead. The following errors get handled (so-long as
 * the effective `tp_trycompare_eq' callback doesn't end up throwing these):
 * - `Error.RuntimeError.NotImplemented' (`DeeError_NotImplemented'; Should indicate compare-not-implemented)
 * - `Error.TypeError'                   (`DeeError_TypeError';      Should indicate unsupported type combination)
 * - `Error.ValueError'                  (`DeeError_ValueError';     Should indicate unsupported instance combination)
 * @return: == -1: `lhs != rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs != rhs'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TryCompareEq)(DeeObject *lhs, DeeObject *rhs);

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed < key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed > key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_CompareKey)(DeeObject *lhs_keyed,
                             DeeObject *rhs, DeeObject *key);

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed != key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed != key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_CompareKeyEq)(DeeObject *lhs_keyed,
                               DeeObject *rhs, DeeObject *key);

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed != key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed != key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TryCompareKeyEq)(DeeObject *lhs_keyed,
                                  DeeObject *rhs, DeeObject *key);


/* Sequence operator invocation. */
DFUNDEF WUNUSED NONNULL((1)) size_t (DCALL DeeObject_Size)(DeeObject *__restrict self); /* @return: (size_t)-1: Error */
DFUNDEF WUNUSED NONNULL((1)) size_t (DCALL DeeObject_SizeFast)(DeeObject *__restrict self); /* @return: (size_t)-1: Fast size cannot be determined */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SizeOb)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_SizeObInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_ContainsAsBool)(DeeObject *self, DeeObject *some_object); /* @return: 1: found */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_Contains)(DeeObject *self, DeeObject *some_object);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_GetItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetItemStringHash)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetItemStringLenHash)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TryGetItem)(DeeObject *self, DeeObject *key); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_TryGetItemIndex)(DeeObject *self, size_t index); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TryGetItemStringHash)(DeeObject *self, char const *__restrict key, Dee_hash_t hash); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_TryGetItemStringLenHash)(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash); /* @return: ITER_DONE: No such key/index */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_DelItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelItemStringHash)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelItemStringLenHash)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_SetItem)(DeeObject *self, DeeObject *index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 3)) int (DCALL DeeObject_SetItemIndex)(DeeObject *self, size_t index, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_SetItemStringHash)(DeeObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeObject_SetItemStringLenHash)(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_GetRange)(DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *(DCALL DeeObject_GetRangeBeginIndex)(DeeObject *self, Dee_ssize_t begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetRangeEndIndex)(DeeObject *self, DeeObject *begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_GetRangeIndex)(DeeObject *__restrict self, Dee_ssize_t begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_GetRangeIndexN)(DeeObject *__restrict self, Dee_ssize_t begin);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_DelRange)(DeeObject *self, DeeObject *begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 3)) int (DCALL DeeObject_DelRangeBeginIndex)(DeeObject *self, Dee_ssize_t begin, DeeObject *end);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelRangeEndIndex)(DeeObject *self, DeeObject *begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_DelRangeIndex)(DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_DelRangeIndexN)(DeeObject *self, Dee_ssize_t begin);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeObject_SetRange)(DeeObject *self, DeeObject *begin, DeeObject *end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 3, 4)) int (DCALL DeeObject_SetRangeBeginIndex)(DeeObject *self, Dee_ssize_t begin, DeeObject *end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_SetRangeEndIndex)(DeeObject *self, DeeObject *begin, Dee_ssize_t end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL DeeObject_SetRangeIndex)(DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end, DeeObject *values);
DFUNDEF WUNUSED NONNULL((1, 3)) int (DCALL DeeObject_SetRangeIndexN)(DeeObject *self, Dee_ssize_t begin, DeeObject *values);
#define DeeObject_GetItemString(self, key)                    DeeObject_GetItemStringHash(self, key, Dee_HashStr(key))
#define DeeObject_GetItemStringLen(self, key, keylen)         DeeObject_GetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeObject_TryGetItemString(self, key)                 DeeObject_TryGetItemStringHash(self, key, Dee_HashStr(key))
#define DeeObject_TryGetItemStringLen(self, key, keylen)      DeeObject_TryGetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeObject_DelItemString(self, key)                    DeeObject_DelItemStringHash(self, key, Dee_HashStr(key))
#define DeeObject_DelItemStringLen(self, key, keylen)         DeeObject_DelItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeObject_SetItemString(self, key, value)             DeeObject_SetItemStringHash(self, key, Dee_HashStr(key), value)
#define DeeObject_SetItemStringLen(self, key, keylen, value)  DeeObject_SetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen), value)

/* Check if a given item exists (`deemon.hasitem(self, index)')
 * @return: == 0: Item doesn't exist
 * @return: > 0:  Item exists
 * @return: < 0:  An error was thrown
 * HINT: Use `Dee_HAS_IS*' to test return value */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_HasItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasItemStringHash)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasItemStringLenHash)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
#define DeeObject_HasItemString(self, key)            DeeObject_HasItemStringHash(self, key, Dee_HashStr(key))
#define DeeObject_HasItemStringLen(self, key, keylen) DeeObject_HasItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))

/* Check if a given item is bound (`self[index] is bound' / `deemon.bounditem(self, index)')
 * @return: Dee_BOUND_YES:     Item is bound.
 * @return: Dee_BOUND_NO:      Item isn't bound. (`UnboundItem' was caught internally)
 * @return: Dee_BOUND_MISSING: Item doesn't exist (`KeyError' was caught).
 * @return: Dee_BOUND_ERR:     An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundItem)(DeeObject *self, DeeObject *index);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_BoundItemIndex)(DeeObject *__restrict self, size_t index);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundItemStringHash)(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundItemStringLenHash)(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
#define DeeObject_BoundItemString(self, key)            DeeObject_BoundItemStringHash(self, key, Dee_HashStr(key))
#define DeeObject_BoundItemStringLen(self, key, keylen) DeeObject_BoundItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))

/* NOTE: The `argv' vector itself isn't inherited; only its elements are! */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_ConcatInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *other);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *(DCALL DeeObject_ExtendInherited)(/*inherit(always)*/ DREF DeeObject *self, size_t argc, /*inherit(always)*/ DREF DeeObject *const *argv);

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
 * Also note that `Object' implements a `__format__' function that calls forward
 * to the `str' operator and allows for alignment of the produced string, as well
 * as the fact that if accessing a sub-classes __format__ attribute causes an
 * AttributeError, or NotImplemented error to be thrown, the object will be formatted
 * using `Object.__format__' as well:
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
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_Iter)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IterNext)(DeeObject *__restrict self);

/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2).first[key]/last[value]'
 * In the case of mapping iterators, these can be used to iterate only the
 * key/value part of the map, without needing to construct a temporary tuple
 * holding both values (as needs to be done by `DeeObject_IterNext'). */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_IterNextPair)(DeeObject *__restrict self, /*out*/ DREF DeeObject *key_and_value[2]);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IterNextKey)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeObject_IterNextValue)(DeeObject *__restrict self);

/* Advance an iterator by "step" items.
 * @return: step:       Success.
 * @return: < step:     Success, but advancing was stopped prematurely because ITER_DONE
 *                      was encountered. Return value is the # of successfully skipped
 *                      entries before "ITER_DONE" was encountered.
 * @return: (size_t)-1: Error. */
DFUNDEF WUNUSED NONNULL((1)) size_t (DCALL DeeObject_IterAdvance)(DeeObject *__restrict self, size_t step);

/* Invoke `proc' for each element of a general-purpose sequence.
 * When `*proc' returns < 0, that value is propagated.
 * Otherwise, return the sum of all calls to it.
 * NOTE: This function does some special optimizations for known sequence types.
 * @return: -1: An error occurred during iteration (or potentially inside of `*proc') */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t /* TODO: Refactor more code to use this instead of `DeeObject_Iter()' */
(DCALL DeeObject_Foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);

/* Same as `DeeObject_Foreach()', but meant for enumeration of mapping key/value pairs. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t /* TODO: Refactor more code to use this instead of `DeeSeq_Unpack()' */
(DCALL DeeObject_ForeachPair)(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);

/* >> DeeObject_GetAttr() -- <self>.<attr>;
 * Retrieve a named attribute of an object
 * @return: * :   The value of the attribute `attr'
 * @return: NULL: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttr)(DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttrString)(DeeObject *__restrict self, char const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_GetAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
#define DeeObject_GetAttrStringLen(self, attr, attrlen) DeeObject_GetAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* >> DeeObject_DelAttr() -- del <self>.<attr>;
 * Delete an attribute, either removing it from existence, marking it as
 * unbound, or doing something else entirely (such as assigning it some
 * default value); the exact behavior here depends on the object `self'.
 * Note that generally, attempting to delete an attribute that is already
 * unbound is considered as no-op (NO error should be thrown).
 * @return: 0 : Success.
 * @return: -1: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttr)(DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttrString)(DeeObject *__restrict self, char const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_DelAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
#define DeeObject_DelAttrStringLen(self, attr, attrlen) DeeObject_DelAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* >> DeeObject_SetAttr() -- <self>.<attr> = <value>;
 * Assign a given `value' to a named attribute `attr' of `self'.
 * @return: 0 : Success.
 * @return: -1: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_SetAttr)(DeeObject *self, /*String*/ DeeObject *attr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeObject_SetAttrString)(DeeObject *self, char const *__restrict attr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeObject_SetAttrStringHash)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeObject_SetAttrStringLenHash)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
#define DeeObject_SetAttrStringLen(self, attr, attrlen, value) DeeObject_SetAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)

/* >> DeeObject_CallAttr() -- <self>.<attr>(<args>...);
 * Convenience (and performance) wrapper for `DeeObject_Call(DeeObject_GetAttr(self, attr), args)'.
 * Even though this function uses its own dedicated code paths, its behavior must always be equal
 * to the 2-step process of first retrieving an attribute, and then calling its current value.
 * The dictated code paths exist in order to skip the need to create a temporary (wrapper) object,
 * as would be necessary for pretty much all built-in object attributes (s.a. `DeeObjMethod_Type')
 * @return: * :   The return value of the `DeeObject_Call()' being applied to the attribute.
 * @return: NULL: An error was thrown. */
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttr)(DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrString)(DeeObject *self, char const *__restrict attr, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringHash)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED ATTR_INS(6, 5) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringLenHash)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv);
#define DeeObject_CallAttrStringLen(self, attr, attrlen, argc, argv) DeeObject_CallAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrKw)(DeeObject *self, /*String*/ DeeObject *attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringKw)(DeeObject *self, char const *__restrict attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringHashKw)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED ATTR_INS(6, 5) NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_CallAttrStringLenHashKw)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeObject_CallAttrStringLenKw(self, attr, attrlen, argc, argv, kw) DeeObject_CallAttrStringLenHashKw(self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrPack)(DeeObject *self, /*String*/ DeeObject *attr, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrStringPack)(DeeObject *self, char const *__restrict attr, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrStringHashPack)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DeeObject_CallAttrStringLenHashPack)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, ...);
#define DeeObject_CallAttrStringLenPack(self, attr, attrlen, ...) DeeObject_CallAttrStringLenHashPack(self, attr, attrlen, Dee_HashPtr(attr, attrlen), __VA_ARGS__)
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrPack)(DeeObject *self, /*String*/ DeeObject *attr, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringPack)(DeeObject *self, char const *__restrict attr, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, size_t argc, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringLenHashPack)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, size_t argc, va_list args);
#define DeeObject_VCallAttrStringLenPack(self, attr, attrlen, argc, args) DeeObject_VCallAttrStringLenHashPack(self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, args)
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DeeObject_CallAttrf)(DeeObject *self, /*String*/ DeeObject *attr, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DeeObject_CallAttrStringf)(DeeObject *self, char const *__restrict attr, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DeeObject_CallAttrStringHashf)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DeeObject_CallAttrStringLenHashf)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, char const *__restrict format, ...);
#define DeeObject_CallAttrStringLenf(self, attr, attrlen, ...) DeeObject_CallAttrStringLenHashf(self, attr, attrlen, Dee_HashPtr(attr, attrlen), __VA_ARGS__)
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_VCallAttrf)(DeeObject *self, /*String*/ DeeObject *attr, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringf)(DeeObject *self, char const *__restrict attr, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringHashf)(DeeObject *self, char const *__restrict attr, Dee_hash_t hash, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeObject_VCallAttrStringLenHashf)(DeeObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, char const *__restrict format, va_list args);
#define DeeObject_VCallAttrStringLenf(self, attr, attrlen, format, args) DeeObject_VCallAttrStringLenHashf(self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_CallAttrTuple)(DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeObject_CallAttrTupleKw)(DeeObject *self, /*String*/ DeeObject *attr, DeeObject *args, DeeObject *kw);
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#define DeeObject_CallAttrTuple(self, attr, args)       DeeObject_CallAttr(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrTupleKw(self, attr, args, kw) DeeObject_CallAttrKw(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */
#define DeeObject_CallAttrStringTuple(self, attr, args)                             DeeObject_CallAttrString(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringHashTuple(self, attr, hash, args)                   DeeObject_CallAttrStringHash(self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringLenTuple(self, attr, attrlen, args)                 DeeObject_CallAttrStringLen(self, attr, attrlen, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringLenHashTuple(self, attr, attrlen, hash, args)       DeeObject_CallAttrStringLenHash(self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeObject_CallAttrStringTupleKw(self, attr, args, kw)                       DeeObject_CallAttrStringKw(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_CallAttrStringHashTupleKw(self, attr, hash, args, kw)             DeeObject_CallAttrStringHashKw(self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_CallAttrStringLenTupleKw(self, attr, attrlen, args, kw)           DeeObject_CallAttrStringLenKw(self, attr, attrlen, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeObject_CallAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw) DeeObject_CallAttrStringLenHashKw(self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)

/* >> DeeObject_HasAttr() -- deemon.hasattr(<self>, <attr>);
 * Check if `self' has an attribute `attr'. Same as the builtin `deemon.hasattr()'
 * function. Note that an attribute that is currently unbound, differs from one
 * that does not exist at all. This function will return `1' (true) for the former,
 * but `0' (false) for the later. During normal attribute access, this difference
 * is reflected by the type of exception: `UnboundAttribute' and `AttributeError'.
 * @return: == 0: Attribute doesn't exist
 * @return: > 0:  Attribute exists
 * @return: < 0:  An error was thrown
 * HINT: Use `Dee_HAS_IS*' to test return value */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttr)(DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttrString)(DeeObject *__restrict self, char const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_HasAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
#define DeeObject_HasAttrStringLen(self, attr, attrlen) DeeObject_HasAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* >> DeeObject_BoundAttr() -- <self>.<attr> is bound;
 * @return: Dee_BOUND_YES:     Attribute is bound.
 * @return: Dee_BOUND_NO:      Attribute isn't bound.
 * @return: Dee_BOUND_MISSING: The attribute doesn't exist.
 * @return: Dee_BOUND_ERR:     An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttr)(DeeObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttrString)(DeeObject *__restrict self, char const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttrStringHash)(DeeObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeObject_BoundAttrStringLenHash)(DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
#define DeeObject_BoundAttrStringLen(self, attr, attrlen) \
	DeeObject_BoundAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* With-operator invocation:
 * >> with (myObject) {
 * >>     ...
 * >> }
 * Translates to:
 * >> DeeObject_Enter(myObject);
 * >> try {
 * >>     ...
 * >> } finally {
 * >>     DeeObject_Leave(myObject);
 * >> }
 * @return: 0 : Success.
 * @return: -1: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_Enter)(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int (DCALL DeeObject_Leave)(DeeObject *__restrict self);


/* Object buffer interface.
 * @param: flags: Set of `DEE_BUFFER_F*'
 * @throw: Error.RuntimeError.NotImplemented: The object doesn't implement the buffer protocol.
 * @throw: Error.ValueError.BufferError:      The object is an atomic buffer, or cannot be written to. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_GetBuf)(DeeObject *__restrict self,
                         DeeBuffer *__restrict info,
                         unsigned int flags);



#ifndef __OPTIMIZE_SIZE__
#define DeeObject_GetAttrString(self, attr)                    DeeObject_GetAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeObject_BoundAttrString(self, attr)                  DeeObject_BoundAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeObject_HasAttrString(self, attr)                    DeeObject_HasAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeObject_DelAttrString(self, attr)                    DeeObject_DelAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeObject_SetAttrString(self, attr, value)             DeeObject_SetAttrStringHash(self, attr, Dee_HashStr(attr), value)
#define DeeObject_CallAttrString(self, attr, argc, argv)       DeeObject_CallAttrStringHash(self, attr, Dee_HashStr(attr), argc, argv)
#define DeeObject_CallAttrStringKw(self, attr, argc, argv, kw) DeeObject_CallAttrStringHashKw(self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeObject_CallAttrStringPack(self, attr, ...)          DeeObject_CallAttrStringHashPack(self, attr, Dee_HashStr(attr), __VA_ARGS__)
#define DeeObject_VCallAttrStringPack(self, attr, argc, args)  DeeObject_VCallAttrStringHashPack(self, attr, Dee_HashStr(attr), argc, args)
#define DeeObject_CallAttrStringf(self, attr, ...)             DeeObject_CallAttrStringHashf(self, attr, Dee_HashStr(attr), __VA_ARGS__)
#define DeeObject_VCallAttrStringf(self, attr, format, args)   DeeObject_VCallAttrStringHashf(self, attr, Dee_HashStr(attr), format, args)
#endif /* !__OPTIMIZE_SIZE__ */

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeObject_InplaceDeepCopy(p_self)                              __builtin_expect(DeeObject_InplaceDeepCopy(p_self), 0)
#define DeeObject_InplaceDeepCopyv(object_vector, object_count)        __builtin_expect(DeeObject_InplaceDeepCopyv(object_vector, object_count), 0)
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
#define DeeObject_Inc(p_self)                                          __builtin_expect(DeeObject_Inc(p_self), 0)
#define DeeObject_Dec(p_self)                                          __builtin_expect(DeeObject_Dec(p_self), 0)
#define DeeObject_InplaceAdd(p_self, some_object)                      __builtin_expect(DeeObject_InplaceAdd(p_self, some_object), 0)
#define DeeObject_InplaceSub(p_self, some_object)                      __builtin_expect(DeeObject_InplaceSub(p_self, some_object), 0)
#define DeeObject_InplaceMul(p_self, some_object)                      __builtin_expect(DeeObject_InplaceMul(p_self, some_object), 0)
#define DeeObject_InplaceDiv(p_self, some_object)                      __builtin_expect(DeeObject_InplaceDiv(p_self, some_object), 0)
#define DeeObject_InplaceMod(p_self, some_object)                      __builtin_expect(DeeObject_InplaceMod(p_self, some_object), 0)
#define DeeObject_InplaceShl(p_self, some_object)                      __builtin_expect(DeeObject_InplaceShl(p_self, some_object), 0)
#define DeeObject_InplaceShr(p_self, some_object)                      __builtin_expect(DeeObject_InplaceShr(p_self, some_object), 0)
#define DeeObject_InplaceAnd(p_self, some_object)                      __builtin_expect(DeeObject_InplaceAnd(p_self, some_object), 0)
#define DeeObject_InplaceOr(p_self, some_object)                       __builtin_expect(DeeObject_InplaceOr(p_self, some_object), 0)
#define DeeObject_InplaceXor(p_self, some_object)                      __builtin_expect(DeeObject_InplaceXor(p_self, some_object), 0)
#define DeeObject_InplacePow(p_self, some_object)                      __builtin_expect(DeeObject_InplacePow(p_self, some_object), 0)
#define DeeObject_DelItem(self, index)                                 __builtin_expect(DeeObject_DelItem(self, index), 0)
#define DeeObject_DelItemIndex(self, index)                            __builtin_expect(DeeObject_DelItemIndex(self, index), 0)
#define DeeObject_DelItemStringHash(self, key, hash)                   __builtin_expect(DeeObject_DelItemStringHash(self, key, hash), 0)
#define DeeObject_DelItemStringLenHash(self, key, keylen, hash)        __builtin_expect(DeeObject_DelItemStringLenHash(self, key, keylen, hash), 0)
#define DeeObject_SetItem(self, index, value)                          __builtin_expect(DeeObject_SetItem(self, index, value), 0)
#define DeeObject_SetItemIndex(self, index, value)                     __builtin_expect(DeeObject_SetItemIndex(self, index, value), 0)
#define DeeObject_SetItemStringHash(self, key, hash, value)            __builtin_expect(DeeObject_SetItemStringHash(self, key, hash, value), 0)
#define DeeObject_SetItemStringLenHash(self, key, keylen, hash, value) __builtin_expect(DeeObject_SetItemStringLenHash(self, key, keylen, hash, value), 0)
#define DeeObject_DelRange(self, begin, end)                           __builtin_expect(DeeObject_DelRange(self, begin, end), 0)
#define DeeObject_SetRange(self, begin, end, value)                    __builtin_expect(DeeObject_SetRange(self, begin, end, value), 0)
#define DeeObject_SetRangeBeginIndex(self, begin, end, value)          __builtin_expect(DeeObject_SetRangeBeginIndex(self, begin, end, value), 0)
#define DeeObject_SetRangeEndIndex(self, begin, end, value)            __builtin_expect(DeeObject_SetRangeEndIndex(self, begin, end, value), 0)
#define DeeObject_SetRangeIndex(self, begin, end, value)               __builtin_expect(DeeObject_SetRangeIndex(self, begin, end, value), 0)
#define DeeObject_DelAttr(self, attr)                                  __builtin_expect(DeeObject_DelAttr(self, attr), 0)
#define DeeObject_SetAttr(self, attr, value)                           __builtin_expect(DeeObject_SetAttr(self, attr, value), 0)
#ifndef DeeObject_DelAttrString
#define DeeObject_DelAttrString(self, attr)                            __builtin_expect(DeeObject_DelAttrString(self, attr), 0)
#endif /* !DeeObject_DelAttrString */
#ifndef DeeObject_DelAttrStringLen
#define DeeObject_DelAttrStringLen(self, attr, attrlen)                __builtin_expect(DeeObject_DelAttrStringLen(self, attr, attrlen), 0)
#endif /* !DeeObject_DelAttrStringLen */
#ifndef DeeObject_SetAttrString
#define DeeObject_SetAttrString(self, attr, value)                     __builtin_expect(DeeObject_SetAttrString(self, attr, value), 0)
#endif /* !DeeObject_SetAttrString */
#ifndef DeeObject_SetAttrStringLen
#define DeeObject_SetAttrStringLen(self, attr, attrlen, value)         __builtin_expect(DeeObject_SetAttrStringLen(self, attr, attrlen, value), 0)
#endif /* !DeeObject_SetAttrStringLen */
#ifndef DeeObject_DelAttrStringHash
#define DeeObject_DelAttrStringHash(self, attr, hash)                  __builtin_expect(DeeObject_DelAttrStringHash(self, attr, hash), 0)
#endif /* !DeeObject_DelAttrStringHash */
#ifndef DeeObject_DelAttrStringLenHash
#define DeeObject_DelAttrStringLenHash(self, attr, attrlen, hash)      __builtin_expect(DeeObject_DelAttrStringLenHash(self, attr, attrlen, hash), 0)
#endif /* !DeeObject_DelAttrStringLenHash */
#ifndef DeeObject_SetAttrStringHash
#define DeeObject_SetAttrStringHash(self, attr, hash, value)           __builtin_expect(DeeObject_SetAttrStringHash(self, attr, hash, value), 0)
#endif /* !DeeObject_SetAttrStringHash */
#ifndef DeeObject_SetAttrStringLenHash
#define DeeObject_SetAttrStringLenHash(self, attr, attrlen, hash, value) \
	__builtin_expect(DeeObject_SetAttrStringLenHash(self, attr, attrlen, hash, value), 0)
#endif /* !DeeObject_SetAttrStringLenHash */
#define DeeObject_Enter(self) __builtin_expect(DeeObject_Enter(self), 0)
#define DeeObject_Leave(self) __builtin_expect(DeeObject_Leave(self), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


INTDEF ATTR_ERROR("Invalid integer size") int _Dee_invalid_integer_size(void);
#ifndef __NO_builtin_choose_expr
#define DeeObject_AsIntX(self, result)                                                                 \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeObject_AsInt8(self, (int8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeObject_AsInt16(self, (int16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeObject_AsInt32(self, (int32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeObject_AsInt64(self, (int64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeObject_AsInt128(self, (Dee_int128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#define DeeObject_AsUIntX(self, result)                                                                  \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeObject_AsUInt8(self, (uint8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeObject_AsUInt16(self, (uint16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeObject_AsUInt32(self, (uint32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeObject_AsUInt64(self, (uint64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeObject_AsUInt128(self, (Dee_uint128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#else /* !__NO_builtin_choose_expr */
#define DeeObject_AsIntX(self, result)                                              \
	(sizeof(*(result)) == 1 ?  DeeObject_AsInt8(self, (int8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeObject_AsInt16(self, (int16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeObject_AsInt32(self, (int32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeObject_AsInt64(self, (int64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeObject_AsInt128(self, (Dee_int128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#define DeeObject_AsUIntX(self, result)                                               \
	(sizeof(*(result)) == 1 ?  DeeObject_AsUInt8(self, (uint8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeObject_AsUInt16(self, (uint16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeObject_AsUInt32(self, (uint32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeObject_AsUInt64(self, (uint64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeObject_AsUInt128(self, (Dee_uint128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#endif /* __NO_builtin_choose_expr */

DECL_END

#endif /* !GUARD_DEEMON_OBJECT_H */
