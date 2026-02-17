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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_C
#define GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack, UNPuSIZ */
#include <deemon/bool.h>               /* Dee_True */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_ErrIndexOutOfBounds */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_Decref*, Dee_Incref, Dee_foreach_t, Dee_ssize_t, Dee_visit_t, ITER_ISOK, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_NewEmpty, DeeSeq_Type, Dee_EmptySeq */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "segments.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
segiter_init(SegmentsIterator *__restrict self,
             size_t argc, DeeObject *const *argv) {
	self->si_len = 1;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":_SeqSegmentsIterator",
	                  &self->si_iter, &self->si_len))
		goto err;
	if unlikely(!self->si_len) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid length passed to `_SeqSegmentsIterator'");
		goto err;
	}
	Dee_Incref(self->si_iter);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
segiter_ctor(SegmentsIterator *__restrict self) {
	self->si_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->si_iter)
		goto err;
	self->si_len = 1;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
segiter_copy(SegmentsIterator *__restrict self,
             SegmentsIterator *__restrict other) {
	self->si_iter = DeeObject_Copy(other->si_iter);
	if unlikely(!self->si_iter)
		goto err;
	self->si_len = other->si_len;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(SegmentsIterator, si_iter) == offsetof(ProxyObject, po_obj));
#define segiter_serialize generic_proxy__serialize_and_memcpy
#define segiter_fini      generic_proxy__fini
#define segiter_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
segiter_next(SegmentsIterator *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *elem;
	size_t i;
	/* Read the first item to check  */
	elem = DeeObject_IterNext(self->si_iter);
	if (!ITER_ISOK(elem))
		return (DREF DeeTupleObject *)elem;
	result = DeeTuple_NewUninitialized(self->si_len);
	if unlikely(!result)
		goto err_elem;
	DeeTuple_SET(result, 0, elem);
	for (i = 1; i < self->si_len; ++i) {
		elem = DeeObject_IterNext(self->si_iter);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err_r_i;
			/* Premature end (aka. last segment)
			 * -> Must return a truncated result tuple. */
			return DeeTuple_TruncateUninitialized(result, i);
		}
		DeeTuple_SET(result, i, elem);
	}
	return result;
err_r_i:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
	return NULL;
err_elem:
	Dee_Decref(elem);
	return NULL;
}

STATIC_ASSERT(offsetof(SegmentsIterator, si_iter) == offsetof(ProxyObject, po_obj));
#define segiter_bool generic_proxy__bool

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
segiter_getseq(SegmentsIterator *__restrict self) {
	DREF DeeObject *base_seq, *result;
	base_seq = DeeObject_GetAttr(self->si_iter, Dee_AsObject(&str_seq));
	if unlikely(!base_seq)
		goto err;
	result = DeeSeq_Segments(base_seq, self->si_len);
	Dee_Decref_unlikely(base_seq);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst segiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &segiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:SeqSegments"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst segiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT_AB, offsetof(SegmentsIterator, si_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(SegmentsIterator, si_len)),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject SeqSegmentsIterator_Type;

STATIC_ASSERT(offsetof(SegmentsIterator, si_iter) == offsetof(ProxyObject, po_obj));
#define segiter_hash          generic_proxy__hash_recursive
#define segiter_compare_eq    generic_proxy__compare_eq_recursive
#define segiter_compare       generic_proxy__compare_recursive
#define segiter_trycompare_eq generic_proxy__trycompare_eq_recursive
#define segiter_cmp           generic_proxy__cmp_recursive

INTERN DeeTypeObject SeqSegmentsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSegmentsIterator",
	/* .tp_doc      = */ DOC("(iter?:?DIterator,len=!1)\n"
	                         "\n"
	                         "next->?DSequence"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SegmentsIterator,
			/* tp_ctor:        */ &segiter_ctor,
			/* tp_copy_ctor:   */ &segiter_copy,
			/* tp_any_ctor:    */ &segiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &segiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&segiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&segiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&segiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &segiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&segiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ segiter_getsets,
	/* .tp_members       = */ segiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



STATIC_ASSERT(offsetof(Segments, s_seq) == offsetof(SegmentsIterator, si_iter));
STATIC_ASSERT(offsetof(Segments, s_len) == offsetof(SegmentsIterator, si_len));
#define seg_copy      segiter_copy
#define seg_serialize segiter_serialize
#define seg_fini      segiter_fini
#define seg_visit     segiter_visit
#define seg_bool      segiter_bool

PRIVATE WUNUSED NONNULL((1)) int DCALL
seg_ctor(Segments *__restrict self) {
	self->s_seq = DeeSeq_NewEmpty();
	self->s_len = 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seg_init(Segments *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->s_len = 1;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":_SeqSegments",
	                  &self->s_seq, &self->s_len))
		goto err;
	if unlikely(!self->s_len) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid length passed to `_SeqSegments'");
		goto err;
	}
	Dee_Incref(self->s_seq);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF SegmentsIterator *DCALL
seg_iter(Segments *__restrict self) {
	DREF SegmentsIterator *result;
	result = DeeObject_MALLOC(SegmentsIterator);
	if unlikely(!result)
		goto done;
	result->si_iter = DeeObject_Iter(self->s_seq);
	if unlikely(!result->si_iter)
		goto err_r;
	result->si_len = self->s_len;
	DeeObject_Init(result, &SeqSegmentsIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
seg_size(Segments *__restrict self) {
	size_t result = DeeObject_Size(self->s_seq);
	if likely(result != (size_t)-1)
		result = (result + (self->s_len - 1)) / self->s_len;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
seg_size_fast(Segments *__restrict self) {
	size_t result = DeeObject_SizeFast(self->s_seq);
	if likely(result != (size_t)-1)
		result = (result + (self->s_len - 1)) / self->s_len;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
seg_getitem_index(Segments *__restrict self, size_t index) {
	size_t i;
	DREF DeeTupleObject *result;
	size_t start = index * self->s_len;
	size_t len   = DeeObject_Size(self->s_seq);
	if (len == (size_t)-1)
		goto err;
	if (start + self->s_len > len) {
		if unlikely(start >= len) {
			DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index,
			                          (len + (self->s_len - 1)) / self->s_len);
			goto err;
		}
		len -= start;
	} else {
		len = self->s_len;
	}
	result = DeeTuple_NewUninitialized(len);
	for (i = 0; i < len; ++i) {
		DREF DeeObject *temp;
		temp = DeeObject_GetItemIndex(self->s_seq, start + i);
		if unlikely(!temp) {
			if (DeeError_Catch(&DeeError_IndexError))
				return DeeTuple_TruncateUninitialized(result, i);
			goto err_r;
		}
		DeeTuple_SET(result, i, temp); /* Inherit reference */
	}
	return result;
err_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}


PRIVATE struct type_seq seg_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seg_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains), /* TODO */
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))NULL,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&seg_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&seg_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&seg_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE struct type_member tpconst seg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT_AB, offsetof(Segments, s_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Segments, s_len)),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject SeqSegments_Type;

PRIVATE WUNUSED NONNULL((1)) DREF Segments *DCALL
seg_get_frozen(Segments *__restrict self) {
	DREF DeeObject *inner_frozen;
	DREF Segments *result;
	inner_frozen = DeeObject_GetAttr(self->s_seq, Dee_AsObject(&str_frozen));
	if unlikely(!inner_frozen)
		goto err;
	if (inner_frozen == self->s_seq) {
		Dee_DecrefNokill(inner_frozen);
		return_reference_(self);
	}
	result = DeeObject_MALLOC(Segments);
	if unlikely(!result)
		goto err_inner;
	result->s_seq = inner_frozen; /* Inherit reference */
	result->s_len = self->s_len;
	DeeObject_Init(result, &SeqSegments_Type);
	return result;
err_inner:
	Dee_Decref(inner_frozen);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst seg_getsets[] = {
	TYPE_GETTER(STR_frozen, &seg_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst seg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqSegmentsIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqSegments_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeTuple_Type), /* s.a. `DeeTuple_NewUninitialized()' calls above */
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqSegments_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSegments",
	/* .tp_doc      = */ DOC("(seq?:?DSequence,len=!1)\n"
	                         "\n"
	                         "[](index:?Dint)->?DSequence"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ Segments,
			/* tp_ctor:        */ &seg_ctor,
			/* tp_copy_ctor:   */ &seg_copy,
			/* tp_any_ctor:    */ &seg_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &seg_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&seg_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&seg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__26B2EC529683DE3C),
	/* .tp_seq           = */ &seg_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ seg_getsets,
	/* .tp_members       = */ seg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ seg_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};




INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Segments(DeeObject *__restrict self, size_t segsize) {
	DREF Segments *result;
	ASSERT(segsize != 0);
	result = DeeObject_MALLOC(Segments);
	if unlikely(!result)
		goto done;
	result->s_seq = self;
	result->s_len = segsize;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqSegments_Type);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_C */
