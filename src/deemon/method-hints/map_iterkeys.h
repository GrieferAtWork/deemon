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
/* deemon.Mapping.__iterkeys__()                                       */
/************************************************************************/
__map_iterkeys__()->?DIterator {
	if (DeeArg_Unpack(argc, argv, ":__map_iterkeys__"))
		goto err;
	return CALL_DEPENDENCY(map_iterkeys, self);
err:
	return NULL;
}



[[wunused]] DREF DeeObject *
__map_iterkeys__.map_iterkeys([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$empty = "default__set_operator_iter__empty"}
%{$with__map_enumerate = {
	/* TODO: Custom iterator type that uses "tp_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}}
%{$with__set_operator_iter = {
	/* NOTE: This only works when the mapping can't have unbound keys! */
	DeeTypeObject *itertyp;
	DREF DefaultIterator_PairSubItem *result;
	result = DeeObject_MALLOC(DefaultIterator_PairSubItem);
	if unlikely(!result)
		goto err;
	result->dipsi_iter = CALL_DEPENDENCY(set_operator_iter, self);
	if unlikely(!result->dipsi_iter)
		goto err_r;
	itertyp = Dee_TYPE(result->dipsi_iter);
	if unlikely((!itertyp->tp_iterator ||
	             !itertyp->tp_iterator->tp_nextkey) &&
	            !DeeType_InheritIterNext(itertyp))
		goto err_r_iter_no_next;
	ASSERT(itertyp->tp_iterator);
	ASSERT(itertyp->tp_iterator->tp_nextkey);
	result->dipsi_next = itertyp->tp_iterator->tp_nextkey;
	DeeObject_Init(result, &DefaultIterator_WithNextKey);
	return (DREF DeeObject *)result;
err_r_iter_no_next:
	Dee_Decref(result->dipsi_iter);
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}}
{
	return LOCAL_CALLATTR(self, 0, NULL);
}

map_iterkeys = {
	DeeMH_map_enumerate_t map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
	if (map_enumerate != REQUIRE(set_operator_foreach_pair)) {
		DeeMH_set_operator_iter_t set_operator_iter = REQUIRE(set_operator_iter);
		if (set_operator_iter == &default__set_operator_iter__empty)
			return &$empty;
		if (set_operator_iter)
			return &$with__set_operator_iter;
	}
	if (map_enumerate)
		return &$with__map_enumerate;
};
