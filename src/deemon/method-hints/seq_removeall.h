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
/* deemon.Sequence.removeall()                                          */
/************************************************************************/
[[kw, alias(Sequence.removeall -> "seq_removeall")]]
__seq_removeall__(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint {
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_max_key,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ "o:removeall",
	                    &item, &start, &end, &max, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? CALL_DEPENDENCY(seq_removeall_with_key, self, item, start, end, max, key)
	         : CALL_DEPENDENCY(seq_removeall, self, item, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}





/* @return: (size_t)-1: Error */
[[wunused]] size_t
__seq_removeall__.seq_removeall([[nonnull]] DeeObject *self,
                                [[nonnull]] DeeObject *item,
                                size_t start, size_t end, size_t max)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_removeif = {
	/* >> return self.removeif(x -> deemon.equals(item, x), start, end, max); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicate *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicate);
	if unlikely(!pred)
		goto err;
	Dee_Incref(item);
	pred->srwrip_item = item;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicate_Type);
	result = DeeSeq_InvokeRemoveIf(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_remove = {
	PRELOAD_DEPENDENCY(seq_remove)
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(!max)
		goto done;
	selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	for (;;) {
		int temp = CALL_DEPENDENCY(seq_remove, self, item, start, end);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (result >= max)
			break;
		if (sequence_size_changes_after_delitem == -1) {
			size_t new_selfsize = CALL_DEPENDENCY(seq_operator_size, self);
			if unlikely(new_selfsize == (size_t)-1)
				goto err;
			sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
		}
		if (sequence_size_changes_after_delitem) {
			--end;
			if (start >= end)
				break;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return result;
err:
	return (size_t)-1;
}}
%{$with__seq_remove__once = { /* When "self" is a set */
	return max ? (size_t)(Dee_ssize_t)CALL_DEPENDENCY(seq_remove, self, item, start, end) : 0;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	PRELOAD_DEPENDENCY(seq_operator_delitem_index)
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(start >= end)
		return 0;
	selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, start);
		if unlikely(!elem)
			goto err;
		if (elem != ITER_DONE) {
			int equal;
			equal = DeeObject_TryCompareEq(item, elem);
			Dee_Decref(elem);
			if unlikely(equal == Dee_COMPARE_ERR)
				goto err;
			if (equal == 0) {
				/* Found one! (delete it) */
				if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = CALL_DEPENDENCY(seq_operator_size, self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ PCKuSIZ, item, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}


/* @return: 0 : Item was not removealld
 * @return: 1 : Item was removealld
 * @return: -1: Error */
[[wunused]] size_t
__seq_removeall__.seq_removeall_with_key([[nonnull]] DeeObject *self,
                                         [[nonnull]] DeeObject *item,
                                         size_t start, size_t end, size_t max,
                                         [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_removeif = {
	/* >> local keyedElem = key(item);
	 * >> return !!self.removeallif(x -> deemon.equals(keyedElem, key(x)), start, end, max); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicateWithKey *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicateWithKey);
	if unlikely(!pred)
		goto err;
	pred->srwripwk_item = DeeObject_Call(key, 1, &item);
	if unlikely(!pred->srwripwk_item)
		goto err_pred;
	Dee_Incref(key);
	pred->srwripwk_key = key;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicateWithKey_Type);
	result = CALL_DEPENDENCY(seq_removeif, self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err_pred:
	DeeObject_FREE(pred);
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_remove_with_key = {
	PRELOAD_DEPENDENCY(seq_remove_with_key)
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(!max)
		goto done;
	selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	for (;;) {
		int temp = CALL_DEPENDENCY(seq_remove_with_key, self, item, start, end, key);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (result >= max)
			break;
		if (sequence_size_changes_after_delitem == -1) {
			size_t new_selfsize = CALL_DEPENDENCY(seq_operator_size, self);
			if unlikely(new_selfsize == (size_t)-1)
				goto err;
			sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
		}
		if (sequence_size_changes_after_delitem) {
			--end;
			if (start >= end)
				break;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return result;
err:
	return (size_t)-1;
}}
%{$with__seq_remove_with_key__once = { /* When "self" is a set */
	return max ? (size_t)(Dee_ssize_t)CALL_DEPENDENCY(seq_remove_with_key, self, item, start, end, key) : 0;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	PRELOAD_DEPENDENCY(seq_operator_delitem_index)
	int sequence_size_changes_after_delitem = -1;
	size_t result = 0;
	size_t selfsize;
	if unlikely(start >= end)
		return 0;
	selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start >= end)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	do {
		DREF DeeObject *elem;
		elem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, start);
		if unlikely(!elem)
			goto err_item;
		if (elem != ITER_DONE) {
			int equal;
			equal = DeeObject_TryCompareKeyEq(item, elem, key);
			Dee_Decref(elem);
			if unlikely(equal == Dee_COMPARE_ERR)
				goto err_item;
			if (equal == 0) {
				/* Found one! (delete it) */
				if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, start))
					goto err_item;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = CALL_DEPENDENCY(seq_operator_size, self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err_item;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err_item;
	} while (start < end);
	Dee_Decref(item);
	return result;
err_item:
	Dee_Decref(item);
err:
	return (size_t)-1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ PCKuSIZ "o", item, start, end, max, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

seq_removeall = {
	if (REQUIRE_NODEFAULT(seq_removeif))
		return &$with__seq_removeif;
	if (REQUIRE_NODEFAULT(seq_remove)) {
		if (Dee_SEQCLASS_ISSETORMAP(SEQ_CLASS))
			return &$with__seq_remove__once;
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size__and__seq_remove;
	}
	if (REQUIRE(seq_operator_delitem_index)) {
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported &&
		    REQUIRE_ANY(seq_operator_trygetitem_index) != &default__seq_operator_trygetitem_index__unsupported)
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index;
	}
};

seq_removeall_with_key = {
	if (REQUIRE_NODEFAULT(seq_removeif))
		return &$with__seq_removeif;
	if (REQUIRE_NODEFAULT(seq_remove_with_key)) {
		if (Dee_SEQCLASS_ISSETORMAP(SEQ_CLASS))
			return &$with__seq_remove_with_key__once;
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size__and__seq_remove_with_key;
	}
	if (REQUIRE(seq_operator_delitem_index)) {
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported &&
		    REQUIRE_ANY(seq_operator_trygetitem_index) != &default__seq_operator_trygetitem_index__unsupported)
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index;
	}
};
