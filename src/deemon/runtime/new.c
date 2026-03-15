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
#ifndef GUARD_DEEMON_RUNTIME_NEW_C
#define GUARD_DEEMON_RUNTIME_NEW_C 1

#include <deemon/api.h>

#ifdef CONFIG_EXPERIMENTAL_USE_TP_NEW

#include <deemon/alloc.h>  /* Dee_Free, Dee_TryCalloc */
#include <deemon/object.h> /* ASSERT_OBJECT, ASSERT_OBJECT_AT, DREF, DeeObject, DeeObject_Check, DeeTypeObject, Dee_Decref*, Dee_Incref*, Dee_REFTRACKER_UNTRACKED, Dee_TYPE, Dee_refcnt_t */

DECL_BEGIN

/* Return the relevant implementations of DeeObject_New() / DeeObject_NewKw() / DeeObject_Copy() */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_t DCALL
DeeType_RequireNew(DeeTypeObject *__restrict self) {
	/* TODO */
}

PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_kw_t DCALL
DeeType_RequireNewKw(DeeTypeObject *__restrict self) {
	/* TODO */
}

PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_copy_t DCALL
DeeType_RequireNewCopy(DeeTypeObject *__restrict self) {
	/* TODO */
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewDefault(DeeTypeObject *__restrict object_type) {
	/* TODO */
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_New(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv) {
	/* TODO */
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewKw(DeeTypeObject *object_type, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	/* TODO */
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Copy(DeeObject *__restrict self) {
	/* TODO */
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_TCopy(DeeTypeObject *tp_self, DeeObject *self) {
	/* TODO */
}



PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_NewTuple)(DeeTypeObject *object_type, DeeObject *args) {
	return DeeObject_New(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_NewTupleKw)(DeeTypeObject *object_type, DeeObject *args, DeeObject *kw) {
	return DeeObject_NewKw(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}


#ifdef __ARCH_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VNewPack, 12),
                    DCALL_ASSEMBLY_NAME(DeeObject_New, 12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *object_type,
                   size_t argc, va_list args) {
	return DeeObject_New(object_type, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* __ARCH_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *object_type,
                   size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_New(object_type, argc, DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !__ARCH_VA_LIST_IS_STACK_POINTER */

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VNewf(DeeTypeObject *object_type,
                char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_New(object_type,
	                       DeeTuple_SIZE(args_tuple),
	                       DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *
DeeObject_NewPack(DeeTypeObject *object_type, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VNewPack(object_type, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_Newf(DeeTypeObject *object_type,
               char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VNewf(object_type, format, args);
	va_end(args);
	return result;
}

DECL_END
#endif /* CONFIG_EXPERIMENTAL_USE_TP_NEW */

#endif /* !GUARD_DEEMON_RUNTIME_NEW_C */
