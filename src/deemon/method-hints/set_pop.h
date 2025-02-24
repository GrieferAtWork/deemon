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
/* deemon.Set.pop()                                                     */
/************************************************************************/
[[alias(Set.pop -> "set_pop")]]
__set_pop__(def?)->?O {
	DeeObject *def = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:__set_pop__", &def))
		goto err;
	return def ? CALL_DEPENDENCY(set_pop_with_default, self, def)
	           : CALL_DEPENDENCY(set_pop, self);
err:
	return NULL;
}



[[wunused]] DREF DeeObject *
__set_pop__.set_pop([[nonnull]] DeeObject *self)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = {
	err_empty_sequence(self);
	return NULL;
}}
%{$with__seq_trygetfirst__and__set_remove = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetfirst, self);
	if unlikely(!ITER_ISOK(result)) {
		if (result == ITER_DONE)
			err_empty_sequence(self);
		goto err;
	}
	if unlikely(DeeSet_InvokeRemove(self, result) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{$with__map_popitem = {
	DREF DeeObject *result = CALL_DEPENDENCY(map_popitem, self);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		err_empty_sequence(self);
		goto err;
	}
	return result;
err:
	return NULL;
}}
%{$with__seq_pop = {
	return CALL_DEPENDENCY(seq_pop, self, -1);
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}




[[wunused]] DREF DeeObject *
__set_pop__.set_pop_with_default([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *default_)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference_(default_)}
%{$with__seq_trygetfirst__and__set_remove = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetfirst, self);
	if unlikely(!ITER_ISOK(result)) {
		if (result == ITER_DONE)
			return_reference_(default_);
		goto err;
	}
	if unlikely(DeeSet_InvokeRemove(self, result) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{$with__map_popitem = {
	DREF DeeObject *result = CALL_DEPENDENCY(map_popitem, self);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		return_reference_(default_);
	}
	return result;
err:
	return NULL;
}}
%{$with__seq_pop = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_pop, self, -1);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_ValueError))
			return_reference_(default_);
		goto err;
	}
	return result;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &default_);
}







set_pop = {
	DeeMH_seq_pop_t seq_pop;
	if (REQUIRE_NODEFAULT(map_popitem))
		return &$with__map_popitem;
	seq_pop = REQUIRE(seq_pop);
	if (seq_pop == &default__seq_pop__empty)
		return &$empty;
	if (seq_pop)
		return &$with__seq_pop;
};

set_pop_with_default = {
	DeeMH_seq_pop_t seq_pop;
	if (REQUIRE_NODEFAULT(map_popitem))
		return &$with__map_popitem;
	seq_pop = REQUIRE(seq_pop);
	if (seq_pop == &default__seq_pop__empty)
		return &$empty;
	if (seq_pop)
		return &$with__seq_pop;
};
