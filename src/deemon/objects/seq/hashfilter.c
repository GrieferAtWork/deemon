/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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

STATIC_ASSERT(offsetof(HashFilter, f_seq) == offsetof(ProxyObject, po_obj));
#define filter_fini  generic_proxy_fini
#define filter_visit generic_proxy_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
filteriterator_ctor(HashFilterIterator *__restrict self) {
	self->fi_iter = DeeObject_Iter(Dee_EmptySeq);
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
	self->fi_iter = DeeObject_Iter(filter->f_seq);
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
	self->fi_iter = DeeObject_Iter(filter->f_seq);
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


STATIC_ASSERT(offsetof(HashFilterIterator, fi_iter) == offsetof(HashFilter, f_seq));
STATIC_ASSERT(offsetof(HashFilterIterator, fi_hash) == offsetof(HashFilter, f_hash));
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
	return Dee_HashCombine(DeeObject_Hash(self->fi_iter), self->fi_hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filteriterator_compare_eq(HashFilterIterator *self, HashFilterIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	if (self->fi_hash != other->fi_hash)
		return 1;
	return DeeObject_CompareEq(self->fi_iter, other->fi_iter);
err:
	return Dee_COMPARE_ERR;
}

STATIC_ASSERT(offsetof(HashFilterIterator, fi_iter) == offsetof(ProxyObject, po_obj));
#define filteriterator_compare generic_proxy_compare_recursive

PRIVATE struct type_cmp filteriterator_cmp = {
	/* .tp_hash       = */ (dhash_t (DCALL *)(DeeObject *__restrict))&filteriterator_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&filteriterator_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&filteriterator_compare,
};


PRIVATE WUNUSED NONNULL((1)) DREF HashFilter *DCALL
filteriterator_seq_get(HashFilterIterator *__restrict self) {
	DREF HashFilter *result;
	DREF DeeObject *base_seq;
	DeeTypeObject *result_type;
	base_seq = DeeObject_GetAttr(self->fi_iter, (DeeObject *)&str_seq);
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


PRIVATE struct type_getset tpconst seq_filteriterator_getsets[] = {
	TYPE_GETTER_F(STR_seq, &filteriterator_seq_get, METHOD_FNOREFESCAPE, "->?Ert:SeqHashFilter"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst map_filteriterator_getsets[] = {
	TYPE_GETTER_F(STR_seq, &filteriterator_seq_get, METHOD_FNOREFESCAPE, "->?Ert:MappingHashFilter"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst filteriterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(HashFilterIterator, fi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD("__hash__", STRUCT_HASH_T, offsetof(HashFilterIterator, fi_hash)),
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
				/* .tp_ctor      = */ (dfunptr_t)&filteriterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filteriterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filteriterator_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&seq_filteriterator_init,
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
	/* .tp_iterator      = */ NULL,
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
				/* .tp_ctor      = */ (dfunptr_t)&filteriterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filteriterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filteriterator_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&map_filteriterator_init,
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
	/* .tp_iterator      = */ NULL,
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
	result->fi_iter = DeeObject_Iter(self->f_seq);
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
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_iter,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};

PRIVATE struct type_member tpconst seq_filter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(HashFilter, f_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__hash__", STRUCT_HASH_T, offsetof(HashFilter, f_hash)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst map_filter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(HashFilter, f_seq), "->?DMapping"),
	TYPE_MEMBER_FIELD("__hash__", STRUCT_HASH_T, offsetof(HashFilter, f_hash)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst seq_filter_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqHashFilterIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqHashFilter_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst map_filter_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapHashFilterIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &MapHashFilter_Type),
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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_SeqHashFilter",
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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_MappingHashFilter",
	                  &self->f_seq, &self->f_hash))
		goto err;
	self->f_seq = Dee_EmptySeq;
	Dee_Incref(self->f_seq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF HashFilter *DCALL
hashfilter_get_frozen(HashFilter *__restrict self) {
	DREF DeeObject *inner_frozen;
	DREF HashFilter *result;
	inner_frozen = DeeObject_GetAttr(self->f_seq, (DeeObject *)&str_frozen);
	if unlikely(!inner_frozen)
		goto err;
	if (inner_frozen == self->f_seq) {
		Dee_DecrefNokill(inner_frozen);
		return_reference_(self);
	}
	result = DeeObject_MALLOC(HashFilter);
	if unlikely(!result)
		goto err_inner;
	result->f_seq  = inner_frozen; /* Inherit reference */
	result->f_hash = self->f_hash;
	DeeObject_Init(result, Dee_TYPE(self));
	return result;
err_inner:
	Dee_Decref(inner_frozen);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst hashfilter_getsets[] = {
	TYPE_GETTER(STR_frozen, &hashfilter_get_frozen, "->?."),
	TYPE_GETSET_END
};

#define seq_filter_getsets hashfilter_getsets
#define map_filter_getsets hashfilter_getsets


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
				/* .tp_ctor      = */ (dfunptr_t)&filter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&seq_filter_init,
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
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ seq_filter_getsets,
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
				/* .tp_ctor      = */ (dfunptr_t)&filter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&map_filter_init,
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
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ map_filter_getsets,
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
