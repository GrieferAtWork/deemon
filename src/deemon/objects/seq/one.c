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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_ONE_C
#define GUARD_DEEMON_OBJECTS_SEQ_ONE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/typecore.h>
/**/

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "concat.h"
#include "default-compare.h"
#include "one.h"
#include "repeat.h"

DECL_BEGIN

typedef DeeSeqOneObject SeqOne;


/************************************************************************/
/* SeqOneIterator                                                       */
/************************************************************************/

/* Try to get a reference to the stored item without
 * (returns "ITER_DONE" if already consumed) */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
soi_trygetitemref(SeqOneIterator *__restrict self) {
	DREF DeeObject *result;
#ifdef CONFIG_NO_THREADS
	result = self->soi_item;
	if (result != ITER_DONE)
		Dee_Incref(result);
#else /* CONFIG_NO_THREADS */
again:
	result = atomic_read(&self->soi_item);
	if (result != ITER_DONE) {
		if (result == NULL) {
			SCHED_YIELD();
			goto again;
		}
		if (!atomic_cmpxch_weak(&self->soi_item, result, NULL))
			goto again;
		Dee_Incref(result);
		atomic_write(&self->soi_item, result);
	}
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
soi_next(SeqOneIterator *__restrict self) {
	DREF DeeObject *result;
#ifdef CONFIG_NO_THREADS
	result = self->soi_item;
	self->soi_item = ITER_DONE;
#else /* CONFIG_NO_THREADS */
again:
	result = atomic_xch(&self->soi_item, ITER_DONE);
	if unlikely(result == NULL) {
		atomic_cmpxch(&self->soi_item, ITER_DONE, NULL);
		SCHED_YIELD();
		goto again;
	}
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
soi_copy(SeqOneIterator *__restrict self,
         SeqOneIterator *__restrict other) {
	self->soi_item = soi_trygetitemref(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
soi_deep(SeqOneIterator *__restrict self,
         SeqOneIterator *__restrict other) {
	DREF DeeObject *item;
	item = soi_trygetitemref(other);
	self->soi_item = DeeObject_DeepCopy(item);
	if unlikely(!self->soi_item)
		goto err_item;
	return 0;
err_item:
	Dee_Decref_unlikely(item);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
soi_init(SeqOneIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	SeqOne *so;
	_DeeArg_Unpack1(err, argc, argv, "_SeqOneIterator", &so);
	if (DeeObject_AssertTypeExact(so, &DeeSeqOne_Type))
		goto err;
	self->soi_item = so->so_item;
	Dee_Incref(self->soi_item);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
soi_fini(SeqOneIterator *__restrict self) {
	if (self->soi_item != ITER_DONE)
		Dee_Decref(self->soi_item);
}

PRIVATE NONNULL((1, 2)) void DCALL
soi_visit(SeqOneIterator *__restrict self, Dee_visit_t proc, void *arg) {
	DeeObject *item;
#ifdef CONFIG_NO_THREADS
	item = self->soi_item;
	if (item != ITER_DONE)
		Dee_Visit(item);
#else /* CONFIG_NO_THREADS */
again:
	item = atomic_read(&self->soi_item);
	if (item != ITER_DONE) {
		if (item == NULL) {
			SCHED_YIELD();
			goto again;
		}
		if (!atomic_cmpxch_weak(&self->soi_item, item, NULL))
			goto again;
		Dee_Visit(item);
		atomic_write(&self->soi_item, item);
	}
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
soi_compare(SeqOneIterator *lhs, SeqOneIterator *rhs) {
	bool lhs_consumed, rhs_consumed;
	if (DeeObject_AssertTypeExact(rhs, &SeqOneIterator_Type))
		goto err;
	lhs_consumed = atomic_read(&lhs->soi_item) != ITER_DONE;
	rhs_consumed = atomic_read(&rhs->soi_item) != ITER_DONE;
	Dee_return_compare(lhs_consumed, rhs_consumed);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp soi_cmp = {
	/* .tp_hash       = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&soi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
soi_getitem(SeqOneIterator *__restrict self) {
	DREF DeeObject *result = soi_trygetitemref(self);
	if likely(result != ITER_DONE)
		return result;
	err_unbound_attribute_string(&SeqOneIterator_Type, "__item__");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
soi_bool(SeqOneIterator *__restrict self) {
	bool bound = atomic_read(&self->soi_item) != ITER_DONE;
	return bound ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
soi_bounditem(SeqOneIterator *__restrict self) {
	bool bound = atomic_read(&self->soi_item) != ITER_DONE;
	return Dee_BOUND_FROMBOOL(bound);
}

#define soi_boundseq soi_bounditem
PRIVATE WUNUSED NONNULL((1)) DREF SeqOne *DCALL
soi_getseq(SeqOneIterator *__restrict self) {
	DREF DeeObject *result = soi_trygetitemref(self);
	if likely(result != ITER_DONE)
		return (DREF SeqOne *)DeeSeq_PackOneInherited(result);
	err_unbound_attribute_string(&SeqOneIterator_Type, STR_seq);
	return NULL;
}

PRIVATE struct type_getset tpconst soi_getsets[] = {
	TYPE_GETTER_BOUND_F(STR_seq, &soi_getseq, &soi_boundseq,
	                    METHOD_FPURECALL | METHOD_FNOREFESCAPE,
	                    "->?Ert:SeqOne"),
	TYPE_GETTER_BOUND_F_NODOC("__item__", &soi_getitem, &soi_bounditem,
	                          METHOD_FPURECALL | METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};



INTERN DeeTypeObject SeqOneIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqOneIterator",
	/* .tp_doc      = */ DOC("(seq:?Ert:SeqOne)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&soi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&soi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&soi_init,
				TYPE_FIXED_ALLOCATOR(SeqOneIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&soi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&soi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&soi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &soi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&soi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ soi_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



/************************************************************************/
/* SeqOne                                                               */
/************************************************************************/
STATIC_ASSERT(offsetof(SeqOne, so_item) == offsetof(ProxyObject, po_obj));
#define so_copy  generic_proxy__copy_alias
#define so_deep  generic_proxy__deepcopy
#define so_init  generic_proxy__init
#define so_fini  generic_proxy__fini
#define so_bool  _DeeNone_reti1_1
#define so_visit generic_proxy__visit

#define so_size      _DeeNone_rets1_1
#define so_size_fast _DeeNone_rets1_1

#define _so_getitem           generic_proxy__getobj
#define so_getfirst           _so_getitem
#define so_getlast            _so_getitem
#define so_mh_seq_trygetfirst _so_getitem
#define so_mh_seq_trygetlast  _so_getitem

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
so_printrepr(SeqOne *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "{ %r }", self->so_item);
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqOneIterator *DCALL
so_iter(SeqOne *__restrict self) {
	DREF SeqOneIterator *result = DeeObject_MALLOC(SeqOneIterator);
	if unlikely(!result)
		goto err;
	result->soi_item = self->so_item;
	Dee_Incref(result->soi_item);
	DeeObject_Init(result, &SeqOneIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_sizeob(SeqOne *__restrict self) {
	(void)self;
	return DeeInt_NewOne();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_contains(SeqOne *__restrict self, DeeObject *item) {
	int cmp = DeeObject_TryCompareEq(item, self->so_item);
	if (cmp == Dee_COMPARE_ERR)
		goto err;
	return_bool(cmp == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_getitem_index(SeqOne *__restrict self, size_t index) {
	if unlikely(index != 0)
		goto err_index;
	return_reference(self->so_item);
err_index:
	err_index_out_of_bounds((DeeObject *)self, index, 1);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_getitem_index_fast(SeqOne *__restrict self, size_t index) {
	ASSERT(index == 0);
	return_reference(self->so_item);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_trygetitem_index(SeqOne *__restrict self, size_t index) {
	if unlikely(index != 0)
		return ITER_DONE;
	return_reference(self->so_item);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
so_bounditem_index(SeqOne *__restrict self, size_t index) {
	(void)self;
	return Dee_BOUND_FROMPRESENT_BOUND(index == 0);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
so_hasitem_index(SeqOne *__restrict self, size_t index) {
	(void)self;
	return index == 0 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_getrange_index(SeqOne *__restrict self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, start, end, 1);
	if (range.sr_start == 0 && range.sr_end == 1)
		return_reference((DeeObject *)self);
	return DeeSeq_NewEmpty();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_getrange_index_n(SeqOne *__restrict self, Dee_ssize_t start) {
	if (start == 0)
		return_reference((DeeObject *)self);
	return DeeSeq_NewEmpty();
}

#define so_asvector_nothrow so_asvector
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
so_asvector(SeqOne *__restrict self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	if likely(dst_length >= 1) {
		dst[0] = self->so_item;
		Dee_Incref(self->so_item);
	}
	return 1;
}

#define so_mh_seq_foreach_reverse so_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
so_foreach(SeqOne *__restrict self, Dee_foreach_t cb, void *arg) {
	return (*cb)(arg, self->so_item);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) DREF DeeTupleObject *DCALL
pack_tuple_with_prepend(DeeObject *first_item, DeeObject *remainder, size_t remainder_size,
                        size_t (DCALL *remainder_tp_asvector)(DeeObject *self,
                                                              size_t dst_length,
                                                              /*out*/ DREF DeeObject **dst)) {
	DREF DeeTupleObject *result;
	size_t total_size, new_remainder_size;
	if (OVERFLOW_UADD(remainder_size, 1, &total_size))
		goto err_overflow;
	result = DeeTuple_NewUninitialized(total_size);
	if unlikely(!result)
		goto err;
again_asvector:
	new_remainder_size = (*remainder_tp_asvector)(remainder, remainder_size,
	                                              result->t_elem + 1);
	if unlikely(new_remainder_size == (size_t)-1)
		goto err_r;
	if unlikely(remainder_size != new_remainder_size) {
		if (remainder_size > new_remainder_size) {
			result = DeeTuple_TruncateUninitialized(result, new_remainder_size);
		} else {
			DREF DeeTupleObject *new_result;
			if (OVERFLOW_UADD(new_remainder_size, 1, &total_size))
				goto err_r_overflow;
			new_result = DeeTuple_ResizeUninitialized(result, total_size);
			if unlikely(!new_result)
				goto err_r;
			remainder_size = new_remainder_size;
			result         = new_result;
			goto again_asvector;
		}
	}
	Dee_Incref(first_item);
	result->t_elem[0] = first_item;
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
err_r_overflow:
	DeeTuple_FreeUninitialized(result);
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * __CHAR_BIT__, true);
	goto err;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
so_add(SeqOne *lhs, DeeObject *rhs) {
	size_t sizehint = DeeObject_SizeFast(rhs);
	if (sizehint != (size_t)-1) {
		if (sizehint == 0)
			return_reference((DeeObject *)lhs);
		if (Dee_TYPE(rhs)->tp_seq &&
		    Dee_TYPE(rhs)->tp_seq->tp_asvector) {
			/* Construct a tuple that prepends "lhs->so_item".
			 *
			 * This is the optimized path for user-code writing:
			 * >> local x = { foo, remainder... };
			 * ... when "remainder" supports the vector-interface */
			DREF DeeTupleObject *result;
			result = pack_tuple_with_prepend(lhs->so_item, rhs, sizehint,
			                                 Dee_TYPE(rhs)->tp_seq->tp_asvector);
			return (DREF DeeObject *)result;
		}
	}
	return DeeSeq_Concat((DeeObject *)lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
so_mul(SeqOne *self, DeeObject *countob) {
	size_t count;
	if (DeeObject_AsSize(countob, &count))
		goto err;
	return DeeSeq_RepeatItem(self->so_item, count);
err:
	return NULL;
}


STATIC_ASSERT(offsetof(SeqOne, so_item) == offsetof(ProxyObject, po_obj));
#define so_hash generic_proxy__hash_recursive

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
so_compare_eq(SeqOne *lhs, DeeObject *rhs) {
	return seq_docompareeq__lhs_vector(&lhs->so_item, 1, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
so_compare(SeqOne *lhs, DeeObject *rhs) {
	return seq_docompare__lhs_vector(&lhs->so_item, 1, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
so_trycompare_eq(SeqOne *lhs, DeeObject *rhs) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs), foreach))
		return 1;
	return so_compare_eq(lhs, rhs);
}






#define so_mh_seq_enumerate_reverse so_mh_seq_enumerate
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
so_mh_seq_enumerate(SeqOne *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	return (*cb)(arg, DeeInt_Zero, self->so_item);
}

#define so_mh_seq_enumerate_index_reverse so_mh_seq_enumerate_index
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
so_mh_seq_enumerate_index(SeqOne *__restrict self,
                          Dee_seq_enumerate_index_t cb, void *arg,
                          size_t start, size_t end) {
	if likely(start == 0 && end >= 1)
		return (*cb)(arg, 0, self->so_item);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqOne *DCALL
so_mh_seq_makeenumeration(SeqOne *__restrict self) {
	DREF DeeObject *item;
	item = DeeTuple_PackPair(DeeInt_Zero, self->so_item);
	if unlikely(!item)
		goto err;
	return (DREF SeqOne *)DeeSeq_PackOneInherited(item);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_mh_seq_makeenumeration_with_intrange(SeqOne *__restrict self,
                                        size_t start, size_t end) {
	if (start == 0 && end >= 1)
		return (DREF DeeObject *)so_mh_seq_makeenumeration(self);
	return DeeSeq_NewEmpty();
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
so_mh_seq_unpack(SeqOne *__restrict self, size_t count, DREF DeeObject *result[]) {
	if likely(count == 1) {
		result[0] = self->so_item;
		Dee_Incref(self->so_item);
		return 0;
	}
	return err_invalid_unpack_size((DeeObject *)self, count, 1);
}

#define so_mh_seq_unpack_ub so_mh_seq_unpack_ex
PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
so_mh_seq_unpack_ex(SeqOne *__restrict self, size_t min_count,
                    size_t max_count, DREF DeeObject *result[]) {
	if likely(min_count <= 1 && max_count >= 1) {
		result[0] = self->so_item;
		Dee_Incref(self->so_item);
		return 1;
	}
	return err_invalid_unpack_size_minmax((DeeObject *)self, min_count, max_count, 1);
}

STATIC_ASSERT(offsetof(SeqOne, so_item) == offsetof(ProxyObject, po_obj));
#define so_mh_seq_all    so_mh_seq_any
#define so_mh_seq_parity so_mh_seq_any
#define so_mh_seq_any    generic_proxy__bool

#define so_mh_seq_all_with_range    so_mh_seq_any_with_range
#define so_mh_seq_parity_with_range so_mh_seq_any_with_range
PRIVATE WUNUSED NONNULL((1)) int DCALL
so_mh_seq_any_with_range(SeqOne *__restrict self,
                         size_t start, size_t end) {
	if (start == 0 && end >= 1)
		return so_mh_seq_any((ProxyObject *)self);
	return 0;
}

#define so_mh_seq_all_with_key    so_mh_seq_any_with_key
#define so_mh_seq_parity_with_key so_mh_seq_any_with_key
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
so_mh_seq_any_with_key(SeqOne *self, DeeObject *key) {
	DREF DeeObject *keyed = DeeObject_Call(key, 1, &self->so_item);
	if unlikely(!keyed)
		goto err;
	return DeeObject_BoolInherited(keyed);
err:
	return -1;
}

#define so_mh_seq_all_with_range_and_key    so_mh_seq_any_with_range_and_key
#define so_mh_seq_parity_with_range_and_key so_mh_seq_any_with_range_and_key
PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
so_mh_seq_any_with_range_and_key(SeqOne *self, size_t start,
                                 size_t end, DeeObject *key) {
	if (start == 0 && end >= 1)
		return so_mh_seq_any_with_key(self, key);
	return 0;
}

#define so_mh_seq_min          _so_getitem
#define so_mh_seq_max          _so_getitem
#define so_mh_seq_max_with_key so_mh_seq_min_with_key
#ifdef DCALL_CALLER_CLEANUP
#define so_mh_seq_min_with_key _so_getitem
#else /* DCALL_CALLER_CLEANUP */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
so_mh_seq_min_with_key(SeqOne *self, DeeObject *key) {
	(void)key;
	return_reference(self->so_item);
}
#endif /* !DCALL_CALLER_CLEANUP */

#define so_mh_seq_max_with_range so_mh_seq_min_with_range
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_mh_seq_min_with_range(SeqOne *__restrict self, size_t start, size_t end) {
	if likely(start == 0 && end >= 1)
		return_reference(self->so_item);
	err_empty_sequence((DeeObject *)self);
	return NULL;
}

#define so_mh_seq_max_with_range_and_key so_mh_seq_min_with_range_and_key
PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
so_mh_seq_min_with_range_and_key(SeqOne *self, size_t start,
                                 size_t end, DeeObject *key) {
	(void)key;
	return so_mh_seq_min_with_range(self, start, end);
}



#define so_mh_seq_reduce so_mh_seq_min_with_key
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
so_mh_seq_reduce_with_init(SeqOne *self, DeeObject *combine, DeeObject *init) {
	DeeObject *args[2];
	args[0] = init;
	args[1] = self->so_item;
	return DeeObject_Call(combine, 2, args);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
so_mh_seq_reduce_with_range(SeqOne *self, DeeObject *combine,
                            size_t start, size_t end) {
	(void)combine;
	return so_mh_seq_min_with_range(self, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
so_mh_seq_reduce_with_range_and_init(SeqOne *self, DeeObject *combine,
                                     size_t start, size_t end,
                                     DeeObject *init) {
	if likely(start == 0 && end >= 1)
		return so_mh_seq_reduce_with_init(self, combine, init);
	err_empty_sequence((DeeObject *)self);
	return NULL;
}


#define so_mh_seq_sum            _so_getitem
#define so_mh_seq_sum_with_range so_mh_seq_min_with_range

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
so_mh_seq_count(SeqOne *self, DeeObject *item) {
	int cmp = DeeObject_TryCompareEq(item, self->so_item);
	if (cmp == Dee_COMPARE_ERR)
		goto err;
	return cmp == 0 ? 1 : 0;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) size_t DCALL
so_mh_seq_count_with_key(SeqOne *self, DeeObject *item, DeeObject *key) {
	int cmp;
	DREF DeeObject *keyed_item;
	keyed_item = DeeObject_Call(key, 1, &item);
	if unlikely(!keyed_item)
		goto err;
	cmp = DeeObject_TryCompareKeyEq(keyed_item, self->so_item, key);
	Dee_Decref(keyed_item);
	if (cmp == Dee_COMPARE_ERR)
		goto err;
	return cmp == 0 ? 1 : 0;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
so_mh_seq_count_with_range(SeqOne *self, DeeObject *item,
                           size_t start, size_t end) {
	if likely(start == 0 && end >= 1)
		return so_mh_seq_count(self, item);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
so_mh_seq_count_with_range_and_key(SeqOne *self, DeeObject *item,
                                   size_t start, size_t end,
                                   DeeObject *key) {
	if likely(start == 0 && end >= 1)
		return so_mh_seq_count_with_key(self, item, key);
	return 0;
}


#if defined(DCALL_RETURN_COMMON) || __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define so_mh_seq_contains                    (*(int(DCALL *)(SeqOne *, DeeObject *))&so_mh_seq_count)
#define so_mh_seq_contains_with_key           (*(int(DCALL *)(SeqOne *, DeeObject *, DeeObject *))&so_mh_seq_count_with_key)
#define so_mh_seq_contains_with_range         (*(int(DCALL *)(SeqOne *, DeeObject *, size_t, size_t))&so_mh_seq_count_with_range)
#define so_mh_seq_contains_with_range_and_key (*(int(DCALL *)(SeqOne *, DeeObject *, size_t, size_t, DeeObject *))&so_mh_seq_count_with_range_and_key)
#else /* DCALL_RETURN_COMMON || __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
so_mh_seq_contains(SeqOne *self, DeeObject *item) {
	return (int)(Dee_ssize_t)so_mh_seq_count(self, item);
}
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
so_mh_seq_contains_with_key(SeqOne *self, DeeObject *item, DeeObject *key) {
	return (int)(Dee_ssize_t)so_mh_seq_count_with_key(self, item, key);
}
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
so_mh_seq_contains_with_range(SeqOne *self, DeeObject *item,
                              size_t start, size_t end) {
	return (int)(Dee_ssize_t)so_mh_seq_count_with_range(self, item, start, end);
}
PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
so_mh_seq_contains_with_range_and_key(SeqOne *self, DeeObject *item,
                                      size_t start, size_t end,
                                      DeeObject *key) {
	return (int)(Dee_ssize_t)so_mh_seq_count_with_range_and_key(self, item, start, end, key);
}
#endif /* !DCALL_RETURN_COMMON && __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */


#define so_mh_seq_rlocate so_mh_seq_locate
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
so_mh_seq_locate(SeqOne *self, DeeObject *match, DeeObject *def) {
	int matches = so_mh_seq_any_with_key(self, match);
	if unlikely(matches < 0)
		goto err;
	if (matches)
		def = self->so_item;
	return_reference(def);
err:
	return NULL;
}

#define so_mh_seq_rlocate_with_range so_mh_seq_locate_with_range
PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
so_mh_seq_locate_with_range(SeqOne *self, DeeObject *match,
                            size_t start, size_t end, DeeObject *def) {
	int matches = so_mh_seq_any_with_range_and_key(self, start, end, match);
	if unlikely(matches < 0)
		goto err;
	if (matches)
		def = self->so_item;
	return_reference(def);
err:
	return NULL;
}

#define so_mh_seq_startswith                    so_mh_seq_contains
#define so_mh_seq_startswith_with_key           so_mh_seq_contains_with_key
#define so_mh_seq_startswith_with_range         so_mh_seq_contains_with_range
#define so_mh_seq_startswith_with_range_and_key so_mh_seq_contains_with_range_and_key
#define so_mh_seq_endswith                      so_mh_seq_contains
#define so_mh_seq_endswith_with_key             so_mh_seq_contains_with_key
#define so_mh_seq_endswith_with_range           so_mh_seq_contains_with_range
#define so_mh_seq_endswith_with_range_and_key   so_mh_seq_contains_with_range_and_key

#define so_mh_seq_rfind so_mh_seq_find
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
so_mh_seq_find(SeqOne *self, DeeObject *item, size_t start, size_t end) {
	size_t count = so_mh_seq_count_with_range(self, item, start, end);
	/* count == -1  -> error     -> return (size_t)Dee_COMPARE_ERR; */
	/* count == 0   -> not found -> return (size_t)-1; */
	/* count == 1   -> found     -> return 0; */
	ASSERT(count == (size_t)-1 || count == 0 || count == 1);
#if Dee_COMPARE_ERR != -2
	if (count == (size_t)-1)
		return (size_t)Dee_COMPARE_ERR;
#endif /* Dee_COMPARE_ERR != -2 */
	return count - 1;
}

#define so_mh_seq_rfind_with_key so_mh_seq_find_with_key
PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
so_mh_seq_find_with_key(SeqOne *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t count = so_mh_seq_count_with_range_and_key(self, item, start, end, key);
	/* count == -1  -> error     -> return (size_t)Dee_COMPARE_ERR; */
	/* count == 0   -> not found -> return (size_t)-1; */
	/* count == 1   -> found     -> return 0; */
	ASSERT(count == (size_t)-1 || count == 0 || count == 1);
#if Dee_COMPARE_ERR != -2
	if (count == (size_t)-1)
		return (size_t)Dee_COMPARE_ERR;
#endif /* Dee_COMPARE_ERR != -2 */
	return count - 1;
}

#define so_mh_seq_sorted so_mh_seq_reversed
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
so_mh_seq_reversed(SeqOne *self, size_t start, size_t end) {
	if (start == 0 && end >= 1)
		return_reference((DeeObject *)self);
	return DeeSeq_NewEmpty();
}

#ifdef DCALL_CALLER_CLEANUP
#define so_mh_seq_sorted_with_key (*(DREF DeeObject *(DCALL *)(SeqOne *, size_t, size_t, DeeObject *))&so_mh_seq_sorted)
#else /* DCALL_CALLER_CLEANUP */
PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
so_mh_seq_sorted_with_key(SeqOne *self, size_t start, size_t end, DeeObject *key) {
	(void)key;
	return so_mh_seq_sorted(self, start, end);
}
#endif /* !DCALL_CALLER_CLEANUP */


#define so_mh_seq_bfind          so_mh_seq_find
#define so_mh_seq_bfind_with_key so_mh_seq_find_with_key

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_CompareKey2(DeeObject *lhs, DeeObject *rhs, DeeObject *key) {
	int result;
	lhs = DeeObject_Call(key, 1, &lhs);
	if unlikely(!lhs)
		goto err;
	result = DeeObject_CompareKey(lhs, rhs, key);
	Dee_Decref(lhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
so_mh_seq_bposition(SeqOne *self, DeeObject *item, size_t start, size_t end) {
	if (start == 0 && end >= 1) {
		int cmp = DeeObject_Compare(item, self->so_item);
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
		if (cmp <= 0)
			return 0;
		return 1;
	}
	return 0;
err:
	return (size_t)Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
so_mh_seq_bposition_with_key(SeqOne *self, DeeObject *item,
                             size_t start, size_t end, DeeObject *key) {
	if (start == 0 && end >= 1) {
		int cmp = DeeObject_CompareKey2(item, self->so_item, key);
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
		if (cmp <= 0)
			return 0;
		return 1;
	}
	return 0;
err:
	return (size_t)Dee_COMPARE_ERR;
}


PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
so_mh_seq_brange(SeqOne *self, DeeObject *item,
                 size_t start, size_t end,
                 size_t result_range[2]) {
	if (start == 0 && end >= 1) {
		int cmp = DeeObject_Compare(item, self->so_item);
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
		if (cmp < 0) {
			result_range[0] = 0;
			result_range[1] = 0;
		} else if (cmp == 0) {
			result_range[0] = 0;
			result_range[1] = 1;
		} else {
			result_range[0] = 1;
			result_range[1] = 1;
		}
	} else {
		result_range[0] = 0;
		result_range[1] = 0;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
so_mh_seq_brange_with_key(SeqOne *self, DeeObject *item,
                          size_t start, size_t end, DeeObject *key,
                          size_t result_range[2]) {
	if (start == 0 && end >= 1) {
		int cmp = DeeObject_CompareKey2(item, self->so_item, key);
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
		if (cmp < 0) {
			result_range[0] = 0;
			result_range[1] = 0;
		} else if (cmp == 0) {
			result_range[0] = 0;
			result_range[1] = 1;
		} else {
			result_range[0] = 1;
			result_range[1] = 1;
		}
	} else {
		result_range[0] = 0;
		result_range[1] = 0;
	}
	return 0;
err:
	return -1;
}


#define so_mh_set_operator_iter    so_iter
#define so_mh_set_operator_foreach so_foreach
#define so_mh_set_operator_sizeob  so_sizeob
#define so_mh_set_operator_size    so_size
#define so_mh_set_operator_hash    so_hash
#define so_mh_map_operator_iter    so_iter
#define so_mh_map_operator_sizeob  so_sizeob
#define so_mh_map_operator_size    so_size
#define so_mh_map_operator_hash    so_hash

PRIVATE struct type_math so_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ DEFIMPL(&default__set_operator_inv),
	/* .tp_pos         = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg         = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&so_add,
	/* .tp_sub         = */ DEFIMPL(&default__set_operator_sub),
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&so_mul,
	/* .tp_div         = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod         = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl         = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr         = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and         = */ DEFIMPL(&default__set_operator_and),
	/* .tp_or          = */ DEFIMPL(&default__set_operator_add),
	/* .tp_xor         = */ DEFIMPL(&default__set_operator_xor),
	/* .tp_pow         = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__set_operator_inplace_sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ DEFIMPL(&default__set_operator_inplace_and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__set_operator_inplace_add),
	/* .tp_inplace_xor = */ DEFIMPL(&default__set_operator_inplace_xor),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

PRIVATE struct type_cmp so_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&so_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&so_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&so_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&so_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_seq so_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&so_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&so_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&so_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&so_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&so_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&so_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&so_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&so_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&so_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&so_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&so_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&so_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&so_trygetitem_index,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&so_asvector,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&so_asvector_nothrow,
};

PRIVATE struct type_method tpconst so_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_enumerate_items__),
	TYPE_METHOD_HINTREF(Sequence_unpack),
	TYPE_METHOD_HINTREF(Sequence_unpackub),
	TYPE_METHOD_HINTREF(Sequence_any),
	TYPE_METHOD_HINTREF(Sequence_all),
	TYPE_METHOD_HINTREF(Sequence_parity),
	TYPE_METHOD_HINTREF(Sequence_reduce),
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
	TYPE_METHOD_HINTREF(__set_size__),
	TYPE_METHOD_HINTREF(__set_hash__),
//	TYPE_METHOD_HINTREF(__set_compare_eq__),
	TYPE_METHOD_HINTREF(__map_iter__),
	TYPE_METHOD_HINTREF(__map_size__),
	TYPE_METHOD_HINTREF(__map_hash__),
//	TYPE_METHOD_HINTREF(__map_getitem__),
//	TYPE_METHOD_HINTREF(__map_contains__),
//	TYPE_METHOD_HINTREF(__map_enumerate__),
//	TYPE_METHOD_HINTREF(__map_enumerate_items__),
//	TYPE_METHOD_HINTREF(__map_compare_eq__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst so_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate, &so_mh_seq_enumerate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &so_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_makeenumeration, &so_mh_seq_makeenumeration, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_makeenumeration_with_intrange, &so_mh_seq_makeenumeration_with_intrange, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &so_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &so_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack, &so_mh_seq_unpack, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack_ex, &so_mh_seq_unpack_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_unpack_ub, &so_mh_seq_unpack_ub, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &so_mh_seq_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &so_mh_seq_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any, &so_mh_seq_any, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_key, &so_mh_seq_any_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &so_mh_seq_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &so_mh_seq_any_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all, &so_mh_seq_all, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &so_mh_seq_all_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, &so_mh_seq_all_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &so_mh_seq_all_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity, &so_mh_seq_parity, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_key, &so_mh_seq_parity_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range, &so_mh_seq_parity_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range_and_key, &so_mh_seq_parity_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce, &so_mh_seq_reduce, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce_with_init, &so_mh_seq_reduce_with_init, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce_with_range, &so_mh_seq_reduce_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reduce_with_range_and_init, &so_mh_seq_reduce_with_range_and_init, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min, &so_mh_seq_min, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_key, &so_mh_seq_min_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range, &so_mh_seq_min_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range_and_key, &so_mh_seq_min_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max, &so_mh_seq_max, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_key, &so_mh_seq_max_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range, &so_mh_seq_max_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range_and_key, &so_mh_seq_max_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum, &so_mh_seq_sum, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sum_with_range, &so_mh_seq_sum_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count, &so_mh_seq_count, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_key, &so_mh_seq_count_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range, &so_mh_seq_count_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &so_mh_seq_count_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains, &so_mh_seq_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &so_mh_seq_contains_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &so_mh_seq_contains_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &so_mh_seq_contains_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_locate, &so_mh_seq_locate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_locate_with_range, &so_mh_seq_locate_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rlocate, &so_mh_seq_rlocate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rlocate_with_range, &so_mh_seq_rlocate_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith, &so_mh_seq_startswith, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_key, &so_mh_seq_startswith_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_range, &so_mh_seq_startswith_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_range_and_key, &so_mh_seq_startswith_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith, &so_mh_seq_endswith, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_key, &so_mh_seq_endswith_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_range, &so_mh_seq_endswith_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_range_and_key, &so_mh_seq_endswith_with_range_and_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find, &so_mh_seq_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &so_mh_seq_find_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &so_mh_seq_rfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &so_mh_seq_rfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reversed, &so_mh_seq_reversed, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted, &so_mh_seq_sorted, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_sorted_with_key, &so_mh_seq_sorted_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bfind, &so_mh_seq_bfind, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bfind_with_key, &so_mh_seq_bfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bposition, &so_mh_seq_bposition, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_bposition_with_key, &so_mh_seq_bposition_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_brange, &so_mh_seq_brange, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_brange_with_key, &so_mh_seq_brange_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_iter, &so_mh_set_operator_iter, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_foreach, &so_mh_set_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_sizeob, &so_mh_set_operator_sizeob, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_size, &so_mh_set_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_operator_hash, &so_mh_set_operator_hash, METHOD_FNOREFESCAPE),
	/* TODO: */
//	TYPE_METHOD_HINT_F(set_operator_compare_eq, &so_mh_set_operator_compare_eq, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(set_operator_trycompare_eq, &so_mh_set_operator_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_iter, &so_mh_map_operator_iter, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_foreach_pair, &so_mh_map_operator_foreach_pair, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_sizeob, &so_mh_map_operator_sizeob, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_size, &so_mh_map_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_operator_hash, &so_mh_map_operator_hash, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_getitem, &so_mh_map_operator_getitem, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_trygetitem, &so_mh_map_operator_trygetitem, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_bounditem, &so_mh_map_operator_bounditem, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_hasitem, &so_mh_map_operator_hasitem, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_contains, &so_mh_map_operator_contains, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_enumerate, &so_mh_map_enumerate, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_makeenumeration, &so_mh_map_makeenumeration, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_compare_eq, &so_mh_map_operator_compare_eq, METHOD_FNOREFESCAPE),
//	TYPE_METHOD_HINT_F(map_operator_trycompare_eq, &so_mh_map_operator_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst so_getsets[] = {
	TYPE_GETTER_AB_F_NODOC(STR_first, &so_getfirst, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_GETTER_AB_F_NODOC(STR_last, &so_getlast, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_GETTER_AB_F(STR_cached, &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_AB_F(STR_frozen, &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_AB_F(STR___set_frozen__, &generic_obj__asset, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_AB_F(STR___map_frozen__, &generic_obj__asmap, METHOD_FCONSTCALL, "->?."),
	/* TODO: */
//	TYPE_GETTER_AB_F_NODOC(STR___map_keys__, &so_mh_map_keys, METHOD_FNOREFESCAPE),
//	TYPE_GETTER_AB_F_NODOC(STR___map_iterkeys__, &so_mh_map_iterkeys, METHOD_FNOREFESCAPE),
//	TYPE_GETTER_AB_F_NODOC(STR___map_values__, &so_mh_map_values, METHOD_FNOREFESCAPE),
//	TYPE_GETTER_AB_F_NODOC(STR___map_itervalues__, &so_mh_map_itervalues, METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst so_members[] = {
	TYPE_MEMBER_CONST("length", DeeInt_One),
	TYPE_MEMBER_FIELD("__item__", STRUCT_OBJECT, offsetof(SeqOne, so_item)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst so_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqOneIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeSeqOne_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqOne",
	/* .tp_doc      = */ DOC("Specialized sequence type that always contains exactly 1 item\n"
	                         "\n"
	                         "(item)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&so_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&so_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&so_init,
				TYPE_FIXED_ALLOCATOR(SeqOne)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&so_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&so_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&so_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&so_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &so_math,
	/* .tp_cmp           = */ &so_cmp,
	/* .tp_seq           = */ &so_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ so_methods,
	/* .tp_getsets       = */ so_getsets,
	/* .tp_members       = */ so_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ so_class_members,
	/* .tp_method_hints  = */ so_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


/* Construct a some-wrapper for `self' */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_PackOne(DeeObject *__restrict item) {
	DREF DeeSeqOneObject *result = DeeObject_MALLOC(DeeSeqOneObject);
	if unlikely(!result)
		goto err;
	Dee_Incref(item);
	result->so_item = item;
	DeeObject_Init(result, &DeeSeqOne_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_PackOneInherited(/*inherit(always)*/ DREF DeeObject *__restrict item) {
	DREF DeeSeqOneObject *result = DeeObject_MALLOC(DeeSeqOneObject);
	if unlikely(!result)
		goto err;
	result->so_item = item; /* Inherited */
	DeeObject_Init(result, &DeeSeqOne_Type);
	return (DREF DeeObject *)result;
err:
	Dee_Decref(item); /* Inherited */
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_PackOneInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict item) {
	DREF DeeSeqOneObject *result = DeeObject_MALLOC(DeeSeqOneObject);
	if unlikely(!result)
		goto err;
	result->so_item = item; /* Inherited */
	DeeObject_Init(result, &DeeSeqOne_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


/* Pack a single-item sequence using a symbolic reference */
PUBLIC NONNULL((1)) void DCALL
DeeSeqOne_DecrefSymbolic(DREF DeeObject *__restrict self) {
	SeqOne *me = (SeqOne *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSeqOne_Type);
	if (!DeeObject_IsShared(me)) {
		DeeObject_FREE(me);
		Dee_DecrefNokill(&DeeSeqOne_Type);
	} else {
		Dee_Incref(me->so_item);
		Dee_Decref_unlikely(me);
	}
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_ONE_C */
