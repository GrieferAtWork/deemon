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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_PAIR_C
#define GUARD_DEEMON_OBJECTS_SEQ_PAIR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* Dee_True, return_bool */
#include <deemon/computed-operators.h> /* CONFIG_ENABLE_SEQ_PAIR_TYPE, DeeSeqPairObject, DeeSeqPair_Type */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_Handled, Dee_ERROR_HANDLED_INTERRUPT */
#include <deemon/format.h>             /* DeeFormat_Printf */
#include <deemon/int.h>                /* DeeInt_One, DeeInt_Zero */
#include <deemon/method-hints.h>       /* Dee_seq_enumerate_index_t, Dee_seq_enumerate_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none-operator.h>      /* _DeeNone_reti1_1 */
#include <deemon/object.h>             /* ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_FROMPRESENT_BOUND, Dee_COMPARE_*, Dee_Decref*, Dee_Incref, Dee_Movrefv, Dee_TYPE, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compare, Dee_ssize_t, Dee_visit_t, ITER_DONE, OBJECT_HEAD, OBJECT_HEAD_INIT, return_reference */
#include <deemon/operator-hints.h>     /* DeeType_HasNativeOperator */
#include <deemon/pair.h>               /* CONFIG_ENABLE_SEQ_PAIR_TYPE, DeeSeqPairObject, DeeSeq_OfOne */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_NewEmpty, DeeSeq_Type, Dee_seq_range */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/super.h>              /* DeeSuper_New */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/type.h>               /* DeeObject_Init, DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, METHOD_FCONSTCALL, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */

#include <hybrid/typecore.h> /* __SIZEOF_SIZE_T__ */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "../int-8bit.h"
#include "default-compare.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE

typedef DeeSeqPairObject SeqPair;

/************************************************************************/
/* SeqPairIterator                                                      */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF SeqPair *spi_pair;  /* [1..1][const] Underlying pair */
	size_t        spi_index; /* [lock(ATOMIC)] Index of next item to yield */
} SeqPairIterator;

INTDEF DeeTypeObject SeqPairIterator_Type;

STATIC_ASSERT(offsetof(SeqPairIterator, spi_pair) == offsetof(ProxyObject, po_obj));
#define spi_serialize generic_proxy__serialize_and_wordcopy_atomic(__SIZEOF_SIZE_T__)
#define spi_fini      generic_proxy__fini
#define spi_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
spi_copy(SeqPairIterator *__restrict self,
         SeqPairIterator *__restrict other) {
	self->spi_pair = other->spi_pair;
	Dee_Incref(self->spi_pair);
	self->spi_index = atomic_read(&other->spi_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
spi_init(SeqPairIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	SeqPair *sp;
	DeeArg_Unpack1(err, argc, argv, "_SeqPairIterator", &sp);
	if (DeeObject_AssertTypeExact(sp, &DeeSeqPair_Type))
		goto err;
	Dee_Incref(sp);
	self->spi_pair  = sp;
	self->spi_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
spi_bool(SeqPairIterator *__restrict self) {
	return atomic_read(&self->spi_index) < 2 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
spi_compare(SeqPairIterator *lhs, SeqPairIterator *rhs) {
	size_t lhs_index, rhs_index;
	if (DeeObject_AssertTypeExact(rhs, &SeqPairIterator_Type))
		goto err;
	lhs_index = atomic_read(&lhs->spi_index);
	rhs_index = atomic_read(&rhs->spi_index);
	Dee_return_compare(lhs_index, rhs_index);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
spi_next(SeqPairIterator *__restrict self) {
	size_t index;
	do {
		index = atomic_read(&self->spi_index);
		if (index >= 2)
			return ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->spi_index, index, index + 1));
	return_reference(self->spi_pair->sp_items[index]);
}

PRIVATE struct type_cmp spi_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&spi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_member tpconst spi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(SeqPairIterator, spi_pair), "->?Ert:SeqPair"),
	TYPE_MEMBER_FIELD(STR_index, STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(SeqPairIterator, spi_index)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqPairIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqPairIterator",
	/* .tp_doc      = */ DOC("(seq:?Ert:SeqPair)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqPairIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &spi_copy,
			/* tp_any_ctor:    */ &spi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &spi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&spi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&spi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&spi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &spi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&spi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ spi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};





/************************************************************************/
/* SeqPair                                                              */
/************************************************************************/

#ifdef DeeInt_8bit
#define DeeInt_Two (&DeeInt_8bit[2])
#else /* DeeInt_8bit */
#define DeeInt_Two (&dee_int_two)
PRIVATE DEFINE_INT15(dee_int_two, 2);
#endif /* !DeeInt_8bit */


STATIC_ASSERT(sizeof(SeqPair) == sizeof(ProxyObject2));
STATIC_ASSERT((offsetof(SeqPair, sp_items) + (0 * sizeof(DeeObject *))) == offsetof(ProxyObject2, po_obj1) ||
              (offsetof(SeqPair, sp_items) + (0 * sizeof(DeeObject *))) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT((offsetof(SeqPair, sp_items) + (1 * sizeof(DeeObject *))) == offsetof(ProxyObject2, po_obj1) ||
              (offsetof(SeqPair, sp_items) + (1 * sizeof(DeeObject *))) == offsetof(ProxyObject2, po_obj2));
#define sp_copy      generic_proxy2__copy_alias12
#define sp_serialize generic_proxy2__serialize
#define sp_fini      generic_proxy2__fini
#define sp_bool      _DeeNone_reti1_1
#define sp_visit     generic_proxy2__visit

STATIC_ASSERT((offsetof(SeqPair, sp_items) + (0 * sizeof(DeeObject *))) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT((offsetof(SeqPair, sp_items) + (1 * sizeof(DeeObject *))) == offsetof(ProxyObject2, po_obj2));
#define sp_hash generic_proxy2__hash_recursive_ordered

PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_init(SeqPair *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "_SeqPair", &seq);
	return DeeObject_InvokeMethodHint(seq_unpack, seq, 2, self->sp_items);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_printrepr(SeqPair *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "{ %r, %r }",
	                        self->sp_items[0],
	                        self->sp_items[1]);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sp_compare_eq(SeqPair *lhs, DeeObject *rhs) {
	return seq_docompareeq__lhs_vector(lhs->sp_items, 2, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sp_compare(SeqPair *lhs, DeeObject *rhs) {
	return seq_docompare__lhs_vector(lhs->sp_items, 2, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sp_trycompare_eq(SeqPair *lhs, DeeObject *rhs) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs), foreach))
		return Dee_COMPARE_NE;
	return sp_compare_eq(lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqPairIterator *DCALL
sp_iter(SeqPair *__restrict self) {
	DREF SeqPairIterator *result = DeeObject_MALLOC(SeqPairIterator);
	if likely(result) {
		Dee_Incref(self);
		result->spi_pair  = self;
		result->spi_index = 0;
		DeeObject_Init(result, &SeqPairIterator_Type);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_sizeob(SeqPair *__restrict self) {
	(void)self;
	return_reference(DeeInt_Two);
}

#define sp_size_fast sp_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sp_size(SeqPair *__restrict self) {
	(void)self;
	return 2;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sp_contains(SeqPair *self, DeeObject *value) {
	int cmp = DeeObject_TryCompareEq(value, self->sp_items[0]);
	if (Dee_COMPARE_ISERR(cmp))
		goto err;
	if (Dee_COMPARE_ISNE(cmp))
		cmp = DeeObject_TryCompareEq(value, self->sp_items[1]);
	if (Dee_COMPARE_ISERR(cmp))
		goto err;
	return_bool(Dee_COMPARE_ISEQ(cmp));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_foreach(SeqPair *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result = (*cb)(arg, self->sp_items[0]);
	if likely(result >= 0) {
		Dee_ssize_t temp = (*cb)(arg, self->sp_items[1]);
		result = unlikely(temp < 0) ? temp : result + temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_mh_seq_foreach_reverse(SeqPair *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result = (*cb)(arg, self->sp_items[1]);
	if likely(result >= 0) {
		Dee_ssize_t temp = (*cb)(arg, self->sp_items[0]);
		result = unlikely(temp < 0) ? temp : result + temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_getitem_index(SeqPair *__restrict self, size_t index) {
	if unlikely(index >= 2)
		goto err_index;
	return_reference(self->sp_items[index]);
err_index:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, 2);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_trygetitem_index(SeqPair *__restrict self, size_t index) {
	if unlikely(index >= 2)
		return ITER_DONE;
	return_reference(self->sp_items[index]);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_getitem_index_fast(SeqPair *__restrict self, size_t index) {
	ASSERT(index < 2);
	return_reference(self->sp_items[index]);
}

#define sp_hasitem_index sp_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_bounditem_index(SeqPair *__restrict self, size_t index) {
	(void)self;
	return Dee_BOUND_FROMPRESENT_BOUND(index < 2);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_getrange_index(SeqPair *__restrict self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, start, end, 1);
	ASSERT(range.sr_start <= range.sr_end);
	ASSERT(range.sr_end <= 2);
	switch (range.sr_start) {
	case 0: break;
	case 1:
		ASSERT(range.sr_end == 1 || range.sr_end == 2);
		if (range.sr_end > 1)
			return DeeSeq_OfOne(self->sp_items[1]);
		ATTR_FALLTHROUGH
	case 2: return DeeSeq_NewEmpty();
	default: __builtin_unreachable();
	}
	switch (range.sr_end) {
	case 0: return DeeSeq_NewEmpty();
	case 1: return DeeSeq_OfOne(self->sp_items[0]);
	case 2: return_reference(self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_getrange_index_n(SeqPair *__restrict self, Dee_ssize_t start) {
	size_t start_index = DeeSeqRange_Clamp_n(start, 2);
	switch (start_index) {
	case 0: return_reference(self);
	case 1: return DeeSeq_OfOne(self->sp_items[1]);
	case 2: return DeeSeq_NewEmpty();
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

#define sp_asvector_nothrow sp_asvector
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sp_asvector(SeqPair *__restrict self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if likely(dst_length >= 2)
		Dee_Movrefv(dst, self->sp_items, 2);
	return 2;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_mh_seq_enumerate(SeqPair *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	Dee_ssize_t temp, result;
	result = (*cb)(arg, DeeInt_Zero, self->sp_items[0]);
	if likely(result >= 0) {
		temp = (*cb)(arg, DeeInt_One, self->sp_items[1]);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_mh_seq_enumerate_index(SeqPair *__restrict self, Dee_seq_enumerate_index_t cb,
                          void *arg, size_t start, size_t end) {
	if (end > 2)
		end = 2;
	if (start >= end)
		return 0;
	switch (start) {
	case 0: {
		Dee_ssize_t temp, result;
		if (end == 1)
			return (*cb)(arg, 0, self->sp_items[0]);
		result = (*cb)(arg, 0, self->sp_items[0]);
		if likely(result >= 0) {
			temp = (*cb)(arg, 1, self->sp_items[1]);
			if unlikely(temp < 0)
				return temp;
			result += temp;
		}
		return result;
	}	break;
	case 1:
		return (*cb)(arg, 1, self->sp_items[1]);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_mh_seq_enumerate_index_reverse(SeqPair *__restrict self, Dee_seq_enumerate_index_t cb,
                                  void *arg, size_t start, size_t end) {
	if (end > 2)
		end = 2;
	if (start >= end)
		return 0;
	switch (start) {
	case 0: {
		Dee_ssize_t temp, result;
		if (end == 1)
			return (*cb)(arg, 0, self->sp_items[0]);
		result = (*cb)(arg, 1, self->sp_items[1]);
		if likely(result >= 0) {
			temp = (*cb)(arg, 0, self->sp_items[0]);
			if unlikely(temp < 0)
				return temp;
			result += temp;
		}
		return result;
	}	break;
	case 1:
		return (*cb)(arg, 1, self->sp_items[1]);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sp_mh_seq_unpack(SeqPair *__restrict self, size_t count, DREF DeeObject *result[]) {
	if likely(count == 2) {
		Dee_Movrefv(result, self->sp_items, 2);
		return 0;
	}
	return DeeRT_ErrUnpackError(self, count, 2);
}

#define sp_mh_seq_unpack_ub sp_mh_seq_unpack_ex
PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
sp_mh_seq_unpack_ex(SeqPair *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	if likely(min_count <= 2 && max_count >= 2) {
		Dee_Movrefv(result, self->sp_items, 2);
		return 2;
	}
	return DeeRT_ErrUnpackErrorEx(self, min_count, max_count, 2);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sp_mh_set_operator_size(SeqPair *__restrict self) {
	int eq = DeeObject_TryCompareEq(self->sp_items[0], self->sp_items[1]);
	switch (eq) {
	case Dee_COMPARE_EQ:
		return 1;
	case Dee_COMPARE_LO:
	case Dee_COMPARE_GR:
		return 2;
	case Dee_COMPARE_ERR:
		return (size_t)-1;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_mh_set_operator_sizeob(SeqPair *__restrict self) {
	int eq = DeeObject_TryCompareEq(self->sp_items[0], self->sp_items[1]);
	switch (eq) {
	case Dee_COMPARE_EQ:
		return_reference(DeeInt_One);
	case Dee_COMPARE_LO:
	case Dee_COMPARE_GR:
		return_reference(DeeInt_Two);
	case Dee_COMPARE_ERR:
		return NULL;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
sp_mh_set_operator_hash(SeqPair *__restrict self) {
	int eq = DeeObject_TryCompareEq(self->sp_items[0], self->sp_items[1]);
	switch (eq) {
	case Dee_COMPARE_EQ:
		return DeeObject_Hash(self->sp_items[0]);
	case Dee_COMPARE_ERR:
		DeeError_Handled(Dee_ERROR_HANDLED_INTERRUPT);
		ATTR_FALLTHROUGH
	case Dee_COMPARE_LO:
	case Dee_COMPARE_GR:
		return DeeObject_Hash(self->sp_items[0]) ^
		       DeeObject_Hash(self->sp_items[1]);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sp_mh_set_operator_foreach(SeqPair *__restrict self, Dee_foreach_t cb, void *arg) {
	int eq = DeeObject_TryCompareEq(self->sp_items[0], self->sp_items[1]);
	switch (eq) {
	case Dee_COMPARE_EQ:
		return (*cb)(arg, self->sp_items[0]);
	case Dee_COMPARE_LO:
	case Dee_COMPARE_GR:
		return sp_foreach(self, cb, arg);
	case Dee_COMPARE_ERR:
		return -1;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sp_mh_set_frozen(SeqPair *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *inner;
	int eq = DeeObject_TryCompareEq(self->sp_items[0], self->sp_items[1]);
	switch (eq) {
	case Dee_COMPARE_EQ:
		inner = DeeSeq_OfOne(self->sp_items[0]);
		break;
	case Dee_COMPARE_LO:
	case Dee_COMPARE_GR:
		Dee_Incref(self);
		inner = Dee_AsObject(self);
		break;
	case Dee_COMPARE_ERR:
		return NULL;
	default: __builtin_unreachable();
	}
	result = DeeSuper_New(&DeeSet_Type, inner);
	Dee_Decref_unlikely(inner);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_BoolWithKey(DeeObject *self, DeeObject *key) {
	DREF DeeObject *keyed = DeeObject_Call(key, 1, &self);
	if unlikely(!keyed)
		goto err;
	return DeeObject_BoolInherited(keyed);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_mh_seq_any(SeqPair *__restrict self) {
	int result = DeeObject_Bool(self->sp_items[0]);
	if (result == 0)
		result = DeeObject_Bool(self->sp_items[1]);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sp_mh_seq_any_with_key(SeqPair *self, DeeObject *key) {
	int result = DeeObject_BoolWithKey(self->sp_items[0], key);
	if (result == 0)
		result = DeeObject_BoolWithKey(self->sp_items[1], key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_mh_seq_any_with_range(SeqPair *__restrict self, size_t start, size_t end) {
	int result = 0;
	if (start <= 0 && end > 0)
		result = DeeObject_Bool(self->sp_items[0]);
	if (result == 0 && (start <= 1 && end > 1))
		result = DeeObject_Bool(self->sp_items[1]);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
sp_mh_seq_any_with_range_and_key(SeqPair *self, size_t start, size_t end, DeeObject *key) {
	int result = 0;
	if (start <= 0 && end > 0)
		result = DeeObject_BoolWithKey(self->sp_items[0], key);
	if (result == 0 && (start <= 1 && end > 1))
		result = DeeObject_BoolWithKey(self->sp_items[1], key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_mh_seq_all(SeqPair *__restrict self) {
	int result = DeeObject_Bool(self->sp_items[0]);
	if (result > 0)
		result = DeeObject_Bool(self->sp_items[1]);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sp_mh_seq_all_with_key(SeqPair *self, DeeObject *key) {
	int result = DeeObject_BoolWithKey(self->sp_items[0], key);
	if (result > 0)
		result = DeeObject_BoolWithKey(self->sp_items[1], key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_mh_seq_all_with_range(SeqPair *__restrict self, size_t start, size_t end) {
	int result = 1;
	if (start <= 0 && end > 0)
		result = DeeObject_Bool(self->sp_items[0]);
	if (result > 0 && (start <= 1 && end > 1))
		result = DeeObject_Bool(self->sp_items[1]);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
sp_mh_seq_all_with_range_and_key(SeqPair *self, size_t start, size_t end, DeeObject *key) {
	int result = 1;
	if (start <= 0 && end > 0)
		result = DeeObject_BoolWithKey(self->sp_items[0], key);
	if (result > 0 && (start <= 1 && end > 1))
		result = DeeObject_BoolWithKey(self->sp_items[1], key);
	return result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_mh_seq_parity(SeqPair *__restrict self) {
	int result = DeeObject_Bool(self->sp_items[0]);
	if (result >= 0) {
		int temp = DeeObject_Bool(self->sp_items[1]);
		if unlikely(temp < 0) {
			result = temp;
		} else {
			result = (result != 0) ^ (temp != 0);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sp_mh_seq_parity_with_key(SeqPair *self, DeeObject *key) {
	int result = DeeObject_BoolWithKey(self->sp_items[0], key);
	if (result >= 0) {
		int temp = DeeObject_BoolWithKey(self->sp_items[1], key);
		if unlikely(temp < 0) {
			result = temp;
		} else {
			result = (result != 0) ^ (temp != 0);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sp_mh_seq_parity_with_range(SeqPair *__restrict self, size_t start, size_t end) {
	if (end > 2)
		end = 2;
	if (start >= end)
		return 0;
	switch (start) {
	case 0:
		if (end == 1)
			return DeeObject_Bool(self->sp_items[0]);
		return sp_mh_seq_parity(self);
	case 1:
		return DeeObject_Bool(self->sp_items[1]);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
sp_mh_seq_parity_with_range_and_key(SeqPair *self, size_t start, size_t end, DeeObject *key) {
	if (end > 2)
		end = 2;
	if (start >= end)
		return 0;
	switch (start) {
	case 0:
		if (end == 1)
			return DeeObject_BoolWithKey(self->sp_items[0], key);
		return sp_mh_seq_parity_with_key(self, key);
	case 1:
		return DeeObject_BoolWithKey(self->sp_items[1], key);
	default: __builtin_unreachable();
	}
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sp_mh_seq_reduce(SeqPair *self, DeeObject *combine) {
	return DeeObject_Call(combine, 2, self->sp_items);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
sp_mh_seq_reduce_with_init(SeqPair *self, DeeObject *combine, DeeObject *init) {
	DREF DeeObject *result;
	DeeObject *pair[2];
	pair[0] = init;
	pair[1] = self->sp_items[0];
	result = DeeObject_Call(combine, 2, pair);
	if likely(result) {
		pair[0] = result;
		pair[1] = self->sp_items[1];
		result = DeeObject_Call(combine, 2, pair);
		Dee_Decref(pair[0]);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sp_mh_seq_reduce_with_range(SeqPair *self, DeeObject *combine, size_t start, size_t end) {
	if (end > 2)
		end = 2;
	if unlikely(start >= end) {
		DeeRT_ErrEmptySequence(self);
		return NULL;
	}
	switch (start) {
	case 0:
		if (end == 1)
			return_reference(self->sp_items[0]);
		return DeeObject_Call(combine, 2, self->sp_items);
	case 1:
		return_reference(self->sp_items[1]);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
sp_mh_seq_reduce_with_range_and_init(SeqPair *self, DeeObject *combine,
                                     size_t start, size_t end, DeeObject *init) {
	DREF DeeObject *result;
	DeeObject *pair[2];
	if (end > 2)
		end = 2;
	if unlikely(start >= end)
		return_reference(init);
	pair[0] = init;
	switch (start) {
	case 0:
		pair[1] = self->sp_items[0];
		result = DeeObject_Call(combine, 2, pair);
		if (end == 1 || unlikely(!result))
			return result;
		pair[0] = result;
		pair[1] = self->sp_items[1];
		result = DeeObject_Call(combine, 2, pair);
		Dee_Decref(pair[0]);
		return result;
	case 1:
		pair[1] = self->sp_items[1];
		return DeeObject_Call(combine, 2, pair);
	default: __builtin_unreachable();
	}
}




PRIVATE struct type_cmp sp_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&sp_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sp_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&sp_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&sp_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_seq sp_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sp_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sp_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sp_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&sp_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&sp_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&sp_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sp_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sp_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&sp_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&sp_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&sp_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&sp_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sp_trygetitem_index,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&sp_asvector,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&sp_asvector_nothrow,
};

PRIVATE struct type_method tpconst sp_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(Sequence_unpack),
	TYPE_METHOD_HINTREF(Sequence_unpackub),
	TYPE_METHOD_HINTREF(Sequence_any),
	TYPE_METHOD_HINTREF(Sequence_all),
	TYPE_METHOD_HINTREF(Sequence_parity),
	TYPE_METHOD_HINTREF(Sequence_reduce),
//TODO:	TYPE_METHOD_HINTREF(Sequence_min),
//TODO:	TYPE_METHOD_HINTREF(Sequence_max),
//TODO:	TYPE_METHOD_HINTREF(Sequence_sum),
//TODO:	TYPE_METHOD_HINTREF(Sequence_count),
//TODO:	TYPE_METHOD_HINTREF(Sequence_contains),
//TODO:	TYPE_METHOD_HINTREF(Sequence_locate),
//TODO:	TYPE_METHOD_HINTREF(Sequence_rlocate),
//TODO:	TYPE_METHOD_HINTREF(Sequence_startswith),
//TODO:	TYPE_METHOD_HINTREF(Sequence_endswith),
//TODO:	TYPE_METHOD_HINTREF(Sequence_find),
//TODO:	TYPE_METHOD_HINTREF(Sequence_rfind),
//TODO:	TYPE_METHOD_HINTREF(Sequence_reversed),
//TODO:	TYPE_METHOD_HINTREF(Sequence_sorted),
//TODO:	TYPE_METHOD_HINTREF(Sequence_bfind),
//TODO:	TYPE_METHOD_HINTREF(Sequence_bposition),
//TODO:	TYPE_METHOD_HINTREF(Sequence_brange),
	TYPE_METHOD_HINTREF(__set_iter__),
	TYPE_METHOD_HINTREF(__set_size__),
	TYPE_METHOD_HINTREF(__set_hash__),
//TODO:	TYPE_METHOD_HINTREF(__map_size__),
//TODO:	TYPE_METHOD_HINTREF(__map_hash__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst sp_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate, &sp_mh_seq_enumerate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &sp_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &sp_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &sp_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack, &sp_mh_seq_unpack, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack_ex, &sp_mh_seq_unpack_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack_ub, &sp_mh_seq_unpack_ub, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any, &sp_mh_seq_any, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_key, &sp_mh_seq_any_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &sp_mh_seq_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &sp_mh_seq_any_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all, &sp_mh_seq_all, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &sp_mh_seq_all_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, &sp_mh_seq_all_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &sp_mh_seq_all_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity, &sp_mh_seq_parity, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_key, &sp_mh_seq_parity_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range, &sp_mh_seq_parity_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range_and_key, &sp_mh_seq_parity_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce, &sp_mh_seq_reduce, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce_with_init, &sp_mh_seq_reduce_with_init, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce_with_range, &sp_mh_seq_reduce_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce_with_range_and_init, &sp_mh_seq_reduce_with_range_and_init, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_min, &sp_mh_seq_min, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_min_with_key, &sp_mh_seq_min_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_min_with_range, &sp_mh_seq_min_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_min_with_range_and_key, &sp_mh_seq_min_with_range_and_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_max, &sp_mh_seq_max, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_max_with_key, &sp_mh_seq_max_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_max_with_range, &sp_mh_seq_max_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_max_with_range_and_key, &sp_mh_seq_max_with_range_and_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sum, &sp_mh_seq_sum, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sum_with_range, &sp_mh_seq_sum_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_count, &sp_mh_seq_count, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_count_with_key, &sp_mh_seq_count_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_count_with_range, &sp_mh_seq_count_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &sp_mh_seq_count_with_range_and_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_contains, &sp_mh_seq_contains, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_contains_with_key, &sp_mh_seq_contains_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_contains_with_range, &sp_mh_seq_contains_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &sp_mh_seq_contains_with_range_and_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_locate, &sp_mh_seq_locate, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_locate_with_range, &sp_mh_seq_locate_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_rlocate, &sp_mh_seq_rlocate, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_rlocate_with_range, &sp_mh_seq_rlocate_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_startswith, &sp_mh_seq_startswith, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_startswith_with_key, &sp_mh_seq_startswith_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_startswith_with_range, &sp_mh_seq_startswith_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_startswith_with_range_and_key, &sp_mh_seq_startswith_with_range_and_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_endswith, &sp_mh_seq_endswith, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_endswith_with_key, &sp_mh_seq_endswith_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_endswith_with_range, &sp_mh_seq_endswith_with_range, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_endswith_with_range_and_key, &sp_mh_seq_endswith_with_range_and_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_find, &sp_mh_seq_find, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_find_with_key, &sp_mh_seq_find_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_rfind, &sp_mh_seq_rfind, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_rfind_with_key, &sp_mh_seq_rfind_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_reversed, &sp_mh_seq_reversed, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sorted, &sp_mh_seq_sorted, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sorted_with_key, &sp_mh_seq_sorted_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_bfind, &sp_mh_seq_bfind, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_bfind_with_key, &sp_mh_seq_bfind_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_bposition, &sp_mh_seq_bposition, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_bposition_with_key, &sp_mh_seq_bposition_with_key, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_brange, &sp_mh_seq_brange, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_brange_with_key, &sp_mh_seq_brange_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_foreach, &sp_mh_set_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_sizeob, &sp_mh_set_operator_sizeob, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_size, &sp_mh_set_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_hash, &sp_mh_set_operator_hash, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(map_operator_iter, &sp_mh_map_operator_iter, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(map_operator_sizeob, &sp_mh_map_operator_sizeob, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(map_operator_size, &sp_mh_map_operator_size, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(map_operator_hash, &sp_mh_map_operator_hash, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst sp_getsets[] = {
	/* These must be in "so_getsets" since that's where method hints are expected to find them. */
	TYPE_GETTER_AB_F(STR_cached, &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_AB_F(STR_frozen, &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_AB_F(STR___set_frozen__, &sp_mh_set_frozen, METHOD_FCONSTCALL, "->?DSet"),
//TODO:	TYPE_GETTER_AB_F(STR___map_frozen__, &sp_mh_map_frozen, METHOD_FCONSTCALL, "->?DMapping"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst sp_members[] = {
	TYPE_MEMBER_FIELD(STR_first, STRUCT_OBJECT_AB, offsetof(SeqPair, sp_items[0])),
	TYPE_MEMBER_FIELD(STR_last, STRUCT_OBJECT_AB, offsetof(SeqPair, sp_items[1])),
	/* Can just re-use first/last getters for sets:
	 * >> local x = {a, b} as Set;
	 * >> print x.first; // Always "a" (if "a == b", this is also correct, since that also means "first == last")
	 * >> print x.last;  // Always "b" (if "a == b", this is also correct, since that also means "first == last") */
	TYPE_MEMBER_FIELD(STR___set_first__, STRUCT_OBJECT_AB, offsetof(SeqPair, sp_items[0])),
	TYPE_MEMBER_FIELD(STR___set_last__, STRUCT_OBJECT_AB, offsetof(SeqPair, sp_items[1])),
	TYPE_MEMBER_CONST("length", DeeInt_Two),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sp_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqPairIterator_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeSeqPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqPair",
	/* .tp_doc      = */ DOC("Specialized sequence type that always contains exactly 2 items\n"
	                         "\n"
	                         "(seq:?T2?O?O)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqPair,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &sp_copy,
			/* tp_any_ctor:    */ &sp_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sp_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sp_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&sp_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sp_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ &sp_cmp,
	/* .tp_seq           = */ &sp_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sp_methods,
	/* .tp_getsets       = */ sp_getsets,
	/* .tp_members       = */ sp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sp_class_members,
	/* .tp_method_hints  = */ sp_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#endif /* CONFIG_ENABLE_SEQ_PAIR_TYPE */



PUBLIC WUNUSED DREF DeeSeqPairObject *
(DCALL DeeSeq_NewPairUninitialized)(void) {
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	return DeeObject_MALLOC(DeeSeqPairObject);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	return (DREF DeeSeqPairObject *)DeeTuple_NewUninitialized(1);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
}

PUBLIC NONNULL((1)) void
(DCALL DeeSeq_FreePairUninitialized)(DREF DeeSeqPairObject *__restrict self) {
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	DeeObject_FREE(self);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	DeeTuple_FreeUninitialized((DeeTupleObject *)self);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
}

PUBLIC ATTR_RETNONNULL NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_InitPair)(/*inherit(always)*/ DREF DeeSeqPairObject *__restrict self,
                        DeeObject *a, DeeObject *b) {
	ASSERT(a != Dee_AsObject(self));
	ASSERT(b != Dee_AsObject(self));
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	Dee_Incref(a);
	self->sp_items[0] = a;
	Dee_Incref(b);
	self->sp_items[1] = b;
	DeeObject_Init(self, &DeeSeqPair_Type);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(DeeTuple_SIZE(self) == 2);
	Dee_Incref(a);
	DeeTuple_SET(self, 0, a);
	Dee_Incref(b);
	DeeTuple_SET(self, 1, b);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
	return Dee_AsObject(self);
}

PUBLIC ATTR_RETNONNULL NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_InitPairInherited)(/*inherit(always)*/ DREF DeeSeqPairObject *__restrict self,
                                 /*inherit(always)*/ DREF DeeObject *a,
                                 /*inherit(always)*/ DREF DeeObject *b) {
	ASSERT(a != Dee_AsObject(self));
	ASSERT(b != Dee_AsObject(self));
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	self->sp_items[0] = a; /* Inherited */
	self->sp_items[1] = b; /* Inherited */
	DeeObject_Init(self, &DeeSeqPair_Type);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(DeeTuple_SIZE(self) == 2);
	DeeTuple_SET(self, 0, a); /* Inherited */
	DeeTuple_SET(self, 1, b); /* Inherited */
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
	return Dee_AsObject(self);
}



/* Construct a new 2-element sequence */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_OfPair(DeeObject *a,
              DeeObject *b) {
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	return DeeSeq_InitPair(result, a, b);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_OfPairInherited(/*inherit(always)*/ DREF DeeObject *a,
                       /*inherit(always)*/ DREF DeeObject *b) {
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	return DeeSeq_InitPairInherited(result, a, b);
err:
	Dee_Decref(b); /* Inherited */
	Dee_Decref(a); /* Inherited */
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_OfPairInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *a,
                                /*inherit(on_success)*/ DREF DeeObject *b) {
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	return DeeSeq_InitPairInherited(result, a, b);
err:
	return NULL;
}


/* Pack a 2-item sequence using a symbolic reference */
PUBLIC NONNULL((1)) void DCALL
DeeSeqPair_DecrefSymbolic(DREF DeeObject *__restrict self) {
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	SeqPair *me = (SeqPair *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSeqPair_Type);
	if (!DeeObject_IsShared(me)) {
		DeeSeq_FreePairUninitialized(me);
		Dee_DecrefNokill(&DeeSeqPair_Type);
	} else {
		Dee_Incref(me->sp_items[1]);
		Dee_Incref(me->sp_items[0]);
		Dee_Decref_unlikely(me);
	}
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(DeeTuple_SIZE(self) == 2);
	DeeTuple_DecrefSymbolic(self);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_PAIR_C */
