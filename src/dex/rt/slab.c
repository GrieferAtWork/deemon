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
#ifndef GUARD_DEX_RT_SLAB_C
#define GUARD_DEX_RT_SLAB_C 1
#define DEE_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include "librt.h"

DECL_BEGIN

#ifndef Dee_SLAB_COUNT
#define Dee_SLAB_COUNT 0
#endif /* !Dee_SLAB_COUNT */


PRIVATE DREF SlabStatObject *DCALL ss_ctor(void) {
	size_t reqsize;
	DREF SlabStatObject *result;
	DREF SlabStatObject *new_result;
	result = (DREF SlabStatObject *)DeeObject_Malloc(COMPILER_OFFSETOF(SlabStatObject, st_stat.st_slabs) +
	                                                 Dee_SLAB_COUNT * sizeof(DeeSlabInfo));
	if
		unlikely(!result)
	goto done;
	reqsize = DeeSlab_Stat(&result->st_stat,
	                       COMPILER_OFFSETOF(DeeSlabStat, st_slabs) +
	                       (Dee_SLAB_COUNT * sizeof(DeeSlabInfo)));
	if
		unlikely(reqsize >
		         COMPILER_OFFSETOF(DeeSlabStat, st_slabs) +
		         (Dee_SLAB_COUNT * sizeof(DeeSlabInfo)))
	{
		size_t oldsize;
do_realloc_result:
		oldsize    = reqsize;
		new_result = (DREF SlabStatObject *)DeeObject_Realloc(result,
		                                                      COMPILER_OFFSETOF(SlabStatObject, st_stat) +
		                                                      reqsize);
		if
			unlikely(!new_result)
		goto err_r;
		result  = new_result;
		reqsize = DeeSlab_Stat(&result->st_stat, reqsize);
		if
			unlikely(reqsize > oldsize)
		goto do_realloc_result;
	} else if unlikely(reqsize <
	                 COMPILER_OFFSETOF(DeeSlabStat, st_slabs) +
	                 (Dee_SLAB_COUNT * sizeof(DeeSlabInfo)))
	{
		new_result = (DREF SlabStatObject *)DeeObject_TryRealloc(result,
		                                                         COMPILER_OFFSETOF(SlabStatObject, st_stat) +
		                                                         reqsize);
		if
			likely(new_result)
		result = new_result;
	}
	DeeObject_Init(result, &SlabStat_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

#define SLABSTAT_DATASIZE(x)                    \
	(COMPILER_OFFSETOF(DeeSlabStat, st_slabs) + \
	 ((x)->st_stat.st_slabcount * sizeof(DeeSlabInfo)))

PRIVATE dhash_t DCALL
ss_hash(SlabStatObject *__restrict self) {
	return Dee_HashPtr(&self->st_stat, SLABSTAT_DATASIZE(self));
}

#define DEFINE_SS_COMPARE(name, op, return_diff_size)                                       \
	PRIVATE DREF DeeObject *DCALL                                                           \
	name(SlabStatObject *__restrict self,                                                   \
	     SlabStatObject *__restrict other) {                                                \
		if (DeeObject_AssertTypeExact(other, &SlabStat_Type))                               \
			return NULL;                                                                    \
		if (SLABSTAT_DATASIZE(self) != SLABSTAT_DATASIZE(other))                            \
			return_diff_size;                                                               \
		return_bool(memcmp(&self->st_stat, &other->st_stat, SLABSTAT_DATASIZE(self)) op 0); \
	}
DEFINE_SS_COMPARE(ss_eq, ==, return_false)
DEFINE_SS_COMPARE(ss_ne, !=, return_true)
DEFINE_SS_COMPARE(ss_lo, <, return_bool_(SLABSTAT_DATASIZE(self) < SLABSTAT_DATASIZE(other)))
DEFINE_SS_COMPARE(ss_le, <=, return_bool_(SLABSTAT_DATASIZE(self) < SLABSTAT_DATASIZE(other)))
DEFINE_SS_COMPARE(ss_gr, >, return_bool_(SLABSTAT_DATASIZE(self) > SLABSTAT_DATASIZE(other)))
DEFINE_SS_COMPARE(ss_ge, >=, return_bool_(SLABSTAT_DATASIZE(self) > SLABSTAT_DATASIZE(other)))
#undef DEFINE_SS_COMPARE

PRIVATE struct type_cmp ss_cmp = {
	/* .tp_hash = */ (dhash_t(DCALL *)(DeeObject *__restrict))&ss_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_ge
};

PRIVATE size_t DCALL
ss_nsi_getsize(SlabStatObject *__restrict self) {
	return self->st_stat.st_slabcount;
}

PRIVATE DREF SlabInfoObject *DCALL
ss_nsi_getitem(SlabStatObject *__restrict self, size_t index) {
	DREF SlabInfoObject *result;
	if
		unlikely(index >= self->st_stat.st_slabcount)
	{
		DeeError_Throwf(&DeeError_IndexError,
		                "Index `%Iu' lies outside the valid bounds `0...%Iu' of sequence of type `%k'",
		                index, self->st_stat.st_slabcount, Dee_TYPE(self));
		return NULL;
	}
	result = DeeObject_MALLOC(SlabInfoObject);
	if
		unlikely(!result)
	goto done;
	result->si_info = &self->st_stat.st_slabs[index];
	result->si_stat = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SlabInfo_Type);
done:
	return result;
}

PRIVATE DREF SlabStatIteratorObject *DCALL
ss_iter(SlabStatObject *__restrict self) {
	DREF SlabStatIteratorObject *result;
	result = DeeObject_MALLOC(SlabStatIteratorObject);
	if
		unlikely(!result)
	goto done;
	result->sti_stat  = self;
	result->sti_index = 0;
	Dee_Incref(self);
	DeeObject_Init(result, &SlabStatIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
ss_size(SlabStatObject *__restrict self) {
	return DeeInt_NewSize(self->st_stat.st_slabcount);
}

PRIVATE DREF SlabInfoObject *DCALL
ss_getitem(SlabStatObject *__restrict self,
           DeeObject *__restrict index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return ss_nsi_getitem(self, index);
err:
	return NULL;
}

PRIVATE struct type_nsi ss_nsi = {
	/* .nsi_class = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&ss_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&ss_nsi_getsize,
			/* .nsi_getitem      = */ (void *)&ss_nsi_getitem,
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

PRIVATE struct type_seq ss_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ss_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &ss_nsi
};

PRIVATE struct type_member ss_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SlabStatIterator_Type),
	TYPE_MEMBER_CONST("item", &SlabInfo_Type),
	TYPE_MEMBER_END
};

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
				/* .tp_ctor      = */ (void *)&ss_ctor,
				/* .tp_copy_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (void *)NULL,
				/* .tp_free      = */ (void *)NULL
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
	/* .tp_cmp           = */ &ss_cmp,
	/* .tp_seq           = */ &ss_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ss_class_members
};



#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)            ((x)->sti_index)
#else /* CONFIG_NO_THREADS */
#define READ_INDEX(x) ATOMIC_READ((x)->sti_index)
#endif /* !CONFIG_NO_THREADS */

PRIVATE int DCALL
ssi_copy(SlabStatIteratorObject *__restrict self,
         SlabStatIteratorObject *__restrict other) {
	self->sti_index = READ_INDEX(other);
	self->sti_stat  = other->sti_stat;
	Dee_Incref(self->sti_stat);
	return 0;
}

PRIVATE void DCALL
ssi_fini(SlabStatIteratorObject *__restrict self) {
	Dee_Decref(self->sti_stat);
}

PRIVATE int DCALL
ssi_bool(SlabStatIteratorObject *__restrict self) {
	return READ_INDEX(self) < self->sti_stat->st_stat.st_slabcount;
}

#define DEFINE_SSI_COMPARE(name, op)                                  \
	PRIVATE DREF DeeObject *DCALL                                     \
	name(SlabStatIteratorObject *__restrict self,                     \
	     SlabStatIteratorObject *__restrict other) {                  \
		if (DeeObject_AssertTypeExact(other, &SlabStatIterator_Type)) \
			return NULL;                                              \
		return_bool(READ_INDEX(self) op READ_INDEX(other));           \
	}
DEFINE_SSI_COMPARE(ssi_eq, ==)
DEFINE_SSI_COMPARE(ssi_ne, !=)
DEFINE_SSI_COMPARE(ssi_lo, <)
DEFINE_SSI_COMPARE(ssi_le, <=)
DEFINE_SSI_COMPARE(ssi_gr, >)
DEFINE_SSI_COMPARE(ssi_ge, >=)
#undef DEFINE_SSI_COMPARE

PRIVATE struct type_cmp ssi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ssi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ssi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ssi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ssi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ssi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&ssi_ge
};


PRIVATE DREF SlabInfoObject *DCALL
ssi_next(SlabStatIteratorObject *__restrict self) {
	DREF SlabInfoObject *result;
	size_t index;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
		index = READ_INDEX(self);
		if (index >= self->sti_stat->st_stat.st_slabcount)
			return (DREF SlabInfoObject *)ITER_DONE;
#ifndef CONFIG_NO_THREADS
		if (ATOMIC_CMPXCH_WEAK(self->sti_index, index, index + 1))
			break;
#else  /* !CONFIG_NO_THREADS */
		self->sti_index index + 1;
#endif /* CONFIG_NO_THREADS */
	}
	result = DeeObject_MALLOC(SlabInfoObject);
	if
		unlikely(!result)
	goto done;
	result->si_stat = self->sti_stat;
	result->si_info = &result->si_stat->st_stat.st_slabs[index];
	Dee_Incref(result->si_stat);
	DeeObject_Init(result, &SlabInfo_Type);
done:
	return result;
}


PRIVATE struct type_member ssi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SlabStatIteratorObject, sti_stat), "->?GSlabStat"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(SlabStatIteratorObject, sti_stat)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SlabStatIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SlabStatIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&ssi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(SlabStatIteratorObject)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&ssi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&ssi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL, /* No visit, because only non-GC objects are reachable */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ssi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ssi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

STATIC_ASSERT(COMPILER_OFFSETOF(SlabStatIteratorObject, sti_stat) ==
              COMPILER_OFFSETOF(SlabInfoObject, si_stat));
#define si_fini  ssi_fini

PRIVATE DREF DeeObject *DCALL
si_str(SlabInfoObject *__restrict self) {
	return DeeString_Newf("size: %Iu, alloc: %Iu/%Iu, free: %Iu/%Iu",
	                      self->si_info->si_itemsize,
	                      self->si_info->si_cur_alloc,
	                      self->si_info->si_max_alloc,
	                      self->si_info->si_cur_free,
	                      self->si_info->si_max_free);
}

PRIVATE DREF DeeObject *DCALL
si_repr(SlabInfoObject *__restrict self) {
	return DeeString_Newf("<slabinfo %k>", self);
}

PRIVATE DREF DeeObject *DCALL
si_get_index(SlabInfoObject *__restrict self) {
	return DeeInt_NewSize((size_t)(self->si_info - self->si_stat->st_stat.st_slabs));
}

PRIVATE DREF DeeObject *DCALL
si_get_slabsize(SlabInfoObject *__restrict self) {
	return DeeInt_NewSize(self->si_info->si_slabend -
	                      self->si_info->si_slabstart);
}

#define DEFINE_FIELD_READER(field_name)                        \
	PRIVATE DREF DeeObject *DCALL                              \
	si_get_##field_name(SlabInfoObject *__restrict self) {     \
		return DeeInt_NewSize(self->si_info->si_##field_name); \
	}
DEFINE_FIELD_READER(itemsize)
DEFINE_FIELD_READER(items_per_page)
DEFINE_FIELD_READER(totalpages)
DEFINE_FIELD_READER(totalitems)
DEFINE_FIELD_READER(cur_alloc)
DEFINE_FIELD_READER(max_alloc)
DEFINE_FIELD_READER(cur_free)
DEFINE_FIELD_READER(max_free)
DEFINE_FIELD_READER(cur_fullpages)
DEFINE_FIELD_READER(max_fullpages)
DEFINE_FIELD_READER(cur_freepages)
DEFINE_FIELD_READER(max_freepages)
DEFINE_FIELD_READER(usedpages)
DEFINE_FIELD_READER(tailpages)
#undef DEFINE_FIELD_READER


PRIVATE struct type_member si_members[] = {
	TYPE_MEMBER_FIELD_DOC("__stat__", STRUCT_OBJECT, offsetof(SlabInfoObject, si_stat), "->?GSlabStat"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset si_getsets[] = {
#define DEFINE_FIELD(name, doc)                                                         \
	{ #name,                                                                            \
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & si_get_##name, NULL, NULL, \
	  DOC("->?Dint\n" doc) }
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
	{ "__index__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & si_get_index, NULL, NULL,
	  DOC("->?Dint\n"
	      "Index of @this slab within :rt:SlabStat") },
	{ NULL }
};

#define DEFINE_SI_COMPARE(name, op)                                                   \
	PRIVATE DREF DeeObject *DCALL                                                     \
	name(SlabInfoObject *__restrict self,                                             \
	     SlabInfoObject *__restrict other) {                                          \
		if (DeeObject_AssertTypeExact(other, &SlabInfo_Type))                         \
			return NULL;                                                              \
		return_bool(memcmp(self->si_info, other->si_info, sizeof(DeeSlabInfo)) op 0); \
	}
DEFINE_SI_COMPARE(si_eq, ==)
DEFINE_SI_COMPARE(si_ne, !=)
DEFINE_SI_COMPARE(si_lo, <)
DEFINE_SI_COMPARE(si_le, <=)
DEFINE_SI_COMPARE(si_gr, >)
DEFINE_SI_COMPARE(si_ge, >=)
#undef DEFINE_SI_COMPARE

PRIVATE struct type_cmp si_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&si_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&si_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&si_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&si_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&si_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&si_ge
};

INTERN DeeTypeObject SlabInfo_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SlabInfo",
	/* .tp_doc      = */ DOC("Element type for :rt:SlabStat"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(SlabInfoObject)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&si_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&si_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&si_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL, /* No visit, because only non-GC objects are reachable */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &si_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
