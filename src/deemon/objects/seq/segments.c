/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_C
#define GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *s_seq;   /* [1..1][const] The underlying sequence that is being segmented. */
	size_t          s_len;   /* [const][!0] The (max) length of a single segment. */
} Segments;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *si_iter; /* [1..1][const] An iterator for the sequence being segmented. */
	size_t          si_len;  /* [const][!0] The (max) length of a single segment. */
} SegmentsIterator;


PRIVATE int DCALL
segiter_init(SegmentsIterator *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	self->si_len = 1;
	if (DeeArg_Unpack(argc, argv, "o|Iu:_SeqSegmentsIterator",
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

PRIVATE int DCALL
segiter_ctor(SegmentsIterator *__restrict self) {
	self->si_iter = DeeObject_IterSelf(Dee_EmptySeq);
	if unlikely(!self->si_iter)
		goto err;
	self->si_len = 1;
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
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

PRIVATE int DCALL
segiter_deep(SegmentsIterator *__restrict self,
             SegmentsIterator *__restrict other) {
	self->si_iter = DeeObject_DeepCopy(other->si_iter);
	if unlikely(!self->si_iter)
		goto err;
	self->si_len = other->si_len;
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
segiter_fini(SegmentsIterator *__restrict self) {
	Dee_Decref(self->si_iter);
}

PRIVATE void DCALL
segiter_visit(SegmentsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_iter);
}

PRIVATE DREF DeeObject *DCALL
segiter_next(SegmentsIterator *__restrict self) {
	DREF DeeObject *elem, *result;
	size_t i;
	/* Read the first item to check  */
	elem = DeeObject_IterNext(self->si_iter);
	if (!ITER_ISOK(elem))
		return elem;
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
	while (i--)
		Dee_Decref(DeeTuple_GET(result, i));
	DeeTuple_FreeUninitialized(result);
	return NULL;
err_elem:
	Dee_Decref(elem);
	return NULL;
}

PRIVATE int DCALL
segiter_bool(SegmentsIterator *__restrict self) {
	return DeeObject_Bool(self->si_iter);
}


PRIVATE DREF DeeObject *DCALL
segiter_getseq(SegmentsIterator *__restrict self) {
	DREF DeeObject *base_seq, *result;
	base_seq = DeeObject_GetAttr(self->si_iter, &str_seq);
	if unlikely(!base_seq)
		goto err;
	result = DeeSeq_Segments(base_seq, self->si_len);
	Dee_Decref_unlikely(base_seq);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset segiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & segiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:SeqSegments") },
	{ NULL }
};

PRIVATE struct type_member segiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SegmentsIterator, si_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(SegmentsIterator, si_len)),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject SeqSegmentsIterator_Type;

#define DEFINE_SEGITER_COMPARE(name, func)                               \
	PRIVATE DREF DeeObject *DCALL                                        \
	name(SegmentsIterator *__restrict self,                              \
	     SegmentsIterator *__restrict other) {                           \
		if (DeeObject_AssertTypeExact(other, &SeqSegmentsIterator_Type)) \
			return NULL;                                                 \
		return func(self->si_iter, other->si_iter);                      \
	}
DEFINE_SEGITER_COMPARE(segiter_eq, DeeObject_CompareEqObject)
DEFINE_SEGITER_COMPARE(segiter_ne, DeeObject_CompareNeObject)
DEFINE_SEGITER_COMPARE(segiter_lo, DeeObject_CompareLoObject)
DEFINE_SEGITER_COMPARE(segiter_le, DeeObject_CompareLeObject)
DEFINE_SEGITER_COMPARE(segiter_gr, DeeObject_CompareGrObject)
DEFINE_SEGITER_COMPARE(segiter_ge, DeeObject_CompareGeObject)
#undef DEFINE_SEGITER_COMPARE

PRIVATE struct type_cmp segiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&segiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&segiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&segiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&segiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&segiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&segiter_ge
};


INTERN DeeTypeObject SeqSegmentsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSegmentsIterator",
	/* .tp_doc      = */ DOC("(iter?:?DIterator,len=!1)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&segiter_ctor,
				/* .tp_copy_ctor = */ (void *)&segiter_copy,
				/* .tp_deep_ctor = */ (void *)&segiter_deep,
				/* .tp_any_ctor  = */ (void *)&segiter_init,
				TYPE_FIXED_ALLOCATOR(SegmentsIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&segiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&segiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&segiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &segiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&segiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */segiter_getsets,
	/* .tp_members       = */segiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE struct type_member seg_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqSegmentsIterator_Type),
	TYPE_MEMBER_END
};


STATIC_ASSERT(COMPILER_OFFSETOF(Segments, s_seq) ==
              COMPILER_OFFSETOF(SegmentsIterator, si_iter));
STATIC_ASSERT(COMPILER_OFFSETOF(Segments, s_len) ==
              COMPILER_OFFSETOF(SegmentsIterator, si_len));
#define seg_copy     segiter_copy
#define seg_deep     segiter_deep
#define seg_fini     segiter_fini
#define seg_visit    segiter_visit
#define seg_bool     segiter_bool

PRIVATE int DCALL
seg_ctor(Segments *__restrict self) {
	self->s_seq = Dee_EmptySeq;
	self->s_len = 1;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE int DCALL
seg_init(Segments *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
	self->s_len = 1;
	if (DeeArg_Unpack(argc, argv, "o|Iu:_SeqSegments",
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


PRIVATE DREF SegmentsIterator *DCALL
seg_iter(Segments *__restrict self) {
	DREF SegmentsIterator *result;
	result = DeeObject_MALLOC(SegmentsIterator);
	if unlikely(!result)
		goto done;
	result->si_iter = DeeObject_IterSelf(self->s_seq);
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

PRIVATE size_t DCALL
seg_nsi_getsize(Segments *__restrict self) {
	size_t result;
	result = DeeObject_Size(self->s_seq);
	if likely(result != (size_t)-1)
		result = (result + (self->s_len - 1)) / self->s_len;
	return result;
}

PRIVATE DREF DeeObject *DCALL
seg_getsize(Segments *__restrict self) {
	size_t result;
	result = DeeObject_Size(self->s_seq);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize((result + (self->s_len - 1)) / self->s_len);
err:
	return NULL;
}

PRIVATE size_t DCALL
seg_nsi_fast_getsize(Segments *__restrict self) {
	size_t result;
	result = DeeFastSeq_GetSize(self->s_seq);
	if likely(result != (size_t)-1)
		result = (result + (self->s_len - 1)) / self->s_len;
	return result;
}

PRIVATE DREF DeeObject *DCALL
seg_nsi_getitem(Segments *__restrict self, size_t index) {
	size_t i;
	DREF DeeObject *result;
	size_t start = index * self->s_len;
	size_t len   = DeeObject_Size(self->s_seq);
	if (len == (size_t)-1)
		goto err;
	if (start + self->s_len > len) {
		if (start >= len) {
			err_index_out_of_bounds((DeeObject *)self,
			                        index,
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
	while (i--)
		Dee_Decref(DeeTuple_GET(result, i));
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
seg_getitem(Segments *__restrict self, DeeObject *__restrict index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return seg_nsi_getitem(self, index);
err:
	return NULL;
}


PRIVATE struct type_nsi seg_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&seg_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&seg_nsi_fast_getsize,
			/* .nsi_getitem      = */ (void *)&seg_nsi_getitem,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)NULL,
			/* .nsi_rfind        = */ (void *)NULL,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL,
			/* .nsi_removeif     = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq seg_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seg_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seg_getsize,
	/* .tp_contains  = */ NULL, /* TODO */
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&seg_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &seg_nsi,
};

PRIVATE struct type_member seg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Segments, s_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Segments, s_len)),
	TYPE_MEMBER_END
};

PRIVATE DeeTypeObject SeqSegments_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSegments",
	/* .tp_doc      = */ DOC("(seq?:?DSequence,len=!1)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&seg_ctor,
				/* .tp_copy_ctor = */ (void *)&seg_copy,
				/* .tp_deep_ctor = */ (void *)&seg_deep,
				/* .tp_any_ctor  = */ (void *)&seg_init,
				TYPE_FIXED_ALLOCATOR(Segments)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&seg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&seg_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&seg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &seg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ seg_class_members
};




INTERN DREF DeeObject *DCALL
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
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_C */
