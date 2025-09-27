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
/* deemon.Sequence.operator size()                                      */
/************************************************************************/
__seq_size__()->?Dint {
	return CALL_DEPENDENCY(seq_operator_sizeob, self);
}



%[define(DEFINE_default_seq_operator_sizeob_with_seq_enumerate_cb =
#ifndef DEFINED_default_seq_operator_sizeob_with_seq_enumerate_cb
#define DEFINED_default_seq_operator_sizeob_with_seq_enumerate_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_operator_sizeob_with_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	DREF DeeObject **p_lastindex = (DREF DeeObject **)arg;
	(void)value;
	Dee_Incref(index);
	Dee_Decref(*p_lastindex);
	*p_lastindex = index;
	return 0;
}
#endif /* !DEFINED_default_seq_operator_sizeob_with_seq_enumerate_cb */
)]


[[operator([Sequence, Set, Mapping]: tp_seq->tp_sizeob)]]
[[wunused]]
DREF DeeObject *__seq_size__.seq_operator_sizeob([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator size"))}
%{$none = return_none}
%{$empty = return DeeInt_NewZero()}
%{using seq_operator_size: {
	size_t seqsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return DeeInt_NewSize(seqsize);
err:
	return NULL;
}}
%{$with__seq_enumerate = [[prefix(DEFINE_default_seq_operator_sizeob_with_seq_enumerate_cb)]] {
	Dee_ssize_t status;
	DREF DeeObject *result = DeeNone_NewRef();
	status = CALL_DEPENDENCY(seq_enumerate, self,
	                         &default_seq_operator_sizeob_with_seq_enumerate_cb,
	                         (void *)&result);
	if unlikely(status < 0)
		goto err_r;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return DeeInt_NewZero();
	}
	if (DeeObject_Inc(&result))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}


%[define(DEFINE_default_seq_size_with_foreach_cb =
#ifndef DEFINED_default_seq_size_with_foreach_cb
#define DEFINED_default_seq_size_with_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return 1;
}
#endif /* !DEFINED_default_seq_size_with_foreach_cb */
)]

%[define(DEFINE_default_seq_size_with_foreach_pair_cb =
#ifndef DEFINED_default_seq_size_with_foreach_pair_cb
#define DEFINED_default_seq_size_with_foreach_pair_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_size_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	(void)arg;
	(void)key;
	(void)value;
	return 1;
}
#endif /* !DEFINED_default_seq_size_with_foreach_pair_cb */
)]


%[define(DEFINE_default_seq_size_with_seq_enumerate_index_cb =
#ifndef DEFINED_default_seq_size_with_seq_enumerate_index_cb
#define DEFINED_default_seq_size_with_seq_enumerate_index_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_size_with_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	(void)value;
	*(size_t *)arg = index;
	return 0;
}
#endif /* !DEFINED_default_seq_size_with_seq_enumerate_index_cb */
)]


[[operator([Sequence, Set, Mapping]: tp_seq->tp_size)]]
[[wunused]]
size_t __seq_size__.seq_operator_size([[nonnull]] DeeObject *__restrict self)
// NOTE: The "unsupported"-impl here is still needed so other hints can
//       differentiate between "$unsupported" and "$with__seq_operator_sizeob"
%{unsupported(auto("operator size"))}
%{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_size_with_foreach_cb)]] {
	return (size_t)CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_size_with_foreach_cb, NULL);
}}
%{$with__seq_operator_foreach_pair = [[prefix(DEFINE_default_seq_size_with_foreach_pair_cb)]] {
	return (size_t)CALL_DEPENDENCY(seq_operator_foreach_pair, self, &default_seq_size_with_foreach_pair_cb, NULL);
}}
%{using seq_operator_sizeob: {
	DREF DeeObject *sizeob;
	sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}}
%{$with__set_operator_sizeob = {
	DREF DeeObject *sizeob;
	sizeob = CALL_DEPENDENCY(set_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}}
%{$with__map_operator_sizeob = {
	DREF DeeObject *sizeob;
	sizeob = CALL_DEPENDENCY(map_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_seq_size_with_foreach_pair_cb)]] {
	return (size_t)CALL_DEPENDENCY(map_enumerate, self, &default_seq_size_with_foreach_pair_cb, NULL);
}}
%{$with__seq_operator_iter = {
	size_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterAdvance(iter, (size_t)-1);
	Dee_Decref_likely(iter);
	return result;
err:
	return (size_t)-1;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_size_with_seq_enumerate_index_cb)]] {
	Dee_ssize_t status;
	size_t result = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self,
	                         &default_seq_size_with_seq_enumerate_index_cb,
	                         (void *)&result, 0, (size_t)-1);
	ASSERT(status == -1 || status == 0);
	if unlikely(status)
		goto err;
	++result;
	if unlikely(result == (size_t)-1)
		DeeRT_ErrIntegerOverflowU(result, (size_t)-2);
	return result;
err:
	return (size_t)-1;
}} = $with__seq_operator_sizeob;


seq_operator_sizeob = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE(seq_operator_size);
	if (seq_operator_size) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (seq_operator_size == &default__seq_operator_size__with__set_operator_sizeob ||
		    seq_operator_size == REQUIRE_NODEFAULT(set_operator_size))
			return REQUIRE(set_operator_sizeob);
		if (seq_operator_size == &default__seq_operator_size__with__map_operator_sizeob ||
		    seq_operator_size == REQUIRE_NODEFAULT(map_operator_size))
			return REQUIRE(map_operator_sizeob);
		if (seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index) {
			if (REQUIRE_NODEFAULT(seq_enumerate))
				return &$with__seq_enumerate;
		}
		return &$with__seq_operator_size;
	}
};

seq_operator_size = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_set_operator_size_t set_operator_size;
	DeeMH_map_operator_size_t map_operator_size;
	if ((set_operator_size = REQUIRE_NODEFAULT(set_operator_size)) != NULL)
		return set_operator_size;
	if ((map_operator_size = REQUIRE_NODEFAULT(map_operator_size)) != NULL)
		return map_operator_size;
	if (REQUIRE_NODEFAULT(seq_operator_sizeob))
		return &$with__seq_operator_sizeob;
	if (REQUIRE_NODEFAULT(set_operator_sizeob))
		return &$with__set_operator_sizeob;
	if (REQUIRE_NODEFAULT(map_operator_sizeob))
		return &$with__map_operator_sizeob;

	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_foreach_pair) {
		DeeMH_seq_operator_foreach_pair_t seq_operator_foreach_pair = REQUIRE(seq_operator_foreach_pair);
		if (seq_operator_foreach_pair == &default__seq_operator_foreach_pair__with__seq_operator_iter)
			return &$with__seq_operator_iter;
		return &$with__seq_operator_foreach_pair;
	}
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate ||
	    seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &$with__seq_operator_iter;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};
