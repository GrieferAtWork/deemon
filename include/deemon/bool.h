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
/*!export Dee_cached_dict_**/
/*!export DeeCachedDict**/
/*!export Dee_bool_object*/
/*!export DeeBool**/
/*!export _DeeBool**/
#ifndef GUARD_DEEMON_BOOL_H
#define GUARD_DEEMON_BOOL_H 1 /*!export-*/

#include "api.h"

#ifndef __INTELLISENSE__
#include "object.h" /* Dee_DecrefNokill, Dee_Decref_unlikely, Dee_Incref */
#endif /* !__INTELLISENSE__ */
#include "types.h" /* DREF, DeeObject, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_REQUIRES_OBJECT */

DECL_BEGIN

#ifdef DEE_SOURCE
#define return_bool   Dee_return_bool   /*!export*/
#define return_bool01 Dee_return_bool01 /*!export*/
#define return_true   Dee_return_true   /*!export*/
#define return_false  Dee_return_false  /*!export*/
#endif /* DEE_SOURCE */


/* HINT: In i386 assembly, `bool' is 8 bytes, so if you want to
 *       convert an integer 0/1 into a boolean, you can use the
 *       following assembly:
 *    >> leal Dee_FalseTrue(,%reg,8), %reg
 *       The fact that this can be done is the reason why a boolean
 *       doesn't store its value in its structure, but rather in its
 *       self-address.
 * WARNING: Only possible when `CONFIG_TRACE_REFCHANGES' is disabled! */
typedef struct Dee_bool_object {
	Dee_OBJECT_HEAD
} DeeBoolObject;

#define DeeBool_Check(x)      DeeObject_InstanceOfExact(x, &DeeBool_Type) /* `bool' is final. */
#define DeeBool_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBool_Type)
#define DeeBool_IsTrue(x)     (Dee_REQUIRES_OBJECT(DeeBoolObject, x) != &Dee_FalseTrue[0])
#ifdef CONFIG_EXPERIMENTAL_PER_THREAD_BOOL
#define DeeBool_CheckTrue(x)  (DeeBool_Check(x) && DeeBool_IsTrue(x))
#define DeeBool_CheckFalse(x) (DeeBool_Check(x) && !DeeBool_IsTrue(x))
#else /* CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */
#define DeeBool_CheckTrue(x)  (Dee_REQUIRES_OBJECT(DeeBoolObject, x) == &Dee_FalseTrue[1])
#define DeeBool_CheckFalse(x) (Dee_REQUIRES_OBJECT(DeeBoolObject, x) == &Dee_FalseTrue[0])
#endif /* !CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */

#ifdef __INTELLISENSE__
#define DeeBool_Incref(x) (void)Dee_AsObject(x)
#define DeeBool_Decref(x) (void)Dee_AsObject(x)
#else /* __INTELLISENSE__ */
#define DeeBool_Incref(x) Dee_Incref(x)
#ifdef CONFIG_EXPERIMENTAL_PER_THREAD_BOOL
#define DeeBool_Decref(x) Dee_Decref_unlikely(x)
#else /* CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */
#define DeeBool_Decref(x) Dee_DecrefNokill(x)
#endif /* !CONFIG_EXPERIMENTAL_PER_THREAD_BOOL */
#endif /* !__INTELLISENSE__ */

DDATDEF DeeTypeObject DeeBool_Type;
DDATDEF DeeBoolObject Dee_FalseTrue[2]; /*!export*/
#define _DeeBool_False      (&Dee_FalseTrue[0])
#define _DeeBool_True       (&Dee_FalseTrue[1])
#define _DeeBool_For(val)   (&Dee_FalseTrue[!!(val)])
#define _DeeBool_For01(val) (&Dee_FalseTrue[val])
#define Dee_False           ((DeeObject *)_DeeBool_False) /*!export*/
#define Dee_True            ((DeeObject *)_DeeBool_True)  /*!export*/
#define DeeBool_For(val)    ((DeeObject *)_DeeBool_For(val))
#define DeeBool_For01(val)  ((DeeObject *)_DeeBool_For01(val))
#define DeeBool_NewFalse()  (DeeBool_Incref(_DeeBool_False), (DeeObject *)_DeeBool_False)
#define DeeBool_NewTrue()   (DeeBool_Incref(_DeeBool_True), (DeeObject *)_DeeBool_True)
#define DeeBool_New(val)    (DeeBool_Incref(_DeeBool_For(val)), (DeeObject *)_DeeBool_For(val))
#define DeeBool_New01(val)  (DeeBool_Incref(_DeeBool_For01(val)), (DeeObject *)_DeeBool_For01(val))
#define Dee_return_bool(val)                                                     \
	do {                                                                         \
		__register struct Dee_bool_object *const _rb_result = _DeeBool_For(val); \
		DeeBool_Incref(_rb_result);                                              \
		return (DREF DeeObject *)_rb_result;                                     \
	}	__WHILE0
#define Dee_return_bool01(val)                                                     \
	do {                                                                           \
		__register struct Dee_bool_object *const _rb_result = _DeeBool_For01(val); \
		DeeBool_Incref(_rb_result);                                                \
		return (DREF DeeObject *)_rb_result;                                       \
	}	__WHILE0
#define Dee_return_true  Dee_return_bool01(1)
#define Dee_return_false Dee_return_bool01(0)

DECL_END

#endif /* !GUARD_DEEMON_BOOL_H */
