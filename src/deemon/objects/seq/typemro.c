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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_C
#define GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* Dee_True, return_false, return_true */
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>           /* DeeRT_ErrIndexOutOfBounds */
#include <deemon/format.h>             /* PRFuSIZ */
#include <deemon/object.h>
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type, Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL, Dee_TYPE_ITERX_FNORMAL, type_nii */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/system-features.h>    /* memcpy */
#include <deemon/util/atomic.h>        /* atomic_read */
#include <deemon/util/hash.h>          /* Dee_HashPointer */
#include <deemon/util/lock.h>          /* Dee_atomic_lock_init */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "typemro.h"

#include <stddef.h> /* NULL, offsetof, size_t */

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
	DeeArg_Unpack1(err, argc, argv, "_TypeMROIterator", &self->tmi_mro.tp_mro_orig);
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
	DeeArg_Unpack1(err, argc, argv, "_TypeBasesIterator", &type);
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

#define typebasesiter_serialize typemroiter_serialize
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
typemroiter_serialize(TypeMROIterator *__restrict self,
                      DeeSerial *__restrict writer,
                      Dee_seraddr_t addr) {
	DeeTypeMRO self__tmi_mro;
	DeeTypeObject *self__tmi_iter;
#define ADDROF(field) (addr + offsetof(TypeMROIterator, field))
	TypeMROIterator_LockAcquire(self);
	self__tmi_iter = self->tmi_iter;
	memcpy(&self__tmi_mro, &self->tmi_mro, sizeof(self->tmi_mro));
	TypeMROIterator_LockRelease(self);
	if (DeeSerial_PutObject(writer, ADDROF(tmi_mro.tp_mro_orig), self__tmi_mro.tp_mro_orig))
		goto err;
	if (self__tmi_iter != self__tmi_mro.tp_mro_orig) {
		if (DeeSerial_PutPointer(writer, ADDROF(tmi_mro.tp_mro_iter), self__tmi_mro.tp_mro_iter))
			goto err;
	} else {
		TypeMROIterator *out;
		out = DeeSerial_Addr2Mem(writer, addr, TypeMROIterator);
		out->tmi_mro.tp_mro_iter = NULL;
	}
	return DeeSerial_PutPointer(writer, ADDROF(tmi_iter), self__tmi_iter);
err:
	return -1;
#undef ADDROF
}

STATIC_ASSERT(offsetof(TypeMROIterator, tmi_mro.tp_mro_orig) == offsetof(ProxyObject, po_obj));
#define typemroiter_fini    generic_proxy__fini_unlikely /* Unlikely because types are usually referenced elsewhere */
#define typebasesiter_fini  generic_proxy__fini_unlikely /* Unlikely because types are usually referenced elsewhere */
#define typemroiter_visit   generic_proxy__visit
#define typebasesiter_visit generic_proxy__visit

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
	/* .nii_class = */ Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL,
	/* .nii_flags = */ Dee_TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&typemroiter_nii_getseq,
			/* .nii_getindex = */ NULL,
			/* .nii_setindex = */ NULL,
			/* .nii_rewind   = */ (Dee_funptr_t)&typemroiter_nii_rewind,
			/* .nii_revert   = */ NULL,
			/* .nii_advance  = */ NULL,
			/* .nii_prev     = */ NULL,
			/* .nii_next     = */ NULL,
			/* .nii_hasprev  = */ NULL,
			/* .nii_peek     = */ (Dee_funptr_t)&typemroiter_nii_peek
		}
	}
};

PRIVATE struct type_nii tpconst typebasesiter_nii = {
	/* .nii_class = */ Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL,
	/* .nii_flags = */ Dee_TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&typebasesiter_nii_getseq,
			/* .nii_getindex = */ NULL,
			/* .nii_setindex = */ NULL,
			/* .nii_rewind   = */ (Dee_funptr_t)&typebasesiter_nii_rewind,
			/* .nii_revert   = */ NULL,
			/* .nii_advance  = */ NULL,
			/* .nii_prev     = */ NULL,
			/* .nii_next     = */ NULL,
			/* .nii_hasprev  = */ NULL,
			/* .nii_peek     = */ (Dee_funptr_t)&typebasesiter_nii_peek
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
	/* .tp_compare       = */ DEFIMPL(&iterator_compare),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &typemroiter_nii,
};

PRIVATE struct type_cmp typebasesiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&typebasesiter_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&typebasesiter_compare_eq,
	/* .tp_compare       = */ DEFIMPL(&iterator_compare),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TypeMROIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &typemroiter_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &typemroiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &typemroiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typemroiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typemroiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&typemroiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &typemroiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typemroiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ typemroiter_getsets,
	/* .tp_members       = */ typemroiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TypeMROIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &typebasesiter_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &typebasesiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &typebasesiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typebasesiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typebasesiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&typebasesiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &typebasesiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typebasesiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ typebasesiter_getsets,
	/* .tp_members       = */ typebasesiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



#define typebases_serialize generic_proxy__serialize
#define typemro_serialize   generic_proxy__serialize

#define typebases_init typemro_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
typemro_init(TypeMRO *__restrict self,
             size_t argc, DeeObject *const *argv) {
	if unlikely(generic_proxy__init((ProxyObject *)self, argc, argv))
		goto err;
	if (DeeObject_AssertType(self->tm_type, &DeeType_Type))
		goto err_type;
	return 0;
err_type:
	Dee_Decref(self->tm_type);
err:
	return -1;
}


STATIC_ASSERT(offsetof(TypeMRO, tm_type) == offsetof(ProxyObject, po_obj));
#define typebases_fini  generic_proxy__fini_unlikely /* Unlikely because types are usually referenced elsewhere */
#define typemro_fini    generic_proxy__fini_unlikely /* Unlikely because types are usually referenced elsewhere */
#define typebases_visit generic_proxy__visit
#define typemro_visit   generic_proxy__visit
#define typemro_copy    generic_proxy__copy_alias
#define typemro_deep    generic_proxy__copy_alias
#define typebases_copy  generic_proxy__copy_alias
#define typebases_deep  generic_proxy__copy_alias

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
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, position);
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
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, position);
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


PRIVATE struct type_seq typemro_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typemro_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&typemro_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&typemro_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&typemro_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&typemro_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typemro_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typemro_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq typebases_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typebases_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&typebases_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&typebases_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&typebases_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&typebases_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typebases_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&typebases_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

#define typebases_members typemro_members
PRIVATE struct type_member tpconst typemro_members[] = {
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_OBJECT, offsetof(TypeMRO, tm_type), "->?DType"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst typemro_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &TypeMROIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeType_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst typebases_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &TypeBasesIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeType_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TypeMRO,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &typemro_copy,
			/* tp_deep_ctor:   */ &typemro_deep,
			/* tp_any_ctor:    */ &typemro_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &typemro_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typemro_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typemro_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&typemro_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__287C06B7236F06BE),
	/* .tp_seq           = */ &typemro_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ typemro_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ typemro_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TypeMRO,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &typebases_copy,
			/* tp_deep_ctor:   */ &typebases_deep,
			/* tp_any_ctor:    */ &typebases_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &typebases_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&typebases_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&typebases_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&typebases_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__287C06B7236F06BE),
	/* .tp_seq           = */ &typebases_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ typebases_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ typebases_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
