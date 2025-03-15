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
	size_t start_index, end_index;
	if (end) {
		if ((DeeInt_Check(start) && DeeInt_Check(end)) &&
		    (DeeInt_TryAsSize(start, &start_index) && DeeInt_TryAsSize(end, &end_index)))
			return CALL_DEPENDENCY(seq_makeenumeration_with_intrange, self, start_index, end_index);
		return CALL_DEPENDENCY(seq_makeenumeration_with_range, self, start, end);
	} else if (start) {
		if (DeeObject_AsSize(start, &start_index))
			goto err;
		return CALL_DEPENDENCY(seq_makeenumeration_with_intrange, self, start_index, (size_t)-1);
	}
	return CALL_DEPENDENCY(seq_makeenumeration, self);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return DeeSeq_NewEmpty()}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
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
}}
%{$with__seq_enumerate = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__seq_enumerate, self);
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}



[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration_with_range([[nonnull]] DeeObject *self,
                                                       [[nonnull]] DeeObject *start,
                                                       [[nonnull]] DeeObject *end)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return DeeSeq_NewEmpty()}
%{using seq_makeenumeration_with_intrange: {
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





[[wunused]] DREF DeeObject *
__seq_enumerate_items__.seq_makeenumeration_with_intrange([[nonnull]] DeeObject *__restrict self,
                                                          size_t start, size_t end)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return DeeSeq_NewEmpty()}
%{using seq_makeenumeration_with_range: {
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
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
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
}}
%{$with__seq_enumerate_index = {
	return (DREF DeeObject *)DefaultEnumerationWithIntFilter_New(&DefaultEnumerationWithIntFilter__with__seq_enumerate_index, self, start, end);
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

seq_makeenumeration = {
	DeeMH_seq_enumerate_t seq_enumerate = REQUIRE(seq_enumerate);
	if (seq_enumerate == &default__seq_enumerate__empty)
		return &$empty;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter)
		return &$with__seq_operator_iter__and__counter;
	if (seq_enumerate) {
		/* The "$with__seq_enumerate" impl is super-inefficient
		 * See if we can *maybe* still use one of the "$with__*size*__and__*getitem*" impls. */
		DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (seq_operator_size != &default__seq_operator_size__unsupported &&
		    seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach &&
		    seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach_pair &&
		    seq_operator_size != &default__seq_operator_size__with__map_enumerate) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
			if (seq_operator_getitem_index) {
				/* Yes! We *do* also have a getitem operator!
				 * Now just make sure that it's O(1), and then we can actually use it! */
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
					return &$empty;
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem) {
					DeeMH_seq_operator_getitem_t seq_operator_getitem = REQUIRE(seq_operator_getitem);
					if (seq_operator_getitem == &default__seq_operator_getitem__empty)
						return &$empty;
					return &$with__seq_operator_sizeob__and__seq_operator_getitem;
				}
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
					goto use__seq_enumerate;
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
					goto use__seq_enumerate;
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
					return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
				if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
					return &$with__seq_operator_size__and__operator_getitem_index_fast;
				return &$with__seq_operator_size__and__seq_operator_getitem_index;
			}
		}
use__seq_enumerate:
		return &$with__seq_enumerate;
	}
};

seq_makeenumeration_with_range = {
	DeeMH_seq_makeenumeration_t seq_makeenumeration;
	if (REQUIRE_NODEFAULT(seq_makeenumeration_with_intrange))
		return &$with__seq_makeenumeration_with_intrange;
	seq_makeenumeration = REQUIRE(seq_makeenumeration);

	if (seq_makeenumeration == &default__seq_makeenumeration__empty)
		return &$empty;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_iter__and__counter ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_enumerate)
		return &$with__seq_makeenumeration_with_intrange;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
};


seq_makeenumeration_with_intrange = {
	DeeMH_seq_makeenumeration_t seq_makeenumeration;
	if (REQUIRE_NODEFAULT(seq_makeenumeration_with_range))
		return &$with__seq_makeenumeration_with_range;

	seq_makeenumeration = REQUIRE(seq_makeenumeration);
	if (seq_makeenumeration == &default__seq_makeenumeration__empty)
		return &$empty;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem)
		return &$with__seq_makeenumeration_with_range;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_iter__and__counter)
		return &$with__seq_operator_iter__and__counter;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_enumerate)
		return &$with__seq_enumerate_index;
};

