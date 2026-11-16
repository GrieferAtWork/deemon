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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_REPEAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_REPEAT_C 1

#include <deemon/api.h>

#include <deemon/accu.h>               /* Dee_accu, Dee_accu_* */
#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1, DeeArg_UnpackStruct2X, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>               /* Dee_True, return_false */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err*, DeeRT_IntegerOverflow_TryCatchSizeOverflow */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Printf, PRFuSIZ */
#include <deemon/gc.h>                 /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/method-hints.h>       /* DeeMH_*_t, DeeObject_InvokeMethodHint, DeeObject_RequireMethodHint, DeeType_RequireMethodHint, Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* DeeNone_NewRef, Dee_None */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_ERR, Dee_BOUND_FROMBOOL, Dee_COMPARE_*, Dee_Decref*, Dee_HAS_*, Dee_Incref, Dee_Setrefv, Dee_TYPE, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compareT, Dee_return_compare_if_ne, Dee_ssize_t, ITER_DONE, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeNO_foreach_t */
#include <deemon/pair.h>               /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#include <deemon/seq.h>                /* DeeIterator_NewEmpty, DeeIterator_Type, DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_*, Dee_EmptySeq, Dee_seq_range */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/thread.h>             /* DeeThread_CheckInterrupt */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */
#include <deemon/util/hash.h>          /* DeeObject_HashGeneric, Dee_HashCombine */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */

#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */
#include <hybrid/overflow.h>  /* OVERFLOW_UADD, OVERFLOW_UMUL */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "one.h"
#include "repeat.h"

#include <stddef.h> /* NULL, offsetof, size_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

/* Simulate the result of `obj [+ obj [+ obj [+ obj [+ obj [+ ...]]]]]`
 * Repeated `count` times. In order to keep this executing in a reasonable
 * amount of time, we always just do `obj * int(count)`, even though not
 * every object necessarily implements `operator *` in the mathematical
 * sense, this works correctly for: `Sequence`, `Bytes`, `string`, `int`,
 * `float`, `bool` */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_object_repeat_sum(DeeObject *obj, size_t count) {
	DREF DeeObject *result;
	DREF DeeObject *multiplier;
	multiplier = DeeInt_NewSize(count);
	if unlikely(!multiplier)
		goto err;
	result = DeeObject_Mul(obj, multiplier);
	Dee_Decref(multiplier);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_object_repeat_sum_inherited(/*inherit(always)*/ DREF DeeObject *obj, size_t count) {
	DREF DeeObject *result = generic_object_repeat_sum(obj, count);
	Dee_Decref(obj);
	return result;
}


#define REPEATITER_READ_NUM(x) atomic_read(&(x)->rpi_num)

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatiter_ctor(RepeatIterator *__restrict self) {
	self->rpi_rep = (DREF Repeat *)DeeSeq_Repeat(Dee_EmptySeq, 1);
	if unlikely(!self->rpi_rep)
		goto err;
	self->rpi_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->rpi_iter) { Dee_Decref(self->rpi_rep); }
	Dee_atomic_rwlock_init(&self->rpi_lock);
	self->rpi_num = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_copy(RepeatIterator *__restrict self,
                RepeatIterator *__restrict other) {
	DREF DeeObject *copy;
	RepeatIterator_LockRead(other);
	copy = other->rpi_iter;
	Dee_Incref(copy);
	self->rpi_num = other->rpi_num;
	RepeatIterator_LockEndRead(other);
	if unlikely((copy = DeeObject_CopyInherited(copy)) == NULL)
		goto err;
	self->rpi_iter = copy;
	self->rpi_rep  = other->rpi_rep;
	Dee_Incref(self->rpi_rep);
	Dee_atomic_rwlock_init(&self->rpi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatiter_init(RepeatIterator *__restrict self,
                size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_SeqRepeatIterator", params: """
	Repeat *repeat:?Ert:SeqRepeat
""", docStringPrefix: "repeatiter");]]]*/
#define repeatiter__SeqRepeatIterator_params "repeat:?Ert:SeqRepeat"
	struct {
		Repeat *repeat;
	} args;
	DeeArg_Unpack1(err, argc, argv, "_SeqRepeatIterator", &args.repeat);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.repeat, &SeqRepeat_Type))
		goto err;
	self->rpi_iter = DeeObject_Iter(args.repeat->rp_seq);
	if unlikely(!self->rpi_iter)
		goto err;
	Dee_Incref(args.repeat);
	Dee_atomic_rwlock_init(&self->rpi_lock);
	self->rpi_rep = args.repeat;
	self->rpi_num = args.repeat->rp_num - 1;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_serialize(RepeatIterator *__restrict self,
                     DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(RepeatIterator, field))
	DREF DeeObject *self__rpi_iter;
	RepeatIterator *out;
	size_t self__rpi_num;
	RepeatIterator_LockRead(self);
	self__rpi_iter = self->rpi_iter;
	Dee_Incref(self__rpi_iter);
	self__rpi_num = self->rpi_num;
	RepeatIterator_LockEndRead(self);
	out = DeeSerial_Addr2Mem(writer, addr, RepeatIterator);
	out->rpi_num = self__rpi_num;
	Dee_atomic_rwlock_init(&out->rpi_lock);
	if (DeeSerial_PutObjectInherited(writer, ADDROF(rpi_iter), self__rpi_iter))
		goto err;
	return DeeSerial_PutObject(writer, ADDROF(rpi_rep), self->rpi_rep);
err:
	return -1;
#undef ADDROF
}

STATIC_ASSERT(offsetof(RepeatIterator, rpi_rep) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(RepeatIterator, rpi_rep) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(RepeatIterator, rpi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(RepeatIterator, rpi_iter) == offsetof(ProxyObject2, po_obj2));
#define repeatiter_fini  generic_proxy2__fini
#define repeatiter_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatiter_print(RepeatIterator *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	size_t remaining_iterations;
	RepeatIterator_LockWrite(self);
	iter = self->rpi_iter;
	Dee_Incref(iter);
	remaining_iterations = self->rpi_num;
	RepeatIterator_LockEndWrite(self);
	result = DeeFormat_Printf(printer, arg,
	                          "<Iterator reading from %k, then repeating %k another %" PRFuSIZ " times>",
	                          iter, self->rpi_rep->rp_seq,
	                          remaining_iterations);
	Dee_Decref(iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatiter_printrepr(RepeatIterator *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	DREF DeeObject *raw_iter;
	int iter_eq_raw_iter;
	size_t remaining_iterations;
	RepeatIterator_LockWrite(self);
	iter = self->rpi_iter;
	Dee_Incref(iter);
	remaining_iterations = self->rpi_num;
	RepeatIterator_LockEndWrite(self);
	raw_iter = DeeObject_Iter(self->rpi_rep->rp_seq);
	if unlikely(!raw_iter)
		goto err_iter;
	if (Dee_TYPE(iter) != Dee_TYPE(raw_iter)) {
		iter_eq_raw_iter = Dee_COMPARE_NE;
	} else {
		iter_eq_raw_iter = DeeObject_CompareEq(iter, raw_iter);
	}
	Dee_Decref(raw_iter);
	if (Dee_COMPARE_ISERR(iter_eq_raw_iter)) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError)) {
			iter_eq_raw_iter = Dee_COMPARE_NE;
		} else {
			goto err_iter;
		}
	}
	result = DeeFormat_Printf(printer, arg,
	                          "Sequence.repeat(%r, %" PRFuSIZ ").operator iter()",
	                          self->rpi_rep->rp_seq, remaining_iterations);
	if (result >= 0 && Dee_COMPARE_ISNE(iter_eq_raw_iter)) {
		/* If "iter" isn't the equal to the start of a
		 * repetition, then include its representation */
		Dee_ssize_t temp;
		temp = DeeFormat_Printf(printer, arg, " <curIter:%r>", iter);
		result = unlikely(temp < 0) ? temp : (result + temp);
	}
	Dee_Decref(iter);
	return result;
err_iter:
	Dee_Decref(iter);
	return -1;
}


PRIVATE NONNULL((1)) void DCALL
repeatiter_clear(RepeatIterator *__restrict self) {
	DREF DeeObject *iter;
	RepeatIterator_LockWrite(self);
	iter = self->rpi_iter; /* Inherit reference */
	self->rpi_iter = DeeNone_NewRef();
	RepeatIterator_LockEndWrite(self);
	Dee_Decref(iter);
}

PRIVATE struct type_gc tpconst repeatiter_gc = {
	/* .tp_gc = */ (void (DCALL *)(DeeObject *__restrict))&repeatiter_clear
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
repeatiter_hash(RepeatIterator *__restrict self) {
	DREF DeeObject *my_iter;
	Dee_hash_t result = REPEATITER_READ_NUM(self);
	RepeatIterator_LockRead(self);
	my_iter = self->rpi_iter;
	Dee_Incref(my_iter);
	RepeatIterator_LockEndRead(self);
	return Dee_HashCombine(result, DeeObject_HashInherited(my_iter));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_compare(RepeatIterator *self, RepeatIterator *other) {
	int result;
	size_t my_len, ot_len;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, &SeqRepeatIterator_Type))
		goto err;
	my_len = REPEATITER_READ_NUM(self);
	ot_len = REPEATITER_READ_NUM(other);
	/* Compare "num" in reverse (because "num" becomes less as the iterator is advanced) */
	Dee_return_compare_if_ne(ot_len, my_len);
	RepeatIterator_LockRead(self);
	my_iter = self->rpi_iter;
	Dee_Incref(my_iter);
	RepeatIterator_LockEndRead(self);
	RepeatIterator_LockRead(other);
	ot_iter = other->rpi_iter;
	Dee_Incref(ot_iter);
	RepeatIterator_LockEndRead(other);
	result = DeeObject_Compare(my_iter, ot_iter);
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_compare_eq(RepeatIterator *self, RepeatIterator *other) {
	int result;
	size_t my_len, ot_len;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, &SeqRepeatIterator_Type))
		goto err;
	my_len = REPEATITER_READ_NUM(self);
	ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return Dee_COMPARE_NE;
	RepeatIterator_LockRead(self);
	my_iter = self->rpi_iter;
	Dee_Incref(my_iter);
	RepeatIterator_LockEndRead(self);
	RepeatIterator_LockRead(other);
	ot_iter = other->rpi_iter;
	Dee_Incref(ot_iter);
	RepeatIterator_LockEndRead(other);
	result = DeeObject_CompareEq(my_iter, ot_iter);
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp repeatiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&repeatiter_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeatiter_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeatiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_next(RepeatIterator *__restrict self) {
	DREF DeeObject *result, *iter;
	RepeatIterator_LockRead(self);
	iter = self->rpi_iter;
	Dee_Incref(iter);
	RepeatIterator_LockEndRead(self);
again_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (result != ITER_DONE) {
done:
		return result;
	}
#ifndef __OPTIMIZE_SIZE__
	if (!REPEATITER_READ_NUM(self))
		goto done;
#endif /* !__OPTIMIZE_SIZE__ */
	/* Create a new iterator for the next loop. */
	iter = DeeObject_Iter(self->rpi_rep->rp_seq);
	if unlikely(!iter)
		goto err;
	COMPILER_READ_BARRIER();
	RepeatIterator_LockWrite(self);
	if unlikely(!self->rpi_num) {
		/* Race condition: iterator was exhausted while we created a new sub-iterator */
		RepeatIterator_LockEndWrite(self);
		Dee_Decref(iter);
		ASSERT(result == ITER_DONE);
		goto done;
	}
	result = self->rpi_iter;
	Dee_Incref(iter);
	self->rpi_iter = iter;
	--self->rpi_num; /* Decrement the number of remaining loops. */
	RepeatIterator_LockEndWrite(self);
	Dee_Decref(result); /* Drop the old iterator. */
	goto again_iter;    /* Read more items. */
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_get_iter(RepeatIterator *__restrict self) {
	DREF DeeObject *result;
	RepeatIterator_LockRead(self);
	result = self->rpi_iter;
	Dee_Incref(result);
	RepeatIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_set_iter(RepeatIterator *self, DeeObject *value) {
	DREF DeeObject *oldvalue;
	Dee_Incref(value);
	RepeatIterator_LockWrite(self);
	oldvalue = self->rpi_iter; /* Inherit reference */
	self->rpi_iter = value;    /* Inherit reference */
	RepeatIterator_LockEndWrite(self);
	Dee_Decref(oldvalue);
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_get_num(RepeatIterator *__restrict self) {
	size_t result;
	RepeatIterator_LockRead(self);
	result = self->rpi_num;
	RepeatIterator_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_set_num(RepeatIterator *__restrict self,
                   DeeObject *__restrict value) {
	size_t newvalue;
	if unlikely(DeeObject_AsSize(value, &newvalue))
		goto err;
	RepeatIterator_LockWrite(self);
	self->rpi_num = newvalue;
	RepeatIterator_LockEndWrite(self);
	return 0;
err:
	return -1;
}


PRIVATE struct type_getset tpconst repeatiter_getsets[] = {
	TYPE_GETSET_F("__iter__", &repeatiter_get_iter, NULL, &repeatiter_set_iter, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETSET_F("__num__", &repeatiter_get_num, NULL, &repeatiter_set_num, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst repeatiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(RepeatIterator, rpi_rep), "->?Ert:SeqRepeat"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqRepeatIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatIterator",
	/* .tp_doc      = */ DOC("(" repeatiter__SeqRepeatIterator_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ RepeatIterator,
			/* tp_ctor:        */ &repeatiter_ctor,
			/* tp_copy_ctor:   */ &repeatiter_copy,
			/* tp_any_ctor:    */ &repeatiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &repeatiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeatiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__iter_operator_bool__with__iter_peek),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeatiter_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeatiter_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&repeatiter_visit,
	/* .tp_gc            = */ &repeatiter_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &repeatiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ repeatiter_getsets,
	/* .tp_members       = */ repeatiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_ctor(Repeat *__restrict self) {
	self->rp_num = 1;
	self->rp_seq = DeeSeq_NewEmpty();
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_copy(Repeat *__restrict self,
            Repeat *__restrict other) {
	self->rp_num = other->rp_num;
	self->rp_seq = other->rp_seq;
	Dee_Incref(self->rp_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_init(Repeat *__restrict self,
            size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_SeqRepeat", params: """
	seq:?DSequence,
	size_t num
""", docStringPrefix: "repeat");]]]*/
#define repeat__SeqRepeat_params "seq:?DSequence,num:?Dint"
	struct {
		DeeObject *seq;
		size_t num;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "_SeqRepeat", &args, &args.seq, "o", _DeeArg_AsObject, &args.num, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	self->rp_seq = args.seq;
	self->rp_num = args.num;
	if unlikely(!self->rp_num) {
		self->rp_seq = Dee_EmptySeq;
		self->rp_num = 1;
	}
	Dee_Incref(self->rp_seq);
	return 0;
err:
	return -1;
}


STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_serialize generic_proxy__serialize_and_memcpy
#define repeat_fini      generic_proxy__fini
#define repeat_visit     generic_proxy__visit
#define repeat_bool      generic_proxy__bool

PRIVATE WUNUSED NONNULL((1)) DREF RepeatIterator *DCALL
repeat_iter(Repeat *__restrict self) {
	DREF RepeatIterator *result;
	result = DeeGCObject_MALLOC(RepeatIterator);
	if unlikely(!result)
		goto err;
	result->rpi_iter = DeeObject_Iter(self->rp_seq);
	if unlikely(!result->rpi_iter)
		goto err_r;
	result->rpi_rep = self;
	result->rpi_num = self->rp_num - 1;
	Dee_atomic_rwlock_init(&result->rpi_lock);
	Dee_Incref(self);
	DeeObject_InitStatic(result, &SeqRepeatIterator_Type);
	return DeeGC_TRACK(RepeatIterator, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_operator_contains generic_proxy__seq_operator_contains

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_size(Repeat *__restrict self) {
	size_t result;
	size_t base_size = DeeObject_Size(self->rp_seq);
	if unlikely(base_size == (size_t)-1)
		return (size_t)-1;
	ASSERTF(self->rp_num != 0, "Prevented during construction");
	if (OVERFLOW_UMUL(base_size, self->rp_num, &result)) {
		DeeRT_ErrIntegerOverflowUMul(base_size, self->rp_num);
		return (size_t)-1;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_sizeob(Repeat *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *base_size;
	DREF DeeObject *multiplier;
	ASSERTF(self->rp_num != 0, "Prevented during construction");
	base_size = DeeObject_SizeOb(self->rp_seq);
	if unlikely(!base_size)
		goto err;
	multiplier = DeeInt_NewSize(self->rp_num);
	if unlikely(!multiplier)
		goto err_base_size;
	result = DeeObject_Mul(base_size, multiplier);
	Dee_Decref(multiplier);
	Dee_Decref(base_size);
	return result;
err_base_size:
	Dee_Decref(base_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_size_fast(Repeat *__restrict self) {
	size_t result = DeeObject_SizeFast(self->rp_seq);
#if 0 /* Not needed -> implicitly done by OVERFLOW_UMUL() below */
	if (result == (size_t)-1)
		return (size_t)-1;
#endif
	if (OVERFLOW_UMUL(result, self->rp_num, &result))
		result = (size_t)-1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_translate_index(Repeat *self, size_t index) {
	size_t seq_size, whole_size, result;
	ASSERTF(self->rp_num != 0, "Prevented during construction");
	if unlikely(index >= (size_t)-1)
		return (size_t)DeeRT_ErrIntegerOverflowU(index, (size_t)-2);
	seq_size = DeeObject_Size(self->rp_seq);
	if unlikely(seq_size == (size_t)-1) {
		DREF DeeObject *actual_size = DeeRT_IntegerOverflow_TryCatchSizeOverflow();
		if (!actual_size)
			goto err;
		Dee_Decref(actual_size);
		/* Sequence is >= SIZE_MAX; `index < SIZE_MAX` -> caller's `index` always maps 1-on-1 */
		return index;
	}
	if (OVERFLOW_UMUL(seq_size, self->rp_num, &whole_size)) {
		/* Sequence is larger than SIZE_MAX -> can't be out-of-bounds */
	} else {
		if unlikely(index >= whole_size) {
			DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self),
			                          index, whole_size);
			goto err;
		}
	}
	ASSERT(seq_size != 0);
	result = index % seq_size;
	ASSERT(result != (size_t)-1);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_translate_index_ob(Repeat *self, DeeObject *index_ob) {
	DREF DeeObject *base_size;
	DREF DeeObject *multiplier;
	DREF DeeObject *whole_size;
	DREF DeeObject *base_index;
	int index_ge_whole_size;
	ASSERTF(self->rp_num != 0, "Prevented during construction");
	base_size = DeeObject_SizeOb(self->rp_seq);
	if unlikely(!base_size)
		goto err;
	multiplier = DeeInt_NewSize(self->rp_num);
	if unlikely(!multiplier)
		goto err_base_size;
	whole_size = DeeObject_Mul(multiplier, base_size);
	Dee_Decref(multiplier);
	if unlikely(!whole_size)
		goto err_base_size;
	index_ge_whole_size = DeeObject_CmpGeAsBool(index_ob, whole_size);
	if unlikely(index_ge_whole_size != 0) {
		if unlikely(index_ge_whole_size < 0)
			goto err_base_size_whole_size;
		DeeRT_ErrIndexOutOfBoundsObj(Dee_AsObject(self), index_ob, whole_size);
		goto err_base_size_whole_size;
	}
	Dee_Decref_likely(whole_size);
	base_index = DeeObject_Mod(index_ob, base_size);
	Dee_Decref_likely(base_size);
	return base_index;
err_base_size_whole_size:
	Dee_Decref(whole_size);
err_base_size:
	Dee_Decref(base_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_getitem(Repeat *self, DeeObject *index_ob) {
	DREF DeeObject *result;
	DREF DeeObject *base_index = repeat_translate_index_ob(self, index_ob);
	if unlikely(!base_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_getitem, self->rp_seq, base_index);
	Dee_Decref_likely(base_index);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_bounditem(Repeat *self, DeeObject *index_ob) {
	int result;
	DREF DeeObject *base_index = repeat_translate_index_ob(self, index_ob);
	if unlikely(!base_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_bounditem, self->rp_seq, base_index);
	Dee_Decref_likely(base_index);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_delitem(Repeat *self, DeeObject *index_ob) {
	int result;
	DREF DeeObject *base_index = repeat_translate_index_ob(self, index_ob);
	if unlikely(!base_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_delitem, self->rp_seq, base_index);
	Dee_Decref_likely(base_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
repeat_setitem(Repeat *self, DeeObject *index_ob, DeeObject *value) {
	int result;
	DREF DeeObject *base_index = repeat_translate_index_ob(self, index_ob);
	if unlikely(!base_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_setitem, self->rp_seq, base_index, value);
	Dee_Decref_likely(base_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_getitem_index(Repeat *__restrict self, size_t index) {
	size_t base_index = repeat_translate_index(self, index);
	if unlikely(base_index == (size_t)-1)
		goto err;
	return DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->rp_seq, base_index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_bounditem_index(Repeat *__restrict self, size_t index) {
	size_t base_index = repeat_translate_index(self, index);
	if unlikely(base_index == (size_t)-1)
		goto err;
	return DeeObject_InvokeMethodHint(seq_operator_bounditem_index, self->rp_seq, base_index);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_delitem_index(Repeat *__restrict self, size_t index) {
	size_t base_index = repeat_translate_index(self, index);
	if unlikely(base_index == (size_t)-1)
		goto err;
	return DeeObject_InvokeMethodHint(seq_operator_delitem_index, self->rp_seq, base_index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
repeat_setitem_index(Repeat *self, size_t index, DeeObject *value) {
	size_t base_index = repeat_translate_index(self, index);
	if unlikely(base_index == (size_t)-1)
		goto err;
	return DeeObject_InvokeMethodHint(seq_operator_setitem_index, self->rp_seq, base_index, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
repeat_xchitem_index(Repeat *self, size_t index, DeeObject *value) {
	size_t base_index = repeat_translate_index(self, index);
	if unlikely(base_index == (size_t)-1)
		goto err;
	return DeeObject_InvokeMethodHint(seq_xchitem_index, self->rp_seq, base_index, value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
repeat_foreach_impl2(DeeObject *__restrict seq,
                     Dee_foreach_t proc, void *arg,
                     DeeMH_seq_operator_foreach_t foreach,
                     size_t count) {
	Dee_ssize_t temp, result = 0;
	size_t i = 0;
	ASSERT(count >= 1);
	do {
		temp = (*foreach)(seq, proc, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err; /* For very high `count` */
	} while (++i < count);
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
repeat_foreach_impl(Repeat *__restrict self,
                    Dee_foreach_t proc, void *arg,
                    DeeMH_seq_operator_foreach_t foreach) {
	return repeat_foreach_impl2(self->rp_seq, proc, arg, foreach, self->rp_num);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach(Repeat *__restrict self, Dee_foreach_t proc, void *arg) {
	DeeTypeObject *tp_seq = Dee_TYPE(self->rp_seq);
	DeeMH_seq_operator_foreach_t foreach = DeeType_RequireMethodHint(tp_seq, seq_operator_foreach);
	return repeat_foreach_impl(self, proc, arg, foreach);
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_foreach_reverse(DeeTupleObject *__restrict self,
                         Dee_foreach_t proc, void *arg);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach_reverse(Repeat *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *as_tuple;
	DeeTypeObject *tp_seq = Dee_TYPE(self->rp_seq);
	DeeMH_seq_foreach_reverse_t foreach_reverse = DeeType_RequireMethodHint(tp_seq, seq_foreach_reverse);
	if (foreach_reverse != NULL)
		return repeat_foreach_impl(self, proc, arg, foreach_reverse);
	/* Convert underlying sequence to "Tuple" (which **does** implement reverse foreach) */
	as_tuple = DeeTuple_FromSequence(self->rp_seq);
	if unlikely(!as_tuple)
		goto err;
	foreach_reverse = (DeeMH_seq_foreach_reverse_t)&tuple_mh_foreach_reverse;
	result = repeat_foreach_impl2(as_tuple, proc, arg, foreach_reverse, self->rp_num);
	Dee_Decref_likely(as_tuple);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach_pair(Repeat *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	DeeTypeObject *tp_seq = Dee_TYPE(self->rp_seq);
	DeeMH_seq_operator_foreach_pair_t foreach_pair = DeeType_RequireMethodHint(tp_seq, seq_operator_foreach_pair);
#if 1
	return repeat_foreach_impl(self, (Dee_foreach_t)proc, arg, (DeeNO_foreach_t)foreach_pair);
#else
	Dee_ssize_t temp, result = 0;
	size_t i = 0;
	do {
		temp = (*foreach_pair)(self->rp_seq, proc, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err; /* For very high `rp_num` */
	} while (++i < self->rp_num);
	return result;
err_temp:
	return temp;
err:
	return -1;
#endif
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_print(Repeat *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Sequence %k repeated %" PRFuSIZ " times>",
	                        self->rp_seq, self->rp_num);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_printrepr(Repeat *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "Sequence.repeat(%r, %" PRFuSIZ ")",
	                        self->rp_seq, self->rp_num);
}

STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_trygetfirst generic_proxy__seq_trygetfirst
#define repeat_getfirst    generic_proxy__seq_getfirst
#define repeat_boundfirst  generic_proxy__seq_boundfirst
#define repeat_delfirst    generic_proxy__seq_delfirst
#define repeat_setfirst    generic_proxy__seq_setfirst
#define repeat_trygetlast  generic_proxy__seq_trygetlast
#define repeat_getlast     generic_proxy__seq_getlast
#define repeat_boundlast   generic_proxy__seq_boundlast
#define repeat_dellast     generic_proxy__seq_dellast
#define repeat_setlast     generic_proxy__seq_setlast


/* Sets force any duplicate values in the underlying mapping to be omitted.
 * So when viewing a "SeqRepeat" object as a set, the "repeat" part goes away. */
STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_set_trygetfirst            generic_proxy__set_trygetfirst
#define repeat_set_getfirst               generic_proxy__set_getfirst
#define repeat_set_boundfirst             generic_proxy__set_boundfirst
#define repeat_set_delfirst               generic_proxy__set_delfirst
#define repeat_set_setfirst               generic_proxy__set_setfirst
#define repeat_set_trygetlast             generic_proxy__set_trygetlast
#define repeat_set_getlast                generic_proxy__set_getlast
#define repeat_set_boundlast              generic_proxy__set_boundlast
#define repeat_set_dellast                generic_proxy__set_dellast
#define repeat_set_setlast                generic_proxy__set_setlast
#define repeat_set_operator_iter          generic_proxy__set_operator_iter
#define repeat_set_operator_foreach       generic_proxy__set_operator_foreach
#define repeat_set_operator_sizeob        generic_proxy__set_operator_sizeob
#define repeat_set_operator_size          generic_proxy__set_operator_size
#define repeat_set_operator_hash          generic_proxy__set_operator_hash
#define repeat_set_operator_compare_eq    generic_proxy__set_operator_compare_eq
#define repeat_set_operator_trycompare_eq generic_proxy__set_operator_trycompare_eq
#define repeat_set_operator_eq            generic_proxy__set_operator_eq
#define repeat_set_operator_ne            generic_proxy__set_operator_ne
#define repeat_set_operator_lo            generic_proxy__set_operator_lo
#define repeat_set_operator_le            generic_proxy__set_operator_le
#define repeat_set_operator_gr            generic_proxy__set_operator_gr
#define repeat_set_operator_ge            generic_proxy__set_operator_ge
#define repeat_set_operator_bool          generic_proxy__set_operator_bool
#define repeat_set_operator_inv           generic_proxy__set_operator_inv
#define repeat_set_operator_add           generic_proxy__set_operator_add
#define repeat_set_operator_sub           generic_proxy__set_operator_sub
#define repeat_set_operator_and           generic_proxy__set_operator_and
#define repeat_set_operator_xor           generic_proxy__set_operator_xor
//TODO:#define repeat_set_operator_inplace_add   generic_proxy__set_operator_inplace_add
//TODO:#define repeat_set_operator_inplace_sub   generic_proxy__set_operator_inplace_sub
//TODO:#define repeat_set_operator_inplace_and   generic_proxy__set_operator_inplace_and
//TODO:#define repeat_set_operator_inplace_xor   generic_proxy__set_operator_inplace_xor
#define repeat_set_frozen                 generic_proxy__set_frozen
#define repeat_set_unify                  generic_proxy__set_unify
#define repeat_set_insert                 generic_proxy__set_insert
#define repeat_set_insertall              generic_proxy__set_insertall
#define repeat_set_remove                 generic_proxy__set_remove
#define repeat_set_removeall              generic_proxy__set_removeall
#define repeat_set_pop                    generic_proxy__set_pop
#define repeat_set_pop_with_default       generic_proxy__set_pop_with_default

/* Just like sets, maps also (kind-of) omit duplicate items.
 *
 * Technically, they only omit duplicate items that are pairs,
 * and where that pair's first element was already encountered,
 * but that rule also describes any item where *both* elements
 * of the pair were already encountered! */
STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_map_operator_iter                       generic_proxy__map_operator_iter
#define repeat_map_operator_foreach_pair               generic_proxy__map_operator_foreach_pair
#define repeat_map_operator_sizeob                     generic_proxy__map_operator_sizeob
#define repeat_map_operator_size                       generic_proxy__map_operator_size
#define repeat_map_operator_hash                       generic_proxy__map_operator_hash
#define repeat_map_operator_getitem                    generic_proxy__map_operator_getitem
#define repeat_map_operator_trygetitem                 generic_proxy__map_operator_trygetitem
#define repeat_map_operator_getitem_index              generic_proxy__map_operator_getitem_index
#define repeat_map_operator_trygetitem_index           generic_proxy__map_operator_trygetitem_index
#define repeat_map_operator_getitem_string_hash        generic_proxy__map_operator_getitem_string_hash
#define repeat_map_operator_trygetitem_string_hash     generic_proxy__map_operator_trygetitem_string_hash
#define repeat_map_operator_getitem_string_len_hash    generic_proxy__map_operator_getitem_string_len_hash
#define repeat_map_operator_trygetitem_string_len_hash generic_proxy__map_operator_trygetitem_string_len_hash
#define repeat_map_operator_bounditem                  generic_proxy__map_operator_bounditem
#define repeat_map_operator_bounditem_index            generic_proxy__map_operator_bounditem_index
#define repeat_map_operator_bounditem_string_hash      generic_proxy__map_operator_bounditem_string_hash
#define repeat_map_operator_bounditem_string_len_hash  generic_proxy__map_operator_bounditem_string_len_hash
#define repeat_map_operator_hasitem                    generic_proxy__map_operator_hasitem
#define repeat_map_operator_hasitem_index              generic_proxy__map_operator_hasitem_index
#define repeat_map_operator_hasitem_string_hash        generic_proxy__map_operator_hasitem_string_hash
#define repeat_map_operator_hasitem_string_len_hash    generic_proxy__map_operator_hasitem_string_len_hash
#define repeat_map_operator_delitem                    generic_proxy__map_operator_delitem
#define repeat_map_operator_delitem_index              generic_proxy__map_operator_delitem_index
#define repeat_map_operator_delitem_string_hash        generic_proxy__map_operator_delitem_string_hash
#define repeat_map_operator_delitem_string_len_hash    generic_proxy__map_operator_delitem_string_len_hash
#define repeat_map_operator_setitem                    generic_proxy__map_operator_setitem
#define repeat_map_operator_setitem_index              generic_proxy__map_operator_setitem_index
#define repeat_map_operator_setitem_string_hash        generic_proxy__map_operator_setitem_string_hash
#define repeat_map_operator_setitem_string_len_hash    generic_proxy__map_operator_setitem_string_len_hash
#define repeat_map_operator_contains                   generic_proxy__map_operator_contains
#define repeat_map_keys                                generic_proxy__map_keys
#define repeat_map_iterkeys                            generic_proxy__map_iterkeys
#define repeat_map_values                              generic_proxy__map_values
#define repeat_map_itervalues                          generic_proxy__map_itervalues
#define repeat_map_enumerate                           generic_proxy__map_enumerate
#define repeat_map_enumerate_range                     generic_proxy__map_enumerate_range
#define repeat_map_makeenumeration                     generic_proxy__map_makeenumeration
#define repeat_map_makeenumeration_with_range          generic_proxy__map_makeenumeration_with_range
#define repeat_map_operator_compare_eq                 generic_proxy__map_operator_compare_eq
#define repeat_map_operator_trycompare_eq              generic_proxy__map_operator_trycompare_eq
#define repeat_map_operator_eq                         generic_proxy__map_operator_eq
#define repeat_map_operator_ne                         generic_proxy__map_operator_ne
#define repeat_map_operator_lo                         generic_proxy__map_operator_lo
#define repeat_map_operator_le                         generic_proxy__map_operator_le
#define repeat_map_operator_gr                         generic_proxy__map_operator_gr
#define repeat_map_operator_ge                         generic_proxy__map_operator_ge
#define repeat_map_operator_add                        generic_proxy__map_operator_add
#define repeat_map_operator_sub                        generic_proxy__map_operator_sub
#define repeat_map_operator_and                        generic_proxy__map_operator_and
#define repeat_map_operator_xor                        generic_proxy__map_operator_xor
//TODO:#define repeat_map_operator_inplace_add                generic_proxy__map_operator_inplace_add
//TODO:#define repeat_map_operator_inplace_sub                generic_proxy__map_operator_inplace_sub
//TODO:#define repeat_map_operator_inplace_and                generic_proxy__map_operator_inplace_and
//TODO:#define repeat_map_operator_inplace_xor                generic_proxy__map_operator_inplace_xor
#define repeat_map_frozen                              generic_proxy__map_frozen
#define repeat_map_setold                              generic_proxy__map_setold
#define repeat_map_setold_ex                           generic_proxy__map_setold_ex
#define repeat_map_setnew                              generic_proxy__map_setnew
#define repeat_map_setnew_ex                           generic_proxy__map_setnew_ex
#define repeat_map_setdefault                          generic_proxy__map_setdefault
#define repeat_map_update                              generic_proxy__map_update
#define repeat_map_remove                              generic_proxy__map_remove
#define repeat_map_removekeys                          generic_proxy__map_removekeys
#define repeat_map_pop                                 generic_proxy__map_pop
#define repeat_map_pop_with_default                    generic_proxy__map_pop_with_default
#define repeat_map_popitem                             generic_proxy__map_popitem



PRIVATE struct type_seq repeat_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_operator_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeat_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&repeat_setitem,
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&repeat_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&repeat_foreach_pair,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeat_bounditem,
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&repeat_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&repeat_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeat_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&repeat_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&repeat_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&repeat_bounditem_index,
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF Repeat *DCALL
repeat_replaceseq_impl(Repeat *self, /*inherit(always)*/ DREF DeeObject *new_seq) {
	DREF Repeat *result;
	if (new_seq == self->rp_seq) {
		Dee_DecrefNokill(new_seq);
		return_reference_(self);
	}
	result = DeeObject_MALLOC(Repeat);
	if unlikely(!result)
		goto err_new_seq;
	result->rp_seq = new_seq; /* Inherit reference */
	result->rp_num = self->rp_num;
	DeeObject_InitStatic(result, &SeqRepeat_Type);
	return result;
err_new_seq:
	Dee_Decref(new_seq);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Repeat *DCALL
repeat_get_frozen(Repeat *__restrict self) {
	DREF DeeObject *inner_frozen;
	inner_frozen = DeeObject_InvokeMethodHint(seq_frozen, self->rp_seq);
	if unlikely(!inner_frozen)
		goto err;
	return repeat_replaceseq_impl(self, inner_frozen);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Repeat *DCALL
repeat_get_cached(Repeat *__restrict self) {
	DREF DeeObject *inner_cached;
	inner_cached = DeeObject_InvokeMethodHint(seq_cached, self->rp_seq);
	if unlikely(!inner_cached)
		goto err;
	return repeat_replaceseq_impl(self, inner_cached);
err:
	return NULL;
}

/* Descriptor for a [start,end) range applied to a SeqRepeat
 * object. This struct describes how that range translates to
 * the underlying sequence. */
struct repeat_range_desc {
	/*
	 *  Sequence.repeat("FOO", 999)
	 *     012345678911111111112222222222
	 *               01234567890123456789
	 *     [-][-][-][-][-][-][-][-][-][-]
	 *     FOOFOOFOOFOOFOOFOOFOOFOOFOOFOO...
	 *      [-------]
	 *      ^        ^
	 *      |        end=10
	 *      start=1
	 *
	 *  This results in a descriptor
	 *
	 *     012345678911111111112222222222
	 *               01234567890123456789
	 *     [-][-][-][-][-][-][-][-][-][-]
	 *     FOOFOOFOOFOOFOOFOOFOOFOOFOOFOO...
	 *      [][-]...[]
	 *      | |     |
	 *      | |     TAIL={rrd_tail_start=0, rrd_tail_end=2}
	 *      | |
	 *      | BODY={rrd_body_rep=2}
	 *      |
	 *      HEAD={rrd_head_start=1, rrd_head_end=3}
	 */
	size_t rrd_head_start; /* Start index of head */
	size_t rrd_head_end;   /* End index of head */
	size_t rrd_body_rep;   /* # of times the entire child-sequence appears in the body */
//	size_t rrd_tail_start; /* Start index of tail (Always `0`) */
	size_t rrd_tail_end;   /* End index of tail */
	size_t rrd_skip;       /* # of elements that were skipped (a multiple of repeat_range_desc_getbodylen1) */
#define repeat_range_desc_hashead(self)      ((self)->rrd_head_end > (self)->rrd_head_start)
#define repeat_range_desc_getheadstart(self) ((self)->rrd_head_start)
#define repeat_range_desc_getheadend(self)   ((self)->rrd_head_end)
#define repeat_range_desc_getheadlen(self)   ((self)->rrd_head_end - (self)->rrd_head_start)
#define repeat_range_desc_hasbody(self)      ((self)->rrd_body_rep != 0)
#define repeat_range_desc_getbodyrep(self)   ((self)->rrd_body_rep)
#define repeat_range_desc_getbodylen1(self)  ((self)->rrd_head_end) /* Only valid when `repeat_range_desc_getbodyrep(self) > 0` */
#define repeat_range_desc_getbodylenN(self)  (repeat_range_desc_getbodyrep(self) * repeat_range_desc_getbodylen1(self))
#define repeat_range_desc_getbodystart(self) 0
#define repeat_range_desc_getbodyend(self)   repeat_range_desc_getbodylen1(self)
#define repeat_range_desc_hastail(self)      ((self)->rrd_tail_end > 0)
#define repeat_range_desc_gettailstart(self) 0
#define repeat_range_desc_gettailend(self)   ((self)->rrd_tail_end)
#define repeat_range_desc_gettaillen(self)   ((self)->rrd_tail_end)

	/* Returns absolute (iow: within the SeqRepeat) indices of chunks. */
#define repeat_range_desc_getabsheadstart(self) ((self)->rrd_skip + (self)->rrd_head_start)
#define repeat_range_desc_getabsheadend(self)   ((self)->rrd_skip + (self)->rrd_head_end)
#define repeat_range_desc_getabsbodystart(self) repeat_range_desc_getabsheadend(self)
#define repeat_range_desc_getabsbodyend(self)   (repeat_range_desc_getabsbodystart(self) + repeat_range_desc_getbodylenN(self))
#define repeat_range_desc_getabstailstart(self) repeat_range_desc_getabsbodyend(self)
#define repeat_range_desc_getabstailend(self)   (repeat_range_desc_getabstailstart(self) + repeat_range_desc_gettaillen(self))
};

/* Modify `self` such that elements are never enumerated more than once.
 * Upon return, you're allowed to assume that `repeat_range_desc_getbodyrep(self) == 0` */
PRIVATE WUNUSED NONNULL((1)) void DCALL
repeat_range_desc_makeonce(struct repeat_range_desc *__restrict self) {
	if (repeat_range_desc_hasbody(self)) {
		/* - Keep `rrd_head_start` unchanged -> range still starts on same element
		 * - Because there is a body, `rrd_head_end` is equal to the length of the
		 *   sub-sequence, meaning that the head is guarantied to hug the end of
		 *   said sub-sequence
		 * - Change the tail-range such that it describes everything not already
		 *   covered by the head (which, like already said: hugs the end of the
		 *   sub-sequence)
		 * - Change the body-repetition counter `rrd_body_rep` to `0` such that
		 *   no extra loops are made between the head and tail (which now cover
		 *   the sub-sequence in its entirety exactly once) */
		self->rrd_tail_end = self->rrd_head_start;
		self->rrd_body_rep = 0;
	}
}

/* Same as `repeat_range_desc_makeonce()`, but retain `rrd_tail_end` */
PRIVATE WUNUSED NONNULL((1)) void DCALL
repeat_range_desc_makeonce_rev(struct repeat_range_desc *__restrict self) {
	if (repeat_range_desc_hasbody(self)) {
		self->rrd_head_start = self->rrd_tail_end;
		self->rrd_body_rep   = 0;
	}
}

/* Initialize `self` from `repeat` and the half-open range described by `[start,end)` */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_range_desc_of(struct repeat_range_desc *__restrict self,
                     Repeat *__restrict repeat,
                     size_t start, size_t end) {
	size_t seq_len, leading_skip;
	size_t total_len;
	ASSERTF(repeat->rp_num != 0, "Prevented during construction");

	self->rrd_body_rep = 0;
	self->rrd_tail_end = 0;
	if (start >= end) {
		/* Simple case: entirely empty */
return_empty:
		self->rrd_skip       = 0;
		self->rrd_head_start = 0;
		self->rrd_head_end   = 0;
		return 0;
	}

	/* Check for simple (but unlikely) case: sequence isn't actually being repeated.
	 * In this case, rely on the fact that the caller will always invoke some sequence
	 * function of the wrapped sequence, which in turn will clamp the `end` index as
	 * necessary. */
	if unlikely(repeat->rp_num == 1) {
		self->rrd_skip       = 0;
		self->rrd_head_start = start;
		self->rrd_head_end   = end;
		return 0;
	}

	/* Query length of underlying sequence. */
	seq_len = DeeObject_InvokeMethodHint(seq_operator_size, repeat->rp_seq);
	if unlikely(seq_len == (size_t)-1) {
		DREF DeeObject *actual_size = DeeRT_IntegerOverflow_TryCatchSizeOverflow();
		if (!actual_size)
			goto err;
		/* Sequence is larger than SIZE_MAX -> caller's range always fits */
		Dee_Decref(actual_size);
		self->rrd_skip       = 0;
		self->rrd_head_start = start;
		self->rrd_head_end   = end;
		return 0;
	}

	/* Clamp `end` if it points beyond the repetition's absolute end */
	if (!OVERFLOW_UMUL(seq_len, repeat->rp_num, &total_len)) {
		if (end > total_len) {
			end = total_len;
			if (start >= end)
				goto return_empty;
		}
	}

	/* Deal with special case: underlying sequence is empty. */
	if unlikely(seq_len == 0)
		goto return_empty;
	leading_skip = (start / seq_len) * seq_len;
	start -= leading_skip;
	end -= leading_skip;
	self->rrd_skip = leading_skip;
	self->rrd_head_start = start;
	if (end <= seq_len) {
		/* Entirely contained in head */
		self->rrd_head_end = end;
	} else {
		size_t count_after_head = end - seq_len; /* # of 0-aligned items to iterate after head */
		self->rrd_head_end = seq_len; /* Head ends at end of underlying sequence (because there is a body/tail) */
		self->rrd_body_rep = count_after_head / seq_len;
		self->rrd_tail_end = count_after_head % seq_len;
		ASSERT(self->rrd_body_rep > 0 || self->rrd_tail_end > 0);
	}
	return 0;
err:
	return -1;
}


STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_any          generic_proxy__seq_any
#define repeat_any_with_key generic_proxy__seq_any_with_key

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_any_with_range(Repeat *__restrict self, size_t start, size_t end) {
	int result = 0;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range)) {
		result = DeeObject_InvokeMethodHint(seq_any_with_range, self->rp_seq,
		                                    repeat_range_desc_getheadstart(&range),
		                                    repeat_range_desc_getheadend(&range));
		if (result != 0)
			goto done;
	}
	if (repeat_range_desc_hastail(&range)) {
		result = DeeObject_InvokeMethodHint(seq_any_with_range, self->rp_seq,
		                                    repeat_range_desc_gettailstart(&range),
		                                    repeat_range_desc_gettailend(&range));
		if (result != 0)
			goto done;
	}
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeat_any_with_range_and_key(Repeat *self, size_t start, size_t end, DeeObject *key) {
	int result = 0;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range)) {
		result = DeeObject_InvokeMethodHint(seq_any_with_range_and_key, self->rp_seq,
		                                    repeat_range_desc_getheadstart(&range),
		                                    repeat_range_desc_getheadend(&range),
		                                    key);
		if (result != 0)
			goto done;
	}
	if (repeat_range_desc_hastail(&range)) {
		result = DeeObject_InvokeMethodHint(seq_any_with_range_and_key, self->rp_seq,
		                                    repeat_range_desc_gettailstart(&range),
		                                    repeat_range_desc_gettailend(&range),
		                                    key);
		if (result != 0)
			goto done;
	}
done:
	return result;
err:
	return -1;
}


STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_all          generic_proxy__seq_all
#define repeat_all_with_key generic_proxy__seq_all_with_key

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_all_with_range(Repeat *__restrict self, size_t start, size_t end) {
	int result = 1;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range)) {
		result = DeeObject_InvokeMethodHint(seq_all_with_range, self->rp_seq,
		                                    repeat_range_desc_getheadstart(&range),
		                                    repeat_range_desc_getheadend(&range));
		if (result <= 0)
			goto done;
	}
	if (repeat_range_desc_hastail(&range)) {
		result = DeeObject_InvokeMethodHint(seq_all_with_range, self->rp_seq,
		                                    repeat_range_desc_gettailstart(&range),
		                                    repeat_range_desc_gettailend(&range));
		if (result <= 0)
			goto done;
	}
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeat_all_with_range_and_key(Repeat *self, size_t start, size_t end, DeeObject *key) {
	int result = 1;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range)) {
		result = DeeObject_InvokeMethodHint(seq_all_with_range_and_key, self->rp_seq,
		                                    repeat_range_desc_getheadstart(&range),
		                                    repeat_range_desc_getheadend(&range),
		                                    key);
		if (result <= 0)
			goto done;
	}
	if (repeat_range_desc_hastail(&range)) {
		result = DeeObject_InvokeMethodHint(seq_all_with_range_and_key, self->rp_seq,
		                                    repeat_range_desc_gettailstart(&range),
		                                    repeat_range_desc_gettailend(&range),
		                                    key);
		if (result <= 0)
			goto done;
	}
done:
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_parity(Repeat *__restrict self) {
	if (!(self->rp_num & 1))
		return 0; /* Even total -> paritiy is always = 0 */
	return DeeObject_InvokeMethodHint(seq_parity, self->rp_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_parity_with_key(Repeat *__restrict self, DeeObject *key) {
	if (!(self->rp_num & 1))
		return 0; /* Even total -> paritiy is always = 0 */
	return DeeObject_InvokeMethodHint(seq_parity_with_key, self->rp_seq, key);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_parity_with_range_impl(Repeat *self, size_t start, size_t end, DeeObject *key) {
	int result = 0;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	if (repeat_range_desc_hashead(&range)) {
		result = key ? DeeObject_InvokeMethodHint(seq_parity_with_range_and_key, self->rp_seq,
		                                          repeat_range_desc_getheadstart(&range),
		                                          repeat_range_desc_getheadend(&range),
		                                          key)
		             : DeeObject_InvokeMethodHint(seq_parity_with_range, self->rp_seq,
		                                          repeat_range_desc_getheadstart(&range),
		                                          repeat_range_desc_getheadend(&range));
		if (result < 0)
			goto done;
	}
	if ((repeat_range_desc_getbodyrep(&range) & 1) != 0) {
		/* Uneven body repetition -> must include parity of body exactly once */
		int temp = key ? DeeObject_InvokeMethodHint(seq_parity_with_key, self->rp_seq, key)
		               : DeeObject_InvokeMethodHint(seq_parity, self->rp_seq);
		if (temp < 0)
			goto err;
		if (temp)
			result = result ? 0 : 1;
	}
	if (repeat_range_desc_hastail(&range)) {
		int temp = key ? DeeObject_InvokeMethodHint(seq_parity_with_range_and_key, self->rp_seq,
		                                            repeat_range_desc_gettailstart(&range),
		                                            repeat_range_desc_gettailend(&range),
		                                            key)
		               : DeeObject_InvokeMethodHint(seq_parity_with_range, self->rp_seq,
		                                            repeat_range_desc_gettailstart(&range),
		                                            repeat_range_desc_gettailend(&range));
		if (temp < 0)
			goto err;
		if (temp)
			result = result ? 0 : 1;
	}
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_parity_with_range(Repeat *__restrict self, size_t start, size_t end) {
	return repeat_parity_with_range_impl(self, start, end, NULL);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeat_parity_with_range_and_key(Repeat *self, size_t start, size_t end, DeeObject *key) {
	return repeat_parity_with_range_impl(self, start, end, key);
}


STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_min          generic_proxy__seq_min
#define repeat_min_with_key generic_proxy__seq_min_with_key
#define repeat_max          generic_proxy__seq_max
#define repeat_max_with_key generic_proxy__seq_max_with_key

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeat_minmax_with_range_impl_helper2(DeeObject *__restrict subseq,
                                      size_t start, size_t end,
                                      DeeObject *def, DeeObject *key,
                                      bool is_max) {
	if (key) {
		DeeMH_seq_min_with_range_and_key_t cb;
		cb = is_max ? DeeObject_RequireMethodHint(subseq, seq_max_with_range_and_key)
		            : DeeObject_RequireMethodHint(subseq, seq_min_with_range_and_key);
		return (*cb)(subseq, start, end, def, key);
	} else {
		DeeMH_seq_min_with_range_t cb;
		cb = is_max ? DeeObject_RequireMethodHint(subseq, seq_max_with_range)
		            : DeeObject_RequireMethodHint(subseq, seq_min_with_range);
		return (*cb)(subseq, start, end, def);
	}
}

/* @return: ITER_DONE: "def" value was used */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_minmax_with_range_impl_helper(DeeObject *__restrict subseq,
                                     size_t start, size_t end,
                                     DeeObject *key, bool is_max) {
	DREF DeeObject *result;
	DREF DeeObject *def = DeeObject_NewDummy();
	if unlikely(!def)
		goto err;
	result = repeat_minmax_with_range_impl_helper2(subseq, start, end, def, key, is_max);
	if (result == def) {
		Dee_DecrefNokill(result);
		result = ITER_DONE;
	}
	Dee_Decref_likely(def);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeat_minmax_with_range_impl(Repeat *__restrict self,
                              size_t start, size_t end,
                              DeeObject *def, DeeObject *key,
                              bool is_max) {
	DREF DeeObject *result;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range) &&
	    repeat_range_desc_hastail(&range)) {
		DREF DeeObject *head_result, *tail_result;
		head_result = repeat_minmax_with_range_impl_helper(self->rp_seq,
		                                                   repeat_range_desc_getheadstart(&range),
		                                                   repeat_range_desc_getheadend(&range),
		                                                   key, is_max);
		if unlikely(!head_result)
			goto err;
		tail_result = repeat_minmax_with_range_impl_helper(self->rp_seq,
		                                                   repeat_range_desc_gettailstart(&range),
		                                                   repeat_range_desc_gettailend(&range),
		                                                   key, is_max);
		if unlikely(!tail_result) {
			if (head_result != ITER_DONE)
				Dee_Decref(head_result);
			goto err;
		}
		if (head_result == ITER_DONE) {
			if (tail_result == ITER_DONE)
				goto return_def;
			result = tail_result;
		} else if (tail_result == ITER_DONE) {
			result = head_result;
		} else {
			int head_lo_tail;
			head_lo_tail = DeeObject_CmpLoAsBool(head_result, tail_result);
			if unlikely(head_lo_tail < 0) {
				Dee_Decref(head_result);
				Dee_Decref(tail_result);
				result = NULL;
			} else if ((head_lo_tail > 0) != is_max) {
				/* Use "head_result" */
				result = head_result;
				Dee_Decref(tail_result);
			} else {
				/* Use "tail_result" */
				result = tail_result;
				Dee_Decref(head_result);
			}
		}
	} else {
		size_t used_start, used_end;
		if (repeat_range_desc_hashead(&range)) {
			used_start = repeat_range_desc_getheadstart(&range);
			used_end   = repeat_range_desc_getheadend(&range);
		} else if (repeat_range_desc_hastail(&range)) {
			used_start = repeat_range_desc_gettailstart(&range);
			used_end   = repeat_range_desc_gettailend(&range);
		} else {
			goto return_def;
		}
		result = repeat_minmax_with_range_impl_helper2(self->rp_seq, used_start, used_end,
		                                               def, key, is_max);
	}
/*done:*/
	return result;
return_def:
	return_reference_(def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeat_min_with_range(Repeat *self, size_t start, size_t end, DeeObject *def) {
	return repeat_minmax_with_range_impl(self, start, end, def, NULL, false);
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
repeat_min_with_range_and_key(Repeat *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	return repeat_minmax_with_range_impl(self, start, end, def, key, false);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeat_max_with_range(Repeat *self, size_t start, size_t end, DeeObject *def) {
	return repeat_minmax_with_range_impl(self, start, end, def, NULL, true);
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
repeat_max_with_range_and_key(Repeat *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	return repeat_minmax_with_range_impl(self, start, end, def, key, true);
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_sum_impl(Repeat *self, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *dummy_def;
	dummy_def = DeeObject_NewDummy();
	if unlikely(!dummy_def)
		goto err;
	result = key ? DeeObject_InvokeMethodHint(seq_sum_with_key, self->rp_seq, dummy_def, key)
	             : DeeObject_InvokeMethodHint(seq_sum, self->rp_seq, dummy_def);
	Dee_Decref_likely(dummy_def);
	if unlikely(!result)
		goto err;
	if (result == dummy_def) {
		result = def;
		Dee_Incref(def);
	} else {
		result = generic_object_repeat_sum_inherited(result, self->rp_num);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_sum(Repeat *self, DeeObject *def) {
	return repeat_sum_impl(self, def, NULL);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
repeat_sum_with_key(Repeat *self, DeeObject *def, DeeObject *key) {
	return repeat_sum_impl(self, def, key);
}

/* Append `subseq.sum(start, end)` to `accu`, but only if that sum is present */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_sum_with_range_impl2(struct Dee_accu *__restrict accu,
                            DeeObject *subseq, size_t start,
                            size_t end, size_t repeat,
                            DeeObject *key) {
	DREF DeeObject *range_sum;
	DREF DeeObject *dummy_def;
	if unlikely(start >= end || repeat == 0)
		return 0; /* Empty range -> do nothing */
	dummy_def = DeeObject_NewDummy();
	if unlikely(!dummy_def)
		goto err;
	range_sum = key ? DeeObject_InvokeMethodHint(seq_sum_with_range_and_key, subseq, start, end, dummy_def, key)
	                : DeeObject_InvokeMethodHint(seq_sum_with_range, subseq, start, end, dummy_def);
	Dee_Decref_likely(dummy_def);
	if unlikely(!range_sum)
		goto err;
	if (range_sum == dummy_def) {
		Dee_Decref_likely(range_sum);
	} else {
		/* If necessary, repeat "range_sum" before added it to the accu */
		if (repeat > 1) {
			range_sum = generic_object_repeat_sum_inherited(range_sum, repeat);
			if unlikely(!range_sum)
				goto err;
		}

		/* Append to accu */
		if (Dee_accu_add_inherited(accu, range_sum) < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
repeat_sum_with_range_impl(Repeat *self, size_t start, size_t end,
                           DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	struct Dee_accu accu;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	Dee_accu_init(&accu);

	/* Head */
	if (repeat_sum_with_range_impl2(&accu, self->rp_seq,
	                                repeat_range_desc_getheadstart(&range),
	                                repeat_range_desc_getheadend(&range),
	                                1, key))
		goto err_accu;

	/* Body */
	if (repeat_sum_with_range_impl2(&accu, self->rp_seq,
	                                repeat_range_desc_getbodystart(&range),
	                                repeat_range_desc_getbodyend(&range),
	                                repeat_range_desc_getbodyrep(&range),
	                                key))
		goto err_accu;

	/* Tail */
	if (repeat_sum_with_range_impl2(&accu, self->rp_seq,
	                                repeat_range_desc_gettailstart(&range),
	                                repeat_range_desc_gettailend(&range),
	                                1, key))
		goto err_accu;

	result = Dee_accu_pack(&accu);
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err_accu:
	Dee_accu_fini(&accu);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeat_sum_with_range(Repeat *self, size_t start, size_t end, DeeObject *def) {
	return repeat_sum_with_range_impl(self, start, end, def, NULL);
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
repeat_sum_with_range_and_key(Repeat *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	return repeat_sum_with_range_impl(self, start, end, def, key);
}


/* NOTE: If `base_count == (size_t)-1`, assumes that
 * an exception was thrown and always propagate as-is */
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_multiply_count(Repeat *__restrict self, size_t base_count) {
	size_t total_count;
	if unlikely(base_count == (size_t)-1)
		return base_count; /* Error happend in caller */
	if (OVERFLOW_UMUL(base_count, self->rp_num, &total_count))
		return (size_t)DeeRT_ErrIntegerOverflowUMul(base_count, self->rp_num);
	ASSERTF(total_count != (size_t)-1,
	        "Can't be case multiplication didn't overflow, meaning `total_count` "
	        "must be a multiple of `base_count`, and `base_count` itself was already "
	        "check to be `!= (size_t)-1`");
	return total_count;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeat_count(Repeat *self, DeeObject *item) {
	size_t base_count = DeeObject_InvokeMethodHint(seq_count, self->rp_seq, item);
	return repeat_multiply_count(self, base_count);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) size_t DCALL
repeat_count_with_key(Repeat *self, DeeObject *item, DeeObject *key) {
	size_t base_count = DeeObject_InvokeMethodHint(seq_count_with_key, self->rp_seq, item, key);
	return repeat_multiply_count(self, base_count);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeat_count_with_range_impl2(DeeObject *subseq, DeeObject *item,
                              size_t start, size_t end, DeeObject *key) {
	return key ? DeeObject_InvokeMethodHint(seq_count_with_range_and_key, subseq, item, start, end, key)
	           : DeeObject_InvokeMethodHint(seq_count_with_range, subseq, item, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeat_count_with_range_impl(Repeat *self, DeeObject *item,
                             size_t start, size_t end, DeeObject *key) {
	size_t result = 0;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	if (repeat_range_desc_hashead(&range)) {
		result = repeat_count_with_range_impl2(self->rp_seq, item,
		                                       repeat_range_desc_getheadstart(&range),
		                                       repeat_range_desc_getheadend(&range),
		                                       key);
		if unlikely(result == (size_t)-1)
			goto err;
	}
	if (repeat_range_desc_hasbody(&range)) {
		size_t multipled_count, new_result;
		size_t base_count = key ? DeeObject_InvokeMethodHint(seq_count_with_key, self->rp_seq, item, key)
		                        : DeeObject_InvokeMethodHint(seq_count, self->rp_seq, item);
		if unlikely(base_count == (size_t)-1)
			goto err;
		if (OVERFLOW_UMUL(base_count, repeat_range_desc_getbodyrep(&range), &multipled_count))
			return (size_t)DeeRT_ErrIntegerOverflowUMul(base_count, repeat_range_desc_getbodyrep(&range));
		if (OVERFLOW_UADD(result, multipled_count, &new_result))
			return (size_t)DeeRT_ErrIntegerOverflowUAdd(result, multipled_count);
		result = new_result;
	}
	if (repeat_range_desc_hastail(&range)) {
		size_t tail_count, new_result;
		tail_count = repeat_count_with_range_impl2(self->rp_seq, item,
		                                           repeat_range_desc_getheadstart(&range),
		                                           repeat_range_desc_getheadend(&range),
		                                           key);
		if unlikely(tail_count == (size_t)-1)
			goto err;
		if (OVERFLOW_UADD(result, tail_count, &new_result))
			return (size_t)DeeRT_ErrIntegerOverflowUAdd(result, tail_count);
		result = new_result;
	}
	if unlikely(result == (size_t)-1)
		return (size_t)DeeRT_ErrIntegerOverflowU(result, (size_t)-2);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeat_count_with_range(Repeat *self, DeeObject *item, size_t start, size_t end) {
	return repeat_count_with_range_impl(self, item, start, end, NULL);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
repeat_count_with_range_and_key(Repeat *self, DeeObject *item,
                                size_t start, size_t end, DeeObject *key) {
	return repeat_count_with_range_impl(self, item, start, end, key);
}



STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_contains          generic_proxy__seq_contains
#define repeat_contains_with_key generic_proxy__seq_contains_with_key

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_contains_with_range_impl2(DeeObject *subseq, DeeObject *item,
                                 size_t start, size_t end, DeeObject *key) {
	return key ? DeeObject_InvokeMethodHint(seq_contains_with_range_and_key, subseq, item, start, end, key)
	           : DeeObject_InvokeMethodHint(seq_contains_with_range, subseq, item, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_contains_with_range_impl(Repeat *__restrict self, DeeObject *item,
                                size_t start, size_t end, DeeObject *key) {
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range)) {
		int status = repeat_contains_with_range_impl2(self->rp_seq, item,
		                                              repeat_range_desc_getheadstart(&range),
		                                              repeat_range_desc_getheadend(&range),
		                                              key);
		if (status != 0)
			return status;
	}
	if (repeat_range_desc_hastail(&range)) {
		int status = repeat_contains_with_range_impl2(self->rp_seq, item,
		                                              repeat_range_desc_gettailstart(&range),
		                                              repeat_range_desc_gettailend(&range),
		                                              key);
		if (status != 0)
			return status;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_contains_with_range(Repeat *self, DeeObject *item, size_t start, size_t end) {
	return repeat_contains_with_range_impl(self, item, start, end, NULL);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
repeat_contains_with_range_and_key(Repeat *self, DeeObject *item,
                                   size_t start, size_t end, DeeObject *key) {
	return repeat_contains_with_range_impl(self, item, start, end, key);
}



STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_locate generic_proxy__seq_locate

/* @return: ITER_DONE: not matched. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_locate_with_range_impl(DeeObject *subseq, DeeObject *match,
                              size_t start, size_t end) {
	DREF DeeObject *result;
	DREF DeeObject *dummy_def = DeeObject_NewDummy();
	if unlikely(!dummy_def)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_locate_with_range,
	                                    subseq, match,
	                                    start, end, dummy_def);
	Dee_Decref(dummy_def);
	if (result == dummy_def)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
repeat_locate_with_range(Repeat *self, DeeObject *match,
                         size_t start, size_t end, DeeObject *def) {
	DREF DeeObject *result;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce(&range);
	if (repeat_range_desc_hashead(&range) &&
	    repeat_range_desc_hastail(&range)) {
		/* Need to inspect both ranges */
		result = repeat_locate_with_range_impl(self->rp_seq, match,
		                                       repeat_range_desc_getheadstart(&range),
		                                       repeat_range_desc_getheadend(&range));
		if (result != ITER_DONE)
			return result; /* Matched, or error */
		goto do_search_tail;
	} else if (repeat_range_desc_hashead(&range)) {
		result = DeeObject_InvokeMethodHint(seq_locate_with_range, self->rp_seq, match,
		                                    repeat_range_desc_getheadstart(&range),
		                                    repeat_range_desc_getheadend(&range), def);
	} else if (repeat_range_desc_hastail(&range)) {
do_search_tail:
		result = DeeObject_InvokeMethodHint(seq_locate_with_range, self->rp_seq, match,
		                                    repeat_range_desc_gettailstart(&range),
		                                    repeat_range_desc_gettailend(&range), def);
	} else {
		return_reference_(def);
	}
	return result;
err:
	return NULL;
}

STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(ProxyObject, po_obj));
#define repeat_rlocate generic_proxy__seq_rlocate

/* @return: ITER_DONE: not matched. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_rlocate_with_range_impl(DeeObject *subseq, DeeObject *match,
                               size_t start, size_t end) {
	DREF DeeObject *result;
	DREF DeeObject *dummy_def = DeeObject_NewDummy();
	if unlikely(!dummy_def)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_rlocate_with_range,
	                                    subseq, match,
	                                    start, end, dummy_def);
	Dee_Decref(dummy_def);
	if (result == dummy_def)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
repeat_rlocate_with_range(Repeat *self, DeeObject *match,
                          size_t start, size_t end, DeeObject *def) {
	DREF DeeObject *result;
	struct repeat_range_desc range;
	if (repeat_range_desc_of(&range, self, start, end))
		goto err;
	repeat_range_desc_makeonce_rev(&range);
	if (repeat_range_desc_hashead(&range) &&
	    repeat_range_desc_hastail(&range)) {
		/* Need to inspect both ranges */
		result = repeat_rlocate_with_range_impl(self->rp_seq, match,
		                                        repeat_range_desc_gettailstart(&range),
		                                        repeat_range_desc_gettailend(&range));
		if (result != ITER_DONE)
			return result; /* Matched, or error */
		goto do_search_head;
	} else if (repeat_range_desc_hashead(&range)) {
do_search_head:
		result = DeeObject_InvokeMethodHint(seq_rlocate_with_range, self->rp_seq, match,
		                                    repeat_range_desc_getheadstart(&range),
		                                    repeat_range_desc_getheadend(&range), def);
	} else if (repeat_range_desc_hastail(&range)) {
		result = DeeObject_InvokeMethodHint(seq_rlocate_with_range, self->rp_seq, match,
		                                    repeat_range_desc_gettailstart(&range),
		                                    repeat_range_desc_gettailend(&range), def);
	} else {
		return_reference_(def);
	}
	return result;
err:
	return NULL;
}




PRIVATE struct type_getset tpconst repeat_getsets[] = {
	TYPE_GETTER(STR_frozen, &repeat_get_frozen, "->?."),
	TYPE_GETTER(STR_cached, &repeat_get_cached, "->?."),
	TYPE_GETSET_BOUND_F_NODOC(STR_first, &repeat_getfirst, &repeat_delfirst, &repeat_setfirst, &repeat_boundfirst, METHOD_FNOREFESCAPE),
	TYPE_GETSET_BOUND_F_NODOC(STR_last, &repeat_getlast, &repeat_dellast, &repeat_setlast, &repeat_boundlast, METHOD_FNOREFESCAPE),
	TYPE_GETSET_BOUND_F_NODOC(STR___set_first__, &repeat_set_getfirst, &repeat_set_delfirst, &repeat_set_setfirst, &repeat_set_boundfirst, METHOD_FNOREFESCAPE),
	TYPE_GETSET_BOUND_F_NODOC(STR___set_last__, &repeat_set_getlast, &repeat_set_dellast, &repeat_set_setlast, &repeat_set_boundlast, METHOD_FNOREFESCAPE),
	TYPE_GETTER_AB_F(STR___set_frozen__, &repeat_set_frozen, METHOD_FNOREFESCAPE, "->?DSet"),
	TYPE_GETTER_AB_F(STR___map_keys__, &repeat_map_keys, METHOD_FNOREFESCAPE, "->?DSet"),
	TYPE_GETTER_AB_F(STR___map_values__, &repeat_map_values, METHOD_FNOREFESCAPE, "->?DSequence"),
	TYPE_GETTER_AB_F(STR___map_iterkeys__, &repeat_map_iterkeys, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETTER_AB_F(STR___map_itervalues__, &repeat_map_itervalues, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETTER_AB_F(STR___map_frozen__, &repeat_map_frozen, METHOD_FNOREFESCAPE, "->?DMapping"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst repeat_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT_AB, offsetof(Repeat, rp_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(Repeat, rp_num)),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst repeat_methods[] = {
	//TODO: TYPE_METHOD_HINTREF(Sequence_unpack),
	//TODO: TYPE_METHOD_HINTREF(Sequence_unpackub),
	TYPE_METHOD_HINTREF(Sequence_any),
	TYPE_METHOD_HINTREF(Sequence_all),
	TYPE_METHOD_HINTREF(Sequence_parity),
	//XXX: TYPE_METHOD_HINTREF(Sequence_reduce),
	TYPE_METHOD_HINTREF(Sequence_min),
	TYPE_METHOD_HINTREF(Sequence_max),
	TYPE_METHOD_HINTREF(Sequence_sum),
	TYPE_METHOD_HINTREF(Sequence_count),
	TYPE_METHOD_HINTREF(Sequence_contains),
	TYPE_METHOD_HINTREF(Sequence_locate),
	TYPE_METHOD_HINTREF(Sequence_rlocate),
	//TODO: TYPE_METHOD_HINTREF(Sequence_startswith),
	//TODO: TYPE_METHOD_HINTREF(Sequence_endswith),
	//TODO: TYPE_METHOD_HINTREF(Sequence_find),
	//TODO: TYPE_METHOD_HINTREF(Sequence_rfind),
	//TODO: TYPE_METHOD_HINTREF(Sequence_erase),
	//TODO: TYPE_METHOD_HINTREF(Sequence_insert),
	//TODO: TYPE_METHOD_HINTREF(Sequence_insertall),
	//TODO: TYPE_METHOD_HINTREF(Sequence_pushfront),
	//TODO: TYPE_METHOD_HINTREF(Sequence_append),
	//TODO: TYPE_METHOD_HINTREF(Sequence_extend),
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	//TODO: TYPE_METHOD_HINTREF(Sequence_clear),
	//TODO: TYPE_METHOD_HINTREF(Sequence_pop),
	//TODO: TYPE_METHOD_HINTREF(Sequence_remove),
	//TODO: TYPE_METHOD_HINTREF(Sequence_rremove),
	//TODO: TYPE_METHOD_HINTREF(Sequence_removeall),
	//TODO: TYPE_METHOD_HINTREF(Sequence_removeif),
	//TODO: TYPE_METHOD_HINTREF(Sequence_resize),
	//TODO: TYPE_METHOD_HINTREF(Sequence_fill),
	//TODO: TYPE_METHOD_HINTREF(Sequence_reverse),
	//TODO: TYPE_METHOD_HINTREF(Sequence_reversed),
	//TODO: TYPE_METHOD_HINTREF(Sequence_sort),
	//TODO: TYPE_METHOD_HINTREF(Sequence_sorted),
	//TODO: TYPE_METHOD_HINTREF(Sequence_bfind),
	//TODO: TYPE_METHOD_HINTREF(Sequence_bposition),
	//TODO: TYPE_METHOD_HINTREF(Sequence_brange),

	/* All Set/Mapping method hints here are implemented using `generic_proxy__*`,
	 * which in turn means is equivalent to an implementation:
	 * >> `(this.__obj__ as Set).SOME_FUNCTION(...)`
	 * >> `(this.__obj__ as Mapping).SOME_FUNCTION(...)` */
	TYPE_METHOD_HINTREF(__set_iter__),
	TYPE_METHOD_HINTREF(__set_size__),
	TYPE_METHOD_HINTREF(__set_hash__),
	TYPE_METHOD_HINTREF(__set_compare_eq__),
	TYPE_METHOD_HINTREF(__set_eq__),
	TYPE_METHOD_HINTREF(__set_ne__),
	TYPE_METHOD_HINTREF(__set_lo__),
	TYPE_METHOD_HINTREF(__set_le__),
	TYPE_METHOD_HINTREF(__set_gr__),
	TYPE_METHOD_HINTREF(__set_ge__),
	TYPE_METHOD_HINTREF(__set_bool__),
	TYPE_METHOD_HINTREF(__set_inv__),
	TYPE_METHOD_HINTREF(__set_add__),
	TYPE_METHOD_HINTREF(__set_sub__),
	TYPE_METHOD_HINTREF(__set_and__),
	TYPE_METHOD_HINTREF(__set_xor__),
	//TODO:TYPE_METHOD_HINTREF(__set_inplace_add__),
	//TODO:TYPE_METHOD_HINTREF(__set_inplace_sub__),
	//TODO:TYPE_METHOD_HINTREF(__set_inplace_and__),
	//TODO:TYPE_METHOD_HINTREF(__set_inplace_xor__),
	TYPE_METHOD_HINTREF(__set_unify__),
	TYPE_METHOD_HINTREF(__set_insert__),
	TYPE_METHOD_HINTREF(__set_insertall__),
	TYPE_METHOD_HINTREF(__set_remove__),
	TYPE_METHOD_HINTREF(__set_removeall__),
	TYPE_METHOD_HINTREF(__set_pop__),
	TYPE_METHOD_HINTREF(__map_iter__),
	TYPE_METHOD_HINTREF(__map_size__),
	TYPE_METHOD_HINTREF(__map_hash__),
	TYPE_METHOD_HINTREF(__map_getitem__),
	TYPE_METHOD_HINTREF(__map_delitem__),
	TYPE_METHOD_HINTREF(__map_setitem__),
	TYPE_METHOD_HINTREF(__map_contains__),
	TYPE_METHOD_HINTREF(__map_enumerate__),
	TYPE_METHOD_HINTREF(__map_enumerate_items__),
	TYPE_METHOD_HINTREF(__map_compare_eq__),
	TYPE_METHOD_HINTREF(__map_eq__),
	TYPE_METHOD_HINTREF(__map_ne__),
	TYPE_METHOD_HINTREF(__map_lo__),
	TYPE_METHOD_HINTREF(__map_le__),
	TYPE_METHOD_HINTREF(__map_gr__),
	TYPE_METHOD_HINTREF(__map_ge__),
	TYPE_METHOD_HINTREF(__map_add__),
	TYPE_METHOD_HINTREF(__map_sub__),
	TYPE_METHOD_HINTREF(__map_and__),
	TYPE_METHOD_HINTREF(__map_xor__),
	//TODO:TYPE_METHOD_HINTREF(__map_inplace_add__),
	//TODO:TYPE_METHOD_HINTREF(__map_inplace_sub__),
	//TODO:TYPE_METHOD_HINTREF(__map_inplace_and__),
	//TODO:TYPE_METHOD_HINTREF(__map_inplace_xor__),
	TYPE_METHOD_HINTREF(__map_setold__),
	TYPE_METHOD_HINTREF(__map_setold_ex__),
	TYPE_METHOD_HINTREF(__map_setnew__),
	TYPE_METHOD_HINTREF(__map_setnew_ex__),
	TYPE_METHOD_HINTREF(__map_setdefault__),
	TYPE_METHOD_HINTREF(__map_update__),
	TYPE_METHOD_HINTREF(__map_remove__),
	TYPE_METHOD_HINTREF(__map_removekeys__),
	TYPE_METHOD_HINTREF(__map_pop__),
	TYPE_METHOD_HINTREF(__map_popitem__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst repeat_method_hints[] = {
//	TODO: Dee_TMH_seq_enumerate_index,
//	TODO: Dee_TMH_seq_enumerate_index_reverse,
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &repeat_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &repeat_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &repeat_trygetlast, METHOD_FNOREFESCAPE),
//	TODO: Dee_TMH_seq_unpack,
//	TODO: Dee_TMH_seq_unpack_ex,
//	TODO: Dee_TMH_seq_unpack_ub,
	TYPE_METHOD_HINT_F(seq_any, &repeat_any, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_key, &repeat_any_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &repeat_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &repeat_any_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all, &repeat_all, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &repeat_all_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, &repeat_all_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &repeat_all_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity, &repeat_parity, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_key, &repeat_parity_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range, &repeat_parity_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range_and_key, &repeat_parity_with_range_and_key, METHOD_FNOREFESCAPE),
	//XXX: Dee_TMH_seq_reduce,
	//XXX: Dee_TMH_seq_reduce_with_init,
	//XXX: Dee_TMH_seq_reduce_with_range,
	//XXX: Dee_TMH_seq_reduce_with_range_and_init,
	TYPE_METHOD_HINT_F(seq_min, &repeat_min, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_key, &repeat_min_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range, &repeat_min_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range_and_key, &repeat_min_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max, &repeat_max, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_key, &repeat_max_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range, &repeat_max_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range_and_key, &repeat_max_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum, &repeat_sum, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_key, &repeat_sum_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_range, &repeat_sum_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_range_and_key, &repeat_sum_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count, &repeat_count, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_key, &repeat_count_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range, &repeat_count_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &repeat_count_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains, &repeat_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &repeat_contains_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &repeat_contains_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &repeat_contains_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_locate, &repeat_locate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_locate_with_range, &repeat_locate_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rlocate, &repeat_rlocate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rlocate_with_range, &repeat_rlocate_with_range, METHOD_FNOREFESCAPE),
//	TODO: Dee_TMH_seq_startswith,
//	TODO: Dee_TMH_seq_startswith_with_key,
//	TODO: Dee_TMH_seq_startswith_with_range,
//	TODO: Dee_TMH_seq_startswith_with_range_and_key,
//	TODO: Dee_TMH_seq_endswith,
//	TODO: Dee_TMH_seq_endswith_with_key,
//	TODO: Dee_TMH_seq_endswith_with_range,
//	TODO: Dee_TMH_seq_endswith_with_range_and_key,
//	TODO: Dee_TMH_seq_find,
//	TODO: Dee_TMH_seq_find_with_key,
//	TODO: Dee_TMH_seq_rfind,
//	TODO: Dee_TMH_seq_rfind_with_key,
//	TODO: Dee_TMH_seq_erase,
//	TODO: Dee_TMH_seq_insert,
//	TODO: Dee_TMH_seq_insertall,
//	TODO: Dee_TMH_seq_pushfront,
//	TODO: Dee_TMH_seq_append,
//	TODO: Dee_TMH_seq_extend,
	TYPE_METHOD_HINT_F(seq_xchitem_index, &repeat_xchitem_index, METHOD_FNOREFESCAPE),
//	TODO: Dee_TMH_seq_clear,
//	TODO: Dee_TMH_seq_pop,
//	TODO: Dee_TMH_seq_remove,
//	TODO: Dee_TMH_seq_remove_with_key,
//	TODO: Dee_TMH_seq_rremove,
//	TODO: Dee_TMH_seq_rremove_with_key,
//	TODO: Dee_TMH_seq_removeall,
//	TODO: Dee_TMH_seq_removeall_with_key,
//	TODO: Dee_TMH_seq_removeif,
//	TODO: Dee_TMH_seq_resize,          // if (this.__num__ != 1) throw ValueError(); else this.__seq__.resize();
//	TODO: Dee_TMH_seq_fill,
//	TODO: Dee_TMH_seq_reverse,         // if (this.__num__ != 1) throw ValueError(); else this.__seq__.reverse();
//	TODO: Dee_TMH_seq_reversed,
//	TODO: Dee_TMH_seq_sort,            // if (this.__num__ != 1) throw ValueError(); else this.__seq__.sort();
//	TODO: Dee_TMH_seq_sort_with_key,
//	TODO: Dee_TMH_seq_sorted,          // this.__seq__.sorted().map(e -> Sequence.repeatitem(e, this.__num__)).flatten;
//	TODO: Dee_TMH_seq_sorted_with_key,
//	TODO: Dee_TMH_seq_bfind,
//	TODO: Dee_TMH_seq_bfind_with_key,
//	TODO: Dee_TMH_seq_bposition,
//	TODO: Dee_TMH_seq_bposition_with_key,
//	TODO: Dee_TMH_seq_brange,
//	TODO: Dee_TMH_seq_brange_with_key,
	TYPE_METHOD_HINT_F(set_operator_iter, &repeat_set_operator_iter, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_foreach, &repeat_set_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_sizeob, &repeat_set_operator_sizeob, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_size, &repeat_set_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_hash, &repeat_set_operator_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_compare_eq, &repeat_set_operator_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_trycompare_eq, &repeat_set_operator_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_eq, &repeat_set_operator_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_ne, &repeat_set_operator_ne, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_lo, &repeat_set_operator_lo, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_le, &repeat_set_operator_le, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_gr, &repeat_set_operator_gr, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_ge, &repeat_set_operator_ge, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_bool, &repeat_set_operator_bool, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_inv, &repeat_set_operator_inv, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_add, &repeat_set_operator_add, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_sub, &repeat_set_operator_sub, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_and, &repeat_set_operator_and, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_xor, &repeat_set_operator_xor, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(set_operator_inplace_add, &repeat_set_operator_inplace_add, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(set_operator_inplace_sub, &repeat_set_operator_inplace_sub, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(set_operator_inplace_and, &repeat_set_operator_inplace_and, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(set_operator_inplace_xor, &repeat_set_operator_inplace_xor, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_unify, &repeat_set_unify, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_insert, &repeat_set_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_insertall, &repeat_set_insertall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_remove, &repeat_set_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_removeall, &repeat_set_removeall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop, &repeat_set_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop_with_default, &repeat_set_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_trygetfirst, &repeat_set_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_trygetlast, &repeat_set_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_iter, &repeat_map_operator_iter, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_foreach_pair, &repeat_map_operator_foreach_pair, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_sizeob, &repeat_map_operator_sizeob, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_size, &repeat_map_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_hash, &repeat_map_operator_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_getitem, &repeat_map_operator_getitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_trygetitem, &repeat_map_operator_trygetitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_getitem_index, &repeat_map_operator_getitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_trygetitem_index, &repeat_map_operator_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_getitem_string_hash, &repeat_map_operator_getitem_string_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_trygetitem_string_hash, &repeat_map_operator_trygetitem_string_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_getitem_string_len_hash, &repeat_map_operator_getitem_string_len_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_trygetitem_string_len_hash, &repeat_map_operator_trygetitem_string_len_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_bounditem, &repeat_map_operator_bounditem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_bounditem_index, &repeat_map_operator_bounditem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_bounditem_string_hash, &repeat_map_operator_bounditem_string_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_bounditem_string_len_hash, &repeat_map_operator_bounditem_string_len_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_hasitem, &repeat_map_operator_hasitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_hasitem_index, &repeat_map_operator_hasitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_hasitem_string_hash, &repeat_map_operator_hasitem_string_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_hasitem_string_len_hash, &repeat_map_operator_hasitem_string_len_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_delitem, &repeat_map_operator_delitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_delitem_index, &repeat_map_operator_delitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_delitem_string_hash, &repeat_map_operator_delitem_string_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_delitem_string_len_hash, &repeat_map_operator_delitem_string_len_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_setitem, &repeat_map_operator_setitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_setitem_index, &repeat_map_operator_setitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_setitem_string_hash, &repeat_map_operator_setitem_string_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_setitem_string_len_hash, &repeat_map_operator_setitem_string_len_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_contains, &repeat_map_operator_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_enumerate, &repeat_map_enumerate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_enumerate_range, &repeat_map_enumerate_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_makeenumeration, &repeat_map_makeenumeration, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_makeenumeration_with_range, &repeat_map_makeenumeration_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_compare_eq, &repeat_map_operator_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_trycompare_eq, &repeat_map_operator_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_eq, &repeat_map_operator_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_ne, &repeat_map_operator_ne, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_lo, &repeat_map_operator_lo, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_le, &repeat_map_operator_le, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_gr, &repeat_map_operator_gr, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_ge, &repeat_map_operator_ge, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_add, &repeat_map_operator_add, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_sub, &repeat_map_operator_sub, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_and, &repeat_map_operator_and, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_xor, &repeat_map_operator_xor, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(map_operator_inplace_add, &repeat_map_operator_inplace_add, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(map_operator_inplace_sub, &repeat_map_operator_inplace_sub, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(map_operator_inplace_and, &repeat_map_operator_inplace_and, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(map_operator_inplace_xor, &repeat_map_operator_inplace_xor, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold, &repeat_map_setold, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold_ex, &repeat_map_setold_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew, &repeat_map_setnew, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew_ex, &repeat_map_setnew_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setdefault, &repeat_map_setdefault, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_update, &repeat_map_update, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_remove, &repeat_map_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_removekeys, &repeat_map_removekeys, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop, &repeat_map_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop_with_default, &repeat_map_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_popitem, &repeat_map_popitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst repeat_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqRepeatIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqRepeat_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqRepeat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeat",
	/* .tp_doc      = */ DOC("(" repeat__SeqRepeat_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ Repeat,
			/* tp_ctor:        */ &repeat_ctor,
			/* tp_copy_ctor:   */ &repeat_copy,
			/* tp_any_ctor:    */ &repeat_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &repeat_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&repeat_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeat_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeat_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&repeat_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__067D79DE76BFB81C),
	/* .tp_seq           = */ &repeat_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ repeat_methods,
	/* .tp_getsets       = */ repeat_getsets,
	/* .tp_members       = */ repeat_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ repeat_class_members,
	/* .tp_method_hints  = */ repeat_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};






#define REPEATITEMPITER_READ_NUM(x) atomic_read(&(x)->rii_num)

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitemiter_ctor(RepeatItemIterator *__restrict self) {
	self->rii_rep = (DREF RepeatItem *)DeeSeq_RepeatItem(Dee_None, 0);
	if unlikely(!self->rii_rep)
		goto err;
	self->rii_obj = Dee_None;
	self->rii_num = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitemiter_copy(RepeatItemIterator *__restrict self,
                    RepeatItemIterator *__restrict other) {
	self->rii_num = REPEATITEMPITER_READ_NUM(other);
	self->rii_obj = other->rii_obj;
	self->rii_rep = other->rii_rep;
	Dee_Incref(self->rii_rep);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitemiter_serialize(RepeatItemIterator *__restrict self,
                         DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(RepeatItemIterator, field))
	RepeatItemIterator *out = DeeSerial_Addr2Mem(writer, addr, RepeatItemIterator);
	out->rii_num = REPEATITEMPITER_READ_NUM(self);
	if (DeeSerial_PutObject(writer, ADDROF(rii_rep), self->rii_rep))
		goto err;
	ASSERT(self->rii_rep->rpit_obj == self->rii_obj);
	return DeeSerial_PutPointer(writer, ADDROF(rii_obj), self->rii_obj);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitemiter_init(RepeatItemIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_SeqRepeatItemIterator", params: """
	RepeatItem *repeat:?Ert:SeqRepeatItem
""", docStringPrefix: "repeatitemiter");]]]*/
#define repeatitemiter__SeqRepeatItemIterator_params "repeat:?Ert:SeqRepeatItem"
	struct {
		RepeatItem *repeat;
	} args;
	DeeArg_Unpack1(err, argc, argv, "_SeqRepeatItemIterator", &args.repeat);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.repeat, &SeqRepeatItem_Type))
		goto err;
	Dee_Incref(args.repeat);
	self->rii_rep = args.repeat;
	self->rii_obj = args.repeat->rpit_obj;
	self->rii_num = args.repeat->rpit_num;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitemiter_print(RepeatItemIterator *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Iterator that will repeat object %k another %" PRFuSIZ " times>",
	                        self->rii_obj, atomic_read(&self->rii_num));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitemiter_printrepr(RepeatItemIterator *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "Sequence.repeatitem(%r, %" PRFuSIZ ").operator iter()",
	                        self->rii_obj, atomic_read(&self->rii_num));
}

STATIC_ASSERT(offsetof(RepeatItemIterator, rii_rep) == offsetof(ProxyObject, po_obj));
#define repeatitemiter_fini  generic_proxy__fini
#define repeatitemiter_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
repeatitemiter_hash(RepeatItemIterator *self) {
	return Dee_HashCombine(DeeObject_HashGeneric(self->rii_obj),
	                       REPEATITEMPITER_READ_NUM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitemiter_compare(RepeatItemIterator *lhs,
                       RepeatItemIterator *rhs) {
	if (DeeObject_AssertTypeExact(rhs, &SeqRepeatItemIterator_Type))
		goto err;
	Dee_return_compare_if_ne(lhs->rii_obj, rhs->rii_obj);
	/* Compare "num" in reverse (because "num" becomes less as the iterator is advanced) */
	Dee_return_compareT(size_t, REPEATITEMPITER_READ_NUM(rhs),
	                    /*   */ REPEATITEMPITER_READ_NUM(lhs));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp repeatitemiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&repeatitemiter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitemiter_next(RepeatItemIterator *__restrict self) {
	size_t count;
	do {
		count = REPEATITEMPITER_READ_NUM(self);
		if (count == 0)
			return ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->rii_num, count, count - 1));
	return_reference_(self->rii_obj);
}

PRIVATE struct type_member tpconst repeatitemiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(RepeatItemIterator, rii_rep), "->?Ert:SeqRepeatItem"),
	TYPE_MEMBER_FIELD("__obj__", STRUCT_OBJECT_AB, offsetof(RepeatItemIterator, rii_obj)),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T, offsetof(RepeatItemIterator, rii_num)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqRepeatItemIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatItemIterator",
	/* .tp_doc      = */ DOC("(" repeatitemiter__SeqRepeatItemIterator_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ RepeatItemIterator,
			/* tp_ctor:        */ &repeatitemiter_ctor,
			/* tp_copy_ctor:   */ &repeatitemiter_copy,
			/* tp_any_ctor:    */ &repeatitemiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &repeatitemiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeatitemiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__iter_operator_bool__with__iter_peek),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeatitemiter_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeatitemiter_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&repeatitemiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &repeatitemiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatitemiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ repeatitemiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_ctor(RepeatItem *__restrict self) {
	self->rpit_num = 0;
	self->rpit_obj = DeeNone_NewRef();
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_copy(RepeatItem *__restrict self,
                RepeatItem *__restrict other) {
	self->rpit_num = other->rpit_num;
	self->rpit_obj = other->rpit_obj;
	Dee_Incref(self->rpit_obj);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_init(RepeatItem *__restrict self,
                size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_SeqRepeatItem", params: """
	obj, size_t num
""", docStringPrefix: "repeatitem");]]]*/
#define repeatitem__SeqRepeatItem_params "obj,num:?Dint"
	struct {
		DeeObject *obj;
		size_t num;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "_SeqRepeatItem", &args, &args.obj, "o", _DeeArg_AsObject, &args.num, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	Dee_Incref(args.obj);
	self->rpit_obj = args.obj;
	self->rpit_num = args.num;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(RepeatItem, rpit_obj) == offsetof(ProxyObject, po_obj));
#define repeatitem_serialize generic_proxy__serialize_and_memcpy
#define repeatitem_fini      generic_proxy__fini
#define repeatitem_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_bool(RepeatItem *__restrict self) {
	return Dee_HAS_FROMBOOL(self->rpit_num != 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_print(RepeatItem *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Object %k repeated %" PRFuSIZ " times>",
	                        self->rpit_obj, self->rpit_num);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_printrepr(RepeatItem *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "Sequence.repeatitem(%r, %" PRFuSIZ ")",
	                        self->rpit_obj, self->rpit_num);
}

PRIVATE WUNUSED NONNULL((1)) DREF RepeatItemIterator *DCALL
repeatitem_iter(RepeatItem *__restrict self) {
	DREF RepeatItemIterator *result;
	result = DeeObject_MALLOC(RepeatItemIterator);
	if unlikely(!result)
		goto done;
	result->rii_rep = self;
	result->rii_obj = self->rpit_obj;
	result->rii_num = self->rpit_num;
	Dee_Incref(self);
	DeeObject_InitStatic(result, &SeqRepeatItemIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitem_operator_contains(RepeatItem *self, DeeObject *item) {
	if unlikely(self->rpit_num == 0)
		return_false;
	return DeeObject_CmpEq(self->rpit_obj, item);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_size(RepeatItem *__restrict self) {
	if unlikely(self->rpit_num == (size_t)-1)
		DeeRT_ErrIntegerOverflowU(self->rpit_num, (size_t)-2);
	return self->rpit_num;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_size_fast(RepeatItem *__restrict self) {
	return self->rpit_num;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getitem_index(RepeatItem *__restrict self, size_t index) {
	if unlikely(index >= self->rpit_num)
		goto err_bounds;
	return_reference_(self->rpit_obj);
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rpit_num);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
repeatitem_getitem_index_fast(RepeatItem *__restrict self, size_t index) {
	(void)index;
	return_reference_(self->rpit_obj);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getrange_index(RepeatItem *__restrict self,
                          Dee_ssize_t i_begin,
                          Dee_ssize_t i_end) {
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->rpit_num);
	return DeeSeq_RepeatItem(self->rpit_obj, range.sr_end - range.sr_start);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getrange_index_n(RepeatItem *__restrict self,
                            Dee_ssize_t i_begin) {
	size_t start = DeeSeqRange_Clamp_n(i_begin, self->rpit_num);
	return DeeSeq_RepeatItem(self->rpit_obj, self->rpit_num - start);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_foreach(RepeatItem *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->rpit_num; ++i) {
		temp = (*proc)(arg, self->rpit_obj);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err; /* For very high `rpit_num` */
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_foreach_pair(RepeatItem *self, Dee_foreach_pair_t proc, void *arg) {
	DREF DeeObject *pair[2];
	Dee_ssize_t temp, result = 0;
	if likely(self->rpit_num) {
		size_t i = 0;
		if (DeeSeq_Unpack(self->rpit_obj, 2, pair))
			goto err;
		do {
			temp = (*proc)(arg, pair[0], pair[1]);
			if unlikely(temp < 0)
				goto err_pair_temp;
			result += temp;
			if (DeeThread_CheckInterrupt())
				goto err; /* For very high `rpit_num` */
		} while (++i < self->rpit_num);
		Dee_Decrefv(pair, 2);
	}
	return result;
err_pair_temp:
	Dee_Decrefv(pair, 2);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_enumerate_index(RepeatItem *self, Dee_seq_enumerate_index_t cb,
                           void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->rpit_num)
		end = self->rpit_num;
	for (i = start; i < end; ++i) {
		temp = (*cb)(arg, i, self->rpit_obj);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err; /* For very high `rpit_num` */
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_enumerate_index_reverse(RepeatItem *self, Dee_seq_enumerate_index_t cb,
                                   void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->rpit_num)
		end = self->rpit_num;
	i = end;
	while (i > start) {
		--i;
		temp = (*cb)(arg, i, self->rpit_obj);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err; /* For very high `rpit_num` */
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_asvector(RepeatItem *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if unlikely(self->rpit_num == (size_t)-1) {
		DeeRT_ErrIntegerOverflowU(self->rpit_num, (size_t)-2);
	} else {
		if likely(dst_length >= self->rpit_num)
			Dee_Setrefv(dst, self->rpit_obj, self->rpit_num);
	}
	return self->rpit_num;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_asvector_nothrow(RepeatItem *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if likely(dst_length >= self->rpit_num)
		Dee_Setrefv(dst, self->rpit_obj, self->rpit_num);
	return self->rpit_num;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_anyall(RepeatItem *__restrict self, int empty_value) {
	return self->rpit_num ? DeeObject_Bool(self->rpit_obj) : empty_value;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_anyall_with_key(RepeatItem *self, DeeObject *key, int empty_value) {
	DREF DeeObject *keyed;
	if (!self->rpit_num)
		return empty_value;
	keyed = DeeObject_Call(key, 1, &self->rpit_obj);
	if unlikely(!keyed)
		goto err;
	return DeeObject_BoolInherited(keyed);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_anyall_with_range(RepeatItem *__restrict self,
                             size_t start, size_t end,
                             int empty_value) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	return (start < end) ? DeeObject_Bool(self->rpit_obj) : empty_value;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeatitem_anyall_with_range_and_key(RepeatItem *__restrict self,
                                     size_t start, size_t end,
                                     DeeObject *key, int empty_value) {
	DREF DeeObject *keyed;
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return empty_value;
	keyed = DeeObject_Call(key, 1, &self->rpit_obj);
	if unlikely(!keyed)
		goto err;
	return DeeObject_BoolInherited(keyed);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_any(RepeatItem *__restrict self) {
	return repeatitem_anyall(self, 0);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_all(RepeatItem *__restrict self) {
	return repeatitem_anyall(self, 1);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_any_with_key(RepeatItem *self, DeeObject *key) {
	return repeatitem_anyall_with_key(self, key, 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_all_with_key(RepeatItem *self, DeeObject *key) {
	return repeatitem_anyall_with_key(self, key, 1);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_any_with_range(RepeatItem *self, size_t start, size_t end) {
	return repeatitem_anyall_with_range(self, start, end, 0);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_all_with_range(RepeatItem *self, size_t start, size_t end) {
	return repeatitem_anyall_with_range(self, start, end, 1);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeatitem_any_with_range_and_key(RepeatItem *self, size_t start, size_t end, DeeObject *key) {
	return repeatitem_anyall_with_range_and_key(self, start, end, key, 0);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeatitem_all_with_range_and_key(RepeatItem *self, size_t start, size_t end, DeeObject *key) {
	return repeatitem_anyall_with_range_and_key(self, start, end, key, 1);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_parity(RepeatItem *__restrict self) {
	if (!(self->rpit_num & 1))
		return 0; /* Even total -> paritiy is always = 0 */
	return repeatitem_any(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_parity_with_key(RepeatItem *self, DeeObject *key) {
	if (!(self->rpit_num & 1))
		return 0; /* Even total -> paritiy is always = 0 */
	return repeatitem_any_with_key(self, key);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_parity_with_range(RepeatItem *self, size_t start, size_t end) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return 0; /* Empty range -> paritiy is always = 0 */
	if (!((end - start) & 1))
		return 0; /* Even total -> paritiy is always = 0 */
	return DeeObject_Bool(self->rpit_obj);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeatitem_parity_with_range_and_key(RepeatItem *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *keyed;
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return 0; /* Empty range -> paritiy is always = 0 */
	if (!((end - start) & 1))
		return 0; /* Even total -> paritiy is always = 0 */
	keyed = DeeObject_Call(key, 1, &self->rpit_obj);
	if unlikely(!keyed)
		goto err;
	return DeeObject_BoolInherited(keyed);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitem_minmax(RepeatItem *self, DeeObject *def) {
	return_reference(self->rpit_num ? self->rpit_obj : def);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
repeatitem_minmax_with_key(RepeatItem *self, DeeObject *def, DeeObject *key) {
	(void)key;
	return repeatitem_minmax(self, def);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeatitem_minmax_with_range(RepeatItem *self, size_t start, size_t end, DeeObject *def) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	return_reference((start < end) ? self->rpit_obj : def);
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
repeatitem_minmax_with_range_and_key(RepeatItem *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	(void)key;
	return repeatitem_minmax_with_range(self, start, end, def);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
generic_object_repeat_sum_with_key(DeeObject *obj, size_t count, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *keyed_obj = DeeObject_Call(obj, 1, &key);
	if unlikely(!keyed_obj)
		goto err;
	result = generic_object_repeat_sum(keyed_obj, count);
	Dee_Decref(keyed_obj);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitem_sum(RepeatItem *self, DeeObject *def) {
	if (self->rpit_num == 0)
		return_reference_(def);
	return generic_object_repeat_sum(self->rpit_obj, self->rpit_num);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
repeatitem_sum_with_key(RepeatItem *self, DeeObject *def, DeeObject *key) {
	if (self->rpit_num == 0)
		return_reference_(def);
	return generic_object_repeat_sum_with_key(self->rpit_obj, self->rpit_num, key);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeatitem_sum_with_range(RepeatItem *self, size_t start, size_t end, DeeObject *def) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return_reference_(def);
	return generic_object_repeat_sum(self->rpit_obj, end - start);
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
repeatitem_sum_with_range_and_key(RepeatItem *self, size_t start, size_t end,
                                  DeeObject *def, DeeObject *key) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return_reference_(def);
	return generic_object_repeat_sum_with_key(self->rpit_obj, end - start, key);
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeatitem_count_impl(RepeatItem *self, DeeObject *item, size_t eq_count) {
	int eq = DeeObject_CompareEq(item, self->rpit_obj);
	if (Dee_COMPARE_ISERR(eq))
		goto err;
	if (Dee_COMPARE_ISNE(eq))
		return 0;
	return eq_count;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) size_t DCALL
repeatitem_count_with_key_impl(RepeatItem *self, DeeObject *item, DeeObject *key, size_t eq_count) {
	int eq;
	DREF DeeObject *keyed_item;
	keyed_item = DeeObject_Call(key, 1, &item);
	if unlikely(!keyed_item)
		goto err;
	eq = DeeObject_CompareKeyEq(keyed_item, self->rpit_obj, key);
	Dee_Decref(keyed_item);
	if (Dee_COMPARE_ISERR(eq))
		goto err;
	if (Dee_COMPARE_ISNE(eq))
		return 0;
	return eq_count;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeatitem_count(RepeatItem *self, DeeObject *item) {
	if (self->rpit_num == 0)
		return 0;
	return repeatitem_count_impl(self, item, self->rpit_num);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) size_t DCALL
repeatitem_count_with_key(RepeatItem *self, DeeObject *item, DeeObject *key) {
	if (self->rpit_num == 0)
		return 0;
	return repeatitem_count_with_key_impl(self, item, key, self->rpit_num);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeatitem_count_with_range(RepeatItem *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return 0;
	return repeatitem_count_impl(self, item, end - start);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
repeatitem_count_with_range_and_key(RepeatItem *self, DeeObject *item,
                                    size_t start, size_t end,
                                    DeeObject *key) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return 0;
	return repeatitem_count_with_key_impl(self, item, key, end - start);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_contains_impl(RepeatItem *self, DeeObject *item) {
	int eq = DeeObject_CompareEq(item, self->rpit_obj);
	return Dee_HAS_FROM_COMPARE_EQ(eq);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
repeatitem_contains_with_key_impl(RepeatItem *self, DeeObject *item, DeeObject *key) {
	int eq;
	DREF DeeObject *keyed_item;
	keyed_item = DeeObject_Call(key, 1, &item);
	if unlikely(!keyed_item)
		goto err;
	eq = DeeObject_CompareKeyEq(keyed_item, self->rpit_obj, key);
	Dee_Decref(keyed_item);
	return Dee_HAS_FROM_COMPARE_EQ(eq);
err:
	return Dee_HAS_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_contains(RepeatItem *self, DeeObject *item) {
	if (self->rpit_num == 0)
		return Dee_HAS_NO;
	return repeatitem_contains_impl(self, item);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
repeatitem_contains_with_key(RepeatItem *self, DeeObject *item, DeeObject *key) {
	if (self->rpit_num == 0)
		return Dee_HAS_NO;
	return repeatitem_contains_with_key_impl(self, item, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_contains_with_range(RepeatItem *self, DeeObject *item, size_t start, size_t end) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return Dee_HAS_NO;
	return repeatitem_contains_impl(self, item);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
repeatitem_contains_with_range_and_key(RepeatItem *self, DeeObject *item,
                                    size_t start, size_t end,
                                    DeeObject *key) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return Dee_HAS_NO;
	return repeatitem_contains_with_key_impl(self, item, key);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
repeatitem_locate_impl(RepeatItem *self, DeeObject *match, DeeObject *def) {
	int match_bool;
	DREF DeeObject *match_result = DeeObject_Call(match, 1, &self->rpit_obj);
	if unlikely(!match_result)
		goto err;
	match_bool = DeeObject_BoolInherited(match_result);
	if (Dee_HAS_ISERR(match_bool))
		goto err;
	return_reference(match_bool ? self->rpit_obj : def);
err:
	return NULL;
}

#define repeatitem_rlocate repeatitem_locate
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
repeatitem_locate(RepeatItem *self, DeeObject *match, DeeObject *def) {
	if (self->rpit_num == 0)
		return_reference_(def);
	return repeatitem_locate_impl(self, match, def);
}

#define repeatitem_rlocate_with_range repeatitem_locate_with_range
PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
repeatitem_locate_with_range(RepeatItem *self, DeeObject *match,
                             size_t start, size_t end, DeeObject *def) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start >= end)
		return_reference_(def);
	return repeatitem_locate_impl(self, match, def);
}

#define repeatitem_startswith                    repeatitem_contains
#define repeatitem_startswith_with_key           repeatitem_contains_with_key
#define repeatitem_startswith_with_range         repeatitem_contains_with_range
#define repeatitem_startswith_with_range_and_key repeatitem_contains_with_range_and_key
#define repeatitem_endswith                      repeatitem_contains
#define repeatitem_endswith_with_key             repeatitem_contains_with_key
#define repeatitem_endswith_with_range           repeatitem_contains_with_range
#define repeatitem_endswith_with_range_and_key   repeatitem_contains_with_range_and_key



#define repeatitem_bfind repeatitem_find
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeatitem_find(RepeatItem *self, DeeObject *item, size_t start, size_t end) {
	int contains = repeatitem_contains_with_range(self, item, start, end);
	if (Dee_HAS_ISERR(contains))
		return (size_t)Dee_COMPARE_ERR;
	if (Dee_HAS_ISNO(contains))
		return (size_t)-1;
	return 0;
}

#define repeatitem_bfind_with_key repeatitem_find_with_key
PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
repeatitem_find_with_key(RepeatItem *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int contains = repeatitem_contains_with_range_and_key(self, item, start, end, key);
	if (Dee_HAS_ISERR(contains))
		return (size_t)Dee_COMPARE_ERR;
	if (Dee_HAS_ISNO(contains))
		return (size_t)-1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeatitem_rfind(RepeatItem *self, DeeObject *item, size_t start, size_t end) {
	int contains = repeatitem_contains_with_range(self, item, start, end);
	if (Dee_HAS_ISERR(contains))
		return (size_t)Dee_COMPARE_ERR;
	if (Dee_HAS_ISNO(contains))
		return (size_t)-1;
	if (end > self->rpit_num)
		end = self->rpit_num;
	ASSERT(end >= 1);
	--end;
	if (end <= (size_t)Dee_COMPARE_ERR)
		return end;
	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR);
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
repeatitem_rfind_with_key(RepeatItem *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int contains = repeatitem_contains_with_range_and_key(self, item, start, end, key);
	if (Dee_HAS_ISERR(contains))
		return (size_t)Dee_COMPARE_ERR;
	if (Dee_HAS_ISNO(contains))
		return (size_t)-1;
	if (end > self->rpit_num)
		end = self->rpit_num;
	ASSERT(end >= 1);
	--end;
	if (end <= (size_t)Dee_COMPARE_ERR)
		return end;
	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR);
	return (size_t)Dee_COMPARE_ERR;
}

#define repeatitem_sorted repeatitem_reversed
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_reversed(RepeatItem *self, size_t start, size_t end) {
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start > end)
		start = end;
	return DeeSeq_RepeatItem(self->rpit_obj, end - start);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
repeatitem_sorted_with_key(RepeatItem *self, size_t start, size_t end, DeeObject *key) {
	(void)key;
	return repeatitem_sorted(self, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
repeatitem_bposition(RepeatItem *self, DeeObject *item, size_t start, size_t end) {
	size_t result;
	/* Must return the index where "item" would need to be inserted. */
	if (end > self->rpit_num)
		end = self->rpit_num;
	result = start;
	if (start < end) {
		int cmp = DeeObject_Compare(item, self->rpit_obj);
		if (Dee_COMPARE_ISERR(cmp))
			goto err;
		/* If "item > __obj__", then would need to insert at "end" instead */
		if (Dee_COMPARE_ISGR(cmp))
			result = end;
	}
	/* Prevent overflow into error signal */
	if (result >= (size_t)Dee_COMPARE_ERR) {
		DeeRT_ErrIntegerOverflowU(result, (size_t)Dee_COMPARE_ERR);
		goto err;
	}
	return result;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
repeatitem_bposition_with_key(RepeatItem *self, DeeObject *item,
                              size_t start, size_t end, DeeObject *key) {
	size_t result;
	/* Must return the index where "item" would need to be inserted. */
	if (end > self->rpit_num)
		end = self->rpit_num;
	result = start;
	if (start < end) {
		int cmp;
		DREF DeeObject *keyed_item;
		keyed_item = DeeObject_Call(key, 1, &item);
		if unlikely(!keyed_item)
			goto err;
		cmp = DeeObject_CompareKey(keyed_item, self->rpit_obj, key);
		Dee_Decref(keyed_item);
		if (Dee_COMPARE_ISERR(cmp))
			goto err;
		/* If "item > __obj__", then would need to insert at "end" instead */
		if (Dee_COMPARE_ISGR(cmp))
			result = end;
	}
	/* Prevent overflow into error signal */
	if (result >= (size_t)Dee_COMPARE_ERR) {
		DeeRT_ErrIntegerOverflowU(result, (size_t)Dee_COMPARE_ERR);
		goto err;
	}
	return result;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
repeatitem_brange(RepeatItem *self, DeeObject *item,
                  size_t start, size_t end, size_t result_range[2]) {
	/* Must return the index where "item" would need to be inserted. */
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start > end)
		start = end;
	result_range[0] = start;
	result_range[1] = end;
	if (start < end) {
		int cmp = DeeObject_Compare(item, self->rpit_obj);
		if (Dee_COMPARE_ISERR(cmp))
			goto err;
		if (Dee_COMPARE_ISLO(cmp)) {
			result_range[1] = start; /* Would need to insert *before* range */
		} else if (Dee_COMPARE_ISGR(cmp)) {
			result_range[0] = end; /* Would need to insert *after* range */
		} else {
			/* Identical -> item range is described as-is */
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
repeatitem_brange_with_key(RepeatItem *self, DeeObject *item,
                           size_t start, size_t end, DeeObject *key,
                           size_t result_range[2]) {
	/* Must return the index where "item" would need to be inserted. */
	if (end > self->rpit_num)
		end = self->rpit_num;
	if (start > end)
		start = end;
	result_range[0] = start;
	result_range[1] = end;
	if (start < end) {
		int cmp;
		DREF DeeObject *keyed_item;
		keyed_item = DeeObject_Call(key, 1, &item);
		if unlikely(!keyed_item)
			goto err;
		cmp = DeeObject_CompareKey(keyed_item, self->rpit_obj, key);
		Dee_Decref(keyed_item);
		if (Dee_COMPARE_ISERR(cmp))
			goto err;
		if (Dee_COMPARE_ISLO(cmp)) {
			result_range[1] = start; /* Would need to insert *before* range */
		} else if (Dee_COMPARE_ISGR(cmp)) {
			result_range[0] = end; /* Would need to insert *after* range */
		} else {
			/* Identical -> item range is described as-is */
		}
	}
	return 0;
err:
	return -1;
}

#define repeatitem_map_operator_iter repeatitem_set_operator_iter
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_set_operator_iter(RepeatItem *__restrict self) {
	if (self->rpit_num == 0)
		return DeeIterator_NewEmpty();
	{
#ifdef CONFIG_ENABLE_SEQ_ONE_TYPE
		DREF SeqOneIterator *result = DeeObject_MALLOC(SeqOneIterator);
		if unlikely(!result)
			goto err;
		result->soi_item = self->rpit_obj;
		Dee_Incref(result->soi_item);
		DeeObject_InitStatic(result, &SeqOneIterator_Type);
		return Dee_AsObject(result);
#else /* CONFIG_ENABLE_SEQ_ONE_TYPE */
		DREF DeeObject *iter;
		DREF DeeObject *tuple;
		tuple = DeeTuple_NewVector(1, &self->rpit_obj);
		if unlikely(!tuple)
			goto err;
		iter = DeeObject_Iter(tuple);
		Dee_Decref_unlikely(tuple);
		return iter;
#endif /* !CONFIG_ENABLE_SEQ_ONE_TYPE */
	}
err:
	return NULL;
}



PRIVATE struct type_seq repeatitem_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatitem_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitem_operator_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&repeatitem_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&repeatitem_foreach_pair,
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&repeatitem_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&repeatitem_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeatitem_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeatitem_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&repeatitem_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&repeatitem_getrange_index_n,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&repeatitem_asvector,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&repeatitem_asvector_nothrow,
};

PRIVATE struct type_member tpconst repeatitem_members[] = {
	TYPE_MEMBER_FIELD("__obj__", STRUCT_OBJECT_AB, offsetof(RepeatItem, rpit_obj)),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(RepeatItem, rpit_num)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getelem_if_nonempty(RepeatItem *__restrict self) {
	if (self->rpit_num == 0)
		return DeeRT_ErrUnboundAttrCStr(self, "__elem__");
	return_reference_(self->rpit_obj);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_boundelem_if_nonempty(RepeatItem *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->rpit_num != 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_trygetelem_if_nonempty(RepeatItem *__restrict self) {
	if (self->rpit_num == 0)
		return ITER_DONE;
	return_reference_(self->rpit_obj);
}

PRIVATE struct type_getset tpconst repeatitem_getsets[] = {
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_BOUND_F("__elem__", &repeatitem_getelem_if_nonempty, &repeatitem_boundelem_if_nonempty, METHOD_FNOREFESCAPE,
	                    "Same as ?#__obj__, but raises :UnboundAttribute if ?#__num__ is zero"),
	TYPE_GETTER_BOUND_F(STR_first, &repeatitem_getelem_if_nonempty, &repeatitem_boundelem_if_nonempty, METHOD_FNOREFESCAPE, "Alias for ?#__elem__"),
	TYPE_GETTER_BOUND_F(STR_last, &repeatitem_getelem_if_nonempty, &repeatitem_boundelem_if_nonempty, METHOD_FNOREFESCAPE, "Alias for ?#__elem__"),
	TYPE_GETTER_BOUND_F(STR___set_first__, &repeatitem_getelem_if_nonempty, &repeatitem_boundelem_if_nonempty, METHOD_FNOREFESCAPE, "Alias for ?#__elem__"),
	TYPE_GETTER_BOUND_F(STR___set_last__, &repeatitem_getelem_if_nonempty, &repeatitem_boundelem_if_nonempty, METHOD_FNOREFESCAPE, "Alias for ?#__elem__"),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst repeatitem_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(Sequence_any),
	TYPE_METHOD_HINTREF(Sequence_all),
	TYPE_METHOD_HINTREF(Sequence_parity),
	//XXX: TYPE_METHOD_HINTREF(Sequence_reduce),
	TYPE_METHOD_HINTREF(Sequence_min),
	TYPE_METHOD_HINTREF(Sequence_max),
	TYPE_METHOD_HINTREF(Sequence_sum),
	TYPE_METHOD_HINTREF(Sequence_count),
	TYPE_METHOD_HINTREF(Sequence_contains),
	TYPE_METHOD_HINTREF(Sequence_locate),
	TYPE_METHOD_HINTREF(Sequence_rlocate),
	TYPE_METHOD_HINTREF(Sequence_startswith),
	TYPE_METHOD_HINTREF(Sequence_endswith),
	TYPE_METHOD_HINTREF(Sequence_find),
	TYPE_METHOD_HINTREF(Sequence_rfind),
	TYPE_METHOD_HINTREF(Sequence_reversed),
	TYPE_METHOD_HINTREF(Sequence_sorted),
	TYPE_METHOD_HINTREF(Sequence_bfind),
	TYPE_METHOD_HINTREF(Sequence_bposition),
	TYPE_METHOD_HINTREF(Sequence_brange),
	TYPE_METHOD_HINTREF(__set_iter__),
	TYPE_METHOD_HINTREF(__map_iter__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst repeatitem_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &repeatitem_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &repeatitem_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &repeatitem_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &repeatitem_trygetelem_if_nonempty, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &repeatitem_trygetelem_if_nonempty, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any, &repeatitem_any, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_key, &repeatitem_any_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &repeatitem_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &repeatitem_any_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all, &repeatitem_all, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &repeatitem_all_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, &repeatitem_all_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &repeatitem_all_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity, &repeatitem_parity, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_key, &repeatitem_parity_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range, &repeatitem_parity_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range_and_key, &repeatitem_parity_with_range_and_key, METHOD_FNOREFESCAPE),
	//XXX: Dee_TMH_seq_reduce,
	//XXX: Dee_TMH_seq_reduce_with_init,
	//XXX: Dee_TMH_seq_reduce_with_range,
	//XXX: Dee_TMH_seq_reduce_with_range_and_init,
	TYPE_METHOD_HINT_F(seq_min, &repeatitem_minmax, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_key, &repeatitem_minmax_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range, &repeatitem_minmax_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range_and_key, &repeatitem_minmax_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max, &repeatitem_minmax, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_key, &repeatitem_minmax_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range, &repeatitem_minmax_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range_and_key, &repeatitem_minmax_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum, &repeatitem_sum, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_key, &repeatitem_sum_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_range, &repeatitem_sum_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_range_and_key, &repeatitem_sum_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count, &repeatitem_count, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_key, &repeatitem_count_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range, &repeatitem_count_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &repeatitem_count_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains, &repeatitem_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &repeatitem_contains_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &repeatitem_contains_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &repeatitem_contains_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_locate, &repeatitem_locate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_locate_with_range, &repeatitem_locate_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rlocate, &repeatitem_rlocate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rlocate_with_range, &repeatitem_rlocate_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith, &repeatitem_startswith, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_key, &repeatitem_startswith_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_range, &repeatitem_startswith_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_range_and_key, &repeatitem_startswith_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith, &repeatitem_endswith, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_key, &repeatitem_endswith_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_range, &repeatitem_endswith_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_range_and_key, &repeatitem_endswith_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find, &repeatitem_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &repeatitem_find_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &repeatitem_rfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &repeatitem_rfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reversed, &repeatitem_reversed, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted, &repeatitem_sorted, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted_with_key, &repeatitem_sorted_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bfind, &repeatitem_bfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bfind_with_key, &repeatitem_bfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bposition, &repeatitem_bposition, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bposition_with_key, &repeatitem_bposition_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_brange, &repeatitem_brange, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_brange_with_key, &repeatitem_brange_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_iter, &repeatitem_set_operator_iter, METHOD_FNOREFESCAPE),
	/* Other set operators should get default impls using `set_operator_iter` */
	TYPE_METHOD_HINT_F(set_trygetfirst, &repeatitem_trygetelem_if_nonempty, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_trygetlast, &repeatitem_trygetelem_if_nonempty, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_iter, &repeatitem_map_operator_iter, METHOD_FNOREFESCAPE),
	/* Other map operators should get default impls using `map_operator_iter` */
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst repeatitem_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqRepeatItemIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqRepeatItem_Type),
	TYPE_MEMBER_CONST(STR___seq_getitem_always_bound__, Dee_True),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqRepeatItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatItem",
	/* .tp_doc      = */ DOC("(" repeatitem__SeqRepeatItem_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ RepeatItem,
			/* tp_ctor:        */ &repeatitem_ctor,
			/* tp_copy_ctor:   */ &repeatitem_copy,
			/* tp_any_ctor:    */ &repeatitem_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &repeatitem_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeatitem_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&repeatitem_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeatitem_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&repeatitem_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&repeatitem_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__D435E2F1295276B1),
	/* .tp_seq           = */ &repeatitem_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ repeatitem_methods,
	/* .tp_getsets       = */ repeatitem_getsets,
	/* .tp_members       = */ repeatitem_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ repeatitem_class_members,
	/* .tp_method_hints  = */ repeatitem_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



/* Construct new repetition-proxy-sequence objects. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Repeat(DeeObject *__restrict self, size_t count) {
	DREF Repeat *result;
	if (!count || DeeObject_SizeFast(self) == 0)
		return_reference_(Dee_EmptySeq);
	result = DeeObject_MALLOC(Repeat);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	result->rp_seq = self;
	result->rp_num = count;
	DeeObject_InitStatic(result, &SeqRepeat_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatItem(DeeObject *__restrict item, size_t count) {
	DREF RepeatItem *result;
	if unlikely(!count)
		return DeeSeq_NewEmpty();
	result = DeeObject_MALLOC(RepeatItem);
	if unlikely(!result)
		goto done;
	Dee_Incref(item);
	result->rpit_obj = item;
	result->rpit_num = count;
	DeeObject_InitStatic(result, &SeqRepeatItem_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatForever(DeeObject *__restrict self) {
	/* XXX: Dedicated sequence-proxy type? */
	return DeeSeq_Repeat(self, (size_t)-1);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatItemForever(DeeObject *__restrict item) {
	/* XXX: Dedicated sequence-proxy type? */
	return DeeSeq_RepeatItem(item, (size_t)-1);
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_REPEAT_C */
