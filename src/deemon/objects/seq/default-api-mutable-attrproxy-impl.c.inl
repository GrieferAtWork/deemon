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
#include "default-api-mutable.c"
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
#define LOCAL_DeeSeq_DefaultEraseWithCallAttrErase                DeeSeq_DefaultEraseWithCallAttrErase
#define LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert              DeeSeq_DefaultInsertWithCallAttrInsert
#define LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll        DeeSeq_DefaultInsertAllWithCallAttrInsertAll
#define LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront        DeeSeq_DefaultPushFrontWithCallAttrPushFront
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend              DeeSeq_DefaultAppendWithCallAttrAppend
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack            DeeSeq_DefaultAppendWithCallAttrPushBack
#define LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend              DeeSeq_DefaultExtendWithCallAttrExtend
#define LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem       DeeSeq_DefaultXchItemIndexWithCallAttrXchItem
#define LOCAL_DeeSeq_DefaultClearWithCallAttrClear                DeeSeq_DefaultClearWithCallAttrClear
#define LOCAL_DeeSeq_DefaultPopWithCallAttrPop                    DeeSeq_DefaultPopWithCallAttrPop
#define LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove              DeeSeq_DefaultRemoveWithCallAttrRemove
#define LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove       DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove
#define LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove            DeeSeq_DefaultRRemoveWithCallAttrRRemove
#define LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove     DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove
#define LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll        DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll
#define LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll
#define LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf          DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf
#define LOCAL_DeeSeq_DefaultResizeWithCallAttrResize              DeeSeq_DefaultResizeWithCallAttrResize
#define LOCAL_DeeSeq_DefaultFillWithCallAttrFill                  DeeSeq_DefaultFillWithCallAttrFill
#define LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse            DeeSeq_DefaultReverseWithCallAttrReverse
#define LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed          DeeSeq_DefaultReversedWithCallAttrReversed
#define LOCAL_DeeSeq_DefaultSortWithCallAttrSort                  DeeSeq_DefaultSortWithCallAttrSort
#define LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort           DeeSeq_DefaultSortWithKeyWithCallAttrSort
#define LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted              DeeSeq_DefaultSortedWithCallAttrSorted
#define LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted       DeeSeq_DefaultSortedWithKeyWithCallAttrSorted
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction)
#define LOCAL_DeeSeq_DefaultEraseWithCallAttrErase                DeeSeq_DefaultEraseWithCallEraseDataFunction
#define LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert              DeeSeq_DefaultInsertWithCallInsertDataFunction
#define LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll        DeeSeq_DefaultInsertAllWithCallInsertAllDataFunction
#define LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront        DeeSeq_DefaultPushFrontWithCallPushFrontDataFunction
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend              DeeSeq_DefaultAppendWithCallAppendDataFunction
#define LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend              DeeSeq_DefaultExtendWithCallExtendDataFunction
#define LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem       DeeSeq_DefaultXchItemIndexWithCallXchItemIndexDataFunction
#define LOCAL_DeeSeq_DefaultClearWithCallAttrClear                DeeSeq_DefaultClearWithCallClearDataFunction
#define LOCAL_DeeSeq_DefaultPopWithCallAttrPop                    DeeSeq_DefaultPopWithCallPopDataFunction
#define LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove              DeeSeq_DefaultRemoveWithCallRemoveDataFunction
#define LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove       DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction
#define LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove            DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction
#define LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove     DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction
#define LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll        DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction
#define LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction
#define LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf          DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction
#define LOCAL_DeeSeq_DefaultResizeWithCallAttrResize              DeeSeq_DefaultResizeWithCallResizeDataFunction
#define LOCAL_DeeSeq_DefaultFillWithCallAttrFill                  DeeSeq_DefaultFillWithCallFillDataFunction
#define LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse            DeeSeq_DefaultReverseWithCallReverseDataFunction
#define LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed          DeeSeq_DefaultReversedWithCallReversedDataFunction
#define LOCAL_DeeSeq_DefaultSortWithCallAttrSort                  DeeSeq_DefaultSortWithCallSortDataFunction
#define LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort           DeeSeq_DefaultSortWithKeyWithCallSortDataFunction
#define LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted              DeeSeq_DefaultSortedWithCallSortedDataFunction
#define LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted       DeeSeq_DefaultSortedWithKeyWithCallSortedDataFunction
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod)
#define LOCAL_DeeSeq_DefaultEraseWithCallAttrErase                DeeSeq_DefaultEraseWithCallEraseDataMethod
#define LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert              DeeSeq_DefaultInsertWithCallInsertDataMethod
#define LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll        DeeSeq_DefaultInsertAllWithCallInsertAllDataMethod
#define LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront        DeeSeq_DefaultPushFrontWithCallPushFrontDataMethod
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend              DeeSeq_DefaultAppendWithCallAppendDataMethod
#define LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend              DeeSeq_DefaultExtendWithCallExtendDataMethod
#define LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem       DeeSeq_DefaultXchItemIndexWithCallXchItemIndexDataMethod
#define LOCAL_DeeSeq_DefaultClearWithCallAttrClear                DeeSeq_DefaultClearWithCallClearDataMethod
#define LOCAL_DeeSeq_DefaultPopWithCallAttrPop                    DeeSeq_DefaultPopWithCallPopDataMethod
#define LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove              DeeSeq_DefaultRemoveWithCallRemoveDataMethod
#define LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove       DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataMethod
#define LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove            DeeSeq_DefaultRRemoveWithCallRRemoveDataMethod
#define LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove     DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataMethod
#define LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll        DeeSeq_DefaultRemoveAllWithCallRemoveAllDataMethod
#define LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataMethod
#define LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf          DeeSeq_DefaultRemoveIfWithCallRemoveIfDataMethod
#define LOCAL_DeeSeq_DefaultResizeWithCallAttrResize              DeeSeq_DefaultResizeWithCallResizeDataMethod
#define LOCAL_DeeSeq_DefaultFillWithCallAttrFill                  DeeSeq_DefaultFillWithCallFillDataMethod
#define LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse            DeeSeq_DefaultReverseWithCallReverseDataMethod
#define LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed          DeeSeq_DefaultReversedWithCallReversedDataMethod
#define LOCAL_DeeSeq_DefaultSortWithCallAttrSort                  DeeSeq_DefaultSortWithCallSortDataMethod
#define LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort           DeeSeq_DefaultSortWithKeyWithCallSortDataMethod
#define LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted              DeeSeq_DefaultSortedWithCallSortedDataMethod
#define LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted       DeeSeq_DefaultSortedWithKeyWithCallSortedDataMethod
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod)
#define LOCAL_DeeSeq_DefaultEraseWithCallAttrErase                DeeSeq_DefaultEraseWithCallEraseDataKwMethod
#define LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert              DeeSeq_DefaultInsertWithCallInsertDataKwMethod
#define LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll        DeeSeq_DefaultInsertAllWithCallInsertAllDataKwMethod
#define LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront        DeeSeq_DefaultPushFrontWithCallPushFrontDataKwMethod
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend              DeeSeq_DefaultAppendWithCallAppendDataKwMethod
#define LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend              DeeSeq_DefaultExtendWithCallExtendDataKwMethod
#define LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem       DeeSeq_DefaultXchItemIndexWithCallXchItemIndexDataKwMethod
#define LOCAL_DeeSeq_DefaultClearWithCallAttrClear                DeeSeq_DefaultClearWithCallClearDataKwMethod
#define LOCAL_DeeSeq_DefaultPopWithCallAttrPop                    DeeSeq_DefaultPopWithCallPopDataKwMethod
#define LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove              DeeSeq_DefaultRemoveWithCallRemoveDataKwMethod
#define LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove       DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataKwMethod
#define LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove            DeeSeq_DefaultRRemoveWithCallRRemoveDataKwMethod
#define LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove     DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataKwMethod
#define LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll        DeeSeq_DefaultRemoveAllWithCallRemoveAllDataKwMethod
#define LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataKwMethod
#define LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf          DeeSeq_DefaultRemoveIfWithCallRemoveIfDataKwMethod
#define LOCAL_DeeSeq_DefaultResizeWithCallAttrResize              DeeSeq_DefaultResizeWithCallResizeDataKwMethod
#define LOCAL_DeeSeq_DefaultFillWithCallAttrFill                  DeeSeq_DefaultFillWithCallFillDataKwMethod
#define LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse            DeeSeq_DefaultReverseWithCallReverseDataKwMethod
#define LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed          DeeSeq_DefaultReversedWithCallReversedDataKwMethod
#define LOCAL_DeeSeq_DefaultSortWithCallAttrSort                  DeeSeq_DefaultSortWithCallSortDataKwMethod
#define LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort           DeeSeq_DefaultSortWithKeyWithCallSortDataKwMethod
#define LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted              DeeSeq_DefaultSortedWithCallSortedDataKwMethod
#define LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted       DeeSeq_DefaultSortedWithKeyWithCallSortedDataKwMethod
#else /* DEFINE_DeeSeq_Default... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeSeq_Default... */



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
	return LOCAL_DeeObject_CallAttrf(self, tsc_xchitem_index_data, &str_xchitem, PCKuSIZ "o", index, value);
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
	int temp;
	size_t result_count;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_removeall_data, &str_removeall,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ,
	                                   item, start, end, max);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSize(result, &result_count);
	Dee_Decref(result);
	if unlikely(!temp)
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
	int temp;
	size_t result_count;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_removeall_data, &str_removeall,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ "o",
	                                   item, start, end, max, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSize(result, &result_count);
	Dee_Decref(result);
	if unlikely(!temp)
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
	int temp;
	size_t result_count;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_removeif_data, &str_removeif,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ,
	                                   should, start, end, max);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSize(result, &result_count);
	Dee_Decref(result);
	if unlikely(!temp)
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

#undef LOCAL_DeeObject_CallAttr
#undef LOCAL_DeeObject_CallAttrf

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

DECL_END

#undef DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod

