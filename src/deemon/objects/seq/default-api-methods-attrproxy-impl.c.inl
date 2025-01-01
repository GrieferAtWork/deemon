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
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeApi_Default, Foo, Bar)     PP_CAT4(DeeApi_Default, Foo, WithCallAttr, Bar)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeApi_Default, Foo, Bar, x) PP_CAT5(DeeApi_Default, Foo, WithCallAttr, Bar, x)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeApi_Default, Foo, Bar)     PP_CAT5(DeeApi_Default, Foo, WithCall, Bar, DataFunction)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeApi_Default, Foo, Bar, x) PP_CAT6(DeeApi_Default, Foo, WithCall, Bar, DataFunction, x)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeApi_Default, Foo, Bar)     PP_CAT5(DeeApi_Default, Foo, WithCall, Bar, DataMethod)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeApi_Default, Foo, Bar, x) PP_CAT6(DeeApi_Default, Foo, WithCall, Bar, DataMethod, x)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeApi_Default, Foo, Bar)     PP_CAT5(DeeApi_Default, Foo, WithCall, Bar, DataKwMethod)
#define LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeApi_Default, Foo, Bar, x) PP_CAT6(DeeApi_Default, Foo, WithCall, Bar, DataKwMethod, x)
#else /* DEFINE_DeeSeq_Default... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeSeq_Default... */

#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Foo, Bar)     LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeSeq_Default, Foo, Bar)
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(Foo, Bar, x) LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeSeq_Default, Foo, Bar, x)
#define LOCAL_DeeSet_DefaultFooWithCallAttrBar(Foo, Bar)     LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeSet_Default, Foo, Bar)
#define LOCAL_DeeSet_DefaultFooWithCallAttrBar_(Foo, Bar, x) LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeSet_Default, Foo, Bar, x)
#define LOCAL_DeeMap_DefaultFooWithCallAttrBar(Foo, Bar)     LOCAL_DeeApi_DefaultFooWithCallAttrBar(DeeMap_Default, Foo, Bar)
#define LOCAL_DeeMap_DefaultFooWithCallAttrBar_(Foo, Bar, x) LOCAL_DeeApi_DefaultFooWithCallAttrBar_(DeeMap_Default, Foo, Bar, x)

#define LOCAL_DeeSeq_DefaultGetFirstWithCallAttrGetFirst                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(GetFirst, GetFirst)
#define LOCAL_DeeSeq_DefaultBoundFirstWithCallAttrGetFirst                     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BoundFirst, GetFirst)
#define LOCAL_DeeSeq_DefaultDelFirstWithCallAttrDelFirst                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(DelFirst, DelFirst)
#define LOCAL_DeeSeq_DefaultSetFirstWithCallAttrSetFirst                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SetFirst, SetFirst)
#define LOCAL_DeeSeq_DefaultGetLastWithCallAttrGetLast                         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(GetLast, GetLast)
#define LOCAL_DeeSeq_DefaultBoundLastWithCallAttrGetLast                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BoundLast, GetLast)
#define LOCAL_DeeSeq_DefaultDelLastWithCallAttrDelLast                         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(DelLast, DelLast)
#define LOCAL_DeeSeq_DefaultSetLastWithCallAttrSetLast                         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SetLast, SetLast)
#define LOCAL_DeeSeq_DefaultCachedWithCallAttrCached                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Cached, Cached)
#define LOCAL_DeeSeq_DefaultAnyWithCallAttrAny                                 LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Any, Any)
#define LOCAL_DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSeq                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(AnyWithKey, Any, ForSeq)
#define LOCAL_DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSetOrMap               LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(AnyWithKey, Any, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultAnyWithRangeWithCallAttrAny                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(AnyWithRange, Any)
#define LOCAL_DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrAny                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(AnyWithRangeAndKey, Any)
#define LOCAL_DeeSeq_DefaultAllWithCallAttrAll                                 LOCAL_DeeSeq_DefaultFooWithCallAttrBar(All, All)
#define LOCAL_DeeSeq_DefaultAllWithKeyWithCallAttrAllForSeq                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(AllWithKey, All, ForSeq)
#define LOCAL_DeeSeq_DefaultAllWithKeyWithCallAttrAllForSetOrMap               LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(AllWithKey, All, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultAllWithRangeWithCallAttrAll                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(AllWithRange, All)
#define LOCAL_DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrAll                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(AllWithRangeAndKey, All)
#define LOCAL_DeeSeq_DefaultParityWithCallAttrParity                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Parity, Parity)
#define LOCAL_DeeSeq_DefaultParityWithKeyWithCallAttrParityForSeq              LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(ParityWithKey, Parity, ForSeq)
#define LOCAL_DeeSeq_DefaultParityWithKeyWithCallAttrParityForSetOrMap         LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(ParityWithKey, Parity, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultParityWithRangeWithCallAttrParity                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(ParityWithRange, Parity)
#define LOCAL_DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrParity            LOCAL_DeeSeq_DefaultFooWithCallAttrBar(ParityWithRangeAndKey, Parity)
#define LOCAL_DeeSeq_DefaultReduceWithCallAttrReduce                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Reduce, Reduce)
#define LOCAL_DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSeq             LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(ReduceWithInit, Reduce, ForSeq)
#define LOCAL_DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSetOrMap        LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(ReduceWithInit, Reduce, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultReduceWithRangeWithCallAttrReduce                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(ReduceWithRange, Reduce)
#define LOCAL_DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrReduce           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(ReduceWithRangeAndInit, Reduce)
#define LOCAL_DeeSeq_DefaultMinWithCallAttrMin                                 LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Min, Min)
#define LOCAL_DeeSeq_DefaultMinWithKeyWithCallAttrMinForSeq                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(MinWithKey, Min, ForSeq)
#define LOCAL_DeeSeq_DefaultMinWithKeyWithCallAttrMinForSetOrMap               LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(MinWithKey, Min, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultMinWithRangeWithCallAttrMin                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(MinWithRange, Min)
#define LOCAL_DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrMin                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(MinWithRangeAndKey, Min)
#define LOCAL_DeeSeq_DefaultMaxWithCallAttrMax                                 LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Max, Max)
#define LOCAL_DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSeq                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(MaxWithKey, Max, ForSeq)
#define LOCAL_DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSetOrMap               LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(MaxWithKey, Max, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultMaxWithRangeWithCallAttrMax                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(MaxWithRange, Max)
#define LOCAL_DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrMax                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(MaxWithRangeAndKey, Max)
#define LOCAL_DeeSeq_DefaultSumWithCallAttrSum                                 LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Sum, Sum)
#define LOCAL_DeeSeq_DefaultSumWithRangeWithCallAttrSum                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SumWithRange, Sum)
#define LOCAL_DeeSeq_DefaultCountWithCallAttrCount                             LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Count, Count)
#define LOCAL_DeeSeq_DefaultCountWithKeyWithCallAttrCountForSeq                LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(CountWithKey, Count, ForSeq)
#define LOCAL_DeeSeq_DefaultCountWithKeyWithCallAttrCountForSetOrMap           LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(CountWithKey, Count, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultCountWithRangeWithCallAttrCount                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(CountWithRange, Count)
#define LOCAL_DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrCount              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(CountWithRangeAndKey, Count)
#define LOCAL_DeeSeq_DefaultContainsWithCallAttrContains                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Contains, Contains)
#define LOCAL_DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSeq          LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(ContainsWithKey, Contains, ForSeq)
#define LOCAL_DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSetOrMap     LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(ContainsWithKey, Contains, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultContainsWithRangeWithCallAttrContains              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(ContainsWithRange, Contains)
#define LOCAL_DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrContains        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(ContainsWithRangeAndKey, Contains)
#define LOCAL_DeeSeq_DefaultLocateWithCallAttrLocateForSeq                     LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(Locate, Locate, ForSeq)
#define LOCAL_DeeSeq_DefaultLocateWithCallAttrLocateForSetOrMap                LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(Locate, Locate, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultLocateWithRangeWithCallAttrLocate                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(LocateWithRange, Locate)
#define LOCAL_DeeSeq_DefaultRLocateWithRangeWithCallAttrRLocate                LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RLocateWithRange, RLocate)
#define LOCAL_DeeSeq_DefaultStartsWithWithCallAttrStartsWith                   LOCAL_DeeSeq_DefaultFooWithCallAttrBar(StartsWith, StartsWith)
#define LOCAL_DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSeq      LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(StartsWithWithKey, StartsWith, ForSeq)
#define LOCAL_DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSetOrMap LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(StartsWithWithKey, StartsWith, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultStartsWithWithRangeWithCallAttrStartsWith          LOCAL_DeeSeq_DefaultFooWithCallAttrBar(StartsWithWithRange, StartsWith)
#define LOCAL_DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrStartsWith    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(StartsWithWithRangeAndKey, StartsWith)
#define LOCAL_DeeSeq_DefaultEndsWithWithCallAttrEndsWith                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(EndsWith, EndsWith)
#define LOCAL_DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSeq          LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(EndsWithWithKey, EndsWith, ForSeq)
#define LOCAL_DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSetOrMap     LOCAL_DeeSeq_DefaultFooWithCallAttrBar_(EndsWithWithKey, EndsWith, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultEndsWithWithRangeWithCallAttrEndsWith              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(EndsWithWithRange, EndsWith)
#define LOCAL_DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrEndsWith        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(EndsWithWithRangeAndKey, EndsWith)
#define LOCAL_DeeSeq_DefaultFindWithCallAttrFind                               LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Find, Find)
#define LOCAL_DeeSeq_DefaultFindWithKeyWithCallAttrFind                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(FindWithKey, Find)
#define LOCAL_DeeSeq_DefaultRFindWithCallAttrRFind                             LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RFind, RFind)
#define LOCAL_DeeSeq_DefaultRFindWithKeyWithCallAttrRFind                      LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RFindWithKey, RFind)
#define LOCAL_DeeSeq_DefaultEraseWithCallAttrErase                             LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Erase, Erase)
#define LOCAL_DeeSeq_DefaultInsertWithCallAttrInsert                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Insert, Insert)
#define LOCAL_DeeSeq_DefaultInsertAllWithCallAttrInsertAll                     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(InsertAll, InsertAll)
#define LOCAL_DeeSeq_DefaultPushFrontWithCallAttrPushFront                     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(PushFront, PushFront)
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrAppend                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Append, Append)
#define LOCAL_DeeSeq_DefaultAppendWithCallAttrPushBack                         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Append, PushBack)
#define LOCAL_DeeSeq_DefaultExtendWithCallAttrExtend                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Extend, Extend)
#define LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(XchItemIndex, XchItem)
#define LOCAL_DeeSeq_DefaultClearWithCallAttrClear                             LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Clear, Clear)
#define LOCAL_DeeSeq_DefaultPopWithCallAttrPop                                 LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Pop, Pop)
#define LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Remove, Remove)
#define LOCAL_DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveWithKey, Remove)
#define LOCAL_DeeSeq_DefaultRRemoveWithCallAttrRRemove                         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RRemove, RRemove)
#define LOCAL_DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove                  LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RRemoveWithKey, RRemove)
#define LOCAL_DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll                     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveAll, RemoveAll)
#define LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveAllWithKey, RemoveAll)
#define LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(RemoveIf, RemoveIf)
#define LOCAL_DeeSeq_DefaultResizeWithCallAttrResize                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Resize, Resize)
#define LOCAL_DeeSeq_DefaultFillWithCallAttrFill                               LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Fill, Fill)
#define LOCAL_DeeSeq_DefaultReverseWithCallAttrReverse                         LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Reverse, Reverse)
#define LOCAL_DeeSeq_DefaultReversedWithCallAttrReversed                       LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Reversed, Reversed)
#define LOCAL_DeeSeq_DefaultSortWithCallAttrSort                               LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Sort, Sort)
#define LOCAL_DeeSeq_DefaultSortWithKeyWithCallAttrSort                        LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SortWithKey, Sort)
#define LOCAL_DeeSeq_DefaultSortedWithCallAttrSorted                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(Sorted, Sorted)
#define LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(SortedWithKey, Sorted)
#define LOCAL_DeeSeq_DefaultBFindWithCallAttrBFind                             LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BFind, BFind)
#define LOCAL_DeeSeq_DefaultBFindWithKeyWithCallAttrBFind                      LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BFindWithKey, BFind)
#define LOCAL_DeeSeq_DefaultBPositionWithCallAttrBPosition                     LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BPosition, BPosition)
#define LOCAL_DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition              LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BPositionWithKey, BPosition)
#define LOCAL_DeeSeq_DefaultBRangeWithCallAttrBRange                           LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BRange, BRange)
#define LOCAL_DeeSeq_DefaultBRangeWithKeyWithCallAttrBRange                    LOCAL_DeeSeq_DefaultFooWithCallAttrBar(BRangeWithKey, BRange)
#define LOCAL_DeeSet_DefaultInsertWithCallAttrInsert                           LOCAL_DeeSet_DefaultFooWithCallAttrBar(Insert, Insert)
#define LOCAL_DeeSet_DefaultRemoveWithCallAttrRemove                           LOCAL_DeeSet_DefaultFooWithCallAttrBar(Remove, Remove)
#define LOCAL_DeeSet_DefaultUnifyWithCallAttrUnify                             LOCAL_DeeSet_DefaultFooWithCallAttrBar(Unify, Unify)
#define LOCAL_DeeSet_DefaultInsertAllWithCallAttrInsertAll                     LOCAL_DeeSet_DefaultFooWithCallAttrBar(InsertAll, InsertAll)
#define LOCAL_DeeSet_DefaultRemoveAllWithCallAttrRemoveAll                     LOCAL_DeeSet_DefaultFooWithCallAttrBar(RemoveAll, RemoveAll)
#define LOCAL_DeeSet_DefaultPopWithCallAttrPop                                 LOCAL_DeeSet_DefaultFooWithCallAttrBar(Pop, Pop)
#define LOCAL_DeeSet_DefaultPopWithDefaultWithCallAttrPop                      LOCAL_DeeSet_DefaultFooWithCallAttrBar(PopWithDefault, Pop)
#define LOCAL_DeeMap_DefaultSetOldWithCallAttrSetOld                           LOCAL_DeeMap_DefaultFooWithCallAttrBar(SetOld, SetOld)
#define LOCAL_DeeMap_DefaultSetOldExWithCallAttrSetOldEx                       LOCAL_DeeMap_DefaultFooWithCallAttrBar(SetOldEx, SetOldEx)
#define LOCAL_DeeMap_DefaultSetNewWithCallAttrSetNew                           LOCAL_DeeMap_DefaultFooWithCallAttrBar(SetNew, SetNew)
#define LOCAL_DeeMap_DefaultSetNewExWithCallAttrSetNewEx                       LOCAL_DeeMap_DefaultFooWithCallAttrBar(SetNewEx, SetNewEx)
#define LOCAL_DeeMap_DefaultSetDefaultWithCallAttrSetDefault                   LOCAL_DeeMap_DefaultFooWithCallAttrBar(SetDefault, SetDefault)
#define LOCAL_DeeMap_DefaultUpdateWithCallAttrUpdate                           LOCAL_DeeMap_DefaultFooWithCallAttrBar(Update, Update)
#define LOCAL_DeeMap_DefaultRemoveWithCallAttrRemove                           LOCAL_DeeMap_DefaultFooWithCallAttrBar(Remove, Remove)
#define LOCAL_DeeMap_DefaultRemoveKeysWithCallAttrRemoveKeys                   LOCAL_DeeMap_DefaultFooWithCallAttrBar(RemoveKeys, RemoveKeys)
#define LOCAL_DeeMap_DefaultPopWithCallAttrPop                                 LOCAL_DeeMap_DefaultFooWithCallAttrBar(Pop, Pop)
#define LOCAL_DeeMap_DefaultPopWithDefaultWithCallAttrPop                      LOCAL_DeeMap_DefaultFooWithCallAttrBar(PopWithDefault, Pop)
#define LOCAL_DeeMap_DefaultPopItemWithCallAttrPopItem                         LOCAL_DeeMap_DefaultFooWithCallAttrBar(PopItem, PopItem)
#define LOCAL_DeeMap_DefaultKeysWithCallAttrKeys                               LOCAL_DeeMap_DefaultFooWithCallAttrBar(Keys, Keys)
#define LOCAL_DeeMap_DefaultValuesWithCallAttrValues                           LOCAL_DeeMap_DefaultFooWithCallAttrBar(Values, Values)
#define LOCAL_DeeMap_DefaultIterKeysWithCallAttrIterKeys                       LOCAL_DeeMap_DefaultFooWithCallAttrBar(IterKeys, IterKeys)
#define LOCAL_DeeMap_DefaultIterValuesWithCallAttrIterValues                   LOCAL_DeeMap_DefaultFooWithCallAttrBar(IterValues, IterValues)



#ifdef DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#define LOCAL_DeeObject_GetAttr(self, tsc_foo_data, attr) \
	DeeObject_GetAttr(self, (DeeObject *)(attr))
#define LOCAL_DeeObject_BoundAttr(self, tsc_foo_data, attr) \
	DeeObject_BoundAttr(self, (DeeObject *)(attr))
#define LOCAL_DeeObject_DelAttr(self, tsc_foo_data, attr) \
	DeeObject_DelAttr(self, (DeeObject *)(attr))
#define LOCAL_DeeObject_SetAttr(self, tsc_foo_data, attr, value) \
	DeeObject_SetAttr(self, (DeeObject *)(attr), value)
#define LOCAL_DeeObject_CallAttr(self, tsc_foo_data, attr, argc, argv) \
	DeeObject_CallAttr(self, (DeeObject *)(attr), argc, argv)
#define LOCAL_DeeObject_CallAttrf(self, tsc_foo_data, attr, ...) \
	DeeObject_CallAttrf(self, (DeeObject *)(attr), __VA_ARGS__)
#elif defined(DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction)
#define LOCAL_DeeObject_GetAttr(self, tsc_foo_data, attr) \
	DeeObject_ThisCall(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_function, self, 0, NULL)
#define LOCAL_DeeObject_BoundAttr(self, tsc_foo_data, attr) \
	call_getter_for_bound(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_function, self)
#define LOCAL_DeeObject_DelAttr(self, tsc_foo_data, attr) \
	call_delete(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_function, self)
#define LOCAL_DeeObject_SetAttr(self, tsc_foo_data, attr, value) \
	call_setter(Dee_TYPE(self)->tp_seq->_tp_seqcache->tsc_foo_data.d_function, self, value)
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

#ifndef Dee_int_SIZE_MAX_DEFINED
#define Dee_int_SIZE_MAX_DEFINED
#if __SIZEOF_SIZE_T__ == 4
DEFINE_UINT32(Dee_int_SIZE_MAX, (uint32_t)-1);
#elif __SIZEOF_SIZE_T__ == 8
DEFINE_UINT64(Dee_int_SIZE_MAX, (uint64_t)-1);
#elif __SIZEOF_SIZE_T__ == 2
DEFINE_UINT16(Dee_int_SIZE_MAX, (uint16_t)-1);
#elif __SIZEOF_SIZE_T__ == 1
DEFINE_UINT8(Dee_int_SIZE_MAX, (uint8_t)-1);
#else /* __SIZEOF_SIZE_T__ == ... */
#error "Unsupported __SIZEOF_SIZE_T__"
#endif /* __SIZEOF_SIZE_T__ != ... */
#endif /* !Dee_int_SIZE_MAX_DEFINED */

/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultAnyWithCallAttrAny(DeeObject *self) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttr(self, tsc_seq_any_data, &str_any, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSeq(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	DREF DeeObject *result;
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_any_data, &str_any, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSetOrMap(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_any_data, &str_any, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultAnyWithRangeWithCallAttrAny(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_any_data, &str_any,
	                                                   PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
LOCAL_DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrAny(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_any_data, &str_any,
	                                                   PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultAllWithCallAttrAll(DeeObject *self) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttr(self, tsc_seq_all_data, &str_all, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultAllWithKeyWithCallAttrAllForSeq(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	DREF DeeObject *result;
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_all_data, &str_all, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultAllWithKeyWithCallAttrAllForSetOrMap(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_all_data, &str_all, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultAllWithRangeWithCallAttrAll(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_all_data, &str_all,
	                                                   PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
LOCAL_DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrAll(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_all_data, &str_all,
	                                                   PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultParityWithCallAttrParity(DeeObject *self) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttr(self, tsc_seq_parity_data, &str_parity, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultParityWithKeyWithCallAttrParityForSeq(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	DREF DeeObject *result;
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_parity_data, &str_parity, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultParityWithKeyWithCallAttrParityForSetOrMap(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_parity_data, &str_parity, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultParityWithRangeWithCallAttrParity(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_parity_data, &str_parity,
	                                                   PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
LOCAL_DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrParity(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_parity_data, &str_parity,
	                                                   PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultReduceWithCallAttrReduce(DeeObject *self, DeeObject *combine) {
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_reduce_data, &str_reduce, 1, &combine);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSeq(DeeObject *self, DeeObject *combine, DeeObject *init) {
	DeeObject *args[4];
	args[0] = combine;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = init;
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_reduce_data, &str_reduce, 4, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init) {
	DeeObject *args[2];
	args[0] = combine;
	args[1] = init;
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_reduce_data, &str_reduce, 2, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultReduceWithRangeWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_reduce_data, &str_reduce, "o" PCKuSIZ PCKuSIZ, combine, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_reduce_data, &str_reduce, "o" PCKuSIZ PCKuSIZ "o", combine, start, end, init);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMinWithCallAttrMin(DeeObject *self) {
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_min_data, &str_min, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMinWithKeyWithCallAttrMinForSeq(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_min_data, &str_min, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMinWithKeyWithCallAttrMinForSetOrMap(DeeObject *self, DeeObject *key) {
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_min_data, &str_min, 1, &key);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMinWithRangeWithCallAttrMin(DeeObject *self, size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_min_data, &str_min, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrMin(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_min_data, &str_min, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMaxWithCallAttrMax(DeeObject *self) {
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_max_data, &str_max, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSeq(DeeObject *self, DeeObject *key) {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_max_data, &str_max, 3, args);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSetOrMap(DeeObject *self, DeeObject *key) {
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_max_data, &str_max, 1, &key);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMaxWithRangeWithCallAttrMax(DeeObject *self, size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_max_data, &str_max, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrMax(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_max_data, &str_max, PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultSumWithCallAttrSum(DeeObject *self) {
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_sum_data, &str_sum, 0, NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultSumWithRangeWithCallAttrSum(DeeObject *self, size_t start, size_t end) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_sum_data, &str_sum, PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultCountWithCallAttrCount(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_count_data, &str_count, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
LOCAL_DeeSeq_DefaultCountWithKeyWithCallAttrCountForSeq(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_count_data, &str_count, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
LOCAL_DeeSeq_DefaultCountWithKeyWithCallAttrCountForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = item;
	args[1] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_count_data, &str_count, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultCountWithRangeWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_count_data, &str_count, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
LOCAL_DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_count_data, &str_count, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultContainsWithCallAttrContains(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_contains_data, &str_contains, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSeq(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_contains_data, &str_contains, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = item;
	args[1] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_contains_data, &str_contains, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultContainsWithRangeWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_contains_data, &str_contains, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_contains_data, &str_contains, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultLocateWithCallAttrLocateForSeq(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[4];
	args[0] = match;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = def;
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_locate_data, &str_locate, 4, args);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultLocateWithCallAttrLocateForSetOrMap(DeeObject *self, DeeObject *match, DeeObject *def) {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return LOCAL_DeeObject_CallAttr(self, tsc_seq_locate_data, &str_locate, 2, args);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultLocateWithRangeWithCallAttrLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *def) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_locate_data, &str_locate, "o" PCKuSIZ PCKuSIZ "o", item, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultRLocateWithRangeWithCallAttrRLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *def) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_rlocate_data, &str_rlocate, "o" PCKuSIZ PCKuSIZ "o", item, start, end, def);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultStartsWithWithCallAttrStartsWith(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_startswith_data, &str_startswith, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_startswith_data, &str_startswith, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = item;
	args[1] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_startswith_data, &str_startswith, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultStartsWithWithRangeWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_startswith_data, &str_startswith, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_startswith_data, &str_startswith, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultEndsWithWithCallAttrEndsWith(DeeObject *self, DeeObject *item) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_endswith_data, &str_endswith, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_endswith_data, &str_endswith, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = item;
	args[1] = key;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_endswith_data, &str_endswith, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultEndsWithWithRangeWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_endswith_data, &str_endswith, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_endswith_data, &str_endswith, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}




/* Mutable sequence functions */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultFindWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_find_data, &str_find, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_find_data, &str_find, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_rfind_data, &str_rfind, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_rfind_data, &str_rfind, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_erase_data, &str_erase, PCKuSIZ PCKuSIZ, index, count);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_insert_data, &str_insert, PCKuSIZ "o", index, item);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_insertall_data, &str_insertall, PCKuSIZ "o", index, items);
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
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_pushfront_data, &str_pushfront, 1, &item);
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
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_append_data, &str_append, 1, &item);
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
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_append_data, &str_pushback, 1, &item);
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
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_extend_data, &str_extend, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultXchItemIndexWithCallAttrXchItem(DeeObject *self, size_t index, DeeObject *value) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_xchitem_data, &str_xchitem, PCKuSIZ "o", index, value);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultClearWithCallAttrClear(DeeObject *self) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_seq_clear_data, &str_clear, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultPopWithCallAttrPop(DeeObject *self, Dee_ssize_t index) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_pop_data, &str_pop, PCKdSIZ, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *item,
                                             size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_remove_data, &str_remove,
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_remove_data, &str_remove,
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_rremove_data, &str_rremove,
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_rremove_data, &str_rremove,
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
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_removeall_data, &str_removeall,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ,
	                                   item, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
LOCAL_DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll(DeeObject *self, DeeObject *item,
                                                          size_t start, size_t end,
                                                          size_t max, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_removeall_data, &str_removeall,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ "o",
	                                   item, start, end, max, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf(DeeObject *self, DeeObject *should,
                                                 size_t start, size_t end, size_t max) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_removeif_data, &str_removeif,
	                                   "o" PCKuSIZ PCKuSIZ PCKuSIZ,
	                                   should, start, end, max);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_DeeSeq_DefaultResizeWithCallAttrResize(DeeObject *self, size_t newsize, DeeObject *filler) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_resize_data, &str_resize,
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_fill_data, &str_fill,
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_reverse_data, &str_reverse,
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
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_reversed_data, &str_reversed,
	                                 PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultSortWithCallAttrSort(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_sort_data, &str_sort,
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_sort_data, &str_sort,
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
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_sorted_data, &str_sorted,
	                                 PCKuSIZ PCKuSIZ, start, end);
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultSortedWithKeyWithCallAttrSorted(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return LOCAL_DeeObject_CallAttrf(self, tsc_seq_sorted_data, &str_sorted,
	                                 PCKuSIZ PCKuSIZ "o", start, end, key);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
LOCAL_DeeSeq_DefaultBFindWithCallAttrBFind(DeeObject *self, DeeObject *item,
                                           size_t start, size_t end) {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_bfind_data, &str_bfind, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_bfind_data, &str_bfind, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_bposition_data, &str_bposition, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_bposition_data, &str_bposition, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_brange_data, &str_brange, "o" PCKuSIZ PCKuSIZ, item, start, end);
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
	result = LOCAL_DeeObject_CallAttrf(self, tsc_seq_brange_data, &str_brange, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
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



/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSet_DefaultInsertWithCallAttrInsert(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_set_insert_data, &str_insert, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSet_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_set_remove_data, &str_remove, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSet_DefaultUnifyWithCallAttrUnify(DeeObject *self, DeeObject *key) {
	return LOCAL_DeeObject_CallAttr(self, tsc_set_unify_data, &str_unify, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSet_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_set_insertall_data, &str_insertall, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSet_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_set_removeall_data, &str_removeall, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSet_DefaultPopWithCallAttrPop(DeeObject *self) {
	return LOCAL_DeeObject_CallAttr(self, tsc_set_pop_data, &str_pop, 0, NULL);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeSet_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *default_) {
	return LOCAL_DeeObject_CallAttr(self, tsc_set_pop_data, &str_pop, 1, &default_);
}



/************************************************************************/
/* For `deemon.Mapping'                                                 */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeMap_DefaultSetOldWithCallAttrSetOld(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_setold_data, &str_setold, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultSetOldExWithCallAttrSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_setold_ex_data, &str_setold_ex, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp)
		return status[1];
	Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
	return ITER_DONE;
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_DeeMap_DefaultSetNewWithCallAttrSetNew(DeeObject *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_setnew_data, &str_setnew, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultSetNewExWithCallAttrSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value) {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_setnew_ex_data, &str_setnew_ex, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultSetDefaultWithCallAttrSetDefault(DeeObject *self, DeeObject *key, DeeObject *value) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	return LOCAL_DeeObject_CallAttr(self, tsc_map_setdefault_data, &str_setdefault, 2, args);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeMap_DefaultUpdateWithCallAttrUpdate(DeeObject *self, DeeObject *items) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_update_data, &str_update, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeMap_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_remove_data, &str_remove, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeMap_DefaultRemoveKeysWithCallAttrRemoveKeys(DeeObject *self, DeeObject *keys) {
	DREF DeeObject *result;
	result = LOCAL_DeeObject_CallAttr(self, tsc_map_removekeys_data, &str_removekeys, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultPopWithCallAttrPop(DeeObject *self, DeeObject *key) {
	return LOCAL_DeeObject_CallAttr(self, tsc_map_pop_data, &str_pop, 1, &key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *key, DeeObject *default_) {
	DeeObject *args[2];
	args[0] = key;
	args[1] = default_;
	return LOCAL_DeeObject_CallAttr(self, tsc_map_pop_data, &str_pop, 2, args);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultPopItemWithCallAttrPopItem(DeeObject *self) {
	return LOCAL_DeeObject_CallAttr(self, tsc_map_popitem_data, &str_popitem, 0, NULL);
}

#ifdef LOCAL_DeeObject_GetAttr
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultGetFirstWithCallAttrGetFirst(DeeObject *__restrict self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_seq_getfirst_data, &str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultBoundFirstWithCallAttrGetFirst(DeeObject *__restrict self) {
	return LOCAL_DeeObject_BoundAttr(self, tsc_seq_getfirst_data, &str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultDelFirstWithCallAttrDelFirst(DeeObject *__restrict self) {
	return LOCAL_DeeObject_DelAttr(self, tsc_seq_delfirst_data, &str_first);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultSetFirstWithCallAttrSetFirst(DeeObject *self, DeeObject *value) {
	return LOCAL_DeeObject_SetAttr(self, tsc_seq_delfirst_data, &str_first, value);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultGetLastWithCallAttrGetLast(DeeObject *__restrict self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_seq_getlast_data, &str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultBoundLastWithCallAttrGetLast(DeeObject *__restrict self) {
	return LOCAL_DeeObject_BoundAttr(self, tsc_seq_getlast_data, &str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
LOCAL_DeeSeq_DefaultDelLastWithCallAttrDelLast(DeeObject *__restrict self) {
	return LOCAL_DeeObject_DelAttr(self, tsc_seq_dellast_data, &str_last);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_DeeSeq_DefaultSetLastWithCallAttrSetLast(DeeObject *self, DeeObject *value) {
	return LOCAL_DeeObject_SetAttr(self, tsc_seq_dellast_data, &str_last, value);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeSeq_DefaultCachedWithCallAttrCached(DeeObject *__restrict self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_seq_cached_data, &str_cached);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultKeysWithCallAttrKeys(DeeObject *self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_map_keys_data, &str_keys);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultValuesWithCallAttrValues(DeeObject *self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_map_values_data, &str_values);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultIterKeysWithCallAttrIterKeys(DeeObject *self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_map_iterkeys_data, &str_iterkeys);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeMap_DefaultIterValuesWithCallAttrIterValues(DeeObject *self) {
	return LOCAL_DeeObject_GetAttr(self, tsc_map_itervalues_data, &str_itervalues);
}
#endif /* LOCAL_DeeObject_GetAttr */

#undef LOCAL_DeeObject_GetAttr
#undef LOCAL_DeeObject_BoundAttr
#undef LOCAL_DeeObject_DelAttr
#undef LOCAL_DeeObject_SetAttr
#undef LOCAL_DeeObject_CallAttr
#undef LOCAL_DeeObject_CallAttrf

#undef LOCAL_DeeSeq_DefaultGetFirstWithCallAttrGetFirst
#undef LOCAL_DeeSeq_DefaultBoundFirstWithCallAttrBoundFirst
#undef LOCAL_DeeSeq_DefaultDelFirstWithCallAttrDelFirst
#undef LOCAL_DeeSeq_DefaultSetFirstWithCallAttrSetFirst
#undef LOCAL_DeeSeq_DefaultGetLastWithCallAttrGetLast
#undef LOCAL_DeeSeq_DefaultBoundLastWithCallAttrBoundLast
#undef LOCAL_DeeSeq_DefaultDelLastWithCallAttrDelLast
#undef LOCAL_DeeSeq_DefaultSetLastWithCallAttrSetLast
#undef LOCAL_DeeSeq_DefaultCachedWithCallAttrCached
#undef LOCAL_DeeSeq_DefaultAnyWithCallAttrAny
#undef LOCAL_DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSeq
#undef LOCAL_DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSetOrMap
#undef LOCAL_DeeSeq_DefaultAnyWithRangeWithCallAttrAny
#undef LOCAL_DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrAny
#undef LOCAL_DeeSeq_DefaultAllWithCallAttrAll
#undef LOCAL_DeeSeq_DefaultAllWithKeyWithCallAttrAllForSeq
#undef LOCAL_DeeSeq_DefaultAllWithKeyWithCallAttrAllForSetOrMap
#undef LOCAL_DeeSeq_DefaultAllWithRangeWithCallAttrAll
#undef LOCAL_DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrAll
#undef LOCAL_DeeSeq_DefaultParityWithCallAttrParity
#undef LOCAL_DeeSeq_DefaultParityWithKeyWithCallAttrParityForSeq
#undef LOCAL_DeeSeq_DefaultParityWithKeyWithCallAttrParityForSetOrMap
#undef LOCAL_DeeSeq_DefaultParityWithRangeWithCallAttrParity
#undef LOCAL_DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrParity
#undef LOCAL_DeeSeq_DefaultReduceWithCallAttrReduce
#undef LOCAL_DeeSeq_DefaultReduceWithInitWithCallAttrReduce
#undef LOCAL_DeeSeq_DefaultReduceWithRangeWithCallAttrReduce
#undef LOCAL_DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrReduce
#undef LOCAL_DeeSeq_DefaultMinWithCallAttrMin
#undef LOCAL_DeeSeq_DefaultMinWithKeyWithCallAttrMinForSeq
#undef LOCAL_DeeSeq_DefaultMinWithKeyWithCallAttrMinForSetOrMap
#undef LOCAL_DeeSeq_DefaultMinWithRangeWithCallAttrMin
#undef LOCAL_DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrMin
#undef LOCAL_DeeSeq_DefaultMaxWithCallAttrMax
#undef LOCAL_DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSeq
#undef LOCAL_DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSetOrMap
#undef LOCAL_DeeSeq_DefaultMaxWithRangeWithCallAttrMax
#undef LOCAL_DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrMax
#undef LOCAL_DeeSeq_DefaultSumWithCallAttrSum
#undef LOCAL_DeeSeq_DefaultSumWithRangeWithCallAttrSum
#undef LOCAL_DeeSeq_DefaultCountWithCallAttrCount
#undef LOCAL_DeeSeq_DefaultCountWithKeyWithCallAttrCountForSeq
#undef LOCAL_DeeSeq_DefaultCountWithKeyWithCallAttrCountForSetOrMap
#undef LOCAL_DeeSeq_DefaultCountWithRangeWithCallAttrCount
#undef LOCAL_DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrCount
#undef LOCAL_DeeSeq_DefaultContainsWithCallAttrContains
#undef LOCAL_DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSeq
#undef LOCAL_DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSetOrMap
#undef LOCAL_DeeSeq_DefaultContainsWithRangeWithCallAttrContains
#undef LOCAL_DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrContains
#undef LOCAL_DeeSeq_DefaultLocateWithCallAttrLocateForSeq
#undef LOCAL_DeeSeq_DefaultLocateWithCallAttrLocateForSetOrMap
#undef LOCAL_DeeSeq_DefaultLocateWithRangeWithCallAttrLocate
#undef LOCAL_DeeSeq_DefaultRLocateWithRangeWithCallAttrRLocate
#undef LOCAL_DeeSeq_DefaultStartsWithWithCallAttrStartsWith
#undef LOCAL_DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSeq
#undef LOCAL_DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSetOrMap
#undef LOCAL_DeeSeq_DefaultStartsWithWithRangeWithCallAttrStartsWith
#undef LOCAL_DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrStartsWith
#undef LOCAL_DeeSeq_DefaultEndsWithWithCallAttrEndsWith
#undef LOCAL_DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSeq
#undef LOCAL_DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSetOrMap
#undef LOCAL_DeeSeq_DefaultEndsWithWithRangeWithCallAttrEndsWith
#undef LOCAL_DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrEndsWith
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
#undef LOCAL_DeeSet_DefaultInsertWithCallAttrInsert
#undef LOCAL_DeeSet_DefaultRemoveWithCallAttrRemove
#undef LOCAL_DeeSet_DefaultUnifyWithCallAttrUnify
#undef LOCAL_DeeSet_DefaultInsertAllWithCallAttrInsertAll
#undef LOCAL_DeeSet_DefaultRemoveAllWithCallAttrRemoveAll
#undef LOCAL_DeeSet_DefaultPopWithCallAttrPop
#undef LOCAL_DeeSet_DefaultPopWithDefaultWithCallAttrPop
#undef LOCAL_DeeMap_DefaultSetOldWithCallAttrSetOld
#undef LOCAL_DeeMap_DefaultSetOldExWithCallAttrSetOldEx
#undef LOCAL_DeeMap_DefaultSetNewWithCallAttrSetNew
#undef LOCAL_DeeMap_DefaultSetNewExWithCallAttrSetNewEx
#undef LOCAL_DeeMap_DefaultSetDefaultWithCallAttrSetDefault
#undef LOCAL_DeeMap_DefaultUpdateWithCallAttrUpdate
#undef LOCAL_DeeMap_DefaultRemoveWithCallAttrRemove
#undef LOCAL_DeeMap_DefaultRemoveKeysWithCallAttrRemoveKeys
#undef LOCAL_DeeMap_DefaultPopWithCallAttrPop
#undef LOCAL_DeeMap_DefaultPopWithDefaultWithCallAttrPop
#undef LOCAL_DeeMap_DefaultPopItemWithCallAttrPopItem
#undef LOCAL_DeeMap_DefaultKeysWithCallAttrKeys
#undef LOCAL_DeeMap_DefaultValuesWithCallAttrValues
#undef LOCAL_DeeMap_DefaultIterKeysWithCallAttrIterKeys
#undef LOCAL_DeeMap_DefaultIterValuesWithCallAttrIterValues

#undef LOCAL_DeeApi_DefaultFooWithCallAttrBar
#undef LOCAL_DeeApi_DefaultFooWithCallAttrBar_
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBar
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBar_
#undef LOCAL_DeeSet_DefaultFooWithCallAttrBar
#undef LOCAL_DeeSet_DefaultFooWithCallAttrBar_
#undef LOCAL_DeeMap_DefaultFooWithCallAttrBar
#undef LOCAL_DeeMap_DefaultFooWithCallAttrBar_

DECL_END

#undef DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
#undef DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod

