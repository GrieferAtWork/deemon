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
/*!export ASSERT_OBJECT_**/
/*!export ASSERT_OBJECT_TYPE_**/
/*!export -CONFIG_HAVE_**/
/*!export DeeAssert_BadObject**/
/*!export DeeBuffer_**/
/*!export DeeFatal_Bad**/
/*!export _DeeFatal_Bad**/
/*!export DeeObject_**/
/*!export Dee_ASSERT_OBJECT_**/
/*!export Dee_BOUND_**/
/*!export Dee_BUFFER_F**/
/*!export Dee_COMPARE_**/
/*!export Dee_Clear_**/
/*!export Dee_Compare**/
/*!export Dee_Incref**/
/*!export Dee_Decref**/
/*!export Dee_Incprefv**/
/*!export Dee_Decprefv**/
/*!export Dee_HAS_**/
/*!export Dee_Movrefv**/
/*!export Dee_Movprefv**/
/*!export -_Dee_PRIVATE_**/
/*!export Dee_Setrefv**/
/*!export Dee_Setprefv**/
/*!export Dee_XClear**/
/*!export Dee_XDecprefv**/
/*!export Dee_XDecrefv**/
/*!export Dee_XDecref**/
/*!export Dee_XIncprefv**/
/*!export Dee_XIncrefv**/
/*!export Dee_XIncref**/
/*!export Dee_XMovprefv**/
/*!export Dee_XMovrefv**/
/*!export Dee_return_**/
/*!export _DeeRefcnt_**/
/*!always include "types.h"*/        /* This header always being included is guarantied by the ABI */
#ifndef GUARD_DEEMON_OBJECT_H
#define GUARD_DEEMON_OBJECT_H 1 /*!export-*/

#include "api.h"

#include <hybrid/__atomic.h> /* __ATOMIC_ACQUIRE, __ATOMIC_SEQ_CST, __hybrid_atomic_* */
#include <hybrid/host.h>     /* __i386__, __x86_64__ */
#include <hybrid/typecore.h> /* __*_TYPE__, __CHAR_UNSIGNED__, __SIZEOF_*__ */

#include "types.h"     /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_SIZEOF_REFCNT_T, Dee_TYPE, Dee_[u]int128_t, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_refcnt_t, Dee_ssize_t */
#include "util/hash.h" /* Dee_HashPtr, Dee_HashStr */
#include "util/lock.h" /* Dee_atomic_lock_t, Dee_atomic_rwlock_t */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, ptrdiff_t, size_t */
#include <stdint.h>  /* intN_t, intptr_t, uintN_t, uintptr_t */

/* To satisfy "fixincludes" (these includes are intentionally missing) */
/*!fixincludes fake_include "none.h"            // DeeNone_Check, DeeNone_CheckExact */
/*!fixincludes fake_include "system-features.h" // CONFIG_HAVE_STRING_H, CONFIG_HAVE_memsetp, memsetp */
/*!fixincludes fake_include "tuple.h"           // DeeTuple_ELEM, DeeTuple_SIZE */
/*!fixincludes fake_include "type.h"            // DeeType_IsAbstract */

DECL_BEGIN

/* Return value for compare "tp_compare_eq", "tp_compare" and "tp_trycompare_eq". */
#define Dee_COMPARE_ERR (-2)
#define Dee_COMPARE_EQ  0
#define Dee_COMPARE_NE  1 /* or "-1" */
#define Dee_COMPARE_LO  (-1)
#define Dee_COMPARE_GR  1

#define Dee_COMPARE_ISERR(x)       __builtin_expect((x) == Dee_COMPARE_ERR, 0)
#define Dee_COMPARE_ISEQ_NO_ERR(x) ((x) == 0) /* Never matches "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISEQ(x)        ((x) == 0) /* Undefined if matches "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISNE_OR_ERR(x) ((x) != 0) /* Guarantied to match "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISNE(x)        ((x) != 0) /* Undefined if matches "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISLO(x)        ((x) < 0)  /* Undefined if matches "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISLE(x)        ((x) <= 0) /* Undefined if matches "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISGR(x)        ((x) > 0)  /* Undefined if matches "Dee_COMPARE_ERR" */
#define Dee_COMPARE_ISGE(x)        ((x) >= 0) /* Undefined if matches "Dee_COMPARE_ERR" */

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
#define Dee_return_DeeObject_Compare_if_ne(a, b)                \
	do {                                                        \
		int _rtceqin_temp = DeeObject_Compare(Dee_AsObject(a),  \
		                                      Dee_AsObject(b)); \
		if (_rtceqin_temp != Dee_COMPARE_EQ)                    \
			return _rtceqin_temp;                               \
	}	__WHILE0
#define Dee_return_DeeObject_CompareEq_if_ne(a, b)                \
	do {                                                          \
		int _rtceqin_temp = DeeObject_CompareEq(Dee_AsObject(a),  \
		                                        Dee_AsObject(b)); \
		if (_rtceqin_temp != Dee_COMPARE_EQ)                      \
			return _rtceqin_temp;                                 \
	}	__WHILE0
#define Dee_return_DeeObject_TryCompareEq_if_ne(a, b)                \
	do {                                                             \
		int _rtceqin_temp = DeeObject_TryCompareEq(Dee_AsObject(a),  \
		                                           Dee_AsObject(b)); \
		if (_rtceqin_temp != Dee_COMPARE_EQ)                         \
			return _rtceqin_temp;                                    \
	}	__WHILE0


#ifndef NDEBUG
#define DeeObject_Check(ob) \
	((ob)->ob_refcnt &&     \
	 (ob)->ob_type &&       \
	 ((DeeObject *)(ob)->ob_type)->ob_refcnt)
#define Dee_ASSERT_OBJECT(ob)                                     Dee_ASSERT_OBJECT_AT(ob, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_OPT(ob)                                 Dee_ASSERT_OBJECT_OPT_AT(ob, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE(ob, type)                          Dee_ASSERT_OBJECT_TYPE_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_OPT(ob, type)                      Dee_ASSERT_OBJECT_TYPE_OPT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_EXACT(ob, type)                    Dee_ASSERT_OBJECT_TYPE_EXACT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT(ob, type)                Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT(ob, type, __FILE__, __LINE__)
#define Dee_ASSERT_OBJECT_AT(ob, file, line)                      (void)(likely(DeeObject_Check(ob)) || (DeeAssert_BadObject((DeeObject *)(ob), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_OPT_AT(ob, file, line)                  (void)(likely(!(ob) || DeeObject_Check(ob)) || (DeeAssert_BadObjectOpt((DeeObject *)(ob), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_AT(ob, type, file, line)           (void)(likely((DeeObject_Check(ob) && DeeObject_InstanceOf(ob, type))) || (DeeAssert_BadObjectType((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_OPT_AT(ob, type, file, line)       (void)(likely(!(ob) || (DeeObject_Check(ob) && DeeObject_InstanceOf(ob, type))) || (DeeAssert_BadObjectTypeOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_EXACT_AT(ob, type, file, line)     (void)(likely((DeeObject_Check(ob) && DeeObject_InstanceOfExact(ob, type))) || (DeeAssert_BadObjectTypeExact((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT(ob, type, file, line) (void)(likely(!(ob) || (DeeObject_Check(ob) && DeeObject_InstanceOfExact(ob, type))) || (DeeAssert_BadObjectTypeExactOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0))

DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObject(DeeObject const *ob, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectOpt(DeeObject const *ob, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectType(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeOpt(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeExact(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);
DFUNDEF ATTR_COLD void DCALL DeeAssert_BadObjectTypeExactOpt(DeeObject const *ob, DeeTypeObject const *wanted_type, char const *file, int line);

#ifdef GUARD_DEEMON_TYPE_H
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)                        Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, __FILE__, __LINE__) /*!export(include("type.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)                    Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, __FILE__, __LINE__) /*!export(include("type.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, file, line)         (void)(likely((DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type)))) || (DeeAssert_BadObjectType((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0)) /*!export(include("type.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, file, line)     (void)(likely(!(ob) || (DeeObject_Check(ob) && (DeeObject_InstanceOf(ob, type) || DeeType_IsAbstract(type)))) || (DeeAssert_BadObjectTypeOpt((DeeObject *)(ob), (DeeTypeObject *)(type), file, line), Dee_BREAKPOINT(), 0)) /*!export(include("type.h"))*/
#endif /* GUARD_DEEMON_TYPE_H */
#else /* !NDEBUG */
#define DeeObject_Check(ob)                                       true
#define Dee_ASSERT_OBJECT(ob)                                     (void)0
#define Dee_ASSERT_OBJECT_OPT(ob)                                 (void)0
#define Dee_ASSERT_OBJECT_TYPE(ob, type)                          (void)0
#define Dee_ASSERT_OBJECT_TYPE_OPT(ob, type)                      (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT(ob, type)                    (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT(ob, type)                (void)0
#define Dee_ASSERT_OBJECT_AT(ob, file, line)                      (void)0
#define Dee_ASSERT_OBJECT_OPT_AT(ob, file, line)                  (void)0
#define Dee_ASSERT_OBJECT_TYPE_AT(ob, type, file, line)           (void)0
#define Dee_ASSERT_OBJECT_TYPE_OPT_AT(ob, type, file, line)       (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_AT(ob, type, file, line)     (void)0
#define Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT(ob, type, file, line) (void)0
#ifdef GUARD_DEEMON_TYPE_H
#define Dee_ASSERT_OBJECT_TYPE_A(ob, type)                        (void)0 /*!export(include("type.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT(ob, type)                    (void)0 /*!export(include("type.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_AT(ob, type, file, line)         (void)0 /*!export(include("type.h"))*/
#define Dee_ASSERT_OBJECT_TYPE_A_OPT_AT(ob, type, file, line)     (void)0 /*!export(include("type.h"))*/
#endif /* GUARD_DEEMON_TYPE_H */
#endif /* NDEBUG */

#ifdef DEE_SOURCE
#define ASSERT_OBJECT                   Dee_ASSERT_OBJECT
#define ASSERT_OBJECT_OPT               Dee_ASSERT_OBJECT_OPT
#define ASSERT_OBJECT_TYPE              Dee_ASSERT_OBJECT_TYPE
#define ASSERT_OBJECT_TYPE_OPT          Dee_ASSERT_OBJECT_TYPE_OPT
#define ASSERT_OBJECT_TYPE_EXACT        Dee_ASSERT_OBJECT_TYPE_EXACT
#define ASSERT_OBJECT_TYPE_EXACT_OPT    Dee_ASSERT_OBJECT_TYPE_EXACT_OPT
#define ASSERT_OBJECT_AT                Dee_ASSERT_OBJECT_AT
#define ASSERT_OBJECT_OPT_AT            Dee_ASSERT_OBJECT_OPT_AT
#define ASSERT_OBJECT_TYPE_AT           Dee_ASSERT_OBJECT_TYPE_AT
#define ASSERT_OBJECT_TYPE_OPT_AT       Dee_ASSERT_OBJECT_TYPE_OPT_AT
#define ASSERT_OBJECT_TYPE_EXACT_AT     Dee_ASSERT_OBJECT_TYPE_EXACT_AT
#define ASSERT_OBJECT_TYPE_EXACT_OPT_AT Dee_ASSERT_OBJECT_TYPE_EXACT_OPT_AT
#ifdef GUARD_DEEMON_TYPE_H
#define ASSERT_OBJECT_TYPE_A        Dee_ASSERT_OBJECT_TYPE_A        /*!export(include("type.h"))*/
#define ASSERT_OBJECT_TYPE_A_OPT    Dee_ASSERT_OBJECT_TYPE_A_OPT    /*!export(include("type.h"))*/
#define ASSERT_OBJECT_TYPE_A_AT     Dee_ASSERT_OBJECT_TYPE_A_AT     /*!export(include("type.h"))*/
#define ASSERT_OBJECT_TYPE_A_OPT_AT Dee_ASSERT_OBJECT_TYPE_A_OPT_AT /*!export(include("type.h"))*/
#endif /* GUARD_DEEMON_TYPE_H */
#endif /* DEE_SOURCE */



/* Destroy a given deemon object (called when its refcnt reaches `0') */
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
DFUNDEF WUNUSED NONNULL((1)) Dee_refcnt_t (DCALL Dee_DecrefAndFetch)(DeeObject *__restrict ob);

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
#define Dee_DecrefAndFetch_untraced(x)  (Dee_Decref(x), (x)->ob_refcnt)
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
#define Dee_DecrefAndFetch_untraced(x)  ((x)->ob_refcnt-- > 1 ? (x)->ob_refcnt : (DeeObject_Destroy((DeeObject *)(x)), 0))
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_Incref_untraced(x)          (void)(++(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     (void)((x)->ob_refcnt += (n))
#define Dee_Decref_untraced(x)          (void)(--(x)->ob_refcnt || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_n_untraced(x, n)     (void)(((x)->ob_refcnt -= (n)) != 0 || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(--(x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(--(x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    (void)(DeeObject_Destroy((DeeObject *)(x)))
#define Dee_DecrefNokill_untraced(x)    (void)(--(x)->ob_refcnt)
#define Dee_DecrefAndFetch_untraced(x)  (--(x)->ob_refcnt ? ((x)->ob_refcnt) : (DeeObject_Destroy((DeeObject *)(x)), 0))
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
#define Dee_DecrefIfOne_untraced(self)  Dee_DecrefIfOne_untraced_d((DeeObject *)(self), __FILE__, __LINE__)
#define Dee_DecrefAndFetch_untraced(x)  Dee_DecrefAndFetch_untraced(x)
LOCAL ATTR_ARTIFICIAL WUNUSED NONNULL((1)) Dee_refcnt_t
(DCALL Dee_DecrefAndFetch_untraced)(DeeObject *__restrict self) {
	Dee_refcnt_t refcnt = _DeeRefcnt_FetchDec(&self->ob_refcnt);
	if (refcnt <= 1)
		DeeObject_Destroy(self);
	return refcnt - 1;
}
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
#define Dee_Incref_untraced(x)          _DeeRefcnt_Inc(&(x)->ob_refcnt)
#define Dee_Incref_n_untraced(x, n)     _DeeRefcnt_Add(&(x)->ob_refcnt, n)
#define Dee_Decref_untraced(x)          (void)(_DeeRefcnt_DecFetch(&(x)->ob_refcnt) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_n_untraced(x, n)     (void)(_DeeRefcnt_SubFetch(&(x)->ob_refcnt, n) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_likely_untraced(x)   (void)(unlikely(_DeeRefcnt_DecFetch(&(x)->ob_refcnt)) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_Decref_unlikely_untraced(x) (void)(likely(_DeeRefcnt_DecFetch(&(x)->ob_refcnt)) || (DeeObject_Destroy((DeeObject *)(x)), 0))
#define Dee_DecrefDokill_untraced(x)    DeeObject_Destroy((DeeObject *)(x))
#define Dee_DecrefNokill_untraced(x)    (void)_DeeRefcnt_DecFetch(&(x)->ob_refcnt)
#define Dee_DecrefIfOne_untraced(self)  Dee_DecrefIfOne_untraced((DeeObject *)(self))
#define Dee_DecrefAndFetch_untraced(x)  Dee_DecrefAndFetch_untraced(x)
LOCAL ATTR_ARTIFICIAL WUNUSED NONNULL((1)) Dee_refcnt_t
(DCALL Dee_DecrefAndFetch_untraced)(DeeObject *__restrict self) {
	Dee_refcnt_t refcnt = _DeeRefcnt_DecFetch(&self->ob_refcnt);
	if (refcnt == 0)
		DeeObject_Destroy(self);
	return refcnt;
}
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

/* General-purpose reference count control functions:
 *
 * ============================== INCREF ==============================
 * Dee_Incref                        Increment reference count
 *                                   - Illegal if "ob_refcnt == 0"
 * Dee_Incref_n                      Increment reference count "n" times
 *                                   - Illegal if "ob_refcnt == 0"
 *                                   - no-op when "n == 0"
 * Dee_IncrefIfNotZero               Increment reference count only if "ob_refcnt != 0"
 *                                   - Returns true/false indicative of "ob_refcnt" having changed
 * ============================== DECREF ==============================
 * Dee_Decref                        Decrement reference count
 *                                   - Invoke `DeeObject_Destroy()' if "ob_refcnt == 0" after decrement
 *                                   - Illegal if "ob_refcnt == 0" at the start of the call
 * Dee_Decref_n                      Decrement reference count "n" times
 *                                   - Invoke `DeeObject_Destroy()' if "ob_refcnt == 0" after decrement
 *                                   - no-op when "n == 0"
 *                                   - Illegal if "ob_refcnt < n" at the start of the call
 * Dee_DecrefDokill                  Assert that "ob_refcnt == 1", the call `DeeObject_Destroy()'
 * Dee_DecrefNokill                  Decrement reference count
 *                                   - Assert that "ob_refcnt >= 2" on entry
 *                                   - Illegal if "ob_refcnt < 2" at the start of the call
 *                                   - `DeeObject_Destroy()' is never called
 * Dee_DecrefIfOne                   Decrement reference count only iff "ob_refcnt == 1"
 *                                   - Illegal if "ob_refcnt == 0" at the start of the call
 *                                   - Returns true/false indicative of "ob_refcnt" having changed
 *                                   - When true is returned, `DeeObject_Destroy()' is also called
 * Dee_DecrefIfNotOne                Decrement reference count only iff "ob_refcnt != 1"
 *                                   - Illegal if "ob_refcnt == 0" at the start of the call
 *                                   - Returns true/false indicative of "ob_refcnt" having changed
 *                                   - `DeeObject_Destroy()' is never called
 * Dee_DecrefAndFetch                Same as Dee_Decref, but also returns the new reference count
 *                                   - When `0' is returned, `DeeObject_Destroy()' was called
 * Dee_Decref_likely                 Same as Dee_Decref, but it is likely that "ob_refcnt == 0" after the decref
 * Dee_Decref_unlikely               Same as Dee_Decref, but it is unlikely that "ob_refcnt == 0" after the decref
 */
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
DFUNDEF WUNUSED NONNULL((1)) Dee_refcnt_t DCALL Dee_DecrefAndFetch_traced(DeeObject *__restrict ob, char const *file, int line);
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
#define Dee_DecrefAndFetch(x)   Dee_DecrefAndFetch_traced((DeeObject *)(x), __FILE__, __LINE__)
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
#define Dee_DecrefAndFetch_traced(x, file, line)  Dee_DecrefAndFetch_untraced(x)
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
#define Dee_DecrefAndFetch(x)                     Dee_DecrefAndFetch_untraced(x)
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


/* Try to define `_Dee_PRIVATE_MEMSETP' with platform-specific optimizations (if possible) */
#undef _Dee_PRIVATE_MEMSETP
#ifdef CONFIG_INLINE_INCREFV
#ifdef CONFIG_HAVE_memsetp
#define _Dee_PRIVATE_MEMSETP memsetp
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
extern void __stosd(unsigned long *, unsigned long, unsigned __int64); /*!export-*/
#define memsetl(dst, c, n)                   \
	(__stosd((unsigned long *)(void *)(dst), \
	         (unsigned long)(c),             \
	         (unsigned __int64)(n)),         \
	 (uint32_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __stosd(unsigned long *, unsigned long, unsigned int); /*!export-*/
#define memsetl(dst, c, n) /*!export-*/      \
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
#define _Dee_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
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
extern void __stosq(unsigned long long *, unsigned long long, unsigned __int64); /*!export-*/
#undef memsetq
#define memsetq(dst, c, n) /*!export-*/           \
	(__stosq((unsigned long long *)(void *)(dst), \
	         (unsigned long long)(c),             \
	         (unsigned __int64)(n)),              \
	 (uint64_t *)(void *)(dst))
#pragma intrinsic(__stosq)
DECL_END
#endif /* _MSC_VER && __x86_64__ */
#endif /* !CONFIG_HAVE_memsetq */
#ifdef CONFIG_HAVE_memsetq
#define _Dee_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
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
extern void __stosw(unsigned short *, unsigned short, unsigned __int64); /*!export-*/
#define memsetw(dst, c, n) /*!export-*/       \
	(__stosw((unsigned short *)(void *)(dst), \
	         (unsigned short)(c),             \
	         (unsigned __int64)(n)),          \
	 (uint16_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __stosw(unsigned short *, unsigned short, unsigned int); /*!export-*/
#define memsetw(dst, c, n) /*!export-*/       \
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
#define _Dee_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
	memsetw(dst, (uint16_t)(pointer), num_pointers)
#endif /* CONFIG_HAVE_memsetw */
#elif __SIZEOF_POINTER__ == 1
#ifdef CONFIG_NO_memset
#undef CONFIG_HAVE_memset
#else /* CONFIG_NO_memset */
#define CONFIG_HAVE_memset
#endif /* !CONFIG_NO_memset */
#ifndef CONFIG_HAVE_memset
#define CONFIG_HAVE_memset
DECL_BEGIN
#undef memset
#define memset dee_memset /*!export-*/
LOCAL WUNUSED ATTR_OUTS(1, 3) void *
dee_memset(void *__restrict dst, int byte, size_t num_bytes) { /*!export-*/
	uint8_t *dst_p = (uint8_t *)dst;
	while (num_bytes--)
		*dst_p++ = (uint8_t)(unsigned int)byte;
	return dst;
}
DECL_END
#endif /* !CONFIG_HAVE_memset */
#define _Dee_PRIVATE_MEMSETP(dst, pointer, num_pointers) \
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
#ifdef _Dee_PRIVATE_MEMSETP
	Dee_Incref_n_untraced(obj, object_count);
	return (DREF DeeObject **)_Dee_PRIVATE_MEMSETP(dst, obj, object_count);
#else /* _Dee_PRIVATE_MEMSETP */
	size_t i;
	Dee_Incref_n_untraced(obj, object_count);
	for (i = 0; i < object_count; ++i)
		dst[i] = obj;
	return dst;
#endif /* !_Dee_PRIVATE_MEMSETP */
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


/* The base class of all objects. */
DDATDEF DeeTypeObject DeeObject_Type; /* `Object' */


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
#define DeeObject_TypeAssertFailed(self, required_type) \
	Dee_ASSUMED_VALUE(DeeObject_TypeAssertFailed(Dee_AsObject(self), required_type), -1)
#define DeeObject_TypeAssertFailed2(self, required_type1, required_type2) \
	Dee_ASSUMED_VALUE(DeeObject_TypeAssertFailed2(Dee_AsObject(self), required_type1, required_type2), -1)
#define DeeObject_TypeAssertFailed3(self, required_type1, required_type2, required_type3) \
	Dee_ASSUMED_VALUE(DeeObject_TypeAssertFailed3(Dee_AsObject(self), required_type1, required_type2, required_type3), -1)
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

#ifndef Dee_visit_t_DEFINED
#define Dee_visit_t_DEFINED /*!export-*/
typedef NONNULL_T((1)) void (DCALL *Dee_visit_t)(DeeObject *__restrict self, void *arg);
#endif /* !Dee_visit_t_DEFINED */

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

#define _Dee_PRIVATE_OBJECT_AS_INT_1(self, result)     DeeObject_AsInt8(self, (int8_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_INT_2(self, result)     DeeObject_AsInt16(self, (int16_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_INT_4(self, result)     DeeObject_AsInt32(self, (int32_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_INT_8(self, result)     DeeObject_AsInt64(self, (int64_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_INT_16(self, result)    DeeObject_AsInt128(self, (Dee_int128_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_INT(size)               _Dee_PRIVATE_OBJECT_AS_INT_##size
#define _Dee_PRIVATE_OBJECT_AS_UINT_1(self, result)    DeeObject_AsUInt8(self, (uint8_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINT_2(self, result)    DeeObject_AsUInt16(self, (uint16_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINT_4(self, result)    DeeObject_AsUInt32(self, (uint32_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINT_8(self, result)    DeeObject_AsUInt64(self, (uint64_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINT_16(self, result)   DeeObject_AsUInt128(self, (Dee_uint128_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINT(size)              _Dee_PRIVATE_OBJECT_AS_UINT_##size
#define _Dee_PRIVATE_OBJECT_AS_UINTM1_1(self, result)  DeeObject_AsUInt8M1(self, (uint8_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINTM1_2(self, result)  DeeObject_AsUInt16M1(self, (uint16_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINTM1_4(self, result)  DeeObject_AsUInt32M1(self, (uint32_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINTM1_8(self, result)  DeeObject_AsUInt64M1(self, (uint64_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINTM1_16(self, result) DeeObject_AsUInt128M1(self, (Dee_uint128_t *)(result))
#define _Dee_PRIVATE_OBJECT_AS_UINTM1(size)            _Dee_PRIVATE_OBJECT_AS_UINTM1_##size

#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_1(self) DeeObject_AsUInt8Direct(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_2(self) DeeObject_AsUInt16Direct(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_4(self) DeeObject_AsUInt32Direct(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_8(self) DeeObject_AsUInt64Direct(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT(size)   _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_##size

#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_1(self) DeeObject_AsUInt8DirectInherited(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_2(self) DeeObject_AsUInt16DirectInherited(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_4(self) DeeObject_AsUInt32DirectInherited(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_8(self) DeeObject_AsUInt64DirectInherited(self)
#define _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED(size)   _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_##size

#define DeeObject_MapAsXInt(size)                 _Dee_PRIVATE_OBJECT_AS_INT(size)
#define DeeObject_MapAsXUInt(size)                _Dee_PRIVATE_OBJECT_AS_UINT(size)
#define DeeObject_MapAsXUIntM1(size)              _Dee_PRIVATE_OBJECT_AS_UINTM1(size)
#define DeeObject_MapAsXUIntDirect(size)          _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT(size)
#define DeeObject_MapAsXUIntDirectInherited(size) _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED(size)

/* Helper macros for converting objects to integers */
#define DeeObject_AsXInt(size, self, result)         _Dee_PRIVATE_OBJECT_AS_INT(size)(self, result)
#define DeeObject_AsXUInt(size, self, result)        _Dee_PRIVATE_OBJECT_AS_UINT(size)(self, result)
#define DeeObject_AsXUIntM1(size, self, result)      _Dee_PRIVATE_OBJECT_AS_UINTM1(size)(self, result)
#define DeeObject_AsXUIntDirect(size, self)          _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT(size)(self)
#define DeeObject_AsXUIntDirectInherited(size, self) _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED(size)(self)
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
#define DeeObject_AsSByte(self, result)          _Dee_PRIVATE_OBJECT_AS_INT_1(self, Dee_REQUIRES_TYPE(__SBYTE_TYPE__ *, result))
#define DeeObject_AsByte(self, result)           _Dee_PRIVATE_OBJECT_AS_UINT_1(self, Dee_REQUIRES_TYPE(__BYTE_TYPE__ *, result))
#define DeeObject_AsByteM1(self, result)         _Dee_PRIVATE_OBJECT_AS_UINTM1_1(self, Dee_REQUIRES_TYPE(__BYTE_TYPE__ *, result))
#define DeeObject_AsByteDirect(self)             _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_1(self)
#define DeeObject_AsByteDirectInherited(self)    _Dee_PRIVATE_OBJECT_AS_DIRECT_UINT_INHERITED_1(self)

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
#define Dee_BOUND_ERR      Dee_HAS_ERR /* Guarantied to equal `Dee_HAS_ERR' to allow (e.g.) `tp_hasitem' to alias `tp_bounditem' */
#define Dee_BOUND_MISSING  Dee_HAS_NO  /* Guarantied to equal `Dee_HAS_NO' to allow (e.g.) `tp_hasitem' to alias `tp_bounditem' */
#define Dee_BOUND_YES      Dee_HAS_YES /* Guarantied to equal `Dee_HAS_YES' to allow (e.g.) `tp_hasitem' to alias `tp_bounditem' */
#define Dee_BOUND_NO       2

/* #define Dee_BOUND_ISBOUND(x) ((x) == Dee_BOUND_YES) */
#define Dee_BOUND_ISBOUND(x) ((x) == Dee_BOUND_YES)

/* #define Dee_BOUND_ISUNBOUND(x) ((x) == Dee_BOUND_NO) */
#define Dee_BOUND_ISUNBOUND(x) ((x) == Dee_BOUND_NO)

/* #define Dee_BOUND_ISMISSING(x) ((x) == Dee_BOUND_MISSING) */
#define Dee_BOUND_ISMISSING(x) Dee_HAS_ISNO(x)

/* #define Dee_BOUND_ISERR(x) ((x) == Dee_BOUND_ERR) */
#define Dee_BOUND_ISERR(x) Dee_HAS_ISERR(x)

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

/* Considers has_value == true as bound */
#define Dee_BOUND_FROMHAS_BOUND(has_value) \
	(Dee_HAS_ISERR(has_value) ? Dee_BOUND_ERR : Dee_BOUND_FROMPRESENT_BOUND(has_value))

/* Considers has_value == true as unbound */
#define Dee_BOUND_FROMHAS_UNBOUND(has_value) \
	(Dee_HAS_ISERR(has_value) ? Dee_BOUND_ERR : Dee_BOUND_FROMPRESENT_UNBOUND(has_value))



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


typedef struct Dee_buffer {
	void  *bb_base; /* [0..bb_size][const] Base address of the buffer.
	                 * NOTE: Only writable if the buffer was acquired with `Dee_BUFFER_FWRITABLE' set. */
	size_t bb_size; /* [const] Size of the buffer (in bytes) */
} DeeBuffer;
#define DeeBuffer_Fini(self) (void)0

/* Possible values for `DeeObject_GetBuf::flags' */
#define Dee_BUFFER_FREADONLY 0x0000 /* Acquire the buffer for reading. */
#define Dee_BUFFER_FWRITABLE 0x0001 /* Acquire the buffer for reading / writing. */
#define Dee_BUFFER_FMASK     0x0001 /* Mask of known buffer flags. */

/* Object buffer interface.
 * @param: flags: Set of `DEE_BUFFER_F*'
 * @throw: Error.RuntimeError.NotImplemented: The object doesn't implement the buffer protocol.
 * @throw: Error.ValueError.BufferError:      The object is an atomic buffer, or cannot be written to. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_GetBuf)(DeeObject *__restrict self,
                         struct Dee_buffer *__restrict info,
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


INTDEF ATTR_ERROR("Invalid integer size") int _Dee_PRIVATE_invalid_integer_size(void);
#ifndef __NO_builtin_choose_expr
#define DeeObject_AsIntX(self, result)                                                                 \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeObject_AsInt8(self, (int8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeObject_AsInt16(self, (int16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeObject_AsInt32(self, (int32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeObject_AsInt64(self, (int64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeObject_AsInt128(self, (Dee_int128_t *)(result)), \
	                                               _Dee_PRIVATE_invalid_integer_size())))))
#define DeeObject_AsUIntX(self, result)                                                                  \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeObject_AsUInt8(self, (uint8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeObject_AsUInt16(self, (uint16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeObject_AsUInt32(self, (uint32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeObject_AsUInt64(self, (uint64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeObject_AsUInt128(self, (Dee_uint128_t *)(result)), \
	                                               _Dee_PRIVATE_invalid_integer_size())))))
#else /* !__NO_builtin_choose_expr */
#define DeeObject_AsIntX(self, result)                                              \
	(sizeof(*(result)) == 1 ?  DeeObject_AsInt8(self, (int8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeObject_AsInt16(self, (int16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeObject_AsInt32(self, (int32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeObject_AsInt64(self, (int64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeObject_AsInt128(self, (Dee_int128_t *)(result)) : \
	                           _Dee_PRIVATE_invalid_integer_size())
#define DeeObject_AsUIntX(self, result)                                               \
	(sizeof(*(result)) == 1 ?  DeeObject_AsUInt8(self, (uint8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeObject_AsUInt16(self, (uint16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeObject_AsUInt32(self, (uint32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeObject_AsUInt64(self, (uint64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeObject_AsUInt128(self, (Dee_uint128_t *)(result)) : \
	                           _Dee_PRIVATE_invalid_integer_size())
#endif /* __NO_builtin_choose_expr */

DECL_END

#endif /* !GUARD_DEEMON_OBJECT_H */
