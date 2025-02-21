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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C
#define GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/gc.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/util/simple-hashset.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "default-api.h"

/**/
#include "unique-iterator.h"

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
	if (DeeArg_Unpack(argc, argv, "o:_DistinctIterator", &self->di_iter))
		goto err;
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

PRIVATE NONNULL((1)) void DCALL
di_fini(DistinctIterator *__restrict self) {
	Dee_Decref(self->di_iter);
	Dee_simple_hashset_with_lock_fini(&self->di_encountered);
}

PRIVATE NONNULL((1, 2)) void DCALL
di_visit(DistinctIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->di_iter);
	Dee_simple_hashset_with_lock_visit(&self->di_encountered, proc, arg);
}

PRIVATE NONNULL((1)) void DCALL
di_clear(DistinctIterator *__restrict self) {
	Dee_simple_hashset_with_lock_clear(&self->di_encountered);
}

PRIVATE struct type_gc di_gc = {
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
	iter_seq = DeeObject_GetAttr(self->di_iter, (DeeObject *)&str_seq);
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_init,
				TYPE_FIXED_ALLOCATOR_GC(DistinctIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_visit,
	/* .tp_gc            = */ &di_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_getsets,
	/* .tp_members       = */ di_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
	if (DeeArg_Unpack(argc, argv, "oo:_DistinctIteratorWithKey",
	                  &self->diwk_iter, &self->diwk_key))
		goto err;
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

PRIVATE NONNULL((1)) void DCALL
uqiwk_fini(DistinctIteratorWithKey *__restrict self) {
	Dee_Decref(self->diwk_iter);
	Dee_simple_hashset_with_lock_fini(&self->diwk_encountered);
	Dee_Decref(self->diwk_key);
}

PRIVATE NONNULL((1, 2)) void DCALL
uqiwk_visit(DistinctIteratorWithKey *__restrict self, dvisit_t proc, void *arg) {
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
	iter_seq = DeeObject_GetAttr(self->diwk_iter, (DeeObject *)&str_seq);
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&uqiwk_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&uqiwk_init,
				TYPE_FIXED_ALLOCATOR_GC(DistinctIteratorWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uqiwk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&uqiwk_visit,
	/* .tp_gc            = */ &uqiwk_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &uqiwk_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uqiwk_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ uqiwk_getsets,
	/* .tp_members       = */ uqiwk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



/************************************************************************/
/* DISTINCT SET W/ KEY                                                  */
/************************************************************************/
STATIC_ASSERT(offsetof(DistinctSetWithKey, dswk_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DistinctSetWithKey, dswk_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(DistinctSetWithKey, dswk_key) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DistinctSetWithKey, dswk_key) == offsetof(ProxyObject2, po_obj2));
#define dswk_copy  generic_proxy2__copy_alias12
#define dswk_deep  generic_proxy2__deepcopy
#define dswk_fini  generic_proxy2__fini
#define dswk_visit generic_proxy2__visit

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
	result->diwk_iter = DeeSeq_OperatorIter(self->dswk_seq);
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
	return (DREF DistinctIteratorWithKey *)DeeGC_Track((DREF DeeObject *)result);
err_r_iter:
	Dee_Decref(result->diwk_iter);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_seq dswk_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dswk_iter,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&dswk_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&dswk_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&dswk_init,
				TYPE_FIXED_ALLOCATOR(DistinctSetWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dswk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dswk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dswk_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dswk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dswk_class_members
};



/************************************************************************/
/* DISTINCT MAP                                                         */
/************************************************************************/
STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_iter) == offsetof(DistinctIterator, di_iter));
STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_encountered) == offsetof(DistinctIterator, di_encountered));
#define dmi_copy  di_copy
#define dmi_fini  di_fini
#define dmi_visit di_visit
#define dmi_gc    di_gc

STATIC_ASSERT(offsetof(DistinctMappingIterator, dmi_iter) == offsetof(ProxyObject, po_obj));
#define dmi_cmp generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1)) int DCALL
dmi_init(DistinctMappingIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	if (DeeArg_Unpack(argc, argv, "o:_DistinctMappingIterator", &self->dmi_iter))
		goto err;
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
	/* .tp_nextpair = */ (int (DCALL *)(DeeObject *__restrict, /*out*/ DREF DeeObject *[2]))&dmi_nextpair
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dmi_getseq(DistinctMappingIterator *__restrict self) {
	DREF DeeObject *iter_seq, *result;
	iter_seq = DeeObject_GetAttr(self->dmi_iter, (DeeObject *)&str_seq);
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
	/* .tp_name     = */ "_DistinctIterator",
	/* .tp_doc      = */ DOC("(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&dmi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&dmi_init,
				TYPE_FIXED_ALLOCATOR_GC(DistinctIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dmi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dmi_visit,
	/* .tp_gc            = */ &dmi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &dmi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &dmi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ dmi_getsets,
	/* .tp_members       = */ dmi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C */
