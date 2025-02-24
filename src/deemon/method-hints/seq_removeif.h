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
/* deemon.Sequence.removeif()                                           */
/************************************************************************/
[[kw, alias(Sequence.removeif -> "seq_removeif")]]
__seq_removeif__(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint {
	size_t result;
	DeeObject *should;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__should_start_end_max,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ ":removeif",
	                    &should, &start, &end, &max))
		goto err;
	result = CALL_DEPENDENCY(seq_removeif, self, should, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}



/* @return: (size_t)-1: Error */
[[wunused]] size_t
__seq_removeif__.seq_removeif([[nonnull]] DeeObject *self,
                              [[nonnull]] DeeObject *should,
                              size_t start, size_t end, size_t max)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_removeall_with_key = {
	/* >> global final class SeqRemoveIfWithRemoveAllItem { operator == (other) -> other; };
	 * >> global final SeqRemoveIfWithRemoveAllItem_DummyInstance = SeqRemoveIfWithRemoveAllItem();
	 * >>
	 * >> class SeqRemoveIfWithRemoveAllItem { operator == (other) -> other; };
	 * >> return self.removeall(SeqRemoveIfWithRemoveAllItem_DummyInstance, start, end, max, key: x -> {
	 * >>     return x === SeqRemoveIfWithRemoveAllItem_DummyInstance ? x : should(x);
	 * >> }); */
	size_t result;
	DREF SeqRemoveIfWithRemoveAllKey *key;
	key = DeeObject_MALLOC(SeqRemoveIfWithRemoveAllKey);
	if unlikely(!key)
		goto err;
	Dee_Incref(should);
	key->sriwrak_should = should;
	DeeObject_Init(key, &SeqRemoveIfWithRemoveAllKey_Type);
	result = CALL_DEPENDENCY(seq_removeall_with_key, self,
	                         &SeqRemoveIfWithRemoveAllItem_DummyInstance,
	                         start, end, max, (DeeObject *)key);
	Dee_Decref_likely(key);
	return result;
err:
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index = {
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
			int should_remove;
			DREF DeeObject *pred_result;
			pred_result = DeeObject_Call(should, 1, &elem);
			Dee_Decref(elem);
			if unlikely(!pred_result)
				goto err;
			should_remove = DeeObject_BoolInherited(pred_result);
			if unlikely(should_remove < 0)
				goto err;
			if (should_remove) {
				/* Delete this one */
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
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ PCKuSIZ, should, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}


seq_removeif = {
	if (REQUIRE_NODEFAULT(seq_removeall_with_key))
		return &$with__seq_removeall_with_key;
	if (REQUIRE(seq_operator_delitem_index)) {
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported &&
		    REQUIRE_ANY(seq_operator_trygetitem_index) != &default__seq_operator_trygetitem_index__unsupported)
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index;
	}
};
