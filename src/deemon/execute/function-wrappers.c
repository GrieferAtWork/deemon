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
#include <deemon/util/lock.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/minmax.h>
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
funcstatics_foreach(FunctionStatics *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	size_t i;
	for (i = code->co_refc; i < code->co_refstaticc; ++i) {
		DREF DeeObject *value;
		DeeFunction_RefLockRead(func);
		value = func->fo_refv[i];
		if unlikely(!value) {
			DeeFunction_RefLockEndRead(func);
			continue;
		}
		Dee_Incref(value);
		DeeFunction_RefLockEndRead(func);
		temp = (*proc)(arg, value);
		Dee_Decref_unlikely(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
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
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcstatics_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &funcstatics_nsi,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&funcstatics_foreach,
	/* .tp_foreach_pair       = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&funcstatics_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&funcstatics_nsi_getsize,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&funcstatics_nsi_getitem,
	/* .tp_getitem_index_fast = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&funcstatics_nsi_getitem_fast,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&funcstatics_nsi_delitem,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&funcstatics_nsi_setitem,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_member tpconst funcstatics_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FunctionStaticsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst funcstatics_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FunctionStatics, fs_func), "->?DFunction"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FunctionStatics_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionStatics",
	/* .tp_doc      = */ DOC("(func:?DFunction)"),
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
funcsymbolsbynameiter_nii_getseq(FunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->fsbni_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcsymbolsbynameiter_nii_getindex(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t base   = self->fsbni_seq->fsbn_rid_start;
	uint16_t result = atomic_read(&self->fsbni_rid);
	ASSERT(result >= base);
	return result - base;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbynameiter_nii_setindex(FunctionSymbolsByNameIterator *__restrict self, size_t new_index) {
	uint16_t sid, base = self->fsbni_seq->fsbn_rid_start;
	if (OVERFLOW_UADD(base, new_index, &sid) || sid > self->fsbni_end)
		sid = self->fsbni_end;
	atomic_write(&self->fsbni_rid, sid);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbynameiter_nii_rewind(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t base = self->fsbni_seq->fsbn_rid_start;
	atomic_write(&self->fsbni_rid, base);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbynameiter_eq(FunctionSymbolsByNameIterator *self,
                         FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func == other->fsbni_func &&
	            atomic_read(&self->fsbni_end) == atomic_read(&other->fsbni_end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbynameiter_ne(FunctionSymbolsByNameIterator *self,
                         FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(self->fsbni_func != other->fsbni_func ||
	            atomic_read(&self->fsbni_end) != atomic_read(&other->fsbni_end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbynameiter_lo(FunctionSymbolsByNameIterator *self,
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
funcsymbolsbynameiter_le(FunctionSymbolsByNameIterator *self,
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
funcsymbolsbynameiter_gr(FunctionSymbolsByNameIterator *self,
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
funcsymbolsbynameiter_ge(FunctionSymbolsByNameIterator *self,
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
funcsymbolsbynameiter_ctor(FunctionSymbolsByNameIterator *__restrict self) {
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
funcsymbolsbynameiter_copy(FunctionSymbolsByNameIterator *__restrict self,
                           FunctionSymbolsByNameIterator *__restrict other) {
	self->fsbni_seq  = other->fsbni_seq;
	self->fsbni_func = other->fsbni_func;
	self->fsbni_rid  = atomic_read(&other->fsbni_rid);
	self->fsbni_end  = other->fsbni_end;
	Dee_Incref(self->fsbni_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbynameiter_init(FunctionSymbolsByNameIterator *__restrict self,
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
funcsymbolsbynameiter_fini(FunctionSymbolsByNameIterator *__restrict self) {
	Dee_Decref(self->fsbni_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
funcsymbolsbynameiter_visit(FunctionSymbolsByNameIterator *__restrict self,
                           dvisit_t proc, void *arg) {
	Dee_Visit(self->fsbni_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbynameiter_bool(FunctionSymbolsByNameIterator *__restrict self) {
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
funcsymbolsbynameiter_next(FunctionSymbolsByNameIterator *__restrict self) {
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
funcsymbolsbynameiter_next_key(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t result_sid;
	for (;;) {
		uint16_t old_sid, new_sid;
		old_sid = atomic_read(&self->fsbni_rid);
		new_sid = old_sid;
		if (new_sid >= self->fsbni_end)
			return ITER_DONE;
		result_sid = new_sid;
		++new_sid;
		if (atomic_cmpxch_or_write(&self->fsbni_rid, old_sid, new_sid))
			break;
	}
	return Code_GetRefNameById(self->fsbni_func->fo_code, result_sid);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolsbynameiter_next_value(FunctionSymbolsByNameIterator *__restrict self) {
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

PRIVATE struct type_nii tpconst funcsymbolsbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&funcsymbolsbynameiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&funcsymbolsbynameiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&funcsymbolsbynameiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&funcsymbolsbynameiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp funcsymbolsbynameiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_ge,
	/* .tp_nii  = */ &funcsymbolsbynameiter_nii,
};

PRIVATE struct type_member tpconst funcsymbolsbynameiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(FunctionSymbolsByNameIterator, fsbni_seq), "->?Ert:FunctionSymbolsByName"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FunctionSymbolsByNameIterator, fsbni_func), "->?DFunction"),
	TYPE_MEMBER_FIELD("__rid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByNameIterator, fsbni_rid)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByNameIterator, fsbni_end)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FunctionSymbolsByNameIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionSymbolsByNameIterator",
	/* .tp_doc      = */ DOC("next->?T2?X2?Dstring?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&funcsymbolsbynameiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&funcsymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&funcsymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&funcsymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &funcsymbolsbynameiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcsymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





/* Search for "name" in `self' and return its ID. If not found,
 * *NO* error is thrown, and `(uint16_t)-1' is returned. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) uint16_t DCALL
DDI_GetRefIdByName(DeeDDIObject const *self, char const *name) {
	/* Reference symbol name */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeDDI_Type);
	if (self->d_exdat) {
		uint8_t const *reader;
		char const *str_base;
		str_base = DeeString_STR(self->d_strtab);
		reader   = self->d_exdat->dx_data;
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

#define funcsymbolsbyname_nsi_nextkey   funcsymbolsbynameiter_next_key
#define funcsymbolsbyname_nsi_nextvalue funcsymbolsbynameiter_next_value

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
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&funcsymbolsbyname_setitem,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &funcsymbolsbyname_nsi,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_nsi_getsize,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_member tpconst funcsymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FunctionSymbolsByNameIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst funcsymbolsbyname_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FunctionSymbolsByName, fsbn_func), "->?DFunction"),
	TYPE_MEMBER_FIELD("__ridstart__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByName, fsbn_rid_start)),
	TYPE_MEMBER_FIELD("__ridend__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByName, fsbn_rid_end)),
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
/* YieldFunctionSymbolsByName                                           */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeYieldFunctionObject *yfsbn_yfunc;     /* [1..1][const] The function of the frame (as a cache) */
	uint16_t                     yfsbn_nargs;     /* [<= yfsbn_yfunc->yf_func->fo_code->co_argc_max][const] The # of arguments to enumerate. */
	uint16_t                     yfsbn_rid_start; /* [<= frsbn_rid_end][const] First RID/SID to enumerate. */
	uint16_t                     yfsbn_rid_end;   /* [<= yfsbn_yfunc->yf_func->fo_code->co_refstaticc][const] Last RID/SID to enumerate, plus 1. */
} YieldFunctionSymbolsByName;

typedef union {
	uint32_t yfsbnii_word; /* Index word */
	struct {
		uint16_t i_aid;    /* Next argument index to enumerate */
		uint16_t i_rid;    /* Next reference/static index to enumerate */
	} yfsbnii_idx;
} YieldFunctionSymbolsByNameIteratorIndex;

typedef struct {
	OBJECT_HEAD
	DREF YieldFunctionSymbolsByName        *yfsbni_seq;  /* [1..1][const] Underlying frame-symbols sequence. */
	YieldFunctionSymbolsByNameIteratorIndex yfsbni_idx;  /* Iterator index */
} YieldFunctionSymbolsByNameIterator;

INTDEF DeeTypeObject YieldFunctionSymbolsByNameIterator_Type;
INTDEF DeeTypeObject YieldFunctionSymbolsByName_Type;

PRIVATE WUNUSED NONNULL((1)) DREF YieldFunctionSymbolsByName *DCALL
yfuncsymbolsbynameiter_nii_getseq(YieldFunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->yfsbni_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
yfuncsymbolsbynameiter_nii_getindex(YieldFunctionSymbolsByNameIterator *__restrict self) {
	size_t result;
	YieldFunctionSymbolsByNameIteratorIndex idx;
	idx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	result = (size_t)idx.yfsbnii_idx.i_aid +
	         (size_t)idx.yfsbnii_idx.i_rid;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbynameiter_nii_setindex(YieldFunctionSymbolsByNameIterator *__restrict self, size_t new_index) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex idx;
	if (new_index < sym->yfsbn_nargs) {
		idx.yfsbnii_idx.i_aid = (uint16_t)new_index;
		idx.yfsbnii_idx.i_rid = 0;
	} else if ((new_index -= sym->yfsbn_nargs, new_index < (uint16_t)(sym->yfsbn_rid_end - sym->yfsbn_rid_start))) {
		idx.yfsbnii_idx.i_aid = sym->yfsbn_nargs;
		idx.yfsbnii_idx.i_rid = sym->yfsbn_rid_start + (uint16_t)new_index;
	} else {
		idx.yfsbnii_idx.i_aid = sym->yfsbn_nargs;
		idx.yfsbnii_idx.i_rid = sym->yfsbn_rid_end;
	}
	atomic_write(&self->yfsbni_idx.yfsbnii_word, idx.yfsbnii_word);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbynameiter_nii_rewind(YieldFunctionSymbolsByNameIterator *__restrict self) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex idx;
	idx.yfsbnii_idx.i_aid = 0;
	idx.yfsbnii_idx.i_rid = sym->yfsbn_rid_start;
	atomic_write(&self->yfsbni_idx.yfsbnii_word, idx.yfsbnii_word);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbynameiter_compare(YieldFunctionSymbolsByNameIterator *lhs,
                               YieldFunctionSymbolsByNameIterator *rhs) {
	YieldFunctionSymbolsByNameIteratorIndex lhs_idx;
	YieldFunctionSymbolsByNameIteratorIndex rhs_idx;
	int result;
	if (lhs->yfsbni_seq != rhs->yfsbni_seq)
		return lhs->yfsbni_seq < rhs->yfsbni_seq ? -1 : 1;
	lhs_idx.yfsbnii_word = atomic_read(&lhs->yfsbni_idx.yfsbnii_word);
	rhs_idx.yfsbnii_word = atomic_read(&rhs->yfsbni_idx.yfsbnii_word);
	if (lhs_idx.yfsbnii_idx.i_aid != rhs_idx.yfsbnii_idx.i_aid) {
		result = lhs_idx.yfsbnii_idx.i_aid < rhs_idx.yfsbnii_idx.i_aid ? -1 : 1;
	} else if (lhs_idx.yfsbnii_idx.i_rid != rhs_idx.yfsbnii_idx.i_rid) {
		result = lhs_idx.yfsbnii_idx.i_rid < rhs_idx.yfsbnii_idx.i_rid ? -1 : 1;
	} else {
		result = 0;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_eq(YieldFunctionSymbolsByNameIterator *self,
                          YieldFunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &YieldFunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(yfuncsymbolsbynameiter_compare(self, other) == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_ne(YieldFunctionSymbolsByNameIterator *self,
                          YieldFunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &YieldFunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(yfuncsymbolsbynameiter_compare(self, other) != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_lo(YieldFunctionSymbolsByNameIterator *self,
                          YieldFunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &YieldFunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(yfuncsymbolsbynameiter_compare(self, other) < 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_le(YieldFunctionSymbolsByNameIterator *self,
                          YieldFunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &YieldFunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(yfuncsymbolsbynameiter_compare(self, other) <= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_gr(YieldFunctionSymbolsByNameIterator *self,
                          YieldFunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &YieldFunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(yfuncsymbolsbynameiter_compare(self, other) > 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_ge(YieldFunctionSymbolsByNameIterator *self,
                          YieldFunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &YieldFunctionSymbolsByNameIterator_Type))
		goto err;
	return_bool(yfuncsymbolsbynameiter_compare(self, other) >= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeYieldFunctionObject *DCALL
yfuncsymbolsbynameiter_get_yfunc(YieldFunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->yfsbni_seq->yfsbn_yfunc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
yfuncsymbolsbynameiter_get_func(YieldFunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->yfsbni_seq->yfsbn_yfunc->yf_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbynameiter_copy(YieldFunctionSymbolsByNameIterator *__restrict self,
                            YieldFunctionSymbolsByNameIterator *__restrict other) {
	self->yfsbni_seq = other->yfsbni_seq;
	Dee_Incref(self->yfsbni_seq);
	self->yfsbni_idx.yfsbnii_word = atomic_read(&other->yfsbni_idx.yfsbnii_word);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbynameiter_init(YieldFunctionSymbolsByNameIterator *__restrict self,
                            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:YieldFunctionSymbolsByNameIterator", &self->yfsbni_seq))
		goto err;
	if (DeeObject_AssertTypeExact(self->yfsbni_seq, &YieldFunctionSymbolsByName_Type))
		goto err;
	Dee_Incref(self->yfsbni_seq);
	self->yfsbni_idx.yfsbnii_idx.i_aid = 0;
	self->yfsbni_idx.yfsbnii_idx.i_rid = self->yfsbni_seq->yfsbn_rid_start;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
yfuncsymbolsbynameiter_fini(YieldFunctionSymbolsByNameIterator *__restrict self) {
	Dee_Decref(self->yfsbni_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
yfuncsymbolsbynameiter_visit(YieldFunctionSymbolsByNameIterator *__restrict self,
                             dvisit_t proc, void *arg) {
	Dee_Visit(self->yfsbni_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbynameiter_bool(YieldFunctionSymbolsByNameIterator *__restrict self) {
	bool result;
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex idx;
	idx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	result = (idx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) ||
	         (idx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end);
	return result ? 1 : 0;
}

/* Returns `NULL' if the arg isn't bound. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) DeeObject *DCALL
YieldFunction_GetArg(DeeYieldFunctionObject const *__restrict self, uint16_t aid) {
	DeeObject *result;
	ASSERT(aid < self->yf_func->fo_code->co_argc_max);
	if likely(aid < self->yf_pargc) {
		result = self->yf_argv[aid];
	} else if (self->yf_kw) {
		result = self->yf_kw->fk_kargv[aid - self->yf_pargc];
		if (!result)
			goto use_default;
	} else {
		DeeCodeObject *code;
use_default:
		code   = self->yf_func->fo_code;
		result = code->co_defaultv[aid - code->co_argc_min];
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_next(YieldFunctionSymbolsByNameIterator *__restrict self) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	DREF DeeTupleObject *result;
	DREF DeeObject *name, *value;
	YieldFunctionSymbolsByNameIteratorIndex oldidx, newidx;
	DeeCodeObject *code = sym->yfsbn_yfunc->yf_func->fo_code;
again:
	oldidx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	newidx.yfsbnii_word = oldidx.yfsbnii_word;
	for (;;) {
		if (newidx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) {
			value = YieldFunction_GetArg(sym->yfsbn_yfunc, newidx.yfsbnii_idx.i_aid);
			if (!value) {
				++newidx.yfsbnii_idx.i_aid;
				continue;
			}
			/* Lookup name */
			if (code->co_keywords) {
				name = (DREF DeeObject *)code->co_keywords[newidx.yfsbnii_idx.i_aid];
				Dee_Incref(name);
			} else {
				name = DeeInt_NewUInt16(newidx.yfsbnii_idx.i_aid);
				if unlikely(!name)
					goto err;
			}
			Dee_Incref(value);
			++newidx.yfsbnii_idx.i_aid;
			break;
		}
		if (newidx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end) {
			char const *refname;
			DeeFunctionObject *func = sym->yfsbn_yfunc->yf_func;
			DeeFunction_RefLockRead(func);
			value = func->fo_refv[newidx.yfsbnii_idx.i_rid];
			if (!ITER_ISOK(value)) {
				DeeFunction_RefLockEndRead(func);
				++newidx.yfsbnii_idx.i_rid;
				continue;
			}
			Dee_Incref(value);
			DeeFunction_RefLockEndRead(func);
			refname = DeeCode_GetRSymbolName((DeeObject *)code, newidx.yfsbnii_idx.i_rid);
			name = refname ? DeeString_New(refname)
			               : DeeInt_NewUInt16(code->co_argc_max + newidx.yfsbnii_idx.i_rid);
			if unlikely(!name)
				goto err_value;
			++newidx.yfsbnii_idx.i_rid;
			break;
		}
		return ITER_DONE;
	}
	if (!atomic_cmpxch_or_write(&self->yfsbni_idx.yfsbnii_word,
	                            oldidx.yfsbnii_word,
	                            newidx.yfsbnii_word)) {
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(name);
		goto again;
	}
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_value_name;
	DeeTuple_SET(result, 0, name);  /* Inherit reference */
	DeeTuple_SET(result, 1, value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_value_name:
	Dee_Decref(name);
err_value:
	Dee_Decref(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_next_key(YieldFunctionSymbolsByNameIterator *__restrict self) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	DeeCodeObject *code = sym->yfsbn_yfunc->yf_func->fo_code;
	DREF DeeObject *name;
	YieldFunctionSymbolsByNameIteratorIndex oldidx, newidx;
again:
	oldidx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	newidx.yfsbnii_word = oldidx.yfsbnii_word;
	for (;;) {
		if (newidx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) {
			/* Lookup name */
			if (code->co_keywords) {
				name = (DREF DeeObject *)code->co_keywords[newidx.yfsbnii_idx.i_aid];
				Dee_Incref(name);
			} else {
				name = DeeInt_NewUInt16(newidx.yfsbnii_idx.i_aid);
				if unlikely(!name)
					goto err;
			}
			++newidx.yfsbnii_idx.i_aid;
			break;
		}
		if (newidx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end) {
			char const *refname;
			refname = DeeCode_GetRSymbolName((DeeObject *)code, newidx.yfsbnii_idx.i_rid);
			name = refname ? DeeString_New(refname)
			               : DeeInt_NewUInt16(code->co_argc_max + newidx.yfsbnii_idx.i_rid);
			if unlikely(!name)
				goto err;
			++newidx.yfsbnii_idx.i_rid;
			break;
		}
		return ITER_DONE;
	}
	if (!atomic_cmpxch_or_write(&self->yfsbni_idx.yfsbnii_word,
	                            oldidx.yfsbnii_word,
	                            newidx.yfsbnii_word)) {
		Dee_Decref_unlikely(name);
		goto again;
	}
	return name;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_next_value(YieldFunctionSymbolsByNameIterator *__restrict self) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	DREF DeeObject *value;
	YieldFunctionSymbolsByNameIteratorIndex oldidx, newidx;
again:
	oldidx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	newidx.yfsbnii_word = oldidx.yfsbnii_word;
	for (;;) {
		if (newidx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) {
			value = YieldFunction_GetArg(sym->yfsbn_yfunc, newidx.yfsbnii_idx.i_aid);
			if (!value) {
				++newidx.yfsbnii_idx.i_aid;
				continue;
			}
			Dee_Incref(value);
			++newidx.yfsbnii_idx.i_aid;
			break;
		}
		if (newidx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end) {
			DeeFunctionObject *func = sym->yfsbn_yfunc->yf_func;
			DeeFunction_RefLockRead(func);
			value = func->fo_refv[newidx.yfsbnii_idx.i_rid];
			if (!ITER_ISOK(value)) {
				DeeFunction_RefLockEndRead(func);
				++newidx.yfsbnii_idx.i_rid;
				continue;
			}
			Dee_Incref(value);
			DeeFunction_RefLockEndRead(func);
			++newidx.yfsbnii_idx.i_rid;
			break;
		}
		return ITER_DONE;
	}
	if (!atomic_cmpxch_or_write(&self->yfsbni_idx.yfsbnii_word,
	                            oldidx.yfsbnii_word,
	                            newidx.yfsbnii_word)) {
		Dee_Decref_unlikely(value);
		goto again;
	}
	return value;
/*
err:
	return NULL;*/
}

PRIVATE struct type_nii tpconst yfuncsymbolsbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&yfuncsymbolsbynameiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&yfuncsymbolsbynameiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&yfuncsymbolsbynameiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&yfuncsymbolsbynameiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp yfuncsymbolsbynameiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_ge,
	/* .tp_nii  = */ &yfuncsymbolsbynameiter_nii,
};

PRIVATE struct type_getset tpconst yfuncsymbolsbynameiter_getsets[] = {
	TYPE_GETTER("__yfunc__", &yfuncsymbolsbynameiter_get_yfunc, "->?Ert:YieldFunction"),
	TYPE_GETTER("__func__", &yfuncsymbolsbynameiter_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst yfuncsymbolsbynameiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_seq), "->?Ert:YieldFunctionSymbolsByName"),
	TYPE_MEMBER_FIELD("__aid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_idx.yfsbnii_idx.i_aid)),
	TYPE_MEMBER_FIELD("__rid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_idx.yfsbnii_idx.i_rid)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject YieldFunctionSymbolsByNameIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionSymbolsByNameIterator",
	/* .tp_doc      = */ DOC("next->?T2?X2?Dstring?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&yfuncsymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&yfuncsymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(YieldFunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&yfuncsymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &yfuncsymbolsbynameiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yfuncsymbolsbynameiter_getsets,
	/* .tp_members       = */ yfuncsymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
yfuncsymbolsbyname_nsi_getsize(YieldFunctionSymbolsByName *__restrict self) {
	size_t result;
	result = self->yfsbn_nargs;
	result += self->yfsbn_rid_end - self->yfsbn_rid_start;
	return result;
}

typedef uint32_t yfuncsymbol_t;
#define YFUNCSYMBOL_ERROR        0x1ffff
#define YFUNCSYMBOL_INVALID      0xffff
#define yfuncsymbol_makearg(aid) (yfuncsymbol_t)(aid)
#define yfuncsymbol_makeref(rid) (yfuncsymbol_t)((rid) | 0x10000)
#define yfuncsymbol_asaid(symid) ((uint16_t)(symid))
#define yfuncsymbol_asrid(symid) ((uint16_t)(symid))
#if 1
#define yfuncsymbol_isaid(symid) ((symid) < 0x10000)
#define yfuncsymbol_isrid(symid) ((symid) >= 0x10000)
#else
#define yfuncsymbol_isaid(symid) (((symid) & 0x10000) == 0)
#define yfuncsymbol_isrid(symid) (((symid) & 0x10000) != 0)
#endif

STATIC_ASSERT(yfuncsymbol_isaid(yfuncsymbol_makearg(0)));
STATIC_ASSERT(!yfuncsymbol_isrid(yfuncsymbol_makearg(0)));
STATIC_ASSERT(!yfuncsymbol_isaid(yfuncsymbol_makeref(0)));
STATIC_ASSERT(yfuncsymbol_isrid(yfuncsymbol_makeref(0)));

/* Try to find the symbol referenced by `name'
 * @return: YFUNCSYMBOL_INVALID: No such symbol "name" */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_TryName2Sym(YieldFunctionSymbolsByName const *self,
                                       char const *name) {
	uint16_t i;
	DeeCodeObject *code = self->yfsbn_yfunc->yf_func->fo_code;
	if (code->co_keywords) {
		for (i = 0; i < self->yfsbn_nargs; ++i) {
			DeeStringObject *argi = code->co_keywords[i];
			if (strcmp(DeeString_STR(argi), name) == 0)
				return yfuncsymbol_makearg(i);
		}
	}
	i = DDI_GetRefIdByName(code->co_ddi, name);
	if (i >= self->yfsbn_rid_start && i < self->yfsbn_rid_end)
		return yfuncsymbol_makeref(i);
	return YFUNCSYMBOL_INVALID;
}

/* Try to find the symbol referenced by `name'
 * @return: YFUNCSYMBOL_INVALID: No such symbol "key"
 * @return: YFUNCSYMBOL_ERROR:   An error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_TryKey2Sym(YieldFunctionSymbolsByName const *self,
                                      DeeObject *key) {
	uint32_t index;
	DeeCodeObject *code;
	if (DeeString_Check(key))
		return YieldFunctionSymbolsByName_TryName2Sym(self, DeeString_STR(key));
	if (DeeObject_AsUIntX(key, &index))
		goto err;
	code = self->yfsbn_yfunc->yf_func->fo_code;
	if (index < code->co_argc_max) {
		if (index >= self->yfsbn_nargs)
			goto badsym;
		return yfuncsymbol_makearg(index);
	}
	index -= code->co_argc_max;
	if (index >= self->yfsbn_rid_start && index < self->yfsbn_rid_end)
		return yfuncsymbol_makeref(index);
badsym:
	return YFUNCSYMBOL_INVALID;
err:
	return YFUNCSYMBOL_ERROR;
}

/* Same as `YieldFunctionSymbolsByName_TryKey2Sym',
 * but throw an error in case of a bad key.
 * @return: YFUNCSYMBOL_ERROR: An error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_Key2Sym(YieldFunctionSymbolsByName const *self,
                                   DeeObject *key) {
	yfuncsymbol_t result = YieldFunctionSymbolsByName_TryKey2Sym(self, key);
	if unlikely(result == YFUNCSYMBOL_INVALID) {
		err_unknown_key((DeeObject *)self, key);
		result = YFUNCSYMBOL_ERROR;
	}
	return result;
}

/* Returns `NULL' if the symbol isn't bound. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
YieldFunction_TryGetSymbol(DeeYieldFunctionObject *self, yfuncsymbol_t sym) {
	ASSERT(sym != YFUNCSYMBOL_ERROR);
	if (yfuncsymbol_isaid(sym)) {
		DeeObject *result;
		result = YieldFunction_GetArg(self, yfuncsymbol_asaid(sym));
		return_reference_(result);
	} else {
		DREF DeeObject *result;
		DeeFunctionObject *func = self->yf_func;
		uint16_t rid = yfuncsymbol_asrid(sym);
		ASSERT(rid < func->fo_code->co_refstaticc);
		DeeFunction_RefLockRead(func);
		result = func->fo_refv[rid];
		if (ITER_ISOK(result)) {
			Dee_Incref(result);
		} else {
			result = NULL;
		}
		DeeFunction_RefLockEndRead(func);
		return result;
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
yfuncsymbolsbyname_nsi_getdefault(YieldFunctionSymbolsByName *self,
                                  DeeObject *key, DeeObject *defl) {
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_TryKey2Sym(self, key);
	if (symid != YFUNCSYMBOL_INVALID) {
		DREF DeeObject *result;
		if unlikely(symid == YFUNCSYMBOL_ERROR)
			goto err;
		result = YieldFunction_TryGetSymbol(self->yfsbn_yfunc, symid);
		if (result != NULL)
			return result;
	}
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
yfuncsymbolsbyname_nsi_setdefault(YieldFunctionSymbolsByName *self,
                                  DeeObject *key, DeeObject *defl) {
	DeeFunctionObject *func;
	DREF DeeObject *result;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_Key2Sym(self, key);
	uint16_t rid;
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid) ||
	            (rid = yfuncsymbol_asrid(symid)) < func->fo_code->co_refc) {
		err_readonly_key((DeeObject *)self, key);
		goto err;
	}
	DeeFunction_RefLockWrite(func);
	result = func->fo_refv[rid];
	if (ITER_ISOK(result)) {
		Dee_Incref(result);
		DeeFunction_RefLockEndWrite(func);
		return result;
	}
	Dee_Incref_n(defl, 2);
	func->fo_refv[rid] = defl;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	result = defl;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
yfuncsymbolsbyname_nsi_updateold(YieldFunctionSymbolsByName *self,
                                 DeeObject *key, DeeObject *value,
                                 DREF DeeObject **p_oldvalue) {
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_TryKey2Sym(self, key);
	uint16_t rid;
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	if unlikely(symid == YFUNCSYMBOL_INVALID)
		return 0;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid) ||
	            (rid = yfuncsymbol_asrid(symid)) < func->fo_code->co_refc) {
		err_readonly_key((DeeObject *)self, key);
		goto err;
	}
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	if (ITER_ISOK(oldvalue)) {
		Dee_Incref(value);
		func->fo_refv[rid] = value;
		DeeFunction_RefLockEndWrite(func);
		DeeFutex_WakeAll(&func->fo_refv[rid]);
		if (p_oldvalue) {
			*p_oldvalue = oldvalue;
		} else {
			Dee_Decref(oldvalue);
		}
		return 1;
	}
	DeeFunction_RefLockEndWrite(func);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
yfuncsymbolsbyname_nsi_insertnew(YieldFunctionSymbolsByName *self,
                                 DeeObject *key, DeeObject *value,
                                 DREF DeeObject **p_oldvalue) {
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_Key2Sym(self, key);
	uint16_t rid;
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid) ||
	            (rid = yfuncsymbol_asrid(symid)) < func->fo_code->co_refc) {
		err_readonly_key((DeeObject *)self, key);
		goto err;
	}
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
	Dee_Incref(value);
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	return 0;
err:
	return -1;
}

#define yfuncsymbolsbyname_nsi_nextkey   yfuncsymbolsbynameiter_next_key
#define yfuncsymbolsbyname_nsi_nextvalue yfuncsymbolsbynameiter_next_value

PRIVATE WUNUSED NONNULL((1)) DREF YieldFunctionSymbolsByNameIterator *DCALL
yfuncsymbolsbyname_iter(YieldFunctionSymbolsByName *__restrict self) {
	DREF YieldFunctionSymbolsByNameIterator *result;
	result = DeeObject_MALLOC(YieldFunctionSymbolsByNameIterator);
	if unlikely(!result)
		goto err;
	result->yfsbni_seq = self;
	Dee_Incref(self);
	result->yfsbni_idx.yfsbnii_idx.i_aid = 0;
	result->yfsbni_idx.yfsbnii_idx.i_rid = self->yfsbn_rid_start;
	DeeObject_Init(result, &YieldFunctionSymbolsByNameIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbyname_contains(YieldFunctionSymbolsByName *self,
                            DeeObject *key) {
	yfuncsymbol_t symid;
	symid = YieldFunctionSymbolsByName_TryKey2Sym(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	return_bool(symid != YFUNCSYMBOL_INVALID);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbyname_getitem(YieldFunctionSymbolsByName *self,
                           DeeObject *key) {
	DREF DeeObject *result;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_Key2Sym(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	result = YieldFunction_TryGetSymbol(self->yfsbn_yfunc, symid);
	if unlikely(!result)
		err_unbound_key((DeeObject *)self, key);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
yfuncsymbolsbyname_setitem(YieldFunctionSymbolsByName *self,
                          DeeObject *key, DeeObject *value) {
	uint16_t rid;
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_Key2Sym(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid) ||
	            (rid = yfuncsymbol_asrid(symid)) < func->fo_code->co_refc) {
		err_readonly_key((DeeObject *)self, key);
		goto err;
	}
	Dee_Incref(value);
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	if (ITER_ISOK(oldvalue))
		Dee_Incref(oldvalue);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbyname_delitem(YieldFunctionSymbolsByName *self,
                           DeeObject *key) {
	uint16_t rid;
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_Key2Sym(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid) ||
	            (rid = yfuncsymbol_asrid(symid)) < func->fo_code->co_refc) {
		err_readonly_key((DeeObject *)self, key);
		goto err;
	}
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	func->fo_refv[rid] = NULL;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	if (ITER_ISOK(oldvalue))
		Dee_Incref(oldvalue);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbyname_copy(YieldFunctionSymbolsByName *__restrict self,
                        YieldFunctionSymbolsByName *__restrict other) {
	self->yfsbn_yfunc = other->yfsbn_yfunc;
	Dee_Incref(self->yfsbn_yfunc);
	self->yfsbn_nargs     = other->yfsbn_nargs;
	self->yfsbn_rid_start = other->yfsbn_rid_start;
	self->yfsbn_rid_end   = other->yfsbn_rid_end;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbyname_init_kw(YieldFunctionSymbolsByName *__restrict self,
                           size_t argc, DeeObject *const *argv,
                           DeeObject *kw) {
	DeeCodeObject *code;
	PRIVATE DEFINE_KWLIST(kwlist, { K(yfunc), K(argc), K(ridstart), K(ridend), KEND });
	self->yfsbn_nargs     = (uint16_t)-1;
	self->yfsbn_rid_start = 0;
	self->yfsbn_rid_end   = (uint16_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|" UNPu16 UNPu16 UNPu16,
	                    &self->yfsbn_yfunc, &self->yfsbn_nargs,
	                    &self->yfsbn_rid_start, &self->yfsbn_rid_end))
		goto err;
	if (DeeObject_AssertTypeExact(self->yfsbn_yfunc, &DeeYieldFunction_Type))
		goto err;
	code = self->yfsbn_yfunc->yf_func->fo_code;
	if (self->yfsbn_nargs > code->co_argc_max)
		self->yfsbn_nargs = code->co_argc_max;
	if (self->yfsbn_rid_end > code->co_refstaticc)
		self->yfsbn_rid_end = code->co_refstaticc;
	if (self->yfsbn_rid_start > self->yfsbn_rid_end)
		self->yfsbn_rid_start = self->yfsbn_rid_end;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
yfuncsymbolsbyname_fini(YieldFunctionSymbolsByName *__restrict self) {
	Dee_Decref(self->yfsbn_yfunc);
}

PRIVATE NONNULL((1, 2)) void DCALL
yfuncsymbolsbyname_visit(YieldFunctionSymbolsByName *__restrict self,
                         dvisit_t proc, void *arg) {
	Dee_Visit(self->yfsbn_yfunc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
yfuncsymbolsbyname_getfunc(YieldFunctionSymbolsByName *__restrict self) {
	return_reference_(self->yfsbn_yfunc->yf_func);
}



PRIVATE struct type_nsi tpconst yfuncsymbolsbyname_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_getdefault,
			/* .nsi_setdefault = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_setdefault,
			/* .nsi_updateold  = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_updateold,
			/* .nsi_insertnew  = */ (dfunptr_t)&yfuncsymbolsbyname_nsi_insertnew,
		}
	}
};

PRIVATE struct type_seq yfuncsymbolsbyname_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&yfuncsymbolsbyname_setitem,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &yfuncsymbolsbyname_nsi,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_nsi_getsize,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_member tpconst yfuncsymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &YieldFunctionSymbolsByNameIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst yfuncsymbolsbyname_getsets[] = {
	TYPE_GETTER("__func__", &yfuncsymbolsbyname_getfunc, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst yfuncsymbolsbyname_members[] = {
	TYPE_MEMBER_FIELD_DOC("__yfunc__", STRUCT_OBJECT, offsetof(YieldFunctionSymbolsByName, yfsbn_yfunc), "->?Ert:YieldFunction"),
	TYPE_MEMBER_FIELD("__nargs__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByName, yfsbn_nargs)),
	TYPE_MEMBER_FIELD("__ridstart__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByName, yfsbn_rid_start)),
	TYPE_MEMBER_FIELD("__ridend__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByName, yfsbn_rid_end)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject YieldFunctionSymbolsByName_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionSymbolsByName",
	/* .tp_doc      = */ DOC("A ${{(int | string): Object}}-like mapping for references, statics, and arguments.\n"
	                         "\n"
	                         "(yfunc:?Ert:YieldFunction,argc?:?Dint,ridstart=!0,ridend?:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&yfuncsymbolsbyname_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(YieldFunctionSymbolsByName),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&yfuncsymbolsbyname_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&yfuncsymbolsbyname_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &yfuncsymbolsbyname_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yfuncsymbolsbyname_getsets,
	/* .tp_members       = */ yfuncsymbolsbyname_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ yfuncsymbolsbyname_class_members
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeYieldFunction_GetArgsByNameWrapper(DeeYieldFunctionObject *__restrict self) {
	DeeCodeObject *code;
	DREF YieldFunctionSymbolsByName *result;
	result = DeeObject_MALLOC(YieldFunctionSymbolsByName);
	if unlikely(!result)
		goto err;
	result->yfsbn_yfunc = self;
	Dee_Incref(self);
	code = self->yf_func->fo_code;
	result->yfsbn_nargs = code->co_argc_max;
	result->yfsbn_rid_start = 0;
	result->yfsbn_rid_end   = 0;
	DeeObject_Init(result, &YieldFunctionSymbolsByName_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeYieldFunction_GetSymbolsByNameWrapper(DeeYieldFunctionObject *__restrict self) {
	DeeCodeObject *code;
	DREF YieldFunctionSymbolsByName *result;
	result = DeeObject_MALLOC(YieldFunctionSymbolsByName);
	if unlikely(!result)
		goto err;
	result->yfsbn_yfunc = self;
	Dee_Incref(self);
	code = self->yf_func->fo_code;
	result->yfsbn_nargs     = code->co_argc_max;
	result->yfsbn_rid_start = 0;
	result->yfsbn_rid_end   = code->co_refstaticc;
	DeeObject_Init(result, &YieldFunctionSymbolsByName_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}




/************************************************************************/
/* FrameArgs                                                            */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeFrameObject *fa_frame; /* [1..1][const] The frame in question */
	DREF DeeCodeObject  *fa_code;  /* [1..1][const] The code running in `fa_frame' (cache) */
} FrameArgs;

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
frameargs_nsi_getsize(FrameArgs *__restrict self) {
	return self->fa_code->co_argc_max;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frameargs_nsi_getitem(FrameArgs *__restrict self, size_t index) {
	struct code_frame const *frame;
	DREF DeeObject *result;
	DeeCodeObject *code = self->fa_code;
	if (index >= code->co_argc_max) {
		err_index_out_of_bounds((DeeObject *)self, index, code->co_argc_max);
		goto err;
	}
	frame = DeeFrame_LockRead((DeeObject *)self->fa_frame);
	if unlikely(!frame)
		goto err;
	if likely(index < frame->cf_argc) {
		result = frame->cf_argv[index];
	} else if (frame->cf_kw) {
		result = frame->cf_kw->fk_kargv[index - frame->cf_argc];
		if (!result)
			goto use_default;
	} else {
use_default:
		result = code->co_defaultv[index - code->co_argc_min];
		if (!result) {
			DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
			err_unbound_index((DeeObject *)self, index);
			goto err;
		}
	}
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
frameargs_get_func(FrameArgs *__restrict self) {
	struct code_frame const *frame;
	DREF DeeFunctionObject *result;
	frame = DeeFrame_LockRead((DeeObject *)self->fa_frame);
	if unlikely(!frame)
		goto err;
	result = frame->cf_func;
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
frameargs_copy(FrameArgs *__restrict self,
               FrameArgs *__restrict other) {
	self->fa_frame = other->fa_frame;
	self->fa_code  = other->fa_code;
	Dee_Incref(self->fa_frame);
	Dee_Incref(self->fa_code);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
frameargs_init(FrameArgs *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	struct code_frame const *frame;
	if (DeeArg_Unpack(argc, argv, "o:FrameArgs", &self->fa_frame))
		goto err;
	if (DeeObject_AssertTypeExact(self->fa_frame, &DeeFrame_Type))
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)self->fa_frame);
	if unlikely(!frame)
		goto err;
	self->fa_code = frame->cf_func->fo_code;
	Dee_Incref(self->fa_code);
	DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
	Dee_Incref(self->fa_frame);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
frameargs_fini(FrameArgs *__restrict self) {
	Dee_Decref(self->fa_frame);
	Dee_Decref(self->fa_code);
}

PRIVATE NONNULL((1, 2)) void DCALL
frameargs_visit(FrameArgs *__restrict self,
                dvisit_t proc, void *arg) {
	Dee_Visit(self->fa_frame);
	Dee_Visit(self->fa_code);
}


PRIVATE struct type_nsi tpconst frameargs_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&frameargs_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&frameargs_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&frameargs_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
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


PRIVATE struct type_seq frameargs_seq = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &frameargs_nsi,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&frameargs_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&frameargs_nsi_getsize,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&frameargs_nsi_getitem,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_getset tpconst frameargs_getsets[] = {
	TYPE_GETTER("__func__", &frameargs_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst frameargs_members[] = {
	TYPE_MEMBER_FIELD_DOC("__frame__", STRUCT_OBJECT, offsetof(FrameArgs, fa_frame), "->?Ert:Frame"),
	TYPE_MEMBER_FIELD_DOC("__code__", STRUCT_OBJECT, offsetof(FrameArgs, fa_code), "->?Ert:Code"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FrameArgs_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FrameArgs",
	/* .tp_doc      = */ DOC("(frame:?Ert:Frame)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&frameargs_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&frameargs_init,
				TYPE_FIXED_ALLOCATOR(FrameArgs)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&frameargs_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&frameargs_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &frameargs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ frameargs_getsets,
	/* .tp_members       = */ frameargs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetArgsWrapper(DeeFrameObject *__restrict self) {
	struct code_frame const *frame;
	DREF FrameArgs *result;
	result = DeeObject_MALLOC(FrameArgs);
	if unlikely(!result)
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)self);
	if unlikely(!frame)
		goto err_r;
	result->fa_code = frame->cf_func->fo_code;
	Dee_Incref(result->fa_code);
	DeeFrame_LockEndRead((DeeObject *)self);
	result->fa_frame = self;
	Dee_Incref(self);
	DeeObject_Init(result, &FrameArgs_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/************************************************************************/
/* FrameLocals                                                          */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeFrameObject *fl_frame;  /* [1..1][const] The frame in question */
	uint16_t             fl_localc; /* [const] The # of local variables there are (cache) */
} FrameLocals;

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framelocals_nsi_getsize(FrameLocals *__restrict self) {
	return self->fl_localc;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framelocals_nsi_getitem(FrameLocals *__restrict self, size_t index) {
	struct code_frame const *frame;
	DREF DeeObject *result;
	if unlikely(index >= self->fl_localc) {
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_localc);
		goto err;
	}
	frame = DeeFrame_LockRead((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	result = frame->cf_frame[index];
	if unlikely(!result) {
		DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framelocals_nsi_delitem(FrameLocals *__restrict self, size_t index) {
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	if unlikely(index >= self->fl_localc) {
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_localc);
		goto err;
	}
	frame = DeeFrame_LockWrite((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	oldvalue = frame->cf_frame[index];
	frame->cf_frame[index] = NULL;
	DeeFrame_LockEndWrite((DeeObject *)self->fl_frame);
	Dee_XDecref(oldvalue);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
framelocals_nsi_setitem(FrameLocals *self, size_t index, DeeObject *value) {
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	if unlikely(index >= self->fl_localc) {
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_localc);
		goto err;
	}
	frame = DeeFrame_LockWrite((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	Dee_Incref(value);
	oldvalue = frame->cf_frame[index];
	frame->cf_frame[index] = value;
	DeeFrame_LockEndWrite((DeeObject *)self->fl_frame);
	Dee_XDecref(oldvalue);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
framelocals_nsi_xchitem(FrameLocals *self, size_t index, DeeObject *value) {
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	if unlikely(index >= self->fl_localc) {
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_localc);
		goto err;
	}
	frame = DeeFrame_LockWrite((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	oldvalue = frame->cf_frame[index];
	if unlikely(!oldvalue) {
		DeeFrame_LockEndWrite((DeeObject *)self->fl_frame);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	frame->cf_frame[index] = value; /* Inherit reference (x2) */
	DeeFrame_LockEndWrite((DeeObject *)self->fl_frame);
	return oldvalue;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
framelocals_get_func(FrameLocals *__restrict self) {
	struct code_frame const *frame;
	DREF DeeFunctionObject *result;
	frame = DeeFrame_LockRead((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	result = frame->cf_func;
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framelocals_copy(FrameLocals *__restrict self,
                 FrameLocals *__restrict other) {
	self->fl_frame  = other->fl_frame;
	self->fl_localc = other->fl_localc;
	Dee_Incref(self->fl_frame);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framelocals_init(FrameLocals *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	struct code_frame const *frame;
	if (DeeArg_Unpack(argc, argv, "o:FrameLocals", &self->fl_frame))
		goto err;
	if (DeeObject_AssertTypeExact(self->fl_frame, &DeeFrame_Type))
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	self->fl_localc = frame->cf_func->fo_code->co_localc;
	DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
	Dee_Incref(self->fl_frame);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
framelocals_fini(FrameLocals *__restrict self) {
	Dee_Decref(self->fl_frame);
}

PRIVATE NONNULL((1, 2)) void DCALL
framelocals_visit(FrameLocals *__restrict self,
                  dvisit_t proc, void *arg) {
	Dee_Visit(self->fl_frame);
}


PRIVATE struct type_nsi tpconst framelocals_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&framelocals_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&framelocals_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&framelocals_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)&framelocals_nsi_delitem,
			/* .nsi_setitem      = */ (dfunptr_t)&framelocals_nsi_setitem,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)&framelocals_nsi_xchitem,
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


PRIVATE struct type_seq framelocals_seq = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &framelocals_nsi,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&framelocals_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&framelocals_nsi_getsize,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&framelocals_nsi_getitem,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&framelocals_nsi_delitem,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&framelocals_nsi_setitem,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_getset tpconst framelocals_getsets[] = {
	TYPE_GETTER("__func__", &framelocals_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst framelocals_members[] = {
	TYPE_MEMBER_FIELD_DOC("__frame__", STRUCT_OBJECT, offsetof(FrameLocals, fl_frame), "->?Ert:Frame"),
	TYPE_MEMBER_FIELD("__localc__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameLocals, fl_localc)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FrameLocals_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FrameLocals",
	/* .tp_doc      = */ DOC("(frame:?Ert:Frame)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&framelocals_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&framelocals_init,
				TYPE_FIXED_ALLOCATOR(FrameLocals)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framelocals_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&framelocals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &framelocals_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ framelocals_getsets,
	/* .tp_members       = */ framelocals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetLocalsWrapper(DeeFrameObject *__restrict self) {
	struct code_frame const *frame;
	DREF FrameLocals *result;
	result = DeeObject_MALLOC(FrameLocals);
	if unlikely(!result)
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)self);
	if unlikely(!frame)
		goto err_r;
	result->fl_localc = frame->cf_func->fo_code->co_localc;
	DeeFrame_LockEndRead((DeeObject *)self);
	result->fl_frame = self;
	Dee_Incref(self);
	DeeObject_Init(result, &FrameLocals_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/************************************************************************/
/* FrameStack                                                           */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeFrameObject *fs_frame;  /* [1..1][const] The frame in question */
} FrameStack;

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framestack_nsi_getsize(FrameStack *__restrict self) {
	uint16_t result;
	struct code_frame const *frame;
	frame = DeeFrame_LockRead((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	result = Dee_code_frame_getspaddr(frame);
	DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framestack_nsi_getitem(FrameStack *__restrict self, size_t index) {
	uint16_t stackc;
	struct code_frame const *frame;
	DREF DeeObject *result;
	frame = DeeFrame_LockRead((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	stackc = Dee_code_frame_getspaddr(frame);
	if unlikely(index >= stackc) {
		DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
		err_index_out_of_bounds((DeeObject *)self, index, stackc);
		goto err;
	}
	result = frame->cf_stack[index];
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
framestack_nsi_xchitem(FrameStack *self, size_t index, DeeObject *value) {
	uint16_t stackc;
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	/* Lock in assembly-mode, since the stack may contain instruction pointers. */
	frame = DeeFrame_LockWriteAssembly((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	stackc = Dee_code_frame_getspaddr(frame);
	if unlikely(index >= stackc) {
		DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
		err_index_out_of_bounds((DeeObject *)self, index, stackc);
		goto err;
	}
	Dee_Incref(value);
	oldvalue = frame->cf_stack[index];
	frame->cf_stack[index] = value; /* Inherit (x2) */
	DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
	return oldvalue;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
framestack_nsi_setitem(FrameStack *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	oldvalue = framestack_nsi_xchitem(self, index, value);
	if unlikely(!oldvalue)
		goto err;
	Dee_Decref(oldvalue);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
framestack_get_func(FrameStack *__restrict self) {
	struct code_frame const *frame;
	DREF DeeFunctionObject *result;
	frame = DeeFrame_LockRead((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	result = frame->cf_func;
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framestack_copy(FrameStack *__restrict self,
                FrameStack *__restrict other) {
	self->fs_frame = other->fs_frame;
	Dee_Incref(self->fs_frame);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framestack_init(FrameStack *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:FrameStack", &self->fs_frame))
		goto err;
	if (DeeObject_AssertTypeExact(self->fs_frame, &DeeFrame_Type))
		goto err;
	Dee_Incref(self->fs_frame);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
framestack_fini(FrameStack *__restrict self) {
	Dee_Decref(self->fs_frame);
}

PRIVATE NONNULL((1, 2)) void DCALL
framestack_visit(FrameStack *__restrict self,
                 dvisit_t proc, void *arg) {
	Dee_Visit(self->fs_frame);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
framestack_nsi_insert(FrameStack *self, size_t index, DeeObject *value) {
	uint16_t stackc, stacka;
	struct code_frame *frame;
	DREF DeeObject **stackv;
	DeeCodeObject *code;

	/* Lock in assembly-mode, since the stack may contain instruction pointers. */
again:
	frame = DeeFrame_LockWriteAssembly((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	stackc = Dee_code_frame_getspaddr(frame);
	if (index > (size_t)stackc)
		index = (size_t)stackc;
	code   = frame->cf_func->fo_code;
	stacka = frame->cf_stacksz;
	if (stacka == 0)
		stacka = (uint16_t)((code->co_framesize / sizeof(DeeObject *)) - code->co_localc);

	/* Ensure that there is enough space on the stack. */
	if (stackc >= stacka) {
		DREF DeeObject **new_stackv;
		uint16_t new_stacka;
		/* Must allocate a larger stack (only allowed if the code has the "CODE_FLENIENT" flag) */
		if (!(code->co_flags & CODE_FLENIENT))
			goto err_stack_too_large_endwrite;
		new_stacka = stacka * 2;
		if unlikely(new_stacka < stackc)
			new_stacka = stackc + 1;
		if unlikely(new_stacka < stackc)
			goto err_stack_too_large_endwrite;
		new_stackv = (DREF DeeObject **)Dee_TryMallocc(new_stacka, sizeof(DREF DeeObject *));
		if unlikely(!new_stackv) {
			DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
			if (Dee_CollectMemoryc(new_stacka, sizeof(DREF DeeObject *)))
				goto again;
			goto err;
		}

		/* Transfer objects to the new stack. */
		new_stackv = (DREF DeeObject **)memcpyc(new_stackv, frame->cf_stack, stackc,
		                                        sizeof(DREF DeeObject *));

		/* Store the new stack within the frame. */
		if (frame->cf_stacksz)
			Dee_Free(frame->cf_stack);
		frame->cf_stack   = new_stackv;
		frame->cf_stacksz = stacka = new_stacka;
		frame->cf_sp      = new_stackv + stackc;
	}

	/* Inject the caller-given "value" into the stack at address=index */
	stackv = frame->cf_stack;
	memmoveupc(stackv + index + 1,
	           stackv + index,
	           stackc - index,
	           sizeof(DREF DeeObject *));
	stackv[index] = value;
	Dee_Incref(value);

	/* Increase the stack pointer to reflect the now-larger stack. */
	++frame->cf_sp;
	DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
	return 0;
err_stack_too_large_endwrite:
	DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
/*err_stack_too_large:*/
	DeeError_Throwf(&DeeError_SegFault, "Stack segment overflow");
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framestack_nsi_pop(FrameStack *self, dssize_t index) {
	uint16_t stackc, i;
	struct code_frame *frame;
	DREF DeeObject **stackv;
	DREF DeeObject *result;

	/* Lock in assembly-mode, since the stack may contain instruction pointers. */
	frame = DeeFrame_LockWriteAssembly((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	stackc = Dee_code_frame_getspaddr(frame);
	if unlikely(!stackc) {
		DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
		err_empty_sequence((DeeObject *)self);
		goto err;
	}

	/* Fix the caller-given "index" so it becomes a proper position within the stack. */
	i = (uint16_t)DeeSeqRange_Clamp_n(index, stackc);
	stackv = frame->cf_stack;
	result = stackv[i]; /* Inherit reference */
	memmovedownc(stackv + index,
	             stackv + index + 1,
	             (stackc - 1) - index,
	             sizeof(DREF DeeObject *));

	/* Decrease the stack pointer to reflect the now-smaller stack. */
	--frame->cf_sp;
	DeeFrame_LockEndWrite((DeeObject *)self->fs_frame);
	return result;
err:
	return NULL;
}

PRIVATE struct type_nsi tpconst framestack_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&framestack_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&framestack_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&framestack_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)&framestack_nsi_setitem,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)&framestack_nsi_xchitem,
			/* .nsi_insert       = */ (dfunptr_t)&framestack_nsi_insert,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)&framestack_nsi_pop,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};


PRIVATE struct type_seq framestack_seq = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &framestack_nsi,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&framestack_nsi_getsize,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&framestack_nsi_getitem,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&framestack_nsi_setitem,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_getset tpconst framestack_getsets[] = {
	TYPE_GETTER("__func__", &framestack_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst framestack_members[] = {
	TYPE_MEMBER_FIELD_DOC("__frame__", STRUCT_OBJECT, offsetof(FrameStack, fs_frame), "->?Ert:Frame"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FrameStack_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FrameStack",
	/* .tp_doc      = */ DOC("(frame:?Ert:Frame)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&framestack_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&framestack_init,
				TYPE_FIXED_ALLOCATOR(FrameStack)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framestack_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&framestack_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &framestack_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ framestack_getsets,
	/* .tp_members       = */ framestack_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetStackWrapper(DeeFrameObject *__restrict self) {
	DREF FrameStack *result;
	result = DeeObject_MALLOC(FrameStack);
	if unlikely(!result)
		goto err;
	result->fs_frame = self;
	Dee_Incref(self);
	DeeObject_Init(result, &FrameStack_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}




/************************************************************************/
/* FrameSymbolsByName                                                   */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeFrameObject    *frsbn_frame;     /* [1..1][const] The frame in question */
	DREF DeeFunctionObject *frsbn_func;      /* [1..1][const] The function of the frame (as a cache) */
	uint16_t                frsbn_nargs;     /* [<= frsbn_func->fo_code->co_argc_max][const] The # of arguments to enumerate. */
	uint16_t                frsbn_rid_start; /* [<= frsbn_rid_end][const] First RID/SID to enumerate. */
	uint16_t                frsbn_rid_end;   /* [<= frsbn_func->fo_code->co_refstaticc][const] Last RID/SID to enumerate, plus 1. */
	uint16_t                frsbn_localc;    /* [<= frsbn_func->fo_code->co_localc][const] The # of locals to enumerate. */
	uint16_t                frsbn_stackc;    /* [const] The # of stack slots to enumerate (during enum, stop early if less than this remain). */
} FrameSymbolsByName;

typedef struct {
	uint16_t frsbnii_aid; /* [<= frsbni_seq->frsbn_nargs] Next arg to enumerate, or `>= frsbn_nargs' if all were enumerated. */
	uint16_t frsbnii_rid; /* [>= frsbni_seq->frsbn_rid_start && <= frsbni_seq->frsbn_rid_end] Next ref/static to enumerate, or `>= frsbn_rid_end' if all were enumerated. */
	uint16_t frsbnii_lid; /* [<= frsbni_seq->frsbn_localc] Next local to enumerate, or `>= frsbn_localc' if all were enumerated. */
	uint16_t frsbnii_nsp; /* [<= frsbni_seq->frsbn_stackc] NextStackPointer to enumerate, or `>= frsbn_stackc' if all were enumerated. */
} FrameSymbolsByNameIteratorIndex;

typedef struct {
	OBJECT_HEAD
	DREF FrameSymbolsByName        *frsbni_seq;   /* [1..1][const] Underlying frame-symbols sequence. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t               frsbni_lock;  /* Lock for the below indices */
#endif /* !CONFIG_NO_THREADS */
	FrameSymbolsByNameIteratorIndex frsbni_idx; /* Iterator index */
} FrameSymbolsByNameIterator;

#define FrameSymbolsByNameIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->frsbni_lock)

INTDEF DeeTypeObject FrameSymbolsByNameIterator_Type;
INTDEF DeeTypeObject FrameSymbolsByName_Type;

PRIVATE WUNUSED NONNULL((1)) DREF FrameSymbolsByName *DCALL
framesymbolsbynameiter_nii_getseq(FrameSymbolsByNameIterator *__restrict self) {
	return_reference_(self->frsbni_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framesymbolsbynameiter_nii_getindex(FrameSymbolsByNameIterator *__restrict self) {
	size_t result;
	FrameSymbolsByName *sym = self->frsbni_seq;
	FrameSymbolsByNameIterator_LockAcquire(self);
	result = (size_t)(self->frsbni_idx.frsbnii_aid) +
	         (size_t)(self->frsbni_idx.frsbnii_rid - sym->frsbn_rid_start) +
	         (size_t)(self->frsbni_idx.frsbnii_lid) +
	         (size_t)(self->frsbni_idx.frsbnii_nsp);
	FrameSymbolsByNameIterator_LockRelease(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbynameiter_nii_setindex(FrameSymbolsByNameIterator *__restrict self, size_t new_index) {
	FrameSymbolsByName *sym = self->frsbni_seq;
	FrameSymbolsByNameIterator_LockAcquire(self);
	if (new_index < sym->frsbn_nargs) {
		self->frsbni_idx.frsbnii_aid = (uint16_t)new_index;
		self->frsbni_idx.frsbnii_rid = 0;
		self->frsbni_idx.frsbnii_lid = 0;
		self->frsbni_idx.frsbnii_nsp = 0;
	} else if ((new_index -= sym->frsbn_nargs, new_index < (uint16_t)(sym->frsbn_rid_end - sym->frsbn_rid_start))) {
		self->frsbni_idx.frsbnii_aid = sym->frsbn_nargs;
		self->frsbni_idx.frsbnii_rid = sym->frsbn_rid_start + (uint16_t)new_index;
		self->frsbni_idx.frsbnii_lid = 0;
		self->frsbni_idx.frsbnii_nsp = 0;
	} else if ((new_index -= (uint16_t)(sym->frsbn_rid_end - sym->frsbn_rid_start), new_index < sym->frsbn_localc)) {
		self->frsbni_idx.frsbnii_aid = sym->frsbn_nargs;
		self->frsbni_idx.frsbnii_rid = sym->frsbn_rid_end;
		self->frsbni_idx.frsbnii_lid = (uint16_t)new_index;
		self->frsbni_idx.frsbnii_nsp = 0;
	} else if ((new_index -= sym->frsbn_localc, new_index < sym->frsbn_stackc)) {
		self->frsbni_idx.frsbnii_aid = sym->frsbn_nargs;
		self->frsbni_idx.frsbnii_rid = sym->frsbn_rid_end;
		self->frsbni_idx.frsbnii_lid = sym->frsbn_localc;
		self->frsbni_idx.frsbnii_nsp = (uint16_t)new_index;
	} else {
		self->frsbni_idx.frsbnii_aid = sym->frsbn_nargs;
		self->frsbni_idx.frsbnii_rid = sym->frsbn_rid_end;
		self->frsbni_idx.frsbnii_lid = sym->frsbn_localc;
		self->frsbni_idx.frsbnii_nsp = sym->frsbn_stackc;
	}
	FrameSymbolsByNameIterator_LockRelease(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbynameiter_nii_rewind(FrameSymbolsByNameIterator *__restrict self) {
	FrameSymbolsByNameIterator_LockAcquire(self);
	self->frsbni_idx.frsbnii_aid = 0;
	self->frsbni_idx.frsbnii_rid = self->frsbni_seq->frsbn_rid_start;
	self->frsbni_idx.frsbnii_lid = 0;
	self->frsbni_idx.frsbnii_nsp = 0;
	FrameSymbolsByNameIterator_LockRelease(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbynameiter_compare(FrameSymbolsByNameIterator *lhs,
                               FrameSymbolsByNameIterator *rhs) {
	int result;
	if (lhs->frsbni_seq != rhs->frsbni_seq)
		return lhs->frsbni_seq < rhs->frsbni_seq ? -1 : 1;
	if (lhs == rhs)
		return 0;
	DeeLock_Acquire2(FrameSymbolsByNameIterator_LockAcquire(lhs),
	                 FrameSymbolsByNameIterator_LockTryAcquire(lhs),
	                 FrameSymbolsByNameIterator_LockRelease(lhs),
	                 FrameSymbolsByNameIterator_LockAcquire(rhs),
	                 FrameSymbolsByNameIterator_LockTryAcquire(rhs),
	                 FrameSymbolsByNameIterator_LockRelease(rhs));
	if (lhs->frsbni_idx.frsbnii_aid != rhs->frsbni_idx.frsbnii_aid) {
		result = lhs->frsbni_idx.frsbnii_aid < rhs->frsbni_idx.frsbnii_aid ? -1 : 1;
	} else if (lhs->frsbni_idx.frsbnii_rid != rhs->frsbni_idx.frsbnii_rid) {
		result = lhs->frsbni_idx.frsbnii_rid < rhs->frsbni_idx.frsbnii_rid ? -1 : 1;
	} else if (lhs->frsbni_idx.frsbnii_lid != rhs->frsbni_idx.frsbnii_lid) {
		result = lhs->frsbni_idx.frsbnii_lid < rhs->frsbni_idx.frsbnii_lid ? -1 : 1;
	} else if (lhs->frsbni_idx.frsbnii_nsp != rhs->frsbni_idx.frsbnii_nsp) {
		result = lhs->frsbni_idx.frsbnii_nsp < rhs->frsbni_idx.frsbnii_nsp ? -1 : 1;
	} else {
		result = 0;
	}
	FrameSymbolsByNameIterator_LockRelease(lhs);
	FrameSymbolsByNameIterator_LockRelease(rhs);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbynameiter_eq(FrameSymbolsByNameIterator *self,
                          FrameSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FrameSymbolsByNameIterator_Type))
		goto err;
	return_bool(framesymbolsbynameiter_compare(self, other) == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbynameiter_ne(FrameSymbolsByNameIterator *self,
                          FrameSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FrameSymbolsByNameIterator_Type))
		goto err;
	return_bool(framesymbolsbynameiter_compare(self, other) != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbynameiter_lo(FrameSymbolsByNameIterator *self,
                          FrameSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FrameSymbolsByNameIterator_Type))
		goto err;
	return_bool(framesymbolsbynameiter_compare(self, other) < 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbynameiter_le(FrameSymbolsByNameIterator *self,
                          FrameSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FrameSymbolsByNameIterator_Type))
		goto err;
	return_bool(framesymbolsbynameiter_compare(self, other) <= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbynameiter_gr(FrameSymbolsByNameIterator *self,
                          FrameSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FrameSymbolsByNameIterator_Type))
		goto err;
	return_bool(framesymbolsbynameiter_compare(self, other) > 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbynameiter_ge(FrameSymbolsByNameIterator *self,
                          FrameSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FrameSymbolsByNameIterator_Type))
		goto err;
	return_bool(framesymbolsbynameiter_compare(self, other) >= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFrameObject *DCALL
framesymbolsbynameiter_get_frame(FrameSymbolsByNameIterator *__restrict self) {
	return_reference_(self->frsbni_seq->frsbn_frame);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
framesymbolsbynameiter_get_func(FrameSymbolsByNameIterator *__restrict self) {
	return_reference_(self->frsbni_seq->frsbn_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbynameiter_copy(FrameSymbolsByNameIterator *__restrict self,
                            FrameSymbolsByNameIterator *__restrict other) {
	self->frsbni_seq = other->frsbni_seq;
	Dee_Incref(self->frsbni_seq);
	Dee_atomic_lock_init(&self->frsbni_lock);
	FrameSymbolsByNameIterator_LockAcquire(other);
	self->frsbni_idx.frsbnii_aid = other->frsbni_idx.frsbnii_aid;
	self->frsbni_idx.frsbnii_rid = other->frsbni_idx.frsbnii_rid;
	self->frsbni_idx.frsbnii_lid = other->frsbni_idx.frsbnii_lid;
	self->frsbni_idx.frsbnii_nsp = other->frsbni_idx.frsbnii_nsp;
	FrameSymbolsByNameIterator_LockRelease(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbynameiter_init(FrameSymbolsByNameIterator *__restrict self,
                            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:FrameSymbolsByNameIterator", &self->frsbni_seq))
		goto err;
	if (DeeObject_AssertTypeExact(self->frsbni_seq, &FrameSymbolsByName_Type))
		goto err;
	Dee_Incref(self->frsbni_seq);
	Dee_atomic_lock_init(&self->frsbni_lock);
	self->frsbni_idx.frsbnii_aid = 0;
	self->frsbni_idx.frsbnii_rid = self->frsbni_seq->frsbn_rid_start;
	self->frsbni_idx.frsbnii_lid = 0;
	self->frsbni_idx.frsbnii_nsp = 0;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
framesymbolsbynameiter_fini(FrameSymbolsByNameIterator *__restrict self) {
	Dee_Decref(self->frsbni_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
framesymbolsbynameiter_visit(FrameSymbolsByNameIterator *__restrict self,
                             dvisit_t proc, void *arg) {
	Dee_Visit(self->frsbni_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbynameiter_bool(FrameSymbolsByNameIterator *__restrict self) {
	bool result;
	FrameSymbolsByName *sym = self->frsbni_seq;
	FrameSymbolsByNameIterator_LockAcquire(self);
	result = (self->frsbni_idx.frsbnii_aid < sym->frsbn_nargs) ||
	         (self->frsbni_idx.frsbnii_rid < sym->frsbn_rid_end) ||
	         (self->frsbni_idx.frsbnii_lid < sym->frsbn_localc) ||
	         (self->frsbni_idx.frsbnii_nsp < sym->frsbn_stackc);
	FrameSymbolsByNameIterator_LockRelease(self);
	return result ? 1 : 0;
}


/* === canonical location ID
 * - aid: aid
 * - rid: co_argc_max + rid                                  // Also for sid
 * - lid: co_argc_max + co_refstaticc + lid
 * - sp:  co_argc_max + co_refstaticc + co_localc + sp */
typedef uint32_t canonical_lid_t;

/* Returns the name of "clid", or an integer object containing "clid" when the name is unknown. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
FrameSymbolsByName_GetCLidName(FrameSymbolsByName *self, canonical_lid_t clid) {
	code_addr_t ip_addr;
	uint16_t sp_size;
	canonical_lid_t orig_clid = clid;
	DeeCodeObject *code = self->frsbn_func->fo_code;
	struct code_frame const *frame;
	struct ddi_state dds;
	uint8_t *ddi_status;

	/* Check for arguments */
	if (clid < code->co_argc_max) {
		if (code->co_keywords)
			return_reference_((DeeObject *)code->co_keywords[clid]);
		goto fallback;
	}
	clid -= code->co_argc_max;

	/* Check for references/statics */
	if (clid < code->co_refstaticc) {
		char const *name;
		name = DeeCode_GetRSymbolName((DeeObject *)code, (uint16_t)clid);
		if (name)
			return DeeString_New(name);
		goto fallback;
	}
	clid -= code->co_refstaticc;

	/* Check for local/stack variables. */
	frame = DeeFrame_LockRead((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	ip_addr = Dee_code_frame_getipaddr(frame);
	sp_size = Dee_code_frame_getspaddr(frame);
	DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);

	ddi_status = DeeCode_FindDDI((DeeObject *)code, &dds, NULL,
	                             ip_addr, DDI_STATE_FNORMAL);
	if (ddi_status == DDI_NEXT_ERR)
		goto err;
	if (ddi_status != DDI_NEXT_DONE) {
		if (clid < code->co_localc) {
			struct ddi_xregs *iter;
			DDI_STATE_DO(iter, &dds) {
				if ((uint16_t)clid < iter->dx_lcnamc) {
					char *local_name;
					if ((local_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_lcnamv[clid])) != NULL) {
						DREF DeeObject *result;
						result = DeeString_New(local_name);
						Dee_ddi_state_fini(&dds);
						return result;
					}
				}
			}
			DDI_STATE_WHILE(iter, &dds);
			Dee_ddi_state_fini(&dds);
			goto fallback;
		}
		clid -= code->co_localc;

		/* Check for stack variables */
		if (clid < sp_size) {
			struct ddi_xregs *iter;
			DDI_STATE_DO(iter, &dds) {
				uint16_t sp_count = iter->dx_base.dr_usp;
				if (sp_count > iter->dx_spnama)
					sp_count = iter->dx_spnama;
				if ((uint16_t)clid < sp_count) {
					char *stack_name;
					if ((stack_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_spnamv[clid])) != NULL) {
						DREF DeeObject *result;
						result = DeeString_New(stack_name);
						Dee_ddi_state_fini(&dds);
						return result;
					}
				}
			}
			DDI_STATE_WHILE(iter, &dds);
			Dee_ddi_state_fini(&dds);
			/*goto fallback;*/
		}
	}

fallback:
	return DeeInt_NEWU(orig_clid);
err:
	return NULL;
}

struct canonical_lid_location {
	DREF DeeObject **cll_ptr;      /* [0..1][1..1] Pointer to object location. */
	bool             cll_writable; /* Is `*cll_ptr' writable? (it may not be in case it's a function argument) */
	bool             cll_isstatic; /* Is this a static variable (if true: need a lock to the function's statics to access) */
};

/* Return the address of "clid" in "self"
 * @return: true:  Success; `result' was filled in with info about the location.
 * @return: false: No such location (only happens for too-great stack locations) */
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
code_frame_get_clid_addr(struct code_frame const *self,
                         struct canonical_lid_location *__restrict result,
                         canonical_lid_t clid) {
	DeeCodeObject *code = self->cf_func->fo_code;
	result->cll_isstatic = false;

	/* Check for arguments */
	if (clid < code->co_argc_max) {
		uint16_t aid = (uint16_t)clid;
		if (aid < self->cf_argc) {
			result->cll_ptr = (DREF DeeObject **)&self->cf_argv[aid];
		} else if (self->cf_kw) {
			result->cll_ptr = &self->cf_kw->fk_kargv[aid - self->cf_argc];
			if (!*result->cll_ptr)
				goto set_aid_default_pointer;
		} else {
set_aid_default_pointer:
			result->cll_ptr = (DREF DeeObject **)&code->co_defaultv[aid - code->co_argc_min];
		}
		result->cll_writable = false;
		return true;
	}
	clid -= code->co_argc_max;

	/* Check for references/statics */
	if (clid < code->co_refstaticc) {
		result->cll_ptr = &self->cf_func->fo_refv[clid];
		result->cll_writable = clid >= code->co_refc;
		result->cll_isstatic = result->cll_writable;
		return true;
	}
	clid -= code->co_refstaticc;

	/* Check for local variables */
	if (clid < code->co_localc) {
		result->cll_ptr = &self->cf_frame[clid];
		result->cll_writable = true;
		return true;
	}
	clid -= code->co_localc;

	/* Check for stack variables */
	if (clid < Dee_code_frame_getspaddr(self)) {
		result->cll_ptr = &self->cf_stack[clid];
		result->cll_writable = true;
		return true;
	}
	return false;
}

/* Returns the value attached to "clid"
 * @return: * :        The value attached to "clid"
 * @return: ITER_DONE: The given "clid" isn't bound
 * @return: NULL:      Error */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
FrameSymbolsByName_GetCLidValue(FrameSymbolsByName *self, canonical_lid_t clid) {
	DREF DeeObject *result = ITER_DONE;
	struct code_frame const *frame;
	struct canonical_lid_location loc;
	frame = DeeFrame_LockRead((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	if likely(code_frame_get_clid_addr(frame, &loc, clid)) {
		if (loc.cll_isstatic) {
			DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
			DeeFunction_RefLockRead(self->frsbn_func);
			result = *loc.cll_ptr;
			if (ITER_ISOK(result)) {
				Dee_Incref(result);
			} else {
				result = ITER_DONE; /* Not bound */
			}
			DeeFunction_RefLockEndRead(self->frsbn_func);
			goto done;
		} else if (*loc.cll_ptr) {
			result = *loc.cll_ptr;
			Dee_Incref(result);
		}
	}
	DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
done:
	return result;
err:
	return NULL;
}

/* Assign a new "value" attached to "clid"
 * @param: clid_name: The name of `clid' (only for error messages)
 * @param: value:     The new value, or NULL to unbound the location.
 * @return: * :        The old value attached to "clid"
 * @return: ITER_DONE: The old value attached to "clid" used to be unbound
 * @return: NULL:      Error */
PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
FrameSymbolsByName_XchCLidValue(FrameSymbolsByName *self,
                                canonical_lid_t clid,
                                DeeObject *clid_name,
                                DeeObject *value) {
	DREF DeeObject *result = ITER_DONE;
	struct code_frame const *frame;
	struct canonical_lid_location loc;
	frame = DeeFrame_LockWrite((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	if unlikely(!code_frame_get_clid_addr(frame, &loc, clid)) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		err_unknown_key((DeeObject *)self, clid_name);
		goto err;
	}
	if unlikely(!loc.cll_writable) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		goto err_ro;
	}
	if (loc.cll_isstatic) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		Dee_XIncref(value);
		DeeFunction_RefLockWrite(self->frsbn_func);
		result = *loc.cll_ptr;
		*loc.cll_ptr = value;
		DeeFunction_RefLockEndWrite(self->frsbn_func);
		DeeFutex_WakeAll(loc.cll_ptr);
	} else {
		Dee_XIncref(value);
		result = *loc.cll_ptr;
		*loc.cll_ptr = value;
	}
	if (result == NULL)
		result = ITER_DONE;
	DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
	return result;
err_ro:
	err_readonly_key((DeeObject *)self, clid_name);
err:
	return NULL;
}

/* Same as `FrameSymbolsByName_XchCLidValue()', but automatically
 * inherits the reference to the old value.
 * @param: clid_name: The name of `clid' (only for error messages)
 * @param: value:     The new value, or NULL to unbound the location.
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
FrameSymbolsByName_SetCLidValue(FrameSymbolsByName *self,
                                canonical_lid_t clid,
                                DeeObject *clid_name,
                                DeeObject *value) {
	DREF DeeObject *oldval;
	oldval = FrameSymbolsByName_XchCLidValue(self, clid, clid_name, value);
	if (ITER_ISOK(oldval)) {
		Dee_Decref_unlikely(oldval);
		return 0;
	}
	if likely(oldval == ITER_DONE)
		return 0;
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
framesymbolsbynameiter_get_idx(FrameSymbolsByNameIterator *__restrict self,
                               FrameSymbolsByNameIteratorIndex *__restrict result) {
	FrameSymbolsByNameIterator_LockAcquire(self);
	memcpy(result, &self->frsbni_idx, sizeof(FrameSymbolsByNameIteratorIndex));
	FrameSymbolsByNameIterator_LockRelease(self);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
framesymbolsbynameiter_cmpxch_idx(FrameSymbolsByNameIterator *__restrict self,
                                  FrameSymbolsByNameIteratorIndex const *__restrict oldval,
                                  FrameSymbolsByNameIteratorIndex const *__restrict newval) {
	FrameSymbolsByNameIterator_LockAcquire(self);
	if (bcmp(&self->frsbni_idx, oldval, sizeof(FrameSymbolsByNameIteratorIndex)) != 0) {
		FrameSymbolsByNameIterator_LockRelease(self);
		return false;
	}
	memcpy(&self->frsbni_idx, newval, sizeof(FrameSymbolsByNameIteratorIndex));
	FrameSymbolsByNameIterator_LockRelease(self);
	return true;
}


/* Convert "idx" into a canonical LID.
 * Returns `(canonical_lid_t)-1' when `idx' indicates `ITER_DONE' */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) canonical_lid_t DCALL
FrameSymbolsByName_Idx2CLid(FrameSymbolsByName const *__restrict self,
                            FrameSymbolsByNameIteratorIndex const *__restrict idx) {
	canonical_lid_t result;
	DeeCodeObject *code;
	if (idx->frsbnii_aid < self->frsbn_nargs)
		return idx->frsbnii_aid;
	code   = self->frsbn_func->fo_code;
	result = code->co_argc_max;
	if (idx->frsbnii_rid < self->frsbn_rid_end)
		return result + idx->frsbnii_rid;
	result += code->co_refstaticc;
	if (idx->frsbnii_lid < self->frsbn_localc)
		return result + idx->frsbnii_lid;
	result += code->co_localc;
	if (idx->frsbnii_nsp < self->frsbn_stackc)
		return result + idx->frsbnii_nsp;
	return (canonical_lid_t)-1;
}

/* Try to increment "idx" by 1
 * @return: true:  Success
 * @return: false: End-of-enumeration reached */
PRIVATE NONNULL((1, 2)) bool DCALL
FrameSymbolsByName_IdxInc(FrameSymbolsByName *__restrict self,
                          FrameSymbolsByNameIteratorIndex *__restrict idx) {
	canonical_lid_t result;
	DeeCodeObject *code;
	if (idx->frsbnii_aid < self->frsbn_nargs) {
		++idx->frsbnii_aid;
		return true;
	}
	code   = self->frsbn_func->fo_code;
	result = code->co_argc_max;
	if (idx->frsbnii_rid < self->frsbn_rid_end) {
		++idx->frsbnii_rid;
		return true;
	}
	result += code->co_refstaticc;
	if (idx->frsbnii_lid < self->frsbn_localc) {
		++idx->frsbnii_lid;
		return true;
	}
	result += code->co_localc;
	if (idx->frsbnii_nsp < self->frsbn_stackc) {
		++idx->frsbnii_nsp;
		return true;
	}
	return false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbynameiter_next(FrameSymbolsByNameIterator *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *name, *value;
	FrameSymbolsByNameIteratorIndex oldidx, newidx;
again:
	framesymbolsbynameiter_get_idx(self, &oldidx);
	memcpy(&newidx, &oldidx, sizeof(newidx));
	for (;;) {
		canonical_lid_t clid;
		clid  = FrameSymbolsByName_Idx2CLid(self->frsbni_seq, &newidx);
		value = FrameSymbolsByName_GetCLidValue(self->frsbni_seq, clid);
		if (ITER_ISOK(value)) {
			name = FrameSymbolsByName_GetCLidName(self->frsbni_seq, clid);
			if unlikely(!name)
				goto err_value;
			result = DeeTuple_NewUninitialized(2);
			if unlikely(!result)
				goto err_value_name;
			DeeTuple_SET(result, 0, name);  /* Inherit reference */
			DeeTuple_SET(result, 1, value); /* Inherit reference */
			FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx);
			break;
		}
		if unlikely(!value)
			goto err;
		if (!FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx))
			return ITER_DONE;
	}
	if (!framesymbolsbynameiter_cmpxch_idx(self, &oldidx, &newidx)) {
		Dee_Decref(result);
		goto again;
	}
	return (DREF DeeObject *)result;
err_value_name:
	Dee_Decref(name);
err_value:
	Dee_Decref(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbynameiter_next_key(FrameSymbolsByNameIterator *__restrict self) {
	canonical_lid_t clid;
	DREF DeeObject *name;
	FrameSymbolsByNameIteratorIndex oldidx, newidx;
again:
	framesymbolsbynameiter_get_idx(self, &oldidx);
	memcpy(&newidx, &oldidx, sizeof(newidx));
	clid = FrameSymbolsByName_Idx2CLid(self->frsbni_seq, &newidx);
	if unlikely(clid == (canonical_lid_t)-1)
		return ITER_DONE;
	name = FrameSymbolsByName_GetCLidName(self->frsbni_seq, clid);
	if unlikely(!name)
		goto err;
	FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx);
	if (!framesymbolsbynameiter_cmpxch_idx(self, &oldidx, &newidx)) {
		Dee_Decref(name);
		goto again;
	}
	return name;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbynameiter_next_value(FrameSymbolsByNameIterator *__restrict self) {
	DREF DeeObject *value;
	FrameSymbolsByNameIteratorIndex oldidx, newidx;
again:
	framesymbolsbynameiter_get_idx(self, &oldidx);
	memcpy(&newidx, &oldidx, sizeof(newidx));
	for (;;) {
		canonical_lid_t clid;
		clid  = FrameSymbolsByName_Idx2CLid(self->frsbni_seq, &newidx);
		value = FrameSymbolsByName_GetCLidValue(self->frsbni_seq, clid);
		if (ITER_ISOK(value)) {
			FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx);
			break;
		}
		if unlikely(!value)
			goto err;
		if (!FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx))
			return ITER_DONE;
	}
	if (!framesymbolsbynameiter_cmpxch_idx(self, &oldidx, &newidx)) {
		Dee_Decref(value);
		goto again;
	}
	return value;
err:
	return NULL;
}

PRIVATE struct type_nii tpconst framesymbolsbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&framesymbolsbynameiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&framesymbolsbynameiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&framesymbolsbynameiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&framesymbolsbynameiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp framesymbolsbynameiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_ge,
	/* .tp_nii  = */ &framesymbolsbynameiter_nii,
};

PRIVATE struct type_getset tpconst framesymbolsbynameiter_getsets[] = {
	TYPE_GETTER("__frame__", &framesymbolsbynameiter_get_frame, "->?Ert:Frame"),
	TYPE_GETTER("__func__", &framesymbolsbynameiter_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst framesymbolsbynameiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(FrameSymbolsByNameIterator, frsbni_seq), "->?Ert:FrameSymbolsByName"),
	TYPE_MEMBER_FIELD("__aid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByNameIterator, frsbni_idx.frsbnii_aid)),
	TYPE_MEMBER_FIELD("__rid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByNameIterator, frsbni_idx.frsbnii_rid)),
	TYPE_MEMBER_FIELD("__lid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByNameIterator, frsbni_idx.frsbnii_lid)),
	TYPE_MEMBER_FIELD("__nsp__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByNameIterator, frsbni_idx.frsbnii_nsp)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FrameSymbolsByNameIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FrameSymbolsByNameIterator",
	/* .tp_doc      = */ DOC("next->?T2?X2?Dstring?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&framesymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&framesymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FrameSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&framesymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &framesymbolsbynameiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ framesymbolsbynameiter_getsets,
	/* .tp_members       = */ framesymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



/* Try to convert "name" into its "canonical location id" within "self"
 * @return: * : The canonical location ID of `name'
 * @return: (canonical_lid_t)-1: An error was thrown.
 * @return: (canonical_lid_t)-2: No symbol exists matching `name', or insufficient debug info */
PRIVATE WUNUSED NONNULL((1, 2)) canonical_lid_t DCALL
FrameSymbolsByName_TryName2LocId(FrameSymbolsByName *self, char const *name) {
	DeeCodeObject *code = self->frsbn_func->fo_code;
	uint16_t i;
	canonical_lid_t result;
	struct ddi_state dds;
	struct code_frame const *frame;
	uint8_t *ddi_status;

	/* Check for argument names. */
	if (code->co_keywords) {
		for (i = 0; i < self->frsbn_nargs; ++i) {
			DeeStringObject *arg_name = code->co_keywords[i];
			ASSERT_OBJECT_TYPE_EXACT(arg_name, &DeeString_Type);
			if (strcmp(DeeString_STR(arg_name), name) == 0)
				return i;
		}
	}
	result = code->co_argc_max;

	/* Check for names of references/statics. */
	i = DDI_GetRefIdByName(code->co_ddi, name);
	if (i >= self->frsbn_rid_start && i < self->frsbn_rid_end)
		return result + i;
	result += code->co_refstaticc;

	/* Check for of locals/stack elements. */
again_lock_frame:
	frame = DeeFrame_LockRead((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	ddi_status = DeeCode_FindDDI((DeeObject *)code, &dds, NULL,
	                             Dee_code_frame_getipaddr(frame),
	                             DDI_STATE_FNOEXCEPT);
	if (ddi_status == DDI_NEXT_ERR) {
		DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
		if (Dee_CollectMemory(1))
			goto again_lock_frame;
		goto err;
	} else if (ddi_status != DDI_NEXT_DONE) {
		struct ddi_xregs *iter;
		DDI_STATE_DO(iter, &dds) {
			uint16_t lc_count = iter->dx_lcnamc;
			uint16_t sp_count = iter->dx_base.dr_usp;
			if (lc_count > self->frsbn_localc)
				lc_count = self->frsbn_localc;
			if (sp_count > iter->dx_spnama)
				sp_count = iter->dx_spnama;
			if (sp_count > self->frsbn_stackc)
				sp_count = self->frsbn_stackc;
			for (i = 0; i < lc_count; ++i) {
				char const *local_name;
				local_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_lcnamv[i]);
				if (local_name && strcmp(local_name, name) == 0) {
					DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
					Dee_ddi_state_fini(&dds);
					return result + i;
				}
			}
			for (i = 0; i < sp_count; ++i) {
				char const *stack_name;
				stack_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_spnamv[i]);
				if (stack_name && strcmp(stack_name, name) == 0) {
					DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
					Dee_ddi_state_fini(&dds);
					return result + code->co_localc + i;
				}
			}
		}
		DDI_STATE_WHILE(iter, &dds);
		DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
		Dee_ddi_state_fini(&dds);
	} else {
		DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
	}
	return (canonical_lid_t)-2;
err:
	return (canonical_lid_t)-1;
}

/* Same as `FrameSymbolsByName_TryName2LocId()', but handles a generic object `key'
 * @return: * : The canonical location ID of `key'
 * @return: (canonical_lid_t)-1: An error was thrown.
 * @return: (canonical_lid_t)-2: No symbol exists matching `key', or insufficient debug info */
PRIVATE WUNUSED NONNULL((1, 2)) canonical_lid_t DCALL
FrameSymbolsByName_TryKey2LocId(FrameSymbolsByName *self, DeeObject *key) {
	canonical_lid_t result;
	if (DeeString_Check(key)) {
		result = FrameSymbolsByName_TryName2LocId(self, DeeString_STR(key));
	} else {
		canonical_lid_t size;
		DeeCodeObject *code;
		if (DeeObject_AsUIntX(key, &result))
			goto err;
		/* Check that "result" can appear in "self" */
		code = self->frsbn_func->fo_code;
		size = code->co_argc_max;
		if (result < size) {
			uint16_t aid = (uint16_t)result;
			if (aid >= self->frsbn_nargs)
				goto err_bad_lid;
		} else if ((size += code->co_refstaticc, result < size)) {
			uint16_t rid = (uint16_t)(result - code->co_argc_max);
			if (rid < self->frsbn_rid_start)
				goto err_bad_lid;
			if (rid >= self->frsbn_rid_end)
				goto err_bad_lid;
		} else if ((size += code->co_localc, result < size)) {
			uint16_t lid = (uint16_t)(result - (code->co_argc_max + code->co_refstaticc));
			if (lid >= self->frsbn_localc)
				goto err_bad_lid;
		} else if (result >= (size + self->frsbn_stackc)) {
			goto err_bad_lid;
		}
	}
	return result;
err_bad_lid:
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid location id %#" PRFx32,
	                result);
err:
	return (canonical_lid_t)-1;
}

/* Same as `FrameSymbolsByName_TryKey2LocId()', but throw an error for missing-keys
 * @return: * : The canonical location ID of `key'
 * @return: (canonical_lid_t)-1: An error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) canonical_lid_t DCALL
FrameSymbolsByName_Key2LocId(FrameSymbolsByName *self, DeeObject *key) {
	canonical_lid_t result = FrameSymbolsByName_TryKey2LocId(self, key);
	if unlikely(result == (canonical_lid_t)-2)
		goto err_no_such_key;
	return result;
err_no_such_key:
	return (canonical_lid_t)err_unknown_key((DeeObject *)self, key);
}



PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framesymbolsbyname_nsi_getsize(FrameSymbolsByName *__restrict self) {
	size_t result;
	result = self->frsbn_nargs;
	result += self->frsbn_rid_end - self->frsbn_rid_start;
	result += self->frsbn_localc;
	result += self->frsbn_stackc;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
framesymbolsbyname_nsi_getdefault(FrameSymbolsByName *self,
                                  DeeObject *key, DeeObject *defl) {
	canonical_lid_t clid = FrameSymbolsByName_TryKey2LocId(self, key);
	if (clid != (canonical_lid_t)-2) {
		DREF DeeObject *result;
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		result = FrameSymbolsByName_GetCLidValue(self, clid);
		if (result != ITER_DONE)
			return result;
	}
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
framesymbolsbyname_nsi_setdefault(FrameSymbolsByName *self,
                                  DeeObject *key, DeeObject *defl) {
	DREF DeeObject *result;
	struct code_frame *frame;
	struct canonical_lid_location loc;
	canonical_lid_t clid = FrameSymbolsByName_Key2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	frame = DeeFrame_LockWrite((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	if unlikely(!code_frame_get_clid_addr(frame, &loc, clid)) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		goto err_no_such_key;
	}
	if (loc.cll_isstatic) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		DeeFunction_RefLockWrite(self->frsbn_func);
		result = *loc.cll_ptr;
		if (ITER_ISOK(result)) {
			Dee_Incref(result);
			DeeFunction_RefLockEndWrite(self->frsbn_func);
		} else {
			Dee_Incref_n(defl, 2);
			*loc.cll_ptr = result = defl;
			DeeFunction_RefLockEndWrite(self->frsbn_func);
			DeeFutex_WakeAll(loc.cll_ptr);
		}
		goto done;
	} else if (*loc.cll_ptr) {
		result = *loc.cll_ptr;
		Dee_Incref(result);
	} else {
		if unlikely(!loc.cll_writable) {
			DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
			err_readonly_key((DeeObject *)self, key);
			goto err;
		}
		Dee_Incref_n(defl, 2);
		*loc.cll_ptr = result = defl;
	}
	DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
done:
	return result;
err_no_such_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
framesymbolsbyname_nsi_updateold(FrameSymbolsByName *self,
                                 DeeObject *key, DeeObject *value,
                                 DREF DeeObject **p_oldvalue) {
	DREF DeeObject *oldvalue;
	struct code_frame *frame;
	struct canonical_lid_location loc;
	canonical_lid_t clid = FrameSymbolsByName_TryKey2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-2)
		goto err_no_such_key;
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	frame = DeeFrame_LockWrite((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	if unlikely(!code_frame_get_clid_addr(frame, &loc, clid)) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		goto err_no_such_key;
	}
	if (loc.cll_isstatic) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		DeeFunction_RefLockWrite(self->frsbn_func);
		oldvalue = *loc.cll_ptr;
		if (ITER_ISOK(oldvalue)) {
			Dee_Incref(value);
			*loc.cll_ptr = value;
			DeeFunction_RefLockEndWrite(self->frsbn_func);
			DeeFutex_WakeAll(loc.cll_ptr);
			if (p_oldvalue) {
				*p_oldvalue = oldvalue;
			} else {
				Dee_Decref(oldvalue);
			}
			return 1;
		}
		DeeFunction_RefLockEndWrite(self->frsbn_func);
		return 0;
	} else if ((oldvalue = *loc.cll_ptr) != NULL) {
		if unlikely(!loc.cll_writable) {
			DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
			err_readonly_key((DeeObject *)self, key);
			goto err;
		}
		Dee_Incref(value);
		*loc.cll_ptr = value;
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		if (p_oldvalue) {
			*p_oldvalue = oldvalue;
		} else {
			Dee_Decref(oldvalue);
		}
		return 1;
	} else {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return 0;
	}
	__builtin_unreachable();
err_no_such_key:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
framesymbolsbyname_nsi_insertnew(FrameSymbolsByName *self,
                                 DeeObject *key, DeeObject *value,
                                 DREF DeeObject **p_oldvalue) {
	DREF DeeObject *oldvalue;
	struct code_frame *frame;
	struct canonical_lid_location loc;
	canonical_lid_t clid = FrameSymbolsByName_Key2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	frame = DeeFrame_LockWrite((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	if unlikely(!code_frame_get_clid_addr(frame, &loc, clid)) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		goto err_no_such_key;
	}
	if (loc.cll_isstatic) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		DeeFunction_RefLockWrite(self->frsbn_func);
		oldvalue = *loc.cll_ptr;
		if (ITER_ISOK(oldvalue)) {
			if (p_oldvalue) {
				*p_oldvalue = oldvalue;
				Dee_Incref(oldvalue);
			}
			DeeFunction_RefLockEndWrite(self->frsbn_func);
			return 1;
		}
		Dee_Incref(value);
		*loc.cll_ptr = value;
		DeeFunction_RefLockEndWrite(self->frsbn_func);
		DeeFutex_WakeAll(loc.cll_ptr);
		return 0;
	} else if (*loc.cll_ptr) {
		if (p_oldvalue) {
			oldvalue = *loc.cll_ptr;
			*p_oldvalue = oldvalue;
			Dee_Incref(oldvalue);
		}
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return 1;
	} else {
		if unlikely(!loc.cll_writable) {
			DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
			err_readonly_key((DeeObject *)self, key);
			goto err;
		}
		Dee_Incref(value);
		*loc.cll_ptr = value;
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return 0;
	}
	__builtin_unreachable();
err_no_such_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return -1;
}

#define framesymbolsbyname_nsi_nextkey   framesymbolsbynameiter_next_key
#define framesymbolsbyname_nsi_nextvalue framesymbolsbynameiter_next_value

PRIVATE WUNUSED NONNULL((1)) DREF FrameSymbolsByNameIterator *DCALL
framesymbolsbyname_iter(FrameSymbolsByName *__restrict self) {
	DREF FrameSymbolsByNameIterator *result;
	result = DeeObject_MALLOC(FrameSymbolsByNameIterator);
	if unlikely(!result)
		goto err;
	result->frsbni_seq = self;
	Dee_Incref(self);
	Dee_atomic_lock_init(&result->frsbni_lock);
	result->frsbni_idx.frsbnii_aid = 0;
	result->frsbni_idx.frsbnii_rid = self->frsbn_rid_start;
	result->frsbni_idx.frsbnii_lid = 0;
	result->frsbni_idx.frsbnii_nsp = 0;
	DeeObject_Init(result, &FrameSymbolsByNameIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbyname_contains(FrameSymbolsByName *self,
                            DeeObject *key) {
	canonical_lid_t clid;
	clid = FrameSymbolsByName_TryKey2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return_bool(clid != (canonical_lid_t)-2);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbyname_getitem(FrameSymbolsByName *self,
                           DeeObject *key) {
	DREF DeeObject *result;
	canonical_lid_t clid = FrameSymbolsByName_Key2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	result = FrameSymbolsByName_GetCLidValue(self, clid);
	if (result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unbound_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2 /*, 3*/)) int DCALL
framesymbolsbyname_setitem(FrameSymbolsByName *self,
                          DeeObject *key, DeeObject *value) {
	canonical_lid_t clid = FrameSymbolsByName_Key2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return FrameSymbolsByName_SetCLidValue(self, clid, key, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_delitem(FrameSymbolsByName *self,
                           DeeObject *key) {
	return framesymbolsbyname_setitem(self, key, NULL);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_copy(FrameSymbolsByName *__restrict self,
                        FrameSymbolsByName *__restrict other) {
	self->frsbn_frame = other->frsbn_frame;
	self->frsbn_func  = other->frsbn_func;
	Dee_Incref(self->frsbn_frame);
	Dee_Incref(self->frsbn_func);
	self->frsbn_nargs     = other->frsbn_nargs;
	self->frsbn_rid_start = other->frsbn_rid_start;
	self->frsbn_rid_end   = other->frsbn_rid_end;
	self->frsbn_localc    = other->frsbn_localc;
	self->frsbn_stackc    = other->frsbn_stackc;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbyname_init_kw(FrameSymbolsByName *__restrict self,
                           size_t argc, DeeObject *const *argv,
                           DeeObject *kw) {
	struct code_frame const *frame;
	DeeCodeObject *code;
	PRIVATE DEFINE_KWLIST(kwlist, { K(frame), K(argc), K(ridstart), K(ridend), K(localc), K(stackc), KEND });
	self->frsbn_nargs     = (uint16_t)-1;
	self->frsbn_rid_start = 0;
	self->frsbn_rid_end   = (uint16_t)-1;
	self->frsbn_localc    = (uint16_t)-1;
	self->frsbn_stackc    = (uint16_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|" UNPu16 UNPu16 UNPu16 UNPu16 UNPu16,
	                    &self->frsbn_frame, &self->frsbn_nargs,
	                    &self->frsbn_rid_start, &self->frsbn_rid_end,
	                    &self->frsbn_localc, &self->frsbn_stackc))
		goto err;
	if (DeeObject_AssertTypeExact(self->frsbn_frame, &DeeFrame_Type))
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	self->frsbn_func = frame->cf_func;
	Dee_Incref(self->frsbn_func);
	if (self->frsbn_stackc > Dee_code_frame_getspaddr(frame))
		self->frsbn_stackc = Dee_code_frame_getspaddr(frame);
	DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
	code = self->frsbn_func->fo_code;
	if (self->frsbn_localc > code->co_localc)
		self->frsbn_localc = code->co_localc;
	if (self->frsbn_nargs > code->co_argc_max)
		self->frsbn_nargs = code->co_argc_max;
	if (self->frsbn_rid_end > code->co_refstaticc)
		self->frsbn_rid_end = code->co_refstaticc;
	if (self->frsbn_rid_start > self->frsbn_rid_end)
		self->frsbn_rid_start = self->frsbn_rid_end;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
framesymbolsbyname_fini(FrameSymbolsByName *__restrict self) {
	Dee_Decref(self->frsbn_frame);
	Dee_Decref(self->frsbn_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
framesymbolsbyname_visit(FrameSymbolsByName *__restrict self,
                         dvisit_t proc, void *arg) {
	Dee_Visit(self->frsbn_frame);
	Dee_Visit(self->frsbn_func);
}



PRIVATE struct type_nsi tpconst framesymbolsbyname_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&framesymbolsbyname_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&framesymbolsbyname_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&framesymbolsbyname_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&framesymbolsbyname_nsi_getdefault,
			/* .nsi_setdefault = */ (dfunptr_t)&framesymbolsbyname_nsi_setdefault,
			/* .nsi_updateold  = */ (dfunptr_t)&framesymbolsbyname_nsi_updateold,
			/* .nsi_insertnew  = */ (dfunptr_t)&framesymbolsbyname_nsi_insertnew,
		}
	}
};

PRIVATE struct type_seq framesymbolsbyname_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&framesymbolsbyname_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&framesymbolsbyname_setitem,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &framesymbolsbyname_nsi,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&framesymbolsbyname_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&framesymbolsbyname_nsi_getsize,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
};

PRIVATE struct type_member tpconst framesymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FrameSymbolsByNameIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst framesymbolsbyname_members[] = {
	TYPE_MEMBER_FIELD_DOC("__frame__", STRUCT_OBJECT, offsetof(FrameSymbolsByName, frsbn_frame), "->?Ert:Frame"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FrameSymbolsByName, frsbn_func), "->?DFunction"),
	TYPE_MEMBER_FIELD("__nargs__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByName, frsbn_nargs)),
	TYPE_MEMBER_FIELD("__ridstart__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByName, frsbn_rid_start)),
	TYPE_MEMBER_FIELD("__ridend__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByName, frsbn_rid_end)),
	TYPE_MEMBER_FIELD("__localc__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByName, frsbn_localc)),
	TYPE_MEMBER_FIELD("__stackc__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FrameSymbolsByName, frsbn_stackc)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject FrameSymbolsByName_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FrameSymbolsByName",
	/* .tp_doc      = */ DOC("A ${{(int | string): Object}}-like mapping for currently relevant "
	                         /**/ "named symbols within a given code-frame.\n"
	                         "\n"
	                         "(frame:?Ert:Frame,argc?:?Dint,ridstart=!0,ridend?:?Dint,localc?:?Dint,stackc?:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&framesymbolsbyname_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(FrameSymbolsByName),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&framesymbolsbyname_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framesymbolsbyname_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&framesymbolsbyname_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &framesymbolsbyname_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ framesymbolsbyname_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ framesymbolsbyname_class_members
};




INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetArgsByNameWrapper(DeeFrameObject *__restrict self) {
	DREF FrameSymbolsByName *result;
	result = (DREF FrameSymbolsByName *)DeeFrame_GetSymbolsByNameWrapper(self);
	if likely(result) {
		result->frsbn_rid_start = 0;
		result->frsbn_rid_end   = 0;
		result->frsbn_localc    = 0;
		result->frsbn_stackc    = 0;
	}
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetLocalsByNameWrapper(DeeFrameObject *__restrict self) {
	DREF FrameSymbolsByName *result;
	result = (DREF FrameSymbolsByName *)DeeFrame_GetSymbolsByNameWrapper(self);
	if likely(result) {
		result->frsbn_nargs     = 0;
		result->frsbn_rid_start = 0;
		result->frsbn_rid_end   = 0;
		result->frsbn_stackc    = 0;
	}
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetStackByNameWrapper(DeeFrameObject *__restrict self) {
	DREF FrameSymbolsByName *result;
	result = (DREF FrameSymbolsByName *)DeeFrame_GetSymbolsByNameWrapper(self);
	if likely(result) {
		result->frsbn_nargs     = 0;
		result->frsbn_rid_start = 0;
		result->frsbn_rid_end   = 0;
		result->frsbn_localc    = 0;
	}
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetVariablesByNameWrapper(DeeFrameObject *__restrict self) {
	DREF FrameSymbolsByName *result;
	result = (DREF FrameSymbolsByName *)DeeFrame_GetSymbolsByNameWrapper(self);
	if likely(result) {
		result->frsbn_nargs     = 0;
		result->frsbn_rid_start = 0;
		result->frsbn_rid_end   = 0;
	}
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFrame_GetSymbolsByNameWrapper(DeeFrameObject *__restrict self) {
	DeeCodeObject *code;
	DREF FrameSymbolsByName *result;
	struct code_frame const *frame;
	result = DeeObject_MALLOC(FrameSymbolsByName);
	if unlikely(!result)
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)self);
	if unlikely(!frame)
		goto err_r;
	result->frsbn_func = frame->cf_func;
	Dee_Incref(result->frsbn_func);
	result->frsbn_stackc = Dee_code_frame_getspaddr(frame);
	DeeFrame_LockEndRead((DeeObject *)self);
	code = result->frsbn_func->fo_code;
	result->frsbn_nargs     = code->co_argc_max;
	result->frsbn_rid_start = 0;
	result->frsbn_rid_end   = code->co_refstaticc;
	result->frsbn_localc    = code->co_localc;
	result->frsbn_frame     = self;
	Dee_Incref(self);
	DeeObject_Init(result, &FrameSymbolsByName_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


DECL_END
#endif /* CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION */

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C */
