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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C
#define GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/traceback.h>
#include <deemon/util/atomic.h>
#include <deemon/util/futex.h>
#include <deemon/util/lock.h>

#include <hybrid/byteswap.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

/**/
#include "../objects/generic-proxy.h"
#include "../objects/seq/default-map-proxy.h"
#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

/**/
#include "function-wrappers.h"

/**/
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

#ifdef __OPTIMIZE_SIZE__
#define NULL_if_Os(x) NULL
#else /* __OPTIMIZE_SIZE__ */
#define NULL_if_Os(x) x
#endif /* !__OPTIMIZE_SIZE__ */

DECL_BEGIN

#define FIELDS_ADJACENT(T, a, b) \
	(COMPILER_OFFSETAFTER(T, a) /*CEIL_ALIGN:COMPILER_ALIGNOF(((T *)0)->b)*/ == offsetof(T, b))

#ifndef CONFIG_HAVE_strcmpz
#define CONFIG_HAVE_strcmpz
#undef strcmpz
#define strcmpz dee_strcmpz
DeeSystem_DEFINE_strcmpz(dee_strcmpz)
#endif /* !CONFIG_HAVE_strcmpz */

/************************************************************************/
/* FunctionStatics_Type                                                 */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFunctionObject, fs_func); /* [1..1][const] Function in question */
} FunctionStatics;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFunctionObject, fsi_func); /* [1..1][const] Function in question */
	uint16_t                                fsi_sid;   /* [>= fsi_func->fo_code->co_refc][lock(ATOMIC)] Index of next static to enumerate. */
	uint16_t                                fsi_end;   /* [== fsi_func->fo_code->co_refstaticc][const] Static enumeration end */
} FunctionStaticsIterator;

INTDEF DeeTypeObject FunctionStaticsIterator_Type;
INTDEF DeeTypeObject FunctionStatics_Type;


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcstaticsiter_getseq(FunctionStaticsIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcstaticsiter_compare(FunctionStaticsIterator *self,
                        FunctionStaticsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionStaticsIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->fsi_func, other->fsi_func);
	Dee_return_compareT(uint16_t, atomic_read(&self->fsi_end),
	                    /*     */ atomic_read(&other->fsi_end));
err:
	return Dee_COMPARE_ERR;
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
	DeeArg_Unpack1(err, argc, argv, "FunctionStaticsIterator", &self->fsi_func);
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

STATIC_ASSERT(offsetof(FunctionStaticsIterator, fsi_func) == offsetof(ProxyObject, po_obj));
#define funcstaticsiter_fini  generic_proxy__fini
#define funcstaticsiter_visit generic_proxy__visit

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
			/* .nii_getseq   = */ (Dee_funptr_t)&funcstaticsiter_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&funcstaticsiter_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&funcstaticsiter_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&funcstaticsiter_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)NULL,
			/* .nii_advance  = */ (Dee_funptr_t)NULL,
			/* .nii_prev     = */ (Dee_funptr_t)NULL,
			/* .nii_next     = */ (Dee_funptr_t)NULL,
			/* .nii_hasprev  = */ (Dee_funptr_t)NULL,
			/* .nii_peek     = */ (Dee_funptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp funcstaticsiter_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&funcstaticsiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &funcstaticsiter_nii,
};

PRIVATE struct type_getset tpconst funcstaticsiter_getsets[] = {
	TYPE_GETTER_AB(STR_seq, &funcstaticsiter_getseq, "->?Ert:FunctionStatics"),
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
				/* .tp_ctor      = */ (Dee_funptr_t)&funcstaticsiter_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&funcstaticsiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&funcstaticsiter_init,
				TYPE_FIXED_ALLOCATOR(FunctionStaticsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcstaticsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&funcstaticsiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&funcstaticsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &funcstaticsiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcstaticsiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ funcstaticsiter_getsets,
	/* .tp_members       = */ funcstaticsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};




PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcstatics_size(FunctionStatics *__restrict self) {
	DeeCodeObject *code = self->fs_func->fo_code;
	return code->co_refstaticc - code->co_refc;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcstatics_getitem_index(FunctionStatics *__restrict self, size_t index) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *result;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, funcstatics_size(self));
		return NULL;
	}
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[index];
	if unlikely(!ITER_ISOK(result)) {
		DeeFunction_RefLockEndRead(func);
		index -= code->co_refc;
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
		return NULL;
	}
	Dee_Incref(result);
	DeeFunction_RefLockEndRead(func);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstatics_bounditem_index(FunctionStatics *__restrict self, size_t index) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DeeObject *result;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc)
		return Dee_BOUND_MISSING;
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[index];
	if unlikely(!ITER_ISOK(result)) {
		DeeFunction_RefLockEndRead(func);
		return Dee_BOUND_NO;
	}
	DeeFunction_RefLockEndRead(func);
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstatics_delitem_index(FunctionStatics *__restrict self, size_t index) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *oldval;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc)
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, funcstatics_size(self));
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
funcstatics_setitem_index(FunctionStatics *self, size_t index, DeeObject *value) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *oldval;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc)
		return DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, funcstatics_size(self));
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
funcstatics_getitem_index_fast(FunctionStatics *__restrict self, size_t index) {
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
funcstatics_mh_seq_xchitem_index(FunctionStatics *self, size_t index, DeeObject *value) {
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	DREF DeeObject *oldval;
	if (OVERFLOW_UADD(index, code->co_refc, &index) || index >= code->co_refstaticc) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, funcstatics_size(self));
		return NULL;
	}
	Dee_Incref(value);
	DeeFunction_RefLockWrite(func);
	oldval = func->fo_refv[index]; /* Inherit reference */
	if unlikely(!ITER_ISOK(oldval)) {
		DeeFunction_RefLockEndWrite(func);
		index -= code->co_refc;
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcstatics_init(FunctionStatics *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "FunctionStatics", &self->fs_func);
	if (DeeObject_AssertTypeExact(self->fs_func, &DeeFunction_Type))
		goto err;
	Dee_Incref(self->fs_func);
	return 0;
err:
	return -1;
}


STATIC_ASSERT(offsetof(FunctionStatics, fs_func) == offsetof(ProxyObject, po_obj));
#define funcstatics_copy  generic_proxy__copy_alias
#define funcstatics_fini  generic_proxy__fini
#define funcstatics_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
funcstatics_mh_seq_enumerate_index(FunctionStatics *self, Dee_seq_enumerate_index_t proc,
                                   void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	DeeFunctionObject *func = self->fs_func;
	DeeCodeObject *code = func->fo_code;
	size_t i, count = code->co_refstaticc - code->co_refc;
	if (end > count)
		end = count;
	for (i = start; i < end; ++i) {
		DREF DeeObject *value;
		DeeFunction_RefLockRead(func);
		value = func->fo_refv[i];
		Dee_XIncref(value);
		DeeFunction_RefLockEndRead(func);
		temp = (*proc)(arg, i, value);
		Dee_XDecref_unlikely(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_method tpconst funcstatics_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst funcstatics_method_hints[] = {
	TYPE_METHOD_HINT(seq_xchitem_index, &funcstatics_mh_seq_xchitem_index),
	TYPE_METHOD_HINT(seq_enumerate_index, &funcstatics_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq funcstatics_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcstatics_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&funcstatics_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&funcstatics_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&funcstatics_getitem_index,
	/* .tp_getitem_index_fast = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&funcstatics_getitem_index_fast,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&funcstatics_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&funcstatics_setitem_index,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&funcstatics_bounditem_index,
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
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
				/* .tp_ctor      = */ (Dee_funptr_t)&funcstatics_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&funcstatics_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&funcstatics_init,
				TYPE_FIXED_ALLOCATOR(FunctionStatics)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcstatics_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&funcstatics_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__287C06B7236F06BE),
	/* .tp_seq           = */ &funcstatics_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ funcstatics_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcstatics_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ funcstatics_class_members,
	/* .tp_method_hints  = */ funcstatics_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
err:
	return NULL;
}




/************************************************************************/
/* FunctionSymbolsByName_Type                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFunctionObject, fsbn_func);     /* [1..1][const] Function in question */
	uint16_t                                fsbn_rid_start; /* [const] First RID/SID to enumerate. */
	uint16_t                                fsbn_rid_end;   /* [const] Last RID/SID to enumerate, plus 1. */
} FunctionSymbolsByName;

typedef struct {
	PROXY_OBJECT_HEAD_EX(FunctionSymbolsByName, fsbni_seq); /* [1..1][const] Function whose references/statics are being enumerated. */
	DeeFunctionObject                          *fsbni_func; /* [== fsbni_seq->fsbn_func][1..1][const] Function whose references/statics are being enumerated. */
	uint16_t                                    fsbni_rid;  /* [lock(ATOMIC)] Next rid (overflowing into sids) to enumerate. */
	uint16_t                                    fsbni_end;  /* [== fsbni_seq->fsbn_rid_end][const] RIS/SID end index. */
} FunctionSymbolsByNameIterator;

INTDEF DeeTypeObject FunctionSymbolsByNameIterator_Type;
INTDEF DeeTypeObject FunctionSymbolsByNameKeysIterator_Type;
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbynameiter_compare(FunctionSymbolsByNameIterator *self,
                              FunctionSymbolsByNameIterator *other) {
	if (DeeObject_AssertTypeExact(other, &FunctionSymbolsByNameIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->fsbni_func, other->fsbni_func);
	Dee_return_compareT(uint16_t, atomic_read(&self->fsbni_rid),
	                    /*     */ atomic_read(&other->fsbni_rid));
err:
	return Dee_COMPARE_ERR;
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
	DeeArg_Unpack1(err, argc, argv, "FunctionSymbolsByNameIterator", &self->fsbni_seq);
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

STATIC_ASSERT(offsetof(FunctionSymbolsByNameIterator, fsbni_seq) == offsetof(ProxyObject, po_obj));
#define funcsymbolsbynameiter_fini  generic_proxy__fini
#define funcsymbolsbynameiter_visit generic_proxy__visit

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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbynameiter_nextpair(FunctionSymbolsByNameIterator *__restrict self,
                               DREF DeeObject *key_and_value[2]) {
	uint16_t result_sid;
	DeeFunctionObject *func = self->fsbni_func;
	for (;;) {
		uint16_t old_sid, new_sid;
		old_sid = atomic_read(&self->fsbni_rid);
		new_sid = old_sid;
		DeeFunction_RefLockRead(func);
		for (;;) {
			if (new_sid >= self->fsbni_end) {
				DeeFunction_RefLockEndRead(func);
				return 1;
			}
			key_and_value[1] = func->fo_refv[new_sid];
			if (ITER_ISOK(key_and_value[1]))
				break;
			++new_sid;
		}
		Dee_Incref(key_and_value[1]);
		DeeFunction_RefLockEndRead(func);
		result_sid = new_sid;
		++new_sid;
		if (atomic_cmpxch_or_write(&self->fsbni_rid, old_sid, new_sid))
			break;
		Dee_Decref_unlikely(key_and_value[1]);
	}
	key_and_value[0] = Code_GetRefNameById(self->fsbni_func->fo_code, result_sid);
	if unlikely(!key_and_value[0])
		goto err_value;
	return 0;
err_value:
	Dee_Decref_unlikely(key_and_value[1]);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolsbynameiter_nextkey(FunctionSymbolsByNameIterator *__restrict self) {
	uint16_t result_sid;
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
			if (ITER_ISOK(func->fo_refv[new_sid]))
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
funcsymbolsbynameiter_nextkey_with_unbound(FunctionSymbolsByNameIterator *__restrict self) {
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
funcsymbolsbynameiter_nextvalue(FunctionSymbolsByNameIterator *__restrict self) {
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

PRIVATE struct type_iterator funcsymbolsbynameiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&funcsymbolsbynameiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_nextvalue,
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextkey),
};

PRIVATE struct type_nii tpconst funcsymbolsbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&funcsymbolsbynameiter_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&funcsymbolsbynameiter_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&funcsymbolsbynameiter_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&funcsymbolsbynameiter_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)NULL,
			/* .nii_advance  = */ (Dee_funptr_t)NULL,
			/* .nii_prev     = */ (Dee_funptr_t)NULL,
			/* .nii_next     = */ (Dee_funptr_t)NULL,
			/* .nii_hasprev  = */ (Dee_funptr_t)NULL,
			/* .nii_peek     = */ (Dee_funptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp funcsymbolsbynameiter_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbynameiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &funcsymbolsbynameiter_nii,
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
				/* .tp_ctor      = */ (Dee_funptr_t)&funcsymbolsbynameiter_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&funcsymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&funcsymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&funcsymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &funcsymbolsbynameiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &funcsymbolsbynameiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcsymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject FunctionSymbolsByNameKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionSymbolsByNameKeysIterator",
	/* .tp_doc      = */ DOC("next->?X2?Dstring?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&funcsymbolsbynameiter_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&funcsymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&funcsymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&funcsymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &funcsymbolsbynameiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbynameiter_nextkey_with_unbound,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ funcsymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};





/* Search for "name" in `self' and return its ID. If not found,
 * *NO* error is thrown, and `(uint16_t)-1' is returned. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) uint16_t DCALL
DDI_GetRefIdByName(DeeDDIObject const *self, char const *name, size_t len) {
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
				if (strcmpz(got_name, name, len) == 0)
					return got_rid;
				reader += 1 + 1;
			}	break;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP16: {
				uint16_t got_rid     = UNALIGNED_GETLE16(reader + 0);
				char const *got_name = str_base + UNALIGNED_GETLE16(reader + 2);
				if (strcmpz(got_name, name, len) == 0)
					return got_rid;
				reader += 2 + 2;
			}	break;

			case DDI_EXDAT_O_RNAM | DDI_EXDAT_OP32: {
				uint16_t got_rid     = UNALIGNED_GETLE16(reader + 0);
				char const *got_name = str_base + UNALIGNED_GETLE32(reader + 2);
				if (strcmpz(got_name, name, len) == 0)
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

#define RID_ERR ((uint16_t)-1)
#define RID_UNK ((uint16_t)-2)

/* Returns the ID associated with `key', or:
 * - Throw an error and return `RID_ERR'
 * - Failed to find the ID and return `RID_UNK' */
PRIVATE WUNUSED NONNULL((1, 2)) uint16_t DCALL
FunctionSymbolsByName_TryGetRefIdByObject(FunctionSymbolsByName const *self, DeeObject *key) {
	uint16_t rid;
	if (DeeString_Check(key)) {
		rid = DDI_GetRefIdByName(self->fsbn_func->fo_code->co_ddi,
		                         DeeString_STR(key),
		                         DeeString_SIZE(key));
		if unlikely(rid < self->fsbn_rid_start)
			goto err_no_such_key;
	} else {
		if unlikely(DeeObject_AsUInt16(key, &rid))
			goto err_maybe_overflow;
		if unlikely(OVERFLOW_UADD(rid, self->fsbn_rid_start, &rid))
			goto err_no_such_key;
	}
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	return rid;
err_no_such_key:
	return RID_UNK;
err_maybe_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		return RID_UNK;
	return RID_ERR;
}

PRIVATE WUNUSED NONNULL((1)) uint16_t DCALL
FunctionSymbolsByName_TryGetRefIdByIndex(FunctionSymbolsByName const *self, size_t key) {
	uint16_t rid;
	if unlikely(OVERFLOW_UADD(key, self->fsbn_rid_start, &rid))
		goto err_no_such_key;
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	return rid;
err_no_such_key:
	return RID_UNK;
/*
err:
	return RID_ERR;*/
}

#define FunctionSymbolsByName_TryGetRefIdByString(self, key) \
	FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, strlen(key))
PRIVATE WUNUSED NONNULL((1)) uint16_t DCALL
FunctionSymbolsByName_TryGetRefIdByStringLen(FunctionSymbolsByName const *self,
                                          char const *key, size_t keylen) {
	uint16_t rid = DDI_GetRefIdByName(self->fsbn_func->fo_code->co_ddi, key, keylen);
	if unlikely(rid < self->fsbn_rid_start)
		goto err_no_such_key;
	if unlikely(rid >= self->fsbn_rid_end)
		goto err_no_such_key;
	return rid;
err_no_such_key:
	return RID_UNK;
/*
err:
	return RID_ERR;*/
}

/* Returns the ID associated with `key', or throw an error and return `RID_ERR' */
PRIVATE WUNUSED NONNULL((1, 2)) uint16_t DCALL
FunctionSymbolsByName_GetRefIdByObject(FunctionSymbolsByName const *self, DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	if unlikely(rid == RID_UNK) {
		DeeRT_ErrUnknownKey(self, key);
		rid = RID_ERR;
	}
	return rid;
}

PRIVATE WUNUSED NONNULL((1)) uint16_t DCALL
FunctionSymbolsByName_GetRefIdByIndex(FunctionSymbolsByName const *self, size_t key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByIndex(self, key);
	if unlikely(rid == RID_UNK) {
		DeeRT_ErrUnknownKeyInt((DeeObject *)self, key);
		rid = RID_ERR;
	}
	return rid;
}

#define FunctionSymbolsByName_GetRefIdByString(self, key) \
	FunctionSymbolsByName_GetRefIdByStringLen(self, key, strlen(key))
PRIVATE WUNUSED NONNULL((1)) uint16_t DCALL
FunctionSymbolsByName_GetRefIdByStringLen(FunctionSymbolsByName const *self,
                                          char const *key, size_t keylen) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, keylen);
	if unlikely(rid == RID_UNK) {
		DeeRT_ErrUnknownKeyStrLen((DeeObject *)self, key, keylen);
		rid = RID_ERR;
	}
	return rid;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
funcsymbolsbyname_size(FunctionSymbolsByName *__restrict self) {
	return self->fsbn_rid_end - self->fsbn_rid_start;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
funcsymbolsbyname_mh_setold_ex(FunctionSymbolsByName *self,
                               DeeObject *key, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func = self->fsbn_func;
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	if unlikely(rid == RID_ERR)
		goto err;
	if unlikely(rid == RID_UNK)
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
	return oldvalue; /* Inherit reference */
err_no_such_key:
	return ITER_DONE;
err_unlock_ro:
	DeeFunction_RefLockEndWrite(func);
	DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
funcsymbolsbyname_mh_setnew_ex(FunctionSymbolsByName *self,
                               DeeObject *key, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func = self->fsbn_func;
	uint16_t rid = FunctionSymbolsByName_GetRefIdByObject(self, key);
	if unlikely(rid == RID_ERR)
		goto err;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	if (ITER_ISOK(oldvalue)) {
		Dee_Incref(oldvalue);
		DeeFunction_RefLockEndWrite(func);
		return oldvalue;
	}
	if unlikely(rid < func->fo_code->co_refc)
		goto err_unlock_ro;
	Dee_Incref(value);
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	return ITER_DONE;
err_unlock_ro:
	DeeFunction_RefLockEndWrite(func);
	DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
err:
	return NULL;
}

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

PRIVATE WUNUSED NONNULL((1)) DREF FunctionSymbolsByNameIterator *DCALL
funcsymbolsbyname_mh_map_iterkeys(FunctionSymbolsByName *__restrict self) {
	DREF FunctionSymbolsByNameIterator *result;
	result = DeeObject_MALLOC(FunctionSymbolsByNameIterator);
	if unlikely(!result)
		goto err;
	result->fsbni_seq = self;
	Dee_Incref(self);
	result->fsbni_func = self->fsbn_func;
	result->fsbni_rid  = self->fsbn_rid_start;
	result->fsbni_end  = self->fsbn_rid_end;
	DeeObject_Init(result, &FunctionSymbolsByNameKeysIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_contains(FunctionSymbolsByName *self,
                           DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	if unlikely(rid == RID_ERR)
		goto err;
	return_bool(rid != RID_UNK);
err:
	return NULL;
}

PRIVATE NONNULL((1)) int DCALL
err_funcsymbolsbyname_rid_unbound(FunctionSymbolsByName *self, uint16_t rid) {
	char const *name = DeeCode_GetRSymbolName((DeeObject *)self->fsbn_func->fo_code, rid);
	return name ? DeeRT_ErrUnboundKeyStr((DeeObject *)self, name)
	            : DeeRT_ErrUnboundKeyInt((DeeObject *)self, rid - self->fsbn_rid_start);
}

PRIVATE NONNULL((1)) int DCALL
err_funcsymbolsbyname_rid_readonly(FunctionSymbolsByName *self, uint16_t rid) {
	char const *name = DeeCode_GetRSymbolName((DeeObject *)self->fsbn_func->fo_code, rid);
	return name ? DeeRT_ErrReadOnlyKeyStr((DeeObject *)self, name)
	            : DeeRT_ErrReadOnlyKeyInt((DeeObject *)self, rid - self->fsbn_rid_start);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolsbyname_getitem_common(FunctionSymbolsByName *self, uint16_t rid) {
	DREF DeeObject *result;
	DeeFunctionObject *func = self->fsbn_func;
	if unlikely(rid == RID_ERR)
		goto err;
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[rid];
	if unlikely(!ITER_ISOK(result))
		goto err_unbound_unlock;
	Dee_Incref(result);
	DeeFunction_RefLockEndRead(func);
	return result;
err_unbound_unlock:
	DeeFunction_RefLockEndRead(func);
/*err_unbound:*/
	err_funcsymbolsbyname_rid_unbound(self, rid);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_getitem(FunctionSymbolsByName *self, DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_GetRefIdByObject(self, key);
	return funcsymbolsbyname_getitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolsbyname_getitem_index(FunctionSymbolsByName *self, size_t key) {
	uint16_t rid = FunctionSymbolsByName_GetRefIdByIndex(self, key);
	return funcsymbolsbyname_getitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_getitem_string_len_hash(FunctionSymbolsByName *self,
                                          char const *key, size_t keylen,
                                          Dee_hash_t UNUSED(hash)) {
	uint16_t rid = FunctionSymbolsByName_GetRefIdByStringLen(self, key, keylen);
	return funcsymbolsbyname_getitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolsbyname_trygetitem_common(FunctionSymbolsByName *self, uint16_t rid) {
	DREF DeeObject *result;
	DeeFunctionObject *func = self->fsbn_func;
	if unlikely(rid == RID_ERR)
		goto err;
	if unlikely(rid == RID_UNK)
		goto err_no_such_key;
	DeeFunction_RefLockRead(func);
	result = func->fo_refv[rid];
	if unlikely(!ITER_ISOK(result))
		goto err_no_such_key_unlock;
	Dee_Incref(result);
	DeeFunction_RefLockEndRead(func);
	return result;
err_no_such_key_unlock:
	DeeFunction_RefLockEndRead(func);
err_no_such_key:
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_trygetitem(FunctionSymbolsByName *self, DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	return funcsymbolsbyname_trygetitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
funcsymbolsbyname_trygetitem_index(FunctionSymbolsByName *self, size_t key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByIndex(self, key);
	return funcsymbolsbyname_trygetitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
funcsymbolsbyname_trygetitem_string_len_hash(FunctionSymbolsByName *self,
                                             char const *key, size_t keylen,
                                             Dee_hash_t UNUSED(hash)) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, keylen);
	return funcsymbolsbyname_trygetitem_common(self, rid);
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_bounditem_common(FunctionSymbolsByName *self, uint16_t rid) {
	DeeFunctionObject *func = self->fsbn_func;
	if unlikely(rid == RID_ERR)
		goto err;
	if (rid == RID_UNK)
		goto err_no_such_key;
	DeeFunction_RefLockRead(func);
	if unlikely(!ITER_ISOK(func->fo_refv[rid])) {
		DeeFunction_RefLockEndRead(func);
		return Dee_BOUND_NO;
	}
	DeeFunction_RefLockEndRead(func);
	return Dee_BOUND_YES;
err_no_such_key:
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_bounditem(FunctionSymbolsByName *self, DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	return funcsymbolsbyname_bounditem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_bounditem_index(FunctionSymbolsByName *self, size_t key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByIndex(self, key);
	return funcsymbolsbyname_bounditem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_bounditem_string_len_hash(FunctionSymbolsByName *self,
                                            char const *key, size_t keylen,
                                            Dee_hash_t UNUSED(hash)) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, keylen);
	return funcsymbolsbyname_bounditem_common(self, rid);
}


#define funcsymbolsbyname_hasitem_common(self, rid) \
	_funcsymbolsbyname_hasitem_common(rid)
PRIVATE ATTR_CONST WUNUSED int DCALL
_funcsymbolsbyname_hasitem_common(uint16_t rid) {
	if unlikely(rid == RID_ERR)
		return Dee_HAS_ERR;
	if (rid == RID_UNK)
		return Dee_HAS_NO;
	return Dee_HAS_YES;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_hasitem(FunctionSymbolsByName *self, DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	return funcsymbolsbyname_hasitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_hasitem_index(FunctionSymbolsByName *self, size_t key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByIndex(self, key);
	return funcsymbolsbyname_hasitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_hasitem_string_len_hash(FunctionSymbolsByName *self,
                                          char const *key, size_t keylen,
                                          Dee_hash_t UNUSED(hash)) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, keylen);
	return funcsymbolsbyname_hasitem_common(self, rid);
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_delitem_common(FunctionSymbolsByName *self, uint16_t rid) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func = self->fsbn_func;
	if unlikely(rid == RID_ERR)
		goto err;
	if unlikely(rid == RID_UNK)
		return 0;
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
	err_funcsymbolsbyname_rid_readonly(self, rid);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_delitem(FunctionSymbolsByName *self,
                          DeeObject *key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByObject(self, key);
	return funcsymbolsbyname_delitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
funcsymbolsbyname_delitem_index(FunctionSymbolsByName *self, size_t key) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByIndex(self, key);
	return funcsymbolsbyname_delitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
funcsymbolsbyname_delitem_string_len_hash(FunctionSymbolsByName *self,
                                          char const *key, size_t keylen,
                                          Dee_hash_t UNUSED(hash)) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, keylen);
	return funcsymbolsbyname_delitem_common(self, rid);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
funcsymbolsbyname_setitem_common(FunctionSymbolsByName *self,
                                 uint16_t rid, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeFunctionObject *func = self->fsbn_func;
	if unlikely(rid == RID_ERR)
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
	err_funcsymbolsbyname_rid_readonly(self, rid);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
funcsymbolsbyname_setitem(FunctionSymbolsByName *self,
                          DeeObject *key, DeeObject *value) {
	uint16_t rid = FunctionSymbolsByName_GetRefIdByObject(self, key);
	return funcsymbolsbyname_setitem_common(self, rid, value);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
funcsymbolsbyname_setitem_index(FunctionSymbolsByName *self, size_t key, DeeObject *value) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByIndex(self, key);
	return funcsymbolsbyname_setitem_common(self, rid, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
funcsymbolsbyname_setitem_string_len_hash(FunctionSymbolsByName *self,
                                          char const *key, size_t keylen,
                                          Dee_hash_t UNUSED(hash), DeeObject *value) {
	uint16_t rid = FunctionSymbolsByName_TryGetRefIdByStringLen(self, key, keylen);
	return funcsymbolsbyname_setitem_common(self, rid, value);
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
funcsymbolsbyname_init_kw(FunctionSymbolsByName *__restrict self,
                          size_t argc, DeeObject *const *argv,
                          DeeObject *kw) {
	DeeCodeObject *code;
	STATIC_ASSERT(FIELDS_ADJACENT(FunctionSymbolsByName, fsbn_func, fsbn_rid_start));
	STATIC_ASSERT(FIELDS_ADJACENT(FunctionSymbolsByName, fsbn_rid_start, fsbn_rid_end));
	self->fsbn_rid_start = 0;
	self->fsbn_rid_end   = (uint16_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__func_ridstart_ridend,
	                          "o|" UNPu16 UNPu16 ":_FunctionSymbolsByName",
	                          &self->fsbn_func))
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

STATIC_ASSERT(offsetof(FunctionSymbolsByName, fsbn_func) == offsetof(ProxyObject, po_obj));
#define funcsymbolsbyname_fini  generic_proxy__fini
#define funcsymbolsbyname_visit generic_proxy__visit


PRIVATE struct type_seq funcsymbolsbyname_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&funcsymbolsbyname_setitem,
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ NULL_if_Os((int (DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_bounditem),
	/* .tp_hasitem                    = */ NULL_if_Os((int (DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_hasitem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&funcsymbolsbyname_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&funcsymbolsbyname_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&funcsymbolsbyname_setitem_index,
	/* .tp_bounditem_index            = */ NULL_if_Os((int (DCALL *)(DeeObject *, size_t))&funcsymbolsbyname_bounditem_index),
	/* .tp_hasitem_index              = */ NULL_if_Os((int (DCALL *)(DeeObject *, size_t))&funcsymbolsbyname_hasitem_index),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&funcsymbolsbyname_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&funcsymbolsbyname_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem_string_len_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem_string_len_hash),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem_string_len_hash),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem_string_len_hash),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem_string_len_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem_string_len_hash),
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&funcsymbolsbyname_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&funcsymbolsbyname_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&funcsymbolsbyname_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&funcsymbolsbyname_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ NULL_if_Os((int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&funcsymbolsbyname_bounditem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ NULL_if_Os((int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&funcsymbolsbyname_hasitem_string_len_hash),
};

PRIVATE struct type_method tpconst funcsymbolsbyname_methods[] = {
	TYPE_METHOD_HINTREF(Mapping_setold_ex),
	TYPE_METHOD_HINTREF(Mapping_setnew_ex),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst funcsymbolsbyname_method_hints[] = {
	TYPE_METHOD_HINT(map_setold_ex, &funcsymbolsbyname_mh_setold_ex),
	TYPE_METHOD_HINT(map_setnew_ex, &funcsymbolsbyname_mh_setnew_ex),
	// TODO: TYPE_METHOD_HINT(map_enumerate, &funcsymbolsbyname_mh_map_enumerate),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst funcsymbolsbyname_getsets[] = {
	TYPE_GETTER_AB(STR_iterkeys, &funcsymbolsbyname_mh_map_iterkeys, "->?#KeysIterator"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst funcsymbolsbyname_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(FunctionSymbolsByName, fsbn_func), "->?DFunction"),
	TYPE_MEMBER_FIELD("__ridstart__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByName, fsbn_rid_start)),
	TYPE_MEMBER_FIELD("__ridend__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(FunctionSymbolsByName, fsbn_rid_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst funcsymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FunctionSymbolsByNameIterator_Type),
	TYPE_MEMBER_CONST(STR_KeysIterator, &FunctionSymbolsByNameKeysIterator_Type),
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
	                         "(func:?DFunction,ridstart=!0,ridend?:?Dint)\n"
	                         "\n"
	                         "[](key:?X2?Dstring?Dint)->"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&funcsymbolsbyname_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&funcsymbolsbyname_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(FunctionSymbolsByName),
				/* .tp_any_ctor  = */ (Dee_funptr_t)&funcsymbolsbyname_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&funcsymbolsbyname_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&funcsymbolsbyname_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E5A99B058858326C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &funcsymbolsbyname_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ funcsymbolsbyname_methods,
	/* .tp_getsets       = */ funcsymbolsbyname_getsets,
	/* .tp_members       = */ funcsymbolsbyname_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ funcsymbolsbyname_class_members,
	/* .tp_method_hints  = */ funcsymbolsbyname_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
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
	return Dee_AsObject(result);
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
	return Dee_AsObject(result);
err:
	return NULL;
}




/************************************************************************/
/* YieldFunctionSymbolsByName                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeYieldFunctionObject, yfsbn_yfunc);    /* [1..1][const] The function of the frame (as a cache) */
	uint16_t                                     yfsbn_nargs;     /* [<= yfsbn_yfunc->yf_func->fo_code->co_argc_max][const] The # of arguments to enumerate. */
	uint16_t                                     yfsbn_rid_start; /* [<= frsbn_rid_end][const] First RID/SID to enumerate. */
	uint16_t                                     yfsbn_rid_end;   /* [<= yfsbn_yfunc->yf_func->fo_code->co_refstaticc][const] Last RID/SID to enumerate, plus 1. */
} YieldFunctionSymbolsByName;

typedef union {
	uint32_t yfsbnii_word; /* Index word */
	struct {
		uint16_t i_aid;    /* Next argument index to enumerate */
		uint16_t i_rid;    /* Next reference/static index to enumerate */
	} yfsbnii_idx;
} YieldFunctionSymbolsByNameIteratorIndex;

typedef struct {
	PROXY_OBJECT_HEAD_EX(YieldFunctionSymbolsByName, yfsbni_seq); /* [1..1][const] Underlying frame-symbols sequence. */
	YieldFunctionSymbolsByNameIteratorIndex          yfsbni_idx;  /* Iterator index */
} YieldFunctionSymbolsByNameIterator;

INTDEF DeeTypeObject YieldFunctionSymbolsByNameIterator_Type;
INTDEF DeeTypeObject YieldFunctionSymbolsByNameKeysIterator_Type;
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
	if (DeeObject_AssertTypeExact(rhs, Dee_TYPE(lhs)))
		goto err;
	Dee_return_compare_if_ne(lhs->yfsbni_seq, rhs->yfsbni_seq);
	lhs_idx.yfsbnii_word = atomic_read(&lhs->yfsbni_idx.yfsbnii_word);
	rhs_idx.yfsbnii_word = atomic_read(&rhs->yfsbni_idx.yfsbnii_word);
	Dee_return_compare_if_ne(lhs_idx.yfsbnii_idx.i_aid, rhs_idx.yfsbnii_idx.i_aid);
	Dee_return_compare(lhs_idx.yfsbnii_idx.i_rid, rhs_idx.yfsbnii_idx.i_rid);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeYieldFunctionObject *DCALL
yfuncsymbolsbynameiter_get_yfunc(YieldFunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->yfsbni_seq->yfsbn_yfunc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
yfuncsymbolsbynameiter_get_func(YieldFunctionSymbolsByNameIterator *__restrict self) {
	return_reference_(self->yfsbni_seq->yfsbn_yfunc->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfuncsymbolsbynamekeysiter_get_seq(YieldFunctionSymbolsByNameIterator *__restrict self) {
	return DeeObject_InvokeMethodHint(map_keys, (DeeObject *)self->yfsbni_seq);
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
	DeeArg_Unpack1(err, argc, argv, "YieldFunctionSymbolsByNameIterator", &self->yfsbni_seq);
	if (Dee_TYPE(self->yfsbni_seq) == &DefaultSequence_MapKeys_Type)
		self->yfsbni_seq = (YieldFunctionSymbolsByName *)((DefaultSequence_MapProxy *)self->yfsbni_seq)->dsmp_map;
	if (DeeObject_AssertTypeExact(self->yfsbni_seq, &YieldFunctionSymbolsByName_Type))
		goto err;
	Dee_Incref(self->yfsbni_seq);
	self->yfsbni_idx.yfsbnii_idx.i_aid = 0;
	self->yfsbni_idx.yfsbnii_idx.i_rid = self->yfsbni_seq->yfsbn_rid_start;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_seq) == offsetof(ProxyObject, po_obj));
#define yfuncsymbolsbynameiter_fini  generic_proxy__fini
#define yfuncsymbolsbynameiter_visit generic_proxy__visit

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

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbynameiter_bool(YieldFunctionSymbolsByNameIterator *__restrict self) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex idx;
	idx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	for (;;) {
		if (idx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) {
			if (YieldFunction_GetArg(sym->yfsbn_yfunc, idx.yfsbnii_idx.i_aid))
				return 1;
			++idx.yfsbnii_idx.i_aid;
			continue;
		}
		if (idx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end) {
			DeeFunctionObject *func = sym->yfsbn_yfunc->yf_func;
			DeeObject *value;
			value = atomic_read(&func->fo_refv[idx.yfsbnii_idx.i_rid]);
			if (!ITER_ISOK(value)) {
				++idx.yfsbnii_idx.i_rid;
				continue;
			}
			return 1;
		}
		return 0;
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbynamekeysiter_bool(YieldFunctionSymbolsByNameIterator *__restrict self) {
	bool result;
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex idx;
	idx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	result = (idx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) ||
	         (idx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end);
	return result ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbynameiter_nextpair(YieldFunctionSymbolsByNameIterator *__restrict self,
                                DREF DeeObject *key_and_value[2]) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex oldidx, newidx;
	DeeCodeObject *code = sym->yfsbn_yfunc->yf_func->fo_code;
again:
	oldidx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	newidx.yfsbnii_word = oldidx.yfsbnii_word;
	for (;;) {
		if (newidx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) {
			key_and_value[1] = YieldFunction_GetArg(sym->yfsbn_yfunc, newidx.yfsbnii_idx.i_aid);
			if (!key_and_value[1]) {
				++newidx.yfsbnii_idx.i_aid;
				continue;
			}
			/* Lookup name */
			if (code->co_keywords) {
				key_and_value[0] = (DREF DeeObject *)code->co_keywords[newidx.yfsbnii_idx.i_aid];
				Dee_Incref(key_and_value[0]);
			} else {
				key_and_value[0] = DeeInt_NewUInt16(newidx.yfsbnii_idx.i_aid);
				if unlikely(!key_and_value[0])
					goto err;
			}
			Dee_Incref(key_and_value[1]);
			++newidx.yfsbnii_idx.i_aid;
			break;
		}
		if (newidx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end) {
			char const *refname;
			DeeFunctionObject *func = sym->yfsbn_yfunc->yf_func;
			DeeFunction_RefLockRead(func);
			key_and_value[1] = func->fo_refv[newidx.yfsbnii_idx.i_rid];
			if (!ITER_ISOK(key_and_value[1])) {
				DeeFunction_RefLockEndRead(func);
				++newidx.yfsbnii_idx.i_rid;
				continue;
			}
			Dee_Incref(key_and_value[1]);
			DeeFunction_RefLockEndRead(func);
			refname = DeeCode_GetRSymbolName((DeeObject *)code, newidx.yfsbnii_idx.i_rid);
			key_and_value[0] = refname ? DeeString_New(refname)
			                           : DeeInt_NewUInt16(code->co_argc_max + newidx.yfsbnii_idx.i_rid);
			if unlikely(!key_and_value[0])
				goto err_value;
			++newidx.yfsbnii_idx.i_rid;
			break;
		}
		return 1;
	}
	if (!atomic_cmpxch_or_write(&self->yfsbni_idx.yfsbnii_word,
	                            oldidx.yfsbnii_word,
	                            newidx.yfsbnii_word)) {
		Dee_Decref_unlikely(key_and_value[1]);
		Dee_Decref_unlikely(key_and_value[0]);
		goto again;
	}
	return 0;
err_value:
	Dee_Decref(key_and_value[1]);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfuncsymbolsbynameiter_nextkey(YieldFunctionSymbolsByNameIterator *__restrict self) {
	YieldFunctionSymbolsByName *sym = self->yfsbni_seq;
	YieldFunctionSymbolsByNameIteratorIndex oldidx, newidx;
	DeeCodeObject *code = sym->yfsbn_yfunc->yf_func->fo_code;
	DREF DeeObject *name;
again:
	oldidx.yfsbnii_word = atomic_read(&self->yfsbni_idx.yfsbnii_word);
	newidx.yfsbnii_word = oldidx.yfsbnii_word;
	for (;;) {
		if (newidx.yfsbnii_idx.i_aid < sym->yfsbn_nargs) {
			if (!YieldFunction_GetArg(sym->yfsbn_yfunc, newidx.yfsbnii_idx.i_aid)) {
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
			++newidx.yfsbnii_idx.i_aid;
			break;
		}
		if (newidx.yfsbnii_idx.i_rid < sym->yfsbn_rid_end) {
			char const *refname;
			DeeFunctionObject *func = sym->yfsbn_yfunc->yf_func;
			DeeObject *value;
			value = atomic_read(&func->fo_refv[newidx.yfsbnii_idx.i_rid]);
			if (!ITER_ISOK(value)) {
				++newidx.yfsbnii_idx.i_rid;
				continue;
			}
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
yfuncsymbolsbynameiter_nextkey_with_unbound(YieldFunctionSymbolsByNameIterator *__restrict self) {
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
yfuncsymbolsbynameiter_nextvalue(YieldFunctionSymbolsByNameIterator *__restrict self) {
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

PRIVATE struct type_iterator yfuncsymbolsbynameiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&yfuncsymbolsbynameiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_nextvalue,
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextkey),
};

PRIVATE struct type_nii tpconst yfuncsymbolsbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)NULL,
			/* .nii_advance  = */ (Dee_funptr_t)NULL,
			/* .nii_prev     = */ (Dee_funptr_t)NULL,
			/* .nii_next     = */ (Dee_funptr_t)NULL,
			/* .nii_hasprev  = */ (Dee_funptr_t)NULL,
			/* .nii_peek     = */ (Dee_funptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp yfuncsymbolsbynameiter_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbynameiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &yfuncsymbolsbynameiter_nii,
};

PRIVATE struct type_getset tpconst yfuncsymbolsbynameiter_getsets[] = {
	TYPE_GETTER_AB("__yfunc__", &yfuncsymbolsbynameiter_get_yfunc, "->?Ert:YieldFunction"),
	TYPE_GETTER_AB("__func__", &yfuncsymbolsbynameiter_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst yfuncsymbolsbynameiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_seq), "->?Ert:YieldFunctionSymbolsByName"),
	TYPE_MEMBER_FIELD("__aid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_idx.yfsbnii_idx.i_aid)),
	TYPE_MEMBER_FIELD("__rid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_idx.yfsbnii_idx.i_rid)),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst yfuncsymbolsbynamekeysiter_getsets[] = {
	TYPE_GETTER_AB("__yfunc__", &yfuncsymbolsbynameiter_get_yfunc, "->?Ert:YieldFunction"),
	TYPE_GETTER_AB("__func__", &yfuncsymbolsbynameiter_get_func, "->?DFunction"),
	TYPE_GETTER_AB(STR_seq, &yfuncsymbolsbynamekeysiter_get_seq, "->?Ert:MapKeys"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst yfuncsymbolsbynamekeysiter_members[] = {
	TYPE_MEMBER_FIELD("__aid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_idx.yfsbnii_idx.i_aid)),
	TYPE_MEMBER_FIELD("__rid__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByNameIterator, yfsbni_idx.yfsbnii_idx.i_rid)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject YieldFunctionSymbolsByNameIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionSymbolsByNameIterator",
	/* .tp_doc      = */ DOC("(symbols:?X2?Ert:YieldFunctionSymbolsByName?C?Ert:MapKeys?Ert:YieldFunctionSymbolsByName)\n"
	                         "\n"
	                         "next->?T2?X2?Dstring?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(YieldFunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&yfuncsymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &yfuncsymbolsbynameiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &yfuncsymbolsbynameiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yfuncsymbolsbynameiter_getsets,
	/* .tp_members       = */ yfuncsymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


INTERN DeeTypeObject YieldFunctionSymbolsByNameKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionSymbolsByNameKeysIterator",
	/* .tp_doc      = */ DOC("(symbols:?X2?Ert:YieldFunctionSymbolsByName?C?Ert:MapKeys?Ert:YieldFunctionSymbolsByName)\n"
	                         "\n"
	                         "next->?X2?Dstring?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&yfuncsymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(YieldFunctionSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynamekeysiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&yfuncsymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &yfuncsymbolsbynameiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfuncsymbolsbynameiter_nextkey_with_unbound,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yfuncsymbolsbynamekeysiter_getsets,
	/* .tp_members       = */ yfuncsymbolsbynamekeysiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
yfuncsymbolsbyname_size(YieldFunctionSymbolsByName *__restrict self) {
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
YieldFunctionSymbolsByName_TryLookupSymByStringLen(YieldFunctionSymbolsByName const *self,
                                                   char const *name, size_t len) {
	uint16_t i;
	DeeCodeObject *code = self->yfsbn_yfunc->yf_func->fo_code;
	if (code->co_keywords) {
		for (i = 0; i < self->yfsbn_nargs; ++i) {
			DeeStringObject *argi = code->co_keywords[i];
			if (strcmpz(DeeString_STR(argi), name, len) == 0)
				return yfuncsymbol_makearg(i);
		}
	}
	i = DDI_GetRefIdByName(code->co_ddi, name, len);
	if (i >= self->yfsbn_rid_start && i < self->yfsbn_rid_end)
		return yfuncsymbol_makeref(i);
	return YFUNCSYMBOL_INVALID;
}

/* Try to find the symbol referenced by `key'
 * @return: YFUNCSYMBOL_INVALID: No such symbol "key"
 * @return: YFUNCSYMBOL_ERROR:   An error was thrown. */
PRIVATE WUNUSED NONNULL((1)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_TryLookupSymByIndex(YieldFunctionSymbolsByName const *self, size_t key) {
	DeeCodeObject *code;
	code = self->yfsbn_yfunc->yf_func->fo_code;
	if (key < code->co_argc_max) {
		if (key >= self->yfsbn_nargs)
			goto badsym;
		return yfuncsymbol_makearg(key);
	}
	key -= code->co_argc_max;
	if (key >= self->yfsbn_rid_start && key < self->yfsbn_rid_end)
		return yfuncsymbol_makeref(key);
badsym:
	return YFUNCSYMBOL_INVALID;
}

/* Try to find the symbol referenced by `key'
 * @return: YFUNCSYMBOL_INVALID: No such symbol "key"
 * @return: YFUNCSYMBOL_ERROR:   An error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_TryLookupSymByObject(YieldFunctionSymbolsByName const *self,
                                                DeeObject *key) {
	size_t index;
	if (DeeString_Check(key)) {
		return YieldFunctionSymbolsByName_TryLookupSymByStringLen(self,
		                                                          DeeString_STR(key),
		                                                          DeeString_SIZE(key));
	}
	index = DeeObject_AsSizeDirect(key);
	if unlikely(index == (size_t)-1)
		goto err_maybe_overflow;
	return YieldFunctionSymbolsByName_TryLookupSymByIndex(self, index);
err_maybe_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		return YFUNCSYMBOL_INVALID;
	return YFUNCSYMBOL_ERROR;
}

/* Same as `YieldFunctionSymbolsByName_TryLookupSymByObject',
 * but throw an error in case of a bad key.
 * @return: YFUNCSYMBOL_ERROR: An error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_LookupSymByObject(YieldFunctionSymbolsByName const *self,
                                             DeeObject *key) {
	yfuncsymbol_t result = YieldFunctionSymbolsByName_TryLookupSymByObject(self, key);
	if unlikely(result == YFUNCSYMBOL_INVALID) {
		DeeRT_ErrUnknownKey(self, key);
		result = YFUNCSYMBOL_ERROR;
	}
	return result;
}

/*
PRIVATE WUNUSED NONNULL((1, 2)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_LookupSymByStringLen(YieldFunctionSymbolsByName const *self,
                                                char const *key, size_t keylen) {
	yfuncsymbol_t result = YieldFunctionSymbolsByName_TryLookupSymByStringLen(self, key, keylen);
	if unlikely(result == YFUNCSYMBOL_INVALID) {
		DeeRT_ErrUnknownKeyStrLen((DeeObject *)self, key, keylen);
		result = YFUNCSYMBOL_ERROR;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) yfuncsymbol_t DCALL
YieldFunctionSymbolsByName_LookupSymByIndex(YieldFunctionSymbolsByName const *self, size_t key) {
	yfuncsymbol_t result = YieldFunctionSymbolsByName_TryLookupSymByIndex(self, key);
	if unlikely(result == YFUNCSYMBOL_INVALID) {
		DeeRT_ErrUnknownKeyInt((DeeObject *)self, key);
		result = YFUNCSYMBOL_ERROR;
	}
	return result;
}
*/

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
yfuncsymbolsbyname_mh_setold_ex(YieldFunctionSymbolsByName *self,
                                DeeObject *key, DeeObject *value) {
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_TryLookupSymByObject(self, key);
	uint16_t rid;
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	if unlikely(symid == YFUNCSYMBOL_INVALID)
		return ITER_DONE;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid))
		goto err_rokey;
	rid = yfuncsymbol_asrid(symid);
	if unlikely(rid < func->fo_code->co_refc)
		goto err_rokey;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	if (ITER_ISOK(oldvalue)) {
		Dee_Incref(value);
		func->fo_refv[rid] = value;
		DeeFunction_RefLockEndWrite(func);
		DeeFutex_WakeAll(&func->fo_refv[rid]);
		return oldvalue;
	}
	DeeFunction_RefLockEndWrite(func);
	return ITER_DONE;
err_rokey:
	DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
yfuncsymbolsbyname_mh_setnew_ex(YieldFunctionSymbolsByName *self,
                                DeeObject *key, DeeObject *value) {
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_LookupSymByObject(self, key);
	uint16_t rid;
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid))
		goto err_rokey;
	rid = yfuncsymbol_asrid(symid);
	if unlikely(rid < func->fo_code->co_refc)
		goto err_rokey;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	if (ITER_ISOK(oldvalue)) {
		Dee_Incref(oldvalue);
		DeeFunction_RefLockEndWrite(func);
		return oldvalue;
	}
	Dee_Incref(value);
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	return ITER_DONE;
err_rokey:
	DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
err:
	return NULL;
}

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

PRIVATE WUNUSED NONNULL((1)) DREF YieldFunctionSymbolsByNameIterator *DCALL
yfuncsymbolsbyname_mh_map_iterkeys(YieldFunctionSymbolsByName *__restrict self) {
	DREF YieldFunctionSymbolsByNameIterator *result;
	result = DeeObject_MALLOC(YieldFunctionSymbolsByNameIterator);
	if unlikely(!result)
		goto err;
	result->yfsbni_seq = self;
	Dee_Incref(self);
	result->yfsbni_idx.yfsbnii_idx.i_aid = 0;
	result->yfsbni_idx.yfsbnii_idx.i_rid = self->yfsbn_rid_start;
	DeeObject_Init(result, &YieldFunctionSymbolsByNameKeysIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbyname_contains(YieldFunctionSymbolsByName *self,
                            DeeObject *key) {
	yfuncsymbol_t symid;
	symid = YieldFunctionSymbolsByName_TryLookupSymByObject(self, key);
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
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_LookupSymByObject(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	result = YieldFunction_TryGetSymbol(self->yfsbn_yfunc, symid);
	if unlikely(!result)
		DeeRT_ErrUnboundKey((DeeObject *)self, key);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yfuncsymbolsbyname_trygetitem(YieldFunctionSymbolsByName *self, DeeObject *key) {
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_TryLookupSymByObject(self, key);
	if (symid != YFUNCSYMBOL_INVALID) {
		DREF DeeObject *result;
		if unlikely(symid == YFUNCSYMBOL_ERROR)
			goto err;
		result = YieldFunction_TryGetSymbol(self->yfsbn_yfunc, symid);
		if (result != NULL)
			return result;
	}
	return ITER_DONE;
err:
	return NULL;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbyname_bounditem(YieldFunctionSymbolsByName *self, DeeObject *key) {
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_TryLookupSymByObject(self, key);
	if (symid != YFUNCSYMBOL_INVALID) {
		DREF DeeObject *result;
		if unlikely(symid == YFUNCSYMBOL_ERROR)
			goto err;
		result = YieldFunction_TryGetSymbol(self->yfsbn_yfunc, symid);
		if (result != NULL) {
			Dee_Decref(result);
			return Dee_BOUND_YES;
		}
		return Dee_BOUND_NO;
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbyname_hasitem(YieldFunctionSymbolsByName *self, DeeObject *key) {
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_TryLookupSymByObject(self, key);
	if (symid != YFUNCSYMBOL_INVALID) {
		if unlikely(symid == YFUNCSYMBOL_ERROR)
			goto err;
		return Dee_HAS_YES;
	}
	return Dee_HAS_NO;
err:
	return Dee_HAS_ERR;
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
yfuncsymbolsbyname_setitem(YieldFunctionSymbolsByName *self,
                           DeeObject *key, DeeObject *value) {
	uint16_t rid;
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_LookupSymByObject(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid))
		goto err_rokey;
	rid = yfuncsymbol_asrid(symid);
	if unlikely(rid < func->fo_code->co_refc)
		goto err_rokey;
	Dee_Incref(value);
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	func->fo_refv[rid] = value;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	if (ITER_ISOK(oldvalue))
		Dee_Incref(oldvalue);
	return 0;
err_rokey:
	DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbyname_delitem(YieldFunctionSymbolsByName *self,
                           DeeObject *key) {
	uint16_t rid;
	DeeFunctionObject *func;
	DREF DeeObject *oldvalue;
	yfuncsymbol_t symid = YieldFunctionSymbolsByName_LookupSymByObject(self, key);
	if unlikely(symid == YFUNCSYMBOL_ERROR)
		goto err;
	func = self->yfsbn_yfunc->yf_func;
	if unlikely(!yfuncsymbol_isrid(symid))
		goto err_rokey;
	rid = yfuncsymbol_asrid(symid);
	if unlikely(rid < func->fo_code->co_refc)
		goto err_rokey;
	DeeFunction_RefLockWrite(func);
	oldvalue = func->fo_refv[rid];
	func->fo_refv[rid] = NULL;
	DeeFunction_RefLockEndWrite(func);
	DeeFutex_WakeAll(&func->fo_refv[rid]);
	if (ITER_ISOK(oldvalue))
		Dee_Incref(oldvalue);
	return 0;
err_rokey:
	DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfuncsymbolsbyname_deep(YieldFunctionSymbolsByName *__restrict self,
                        YieldFunctionSymbolsByName *__restrict other) {
	self->yfsbn_yfunc = (DREF DeeYieldFunctionObject *)DeeObject_DeepCopy((DeeObject *)other->yfsbn_yfunc);
	if unlikely(!self->yfsbn_yfunc)
		goto err;
	self->yfsbn_nargs     = other->yfsbn_nargs;
	self->yfsbn_rid_start = other->yfsbn_rid_start;
	self->yfsbn_rid_end   = other->yfsbn_rid_end;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfuncsymbolsbyname_init_kw(YieldFunctionSymbolsByName *__restrict self,
                           size_t argc, DeeObject *const *argv,
                           DeeObject *kw) {
	STATIC_ASSERT(FIELDS_ADJACENT(YieldFunctionSymbolsByName, yfsbn_yfunc, yfsbn_nargs));
	STATIC_ASSERT(FIELDS_ADJACENT(YieldFunctionSymbolsByName, yfsbn_nargs, yfsbn_rid_start));
	STATIC_ASSERT(FIELDS_ADJACENT(YieldFunctionSymbolsByName, yfsbn_rid_start, yfsbn_rid_end));
	DeeCodeObject *code;
	self->yfsbn_nargs     = (uint16_t)-1;
	self->yfsbn_rid_start = 0;
	self->yfsbn_rid_end   = (uint16_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__yfunc_argc_ridstart_ridend,
	                          "o|" UNPu16 UNPu16 UNPu16 ":_YieldFunctionSymbolsByName",
	                          &self->yfsbn_yfunc))
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

STATIC_ASSERT(offsetof(YieldFunctionSymbolsByName, yfsbn_yfunc) == offsetof(ProxyObject, po_obj));
#define yfuncsymbolsbyname_fini  generic_proxy__fini
#define yfuncsymbolsbyname_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeFunctionObject *DCALL
yfuncsymbolsbyname_getfunc(YieldFunctionSymbolsByName *__restrict self) {
	return_reference_(self->yfsbn_yfunc->yf_func);
}


PRIVATE struct type_seq yfuncsymbolsbyname_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&yfuncsymbolsbyname_setitem,
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ NULL_if_Os((int (DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_bounditem),
	/* .tp_hasitem                    = */ NULL_if_Os((int (DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_hasitem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_size,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&yfuncsymbolsbyname_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__delitem_index__with__delitem), // TODO: (int (DCALL *)(DeeObject *, size_t))&yfuncsymbolsbyname_delitem_index,
	/* .tp_setitem_index              = */ DEFIMPL(&default__setitem_index__with__setitem), // TODO: (int (DCALL *)(DeeObject *, size_t, DeeObject *))&yfuncsymbolsbyname_setitem_index,
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__bounditem), // TODO: (int (DCALL *)(DeeObject *, size_t))&yfuncsymbolsbyname_bounditem_index,
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__hasitem), // TODO: (int (DCALL *)(DeeObject *, size_t))&yfuncsymbolsbyname_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yfuncsymbolsbyname_trygetitem,
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__trygetitem), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&yfuncsymbolsbyname_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&yfuncsymbolsbyname_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&yfuncsymbolsbyname_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem), // TODO: (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&yfuncsymbolsbyname_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem), // TODO: (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&yfuncsymbolsbyname_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem), // TODO: (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&yfuncsymbolsbyname_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem), // TODO: (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&yfuncsymbolsbyname_hasitem_string_len_hash,
};

PRIVATE struct type_method tpconst yfuncsymbolsbyname_methods[] = {
	TYPE_METHOD_HINTREF(Mapping_setold_ex),
	TYPE_METHOD_HINTREF(Mapping_setnew_ex),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst yfuncsymbolsbyname_method_hints[] = {
	TYPE_METHOD_HINT(map_setold_ex, &yfuncsymbolsbyname_mh_setold_ex),
	TYPE_METHOD_HINT(map_setnew_ex, &yfuncsymbolsbyname_mh_setnew_ex),
	// TODO: TYPE_METHOD_HINT(map_enumerate, &yfuncsymbolsbyname_mh_map_enumerate),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst yfuncsymbolsbyname_getsets[] = {
	TYPE_GETTER_AB(STR_iterkeys, &yfuncsymbolsbyname_mh_map_iterkeys, "->?#KeysIterator"),
	TYPE_GETTER_AB("__func__", &yfuncsymbolsbyname_getfunc, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst yfuncsymbolsbyname_members[] = {
	TYPE_MEMBER_FIELD_DOC("__yfunc__", STRUCT_OBJECT, offsetof(YieldFunctionSymbolsByName, yfsbn_yfunc), "->?Ert:YieldFunction"),
	TYPE_MEMBER_FIELD("__nargs__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByName, yfsbn_nargs)),
	TYPE_MEMBER_FIELD("__ridstart__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByName, yfsbn_rid_start)),
	TYPE_MEMBER_FIELD("__ridend__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(YieldFunctionSymbolsByName, yfsbn_rid_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst yfuncsymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &YieldFunctionSymbolsByNameIterator_Type),
	TYPE_MEMBER_CONST(STR_KeysIterator, &YieldFunctionSymbolsByNameKeysIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject YieldFunctionSymbolsByName_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionSymbolsByName",
	/* .tp_doc      = */ DOC("A ${{(int | string): Object}}-like mapping for references, statics, and arguments.\n"
	                         "\n"
	                         "(yfunc:?Ert:YieldFunction,argc?:?Dint,ridstart=!0,ridend?:?Dint)\n"
	                         "\n"
	                         "[](key:?X2?Dstring?Dint)->"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&yfuncsymbolsbyname_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&yfuncsymbolsbyname_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(YieldFunctionSymbolsByName),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&yfuncsymbolsbyname_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfuncsymbolsbyname_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&yfuncsymbolsbyname_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E5A99B058858326C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &yfuncsymbolsbyname_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ yfuncsymbolsbyname_methods,
	/* .tp_getsets       = */ yfuncsymbolsbyname_getsets,
	/* .tp_members       = */ yfuncsymbolsbyname_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ yfuncsymbolsbyname_class_members,
	/* .tp_method_hints  = */ yfuncsymbolsbyname_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
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
	result->yfsbn_nargs = code->co_argc_max;
	result->yfsbn_rid_start = 0;
	result->yfsbn_rid_end   = code->co_refstaticc;
	DeeObject_Init(result, &YieldFunctionSymbolsByName_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}




/************************************************************************/
/* FrameArgs                                                            */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeFrameObject, fa_frame, /* [1..1][const] The frame in question */
	                      DeeCodeObject,  fa_code); /* [1..1][const] The code running in `fa_frame' (cache) */
} FrameArgs;

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
frameargs_size(FrameArgs *__restrict self) {
	return self->fa_code->co_argc_max;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frameargs_getitem_index(FrameArgs *__restrict self, size_t index) {
	struct code_frame const *frame;
	DREF DeeObject *result;
	DeeCodeObject *code = self->fa_code;
	if unlikely(index >= code->co_argc_max) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, code->co_argc_max);
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
			DeeRT_ErrUnboundIndex((DeeObject *)self, index);
			goto err;
		}
	}
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
	return result;
err:
	return NULL;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1)) int DCALL
frameargs_bounditem_index(FrameArgs *__restrict self, size_t index) {
	struct code_frame const *frame;
	DeeCodeObject *code = self->fa_code;
	if (index >= code->co_argc_max)
		return Dee_BOUND_MISSING;
	frame = DeeFrame_LockRead((DeeObject *)self->fa_frame);
	if unlikely(!frame)
		goto err;
	if likely(index < frame->cf_argc) {
		/*frame->cf_argv[index];*/
	} else if (frame->cf_kw) {
		if (!frame->cf_kw->fk_kargv[index - frame->cf_argc])
			goto use_default;
	} else {
use_default:
		if (!code->co_defaultv[index - code->co_argc_min]) {
			DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
			return Dee_BOUND_NO;
		}
	}
	DeeFrame_LockEndRead((DeeObject *)self->fa_frame);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}
#endif /* !__OPTIMIZE_SIZE__ */

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

PRIVATE WUNUSED NONNULL((1)) int DCALL
frameargs_init(FrameArgs *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	struct code_frame const *frame;
	DeeArg_Unpack1(err, argc, argv, "FrameArgs", &self->fa_frame);
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


STATIC_ASSERT(offsetof(FrameArgs, fa_frame) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(FrameArgs, fa_frame) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(FrameArgs, fa_code) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(FrameArgs, fa_code) == offsetof(ProxyObject2, po_obj2));
#define frameargs_copy  generic_proxy2__copy_alias12
#define frameargs_fini  generic_proxy2__fini
#define frameargs_visit generic_proxy2__visit

PRIVATE struct type_seq frameargs_seq = {
	/* .tp_iter               = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&frameargs_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&frameargs_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&frameargs_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ NULL_if_Os((int (DCALL *)(DeeObject *, size_t))&frameargs_bounditem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
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
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&frameargs_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&frameargs_init,
				TYPE_FIXED_ALLOCATOR(FrameArgs)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&frameargs_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&frameargs_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &frameargs_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ frameargs_getsets,
	/* .tp_members       = */ frameargs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL, /* TODO: seq_enumerate_index */
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/************************************************************************/
/* FrameLocals                                                          */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFrameObject, fl_frame); /* [1..1][const] The frame in question */
	uint16_t                             fl_localc; /* [const] The # of local variables there are (cache) */
} FrameLocals;

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framelocals_size(FrameLocals *__restrict self) {
	return self->fl_localc;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framelocals_getitem_index(FrameLocals *__restrict self, size_t index) {
	struct code_frame const *frame;
	DREF DeeObject *result;
	if unlikely(index >= self->fl_localc) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_localc);
		goto err;
	}
	frame = DeeFrame_LockRead((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	result = frame->cf_frame[index];
	if unlikely(!result) {
		DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
	return result;
err:
	return NULL;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1)) int DCALL
framelocals_bounditem_index(FrameLocals *__restrict self, size_t index) {
	struct code_frame const *frame;
	if unlikely(index >= self->fl_localc)
		return Dee_BOUND_MISSING;
	frame = DeeFrame_LockRead((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	if unlikely(!frame->cf_frame[index]) {
		DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
		return Dee_BOUND_NO;
	}
	DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE WUNUSED NONNULL((1)) int DCALL
framelocals_delitem_index(FrameLocals *__restrict self, size_t index) {
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	if unlikely(index >= self->fl_localc) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_localc);
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
framelocals_setitem_index(FrameLocals *self, size_t index, DeeObject *value) {
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	if unlikely(index >= self->fl_localc) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_localc);
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
framelocals_mh_seq_xchitem_index(FrameLocals *self, size_t index, DeeObject *value) {
	struct code_frame *frame;
	DREF DeeObject *oldvalue;
	if unlikely(index >= self->fl_localc) {
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->fl_localc);
		goto err;
	}
	frame = DeeFrame_LockWrite((DeeObject *)self->fl_frame);
	if unlikely(!frame)
		goto err;
	oldvalue = frame->cf_frame[index];
	if unlikely(!oldvalue) {
		DeeFrame_LockEndWrite((DeeObject *)self->fl_frame);
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	frame->cf_frame[index] = value; /* Inherit reference (x2) */
	DeeFrame_LockEndWrite((DeeObject *)self->fl_frame);
	return oldvalue;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
framelocals_mh_seq_enumerate_index(FrameLocals *__restrict self, Dee_seq_enumerate_index_t proc,
                                   void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct code_frame const *frame;
	if (end > self->fl_localc)
		end = self->fl_localc;
	for (; start < end; ++start) {
		DREF DeeObject *item;
		frame = DeeFrame_LockRead((DeeObject *)self->fl_frame);
		if unlikely(!frame)
			goto err;
		item = frame->cf_frame[start];
		Dee_XIncref(item);
		DeeFrame_LockEndRead((DeeObject *)self->fl_frame);
		temp = (*proc)(arg, start, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
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
	DeeArg_Unpack1(err, argc, argv, "FrameLocals", &self->fl_frame);
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

STATIC_ASSERT(offsetof(FrameLocals, fl_frame) == offsetof(ProxyObject, po_obj));
#define framelocals_fini  generic_proxy__fini
#define framelocals_visit generic_proxy__visit

PRIVATE struct type_method tpconst framelocals_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst framelocals_method_hints[] = {
	TYPE_METHOD_HINT(seq_xchitem_index, &framelocals_mh_seq_xchitem_index),
	TYPE_METHOD_HINT(seq_enumerate_index, &framelocals_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq framelocals_seq = {
	/* .tp_iter               = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&framelocals_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&framelocals_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&framelocals_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&framelocals_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&framelocals_setitem_index,
	/* .tp_bounditem_index    = */ NULL_if_Os((int (DCALL *)(DeeObject *, size_t))&framelocals_bounditem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
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
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&framelocals_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&framelocals_init,
				TYPE_FIXED_ALLOCATOR(FrameLocals)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framelocals_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&framelocals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &framelocals_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ framelocals_methods,
	/* .tp_getsets       = */ framelocals_getsets,
	/* .tp_members       = */ framelocals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ framelocals_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/************************************************************************/
/* FrameStack                                                           */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeFrameObject, fs_frame); /* [1..1][const] The frame in question */
} FrameStack;

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framestack_size(FrameStack *__restrict self) {
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
framestack_getitem_index(FrameStack *__restrict self, size_t index) {
	uint16_t stackc;
	struct code_frame const *frame;
	DREF DeeObject *result;
	frame = DeeFrame_LockRead((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	stackc = Dee_code_frame_getspaddr(frame);
	if unlikely(index >= stackc) {
		DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, stackc);
		goto err;
	}
	result = frame->cf_stack[index];
	Dee_Incref(result);
	DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
	return result;
err:
	return NULL;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1)) int DCALL
framestack_bounditem_index(FrameStack *__restrict self, size_t index) {
	uint16_t stackc;
	struct code_frame const *frame;
	DeeObject *value;
	frame = DeeFrame_LockRead((DeeObject *)self->fs_frame);
	if unlikely(!frame)
		goto err;
	stackc = Dee_code_frame_getspaddr(frame);
	if unlikely(index >= stackc) {
		DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
		return Dee_BOUND_MISSING;
	}
	value = frame->cf_stack[index];
	DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
	return Dee_BOUND_FROMBOOL(value);
err:
	return Dee_BOUND_ERR;
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
framestack_mh_seq_xchitem_index(FrameStack *self, size_t index, DeeObject *value) {
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
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, stackc);
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
framestack_setitem_index(FrameStack *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	oldvalue = framestack_mh_seq_xchitem_index(self, index, value);
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
framestack_init(FrameStack *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "FrameStack", &self->fs_frame);
	if (DeeObject_AssertTypeExact(self->fs_frame, &DeeFrame_Type))
		goto err;
	Dee_Incref(self->fs_frame);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(FrameStack, fs_frame) == offsetof(ProxyObject, po_obj));
#define framestack_copy  generic_proxy__copy_alias
#define framestack_fini  generic_proxy__fini
#define framestack_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
framestack_mh_seq_enumerate_index(FrameStack *__restrict self, Dee_seq_enumerate_index_t proc,
                                  void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct code_frame const *frame;
	for (; start < end; ++start) {
		uint16_t stackc;
		DREF DeeObject *item;
		frame = DeeFrame_LockRead((DeeObject *)self->fs_frame);
		if unlikely(!frame)
			goto err;
		stackc = Dee_code_frame_getspaddr(frame);
		if (end > stackc) {
			end = stackc;
			if (start >= end) {
				DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
				break;
			}
		}
		item = frame->cf_stack[start];
		Dee_Incref(item);
		DeeFrame_LockEndRead((DeeObject *)self->fs_frame);
		temp = (*proc)(arg, start, item);
		Dee_Decref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
framestack_mh_seq_insert(FrameStack *self, size_t index, DeeObject *value) {
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
framestack_mh_seq_pop(FrameStack *self, Dee_ssize_t index) {
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
		DeeRT_ErrEmptySequence(self);
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

PRIVATE struct type_method tpconst framestack_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(Sequence_insert),
	TYPE_METHOD_HINTREF(Sequence_pop),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst framestack_method_hints[] = {
	TYPE_METHOD_HINT(seq_xchitem_index, &framestack_mh_seq_xchitem_index),
	TYPE_METHOD_HINT(seq_insert, &framestack_mh_seq_insert),
	TYPE_METHOD_HINT(seq_pop, &framestack_mh_seq_pop),
	TYPE_METHOD_HINT(seq_enumerate_index, &framestack_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst framestack_getsets[] = {
	TYPE_GETTER("__func__", &framestack_get_func, "->?DFunction"),
	TYPE_GETSET_END
};

PRIVATE struct type_seq framestack_seq = {
	/* .tp_iter               = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&framestack_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&framestack_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&framestack_setitem_index,
	/* .tp_bounditem_index    = */ NULL_if_Os((int (DCALL *)(DeeObject *, size_t))&framestack_bounditem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE struct type_member tpconst framestack_members[] = {
	TYPE_MEMBER_FIELD_DOC("__frame__", STRUCT_OBJECT, offsetof(FrameStack, fs_frame), "->?Ert:Frame"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst framestack_class_members[] = {
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
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
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&framestack_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&framestack_init,
				TYPE_FIXED_ALLOCATOR(FrameStack)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framestack_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&framestack_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__22D95991F3D69B20),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &framestack_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ framestack_methods,
	/* .tp_getsets       = */ framestack_getsets,
	/* .tp_members       = */ framestack_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ framestack_class_members,
	/* .tp_method_hints  = */ framestack_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
err:
	return NULL;
}




/************************************************************************/
/* FrameSymbolsByName                                                   */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeFrameObject,    frsbn_frame,     /* [1..1][const] The frame in question */
	                      DeeFunctionObject, frsbn_func);     /* [1..1][const] The function of the frame (as a cache) */
	uint16_t                                 frsbn_nargs;     /* [<= frsbn_func->fo_code->co_argc_max][const] The # of arguments to enumerate. */
	uint16_t                                 frsbn_rid_start; /* [<= frsbn_rid_end][const] First RID/SID to enumerate. */
	uint16_t                                 frsbn_rid_end;   /* [<= frsbn_func->fo_code->co_refstaticc][const] Last RID/SID to enumerate, plus 1. */
	uint16_t                                 frsbn_localc;    /* [<= frsbn_func->fo_code->co_localc][const] The # of locals to enumerate. */
	uint16_t                                 frsbn_stackc;    /* [const] The # of stack slots to enumerate (during enum, stop early if less than this remain). */
} FrameSymbolsByName;

typedef struct {
	uint16_t frsbnii_aid; /* [<= frsbni_seq->frsbn_nargs] Next arg to enumerate, or `>= frsbn_nargs' if all were enumerated. */
	uint16_t frsbnii_rid; /* [>= frsbni_seq->frsbn_rid_start && <= frsbni_seq->frsbn_rid_end] Next ref/static to enumerate, or `>= frsbn_rid_end' if all were enumerated. */
	uint16_t frsbnii_lid; /* [<= frsbni_seq->frsbn_localc] Next local to enumerate, or `>= frsbn_localc' if all were enumerated. */
	uint16_t frsbnii_nsp; /* [<= frsbni_seq->frsbn_stackc] NextStackPointer to enumerate, or `>= frsbn_stackc' if all were enumerated. */
} FrameSymbolsByNameIteratorIndex;

typedef struct {
	PROXY_OBJECT_HEAD_EX(FrameSymbolsByName, frsbni_seq); /* [1..1][const] Underlying frame-symbols sequence. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t                        frsbni_lock; /* Lock for the below indices */
#endif /* !CONFIG_NO_THREADS */
	FrameSymbolsByNameIteratorIndex          frsbni_idx;  /* Iterator index */
} FrameSymbolsByNameIterator;

#define FrameSymbolsByNameIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->frsbni_lock)
#define FrameSymbolsByNameIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->frsbni_lock)

INTDEF DeeTypeObject FrameSymbolsByNameIterator_Type;
INTDEF DeeTypeObject FrameSymbolsByNameKeysIterator_Type;
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
	if (DeeObject_AssertTypeExact(rhs, &FrameSymbolsByNameIterator_Type))
		goto err;
	Dee_return_compare_if_ne(lhs->frsbni_seq, rhs->frsbni_seq);
	if (lhs == rhs)
		return 0;
	DeeLock_Acquire2(FrameSymbolsByNameIterator_LockAcquire(lhs),
	                 FrameSymbolsByNameIterator_LockTryAcquire(lhs),
	                 FrameSymbolsByNameIterator_LockRelease(lhs),
	                 FrameSymbolsByNameIterator_LockAcquire(rhs),
	                 FrameSymbolsByNameIterator_LockTryAcquire(rhs),
	                 FrameSymbolsByNameIterator_LockRelease(rhs));
	if (lhs->frsbni_idx.frsbnii_aid != rhs->frsbni_idx.frsbnii_aid) {
		result = Dee_CompareNe(lhs->frsbni_idx.frsbnii_aid, rhs->frsbni_idx.frsbnii_aid);
	} else if (lhs->frsbni_idx.frsbnii_rid != rhs->frsbni_idx.frsbnii_rid) {
		result = Dee_CompareNe(lhs->frsbni_idx.frsbnii_rid, rhs->frsbni_idx.frsbnii_rid);
	} else if (lhs->frsbni_idx.frsbnii_lid != rhs->frsbni_idx.frsbnii_lid) {
		result = Dee_CompareNe(lhs->frsbni_idx.frsbnii_lid, rhs->frsbni_idx.frsbnii_lid);
	} else if (lhs->frsbni_idx.frsbnii_nsp != rhs->frsbni_idx.frsbnii_nsp) {
		result = Dee_CompareNe(lhs->frsbni_idx.frsbnii_nsp, rhs->frsbni_idx.frsbnii_nsp);
	} else {
		result = 0;
	}
	FrameSymbolsByNameIterator_LockRelease(lhs);
	FrameSymbolsByNameIterator_LockRelease(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
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
	DeeArg_Unpack1(err, argc, argv, "FrameSymbolsByNameIterator", &self->frsbni_seq);
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

STATIC_ASSERT(offsetof(FrameSymbolsByNameIterator, frsbni_seq) == offsetof(ProxyObject, po_obj));
#define framesymbolsbynameiter_fini  generic_proxy__fini
#define framesymbolsbynameiter_visit generic_proxy__visit

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
					char const *local_name;
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
					char const *stack_name;
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
 * @param: value:     The new value, or NULL to unbound the location.
 * @return: * :        The old value attached to "clid"
 * @return: ITER_DONE: The old value attached to "clid" used to be unbound
 * @return: NULL:      Error */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
FrameSymbolsByName_XchCLidValue(FrameSymbolsByName *self,
                                canonical_lid_t clid,
                                /*0..1*/ DeeObject *value) {
	DREF DeeObject *result = ITER_DONE;
	struct code_frame const *frame;
	struct canonical_lid_location loc;
	frame = DeeFrame_LockWrite((DeeObject *)self->frsbn_frame);
	if unlikely(!frame)
		goto err;
	if unlikely(!code_frame_get_clid_addr(frame, &loc, clid)) {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		DREF DeeObject *name;
		name = FrameSymbolsByName_GetCLidName(self, clid);
		if unlikely(!name)
			goto err;
		DeeRT_ErrUnknownKey(self, name);
		Dee_Decref(name);
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
	{
		DREF DeeObject *name;
err_ro:
		name = FrameSymbolsByName_GetCLidName(self, clid);
		if unlikely(!name)
			goto err;
		DeeRT_ErrReadOnlyKey((DeeObject *)self, name);
		Dee_Decref(name);
	}
err:
	return NULL;
}

/* Same as `FrameSymbolsByName_XchCLidValue()', but automatically
 * inherits the reference to the old value.
 * @param: value: The new value, or NULL to unbound the location.
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
FrameSymbolsByName_SetCLidValue(FrameSymbolsByName *self,
                                canonical_lid_t clid,
                                /*0..1*/ DeeObject *value) {
	DREF DeeObject *oldval;
	oldval = FrameSymbolsByName_XchCLidValue(self, clid, value);
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


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbynameiter_nextpair(FrameSymbolsByNameIterator *__restrict self,
                                DREF DeeObject *key_and_value[2]) {
	FrameSymbolsByNameIteratorIndex oldidx, newidx;
again:
	framesymbolsbynameiter_get_idx(self, &oldidx);
	memcpy(&newidx, &oldidx, sizeof(newidx));
	for (;;) {
		canonical_lid_t clid;
		clid             = FrameSymbolsByName_Idx2CLid(self->frsbni_seq, &newidx);
		key_and_value[1] = FrameSymbolsByName_GetCLidValue(self->frsbni_seq, clid);
		if (ITER_ISOK(key_and_value[1])) {
			key_and_value[0] = FrameSymbolsByName_GetCLidName(self->frsbni_seq, clid);
			if unlikely(!key_and_value[0])
				goto err_value;
			FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx);
			break;
		}
		if unlikely(!key_and_value[1])
			goto err;
		if (!FrameSymbolsByName_IdxInc(self->frsbni_seq, &newidx))
			return 1;
	}
	if (!framesymbolsbynameiter_cmpxch_idx(self, &oldidx, &newidx)) {
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		goto again;
	}
	return 0;
err_value:
	Dee_Decref(key_and_value[1]);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbynameiter_nextkey_with_unbound(FrameSymbolsByNameIterator *__restrict self) {
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
framesymbolsbynameiter_nextvalue(FrameSymbolsByNameIterator *__restrict self) {
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

PRIVATE struct type_iterator framesymbolsbynameiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&framesymbolsbynameiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_nextvalue,
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextvalue),
};

PRIVATE struct type_nii tpconst framesymbolsbynameiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&framesymbolsbynameiter_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&framesymbolsbynameiter_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&framesymbolsbynameiter_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&framesymbolsbynameiter_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)NULL,
			/* .nii_advance  = */ (Dee_funptr_t)NULL,
			/* .nii_prev     = */ (Dee_funptr_t)NULL,
			/* .nii_next     = */ (Dee_funptr_t)NULL,
			/* .nii_hasprev  = */ (Dee_funptr_t)NULL,
			/* .nii_peek     = */ (Dee_funptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp framesymbolsbynameiter_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&framesymbolsbynameiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &framesymbolsbynameiter_nii,
};

PRIVATE struct type_getset tpconst framesymbolsbynameiter_getsets[] = {
	TYPE_GETTER_AB("__frame__", &framesymbolsbynameiter_get_frame, "->?Ert:Frame"),
	TYPE_GETTER_AB("__func__", &framesymbolsbynameiter_get_func, "->?DFunction"),
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
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&framesymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&framesymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FrameSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&framesymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &framesymbolsbynameiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &framesymbolsbynameiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ framesymbolsbynameiter_getsets,
	/* .tp_members       = */ framesymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject FrameSymbolsByNameKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FrameSymbolsByNameKeysIterator",
	/* .tp_doc      = */ DOC("next->?X2?Dstring?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&framesymbolsbynameiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&framesymbolsbynameiter_init,
				TYPE_FIXED_ALLOCATOR(FrameSymbolsByNameIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&framesymbolsbynameiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &framesymbolsbynameiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&framesymbolsbynameiter_nextkey_with_unbound,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ framesymbolsbynameiter_getsets,
	/* .tp_members       = */ framesymbolsbynameiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



/* Try to convert "name" into its "canonical location id" within "self"
 * @return: * : The canonical location ID of `name'
 * @return: (canonical_lid_t)-1: An error was thrown.
 * @return: (canonical_lid_t)-2: No symbol exists matching `name', or insufficient debug info */
PRIVATE WUNUSED NONNULL((1, 2)) canonical_lid_t DCALL
FrameSymbolsByName_TryName2LocId(FrameSymbolsByName *self,
                                 char const *name, size_t len) {
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
			if (strcmpz(DeeString_STR(arg_name), name, len) == 0)
				return i;
		}
	}
	result = code->co_argc_max;

	/* Check for names of references/statics. */
	i = DDI_GetRefIdByName(code->co_ddi, name, len);
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
				if (local_name && strcmpz(local_name, name, len) == 0) {
					DeeFrame_LockEndRead((DeeObject *)self->frsbn_frame);
					Dee_ddi_state_fini(&dds);
					return result + i;
				}
			}
			for (i = 0; i < sp_count; ++i) {
				char const *stack_name;
				stack_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_spnamv[i]);
				if (stack_name && strcmpz(stack_name, name, len) == 0) {
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

PRIVATE WUNUSED NONNULL((1)) canonical_lid_t DCALL
FrameSymbolsByName_TryVerifyLocId(FrameSymbolsByName *self, size_t ulid) {
	/* Check that "lid" can appear in "self" */
	canonical_lid_t size;
	DeeCodeObject *code;
	code = self->frsbn_func->fo_code;
	size = code->co_argc_max;
	if (ulid < (size_t)size) {
		uint16_t aid = (uint16_t)ulid;
		if (aid >= self->frsbn_nargs)
			goto err_bad_lid;
	} else if ((size += code->co_refstaticc, ulid < (size_t)size)) {
		uint16_t rid = (uint16_t)(ulid - code->co_argc_max);
		if (rid < self->frsbn_rid_start)
			goto err_bad_lid;
		if (rid >= self->frsbn_rid_end)
			goto err_bad_lid;
	} else if ((size += code->co_localc, ulid < (size_t)size)) {
		uint16_t lid = (uint16_t)(ulid - (code->co_argc_max + code->co_refstaticc));
		if (lid >= self->frsbn_localc)
			goto err_bad_lid;
	} else if (ulid >= (size_t)(size + self->frsbn_stackc)) {
		goto err_bad_lid;
	}
	return (canonical_lid_t)ulid;
err_bad_lid:
	return (canonical_lid_t)-2;
}

PRIVATE WUNUSED NONNULL((1)) canonical_lid_t DCALL
FrameSymbolsByName_VerifyLocId(FrameSymbolsByName *self, size_t lid) {
	canonical_lid_t result;
	result = FrameSymbolsByName_TryVerifyLocId(self, lid);
	if unlikely(result == (canonical_lid_t)-2) {
		result = (canonical_lid_t)DeeError_Throwf(&DeeError_KeyError,
		                                          "Invalid location id %#" PRFxSIZ,
		                                          lid);
	}
	return result;
}

/* Same as `FrameSymbolsByName_TryName2LocId()', but handles a generic object `key'
 * @return: * : The canonical location ID of `key'
 * @return: (canonical_lid_t)-1: An error was thrown.
 * @return: (canonical_lid_t)-2: No symbol exists matching `key', or insufficient debug info */
PRIVATE WUNUSED NONNULL((1, 2)) canonical_lid_t DCALL
FrameSymbolsByName_TryKey2LocId(FrameSymbolsByName *self, DeeObject *key) {
	canonical_lid_t result;
	if (DeeString_Check(key)) {
		result = FrameSymbolsByName_TryName2LocId(self,
		                                          DeeString_STR(key),
		                                          DeeString_SIZE(key));
	} else {
		if (DeeObject_AsUIntX(key, &result))
			goto err_maybe_overflow;
		result = FrameSymbolsByName_TryVerifyLocId(self, result);
	}
	return result;
err_maybe_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		return (canonical_lid_t)-2;
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
	return (canonical_lid_t)DeeRT_ErrUnknownKey(self, key);
}



PRIVATE WUNUSED NONNULL((1)) size_t DCALL
framesymbolsbyname_size(FrameSymbolsByName *__restrict self) {
	size_t result;
	result = self->frsbn_nargs;
	result += self->frsbn_rid_end - self->frsbn_rid_start;
	result += self->frsbn_localc;
	result += self->frsbn_stackc;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
framesymbolsbyname_mh_setold_ex(FrameSymbolsByName *self,
                                DeeObject *key, DeeObject *value) {
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
			return oldvalue;
		}
		DeeFunction_RefLockEndWrite(self->frsbn_func);
		return ITER_DONE;
	} else if ((oldvalue = *loc.cll_ptr) != NULL) {
		if unlikely(!loc.cll_writable) {
			DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
			DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
			goto err;
		}
		Dee_Incref(value);
		*loc.cll_ptr = value;
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return oldvalue;
	} else {
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return ITER_DONE;
	}
	__builtin_unreachable();
err_no_such_key:
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
framesymbolsbyname_mh_setnew_ex(FrameSymbolsByName *self,
                                DeeObject *key, DeeObject *value) {
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
			Dee_Incref(oldvalue);
			DeeFunction_RefLockEndWrite(self->frsbn_func);
			return oldvalue;
		}
		Dee_Incref(value);
		*loc.cll_ptr = value;
		DeeFunction_RefLockEndWrite(self->frsbn_func);
		DeeFutex_WakeAll(loc.cll_ptr);
		return ITER_DONE;
	} else if (*loc.cll_ptr) {
		oldvalue = *loc.cll_ptr;
		Dee_Incref(oldvalue);
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return oldvalue;
	} else {
		if unlikely(!loc.cll_writable) {
			DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
			DeeRT_ErrReadOnlyKey((DeeObject *)self, key);
			goto err;
		}
		Dee_Incref(value);
		*loc.cll_ptr = value;
		DeeFrame_LockEndWrite((DeeObject *)self->frsbn_frame);
		return ITER_DONE;
	}
	__builtin_unreachable();
err_no_such_key:
	DeeRT_ErrUnknownKey(self, key);
err:
	return NULL;
}

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

PRIVATE WUNUSED NONNULL((1)) DREF FrameSymbolsByNameIterator *DCALL
framesymbolsbyname_mh_map_keysiter(FrameSymbolsByName *__restrict self) {
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
	DeeObject_Init(result, &FrameSymbolsByNameKeysIterator_Type);
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
framesymbolsbyname_getitem(FrameSymbolsByName *self, DeeObject *key) {
	DREF DeeObject *result;
	canonical_lid_t clid = FrameSymbolsByName_Key2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	result = FrameSymbolsByName_GetCLidValue(self, clid);
	if (result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	DeeRT_ErrUnboundKey((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbyname_trygetitem(FrameSymbolsByName *self, DeeObject *key) {
	canonical_lid_t clid = FrameSymbolsByName_TryKey2LocId(self, key);
	if (clid != (canonical_lid_t)-2) {
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		return FrameSymbolsByName_GetCLidValue(self, clid);
	}
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_bounditem(FrameSymbolsByName *self, DeeObject *key) {
	canonical_lid_t clid = FrameSymbolsByName_TryKey2LocId(self, key);
	if (clid != (canonical_lid_t)-2) {
		DREF DeeObject *result;
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		result = FrameSymbolsByName_GetCLidValue(self, clid);
		if unlikely(!result)
			goto err;
		if (result == ITER_DONE)
			return Dee_BOUND_NO;
		Dee_Decref(result);
		return Dee_BOUND_YES;
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_hasitem(FrameSymbolsByName *self, DeeObject *key) {
	canonical_lid_t clid = FrameSymbolsByName_TryKey2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return clid != (canonical_lid_t)-2 ? 1 : 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2 /*, 3*/)) int DCALL
framesymbolsbyname_setitem(FrameSymbolsByName *self,
                           DeeObject *key, DeeObject *value) {
	canonical_lid_t clid = FrameSymbolsByName_Key2LocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return FrameSymbolsByName_SetCLidValue(self, clid, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_delitem(FrameSymbolsByName *self, DeeObject *key) {
	return framesymbolsbyname_setitem(self, key, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbyname_getitem_index(FrameSymbolsByName *self, size_t key) {
	DREF DeeObject *result;
	canonical_lid_t clid = FrameSymbolsByName_VerifyLocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	result = FrameSymbolsByName_GetCLidValue(self, clid);
	if (result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	DeeRT_ErrUnboundKeyInt((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbyname_trygetitem_index(FrameSymbolsByName *self, size_t key) {
	canonical_lid_t clid = FrameSymbolsByName_TryVerifyLocId(self, key);
	if (clid != (canonical_lid_t)-2) {
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		return FrameSymbolsByName_GetCLidValue(self, clid);
	}
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbyname_bounditem_index(FrameSymbolsByName *self, size_t key) {
	canonical_lid_t clid = FrameSymbolsByName_TryVerifyLocId(self, key);
	if (clid != (canonical_lid_t)-2) {
		DREF DeeObject *result;
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		result = FrameSymbolsByName_GetCLidValue(self, clid);
		if unlikely(!result)
			goto err;
		if (result == ITER_DONE)
			return Dee_BOUND_NO; /* Unbound */
		Dee_Decref(result);
		return Dee_BOUND_YES; /* Bound */
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbyname_hasitem_index(FrameSymbolsByName *self, size_t key) {
	canonical_lid_t clid = FrameSymbolsByName_TryVerifyLocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return clid != (canonical_lid_t)-2 ? 1 : 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1 /*, 3*/)) int DCALL
framesymbolsbyname_setitem_index(FrameSymbolsByName *self, size_t key, DeeObject *value) {
	canonical_lid_t clid = FrameSymbolsByName_VerifyLocId(self, key);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return FrameSymbolsByName_SetCLidValue(self, clid, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
framesymbolsbyname_delitem_index(FrameSymbolsByName *self, size_t key) {
	return framesymbolsbyname_setitem_index(self, key, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
framesymbolsbyname_getitem_string_len_hash(FrameSymbolsByName *self,
                                           char const *key, size_t keylen,
                                           Dee_hash_t UNUSED(hash)) {
	DREF DeeObject *result;
	canonical_lid_t clid = FrameSymbolsByName_TryName2LocId(self, key, keylen);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	result = FrameSymbolsByName_GetCLidValue(self, clid);
	if (result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	DeeRT_ErrUnboundKeyStr((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
framesymbolsbyname_trygetitem_string_len_hash(FrameSymbolsByName *self,
                                              char const *key, size_t keylen,
                                              Dee_hash_t UNUSED(hash)) {
	canonical_lid_t clid = FrameSymbolsByName_TryName2LocId(self, key, keylen);
	if (clid != (canonical_lid_t)-2) {
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		return FrameSymbolsByName_GetCLidValue(self, clid);
	}
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_bounditem_string_len_hash(FrameSymbolsByName *self,
                                             char const *key, size_t keylen,
                                             Dee_hash_t UNUSED(hash)) {
	canonical_lid_t clid = FrameSymbolsByName_TryName2LocId(self, key, keylen);
	if (clid != (canonical_lid_t)-2) {
		DREF DeeObject *result;
		if unlikely(clid == (canonical_lid_t)-1)
			goto err;
		result = FrameSymbolsByName_GetCLidValue(self, clid);
		if unlikely(!result)
			goto err;
		if (result == ITER_DONE)
			return Dee_BOUND_NO; /* Unbound */
		Dee_Decref(result);
		return Dee_BOUND_YES; /* Bound */
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_hasitem_string_len_hash(FrameSymbolsByName *self,
                                           char const *key, size_t keylen,
                                           Dee_hash_t UNUSED(hash)) {
	canonical_lid_t clid = FrameSymbolsByName_TryName2LocId(self, key, keylen);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return clid != (canonical_lid_t)-2 ? 1 : 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2 /*, 5*/)) int DCALL
framesymbolsbyname_setitem_string_len_hash(FrameSymbolsByName *self,
                                           char const *key, size_t keylen,
                                           Dee_hash_t UNUSED(hash), DeeObject *value) {
	canonical_lid_t clid = FrameSymbolsByName_TryName2LocId(self, key, keylen);
	if unlikely(clid == (canonical_lid_t)-1)
		goto err;
	return FrameSymbolsByName_SetCLidValue(self, clid, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
framesymbolsbyname_delitem_string_len_hash(FrameSymbolsByName *self,
                                           char const *key, size_t keylen,
                                           Dee_hash_t hash) {
	return framesymbolsbyname_setitem_string_len_hash(self, key, keylen, hash, NULL);
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
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("_FrameSymbolsByName", params: "
	DeeFrameObject *frame;
	uint16_t        argc = (uint16_t)-1;
	uint16_t        ridstart = 0;
	uint16_t        ridend = (uint16_t)-1;
	uint16_t        localc = (uint16_t)-1;
	uint16_t        stackc = (uint16_t)-1;
");]]]*/
	struct {
		DeeFrameObject *frame;
		uint16_t argc;
		uint16_t ridstart;
		uint16_t ridend;
		uint16_t localc;
		uint16_t stackc;
	} args;
	args.argc = (uint16_t)-1;
	args.ridstart = 0;
	args.ridend = (uint16_t)-1;
	args.localc = (uint16_t)-1;
	args.stackc = (uint16_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__frame_argc_ridstart_ridend_localc_stackc, "o|" UNPx16 UNPu16 UNPx16 UNPx16 UNPx16 ":_FrameSymbolsByName", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.frame, &DeeFrame_Type))
		goto err;
	frame = DeeFrame_LockRead((DeeObject *)args.frame);
	if unlikely(!frame)
		goto err;
	self->frsbn_func = frame->cf_func;
	Dee_Incref(self->frsbn_func);
	if (args.stackc > Dee_code_frame_getspaddr(frame))
		args.stackc = Dee_code_frame_getspaddr(frame);
	DeeFrame_LockEndRead((DeeObject *)args.frame);
	code = self->frsbn_func->fo_code;
	if (args.localc > code->co_localc)
		args.localc = code->co_localc;
	if (args.argc > code->co_argc_max)
		args.argc = code->co_argc_max;
	if (args.ridend > code->co_refstaticc)
		args.ridend = code->co_refstaticc;
	if (args.ridstart > args.ridend)
		args.ridstart = args.ridend;
	self->frsbn_frame     = args.frame;
	self->frsbn_nargs     = args.argc;
	self->frsbn_rid_start = args.ridstart;
	self->frsbn_rid_end   = args.ridend;
	self->frsbn_localc    = args.localc;
	self->frsbn_stackc    = args.stackc;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(FrameSymbolsByName, frsbn_frame) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(FrameSymbolsByName, frsbn_frame) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(FrameSymbolsByName, frsbn_func) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(FrameSymbolsByName, frsbn_func) == offsetof(ProxyObject2, po_obj2));
#define framesymbolsbyname_fini  generic_proxy2__fini
#define framesymbolsbyname_visit generic_proxy2__visit


PRIVATE struct type_seq framesymbolsbyname_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&framesymbolsbyname_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&framesymbolsbyname_setitem,
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&framesymbolsbyname_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&framesymbolsbyname_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&framesymbolsbyname_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&framesymbolsbyname_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&framesymbolsbyname_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&framesymbolsbyname_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&framesymbolsbyname_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&framesymbolsbyname_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&framesymbolsbyname_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem_string_len_hash), /* DEFAULT! */
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem_string_len_hash), /* DEFAULT! */
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem_string_len_hash), /* DEFAULT! */
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem_string_len_hash), /* DEFAULT! */
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem_string_len_hash), /* DEFAULT! */
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem_string_len_hash), /* DEFAULT! */
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&framesymbolsbyname_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&framesymbolsbyname_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&framesymbolsbyname_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&framesymbolsbyname_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&framesymbolsbyname_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&framesymbolsbyname_hasitem_string_len_hash,
};

PRIVATE struct type_method tpconst framesymbolsbyname_methods[] = {
	TYPE_METHOD_HINTREF(Mapping_setold_ex),
	TYPE_METHOD_HINTREF(Mapping_setnew_ex),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst framesymbolsbyname_getsets[] = {
	TYPE_GETTER_AB(STR_iterkeys, &framesymbolsbyname_mh_map_keysiter, "->?#KeysIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_method_hint tpconst framesymbolsbyname_method_hints[] = {
	TYPE_METHOD_HINT(map_setold_ex, &framesymbolsbyname_mh_setold_ex),
	TYPE_METHOD_HINT(map_setnew_ex, &framesymbolsbyname_mh_setnew_ex),
	// TODO: TYPE_METHOD_HINT(map_enumerate, &framesymbolsbyname_enumerate),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst framesymbolsbyname_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &FrameSymbolsByNameIterator_Type),
	TYPE_MEMBER_CONST(STR_KeysIterator, &FrameSymbolsByNameKeysIterator_Type),
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
	                         "(frame:?Ert:Frame,argc?:?Dint,ridstart=!0,ridend?:?Dint,localc?:?Dint,stackc?:?Dint)\n"
	                         "\n"
	                         "[](key:?X2?Dstring?Dint)->"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&framesymbolsbyname_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(FrameSymbolsByName),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&framesymbolsbyname_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&framesymbolsbyname_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&framesymbolsbyname_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E5A99B058858326C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &framesymbolsbyname_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ framesymbolsbyname_methods,
	/* .tp_getsets       = */ framesymbolsbyname_getsets,
	/* .tp_members       = */ framesymbolsbyname_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ framesymbolsbyname_class_members,
	/* .tp_method_hints  = */ framesymbolsbyname_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
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
	return Dee_AsObject(result);
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
	return Dee_AsObject(result);
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
	return Dee_AsObject(result);
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
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_WRAPPERS_C */
