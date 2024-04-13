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
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/futex.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

/**/
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

/**/
#include "function-wrappers.h"

#ifdef CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION
DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

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
	if (!DeeFunction_Check(self->fsi_func)) {
		if (DeeObject_AssertTypeExact(self->fsi_func, &FunctionStatics_Type))
			goto err;
		self->fsi_func = ((FunctionStatics *)self->fsi_func)->fs_func;
	}
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
	/* .tp_nii  = */ &funcstaticsiter_nii,
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
/* FunctionSymbolsByName_Type                                           */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeFunctionObject *fsbn_func;      /* [1..1][const] Function in question */
	uint16_t                fsbn_rid_start; /* [const] First RID/SID to enumerate. */
	uint16_t                fsbn_rid_end;   /* [const] Last RID/SID to enumerate, plus 1. */
} FunctionSymbolsByName;

typedef struct {
	OBJECT_HEAD
	DREF FunctionSymbolsByName *fsbni_seq;  /* [1..1][const] Function whose references/statics are being enumerated. */
	DeeFunctionObject          *fsbni_func; /* [== fsbni_seq->fsbn_func][1..1][const] Function whose references/statics are being enumerated. */
	uint16_t                    fsbni_rid;  /* [lock(ATOMIC)] Next rid (overflowing into sids) to enumerate. */
	uint16_t                    fsbni_end;  /* [== fsbni_seq->fsbn_rid_end][const] RIS/SID end index. */
} FunctionSymbolsByNameIterator;

INTDEF DeeTypeObject FunctionSymbolsByNameIterator_Type;
INTDEF DeeTypeObject FunctionSymbolsByName_Type;


PRIVATE WUNUSED NONNULL((1)) DREF FunctionSymbolsByName *DCALL
funcsymbolbynameiter_nii_getseq(FunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->fsbni_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcsymbolbynameiter_nii_getindex(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t base   = self->fsbni_seq->fsbn_rid_start;
	uint16_t result = atomic_read(&self->fsbni_rid);
	ASSERT(result >= base);
	return result - base;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolbynameiter_nii_setindex(FunctionSymbolsByNameIterator *__restrict self, size_t new_index) {
	uint16_t sid, base = self->fsbni_seq->fsbn_rid_start;
	if (OVERFLOW_UADD(base, new_index, &sid) || sid > self->fsbni_end)
		sid = self->fsbni_end;
	atomic_write(&self->fsbni_rid, sid);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolbynameiter_nii_rewind(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t base = self->fsbni_seq->fsbn_rid_start;
	atomic_write(&self->fsbni_rid, base);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolbynameiter_eq(FunctionSymbolsByNameIterator *self,
                        FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func == other->fsbni_func &&
	            atomic_read(&self->fsbni_end) == atomic_read(&other->fsbni_end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolbynameiter_ne(FunctionSymbolsByNameIterator *self,
                        FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func != other->fsbni_func ||
	            atomic_read(&self->fsbni_end) != atomic_read(&other->fsbni_end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolbynameiter_lo(FunctionSymbolsByNameIterator *self,
                        FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func == other->fsbni_func
	            ? atomic_read(&self->fsbni_end) < atomic_read(&other->fsbni_end)
	            : self->fsbni_func < other->fsbni_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolbynameiter_le(FunctionSymbolsByNameIterator *self,
                        FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func == other->fsbni_func
	            ? atomic_read(&self->fsbni_end) <= atomic_read(&other->fsbni_end)
	            : self->fsbni_func <= other->fsbni_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolbynameiter_gr(FunctionSymbolsByNameIterator *self,
                        FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func == other->fsbni_func
	            ? atomic_read(&self->fsbni_end) > atomic_read(&other->fsbni_end)
	            : self->fsbni_func > other->fsbni_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolbynameiter_ge(FunctionSymbolsByNameIterator *self,
                        FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func == other->fsbni_func
	            ? atomic_read(&self->fsbni_end) >= atomic_read(&other->fsbni_end)
	            : self->fsbni_func >= other->fsbni_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolbynameiter_ctor(FunctionSymbolsByNameIterator *__restrict self) {
	self->fsbni_seq = (DREF FunctionSymbolsByName *)DeeObject_NewDefault(&FunctionSymbolsByName_Type);
	if unlikely(!self->fsbni_seq)
		goto err;
	self->fsbni_func = self->fsbni_seq->fsbn_func;
	self->fsbni_rid  = self->fsbni_seq->fsbn_rid_start;
	self->fsbni_end  = self->fsbni_seq->fsbn_rid_end;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolbynameiter_copy(FunctionSymbolsByNameIterator *__restrict self,
                          FunctionSymbolsByNameIterator *__restrict other) {
	self->fsbni_seq  = other->fsbni_seq;
	self->fsbni_func = other->fsbni_func;
	self->fsbni_rid  = atomic_read(&other->fsbni_rid);
	self->fsbni_end  = other->fsbni_end;
	Dee_Incref(self->fsbni_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolbynameiter_init(FunctionSymbolsByNameIterator *__restrict self,
                          size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:FunctionSymbolsByNameIterator", &self->fsbni_seq))
		goto err;
	if (DeeObject_AssertTypeExact(self->fsbni_seq, &FunctionSymbolsByName_Type))
		goto err;
	Dee_Incref(self->fsbni_seq);
	self->fsbni_func = self->fsbni_seq->fsbn_func;
	self->fsbni_rid  = self->fsbni_seq->fsbn_rid_start;
	self->fsbni_end  = self->fsbni_seq->fsbn_rid_end;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
funcsymbolbynameiter_fini(FunctionSymbolsByNameIterator *__restrict self) {
	Dee_Decref(self->fsbni_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
funcsymbolbynameiter_visit(FunctionSymbolsByNameIterator *__restrict self,
                           dvisit_t proc, void *arg) {
	Dee_Visit(self->fsbni_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolbynameiter_bool(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t sid = atomic_read(&self->fsbni_rid);
	while (sid < self->fsbni_end) {
		DeeObject *v = atomic_read(&self->fsbni_func->fo_refv[sid]);
		if (ITER_ISOK(v))
			return 1;
		++sid;
	}
	return 0;
}

/* Returns the string-name, or int-id (if there is no DDI info) */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Code_GetRefNameById(DeeCodeObject const *__restrict self, uint16_t rid) {
	char const *name = DeeCode_GetRSymbolName((DeeObject *)self, rid);
	if (name)
		return DeeString_New(name);
	return DeeInt_NewUInt16(rid);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolbynameiter_next(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t result_sid;
	DREF DeeTupleObject *result;
	DREF DeeObject *name;
	DREF DeeObject *value;
	DeeFunctionObject *func = self->fsbni_func;
	for (;;) {
		uint16_t old_sid, new_sid;
		old_sid = atomic_read(&self->fsbni_rid);
		new_sid = old_sid;
		DeeFunction_RefLockRead(func);
		for (;;) {
			if (new_sid >= self->fsbni_end) {
				DeeFunction_RefLockEndRead(func);
				return ITER_DONE;
			}
			value = func->fo_refv[new_sid];
			if (ITER_ISOK(value))
				break;
			++new_sid;
		}
		Dee_Incref(value);
		DeeFunction_RefLockEndRead(func);
		result_sid = new_sid;
		++new_sid;
		if (atomic_cmpxch_or_write(&self->fsbni_rid, old_sid, new_sid))
			break;
		Dee_Decref_unlikely(value);
	}
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_value;
	name = Code_GetRefNameById(self->fsbni_func->fo_code, result_sid);
	if unlikely(!name)
		goto err_value_result;
	DeeTuple_SET(result, 0, name);  /* Inherit reference */
	DeeTuple_SET(result, 1, value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_value_result:
	DeeTuple_FreeUninitialized(result);
err_value:
	Dee_Decref_unlikely(value);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolbynameiter_next_key(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t result_sid;
	DeeFunctionObject *func = self->fsbni_func;
	for (;;) {
		uint16_t old_sid, new_sid;
		old_sid = atomic_read(&self->fsbni_rid);
		new_sid = old_sid;
		DeeFunction_RefLockRead(func);
		for (;;) {
			DeeObject *value;
			if (new_sid >= self->fsbni_end) {
				DeeFunction_RefLockEndRead(func);
				return ITER_DONE;
			}
			value = func->fo_refv[new_sid];
			if (ITER_ISOK(value))
				break;
			++new_sid;
		}
		DeeFunction_RefLockEndRead(func);
		result_sid = new_sid;
		++new_sid;
		if (atomic_cmpxch_or_write(&self->fsbni_rid, old_sid, new_sid))
			break;
	}
	return Code_GetRefNameById(self->fsbni_func->fo_code, result_sid);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolbynameiter_next_value(FunctionSymbolsByNameIterator *__restrict self) {
	DREF DeeObject *result;
	DeeFunctionObject *func = self->fsbni_func;
	for (;;) {
		uint16_t old_sid, new_sid;
		old_sid = atomic_read(&self->fsbni_rid);
		new_sid = old_sid;
		DeeFunction_RefLockRead(func);
		for (;;) {
			if (new_sid >= self->fsbni_end) {
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
		if (atomic_cmpxch_or_write(&self->fsbni_rid, old_sid, new_sid))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE struct type_nii tpconst funcsymbolbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&funcsymbolbynameiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&funcsymbolbynameiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&funcsymbolbynameiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&funcsymbolbynameiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp funcsymbolbynameiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolbynameiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolbynameiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolbynameiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolbynameiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolbynameiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolbynameiter_ge,
	/* .tp_nii  = */ &funcsymbolbynameiter_nii,
};

PRIVATE struct type_member tpconst funcsymbolbynameiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(FunctionSymbolsByNameIterator, fsbni_seq), "->?Ert:FunctionSymbolsByName"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FunctionSymbolsByNameIterator, fsbni_func), "->?DFunction"),
	TYPE_MEMBER_FIELD("__rid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByNameIterator, fsbni_rid)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByNameIterator, fsbni_end)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FunctionSymbolsByNameIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionSymbolsByNameIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&funcsymbolbynameiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&funcsymbolbynameiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&funcsymbolbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcsymbolbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&funcsymbolbynameiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&funcsymbolbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &funcsymbolbynameiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolbynameiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcsymbolbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





/* Search for "name" in `self' and return its ID. If not found,
 * *NO* error is thrown, and `(uint16_t)-1' is returned. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) uint16_t DCALL
DDI_GetRefIdByName(DeeDDIObject const *self, char const *name) {
	/* Reference symbol name */
	uint8_t const *reader;
	char const *str_base;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeDDI_Type);
	str_base = DeeString_STR(self->d_strtab);
	if (self->d_exdat) {
		reader = self->d_exdat->dx_data;
		for (;;) {
			uint8_t op = *reader++;
			switch (op) {

			case DDI_EXDAT_O_END:
				goto done_exdat;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP8: {
				uint8_t got_rid      = UNALIGNED_GETLE8(reader + 0);
				char const *got_name = str_base + UNALIGNED_GETLE8(reader + 1);
				if (strcmp(got_name, name) == 0)
					return got_rid;
				reader += 1 + 1;
			}	break;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP16: {
				uint16_t got_rid     = UNALIGNED_GETLE16(reader + 0);
				char const *got_name = str_base + UNALIGNED_GETLE16(reader + 2);
				if (strcmp(got_name, name) == 0)
					return got_rid;
				reader += 2 + 2;
			}	break;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP32: {
				uint16_t got_rid     = UNALIGNED_GETLE16(reader + 0);
				char const *got_name = str_base + UNALIGNED_GETLE32(reader + 2);
				if (strcmp(got_name, name) == 0)
					return got_rid;
				reader += 2 + 4;
			}	break;

			default:
				switch (op & DDI_EXDAT_OPMASK) {

				case DDI_EXDAT_OP8:
					reader += 1 + 1;
					break;

				case DDI_EXDAT_OP16:
					reader += 2 + 2;
					break;

				case DDI_EXDAT_OP32:
					reader += 2 + 4;
					break;

				default: break;
				}
				break;
			}
		}
	}
done_exdat:
	return (uint16_t)-1;
}

/* Returns the ID associated with `key', or throw an error and return `(uint16_t)-1' */
PRIVATE WUNUSED NONNULL((1, 2)) uint16_t DCALL
FunctionSymbolsByName_GetRefIdByName(FunctionSymbolsByName const *self, DeeObject *key) {
	uint16_t rid;
	if (DeeString_Check(key)) {
		char const *name = DeeString_STR(key);
		rid = DDI_GetRefIdByName(self->fsbn_func->fo_code->co_ddi, name);
		if unlikely(rid < self->fsbn_rid_start)
			goto err_no_such_key;
	} else {
		if unlikely(DeeObject_AsUInt16(key, &rid))
			goto err;
		if unlikely(OVERFLOW_UADD(rid, self->fsbn_rid_start, &rid))
			goto err_no_such_key;
	}
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	return rid;
err_no_such_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return (uint16_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcsymbolsbyname_nsi_getsize(FunctionSymbolsByName *__restrict self) {
	return self->fsbn_rid_end - self->fsbn_rid_start;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
funcsymbolsbyname_nsi_getdefault(FunctionSymbolsByName *self,
                                 DeeObject *key, DeeObject *defl) {
	DREF DeeObject *result;
	DeeFunctionObject *func = self->fsbn_func;
	uint16_t rid;
	if (DeeString_Check(key)) {
		char const *name = DeeString_STR(key);
		rid = DDI_GetRefIdByName(func->fo_code->co_ddi, name);
		if unlikely(rid < self->fsbn_rid_start)
			goto err_no_such_key;
	} else {
		if unlikely(DeeObject_AsUInt16(key, &rid))
			goto err;
		if unlikely(OVERFLOW_UADD(rid, self->fsbn_rid_start, &rid))
			goto err_no_such_key;
	}
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[rid];
	if unlikely(!ITER_ISOK(result)) {
		DeeFunction_RefLockEndRead(func);
		goto err_no_such_key;
	}
	Dee_Incref(result);
	DeeFunction_RefLockEndRead(func);
	return result;
err_no_such_key:
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
funcsymbolsbyname_nsi_setdefault(FunctionSymbolsByName *self,
                                 DeeObject *key, DeeObject *defl) {
	DREF DeeObject *result;
	DeeFunctionObject *func;
	uint16_t rid = FunctionSymbolsByName_GetRefIdByName(self, key);
	if unlikely(rid == (uint16_t)-1)
		goto err;
	func = self->fsbn_func;
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[rid];
	if (ITER_ISOK(result)) {
		Dee_Incref(result);
		DeeFunction_RefLockEndRead(func);
	} else {
		if (!DeeFunction_RefLockUpgrade(func)) {
			result = func->fo_refv[rid];
			if (ITER_ISOK(result)) {
				Dee_Incref(result);
				DeeFunction_RefLockEndWrite(func);
				goto done;
			}
		}
		if unlikely(rid < func->fo_code->co_refc)
			goto err_unlock_ro;
		Dee_Incref_n(defl, 2);
		func->fo_refv[rid] = defl;
		result = defl;
		DeeFunction_RefLockEndWrite(func);
		DeeFutex_WakeAll(&func->fo_refv[rid]);
	}
done:
	return result;
err_unlock_ro:
	DeeFunction_RefLockEndWrite(func);
	err_readonly_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
funcsymbolsbyname_nsi_updateold(FunctionSymbolsByName *self,
                                DeeObject *key, DeeObject *value,
                                DREF DeeObject **p_oldvalue) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func = self->fsbn_func;
	uint16_t rid;
	if (DeeString_Check(key)) {
		char const *name = DeeString_STR(key);
		rid = DDI_GetRefIdByName(func->fo_code->co_ddi, name);
		if unlikely(rid < self->fsbn_rid_start)
			goto err_no_such_key;
	} else {
		if unlikely(DeeObject_AsUInt16(key, &rid))
			goto err;
		if unlikely(OVERFLOW_UADD(rid, self->fsbn_rid_start, &rid))
			goto err_no_such_key;
	}
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	if unlikely(!ITER_ISOK(oldvalue)) {
		DeeFunction_RefLockEndWrite(func);
		goto err_no_such_key;
	}
	if unlikely(rid < func->fo_code->co_refc)
		goto err_unlock_ro;
	Dee_Incref(value);
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	if (p_oldvalue) {
		*p_oldvalue = oldvalue; /* Inherit reference */
	} else {
		Dee_Decref(oldvalue);
	}
	return 1;
err_no_such_key:
	return 0;
err_unlock_ro:
	DeeFunction_RefLockEndWrite(func);
	err_readonly_key((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
funcsymbolsbyname_nsi_insertnew(FunctionSymbolsByName *self,
                                DeeObject *key, DeeObject *value,
                                DREF DeeObject **p_oldvalue) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func;
	uint16_t rid = FunctionSymbolsByName_GetRefIdByName(self, key);
	func = self->fsbn_func;
	if unlikely(rid == (uint16_t)-1)
		goto err;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	if (ITER_ISOK(oldvalue)) {
		if (p_oldvalue) {
			Dee_Incref(oldvalue);
			*p_oldvalue = oldvalue;
		}
		DeeFunction_RefLockEndWrite(func);
		return 1;
	}
	if unlikely(rid < func->fo_code->co_refc)
		goto err_unlock_ro;
	Dee_Incref(value);
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	return 0;
err_unlock_ro:
	DeeFunction_RefLockEndWrite(func);
	err_readonly_key((DeeObject *)self, key);
err:
	return -1;
}

#define funcsymbolsbyname_nsi_nextkey   funcsymbolbynameiter_next_key
#define funcsymbolsbyname_nsi_nextvalue funcsymbolbynameiter_next_value

PRIVATE WUNUSED NONNULL((1)) DREF FunctionSymbolsByNameIterator *DCALL
funcsymbolsbyname_iter(FunctionSymbolsByName *__restrict self) {
	DREF FunctionSymbolsByNameIterator *result;
	result = DeeObject_MALLOC(FunctionSymbolsByNameIterator);
	if unlikely(!result)
		goto err;
	result->fsbni_seq = self;
	Dee_Incref(self);
	result->fsbni_func = self->fsbn_func;
	result->fsbni_rid  = self->fsbn_rid_start;
	result->fsbni_end  = self->fsbn_rid_end;
	DeeObject_Init(result, &FunctionSymbolsByNameIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_contains(FunctionSymbolsByName *self,
                           DeeObject *key) {
	uint16_t rid;
	if (DeeString_Check(key)) {
		char const *name = DeeString_STR(key);
		rid = DDI_GetRefIdByName(self->fsbn_func->fo_code->co_ddi, name);
		if unlikely(rid < self->fsbn_rid_start)
			goto err_no_such_key;
	} else {
		if unlikely(DeeObject_AsUInt16(key, &rid))
			goto err;
		if unlikely(OVERFLOW_UADD(rid, self->fsbn_rid_start, &rid))
			goto err_no_such_key;
	}
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	return_true;
err_no_such_key:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_getitem(FunctionSymbolsByName *self,
                          DeeObject *key) {
	DREF DeeObject *result;
	DeeFunctionObject *func;
	uint16_t rid = FunctionSymbolsByName_GetRefIdByName(self, key);
	func = self->fsbn_func;
	if unlikely(rid == (uint16_t)-1)
		goto err;
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[rid];
	if unlikely(!ITER_ISOK(result)) {
		DeeFunction_RefLockEndRead(func);
		err_unbound_key((DeeObject *)self, key);
		goto err;
	}
	Dee_Incref(result);
	DeeFunction_RefLockEndRead(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_delitem(FunctionSymbolsByName *self,
                          DeeObject *key) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func;
	uint16_t rid = FunctionSymbolsByName_GetRefIdByName(self, key);
	func = self->fsbn_func;
	if unlikely(rid == (uint16_t)-1)
		goto err;
	if unlikely(rid < func->fo_code->co_refc)
		goto err_ro;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	func->fo_refv[rid] = NULL;
	DeeFunction_RefLockEndWrite(func);
	if (oldvalue != NULL) {
		DeeFutex_WakeAll(&func->fo_refv[rid]);
		if (oldvalue != ITER_DONE)
			Dee_Decref(oldvalue);
	}
	return 0;
err_ro:
	err_readonly_key((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
funcsymbolsbyname_setitem(FunctionSymbolsByName *self,
                          DeeObject *key, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func;
	uint16_t rid = FunctionSymbolsByName_GetRefIdByName(self, key);
	func = self->fsbn_func;
	if unlikely(rid == (uint16_t)-1)
		goto err;
	if unlikely(rid < func->fo_code->co_refc)
		goto err_ro;
	Dee_Incref(value);
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	if (ITER_ISOK(oldvalue))
		Dee_Decref(oldvalue);
	return 0;
err_ro:
	err_readonly_key((DeeObject *)self, key);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_ctor(FunctionSymbolsByName *__restrict self) {
	self->fsbn_func = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)&DeeCode_Empty);
	if unlikely(!self->fsbn_func)
		goto err;
	self->fsbn_rid_start = 0;
	self->fsbn_rid_end   = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_copy(FunctionSymbolsByName *__restrict self,
                       FunctionSymbolsByName *__restrict other) {
	self->fsbn_func      = other->fsbn_func;
	self->fsbn_rid_start = other->fsbn_rid_start;
	self->fsbn_rid_end   = other->fsbn_rid_end;
	Dee_Incref(self->fsbn_func);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_init(FunctionSymbolsByName *__restrict self,
                       size_t argc, DeeObject *const *argv) {
	DeeCodeObject *code;
	self->fsbn_rid_start = 0;
	self->fsbn_rid_end   = (uint16_t)-1;
	if (DeeArg_Unpack(argc, argv, "o|" UNPu16 UNPu16 ":FunctionSymbolsByName",
	                  &self->fsbn_func, &self->fsbn_rid_start, &self->fsbn_rid_end))
		goto err;
	if (DeeObject_AssertTypeExact(self->fsbn_func, &DeeFunction_Type))
		goto err;
	code = self->fsbn_func->fo_code;
	if (self->fsbn_rid_end > code->co_refstaticc)
		self->fsbn_rid_end = code->co_refstaticc;
	if (self->fsbn_rid_start < self->fsbn_rid_end)
		self->fsbn_rid_start = self->fsbn_rid_end;
	Dee_Incref(self->fsbn_func);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
funcsymbolsbyname_fini(FunctionSymbolsByName *__restrict self) {
	Dee_Decref(self->fsbn_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
funcsymbolsbyname_visit(FunctionSymbolsByName *__restrict self,
                        dvisit_t proc, void *arg) {
	Dee_Visit(self->fsbn_func);
}



PRIVATE struct type_nsi tpconst funcsymbolsbyname_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&funcsymbolsbyname_nsi_getsize,
			/* .nsi_getsize    = */ (dfunptr_t)&funcsymbolsbyname_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&funcsymbolsbyname_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&funcsymbolsbyname_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&funcsymbolsbyname_nsi_getdefault,
			/* .nsi_setdefault = */ (dfunptr_t)&funcsymbolsbyname_nsi_setdefault,
			/* .nsi_updateold  = */ (dfunptr_t)&funcsymbolsbyname_nsi_updateold,
			/* .nsi_insertnew  = */ (dfunptr_t)&funcsymbolsbyname_nsi_insertnew,
		}
	}
};


PRIVATE struct type_seq funcsymbolsbyname_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&funcsymbolsbyname_setitem,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &funcsymbolsbyname_nsi
};

PRIVATE struct type_member tpconst funcsymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FunctionSymbolsByNameIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst funcsymbolsbyname_members[] = {
	TYPE_MEMBER_FIELD("__func__", STRUCT_OBJECT, offsetof(FunctionSymbolsByName, fsbn_func)),
	TYPE_MEMBER_FIELD("__rid_start__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByName, fsbn_rid_start)),
	TYPE_MEMBER_FIELD("__rid_end__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByName, fsbn_rid_end)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FunctionSymbolsByName_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionSymbolsByName",
	/* .tp_doc      = */ DOC("A ${{(int | string): Object}}-like mapping for named symbols that are "
	                         /**/ "persistent across multiple function invocations. In the presence of "
	                         /**/ "debug information, every slot #Ishould have a name, but when a slot "
	                         /**/ "doesn't have a name for some reason, its merged RID/SID can be used "
	                         /**/ "instead (depending on which ID-range is being referenced).\n"
	                         "\n"
	                         "(function:?DFunction,ridstart=!0,ridend?:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&funcsymbolsbyname_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&funcsymbolsbyname_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&funcsymbolsbyname_init,
				TYPE_FIXED_ALLOCATOR(FunctionSymbolsByName)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&funcsymbolsbyname_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &funcsymbolsbyname_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcsymbolsbyname_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ funcsymbolsbyname_class_members
};





INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetRefsByNameWrapper(DeeFunctionObject *__restrict self) {
	DREF FunctionSymbolsByName *result;
	result = DeeObject_MALLOC(FunctionSymbolsByName);
	if unlikely(!result)
		goto err;
	result->fsbn_func = self;
	Dee_Incref(self);
	result->fsbn_rid_start = 0;
	result->fsbn_rid_end   = self->fo_code->co_refc;
	DeeObject_Init(result, &FunctionSymbolsByName_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetStaticsByNameWrapper(DeeFunctionObject *__restrict self) {
	DREF FunctionSymbolsByName *result;
	result = DeeObject_MALLOC(FunctionSymbolsByName);
	if unlikely(!result)
		goto err;
	result->fsbn_func = self;
	Dee_Incref(self);
	result->fsbn_rid_start = self->fo_code->co_refc;
	result->fsbn_rid_end   = self->fo_code->co_refstaticc;
	DeeObject_Init(result, &FunctionSymbolsByName_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_GetSymbolsByNameWrapper(DeeFunctionObject *__restrict self) {
	DREF FunctionSymbolsByName *result;
	result = DeeObject_MALLOC(FunctionSymbolsByName);
	if unlikely(!result)
		goto err;
	result->fsbn_func = self;
	Dee_Incref(self);
	result->fsbn_rid_start = 0;
	result->fsbn_rid_end   = self->fo_code->co_refstaticc;
	DeeObject_Init(result, &FunctionSymbolsByName_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}




/************************************************************************/
/* ...                                                                  */
/************************************************************************/

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
