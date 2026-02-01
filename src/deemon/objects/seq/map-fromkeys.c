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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMKEYS_C
#define GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMKEYS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* Dee_True */
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>           /* DeeRT_ErrUnknownKey */
#include <deemon/map.h>                /* DeeMapping_Type */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint */
#include <deemon/none.h>               /* DeeNone_NewRef, Dee_None */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_BOUND_ERR, Dee_BOUND_FROMPRESENT_BOUND, Dee_Decref, Dee_Incref, Dee_foreach_pair_t, Dee_ssize_t, Dee_visit_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/seq.h>                /* DeeIterator_NewEmpty, DeeIterator_Type */
#include <deemon/set.h>                /* DeeSet_NewEmpty, DeeSet_Type, Dee_EmptySet */
#include <deemon/super.h>              /* DeeSuper_New */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, STRUCT_OBJECT, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "map-fromkeys.h"
#include "mapped.h"
#include "repeat.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

/************************************************************************/
/* MapFromKeysIterator                                                  */
/************************************************************************/
STATIC_ASSERT(offsetof(MapFromKeysIterator, mfki_iter) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(MapFromKeysIterator, mfki_base) == offsetof(ProxyObject2, po_obj2));
#define mfkvi_copy      generic_proxy2__copy_recursive1_alias2
#define mfkci_copy      generic_proxy2__copy_recursive1_alias2
#define mfkvi_deep      generic_proxy2__deepcopy
#define mfkci_deep      generic_proxy2__deepcopy
#define mfkvi_serialize generic_proxy2__serialize
#define mfkci_serialize generic_proxy2__serialize
#define mfkvi_fini      generic_proxy2__fini
#define mfkci_fini      generic_proxy2__fini
#define mfkvi_visit     generic_proxy2__visit
#define mfkci_visit     generic_proxy2__visit

STATIC_ASSERT(offsetof(MapFromKeysIterator, mfki_iter) == offsetof(ProxyObject, po_obj));
#define mfkvi_bool generic_proxy__bool
#define mfkci_bool generic_proxy__bool
#define mfkvi_cmp  generic_proxy__cmp_recursive
#define mfkci_cmp  generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfkvi_ctor(MapFromKeysIterator *__restrict self) {
	self->mfki_base = MapFromKeysAndValue_New(Dee_EmptySet, Dee_None);
	if unlikely(!self->mfki_base)
		goto err;
	self->mfki_iter = DeeIterator_NewEmpty();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfkci_ctor(MapFromKeysIterator *__restrict self) {
	self->mfki_base = MapFromKeysAndCallback_New(Dee_EmptySet, Dee_None);
	if unlikely(!self->mfki_base)
		goto err;
	self->mfki_iter = DeeIterator_NewEmpty();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfkvi_init(MapFromKeysIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_MapFromKeysAndValueIterator", &self->mfki_base);
	if (DeeObject_AssertTypeExact(self->mfki_base, &MapFromKeysAndValue_Type))
		goto err;
	self->mfki_iter = DeeObject_InvokeMethodHint(set_operator_iter, self->mfki_base->mfk_keys);
	if unlikely(!self->mfki_iter)
		goto err;
	Dee_Incref(self->mfki_base);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfkci_init(MapFromKeysIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_MapFromKeysAndCallbackIterator", &self->mfki_base);
	if (DeeObject_AssertTypeExact(self->mfki_base, &MapFromKeysAndCallback_Type))
		goto err;
	self->mfki_iter = DeeObject_InvokeMethodHint(set_operator_iter, self->mfki_base->mfk_keys);
	if unlikely(!self->mfki_iter)
		goto err;
	Dee_Incref(self->mfki_base);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(MapFromKeysIterator, mfki_iter) == offsetof(ProxyObject, po_obj));
#define mfkvi_nextkey generic_proxy__iter_next
#define mfkci_nextkey generic_proxy__iter_next
#define mfkvi_advance generic_proxy__iter_advance
#define mfkci_advance generic_proxy__iter_advance

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfkvi_nextpair(MapFromKeysIterator *__restrict self, DREF DeeObject *key_and_value[2]) {
	key_and_value[0] = DeeObject_IterNext(self->mfki_iter);
	if (!ITER_ISOK(key_and_value[0]))
		return likely(key_and_value[0]) ? 1 : -1;
	key_and_value[1] = self->mfki_base->mfk_value;
	Dee_Incref(key_and_value[1]);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfkci_nextpair(MapFromKeysIterator *__restrict self, DREF DeeObject *key_and_value[2]) {
	key_and_value[0] = DeeObject_IterNext(self->mfki_iter);
	if (!ITER_ISOK(key_and_value[0]))
		return likely(key_and_value[0]) ? 1 : -1;
	key_and_value[1] = DeeObject_Call(self->mfki_base->mfk_value, 1, key_and_value);
	if unlikely(!key_and_value[1])
		goto err_key;
	return 0;
err_key:
	Dee_Decref(key_and_value[0]);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mfkvi_nextvalue(MapFromKeysIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_IterNext(self->mfki_iter);
	if (ITER_ISOK(result)) {
		Dee_Decref(result);
		result = self->mfki_base->mfk_value;
		Dee_Incref(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mfkci_nextvalue(MapFromKeysIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_IterNext(self->mfki_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->mfki_base->mfk_value, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE struct type_iterator mfkvi_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&mfkvi_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkvi_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkvi_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&mfkvi_advance,
};

PRIVATE struct type_iterator mfkci_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&mfkci_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkci_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkci_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&mfkci_advance,
};


PRIVATE struct type_member tpconst mfkvi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapFromKeysIterator, mfki_base), "->?Ert:MapFromKeysAndValue"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapFromKeysIterator, mfki_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define mfkci_members mfkvi_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member tpconst mfkci_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapFromKeysIterator, mfki_base), "->?Ert:MapFromKeysAndCallback"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapFromKeysIterator, mfki_iter), "->?DIterator"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */


INTERN DeeTypeObject MapFromKeysAndValueIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromKeysAndValueIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?Ert:MapFromKeysAndValue)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ MapFromKeysIterator,
			/* tp_ctor:        */ &mfkvi_ctor,
			/* tp_copy_ctor:   */ &mfkvi_copy,
			/* tp_deep_ctor:   */ &mfkvi_deep,
			/* tp_any_ctor:    */ &mfkvi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mfkvi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mfkvi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mfkvi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mfkvi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &mfkvi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &mfkvi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mfkvi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject MapFromKeysAndCallbackIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromKeysAndCallbackIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?Ert:MapFromKeysAndCallback)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ MapFromKeysIterator,
			/* tp_ctor:        */ &mfkci_ctor,
			/* tp_copy_ctor:   */ &mfkci_copy,
			/* tp_deep_ctor:   */ &mfkci_deep,
			/* tp_any_ctor:    */ &mfkci_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mfkci_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mfkci_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mfkci_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mfkci_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &mfkci_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &mfkci_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mfkci_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};





/************************************************************************/
/* MapFromKeys                                                          */
/************************************************************************/
STATIC_ASSERT(offsetof(MapFromKeys, mfk_keys) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapFromKeys, mfk_keys) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapFromKeys, mfk_value) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapFromKeys, mfk_value) == offsetof(ProxyObject2, po_obj2));
#define mfkv_copy      generic_proxy2__copy_alias12
#define mfkc_copy      generic_proxy2__copy_alias12
#define mfkv_deep      generic_proxy2__deepcopy
#define mfkc_deep      generic_proxy2__deepcopy
#define mfkv_fini      generic_proxy2__fini
#define mfkc_fini      generic_proxy2__fini
#define mfkv_visit     generic_proxy2__visit
#define mfkc_visit     generic_proxy2__visit
#define mfkv_serialize generic_proxy2__serialize
#define mfkc_serialize generic_proxy2__serialize

STATIC_ASSERT(offsetof(MapFromKeys, mfk_keys) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(MapFromKeys, mfk_value) == offsetof(ProxyObject2, po_obj2));
#define mfkv_init generic_proxy2__init
#define mfkc_init generic_proxy2__init

STATIC_ASSERT(offsetof(MapFromKeys, mfk_keys) == offsetof(ProxyObject, po_obj));
#define mfkv_bool generic_proxy__seq_operator_bool
#define mfkc_bool generic_proxy__seq_operator_bool

#define mfkc_ctor mfkv_ctor
PRIVATE WUNUSED NONNULL((1)) int DCALL
mfkv_ctor(MapFromKeys *__restrict self) {
	self->mfk_keys  = DeeSet_NewEmpty();
	self->mfk_value = DeeNone_NewRef();
	return 0;
}

#define mfkc_iterkeys generic_proxy__set_operator_iter
#define mfkv_iterkeys generic_proxy__set_operator_iter
#define mfkc_keys     mfkv_keys
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mfkv_keys(MapFromKeys *__restrict self) {
	return DeeSuper_New(&DeeSet_Type, self->mfk_keys);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mfkv_values(MapFromKeys *__restrict self) {
	size_t size = DeeObject_InvokeMethodHint(set_operator_size, self->mfk_keys);
	if unlikely(size == (size_t)-1)
		goto err;
	return DeeSeq_RepeatItem(self->mfk_value, size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mfkc_values(MapFromKeys *__restrict self) {
	return DeeSeq_Map(self->mfk_keys, self->mfk_value);
}

PRIVATE struct type_member tpconst mfkv_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapFromKeysAndValueIterator_Type),
	TYPE_MEMBER_CONST(STR_Keys, &DeeSet_Type),
	TYPE_MEMBER_CONST(STR_Values, &SeqRepeatItem_Type),
	TYPE_MEMBER_CONST(STR_IterValues, &SeqRepeatItemIterator_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst mfkv_getsets[] = {
	TYPE_GETTER_AB(STR_keys, &mfkv_keys, "->?DSet"),
	TYPE_GETTER_AB(STR_values, &mfkv_values, "->?Ert:SeqRepeatItem"),
	TYPE_GETTER_AB(STR_iterkeys, &mfkv_iterkeys, "->?DIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst mfkc_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapFromKeysAndCallbackIterator_Type),
	TYPE_MEMBER_CONST(STR_Keys, &DeeSet_Type),
	TYPE_MEMBER_CONST(STR_Values, &SeqMapped_Type),
	TYPE_MEMBER_CONST(STR_IterValues, &SeqMappedIterator_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst mfkc_getsets[] = {
	TYPE_GETTER_AB(STR_keys, &mfkc_keys, "->?DSet"),
	TYPE_GETTER_AB(STR_values, &mfkc_values, "->?Ert:SeqMapped"),
	TYPE_GETTER_AB(STR_iterkeys, &mfkc_iterkeys, "->?DIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst mfkv_members[] = {
	TYPE_MEMBER_FIELD("__keys__", STRUCT_OBJECT, offsetof(MapFromKeys, mfk_keys)),
	TYPE_MEMBER_FIELD("__value__", STRUCT_OBJECT, offsetof(MapFromKeys, mfk_value)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst mfkc_members[] = {
	TYPE_MEMBER_FIELD("__keys__", STRUCT_OBJECT, offsetof(MapFromKeys, mfk_keys)),
	TYPE_MEMBER_FIELD_DOC("__valuefor__", STRUCT_OBJECT, offsetof(MapFromKeys, mfk_value), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF MapFromKeysIterator *DCALL
mfkv_iter(MapFromKeys *__restrict self) {
	DREF MapFromKeysIterator *result;
	result = DeeObject_MALLOC(MapFromKeysIterator);
	if unlikely(!result)
		goto err;
	result->mfki_iter = DeeObject_InvokeMethodHint(set_operator_iter, self->mfk_keys);
	if unlikely(!result->mfki_iter)
		goto err_r;
	result->mfki_base = self;
	Dee_Incref(self);
	DeeObject_Init(result, &MapFromKeysAndValueIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromKeysIterator *DCALL
mfkc_iter(MapFromKeys *__restrict self) {
	DREF MapFromKeysIterator *result;
	result = DeeObject_MALLOC(MapFromKeysIterator);
	if unlikely(!result)
		goto err;
	result->mfki_iter = DeeObject_InvokeMethodHint(set_operator_iter, self->mfk_keys);
	if unlikely(!result->mfki_iter)
		goto err_r;
	result->mfki_base = self;
	Dee_Incref(self);
	DeeObject_Init(result, &MapFromKeysAndCallbackIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


STATIC_ASSERT(offsetof(MapFromKeys, mfk_keys) == offsetof(ProxyObject, po_obj));
#define mfkv_sizeob   generic_proxy__set_operator_sizeob
#define mfkc_sizeob   generic_proxy__set_operator_sizeob
#define mfkv_contains generic_proxy__seq_operator_contains
#define mfkc_contains generic_proxy__seq_operator_contains
#define mfkv_size     generic_proxy__set_operator_size
#define mfkc_size     generic_proxy__set_operator_size
#define mfkv_hasitem  generic_proxy__seq_contains
#define mfkc_hasitem  generic_proxy__seq_contains

#define mfkc_bounditem mfkv_bounditem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfkv_bounditem(MapFromKeys *__restrict self, DeeObject *key) {
	int result = DeeObject_InvokeMethodHint(seq_contains, self->mfk_keys, key);
	if unlikely(result < 0)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(result);
err:
	return Dee_BOUND_ERR;
}

#define mfkc_delitem mfkv_delitem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfkv_delitem(MapFromKeys *__restrict self, DeeObject *key) {
	int result = DeeObject_InvokeMethodHint(set_remove, self->mfk_keys, key);
	if (result > 0)
		result = 0;
	return result;
}


struct mfkX_foreach_pair_data {
	Dee_foreach_pair_t mfkX_fpd_cb;    /* [1..1] Inner callback. */
	void              *mfkX_fpd_arg;   /* [?..?] Cookie for `mfkX_fpd_cb'. */
	DeeObject         *mfkX_fpd_value; /* [1..1] Value to yield for every possible key, or callback. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
mfkv_foreach_pair_cb(void *arg, DeeObject *key) {
	struct mfkX_foreach_pair_data *data = (struct mfkX_foreach_pair_data *)arg;
	return (*data->mfkX_fpd_cb)(data->mfkX_fpd_arg, key, data->mfkX_fpd_value);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mfkv_foreach_pair(MapFromKeys *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct mfkX_foreach_pair_data data;
	data.mfkX_fpd_cb    = cb;
	data.mfkX_fpd_arg   = arg;
	data.mfkX_fpd_value = self->mfk_value;
	return DeeObject_InvokeMethodHint(set_operator_foreach, self->mfk_keys,
	                                  &mfkv_foreach_pair_cb, &data);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
mfkc_foreach_pair_cb(void *arg, DeeObject *key) {
	Dee_ssize_t result;
	struct mfkX_foreach_pair_data *data = (struct mfkX_foreach_pair_data *)arg;
	DREF DeeObject *value = DeeObject_Call(data->mfkX_fpd_value, 1, &key);
	if unlikely(!value)
		goto err;
	result = (*data->mfkX_fpd_cb)(data->mfkX_fpd_arg, key, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mfkc_foreach_pair(MapFromKeys *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct mfkX_foreach_pair_data data;
	data.mfkX_fpd_cb    = cb;
	data.mfkX_fpd_arg   = arg;
	data.mfkX_fpd_value = self->mfk_value;
	return DeeObject_InvokeMethodHint(set_operator_foreach, self->mfk_keys,
	                                  &mfkc_foreach_pair_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfkv_getitem(MapFromKeys *self, DeeObject *key) {
	int result = DeeObject_InvokeMethodHint(seq_contains, self->mfk_keys, key);
	if unlikely(result <= 0) {
		if (result == 0)
			DeeRT_ErrUnknownKey(self, key);
		goto err;
	}
	return_reference_(self->mfk_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfkv_trygetitem(MapFromKeys *self, DeeObject *key) {
	int result = DeeObject_InvokeMethodHint(seq_contains, self->mfk_keys, key);
	if unlikely(result <= 0) {
		if (result == 0)
			return ITER_DONE;
		goto err;
	}
	return_reference_(self->mfk_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfkc_getitem(MapFromKeys *self, DeeObject *key) {
	int result = DeeObject_InvokeMethodHint(seq_contains, self->mfk_keys, key);
	if unlikely(result <= 0) {
		if (result == 0)
			DeeRT_ErrUnknownKey(self, key);
		goto err;
	}
	return DeeObject_Call(self->mfk_value, 1, &key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfkc_trygetitem(MapFromKeys *self, DeeObject *key) {
	int result = DeeObject_InvokeMethodHint(seq_contains, self->mfk_keys, key);
	if unlikely(result <= 0) {
		if (result == 0)
			return ITER_DONE;
		goto err;
	}
	return DeeObject_Call(self->mfk_value, 1, &key);
err:
	return NULL;
}



PRIVATE struct type_seq mfkv_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkv_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkv_sizeob,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfkv_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfkv_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfkv_delitem,
	/* .tp_setitem            = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange           = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&mfkv_foreach_pair,
	/* .tp_bounditem          = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfkv_bounditem,
	/* .tp_hasitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfkv_hasitem,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&mfkv_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__delitem_index__with__delitem),
	/* .tp_setitem_index      = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index     = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index     = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n   = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfkv_trygetitem,
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq mfkc_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkc_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfkc_sizeob,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfkc_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfkc_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfkc_delitem,
	/* .tp_setitem            = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange           = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&mfkc_foreach_pair,
	/* .tp_bounditem          = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfkc_bounditem,
	/* .tp_hasitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfkc_hasitem,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&mfkc_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__delitem_index__with__delitem),
	/* .tp_setitem_index      = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index     = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index     = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n   = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfkc_trygetitem,
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};


INTERN DeeTypeObject MapFromKeysAndValue_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromKeysAndValue",
	/* .tp_doc      = */ DOC("()\n"
	                         "(keys:?DSet,value)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ MapFromKeys,
			/* tp_ctor:        */ &mfkv_ctor,
			/* tp_copy_ctor:   */ &mfkv_copy,
			/* tp_deep_ctor:   */ &mfkv_deep,
			/* tp_any_ctor:    */ &mfkv_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mfkv_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mfkv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mfkv_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mfkv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__B29E727096DFDAFC),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &mfkv_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mfkv_getsets,
	/* .tp_members       = */ mfkv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mfkv_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject MapFromKeysAndCallback_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromKeysAndCallback",
	/* .tp_doc      = */ DOC("()\n"
	                         "(keys:?DSet,valuefor:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ MapFromKeys,
			/* tp_ctor:        */ &mfkc_ctor,
			/* tp_copy_ctor:   */ &mfkc_copy,
			/* tp_deep_ctor:   */ &mfkc_deep,
			/* tp_any_ctor:    */ &mfkc_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mfkc_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mfkc_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mfkc_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mfkc_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__B29E727096DFDAFC),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &mfkc_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mfkc_getsets,
	/* .tp_members       = */ mfkc_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mfkc_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMKEYS_C */
