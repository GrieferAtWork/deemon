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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C
#define GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/atomic.h>
#include <deemon/util/futex.h>

#include <hybrid/overflow.h>

/**/
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

/**/
#include "function-wrappers.h"

#ifdef CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION
DECL_BEGIN

/************************************************************************/
/* FunctionStatics_Type                                                 */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeFunctionObject *fs_func; /* [1..1][const] Function in question */
} FunctionStatics;

typedef struct {
	OBJECT_HEAD
	DREF DeeFunctionObject *fsi_func; /* [1..1][const] Function in question */
	uint16_t                fsi_sid;  /* [>= fsi_func->fo_code->co_refc][lock(ATOMIC)] Index of next static to enumerate. */
	uint16_t                fsi_end;  /* [== fsi_func->fo_code->co_refstaticc][const] Static enumeration end */
} FunctionStaticsIterator;

INTDEF DeeTypeObject FunctionStaticsIterator_Type;
INTDEF DeeTypeObject FunctionStatics_Type;


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcstaticsiter_nii_getseq(FunctionStaticsIterator *__restrict self) {
	return DeeFunction_GetStaticsWrapper(self->fsi_func);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcstaticsiter_nii_getindex(FunctionStaticsIterator *__restrict self) {
	uint16_t base   = self->fsi_func->fo_code->co_refc;
	uint16_t result = atomic_read(&self->fsi_sid);
	ASSERT(result >= base);
	return result - base;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstaticsiter_nii_setindex(FunctionStaticsIterator *__restrict self, size_t new_index) {
	uint16_t sid, base = self->fsi_func->fo_code->co_refc;
	if (OVERFLOW_UADD(base, new_index, &sid) || sid > self->fsi_end)
		sid = self->fsi_end;
	atomic_write(&self->fsi_sid, sid);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstaticsiter_nii_rewind(FunctionStaticsIterator *__restrict self) {
	uint16_t base = self->fsi_func->fo_code->co_refc;
	atomic_write(&self->fsi_sid, base);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcstaticsiter_eq(FunctionStaticsIterator *self,
                   FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	return_bool(self->fsi_func == other->fsi_func &&
	            atomic_read(&self->fsi_end) == atomic_read(&other->fsi_end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcstaticsiter_ne(FunctionStaticsIterator *self,
                   FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	return_bool(self->fsi_func != other->fsi_func ||
	            atomic_read(&self->fsi_end) != atomic_read(&other->fsi_end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcstaticsiter_lo(FunctionStaticsIterator *self,
                   FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	return_bool(self->fsi_func == other->fsi_func
	            ? atomic_read(&self->fsi_end) < atomic_read(&other->fsi_end)
	            : self->fsi_func < other->fsi_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcstaticsiter_le(FunctionStaticsIterator *self,
                   FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	return_bool(self->fsi_func == other->fsi_func
	            ? atomic_read(&self->fsi_end) <= atomic_read(&other->fsi_end)
	            : self->fsi_func <= other->fsi_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcstaticsiter_gr(FunctionStaticsIterator *self,
                   FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	return_bool(self->fsi_func == other->fsi_func
	            ? atomic_read(&self->fsi_end) > atomic_read(&other->fsi_end)
	            : self->fsi_func > other->fsi_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcstaticsiter_ge(FunctionStaticsIterator *self,
                   FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	return_bool(self->fsi_func == other->fsi_func
	            ? atomic_read(&self->fsi_end) >= atomic_read(&other->fsi_end)
	            : self->fsi_func >= other->fsi_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstaticsiter_ctor(FunctionStaticsIterator *__restrict self) {
	self->fsi_func = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)&DeeCode_Empty);
	if unlikely(!self->fsi_func)
		goto err;
	self->fsi_sid = 0;
	self->fsi_end = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcstaticsiter_copy(FunctionStaticsIterator *__restrict self,
                     FunctionStaticsIterator *__restrict other) {
	self->fsi_func = other->fsi_func;
	Dee_Incref(self->fsi_func);
	self->fsi_sid = atomic_read(&other->fsi_sid);
	self->fsi_end = other->fsi_end;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstaticsiter_init(FunctionStaticsIterator *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:FunctionStaticsIterator", &self->fsi_func))
		goto err;
	if (DeeObject_AssertTypeExact(self->fsi_func, &DeeFunction_Type))
		goto err;
	Dee_Incref(self->fsi_func);
	self->fsi_sid = self->fsi_func->fo_code->co_refc;
	self->fsi_end = self->fsi_func->fo_code->co_refstaticc;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
funcstaticsiter_fini(FunctionStaticsIterator *__restrict self) {
	Dee_Decref(self->fsi_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
funcstaticsiter_visit(FunctionStaticsIterator *__restrict self,
                      dvisit_t proc, void *arg) {
	Dee_Visit(self->fsi_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstaticsiter_bool(FunctionStaticsIterator *__restrict self) {
	uint16_t sid = atomic_read(&self->fsi_sid);
	while (sid < self->fsi_end) {
		DeeObject *v = atomic_read(&self->fsi_func->fo_refv[sid]);
		if (ITER_ISOK(v))
			return 1;
		++sid;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcstaticsiter_next(FunctionStaticsIterator *__restrict self) {
	DREF DeeObject *result;
	DeeFunctionObject *func = self->fsi_func;
	for (;;) {
		uint16_t old_sid, new_sid;
		old_sid = atomic_read(&self->fsi_sid);
		new_sid = old_sid;
		DeeFunction_RefLockRead(func);
		for (;;) {
			if (new_sid >= self->fsi_end) {
				DeeFunction_RefLockEndRead(func);
				return ITER_DONE;
			}
			result = func->fo_refv[new_sid];
			if (ITER_ISOK(result))
				break;
			++new_sid;
		}
		Dee_Incref(result);
		DeeFunction_RefLockEndRead(func);
		++new_sid;
		if (atomic_cmpxch_or_write(&self->fsi_sid, old_sid, new_sid))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE struct type_nii tpconst funcstaticsiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&funcstaticsiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&funcstaticsiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&funcstaticsiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&funcstaticsiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp funcstaticsiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_ge,
};

PRIVATE struct type_getset tpconst funcstaticsiter_getsets[] = {
	TYPE_GETTER(STR_seq, &funcstaticsiter_nii_getseq, "->?Ert:FunctionStatics"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst funcstaticsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FunctionStaticsIterator, fsi_func), "->?DFunction"),
	TYPE_MEMBER_FIELD("__sid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionStaticsIterator, fsi_sid)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionStaticsIterator, fsi_end)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FunctionStaticsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionStaticsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&funcstaticsiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&funcstaticsiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&funcstaticsiter_init,
				TYPE_FIXED_ALLOCATOR(FunctionStaticsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcstaticsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&funcstaticsiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&funcstaticsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &funcstaticsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcstaticsiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ funcstaticsiter_getsets,
	/* .tp_members       = */ funcstaticsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcstatics_nsi_getsize(FunctionStatics *__restrict self) {
	DeeCodeObject *code = self->fs_func->fo_code;
	return code->co_refstaticc - code->co_refc;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcstatics_nsi_getitem(FunctionStatics *__restrict self, size_t index) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *result;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc) {
		err_index_out_of_bounds((DeeObject *)self, index, funcstatics_nsi_getsize(self));
		return NULL;
	}
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[index];
	if unlikely(!ITER_ISOK(result)) {
		DeeFunction_RefLockEndRead(func);
		index -= code->co_refc;
		err_unbound_index((DeeObject *)self, index);
		return NULL;
	}
	Dee_Incref(result);
	DeeFunction_RefLockEndRead(func);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstatics_nsi_delitem(FunctionStatics *__restrict self, size_t index) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *oldval;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc)
		return err_index_out_of_bounds((DeeObject *)self, index, funcstatics_nsi_getsize(self));
	DeeFunction_RefLockWrite(func);
	oldval = func->fo_refv[index]; /* Inherit reference */
	func->fo_refv[index] = NULL;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[index]);
	if (ITER_ISOK(oldval))
		Dee_Decref(oldval);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
funcstatics_nsi_setitem(FunctionStatics *self, size_t index, DeeObject *value) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *oldval;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc)
		return err_index_out_of_bounds((DeeObject *)self, index, funcstatics_nsi_getsize(self));
	Dee_Incref(value);
	DeeFunction_RefLockWrite(func);
	oldval = func->fo_refv[index]; /* Inherit reference */
	func->fo_refv[index] = value;  /* Inherit reference */
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[index]);
	if (ITER_ISOK(oldval))
		Dee_Decref(oldval);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcstatics_nsi_getitem_fast(FunctionStatics *__restrict self, size_t index) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *result;
	ASSERT(index + code->co_refc >= index);
	index += code->co_refc;
	ASSERT(index < code->co_refstaticc);
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[index];
	if unlikely(!ITER_ISOK(result)) {
		result = NULL;
	} else {
		Dee_Incref(result);
	}
	DeeFunction_RefLockEndRead(func);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
funcstatics_nsi_xchitem(FunctionStatics *self, size_t index, DeeObject *value) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *oldval;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc) {
		err_index_out_of_bounds((DeeObject *)self, index, funcstatics_nsi_getsize(self));
		return NULL;
	}
	Dee_Incref(value);
	DeeFunction_RefLockWrite(func);
	oldval = func->fo_refv[index]; /* Inherit reference */
	if unlikely(!ITER_ISOK(oldval)) {
		DeeFunction_RefLockEndWrite(func);
		index -= code->co_refc;
		err_unbound_index((DeeObject *)self, index);
		Dee_Decref_unlikely(value);
		return NULL;
	}
	func->fo_refv[index] = value; /* Inherit reference */
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[index]);
	return oldval;
}

PRIVATE WUNUSED NONNULL((1)) DREF FunctionStaticsIterator *DCALL
funcstatics_iter(FunctionStatics *__restrict self) {
	DREF FunctionStaticsIterator *result;
	result = DeeObject_MALLOC(FunctionStaticsIterator);
	if unlikely(!result)
		goto err;
	result->fsi_func = self->fs_func;
	Dee_Incref(result->fsi_func);
	result->fsi_sid = self->fs_func->fo_code->co_refc;
	result->fsi_end = self->fs_func->fo_code->co_refstaticc;
	DeeObject_Init(result, &FunctionStaticsIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstatics_ctor(FunctionStatics *__restrict self) {
	self->fs_func = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)&DeeCode_Empty);
	if unlikely(!self->fs_func)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcstatics_copy(FunctionStatics *__restrict self,
                 FunctionStatics *__restrict other) {
	self->fs_func = other->fs_func;
	Dee_Incref(self->fs_func);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstatics_init(FunctionStatics *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:FunctionStatics", &self->fs_func))
		goto err;
	if (DeeObject_AssertTypeExact(self->fs_func, &DeeFunction_Type))
		goto err;
	Dee_Incref(self->fs_func);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
funcstatics_fini(FunctionStatics *__restrict self) {
	Dee_Decref(self->fs_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
funcstatics_visit(FunctionStatics *__restrict self,
                  dvisit_t proc, void *arg) {
	Dee_Visit(self->fs_func);
}


PRIVATE struct type_nsi tpconst funcstatics_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&funcstatics_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&funcstatics_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&funcstatics_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)&funcstatics_nsi_delitem,
			/* .nsi_setitem      = */ (dfunptr_t)&funcstatics_nsi_setitem,
			/* .nsi_getitem_fast = */ (dfunptr_t)&funcstatics_nsi_getitem_fast,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)&funcstatics_nsi_xchitem,
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


PRIVATE struct type_seq funcstatics_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcstatics_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &funcstatics_nsi
};

PRIVATE struct type_member tpconst funcstatics_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FunctionStaticsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst funcstatics_members[] = {
	TYPE_MEMBER_FIELD("__func__", STRUCT_OBJECT, offsetof(FunctionStatics, fs_func)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FunctionStatics_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionStatics",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&funcstatics_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&funcstatics_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&funcstatics_init,
				TYPE_FIXED_ALLOCATOR(FunctionStatics)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcstatics_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&funcstatics_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &funcstatics_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcstatics_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ funcstatics_class_members
};




/* Callbacks to create specialized function wrappers. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetStaticsWrapper(DeeFunctionObject *__restrict self) {
	DREF FunctionStatics *result;
	result = DeeObject_MALLOC(FunctionStatics);
	if unlikely(!result)
		goto err;
	result->fs_func = self;
	Dee_Incref(self);
	DeeObject_Init(result, &FunctionStatics_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}




/************************************************************************/
/* ...                                                                  */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetRefsByNameWrapper(DeeFunctionObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetStaticsByNameWrapper(DeeFunctionObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetSymbolsByNameWrapper(DeeFunctionObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeYieldFunction_GetArgsByNameWrapper(DeeYieldFunctionObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeYieldFunction_GetSymbolsByNameWrapper(DeeYieldFunctionObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetArgsWrapper(DeeFrameObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetLocalsWrapper(DeeFrameObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetStackWrapper(DeeFrameObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetArgsByNameWrapper(DeeFrameObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetVariablesByNameWrapper(DeeFrameObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetSymbolsByNameWrapper(DeeFrameObject *__restrict self) {
	(void)self;
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}


DECL_END
#endif /* CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION */

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C */
