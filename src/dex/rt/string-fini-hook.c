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
#ifndef GUARD_DEX_RT_STRING_FINI_HOOK_C
#define GUARD_DEX_RT_STRING_FINI_HOOK_C 1
#define DEE_SOURCE

#include "librt.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>        /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>          /* DeeArg_Unpack1 */
#include <deemon/error.h>        /* DeeError_Print, Dee_ERROR_PRINT_DOHANDLE */
#include <deemon/format.h>       /* DeeFormat_Printf */
#include <deemon/int.h>          /* DeeInt_NewUIntptr */
#include <deemon/object.h>       /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Decref*, Dee_Incref, Dee_WEAKREF_SUPPORT, Dee_WEAKREF_SUPPORT_ADDR, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, Dee_visit_t, Dee_weakref_support_fini, Dee_weakref_support_init, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/string.h>       /* DeeString*, Dee_string_fini_hook, Dee_string_fini_hook_decref */
#include <deemon/type.h>         /* DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, STRUCT_OBJECT, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_MEMBER_END, TYPE_MEMBER_FIELD_DOC, type_cmp, type_member */
#include <deemon/util/hash.h>    /* DeeObject_Id */
#include <deemon/util/weakref.h> /* Dee_WEAKREF, Dee_weakref_* */

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

typedef struct string_fini_hook_object StringFiniHook;
struct user_string_fini_hook {
	struct Dee_string_fini_hook usfh_hook; /* Underlying hook interface */
	Dee_WEAKREF(StringFiniHook) usfh_user; /* Weak reference to user-visible hook callback container */
};
#define user_string_fini_hook_fromhook(self) \
	COMPILER_CONTAINER_OF(self, struct user_string_fini_hook, usfh_hook)
#define user_string_fini_hook_alloc()    DeeObject_MALLOC(struct user_string_fini_hook)
#define user_string_fini_hook_free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(struct user_string_fini_hook *, self))

struct string_fini_hook_object {
	OBJECT_HEAD
	Dee_WEAKREF_SUPPORT
	DREF DeeObject                    *sfh_cb;   /* [1..1][const] User-defined callable that gets invoked. */
	DREF struct user_string_fini_hook *sfh_hook; /* [1..1][const] Finalization hook descriptor. */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
StringFiniHook_OnFini_impl(StringFiniHook *__restrict self,
                           DeeStringObject const *__restrict string) {
	uintptr_t string_id = DeeObject_Id(string);
	DREF DeeObject *result, *argv[1];
	argv[0] = DeeInt_NewUIntptr(string_id);
	if unlikely(!argv[0])
		goto err;
	result = DeeObject_Call(self->sfh_cb, 1, argv);
	Dee_Decref_likely(argv[0]); /* *_likely -> object IDs are usually large numbers, so probably wasn't shared */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely -> it's probably just "none" */
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
StringFiniHook_OnFini(StringFiniHook *__restrict self,
                      DeeStringObject const *__restrict string) {
	int error = StringFiniHook_OnFini_impl(self, string);
	if unlikely(error) {
		DeeError_Print("Unhandled exception in string finalization hook",
		               Dee_ERROR_PRINT_DOHANDLE);
	}
}


PRIVATE NONNULL((1)) void DCALL
user_string_fini_hook_destroy(struct Dee_string_fini_hook *__restrict self) {
	struct user_string_fini_hook *me = user_string_fini_hook_fromhook(self);
	Dee_weakref_fini(&me->usfh_user);
	user_string_fini_hook_free(me);
}

PRIVATE NONNULL((1, 2)) void DCALL
user_string_fini_hook_onfini(struct Dee_string_fini_hook *__restrict self,
                             DeeStringObject const *__restrict string) {
	DREF StringFiniHook *user;
	struct user_string_fini_hook *me = user_string_fini_hook_fromhook(self);
	user = (DREF StringFiniHook *)Dee_weakref_lock(&me->usfh_user);
	if likely(user) {
		StringFiniHook_OnFini(user, string);
		Dee_Decref_unlikely(user);
	}
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
sfh_init(StringFiniHook *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF struct user_string_fini_hook *hook;
	DeeArg_Unpack1(err, argc, argv, "_StringFiniHook", &self->sfh_cb);
	hook = user_string_fini_hook_alloc();
	if unlikely(!hook)
		goto err;
	Dee_weakref_support_init(self);
	Dee_weakref_init(&hook->usfh_user, Dee_AsObject(self), NULL);
	hook->usfh_hook.sfh_refcnt  = 1;
	hook->usfh_hook.sfh_destroy = &user_string_fini_hook_destroy;
	hook->usfh_hook.sfh_onfini  = &user_string_fini_hook_onfini;
	self->sfh_hook = hook; /* Inherit reference */
	Dee_Incref(self->sfh_cb);
	if unlikely(DeeString_AddFiniHook(&hook->usfh_hook))
		goto err_self_hook;
	return 0;
err_self_hook:
	Dee_DecrefNokill(self->sfh_cb);
	Dee_weakref_fini(&hook->usfh_user);
	Dee_weakref_support_fini(self);
	user_string_fini_hook_free(hook);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
sfh_fini(StringFiniHook *__restrict self) {
	DeeString_RemoveFiniHook(&self->sfh_hook->usfh_hook);
	Dee_string_fini_hook_decref(&self->sfh_hook->usfh_hook);
	Dee_weakref_support_fini(self);
	Dee_Decref(self->sfh_cb);
}

PRIVATE NONNULL((1, 2)) void DCALL
sfh_visit(StringFiniHook *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->sfh_cb);
}


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
sfh_hash(StringFiniHook *__restrict self) {
	return DeeObject_Hash(self->sfh_cb);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfh_compare_eq(StringFiniHook *lhs, StringFiniHook *rhs) {
	if (DeeObject_AssertType(rhs, &StringFiniHook_Type))
		goto err;
	return DeeObject_CompareEq(lhs->sfh_cb, rhs->sfh_cb);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfh_compare(StringFiniHook *lhs, StringFiniHook *rhs) {
	if (DeeObject_AssertType(rhs, &StringFiniHook_Type))
		goto err;
	return DeeObject_Compare(lhs->sfh_cb, rhs->sfh_cb);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfh_trycompare_eq(StringFiniHook *lhs, StringFiniHook *rhs) {
	if (DeeObject_AssertType(rhs, &StringFiniHook_Type))
		goto err;
	return DeeObject_TryCompareEq(lhs->sfh_cb, rhs->sfh_cb);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sfh_print(StringFiniHook *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<StringFiniHook %k>", self->sfh_cb);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sfh_printrepr(StringFiniHook *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "import(\"rt\").StringFiniHook(%r)", self->sfh_cb);
}


PRIVATE struct type_cmp sfh_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&sfh_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sfh_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&sfh_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&sfh_trycompare_eq,
};

PRIVATE struct type_member tpconst sfh_members[] = {
	TYPE_MEMBER_FIELD_DOC("__cb__", STRUCT_OBJECT, offsetof(StringFiniHook, sfh_cb), "->?DCallable"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject StringFiniHook_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringFiniHook",
	/* .tp_doc      = */ DOC("String finalization hook. The function registered with this hook "
	                         /**/ "gets called every time a ?Dstring object that had finalization "
	                         /**/ "hooks enabled is destroyed (as a result of no-one referencing it "
	                         /**/ "anymore). The hook remains valid for as long as it is referenced.\n"
	                         "The ?#__cb__ is invoked as ${__cb__(Object.id(<destroyed-string>))}\n"
	                         "Finalization hooks for strings are enabled using ?A__enable_fini_hooks__?Dstring\n"

	                         "(cb:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(StringFiniHook),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringFiniHook,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ &sfh_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Can't be serialized */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfh_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sfh_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sfh_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sfh_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sfh_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sfh_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_RT_STRING_FINI_HOOK_C */
