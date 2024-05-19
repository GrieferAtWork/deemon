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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_C
#define GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "typemro.h"

DECL_BEGIN

#define typebasesiter_copy typemroiter_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
typemroiter_copy(TypeMROIterator *__restrict self,
                 TypeMROIterator *__restrict other) {
	Dee_atomic_lock_init(&self->tmi_lock);
	TypeMROIterator_LockAcquire(other);
	self->tmi_iter = other->tmi_iter;
	memcpy(&self->tmi_mro, &other->tmi_mro, sizeof(self->tmi_mro));
	TypeMROIterator_LockRelease(other);
	Dee_Incref((DeeTypeObject *)self->tmi_mro.tp_mro_orig);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typemroiter_init(TypeMROIterator *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_TypeMROIterator", &self->tmi_mro.tp_mro_orig))
		goto err;
	if (DeeObject_InstanceOfExact(self->tmi_mro.tp_mro_orig, &TypeMRO_Type)) {
		self->tmi_mro.tp_mro_orig = ((TypeMRO *)self->tmi_mro.tp_mro_orig)->tm_type;
	} else {
		if (DeeObject_AssertType(self->tmi_mro.tp_mro_orig, &DeeType_Type))
			goto err;
	}
	Dee_Incref((DeeTypeObject *)self->tmi_mro.tp_mro_orig);
	self->tmi_iter = NULL;
	Dee_atomic_lock_init(&self->tmi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typebasesiter_init(TypeMROIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	DeeTypeObject *type;
	if (DeeArg_Unpack(argc, argv, "o:_TypeBasesIterator", &type))
		goto err;
	if (DeeObject_InstanceOfExact(type, &TypeBases_Type)) {
		type = ((TypeMRO *)type)->tm_type;
	} else {
		if (DeeObject_AssertType(type, &DeeType_Type))
			goto err;
	}
	self->tmi_iter = DeeTypeMRO_Init(&self->tmi_mro, type);
	Dee_Incref(type);
	Dee_atomic_lock_init(&self->tmi_lock);
	return 0;
err:
	return -1;
}

#define typebasesiter_fini typemroiter_fini
PRIVATE NONNULL((1)) void DCALL
typemroiter_fini(TypeMROIterator *__restrict self) {
	Dee_Decref_unlikely((DeeTypeObject *)self->tmi_mro.tp_mro_orig);
}

#define typebasesiter_visit typemroiter_visit
PRIVATE NONNULL((1, 2)) void DCALL
typemroiter_visit(TypeMROIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit((DeeTypeObject *)self->tmi_mro.tp_mro_orig);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typemroiter_bool(TypeMROIterator *__restrict self) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	TypeMROIterator_LockAcquire(self);
	iter = self->tmi_iter;
	memcpy(&mro, &self->tmi_mro, sizeof(DeeTypeMRO));
	TypeMROIterator_LockRelease(self);
	return (iter == NULL || DeeTypeMRO_Next(&mro, iter) != NULL) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typebasesiter_bool(TypeMROIterator *__restrict self) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	TypeMROIterator_LockAcquire(self);
	iter = self->tmi_iter;
	memcpy(&mro, &self->tmi_mro, sizeof(DeeTypeMRO));
	TypeMROIterator_LockRelease(self);
	ASSERT(iter != NULL);
	return DeeTypeMRO_NextDirectBase(&mro, iter) != NULL ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typemroiter_next(TypeMROIterator *__restrict self) {
	DeeTypeObject *result;
	TypeMROIterator_LockAcquire(self);
	result = self->tmi_iter;
	if (result == NULL) {
		/* Special case for first MRO element (which is always the original type). */
		result = (DeeTypeObject *)self->tmi_mro.tp_mro_orig;
		self->tmi_iter = result;
	} else {
		DeeTypeMRO mro;
		memcpy(&mro, &self->tmi_mro, sizeof(DeeTypeMRO));
		result = DeeTypeMRO_Next(&mro, self->tmi_iter);
		if (result == NULL) {
			TypeMROIterator_LockRelease(self);
			return (DREF DeeTypeObject *)ITER_DONE;
		}
		memcpy(&self->tmi_mro, &mro, sizeof(DeeTypeMRO));
		self->tmi_iter = result;
	}
	TypeMROIterator_LockRelease(self);
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typebasesiter_next(TypeMROIterator *__restrict self) {
	DeeTypeObject *result;
	DeeTypeMRO mro;
	TypeMROIterator_LockAcquire(self);
	memcpy(&mro, &self->tmi_mro, sizeof(DeeTypeMRO));
	result = DeeTypeMRO_NextDirectBase(&mro, self->tmi_iter);
	if (result == NULL) {
		TypeMROIterator_LockRelease(self);
		return (DREF DeeTypeObject *)ITER_DONE;
	}
	memcpy(&self->tmi_mro, &mro, sizeof(DeeTypeMRO));
	self->tmi_iter = result;
	TypeMROIterator_LockRelease(self);
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF TypeMRO *DCALL
typemroiter_getseq(TypeMROIterator *__restrict self) {
	return TypeMRO_New((DeeTypeObject *)self->tmi_mro.tp_mro_orig);
}

PRIVATE WUNUSED NONNULL((1)) DREF TypeMRO *DCALL
typebasesiter_getseq(TypeMROIterator *__restrict self) {
	return TypeBases_New((DeeTypeObject *)self->tmi_mro.tp_mro_orig);
}

PRIVATE struct type_getset tpconst typemroiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &typemroiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:TypeMRO"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst typebasesiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &typebasesiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:TypeBases"),
	TYPE_GETSET_END
};

#define typebasesiter_members typemroiter_members
PRIVATE struct type_member tpconst typemroiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_OBJECT, offsetof(TypeMROIterator, tmi_mro.tp_mro_orig), "->?DType"),
	TYPE_MEMBER_END
};

#define typemroiter_nii_getseq   typemroiter_getseq
#define typebasesiter_nii_getseq typebasesiter_getseq

PRIVATE WUNUSED NONNULL((1)) int DCALL
typemroiter_nii_rewind(TypeMROIterator *__restrict self) {
	TypeMROIterator_LockAcquire(self);
	self->tmi_iter = NULL;
	TypeMROIterator_LockRelease(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typebasesiter_nii_rewind(TypeMROIterator *__restrict self) {
	TypeMROIterator_LockAcquire(self);
	self->tmi_iter = (DeeTypeObject *)self->tmi_mro.tp_mro_orig;
	TypeMROIterator_LockRelease(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typemroiter_nii_peek(TypeMROIterator *__restrict self) {
	DeeTypeObject *result;
	TypeMROIterator_LockAcquire(self);
	result = self->tmi_iter;
	if (result == NULL) {
		TypeMROIterator_LockRelease(self);
		/* Special case for first MRO element (which is always the original type). */
		result = (DeeTypeObject *)self->tmi_mro.tp_mro_orig;
	} else {
		DeeTypeMRO mro;
		memcpy(&mro, &self->tmi_mro, sizeof(DeeTypeMRO));
		TypeMROIterator_LockRelease(self);
		result = DeeTypeMRO_Next(&mro, self->tmi_iter);
		if (result == NULL)
			return (DREF DeeTypeObject *)ITER_DONE;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typebasesiter_nii_peek(TypeMROIterator *__restrict self) {
	DeeTypeObject *result;
	DeeTypeMRO mro;
	TypeMROIterator_LockAcquire(self);
	memcpy(&mro, &self->tmi_mro, sizeof(DeeTypeMRO));
	result = self->tmi_iter;
	TypeMROIterator_LockRelease(self);
	result = DeeTypeMRO_NextDirectBase(&mro, result);
	if (result == NULL)
		return (DREF DeeTypeObject *)ITER_DONE;
	return_reference_(result);
}

PRIVATE struct type_nii tpconst typemroiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_UNIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&typemroiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)NULL,
			/* .nii_setindex = */ (dfunptr_t)NULL,
			/* .nii_rewind   = */ (dfunptr_t)&typemroiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)&typemroiter_nii_peek
		}
	}
};

PRIVATE struct type_nii tpconst typebasesiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_UNIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&typebasesiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)NULL,
			/* .nii_setindex = */ (dfunptr_t)NULL,
			/* .nii_rewind   = */ (dfunptr_t)&typebasesiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)&typebasesiter_nii_peek
		}
	}
};

#define typebasesiter_hash       typemroiter_hash
#define typebasesiter_compare_eq typemroiter_compare_eq

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
typemroiter_hash(TypeMROIterator *self) {
	return Dee_HashPointer(atomic_read(&self->tmi_iter));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
typemroiter_compare_eq(TypeMROIterator *self, TypeMROIterator *other) {
	DeeTypeObject *lhs_iter, *rhs_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	lhs_iter = atomic_read(&self->tmi_iter);
	rhs_iter = atomic_read(&other->tmi_iter);
	return lhs_iter == rhs_iter ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp typemroiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&typemroiter_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&typemroiter_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &typemroiter_nii,
};

PRIVATE struct type_cmp typebasesiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&typebasesiter_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&typebasesiter_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &typebasesiter_nii,
};

INTERN DeeTypeObject TypeMROIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeMROIterator",
	/* .tp_doc      = */ DOC("(type:?DType)\n"
	                         "(mro:?Ert:TypeMRO)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&typemroiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&typemroiter_init,
				TYPE_FIXED_ALLOCATOR(TypeMROIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typemroiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typemroiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&typemroiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &typemroiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typemroiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ typemroiter_getsets,
	/* .tp_members       = */ typemroiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject TypeBasesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeBasesIterator",
	/* .tp_doc      = */ DOC("(type:?DType)\n"
	                         "(mro:?Ert:TypeBases)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&typebasesiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&typebasesiter_init,
				TYPE_FIXED_ALLOCATOR(TypeMROIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typebasesiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typebasesiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&typebasesiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &typebasesiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typebasesiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ typebasesiter_getsets,
	/* .tp_members       = */ typebasesiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




PRIVATE WUNUSED NONNULL((1)) int DCALL
typemro_init(TypeMRO *__restrict self,
             size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_TypeMRO", &self->tm_type))
		goto err;
	if (DeeObject_AssertType(self->tm_type, &DeeType_Type))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typebases_init(TypeMRO *__restrict self,
               size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_TypeBases", &self->tm_type))
		goto err;
	if (DeeObject_AssertType(self->tm_type, &DeeType_Type))
		goto err;
	return 0;
err:
	return -1;
}


#define typebases_fini typemro_fini
PRIVATE NONNULL((1)) void DCALL
typemro_fini(TypeMRO *__restrict self) {
	Dee_Decref_unlikely(self->tm_type);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typemro_bool(TypeMRO *__restrict self) {
	/* MRO is never empty, because it always contains at least the type itself */
	(void)self;
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
typebases_bool(TypeMRO *__restrict self) {
	/* Bases are empty when the type doesn't have any bases. */
	return DeeType_Base(self->tm_type) != NULL ? 1 : 0;
}

#define typebases_visit typemro_visit
PRIVATE NONNULL((1, 2)) void DCALL
typemro_visit(TypeMRO *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->tm_type);
}


PRIVATE WUNUSED NONNULL((1)) DREF TypeMROIterator *DCALL
typemro_iter(TypeMRO *__restrict self) {
	DREF TypeMROIterator *result;
	result = DeeObject_MALLOC(TypeMROIterator);
	if unlikely(!result)
		goto done;
	result->tmi_iter = NULL;
	DeeTypeMRO_Init(&result->tmi_mro, self->tm_type);
	Dee_Incref(self->tm_type);
	Dee_atomic_lock_init(&result->tmi_lock);
	DeeObject_Init(result, &TypeMROIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF TypeMROIterator *DCALL
typebases_iter(TypeMRO *__restrict self) {
	DREF TypeMROIterator *result;
	result = DeeObject_MALLOC(TypeMROIterator);
	if unlikely(!result)
		goto done;
	result->tmi_iter = DeeTypeMRO_Init(&result->tmi_mro, self->tm_type);
	Dee_Incref(self->tm_type);
	Dee_atomic_lock_init(&result->tmi_lock);
	DeeObject_Init(result, &TypeBasesIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
typemro_contains(TypeMRO *self, DeeTypeObject *elem) {
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	do {
		if (iter == elem)
			return_true;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return_false;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
typebases_contains(TypeMRO *self, DeeTypeObject *elem) {
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		if (iter == elem)
			return_true;
	}
	return_false;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
typemro_size(TypeMRO *__restrict self) {
	size_t result = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	do {
		++result;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
typebases_size(TypeMRO *__restrict self) {
	size_t result = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL)
		++result;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typemro_getitem_index(TypeMRO *__restrict self, size_t index) {
	size_t position = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	do {
		if (position == index)
			return_reference_(iter);
		++position;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	err_index_out_of_bounds((DeeObject *)self, index, position);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typemro_getitem_index_fast(TypeMRO *__restrict self, size_t index) {
	size_t position = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	for (;;) {
		if (position == index)
			return_reference_(iter);
		++position;
		iter = DeeTypeMRO_Next(&mro, iter);
		ASSERTF(iter != NULL,
		        "Index out-of-bounds: [index:%" PRFuSIZ ", size:%" PRFuSIZ "]",
		        index, position);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typebases_getitem_index(TypeMRO *__restrict self, size_t index) {
	size_t position = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		if (position == index)
			return_reference_(iter);
		++position;
	}
	err_index_out_of_bounds((DeeObject *)self, index, position);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
typebases_getitem_index_fast(TypeMRO *__restrict self, size_t index) {
	size_t position = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self->tm_type);
	for (;;) {
		iter = DeeTypeMRO_NextDirectBase(&mro, iter);
		ASSERTF(iter != NULL,
		        "Index out-of-bounds: [index:%" PRFuSIZ ", size:%" PRFuSIZ "]",
		        index, position);
		if (position == index)
			return_reference_(iter);
		++position;
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
typemro_foreach(TypeMRO *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter = DeeTypeMRO_Init(&mro, self->tm_type);
	do {
		temp = (*proc)(arg, (DeeObject *)iter);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
typebases_foreach(TypeMRO *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeTypeMRO mro;
	DeeTypeObject *iter = DeeTypeMRO_Init(&mro, self->tm_type);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		temp = (*proc)(arg, (DeeObject *)iter);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}


PRIVATE struct type_nsi tpconst typemro_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&typemro_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&typemro_size,
			/* .nsi_getitem      = */ (dfunptr_t)&typemro_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&typemro_getitem_index_fast,
		}
	}
};

PRIVATE struct type_nsi tpconst typebases_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&typebases_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&typebases_size,
			/* .nsi_getitem      = */ (dfunptr_t)&typebases_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&typebases_getitem_index_fast,
		}
	}
};

PRIVATE struct type_seq typemro_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typemro_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&typemro_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &typemro_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&typemro_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&typemro_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&typemro_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typemro_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typemro_getitem_index_fast,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_seq typebases_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typebases_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&typebases_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &typebases_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&typebases_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&typebases_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&typebases_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typebases_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typebases_getitem_index_fast,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

#define typebases_members typemro_members
PRIVATE struct type_member tpconst typemro_members[] = {
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_OBJECT, offsetof(TypeMRO, tm_type), "->?DType"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst typemro_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &TypeMROIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst typebases_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &TypeBasesIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject TypeMRO_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeMRO",
	/* .tp_doc      = */ DOC("(type:?DType)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&typemro_init,
				TYPE_FIXED_ALLOCATOR(TypeMRO)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typemro_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typemro_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&typemro_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &typemro_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ typemro_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ typemro_class_members
};

INTERN DeeTypeObject TypeBases_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeBases",
	/* .tp_doc      = */ DOC("(type:?DType)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&typebases_init,
				TYPE_FIXED_ALLOCATOR(TypeMRO)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typebases_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typebases_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&typebases_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &typebases_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ typebases_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ typebases_class_members
};


/* Construct wrappers for the mro/bases of a given type. */
INTERN WUNUSED NONNULL((1)) DREF TypeMRO *DCALL
TypeMRO_New(DeeTypeObject *__restrict self) {
	DREF TypeMRO *result;
	result = DeeObject_MALLOC(TypeMRO);
	if unlikely(!result)
		goto done;
	result->tm_type = self;
	Dee_Incref(self);
	DeeObject_Init(result, &TypeMRO_Type);
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF TypeMRO *DCALL
TypeBases_New(DeeTypeObject *__restrict self) {
	DREF TypeMRO *result;
	result = DeeObject_MALLOC(TypeMRO);
	if unlikely(!result)
		goto done;
	result->tm_type = self;
	Dee_Incref(self);
	DeeObject_Init(result, &TypeBases_Type);
done:
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_C */
