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

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1, DeeArg_UnpackStruct2X, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>               /* Dee_True, return_false */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Printf, PRFuSIZ */
#include <deemon/gc.h>                 /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/int.h>                /* DeeInt_NewSize, DeeInt_NewZero */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint, Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* DeeNone_NewRef, Dee_None */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_FROMBOOL, Dee_COMPARE_*, Dee_Decref*, Dee_HAS_*, Dee_Incref, Dee_Setrefv, Dee_TYPE, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compareT, Dee_return_compare_if_ne, Dee_ssize_t, ITER_DONE, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeNO_foreach_pair_t, DeeNO_foreach_t, DeeType_RequireNativeOperator */
#include <deemon/pair.h>               /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#include <deemon/seq.h>                /* DeeIterator_NewEmpty, DeeIterator_Type, DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_*, Dee_EmptySeq, Dee_seq_range */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/thread.h>             /* DeeThread_CheckInterrupt */
#include <deemon/tuple.h>              /* DeeTuple_NewVector */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */
#include <deemon/util/hash.h>          /* DeeObject_HashGeneric, Dee_HashCombine */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */

#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */
#include <hybrid/overflow.h>  /* OVERFLOW_UMUL */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "one.h"
#include "repeat.h"

#include <stddef.h> /* NULL, offsetof, size_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

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
#define repeat_contains generic_proxy__seq_operator_contains

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_getitem(Repeat *self, DeeObject *index_ob) {
	/* FIXME: Don't use C types here -- must use deemon objects! */
	size_t index;
	size_t seq_size;
	if unlikely(DeeObject_AsSize(index_ob, &index))
		goto err_maybe_overflow;
	seq_size = DeeObject_Size(self->rp_seq);
	if unlikely(seq_size == (size_t)-1)
		goto err;
	if unlikely(index >= seq_size * self->rp_num) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index,
		                          seq_size * self->rp_num);
		goto err;
	}
	index %= seq_size;
	return DeeObject_GetItemIndex(self->rp_seq, index);
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_size(Repeat *__restrict self) {
	size_t result;
	size_t base_size;
	if unlikely(self->rp_num == 0)
		return 0;
	base_size = DeeObject_Size(self->rp_seq);
	if unlikely(base_size == (size_t)-1)
		return (size_t)-1;
	if (OVERFLOW_UMUL(base_size, self->rp_num, &result)) {
		DeeRT_ErrIntegerOverflowUAdd(base_size, self->rp_num);
		return (size_t)-1;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_sizeob(Repeat *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *base_size;
	DREF DeeObject *multiplier;
	if unlikely(self->rp_num == 0)
		return DeeInt_NewZero();
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_getitem_index(Repeat *__restrict self, size_t index) {
	size_t seq_size;
	seq_size = DeeObject_Size(self->rp_seq);
	if unlikely(seq_size == (size_t)-1)
		goto err;
	if unlikely(index >= seq_size * self->rp_num) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index,
		                          seq_size * self->rp_num);
		goto err;
	}
	index %= seq_size;
	return DeeObject_GetItemIndex(self->rp_seq, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach(Repeat *__restrict self, Dee_foreach_t proc, void *arg) {
	DeeTypeObject *tp_seq;
	Dee_ssize_t temp, result;
	DeeNO_foreach_t foreach;
	size_t i;
	tp_seq  = Dee_TYPE(self->rp_seq);
	foreach = DeeType_RequireNativeOperator(tp_seq, foreach);
	i = 0;
	result = 0;
	do {
		temp = (*foreach)(self->rp_seq, proc, arg);
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
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach_pair(Repeat *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	DeeTypeObject *tp_seq;
	Dee_ssize_t temp, result;
	DeeNO_foreach_pair_t foreach_pair;
	size_t i;
	tp_seq  = Dee_TYPE(self->rp_seq);
	foreach_pair = DeeType_RequireNativeOperator(tp_seq, foreach_pair);
	i = 0;
	result = 0;
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

PRIVATE struct type_seq repeat_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&repeat_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&repeat_foreach_pair,
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&repeat_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&repeat_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeat_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
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
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE WUNUSED NONNULL((1)) DREF Repeat *DCALL
repeat_get_frozen(Repeat *__restrict self) {
	DREF DeeObject *inner_frozen;
	DREF Repeat *result;
	inner_frozen = DeeObject_InvokeMethodHint(seq_frozen, self->rp_seq);
	if unlikely(!inner_frozen)
		goto err;
	if (inner_frozen == self->rp_seq) {
		Dee_DecrefNokill(inner_frozen);
		return_reference_(self);
	}
	result = DeeObject_MALLOC(Repeat);
	if unlikely(!result)
		goto err_inner;
	result->rp_seq = inner_frozen; /* Inherit reference */
	result->rp_num = self->rp_num;
	DeeObject_InitStatic(result, &SeqRepeat_Type);
	return result;
err_inner:
	Dee_Decref(inner_frozen);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst repeat_getsets[] = {
	TYPE_GETTER(STR_frozen, &repeat_get_frozen, "->?."),
	TYPE_GETTER(STR_cached, &repeat_get_frozen, "->?."),
	/* TODO: first (require __num__ != 0, then __seq__.first; also support bound+del+set) */
	/* TODO: last (require __num__ != 0, then __seq__.last; also support bound+del+set) */
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst repeat_methods[] = {
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst repeat_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT_AB, offsetof(Repeat, rp_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(Repeat, rp_num)),
	TYPE_MEMBER_END
};

PRIVATE struct type_method_hint tpconst repeat_method_hints[] = {
//	TODO: Dee_TMH_seq_enumerate_index,
//	TODO: Dee_TMH_seq_foreach_reverse,
//	TODO: Dee_TMH_seq_enumerate_index_reverse,
//	TODO: Dee_TMH_seq_unpack,
//	TODO: Dee_TMH_seq_unpack_ex,
//	TODO: Dee_TMH_seq_unpack_ub,
//	TODO: Dee_TMH_seq_any,
//	TODO: Dee_TMH_seq_any_with_key,
//	TODO: Dee_TMH_seq_any_with_range,
//	TODO: Dee_TMH_seq_any_with_range_and_key,
//	TODO: Dee_TMH_seq_all,
//	TODO: Dee_TMH_seq_all_with_key,
//	TODO: Dee_TMH_seq_all_with_range,
//	TODO: Dee_TMH_seq_all_with_range_and_key,
//	TODO: Dee_TMH_seq_parity,
//	TODO: Dee_TMH_seq_parity_with_key,
//	TODO: Dee_TMH_seq_parity_with_range,
//	TODO: Dee_TMH_seq_parity_with_range_and_key,
//	TODO: Dee_TMH_seq_reduce,
//	TODO: Dee_TMH_seq_reduce_with_init,
//	TODO: Dee_TMH_seq_reduce_with_range,
//	TODO: Dee_TMH_seq_reduce_with_range_and_init,
//	TODO: Dee_TMH_seq_min,
//	TODO: Dee_TMH_seq_min_with_key,
//	TODO: Dee_TMH_seq_min_with_range,
//	TODO: Dee_TMH_seq_min_with_range_and_key,
//	TODO: Dee_TMH_seq_max,
//	TODO: Dee_TMH_seq_max_with_key,
//	TODO: Dee_TMH_seq_max_with_range,
//	TODO: Dee_TMH_seq_max_with_range_and_key,
//	TODO: Dee_TMH_seq_sum,
//	TODO: Dee_TMH_seq_sum_with_key,
//	TODO: Dee_TMH_seq_sum_with_range,
//	TODO: Dee_TMH_seq_sum_with_range_and_key,
//	TODO: Dee_TMH_seq_count,
//	TODO: Dee_TMH_seq_count_with_key,
//	TODO: Dee_TMH_seq_count_with_range,
//	TODO: Dee_TMH_seq_count_with_range_and_key,
//	TODO: Dee_TMH_seq_contains,
//	TODO: Dee_TMH_seq_contains_with_key,
//	TODO: Dee_TMH_seq_contains_with_range,
//	TODO: Dee_TMH_seq_contains_with_range_and_key,
//	TODO: Dee_TMH_seq_locate,
//	TODO: Dee_TMH_seq_locate_with_range,
//	TODO: Dee_TMH_seq_rlocate,
//	TODO: Dee_TMH_seq_rlocate_with_range,
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
//	TODO: Dee_TMH_seq_xchitem_index,
//	TODO: Dee_TMH_seq_clear,
//	TODO: Dee_TMH_seq_pop,
//	TODO: Dee_TMH_seq_remove,
//	TODO: Dee_TMH_seq_remove_with_key,
//	TODO: Dee_TMH_seq_rremove,
//	TODO: Dee_TMH_seq_rremove_with_key,
//	TODO: Dee_TMH_seq_removeall,
//	TODO: Dee_TMH_seq_removeall_with_key,
//	TODO: Dee_TMH_seq_removeif,
//	TODO: Dee_TMH_seq_resize,
//	TODO: Dee_TMH_seq_fill,
//	TODO: Dee_TMH_seq_reverse,
//	TODO: Dee_TMH_seq_reversed,
//	TODO: Dee_TMH_seq_sort,
//	TODO: Dee_TMH_seq_sort_with_key,
//	TODO: Dee_TMH_seq_sorted,
//	TODO: Dee_TMH_seq_sorted_with_key,
//	TODO: Dee_TMH_seq_bfind,
//	TODO: Dee_TMH_seq_bfind_with_key,
//	TODO: Dee_TMH_seq_bposition,
//	TODO: Dee_TMH_seq_bposition_with_key,
//	TODO: Dee_TMH_seq_brange,
//	TODO: Dee_TMH_seq_brange_with_key,
//	TODO: Dee_TMH_set_operator_iter,
//	TODO: Dee_TMH_set_operator_foreach,
//	TODO: Dee_TMH_set_operator_sizeob,
//	TODO: Dee_TMH_set_operator_size,
//	TODO: Dee_TMH_set_operator_hash,
//	TODO: Dee_TMH_set_operator_compare_eq,
//	TODO: Dee_TMH_set_operator_trycompare_eq,
//	TODO: Dee_TMH_set_operator_eq,
//	TODO: Dee_TMH_set_operator_ne,
//	TODO: Dee_TMH_set_operator_lo,
//	TODO: Dee_TMH_set_operator_le,
//	TODO: Dee_TMH_set_operator_gr,
//	TODO: Dee_TMH_set_operator_ge,
//	TODO: Dee_TMH_set_operator_bool,
//	TODO: Dee_TMH_set_operator_inv,
//	TODO: Dee_TMH_set_operator_add,
//	TODO: Dee_TMH_set_operator_sub,
//	TODO: Dee_TMH_set_operator_and,
//	TODO: Dee_TMH_set_operator_xor,
//	TODO: Dee_TMH_set_operator_inplace_add,
//	TODO: Dee_TMH_set_operator_inplace_sub,
//	TODO: Dee_TMH_set_operator_inplace_and,
//	TODO: Dee_TMH_set_operator_inplace_xor,
//	TODO: Dee_TMH_set_frozen,
//	TODO: Dee_TMH_set_unify,
//	TODO: Dee_TMH_set_insert,
//	TODO: Dee_TMH_set_insertall,
//	TODO: Dee_TMH_set_remove,
//	TODO: Dee_TMH_set_removeall,
//	TODO: Dee_TMH_set_pop,
//	TODO: Dee_TMH_set_pop_with_default,
//	TODO: Dee_TMH_set_trygetfirst,
//	TODO: Dee_TMH_set_getfirst,
//	TODO: Dee_TMH_set_boundfirst,
//	TODO: Dee_TMH_set_delfirst,
//	TODO: Dee_TMH_set_setfirst,
//	TODO: Dee_TMH_set_trygetlast,
//	TODO: Dee_TMH_set_getlast,
//	TODO: Dee_TMH_set_boundlast,
//	TODO: Dee_TMH_set_dellast,
//	TODO: Dee_TMH_set_setlast,
//	TODO: Dee_TMH_map_operator_iter,
//	TODO: Dee_TMH_map_operator_foreach_pair,
//	TODO: Dee_TMH_map_operator_sizeob,
//	TODO: Dee_TMH_map_operator_size,
//	TODO: Dee_TMH_map_operator_hash,
//	TODO: Dee_TMH_map_operator_getitem,
//	TODO: Dee_TMH_map_operator_trygetitem,
//	TODO: Dee_TMH_map_operator_getitem_index,
//	TODO: Dee_TMH_map_operator_trygetitem_index,
//	TODO: Dee_TMH_map_operator_getitem_string_hash,
//	TODO: Dee_TMH_map_operator_trygetitem_string_hash,
//	TODO: Dee_TMH_map_operator_getitem_string_len_hash,
//	TODO: Dee_TMH_map_operator_trygetitem_string_len_hash,
//	TODO: Dee_TMH_map_operator_bounditem,
//	TODO: Dee_TMH_map_operator_bounditem_index,
//	TODO: Dee_TMH_map_operator_bounditem_string_hash,
//	TODO: Dee_TMH_map_operator_bounditem_string_len_hash,
//	TODO: Dee_TMH_map_operator_hasitem,
//	TODO: Dee_TMH_map_operator_hasitem_index,
//	TODO: Dee_TMH_map_operator_hasitem_string_hash,
//	TODO: Dee_TMH_map_operator_hasitem_string_len_hash,
//	TODO: Dee_TMH_map_operator_delitem,
//	TODO: Dee_TMH_map_operator_delitem_index,
//	TODO: Dee_TMH_map_operator_delitem_string_hash,
//	TODO: Dee_TMH_map_operator_delitem_string_len_hash,
//	TODO: Dee_TMH_map_operator_setitem,
//	TODO: Dee_TMH_map_operator_setitem_index,
//	TODO: Dee_TMH_map_operator_setitem_string_hash,
//	TODO: Dee_TMH_map_operator_setitem_string_len_hash,
//	TODO: Dee_TMH_map_operator_contains,
//	TODO: Dee_TMH_map_keys,
//	TODO: Dee_TMH_map_iterkeys,
//	TODO: Dee_TMH_map_values,
//	TODO: Dee_TMH_map_itervalues,
//	TODO: Dee_TMH_map_enumerate,
//	TODO: Dee_TMH_map_enumerate_range,
//	TODO: Dee_TMH_map_makeenumeration,
//	TODO: Dee_TMH_map_makeenumeration_with_range,
//	TODO: Dee_TMH_map_operator_compare_eq,
//	TODO: Dee_TMH_map_operator_trycompare_eq,
//	TODO: Dee_TMH_map_operator_eq,
//	TODO: Dee_TMH_map_operator_ne,
//	TODO: Dee_TMH_map_operator_lo,
//	TODO: Dee_TMH_map_operator_le,
//	TODO: Dee_TMH_map_operator_gr,
//	TODO: Dee_TMH_map_operator_ge,
//	TODO: Dee_TMH_map_operator_add,
//	TODO: Dee_TMH_map_operator_sub,
//	TODO: Dee_TMH_map_operator_and,
//	TODO: Dee_TMH_map_operator_xor,
//	TODO: Dee_TMH_map_operator_inplace_add,
//	TODO: Dee_TMH_map_operator_inplace_sub,
//	TODO: Dee_TMH_map_operator_inplace_and,
//	TODO: Dee_TMH_map_operator_inplace_xor,
//	TODO: Dee_TMH_map_frozen,
//	TODO: Dee_TMH_map_setold,
//	TODO: Dee_TMH_map_setold_ex,
//	TODO: Dee_TMH_map_setnew,
//	TODO: Dee_TMH_map_setnew_ex,
//	TODO: Dee_TMH_map_setdefault,
//	TODO: Dee_TMH_map_update,
//	TODO: Dee_TMH_map_remove,
//	TODO: Dee_TMH_map_removekeys,
//	TODO: Dee_TMH_map_pop,
//	TODO: Dee_TMH_map_pop_with_default,
//	TODO: Dee_TMH_map_popitem,
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

PRIVATE ATTR_CONST WUNUSED int DCALL
repeatitem_any_to_parity(RepeatItem *__restrict self, int any_result) {
	if (any_result > 0 && (self->rpit_num & 1))
		any_result = 0; /* Positive parity with uneven count before zero; negative parity stays unchanged */
	return any_result;
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
	int result = repeatitem_any(self);
	return repeatitem_any_to_parity(self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_parity_with_key(RepeatItem *self, DeeObject *key) {
	int result = repeatitem_any_with_key(self, key);
	return repeatitem_any_to_parity(self, result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_parity_with_range(RepeatItem *self, size_t start, size_t end) {
	int result = repeatitem_any_with_range(self, start, end);
	return repeatitem_any_to_parity(self, result);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
repeatitem_parity_with_range_and_key(RepeatItem *self, size_t start, size_t end, DeeObject *key) {
	int result = repeatitem_any_with_range_and_key(self, start, end, key);
	return repeatitem_any_to_parity(self, result);
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
//	TODO: Dee_TMH_seq_reduce,
//	TODO: Dee_TMH_seq_reduce_with_init,
//	TODO: Dee_TMH_seq_reduce_with_range,
//	TODO: Dee_TMH_seq_reduce_with_range_and_init,
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
