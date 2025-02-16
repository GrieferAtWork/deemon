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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_REMOVEIF_CB_C
#define GUARD_DEEMON_OBJECTS_SEQ_REMOVEIF_CB_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/format.h>

/**/
#include "removeif-cb.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
srwrip_init(SeqRemoveWithRemoveIfPredicate *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicate", &self->srwrip_item))
		goto err;
	Dee_Incref(self->srwrip_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
srwripwk_init(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_SeqRemoveWithRemoveIfPredicateWithKey",
	                  &self->srwripwk_item, &self->srwripwk_key))
		goto err;
	Dee_Incref(self->srwripwk_item);
	Dee_Incref(self->srwripwk_key);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
srwrip_fini(SeqRemoveWithRemoveIfPredicate *__restrict self) {
	Dee_Decref(self->srwrip_item);
}

PRIVATE NONNULL((1, 2)) void DCALL
srwrip_visit(SeqRemoveWithRemoveIfPredicate *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->srwrip_item);
}

PRIVATE NONNULL((1)) void DCALL
srwripwk_fini(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self) {
	Dee_Decref(self->srwripwk_item);
	Dee_Decref(self->srwripwk_key);
}

PRIVATE NONNULL((1, 2)) void DCALL
srwripwk_visit(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->srwripwk_item);
	Dee_Visit(self->srwripwk_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
srwrip_call(SeqRemoveWithRemoveIfPredicate *self, size_t argc, DeeObject *const *argv) {
	int equals;
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicate", &item))
		goto err;
	equals = DeeObject_TryCompareEq(self->srwrip_item, item);
	if unlikely(equals == Dee_COMPARE_ERR)
		goto err;
	return_bool_(equals == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
srwripwk_call(SeqRemoveWithRemoveIfPredicateWithKey *self, size_t argc, DeeObject *const *argv) {
	int equals;
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicateWithKey", &item))
		goto err;
	equals = DeeObject_TryCompareKeyEq(self->srwripwk_item, item, self->srwripwk_key);
	if unlikely(equals == Dee_COMPARE_ERR)
		goto err;
	return_bool_(equals == 0);
err:
	return NULL;
}

STATIC_ASSERT(offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_item) ==
              offsetof(SeqRemoveWithRemoveIfPredicate, srwrip_item));
#define srwrip_members (srwripwk_members + 1)
PRIVATE struct type_member tpconst srwripwk_members[] = {
	TYPE_MEMBER_FIELD("__item__", STRUCT_OBJECT, offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_item)),
	TYPE_MEMBER_FIELD("__key__", STRUCT_OBJECT, offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_key)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqRemoveWithRemoveIfPredicate_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveWithRemoveIfPredicate",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&srwrip_init,
				TYPE_FIXED_ALLOCATOR(SeqRemoveWithRemoveIfPredicate)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srwrip_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&srwrip_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&srwrip_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ srwrip_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

INTERN DeeTypeObject SeqRemoveWithRemoveIfPredicateWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveWithRemoveIfPredicateWithKey",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&srwripwk_init,
				TYPE_FIXED_ALLOCATOR(SeqRemoveWithRemoveIfPredicateWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srwripwk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&srwripwk_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&srwripwk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ srwripwk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};









PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_removeif_with_removeall_item_compare_eq(DeeObject *self, DeeObject *should_result) {
	int result;
	(void)self;
	result = DeeObject_Bool(should_result);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_removeif_with_removeall_item_eq(DeeObject *self, DeeObject *should_result) {
	(void)self;
	return_reference_(should_result);
}

PRIVATE struct type_cmp seq_removeif_with_removeall_item_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ &seq_removeif_with_removeall_item_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &seq_removeif_with_removeall_item_compare_eq,
	/* .tp_eq            = */ &seq_removeif_with_removeall_item_eq,
};

INTERN DeeTypeObject SeqRemoveIfWithRemoveAllItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveIfWithRemoveAllItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &seq_removeif_with_removeall_item_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

INTERN DeeObject SeqRemoveIfWithRemoveAllItem_DummyInstance = {
	OBJECT_HEAD_INIT(&SeqRemoveIfWithRemoveAllItem_Type)
};

STATIC_ASSERT(offsetof(SeqRemoveIfWithRemoveAllKey, sriwrak_should) == offsetof(ProxyObject, po_obj));
#define seq_removeif_with_removeall_key_init  generic_proxy__init
#define seq_removeif_with_removeall_key_fini  generic_proxy__fini
#define seq_removeif_with_removeall_key_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_removeif_with_removeall_key_printrepr(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                          Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "rt.SeqRemoveIfWithRemoveAllKey(%r)", self->sriwrak_should);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_removeif_with_removeall_key_call(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveIfWithRemoveAllKey", &item))
		goto err;
	if (item == &SeqRemoveIfWithRemoveAllItem_DummyInstance)
		return_reference_(item);
	return DeeObject_Call(self->sriwrak_should, argc, argv);
err:
	return NULL;
}

INTERN DeeTypeObject SeqRemoveIfWithRemoveAllKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveIfWithRemoveAllKey",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&seq_removeif_with_removeall_key_init,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *))&seq_removeif_with_removeall_key_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&seq_removeif_with_removeall_key_printrepr,
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&seq_removeif_with_removeall_key_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *, dvisit_t, void *))&seq_removeif_with_removeall_key_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_REMOVEIF_CB_C */
