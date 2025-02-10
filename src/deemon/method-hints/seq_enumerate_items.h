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
%{unsupported(auto)}
%{$empty = {
	return_empty_seq;
}}
%{$with__seq_operator_size__and__getitem_index_fast = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast, self);
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index, self);
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index, self);
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem, self);
}}
%{$with__seq_operator_getitem_index = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_getitem_index, self);
}}
%{$with__seq_operator_getitem = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_getitem, self);
}}
%{$with__seq_operator_iter__and__counter = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_operator_iter__and__counter, self);
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration_with_intrange([[nonnull]] DeeObject *__restrict self,
                                                          size_t start, size_t end)
%{unsupported(auto)}
%{$empty = {
	return_empty_seq;
}}
%{$with__seq_makeenumeration_with_range = {
	DREF DeeObject *result, *startob, *endob;
	startob = DeeInt_NewSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = CALL_DEPENDENCY(seq_makeenumeration_with_range, self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__getitem_index_fast = {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast, self, start, end);
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index, self, start, end);
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index, self, start, end);
}}
%{$with__seq_operator_getitem_index = {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index, self, start, end);
}}
%{$with__seq_operator_iter__and__counter = {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter, self, start, end);
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration_with_range([[nonnull]] DeeObject *self,
                                                       [[nonnull]] DeeObject *start,
                                                       [[nonnull]] DeeObject *end)
%{unsupported(auto)}
%{$empty = {
	return_empty_seq;
}}
%{$with__seq_makeenumeration_with_intrange = {
	size_t start_index, end_index;
	if (DeeObject_AsSize(start, &start_index))
		goto err;
	if (DeeObject_AsSize(end, &end_index))
		goto err;
	return CALL_DEPENDENCY(seq_makeenumeration_with_intrange, self, start_index, end_index);
err:
	return NULL;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem, self, start, end);
}}
%{$with__seq_operator_getitem = {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__seq_operator_getitem, self, start, end);
}} {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return LOCAL_CALLATTR(self, 2, args);
}


seq_makeenumeration = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
	if (seq_operator_foreach)
		return &$with__seq_operator_iter__and__counter;
};

seq_makeenumeration_with_intrange = {
	DeeMH_seq_makeenumeration_t seq_makeenumeration;
	if (REQUIRE_NODEFAULT(seq_makeenumeration_with_range))
		return &$with__seq_makeenumeration_with_range;

	seq_makeenumeration = REQUIRE(seq_makeenumeration);
	if (seq_makeenumeration == &default__seq_makeenumeration__empty)
		return &$empty;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__getitem_index_fast)
		return &$with__seq_operator_size__and__getitem_index_fast;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_makeenumeration_with_range;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem)
		return &$with__seq_makeenumeration_with_range;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_iter__and__counter)
		return &$with__seq_operator_iter__and__counter;
};

seq_makeenumeration_with_range = {
	DeeMH_seq_makeenumeration_t seq_makeenumeration;
	if (REQUIRE_NODEFAULT(seq_makeenumeration_with_intrange))
		return &$with__seq_makeenumeration_with_intrange;

	seq_makeenumeration = REQUIRE(seq_makeenumeration);
	if (seq_makeenumeration == &default__seq_makeenumeration__empty)
		return &$empty;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__getitem_index_fast ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem_index)
		return &$with__seq_makeenumeration_with_intrange;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_iter__and__counter)
		return &$with__seq_makeenumeration_with_intrange;
};

