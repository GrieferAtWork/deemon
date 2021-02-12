/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_HASHFILTER_C
#define GUARD_DEEMON_OBJECTS_SEQ_HASHFILTER_C 1

#include "hashfilter.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>

#include "../../runtime/strings.h"

DECL_BEGIN

PRIVATE NONNULL((1)) void DCALL
filter_fini(HashFilter *__restrict self) {
	Dee_Decref(self->f_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
filter_visit(HashFilterIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->fi_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filteriterator_ctor(HashFilterIterator *__restrict self) {
	self->fi_iter = DeeObject_IterSelf(Dee_EmptySeq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_hash = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_filteriterator_init(HashFilterIterator *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	HashFilter *filter;
	if (DeeArg_Unpack(argc, argv, "o:_SeqHashFilterIterator", &filter))
		goto err;
	if (DeeObject_AssertTypeExact(filter, &SeqHashFilter_Type))
		goto err;
	self->fi_iter = DeeObject_IterSelf(filter->f_seq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_hash = filter->f_hash;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
map_filteriterator_init(HashFilterIterator *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	HashFilter *filter;
	if (DeeArg_Unpack(argc, argv, "o:_MappingHashFilterIterator", &filter))
		goto err;
	if (DeeObject_AssertTypeExact(filter, &MapHashFilter_Type))
		goto err;
	self->fi_iter = DeeObject_IterSelf(filter->f_seq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_hash = filter->f_hash;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filteriterator_copy(HashFilterIterator *__restrict self,
                    HashFilterIterator *__restrict other) {
	self->fi_iter = DeeObject_Copy(other->fi_iter);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_hash = other->fi_hash;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filteriterator_deep(HashFilterIterator *__restrict self,
                    HashFilterIterator *__restrict other) {
	self->fi_iter = DeeObject_DeepCopy(other->fi_iter);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_hash = other->fi_hash;
	return 0;
err:
	return -1;
}


STATIC_ASSERT(COMPILER_OFFSETOF(HashFilterIterator, fi_iter) == COMPILER_OFFSETOF(HashFilter, f_seq));
STATIC_ASSERT(COMPILER_OFFSETOF(HashFilterIterator, fi_hash) == COMPILER_OFFSETOF(HashFilter, f_hash));
#define filteriterator_fini  filter_fini
#define filteriterator_visit filter_visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_filteriterator_next(HashFilterIterator *__restrict self) {
	DREF DeeObject *result;
again:
	result = DeeObject_IterNext(self->fi_iter);
	if unlikely(!ITER_ISOK(result))
		goto done;
	/* Check if the hash matches. */
	if (DeeObject_Hash(result) != self->fi_hash) {
		Dee_Decref(result);
		goto again;
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_filteriterator_next(HashFilterIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *key_and_value[2];
	dhash_t key_hash;
again:
	result = DeeObject_IterNext(self->fi_iter);
	if unlikely(!ITER_ISOK(result))
		goto done;
	if (DeeObject_Unpack(result, 2, key_and_value))
		goto err_r;
	Dee_Decref(key_and_value[1]);
	key_hash = DeeObject_Hash(key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	/* Check if the hash matches. */
	if (key_hash != self->fi_hash) {
		Dee_Decref(result);
		goto again;
	}
done:
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
filteriterator_hash(HashFilterIterator *__restrict self) {
	return DeeObject_Hash(self->fi_iter) ^ self->fi_hash;
}

#define DEFINE_FILTERITERATOR_COMPARE(name, compare_object, more)          \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                  \
	name(HashFilterIterator *self, HashFilterIterator *other) {            \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                                      \
		more                                                               \
		return compare_object(self->fi_iter, other->fi_iter);              \
	err:                                                                   \
		return NULL;                                                       \
	}
DEFINE_FILTERITERATOR_COMPARE(filteriterator_eq, DeeObject_CompareEqObject, if (self->fi_hash != other->fi_hash) return_false;)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_ne, DeeObject_CompareNeObject, if (self->fi_hash != other->fi_hash) return_true;)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_lo, DeeObject_CompareLoObject, )
DEFINE_FILTERITERATOR_COMPARE(filteriterator_le, DeeObject_CompareLeObject, )
DEFINE_FILTERITERATOR_COMPARE(filteriterator_gr, DeeObject_CompareGrObject, )
DEFINE_FILTERITERATOR_COMPARE(filteriterator_ge, DeeObject_CompareGeObject, )
#undef DEFINE_FILTERITERATOR_COMPARE

PRIVATE struct type_cmp filteriterator_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&filteriterator_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filteriterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filteriterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filteriterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filteriterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filteriterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filteriterator_ge,
};


PRIVATE WUNUSED NONNULL((1)) DREF HashFilter *DCALL
filteriterator_seq_get(HashFilterIterator *__restrict self) {
	DREF HashFilter *result;
	DREF DeeObject *base_seq;
	DeeTypeObject *result_type;
	base_seq = DeeObject_GetAttr(self->fi_iter, &str_seq);
	if unlikely(!base_seq)
		goto err;
	result = DeeObject_MALLOC(HashFilter);
	if unlikely(!result)
		goto err_base_seq;
	result->f_seq  = base_seq; /* Inherit reference. */
	result->f_hash = self->fi_hash;
	result_type = Dee_TYPE(self) == &SeqHashFilterIterator_Type
	              ? &SeqHashFilter_Type
	              : &MapHashFilter_Type;
	DeeObject_Init(result, result_type);
	return result;
err_base_seq:
	Dee_Decref(base_seq);
err:
	return NULL;
}


PRIVATE struct type_getset seq_filteriterator_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filteriterator_seq_get,
	  NULL,
	  NULL,
	  DOC("->?Ert:SeqHashFilter") },
	{ NULL }
};

PRIVATE struct type_getset map_filteriterator_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filteriterator_seq_get,
	  NULL,
	  NULL,
	  DOC("->?Ert:MappingHashFilter") },
	{ NULL }
};

PRIVATE struct type_member filteriterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(HashFilterIterator, fi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD("__hash__", STRUCT_DHASH_T, offsetof(HashFilterIterator, fi_hash)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqHashFilterIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqHashFilterIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqHashFilter)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&filteriterator_ctor,
				/* .tp_copy_ctor = */ (void *)&filteriterator_copy,
				/* .tp_deep_ctor = */ (void *)&filteriterator_deep,
				/* .tp_any_ctor  = */ (void *)&seq_filteriterator_init,
				TYPE_FIXED_ALLOCATOR(HashFilterIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filteriterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filteriterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &filteriterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seq_filteriterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ seq_filteriterator_getsets,
	/* .tp_members       = */ filteriterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject MapHashFilterIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingHashFilterIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:MappingHashFilter)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&filteriterator_ctor,
				/* .tp_copy_ctor = */ (void *)&filteriterator_copy,
				/* .tp_deep_ctor = */ (void *)&filteriterator_deep,
				/* .tp_any_ctor  = */ (void *)&map_filteriterator_init,
				TYPE_FIXED_ALLOCATOR(HashFilterIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filteriterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filteriterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &filteriterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&map_filteriterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ map_filteriterator_getsets,
	/* .tp_members       = */ filteriterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) DREF HashFilterIterator *DCALL
filter_iter(HashFilter *__restrict self) {
	DREF HashFilterIterator *result;
	DeeTypeObject *result_type;
	result = DeeObject_MALLOC(HashFilterIterator);
	if unlikely(!result)
		goto done;
	result->fi_iter = DeeObject_IterSelf(self->f_seq);
	if unlikely(!result->fi_iter)
		goto err_r;
	result->fi_hash = self->f_hash;
	result_type = Dee_TYPE(self) == &SeqHashFilter_Type
	              ? &SeqHashFilterIterator_Type
	              : &MapHashFilterIterator_Type;
	DeeObject_Init(result, result_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE struct type_seq filter_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_member seq_filter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(HashFilter, f_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__hash__", STRUCT_DHASH_T, offsetof(HashFilter, f_hash)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member map_filter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(HashFilter, f_seq), "->?DMapping"),
	TYPE_MEMBER_FIELD("__hash__", STRUCT_DHASH_T, offsetof(HashFilter, f_hash)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member seq_filter_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqHashFilterIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member map_filter_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &MapHashFilterIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_ctor(HashFilter *__restrict self) {
	self->f_seq  = Dee_EmptySeq;
	self->f_hash = 0;
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filter_copy(HashFilter *__restrict self,
            HashFilter *__restrict other) {
	self->f_seq  = other->f_seq;
	self->f_hash = other->f_hash;
	Dee_Incref(self->f_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filter_deep(HashFilter *__restrict self,
            HashFilter *__restrict other) {
	self->f_seq = DeeObject_DeepCopy(other->f_seq);
	if unlikely(!self->f_seq)
		goto err;
	self->f_hash = other->f_hash;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_filter_init(HashFilter *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oIu:_SeqHashFilter",
	                  &self->f_seq, &self->f_hash))
		goto err;
	self->f_seq = Dee_EmptySeq;
	Dee_Incref(self->f_seq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
map_filter_init(HashFilter *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oIu:_MappingHashFilter",
	                  &self->f_seq, &self->f_hash))
		goto err;
	self->f_seq = Dee_EmptySeq;
	Dee_Incref(self->f_seq);
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject SeqHashFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqHashFilter",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,hash:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&filter_ctor,
				/* .tp_copy_ctor = */ (void *)&filter_copy,
				/* .tp_deep_ctor = */ (void *)&filter_deep,
				/* .tp_any_ctor  = */ (void *)&seq_filter_init,
				TYPE_FIXED_ALLOCATOR(HashFilter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &filter_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seq_filter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ seq_filter_class_members
};

INTERN DeeTypeObject MapHashFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingHashFilter",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,hash:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&filter_ctor,
				/* .tp_copy_ctor = */ (void *)&filter_copy,
				/* .tp_deep_ctor = */ (void *)&filter_deep,
				/* .tp_any_ctor  = */ (void *)&map_filter_init,
				TYPE_FIXED_ALLOCATOR(HashFilter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &filter_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ map_filter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ map_filter_class_members
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_HashFilter(DeeObject *self, Dee_hash_t hash) {
	DREF HashFilter *result;
	result = DeeObject_MALLOC(HashFilter);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	result->f_seq  = self;
	result->f_hash = hash;
	DeeObject_Init(result, &SeqHashFilter_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_HashFilter(DeeObject *self, Dee_hash_t hash) {
	DREF HashFilter *result;
	result = DeeObject_MALLOC(HashFilter);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	result->f_seq  = self;
	result->f_hash = hash;
	DeeObject_Init(result, &MapHashFilter_Type);
done:
	return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_HASHFILTER_C */
