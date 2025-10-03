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
#ifndef GUARD_DEX_RT_SLAB_C
#define GUARD_DEX_RT_SLAB_C 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include "librt.h"

DECL_BEGIN

#ifndef Dee_SLAB_COUNT
#define Dee_SLAB_COUNT 0
#endif /* !Dee_SLAB_COUNT */


PRIVATE WUNUSED DREF SlabStatObject *DCALL ss_ctor(void) {
	size_t reqsize;
	DREF SlabStatObject *result;
	DREF SlabStatObject *new_result;
	result = (DREF SlabStatObject *)DeeObject_Mallocc(offsetof(SlabStatObject, st_stat.st_slabs),
	                                                  Dee_SLAB_COUNT, sizeof(DeeSlabInfo));
	if unlikely(!result)
		goto done;
	reqsize = DeeSlab_Stat(&result->st_stat,
	                       _Dee_MallococBufsize(offsetof(DeeSlabStat, st_slabs),
	                                            Dee_SLAB_COUNT, sizeof(DeeSlabInfo)));
	if unlikely(reqsize >
	            _Dee_MallococBufsize(offsetof(DeeSlabStat, st_slabs),
	                                 Dee_SLAB_COUNT, sizeof(DeeSlabInfo))) {
		size_t oldsize;
do_realloc_result:
		oldsize    = reqsize;
		new_result = (DREF SlabStatObject *)DeeObject_Realloc(result,
		                                                      offsetof(SlabStatObject, st_stat) +
		                                                      reqsize);
		if unlikely(!new_result)
			goto err_r;
		result  = new_result;
		reqsize = DeeSlab_Stat(&result->st_stat, reqsize);
		if unlikely(reqsize > oldsize)
			goto do_realloc_result;
	} else if unlikely(reqsize <
	                   _Dee_MallococBufsize(offsetof(DeeSlabStat, st_slabs),
	                                        Dee_SLAB_COUNT, sizeof(DeeSlabInfo))) {
		new_result = (DREF SlabStatObject *)DeeObject_TryRealloc(result,
		                                                         offsetof(SlabStatObject, st_stat) +
		                                                         reqsize);
		if likely(new_result)
			result = new_result;
	}
	DeeObject_Init(result, &SlabStat_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

#define SLABSTAT_DATASIZE(x) \
	_Dee_MallococBufsize(offsetof(DeeSlabStat, st_slabs), (x)->st_stat.st_slabcount, sizeof(DeeSlabInfo))

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
ss_hash(SlabStatObject *__restrict self) {
	return Dee_HashPtr(&self->st_stat, SLABSTAT_DATASIZE(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ss_compare(SlabStatObject *self, SlabStatObject *other) {
	int result;
	if (DeeObject_AssertTypeExact(other, &SlabStat_Type))
		goto err;
	Dee_return_compare_if_neT(size_t, SLABSTAT_DATASIZE(self), SLABSTAT_DATASIZE(other));
	result = memcmp(&self->st_stat, &other->st_stat, SLABSTAT_DATASIZE(self));
	return Dee_CompareFromDiff(result);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp ss_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&ss_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ss_compare,
};

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ss_size(SlabStatObject *__restrict self) {
	return self->st_stat.st_slabcount;
}

PRIVATE WUNUSED NONNULL((1)) DREF SlabInfoObject *DCALL
ss_getitem_index(SlabStatObject *__restrict self, size_t index) {
	DREF SlabInfoObject *result;
	if unlikely(index >= self->st_stat.st_slabcount) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->st_stat.st_slabcount);
		return NULL;
	}
	result = DeeObject_MALLOC(SlabInfoObject);
	if unlikely(!result)
		goto done;
	result->si_info = &self->st_stat.st_slabs[index];
	result->si_stat = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SlabInfo_Type);
done:
	return result;
}

PRIVATE struct type_seq ss_seq = {
	/* .tp_iter            = */ NULL,
	/* .tp_sizeob          = */ NULL,
	/* .tp_contains        = */ NULL,
	/* .tp_getitem         = */ NULL,
	/* .tp_delitem         = */ NULL,
	/* .tp_setitem         = */ NULL,
	/* .tp_getrange        = */ NULL,
	/* .tp_delrange        = */ NULL,
	/* .tp_setrange        = */ NULL,
	/* .tp_foreach         = */ NULL,
	/* .tp_foreach_pair    = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ (size_t (DCALL *)(DeeObject *__restrict))&ss_size,
	/* .tp_size_fast       = */ (size_t (DCALL *)(DeeObject *__restrict))&ss_size,
	/* .tp_getitem_index   = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ss_getitem_index,
};

PRIVATE struct type_member tpconst ss_class_members[] = {
	TYPE_MEMBER_CONST("item", &SlabInfo_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ss_printrepr(SlabStatObject *__restrict UNUSED(self),
             Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_PRINT(printer, arg, "import(\"rt\").SlabStat()");
}


INTERN DeeTypeObject SlabStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SlabStat",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ss_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ss_printrepr,

	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ss_cmp,
	/* .tp_seq           = */ &ss_seq,
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
	/* .tp_class_members = */ ss_class_members
};


PRIVATE NONNULL((1)) void DCALL
si_fini(SlabInfoObject *__restrict self) {
	Dee_Decref(self->si_stat);
}

PRIVATE NONNULL((1, 2)) void DCALL
si_visit(SlabInfoObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_stat);
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
si_print(SlabInfoObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg,
	                        "size: %" PRFuSIZ ", "
	                        "alloc: %" PRFuSIZ "/%" PRFuSIZ ", "
	                        "free: %" PRFuSIZ "/%" PRFuSIZ,
	                        self->si_info->si_itemsize,
	                        self->si_info->si_cur_alloc,
	                        self->si_info->si_max_alloc,
	                        self->si_info->si_cur_free,
	                        self->si_info->si_max_free);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
si_printrepr(SlabInfoObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	size_t index = self->si_info - self->si_stat->st_stat.st_slabs;
	return DeeFormat_Printf(printer, arg, "%r[%" PRFuSIZ "]", self->si_stat, index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
si_get_index(SlabInfoObject *__restrict self) {
	return DeeInt_NewSize((size_t)(self->si_info - self->si_stat->st_stat.st_slabs));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
si_get_slabsize(SlabInfoObject *__restrict self) {
	return DeeInt_NewSize(self->si_info->si_slabend -
	                      self->si_info->si_slabstart);
}

#define DEFINE_SLABINFO_FIELD_READER(field_name)               \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL         \
	si_get_##field_name(SlabInfoObject *__restrict self) {     \
		return DeeInt_NewSize(self->si_info->si_##field_name); \
	}
DEFINE_SLABINFO_FIELD_READER(itemsize)
DEFINE_SLABINFO_FIELD_READER(items_per_page)
DEFINE_SLABINFO_FIELD_READER(totalpages)
DEFINE_SLABINFO_FIELD_READER(totalitems)
DEFINE_SLABINFO_FIELD_READER(cur_alloc)
DEFINE_SLABINFO_FIELD_READER(max_alloc)
DEFINE_SLABINFO_FIELD_READER(cur_free)
DEFINE_SLABINFO_FIELD_READER(max_free)
DEFINE_SLABINFO_FIELD_READER(cur_fullpages)
DEFINE_SLABINFO_FIELD_READER(max_fullpages)
DEFINE_SLABINFO_FIELD_READER(cur_freepages)
DEFINE_SLABINFO_FIELD_READER(max_freepages)
DEFINE_SLABINFO_FIELD_READER(usedpages)
DEFINE_SLABINFO_FIELD_READER(tailpages)
#undef DEFINE_SLABINFO_FIELD_READER


PRIVATE struct type_member tpconst si_members[] = {
	TYPE_MEMBER_FIELD_DOC("__stat__", STRUCT_OBJECT, offsetof(SlabInfoObject, si_stat), "->?GSlabStat"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst si_getsets[] = {
#define DEFINE_FIELD(name, doc) TYPE_GETTER_F(#name, &si_get_##name, METHOD_FNOREFESCAPE, "->?Dint\n" doc)
	DEFINE_FIELD(slabsize, "Total size of the slab (in bytes)"),
	DEFINE_FIELD(itemsize, "Slab item size (in bytes)"),
	DEFINE_FIELD(items_per_page, "Number of items per page"),
	DEFINE_FIELD(totalpages, "Number of pages designated for this slab"),
	DEFINE_FIELD(totalitems, "Max number of items which may be allocated by the slab (same as @totalpages * @items_per_page)"),
	DEFINE_FIELD(cur_alloc, "Number of items (@itemsize-sized data blocks) currently allocated"),
	DEFINE_FIELD(max_alloc, "Max number of items that were ever allocated"),
	DEFINE_FIELD(cur_free, "Number of items in initialized pages currently marked as free"),
	DEFINE_FIELD(max_free, "Max number of items that were ever marked as free"),
	DEFINE_FIELD(cur_fullpages, "Number of initialized pages that currently are fully in use"),
	DEFINE_FIELD(max_fullpages, "Max number of initialized pages that were ever in use at the same time"),
	DEFINE_FIELD(cur_freepages, "Number of initialized pages containing unallocated items"),
	DEFINE_FIELD(max_freepages, "Max number of initialized pages containing unallocated items at any point int time"),
	DEFINE_FIELD(usedpages, "Number of pages which are currently being used (@cur_fullpages + @cur_freepages)"),
	DEFINE_FIELD(tailpages, "Number of pages which haven't been allocated, yet"),
#undef DEFINE_FIELD
	TYPE_GETTER_F("__index__", &si_get_index, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Index of @this slab within ?GSlabStat"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
si_hash(SlabInfoObject *self) {
	return Dee_HashPtr(self->si_info, sizeof(DeeSlabInfo));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
si_compare(SlabInfoObject *self, SlabInfoObject *other) {
	int result;
	if (DeeObject_AssertTypeExact(other, &SlabInfo_Type))
		goto err;
	result = memcmp(self->si_info, other->si_info, sizeof(DeeSlabInfo));
	return Dee_CompareFromDiff(result);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp si_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&si_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&si_compare,
};

INTERN DeeTypeObject SlabInfo_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SlabInfo",
	/* .tp_doc      = */ DOC("Element type for ?GSlabStat"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(SlabInfoObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&si_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&si_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&si_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&si_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &si_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ si_getsets,
	/* .tp_members       = */ si_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_RT_SLAB_C */
