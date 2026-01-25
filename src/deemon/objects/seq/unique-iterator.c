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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C
#define GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>               /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>                 /* DeeArg_Unpack1, DeeArg_Unpack2 */
#include <deemon/computed-operators.h>
#include <deemon/gc.h>                  /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK */
#include <deemon/map.h>                 /* DeeMapping_Type */
#include <deemon/method-hints.h>        /* DeeObject_InvokeMethodHint */
#include <deemon/object.h>
#include <deemon/operator-hints.h>      /* DeeType_RequireSupportedNativeOperator */
#include <deemon/seq.h>                 /* DeeIterator_Type */
#include <deemon/serial.h>              /* DeeSerial*, Dee_seraddr_t */
#include <deemon/set.h>                 /* DeeSet_Type */
#include <deemon/super.h>               /* DeeSuper_New */
#include <deemon/thread.h>              /* DeeThread_CheckInterrupt */
#include <deemon/util/simple-hashset.h> /* Dee_simple_hashset_with_lock_* */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "unique-iterator.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN



/************************************************************************/
/* DISTINCT SET                                                         */
/************************************************************************/
STATIC_ASSERT(offsetof(DistinctIterator, di_iter) == offsetof(ProxyObject, po_obj));
#define di_cmp generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_copy(DistinctIterator *__restrict self,
        DistinctIterator *__restrict other) {
	int result;
	result = Dee_simple_hashset_with_lock_copy(&self->di_encountered,
	                                           &other->di_encountered);
	if likely(result == 0) {
		self->di_iter = DeeObject_Copy(other->di_iter);
		if unlikely(!self->di_iter)
			goto err_encountered;
		self->di_tp_next = other->di_tp_next;
	}
	return result;
err_encountered:
	Dee_simple_hashset_with_lock_fini(&self->di_encountered);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_init(DistinctIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack1(err, argc, argv, "_DistinctIterator", &self->di_iter);
	itertyp = Dee_TYPE(self->di_iter);
	self->di_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->di_tp_next)
		return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
	Dee_Incref(self->di_iter);
	Dee_simple_hashset_with_lock_init(&self->di_encountered);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_serialize(DistinctIterator *__restrict self,
             DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DistinctIterator, field))
	int result = DeeSerial_PutObject(writer, ADDROF(di_iter), self->di_iter);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(di_tp_next), self->di_tp_next);
	if likely(result == 0)
		result = Dee_simple_hashset_with_lock_serialize(&self->di_encountered, writer, ADDROF(di_encountered));
	return result;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
di_fini(DistinctIterator *__restrict self) {
	Dee_Decref(self->di_iter);
	Dee_simple_hashset_with_lock_fini(&self->di_encountered);
}

PRIVATE NONNULL((1, 2)) void DCALL
di_visit(DistinctIterator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->di_iter);
	Dee_simple_hashset_with_lock_visit(&self->di_encountered, proc, arg);
}

PRIVATE NONNULL((1)) void DCALL
di_clear(DistinctIterator *__restrict self) {
	Dee_simple_hashset_with_lock_clear(&self->di_encountered);
}

PRIVATE struct type_gc tpconst di_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&di_clear
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_next(DistinctIterator *__restrict self) {
	DREF DeeObject *result;
	for (;;) {
		int exists;
		result = (*self->di_tp_next)(self->di_iter);
		if (!ITER_ISOK(result))
			break;
		exists = Dee_simple_hashset_with_lock_insert(&self->di_encountered, result);
		if (exists > 0)
			break;
		Dee_Decref(result);
		if unlikely(exists < 0)
			goto err;

		/* Must check for interrupts in case "iter" is something
		 * like "Sequence.repeatitem(item).operator iter()" */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE struct type_member tpconst uqiwk_members[] = {
	TYPE_MEMBER_FIELD("__key__", STRUCT_OBJECT, offsetof(DistinctIteratorWithKey, diwk_key)),
#define di_members  (uqiwk_members + 1)
#define dmi_members di_members
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(DistinctIteratorWithKey, diwk_iter)),
	TYPE_MEMBER_FIELD("__num_encountered__", STRUCT_ATOMIC | STRUCT_CONST | STRUCT_SIZE_T,
	                  offsetof(DistinctIteratorWithKey, diwk_encountered.shswl_set.shs_size)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_getseq(DistinctIterator *__restrict self) {
	DREF DeeObject *iter_seq, *result;
	iter_seq = DeeObject_GetAttr(self->di_iter, Dee_AsObject(&str_seq));
	if unlikely(!iter_seq)
		goto err;
	result = DeeSuper_New(&DeeSet_Type, iter_seq);
	Dee_Decref(iter_seq);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst di_getsets[] = {
	TYPE_GETTER(STR_seq, &di_getseq,
	            "->?DSet\n"
	            "Returns ${this.__iter__.seq as Set}"),
	/* TODO: "__encountered__->?DSet" (using a custom wrapper object) */
	TYPE_GETSET_END
};

INTERN DeeTypeObject DistinctIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DistinctIterator",
	/* .tp_doc      = */ DOC("(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DistinctIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &di_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_visit,
	/* .tp_gc            = */ &di_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_getsets,
	/* .tp_members       = */ di_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



/************************************************************************/
/* WITH KEY                                                             */
/************************************************************************/
STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_iter) == offsetof(DistinctIterator, di_iter));
STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_tp_next) == offsetof(DistinctIterator, di_tp_next));
STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_encountered) == offsetof(DistinctIterator, di_encountered));
#define uqiwk_clear di_clear
#define uqiwk_gc    di_gc

STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_iter) == offsetof(ProxyObject, po_obj));
#define uqiwk_cmp generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uqiwk_copy(DistinctIteratorWithKey *__restrict self,
           DistinctIteratorWithKey *__restrict other) {
	int result;
	result = Dee_simple_hashset_with_lock_copy(&self->diwk_encountered,
	                                           &other->diwk_encountered);
	if likely(result == 0) {
		Dee_Incref(other->diwk_iter);
		self->diwk_iter    = other->diwk_iter;
		self->diwk_tp_next = other->diwk_tp_next;
		Dee_Incref(other->diwk_key);
		self->diwk_key = other->diwk_key;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uqiwk_init(DistinctIteratorWithKey *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack2(err, argc, argv, "_DistinctIteratorWithKey",
	                &self->diwk_iter, &self->diwk_key);
	itertyp = Dee_TYPE(self->diwk_iter);
	self->diwk_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->diwk_tp_next)
		return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
	Dee_Incref(self->diwk_iter);
	Dee_Incref(self->diwk_key);
	Dee_simple_hashset_with_lock_init(&self->diwk_encountered);
	return 0;
err:
	return -1;
}

#if 1
STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_iter) == offsetof(DistinctIterator, di_iter));
STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_tp_next) == offsetof(DistinctIterator, di_tp_next));
STATIC_ASSERT(offsetof(DistinctIteratorWithKey, diwk_encountered) == offsetof(DistinctIterator, di_encountered));
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uqiwk_serialize(DistinctIteratorWithKey *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DistinctIteratorWithKey, field))
	int result = di_serialize((DistinctIterator *)self, writer, addr);
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, ADDROF(diwk_key), self->diwk_key);
	return result;
#undef ADDROF
}
#else
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uqiwk_serialize(DistinctIteratorWithKey *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DistinctIteratorWithKey, field))
	int result = DeeSerial_PutObject(writer, ADDROF(diwk_iter), self->diwk_iter);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(diwk_tp_next), self->diwk_tp_next);
	if likely(result == 0)
		result = Dee_simple_hashset_with_lock_serialize(&self->diwk_encountered, writer, ADDROF(diwk_encountered));
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, ADDROF(diwk_key), self->diwk_key);
	return result;
#undef ADDROF
}
#endif

PRIVATE NONNULL((1)) void DCALL
uqiwk_fini(DistinctIteratorWithKey *__restrict self) {
	Dee_Decref(self->diwk_iter);
	Dee_simple_hashset_with_lock_fini(&self->diwk_encountered);
	Dee_Decref(self->diwk_key);
}

PRIVATE NONNULL((1, 2)) void DCALL
uqiwk_visit(DistinctIteratorWithKey *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->diwk_iter);
	Dee_simple_hashset_with_lock_visit(&self->diwk_encountered, proc, arg);
	Dee_Visit(self->diwk_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uqiwk_next(DistinctIteratorWithKey *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *keyed_result;
	for (;;) {
		int exists;
		result = (*self->diwk_tp_next)(self->diwk_iter);
		if (!ITER_ISOK(result))
			break;
		keyed_result = DeeObject_Call(self->diwk_key, 1, &result);
		if unlikely(!keyed_result)
			goto err_r;
		exists = Dee_simple_hashset_with_lock_insert(&self->diwk_encountered, keyed_result);
		Dee_Decref_unlikely(keyed_result);
		if (exists > 0)
			break;
		Dee_Decref(result);
		if unlikely(exists < 0)
			goto err;

		/* Must check for interrupts in case "iter" is something
		 * like "Sequence.repeatitem(item).unique(x -> x.lower())" */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DistinctSetWithKey *DCALL
uqiwk_getseq(DistinctIteratorWithKey *__restrict self) {
	DREF DeeObject *iter_seq;
	DREF DistinctSetWithKey *result;
	iter_seq = DeeObject_GetAttr(self->diwk_iter, Dee_AsObject(&str_seq));
	if unlikely(!iter_seq)
		goto err;
	result = DeeObject_MALLOC(DistinctSetWithKey);
	if unlikely(!result)
		goto err_iter_seq;
	result->dswk_seq = iter_seq; /* Inherit reference */
	result->dswk_key = self->diwk_key;
	Dee_Incref(self->diwk_key);
	DeeObject_Init(result, &DistinctSetWithKey_Type);
	return result;
err_iter_seq:
	Dee_Decref(iter_seq);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst uqiwk_getsets[] = {
	TYPE_GETTER(STR_seq, &uqiwk_getseq,
	            "->?Ert:DistinctSetWithKey"),
	/* TODO: "__encountered__->?DSet" (using a custom wrapper object) */
	TYPE_GETSET_END
};

INTERN DeeTypeObject DistinctIteratorWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DistinctIteratorWithKey",
	/* .tp_doc      = */ DOC("(objWithNext,key:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DistinctIteratorWithKey,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &uqiwk_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &uqiwk_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &uqiwk_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uqiwk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&uqiwk_visit,
	/* .tp_gc            = */ &uqiwk_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &uqiwk_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uqiwk_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ uqiwk_getsets,
	/* .tp_members       = */ uqiwk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



/************************************************************************/
/* DISTINCT SET W/ KEY                                                  */
/************************************************************************/
STATIC_ASSERT(offsetof(DistinctSetWithKey, dswk_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DistinctSetWithKey, dswk_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(DistinctSetWithKey, dswk_key) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DistinctSetWithKey, dswk_key) == offsetof(ProxyObject2, po_obj2));
#define dswk_copy      generic_proxy2__copy_alias12
#define dswk_deep      generic_proxy2__deepcopy
#define dswk_fini      generic_proxy2__fini
#define dswk_visit     generic_proxy2__visit
#define dswk_serialize generic_proxy2__serialize

STATIC_ASSERT(offsetof(DistinctSetWithKey, dswk_seq) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(DistinctSetWithKey, dswk_key) == offsetof(ProxyObject2, po_obj2));
#define dswk_init generic_proxy2__init

PRIVATE WUNUSED NONNULL((1)) DREF DistinctIteratorWithKey *DCALL
dswk_iter(DistinctSetWithKey *__restrict self) {
	DeeTypeObject *itertyp;
	DREF DistinctIteratorWithKey *result;
	result = DeeGCObject_MALLOC(DistinctIteratorWithKey);
	if unlikely(!result)
		goto err;
	result->diwk_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->dswk_seq);
	if unlikely(!result->diwk_iter)
		goto err_r;
	itertyp = Dee_TYPE(result->diwk_iter);
	result->diwk_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!result->diwk_tp_next) {
		err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
		goto err_r_iter;
	}
	result->diwk_key = self->dswk_key;
	Dee_Incref(self->dswk_key);
	Dee_simple_hashset_with_lock_init(&result->diwk_encountered);
	DeeObject_Init(result, &DistinctIteratorWithKey_Type);
	return DeeGC_TRACK(DistinctIteratorWithKey, result);
err_r_iter:
	Dee_Decref(result->diwk_iter);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_seq dswk_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dswk_iter,
	/* .tp_sizeob       = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains     = */ DEFIMPL(&default__seq_operator_contains),
	/* .tp_getitem      = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem      = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem      = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach      = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem    = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem      = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size         = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
};

PRIVATE struct type_member tpconst dswk_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DistinctSetWithKey, dswk_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__key__", STRUCT_OBJECT, offsetof(DistinctSetWithKey, dswk_key), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst dswk_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DistinctIteratorWithKey_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DistinctSetWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DistinctSetWithKey",
	/* .tp_doc      = */ DOC("(objWithIter,key:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DistinctSetWithKey,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &dswk_copy,
			/* tp_deep_ctor:   */ &dswk_deep,
			/* tp_any_ctor:    */ &dswk_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dswk_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dswk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dswk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__F6E3D7B2219AE1EB),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__A5C53AFDF1233C5A),
	/* .tp_seq           = */ &dswk_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dswk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dswk_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



/************************************************************************/
/* DISTINCT MAP                                                         */
/************************************************************************/
STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_iter) == offsetof(DistinctIterator, di_iter));
STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_tp_nextpair) == offsetof(DistinctIterator, di_tp_next));
STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_encountered) == offsetof(DistinctIterator, di_encountered));
#define dmi_serialize di_serialize
#define dmi_copy      di_copy
#define dmi_fini      di_fini
#define dmi_visit     di_visit
#define dmi_gc        di_gc

STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_iter) == offsetof(ProxyObject, po_obj));
#define dmi_cmp generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1)) int DCALL
dmi_init(DistinctMappingIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack1(err, argc, argv, "_DistinctMappingIterator", &self->dmi_iter);
	itertyp = Dee_TYPE(self->dmi_iter);
	self->dmi_tp_nextpair = DeeType_RequireSupportedNativeOperator(itertyp, nextpair);
	if unlikely(!self->dmi_tp_nextpair)
		return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
	Dee_Incref(self->dmi_iter);
	Dee_simple_hashset_with_lock_init(&self->dmi_encountered);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dmi_nextpair(DistinctMappingIterator *__restrict self,
             /*out*/ DREF DeeObject *key_and_value[2]) {
	int result;
again:
	result = (*self->dmi_tp_nextpair)(self->dmi_iter, key_and_value);
	if likely(result == 0) {
		int exists = Dee_simple_hashset_with_lock_insert(&self->dmi_encountered, key_and_value[0]);
		if unlikely(exists) {
			Dee_Decref(key_and_value[1]);
			Dee_Decref(key_and_value[0]);
			if unlikely(exists < 0)
				goto err;
			goto again;
		}
	}
	return result;
err:
	return -1;
}

PRIVATE struct type_iterator dmi_iterator = {
	/* .tp_nextpair = */ (int (DCALL *)(DeeObject *__restrict, /*out*/ DREF DeeObject *[2]))&dmi_nextpair,
	/* .tp_nextkey   = */ DEFIMPL(&default__nextkey__with__nextpair),
	/* .tp_nextvalue = */ DEFIMPL(&default__nextvalue__with__nextpair),
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextpair),
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dmi_getseq(DistinctMappingIterator *__restrict self) {
	DREF DeeObject *iter_seq, *result;
	iter_seq = DeeObject_GetAttr(self->dmi_iter, Dee_AsObject(&str_seq));
	if unlikely(!iter_seq)
		goto err;
	result = DeeSuper_New(&DeeMapping_Type, iter_seq);
	Dee_Decref(iter_seq);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst dmi_getsets[] = {
	TYPE_GETTER(STR_seq, &dmi_getseq,
	            "->?DMapping\n"
	            "Returns ${this.__iter__.seq as Mapping}"),
	/* TODO: "__encountered_keys__->?DSet" (using a custom wrapper object) */
	TYPE_GETSET_END
};

INTERN DeeTypeObject DistinctMappingIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DistinctMappingIterator",
	/* .tp_doc      = */ DOC("(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DistinctMappingIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &dmi_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &dmi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dmi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dmi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dmi_visit,
	/* .tp_gc            = */ &dmi_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &dmi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &dmi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ dmi_getsets,
	/* .tp_members       = */ dmi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C */
