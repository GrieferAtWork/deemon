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
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/kwlist.h"
#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
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

PRIVATE struct type_getset tpconst seq_class_getsets[] = {
	TYPE_GETTER(STR_Iterator, &seqtype_get_Iterator,
	            "->?DType\n"
	            "Returns the Iterator class used by instances of @this Sequence type\n"
	            "Should a sub-class implement its own Iterator, this attribute should be overwritten"),
	TYPE_GETTER("Frozen", &seq_Frozen_get,
	            "->?DType\n"
	            "Returns the type of Sequence returned by the #i:frozen property"),
	TYPE_GETSET_END
};


/* === General-purpose Sequence methods. === */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_empty_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":empty"))
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_front_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":front"))
		goto err;
	return DeeObject_InvokeMethodHint(seq_getfirst, self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_back_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":back"))
		goto err;
	return DeeObject_InvokeMethodHint(seq_getlast, self);
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_filter(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *pred_keep;
	if (DeeArg_Unpack(argc, argv, "o:filter", &pred_keep))
		goto err;
	return DeeSeq_Filter(self, pred_keep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_ubfilter(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *pred_keep;
	if (DeeArg_Unpack(argc, argv, "o:ubfilter", &pred_keep))
		goto err;
	return DeeSeq_FilterAsUnbound(self, pred_keep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_map(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *mapper;
	if (DeeArg_Unpack(argc, argv, "o:map", &mapper))
		goto err;
	return DeeSeq_Map(self, mapper);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_segments(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t segsize;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":segments", &segsize))
		goto err;
	if unlikely(!segsize)
		goto err_invalid_segsize;
	return DeeSeq_Segments(self, segsize);
err_invalid_segsize:
	err_invalid_segment_size(segsize);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distribute(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t segsize, mylen;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":distribute", &segsize))
		goto err;
	if unlikely(!segsize)
		goto err_invalid_segsize;
	mylen = DeeObject_InvokeMethodHint(seq_operator_size, self);
	if unlikely(mylen == (size_t)-1)
		goto err;
	mylen += segsize - 1;
	mylen /= segsize;
	if unlikely(!mylen)
		return_empty_seq;
	return DeeSeq_Segments(self, mylen);
err_invalid_segsize:
	err_invalid_distribution_count(segsize);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_combinations(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t r;
	bool cached = true;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__r_cached, UNPuSIZ "|b:combinations", &r, &cached))
		goto err;
	if (cached) {
		self = DeeObject_InvokeMethodHint(seq_cached, self);
		if unlikely(!self)
			goto err;
	} else {
		Dee_Incref(self);
	}
	return DeeSeq_Combinations(self, r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_repeatcombinations(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t r;
	bool cached = true;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__r_cached, UNPuSIZ "|b:repeatcombinations", &r, &cached))
		goto err;
	if (cached) {
		self = DeeObject_InvokeMethodHint(seq_cached, self);
		if unlikely(!self)
			goto err;
	} else {
		Dee_Incref(self);
	}
	return DeeSeq_RepeatCombinations(self, r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_permutations(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t r = (size_t)-1;
	bool cached = true;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__r_cached, "|" UNPuSIZ "b:permutations", &r, &cached))
		goto err;
	if (cached) {
		self = DeeObject_InvokeMethodHint(seq_cached, self);
		if unlikely(!self)
			goto err;
	} else {
		Dee_Incref(self);
	}
	return DeeSeq_Permutations2(self, r);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_index(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:index",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeObject_InvokeMethodHint(seq_find_with_key, self, item, start, end, key)
	         : DeeObject_InvokeMethodHint(seq_find, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_no_item;
	return DeeInt_NewSize(result);
err_no_item:
	err_item_not_found(self, item);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_rindex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rindex",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeObject_InvokeMethodHint(seq_rfind_with_key, self, item, start, end, key)
	         : DeeObject_InvokeMethodHint(seq_rfind, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_no_item;
	return DeeInt_NewSize(result);
err_no_item:
	err_item_not_found(self, item);
err:
	return NULL;
}


DOC_DEF(seq_byhash_doc,
        "(template:?O)->?DSequence\n"
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
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_byhash(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__template, "o:byhash", &template_))
		goto err;
	return DeeSeq_HashFilter(self, DeeObject_Hash(template_));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_unpack(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t size_or_minsize;
	DeeObject *maxsize_ob = NULL;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:unpack", &size_or_minsize, &maxsize_ob))
		goto err;
	if (maxsize_ob == NULL) {
#ifndef __OPTIMIZE_SIZE__
		if (DeeTuple_Check(self)) {
			size_t real_size = DeeTuple_SIZE(self);
			if unlikely(real_size != size_or_minsize) {
				err_invalid_unpack_size(self, size_or_minsize, real_size);
				goto err;
			}
			return_reference_(self);
		}
#endif /* !__OPTIMIZE_SIZE__ */
		result = DeeTuple_NewUninitialized(size_or_minsize);
		if unlikely(!result)
			goto err;
		if unlikely(DeeSeq_Unpack(self, size_or_minsize, DeeTuple_ELEM(result)))
			goto err_r;
	} else {
		size_t maxsize, realsize;
		if unlikely(DeeObject_AsSize(maxsize_ob, &maxsize))
			goto err;
#ifndef __OPTIMIZE_SIZE__
		if (DeeTuple_Check(self)) {
			size_t real_size = DeeTuple_SIZE(self);
			if unlikely(real_size < size_or_minsize || real_size > maxsize) {
				err_invalid_unpack_size_minmax(self, size_or_minsize, maxsize, real_size);
				goto err;
			}
			return_reference_(self);
		}
#endif /* !__OPTIMIZE_SIZE__ */
		result = DeeTuple_NewUninitialized(maxsize);
		if unlikely(!result)
			goto err;
		realsize = DeeObject_InvokeMethodHint(seq_unpack_ex, self,
		                                      size_or_minsize, maxsize,
		                                      DeeTuple_ELEM(result));
		if unlikely(realsize == (size_t)-1)
			goto err_r;
		result = DeeTuple_TruncateUninitialized(result, realsize);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

#ifdef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE
#else /* CONFIG_NO_DEEMON_100_COMPAT */
INTERN /* Needed for alias `List.unique' */
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distinct(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DistinctSetWithKey *result;
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__key, "|o:distinct", &key))
		goto err;
	if (!key)
		return DeeSuper_New(&DeeSet_Type, self);
	result = DeeObject_MALLOC(DistinctSetWithKey);
	if unlikely(!result)
		goto err;
	result->dswk_key = key;
	Dee_Incref(key);
	result->dswk_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DistinctSetWithKey_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bcontains(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bcontains",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeObject_InvokeMethodHint(seq_bfind_with_key, self, item, start, end, key)
	         : DeeObject_InvokeMethodHint(seq_bfind, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return_bool_(result != (size_t)-1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bindex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bindex",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeObject_InvokeMethodHint(seq_bfind_with_key, self, item, start, end, key)
	         : DeeObject_InvokeMethodHint(seq_bfind, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_not_found;
	return DeeInt_NewSize(result);
err_not_found:
	err_item_not_found(self, item);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_blocateall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, result_range[2];
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:blocateall",
	                    &item, &start, &end, &key))
		goto err;
	if (!DeeNone_Check(key)
	    ? DeeObject_InvokeMethodHint(seq_brange_with_key, self, item, start, end, key, result_range)
	    : DeeObject_InvokeMethodHint(seq_brange, self, item, start, end, result_range))
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
	DeeObject *item;
	size_t index, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end,
	                    "o|" UNPuSIZ UNPuSIZ "o:binsert",
	                    &item, &start, &end))
		goto err;
	index = DeeObject_InvokeMethodHint(seq_bposition, self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(DeeObject_InvokeMethodHint(seq_insert, self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because aliased by `List.pop_front' */
seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return DeeObject_InvokeMethodHint(seq_pop, self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because aliased by `List.pop_back' */
seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
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
	Dee_Decref(result);
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
	TYPE_KWMETHOD(DeeMA_Sequence_reduce_name, &DeeMA_Sequence_reduce,
	              "(combine:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,init?)->\n"
	              DOC_throws_ValueError_if_empty
	              "Combines consecutive elements of @this Sequence by passing them as pairs of 2 to @combine, "
	              /**/ "then re-using its return value in the next invocation, before finally returning its last "
	              /**/ "return value. If the Sequence consists of only 1 element, @combine is never invoked.\n"
	              "When given, @init is used as the initial lhs-operand, rather than the first element of the Sequence\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function reduce}¹|${this.reduce(combine)}&"
	              /**/ "${function reduce}²|${"
	              /**/ /**/ "return start != 0 || end != int.SIZE_MAX ? this.reduce(combine, start, end)\n"
	              /**/ /**/ "                                         : this.reduce(combine);"
	              /**/ "}&"
	              /**/ "?#enumerate³|${"
	              /**/ /**/ "local result = init is bound ? Cell(init) : Cell();\n"
	              /**/ /**/ "Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value is bound)\n"
	              /**/ /**/ "		result.value = result.value is bound ? combine(result.value, value) : value;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "if (result.value !is bound)\n"
	              /**/ /**/ "	throw ValueError(...);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?.|"
	              /**/ "{³}Only when ?#enumerate has a valid implementation for the given arguments"
	              "}"),
	TYPE_KWMETHOD("enumerate", &seq_enumerate,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?S?T2?Dint?O\n"
	              "(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N\n"
	              "Enumerate indices/keys and associated values of @this sequence\n"
	              "This function can be used to easily enumerate sequence indices and values, "
	              /**/ "including being able to enumerate indices/keys that are currently unbound\n"
	              "${"
	              /**/ "import FixedList from collections;\n"
	              /**/ "local x = FixedList(4);\n"
	              /**/ "x[1] = 10;\n"
	              /**/ "x[3] = 20;\n"
	              /**/ "/* [1]: 10                 [3]: 20 */\n"
	              /**/ "for (local key, value: x.enumerate())\n"
	              /**/ "	print f\"[{repr key}]: {repr value}\"\n"
	              /**/ "/* [0]: <unbound>          [1]: 10\n"
	              /**/ " * [2]: <unbound>          [3]: 20 */\n"
	              /**/ "x.enumerate((key, value?) -\\> {\n"
	              /**/ "	print f\"[{repr key}]: {value is bound ? repr value : \"<unbound>\"}\";\n"
	              /**/ "});"
	              "}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${property iterkeys}¹³|${"
	              /**/ /**/ "foreach (local key: Mapping.iterkeys(this)) {\n"
	              /**/ /**/ "	local myItem\n"
	              /**/ /**/ "	local status;\n"
	              /**/ /**/ "	if (!(start <= key) || !(end > key))\n"
	              /**/ /**/ "		continue; // Only when given\n"
	              /**/ /**/ "	try {\n"
	              /**/ /**/ "		myItem = this[index];\n"
	              /**/ /**/ "	} catch (UnboundItem | KeyError | IndexError) {\n"
	              /**/ /**/ "		goto invokeWithUnbound;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	status = cb(key, myItem);\n"
	              /**/ /**/ "	goto handleStatus;\n"
	              /**/ /**/ "invokeWithUnbound:\n"
	              /**/ /**/ "	status = cb(key);\n"
	              /**/ /**/ "handleStatus:\n"
	              /**/ /**/ "	if (status !is none)\n"
	              /**/ /**/ "		return status;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return none;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}¹²|${"
	              /**/ /**/ "local realSize = ##this;\n"
	              /**/ /**/ "if (end > realSize)\n"
	              /**/ /**/ "	end = realSize;\n"
	              /**/ /**/ "if (start > end)\n"
	              /**/ /**/ "	start = end;\n"
	              /**/ /**/ "for (local i: [start:end]) {\n"
	              /**/ /**/ "	local myItem\n"
	              /**/ /**/ "	local status;\n"
	              /**/ /**/ "	try {\n"
	              /**/ /**/ "		myItem = this[index];\n"
	              /**/ /**/ "	} catch (UnboundItem) {\n"
	              /**/ /**/ "		goto invokeWithUnbound;\n"
	              /**/ /**/ "	} catch (IndexError) {\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	status = cb(i, myItem);\n"
	              /**/ /**/ "	goto handleStatus;\n"
	              /**/ /**/ "invokeWithUnbound:\n"
	              /**/ /**/ "	status = cb(i);\n"
	              /**/ /**/ "handleStatus:\n"
	              /**/ /**/ "	if (status !is none)\n"
	              /**/ /**/ "		return status;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return none;"
	              /**/ "}&"
	              /**/ "${operator iter}¹²|${"
	              /**/ /**/ "local it = this.operator iter();\n"
	              /**/ /**/ "for (none: [:start]) {\n"
	              /**/ /**/ "	foreach (none: it)\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "for (local i: [start:end]) {\n"
	              /**/ /**/ "	foreach (local v: it) {\n"
	              /**/ /**/ "		local status = cb(i, v);\n"
	              /**/ /**/ "		if (status !is none)\n"
	              /**/ /**/ "			return status;\n"
	              /**/ /**/ "		goto next_i;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	break;\n"
	              /**/ /**/ "next_i:;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return none;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}When @cb isn't given, filter for bound items and yield as ${(key, value)} pairs|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?.|"
	              /**/ "{³}Only when ?A__seqclass__?DType is ?DMapping"
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_sum_name, &DeeMA_Sequence_sum,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N\n"
	              "Returns the sum of all elements, or ?N if the Sequence is empty\n"
	              "This, alongside ?Ajoin?Dstring is the preferred way of merging lists "
	              /**/ "of strings into a single string\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function sum}|${"
	              /**/ /**/ "return start != 0 || end != int.SIZE_MAX ? this.sum(start, end)\n"
	              /**/ /**/ "                                         : this.sum();"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "for (local x: this)\n"
	              /**/ /**/ "	result = result is bound ? result + x : x;\n"
	              /**/ /**/ "if (result !is bound)\n"
	              /**/ /**/ "	result = none;\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value is bound)\n"
	              /**/ /**/ "		result.value = result.value is bound ? result.value + value : value;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "if (result.value !is bound)\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_any_name, &DeeMA_Sequence_any,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              "Returns ?t if any element of @this Sequence evaluates to ?t\n"
	              "If @this Sequence is empty, ?f is returned\n"
	              "This function has the same effect as ${this || ...}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function any}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.any(start, end)\n"
	              /**/ /**/ "	                                         : this.any();\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.any(start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.any(0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.any(key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "for (local x: this) {\n"
	              /**/ /**/ "	if ((key ?? (v -\\> v))(x))\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return false;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "return Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	return (value is bound && (key ?? (v -\\> v))(value)) ? true : none;\n"
	              /**/ /**/ "}, start, end) ?? false;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_all_name, &DeeMA_Sequence_all,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_key
	              "Returns ?t if all elements of @this Sequence evaluate to ?t\n"
	              "If @this Sequence is empty, ?t is returned\n"
	              "This function has the same effect as ${this && ...}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function all}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.all(start, end)\n"
	              /**/ /**/ "	                                         : this.all();\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.all(start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.all(0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.all(key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "for (local x: this) {\n"
	              /**/ /**/ "	if (!(key ?? (v -\\> v))(x))\n"
	              /**/ /**/ "		return false;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return true;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "return Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	return (value is bound && !(key ?? (v -\\> v))(value)) ? false : none;\n"
	              /**/ /**/ "}, start, end) ?? true;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_parity_name, &DeeMA_Sequence_parity,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_key
	              "Returns ?t or ?f indicative of the parity of Sequence elements that are ?t\n"
	              "If @this Sequence is empty, ?f is returned\n"
	              "Parity here refers to ${##this.filter(x -\\> !!x) % 2}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function parity}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.parity(start, end)\n"
	              /**/ /**/ "	                                         : this.parity();\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.parity(start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.parity(0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.parity(key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "local result = false;\n"
	              /**/ /**/ "for (local x: this) {\n"
	              /**/ /**/ "	if ((key ?? (v -\\> v))(x))\n"
	              /**/ /**/ "		result = !result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "local result = Cell(false);\n"
	              /**/ /**/ "return Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value is bound && (key ?? (v -\\> v))(value))\n"
	              /**/ /**/ "		result.value = !result.value;\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_min_name, &DeeMA_Sequence_min,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N\n"
	              DOC_param_key
	              "Returns the smallest element of @this Sequence. If @this Sequence is empty, ?N is returned.\n"
	              "When no @key is given, this function has the same effect as ${this < ...}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function min}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.min(start, end)\n"
	              /**/ /**/ "	                                         : this.min();\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.min(start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.min(0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.min(key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "local keyedResult;\n"
	              /**/ /**/ "for (local item: this) {\n"
	              /**/ /**/ "	if (result !is bound) {\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "	} else {\n"
	              /**/ /**/ "		if (keyedResult !is bound)\n"
	              /**/ /**/ "			keyedResult = key(result);\n"
	              /**/ /**/ "		local keyedItem = key(item);\n"
	              /**/ /**/ "		if (!(keyedResult < keyedItem)) {\n"
	              /**/ /**/ "			result = item;\n"
	              /**/ /**/ "			keyedResult = keyedItem;\n"
	              /**/ /**/ "		}\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (result !is bound)\n"
	              /**/ /**/ "	result = none;\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "return Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value !is bound)\n"
	              /**/ /**/ "		return;\n"
	              /**/ /**/ "	if (result.value !is bound) {\n"
	              /**/ /**/ "		result.value = (value,);\n"
	              /**/ /**/ "	} else {\n"
	              /**/ /**/ "		local current = result.value;\n"
	              /**/ /**/ "		if (#current == 1)\n"
	              /**/ /**/ "			current = (current.first, key(current.first));\n"
	              /**/ /**/ "		local keyedValue = key(value);\n"
	              /**/ /**/ "		if (!(current.last < keyedValue))\n"
	              /**/ /**/ "			result.value = (value, keyedValue);\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "if (result.value !is bound)\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "return result.value.first;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_max_name, &DeeMA_Sequence_max,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N\n"
	              DOC_param_key
	              "Returns the greatest element of @this Sequence. If @this Sequence is empty, ?N is returned.\n"
	              "When no @key is given, this function has the same effect as ${this > ...}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function max}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.max(start, end)\n"
	              /**/ /**/ "	                                         : this.max();\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.max(start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.max(0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.max(key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "local keyedResult;\n"
	              /**/ /**/ "for (local item: this) {\n"
	              /**/ /**/ "	if (result !is bound) {\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "	} else {\n"
	              /**/ /**/ "		if (keyedResult !is bound)\n"
	              /**/ /**/ "			keyedResult = key(result);\n"
	              /**/ /**/ "		local keyedItem = key(item);\n"
	              /**/ /**/ "		if (keyedResult < keyedItem) {\n"
	              /**/ /**/ "			result = item;\n"
	              /**/ /**/ "			keyedResult = keyedItem;\n"
	              /**/ /**/ "		}\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (result !is bound)\n"
	              /**/ /**/ "	result = none;\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "return Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value !is bound)\n"
	              /**/ /**/ "		return;\n"
	              /**/ /**/ "	if (result.value !is bound) {\n"
	              /**/ /**/ "		result.value = (value,);\n"
	              /**/ /**/ "	} else {\n"
	              /**/ /**/ "		local current = result.value;\n"
	              /**/ /**/ "		if (#current == 1)\n"
	              /**/ /**/ "			current = (current.first, key(current.first));\n"
	              /**/ /**/ "		local keyedValue = key(value);\n"
	              /**/ /**/ "		if (current.last < keyedValue)\n"
	              /**/ /**/ "			result.value = (value, keyedValue);\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "if (result.value !is bound)\n"
	              /**/ /**/ "	return none;\n"
	              /**/ /**/ "return result.value.first;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_count_name, &DeeMA_Sequence_count,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns the number of instances of a given object @item in @this Sequence\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function count}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.count(item, start, end)\n"
	              /**/ /**/ "	                                         : this.count(item);\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.count(item, start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.count(item, 0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.count(item, key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result = 0;\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "for (local item: this) {\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(item)))\n"
	              /**/ /**/ "		++result;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result = Cell(0);\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "return Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value is bound) {\n"
	              /**/ /**/ "		if (deemon.equals(keyedItem, key(value)))\n"
	              /**/ /**/ "			++result.value;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_locate_name, &DeeMA_Sequence_locate,
	              "(match:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]\n"
	              "Locate and return the first element such that ${match(elem)} "
	              /**/ "is true, or @def when no such element exists\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function locate}²|${this.locate(item, start, end, def)}&"
	              /**/ "${function locate}¹³|${this.locate(item, def)}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "for (local item: this) {\n"
	              /**/ /**/ "	if (match(item))\n"
	              /**/ /**/ "		return item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return def;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "local result = Cell(def);\n"
	              /**/ /**/ "local ok = Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value is bound && match(value)) {\n"
	              /**/ /**/ "		result.value = value;\n"
	              /**/ /**/ "		return true;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "return result.value;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?.|"
	              /**/ "{³}Only when ?A__seqclass__?DType is ?DSet or ?DMapping"
	              "}"),
	TYPE_KWMETHOD(DeeMA_Sequence_rlocate_name, &DeeMA_Sequence_rlocate,
	              "(match:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]\n"
	              "Locate and return the last element such that ${match(elem)} "
	              /**/ "is true, or @def when no such element exists\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function locate}²|${this.locate(item, start, end, def)}&"
	              /**/ "${function locate}¹³|${this.locate(item, def)}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "local result = def;\n"
	              /**/ /**/ "for (local item: this) {\n"
	              /**/ /**/ "	if (match(item))\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "local realSize = ##this;\n"
	              /**/ /**/ "if (end > realSize)\n"
	              /**/ /**/ "	end = realSize;\n"
	              /**/ /**/ "if (start > end)\n"
	              /**/ /**/ "	start = end;\n"
	              /**/ /**/ "local result = def;\n"
	              /**/ /**/ "for (local i: [start: end]) {\n"
	              /**/ /**/ "	local item;\n"
	              /**/ /**/ "	try {\n"
	              /**/ /**/ "		item = this[index];\n"
	              /**/ /**/ "	} catch (UnboundItem) {\n"
	              /**/ /**/ "		continue;\n"
	              /**/ /**/ "	} catch (IndexError) {\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	if (match(item))\n"
	              /**/ /**/ "		result = item;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return result;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?.|"
	              /**/ "{³}Only when ?A__seqclass__?DType is ?DSet or ?DMapping"
	              "}"),

	TYPE_METHOD("filter", &seq_filter,
	            "(keep:?DCallable)->?DSequence\n"
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
	            "(keep:?DCallable)->?DSequence\n"
	            "#pkeep{A key function which is called for each element of @this Sequence}"
	            "Returns a sub-Sequence of all elements for which ${keep(item)} evaluates to ?t. "
	            /**/ "Same as ?#filter, but the returned sequence has the same size as @this, and "
	            /**/ "filtered elements are simply treated as though they were unbound:\n"
	            "${"
	            /**/ "assert { 10, 20 }.ubfilter(x -\\> x > 10)[0] !is bound;\n"
	            /**/ "assert { 10, 20 }.ubfilter(x -\\> x > 10)[1] is bound;"
	            "}"),
	TYPE_METHOD("map", &seq_map,
	            "(mapper:?DCallable)->?DSequence\n"
	            "#pmapper{A key function invoked to map members of @this Sequence}"
	            "Returns a Sequence that is a transformation of @this, with each element passed "
	            /**/ "to @mapper for processing before being returned\n"
	            "${"
	            /**/ "function map(mapper: Callable): Sequence {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		yield mapper(x);\n"
	            /**/ "}"
	            "}"),

	TYPE_KWMETHOD(DeeMA_Sequence_contains_name, &DeeMA_Sequence_contains,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t if @this Sequence contains an element matching @item"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_startswith_name, &DeeMA_Sequence_startswith,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t / ?f indicative of @this Sequence's first element matching :item\n"
	              "The implementation of this is derived from #first, where the found is then compared "
	              /**/ "against @item, potentially through use of @{key}: ${key(first) == key(item)} or ${first == item}, "
	              /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_endswith_name, &DeeMA_Sequence_endswith,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t / ?f indicative of @this Sequence's last element matching :item\n"
	              "The implementation of this is derived from #last, where the found is then compared "
	              /**/ "against @item, potentially through use of @{key}: ${key(last) == key(item)} or ${last == item}, "
	              /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_find_name, &DeeMA_Sequence_find,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Search for the first element matching @item and return its index. "
	              /**/ "If no such element exists, return ${-1} instead"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_rfind_name, &DeeMA_Sequence_rfind,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Search for the last element matching @item and return its index. "
	              /**/ "If no such element exists, return ${-1} instead"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_index, &seq_index,
	              "(item,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              DOC_throws_ValueError_if_not_found
	              "Search for the first element matching @item and return its index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_rindex, &seq_rindex,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              DOC_throws_ValueError_if_not_found
	              "Search for the last element matching @item and return its index"
	              ""), /* TODO: Requirements|Implementation table */

	TYPE_METHOD("segments",
	            &seq_segments,
	            "(segmentSize:?Dint)->?S?DSequence\n"
	            "#tIntegerOverflow{@segmentSize is negative, or too large}"
	            "#tValueError{The given @segmentSize is zero}"
	            "Return a Sequence of sequences contains all elements from @this Sequence, "
	            /**/ "with the first n sequences all consisting of @segmentSize elements, before "
	            /**/ "the last one contains the remainder of up to @segmentSize elements"),
	TYPE_METHOD("distribute",
	            &seq_distribute,
	            "(bucketCount:?Dint)->?S?DSequence\n"
	            "#tIntegerOverflow{@bucketCount is negative, or too large}"
	            "#tValueError{The given @bucketCount is zero}"
	            "Re-distribute the elements of @this Sequence to form @bucketCount similarly-sized "
	            /**/ "buckets of objects, with the last bucket containing the remaining elements, "
	            /**/ "making its length a little bit shorter than the other buckets\n"
	            "This is similar to #segments, however rather than having the caller specify the "
	            /**/ "size of the a bucket, the number of buckets is specified instead."),
	TYPE_KWMETHOD("combinations",
	              &seq_combinations,
	              "(r:?Dint,cached=!t)->?S?DSequence\n"
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
	              "(r:?Dint,cached=!t)->?S?DSequence\n"
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
	              "(r?:?Dint,cached=!t)->?S?DSequence\n"
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

	TYPE_METHOD("unpack", &seq_unpack,
	            "(size:?Dint)->?S?O\n"
	            "(minsize:?Dint,maxsize:?Dint)->?S?O\n"
	            "Unpack elements of this sequence (skipping over unbound items), whilst asserting "
	            /**/ "that the resulting sequence's size is either equal to @size, or lies within "
	            /**/ "the (inclusive) bounds ${[minsize, maxsize]}"),

	TYPE_KWMETHOD("distinct", &seq_distinct,
	              "(key?:?DCallable)->?DSet\n"
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
	 * Similar to ?#distinct (and actually identical when @this is stored),
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
	 * Yield every elements of @this (except for the first) sequence as a pair (predecessor, elem):
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

	/* TODO: unboundas(item)->?S?O
	 * Return a view of @this sequence that replaces every instance of "item" with
	 * unbound elements. Same as:
	 * >> this.ubfilter(e -> !deemon.equals(item, e));
	 */


	/* TODO: findall: "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?S?Dint"
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

	/* TODO: countseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = SIZE_MAX, key: Callable = none): int */

	/* TODO: partition(item: Object, start: int = 0, end: int = SIZE_MAX, key: Callable = none): (Sequence, item, Sequence) */
	/* TODO: partitionseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = SIZE_MAX, key: Callable = none): (Sequence, subseq, Sequence) */

	/* TODO: rpartition(item: Object, start: int = 0, end: int = SIZE_MAX, key: Callable = none): (Sequence, item, Sequence) */
	/* TODO: rpartitionseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = SIZE_MAX, key: Callable = none): (Sequence, subseq, Sequence) */

	/* TODO: startswithseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = SIZE_MAX, key: Callable = none): bool */

	/* TODO: endswithseq(subseq: Sequence | rt.SeqSome, start: int = 0, end: int = SIZE_MAX, key: Callable = none): bool */

	/* Functions for mutable sequences. */
	TYPE_KWMETHOD(DeeMA_Sequence_reversed_name, &DeeMA_Sequence_reversed,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?DSequence\n"
	              "Return a Sequence that contains the elements of @this Sequence in reverse order\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_sorted_name, &DeeMA_Sequence_sorted,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?DSequence\n"
	              "Return a Sequence that contains all elements from @this Sequence, "
	              /**/ "but sorted in ascending order, or in accordance to @key\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_insert_name, &DeeMA_Sequence_insert,
	              "(index:?Dint,item)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Insert the given @item under @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_insertall_name, &DeeMA_Sequence_insertall,
	              "(index:?Dint,items:?DSequence)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Insert all elements from @items at @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_append_name, &DeeMA_Sequence_append,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Append the given @item at the end of @this Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_extend_name, &DeeMA_Sequence_extend,
	            "(items:?DSequence)\n"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Append all elements from @items at the end of @this Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_erase_name, &DeeMA_Sequence_erase,
	              "(index:?Dint,count=!1)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Erase up to @count elements starting at @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_xchitem_name, &DeeMA_Sequence_xchitem,
	              "(index:?Dint,value)->\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Exchange the @index'th element of @this Sequence with the given "
	              /**/ "@value, returning the old element found under that index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_pop_name, &DeeMA_Sequence_pop,
	              "(index=!-1)->\n"
	              "#tIntegerOverflow{The given @index is too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Pop the @index'th element of @this Sequence and return it. When @index is lower "
	              /**/ "than $0, add ${##this} prior to index selection"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(STR_popfront, &seq_popfront,
	            "->\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ${this.pop(0)}"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(STR_popback, &seq_popback,
	            "->\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ${this.pop(-1)}"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_pushfront_name, &DeeMA_Sequence_pushfront,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ?#insert at position $0"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_pushback_name, &DeeMA_Sequence_pushback,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Alias for ?#append"),
	TYPE_KWMETHOD(DeeMA_Sequence_remove_name, &DeeMA_Sequence_remove,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find the first instance of @item and remove it, returning ?t if an "
	              /**/ "element got removed, or ?f if @item could not be found"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_rremove_name, &DeeMA_Sequence_rremove,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find the last instance of @item and remove it, returning ?t if an "
	              /**/ "element got removed, or ?f if @item could not be found"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_removeall_name, &DeeMA_Sequence_removeall,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find all instance of @item and remove them, returning the number of "
	              /**/ "instances found (and consequently removed)"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_removeif_name, &DeeMA_Sequence_removeif,
	              "(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Remove all elements within the given sub-range, for which ${should(item)} "
	              /**/ "evaluates to ?t, and return the number of elements found (and consequently removed)"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(DeeMA_Sequence_clear_name, &DeeMA_Sequence_clear,
	            "()\n"
	            "#tSequenceError{@this Sequence is immutable}"
	            "Clear all elements from the Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_resize_name, &DeeMA_Sequence_resize,
	              "(size:?Dint,filler=!N)\n"
	              "#tSequenceError{@this Sequence isn't resizable}"
	              "Resize @this Sequence to have a new length of @size "
	              /**/ "items, using @filler to initialize newly added entries"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_fill_name, &DeeMA_Sequence_fill,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,filler=!N)\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "Assign @filler to all elements within the given sub-range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_reverse_name, &DeeMA_Sequence_reverse,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "Reverse the order of all elements within the given range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_sort_name, &DeeMA_Sequence_sort,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Sort the elements within the given range"
	              ""), /* TODO: Requirements|Implementation table */

	/* Binary search API */
	TYPE_KWMETHOD(DeeMA_Sequence_bfind_name, &DeeMA_Sequence_bfind,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?N\n"
	              DOC_param_item
	              DOC_param_key
	              "Do a binary search (requiring @this to be sorted via @key) for @item\n"
	              "In case multiple elements match @item, the returned index will be "
	              /**/ "that for one of them, though it is undefined which one specifically.\n"
	              "When no elements of @this match, ?N is returned."
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD("bcontains", &seq_bcontains,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Wrapper around ?#bfind that simply returns ${this.bfind(...) !is none}"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD("bindex", &seq_bindex,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "#tValueError{The Sequence does not contain an item matching @item}"
	              "Same as ?#bfind, but throw an :ValueError instead of returning ?N."
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_bposition_name, &DeeMA_Sequence_bposition,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Same as ?#bfind, but return (an) index where @item should be inserted, rather "
	              /**/ "than ?N when @this doesn't contain any matching object"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(DeeMA_Sequence_brange_name, &DeeMA_Sequence_brange,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?T2?Dint?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Similar to ?#bfind, but return a tuple ${[begin,end)} of integers representing "
	              /**/ "the lower and upper bound of indices for elements from @this matching @item.\n"
	              "NOTE: The returned tuple is allowed to be an ASP, meaning that its elements may "
	              /**/ "be calculated lazily, and are prone to change as the result of @this changing."
	              ""), /* TODO: Requirements|Implementation table */

	TYPE_KWMETHOD("blocateall", &seq_blocateall,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?S?O\n"
	              DOC_param_item
	              DOC_param_key
	              "Return the sub-range from @this of elements matching @item, as returned by ?#brange\n"
	              "${"
	              /**/ "function blocateall(args..., **kwds) {\n"
	              /**/ "	import Sequence from deemon;\n"
	              /**/ "	local begin, end = this.brange(args..., **kwds)...;\n"
	              /**/ "	return (this as Sequence)[begin:end];\n"
	              /**/ "}"
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
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)\n"
	              "Helper wrapper for ?#insert and ?#bposition that automatically determines "
	              /**/ "the index where a given @item should be inserted to ensure that @this sequence "
	              /**/ "remains sorted according to @key. Note that this function makes virtual calls as "
	              /**/ "seen in the following template, meaning it usually doesn't need to be overwritten "
	              /**/ "by sub-classes.\n"
	              "${"
	              /**/ "function binsert(item: Object, key: Callable = none) {\n"
	              /**/ "	local index = this.bposition(item, key);\n"
	              /**/ "	return this.insert(index, item);\n"
	              /**/ "}"
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
	TYPE_METHOD("__bool__", &DeeMA___seq_bool__,
	            "->?Dbool\n"
	            "Alias for ${!!(this as Sequence)}"),
	TYPE_METHOD("__iter__", &DeeMA___seq_iter__,
	            "->?DIterator\n"
	            "Alias for ${(this as Sequence).operator iter()}"),
	TYPE_METHOD("__size__", &DeeMA___seq_size__,
	            "->?Dint\n"
	            "Alias for ${##(this as Sequence)}"),
	TYPE_METHOD("__getitem__", &DeeMA___seq_getitem__,
	            "(index:?Dint)->\n"
	            "Alias for ${(this as Sequence)[index]}"),
	TYPE_METHOD("__delitem__", &DeeMA___seq_delitem__,
	            "(index:?Dint)\n"
	            "Alias for ${del (this as Sequence)[index]}"),
	TYPE_METHOD("__setitem__", &DeeMA___seq_setitem__,
	            "(index:?Dint,value)\n"
	            "Alias for ${(this as Sequence)[index] = value}"),
	TYPE_KWMETHOD("__getrange__", &DeeMA___seq_getrange__,
	              "(start=!0,end?:?X2?N?Dint)->?S?O\n"
	              "Alias for ${(this as Sequence)[start:end]}"),
	TYPE_KWMETHOD("__delrange__", &DeeMA___seq_delrange__,
	              "(start=!0,end?:?X2?N?Dint)\n"
	              "Alias for ${del (this as Sequence)[start:end]}"),
	TYPE_KWMETHOD("__setrange__", &DeeMA___seq_setrange__,
	              "(start=!0,end?:?X2?N?Dint,values:?S?O)\n"
	              "Alias for ${(this as Sequence)[start:end] = values}"),
	TYPE_METHOD("__enumerate__", &DeeMA___seq_enumerate__,
	            "(cb)->\n"
	            "Alias for ${(this as Sequence).enumerate(cb)}"),
	TYPE_METHOD("__hash__", &DeeMA___seq_hash__,
	            "->?Dint\n"
	            "Alias for ${(this as Sequence).operator hash()}"),
	TYPE_METHOD("__compare_eq__", &DeeMA___seq_compare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence).operator == (rhs)}"),
	TYPE_METHOD("__compare__", &DeeMA___seq_compare__,
	            "(rhs:?S?O)->?Dint\n"
	            "Alias for ${deemon.compare(this as Sequence, rhs)}"),
	TYPE_METHOD("__eq__", &DeeMA___seq_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) == (rhs)}"),
	TYPE_METHOD("__ne__", &DeeMA___seq_ne__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) != (rhs)}"),
	TYPE_METHOD("__lo__", &DeeMA___seq_lo__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) < (rhs)}"),
	TYPE_METHOD("__le__", &DeeMA___seq_le__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) <= (rhs)}"),
	TYPE_METHOD("__gr__", &DeeMA___seq_gr__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) > (rhs)}"),
	TYPE_METHOD("__ge__", &DeeMA___seq_ge__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) >= (rhs)}"),
	TYPE_METHOD("__inplace_add__", &DeeMA___seq_inplace_add__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Sequence) += rhs}"),
	TYPE_METHOD("__inplace_mul__", &DeeMA___seq_inplace_mul__,
	            "(factor:?Dint)->?.\n"
	            "Alias for ${(this as Sequence) *= factor}"),

	/* Old function names/deprecated functions. */
	TYPE_METHOD("transform", &seq_map,
	            "(mapper:?DCallable)->?DSequence\n"
	            "Deprecated alias for ?#map"),
	TYPE_KWMETHOD("xch", &DeeMA_Sequence_xchitem,
	              "(index:?Dint,value)->\n"
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
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isnonempty(DeeObject *__restrict self) {
	int result = DeeObject_InvokeMethodHint(seq_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}


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
	                  "Depending on the nearest implemented group of operators, "
	                  /**/ "one of the following implementations is chosen\n"
	                  "For ${operator []}:\n"
	                  "${"
	                  /**/ "property first: Object = {\n"
	                  /**/ "	get(): Object {\n"
	                  /**/ "		import Error from deemon;\n"
	                  /**/ "		try {\n"
	                  /**/ "			return this[0];\n"
	                  /**/ "		} catch (Error.ValueError.IndexError) {\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		}\n"
	                  /**/ "	}\n"
	                  /**/ "	del() {\n"
	                  /**/ "		try {\n"
	                  /**/ "			del this[0]; // If `operator delitem' doesn't exist, throw NotImplemented\n"
	                  /**/ "		} catch (Error.ValueError.IndexError) {\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		}\n"
	                  /**/ "	}\n"
	                  /**/ "	set(value: Object) {\n"
	                  /**/ "		try {\n"
	                  /**/ "			this[0] = value; /* If `operator setitem' doesn't exist, throw NotImplemented */\n"
	                  /**/ "		} catch (Error.ValueError.IndexError) {\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		}\n"
	                  /**/ "	}\n"
	                  /**/ "}"
	                  "}\n"
	                  "For ${operator iter}:\n"
	                  "${"
	                  /**/ "property first: Object = {\n"
	                  /**/ "	get(): Object {\n"
	                  /**/ "		import Error, Signal from deemon;\n"
	                  /**/ "		local it = this.operator iter();\n"
	                  /**/ "		try {\n"
	                  /**/ "			return it.operator next();\n"
	                  /**/ "		} catch (Signal.StopIteration) {\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		}\n"
	                  /**/ "	}\n"
	                  /**/ "}"
	                  "}"),
	TYPE_GETSET_BOUND(STR_last,
	                  &default__seq_getlast,
	                  &default__seq_dellast,
	                  &default__seq_setlast,
	                  &default__seq_boundlast,
	                  "->\n"
	                  "Access the last item of the Sequence\n"
	                  "Depending on the nearest implemented group of operators, "
	                  /**/ "one of the following implementations is chosen\n"
	                  "For ${operator []} and ${operator ##}:\n"
	                  "${"
	                  /**/ "property last: Object = {\n"
	                  /**/ "	get(): Object {\n"
	                  /**/ "		import Error from deemon;\n"
	                  /**/ "		local mylen = ##this;\n"
	                  /**/ "		if (!mylen)\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		return this[mylen - 1];\n"
	                  /**/ "	}\n"
	                  /**/ "	del() {\n"
	                  /**/ "		import Error from deemon;\n"
	                  /**/ "		local mylen = ##this;\n"
	                  /**/ "		if (!mylen)\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		del this[mylen - 1];\n"
	                  /**/ "	}\n"
	                  /**/ "	set(value: Object) {\n"
	                  /**/ "		import Error from deemon;\n"
	                  /**/ "		local mylen = ##this;\n"
	                  /**/ "		if (!mylen)\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		this[mylen - 1] = value;\n"
	                  /**/ "	}\n"
	                  /**/ "}}\n"
	                  /**/ "For ${operator iter}:\n"
	                  /**/ "${"
	                  /**/ "property last: Object = {\n"
	                  /**/ "	get(): Object {\n"
	                  /**/ "		import Error, Signal from deemon;\n"
	                  /**/ "		local it = this.operator iter();\n"
	                  /**/ "		local result;\n"
	                  /**/ "		try {\n"
	                  /**/ "			for (;;)\n"
	                  /**/ "				result = it.operator next();\n"
	                  /**/ "		} catch (Signal.StopIteration) {\n"
	                  /**/ "		}\n"
	                  /**/ "		if (result !is bound)\n"
	                  /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	                  /**/ "		return result;\n"
	                  /**/ "	}\n"
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
	            "As such, ?#cached behaves similar to ?#frozen, except that the rather than "
	            /**/ "evaluating the elements of the sequence immediately, said evaluation will "
	            /**/ "happen the first time elements are accessed. Additionally, there is no "
	            /**/ "requirement that changes to the original sequence won't propagate into the "
	            /**/ "\"cached\" sequence (even after the cache may have been populated).\n"
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
	DeeObject *start, *end = NULL, *step = NULL, *result;
	if (DeeArg_Unpack(argc, argv, "o|oo:range", &start, &end, &step))
		goto err;
	if (end)
		return DeeRange_New(start, end, step);
	/* Use a default-constructed instance of `type(start)' for the real start. */
	end = DeeObject_NewDefault(Dee_TYPE(start));
	if unlikely(!end)
		goto err;
	result = DeeRange_New(end, start, step);
	Dee_Decref(end);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_repeat(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DeeObject *obj;
	size_t count;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":repeat", &obj, &count))
		goto err;
	return DeeSeq_RepeatItem(obj, count);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
seq_class_repeatseq(DeeObject *__restrict UNUSED(self),
                    size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	size_t count;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":repeatseq", &seq, &count))
		goto err;
	return DeeSeq_Repeat(seq, count);
err:
	return NULL;
}

INTDEF DeeTypeObject SeqConcat_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_concat(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	if (!argc)
		return_empty_seq;
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
	TYPE_METHOD("repeat", &seq_class_repeat,
	            /* TODO: Rename to "repeatitem" */
	            /* TODO: The "count" argument should be optional, and if not given, means "infinite" */
	            "(obj,count:?Dint)->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Create a proxy-Sequence that yields @obj a total of @count times\n"
	            "The main purpose of this function is to construct large sequences "
	            /**/ "to be used as initializers for mutable sequences such as ?DList"),
	TYPE_METHOD("repeatseq", &seq_class_repeatseq,
	            /* TODO: Rename to "repeat" (and change from "tp_class_methods" to "tp_methods") */
	            /* TODO: The "count" argument should be optional, and if not given, means "infinite" */
	            "(seq:?DSequence,count:?Dint)->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Repeat all the elements from @seq a total of @count times\n"
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
"When invoked directly, an empty, general-purpose Sequence is returned\n"
"\n"

"bool->\n"
"Returns ?t/?f indicative of @this Sequence being non-empty.\n"
"#T{Requirements|Implementation~"
/**/ "${operator bool}¹|${return !!this;}&"
/**/ "${operator size}¹²|${return !!##this;}&"
/**/ "${operator iter}²|${"
/**/ /**/ "for (none: this)\n"
/**/ /**/ "	return true;\n"
/**/ /**/ "return false;"
/**/ "}&"
/**/ "${operator ==}¹²|${return !(this == {});}&"
/**/ "${operator !=}¹²|${return this != {};}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

"repr->\n"
"Returns the representation of all Sequence elements, using "
/**/ "abstract Sequence syntax\n"
"e.g.: ${{ 10, 20, \"foo\" }}\n"
"${"
/**/ "operator repr(fp) {\n"
/**/ "	print fp: \"{ \",\n"
/**/ "	local isFirst = true;\n"
/**/ "	for (local item: (this as Sequence)) {\n"
/**/ "		if (!isFirst)\n"
/**/ "			print fp: \", \",\n"
/**/ "		isFirst = false;\n"
/**/ "		print fp: repr item,;\n"
/**/ "	}\n"
/**/ "}"
"}\n"
"\n"

"+(other:?DSequence)->\n"
"Returns a proxy Sequence for accessing elements from @this "
/**/ "Sequence, and those from @other in a seamless stream of items\n"
"This operator is implemented similar to the following, however the actual "
/**/ "return type may be a proxy Sequence that further optimizes the iteration "
/**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
/**/ "as how the sub-range is accessed (i.e. ${##(this + other)} will invoke ${(##this).operator int() + (#other).operator int()}).\n"
"${"
/**/ "operator + (other) {\n"
/**/ "	yield (this as Sequence)...;\n"
/**/ "	yield (other as Sequence)...;\n"
/**/ "}"
"}\n"
"\n"

"*(count:?Dint)->\n"
"#tIntegerOverflow{@count is negative, or larger than ?ASIZE_MAX?Dint}"
"Returns a proxy Sequence for accessing the elements of @this "
/**/ "Sequence, consecutively repeated a total of @count times\n"
"The implementation is allowed to assume that the number and value of "
/**/ "items of @this Sequence don't change between multiple iterations\n"
"This operator is implemented similar to the following, however the actual "
/**/ "return type may be a proxy Sequence that further optimizes the iteration "
/**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
/**/ "as how the sub-range is accessed (i.e. ${##(this * 4)} will invoke ${(##this).operator int() * 4}).\n"
"${"
/**/ "operator * (count) {\n"
/**/ "	import int, Error from deemon;\n"
/**/ "	count = count.operator int();\n"
/**/ "	if (count < 0)\n"
/**/ "		throw Error.ValueError.ArithmeticError.IntegerOverflow();\n"
/**/ "	while (count) {\n"
/**/ "		--count;\n"
/**/ "		yield (this as Sequence)...;\n"
/**/ "	}\n"
/**/ "}"
"}\n"
"\n"

"hash->\n"
"Returns the hash of all items of @this ?.\n"
"#T{Requirements|Implementation~"
/**/ "${operator hash}¹|${return this.operator hash();}&"
/**/ "${operator iter}²|${"
/**/ /**/ "local result = rt.HASHOF_EMPTY_SEQUENCE;\n"
/**/ /**/ "for (local x: this)\n"
/**/ /**/ "	result = deemon.hash(result, x);\n"
/**/ /**/ "return result;"
/**/ "}&"
/**/ "${operator size}, ${operator getitem}¹²|${"
/**/ /**/ "local size = ##this;\n"
/**/ /**/ "local result = rt.HASHOF_EMPTY_SEQUENCE;\n"
/**/ /**/ "for (local i: [:size]) {\n"
/**/ /**/ "	local item;\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		item = this[i];\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		item = rt.HASHOF_UNBOUND_ITEM;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	result = deemon.hash(result, item);\n"
/**/ /**/ "}\n"
/**/ /**/ "return result;"
/**/ "}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

"<->\n"
"<=->\n"
"==->\n"
"!=->\n"
">->\n"
">=->\n"
"Returns ?t/?f indicative of a lexicographical comparison between @this and @other."
"#T{Requirements|Implementation~"
/**/ "${operator <=>}¹²|${return this <=> other;}&"
/**/ "${operator iter}²³|${"
/**/ /**/ "local myIter = this.operator iter();\n"
/**/ /**/ "if (IMPL_OF_OPERATOR_ITER(other) == WITH_SIZE_AND_GETITEM) {\n"
/**/ /**/ "	local otSize = ##other;\n"
/**/ /**/ "	for (local otIndex: [:otSize]) {\n"
/**/ /**/ "		local myItem;\n"
/**/ /**/ "		foreach (myItem: myIter)\n"
/**/ /**/ "			goto hasMyItem;\n"
/**/ /**/ "		return LESS_THAN;\n"
/**/ /**/ "hasMyItem:\n"
/**/ /**/ "		local otItem;\n"
/**/ /**/ "		try {\n"
/**/ /**/ "			otItem = other[otIndex];\n"
/**/ /**/ "		} catch (UnboundItem) {\n"
/**/ /**/ "			return GREATER_THAN;\n"
/**/ /**/ "		} catch (IndexError) {\n"
/**/ /**/ "			return GREATER_THAN;\n"
/**/ /**/ "		}\n"
/**/ /**/ "		local cmp = myItem <=> otItem;\n"
/**/ /**/ "		if (cmp)\n"
/**/ /**/ "			return cmp;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	foreach (none: myIter)\n"
/**/ /**/ "		return GREATER_THAN;\n"
/**/ /**/ "} else {\n"
/**/ /**/ "	local otIter = this.operator iter();\n"
/**/ /**/ "	for (;;) {\n"
/**/ /**/ "		local myItem, otItem;\n"
/**/ /**/ "		foreach (myItem: myIter)\n"
/**/ /**/ "			goto hasMyItem2;\n"
/**/ /**/ "		break;\n"
/**/ /**/ "hasMyItem2:\n"
/**/ /**/ "		foreach (otItem: otIter)\n"
/**/ /**/ "			goto hasOtItem2;\n"
/**/ /**/ "		return GREATER_THAN;\n"
/**/ /**/ "hasOtItem2:\n"
/**/ /**/ "		local cmp = myItem <=> otItem;\n"
/**/ /**/ "		if (cmp)\n"
/**/ /**/ "			return cmp;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	foreach (none: otIter)\n"
/**/ /**/ "		return LESS_THAN;\n"
/**/ /**/ "}\n"
/**/ /**/ "return EQUAL;\n"
/**/ "}&"
/**/ "${operator size}, ${operator getitem}¹²³|${"
/**/ /**/ "local mySize = ##this;\n"
/**/ /**/ "if (IMPL_OF_OPERATOR_ITER(other) == WITH_SIZE_AND_GETITEM) {\n"
/**/ /**/ "	local otSize = ##other;\n"
/**/ /**/ "	local commonSize = {mySize, otSize} < ...;\n"
/**/ /**/ "	for (local index: [:commonSize]) {\n"
/**/ /**/ "		local myItem, otItem;\n"
/**/ /**/ "		try {\n"
/**/ /**/ "			myItem = this[index];\n"
/**/ /**/ "		} catch (UnboundItem) {\n"
/**/ /**/ "			try {\n"
/**/ /**/ "				other[index];\n"
/**/ /**/ "			} catch (UnboundItem) {\n"
/**/ /**/ "				continue;\n"
/**/ /**/ "			} catch (IndexError) {\n"
/**/ /**/ "				return GREATER_THAN;\n"
/**/ /**/ "			}\n"
/**/ /**/ "			return LESS_THAN;\n"
/**/ /**/ "		} catch (IndexError) {\n"
/**/ /**/ "			return LESS_THAN;\n"
/**/ /**/ "		}\n"
/**/ /**/ "		try {\n"
/**/ /**/ "			otItem = other[index];\n"
/**/ /**/ "		} catch (UnboundItem | IndexError) {\n"
/**/ /**/ "			return GREATER_THAN;\n"
/**/ /**/ "		}\n"
/**/ /**/ "		local cmp = myItem <=> otItem;\n"
/**/ /**/ "		if (cmp)\n"
/**/ /**/ "			return cmp;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	if (mySize < otSize)\n"
/**/ /**/ "		return LESS_THAN;\n"
/**/ /**/ "	if (mySize > otSize)\n"
/**/ /**/ "		return GREATER_THAN;\n"
/**/ /**/ "} else {\n"
/**/ /**/ "	local otIter = this.operator iter();\n"
/**/ /**/ "	for (local index: [:mySize]) {\n"
/**/ /**/ "		local myItem, otItem;\n"
/**/ /**/ "		try {\n"
/**/ /**/ "			myItem = this[index];\n"
/**/ /**/ "		} catch (UnboundItem) {\n"
/**/ /**/ "			foreach (none: otIter)\n"
/**/ /**/ "				return LESS_THAN;\n"
/**/ /**/ "			return GREATER_THAN;\n"
/**/ /**/ "		} catch (IndexError) {\n"
/**/ /**/ "			foreach (none: otIter)\n"
/**/ /**/ "				return LESS_THAN;\n"
/**/ /**/ "			return EQUALS;\n"
/**/ /**/ "		}\n"
/**/ /**/ "		foreach(otItem: otIter)\n"
/**/ /**/ "			goto hasOtItem;\n"
/**/ /**/ "		return GREATER_THAN;\n"
/**/ /**/ "hasOtItem:\n"
/**/ /**/ "		local cmp = myItem <=> otItem;\n"
/**/ /**/ "		if (cmp)\n"
/**/ /**/ "			return cmp;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	foreach (none: otIter)\n"
/**/ /**/ "		return LESS_THAN;\n"
/**/ /**/ "}\n"
/**/ /**/ "return EQUAL;\n"
/**/ "}&"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements|"
/**/ "{³}A more optimized implementation is used for ${operator ==} and ${operator !=}"
"}\n"
"\n"

"[]->\n"
"#tOverflowError{The given @index is negative}"
"#tIndexError{The given @index is greater than the length of @this Sequence (${##this})}"
"#tUnboundItem{The item associated with @index is unbound}"
"Returns the @{index}th element of @this Sequence, as determinable by enumeration\n"
"#T{Requirements|Implementation~"
/**/ "${operator getitem}¹|${return this[index];}&"
/**/ "${operator iter}|${"
/**/ /**/ "local i = 0;\n"
/**/ /**/ "for (local v: this) {\n"
/**/ /**/ "	if (i >= index)\n"
/**/ /**/ "		return v;\n"
/**/ /**/ "	++i;\n"
/**/ /**/ "}\n"
/**/ /**/ "throw IndexError(...);"
/**/ "}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?."
"}\n"
"\n"

"#->\n"
"Returns the length of @this Sequence, as determinable by enumeration\n"
"#T{Requirements|Implementation~"
/**/ "${operator size}¹|${return #this;}&"
/**/ "${operator iter}²|${"
/**/ /**/ "local result = 0;\n"
/**/ /**/ "for (none: this)\n"
/**/ /**/ "	++result;\n"
/**/ /**/ "return result;"
/**/ "}&"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

"contains->\n"
"Returns ?t/?f indicative of @item being apart of @this Sequence\n"
"This operator is an alias for the #contains member function, "
/**/ "which allows the use of an additional key function\n"
"#T{Requirements|Implementation~"
/**/ "${operator contains}¹|${return item in this;}&"
/**/ "${operator iter}|${"
/**/ /**/ "for (local x: this) {\n"
/**/ /**/ "	if (deemon.equals(item, x))\n"
/**/ /**/ "		return true;\n"
/**/ /**/ "}\n"
/**/ /**/ "return false;"
/**/ "}&"
/**/ "${operator getitem}, ${operator size}¹²|${"
/**/ /**/ "local size = ##this;\n"
/**/ /**/ "for (local i: [:size]) {\n"
/**/ /**/ "	local myItem;\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		myItem = this[i];\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	if (deemon.equals(item, myItem))\n"
/**/ /**/ "		return true;\n"
/**/ /**/ "}\n"
/**/ /**/ "return false;"
/**/ "}&"
/**/ "${operator getitem}¹²|${"
/**/ /**/ "for (local i = 0;; ++i) {\n"
/**/ /**/ "	local myItem;\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		myItem = this[i];\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	if (deemon.equals(item, myItem))\n"
/**/ /**/ "		return true;\n"
/**/ /**/ "}\n"
/**/ /**/ "return false;"
/**/ "}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
"~->?DSet\n"
"Alias for ${(this as Set).operator ~ ()}. S.a. ?Aop:inv?DSet\n"
"\n"

"sub->?DSet\n"
"Alias for ${(this as Set).operator - (other)}. S.a. ?Aop:sub?DSet\n"
"\n"

"|->?DSet\n"
"Alias for ${(this as Set).operator | (other)}. S.a. ?Aop:or?DSet\n"
"\n"

"&->?DSet\n"
"Alias for ${(this as Set).operator & (other)}. S.a. ?Aop:and?DSet\n"
"\n"

"^->?DSet\n"
"Alias for ${(this as Set).operator ^ (other)}. S.a. ?Aop:xor?DSet\n"
"\n"
#endif /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */

"[:](start:?X2?N?Dint,end:?X2?N?Dint)->\n"
"Returns a sub-range of @this Sequence, spanning across all elements from @start to @end\n"
"If either @start or @end is smaller than ${0}, ${##this} is added once to either\n"
"If @end is greater than the length of @this Sequence, it is clamped to its length\n"
"When @start is greater than, or equal to @end or ${##this}, an empty Sequence is returned\n"
"This operator is implemented similar to the following, however the actual "
/**/ "return type may be a proxy Sequence that further optimizes the iteration "
/**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
/**/ "as how the sub-range is accessed (i.e. ${this[10:20][3]} will invoke ${this[13]}).\n"
"#T{Requirements|Implementation~"
/**/ "${operator getrange}¹|${return this[start:end];}&"
/**/ "${operator getitem}, ${operator size}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this)...;\n"
/**/ /**/ "for (local i: [start:end]) {\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		yield this[i];\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}"
/**/ "${operator iter}, ${operator size}²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this)...;\n"
/**/ /**/ "local it = this.operator iter();\n"
/**/ /**/ "for (none: [:start]) {\n"
/**/ /**/ "	foreach (none: it)\n"
/**/ /**/ "		break;\n"
/**/ /**/ "}\n"
/**/ /**/ "for (none: [start:end]) {\n"
/**/ /**/ "	foreach (local x: it) {\n"
/**/ /**/ "		yield x;\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

"iter->\n"
"Returns a general-purpose Iterator using ${operator []} (getitem) and ${operator ##} (size) "
/**/ "to enumerate a Sequence that is implemented using a size+index approach\n"
"#T{Requirements|Implementation~"
/**/ "${operator iter}|${return this.operator iter();}&"
/**/ "${operator getitem}, ${operator size}¹²|${"
/**/ /**/ "local size = ##this;\n"
/**/ /**/ "for (local i: [:size]) {\n"
/**/ /**/ "	local item;\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		item = this[i];\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	yield item;\n"
/**/ /**/ "}\n"
/**/ /**/ "return false;"
/**/ "}&"
/**/ "${operator getitem}¹²|${"
/**/ /**/ "for (local i = 0;; ++i) {\n"
/**/ /**/ "	local item;\n"
/**/ /**/ "	try {\n"
/**/ /**/ "		item = this[i];\n"
/**/ /**/ "	} catch (UnboundItem) {\n"
/**/ /**/ "		continue;\n"
/**/ /**/ "	} catch (IndexError) {\n"
/**/ /**/ "		break;\n"
/**/ /**/ "	}\n"
/**/ /**/ "	yield item;\n"
/**/ /**/ "}\n"
/**/ /**/ "return false;"
/**/ "}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

":=->\n"
"Alias for ${this[:] = other}\n"
"\n"

"del[]->\n"
"#tIntegerOverflow{The given @index is negative, or too large}"
"#tIndexError{The given @index is out of bounds}"
"Either remove (as per ?#erase) the item under @index (?#{op:size} changes), or mark "
/**/ "said item as unbound (?#{op:size} remains unchanged)\n"
"#T{Requirements|Implementation~"
/**/ "${operator delitem}¹|${del this[index];}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?."
"}\n"
"\n"

"[]=->\n"
"#tIntegerOverflow{The given @index is negative, or too large}"
"#tIndexError{The given @index is out of bounds}"
"Set the value of @index to @value\n"
"#T{Requirements|Implementation~"
/**/ "${operator setitem}¹|${this[index] = value;}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?."
"}\n"
"\n"

"del[:]->\n"
"#tIntegerOverflow{@start or @end are too large}"
"#tSequenceError{@this Sequence cannot be resized}"
"Delete, or unbind all items within the given range\n"
"#T{Requirements|Implementation~"
/**/ "${operator delrange}¹|${del this[start:end];}&"
/**/ "${operator setrange}¹²|${this[start:end] = none;}&"
/**/ "${operator size}, ${function erase}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end)\n"
/**/ /**/ "	this.erase(start, end - start);"
/**/ "}&"
/**/ "${operator size}, ${function pop}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end) {\n"
/**/ /**/ "	while (end > start) {\n"
/**/ /**/ "		--end\n"
/**/ /**/ "		this.pop(end);\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}&"
/**/ "${operator size}, ${operator delitem}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end) {\n"
/**/ /**/ "	while (end > start) {\n"
/**/ /**/ "		--end\n"
/**/ /**/ "		del this[end];\n"
/**/ /**/ "	}\n"
/**/ /**/ "}"
/**/ "}"
"}"
"#L{"
/**/ "{¹}Only when ?A__seqclass__?DType is ?.|"
/**/ "{²}Default implementation provided if sub-class matches requirements"
"}\n"
"\n"

"[:]=->\n"
"#tIntegerOverflow{@start or @end are too large}"
"#tSequenceError{@this Sequence is immutable, or cannot be resized}"
"Override the given range with items from @{values}. With @values is smaller than "
/**/ "the target range, @this sequence will be shrunk. When it is larger, it will grow. "
/**/ "Note that unlike ?#{op:delrange}, this operator is required to resize the sequence, and "
/**/ "is not allowed to leave items unbound.\n"
"#T{Requirements|Implementation~"
/**/ "${operator setrange}¹|${this[start:end] = values;}&"
/**/ "${operator size}, ${function erase}, ${function insertall}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end)\n"
/**/ /**/ "	this.erase(start, end - start);\n"
/**/ /**/ "this.insertall(start, values);"
/**/ "}&"
/**/ "${operator size}, ${function pop}, ${function insertall}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end) {\n"
/**/ /**/ "	while (end > start) {\n"
/**/ /**/ "		--end\n"
/**/ /**/ "		this.pop(end);\n"
/**/ /**/ "	}\n"
/**/ /**/ "}\n"
/**/ /**/ "this.insertall(start, values);"
/**/ "}&"
/**/ "${operator size}, ${function erase}, ${function insert}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end)\n"
/**/ /**/ "	this.erase(start, end - start);\n"
/**/ /**/ "for (local v: values)\n"
/**/ /**/ "	this.insert(start++, v);"
/**/ "}&"
/**/ "${operator size}, ${function pop}, ${function insert}¹²|${"
/**/ /**/ "start, end = util.clamprange(start, end, ##this);\n"
/**/ /**/ "if (start < end) {\n"
/**/ /**/ "	while (end > start) {\n"
/**/ /**/ "		--end\n"
/**/ /**/ "		this.pop(end);\n"
/**/ /**/ "	}\n"
/**/ /**/ "}\n"
/**/ /**/ "for (local v: values)\n"
/**/ /**/ "	this.insert(start++, v);"
/**/ "}&"
"}"
"#B{¹}: Only when ?A__seqclass__?DType is ?.\n"
"#B{²}: Default implementation provided if sub-class matches requirements";
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
				/* .tp_ctor      = */ (dfunptr_t)&DeeNone_OperatorCtor, /* Allow default-construction of Sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
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
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
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
