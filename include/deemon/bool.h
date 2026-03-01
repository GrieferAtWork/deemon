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
/*!export DeeBool**/
/*!export _DeeBool**/
#ifndef GUARD_DEEMON_BOOL_H
#define GUARD_DEEMON_BOOL_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include <stdint.h> /* uintptr_t */

#ifndef __INTELLISENSE__
#include "object.h" /* DeeObject_NewRef, Dee_DecrefNokill, Dee_Decref_unlikely, Dee_Incref */
#endif /* !__INTELLISENSE__ */
#include "types.h" /* DREF, DeeObject, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_OBJECT_OFFSETOF_DATA, Dee_REQUIRES_OBJECT */

DECL_BEGIN

#ifdef DEE_SOURCE
#define return_bool   Dee_return_bool
#define return_bool01 Dee_return_bool01
#define return_true   Dee_return_true
#define return_false  Dee_return_false
#endif /* DEE_SOURCE */

#ifdef CONFIG_EXPERIMENTAL_PER_THREAD_BOOL

#if Dee_OBJECT_OFFSETOF_DATA <= 4
#define _Dee_SIZEOF_BOOL_OBJECT 4
#elif Dee_OBJECT_OFFSETOF_DATA <= 8
#define _Dee_SIZEOF_BOOL_OBJECT 8
#elif Dee_OBJECT_OFFSETOF_DATA <= 16
#define _Dee_SIZEOF_BOOL_OBJECT 16
#elif Dee_OBJECT_OFFSETOF_DATA <= 32
#define _Dee_SIZEOF_BOOL_OBJECT 32
#elif Dee_OBJECT_OFFSETOF_DATA <= 64
#define _Dee_SIZEOF_BOOL_OBJECT 64
#elif Dee_OBJECT_OFFSETOF_DATA <= 128
#define _Dee_SIZEOF_BOOL_OBJECT 128
#else /* Dee_OBJECT_OFFSETOF_DATA == 128 */
#error "Configured 'Dee_OBJECT_OFFSETOF_DATA' is too large"
#endif /* Dee_OBJECT_OFFSETOF_DATA != 128 */

typedef struct ATTR_ALIGNED(_Dee_SIZEOF_BOOL_OBJECT) {
	Dee_OBJECT_HEAD
#if _Dee_SIZEOF_BOOL_OBJECT > Dee_OBJECT_OFFSETOF_DATA
	__BYTE_TYPE__ _b_pad[_Dee_SIZEOF_BOOL_OBJECT - Dee_OBJECT_OFFSETOF_DATA];
#endif /* _Dee_SIZEOF_BOOL_OBJECT > Dee_OBJECT_OFFSETOF_DATA */
} DeeBoolObject;

#if _Dee_SIZEOF_BOOL_OBJECT == 4
#define _Dee_ALIGNOF_BOOL_PAIR 8
#elif _Dee_SIZEOF_BOOL_OBJECT == 8
#define _Dee_ALIGNOF_BOOL_PAIR 16
#elif _Dee_SIZEOF_BOOL_OBJECT == 16
#define _Dee_ALIGNOF_BOOL_PAIR 32
#elif _Dee_SIZEOF_BOOL_OBJECT == 32
#define _Dee_ALIGNOF_BOOL_PAIR 64
#elif _Dee_SIZEOF_BOOL_OBJECT == 64
#define _Dee_ALIGNOF_BOOL_PAIR 128
#else /* _Dee_SIZEOF_BOOL_OBJECT == ... */
#define _Dee_ALIGNOF_BOOL_PAIR (_Dee_SIZEOF_BOOL_OBJECT * 2)
#endif /* _Dee_SIZEOF_BOOL_OBJECT != ... */

union _Dee_bool_pair;
typedef union ATTR_ALIGNED(_Dee_ALIGNOF_BOOL_PAIR) _Dee_bool_pair {
	DeeBoolObject bp_bools[2];
} _DeeBool_Pair;

DDATDEF ATTR_ALIGNED(_Dee_ALIGNOF_BOOL_PAIR) _DeeBool_Pair Dee_FalseTrue;

#define DeeBool_IsTrue(x)     (((uintptr_t)(x) & _Dee_SIZEOF_BOOL_OBJECT) != 0)
#define DeeBool_CheckTrue(x)  (DeeBool_Check(x) && DeeBool_IsTrue(x))
#define DeeBool_CheckFalse(x) (DeeBool_Check(x) && !DeeBool_IsTrue(x))

/* Returns a pointer to the calling thread's thread-local boolean pair. */
DFUNDEF ATTR_CONST ATTR_RETNONNULL WUNUSED _DeeBool_Pair *DCALL DeeBool_GetPair(void);
#define _DeeBool_For01(val) (&DeeBool_GetPair()->bp_bools[val])

#else /* CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */

/* HINT: In i386 assembly, `bool' is 8 bytes, so if you want to
 *       convert an integer 0/1 into a boolean, you can use the
 *       following assembly:
 *    >> leal Dee_FalseTrue(,%reg,8), %reg
 *       The fact that this can be done is the reason why a boolean
 *       doesn't store its value in its structure, but rather in its
 *       self-address.
 * WARNING: Only possible when `CONFIG_TRACE_REFCHANGES' is disabled! */
typedef struct {
	Dee_OBJECT_HEAD
} DeeBoolObject;

#define DeeBool_IsTrue(x)     (Dee_REQUIRES_OBJECT(DeeBoolObject, x) != &Dee_FalseTrue.bp_bools[0])
#define DeeBool_CheckTrue(x)  (Dee_REQUIRES_OBJECT(DeeBoolObject, x) == &Dee_FalseTrue.bp_bools[1])
#define DeeBool_CheckFalse(x) (Dee_REQUIRES_OBJECT(DeeBoolObject, x) == &Dee_FalseTrue.bp_bools[0])

typedef union _Dee_bool_pair {
	DeeBoolObject bp_bools[2];
} _DeeBool_Pair;

DDATDEF _DeeBool_Pair Dee_FalseTrue;
#define _DeeBool_For01(val) (&Dee_FalseTrue.bp_bools[val])
#endif /* !CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */

#define DeeBool_Check(x)      DeeObject_InstanceOfExact(x, &DeeBool_Type) /* `bool' is final. */
#define DeeBool_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBool_Type)
DDATDEF DeeTypeObject DeeBool_Type;

#ifdef __INTELLISENSE__
#define DeeBool_Incref(x) (void)Dee_AsObject(x)
#define DeeBool_Decref(x) (void)Dee_AsObject(x)
#define DeeBool_NewRef(x) (x)
#else /* __INTELLISENSE__ */
#define DeeBool_Incref(x) Dee_Incref(x)
#define DeeBool_NewRef(x) DeeObject_NewRef(x)
#ifdef CONFIG_EXPERIMENTAL_PER_THREAD_BOOL
#define DeeBool_Decref(x) Dee_Decref_unlikely(x)
#else /* CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */
#define DeeBool_Decref(x) Dee_DecrefNokill(x)
#endif /* !CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */
#endif /* !__INTELLISENSE__ */

/* These 2 are always linked to globals so they can be used in static initializers. */
#define Dee_False ((DeeObject *)&Dee_FalseTrue.bp_bools[0])
#define Dee_True  ((DeeObject *)&Dee_FalseTrue.bp_bools[1])

/* General-purpose macros... */
#define DeeBool_For01(val)  ((DeeObject *)_DeeBool_For01(val))
#define DeeBool_New01(val)  DeeBool_NewRef(DeeBool_For01(val))
#define DeeBool_NewFalse()  DeeBool_New01(0)
#define DeeBool_NewTrue()   DeeBool_New01(1)
#define Dee_return_bool01(val)                                            \
	do {                                                                  \
		__register DeeBoolObject *const _rb_result = _DeeBool_For01(val); \
		DeeBool_Incref(_rb_result);                                       \
		return (DREF DeeObject *)_rb_result;                              \
	}	__WHILE0

#define DeeBool_For(val)     DeeBool_For01(!!(val))
#define DeeBool_New(val)     DeeBool_New01(!!(val))
#define Dee_return_bool(val) Dee_return_bool01(!!(val))
#define Dee_return_false     Dee_return_bool01(0)
#define Dee_return_true      Dee_return_bool01(1)


DECL_END

#endif /* !GUARD_DEEMON_BOOL_H */
