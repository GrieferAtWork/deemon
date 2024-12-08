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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_C
#define GUARD_DEEMON_OBJECTS_SEQ_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/combinations.h"
#include "seq/concat.h"
#include "seq/default-api.h"
#include "seq/default-iterators.h"
#include "seq/default-reversed.h"
#include "seq/default-sequences.h"
#include "seq/each.h"
#include "seq/filter.h"
#include "seq/hashfilter.h"
#include "seq/locateall.h"
#include "seq/mapped.h"
#include "seq/repeat.h"
#include "seq/segments.h"
#include "seq/simpleproxy.h"
#include "seq/svec.h"
#include "seq/unique-iterator.h"
#include "seq_functions.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__


/* Provide aliases for certain Set operators in Sequence */
#undef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
#define CONFIG_HAVE_SET_OPERATORS_IN_SEQ

DECL_BEGIN

/* TODO: Re-write all of the documentation for functions/operators to document how defaults work now. */

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

/* Lookup the closest NSI descriptor for `tp', or return `NULL'
 * if the top-most type implementing any sequence operator doesn't
 * expose NSI functionality. */
PUBLIC WUNUSED NONNULL((1)) struct type_nsi const *DCALL
DeeType_NSI(DeeTypeObject *__restrict tp) {
	ASSERT_OBJECT_TYPE(tp, &DeeType_Type);
	if (tp->tp_seq)
		return tp->tp_seq->tp_nsi;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
seqtype_get_Iterator(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	if ((self->tp_seq && self->tp_seq->tp_iter) || DeeType_InheritIter(self)) {
		DREF DeeObject *(DCALL *tp_iter)(DeeObject *__restrict self);
		tp_iter = self->tp_seq->tp_iter;
		if (tp_iter == &DeeObject_DefaultIterWithForeach) {
			result = &DefaultIterator_WithForeach_Type;
		} else if (tp_iter == &DeeObject_DefaultIterWithForeachPair) {
			result = &DefaultIterator_WithForeachPair_Type;
		} else if (tp_iter == &DeeObject_DefaultIterWithEnumerate) {
			result = &DefaultIterator_WithEnumerateSeq_Type;
		} else if (tp_iter == &DeeObject_DefaultIterWithEnumerateIndex) {
			result = &DefaultIterator_WithEnumerateIndexSeq_Type;
		} else if (tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItem) {
			result = &DefaultIterator_WithIterKeysAndTryGetItemSeq_Type;
		} else if (tp_iter == &DeeObject_DefaultIterWithIterKeysAndGetItem) {
			result = &DefaultIterator_WithIterKeysAndGetItemSeq_Type; /* or: DefaultIterator_WithIterKeysAndTGetItemSeq_Type */
		} else if (tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault) {
			result = &DefaultIterator_WithIterKeysAndTTryGetItemSeq_Type; /* or: DefaultIterator_WithIterKeysAndTryGetItemSeq_Type */
		} else if (tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast) {
			result = &DefaultIterator_WithSizeAndGetItemIndexFast_Type;
		} else if (tp_iter == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex) {
			result = &DefaultIterator_WithSizeAndTryGetItemIndex_Type;
		} else if (tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndex) {
			result = &DefaultIterator_WithSizeAndGetItemIndex_Type;
		} else if (tp_iter == &DeeSeq_DefaultIterWithGetItemIndex) {
			result = &DefaultIterator_WithGetItemIndex_Type;
		} else if (tp_iter == &DeeSeq_DefaultIterWithSizeObAndGetItem) {
			result = &DefaultIterator_WithSizeObAndGetItem_Type; /*or: DefaultIterator_WithTSizeAndGetItem_Type */
		} else if (tp_iter == &DeeSeq_DefaultIterWithGetItem) {
			result = &DefaultIterator_WithGetItem_Type; /* or: DefaultIterator_WithTGetItem_Type */
		} else if (tp_iter == &DeeMap_DefaultIterWithEnumerate) {
			result = &DefaultIterator_WithEnumerateMap_Type;
		} else if (tp_iter == &DeeMap_DefaultIterWithEnumerateIndex) {
			result = &DefaultIterator_WithEnumerateIndexMap_Type;
		} else if (tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItem) {
			result = &DefaultIterator_WithIterKeysAndTryGetItemMap_Type;
		} else if (tp_iter == &DeeMap_DefaultIterWithIterKeysAndGetItem) {
			result = &DefaultIterator_WithIterKeysAndGetItemMap_Type; /* or: DefaultIterator_WithIterKeysAndTGetItemMap_Type */
		} else if (tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault) {
			result = &DefaultIterator_WithIterKeysAndTTryGetItemMap_Type; /* or: DefaultIterator_WithIterKeysAndTryGetItemMap_Type */
		}
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


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
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
	DO(err, DeeObject_Foreach(self, &foreach_seq_printrepr_cb, &data));
	DO(err, data.fsprd_first ? DeeFormat_PRINT(printer, arg, "}")
	                         : DeeFormat_PRINT(printer, arg, " }"));
done:
	return result;
err:
	return temp;
#undef DO
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_mul(DeeObject *self, DeeObject *countob) {
	size_t count;
	if (DeeObject_AsSize(countob, &count))
		goto err;
	return DeeSeq_Repeat(self, count);
err:
	return NULL;
}


PRIVATE struct type_math seq_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
	/* .tp_inv         = */ &DeeSet_OperatorInv,
#else /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
	/* .tp_inv         = */ NULL,
#endif /* !CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &DeeSeq_Concat,
#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
	/* .tp_inv         = */ &DeeSet_OperatorSub,
#else /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
	/* .tp_sub         = */ NULL,
#endif /* !CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
	/* .tp_mul         = */ &seq_mul,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
	/* .tp_and         = */ &DeeSet_OperatorAnd,
	/* .tp_or          = */ &DeeSet_OperatorAdd,
	/* .tp_xor         = */ &DeeSet_OperatorXor,
#else /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
	/* .tp_and         = */ NULL,
	/* .tp_or          = */ NULL,
	/* .tp_xor         = */ NULL,
#endif /* !CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ &DeeSeq_OperatorInplaceAdd,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ &DeeSeq_OperatorInplaceMul,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL,
};


/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("Frozen");
]]]*/
#define Dee_HashStr__Frozen _Dee_HashSelectC(0xa7ed3902, 0x16013e56a91991ea)
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
seq_Frozen_get(DeeTypeObject *__restrict self) {
	int error;
	DREF DeeTypeObject *result;
	struct attribute_info info;
	struct attribute_lookup_rules rules;
	rules.alr_name       = "Frozen";
	rules.alr_hash       = Dee_HashStr__Frozen;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = ATTR_PERMGET | ATTR_IMEMBER;
	rules.alr_perm_value = ATTR_PERMGET | ATTR_IMEMBER;
	error = DeeObject_FindAttr(Dee_TYPE(self),
	                           (DeeObject *)self,
	                           &info,
	                           &rules);
	if unlikely(error < 0)
		goto err;
	if (error != 0) {
		/* If the type doesn't provide its own override for `Frozen', the default
		 * implementation provided by us will return information via a tuple instead. */
		return_reference_(&DeeTuple_Type);
	}
	if (info.a_attrtype) {
		result = info.a_attrtype;
		Dee_Incref(result);
	} else if (info.a_decl == (DeeObject *)&DeeSeq_Type) {
		/* We've the ones implementing the attribute, and since we know that we're
		 * already implementing it by casting ourself to a tuple, inform the caller
		 * of exactly that. */
		result = &DeeTuple_Type;
		Dee_Incref(&DeeTuple_Type);
	} else {
		if (info.a_doc) {
			/* TODO: Use doc meta-information to determine the return type! */
		}
		/* Fallback: just tell the caller what they already know: a Sequence will be returned... */
		result = &DeeSeq_Type;
		Dee_Incref(&DeeSeq_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
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
seq_at_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:at", &index))
		goto err;
	return DeeObject_GetItem(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_empty_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":empty"))
		goto err;
	result = DeeObject_Bool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_non_empty_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":non_empty"))
		goto err;
	result = DeeObject_Bool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_front_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":front"))
		goto err;
	return DeeObject_GetAttr(self, (DeeObject *)&str_first);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_back_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":back"))
		goto err;
	return DeeObject_GetAttr(self, (DeeObject *)&str_last);
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
seq_locateall(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "o|o:locateall", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_LocateAll(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_LocateAll(self, elem, key);
		Dee_Decref(elem);
	}
	return result;
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
	mylen = DeeObject_Size(self);
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
seq_combinations(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t r;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":combinations", &r))
		goto err;
	return DeeSeq_Combinations(self, r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_repeatcombinations(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t r;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":repeatcombinations", &r))
		goto err;
	return DeeSeq_RepeatCombinations(self, r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_permutations(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t r;
	DeeObject *arg = Dee_None;
	if (DeeArg_Unpack(argc, argv, "|o:permutations", &arg))
		goto err;
	if (DeeNone_Check(arg))
		return DeeSeq_Permutations(self);
	if (DeeObject_AsSize(arg, &r))
		goto err;
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
	         ? DeeSeq_InvokeFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeFind(self, item, start, end);
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
	         ? DeeSeq_InvokeRFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeRFind(self, item, start, end);
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
		if unlikely(DeeObject_Unpack(self, size_or_minsize, DeeTuple_ELEM(result)))
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
		realsize = DeeObject_UnpackEx(self, size_or_minsize, maxsize,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distinct(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF UniqueSetWithKey *result;
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__key, "|o:distinct", &key))
		goto err;
	if (!key)
		return DeeSuper_New(&DeeSet_Type, self);
	result = DeeObject_MALLOC(UniqueSetWithKey);
	if unlikely(!result)
		goto err;
	result->uswk_key = key;
	Dee_Incref(key);
	result->uswk_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &UniqueSetWithKey_Type);
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
	         ? DeeSeq_InvokeBFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeBFind(self, item, start, end);
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
	         ? DeeSeq_InvokeBFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeBFind(self, item, start, end);
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
	    ? DeeSeq_InvokeBRangeWithKey(self, item, start, end, key, result_range)
	    : DeeSeq_InvokeBRange(self, item, start, end, result_range))
		goto err;
	return DeeSeq_OperatorGetRangeIndex(self, result_range[0], result_range[1]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_binsert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item;
	size_t index, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end,
	                    "o|" UNPuSIZ UNPuSIZ "o:binsert",
	                    &item, &start, &end))
		goto err;
	index = DeeSeq_InvokeBPosition(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(DeeSeq_InvokeInsert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because aliased by `List.pop_front' */
seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return DeeSeq_InvokePop(self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because aliased by `List.pop_back' */
seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	return DeeSeq_InvokePop(self, -1);
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
	TYPE_KWMETHOD(STR_reduce, &DeeMH_seq_reduce,
	              "(combine:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,init?)->\n"
	              DOC_throws_ValueError_if_empty
	              "Combines consecutive elements of @this Sequence by passing them as pairs of 2 to @combine, "
	              /**/ "then re-using its return value in the next invocation, before finally returning its last "
	              /**/ "return value. If the Sequence consists of only 1 element, @combine is never invoked.\n"
	              "When given, @init is used as the initial lhs-operand, rather than the first element of the Sequence\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function reduce}|${"
	              /**/ /**/ "return start != 0 || end != int.SIZE_MAX ? this.reduce(combine, start, end)\n"
	              /**/ /**/ "                                         : this.reduce(combine);"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "for (local x: this)\n"
	              /**/ /**/ "	init = init is bound ? combine(init, x) : x;\n"
	              /**/ /**/ "if (init !is bound)\n"
	              /**/ /**/ "	throw ValueError(...);\n"
	              /**/ /**/ "return init;"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
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
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD("enumerate", &DeeMH_seq_enumerate,
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
	              /**/ "	print f\"[{repr key}]: {value is bound ? repr value : \"<unbound>\"}\"\n"
	              /**/ "});"
	              "}\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${operator iterkeys}¹|${" /* TODO: Expose "operator iterkeys" to user-code */
	              /**/ /**/ "foreach (local key: Object.__iterkeys__(this)) {\n"
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
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "return none;"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}When @cb isn't given, filter for bound items and yield as ${(key, value)} pairs"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?.|"
	              "}"),
	TYPE_KWMETHOD(STR_sum, &DeeMH_seq_sum,
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
	TYPE_KWMETHOD(STR_any, &DeeMH_seq_any,
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
	TYPE_KWMETHOD(STR_all, &DeeMH_seq_all,
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
	TYPE_KWMETHOD(STR_parity, &DeeMH_seq_parity,
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
	TYPE_KWMETHOD(STR_min, &DeeMH_seq_min,
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
	TYPE_KWMETHOD(STR_max, &DeeMH_seq_max,
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
	TYPE_KWMETHOD(STR_count, &DeeMH_seq_count,
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
	              /**/ /**/ "for (local myItem: this) {\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(myItem)))\n"
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
	TYPE_KWMETHOD(STR_locate, &DeeMH_seq_locate,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->\n"
	              DOC_param_item
	              DOC_param_key
	              DOC_throws_ValueError_if_not_found
	              "Returns the first item equal to @item\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function locate}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.locate(item, start, end)\n"
	              /**/ /**/ "	                                         : this.locate(item);\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.locate(item, start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.locate(item, 0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.locate(item, key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "for (local myItem: this) {\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(myItem)))\n"
	              /**/ /**/ "		return myItem;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "throw ValueError(...);"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local result = Cell();\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "local ok = Sequence.enumerate(this, (none, value?) -\\> {\n"
	              /**/ /**/ "	if (value is bound) {\n"
	              /**/ /**/ "		if (deemon.equals(keyedItem, key(value))) {\n"
	              /**/ /**/ "			result.value = value;\n"
	              /**/ /**/ "			return true;\n"
	              /**/ /**/ "		}\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "}, start, end);\n"
	              /**/ /**/ "if (ok !is none)\n"
	              /**/ /**/ "	return result.value;\n"
	              /**/ /**/ "throw ValueError(...);"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),
	TYPE_KWMETHOD(STR_rlocate, &DeeMH_seq_rlocate,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->\n"
	              DOC_param_item
	              DOC_param_key
	              DOC_throws_ValueError_if_not_found
	              "Returns the last item equal to @item\n"
	              "#T{Requirements|Implementation~"
	              /**/ "${function rlocate}|${"
	              /**/ /**/ "if (key is none) {\n"
	              /**/ /**/ "	return start != 0 || end != int.SIZE_MAX ? this.rlocate(item, start, end)\n"
	              /**/ /**/ "	                                         : this.rlocate(item);\n"
	              /**/ /**/ "} else if (start != 0 || end != int.SIZE_MAX) {\n"
	              /**/ /**/ "	return this.rlocate(item, start, end, key);\n"
	              /**/ /**/ "} else if (type(this).__seqclass__ == Sequence) {\n"
	              /**/ /**/ "	return this.rlocate(item, 0, int.SIZE_MAX, key);\n"
	              /**/ /**/ "} else {\n"
	              /**/ /**/ "	return this.rlocate(item, key);\n"
	              /**/ /**/ "}\n"
	              /**/ "}&"
	              /**/ "${operator iter}¹|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "for (local myItem: this) {\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(myItem)))\n"
	              /**/ /**/ "		result = myItem;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (result is bound)\n"
	              /**/ /**/ "	return result;\n"
	              /**/ /**/ "throw ValueError(...);"
	              /**/ "}&"
	              /**/ "${operator size}, ${operator getitem}²|${"
	              /**/ /**/ "key = key ?? x -\\> x;\n"
	              /**/ /**/ "local realSize = ##this;\n"
	              /**/ /**/ "if (end > realSize)\n"
	              /**/ /**/ "	end = realSize;\n"
	              /**/ /**/ "if (start > end)\n"
	              /**/ /**/ "	start = end;\n"
	              /**/ /**/ "local result;\n"
	              /**/ /**/ "local keyedItem = key(item);\n"
	              /**/ /**/ "for (local i: [start: end]) {\n"
	              /**/ /**/ "	local myItem;\n"
	              /**/ /**/ "	try {\n"
	              /**/ /**/ "		myItem = this[index];\n"
	              /**/ /**/ "	} catch (UnboundItem) {\n"
	              /**/ /**/ "		continue;\n"
	              /**/ /**/ "	} catch (IndexError) {\n"
	              /**/ /**/ "		break;\n"
	              /**/ /**/ "	}\n"
	              /**/ /**/ "	if (deemon.equals(keyedItem, key(myItem)))\n"
	              /**/ /**/ "		result = myItem;\n"
	              /**/ /**/ "}\n"
	              /**/ /**/ "if (result is bound)\n"
	              /**/ /**/ "	return result;\n"
	              /**/ /**/ "throw ValueError(...);"
	              /**/ "}"
	              "}"
	              "#L{"
	              /**/ "{¹}Only when @start/@end aren't given or describe the entire sequence|"
	              /**/ "{²}Only when ?A__seqclass__?DType is ?."
	              "}"),

	/* TODO: findall: "(item,start:?Dint,end:?Dint,key:?DCallable=!N)->?S?Dint" */
	/* TODO: findallof: "(items:?S?O,start:?Dint,end:?Dint,key:?DCallable=!N)->?S?Dint" */
	/* TODO: findany: "(items:?S?O,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?N" */
	/* TODO: rfindany: "(items:?S?O,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?N" */
	/* TODO: indexany: "(items:?S?O,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint" */
	/* TODO: rindexany: "(items:?S?O,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint" */

	TYPE_METHOD("filter", &seq_filter,
	            "(keep:?DCallable)->?DSequence\n"
	            "#pkeep{A key function which is called for each element of @this Sequence"
	            /**/ "Returns a sub-Sequence of all elements for which ${keep(item)} evaluates to ?t}"
	            "Semantically, this is identical to ${(for (local x: this) if (keep(x)) x)}\n"
	            "${"
	            /**/ "function filter(keep: Callable): Sequence {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		if (keep(x))\n"
	            /**/ "			yield x;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("ubfilter", &seq_ubfilter,
	            "(keep:?DCallable)->?DSequence\n"
	            "#pkeep{A key function which is called for each element of @this Sequence"
	            /**/ "Returns a sub-Sequence of all elements for which ${keep(item)} evaluates to ?t}"
	            "Same as ?#filter, but the returned sequence has the same size as @this, and filtered "
	            /**/ "elements are simply treated as though they were unbound:\n"
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

	TYPE_METHOD("locateall", &seq_locateall,
	            /* TODO: "(item,start:?Dint,end:?Dint,key:?DCallable=!N)->?S?O\n" */
	            "(item,key:?DCallable=!N)->?S?O\n"
	            DOC_param_item
	            DOC_param_key
	            "Returns a Sequence of items equal to @item\n"
	            "${"
	            /**/ "function locateall(item: Object, key: Callable): Sequence {\n"
	            /**/ "	import Error from deemon;\n"
	            /**/ "	if (key !is none)\n"
	            /**/ "		item = key(item);\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (key !is none) {\n"
	            /**/ "			if (item == key(x))\n"
	            /**/ "				yield x;\n"
	            /**/ "		} else {\n"
	            /**/ "			if (item == x)\n"
	            /**/ "				yield x;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),

	TYPE_KWMETHOD(STR_contains, &DeeMH_seq_contains,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t if @this Sequence contains an element matching @item"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_startswith, &DeeMH_seq_startswith,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t / ?f indicative of @this Sequence's first element matching :item\n"
	              "The implementation of this is derived from #first, where the found is then compared "
	              /**/ "against @item, potentially through use of @{key}: ${key(first) == key(item)} or ${first == item}, "
	              /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_endswith, &DeeMH_seq_endswith,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_item
	              DOC_param_key
	              "Returns ?t / ?f indicative of @this Sequence's last element matching :item\n"
	              "The implementation of this is derived from #last, where the found is then compared "
	              /**/ "against @item, potentially through use of @{key}: ${key(last) == key(item)} or ${last == item}, "
	              /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_find, &DeeMH_seq_find,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Search for the first element matching @item and return its index. "
	              /**/ "If no such element exists, return ${-1} instead"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_rfind, &DeeMH_seq_rfind,
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
	TYPE_METHOD("combinations",
	            &seq_combinations,
	            "(r:?Dint)->?S?DSequence\n"
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
	TYPE_METHOD("repeatcombinations",
	            &seq_repeatcombinations,
	            "(r:?Dint)->?S?DSequence\n"
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
	TYPE_METHOD("permutations",
	            &seq_permutations,
	            "(r:?Dint=!N)->?S?DSequence\n"
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

	/* TODO: join(items: {Sequence...}): Sequence */
	/* TODO: strip(item: Object, key: Callable = none): Sequence */
	/* TODO: lstrip(item: Object, key: Callable = none): Sequence */
	/* TODO: rstrip(item: Object, key: Callable = none): Sequence */
	/* TODO: split(sep: Object, key: Callable = none): Sequence */

	/* TODO: countseq(seq: Sequence, key: Callable = none): int */
	/* TODO: partition(item: Object, key: Callable = none): (Sequence, (item), Sequence) */
	/* TODO: rpartition(item: Object, key: Callable = none): (Sequence, (item), Sequence) */
	/* TODO: partitionseq(seq: Sequence, key: Callable = none): (Sequence, seq, Sequence) */
	/* TODO: rpartitionseq(seq: Sequence, key: Callable = none): (Sequence, seq, Sequence) */
	/* TODO: startswithseq(seq: Sequence, key: Callable = none): bool */
	/* TODO: endswithseq(seq: Sequence, key: Callable = none): bool */
	/* TODO: findseq(seq: Sequence, key: Callable = none): int */
	/* TODO: rfindseq(seq: Sequence, key: Callable = none): int */
	/* TODO: indexseq(seq: Sequence, key: Callable = none): int */
	/* TODO: rindexseq(seq: Sequence, key: Callable = none): int */
	/* TODO: stripseq(items: Sequence, key: Callable = none): Sequence */
	/* TODO: lstripseq(items: Sequence, key: Callable = none): Sequence */
	/* TODO: rstripseq(items: Sequence, key: Callable = none): Sequence */
	/* TODO: splitseq(seq: Sequence, key: Callable = none): Sequence */


	/* Functions for mutable sequences. */
	TYPE_KWMETHOD(STR_reversed, &DeeMH_seq_reversed,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?DSequence\n"
	              "Return a Sequence that contains the elements of @this Sequence in reverse order\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_sorted, &DeeMH_seq_sorted,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?DSequence\n"
	              "Return a Sequence that contains all elements from @this Sequence, "
	              /**/ "but sorted in ascending order, or in accordance to @key\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_insert, &DeeMH_seq_insert,
	              "(index:?Dint,item)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Insert the given @item under @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_insertall, &DeeMH_seq_insertall,
	              "(index:?Dint,items:?DSequence)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Insert all elements from @items at @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(STR_append, &DeeMH_seq_append,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Append the given @item at the end of @this Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(STR_extend, &DeeMH_seq_extend,
	            "(items:?DSequence)\n"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Append all elements from @items at the end of @this Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_erase, &DeeMH_seq_erase,
	              "(index:?Dint,count=!1)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Erase up to @count elements starting at @index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_xchitem, &DeeMH_seq_xchitem,
	              "(index:?Dint,value)->\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "Exchange the @index'th element of @this Sequence with the given "
	              /**/ "@value, returning the old element found under that index"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_pop, &DeeMH_seq_pop,
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
	TYPE_METHOD(STR_pushfront, &DeeMH_seq_pushfront,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Convenience wrapper for ?#insert at position $0"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(STR_pushback, &DeeMH_seq_append,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "Alias for ?#append"),
	TYPE_KWMETHOD(STR_remove, &DeeMH_seq_remove,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find the first instance of @item and remove it, returning ?t if an "
	              /**/ "element got removed, or ?f if @item could not be found"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_rremove, &DeeMH_seq_rremove,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find the last instance of @item and remove it, returning ?t if an "
	              /**/ "element got removed, or ?f if @item could not be found"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_removeall, &DeeMH_seq_removeall,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Find all instance of @item and remove them, returning the number of "
	              /**/ "instances found (and consequently removed)"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_removeif, &DeeMH_seq_removeif,
	              "(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Remove all elements within the given sub-range, for which ${should(item)} "
	              /**/ "evaluates to ?t, and return the number of elements found (and consequently removed)"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_METHOD(STR_clear, &DeeMH_seq_clear,
	            "()\n"
	            "#tSequenceError{@this Sequence is immutable}"
	            "Clear all elements from the Sequence"
	            ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_resize, &DeeMH_seq_resize,
	              "(size:?Dint,filler=!N)\n"
	              "#tSequenceError{@this Sequence isn't resizable}"
	              "Resize @this Sequence to have a new length of @size "
	              /**/ "items, using @filler to initialize newly added entries"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_fill, &DeeMH_seq_fill,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,filler=!N)\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "Assign @filler to all elements within the given sub-range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_reverse, &DeeMH_seq_reverse,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "Reverse the order of all elements within the given range"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_sort, &DeeMH_seq_sort,
	              "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)\n"
	              DOC_param_key
	              "#tSequenceError{@this Sequence is immutable}"
	              "Sort the elements within the given range"
	              ""), /* TODO: Requirements|Implementation table */

	/* Binary search API */
	TYPE_KWMETHOD(STR_bfind, &DeeMH_seq_bfind,
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
	TYPE_KWMETHOD(STR_bposition, &DeeMH_seq_bposition,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Same as ?#bfind, but return (an) index where @item should be inserted, rather "
	              /**/ "than ?N when @this doesn't contain any matching object"
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_brange, &DeeMH_seq_brange,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?T2?Dint?Dint\n"
	              DOC_param_item
	              DOC_param_key
	              "Similar to ?#bfind, but return a tuple ${[begin,end)} of integers representing "
	              /**/ "the lower and upper bound of indices for elements from @this matching @item.\n"
	              "NOTE: The returned tuple is allowed to be an ASP, meaning that its elements may "
	              /**/ "be calculated lazily, and are prone to change as the result of @this changing."
	              ""), /* TODO: Requirements|Implementation table */
	TYPE_KWMETHOD(STR_blocate, &DeeMH_seq_blocate,
	              "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N,defl?)->\n"
	              DOC_param_item
	              DOC_param_key
	              "#tValueError{The Sequence does not contain an item matching @item}"
	              "Same as ?#bfind, but return the matching item, rather than its index"
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
	 * >> (ob as Sequence)[42];
	 *
	 * TODO: The compiler should also be able to automatically
	 *       optimize the second version into the first. */
	TYPE_METHOD("__bool__", &default_seq___bool__,
	            "->?Dbool\n"
	            "Alias for ${!!(this as Sequence)}"),
	TYPE_METHOD("__iter__", &default_seq___iter__,
	            "->?DIterator\n"
	            "Alias for ${(this as Sequence).operator iter()}"),
	TYPE_METHOD("__size__", &default_seq___size__,
	            "->?Dint\n"
	            "Alias for ${##(this as Sequence)}"),
	TYPE_METHOD("__contains__", &default_seq___contains__,
	            "(item)->?Dbool\n"
	            "Alias for ${item in (this as Sequence)}"),
	TYPE_METHOD("__getitem__", &default_seq___getitem__,
	            "(index:?Dint)->\n"
	            "Alias for ${(this as Sequence)[index]}"),
	TYPE_METHOD("__delitem__", &default_seq___delitem__,
	            "(index:?Dint)\n"
	            "Alias for ${del (this as Sequence)[index]}"),
	TYPE_METHOD("__setitem__", &default_seq___setitem__,
	            "(index:?Dint,value)\n"
	            "Alias for ${(this as Sequence)[index] = value}"),
	TYPE_KWMETHOD("__getrange__", &default_seq___getrange__,
	              "(start=!0,end?:?X2?N?Dint)->?S?O\n"
	              "Alias for ${(this as Sequence)[start:end]}"),
	TYPE_KWMETHOD("__delrange__", &default_seq___delrange__,
	              "(start=!0,end?:?X2?N?Dint)\n"
	              "Alias for ${del (this as Sequence)[start:end]}"),
	TYPE_KWMETHOD("__setrange__", &default_seq___setrange__,
	              "(start=!0,end?:?X2?N?Dint,values:?S?O)\n"
	              "Alias for ${(this as Sequence)[start:end] = values}"),
	TYPE_METHOD("__foreach__", &default_seq___foreach__,
	            "(cb)->\n"
	            "Alias for:\n"
	            "${"
	            /**/ "for (local item: this as Sequence) {\n"
	            /**/ "	local res = cb(item);\n"
	            /**/ "	if (res !is none)\n"
	            /**/ "		return res;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("__foreach_pair__", &default_seq___foreach_pair__,
	            "(cb)->\n"
	            "Alias for:\n"
	            "${"
	            /**/ "for (local key, value: this as Sequence) {\n"
	            /**/ "	local res = cb(key, value);\n"
	            /**/ "	if (res !is none)\n"
	            /**/ "		return res;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("__enumerate__", &default_seq___enumerate__,
	            "(cb)->\n"
	            "Alias for ${(this as Sequence).enumerate(cb)}"),
	TYPE_KWMETHOD("__enumerate_index__", &default_seq___enumerate_index__,
	              "(cb,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->\n"
	              "Alias for ${Sequence.enumerate(this, cb, start, end)}"),
	TYPE_METHOD("__iterkeys__", &default_seq___iterkeys__,
	            "->?DIterator\n"
	            "Alias for (with special optimizations for known types):\n"
	            "${"
	            /**/ "// When indices cannot be unbound (enumerate all indices)\n"
	            /**/ "return [:##(this as Sequence)].operator iter();\n"
	            /**/ "\n"
	            /**/ "// When indices can be unbound (enumerate bound indices)\n"
	            /**/ "return (() -> {\n"
	            /**/ "	local keys = [];\n"
	            /**/ "	Sequence.enumerate(this, (key, ...) -> {\n"
	            /**/ "		keys.append(key);\n"
	            /**/ "	});\n"
	            /**/ "	return keys;\n"
	            /**/ "})().operator iter()"
	            "}"),
	TYPE_METHOD("__bounditem__", &default_seq___bounditem__,
	            "(index:?Dint,allow_missing=!t)->?Dbool\n"
	            "Alias for ${deemon.bounditem(this as Sequence, index, allow_missing)}"),
	TYPE_METHOD("__hasitem__", &default_seq___hasitem__,
	            "(index:?Dint)->?Dbool\n"
	            "Alias for ${deemon.hasitem(this as Sequence, index)}"),
	TYPE_METHOD("__size_fast__", &default_seq___size_fast__,
	            "->?X2?N?Dint\n"
	            "Returns the same as ?#op:size, but do so in #C{O(1)} time. "
	            /**/ "If the size cannot be computed in that time, return !N instead."),
	TYPE_METHOD("__getitem_index__", &default_seq___getitem_index__,
	            "(index:?Dint)->\n"
	            "Alias for ${(this as Sequence)[index]}"),
	TYPE_METHOD("__delitem_index__", &default_seq___delitem_index__,
	            "(index:?Dint)\n"
	            "Alias for ${del (this as Sequence)[index]}"),
	TYPE_METHOD("__setitem_index__", &default_seq___setitem_index__,
	            "(index:?Dint,value)\n"
	            "Alias for ${(this as Sequence)[index] = value}"),
	TYPE_METHOD("__bounditem_index__", &default_seq___bounditem_index__,
	            "(index:?Dint,allow_missing=!t)->?Dbool\n"
	            "Alias for ${deemon.bounditem(this as Sequence, index, allow_missing)}"),
	TYPE_METHOD("__hasitem_index__", &default_seq___hasitem_index__,
	            "(index:?Dint)->?Dbool\n"
	            "Alias for ${deemon.hasitem(this as Sequence, index)}"),
	TYPE_KWMETHOD("__getrange_index__", &default_seq___getrange_index__,
	              "(start=!0,end?:?X2?N?Dint)->?S?O\n"
	              "Alias for ${(this as Sequence)[start:end]}"),
	TYPE_KWMETHOD("__delrange_index__", &default_seq___delrange_index__,
	              "(start=!0,end?:?X2?N?Dint)\n"
	              "Alias for ${del (this as Sequence)[start:end]}"),
	TYPE_KWMETHOD("__setrange_index__", &default_seq___setrange_index__,
	              "(start=!0,end?:?X2?N?Dint,values:?S?O)\n"
	              "Alias for ${(this as Sequence)[start:end] = values}"),
	TYPE_METHOD("__trygetitem__", &default_seq___trygetitem__,
	            "(index:?Dint,def=!N)->\n"
	            "Alias for ${try (this as Sequence)[index] catch (IndexError | UnboundItem) def}"),
	TYPE_METHOD("__trygetitem_index__", &default_seq___trygetitem_index__,
	            "(index:?Dint,def=!N)->\n"
	            "Alias for ${try (this as Sequence)[index] catch (IndexError | UnboundItem) def}"),
	TYPE_METHOD("__hash__", &default_seq___hash__,
	            "->?Dint\n"
	            "Alias for ${(this as Sequence).operator hash()}"),
	TYPE_METHOD("__compare_eq__", &default_seq___compare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence).operator == (rhs)}"),
	TYPE_METHOD("__compare__", &default_seq___compare__,
	            "(rhs:?S?O)->?Dint\n"
	            "Alias for ${deemon.compare(this as Sequence, rhs)}"),
	TYPE_METHOD("__trycompare_eq__", &default_seq___trycompare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${deemon.equals(this as Sequence, rhs)}"),
	TYPE_METHOD("__eq__", &default_seq___eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) == (rhs)}"),
	TYPE_METHOD("__ne__", &default_seq___ne__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) != (rhs)}"),
	TYPE_METHOD("__lo__", &default_seq___lo__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) < (rhs)}"),
	TYPE_METHOD("__le__", &default_seq___le__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) <= (rhs)}"),
	TYPE_METHOD("__gr__", &default_seq___gr__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) > (rhs)}"),
	TYPE_METHOD("__ge__", &default_seq___ge__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Sequence) >= (rhs)}"),
	TYPE_METHOD("__inplace_add__", &default_seq___inplace_add__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Sequence) += rhs}"),
	TYPE_METHOD("__inplace_mul__", &default_seq___inplace_mul__,
	            "(factor:?Dint)->?.\n"
	            "Alias for ${(this as Sequence) *= factor}"),

	/* Old function names/deprecated functions. */
	TYPE_METHOD("transform", &seq_map,
	            "(mapper:?DCallable)->?DSequence\n"
	            "Deprecated alias for ?#map"),
	TYPE_KWMETHOD("xch", &DeeMH_seq_xchitem,
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
	TYPE_METHOD("non_empty", &seq_non_empty_deprecated,
	            "->?Dbool\n"
	            "Deprecated alias for ?#isnonempty"),
	TYPE_METHOD("at", &seq_at_deprecated,
	            "(index:?Dint)->\n"
	            "Deprecated alias for ${this[index]}"),
	TYPE_METHOD(STR_get, &seq_at_deprecated,
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
	return_bool(Dee_TYPE(self) == &DeeSeq_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isempty(DeeObject *__restrict self) {
	int result = DeeSeq_OperatorBool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isnonempty(DeeObject *__restrict self) {
	int result = DeeSeq_OperatorBool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}



PRIVATE struct type_getset tpconst seq_getsets[] = {
	TYPE_GETTER("length", &DeeObject_SizeOb, "->?Dint\nAlias for ${##this}"),
	TYPE_GETSET_BOUND(STR_first,
	                  &default_seq_getfirst,
	                  &default_seq_delfirst,
	                  &default_seq_setfirst,
	                  &default_seq_boundfirst,
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
	                  &default_seq_getlast,
	                  &default_seq_dellast,
	                  &default_seq_setlast,
	                  &default_seq_boundlast,
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
	TYPE_GETTER("each", &DeeSeq_Each,
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
	TYPE_GETTER("ids", &SeqIds_New,
	            "->?S?Dint\n"
	            "Returns a special proxy object for accessing the ids of Sequence elements\n"
	            "This is equivalent to ${this.transform(x -\\> Object.id(x))}"),
	TYPE_GETTER("types", &SeqTypes_New,
	            "->?S?DType\n"
	            "Returns a special proxy object for accessing the types of Sequence elements\n"
	            "This is equivalent to ${this.transform(x -\\> type(x))}"),
	TYPE_GETTER("classes", &SeqClasses_New,
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
	TYPE_GETTER(STR_frozen, &DeeTuple_FromSequence,
	            "->?#Frozen\n"
	            "Returns a copy of @this Sequence, with all of its current elements, as well as "
	            /**/ "their current order frozen in place, constructing a snapshot of the Sequence's "
	            /**/ "current elements. - The actual type of Sequence returned is implementation- "
	            /**/ "and type- specific, but a guaranty is made that nothing no non-implementation-"
	            /**/ "specific functions/attributes (i.e. anything that doesn't match #C{__*__}) will "
	            /**/ "be able to change the elements of the returned sequence.\n"
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
	            "(obj,count:?Dint)->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Create a proxy-Sequence that yields @obj a total of @count times\n"
	            "The main purpose of this function is to construct large sequences "
	            /**/ "to be used as initializers for mutable sequences such as ?DList"),
	TYPE_METHOD("repeatseq", &seq_class_repeatseq,
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
generic_seq_assign(DeeObject *self, DeeObject *other) {
	return DeeObject_SetRange(self, Dee_None, Dee_None, other);
}


INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));


PRIVATE struct type_operator const seq_operators[] = {
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


/* Use sequence class members to expose all of the default sequence/iterator types for use by `rt' */
PRIVATE struct type_member tpconst seq_class_members[] = {
	TYPE_MEMBER_CONST("__SeqWithSizeAndGetItemIndex__", &DefaultSequence_WithSizeAndGetItemIndex_Type),
	TYPE_MEMBER_CONST("__SeqWithSizeAndGetItemIndexFast__", &DefaultSequence_WithSizeAndGetItemIndexFast_Type),
	TYPE_MEMBER_CONST("__SeqWithSizeAndTryGetItemIndex__", &DefaultSequence_WithSizeAndTryGetItemIndex_Type),
	TYPE_MEMBER_CONST("__SeqWithSizeAndGetItem__", &DefaultSequence_WithSizeAndGetItem_Type),
	TYPE_MEMBER_CONST("__SeqWithTSizeAndGetItem__", &DefaultSequence_WithTSizeAndGetItem_Type),
	TYPE_MEMBER_CONST("__SeqWithIter__", &DefaultSequence_WithIter_Type),
	TYPE_MEMBER_CONST("__SeqWithIterAndLimit__", &DefaultSequence_WithIterAndLimit_Type),
	TYPE_MEMBER_CONST("__SeqWithTIterAndLimit__", &DefaultSequence_WithTIterAndLimit_Type),
	TYPE_MEMBER_CONST("__IterWithGetItemIndex__", &DefaultIterator_WithGetItemIndex_Type),
	TYPE_MEMBER_CONST("__IterWithGetItemIndexPair__", &DefaultIterator_WithGetItemIndexPair_Type),
	TYPE_MEMBER_CONST("__IterWithSizeAndGetItemIndex__", &DefaultIterator_WithSizeAndGetItemIndex_Type),
	TYPE_MEMBER_CONST("__IterWithSizeAndGetItemIndexPair__", &DefaultIterator_WithSizeAndGetItemIndexPair_Type),
	TYPE_MEMBER_CONST("__IterWithSizeAndGetItemIndexFast__", &DefaultIterator_WithSizeAndGetItemIndexFast_Type),
	TYPE_MEMBER_CONST("__IterWithSizeAndGetItemIndexFastPair__", &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type),
	TYPE_MEMBER_CONST("__IterWithSizeAndTryGetItemIndex__", &DefaultIterator_WithSizeAndTryGetItemIndex_Type),
	TYPE_MEMBER_CONST("__IterWithSizeAndTryGetItemIndexPair__", &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type),
	TYPE_MEMBER_CONST("__IterWithGetItem__", &DefaultIterator_WithGetItem_Type),
	TYPE_MEMBER_CONST("__IterWithTGetItem__", &DefaultIterator_WithTGetItem_Type),
	TYPE_MEMBER_CONST("__IterWithSizeObAndGetItem__", &DefaultIterator_WithSizeObAndGetItem_Type),
	TYPE_MEMBER_CONST("__IterWithTSizeObAndGetItem__", &DefaultIterator_WithTSizeAndGetItem_Type),
	TYPE_MEMBER_CONST("__IterWithNextAndLimit__", &DefaultIterator_WithNextAndLimit_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndGetItemForSeq__", &DefaultIterator_WithIterKeysAndGetItemSeq_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndTGetItemForSeq__", &DefaultIterator_WithIterKeysAndTGetItemSeq_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndTryGetItemForSeq__", &DefaultIterator_WithIterKeysAndTryGetItemSeq_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndTTryGetItemForSeq__", &DefaultIterator_WithIterKeysAndTTryGetItemSeq_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndGetItemForMap__", &DefaultIterator_WithIterKeysAndGetItemMap_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndTGetItemForMap__", &DefaultIterator_WithIterKeysAndTGetItemMap_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndTryGetItemForMap__", &DefaultIterator_WithIterKeysAndTryGetItemMap_Type),
	TYPE_MEMBER_CONST("__IterWithIterKeysAndTTryGetItemForMap__", &DefaultIterator_WithIterKeysAndTTryGetItemMap_Type),
	TYPE_MEMBER_CONST("__IterWithForeach__", &DefaultIterator_WithForeach_Type),
	TYPE_MEMBER_CONST("__IterWithForeachPair__", &DefaultIterator_WithForeachPair_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateMap__", &DefaultIterator_WithEnumerateMap_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateIndexSeq__", &DefaultIterator_WithEnumerateIndexSeq_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateSeq__", &DefaultIterator_WithEnumerateSeq_Type),
	TYPE_MEMBER_CONST("__IterWithEnumerateIndexMap__", &DefaultIterator_WithEnumerateIndexMap_Type),
	TYPE_MEMBER_CONST("__IterWithNextAndCounterPair__", &DefaultIterator_WithNextAndCounterPair_Type),
	TYPE_MEMBER_CONST("__IterWithNextAndCounterAndLimitPair__", &DefaultIterator_WithNextAndCounterAndLimitPair_Type),
	TYPE_MEMBER_CONST("__IterWithNextAndUnpackFilter__", &DefaultIterator_WithNextAndUnpackFilter_Type),
	TYPE_MEMBER_CONST("__IterWithNextKey__", &DefaultIterator_WithNextKey),
	TYPE_MEMBER_CONST("__IterWithNextValue__", &DefaultIterator_WithNextValue),
	TYPE_MEMBER_CONST("__SeqReversedWithGetItemIndex__", &DefaultReversed_WithGetItemIndex_Type),
	TYPE_MEMBER_CONST("__SeqReversedWithGetItemIndexFast__", &DefaultReversed_WithGetItemIndexFast_Type),
	TYPE_MEMBER_CONST("__SeqReversedWithTryGetItemIndex__", &DefaultReversed_WithTryGetItemIndex_Type),
	TYPE_MEMBER_END
};


#ifdef CONFIG_HAVE_SET_OPERATORS_IN_SEQ
#define IF_HAVE_SET_OPERATORS(x) x
#else /* CONFIG_HAVE_SET_OPERATORS_IN_SEQ */
#define IF_HAVE_SET_OPERATORS(x) /* nothing */
#endif /* !CONFIG_HAVE_SET_OPERATORS_IN_SEQ */

/* `Sequence from deemon' */
PUBLIC DeeTypeObject DeeSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Sequence),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any Sequence "
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
	                         /**/ "${operator bool}|${return !!this;}&"
	                         /**/ "${operator size}¹²|${return !!##this;}&"
	                         /**/ "${operator iter}¹²|${"
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
	                         /**/ "	for (local item: this) {\n"
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
	                         /**/ "	yield this...;\n"
	                         /**/ "	yield other...;\n"
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
	                         /**/ "		yield this...;\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "hash->\n"
	                         "Returns the hash of all items of @this ?.\n"
	                         "#T{Requirements|Implementation~"
	                         /**/ "${operator hash}|${return this.operator hash();}&"
	                         /**/ "${operator iter}¹²|${"
	                         /**/ /**/ "local result = 0; // DEE_HASHOF_EMPTY_SEQUENCE\n"
	                         /**/ /**/ "for (local x: this)\n"
	                         /**/ /**/ "	result = deemon.hash(result, x);\n"
	                         /**/ /**/ "return result;"
	                         /**/ "}&"
	                         /**/ "${operator size}, ${operator getitem}¹²|${"
	                         /**/ /**/ "local size = ##this;\n"
	                         /**/ /**/ "local result = 0; // DEE_HASHOF_EMPTY_SEQUENCE\n"
	                         /**/ /**/ "for (local i: [:size]) {\n"
	                         /**/ /**/ "	local item;\n"
	                         /**/ /**/ "	try {\n"
	                         /**/ /**/ "		item = this[i];\n"
	                         /**/ /**/ "	} catch (UnboundItem) {\n"
	                         /**/ /**/ "		item = 0; // DEE_HASHOF_UNBOUND_ITEM\n"
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
	                         /**/ "${operator iter}¹²³|${"
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
	                         /**/ "${operator getitem}|${return this[index];}&"
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
	                         "\n"

	                         "#->\n"
	                         "Returns the length of @this Sequence, as determinable by enumeration\n"
	                         "#T{Requirements|Implementation~"
	                         /**/ "${operator size}|${return #this;}&"
	                         /**/ "${operator iter}¹²|${"
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
	                         /**/ "${operator contains}|${return item in this;}&"
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

	                         IF_HAVE_SET_OPERATORS("~->?DSet\n")
	                         IF_HAVE_SET_OPERATORS("Alias for ${(this as Set).operator ~ ()}. S.a. ?Aop:inv?DSet\n")
	                         IF_HAVE_SET_OPERATORS("\n")

	                         IF_HAVE_SET_OPERATORS("sub->?DSet\n")
	                         IF_HAVE_SET_OPERATORS("Alias for ${(this as Set).operator - (other)}. S.a. ?Aop:sub?DSet\n")
	                         IF_HAVE_SET_OPERATORS("\n")

	                         IF_HAVE_SET_OPERATORS("|->?DSet\n")
	                         IF_HAVE_SET_OPERATORS("Alias for ${(this as Set).operator | (other)}. S.a. ?Aop:or?DSet\n")
	                         IF_HAVE_SET_OPERATORS("\n")

	                         IF_HAVE_SET_OPERATORS("&->?DSet\n")
	                         IF_HAVE_SET_OPERATORS("Alias for ${(this as Set).operator & (other)}. S.a. ?Aop:and?DSet\n")
	                         IF_HAVE_SET_OPERATORS("\n")

	                         IF_HAVE_SET_OPERATORS("^->?DSet\n")
	                         IF_HAVE_SET_OPERATORS("Alias for ${(this as Set).operator ^ (other)}. S.a. ?Aop:xor?DSet\n")
	                         IF_HAVE_SET_OPERATORS("\n")

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
	                         /**/ "${operator getrange}|${return this[start:end];}&"
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
	                         /**/ "${operator iter}, ${operator size}¹²|${"
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
	                         /**/ "${operator delitem}|${del this[index];}"
	                         "}\n"
	                         "\n"

	                         "[]=->\n"
	                         "#tIntegerOverflow{The given @index is negative, or too large}"
	                         "#tIndexError{The given @index is out of bounds}"
	                         "Set the value of @index to @value\n"
	                         "#T{Requirements|Implementation~"
	                         /**/ "${operator setitem}|${this[index] = value;}"
	                         "}\n"
	                         "\n"

	                         "del[:]->\n"
	                         "#tIntegerOverflow{@start or @end are too large}"
	                         "#tSequenceError{@this Sequence cannot be resized}"
	                         "Delete, or unbind all items within the given range\n"
	                         "#T{Requirements|Implementation~"
	                         /**/ "${operator delrange}|${del this[start:end];}&"
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
	                         /**/ "${operator setrange}|${this[start:end] = values;}&"
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
	                         "#B{²}: Default implementation provided if sub-class matches requirements"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_SEQ << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1, /* Allow default-construction of Sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ &generic_seq_assign,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ &DeeSeq_OperatorBool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ &default_seq_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &seq_math,
	/* .tp_cmp           = */ &DeeSeq_OperatorCmp,
	/* .tp_seq           = */ &DeeSeq_OperatorSeq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ seq_methods,
	/* .tp_getsets       = */ seq_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ seq_class_methods,
	/* .tp_class_getsets = */ seq_class_getsets,
	/* .tp_class_members = */ seq_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ seq_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(seq_operators)
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


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_C */
