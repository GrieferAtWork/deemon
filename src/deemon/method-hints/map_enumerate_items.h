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
/* deemon.Mapping.__map_enumerate_items__()                             */
/************************************************************************/

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects.
 * Together with `__map_enumerate__', this API is used to implement `Mapping.enumerate()' */

__map_enumerate_items__(start?,end?)->?S?T2?O?O {
	DeeObject *start, *end = NULL;
	if (DeeArg_Unpack(argc, argv, "|oo:__map_enumerate_items__", &start, &end))
		goto err;
	if (end)
		return DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, start, end);
	return DeeObject_InvokeMethodHint(map_makeenumeration, self);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__map_enumerate_items__.map_makeenumeration([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$empty = "default__seq_makeenumeration__empty"}
%{$with__operator_iter = [[inherit_as(default__map_makeenumeration__with__map_operator_iter)]] "DeeObject_NewRef"}
%{$with__map_operator_iter = {
	return DeeSuper_New(&DeeMapping_Type, self); /* DefaultEnumeration__with__map_operator_iter__and__unpack */
}}
%{$with__map_iterkeys__and__map_operator_getitem = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem, self);
}}
%{$with__map_iterkeys__and__map_operator_trygetitem = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem, self);
}}
%{$with__map_enumerate = {
	return (DREF DeeObject *)DefaultEnumeration_New(&DefaultEnumeration__with__map_enumerate, self);
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}



[[wunused]] DREF DeeObject *
__map_enumerate_items__.map_makeenumeration_with_range([[nonnull]] DeeObject *self,
                                                       [[nonnull]] DeeObject *start,
                                                       [[nonnull]] DeeObject *end)
%{unsupported(auto)}
%{$empty = "default__seq_makeenumeration_with_range__empty"}
%{$with__map_operator_iter = {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack, self, start, end);
}}
%{$with__map_iterkeys__and__map_operator_getitem = {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem, self, start, end);
}}
%{$with__map_iterkeys__and__map_operator_trygetitem = {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem, self, start, end);
}}
%{$with__map_enumerate_range = {
	return (DREF DeeObject *)DefaultEnumerationWithFilter_New(&DefaultEnumerationWithFilter__with__map_enumerate_range, self, start, end);
}}
/*%{$with__map_makeenumeration = {
	// TODO
}}*/ {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return LOCAL_CALLATTR(self, 2, args);
}





map_makeenumeration = {
	DeeMH_map_enumerate_t map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
	if (map_enumerate == &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem) {
		DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
		DeeMH_map_iterkeys_t map_iterkeys = REQUIRE(map_iterkeys);
		if (map_iterkeys == &default__map_iterkeys__empty)
			return &$empty;
		if (map_iterkeys == &default__map_iterkeys__with__map_operator_iter) {
return__with__map_operator_iter:
			if (DeeType_RequireSupportedNativeOperator(THIS_TYPE, iter) == REQUIRE(map_operator_iter))
				return &$with__operator_iter;
			return &$with__map_operator_iter;
		}

		map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
		if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
			return &$empty;
		if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_operator_getitem)
			return &$with__map_iterkeys__and__map_operator_getitem;
		return &$with__map_iterkeys__and__map_operator_trygetitem;
	}
	if (map_enumerate) {
		/* The "$with__map_enumerate" impl is super-inefficient.
		 * See if we can use one of the others, even if that one would work. */
		DeeMH_map_operator_iter_t map_operator_iter = REQUIRE(map_operator_iter);
		if (map_operator_iter == &default__map_operator_iter__empty)
			return &$empty;
		if (map_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem)
			return &$with__map_iterkeys__and__map_operator_trygetitem;
		if (map_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
			return &$with__map_iterkeys__and__map_operator_getitem;
		if (map_operator_iter != &default__seq_operator_iter__with__map_enumerate)
			goto return__with__map_operator_iter;
		return &$with__map_enumerate;
	}
};

map_makeenumeration_with_range = {
	DeeMH_map_makeenumeration_t map_makeenumeration = REQUIRE(map_makeenumeration);
	if (map_makeenumeration == &default__map_makeenumeration__empty)
		return &$empty;
	if (map_makeenumeration == &default__map_makeenumeration__with__operator_iter ||
	    map_makeenumeration == &default__map_makeenumeration__with__map_operator_iter)
		return &$with__map_operator_iter;
	if (map_makeenumeration == &default__map_makeenumeration__with__map_iterkeys__and__map_operator_getitem)
		return &$with__map_iterkeys__and__map_operator_getitem;
	if (map_makeenumeration == &default__map_makeenumeration__with__map_iterkeys__and__map_operator_trygetitem)
		return &$with__map_iterkeys__and__map_operator_trygetitem;
	if (map_makeenumeration == &default__map_makeenumeration__with__map_enumerate)
		return &$with__map_enumerate_range;
	/*if (map_makeenumeration) // TODO
		return &$with__map_makeenumeration;*/
};
