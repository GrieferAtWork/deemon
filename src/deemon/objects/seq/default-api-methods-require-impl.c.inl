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
//#define DEFINE_DeeType_SeqCache_RequireFind
//#define DEFINE_DeeType_SeqCache_RequireFindWithKey
//#define DEFINE_DeeType_SeqCache_RequireRFind
//#define DEFINE_DeeType_SeqCache_RequireRFindWithKey
//#define DEFINE_DeeType_SeqCache_RequireErase
//#define DEFINE_DeeType_SeqCache_RequireInsert
//#define DEFINE_DeeType_SeqCache_RequireInsertAll
//#define DEFINE_DeeType_SeqCache_RequirePushFront
//#define DEFINE_DeeType_SeqCache_RequireAppend
//#define DEFINE_DeeType_SeqCache_RequireExtend
//#define DEFINE_DeeType_SeqCache_RequireXchItemIndex
//#define DEFINE_DeeType_SeqCache_RequireClear
//#define DEFINE_DeeType_SeqCache_RequirePop
//#define DEFINE_DeeType_SeqCache_RequireRemove
//#define DEFINE_DeeType_SeqCache_RequireRemoveWithKey
//#define DEFINE_DeeType_SeqCache_RequireRRemove
//#define DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
//#define DEFINE_DeeType_SeqCache_RequireRemoveAll
//#define DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
//#define DEFINE_DeeType_SeqCache_RequireRemoveIf
//#define DEFINE_DeeType_SeqCache_RequireResize
//#define DEFINE_DeeType_SeqCache_RequireFill
//#define DEFINE_DeeType_SeqCache_RequireReverse
//#define DEFINE_DeeType_SeqCache_RequireReversed
//#define DEFINE_DeeType_SeqCache_RequireSort
//#define DEFINE_DeeType_SeqCache_RequireSortWithKey
//#define DEFINE_DeeType_SeqCache_RequireSorted
//#define DEFINE_DeeType_SeqCache_RequireSortedWithKey
//#define DEFINE_DeeType_SeqCache_RequireBFind
#define DEFINE_DeeType_SeqCache_RequireBFindWithKey
//#define DEFINE_DeeType_SeqCache_RequireBPosition
//#define DEFINE_DeeType_SeqCache_RequireBPositionWithKey
//#define DEFINE_DeeType_SeqCache_RequireBRange
//#define DEFINE_DeeType_SeqCache_RequireBRangeWithKey
//#define DEFINE_DeeType_SeqCache_RequireBLocate
//#define DEFINE_DeeType_SeqCache_RequireBLocateWithKey
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_SeqCache_RequireFind) +             \
     defined(DEFINE_DeeType_SeqCache_RequireFindWithKey) +      \
     defined(DEFINE_DeeType_SeqCache_RequireRFind) +            \
     defined(DEFINE_DeeType_SeqCache_RequireRFindWithKey) +     \
     defined(DEFINE_DeeType_SeqCache_RequireErase) +            \
     defined(DEFINE_DeeType_SeqCache_RequireInsert) +           \
     defined(DEFINE_DeeType_SeqCache_RequireInsertAll) +        \
     defined(DEFINE_DeeType_SeqCache_RequirePushFront) +        \
     defined(DEFINE_DeeType_SeqCache_RequireAppend) +           \
     defined(DEFINE_DeeType_SeqCache_RequireExtend) +           \
     defined(DEFINE_DeeType_SeqCache_RequireXchItemIndex) +     \
     defined(DEFINE_DeeType_SeqCache_RequireClear) +            \
     defined(DEFINE_DeeType_SeqCache_RequirePop) +              \
     defined(DEFINE_DeeType_SeqCache_RequireRemove) +           \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveWithKey) +    \
     defined(DEFINE_DeeType_SeqCache_RequireRRemove) +          \
     defined(DEFINE_DeeType_SeqCache_RequireRRemoveWithKey) +   \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveAll) +        \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey) + \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveIf) +         \
     defined(DEFINE_DeeType_SeqCache_RequireResize) +           \
     defined(DEFINE_DeeType_SeqCache_RequireFill) +             \
     defined(DEFINE_DeeType_SeqCache_RequireReverse) +          \
     defined(DEFINE_DeeType_SeqCache_RequireReversed) +         \
     defined(DEFINE_DeeType_SeqCache_RequireSort) +             \
     defined(DEFINE_DeeType_SeqCache_RequireSortWithKey) +      \
     defined(DEFINE_DeeType_SeqCache_RequireSorted) +           \
     defined(DEFINE_DeeType_SeqCache_RequireSortedWithKey) +    \
     defined(DEFINE_DeeType_SeqCache_RequireBFind) +            \
     defined(DEFINE_DeeType_SeqCache_RequireBFindWithKey) +     \
     defined(DEFINE_DeeType_SeqCache_RequireBPosition) +        \
     defined(DEFINE_DeeType_SeqCache_RequireBPositionWithKey) + \
     defined(DEFINE_DeeType_SeqCache_RequireBRange) +           \
     defined(DEFINE_DeeType_SeqCache_RequireBRangeWithKey) +    \
     defined(DEFINE_DeeType_SeqCache_RequireBLocate) +          \
     defined(DEFINE_DeeType_SeqCache_RequireBLocateWithKey)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeType_SeqCache_Require... */

DECL_BEGIN

#ifdef DEFINE_DeeType_SeqCache_RequireFind
#define LOCAL_CANONICAL_NAME             find
#define LOCAL_generic_seq_foo            generic_seq_find
#define LOCAL_tsc_foo                    tsc_find
#define LOCAL_DeeSeq_AttrFoo             Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithEnumerateIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireFindWithKey)
#define LOCAL_CANONICAL_NAME             find
#define LOCAL_generic_seq_foo            generic_seq_find
#define LOCAL_tsc_foo                    tsc_find_with_key
#define LOCAL_DeeSeq_AttrFoo             FindWithKey
#define LOCAL_DeeSeq_AttrBar             Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithKeyWithEnumerateIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireRFind)
#define LOCAL_CANONICAL_NAME             rfind
#define LOCAL_generic_seq_foo            generic_seq_rfind
#define LOCAL_tsc_foo                    tsc_rfind
#define LOCAL_DeeSeq_AttrFoo             RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithEnumerateIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireRFindWithKey)
#define LOCAL_CANONICAL_NAME             rfind
#define LOCAL_generic_seq_foo            generic_seq_rfind
#define LOCAL_tsc_foo                    tsc_rfind_with_key
#define LOCAL_DeeSeq_AttrFoo             RFindWithKey
#define LOCAL_DeeSeq_AttrBar             RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithKeyWithEnumerateIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireErase)
#define LOCAL_CANONICAL_NAME  erase
#define LOCAL_generic_seq_foo generic_seq_erase
#define LOCAL_tsc_foo         tsc_erase
#define LOCAL_DeeSeq_AttrFoo  Erase
#elif defined(DEFINE_DeeType_SeqCache_RequireInsert)
#define LOCAL_CANONICAL_NAME  insert
#define LOCAL_generic_seq_foo generic_seq_insert
#define LOCAL_tsc_foo         tsc_insert
#define LOCAL_DeeSeq_AttrFoo  Insert
#elif defined(DEFINE_DeeType_SeqCache_RequireInsertAll)
#define LOCAL_CANONICAL_NAME  insertall
#define LOCAL_generic_seq_foo generic_seq_insertall
#define LOCAL_tsc_foo         tsc_insertall
#define LOCAL_DeeSeq_AttrFoo  InsertAll
#elif defined(DEFINE_DeeType_SeqCache_RequirePushFront)
#define LOCAL_CANONICAL_NAME             pushfront
#define LOCAL_generic_seq_foo            generic_seq_pushfront
#define LOCAL_tsc_foo                    tsc_pushfront
#define LOCAL_DeeSeq_AttrFoo             PushFront
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultPushFrontWithTSCInsert /* Use insert() by default */
#elif defined(DEFINE_DeeType_SeqCache_RequireAppend)
#define LOCAL_CANONICAL_NAME  append
#define LOCAL_generic_seq_foo generic_seq_append
#define LOCAL_tsc_foo         tsc_append
#define LOCAL_DeeSeq_AttrFoo  Append
#elif defined(DEFINE_DeeType_SeqCache_RequireExtend)
#define LOCAL_CANONICAL_NAME  extend
#define LOCAL_generic_seq_foo generic_seq_extend
#define LOCAL_tsc_foo         tsc_extend
#define LOCAL_DeeSeq_AttrFoo  Extend
#elif defined(DEFINE_DeeType_SeqCache_RequireXchItemIndex)
#define LOCAL_CANONICAL_NAME  xchitem
#define LOCAL_generic_seq_foo generic_seq_xchitem
#define LOCAL_tsc_foo         tsc_xchitem_index
#define LOCAL_tsc_foo_data    tsc_xchitem_data
#define LOCAL_DeeSeq_AttrFoo  XchItemIndex
#define LOCAL_DeeSeq_AttrBar  XchItem
#elif defined(DEFINE_DeeType_SeqCache_RequireClear)
#define LOCAL_CANONICAL_NAME  clear
#define LOCAL_generic_seq_foo generic_seq_clear
#define LOCAL_tsc_foo         tsc_clear
#define LOCAL_DeeSeq_AttrFoo  Clear
#elif defined(DEFINE_DeeType_SeqCache_RequirePop)
#define LOCAL_CANONICAL_NAME  pop
#define LOCAL_generic_seq_foo generic_seq_pop
#define LOCAL_tsc_foo         tsc_pop
#define LOCAL_DeeSeq_AttrFoo  Pop
#elif defined(DEFINE_DeeType_SeqCache_RequireRemove)
#define LOCAL_CANONICAL_NAME  remove
#define LOCAL_generic_seq_foo generic_seq_remove
#define LOCAL_tsc_foo         tsc_remove
#define LOCAL_DeeSeq_AttrFoo  Remove
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveWithKey)
#define LOCAL_CANONICAL_NAME  remove
#define LOCAL_generic_seq_foo generic_seq_remove
#define LOCAL_tsc_foo         tsc_remove_with_key
#define LOCAL_DeeSeq_AttrFoo  RemoveWithKey
#define LOCAL_DeeSeq_AttrBar  Remove
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemove)
#define LOCAL_CANONICAL_NAME  rremove
#define LOCAL_generic_seq_foo generic_seq_rremove
#define LOCAL_tsc_foo         tsc_rremove
#define LOCAL_DeeSeq_AttrFoo  RRemove
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemoveWithKey)
#define LOCAL_CANONICAL_NAME  rremove
#define LOCAL_generic_seq_foo generic_seq_rremove
#define LOCAL_tsc_foo         tsc_rremove_with_key
#define LOCAL_DeeSeq_AttrFoo  RRemoveWithKey
#define LOCAL_DeeSeq_AttrBar  RRemove
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAll)
#define LOCAL_CANONICAL_NAME  removeall
#define LOCAL_generic_seq_foo generic_seq_removeall
#define LOCAL_tsc_foo         tsc_removeall
#define LOCAL_DeeSeq_AttrFoo  RemoveAll
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey)
#define LOCAL_CANONICAL_NAME  removeall
#define LOCAL_generic_seq_foo generic_seq_removeall
#define LOCAL_tsc_foo         tsc_removeall_with_key
#define LOCAL_DeeSeq_AttrFoo  RemoveAllWithKey
#define LOCAL_DeeSeq_AttrBar  RemoveAll
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveIf)
#define LOCAL_CANONICAL_NAME  removeif
#define LOCAL_generic_seq_foo generic_seq_removeif
#define LOCAL_tsc_foo         tsc_removeif
#define LOCAL_DeeSeq_AttrFoo  RemoveIf
#elif defined(DEFINE_DeeType_SeqCache_RequireResize)
#define LOCAL_CANONICAL_NAME             resize
#define LOCAL_generic_seq_foo            generic_seq_resize
#define LOCAL_tsc_foo                    tsc_resize
#define LOCAL_DeeSeq_AttrFoo             Resize
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultResizeWithSizeAndTSCEraseAndTSCExtend /* Use erase() and extend() by default */
#elif defined(DEFINE_DeeType_SeqCache_RequireFill)
#define LOCAL_CANONICAL_NAME  fill
#define LOCAL_generic_seq_foo generic_seq_fill
#define LOCAL_tsc_foo         tsc_fill
#define LOCAL_DeeSeq_AttrFoo  Fill
#elif defined(DEFINE_DeeType_SeqCache_RequireReverse)
#define LOCAL_CANONICAL_NAME  reverse
#define LOCAL_generic_seq_foo generic_seq_reverse
#define LOCAL_tsc_foo         tsc_reverse
#define LOCAL_DeeSeq_AttrFoo  Reverse
#elif defined(DEFINE_DeeType_SeqCache_RequireReversed)
#define LOCAL_CANONICAL_NAME             reversed
#define LOCAL_generic_seq_foo            generic_seq_reversed
#define LOCAL_tsc_foo                    tsc_reversed
#define LOCAL_DeeSeq_AttrFoo             Reversed
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReversedWithCopyForeachDefault
#elif defined(DEFINE_DeeType_SeqCache_RequireSort)
#define LOCAL_CANONICAL_NAME  sort
#define LOCAL_generic_seq_foo generic_seq_sort
#define LOCAL_tsc_foo         tsc_sort
#define LOCAL_DeeSeq_AttrFoo  Sort
#elif defined(DEFINE_DeeType_SeqCache_RequireSortWithKey)
#define LOCAL_CANONICAL_NAME  sort
#define LOCAL_generic_seq_foo generic_seq_sort
#define LOCAL_tsc_foo         tsc_sort_with_key
#define LOCAL_DeeSeq_AttrFoo  SortWithKey
#define LOCAL_DeeSeq_AttrBar  Sort
#elif defined(DEFINE_DeeType_SeqCache_RequireSorted)
#define LOCAL_CANONICAL_NAME             sorted
#define LOCAL_generic_seq_foo            generic_seq_sorted
#define LOCAL_tsc_foo                    tsc_sorted
#define LOCAL_DeeSeq_AttrFoo             Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithCopyForeachDefault
#elif defined(DEFINE_DeeType_SeqCache_RequireSortedWithKey)
#define LOCAL_CANONICAL_NAME             sorted
#define LOCAL_generic_seq_foo            generic_seq_sorted
#define LOCAL_tsc_foo                    tsc_sorted_with_key
#define LOCAL_DeeSeq_AttrFoo             SortedWithKey
#define LOCAL_DeeSeq_AttrBar             Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault
#elif defined(DEFINE_DeeType_SeqCache_RequireBFind)
#define LOCAL_CANONICAL_NAME  bfind
#define LOCAL_generic_seq_foo generic_seq_bfind
#define LOCAL_tsc_foo         tsc_bfind
#define LOCAL_DeeSeq_AttrFoo  BFind
#elif defined(DEFINE_DeeType_SeqCache_RequireBFindWithKey)
#define LOCAL_CANONICAL_NAME  bfind
#define LOCAL_generic_seq_foo generic_seq_bfind
#define LOCAL_tsc_foo         tsc_bfind_with_key
#define LOCAL_DeeSeq_AttrFoo  BFindWithKey
#define LOCAL_DeeSeq_AttrBar  BFind
#elif defined(DEFINE_DeeType_SeqCache_RequireBPosition)
#define LOCAL_CANONICAL_NAME  bposition
#define LOCAL_generic_seq_foo generic_seq_bposition
#define LOCAL_tsc_foo         tsc_bposition
#define LOCAL_DeeSeq_AttrFoo  BPosition
#elif defined(DEFINE_DeeType_SeqCache_RequireBPositionWithKey)
#define LOCAL_CANONICAL_NAME  bposition
#define LOCAL_generic_seq_foo generic_seq_bposition
#define LOCAL_tsc_foo         tsc_bposition_with_key
#define LOCAL_DeeSeq_AttrFoo  BPositionWithKey
#define LOCAL_DeeSeq_AttrBar  BPosition
#elif defined(DEFINE_DeeType_SeqCache_RequireBRange)
#define LOCAL_CANONICAL_NAME  brange
#define LOCAL_generic_seq_foo generic_seq_brange
#define LOCAL_tsc_foo         tsc_brange
#define LOCAL_DeeSeq_AttrFoo  BRange
#elif defined(DEFINE_DeeType_SeqCache_RequireBRangeWithKey)
#define LOCAL_CANONICAL_NAME  brange
#define LOCAL_generic_seq_foo generic_seq_brange
#define LOCAL_tsc_foo         tsc_brange_with_key
#define LOCAL_DeeSeq_AttrFoo  BRangeWithKey
#define LOCAL_DeeSeq_AttrBar  BRange
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocate)
#define LOCAL_CANONICAL_NAME  blocate
#define LOCAL_generic_seq_foo generic_seq_blocate
#define LOCAL_tsc_foo         tsc_blocate
#define LOCAL_DeeSeq_AttrFoo  BLocate
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocateWithKey)
#define LOCAL_CANONICAL_NAME  blocate
#define LOCAL_generic_seq_foo generic_seq_blocate
#define LOCAL_tsc_foo         tsc_blocate_with_key
#define LOCAL_DeeSeq_AttrFoo  BLocateWithKey
#define LOCAL_DeeSeq_AttrBar  BLocate
#else /* DEFINE_DeeType_SeqCache_Require... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeType_SeqCache_Require... */

#ifndef LOCAL_DeeSeq_AttrBar
#define LOCAL_DeeSeq_AttrBar LOCAL_DeeSeq_AttrFoo
#endif /* !LOCAL_DeeSeq_AttrBar */

#ifndef LOCAL_DeeType_SeqCache_RequireFoo
#define LOCAL_DeeType_SeqCache_RequireFoo_private_uncached PP_CAT3(DeeType_SeqCache_Require, LOCAL_DeeSeq_AttrFoo, _private_uncached)
#define LOCAL_DeeType_SeqCache_RequireFoo_uncached         PP_CAT3(DeeType_SeqCache_Require, LOCAL_DeeSeq_AttrFoo, _uncached)
#define LOCAL_DeeType_SeqCache_RequireFoo                  PP_CAT2(DeeType_SeqCache_Require, LOCAL_DeeSeq_AttrFoo)
#else /* !LOCAL_DeeType_SeqCache_RequireFoo */
#ifndef LOCAL_DeeType_SeqCache_RequireFoo_private_uncached
#define LOCAL_DeeType_SeqCache_RequireFoo_private_uncached PP_CAT2(LOCAL_DeeType_SeqCache_RequireFoo, _private_uncached)
#endif /* !LOCAL_DeeType_SeqCache_RequireFoo_private_uncached */
#ifndef LOCAL_DeeType_SeqCache_RequireFoo_uncached
#define LOCAL_DeeType_SeqCache_RequireFoo_uncached PP_CAT2(LOCAL_DeeType_SeqCache_RequireFoo, _uncached)
#endif /* !LOCAL_DeeType_SeqCache_RequireFoo_uncached */
#endif /* LOCAL_DeeType_SeqCache_RequireFoo */

#ifndef LOCAL_DeeSeq_DefaultFooWithCallAttrBar
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar \
	PP_CAT4(DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCallAttr, LOCAL_DeeSeq_AttrBar)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallAttrBar */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction \
	PP_CAT5(DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataFunction)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod \
	PP_CAT5(DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataMethod)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod \
	PP_CAT5(DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataKwMethod)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod */
#ifndef LOCAL_DeeSeq_DefaultFooWithError
#define LOCAL_DeeSeq_DefaultFooWithError \
	PP_CAT3(DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithError)
#endif /* !LOCAL_DeeSeq_DefaultFooWithError */


#ifndef LOCAL_Dee_tsc_foo_t
#define LOCAL_Dee_tsc_foo_t PP_CAT3(Dee_, LOCAL_tsc_foo, _t)
#endif /* !LOCAL_Dee_tsc_foo_t */
#ifndef LOCAL_tsc_foo_data
#define LOCAL_tsc_foo_data PP_CAT3(tsc_, LOCAL_CANONICAL_NAME, _data)
#endif /* !LOCAL_tsc_foo_data */

#define LOCAL_CANONICAL_NAME_LENGTHOF                      COMPILER_STRLEN(PP_STR(LOCAL_CANONICAL_NAME))
#define LOCAL_CANONICAL_NAME_str                           PP_CAT2(str_, LOCAL_CANONICAL_NAME)
#define LOCAL_CANONICAL_NAME_STR                           PP_CAT2(STR_, LOCAL_CANONICAL_NAME)
#define LOCAL_CANONICAL_NAME_Dee_HashStr                   PP_CAT2(Dee_HashStr__, LOCAL_CANONICAL_NAME)


/* Mutable sequence functions */

PRIVATE WUNUSED NONNULL((1, 2)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireFoo_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self) {
	struct Dee_attrinfo attrinfo;
	(void)orig_type;

	/* Check if the type defines an attribute matching the canonical name of this function. */
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL,
	                                                LOCAL_CANONICAL_NAME_STR,
	                                                LOCAL_CANONICAL_NAME_LENGTHOF,
	                                                LOCAL_CANONICAL_NAME_Dee_HashStr,
	                                                &attrinfo)) {
		struct Dee_type_seq_cache *sc = DeeType_TryRequireSeqCache(self);
		if likely(sc) {
			switch (attrinfo.ai_type) {
			case Dee_ATTRINFO_METHOD:
				if ((Dee_funptr_t)attrinfo.ai_value.v_method->m_func == (Dee_funptr_t)&LOCAL_generic_seq_foo)
					return &LOCAL_DeeSeq_DefaultFooWithError;
				atomic_write(&sc->LOCAL_tsc_foo_data.d_method, attrinfo.ai_value.v_method->m_func);
				if (attrinfo.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
					return &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod;
				return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;
			case Dee_ATTRINFO_ATTR:
				ASSERT(attrinfo.ai_type == Dee_ATTRINFO_ATTR);
				if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
				    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
					struct class_desc *desc = DeeClass_DESC(self);
					uint16_t id = attrinfo.ai_value.v_attr->ca_addr;
					DREF DeeObject *callback;
					Dee_class_desc_lock_read(desc);
					callback = desc->cd_members[id];
					Dee_XIncref(callback);
					Dee_class_desc_lock_endread(desc);
					if likely(callback) {
						if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_foo_data.d_function, NULL, callback))
							Dee_Decref(callback);
						return &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction;
					}
				}
				break;
			default: break;
			}
		}
		return &LOCAL_DeeSeq_DefaultFooWithCallAttrBar;
	}

#ifdef DEFINE_DeeType_SeqCache_RequireFind
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireFindWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireRFind)
	if (DeeType_SeqCache_TryRequireEnumerateIndexReverse(self))
		return &DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_SeqCache_RequireRFindWithKey)
	if (DeeType_SeqCache_TryRequireEnumerateIndexReverse(self))
		return &DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_SeqCache_RequireErase)
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELRANGE))
		return &DeeSeq_DefaultEraseWithDelRangeIndex;
	{
		Dee_tsc_pop_t tsc_pop;
		tsc_pop = DeeType_SeqCache_RequirePop_private_uncached(orig_type, self);
		if (tsc_pop != NULL &&
		    tsc_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSeq_DefaultEraseWithPop;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireInsert)
	{
		Dee_tsc_insertall_t tsc_insertall;
		tsc_insertall = DeeType_SeqCache_RequireInsertAll_private_uncached(orig_type, self);
		if (tsc_insertall != NULL &&
		    tsc_insertall != &DeeSeq_DefaultInsertAllWithError)
			return &DeeSeq_DefaultInsertWithTSCInsertAll;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireInsertAll)
	if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE) &&
	    self->tp_seq->tp_setrange_index != &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll)
		return &DeeSeq_DefaultInsertAllWithSetRangeIndex;
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insert, 6, Dee_HashStr__insert, &attrinfo))
		return &DeeSeq_DefaultInsertAllWithTSCInsertForeach;
#elif defined(DEFINE_DeeType_SeqCache_RequirePushFront)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAppend)
	/* Check for "pushback()" */
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pushback, 8,
	                                                Dee_HashStr__pushback, &attrinfo)) {
		struct Dee_type_seq_cache *sc = DeeType_TryRequireSeqCache(self);
		if likely(sc) {
			switch (attrinfo.ai_type) {
			case Dee_ATTRINFO_METHOD:
				if ((Dee_funptr_t)attrinfo.ai_value.v_method->m_func == (Dee_funptr_t)&LOCAL_generic_seq_foo)
					return &LOCAL_DeeSeq_DefaultFooWithError;
				atomic_write(&sc->LOCAL_tsc_foo_data.d_method, attrinfo.ai_value.v_method->m_func);
				if (attrinfo.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
					return &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod;
				return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;
			case Dee_ATTRINFO_ATTR:
				ASSERT(attrinfo.ai_type == Dee_ATTRINFO_ATTR);
				if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
				    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
					struct class_desc *desc = DeeClass_DESC(self);
					uint16_t id = attrinfo.ai_value.v_attr->ca_addr;
					DREF DeeObject *callback;
					Dee_class_desc_lock_read(desc);
					callback = desc->cd_members[id];
					Dee_XIncref(callback);
					Dee_class_desc_lock_endread(desc);
					if likely(callback) {
						if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_foo_data.d_function, NULL, callback))
							Dee_Decref(callback);
						return &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction;
					}
				}
				break;
			default: break;
			}
		}
		return &DeeSeq_DefaultAppendWithCallAttrPushBack;
	}
	{
		Dee_tsc_extend_t tsc_extend;
		tsc_extend = DeeType_SeqCache_RequireExtend_private_uncached(orig_type, self);
		if (tsc_extend != NULL &&
		    tsc_extend != &DeeSeq_DefaultExtendWithError) {
			if (tsc_extend == &DeeSeq_DefaultExtendWithSizeAndTSCInsertAll)
				return &DeeSeq_DefaultAppendWithSizeAndTSCInsert;;
			return &DeeSeq_DefaultAppendWithTSCExtend;
		}
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		Dee_tsc_insert_t tsc_insert;
		tsc_insert = DeeType_SeqCache_RequireInsert_private_uncached(orig_type, self);
		if (tsc_insert != NULL &&
		    tsc_insert != &DeeSeq_DefaultInsertWithError)
			return &DeeSeq_DefaultAppendWithSizeAndTSCInsert;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireExtend)
	if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_append, 6, Dee_HashStr__append, &attrinfo))
			return &DeeSeq_DefaultExtendWithTSCAppendForeach;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pushback, 8, Dee_HashStr__pushback, &attrinfo))
			return &DeeSeq_DefaultExtendWithTSCAppendForeach;
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		Dee_tsc_insertall_t tsc_insertall;
		tsc_insertall = DeeType_SeqCache_RequireInsertAll_private_uncached(orig_type, self);
		if (tsc_insertall != NULL &&
		    tsc_insertall != &DeeSeq_DefaultInsertAllWithError)
			return &DeeSeq_DefaultExtendWithSizeAndTSCInsertAll;
	}
	if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
		Dee_tsc_insert_t tsc_insert;
		tsc_insert = DeeType_SeqCache_RequireInsert_private_uncached(orig_type, self);
		if (tsc_insert != NULL &&
			tsc_insert != &DeeSeq_DefaultInsertWithError)
			return &DeeSeq_DefaultExtendWithTSCAppendForeach;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireXchItemIndex)
	if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM))
		return &DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex;
#elif defined(DEFINE_DeeType_SeqCache_RequireClear)
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELRANGE)) {
		if (self->tp_seq->tp_delrange_index_n == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ||
		    self->tp_seq->tp_delrange_index_n == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault)
			return &DeeSeq_DefaultClearWithSetRangeIndexN;
		return &DeeSeq_DefaultClearWithDelRangeIndexN;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
		return &DeeSeq_DefaultClearWithSetRangeIndexN;
	{
		Dee_tsc_erase_t tsc_erase;
		tsc_erase = DeeType_SeqCache_RequireErase_private_uncached(orig_type, self);
		if (tsc_erase != NULL &&
		    tsc_erase != &DeeSeq_DefaultEraseWithError)
			return &DeeSeq_DefaultClearWithTSCErase;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequirePop)
	if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_erase, 5, Dee_HashStr__erase, &attrinfo))
			return &DeeSeq_DefaultPopWithSizeAndGetItemIndexAndTSCErase;
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
		return &DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex;
#elif defined(DEFINE_DeeType_SeqCache_RequireRemove)
	{
		Dee_tsc_removeall_t tsc_removeall;
		tsc_removeall = DeeType_SeqCache_RequireRemoveAll_private_uncached(orig_type, self);
		if (tsc_removeall != NULL &&
		    tsc_removeall != &DeeSeq_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultRemoveWithTSCRemoveAll;
	}
	{
		Dee_tsc_removeif_t tsc_removeif;
		tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
		if (tsc_removeif != NULL &&
		    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultRemoveWithTSCRemoveIf;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
		Dee_tsc_find_t tsc_find;
		tsc_find = DeeType_SeqCache_RequireFind_private_uncached(orig_type, self);
		if (tsc_find == &DeeSeq_DefaultFindWithEnumerateIndex)
			return &DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex;
		if (tsc_find)
			return &DeeSeq_DefaultRemoveWithTSCFindAndDelItemIndex;
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveWithKey)
	{
		Dee_tsc_removeall_with_key_t tsc_removeall_with_key;
		tsc_removeall_with_key = DeeType_SeqCache_RequireRemoveAllWithKey_private_uncached(orig_type, self);
		if (tsc_removeall_with_key != NULL &&
		    tsc_removeall_with_key != &DeeSeq_DefaultRemoveAllWithKeyWithError)
			return &DeeSeq_DefaultRemoveWithKeyWithTSCRemoveAllWithKey;
	}
	{
		Dee_tsc_removeif_t tsc_removeif;
		tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
		if (tsc_removeif != NULL &&
		    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
		Dee_tsc_find_with_key_t tsc_find_with_key;
		tsc_find_with_key = DeeType_SeqCache_RequireFindWithKey_private_uncached(orig_type, self);
		if (tsc_find_with_key == &DeeSeq_DefaultFindWithKeyWithEnumerateIndex)
			return &DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
		if (tsc_find_with_key)
			return &DeeSeq_DefaultRemoveWithKeyWithTSCFindWithKeyAndDelItemIndex;
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemove)
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
		Dee_tsc_rfind_t tsc_rfind;
		tsc_rfind = DeeType_SeqCache_RequireRFind_private_uncached(orig_type, self);
		if (tsc_rfind == &DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse)
			return &DeeSeq_DefaultRRemoveWithTSCEnumerateIndexReverseAndDelItemIndex;
		if (tsc_rfind == &DeeSeq_DefaultRFindWithEnumerateIndex)
			return &DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex;
		if (tsc_rfind)
			return &DeeSeq_DefaultRRemoveWithTSCRFindAndDelItemIndex;
		if (DeeType_SeqCache_TryRequireEnumerateIndexReverse(orig_type))
			return &DeeSeq_DefaultRRemoveWithTSCEnumerateIndexReverseAndDelItemIndex;
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemoveWithKey)
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
		Dee_tsc_rfind_with_key_t tsc_rfind_with_key;
		tsc_rfind_with_key = DeeType_SeqCache_RequireRFindWithKey_private_uncached(orig_type, self);
		if (tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse)
			return &DeeSeq_DefaultRRemoveWithKeyWithTSCEnumerateIndexReverseAndDelItemIndex;
		if (tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithEnumerateIndex)
			return &DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
		if (tsc_rfind_with_key)
			return &DeeSeq_DefaultRRemoveWithKeyWithTSCRFindWithKeyAndDelItemIndex;
		if (DeeType_SeqCache_TryRequireEnumerateIndexReverse(orig_type))
			return &DeeSeq_DefaultRRemoveWithKeyWithTSCEnumerateIndexReverseAndDelItemIndex;
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAll)
	{
		Dee_tsc_removeif_t tsc_removeif;
		tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
		if (tsc_removeif != NULL &&
		    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultRemoveAllWithTSCRemoveIf;
	}
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo))
		return &DeeSeq_DefaultRemoveAllWithTSCRemove;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
		return &DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex;
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
		return &DeeSeq_DefaultRemoveAllWithTSCRemove;
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey)
	{
		Dee_tsc_removeif_t tsc_removeif;
		tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
		if (tsc_removeif != NULL &&
		    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf;
	}
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo))
		return &DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
		return &DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex;
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
		return &DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey;
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveIf)
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removeall, 9, Dee_HashStr__removeall, &attrinfo))
		return &DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey;
	if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo))
		return &DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
		return &DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex;
	if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
		return &DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey;
#elif defined(DEFINE_DeeType_SeqCache_RequireResize)
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (DeeType_HasOperator(orig_type, OPERATOR_DELRANGE) &&
		    orig_type->tp_seq->tp_delrange_index != &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone &&
		    orig_type->tp_seq->tp_delrange_index != &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault)
			return &DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex;
		return &DeeSeq_DefaultResizeWithSizeAndSetRangeIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireFill)
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultFillWithSizeAndSetRangeIndex;
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER))
			return &DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireReverse)
	if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
		return &DeeSeq_DefaultReverseWithTSCReversedAndSetRangeIndex;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (DeeType_HasOperator(orig_type, OPERATOR_DELITEM))
			return &DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex;
		return &DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireReversed)
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		struct type_seq *seq = self->tp_seq;
		ASSERT(seq->tp_getitem_index);
		ASSERT(seq->tp_trygetitem_index);
		if (seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultReversedWithProxySizeAndGetItemIndexFast;
		if (!DeeType_IsDefaultGetItemIndex(seq->tp_getitem_index))
			return &DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex;
		if (!DeeType_IsDefaultTryGetItemIndex(seq->tp_trygetitem_index))
			return &DeeSeq_DefaultReversedWithProxySizeAndTryGetItemIndex;
		return &DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultReversedWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_SeqCache_RequireSort)
	if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
		return &DeeSeq_DefaultSortWithTSCSortedAndSetRangeIndex;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
		return &DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex;
#elif defined(DEFINE_DeeType_SeqCache_RequireSortWithKey)
	if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
		return &DeeSeq_DefaultSortWithKeyWithTSCSortedAndSetRangeIndex;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
		return &DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex;
#elif defined(DEFINE_DeeType_SeqCache_RequireSorted)
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (orig_type->tp_seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast;
		return &DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultSortedWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_SeqCache_RequireSortedWithKey)
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (orig_type->tp_seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast;
		return &DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_SeqCache_RequireBFind)
	{
		Dee_tsc_brange_t tsc_brange;
		tsc_brange = DeeType_SeqCache_RequireBRange_private_uncached(orig_type, self);
		if (tsc_brange != NULL &&
		    tsc_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithTSCBRange;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBFindWithKey)
	{
		Dee_tsc_brange_with_key_t tsc_brange_with_key;
		tsc_brange_with_key = DeeType_SeqCache_RequireBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_brange_with_key != NULL &&
		    tsc_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithKeyWithTSCBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBPosition)
	{
		Dee_tsc_brange_t tsc_brange;
		tsc_brange = DeeType_SeqCache_RequireBRange_private_uncached(orig_type, self);
		if (tsc_brange != NULL &&
		    tsc_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithTSCBRange;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBPositionWithKey)
	{
		Dee_tsc_brange_with_key_t tsc_brange_with_key;
		tsc_brange_with_key = DeeType_SeqCache_RequireBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_brange_with_key != NULL &&
		    tsc_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithKeyWithTSCBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBRange)
	if (DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) && DeeType_HasOperator(self, OPERATOR_SIZE))
		return &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex;
#elif defined(DEFINE_DeeType_SeqCache_RequireBRangeWithKey)
	if (DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) && DeeType_HasOperator(self, OPERATOR_SIZE))
		return &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex;
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocate)
	{
		Dee_tsc_bfind_t tsc_bfind;
		tsc_bfind = DeeType_SeqCache_RequireBFind_private_uncached(orig_type, self);
		if (tsc_bfind != NULL &&
		    tsc_bfind != &DeeSeq_DefaultBFindWithError) {
			if (tsc_bfind == &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithTSCBFindAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocateWithKey)
	{
		Dee_tsc_bfind_with_key_t tsc_bfind_with_key;
		tsc_bfind_with_key = DeeType_SeqCache_RequireBFindWithKey_private_uncached(orig_type, self);
		if (tsc_bfind_with_key != NULL &&
		    tsc_bfind_with_key != &DeeSeq_DefaultBFindWithKeyWithError) {
			if (tsc_bfind_with_key == &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithKeyWithTSCBFindWithKeyAndGetItemIndex;
		}
	}
#endif /* ... */

	return NULL;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireFoo_uncached(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	iter = DeeTypeMRO_Init(&mro, self);
	do {
		LOCAL_Dee_tsc_foo_t result;
		result = LOCAL_DeeType_SeqCache_RequireFoo_private_uncached(self, iter);
		if (result)
			return result;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return &LOCAL_DeeSeq_DefaultFooWithError;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_tsc_foo_t result;
	struct Dee_type_seq_cache *sc;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_foo)
			return sc->LOCAL_tsc_foo;
	}
	result = LOCAL_DeeType_SeqCache_RequireFoo_uncached(self);
	sc     = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->LOCAL_tsc_foo, result);
	return result;
}


#undef LOCAL_DeeSeq_AttrBar
#undef LOCAL_DeeType_SeqCache_RequireFoo_private_uncached
#undef LOCAL_DeeType_SeqCache_RequireFoo_uncached
#undef LOCAL_DeeType_SeqCache_RequireFoo
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBar
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod
#undef LOCAL_DeeSeq_DefaultFooWithError
#undef LOCAL_Dee_tsc_foo_t
#undef LOCAL_tsc_foo_data
#undef LOCAL_CANONICAL_NAME_LENGTHOF
#undef LOCAL_CANONICAL_NAME_str
#undef LOCAL_CANONICAL_NAME_STR
#undef LOCAL_CANONICAL_NAME_Dee_HashStr


#undef LOCAL_CANONICAL_NAME
#undef LOCAL_generic_seq_foo
#undef LOCAL_tsc_foo
#undef LOCAL_DeeSeq_AttrFoo
#undef LOCAL_DeeSeq_AttrBar

DECL_END

#undef DEFINE_DeeType_SeqCache_RequireFind
#undef DEFINE_DeeType_SeqCache_RequireFindWithKey
#undef DEFINE_DeeType_SeqCache_RequireRFind
#undef DEFINE_DeeType_SeqCache_RequireRFindWithKey
#undef DEFINE_DeeType_SeqCache_RequireErase
#undef DEFINE_DeeType_SeqCache_RequireInsert
#undef DEFINE_DeeType_SeqCache_RequireInsertAll
#undef DEFINE_DeeType_SeqCache_RequirePushFront
#undef DEFINE_DeeType_SeqCache_RequireAppend
#undef DEFINE_DeeType_SeqCache_RequireExtend
#undef DEFINE_DeeType_SeqCache_RequireXchItemIndex
#undef DEFINE_DeeType_SeqCache_RequireClear
#undef DEFINE_DeeType_SeqCache_RequirePop
#undef DEFINE_DeeType_SeqCache_RequireRemove
#undef DEFINE_DeeType_SeqCache_RequireRemoveWithKey
#undef DEFINE_DeeType_SeqCache_RequireRRemove
#undef DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
#undef DEFINE_DeeType_SeqCache_RequireRemoveAll
#undef DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
#undef DEFINE_DeeType_SeqCache_RequireRemoveIf
#undef DEFINE_DeeType_SeqCache_RequireResize
#undef DEFINE_DeeType_SeqCache_RequireFill
#undef DEFINE_DeeType_SeqCache_RequireReverse
#undef DEFINE_DeeType_SeqCache_RequireReversed
#undef DEFINE_DeeType_SeqCache_RequireSort
#undef DEFINE_DeeType_SeqCache_RequireSortWithKey
#undef DEFINE_DeeType_SeqCache_RequireSorted
#undef DEFINE_DeeType_SeqCache_RequireSortedWithKey
#undef DEFINE_DeeType_SeqCache_RequireBFind
#undef DEFINE_DeeType_SeqCache_RequireBFindWithKey
#undef DEFINE_DeeType_SeqCache_RequireBPosition
#undef DEFINE_DeeType_SeqCache_RequireBPositionWithKey
#undef DEFINE_DeeType_SeqCache_RequireBRange
#undef DEFINE_DeeType_SeqCache_RequireBRangeWithKey
#undef DEFINE_DeeType_SeqCache_RequireBLocate
#undef DEFINE_DeeType_SeqCache_RequireBLocateWithKey
