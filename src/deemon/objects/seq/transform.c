/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C
#define GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C 1

#include "transform.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include <hybrid/minmax.h>

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif /* !CONFIG_NO_THREADS */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

PRIVATE NONNULL((1)) void DCALL
transiter_fini(TransformationIterator *__restrict self) {
	Dee_Decref(self->ti_iter);
	Dee_Decref(self->ti_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
transiter_visit(TransformationIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ti_iter);
	Dee_Visit(self->ti_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
transiter_bool(TransformationIterator *__restrict self) {
	return DeeObject_Bool(self->ti_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
transiter_next(TransformationIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->ti_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		/* Invoke the transformation callback. */
		new_result = DeeObject_Call(self->ti_func, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
transiter_seq_get(TransformationIterator *__restrict self) {
	/* Forward access to this attribute to the pointed-to iterator. */
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetAttr(self->ti_iter, (DeeObject *)&str_seq);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Transform(orig, self->ti_func);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst transiter_getsets[] = {
	TYPE_GETTER(STR_seq, &transiter_seq_get, "->?Ert:SeqTransformation"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst transiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(TransformationIterator, ti_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(TransformationIterator, ti_func), "->?DCallable"),
	TYPE_MEMBER_END
};

#define DEFINE_COMPARE(name, base, opname)                                               \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                \
	name(TransformationIterator *self, TransformationIterator *other) {                  \
		if (DeeObject_AssertTypeExact(other, &SeqTransformationIterator_Type))           \
			goto err;                                                                    \
		if (self->ti_func != other->ti_func) {                                           \
			int temp = DeeObject_CompareEq(self->ti_func, other->ti_func);               \
			if (temp <= 0) {                                                             \
				if (temp == 0)                                                           \
					err_unimplemented_operator(&SeqTransformationIterator_Type, opname); \
				goto err;                                                                \
			}                                                                            \
		}                                                                                \
		return base((DeeObject *)self, (DeeObject *)other);                              \
	err:                                                                                 \
		return NULL;                                                                     \
	}
DEFINE_COMPARE(transiter_eq, DeeObject_CompareEqObject, OPERATOR_EQ)
DEFINE_COMPARE(transiter_ne, DeeObject_CompareNeObject, OPERATOR_NE)
DEFINE_COMPARE(transiter_lo, DeeObject_CompareLoObject, OPERATOR_LO)
DEFINE_COMPARE(transiter_le, DeeObject_CompareLeObject, OPERATOR_LE)
DEFINE_COMPARE(transiter_gr, DeeObject_CompareGrObject, OPERATOR_GR)
DEFINE_COMPARE(transiter_ge, DeeObject_CompareGeObject, OPERATOR_GE)
#undef DEFINE_COMPARE

PRIVATE struct type_cmp transiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&transiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&transiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&transiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&transiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&transiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&transiter_ge
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
transiter_copy(TransformationIterator *__restrict self,
               TransformationIterator *__restrict other) {
	self->ti_iter = DeeObject_Copy(other->ti_iter);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = other->ti_func;
	Dee_Incref(self->ti_func);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
transiter_deep(TransformationIterator *__restrict self,
               TransformationIterator *__restrict other) {
	self->ti_iter = DeeObject_Copy(other->ti_iter);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = DeeObject_Copy(other->ti_func);
	if unlikely(!self->ti_func)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->ti_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
transiter_ctor(TransformationIterator *__restrict self) {
	self->ti_iter = DeeObject_IterSelf(Dee_EmptySeq);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
transiter_init(TransformationIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	Transformation *trans;
	if (DeeArg_Unpack(argc, argv, "o:_SeqTransformationIterator", &trans))
		goto err;
	if (DeeObject_AssertTypeExact(trans, &SeqTransformation_Type))
		goto err;
	self->ti_iter = DeeObject_IterSelf(trans->t_seq);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = trans->t_fun;
	Dee_Incref(self->ti_func);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject SeqTransformationIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqTransformationIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&transiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&transiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&transiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&transiter_init,
				TYPE_FIXED_ALLOCATOR(TransformationIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&transiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&transiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&transiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &transiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&transiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ transiter_getsets,
	/* .tp_members       = */ transiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE NONNULL((1)) void DCALL
trans_fini(Transformation *__restrict self) {
	Dee_Decref(self->t_seq);
	Dee_Decref(self->t_fun);
}

PRIVATE NONNULL((1, 2)) void DCALL
trans_visit(Transformation *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->t_seq);
	Dee_Visit(self->t_fun);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_bool(Transformation *__restrict self) {
	return DeeObject_Bool(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_iter(Transformation *__restrict self) {
	DREF TransformationIterator *result;
	result = DeeObject_MALLOC(TransformationIterator);
	if unlikely(!result)
		goto err;
	/* Create the underlying iterator. */
	result->ti_iter = DeeObject_IterSelf(self->t_seq);
	if unlikely(!result->ti_iter)
		goto err_r;
	/* Assign the transformation functions. */
	result->ti_func = self->t_fun;
	Dee_Incref(self->t_fun);
	DeeObject_Init(result, &SeqTransformationIterator_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_member tpconst trans_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Transformation, t_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(Transformation, t_fun), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst trans_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqTransformationIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_size(Transformation *__restrict self) {
	return DeeObject_SizeObject(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_getitem(Transformation *self,
              DeeObject *index) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetItem(self->t_seq, index);
	if unlikely(!orig)
		goto err;
	result = DeeObject_Call(self->t_fun, 1, &orig);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
trans_getrange(Transformation *__restrict self,
               DeeObject *__restrict start,
               DeeObject *__restrict end) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRange(self->t_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Transform(orig, self->t_fun);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
trans_nsi_getsize(Transformation *__restrict self) {
	return DeeObject_Size(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
trans_nsi_getsize_fast(Transformation *__restrict self) {
	return DeeFastSeq_GetSize(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_nsi_getitem(Transformation *__restrict self, size_t index) {
	DREF DeeObject *inner[1], *result;
	inner[0] = DeeObject_GetItemIndex(self->t_seq, index);
	if unlikely(!inner[0])
		goto err;
	result = DeeObject_Call(self->t_fun, 1, inner);
	Dee_Decref(inner[0]);
	return result;
err:
	return NULL;
}


PRIVATE struct type_nsi tpconst trans_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&trans_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&trans_nsi_getsize_fast,
			/* .nsi_getitem      = */ (dfunptr_t)&trans_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq trans_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&trans_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&trans_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&trans_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&trans_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &trans_nsi
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_ctor(Transformation *__restrict self) {
	self->t_seq = Dee_EmptySeq;
	self->t_fun = Dee_None;
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_copy(Transformation *__restrict self,
           Transformation *__restrict other) {
	self->t_seq = other->t_seq;
	self->t_fun = other->t_fun;
	Dee_Incref(self->t_seq);
	Dee_Incref(self->t_fun);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_deep(Transformation *__restrict self,
           Transformation *__restrict other) {
	self->t_seq = DeeObject_DeepCopy(other->t_seq);
	if unlikely(!self->t_seq)
		goto err;
	self->t_fun = DeeObject_DeepCopy(other->t_fun);
	if unlikely(!self->t_fun)
		goto err_seq;
	return 0;
err_seq:
	Dee_Decref(self->t_seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_init(Transformation *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_SeqTransformation", &self->t_seq, &self->t_fun))
		goto err;
	Dee_Incref(self->t_seq);
	Dee_Incref(self->t_fun);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject SeqTransformation_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqTransformation",
	/* .tp_doc      = */ DOC("(seq:?DSequence,fun:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&trans_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&trans_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&trans_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&trans_init,
				TYPE_FIXED_ALLOCATOR(Transformation)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&trans_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&trans_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&trans_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &trans_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ trans_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ trans_class_members
};



INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Transform(DeeObject *self,
                 DeeObject *transformation) {
	DREF Transformation *result;
	/* Create a new transformation sequence. */
	result = DeeObject_MALLOC(Transformation);
	if unlikely(!result)
		goto done;
	result->t_seq = self;
	result->t_fun = transformation;
	Dee_Incref(self);
	Dee_Incref(transformation);
	DeeObject_Init(result, &SeqTransformation_Type);
done:
	return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C */
