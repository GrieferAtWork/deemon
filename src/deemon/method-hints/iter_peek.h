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

/************************************************************************/
/* deemon.Iterator.peek()                                               */
/************************************************************************/
[[alias(Iterator.peek)]]
__iter_peek__(def?)->?O {
	DREF DeeObject *result = CALL_DEPENDENCY(iter_peek, self);
	if (result == ITER_DONE) {
		result = def;
		if (result) {
			Dee_Incref(result);
		} else {
			DeeError_Throw(&DeeError_StopIteration_instance);
		}
	}
	return result;
}


%[define(DEFINE_iterator_dummy =
#ifndef DEFINED_iterator_dummy
#define DEFINED_iterator_dummy
PRIVATE DeeObject iterator_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };
PRIVATE DeeObject *tpconst iterator_dummy_vec[1] = { &iterator_dummy };
#endif /* !DEFINED_iterator_dummy */
)]

/* @return: ITER_DONE: Iterator is exhausted */
[[wunused]] DREF DeeObject *
__iter_peek__.iter_peek([[nonnull]] DeeObject *self)
%{unsupported(err_iter_unsupportedf(self, "peek"))}
%{$none = return_none}
%{$empty = ITER_DONE}
%{$with__operator_copy__and__operator_next = {
	DREF DeeObject *result;
	DREF DeeObject *copy = DeeObject_Copy(self);
	if unlikely(!copy)
		goto err;
	result = DeeObject_IterNext(copy);
	Dee_Decref_unlikely(copy);
	return result;
err:
	return NULL;
}}
%{$with__operator_next__and__iter_revert = {
	DREF DeeObject *result = DeeObject_IterNext(self);
	if (ITER_ISOK(result)) {
		size_t step = CALL_DEPENDENCY(iter_revert, self, 1);
		if unlikely(step == (size_t)-1)
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}}
%{$with__iter_getindex__and_operator_next__and__iter_setindex = {
	DREF DeeObject *result;
	size_t old_index = CALL_DEPENDENCY(iter_getindex, self);
	if unlikely(old_index == (size_t)-1)
		goto err;
	result = DeeObject_IterNext(self);
	if (ITER_ISOK(result)) {
		/* Restore iterator index */
		if (CALL_DEPENDENCY(iter_setindex, self, old_index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
[[prefix(DEFINE_iterator_dummy)]] {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, iterator_dummy_vec);
	if (result == &iterator_dummy) {
		Dee_DecrefNokill(&iterator_dummy);
		result = ITER_DONE;
	}
	return result;
}


iter_peek = {
	if (DeeType_RequireSupportedNativeOperator(THIS_TYPE, iter_next)) {
		DeeMH_iter_getindex_t iter_getindex;
		if (REQUIRE_NODEFAULT(iter_revert))
			return &$with__operator_next__and__iter_revert;
		if ((iter_getindex = REQUIRE(iter_getindex)) != NULL) {
			if (iter_getindex == &default__iter_getindex__empty)
				return &$empty;
			if (iter_getindex == &default__iter_getindex__with__iter_getseq__and__iter_operator_compare_eq &&
			    DeeType_IsCopyable(THIS_TYPE))
				return &$with__operator_copy__and__operator_next;
			if (iter_getindex) {
				DeeMH_iter_setindex_t iter_setindex = REQUIRE(iter_setindex);
				if (iter_setindex == &default__iter_setindex__empty)
					return &$empty;
				if (iter_setindex == &default__iter_setindex__with__iter_getseq__and__iter_operator_assign &&
				    DeeType_IsCopyable(THIS_TYPE))
					return &$with__operator_copy__and__operator_next;
				return &$with__iter_getindex__and_operator_next__and__iter_setindex;
			}
		}
		if (DeeType_IsCopyable(THIS_TYPE))
			return &$with__operator_copy__and__operator_next;
	}
};
