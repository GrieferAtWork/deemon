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

/************************************************************************/
/* deemon.Sequence.__seq_enumerate_items__()                            */
/************************************************************************/

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects.
 * Together with `__seq_enumerate__', this API is used to implement `Sequence.enumerate()' */

__seq_enumerate_items__(start?:?X2?Dint?O,end?:?X2?Dint?O)->?S?T2?Dint?O {
	size_t start, end;
	DeeObject *startob = NULL, *endob = NULL;
	if (DeeArg_Unpack(argc, argv, "|oo:__seq_enumerate_items__", &startob, &endob))
		goto err;
	if (endob) {
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end)))
			return DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end);
		return DeeSeq_InvokeMakeEnumerationWithRange(self, startob, endob);
	} else if (startob) {
		if (DeeObject_AsSize(startob, &start))
			goto err;
		return DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, (size_t)-1);
	}
	return DeeSeq_InvokeMakeEnumeration(self);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration_with_int_range([[nonnull]] DeeObject *__restrict self,
                                                           size_t start, size_t end)
%{unsupported(auto)} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration_with_range([[nonnull]] DeeObject *self,
                                                       [[nonnull]] DeeObject *start,
                                                       [[nonnull]] DeeObject *end)
%{unsupported(auto)} {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return LOCAL_CALLATTR(self, 2, args);
}


seq_makeenumeration = {
	/* TODO: All of this stuff shouldn't be using generic operators, but *specifically* sequence operators! */
	/* TODO: Also: the implementation variants here should be defined by magic! */
	if (DeeType_HasPrivateOperator(THIS_TYPE, OPERATOR_ITER)) {
		unsigned int seqclass;
		if (THIS_TYPE->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndGetItem) {
			return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem ||
		           THIS_TYPE->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault) {
			return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex ||
		           THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeObAndGetItem) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithCounterAndForeach ||
		           THIS_TYPE->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithCounterAndIter) {
			if (THIS_TYPE->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndGetItem ||
			    THIS_TYPE->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
			if (THIS_TYPE->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItem ||
			    THIS_TYPE->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault ||
			    THIS_TYPE->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItem ||
			    THIS_TYPE->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
			if (THIS_TYPE->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
			if (THIS_TYPE->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndex)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
			if (THIS_TYPE->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
			if (THIS_TYPE->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeObAndGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
			if (THIS_TYPE->tp_seq->tp_iter == &DeeSeq_DefaultIterWithGetItemIndex ||
			    THIS_TYPE->tp_seq->tp_iter == &DeeSeq_DefaultIterWithGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		} else if (THIS_TYPE->tp_seq->tp_enumerate == &DeeMap_DefaultEnumerateWithIter) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		}
		if (THIS_TYPE->tp_seq->tp_iterkeys && !DeeType_IsDefaultIterKeys(THIS_TYPE->tp_seq->tp_iterkeys)) {
			if (THIS_TYPE->tp_seq->tp_iterkeys == &DeeMap_DefaultIterKeysWithIter ||
			    THIS_TYPE->tp_seq->tp_iterkeys == &DeeMap_DefaultIterKeysWithIterDefault)
				return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
			if (THIS_TYPE->tp_seq->tp_iterkeys == &DeeObject_DefaultIterKeysWithEnumerate ||
			    THIS_TYPE->tp_seq->tp_iterkeys == &DeeObject_DefaultIterKeysWithEnumerateIndex)
				return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
			if (THIS_TYPE->tp_seq->tp_iterkeys == &DeeSeq_DefaultIterKeysWithSizeOb)
				return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
			if (THIS_TYPE->tp_seq->tp_iterkeys == &DeeSeq_DefaultIterKeysWithSize ||
			    THIS_TYPE->tp_seq->tp_iterkeys == &DeeSeq_DefaultIterKeysWithSizeDefault) {
				if (THIS_TYPE->tp_seq->tp_trygetitem && (!DeeType_IsDefaultTryGetItem(THIS_TYPE->tp_seq->tp_trygetitem) ||
				                                    !DeeType_IsDefaultTryGetItemIndex(THIS_TYPE->tp_seq->tp_trygetitem_index)))
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
				if (THIS_TYPE->tp_seq->tp_getitem && (!DeeType_IsDefaultGetItem(THIS_TYPE->tp_seq->tp_getitem) ||
				                                 !DeeType_IsDefaultGetItemIndex(THIS_TYPE->tp_seq->tp_getitem_index)))
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
				if (THIS_TYPE->tp_seq->tp_trygetitem)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
				if (THIS_TYPE->tp_seq->tp_getitem)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
			} else {
				if (THIS_TYPE->tp_seq->tp_trygetitem && !DeeType_IsDefaultTryGetItem(THIS_TYPE->tp_seq->tp_trygetitem))
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
				if (THIS_TYPE->tp_seq->tp_getitem && !DeeType_IsDefaultGetItem(THIS_TYPE->tp_seq->tp_getitem))
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
				if (THIS_TYPE->tp_seq->tp_trygetitem)
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
				if (THIS_TYPE->tp_seq->tp_getitem)
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
			}
		}
		seqclass = SEQ_CLASS;
		if (seqclass == Dee_SEQCLASS_SEQ) {
			if (DeeType_HasOperator(THIS_TYPE, OPERATOR_SIZE)) {
				if (THIS_TYPE->tp_seq->tp_getitem_index_fast)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
				if (DeeType_HasOperator(THIS_TYPE, OPERATOR_GETITEM)) {
					if (!DeeType_IsDefaultGetItemIndex(THIS_TYPE->tp_seq->tp_getitem_index))
						return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
					if (!DeeType_IsDefaultTryGetItemIndex(THIS_TYPE->tp_seq->tp_trygetitem_index))
						return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
					if (!DeeType_IsDefaultGetItem(THIS_TYPE->tp_seq->tp_getitem))
						return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
				}
			} else if (DeeType_HasOperator(THIS_TYPE, OPERATOR_GETITEM)) {
				if (!DeeType_IsDefaultGetItemIndex(THIS_TYPE->tp_seq->tp_getitem_index) ||
				    !DeeType_IsDefaultTryGetItemIndex(THIS_TYPE->tp_seq->tp_trygetitem_index) ||
				    !DeeType_IsDefaultGetItem(THIS_TYPE->tp_seq->tp_getitem))
					return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			}
			if (!DeeType_IsDefaultIter(THIS_TYPE->tp_seq->tp_iter))
				return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
		if (THIS_TYPE->tp_seq->tp_enumerate != NULL &&
		    THIS_TYPE->tp_seq->tp_enumerate != THIS_TYPE->tp_seq->tp_foreach_pair &&
		    !DeeType_IsDefaultEnumerate(THIS_TYPE->tp_seq->tp_enumerate)) {
			return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
		} else if (THIS_TYPE->tp_seq->tp_enumerate_index != NULL &&
		           !DeeType_IsDefaultEnumerateIndex(THIS_TYPE->tp_seq->tp_enumerate_index)) {
			return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
		}
		if (seqclass == Dee_SEQCLASS_SEQ) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		} else if (seqclass == Dee_SEQCLASS_MAP) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		} else if (seqclass == Dee_SEQCLASS_NONE) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
	}
};

seq_makeenumeration_with_int_range = {
	/* TODO: All of this stuff shouldn't be using generic operators, but *specifically* sequence operators! */
	/* TODO: Also: the implementation variants here should be defined by magic! */
	DeeMH_seq_makeenumeration_t seq_makeenumeration = REQUIRE(seq_makeenumeration);
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexFastAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndTryGetItemIndexAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeObAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithGetItemIndexAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndTryGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterAndCounter)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter;
	if (seq_makeenumeration == &DeeMap_DefaultMakeEnumerationWithIterAndUnpack)
		return &DeeMap_DefaultMakeEnumerationWithIntRangeWithIterAndUnpackAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithEnumerate)
		return &DeeMap_DefaultMakeEnumerationWithIntRangeWithEnumerateIndex;
};

seq_makeenumeration_with_range = {
	/* TODO: All of this stuff shouldn't be using generic operators, but *specifically* sequence operators! */
	/* TODO: Also: the implementation variants here should be defined by magic! */
	DeeMH_seq_makeenumeration_t seq_makeenumeration = REQUIRE(seq_makeenumeration);
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast ||
	    seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex ||
	    seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex ||
	    seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndTryGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterAndCounter)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithIterAndCounterAndFilter;
	if (seq_makeenumeration == &DeeMap_DefaultMakeEnumerationWithIterAndUnpack)
		return &DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithEnumerate)
		return &DeeMap_DefaultMakeEnumerationWithRangeWithEnumerateAndFilter;
};

