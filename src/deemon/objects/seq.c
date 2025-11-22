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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_C
#define GUARD_DEEMON_OBJECTS_SEQ_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memset */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/kwlist.h"
#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/combinations.h"
#include "seq/default-iterators.h"
#include "seq/default-sequences.h"
#include "seq/each.h"
#include "seq/enumerate-cb.h"
#include "seq/filter.h"
#include "seq/flat.h"
#include "seq/hashfilter.h"
#include "seq/mapped.h"
#include "seq/repeat.h"
#include "seq/segments.h"
#include "seq/simpleproxy.h"
#include "seq/unique-iterator.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

/* Provide aliases for certain Set operators in Sequence */
#undef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
#define CONFIG_HAVE_SET_OPERATORS_IN_SEQ

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#define do_fix_negative_range_index(index, size) \
	((size) - ((size_t)(-(index)) % (size)))


/* Clamp a range, as given to `operator [:]' & friends to the bounds
 * accepted by the associated sequence. This handles stuff like negative
 * index over-roll and past-the-end truncation. */
PUBLIC ATTR_INOUT(1) void DCALL
DeeSeqRange_DoClamp(struct Dee_seq_range *__restrict self,
                    size_t size) {
	/* Fix invalid start indices. */
	if (self->sr_start >= size) {
		if (self->sr_istart >= 0)
			goto empty_range; /* Range starts at too great of an index. */

		/* Fast-case for when `-1' is used (or anything with an
		 * absolute value less than the sequence's size) */
		self->sr_istart += size;
		if unlikely(self->sr_istart < 0) {
			/* Check for special case: empty sequence (else the mod will
			 * fault due to divide-by-zero) -> only valid range is [0:0] */
			if unlikely(size == 0)
				goto empty_range;
			self->sr_start = do_fix_negative_range_index(self->sr_istart, size);
		}
	}
	ASSERT(self->sr_start <= size);

	/* Fix invalid end indices. */
	if (self->sr_end > size) {
		if (self->sr_iend < 0) {
			self->sr_iend += size;
			if unlikely(self->sr_iend < 0) {
				/* Check for special case: empty sequence (else the mod will
				 * fault due to divide-by-zero) -> only valid range is [0:0] */
				if unlikely(size == 0)
					goto empty_range;
				self->sr_end = do_fix_negative_range_index(self->sr_iend, size);
			}
		} else {
			self->sr_end = size;
		}
	}
	ASSERT(self->sr_end <= size);

	/* Fix range-end happening before range-start. */
	if unlikely(self->sr_end < self->sr_start)
		self->sr_end = self->sr_start;
	return;
empty_range:
	self->sr_start = size;
	self->sr_end   = size;
}

/* Specialized version of `DeeSeqRange_DoClamp()' for `[istart:none]' range expressions. */
PUBLIC ATTR_CONST WUNUSED size_t DCALL
DeeSeqRange_DoClamp_n(Dee_ssize_t start, size_t size) {
	if likely((size_t)start >= size) {
		if (start >= 0)
			goto empty_range;
		start += size;
		if unlikely((size_t)start >= size) {
			if unlikely(size == 0)
				goto empty_range;
			start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
		}
	}
	return (size_t)start;
empty_range:
	return size;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
seqtype_get_Iterator(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_seq_operator_iter_t seq_operator_iter = DeeType_RequireMethodHint(self, seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty) {
		/*result = &DeeIterator_Type;*/
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast) {
		result = &DefaultIterator_WithSizeAndGetItemIndexFast_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		result = &DefaultIterator_WithSizeAndTryGetItemIndex_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index) {
		result = &DefaultIterator_WithSizeAndGetItemIndex_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem_index) {
		result = &DefaultIterator_WithGetItemIndex_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem) {
		result = &DefaultIterator_WithSizeObAndGetItem_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem) {
		result = &DefaultIterator_WithGetItem_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate) {
		/* TODO */
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate) {
		/* TODO */
	} else if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate_index) {
		/* TODO */
	} else if (seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem) {
		result = &DefaultIterator_WithIterKeysAndTryGetItemMap_Type;
	} else if (seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem) {
		result = &DefaultIterator_WithIterKeysAndGetItemMap_Type;
	} else if (seq_operator_iter == &default__set_operator_iter__with__seq_operator_iter) {
		result = &DistinctIterator_Type;
	}
	return_reference_(result);
}

struct foreach_seq_printrepr_data {
	Dee_formatprinter_t fsprd_printer; /* [1..1] Underlying printer. */
	void               *fsprd_arg;     /* [?..?] Cookie for `fsprd_printer' */
	bool                fsprd_first;   /* Is this the first element? */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
foreach_seq_printrepr_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t temp, result;
	struct foreach_seq_printrepr_data *data;
	data = (struct foreach_seq_printrepr_data *)arg;
	if (data->fsprd_first) {
		data->fsprd_first = false;
		return DeeObject_PrintRepr(elem, data->fsprd_printer, data->fsprd_arg);
	}
	result = DeeFormat_PRINT(data->fsprd_printer, data->fsprd_arg, ", ");
	if likely(result >= 0) {
		temp = DeeObject_PrintRepr(elem, data->fsprd_printer, data->fsprd_arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
default_seq_printrepr_impl(DeeObject *__restrict self,
                           Dee_formatprinter_t printer, void *arg,
                           DeeMH_seq_operator_foreach_t seq_foreach) {
#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
	struct foreach_seq_printrepr_data data;
	Dee_ssize_t temp, result = 0;
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	data.fsprd_printer = printer;
	data.fsprd_arg     = arg;
	data.fsprd_first   = true;
	DO(err, (*seq_foreach)(self, &foreach_seq_printrepr_cb, &data));
	DO(err, data.fsprd_first ? DeeFormat_PRINT(printer, arg, "}")
	                         : DeeFormat_PRINT(printer, arg, " }"));
done:
	return result;
err:
	return temp;
#undef DO
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return default_seq_printrepr_impl(self, printer, arg, DeeObject_RequireMethodHint(self, seq_operator_foreach));
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return default_seq_printrepr_impl(self, printer, arg, DeeObject_RequireMethodHint(self, set_operator_foreach));
}


#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
#define IF_HAVE_SET_OPERATORS_IN_SEQ(x) x
#else /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
#define IF_HAVE_SET_OPERATORS_IN_SEQ(x) NULL
#endif /* !CONFIG_HAVE_SET_OPERATORS_IN_SEQ */

PRIVATE struct type_math seq_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_inv),
	/* .tp_pos         = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg         = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add         = */ &default__seq_operator_add,
	/* .tp_sub         = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_sub),
	/* .tp_mul         = */ &default__seq_operator_mul,
	/* .tp_div         = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod         = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl         = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr         = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and         = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_and),
	/* .tp_or          = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_add),
	/* .tp_xor         = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_xor),
	/* .tp_pow         = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL_UNSUPPORTED(&default__inc__unsupported),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ &default__seq_operator_inplace_add,
	/* .tp_inplace_sub = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_inplace_sub),
	/* .tp_inplace_mul = */ &default__seq_operator_inplace_mul,
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_inplace_and),
	/* .tp_inplace_or  = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_inplace_add),
	/* .tp_inplace_xor = */ IF_HAVE_SET_OPERATORS_IN_SEQ(&default__set_operator_inplace_xor),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
seq_Frozen_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSeq_Type;
	DeeMH_seq_frozen_t seq_frozen = DeeType_RequireMethodHint(self, seq_frozen);
	if (seq_frozen == &DeeObject_NewRef) {
		result = self;
	} else if (seq_frozen == &DeeTuple_FromSequence) {
		result = &DeeTuple_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get___seq_getitem_always_bound__(DeeTypeObject *__restrict self) {
	bool has = DeeType_HasTrait(self, DeeType_TRAIT___seq_getitem_always_bound__);
	return_bool(has);
}

PRIVATE struct type_getset tpconst seq_class_getsets[] = {
	TYPE_GETTER(STR_Iterator, &seqtype_get_Iterator,
	            "->?DType\n"
	            "Returns the Iterator class used by instances of @this Sequence type\n"
	            "Should a sub-class implement its own Iterator, this attribute should be overwritten"),
	TYPE_GETTER(STR_Frozen, &seq_Frozen_get,
	            "->?DType\n"
	            "Returns the type of Sequence returned by the #i:frozen property"),
	TYPE_GETTER("__seq_getitem_always_bound__", &seq_get___seq_getitem_always_bound__,
	            "->?Dbool\n"
	            "Evaluates to ?t if ?#{op:getitem} never throws :UnboundItem\n"
	            "\n"
	            "Sub-classes that implement ${Sequence.operator []} (or unrelated classes "
	            /**/ "that define ${__seq_getitem__}), such that it never throws "
	            /**/ ":UnboundItem errors should override this property like: "
	            /**/ "${public static final __seq_getitem_always_bound__ = true}. "
	            /**/ "Doing so allows the deemon runtime to implement generated "
	            /**/ "sequence functions more efficiently in some cases. In order "
	            /**/ "for deemon to see and understand the attribute, it #Bmust be "
	            /**/ "written exactly as seen in the example. It may not be a static "
	            /**/ "property, or evaluate to something other than ?t. Otherwise, "
	            /**/ "the hint is ignored and may as well not be present at all."),
	TYPE_GETSET_END
};


/* === General-purpose Sequence methods. === */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_empty_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("empty", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "empty");
/*[[[end]]]*/
	result = DeeObject_InvokeMethodHint(seq_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_front_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("front", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "front");
/*[[[end]]]*/
	return DeeObject_InvokeMethodHint(seq_getfirst, self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_back_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("back", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "back");
/*[[[end]]]*/
	return DeeObject_InvokeMethodHint(seq_getlast, self);
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_filter(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("filter", params: "
	keep:?DCallable
", docStringPrefix: "seq");]]]*/
#define seq_filter_params "keep:?DCallable"
	struct {
		DeeObject *keep;
	} args;
	DeeArg_Unpack1(err, argc, argv, "filter", &args.keep);
/*[[[end]]]*/
	return DeeSeq_Filter(self, args.keep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_ubfilter(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ubfilter", params: "
	keep:?DCallable
", docStringPrefix: "seq");]]]*/
#define seq_ubfilter_params "keep:?DCallable"
	struct {
		DeeObject *keep;
	} args;
	DeeArg_Unpack1(err, argc, argv, "ubfilter", &args.keep);
/*[[[end]]]*/
	return DeeSeq_FilterAsUnbound(self, args.keep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_map(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("map", params: "
	mapper:?DCallable
", docStringPrefix: "seq");]]]*/
#define seq_map_params "mapper:?DCallable"
	struct {
		DeeObject *mapper;
	} args;
	DeeArg_Unpack1(err, argc, argv, "map", &args.mapper);
/*[[[end]]]*/
	return DeeSeq_Map(self, args.mapper);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_segments(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("segments", params: "
	size_t segment_length
", docStringPrefix: "seq");]]]*/
#define seq_segments_params "segment_length:?Dint"
	struct {
		size_t segment_length;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "segments", &args.segment_length, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if unlikely(!args.segment_length)
		goto err_invalid_segsize;
	return DeeSeq_Segments(self, args.segment_length);
err_invalid_segsize:
	err_invalid_segment_size(args.segment_length);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distribute(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t mylen;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("distribute", params: "
	size_t segment_count
", docStringPrefix: "seq");]]]*/
#define seq_distribute_params "segment_count:?Dint"
	struct {
		size_t segment_count;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "distribute", &args.segment_count, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if unlikely(!args.segment_count)
		goto err_invalid_segment_count;
	mylen = DeeObject_InvokeMethodHint(seq_operator_size, self);
	if unlikely(mylen == (size_t)-1)
		goto err;
	mylen += args.segment_count - 1;
	mylen /= args.segment_count;
	if unlikely(!mylen)
		return DeeSeq_NewEmpty();
	return DeeSeq_Segments(self, mylen);
err_invalid_segment_count:
	err_invalid_distribution_count(args.segment_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_combinations(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("combinations", params: "
	size_t r;
	bool cached = true;
", docStringPrefix: "seq");]]]*/
#define seq_combinations_params "r:?Dint,cached=!t"
	struct {
		size_t r;
		bool cached;
	} args;
	args.cached = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__r_cached, UNPuSIZ "|b:combinations", &args))
		goto err;
/*[[[end]]]*/
	if (args.cached) {
		self = DeeObject_InvokeMethodHint(seq_cached, self);
		if unlikely(!self)
			goto err;
	} else {
		Dee_Incref(self);
	}
	return DeeSeq_Combinations(self, args.r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_repeatcombinations(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("repeatcombinations", params: "
	size_t r;
	bool cached = true;
", docStringPrefix: "seq");]]]*/
#define seq_repeatcombinations_params "r:?Dint,cached=!t"
	struct {
		size_t r;
		bool cached;
	} args;
	args.cached = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__r_cached, UNPuSIZ "|b:repeatcombinations", &args))
		goto err;
/*[[[end]]]*/
	if (args.cached) {
		self = DeeObject_InvokeMethodHint(seq_cached, self);
		if unlikely(!self)
			goto err;
	} else {
		Dee_Incref(self);
	}
	return DeeSeq_RepeatCombinations(self, args.r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_permutations(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("permutations", params: "
	size_t r = (size_t)-1;
	bool cached = true;
", docStringPrefix: "seq");]]]*/
#define seq_permutations_params "r=!-1,cached=!t"
	struct {
		size_t r;
		bool cached;
	} args;
	args.r = (size_t)-1;
	args.cached = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__r_cached, "|" UNPxSIZ "b:permutations", &args))
		goto err;
/*[[[end]]]*/
	if (args.cached) {
		self = DeeObject_InvokeMethodHint(seq_cached, self);
		if unlikely(!self)
			goto err;
	} else {
		Dee_Incref(self);
	}
	return DeeSeq_Permutations2(self, args.r);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_index(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("index", params: "
	item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
", docStringPrefix: "seq");]]]*/
#define seq_index_params "item,start=!0,end=!-1,key:?DCallable=!N"
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:index", &args))
		goto err;
/*[[[end]]]*/
	result = !DeeNone_Check(args.key)
	         ? DeeObject_InvokeMethodHint(seq_find_with_key, self, args.item, args.start, args.end, args.key)
	         : DeeObject_InvokeMethodHint(seq_find, self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_no_item;
	return DeeInt_NewSize(result);
err_no_item:
	DeeRT_ErrItemNotFoundEx(self, args.item, args.start, args.end, args.key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_rindex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("rindex", params: "
	item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
", docStringPrefix: "seq");]]]*/
#define seq_rindex_params "item,start=!0,end=!-1,key:?DCallable=!N"
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:rindex", &args))
		goto err;
/*[[[end]]]*/
	result = !DeeNone_Check(args.key)
	         ? DeeObject_InvokeMethodHint(seq_rfind_with_key, self, args.item, args.start, args.end, args.key)
	         : DeeObject_InvokeMethodHint(seq_rfind, self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_no_item;
	return DeeInt_NewSize(result);
err_no_item:
	DeeRT_ErrItemNotFoundEx(self, args.item, args.start, args.end, args.key);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_byhash(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("byhash", params: "
	DeeObject *template
", docStringPrefix: "seq");]]]*/
#define seq_byhash_params "template"
	struct {
		DeeObject *template_;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__template, "o:byhash", &args))
		goto err;
/*[[[end]]]*/
	return DeeSeq_HashFilter(self, DeeObject_Hash(args.template_));
err:
	return NULL;
}

DOC_DEF(seq_byhash_doc,
        "(" seq_byhash_params ")->?DSequence\n"
        "#ptemplate{The object who's hash should be used to search for collisions}"
        "Find all objects apart of @this sequence who's hash matches that of @template\n"
        "Note that when hashing ?Dint objects, integers who's value lies within the range "
        /**/ "of valid hash values get hashed to their original value, meaning that the following "
        /**/ "two uses of this function are identical (because ${x.operator hash() == x} when $x "
        /**/ "is an integer that contains a valid hash value):\n"
        "${"
        /**/ "local a = seq.byhash(\"foo\");\n"
        /**/ "local b = seq.byhash(\"foo\".operator hash());\n"
        "}\n"
        "The intended use-case is to query contained elements of mappings/sets by-hash, "
        /**/ "rather than by-key, thus allowing user-code more control in regards to is-contained/"
        /**/ "lookup-element, specifically in scenarios where objects are used as keys that are "
        /**/ "expensive to construct, such that "
        /**/ "${return myDict[ConstructNewCopyOfKey(values...)]} "
        /**/ "can be written more efficiently as "
        /**/ "${for (local e: myDict.byhash(hashOfKeyFromValues(values...))) if (e.equals(values...)) return e;}");


#ifdef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE
#else /* CONFIG_NO_DEEMON_100_COMPAT */
INTERN /* Needed for alias `List.unique' */
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distinct(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DistinctSetWithKey *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("distinct", params: "
	DeeObject *key: ?DCallable = NULL;
", docStringPrefix: "seq");]]]*/
#define seq_distinct_params "key?:?DCallable"
	struct {
		DeeObject *key;
	} args;
	args.key = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__key, "|o:distinct", &args))
		goto err;
/*[[[end]]]*/
	if (!args.key)
		return DeeSuper_New(&DeeSet_Type, self);
	result = DeeObject_MALLOC(DistinctSetWithKey);
	if unlikely(!result)
		goto err;
	result->dswk_key = args.key;
	Dee_Incref(args.key);
	result->dswk_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DistinctSetWithKey_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bcontains(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("bcontains", params: "
	item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
", docStringPrefix: "seq");]]]*/
#define seq_bcontains_params "item,start=!0,end=!-1,key:?DCallable=!N"
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:bcontains", &args))
		goto err;
/*[[[end]]]*/
	result = !DeeNone_Check(args.key)
	         ? DeeObject_InvokeMethodHint(seq_bfind_with_key, self, args.item, args.start, args.end, args.key)
	         : DeeObject_InvokeMethodHint(seq_bfind, self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return_bool(result != (size_t)-1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bindex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("bindex", params: "
	item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
", docStringPrefix: "seq");]]]*/
#define seq_bindex_params "item,start=!0,end=!-1,key:?DCallable=!N"
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:bindex", &args))
		goto err;
/*[[[end]]]*/
	result = !DeeNone_Check(args.key)
	         ? DeeObject_InvokeMethodHint(seq_bfind_with_key, self, args.item, args.start, args.end, args.key)
	         : DeeObject_InvokeMethodHint(seq_bfind, self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_not_found;
	return DeeInt_NewSize(result);
err_not_found:
	DeeRT_ErrItemNotFoundEx(self, args.item, args.start, args.end, args.key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_blocateall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result_range[2];
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("blocateall", params: "
	item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
", docStringPrefix: "seq");]]]*/
#define seq_blocateall_params "item,start=!0,end=!-1,key:?DCallable=!N"
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:blocateall", &args))
		goto err;
/*[[[end]]]*/
	if (!DeeNone_Check(args.key)
	    ? DeeObject_InvokeMethodHint(seq_brange_with_key, self, args.item, args.start, args.end, args.key, result_range)
	    : DeeObject_InvokeMethodHint(seq_brange, self, args.item, args.start, args.end, result_range))
		goto err;
	return DeeObject_InvokeMethodHint(seq_operator_getrange_index, self,
	                                  result_range[0], result_range[1]);
err:
	return NULL;
}

#ifdef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE
#else /* CONFIG_NO_DEEMON_100_COMPAT */
INTERN /* Needed for alias `List.sorted_insert' */
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_binsert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("binsert", params: "
	item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
", docStringPrefix: "seq");]]]*/
#define seq_binsert_params "item,start=!0,end=!-1,key:?DCallable=!N"
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:binsert", &args))
		goto err;
/*[[[end]]]*/
	index = !DeeNone_Check(args.key)
	        ? DeeObject_InvokeMethodHint(seq_bposition_with_key, self, args.item, args.start, args.end, args.key)
	        : DeeObject_InvokeMethodHint(seq_bposition, self, args.item, args.start, args.end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(DeeObject_InvokeMethodHint(seq_insert, self, index, args.item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because aliased by `List.pop_front' */
seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("popfront", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "popfront");
/*[[[end]]]*/
	return DeeObject_InvokeMethodHint(seq_pop, self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because aliased by `List.pop_back' */
seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("popback", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "popback");
/*[[[end]]]*/
	return DeeObject_InvokeMethodHint(seq_pop, self, -1);
err:
	return NULL;
}


/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("first");
print define_Dee_HashStr("last");
print define_Dee_HashStr("keys");
print define_Dee_HashStr("values");
print define_Dee_HashStr("iterkeys");
print define_Dee_HashStr("itervalues");
print define_Dee_HashStr("cb");
print define_Dee_HashStr("start");
print define_Dee_HashStr("end");
]]]*/
#define Dee_HashStr__first _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)
#define Dee_HashStr__last _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)
#define Dee_HashStr__keys _Dee_HashSelectC(0x97e36be1, 0x654d31bc4825131c)
#define Dee_HashStr__values _Dee_HashSelectC(0x33b551c8, 0xf6e3e991b86d1574)
#define Dee_HashStr__iterkeys _Dee_HashSelectC(0x62bd6adc, 0x535ac8ab28094ab3)
#define Dee_HashStr__itervalues _Dee_HashSelectC(0xcb00bab3, 0xe9a89082a994930a)
#define Dee_HashStr__cb _Dee_HashSelectC(0x75ffadba, 0x2501dbb50208b92e)
#define Dee_HashStr__start _Dee_HashSelectC(0xa2ed6890, 0x80b621ce3c3982d5)
#define Dee_HashStr__end _Dee_HashSelectC(0x37fb4a05, 0x6de935c204dc3d01)
/*[[[end]]]*/

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
do_seq_enumerate_with_kw(DeeObject *self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeObject *cb, *startob, *endob;
	size_t start, end;
	DeeKwArgs kwds;
	if (DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
	switch (argc) {

	case 0: {
		if unlikely((cb = DeeKwArgs_TryGetItemNRStringHash(&kwds, "cb", Dee_HashStr__cb)) == NULL)
			goto err;
		if unlikely((startob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
			goto err;
		if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (cb != ITER_DONE) {
handle_with_cb:
			if (endob != ITER_DONE) {
				if (startob != ITER_DONE) {
					if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
					    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
						result = seq_call_enumerate_with_intrange(self, cb, start, end);
					} else {
						result = seq_call_enumerate_with_range(self, cb, startob, endob);
					}
				} else if (DeeInt_Check(endob) && DeeInt_TryAsSize(endob, &end)) {
					result = seq_call_enumerate_with_intrange(self, cb, 0, end);
				} else {
					startob = DeeObject_NewDefault(Dee_TYPE(endob));
					if unlikely(!startob)
						goto err;
					result = seq_call_enumerate_with_range(self, cb, startob, endob);
					Dee_Decref(startob);
				}
			} else if (startob == ITER_DONE) {
				result = seq_call_enumerate(self, cb);
			} else {
				ASSERT(startob != ITER_DONE);
				ASSERT(endob == ITER_DONE);
				if (DeeObject_AsSize(startob, &start))
					goto err;
				result = seq_call_enumerate_with_intrange(self, cb, start, (size_t)-1);
			}
		} else {
			if (endob != ITER_DONE) {
				if (startob != ITER_DONE) {
					if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
					    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
						result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, end);
					} else {
						result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_range, self, startob, endob);
					}
				} else if (DeeInt_Check(endob) && DeeInt_TryAsSize(endob, &end)) {
					result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, 0, end);
				} else {
					startob = DeeObject_NewDefault(Dee_TYPE(endob));
					if unlikely(!startob)
						goto err;
					result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_range, self, startob, endob);
					Dee_Decref(startob);
				}
			} else if (startob == ITER_DONE) {
				result = DeeObject_InvokeMethodHint(seq_makeenumeration, self);
			} else {
				ASSERT(startob != ITER_DONE);
				ASSERT(endob == ITER_DONE);
				if (DeeObject_AsSize(startob, &start))
					goto err;
				result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, (size_t)-1);
			}
		}
	}	break;

	case 1: {
		cb = argv[0];
		if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (DeeCallable_Check(cb)) {
			if unlikely((startob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
				goto err;
			goto handle_with_cb;
		}
		startob = cb;
		if (endob != ITER_DONE) {
			if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
			    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
				result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, end);
			} else {
				result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_range, self, startob, endob);
			}
		} else {
			if (DeeObject_AsSize(startob, &start))
				goto err;
			result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, (size_t)-1);
		}
	}	break;

	case 2: {
		cb = argv[0];
		if (DeeCallable_Check(cb)) {
			if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
				goto err;
			startob = argv[1];
			goto handle_with_cb;
		}
		startob = argv[0];
		endob   = argv[1];
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
			result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, end);
		} else {
			result = DeeObject_InvokeMethodHint(seq_makeenumeration_with_range, self, startob, endob);
		}
	}	break;

	case 3: {
		cb      = argv[0];
		startob = argv[1];
		endob   = argv[2];
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
			result = seq_call_enumerate_with_intrange(self, cb, start, end);
		} else {
			result = seq_call_enumerate_with_range(self, cb, startob, endob);
		}
	}	break;

	default:
		goto err_bad_args;
	}
	if unlikely(DeeKwArgs_Done(&kwds, argc, "enumerate"))
		goto err_r;
	return result;
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
	goto err;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_enumerate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start, end;
	if unlikely(kw)
		return do_seq_enumerate_with_kw(self, argc, argv, kw);
	if likely(argc == 0)
		return DeeObject_InvokeMethodHint(seq_makeenumeration, self);
	if (DeeCallable_Check(argv[0])) {
		if (argc == 1)
			return seq_call_enumerate(self, argv[0]);
		if unlikely(argc == 2) {
			if (DeeObject_AsSize(argv[1], &start))
				goto err;
			return seq_call_enumerate_with_intrange(self, argv[0], start, (size_t)-1);
		}
		if (argc != 3)
			goto err_bad_args;
		if ((DeeInt_Check(argv[1]) && DeeInt_Check(argv[2])) &&
		    (DeeInt_TryAsSize(argv[1], &start) && DeeInt_TryAsSize(argv[2], &end)))
			return seq_call_enumerate_with_intrange(self, argv[0], start, end);
		return seq_call_enumerate_with_range(self, argv[0], argv[1], argv[2]);
	} else {
		if unlikely(argc == 1) {
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			return DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, (size_t)-1);
		}
		if (argc != 2)
			goto err_bad_args;
		if ((DeeInt_Check(argv[0]) && DeeInt_Check(argv[1])) &&
		    (DeeInt_TryAsSize(argv[0], &start) && DeeInt_TryAsSize(argv[1], &end)))
			return DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, start, end);
		return DeeObject_InvokeMethodHint(seq_makeenumeration_with_range, self, argv[0], argv[1]);
	}
	__builtin_unreachable();
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
err:
	return NULL;
}

#define DOC_throws_ValueError_if_empty \
	"#tValueError{The specified range is empty}"
#define DOC_throws_ValueError_if_not_found \
	"#tValueError{The specified range does not contain an element matching @item}"
#define DOC_param_item \
	"#pitem{The item to search for}"
#define DOC_param_key \
	"#pkey{A key function to transform item values}"

INTDEF struct type_method tpconst seq_methods[];
INTERN_TPCONST struct type_method tpconst seq_methods[] = {

	TYPE_METHOD(DeeMA_Sequence_compare_name, &DeeMA_Sequence_compare,
	            "" DeeMA_Sequence_compare_doc "\n"
	            "Used to implement ?Dcompare for Sequence objects. Calling this function "
	            "directly is equivalent to ${deemon.compare(this as Sequence, rhs)}\n"
	            "#T{Requirements|Implementation~"
	            /**/ "${function compare(rhs: Sequence): int} (?A__seqclass__?DType is ?.)" /**/ "|${return this.compare(rhs);}&"
	            /**/ "${function __seq_compare__(rhs: Sequence): int}" /*                     */ "|${return this.__seq_compare__(rhs);}&"
	            /**/ "?#{op:eq}, ?#{op:lo}" /**/ "|${return Sequence.__eq__(this, rhs) ? 0 : Sequence.__lo__(this, rhs) ? -1 : 1;}&"
	            /**/ "?#{op:eq}, ?#{op:le}" /**/ "|${return Sequence.__eq__(this, rhs) ? 0 : Sequence.__le__(this, rhs) ? -1 : 1;}&"
	            /**/ "?#{op:eq}, ?#{op:gr}" /**/ "|${return Sequence.__eq__(this, rhs) ? 0 : Sequence.__gr__(this, rhs) ? 1 : -1;}&"
	            /**/ "?#{op:eq}, ?#{op:ge}" /**/ "|${return Sequence.__eq__(this, rhs) ? 0 : Sequence.__ge__(this, rhs) ? 1 : -1;}&"
	            /**/ "?#{op:ne}, ?#{op:lo}" /**/ "|${return Sequence.__ne__(this, rhs) ? (Sequence.__lo__(this, rhs) ? -1 : 1) : 0;}&"
	            /**/ "?#{op:ne}, ?#{op:le}" /**/ "|${return Sequence.__ne__(this, rhs) ? (Sequence.__le__(this, rhs) ? -1 : 1) : 0;}&"
	            /**/ "?#{op:ne}, ?#{op:gr}" /**/ "|${return Sequence.__ne__(this, rhs) ? (Sequence.__gr__(this, rhs) ? 1 : -1) : 0;}&"
	            /**/ "?#{op:ne}, ?#{op:ge}" /**/ "|${return Sequence.__ne__(this, rhs) ? (Sequence.__ge__(this, rhs) ? 1 : -1) : 0;}&"
	            /**/ "?#{op:lo}, ?#{op:gr}" /**/ "|${return Sequence.__lo__(this, rhs) ? -1 : Sequence.__gr__(this, rhs) ? 1 : 0;}&"
	            /**/ "?#{op:le}, ?#{op:ge}" /**/ "|${return Sequence.__le__(this, rhs) ? (Sequence.__ge__(this, rhs) ? 0 : -1) : 1;}&"
	            /**/ "?#{op:iter}" /*         */ "|${"
	            /**/ /**/ "##define TAKE1(v, iter) ({ local _r = false; foreach(v: iter) { _r = true; break; } _r; })\n"
	            /**/ /**/ "local lhsIter = Sequence.__iter__(this);\n"
	            /**/ /**/ "local rhsIter = rhs.operator iter();\n"
	            /**/ /**/ "for (;;) {\n"
	            /**/ /**/ "	local lhsItem, rhsItem;\n"
	            /**/ /**/ "	if (TAKE1(lhsItem, lhsIter)) {\n"
	            /**/ /**/ "		if (!TAKE1(rhsItem, rhsIter))\n"
	            /**/ /**/ "			return 1;\n"
	            /**/ /**/ "		local cmp = deemon.compare(lhsItem, rhsItem);\n"
	            /**/ /**/ "		if (cmp != 0)\n"
	            /**/ /**/ "			return cmp;\n"
	            /**/ /**/ "	} else {\n"
	            /**/ /**/ "		return TAKE1(rhsItem, rhsIter) ? -1 : 0;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "}"
	            /**/ "}&"
	            "}"),


	TYPE_METHOD(DeeMA_Sequence_equals_name, &DeeMA_Sequence_equals,
	            "(rhs:?X2?DSequence?S?O)->?Dbool\n"
	            "Used to implement ?Dequals for Sequence objects. Calling this function "
	            "directly is equivalent to ${deemon.equals(this as Sequence, rhs)}\n"
	            "#T{Requirements|Implementation~"
	            /**/ "${function equals(rhs: Sequence): int | bool} (?A__seqclass__?DType is ?.)" /**/ "|${"
	            /**/ /**/ "local result = this.equals(rhs);\n"
	            /**/ /**/ "return result is int ? (result == 0) : !!result;"
	            /**/ "}&"
	            /**/ "${function __seq_compare_eq__(rhs: Sequence): int | bool}" /*                 */ "|${"
	            /**/ /**/ "local result = this.__seq_compare_eq__(rhs);\n"
	            /**/ /**/ "return result is int ? (result == 0) : !!result;"
	            /**/ "}&"
	            /**/ "?#{op:eq}" /*  */ "|${return !!Sequence.__eq__(this, rhs);}&"
	            /**/ "?#{op:ne}" /*  */ "|${return !Sequence.__ne__(this, rhs);}&"
	            /**/ "?#{op:iter}" /**/ "|${"
	            /**/ /**/ "##define TAKE1(v, iter) ({ local _r = false; foreach(v: iter) { _r = true; break; } _r; })\n"
	            /**/ /**/ "local lhsIter = Sequence.__iter__(this);\n"
	            /**/ /**/ "local rhsIter = rhs.operator iter();\n"
	            /**/ /**/ "for (;;) {\n"
	            /**/ /**/ "	local lhsItem, rhsItem;\n"
	            /**/ /**/ "	if (TAKE1(lhsItem, lhsIter)) {\n"
	            /**/ /**/ "		if (!TAKE1(rhsItem, rhsIter))\n"
	            /**/ /**/ "			return false;\n"
	            /**/ /**/ "		if (!deemon.equals(lhsItem, rhsItem))\n"
	            /**/ /**/ "			return false;\n"
	            /**/ /**/ "	} else {\n"
	            /**/ /**/ "		return !TAKE1(rhsItem, rhsIter);\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "}"
	            /**/ "}&"
	            /**/ "?#compare" /**/ "|${return Sequence.compare(this, rhs) == 0;}"
	            "}"),


	TYPE_METHOD(DeeMA_Sequence_unpack_name, &DeeMA_Sequence_unpack,
	            "" DeeMA___seq_unpack___doc "\n"
	            "Unpack elements of this sequence (skipping over unbound items), whilst asserting "
	            /**/ "that the resulting sequence's size is either equal to @length, or lies within "
	            /**/ "the (inclusive) bounds ${[minsize, maxsize]}\n"

	            "When called with 1 argument ($length):"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_unpack__(length: int): Tuple}" /**/ "|${"
	            /**/ /**/ "local result = this.__seq_unpack__(length);\n"
	            /**/ /**/ "if (result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "${function unpack(length: int): Tuple} (?A__seqclass__?DType is ?.)" /**/ "|${"
	            /**/ /**/ "local result = this.unpack(length);\n"
	            /**/ /**/ "if (result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "?#{op:size}, ?#{op:getitem} (requires $__seq_getitem_always_bound__)" /**/ "|${"
	            /**/ /**/ "if (Sequence.__size__(this) != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "local result = List(length);\n"
	            /**/ /**/ "for (local i: [:length]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		throw UnpackError(...);\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	result[i] = item;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return Tuple(result);"
	            /**/ "}&"
	            /**/ "?#{op:iter}" /**/ "|${"
	            /**/ /**/ "local result = ();\n"
	            /**/ /**/ "foreach (local item: Sequence.__iter__(this))\n"
	            /**/ /**/ "	result = (result..., item);\n"
	            /**/ /**/ "if (##result != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}"
	            "}\n"

	            "When called with 2 argument ($min, $max):"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_unpack__(min: int, max: int): Tuple}" /**/ "|${"
	            /**/ /**/ "local result = this.__seq_unpack__(min, max);\n"
	            /**/ /**/ "if (result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result < min || ##result > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "${function unpack(min: int, max: int): Tuple}" /**/ "|${"
	            /**/ /**/ "local result = this.unpack(min, max);\n"
	            /**/ /**/ "if (result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result < min || ##result > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "?#{op:size}, ?#{op:getitem} (requires $__seq_getitem_always_bound__)" /**/ "|${"
	            /**/ /**/ "local myLength = Sequence.__size__(this);\n"
	            /**/ /**/ "if (myLength < min || myLength > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "local result = List(myLength);\n"
	            /**/ /**/ "for (local i: [:myLength]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (KeyError) {\n"
	            /**/ /**/ "		throw UnpackError(...);\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	result[i] = item;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return Tuple(result);"
	            /**/ "}&"
	            /**/ "?#{op:iter}" /**/ "|${"
	            /**/ /**/ "local result = ();\n"
	            /**/ /**/ "foreach (local item: Sequence.__iter__(this))\n"
	            /**/ /**/ "	result = (result..., item);\n"
	            /**/ /**/ "if (##result < min || ##result > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}"
	            "}"),


	TYPE_METHOD(DeeMA_Sequence_unpackub_name, &DeeMA_Sequence_unpackub,
	            "" DeeMA___seq_unpackub___doc "\n"
	            "Same as ?#unpack, but don't skip over unbound items. As a consequence, "
	            /**/ "not all of the returned sequence's indices are necessarily bound.\n"

	            "When called with 1 argument ($length):"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_unpackub__(length: int): rt.NullableTuple | Tuple}" /**/ "|${"
	            /**/ /**/ "local result = this.__seq_unpackub__(length);\n"
	            /**/ /**/ "if (result !is rt.NullableTuple && result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "${function unpackub(length: int): rt.NullableTuple} (?A__seqclass__?DType is ?.)" /**/ "|${"
	            /**/ /**/ "local result = this.unpackub(length);\n"
	            /**/ /**/ "if (result !is rt.NullableTuple && result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "?#{op:size}, ?#{op:getitem}" /**/ "|${"
	            /**/ /**/ "if (Sequence.__size__(this) != length)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "local result = collections.FixedList(length);\n"
	            /**/ /**/ "for (local i: [:length]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (KeyError) {\n"
	            /**/ /**/ "		throw UnpackError(...);\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	result[i] = item;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return rt.NullableTuple(result);"
	            /**/ "}&"
	            /**/ "?#unpack" /**/ "|${return Sequence.unpack(this, length);}"
	            "}\n"

	            "When called with 2 argument ($min, $max):"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_unpackub__(min: int, max: int): rt.NullableTuple | Tuple}" /**/ "|${"
	            /**/ /**/ "local result = this.__seq_unpackub__(min, max);\n"
	            /**/ /**/ "if (result !is rt.NullableTuple && result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result < min || ##result > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "${function unpackub(min: int, max: int): rt.NullableTuple}" /**/ "|${"
	            /**/ /**/ "local result = this.unpackub(min, max);\n"
	            /**/ /**/ "if (result !is rt.NullableTuple && result !is Tuple)\n"
	            /**/ /**/ "	throw TypeError(...);\n"
	            /**/ /**/ "if (##result < min || ##result > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "return result;"
	            /**/ "}&"
	            /**/ "?#{op:size}, ?#{op:getitem}" /**/ "|${"
	            /**/ /**/ "local myLength = Sequence.__size__(this);\n"
	            /**/ /**/ "if (myLength < min || myLength > max)\n"
	            /**/ /**/ "	throw UnpackError(...);\n"
	            /**/ /**/ "local result = collections.FixedList(myLength);\n"
	            /**/ /**/ "for (local i: [:myLength]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		throw UnpackError(...);\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	result[i] = item;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return rt.NullableTuple(result);"
	            /**/ "}&"
	            /**/ "?#unpack" /**/ "|${return Sequence.unpack(this, min, max);}"
	            "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_any_name, &DeeMA_Sequence_any,
	              "" DeeMA_Sequence_any_doc "\n"
	              "Returns ?t if any element of @this Sequence evaluates to ?t\n"
	              "If @this Sequence is empty, ?f is returned\n"
	              "This function is used to implement ${this || ...}\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_any__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_any__();}&"
	              /**/ "${function any(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.any();}&"
	              /**/ "?#{op:bool} (?A__seqclass__?DType is ?DMapping)" /*                      */ "|${"
	              /**/ /**/ "/* All sequence-like map items are \"true\" (because they\n"
	              /**/ /**/ " * are non-empty (2-element) tuples). As such, so-long as\n"
	              /**/ /**/ " * a mapping itself is non-empty, there will always exist\n"
	              /**/ /**/ " * an **item** (the 2-element tuple) that evaluations to\n"
	              /**/ /**/ " * true. */\n"
	              /**/ /**/ "return Sequence.__bool__(this);"
	              /**/ "}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (item)\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return false;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_any__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_any__(0, int.SIZE_MAX, key);}&"
	              /**/ "${function any(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.any(0, int.SIZE_MAX, key);}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (key(item))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return false;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_any__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_any__(start, end);}&"
	              /**/ "${function any(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.any(start, end);}&"
	              /**/ "?#{op:bool}, ?A__size__?DMapping (?A__seqclass__?DType is ?DMapping)" /**/ "|${"
	              /**/ /**/ "/* All sequence-like map items are \"true\" (because they\n"
	              /**/ /**/ " * are non-empty (2-element) tuples). As such, so-long as\n"
	              /**/ /**/ " * a mapping itself is non-empty, there will always exist\n"
	              /**/ /**/ " * an **item** (the 2-element tuple) that evaluations to\n"
	              /**/ /**/ " * true. */\n"
	              /**/ /**/ "if (start <= end)\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "if (start == 0)\n"
	              /**/ /**/ "	return Sequence.__bool__(this);\n"
	              /**/ /**/ "return start < Mapping.__size__(this);"
	              /**/ "}&"
	              /**/ "?#__enumerate__" /*                                                     */ "|${"
	              /**/ /**/ "return this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && item)\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end) ?? false;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_any__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_any__(start, end, key);}&"
	              /**/ "${function any(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.any(start, end, key);}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "return this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && key(item))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end) ?? false;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_all_name, &DeeMA_Sequence_all,
	              "" DeeMA_Sequence_all_doc "\n"
	              DOC_param_key
	              "Returns ?t if all elements of @this Sequence evaluate to ?t\n"
	              "If @this Sequence is empty, ?t is returned\n"
	              "This function is used to implement ${this && ...}\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_all__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_all__();}&"
	              /**/ "${function all(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.all();}&"
	              /**/ "?A__seqclass__?DType is ?DMapping" /*                      */ "|${"
	              /**/ /**/ "/* Mappings are made up of non-empty (2-element) tuples, so they can never\n"
	              /**/ /**/ " * have items (the 2-element tuples) that evaluate to \"false\". As such, the\n"
	              /**/ /**/ " * Sequence.all() operator for mappings can return a constant \"true\". */\n"
	              /**/ /**/ "return true;"
	              /**/ "}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (!item)\n"
	              /**/ /**/ "		return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return true;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_all__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_all__(0, int.SIZE_MAX, key);}&"
	              /**/ "${function all(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.all(0, int.SIZE_MAX, key);}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (!key(item))\n"
	              /**/ /**/ "		return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return true;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_all__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_all__(start, end);}&"
	              /**/ "${function all(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.all(start, end);}&"
	              /**/ "?A__seqclass__?DType is ?DMapping" /*                      */ "|${"
	              /**/ /**/ "/* Mappings are made up of non-empty (2-element) tuples, so they can never\n"
	              /**/ /**/ " * have items (the 2-element tuples) that evaluate to \"false\". As such, the\n"
	              /**/ /**/ " * Sequence.all() operator for mappings can return a constant \"true\". */\n"
	              /**/ /**/ "return true;"
	              /**/ "}&"
	              /**/ "?#__enumerate__" /*                                                     */ "|${"
	              /**/ /**/ "return this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && !item)\n"
	              /**/ /**/ "		return false;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end) ?? true;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_all__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_all__(start, end, key);}&"
	              /**/ "${function all(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.all(start, end, key);}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "return this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && !key(item))\n"
	              /**/ /**/ "		return false;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end) ?? true;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_parity_name, &DeeMA_Sequence_parity,
	              "" DeeMA_Sequence_parity_doc "\n"
	              DOC_param_key
	              "Returns ?t or ?f indicative of the parity of Sequence elements that are ?t\n"
	              "If @this Sequence is empty, ?f is returned\n"
	              "Parity here refers to ${##this.filter(x -\\> !!x) % 2}\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_parity__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_parity__();}&"
	              /**/ "${function parity(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.parity();}&"
	              /**/ "?#count (?A__seqclass__?DType is ?DSet or ?DMapping)" /**/ "|${return !!(Sequence.count(this, true) % 1);}&"
	              /**/ "?#{op:iter}" /*                                         */ "|${"
	              /**/ /**/ "local result = false;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (item)\n"
	              /**/ /**/ "		result = !result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_parity__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_parity__(0, int.SIZE_MAX, key);}&"
	              /**/ "${function parity(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.parity(0, int.SIZE_MAX, key);}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result = false;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (key(item))\n"
	              /**/ /**/ "		result = !result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_parity__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_parity__(start, end);}&"
	              /**/ "${function parity(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.parity(start, end);}&"
	              /**/ "?#count (?A__seqclass__?DType is ?DSet or ?DMapping)" /**/ "|${return !!(Sequence.count(this, true, start, end) % 1);}&"
	              /**/ "?#__enumerate__" /*                                                     */ "|${"
	              /**/ /**/ "local result = Cell(false);\n"
	              /**/ /**/ "this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && item)\n"
	              /**/ /**/ "		result.value = !result.value;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_parity__(start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_parity__(start, end, key);}&"
	              /**/ "${function parity(start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.parity(start, end, key);}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "local result = Cell(false);\n"
	              /**/ /**/ "return this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && key(item))\n"
	              /**/ /**/ "		result.value = !result.value;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_reduce_name, &DeeMA_Sequence_reduce,
	              "" DeeMA_Sequence_reduce_doc "\n"
	              DOC_throws_ValueError_if_empty
	              "Combines consecutive elements of @this Sequence by passing them as pairs of 2 to @combine, "
	              /**/ "then re-using its return value in the next invocation, before finally returning its last "
	              /**/ "return value. If the Sequence consists of only 1 element, @combine is never invoked.\n"
	              "When given, @init is used as the initial lhs-operand, rather than the first element of the Sequence\n"

	              "When ${start == 0 && end == -1 && init !is bound}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_reduce__(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_reduce__(combine);}&"
	              /**/ "${function reduce(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.reduce(combine);}&"
	              /**/ "?#{op:iter}" /**/ "|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this))\n"
	              /**/ /**/ "	result = result is bound ? combine(result, item) : item;\n"
	              /**/ /**/ "if (result !is bound)\n"
	              /**/ /**/ "	throw ValueError(...);\n"
	              /**/ /**/ "return result;\n"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && init is bound}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_reduce__(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_reduce__(combine, 0, int.SIZE_MAX, init);}&"
	              /**/ "${function reduce(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.reduce(combine, 0, int.SIZE_MAX, init);}&"
	              /**/ "?#{op:iter}" /**/ "|${"
	              /**/ /**/ "local result = init;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this))\n"
	              /**/ /**/ "	result = combine(result, item);\n"
	              /**/ /**/ "return result;\n"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && init !is bound}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_reduce__(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_reduce__(combine, start, end);}&"
	              /**/ "${function reduce(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.reduce(combine, start, end);}&"
	              /**/ "?#__enumerate__" /**/ "|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "Sequence.__enumerate__(this, (i, item?) -\\> {\n"
	              /**/ /**/ "	result.value = result.value is bound ? combine(result.value, item) : item;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "if (result.value !is bound)\n"
	              /**/ /**/ "	throw ValueError(...);\n"
	              /**/ /**/ "return result.value;\n"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && init is bound}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_reduce__(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_reduce__(combine, start, end, init);}&"
	              /**/ "${function reduce(combine: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.reduce(combine, start, end, init);}&"
	              /**/ "?#__enumerate__" /**/ "|${"
	              /**/ /**/ "local result = Cell(init);\n"
	              /**/ /**/ "Sequence.__enumerate__(this, (i, item?) -\\> {\n"
	              /**/ /**/ "	result.value = combine(result.value, item);\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;\n"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_min_name, &DeeMA_Sequence_min,
	              "" DeeMA_Sequence_min_doc "\n"
	              DOC_param_key
	              "Returns the smallest element of @this Sequence. "
	              /**/ "If @this Sequence is empty, ?N is returned.\n"
	              "This function is used to implement ${this < ...}\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_min__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_min__();}&"
	              /**/ "${function min(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.min();}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (result !is bound || !(result < item))\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result is bound ? result : none;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_min__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_min__(0, int.SIZE_MAX, key);}&"
	              /**/ "${function min(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.min(0, int.SIZE_MAX, key);}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "local keyedResult;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	local keyedItem = key(item);\n"
	              /**/ /**/ "	if (keyedResult !is bound || !(keyedResult < keyedItem)) {\n"
	              /**/ /**/ "		keyedResult = keyedItem;\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result is bound ? result : none;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_min__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_min__(start, end);}&"
	              /**/ "${function min(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.min(start, end);}&"
	              /**/ "?#__enumerate__" /*                                                     */ "|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && (result.value !is bound || !(result.value < item)))\n"
	              /**/ /**/ "		result.value = item;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value is bound ? result.value : none;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_min__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_min__(start, end, key);}&"
	              /**/ "${function min(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.min(start, end, key);}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "local keyedResult = Cell();\n"
	              /**/ /**/ "this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound) {\n"
	              /**/ /**/ "		local keyedItem = key(item);\n"
	              /**/ /**/ "		if (keyedResult.value !is bound || !(keyedResult.value < keyedItem)) {\n"
	              /**/ /**/ "			keyedResult.value = keyedItem;\n"
	              /**/ /**/ "			result.value = item;\n"
	              /**/ /**/ "		}\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value is bound ? result.value : none;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_max_name, &DeeMA_Sequence_max,
	              "" DeeMA_Sequence_max_doc "\n"
	              DOC_param_key
	              "Returns the greatest element of @this Sequence. "
	              /**/ "If @this Sequence is empty, ?N is returned.\n"
	              "This function is used to implement ${this > ...}\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_max__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_max__();}&"
	              /**/ "${function max(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.max();}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (result !is bound || !(result > item))\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result is bound ? result : none;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_max__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_max__(0, int.SIZE_MAX, key);}&"
	              /**/ "${function max(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.max(0, int.SIZE_MAX, key);}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "local keyedResult;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	local keyedItem = key(item);\n"
	              /**/ /**/ "	if (keyedResult !is bound || !(keyedResult > keyedItem)) {\n"
	              /**/ /**/ "		keyedResult = keyedItem;\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result is bound ? result : none;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_max__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_max__(start, end);}&"
	              /**/ "${function max(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.max(start, end);}&"
	              /**/ "?#__enumerate__" /*                                                     */ "|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && (result.value !is bound || !(result.value > item)))\n"
	              /**/ /**/ "		result.value = item;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value is bound ? result.value : none;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_max__(start: int = 0, end: int = -1, key: Callable | none = none): Object}" /*                     */ "|${return this.__seq_max__(start, end, key);}&"
	              /**/ "${function max(start: int = 0, end: int = -1, key: Callable | none = none): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.max(start, end, key);}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "local keyedResult = Cell();\n"
	              /**/ /**/ "this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound) {\n"
	              /**/ /**/ "		local keyedItem = key(item);\n"
	              /**/ /**/ "		if (keyedResult.value !is bound || !(keyedResult.value > keyedItem)) {\n"
	              /**/ /**/ "			keyedResult.value = keyedItem;\n"
	              /**/ /**/ "			result.value = item;\n"
	              /**/ /**/ "		}\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value is bound ? result.value : none;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_sum_name, &DeeMA_Sequence_sum,
	              "" DeeMA_Sequence_sum_doc "\n"
	              "Returns the sum of all elements, or ?N if the Sequence is empty\n"
	              "This function is used to implement ${this + ...}\n"

	              "When ${start == 0 && end == -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_sum__(start: int = 0, end: int = -1): Object}" /*                     */ "|${return this.__seq_sum__();}&"
	              /**/ "${function sum(start: int = 0, end: int = -1): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.sum();}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this))\n"
	              /**/ /**/ "	result = result is bound ? result + item : item;\n"
	              /**/ /**/ "return result is bound ? result : none;"
	              /**/ "}"
	              "}\n"

	              "When ${start != 0 || end != -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_sum__(start: int = 0, end: int = -1): Object}" /*                     */ "|${return this.__seq_sum__(start, end);}&"
	              /**/ "${function sum(start: int = 0, end: int = -1): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.sum(start, end);}&"
	              /**/ "?#__enumerate__" /*                                                     */ "|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "this.__enumerate__((i, item?) -\\> {\n"
	              /**/ /**/ "	result.value = result.value is bound ? result.value + item : item;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value is bound ? result.value : none;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_count_name, &DeeMA_Sequence_count,
	              "" DeeMA_Sequence_count_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns the number of instances of a given object @item in @this Sequence\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_count__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int}" /*                     */ "|${return this.__seq_count__(item).operator int();}&"
	              /**/ "${function count(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int} (?A__seqclass__?DType is ?.)" /**/ "|${return this.count(item).operator int();}&"
	              /**/ "?#contains (?A__seqclass__?DType is ?DSet or ?DMapping)" /*              */ "|${return Sequence.contains(this, item) ? 1 : 0;}&"
	              /**/ "?#find" /*                                                               */ "|${"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "for (local i = 0;; ++i) {\n"
	              /**/ /**/ "	i = Sequence.find(this, item, i);\n"
	              /**/ /**/ "	if (i < 0)\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "foreach (local thisItem: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (deemon.equals(item, thisItem))\n"
	              /**/ /**/ "		++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_count__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int}" /*                     */ "|${return this.__seq_count__(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "${function count(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int} (?A__seqclass__?DType is ?.)" /**/ "|${return this.count(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "?#contains (?A__seqclass__?DType is ?DSet or ?DMapping)" /*              */ "|${return Sequence.contains(this, item, 0, int.SIZE_MAX, key) ? 1 : 0;}&"
	              /**/ "?#find" /*                                                               */ "|${"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "for (local i = 0;; ++i) {\n"
	              /**/ /**/ "	i = Sequence.find(this, item, i, int.SIZE_MAX, key);\n"
	              /**/ /**/ "	if (i < 0)\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "foreach (local thisItem: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(thisItem)))\n"
	              /**/ /**/ "		++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_count__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int}" /*                     */ "|${return this.__seq_count__(item, start, end);}&"
	              /**/ "${function count(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int} (?A__seqclass__?DType is ?.)" /**/ "|${return this.count(item, start, end);}&"
	              /**/ "?#contains (?A__seqclass__?DType is ?DSet or ?DMapping)" /*              */ "|${return Sequence.contains(this, item, start, end) ? 1 : 0;}&"
	              /**/ "?#find" /*                                                               */ "|${"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "for (local i = start;; ++i) {\n"
	              /**/ /**/ "	i = Sequence.find(this, item, i, end);\n"
	              /**/ /**/ "	if (i < 0)\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "local result = Cell(0);\n"
	              /**/ /**/ "Sequence.__enumerate__(this, (i, thisItem?) -\\> {\n"
	              /**/ /**/ "	if (thisItem is bound && deemon.equals(item, thisItem))\n"
	              /**/ /**/ "		++result.value;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_count__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int}" /*                     */ "|${return this.__seq_count__(item, start, end, key);}&"
	              /**/ "${function count(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): int} (?A__seqclass__?DType is ?.)" /**/ "|${return this.count(item, start, end, key);}&"
	              /**/ "?#contains (?A__seqclass__?DType is ?DSet or ?DMapping)" /*              */ "|${return Sequence.contains(this, item, start, end, key) ? 1 : 0;}&"
	              /**/ "?#find" /*                                                               */ "|${"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "for (local i = start;; ++i) {\n"
	              /**/ /**/ "	i = Sequence.find(this, item, i, end, key);\n"
	              /**/ /**/ "	if (i < 0)\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "local result = Cell(0);\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "Sequence.__enumerate__(this, (i, thisItem?) -\\> {\n"
	              /**/ /**/ "	if (thisItem is bound && deemon.equals(keyedItem, key(thisItem)))\n"
	              /**/ /**/ "		++result.value;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_contains_name, &DeeMA_Sequence_contains,
	              "" DeeMA_Sequence_contains_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t if @this Sequence contains an element matching @item\n"

	              "When ${start == 0 && end == -1 && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_contains__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_contains__(item);}&"
	              /**/ "${function contains(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.contains(item);}&"
	              /**/ "${operator contains(item: Object): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return item in this;}&"
	              /**/ "?A__getitem__?DMapping ($__map_getitem__)" /*                            */ "|${"
	              /**/ /**/ "local key, value = item...;\n"
	              /**/ /**/ "local realValue;\n"
	              /**/ /**/ "try {\n"
	              /**/ /**/ "	realValue = Mapping.__getitem__(this, key);\n"
	              /**/ /**/ "} catch (KeyError) {\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return deemon.equals(value, realValue);"
	              /**/ "}&"
	              /**/ "?#find" /*                                                               */ "|${return Sequence.find(this, item) != -1;}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "foreach (local thisItem: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (deemon.equals(item, thisItem))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return false;"
	              /**/ "}"
	              "}\n"

	              "When ${start == 0 && end == -1 && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_contains__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_contains__(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "${function contains(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.contains(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "?#find" /*                                                               */ "|${return Sequence.find(this, item, 0, int.SIZE_MAX, key) != -1;}&"
	              /**/ "?#{op:iter}" /*                                                          */ "|${"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "foreach (local thisItem: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(thisItem)))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return false;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_contains__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_contains__(item, start, end);}&"
	              /**/ "${function contains(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.contains(item, start, end);}&"
	              /**/ "?#find" /*                                                               */ "|${return Sequence.find(this, item, start, end) != -1;}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "return this.__enumerate__((i, thisItem?) -\\> {\n"
	              /**/ /**/ "	if (thisItem is bound && deemon.equals(item, thisItem))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end) ?? false;"
	              /**/ "}"
	              "}\n"

	              "When ${(start != 0 || end != -1) && key !is none}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_contains__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_contains__(item, start, end, key);}&"
	              /**/ "${function contains(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.contains(item, start, end, key);}&"
	              /**/ "?#find" /*                                                               */ "|${return Sequence.find(this, item, start, end, key) != -1;}&"
	              /**/ "?#__enumerate__" /*                                                      */ "|${"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "return this.__enumerate__((i, thisItem?) -\\> {\n"
	              /**/ /**/ "	if (thisItem is bound && deemon.equals(keyedItem, key(thisItem)))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end) ?? false;"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_locate_name, &DeeMA_Sequence_locate,
	              "" DeeMA_Sequence_locate_doc "\n"
	              "Locate and return the first element such that ${match(elem)} "
	              /**/ "is true, or @def when no such element exists\n"

	              "When ${start == 0 && end == -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_locate__(match: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_locate__(match, 0, int.SIZE_MAX, def);}&"
	              /**/ "${function locate(match: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.locate(match, 0, int.SIZE_MAX, def);}&"
	              /**/ "?#{op:iter}" /**/ "|${"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (match(item))\n"
	              /**/ /**/ "		return item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return def;\n"
	              /**/ "}"
	              "}\n"

	              "When ${start != 0 || end != -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_locate__(match: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_locate__(match, start, end, def);}&"
	              /**/ "${function locate(match: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.locate(match, start, end, def);}&"
	              /**/ "?#__enumerate__" /**/ "|${"
	              /**/ /**/ "local result = Cell(def);\n"
	              /**/ /**/ "Sequence.__enumerate__(this, (i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && match(item)) {\n"
	              /**/ /**/ "		result.value = item;\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;\n"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_rlocate_name, &DeeMA_Sequence_rlocate,
	              "" DeeMA_Sequence_rlocate_doc "\n"
	              "Locate and return the last element such that ${match(elem)} "
	              /**/ "is true, or @def when no such element exists\n"

	              "When ${start == 0 && end == -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_rlocate__(match: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_rlocate__(match, 0, int.SIZE_MAX, def);}&"
	              /**/ "${function rlocate(match: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.rlocate(match, 0, int.SIZE_MAX, def);}&"
	              /**/ "?#__iter_reverse__" /**/ "|${"
	              /**/ /**/ "foreach (local item: Sequence.__iter_reverse__(this)) {\n"
	              /**/ /**/ "	if (match(item))\n"
	              /**/ /**/ "		return item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return def;\n"
	              /**/ "}&"
	              /**/ "?#{op:iter}" /**/ "|${"
	              /**/ /**/ "local result = def;\n"
	              /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	              /**/ /**/ "	if (match(item))\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;\n"
	              /**/ "}"
	              "}\n"

	              "When ${start != 0 || end != -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_rlocate__(match: Callable, start: int = 0, end: int = -1, init?: Object): Object}" /*                     */ "|${return this.__seq_rlocate__(match, start, end, def);}&"
	              /**/ "${function rlocate(match: Callable, start: int = 0, end: int = -1, init?: Object): Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.rlocate(match, start, end, def);}&"
	              /**/ "?#__enumerate_reverse__" /**/ "|${"
	              /**/ /**/ "local result = Cell(def);\n"
	              /**/ /**/ "Sequence.__enumerate_reverse__(this, (i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && match(item)) {\n"
	              /**/ /**/ "		result.value = item;\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;\n"
	              /**/ "}&"
	              /**/ "?#__enumerate__" /**/ "|${"
	              /**/ /**/ "local result = Cell(def);\n"
	              /**/ /**/ "Sequence.__enumerate__(this, (i, item?) -\\> {\n"
	              /**/ /**/ "	if (item is bound && match(item))\n"
	              /**/ /**/ "		result.value = item;\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;\n"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_startswith_name, &DeeMA_Sequence_startswith,
	              "" DeeMA_Sequence_startswith_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t / ?f indicative of @this Sequence's first element matching :item\n"
	              "The implementation of this is derived from #first, where the found is then compared "
	              /**/ "against @item, potentially through use of @{key}: ${key(first) == key(item)} or ${first == item}, "
	              /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned\n"

	              "When ${start == 0 && end == -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_startswith__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_startswith__(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "${function startswith(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.startswith(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "?#first" /**/ "|${"
	              /**/ /**/ "local first;\n"
	              /**/ /**/ "try {\n"
	              /**/ /**/ "	first = Sequence.first(this);\n"
	              /**/ /**/ "} catch (UnboundAttribute | IndexError) {\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (key is none)\n"
	              /**/ /**/ "	key = x -\\> x;\n"
	              /**/ /**/ "return key(item) == key(first);\n"
	              /**/ "}"
	              "}\n"

	              "When ${start != 0 || end != -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_startswith__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_startswith__(item, start, end, key);}&"
	              /**/ "${function startswith(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.startswith(item, start, end, key);}&"
	              /**/ "?#{op:getitem}" /**/ "|${"
	              /**/ /**/ "local first;\n"
	              /**/ /**/ "if (start >= end)\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "try {\n"
	              /**/ /**/ "	first = Sequence.__getitem__(this, start);\n"
	              /**/ /**/ "} catch (IndexError | UnboundItem) {\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (key is none)\n"
	              /**/ /**/ "	key = x -\\> x;\n"
	              /**/ /**/ "return key(item) == key(first);\n"
	              /**/ "}"
	              "}"),


	TYPE_KWMETHOD(DeeMA_Sequence_endswith_name, &DeeMA_Sequence_endswith,
	              "" DeeMA_Sequence_endswith_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t / ?f indicative of @this Sequence's last element matching :item\n"
	              "The implementation of this is derived from #last, where the found is then compared "
	              /**/ "against @item, potentially through use of @{key}: ${key(last) == key(item)} or ${last == item}, "
	              /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned\n"

	              "When ${start == 0 && end == -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_endswith__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_endswith__(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "${function endswith(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.endswith(item, 0, int.SIZE_MAX, key);}&"
	              /**/ "?#last" /**/ "|${"
	              /**/ /**/ "local last;\n"
	              /**/ /**/ "try {\n"
	              /**/ /**/ "	last = Sequence.last(this);\n"
	              /**/ /**/ "} catch (UnboundAttribute | IndexError) {\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (key is none)\n"
	              /**/ /**/ "	key = x -\\> x;\n"
	              /**/ /**/ "return key(item) == key(last);\n"
	              /**/ "}"
	              "}\n"

	              "When ${start != 0 || end != -1}:"
	              "#T{Requirements|Implementation~"
	              /**/ "${function __seq_endswith__(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool}" /*                     */ "|${return this.__seq_endswith__(item, start, end, key);}&"
	              /**/ "${function endswith(item: Object, start: int = 0, end: int = -1, key: Callable | none = none): bool} (?A__seqclass__?DType is ?.)" /**/ "|${return this.endswith(item, start, end, key);}&"
	              /**/ "?#{op:size}, ?#{op:getitem}" /**/ "|${"
	              /**/ /**/ "local last;\n"
	              /**/ /**/ "if (start >= end)\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "local selfsize = Sequence.__size__(this);\n"
	              /**/ /**/ "if (end > selfsize) {\n"
	              /**/ /**/ "	end = selfsize;\n"
	              /**/ /**/ "	if (start >= end)\n"
	              /**/ /**/ "		return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "try {\n"
	              /**/ /**/ "	last = Sequence.__getitem__(this, end - 1);\n"
	              /**/ /**/ "} catch (IndexError | UnboundItem) {\n"
	              /**/ /**/ "	return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (key is none)\n"
	              /**/ /**/ "	key = x -\\> x;\n"
	              /**/ /**/ "return key(item) == key(last);\n"
	              /**/ "}"
	              "}"),


	/* TODO: REWRITE DOCUMENTATION FOR EVERYTHING BELOW */
	/* TODO: REWRITE DOCUMENTATION FOR EVERYTHING BELOW */
	/* TODO: REWRITE DOCUMENTATION FOR EVERYTHING BELOW */

	/* Method hint API functions. */
	TYPE_KWMETHOD(DeeMA_Sequence_find_name, &DeeMA_Sequence_find,
	              "" DeeMA_Sequence_find_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Search for the first element matching @item and return its index. "
	              /**/ "If no such element exists, return ${-1} instead"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_rfind_name, &DeeMA_Sequence_rfind,
	              "" DeeMA_Sequence_rfind_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Search for the last element matching @item and return its index. "
	              /**/ "If no such element exists, return ${-1} instead"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_erase_name, &DeeMA_Sequence_erase,
	              "" DeeMA_Sequence_erase_doc "\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Erase up to @count elements starting at @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_insert_name, &DeeMA_Sequence_insert,
	              "" DeeMA_Sequence_insert_doc "\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Insert the given @item under @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_insertall_name, &DeeMA_Sequence_insertall,
	              "" DeeMA_Sequence_insertall_doc "\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Insert all elements from @items at @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_pushfront_name, &DeeMA_Sequence_pushfront,
	            "" DeeMA_Sequence_pushfront_doc "\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ?#insert at position $0"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_append_name, &DeeMA_Sequence_append,
	            "" DeeMA_Sequence_append_doc "\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Append the given @item at the end of @this Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_pushback_name, &DeeMA_Sequence_pushback,
	            "" DeeMA_Sequence_pushback_doc "\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Alias for ?#append"),
	TYPE_METHOD(DeeMA_Sequence_extend_name, &DeeMA_Sequence_extend,
	            "" DeeMA_Sequence_extend_doc "\n"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Append all elements from @items at the end of @this Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_xchitem_name, &DeeMA_Sequence_xchitem,
	              "" DeeMA_Sequence_xchitem_doc "\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Exchange the @index'th element of @this Sequence with the given "
	              /**/ "@value, returning the old element found under that index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_clear_name, &DeeMA_Sequence_clear,
	            "" DeeMA_Sequence_clear_doc "\n"
	            "#tSequenceError{@this Sequence is immutable}"
	            "Clear all elements from the Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_pop_name, &DeeMA_Sequence_pop,
	              "" DeeMA_Sequence_pop_doc "\n"
	              "#tIntegerOverflow{The given @index is too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Pop the @index'th element of @this Sequence and return it. When @index is lower "
	              /**/ "than $0, add ${##this} prior to index selection"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_remove_name, &DeeMA_Sequence_remove,
	              "" DeeMA_Sequence_remove_doc "\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find the first instance of @item and remove it, returning ?t if an "
	              /**/ "element got removed, or ?f if @item could not be found"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_rremove_name, &DeeMA_Sequence_rremove,
	              "" DeeMA_Sequence_rremove_doc "\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find the last instance of @item and remove it, returning ?t if an "
	              /**/ "element got removed, or ?f if @item could not be found"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_removeall_name, &DeeMA_Sequence_removeall,
	              "" DeeMA_Sequence_removeall_doc "\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find all instance of @item and remove them, returning the number of "
	              /**/ "instances found (and consequently removed)"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_removeif_name, &DeeMA_Sequence_removeif,
	              "" DeeMA_Sequence_removeif_doc "\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Remove all elements within the given sub-range, for which ${should(item)} "
	              /**/ "evaluates to ?t, and return the number of elements found (and consequently removed)"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_resize_name, &DeeMA_Sequence_resize,
	              "" DeeMA_Sequence_resize_doc "\n"
	              "#tSequenceError{@this Sequence isn't resizable}"
	              "Resize @this Sequence to have a new length of @size "
	              /**/ "items, using @filler to initialize newly added entries"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_fill_name, &DeeMA_Sequence_fill,
	              "" DeeMA_Sequence_fill_doc "\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "Assign @filler to all elements within the given sub-range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_reverse_name, &DeeMA_Sequence_reverse,
	              "" DeeMA_Sequence_reverse_doc "\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "Reverse the order of all elements within the given range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_reversed_name, &DeeMA_Sequence_reversed,
	              "" DeeMA_Sequence_reversed_doc "\n"
	              "Return a Sequence that contains the elements of @this Sequence in reverse order\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_sort_name, &DeeMA_Sequence_sort,
	              "" DeeMA_Sequence_sort_doc "\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Sort the elements within the given range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_sorted_name, &DeeMA_Sequence_sorted,
	              "" DeeMA_Sequence_sorted_doc "\n"
	              "Return a Sequence that contains all elements from @this Sequence, "
	              /**/ "but sorted in ascending order, or in accordance to @key\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_bfind_name, &DeeMA_Sequence_bfind,
	              "" DeeMA_Sequence_bfind_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Do a binary search (requiring @this to be sorted via @key) for @item\n"
	              "In case multiple elements match @item, the returned index will be "
	              /**/ "that for one of them, though it is undefined which one specifically.\n"
	              "When no elements of @this match, ?N is returned."
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_bposition_name, &DeeMA_Sequence_bposition,
	              "" DeeMA_Sequence_bposition_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Same as ?#bfind, but return (an) index where @item should be inserted, rather "
	              /**/ "than ?N when @this doesn't contain any matching object"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_brange_name, &DeeMA_Sequence_brange,
	              "" DeeMA_Sequence_brange_doc "\n"
	              DOC_param_item
	              DOC_param_key
	              "Similar to ?#bfind, but return a tuple ${[begin,end)} of integers representing "
	              /**/ "the lower and upper bound of indices for elements from @this matching @item.\n"
	              "NOTE: The returned tuple is allowed to be an ASP, meaning that its elements may "
	              /**/ "be calculated lazily, and are prone to change as the result of @this changing."
	              ""), /* TODO: Requirements|Implementation table */





	/* Default operations for all sequences. */
	TYPE_METHOD("filter", &seq_filter,
	            "(" seq_filter_params ")->?DSequence\n"
	            "#pkeep{A key function which is called for each element of @this Sequence}"
	            "Returns a sub-Sequence of all elements for which ${keep(item)} evaluates to ?t. "
	            /**/ "Semantically, this is identical to ${(for (local x: this) if (keep(x)) x)}\n"
	            "${"
	            /**/ "function filter(keep: Callable): Sequence {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		if (keep(x))\n"
	            /**/ "			yield x;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("ubfilter", &seq_ubfilter,
	            "(" seq_ubfilter_params ")->?DSequence\n"
	            "#pkeep{A key function which is called for each element of @this Sequence}"
	            "Returns a sub-Sequence of all elements for which ${keep(item)} evaluates to ?t. "
	            /**/ "Same as ?#filter, but the returned sequence has the same size as @this, "
	            /**/ "and filtered elements are treated as though they were unbound:\n"
	            "${"
	            /**/ "assert { 10, 20 }.ubfilter(x -\\> x > 10)[0] !is bound;\n"
	            /**/ "assert { 10, 20 }.ubfilter(x -\\> x > 10)[1] is bound;"
	            "}"),
	TYPE_METHOD("map", &seq_map,
	            "(" seq_map_params ")->?DSequence\n"
	            "#pmapper{A key function invoked to map members of @this Sequence}"
	            "Returns a Sequence that is a transformation of @this, with each element passed "
	            /**/ "to @mapper for processing before being returned\n"
	            "${"
	            /**/ "function map(mapper: Callable): Sequence {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		yield mapper(x);\n"
	            /**/ "}"
	            "}"),

	TYPE_KWMETHOD(STR_index, &seq_index,
	              "(" seq_index_params ")->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              DOC_throws_ValueError_if_not_found
	              "Search for the first element matching @item and return its index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_rindex, &seq_rindex,
	              "(" seq_rindex_params ")->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              DOC_throws_ValueError_if_not_found
	              "Search for the last element matching @item and return its index"
	              ""), /* TODO: Requirements|Implementation table */

	TYPE_METHOD("segments",
	            &seq_segments,
	            "(" seq_segments_params ")->?S?DSequence\n"
	            "#tIntegerOverflow{@segment_length is negative, or too large}"
	            "#tValueError{The given @segment_length is zero}"
	            "Return a Sequence of sequences contains all elements from @this Sequence, "
	            /**/ "with the first n sequences all consisting of @segment_length elements, before "
	            /**/ "the last one contains the remainder of up to @segment_length elements"),
	TYPE_METHOD("distribute",
	            &seq_distribute,
	            "(" seq_distribute_params ")->?S?DSequence\n"
	            "#tIntegerOverflow{@segment_count is negative, or too large}"
	            "#tValueError{The given @segment_count is zero}"
	            "Re-distribute the elements of @this Sequence to form @segment_count similarly-sized "
	            /**/ "buckets of objects, with the last bucket containing the remaining elements, "
	            /**/ "making its length a little less than the other buckets\n"
	            "This is similar to #segments, however rather than having the caller specify the "
	            /**/ "size of the a bucket, the number of buckets is specified instead."),
	TYPE_KWMETHOD("combinations",
	              &seq_combinations,
	              "(" seq_combinations_params ")->?S?DSequence\n"
	              "#pcached{When true, automatically wrap using ?#cached like ${this.cached.combinations(r, cached: false)}}"
	              "#tIntegerOverflow{@r is negative, or too large}"
	              "Returns a Sequence of r-long sequences representing all possible (ordered) "
	              /**/ "combinations of elements retrieved from @this\n"
	              "${"
	              /**/ "/* { (\"A\", \"B\"), (\"A\", \"C\"), (\"A\", \"D\"),\n"
	              /**/ " *   (\"B\", \"C\"), (\"B\", \"D\"), (\"C\", \"D\") } */\n"
	              /**/ "print repr \"ABCD\".combinations(2);"
	              "}\n"
	              "Notice that a combination such as $\"BA\" is not produced, as only possible "
	              /**/ "combinations with their original element order still in tact may be returned\n"
	              "When @this Sequence implements ?#{op:getitem} and ?#{op:size}, those will be invoked "
	              /**/ "as items are retrieved by index. Otherwise, all elements from @this Sequence "
	              /**/ "are loaded at once when #combinations is called first.\n"
	              "When @r is greater than ${##this}, an empty Sequence is returned (${{}})\n"
	              "Hint: The python equivalent of this function is "
	              /**/ "#A{itertools.combinations|https://docs.python.org/3/library/itertools.html##itertools.combinations}"),
	TYPE_KWMETHOD("repeatcombinations",
	              &seq_repeatcombinations,
	              "(" seq_repeatcombinations_params ")->?S?DSequence\n"
	              "#pcached{When true, automatically wrap using ?#cached like ${this.cached.repeatcombinations(r, cached: false)}}"
	              "#tIntegerOverflow{@r is negative, or too large}"
	              "Same as #combinations, however elements of @this Sequence may be repeated (though element order is still enforced)\n"
	              "${"
	              /**/ "/* { (\"A\", \"A\"), (\"A\", \"B\"), (\"A\", \"C\"),\n"
	              /**/ " *   (\"B\", \"B\"), (\"B\", \"C\"), (\"C\", \"C\") } */\n"
	              /**/ "print repr \"ABC\".repeatcombinations(2);"
	              "}\n"
	              "When @this Sequence implements ?#{op:getitem} and ?#{op:size}, those will be invoked "
	              /**/ "as items are retrieved by index. Otherwise, all elements from @this Sequence "
	              /**/ "are loaded at once when #repeatcombinations is called first.\n"
	              "When @r is $0, a Sequence containing a single, empty Sequence is returned (${{{}}})\n"
	              "When ${##this} is zero, an empty Sequence is returned (${{}})\n"
	              "Hint: The python equivalent of this function is "
	              /**/ "#A{itertools.combinations_with_replacement|https://docs.python.org/3/library/itertools.html##itertools.combinations_with_replacement}"),
	TYPE_KWMETHOD("permutations",
	              &seq_permutations,
	              "(" seq_permutations_params ")->?S?DSequence\n"
	              "#pcached{When true, automatically wrap using ?#cached like ${this.cached.permutations(r, cached: false)}}"
	              "#tIntegerOverflow{@r is negative, or too large}"
	              "Same as #combinations, however the order of elements must "
	              /**/ "not be enforced, though indices may not be repeated\n"
	              "When @r is ?N, ${##this} is used instead\n"
	              "${"
	              /**/ "/* { (\"A\", \"B\"), (\"A\", \"C\"), (\"B\", \"A\"),\n"
	              /**/ " *   (\"B\", \"C\"), (\"C\", \"A\"), (\"C\", \"B\") } */\n"
	              /**/ "print repr \"ABC\".permutations(2);"
	              "}\n"
	              "When @this Sequence implements ?#{op:getitem} and ?#{op:size}, those will be invoked "
	              /**/ "as items are retrieved by index. Otherwise, all elements from @this Sequence "
	              /**/ "are loaded at once when #repeatcombinations is called first.\n"
	              "When @r is $0, a Sequence containing a single, empty Sequence is returned (${{{}}})\n"
	              "When ${##this} is zero, an empty Sequence is returned (${{}})\n"
	              "Hint: The python equivalent of this function is "
	              /**/ "#A{itertools.permutations|https://docs.python.org/3/library/itertools.html##itertools.permutations}"),

	TYPE_KWMETHOD("byhash", &seq_byhash, DOC_GET(seq_byhash_doc)),

	TYPE_KWMETHOD("distinct", &seq_distinct,
	              "(" seq_distinct_params ")->?DSet\n"
	              DOC_param_key
	              "When @key is not given, same as ${this as Set} (reminder: when enumerating "
	              /**/ "items of a ?. casted into a ?DSet, deemon will ensure that every "
	              /**/ "distinct element of the ?. is enumerate exactly once). Otherwise, "
	              /**/ "returns a custom ?DSet proxy that only enumerates elements from @this "
	              /**/ "for which ${key(elem)} returns a distinct result\n"
	              "${"
	              /**/ "function distinct(key?: Callable): Set {\n"
	              /**/ "	if (key !is bound)\n"
	              /**/ "		return this as Set;\n"
	              /**/ "	local encountered = HashSet();\n"
	              /**/ "	for (local item: this) {\n"
	              /**/ "		local keyedItem = key(item);\n"
	              /**/ "		if (encountered.insert(keyedItem))\n"
	              /**/ "			yield item;\n"
	              /**/ "	}\n"
	              /**/ "}"
	              "}"),
	/* TODO: unique(key?:?DCallable)->?.
	 * Similar to ?#distinct (and actually identical when @this is sorted),
	 * but only skip identical, consecutive items (iow: only keep track of
	 * the most-recently encountered item, rather than all that have been
	 * encountered across the entire sequence). */

	/* TODO: takewhile(cond:?DCallble)->?.
	 * Same as "filter(cond)", but only yield leading matches:
	 * >> function takewhile(cond: Callble): Sequence {
	 * >>     for (local item: this as Sequence) {
	 * >>         if (!cond(item))
	 * >>             break;
	 * >>         yield item;
	 * >>     }
	 * >> } */

	/* TODO: dropwhile(cond:?DCallble)->?.
	 * Matches everything not matched by ?#takewhile:
	 * >> function takeafter(cond: Callble): Sequence {
	 * >>     local iter = (this as Sequence).operator iter();
	 * >>     foreach (local item: iter) {
	 * >>         if (!cond(item)) {
	 * >>             yield item;
	 * >>             break;
	 * >>         }
	 * >>     }
	 * >>     foreach (local item: iter)
	 * >>         yield item;
	 * >> } */

	/* TODO: pairwise->?S?T2?O?O
	 * Yield every element of @this sequence (except for the first) as a pair (predecessor, elem):
	 * >> property pairwise: {(Object, Object)...} = {
	 * >>     get() {
	 * >>         local iter = (this as Sequence).operator iter();
	 * >>         local prev;
	 * >>         foreach (prev: iter)
	 * >>             goto start;
	 * >>         return;
	 * >> start:
	 * >>         foreach (local next: iter) {
	 * >>             yield (prev, next);
	 * >>             prev = next;
	 * >>         }
	 * >>     }
	 * >> } */

	/* TODO: asunbound(item)->?S?O
	 * Return a view of @this sequence that replaces every instance of "item" with
	 * unbound elements. Same as:
	 * >> this.ubfilter(e -> !deemon.equals(item, e)); */

	/* TODO: unboundas(item)->?S?O
	 * Return a view of @this sequence that replaces every unbound element with "item" */

	/* TODO: findall: "(item,start=!0,end=!-1,key:?DCallable=!N)->?S?Dint"
	 * > Find not just the first, but all indices of @item */
	/* TODO: findallseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = -1, key: Callable = none): {int...} */

	/* TODO: findseq(subseq: Sequence | rt.SeqSome, key: Callable = none): (int, int) | none */
	/* TODO: rfindseq(subseq: Sequence | rt.SeqSome, key: Callable = none): (int, int) | none */

	/* TODO: indexseq(subseq: Sequence | rt.SeqSome, key: Callable = none): (int, int) */
	/* TODO: rindexseq(subseq: Sequence | rt.SeqSome, key: Callable = none): (int, int) */

	/* TODO: join: "(seqs:?S?S?O)->?." */

	/* TODO: strip(item: Object, key: Callable = none): Sequence */
	/* TODO: sstrip(subseq: Sequence, key: Callable = none): Sequence */

	/* TODO: lstrip(item: Object, key: Callable = none): Sequence */
	/* TODO: lsstrip(subseq: Sequence, key: Callable = none): Sequence */

	/* TODO: rstrip(item: Object, key: Callable = none): Sequence */
	/* TODO: rsstrip(subseq: Sequence, key: Callable = none): Sequence */

	/* TODO: split(item: Object, key: Callable = none): Sequence */
	/* TODO: splitseq(subseq: Sequence | rt.SeqSome, key: Callable = none): Sequence */

	/* TODO: countseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = -1, key: Callable = none): int */

	/* TODO: partition(item: Object, start: int = 0, end: int = -1, key: Callable = none): (Sequence, item, Sequence) */
	/* TODO: partitionseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = -1, key: Callable = none): (Sequence, subseq, Sequence) */

	/* TODO: rpartition(item: Object, start: int = 0, end: int = -1, key: Callable = none): (Sequence, item, Sequence) */
	/* TODO: rpartitionseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = -1, key: Callable = none): (Sequence, subseq, Sequence) */

	/* TODO: startswithseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = -1, key: Callable = none): bool */

	/* TODO: endswithseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = -1, key: Callable = none): bool */


	/* Special proxy functions that are implemented using multiple method hints */
	TYPE_KWMETHOD("enumerate", &seq_enumerate,
	              "" DeeMA___seq_enumerate___doc "\n"
	              "" DeeMA___seq_enumerate_items___doc "\n"
	              "Enumerate indices/keys and associated values of @this sequence\n"
	              "This function can be used to easily enumerate sequence indices and "
	              /**/ "values, including being able to enumerate indices/keys that "
	              /**/ "are currently unbound. (s.a. ?#__enumerate__, ?#__enumerate_items__)\n"
	              "${"
	              /**/ "import FixedList from collections;\n"
	              /**/ "local x = FixedList(4);\n"
	              /**/ "x[1] = 10;\n"
	              /**/ "x[3] = 20;\n"
	              /**/ "/* [1]: 10                 [3]: 20 */\n"
	              /**/ "for (local index, value: x.enumerate())\n"
	              /**/ "	print f\"[{repr index}]: {repr value}\";\n"
	              /**/ "\\\n"
	              /**/ "/* [0]: <unbound>          [1]: 10\n"
	              /**/ " * [2]: <unbound>          [3]: 20 */\n"
	              /**/ "x.enumerate((index, value?) -\\> {\n"
	              /**/ "	print f\"[{repr index}]: {value is bound ? repr value : \"<unbound>\"}\";\n"
	              /**/ "});"
	              "}"),

	/* Helper functions for mutable sequences. */
	TYPE_METHOD(STR_popfront, &seq_popfront,
	            "->\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ${this.pop(0)}"),
	TYPE_METHOD(STR_popback, &seq_popback,
	            "->\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ${this.pop(-1)}"),

	/* Binary search API */
	TYPE_KWMETHOD("bcontains", &seq_bcontains,
	              "(" seq_bcontains_params ")->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Wrapper around ?#bfind that returns indicative of @item being bound:\n"
	              "${"
	              /**/ "local index = Sequence.bfind(this, start, end, key);\n"
	              /**/ "return index !is none;"
	              "}"),
	TYPE_KWMETHOD("bindex", &seq_bindex,
	              "(" seq_bindex_params ")->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "#tValueError{The Sequence does not contain an item matching @item}"
	              "Same as ?#bfind, but throw an :ValueError instead of returning ?N:\n"
	              "${"
	              /**/ "local index = Sequence.bfind(this, start, end, key);\n"
	              /**/ "if (index is none)\n"
	              /**/ "	throw ValueError(...);\n"
	              /**/ "return index;"
	              "}"),
	TYPE_KWMETHOD("blocateall", &seq_blocateall,
	              "(" seq_blocateall_params ")->?S?O\n"
	              DOC_param_item
	              DOC_param_key
	              "Return the sub-range from @this of elements matching @item, as returned by ?#brange\n"
	              "${"
	              /**/ "local rstart, rend = Sequence.brange(this, item, start, end, key)...;\n"
	              /**/ "return Sequence.__getrange__(this, rstart, rend);\n"
	              "}\n"
	              "Here is a really neat usage-example for this function: find all strings within "
	              /**/ "a sorted sequence of strings that start with a given prefix-string:\n"
	              "${"
	              /**/ "local lines: {string...} = ...; /* Must be sorted! */\n"
	              /**/ "local prefix: string     = ...;\n"
	              /**/ "/* The process of looking up relevant lines here is O(log(##lines))! */\n"
	              /**/ "for (local l: lines.blocateall(prefix, s -#> s.substr(0, ##prefix)))\n"
	              /**/ "	print l;\n"
	              "}"),
	TYPE_KWMETHOD("binsert", &seq_binsert,
	              "(" seq_binsert_params ")\n"
	              "Helper wrapper for ?#insert and ?#bposition that automatically determines "
	              /**/ "the index where a given @item should be inserted to ensure that @this sequence "
	              /**/ "remains sorted according to @key. Note that this function makes virtual calls as "
	              /**/ "seen in the following template, meaning it usually doesn't need to be overwritten "
	              /**/ "by sub-classes.\n"
	              "${"
	              /**/ "local index = this.bposition(item, key);\n"
	              /**/ "return this.insert(index, item);"
	              "}"),






	/* Sequence operators as member functions:
	 * >> __getitem__(index:?Dint)->
	 * >> __delitem__(index:?Dint)
	 * >> __setitem__(index:?Dint,value)
	 * >> ...
	 *
	 * Using these, user-code can write:
	 * >> Sequence.__getitem__(ob, 42);
	 * instead of (and needing to create a super-proxy):
	 * >> (ob as Sequence)[42]; */

	/* Method hint operator invocation. */
	TYPE_METHOD("__bool__", &DeeMA___seq_bool__, DeeMA___seq_bool___doc "\nAlias for ${!!(this as Sequence)} (s.a. ?#{op:bool})"),
	TYPE_METHOD("__size__", &DeeMA___seq_size__, DeeMA___seq_size___doc "\nAlias for ${##(this as Sequence)} (s.a. ?#{op:size})"),
	TYPE_METHOD("__iter__", &DeeMA___seq_iter__, "->?#Iterator\nAlias for ${(this as Sequence).operator iter()} (s.a. ?#{op:iter})"),
	TYPE_METHOD("__getitem__", &DeeMA___seq_getitem__, DeeMA___seq_getitem___doc "\nAlias for ${(this as Sequence)[index]} (s.a. ?#{op:getitem})"),
	TYPE_METHOD("__delitem__", &DeeMA___seq_delitem__, DeeMA___seq_delitem___doc "\nAlias for ${del (this as Sequence)[index]} (s.a. ?#{op:delitem})"),
	TYPE_METHOD("__setitem__", &DeeMA___seq_setitem__, DeeMA___seq_setitem___doc "\nAlias for ${(this as Sequence)[index] = value} (s.a. ?#{op:setitem})"),
	TYPE_KWMETHOD("__getrange__", &DeeMA___seq_getrange__, DeeMA___seq_getrange___doc "\nAlias for ${(this as Sequence)[start:end]} (s.a. ?#{op:getrange})"),
	TYPE_KWMETHOD("__delrange__", &DeeMA___seq_delrange__, "(start=!0,end?:?X2?N?Dint)\nAlias for ${del (this as Sequence)[start:end]} (s.a. ?#{op:delrange})"),
	TYPE_KWMETHOD("__setrange__", &DeeMA___seq_setrange__, "(start=!0,end?:?X2?N?Dint,values:?S?O)\nAlias for ${(this as Sequence)[start:end] = values} (s.a. ?#{op:setrange})"),
	TYPE_METHOD("__assign__", &DeeMA___seq_assign__, DeeMA___seq_assign___doc "\nAlias for ${(this as Sequence) := items} (s.a. ?#{op:assign})"),
	TYPE_METHOD("__hash__", &DeeMA___seq_hash__, DeeMA___seq_hash___doc "\nAlias for ${(this as Sequence).operator hash()} (s.a. ?#{op:hash})"),
	TYPE_METHOD("__compare__", &DeeMA___seq_compare__, DeeMA___seq_compare___doc "\nAlias for ${Sequence.compare(this, rhs)} (s.a. ?#compare)"),
	TYPE_METHOD("__compare_eq__", &DeeMA___seq_compare_eq__, "(rhs:?X2?DSequence?S?O)->?Dbool\nAlias for ${Sequence.equals(this, rhs)} (s.a. ?#equals)"),
	TYPE_METHOD("__eq__", &DeeMA___seq_eq__, DeeMA___seq_eq___doc "\nAlias for ${(this as Sequence) == rhs} (s.a. ?#{op:eq})"),
	TYPE_METHOD("__ne__", &DeeMA___seq_ne__, DeeMA___seq_ne___doc "\nAlias for ${(this as Sequence) != rhs} (s.a. ?#{op:ne})"),
	TYPE_METHOD("__lo__", &DeeMA___seq_lo__, DeeMA___seq_lo___doc "\nAlias for ${(this as Sequence) < rhs} (s.a. ?#{op:lo})"),
	TYPE_METHOD("__le__", &DeeMA___seq_le__, DeeMA___seq_le___doc "\nAlias for ${(this as Sequence) <= rhs} (s.a. ?#{op:le})"),
	TYPE_METHOD("__gr__", &DeeMA___seq_gr__, DeeMA___seq_gr___doc "\nAlias for ${(this as Sequence) > rhs} (s.a. ?#{op:gr})"),
	TYPE_METHOD("__ge__", &DeeMA___seq_ge__, DeeMA___seq_ge___doc "\nAlias for ${(this as Sequence) >= rhs} (s.a. ?#{op:ge})"),
	TYPE_METHOD("__add__", &DeeMA___seq_add__, DeeMA___seq_add___doc "\nAlias for ${(this as Sequence) + rhs} (s.a. ?#{op:add})"),
	TYPE_METHOD("__mul__", &DeeMA___seq_mul__, DeeMA___seq_mul___doc "\nAlias for ${(this as Sequence) * repeat} (s.a. ?#{op:mul})"),
	TYPE_METHOD("__inplace_add__", &DeeMA___seq_inplace_add__, DeeMA___seq_inplace_add___doc "\nAlias for ${(this as Sequence) += rhs} (s.a. ?#{op:iadd})"),
	TYPE_METHOD("__inplace_mul__", &DeeMA___seq_inplace_mul__, DeeMA___seq_inplace_mul___doc "\nAlias for ${(this as Sequence) *= repeat} (s.a. ?#{op:imul})"),
	TYPE_METHOD("__enumerate__", &DeeMA___seq_enumerate__,
	            "" DeeMA___seq_enumerate___doc "\n"
	            "Enumerate bound (and unbound) sequence items by their index. Used to implement "
	            /**/ "${(this as Sequence).enumerate(cb[,start,end])} (s.a. ?#enumerate)\n"
	            "\n"

	            "When ${start == 0 && end == -1}:"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_enumerate__(cb: Callable, start: int = 0, end: int = -1): Object | none}" /**/ "|${return this.__seq_enumerate__(cb);}&"
	            /**/ "?#{op:getitem}, ?#{op:size}" /**/ "|${"
	            /**/ /**/ "for (local i: [:Sequence.__size__(this)]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		local r = cb(i);\n"
	            /**/ /**/ "		if (r !is none)\n"
	            /**/ /**/ "			return r;\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	local r = cb(i, item);\n"
	            /**/ /**/ "	if (r !is none)\n"
	            /**/ /**/ "		return r;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}&"
	            /**/ "?#{op:getitem}" /*             */ "|${"
	            /**/ /**/ "for (local i = 0;; ++i) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		local r = cb(i);\n"
	            /**/ /**/ "		if (r !is none)\n"
	            /**/ /**/ "			return r;\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	local r = cb(i, item);\n"
	            /**/ /**/ "	if (r !is none)\n"
	            /**/ /**/ "		return r;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}&"
	            /**/ "?#{op:iter}" /*                */ "|${"
	            /**/ /**/ "local i = 0;\n"
	            /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	            /**/ /**/ "	local r = cb(i, item);\n"
	            /**/ /**/ "	if (r !is none)\n"
	            /**/ /**/ "		return r;\n"
	            /**/ /**/ "	++i;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}"
	            "}\n"

	            "When ${start != 0 || end != -1}:"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_enumerate__(cb: Callable, start: int = 0, end: int = -1): Object | none}" /**/ "|${return this.__seq_enumerate__(cb, start, end);}&"
	            /**/ "?#{op:getitem}, ?#{op:size}" /**/ "|${"
	            /**/ /**/ "local size = Sequence.__size__(this);\n"
	            /**/ /**/ "if (end > size)\n"
	            /**/ /**/ "	end = size;\n"
	            /**/ /**/ "for (local i: [start:end]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		local r = cb(i);\n"
	            /**/ /**/ "		if (r !is none)\n"
	            /**/ /**/ "			return r;\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	local r = cb(i, item);\n"
	            /**/ /**/ "	if (r !is none)\n"
	            /**/ /**/ "		return r;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}&"
	            /**/ "?#{op:getitem}" /*             */ "|${"
	            /**/ /**/ "for (local i: [start:end]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		local r = cb(i);\n"
	            /**/ /**/ "		if (r !is none)\n"
	            /**/ /**/ "			return r;\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	local r = cb(i, item);\n"
	            /**/ /**/ "	if (r !is none)\n"
	            /**/ /**/ "		return r;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}&"
	            /**/ "?#{op:iter}" /*                */ "|${"
	            /**/ /**/ "local iter = Sequence.__iter__(this);\n"
	            /**/ /**/ "if (Iterator.advance(iter, start) < start)\n"
	            /**/ /**/ "	return none;\n"
	            /**/ /**/ "for (local i: [start:end]) {\n"
	            /**/ /**/ "	foreach (local item: iter) {\n"
	            /**/ /**/ "		local r = cb(i, item);\n"
	            /**/ /**/ "		if (r !is none)\n"
	            /**/ /**/ "			return r;\n"
	            /**/ /**/ "		goto next_item;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	break;\n"
	            /**/ /**/ "next_item:;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("__enumerate_items__", &DeeMA___seq_enumerate_items__,
	            "" DeeMA___seq_enumerate_items___doc "\n"
	            "Enumerate bound sequence items by their index. Used to implement "
	            /**/ "${(this as Sequence).enumerate([start,end])} (s.a. ?#enumerate)\n"
	            "\n"

	            "When ${start == 0 && end == -1}:"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_enumerate_items__(start: int = 0, end: int = -1): {(int, Object)...}}" /**/ "|${return this.__seq_enumerate_items__();}&"
	            /**/ "?#{op:getitem}, ?#{op:size}" /**/ "|${"
	            /**/ /**/ "for (local i: [:Sequence.__size__(this)]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	yield (i, item);\n"
	            /**/ /**/ "}"
	            /**/ "}&"
	            /**/ "?#{op:getitem}" /*             */ "|${"
	            /**/ /**/ "for (local i = 0;; ++i) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	yield (i, item);\n"
	            /**/ /**/ "}"
	            /**/ "}&"
	            /**/ "?#{op:iter}" /*                */ "|${"
	            /**/ /**/ "local i = 0;\n"
	            /**/ /**/ "foreach (local item: Sequence.__iter__(this)) {\n"
	            /**/ /**/ "	yield (i, item);\n"
	            /**/ /**/ "	++i;\n"
	            /**/ /**/ "}"
	            /**/ "}"
	            "}\n"

	            "When ${start != 0 || end != -1}:"
	            "#T{Requirements|Implementation~"
	            /**/ "${function __seq_enumerate_items__(start: int = 0, end: int = -1): Object | none}" /**/ "|${return this.__seq_enumerate_items__(start, end);}&"
	            /**/ "?#{op:getitem}, ?#{op:size}" /**/ "|${"
	            /**/ /**/ "local size = Sequence.__size__(this);\n"
	            /**/ /**/ "if (end > size)\n"
	            /**/ /**/ "	end = size;\n"
	            /**/ /**/ "for (local i: [start:end]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	yield (i, item);\n"
	            /**/ /**/ "}"
	            /**/ "}&"
	            /**/ "?#{op:getitem}" /*             */ "|${"
	            /**/ /**/ "for (local i: [start:end]) {\n"
	            /**/ /**/ "	local item;\n"
	            /**/ /**/ "	try {\n"
	            /**/ /**/ "		item = Sequence.__getitem__(this, i);\n"
	            /**/ /**/ "	} catch (UnboundItem) {\n"
	            /**/ /**/ "		continue;\n"
	            /**/ /**/ "	} catch (IndexError) {\n"
	            /**/ /**/ "		break;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	yield (i, item);\n"
	            /**/ /**/ "}"
	            /**/ "}&"
	            /**/ "?#{op:iter}" /*                */ "|${"
	            /**/ /**/ "local iter = Sequence.__iter__(this);\n"
	            /**/ /**/ "if (Iterator.advance(iter, start) < start)\n"
	            /**/ /**/ "	return none;\n"
	            /**/ /**/ "for (local i: [start:end]) {\n"
	            /**/ /**/ "	foreach (local item: iter) {\n"
	            /**/ /**/ "		yield (i, item);\n"
	            /**/ /**/ "		goto next_item;\n"
	            /**/ /**/ "	}\n"
	            /**/ /**/ "	break;\n"
	            /**/ /**/ "next_item:;\n"
	            /**/ /**/ "}\n"
	            /**/ /**/ "return none;"
	            /**/ "}"
	            "}"),
	/* TODO: __iter_reverse__ */
	/* TODO: __enumerate_reverse__ */
	/* TODO: __enumerate_items_reverse__ */

	/* Old function names/deprecated functions. */
	TYPE_METHOD("transform", &seq_map,
	            "(mapper:?DCallable)->?DSequence\n"
	            "Deprecated alias for ?#map"),
	TYPE_KWMETHOD("xch", &DeeMA_Sequence_xchitem,
	              "" DeeMA_Sequence_xchitem_doc "\n"
	              "Deprecated alias for ?#xchitem (will be removed soon)"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("front", &seq_front_deprecated,
	            "->\n"
	            "Deprecated alias for ?#first"),
	TYPE_METHOD("back", &seq_back_deprecated,
	            "->\n"
	            "Deprecated alias for ?#last"),
	TYPE_METHOD("empty", &seq_empty_deprecated,
	            "->?Dbool\n"
	            "Deprecated alias for ?#isempty"),
	TYPE_METHOD("non_empty", &DeeMA___seq_bool__,
	            "->?Dbool\n"
	            "Deprecated alias for ?#isnonempty"),
	TYPE_METHOD("at", &DeeMA___seq_getitem__,
	            "(index:?Dint)->\n"
	            "Deprecated alias for ${this[index]}"),
	TYPE_METHOD(STR_get, &DeeMA___seq_getitem__,
	            "(index:?Dint)->\n"
	            "Deprecated alias for ${this[index]}\n"
	            "In older versions of deemon, this function (as well as ${operator []}) "
	            /**/ "would modulate the given @index by the length of the Sequence. Starting "
	            /**/ "with deemon 200, this behavior no longer exists, and neither is it still "
	            /**/ "supported"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isfrozen(DeeObject *__restrict self) {
	/* TODO: This should be smarter than just this: */
	return_bool(Dee_TYPE(self) == &DeeSeq_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isempty(DeeObject *__restrict self) {
	int result = DeeObject_InvokeMethodHint(seq_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isnonempty(DeeObject *__restrict self) {
	int result = DeeObject_InvokeMethodHint(seq_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

#define seq_asseq DeeObject_NewRef
#define seq_asset generic_obj__asset
#define seq_asmap generic_obj__asmap

PRIVATE struct type_getset tpconst seq_getsets[] = {
	TYPE_GETTER("length", &default__seq_operator_sizeob,
	            "->?Dint\nAlias for ${##(this as Sequence)}"),
	TYPE_GETSET_BOUND(STR_first,
	                  &default__seq_getfirst,
	                  &default__seq_delfirst,
	                  &default__seq_setfirst,
	                  &default__seq_boundfirst,
	                  "->\n"
	                  "Access the first item of the Sequence\n"

	                  "When reading the attribute (as in ${x = Sequence.first(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __seq_first__: Object}" /*                                         */ "|${return this.__seq_first__;}&"
	                  /**/ "${property __set_first__: Object} (when ?#__seq_getitem_always_bound__)" /*   */ "|${return this.__set_first__;}&"
	                  /**/ "${function first: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${return this.first;}&"
	                  /**/ "?#{op:getitem}" /**/ "|${"
	                  /**/ /**/ "try {\n"
	                  /**/ /**/ "	return Sequence.__getitem__(this, 0);\n"
	                  /**/ /**/ "} catch (IndexError) {\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Sequence.first);"
	                  /**/ "}"
	                  /**/ "?#{op:iter}" /**/ "|${"
	                  /**/ /**/ "local iter = Sequence.__iter__(this);\n"
	                  /**/ /**/ "foreach (local item: iter)\n"
	                  /**/ /**/ "	return item;\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Sequence.first);"
	                  /**/ "}"
	                  "}\n"

	                  "When deleting the attribute (as in ${Sequence.first.delete(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __seq_first__: Object}" /*                                         */ "|${del this.__seq_first__;}&"
	                  /**/ "${property __set_first__: Object} (when ?#__seq_getitem_always_bound__)" /*   */ "|${del this.__set_first__;}&"
	                  /**/ "${function first: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${del this.first;}&"
	                  /**/ "?#{op:delitem}" /**/ "|${"
	                  /**/ /**/ "try {\n"
	                  /**/ /**/ "	Sequence.__delitem__(this, 0);\n"
	                  /**/ /**/ "} catch (IndexError) {\n"
	                  /**/ /**/ "}"
	                  /**/ "}"
	                  "}\n"

	                  "When setting the attribute (as in ${Sequence.first.set(mySeq, value)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __seq_first__: Object}" /*                                         */ "|${this.__seq_first__ = value;}&"
	                  /**/ "${property __set_first__: Object} (when ?#__seq_getitem_always_bound__)" /*   */ "|${this.__set_first__ = value;}&"
	                  /**/ "${function first: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${this.first = value;}&"
	                  /**/ "?#{op:setitem}" /**/ "|${Sequence.__setitem__(this, 0, value);}"
	                  "}"),
	TYPE_GETSET_BOUND(STR_last,
	                  &default__seq_getlast,
	                  &default__seq_dellast,
	                  &default__seq_setlast,
	                  &default__seq_boundlast,
	                  "->\n"
	                  "Access the last item of the Sequence\n"

	                  "When reading the attribute (as in ${x = Sequence.last(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __seq_last__: Object}" /*                                         */ "|${return this.__seq_last__;}&"
	                  /**/ "${property __set_last__: Object} (when ?#__seq_getitem_always_bound__)" /*   */ "|${return this.__set_last__;}&"
	                  /**/ "${function last: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${return this.last;}&"
	                  /**/ "?#{op:getitem}, ?#{op:size}" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "if (size) {\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		return Sequence.__getitem__(this, size - 1);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Sequence.last);"
	                  /**/ "}"
	                  /**/ "?#{op:iter}" /**/ "|${"
	                  /**/ /**/ "local iter = Sequence.__iter__(this);\n"
	                  /**/ /**/ "local item;\n"
	                  /**/ /**/ "foreach (item: iter) {\n"
	                  /**/ /**/ "	foreach (item: iter)\n"
	                  /**/ /**/ "		;\n"
	                  /**/ /**/ "	return item;\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Sequence.last);"
	                  /**/ "}"
	                  "}\n"

	                  "When deleting the attribute (as in ${Sequence.last.delete(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __seq_last__: Object}" /*                                         */ "|${del this.__seq_last__;}&"
	                  /**/ "${property __set_last__: Object} (when ?#__seq_getitem_always_bound__)" /*   */ "|${del this.__set_last__;}&"
	                  /**/ "${function last: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${del this.last;}&"
	                  /**/ "?#{op:delitem}, ?#{op:size}" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "if (size) {\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		Sequence.__delitem__(this, size - 1);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}"
	                  /**/ "}"
	                  "}\n"

	                  "When setting the attribute (as in ${Sequence.last.set(mySeq, value)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __seq_last__: Object}" /*                                         */ "|${this.__seq_last__ = value;}&"
	                  /**/ "${property __set_last__: Object} (when ?#__seq_getitem_always_bound__)" /*   */ "|${this.__set_last__ = value;}&"
	                  /**/ "${function last: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${this.last = value;}&"
	                  /**/ "?#{op:setitem}, ?#{op:size}" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "if (!size)\n"
	                  /**/ /**/ "	throw EmptySequence();\n"
	                  /**/ /**/ "Sequence.__setitem__(this, size - 1, value);"
	                  /**/ "}"
	                  "}"),

	TYPE_GETTER_AB("each", &DeeSeq_Each,
	               "->?Ert:SeqEach\n"
	               "Returns a special proxy object that mirrors any operation performed on "
	               /**/ "it onto each element of @this Sequence, evaluating to another proxy object "
	               /**/ "that allows the same, but also allows being used as a regular Sequence:\n"
	               "${"
	               /**/ "local locks = { get_lock(\"a\"), get_lock(\"b\") };\n"
	               /**/ "with (locks.each) { ... }\n"
	               /**/ "local strings = { \"foo\", \"bar\", \"foobar\" };\n"
	               /**/ "for (local x: strings.each.upper())\n"
	               /**/ "	print x; /* \"FOO\", \"BAR\", \"FOOBAR\" */\n"
	               /**/ "local lists = { [10, 20, 30], [1, 2, 3], [19, 41, 57] };\n"
	               /**/ "del lists.each[0];\n"
	               /**/ "print repr lists; /* { [20, 30], [2, 3], [41, 57] } */"
	               "}\n"
	               "WARNING: When invoking member functions, be sure to expand the generated "
	               /**/ "Sequence to ensure that the operator actually gets applied. "
	               /**/ "The only exception to this rule are operators that don't have an "
	               /**/ "actual return value and thus cannot be used in expand expressions:\n"
	               "${"
	               /**/ "local lists = { [10, 20, 30], [1, 2, 3], [19, 41, 57] };\n"
	               /**/ "lists.each.insert(0, 9)...; /* Expand the wrapped Sequence to ensure invocation */\n"
	               /**/ "lists.each[0] = 8;          /* No need for (or way to) expand in this case */\n"
	               /**/ "del lists.each[0];          /* No need for (or way to) expand in this case */"
	               "}"),
	TYPE_GETTER_AB("some", &DeeSeq_Some,
	               "->?Ert:SeqSome\n"
	               "Returns a custom proxy object that chains and forwards operators applied to it to "
	               /**/ "all of the items of @this ?., until a ${opreator bool} is used, at which point "
	               /**/ "!t is returned if at least one item exists in the original sequence that matches "
	               /**/ "the constructed condition:\n"
	               "${"
	               /**/ "local seq = { 1, 5, 9, 11 };\n"
	               /**/ "if (seq.some > 10) {\n"
	               /**/ "	...\n"
	               /**/ "}\n"
	               /**/ "\\\n"
	               /**/ "/* Same as: */\n"
	               /**/ "if (Sequence.any(seq.each > 10)) {\n"
	               /**/ "	...\n"
	               /**/ "}\n"
	               /**/ "\\\n"
	               /**/ "/* Similar to (assuming that `seq' doesn't contain `none') */\n"
	               /**/ "if (seq.locate(x -\\> x > 10) !is none) {\n"
	               /**/ "	...\n"
	               /**/ "}"
	               "}\n"
	               "Also useful when wanting to find the first/last of a set of objects within some sequence. "
	               /**/ "?. has no #Cfindany/#Cfind_first_of/etc. function because the same can be done like this:\n"
	               "${"
	               /**/ "local seq = { 1, 7, 13, 5, 99, 3 };\n"
	               /**/ "print seq.find({ 5, 7 }.some);  /* 1 */\n"
	               /**/ "print seq.rfind({ 5, 7 }.some); /* 3 */\n"
	               /**/ "\\\n"
	               /**/ "/* Similar to (except `seq' is only enumerated once) */\n"
	               /**/ "print (for (local x: { 5, 7 }) seq.find(x)) < ...;  /* 1 -- (would break if `seq' didn't contain both 5 and 7) */\n"
	               /**/ "print (for (local x: { 5, 7 }) seq.rfind(x)) > ...; /* 3 */"
	               "}\n"
	               "This works because ?#find and similar functions always perform comparisons with the "
	               /**/ "item being searched on the left-hand-side, meaning #Iits compare operators are "
	               /**/ "invoked in order to determine equality. As such, ${(seq.some == foo).operator bool()} "
	               /**/ "has been programmed to return the same as ${(foo in seq).operator bool()}"),
	TYPE_GETTER_AB("ids", &SeqIds_New,
	               "->?S?Dint\n"
	               "Returns a special proxy object for accessing the ids of Sequence elements\n"
	               "This is equivalent to ${this.transform(x -\\> Object.id(x))}"),
	TYPE_GETTER_AB("types", &SeqTypes_New,
	               "->?S?DType\n"
	               "Returns a special proxy object for accessing the types of Sequence elements\n"
	               "This is equivalent to ${this.transform(x -\\> type(x))}"),
	TYPE_GETTER_AB("classes", &SeqClasses_New,
	               "->?S?DType\n"
	               "Returns a special proxy object for accessing the classes of Sequence elements\n"
	               "This is equivalent to ${this.transform(x -\\> x.class)}"),
	/* TODO: hashes = this.map(e => e.operator hash());
	 * Needs special case because "this.each.operator hash()" wouldn't work */
	TYPE_GETTER("isempty", &seq_get_isempty,
	            "->?Dbool\n"
	            "Returns ?t if @this Sequence is empty\n"
	            "Implemented as (s.a. ?#{op:bool}):\n"
	            "${"
	            /**/ "property isempty: bool = {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		return !this.operator bool();\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("isnonempty", &seq_get_isnonempty,
	            "->?Dbool\n"
	            "Alias for ?#{op:bool}"),
	TYPE_GETTER_AB("flatten", &DeeSeq_Flat,
	               "->?Ert:SeqFlat\n"
	               "Flatten a sequence ${{{T...}...}} to ${{T...}}:"
	               "${"
	               /**/ "local x = { {0, 1, 2}, {5, 7, 9}, {1, 10, 1} };\n"
	               /**/ "print repr x.flatten; /* { 0, 1, 2, 5, 7, 9, 1, 10, 1 } */"
	               "}"),
	TYPE_GETTER_AB("asseq", &seq_asseq, "->?.\nOptimized version of ${this as Sequence}"),
	TYPE_GETTER_AB("asset", &seq_asset, "->?DSet\nOptimized version of ${this as Set}"),
	TYPE_GETTER_AB("asmap", &seq_asmap, "->?DMapping\nOptimized version of ${this as Mapping}"),

	/* TODO: Variants of this need to be added for `Set' and `Mapping' */
	TYPE_GETTER(STR_cached, &default__seq_cached,
	            "->?.\n"
	            "Returns a sequence that is a lazily-populated, read-only proxy of @this ?..\n"
	            "The returned proxy will ensure that any indirect side-effects¹ of accessing elements, "
	            /**/ "or iterating the sequence will only happen once upon initial access/iteration. "
	            /**/ "E.g. a yield-function is only executed one time, and only up the point that "
	            /**/ "elements have been requested for.\n"
	            "Sequence types where element access/iteration has no indirect side-effects¹, meaning "
	            /**/ "essentially every materialized ?. type (e.g. ?DList, ?DTuple, ?DDict, ...), can "
	            /**/ "implement this property as a no-op: ${property cached = { . -\\> this; }}\n"
	            "As such, ?#cached behaves similar to ?#frozen, except that rather than evaluating "
	            /**/ "the elements of the sequence immediately, said evaluation is delayed until "
	            /**/ "the first time an element are accessed. Additionally, there is no requirement "
	            /**/ "that changes to the original sequence won't propagate into the #Icached "
	            /**/ "sequence (even after the cache may have been populated).\n"
	            "The difference between some $seq and ${seq.cached} is that the later "
	            /**/ "will allow for #B{O(1)} access to elements after an initial access, and "
	            /**/ "that any indirect side-effects¹ of accessing some element will happen "
	            /**/ "exactly once.\n"
	            "${"
	            /**/ "/* \"lines\" will lazily populate itself from the contents of \"data.txt\" */\n"
	            /**/ "local lines = Sequence.cached(File.open(\"data.txt\", \"rb\"));\n"
	            /**/ "print lines.locate(line -\\> !line.startswith(\"##\"));\n"
	            /**/ "\\\n"
	            /**/ "/* This will print the first couple of lines from the cache, up until (and\n"
	            /**/ " * including) the already-accessed line, then continue reading from \"data.txt\" */\n"
	            /**/ "for (local line: lines)\n"
	            /**/ "	print line;"
	            "}\n"
	            "${"
	            /**/ "function getItems() {\n"
	            /**/ "	print \"Enter\";\n"
	            /**/ "	yield 10;\n"
	            /**/ "	print \"Middle\";\n"
	            /**/ "	yield 20;\n"
	            /**/ "	print \"Leave\";\n"
	            /**/ "}\n"
	            /**/ "\\\n"
	            /**/ "local items = getItems().cached;\n"
	            /**/ "print items[0];   /* Enter 10 */\n"
	            /**/ "print items[0];   /* 10 */\n"
	            /**/ "print items[1];   /* Middle 20 */\n"
	            /**/ "print items[0];   /* 10 */\n"
	            /**/ "print items[1];   /* 20 */\n"
	            /**/ "print ##items;     /* Leave 3 */\n"
	            /**/ "print repr items; /* { 10, 20, 30 } */"
	            "}\n"
	            "#L{"
	            /**/ "{¹}An indirect side-effect is a change to the program's global state, usually caused "
	            /**/ /**/ "by the invocation of user-code. Examples include: changes to global variables, "
	            /**/ /**/ "output being written to a file, or text being printed to stdout."
	            "}"),

	/* TODO: itemtype->?DType
	 *       Check if the type of @this overrides the ?#ItemType class attribute.
	 *       If so, return its value; else, return the common base-class of all
	 *       items in @this ?.. When @this is empty, ?O is returned. */

	/* TODO: ItemType->?DType   (class property)
	 *       When this type of ?. only allows items of a certain ?DType,
	 *       this class attribute is overwritten with that ?DType. Else,
	 *       it simply evaluates to ?O */

	TYPE_GETTER("isfrozen", &seq_get_isfrozen,
	            "->?Dbool\n"
	            "Evaluates to true if the ?Aid?Os of elements of "
	            /**/ "@this Sequence can never change though use of any non-implementation-"
	            /**/ "specific functions/attributes (i.e. anything that doesn't match #C{__*__}).\n"
	            "This differs from the inverse of ?#ismutable, as in the case "
	            /**/ "of a proxy Sequence, this property depends on the underlying "
	            /**/ "Sequence, and the kind of transformation applied to it, rather "
	            /**/ "than what is exposed by the proxy itself"),
	TYPE_GETTER(STR_frozen, &default__seq_frozen,
	            "->?#Frozen\n"
	            "Returns a copy of @this Sequence, with all of its current elements, as well as "
	            /**/ "their current order frozen in place, constructing a snapshot of the Sequence's "
	            /**/ "current elements. - The actual type of Sequence returned is implementation- "
	            /**/ "and type- specific, but a guaranty is made that no non-implementation-specific "
	            /**/ "functions/attributes (i.e. anything that doesn't match #C{__*__}) will be able "
	            /**/ "to change the elements of the returned sequence.\n"
	            "By default, this attribute simply casts @this Sequence into a ?DTuple, but sequence "
	            /**/ "types that are known to already be immutable can override this attribute such "
	            /**/ "that they simple re-return themselves.\n"
	            "Note that this attributes does NOT perform a deep copy, and does NOT protect from "
	            "potential changes made to the state of the elements of @this sequence. It ONLY makes "
	            "a snapshot of the sequence itself. If you want to construct a frozen deep copy, you "
	            "should do the following instead (assuming that elements of the sequence support being "
	            "deep-copied):\n"
	            "${"
	            /**/ "local s = getSeq();\n"
	            /**/ "local s = (for (local x: s) deepcopy x).frozen;"
	            "}"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_range(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	/*  Offering the same functionality as the legacy `util::range()',
	 * `Sequence.range()' is the new builtin way of getting this
	 *  behavior from a core function (since `Sequence' is a
	 *  builtin type like `List', `Tuple', etc.). */
	DeeObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("range", params: """
	DeeObject *start;
	DeeObject *end = NULL;
	DeeObject *step = NULL;
""");]]]*/
	struct {
		DeeObject *start;
		DeeObject *end;
		DeeObject *step;
	} args;
	args.end = NULL;
	args.step = NULL;
	DeeArg_UnpackStruct1Or2Or3(err, argc, argv, "range", &args, &args.start, &args.end, &args.step);
/*[[[end]]]*/
	if (args.end)
		return DeeRange_New(args.start, args.end, args.step);
	/* Use a default-constructed instance of `type(start)' for the real start. */
	args.end = DeeObject_NewDefault(Dee_TYPE(args.start));
	if unlikely(!args.end)
		goto err;
	result = DeeRange_New(args.end, args.start, NULL);
	Dee_Decref(args.end);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_repeatitem(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
#define seq_repeatitem_params "item,count?:?Dint"
	switch (argc) {
	case 1:
		return DeeSeq_RepeatItemForever(argv[0]);
	case 2: {
		size_t count;
		if (DeeObject_AsSize(argv[1], &count))
			goto err;
		return DeeSeq_RepeatItem(argv[0], count);
	}	break;
	default:
		DeeArg_BadArgcEx("repeatitem", argc, 1, 2);
		break;
	}
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
seq_class_repeat(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
#define seq_repeat_params "seq:?.,count?:?Dint"
	switch (argc) {
	case 1:
		return DeeSeq_RepeatForever(argv[0]);
	case 2: {
		size_t count;
		if (DeeObject_AsSize(argv[1], &count))
			goto err;
		return DeeSeq_Repeat(argv[0], count);
	}	break;
	default:
		DeeArg_BadArgcEx("repeat", argc, 1, 2);
		break;
	}
err:
	return NULL;
}

INTDEF DeeTypeObject SeqConcat_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_concat(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	if (!argc)
		return DeeSeq_NewEmpty();
	if (argc == 1)
		return_reference_(argv[0]);
	result = DeeTuple_NewVector(argc, argv);
	if likely(result) {
		ASSERT(result->ob_type == &DeeTuple_Type);
		Dee_DecrefNokill(&DeeTuple_Type);
		Dee_Incref(&SeqConcat_Type);
		result->ob_type = &SeqConcat_Type;
	}
	return result;
}

PRIVATE struct type_method tpconst seq_class_methods[] = {
	TYPE_METHOD("range", &seq_class_range,
	            "(end:?Dint)->?S?Dint\n"
	            "(end)->?DSequence\n"
	            "(start:?Dint,end:?Dint)->?S?Dint\n"
	            "(start,end)->?DSequence\n"
	            "(start:?Dint,end:?Dint,step:?Dint)->?S?Dint\n"
	            "(start,end,step)->?DSequence\n"
	            "Create a new Sequence object for enumeration of indices. "
	            /**/ "This function is a simple wrapper for the same "
	            /**/ "functionality available through the following usercode:\n"
	            "${"
	            /**/ "local x = [:end];\n"
	            /**/ "local x = [start:end];\n"
	            /**/ "local x = [start:end, step];"
	            "}"),
	TYPE_METHOD("repeatitem", &seq_class_repeatitem,
	            "(" seq_repeatitem_params ")->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Create a proxy-Sequence that yields @item a total of @count times. "
	            /**/ "When @count is omitted, @item is yielded forever.\n"
	            "The main purpose of this function is to construct large sequences "
	            /**/ "to be used as initializers for mutable sequences such as ?DList"),
	TYPE_METHOD("repeat", &seq_class_repeat,
	            "(" seq_repeat_params ")->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Repeat all the elements from @seq a total of @count times. "
	            /**/ "When @count is omitted, @seq is repeated forever.\n"
	            "This is the same as ${(seq as Sequence from deemon) * count}"),
	TYPE_METHOD("concat", &seq_class_concat,
	            "(seqs!:?DSequence)->?DSequence\n"
	            "Returns a proxy-Sequence describing the concatenation of all of the given sequences\n"
	            "When only 1 Sequence is given, that Sequence is forwarded directly.\n"
	            "When no sequences are given, an empty Sequence is returned\n"
	            "Hint: The python equivalent of this function is "
	            /**/ "#A{itertools.chain|https://docs.python.org/3/library/itertools.html##itertools.chain}"),
	TYPE_METHOD_END
};


PRIVATE struct type_operator const seq_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_CONSTELEM_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_CONSTELEM_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL),
};


PRIVATE struct type_member tpconst seq_class_members[] = {
	/* Here primarily to allow doc strings to do stuff like `{string...}.Some'
	 * to indicate "(sequence of string).some" */
	TYPE_MEMBER_CONST("Some", &DeeSeqSome_Type),

	/* TODO: Deprecated -- remove these (librt should create custom scenarios for all of these) */
	TYPE_MEMBER_CONST("__SeqWithIter__", &DefaultSequence_WithIter_Type),

	TYPE_MEMBER_CONST("__IterWithForeach__", &DefaultIterator_WithForeach_Type),
	TYPE_MEMBER_CONST("__IterWithForeachPair__", &DefaultIterator_WithForeachPair_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateMap__", &DefaultIterator_WithEnumerateMap_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateIndexSeq__", &DefaultIterator_WithEnumerateIndexSeq_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateSeq__", &DefaultIterator_WithEnumerateSeq_Type),
	TYPE_MEMBER_END
};



PRIVATE struct type_cmp seq_cmp = {
	/* .tp_hash          = */ &default__seq_operator_hash,
	/* .tp_compare_eq    = */ &default__seq_operator_compare_eq,
	/* .tp_compare       = */ &default__seq_operator_compare,
	/* .tp_trycompare_eq = */ &default__seq_operator_trycompare_eq,
	/* .tp_eq            = */ &default__seq_operator_eq,
	/* .tp_ne            = */ &default__seq_operator_ne,
	/* .tp_lo            = */ &default__seq_operator_lo,
	/* .tp_le            = */ &default__seq_operator_le,
	/* .tp_gr            = */ &default__seq_operator_gr,
	/* .tp_ge            = */ &default__seq_operator_ge,
};

PRIVATE struct type_seq seq_seq = {
	/* .tp_iter                       = */ &default__seq_operator_iter,
	/* .tp_sizeob                     = */ &default__seq_operator_sizeob,
	/* .tp_contains                   = */ &default__seq_operator_contains,
	/* .tp_getitem                    = */ &default__seq_operator_getitem,
	/* .tp_delitem                    = */ &default__seq_operator_delitem,
	/* .tp_setitem                    = */ &default__seq_operator_setitem,
	/* .tp_getrange                   = */ &default__seq_operator_getrange,
	/* .tp_delrange                   = */ &default__seq_operator_delrange,
	/* .tp_setrange                   = */ &default__seq_operator_setrange,
	/* .tp_foreach                    = */ &default__seq_operator_foreach,
	/* .tp_foreach_pair               = */ &default__seq_operator_foreach_pair,
	/* .tp_bounditem                  = */ &default__seq_operator_bounditem,
	/* .tp_hasitem                    = */ &default__seq_operator_hasitem,
	/* .tp_size                       = */ &default__seq_operator_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ &default__seq_operator_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ &default__seq_operator_delitem_index,
	/* .tp_setitem_index              = */ &default__seq_operator_setitem_index,
	/* .tp_bounditem_index            = */ &default__seq_operator_bounditem_index,
	/* .tp_hasitem_index              = */ &default__seq_operator_hasitem_index,
	/* .tp_getrange_index             = */ &default__seq_operator_getrange_index,
	/* .tp_delrange_index             = */ &default__seq_operator_delrange_index,
	/* .tp_setrange_index             = */ &default__seq_operator_setrange_index,
	/* .tp_getrange_index_n           = */ &default__seq_operator_getrange_index_n,
	/* .tp_delrange_index_n           = */ &default__seq_operator_delrange_index_n,
	/* .tp_setrange_index_n           = */ &default__seq_operator_setrange_index_n,
	/* .tp_trygetitem                 = */ &default__seq_operator_trygetitem,
	/* .tp_trygetitem_index           = */ &default__seq_operator_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ &default__trygetitem_string_hash__with__trygetitem,
	/* .tp_getitem_string_hash        = */ &default__getitem_string_hash__with__getitem,
	/* .tp_delitem_string_hash        = */ &default__delitem_string_hash__with__delitem,
	/* .tp_setitem_string_hash        = */ &default__setitem_string_hash__with__setitem,
	/* .tp_bounditem_string_hash      = */ &default__bounditem_string_hash__with__bounditem,
	/* .tp_hasitem_string_hash        = */ &default__hasitem_string_hash__with__hasitem,
	/* .tp_trygetitem_string_len_hash = */ &default__trygetitem_string_len_hash__with__trygetitem,
	/* .tp_getitem_string_len_hash    = */ &default__getitem_string_len_hash__with__getitem,
	/* .tp_delitem_string_len_hash    = */ &default__delitem_string_len_hash__with__delitem,
	/* .tp_setitem_string_len_hash    = */ &default__setitem_string_len_hash__with__setitem,
	/* .tp_bounditem_string_len_hash  = */ &default__bounditem_string_len_hash__with__bounditem,
	/* .tp_hasitem_string_len_hash    = */ &default__hasitem_string_len_hash__with__hasitem,
	/* .tp_asvector                   = */ NULL,
	/* .tp_asvector_nothrow           = */ NULL,
};


#ifdef CONFIG_NO_DOC
#define seq_doc NULL
#else /* CONFIG_NO_DOC */
PRIVATE char const seq_doc[] =
"A recommended abstract base class for any Sequence "
/**/ "type that wishes to implement a Sequence protocol\n"
"When derived from, ?. implements numerous Sequence-related "
/**/ "member functions, such as ?#find or ?#reduce, as well as "
/**/ "operators such as ?#{op:add} and ?#{op:eq}\n"
"An object derived from this class should implement at at least one of:\n"
"#L-{"
/**/ "${operator iter}, aka. ${operator for} (?#{op:iter})|"
/**/ "${operator [] (index: int): Object} (?#{op:getitem}) and ${operator ## (): int} (?#{op:size})"
"}\n"
"The abstract declaration syntax for a generic Sequence is ${{Object...}}\n"
"Types derived from ?. will automatically have missing sequence-related operators "
/**/ "substituted when not explicitly implemented by the user, based on present "
/**/ "member functions and operators. This behavior is further documented for each "
/**/ "operator/function individually.\n"
"\n"

"()\n"
"A no-op default constructor that is implicitly called by sub-classes\n"
"When invoked directly, an empty, general-purpose Sequence equivalent "
/**/ "to ?Ert:Sequence_empty is returned\n"
"\n"

"bool->\n"
"Returns ?t/?f indicative of @this Sequence being non-empty. (s.a. ?#__bool__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_bool__(): bool}" /*                     */ "|${"
/**/ /**/ "local r = this.__seq_bool__();\n"
/**/ /**/ "if (r !is bool)\n"
/**/ /**/ "	throw TypeError(...);\n"
/**/ /**/ "return !!r;"
/**/ "}&"
/**/ "${operator bool} (?A__seqclass__?DType is ?.)" /*        */ "|${return !!this;}&"
/**/ "${operator bool} (?A__seqclass__?DType is ?DSet)" /*     */ "|${return !!this;}&"
/**/ "${operator bool} (?A__seqclass__?DType is ?DMapping)" /* */ "|${return !!this;}&"
/**/ "?A__bool__?DSet (when ?#__seq_getitem_always_bound__)" /**/ "|${return !Set.__bool__(this);}&"
/**/ "?A__size__?DSet (when ?#__seq_getitem_always_bound__)" /**/ "|${return !!Set.__size__(this);}&"
/**/ "?#equals" /*                                             */ "|${return !Sequence.equals(this, {});}&"
/**/ "?Aequals?DSet" /*                                        */ "|${return !Set.equals(this, {} as Set);}&"
/**/ "?Aequals?DMapping" /*                                    */ "|${return !Mapping.equals({} as Mapping);}&"
/**/ "?#{op:size}" /*                                          */ "|${return !!##(this as Sequence);}"
"}\n"
"\n"


"#->\n"
"Returns the length of @this Sequence, as determinable by enumeration (s.a. ?#__size__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_size__(): int}" /*            */ "|${return this.__seq_size__();}&"
/**/ "${operator ##} (?A__seqclass__?DType is ?.)" /**/ "|${return ##this;}&"
/**/ "?A__size__?DSet (when ?#__seq_getitem_always_bound__)" /**/ "|${return Set.__size__(this);}&"
/**/ "?#{op:iter}" /**/ "|${"
/**/ /**/ "local result = 0;\n"
/**/ /**/ "foreach (none: Sequence.__iter__(this))\n"
/**/ /**/ "	++result;\n"
/**/ /**/ "return result;"
/**/ "}"
"}\n"
"\n"


"iter->\n"
"Construct an iterator for enumerating @this Sequence. "
/**/ "The iterator may be implemented using a size+index approach. (s.a. ?#__iter__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_iter__(): Iterator}" /**/ "|${return this.__seq_iter__();}&"
/**/ "${operator iter} (any type of ?O)" /*   */ "|${return this.operator iter();}&"
/**/ "?#{op:getitem}, ?#{op:size}" /*         */ "|${"
/**/ /**/ "for (local i: [:Sequence.__size__(this)]) {\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		yield Sequence.__getitem__(this, i);\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}&"
/**/ "?#{op:getitem}" /*                      */ "|${"
/**/ /**/ "for (local i = 0;; ++i) {\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		yield Sequence.__getitem__(this, i);\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}&"
/**/ "${operator iter} (?DSet)|${return Set.__iter__(this);}&"
/**/ "${operator iter} (?DMapping)|${return Mapping.__iter__(this);}"
"}\n"
"\n"


"[]->\n"
"#tOverflowError{The given @index is negative}"
"#tIndexError{The given @index is greater than the length of @this Sequence (s.a. ?#{op:size})}"
"#tUnboundItem{The item associated with @index is unbound}"
"Returns the @{index}th element of @this Sequence. (s.a. ?#__getitem__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_getitem__(index: int): Object}" /**/ "|${return this.__seq_getitem__(index);}&"
/**/ "${operator []} (?A__seqclass__?DType is ?.)" /*    */ "|${return this[index];}&"
/**/ "?#{op:iter}" /*                                    */ "|${"
/**/ /**/ "local i = 0;\n"
/**/ /**/ "foreach (local v: Sequence.__iter__(this)) {\n"
/**/ /**/ "	if (i >= index)\n"
/**/ /**/ "		return v;\n"
/**/ /**/ "	++i;\n"
/**/ /**/ "}\n"
/**/ /**/ "throw IndexError(seq: this, index: index, length: i);"
/**/ "}"
"}\n"
"\n"


"del[]->\n"
"#tIntegerOverflow{The given @index is negative, or too large}"
"#tIndexError{The given @index is out of bounds}"
"Either remove (as per ?#erase) the item under @index (?#{op:size} changes), or mark "
/**/ "said item as unbound (?#{op:size} remains unchanged). (s.a. ?#__delitem__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_delitem__(index: int)}" /*       */ "|${return this.__seq_delitem__(index);}&"
/**/ "${operator del[]} (?A__seqclass__?DType is ?.)" /**/ "|${del this[index];}&"
/**/ "?#erase, ?#{op:size}" /*                          */ "|${"
/**/ /**/ "if (index < 0)\n"
/**/ /**/ "	throw IntegerOverflow(...);\n"
/**/ /**/ "local size = Sequence.__size__(this);\n"
/**/ /**/ "if (index >= size)\n"
/**/ /**/ "	throw IndexError(seq: this, index: index, length: size);\n"
/**/ /**/ "Sequence.erase(this, index, 1);"
/**/ "}"
"}\n"
"\n"


"[]=->\n"
"#tIntegerOverflow{The given @index is negative, or too large}"
"#tIndexError{The given @index is out of bounds}"
"Set the value of @index to @value. (s.a. ?#__setitem__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_setitem__(index: int, value: Object)}" /**/ "|${return this.__seq_setitem__(index, value);}&"
/**/ "${operator []=} (?A__seqclass__?DType is ?.)" /*          */ "|${this[index] = value;}"
"}\n"
"\n"


"[:](start:?X2?N?Dint,end:?X2?N?Dint)->\n"
"Returns a sub-range of @this Sequence, spanning across all elements from @start to @end. (s.a. ?#__getrange__)\n"
"If either @start or @end is smaller than ${0}, ${##this} is added once to either\n"
"If @end is greater than the length of @this Sequence, it is clamped to its length\n"
"When @start is greater than, or equal to @end or ${##this}, an empty Sequence is returned\n"
"This operator is implemented similar to the following, however the actual "
/**/ "return type may be a proxy Sequence that further optimizes the iteration "
/**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
/**/ "as how the sub-range is accessed (i.e. ${this[10:20][3]} will invoke ${this[13]}).\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_getrange__(start: int | none, end: int | none)}: Sequence" /**/ "|${return this.__seq_getrange__(start, end);}&"
/**/ "${operator [:]} (?A__seqclass__?DType is ?.)" /*                              */ "|${return this[start:end];}&"
/**/ "?#{op:getitem}, ?#{op:size}" /*                                               */ "|${"
/**/ /**/ "start, end = util.clamprange(start, end, Sequence.__size__(this))...;\n"
/**/ /**/ "for (local i: [start:end]) {\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		yield Sequence.__getitem__(this, i);\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}&"
/**/ "?#{op:iter}, ?#{op:size}" /*                                                  */ "|${"
/**/ /**/ "start, end = util.clamprange(start, end, Sequence.__size__(this))...;\n"
/**/ /**/ "local it = Sequence.__iter__(this);\n"
/**/ /**/ "Iterator.advance(it, start);\n"
/**/ /**/ "for (none: [start:end]) {\n"
/**/ /**/ "	foreach (local x: it) {\n"
/**/ /**/ "		yield x;\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}"
"}\n"
"\n"


"del[:]->\n"
"#tIntegerOverflow{@start or @end are too large}"
"#tSequenceError{@this Sequence cannot be resized}"
"Delete, or unbind all items within the given range. (s.a. ?#__delrange__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_delrange__(start: int | none, end: int | none)}" /**/ "|${this.__seq_delrange__(start, end);}&"
/**/ "${operator del[:]} (?A__seqclass__?DType is ?.)" /*                 */ "|${del this[start:end];}&"
/**/ "?#{op:setrange}" /*                                                 */ "|${Sequence.__setrange__(this, start, end, none);}&"
/**/ "?#erase, ?#{op:size}" /*                                            */ "|${"
/**/ /**/ "start, end = util.clamprange(start, end, Sequence.__size__(this));\n"
/**/ /**/ "if (start < end)\n"
/**/ /**/ "	Sequence.erase(this, start, end - start);"
/**/ "}"
"}\n"
"\n"


"[:]=->\n"
"#tIntegerOverflow{@start or @end are too large}"
"#tSequenceError{@this Sequence is immutable, or cannot be resized}"
"Override the given range with items from @{values}. With @values is smaller than "
/**/ "the target range, @this sequence will be shrunk. When it is greater, it will grow. "
/**/ "Note that unlike ?#{op:delrange}, this operator is required to resize the sequence, and "
/**/ "is not allowed to leave items unbound. (s.a. ?#__setrange__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_setrange__(start: int | none, end: int | none, values: {Object...})}" /**/ "|${this.__seq_setrange__(start, end, values);}&"
/**/ "${operator [:]=} (?A__seqclass__?DType is ?.)" /*                                        */ "|${this[start:end] = values;}&"
/**/ "?#{op:size}, ?#erase, ?#insertall" /*                                                    */ "|${"
/**/ /**/ "start, end = util.clamprange(start, end, Sequence.__size__(this));\n"
/**/ /**/ "if (start < end)\n"
/**/ /**/ "	Sequence.erase(this, start, end - start);\n"
/**/ /**/ "Sequence.insertall(this, start, values);"
/**/ "}"
"}\n"
"\n"


":=(items:?X2?DSequence?S?O)->\n"
"Overeride the items of @this sequence with those of @other "
/**/ "(semantically equivalent to ${this[:] = items}) (s.a. ?#__assign__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_assign__(items: {Object...})}" /**/ "|${this.__seq_assign__(items);}&"
/**/ "${operator :=} (?A__seqclass__?DType is ?.)" /*   */ "|${this[start:end] = items;}&"
/**/ "?#{op:setrange}" /*                               */ "|${Sequence.__setrange__(this, none, none, items);}"
"}\n"
"\n"


"hash->\n"
"Returns the hash of all items of @this ?. (s.a. ?#__hash__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_hash__(): int}" /*              */ "|${return this.__seq_hash__().operator int();}&"
/**/ "${operator hash} (?A__seqclass__?DType is ?.)" /**/ "|${return this.operator hash().operator int();}&"
/**/ "?#{op:iter}" /*                                  */ "|${"
/**/ /**/ "local result = rt.HASHOF_EMPTY_SEQUENCE;\n"
/**/ /**/ "foreach (local x: Sequence.__iter__(this))\n"
/**/ /**/ "	result = deemon.hash(result, x);\n"
/**/ /**/ "return result;"
/**/ "}"
"}\n"
"\n"


"==(rhs:?X2?DSequence?S?O)->\n"
"Returns ?t/?f indicative of a @this and @rhs being equal. (s.a. ?#__eq__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_eq__(rhs: Sequence): bool}" /**/ "|${return this.__seq_eq__(rhs);}&"
/**/ "${operator ==} (?A__seqclass__?DType is ?.)" /**/ "|${return this == rhs;}&"
/**/ "${operator !=} (?A__seqclass__?DType is ?.)" /**/ "|${return !(this != rhs);}&"
/**/ "?#equals" /*                                   */ "|${return Sequence.equals(this, rhs);}"
"}\n"
"\n"


"!=(rhs:?X2?DSequence?S?O)->\n"
"Returns ?t/?f indicative of a @this and @rhs begin non-equal. (s.a. ?#__ne__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_ne__(rhs: Sequence): bool}" /**/ "|${return this.__seq_ne__(rhs);}&"
/**/ "${operator !=} (?A__seqclass__?DType is ?.)" /**/ "|${return this != rhs;}&"
/**/ "${operator ==} (?A__seqclass__?DType is ?.)" /**/ "|${return !(this == rhs);}&"
/**/ "?#equals" /*                                   */ "|${return !Sequence.equals(this, rhs);}"
"}\n"
"\n"


"<(rhs:?X2?DSequence?S?O)->\n"
"Returns ?t/?f indicative of a lexicographical comparison between @this and @rhs. (s.a. ?#__lo__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_lo__(rhs: Sequence): bool}" /**/ "|${return this.__seq_lo__(rhs);}&"
/**/ "${operator <} (?A__seqclass__?DType is ?.)" /* */ "|${return this < rhs;}&"
/**/ "${operator >=} (?A__seqclass__?DType is ?.)" /**/ "|${return !(this >= rhs);}&"
/**/ "?#compare" /*                                  */ "|${return Sequence.compare(this, rhs) < 0;}"
"}\n"
"\n"


"<=(rhs:?X2?DSequence?S?O)->\n"
"Returns ?t/?f indicative of a lexicographical comparison between @this and @rhs. (s.a. ?#__le__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_le__(rhs: Sequence): bool}" /**/ "|${return this.__seq_le__(rhs);}&"
/**/ "${operator <=} (?A__seqclass__?DType is ?.)" /**/ "|${return this <= rhs;}&"
/**/ "${operator >} (?A__seqclass__?DType is ?.)" /* */ "|${return !(this > rhs);}&"
/**/ "?#compare" /*                                  */ "|${return Sequence.compare(this, rhs) <= 0;}"
"}\n"
"\n"


">(rhs:?X2?DSequence?S?O)->\n"
"Returns ?t/?f indicative of a lexicographical comparison between @this and @rhs. (s.a. ?#__gr__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_gr__(rhs: Sequence): bool}" /**/ "|${return this.__seq_gr__(rhs);}&"
/**/ "${operator >} (?A__seqclass__?DType is ?.)" /* */ "|${return this > rhs;}&"
/**/ "${operator <=} (?A__seqclass__?DType is ?.)" /**/ "|${return !(this <= rhs);}&"
/**/ "?#compare" /*                                  */ "|${return Sequence.compare(this, rhs) > 0;}"
"}\n"
"\n"


">=(rhs:?X2?DSequence?S?O)->\n"
"Returns ?t/?f indicative of a lexicographical comparison between @this and @rhs. (s.a. ?#__ge__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_ge__(rhs: Sequence): bool}" /**/ "|${return this.__seq_ge__(rhs);}&"
/**/ "${operator >=} (?A__seqclass__?DType is ?.)" /**/ "|${return this >= rhs;}&"
/**/ "${operator <} (?A__seqclass__?DType is ?.)" /* */ "|${return !(this < rhs);}&"
/**/ "?#compare" /*                                  */ "|${return Sequence.compare(this, rhs) >= 0;}"
"}\n"
"\n"


"+(rhs:?X2?DSequence?S?O)->\n"
"Returns a proxy Sequence for accessing elements from @this "
/**/ "Sequence, and those from @rhs in a seamless stream of items. (s.a. ?#__add__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_add__(rhs: Sequence): Sequence}" /**/ "|${return this.__seq_add__(rhs);}&"
/**/ "${operator +} (?A__seqclass__?DType is ?.)" /*      */ "|${return this + rhs;}&"
/**/ "?#__iter__ ¹" /*                                    */ "|${"
/**/ /**/ "foreach (local item: Sequence.__iter__(this))\n"
/**/ /**/ "	yield item;"
/**/ /**/ "yield rhs...;"
/**/ "}"
"}"
"#L{"
/**/ "{¹}The actual return type may be a proxy Sequence that further optimizes the iteration "
/**/ /**/ "strategy used, based on which operators have been implemented by sub-classes, "
/**/ /**/ "as well as how the sub-range is accessed (e.g. ${##(this + rhs)} will invoke "
/**/ /**/ "${(##this).operator int() + (#rhs).operator int()})."
"}\n"
"\n"


"*(repeat:?Dint)->\n"
"#tIntegerOverflow{@repeat is negative, or greater than ?ASIZE_MAX?Dint}"
"Returns a proxy Sequence for accessing the elements of @this "
/**/ "Sequence, consecutively repeated a total of @repeat times. (s.a. ?#__mul__)\n"
"The implementation is allowed to assume that the number and value of "
/**/ "items of @this Sequence don't change between multiple iterations\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_mul__(repeat: int): Sequence}" /**/ "|${return this.__seq_mul__(repeat);}&"
/**/ "${operator *} (?A__seqclass__?DType is ?.)" /*    */ "|${return this * repeat;}&"
/**/ "?#__iter__ ¹" /*                                  */ "|${"
/**/ /**/ "for (none: [:repeat.operator int()]) {\n"
/**/ /**/ "	foreach (local item: Sequence.__iter__(this))\n"
/**/ /**/ "		yield item;\n"
/**/ /**/ "}"
/**/ "}"
"}"
"#L{"
/**/ "{¹}The actual return type may be a proxy Sequence that further optimizes the iteration "
/**/ /**/ "strategy used, based on which operators have been implemented by sub-classes, "
/**/ /**/ "as well as how the sub-range is accessed (i.e. ${##(this * 4)} will invoke "
/**/ /**/ "${(##this).operator int() * 4}).\n"
"}\n"
"\n"


"+=(rhs:?X2?DSequence?S?O)->\n"
"Append items from @rhs onto @this sequence, or replace @this "
/**/ "sequence with a proxy containing items from both sides. (s.a. ?#__inplace_add__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_inplace_add__(rhs: Sequence): Sequence}" /**/ "|${this = this.__seq_inplace_add__(rhs);}&"
/**/ "${operator +=} (?A__seqclass__?DType is ?.)" /*             */ "|${this += rhs;}&"
/**/ "?#extend" /*                                                */ "|${Sequence.extend(this, rhs);}&"
/**/ "?#{op:add}" /*                                              */ "|${this = Sequence.__add__(this, rhs);}"
"}\n"
"\n"


"*=(repeat:?Dint)->\n"
"#tIntegerOverflow{@repeat is negative, or greater than ?ASIZE_MAX?Dint}"
"Repeat items from @this onto @repeat times, and replace @this with the result. (s.a. ?#__inplace_mul__)\n"
"#T{Requirements|Implementation~"
/**/ "${function __seq_inplace_mul__(repeat: int): Sequence}" /**/ "|${this = this.__seq_inplace_mul__(repeat);}&"
/**/ "${operator *=} (?A__seqclass__?DType is ?.)" /*           */ "|${this *= repeat;}&"
/**/ "?#extend, ?#clear, ?#{op:mul}" /*                         */ "|${"
/**/ /**/ "repeat = repeat.operator int();\n"
/**/ /**/ "if (repeat == 0) {\n"
/**/ /**/ "	Sequence.clear(this);\n"
/**/ /**/ "} else if (repeat != 1) {\n"
/**/ /**/ "	local extra = rt.SeqRepeat(this, repeat - 1);\n"
/**/ /**/ "	local extra = Sequence.frozen(extra);\n"
/**/ /**/ "	Sequence.extend(this, extra);\n"
/**/ /**/ "}"
/**/ "}&"
/**/ "?#{op:mul}" /*                                            */ "|${this = Sequence.__mul__(this, repeat);}"
"}\n"
"\n"


"contains->\n"
"Returns ?t/?f indicative of @item being apart of @this Sequence.\n"
"Alias for ${Sequence.contains(this, item)}. (s.a. ?#contains)\n"
"\n"


"repr->\n"
"Returns the representation of all Sequence elements, "
/**/ "using abstract Sequence syntax\n"
"e.g.: ${{ 10, 20, \"foo\" }}\n"
"${"
/**/ "operator repr(fp: File) {\n"
/**/ "	print fp: \"{ \",;\n"
/**/ "	local isFirst = true;\n"
/**/ "	foreach (local item: Sequence.__iter__(this)) {\n"
/**/ "		if (!isFirst)\n"
/**/ "			print fp: \", \",;\n"
/**/ "		isFirst = false;\n"
/**/ "		print fp: repr item,;\n"
/**/ "	}\n"
/**/ "	if (!isFirst)\n"
/**/ "		print fp: \" \",;\n"
/**/ "	print fp: \"}\",;\n"
/**/ "}"
"}"

#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
"\n"
"\n"
"~->?DSet\n"
"Alias for ${(this as Set).operator ~ ()}. (s.a. ?A{op:inv}?DSet)\n"
"\n"

"sub->?DSet\n"
"Alias for ${(this as Set).operator - (other)}. (s.a. ?A{op:sub}?DSet)\n"
"\n"

"|->?DSet\n"
"Alias for ${(this as Set).operator | (other)}. (s.a. ?A{op:or}?DSet)\n"
"\n"

"&->?DSet\n"
"Alias for ${(this as Set).operator & (other)}. (s.a. ?A{op:and}?DSet)\n"
"\n"

"^->?DSet\n"
"Alias for ${(this as Set).operator ^ (other)}. (s.a. ?A{op:xor}?DSet)"
#endif /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
"";
#endif /* !CONFIG_NO_DOC */



/* `Sequence from deemon' */
PUBLIC DeeTypeObject DeeSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Sequence),
	/* .tp_doc      = */ seq_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_SEQ << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&DeeNone_OperatorCtor, /* Allow default-construction of Sequence objects. */
				/* .tp_copy_ctor = */ (Dee_funptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL,
				/* .tp_writedec    = */ (Dee_funptr_t)&DeeNone_OperatorWriteDec
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ &default__seq_operator_assign,
		/* .tp_move_assign = */ &default__move_assign__with__assign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ &default__seq_operator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ &default_seq_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &seq_math,
	/* .tp_cmp           = */ &seq_cmp,
	/* .tp_seq           = */ &seq_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ seq_methods,
	/* .tp_getsets       = */ seq_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ seq_class_methods,
	/* .tp_class_getsets = */ seq_class_getsets,
	/* .tp_class_members = */ seq_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ seq_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(seq_operators),
	/* .tp_mhcache       = */ &mh_cache_empty,
};

/* An empty instance of a generic sequence object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSeq_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSeq_Type' should be considered stub/empty,
 *       but obviously something like an empty tuple is also an empty sequence. */
PUBLIC DeeObject DeeSeq_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeSeq_Type)
};


PRIVATE ATTR_NOINLINE ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetSeqClass_uncached(DeeTypeObject const *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	iter = DeeTypeMRO_Init(&mro, self);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		unsigned int result = DeeType_GetSeqClass(iter);
		if (result != Dee_SEQCLASS_NONE)
			return result;
	}
	return Dee_SEQCLASS_NONE;
}

/* Sequence type classification
 * @return: * : One of `Dee_SEQCLASS_*' */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetSeqClass(DeeTypeObject const *__restrict self) {
	unsigned int result;
	result = (self->tp_features & Dee_TF_SEQCLASS_MASK) >> Dee_TF_SEQCLASS_SHFT;
	ASSERT(result < Dee_SEQCLASS_COUNT);
	if (result == Dee_SEQCLASS_UNKNOWN) {
		result = DeeType_GetSeqClass_uncached(self);
		ASSERT(result != Dee_SEQCLASS_UNKNOWN);
		ASSERT(result < Dee_SEQCLASS_COUNT);
		atomic_or(&((DeeTypeObject *)self)->tp_features,
		          (uint32_t)result << Dee_TF_SEQCLASS_SHFT);
	}
	return result;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Sum(DeeObject *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_sum, self);
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_Any(DeeObject *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_any, self);
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_All(DeeObject *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_all, self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Min(DeeObject *self) {
	return DeeObject_InvokeMethodHint(seq_min, self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Max(DeeObject *self) {
	return DeeObject_InvokeMethodHint(seq_max, self);
}

/* Unpack the given sequence `self' into `dst_length' items then stored within the `dst' vector.
 * This operator follows `DeeObject_Foreach()' semantics, in that unbound items are skipped.
 * @return: 0 : Success (`dst' now contains exactly `dst_length' references to [1..1] objects)
 * @return: -1: An error was thrown (`dst' may have been modified, but contains no references) */
PUBLIC WUNUSED ATTR_OUTS(3, 2) NONNULL((1)) int
(DCALL DeeSeq_Unpack)(DeeObject *__restrict self, size_t dst_length,
                      /*out*/ DREF DeeObject **__restrict dst) {
	return DeeObject_InvokeMethodHint(seq_unpack, self, dst_length, dst);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_C */
