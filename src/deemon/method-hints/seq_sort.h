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
/* deemon.Sequence.sort()                                               */
/************************************************************************/
[[kw, alias(Sequence.sort)]]
[[docstring("(key:?DCallable=!N)->?Dbool\n"
            "(start:?Dint,end:?Dint,key:?DCallable=!N)->?Dbool")]]
__seq_sort__(size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N) {{
	int error;
#ifdef __OPTIMIZE_SIZE__
	if (!kw && argc == 1) {
		result = CALL_DEPENDENCY(seq_sort_with_key, self, argv[0]);
		goto check_result;
	}
#else /* __OPTIMIZE_SIZE__ */
	if (!kw) {
		size_t start, end;
		switch (argc) {
		case 0:
			error = CALL_DEPENDENCY(seq_sort, self, 0, (size_t)-1);
			goto check_error;
		case 1:
			error = CALL_DEPENDENCY(seq_sort_with_key, self, 0, (size_t)-1, argv[0]);
			goto check_error;
		case 2:
		case 3:
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &end))
				goto err;
			if (argc == 2) {
				error = CALL_DEPENDENCY(seq_sort, self, start, end);
			} else {
				error = CALL_DEPENDENCY(seq_sort_with_key, self, start, end, argv[2]);
			}
			goto check_error;
		default:
			err_invalid_argc("__seq_sort__", argc, 0, 3);
			goto err;
		}
		__builtin_unreachable();
	}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__seq_sort__", params: "
	size_t start = 0, size_t end = (size_t)-1, key:?DCsortable=!N
");]]]*/
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_sort__", &args))
		goto err;
/*[[[end]]]*/
	if (DeeNone_Check(args.key)) {
		error = CALL_DEPENDENCY(seq_sort, self, args.start, args.end);
	} else {
		error = CALL_DEPENDENCY(seq_sort_with_key, self, args.start, args.end, args.key);
	}
check_error:
	if unlikely(error)
		goto err;
	return_none;
err:
	return NULL;
}}



[[wunused]] int
__seq_sort__.seq_sort([[nonnull]] DeeObject *self,
                      size_t start, size_t end)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_sorted__and__seq_operator_setrange_index = {
	DREF DeeObject *sorted = CALL_DEPENDENCY(seq_sorted, self, start, end);
	int result = CALL_DEPENDENCY(seq_operator_setrange_index, self, start, end, sorted);
	Dee_Decref(sorted);
	return result;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index = {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}





[[wunused]] int
__seq_sort__.seq_sort_with_key([[nonnull]] DeeObject *self,
                               size_t start, size_t end,
                               [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_sorted_with_key__and__seq_operator_setrange_index = {
	DREF DeeObject *sorted = CALL_DEPENDENCY(seq_sorted_with_key, self, start, end, key);
	int result = CALL_DEPENDENCY(seq_operator_setrange_index, self, start, end, sorted);
	Dee_Decref(sorted);
	return result;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index = {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}





seq_sort = {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	if (REQUIRE_NODEFAULT(seq_sorted) &&
	    REQUIRE_ANY(seq_operator_setrange_index) != &default__seq_operator_setrange_index__unsupported)
		return &$with__seq_sorted__and__seq_operator_setrange_index;
	seq_operator_setitem_index = REQUIRE(seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &$empty;
		if ((seq_operator_size = REQUIRE_ANY(seq_operator_size)) != &default__seq_operator_size__unsupported &&
		    (seq_operator_getitem_index = REQUIRE_ANY(seq_operator_getitem_index)) != &default__seq_operator_getitem_index__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &$empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &$empty;
			return &$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index;
		}
	}
};


seq_sort_with_key = {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	if (REQUIRE_NODEFAULT(seq_sorted_with_key) &&
	    REQUIRE_ANY(seq_operator_setrange_index) != &default__seq_operator_setrange_index__unsupported)
		return &$with__seq_sorted_with_key__and__seq_operator_setrange_index;
	seq_operator_setitem_index = REQUIRE(seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &$empty;
		if ((seq_operator_size = REQUIRE_ANY(seq_operator_size)) != &default__seq_operator_size__unsupported &&
		    (seq_operator_getitem_index = REQUIRE_ANY(seq_operator_getitem_index)) != &default__seq_operator_getitem_index__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &$empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &$empty;
			return &$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index;
		}
	}
};
