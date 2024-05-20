/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "default-api-methods.c"
#define DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
//#define DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
//#define DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
//#define DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#if (defined(DEFINE_DeeSeq_DefaultFooWithCallAttrFoo) +         \
     defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction) + \
     defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod) +   \
     defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeSeq_Default... */

#ifdef DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Foo, Bar) PP_CAT4(DeeSeq_Default, Foo, WithCallAttr, Bar)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction)
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Foo, Bar) PP_CAT5(DeeSeq_Default, Foo, WithCall, Bar, DataFunction)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod)
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Foo, Bar) PP_CAT5(DeeSeq_Default, Foo, WithCall, Bar, DataMethod)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod)
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Foo, Bar) PP_CAT5(DeeSeq_Default, Foo, WithCall, Bar, DataKwMethod)
#else /* DEFINE_DeeSeq_Default... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeSeq_Default... */

#define LOCAL_DeeSeq_DefaultFindWithCallAttrFind                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Find, Find)
#define LOCAL_DeeSeq_DefaultFindWithKeyWithCallAttrFind           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(FindWithKey, Find)
#define LOCAL_DeeSeq_DefaultRFindWithCallAttrRFind                LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RFind, RFind)
#define LOCAL_DeeSeq_DefaultRFindWithKeyWithCallAttrRFind         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RFindWithKey, RFind)
#define LOCAL_DeeSeq_DefaultEraseWithCallAttrErase                LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Erase, Erase)
#define LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Insert, Insert)
#define LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(InsertAll, InsertAll)
#define LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(PushFront, PushFront)
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Append, Append)
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack            LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Append, PushBack)
#define LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Extend, Extend)
#define LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(XchItemIndex, XchItem)
#define LOCAL_DeeSeq_DefaultClearWithCallAttrClear                LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Clear, Clear)
#define LOCAL_DeeSeq_DefaultPopWithCallAttrPop                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Pop, Pop)
#define LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Remove, Remove)
#define LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveWithKey, Remove)
#define LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove            LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RRemove, RRemove)
#define LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RRemoveWithKey, RRemove)
#define LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveAll, RemoveAll)
#define LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveAllWithKey, RemoveAll)
#define LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf          LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveIf, RemoveIf)
#define LOCAL_DeeSeq_DefaultResizeWithCallAttrResize              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Resize, Resize)
#define LOCAL_DeeSeq_DefaultFillWithCallAttrFill                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Fill, Fill)
#define LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse            LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Reverse, Reverse)
#define LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed          LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Reversed, Reversed)
#define LOCAL_DeeSeq_DefaultSortWithCallAttrSort                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Sort, Sort)
#define LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SortWithKey, Sort)
#define LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Sorted, Sorted)
#define LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SortedWithKey, Sorted)
#define LOCAL_DeeSeq_DefaultBFindWithCallAttrBFind                LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BFind, BFind)
#define LOCAL_DeeSeq_DefaultBFindWithKeyWithCallAttrBFind         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BFindWithKey, BFind)
#define LOCAL_DeeSeq_DefaultBPositionWithCallAttrBPosition        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BPosition, BPosition)
#define LOCAL_DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BPositionWithKey, BPosition)
#define LOCAL_DeeSeq_DefaultBRangeWithCallAttrBRange              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BRange, BRange)
#define LOCAL_DeeSeq_DefaultBRangeWithKeyWithCallAttrBRange       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BRangeWithKey, BRange)
#define LOCAL_DeeSeq_DefaultBLocateWithCallAttrBLocate            LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BLocate, BLocate)
#define LOCAL_DeeSeq_DefaultBLocateWithKeyWithCallAttrBLocate     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BLocateWithKey, BLocate)



#ifdef DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#define LOCAL_DeeObject_CallAttr(self, tsc_foo_data, attr, argc, argv) \
	DeeObject_CallAttr(self, (DeeObject *)(attr), argc, argv)
#define LOCAL_DeeObject_CallAttrf(self, tsc_foo_data, attr, ...) \
	DeeObject_CallAttrf(self, (DeeObject *)(attr), __VA_ARGS__)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction)
#define LOCAL_DeeObject_CallAttr(self, tsc_foo_data, attr, argc, argv) \
	DeeObject_ThisCall(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_function, self, argc, argv)
#define LOCAL_DeeObject_CallAttrf(self, tsc_foo_data, attr, ...) \
	DeeObject_ThisCallf(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_function, self, __VA_ARGS__)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod)
#define LOCAL_DeeObject_CallAttr(self, tsc_foo_data, attr, argc, argv) \
	(*Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_method)(self, argc, argv)
#define LOCAL_DeeObject_CallAttrf(self, tsc_foo_data, attr, ...) \
	DeeObjMethod_CallFuncf(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_method, self, __VA_ARGS__)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod)
#define LOCAL_DeeObject_CallAttr(self, tsc_foo_data, attr, argc, argv) \
	(*Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_kwmethod)(self, argc, argv, NULL)
#define LOCAL_DeeObject_CallAttrf(self, tsc_foo_data, attr, ...) \
	DeeKwObjMethod_CallFuncf(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_kwmethod, self, __VA_ARGS__)
#else /* DEFINE_DeeSeq_Default... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeSeq_Default... */


/************************************************************************/
/* Attribute proxy implementations.                                     */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultFindWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_find_data, &str_find, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
LOCAL_DeeSeq_DefaultFindWithKeyWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_find_data, &str_find, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultRFindWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_rfind_data, &str_rfind, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
LOCAL_DeeSeq_DefaultRFindWithKeyWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_rfind_data, &str_rfind, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultEraseWithCallAttrErase(DeeObject *self, size_t index, size_t count) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_erase_data, &str_erase, PCKuSIZ PCKuSIZ, index, count);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert(DeeObject *self, size_t index, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_insert_data, &str_insert, PCKuSIZ "o", index, item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, size_t index, DeeObject *items) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_insertall_data, &str_insertall, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_pushfront_data, &str_pushfront, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_append_data, &str_append, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

#ifdef LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack
INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_append_data, &str_pushback, 1, &item);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}
#endif /* LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_extend_data, &str_extend, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem(DeeObject *self, size_t index, DeeObject *value) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_xchitem_data, &str_xchitem, PCKuSIZ "o", index, value);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultClearWithCallAttrClear(DeeObject *self) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_clear_data, &str_clear, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultPopWithCallAttrPop(DeeObject *self, Dee_ssize_t index) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_pop_data, &str_pop, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *item,
                                             size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_remove_data, &str_remove,
	                                   "o" PCKuSIZ PCKuSIZ,
	                                   item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove(DeeObject *self, DeeObject *item,
                                                    size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_remove_data, &str_remove,
	                                   "o" PCKuSIZ PCKuSIZ "o",
	                                   item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_rremove_data, &str_rremove,
	                                   "o" PCKuSIZ PCKuSIZ,
	                                   item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove(DeeObject *self, DeeObject *item,
                                                      size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_rremove_data, &str_rremove,
	                                   "o" PCKuSIZ PCKuSIZ "o",
	                                   item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, size_t max) {
	int error;
	size_t result_count;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_removeall_data, &str_removeall,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ,
	                                   item, start, end, max);
	if unlikely(!result)
		goto err;
	error = DeeObject_AsSize(result, &result_count);
	Dee_Decref(result);
	if unlikely(error)
		goto err;
	if unlikely(result_count == (size_t)-1)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return result_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll(DeeObject *self, DeeObject *item,
                                                          size_t start, size_t end,
                                                          size_t max, DeeObject *key) {
	int error;
	size_t result_count;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_removeall_data, &str_removeall,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ "o",
	                                   item, start, end, max, key);
	if unlikely(!result)
		goto err;
	error = DeeObject_AsSize(result, &result_count);
	Dee_Decref(result);
	if unlikely(error)
		goto err;
	if unlikely(result_count == (size_t)-1)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return result_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf(DeeObject *self, DeeObject *should,
                                                 size_t start, size_t end, size_t max) {
	int error;
	size_t result_count;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_removeif_data, &str_removeif,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ,
	                                   should, start, end, max);
	if unlikely(!result)
		goto err;
	error = DeeObject_AsSize(result, &result_count);
	Dee_Decref(result);
	if unlikely(error)
		goto err;
	if unlikely(result_count == (size_t)-1)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return result_count;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_DeeSeq_DefaultResizeWithCallAttrResize(DeeObject *self, size_t newsize, DeeObject *filler) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_resize_data, &str_resize,
	                                   PCKuSIZ "o", newsize, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
LOCAL_DeeSeq_DefaultFillWithCallAttrFill(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_fill_data, &str_fill,
	                                   PCKuSIZ PCKuSIZ "o",
	                                   start, end, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_reverse_data, &str_reverse,
	                                   PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed(DeeObject *self, size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_reversed_data, &str_reversed,
	                                 PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultSortWithCallAttrSort(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_sort_data, &str_sort,
	                                   PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_sort_data, &str_sort,
	                                   PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted(DeeObject *self, size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_sorted_data, &str_sorted,
	                                 PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_sorted_data, &str_sorted,
	                                 PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultBFindWithCallAttrBFind(DeeObject *self, DeeObject *item,
                                           size_t start, size_t end) {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_bfind_data, &str_bfind, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return (size_t)-1;
	}
	temp = DeeObject_AsSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
LOCAL_DeeSeq_DefaultBFindWithKeyWithCallAttrBFind(DeeObject *self, DeeObject *item,
                                                  size_t start, size_t end, DeeObject *key) {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_bfind_data, &str_bfind, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return (size_t)-1;
	}
	temp = DeeObject_AsSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultBPositionWithCallAttrBPosition(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end) {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_bposition_data, &str_bposition, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
LOCAL_DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition(DeeObject *self, DeeObject *item,
                                                          size_t start, size_t end, DeeObject *key) {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_bposition_data, &str_bposition, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_DeeSeq_DefaultBRangeWithCallAttrBRange(DeeObject *self, DeeObject *item,
                                             size_t start, size_t end, size_t result_range[2]) {
	int temp;
	DREF DeeObject *result, *result_range_objs[2];
	result = LOCAL_DeeObject_CallAttrf(self, tsc_brange_data, &str_brange, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, result_range_objs);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_AsSize(result_range_objs[0], &result_range[0]);
	if likely(temp == 0)
		temp = DeeObject_AsSize(result_range_objs[1], &result_range[1]);
	Dee_Decrefv(result_range_objs, 2);
	if unlikely(temp)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
LOCAL_DeeSeq_DefaultBRangeWithKeyWithCallAttrBRange(DeeObject *self, DeeObject *item,
                                                    size_t start, size_t end, DeeObject *key,
                                                    size_t result_range[2]) {
	int temp;
	DREF DeeObject *result, *result_range_objs[2];
	result = LOCAL_DeeObject_CallAttrf(self, tsc_brange_data, &str_brange, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, result_range_objs);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_AsSize(result_range_objs[0], &result_range[0]);
	if likely(temp == 0)
		temp = DeeObject_AsSize(result_range_objs[1], &result_range[1]);
	Dee_Decrefv(result_range_objs, 2);
	if unlikely(temp)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultBLocateWithCallAttrBLocate(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_blocate_data, &str_blocate, "o" PCKuSIZ PCKuSIZ, item, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultBLocateWithKeyWithCallAttrBLocate(DeeObject *self, DeeObject *item,
                                                      size_t start, size_t end, DeeObject *key) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_blocate_data, &str_blocate, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
}


#undef LOCAL_DeeObject_CallAttr
#undef LOCAL_DeeObject_CallAttrf

#undef LOCAL_DeeSeq_DefaultFindWithCallAttrFind
#undef LOCAL_DeeSeq_DefaultFindWithKeyWithCallAttrFind
#undef LOCAL_DeeSeq_DefaultRFindWithCallAttrRFind
#undef LOCAL_DeeSeq_DefaultRFindWithKeyWithCallAttrRFind
#undef LOCAL_DeeSeq_DefaultEraseWithCallAttrErase
#undef LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert
#undef LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll
#undef LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront
#undef LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend
#undef LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack
#undef LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend
#undef LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem
#undef LOCAL_DeeSeq_DefaultClearWithCallAttrClear
#undef LOCAL_DeeSeq_DefaultPopWithCallAttrPop
#undef LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove
#undef LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove
#undef LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove
#undef LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove
#undef LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll
#undef LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll
#undef LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf
#undef LOCAL_DeeSeq_DefaultResizeWithCallAttrResize
#undef LOCAL_DeeSeq_DefaultFillWithCallAttrFill
#undef LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse
#undef LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed
#undef LOCAL_DeeSeq_DefaultSortWithCallAttrSort
#undef LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort
#undef LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted
#undef LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted
#undef LOCAL_DeeSeq_DefaultBFindWithCallAttrBFind
#undef LOCAL_DeeSeq_DefaultBFindWithKeyWithCallAttrBFind
#undef LOCAL_DeeSeq_DefaultBPositionWithCallAttrBPosition
#undef LOCAL_DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition
#undef LOCAL_DeeSeq_DefaultBRangeWithCallAttrBRange
#undef LOCAL_DeeSeq_DefaultBRangeWithKeyWithCallAttrBRange
#undef LOCAL_DeeSeq_DefaultBLocateWithCallAttrBLocate
#undef LOCAL_DeeSeq_DefaultBLocateWithKeyWithCallAttrBLocate

#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBar

DECL_END

#undef DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod

