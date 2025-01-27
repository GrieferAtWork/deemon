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

/* !!!!!!!!!!!!!!!!!!
 * After making changes to this file (or any of the files %[include]-ed below), you must run:
 * >> deemon -F include/deemon/method-hints.h src/deemon/runtime/method-hint-defaults.h src/deemon/runtime/method-hint-defaults.c src/deemon/runtime/method-hints.h src/deemon/runtime/method-hints.c src/deemon/runtime/method-hint-select.h src/deemon/runtime/method-hint-select.c src/deemon/runtime/method-hint-wrappers.c src/deemon/runtime/strings.h
 */

%[include("seq_operator_bool.h")]
%[include("seq_operator_size.h")]
%[include("seq_operator_iter.h")]
%[include("seq_operator_iterkeys.h")]
%[include("seq_operator_enumerate.h")]

///* Common utility functions... */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), Dee_ssize_t, DCALL, seq_foreach_reverse, (DeeObject *__restrict self, Dee_foreach_t proc, void *arg)) /* [0..1] Not necessarily available! */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), Dee_ssize_t, DCALL, seq_enumerate_index_reverse, (DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) /* [0..1] Not necessarily available! */

%[include("seq_operator_getitem.h")]
%[include("seq_operator_delitem.h")]
%[include("seq_operator_setitem.h")]

%[include("seq_operator_getrange.h")]
%[include("seq_operator_delrange.h")]
%[include("seq_operator_setrange.h")]

%[include("seq_operator_hash.h")]
%[include("seq_operator_compare.h")]
%[include("seq_operator_compare_eq.h")]
%[include("seq_operator_trycompare_eq.h")]
%[include("seq_operator_cmp.h")]

//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_operator_inplace_add, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__seq_inplace_add__, "__seq_inplace_add__", "(rhs:?S?O)->?.")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_operator_inplace_mul, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__seq_inplace_mul__, "__seq_inplace_mul__", "(factor:?Dint)->?.")
//
//
///* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_makeenumeration, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_makeenumeration_with_int_range, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, seq_makeenumeration_with_range, (DeeObject *self, DeeObject *start, DeeObject *end))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_enumerate, "enumerate",
//                                     "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?S?T2?Dint?O\n"
//                                     "(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N")


%[include("seq_first.h")]
%[include("seq_last.h")]

// TODO: Sequence.cached
// TODO: Sequence.frozen

%[include("seq_any.h")]
%[include("seq_all.h")]

//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, seq_parity, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_parity_with_key, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, seq_parity_with_range, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 4)), int, DCALL, seq_parity_with_range_and_key, (DeeObject *self, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_parity, "parity", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_parity__, "__seq_parity__", seq_parity)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, seq_reduce, (DeeObject *self, DeeObject *combine))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, seq_reduce_with_init, (DeeObject *self, DeeObject *combine, DeeObject *init))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, seq_reduce_with_range, (DeeObject *self, DeeObject *combine, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), DREF DeeObject *, DCALL, seq_reduce_with_range_and_init, (DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_reduce, "reduce", "(combine:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,init?)->")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_reduce__, "__seq_reduce__", seq_reduce)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_min, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, seq_min_with_key, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_min_with_range, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 4)), DREF DeeObject *, DCALL, seq_min_with_range_and_key, (DeeObject *self, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_min, "min", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_min__, "__seq_min__", seq_min)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_max, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, seq_max_with_key, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_max_with_range, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 4)), DREF DeeObject *, DCALL, seq_max_with_range_and_key, (DeeObject *self, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_max, "max", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_max__, "__seq_max__", seq_max)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_sum, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_sum_with_range, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_sum, "sum", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_sum__, "__seq_sum__", seq_sum)
//
///* @return: * : Count
// * @return: (size_t)-1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_count, (DeeObject *self, DeeObject *item))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), size_t, DCALL, seq_count_with_key, (DeeObject *self, DeeObject *item, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_count_with_range, (DeeObject *self, DeeObject *item, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), size_t, DCALL, seq_count_with_range_and_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_count, "count", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_count__, "__seq_count__", seq_count)
//
///* @return: 0 : Not contained
// * @return: 1 : Is contained
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_contains, (DeeObject *self, DeeObject *item))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), int, DCALL, seq_contains_with_key, (DeeObject *self, DeeObject *item, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_contains_with_range, (DeeObject *self, DeeObject *item, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, seq_contains_with_range_and_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_contains, "contains", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, seq_operator_contains, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_contains__, "__seq_contains__", seq_contains)
//
///* Returns the first element (within the given range) where `match(elem)' is true. */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, seq_locate, (DeeObject *self, DeeObject *match, DeeObject *def))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), DREF DeeObject *, DCALL, seq_locate_with_range, (DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_locate, "locate", "(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_locate__, "__seq_locate__", seq_locate)
//
///* Returns the last element (within the given range) where `match(elem)' is true. */
///*Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, seq_rlocate, (DeeObject *self, DeeObject *match, DeeObject *def))*/ /* Wouldn't make sense: for reverse, you need indices */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), DREF DeeObject *, DCALL, seq_rlocate_with_range, (DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_rlocate, "rlocate", "(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_rlocate__, "__seq_rlocate__", seq_rlocate)
//
///* @return: 0 : Does not start with
// * @return: 1 : Does start with
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_startswith, (DeeObject *self, DeeObject *item))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), int, DCALL, seq_startswith_with_key, (DeeObject *self, DeeObject *item, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_startswith_with_range, (DeeObject *self, DeeObject *item, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, seq_startswith_with_range_and_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_startswith, "startswith", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_startswith__, "__seq_startswith__", seq_startswith)
//
///* @return: 0 : Does not end with
// * @return: 1 : Does end with
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_endswith, (DeeObject *self, DeeObject *item))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), int, DCALL, seq_endswith_with_key, (DeeObject *self, DeeObject *item, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_endswith_with_range, (DeeObject *self, DeeObject *item, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, seq_endswith_with_range_and_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_endswith, "endswith", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_endswith__, "__seq_endswith__", seq_endswith)

%[include("seq_find.h")]

///* @return: * :         Index of `item' in `self'
// * @return: (size_t)-1: `item' could not be located in `self'
// * @return: (size_t)Dee_COMPARE_ERR: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_rfind, (DeeObject *self, DeeObject *item, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), size_t, DCALL, seq_rfind_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_rfind, "rfind", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_rfind__, "__seq_rfind__", seq_rfind)

%[include("seq_erase.h")]
%[include("seq_insert.h")]
%[include("seq_insertall.h")]
%[include("seq_pushfront.h")]
%[include("seq_append.h")]
%[include("seq_extend.h")]
%[include("seq_xchitem.h")]
%[include("seq_clear.h")]
%[include("seq_pop.h")]

//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_remove, (DeeObject *self, DeeObject *item, size_t start, size_t end)) /* @return: 1: Removed; 0: Not removed; -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, seq_remove_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key)) /* @return: 1: Removed; 0: Not removed; -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_remove, "remove", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_remove__, "__seq_remove__", seq_remove)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, seq_rremove, (DeeObject *self, DeeObject *item, size_t start, size_t end)) /* @return: 1: Removed; 0: Not removed; -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, seq_rremove_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key)) /* @return: 1: Removed; 0: Not removed; -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_rremove, "rremove", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_rremove__, "__seq_rremove__", seq_rremove)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_removeall, (DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 6)), size_t, DCALL, seq_removeall_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_removeall, "removeall", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_removeall__, "__seq_removeall__", seq_removeall)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_removeif, (DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_removeif, "removeif", "(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_removeif__, "__seq_removeif__", seq_removeif)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 3)), int, DCALL, seq_resize, (DeeObject *self, size_t newsize, DeeObject *filler))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_resize, "resize", "(size:?Dint,filler=!N)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_resize__, "__seq_resize__", seq_resize)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 4)), int, DCALL, seq_fill, (DeeObject *self, size_t start, size_t end, DeeObject *filler))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_fill, "fill", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,filler=!N)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_fill__, "__seq_fill__", seq_fill)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, seq_reverse, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_reverse, "reverse", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_reverse__, "__seq_reverse__", seq_reverse)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_reversed, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_reversed, "reversed", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?DSequence")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_reversed__, "__seq_reversed__", seq_reversed)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, seq_sort, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 4)), int, DCALL, seq_sort_with_key, (DeeObject *self, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_sort, "sort", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_sort__, "__seq_sort__", seq_sort)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, seq_sorted, (DeeObject *self, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 4)), DREF DeeObject *, DCALL, seq_sorted_with_key, (DeeObject *self, size_t start, size_t end, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_sorted, "sorted", "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?DSequence")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_sorted__, "__seq_sorted__", seq_sorted)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_bfind, (DeeObject *self, DeeObject *item, size_t start, size_t end)) /* @return: (size_t)-1: Not found; @return (size_t)Dee_COMPARE_ERR: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), size_t, DCALL, seq_bfind_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key)) /* @return: (size_t)-1: Not found; @return (size_t)Dee_COMPARE_ERR: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_bfind, "bfind", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_bfind__, "__seq_bfind__", seq_bfind)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), size_t, DCALL, seq_bposition, (DeeObject *self, DeeObject *item, size_t start, size_t end)) /* @return: (size_t)Dee_COMPARE_ERR: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), size_t, DCALL, seq_bposition_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key)) /* @return: (size_t)Dee_COMPARE_ERR: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_bposition, "bposition", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_bposition__, "__seq_bposition__", seq_bposition)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, seq_brange, (DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5, 6)), int, DCALL, seq_brange_with_key, (DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]))
//Dee_DEFINE_TYPE_METHOD_HINT_KWMETHOD(seq_brange, "brange", "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?Dint")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__seq_brange__, "__seq_brange__", seq_brange)




/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
%[include("set_operator_iter.h")]

//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, set_operator_sizeob, (DeeObject *__restrict self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), size_t, DCALL, set_operator_size, (DeeObject *__restrict self))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_size__, "__set_size__", "->?Dint")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), Dee_hash_t, DCALL, set_operator_hash, (DeeObject *__restrict self))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_hash__, "__set_hash__", "->?Dint")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_operator_compare_eq, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_compare_eq__, "__set_compare_eq__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_operator_trycompare_eq, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_trycompare_eq__, "__set_trycompare_eq__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_eq, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_eq__, "__set_eq__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_ne, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_ne__, "__set_ne__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_lo, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_lo__, "__set_lo__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_le, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_le__, "__set_le__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_gr, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_gr__, "__set_gr__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_ge, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_ge__, "__set_ge__", "(rhs:?S?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, set_operator_inv, (DeeObject *__restrict self))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_inv__, "__set_inv__", "->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_add, (DeeObject *self, DeeObject *some_object)) /* {"a"} + {"b"}         -> {"a","b"} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_add__, "__set_add__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_sub, (DeeObject *self, DeeObject *some_object)) /* {"a","b"} - {"b"}     -> {"a"} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_sub__, "__set_sub__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_and, (DeeObject *self, DeeObject *some_object)) /* {"a","b"} & {"a"}     -> {"a"} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_and__, "__set_and__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_operator_xor, (DeeObject *self, DeeObject *some_object)) /* {"a","b"} ^ {"a","c"} -> {"b","c"} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_xor__, "__set_xor__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_operator_inplace_add, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_inplace_add__, "__set_inplace_add__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_operator_inplace_sub, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_inplace_sub__, "__set_inplace_sub__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_operator_inplace_and, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_inplace_and__, "__set_inplace_and__", "(rhs:?DSet)->?DSet")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_operator_inplace_xor, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__set_inplace_xor__, "__set_inplace_xor__", "(rhs:?DSet)->?DSet")
//
///* Insert a key into a set
// * @return: 1 : Given `key' was inserted and wasn't already present
// * @return: 0 : Given `key' was already present
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_insert, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(set_insert, "insert", "(key)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__set_insert__, "__set_insert__", set_insert)
//
///* Remove a key from a set
// * @return: 1 : Given `key' was removed
// * @return: 0 : Given `key' was wasn't present
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_remove, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(set_remove, "remove", "(key)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__set_remove__, "__set_remove__", set_remove)
//
///* Insert `key' if not already present and re-return `key'.
// * If already present, return the pre-existing (and equal) instance instead.
// * @return: NULL: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_unify, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(set_unify, "unify", "(key)->")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__set_unify__, "__set_unify__", set_unify)
//
///* @return: 0 : Success
// * @return: -1: Error  */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_insertall, (DeeObject *self, DeeObject *keys))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(set_insertall, "insertall", "(keys:?S?O)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__set_insertall__, "__set_insertall__", set_insertall)
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, set_removeall, (DeeObject *self, DeeObject *keys))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(set_removeall, "removeall", "(keys:?S?O)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__set_removeall__, "__set_removeall__", set_removeall)
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, set_pop, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, set_pop_with_default, (DeeObject *self, DeeObject *default_))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(set_pop, "pop", "(def?)->")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__set_pop__, "__set_pop__", set_pop)
//
//
//
//
//
///************************************************************************/
///* For `deemon.Mapping'                                                 */
///************************************************************************/
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_contains, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_contains__, "__map_contains__", "(key)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_getitem, (DeeObject *self, DeeObject *index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_trygetitem, (DeeObject *self, DeeObject *index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_bounditem, (DeeObject *self, DeeObject *index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_hasitem, (DeeObject *self, DeeObject *index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, map_operator_getitem_index, (DeeObject *self, size_t index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, map_operator_trygetitem_index, (DeeObject *self, size_t index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, map_operator_bounditem_index, (DeeObject *self, size_t index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, map_operator_hasitem_index, (DeeObject *self, size_t index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_getitem_string_hash, (DeeObject *self, char const *key, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_trygetitem_string_hash, (DeeObject *self, char const *key, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_bounditem_string_hash, (DeeObject *self, char const *key, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_hasitem_string_hash, (DeeObject *self, char const *key, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_getitem_string_len_hash, (DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_trygetitem_string_len_hash, (DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_bounditem_string_len_hash, (DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_hasitem_string_len_hash, (DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_getitem__, "__map_getitem__", "(key)->")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_delitem, (DeeObject *self, DeeObject *index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), int, DCALL, map_operator_delitem_index, (DeeObject *self, size_t index))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_delitem_string_hash, (DeeObject *self, char const *key, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_delitem_string_len_hash, (DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_delitem__, "__map_delitem__", "(key)")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), int, DCALL, map_operator_setitem, (DeeObject *self, DeeObject *index, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 3)), int, DCALL, map_operator_setitem_index, (DeeObject *self, size_t index, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 4)), int, DCALL, map_operator_setitem_string_hash, (DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 5)), int, DCALL, map_operator_setitem_string_len_hash, (DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_setitem__, "__map_setitem__", "(key,value)")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), Dee_ssize_t, DCALL, map_operator_enumerate, (DeeObject *__restrict self, Dee_enumerate_t proc, void *arg))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), Dee_ssize_t, DCALL, map_operator_enumerate_index, (DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_enumerate__, "__map_enumerate__",
//                                   "(cb:?DCallable)->?X2?O?N\n" /* function cb(key, value?) */
//                                   "(cb:?DCallable,startkey:?Dint,endkey:?Dint)->?X2?O?N")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_compare_eq, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_compare_eq__, "__map_compare_eq__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_trycompare_eq, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_trycompare_eq__, "__map_trycompare_eq__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_eq, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_eq__, "__map_eq__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_ne, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_ne__, "__map_ne__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_lo, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_lo__, "__map_lo__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_le, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_le__, "__map_le__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_gr, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_gr__, "__map_gr__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_ge, (DeeObject *self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_ge__, "__map_ge__", "(rhs:?X2?M?O?O?S?T2?O?O)->?Dbool")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_add, (DeeObject *self, DeeObject *some_object)) /* {"a":1} + {"b":2}       -> {"a":1,"b":2} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_add__, "__map_add__", "(rhs:?X2?M?O?O?S?T2?O?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_sub, (DeeObject *self, DeeObject *some_object)) /* {"a":1,"b":2} - {"a"}   -> {"b":2} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_sub__, "__map_sub__", "(rhs:?X2?DSet?S?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_and, (DeeObject *self, DeeObject *some_object)) /* {"a":1,"b":2} & {"a"}   -> {"a":1} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_and__, "__map_and__", "(rhs:?X2?DSet?S?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_operator_xor, (DeeObject *self, DeeObject *some_object)) /* {"a":1,"b":2} ^ {"a":3} -> {"b":2} */
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_xor__, "__map_xor__", "(rhs:?X2?M?O?O?S?T2?O?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_inplace_add, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_inplace_add__, "__map_inplace_add__", "(rhs:?X2?M?O?O?S?T2?O?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_inplace_sub, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_inplace_sub__, "__map_inplace_sub__", "(rhs:?X2?DSet?S?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_inplace_and, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_inplace_and__, "__map_inplace_and__", "(rhs:?X2?DSet?S?O)->?DMapping")
//
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int, DCALL, map_operator_inplace_xor, (DREF DeeObject **__restrict p_self, DeeObject *some_object))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(__map_inplace_xor__, "__map_inplace_xor__", "(rhs:?X2?M?O?O?S?T2?O?O)->?DMapping")
//
//
///* Override the value of a pre-existing key
// * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
// * @return: 1 :   The value of `key' was set to `value'
// * @return: 0 :   The given `key' doesn't exist (nothing was updated)
// * @return: -1:   Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), int , DCALL, map_setold, (DeeObject *self, DeeObject *key, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_setold, "setold", "(key,value)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_setold__, "__map_setold__", map_setold)
//
///* @return: * :        The value of `key' was set to `value' (returned object is the old value)
// * @return: ITER_DONE: The given `key' doesn't exist (nothing was updated)
// * @return: NULL:      Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, map_setold_ex, (DeeObject *self, DeeObject *key, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_setold_ex, "setold_ex", "(key,value)->?T2?Dbool?X2?O?N")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_setold_ex__, "__map_setold_ex__", map_setold_ex)
//
///* Insert a new key whilst making sure that the key doesn't already exist
// * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
// * @return: 1 :   The value of `key' was set to `value' (the key didn't exist or used to be unbound)
// * @return: 0 :   The given `key' already exists (nothing was inserted)
// * @return: -1:   Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), int , DCALL, map_setnew, (DeeObject *self, DeeObject *key, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_setnew, "setnew", "(key,value)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_setnew__, "__map_setnew__", map_setnew)
//
///* @return: ITER_DONE: The value of `key' was set to `value' (the key didn't exist or used to be unbound)
// * @return: * :        The given `key' already exists (nothing was inserted; returned object is the already-present value)
// * @return: -1:        Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, map_setnew_ex, (DeeObject *self, DeeObject *key, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_setnew_ex, "setnew_ex", "(key,value)->?T2?Dbool?X2?O?N")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_setnew_ex__, "__map_setnew_ex__", map_setnew_ex)
//
///* Same semantic functionality as `Dee_mh_map_setnew_ex_t': insert if not already present
// * @return: * : The value associated with key after the call:
// *              - if already present and nothing was inserted, its old value
// *              - if used-to-be absent/unbound and was assigned/inserted, `value'
// * @return: NULL: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, map_setdefault, (DeeObject *self, DeeObject *key, DeeObject *value))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_setdefault, "setdefault", "(key,value)->")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_setdefault__, "__map_setdefault__", map_setdefault)
//
///* Copy all key-value pairs from `items' and assign them to `self'.
// * Same as `for (local key, value: items) self[key] = value;'
// * @return: 0 : Success
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int , DCALL, map_update, (DeeObject *self, DeeObject *items))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_update, "update", "(items:?M?O?O)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_update__, "__map_update__", map_update)
//
///* Remove a single key, returning true/false indicative of that key having been removed.
// * @return: 1 : Key was removed
// * @return: 0 : Key didn't exist (nothing was removed)
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int , DCALL, map_remove, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_remove, "remove", "(key)->?Dbool")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_remove__, "__map_remove__", map_remove)
//
///* Delete all keys that appear in `keys'.
// * Same as `for (local key: keys) del self[key];'
// * @return: 0 : Success
// * @return: -1: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), int , DCALL, map_removekeys, (DeeObject *self, DeeObject *keys))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_removekeys, "removekeys", "(keys:?S?O)")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_removekeys__, "__map_removekeys__", map_removekeys)
//
///* Remove/unbind `key' and return whatever used to be assigned to it.
// * When the key was already absent/unbound, return `default_' or throw a `KeyError'
// * @return: * :   The old value of `key'
// * @return: NULL: Error */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2)), DREF DeeObject *, DCALL, map_pop, (DeeObject *self, DeeObject *key))
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1, 2, 3)), DREF DeeObject *, DCALL, map_pop_with_default, (DeeObject *self, DeeObject *key, DeeObject *default_))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_pop, "pop", "(key,def?)->")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_pop__, "__map_pop__", map_pop)
//
///* Remove a random key/value pair from `self' and store it in `key_and_value' (returns "none" if nothing found) */
//Dee_DEFINE_TYPE_METHOD_HINT_FUNC(WUNUSED_T NONNULL_T((1)), DREF DeeObject *, DCALL, map_popitem, (DeeObject *self))
//Dee_DEFINE_TYPE_METHOD_HINT_METHOD(map_popitem, "popitem", "->?X2?T2?O?O?N")
//Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(__map_popitem__, "__map_popitem__", map_popitem)

